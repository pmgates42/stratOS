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
    USB_ERR_NONE,               /* No error */
    USB_ERR_HW,                 /* Hardware level error */
    USB_ERR_CORE_FAULT,         /* Core fault error */
};

usb_err_t usb_core_init(void);
