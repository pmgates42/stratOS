/**********************************************************
 * 
 *  snsr_manager.c
 * 
 * 
 *  DESCRIPTION:
 *      Manages all sensor I/O for the system
 *
 *  NOTES:
 *      Sensors must be configured within the system before they
 *      can be managed by this module.
 *
 */


#include "config.h"
#include "peripherals/snsr/snsr.h"
#include "debug.h"

// todo stubbing sensor config while flash code is incomplete
static kernel_config_t stubbed_config = {0};

config_err_t8 config_get_sys_config(kernel_config_t * config)
{
    stubbed_config.snsr_configs[0].hw_type = SNSR_HW_HCSR04;
    stubbed_config.snsr_configs[0].hw_config.hc_sr04.echo = 24;
    stubbed_config.snsr_configs[0].hw_config.hc_sr04.trig = 25;

    stubbed_config.num_snsrs = 1;

    memcpy(config, &stubbed_config, sizeof(kernel_config_t));

    return CONFIG_ERR_NONE;

}
