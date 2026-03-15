/**********************************************************
 * 
 *  sim_debug.c
 * 
 *  DESCRIPTION:
 *      OS simulator debug module. Right now hardware debug
 *      operations are stubbed out but ideally some day there
 *      will be simulator front ends we can communicate with
 *
 */

#include "generic.h"
#include "printf.h"
#include <stdarg.h>

void debug_init(void)
{

}

void debug_set_led(void)
{

}

void debug_clr_led(void)
{

}

void debug_toggle_led(void)
{

}

void debug_printf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}