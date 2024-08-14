/**********************************************************
 * 
 *  sim_cpu.c
 * 
 *  DESCRIPTION:
 *     Simulated CPU definitions
 *
 */

#include "generic.h"

/**********************************************************
 * 
 * cpu_init()
 * 
 * DESCRIPTION:
 *      Do any CPU specific initialization. Called on kernel
 *      entry.
 * 
 */

boolean cpu_init(void)
{
    return TRUE;
}

/**********************************************************
 * 
 * get_el()
 * 
 * DESCRIPTION:
 *      Simulated procedure to get Execution Level (EL)
 * 
 */

uint32_t get_el(void)
{
    return 0;
}
