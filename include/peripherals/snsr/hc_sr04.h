/**********************************************************
 * 
 *  hc_sr04.c
 * 
 * 
 *  DESCRIPTION:
 *       HC-SR04 public interface
 *
 */

#pragma once

#include "generic.h"

typedef struct
{
    uint8_t trig;
    uint8_t echo;

} hc_sr04_config_t;