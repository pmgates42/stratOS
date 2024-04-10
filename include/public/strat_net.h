#pragma once

#include "generic.h"

#ifndef MAX_CONCURRENT_SOCKETS      /* [$cfg] Maximum number of sockets active at once */
#define MAX_CONCURRENT_SOCKETS 10
#endif

typedef uint8_t strat_net_addr_family_t;
enum
{
    STRAT_NET_ADDR_FAMILY_IPV4,
    STRAT_NET_ADDR_FAMILY_IPV6,             /* unsupported */
    STRAT_NET_ADDR_FAMILY_BLUETOOTH,        /* unsupported */
    STRAT_NET_ADDR_FAMILY_CNT
};

typedef uint8_t strat_net_socket_type_t;
enum
{
    STRAT_NET_SOCKET_TYPE_STREAM,   /* Stream (TCP, Bluetooth) */
    STRAT_NET_SOCKET_TYPE_DGRAM,    /* Datagram (UDP) */
    STRAT_NET_SOCKET_TYPE_RAW,      /* Raw */
    STRAT_NET_SOCKET_TYPE_CNT
};

typedef struct
{
    strat_net_addr_family_t family; /* Address family */
    strat_net_socket_type_t type;   /* Socket type */
    uint32_t timeout_ms;            /* Timeout milliseconds */

    union dest
    {
        struct
        {
            uint32_t ip;
            uint16_t port;
        } ipv4;

        struct
        {
            uint8_t ip[16];
            uint16_t port;
        } ipv6;

        struct
        {
            uint8_t mac[6];
            uint16_t port;
        } bluetooth;
    } dest;
    
} strat_net_socket_def_t;
typedef uint8_t strat_net_error_t;

enum
    {
    START_NET_ERR_NONE,
    STRAT_NET_ERR_INVALID_ARG,
    STRAT_NET_ERR_NO_RESOURCES,
    };

typedef uint8_t strat_net_sock_dscrptr_t;   /* Socket descriptor */

/* Sys calls */
void strat_net_transmit( strat_net_sock_dscrptr_t sock_dscrptr, void *data, size_t len );