# Beta OS 🧪

> A from-scratch x86_64 operating system project.

Beta OS is an experimental operating system being built from the ground up for learning, experimentation, and eventually becoming a complete standalone OS.

**This is not a Linux distribution.** The goal is to write the kernel, low-level components, userspace, drivers, filesystem support, and tooling ourselves.

## 🚧 Current status

**Stage: Bootloader prototype**

The repository currently contains a minimal BIOS boot sector that creates a bootable disk image and displays an initial boot message.

### Roadmap

- [x] Initial BIOS boot sector
- [x] Bootable disk image
- [x] QEMU run target
- [x] Automated GitHub Actions build
- [ ] Enter x86_64 long mode
- [ ] Load a real kernel
- [ ] Kernel entry point
- [ ] GDT and IDT
- [ ] Interrupt handling
- [ ] Physical and virtual memory management
- [ ] Keyboard input
- [ ] Timer support
- [ ] Kernel heap
- [ ] Basic process/task system
- [ ] System calls
- [ ] Filesystem
- [ ] Userspace and `init`
- [ ] Interactive shell
- [ ] Networking
- [ ] Graphics/framebuffer
- [ ] Desktop environment

## 🏗️ Project layout

```text
beta-os/
├── boot/                 # Boot code
│   └── boot.asm
├── kernel/               # Kernel source
├── libc/                 # Userspace C library
├── userspace/            # Programs and shell
├── filesystem/           # Filesystem implementation
├── tools/                # Development/build tools
├── scripts/              # Build and testing scripts
├── .github/workflows/    # Automated builds
└── Makefile              # Local build entry point
```

## 🔨 Building

### Requirements

A Linux development environment with:

- `make`
- `nasm`
- `qemu-system-x86_64`

Build the disk image:

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

The generated image is placed at:

```text
build/beta-os.img
```

## 🤖 Automated builds

Every push and pull request can build the OS through GitHub Actions. The workflow installs NASM and QEMU, creates the disk image, performs a basic QEMU smoke test, and uploads the resulting image as an artifact.

## 🎯 Long-term goal

Beta OS is intended to grow from a tiny bootable experiment into a real general-purpose operating system with its own:

- Kernel
- Boot process
- Drivers
- Process model
- Memory manager
- Filesystem
- Userspace
- Shell
- Package manager
- Networking stack
- Graphical system

The package manager is planned to eventually provide commands such as:

```text
pkg install <package>
pkg remove <package>
pkg update
pkg upgrade
pkg search <package>
```

## 🧪 Development philosophy

Beta OS will be developed in small, testable stages. Early versions will prioritize understanding and correctness over performance or feature count.

QEMU and VirtualBox are the primary testing targets while the low-level system is under development.

## 📜 License

A project license will be added as the project matures.

---

**Beta OS is experimental software. Expect bugs, crashes, missing features, and occasional kernel-induced chaos.** 💀
