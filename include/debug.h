/**********************************************************
 * 
 *  debug.h
 * 
 * 
 *  DESCRIPTION:
 *      HW common debug definitions
 *
 */

#pragma once

void debug_init(void);
void debug_set_led(void);
void debug_clr_led(void);
void debug_toggle_led(void);
void debug_printf(char *fmt, ...);
