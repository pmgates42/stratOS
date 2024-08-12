/**********************************************************
 * 
 *  bcm2xxx_timer.h
 * 
 * 
 *  DESCRIPTION:
 *      BCM2XXX Timer interface
 *
 */


#pragma once

#include "generic.h"

typedef uint8_t bcm2xxx_timer_t8;
enum
    {
    BM2XXX_TIMER_CHNL_0,
    BM2XXX_TIMER_CHNL_1,
    BM2XXX_TIMER_CHNL_2,
    BM2XXX_TIMER_CHNL_3,
    BCMXXX_TIMER_CHNL_COUNT,
    };

void bcm2xxx_timer_irq_hndlr(bcm2xxx_timer_t8 timer);