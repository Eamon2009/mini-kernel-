/* drivers/timer.c
 * Programs the 8253/8254 Programmable Interval Timer (PIT)
 * channel 0 to fire IRQ 0 at a configurable rate.
 * Maintains a monotonic tick counter for timekeeping. */

#include "timer.h"
#include "../cpu/irq.h"
#include "../lib/kprintf.h"

/* ── State ──────────────────────────────────────────────── */
static volatile uint32_t ticks = 0; /* incremented each IRQ 0 */
static uint32_t tick_hz = 0;        /* configured frequency     */

/* ── IRQ 0 handler ──────────────────────────────────────── */
static void timer_irq_handler(registers_t *r)
{
       UNUSED(r);
       ticks++;
}

/* ── timer_init ─────────────────────────────────────────── */
void timer_init(uint32_t hz)
{
       tick_hz = hz;

       /* Divisor: PIT fires at PIT_BASE_HZ / divisor Hz */
       uint32_t divisor = PIT_BASE_HZ / hz;

       /* Command: channel 0, lobyte/hibyte access, rate generator */
       outb(PIT_CMD, 0x36);

       /* Send divisor low byte then high byte */
       outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
       outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

       irq_register_handler(0, timer_irq_handler);
       kprintf("[TIMER] PIT set to %u Hz (divisor %u)\n", hz, divisor);
}

/* ── timer_get_ticks ────────────────────────────────────── */
uint32_t timer_get_ticks(void)
{
       return ticks;
}

/* ── timer_sleep ────────────────────────────────────────── */
void timer_sleep(uint32_t ms)
{
       /* Convert ms → ticks: ticks_per_ms = tick_hz / 1000 */
       uint32_t target = ticks + (ms * tick_hz) / 1000;
       while (ticks < target)
              asm volatile("hlt"); /* sleep between ticks */
}