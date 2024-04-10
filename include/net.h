#include "generic.h"
#include "strat_net.h"

void sock_api_init( void );

void strat_net_udp_rx( strat_net_sock_dscrptr_t desc, void * data, size_t size );

void strat_net_udp_tx( strat_net_sock_dscrptr_t desc, void * out, size_t size );
