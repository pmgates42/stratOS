#ifdef EMBEDDED_BUILD

/**********************************************************
 * 
 *  mm.S
 * 
 * 
 *  DESCRIPTION:
 *      Memory management functions
 *
 */

.global memzero
memzero:
    /* parameters:
        x0 = pointer to memory
        x1 = number of bytes to zero */

    str xzr, [x0], #8     /* store zero to memory, increment pointer */
    subs x1, x1, #8       /* decrement counter */
    b.gt memzero          /* if counter > 0, repeat */
    ret

#endif