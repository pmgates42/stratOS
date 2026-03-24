/* #define DEBUG_TEST */

#ifndef EMBEDDED_BUILD
    #include <stdio.h>
    #include <stdlib.h>
#else
    #include "printf.h"
    #undef printf
    #undef sprintf
#endif

#include "generic.h"
#include "application/app_interface.h"
#include "uapi/strat_os_sched.h"
#include "peripherals/timer.h"
#include "unity.h"

#ifdef EMBEDDED_BUILD
    #define printf tfp_printf
    #define sprintf tfp_sprintf
#endif

#if defined(DEBUG_TEST)
#include "debug.h"
#endif

#define SCHED_RATE_TEST_WINDOW_MS            12000U
#define SCHED_RATE_TEST_VERIFIER_PERIOD_MS   100U
#define SCHED_RATE_TEST_TOLERANCE_US         10ULL
#define SCHED_RATE_TEST_WARMUP_INTERVALS     3U
#define SCHED_RATE_TEST_MIN_INTERVAL_SAMPLES 3U

typedef struct
{
    uint32_t period_ms;
    volatile uint32_t run_count;
    uint32_t interval_count;
    uint32_t checked_interval_count;
    uint64_t first_run_us;
    uint64_t last_run_us;
    uint64_t total_interval_us;
    uint64_t max_abs_error_us;
} sched_rate_stat_t;

static sched_rate_stat_t g_stats[] =
{
    { 20U,   0U, 0U, 0U, 0U, 0U, 0U, 0U },
    { 30U,   0U, 0U, 0U, 0U, 0U, 0U, 0U },
    { 40U,   0U, 0U, 0U, 0U, 0U, 0U, 0U },
    { 50U,   0U, 0U, 0U, 0U, 0U, 0U, 0U },
    { 80U,   0U, 0U, 0U, 0U, 0U, 0U, 0U },
    { 120U,  0U, 0U, 0U, 0U, 0U, 0U, 0U },
    { 200U,  0U, 0U, 0U, 0U, 0U, 0U, 0U },
    { 500U,  0U, 0U, 0U, 0U, 0U, 0U, 0U },
    { 1000U, 0U, 0U, 0U, 0U, 0U, 0U, 0U }
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
static void finish_test(int unity_result);

static void debug_print_task_func_ptr(const char * label, void (*task_func)(void))
{
    size_t raw_ptr = (size_t)task_func;
    uint64_t raw_ptr64 = (uint64_t)raw_ptr;
    uint32_t hi = 0U;
    uint32_t lo = (uint32_t)(raw_ptr & 0xFFFFFFFFUL);

    if(sizeof(size_t) > sizeof(uint32_t))
    {
        hi = (uint32_t)((raw_ptr64 >> 32U) & 0xFFFFFFFFULL);
    }

    printf("\n%s=0x%08x%08x", label, (unsigned int)hi, (unsigned int)lo);
}

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
        g_stats[i].checked_interval_count = 0U;
        g_stats[i].first_run_us = 0U;
        g_stats[i].last_run_us = 0U;
        g_stats[i].total_interval_us = 0U;
        g_stats[i].max_abs_error_us = 0U;
    }

    g_test_finished = FALSE;
    g_test_start_us = get_time_us();

    printf("\nInitializing scheduler with %u tasks\n", (unsigned int)list_cnt(app_tasks));

    #if defined(EMBEDDED_BUILD) && defined(DEBUG_TEST)
        debug_init();
        debug_set_led();
    #endif

    debug_print_task_func_ptr("sched_task_0", sched_task_0);

    return (STRAT_OS_SCHED_init(app_tasks, list_cnt(app_tasks)) == STRAT_OS_SCHED_ERR__NO_ERROR);
}

static uint64_t get_time_us(void)
{
    return timer_get_time_us();
}

static void record_task_run(uint32_t idx)
{
    uint64_t now_us;
    uint64_t expected_interval_us;

    assert( idx < (uint32_t)list_cnt(g_stats), "Invalid task index" );

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

        if(g_stats[idx].interval_count > SCHED_RATE_TEST_WARMUP_INTERVALS)
        {
            g_stats[idx].checked_interval_count++;
            if(abs_error_us > g_stats[idx].max_abs_error_us)
            {
                g_stats[idx].max_abs_error_us = abs_error_us;
            }
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

    printf("\n[sched_rate_test] elapsed_ms=%u, tolerance_us=%u\n",
           (unsigned int)elapsed_ms,
           (unsigned int)SCHED_RATE_TEST_TOLERANCE_US);

    for(i = 0U; i < (uint32_t)list_cnt(g_stats); i++)
    {
        uint64_t avg_interval_us = 0U;

        if(g_stats[i].interval_count > 0U)
        {
            avg_interval_us = g_stats[i].total_interval_us / g_stats[i].interval_count;
        }

           printf("[sched_rate_test] period_ms=%u runs=%u intervals=%u checked=%u avg_interval_us=%u max_abs_error_us=%u\n",
               (unsigned int)g_stats[i].period_ms,
               (unsigned int)g_stats[i].run_count,
               (unsigned int)g_stats[i].interval_count,
               (unsigned int)g_stats[i].checked_interval_count,
               (unsigned int)avg_interval_us,
               (unsigned int)g_stats[i].max_abs_error_us);
    }
}

static void test_three_tasks_execute_at_configured_rate(void)
{
    uint32_t i;

    for(i = 0U; i < (uint32_t)list_cnt(g_stats); i++)
    {
        TEST_ASSERT_GREATER_THAN_UINT32(0U, g_stats[i].run_count);
        TEST_ASSERT_GREATER_OR_EQUAL_UINT32(SCHED_RATE_TEST_MIN_INTERVAL_SAMPLES, g_stats[i].checked_interval_count);
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

    finish_test(unity_result);
}

static void finish_test(int unity_result)
{
#ifdef EMBEDDED_BUILD
    if(unity_result == 0)
    {
        printf("\n[sched_rate_test] COMPLETE PASS\n");
    }
    else
    {
        printf("\n[sched_rate_test] COMPLETE FAIL\n");
    }

    while(TRUE)
    {
        /* Hold terminal state for hardware runs after result is printed. */
    }
#else
    exit(unity_result == 0 ? 0 : 1);
#endif
}
