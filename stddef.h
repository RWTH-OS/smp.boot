/*
 * =====================================================================================
 *
 *       Filename:  stddef.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02.09.2011 14:04:43
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

#ifndef STDDEF_H
#define STDDEF_H

#if __x86_64__
    typedef unsigned char      u8;
    typedef unsigned short     u16;
    typedef unsigned int       u32;
    typedef unsigned long long u64;
#else
    typedef unsigned char      u8;
    typedef unsigned short     u16;
    typedef unsigned int       u32;
    typedef unsigned long long u64;
#endif

typedef u8  uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

typedef unsigned long size_t;   // size of a pointer; 32/64 bit safe
typedef unsigned long ptr_t;    // size of a pointer; 32/64 bit safe

#define KB  (1024)
#define MB  (1024 * KB)
#define GB  (1024 * MB)

#endif
