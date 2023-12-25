/**********************************************************
 * 
 *  hc_sr04.c
 * 
 * 
 *  DESCRIPTION:
 *      HC-SR04 Ultrasonic Sensor Driver
 *
 */

#include "generic.h"
#include "config.h"
#include "peripherals/gpio.h"
#include "peripherals/drivers/hc_sr04.h"
#include "peripherals/snsr/snsr.h"
#include "debug.h"

typedef struct
{
    uint16_t         inst_id;  /* instance id */
    hc_sr04_config_t pin_cfg;  /* pin config */

} hc_instance_cb;   /* instance control block */

typedef struct
{
    hc_instance_cb instances[CFG_MAX_DST_SNSR];
                                /* instances managed by this driver */
    uint16_t    count;          /* count of existing instances */
} instance_cb_lst_t;

static instance_cb_lst_t s_instance_cb_lst;    /* instance control block list */

static snsr_err_t8 register_sensor(snsr_config_t config);
static void init(void);

hc_sr04_intf_t hc_sr04_get_reg_intf(void)
{
    hc_sr04_intf_t hc_intf;

    hc_intf.init = init;
    hc_intf.register_snsr = register_sensor;

    return hc_intf;
}

static void init(void)
{
    clr_mem(&s_instance_cb_lst, sizeof(instance_cb_lst_t));
    s_instance_cb_lst.count = 0;

}

static snsr_err_t8 register_sensor(snsr_config_t config)
{
    /* local vars */
    uint16_t index;

    /* input validation */
    if(config.hw_type != SNSR_HW_HCSR04)
    {
        return SNSR_ERR_INVLD_CFG;
    }

    if(s_instance_cb_lst.count < CFG_MAX_DST_SNSR)
    {
        return SNSR_ERR_SNSR_LIMIT;
    }

    /* store new instance */
    index = s_instance_cb_lst.count;
    s_instance_cb_lst.instances[index].pin_cfg = config.hw_config.hc_sr04;
    s_instance_cb_lst.instances[index].inst_id = index;
    s_instance_cb_lst.count++;

    /* set up GPIO pins */
    gpio_pin_setas_inp(config.hw_config.hc_sr04.echo);
    gpio_pin_setas_outp(config.hw_config.hc_sr04.trig);

    return SNSR_ERR_NONE;
}
