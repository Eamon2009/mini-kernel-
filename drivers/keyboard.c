/* drivers/keyboard.c
 * PS/2 keyboard driver using IRQ 1 (vector 33).
 * Translates Set-1 scan codes to ASCII via a lookup table
 * and buffers them in a small ring buffer. */

#include "keyboard.h"
#include "vga.h"
#include "../cpu/irq.h"
#include "../lib/kprintf.h"

/* ── Scan code → ASCII lookup (Set 1, unshifted) ───────── */
static const char sc_ascii[] = {
    /*00*/ 0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    /*0F*/ '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    /*1D*/ 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    /*2A*/ 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    /*37*/ '*', 0, ' '};

static const char sc_ascii_shift[] = {
    /*00*/ 0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    /*0F*/ '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    /*1D*/ 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    /*2A*/ 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    /*37*/ '*', 0, ' '};

/* ── Ring buffer ────────────────────────────────────────── */
#define KB_BUF_SIZE 64
static char kb_buf[KB_BUF_SIZE];
static uint8_t kb_head = 0; /* read index  */
static uint8_t kb_tail = 0; /* write index */

static void buf_push(char c)
{
       uint8_t next = (kb_tail + 1) % KB_BUF_SIZE;
       if (next != kb_head) /* drop if full */
              kb_buf[kb_tail = next - 1, kb_tail++] = c;
       kb_tail = next;
}

static int buf_pop(char *out)
{
       if (kb_head == kb_tail)
              return 0;
       *out = kb_buf[kb_head];
       kb_head = (kb_head + 1) % KB_BUF_SIZE;
       return 1;
}

/* ── Modifier state ─────────────────────────────────────── */
static uint8_t shift_held = 0;
static uint8_t caps_lock = 0;

/* ── IRQ 1 handler ──────────────────────────────────────── */
static void keyboard_irq_handler(registers_t *r)
{
       UNUSED(r);
       uint8_t sc = inb(KB_DATA_PORT);

       uint8_t released = sc & KB_RELEASED;
       uint8_t code = sc & ~KB_RELEASED; /* strip release bit */

       /* Track modifier keys */
       if (code == 0x2A || code == 0x36)
       { /* L-Shift / R-Shift */
              shift_held = !released;
              return;
       }
       if (code == 0x3A && !released)
       { /* Caps Lock toggle on press */
              caps_lock ^= 1;
              return;
       }

       if (released)
              return; /* ignore key-up for printable keys */

       /* Translate scan code → ASCII */
       if (code >= ARRAY_SIZE(sc_ascii))
              return;

       char c = shift_held ? sc_ascii_shift[code] : sc_ascii[code];
       if (!c)
              return;

       /* Apply Caps Lock to letters only */
       if (caps_lock)
       {
              if (c >= 'a' && c <= 'z')
                     c -= 32;
              else if (c >= 'A' && c <= 'Z')
                     c += 32;
       }

       buf_push(c);
}

/* ── Public API ─────────────────────────────────────────── */
void keyboard_init(void)
{
       irq_register_handler(1, keyboard_irq_handler);
       kprintf("[KB] PS/2 keyboard driver ready\n");
}

int keyboard_haschar(void)
{
       return kb_head != kb_tail;
}

char keyboard_getchar(void)
{
       char c;
       while (!buf_pop(&c))
              asm volatile("hlt"); /* sleep until IRQ 1 fills the buffer */
       return c;
}