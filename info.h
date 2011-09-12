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
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef INFO_H
#define INFO_H

#include "config.h"

typedef struct {
    uint32_t mb_adr;        /* first position: address of multiboot info structure */

    /* CPUID */
    uint32_t cpuid_max;
    uint32_t cpuid_high_max;
    uint32_t cpuid_family;

    /* BDA and EBDA */
    uint32_t ebda_adr;
    uint32_t ebda_size;

    /* CPUs */
    uint32_t cpu_cnt;
    struct {
        uint32_t lapic_id;
    } cpu[MAX_CPU];
    uint32_t lapic_adr;

    /* I/O APICs */
    uint32_t ioapic_cnt;
    struct {
        uint32_t id;
        uint32_t adr;
    } ioapic[MAX_IOAPIC];

} hw_info_t;


#endif // INFO_H

