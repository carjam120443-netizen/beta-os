#include "fs.h"

#define ATA_DATA 0x1F0
#define ATA_SECTOR_COUNT 0x1F2
#define ATA_LBA0 0x1F3
#define ATA_LBA1 0x1F4
#define ATA_LBA2 0x1F5
#define ATA_DRIVE 0x1F6
#define ATA_STATUS 0x1F7
#define ATA_COMMAND 0x1F7
#define ATA_CMD_READ 0x20
#define ATA_CMD_WRITE 0x30
#define ATA_CMD_FLUSH 0xE7
#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08

static fs_file_t files[FS_MAX_FILES];
static int persistent;

static inline void outb(uint16_t p, uint8_t v) {
    __asm__ volatile("outb %0,%1" :: "a"(v), "Nd"(p));
}

static inline void outw(uint16_t p, uint16_t v) {
    __asm__ volatile("outw %0,%1" :: "a"(v), "Nd"(p));
}

static inline uint8_t inb(uint16_t p) {
    uint8_t v;
    __asm__ volatile("inb %1,%0" : "=a"(v) : "Nd"(p));
    return v;
}

static inline uint16_t inw(uint16_t p) {
    uint16_t v;
    __asm__ volatile("inw %1,%0" : "=a"(v) : "Nd"(p));
    return v;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

static size_t str_len(const char *s) {
    size_t n = 0;
    while (s[n]) {
        ++n;
    }
    return n;
}

static int str_eq(const char *a, const char *b) {
    size_t i = 0;
    while (a[i] && b[i] && a[i] == b[i]) {
        ++i;
    }
    return a[i] == b[i];
}

static int ata_wait(uint8_t mask, uint8_t value, uint32_t timeout) {
    while (timeout--) {
        uint8_t s = inb(ATA_STATUS);
        if (s == 0 || s == 0xFF) {
            return -1;
        }
        if ((s & mask) == value) {
            return 0;
        }
        io_wait();
    }
    return -1;
}

static int ata_rw(uint32_t lba, uint8_t count, void *buffer, int write) {
    if (!count || ata_wait(ATA_SR_BSY, 0, 100000)) {
        return -1;
    }

    outb(ATA_DRIVE, (uint8_t)(0xE0 | ((lba >> 24) & 0x0F)));
    outb(ATA_SECTOR_COUNT, count);
    outb(ATA_LBA0, (uint8_t)lba);
    outb(ATA_LBA1, (uint8_t)(lba >> 8));
    outb(ATA_LBA2, (uint8_t)(lba >> 16));
    outb(ATA_COMMAND, write ? ATA_CMD_WRITE : ATA_CMD_READ);

    uint16_t *w = (uint16_t *)buffer;
    for (uint32_t s = 0; s < count; ++s) {
        if (ata_wait(ATA_SR_BSY | ATA_SR_DRQ, ATA_SR_DRQ, 100000)) {
            return -1;
        }
        for (uint32_t i = 0; i < 256; ++i) {
            if (write) {
                outw(ATA_DATA, w[s * 256 + i]);
            } else {
                w[s * 256 + i] = inw(ATA_DATA);
            }
        }
    }

    if (write) {
        outb(ATA_COMMAND, ATA_CMD_FLUSH);
        if (ata_wait(ATA_SR_BSY, 0, 100000)) {
            return -1;
        }
    }
    return 0;
}

static int ata_read(uint32_t lba, uint8_t n, void *b) {
    return ata_rw(lba, n, b, 0);
}

static int ata_write(uint32_t lba, uint8_t n, const void *b) {
    return ata_rw(lba, n, (void *)b, 1);
}

typedef struct {
    uint32_t magic, version, files, reserved;
} fs_disk_header_t;

typedef struct {
    char name[FS_NAME_MAX];
    uint32_t size, used;
} fs_disk_entry_t;

static int disk_load(void) {
    uint8_t sector[FS_SECTOR_SIZE] = {0};
    fs_disk_header_t *h = (fs_disk_header_t *)sector;

    if (ata_read(FS_DISK_LBA, 1, sector)) {
        return -1;
    }
    if (h->magic != FS_DISK_MAGIC || h->version != 1) {
        return -1;
    }

    uint8_t meta[FS_SECTOR_SIZE * 5] = {0};
    if (ata_read(FS_DISK_LBA + 1, 5, meta)) {
        return -1;
    }

    fs_disk_entry_t *e = (fs_disk_entry_t *)meta;
    for (size_t i = 0; i < FS_MAX_FILES; ++i) {
        files[i].used = e[i].used ? 1 : 0;
        files[i].size = e[i].size <= FS_DATA_MAX ? e[i].size : 0;
        for (size_t j = 0; j < FS_NAME_MAX; ++j) {
            files[i].name[j] = e[i].name[j];
        }
        if (files[i].used && ata_read(FS_DISK_LBA + 6 + (uint32_t)i * 8, 8, files[i].data)) {
            return -1;
        }
    }
    return 0;
}

static int disk_format(void) {
    uint8_t sector[FS_SECTOR_SIZE] = {0};
    fs_disk_header_t *h = (fs_disk_header_t *)sector;
    h->magic = FS_DISK_MAGIC;
    h->version = 1;
    h->files = FS_MAX_FILES;

    if (ata_write(FS_DISK_LBA, 1, sector)) {
        return -1;
    }

    uint8_t meta[FS_SECTOR_SIZE * 5] = {0};
    return ata_write(FS_DISK_LBA + 1, 5, meta);
}

int fs_sync(void) {
    if (!persistent) {
        return -1;
    }

    uint8_t meta[FS_SECTOR_SIZE * 5] = {0};
    fs_disk_entry_t *e = (fs_disk_entry_t *)meta;
    for (size_t i = 0; i < FS_MAX_FILES; ++i) {
        e[i].used = (uint32_t)files[i].used;
        e[i].size = (uint32_t)files[i].size;
        for (size_t j = 0; j < FS_NAME_MAX; ++j) {
            e[i].name[j] = files[i].name[j];
        }
        if (files[i].used && ata_write(FS_DISK_LBA + 6 + (uint32_t)i * 8, 8, files[i].data)) {
            return -1;
        }
    }
    return ata_write(FS_DISK_LBA + 1, 5, meta);
}

void fs_init(void) {
    persistent = 0;
    for (size_t i = 0; i < FS_MAX_FILES; ++i) {
        files[i].used = 0;
        files[i].size = 0;
    }
    if (disk_load() == 0 || disk_format() == 0) {
        persistent = 1;
    }
}

int fs_is_persistent(void) {
    return persistent;
}

int fs_create(const char *name, const void *data, size_t size) {
    if (!name || !data || size > FS_DATA_MAX || !str_len(name) || str_len(name) >= FS_NAME_MAX) {
        return -1;
    }
    if (fs_find(name)) {
        return -2;
    }

    for (size_t i = 0; i < FS_MAX_FILES; ++i) {
        if (!files[i].used) {
            size_t n = str_len(name);
            for (size_t j = 0; j <= n; ++j) {
                files[i].name[j] = name[j];
            }
            const uint8_t *s = (const uint8_t *)data;
            for (size_t j = 0; j < size; ++j) {
                files[i].data[j] = s[j];
            }
            for (size_t j = size; j < FS_DATA_MAX; ++j) {
                files[i].data[j] = 0;
            }
            files[i].size = size;
            files[i].used = 1;
            return !persistent || fs_sync() == 0 ? 0 : -3;
        }
    }
    return -1;
}

int fs_write(const char *name, const void *data, size_t size) {
    if (!name || !data || size > FS_DATA_MAX) {
        return -1;
    }

    fs_file_t *f = (fs_file_t *)fs_find(name);
    if (!f) {
        return fs_create(name, data, size);
    }

    const uint8_t *s = (const uint8_t *)data;
    for (size_t i = 0; i < size; ++i) {
        f->data[i] = s[i];
    }
    for (size_t i = size; i < FS_DATA_MAX; ++i) {
        f->data[i] = 0;
    }
    f->size = size;
    return !persistent || fs_sync() == 0 ? 0 : -3;
}

int fs_remove(const char *name) {
    fs_file_t *f = (fs_file_t *)fs_find(name);
    if (!f) {
        return -1;
    }
    f->used = 0;
    f->size = 0;
    return !persistent || fs_sync() == 0 ? 0 : -2;
}

const fs_file_t *fs_find(const char *name) {
    if (!name) {
        return NULL;
    }
    for (size_t i = 0; i < FS_MAX_FILES; ++i) {
        if (files[i].used && str_eq(files[i].name, name)) {
            return &files[i];
        }
    }
    return NULL;
}

size_t fs_read(const char *name, void *out, size_t max_size) {
    const fs_file_t *f = fs_find(name);
    if (!f || !out) {
        return 0;
    }
    size_t n = f->size < max_size ? f->size : max_size;
    uint8_t *d = (uint8_t *)out;
    for (size_t i = 0; i < n; ++i) {
        d[i] = f->data[i];
    }
    return n;
}

size_t fs_list(char *out, size_t max_size) {
    if (!out || !max_size) {
        return 0;
    }

    size_t p = 0;
    for (size_t i = 0; i < FS_MAX_FILES; ++i) {
        if (files[i].used) {
            for (size_t j = 0; files[i].name[j] && p + 1 < max_size; ++j) {
                out[p++] = files[i].name[j];
            }
            if (p + 1 < max_size) {
                out[p++] = '\n';
            }
        }
    }
    out[p < max_size ? p : max_size - 1] = 0;
    return p;
}
