/* drivers/timer.h */
#ifndef TIMER_H
#define TIMER_H

#include "../kernel/kernel.h"

/* 8253/8254 PIT base frequency (Hz) — hardware constant */
#define PIT_BASE_HZ 1193182

/* PIT I/O ports */
#define PIT_CHANNEL0 0x40 /* channel 0 data (IRQ 0) */
#define PIT_CMD 0x43      /* mode/command register  */

void timer_init(uint32_t hz);   /* program PIT to fire at hz Hz */
uint32_t timer_get_ticks(void); /* monotonic tick counter */
void timer_sleep(uint32_t ms);  /* busy-sleep for ms milliseconds */

#endif