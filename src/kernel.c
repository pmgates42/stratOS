#include "generic.h"
#include "uart.h"
#include "sched.h"

static void tty_task(void);

static sched_usr_tsk_t task_list[] =
    {
    { 10 /* ms */, tty_task }
    };

/* Called from boot.S, this is just for testing rn */
void kernel_main()
{
    /* Initialize the task scheduler */
    sched_init( task_list, 1 );
 
    uart_init();
    uart_send_string("Kernel initializing");

}

static void tty_task(void)
{
    /* echo back user input */
    while(1)
        {
        uart_send(uart_recv());
        }
}