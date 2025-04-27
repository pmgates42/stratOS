/**********************************************************
 * 
 *  bcm2xxx_debug.c
 * 
 * 
 *  DESCRIPTION:
 *      Debug utilities
 *
 */

#include "peripherals/gpio.h"
#include "bcm2xxx_gpio.h"

#ifndef BCM2XXX_DEBUG_LED_PIN
    #define BCM2XXX_DEBUG_LED_PIN 23
    #warning !!! Setting default debug LED pin (BCM2XXX_DEBUG_LED_PIN) to GPIO 23 please update if necessary !!!
#endif

void debug_init(void)
{
    /* Initialize debug LED */
    gpio_pin_set_func(BCM2XXX_DEBUG_LED_PIN, BCM2XXX_GPIO_FUNC_OUTPUT);
    gpio_pin_enable(BCM2XXX_DEBUG_LED_PIN);
}

void debug_set_led(void)
{
    gpio_set(BCM2XXX_DEBUG_LED_PIN);
}

void debug_clr_led(void)
{
    gpio_clr(BCM2XXX_DEBUG_LED_PIN);
}

void debug_toggle_led(void)
{
    if(gpio_get(BCM2XXX_DEBUG_LED_PIN))
    {
        gpio_clr(BCM2XXX_DEBUG_LED_PIN);
    }
    else
    {
        gpio_set(BCM2XXX_DEBUG_LED_PIN);
    }
}