/*
 * processString.c
 *
 *  Created on: 2009/4/29
 *      Author: Charles
 */
//====================================include=======================================================
//c std library
	#include <stdio.h>
	#include <stdlib.h>
	#include <stdarg.h>
	#include <string.h>
//ZiLog BSP library
//myself library
	#include "POSAPI.h"
	#include "printer.h"
	#include "font_type.h"
	#include "charles_base.h"
	#include "massage.h"
	 
static UCHAR *PRINT_BUF = 0;
UINT8	g_bitmap[1024];//used in GenerateDoubleHeightBitMap
UINT8	srcbmp[1024];//used in ReverseBitmap
UINT8	InitDoneFlag[MAX_FONT];//to determine if FONTx have inited
UINT32	g_HeatCount = 0;
UINT32	g_MotorTimer1 = 0;
UINT32	g_MotorTimer2 = 0;
extern struct FONT_PAIR FONT_TABLE[MAX_FONT];
extern	unsigned long point;
struct ERROR_INFO ERRNO;
extern void motor_stop();
BSP_BOOL StringProcOpen()
{
	PRINT_BUF = malloc(512);
	//20210730comment out for prevent use pointer as condition
	if(PRINT_BUF==NULL)
	{
		PRN_setStatus(PRINT_STATUS_BUFFERFULL);
		return FALSE;
	}
		
	memset(PRINT_BUF,NULL,512);
	return TRUE;
}

void StringProcClose()
{
	printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
	motor_stop();//20210915 add by West, 
				 //make sure motor stop when exception occure to prevent from AP coredump when motor running(or motor will burn in few second).
	free(PRINT_BUF);
	PRINT_BUF = 0;
}
//static inline 
UINT32 PRN_CountBits( UINT32 n )
{
	UINT32					m, t;

	m = 0x49249249;//001001...
	t = n & (m << 1);//n's bit 1,4,7,10,13,16...31
	n = n - (t >> 1);
	t = n & (m >> 1);
	n = n + t;

	m = 0xC71C71C7;//000111000111...
	n = n + (n >> 3);
	n = n & m;

	n = n + (n >> 6);
	n = n + (n >> 12);
	n = n + (n >> 24);
	n = n & 63;//0x3f

	return( n );
}
unsigned long PRN_getCountofHigh(SUBROWMAP data)
{
	int i;
	unsigned long sum = 0;
	unsigned long *temp = (unsigned long*)data;
	for(i=0;i<48/4;i++)
	{
		sum += PRN_CountBits(temp[i]);
	}
	return sum;
}
void	setupHeatMotorTime( struct ROWMAP *map )
{
UINT32	index;
UINT32	heatCount = PRN_getCountofHigh(map->data);
	
	if( heatCount )
	{

	  map->heat_time = setupTimer_uS( 100 );  
	  map->motor_time1 = point;
	  map->motor_time2 = point;
	  map->info[0]	    = heatCount;
	  map->info[1]	    = heatCount>>8;
	}
	else
	{

	  map->motor_time1 = point;
	  map->motor_time2 = point;
	  map->heat_time = 0;
	  map->info[0] = 0;
	  map->info[1] = 0;
	}
}
void copyBitMap( unsigned char attr, unsigned long start_bit,struct FONT *pFont,unsigned long font_height,const unsigned char *s,unsigned char d[PRN_MAX_ROW_HEIGHT][PRN_DOT_LINE_BYTES])
{
	if(s==0)
	{
		s = pFont->bitmap;
	}
	unsigned long end_bit = start_bit+pFont->width;
	unsigned char width_of_byte = pFont->width/8;
	unsigned long i,j;
	unsigned char mask = 0xff;
	unsigned long new_start_bit = start_bit%8;
	unsigned long fheight;
	unsigned long factor;

	fheight = font_height;
	factor = 1;
//	if( attr & 0x20 )
//	  {
//	  fheight *= 2;
//	  factor = 2;
//	  }

	if(new_start_bit!=0)
	{//if(start_bit % 8 != 0)
		for(j=0;j<fheight;j++)
		{
			if(pFont->width%8!=0)
			{
				for(i=0;i<(width_of_byte);i++)
				{
					d[factor*j][i+start_bit/8] |= (s[factor*j*((pFont->width/8)+1)+i]&0xff)>>(8-new_start_bit);
					d[factor*j][i+start_bit/8+1] = ((s[factor*j*((pFont->width/8)+1)+i]&0xff)<<(8-new_start_bit));
				}
				d[factor*j][i+start_bit/8] |= (s[factor*j*((pFont->width/8)+1)+i]&0xff)>>(8-new_start_bit);
			}
			else
			{
				for(i=0;i<(width_of_byte);i++)
				{
					d[factor*j][i+start_bit/8] |= (s[j*((pFont->width/8))+i]&0xff)>>(8-new_start_bit);
					d[factor*j][i+start_bit/8+1] = ((s[factor*j*((pFont->width/8))+i]&0xff)<<(8-new_start_bit));
				}
			}
		}
	}
	else
	{//if(start_bit % 8 == 0)
		if(pFont->width%8==0)
		for(i=0;i<width_of_byte;i++)
		{
			for(j=0;j<fheight;j++)
			{
				d[factor*j][i+start_bit/8]=s[factor*j*(pFont->width/8)+i];
			}
		}
		else
		for(i=0;i<width_of_byte;i++)
		{
			for(j=0;j<fheight;j++)
			{
				d[factor*j][i+start_bit/8]=s[factor*j*(pFont->width/8+1)+i];
			}
		}
		if(end_bit%8!=0)
		{
			for(j=0;j<fheight;j++)
			{
				d[factor*j][width_of_byte+start_bit/8] = s[factor*j*(pFont->width/8+1)+width_of_byte];
			}

		}

	}
	
}
UINT8	*ReverseBitmap( UINT32 flag, UINT32 bytes, UINT32 height, UINT32 width, UINT8 *bmp )
{
UINT8	data;
UINT32	i = 0;
UINT32	j = 0;
UINT32	cnt;
UINT32	remain;
UINT32	shift;
UINT32	hbytes;


	memset( srcbmp, 0x00, sizeof(srcbmp) );
	memmove( srcbmp, bmp, bytes );

#if	0	
	if( flag )
	  {
	  for( i=0; i<bytes; i++ )
	     srcbmp[i] = ~srcbmp[i];
	  }
#endif

#if	1
	if( flag )
	  {
	  cnt = width/8;	// horizontal integer part (bytes)
	  remain = width % 8;	// horizontal residue part (bits)
	  shift = 8 - remain;
	  
	  hbytes = cnt;		// horizontal bytes
	  if( remain )
	    hbytes++;
	
	  for( i=0; i<height; i++ )
	     {
	     // process integer part
	     if( cnt )
	       {
	       for( j=0; j<cnt; j++ )
	          srcbmp[i*hbytes+j] = ~srcbmp[i*hbytes+j];
	       }
	     
	     // process residue part
	     if( remain )
	       srcbmp[i*hbytes+j] = ((~srcbmp[i*hbytes+j]) >> shift) << shift;
	     }
	  }
#endif

	return( srcbmp );
}
UINT8	*GenerateDoubleHeightBitMap( UINT32 height, UINT32 width, UINT8 *src )
{
UINT32	i;
UINT32	j;
UINT32	k;
UINT32	h_cnt;


	h_cnt = width/8;
	if( width % 8 )
	  h_cnt++;
	
	k = 0;  
	for( i=0; i<height; i++ )
	   {
	   for( j=0; j<h_cnt; j++ )
	      {
	      g_bitmap[i*h_cnt+j+k*h_cnt] = *src;
	      g_bitmap[i*h_cnt+j+k*h_cnt+h_cnt] = *src;
	      src++;
	      }
	   k++;
	   }
	   
	return( g_bitmap );
}
struct ROWMAP* PRN_generateRowmap(unsigned char s[PRN_DOT_LINE_BYTES])
{
UINT32	index;
struct ROWMAP *head;
//20210730comment out for prevent use pointer as condition
	// do{
	  if( PRN_getStatus() == PRINT_STATUS_PAPEREMPTY )
	    return 0;
	    
	  head = malloc(sizeof(struct ROWMAP));	// PATCH: 2009-11-16
	  memset(head,NULL,sizeof(struct ROWMAP));
	// }while( head == NULL );
	
	if(head == NULL)
	{
		PRN_setStatus(PRINT_STATUS_BUFFERFULL);
		printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
		return 0;
	}
		
		
	if( PRN_getStatus() == PRINT_STATUS_PAPEREMPTY )
	{
		printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
		return 0;
	}
	    
	memcpy(head->data,s,PRN_DOT_LINE_BYTES);
	unsigned long heatCount = PRN_getCountofHigh(s);
	
//	if( heatCount )
	  g_HeatCount = heatCount;
	// printf("g_HeatCount =%ld\n",g_HeatCount);
	index = heatCount / 8;
	if( index )
	  index--;

	if( heatCount )
	  {
//	  head->heat_time = setupTimer_uS( PRT_Settings[index][0] );	  
//	  head->motor_time1 = PRT_Settings[index][1];
//	  head->motor_time2 = PRT_Settings[index][2];
//	  head->info[0]	    = PRT_Settings[index][3];
//	  head->info[2]	    = PRT_Settings[index][3];

	  head->heat_time = setupTimer_uS( 100 );  
	  head->motor_time1 = point;
	  head->motor_time2 = point;
	  // head->info[0]	    = 3*4;
	  // head->info[2]	    = 3*4;
	  head->info[0]	    = heatCount;
	  head->info[1]	    = heatCount>>8;
	  }
	else
	  {
//	  head->motor_time1 = PRT_Settings[index][1];
//	  head->motor_time2 = PRT_Settings[index][2];
//	  head->heat_time = 0;
//	  head->info[2] = 0;

	  head->motor_time1 = point;
	  head->motor_time2 = point;
	  head->heat_time = 0;
	  // head->info[2] = 0;
	  head->info[0] = 0;
	  head->info[1] = 0;
	  }
	  
//	g_MotorTimer1 = PRT_Settings[index][1];
//	g_MotorTimer2 = PRT_Settings[index][2];

	g_MotorTimer1 = point;
	g_MotorTimer2 = point;

	return head;
}
BSP_BOOL PRN_generateRowmaps(unsigned char s[PRN_MAX_ROW_HEIGHT][PRN_DOT_LINE_BYTES],struct FONT_PAIR *font,struct ROWMAP_PAIR *result, UINT32 factor)
{
	result->count = 0;//initial
	struct ROWMAP *temp,*head;
	int lines,index = 0;
	lines = font->height*factor + PRN_getILG();
	if(lines > PRN_MAX_ROW_HEIGHT)
	{
		printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
		return FALSE;
	}
		
	temp = PRN_generateRowmap(s[index]);
	if(temp == 0)
	{
		printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
		return FALSE;
	}
	result->head = temp;
	for(index = 1;index < lines-1 ;index++)
	{
		temp->next = PRN_generateRowmap(s[index]);
		if(temp->next == 0)
			goto fail;
		temp->next->last = temp;
		temp = temp->next;
	}
	temp->next = PRN_generateRowmap(s[index]);
	if(temp->next == 0)
		goto fail;
	result->tail = temp->next;
	result->tail->last = temp;
	result->count = lines;
	return TRUE;
fail:
	printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
	motor_stop();//20210915 add by West, 
				 //make sure motor stop when exception occure.
				 //Prevent from AP coredump when motor running(or motor will literally burn in few second).
	head = result->head;
	while(head!=NULL)
	{
		temp = head;
		head = head->next;
		free(temp);
	}

	return FALSE;
}
//static 
BSP_BOOL line2bitmap(char *string,unsigned char length,unsigned char fontID,struct ROWMAP_PAIR *result)
{
	BSP_BOOL status;
	SUBROWMAP *line;
	static unsigned char table[PRN_MAX_ROW_HEIGHT][PRN_DOT_LINE_BYTES]={0};
	line = (SUBROWMAP *)table;
	int i,j,k;
	unsigned long unit = 0,width;
	unsigned long bitmap;

	unsigned long int start_bit=0;
	unsigned long count = 0;
	unsigned long int code;
	unsigned char *source_bitmap;
	UINT8	FontID;
	UINT8	*pSrcBmp;
	UINT32	factor = 1;
	
//	g_length = length;
	FontID = fontID & 0x0F;	// extract font id
	// printf("FontID=%d\n",FontID);
	memset( table, 0x00, sizeof(table) );	// 2010-02-11
	
	while(start_bit<PRN_DOTS_PER_LINE)
	{
//		if((string[count]&0x80)!=0)
		// if( ((string[count]&0x80)!= 0) && (FONT_TABLE[FontID].MBC.code_list != 0) )	// PATCH: 2012-04-30
		if( (string[count]&0x80)!= 0)	// PATCH: 2020-11-02
		{
		      if( (start_bit + FONT_TABLE[FontID].MBC.width) <= PRN_DOTS_PER_LINE )	// PATCH: 2010-02-24
		      	{
			code=0;
			for(i=0;i<FONT_TABLE[FontID].MBC.code_size;i++)
				code|=(string[count+i]<<(8*i));
			source_bitmap = (unsigned char*)Font_getBitMap(code,FontID);
			if( fontID & attrX2HEIGHT )	// double height
			  {
			  factor = 2;
			  source_bitmap = GenerateDoubleHeightBitMap( FONT_TABLE[FontID].height, FONT_TABLE[FontID].MBC.width,  source_bitmap );
			  }

			if( length )
			  {
			  if( (fontID & attrREVERSE) == attrREVERSE )
			    pSrcBmp = ReverseBitmap( 1, FONT_TABLE[FontID].MBC.member_size*factor, FONT_TABLE[FontID].height*factor, FONT_TABLE[FontID].MBC.width, source_bitmap );
			  else
			    pSrcBmp = ReverseBitmap( 0, FONT_TABLE[FontID].MBC.member_size*factor, FONT_TABLE[FontID].height*factor, FONT_TABLE[FontID].MBC.width, source_bitmap );

//			  copyBitMap(fontID & 0xF0, start_bit,&FONT_TABLE[FontID].MBC,FONT_TABLE[FontID].height,source_bitmap,table);
			  copyBitMap(fontID & 0xF0, start_bit,&FONT_TABLE[FontID].MBC,FONT_TABLE[FontID].height*factor,pSrcBmp,table);
//			  length -= 2;
			  length -= 1;
			  }
			else
//			  copyBitMap(fontID & 0x00, start_bit,&FONT_TABLE[FontID].MBC,FONT_TABLE[FontID].height,source_bitmap,table);
			  copyBitMap(fontID & 0x00, start_bit,&FONT_TABLE[FontID].MBC,FONT_TABLE[FontID].height*factor,source_bitmap,table);
			}
			
		      start_bit+=FONT_TABLE[FontID].MBC.width;
		      count+=FONT_TABLE[FontID].MBC.code_size;
		}
		else
		{
		      if( (start_bit + FONT_TABLE[FontID].SBC.width) <= PRN_DOTS_PER_LINE )	// PATCH: 2010-02-24
		      	{
			code=string[count];
			// printf("input string code=%d\n",code);
			source_bitmap = (unsigned char*)Font_getBitMap(code,FontID);
			if( fontID & attrX2HEIGHT )	// double height
			  {
			  factor = 2;
			  source_bitmap = GenerateDoubleHeightBitMap( FONT_TABLE[FontID].height, FONT_TABLE[FontID].SBC.width,  source_bitmap );
			  }
			
			if( length )
			  {
			  if( (fontID & attrREVERSE) == attrREVERSE )
			    pSrcBmp = ReverseBitmap( 1, FONT_TABLE[FontID].SBC.member_size*factor, FONT_TABLE[FontID].height*factor, FONT_TABLE[FontID].SBC.width, source_bitmap );
			  else
			    pSrcBmp = ReverseBitmap( 0, FONT_TABLE[FontID].SBC.member_size*factor, FONT_TABLE[FontID].height*factor, FONT_TABLE[FontID].SBC.width, source_bitmap );
			    
			  copyBitMap(fontID & 0xF0, start_bit,&FONT_TABLE[FontID].SBC,FONT_TABLE[FontID].height*factor,pSrcBmp,table);
			  length -= 1;
			  }
			else
			  copyBitMap(fontID & 0x00, start_bit,&FONT_TABLE[FontID].SBC,FONT_TABLE[FontID].height*factor,source_bitmap,table);
			}
			
		      start_bit+=FONT_TABLE[FontID].SBC.width;
		      count++;
		}
	}
	//20201030 comment out for AS350+ no need to endian transfer
	/*
	unsigned short *pTemp = (unsigned short*)table;
	for(i=0;i<FONT_TABLE[FontID].height*factor;i++)
	{
		for(j=0;j<(PRN_DOT_LINE_BYTES/2);j++)
		{
			if(pTemp[i*PRN_SPI_LEN+j]!=0)
			{
				// DEBUGPRINTF("%X ",pTemp[i*24+j]);
				endian_B2L(&pTemp[i*PRN_SPI_LEN+j]);
				// DEBUGPRINTF("%X ",pTemp[i*24+j]);
			}
		}
	}
	*/
	status = PRN_generateRowmaps(table,&FONT_TABLE[FontID],result, factor);
	return( status );
}
RESPONSE API_text_proc(char *str,UCHAR fontID)
{
#ifdef PRN_DEBUG
	if(cstr==0)
	{
		ERRNO.error = INPUT_PARAMETER_INCORRECTLY;
		ERRNO.info1 = 1;
		return RETURN_ERROR;
	}
//	if(FONT_TABLE[fontID].PRINT_STATUS_IDEL!=SEM_B_WAIT)
//	{
//		ERRNO.error = INPUT_PARAMETER_INCORRECTLY;
//		ERRNO.info1 = 2;
//		return RETURN_ERROR;
//	}
	if((FONT_TABLE[fontID].SBC.type==USELESS_FONT)&&(FONT_TABLE[fontID].MBC.type==USELESS_FONT))
	{
		ERRNO.error = INPUT_PARAMETER_INCORRECTLY;
		ERRNO.info1 = 2;
		return RETURN_ERROR;
	}
#endif

UINT32	fGenBitMap = 0;	// flag for line2bitmap()
UINT8	FontID;
UINT8	strlen;
UINT8	strcnt;
UINT8	attrcnt;
struct ROWMAP *h,*t;
struct FONT *pFont;
int fontType = 0;//1: SBC, 2: MBC, 3: SBC+MBC
unsigned long int i = 0,j,count=0,count_of_width=0;
char temp[PRN_DOTS_PER_LINE/8+8];//+8:add MBCï¿½É¦ï¿½ï¿½iï¿½ï¿½Wï¿½Lï¿½ï¿½ï¿?
struct ROWMAP_PAIR result,temp_result;

	FontID = fontID & 0x0F;	// extract font id
	strlen = str[0];
	strcnt = 0;
	// printf("FontID =%d\n",FontID );
	
	//??«æ??è¨»è§£20201029
	// if( PRN_getStatus() == PRINT_STATUS_PAPEREMPTY )
	  // return( RETURN_ERROR );

//	if(PRINT_BUF[0]!=0)
//	{
		memmove(&PRINT_BUF[PRINT_BUF[0]+1],&str[1],str[0]);
		PRINT_BUF[0] += str[0];
//	}
	// for(int gg=0;gg<36;gg++)
	// 	printf("PRINT_BUF[%d]=%x\n",gg,PRINT_BUF[gg]);
	PRN_releaseFormFeed();
	if(FONT_TABLE[FontID].MBC.bitmap!=0)
		fontType+=2;
	if(FONT_TABLE[FontID].SBC.bitmap!=0)
		fontType+=1;
	
	// printf("fontType =%d\n",fontType );
	memset(temp,0x20,PRN_DOTS_PER_LINE/8+8);
	
	memset(&result,0,sizeof(struct ROWMAP_PAIR));
	//count_text_call++;
	for(i=1;i<PRINT_BUF[0]+1;i++)
	{
		// printf("PRINT_BUF[%ld]=%x\n",i,PRINT_BUF[i]);
		if(PRINT_BUF[i]==0x0a)//||PRINT_BUF[i] == 0x0c)//line feed, form feed
		{
			goto genBitmap;
		}
		if(PRINT_BUF[i]==0x0c)
			continue;
//		else if(PRINT_BUF[i]>=0x80)
		// else if( (PRINT_BUF[i]>=0x80) && (FONT_TABLE[FontID].MBC.code_list != 0) )	// 2012-04-30
	
		else if( PRINT_BUF[i]>=0x80 )	// 2020-11-02
		{
			if((fontType&2)!=0)
			{
				if(i+FONT_TABLE[FontID].MBC.code_size>PRINT_BUF[0]+1)
				{//complete
					count = PRINT_BUF[0]+1 - i;
					i++;
					break;
				}
				else
				{
					memcpy(&temp[count],&PRINT_BUF[i],FONT_TABLE[FontID].MBC.code_size);
					count += FONT_TABLE[FontID].MBC.code_size;
					i+=(FONT_TABLE[FontID].MBC.code_size-1);
					count_of_width += FONT_TABLE[FontID].MBC.width;

					strcnt++;
				}
			}
			else
			{//MBC font not exist.
				temp[count] = 0x20;
				count++;
				count_of_width += FONT_TABLE[FontID].SBC.width;
			}
		}
		else if(PRINT_BUF[i]<0x20)
		{
			temp[count] = 0x20;
			count++;
			count_of_width += FONT_TABLE[FontID].SBC.width;
		}
		else
		{
			temp[count] = PRINT_BUF[i];
			count++;
			count_of_width += FONT_TABLE[FontID].SBC.width;

			strcnt++;
		}
		// printf("count_of_width=%ld\tPRN_DOTS_PER_LINE=%d\n",count_of_width,PRN_DOTS_PER_LINE);
		//*************************************
		if(count_of_width >=PRN_DOTS_PER_LINE)
		{
			if(count_of_width > PRN_DOTS_PER_LINE)
			{// >384
			//	if((PRINT_BUF[count-FONT_TABLE[fontID].MBC.code_size])>=0x80)
				// if( ((PRINT_BUF[count-FONT_TABLE[FontID].MBC.code_size])>=0x80) && (FONT_TABLE[FontID].MBC.code_list != 0) )	// 2012-04-30
				if( (PRINT_BUF[count-FONT_TABLE[FontID].MBC.code_size])>=0x80)	// 2020-11-02
				{//last word is MBC
					count -= FONT_TABLE[FontID].MBC.code_size;
					memset(&temp[count],0x20,FONT_TABLE[FontID].MBC.code_size);
					i -= FONT_TABLE[FontID].MBC.code_size;
				}
				else
				{
					count -=1;
					temp[count] = 0x20;
					i--;
				}
			}
			if((PRINT_BUF[i+1]==0x0a)||(PRINT_BUF[i+1]==0x0c))
			{
				i++;
			}
		genBitmap:
			fGenBitMap = 1;
			if(line2bitmap(temp,strcnt,fontID,&temp_result)==TRUE)
			{
				
				if(result.count==0)
				{
					memcpy(&result,&temp_result,sizeof(struct ROWMAP_PAIR));
				}
				else
				{
					result.tail->next = temp_result.head;
					temp_result.head->last = result.tail;
					result.tail = temp_result.tail;
					result.count += temp_result.count;
				}
				// printf("result.count=%ld\n",result.count);
				// double height
//				if( fontID & 0x20 )
//				  {
//				  if( line2bitmapEX( fontID, &temp_result ) == TRUE )
//				    {
//				    result.tail->next = temp_result.head;
//				    temp_result.head->last = result.tail;
//				    result.tail = temp_result.tail;
//				    result.count += temp_result.count;
//				    }
//				  }
			}
			else
				goto fail;
			
			memset(temp,0x20,PRN_DOTS_PER_LINE/8);
			count=0;
			count_of_width = 0;
			
		}
	} // for()
	// for(int g=0;g<i;g++)
	// printf("PRINT_BUF[%d]=%x\n",g,PRINT_BUF[g]);
	// printf("count=%ld\n",count);
	if(count!=0)
	{
		PRINT_BUF[0] = count;
		memmove(&PRINT_BUF[1],&PRINT_BUF[i-count],count);
	}
	else
	{
		if(PRINT_BUF[i-1]==0x0c)
			PRN_findFormFeed();
		PRINT_BUF[0] = 0;

	}
	
	if( !fGenBitMap )	// PATCH: 2011-07-20
	  return( RETURN_OK );	// exit if line2bitmap() is not called
	if(PRN_insert_rows(result.count,result.head,result.tail)==RETURN_FATAL_FAIL)
		goto fail;
	else
	{
		// printf("RETURN_OK\n"  );
		return RETURN_OK;
	}
		
fail:
	printf("fail?\n"  );
	motor_stop();//20210915 add by West, 
				 //make sure motor stop when exception occure.
				 //Prevent from AP segmentation fault when motor running(or motor will literally burn in few second).
	h = result.head;
	while(h!=NULL)
	{
		t = h;
		h = h->next;
		free(t);
	}
	return RETURN_ERROR;
}
unsigned char API_PRNpicture(void *pPic,unsigned long width,unsigned long length,unsigned long x_mov)
{
//	if(PRN_getStatus()==PRINT_STATUS_COMPLETE)
//		PRN_setStatus(PRINT_STATUS_IDEL);
	struct ROWMAP *phead,*pcurr,*plast;
	unsigned char *pic = (unsigned char*)pPic;
	unsigned long i,j,pic_index = 0;
	// printf("width=%d\n",width);
	// printf("length=%d\n",length);
	// printf("x_mov=%d\n",x_mov);
	if((x_mov+width)>PRN_DOT_LINE_BYTES)
	{
		ERRNO.error = INPUT_PARAMETER_INCORRECTLY;
		ERRNO.info1 = 2;
		return FALSE;
	}
	if(pPic==0)
	{
		ERRNO.error = INPUT_PARAMETER_INCORRECTLY;
		ERRNO.info1 =1;
		return FALSE;
	}
	if(length == 0)
	{
		ERRNO.error = INPUT_PARAMETER_INCORRECTLY;
		ERRNO.info1 = 3;
		return FALSE;
	}
//20210730comment out for prevent use pointer as condition
	// do{
	  if( PRN_getStatus() == PRINT_STATUS_PAPEREMPTY )
	    return FALSE;
	    
	  phead = malloc(sizeof(struct ROWMAP));	// PATCH: 2009-11-16
	// }while( phead == NULL );
	
	if(phead == NULL)
	{
		PRN_setStatus(PRINT_STATUS_BUFFERFULL);
		return FALSE;
	}
	memset(phead,0,sizeof(struct ROWMAP));	
		
	if( PRN_getStatus() == PRINT_STATUS_PAPEREMPTY )
	    return FALSE;

	for(j=x_mov;j<(x_mov+width);j++)
		phead->data[j] = pic[pic_index++];
	
	// printf("put graphic after for 1\n");
	// for(j=0;j<PRN_DOT_LINE_BYTES/2;j++)
		// endian_B2L(&phead->data[2*j]);
	setupHeatMotorTime(phead);
	plast=phead;
	// printf("length=%d\n",length);
	for(i=1;i<length;i++)
	{
//20210730comment out for prevent use pointer as condition
		// do{
		  if( PRN_getStatus() == PRINT_STATUS_PAPEREMPTY )
		    break;
		  
		  pcurr= malloc(sizeof(struct ROWMAP));	// PATCH: 2009-11-16
		// }while( pcurr == NULL );
		
		if(pcurr==NULL)
		{
			// while(phead!=0)
			// {
				printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
				motor_stop();//20210915 add by West, 
							 //make sure motor stop when exception occure.
				 			 //Prevent from AP coredump when motor running(or motor will literally burn in few second).
				pcurr = phead;
				phead = phead->next;
				free(pcurr);
			// }
			PRN_setStatus(PRINT_STATUS_BUFFERFULL);
			return FALSE;
		}
		
		if( PRN_getStatus() == PRINT_STATUS_PAPEREMPTY )
		    break;
		memset(pcurr,0,sizeof(struct ROWMAP));
		pcurr->last = plast;
		plast->next = pcurr;
		pic_index=0;
		// printf("put graphic before for2 i=%d\n",i);
		for(j=x_mov;j<(x_mov+width);j++)
		{
			pcurr->data[j] = pic[i*width+pic_index];
			// pcurr->data[j] = ~pic[i*width+pic_index];//modified 20210118 reverse 1 and 0 for turn bitmap format from lcd to printer
			pic_index++;
		}
		// printf("put graphic after for2 i=%d\n",i);
		// for(j=0;j<PRN_DOT_LINE_BYTES/2;j++)
			// endian_B2L(&pcurr->data[2*j]);
		//DEBUGPRINTF("x mov = %d,pic index = %d\r\n",j,pic_index);
		setupHeatMotorTime(pcurr);
		plast = pcurr;
	}
	// printf("put graphic after for(i=1;i<length;i++)\n");
	pcurr->next = 0;
	// endian_B2L(&pTemp[i*24+j]);
	
	PRN_releaseFormFeed();
	PRN_insert_rows(length,phead,pcurr);
	PRN_findFormFeed();
	return TRUE;

}