/*
 * =====================================================================================
 *
 *       Filename:  keyboard.c
 *
 *    Description:  keyboard driver (PS/2)
 *
 *        Version:  1.0
 *        Created:  11.01.2012 15:09:42
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "keyboard.h"
#include "system.h"
#include "cpu.h"

/*
 * http://lowlevel.eu/wiki/KBC
 */

#define KBC_STATUS   0x64
#define KBC_EA       0x60

/*
 * send command to keyboard
 */
static void send_command(uint8_t command)
{
    // wait until cmd buffer is free
    while ((inportb(KBC_STATUS) & 2)) {}
    outportb(KBC_EA, command);
}

/*
 * get scan code (can be called to poll or by interrupt handler)
 */
uint8_t keyboard_get_scancode()
{
    static unsigned e0_code = 1;
    static unsigned e1_code = 0;
    static uint16_t e1_prev = 0;

    uint8_t scancode = 0;
   
    if (inportb(KBC_STATUS) & 1) {
        scancode = inportb(KBC_EA);
        if (scancode == 0xe0) {
            scancode <<= 8;
            // TODO....

        }
    }

    return scancode;
}
uint8_t scancode_to_keycode(uint16_t scancode)
{
    switch (scancode) {
        case 0x02 : return '1'; break;
        case 0x03 : return '2'; break;
        case 0x04 : return '3'; break;
        case 0x05 : return '4'; break;
        case 0x06 : return '5'; break;
        case 0x07 : return '6'; break;
        case 0x08 : return '7'; break;
        case 0x09 : return '8'; break;
        case 0x0a : return '9'; break;
        case 0x0b : return '0'; break;

        case 0x10 : return 'q'; break;
        case 0x11 : return 'w'; break;
        case 0x12 : return 'e'; break;
        case 0x13 : return 'r'; break;
        case 0x14 : return 't'; break;
        case 0x15 : return 'z'; break;
        case 0x16 : return 'u'; break;
        case 0x17 : return 'i'; break;
        case 0x18 : return 'o'; break;
        case 0x19 : return 'p'; break;
        case 0x1c : return '\n'; break;

        case 0x1e : return 'a'; break;
        case 0x1f : return 's'; break;
        case 0x20 : return 'd'; break;
        case 0x21 : return 'f'; break;
        case 0x22 : return 'g'; break;
        case 0x23 : return 'h'; break;
        case 0x24 : return 'j'; break;
        case 0x25 : return 'k'; break;
        case 0x26 : return 'l'; break;

        case 0x2c : return 'y'; break;
        case 0x2d : return 'x'; break;
        case 0x2e : return 'c'; break;
        case 0x2f : return 'v'; break;
        case 0x30 : return 'b'; break;
        case 0x31 : return 'n'; break;
        case 0x32 : return 'm'; break;
        case 0x39 : return ' '; break;
    }
    return 0;
}

int keyboard_mode(kb_mode_t mode)
{
    if (mode == kbm_poll) {
        /* remove keyboard interrupt handler */
        return 0;
    } else {
        /* install keyboard interrupt handler */
        printf("sorry, interrupt based keyboard mode not supported, yet.\n");
        return 1;
    }
    return 0;
}

int keyboard_init(kb_mode_t mode)
{
    keyboard_mode(mode);
    
    // empty keyboard buffer
    while (inportb(KBC_STATUS) & 1) {
        inportb(KBC_EA);
    }   
 
    // activate keyboard
    send_command(0xF4);

    return 0;
}

