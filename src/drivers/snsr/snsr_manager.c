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

#include "generic.h"
#include "peripherals/snsr/snsr.h"
#include "config.h"
#include "debug.h"
#include "uart.h"

#define MAX_NUM_SNSR_PER_TYPE  25

/* Types */
typedef struct
{
    snsr_hardware_t8 hw;
    snsr_type_t8 type;

} snsr_hw_type_map_t;

typedef struct
{
    snsr_config_t * config;
} active_sensor;

typedef struct
{
    active_sensor snsr_lst[MAX_NUM_SNSR_PER_TYPE];
                                    /* List of active sensors */
    uint8_t active_cnt;             /* Count of active sensors */

} active_sensor_lst_t;

/* Constants */
static const snsr_hw_type_map_t hw_type_map[] =
{
    { SNSR_HW_HCSR04, SNSR_TYPE_DIST }
};

/* Variables */
static active_sensor_lst_t active_dst_sensors;

static boolean load_configured_sensors(void);
static snsr_type_t8 get_snsr_type(snsr_hardware_t8 hw_type);

/**********************************************************
 * 
 *  snsr_init()
 * 
 * 
 *  DESCRIPTION:
 *      Initialize all of the configured sensors
 *
 */

snsr_err_t8 snsr_init(void)
{
    /* ininitialize the active sensor lists */
    clr_mem(&active_dst_sensors, sizeof(active_dst_sensors));

    /* ensure the uart is initialized */
    if(!uart_is_init())
    {
        uart_init();
    }

    if(!load_configured_sensors())
    {
        return SNSR_ERR_INVLD_CFG;
    }
    return SNSR_ERR_NONE;

}

/**********************************************************
 * 
 *  load_configured_sensors()
 * 
 * 
 *  DESCRIPTION:
 *      Load in all of the configured sensors. Returns TRUE
 *      if we were able to successfully load the sensor config.
 *      Returns FALSE otherwise
 *
 */

static boolean load_configured_sensors(void)
{
    /* Local variables */
    kernel_config_t kernel_config;
    uint8_t i;
    snsr_hardware_t8 tmp_hw_type;
    snsr_type_t8 tmp_snsr_type;
    uint8_t tmp_idx;

    /* input validation */
    if(CONFIG_ERR_NONE != config_get_sys_config(&kernel_config) || 
      (CFG_SNSR_MAX_CFGS < kernel_config.num_snsrs))
    {
        return FALSE;
    }

    /* add all configured sensors to the appropriate list */
    for(i = 0; i < kernel_config.num_snsrs; i++)
    {
        tmp_hw_type = kernel_config.snsr_configs[i].hw;
        tmp_snsr_type = get_snsr_type(tmp_hw_type);

        switch(tmp_snsr_type)
        {
        /* add the configured distance sensor */
        case SNSR_TYPE_DIST:
            tmp_idx = active_dst_sensors.active_cnt;
            active_dst_sensors.snsr_lst[tmp_idx].config = &kernel_config.snsr_configs[i];
            break;

        default:
            uart_send_string("Invalild sensor configuration\n");
            break;
        }
    }

    debug_set_led();
    return TRUE;
}

/**********************************************************
 * 
 *  get_snsr_type()
 * 
 * 
 *  DESCRIPTION:
 *      Get the sensor type given the sensor hardware type.
 *
 */

static snsr_type_t8 get_snsr_type(snsr_hardware_t8  hw_type)
{
    uint8_t i;

    /* input validation */
    if(hw_type >= SNSR_HW_COUNT)
    {
        return SNSR_TYPE_INVLD;
    }

    for(i = 0; i < list_cnt(hw_type_map); i++)
    {
        if(hw_type_map[i].hw == hw_type)
            {
            return hw_type_map[i].type;
            }
    }

    /* could not find sensor type in list */
    return SNSR_TYPE_INVLD;
}
