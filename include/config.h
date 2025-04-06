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
 *  Module IDs
 * 
 * 
 *  DESCRIPTION:
 *      Configuration Module IDs. These are used to identify
 *      module specific resources such as config variables.
 * 
 *      These are used to bridge the gap between different
 *      user or hardware specific configurations (e.g., GPIO
 *      pins) and the kernel level processing.
 * 
 *      E.g., For the BCM27XX, the SPI0 MOSI/MISO pins may be
 *      10/9 for a specific RPI revision, but a user 
 *      may be working with a different board or a different
 *      RPI revision not yet supported by this OS. That is why
 *      we allow these peripheral configs to be as flexible
 *      as possible.
 * 
 *   NOTES:
 *      A project can define its own modules in user space
 *      if say, they need to write user level drivers that
 *      only need to interface with the kernel insomuch
 *      they only care about writing to pins and such.
 *      To accomplish this, projects can implement Module
 *      IDs that start after CONFIG_MODULE_ID__KERNEL_LAST.
 *
 *      WARNING: It is recommended to not use these IDs,
 *      project or not, as an index into any array since
 *      these ids may grow, shrink, or be re-ordered.
 */

typedef uint16_t config_module_id_type; /* Module ID type */
enum
{
CONFIG_MODULE_ID__SPI = 0,

CONFIG_MODULE_ID__KERNEL_LAST = CONFIG_MODULE_ID__SPI
};


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
#define CFG_SNSR_MAX_CFGS       100                     /* Maximum number of sensors */
#define CFG_MAX_DST_SNSR        10                      /* Maximum number of distance sensors */
#define CFG_PIN_CFG_MAX_RGSTRD  (CFG_MAX_MODULES * 10)  /* Maximum number of registered pin configs */
#define CFG_PIN_CFG_MAX_PINS    15                      /* Maximum number of pin configs per module */
#define CFG_PIN_CFG_MAX_PIN_NUM 800                     /* Largest pin number (sorry CPUs with over 800 pins) */

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
#define CFG_INVALID_PIN_ID      ( CFG_PIN_CFG_MAX_PIN_NUM + 1 )

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
    CONFIG_ERR_INVLD_CONFIG_ID,
    CONFIG_ERR_INVLD_PIN_ID,
    CONFIG_ERR_INVLD_MODULE_ID,
    CONFIG_ERR_REACHED_MAX_PIN_CFGS,
    CONFIG_ERR_MODULE_REACHED_MAX_PINS,
    CONFIG_ERR_INVALID_INPUT,
    CONFIG_ERR_UNREGISTERED,
};

typedef uint8_t config_bit_order_t8;
enum
{
    CONFIG_BIT_ORDER_MSB,
    CONFIG_BIT_ORDER_LSB
};

typedef uint8_t config_endianness_t8;
enum
{
    CONFIG_ENDIAN_LITTLE,
    CONFIG_ENDIAN_BIG
};

/**********************************************************
 * 
 *  spi_parameter_config_type
 * 
 *  DESCRIPTION:
 *     SPI parameters.
 * 
 *  NOTES:
 *      Right now, the SPI driver is very simple so things
 *      like polarity cs polarity transfer mode (half/full
 *      duplex) are not necessary. Surely, this config will
 *      be expanded in the future.
 *
 */

typedef struct __attribute__((aligned(4)))
{
    uint32_t slck_speed_hz;
    uint8_t  data_size;
    config_bit_order_t8 bit_order;
    config_endianness_t8 endianness;
} spi_parameter_config_type;

typedef struct __attribute__((aligned(4)))
{
    spi_parameter_config_type spi_params;   /* CONFIG_ID_SPI_PARAMS */
} config_block_t;

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

typedef uint16_t config_pin_id_type;

typedef struct
{
    uint16_t           pin_number;
    config_pin_id_type id;

} config_pin_type;

typedef struct
{
    config_module_id_type module_id;
    config_pin_type       pins[CFG_PIN_CFG_MAX_PINS];
} pin_config_type;

typedef uint8_t config_id_type;
enum
{
    CONFIG_ID_SPI_PARAMS,
    CONFIG_ID_COUNT = CONFIG_ID_SPI_PARAMS + 20,
};

/**********************************************************
 * 
 *  config_module_init()
 * 
 *  DESCRIPTION:
 *     Initialize the config module. Must be called before
 *     any config operations can occur.
 *
 */

config_err_t8 config_module_init(void);

/**********************************************************
 * 
 *  config_register_pin_for_module()
 * 
 *  DESCRIPTION:
 *     Register a single pin for a single module.
 *
 */

config_err_t8 config_register_pin_for_module(config_module_id_type module_id, uint16_t pin, config_pin_id_type id);

/**********************************************************
 * 
 *  config_register_spi_parameters()
 * 
 *  DESCRIPTION:
 *     Register SPI parameters with the config module.
 *
 */

config_err_t8 config_register_spi_parameters(spi_parameter_config_type params); // TODO make this device specific based on the current clock

/**********************************************************
 * 
 *  config_get_registration_status()
 * 
 *  DESCRIPTION:
 *     Check if a config is registered
 *
 */

boolean config_get_registration_status(config_id_type config_id);

/**********************************************************
 * 
 *  config_pin_is_registered()
 * 
 *  DESCRIPTION:
 *     Check if a pin is registered for a module
 *
 */

boolean config_pin_is_registered(config_module_id_type module_id, config_pin_id_type pin_id);

/**********************************************************
 * 
 *  config_lookup_pin()
 * 
 *  DESCRIPTION:
 *     Get the configured pin provided the pid_id
 *
 */

config_err_t8 config_lookup_pin(config_pin_id_type pin_id, uint32_t * out_ptr);

/**********************************************************
 * 
 *  config_read()
 * 
 *  DESCRIPTION:
 *      Read a config
 *
 */

config_err_t8 config_read(config_id_type config_id, config_block_t * out_config);