CC = aarch64-elf-gcc
LD = aarch64-elf-ld
CFLAGS = -ffreestanding -nostdlib -nostartfiles -mstrict-align -Wall -Wextra
LDFLAGS = -T linker.ld -nostdlib

all: kernel.elf

boot.o: boot.s
	$(CC) $(CFLAGS) -c boot.s -o boot.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

kernel.elf: boot.o kernel.o
	$(LD) $(LDFLAGS) boot.o kernel.o -o kernel.elf

clean:
	rm -f *.o *.elf

run: kernel.elf
	qemu-system-aarch64 -M virt -cpu cortex-a72 -nographic -kernel kernel.elf

.PHONY: all clean run
