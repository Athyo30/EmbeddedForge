/* Simplification note (read this first): a real two-stage bootloader
 * ships as TWO SEPARATE flashed images (boot at a fixed low address,
 * app flashed independently later, possibly over serial/XMODEM/CAN).
 * QEMU's `-kernel` only loads one ELF file, so this script instead
 * places BOTH a "boot" vector table and an "app" vector table inside
 * ONE linked binary, at two different flash addresses. This still
 * exercises the real concept — VTOR relocation and a manual jump to
 * a second vector table — without needing real flashing hardware.
 * .data/.bss are shared between boot and app here for simplicity;
 * a real two-image system would give the app its own regions too. */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
    SRAM  (rwx): ORIGIN = 0x20000000, LENGTH = 64K
}

_estack = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{
    /* ---- boot image: lives at the start of flash, hardware reads
     * this vector table on a real cold boot ---- */
    .boot_vector : { KEEP(*(.boot_vector)) } > FLASH
    .boot_text   : { *(.boot_text) *(.boot_rodata*) } > FLASH

    /* ---- app image: force it to start at a fixed offset (16K in),
     * simulating "the app was flashed starting at this address" ---- */
    _app_vector_start = 0x4000;
    .app_vector 0x4000 : { KEEP(*(.app_vector)) } > FLASH
    .app_text   : { *(.app_text) *(.app_rodata*) } > FLASH

    _sidata = LOADADDR(.data);
    .data : {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > SRAM AT > FLASH

    .bss : {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > SRAM
}
