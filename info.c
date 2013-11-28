/*
 * =====================================================================================
 *
 *       Filename:  info.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  23.03.2012 11:20:38
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "info_struct.h"
#include "menu.h"
#include "apic.h"

void info_menu(void)
{
    int t = 1;
    do {
        menu_entry_t infomenu[] = {
            {1, "localAPIC"},
            {999, "return"},
            {0,0}
        };
        t = menu("Info", infomenu, t);
        switch (t) {
            case 1 :
                print_apic();
                break;
        }
    } while (t != 999);


}
