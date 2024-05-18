/**********************************************************
 * 
 *  cpu.h
 * 
 */

#include "generic.h"
#include "cpu_impl.h"

/**********************************************************
 * 
 * CYCLES_PER_US
 * 
 * DESCRIPTION:
 *      Cycles per microsecond.
 * 
 * NOTES:
 *      CPU must define SYSTEM_CLOCK_FREQUENCY. Dynamic clock
 *      frequency is NOT supported at this time.
 * 
 */

#define CYCLES_PER_US ((uint32_t)SYSTEM_CLOCK_FREQUENCY / 1000000)


/**********************************************************
 * 
 * cpu_power_on_usb()
 * 
 * DESCRIPTION:
 *      Many USB controllers live on the CPUs SoC and need
 *      to be powered. Returning false here will simply mean that
 *      the power on was not successful and will not necessarily
 *      inhibit use of a USB controller. Check specs for what is
 *      necessary for your system.
 * 
 * NOTES:
 *      true Power on of USB was successful
 *      false Power on of USB was not successful
 * 
 */

boolean cpu_power_on_usb(void);


/**********************************************************
 * 
 * cpu_init()
 * 
 * DESCRIPTION:
 *      Do any CPU specific initialization. Called on kernel
 *      entry.
 * 
 */

boolean cpu_init(void);