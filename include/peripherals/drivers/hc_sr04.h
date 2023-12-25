/**********************************************************
 * 
 *  hc_sr04.c
 * 
 * 
 *  DESCRIPTION:
 *       HC-SR04 Ultrasonic Sensor kernel to hardware
 *       interface
 * 
 *  NOTES:
 *       The TRIG pin is an output pin intended to
 *       transmit a 10uS TTL pulse that will trigger
 *       a new reading from the ECHO pin.
 * 
 *       The ECHO pin sends a pulse back that is
 *       proportional to the distance the sound that
 *       the trigger produced travelled.
 *       
 *       https://www.handsontec.com/dataspecs/HC-SR04-Ultrasonic.pdf
 * 
 */

#pragma once

#include "generic.h"
#include "config.h"
#include "peripherals/snsr/snsr.h"

/* Types */
typedef struct
{
     void (* init)(void);                           /* Initialize sensor callback */
     snsr_err_t8 (* register_snsr)(snsr_config_t config); 
                                                    /* Regsiter sensor callback   */
} hc_sr04_intf_t;

/* Manager functions */
void hc_sr04_intf_init(void);                      /* Initialize this manager */
void hc_sr04_intf_reg_intf(hc_sr04_intf_t intf);   /* Register interface      */

/* Interface functions */
hc_sr04_intf_t hc_sr04_get_reg_intf(void);         /* Get registerable interface  */
void hc_sr04_init(void);                           /* Initialize driver           */
snsr_err_t8 hc_sr04_register_snsr(snsr_config_t config);  /* Register new sensor instance*/