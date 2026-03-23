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
C_SRCS :=
C_SRCS += kernel/main.c
C_SRCS += kernel/panic.c
C_SRCS += kernel/shell.c
C_SRCS += drivers/vga.c
C_SRCS += drivers/keyboard.c
C_SRCS += drivers/timer.c
C_SRCS += cpu/gdt.c
C_SRCS += cpu/idt.c
C_SRCS += cpu/isr.c
C_SRCS += cpu/irq.c
C_SRCS += mm/kmalloc.c
C_SRCS += mm/paging.c
C_SRCS += mm/pmm.c
C_SRCS += lib/string.c
C_SRCS += lib/kprintf.c
C_SRCS += lib/ports.c

ASM_SRCS :=
ASM_SRCS += boot/boot.asm
ASM_SRCS += boot/kernel_entry.asm
ASM_SRCS += cpu/tables_flush.asm

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
