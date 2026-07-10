#ifndef UART_BOOT_H
#define UART_BOOT_H
#include <stdint.h>
void uart_putc(char c);
void uart_puts(const char *s);
void uart_puthex(uint32_t v);
#endif
