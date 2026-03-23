# mini-kernel

A minimal x86 operating system kernel written from scratch in C and NASM assembly. Built without any standard library, operating system support, or external dependencies — just a cross-compiler, an assembler, and bare metal.

> Built step-by-step as a learning reference for low-level systems programming.

---

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Subsystems](#subsystems)
  - [Boot](#boot)
  - [CPU](#cpu)
  - [Drivers](#drivers)
  - [Memory Management](#memory-management)
  - [Standard Library](#standard-library)
- [Prerequisites](#prerequisites)
- [Building](#building)
- [Running](#running)
- [Debugging with GDB](#debugging-with-gdb)
- [Boot Sequence](#boot-sequence)
- [Memory Layout](#memory-layout)
- [What Happens at Runtime](#what-happens-at-runtime)
- [Extending the Kernel](#extending-the-kernel)
- [References](#references)

---

## Overview

mini-kernel boots via GRUB (Multiboot 1), enters 32-bit protected mode, and initialises a complete foundational kernel layer:

| Layer | What it does |
|---|---|
| **Boot** | GRUB multiboot header, stack setup, ISR/IRQ assembly stubs |
| **CPU** | GDT flat model, IDT with 256 gates, exception + hardware IRQ dispatch |
| **Drivers** | VGA text output, PS/2 keyboard (IRQ 1), PIT timer at 100 Hz (IRQ 0) |
| **Memory** | Bitmap physical frame allocator, identity-mapped paging, kernel heap |
| **Library** | `memset`/`memcpy`/`strcmp`, `kprintf` with `%c %s %d %u %x %p`, port I/O |

The kernel is entirely interrupt-driven after init. The idle loop uses `hlt` — the CPU sleeps until the next IRQ fires.

---

## Architecture

```
GRUB
  └─ boot/boot.asm          ← multiboot header, stack, calls kernel_main()
       └─ boot/kernel_entry.asm   ← 32+16 ISR/IRQ stubs + common trampolines
            └─ kernel/kernel_main.c   ← C entry, ordered init sequence
                 ├─ cpu/              ← GDT → IDT → ISRs → IRQs
                 ├─ drivers/          ← VGA → timer → keyboard
                 ├─ mm/               ← PMM → paging → heap
                 └─ lib/              ← string, kprintf, ports
```

Target: **i686** (32-bit x86), **Multiboot 1**, **ELF** binary, loaded at physical address `0x100000`.

---

## Project Structure

```
mykernel/
├── Makefile                   # Build rules and link order
├── linker.ld                  # Memory layout script
├── grub.cfg                   # GRUB bootloader config
│
├── boot/
│   ├── boot.asm               # Multiboot header, 16 KB stack, calls kernel_main
│   └── kernel_entry.asm       # ISR stubs 0–31, IRQ stubs 0–15, common trampolines
│
├── kernel/
│   ├── kernel_main.c          # C entry point, ordered subsystem initialisation
│   ├── kernel.h               # Primitive types, macros, port I/O inlines, structs
│   └── panic.c                # Kernel panic — red screen, register dump, halt
│
├── drivers/
│   ├── vga.c / vga.h          # VGA text mode — 80×25, scroll, hardware cursor
│   ├── keyboard.c / keyboard.h # PS/2 keyboard — IRQ 1, scan codes, ring buffer
│   └── timer.c / timer.h      # PIT 8253 — configurable Hz, tick counter, sleep
│
├── cpu/
│   ├── gdt.c / gdt.h          # Global Descriptor Table — flat 32-bit model
│   ├── idt.c / idt.h          # Interrupt Descriptor Table — 256 gates
│   ├── isr.c / isr.h          # CPU exception handlers (vectors 0–31)
│   └── irq.c / irq.h          # 8259 PIC remap, hardware IRQ dispatch (vectors 32–47)
│
├── mm/
│   ├── pmm.c / pmm.h          # Physical memory manager — bitmap, 4 KB frames
│   ├── paging.c / paging.h    # Page directory, identity map 0–4 MB, CR0.PG
│   └── kmalloc.c / kmalloc.h  # Kernel heap — bump pointer + free-list
│
└── lib/
    ├── string.c / string.h    # memset, memcpy, memcmp, strlen, strcpy, strcmp
    ├── kprintf.c / kprintf.h  # Kernel printf — %c %s %d %u %x %p %%
    └── ports.c / ports.h      # port_inb / port_outb wrappers (C linkage)
```

**27 source files + grub.cfg. Zero external libraries.**

---

## Subsystems

### Boot

**`boot/boot.asm`** — the first code the CPU runs after GRUB hands control over.

- Places the Multiboot 1 magic header (`0x1BADB002`) in its own `.multiboot` section so the linker guarantees it sits in the first 8 KB of the binary — GRUB will not boot the kernel otherwise.
- Allocates a 16 KB stack in `.bss` (no binary bloat — `resb` reserves space without emitting zeros).
- Sets `esp`, pushes `ebx` (multiboot info pointer) then `eax` (magic value) onto the stack as cdecl arguments, then calls `kernel_main(uint32_t magic, multiboot_info_t *mbi)`.
- Contains a `cli; hlt` loop after the call — if `kernel_main` ever returns, the machine halts safely.

**`boot/kernel_entry.asm`** — the assembly half of the interrupt system.

- Two NASM macros (`ISR_NOERR`, `ISR_ERR`) generate 32 individual ISR stubs. Exceptions that don't push an error code get a dummy `push 0` so the stack frame is always the same size.
- 16 IRQ stubs map hardware lines to vectors 32–47 (after PIC remapping).
- `isr_common_stub` and `irq_common_stub` save all registers with `pusha`, switch to kernel data segments, call the C handler with a pointer to the frame, then restore and `iret`.

---

### CPU

**`cpu/gdt.c`** — three descriptors: null, kernel code (`0x9A`, ring 0 executable), kernel data (`0x92`, ring 0 writable). All span the full 4 GB with granularity `0xCF` (4 KB pages, 32-bit operand size). Loaded via `gdt_flush()`, an external ASM stub that calls `lgdt` and performs a far jump to reload `CS` with the new code segment selector.

**`cpu/idt.c`** — 256 interrupt gates, all zeroed (not-present) on init. `idt_set_gate()` accepts a handler address, selector, and flags — it ORs the flags with `0x60` internally. ISR gates are installed with flags `0x8E` (present, ring 0, 32-bit interrupt gate — which clears `IF` on entry automatically).

**`kernel/panic.c`** — `panic()` is declared `NORETURN` in `kernel.h` and defined here. Calls `cli` immediately, paints the screen red via `vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED)` + `vga_clear()`, prints the message and a full register dump if `panic_set_regs()` was called by an ISR beforehand, then halts with a `cli; hlt` loop. `panic_set_regs(registers_t *r)` is called by `isr_handler` before invoking `panic()` so the dump is always populated on exceptions.

**`cpu/isr.c`** — installs the 32 ASM stubs into the IDT, maintains a C handler table, and dispatches to registered handlers. Unhandled exceptions call `panic_set_regs()` then `panic()` with the exception name.

**`cpu/irq.c`** — remaps the 8259 PIC so IRQ 0–15 land on vectors 32–47 (away from CPU exceptions), installs the 16 IRQ stubs, and sends an End-Of-Interrupt (`0x20`) to the PIC after every handler runs. Forgetting EOI permanently masks that IRQ line.

---

### Drivers

**`drivers/vga.c`** — writes directly to the memory-mapped VGA framebuffer at `0xB8000`. Each cell is 2 bytes: the low byte is ASCII, the high byte is the colour attribute (background in bits 6–4, foreground in bits 3–0). Handles `\n`, `\r`, `\t`, scroll, and moves the hardware cursor via ports `0x3D4`/`0x3D5`.

**`drivers/keyboard.c`** — reads PS/2 Set-1 scan codes from port `0x60` on IRQ 1. Translates codes to ASCII using two lookup tables (`sc_ascii` and `sc_ascii_shift`). Tracks Left/Right Shift (scan codes `0x2A`/`0x36`) and Caps Lock (`0x3A`) state. Buffers characters in a 64-byte ring buffer. `keyboard_getchar()` sleeps with `hlt` until a character is available; `keyboard_haschar()` is non-blocking.

**`drivers/timer.c`** — programs the 8253/8254 PIT channel 0 with `outb(PIT_CMD, 0x36)` followed by the 16-bit divisor (low byte then high byte). At 100 Hz the divisor is 11,931. Maintains a `volatile uint32_t ticks` counter incremented on every IRQ 0. `timer_sleep(ms)` converts milliseconds to ticks and halts until the target is reached.

---

### Memory Management

**`mm/pmm.c`** — bitmap allocator. One bit per 4 KB frame, stored in a static `uint32_t bitmap[BMP_WORDS]` array inside the kernel's `.bss`. Initialised all-ones (everything reserved), then the GRUB memory map (`mmap_addr` / `mmap_length`) marks available RAM free. All frames from `0x0` up to `kernel_end + sizeof(bitmap)` are re-marked used to protect the kernel image and bitmap. `pmm_alloc_frame()` scans 32 bits at a time and returns a physical address. Supports up to 1 GB RAM.

**`mm/paging.c`** — creates a page directory and one page table in BSS (4 KB aligned). Identity-maps the first 4 MB (virtual == physical), loads CR3, and sets `CR0.PG`. Registers a page fault handler (ISR 14) that prints the faulting address from `CR2`. `paging_map()` allocates page tables on demand from the PMM.

**`mm/kmalloc.c`** — heap base is `0x400000`, ceiling `0x800000`. Each allocation prepends a `block_t` header with a magic number (`0xC0FFEE00` = used, `0xDEADBEEF` = free). `kmalloc_init()` maps an initial 4 KB page and sets up the first free block. `kmalloc()` first scans the free list, then calls `paging_map()` + `pmm_alloc_frame()` to extend the heap by one aligned chunk. `kfree()` validates the magic before marking the block free — a bad pointer is caught and logged rather than silently corrupting the heap.

---

### Standard Library

**`lib/string.c`** — `memset`, `memcpy`, `memcmp`, `strlen`, `strcpy`, `strcmp`, `strchr`. Implemented without any libc dependency.

**`lib/kprintf.c`** — format string parser supporting `%c`, `%s`, `%d`, `%u`, `%x`, `%p`, `%%`. Uses `__builtin_va_list` and GCC's `__builtin_va_start` / `__builtin_va_arg` builtins (works in freestanding mode without `<stdarg.h>`). Integer printing is recursive-free — digits are buffered into a local array then printed in reverse. Writes directly to `vga_putchar()`.

**`lib/ports.c`** — C-linkage wrappers around the `inb`/`outb` inline assembly in `kernel.h`. Useful when a translation unit needs port I/O without pulling in all of `kernel.h`.

---

## Prerequisites

### Ubuntu / Debian

```bash
sudo apt update
sudo apt install -y \
    gcc-i686-linux-gnu \
    binutils \
    nasm \
    qemu-system-x86 \
    grub-pc-bin \
    xorriso \
    mtools
```

> Update `Makefile`: change `CC = i686-elf-gcc` to `CC = i686-linux-gnu-gcc`

### macOS (Homebrew)

```bash
brew install i686-elf-gcc i686-elf-binutils nasm qemu xorriso
```

### Verify

```bash
i686-elf-gcc --version    # cross-compiler
nasm --version             # assembler
qemu-system-i386 --version # emulator
grub-mkrescue --version    # ISO builder
```

---

## Building

```bash
# Clone / enter the project directory
cd mykernel

# Compile all sources → mykernel.bin (ELF)
make

# Wrap in a GRUB-bootable ISO → mykernel.iso
make iso

# Wipe all object files and outputs
make clean
```

---

## Running

```bash
# Build ISO and launch QEMU in one step
make run

# Or run the ISO manually
qemu-system-i386 -cdrom mykernel.iso

# Run the raw ELF binary (QEMU acts as bootloader)
qemu-system-i386 -kernel mykernel.bin

# With 128 MB RAM and serial output to your terminal
qemu-system-i386 -cdrom mykernel.iso -m 128M -serial stdio
```

### Verify the binary before running

```bash
# Confirm ELF headers are correct
i686-elf-readelf -h mykernel.bin

# Confirm multiboot header is present and valid (exit 0 = OK)
grub-file --is-x86-multiboot mykernel.bin && echo "multiboot OK"

# Inspect the entry point disassembly
i686-elf-objdump -d mykernel.bin | head -40
```

---

## Debugging with GDB

QEMU exposes a GDB stub on port 1234 when launched with `-s -S`. The `-S` flag freezes the CPU at boot so you can set breakpoints before execution starts.

**Terminal 1 — launch QEMU frozen:**

```bash
qemu-system-i386 -kernel mykernel.bin -s -S
```

**Terminal 2 — attach GDB:**

```bash
gdb mykernel.bin
(gdb) target remote :1234
(gdb) break kernel_main
(gdb) continue
```

Useful GDB commands in kernel context:

| Command | Description |
|---|---|
| `info registers` | Dump all CPU registers |
| `x/10i $eip` | Disassemble 10 instructions at current EIP |
| `x/16xw 0xB8000` | Inspect VGA framebuffer |
| `break panic` | Break on kernel panic |
| `set architecture i386` | Ensure 32-bit mode |

---

## Boot Sequence

```
Power on / QEMU start
  └─ GRUB scans first 8 KB of binary for magic 0x1BADB002
       └─ GRUB verifies checksum, loads ELF into RAM at 0x100000
            └─ GRUB jumps to kernel_start in 32-bit protected mode
                 └─ boot.asm: set esp, push eax (magic) + ebx (mbi), call kernel_main
                      └─ kernel_main.c:
                           1. vga_init()        — console first, so kprintf works
                           2. verify magic      — confirm GRUB handshake
                           3. gdt_init()        — flat segment model
                           4. idt_init()        — 256-gate descriptor table
                           5. isr_init()        — install exception stubs 0–31
                           6. irq_init()        — remap PIC, install IRQ stubs 32–47
                           7. pmm_init(mbi)     — parse GRUB memory map
                           8. paging_init()     — identity map 0–4 MB, enable CR0.PG
                           9. kmalloc_init()    — set up kernel heap at 0x400000
                          10. timer_init(100)   — PIT at 100 Hz
                          11. keyboard_init()   — PS/2 driver on IRQ 1
                          12. sti              — interrupts enabled (last)
                               └─ hlt idle loop — CPU sleeps, wakes on every IRQ
```

---

## Memory Layout

```
Physical address      Contents
──────────────────────────────────────────────────────
0x00000000            BIOS data, real-mode IVT (reserved)
0x00007C00            MBR / GRUB stage 1 (transient)
0x00100000  ←─────── kernel load address (1 MB)
  .text               code — multiboot header first
  .rodata             read-only data (string literals, const tables)
  .data               initialised globals
  .bss                uninitialised (zero-filled at boot) — stack (16 KB),
                      page directory, page table, PMM bitmap
  kernel_end ←─────── linker symbol exported to pmm.c
0x00400000            kernel heap base  (HEAP_START in kmalloc.c)
0x00800000            kernel heap ceiling (HEAP_MAX in kmalloc.c)
```

Page layout (after `paging_init`):

```
Virtual == Physical    0x000000 – 0x3FFFFF    identity-mapped (first 4 MB)
Heap pages             0x400000+              mapped on demand by kmalloc
```

---

## What Happens at Runtime

After init the kernel sits in the `hlt` idle loop. All activity is interrupt-driven:

| Event | IRQ | Handler |
|---|---|---|
| Timer tick (10 ms) | IRQ 0 → vector 32 | `timer_irq_handler` increments `ticks` |
| Key pressed | IRQ 1 → vector 33 | `keyboard_irq_handler` decodes scan code, pushes to ring buffer |
| CPU exception | vectors 0–31 | `isr_handler` — dispatches to registered handler or panics |
| Page fault | vector 14 | Prints faulting address from CR2 then panics |

---

## Extending the Kernel

The architecture is designed to be extended one layer at a time:

**Interactive shell** — wire `keyboard_getchar()` into a read-eval loop. Parse commands like `help`, `mem`, `reboot`. All the pieces are already in place.

**Process scheduler** — add a `task_t` struct with a saved `esp` and a stack. Use the 100 Hz timer IRQ to context-switch between tasks. Round-robin scheduling fits in under 50 lines of C on top of this kernel.

**User mode (ring 3)** — add a user-mode GDT entry (DPL=3), set up a TSS, and `iret` into ring 3. Add system call dispatch via `int 0x80` (vector 128 in the IDT).

**Higher-half kernel** — remap the kernel to `0xC0100000` (3 GB virtual) in the page directory. This frees the entire lower 3 GB of virtual address space for user processes.

**File system** — add an ATA PIO driver (ports `0x1F0`+, IRQ 14/15), then layer a FAT12/16 or simple ext2 reader on top.

**Serial output** — add a UART driver (port `0x3F8`, IRQ 4) and mirror `kprintf` output to it. Extremely useful for automated testing — QEMU can pipe serial to stdout.

---

## References

- [OSDev Wiki](https://wiki.osdev.org) — the definitive reference for bare-metal x86 programming
- [Intel 64 and IA-32 Architectures Software Developer Manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
- [8259 PIC datasheet](https://pdos.csail.mit.edu/6.828/2014/readings/hardware/8259A.pdf)
- [8253/8254 PIT datasheet](https://www.scs.stanford.edu/10wi-cs140/pintos/specs/8254.pdf)
- [James Molloy's kernel tutorial](http://www.jamesmolloy.co.uk/tutorial_html/) — classic walkthrough that inspired this project

---

*mini-kernel — built from scratch, one approved step at a time.*