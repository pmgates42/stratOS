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

typedef uint8_t snsr_type_t8;
enum
    {
    SNSR_TYPE_DIST = 0,
    SNSR_TYPE_COUNT,
    SNSR_TYPE_INVLD = SNSR_TYPE_COUNT,
    };

typedef uint8_t snsr_hardware_t8;
enum
    {
    SNSR_HW_NONE = 0,
    SNSR_HW_HCSR04,
    SNSR_HW_COUNT,
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
    SNSR_ERR_UNKNOWN = SNSR_ERR_COUNT,
    };

void snsr_init(void);