// unit_test_ssched.c
#include <stdio.h>
#include <setjmp.h>
#include "generic.h"
#include "sched.h"
#include "peripherals/timer.h"
#include "../../../ssched/ssched.c"

#define MAX_NUMBER_OF_TASKS 10

/* test variables */
sched_usr_tsk_t task_list[MAX_NUMBER_OF_TASKS];

uint64_t task_call_count = 0;

jmp_buf buf;

/* functions */
static void test(void);
static void task_func(void);

int main() {
    test();

    return 0;
}

/* unit tests */
static void test()
{
    /* init with a single task */
    task_list[0].period_ms = 1500;
    task_list[0].task_func = task_func;
    sched_init(task_list, 1);

    if (setjmp(buf) == 0) {
        printf("Successfully set the jump point.");

        schedule_isr();
        sched_main();
    } else {
        printf("Task function was called!\n");
    }
}

static void task_func(void)
{
    task_call_count = task_call_count + 1;

    printf("\njump_buf=%d\n", buf);
    printf("\ntest=%d\n", test);
    longjmp(buf, 1);
}

/* helper functions */

/* mocked functions */

timer_err_t8 timer_alloc(timer_id_t8 * timer_id, void_func_t irq_cb, uint32_t ticks)
{
    (void)timer_id;
    (void)irq_cb;
    (void)ticks;

    return TIMER_ERR_NONE;
}

void uart_init()
{
}

boolean uart_is_init()
{
    return TRUE;
}