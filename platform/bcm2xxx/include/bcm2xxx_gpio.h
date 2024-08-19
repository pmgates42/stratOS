/**********************************************************
 * 
 *  bcm2xxx_gpio.h
 * 
 * 
 *  DESCRIPTION:
 *      User level GPIO interface.
 *
 */

#pragma once

#include "generic.h"
#include "peripherals/gpio.h"

typedef enum
{
    BCM2XXX_GPIO_FUNC_INPUT = 0,
    BCM2XXX_GPIO_FUNC_OUTPUT = 1,
    BCM2XXX_GPIO_FUNC_ALT0 = 4,
    BCM2XXX_GPIO_FUNC_ALT1 = 5,
    BCM2XXX_GPIO_FUNC_ALT2 = 6,
    BCM2XXX_GPIO_FUNC_ALT3 = 7,
    BCM2XXX_GPIO_FUNC_ALT4 = 3,
    BCM2XXX_GPIO_FUNC_ALT5 = 2

} bcm2xxx_gpio_fnc_t;  /* GPIO functon types */
