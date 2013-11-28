/*
 * =====================================================================================
 *
 *       Filename:  smp.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  19.10.2011 13:23:26
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

#ifndef SMP_H
#define SMP_H

#include "system.h"
#include "config.h"
#include "sync.h"

/*
 * per-cpu info structure
 */
#define SMP_FLAG_HALT   (1u <<0)
#define SMP_FLAG_HALTED (1u <<1)
typedef struct {
    unsigned cpu_id;   
    volatile unsigned flags;
    mutex_t wakelock;
} cpu_info_t;


/*
 * Stack (growing downwards with per-cpu info structure on opposite (lower) end, where the stack should never grow to)
 */
typedef union {
    unsigned stack[STACK_FRAMES * 4096 / sizeof(unsigned)];
    cpu_info_t info;
} stack_t; // __attribute__((packed));

extern stack_t stack[MAX_CPU];

int smp_init(void);

static inline volatile cpu_info_t * my_cpu_info()
{
    void volatile * p;
#   ifdef __x86_64__
    __asm__ volatile("movq %%rsp, %%rax \n\t andq %%rdx, %%rax" : "=a"(p) : "d" ( ~STACK_MASK ));
#   else
    __asm__ volatile("movl %%esp, %%eax \n\t andl %%edx, %%eax" : "=a"(p) : "d" ( ~STACK_MASK ));
#   endif
    return (cpu_info_t volatile * )p;
}
#define CPU_ID (my_cpu_info()->cpu_id)

#define STATUS_WAKEUP   '^'
#define STATUS_RUNNING  '.'
#define STATUS_MUTEX    'm'
#define STATUS_BARRIER  'b'
#define STATUS_FLAG     'f'
#define STATUS_DELAY    'd'
#define STATUS_HALT     '-'
#define STATUS_ERROR    'E'
#define STATUS_STOP     '_'
#define STATUS_NOTUP    '|'

void smp_status(char c);

void smp_halt(void);
void smp_wakeup(unsigned cpu_id);

#endif // SMP_H

