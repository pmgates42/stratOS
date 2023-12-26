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
#include "utils.h"
#include "uart.h"//TODO remove after testing

typedef struct
{
    uint16_t         inst_id;  /* instance id */
    hc_sr04_config_t pin_cfg;  /* pin config */
    float            dist;     /* calculated distance (cm) */

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

/**********************************************************
 * 
 *  hc_sr04_get_reg_intf()
 * 
 *  DESCRIPTION:
 *     Contracted get driver registration/callbacks function   
 * 
 */

hc_sr04_intf_t hc_sr04_get_reg_intf(void)
{
    hc_sr04_intf_t hc_intf;

    hc_intf.init = init;
    hc_intf.register_snsr = register_sensor;

    return hc_intf;
}

/**********************************************************
 * 
 *  init()
 * 
 *  DESCRIPTION:
 *     Contracted initialization function   
 * 
 */

static void init(void)
{
    clr_mem(&s_instance_cb_lst, sizeof(instance_cb_lst_t));
    s_instance_cb_lst.count = 0;

}

/**********************************************************
 * 
 *  register_sensor()
 * 
 *  DESCRIPTION:
 *     Contracted instance register function   
 * 
 */

static snsr_err_t8 register_sensor(snsr_config_t config)
{
    /* local vars */
    uint16_t index;
    hc_sr04_config_t p_cfg;

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

    p_cfg = s_instance_cb_lst.instances[index].pin_cfg;

    /* set up GPIO pins */
    gpio_pin_enable(p_cfg.echo);
    gpio_pin_enable(p_cfg.trig);
    gpio_pin_setas_inp(p_cfg.echo);
    gpio_pin_setas_outp(p_cfg.trig);

    /* initialize the trigger to low */
    gpio_clr(p_cfg.trig);

    return SNSR_ERR_NONE;
}

/**********************************************************
 * 
 *  update()
 * 
 *  DESCRIPTION:
 *     Contracted update function. Updates distance data for
 *     each registered instance.
 * 
 */

static void update(void)
{
    #define MAX_ECHO_ITERS 0xF0000000

    /* local vars */
    uint8_t i;
    hc_sr04_config_t p_cfg;
    uint64_t echo_iters = 0;

    /* update all instances */
    for(i = 0; i < s_instance_cb_lst.count; i++)
    {
        /* send the trigger signal */
        p_cfg = s_instance_cb_lst.instances[i].pin_cfg;
        gpio_set(p_cfg.trig);
        delay_us(10);
        gpio_clr(p_cfg.trig);

        /* retrieve the echo signal */
        while(1 == gpio_read_outp(p_cfg.echo))
        {
            echo_iters++;
            if(echo_iters == MAX_ECHO_ITERS) break;
        }

        /* calculate the distance */
        s_instance_cb_lst.instances[i].dist = (float)(echo_iters/58);
        uart_send('\n');
        uart_send_uint32(echo_iters);
        uart_send('\n');
    }


    #undef MAX_ECHO_ITERS

}