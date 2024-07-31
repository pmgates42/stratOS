// unit_test_ssched.c
#include <stdio.h>
#include "ssched.c"

int main() {
    // Your test code here
    printf("Running unit tests...\n");
    return 0;
}

/* mocked functions */

timer_err_t8 timer_alloc(timer_id_t8 * timer_id, void_func_t irq_cb, uint32_t ticks)
{
return TIMER_ERR_NONE;
}

void uart_init()
{
}

boolean uart_is_init()
{
return TRUE;
}