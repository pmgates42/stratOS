/**********************************************************
 * 
 *  config.c
 * 
 * 
 *  DESCRIPTION:
 *      System configuration interface
 *
 *  NOTES:
 *      Types in this file map directly to nonvol data.
 *      Be aware that changes will break backwards
 *      compatability.
 * 
 */

#pragma once

#include "generic.h"

/**********************************************************
 * 
 *  Limits
 * 
 * 
 *  DESCRIPTION:
 *      the limit constants defined here can be adjusted
 *      if necessary (if we need to use more or less memory
 *      somewhere) but consider the implications when changing
 *      these values.
 *
 */

#define CFG_MAX_MODULES         10
#define CFG_SNSR_MAX_CFGS       100             /* Maximum number of sensors */
#define CFG_MAX_DST_SNSR        10              /* Maximum number of distance sensors */
#define CFG_PIN_CFG_MAX_RGSTRD  CFG_MAX_MODULES /* Maximum number of registered pin configs */
#define CFG_PIN_CFG_MAX_PINS    15              /* Maximum number of pin configs per module */
#define CFG_PIN_CFG_MAX_PIN_NUM 800             /* Largest pin number (sorry CPUs with over 800 pins) */

/**********************************************************
 * 
 *  Invalid Values
 * 
 * 
 *  DESCRIPTION:
 *      These are invalid /pre-initialized values for various
 *      configs
 *
 */

#define CFG_INVALID_MODULE_ID   ( CFG_MAX_MODULES         + 1 )
#define CFG_INVALID_PIN_NUMBER  ( CFG_PIN_CFG_MAX_PIN_NUM + 1 )

typedef uint16_t config_module_id_type; /* Module ID type */

typedef uint8_t snsr_hardware_t8;
enum
    {
    SNSR_HW_NONE = 0,
    SNSR_HW_HCSR04,
    SNSR_HW_COUNT,
    };

typedef uint8_t config_err_t8;
enum
{
    CONFIG_ERR_NONE = 0,
    CONFIG_ERR_INVLD_CONFIG,
    CONFIG_ERR_INVLD_MODULE_ID,
    CONFIG_ERR_REACHED_MAX_PIN_CFGS,
    CONFIG_ERR_MODULE_REACHED_MAX_PINS,
};

typedef struct
{
    uint8_t trig;   /* TRIG pin */
    uint8_t echo;   /* ECHO pin */

} hc_sr04_config_t;

typedef union
{   
    hc_sr04_config_t hc_sr04;   /* HC-SR04 Ultrasonic Distance Sensor */

} snsr_hw_config_t;

typedef struct snsr_config_struc
{
    snsr_hw_config_t hw_config;     /* Hardware configuration */
    snsr_hardware_t8 hw_type;       /* Hardware type */

} snsr_config_t;

typedef struct  __attribute__((packed))
{
    uint16_t num_snsrs;
    snsr_config_t snsr_configs[CFG_SNSR_MAX_CFGS];

} kernel_config_t;

config_err_t8 config_get_sys_config(kernel_config_t *config);

typedef uint16_t config_pin_type;
typedef struct
{
    config_module_id_type module_id;
    config_pin_type       pins[CFG_PIN_CFG_MAX_PINS];
} pin_config_type;


config_err_t8 config_module_init(void);

config_err_t8 config_register_pins_for_module(config_module_id_type module_id, config_pin_type pin_cfg);
