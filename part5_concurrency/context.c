#include "context.h"

/* ==================================================================
 * THE BIG PICTURE — read this before touching anything.
 *
 * This is a COOPERATIVE scheduler: a task keeps running until it
 * calls yield() itself. Nothing interrupts it against its will (that
 * would be PREEMPTIVE scheduling — a real RTOS feature, and a much
 * hairier thing to bootstrap correctly; noted as a stretch goal at
 * the bottom of this file). Cooperative is simpler and is exactly
 * what coroutines/fibers are, under the hood.
 *
 * The core trick: each task gets its OWN stack (a plain array you
 * own, no malloc). "Switching tasks" means: save this task's
 * callee-saved registers (r4-r11, which the ARM calling convention
 * says a function must preserve across calls) onto ITS stack, then
 * physically change the CPU's SP register to point into a DIFFERENT
 * task's stack, then restore THAT task's registers from there. The
 * task that resumes has no idea it was ever paused — from its own
 * point of view, yield() just... returned. Same as any function call.
 * ================================================================== */

tcb_t   tasks[NUM_TASKS];
uint32_t task_stacks[NUM_TASKS][STACK_WORDS]; //I und why each task gets its own 1KB stack. 
volatile int current_task = 0;

/* ------------------------------------------------------------------
 * PROVIDED: task_init builds a FAKE saved-context on a task's stack,
 * shaped to exactly match what yield()'s "push {r4-r11, lr}" writes
 * (see below). Because it matches that shape, the very first time
 * this task is switched in, yield()'s own restore code can't tell
 * the difference between "a real previous yield() call" and "a
 * task that has never run" — it just pops 8 don't-care registers and
 * an "lr" that happens to be the task's entry point, then `bx lr`
 * jumps there. No special-casing needed for first-run. Neat trick,
 * worth understanding, not worth re-deriving under time pressure —
 * hence provided complete.
 * ------------------------------------------------------------------ */
void task_init(int id, void (*entry)(void))
{
    uint32_t *sp = &task_stacks[id][STACK_WORDS];   /* top of this task's stack */ //But here, I dont und whats happening.
    sp -= 9;                                         /* room for r4-r11 + lr */
    for (int i = 0; i < 8; i++) sp[i] = 0;            /* r4-r11: don't-care */
    sp[8] = (uint32_t)entry;                          /* "lr" slot -> entry fn */
    tasks[id].sp = sp;
}

/* ------------------------------------------------------------------
 * YOUR EXERCISE: this is the entire scheduling POLICY. Everything
 * about save/restore/stack-swapping is mechanism (provided, below);
 * THIS function is the only place that decides WHICH task runs next.
 * Round-robin: just move to the next task, wrapping back to 0.
 * ------------------------------------------------------------------ */
int pick_next_task(void)
{
    /* TODO: advance current_task to the next task, wrapping around
     *       NUM_TASKS, and return the new value. (current_task is
     *       the global above — update it AND return it.) */
    current_task = ((current_task+1) % NUM_TASKS);
    return current_task;
}

/* ------------------------------------------------------------------
 * PROVIDED: the actual context switch, in assembly. Read the block
 * comment above for the concept; here's the mechanical walk-through:
 *
 *   push {r4-r11, lr}   -- save THIS task's registers onto ITS OWN
 *                          stack (whatever SP currently points to).
 *                          lr is saved too because it holds "where
 *                          to return to" for THIS call to yield() —
 *                          without it, resuming this task later
 *                          wouldn't know where to continue.
 *   ... save current sp into tasks[current_task].sp ...
 *   bl pick_next_task   -- ask the scheduler who runs next
 *   ... load tasks[new current_task].sp into sp ...          <- the
 *                          ACTUAL stack swap. Everything above this
 *                          line touched the OLD task's stack; every-
 *                          thing below touches the NEW one.
 *   pop {r4-r11, lr}    -- restore the NEW task's saved registers
 *   bx lr               -- return — but now "returning" means
 *                          jumping back into wherever the NEW task
 *                          was when IT last called yield() (or its
 *                          entry point, if this is its first run).
 * ------------------------------------------------------------------ */
__attribute__((naked)) void yield(void)
{
    __asm volatile (
        "push {r4-r11, lr}      \n"
        "ldr  r2, =tasks        \n"
        "ldr  r1, =current_task \n"
        "ldr  r1, [r1]          \n"
        "lsl  r3, r1, #2        \n"
        "add  r2, r2, r3        \n"
        "mov  r3, sp            \n"
        "str  r3, [r2]          \n"   /* tasks[current_task].sp = sp */

        "bl   pick_next_task    \n"   /* r0 = new current_task index */

        "ldr  r2, =tasks        \n"
        "lsl  r3, r0, #2        \n"
        "add  r2, r2, r3        \n"
        "ldr  r3, [r2]          \n"   /* r3 = tasks[new].sp */
        "mov  sp, r3            \n"   /* THE actual stack swap */
        "pop  {r4-r11, lr}      \n"
        "bx   lr                \n"
    );
}

/* ------------------------------------------------------------------
 * PROVIDED: kicks off task[current_task] for the very first time.
 * main() calls this once, after task_init-ing everyone, and it never
 * returns — control passes permanently into the task/yield loop.
 * ------------------------------------------------------------------ */
__attribute__((naked)) void start_scheduler(void)
{
    __asm volatile (
        "ldr  r2, =tasks        \n"
        "ldr  r1, =current_task \n"
        "ldr  r1, [r1]          \n"
        "lsl  r3, r1, #2        \n"
        "add  r2, r2, r3        \n"
        "ldr  r3, [r2]          \n"
        "mov  sp, r3            \n"
        "pop  {r4-r11, lr}      \n"
        "bx   lr                \n"
    );
}

/* ==================================================================
 * STRETCH GOAL (not scaffolded here — genuinely hard, do this only
 * once the above feels easy): make this PREEMPTIVE. A SysTick
 * interrupt fires on a timer and forces a switch even if a task
 * never calls yield(). The real mechanism is PendSV + the PSP/MSP
 * dual-stack split + a CONTROL-register switch, and bootstrapping
 * the very first switch correctly is notoriously fiddly — real RTOS
 * ports (FreeRTOS's Cortex-M port.c, for instance) spend real care
 * getting exactly this handoff right. Worth reading about once this
 * cooperative version is solid, not before.
 * ================================================================== */
