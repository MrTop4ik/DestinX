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
- [x] **GDT:** Configured.
- [x] **IDT:** Configured.
- [x] **Paging:** 64-bit Paging Enabled.
- [x] **Higher Half Kernel:** Implemented.
- [x] **KMemory Allocation:**
    - **PMM** Implemented.
    - **VMM** Implemented.
    - **Buddy Allocator** Implemented.
    - **Slab Allocator** Implemented.
    - **KMAlloc** Implemented.
    - **KFree** Implemented.
    - **VMalloc** Implemented.
    - **VFree** Implemented.
- [x] **Drivers:** 
    - **LFB** Implemented.
    - **QEMU Serial** Implemented.
    - **PIT** Implemented.
    - **LAPIC Timer** Implemented.
- [x] **Scheduler:**
    - **Multithreading** Implemented.
- [x] **Synchronization:**
    - **Spinlock** Implemented.
    - **Mutex** Implemented.
- [x] **Kernel Logging:**
    - **KLog Buffer** Implemented.
    - **Idle Thread Logging**
- [ ] **Syscalls:** In Progress.
- [ ] **Userspace:** In Progress.
## Getting Started

### Prerequisites
| Category | Tool | Notes |
| :--- | :--- | :--- |
| **Cross-Compiler** | `x86_64-elf-gcc` | Required `-mno-red-zone` flag to prevent stack corruption |
| **Assembler** | `nasm` | Used for bootloader entry and interrupt stubs |
| **Emulator** | `QEMU` | Run with `-machine q35` and `-bios OVMF.fd` |
| **ISO Creation** | `grub-common` | Uses `grub-mkrescue` to build the bootable image |
| **UEFI Support** | `grub-efi-amd64-bin` | Provides modules for `UEFI` booting and `ACPI 2.0` (Debian/Ubuntu) |

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

## Potential Issues on Other Platforms
Since the project is primarily built and tested under **WSL2 Ubuntu**, there may be differences or issues when running on other environments:

- **Arch Linux:**  
  - No separate `grub-efi-amd64-bin` package.  
  - Currently there are **issues with booting on UEFI** that are being investigated and worked on.

- **Pure Linux (non-WSL):**  
  - QEMU options (`-bios OVMF.fd`, `-machine q35`) may differ depending on package versions.  
  - Paths to `OVMF.fd` can vary between distros.


- **Windows (without WSL2):**  
  - Building with `nasm` and `x86_64-elf-gcc` requires a cross-toolchain setup (e.g., MSYS2 or Cygwin).  
  - ISO creation with `grub-mkrescue` may not work out of the box.

- **macOS:**  
  - QEMU installation and EFI firmware paths differ.  
  - GNU toolchain versions may cause incompatibilities.

## Current Issues
- **Userspace:**
  - Thread Exit in Create User Thread function won't work.
- **KLog:**
  - Sometimes it doesn't printing to screen and sometimes printing.
- **Booting on Archlinux**
  - Currently it doesn't booting with UEFI and raises #PF.