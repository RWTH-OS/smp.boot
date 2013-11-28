/*
 * =====================================================================================
 *
 *       Filename:  lib.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  29.08.2011 08:02:40
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


#include "system.h"

/*
 * This file is used in 32-Bit Boot Code (REAL MODE) called from startXX.asm and boot32.c
 * as well as in the final kernel (main.c etc.)
 */

void *memcpy(void *dest, const void *src, int count)
{
    /* copy 'count' bytes of data from 'src' to 'dest', finally return 'dest' */
    char *dp = (char *)dest;
    char *sp = (char *)src;
    for(; count != 0; count--) *dp++ = *sp++;
    return dest;
}

void *memset(void *dest, int val, int count)
{
    /* set 'count' bytes in 'dest' to 'val'.  Again, return 'dest' */
    char *temp = (char *)dest;
    for( ; count != 0; count--) *temp++ = (char)val;
    return dest;
}

unsigned short *memsetw(unsigned short *dest, unsigned short val, int count)
{
    /* Same as above, but this time, we're working with a 16-bit
    *  'val' and dest pointer. Your code can be an exact copy of
    *  the above, provided that your local variables if any, are
    *  unsigned short */
    unsigned short *temp = (unsigned short *)dest;
    for( ; count != 0; count--) *temp++ = val;
    return dest;
}

int strlen(const char *str)
{
    /* This loops through character array 'str', returning how
    *  many characters it needs to check before it finds a 0.
    *  In simple words, it returns the length in bytes of a string */
    int retval;
    for(retval = 0; *str != '\0'; str++) retval++;
    return retval;
}

int strcmp(const char *a, const char *b) 
{
    while (*a != 0 && *b != 0 && (*a == *b)) {
        a++;
        b++;
    }
    return (*a - *b);
}
int strncmp(const char *a, const char *b, int n) 
{
    while (n > 0 && *a != 0 && *b != 0 && (*a == *b)) {
        n--;
        a++;
        b++;
    }
    return n==0?0:(*a - *b);
}

/*
 * can handle decimal (not starting with 0), octal (starting with zero), hex (starting with 0x) and negative values (starting with -).
 */
int atoi(const char *a)
{
    int i = 0;
    int s = 1;
    int base = 10;
    if (*a == '-') {
        s = -1;
        a++;
    }
    if (*a == '0') {
        base = 8;
        a++;
        if (*a == 'x' || *a == 'X') {
            base = 16;
            a++;
        }
    }
    while ((*a >= '0' && *a <= '0'+(base>10?10:base)-1) || (base>10 && ((*a >= 'A' && *a <= 'A'+base-11) || (*a >= 'a' && *a <= 'a'+base-11)))) {
        i *= base;
        if (*a <= '9')
            i += *a - '0';
        else if (*a >= 'a')
            i += *a - 'a'+10;
        else
            i += *a - 'A'+10;
        a++;
    }
    i *= s;
    return i;
}


int abs(int j)
{
    if (j >= 0) return j;
    else return -j;
}

long labs(long j)
{
    if (j >= 0) return j;
    else return -j;
}

#if ! __x86_64__
/*
 * divide u64/u64 (needed for rdtsc)
 * In 32 bit mode, this can't be done directly and the compiler need this function.
 * Here, we don't use the FPU or SSE, but shift the arguments into 32 bit,
 * divide in 32 bit and restore the result to 64 bits (using number of places shifted before)
 */
uint64_t __udivdi3(uint64_t n, uint64_t d)
{
    uint32_t n32, d32, r32, c=0;
    uint64_t r;
    while (n > 0xFFFFFFFFull) {
        n >>= 1;
        c++;
    }
    n32 = (uint32_t)(n & 0xFFFFFFFFull);
    while (d > 0xFFFFFFFFull && c >= 1) {
        d >>= 1;
        c--;
    }
    d32 = (uint32_t)(d & 0xFFFFFFFFull);
    r32 = n32/d32;
    r = (uint64_t)r32 << c;
    return r;
}

#endif
