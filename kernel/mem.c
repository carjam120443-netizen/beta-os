#include "mem.h"
#include <stdint.h>

#define HEAP_SIZE (256u * 1024u)
static uint8_t heap[HEAP_SIZE];
static size_t offset;

void mem_init(void) { offset = 0; }

void *kmalloc(size_t size) {
    if (!size) return NULL;
    size = (size + 15u) & ~(size_t)15u;
    if (offset > HEAP_SIZE - size) return NULL;
    void *p = &heap[offset];
    offset += size;
    return p;
}

void kfree(void *ptr) { (void)ptr; }
size_t mem_used(void) { return offset; }
size_t mem_capacity(void) { return HEAP_SIZE; }
