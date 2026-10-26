set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER x86_64-elf-gcc)
set(CMAKE_CXX_COMPILER x86_64-elf-g++)
set(CMAKE_ASM_NASM_COMPILER nasm)

set(X86_64_KERNEL_C_FLAGS 
    -ffreestanding 
    -O2 
    -Wall 
    -Wextra 
    -mno-red-zone 
    -mno-sse 
    -mno-sse2 
    -mcmodel=kernel
)

set(X86_64_KERNEL_ASM_FLAGS 
    "-f elf64" 
    "-i ${CMAKE_SOURCE_DIR}/arch/x86_64/boot/"
)
