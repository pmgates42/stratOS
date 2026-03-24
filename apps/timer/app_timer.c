#ifndef EMBEDDED_BUILD
    #include <stdio.h>
    #include <stdlib.h>
#else
    #include "printf.h"
    #undef printf
#endif

#include "generic.h"
#include "application/app_interface.h"
#include "uapi/strat_os_sched.h"

#ifdef EMBEDDED_BUILD
    #define printf tfp_printf
#endif

#define TIMER_PERIOD_MS      1000U
#define TIMER_END_SECONDS    100U

typedef struct
{
    uint32_t seconds;
    boolean done;
} timer_state_t;

static timer_state_t g_timer_state = { 0U, FALSE };

static void timer_task(void);
static void finish_timer_test(void);

static const STRAT_OS_task_type app_tasks[] =
{
    { TIMER_PERIOD_MS, timer_task }
};

boolean APP_sched_configure(void)
{
    g_timer_state.seconds = 0U;
    g_timer_state.done = FALSE;

    printf("\n[timer] starting 0..%u second countdown", (unsigned int)TIMER_END_SECONDS);

    return (STRAT_OS_SCHED_init(app_tasks, list_cnt(app_tasks)) == STRAT_OS_SCHED_ERR__NO_ERROR);
}

static void timer_task(void)
{
    if(g_timer_state.done == TRUE)
    {
        return;
    }

    printf("\n[timer] second=%u", (unsigned int)g_timer_state.seconds);

    if(g_timer_state.seconds >= TIMER_END_SECONDS)
    {
        finish_timer_test();
        return;
    }

    g_timer_state.seconds++;
}

static void finish_timer_test(void)
{
    g_timer_state.done = TRUE;

    printf("\n[timer] COMPLETE PASS\n");

#ifdef EMBEDDED_BUILD
    while(TRUE)
    {
        /* Hold terminal output after completion on hardware. */
    }
#else
    exit(0);
#endif
}
