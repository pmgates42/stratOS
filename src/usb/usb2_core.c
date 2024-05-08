/**********************************************************
 * 
 *  usb2_core.c
 * 
 * DESCRIPTION:
 *      USB 2.0 core driver
 * 
 * NOTES:
 *      This driver is hardware agnostic.
 * 
 */

#include "usb.h"
#include "cpu.h"

/**
 * usb_core_init
 * 
 * @brief Initialize the USB 2.0 core driver
 * 
 * @return usb_err_t 
 */
usb_err_t usb_core_init(void)
{
cpu_power_on_usb();

return USB_ERR_NONE;
}