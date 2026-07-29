# PointerOS: A 64-bit x86 Hobby Operating System

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
* **Development Environment:** WSL2 Ubuntu

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

### Scheduling & Sync
- [x] **Multithreading**: Round Robin scheduler implemented.
- [x] **Synchronization**: `Spinlock` and `Mutex` are implemented.

### User Stack Protection
- [x] **Page Guard**: Expand stack when needed and close on stack limit.

### Syscalls
- [x] **SYS EXIT**: Kills current thread.

### Usersapace
- [x] **Switching to ring 3**: User threads can execute code.
- [X] **TEST THREAD**: Currently there is `TEST THREAD` that was mapped to only test if it works.
- [X] **TEST EXIT ROUTINE**: `EXIT ROUTINE` is just test version and will be improved.

---

## Roadmap & In Progress

- [ ] **Processes**: Isolation of tasks from one another.

---
## Getting Started

| **Category** | **Tool** | **Notes** |
| :--- | :--- | :--- |
| **Cross-Compiler** | `x86_64-elf-gcc` | Requires `-mno-red-zone` flag |
| **Assembler** | `nasm` | For `bootstrapper` and `ASM` parts |
| **Emulator** | `QEMU` | Run with `-machine q35` & `OVMF` |
| **ISO Tools** | `grub-common` | Uses `grub-mkrescue` |
| **UEFI Support** | `OVMF` | Provides `ACPI 2.0` runtime modules |

### Build & Run
```bash
# Clone the repository
git clone https://github.com/MrTop4ik/PointerOS/
cd PointerOS

# Create logs directory
mkdir logs

# Compile and create ISO image
make iso

# Run in QEMU
make run
```

### Bug Tracker
* **KLog Flakiness**: `Kernel log buffer` occasionally misses rendering text frames onto the `LFB` screen, though serial output remains intact.
* **Arch Linux UEFI Crash**: Booting on `Arch Linux` via `UEFI` triggers an immediate `Page Fault` (#PF). Investigating alternative linker/alignment constraints.

### Cross-Platform Notice
The development workflow is heavily tuned for `WSL2 Ubuntu`. If you are building on other systems, keep in mind:
* **Native Linux**: Paths to the `OVMF.fd` image vary significantly across distributions (e.g., Debian vs Fedora vs Arch).
* **Windows (Native)**: Requires specialized environments like `WSL2/MSYS2/Cygwin` to resolve standard GNU utils, toolchains, and grub-mkrescue.
* **macOS**: Standard QEMU syntax differs, and an explicit `cross-compiler` target configuration is strictly mandatory.
