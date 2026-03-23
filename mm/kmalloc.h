/* mm/kmalloc.h */
#ifndef KMALLOC_H
#define KMALLOC_H
#include "../kernel/kernel.h"

void kmalloc_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);
#endif