#include "shell.h"
#include "fs.h"
#include "net.h"
#include "exec.h"
#include <stdint.h>

#define VGA_MEMORY ((volatile uint16_t *)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint32_t row;
static uint32_t col;

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void putc(char c) {
    if (c == '\n') { row = (row + 1) % VGA_HEIGHT; col = 0; return; }
    VGA_MEMORY[row * VGA_WIDTH + col] = 0x0F00 | (uint8_t)c;
    col = (col + 1) % VGA_WIDTH;
    if (col == 0) row = (row + 1) % VGA_HEIGHT;
}

static void puts(const char *s) {
    while (*s) putc(*s++);
}

static int eq(const char *a, const char *b) {
    size_t i = 0;
    while (a[i] && b[i] && a[i] == b[i]) ++i;
    return a[i] == b[i];
}

static char keymap(uint8_t scancode) {
    static const char map[] = "\0\0" "1234567890-=" "\0" "qwertyuiop[]" "\0" "asdfghjkl;'`" "\\" "zxcvbnm,./";
    if (scancode < 2 || scancode > 53) return 0;
    return map[scancode];
}

static uint8_t keyboard_get(void) {
    while ((inb(0x64) & 1u) == 0) { __asm__ volatile ("pause"); }
    return inb(0x60);
}

static void command(const char *cmd) {
    if (eq(cmd, "help")) {
        puts("help  clear  uname  ls  cat  mem  net  pkg");
    } else if (eq(cmd, "clear")) {
        for (uint32_t y = 0; y < VGA_HEIGHT; ++y) for (uint32_t x = 0; x < VGA_WIDTH; ++x) VGA_MEMORY[y * VGA_WIDTH + x] = 0x0700 | ' ';
        row = 0; col = 0;
    } else if (eq(cmd, "uname")) {
        puts("Beta OS x86_64");
    } else if (eq(cmd, "ls")) {
        puts("/  /bin  /dev  /etc  /home  /tmp  /usr");
    } else if (eq(cmd, "cat")) {
        puts("usage: cat <file>");
    } else if (eq(cmd, "mem")) {
        puts("memory manager: early kernel allocator pending");
    } else if (eq(cmd, "net")) {
        puts(net_is_ready() ? "network core: ready (loopback)" : "network core: offline");
    } else if (eq(cmd, "pkg")) {
        puts("pkg: OpenBSD-current amd64 repository configured");
        puts("usage: pkg update | search | install | remove | upgrade | info");
    } else if (cmd[0] == '\0') {
        return;
    } else {
        puts("unknown command");
    }
}

void shell_run(void) {
    char buffer[80];
    size_t length = 0;
    puts("Beta OS shell\n");
    puts("Type 'help' for commands.\n\n");
    puts("beta# ");
    for (;;) {
        uint8_t scancode = keyboard_get();
        if (scancode & 0x80u) continue;
        if (scancode == 28) {
            buffer[length] = '\0';
            command(buffer);
            putc('\n');
            length = 0;
            puts("beta# ");
        } else if (scancode == 14) {
            if (length > 0) { --length; if (col > 0) --col; putc(' '); if (col > 0) --col; }
        } else {
            char c = keymap(scancode);
            if (c && length + 1 < sizeof(buffer)) { buffer[length++] = c; putc(c); }
        }
    }
}
