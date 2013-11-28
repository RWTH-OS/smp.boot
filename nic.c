/*
 * =====================================================================================
 *
 *       Filename:  nic.c
 *
 *    Description:  Network Interface Card drivers
 *
 *        Version:  1.0
 *        Created:  11.01.2012 13:27:14
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

#include "pci.h"
#include "system.h"
#include "cpu.h"
#include "config.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_DRIVER > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_DRIVER > 1)

int rtl8139_init(unsigned bus, unsigned slot)
{
    uint16_t iobase;
    iobase = pci_config_read(bus, slot, 0, 0x10);
    if (iobase & 1) {
        // io-port: ignore 2 least significant bits
        iobase &= ~0x03;
    }
    IFV printf("rtl8139: iobase = 0x%x\n", iobase);

    uint8_t mac[6];
    unsigned u;
    for (u=0; u<6; u++) {
        mac[u] = inportb(iobase + u);
    }
    IFV printf("rtl8139: mac = %x:%x:%x:%x:%x:%x\n", 
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return 1;  // the driver was not loaded, so far, we just print some information
}

