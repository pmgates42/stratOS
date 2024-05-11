/**********************************************************
 * 
 *  bcm2xxx_mb.h
 * 
 *  DESCRIPTION:
 *      BCM 283x Mailbox interface definitions
 *
 */

#include "generic.h"
#include "bcm2xxx_mb.h"
#include "peripherals/base.h"
#include "printf.h"
#include "utils.h"
#include "../../drivers/usb/dwc2/dwc2.h"

#define MB_FULL   0x80000000
#define MB_EMPTY  0x40000000

typedef uint8_t mb_channel_t;
enum
{
    POWER_CHNL = 0,
};

// typedef struct
// {
//     reg32_t read_byt;
//     reg32_t __pad_0[5];
//     reg32_t status_byt;
//     reg32_t __pad_1[1];
//     reg32_t write_byt;
// } mb_reg_type;

#define REG_MB ((volatile uint32_t *)(PBASE + 0xB880))

static uint32_t current_mb_val;

/**
 * bcm2xx_mb_init
 * 
 * @brief Init module
 * 
 */
void bcm2xxx_mb_init(void)
{
current_mb_val = 0;

}


/**
 * bcm2xxx_power_on
 * 
 * @brief Power on hardware componen, i.e,. USB, SD, etc.
 * 
 * @param type of power on to perform
 */
void bcm2xxx_power_on(bcm2xxx_power_on_t type)
{
    //TODO
    /* block for now. put this in a task */
    while(REG_MB[6] & MB_FULL)
        ;

    current_mb_val |= type;
    REG_MB[8] = ( current_mb_val & ~( 0x0F ) ) | POWER_CHNL;

    delay(600);

    #define REG_VBUS *( ( volatile uint32_t * )HPRT0 )
    uint32_t hprt0 = REG_VBUS;
    hprt0 &= ~(HPRT0_ENA | HPRT0_CONNDET | HPRT0_ENACHG | HPRT0_OVRCURRCHG);
    if( !(hprt0 & HPRT0_PWR) )
    {
        printf("vbus NOT on. Turning on vbus\n");
        hprt0 |= HPRT0_PWR;
        REG_VBUS = hprt0;

        printf("vbus on");
    }
    else
    {
        printf("vbus on");
    }

    // printf("Successful power on of component %d", type);
}
