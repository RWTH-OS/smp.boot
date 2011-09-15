/*
 * =====================================================================================
 *
 *       Filename:  config.h
 *
 *    Description:  
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
 * verbosity
 */
#define VERBOSE             0
#define VERBOSE_BOOT        0
#define VERBOSE_APIC        2

/*
 * preliminary: CPU Frequency
 * (until it can be correctly measured)
 */
#define TSC_PER_USEC (2666ul)

#endif // CONFIG_H
