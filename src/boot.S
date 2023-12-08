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
    /* Set GPIO pin 23 as an output */
    ldr x0, =0x3F200000   // Base address for GPIO on Raspberry Pi 3 B+
    ldr x1, =((23/10)*4)   // GPIO register offset for GPFSEL2 (23 is in the third register)
    ldr x2, [x0, x1]      // Read the current value of GPFSEL2
    orr x2, x2, #(1 << ((23%10)*3))   // Set bits 6-8 to 001 to set GPIO pin 23 as an output
    str x2, [x0, x1]      // Write the modified value back to GPFSEL2

    /* Set the GPIO pin 23 high to turn on the LED */
    ldr x1, =((23/32)*4)   // GPIO register offset for GPSET0
    ldr x2, =1 << (23%32)  // Set the bit corresponding to GPIO pin 23
    str x2, [x0, x1]      // Write 1 to the corresponding bit in GPSET0

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

