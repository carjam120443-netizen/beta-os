#ifndef BETA_EXEC_H
#define BETA_EXEC_H
#include <stddef.h>
#include <stdint.h>
#define EXEC_MAX_PROGRAMS 16
typedef struct { const char *name; uintptr_t entry; size_t size; } program_t;
void exec_init(void);
int exec_register(const char *name, uintptr_t entry, size_t size);
const program_t *exec_find(const char *name);
int exec_validate_elf64(const uint8_t *image, size_t size);
int exec_load_elf64(const uint8_t *image, size_t size, uint8_t *dest, size_t dest_size, uintptr_t *entry);
#endif
