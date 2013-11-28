/*
 * =====================================================================================
 *
 *       Filename:  benchmark.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.11.2011 14:19:40
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

#ifndef BENCHMARK_H
#define BENCHMARK_H

typedef enum {cm_cache_disable, cm_write_back, cm_write_through} cachemode_t;
typedef struct bench_opt_s {
    cachemode_t cm_buffer;
    cachemode_t cm_contender;
    unsigned timebase;
} bench_opt_t;
extern bench_opt_t bench_opt;

void hourglass(unsigned sec);
void load_until_flag(void *buffer, size_t size, size_t stride, flag_t *flag);
uint64_t range_stride(void *buffer, size_t range, size_t stride, uint64_t *p_pc0);

typedef enum {AT_READ, AT_WRITE, AT_UPDATE, AT_ATOMIC} access_t;
void worker(volatile unsigned long *p_buffer, size_t range, size_t stride, access_t type, unsigned sec);


void bench_hourglass();
void bench_hourglass_worker(void *p_contender);
void bench_hourglass_hyperthread();
void bench_worker(void *p_buffer, void *p_contender);
void bench_worker_cut(void *p_buffer, void *p_contender, size_t worker_size);
void bench_rangestride(void *p_buffer);
void bench_mem(void *p_buffer, void *p_contender);

#endif  // BENCHMARK_H
