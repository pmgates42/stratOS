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
#include "utils.h"
#include "usb.h"
#include "dwc2.h"
#include "printf.h"

/* Register definitions */

#define REG_CORE_RESET *( ( volatile unsigned char * )GRSTCTL )
#define SOFT_RESET (0x1 << 0)
#define SOFT_RESET_TIMEOUT_CYCLES 500

/* static procs */

static boolean do_soft_reset(void);

/**
 * usb_hcd_init
 * 
 * @brief Contracted HCD function to initialize driver.
 * 
 * Perform the following operations on the DWC hardware:
 * 
 *  - Do a soft reset, i.e,. restart the firmware on the
 *    device.
 *  - Set up Direct Memory Addressing (DMA). This let's
 *    us share RX/TX buffers with the controller.
 *  - Enable USB IRQ.
 *  - 
 * 
 * @return usb_err_t 
 */
usb_err_t usb_hcd_init(void)
{
    if(!do_soft_reset())
    {
        printf("\nFailed to reset DWC2\n");
        return USB_ERR_HW;
    }

    return USB_ERR_NONE;
}

/**
 * Return TRUE if restart was successful
 * 
 */
static boolean do_soft_reset(void)
{
    uint8_t cycles = 0;

    REG_CORE_RESET = SOFT_RESET;

    while( ( REG_CORE_RESET & SOFT_RESET ) && ( cycles < SOFT_RESET_TIMEOUT_CYCLES ) )
    {
        delay(1);
        cycles++;
    }

    return (cycles < SOFT_RESET_TIMEOUT_CYCLES);
}