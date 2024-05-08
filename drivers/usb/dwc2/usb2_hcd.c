/**********************************************************
 * 
 *  usb2_hcd.c
 * 
 * DESCRIPTION:
 *      Host Controller Driver (HCD) for Synopsys DesignWare
 *      Hi-Speed USB 2.0 On-The-Go Controller.
 * 
 * NOTES:
 *      There is limited documentation for the USB Controller
 *      hardware interface. This driver was written by reverse
 *      engineering the Raspberry Pi Linux kernel and the
 *      Embedded Xinu project.
 * 
 *      The public functions in this file are contracted in the
 *      StratOS usb_hcd.h file.
 * 
 */

#include "generic.h"
#include "usb.h"

/**
 * usb_hcd_power_on
 * 
 * @brief Contracted HCD function to power on the USB Controller
 * 
 * @return usb_err_t 
 */
usb_err_t usb_hcd_power_on(void)
{

}