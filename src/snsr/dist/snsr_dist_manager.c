/**********************************************************
 * 
 *  snsr_dist_manager.c
 * 
 * 
 *  DESCRIPTION:
 *      Manages all sensor distance sensor.
 *
 */

#include "generic.h"
#include "peripherals/snsr/snsr.h"
#include "debug.h"
#include "config.h"
#include "peripherals/drivers/hc_sr04.h"

static snsr_err_t8 init_snsr(snsr_cb_t * snsr);

/**********************************************************
 * 
 *  snsr_dist_manager_init()
 * 
 * 
 *  DESCRIPTION:
 *      Initialize distance sensor manager
 *
 */

void snsr_dist_manager_init(void)
{

}

/**********************************************************
 * 
 *  snsr_dist_register_snsr()
 * 
 * 
 *  DESCRIPTION:
 *      Register distance sensor
 *
 */

snsr_err_t8 snsr_dist_register_snsr(snsr_cb_t * snsr)
{
    /* input validation */
    if(NULL == snsr )
    {
        return SNSR_ERR_INVLD_PARMS;
    }
 
    if(SNSR_TYPE_DIST != snsr_get_snsr_type(snsr->config.hw_type))
    {
        snsr->registered = FALSE;
        return SNSR_ERR_INVLD_TYPE;
    }

    init_snsr(snsr);
    snsr->registered = TRUE;
    
    return SNSR_ERR_NONE;

}

/**********************************************************
 * 
 *  init_snsr()
 * 
 * 
 *  DESCRIPTION:
 *      Initialize a distacne sensor
 *
 */

static snsr_err_t8 init_snsr(snsr_cb_t * snsr)
{
    /* input validtion */
    if(NULL == snsr)
    {
        return SNSR_ERR_INVLD_PARMS;
    }

    /* call the appropriate initialization procedure */
    switch (snsr->config.hw_type)
    {
    /* initialize the HC-SR04 ultrasonic sensor */
    case SNSR_HW_HCSR04:
        snsr_config_t tmp;
        hc_sr04_register_snsr(tmp);//todo mem failure here?
        break;

    /* invalid hardware type */
    default:
        break;
    }

    return SNSR_ERR_NONE;
}