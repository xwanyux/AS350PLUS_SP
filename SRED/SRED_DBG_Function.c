//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL                                                    **
//**  PRODUCT  : AS350-X6						                            **
//**                                                                        **
//**  FILE     : SRED_DBG_Function.C						                **
//**  MODULE   : SRED_DBG_xxx()                                             **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/15                                                 **
//**  EDITOR   : Tammy                                                      **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.	        **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>
#include "POSAPI.h"

//AUX Device Handle Number
UINT8	SRED_dbg_dhnAUX = 0;

//Flag to Enable Debug Function
UINT8	SRED_dbg_flgEnable = TRUE;


// ---------------------------------------------------------------------------
// FUNCTION: open auxiliary port for communicaiton. (115200, 8, n, 1)
// INPUT   : port
// OUTPUT  : none
// RETURN  : Device Handle Number
//           apiOutOfService
// ---------------------------------------------------------------------------
UINT8	SRED_DBG_Open_AUX(UINT8 port)
{
	API_AUX			sbuf;

	SRED_dbg_dhnAUX = apiOutOfService;

	if(SRED_dbg_flgEnable == TRUE)
	{
		sbuf.Mode = auxBYPASS;
		sbuf.Baud = COM_115200 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
		sbuf.Tob = 10;
		sbuf.Tor = 50;
		sbuf.Acks = 0;
		sbuf.Resend = 0;

		SRED_dbg_dhnAUX = api_aux_open(port, sbuf);
	}

	return SRED_dbg_dhnAUX;
}

// ---------------------------------------------------------------------------
// FUNCTION: close auxiliary port.
// INPUT   : none
// OUTPUT  : none
// RETURN  : TRUE  - OK
//           FALSE - device error
// ---------------------------------------------------------------------------
void	SRED_DBG_Close_AUX(void)
{
	if(SRED_dbg_flgEnable == TRUE)
	{
		api_aux_close(SRED_dbg_dhnAUX);
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: transmit data to auxiliary port.
// INPUT   : auxDHN  - device handle number
//           sndData - the data to be transmitted
// OUTPUT  : none
// RETURN  : TRUE  - data ready
//           FALSE - device error
// ---------------------------------------------------------------------------
UINT8	SRED_DBG_Tx_AUX(UINT8 auxDHN, UINT8 *sndData)
{
	if(api_aux_txready(auxDHN) == apiReady)
	{
		api_aux_txstring(auxDHN, sndData);
		// 	while (api_aux_txready(auxDHN) != apiReady);
	}
	else
		return FALSE;

	return TRUE;
}

UINT8	SRED_DBG_SubSplit(UINT8 data)
{
	return (data > 9) ? (data - 9 + '@') : (data + '0');
}

void	SRED_DBG_Split(UINT8 *des, UINT8 *src, char pair_len)
{
	UINT8 cnt;

	for(cnt = 0; cnt < pair_len; cnt++, src++)
	{
		*des++ = SRED_DBG_SubSplit((*src & 0xF0) >> 4);
		*des++ = SRED_DBG_SubSplit(*src & 0x0F);
	}
}

void	SRED_DBG_Put_Data(UINT8 iptOption, UINT16 iptLen, UINT8 *iptHex)
{
	UINT8	datBuffer[2 + 3 + 48 + 2] = {0};	//Len(2) + Header(3) + Data(16*3) + New Line(2)
	UINT8	sptHex[16 * 2] = {0};			//Hex(1) -> Ascii(2)
	UINT16	idxDat = 0;
	UINT8	idxRow = 0;
	UINT8	strPosition = 0;

	if(SRED_dbg_flgEnable == TRUE)
	{
		if(iptOption != 0)
		{
			strPosition = 3;
		}

		for(idxDat = 0; idxDat < iptLen; idxDat += 16)
		{
			memset(datBuffer, 0, 55);
			memset(sptHex, 0, 32);

			if(iptOption != 0)
			{
				if(idxDat == 0)
				{
					(iptOption == 1) ? (memset(&datBuffer[2], '>', 2)) : (memset(&datBuffer[2], '<', 2));
					datBuffer[4] = ' ';
				}
				else
				{
					memset(&datBuffer[2], ' ', 3);
				}
			}

			if((iptLen - idxDat) < 16)
			{
				datBuffer[0] = strPosition + (iptLen - idxDat) * 3 + 2;

				SRED_DBG_Split(sptHex, &iptHex[idxDat], (iptLen - idxDat));

				for(idxRow = 0; idxRow < (iptLen - idxDat); idxRow++)
				{
					memcpy(&datBuffer[2 + strPosition + idxRow * 3], &sptHex[idxRow * 2], 2);
					datBuffer[2 + strPosition + idxRow * 3 + 2] = ' ';
				}

				datBuffer[2 + strPosition + (iptLen - idxDat) * 3] = 0x0D;
				datBuffer[2 + strPosition + (iptLen - idxDat) * 3 + 1] = 0x0A;
			}
			else
			{
				datBuffer[0] = strPosition + 16 * 3 + 2;

				SRED_DBG_Split(sptHex, &iptHex[idxDat], 16);

				for(idxRow = 0; idxRow < 16; idxRow++)
				{
					memcpy(&datBuffer[2 + strPosition + idxRow * 3], &sptHex[idxRow * 2], 2);
					datBuffer[2 + strPosition + idxRow * 3 + 2] = ' ';
				}

				datBuffer[strPosition + 50] = 0x0D;
				datBuffer[strPosition + 51] = 0x0A;
			}

			SRED_DBG_Tx_AUX(SRED_dbg_dhnAUX, datBuffer);
		}
	}
}

void	SRED_DBG_Put_Hex(UINT16 iptLen, UINT8 *iptHex)
{
	SRED_DBG_Put_Data(0, iptLen, iptHex);
}

void	SRED_DBG_Put_String(UINT8 iptLen, UINT8 *iptString)
{
	UINT8	datBuffer[2 + 64 + 2] = {0};
	UINT8	datNewLine[2] = {0x0D,0x0A};

	if(SRED_dbg_flgEnable == TRUE)
	{
		if(iptLen <= 64)
		{
			//Length
			datBuffer[0] = iptLen + 2;

			//String Data
			memcpy(&datBuffer[2], iptString, iptLen);

			//New Line
			memcpy(&datBuffer[2 + iptLen], datNewLine, 2);

			SRED_DBG_Tx_AUX(SRED_dbg_dhnAUX, datBuffer);
		}
	}
}

void	SRED_DBG_Put_UCHAR(UINT8 iptUCHAR)
{
	UINT8	datBuffer[2 + 2 + 2] = {0};
	UINT8	datNewLine[2] = {0x0D,0x0A};

	if(SRED_dbg_flgEnable == TRUE)
	{
		//Length
		datBuffer[0] = 2 + 2;

		//Convert to Ascii
		SRED_DBG_Split(&datBuffer[2], &iptUCHAR, 1);

		//New Line
		memcpy(&datBuffer[2 + 2], datNewLine, 2);

		SRED_DBG_Tx_AUX(SRED_dbg_dhnAUX, datBuffer);
	}
}

void	SRED_DBG_Put_UINT(UINT16 iptUINT)
{
	UINT8	datBuffer[2 + 4 + 2] = {0};
	UINT8	datNewLine[2] = {0x0D,0x0A};
	UINT8	datBcdInt[2] = {0};

	if(SRED_dbg_flgEnable == TRUE)
	{
		//Length
		datBuffer[0] = 4 + 2;

		//Convert to Ascii
		datBcdInt[0] = (iptUINT & 0xFF00) >> 8;
		datBcdInt[1] = iptUINT & 0x00FF;
		SRED_DBG_Split(&datBuffer[2], datBcdInt, 2);

		//New Line
		memcpy(&datBuffer[2 + 4], datNewLine, 2);

		SRED_DBG_Tx_AUX(SRED_dbg_dhnAUX, datBuffer);
	}
}

void	SRED_DBG_Put_ULONG(UINT32 iptULONG)
{
	UINT8	datBuffer[2 + 8 + 2] = {0};
	UINT8	datNewLine[2] = {0x0D,0x0A};
	UINT8	datBcdInt[4] = {0};

	if(SRED_dbg_flgEnable == TRUE)
	{
		//Length
		datBuffer[0] = 8 + 2;

		//Convert to Ascii
		datBcdInt[0] = (iptULONG & 0xFF000000) >> 24;
		datBcdInt[1] = (iptULONG & 0x00FF0000) >> 16;
		datBcdInt[2] = (iptULONG & 0x0000FF00) >> 8;
		datBcdInt[3] = iptULONG & 0x000000FF;
		SRED_DBG_Split(&datBuffer[2], datBcdInt, 4);

		//New Line
		memcpy(&datBuffer[2 + 8], datNewLine, 2);

		SRED_DBG_Tx_AUX(SRED_dbg_dhnAUX, datBuffer);
	}
}
