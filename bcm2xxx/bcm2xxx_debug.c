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
#include "bcm2xxx_pvg_gpio.h"

#ifndef BCM2XXX_DEBUG_LED_PIN
#define BCM2XXX_DEBUG_LED_PIN 23
#warning !!! Setting default debug LED pin (BCM2XXX_DEBUG_LED_PIN) to GPIO 23 please update if necessary !!!
#endif

void debug_set_led(void)
{
    /* set the debug led */
    gpio_pin_set_func(BCM2XXX_DEBUG_LED_PIN, BCM2XXX_GPIO_FUNC_OUTPUT);
    gpio_pin_enable(BCM2XXX_DEBUG_LED_PIN);
    gpio_set(BCM2XXX_DEBUG_LED_PIN);
}