# Beta OS 🧪

> A from-scratch **x86_64** operating system.

Beta OS is an experimental operating system being written from the ground up. It is **not Linux-based** and does not use a Linux kernel.

The project starts at the bootloader/kernel boundary and will gradually grow into a complete operating system with its own kernel, drivers, userspace, filesystem, shell, package manager, and graphical stack.

## 🚀 Current status

**Stage: x86_64 kernel bring-up**

Beta OS now has a real 64-bit kernel entry path. GRUB loads the kernel using Multiboot2, the bootstrap code creates initial paging structures, enables CPU long mode, and transfers control to a freestanding C kernel.

The kernel currently initializes the VGA text display and confirms that it reached 64-bit mode.

### Roadmap

- [x] Initial boot prototype
- [x] x86_64 kernel ELF
- [x] Multiboot2 boot header
- [x] 64-bit long-mode transition
- [x] Initial page tables
- [x] C kernel entry point
- [x] VGA text output
- [x] Bootable ISO
- [x] GitHub Actions build
- [ ] GDT cleanup and kernel descriptor management
- [ ] IDT and interrupt handlers
- [ ] PIC/APIC support
- [ ] Physical memory manager
- [ ] Virtual memory manager
- [ ] Kernel heap allocator
- [ ] PS/2 keyboard driver
- [ ] PIT/HPET/APIC timer
- [ ] Process and thread scheduler
- [ ] System-call interface
- [ ] Executable loader
- [ ] Virtual filesystem layer
- [ ] Disk driver
- [ ] BetaFS filesystem
- [ ] `init` and userspace
- [ ] Interactive shell
- [ ] `pkg` package manager
- [ ] Networking stack
- [ ] Framebuffer graphics
- [ ] Window manager
- [ ] Desktop environment

## 🧠 Architecture

Beta OS targets **64-bit x86 (x86_64 / AMD64)**.

```text
BIOS/firmware
      │
      ▼
    GRUB
      │
      ▼
 Multiboot2
      │
      ▼
32-bit bootstrap
      │
      ├── page tables
      ├── PAE
      ├── EFER.LME
      └── CR0.PG
      │
      ▼
 x86_64 long mode
      │
      ▼
  C kernel
      │
      ▼
  Kernel services
```

The current kernel identity-maps the first 2 MiB with a 2 MiB page so the earliest 64-bit code has a simple address space. This will be replaced by a proper virtual-memory subsystem later.

## 📁 Project layout

```text
beta-os/
├── boot/
│   ├── boot.asm          # x86_64 entry + long-mode setup
│   └── linker.ld         # Kernel linker script
├── grub/
│   └── grub.cfg          # Boot menu/configuration
├── kernel/
│   └── kernel.c          # First C kernel
├── libc/                 # Future userspace C library
├── userspace/            # Future userspace programs
├── filesystem/           # Future filesystem code
├── tools/                # Development tools
├── scripts/              # Build/test scripts
├── .github/workflows/    # CI builds
└── Makefile              # Kernel + ISO build system
```

## 🔨 Build Beta OS

### Requirements

A Linux development environment with:

- GCC
- GNU binutils
- Make
- NASM
- GRUB tools
- Xorriso
- QEMU

Build the x86_64 kernel and bootable ISO:

```bash
make
```

Run it in QEMU:

```bash
make run
```

Clean generated files:

```bash
make clean
```

Build output:

```text
build/kernel.elf
build/beta-os.iso
```

## 🖥️ VirtualBox

The generated ISO is intended to be usable as a BIOS boot ISO in VirtualBox as well as QEMU.

Create a new VM with:

- Type: **Other**
- Architecture: **x86_64**
- RAM: 512 MB or more
- Storage: optional for now
- Optical drive: `beta-os.iso`
- EFI: disabled for the current BIOS/GRUB boot path

UEFI support will be added later.

## 🤖 Continuous integration

GitHub Actions builds the x86_64 kernel and ISO on pushes, pull requests, and manual workflow runs. The workflow also performs a QEMU smoke test and uploads both the ISO and kernel ELF as build artifacts.

## 📦 Future package manager

Beta OS will eventually have its own native package manager called `pkg`.

Planned commands:

```text
pkg search <name>
pkg install <name>
pkg remove <name>
pkg update
pkg upgrade
pkg list
pkg info <name>
pkg repo add <url>
```

The package format, repository format, dependency resolver, signatures, and installation database will be designed specifically for Beta OS rather than copied directly from an existing Linux distribution.

## 🎯 Long-term vision

The end goal is a standalone general-purpose OS built layer by layer:

**boot → kernel → drivers → memory → processes → filesystem → userspace → networking → graphics → desktop → applications**

No Linux kernel. No Ubuntu base. No Debian base. Just Beta OS. 🧪

## ⚠️ Experimental

Beta OS is extremely early-stage software. Crashes and incomplete functionality are expected while the kernel is being developed.

## 📜 License

A project license will be added as the project matures.
