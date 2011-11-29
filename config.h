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
 * maximum supported memory (currently 2 GB)
 * ATTN: ulong is 32 bit on __x86_32__, so 4GB==0 (overflow), max value would be 4GB-1, but that's not page-aligned...
 */
#define MAX_MEM    (2ul*1024*1024*1024)

/*
 * Stack size for each CPU (number of frames (4 kB); total stack size is 4096 * STACK_FRAMES))
 */
#define STACK_FRAMES   2

/*
 * verbosity: set to 0 (off), 1 (normal), 2 (chatter)
 * VERBOSE (first line) has global effect over all subsequent settings
 */
#define VERBOSE             0
#define VERBOSE_BOOT        0
#define VERBOSE_APIC        0
#define VERBOSE_SYNC        0
#define VERBOSE_MM          0
#define VERBOSE_ISR         0
#define VERBOSE_PIT         0
#define VERBOSE_SMM         0
#define VERBOSE_TESTS       2
#define VERBOSE_MAIN        0

/*
 * preliminary: CPU Frequency
 * (until it can be correctly measured)
 */
#define TSC_PER_USEC (2666ul)


/*
 * settings for benchmarks
 */
#if 1
/* real workload for benchmarks (set #if 1) */
#define BENCH_HOURGLAS_SEC     10u
//#define BENCH_MIN_STRIDE_POW2   4  
#define BENCH_MIN_STRIDE_POW2   6  
//#define BENCH_MAX_STRIDE_POW2   9
#define BENCH_MAX_STRIDE_POW2   7
#define BENCH_MIN_RANGE_POW2    12
//#define BENCH_MAX_RANGE_POW2    25
#define BENCH_MAX_RANGE_POW2    16
#else
/* short workload for quick testing (set #if 0) */
#define BENCH_HOURGLAS_SEC      1u 
#define BENCH_MIN_STRIDE_POW2   4  
#define BENCH_MAX_STRIDE_POW2   5
#define BENCH_MIN_RANGE_POW2    12
#define BENCH_MAX_RANGE_POW2    13
#endif

#endif 
