/*
 * APK_ParseFci.c
 *
 *  Created on: Mar 7, 2009
 *      Author: charles
 */
#include <string.h>
#include "za9_defines.h"
#include "bsp_types.h"
#include "POSAPI.h"
#include "GDATAEX.h"
#include "EMVAPI.h"
//remove inline Wayne
static UCHAR FCI_ParseTempleteA5(UCHAR type,UCHAR *A5Content,unsigned long A5Len);

// ---------------------------------------------------------------------
UCHAR FCI_Parse(UCHAR type,UCHAR *fci)
{
	unsigned long totalLen;
	unsigned long fciLen;
	unsigned long index = 0;
	unsigned long flag = 0;
	unsigned long PassCondition;
	unsigned long FailCondition;
	unsigned char cnt = 0;
	UCHAR templete;
	UCHAR tag;
	unsigned long tagLen;
	totalLen = fci[index++];
	totalLen += fci[index++]*256;
	templete = fci[index++];
	if(templete!=0x6f)
	{
		return FALSE;
	}
	fciLen = apk_GetBERLEN(&fci[index],&cnt);
	if((totalLen-2)!=(1+cnt+fciLen))
		return FALSE;
	index+=cnt;
	do{
		tag = fci[index++];
		if(tag==0x84)
		{
			flag |= BIT0;//tag 84 is find.
			tagLen = apk_GetBERLEN(&fci[index],&cnt);
			index += (tagLen+cnt);
			if(tagLen==0)
				flag |= BIT1;//tag 84 have error.
		}
		else if(tag==0xa5)
		{
			flag |= BIT2;//tag A5 is find.
			tagLen = apk_GetBERLEN(&fci[index],&cnt);
			index += (cnt);
			if(tagLen==0)
				flag |= BIT3;//tag A5's length = 0.
			else
			{
				if(FCI_ParseTempleteA5(type,&fci[index],tagLen)==FALSE)
					flag |= BIT10;//templet A5 have fail.
			}
			index+=tagLen;
		}
		else if((tag==0x00)||(tag==0xff))//padding
		{
			flag |= BIT8;//fci have padding.
			while(fci[index]==tag)
			{
				index++;
			}
		}
		else
		{

			if((tag&0x1f)==0x1f)
			{//tag is two byte.
				if(tag==0x9f)
				{
					tag = fci[index];
					if((tag==0x38)||(tag==0x11)||(tag==12))
					{
						flag |= BIT6;//fci have templete fail.
					}
				}
				else if((tag==0x5f)&&(fci[index]==0x2d))
					flag |= BIT6;//fci have templete fail.
				else if((tag==0xbf)&&(fci[index]==0x0c))
					flag |= BIT6;//fci have templete fail.
				index++;
			}
			else
			{
				if((tag==0x88)||(tag==0x50)||(tag==0x87))
					flag |= BIT6;//fci have templete fail.
				else
					flag |= BIT7;//fci have unknow tag.
			}

			tagLen = apk_GetBERLEN(&fci[index],&cnt);
			index += (tagLen+cnt);
		}
	}while(totalLen>index);//((totalLen-2)>(index-2));//tatal Len - SW, index - size(totalLen)
	PassCondition = (BIT0|BIT2);
	FailCondition = (BIT6|BIT10);
	if((flag&(PassCondition))!=PassCondition)
	{
		return FALSE;
	}
	if((flag&FailCondition)!=0)
		return FALSE;
	return TRUE;
}

// ---------------------------------------------------------------------
//remove inline Wayne
static UCHAR FCI_ParseTempleteA5(UCHAR type,UCHAR *A5Content,unsigned long A5Len)
{
	unsigned long index = 0;
	unsigned long PassCondition;
	unsigned long flag = 0;
	unsigned char cnt = 0;
	unsigned char tag;
	unsigned long tagLen;
	if(type==2)//adf
		PassCondition = 0;
	else// 0->PSE, 1->DDF
		PassCondition = BIT0;//templete A5 have tag 88.

	while(index<A5Len)
	{
		tag = A5Content[index++];
		if(tag==0x88)
		{
			flag = BIT0;//templete A5 have tag 88.
		}
		else if((tag==0x00)||(tag==0xff))
		{
			flag |= BIT8;//fci have padding.
			while(A5Content[index]==tag)
			{
				index++;
			}
		}
		else if((tag&0x1f)==0x1f)
		{
			index++;//tag have two byte.
		}
		tagLen = apk_GetBERLEN(&A5Content[index],&cnt);
		index += (tagLen+cnt);
	}
	if((flag&PassCondition)!=PassCondition)
		return FALSE;
	return TRUE;
}
