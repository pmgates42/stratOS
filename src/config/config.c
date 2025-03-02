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

#define INVALID_MODULE_ENTRY_IDX  0xFF
#define INVALID_MODULE_PIN_IDX    0xFF

// todo stubbing sensor config while flash code is incomplete
static kernel_config_t stubbed_config = {0};

static pin_config_type registered_module_pin_configs[CFG_PIN_CFG_MAX_RGSTRD];
static uint8_t pin_cfgs_cnt;

config_err_t8 config_get_sys_config(kernel_config_t * config)
{
    stubbed_config.snsr_configs[0].hw_type = SNSR_HW_HCSR04;
    stubbed_config.snsr_configs[0].hw_config.hc_sr04.echo = 24;
    stubbed_config.snsr_configs[0].hw_config.hc_sr04.trig = 25;

    stubbed_config.num_snsrs = 1;

    memcpy(config, &stubbed_config, sizeof(kernel_config_t));

    return CONFIG_ERR_NONE;

}


/**********************************************************
 * 
 *  config_module_init()
 * 
 *  DESCRIPTION:
 *     Initialize the config module config lists
 *
 */

config_err_t8 config_module_init(void)
{
    uint8_t i;

    pin_cfgs_cnt = 0;
    clr_mem( registered_module_pin_configs, sizeof registered_module_pin_configs);

    for( i = 0; i < list_cnt( registered_module_pin_configs ); i++ )
    {
        registered_module_pin_configs[i].module_id = CFG_INVALID_MODULE_ID;

        for( uint8_t j = 0; j < list_cnt( registered_module_pin_configs[i].pins ); j++ )
        {
            registered_module_pin_configs[i].pins[j].pin_number = CFG_INVALID_PIN_NUMBER;
            registered_module_pin_configs[i].pins[j].id         = CFG_INVALID_PIN_ID;
        }
    }

    return CONFIG_ERR_NONE;
}

/**********************************************************
 * 
 *  config_register_pins_for_module()
 * 
 *  DESCRIPTION:
 *     Register GPIO pins for a given module
 *
 */

config_err_t8 config_register_pins_for_module(config_module_id_type module_id, uint16_t pin, config_pin_id_type id)
{
    uint8_t         i;
    uint8_t         entry_index;
    uint8_t         pin_index;

    if( pin >= CFG_INVALID_PIN_NUMBER )
    {
        return CONFIG_ERR_INVLD_CONFIG; 
    }

    if( module_id >= CFG_MAX_MODULES)
        return CONFIG_ERR_INVLD_MODULE_ID;

    /* look for an existing entry for this module */
    entry_index = INVALID_MODULE_ENTRY_IDX;
    for( i = 0; i < list_cnt( registered_module_pin_configs ); i++ )
    {
        if( module_id == registered_module_pin_configs[i].module_id )
            {
            entry_index = i;
            break;
            }
    }

    /* create a new entry if necessary */
    if( entry_index == INVALID_MODULE_ENTRY_IDX )
    {
        if( pin_cfgs_cnt >= CFG_PIN_CFG_MAX_RGSTRD )
        {
            return CONFIG_ERR_REACHED_MAX_PIN_CFGS;
        }
        else
        {
            registered_module_pin_configs[ pin_cfgs_cnt ].module_id = module_id;

            entry_index = pin_cfgs_cnt;
            pin_cfgs_cnt++;    
        }
    }

    pin_index = INVALID_MODULE_PIN_IDX;
    for( i = 0; i < list_cnt( registered_module_pin_configs[ entry_index ].pins ); i++ )
    {
        if( registered_module_pin_configs[ entry_index ].pins[ i ].pin_number == CFG_INVALID_PIN_NUMBER )
        {
            pin_index = i;
            break;
        }
    }
    
    if( pin_index != INVALID_MODULE_PIN_IDX )
    {
        registered_module_pin_configs[ entry_index ].pins[ pin_index ].pin_number = pin;
        registered_module_pin_configs[ entry_index ].pins[ pin_index ].id         = id;
    }
    else
    {
        return CONFIG_ERR_MODULE_REACHED_MAX_PINS;
    }

    return CONFIG_ERR_NONE;

}

/**********************************************************
 * 
 *  config_pin_is_registered()
 * 
 *  DESCRIPTION:
 *     Check if pin with pin_id is registered to the given
 *     module. 
 *
 */

boolean config_pin_is_registered(config_module_id_type module_id, config_pin_id_type pin_id)
{
    uint8_t i;
    uint8_t module_index = INVALID_MODULE_ENTRY_IDX;

    for( i = 0; i < list_cnt(registered_module_pin_configs); i++ )
    {
        if( module_id == registered_module_pin_configs[i].module_id)
            {
            module_index = i;
            break;
            }
            
    }

    /* Couldn't find module */
    if(module_id == INVALID_MODULE_ENTRY_IDX)
        return FALSE;

    for( i = 0; i < list_cnt(registered_module_pin_configs[module_id].pins); i++ )
    {
        if( pin_id == registered_module_pin_configs[module_id].pins[i].id)
            {
            /* found pin with module id */
            return TRUE;
            }
            
    }

    return FALSE;
}
