#include <string.h>
#include "POSAPI.h"
#include "OS_Function.h"


UCHAR ecl_lv1_uti_dhn_buz_1S;
UCHAR ecl_lv1_uti_dhn_buz_2S;
UCHAR ecl_lv1_uti_bufConvert[4];


void ECL_LV1_UTI_Open_Buzzer_1S(void)
{
	UCHAR	buf[6]={0};

	buf[0]=1;
	buf[1]=5;
	buf[2]=1;

	ecl_lv1_uti_dhn_buz_1S = api_buz_open(buf);
}


void ECL_LV1_UTI_Open_Buzzer_2S(void)
{
	UCHAR	buf[6]={0};

	buf[0]=2;
	buf[1]=5;
	buf[2]=1;
	
	ecl_lv1_uti_dhn_buz_2S = api_buz_open(buf);
}


void ECL_LV1_UTI_Beep_1S(void)
{
	api_buz_sound( ecl_lv1_uti_dhn_buz_1S );
}


void ECL_LV1_UTI_Beep_2S(void)
{
	api_buz_sound( ecl_lv1_uti_dhn_buz_2S );
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


void ECL_LV1_UTI_Wait(ULONG timUnit)
{
	UCHAR	dhn;

	dhn = api_tim2_open( timUnit );

	if (dhn == apiOutOfService)
	{
		return;
	}
	
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


void ECL_LV1_UTI_L2C(ULONG iptData, UCHAR * optData)
{
	optData[0]=(UCHAR)(iptData & 0x000000FF);
	optData[1]=(UCHAR)((iptData & 0x0000FF00)>>8);
	optData[2]=(UCHAR)((iptData & 0x00FF0000)>>16);
	optData[3]=(UCHAR)((iptData & 0xFF000000)>>24);
}


UCHAR *ECL_LV1_UTI_L2P(ULONG iptData)
{
	ECL_LV1_UTI_L2C(iptData, ecl_lv1_uti_bufConvert);

	return (UCHAR*)&ecl_lv1_uti_bufConvert;
}


ULONG ECL_LV1_UTI_Get_BitRangeValue(ULONG iptData, UCHAR iptRngHigh, UCHAR iptRngLow)
{
	UCHAR	cntIndex=0;
	ULONG	mskValue=0;

	if ((iptRngHigh > 31) || (iptRngHigh < iptRngLow))
	{
		return 0;
	}

	for (cntIndex=iptRngLow; cntIndex < (iptRngHigh+1); cntIndex++)
	{
		mskValue+=(1 << cntIndex);
	}

	return ((iptData & mskValue) >> iptRngLow);
}


ULONG ECL_LV1_UTI_Set_BitRangeValue(ULONG iptData, UCHAR iptRngHigh, UCHAR iptRngLow, ULONG iptBitRngValue)
{
	UCHAR	cntIndex=0;
	ULONG	mskValue=0;

	if ((iptRngHigh > 31) || (iptRngHigh < iptRngLow))
	{
		return 0;
	}

	for (cntIndex=iptRngLow; cntIndex < (iptRngHigh+1); cntIndex++)
	{
		mskValue+=(1 << cntIndex);
	}

	return ((iptData & ~mskValue) | ((iptBitRngValue << iptRngLow) & mskValue));
}

