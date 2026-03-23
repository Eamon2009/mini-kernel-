/* lib/kprintf.h */
#ifndef KPRINTF_H
#define KPRINTF_H
#include "../kernel/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

void kprintf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
