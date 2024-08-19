/**********************************************************
 * 
 *  ip.c
 * 
 * 
 *  DESCRIPTION:
 *      IP communication module
 *
 */

#include "generic.h"
#include "strat_net.h"

#define MAX_PACKET_SIZE 1500

typedef struct
{
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t len;
    uint16_t chksum;
} udp_pkt_hdr_t;    /* UDP packet header */

char s_packet_data[ MAX_PACKET_SIZE ];


void strat_net_udp_rx( void * data, size_t size )
{
(void)data;
(void)size;
}

void strat_net_udp_tx( void * out, size_t size )
{
(void)out;
(void)size;
}
