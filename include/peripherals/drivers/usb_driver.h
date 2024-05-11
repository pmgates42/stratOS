/**********************************************************
 * 
 *  usb_driver.h
 * 
 * 
 */

#pragma once

#include "generic.h"

#define MAX_NUM_OF_IN_ENDPOINTS 16

/* Types */

typedef uint8_t usb_driver_id_t;

typedef struct
{
    usb_driver_id_t id;     /* Unique identifier */
    boolean bound;          /* driver is bound to a device */
} usb_driver_t;