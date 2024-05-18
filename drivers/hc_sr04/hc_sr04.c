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
#include "printf.h"

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
}

static snsr_err_t8 register_sensor(snsr_config_t config)
{
    /* local vars */
    uint16_t index;

    /* input validation */
    if(config.hw_type != SNSR_HW_HCSR04)
    {
        printf("SNSR_ERR_INVLD_CFG");
        return SNSR_ERR_INVLD_CFG;
    }

    if(s_instance_cb_lst.count >= CFG_MAX_DST_SNSR)
    {
        printf("SNSR_ERR_INVLD_CFG. Count is %d", s_instance_cb_lst.count);
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

    gpio_pin_enable(config.hw_config.hc_sr04.echo);
    gpio_pin_enable(config.hw_config.hc_sr04.trig);

    printf("New HC_SR04 sensor\ntrig_pin=%d,\necho_pin=%d",
            s_instance_cb_lst.instances[index].pin_cfg.trig,
            s_instance_cb_lst.instances[index].pin_cfg.echo);

    return SNSR_ERR_NONE;
}

void update(void)
{
    uint8_t i;
    uint8_t trig_pin;
    uint8_t echo_pin;
    uint32_t us_delay = 0;

    while (1)
    {
        /* Update all instances */
        for(i = 0; i < s_instance_cb_lst.count; i++)
        {
            us_delay = 0;

            trig_pin = s_instance_cb_lst.instances->pin_cfg.trig;
            echo_pin = s_instance_cb_lst.instances->pin_cfg.echo;

            /* Set the trigger pin high for 10uS */
            gpio_set(trig_pin);
            delay_us(10);
            gpio_clr(trig_pin);

            /* Wait for echo pin to go high */
            while( 0 == gpio_get(echo_pin) )
                ;

            /* Wait for echo pin to go low */
            while( 1 == gpio_get(echo_pin) )
            {
                delay_us(1);
                us_delay++;
            }

            printf("\nDelay was %d microseconds.", us_delay);

            delay_ms(1000);
        }
    }
 
}