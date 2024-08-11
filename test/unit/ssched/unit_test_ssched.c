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
uint64_t task_overrun_call_count = 0;

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
    int i;

    // Test that a task registered on initialization will be executed
    {
        /* init with a single task */
        task_list[0].period_ms = 1500;
        task_list[0].task_func = task_func;
        sched_init(task_list, 1);

        if (setjmp(buf) == 0) {
            schedule_isr();
            sched_main();
        } else {
            printf("Task function was called!\n");
        }
        task_call_count = 0; /* reset call count */
    }

    // Test that a registered task will be called at specified rate in milliseconds
    {
        if (setjmp(buf) == 0) {
            /* Push the ticks to 1500 */
            for(i = 0; i < 1500; i++)
            {
                schedule_isr();
            }
            sched_main();
        } else {
            printf("Task function total call count was %d.\n", task_call_count);
        }

        if (setjmp(buf) == 0) {
            /*  Do a few more iterations */
            for(i = 0; i < (1500 * 2); i++)
            {
                schedule_isr();
            }

            sched_main();
        } else {
            printf("Current state=%d, task is scheduled=%d\n", scheduler_state, task_head->scheduled);
        }
    }
}

static void task_func(void)
{
    task_call_count = task_call_count + 1;
}

static void task_overrun(void)
{
    task_overrun_call_count = task_overrun_call_count + 1;

    /* TODO */
    /* simulate not returning from function for X cycles */
    if(FALSE)
    {
        longjmp(buf, 1);
    }
}

/* hook to stop loop */
void ssched_log_insert_task_cycle_stat_entry(void)
{
    longjmp(buf, 1);
}

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