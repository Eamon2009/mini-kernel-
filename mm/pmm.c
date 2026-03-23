/* mm/pmm.c
 * Bitmap-based physical frame allocator.
 * One bit per 4 KB frame: 0 = free, 1 = used.
 * The bitmap itself lives just above kernel_end. */

#include "pmm.h"
#include "../lib/kprintf.h"
#include "../lib/string.h"

extern uint32_t kernel_end; /* symbol from linker.ld */

#define MAX_FRAMES (1024 * 256) /* support up to 1 GB RAM */
#define BMP_WORDS (MAX_FRAMES / 32)

static uint32_t bitmap[BMP_WORDS];
static uint32_t total_frames = 0;
static uint32_t free_frames = 0;

static void bmp_set(uint32_t f) { bitmap[f / 32] |= BIT(f % 32); }
static void bmp_clear(uint32_t f) { bitmap[f / 32] &= ~BIT(f % 32); }
static int bmp_test(uint32_t f) { return !!(bitmap[f / 32] & BIT(f % 32)); }

void pmm_init(multiboot_info_t *mbi)
{
       /* Mark everything used by default */
       memset(bitmap, 0xFF, sizeof(bitmap));

       mmap_entry_t *entry = (mmap_entry_t *)mbi->mmap_addr;
       mmap_entry_t *end = (mmap_entry_t *)(mbi->mmap_addr + mbi->mmap_length);

       while (entry < end)
       {
              if (entry->type == 1)
              { /* available RAM */
                     uint32_t base = (uint32_t)entry->base_addr;
                     uint32_t frames = (uint32_t)(entry->length / PMM_FRAME_SIZE);
                     for (uint32_t i = 0; i < frames; i++)
                     {
                            uint32_t frame = (base / PMM_FRAME_SIZE) + i;
                            if (frame < MAX_FRAMES)
                            {
                                   bmp_clear(frame);
                                   free_frames++;
                                   total_frames++;
                            }
                     }
              }
              entry = (mmap_entry_t *)((uint32_t)entry + entry->size + 4);
       }

       /* Re-mark frames occupied by kernel + bitmap as used */
       uint32_t used_end = ALIGN_UP((uint32_t)&kernel_end + sizeof(bitmap),
                                    PMM_FRAME_SIZE);
       for (uint32_t addr = 0; addr < used_end; addr += PMM_FRAME_SIZE)
       {
              uint32_t f = addr / PMM_FRAME_SIZE;
              if (!bmp_test(f))
              {
                     bmp_set(f);
                     free_frames--;
              }
       }

       kprintf("[PMM] %u MB RAM, %u frames free\n",
               (total_frames * PMM_FRAME_SIZE) >> 20, free_frames);
}

uint32_t pmm_alloc_frame(void)
{
       for (uint32_t w = 0; w < BMP_WORDS; w++)
       {
              if (bitmap[w] == 0xFFFFFFFF)
                     continue;
              for (int b = 0; b < 32; b++)
              {
                     uint32_t f = w * 32 + b;
                     if (!bmp_test(f))
                     {
                            bmp_set(f);
                            free_frames--;
                            return f * PMM_FRAME_SIZE;
                     }
              }
       }
       return 0; /* out of memory */
}

void pmm_free_frame(uint32_t addr)
{
       uint32_t f = addr / PMM_FRAME_SIZE;
       if (bmp_test(f))
       {
              bmp_clear(f);
              free_frames++;
       }
}

uint32_t pmm_free_count(void) { return free_frames; }