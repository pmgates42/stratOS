/**********************************************************
* 
*  vector.S
* 
*  DESCRIPTION:
*      Contains the vector table defintions for ARMv8-A
*      AArch-64.
*/
#include "vector.h"

/**********************************************************
* 
*  define_vector()
* 
*  DESCRIPTION:
*      Helpful macro to define a vector table entry.
*
*/

.macro	define_vector label
.align	7           /* 128 byte alignment */
b	\label
.endm

/**********************************************************
* 
*  save_context()
* 
*  DESCRIPTION:
*      Helpful macro to save the context of the current
*      execution state.
*
*   NOTES:
*      Save all of the "working" registers. The link
*      register is saved separately.
*
*/

.macro	save_context

/* allocate space for the stack frame */
sub	sp, sp, #STACK_FRAME_SIZE

/* save the working registers */
stp	x0,  x1,  [sp, #16 * 0]
stp	x2,  x3,  [sp, #16 * 1]
stp	x4,  x5,  [sp, #16 * 2]
stp	x6,  x7,  [sp, #16 * 3]
stp	x8,  x9,  [sp, #16 * 4]
stp	x10, x11, [sp, #16 * 5]
stp	x12, x13, [sp, #16 * 6]
stp	x14, x15, [sp, #16 * 7]
stp	x16, x17, [sp, #16 * 8]
stp	x18, x19, [sp, #16 * 9]
stp	x20, x21, [sp, #16 * 10]
stp	x22, x23, [sp, #16 * 11]
stp	x24, x25, [sp, #16 * 12]
stp	x26, x27, [sp, #16 * 13]
stp	x28, x29, [sp, #16 * 14]

/* save the link address */
str	x30, [sp, #16 * 15] 
.endm


/**********************************************************
* 
*  restore_context()
* 
*  DESCRIPTION:
*      Helpful macro to restore the context of the current
*      execution state.
*
*/

/* restore the working registers */
.macro	restore_context
ldp	x0,  x1,   [sp, #16 * 0]
ldp	x2,  x3,   [sp, #16 * 1]
ldp	x4,  x5,   [sp, #16 * 2]
ldp	x6,  x7,   [sp, #16 * 3]
ldp	x8,  x9,   [sp, #16 * 4]
ldp	x10, x11,  [sp, #16 * 5]
ldp	x12, x13,  [sp, #16 * 6]
ldp	x14, x15,  [sp, #16 * 7]
ldp	x16, x17,  [sp, #16 * 8]
ldp	x18, x19,  [sp, #16 * 9]
ldp	x20, x21,  [sp, #16 * 10]
ldp	x22, x23,  [sp, #16 * 11]
ldp	x24, x25,  [sp, #16 * 12]
ldp	x26, x27,  [sp, #16 * 13]
ldp	x28, x29,  [sp, #16 * 14]
ldr	x30,       [sp, #16 * 15]

/* deallocate stack frame space */
add	sp, sp, #STACK_FRAME_SIZE		
eret
.endm

/**********************************************************
* 
*  exception_vector_table
* 
* 
*  DESCRIPTION:
*      Table of all exception vectors.
*
*   NOTES:
*      The vector table is aligned based on the size of the
*      total number of vectors. AArch-64 gives us 4 types of
*      vectors (sync, IRQ, FIQ, SError). Each type has 4
*      vectors (EL1t, EL1h, EL0_64, EL0_32). So we have a
*      total of 16 vectors. 16 * 128 = 2048. So we align the
*      vector table on a 2048 (bit set 11) byte boundary.
*/

.align    11
exception_vector_table:
    /* Trap (EL1t) vectors */
    define_vector vector_sync_el1t
    define_vector vector_irq_el1t
    define_vector vector_fiq_el1t
    define_vector vector_serror_el1t

    /* Hypervisor (EL1h) vectors */
    define_vector vector_sync_el1h
    define_vector vector_irq_el1h       /* Call to the contracted IRQ handler */
    define_vector vector_fiq_el1h
    define_vector vector_serror_el1h

    /* User 64-bit mode (EL0_64) vectors */
    define_vector vector_sync_el0_64
    define_vector vector_irq_el0_64
    define_vector vector_fiq_el0_64
    define_vector vector_serror_el0_64

    /* User 32-bit mode (EL0_32) vectors */
    define_vector vector_sync_el0_32
    define_vector vector_irq_el0_32
    define_vector vector_fiq_el0_32
    define_vector vector_serror_el0_32


// for now, just map these to common handler functions
vector_sync_el1t:
vector_sync_el1h:
vector_sync_el0_64:
vector_sync_el0_32:
    b vector_sync_common

vector_irq_el1t:
vector_irq_el0_64:
vector_irq_el0_32:
    b vector_irq_common

vector_fiq_el1t:
vector_fiq_el1h:
vector_fiq_el0_64:
vector_fiq_el0_32:
    b vector_fiq_common

vector_serror_el1t:
vector_serror_el1h:
vector_serror_el0_64:
vector_serror_el0_32:
    b vector_serror_common

vector_irq_el1h:
   	save_context 
	bl	irq_handle_irqs
	restore_context  

vector_sync_common:
    nop
    b vector_sync_common

vector_irq_common:
    nop
    b vector_irq_common

vector_fiq_common:
    nop
    b vector_fiq_common

vector_serror_common:
    nop
    b vector_serror_common


/**********************************************************
* 
*  vector_enable_irq()
* 
*  DESCRIPTION:
*      Enable IRQs.
*
*/
.globl vector_enable_irq
vector_enable_irq:
    /* set CPU IRQ mask */
    msr    daifclr, #2
    ret

/**********************************************************
* 
*  vector_disable_irq()
* 
*  DESCRIPTION:
*      Disable IRQs.
*
*/

.globl vector_disable_irq
vector_disable_irq:
    /* clear CPU IRQ mask */
    msr    daifset, #2
    ret

/**********************************************************
* 
*  vector_init()
* 
*  DESCRIPTION:
*      Initialize the vector table.
*
*/

.globl    vector_init
vector_init:
    adr x0, exception_vector_table
    msr vbar_el1, x0
    ret