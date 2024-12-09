/*
 * fbutils-linux.c
 *
 * Utility routines for framebuffer interaction
 *
 * Copyright 2002 Russell King and Doug Lowder
 *
 * This file is placed under the GPL.  Please see the
 * file COPYING for details.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
/*
 *add choose font function 20200604 by west chu
 */
 
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <linux/fb.h>

#include "fbutils.h"
#include "OS_FONT.h"
#include "POSAPI.h"
#include "DEV_LCD.h"

union multiptr {
	uint8_t *p8;
	uint16_t *p16;
	uint32_t *p32;
};

static int32_t con_fd, last_vt = -1;
static struct fb_fix_screeninfo fix;
static struct fb_var_screeninfo var;
static unsigned char *fbuffer;
static unsigned char **line_addr;
static int32_t fb_fd;
static int32_t bytes_per_pixel;
static uint32_t transp_mask;
static uint32_t colormap[256];
uint32_t xres, yres;
uint32_t xres_orig, yres_orig;
int8_t rotation;
int8_t alternative_cross;

static char *defaultfbdevice = "/dev/fb0";
static char *defaultconsoledevice = "/dev/tty";
static char *fbdevice;
static char *consoledevice;
#define VTNAME_LEN 128
extern UINT8  Alphanumeric2BIG5[127][2];
extern	unsigned char ext_font816[];
OS_FDT		FDT[MAX_FDT_NO];
UCHAR open_framebuffer(void)
{
	struct vt_stat vts;
	char vtname[VTNAME_LEN];
	int32_t fd, nr;
	uint32_t y, addr;

	if ((fbdevice = getenv("TSLIB_FBDEVICE")) == NULL)
		fbdevice = defaultfbdevice;

	if ((consoledevice = getenv("TSLIB_CONSOLEDEVICE")) == NULL)
		consoledevice = defaultconsoledevice;

	if (strcmp(consoledevice, "none") != 0) {
		if (strlen(consoledevice) >= VTNAME_LEN){
			perror("console name > VTNAME_LEN");
			printf("%s",consoledevice);
			return 0;
		}
		sprintf(vtname, "%s%d", consoledevice, 1);
		fd = open(vtname, O_WRONLY);
		if (fd < 0) {
			perror("open vtname fail");
			return 0;
		}

		if (ioctl(fd, VT_OPENQRY, &nr) < 0) {
			perror("ioctl fail");
			close(fd);
			return 0;
		}
		close(fd);
		sprintf(vtname, "%s%d", consoledevice, nr);
		con_fd = open(vtname, O_RDWR | O_NDELAY);
		if (con_fd < 0) {
			perror("con_fd open vtname fail");
			return 0;
		}

		if (ioctl(con_fd, VT_GETSTATE, &vts) == 0)
			last_vt = vts.v_active;

		if (ioctl(con_fd, VT_ACTIVATE, nr) < 0) {
			perror("con_fd ioctl fail");
			close(con_fd);
			return 0;
		}

#ifndef TSLIB_NO_VT_WAITACTIVE
		if (ioctl(con_fd, VT_WAITACTIVE, nr) < 0) {
			perror("VT_WAITACTIVE");
			close(con_fd);
			return 0;
		}
#endif

		if (ioctl(con_fd, KDSETMODE, KD_GRAPHICS) < 0) {
			perror("KDSETMODE");
			close(con_fd);
			return 0;
		}

	}

	fb_fd = open(fbdevice, O_RDWR);
	if (fb_fd == -1) {
		perror("open fbdevice");
		return 0;
	}

	if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &fix) < 0) {
		perror("ioctl FBIOGET_FSCREENINFO");
		close(fb_fd);
		return 0;
	}

	if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &var) < 0) {
		perror("ioctl FBIOGET_VSCREENINFO");
		close(fb_fd);
		return 0;
	}

	xres_orig = var.xres;
	yres_orig = var.yres;
	printf("default xres=%d yres=%d\n",xres_orig,yres_orig);
	//rotation=3;
	if (rotation & 1) {
		/* 1 or 3 */
		y = var.yres;
		yres = var.xres;
		xres = y;
	} else {
		/* 0 or 2 */
		xres = var.xres;
		yres = var.yres;
	}

	fbuffer = mmap(NULL,
		       fix.smem_len,
		       PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED,
		       fb_fd,
		       0);
	if (fbuffer == (unsigned char *)-1) {
		perror("mmap framebuffer");
		close(fb_fd);
		return 0;
	}
	memset(fbuffer, 0, fix.smem_len);
	
	bytes_per_pixel = (var.bits_per_pixel + 7) / 8;
	transp_mask = ((1 << var.transp.length) - 1) <<
		var.transp.offset; /* transp.length unlikely > 32 */
	line_addr = malloc(sizeof(*line_addr) * var.yres_virtual);
	addr = 0;
	for (y = 0; y < var.yres_virtual; y++, addr += fix.line_length)
		line_addr[y] = fbuffer + addr;
	
	//printf("xres=%d yres=%d\n",xres,yres);
	return 1;
}

void close_framebuffer(void)
{
	memset(fbuffer, 0, fix.smem_len);
	munmap(fbuffer, fix.smem_len);
	close(fb_fd);

	if (strcmp(consoledevice, "none") != 0) {
		if (ioctl(con_fd, KDSETMODE, KD_TEXT) < 0)
			perror("KDSETMODE");

		if (last_vt >= 0)
			if (ioctl(con_fd, VT_ACTIVATE, last_vt))
				perror("VT_ACTIVATE");

		close(con_fd);
	}

	free(line_addr);

	xres = 0;
	yres = 0;
	rotation = 0;
}

void put_cross(int32_t x, int32_t y, uint32_t colidx)
{
	line(x - 10, y, x - 2, y, colidx);
	line(x + 2, y, x + 10, y, colidx);
	line(x, y - 10, x, y - 2, colidx);
	line(x, y + 2, x, y + 10, colidx);

	if (!alternative_cross) {
		line(x - 6, y - 9, x - 9, y - 9, colidx + 1);
		line(x - 9, y - 8, x - 9, y - 6, colidx + 1);
		line(x - 9, y + 6, x - 9, y + 9, colidx + 1);
		line(x - 8, y + 9, x - 6, y + 9, colidx + 1);
		line(x + 6, y + 9, x + 9, y + 9, colidx + 1);
		line(x + 9, y + 8, x + 9, y + 6, colidx + 1);
		line(x + 9, y - 6, x + 9, y - 9, colidx + 1);
		line(x + 8, y - 9, x + 6, y - 9, colidx + 1);
	} else if (alternative_cross == 1) {
		line(x - 7, y - 7, x - 4, y - 4, colidx + 1);
		line(x - 7, y + 7, x - 4, y + 4, colidx + 1);
		line(x + 4, y - 4, x + 7, y - 7, colidx + 1);
		line(x + 4, y + 4, x + 7, y + 7, colidx + 1);
	}
}
UCHAR put_graphic(UINT x,UINT y,UINT graphWidth,UINT graphHeight,UCHAR *pData){
	UCHAR colidx[3];
	static int color_code;
	if(x>xres||y>yres){
		return apiFailed;
	}
	colidx[0]=*(pData+2);
	colidx[1]=*(pData+1);
	colidx[2]=*pData;
	memmove(&color_code,&colidx[0],sizeof(UCHAR)*3);
	setcolor(0, color_code);
	for(ULONG i=0;i<graphHeight;i++){
		for(ULONG j=0;j<graphWidth;j++){
			colidx[0]=*(pData+2);
			colidx[1]=*(pData+1);
			colidx[2]=*pData;
			memmove(&color_code,&colidx[0],sizeof(UCHAR)*3);
			setcolor(0, color_code);
			pixel(x + j, y + i, 0);
			
			pData+=3;
		}
	}
	return apiOK;
}

UCHAR put_graphic_BMP555(UINT x,UINT y,UINT graphWidth,UINT graphHeight,UCHAR *pData){
UCHAR colidx[3];
int color_code;
UINT16 data1,data2;

	for(ULONG i=0;i<graphWidth;i++){	
		
		for(ULONG j=0;j<graphHeight;j++){
			data1=*pData++;
			data2=*pData++;
			colidx[0]=(((data2)&0x7C)<<1)|(((data2)&0x70)>>4);
			colidx[1]=(((data1&0xE0)>>2)|((data2&0x03)<<6))|(data2&0x03)|((data1&0x80)>>7);
			colidx[2]=(((data1)&0x1F)<<3)|(((data1)&0x1C)>>2);
			memmove(&color_code,&colidx[0],sizeof(UCHAR)*3);
			
			setcolor(0, color_code);
			pixel(x + j, y + i, 0);
		}
	}
	
	return apiOK;
}
static void put_char(ULONG x, ULONG y, ULONG c,API_LCDTFT_PARA para,UCHAR fontLen)
//static void put_char(int32_t x, int32_t y, int32_t c, int32_t colidx)
{
ULONG cnt=0;
UCHAR *data, bits;
UCHAR step;
//set attribute
UCHAR fid=para.Font;
UCHAR fontHeight=para.FontHeight;
UCHAR fontWidth=para.FontWidth;
UCHAR* CDaddr;//custom code addr
UCHAR* CDaddr_end;//custom code addr
UCHAR* BPaddr;//bitmap code addr
UCHAR* BPaddr_end;//bitmap code addr
OS_FDT	CustomFDT;
//set color
int FGcolidx,BGcolidx;
int i=0, j=0,k=0;
	UCHAR BGpalette[3]={para.BG_Palette[2],para.BG_Palette[1],para.BG_Palette[0]};
	UCHAR FGpalette[3]={para.FG_Palette[2],para.FG_Palette[1],para.FG_Palette[0]};
	memmove(&BGcolidx,&BGpalette,3);
	memmove(&FGcolidx,&FGpalette,3);	
	setcolor(0, BGcolidx);
	setcolor(1, FGcolidx);
	BGcolidx=0;
	FGcolidx=1;
	// printf("@@fid=%d\n",fid);
	
	if((fid<=FONT_1)||(fid==FONT12))
	{
		if(fid==FONT1){
		// #ifdef _build_DSS_
			data=&ext_font816[0];
		// #else
			// data=&ASCII_FONT_8X16[0];
		// #endif
			fontLen=16;
			fontWidth=8;
			fontHeight=16;
		}
		else if(fid==FONT0){
			data=&ASCII_FONT_6X8[0];
			fontLen=8;
			fontWidth=8;
			fontHeight=8;
		}
		else if(fid==FONT12){
			data=&ASCII_FONT_12X24[0];
			fontLen=48;
			fontWidth=16;
			fontHeight=24;
			c-=0x20;
		}
		step=fontLen/fontHeight;
		// if(rotation==0||rotation==2)

			for (i = 0; i < fontLen;i++) 
			{
				bits = *(data+fontLen * c + i);
				for (j = 0; j < fontWidth; j++, bits <<= 1)
				{
					if(j==8)
					{
						i++;
						bits = *(data+fontLen * c + i);
					}
					if (bits & 0x80)
						pixel(x + j, y + (i/step), FGcolidx);
					else
						pixel(x + j, y + (i/step), BGcolidx);
				}
			}
		// else
		// 	for (i = 0; i < fontLen;) 
		// 	{		
		// 		bits = *(data+fontLen * c + i);
		// 		for (j = 0; j < fontWidth; j++, bits <<= 1)
		// 		{
		// 			if (bits & 0x80)
		// 				pixel(x + j, y + (i/step), FGcolidx);
		// 			else
		// 				pixel(x + j, y + (i/step), BGcolidx);
		// 			if((j+1)%8==0)
		// 			{
		// 				i++;
		// 				bits = *(data+fontLen * c + i);
		// 			}
						
		// 		}
		// 	}
	}
	else
	{
		// printf("fid=%d ",fid);
		fontHeight	=FDT[fid].Height;
		fontWidth	=FDT[fid].Width;
		fontLen		=FDT[fid].ByteNo;
		CDaddr		=FDT[fid].CodStAddr;
		CDaddr_end	=FDT[fid].CodEndAddr;
		data		=FDT[fid].BmpStAddr;
		// BPaddr		=FDT[fid].BmpStAddr;
		// BPaddr_end	=FDT[fid].BmpEndAddr;
		//count input data order
		for(;CDaddr!=CDaddr_end;CDaddr++,cnt++)
			if(c==*CDaddr)
				break;
		// printf("cnt=%d\n",cnt);
		step=fontLen/fontHeight;
		// if(rotation==0||rotation==2)
			for (i = 0; i < fontLen;) 
			{		
				bits = *(data+fontLen * cnt + i);
				for (j = 0; j < fontWidth; j++, bits <<= 1)
				{
					if (bits & 0x80)
						pixel(x + j, y + (i/step), FGcolidx);
					else
						pixel(x + j, y + (i/step), BGcolidx);
					if((j+1)%8==0)
					{
						i++;
						bits = *(data+fontLen * cnt + i);
					}
				}
			}
		// else
		// 	for (i = 0; i < fontLen;) 
		// 	{		
		// 		bits = *(data+fontLen * cnt + i);
		// 		for (j = 0; j < fontWidth; j++, bits <<= 1)
		// 		{
		// 			if (bits & 0x80)
		// 				pixel(x + j, y + (i/step), FGcolidx);
		// 			else
		// 				pixel(x + j, y + (i/step), BGcolidx);
		// 			if((j+1)%8==0)
		// 			{
		// 				i++;
		// 				bits = *(data+fontLen * cnt + i);
		// 			}
		// 		}
		// 	}
	}
}
UCHAR find_Bg5FontPos(UCHAR *Bg5cd,ULONG *bg5pos){
FILE *fp;
int ret;
fpos_t pos;
UCHAR buff[2];
UCHAR Bgbuff[2];
	Bgbuff[0]=*Bg5cd;
	Bgbuff[1]=*(Bg5cd+1);
		fp=fopen("/usr/bin/BIG5.BIN", "rb");
	if (!fp){
	return 0;
	
	}
 
	do{
	ret=fread(&buff, sizeof(UCHAR), 2, fp);
	if(ret<2)//if can't find match code
	{
		break;
	}
	}while(buff[0]!=Bgbuff[0] || buff[1]!=Bgbuff[1]);
	*bg5pos=ftell(fp);
	*bg5pos=((*bg5pos)-1)/2;//count which byte of big5 word in BIG5.BIN	
	fclose(fp);
	return 1;
	
}
UCHAR Bg5byte(ULONG bg5pos,UCHAR *fontbyte){//Input: which byte of big5 word  
FILE *fp;
UCHAR fontLen=fontbyte[0];
	if(fontLen>32)
		fp=fopen("/usr/bin/FONT2_N_24x24.BIN", "rb");
	else
		fp=fopen("/usr/bin/FONT4.BIN", "rb");
	if (!fp)
    return 0;
	
	fseek(fp,bg5pos,SEEK_CUR);
	fread(fontbyte, sizeof(UCHAR), fontLen, fp);
	fclose(fp);
	return 1;
	
}
 static void put_Big5char(int32_t x, int32_t y, UCHAR *Bg5cd, API_LCDTFT_PARA para,ULONG fontLen)
{	
UCHAR	BIG5CODE[2];
UCHAR	fid;
UCHAR	FONT_BYTE;
UCHAR*	bit;
UCHAR*	bit2;
ULONG	bg5pos;
OS_FDT	*pFdt;	
	fid = para.Font & 0x0F;
	
	if((*Bg5cd<0x7F)&&(*Bg5cd>0x19))
	{
		/*
		BIG5CODE[0]=Alphanumeric2BIG5[*Bg5cd][0];
		BIG5CODE[1]=Alphanumeric2BIG5[*(Bg5cd++)][1];
		*/
		if((fid==FONT2)||(fid==FONT12))//24*24
			fid=FONT12;//12*24
		else//16*16
			fid=FONT1;//8*16
		pFdt = OS_FONT_GetFdtAddr( fid );
		
		FONT_BYTE=pFdt->ByteNo;
		para.Font=fid;
		put_char( x, y, *Bg5cd, para, FONT_BYTE);
		return;
	}
	else
	{
		BIG5CODE[0]=*(Bg5cd++);
		BIG5CODE[1]=*(Bg5cd++);
	}
	// printf("fid=%d  *Bg5cd=0x%x\n",fid,*Bg5cd);	
	pFdt = OS_FONT_GetFdtAddr( fid );
	FONT_BYTE=pFdt->ByteNo;
	bit=malloc(FONT_BYTE);
	// printf("\n");
	if(find_Bg5FontPos(BIG5CODE,&bg5pos)!=1){ 
		return ;
	}
	bg5pos=(ULONG)(bg5pos*FONT_BYTE);
	bit[0]=FONT_BYTE;
	if(Bg5byte(bg5pos,bit)!=1){
		return ;
	}
	bit2=bit;	
	int32_t i, j,k=0;
	//set color
	static int FGcolidx,BGcolidx;
	UCHAR BGpalette[3]={para.BG_Palette[2],para.BG_Palette[1],para.BG_Palette[0]};
	UCHAR FGpalette[3]={para.FG_Palette[2],para.FG_Palette[1],para.FG_Palette[0]};
	memmove(&BGcolidx,&BGpalette,sizeof(BGpalette));
	memmove(&FGcolidx,&FGpalette,sizeof(FGpalette));
	setcolor(0, BGcolidx);
	setcolor(1, FGcolidx);
	BGcolidx=0;
	FGcolidx=1;
	// printf("para.FontHeight=%d  para.FontWidth=%d\n",para.FontHeight,para.FontWidth);
	for (i = 0; i < para.FontHeight; i++) {//Big5 chinese font
		for (j = 0; j < para.FontWidth; j++,k++, *bit <<= 1){
			if(k==8){bit++;k=0;}
			if (*bit & 0x80){
				pixel(x + j, y + i, FGcolidx);
			}	
			else{
				pixel(x + j, y + i, BGcolidx);
			}
			
		}
	}
	
	bit=bit2;
	free(bit);
}

void put_Bg5string(int32_t x, int32_t y, UCHAR *s, API_LCDTFT_PARA para,ULONG fontLen)
{
	int32_t i;
	// printf("fontLen=%d\n",fontLen);
	for(i = 0; i<fontLen; i++ ){
		if(*s<0x20)
			return;
		put_Big5char(x, y, s, para,fontLen);
		if((*s<0x7F)&&(*s>0x19))
		{
			
			x += (para.FontWidth/2);
			s++;
		}							
		else
		{
			x += para.FontWidth;
			s+=2;
			i++;
		}
			
	}
}

void put_string(int32_t x, int32_t y, char *s, API_LCDTFT_PARA para, ULONG Len)
{
int32_t i;	
UINT16 fontLen=0;
UCHAR fid=para.Font;
int FGcolidx,BGcolidx;
	UCHAR BGpalette[3]={para.BG_Palette[2],para.BG_Palette[1],para.BG_Palette[0]};
	UCHAR FGpalette[3]={para.FG_Palette[2],para.FG_Palette[1],para.FG_Palette[0]};
	memmove(&BGcolidx,&BGpalette,3);
	memmove(&FGcolidx,&FGpalette,3);	
	setcolor(0, BGcolidx);
	setcolor(1, FGcolidx);
	// printf("@@fid=%d\n",fid);
	if(rotation==ROTATE_270||rotation==ROTATE_90)
	{
		x=para.FontWidth*(Len-1);
		for (i = 0; i<Len; i++, x -= para.FontWidth, s++)
			put_char( x, y, *s, para, fontLen);
	}
	else
		// for (i = 0; *s; i++, x += para.FontWidth, s++)
		for (i = 0; i<Len; i++, x += para.FontWidth, s++)
		{
			put_char( x, y, *s, para, fontLen);
		}
		//put_char (x, y, *s, colidx);
}

void put_string_center(int32_t x, int32_t y, char *s, uint32_t colidx)
{
	size_t sl = strlen(s);

	/*put_string(x - (sl / 2) * font_vga_8x8.width,
		   y - font_vga_8x8.height / 2, s, colidx);*/
}

void setcolor(uint32_t colidx, uint32_t value)
{
	uint32_t res;
	uint16_t red, green, blue;
	struct fb_cmap cmap;

	if (colidx > 255) {
		return;
	}

	switch (bytes_per_pixel) {
	default:
	case 1:
		res = colidx;
		red = (value >> 8) & 0xff00;
		green = value & 0xff00;
		blue = (value << 8) & 0xff00;
		cmap.start = colidx;
		cmap.len = 1;
		cmap.red = &red;
		cmap.green = &green;
		cmap.blue = &blue;
		cmap.transp = NULL;

		if (ioctl(fb_fd, FBIOPUTCMAP, &cmap) < 0)
			perror("ioctl FBIOPUTCMAP");
		break;
	case 2:
	case 3:
	case 4:
		red = (value >> 16) & 0xff;
		green = (value >> 8) & 0xff;
		blue = value & 0xff;
		res = ((red >> (8 - var.red.length)) << var.red.offset) |
		      ((green >> (8 - var.green.length)) << var.green.offset) |
		      ((blue >> (8 - var.blue.length)) << var.blue.offset);
	}
	colormap[colidx] = res;
}

static void __pixel_loc(int32_t x, int32_t y, union multiptr *loc)
{
	switch (rotation) {
	case 0:
	default:
		if(y>(yres-1)||x>(xres-1)){break;}//if exceed LCD boundary,do nothing.
		loc->p8 = line_addr[y] + x * bytes_per_pixel;		
		break;
	case 1:
		if(x>(yres-1)||y>(xres-1)){break;}//if exceed LCD boundary,do nothing.
		loc->p8 = line_addr[x] + (xres - y - 1) * bytes_per_pixel;
		break;
	case 2:
		if(x>(xres-1)||y>(yres-1)){break;}//if exceed LCD boundary,do nothing.
		loc->p8 = line_addr[yres - y - 1] + (xres - x - 1) * bytes_per_pixel;
		break;
	case 3:		
	//printf("x=%d\ty=%d\n",x,y);
		if(x>(yres-1)||y>(xres-1)){break;}//if exceed LCD boundary,do nothing.
		loc->p8 = line_addr[yres - x - 1] + y * bytes_per_pixel;
		break;
	}
}

static inline void __setpixel(union multiptr loc, uint32_t xormode, uint32_t color)
{
	switch (bytes_per_pixel) {
	case 1:
	default:
		if (xormode)
			*loc.p8 ^= color;
		else
			*loc.p8 = color;
		*loc.p8 |= transp_mask;
		break;
	case 2:
		if (xormode)
			*loc.p16 ^= color;
		else
			*loc.p16 = color;
		*loc.p16 |= transp_mask;
		break;
	case 3:
		if (xormode) {
			*loc.p8++ ^= (color >> 16) & 0xff;
			*loc.p8++ ^= (color >> 8) & 0xff;
			*loc.p8 ^= color & 0xff;
		} else {
			*loc.p8++ = (color >> 16) & 0xff;
			*loc.p8++ = (color >> 8) & 0xff;
			*loc.p8 = color & 0xff;
		}
		*loc.p8 |= transp_mask;
		break;
	case 4:
		if (xormode)
			*loc.p32 ^= color;
		else
			*loc.p32 = color;
		*loc.p32 |= transp_mask;
		break;
	}
}

void pixel(int32_t x, int32_t y, uint32_t colidx)
{
	uint32_t xormode;
	union multiptr loc;

	/*if ((x < 0) || ((uint32_t)x >= xres) ||
	    (y < 0) || ((uint32_t)y >= yres))
		return;*/

	xormode = colidx & XORMODE;
	colidx &= ~XORMODE;

	if (colidx > 255) {
		return;
	}
	if(rotation==ROTATE_0||rotation==ROTATE_180){
		if(x>(yres-1)||y>(xres-1)){//if exceed LCD boundary,do nothing.
			return;
		}
	}
	if(rotation==ROTATE_270||rotation==ROTATE_90)
		if(x>(xres-1)||y>(yres-1)){//if exceed LCD boundary,do nothing.
			// printf("x=%d xres=%d\ty=%d yres=%d\n",x,xres,y,yres);
			return;
			}
		
	
	__pixel_loc(x, y, &loc);
	__setpixel(loc, xormode, colormap[colidx]);
}

void line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t colidx)
{
	int32_t tmp;
	int32_t dx = x2 - x1;
	int32_t dy = y2 - y1;

	if (abs(dx) < abs(dy)) {
		if (y1 > y2) {
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		x1 <<= 16;
		/* dy is apriori >0 */
		dx = (dx << 16) / dy;
		while (y1 <= y2) {
			pixel(x1 >> 16, y1, colidx);
			x1 += dx;
			y1++;
		}
	} else {
		if (x1 > x2) {
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		y1 <<= 16;
		dy = dx ? (dy << 16) / dx : 0;
		while (x1 <= x2) {
			pixel(x1, y1 >> 16, colidx);
			y1 += dy;
			x1++;
		}
	}
}

void rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t colidx)
{
	line(x1, y1, x2, y1, colidx);
	line(x2, y1+1, x2, y2-1, colidx);
	line(x2, y2, x1, y2, colidx);
	line(x1, y2-1, x1, y1+1, colidx);
}

void fillrect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t colidx)
{
	int32_t tmp;
	uint32_t xormode;
	union multiptr loc;
	/* Clipping and sanity checking */
	if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
	if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }
	 

	if ((x1 > x2) || (y1 > y2))
		return;
	if((rotation==ROTATE_270)||(rotation==ROTATE_90))
	{
		if ((x1>xres)||(x2>xres))
		{
			printf("API_LCDTFT:fillrect(): input arguments error: x1=%d  x2=%d xres=%d\n",x1,x2,xres);
			// return;
		}
		if ((y1>yres)||(y2>yres))
		{
			printf("API_LCDTFT:fillrect(): input arguments error: y1=%d  y2=%d yres=%d\n",y1,y2,yres);
			// return;
		}
	}
	if((rotation==ROTATE_0)||(rotation==ROTATE_180))
	{
		if ((x1>yres)||(x2>yres))
		{
			printf("API_LCDTFT:fillrect(): input arguments error: x1=%d  x2=%d yres=%d\n",x1,x2,yres);
			// return;
		}
		if ((y1>xres)||(y2>xres))
		{
			printf("API_LCDTFT:fillrect(): input arguments error: y1=%d  y2=%d xres=%d\n",y1,y2,xres);
			// return;
		}
	}
	xormode = colidx & XORMODE;
	colidx &= ~XORMODE;

	if (colidx > 255) {
		return;
	}
	//printf("transp_mask=%x",transp_mask);
	colidx = colormap[colidx];
	for (; y1 <= y2; y1++) {
		for (tmp = x1; tmp <= x2; tmp++) {
			__pixel_loc(tmp, y1, &loc);
			__setpixel(loc, xormode, colidx);
			loc.p8 += bytes_per_pixel;
		}
	}
}
//to get specific area pixel 
//INPUT:XY start and end position
//OUTPUT:pData
//				to save the location pixel value.each values are 1 bit long.

UCHAR getpixel(union multiptr loc)
{
	UINT32 color;
	
	color=*loc.p32;
	//printf("color=%lx\n",color);
	if(color>0x888888)
		return 1;//white
	else
		return 0;//black
}

void getpixelvalue(int32_t x1, int32_t y1, int32_t x2, int32_t y2, UCHAR* pData)
{
int32_t tmp;
uint32_t j=0;
union multiptr loc;
UCHAR data=0,i=0;
UCHAR* pData_bak;
FILE *fp2;
	pData_bak=pData;
	/* Clipping and sanity checking */
	if (x1 > x2) { tmp = x1; x1 = x2; x2 = tmp; }
	if (y1 > y2) { tmp = y1; y1 = y2; y2 = tmp; }


	if ((x1 > x2) || (y1 > y2))
		return;

	
	for (; y1 < y2; y1++) {
		//printf("y1=%d\n",y1);
		for (tmp = x1; tmp < x2; tmp++,i++,j++) {
			__pixel_loc(tmp, y1, &loc);	
			if(i>7)
			{
				*pData=data;
				//printf("data=%x",data);
				pData++;
				i=0;
				data=0;
			}
			else
				data=data<<1;
			
			data=data|getpixel(loc);			
			loc.p8 += bytes_per_pixel;
		}
	}
	pData=pData_bak;
	// fp2=fopen( "/usr/bin/signbin", "wb" );
	// fwrite(pData,sizeof(signbmp),1,fp2);
}