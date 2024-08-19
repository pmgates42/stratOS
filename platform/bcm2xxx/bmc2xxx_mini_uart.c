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

#include "include/bcm2xxx_gpio.h"
#include "peripherals/aux.h"
#include "cpu_impl.h"
#include "uart.h"

#define BASE 10

#define TX_PIN 14
#define RX_PIN 15

#define TX_READY_BIT (1 << 5)
#define RX_READY_BIT 1

#define DIGIT_TO_CHAR(digit) (digit + '0')
                        /* Convert uint8 digit to ASCII character */

/* static variables */
static boolean s_uart_init = FALSE;

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
    REG_AUX_BASE->mu_ier = 0xD; /* Bits 3:2 are NOT don't care: https://elinux.org/BCM2835_datasheet_errata#p12 */
    REG_AUX_BASE->mu_lcr = 0x3;
    REG_AUX_BASE->mu_mcr = 0x0;

    #if !(RPI_SUB_VERSION == RPI_3B_PLUS)
        #error "Only 3B+ support currently exists
    #endif

    #if !(MU_BUAD_RATE == 115200)
        #error "Invalid baud rate!"
    #endif

    #if !(SYSTEM_CLOCK_FREQUENCY == 250000000)
        #error "Invalid System Clock Frequency!"
    #endif

    /* Baud rate = (SYSTEM_CLOCK_FREQUENCY / (8 * BAUD_RATE)) - 1; */
    REG_AUX_BASE->mu_baudrate = REG_AUX_BASE->mu_baudrate = (unsigned int)((SYSTEM_CLOCK_FREQUENCY / (8 * MU_BUAD_RATE)) - 1);

    /* Enable the TX/RX */
    REG_AUX_BASE->mu_control = 0x3;

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

    /* If this is a newline, add carriage return */
    if(c == '\n')
    {
        uart_send('\r');
    }
}

/**********************************************************
 * 
 *  uart_send_uint32()
 * 
 * 
 *  DESCRIPTION:
 *      TX uint32
 *
 */

void uart_send_uint32(uint32_t n)
{
    /* buffer to store 10 chars and null terminator */
    char buffer[12];
    int index = 0;

    if (n == 0)
    {
        uart_send('0');
        return;
    }

    /* convert the uint32_t to ASCII in reverse order */
    while (n > 0)
    {
        uint8_t digit = n % BASE;
        buffer[index++] = DIGIT_TO_CHAR(digit);
        n /= BASE;
    }

    /* Send each character over UART in reverse order */
    for (int i = index - 1; i >= 0; i--)
    {
        uart_send(buffer[i]);
    }
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


/**********************************************************
 * 
 *  uart_send_string()
 * 
 * 
 *  DESCRIPTION:
 *      Send a string over UART
 *
 */

void uart_send_string(char* str)
{
	for (int i = 0; str[i] != '\0'; i ++) {
		uart_send((char)str[i]);
	}
}

/**********************************************************
 * 
 *  putc()
 * 
 * 
 *  DESCRIPTION:
*       Contracted printf function
 *
 */

void putc (void* p, char c)
{
    (void)p;
	uart_send(c);
}