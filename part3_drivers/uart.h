#ifndef UART_H
#define UART_H
#include <stdint.h>
void uart_init(void);          /* enable UART + RX interrupt in NVIC */
void uart_putc(char c);
void uart_puts(const char *s);
int  uart_getc(uint8_t *out);  /* pop one byte from RX ring; -1 if none */
#endif
