/**
 * sysregs.h
 * 
 * This is stolen from the Raspberry Pi 3 Linux kernel source code.
 * See the Arm Architecture Reference Manual Sections D13.2 and C5.2
 * for details on these registers
 * 
 */
#ifndef _SYSREGS_H
#define _SYSREGS_H

// ***************************************
// SCTLR_EL1, System Control Register (EL1), Page 2654 of AArch64-Reference-Manual (D13.2.113)
// ***************************************

#define SCTLR_RESERVED                  (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)
#define SCTLR_EE_LITTLE_ENDIAN          (0 << 25)
#define SCTLR_EOE_LITTLE_ENDIAN         (0 << 24)
#define SCTLR_I_CACHE_DISABLED          (0 << 12)
#define SCTLR_D_CACHE_DISABLED          (0 << 2)
#define SCTLR_MMU_DISABLED              (0 << 0)
#define SCTLR_MMU_ENABLED               (1 << 0)

#define SCTLR_VALUE_MMU_DISABLED	(SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)

// ***************************************
// HCR_EL2, Hypervisor Configuration Register (EL2), Page 2487 of AArch64-Reference-Manual (D13.2.47)
// ***************************************

#define HCR_RW	    			(1 << 31)
#define HCR_VALUE				HCR_RW

// ***************************************
// SCR_EL3, Secure Configuration Register (EL3), Page 2648 of AArch64-Reference-Manual (D13.2.112)
// ***************************************

#define SCR_RESERVED	    	(3 << 4)
#define SCR_RW					(1 << 10)
#define SCR_NS					(1 << 0)
#define SCR_VALUE	    	    (SCR_RESERVED | SCR_RW | SCR_NS)

// ***************************************
// SPSR_EL3, Saved Program Status Register (EL3) Page 389 of AArch64-Reference-Manual (C5.2.19)
// ***************************************

#define SPSR_MASK_ALL 			(7 << 6)
#define SPSR_EL1h				(5 << 0)
#define SPSR_EL2h				(9 << 0)	// according to Page 393 of AArch64-Reference-Manual.
#define SPSR_VALUE_EL1			(SPSR_MASK_ALL | SPSR_EL1h) // SPSR value to boot into EL1
#define SPSR_VALUE_EL2			(SPSR_MASK_ALL | SPSR_EL2h)

#endif
