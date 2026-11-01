/**********************************************************
 *
 *  w5500_regs.h
 *
 *  DESCRIPTION:
 *      WIZnet W5500 register map, SPI frame encoding and
 *      bit constants used by drivers/w5500/w5500.c.
 *
 *      All multi-byte registers are big-endian on the wire.
 *
 */

#pragma once

#include "generic.h"

/* ---- SPI frame (variable data length mode) -------------------------
 *
 *   addr[15:8], addr[7:0], control, data[0..N-1]
 *
 *   control = (BSB << 3) | (RWB << 2) | OM
 */

#define W5500_FRAME_HEADER_SIZE     3U

#define W5500_BSB_COMMON            0x00U
#define W5500_BSB_SOCKET_REG(n)     ((uint8_t)(((n) * 4U) + 1U))
#define W5500_BSB_SOCKET_TX(n)      ((uint8_t)(((n) * 4U) + 2U))
#define W5500_BSB_SOCKET_RX(n)      ((uint8_t)(((n) * 4U) + 3U))

#define W5500_RWB_READ              0x00U
#define W5500_RWB_WRITE             0x04U
#define W5500_OM_VDM                0x00U

#define W5500_CONTROL_BYTE(bsb, rwb) \
    ((uint8_t)((((uint8_t)(bsb)) << 3) | ((uint8_t)(rwb)) | W5500_OM_VDM))

/* ---- Common register block (BSB 0) --------------------------------- */

#define W5500_REG_MR                0x0000U
#define W5500_REG_SHAR              0x0009U     /* 6 bytes, source MAC */
#define W5500_REG_IR                0x0015U
#define W5500_REG_IMR               0x0016U
#define W5500_REG_SIMR              0x0018U
#define W5500_REG_PHYCFGR           0x002EU
#define W5500_REG_VERSIONR          0x0039U

#define W5500_MR_RST                0x80U
#define W5500_PHYCFGR_LNK           0x01U
#define W5500_VERSIONR_VALUE        0x04U

/* ---- Socket register block (BSB n*4+1), offsets ------------------- */

#define W5500_SN_MR                 0x0000U
#define W5500_SN_CR                 0x0001U
#define W5500_SN_IR                 0x0002U
#define W5500_SN_SR                 0x0003U
#define W5500_SN_RXBUF_SIZE         0x001EU
#define W5500_SN_TXBUF_SIZE         0x001FU
#define W5500_SN_TX_FSR             0x0020U
#define W5500_SN_TX_RD              0x0022U
#define W5500_SN_TX_WR              0x0024U
#define W5500_SN_RX_RSR             0x0026U
#define W5500_SN_RX_RD              0x0028U
#define W5500_SN_RX_WR              0x002AU
#define W5500_SN_IMR                0x002CU

#define W5500_SN_MR_MACRAW          0x04U
#define W5500_SN_MR_MFEN            0x80U       /* MAC filter enable  */
#define W5500_SN_MR_PROTOCOL_MASK   0x0FU

#define W5500_SN_CR_OPEN            0x01U
#define W5500_SN_CR_CLOSE           0x10U
#define W5500_SN_CR_SEND            0x20U
#define W5500_SN_CR_RECV            0x40U

#define W5500_SN_SR_CLOSED          0x00U
#define W5500_SN_SR_MACRAW          0x42U

#define W5500_SN_IR_SEND_OK         0x10U
#define W5500_SN_IR_TIMEOUT         0x08U
#define W5500_SN_IR_RECV            0x04U

/* ---- Sizes --------------------------------------------------------- */

#define W5500_SOCKET_COUNT          8U
#define W5500_MACRAW_SOCKET         0U
#define W5500_BUFFER_SIZE_KB        16U
#define W5500_BUFFER_SIZE           (W5500_BUFFER_SIZE_KB * 1024U)
#define W5500_BUFFER_MASK           (W5500_BUFFER_SIZE - 1U)
#define W5500_RX_LENGTH_PREFIX_SIZE 2U
#define W5500_MIN_FRAME_SIZE        14U
#define W5500_MAX_FRAME_SIZE        1514U
