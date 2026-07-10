#include <stdint.h>
#include "context.h"
#include "uart.h"

/* PROVIDED. Two tasks, each an infinite loop that prints its own
 * letter a fixed number of times then yields. If your context switch
 * is correct, you'll see the output INTERLEAVE (A,B,A,B,...) rather
 * than one task hogging forever — that interleaving is your actual
 * proof two independent stacks and register sets are being swapped
 * correctly, not just "the code compiled." */

static void task_a(void)
{
    for (;;) {
        uart_puts("A");
        yield();
    }
}

static void task_b(void)
{
    for (;;) {
        uart_puts("B");
        yield();
    }
}

int main(void)
{
    uart_init();
    uart_puts("\r\n== part5: cooperative scheduler ==\r\n");

    task_init(0, task_a);
    task_init(1, task_b);
    current_task = 0;

    start_scheduler();   /* never returns */

    for (;;) {}
}
