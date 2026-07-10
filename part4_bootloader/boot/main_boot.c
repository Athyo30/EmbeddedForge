#include <stdint.h>
#include "uart_boot.h"

extern uint32_t _app_vector_start;    /* linker symbol: app's vector table addr */
extern void jump_to_app(uint32_t sp, uint32_t pc);   /* provided, in jump_to_app.c */

/* SCB->VTOR — the Vector Table Offset Register. Cortex-M hardware
 * reads exceptions/interrupts from whatever address is IN THIS
 * REGISTER, not always from address 0. On cold boot it defaults to
 * 0 (hence why the boot vector table at address 0 is what runs
 * first) — but you can repoint it, which is exactly how a bootloader
 * hands off interrupt handling to the application's own vector table. */
#define SCB_VTOR (*(volatile uint32_t *)0xE000ED08u) //This hardcoded address is the register in which the address of the VTOR lives in. Initially SCB_VTOR is 0x0000, cause thats whats in this register at 0xE000ED08u address. 

/* Sanity bounds for a plausible initial stack pointer on this board
 * (SRAM is 0x20000000..0x20010000 — see linker.ld). A real bootloader
 * would check a lot more (CRC/signature over the app image, a magic
 * number, etc). This is a minimal, teachable version of the same
 * instinct: don't blindly jump to whatever is sitting in flash. */
#define SRAM_BASE 0x20000000u
#define SRAM_END  0x20010000u

/* ==================================================================
 * YOUR EXERCISE:
 *   1. Read the app's initial SP and entry point (reset handler
 *      address) out of the app vector table (slots 0 and 1 — same
 *      layout you already know from Part 2/3's vector tables).
 *   2. Sanity-check the SP: is it within SRAM_BASE..SRAM_END? If not,
 *      print an error and halt (don't jump into garbage).
 *   3. Relocate the vector table: write the app vector table's
 *      address into SCB_VTOR, so interrupts/exceptions from now on
 *      are looked up from the APP's table, not the boot table.
 *   4. Call jump_to_app(sp, pc) — this does not return.
 * ================================================================== */
int boot_main(void)
{
    uart_puts("\r\n== part4: bootloader ==\r\n");

    uint32_t *app_vec = (uint32_t *)&_app_vector_start;

    //read app_vec[0] (initial SP) and app_vec[1] (entry)
    uint32_t app_sp = app_vec[0];
    uint32_t app_pc = app_vec[1]; //Reset Handler

    //SANITY CHECKING SP ADDRESS BEFORE JUMPING
    if(app_sp < SRAM_BASE || app_sp > SRAM_END)
    {
        //Out of bounds. 
        uart_puts("bad app SP, halting\r\n");
        for(;;){} //Will just spin here.
    }

    uart_puts("app vector table at: "); uart_puthex((uint32_t)app_vec); uart_puts("\r\n");
    uart_puts("app SP: "); uart_puthex(app_sp); uart_puts("\r\n");    
    uart_puts("app PC: "); uart_puthex(app_pc); uart_puts("\r\n");    

    SCB_VTOR = (uint32_t) app_vec; //Casting to uint32_t cause app_vec is a pointer

    uart_puts("jumping to app...\r\n");

    jump_to_app(app_sp, app_pc);

    for (;;) {}   /* should never get here */
}
