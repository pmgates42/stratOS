#include "generic.h"
#include "application/app_interface.h"

#include "app_test.h"
#include "uapi/strat_os_core_utils.h"

void APP_test_setup(void)
{
}

void APP_test_teardown(void)
{
}

static boolean APP_core_utils_test_configure(void)
{
    return TRUE;
}

boolean APP_sched_configure(void)
{
    return APP_core_utils_test_configure();
}
