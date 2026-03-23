/* mm/pmm.h — Physical Memory Manager */
#ifndef PMM_H
#define PMM_H
#include "../kernel/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PMM_FRAME_SIZE 4096 /* 4 KB frames */

void pmm_init(multiboot_info_t *mbi);
uint32_t pmm_alloc_frame(void); /* returns physical address or 0 */
uint32_t pmm_alloc_frame_below(uint32_t addr_limit); /* addr < addr_limit */
void pmm_free_frame(uint32_t addr);
uint32_t pmm_free_count(void);

#ifdef __cplusplus
}
#endif

#endif
