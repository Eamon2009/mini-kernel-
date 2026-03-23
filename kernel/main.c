/* kernel/kernel_main.c
 * First C function called by boot.asm.
 * Initialises every subsystem in dependency order,
 * then drops into the idle loop. */

#include "kernel.h"

/* drivers */
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"

/* cpu */
#include "../cpu/gdt.h"
#include "../cpu/idt.h"
#include "../cpu/isr.h"
#include "../cpu/irq.h"

/* memory */
#include "../mm/pmm.h"
#include "../mm/paging.h"
#include "../mm/kmalloc.h"

/* lib */
#include "../lib/kprintf.h"

/* Multiboot info struct (abridged — full def in kernel.h) */
void kernel_main(uint32_t magic, multiboot_info_t *mbi)
{
       /* ── 1. VGA text mode console (no dependencies) ──────── */
       vga_init();
       kprintf("mini-kernel booting...\n");

       /* ── 2. Verify GRUB handshake ────────────────────────── */
       if (magic != MULTIBOOT_MAGIC)
       {
              kprintf("[FATAL] Bad multiboot magic: 0x%x\n", magic);
              panic("Invalid bootloader");
       }
       kprintf("[OK] Multiboot magic verified\n");

       /* ── 3. CPU tables ───────────────────────────────────── */
       gdt_init();
       kprintf("[OK] GDT loaded\n");

       idt_init();
       kprintf("[OK] IDT loaded\n");

       isr_init(); /* install exception handlers 0–31 */
       kprintf("[OK] ISRs installed\n");

       irq_init(); /* remap PIC, install IRQ handlers 0–15 */
       kprintf("[OK] IRQs remapped and installed\n");

       /* ── 4. Memory management ────────────────────────────── */
       pmm_init(mbi); /* parse GRUB memory map, build free-frame bitmap */
       kprintf("[OK] PMM initialised\n");

       paging_init(); /* identity-map kernel, enable CR0.PG */
       kprintf("[OK] Paging enabled\n");

       kmalloc_init(); /* set up heap after kernel_end */
       kprintf("[OK] Heap ready\n");

       /* ── 5. Hardware drivers ─────────────────────────────── */
       timer_init(100); /* PIT at 100 Hz — must come before sti() */
       kprintf("[OK] Timer at 100 Hz\n");

       keyboard_init();
       kprintf("[OK] Keyboard driver ready\n");

       /* ── 6. Enable interrupts ────────────────────────────── */
       asm volatile("sti");
       kprintf("[OK] Interrupts enabled\n");

       kprintf("\nmini-kernel ready.\n\n");

       /* ── Idle loop ───────────────────────────────────────── */
       for (;;)
       {
              asm volatile("hlt"); /* sleep until next interrupt */
       }
}