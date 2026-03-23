#ifndef ISR_H
#define ISR_H

#include "../kernel/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*isr_t)(registers_t *);

void isr_init(void);
void isr_register_handler(uint8_t num, isr_t handler);
void register_interrupt_handler(uint8_t n, isr_t handler);

#ifdef __cplusplus
}
#endif

#endif
