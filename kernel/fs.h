#ifndef BETA_FS_H
#define BETA_FS_H

#include <stdint.h>
#include <stddef.h>

#define FS_MAX_FILES 32
#define FS_NAME_MAX 64
#define FS_DATA_MAX 4096

typedef struct {
    char name[FS_NAME_MAX];
    uint8_t data[FS_DATA_MAX];
    size_t size;
    int used;
} fs_file_t;

void fs_init(void);
int fs_create(const char *name, const void *data, size_t size);
const fs_file_t *fs_find(const char *name);
size_t fs_read(const char *name, void *out, size_t max_size);

#endif
