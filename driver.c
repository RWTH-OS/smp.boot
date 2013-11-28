/*
 * =====================================================================================
 *
 *       Filename:  driver.c
 *
 *    Description:  interface for device drivers
 *
 *        Version:  1.0
 *        Created:  11.01.2012 10:23:33
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (wassen@lfbs.rwth-aachen.de), 
 *        Company:  Lehrstuhl für Betriebssysteme (Chair for Operating Systems)
 *                  RWTH Aachen University
 *
 * Copyright (c) 2012, Georg Wassen, RWTH Aachen University
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


#include "config.h"
#include "system.h"
#include "pci.h"
#include "stddef.h"
#include "nic.h"

#define IFV   if (VERBOSE > 0 || VERBOSE_DRIVER > 0)
#define IFVV  if (VERBOSE > 1 || VERBOSE_DRIVER > 1)

int driver_check_pci(uint16_t vendor, uint16_t device, unsigned bus, unsigned slot)
{
    uint32_t vendev = (uint32_t)vendor << 16 | device;
    switch (vendev) {
        case 0x10ec8139 :
            // found in qemu (32 and 64)
            IFV printf("found: Realtek RTL 8139 Fast Ethernet NIC (%x:%x)\n", bus, slot);
            rtl8139_init(bus, slot);
            return 1;
            break;
        case 0x808610cc :
            // found on xaxis
            IFV printf("found: Intel Ethernet NIC (%x:%x)\n", bus, slot);
            // call driver_init
            return 1;
            break;
        default:
            IFVV printf("unknown vendor/device.\n");
    }
    return 0;
}
