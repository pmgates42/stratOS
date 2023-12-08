/**********************************************************
 * 
 *  mini_uart.h
 * 
 * 
 *  DESCRIPTION:
 *      Defines interface to the rpi's mini
 *      uart (aka UART0)
 *
 *   NOTES:
 *      The mini UART is a dumb UART and has the following
 *      limitations:
 *      
 *      1. Lower baud rates when compared to other UARTs on rpi
 *      2. No RTS/CTS, i.e., no hardware flow control.
 *      3. Smaller buffers
 *      4. Shares clock with BCM2xxx
 * 
 */

#include "include/bcm2xxx_pvg_gpio.h"
#include "peripherals/aux.h"
#include "rpi_common.h"
#include "uart.h"

#define TX_PIN 14
#define RX_PIN 15

#define TX_READY_BIT (1 << 5)
#define RX_READY_BIT 1

/* static variables */
static boolean s_uart_init = FALSE; /* gaurd against bss clear fail */

/**********************************************************
 * 
 *  uart_init()
 * 
 * 
 *  DESCRIPTION:
 *      initialize the mini UART
 *
 */


void uart_init()
{
    if(s_uart_init)
    {
        return;
    }

    gpio_pin_set_func(TX_PIN, BCM2XXX_GPIO_FUNC_ALT5);
    gpio_pin_set_func(RX_PIN, BCM2XXX_GPIO_FUNC_ALT5);

    gpio_pin_enable(TX_PIN);
    gpio_pin_enable(RX_PIN);

    /* Enable the mini uart */
    REG_AUX_BASE->enables = 0x1;
    REG_AUX_BASE->mu_control = 0x0;
    REG_AUX_BASE->mu_ier = 0x0;
    REG_AUX_BASE->mu_lcr = 0x3;
    REG_AUX_BASE->mu_mcr = 0x0;

    /* The value of the baud rate is calculated as
     * baudrate = (system_clock_freq)/(8 * (baudrate_reg + 1))
     */
    REG_AUX_BASE->mu_baudrate = (int)((SYSTEM_CLOCK_FREQUENCY / (8 * BAUD_RATE)) - 1);

    /* Enable the TX/RX */
    REG_AUX_BASE->mu_control = 0x3;

    uart_send('\r');
    uart_send('\n');

    s_uart_init = TRUE;
}

/**********************************************************
 * 
 *  uart_is_init()
 * 
 * 
 *  DESCRIPTION:
 *      TX data via the mini-uart
 *
 */

boolean uart_is_init(void)
{
    return s_uart_init;
}


/**********************************************************
 * 
 *  uart_send()
 * 
 * 
 *  DESCRIPTION:
 *      TX data via the mini-uart
 *
 */

void uart_send(char c)
{
    /* wait until the TX buf accept at least one byte */
    while(0 == (REG_AUX_BASE->mu_lsr & TX_READY_BIT) )
            ;

    /* set the byte in the IO reg */
    REG_AUX_BASE->mu_io = c;
}


/**********************************************************
 * 
 *  uart_recv()
 * 
 * 
 *  DESCRIPTION:
 *      RX data from the mini-uart buf
 *
 */

uint8_t uart_recv(void)
{
    /* wait until the TX buf accept at least one byte */
    while(0 == (REG_AUX_BASE->mu_lsr & RX_READY_BIT ) )
            ;

    /* Read the available byte from the buf */
    return( REG_AUX_BASE->mu_io & 0xFF );
}


void uart_send_string(char *str)
{
    while(*str)
    {   if(*str == '\n')
            uart_send('\r');

        uart_send(*str);
        str++;
    }
}