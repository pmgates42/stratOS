#include "generic.h"
#include "strat_net.h"

void sock_api_init( void );

void strat_net_udp_rx( void * data, size_t size );

void strat_net_udp_tx( void * out, size_t size );

//TODO remove after testing?
void set_static_ip();
void net_init();
int get_packet(char *buffer, size_t buffer_size, int timeout_us);
void net_proc();