# DestinX: A 64-bit x86 Hobby Operating System

A hobby operating system kernel written from scratch in **C** and **x86 Assembly**.

---

> [!NOTE]
> Due to frequent power outages in my city, 
> I currently have limited opportunities to commit regularly.
> Development of the OS is ongoing, but updates may appear less frequently than usual.

## Tech Stack
* **Language:** C (Freestanding, no standard library), x86 Assembly (NASM)
* **Architecture:** x86_64 (x86 64-bit)
* **Build System:** Makefile, Linker scripts
* **Emulation:** QEMU
* **Bootloader:** GRUB (Multiboot2)
* **Development Environment:** ArchLinux

## Current Features

### Core & Memory Management
- [x] **GDT** & **IDT**: Fully configured and loaded.
- [x] **Paging**: 64-bit paging enabled with a Higher Half Kernel architecture.
- [x] **Memory Allocators**:
  - Physical Memory Manager (**PMM**)
  - Virtual Memory Manager (**VMM**)
  - **Buddy Allocator** & **Slab Allocator**
  - Kernel Dynamic Memory allocation with `KMAlloc` / `Vmalloc`

### Drivers & Subsystems
- [x] **Graphics**: Linear Framebuffer (**LFB**) output.
- [x] **I/O**: QEMU Serial logging interface.
- [x] **Timers**: Programmable Interval Timer (**PIT**) & **LAPIC Timer**.
- [x] **PCI**: **Reading** and **writting** dword, enabling **MSI**.
- [x] **AHCI**: **Reading** and **writting** data to disk.

### Scheduling & Sync
- [x] **Multithreading**: Round Robin scheduler implemented.
- [x] **Synchronization**: `Spinlock` and `Mutex` are implemented.

### Syscalls
- [x] **SYS READ**: Reading from `file` by fd, buffer pointer, and length passed in rdi, rsi, and rdx, respectively. 
- [x] **SYS WRITE**: Write to `file` by fd, buffer pointer, and length passed in rdi, rsi, and rdx, respectively.
- [x] **SYS OPEN**: Create `struct FILE` in `fd table` for file with path passed in rdi and return `fd`.
- [x] **SYS CLOSE**: Remove `struct FILE` in `fd table` by fd passed in rdi and return `status` (0 for success).
- [x] **SYS LSEEK**: Move files `position` by fd, offset and whence passed in rdi, rsi, and rdx, respectively.
- [x] **SYS MMAP**: Allocate `memory` with `certain` flags.
- [x] **SYS MUNMAP**: Free `memory` allocated with `SYS MMAP` by addr from rdi.
- [x] **SYS BRK**: Moves current user process's `heap end` to addr passed in rdi.
- [x] **SYS EXIT**: Kills current `thread`.
- [x] **SYS EXIT GROUP**: Kills `every` process's thread.

### Usersapace
- [x] **Switching to ring 3**: User threads can execute code.
- [x] **Page Guard**: Expand stack when needed and close on stack limit.
- [x] **Processes**: Isolation of tasks from one another.
- [x] **ELF parser**: Parse ELF files and load them.
- [x] **Heap**: Allocate memory with `SYS BRK`.

### File System
- [x] **VFS**: Provide single, uniform interface for programs to use files.
- [x] **DFS**:
  - **Mounting** root directory.
  - **Reading** files data.
  - **Writting** to files.
---

## Roadmap & In Progress
- [ ] **Page Cache**: Create page cache array to optimize reading and writting to files.
- [ ] **Optimize LFB**: Optimize LFB with AVX instructions.
- [ ] **Support more PROT and MAP**: Rewrite SYS MMAP with more prots and flags.
- [ ] **LibC**: libc support for user programs.

---
## Getting Started

| **Category** | **Tool** | **Notes** |
| :--- | :--- | :--- |
| **Cross-Compiler** | `x86_64-elf-gcc` | Requires `-mno-red-zone` flag |
| **Assembler** | `nasm` | For `bootstrapper` and `ASM` parts |
| **Emulator** | `QEMU` | Run with `-machine q35` & `OVMF` |
| **ISO Tools** | `grub-common` | Uses `grub-mkrescue` |
| **UEFI Support** | `OVMF` | Provides `ACPI 2.0` runtime modules |
| **FS Support** | `DFS` | Provides `DFS` uitls |

### Build & Run
```bash
# Clone the repository
git clone https://github.com/MrTop4ik/DestinX
cd DestinX

# Create logs directory
mkdir logs

# Compile and create ISO image
make iso

# Compile amd link programs
make progs

# Make disk image
make disk

# Run in QEMU
make run
```

### Bug Tracker
* **Arch Linux UEFI Crash**: Booting on `Arch Linux` via `UEFI` triggers an immediate `Page Fault` (#PF).
* **SYSRETQ**: Currently, `sysretq` does not work correctly, so `iretq` is used instead.

### Cross-Platform Notice
The development workflow is heavily tuned for `ArchLinux and WSL2 Ubuntu`. If you are building on other systems, keep in mind:
* **Native Linux**: Paths to the `OVMF.fd` image vary significantly across distributions (e.g., Debian vs Fedora vs Arch).
* **Windows (Native)**: Requires specialized environments like `WSL2/MSYS2/Cygwin` to resolve standard GNU utils, toolchains, and grub-mkrescue.
* **macOS**: Standard QEMU syntax differs, and an explicit `cross-compiler` target configuration is strictly mandatory.
