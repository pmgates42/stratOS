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
#include "irq.h"


/* Register definitions */

#define REG_CORE_RESET *( ( volatile uint32_t * )GRSTCTL )
#define SOFT_RESET (0x1 << 0)
#define SOFT_RESET_TIMEOUT_CYCLES 500

#define REG_GBL_RX_FIFO_SZ *( ( volatile uint32_t * )GRXFSIZ )
#define RX_FIFO_BUF_SZ ( 12 * 1024 )                /* 12 KB */

#define REG_GBL_NP_TX_FIFO_SZ *( ( volatile uint32_t * )GNPTXFSIZ )
#define TX_FIFO_BUF_SZ ( 12 * 1024 )                /* 12 KB */

#define REG_HOST_PRDC_TX_FIFO_SZ *( ( volatile uint32_t * )HPTXFSIZ )
#define HOST_PRDC_TX_FIFO_BUF_SZ ( 12 * 1024 )      /* 12 KB */

#define REG_AHB_CFG *( ( volatile uint32_t * )GAHBCFG )

#define REG_GBL_INTR_MSK *( ( volatile uint32_t * )GINTMSK )
#define REG_GBL_INTR_STS *( ( volatile uint32_t * )GINTSTS )

#define EN_HOST_CHNL_INTR ( 0x1 << 25 )
#define EN_HOST_PORT_INTR ( 0x1 << 25 )
#define EN_AHB_INTR ( 0x1  << 0 )

#define REG_VENDOR_ID *( ( volatile uint32_t * )GSNPSID )

/* static procs */

static boolean do_soft_reset(void);
static void setup_dma(void);
static boolean setup_intr(void);

/**
 * usb_hcd_init
 * 
 * @brief Contracted HCD function to initialize driver.
 * 
 * Perform the following operations on the DWC hardware:
 * 
 *  - Do a soft reset, i.e,. restart the firmware on the
 *    device.
 *  - Set up Direct Memory Addressing (DMA).
 *  - Enable USB IRQ.
 * 
 * @return usb_err_t 
 */

usb_err_t usb_hcd_init(void)
{
    /* Verify vendor id */
    if(REG_VENDOR_ID != 0x4F54280A)
    {
        /* We are using this driver with the improper
         * hardware or there is an invalid peripheral base
         * address. In any case, something has gone wrong
         * with the system configuration */
        #ifdef DWC2_SHOW_DEBUG_DATA
            printf("Invalid vendor id! Expected 0x4F54280A, got %d", REG_VENDOR_ID);
        #endif
        return USB_ERR_INVLD_CNFG;
    }

    /* print vendor id */
    printf("DWC2 Vendor id %d", REG_VENDOR_ID);

    if(!do_soft_reset())
    {
        #ifdef DWC2_SHOW_DEBUG_DATA
            printf("\nFailed to reset DWC2\n");
        #endif
        return USB_ERR_INVLD_STATE;
    }

    setup_dma();

    if(!setup_intr())
    {
        #ifdef DWC2_SHOW_DEBUG_DATA
            printf("\nFailed to set up interrupts DWC2\n");
        #endif
        return USB_ERR_INVLD_STATE;
    }

    return USB_ERR_NONE;
}

/**
 * do_soft_reset
 * 
 * @brief Initiate a software reset on the USB controller
 * and wait for it to indicate completion. Timeout after
 * SOFT_RESET_TIMEOUT_CYCLES cycles.
 * 
 * @return TRUE if restart was successful FALSE otherwise
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

/**
 * setup_dma
 * 
 * @brief set up DMA. Allows controller to access the
 * buffers directly.
 * 
 * @note There are three types of buffers that the USB
 * controller can use:
 * 
 *  - RX buffer
 *      Contains data recieved by other devices that
 *      need to be processed.
 * 
 *  - Non-periodic TX buffer
 *      Used to transfer non-periodic data, i.e., bulk
 *      and control communications
 * 
 * - Host periodic TX buffer
 *      Periodic buffer used for isochronous (stream)
 *      transfers
 * 
 */

static void setup_dma(void)
{
    /* Set the RX buffer size */
    REG_GBL_RX_FIFO_SZ = RX_FIFO_BUF_SZ;

    /*  Low 16 bits are from start of FIFO buffer reserved memory.
    *  High bits are for non-periodic TX buffer size.
    */
    REG_GBL_NP_TX_FIFO_SZ =  ( TX_FIFO_BUF_SZ << 16 ) | RX_FIFO_BUF_SZ;

    /*  Low 16 bits are from start of FIFO buffer reserved memory.
    *  High bits are for host periodic TX buffer size.
    */
    REG_HOST_PRDC_TX_FIFO_SZ = ( HOST_PRDC_TX_FIFO_BUF_SZ << 16 ) | ( TX_FIFO_BUF_SZ + RX_FIFO_BUF_SZ );

    /* Enable DMA */
    REG_AHB_CFG |= GAHBCFG_DMA_EN;

    printf("\nDMA enabled\n");
}

/**
 * setup_intr 
 *
 * @brief Set up USB interrupts
 * 
 */
static boolean setup_intr(void)
{
    /* Clear any pending interrupts */
    REG_GBL_INTR_MSK = 0;
    REG_GBL_INTR_STS = BS_ALL_32;

    /* Enable interrupts on the controller */
    REG_GBL_INTR_MSK |= EN_HOST_CHNL_INTR | EN_HOST_PORT_INTR;

    /* Enable IRQs on this CPU */
    if( !irq_enable_usb() )
    {
        return FALSE;
    }

    /* Enable interrupts on AHB config reg */
    REG_AHB_CFG |= EN_HOST_CHNL_INTR;

    return TRUE;
}
