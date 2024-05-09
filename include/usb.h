/**********************************************************
 * 
 *  usb.h
 * 
 * DESCRIPTION:
 *      USB core header available for consumption by USB device
 *      drivers and kernel boot procedures.
 */

#pragma once

#include "generic.h"

typedef uint8_t usb_err_t;
enum
{
    USB_ERR_NONE,
    USB_ERR_HW,
};

usb_err_t usb_core_init(void);
