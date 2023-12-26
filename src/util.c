/**********************************************************
 * 
 *  util.c
 * 
 * 
 *  DESCRIPTION:
 *      Commonly available utilities
 *
 */

#include "utils.h"
#include "generic.h"
#include "rpi_common.h"

void delay_us(uint64_t ms)
{
    /* Calculate the number of cycles needed for the delay */
    uint32_t cycles = (uint32_t)((uint64_t)SYSTEM_CLOCK_FREQUENCY * ms / 1000000);

    /* delay */
    delay(cycles);

}