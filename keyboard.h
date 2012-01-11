/*
 * =====================================================================================
 *
 *       Filename:  keyboard.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11.01.2012 15:09:58
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stddef.h"

typedef enum {kbm_poll, kbm_interrupt} kb_mode_t;

uint8_t keyboard_get_scancode();
int keyboard_mode(kb_mode_t mode);
int keyboard_init(kb_mode_t mode);

#endif // KEYBOARD_H
