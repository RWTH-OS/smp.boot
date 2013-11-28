/*
 * =====================================================================================
 *
 *       Filename:  menu.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  22.02.2012 13:38:52
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

#include "screen.h"
#include "keyboard.h"
#include "menu.h"
#include "smp.h"
#include "sync.h"


int menu(char * caption, menu_entry_t entries[], int def)
{
    unsigned myid = my_cpu_info()->cpu_id;
    unsigned u;
    unsigned selection = 0;
    unsigned cont = 2;
    unsigned char backup_attrib = gettextattrib();
    static volatile int result = 0;

    barrier(&global_barrier);

    if (myid == 0) {
        settextcolor(COLOR_FG_YELLOW, COLOR_BG_BLUE);
        printf("--%s--\n", caption);

        while (cont) {
            for (u = 0; ; u++) {
                if (entries[u].string == 0) break;
                if (cont == 2 && entries[u].value == def) selection = u;
                if (selection == u) {
                    settextcolor(COLOR_FG_BLUE, COLOR_BG_WHITE);
                } else {
                    settextcolor(COLOR_FG_WHITE, COLOR_BG_BLUE);
                }
                printf("[%31s]", entries[u].string);
                if (selection == u) printf(" **"); else printf("   ");
                printf("\n");
            }
            cont = 1;
            switch (wait_for_key()) {
                case KEY_DOWN : 
                    if (selection < u-1) selection++;
                    break;
                case KEY_UP :
                    if (selection > 0) selection--;
                    break;
                case '\n' :
                    cont = 0;
                    break;
            }
            if (! cont) break;
            locate(0, -u);
        }
        settextattrib(backup_attrib);
        result = entries[selection].value;
    }
    
    barrier(&global_barrier);
    return result;
}

