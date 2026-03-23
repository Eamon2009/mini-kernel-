/* mini-kernel linker script
   Targets i686 protected mode, loaded by GRUB at 1 MB (0x100000) */

ENTRY(kernel_start)         /* defined in boot/boot.asm */

SECTIONS
{
    . = 0x100000;           /* load address: 1 MB mark */

    /* ── Multiboot + boot code ─────────────────────────── */
    .text ALIGN(4K) :
    {
        *(.multiboot)       /* multiboot header MUST come first */
        *(.text)            /* all code sections */
    }

    /* ── Read-only data ────────────────────────────────── */
    .rodata ALIGN(4K) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* C++ global constructors */
    .init_array ALIGN(4K) :
    {
        __init_array_start = .;
        KEEP(*(.init_array))
        KEEP(*(.init_array.*))
        __init_array_end = .;
    }

    /* ── Initialised data ──────────────────────────────── */
    .data ALIGN(4K) :
    {
        *(.data)
    }

    /* ── Uninitialised data (zero-filled by boot) ──────── */
    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* ── Kernel end symbol (used by pmm.c) ─────────────── */
    kernel_end = .;

    /* Discard debugging / unused GNU sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.GNU-stack)
    }
}
