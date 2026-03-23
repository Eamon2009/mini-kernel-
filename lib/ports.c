/* lib/ports.c */
#include "ports.h"

uint8_t port_inb(uint16_t port)
{
       uint8_t val;
       __asm__ __volatile__("inb %1, %0" : "=a"(val) : "Nd"(port));
       return val;
}
void port_outb(uint16_t port, uint8_t val)
{
       __asm__ __volatile__("outb %0, %1" ::"a"(val), "Nd"(port));
}
uint16_t port_inw(uint16_t port)
{
       uint16_t val;
       __asm__ __volatile__("inw %1, %0" : "=a"(val) : "Nd"(port));
       return val;
}
void port_outw(uint16_t port, uint16_t val)
{
       __asm__ __volatile__("outw %0, %1" ::"a"(val), "Nd"(port));
}
