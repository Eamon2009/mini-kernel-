/* lib/ports.h
 * Non-inline port I/O for drivers that can't include kernel.h
 * directly, or need a .c-linkage version for linking. */
#ifndef PORTS_H
#define PORTS_H
#include "../kernel/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/* These are thin wrappers — the real inlines live in kernel.h.
 * Provided here so object files that only include ports.h
 * still get working port I/O without pulling in all of kernel.h. */
uint8_t port_inb(uint16_t port);
void port_outb(uint16_t port, uint8_t val);
uint16_t port_inw(uint16_t port);
void port_outw(uint16_t port, uint16_t val);

#ifdef __cplusplus
}
#endif

#endif
