set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER x86_64-elf-gcc)
set(CMAKE_CXX_COMPILER x86_64-elf-g++)
set(CMAKE_ASM_NASM_COMPILER nasm)

set(CMAKE_C_FLAGS "-std=gnu99 -ffreestanding -O2 -Wall -Wextra -mno-red-zone -mno-red-zone -mno-sse -mno-sse2 -mcmodel=kernel" CACHE STRING "" FORCE)
set(CMAKE_ASM_NASM_FLAGS "-f elf64 -i ${CMAKE_CURRENT_SOURCE_DIR}/arch/x86_64/boot/" CACHE STRING "" FORCE)
