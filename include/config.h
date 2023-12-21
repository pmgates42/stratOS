/**********************************************************
 * 
 *  config.c
 * 
 * 
 *  DESCRIPTION:
 *      System configuration interface
 *
 */

#pragma once

#include "generic.h"
#include "peripherals/snsr/snsr.h"
#include "peripherals/snsr/hc_sr04.h"

#define SNSR_MAX_CHNL 8   /* Maximum number of channels per sensor */
#define SNSR_MAX_CFGS 100 /* Maximum number of sensors */

typedef uint8_t config_err_t8;
enum
{
    CONFIG_ERR_NONE = 0,
    CONFIG_ERR_INVLD_CONFIG,
};

typedef union
{   
    hc_sr04_config_t hc_sr04;   /* HC-SR04 Ultrasonic Distance Sensor */

} snsr_hw_config_t;

typedef struct
{
    char name[16];                  /* Name of sensor */
    snsr_hw_config_t hw_config;     /* Hardware configuration */
    snsr_hardware_t8 hw;            /* Hardware type */

} snsr_config_t;

typedef struct
{
    uint16_t num_snsrs;
    snsr_config_t snsr_configs[SNSR_MAX_CFGS];

} kernel_config_t;

config_err_t8 config_get_sys_config(kernel_config_t *config);
