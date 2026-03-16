#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "generic.h"
#include "application/app_interface.h"
#include "uapi/strat_os_sched.h"
#include "unity.h"

#define SCHED_RATE_TEST_WINDOW_MS            12000U
#define SCHED_RATE_TEST_VERIFIER_PERIOD_MS   100U
#define SCHED_RATE_TEST_TOLERANCE_US         50000ULL
#define SCHED_RATE_TEST_MIN_INTERVAL_SAMPLES 3U

typedef struct
{
    uint32_t period_ms;
    volatile uint32_t run_count;
    uint32_t interval_count;
    uint64_t first_run_us;
    uint64_t last_run_us;
    uint64_t total_interval_us;
    uint64_t max_abs_error_us;
} sched_rate_stat_t;

static sched_rate_stat_t g_stats[] =
{
    { 20U,   0U, 0U, 0U, 0U, 0U, 0U },
    { 30U,   0U, 0U, 0U, 0U, 0U, 0U },
    { 40U,   0U, 0U, 0U, 0U, 0U, 0U },
    { 50U,   0U, 0U, 0U, 0U, 0U, 0U },
    { 80U,   0U, 0U, 0U, 0U, 0U, 0U },
    { 120U,  0U, 0U, 0U, 0U, 0U, 0U },
    { 200U,  0U, 0U, 0U, 0U, 0U, 0U },
    { 500U,  0U, 0U, 0U, 0U, 0U, 0U },
    { 1000U, 0U, 0U, 0U, 0U, 0U, 0U }
};

static volatile boolean g_test_finished;
static uint64_t g_test_start_us;

static uint64_t get_time_us(void);
static void record_task_run(uint32_t idx);
static void sched_task_0(void);
static void sched_task_1(void);
static void sched_task_2(void);
static void sched_task_3(void);
static void sched_task_4(void);
static void sched_task_5(void);
static void sched_task_6(void);
static void sched_task_7(void);
static void sched_task_8(void);
static void sched_rate_verifier_task(void);
static void test_three_tasks_execute_at_configured_rate(void);
static void print_rate_summary(uint64_t elapsed_ms);

void setUp(void)
{
}

void tearDown(void)
{
}

static const STRAT_OS_task_type app_tasks[] =
{
    { 20U,   sched_task_0 },
    { 30U,   sched_task_1 },
    { 40U,   sched_task_2 },
    { 50U,   sched_task_3 },
    { 80U,   sched_task_4 },
    { 120U,  sched_task_5 },
    { 200U,  sched_task_6 },
    { 500U,  sched_task_7 },
    { 1000U, sched_task_8 },
    { SCHED_RATE_TEST_VERIFIER_PERIOD_MS, sched_rate_verifier_task }
};

boolean APP_sched_configure(void)
{
    uint32_t i;

    for(i = 0U; i < (uint32_t)list_cnt(g_stats); i++)
    {
        g_stats[i].run_count = 0U;
        g_stats[i].interval_count = 0U;
        g_stats[i].first_run_us = 0U;
        g_stats[i].last_run_us = 0U;
        g_stats[i].total_interval_us = 0U;
        g_stats[i].max_abs_error_us = 0U;
    }

    g_test_finished = FALSE;
    g_test_start_us = get_time_us();

    return (STRAT_OS_SCHED_init(app_tasks, list_cnt(app_tasks)) == STRAT_OS_SCHED_ERR__NO_ERROR);
}

static uint64_t get_time_us(void)
{
    struct timeval tv;

    (void)gettimeofday(&tv, NULL);
    return ((uint64_t)tv.tv_sec * 1000000ULL) + (uint64_t)tv.tv_usec;
}

static void record_task_run(uint32_t idx)
{
    uint64_t now_us;
    uint64_t expected_interval_us;

    if(idx >= (uint32_t)list_cnt(g_stats))
    {
        return;
    }

    now_us = get_time_us();
    g_stats[idx].run_count++;

    if(g_stats[idx].first_run_us == 0U)
    {
        g_stats[idx].first_run_us = now_us;
        g_stats[idx].last_run_us = now_us;
        return;
    }

    expected_interval_us = (uint64_t)g_stats[idx].period_ms * 1000ULL;

    if(now_us > g_stats[idx].last_run_us)
    {
        uint64_t interval_us = now_us - g_stats[idx].last_run_us;
        uint64_t abs_error_us = (interval_us > expected_interval_us)
                              ? (interval_us - expected_interval_us)
                              : (expected_interval_us - interval_us);

        g_stats[idx].interval_count++;
        g_stats[idx].total_interval_us += interval_us;

        if(abs_error_us > g_stats[idx].max_abs_error_us)
        {
            g_stats[idx].max_abs_error_us = abs_error_us;
        }
    }

    g_stats[idx].last_run_us = now_us;
}

static void sched_task_0(void) { record_task_run(0U); }
static void sched_task_1(void) { record_task_run(1U); }
static void sched_task_2(void) { record_task_run(2U); }
static void sched_task_3(void) { record_task_run(3U); }
static void sched_task_4(void) { record_task_run(4U); }
static void sched_task_5(void) { record_task_run(5U); }
static void sched_task_6(void) { record_task_run(6U); }
static void sched_task_7(void) { record_task_run(7U); }
static void sched_task_8(void) { record_task_run(8U); }

static void print_rate_summary(uint64_t elapsed_ms)
{
    uint32_t i;

    printf("\n[sched_rate_test] elapsed_ms=%llu, tolerance_us=%llu\n",
           (unsigned long long)elapsed_ms,
           (unsigned long long)SCHED_RATE_TEST_TOLERANCE_US);

    for(i = 0U; i < (uint32_t)list_cnt(g_stats); i++)
    {
        uint64_t avg_interval_us = 0U;

        if(g_stats[i].interval_count > 0U)
        {
            avg_interval_us = g_stats[i].total_interval_us / g_stats[i].interval_count;
        }

        printf("[sched_rate_test] period_ms=%u runs=%u intervals=%u avg_interval_us=%llu max_abs_error_us=%llu\n",
               (unsigned int)g_stats[i].period_ms,
               (unsigned int)g_stats[i].run_count,
               (unsigned int)g_stats[i].interval_count,
               (unsigned long long)avg_interval_us,
               (unsigned long long)g_stats[i].max_abs_error_us);
    }
}

static void test_three_tasks_execute_at_configured_rate(void)
{
    uint32_t i;

    for(i = 0U; i < (uint32_t)list_cnt(g_stats); i++)
    {
        TEST_ASSERT_GREATER_THAN_UINT32(0U, g_stats[i].run_count);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32(SCHED_RATE_TEST_MIN_INTERVAL_SAMPLES, g_stats[i].interval_count);
        TEST_ASSERT_LESS_OR_EQUAL_UINT64(SCHED_RATE_TEST_TOLERANCE_US, g_stats[i].max_abs_error_us);
    }
}

static void sched_rate_verifier_task(void)
{
    uint64_t elapsed_ms;
    int unity_result;

    if(g_test_finished == TRUE)
    {
        return;
    }

    if(g_test_start_us == 0U)
    {
        return;
    }

    elapsed_ms = (get_time_us() - g_test_start_us) / 1000ULL;

    if(elapsed_ms < SCHED_RATE_TEST_WINDOW_MS)
    {
        return;
    }

    g_test_finished = TRUE;

    UNITY_BEGIN();
    RUN_TEST(test_three_tasks_execute_at_configured_rate);
    unity_result = UNITY_END();

    print_rate_summary(elapsed_ms);

    exit(unity_result == 0 ? 0 : 1);
}
