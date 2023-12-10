#include "generic.h"
#include "uart.h"
#include "sched.h"
#include "irq.h"
#include "peripherals/timer.h"

static void tty_task(void);

static sched_usr_tsk_t task_list[] =
    {
    { 10 /* ms */, tty_task }
    };

/* Called from boot.S, this is just for testing rn */
void kernel_main()
{
    /* Initialize hardware modules */
    uart_init();
    //uart_send_string("Kernel initializing\n");

    irq_init();
    timer_init();

    /* Initialize OS level modules */
    sched_init( task_list, 1 );

    /* echo back user input */
    // while(1)
    //     {
    //     uart_send_string("Kernel initializing\n");
    //     uart_send(uart_recv());
    //     }

    uart_send_string("Kernel Initializing");

    while(1)
        {
        uart_send(uart_recv());
        }
}

static void tty_task(void)
{
    /* echo back user input */
    while(1)
        {
        uart_send('G');
        }
}