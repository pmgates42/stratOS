/**********************************************************
 * 
 *  hc_sr04_intf_mgr.c
 * 
 * 
 *  DESCRIPTION:
 *     HC-SR04 interface manager. Middle man inbetween
 *     optionally configured sensor hardware driver
 *     code and the kernel.
 *
 */

#include "peripherals/drivers/hc_sr04.h"
#include "peripherals/snsr/snsr.h"
#include "generic.h"
#include "debug.h"

static hc_sr04_intf_t s_intf;

/**********************************************************
 * 
 *  hc_sr04_intf_reg_intf()
 * 
 * 
 *  DESCRIPTION:
 *      Register an interface
 *
 */

void hc_sr04_intf_reg_intf(hc_sr04_intf_t intf)
{
    memcpy(&s_intf, &intf, sizeof(hc_sr04_intf_t));
}

/**********************************************************
 * 
 *  hc_sr04_init()
 * 
 * 
 *  DESCRIPTION:
 *      Initialize the interface manager
 *
 */

void hc_sr04_intf_init(void)
{
    clr_mem(&s_intf, sizeof(hc_sr04_intf_t));
}

/**********************************************************
 * 
 *  hc_sr04_init()
 * 
 * 
 *  DESCRIPTION:
 *      Initialize the hardware driver
 *
 */

void hc_sr04_init(void)
{
    if(NULL != s_intf.init)
    {
        s_intf.init();
    }

}

/**********************************************************
 * 
 *  hc_sr04_register_snsr()
 * 
 * 
 *  DESCRIPTION:
 *      Register a new sensor instance with the driver
 *
 */

snsr_err_t8 hc_sr04_register_snsr(snsr_config_t config)
{
    if(NULL != s_intf.register_snsr)
    {
        return s_intf.register_snsr(config);
    }

    return SNSR_ERR_FAILED;
}

/**********************************************************
 * 
 *  hc_sr04_update()
 * 
 * 
 *  DESCRIPTION:
 *      Register a new sensor instance with the driver
 *
 */

void hc_sr04_update(void)
{
    if(NULL != s_intf.update)
    {
        s_intf.update();
    }
}
