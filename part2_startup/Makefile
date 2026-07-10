CROSS  := arm-none-eabi-
CC     := $(CROSS)gcc
CFLAGS := -mcpu=cortex-m3 -mthumb -Wall -Wextra -g -O0 -ffreestanding -nostdlib
OBJS   := startup.o main.o uart.o

# Run it: prints startup results over the emulated UART.
# Quit QEMU with:  Ctrl-A  then  x
qemu: firmware.elf
	qemu-system-arm -M lm3s6965evb -nographic -kernel firmware.elf

firmware.elf: $(OBJS) linker.ld
	$(CC) $(CFLAGS) -T linker.ld $(OBJS) -o $@
	$(CROSS)size $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o firmware.elf

.PHONY: qemu clean
