CROSS  := arm-none-eabi-
CC     := $(CROSS)gcc
CFLAGS := -mcpu=cortex-m3 -mthumb -Wall -Wextra -g -O0 -ffreestanding -nostdlib -I boot
OBJS   := boot/startup_boot.o boot/main_boot.o boot/uart_boot.o boot/jump_to_app.o \
          app/startup_app.o app/main_app.o

qemu: firmware.elf
	qemu-system-arm -M lm3s6965evb -nographic -kernel firmware.elf

firmware.elf: $(OBJS) linker.ld
	$(CC) $(CFLAGS) -T linker.ld $(OBJS) -o $@
	$(CROSS)size $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f boot/*.o app/*.o firmware.elf

.PHONY: qemu clean
