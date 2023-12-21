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

// todo stubbing sensor config while flash code is incomplete
static kernel_config_t stubbed_config;

config_err_t8 config_get_sys_config(kernel_config_t *config)
{
    snsr_config_t hc_sr04_snsr_cfg;

    hc_sr04_snsr_cfg.hw = SNSR_HW_HCSR04;
    hc_sr04_snsr_cfg.hw_config.hc_sr04.echo = 24;
    hc_sr04_snsr_cfg.hw_config.hc_sr04.trig = 25;

    stubbed_config.snsr_configs[0] = hc_sr04_snsr_cfg;
    stubbed_config.num_snsrs = 1;

    config = &stubbed_config;

    return CONFIG_ERR_NONE;

}
