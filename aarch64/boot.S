#ifdef EMBEDDED_BUILD

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
#include "sysregs.h"

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

    /* Boot into EL1 
        OS ARM code typically runs in EL1.
    */

    ldr x0, =SCTLR_VALUE_MMU_DISABLED
    msr sctlr_el1, x0

    ldr x0, =HCR_VALUE
    msr hcr_el2, x0

    ldr x0, =SCR_VALUE
    msr scr_el3, x0

    ldr x0, =SPSR_VALUE_EL1
    msr spsr_el3, x0    

    adr x0, el1_entry
    msr elr_el3, x0

    /* return from the exception. Reset the program counter
    and return from current exception lvl
    */
    eret

el1_entry:
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


#endif