/**********************************************************
 * 
 *  sock_api.c
 * 
 * 
 *  DESCRIPTION:
 *      Kernel socket API (Go socks)
 *
 */

#include "generic.h"
#include "strat_net.h"
#include "net.h"

/* Local types */
typedef struct
{
    strat_net_socket_def_t sock_table[MAX_CONCURRENT_SOCKETS];
    uint8_t                sock_cnt;
} sock_ctrl_t;

typedef void ( *tx_proc )( void *, size_t );
typedef void ( *rx_proc )( void *, size_t );

/* Module control vars */
static sock_ctrl_t sock_ctrl;
static tx_proc tx_procs[MAX_CONCURRENT_SOCKETS];
static rx_proc rx_procs[MAX_CONCURRENT_SOCKETS];

/**********************************************************
 * 
 *  sock_api_init
 * 
 */
void sock_api_init( void )
{
    /* clear the socket table */
    clr_mem( &sock_ctrl, sizeof( sock_ctrl ) );
}

/* Sys calls */

/**********************************************************
 * 
 *  strat_socket
 * 
 * 
 *  DESCRIPTION:
 *      Create a new socket
 *
 */

strat_net_error_t strat_socket( strat_net_socket_def_t def, strat_net_sock_dscrptr_t *sock_dscrptr )
{
/* input validation */
if( def.family >= STRAT_NET_ADDR_FAMILY_CNT || def.type >= STRAT_NET_ADDR_FAMILY_CNT )
    return STRAT_NET_ERR_INVALID_ARG;

if(sock_ctrl.sock_cnt >= MAX_CONCURRENT_SOCKETS)
    return STRAT_NET_ERR_NO_RESOURCES;

/* register the new socket */
sock_ctrl.sock_table[sock_ctrl.sock_cnt] = def;

*sock_dscrptr = sock_ctrl.sock_cnt;

/* setup the rx/tx procs based on the socket type */
switch(def.type)
{
    // case STRAT_NET_SOCKET_TYPE_STREAM:
    //     tx_procs[sock_ctrl.sock_cnt] = strat_net_tcp_tx;
    //     rx_procs[sock_ctrl.sock_cnt] = strat_net_tcp_rx;
    //     break;

    case STRAT_NET_SOCKET_TYPE_DGRAM:
        tx_procs[sock_ctrl.sock_cnt] = strat_net_udp_tx;
        rx_procs[sock_ctrl.sock_cnt] = strat_net_udp_rx;
        break;

    // case STRAT_NET_SOCKET_TYPE_RAW:
    //     tx_procs[sock_ctrl.sock_cnt] = strat_net_raw_tx;
    //     rx_procs[sock_ctrl.sock_cnt] = strat_net_raw_rx;
    //     break;

    /* defensive programming */
    default:
        return STRAT_NET_ERR_INVALID_ARG;
}

sock_ctrl.sock_cnt++;

return START_NET_ERR_NONE;
}

void strat_net_transmit( strat_net_sock_dscrptr_t sock_dscrptr, void *data, size_t len )
{
    /* input validation */
    if( sock_dscrptr >= sock_ctrl.sock_cnt && !tx_procs[sock_dscrptr] )
        return;

    tx_procs[sock_dscrptr]( data, len );
}

void start_net_receive( strat_net_sock_dscrptr_t sock_dscrptr, void *out, size_t len )
{
    /* input validation */
    if( sock_dscrptr >= sock_ctrl.sock_cnt && !rx_procs[sock_dscrptr] )
        return;

    rx_procs[sock_dscrptr]( out, len );
}
