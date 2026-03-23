/* cpu/idt.c — Interrupt Descriptor Table
 * 256 gates. isr.c fills 0–31, irq.c fills 32–47.
 * All unused gates left zeroed (not-present). */

#include "idt.h"

#define IDT_ENTRIES 256

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

extern void idt_flush(uint32_t); /* lidt stub in asm */

void idt_set_gate(uint8_t num, uint32_t handler,
                  uint16_t sel, uint8_t flags)
{
       idt[num].offset_low = handler & 0xFFFF;
       idt[num].offset_high = (handler >> 16) & 0xFFFF;
       idt[num].selector = sel;
       idt[num].zero = 0;
       idt[num].type_attr = flags | 0x60;
}

void idt_init(void)
{
       idt_ptr.limit = sizeof(idt) - 1;
       idt_ptr.base = (uint32_t)&idt;
       /* zero all gates — marks them not-present */
       for (int i = 0; i < IDT_ENTRIES; i++)
       {
              idt_set_gate(i, 0, 0x08, 0x00);
       }
       idt_flush((uint32_t)&idt_ptr);
}