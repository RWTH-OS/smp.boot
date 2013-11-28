/*
 * =====================================================================================
 *
 *       Filename:  cpu.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.11.2011 14:05:00
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

#ifndef CPU_H
#define CPU_H

#include "system.h"

/* We will use this later on for reading from the I/O ports to get data
*  from devices such as the keyboard. We are using what is called
*  'inline assembly' in these routines to actually do the work */
static inline unsigned char inportb (unsigned short _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}
static inline uint32_t inportl (unsigned short _port)
{
    uint32_t rv;
    __asm__ __volatile__ ("inl %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

/* We will use this to write to I/O ports to send bytes to devices. This
*  will be used in the next tutorial for changing the textmode cursor
*  position. Again, we use some inline assembly for the stuff that simply
*  cannot be done in C */
static inline void outportb (unsigned short _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}
static inline void outportl (unsigned short _port, uint32_t _data)
{
    __asm__ __volatile__ ("outl %1, %0" : : "dN" (_port), "a" (_data));
}

static inline void halt()
{
    printf("System halted.");
    while (1) {
        __asm__ volatile ("hlt");
    }
}



/*
 * rdmsr(), wrmsr() 
 * from perfcount.c (by Christian Spoo)
 */
static uint64_t rdmsr(uint32_t msr) {
  uint32_t low, high;

  __asm__ __volatile__ ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
  return ((uint64_t)high << 32) | low;
}

static void wrmsr(uint32_t msr, uint64_t value) {
  uint32_t low = value & 0xFFFFFFFF;
  uint32_t high = value >> 32;

  __asm__ __volatile__ ("wrmsr" :: "a"(low), "c"(msr), "d"(high));
}





void reboot();
void stop();

inline static unsigned sti(void) {
    ptr_t flags;
    __asm__ volatile ("pushf; sti; pop %0" : "=r"(flags) : : "memory");
    return (flags & (1<<9));
}

/* return if IF was previously set (to use sti afterwards conditionally) */
inline static unsigned cli(void) {
    ptr_t flags;
    __asm__ volatile ("pushf; cli; pop %0" : "=r"(flags) : : "memory");
    return (flags & (1<<9));
}

inline static void mfence(void) 
{
    __asm__ volatile ("mfence");
}

inline static void cpuid(uint32_t func, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile ("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(func));
}

inline static void cpuid2(uint32_t func, uint32_t subfunc, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ volatile ("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "a"(func), "c"(subfunc));
}

inline static uint32_t cpuid_eax(uint32_t code) {
    uint32_t eax, ebx, ecx, edx;

    cpuid(code, &eax, &ebx, &ecx, &edx);

    return eax;
}

inline static uint32_t cpuid_ebx(uint32_t code) {
    uint32_t eax, ebx, ecx, edx;

    cpuid(code, &eax, &ebx, &ecx, &edx);

    return ebx;
}

inline static uint32_t cpuid_ecx(uint32_t code) {
    uint32_t eax, ebx, ecx, edx;

    cpuid(code, &eax, &ebx, &ecx, &edx);

    return ecx;
}

inline static uint32_t cpuid_edx(uint32_t code) {
    uint32_t eax, ebx, ecx, edx;

    cpuid(code, &eax, &ebx, &ecx, &edx);

    return edx;
}


#endif //  CPU_H
