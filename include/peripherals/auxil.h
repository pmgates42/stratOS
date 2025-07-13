#pragma once

#include "generic.h"
#include "peripherals/base.h"

// TODO this is BCM 283x specific. Move into appropriate header file.
typedef struct
{
    reg32_t irq_status;        /* IRQ status */
    reg32_t enables;           /* Enable bits for various interrupt sources */
    reg32_t reserved[14];      /* Reserved */
    reg32_t mu_io;             /* Mini Uart I/O Data */
    reg32_t mu_ier;            /* Mini Uart Interrupt Enable */
    reg32_t mu_iir;            /* Mini Uart Interrupt Identify */
    reg32_t mu_lcr;            /* Mini Uart Line Control */
    reg32_t mu_mcr;            /* Mini Uart Modem Control */
    reg32_t mu_lsr;            /* Mini Uart Line Status */
    reg32_t mu_msr;            /* Mini Uart Modem Status */
    reg32_t mu_scratch;        /* Mini Uart Scratch */
    reg32_t mu_control;        /* Mini Uart Extra Control */
    reg32_t mu_status;         /* Mini Uart Extra Status */
    reg32_t mu_baudrate;       /* Mini Uart Baudrate */

} bcm2xxx_aux_reg_t;    /* BCM2XXX Autilary register peripheral map */

#define REG_AUX_BASE ((volatile bcm2xxx_aux_reg_t *)(PBASE + 0x215000))
