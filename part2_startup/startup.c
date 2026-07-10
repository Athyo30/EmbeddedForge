#include <stdint.h>

/* Provided: symbols from the linker script. */
extern uint32_t _sidata, _sdata, _edata, _sbss, _ebss, _estack;

int  main(void);
void Reset_Handler(void);
void Default_Handler(void) { for (;;) {} }

/* Provided: the Cortex-M vector table. Word 0 is the initial stack
 * pointer, word 1 is the reset vector. QEMU reads these on boot. */
__attribute__((section(".isr_vector")))
void (* const vector_table[])(void) = {
    (void (*)(void))(&_estack),   /* initial MSP            */ 
    Reset_Handler,                /* reset                  */
    Default_Handler,              /* NMI                    */
    Default_Handler,              /* HardFault              */
    /* (rest omitted — not needed for this exercise)        */
};

/* ==================================================================
 * YOUR EXERCISE: the reset handler is the first C code that runs.
 * The C environment does NOT exist yet. Before main can run you must
 * hand-build it:
 *
 *   1. Copy the .data section from its load address in FLASH
 *      (&_sidata) to its run address in SRAM (&_sdata .. &_edata).
 *      Until you do this, every initialized global holds garbage.
 *
 *   2. Zero the .bss section (&_sbss .. &_ebss). The C standard
 *      promises uninitialized globals are 0; nobody does that for
 *      you.
 *
 *   3. Call main().
 *
 * Work in terms of uint32_t* and word-copy — the linker aligns these
 * regions to 4 bytes. Reason about the pointer arithmetic; don't
 * guess. main.c will print whether each step worked.
 * ================================================================== */
/*
Approach: So need to start copying from _sidata -> sdata to edata. But how much? Whats the end point?
And then need to zero out bss (sbss -> ebss)
*/
void Reset_Handler(void)
{
    /* TODO 1: copy .data (flash -> sram) */
    uint32_t* addr_flash_ptr = &_sidata;
    uint32_t* addr_ram_ptr = &_sdata;

    while(addr_ram_ptr != &_edata)
    {
        *addr_ram_ptr = *addr_flash_ptr;
        addr_flash_ptr++;
        addr_ram_ptr++;
    }

    /* TODO 2: zero .bss */
    uint32_t* addr_bss_ptr = &_sbss;
    while(addr_bss_ptr != &_ebss)
    {
        *addr_bss_ptr = 0;
        addr_bss_ptr++;
    }
    /* TODO 3: call main, then trap if it ever returns */
    main();
    for (;;) {}
}
