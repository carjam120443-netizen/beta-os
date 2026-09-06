BUILD := build
KERNEL := $(BUILD)/kernel.elf
ISO := $(BUILD)/beta-os.iso

CFLAGS := -m64 -ffreestanding -fno-pie -fno-stack-protector -mno-red-zone -Wall -Wextra -Werror -O2
LDFLAGS := -m elf_x86_64 -nostdlib -z max-page-size=0x1000

KERNEL_OBJECTS := $(BUILD)/boot.o $(BUILD)/kernel.o $(BUILD)/fs.o $(BUILD)/net.o $(BUILD)/exec.o $(BUILD)/shell.o

.PHONY: all iso run clean

all: $(ISO)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.o: boot/boot.asm | $(BUILD)
	nasm -f elf64 $< -o $@

$(BUILD)/kernel.o: kernel/kernel.c kernel/fs.h kernel/net.h kernel/exec.h kernel/shell.h | $(BUILD)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD)/fs.o: kernel/fs.c kernel/fs.h | $(BUILD)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD)/net.o: kernel/net.c kernel/net.h | $(BUILD)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD)/exec.o: kernel/exec.c kernel/exec.h | $(BUILD)
	gcc $(CFLAGS) -c $< -o $@

$(BUILD)/shell.o: kernel/shell.c kernel/shell.h kernel/fs.h kernel/net.h kernel/exec.h | $(BUILD)
	gcc $(CFLAGS) -c $< -o $@

$(KERNEL): $(KERNEL_OBJECTS) boot/linker.ld
	ld $(LDFLAGS) -T boot/linker.ld -o $@ $(KERNEL_OBJECTS)

$(ISO): $(KERNEL) grub/grub.cfg pkg/pkg.conf
	mkdir -p $(BUILD)/iso/boot/grub $(BUILD)/iso/etc/pkg
	cp $(KERNEL) $(BUILD)/iso/boot/kernel.elf
	cp grub/grub.cfg $(BUILD)/iso/boot/grub/grub.cfg
	cp pkg/pkg.conf $(BUILD)/iso/etc/pkg/pkg.conf
	grub-mkrescue -o $@ $(BUILD)/iso

iso: $(ISO)

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO)

clean:
	rm -rf $(BUILD)
