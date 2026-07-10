#include <stdint.h>

/* PROVIDED. This is raw ARM boilerplate, not the concept this exercise
 * is testing — so it's given to you complete. What it does, in plain
 * terms: a normal function call can't change the stack pointer out
 * from under itself safely, so this has to be written in assembly.
 *   msr msp, r0   -> load the app's initial stack pointer (1st arg)
 *   bx  r1        -> branch to the app's reset handler address (2nd arg)
 * `naked` means: no compiler-generated prologue/epilogue — this
 * function's body is EXACTLY the two instructions below, nothing more. */
__attribute__((naked)) void jump_to_app(uint32_t sp, uint32_t pc)
{
    __asm volatile (
        "msr msp, r0 \n"
        "bx  r1      \n"
    );
}
