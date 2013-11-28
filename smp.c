/*
 * =====================================================================================
 *
 *       Filename:  smp.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  19.10.2011 13:23:14
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

#include "smp.h"
#include "system.h"
#include "apic.h"
#include "cpu.h"


stack_t stack[MAX_CPU] __attribute__(( aligned(STACK_SIZE) ));

int smp_init(void)
{
    /* this is run before any other CPU (AP) is called */
    unsigned u;
    for (u=0; u<MAX_CPU; u++) {
        stack[u].info.cpu_id = u;
        stack[u].info.flags = 0;
        //if (u<4) printf("stack[%u].info.cpu_id at 0x%x value: %u\n", u, &(stack[u].info.cpu_id), stack[u].info.cpu_id);
        mutex_init(&(stack[u].info.wakelock));  // state: unlocked
    }
    return 0;
}


void smp_status(char c)
{
    status_putch(6+my_cpu_info()->cpu_id, c);
}

void smp_halt(void)
{
    unsigned if_backup;
    smp_status(STATUS_HALT);
    MASK_SET(my_cpu_info()->flags, SMP_FLAG_HALT|SMP_FLAG_HALTED);
    if_backup = sti();
    while (IS_MASK_SET(my_cpu_info()->flags, SMP_FLAG_HALT)) {
        __asm__ volatile ("hlt");
    }
    MASK_CLEAR(my_cpu_info()->flags, SMP_FLAG_HALTED);
    smp_status(STATUS_RUNNING);
    if (!if_backup) cli();
}

void smp_wakeup(unsigned cpu_id)
{
    /* wait until CPU is in halted state (in case it is not there, yet) */
    while (IS_MASK_CLEAR(stack[cpu_id].info.flags, SMP_FLAG_HALTED)) {
        udelay(500);
    }
    /* remove flag */
    MASK_CLEAR(stack[cpu_id].info.flags, SMP_FLAG_HALT);
    udelay(5);
    /* send IPI until it is up */
    while (IS_MASK_SET(stack[cpu_id].info.flags, SMP_FLAG_HALTED)) {
        send_ipi(cpu_id, 128);
        udelay(500);
    }
}

