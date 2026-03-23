# mini-kernel Makefile
# Toolchain
CC      = i686-elf-gcc
AS      = nasm
LD      = i686-elf-ld

# Flags
CFLAGS  = -m32 -ffreestanding -O2 -Wall -Wextra \
          -fno-stack-protector -nostdlib -nostdinc \
          -I kernel -I lib

ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -m elf_i386 --nostdlib

# Output
TARGET  = mykernel.bin
ISO     = mykernel.iso

# Source files — order matters for linking
C_SRCS  = kernel/kernel_main.c \
          kernel/panic.c       \
          drivers/vga.c        \
          drivers/keyboard.c   \
          drivers/timer.c      \
          cpu/gdt.c            \
          cpu/idt.c            \
          cpu/isr.c            \
          cpu/irq.c            \
          mm/kmalloc.c         \
          mm/paging.c          \
          mm/pmm.c             \
          lib/string.c         \
          lib/kprintf.c        \
          lib/ports.c

ASM_SRCS = boot/boot.asm \
           boot/kernel_entry.asm

# Object files
C_OBJS   = $(C_SRCS:.c=.o)
ASM_OBJS = $(ASM_SRCS:.asm=.o)
OBJS     = $(ASM_OBJS) $(C_OBJS)

# Default target
.PHONY: all clean iso run

all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

# Compile C
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble
%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

# Build bootable ISO with GRUB
iso: $(TARGET)
	mkdir -p isodir/boot/grub
	cp $(TARGET) isodir/boot/$(TARGET)
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) isodir

# Run in QEMU
run: iso
	qemu-system-i386 -cdrom $(ISO)

clean:
	rm -f $(OBJS) $(TARGET) $(ISO)
	rm -rf isodir