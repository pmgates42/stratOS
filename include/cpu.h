/**********************************************************
 * 
 *  cpu.h
 * 
 */

#include "generic.h"

/**
 * cpu_power_on_usb
 * @brief Many USB controllers live on the CPUs SoC and need
 * to be powered. Returning false here will simply mean that
 * the power on was not successful and will not necessarily
 * inhibit use of a USB controller. Check specs for what is
 * necessary for your system.
 * 
 * @return true Power on of USB was successful
 * @return false Power on of USB was not successful
 */
boolean cpu_power_on_usb(void);

/**
 * cpu_init
 * 
 * @brief Do any CPU specific initialization. Called on
 * kernel entry.
 * 
 * @return boolean 
 */
boolean cpu_init(void);