/* Memory map for QEMU's lm3s6965evb (Cortex-M3).
 * You do NOT need to edit this — but you should READ it. The symbols
 * defined here (_sidata, _sdata, _edata, _sbss, _ebss, _estack) are
 * exactly what your reset handler consumes. Understanding the LMA-vs-VMA
 * distinction (where .data is STORED in flash vs where it RUNS in RAM)
 * is the entire reason a startup routine has to exist. */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
    SRAM  (rwx): ORIGIN = 0x20000000, LENGTH = 64K
}

_estack = ORIGIN(SRAM) + LENGTH(SRAM);   /* stack grows down from top */

SECTIONS
{
    .isr_vector : { KEEP(*(.isr_vector)) } > FLASH

    .text : {
        *(.text*)
        *(.rodata*)
    } > FLASH

    /* _sidata = address in FLASH where the initial .data image lives */
    _sidata = LOADADDR(.data);

    .data : {
        _sdata = .;          /* .data start in SRAM (VMA) */
        *(.data*)
        _edata = .;          /* .data end in SRAM */
    } > SRAM AT > FLASH      /* runs in SRAM, stored in FLASH */

    .bss : {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > SRAM
}
