#include <stdint.h>
#include "../boot/uart_boot.h"

#define SCB_VTOR (*(volatile uint32_t *)0xE000ED08u)

/* PROVIDED. If you see this print, with VTOR correctly reporting
 * 0x4000, your bootloader's relocation + jump both worked — this is
 * your actual proof, the same way part2's main.c self-checked .data
 * and .bss. */
int app_main(void)
{
    uart_puts("== part4: APPLICATION running ==\r\n");
    uart_puts("SCB_VTOR now reads: "); uart_puthex(SCB_VTOR); uart_puts("\r\n");
    uart_puts(SCB_VTOR == 0x4000u ? "VTOR relocated correctly: OK\r\n"
                                   : "VTOR relocated correctly: FAIL\r\n");
    for (;;) {}
}
