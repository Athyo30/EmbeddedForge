#ifndef CONTEXT_H
#define CONTEXT_H
#include <stdint.h>

#define NUM_TASKS   2
#define STACK_WORDS 256   /* 1KB stack per task — plenty for this exercise */

typedef struct {
    uint32_t *sp;   /* this task's saved stack pointer, when it's NOT running */
} tcb_t;

extern tcb_t   tasks[NUM_TASKS];
extern volatile int current_task;

void task_init(int id, void (*entry)(void));
void yield(void);
void start_scheduler(void);   /* never returns */

#endif
