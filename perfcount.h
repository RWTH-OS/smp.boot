/*
 * =====================================================================================
 *
 *       Filename:  perfcount.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  14.02.2012 10:02:12
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Christian Spoo (cs) (spoo@lfbs.rwth-aachen.de), 
 *        Company:  Lehrstuhl für Betriebssysteme (Chair for Operating Systems)
 *                  RWTH Aachen University
 *
 * Copyright (c) 2012, Christian Spoo, RWTH Aachen University
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

#ifndef PERFCOUNT_H
#define PERFCOUNT_H

#include "stddef.h"

#define PERFCOUNT_L1DATA    (uint64_t)0xFF000151ull
#define PERFCOUNT_L2        (uint64_t)0xFF000224ull
#define PERFCOUNT_L3        (uint64_t)0xFF000309ull

void perfcount_init(unsigned int counter, uint64_t config);
uint64_t perfcount_raw(uint8_t event, uint8_t umask);

void perfcount_start(unsigned int counter);
void perfcount_stop(unsigned int counter);
void perfcount_reset(unsigned int counter);
uint64_t perfcount_read(unsigned int counter);

#endif

