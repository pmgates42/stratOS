/**********************************************************
 * 
 *  usb_hcd.h
 * 
 * DESCRIPTION:
 *      USB Host Controller Driver (HCD) spec. Declarations
 *      not defined in this file should be implemented by a
 *      hardware facing USB 2.0 driver and will be called by
 *      the USB 2.0 Core driver to perform ll operations.
 * 
 */

#include "usb.h"

/**
 * usb_hcd_power_on
 * 
 * @brief power on the USB hardware. This is required for
 * some systems.
 * 
 */
usb_err_t usb_hcd_power_on(void)
