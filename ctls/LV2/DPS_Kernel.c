#include <string.h>
#include <stdlib.h>
#include "POSAPI.h"
#include "ECL_Tag.h"
#include "ECL_Tag_Define.h"
#include "Define.h"
#include "Datastructure.h"
#include "Function.h"
#include "DPS_Kernel_Define.h"
#include "Glv_ReaderConfPara.h"
#include "ODA_Record.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Function.h"
#include "VAP_ReaderInterface_Define.h"
#include "api_pcd_vap_Command.h"
#include "DBG_Function.h"

#ifndef _PLATFORM_AS210
#include "LCDTFTAPI.h"
#else
#include "xioapi.h"
#endif

// Add by Wayne 2020/08/21 to avoid compiler warning
#include "UTILS_CTLS.H"


extern void ETP_Load_TransactionData(void);


//DBG_Function.c
extern	UCHAR	dbg_flgEnable;
extern	UCHAR	dbg_dhnAUX;

//ETP_entrypoint.c
extern	UCHAR 	etp_flgIsuUpdateErr;
extern	UCHAR	etp_rcvData[ETP_BUFFER_SIZE];	//Receive Data Buffer
extern	UINT	etp_rcvLen;						//Receive Length
extern	OUTCOME etp_Outcome;
extern	UCHAR 	etp_flgRestart;
extern  UCHAR 	etp_flgComError;
extern	UCHAR	etp_flgCmdTimeout;

//ECL_CAPK.c
extern	CAPK	glv_CAPK[CAPK_NUMBER];

//VAP_ReaderInterface.c
extern	UCHAR	vap_rif_dhnAUX;
extern	UCHAR	vap_rif_rcvLen;
extern	UCHAR	vap_rif_rcvBuffer[VAP_RIF_BUFFER_SIZE];
extern	UCHAR	vap_rif_sndBuffer[VAP_RIF_BUFFER_SIZE];
extern	UCHAR	VAP_Reset_Display_Flag;
extern	UCHAR	VAP_Reset_Command_Occured;
extern	void	VAP_RIF_TRA_ShowStatus(void);
extern	void	VAP_RIF_TRA_Issuer_Update(void);

//api_pcd_Vap_Command.c
extern	UCHAR	SchemeID;
extern	UCHAR	D1_Data[150];		//Used for Ready For Sale D1 Part, D2 Part
extern	UCHAR	Tag57_Data[19];
extern	UINT	D1_Data_Length;
extern	UCHAR	Tag57_Data_Length;
extern	UCHAR	L3_Response_Code;
extern	UCHAR	api_pcd_vap_flgMutAuthenticate;
extern	UCHAR	api_Issuer_Update_Success_Flag_For_AOSA;

//api_pcd_vap_Function.c
extern	UCHAR	Issuer_Script_Data[1024];

//UTILS.c
extern	UCHAR	ut_flgLenOfT_9F80;

//Receive Buffer
//UCHAR	dps_rcvBuff[DEP_BUFFER_SIZE_RECEIVE] = {0};	//Receive Data Buffer
UCHAR	*dps_rcvBuff;
UINT	dps_rcvLen = 0;				//Receive Length

//Send Buffer
//UCHAR	dps_sendBuff[DEP_BUFFER_SIZE_SEND] = {0};	//Send Data Buffer
UCHAR	*dps_sendBuff;
UINT	dps_sendLen = 0;				//Send Length

//GPO Response Buffer
//UCHAR	dps_GPOBuff[DEP_BUFFER_SIZE_RECEIVE] = {0};	//Recv GPO Buffer
UCHAR	*dps_GPOBuff;
UINT	dps_GPOBuffLen = 0;				//Recv GPO Length

//Read Record Response Buffer
//UCHAR	dps_RRBuff[DEP_BUFFER_SIZE_RECEIVE] = {0};	//Recv RR Buffer
UCHAR	*dps_RRBuff;
UINT	dps_RRBuffLen = 0;				//Recv RR Length

//D-PAS MS Mode
UCHAR	MS_Mode = FALSE;

//Cryptogram Type
UCHAR 	dps_AAC = 0;
UCHAR 	dps_TC = 0;
UCHAR 	dps_ARQC = 0;

//Mode - Indicator
UCHAR	dps_Decline_Required_by_Reader = 0;
UCHAR	dps_Online_Required_by_Reader  = 0;

//Track1,Track2 Data
UCHAR 	dps_MSD_Track1_Data[79] = {0};	//[ISO7813] Track 1 consists of up to 79 alphanumeric characters (include DCVV Plus and Extended Unpredictable Number)
UINT 	dps_MSD_Track1_Data_Length = 0;
UCHAR	dps_MSD_Track2_Data[40] = {0};	//[ISO7813] Track 2 consists of up to 40 numeric digits
UINT	dps_MSD_Track2_Data_Length = 0;

//Issuer Update Processing supported
UCHAR	dps_flgIssuerUpdate = FALSE;
UCHAR	flgIST = FALSE;	//Issuer Script Template flag

//Store TVR and TSI for second tap
UCHAR	dps_bufTVR[3 + ECL_LENGTH_95] = {0};
UCHAR	dps_bufTSI[3 + ECL_LENGTH_9B] = {0};
ECL_TAG dps_tagTVR = {dps_bufTVR,dps_bufTVR + 3};
ECL_TAG dps_tagTSI = {dps_bufTSI,dps_bufTSI + 3};

//Flag
UCHAR	dps_flgKrnDiscover = FALSE;

UCHAR	dps_Issuer_Update_Flag = FALSE;

UCHAR	dps_flgListAIDSelMethod = FALSE;
UCHAR	dps_flgZIPconfig = FALSE;


struct track1_character_set{
	UCHAR	character;
	UCHAR	binary;
};
typedef struct track1_character_set TRACK1_CHARACTER_SET;
#define TRACK1_CHARACTER_SET_LEN sizeof(TRACK1_CHARACTER_SET)

const TRACK1_CHARACTER_SET characterTable[CHARACTER_NUM] = {
	{' ',0x40},		{'@',0x20},
	{'!',0x01},		{'A',0x61},
	{'\"',0x02},	{'B',0x62},
	{'#',0x43},		{'C',0x23},
	{'$',0x04},		{'D',0x64},
	{'%',0x45},		{'E',0x25},
	{'&',0x46},		{'F',0x26},
	{'\'',0x07},	{'G',0x67},
	{'(',0x08},		{'H',0x68},
	{')',0x49},		{'I',0x29},
	{'*',0x4A},		{'J',0x2A},
	{'+',0x0B},		{'K',0x6B},
	{',',0x4C},		{'L',0x2C},
	{'-',0x0D},		{'M',0x6D},
	{'.',0x0E},		{'N',0x6E},
	{'/',0x4F},		{'O',0x2F},
	{'0',0x10},		{'P',0x70},
	{'1',0x51},		{'Q',0x31},
	{'2',0x52},		{'R',0x32},
	{'3',0x13},		{'S',0x73},
	{'4',0x54},		{'T',0x34},
	{'5',0x15},		{'U',0x75},
	{'6',0x16},		{'V',0x76},
	{'7',0x57},		{'W',0x37},
	{'8',0x58},		{'X',0x38},
	{'9',0x19},		{'Y',0x79},
	{':',0x1A},		{'Z',0x7A},
	{';',0x5B},		{'[',0x3B},
	{'<',0x1C},		{'\\',0x7C},
	{'=',0x5D},		{']',0x3D},
	{'>',0x5E},		{'^',0x3E},
	{'?',0x1F},		{'_',0x7F}
};


UCHAR ZIP_Create_MSDTrackNoDCVV(void)
{
	UCHAR	*ptrData = NULLPTR;
	UINT	lenOfV = 0;
	UINT	Cnt = 0, Offset = 0;
	UCHAR	tempBuffer[40] = { 0 };


	//Reset Data
	dps_MSD_Track1_Data_Length = 0;
	memset(dps_MSD_Track1_Data, 0x00, 79);
	dps_MSD_Track2_Data_Length = 0;
	memset(dps_MSD_Track2_Data, 0x00, 40);


	// Track 1 Data 
	UT_Get_TLVLengthOfV(glv_tag56.Length, &lenOfV);
	if (lenOfV == 0)
		return FAIL;
	ptrData = glv_tag56.Value;

	dps_MSD_Track1_Data[0] = '%';
	memcpy(&dps_MSD_Track1_Data[1], ptrData, lenOfV);
	dps_MSD_Track1_Data[1 + lenOfV] = '?';
	//LRC
	dps_MSD_Track1_Data_Length = lenOfV + 2;
	dps_MSD_Track1_Data[lenOfV + 2] = DPS_7bit_LRC(dps_MSD_Track1_Data, dps_MSD_Track1_Data_Length, 1);
	dps_MSD_Track1_Data_Length += 1;


	// Track 2 Data 
	UT_Get_TLVLengthOfV(glv_tag57.Length, &lenOfV);
	if (lenOfV == 0)
		return FAIL;

	UT_Split(tempBuffer, glv_tag57.Value, lenOfV);

	dps_MSD_Track2_Data[Offset + Cnt] = ';';
	Offset++;
	while (Cnt < (2 * lenOfV) - 1)		// not counting the last byte was 'F'(padding) 
	{
		if (tempBuffer[Cnt] == 'D')		// 'D'(0x44) => '='(0x3D)
			dps_MSD_Track2_Data[Offset + Cnt] = 0x3D;
		else
			dps_MSD_Track2_Data[Offset + Cnt] = tempBuffer[Cnt];
		Cnt++;
	}
	dps_MSD_Track2_Data[Offset + Cnt] = '?';

	//LRC
	dps_MSD_Track2_Data_Length = Offset + Cnt + 1;
	dps_MSD_Track2_Data[dps_MSD_Track2_Data_Length] = DPS_7bit_LRC(dps_MSD_Track2_Data, dps_MSD_Track2_Data_Length, 2);
	dps_MSD_Track2_Data_Length += 1;

	return SUCCESS;
}

UCHAR ZIP_Create_MSDTrack(void)
{
	UCHAR	*ptrData = NULLPTR;
	UINT	lenOfV = 0;
	UCHAR	numIndex = 0;
	UCHAR	lenPAN = 0;
	UINT	len9F37 = 0;
	UINT	lenData = 0;
	UCHAR	lenPadding = 0;
	UCHAR	DCVV[3] = { 0 };
	UCHAR	i = 0;
	UCHAR	temp_Track2_Data[38] = { 0 }, tempBuffer[38] = { 0 };
	UCHAR	flagTrack1_long = 0; // check if len of track1 > 68
	UCHAR	Emp[5] = { 0x00 };

	//Reset Data
	dps_MSD_Track1_Data_Length = 0;
	memset(dps_MSD_Track1_Data, 0x00, 79);
	dps_MSD_Track2_Data_Length = 0;
	memset(dps_MSD_Track2_Data, 0x00, 40);

	DBG_Put_String(25, (UCHAR*)&"=====ZIP-CreateTrack=====:");
	UINT L;
	UT_Get_TLVLengthOfV(glv_tag56.Length, &L);
	if (L > 68)
		flagTrack1_long = 1;
	else
		flagTrack1_long = 0;
	//UT_Get_TLVLengthOfV(glv_tag9F37.Length, &L);

	//Find DCVV value
	UT_Get_TLVLengthOfV(glv_tag9F7E.Length, &lenOfV);
	if (lenOfV != 0)
		memcpy(DCVV, glv_tag9F7E.Value, lenOfV);
	else
	{
		UT_Get_TLVLengthOfV(glv_tag9F80.Length, &lenOfV);
		if (lenOfV != 0)
			memcpy(DCVV, glv_tag9F80.Value, lenOfV);
	}


	//Update Track 1 Data (ASCII format)
	UT_Get_TLVLengthOfV(glv_tag56.Length, &lenOfV);
	if (lenOfV == 0)
		return FAIL;

	ptrData = glv_tag56.Value;

	//Field 1 - Start Sentinel
	dps_MSD_Track1_Data[lenData++] = '%';

	//Field 2 - Format Code
	dps_MSD_Track1_Data[lenData++] = *ptrData;	//B
	ptrData++;

	//Field 3 - PAN
	for (numIndex = 0; numIndex < (16 + 1); numIndex++)	//PAN(16) + Separator(1)
	{
		if (*ptrData == '^')
		{
			lenPAN = numIndex;
			break;
		}
		dps_MSD_Track1_Data[lenData++] = *ptrData;
		ptrData++;
	}

	//Field 4 - Field Separator
	dps_MSD_Track1_Data[lenData++] = *ptrData;	//^
	ptrData++;	//Point to Cardholder Name

				//Field 5 - Cardholder Name
	memcpy(&dps_MSD_Track1_Data[lenData], ptrData, 26);
	lenData += 26;
	ptrData += 26;	//Point to Field Separator

					//Field 6 - Field Separator
	dps_MSD_Track1_Data[lenData++] = *ptrData;	//^
	ptrData++;	//Point to Expiry Date

				//Field 7 - Expiry Date
	memcpy(&dps_MSD_Track1_Data[lenData], ptrData, 4);
	lenData += 4;
	ptrData += 4;	//Point to Service Code

					//Field 8 - Service Code
	memcpy(&dps_MSD_Track1_Data[lenData], ptrData, 3);
	lenData += 3;
	ptrData += 3;	//Point to IDD in position 54

			//Field 9 - RFU / PVD
			/*If the PAN is less than 16 digits, zeroes must be padded after Service Code (in the card during personalization)
			to ensure that DCVV values are still inserted at the positions shown.*/
	memcpy(&dps_MSD_Track1_Data[lenData], ptrData, 1);
	lenData++;
	ptrData++;


	//Field 10 - IDD
	memcpy(&dps_MSD_Track1_Data[lenData], ptrData, 4);
	lenData += 4;
	ptrData += 4;	//Point to IDD in position 55

					//Field 11 - DCVV
	for (i = 0; i < 3; i++)
	{
		dps_MSD_Track1_Data[lenData] = DCVV[i] + 0x30;
		lenData++;
	}
	ptrData += 3;	//Point to IDD in position 62

					//Field 12 - ATC
	memcpy(&dps_MSD_Track1_Data[lenData], ptrData, 3);
	lenData += 3;
	ptrData += 3;	//point to position 65

					//Field 13 - UN
					//Insert 2 digit numeric UN data from the terminal generated UN sent in PDOL to card (least significant digits of the UN) in positions 65-66
	UT_Get_TLVLengthOfV(glv_tag9F37.Length, &len9F37);
	if (flagTrack1_long)	// about DCVV+
	{
		UT_Split(&dps_MSD_Track1_Data[lenData], glv_tag9F37.Value, len9F37);
		lenData += 2 * len9F37;
		ptrData += 2 * len9F37;

		if (memcmp(ptrData, Emp, 5))
		{
			memcpy(&dps_MSD_Track1_Data[lenData], ptrData, 5);	//remain 5 byte (byte:73~77)
			lenData += 5;
		}
	}
	else	//origianl
	{
		if (glv_tag9F37.Length[0] > 1)
		{
			//If the Unpredictable Number is longer than 1 byte then insert the complete Unpredictable Number into Track 1
			memcpy(&dps_MSD_Track1_Data[lenData], glv_tag9F37.Value, 2);
			lenData += 2;
		}
		else
		{
			UT_Split(&dps_MSD_Track1_Data[lenData], glv_tag9F37.Value, 1);
			lenData += 2;
		}
	}

	//Field 14 - End Sentinel
	dps_MSD_Track1_Data[lenData++] = '?';

	//LRC
	dps_MSD_Track1_Data_Length = lenData;
	dps_MSD_Track1_Data[lenData] = DPS_7bit_LRC(dps_MSD_Track1_Data, dps_MSD_Track1_Data_Length, 1);
	dps_MSD_Track1_Data_Length += 1;
	
	//Reset
	lenData = 0;
	//Update Track 2 Equivalentdata (packed BCD format)
	UT_Split(tempBuffer, glv_tag57.Value, (char)glv_tag57.Length[0]);

	ptrData = tempBuffer;

	//Field 2 - PAN
	for (numIndex = 0; numIndex < (16 + 1); numIndex++)	//PAN(16) + Separator(1)
	{
		if (*ptrData == 'D')
		{
			lenPAN = numIndex;
			break;
		}
		temp_Track2_Data[lenData++] = *ptrData;
		ptrData++;
	}

	//Field 3 - Separator
	temp_Track2_Data[lenData++] = 0x3D;
	ptrData++;	//Point to Expiry Date

				//Field 4 - Expiry Date
	memcpy(&temp_Track2_Data[lenData], ptrData, 4);
	lenData += 4;
	ptrData += 4;	//Point to Service Code

					//Field 5 - Service Code
	memcpy(&temp_Track2_Data[lenData], ptrData, 3);
	lenData += 3;
	ptrData += 3;	//Point to IDD in positon 26

					//Field 6 - RFU
					/*If the PAN is less than 16 digits, zeroes must be padded after Service Code (in the card during personalization)
					to ensure that DCVV values are still inserted at the positions shown.*/
	if (lenPAN < 16)
	{
		lenPadding = 16 - lenPAN;
		memset(&temp_Track2_Data[lenData], 0x30, lenPadding);
		lenData += lenPadding;
	}


	memcpy(&temp_Track2_Data[lenData], ptrData, 1);
	lenData++;
	ptrData++;

	//Field 7 - IDD
	memcpy(&temp_Track2_Data[lenData], ptrData, 4);
	lenData += 4;
	ptrData += 4;

	//Field 8 - DCVV
	for (i = 0; i < 3; i++)
	{
		temp_Track2_Data[lenData] = DCVV[i] + 0x30;
		lenData++;
	}
	ptrData += 3;	//Point to IDD in position 34

					//Field 9 - ATC
	memcpy(&temp_Track2_Data[lenData], ptrData, 3);
	lenData += 3;
	ptrData += 3;	//Point to position 37

					//Field 10 - UN
					//Insert 2 digit numeric UN data from the terminal generated UN sent in PDOL to card (least significant digits of the UN) in positions 37-38
	if (0 && glv_tag9F37.Length[0] > 1)
	{
		//If the Unpredictable Number is longer than 1 byte then only the leftmost byte (2 decimal digits) is placed into Track 2
		memcpy(&temp_Track2_Data[lenData], glv_tag9F37.Value, 2);
		lenData += 2;
		ptrData += 2;
	}
	else
	{
		UT_Split(&temp_Track2_Data[lenData], glv_tag9F37.Value, 1);
		lenData += 2;
		ptrData += 2;
	}

	//if (*ptrData == 'F')
	//{
	//	//temp_Track2_Data[lenData++] = *ptrData;
	//}

	//Field 1 - Start Sentinel
	dps_MSD_Track2_Data[0] = ';';

	memcpy(&dps_MSD_Track2_Data[1], temp_Track2_Data, lenData);

	//Field 11 - End Sentinel
	dps_MSD_Track2_Data[1 + lenData] = '?';

	//LRC
	dps_MSD_Track2_Data_Length = 1 + lenData + 1;
	dps_MSD_Track2_Data[2 + lenData] = DPS_7bit_LRC(dps_MSD_Track2_Data, (2 + lenData), 2);
	dps_MSD_Track2_Data_Length += 1;

	//DBG_Put_String(10, (UCHAR*)&"new Track1");
	//DBG_Put_Hex(dps_MSD_Track1_Data_Length, dps_MSD_Track1_Data);
	//DBG_Put_String(10, (UCHAR*)&"new Track2");
	//DBG_Put_Hex(dps_MSD_Track2_Data_Length, dps_MSD_Track2_Data);
	return SUCCESS;
}

UCHAR ZIP_RAD_Process(void)
{
	UINT	tag94_len = 0, i;
	UCHAR	SFINUM = 0, rspCode, AFL_B4 = 0, j;
	UCHAR	*ptrData = NULLPTR, *ptrData1 = NULLPTR, *ptrData2 = NULLPTR;
	UCHAR	lenOfL = 0;
	UINT	lenOfV = 0;
	UCHAR	*odaRecAddr = NULLPTR;
	UINT	odaRecLen = 0;
	UINT	len56 = 0, len57 = 0, lenDCVV = 0, len70 = 0;
	UINT	lenUnexpectTag = 0;
	UINT	lenCnt = 0;
	UCHAR	lenOfT = 0;
	UCHAR	tag70[1] = { 0x70 }, tag56[1] = { 0x56 }, tag57[1] = { 0x57 };
	UCHAR	tag9F7E[2] = { 0x9F, 0x7E }, tag9F80[2] = {0x9F, 0x80};

	DBG_Put_String(31, (UCHAR*)"==== Read Application Data ====");	// ==== [Debug] ====

	ODA_Clear_Record();
	odaRecAddr = &oda_bufRecord[2];

	UT_Get_TLVLengthOfV(glv_tag94.Length, &tag94_len);

	//Step 1 : Application File Locator (AFL)
	if (tag94_len == 0)
		return SUCCESS;

	//Step 2 : Read all entries defined in the AFL
	for (i = 0; i < tag94_len; i += 4)
	{
		SFINUM = glv_tag94.Value[i];
		SFINUM = SFINUM >> 3;
		AFL_B4 = glv_tag94.Value[i + 3];

		for (j = glv_tag94.Value[i + 1]; j <= glv_tag94.Value[i + 2]; j++)	//First Record(Byte2) and Last Record(Byte 3)
		{
			//Reset the Read Record Buff
			dps_RRBuffLen = 0;
			memset(dps_RRBuff, 0, DEP_BUFFER_SIZE_RECEIVE);

			//Send READ RECORD Command and Receive
			rspCode = ECL_APDU_Read_Record(j, SFINUM, &dps_RRBuffLen, dps_RRBuff);
			/*DBG_Put_String(7, (UCHAR*)&"RR Res:");
			DBG_Put_Hex(dps_RRBuffLen, dps_RRBuff);*/
			if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
				etp_flgCmdTimeout = TRUE;

			if (rspCode == ECL_LV1_SUCCESS)
			{
				rspCode = UT_Check_SW12(&dps_RRBuff[dps_RRBuffLen - 2], STATUSWORD_9000);
				if (rspCode != TRUE)
					return FAIL;

				if (AFL_B4)	//Number of Records Involved in Offline Data Authentication (ODA)
				{
					//SFINUM : Byte1
					//RECNUM : record �Ӽ��`�p
					//dps_RRBuff : Read Record Response Buffer(���tSW1�BSW2)
					//DPS_RAD_SAD_Data(SFINUM, RECNUM, dps_RRBuffLen, dps_RRBuff);	//save application data
					//AFL_B4--;
					//RECNUM++;
					//					-------------------
					//oda_bufRecord[] = | Record Template |
					//                  -------------------
					if (SFINUM < 11)
					{
						ptrData = UT_Find_Tag(tag70, (dps_RRBuffLen - 2), dps_RRBuff);
						ptrData++;
						UT_Get_TLVLengthOfL(ptrData, &lenOfL);
						UT_Get_TLVLengthOfV(ptrData, &lenOfV);

						memcpy(odaRecAddr, &ptrData[lenOfL], lenOfV);

						odaRecAddr += lenOfV;
						odaRecLen += lenOfV;
						UT_S2C(odaRecLen, oda_bufRecord);
					}
					//					------------------------------------
					//oda_bufRecord[] = | Tag70 | Length | Record Template |
					//                  ------------------------------------
					else
					{
						odaRecLen += (dps_RRBuffLen - 2);
						UT_S2C(odaRecLen, oda_bufRecord);
						memcpy(odaRecAddr, dps_RRBuff, dps_RRBuffLen - 2);
						odaRecAddr += (dps_RRBuffLen - 2);
					}
					AFL_B4--;
				}
			}
			else if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))	//Timeout 
				return 0xFE;
			else if (rspCode == ECL_LV1_STOP_ICCARD)	//contact interrupt
				return 0xFF;
			else
				return rspCode;
		}
	}
	//if ((DPS_Check_ZIP_AID() == SUCCESS) || (MS_Mode == TRUE))	
	if (1)
	{
		//ptrData = apk_EMVCL_FindTag(0x70, 0x00, dps_RRBuff);
		ptrData = UT_Find_Tag(tag70, (dps_RRBuffLen - 2), dps_RRBuff);
		ptrData++;
		
		if (ptrData == NULLPTR)
			return FAIL;
		UT_Get_TLVLengthOfV(ptrData, &len70);
		// =====check TLV struct=====  ZIP 4.3.2.Unexpected tag
		ptrData += 1;

		while (lenCnt < len70)
		{
			UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);			
			if (*ptrData == 0x56 || *ptrData == 0x57 || (*ptrData == 0x9F && *(ptrData + 1) == 0x7E)
				|| (*ptrData == 0x9F && *(ptrData + 1) == 0x80))
				;
			else
				lenUnexpectTag += lenOfT + lenOfL + lenOfV;

			if ((*ptrData == 0x9F && *(ptrData + 1) == 0x80))  //9F80 against EMV tag rule (only for ZIP)
			{
				lenOfT = 0x02;
				lenOfL = 0x01;
				lenOfV = 0x03;
			}
			ptrData += lenOfT + lenOfL + lenOfV;
			lenCnt += lenOfT + lenOfL + lenOfV;
		}
		//Save Track 1 data
		//ptrData = apk_EMVCL_FindTag(0x56, 0x00, dps_RRBuff);
		ptrData = UT_Find_Tag(tag56, (dps_RRBuffLen - 2), dps_RRBuff);		
		if (ptrData != NULLPTR)
		{
			ptrData++;
			UT_Get_TLVLengthOfL(ptrData, &lenOfL);
			UT_Get_TLVLengthOfV(ptrData, &lenOfV);
			len56 = lenOfV;
			if (len56 > 76)
				return FAIL;
			UT_Set_TagLength(lenOfV, glv_tag56.Length);
			memcpy(glv_tag56.Value, &ptrData[lenOfL], lenOfV);
		}
		else
			return FAIL;
		//Save Track 2 data
		//ptrData = apk_EMVCL_FindTag(0x57, 0x00, dps_RRBuff);
		ptrData = UT_Find_Tag(tag57, (dps_RRBuffLen - 2), dps_RRBuff);
		if (ptrData != NULLPTR)
		{
			ptrData++;
			UT_Get_TLVLengthOfL(ptrData, &lenOfL);
			UT_Get_TLVLengthOfV(ptrData, &lenOfV);
			len57 = lenOfV;
			if (len57 > 19)
				return FAIL;
			UT_Set_TagLength(lenOfV, glv_tag57.Length);
			memcpy(glv_tag57.Value, &ptrData[lenOfL], lenOfV);
		}
		else
			return FAIL;

		DBG_Put_String(7, (UCHAR*)&"RR Res:");
//		DBG_Put_Hex(dps_RRBuffLen, dps_RRBuff);
		//Step 4 : Dynamic Card Verification Value (DCVV)
		//ptrData1 = apk_EMVCL_FindTag(0x9F, 0x7E, dps_RRBuff);	// 9F7D major ver. >=2
		//ptrData2 = apk_EMVCL_FindTag(0x9F, 0x80, dps_RRBuff);	// 9F7D major = 0x00, 0x01
		ptrData1 = UT_Find_Tag(tag9F7E, (dps_RRBuffLen - 2), dps_RRBuff);
		ptrData2 = UT_Find_Tag(tag9F80, (dps_RRBuffLen - 2), dps_RRBuff);
		if ((ptrData1 != NULLPTR) || (ptrData2 != NULLPTR))	//DCVV present
		{
			DBG_Put_String(14, (UCHAR*)"==== DCVV ====");
			if (ptrData1 != NULLPTR && (glv_tag9F7D.Value[0] == 0x00 || glv_tag9F7D.Value[0] == 0x01))
			{
				// ZIP 4.3.5. Valid Data (tag 9F7D with version less than 2.0 and Tag DCVV 9F7E)
				// return FAIL;
			}
			if (ptrData2 != NULLPTR && glv_tag9F7D.Value[0] >= 0x02)
				return FAIL;

			if (ptrData1 != NULLPTR)
			{
				ptrData1 += 2;
				UT_Get_TLVLengthOfV(ptrData1, &lenDCVV);
				UT_Set_TagLength(lenDCVV, glv_tag9F7E.Length);
				memcpy(glv_tag9F7E.Value, ptrData1 + 1, lenDCVV);
			}
			if (ptrData2 != NULLPTR)
			{
				ptrData2 += 2;
				UT_Get_TLVLengthOfV(ptrData2, &lenDCVV);
				UT_Set_TagLength(lenDCVV, glv_tag9F80.Length);
				memcpy(glv_tag9F80.Value, ptrData2 + 1, lenDCVV);
			}

			/*DBG_Put_String(15, (UCHAR*)&"checking len..1");*/
			if (lenDCVV != 3 || (lenUnexpectTag + len56 + len57 + lenDCVV + 7 != len70))
				return FAIL;
			// Insert UN and DCVV into Track 1 and Track 2 data
			ZIP_Create_MSDTrack();
		}
		else
		{
			//DBG_Put_String(15, (UCHAR*)&"checking len..2");

			if (len56 + len57 + 4 != len70)
				return FAIL;

			ZIP_Create_MSDTrackNoDCVV();
		}
	}
	return SUCCESS;
}

UCHAR ZIP_AIP_AFL_Process(void)
{
	UCHAR	*data_point = 0;
	UCHAR   *ptrTag = 0;
	UCHAR	cnt;
	UINT	Value_Len = 0, lenGPO = 0;
	UCHAR   ZIP_AIP[2] = { 0x00, 0x00 };
	UCHAR   ZIP_AFL[4] = { 0x08, 0x01, 0x01, 0x00 };
	UCHAR	ZIP_GPO80[7] = { 0x06, 0x00, 0x00, 0x08, 0x01, 0x01, 0x00 };
	UCHAR   LenOfL = 0, lenOfT = 0, lenOfL = 0;
	UINT	lenOfV = 0, len77 = 0, lenCnt = 0;
	UCHAR	tag77[1] = { 0x77 }, tag82[1] = { 0x82 }, tag94[1] = { 0x94 };

	UT_EMVCL_GetBERLEN(&dps_GPOBuff[1], &cnt, &Value_Len); // get total length of data elements
	lenGPO = Value_Len;
															// check legal length
	if (Value_Len < 6)	//mandatory data should always include the AIP (2 bytes) and the AFL (multiple of 4 bytes, up to 252 bytes)
		return FAIL;

	if (dps_GPOBuff[0] == 0x80)	// Format 1
	{
		if (memcmp(&dps_GPOBuff[1], ZIP_GPO80, 7))
			return FAIL;
	}
	else if ((dps_GPOBuff[0] == 0x77))	// check response format (2)
	{
		//ptrTag = apk_EMVCL_FindTag(0x77, 0x00, dps_GPOBuff);
		ptrTag = UT_Find_Tag(tag77, lenGPO, dps_GPOBuff);
		ptrTag++;
		UT_EMVCL_GetBERLEN(ptrTag, &LenOfL, &len77);
		ptrTag++;
		while (lenCnt<len77)
		{
			UT_Get_TLVLength(ptrTag, &lenOfT, &lenOfL, &lenOfV);
			ptrTag += lenOfT + lenOfL + lenOfV;
			lenCnt += lenOfT + lenOfL + lenOfV;
		}
		if (lenCnt != len77)
			return FAIL;


		// find AIP[2]
		//data_point = apk_EMVCL_FindTag(0x82, 0x00, &dps_GPOBuff[0]);
		data_point = UT_Find_Tag(tag82, lenGPO, dps_GPOBuff);
		data_point++;
		if (data_point != 0)
		{
			UT_EMVCL_GetBERLEN(data_point, &cnt, &Value_Len);

			if (Value_Len != 2)
				return FAIL;

			if (memcmp(data_point + cnt, ZIP_AIP, 2))
				return FAIL;
		}
		else
			return FAIL;

		//data_point = apk_EMVCL_FindTag(0x94, 0x00, &dps_GPOBuff[0]);
		data_point = UT_Find_Tag(tag94, lenGPO, dps_GPOBuff);
		data_point++;
		if (data_point != 0)
		{
			UT_EMVCL_GetBERLEN(data_point, &cnt, &Value_Len);
			if (Value_Len != 4)
				return FAIL;
		}
		else
			return FAIL;	// AFL not present

		if (memcmp(data_point + cnt, ZIP_AFL, 4))
			return FAIL;
	}
	else
		return FAIL;

	UT_Set_TagLength(2, glv_tag82.Length);
	UT_Set_TagLength(4, glv_tag94.Length);
	memcpy(glv_tag82.Value, ZIP_AIP, 2);
	memcpy(glv_tag94.Value, ZIP_AFL, 4);

	return SUCCESS;
}
UCHAR ZIP_Mandatory_Data_Checks(void)
{
	UCHAR	rspCode;

	if (UT_Check_SW12(&dps_GPOBuff[dps_GPOBuffLen - 2], STATUSWORD_9000))
	{
		if (dps_GPOBuff[0] != 0x77 && dps_GPOBuff[0] != 0x80)	// 2 format of GPO response
			return FAIL;

		rspCode = ZIP_AIP_AFL_Process();	//check whether AIP and AFL are included
		if (rspCode == SUCCESS)
			return SUCCESS;
		else
			return FAIL;
	}
	else
		return FAIL;
}

UCHAR ZIP_Initiate_Process_GPO()
{
	UCHAR	rspCode;
	UINT	Len;

	rspCode = SUCCESS;
	if (rspCode == SUCCESS)
	{
		UT_Get_TLVLengthOfV(glv_tag9F37.Length, &Len);
		dps_sendBuff[0] = 0x83;
		dps_sendBuff[1] = Len;
		memcpy(&dps_sendBuff[2], glv_tag9F37.Value, dps_sendBuff[1]);
		dps_sendLen = 2 + dps_sendBuff[1];

		// send/recv GPO Command
		rspCode = ECL_APDU_Get_ProcessingOptions(dps_sendLen, dps_sendBuff, &dps_rcvLen, dps_rcvBuff);
		if (rspCode == ECL_LV1_SUCCESS)
		{
			dps_GPOBuffLen = dps_rcvLen;
			memcpy(dps_GPOBuff, dps_rcvBuff, dps_rcvLen);
		}
		if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
		{
			etp_flgCmdTimeout = TRUE;
			return 0xFE;
		}
		return rspCode;
	}
	else
		return FAIL;

	return SUCCESS;
}
UCHAR ZIP_Initiate_App_Process()
{
	UCHAR	rspCode = 0;
	UINT	tmpLEN = 0;

	//reset GPO Buffer
	dps_GPOBuffLen = 0;
	memset(dps_GPOBuff, 0x00, DEP_BUFFER_SIZE_RECEIVE);

	//Send GPO command 
	rspCode = ZIP_Initiate_Process_GPO();

	//Contact occur
	if (rspCode == ECL_LV1_STOP_ICCARD)
		return 0xFF;

	//Timeout occur
	if (rspCode == 0xFE)
		return rspCode;

	//Those of tag that will be updated by the terminal should set length.
	UT_Get_TLVLengthOfV(glv_tag95.Length, &tmpLEN);	//TVR		
	if (!tmpLEN)
		glv_tag95.Length[0] = 0x05;

	UT_Get_TLVLengthOfV(glv_tag9B.Length, &tmpLEN);	//TSI		
	if (!tmpLEN)
		glv_tag9B.Length[0] = 0x02;

	rspCode = ZIP_Mandatory_Data_Checks();
	return rspCode;
}
//----------------------------------------------------
//
//	Seperate with dpas		william 2017/08/29/
//
//----------------------------------------------------
void ZIP_TransFlow()
{
	UCHAR rspCode;

	DBG_Put_String(19, (UCHAR *)"===ZIP_TransFlow===");

	rspCode = ZIP_Initiate_App_Process();
	if ((etp_Outcome.Start == ETP_OCP_Start_NA) || (etp_Outcome.Start == ETP_OCP_Start_B) || (etp_Outcome.Start == ETP_OCP_Start_C))
	{
		switch (rspCode)
		{
		case SUCCESS:
			if (dps_Decline_Required_by_Reader == 1)
			{
				DPS_Declined_OutCome();
				break;
			}
			//Read Application Data
			rspCode = ZIP_RAD_Process();
			if (rspCode == SUCCESS)
			{
				dps_Online_Required_by_Reader = 1;
				DPS_Online_Request_OutCome();

				DPS_OutcomeProcess();
				/*rspCode = FAIL;// DPS_CVM_Common_Process();
				if (rspCode == SUCCESS)
				{
				//��ɨM�w�n��ODA�H
				if ((dps_Decline_Required_by_Reader == 0) && (dps_Online_Required_by_Reader == 0))
				{
					DPS_ODA_Process();
				}
					DPS_Processing_Restriction();
					DPS_TAA_Process_Part1();
					DPS_OutcomeProcess();
				}*/
			}
			else if (rspCode == 0xFE)	//Read Record timeout
			{
				//if time out, reset the data that receive from GPO Response, Read Record Response
				ECL_Reset_Tag();
				ETP_Load_TransactionData();

				//Set Timeout Outcome
				ETP_Set_DefaultOutcome(ETP_OUTCOME_Timeout);
				ECL_LV1_RESET();
			}
			else if (rspCode == 0xFF)				//Contact interrupt
			{
				//Disable LED
				UT_Set_LEDSignal(IID_LED_YELLOW, 0, 0);
				etp_Outcome.ocmMsgID = ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
			}
			else
				DPS_End_Application_OutCome();
			break;

		case DPS_Try_Another:
			DPS_Try_Another_Interface_OutCome();
			break;

		case DPS_Select_Next:
			DPS_Select_Next_OutCome();
			break;

		case FAIL:
			DPS_End_Application_OutCome();
			break;

		case 0xFE:	//timeout
			ETP_Set_DefaultOutcome(ETP_OUTCOME_Timeout);
			ECL_LV1_RESET();
			break;

		case 0xFF:	//Contact interrupt
			etp_Outcome.ocmMsgID = ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
			break;
		}

		//reset Cryptogram Type		
		dps_AAC = 0;
		dps_TC = 0;
		dps_ARQC = 0;

		//reset the indicator
		dps_Decline_Required_by_Reader = 0;
		dps_Online_Required_by_Reader = 0;

		//buffer reset
		memset(dps_RRBuff, 0x00, DEP_BUFFER_SIZE_RECEIVE);
		dps_RRBuffLen = 0;
		memset(dps_GPOBuff, 0x00, DEP_BUFFER_SIZE_RECEIVE);
		dps_GPOBuffLen = 0;
		memset(dps_rcvBuff, 0x00, DEP_BUFFER_SIZE_RECEIVE);
		dps_rcvLen = 0;
		memset(dps_sendBuff, 0x00, DEP_BUFFER_SIZE_SEND);
		dps_sendLen = 0;
		//reset flag
		dps_flgIssuerUpdate = FALSE;

		ut_flgLenOfT_9F80 = FALSE;
	}
}


UCHAR DPS_Allocate_Buffer(void)
{
	dps_rcvBuff=malloc(DEP_BUFFER_SIZE_RECEIVE);
	dps_sendBuff=malloc(DEP_BUFFER_SIZE_SEND);
	dps_GPOBuff=malloc(DEP_BUFFER_SIZE_RECEIVE);
	dps_RRBuff=malloc(DEP_BUFFER_SIZE_RECEIVE);

	if ((dps_rcvBuff == NULLPTR) ||
		(dps_sendBuff == NULLPTR) ||
		(dps_GPOBuff == NULLPTR) ||
		(dps_RRBuff == NULLPTR))
	{
		return FAIL;
	}

	return SUCCESS;
}

void DPS_Free_Buffer(void)
{
	free(dps_rcvBuff);
	free(dps_sendBuff);
	free(dps_GPOBuff);
	free(dps_RRBuff);
}

void DPS_DBG_Put_Process(UCHAR iptFigure, UCHAR iptID, UCHAR iptRspCode)
{
	UCHAR	datBuffer[32] = {0};
	UCHAR	msgFigure[2] = {' ',' '};
	UCHAR	msgID[3] = {' ',' ',' '};
	UCHAR	msgRsp[5] = {' ',' ',' ',' ',' '};

	if(dbg_flgEnable == TRUE)
	{
		switch(iptFigure)
		{
			case DPS_FIGURE_1:							msgFigure[0] = '1';	break;
			case DPS_FIGURE_2:							msgFigure[0] = '2';	break;
			case DPS_FIGURE_3:							msgFigure[0] = '3';	break;
			case DPS_FIGURE_4:							msgFigure[0] = '4';	break;
			case DPS_FIGURE_5:							msgFigure[0] = '5';	break;
			case DPS_FIGURE_6:							msgFigure[0] = '6';	break;
			case DPS_FIGURE_7:							msgFigure[0] = '7';	break;
			case DPS_FIGURE_8:							msgFigure[0] = '8';	break;
			case DPS_FIGURE_9:							msgFigure[0] = '9';	break;
			case DPS_FIGURE_10:							msgFigure[0] = '1';
														msgFigure[1] = '0';	break;
			case DPS_FIGURE_11:						    msgFigure[0] = '1';
														msgFigure[1] = '1';	break;
			case DPS_FIGURE_12:							msgFigure[0] = '1';
														msgFigure[1] = '2';	break;
			case DPS_FIGURE_13:							msgFigure[0] = '1';
														msgFigure[1] = '3';	break;
			case DPS_FIGURE_14:							msgFigure[0] = '1';
														msgFigure[1] = '4';	break;
			case DPS_FIGURE_15:							msgFigure[0] = '1';
														msgFigure[1] = '5';	break;
			case DPS_FIGURE_16:							msgFigure[0] = '1';
														msgFigure[1] = '6';	break;
			case DPS_FIGURE_17:							msgFigure[0] = '1';
														msgFigure[1] = '7';	break;
			case DPS_FIGURE_18:							msgFigure[0] = '1';
														msgFigure[1] = '8';	break;
			case DPS_FIGURE_19:							msgFigure[0] = '1';
														msgFigure[1] = '9';	break;
			case DPS_FIGURE_20:							msgFigure[0] = '2';
														msgFigure[1] = '0';	break;
			case DPS_FIGURE_21:							msgFigure[0] = '2';
														msgFigure[1] = '1';	break;
			case DPS_FIGURE_22:							msgFigure[0] = '2';
														msgFigure[1] = '2';	break;
			case DPS_FIGURE_23:							msgFigure[0] = '2';
														msgFigure[1] = '3';	break;
			case DPS_FIGURE_24:							msgFigure[0] = '2';
														msgFigure[1] = '4';	break;
			case DPS_FIGURE_25:							msgFigure[0] = '2';
														msgFigure[1] = '5';	break;
			case DPS_FIGURE_26:							msgFigure[0] = '2';
														msgFigure[1] = '6';	break;
			case DPS_FIGURE_27:							msgFigure[0] = '2';
														msgFigure[1] = '7';	break;
		}

		if(iptID >= 0x1A)
		{
			msgID[1] = ((iptID & 0xF0) >> 4) + '0';
			msgID[2] = ((iptID & 0x0F) % 10) + 'A';
		}
		else
		{
			if(iptID < 10)
			{
				msgID[2] = iptID + '0';
			}
			else if(iptID < 100)
			{
				msgID[1] = (iptID / 10) + '0';
				msgID[2] = (iptID % 10) + '0';
			}
			else
			{
				msgID[0] = (iptID / 100) + '0';
				msgID[1] = ((iptID / 10) % 10) + '0';
				msgID[2] = (iptID % 10) + '0';
			}
		}

		if(iptRspCode == 1)
		{
			msgRsp[0] = 'T';
			msgRsp[1] = 'R';
			msgRsp[2] = 'U';
			msgRsp[3] = 'E';
		}
		else if(iptRspCode == 0)
		{
			msgRsp[0] = 'F';
			msgRsp[1] = 'A';
			msgRsp[2] = 'L';
			msgRsp[3] = 'S';
			msgRsp[4] = 'E';
		}

		//Length
		datBuffer[0] = 14;

		//State
		memcpy(&datBuffer[2], msgFigure, 2);
		datBuffer[4] = '.';

		//ID
		memcpy(&datBuffer[5], msgID, 3);
		datBuffer[8]=' ';

		//Response Code
		memcpy(&datBuffer[9], msgRsp, 5);

		//New Line
		datBuffer[14]=0x0D;
		datBuffer[15]=0x0A;

		UT_Tx_AUX(dbg_dhnAUX, datBuffer);
	}
}

void	DPS_Load_TransactionData(void)
{
	//Terminal Type
	UT_Set_TagLength(ECL_LENGTH_9F35, glv_tag9F35.Length);
	memcpy(glv_tag9F35.Value, &glv_par9F35[5], ECL_LENGTH_9F35);

	//Application Version Number (Reader)
	UT_Set_TagLength(ECL_LENGTH_9F09, glv_tag9F09.Length);
	memcpy(glv_tag9F09.Value, &glv_par9F09[5], ECL_LENGTH_9F09);
}

UCHAR	DPS_Issuer_Update_Process(void)
{
	UINT	lenOfIST = 0;
	UCHAR	*ptrData = NULLPTR;
	UCHAR	*templateTag_ptr = NULLPTR;
	UCHAR	lenOfT = 0;
	UCHAR	lenOfL = 0;
	UINT	lenOfV = 0;
	UINT	lenOfScriptValue = 0;
	UINT	lenOfCmd = 0;
	UCHAR	*ptrCmd = NULLPTR;
	UCHAR	rspCode = 0;

	DBG_Put_String(31, (UCHAR*)"==== Issuer Update Process ====");	// ==== [Debug] ====

	lenOfIST = Issuer_Script_Data[0] + Issuer_Script_Data[1] * 256;

	if(lenOfIST != 0)
	{
		ptrData = &Issuer_Script_Data[2];

		while(lenOfIST)
		{
			//Find Issuer Script Template
			if((*ptrData == 0x71) || (*ptrData == 0x72))
			{
				templateTag_ptr = ptrData;

				UT_Get_TLVLength(templateTag_ptr, &lenOfT, &lenOfL, &lenOfV);

				lenOfScriptValue = lenOfV;

				//Step 1b : Check if only one Issuer Script Template present
				if((*(ptrData + lenOfT + lenOfL + lenOfV) == 0x71) || (*(ptrData + lenOfT + lenOfL + lenOfV) == 0x72))
				{
					DPS_DBG_Put_Process(DPS_FIGURE_27, 0x1B, FALSE);

					//Show TVR and TSI value (for test)
					DBG_Put_String(4, (UCHAR*)"TVR:");
					DBG_Put_Hex(5, glv_tag95.Value);

					DBG_Put_String(4, (UCHAR*)"TSI:");
					DBG_Put_Hex(2, glv_tag9B.Value);

					DPS_End_Application_OutCome();

					return SUCCESS;
				}

				DPS_DBG_Put_Process(DPS_FIGURE_27, 0x1B, TRUE);

				//Step 2
				glv_tag9B.Value[0] |= DPS_TSI_SCRIPT_PROCESSING_WAS_PERFORMED;
				glv_tag9B.Value[0] |= DPS_TSI_ISSUER_AUTHENTICATION_WAS_PERFORMED;

				DPS_DBG_Put_Process(DPS_FIGURE_27, 2, 0xFF);

				ptrData += (lenOfT + lenOfL);
				lenOfIST -= (lenOfT + lenOfL);
			}
			else if((*ptrData == 0x9F) && (*(ptrData + 1) == 0x18))	//Check tag 9F18
			{
				UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
				ptrData += (lenOfT + lenOfL + lenOfV);
				lenOfIST -= (lenOfT + lenOfL + lenOfV);
				lenOfScriptValue -= (lenOfT + lenOfL + lenOfV);
			}
			else if(*ptrData == 0x86)	//Check command
			{
				UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

				lenOfCmd = lenOfV;
				ptrCmd = ptrData + lenOfT + lenOfL;

				dps_rcvLen = 0;
				memset(dps_rcvBuff, 0x00, DEP_BUFFER_SIZE_RECEIVE);
				
				//Step 3
				rspCode = ECL_LV1_DEP(lenOfCmd, ptrCmd, &dps_rcvLen, dps_rcvBuff, 1000);

				DPS_DBG_Put_Process(DPS_FIGURE_27, 3, 0xFF);

				if((rspCode == ECL_LV1_TIMEOUT_USER) || (rspCode == ECL_LV1_TIMEOUT_ISO))
				{
					etp_flgCmdTimeout=TRUE;
				}
				
				if(UT_Check_SW12(&dps_rcvBuff[dps_rcvLen - 2], STATUSWORD_9000))	//Step 4a
				{
					DPS_DBG_Put_Process(DPS_FIGURE_27, 0x4A, TRUE);

					ptrData += (lenOfT + lenOfL + lenOfV);
					lenOfIST -= (lenOfT + lenOfL + lenOfV);
					lenOfScriptValue -= (lenOfT + lenOfL + lenOfV); 
				}
				else if(UT_Check_SW12(&dps_rcvBuff[dps_rcvLen - 2], STATUSWORD_6982))	//Step 4b
				{
					DPS_DBG_Put_Process(DPS_FIGURE_27, 0x4A, FALSE);
					DPS_DBG_Put_Process(DPS_FIGURE_27, 0x4B, TRUE);

					//Step 5a
					glv_tag95.Value[4] |= DPS_TVR_ISSUER_AUTHENTICATION_FAILED;

					DPS_DBG_Put_Process(DPS_FIGURE_27, 0x5A, 0xFF);
					
					//Step 5b
					glv_tag95.Value[4] |= DPS_TVR_SCRIPT_PROCESSING_FAILED_BEFORE_FINAL_GENERATE_AC;

					DPS_DBG_Put_Process(DPS_FIGURE_27, 0x5B, 0xFF);

					//Show TVR and TSI value (for test)
					DBG_Put_String(4, (UCHAR*)"TVR:");
					DBG_Put_Hex(5, glv_tag95.Value);

					DBG_Put_String(4, (UCHAR*)"TSI:");
					DBG_Put_Hex(2, glv_tag9B.Value);

					DPS_End_Application_OutCome();

					return SUCCESS;
				}
				else
				{
					DPS_DBG_Put_Process(DPS_FIGURE_27, 0x4A, FALSE);
					DPS_DBG_Put_Process(DPS_FIGURE_27, 0x4B, FALSE);

					//Step 5b
					glv_tag95.Value[4] |= DPS_TVR_SCRIPT_PROCESSING_FAILED_BEFORE_FINAL_GENERATE_AC;

					DPS_DBG_Put_Process(DPS_FIGURE_27, 0x5B, 0xFF);

					//Show TVR and TSI value (for test)
					DBG_Put_String(4, (UCHAR*)"TVR:");
					DBG_Put_Hex(5, glv_tag95.Value);

					DBG_Put_String(4, (UCHAR*)"TSI:");
					DBG_Put_Hex(2, glv_tag9B.Value);

					DPS_End_Application_OutCome();

					return SUCCESS;
				}
			}
			else  //Check format error
			{
				//Show TVR and TSI value (for test)
				DBG_Put_String(4, (UCHAR*)"TVR:");
				DBG_Put_Hex(5, glv_tag95.Value);

				DBG_Put_String(4, (UCHAR*)"TSI:");
				DBG_Put_Hex(2, glv_tag9B.Value);

				DPS_End_Application_OutCome();

				return FAIL;
			}
		}

		//Step 6 : No other script command to send
		DPS_DBG_Put_Process(DPS_FIGURE_27, 6, FALSE);
	}

	//Show TVR and TSI value (for test)
	DBG_Put_String(4, (UCHAR*)"TVR:");
	DBG_Put_Hex(5, glv_tag95.Value);

	DBG_Put_String(4, (UCHAR*)"TSI:");
	DBG_Put_Hex(2, glv_tag9B.Value);

	DPS_End_Application_OutCome();

	return SUCCESS;
}

UCHAR DPS_Patch_Chip_Data(UCHAR Tag_Total_Len, UCHAR *tagList)
{
	UINT	i = 0, VLength = 0;
	UCHAR	TLength = 0, LLength = 0, rspCode = 0,*Len_Addr = 0, *Value_Addr = 0;
	ULONG	Index = 0;

	//Cross Test : MS mode should be offline if TTQ B1b8 = 0 
	if(((DPS_Check_ZIP_AID() == SUCCESS) || (MS_Mode == TRUE)) && (glv_tag9F66.Value[0] & DPS_TTQ_MAGNETIC_STRIPE_MODE_SUPPORTED))	//Legacy Mode or MS Mode
	{
		DPS_Online_Request_OutCome();

		//Always go online
		Online_Data[i++]=0xDF;
		Online_Data[i++]=0x0F;
		Online_Data[i++]=0x01;
		Online_Data[i++]=0x02;

		DBG_Put_String(30, (UCHAR*)"Online Transaction for MS Mode");	// === [Debug] ====
	}
	else  //D-PAS EMV Mode
	{
		if(dps_Decline_Required_by_Reader)
		{
			Online_Data[i++]=0xDF;
			Online_Data[i++]=0x1F;
			Online_Data[i++]=0x01;
			Online_Data[i++]=0x01;		//Decline

			DBG_Put_String(15, (UCHAR*)"Offline Decline");	// === [Debug] ====
		}
		else
		{
			if(dps_Online_Required_by_Reader)
			{
				Online_Data[i++]=0xDF;
				Online_Data[i++]=0x0F;
				Online_Data[i++]=0x01;
				Online_Data[i++]=0x01;	//Online

				DBG_Put_String(31, (UCHAR*)"Online Transaction for EMV Mode");	// === [Debug] ====
			}
			else
			{
				Online_Data[i++]=0xDF;
				Online_Data[i++]=0x1F;
				Online_Data[i++]=0x01;
				Online_Data[i++]=0x00;		//Approve

				DBG_Put_String(15, (UCHAR*)"Offline Approve");	// === [Debug] ====
			}
		}
	}

	while(Tag_Total_Len)
	{
		TLength = LLength = VLength = Index = 0;
		UT_Get_TLVLengthOfT(tagList, &TLength);
		rspCode = UT_Search(TLength, tagList, (UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &Index);
		if(rspCode == SUCCESS)
		{
			Len_Addr = (UCHAR *)(glv_addTable[Index]);
			Value_Addr = (UCHAR *)(glv_addTable[Index] + 3);
			
			UT_Get_TLVLengthOfV(Len_Addr, &VLength);
			if(VLength)
			{
				memcpy(&Online_Data[i], tagList, TLength);	//Tag
				i += TLength;

				UT_Get_TLVLengthOfL(Len_Addr, &LLength);
				memcpy(&Online_Data[i], Len_Addr, LLength);	//Length
				i += LLength;

				UT_Get_TLVLengthOfV(Len_Addr, &VLength);
				memcpy(&Online_Data[i], Value_Addr, VLength);	//Value
				i += VLength;
			}

			tagList += TLength;
			Tag_Total_Len -= TLength;
		}
	}

	Online_Data_Length = i;
	
	return SUCCESS;
}

void DPS_Patch_Track_Data(void)
{
	//Reset Whole Data
	D1_Data_Length = 0;
	memset(D1_Data,0x00,79);
	Tag57_Data_Length = 0;
	memset(Tag57_Data,0x00,40);

	//D2 - Track 2
	Tag57_Data_Length = dps_MSD_Track2_Data_Length;
	memcpy(Tag57_Data, dps_MSD_Track2_Data, Tag57_Data_Length);

	//D1 - Track 1
	D1_Data_Length = dps_MSD_Track1_Data_Length;
	memcpy(D1_Data, dps_MSD_Track1_Data, D1_Data_Length);
}

void DPS_OutcomeProcess(void)
{
	//[C-6 Kernel 6 Table 4-8]
	//Approved and Online request(EMV Mode)
	UCHAR	EMV_Data_Element[41] = {0x9F,0x02,0x9F,0x03,0x9F,0x26,0x9F,0x06,0x82,0x5F,0x34,0x9F,0x36,0x9F,0x07,0x5F,0x20,0x9F,0x27,0x84,
								    0x9F,0x10,0x9F,0x09,0x9F,0x33,0x9F,0x1A,0x9F,0x35,0x95,0x9F,0x1F,0x57,0x9A,0x9C,0x9F,0x37,0x9B,0x5F,
								    0x2A};
	
	//Online request MS Mode and Legacy MS Mode
	UCHAR	MS_Data_Element[16] = {0x9F,0x02,0x9F,0x03,0x9F,0x33,0x9F,0x1A,0x9F,0x35,0x56,0x57,0x9A,0x9C,0x95,0x9B};

	//Declined(All Transactions)
	UCHAR	Decline_Data_Element[14] = {0x9F,0x02,0x9F,0x03,0x9F,0x33,0x9F,0x1A,0x9F,0x35,0x9A,0x9C,0x95,0x9B};

	UCHAR	tagList[41] = {0};
	UCHAR	Tag_Total_Len = 0;
	UCHAR	rspCode = 0;

	//Copy Outcome Status for L3 Response
	if (etp_Outcome.ocmStatus == ETP_OCP_UIS_CardReadSuccessfully)	//Approve, Online Request, Decline
	{
//		DPS_Patch_Track_Data();	//patch D1, D2
		VGL_AS210_D1_Track();

		//Patch D3
		if(dps_Decline_Required_by_Reader)	//Decline
		{
			Tag_Total_Len = 14;
			memcpy(tagList, Decline_Data_Element, Tag_Total_Len);
		}
		else
		{
			if(dps_Online_Required_by_Reader)	//Online
			{
				if((DPS_Check_ZIP_AID() == SUCCESS) || (MS_Mode == TRUE))	//Legacy Mode or MS Mode
				{
					Tag_Total_Len = 16;
					memcpy(tagList, MS_Data_Element, Tag_Total_Len);
				}
				else  //EMV Mode
				{
					Tag_Total_Len = 41;
					memcpy(tagList, EMV_Data_Element, Tag_Total_Len);
				}
			}
			else  //Approve
			{
				Tag_Total_Len = 41;
				memcpy(tagList, EMV_Data_Element, Tag_Total_Len);
			}
		}

		rspCode = DPS_Patch_Chip_Data(Tag_Total_Len, tagList);	//patch D3
		if(rspCode == FAIL)
		{
			UT_PutMsg(1,0,FONT0,17,(UCHAR *)"Online Data Error");
			UT_WaitKey();
		}

		L3_Response_Code = VAP_RIF_RC_DATA;
	}
	else
	{
		L3_Response_Code = VAP_RIF_RC_FAILURE;
	}
}
/*
UCHAR DPS_Online_Request_ModeWithTimer(void)
{
	UCHAR	rspCode = 0;
	UINT	time_hnd, get_time, cur_time;
	UINT	timeOut = 5;	//5 seconds
	UCHAR	tmpValue = 0;
	UCHAR	dspMessage0[21] = {0};	//Display Message 0
	UCHAR	dspMessage1[21] = {0};	//Display Message 1

	DBG_Put_String(40, (UCHAR*)"==== Online Request Mode with Timer ====");	// ==== [Debug] ====

	//Start timer
	time_hnd = api_tim_open(100);	// 100 = 1 seconds , 1 unit = 0.01 seconds.
	api_tim_gettick(time_hnd,(UCHAR *)&cur_time);

	if((glv_tag9F66.Value[0] & DPS_TTQ_OFFLINE_ONLY_READER) == 0)	//Reader is enable to process online
	{
		do
		{
			rspCode = api_aux_rxready(vap_rif_dhnAUX, (UCHAR*)&vap_rif_rcvLen);
			if(rspCode == apiOK)
			{
				//Step 4
				api_aux_rxstring(vap_rif_dhnAUX, vap_rif_rcvBuffer);
				api_tim_close(time_hnd);
				//Step 5
				if(vap_rif_rcvBuffer[2] == VAP_RIF_INSTRUCTION_TRA_ShowStatus)
				{
//					VAP_RIF_TRA_ShowStatus();
					return SUCCESS;
				}
				else if(vap_rif_rcvBuffer[2] == VAP_RIF_INSTRUCTION_TRA_Issuer_Update)
				{
//					VAP_RIF_TRA_Issuer_Update();
					return SUCCESS;
				}
				else
				{
					UT_PutStr(7, 0, FONT0, 13, (UCHAR*)"Command Error");
					return FAIL;
				}
			}

			api_tim_gettick(time_hnd,(UCHAR *)&get_time);

		} while((get_time - cur_time) < timeOut);	//Step 1
	}

	//Time expired
	api_tim_close(time_hnd);

	DPS_DBG_Put_Process(DPS_FIGURE_25, 1, TRUE);

	//Step 2
	if(glv_tag9F71.Value[1] & DPS_CPR_SWITCH_OTHER_INTERFACE_IF_UNABLE_TO_PROCESS_ONLINE)
	{
		DPS_DBG_Put_Process(DPS_FIGURE_25, 2, TRUE);

		//Step 3
		tmpValue = glv_tag9F66.Value[0];
		tmpValue &= 0x90;	//TTQ B1b8 : Magnetic stripe mode supported, B1b5 : EMV contact chip supported
		if(tmpValue)	//Ask user to insert or swipe the card
		{
			DPS_DBG_Put_Process(DPS_FIGURE_25, 3, TRUE);

			UT_LED_Switch(IID_LED_RED,2);
			UT_Buz_Option(FALSE);

			memcpy(dspMessage0, (UCHAR*)"  Please insert or   ",	21);
			memcpy(dspMessage1, (UCHAR*)"     swipe card      ",	21);
			UT_PutStr(1, 0, FONT1, 21, dspMessage0);
			UT_PutStr(2, 0, FONT1, 21, dspMessage1);

			UT_WaitTime(200);
		}
		else  //Decline Transaction
		{
			DPS_DBG_Put_Process(DPS_FIGURE_25, 3, FALSE);

			UT_LED_Switch(IID_LED_RED,2);
			UT_Buz_Option(FALSE);

			memcpy(dspMessage0, (UCHAR*)"                     ",	21);
			memcpy(dspMessage1, (UCHAR*)"      DECLINE        ",	21);
			UT_PutStr(1, 0, FONT1, 21, dspMessage0);
			UT_PutStr(2, 0, FONT1, 21, dspMessage1);

			UT_WaitTime(200);
		}
	}
	else  //Decline Transaction
	{
		DPS_DBG_Put_Process(DPS_FIGURE_25, 2, FALSE);

		UT_LED_Switch(IID_LED_RED,2);
		UT_Buz_Option(FALSE);

		memcpy(dspMessage0, (UCHAR*)"                     ",	21);
		memcpy(dspMessage1, (UCHAR*)"      DECLINE        ",	21);
		UT_PutStr(1, 0, FONT1, 21, dspMessage0);
		UT_PutStr(2, 0, FONT1, 21, dspMessage1);

		UT_WaitTime(200);
	}

	return SUCCESS;
}
*/
void DPS_TAA_Process_Part2(void)
{
	UCHAR	tmpValue = 0;

	DBG_Put_String(36, (UCHAR*)"==== Terminal Action Analysis 2 ====");	// ==== [Debug] ====

	//Step 1
	if(glv_tag9F66.Value[1] & DPS_TTQ_ONLINE_CRYPTOGRAM_REQUIRED)
	{
		DPS_DBG_Put_Process(DPS_FIGURE_24, 1, TRUE);

		//Step 2
		tmpValue = glv_tag9F27.Value[0];
		tmpValue &= 0xC0;
		if(tmpValue == 0x40)	//Transaction Certificate (TC)
		{
			DPS_DBG_Put_Process(DPS_FIGURE_24, 2, TRUE);

			//Decline Transaction
			dps_Decline_Required_by_Reader = 1;
			DPS_Declined_OutCome();
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_24, 2, FALSE);

			//Process online authorization
			dps_Online_Required_by_Reader = 1;
			if(dps_flgIssuerUpdate == TRUE)
			{
				DPS_Online_Request_For_Two_Presentments();
			}
			else
			{
				DPS_Online_Request_OutCome();
			}
		}
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_24, 1, FALSE);

		//Step 3
		tmpValue = glv_tag9F27.Value[0];
		tmpValue &= 0xC0;
		if(tmpValue == 0x80)	//Authorisation Request Cryptogram (ARQC)
		{
			DPS_DBG_Put_Process(DPS_FIGURE_24, 3, TRUE);

			//Process online authorization
			dps_Online_Required_by_Reader = 1;
			if(dps_flgIssuerUpdate == TRUE)
			{
				DPS_Online_Request_For_Two_Presentments();
			}
			else
			{
				DPS_Online_Request_OutCome();
			}
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_24, 3, FALSE);

			//Step 4
			if(glv_tag95.Value[0] & DPS_TVR_CDA_FAILED)
			{
				DPS_DBG_Put_Process(DPS_FIGURE_24, 4, TRUE);

				//Decline Transaction
				dps_Decline_Required_by_Reader = 1;
			}
			else
			{
				DPS_DBG_Put_Process(DPS_FIGURE_24, 4, FALSE);

				//Approve offline authorization
				dps_Decline_Required_by_Reader = 0;
				dps_Online_Required_by_Reader = 0;
				DPS_Approved_OutCome();
			}
		}
	}
}

UCHAR DPS_TAA_Process_Part1(void)
{
	UCHAR	tmpValue = 0;
	UCHAR	flowControl = 0;

	DBG_Put_String(36, (UCHAR*)"==== Terminal Action Analysis 1 ====");	// ==== [Debug] ====

	//[C-6 Kernel 6 : 3.8 Online Processing]
	//Check if reader and card are support Issuer Update Processing
	if((glv_tag9F66.Value[2] & DPS_TTQ_ISSUER_UPDATE_PROCESSING_SUPPORTED) &&
	   (glv_tag9F71.Value[1] & DPS_CPR_ISSUER_UPDATE_PROCESSING_SUPPORTED))
	{
		dps_flgIssuerUpdate = TRUE;
	}

	//Step 1
	tmpValue = glv_tag9F27.Value[0];
	tmpValue &= 0xC0;
	if(tmpValue == 0x00)	//Application Authentication Cryptogram (AAC)
	{
		DPS_DBG_Put_Process(DPS_FIGURE_23, 1, TRUE);

		//Step 2
		if((glv_tag9F71.Value[0] & DPS_CPR_PID_LIMIT_REACHED_LOYALTY_TRANSACTION_APPROVED) != 0)
		{
			DPS_DBG_Put_Process(DPS_FIGURE_23, 2, TRUE);

			//Approve transaction
			dps_Decline_Required_by_Reader = 0;
			dps_Online_Required_by_Reader = 0;
			DPS_Approved_OutCome();
			return SUCCESS;
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_23, 2, FALSE);

			flowControl = 'B';
		}
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_23, 1, FALSE);

		//Step 3
		if(glv_tag95.Value[0] & DPS_TVR_CDA_FAILED)
		{
			DPS_DBG_Put_Process(DPS_FIGURE_23, 3, TRUE);

			//Step 4
			if(glv_tag9F71.Value[1] & DPS_CPR_DECLINE_SWITCH_OTHER_INTERFACE_IF_CDA_FAILED)
			{
				DPS_DBG_Put_Process(DPS_FIGURE_23, 4, TRUE);

				flowControl = 'A';
			}
			else
			{
				DPS_DBG_Put_Process(DPS_FIGURE_23, 4, FALSE);

				//Step 6
				if(glv_tag9F71.Value[1] & DPS_CPR_PROCESS_ONLINE_IF_CDA_FAILED)
				{
					DPS_DBG_Put_Process(DPS_FIGURE_23, 6, TRUE);

					flowControl = 'D';
				}
				else
				{
					DPS_DBG_Put_Process(DPS_FIGURE_23, 6, FALSE);

					flowControl = 'C';
				}
			}
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_23, 3, FALSE);

			flowControl = 'C';
		}
	}
		if(flowControl == 'C')
		{
			//Step 8a : Update 1c in [Terminal Application Specification Bulletin CL TAS-002, v1.0]
			if((glv_tag95.Value[1] & DPS_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT) == 0)
			{
				DPS_DBG_Put_Process(DPS_FIGURE_23, 0x8A, FALSE);

				//Step 8b : Update 1c in [Terminal Application Specification Bulletin CL TAS-002, v1.0]
				if((glv_tag95.Value[0] & DPS_TVR_CARD_APPEARS_ON_TERMINAL_EXCEPTION_FILE) == 0)
				{
					DPS_DBG_Put_Process(DPS_FIGURE_23, 0x8B, FALSE);
				
					//Step 8c : Update 1c in [Terminal Application Specification Bulletin CL TAS-002, v1.0]
					if((glv_tag95.Value[0] & DPS_TVR_ICC_DATA_MISSING) == 0)
					{
						DPS_DBG_Put_Process(DPS_FIGURE_23, 0x8C, FALSE);

						//Step 9
						if(glv_tag95.Value[1] & DPS_TVR_EXPIRED_APPLICATION)
						{
							DPS_DBG_Put_Process(DPS_FIGURE_23, 9, TRUE);

							//Step 10
							if(glv_tag9F71.Value[1] & DPS_CPR_DECLINE_IF_CARD_EXPIRED)
							{
								DPS_DBG_Put_Process(DPS_FIGURE_23, 10, TRUE);

								flowControl = 'B';
							}
							else
							{
								DPS_DBG_Put_Process(DPS_FIGURE_23, 10, FALSE);

								//Step 11
								if(glv_tag9F71.Value[1] & DPS_CPR_PROCESS_ONLINE_IF_CARD_EXPIRED)
								{
									DPS_DBG_Put_Process(DPS_FIGURE_23, 11, TRUE);

									flowControl = 'D';
								}
								else
								{
									DPS_DBG_Put_Process(DPS_FIGURE_23, 11, FALSE);

									//Step 12 : Update 1c in [Terminal Application Specification Bulletin CL TAS-002, v1.0]
									if(glv_tag95.Value[1] & DPS_TVR_APPLICATION_NOT_YET_EFFECTIVE)
									{
										DPS_DBG_Put_Process(DPS_FIGURE_23, 12, TRUE);

										flowControl = 'D';
									}
									else
									{
										DPS_DBG_Put_Process(DPS_FIGURE_23, 12, FALSE);

										//Validate Choice
										DPS_TAA_Process_Part2();
										return SUCCESS;
									}
								}
							}
						}
						else
						{
							DPS_DBG_Put_Process(DPS_FIGURE_23, 9, FALSE);

							//Step 12 : Update 1c in [Terminal Application Specification Bulletin CL TAS-002, v1.0]
							if(glv_tag95.Value[1] & DPS_TVR_APPLICATION_NOT_YET_EFFECTIVE)
							{
								DPS_DBG_Put_Process(DPS_FIGURE_23, 12, TRUE);

								flowControl = 'D';
							}
							else
							{
								DPS_DBG_Put_Process(DPS_FIGURE_23, 12, FALSE);

								//Validate Choice
								DPS_TAA_Process_Part2();
								return SUCCESS;
							}
						}	
					}
					else
					{
						DPS_DBG_Put_Process(DPS_FIGURE_23, 0x8C, TRUE);

						flowControl = 'B';
					}
				}
				else
				{
					DPS_DBG_Put_Process(DPS_FIGURE_23, 0x8B, TRUE);

					flowControl = 'B';
				}
			}
			else
			{
				DPS_DBG_Put_Process(DPS_FIGURE_23, 0x8A, TRUE);

				flowControl = 'B';
			}
		}
		
		if(flowControl == 'D')
		{
			//Step 7
			if((glv_tag9F66.Value[0] & DPS_TTQ_OFFLINE_ONLY_READER) == 0)
			{
				DPS_DBG_Put_Process(DPS_FIGURE_23, 7, TRUE);

				//Process online authorization
				dps_Online_Required_by_Reader = 1;
				if(dps_flgIssuerUpdate == TRUE)
				{
					DPS_Online_Request_For_Two_Presentments();
				}
				else
				{
					DPS_Online_Request_OutCome();
				}
				return SUCCESS;
			}
			else
			{
				DPS_DBG_Put_Process(DPS_FIGURE_23, 7, FALSE);

				//Step 8
				if(glv_tag9F71.Value[1] & DPS_CPR_SWITCH_OTHER_INTERFACE_IF_UNABLE_TO_PROCESS_ONLINE)
				{
					DPS_DBG_Put_Process(DPS_FIGURE_23, 8, TRUE);

					flowControl = 'A';
				}
				else
				{
					DPS_DBG_Put_Process(DPS_FIGURE_23, 8, FALSE);

					flowControl = 'B';
				}
			}
		}

		if(flowControl == 'A')
		{
			//Step 5
			tmpValue = glv_tag9F66.Value[0];
			tmpValue &= 0x90;	//TTQ B1b8 : Magnetic stripe mode supported, B1b5 : EMV contact chip supported
			if(tmpValue)
			{
				DPS_DBG_Put_Process(DPS_FIGURE_23, 5, TRUE);

				DPS_Try_Another_Interface_OutCome();	//Ask user to insert or swipe the card
				return FAIL;
			}
			else
			{
				DPS_DBG_Put_Process(DPS_FIGURE_23, 5, FALSE);

				//Decline Transaction
				dps_Decline_Required_by_Reader = 1;
				DPS_Declined_OutCome();
				return SUCCESS;
			}
		}

		if(flowControl == 'B')
		{
			//Decline Transaction
			dps_Decline_Required_by_Reader = 1;
			DPS_Declined_OutCome();
			return SUCCESS;
		}

		return SUCCESS;
}

UCHAR DPS_Compare_Date(UCHAR *iptDate1, UCHAR *iptDate2)
{
	UINT	Year1 = 0;
	UINT	Year2 = 0;
	UCHAR	Month1 = 0;
	UCHAR	Month2 = 0;
	UCHAR	Day1 = 0;
	UCHAR	Day2 = 0;
	UCHAR	tmpCentury = 0;

	//Compare Year
	tmpCentury = UT_SetCentury(iptDate1[0]);
	Year1 = tmpCentury*256 + iptDate1[0];
	
	tmpCentury = UT_SetCentury(iptDate2[0]);
	Year2 = tmpCentury*256 + iptDate2[0];

	if (Year1 > Year2)
	{
		return 1;
	}
	else if (Year1 < Year2)
	{
		return 2;
	}
	else
	{
		//Compare Month
		Month1 = iptDate1[1];
		Month2 = iptDate2[1];
		
		if (Month1 > Month2)
		{
			return 1;
		}
		else if (Month1 < Month2)
		{
			return 2;
		}
		else
		{
			//Compare Day
			Day1 = iptDate1[2];
			Day2 = iptDate2[2];

			if (Day1 > Day2)
			{
				return 1;
			}
			else if (Day1 < Day2)
			{
				return 2;
			}
		}
	}

	//Date1 & Date2 are the Same
	return 0;
}

UCHAR DPS_Processing_Restriction(void)
{
	UINT	Tmp_Len = 0, Tmp_Len2 = 0;
	UCHAR	emptySet[10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	UCHAR	ExFileIndex = 0;
	UCHAR	flagEmpty = TRUE;
	UCHAR	i = 0;

	DBG_Put_String(33, (UCHAR*)"==== Processing Restrictions ====");	// ==== [Debug] ====

	//Step 1 : Set TSI B1b4 to ��1��
	glv_tag9B.Value[0] |= DPS_TSI_TERMINAL_RISK_MANAGEMENT_WAS_PERFORMED;

	DPS_DBG_Put_Process(DPS_FIGURE_22, 1, 0xFF);

	//Update 2c in [Terminal Application Specification Bulletin CL TAS-002, v1.0]
	//Terminal shall check if the Application Usage Control (AUC) and Issuer Country Code are present.
	if((glv_tag9F07.Length[0] != 0) && (glv_tag5F28.Length[0] != 0))
	{
		//If yes, it shall perform the usage control checks as described in [EMV 4.2-3] to determine if the service is allowed.
		if((glv_tag9F07.Length[0] != 2) || (glv_tag5F28.Length[0] != 2))	//[Test Case : Non_CDA_checks_048]
		{
			glv_tag95.Value[0] |= DPS_TVR_ICC_DATA_MISSING;

			//Terminate Transaction
			return FAIL;
		}
	}
	else
	{
		//Otherwise it shall skip the usage control checks and go to step 4.
		goto STEP_4;
	}

	//Step 2 : Application Usage Control (AUC) Check
	//[EMV Book 3 10.4.2]

	//The transaction is being conducted at an ATM
	if((glv_tag9F35.Value[0] == 0x14) || (glv_tag9F35.Value[0] == 0x15) || (glv_tag9F35.Value[0] == 0x16))
	{
		if((glv_tag9F07.Value[0] & DPS_AUC_VALID_AT_ATMS) == 0)
		{
			glv_tag95.Value[1] |= DPS_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;	//Step 3

			DPS_DBG_Put_Process(DPS_FIGURE_22, 2, FALSE);
			DPS_DBG_Put_Process(DPS_FIGURE_22, 3, 0xFF);
		}
	}
	else  //The transaction is not being conducted at an ATM
	{
		if((glv_tag9F07.Value[0] & DPS_AUC_VALID_AT_TERMINALS_OTHER_THAN_ATMS) == 0)
		{
			glv_tag95.Value[1] |= DPS_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;	//Step 3

			DPS_DBG_Put_Process(DPS_FIGURE_22, 2, FALSE);
			DPS_DBG_Put_Process(DPS_FIGURE_22, 3, 0xFF);
		}
	}

	/*
	Tag 9F10 - Issuer Application Data (IAD) :
	Derivation Key Index (DKI)							(1)
	Cryptogram Version Number (CVN)						(1)
	Card Verification Results (CVR)						(8)
	Issuer Discretionary Data items listed in IADOL		(n)
	*/

	/*
	Tag 9F53 - Contactless Card Verification Results (CL CVR)
	B2b5 : Domestic Transaction (based on Contactless-ACO setting)
	B2b4 : International Transaction
	*/

	//Check domestic transaction and international transaction by CVR in IAD
	if(glv_tag9C.Value[0] == DPS_TXT_Cash)
	{
		if((glv_tag9F10.Value[3] & 0x10) != 0)	//Domestic Transaction
		{
			if((glv_tag9F07.Value[0] & DPS_AUC_VALID_FOR_DOMESTIC_CASH_TRANSACTIONS) == 0)
			{
				glv_tag95.Value[1] |= DPS_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;	//Step 3

				DPS_DBG_Put_Process(DPS_FIGURE_22, 2, FALSE);
				DPS_DBG_Put_Process(DPS_FIGURE_22, 3, 0xFF);
			}
		}
		else if((glv_tag9F10.Value[3] & 0x08) != 0)	//International Transaction
		{
			if((glv_tag9F07.Value[0] & DPS_AUC_VALID_FOR_INTERNATIONAL_CASH_TRANSACTIONS) == 0)
			{
				glv_tag95.Value[1] |= DPS_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;	//Step 3

				DPS_DBG_Put_Process(DPS_FIGURE_22, 2, FALSE);
				DPS_DBG_Put_Process(DPS_FIGURE_22, 3, 0xFF);
			}
		}
	}
	else if(glv_tag9C.Value[0] == DPS_TXT_Purchase)
	{
		if((glv_tag9F10.Value[3] & 0x10) != 0)	//Domestic Transaction
		{
			if(((glv_tag9F07.Value[0] & DPS_AUC_VALID_FOR_DOMESTIC_GOODS) == 0) || 
			   (((glv_tag9F07.Value[0] & DPS_AUC_VALID_FOR_DOMESTIC_GOODS) == 0) && ((glv_tag9F07.Value[0] & DPS_AUC_VALID_FOR_DOMESTIC_SERVICES) == 0)))
			{
				glv_tag95.Value[1] |= DPS_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;	//Step 3

				DPS_DBG_Put_Process(DPS_FIGURE_22, 2, FALSE);
				DPS_DBG_Put_Process(DPS_FIGURE_22, 3, 0xFF);
			}
		}
		else if((glv_tag9F10.Value[3] & 0x08) != 0)	//International Transaction
		{
			if(((glv_tag9F07.Value[0] & DPS_AUC_VALID_FOR_INTERNATIONAL_GOODS) == 0) || 
			   (((glv_tag9F07.Value[0] & DPS_AUC_VALID_FOR_INTERNATIONAL_GOODS) == 0) && ((glv_tag9F07.Value[0] & DPS_AUC_VALID_FOR_INTERNATIONAL_SERVICES) == 0)))
			{
				glv_tag95.Value[1] |= DPS_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;	//Step 3

				DPS_DBG_Put_Process(DPS_FIGURE_22, 2, FALSE);
				DPS_DBG_Put_Process(DPS_FIGURE_22, 3, 0xFF);
			}
		}
	}
	else if(glv_tag9C.Value[0] == DPS_TXT_CashBack)
	{
		if((glv_tag9F10.Value[3] & 0x10) != 0)	//Domestic Transaction
		{
			if((glv_tag9F07.Value[1] & DPS_AUC_DOMESTIC_CASHBACK_ALLOWED) == 0)
			{
				glv_tag95.Value[1] |= DPS_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;	//Step 3

				DPS_DBG_Put_Process(DPS_FIGURE_22, 2, FALSE);
				DPS_DBG_Put_Process(DPS_FIGURE_22, 3, 0xFF);
			}
		}
		else if((glv_tag9F10.Value[3] & 0x08) != 0)	//International Transaction
		{
			if((glv_tag9F07.Value[1] & DPS_AUC_INTERNATIONAL_CASHBACK_ALLOWED) == 0)
			{
				glv_tag95.Value[1] |= DPS_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;	//Step 3

				DPS_DBG_Put_Process(DPS_FIGURE_22, 2, FALSE);
				DPS_DBG_Put_Process(DPS_FIGURE_22, 3, 0xFF);
			}
		}
	}

	if((glv_tag95.Value[1] & DPS_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT) == 0)
	{
		DPS_DBG_Put_Process(DPS_FIGURE_22, 2, TRUE);
	}

STEP_4:
	//Step 4 : Application Expired Check
	UT_Get_TLVLengthOfV(glv_tag5F24.Length, &Tmp_Len);
	if(Tmp_Len != 0)
	{
		if(DPS_Compare_Date(glv_tag5F24.Value, glv_tag9A.Value) == 2)	//Application Expired Date < Transaction Date
		{
			DPS_DBG_Put_Process(DPS_FIGURE_22, 4, TRUE);

			glv_tag95.Value[1] |= DPS_TVR_EXPIRED_APPLICATION;	//Step 5

			DPS_DBG_Put_Process(DPS_FIGURE_22, 5, 0xFF);
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_22, 4, FALSE);
		}
	}

	//Step 6 : Check if terminal embeds an exception file that contains a list of PAN(s)
	for(ExFileIndex = 0 ; ExFileIndex < 10 ; ExFileIndex++)
	{
		if(memcmp(Exception_File[ExFileIndex], emptySet, 10))
		{
			flagEmpty = FALSE;
		}
	}

	if(flagEmpty == FALSE)	//exception file present
	{
		DPS_DBG_Put_Process(DPS_FIGURE_22, 6, TRUE);

		//Step 7 : Verify if the PAN returned by the card is found inside the exception file
		UT_Get_TLVLengthOfV(glv_tag5A.Length, &Tmp_Len);

		if(Tmp_Len != 0)
		{
			for(i = 0 ; i < 10 ; i++)
			{
				if(UT_memcmp(&Exception_File[i][0], glv_tag5A.Value, Tmp_Len) == 0)	//matched
				{
					DPS_DBG_Put_Process(DPS_FIGURE_22, 7, TRUE);

					glv_tag95.Value[0] |= DPS_TVR_CARD_APPEARS_ON_TERMINAL_EXCEPTION_FILE;	//Step 8

					DPS_DBG_Put_Process(DPS_FIGURE_22, 8, 0xFF);

					break;
				}
			}

			if((glv_tag95.Value[0] & DPS_TVR_CARD_APPEARS_ON_TERMINAL_EXCEPTION_FILE) == 0)
			{
				DPS_DBG_Put_Process(DPS_FIGURE_22, 7, FALSE);
			}
		}
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_22, 6, FALSE);
	}

	//Step 9 : Check if the card's Application Effective Date and Application Version Number are available
	UT_Get_TLVLengthOfV(glv_tag5F25.Length, &Tmp_Len);
	UT_Get_TLVLengthOfV(glv_tag9F08.Length, &Tmp_Len2);

	if((Tmp_Len != 0) && (Tmp_Len2 != 0))
	{
		DPS_DBG_Put_Process(DPS_FIGURE_22, 9, TRUE);

		//Step 10 : Application Version Number Check
		if(memcmp(glv_tag9F08.Value, glv_tag9F09.Value, 2))
		{
			DPS_DBG_Put_Process(DPS_FIGURE_22, 10, FALSE);

			glv_tag95.Value[1] |= DPS_TVR_ICC_AND_TERMINAL_HAVE_DIFFERENT_APPLICATION_VERSIONS;	//Step 11

			DPS_DBG_Put_Process(DPS_FIGURE_22, 11, 0xFF);
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_22, 10, TRUE);
		}

		//Step 12 : Application Effective Date Check
		if(DPS_Compare_Date(glv_tag5F25.Value, glv_tag9A.Value) == 1)	//Application Effective Date > Transaction Date
		{
			DPS_DBG_Put_Process(DPS_FIGURE_22, 12, TRUE);

			glv_tag95.Value[1] |= DPS_TVR_APPLICATION_NOT_YET_EFFECTIVE;	//Step 13

			DPS_DBG_Put_Process(DPS_FIGURE_22, 13, 0xFF);
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_22, 12, FALSE);
		}
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_22, 9, FALSE);
	}

	//Show TVR and TSI value (for test)
	DBG_Put_String(4, (UCHAR*)"TVR:");
	DBG_Put_Hex(5, glv_tag95.Value);

	DBG_Put_String(4, (UCHAR*)"TSI:");
	DBG_Put_Hex(2, glv_tag9B.Value);

	return SUCCESS;
}

void DPS_Check_api_rsa_recover(UCHAR iptLen, UCHAR *recData)
{
	UCHAR	idxNum = 0;
	UCHAR	sftBytes = 0;
	UCHAR	*ptrData = NULLPTR;
	UCHAR	recBuffer[256] = {0};
	
	ptrData = recData;

	//Calculate Shift Bytes
	for (idxNum = 0 ; idxNum < 3 ; idxNum++)
	{
		if (ptrData[0] == 0x00)
		{
			sftBytes++;
		}

		ptrData++;
	}

	//Shift Data
	memcpy(recBuffer, &recData[sftBytes], iptLen);
	memcpy(recData, recBuffer, iptLen);
}

UCHAR DPS_Check_CRL(UCHAR *iptRID, UCHAR iptIndex, UCHAR *iptSerNumber)
{
	UCHAR	idxNum = 0;
	UCHAR	endOfList[9] = {0};

	for (idxNum = 0 ; idxNum < CRL_NUMBER ; idxNum++)
	{
		if (!memcmp(glv_CRL[idxNum].RID, endOfList, 9))
		{
			break;
		}
		else
		{
			if (!memcmp(glv_CRL[idxNum].RID, iptRID, 5))
			{
				if (glv_CRL[idxNum].Index == iptIndex)
				{
					if (!memcmp(glv_CRL[idxNum].serNumber, iptSerNumber, 3))
					{
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

UCHAR DPS_Retrieve_PK_CA(UCHAR *iptRID, UCHAR iptIndex, UCHAR *optModLen, UCHAR	*optModulus, UCHAR *optExponent)
{
	UCHAR	idxNum = 0;


	//Using this index and the RID, the terminal can identify and retrieve the terminal-stored Certification Authority Public Key Modulus and 
	//Exponent and associated key-related information, and the corresponding algorithm to be used.
	//[Book 2 5.2/6.2]
	for (idxNum = 0; idxNum < CAPK_NUMBER; idxNum++)
	{
		if (!memcmp(glv_CAPK[idxNum].RID, iptRID, 5))
		{
			if (glv_CAPK[idxNum].Index == iptIndex)
			{
				optModLen[0] = glv_CAPK[idxNum].Length;
				memcpy(optModulus, glv_CAPK[idxNum].Modulus, glv_CAPK[idxNum].Length);
				memcpy(optExponent, glv_CAPK[idxNum].Exponent, 3);
				
				return SUCCESS;
			}
		}
	}

	return FAIL;
}

UCHAR DPS_Retrieve_PK_Issuer(UCHAR iptModLen, UCHAR	*iptModulus, UCHAR *iptExponent, UCHAR *optModLen, UCHAR *optModulus, UCHAR *optExponent)
{
	UCHAR	rspCode = FALSE;
	UINT	lenOfV = 0;
	UINT	lenOfIPKC = 0;				//Length of Issuer Public Key Certificate
	UINT	lenOfIPKR = 0;				//Length of Issuer Public Key Remainder
	UINT	lenOfIPKL = 0;				//Length of Issuer Public Key Length
	UINT	lenOfIPKE = 0;				//Length of Issuer Public Key Exponent
	UINT	lenOfLMD = 0;					//Length of Leftmost Digits of the Issuer Public Key
	UINT	lenOfHash = 0;				//Length of Hash Data
	UCHAR	bufCertificate[2+256] = {0};	//Buffer of Certificate [Len(2)+Data(256)]
	UCHAR	bufHash[512] = {0};			//Buffer of Hash
	UCHAR	bufModulus[2+256] = {0};		//Buffer of Modulus
	UCHAR	rstHash[20] = {0};			//Result of Hash


	//[Book 2 5.3/6.3]
	//Step 1: Check Issuer Public Key Certificate Length = CAPK Modulus Length
	UT_Get_TLVLengthOfV(glv_tag90.Length, &lenOfV);
	lenOfIPKC = lenOfV;

	if (lenOfIPKC == 0)
	{
		glv_tag95.Value[0] |= DPS_TVR_ICC_DATA_MISSING;
	}
	
	if (iptModLen != lenOfIPKC)
	{
		return FAIL;
	}

	//Step 2: Recover Certificate and Check Trailer = 0xBC
	bufModulus[0] = iptModLen;
	memcpy(&bufModulus[2], iptModulus, iptModLen);

	rspCode = api_rsa_loadkey(bufModulus, iptExponent);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	UT_S2C(lenOfIPKC, &bufCertificate[0]);
	memcpy(&bufCertificate[2], glv_tag90.Value, lenOfIPKC);

	rspCode = api_rsa_recover(bufCertificate, bufCertificate);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	DPS_Check_api_rsa_recover(bufCertificate[0], &bufCertificate[2]);

	if (bufCertificate[2+lenOfIPKC-1] != 0xBC)	//Decrease 1 to Point to Trailer
	{
		return FAIL;
	}

	//Step 3: Check Header = 0x6A
	if (bufCertificate[2+0] != 0x6A)
	{
		return FAIL;
	}

	//Step 4: Check Certificate Format = 0x02
	if (bufCertificate[2+1] != 0x02)	//Add 1 to Point to Certificate Format
	{
		return FAIL;
	}

	//Step 5: Concatenation
	lenOfHash = lenOfIPKC-(1+20+1);						//Header(1)+Hash Result(20)+Trailer(1)
	memcpy(bufHash, &bufCertificate[2+1], lenOfHash);	//Add 1 to Point to Certificate Format

	lenOfIPKL = bufCertificate[2+13];
	lenOfLMD = lenOfIPKC-36;

	UT_Get_TLVLengthOfV(glv_tag92.Length, &lenOfV);
	lenOfIPKR = lenOfV;

	if (lenOfIPKR != 0)
	{
		memcpy(&bufHash[lenOfHash], glv_tag92.Value, lenOfIPKR);
		lenOfHash += lenOfIPKR;
	}
	else
	{
		if (lenOfIPKL > lenOfLMD)
		{
			glv_tag95.Value[0] |= DPS_TVR_ICC_DATA_MISSING;
			return FAIL;
		}
	}

	UT_Get_TLVLengthOfV(glv_tag9F32.Length, &lenOfV);
	lenOfIPKE = lenOfV;

	if (lenOfIPKE == 0)
	{
		glv_tag95.Value[0] |= DPS_TVR_ICC_DATA_MISSING;
		return FAIL;
	}

	memcpy(&bufHash[lenOfHash], glv_tag9F32.Value, lenOfIPKE);
	lenOfHash += lenOfIPKE;

	//Step 6: Apply Hash Algorithm
	rspCode = api_sys_SHA1(lenOfHash, bufHash, rstHash);
	if (rspCode == apiFailed)
	{
		return FAIL;
	}

	//Step 7: Compare Hash Result
	rspCode = UT_bcdcmp(&bufCertificate[2+lenOfIPKC-(20+1)], rstHash, 20); //Hash Result(20)+Trailer(1)
	if (rspCode != 0)
	{
		return FAIL;
	}

	//Step 8: Verify the Issuer Identifier = PAN
	rspCode = UT_CNcmp(&bufCertificate[2+2], glv_tag5A.Value, 4);	//Add 2 to Point to Issuer Identifier
	if (rspCode == FALSE)
	{
		return FAIL;
	}

	//Step 9: Verify Certificate Expiration Date
	rspCode = UT_VerifyCertificateExpDate(&bufCertificate[2+6]);	//Add 6 to Point to Certificate Expiration Date
	if (rspCode == FALSE)
	{
		return FAIL;
	}

	//Step 10: Verify Certification Revocation List
	rspCode = DPS_Check_CRL(glv_tag9F06.Value, glv_tag8F.Value[0], &bufCertificate[2+8]);	//Add 8 to Point to Certificate Serial Number
	if (rspCode == TRUE)
	{
		return FAIL;
	}

	//Step 11: Check Issuer Public Key Algorithm Indicator = 0x01
	if (bufCertificate[2+12] != 0x01)	//Add 12 to Point to Issuer Public Key Algorithm Indicator
	{
		return FAIL;
	}

	//Step 12: Concatenate Issuer Public Key Modulus
	memcpy(optModulus, &bufCertificate[2+15], lenOfLMD);	//Add 15 to Point to Issuer Public Key or Leftmost Digits of the Issuer Public Key

	if (lenOfIPKR != 0)
	{
		memcpy(&optModulus[lenOfLMD], glv_tag92.Value, lenOfIPKR);
	}
		
	optModLen[0] = lenOfIPKL;

	if (lenOfIPKE == 1)
	{
		memset(optExponent, 0, 3);
		optExponent[2] = glv_tag9F32.Value[0];
	}
	else if (lenOfIPKE == 3)
	{
		memcpy(optExponent, glv_tag9F32.Value, 3);
	}
	
	return SUCCESS;
}

UCHAR DPS_Retrieve_PK_ICC(UCHAR	iptModLen, UCHAR *iptModulus, UCHAR	*iptExponent, UCHAR	*optModLen, UCHAR *optModulus, UCHAR *optExponent)
{
	UCHAR	rspCode = FALSE;
	UINT	lenOfV = 0;
	UINT	lenOfIPKC = 0;				//Length of ICC Public Key Certificate
	UINT	lenOfIPKR = 0;				//Length of ICC Public Key Remainder
	UINT	lenOfIPKL = 0;				//Length of ICC Public Key Length
	UINT	lenOfIPKE = 0;				//Length of ICC Public Key Exponent
	UINT	lenOfLMD = 0;					//Length of Leftmost Digits of the ICC Public Key
	UINT	lenOfHash = 0;				//Length of Hash Data
	UINT	lenOfSDTBA = 0;				//Length of Static Data To Be Authenticate
	UCHAR	bufCertificate[2+256] = {0};	//Buffer of Certificate [Len(2)+Data(256)]
	UCHAR	bufHash[512+2048] = {0};		//Buffer of Hash (2048 for SDTBA)			//Hash Buffer
	UCHAR	bufModulus[2+256] = {0};		//Buffer of Modulus
	UCHAR	bufPAN[10] = {0};				//Buffer of PAN
	UCHAR	rstHash[20] = {0};			//Result of Hash


	//[Book 2 6.4]
	//Step 1: Check ICC Public Key Certificate Length = Issuer Public Key Modulus Length
	UT_Get_TLVLengthOfV(glv_tag9F46.Length, &lenOfV);
	lenOfIPKC = lenOfV;

	if (lenOfIPKC == 0)
	{
		glv_tag95.Value[0] |= DPS_TVR_ICC_DATA_MISSING;
	}

	if (lenOfIPKC != iptModLen)
	{
		return FAIL;
	}

	//Step 2: Recover Certificate and Check Trailer = 0xBC
	bufModulus[0] = iptModLen;
	memcpy(&bufModulus[2], iptModulus, iptModLen);

	rspCode = api_rsa_loadkey(bufModulus, iptExponent);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	bufCertificate[0] = lenOfIPKC & 0x00FF;
	bufCertificate[1] = (lenOfIPKC & 0xFF00) >> 8;
	memcpy(&bufCertificate[2], glv_tag9F46.Value, lenOfIPKC);

	rspCode = api_rsa_recover(bufCertificate, bufCertificate);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	DPS_Check_api_rsa_recover(bufCertificate[0], &bufCertificate[2]);

	if (bufCertificate[2+lenOfIPKC-1] != 0xBC)	//Decrease 1 to Point to Trailer
	{
		return FAIL;
	}

	//Step 3: Check Header = 0x6A
	if (bufCertificate[2+0] != 0x6A)
	{
		return FAIL;
	}

	//Step 4: Check Certificate Format = 0x04
	if (bufCertificate[2+1] != 0x04)	//Add 1 to Point to Certificate Format
	{
		return FAIL;
	}

	//Step 5: Concatenation
	lenOfHash = lenOfIPKC-(1+20+1);						//Header(1)+Hash Result(20)+Trailer(1)
	memcpy(bufHash, &bufCertificate[2+1], lenOfHash);	//Add 1 to Point to Certificate Format

	lenOfIPKL = bufCertificate[2+19];
	lenOfLMD = lenOfIPKC-42;

	UT_Get_TLVLengthOfV(glv_tag9F48.Length, &lenOfV);
	lenOfIPKR = lenOfV;

	if (lenOfIPKR != 0)
	{
		memcpy(&bufHash[lenOfHash], glv_tag9F48.Value, lenOfIPKR);
		lenOfHash += lenOfIPKR;
	}
	else
	{
		if (lenOfIPKL > lenOfLMD)
		{
			return FAIL;
		}
	}

	UT_Get_TLVLengthOfV(glv_tag9F47.Length, &lenOfV);
	lenOfIPKE = lenOfV;

	if (lenOfIPKE == 0)
	{
		glv_tag95.Value[0] |= DPS_TVR_ICC_DATA_MISSING;
		
		return FAIL;
	}
	else
	{
		memcpy(&bufHash[lenOfHash], glv_tag9F47.Value, lenOfIPKE);
		lenOfHash += lenOfIPKE;
	}

	lenOfSDTBA = UT_C2S(&oda_bufRecord[0]);
	if (lenOfSDTBA != 0)
	{
		if ((lenOfHash+lenOfSDTBA) > sizeof(bufHash))
		{
			return FAIL;
		}

		memcpy(&bufHash[lenOfHash], &oda_bufRecord[2], lenOfSDTBA);
		lenOfHash += lenOfSDTBA;
	}

	//Step 6: Apply Hash Algorithm
	rspCode = api_sys_SHA1(lenOfHash, bufHash, rstHash);
	if (rspCode == apiFailed)
	{
		return FAIL;
	}

	//Step 7: Compare Hash Result
	rspCode = UT_bcdcmp(&bufCertificate[2+lenOfIPKC-(20+1)], rstHash, 20); //Hash Result(20)+Trailer(1)
	if (rspCode != 0)
	{
		return FAIL;
	}

	//Step 8: Compare PAN
	memset(bufPAN, 0xFF, 10);
	UT_Get_TLVLengthOfV(glv_tag5A.Length, &lenOfV);
	memcpy(bufPAN, glv_tag5A.Value, lenOfV);

	rspCode = UT_CNcmp2(&bufCertificate[2+2], bufPAN, 10);	//Add 2 to Point to Application PAN
	if (rspCode == FALSE)
	{
		return FAIL;
	}

	//Step 9: Verify Certificate Expiration Date
	rspCode = UT_VerifyCertificateExpDate(&bufCertificate[2+12]);	//Add 12 to Point to Certificate Expiration Date
	if (rspCode == FALSE)
	{
		return FAIL;
	}

	//Step 10: Check ICC Public Key Algorithm Indicator = 0x01
	if (bufCertificate[2+18] != 0x01)	//Add 18 to Point to ICC Public Key Algorithm Indicator
	{
		return FAIL;
	}

	//Step 11: Concatenate ICC Public Key Modulus
	memcpy(optModulus, &bufCertificate[2+21], lenOfLMD);	//Add 21 to Point to ICC Public Key or Leftmost Digits of the Issuer Public Key

	if (lenOfIPKR != 0)
	{
		memcpy(&optModulus[lenOfLMD], glv_tag9F48.Value, lenOfIPKR);
	}

	optModLen[0] = lenOfIPKL;

	if (lenOfIPKE == 1)
	{
		memset(optExponent, 0, 3);
		optExponent[2] = glv_tag9F47.Value[0];
	}
	else if (lenOfIPKE == 3)
	{
		memcpy(optExponent, glv_tag9F47.Value, 3);
	}
	
	return SUCCESS;
}

UCHAR DPS_Verify_DynamicSignature(UCHAR	iptModLen, UCHAR *iptModulus, UCHAR	*iptExponent)
{
	UCHAR	rspCode = FALSE;
	UCHAR	lenOfT = 0;
	UCHAR	lenOfL = 0;
	UINT	lenOfV = 0;
	UINT	lenOfTag77 = 0;
	UINT	lenOfRemainder = 0;
	UINT	lenOfData = 0;
	UINT	lenOfHash = 0;			//Length of Hash Data
	UINT	lenOfRmvPadding = 0;		//Length of Padding Removed Buffer
	UINT	lenOfSDAD = 0;			//Length of SDAD
	UCHAR	lenOfIDN = 0;				//Length of ICC Dynamic Number
	UCHAR	bufSDAD[2+256] = {0};		//Buffer of SDAD [Len(2)+Data(256)]
	UCHAR	bufHash[2048] = {0};		//Buffer of Hash
	UCHAR	bufRmvPadding[512] = {0};	//Buffer of Padding Removed
	UCHAR	bufModulus[2+256] = {0};	//Buffer of Modulus
	UCHAR	rstHash[20] = {0};		//Result of Hash	
	UCHAR	*ptrStart = NULLPTR;
	UCHAR	*ptrSDAD = NULLPTR;
	UCHAR	*ptrAftSDAD = NULLPTR;
	UCHAR	tag9F4B[2] = {0x9F,0x4B};
	

	//[Book 2 6.6.2]
	//Step 1: Check Signed Dynamic Application Data Length = ICC Public Key Modulus Length
	UT_Get_TLVLengthOfV(glv_tag9F4B.Length, &lenOfV);
	lenOfSDAD = lenOfV;
	
	if (lenOfSDAD != iptModLen)
	{
		return FAIL;
	}

	//Step 2: Recover SDAD and Check Trailer = 0xBC
	bufModulus[0] = iptModLen;
	memcpy(&bufModulus[2], iptModulus, iptModLen);
	rspCode = api_rsa_loadkey(bufModulus, iptExponent);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	UT_S2C(lenOfSDAD, &bufSDAD[0]);
	memcpy(&bufSDAD[2], glv_tag9F4B.Value, lenOfSDAD);

	rspCode = api_rsa_recover(bufSDAD, bufSDAD);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	DPS_Check_api_rsa_recover(bufSDAD[0], &bufSDAD[2]);

	if (bufSDAD[2+lenOfSDAD-1] != 0xBC)	//Decrease 1 to Point to Trailer
	{
		return FAIL;
	}

	//Step 3: Check Header = 0x6A
	if (bufSDAD[2+0] != 0x6A)
	{
		return FAIL;
	}

	//Step 4: Check Signed Data Format = 0x05
	if (bufSDAD[2+1] != 0x05)	//Add 1 to Point to Signed Data Format
	{
		return FAIL;
	}

	if (bufSDAD[2+2] != 0x01)	//Add 2 to Point to Hash Algorithm Indicator
	{
		return FAIL;
	}

	//Step 5: Retrieve ICC Dynamic Data
	lenOfIDN = bufSDAD[2+4];	//Add 4 to Point to ICC Dynamic Number Length

	//Step 6: Check CID retrieved from the ICC Dynamic Data = CID obtained from the response to the GENERATE AC command
	if (bufSDAD[2+(5+lenOfIDN)] != glv_tag9F27.Value[0])	//Add (5+lenOfIDNL) to Point to Cryptogram Information Data
	{
		return FAIL;
	}

	//Step 7: Concatenation
	lenOfHash = lenOfSDAD-(1+20+1);				//Header(1)+Hash Result(20)+Trailer(1)
	memcpy(bufHash, &bufSDAD[2+1], lenOfHash);	//Start from Signed Data Format

	memcpy(&bufHash[lenOfHash], glv_tag9F37.Value, 4);
	lenOfHash += 4;

	//Step 8: Apply Hash Algorithm
	rspCode = api_sys_SHA1(lenOfHash, bufHash, rstHash);
	if (rspCode == apiFailed)
	{
		return FAIL;
	}

	//Step 9: Compare Hash Result
	rspCode = UT_bcdcmp(&bufSDAD[2+lenOfSDAD-(20+1)], rstHash, 20); //Hash Result(20)+Trailer(1)
	if (rspCode != 0)
	{
		return FAIL;
	}

	//Step 10: Concatenation
	memset(bufHash, 0, sizeof(bufHash));
	lenOfHash = 0;

	//PDOL values sent by the terminal
	UT_Get_TLVLengthOfV(glv_tagDF8111.Length, &lenOfV);	//PDOL Related Data
	memcpy(bufHash, glv_tagDF8111.Value, lenOfV);
	lenOfHash += lenOfV;

	//TLV data returned by the card to GPO command
	UT_Get_TLVLength(oda_bufRspGPO, &lenOfT, &lenOfL, &lenOfV);
	lenOfTag77 = lenOfV;
	ptrStart = oda_bufRspGPO + (lenOfT + lenOfL);	//Point to TLV-V

	//Exclude Signed Dynamic Application Data
	ptrSDAD = UT_Find_Tag(tag9F4B, lenOfV, ptrStart);
	if (ptrSDAD != NULLPTR)
	{
		lenOfData = ptrSDAD - ptrStart;
		if (lenOfData != 0)
		{
			rspCode = UT_Remove_PaddingData((ptrSDAD-ptrStart), ptrStart, &lenOfRmvPadding, bufRmvPadding);
			if (rspCode == FAIL)
			{
				return FAIL;
			}

			memcpy(&bufHash[lenOfHash], bufRmvPadding, lenOfRmvPadding);

			lenOfHash += lenOfRmvPadding;
		}

		UT_Get_TLVLength(ptrSDAD, &lenOfT, &lenOfL, &lenOfV);
		ptrAftSDAD = ptrSDAD+(lenOfT+lenOfL+lenOfV);	//Skip SDAD TLV

		lenOfRemainder = lenOfTag77-(lenOfData+(lenOfT+lenOfL+lenOfV));	//Remainder Length = Total - (Data Before SDAD + SDAD)
		if (lenOfRemainder != 0)	
		{
			rspCode = UT_Remove_PaddingData(lenOfRemainder, ptrAftSDAD, &lenOfRmvPadding, bufRmvPadding);
			if (rspCode == FAIL)
			{
				return FAIL;
			}
			
			memcpy(&bufHash[lenOfHash], bufRmvPadding, lenOfRmvPadding);
			lenOfHash += lenOfRmvPadding;
		}
	}
	else
	{
		return FAIL;
	}

	//Step 11: Apply Hash Algorithm
	memset(rstHash, 0, 20);
	rspCode = api_sys_SHA1(lenOfHash, bufHash, rstHash);
	if (rspCode == apiFailed)
	{
		return FAIL;
	}

	//Step 12: Compare Transaction Data Hash Code
	rspCode = UT_bcdcmp(&bufSDAD[2+5+lenOfIDN+9], rstHash, 20);	//Add 14+lenOfIDNL to Point to Transaction Data Hash Code
	if (rspCode != 0)
	{
		return FAIL;
	}

	if ((lenOfIDN >= 2) && (lenOfIDN <= 8))
	{
		UT_Set_TagLength(lenOfIDN, glv_tag9F4C.Length);
		memcpy(glv_tag9F4C.Value, &bufSDAD[2+5], lenOfIDN);	//Add 5 to Point to ICC Dynamic Number
	}

	UT_Set_TagLength(8, glv_tag9F26.Length);
	memcpy(glv_tag9F26.Value, &bufSDAD[2+5+lenOfIDN+1], 8);	//Add 6+lenOfIDNL to Point to TC or ARQC
	
	return SUCCESS;
}

UCHAR DPS_OfflineDataAuthentication_CDA(void)
{
	UCHAR	cauModLen = 0;
	UCHAR	issModLen = 0;
	UCHAR	iccModLen = 0;
	UCHAR	cauModulus[255] = {0};
	UCHAR	issModulus[255] = {0};
	UCHAR	iccModulus[255] = {0};
	UCHAR	cauExponent[3] = {0};
	UCHAR	issExponent[3] = {0};
	UCHAR	iccExponent[3] = {0};
	UCHAR	rspCode = FAIL;


	//tag'9F06' : Application Identifier (AID), tag'8F' : Certification Authority Public Key Index
	rspCode = DPS_Retrieve_PK_CA(glv_tag9F06.Value, glv_tag8F.Value[0], &cauModLen, cauModulus, cauExponent);
	if (rspCode == SUCCESS)
	{
		DBG_Put_String(13, (UCHAR*)"Retrieve CAPK");	// ==== [Debug] ====

		rspCode = DPS_Retrieve_PK_Issuer(cauModLen, cauModulus, cauExponent, &issModLen, issModulus, issExponent);
		if (rspCode == SUCCESS)
		{
			DBG_Put_String(18, (UCHAR*)"Retrieve Issuer PK");	// ==== [Debug] ====

			rspCode = DPS_Retrieve_PK_ICC(issModLen, issModulus, issExponent, &iccModLen, iccModulus, iccExponent);
			if (rspCode == SUCCESS)
			{
				DBG_Put_String(15, (UCHAR*)"Retrieve ICC PK");	// ==== [Debug] ====

				rspCode = DPS_Verify_DynamicSignature(iccModLen, iccModulus, iccExponent);
				if (rspCode == SUCCESS)
				{
					DBG_Put_String(11, (UCHAR*)"Verify SDAD");	// ==== [Debug] ====

					glv_tag95.Value[0] &= 0xFB;	//CDA successfully completed
					return SUCCESS;
				}
			}
		}
	}
	else
		glv_tag95.Value[0] |= DPS_TVR_ICC_DATA_MISSING;

	glv_tag95.Value[0] |= DPS_TVR_CDA_FAILED;

	return FAIL;
}

//D-PAS does not support SDA and DDA
//CDA is mandatory if Terminal supports offline transactions
UCHAR DPS_ODA_Process(void)
{
	UCHAR	rspCode = 0;

	DBG_Put_String(13, (UCHAR*)"==== ODA ====");	// ==== [Debug] ====

	//Set TVR B1b8 to ��0�� and TSI B1b8 to ��1��
	glv_tag95.Value[0] &= 0x7F;	//Offline data authentication was not performed
	glv_tag9B.Value[0] |= DPS_TSI_OFFLINE_DATA_AUTHENTICATION_WAS_PERFORMED;
	
	DPS_DBG_Put_Process(DPS_FIGURE_21, 1, 0xFF);

	//Check CDA mandatory data
	if(glv_tag8F.Length[0] == 0)
	{
		//Set TVR B1b6 and B1b3 to ��1��
		glv_tag95.Value[0] |= 0x24;	//ICC Data missing, CDA failed

		DPS_DBG_Put_Process(DPS_FIGURE_21, 4, FALSE);

		return FAIL;
	}

	DPS_DBG_Put_Process(DPS_FIGURE_21, 4, TRUE);

	rspCode = DPS_OfflineDataAuthentication_CDA();
	return rspCode;
}

UCHAR DPS_CVM_Mobile_Process(void)
{
	UCHAR	tmp_value = 0;

	DBG_Put_String(20, (UCHAR*)"==== CVM Mobile ====");	// ==== [Debug] ====

	//Step 1
	tmp_value = glv_tag9F53.Value[1];
	tmp_value &= 0x01;	//CVR B2b1 : Confirmation Code Verification performed and failed (Mobile implementation only)
	if(!tmp_value)
	{
		DPS_DBG_Put_Process(DPS_FIGURE_20, 1, FALSE);

		//Step 3
		tmp_value = glv_tag9F53.Value[1];
		tmp_value &= 0x02;	//CVR B2b2 : Confirmation Code Verification performed (Mobile implementation only)
		if(tmp_value)
		{
			etp_Outcome.CVM = ETP_OCP_CVM_ConfirmationCodeVerified;	//Approve CVM

			DPS_DBG_Put_Process(DPS_FIGURE_20, 3, TRUE);

			return SUCCESS;
		}
		
		DPS_DBG_Put_Process(DPS_FIGURE_20, 3, FALSE);
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_20, 1, TRUE);
	}

	//Step 2 : CPR B2b2 = 1 ?
	tmp_value = glv_tag9F71.Value[1];
	tmp_value &= DPS_CPR_CVM_FALLBACK_TO_SIGNATURE_ALLOWED;
	if(tmp_value)
	{
		DPS_DBG_Put_Process(DPS_FIGURE_20, 2, TRUE);

		//Step 6 : TTQ B1b2 = 1 ?
		tmp_value = glv_tag9F66.Value[0];
		tmp_value &= DPS_TTQ_SIGNATURE_SUPPORTED;
		if(tmp_value)
		{
			etp_Outcome.CVM = ETP_OCP_CVM_ObtainSignature;	//Ask user to sign a receipt and approve CVM

			DPS_DBG_Put_Process(DPS_FIGURE_20, 6, TRUE);

			return SUCCESS;
		}

		DPS_DBG_Put_Process(DPS_FIGURE_20, 6, FALSE);
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_20, 2, FALSE);
	}
	
	//Step 4 : CPR B2b1 = 1 ?
	tmp_value = glv_tag9F71.Value[1];
	tmp_value &= DPS_CPR_CVM_FALLBACK_TO_NO_CVM_ALLOWED;	//CPR B2b1 : CVM Fallback to No CVM allowed

	//Test for [DPAS_EMV_C6_v1.0_R1_Library]
	if(tmp_value & DPS_CPR_CVM_FALLBACK_TO_NO_CVM_ALLOWED)
	{
		etp_Outcome.CVM = ETP_OCP_CVM_NoCVM;	//Approve CVM

		DPS_DBG_Put_Process(DPS_FIGURE_20, 4, TRUE);

		return SUCCESS;
	}

	//Test for [DPAS_EMV_C6_v1.21_R1_Library]
	//Update 23b in [Terminal Application Specification Bulletin CL TAS-002, v1.0]
	//if((tmp_value & DPS_CPR_CVM_FALLBACK_TO_NO_CVM_ALLOWED) && (glv_tag9F33.Value[1] & 0x08))	//Check CVM Fallback to No CVM allowed & No CVM Required
	//{
	//	etp_Outcome.CVM = ETP_OCP_CVM_NoCVM;	//Approve CVM

	//	DPS_DBG_Put_Process(DPS_FIGURE_20, 4, TRUE);

	//	return SUCCESS;
	//}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_20, 4, FALSE);

		//Step 5 : set TVR B3b8 = 1
		glv_tag95.Value[2] |= DPS_TVR_CARDHOLDER_VERIFICATION_WAS_NOT_SUCCESSFUL;

		//Show TVR and TSI value (for test)
		DBG_Put_String(4, (UCHAR*)"TVR:");
		DBG_Put_Hex(5, glv_tag95.Value);

		DBG_Put_String(4, (UCHAR*)"TSI:");
		DBG_Put_Hex(2, glv_tag9B.Value);

		DPS_DBG_Put_Process(DPS_FIGURE_20, 5, 0xFF);

		//return DPS_End_APP_Processing_Error;
		return FAIL;
	}
}

UCHAR DPS_CVM_Common_Process(void)
{
	UCHAR	tmp_value = 0;
	UCHAR	rspCode = 0;

	DBG_Put_String(13, (UCHAR*)"==== CVM ====");	// ==== [Debug] ====

	//Step 1 : Check CVM Required
	tmp_value = glv_tag9F66.Value[1];
	tmp_value &= DPS_TTQ_CVM_REQUIRED;
	if(!tmp_value)
	{
		DPS_DBG_Put_Process(DPS_FIGURE_19, 1, FALSE);

		//Step 2 : Check Online PIN required and Signature required
		tmp_value = glv_tag9F71.Value[0];
		tmp_value &= 0xC0;	//CPR B1b8-7 : Online PIN required and Signature required
		if(!tmp_value)
		{
			etp_Outcome.CVM = ETP_OCP_CVM_NA;	//Skip CVM

			//Show TVR and TSI value (for test)
			DBG_Put_String(4, (UCHAR*)"TVR:");
			DBG_Put_Hex(5, glv_tag95.Value);

			DBG_Put_String(4, (UCHAR*)"TSI:");
			DBG_Put_Hex(2, glv_tag9B.Value);

			DPS_DBG_Put_Process(DPS_FIGURE_19, 2, TRUE);

			return SUCCESS;
		}

		DPS_DBG_Put_Process(DPS_FIGURE_19, 2, FALSE);
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_19, 1, TRUE);
	}

	//Step 3 : Set Cardholder verification was performed
	glv_tag9B.Value[0] |= DPS_TSI_CARDHOLDER_VERIFICATION_WAS_PERFORMED;

	DPS_DBG_Put_Process(DPS_FIGURE_19, 3, 0xFF);

	//Step 4 : Check Online PIN required
	tmp_value = glv_tag9F71.Value[0];
	tmp_value &= DPS_CPR_ONLINE_PIN_REQUIRED;
	if(tmp_value)
	{
		DPS_DBG_Put_Process(DPS_FIGURE_19, 4, TRUE);

		//Step 6 : Verify that the terminal supports online PIN
		tmp_value = glv_tag9F66.Value[0];
		tmp_value &= DPS_TTQ_ONLINE_PIN_SUPPORTED;
		if(tmp_value)
		{
			DPS_DBG_Put_Process(DPS_FIGURE_19, 6, TRUE);

			//Step 9 : Set Online PIN entered
			glv_tag95.Value[2] |= DPS_TVR_ONLINE_PIN_ENTERED;

			etp_Outcome.CVM = ETP_OCP_CVM_OnlinePIN;

			//Show TVR and TSI value (for test)
			DBG_Put_String(4, (UCHAR*)"TVR:");
			DBG_Put_Hex(5, glv_tag95.Value);

			DBG_Put_String(4, (UCHAR*)"TSI:");
			DBG_Put_Hex(2, glv_tag9B.Value);

			DPS_DBG_Put_Process(DPS_FIGURE_19, 9, 0xFF);

			return SUCCESS;
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_19, 6, FALSE);

			//Step 7 : Check CVM Fallback to Signature allowed
			tmp_value = glv_tag9F71.Value[1];
			tmp_value &= DPS_CPR_CVM_FALLBACK_TO_SIGNATURE_ALLOWED;
			if(tmp_value)
			{
				DPS_DBG_Put_Process(DPS_FIGURE_19, 7, TRUE);

				//Step 8 : Check Signature supported
				tmp_value = glv_tag9F66.Value[0];
				tmp_value &= DPS_TTQ_SIGNATURE_SUPPORTED;
				if(tmp_value)
				{
					etp_Outcome.CVM = ETP_OCP_CVM_ObtainSignature;	//Ask user to sign a receipt and approve CVM

					return SUCCESS;
				}
			}
			else
			{
				DPS_DBG_Put_Process(DPS_FIGURE_19, 7, FALSE);
			}
		}
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_19, 4, FALSE);

		//Step 5 : Check Signature required
		tmp_value = glv_tag9F71.Value[0];
		tmp_value &= DPS_CPR_SIGNATURE_REQUIRED;
		if(tmp_value)
		{
			DPS_DBG_Put_Process(DPS_FIGURE_19, 5, TRUE);

			//Step 8 : Check Signature supported
			tmp_value = glv_tag9F66.Value[0];
			tmp_value &= DPS_TTQ_SIGNATURE_SUPPORTED;
			if(tmp_value)
			{
				etp_Outcome.CVM = ETP_OCP_CVM_ObtainSignature;	//Ask user to sign a receipt and approve CVM

				//Show TVR and TSI value (for test)
				DBG_Put_String(4, (UCHAR*)"TVR:");
				DBG_Put_Hex(5, glv_tag95.Value);

				DBG_Put_String(4, (UCHAR*)"TSI:");
				DBG_Put_Hex(2, glv_tag9B.Value);

				DPS_DBG_Put_Process(DPS_FIGURE_19, 8, TRUE);

				return SUCCESS;
			}
			else
			{
				DPS_DBG_Put_Process(DPS_FIGURE_19, 8, FALSE);
			}
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_19, 5, FALSE);
		}
	}

	//Step 10 : Check Consumer Device CVM supported
	tmp_value = glv_tag9F66.Value[2];
	tmp_value &= DPS_TTQ_CONSUMER_DEVICE_CVM_SUPPORTED;
	if(tmp_value)
	{
		DPS_DBG_Put_Process(DPS_FIGURE_19, 10, TRUE);

		//Step 12 : Check Consumer Device CVM Performed
		tmp_value = glv_tag9F71.Value[0];
		tmp_value &= DPS_CPR_CONSUMER_DEVICE_CVM_PERFORMED;
		if(tmp_value)
		{
			DPS_DBG_Put_Process(DPS_FIGURE_19, 12, TRUE);

			rspCode = DPS_CVM_Mobile_Process();	//Go to CVM Mobile
			return rspCode;
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_19, 12, FALSE);
		}
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_19, 10, FALSE);
	}

	//Step 11 : Check CVM Fallback to No CVM allowed & No CVM Required
	tmp_value = glv_tag9F71.Value[1];

	//Test for [DPAS_EMV_C6_v1.0_R1_Library]
	if(tmp_value & DPS_CPR_CVM_FALLBACK_TO_NO_CVM_ALLOWED)
	{
		etp_Outcome.CVM = ETP_OCP_CVM_NoCVM;	//Approve CVM

		//Show TVR and TSI value (for test)
		DBG_Put_String(4, (UCHAR*)"TVR:");
		DBG_Put_Hex(5, glv_tag95.Value);

		DBG_Put_String(4, (UCHAR*)"TSI:");
		DBG_Put_Hex(2, glv_tag9B.Value);

		DPS_DBG_Put_Process(DPS_FIGURE_19, 11, TRUE);

		return SUCCESS;
	}

	//Test for [DPAS_EMV_C6_v1.21_R1_Library]
	//Update 23a in [Terminal Application Specification Bulletin CL TAS-002, v1.0]
	//if((tmp_value & DPS_CPR_CVM_FALLBACK_TO_NO_CVM_ALLOWED) && (glv_tag9F33.Value[1] & 0x08))
	//{
	//	etp_Outcome.CVM = ETP_OCP_CVM_NoCVM;	//Approve CVM

	//	//Show TVR and TSI value (for test)
	//	DBG_Put_String(4, (UCHAR*)"TVR:");
	//	DBG_Put_Hex(5, glv_tag95.Value);

	//	DBG_Put_String(4, (UCHAR*)"TSI:");
	//	DBG_Put_Hex(2, glv_tag9B.Value);

	//	DPS_DBG_Put_Process(DPS_FIGURE_19, 11, TRUE);

	//	return SUCCESS;
	//}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_19, 11, FALSE);

		//Step 13 : Check if it supports contact or magnetic stripe
		tmp_value = glv_tag9F66.Value[0];
		tmp_value &= 0x90;	//TTQ B1b8 : Magnetic stripe mode supported, B1b5 : EMV contact chip supported
		if(tmp_value)	//Ask user to insert or swipe the card
		{
			DPS_DBG_Put_Process(DPS_FIGURE_19, 13, TRUE);

			//Show TVR and TSI value (for test)
			DBG_Put_String(4, (UCHAR*)"TVR:");
			DBG_Put_Hex(5, glv_tag95.Value);

			DBG_Put_String(4, (UCHAR*)"TSI:");
			DBG_Put_Hex(2, glv_tag9B.Value);

			return DPS_Try_Another;
		}
		else  //Ask user to use another card
		{
			DPS_DBG_Put_Process(DPS_FIGURE_19, 13, FALSE);

			//return DPS_End_APP_Processing_Error;
			return FAIL;
		}	
	}
}

UCHAR DPS_7bit_LRC(UCHAR *buf, UINT len, UCHAR track)
{
	UCHAR	i = 0;
	UCHAR	j = 0;
	UCHAR	xorBuf = 0;
	UCHAR	lrc = 0;

	if(track == 1)	// Calculating odd parity and LRC for Track 1
	{
		for(i = 0 ; i < len ; i++)
		{
			for(j = 0 ; j < CHARACTER_NUM ; j++)
			{
				if(buf[i] == characterTable[j].character)
				{
					xorBuf ^= characterTable[j].binary;
					break;
				}
			}
		}

		xorBuf &= 0xBF;

		//Search character table
		for(j = 0 ; j < CHARACTER_NUM ; j++)
		{
			if(xorBuf == (characterTable[j].binary & 0xBF))
			{
				lrc = characterTable[j].character;
			}
		}
	}
	else  // Calculating odd parity and LRC for Track 2
	{
		for(i = 0 ; i < len ; i++)
		{
			xorBuf ^= (buf[i] - '0');
		}

		xorBuf &= 0x0F;
		xorBuf += '0';

		lrc = xorBuf;
	}

	return lrc;
}

UCHAR DPS_LRC(UCHAR *buf, UINT len)
{
	UINT i;
	UCHAR lrc = 0;		 
	 
	for( i = 0; i < len; i++ )
	{
		lrc = lrc + buf[i];
	}
	 
	return ((UCHAR)(-((char)lrc)));
}

void DPS_Create_MSDTrackNoDCVV(void)
{
	UINT	lenData = 0;
	UINT	lenOfV = 0;

	//Reset Data
	dps_MSD_Track1_Data_Length = 0;
	memset(dps_MSD_Track1_Data, 0x00, 79);
	dps_MSD_Track2_Data_Length = 0;
	memset(dps_MSD_Track2_Data, 0x00, 40);

	//Update Track 1 Data (ASCII format)
	//Field 1 - Start Sentinel
	dps_MSD_Track1_Data[lenData++] = '%';

	//Field 2 - Tag 56 return from card
	UT_Get_TLVLengthOfV(glv_tag56.Length, &lenOfV);

	memcpy(&dps_MSD_Track1_Data[lenData], glv_tag56.Value, lenOfV);
	lenData += lenOfV;

	//Field 3 - End Sentinel
	dps_MSD_Track1_Data[lenData++] = '?';

	//Field 4 - LRC
	dps_MSD_Track1_Data_Length = lenData;
	
	dps_MSD_Track1_Data[lenData] = DPS_7bit_LRC(dps_MSD_Track1_Data, dps_MSD_Track1_Data_Length, 1);
	dps_MSD_Track1_Data_Length ++;

	//Reset
	lenData = 0;

	//Update Track 2 Equivalentdata (packed BCD format)
	//Field 1 - Start Sentinel
	dps_MSD_Track2_Data[lenData++] = ';';
	
	//Field 2 - Tag 57 return from card
	UT_Get_TLVLengthOfV(glv_tag57.Length, &lenOfV);
	UT_Split(&dps_MSD_Track2_Data[lenData], glv_tag57.Value, (char)lenOfV);
	lenData += (2 * lenOfV);

	//Field 3 - End Sentinel
	dps_MSD_Track2_Data[lenData++] = '?';

	//Field 4 - LRC
	dps_MSD_Track2_Data_Length = lenData;

	dps_MSD_Track2_Data[lenData] = DPS_7bit_LRC(dps_MSD_Track2_Data, dps_MSD_Track2_Data_Length, 2);
	dps_MSD_Track2_Data_Length ++;
}

void DPS_Create_MSDTrack(void)
{
	UCHAR	*ptrData = NULLPTR;
	UINT	lenOfV = 0;
	UCHAR	numIndex = 0;
	UCHAR	lenPAN = 0;
	UINT	lenData = 0;
	UCHAR	lenPadding = 0;
	UCHAR	DCVV[3] = {0};
	UCHAR	i = 0;
	UCHAR	tempBuffer[38] = {0};

	//Reset Data
	dps_MSD_Track1_Data_Length = 0;
	memset(dps_MSD_Track1_Data, 0x00, 79);
	dps_MSD_Track2_Data_Length = 0;
	memset(dps_MSD_Track2_Data, 0x00, 40);

	//Find DCVV value
	UT_Get_TLVLengthOfV(glv_tag9F7E.Length, &lenOfV);
	if(lenOfV != 0)
	{
		memcpy(DCVV, glv_tag9F7E.Value, lenOfV);
	}
	else
	{
		UT_Get_TLVLengthOfV(glv_tag9F80.Length, &lenOfV);
		if(lenOfV != 0)
		{
			memcpy(DCVV, glv_tag9F80.Value, lenOfV);
		}
	}

	//Update Track 1 Data (ASCII format)
	ptrData = glv_tag56.Value;

	//Field 1 - Start Sentinel
	dps_MSD_Track1_Data[lenData++] = '%';

	//Field 2 - Format Code
	dps_MSD_Track1_Data[lenData++] = *ptrData;	//B
	ptrData++;

	//Field 3 - PAN
	for(numIndex = 0 ; numIndex < (16 + 1) ; numIndex++)	//PAN(16) + Separator(1)
	{
		if(*ptrData == '^')
		{
			lenPAN = numIndex;
			break;
		}
		dps_MSD_Track1_Data[lenData++] = *ptrData;
		ptrData++;
	}

	//Field 4 - Field Separator
	dps_MSD_Track1_Data[lenData++] = *ptrData;	//^
	ptrData++;	//Point to Cardholder Name

	//Field 5 - Cardholder Name
	memcpy(&dps_MSD_Track1_Data[lenData], ptrData, 26);
	lenData += 26;
	ptrData += 26;	//Point to Field Separator

	//Field 6 - Field Separator
	dps_MSD_Track1_Data[lenData++] = *ptrData;	//^
	ptrData++;	//Point to Expiry Date

	//Field 7 - Expiry Date
	memcpy(&dps_MSD_Track1_Data[lenData], ptrData, 4);
	lenData += 4;
	ptrData += 4;	//Point to Service Code

	//Field 8 - Service Code
	memcpy(&dps_MSD_Track1_Data[lenData], ptrData, 3);
	lenData += 3;
	ptrData += 3;	//Point to IDD in position 54

	//Field 9 - RFU
	/*If the PAN is less than 16 digits, zeroes must be padded after Service Code (in the card during personalization) 
	  to ensure that DCVV values are still inserted at the positions shown.*/
	if(lenPAN < 16)
	{
		lenPadding = 16 - lenPAN;
		memset(&dps_MSD_Track1_Data[lenData], 0x30, lenPadding);
		lenData += lenPadding;	
	}

	//Field 10 - IDD
	memcpy(&dps_MSD_Track1_Data[lenData], ptrData, 5);
	lenData += 5;
	ptrData += 5;	//Point to IDD in position 55

	//Field 11 - DCVV
	for(i = 0 ; i < 3 ; i++)
	{
		dps_MSD_Track1_Data[lenData] = DCVV[i] + 0x30;
		lenData++;
	}
	ptrData += 3;	//Point to IDD in position 62

	//Field 12 - ATC
	memcpy(&dps_MSD_Track1_Data[lenData], ptrData, 3);
	lenData += 3;
	ptrData += 3;	//point to position 65

	//Field 13 - UN
	//Insert 2 digit numeric UN data from the terminal generated UN sent in PDOL to card (least significant digits of the UN) in positions 65-66
	if(glv_tag9F37.Length[0] > 1)
	{
		//If the Unpredictable Number is longer than 1 byte then insert the complete Unpredictable Number into Track 1
		memcpy(&dps_MSD_Track1_Data[lenData], glv_tag9F37.Value, 2);
		lenData += 2;
	}
	else
	{
		UT_Split(&dps_MSD_Track1_Data[lenData], glv_tag9F37.Value, 1);
		lenData += 2;
	}

	//Field 14 - End Sentinel
	dps_MSD_Track1_Data[lenData++] = '?';

	//Field 15 - LRC
	dps_MSD_Track1_Data_Length = lenData;
	dps_MSD_Track1_Data[lenData] = DPS_7bit_LRC(dps_MSD_Track1_Data, dps_MSD_Track1_Data_Length, 1);
	dps_MSD_Track1_Data_Length ++;

	//Reset
	lenData = 0;

	//Update Track 2 Equivalentdata (packed BCD format)
	UT_Split(tempBuffer, glv_tag57.Value, (char)glv_tag57.Length[0]);

	ptrData = tempBuffer;

	//Field 1 - Start Sentinel
	dps_MSD_Track2_Data[lenData++] = ';';
	
	//Field 2 - PAN
	for(numIndex = 0 ; numIndex < (16 + 1) ; numIndex++)	//PAN(16) + Separator(1)
	{
		if(*ptrData == 'D')
		{
			lenPAN = numIndex;
			break;
		}
		dps_MSD_Track2_Data[lenData++] = *ptrData;
		ptrData++;
	}

	//Field 3 - Separator
	dps_MSD_Track2_Data[lenData++] = *ptrData;
	ptrData++;	//Point to Expiry Date

	//Field 4 - Expiry Date
	memcpy(&dps_MSD_Track2_Data[lenData], ptrData, 4);
	lenData += 4;
	ptrData += 4;	//Point to Service Code

	//Field 5 - Service Code
	memcpy(&dps_MSD_Track2_Data[lenData], ptrData, 3);
	lenData += 3;
	ptrData += 3;	//Point to IDD in positon 26

	//Field 6 - RFU
	/*If the PAN is less than 16 digits, zeroes must be padded after Service Code (in the card during personalization) 
	  to ensure that DCVV values are still inserted at the positions shown.*/
	if(lenPAN < 16)
	{
		lenPadding = 16 - lenPAN;
		memset(&dps_MSD_Track2_Data[lenData], 0x30, lenPadding);
		lenData += lenPadding;	
	}

	//Field 7 - IDD
	memcpy(&dps_MSD_Track2_Data[lenData], ptrData, 5);
	lenData += 5;
	ptrData += 5;

	//Field 8 - DCVV
	for(i = 0 ; i < 3 ; i++)
	{
		dps_MSD_Track2_Data[lenData] = DCVV[i] + 0x30;
		lenData++;
	}

	ptrData += 3;	//Point to IDD in position 34

	//Field 9 - ATC
	memcpy(&dps_MSD_Track2_Data[lenData], ptrData, 3);
	lenData += 3;
	ptrData += 3;	//Point to position 37

	//Field 10 - UN
	//Insert 2 digit numeric UN data from the terminal generated UN sent in PDOL to card (least significant digits of the UN) in positions 37-38
	if(glv_tag9F37.Length[0] > 1)
	{
		//If the Unpredictable Number is longer than 1 byte then only the leftmost byte (2 decimal digits) is placed into Track 2
		memcpy(&dps_MSD_Track2_Data[lenData], glv_tag9F37.Value, 2);
		lenData += 2;
		ptrData += 2;
	}
	else
	{
		UT_Split(&dps_MSD_Track2_Data[lenData], glv_tag9F37.Value, 1);
		lenData += 2;
		ptrData += 2;
	}

	//Field 11 - End Sentinel
	dps_MSD_Track2_Data[lenData++] = '?';

	//Field 12 - LRC
	dps_MSD_Track2_Data_Length = lenData;
	dps_MSD_Track2_Data[lenData] = DPS_7bit_LRC(dps_MSD_Track2_Data, dps_MSD_Track2_Data_Length, 2);
	dps_MSD_Track2_Data_Length ++;
}

UCHAR DPS_Check_ZIP_AID(void)
{
	UCHAR	aidZIP[7] = {0xA0,0x00,0x00,0x03,0x24,0x10,0x10};

	if(!memcmp(glv_tag9F06.Value, aidZIP, 7))
		return SUCCESS;

	return FAIL;
}

UCHAR DPS_Check_DPAS_AID(void)
{
	UCHAR	aidMS[7] = {0xA0,0x00,0x00,0x01,0x52,0x30,0x10};
	
	//Contactless Application Configuration Options (CL-ACO)
	//B1b8 : D-PAS MS Mode is the preferred contactless mode using D-PAS AID

	if(!memcmp(glv_tag9F06.Value, aidMS, 7))
		return SUCCESS;

	return FAIL;
}

void DPS_Retrieve_Expired_Date_From_Track2(void)
{
	UCHAR	Track2Data[150] = {0};
	UINT	lenOf57 = 0;
	UCHAR	lenPAN = 0;
	UCHAR	numIndex = 0;
	UCHAR	*ptrData = NULLPTR;
	UCHAR	tempDate[6] = {0};
	UINT	year = 0;
	UCHAR	tmpCentury = 0;

	UT_Get_TLVLengthOfV(glv_tag57.Length, &lenOf57);
	UT_Split(Track2Data, glv_tag57.Value, (char)lenOf57);

	for(numIndex = 0 ; numIndex < (19+1) ; numIndex++)	//PAN(19) + Separator(1)
	{
		if(Track2Data[numIndex] == 'D')
		{
			lenPAN = numIndex;
			break;
		}
	}

	ptrData = &Track2Data[lenPAN + 1];	//Point to First Position after Separator
	memcpy(tempDate, ptrData, 4);
	UT_Compress(glv_tag5F24.Value, tempDate, 3);
	UT_Set_TagLength(3, glv_tag5F24.Length);

	//pad the last date of the month
	if(glv_tag5F24.Value[1] == 0x02)
	{
		tmpCentury = UT_SetCentury(glv_tag5F24.Value[0]);
		year = tmpCentury*256 + glv_tag5F24.Value[0];

		//check bissextile year
		if((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)))
		{
			glv_tag5F24.Value[2] = 0x29;
		}
		else
		{
			glv_tag5F24.Value[2] = 0x28;
		}
	}
	else if((glv_tag5F24.Value[1] == 0x04) || (glv_tag5F24.Value[1] == 0x06) || (glv_tag5F24.Value[1] == 0x09) || (glv_tag5F24.Value[1] == 0x11))
	{
		glv_tag5F24.Value[2] = 0x30;
	}
	else
	{
		glv_tag5F24.Value[2] = 0x31;
	}
}

void DPS_Retrieve_PAN_From_Track2(void)
{
	UINT	i = 0;

	for(i = 0 ; i < ECL_LENGTH_5A ; i++)
	{
		if(((glv_tag57.Value[i] & 0xF0) >> 4) == 0x0D)
		{
			UT_Set_TagLength(i, glv_tag5A.Length);
			break;
		}
		else
		{
			glv_tag5A.Value[i] = glv_tag57.Value[i];
		}
	}
}

UCHAR DPS_Check_RR_R_Tag(UCHAR SFINUMBER, UINT RRR_Len, UCHAR *RRR_Buf)
{
	UCHAR redundant = 0, tag9F02[4] = {0x9F,0x02,0x00,0x00};
	UCHAR TLength,LLength, tmplen = 0, *value_addr = 0, *len_addr = 0, rspCode;//,*ptrrec,tmptag;
	UINT tmp_tag_len, VLength;
	//UINT iTempLen;
	ULONG Index;

	//ICTK check Read Record Wrong Format		
	//Check the Format First
	UT_Get_TLVLengthOfT(RRR_Buf, &TLength);
	UT_Get_TLVLengthOfL(&RRR_Buf[TLength], &LLength);
	UT_Get_TLVLengthOfV(&RRR_Buf[TLength], &VLength);
	
	if(!((RRR_Buf[TLength+LLength+VLength] == 0x90) && (RRR_Buf[TLength+LLength+VLength+1] == 0x00)))	//check SW1 and SW2
		return FAIL;

	//For files with SFI in the range 1 to 10, the record tag ('70') and the record length are excluded from the offline data authentication process. 
	//All other data in the data field of the response to the READ RECORD command (excluding SW1 SW2) is included.
	//For files with SFI in the range 11 to 30, the record tag ('70') and the record length are not excluded from the offline data authentication process. 
	//Thus all data in the data field of the response to the READ RECORD command (excluding SW1 SW2) is included.
	if(RRR_Buf[0] != 0x70)
	{
		if(SFINUMBER<11)
		{
			return FAIL;
		}
	}
	//ICTK check Read Record Wrong Format	end
	
	while(tmplen != (RRR_Len-2))	//exclude SW1 SW2
	{
		//tmptag = *RRR_Buf;	//get first byte tag to check
		TLength = LLength = tmp_tag_len = 0;
		Index = 0;

		//is tag a Primitive data element ?
		//if(tmptag & 0x20)		//it's a Constructed data element
		if((*RRR_Buf & 0x20) && (*RRR_Buf != 0xFF))
		{	
			UT_Get_TLVLengthOfT(RRR_Buf, &TLength);	//get tag's Length
			
			tmplen += TLength;			
			RRR_Buf += TLength;			//shift buf pointer, and now we are in "length" field
						
			UT_Get_TLVLengthOfL(RRR_Buf, &LLength);	//get length's Length
		
			tmplen += LLength;
			RRR_Buf += LLength;			//shift buf pointer, and now we are in "Constructed data" field
		}
		else if((*RRR_Buf == 0x00) || (*RRR_Buf == 0xFF))	//ICTK, padding with 0x00,oxFF
		{
			tmplen += 1;
			RRR_Buf += 1;
		}
		else				//this tag is a Primitive data element
		{
			UT_Get_TLVLengthOfT(RRR_Buf, &TLength);	//get tag's Length
			
			rspCode = UT_Search(TLength, RRR_Buf, (UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &Index);	//search glv_tagTable and get the index	
			if(rspCode == SUCCESS)
			{						
				if(*glv_addTable[Index] != 0)		//test if it is redundant?
				{	
					//0507 ICTK, when card return 9F02 , Ignore it
					if(memcmp(glv_tagTable[Index].Tag,tag9F02,4))
					{
						redundant = 0x01;
						break;
					}
					else
					{
						tmplen += TLength;
						RRR_Buf += TLength;			//shift buf pointer, and now we are in "length" field

						UT_Get_TLVLengthOfL(RRR_Buf,&LLength);				//get length's Length
						UT_Get_TLVLengthOfV(RRR_Buf,&VLength);				//get Value's Length
						tmplen += LLength;
						tmplen += VLength;

						RRR_Buf += LLength;
						RRR_Buf += VLength;
					}
				}
				//copy "Read Record Response" to TAG table
				else
				{
					tmplen += TLength;
					RRR_Buf += TLength;			//shift buf pointer, and now we are in "length" field

					UT_Get_TLVLengthOfL(RRR_Buf,&LLength);	//get length's Length
					tmplen += LLength;
					UT_Get_TLVLengthOfV(RRR_Buf,&tmp_tag_len);
					tmplen += tmp_tag_len;
				
					len_addr = (UCHAR *)(glv_addTable[Index]);	//point to glv_addtable length address
					while(LLength--)
					{
						*len_addr++ = *RRR_Buf++;		//shift buf pointer, and now we're in "value" field
					}
					value_addr = (UCHAR *)(glv_addTable[Index]+3);	//point to glv_addtable value address
					while(tmp_tag_len--)	//copy data to glv_addTable
					{
						*value_addr++ = *RRR_Buf++ ;		//shift buf pointer, and now we're in "tag" field
					}
				}
			}
			else			//can't recognize, ignore it
			{
				tmplen += TLength;
				RRR_Buf += TLength;			//shift buf pointer, and now we are in "length" field

				UT_Get_TLVLengthOfL(RRR_Buf,&LLength);	//get length's Length
				tmplen += LLength;
				UT_Get_TLVLengthOfV(RRR_Buf,&tmp_tag_len);
				tmplen += tmp_tag_len;	
				
				RRR_Buf += LLength;				
				RRR_Buf += tmp_tag_len;	
			}
		}
	}
	if(redundant == 0x01)
	{
		return FAIL;
	}	
	else
		return SUCCESS;
}

UCHAR DPS_RAD_SAD_Data(UCHAR SFINUM, UINT RECNUM, UINT RRR_Len, UCHAR *RRR_Buf)
{
	//We only store the record that depends on AFL Byte4 
	//Store whole Tag70 for ODA
	if(RRR_Buf[0] == 0x70)
	{		
		// (1) check record LENGTH & template TAG=70
		if( (RRR_Buf[1] & 0x80) == 0 )	// 1-byte length (1..127)
		{
			//					   ---------------------------------------------
			//ADDR_DPS_REC_Buf[] = | SFINUM | Tag70 | Length | Record Template |
			//                     ---------------------------------------------
		  	memcpy((ADDR_DPS_REC_START+DPS_REC_LEN*(RECNUM)), &SFINUM, 1);
		  	memcpy((ADDR_DPS_REC_START+DPS_REC_LEN*(RECNUM))+1, RRR_Buf, RRR_Len-2);	//minus SW1,SW2
		}
		else // 2-byte length
		{
			switch( RRR_Buf[1] & 0x7f )
			{
				//					   ---------------------------------------------
				//ADDR_DPS_REC_Buf[] = | SFINUM | Tag70 | Length | Record Template |
				//                     ---------------------------------------------
				case 0x01: // 1-byte length (128..255)
					memcpy((ADDR_DPS_REC_START+DPS_REC_LEN*(RECNUM)), &SFINUM, 1);
		  			memcpy((ADDR_DPS_REC_START+DPS_REC_LEN*(RECNUM))+1, RRR_Buf,RRR_Len-2);	//minus SW1,SW2
					break;
					
				case 0x02: // 2-byte length (256..65535)			
					memcpy((ADDR_DPS_REC_START+DPS_REC_LEN*(RECNUM)), &SFINUM, 1);
		  			memcpy((ADDR_DPS_REC_START+DPS_REC_LEN*(RECNUM))+1, RRR_Buf, RRR_Len-2);	//minus SW1,SW2
					break;
				default:   // out of spec
					return FAIL;
			}
		}
	}
	return SUCCESS;
}

UCHAR DPS_RAD_Process(void)
{
	UINT	tag94_len = 0, i;
	UCHAR	SFINUM = 0,rspCode, AFL_B4 = 0, j;
	UCHAR	*ptrData = NULLPTR, *ptrData1 = NULLPTR, *ptrData2 = NULLPTR;
	UCHAR	lenOfL = 0;
	UINT	lenOfV = 0;
	UCHAR	*odaRecAddr = NULLPTR;
	UINT	odaRecLen = 0;
	UCHAR	tag70[1] = {0x70}, tag56[1] = {0x56}, tag57[1] = {0x57}, tag9F7E[2] = {0x9F,0x7E}, tag9F80[2] = {0x9F,0x80};

	DBG_Put_String(31, (UCHAR*)"==== Read Application Data ====");	// ==== [Debug] ====

	ODA_Clear_Record();
	odaRecAddr = &oda_bufRecord[2];

	UT_Get_TLVLengthOfV(glv_tag94.Length, &tag94_len);

	//Step 1 : Application File Locator (AFL)
	if(tag94_len == 0)
	{
		DPS_DBG_Put_Process(DPS_FIGURE_18, 1, FALSE);

		return SUCCESS;
	}

	DPS_DBG_Put_Process(DPS_FIGURE_18, 1, TRUE);

	//Step 2 : Read all entries defined in the AFL
	for(i = 0 ; i < tag94_len ; i += 4)
	{
		SFINUM = glv_tag94.Value[i];
		SFINUM = SFINUM>>3;
		AFL_B4 = glv_tag94.Value[i+3];

		for(j = glv_tag94.Value[i+1] ; j <= glv_tag94.Value[i+2] ; j++)	//First Record(Byte2) and Last Record(Byte 3)
		{
			//Reset the Read Record Buff
			dps_RRBuffLen = 0;
			memset(dps_RRBuff, 0, DEP_BUFFER_SIZE_RECEIVE);

			//Send READ RECORD Command and Receive
			rspCode = ECL_APDU_Read_Record(j, SFINUM, &dps_RRBuffLen, dps_RRBuff);

			if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
				etp_flgCmdTimeout = TRUE;

			if(rspCode == ECL_LV1_SUCCESS)
			{
				rspCode = UT_Check_SW12(&dps_RRBuff[dps_RRBuffLen-2], STATUSWORD_9000);
				if(rspCode != TRUE)
					return FAIL;
				
				if(AFL_B4)	//Number of Records Involved in Offline Data Authentication (ODA)
				{
					//SFINUM : Byte1
					//RECNUM : record �Ӽ��`�p
					//dps_RRBuff : Read Record Response Buffer(���tSW1�BSW2)
					//DPS_RAD_SAD_Data(SFINUM, RECNUM, dps_RRBuffLen, dps_RRBuff);	//save application data
					//AFL_B4--;
					//RECNUM++;

					//					-------------------
					//oda_bufRecord[] = | Record Template |
					//                  -------------------
					if(SFINUM < 11)
					{
						ptrData = UT_Find_Tag(tag70, (dps_RRBuffLen - 2), dps_RRBuff);
						ptrData++;
						UT_Get_TLVLengthOfL(ptrData, &lenOfL);
						UT_Get_TLVLengthOfV(ptrData, &lenOfV);

						memcpy(odaRecAddr, &ptrData[lenOfL], lenOfV);

						odaRecAddr += lenOfV;
						odaRecLen += lenOfV;
						UT_S2C(odaRecLen, oda_bufRecord);
					}
					//					------------------------------------
					//oda_bufRecord[] = | Tag70 | Length | Record Template |
					//                  ------------------------------------
					else
					{
						odaRecLen += (dps_RRBuffLen - 2);
						UT_S2C(odaRecLen, oda_bufRecord);
						memcpy(odaRecAddr, dps_RRBuff, dps_RRBuffLen - 2);
						odaRecAddr += (dps_RRBuffLen - 2);
					}
					
					AFL_B4--;
				}

				rspCode = DPS_Check_RR_R_Tag(SFINUM, dps_RRBuffLen, dps_RRBuff);
				if(rspCode == FAIL)
				{
					return FAIL;
				}
			}
			else if((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))	//Timeout 
				return 0xFE;
			else if(rspCode == ECL_LV1_STOP_ICCARD)	//contact interrupt
				return 0xFF;
			else
				return rspCode;
		}
	}

	DPS_DBG_Put_Process(DPS_FIGURE_18, 2, 0xFF);

	//Step 3 : ZIP AID or Magstripe Mode
	if((DPS_Check_ZIP_AID() == SUCCESS) || (MS_Mode == TRUE))	//Legacy Mode or MS Mode
	{
		DPS_DBG_Put_Process(DPS_FIGURE_18, 3, TRUE);

		//Save Track 1 data
		ptrData = UT_Find_Tag(tag56, (dps_RRBuffLen - 2), dps_RRBuff);
		if(ptrData != NULLPTR)
		{
			ptrData++;
			UT_Get_TLVLengthOfL(ptrData, &lenOfL);
			UT_Get_TLVLengthOfV(ptrData, &lenOfV);
			UT_Set_TagLength(lenOfV, glv_tag56.Length);
			memcpy(glv_tag56.Value, &ptrData[lenOfL], lenOfV);
		}

		//Save Track 2 data
		ptrData = UT_Find_Tag(tag57, (dps_RRBuffLen - 2), dps_RRBuff);
		if(ptrData != NULLPTR)
		{
			ptrData++;
			UT_Get_TLVLengthOfL(ptrData, &lenOfL);
			UT_Get_TLVLengthOfV(ptrData, &lenOfV);
			UT_Set_TagLength(lenOfV, glv_tag57.Length);
			memcpy(glv_tag57.Value, &ptrData[lenOfL], lenOfV);
		}

		//Step 4 : Dynamic Card Verification Value (DCVV)
		ptrData1 = UT_Find_Tag(tag9F7E, (dps_RRBuffLen - 2), dps_RRBuff);
		ptrData2 = UT_Find_Tag(tag9F80, (dps_RRBuffLen - 2), dps_RRBuff);	
		if((ptrData1 != NULLPTR) || (ptrData2 != NULLPTR))	//DCVV present
		{
			DPS_DBG_Put_Process(DPS_FIGURE_18, 4, TRUE);

			if(ptrData1 != NULLPTR)
			{
				ptrData1 += 2;
				UT_Get_TLVLengthOfL(ptrData1, &lenOfL);
				UT_Get_TLVLengthOfV(ptrData1, &lenOfV);
				UT_Set_TagLength(lenOfV, glv_tag9F7E.Length);
				memcpy(glv_tag9F7E.Value, &ptrData1[lenOfL], lenOfV);
			}
			else if(ptrData2 != NULLPTR)
			{
				ptrData2 += 2;
				UT_Get_TLVLengthOfL(ptrData2, &lenOfL);
				UT_Get_TLVLengthOfV(ptrData2, &lenOfV);
				UT_Set_TagLength(lenOfV, glv_tag9F80.Length);
				memcpy(glv_tag9F80.Value, &ptrData2[lenOfL], lenOfV);
			}

			//Step 5 : Insert UN and DCVV into Track 1 and Track 2 data
			DPS_Create_MSDTrack();
			DPS_DBG_Put_Process(DPS_FIGURE_18, 5, 0xFF);
		}
		else
		{
			//DCVV is disabled
			DPS_Create_MSDTrackNoDCVV();

			DPS_DBG_Put_Process(DPS_FIGURE_18, 4, FALSE);
		}
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_18, 3, FALSE);
	}

	return SUCCESS;
}

UCHAR DPS_CDA_Checks(void)
{
	UCHAR	*ptrData = NULLPTR;
	UCHAR	tag94[1] = {0x94}, tag9F4B[2] = {0x9F, 0x4B}, tag9F26[2] = {0x9F, 0x26}, tagD1[1] = {0xD1};
	UINT	lenOfV = 0;

	DBG_Put_String(20, (UCHAR*)"==== CDA Checks ====");	// ==== [Debug] ====

	//Step 1 : Application File Locator (AFL)
	ptrData = UT_Find_Tag(tag94, (dps_GPOBuffLen-2), dps_GPOBuff);
	if(ptrData != NULLPTR)	//AFL present
	{
		ptrData += 1;
		UT_Get_TLVLengthOfV(ptrData, &lenOfV);
		if((lenOfV % 4) != 0)
		{
			DPS_DBG_Put_Process(DPS_FIGURE_17, 1, FALSE);

			glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction
			
			//Show TVR and TSI value (for test)
			DBG_Put_String(4, (UCHAR*)"TVR:");
			DBG_Put_Hex(5, glv_tag95.Value);

			DBG_Put_String(4, (UCHAR*)"TSI:");
			DBG_Put_Hex(2, glv_tag9B.Value);
			
			return FAIL;
		}

		DPS_DBG_Put_Process(DPS_FIGURE_17, 1, TRUE);
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_17, 1, FALSE);

		glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction

		//Show TVR and TSI value (for test)
		DBG_Put_String(4, (UCHAR*)"TVR:");
		DBG_Put_Hex(5, glv_tag95.Value);

		DBG_Put_String(4, (UCHAR*)"TSI:");
		DBG_Put_Hex(2, glv_tag9B.Value);

		return FAIL;
	}

	//Step 2 : Signed Dynamic Application Data (SDAD)
	ptrData = UT_Find_Tag(tag9F4B, (dps_GPOBuffLen-2), dps_GPOBuff);
	if(ptrData == NULLPTR)	//SDAD not present
	{
		DPS_DBG_Put_Process(DPS_FIGURE_17, 2, FALSE);

		glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction

		//Show TVR and TSI value (for test)
		DBG_Put_String(4, (UCHAR*)"TVR:");
		DBG_Put_Hex(5, glv_tag95.Value);

		DBG_Put_String(4, (UCHAR*)"TSI:");
		DBG_Put_Hex(2, glv_tag9B.Value);

		return FAIL;
	}

	DPS_DBG_Put_Process(DPS_FIGURE_17, 2, TRUE);

	//Step 3 : Application Cryptogram (AC)
	ptrData = UT_Find_Tag(tag9F26, (dps_GPOBuffLen-2), dps_GPOBuff);
	if(ptrData != NULLPTR)	//AC present
	{
		DPS_DBG_Put_Process(DPS_FIGURE_17, 3, TRUE);

		//[Contactless D-PAS Table 16] - TC with and AAC with CDA for Loyalty only
		if((dps_TC == 1) || ((dps_AAC == 1) && ((glv_tag9F71.Value[0] & DPS_CPR_PID_LIMIT_REACHED_LOYALTY_TRANSACTION_APPROVED) != 0)))
		{
			DBG_Put_String(24, (UCHAR*)"AC should not be present");	// ==== [Debug] ====
			return FAIL;
		}
	}

	DPS_DBG_Put_Process(DPS_FIGURE_17, 3, FALSE);

	//Step 4 : Offline Balance
	ptrData = UT_Find_Tag(tagD1, (dps_GPOBuffLen-2), dps_GPOBuff);
	if(ptrData != NULLPTR)	//Offline Balance present
	{
		DPS_DBG_Put_Process(DPS_FIGURE_17, 4, TRUE);

		//Step 5
		ptrData += 1;
		UT_Get_TLVLengthOfV(ptrData, &lenOfV);
		if(lenOfV == 6)
		{
			//Show TVR and TSI value (for test)
			DBG_Put_String(4, (UCHAR*)"TVR:");
			DBG_Put_Hex(5, glv_tag95.Value);

			DBG_Put_String(4, (UCHAR*)"TSI:");
			DBG_Put_Hex(2, glv_tag9B.Value);

			DPS_DBG_Put_Process(DPS_FIGURE_17, 5, TRUE);

			return SUCCESS;
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_17, 5, FALSE);
			return FAIL;
		}
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_17, 4, FALSE);

		return SUCCESS;
	}
	
	return FAIL;
}

UCHAR DPS_Non_CDA_Checks(void)
{
	UCHAR	rspCode = 0;
	UCHAR	*ptrData = NULLPTR;
	UCHAR	tag9F27[2] = {0x9F, 0x27}, tag9F71[2] = {0x9F, 0x71}, tag9F26[2] = {0x9F, 0x26}, tag9F4B[2] = {0x9F, 0x4B}, tagD1[1] = {0xD1}, tag94[1] = {0x94},
			tag57[1] = {0x57}, tag5F34[2] = {0x5F, 0x34}, tag9F07[2] = {0x9F, 0x07}, tag5F25[2] = {0x5F, 0x25}, tag9F08[2] = {0x9F, 0x08};
	UCHAR	lenOfT = 0;
	UCHAR	lenOfL = 0;
	UINT	lenOfV = 0;
	UCHAR	tmp_value1 = 0, tmp_value2 = 0;

	DBG_Put_String(24, (UCHAR*)"==== Non CDA Checks ====");	// ==== [Debug] ====

	//Step 1
	//Cryptogram Information Data (CID)
	ptrData = UT_Find_Tag(tag9F27, (dps_GPOBuffLen-2), dps_GPOBuff);
	if(ptrData != NULLPTR)	//CID present
	{
		rspCode = UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		if(rspCode == SUCCESS)
		{
			UT_Set_TagLength(lenOfV, glv_tag9F27.Length);
			memcpy(glv_tag9F27.Value, &ptrData[lenOfT+lenOfL], lenOfV);

			tmp_value1 = glv_tag9F27.Value[0];
			tmp_value1 &= 0xC0;	//Check CID B1b8-7

			if(tmp_value1 == 0x00)
			{
				dps_AAC = 1;
				DBG_Put_String(3, (UCHAR*)"AAC");	// ==== [Debug] ====
			}
			else if(tmp_value1 == 0x40)
			{
				dps_TC = 1;
				DBG_Put_String(2, (UCHAR*)"TC");	// ==== [Debug] ====
			}
			else if(tmp_value1 == 0x80)
			{
				dps_ARQC = 1;
				DBG_Put_String(4, (UCHAR*)"ARQC");	// ==== [Debug] ====
			}
		}
	}

	//Card Processing Requirement (CPR)
	ptrData = UT_Find_Tag(tag9F71, (dps_GPOBuffLen-2), dps_GPOBuff);
	if(ptrData != NULLPTR)	//CPR present
	{
		rspCode = UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		if(rspCode == SUCCESS)
		{
			UT_Set_TagLength(lenOfV, glv_tag9F71.Length);
			memcpy(glv_tag9F71.Value, &ptrData[lenOfT+lenOfL], lenOfV);
			tmp_value2 = glv_tag9F71.Value[0];
			tmp_value2 &= 0x20;	//Check CPR B1b6
		}
	}

	//Transaction Certificate(TC) or the Program Identifier (PID) Limit is reached
	if((dps_TC == 1) || (tmp_value2 == 0x20))
	{
		DPS_DBG_Put_Process(DPS_FIGURE_16, 1, TRUE);

		rspCode = DPS_CDA_Checks();
		return rspCode;
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_16, 1, FALSE);

		//Step 2 : Find AC[8]
		ptrData = UT_Find_Tag(tag9F26, (dps_GPOBuffLen-2), dps_GPOBuff);
		if(ptrData != NULLPTR)	//AC present
		{
			ptrData += 2;
			UT_Get_TLVLengthOfV(ptrData, &lenOfV);
			if(lenOfV != 8)
			{
				DPS_DBG_Put_Process(DPS_FIGURE_16, 2, FALSE);

				glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction
				
				//Show TVR and TSI value (for test)
				DBG_Put_String(4, (UCHAR*)"TVR:");
				DBG_Put_Hex(5, glv_tag95.Value);

				DBG_Put_String(4, (UCHAR*)"TSI:");
				DBG_Put_Hex(2, glv_tag9B.Value);

				return FAIL;
			}

			DPS_DBG_Put_Process(DPS_FIGURE_16, 2, TRUE);
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_16, 2, FALSE);

			glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction

			//Show TVR and TSI value (for test)
			DBG_Put_String(4, (UCHAR*)"TVR:");
			DBG_Put_Hex(5, glv_tag95.Value);

			DBG_Put_String(4, (UCHAR*)"TSI:");
			DBG_Put_Hex(2, glv_tag9B.Value);

			return FAIL;
		}

		//Step 3 : Signed Dynamic Application Data (SDAD)
		ptrData = UT_Find_Tag(tag9F4B, (dps_GPOBuffLen-2), dps_GPOBuff);
		if(ptrData != NULLPTR)	//SDAD present
		{
			DPS_DBG_Put_Process(DPS_FIGURE_16, 3, TRUE);

			return FAIL;
		}

		DPS_DBG_Put_Process(DPS_FIGURE_16, 3, FALSE);

		//Step 4 : Offline Balance
		ptrData = UT_Find_Tag(tagD1, (dps_GPOBuffLen-2), dps_GPOBuff);
		if(ptrData != NULLPTR)	//Offline Balance present
		{
			ptrData += 1;
			UT_Get_TLVLengthOfV(ptrData, &lenOfV);
			if(lenOfV != 6)
			{
				DPS_DBG_Put_Process(DPS_FIGURE_16, 4, TRUE);

				return FAIL;
			}
		}

		DPS_DBG_Put_Process(DPS_FIGURE_16, 4, FALSE);

		//Step 5 : Application File Locator (AFL)
		ptrData = UT_Find_Tag(tag94, (dps_GPOBuffLen-2), dps_GPOBuff);
		if(ptrData == NULLPTR)	//AFL not present
		{
			DPS_DBG_Put_Process(DPS_FIGURE_16, 5, FALSE);

			//Step 7 : Track 2 Equivalent Data
			ptrData = UT_Find_Tag(tag57, (dps_GPOBuffLen-2), dps_GPOBuff);
			if(ptrData != NULLPTR)	//Track 2 Equivalent Data present
			{
				ptrData += 1;
				UT_Get_TLVLengthOfV(ptrData, &lenOfV);
				if(lenOfV < 20)
				{
					DPS_DBG_Put_Process(DPS_FIGURE_16, 7, TRUE);

					//Step 8 : PAN Sequence Number
					ptrData = UT_Find_Tag(tag5F34, (dps_GPOBuffLen-2), dps_GPOBuff);
					if(ptrData != NULLPTR)	//PAN Sequence Number present
					{
						ptrData += 2;
						UT_Get_TLVLengthOfV(ptrData, &lenOfV);
						if(lenOfV == 1)
						{
							DPS_DBG_Put_Process(DPS_FIGURE_16, 8, TRUE);

							//Step 9 : Application Usage Control
							ptrData = UT_Find_Tag(tag9F07, (dps_GPOBuffLen-2), dps_GPOBuff);
							if(ptrData != NULLPTR)	//Application Usage Control present
							{
								ptrData += 2;
								UT_Get_TLVLengthOfV(ptrData, &lenOfV);
								if(lenOfV == 2)
								{
									DPS_DBG_Put_Process(DPS_FIGURE_16, 9, TRUE);

									//Step 10 : Application Effective Date
									ptrData = UT_Find_Tag(tag5F25, (dps_GPOBuffLen-2), dps_GPOBuff);
									if(ptrData != NULLPTR)	//Application Effective Date present
									{
										ptrData += 2;
										UT_Get_TLVLengthOfV(ptrData, &lenOfV);
										if(lenOfV == 3)
										{
											DPS_DBG_Put_Process(DPS_FIGURE_16, 10, TRUE);

											//Step 11 : Application Version Number
											ptrData = UT_Find_Tag(tag9F08, (dps_GPOBuffLen-2), dps_GPOBuff);
											if(ptrData != NULLPTR)	//Application Version Number present
											{
												ptrData += 2;
												UT_Get_TLVLengthOfV(ptrData, &lenOfV);
												if(lenOfV == 2)
												{
													DPS_DBG_Put_Process(DPS_FIGURE_16, 11, TRUE);

													return SUCCESS;	//Go to next transaction step
												}
												else
												{
													DPS_DBG_Put_Process(DPS_FIGURE_16, 11, FALSE);

													glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction

													//Show TVR and TSI value (for test)
													DBG_Put_String(4, (UCHAR*)"TVR:");
													DBG_Put_Hex(5, glv_tag95.Value);

													DBG_Put_String(4, (UCHAR*)"TSI:");
													DBG_Put_Hex(2, glv_tag9B.Value);

													return FAIL;
												}
											}
											else
											{
												DPS_DBG_Put_Process(DPS_FIGURE_16, 11, FALSE);

												glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction

												//Show TVR and TSI value (for test)
												DBG_Put_String(4, (UCHAR*)"TVR:");
												DBG_Put_Hex(5, glv_tag95.Value);

												DBG_Put_String(4, (UCHAR*)"TSI:");
												DBG_Put_Hex(2, glv_tag9B.Value);

												return FAIL;
											}
										}
										else
										{
											DPS_DBG_Put_Process(DPS_FIGURE_16, 10, FALSE);

											glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction
											
											//Show TVR and TSI value (for test)
											DBG_Put_String(4, (UCHAR*)"TVR:");
											DBG_Put_Hex(5, glv_tag95.Value);

											DBG_Put_String(4, (UCHAR*)"TSI:");
											DBG_Put_Hex(2, glv_tag9B.Value);

											return FAIL;
										}
									}
									else
									{
										DPS_DBG_Put_Process(DPS_FIGURE_16, 10, FALSE);

										glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction
										
										//Show TVR and TSI value (for test)
										DBG_Put_String(4, (UCHAR*)"TVR:");
										DBG_Put_Hex(5, glv_tag95.Value);

										DBG_Put_String(4, (UCHAR*)"TSI:");
										DBG_Put_Hex(2, glv_tag9B.Value);

										return FAIL;
									}
								}
								else
								{
									DPS_DBG_Put_Process(DPS_FIGURE_16, 9, FALSE);

									glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction

									//Show TVR and TSI value (for test)
									DBG_Put_String(4, (UCHAR*)"TVR:");
									DBG_Put_Hex(5, glv_tag95.Value);

									DBG_Put_String(4, (UCHAR*)"TSI:");
									DBG_Put_Hex(2, glv_tag9B.Value);

									return FAIL;
								}
							}
							else
							{
								DPS_DBG_Put_Process(DPS_FIGURE_16, 9, FALSE);

								glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction

								//Show TVR and TSI value (for test)
								DBG_Put_String(4, (UCHAR*)"TVR:");
								DBG_Put_Hex(5, glv_tag95.Value);

								DBG_Put_String(4, (UCHAR*)"TSI:");
								DBG_Put_Hex(2, glv_tag9B.Value);

								return FAIL;
							}
						}
						else
						{
							DPS_DBG_Put_Process(DPS_FIGURE_16, 8, FALSE);

							glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction

							//Show TVR and TSI value (for test)
							DBG_Put_String(4, (UCHAR*)"TVR:");
							DBG_Put_Hex(5, glv_tag95.Value);

							DBG_Put_String(4, (UCHAR*)"TSI:");
							DBG_Put_Hex(2, glv_tag9B.Value);

							return FAIL;
						}
					}
					else
					{
						DPS_DBG_Put_Process(DPS_FIGURE_16, 8, FALSE);

						glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction

						//Show TVR and TSI value (for test)
						DBG_Put_String(4, (UCHAR*)"TVR:");
						DBG_Put_Hex(5, glv_tag95.Value);

						DBG_Put_String(4, (UCHAR*)"TSI:");
						DBG_Put_Hex(2, glv_tag9B.Value);

						return FAIL;
					}
				}
				else
				{
					DPS_DBG_Put_Process(DPS_FIGURE_16, 7, FALSE);

					glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction

					//Show TVR and TSI value (for test)
					DBG_Put_String(4, (UCHAR*)"TVR:");
					DBG_Put_Hex(5, glv_tag95.Value);

					DBG_Put_String(4, (UCHAR*)"TSI:");
					DBG_Put_Hex(2, glv_tag9B.Value);

					return FAIL;
				}
			}
			else
			{
				DPS_DBG_Put_Process(DPS_FIGURE_16, 7, FALSE);

				glv_tag95.Value[0] |= 0x20;	//Set TVR B1b6 to ��1�� and end the transaction

				//Show TVR and TSI value (for test)
				DBG_Put_String(4, (UCHAR*)"TVR:");
				DBG_Put_Hex(5, glv_tag95.Value);

				DBG_Put_String(4, (UCHAR*)"TSI:");
				DBG_Put_Hex(2, glv_tag9B.Value);

				return FAIL;
			}
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_16, 5, TRUE);

			//Step 6 : Application File Locator (AFL) length
			ptrData += 1;
			UT_Get_TLVLengthOfV(ptrData, &lenOfV);
			if((lenOfV % 4) == 0)
			{
				DPS_DBG_Put_Process(DPS_FIGURE_16, 6, TRUE);

				return SUCCESS;	//go to next transaction step
			}
			else
			{
				DPS_DBG_Put_Process(DPS_FIGURE_16, 6, FALSE);

				return FAIL;
			}
		}
	}

	return SUCCESS;
}

void DPS_Clear_Buffer(void)
{
	dps_rcvLen = 0;
	memset(dps_rcvBuff, 0, DEP_BUFFER_SIZE_RECEIVE);
}

UCHAR DPS_Select_ZIP_AID(void)
{
	UCHAR	rspCode = 0;
	UCHAR 	cmdZIP[13] = {	
				0x00,	//CLA
				0xA4,	//INS
				0x04,	//P1
				0x00,	//P2
				0x07,	//Lc
				0xA0, 0x00, 0x00, 0x03, 0x24, 0x10, 0x10,	//Data
				0x00};	//Le
	
	DPS_Clear_Buffer();

	rspCode = ECL_LV1_DEP(13, cmdZIP, &etp_rcvLen, etp_rcvData, 0);

	return rspCode;
}

void DPS_Approved_OutCome(void)
{
	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
//	etp_Outcome.CVM				= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_Yes;

	if(etp_Outcome.CVM == ETP_OCP_CVM_ObtainSignature)
	{
		etp_Outcome.ocmMsgID = ETP_OCP_UIM_ApprovedPleaseSign;
	}
	else
	{
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_Approved;
	}
	
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_CardReadSuccessfully;
	etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_No;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_Yes;
	etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;
}

void DPS_Online_Request_OutCome(void)
{
	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
//	etp_Outcome.CVM				= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_Yes;
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_AuthorisingPleaseWait;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_CardReadSuccessfully;
	etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_No;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_Yes;
	etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;
}

void DPS_Online_Request_For_Two_Presentments(void)
{
	etp_Outcome.Start			= ETP_OCP_Start_B;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
//	etp_Outcome.CVM				= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_Yes;
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_AuthorisingPleaseWait;
	etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_Yes;
	etp_Outcome.rstMsgID		= ETP_OCP_UIM_PresentCardAgain;
	etp_Outcome.rstStatus		= ETP_OCP_UIS_ReadyToRead;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_Yes;
	etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;
}

void DPS_Declined_OutCome(void)
{
	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM				= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_Yes;
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_NotAuthorised;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_CardReadSuccessfully;
	etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_No;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_Yes;
	etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;
}

void DPS_Try_Another_Interface_OutCome(void)
{
	L3_Response_Code = VAP_RIF_RC_OTHER_INTERFACE;

	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM				= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_Yes;
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PleaseInsertOrSwipeCard;
	//etp_Outcome.ocmStatus		= ETP_OCP_UIS_ReadyToRead;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_CardReadSuccessfully;
	etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_No;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
	etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_ContactChip;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

//For Termination of First presentment or following an Online Request with ��Two presentment�� or unrecoverable error
void DPS_End_Application_OutCome(void)
{
	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;
	etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_No;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
	etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

//For Processing Error : Conditions for use of contactless not satisfied
void DPS_End_Application_For_Processing_Error(void)
{
	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_Yes;
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
	etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_No;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
	etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

void DPS_Try_Again_OutCome(void)
{
	etp_flgComError = TRUE;

	etp_Outcome.Start			= ETP_OCP_Start_B;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_Yes;
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_SeePhoneForInstructions;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
	etp_Outcome.hldTime			= 13;
	etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_Yes;
	etp_Outcome.rstStatus		= ETP_OCP_UIS_ReadyToRead;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
	etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= 13;
	etp_Outcome.rmvTimeout		= 0;
}

void DPS_Select_Next_OutCome(void)
{
	//remarked by Tammy 2018/01/05
	//etp_flgRestart = TRUE;
	ETP_Remove_CandidateList();
	
	etp_Outcome.Start			= ETP_OCP_Start_C;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;
	etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_No;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
	etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

void DPS_Try_Another_Card_OutCome(void)
{
	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_Yes;
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_ReadyToRead;
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= FALSE;
	etp_Outcome.dscData			= FALSE;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

UCHAR DPS_Verify_AFL( UCHAR *AFL_Data, UINT AFL_Length)
{
	UCHAR	i;
	UCHAR	sfi;
	UCHAR	st_rec_num;
	UCHAR	end_rec_num;
	UCHAR	oda_rec_cnt;

    // check all AFL entries
    for(i = 0 ; i < AFL_Length ; i += 4)
    {
       sfi = AFL_Data[i];
       st_rec_num = AFL_Data[i+1];
       end_rec_num = AFL_Data[i+2];
       oda_rec_cnt = AFL_Data[i+3];

       if( (sfi == 0) || ((sfi & 0x07) != 0) || ((sfi & 0xf8) == 0xf8) ||
           (st_rec_num == 0) || (end_rec_num < st_rec_num) ||
           ((end_rec_num - st_rec_num + 1) < oda_rec_cnt) )
       {	
         return FALSE ;
       }
    }

    return TRUE ;
}

UCHAR DPS_AIP_AFL_Process(void)
{	
	UCHAR	*data_point = 0;
	UCHAR	AIP[2] = {0};
	UCHAR	cnt, AFL_Data[252] = {0};
	UINT	Value_Len = 0, AFL_Length = 0;
	UCHAR	tag82[1] = {0x82}, tag94[1] = {0x94};

    UT_EMVCL_GetBERLEN(&dps_GPOBuff[1], &cnt ,&Value_Len); // get total length of data elements

	// check response format (2)
	if((dps_GPOBuff[0] == 0x77))
    {
		// find AIP[2]
		data_point = UT_Find_Tag(tag82, (dps_GPOBuffLen - 2), dps_GPOBuff);
	    if(data_point != 0)
	    {
			data_point++;
        	UT_EMVCL_GetBERLEN(data_point, &cnt ,&Value_Len);
            if(Value_Len == 0)
            	return FAIL; // AIP not present

			//ICTK, AIP length = 3, it should be terminated
			if(Value_Len != 2)
				return FAIL;
            // save AIP[2]
            AIP[0] = *(data_point+cnt);
            AIP[1] = *(data_point+cnt+1);

			//�x�stag 82
			//glv_tag82.Length[0] = 0x02;
			//memcpy(glv_tag82.Value,AIP,2);
		}
        else
            return FAIL; // AIP not present

        // find AFL[4n]
		data_point = UT_Find_Tag(tag94, (dps_GPOBuffLen - 2), dps_GPOBuff);
		if(data_point != 0)
		{
			data_point++;
			UT_EMVCL_GetBERLEN(data_point, &cnt, &Value_Len);

			// save AFL[4n]
			if((Value_Len % 4) == 0)
			{
				AFL_Length = Value_Len;
				memcpy(AFL_Data, &data_point[cnt], AFL_Length);

				//To verify AFL before actual reading records
				if(DPS_Verify_AFL(AFL_Data, AFL_Length) == FALSE)
				{
					return FAIL;
				}
			}
		}
    }
	else
	{
		return FAIL;
	}

	return SUCCESS;
}

UCHAR DPS_Check_GPO_R_Data(UINT GPOR_Len, UCHAR *GPOR_Buf)
{
	UCHAR	TLength, LLength = 0, *value_addr = 0, *len_addr = 0, rspCode, redundant = 0;
	ULONG	Index;
	UINT	tmplen = 0, VLength = 0;

	while(tmplen != (GPOR_Len-2))
	{
		TLength = LLength = VLength = 0;
		Index = 0;
		
		//Get first byte tag to check
		//is tag a Primitive data element ?
		if((*GPOR_Buf & 0x20) && (*GPOR_Buf != 0xFF))		//it's a Constructed data element
		{	
			UT_Get_TLVLengthOfT(GPOR_Buf, &TLength);	//get tag's Length
			UT_Get_TLVLengthOfL(&GPOR_Buf[TLength], &LLength);	//get length's Length
			UT_Get_TLVLengthOfV(&GPOR_Buf[TLength], &VLength);	//get Value's Length

			//ICTK check GPO Length
			if(!((GPOR_Buf[TLength+LLength+VLength] == 0x90) && (GPOR_Buf[TLength+LLength+VLength+1] == 0x00)))
			{
				return FAIL;
			}
			
			tmplen += TLength;	
			GPOR_Buf += TLength;			
		
			tmplen += LLength;
			GPOR_Buf += LLength;			//shift buf pointer, and now we are in "Constructed data" field
		}
		else if((*GPOR_Buf == 0x00)||(*GPOR_Buf == 0xFF))			//ICTK, padding with 0x00
		{
			tmplen += 1;	
			GPOR_Buf += 1;
		}
		else	//this tag is a Primitive data element
		{
			UT_Get_TLVLengthOfT(GPOR_Buf, &TLength);	//get tag's Length

			rspCode = UT_Search(TLength,GPOR_Buf, (UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &Index);	//search glv_tagTable and get the index 
			if(rspCode == SUCCESS)
			{	
				len_addr = (UCHAR *)(glv_addTable[Index]);	//point to glv_addtable length addres

					tmplen += TLength;
					GPOR_Buf += TLength;			//shift buf pointer, and now we are in "length" field

					UT_Get_TLVLengthOfL(GPOR_Buf, &LLength);				//get length's Length
					UT_Get_TLVLengthOfV(GPOR_Buf, &VLength);				//get Value's Length
					tmplen += LLength;
					tmplen += VLength;
					
					while(LLength--)
					{
						*len_addr++ = *GPOR_Buf++;		//shift buf pointer, and now we're in "value" field
					}
					value_addr = (UCHAR *)(glv_addTable[Index]+3);	//point to glv_addtable value address
					while(VLength--)	//copy data to glv_addTable
					{
						*value_addr++ = *GPOR_Buf++;		//shift buf pointer, and now we're in "tag" field
					}
			}
			else
			{
				tmplen += TLength;
				GPOR_Buf += TLength;			//shift buf pointer, and now we are in "length" field

				UT_Get_TLVLengthOfL(GPOR_Buf, &LLength);				//get length's Length
				UT_Get_TLVLengthOfV(GPOR_Buf, &VLength);				//get Value's Length
				tmplen += LLength;
				tmplen += VLength;

				GPOR_Buf += LLength;
				GPOR_Buf += VLength;
			}
		}		
	}

	if(redundant == 0x01)
		return FAIL;
	else 
		return SUCCESS;		
}

UCHAR DPS_Mandatory_Data_Checks(void)
{
	UCHAR	rspCode;
	UCHAR	*ptrData = NULLPTR;
	UCHAR	tag82[1] = {0x82}, tag94[1] = {0x94}, tag9F36[2] = {0x9F, 0x36}, tag9F10[2] = {0x9F, 0x10}, tag9F27[2] = {0x9F, 0x27}, 
		    tag9F71[2] = {0x9F, 0x71};
	UCHAR	lenOfT = 0;
	UINT	lenOfV = 0;

	if(UT_Check_SW12(&dps_GPOBuff[dps_GPOBuffLen - 2], STATUSWORD_9000))
	{
		DPS_DBG_Put_Process(DPS_FIGURE_15, 1, TRUE);

		//Check GPO response
		if(dps_GPOBuff[0] != 0x77)	//The data field returned in the GPO response message shall be coded according to EMV format 2 for both the D-PAS MS and EMV Modes.
		{
			return DPS_Select_Next;	//Step 10 : select Zip application
		}

		//Step 4 : Check format
		//[EMV Book 3 6.5.8.4]
		rspCode = DPS_AIP_AFL_Process();	//Check whether AIP and AFL are included
		if(rspCode == SUCCESS)
		{
			DPS_DBG_Put_Process(DPS_FIGURE_15, 4, TRUE);

			rspCode = DPS_Check_GPO_R_Data(dps_GPOBuffLen, dps_GPOBuff);	//Check and store GPO response data
			if(rspCode == SUCCESS)
			{
				//Step 5 : Find AIP[2]
				ptrData = UT_Find_Tag(tag82, (dps_GPOBuffLen-2), dps_GPOBuff);
				if(ptrData != NULLPTR)
				{
					UT_Get_TLVLengthOfT(ptrData, &lenOfT);	//Get Tag's Length
					ptrData += lenOfT;	//Shift to "Length" field

					UT_Get_TLVLengthOfV(ptrData, &lenOfV);
					if(lenOfV != 2)
					{
						DPS_DBG_Put_Process(DPS_FIGURE_15, 5, FALSE);

						return DPS_Select_Next;	//Step 10 : Select Zip application
					}

					DPS_DBG_Put_Process(DPS_FIGURE_15, 5, TRUE);
				}
				else
				{
					DPS_DBG_Put_Process(DPS_FIGURE_15, 5, FALSE);

					return DPS_Select_Next;	//Step 10 : Select Zip application
				}
				
				//Update 14 in [Terminal Application Specification Bulletin CL TAS-002, v1.0]
				//Step 5a : Zip AID or Magstripe Mode
				if((DPS_Check_ZIP_AID() == SUCCESS) || (MS_Mode == TRUE))	//Legacy Mode or MS Mode
				{
					DPS_DBG_Put_Process(DPS_FIGURE_15, 0x5A, TRUE);

					//Update 14 in [Terminal Application Specification Bulletin CL TAS-002, v1.0]
					//Step 5b : Tag 94 present and length is multiple of 4
					ptrData = UT_Find_Tag(tag94, (dps_GPOBuffLen-2), dps_GPOBuff);
					if(ptrData != NULLPTR)	//AFL present
					{
						ptrData += 1;

						UT_Get_TLVLengthOfV(ptrData, &lenOfV);

						if((lenOfV % 4) == 0)
						{
							DPS_DBG_Put_Process(DPS_FIGURE_15, 0x5B, TRUE);

							return SUCCESS;
						}
						else
						{
							DPS_DBG_Put_Process(DPS_FIGURE_15, 0x5B, FALSE);

							//Check if it supports contact or magnetic stripe
							if(glv_tag9F66.Value[0] & 0x90)	//TTQ B1b8 : Magnetic stripe mode supported, B1b5 : EMV contact chip supported
							{
								DPS_DBG_Put_Process(DPS_FIGURE_15, 13, TRUE);

								return DPS_Try_Another;
							}
							else
							{
								DPS_DBG_Put_Process(DPS_FIGURE_15, 13, FALSE);

								return FAIL;
							}
						}
					}
					else
					{
						DPS_DBG_Put_Process(DPS_FIGURE_15, 0x5B, FALSE);

						//Check if it supports contact or magnetic stripe
						if(glv_tag9F66.Value[0] & 0x90)	//TTQ B1b8 : Magnetic stripe mode supported, B1b5 : EMV contact chip supported
						{
							DPS_DBG_Put_Process(DPS_FIGURE_15, 13, TRUE);

							return DPS_Try_Another;
						}
						else
						{
							DPS_DBG_Put_Process(DPS_FIGURE_15, 13, FALSE);

							return FAIL;
						}
					}
				}

				DPS_DBG_Put_Process(DPS_FIGURE_15, 0x5A, FALSE);

				//Step 6 : Find ATC and check length
				ptrData = UT_Find_Tag(tag9F36, (dps_GPOBuffLen-2), dps_GPOBuff);
				if(ptrData != NULLPTR)
				{
					UT_Get_TLVLengthOfT(ptrData, &lenOfT);	//Get Tag's Length
					ptrData += lenOfT;	//Shift to "Length" field

					UT_Get_TLVLengthOfV(ptrData, &lenOfV);
					if(lenOfV != 2)
					{
						DPS_DBG_Put_Process(DPS_FIGURE_15, 6, FALSE);

						return DPS_Select_Next;	//Step 10 : Select Zip application
					}

					DPS_DBG_Put_Process(DPS_FIGURE_15, 6, TRUE);
				}
				else
				{
					DPS_DBG_Put_Process(DPS_FIGURE_15, 6, FALSE);

					return DPS_Select_Next;	//Step 10 : Select Zip application
				}

				//Step 7 : Find IAD
				ptrData = UT_Find_Tag(tag9F10, (dps_GPOBuffLen-2), dps_GPOBuff);
				if(ptrData == NULLPTR)	//IAD not present
				{
					DPS_DBG_Put_Process(DPS_FIGURE_15, 7, FALSE);

					return DPS_Select_Next;	//Step 10 : Select Zip application
				}
				else
				{
					//Retrieve contactless CVR (Tag 9F53) from IAD (Tag 9F10) Byte 3 to Byte 10 for CVM Mobile process
					UT_Set_TagLength(8, glv_tag9F53.Length);
					memcpy(glv_tag9F53.Value, &glv_tag9F10.Value[2], 8);
				}

				DPS_DBG_Put_Process(DPS_FIGURE_15, 7, TRUE);

				//Step 8 : Find CID and check length
				ptrData = UT_Find_Tag(tag9F27, (dps_GPOBuffLen-2), dps_GPOBuff);
				if(ptrData != NULLPTR)
				{
					UT_Get_TLVLengthOfT(ptrData, &lenOfT);	//Get Tag's Length
					ptrData += lenOfT;	//Shift to "Length" field

					UT_Get_TLVLengthOfV(ptrData, &lenOfV);
					if(lenOfV != 1)
					{
						DPS_DBG_Put_Process(DPS_FIGURE_15, 8, FALSE);

						return DPS_Select_Next;	//Step 10 : Select Zip application
					}

					DPS_DBG_Put_Process(DPS_FIGURE_15, 8, TRUE);
				}
				else
				{
					DPS_DBG_Put_Process(DPS_FIGURE_15, 8, FALSE);

					return DPS_Select_Next;	//Step 10 : Select Zip application
				}

				//Step 9 : Find CPR and check length
				ptrData = UT_Find_Tag(tag9F71, (dps_GPOBuffLen-2), dps_GPOBuff);
				if(ptrData != NULLPTR)
				{
					UT_Get_TLVLengthOfT(ptrData, &lenOfT);	//Get Tag's Length
					ptrData += lenOfT;	//Shift to "Length" field

					UT_Get_TLVLengthOfV(ptrData, &lenOfV);
					if(lenOfV != 2)
					{
						DPS_DBG_Put_Process(DPS_FIGURE_15, 9, FALSE);

						return DPS_Select_Next;	//Step 10 : Select Zip application
					}

					DPS_DBG_Put_Process(DPS_FIGURE_15, 9, TRUE);
				}
				else
				{
					DPS_DBG_Put_Process(DPS_FIGURE_15, 9, FALSE);

					return DPS_Select_Next;	//Step 10 : Select Zip application
				}

				//Update 14 in [Terminal Application Specification Bulletin CL TAS-002, v1.0]
				//Step 9a : Set TVR B1b8 to 1
				glv_tag95.Value[0] |= DPS_TVR_OFFLINE_DATA_AUTHENTICATION_WAS_NOT_PERFORMED;
				DPS_DBG_Put_Process(DPS_FIGURE_15, 0x9A, 0xFF);

				rspCode = DPS_Non_CDA_Checks();
				return rspCode;
			}
			else
			{
				return DPS_Select_Next;	//Step 10 : Select Zip application
			}
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_15, 4, FALSE);

			return DPS_Select_Next;	//Step 10 : Select Zip application
		}
	}
	else if(UT_Check_SW12(&dps_GPOBuff[dps_GPOBuffLen - 2], STATUSWORD_6984))	//Try Another Interface
	{
		DPS_DBG_Put_Process(DPS_FIGURE_15, 1, FALSE);

		DPS_DBG_Put_Process(DPS_FIGURE_15, 2, TRUE);

		DPS_DBG_Put_Process(DPS_FIGURE_15, 3, 0xFF);

		return DPS_Try_Another;	//Step 3 : Ask user to insert or swipe the cards
	}
	else if(UT_Check_SW12(&dps_GPOBuff[dps_GPOBuffLen - 2], STATUSWORD_6986))	//Update 14 in [Terminal Application Specification Bulletin CL TAS-002, v1.0]
	{
		DPS_DBG_Put_Process(DPS_FIGURE_15, 1, FALSE);

		DPS_DBG_Put_Process(DPS_FIGURE_15, 2, FALSE);

		DPS_DBG_Put_Process(DPS_FIGURE_15, 0x3A, TRUE);
		
		return DPS_Try_Again;	//Step 3b : Terminate and Restart
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_15, 1, FALSE);

		DPS_DBG_Put_Process(DPS_FIGURE_15, 2, FALSE);

		DPS_DBG_Put_Process(DPS_FIGURE_15, 0x3A, FALSE);

		return DPS_Select_Next;	//Step 10 : Select Zip application
	}

	return DPS_Select_Next;
}

UCHAR DPS_Copy_CardResponse(UINT iptLen, UCHAR *iptData)
{
	UCHAR	flgConTag=FALSE;
	UCHAR	flgEncError=FALSE;
	UCHAR	flgPrimitive=FALSE;
	UCHAR	flgInTagTable=FALSE;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenInTerminal=0;
	UCHAR	rspCode=FAIL;
	UCHAR	*ptrData=NULLPTR;
	UINT	datLen=0;
	UINT	parLen=0;
	UINT	padLen=0;
	UINT	idxTag=0;
	UINT	lenOfV_Table=0;
	

	//Point to Start of Data
	ptrData=iptData;

	rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == FAIL)
	{
		return FAIL;
	}

	//Check If the TLV String is a Single Consructed Tag
//	flgConTag=apk_EMVCL_CheckConsTag(ptrData[0]);
	flgConTag=UT_Check_ConstructedTag(ptrData);
	if (flgConTag == TRUE)
	{
		ptrData+=(lenOfT+lenOfL);	//Point to Constructed Tag Value
		datLen=lenOfV;

		//Check Receive Length = Tag Length
		if (iptLen != (lenOfT+lenOfL+lenOfV))
		{
			return FAIL;
		}

		//Check Length = 0
		if (lenOfV == 0)
		{
			return SUCCESS;
		}
	}
	else
	{
		datLen=lenOfT+lenOfL+lenOfV;
	}

	//Check BER Coding, Single Constructed Format or Primitive Tag
	flgEncError=UT_Check_EMVTagEncodingError(datLen, ptrData);
	if (flgEncError == TRUE)
	{
		return FAIL;
	}

	do
	{
		//Reset Flag
		flgPrimitive=FALSE;
		
		//Check Padding
		if ((ptrData[0] == 0x00) || (ptrData[0] == 0xFF))
		{
			UT_Check_Padding(datLen, parLen, ptrData, &padLen);

			ptrData+=padLen;
			parLen+=padLen;
			continue;
		}

		//Get TLV Length
		rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		if (rspCode == FAIL)
		{
			return FAIL;	//TLV Error
		}

		//Check Primitive Tag
		flgPrimitive=UT_Check_PrimitiveTag(ptrData);
		if (flgPrimitive == TRUE)
		{
			flgInTagTable=UT_Search_Record(lenOfT, ptrData, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &idxTag);
			if (flgInTagTable == SUCCESS)
			{
				UT_Get_TLVLengthOfV((UCHAR*)glv_addTable[idxTag], &lenInTerminal);
				if (lenInTerminal != 0)
				{
					return FAIL;	//Redundant
				}

				lenOfV_Table=glv_tagTable[idxTag].MASTER_MaxLength[0]*256+glv_tagTable[idxTag].MASTER_MaxLength[1];
				if (lenOfV_Table == 0)	//Length Undefined
				{
					if (lenOfV <= 1024)
					{
						//Store Card Data
						UT_Set_TagLength(lenOfV, (UCHAR*)glv_addTable[idxTag]);
						memcpy((char*)(glv_addTable[idxTag]+3), &ptrData[lenOfT+lenOfL], lenOfV);
					}
				}
				else
				{
					if (lenOfV <= lenOfV_Table)
					{
						//Store Card Data
						UT_Set_TagLength(lenOfV, (UCHAR*)glv_addTable[idxTag]);
						memcpy((char*)(glv_addTable[idxTag]+3), &ptrData[lenOfT+lenOfL], lenOfV);
					}
				}
			}
		}

		ptrData+=(lenOfT+lenOfL+lenOfV);
		parLen+=(lenOfT+lenOfL+lenOfV);
	} while (parLen < datLen);

	return SUCCESS;
}

void DPS_ADF_Optional_Check(UINT lenFCI, UCHAR *datFCI)
{
	UCHAR	datBuffer[ETP_BUFFER_SIZE+2] = {0};
	UCHAR	*ptrData = NULLPTR;
	UCHAR	rspCode = 0;
	ULONG	int9F1B = 0;
	UCHAR	asc9F1B[10] = {0};
	UCHAR	bcd9F1B[5] = {0};
	UCHAR	trmFL[6] = {0};
	UCHAR	tag9F7D[2] = {0x9F,0x7D};

	//Step 12
	if(Issuer_Script_Data[0] != 0)
	{
		DPS_DBG_Put_Process(DPS_FIGURE_12, 12, TRUE);

		//Step 13
		flgIST = TRUE;

		DPS_DBG_Put_Process(DPS_FIGURE_12, 13, 0xFF);
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_12, 12, FALSE);

		//Step 14 : Reset TSI and TVR
		memset(glv_tag9B.Value, 0x00, 2);
		memset(glv_tag95.Value, 0x00, 5);
		DPS_DBG_Put_Process(DPS_FIGURE_12, 14, 0xFF);

		//Update 24 in [Terminal Application Specification Bulletin CL TAS-002, v1.0]
		//Step 15 : Transaction Amount > Floor Limit
		int9F1B = glv_tag9F1B.Value[0] * (1 << 24) + glv_tag9F1B.Value[1] * (1 << 16) + glv_tag9F1B.Value[2] * (1 << 8) + glv_tag9F1B.Value[3];
		UT_INT2ASC(int9F1B, asc9F1B);
		UT_Compress(bcd9F1B, asc9F1B, 5);
		memcpy(&trmFL[1], bcd9F1B, 5);

		rspCode = UT_bcdcmp(glv_tag9F02.Value, trmFL, 6);
		if(rspCode == 1)
		{
			DPS_DBG_Put_Process(DPS_FIGURE_12, 15, TRUE);

			//Step 15a
			glv_tag95.Value[3] |= DPS_TVR_TRANSACTION_EXCEEDS_FLOOR_LIMIT;

			DPS_DBG_Put_Process(DPS_FIGURE_12, 15, 0xFF);
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_12, 15, FALSE);
		}

		//Step 16
		if(glv_tag9F66.Value[0] & DPS_TTQ_MAGNETIC_STRIPE_MODE_SUPPORTED)
		{
			DPS_DBG_Put_Process(DPS_FIGURE_12, 16, TRUE);

			//Step 17
			datBuffer[0] = (lenFCI & 0x00FF);
			datBuffer[1] = (lenFCI & 0xFF00) >> 8;
			memcpy(&datBuffer[2], datFCI, lenFCI);

			ptrData = UT_Find_Tag(tag9F7D, lenFCI, &datBuffer[2]);
			if(ptrData != NULLPTR)
			{
				DPS_DBG_Put_Process(DPS_FIGURE_12, 17, TRUE);

				//Step 18
				DPS_DBG_Put_Process(DPS_FIGURE_12, 18, 0xFF);
			}
			else
			{
				DPS_DBG_Put_Process(DPS_FIGURE_12, 17, FALSE);
			}
		}
		else
		{
			DPS_DBG_Put_Process(DPS_FIGURE_12, 16, FALSE);
		}

		//Step 19
		DPS_DBG_Put_Process(DPS_FIGURE_12, 19, 0xFF);
	}
}

UCHAR DPS_Initiate_Process_GPO(UINT lenFCI, UCHAR *datFCI)
{
	UCHAR	rspCode;
	UINT	PDOL_Len;
	UCHAR	*ptrData = NULLPTR;
	UCHAR	lenOfL = 0;
	UINT	lenOfV = 0;
	UCHAR	tag9F37[2] = {0x9F, 0x37}, tag83[1] = {0x83};

	//PDOL = glv_tag9F38.Value;
	UT_Get_TLVLengthOfV(glv_tag9F38.Length, &PDOL_Len);
	
	//Set tag 9F37 length
	ptrData = UT_Find_TagInDOL(tag9F37, PDOL_Len, glv_tag9F38.Value);
	if(ptrData != NULLPTR)
	{
		UT_Set_TagLength(*(ptrData + 2), glv_tag9F37.Length);
	}

	//Handle PDOL Data
	rspCode = DPS_DOL_Get_DOLData(PDOL_Len, glv_tag9F38.Value, (UCHAR *)&dps_sendLen, dps_sendBuff);

	//Copy PDOL related data
	ptrData = UT_Find_Tag(tag83, dps_sendLen, dps_sendBuff);
	ptrData++;
	UT_Get_TLVLengthOfL(ptrData, &lenOfL);
	UT_Get_TLVLengthOfV(ptrData, &lenOfV);
	UT_Set_TagLength(lenOfV, glv_tagDF8111.Length);
	memcpy(glv_tagDF8111.Value, &ptrData[lenOfL], lenOfV);

	if(rspCode == SUCCESS)
	{	
		DPS_ADF_Optional_Check(lenFCI, datFCI);

		if(flgIST == TRUE)
		{
			//Send Script Command
			return DPS_IssuerUpdate;
		}
		else
		{
			//Send GPO Command
			rspCode = ECL_APDU_Get_ProcessingOptions(dps_sendLen, dps_sendBuff, &dps_rcvLen, dps_rcvBuff);

			if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
				etp_flgCmdTimeout = TRUE;
		
			if(rspCode == ECL_LV1_SUCCESS)
			{
				dps_GPOBuffLen = dps_rcvLen;
				memcpy(dps_GPOBuff, dps_rcvBuff, dps_rcvLen);
				DPS_Copy_CardResponse(dps_GPOBuffLen, dps_GPOBuff);	//Save GPO response TLV data

				ODA_Clear_GPOResponse();
				memcpy(oda_bufRspGPO, dps_GPOBuff, dps_GPOBuffLen - 2);	//Save GPO response for ODA
			}
			//else
			//	return rspCode;
			return rspCode;
		}
	}
	else
		return FAIL;

	return SUCCESS;
}

UCHAR DPS_Initiate_App_Process(UINT lenFCI, UCHAR *datFCI)
{
	UCHAR	rspCode = 0;
	UINT	tmpLEN = 0;

	DBG_Put_String(41, (UCHAR*)"==== Initiate Application Processing ====");	// ==== [Debug] ====

	//Reset GPO Buffer
	dps_GPOBuffLen = 0;
	memset(dps_GPOBuff,0x00,DEP_BUFFER_SIZE_RECEIVE);

	//Send GPO command
	rspCode = DPS_Initiate_Process_GPO(lenFCI, datFCI);

	//Timeout occur
	if((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
	{
		return 0xFE;
	}

	//Issuer Update Processing
	if(rspCode == DPS_IssuerUpdate)
	{
		return DPS_IssuerUpdate;
	}

	//Those of tag that will be updated by the terminal should set length.
	UT_Get_TLVLengthOfV(glv_tag95.Length, &tmpLEN);	//TVR		
	if(!tmpLEN)
		glv_tag95.Length[0] = 0x05;

	UT_Get_TLVLengthOfV(glv_tag9B.Length, &tmpLEN);	//TSI		
	if(!tmpLEN)
		glv_tag9B.Length[0] = 0x02;

	rspCode = DPS_Mandatory_Data_Checks();
		return rspCode;
}

void DPS_Start_Kernel(UINT lenFCI, UCHAR * datFCI)
{
	UCHAR	rspCode = 0;
	UCHAR	tag9F7D[2] = {0x9F,0x7D};
	UCHAR	*ptrData = NULLPTR;
	UINT	lenOf9F4B = 0;

	rspCode=DPS_Allocate_Buffer();
	if (rspCode == FAIL)
	{
		return;
	}

	ut_flgLenOfT_9F80 = TRUE;

	dps_flgKrnDiscover = TRUE;

	DPS_Load_TransactionData();

	//tag 9F7D present if the Issuer prefers to process a transaction using the D-PAS MS Mode
	ptrData = UT_Find_Tag(tag9F7D, lenFCI, datFCI);
	if(ptrData != NULLPTR)
	{
		MS_Mode = TRUE;
	}

	if(dps_flgZIPconfig)
	{
		SchemeID = VAP_Scheme_ZIP;
		ZIP_TransFlow();
		return;
	}

	if((DPS_Check_ZIP_AID() == SUCCESS) || (MS_Mode == TRUE))	//Legacy Mode or MS Mode
	{
		SchemeID = VAP_Scheme_DPAS_Magstripe;
	}
	else  //EMV Mode
	{
		SchemeID = VAP_Scheme_DPAS_EMV;
	}

	//Reset the indicator
	dps_Decline_Required_by_Reader = 0;
	dps_Online_Required_by_Reader = 0;

	/*if(((etp_Outcome.Start == ETP_OCP_Start_NA) && (etp_flgRestart == FALSE)) ||
		((etp_Outcome.Start == ETP_OCP_Start_B) && (etp_flgRestart == FALSE)) ||
		((etp_Outcome.Start == ETP_OCP_Start_C) && (etp_flgRestart == FALSE)))*/
	if((etp_Outcome.Start == ETP_OCP_Start_NA) || (etp_Outcome.Start == ETP_OCP_Start_B) || (etp_Outcome.Start == ETP_OCP_Start_C))
	{
		if(dps_Issuer_Update_Flag == TRUE)
		{	
			//Copy TVR and TSI to tag 95 and tag 9B for Issuer Update to use
			memcpy(glv_tag95.Length, dps_tagTVR.Length, (3 + ECL_LENGTH_95));
			memcpy(glv_tag9B.Length, dps_tagTSI.Length, (3 + ECL_LENGTH_9B));
		}

		rspCode = DPS_Initiate_App_Process(lenFCI, datFCI);

		switch(rspCode)
		{
			case SUCCESS:
				if(dps_Decline_Required_by_Reader == 1)
				{
					DPS_Declined_OutCome();
					break;
				}

				//Read Application Data
				rspCode = DPS_RAD_Process();
				if(rspCode == SUCCESS)
				{
					DPS_Retrieve_PAN_From_Track2();
					DPS_Retrieve_Expired_Date_From_Track2();

					//If the transaction is processed offline and the application has returned SDAD to the GPO command
					UT_Get_TLVLengthOfV(glv_tag9F4B.Length, &lenOf9F4B);
					if((dps_TC == 1) && (lenOf9F4B != 0))
					{
						DPS_ODA_Process();
					}

					rspCode = DPS_CVM_Common_Process();
					if(rspCode == SUCCESS)
					{
						rspCode = DPS_Processing_Restriction();
						if(rspCode == FAIL)
						{
							//DPS_End_Application_For_Processing_Error();	//Ask user to use another card
							DPS_End_Application_OutCome();
							break;
						}

						rspCode = DPS_TAA_Process_Part1();
						if(rspCode == SUCCESS)
						{
							DPS_OutcomeProcess();
							break;
						}
					}
					else if(rspCode == DPS_Try_Another)
					{
						DPS_Try_Another_Interface_OutCome();
						break;
					}
					//else if(rspCode == DPS_End_APP_Processing_Error)
					//{
					//	DPS_End_Application_For_Processing_Error();	//Ask user to use another card
					//	break;
					//}
					else if(rspCode == FAIL)
					{
						DPS_End_Application_OutCome();
						break;
					}
				}
				else if(rspCode == 0xFE)	//Read Record timeout
				{
					//If time out, reset the data that receive from GPO Response, Read Record Response
					ECL_Reset_Tag();
					ETP_Load_TransactionData();

					//Set Timeout Outcome
					ETP_Set_DefaultOutcome(ETP_OUTCOME_Timeout);
					
					ECL_LV1_RESET();
				}
				else if(rspCode == 0xFF)				//Contact interrupt
				{
					//Disable LED
					UT_Set_LEDSignal(IID_LED_YELLOW, 0, 0);				
					etp_Outcome.ocmMsgID = ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
				}
				else
				{
					DPS_End_Application_OutCome();
				}

				break;

			case DPS_Try_Another:
				DPS_Try_Another_Interface_OutCome();
				break;

			case DPS_Select_Next:
				DPS_Select_Next_OutCome();
				break;

			case DPS_Try_Again:
				DPS_Try_Again_OutCome();
				break;

			case FAIL:
				DPS_End_Application_OutCome();
				break;

			case DPS_IssuerUpdate:
				rspCode = DPS_Issuer_Update_Process();

				flgIST = FALSE;
				break;

			case 0xFE:	//Timeout
				ETP_Set_DefaultOutcome(ETP_OUTCOME_Timeout);
				ECL_LV1_RESET();
				break;

			case 0xFF:	//Contact interrupt
				etp_Outcome.ocmMsgID = ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
				break;
		}

		//Store TVR and TSI of 1st transaction for Issuer Update to use
		memcpy(dps_tagTVR.Length, glv_tag95.Length, (3 + ECL_LENGTH_95));
		memcpy(dps_tagTSI.Length, glv_tag9B.Length, (3 + ECL_LENGTH_9B));

		//Reset D-PAS MS Mode
		MS_Mode = FALSE;

		//Reset Cryptogram Type		
		dps_AAC = 0;
		dps_TC = 0;
		dps_ARQC = 0;

		//Reset the indicator
		dps_Decline_Required_by_Reader = 0;
		dps_Online_Required_by_Reader = 0;

		//Buffer reset
		memset(dps_RRBuff,0x00,DEP_BUFFER_SIZE_RECEIVE);
		dps_RRBuffLen = 0;

		memset(dps_GPOBuff,0x00,DEP_BUFFER_SIZE_RECEIVE);
		dps_GPOBuffLen = 0;

		memset(dps_rcvBuff,0x00,DEP_BUFFER_SIZE_RECEIVE);
		dps_rcvLen = 0;

		memset(dps_sendBuff,0x00,DEP_BUFFER_SIZE_SEND);
		dps_sendLen = 0;

		//Reset flag
		dps_flgIssuerUpdate = FALSE;

		ut_flgLenOfT_9F80 = FALSE;
	}

	DPS_Free_Buffer();
}
