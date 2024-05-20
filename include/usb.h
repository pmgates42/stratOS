/**********************************************************
 * 
 *  usb.h
 * 
 * DESCRIPTION:
 *      USB core header available for consumption by USB device
 *      drivers and kernel boot procedures.
 * 
 */

#pragma once

#include "generic.h"

/* Types */

typedef uint8_t usb_err_t;
enum
{
    USB_ERR_NONE,               /* No error */
    USB_ERR_CORE_FAULT,         /* Core fault error */
    USB_ERR_MEM,                /* Memory error */
    USB_ERR_INVLD_STATE,        /* Invalid state */
    USB_ERR_INVLD_CNFG,         /* Invalid configuration */
};

usb_err_t usb_core_init(void);

/* USB Packet types */

/* 4-bit PID codes
 *
 * Universal Serial Bus Specification Revision 2.0 Section 8.3.1
 * Table 8-1.
 */
typedef uint8_t usb_pid_code_t;
enum
{
    /* TOKEN */
    USB_PID_CODE_OUT = 0x1,
    USB_PID_CODE_IN = 0x9,
    USB_PID_CODE_SOF = 0x5,
    USB_PID_CODE_SETUP = 0xD,

    /* DATA */
    USB_PID_CODE_DATA0 = 0x3,
    USB_PID_CODE_DATA1 = 0xB,
    USB_PID_CODE_DATA2 = 0x7,
    USB_PID_CODE_MDATA = 0xF,

    /* HANDSHAKE */
    USB_PID_CODE_ACK = 0x2,
    USB_PID_CODE_NAK = 0xA,
    USB_PID_CODE_STALL = 0xE,
    USB_PID_CODE_NYET = 0x6,

    /* SPECIAL */

    /*
     * PID code is used by Token (PRE) and handshake (ERR) 
     */
    USB_PID_CODE_PRE = 0x0C,
    USB_PID_CODE_ERR = 0x0C,

    USB_PID_CODE_SPLIT = 0x08,
    USB_PID_CODE_PING = 0x04,
    USB_PID_CODE_RESRVD = 0x00,

};


/* Packet types
 *
 * Universal Serial Bus Specification Revision 2.0 Section 8.4
 */

typedef struct
{
    usb_pid_code_t pid : 4;        /* Packet Identifier */
    usb_pid_code_t pid_cmpl : 4;   /* PID 1's compliment */
} usb_pkt_pid_t;

typedef struct
{
    usb_pkt_pid_t pid;      /* PID field */
    uint8_t addr : 7;       /* Device address */
    uint8_t endp : 4;       /* Endpoint number */
    uint8_t crc : 5;        /* CRC5 */
} usb_pkt_token_t;

typedef struct
{
    usb_pkt_pid_t pid;      /* PID field */
    void * data;            /* Data field (0-8192 bits) */
    uint16_t crc;           /* CRC16 */
} usb_pkt_data_t;

typedef struct
{
    usb_pkt_pid_t pid;      /* PID field */
} usb_pkt_handshake_t;
