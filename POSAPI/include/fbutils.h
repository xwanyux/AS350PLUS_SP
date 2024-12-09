/*
 * fbutils.h
 *
 * Headers for utility routines for framebuffer interaction
 *
 * Copyright 2002 Russell King and Doug Lowder
 *
 * This file is placed under the GPL.  Please see the
 * file COPYING for details.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#ifndef _FBUTILS_H
#define _FBUTILS_H

#include <stdint.h>
#include "OS_FONT.h"
#include "POSAPI.h"
#include "LCDTFTAPI.h"
/* This constant, being ORed with the color index tells the library
 * to draw in exclusive-or mode (that is, drawing the same second time
 * in the same place will remove the element leaving the background intact).
 */
#define XORMODE	0x80000000

extern uint32_t xres, yres;
extern uint32_t xres_orig, yres_orig;
extern int8_t rotation;
extern int8_t alternative_cross;

UCHAR put_graphic(UINT x,UINT y,UINT graphWidth,UINT graphHeight,UCHAR *pData);
UCHAR put_graphic_BMP555(UINT x,UINT y,UINT graphWidth,UINT graphHeight,UCHAR *pData);
UCHAR open_framebuffer(void);
void close_framebuffer(void);
void setcolor(unsigned colidx, unsigned value);
void put_cross(int x, int y, unsigned colidx);
void put_string(int32_t x, int32_t y, char *s, API_LCDTFT_PARA para, ULONG fontLen);
void put_Bg5string(int32_t x, int32_t y, UCHAR *s, API_LCDTFT_PARA para,ULONG fontLen);
void put_string_center(int x, int y, char *s, unsigned colidx);
void pixel(int x, int y, unsigned colidx);
void line(int x1, int y1, int x2, int y2, unsigned colidx);
void rect(int x1, int y1, int x2, int y2, unsigned colidx);
void fillrect(int x1, int y1, int x2, int y2, unsigned colidx);
void getpixelvalue(int32_t x1, int32_t y1, int32_t x2, int32_t y2, UCHAR* pData);

#endif /* _FBUTILS_H */
