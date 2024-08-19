/**********************************************************
 * 
 *  cpu_impl.h
 * 
 *  DESCRIPTION:
 *      Contains common definitions and interfaces
 *      for the Raspberry Pi.
 */

#pragma once

#include "generic.h"

#if RPI_VERSION == 3
    #define SYSTEM_CLOCK_FREQUENCY 250000000
#elif RPI_VERSION == 4
    #define SYSTEM_CLOCK_FREQUENCY 5
#else
    #error "Unsupported RPI_VERSION"
#endif

#define RPI_3B_PLUS 1

#ifdef RPI_SUB_VERSION
    #if RPI_SUB_VERSION == RPI_3B_PLUS
        #define MU_BUAD_RATE 115200
    #else
        #define MU_BUAD_RATE 115200
    #endif
    #warning "Only the Raspberry Pi 3B+ is supported and tested at this time"
#else
    #define MU_BUAD_RATE 115200
    #warning "Only the Raspberry Pi 3B+ is supported and tested at this time"
#endif
