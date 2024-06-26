/**********************************************************
 * 
 *  utils.S
 * 
 * 
 *  DESCRIPTION:
 *      Utility definitions
 */

.global delay
delay:
    /* delay for x0 cycles */
    subs x0, x0, #1
    bne delay
    ret

.global put32
put32:
    str w1, [x0]
    ret

.global get32
get32:
    ldr w0, [x0]
    ret

.global get_el
get_el:
    mrs x0, CurrentEL
    lsr x0, x0, #2
    ret