/**********************************************************
 * 
 *  snsr_dist.h
 * 
 * 
 *  DESCRIPTION:
 *      common distance sensor interface
 *
 */

#include "peripherals/snsr/snsr.h"
#include "generic.h"

typedef struct
{
    uint16_t id;
    float dist;    
} snsr_dist_hndl_t;

snsr_err_t8 snsr_dist_init(snsr_dist_hndl_t *snsr);
void snsr_dist_step(snsr_dist_hndl_t *snsr);