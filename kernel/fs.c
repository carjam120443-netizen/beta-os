#include "fs.h"

static fs_file_t files[FS_MAX_FILES];

static size_t str_len(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') ++n;
    return n;
}

static int str_eq(const char *a, const char *b) {
    size_t i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) return 0;
        ++i;
    }
    return a[i] == b[i];
}

void fs_init(void) {
    for (size_t i = 0; i < FS_MAX_FILES; ++i) files[i].used = 0;
}

int fs_create(const char *name, const void *data, size_t size) {
    if (!name || !data || size > FS_DATA_MAX || str_len(name) >= FS_NAME_MAX) return -1;
    for (size_t i = 0; i < FS_MAX_FILES; ++i) {
        if (!files[i].used) {
            size_t n = str_len(name);
            for (size_t j = 0; j <= n; ++j) files[i].name[j] = name[j];
            const uint8_t *src = (const uint8_t *)data;
            for (size_t j = 0; j < size; ++j) files[i].data[j] = src[j];
            files[i].size = size;
            files[i].used = 1;
            return 0;
        }
    }
    return -1;
}

const fs_file_t *fs_find(const char *name) {
    for (size_t i = 0; i < FS_MAX_FILES; ++i) {
        if (files[i].used && str_eq(files[i].name, name)) return &files[i];
    }
    return NULL;
}

size_t fs_read(const char *name, void *out, size_t max_size) {
    const fs_file_t *file = fs_find(name);
    if (!file || !out) return 0;
    size_t n = file->size < max_size ? file->size : max_size;
    uint8_t *dst = (uint8_t *)out;
    for (size_t i = 0; i < n; ++i) dst[i] = file->data[i];
    return n;
}
