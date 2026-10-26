cmake_minimum_required(VERSION 3.15)
project(DestinX LANGUAGES C ASM_NASM)

include_directories(include)

file(GLOB_RECURSE KERNEL_SOURCES "*.c" "*.asm")
list(FILTER KERNEL_SOURCES EXCLUDE REGEX "files/.*")

add_executable(kernel.bin ${KERNEL_SOURCES})

target_link_options(kernel.bin PRIVATE 
    "-T${CMAKE_CURRENT_SOURCE_DIR}/arch/x86_64/boot/linker.ld"
    "-ffreestanding"
    "-O2"
    "-nostdlib"
)
target_link_libraries(kernel.bin PRIVATE gcc)

add_custom_command(
    TARGET kernel.bin
    POST_BUILD
    COMMAND grub-file --is-x86-multiboot2 kernel.bin
    COMMAND ${CMAKE_COMMAND} -E copy kernel.bin ${CMAKE_CURRENT_SOURCE_DIR}/isodir/boot/kernel.bin
    COMMAND grub-mkrescue -o kernel.iso ${CMAKE_CURRENT_SOURCE_DIR}/isodir
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/files/test.bin ${CMAKE_CURRENT_SOURCE_DIR}/files/test.elf
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/files/test.asm
    COMMAND nasm -f bin ${CMAKE_CURRENT_SOURCE_DIR}/files/test.asm -o ${CMAKE_CURRENT_SOURCE_DIR}/files/test.bin
    COMMAND nasm -f elf64 ${CMAKE_CURRENT_SOURCE_DIR}/files/test.asm -o test_user.o
    COMMAND x86_64-elf-ld test_user.o -o ${CMAKE_CURRENT_SOURCE_DIR}/files/test.elf
)

add_custom_target(compile_progs DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/files/test.bin ${CMAKE_CURRENT_SOURCE_DIR}/files/test.elf)

add_custom_command(
    OUTPUT disk.img
    DEPENDS 
        ${CMAKE_CURRENT_SOURCE_DIR}/files/test.txt 
        ${CMAKE_CURRENT_SOURCE_DIR}/files/test.bin 
        ${CMAKE_CURRENT_SOURCE_DIR}/files/test.elf
        
    COMMAND mkfs.dfs disk.img 32
    COMMAND dfs-tools disk.img mkdir /usr
    COMMAND dfs-tools disk.img mkdir /usr/bin
    COMMAND dfs-tools disk.img mkdir /usr/txt
    COMMAND dfs-tools disk.img add ${CMAKE_CURRENT_SOURCE_DIR}/files/test.txt /usr/txt/test.txt
    COMMAND dfs-tools disk.img add ${CMAKE_CURRENT_SOURCE_DIR}/files/test.bin /usr/bin/test.bin
    COMMAND dfs-tools disk.img add ${CMAKE_CURRENT_SOURCE_DIR}/files/test.elf /usr/bin/test.elf
)

add_custom_target(dfs_disk ALL DEPENDS disk.img compile_progs)

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set(QEMU_ARCH_BINARY "qemu-system-x86_64")
elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i686" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "i386")
    set(QEMU_ARCH_BINARY "qemu-system-i386")
else()
    set(QEMU_ARCH_BINARY "qemu-system-${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(QEMUFLAGS -m 4096 -accel kvm -cpu host,migratable=no,+invtsc -machine q35,acpi=on -serial stdio -drive id=disk,file=disk.img,format=raw,if=none -device ide-hd,drive=disk,bus=ide.0 -device virtio-gpu-pci,disable-legacy=on)

add_custom_target(run
    COMMAND ${QEMU_ARCH_BINARY} -cdrom kernel.iso ${QEMUFLAGS}
    DEPENDS kernel.bin dfs_disk
)