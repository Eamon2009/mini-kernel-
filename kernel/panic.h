#ifndef PANIC_H
#define PANIC_H

#include "kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

void panic_set_regs(registers_t *r);

#ifdef __cplusplus
}
#endif

#endif
