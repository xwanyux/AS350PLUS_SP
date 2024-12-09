/*
�W�١Gfont_type.c
�@�̡GCharles Tsai
�γ~�G
�\��G
��x�G
	2008/Sep/24	�ɮ׫إ�
*/
//====================================include=======================================================
//c std library
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
//ZiLog BSP library

//myself library
	#include "POSAPI.h"
	#include "massage.h"
	#include "option.h"

//program define header
	#include "charles_base.h"
	#include "font_type.h"


//	#include "big5_24_map.h"

//	#include "printer.h"
//====================================globe variable================================================
extern struct ERROR_INFO ERRNO;
extern UINT8  ASCII_FONT_8X16[4096];
extern UINT8  InitDoneFlag[MAX_FONT];
extern unsigned char ASCII_FONT_12X24[];
//struct FONT font_table[max_font];
struct FONT_PAIR FONT_TABLE[MAX_FONT];
UCHAR BIG5_BITMAPCODE[72];
inline void font_init()
{
	int i;

	for(i=0;i<MAX_FONT;i++)
	{
		if(InitDoneFlag[i]!=1)//init the font if the font have not inited.
		{
			memset(&FONT_TABLE[i],0,sizeof(struct FONT));
			// FONT_TABLE[i].idel = SEM_B_SIGNAL;
			FONT_TABLE[i].SBC.type = USELESS_FONT;
			FONT_TABLE[i].MBC.type = USELESS_FONT;
			FONT_TABLE[i].height = 0;
			InitDoneFlag[i]=1;
		}

	}
	// FONT_TABLE[0].height = 16;

	// FONT_TABLE[0].SBC.type = ASCII;
	// FONT_TABLE[0].SBC.width = 8;
	// FONT_TABLE[0].SBC.member_size = (1*16);
	// FONT_TABLE[0].SBC.bitmap = ASCII_FONT_8X16;
	// FONT_TABLE[0].SBC.code_size = 1;
	// FONT_TABLE[0].SBC.code_list = 0;
	// FONT_TABLE[0].SBC.member = 256;
	FONT_TABLE[0].height = 24;

	FONT_TABLE[0].SBC.type = ASCII;
	FONT_TABLE[0].SBC.width = 12;
	FONT_TABLE[0].SBC.member_size = (2*24);
	FONT_TABLE[0].SBC.bitmap = ASCII_FONT_12X24;
	FONT_TABLE[0].SBC.code_size = 1;
	FONT_TABLE[0].SBC.code_list = 0;
	FONT_TABLE[0].SBC.member = 95;
	
	// FONT_TABLE[1].height=24;
	// FONT_TABLE[1].MBC.type = BIG5;
	// FONT_TABLE[1].MBC.width=24;
	// FONT_TABLE[1].MBC.member_size = (3*24);
	// //FONT_TABLE[1].MBC.bitmap = big5_24x24;
	// FONT_TABLE[1].MBC.bitmap = ASCII_FONT_12X24;//point to valid address to meet "if" condition
	// FONT_TABLE[1].MBC.code_size = 2;
	// //FONT_TABLE[1].MBC.code_list = (unsigned char*)BIG5_24_code_list;
	// FONT_TABLE[1].MBC.code_list = ASCII_FONT_12X24;
	// FONT_TABLE[1].MBC.member = 83;


	// FONT_TABLE[1].SBC.type = ASCII;
	// FONT_TABLE[1].SBC.width = 12;
	// FONT_TABLE[1].SBC.member_size = (2*24);//width%8!=0;
	// FONT_TABLE[1].SBC.bitmap = ASCII_FONT_12X24;
	// FONT_TABLE[1].SBC.code_size = 1;
	// FONT_TABLE[1].SBC.code_list = 0;
	// FONT_TABLE[1].SBC.member = 95;

	FONT_TABLE[2].height=24;
	FONT_TABLE[2].MBC.type = BIG5;
	FONT_TABLE[2].MBC.width=24;
	FONT_TABLE[2].MBC.member_size = (3*24);
	//FONT_TABLE[1].MBC.bitmap = big5_24x24;
	FONT_TABLE[2].MBC.bitmap = ASCII_FONT_12X24;//point to valid address to meet "if" condition
	FONT_TABLE[2].MBC.code_size = 2;
	//FONT_TABLE[1].MBC.code_list = (unsigned char*)BIG5_24_code_list;
	FONT_TABLE[2].MBC.code_list = ASCII_FONT_12X24;
	FONT_TABLE[2].MBC.member = 83;


	FONT_TABLE[2].SBC.type = ASCII;
	FONT_TABLE[2].SBC.width = 12;
	FONT_TABLE[2].SBC.member_size = (2*24);//width%8!=0;
	FONT_TABLE[2].SBC.bitmap = ASCII_FONT_12X24;
	FONT_TABLE[2].SBC.code_size = 1;
	FONT_TABLE[2].SBC.code_list = 0;
	FONT_TABLE[2].SBC.member = 95;

	// FONT_TABLE[3].height=24;
	// FONT_TABLE[3].MBC.type = BIG5;
	// FONT_TABLE[3].MBC.width=24;
	// FONT_TABLE[3].MBC.member_size = (3*24);
	// FONT_TABLE[3].MBC.bitmap = ASCII_FONT_12X24;//point to valid address to meet "if" condition
	// FONT_TABLE[3].MBC.code_size = 2;
	// FONT_TABLE[3].MBC.code_list = ASCII_FONT_12X24;
	// FONT_TABLE[3].MBC.member = 83;


	// FONT_TABLE[3].SBC.type = ASCII;
	// FONT_TABLE[3].SBC.width = 12;
	// FONT_TABLE[3].SBC.member_size = (2*24);//width%8!=0;
	// FONT_TABLE[3].SBC.bitmap = ASCII_FONT_12X24;
	// FONT_TABLE[3].SBC.code_size = 1;
	// FONT_TABLE[3].SBC.code_list = 0;
	// FONT_TABLE[3].SBC.member = 95;
	//============
	FONT_TABLE[4].height=16;
	FONT_TABLE[4].MBC.type = BIG5;
	FONT_TABLE[4].MBC.width=16;
	FONT_TABLE[4].MBC.member_size = (2*16);
	FONT_TABLE[4].MBC.bitmap = ASCII_FONT_8X16;//point to valid address to meet "if" condition
	FONT_TABLE[4].MBC.code_size = 2;
	FONT_TABLE[4].MBC.code_list = ASCII_FONT_8X16;
	FONT_TABLE[4].MBC.member = 83;


	FONT_TABLE[4].SBC.type = ASCII;
	FONT_TABLE[4].SBC.width = 8;
	FONT_TABLE[4].SBC.member_size = (1*16);//width%8!=0;
	FONT_TABLE[4].SBC.bitmap = ASCII_FONT_8X16;
	FONT_TABLE[4].SBC.code_size = 1;
	FONT_TABLE[4].SBC.code_list = 0;
	FONT_TABLE[4].SBC.member = 95;
//============
	FONT_TABLE[5].height=24;
	FONT_TABLE[5].MBC.type = BIG5;
	FONT_TABLE[5].MBC.width=24;
	FONT_TABLE[5].MBC.member_size = (3*24);
	FONT_TABLE[5].MBC.bitmap = ASCII_FONT_12X24;//point to valid address to meet "if" condition
	FONT_TABLE[5].MBC.code_size = 2;
	FONT_TABLE[5].MBC.code_list = ASCII_FONT_12X24;
	FONT_TABLE[5].MBC.member = 83;


	FONT_TABLE[5].SBC.type = ASCII;
	FONT_TABLE[5].SBC.width = 12;
	FONT_TABLE[5].SBC.member_size = (2*24);//width%8!=0;
	FONT_TABLE[5].SBC.bitmap = ASCII_FONT_12X24;
	FONT_TABLE[5].SBC.code_size = 1;
	FONT_TABLE[5].SBC.code_list = 0;
	FONT_TABLE[5].SBC.member = 95;
/*
	FONT_TABLE[3].type = BIG5;
	FONT_TABLE[3].height=24;
	FONT_TABLE[3].width=24;
	FONT_TABLE[3].member_size = (3*24);
	FONT_TABLE[3].bitmap = new_big5_24;*/
}

UCHAR*	big5_bitmap( UCHAR* Bg5cd ,UCHAR fontID)
{
FILE *fp;
int ret;
fpos_t pos;
UCHAR buff[2];
UCHAR Bgbuff[2];
ULONG bg5pos;
ULONG fontLen=FONT_TABLE[fontID].MBC.member_size;
//find big5 code position
	Bgbuff[0]=*Bg5cd;
	Bgbuff[1]=*(Bg5cd+1);
	fp=fopen("/usr/bin/BIG5.BIN", "rb");
	if (!fp)
	return 0;	
 
	do{
		ret=fread(&buff, sizeof(UCHAR), 2, fp);
		if(ret<2)//if can't find match code		
			break;
	}while(buff[0]!=Bgbuff[0] || buff[1]!=Bgbuff[1]);
	bg5pos=ftell(fp);
	
	fclose(fp);
//get big5 font bitmap
// fp=fopen("/usr/bin/FONT2_N_24x24.BIN", "rb");
	if(fontID==FONT2)
		fp=fopen("/usr/bin/FONT2_N_24x24.BIN", "rb");
	else if(fontID==FONT4)
		fp=fopen("/usr/bin/FONT4.BIN", "rb");
	else
	{
		//MBC code list are at default address.Means user not define MBC code list
		if(FONT_TABLE[fontID].MBC.code_list==ASCII_FONT_12X24)
		{
			fp=fopen("/usr/bin/FONT2_N_24x24.BIN", "rb");
			fontLen=72;
		}
		else
			return  FONT_TABLE[fontID].MBC.code_list[bg5pos];//return user defined bitmapcode
	}
	if (!fp)
    return 0;
	bg5pos=(bg5pos-1)/2*fontLen;//count which byte of big5 word in BIG5.BIN	
	fseek(fp,bg5pos,SEEK_CUR);
	fread(BIG5_BITMAPCODE, sizeof(UCHAR), fontLen, fp);
	fclose(fp);
	return BIG5_BITMAPCODE;
}
inline const unsigned char* Font_getBitMap(unsigned short code,unsigned char fontID)//1~2byte
{
	unsigned long int font_size;
	unsigned char *temp;
	unsigned long int index;
	unsigned long int i;
	// printf("fontID=%d\n",fontID);
	// printf("code=%d\n",(code&0xff));
	
	if((code&0xff)<0x80)
	{
SBC_BMP:
		if( FONT_TABLE[fontID].SBC.code_list == 0 )	// 2012-04-16, using default rule if code_list is NULL.
		  {
		  if(code>=0x20&&code<=0x7e)
		  	{
				if(&FONT_TABLE[fontID].SBC.bitmap[0] == &ASCII_FONT_12X24[0])
					code-=0x20;//20201030comment out for different code list
		  		return &(FONT_TABLE[fontID].SBC.bitmap[code*FONT_TABLE[fontID].SBC.member_size]);
		  	}
		  else
		  	{
		  		return &(FONT_TABLE[fontID].SBC.bitmap[0]);//�Ǧ^�ťժ�bitmap
		  	}
		  }
		else	// 2012-04-16, search code from list
		  {
		  for( i=0; i<256; i++ )
		     {
		     if( code == FONT_TABLE[fontID].SBC.code_list[i] )
		       return &(FONT_TABLE[fontID].SBC.bitmap[i*FONT_TABLE[fontID].SBC.member_size]);
		     }
		     printf("code not found\n");
		  return (unsigned char *)0;	// return NULL ptr if code not found.
		  }
	}
	else
	{
		if( FONT_TABLE[fontID].MBC.code_list == 0 )	// 2012-04-30, print SBC if MBC.code_list = 0
		  goto SBC_BMP;
		
		// PATCH: 2010-02-19
		// temp = ((unsigned char*)bsearch(
				// (UINT8 *)&code, (UINT8 *)FONT_TABLE[fontID].MBC.code_list, (int)FONT_TABLE[fontID].MBC.member,
				// FONT_TABLE[fontID].MBC.code_size, big5_compare));
			return big5_bitmap( (UCHAR*)&code ,fontID);
	}

	//#endif
}
