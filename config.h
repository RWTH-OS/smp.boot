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
 *         Author:  Georg Wassen (gw) (wassen@lfbs.rwth-aachen.de), 
 *        Company:  Lehrstuhl für Betriebssysteme (Chair for Operating Systems)
 *                  RWTH Aachen University
 *
 * Copyright (c) 2011, Georg Wassen, RWTH Aachen University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of the University nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
 * maximum number of PCI Express config spaces
 */
#define MAX_PCIE  1

/*
 * maximum number of Cache levels in hw_info.cpuid_cache[]
 */
#define MAX_CACHE 4

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
#define VERBOSE_ISR         1
#define VERBOSE_PIT         0
#define VERBOSE_SMM         0
#define VERBOSE_PCI         0
#define VERBOSE_KEYBOARD    0
#define VERBOSE_DRIVER      0
#define VERBOSE_TESTS       2
#define VERBOSE_MAIN        0
/*
 * OFFER_MENU - menu selection of kernel activities
 *     0 - no menu; automatic/keyboard-less mode
 *     1 - hit a key within 5 seconds to get to menu mode
 *     2 - enforce menu (no fallback without keyboard)
 */
#define OFFER_MENU          2


/*
 * scrollback buffer
 * (set to 0 to deactivate)
 */
#define SCROLLBACK_BUF_SIZE   (1024*80)


/*
 * preliminary: CPU Frequency
 * (until it can be correctly measured)
 */
#define TSC_PER_USEC (2666ul)


/*
 * settings for benchmarks
 */
#if 0
/* real workload for benchmarks (set #if 1) */

#define BENCH_WORK_FLAGS          0
//#define BENCH_WORK_FLAGS          (MM_WRITE_THROUGH)
//#define BENCH_WORK_FLAGS          (MM_CACHE_DISABLE)

//#define BENCH_LOAD_FLAGS          0
#define BENCH_LOAD_FLAGS          (MM_WRITE_THROUGH)
//#define BENCH_LOAD_FLAGS          (MM_CACHE_DISABLE)

#define BENCH_HOURGLASS_SEC     10u
#define BENCH_MIN_STRIDE_POW2   4  
#define BENCH_MAX_STRIDE_POW2   9
#define BENCH_MIN_RANGE_POW2    12
#define BENCH_MAX_RANGE_POW2    25
#define BENCH_RANGESTRIDE_REP   (512*1024*1024)
#else
/* short workload for quick testing (set #if 0) */
#define BENCH_WORK_FLAGS          0
#define BENCH_LOAD_FLAGS          0
#define BENCH_HOURGLASS_SEC      2u 
#define BENCH_MIN_STRIDE_POW2   4  
#define BENCH_MAX_STRIDE_POW2   5
#define BENCH_MIN_RANGE_POW2    12
#define BENCH_MAX_RANGE_POW2    13
#define BENCH_RANGESTRIDE_REP   (256*1024*1024)
#endif

#endif 
