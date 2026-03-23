/* kernel/kernel.h
 *  Shared primitive types, macros, and structs used
 * across every subsystem. Include this everywhere. */

#ifndef KERNEL_H
#define KERNEL_H

#ifdef __cplusplus
extern "C" {
#endif

/* ── Primitive types (no stdint.h in freestanding env) ──── */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef uint32_t size_t;
typedef int32_t ssize_t;
typedef uint32_t uintptr_t;

#define NULL ((void *)0)
#define TRUE 1
#define FALSE 0

/* ── Utility macros ─────────────────────────────────────── */
#define UNUSED(x) ((void)(x))
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define ALIGN_UP(v, a) (((v) + (a) - 1) & ~((a) - 1))
#define ALIGN_DN(v, a) ((v) & ~((a) - 1))
#define BIT(n) (1u << (n))

/* ── Compiler hints ─────────────────────────────────────── */
#define PACKED __attribute__((packed))
#define NORETURN __attribute__((noreturn))
#define INLINE __attribute__((always_inline)) static inline

/* ── Multiboot magic ────────────────────────────────────── */
#define MULTIBOOT_MAGIC 0x2BADB002u

/* ── Multiboot info struct (GRUB fills this in) ─────────── */
typedef struct
{
       uint32_t flags;
       uint32_t mem_lower; /* KB below 1 MB */
       uint32_t mem_upper; /* KB above 1 MB */
       uint32_t boot_device;
       uint32_t cmdline;
       uint32_t mods_count;
       uint32_t mods_addr;
       uint32_t syms[4];
       uint32_t mmap_length; /* byte length of memory map */
       uint32_t mmap_addr;   /* physical address of mmap entries */
       uint32_t drives_length;
       uint32_t drives_addr;
       uint32_t config_table;
       uint32_t boot_loader_name;
       uint32_t apm_table;
       uint32_t vbe_control_info;
       uint32_t vbe_mode_info;
       uint16_t vbe_mode;
       uint16_t vbe_interface_seg;
       uint16_t vbe_interface_off;
       uint16_t vbe_interface_len;
} PACKED multiboot_info_t;

/* ── Multiboot memory map entry ─────────────────────────── */
typedef struct
{
       uint32_t size;      /* size of this entry (not including this field) */
       uint64_t base_addr; /* start of memory region */
       uint64_t length;    /* byte length of region */
       uint32_t type;      /* 1 = available RAM, anything else = reserved */
} PACKED mmap_entry_t;

/* ── CPU register frame (matches kernel_entry.asm push order) */
typedef struct
{
       uint32_t ds;
       uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pusha */
       uint32_t int_no, err_code;                       /* pushed by stub */
       uint32_t eip, cs, eflags, useresp, ss;           /* pushed by CPU */
} PACKED registers_t;

/* ── Port I/O inlines ───────────────────────────────────── */
INLINE uint8_t inb(uint16_t port)
{
       uint8_t val;
       __asm__ __volatile__("inb %1, %0" : "=a"(val) : "Nd"(port));
       return val;
}
INLINE void outb(uint16_t port, uint8_t val)
{
       __asm__ __volatile__("outb %0, %1" ::"a"(val), "Nd"(port));
}
INLINE uint16_t inw(uint16_t port)
{
       uint16_t val;
       __asm__ __volatile__("inw %1, %0" : "=a"(val) : "Nd"(port));
       return val;
}
INLINE void outw(uint16_t port, uint16_t val)
{
       __asm__ __volatile__("outw %0, %1" ::"a"(val), "Nd"(port));
}
INLINE void io_wait(void)
{
       outb(0x80, 0); /* write to unused POST port — ~1–4 µs delay */
}

/* ── kernel/panic.c ─────────────────────────────────────── */
NORETURN void panic(const char *msg);

#ifdef __cplusplus
}
#endif

#endif /* KERNEL_H */
