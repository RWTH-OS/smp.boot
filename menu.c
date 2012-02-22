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
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "screen.h"
#include "keyboard.h"
#include "menu.h"


int menu(menu_entry_t entries[])
{
    unsigned u;
    unsigned selection = 0;
    unsigned cont = 1;
    while (cont) {
        for (u = 0; ; u++) {
            if (entries[u].string == 0) break;
            if (selection == u) {
                settextcolor(COLOR_FG_BLACK, COLOR_BG_WHITE);
            } else {
                settextcolor(COLOR_FG_WHITE, COLOR_BG_BLACK);
            }
            printf("[%31s]\n", entries[u].string);
        }
        switch (wait_for_key()) {
            case KEY_UP : 
               if (selection <= u) selection++;
               break;
            case KEY_DOWN :
               if (selection > 0) selection--;
               break;
            case '\n' :
               cont = 0;
               break;
        }
        if (! cont) break;
        locate(0, -u);
    }
    return entries[selection].value;
}

