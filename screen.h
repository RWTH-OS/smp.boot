/*
 * =====================================================================================
 *
 *       Filename:  screen.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  22.02.2012 13:30:35
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef SCREEN_H
#define SCREEN_H

#define COLOR_FG_BLACK        0x00
#define COLOR_FG_BLUE         0x01
#define COLOR_FG_GREEN        0x02
#define COLOR_FG_TEAL         0x03
#define COLOR_FG_RED          0x04
#define COLOR_FG_VIOLETT      0x05
#define COLOR_FG_BROWN        0x06
#define COLOR_FG_SILVER       0x07
#define COLOR_FG_GRAY         0x08
#define COLOR_FG_LIGHT_BLUE   0x09
#define COLOR_FG_LIGHT_GREEN  0x0a
#define COLOR_FG_LIGHT_TEAL   0x0b
#define COLOR_FG_LIGHT_RED    0x0c
#define COLOR_FG_PINK         0x0d
#define COLOR_FG_YELLOW       0x0e
#define COLOR_FG_WHITE        0x0f

#define COLOR_BG_BLACK        0x00
#define COLOR_BG_BLUE         0x01
#define COLOR_BG_GREEN        0x02
#define COLOR_BG_TEAL         0x03
#define COLOR_BG_RED          0x04
#define COLOR_BG_VIOLETT      0x05
#define COLOR_BG_BROWN        0x06
#define COLOR_BG_WHITE        0x07

#define COLOR_BLINK           0x08

#define ATTRIB(fg, bg) (((fg) & 0x0F) | ((bg) << 4))

void cls();
void locate(int abs_x, int rel_y);
void putch(char c);
void status_putch(int x, int c);
void puts(char *str);
void settextcolor(unsigned char forecolor, unsigned char backcolor);
unsigned char gettextattrib();
void settextattrib(unsigned char a);
void init_video();
#if SCROLLBACK_BUF_SIZE
void init_video_scrollback(void);
void video_scrollback(void);
#endif
void itoa (char *buf, int base, long d);
void printf_nomutex(const char *format, ...);
void printf (const char *format, ...);
    //// __attribute__ (( format(printf, 1, 2) ));  <-- this is a bad idea, simply because we're not really compatible...


#endif // SCREEN_H
