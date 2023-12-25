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

#define CFG_SNSR_MAX_CFGS 100 /* Maximum number of sensors */
#define CFG_MAX_DST_SNSR  10  /* Maximum number of distance sensors */

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
