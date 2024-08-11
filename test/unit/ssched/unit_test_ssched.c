// unit_test_ssched.c
#include <stdio.h>
#include <setjmp.h>
#include "generic.h"
#include "sched.h"
#include "unity.h"
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
void run_single_cycle(u_int64_t period);
void tick_system(uint64_t ticks);

void setUp(void)
{
}

void tearDown(void)
{
}

int main() {
    test();

    return 0;
}

/* unit tests */
static void test()
{
    #define TASK_PERIOD 1500
    int i;

    // Test that a registered task will be executed at the rate specified in the definition block
    task_list[0].period_ms = TASK_PERIOD;
    task_list[0].task_func = task_func;
    sched_init(task_list, 1);

    for(i = 0; i < 38; i++)
    {
        run_single_cycle(TASK_PERIOD);
    }

    TEST_ASSERT_EQUAL_UINT64(38, task_call_count);

    task_call_count = 0; /* reset call count */

    printf("yay passed the test\n");

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

/* helper functions */

/**********************************************************
 *
 *  run_single_cycle()
 *
 *
 *  DESCRIPTION:
 *      Helper function to run scheduler for a specific
 *      period.
 *
 */

void run_single_cycle(u_int64_t period)
{
    if (setjmp(buf) == 0) {
        tick_system(period / MS_PER_TICKS);
    } else {
        return;
    }
}

/**********************************************************
 *
 *  tick_system()
 *
 *
 *  DESCRIPTION:
 *      Helper function to tick the system forward in time
 *      by simulating timer based scheduling ISR.
 *
 *  NOTES:
 *      calls sched_main() after each task period which is
 *      not representative of the real world execution,
 *      sequence but should still be sufficient to test
 *      scheduling behavior.
 */

void tick_system(uint64_t ticks)
{
    uint64_t i;

    for(i = 0; i < ticks; i++)
    {
        schedule_isr();
    }
    sched_main();
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