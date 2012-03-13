/*
 * =====================================================================================
 *
 *       Filename:  menu.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  22.02.2012 13:40:43
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef MENU_H
#define MENU_H

typedef struct {
    int value;
    char *string;
} menu_entry_t;

int menu(char * caption, menu_entry_t entries[]);


#endif // MENU_H
