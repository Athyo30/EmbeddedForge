#include <stdint.h>

/* PROVIDED — you already built this exact logic in Part 2. Nothing
 * new here: copy .data, zero .bss, then call boot_main() (this
 * stage's "main", defined in main_boot.c where your exercise is). */

extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _estack;

int  boot_main(void);
void Reset_Handler(void);
void Default_Handler(void) { for (;;) {} }

__attribute__((section(".boot_vector")))
void (* const boot_vector_table[])(void) = {
    (void (*)(void))(&_estack),
    Reset_Handler,
    Default_Handler,   /* NMI */
    Default_Handler,   /* HardFault */
};

__attribute__((section(".boot_text")))
void Reset_Handler(void)
{
    uint32_t *s = &_sidata, *d = &_sdata;
    while (d < &_edata) *d++ = *s++;
    for (d = &_sbss; d < &_ebss; ) *d++ = 0;

    boot_main();
    for (;;) {}   /* boot_main should never return — trap if it does */
}
