/**********************************************************
 * 
 *  sim_usb.c
 * 
 *  DESCRIPTION:
 *      Simulated USB definitions
 *
 *      NOT IMPLEMENTED.
 */

#include "generic.h"
#include "usb.h"

boolean cpu_power_on_usb(void)
{
    return FALSE;
}

/**********************************************************
 * 
 *  usb_hcd_init()
 * 
 *  DESCRIPTION:
 *     Contracted HCD function to initialize driver.
 *
 *      NOT IMPLEMENTED.
 */

usb_err_t usb_hcd_init(void)
{
    return USB_ERR_INVLD_STATE;
}