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
void load_until_flag(void *buffer, size_t size, flag_t *flag);
uint64_t range_stride(void *buffer, size_t range, size_t stride);


#endif  // BENCHMARK_H
