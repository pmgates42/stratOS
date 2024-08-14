/**********************************************************
 * 
 *  bcm2xxx_mb.h
 * 
 *  DESCRIPTION:
 *      BCM 283x Mailbox interface
 *
 */

#pragma once

#include "generic.h"

typedef uint8_t bcm2xxx_power_on_t;
enum
{
    BCM_2XXX_POWER_ON_TYPE_USB = (0x1 << 7)
};

void bcm2xxx_mb_init(void);
void bcm2xxx_power_on(bcm2xxx_power_on_t type);
