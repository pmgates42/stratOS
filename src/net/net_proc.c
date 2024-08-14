/**********************************************************
 * 
 *  net_proc.c
 * 
 *      Network processing task and friends
 *
 */

#include "net.h"
#include "generic.h"

#define IN_DATA_BUFF_SIZE 1200

static char in_data_buffer[ IN_DATA_BUFF_SIZE ];

/**********************************************************
 * 
 *  net_proc()
 * 
 *  DESCRIPTION:
 *      Network processing task
 *
 */

void net_proc()
{
    get_packet(&in_data_buffer, IN_DATA_BUFF_SIZE);

    printf("packet_data=%s", in_data_buffer);
}