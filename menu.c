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


int menu(char * caption, menu_entry_t entries[])
{
    unsigned u;
    unsigned selection = 0;
    unsigned cont = 1;
    unsigned char backup_attrib = gettextattrib();

    settextcolor(COLOR_FG_YELLOW, COLOR_BG_BLUE);
    printf("--%s--\n", caption);

    while (cont) {
        for (u = 0; ; u++) {
            if (entries[u].string == 0) break;
            if (selection == u) {
                settextcolor(COLOR_FG_BLUE, COLOR_BG_WHITE);
            } else {
                settextcolor(COLOR_FG_WHITE, COLOR_BG_BLUE);
            }
            printf("[%31s]", entries[u].string);
            if (selection == u) printf(" **"); else printf("   ");
            printf("\n");
        }
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
    return entries[selection].value;
}

