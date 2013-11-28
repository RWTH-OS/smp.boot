/*
 * =====================================================================================
 *
 *       Filename:  sync.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  19.10.2011 16:11:57
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

#ifndef SYNC_H
#define SYNC_H

#include "types.h"

typedef volatile int mutex_t;
#define MUTEX_INITIALIZER           ((mutex_t)1)
#define MUTEX_INITIALIZER_LOCKED    ((mutex_t)0)
void mutex_init(mutex_t *m);
void mutex_lock(mutex_t *m);
void mutex_unlock(mutex_t *m);
int mutex_trylock(mutex_t *m);

typedef struct {
    volatile unsigned cnt;
    volatile unsigned epoch;
    volatile unsigned max;
} barrier_t;
//#define BARRIER_INITIALIZER(m)  (barrier_t){ .cnt=0, .epoch=0, .max=m };
#define BARRIER_INITIALIZER(m)  { .cnt=0, .epoch=0, .max=m };
extern barrier_t global_barrier;
void barrier_init(barrier_t *b, int max);
void barrier(barrier_t *b);

typedef struct {
    volatile unsigned flag;
    volatile unsigned next;
} flag_t;
//#define FLAG_INITIALIZER  (flag_t){ .flag=0, .next=0 };
#define FLAG_INITIALIZER  { .flag=0, .next=0 };
void flag_init(flag_t *flag);
void flag_signal(flag_t *flag);
void flag_wait(flag_t *flag);
int flag_trywait(flag_t *flag);

unsigned collective_only(cpumask_t mask);
void collective_end();

#endif  // SYNC_H

