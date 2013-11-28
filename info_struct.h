/*
 * =====================================================================================
 *
 *       Filename:  info.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11.09.2011 13:51:42
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

#ifndef INFO_STRUCT_H
#define INFO_STRUCT_H

#include "stddef.h"
#include "config.h"

typedef enum {vend_unknown, vend_intel, vend_amd} vendor_t;

extern char *vendor_string[];       // in cpu.c

typedef struct {
    uint32_t mb_adr;        /* first position: address of multiboot info structure */

    /* MULTIBOOT */
    uint32_t cmd_maxcpu;
    uint32_t cmd_cpumask;
    uint32_t cmd_noacpi;
    uint32_t cmd_nomps;

    /* CPUID */
    uint32_t cpuid_max;
    uint32_t cpuid_high_max;
    uint32_t cpuid_family;
    uint32_t cpuid_cachelinesize;
    uint32_t cpuid_lapic_id;
    uint32_t maxphyaddr;
    vendor_t cpu_vendor;
    union {
        char c[48];
        uint32_t u32[12];
    } cpuid_processor_name;
    uint16_t cpuid_threads_per_package;
    struct {
        uint8_t level;
        char type;      // Data, Instruction, Unified
        uint8_t shared_by;
        uint8_t line_size;
        uint32_t size;
        // associativity... (n-way)?
    } cpuid_cache[MAX_CACHE];

    /* BDA and EBDA */
    uint32_t ebda_adr;
    uint32_t ebda_size;

    /* CPUs */
    uint32_t cpu_cnt;
    struct {
        uint32_t lapic_id;
    } cpu[MAX_CPU];
    uint32_t lapic_adr;

    /* BUS */
    uint32_t bus_isa_id;        /* what bus-id has the ISA bus (for redirecting the IRQs) */

    /* I/O APICs */
    uint32_t ioapic_cnt;
    struct {
        uint32_t id;
        uint32_t adr;
    } ioapic[MAX_IOAPIC];

    /* PCI-Express Config Space */
    uint32_t pcie_cnt;
    struct {
        ptr_t base_adr;
        uint16_t grp_nbr;
        uint8_t bus_start;
        uint8_t bus_end;
    } pcie_cfg[MAX_PCIE];

    /* TSC */
    //uint64_t tsc_per_sec;
    uint32_t tsc_per_usec;      
    uint32_t usec_per_mtsc;
                                /* ATTN: when using uint64_t (as in tsc_per_sec),
                                 * this structure behaves differently in 32 bit and 64 bit.
                                 * And it is used from boot32.c in 32 bit mode and later in the 64 bit kernel also. 
                                 * this is why __attribute__((packed)) is needed on the struct! */

} __attribute__((packed)) hw_info_t;

extern hw_info_t hw_info; 

#endif // INFO_STRUCT_H

