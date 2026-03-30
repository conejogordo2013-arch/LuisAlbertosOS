CROSS ?= i686-elf
CC := $(CROSS)-gcc
LD := $(CROSS)-ld
AS := nasm
CFLAGS := -ffreestanding -nostdlib -m32 -fno-pic -fno-stack-protector -Iinclude -Wall -Wextra

KERNEL_C := $(wildcard kernel/*.c drivers/*.c)
KERNEL_O := $(KERNEL_C:.c=.o) boot/kernel_entry.o

all: bin/boot.bin bin/kernel.bin

boot/kernel_entry.o: boot/kernel_entry.asm
	$(AS) -f elf32 $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

bin/boot.bin: boot/LABootL.asm
	mkdir -p bin
	$(AS) -f bin $< -o $@

bin/kernel.bin: $(KERNEL_O) linker/kernel.ld
	$(LD) -m elf_i386 -T linker/kernel.ld -o bin/kernel.elf $(KERNEL_O)
	objcopy -O binary bin/kernel.elf $@

run: all
	qemu-system-i386 -drive format=raw,file=../LuisAlbertoOS.img -serial stdio

clean:
	rm -f $(KERNEL_O) bin/*
