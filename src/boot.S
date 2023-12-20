/**********************************************************
 * 
 *  boot.S
 * 
 * 
 *  DESCRIPTION:
 *      Kernel boot code
 *
 */

#include "mm.h"

.section ".text.boot"

.global _start
_start:
    /* check if we are running on the master CPU
     if not, hang */
    mrs x0, mpidr_el1
    and x0, x0, #0xFF
    cbz x0, master
    b hang

master:
    /* clear BSS */
    adr x0, bss_begin     
    adr x1, bss_end
    sub x1, x1, x0
    bl memzero

    /* initialize the stack pointer and branch to kernel_main */
    mov sp, #LOW_MEMORY
    bl kernel_main
    b hang

hang:
    wfe
    b hang

