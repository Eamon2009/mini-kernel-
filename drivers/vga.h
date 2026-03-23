/* drivers/vga.h */
#ifndef VGA_H
#define VGA_H

#include "../kernel/kernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/* VGA text mode constants */
#define VGA_COLS 80
#define VGA_ROWS 25
#define VGA_BUFFER ((volatile uint16_t *)0xB8000)

/* Standard 4-bit CGA colours (foreground or background) */
typedef enum
{
       VGA_COLOR_BLACK = 0,
       VGA_COLOR_BLUE = 1,
       VGA_COLOR_GREEN = 2,
       VGA_COLOR_CYAN = 3,
       VGA_COLOR_RED = 4,
       VGA_COLOR_MAGENTA = 5,
       VGA_COLOR_BROWN = 6,
       VGA_COLOR_LIGHT_GREY = 7,
       VGA_COLOR_DARK_GREY = 8,
       VGA_COLOR_LIGHT_BLUE = 9,
       VGA_COLOR_LIGHT_GREEN = 10,
       VGA_COLOR_LIGHT_CYAN = 11,
       VGA_COLOR_LIGHT_RED = 12,
       VGA_COLOR_LIGHT_MAGENTA = 13,
       VGA_COLOR_LIGHT_BROWN = 14,
       VGA_COLOR_WHITE = 15,
} vga_color_t;

void vga_init(void);
void vga_clear(void);
void vga_set_color(vga_color_t fg, vga_color_t bg);
void vga_putchar(char c);
void vga_puts(const char *s);

#ifdef __cplusplus
}
#endif

#endif
