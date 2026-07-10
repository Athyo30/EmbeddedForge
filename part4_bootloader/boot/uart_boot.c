#include <stdint.h>
#include "uart_boot.h"

#define UART0     0x4000C000u
#define UART_DR   (*(volatile uint32_t*)(UART0+0x00))
#define UART_FR   (*(volatile uint32_t*)(UART0+0x18))
#define FR_TXFF   (1u<<5)

void uart_putc(char c){ while(UART_FR & FR_TXFF){} UART_DR = (uint32_t)c; }
void uart_puts(const char *s){ while(*s) uart_putc(*s++); }

/* Minimal hex printer — useful for printing addresses without pulling
 * in printf (there is no libc here). */
void uart_puthex(uint32_t v)
{
    uart_puts("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uint8_t nib = (v >> i) & 0xF;
        uart_putc(nib < 10 ? ('0' + nib) : ('A' + nib - 10));
    }
}
