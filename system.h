/*
 * =====================================================================================
 *
 *       Filename:  system.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  24.08.2011 14:44:29
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

#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <stddef.h>
#include "config.h"
#include "info_struct.h"
#include "time.h"

/* Verbosity: if not defined here, modules may define their own */
//#define VERBOSE 0

#define PAGE_BITS    12
#define PAGE_SIZE    (1<<PAGE_BITS)
#define PAGE_MASK    (PAGE_SIZE-1)
#if __x86_64__
#   define INDEX_BITS   9
#else
#   define INDEX_BITS  10
#endif
#define INDEX_MASK    ((1<<INDEX_BITS) -1)

#define STACK_SIZE    ((ptr_t)STACK_FRAMES * PAGE_SIZE)
#define STACK_MASK    (STACK_SIZE-1)

#if __x86_64__
struct regs
{
	unsigned long long cr2, cr3, gs, fs, es, ds;				/* pushed the segs last */
	unsigned long long r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, _zero, rbx, rdx, rcx, rax ;
	unsigned long long int_no, err_code;			/* our 'push byte #' and ecodes do this */
	unsigned long long rip, cs, rflags, rsp, ss;			/* pushed by the processor automatically */
} __attribute__((packed));
#else
struct regs
{
    unsigned int cr2, cr3, gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cx, eflags, useresp, ss;
};
#endif


/*
 * int i;
 * int array[] = {1,2,3,4};
 * foreach(i, array) {
 *   printf("%i\n", i);
 * }
 */
#define foreach(item, array) \
        for(int keep = 1, \
                count = 0,\
                size = sizeof (array) / sizeof *(array); \
                keep && count != size; \
                keep = !keep, count++) \
        for(item = array[count]; keep; keep = !keep)
//#define foreach(item, array) \-
//    for (int count=0, size = sizeof(array)/sizeof *(array), item=array[count]; count < size; count++)




/* lib.c */
#include "lib.h"

#define NULL ((void*)0)

/* idt.c */
void idt_install();
void idt_install_ap();
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);

/* isr.c */
void isr_install();

/* apic.c */
#include "apic.h"

#include "screen.h"

#endif
