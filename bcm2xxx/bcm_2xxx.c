/**********************************************************
 * 
 *  bcm_2xxx_soc.c
 * 
 */

#include "generic.h"
#include "bcm2xxx_mb.h"
#include "bcm2xxx_mb.h"

/**
 * cpu_power_on_usb
 * @brief Contracted CPU function
 * 
 * @return true Power on of USB was successful
 * @return false Power on of USB was not successful
 */
boolean  cpu_power_on_usb(void)
{
bcm2xxx_power_on(BCM_2XXX_POWER_ON_TYPE_USB);

return TRUE;
}

boolean cpu_init(void)
{
bcm2xxx_mb_init();

return TRUE;
}