/**********************************************************
 *
 *  sim_timer.c
 *
 *  DESCRIPTION:
 *      Simluated timer module.
 *
 */

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "generic.h"
#include "peripherals/timer.h"

#define TICKS_PER_USEC 1 /* System timer runs at 1Mhz */
#define TICKS_PER_MS ( 1000 * TICKS_PER_USEC )
#define TICKS_PER_SECOND ( 1000 * TICKS_PER_MS )

#define MS_TO_USEC (1000)
#define US_TO_MSEC (1/US_TO_MSEC)

#define SIM_MAX_TIMERS 10

typedef struct
{
    boolean allocated;
    uint32_t tick_rate;
    void_func_t irq_cb;
    pthread_t thread;
} sim_timer_ctrl_t;

static sim_timer_ctrl_t sim_timers[SIM_MAX_TIMERS];
static pthread_mutex_t sim_timer_lock = PTHREAD_MUTEX_INITIALIZER;

void* simulate_shed_timer_isr(void* arg);
static void timespec_add_us(struct timespec * ts, uint32_t us);

static void timespec_add_us(struct timespec * ts, uint32_t us)
{
    uint64_t add_nsec;

    if(ts == NULL)
    {
        return;
    }

    add_nsec = (uint64_t)us * 1000ULL;
    ts->tv_sec += (time_t)(add_nsec / 1000000000ULL);
    ts->tv_nsec += (long)(add_nsec % 1000000000ULL);

    if(ts->tv_nsec >= 1000000000L)
    {
        ts->tv_sec += 1;
        ts->tv_nsec -= 1000000000L;
    }
}

/**********************************************************
 *
 *  timer_init()
 *
 *  DESCRIPTION:
 *      Initialize timer system
 *
 */

void timer_init()
{
    uint8_t i;
    pthread_mutex_lock(&sim_timer_lock);
    for(i = 0; i < SIM_MAX_TIMERS; i++)
    {
        sim_timers[i].allocated = FALSE;
        sim_timers[i].tick_rate = 0;
        sim_timers[i].irq_cb = NULL;
    }
    pthread_mutex_unlock(&sim_timer_lock);
}

/**********************************************************
 *
 *  timer_alloc()
 *
 *  DESCRIPTION:
 *      Allocate a timer.
 *
 */

timer_err_t8 timer_alloc(timer_id_t8 * timer_id, void_func_t irq_cb, uint32_t ticks)
{
    uint8_t i;
    sim_timer_ctrl_t * timer = NULL;

    if(NULL == timer_id || NULL == irq_cb || ticks == 0)
    {
        return TIMER_ERR_INVALID_PRMTRS;
    }

    pthread_mutex_lock(&sim_timer_lock);
    for(i = 0; i < SIM_MAX_TIMERS; i++)
    {
        if(sim_timers[i].allocated == FALSE)
        {
            sim_timers[i].allocated = TRUE;
            sim_timers[i].tick_rate = ticks;
            sim_timers[i].irq_cb = irq_cb;
            *timer_id = i;
            timer = &sim_timers[i];
            break;
        }
    }
    pthread_mutex_unlock(&sim_timer_lock);

    if(NULL == timer)
    {
        return TIMER_ERR_RESOURCE_UNAVAILABLE;
    }

    if (pthread_create(&timer->thread, NULL, simulate_shed_timer_isr, timer) != 0) {
        pthread_mutex_lock(&sim_timer_lock);
        timer->allocated = FALSE;
        timer->tick_rate = 0;
        timer->irq_cb = NULL;
        pthread_mutex_unlock(&sim_timer_lock);
        printf("Failed to create thread");
        return TIMER_ERR_RESOURCE_UNAVAILABLE;
    }

    /* Detached timer threads run for the life of the simulator. */
    pthread_detach(timer->thread);

    return TIMER_ERR_NONE;
}

void* simulate_shed_timer_isr(void* arg) {
    sim_timer_ctrl_t * timer = (sim_timer_ctrl_t *)arg;
    struct timespec next_wakeup;
    int sleep_err;

    if(NULL == timer)
    {
        return NULL;
    }

    if(clock_gettime(CLOCK_MONOTONIC, &next_wakeup) != 0)
    {
        return NULL;
    }

    while (1) {
        if(timer->irq_cb != NULL)
        {
            timer->irq_cb();
        }

        timespec_add_us(&next_wakeup, timer->tick_rate);

        do
        {
            sleep_err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_wakeup, NULL);
        } while(sleep_err == EINTR);

        if(sleep_err != 0)
        {
            usleep(TICKS_PER_USEC * timer->tick_rate);
            if(clock_gettime(CLOCK_MONOTONIC, &next_wakeup) != 0)
            {
                return NULL;
            }
        }
    }
    return NULL;
}

void delay_us(uint32_t us) {
    usleep(us);
}