#ifndef BETA_MEM_H
#define BETA_MEM_H
#include <stddef.h>
void mem_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
size_t mem_used(void);
size_t mem_capacity(void);
#endif
