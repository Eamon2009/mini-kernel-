/* mm/kmalloc.h */
#ifndef KMALLOC_H
#define KMALLOC_H
#include "../kernel/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

void kmalloc_init(void);
void *kmalloc(size_t size);
void kfree(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
