/**********************************************************
 * 
 *  sim_uart.c
 * 
 *  DESCRIPTION:
 *     Simulated UART definitions
 *
 */

#include "generic.h"

static initialized;

/**********************************************************
 * 
 * uart_init()
 * 
 * DESCRIPTION:
 *      Initialize UART
 * 
 */

void uart_init(void)
{
    initialized = TRUE;
}

/**********************************************************
 * 
 * uart_is_init()
 * 
 * DESCRIPTION:
 *      UART driver has been initialized
 * 
 */

boolean uart_is_init(void)
{
    return initialized;
}
