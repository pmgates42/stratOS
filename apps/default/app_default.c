#include "generic.h"
#include "application/app_interface.h"
#include "uapi/strat_os_sched.h"
#include "peripherals/spi.h"

static const STRAT_OS_task_type app_tasks[] =
{
    { 30, spi_tx_periodic }
};

/* Weak default hook: applications can provide a strong APP_sched_configure(). */
#if defined(__GNUC__)
__attribute__((weak))
#endif
boolean APP_sched_configure(void)
{
    return (STRAT_OS_SCHED_init(app_tasks, list_cnt(app_tasks)) == STRAT_OS_SCHED_ERR__NO_ERROR);
}
