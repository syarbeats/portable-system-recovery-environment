#ifndef PSRE_HEAP_H
#define PSRE_HEAP_H
#include <stddef.h>
#include <stdbool.h>
bool heap_initialize(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
size_t heap_get_capacity(void);
size_t heap_get_free_bytes(void);
#endif
