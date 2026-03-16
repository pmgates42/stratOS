#include <stdio.h>
#include <stdlib.h>

#include "generic.h"
#include "application/app_interface.h"
#include "uapi/strat_os_sched.h"
#include "unity.h"

#define SCHED_RATE_TEST_VERIFIER_TARGET_RUNS 20U
#define SCHED_RATE_TEST_VERIFIER_PERIOD_MS  100U
#define SCHED_RATE_TEST_MIN_PERCENT         70U
#define SCHED_RATE_TEST_MAX_EXTRA_RUNS      4U

typedef struct
{
    uint32_t period_ms;
    volatile uint32_t run_count;
} sched_rate_stat_t;

static sched_rate_stat_t g_stats[3] =
{
    { 30U, 0U },
    { 40U, 0U },
    { 50U, 0U }
};

static volatile boolean g_test_finished;
static volatile uint32_t g_verifier_run_count;

static void sched_task_20ms(void);
static void sched_task_30ms(void);
static void sched_task_50ms(void);
static void sched_rate_verifier_task(void);
static void test_three_tasks_execute_at_configured_rate(void);

void setUp(void)
{
}

void tearDown(void)
{
}

static const STRAT_OS_task_type app_tasks[] =
{
    { 30U,  sched_task_20ms },
    { 40U,  sched_task_30ms },
    { 50U,  sched_task_50ms },
    { 100U, sched_rate_verifier_task }
};

boolean APP_sched_configure(void)
{
    g_test_finished = FALSE;
    g_verifier_run_count = 0U;

    return (STRAT_OS_SCHED_init(app_tasks, list_cnt(app_tasks)) == STRAT_OS_SCHED_ERR__NO_ERROR);
}

static void sched_task_20ms(void)
{
    g_stats[0].run_count++;
}

static void sched_task_30ms(void)
{
    g_stats[1].run_count++;
}

static void sched_task_50ms(void)
{
    g_stats[2].run_count++;
}

static void test_three_tasks_execute_at_configured_rate(void)
{
    uint32_t i;
    uint32_t expected_window_ms;

    TEST_ASSERT_GREATER_THAN_UINT32(0U, g_stats[0].run_count);
    TEST_ASSERT_GREATER_THAN_UINT32(0U, g_stats[1].run_count);
    TEST_ASSERT_GREATER_THAN_UINT32(0U, g_stats[2].run_count);

    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(g_stats[1].run_count, g_stats[0].run_count);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(g_stats[2].run_count, g_stats[1].run_count);

    expected_window_ms = SCHED_RATE_TEST_VERIFIER_TARGET_RUNS * SCHED_RATE_TEST_VERIFIER_PERIOD_MS;

    for(i = 0U; i < (uint32_t)list_cnt(g_stats); i++)
    {
        uint32_t expected_runs = (expected_window_ms + (g_stats[i].period_ms / 2U)) / g_stats[i].period_ms;
        uint32_t min_runs = (expected_runs * SCHED_RATE_TEST_MIN_PERCENT) / 100U;
        uint32_t max_runs = expected_runs + SCHED_RATE_TEST_MAX_EXTRA_RUNS;

        TEST_ASSERT_GREATER_OR_EQUAL_UINT32(min_runs, g_stats[i].run_count);
        TEST_ASSERT_LESS_OR_EQUAL_UINT32(max_runs, g_stats[i].run_count);
    }
}

static void sched_rate_verifier_task(void)
{
    int unity_result;

    if(g_test_finished == TRUE)
    {
        return;
    }

    g_verifier_run_count++;

    if(g_verifier_run_count < SCHED_RATE_TEST_VERIFIER_TARGET_RUNS)
    {
        return;
    }

    g_test_finished = TRUE;

    UNITY_BEGIN();
    RUN_TEST(test_three_tasks_execute_at_configured_rate);
    unity_result = UNITY_END();

        printf("\n[sched_rate_test] verifier_runs=%u, task30=%u, task40=%u, task50=%u\n",
            (unsigned int)g_verifier_run_count,
           (unsigned int)g_stats[0].run_count,
           (unsigned int)g_stats[1].run_count,
           (unsigned int)g_stats[2].run_count);

    exit(unity_result == 0 ? 0 : 1);
}
