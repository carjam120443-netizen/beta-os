BUILD := build

.PHONY: all clean run

all: $(BUILD)/beta-os.img

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/boot.bin: boot/boot.asm | $(BUILD)
	nasm -f bin $< -o $@

$(BUILD)/beta-os.img: $(BUILD)/boot.bin
	dd if=/dev/zero of=$@ bs=512 count=2880 status=none
	dd if=$(BUILD)/boot.bin of=$@ bs=512 count=1 conv=notrunc status=none

run: $(BUILD)/beta-os.img
	qemu-system-x86_64 -drive format=raw,file=$(BUILD)/beta-os.img

clean:
	rm -rf $(BUILD)
