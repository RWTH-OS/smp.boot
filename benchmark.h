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
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef BENCHMARK_H
#define BENCHMARK_H



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
