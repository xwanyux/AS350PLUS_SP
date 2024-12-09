#include <string.h>
#include "POSAPI.h"
#include "OS_Function.h"





UCHAR ecl_lv1_uti_dhn_buz;


void ECL_LV1_UTI_OpenBuzzer_1S(void)
{
	UCHAR	buf[3];

	buf[0]=1;
	buf[1]=5;
	buf[2]=1;
	ecl_lv1_uti_dhn_buz = api_buz_open(buf);
}


void ECL_LV1_UTI_BUZ_Beep1(void)
{
	api_buz_sound( ecl_lv1_uti_dhn_buz );
}


UINT ECL_LV1_UTI_pow(UCHAR numBase, UCHAR numPow)
{
	UCHAR i=0;
	UINT numTemp=1;

	if (numPow != 0)
	{
		for (i=0; i<numPow; i++)
			numTemp*=numBase;
	}
	
	return numTemp;
}


void ECL_LV1_UTI_WaitTime(UINT tenms)
{
	ULONG	tick1, tick2;

	//Timer Setting
	tick1=OS_GET_SysTimerFreeCnt();
	
	do{
	  	//Get Timer Value
		tick2=OS_GET_SysTimerFreeCnt();
	} while((tick2 - tick1) < tenms);
}


void ECL_LV1_UTI_Wait(ULONG timUnit)
{
	UCHAR	dhn;

	dhn = api_tim2_open( timUnit );
	while( api_tim2_status( dhn ) != apiReady );
	api_tim2_close( dhn );
}


UINT ECL_LV1_UTI_UpdateCrc(unsigned char ch, unsigned short *lpwCrc)
{	//ISO 14443-4 Annex B UpdateCrc
	ch = (ch^(unsigned char)((*lpwCrc) & 0x00FF));
	ch = (ch^(ch<<4));
	*lpwCrc = (*lpwCrc >> 8)^((unsigned short)ch << 8)^((unsigned short)ch<<3)^((unsigned short)ch>>4);
	return(*lpwCrc);
}


void ECL_LV1_UTI_ComputeCrc(UCHAR CRCType, char *Data, int Length, BYTE *TransmitFirst, BYTE *TransmitSecond)
{	//ISO 14443-4 Annex B ComputeCrc
	unsigned char chBlock;
	unsigned short wCrc;
	switch(CRCType) {
		case 'A':
			wCrc = 0x6363; /* ITU-V.41 */
			break;
		case 'B':
			wCrc = 0xFFFF; /* ISO/IEC 13239 (formerly ISO/IEC 3309) */
			break;
		default:
			return;
	}

	do {
		chBlock = *Data++;
		ECL_LV1_UTI_UpdateCrc(chBlock, &wCrc);
	} while (--Length);
	if (CRCType == 'B')
		wCrc = ~wCrc; /* ISO/IEC 13239 (formerly ISO/IEC 3309) */
	*TransmitFirst = (BYTE) (wCrc & 0xFF);
	*TransmitSecond = (BYTE) ((wCrc >> 8) & 0xFF);
	return;
}
