
/* kernel/panic.c
 * Called when the kernel encounters an unrecoverable error.
 * Prints a message, dumps the register frame if available,
 * disables interrupts and halts forever. */

#include "kernel.h"
#include "../lib/kprintf.h"
#include "../drivers/vga.h"

/* Last register frame captured by an ISR — set by isr.c */
static registers_t *panic_regs = NULL;

void panic_set_regs(registers_t *r)
{
       panic_regs = r;
}

/* ── panic() ────────────────────────────────────────────────
 * NORETURN — declared in kernel.h.
 * Safe to call from anywhere, even before kprintf is ready
 * (vga_puts is used as a fallback). */
NORETURN void panic(const char *msg)
{
       /* Kill interrupts immediately — no more preemption */
       asm volatile("cli");

       /* Paint top two rows red so it's unmissable */
       vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
       vga_clear();

       kprintf("\n  *** KERNEL PANIC ***\n");
       kprintf("  %s\n\n", msg);

       /* Dump saved register frame if an ISR triggered the panic */
       if (panic_regs)
       {
              kprintf("  -- register dump --\n");
              kprintf("  eax=%08x  ebx=%08x  ecx=%08x  edx=%08x\n",
                      panic_regs->eax, panic_regs->ebx,
                      panic_regs->ecx, panic_regs->edx);
              kprintf("  esi=%08x  edi=%08x  ebp=%08x  esp=%08x\n",
                      panic_regs->esi, panic_regs->edi,
                      panic_regs->ebp, panic_regs->esp);
              kprintf("  eip=%08x  cs= %08x  eflags=%08x\n",
                      panic_regs->eip, panic_regs->cs,
                      panic_regs->eflags);
              kprintf("  int=%u  err=%08x\n",
                      panic_regs->int_no, panic_regs->err_code);
       }

       kprintf("\n  System halted. Restart your machine.\n");

       /* Halt forever — NMI-proof loop */
       for (;;)
       {
              asm volatile("cli; hlt");
       }
}