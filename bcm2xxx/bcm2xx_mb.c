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

#define MB_FULL   0x80000000
#define MB_EMPTY  0x40000000

typedef uint8_t mb_channel_t;
enum
{
    POWER_CHNL = 0,
};

typedef struct
{
    reg32_t read_byt;
    reg32_t __pad_0[5];
    reg32_t status_byt;
    reg32_t __pad_1[1];
    reg32_t write_byt;
} mb_reg_type;

#define REG_MB ((volatile mb_reg_type *)(PBASE + 0xB880))

static uint32_t current_mb_val;

/**
 * bcm2xx_mb_init
 * 
 * @brief Init module
 * 
 */
void bcm2xxx_mb_init(void)
{
current_mb_val = REG_MB->write_byt;

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
    while(REG_MB->status_byt & MB_FULL)
        ;

    //TODO abstract into helper function to set any channel
    if( !(current_mb_val & type) )
    {
        current_mb_val |= type;
        REG_MB->write_byt = ( current_mb_val & ~( 0x0F ) ) | POWER_CHNL;
    }

    printf("\nSuccessful power on of component %d", type);
}

