/*
 * =====================================================================================
 *
 *       Filename:  screen.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  24.08.2011 16:44:29
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
