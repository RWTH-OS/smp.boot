/*
 * =====================================================================================
 *
 *       Filename:  config.h
 *
 *    Description:  configuration for this kernel.
 *                  The defines are also made available as config.inc for assembler code.
 *
 *        Version:  1.0
 *        Created:  11.09.2011 10:23:32
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef CONFIG_H
#define CONFIG_H

/*
 * number of supported CPUs (maximum)
 */
#define MAX_CPU   16

/*
 * number of supported I/O APICs (maximum)
 */
#define MAX_IOAPIC 1

/*
 * page frame for SMP startup code (phys. adr. is SMP_FRAME << 12)
 */
#define SMP_FRAME  0x88

/*
 * Stack size for each CPU (number of frames (4 kB); total stack size is 4096 * STACK_FRAMES))
 */
#define STACK_FRAMES   2

/*
 * verbosity
 */
#define VERBOSE             0
#define VERBOSE_BOOT        0
#define VERBOSE_APIC        1

/*
 * preliminary: CPU Frequency
 * (until it can be correctly measured)
 */
#define TSC_PER_USEC (2666ul)

#endif 
