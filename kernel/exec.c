#include "exec.h"

static program_t programs[EXEC_MAX_PROGRAMS];
static size_t program_count;

void exec_init(void) {
    program_count = 0;
}

int exec_register(const char *name, uintptr_t entry, size_t size) {
    if (!name || program_count >= EXEC_MAX_PROGRAMS) return -1;
    programs[program_count].name = name;
    programs[program_count].entry = entry;
    programs[program_count].size = size;
    ++program_count;
    return 0;
}

const program_t *exec_find(const char *name) {
    for (size_t i = 0; i < program_count; ++i) {
        const char *a = programs[i].name;
        size_t j = 0;
        while (a[j] != '\0' && name[j] != '\0' && a[j] == name[j]) ++j;
        if (a[j] == '\0' && name[j] == '\0') return &programs[i];
    }
    return (const program_t *)0;
}

int exec_validate_elf64(const uint8_t *image, size_t size) {
    if (!image || size < 64) return 0;
    if (image[0] != 0x7f || image[1] != 'E' || image[2] != 'L' || image[3] != 'F') return 0;
    if (image[4] != 2 || image[5] != 1) return 0;
    if (image[18] != 0x3e || image[19] != 0x00) return 0;
    return 1;
}
