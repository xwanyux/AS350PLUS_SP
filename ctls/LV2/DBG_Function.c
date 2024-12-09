#include <string.h>
#include "POSAPI.h"
#include "Define.h"


#ifdef _PLATFORM_AS350
#ifdef _SCREEN_SIZE_320x240
#include "USBAPI.h"
#endif
#endif

#define TRUE	1
#define FALSE	0


extern	UCHAR	UT_Split(UCHAR * des, UCHAR * src, char pair_len);
extern	UCHAR	UT_Tx_AUX(UCHAR auxDHN, UCHAR * sndData);


//		AUX Device Handle Number
UCHAR	dbg_dhnAUX=0;

//		Flag to Enable Debug Function
UCHAR	dbg_flgEnable=FALSE;


void DBG_Close_AUX(void)
{
	if (dbg_flgEnable == TRUE)
	{
#ifdef AS350_PLUS
	api_aux_close(dbg_dhnAUX);
	return;
#else

#ifdef _PLATFORM_AS350
#ifdef _SCREEN_SIZE_320x240
		api_usb_close(dbg_dhnAUX);
#else
		api_aux_close(dbg_dhnAUX);
#endif
#else
		api_aux_close(dbg_dhnAUX);
#endif

#endif
	}
}


UCHAR DBG_Open_AUX(UCHAR iptPrtNo)
{
#ifdef AS350_PLUS
	API_AUX			sbuf;
#else
#ifdef _PLATFORM_AS350
#ifdef _SCREEN_SIZE_320x240
	API_USB_PARA	sbuf;
	memset( &sbuf, 0x00, sizeof(API_USB_PARA) );
#else
	API_AUX			sbuf;
#endif
#else
	API_AUX			sbuf;
#endif
#endif

	dbg_dhnAUX=apiOutOfService;
	
	if (dbg_flgEnable == TRUE)
	{
		sbuf.Mode	=auxBYPASS;
		sbuf.Baud	=COM_115200+COM_CHR8+COM_NOPARITY+COM_STOP1;
		sbuf.Tob	=10;
		sbuf.Tor	=50;
		sbuf.Acks	=0;
		sbuf.Resend	=0;

#ifdef AS350_PLUS
		dbg_dhnAUX=api_aux_open(iptPrtNo, sbuf);
		return dbg_dhnAUX;
#else
		
#ifdef _PLATFORM_AS350
#ifdef _SCREEN_SIZE_320x240
		dbg_dhnAUX=api_usb_open(iptPrtNo, sbuf);
#else
		dbg_dhnAUX=api_aux_open(iptPrtNo, sbuf);
#endif
#else
		dbg_dhnAUX=api_aux_open(iptPrtNo, sbuf);
#endif
#endif

	}
	
	return dbg_dhnAUX;
}


void DBG_Put_Data(UCHAR iptOption, UINT iptLen, UCHAR *iptHex)
{
	UCHAR	datBuffer[2+3+48+2]={0};	//Len(2) + Header(3) + Data(16*3) + New Line(2)
	UCHAR	sptHex[16*2]={0};			//Hex(1) -> Ascii(2)
	UINT	idxDat=0;
	UCHAR	idxRow=0;	
	UCHAR	strPosition=0;

	if (dbg_flgEnable == TRUE)
	{
		if (iptOption != 0)
		{
			strPosition=3;
		}
		
		for (idxDat=0; idxDat < iptLen; idxDat+=16)
		{
			memset(datBuffer, 0, 55);
			memset(sptHex, 0, 32);

			if (iptOption != 0)
			{
				if (idxDat == 0)
				{
					(iptOption == 1)?(memset(&datBuffer[2], '>', 2)):(memset(&datBuffer[2], '<', 2));
					datBuffer[4]=' ';
				}
				else
				{
					memset(&datBuffer[2], ' ', 3);
				}
			}
			
			if ((iptLen-idxDat) < 16)
			{
				datBuffer[0]=strPosition+(iptLen-idxDat)*3+2;

				UT_Split(sptHex, &iptHex[idxDat], (iptLen-idxDat));

				for (idxRow=0; idxRow < (iptLen-idxDat); idxRow++)
				{
					memcpy(&datBuffer[2+strPosition+idxRow*3], &sptHex[idxRow*2], 2);
					datBuffer[2+strPosition+idxRow*3+2]=' ';
				}
				
				datBuffer[2+strPosition+(iptLen-idxDat)*3]=0x0D;
				datBuffer[2+strPosition+(iptLen-idxDat)*3+1]=0x0A;
			}
			else
			{
				datBuffer[0]=strPosition+16*3+2;

				UT_Split(sptHex, &iptHex[idxDat], 16);

				for (idxRow=0; idxRow < 16; idxRow++)
				{
					memcpy(&datBuffer[2+strPosition+idxRow*3], &sptHex[idxRow*2], 2);
					datBuffer[2+strPosition+idxRow*3+2]=' ';
				}
				
				datBuffer[strPosition+50]=0x0D;
				datBuffer[strPosition+51]=0x0A;
			}

			UT_Tx_AUX(dbg_dhnAUX, datBuffer);
		}
	}
}


void DBG_Put_DateTime(void)
{
	UCHAR	rtcBuffer[12]={0};
	UCHAR	datBuffer[18]={0};

	if (dbg_flgEnable == TRUE)
	{
		api_rtc_getdatetime(0, rtcBuffer);

		//Length
		datBuffer[0]=16;

		//Year(20--)
		datBuffer[2]='2';
		datBuffer[3]='0';

		//RTC
		memcpy(&datBuffer[4], rtcBuffer, 12);

		//New Line
		datBuffer[16]=0x0D;
		datBuffer[17]=0x0A;

		UT_Tx_AUX(dbg_dhnAUX, datBuffer);
	}
}


void DBG_Put_Dialog(UCHAR iptDirection, UINT iptLen, UCHAR *iptData)
{
	if (iptDirection == 0)
	{
		DBG_Put_Data(1, iptLen, iptData);
	}
	else
	{
		DBG_Put_Data(2, iptLen, iptData);
	}
}


void DBG_Put_Hex(UINT iptLen, UCHAR *iptHex)
{
	DBG_Put_Data(0, iptLen, iptHex);
}


void DBG_Put_UCHAR(UCHAR iptUCHAR)
{
	UCHAR	datBuffer[2+2+2]={0};
	UCHAR	datNewLine[2]={0x0D,0x0A};

	if (dbg_flgEnable == TRUE)
	{
		//Length
		datBuffer[0]=2+2;
		
		//Convert to Ascii
		UT_Split(&datBuffer[2], &iptUCHAR, 1);
		
		//New Line
		memcpy(&datBuffer[2+2], datNewLine, 2);
		
		UT_Tx_AUX(dbg_dhnAUX, datBuffer);
	}
}


void DBG_Put_UINT(UINT iptUINT)
{
	UCHAR	datBuffer[2+4+2]={0};
	UCHAR	datNewLine[2]={0x0D,0x0A};
	UCHAR	datBcdInt[2]={0};

	if (dbg_flgEnable == TRUE)
	{
		//Length
		datBuffer[0]=4+2;
		
		//Convert to Ascii
		datBcdInt[0]=(iptUINT & 0xFF00) >> 8;
		datBcdInt[1]=iptUINT & 0x00FF;
		UT_Split(&datBuffer[2], datBcdInt, 2);
		
		//New Line
		memcpy(&datBuffer[2+4], datNewLine, 2);
		
		UT_Tx_AUX(dbg_dhnAUX, datBuffer);
	}
}


void DBG_Put_ULONG(ULONG iptULONG)
{
	UCHAR	datBuffer[2+8+2]={0};
	UCHAR	datNewLine[2]={0x0D,0x0A};
	UCHAR	datBcdInt[4]={0};

	if (dbg_flgEnable == TRUE)
	{
		//Length
		datBuffer[0]=8+2;
		
		//Convert to Ascii
		datBcdInt[0]=(iptULONG & 0xFF000000) >> 24;
		datBcdInt[1]=(iptULONG & 0x00FF0000) >> 16;
		datBcdInt[2]=(iptULONG & 0x0000FF00) >> 8;
		datBcdInt[3]=iptULONG & 0x000000FF;
		UT_Split(&datBuffer[2], datBcdInt, 4);
		
		//New Line
		memcpy(&datBuffer[2+8], datNewLine, 2);
		
		UT_Tx_AUX(dbg_dhnAUX, datBuffer);
	}
}


void DBG_Put_String(UCHAR iptLen, UCHAR *iptString)
{
	UCHAR	datBuffer[2+64+2]={0};
	UCHAR	datNewLine[2]={0x0D,0x0A};

	if (dbg_flgEnable == TRUE)
	{
		if (iptLen <= 64)
		{
			//Length
			datBuffer[0]=iptLen+2;

			//String Data
			memcpy(&datBuffer[2], iptString, iptLen);

			//New Line
			memcpy(&datBuffer[2+iptLen], datNewLine, 2);

			UT_Tx_AUX(dbg_dhnAUX, datBuffer);
		}
	}
}


void DBG_Put_Text(char *iptText)
{
	UCHAR	datBuffer[2+64+2]={0};
	UCHAR	datNewLine[2]={0x0D,0x0A};
	UINT	iptLen=0;

	if (dbg_flgEnable == TRUE)
	{
		iptLen=(UINT)strlen(iptText);
		if (iptLen <= 64)
		{
			//Length
			datBuffer[0]=iptLen+2;

			//String Data
			memcpy(&datBuffer[2], iptText, iptLen);

			//New Line
			memcpy(&datBuffer[2+iptLen], datNewLine, 2);

			UT_Tx_AUX(dbg_dhnAUX, datBuffer);
		}
	}
}
