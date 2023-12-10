/**********************************************************
 * 
 *  sched.h
 * 
 * 
 *  DESCRIPTION:
 *      Timer interface
 *
 */

#pragma once

#include "generic.h"

typedef uint8_t timer_id_t8;

typedef uint8_t timer_err_t8;
enum
    {
    TIMER_ERR_NONE,
    TIMER_ERR_RESOURCE_UNAVAILABLE,
    TIMER_ERR_INVALID_PRMTRS,
    };

void timer_init();

timer_err_t8 timer_alloc(timer_id_t8 * timer_id, void_func_t irq_cb, uint32_t ticks);

