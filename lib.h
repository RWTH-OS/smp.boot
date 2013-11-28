/*
 * =====================================================================================
 *
 *       Filename:  lib.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12.09.2011 13:43:07
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

#ifndef LIB_H
#define LIB_H

// note: var and bit are evaluated only once!
#define BIT_SET(var, bit)       ((var) |=  (1 << (bit)))
#define BIT_CLEAR(var, bit)     ((var) &= ~(1 << (bit)))
#define MASK_SET(var, mask)       ((var) |=  (mask))
#define MASK_CLEAR(var, mask)     ((var) &= ~(mask))
#define IS_BIT_SET(var, bit)    (((var) & (1 << bit)) != 0)
#define IS_BIT_CLEAR(var, bit)  (((var) & (1 << bit)) == 0)
#define IS_MASK_SET(var, mask)    (((var) & (mask)) != 0)
#define IS_MASK_CLEAR(var, mask)  (((var) & (mask)) == 0)

/*
 * MASK(n) - create bit-mask of n 1s
 * Example: MASK(3) returns 0b0111 = 0x07
 *          MASK(8) returns 0x1111_1111 = 0xFF
 * Attention: Overflow, don't use with 32 (on x86_32) or 64 (on x86_64)
 *
 */
#define MASK(n)  ((1<<(n))-1)

/*
 * BITS_FROM_CNT(reg, from, cnt) - extracts cnt bits from reg(ister) beginning at position from 
 * BITS_FROM_TO(reg, from, to) - extracts the bits from reg(ister) from:to (from<to, both including)
 * Example: BITS_FROM_TO(0x12345678, 8, 15)  -> 0x56
 *          BITS_FROM_CNT(0xdeadbeef, 8, 16) -> 0xadbe
 */
#define BITS_FROM_CNT(reg, from, cnt)   (((reg) >> (from)) & MASK(cnt))
#define BITS_FROM_TO(reg, from, to)     BITS_FROM_CNT((reg), (from), ((to)-(from)+1))


void *memcpy(void *dest, const void *src, int count);
void *memset(void *dest, int val, int count);
unsigned short *memsetw(unsigned short *dest, unsigned short val, int count);
int strlen(const char *str);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, int n);
int atoi(const char *a);

int abs(int j);
long labs(long j);

#if ! __x86_64__
uint64_t __udivdi3(uint64_t n, uint64_t d);
#endif

#endif // LIB_H
