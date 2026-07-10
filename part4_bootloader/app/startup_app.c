#include <stdint.h>

/* PROVIDED. The "application" side of this exercise — not what you're
 * being tested on, just needs to exist so boot_main() has something
 * real to jump to. Note this vector table is a SEPARATE array from
 * boot's, placed at .app_vector (address 0x4000, per linker.ld) — this
 * is the second vector table your bootloader will point SCB_VTOR at. */

extern uint32_t _estack;
int app_main(void);
void App_Reset_Handler(void);
void App_Default_Handler(void) { for (;;) {} }

__attribute__((section(".app_vector")))
void (* const app_vector_table[])(void) = {
    (void (*)(void))(&_estack),   /* reusing the same stack top — see
                                    * linker.ld's note on shared SRAM */
    App_Reset_Handler,
    App_Default_Handler,          /* NMI */
    App_Default_Handler,          /* HardFault */
};

__attribute__((section(".app_text")))
void App_Reset_Handler(void)
{
    /* NOTE: .data/.bss were already initialized by the BOOT stage's
     * Reset_Handler before it ever jumped here (they're shared in
     * this simplified single-image setup — see linker.ld). A real
     * second flashed image would need to redo that copy itself. */
    app_main();
    for (;;) {}
}
