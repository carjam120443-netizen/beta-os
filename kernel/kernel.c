#include <stdint.h>
#include "fs.h"
#include "net.h"
#include "exec.h"
#include "shell.h"

#define VGA_MEMORY ((volatile uint16_t*)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static void clear_screen(void) {
    for (uint32_t y = 0; y < VGA_HEIGHT; ++y) {
        for (uint32_t x = 0; x < VGA_WIDTH; ++x) {
            VGA_MEMORY[y * VGA_WIDTH + x] = 0x0700 | ' ';
        }
    }
}

static void print_at(const char *text, uint32_t row) {
    uint32_t column = 0;
    while (text[column] && column < VGA_WIDTH) {
        VGA_MEMORY[row * VGA_WIDTH + column] = 0x0F00 | (uint8_t)text[column];
        ++column;
    }
}

void kmain(uint32_t multiboot_info) {
    (void)multiboot_info;
    clear_screen();
    print_at("Beta OS", 1);
    print_at("x86_64 kernel online.", 2);
    print_at("Initializing filesystem, networking, userspace, and shell...", 4);

    fs_init();
    exec_init();
    net_init();
    (void)fs_create("/etc/os-release", "NAME=Beta OS\nARCH=x86_64\n", 25);
    (void)exec_register("/bin/init", 0, 0);

    shell_run();
    for (;;) __asm__ volatile ("hlt");
}
