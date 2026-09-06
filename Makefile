BUILD := build
KERNEL := $(BUILD)/kernel.elf
ISO := $(BUILD)/beta-os.iso

CFLAGS := -m64 -ffreestanding -fno-pie -fno-stack-protector -mno-red-zone -Wall -Wextra -Werror -O2
LDFLAGS := -m elf_x86_64 -nostdlib -z max-page-size=0x1000

.PHONY: all iso run clean

all: $(ISO)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.o: boot/boot.asm | $(BUILD)
	nasm -f elf64 $< -o $@

$(BUILD)/kernel.o: kernel/kernel.c | $(BUILD)
	gcc $(CFLAGS) -c $< -o $@

$(KERNEL): $(BUILD)/boot.o $(BUILD)/kernel.o boot/linker.ld
	ld $(LDFLAGS) -T boot/linker.ld -o $@ $(BUILD)/boot.o $(BUILD)/kernel.o

$(ISO): $(KERNEL) grub/grub.cfg
	mkdir -p $(BUILD)/iso/boot/grub
	cp $(KERNEL) $(BUILD)/iso/boot/kernel.elf
	cp grub/grub.cfg $(BUILD)/iso/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(BUILD)/iso

iso: $(ISO)

run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO)

clean:
	rm -rf $(BUILD)
