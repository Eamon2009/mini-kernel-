/* cpu/irq.h */
#ifndef IRQ_H
#define IRQ_H
#include "../kernel/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*irq_handler_t)(registers_t *);

void irq_init(void);
void irq_register_handler(uint8_t irq, irq_handler_t h);
void irq_handler(registers_t *r); /* called from kernel_entry.asm */

#ifdef __cplusplus
}
#endif
#endif
