#pragma once

#include "generic.h"


/**********************************************************
 * 
 *  SIMULATOR_MAIN_TASK_PERIOD
 * 
 *  DESCRIPTION:
 *     Defines the period that gpio_maintenance_task should
 *     run. This value is basically the bottleneck to
 *     detecting input or sending output to our simulated
 *     hardware device.
 * 
 *     Ideally, it would run as fast as possible in order to
 *     be as close as possible to "real" hardware, but we also
 *     perform file read/write operations so its duty cycle
 *     needs to be long enough to complete these tasks.
 *
 */
#define SIMULATOR_MAINT_TASK_PERIOD_MS  500

#define HARDWARE_MAINT_TASK_PERIOD_MS   0  /* not implemented */

void gpio_pin_set_func(uint32_t pin, uint8_t fnc);
void gpio_pin_setas_outp(uint32_t pin);
void gpio_pin_setas_inp(uint32_t pin);
void gpio_pin_enable(uint32_t pin);
void gpio_set(uint32_t pin);
void gpio_clr(uint32_t pin);
void gpio_maintenance_task();
boolean gpio_get(uint32_t pin);
