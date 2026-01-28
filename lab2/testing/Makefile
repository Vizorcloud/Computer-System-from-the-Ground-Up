#
# A simple makefile for building program composed of C/asm source files.
# Builds for RISC-V target using cross-development toolchain.

NAME = testing

ARCH = -march=rv64im -mabi=lp64
ASFLAGS = $(ARCH)
CFLAGS = $(ARCH) -g -Og -I$(CS107E)/include -Wall -ffreestanding
LDFLAGS = -nostdlib -T memmap.ld

all: $(NAME).bin

%.bin: %.elf
	riscv64-unknown-elf-objcopy $< -O binary $@

%.elf: start.o cstart.o %.o
	riscv64-unknown-elf-ld $(LDFLAGS) $^ -o $@

%.o: %.c
	riscv64-unknown-elf-gcc $(CFLAGS) -c $< -o $@

%.o: %.s
	riscv64-unknown-elf-as $(ASFLAGS) -c $< -o $@

test: $(NAME).bin
	mango-run $<

clean:
	rm -f *.o *.elf *.bin

.PHONY: all clean test
.PRECIOUS: %.elf %.o
