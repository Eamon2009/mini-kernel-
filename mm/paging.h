/* mm/paging.h */
#ifndef PAGING_H
#define PAGING_H
#include "../kernel/kernel.h"

#define PAGE_PRESENT BIT(0)
#define PAGE_WRITE BIT(1)
#define PAGE_USER BIT(2)
#define PAGE_SIZE 4096

void paging_init(void);
void paging_map(uint32_t virt, uint32_t phys, uint32_t flags);
uint32_t paging_virt_to_phys(uint32_t virt);
#endif