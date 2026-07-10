#include <stdint.h>
#include "uart.h"

/* This global lives in .data. It reads back as 0xCAFEBABE ONLY if your
 * reset handler copied .data from flash into SRAM. If you skip TODO 1,
 * it holds garbage and the check below fails. This is the load-bearing
 * proof that your startup code works. */
volatile uint32_t data_check = 0xCAFEBABEu;

/* This lives in .bss. The C standard says it must be zero at main().
 * (Note: QEMU happens to zero-fill RAM at boot, so this check can pass
 * even if you skip TODO 2 — on real silicon it would not. Zero it
 * anyway; that's the contract.) */
uint32_t bss_check[8];

static void report(const char *label, int ok)
{
    uart_puts(label);
    uart_puts(ok ? ": OK\r\n" : ": FAIL\r\n");
}

int main(void)
{
    uart_init();
    uart_puts("\r\n== part2: startup ==\r\n");

    report(".data copied", data_check == 0xCAFEBABEu);

    int bss_zero = 1;
    for (int i = 0; i < 8; i++) if (bss_check[i] != 0) bss_zero = 0;
    report(".bss zeroed ", bss_zero);

    uart_puts("main reached the end.\r\n");
    for (;;) { }
}
