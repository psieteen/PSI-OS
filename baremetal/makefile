CC = aarch64-elf-gcc
LD = aarch64-elf-ld
CFLAGS = -ffreestanding -nostdlib -nostartfiles -mstrict-align -Wall -Wextra
LDFLAGS = -T linker.ld -nostdlib

CSRCS = uart.c utils.c timer.c shadow_ai.c shell.c kernel_main.c
OBJS = boot.o $(CSRCS:.c=.o)

all: kernel.elf

boot.o: boot.s
	$(CC) $(CFLAGS) -c boot.s -o boot.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o kernel.elf

clean:
	rm -f *.o *.elf

run: kernel.elf
	qemu-system-aarch64 -M virt -cpu cortex-a72 -nographic -kernel kernel.elf

.PHONY: all clean run
