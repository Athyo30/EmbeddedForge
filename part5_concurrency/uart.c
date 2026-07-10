#include <stdint.h>
#include "uart.h"

/* PROVIDED for Part 2 so main() can talk. In Part 3 you will throw this
 * away and write the UART driver yourself from the register map. For now,
 * treat it as a black box. */

#define UART0_BASE 0x4000C000u
#define UART_DR   (*(volatile uint32_t *)(UART0_BASE + 0x00))
#define UART_FR   (*(volatile uint32_t *)(UART0_BASE + 0x18))
#define FR_TXFF   (1u << 5)   /* transmit FIFO full */

void uart_init(void) { /* QEMU's model needs no clock/pin setup */ }

void uart_putc(char c)
{
    while (UART_FR & FR_TXFF) { }
    UART_DR = (uint32_t)c;
}

void uart_puts(const char *s)
{
    while (*s) uart_putc(*s++);
}
