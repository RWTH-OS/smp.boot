/*
 * =====================================================================================
 *
 *       Filename:  mm.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  21.10.2011 10:27:11
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

#ifndef MM_H
#define MM_H

typedef     unsigned long    frame_t;    // the number of a physical page frame
typedef     unsigned long    page_t;     // the number of a virtual page

int mm_init();
int mm_init_ap();

#define MM_WRITE_THROUGH    0x0010
#define MM_CACHE_DISABLE    0x0020

ptr_t virt_to_phys(void * adr);

void *heap_alloc(unsigned nbr_pages, unsigned flags) __attribute__ ((malloc));
void heap_reconfig(void *p, size_t size, unsigned flags);
void tlb_shootdown(void *adr, size_t size);



#endif // MM_H

