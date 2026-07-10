#include <stdint.h>
/* PROVIDED — identical pattern to Part 2, you've already built this. */
extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _estack;
int  main(void);
void Reset_Handler(void);
void Default_Handler(void) { for (;;) {} }

__attribute__((section(".isr_vector")))
void (* const vector_table[])(void) = {
    (void (*)(void))(&_estack), Reset_Handler,
    Default_Handler, Default_Handler,
};

void Reset_Handler(void)
{
    uint32_t *s = &_sidata, *d = &_sdata;
    while (d < &_edata) *d++ = *s++;
    for (d = &_sbss; d < &_ebss; ) *d++ = 0;
    main();
    for (;;) {}
}
