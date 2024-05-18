/**********************************************************
 * 
 *  snsr.h
 * 
 * 
 *  DESCRIPTION:
 *      common sensor types and definitions
 *
 */

#pragma once

#include "generic.h"
#include "config.h"

typedef uint16_t snsr_id_t16;

typedef uint8_t snsr_type_t8;
enum
    {
    SNSR_TYPE_DIST = 0,
    SNSR_TYPE_COUNT,
    SNSR_TYPE_INVLD = SNSR_TYPE_COUNT,
    };

typedef uint8_t snsr_err_t8;
enum
    {
    SNSR_ERR_NONE = 0,
    SNSR_ERR_INVLD_PARMS,
    SNSR_ERR_INVLD_STATE,
    SNSR_ERR_INVLD_MODE,
    SNSR_ERR_INVLD_TYPE,
    SNSR_ERR_INVLD_SIZE,
    SNSR_ERR_INVLD_CHNL,
    SNSR_ERR_INVLD_CHNL_TYPE,
    SNSR_ERR_INVLD_CHNL_NUM,
    SNSR_ERR_INVLD_CHNL_STATE,
    SNSR_ERR_INVLD_CHNL_OP,
    SNSR_ERR_FAILED,
    SNSR_ERR_TIMEOUT,
    SNSR_ERR_COUNT,
    SNSR_ERR_INVLD_CFG,
    SNSR_ERR_SNSR_LIMIT,
    SNSR_ERR_UNKNOWN = SNSR_ERR_COUNT,
    };

typedef struct
{
    snsr_config_t config;   /* Sensor config (NV copy) */
    boolean registered;     /* Sensor is registered */
    snsr_id_t16 sid;        /* Sensor ID */

} snsr_cb_t;    /* Sensor control block */

snsr_err_t8 snsr_init(void);
snsr_type_t8 snsr_get_snsr_type(snsr_hardware_t8 hw_type);
