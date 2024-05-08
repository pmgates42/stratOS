/**********************************************************
 * 
 *  usb.h
 * 
 * DESCRIPTION:
 *      USB core header available for consumption by USB device
 *      drivers and kernel boot procedures.
 */

#include "generic.h"

typedef uint8_t usb_err_t;
enum
{
    USB_ERR_NONE,
};

usb_err_t usb_core_init(void);
