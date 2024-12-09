#include <string.h>
#include "Glv_ReaderConfPara.h"
#include "VAP_ReaderInterface_Define.h"
#include "Function.h"
#include "FLSAPI.h"
#include "MIFARE_Define.h"
#include "api_pcd_vap_Command.h"
#include "ECL_Tag.h"
#include "JCB_Kernel_Define.h"
#include "ECL_LV1_Function.h"
#include "MPP_Define.h"
#include "AEX_Define.h"

#ifdef _PLATFORM_AS210
#include "XIOAPI.h"
#else
#include "LCDTFTAPI.h"
#endif

// Add by Wayne 2020/08/21 to avoid compiler warning
// <-----
#include <stdlib.h>
#include "UTILS_CTLS.H"
#include <stdio.h>
// <------


//20140114 V4, Print AOSA Problem
extern UCHAR VGL_Print_AOSA_Amount[14];
UCHAR api_Issuer_Update_Success_Flag_For_AOSA=FALSE;

//20140107
UCHAR api_InvalidateReader = FALSE;

//20140105 flag for Reader Index Changed?
extern UCHAR VAP_Reader_Index_Change;

//System Reader Index
extern volatile UCHAR	os_WAVE_TXI_H;// default RXI
extern volatile UCHAR	os_WAVE_TXI_L;

UCHAR VISA_MSession_Key[16];
UCHAR VISA_ASession_Key[16];

//UCHAR api_msg_amount[32]={'A','m','t',':','N','T','$'};
UCHAR api_msg_amount[32]={0};
UCHAR api_msg_amount_len = 0;


//20131031
#ifndef _PLATFORM_AS210
extern UCHAR apk_tsc_switch( UCHAR flag );	// tsc switch
#endif

#ifdef _PLATFORM_AS350_LITE
#ifdef _ACQUIRER_CTCB
extern void PIC_QShowPicture(UCHAR pic_id, UINT pic_Width, UINT pic_Height, UINT pic_Xleft, UINT pic_Ytop);
#endif
#endif

//main.c
extern UCHAR	main_card_remove;
extern UCHAR	main_clean_screen;
extern UCHAR	main_txn_complete;

//ETP_EntryPoint.c
extern UINT		etp_tmrSale;					//P_SALE_TIMEOUT
extern UCHAR	etp_flgMtiAID_UPI;
extern UCHAR	etp_flgMtiAID_Discover;

//MPP_Kernel.c
extern UINT		mpp_lenDE_xmlBuf;
extern UCHAR	*mpp_ptrDE_xmlBuf;

//JCB_Kernel_Config.c
extern UCHAR JCB_TAA_Result;

//AEX_Kernel.c
extern UCHAR	aex_parFunction;
extern UCHAR	aex_rngUnpNumber;
extern UCHAR	aex_tmrDeactivation;

extern LIMITSET	aex_drlDefault;
extern LIMITSET	aex_drlSets[AEX_DRL_NUMBER];


// 20140107For RFS D4 other Data
UCHAR Available_Offline_Amount[7] = {0};
UCHAR Available_Offline_Amount_AP[7] = {0};
UCHAR api_tag9F39[11]={0};
UCHAR api_tagDF0E[11]={0};
UCHAR api_W3_DDAFail[3] = {0};
UCHAR api_Tag9F74_Data[192];
UCHAR api_Tag9F74_Data_Length;
//For D3 Part
UCHAR SchemeID;
UCHAR L3_Response_Code;
//For D1 D2 Part
UCHAR D1_Data[150]={0};
UINT  D1_Data_Length=0;
UCHAR Tag57_Data[39]={0};
UCHAR Tag57_Data_Length=0;

//random number
static UCHAR	api_pcd_vap_rndB[8]={0};
static UCHAR	api_pcd_vap_rndR[8]={0};

//		Authentication Session Key
static UCHAR	api_pcd_vap_autSesKey[16]={0};

//		Flag
UCHAR	api_pcd_vap_flgAdmMode=FALSE;
UCHAR	api_pcd_vap_flgMutAuthenticate=FALSE;
UCHAR	api_pcd_vap_flgDbgAndOptMode=FALSE;
UCHAR	api_pcd_vap_flgNormalMode=TRUE;
UCHAR	api_pcd_vap_flgRFSBeep=TRUE;
UCHAR	api_pcd_vap_flgJCBRefResponse=FALSE;


extern void api_emvk_IncTransSequenceCounter( void );


UCHAR	API_Check_SchemeID(UCHAR SID)
{
	UCHAR Scheme_Index = 0;

	for(Scheme_Index=0;Scheme_Index<VAP_SCHEME_NUMBER;Scheme_Index++)
	{
		if(glv_vap_Scheme[Scheme_Index].ID == SID)
		{
			if(glv_vap_Scheme[Scheme_Index].Support == 0x01)
				return SUCCESS;
		}
	}

	return FAIL;
}

//FIND ADD CAPK Address
UCHAR	API_FIND_ADD_CAPK_Addr(UCHAR ID, UCHAR Index,ULONG *Address)
{
	CAPK tempCAPK;

	UCHAR  AID[5]={0x00,0x00,0x00,0x00,0x00};
	UCHAR AID1[5]={0xA0,0x00,0x00,0x00,0x03};
	UCHAR AID2[5]={0xA0,0x00,0x00,0x00,0x04};
	UCHAR AID3[5]={0xA0,0x00,0x00,0x00,0x65};
	UCHAR AID4[5]={0xA0,0x00,0x00,0x99,0x99};
	UCHAR AID5[5]={0xB0,0x12,0x34,0x56,0x78};
	UCHAR AID6[5]={0xA0,0x00,0x00,0x03,0x33};
	UCHAR AID7[5]={0xA0,0x00,0x00,0x00,0x25};
	UCHAR AID8[5]={0xA0,0x00,0x00,0x01,0x52};

	UCHAR rspCode,FreeFlag=0;

	UINT i;

	switch(ID)
	{
		case VAP_CAPK_ID_VISA:			memcpy(AID,AID1,5);	break;
		case VAP_CAPK_ID_MASTER:		memcpy(AID,AID2,5);	break;
		case VAP_CAPK_ID_JCB:			memcpy(AID,AID3,5);	break;
		case VAP_CAPK_ID_VISA_TEST:		memcpy(AID,AID4,5);	break;
		case VAP_CAPK_ID_MASTER_TEST:	memcpy(AID,AID5,5);	break;
		case VAP_CAPK_ID_UPI:			memcpy(AID,AID6,5);	break;
		case VAP_CAPK_ID_AE:			memcpy(AID,AID7,5);	break;
		case VAP_CAPK_ID_DISCOVER:		memcpy(AID,AID8,5);	break;

		default:						return FAIL;
	}
	
#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
	rspCode = apiOK;
#endif

	if(rspCode == apiOK)
	{
		//Is there any same CAPK in Flash
		for(i = 0;i<CAPK_NUMBER;i++)
		{
			rspCode = api_fls_read(FLSID_CAPK,CAPK_LEN*i,CAPK_LEN,(UCHAR *)&tempCAPK);
	
			if(rspCode == apiOK) //Read Success
			{	
				if(!(memcmp(tempCAPK.RID,AID,5)) && (tempCAPK.Index == Index))	//Find Same CAPK 
				{	
					*Address = CAPK_LEN*i;
					return SUCCESS;
				}
				else
				{
					if(FreeFlag == 0)
					{
						if( memcmp(tempCAPK.RID,AID1,5)&&
							memcmp(tempCAPK.RID,AID2,5)&&
							memcmp(tempCAPK.RID,AID3,5)&&
							memcmp(tempCAPK.RID,AID4,5)&&
							memcmp(tempCAPK.RID,AID5,5)&&
							memcmp(tempCAPK.RID,AID6,5)&&
							memcmp(tempCAPK.RID,AID7,5)&&
							memcmp(tempCAPK.RID,AID8,5))	//Find Free Space
						{	
							*Address = CAPK_LEN*i;
							FreeFlag = 1;
						}
					}
				}
			}
			else					//Read Fail
				return FAIL;
		}
		
			if(FreeFlag == 1)
				return SUCCESS;
			else
				return FAIL;	
	}
	else
		return FAIL;	

	
}

//FIND CAPK Address
UCHAR	API_FIND_CAPK_Addr(UCHAR ID, UCHAR Index, ULONG *Address)
{
	CAPK tempCAPK;

	UCHAR  AID[5]={0x00,0x00,0x00,0x00,0x00};
	UCHAR AID1[5]={0xA0,0x00,0x00,0x00,0x03};
	UCHAR AID2[5]={0xA0,0x00,0x00,0x00,0x04};
	UCHAR AID3[5]={0xA0,0x00,0x00,0x00,0x65};
	UCHAR AID4[5]={0xA0,0x00,0x00,0x99,0x99};
	UCHAR AID5[5]={0xB0,0x12,0x34,0x56,0x78};
	UCHAR AID6[5]={0xA0,0x00,0x00,0x03,0x33};
	UCHAR AID7[5]={0xA0,0x00,0x00,0x00,0x25};
	UCHAR AID8[5]={0xA0,0x00,0x00,0x01,0x52};

	UCHAR rspCode;

	UINT i;

	switch(ID)
	{
		case VAP_CAPK_ID_VISA:			memcpy(AID,AID1,5);	break;
		case VAP_CAPK_ID_MASTER:		memcpy(AID,AID2,5);	break;
		case VAP_CAPK_ID_JCB:			memcpy(AID,AID3,5);	break;
		case VAP_CAPK_ID_VISA_TEST:		memcpy(AID,AID4,5);	break;
		case VAP_CAPK_ID_MASTER_TEST:	memcpy(AID,AID5,5);	break;
		case VAP_CAPK_ID_UPI:			memcpy(AID,AID6,5);	break;
		case VAP_CAPK_ID_AE:			memcpy(AID,AID7,5);	break;
		case VAP_CAPK_ID_DISCOVER:		memcpy(AID,AID8,5);	break;

		default:						return FAIL;
	}
	
#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else	
	rspCode = apiOK;
#endif

	if(rspCode == apiOK)
	{
		for(i=0;i<CAPK_NUMBER;i++)
		{
			rspCode = api_fls_read(FLSID_CAPK,CAPK_LEN*i,CAPK_LEN,(UCHAR *)&tempCAPK);

			if(rspCode == apiOK)
			{
				if((!memcmp(tempCAPK.RID,AID,5)) && (tempCAPK.Index == Index))
				{
					*Address = CAPK_LEN*i;
					return SUCCESS;
				}
			}
			else
				return FAIL;
		}
		return FAIL;
	}														//20140120 V2, Changed
	else													//20140120 V2, Changed
		return FAIL;										//20140120 V2, Changed	
}


UCHAR api_pcd_vap_Check_PayPassConfigurationData(UINT iptLen, UCHAR *iptData)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	*ptrData=NULLPTR;
	UINT	lenOfData=0;
	UINT	lenOfDatSet=0;
	UCHAR	rspCode=0;
	
	if (iptLen <= (FLS_SIZE_CONFIGURATION_DATA-2))
	{
		ptrData=iptData;

		//Check AID
		if ((ptrData[0] == 0x9F) && (ptrData[1] == 0x06))
		{
			UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

			ptrData+=(lenOfT+lenOfL+lenOfV);
			lenOfData+=(lenOfT+lenOfL+lenOfV);

			//Check Transaction Type
			if (ptrData[0] == 0x9C)
			{
				UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

				ptrData+=(lenOfT+lenOfL+lenOfV);
				lenOfData+=(lenOfT+lenOfL+lenOfV);

				//Parse TLV
				do
				{
					rspCode=UT_Get_TLVLength(ptrData+lenOfDatSet, &lenOfT, &lenOfL, &lenOfV);
					if (rspCode == SUCCESS)
					{
						lenOfDatSet+=(lenOfT+lenOfL+lenOfV);
					}
					else
					{
						break;
					}
				} while ((lenOfData+lenOfDatSet) < iptLen);

				if ((lenOfData+lenOfDatSet) == iptLen)
				{
					return SUCCESS;
				}
			}
		}
	}
	
	return FAIL;
}

UCHAR api_pcd_vap_Check_MultipleAIDData(UINT iptLen, UCHAR *iptData, UCHAR iptMaxSet)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	curLen=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=FAIL;
	UCHAR	cntIndex=0;

	if (iptLen <= (iptMaxSet*FLS_SIZE_MULTIAID_AID_SET))
	{
		ptrData=iptData;

		for (cntIndex=0; cntIndex < iptMaxSet; cntIndex++)
		{
			//Check AID
			if ((ptrData[0] == 0x9F) && (ptrData[1] == 0x06))
			{
				rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == SUCCESS)
				{
					if (lenOfV <= ECL_LENGTH_9F06)
					{
						curLen+=(lenOfT+lenOfL+lenOfV);
						ptrData+=(lenOfT+lenOfL+lenOfV);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}

			//Check TTQ
			if ((ptrData[0] == 0xDF) && (ptrData[1] == 0x77))
			{
				rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == SUCCESS)
				{
					if (lenOfV == ECL_LENGTH_9F66)
					{
						curLen+=(lenOfT+lenOfL+lenOfV);
						ptrData+=(lenOfT+lenOfL+lenOfV);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}

			//Check Risk Parameter
			if ((ptrData[0] == 0xDF) && (ptrData[1] == 0x73))
			{
				rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == SUCCESS)
				{
					if (lenOfV == 2)
					{
						curLen+=(lenOfT+lenOfL+lenOfV);
						ptrData+=(lenOfT+lenOfL+lenOfV);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}

			//Check Transaction Limit
			if ((ptrData[0] == 0xDF) && (ptrData[1] == 0x70))
			{
				rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == SUCCESS)
				{
					if (lenOfV == 6)
					{
						curLen+=(lenOfT+lenOfL+lenOfV);
						ptrData+=(lenOfT+lenOfL+lenOfV);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}

			//Check CVM Required Limit
			if ((ptrData[0] == 0xDF) && (ptrData[1] == 0x71))
			{
				rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == SUCCESS)
				{
					if (lenOfV == 6)
					{
						curLen+=(lenOfT+lenOfL+lenOfV);
						ptrData+=(lenOfT+lenOfL+lenOfV);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}

			//Check CL Floor Limit
			if ((ptrData[0] == 0xDF) && (ptrData[1] == 0x72))
			{
				rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == SUCCESS)
				{
					if (lenOfV == 6)
					{
						curLen+=(lenOfT+lenOfL+lenOfV);
						ptrData+=(lenOfT+lenOfL+lenOfV);
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}

			if (iptLen == curLen)
			{
				return SUCCESS;
			}
		}
	}
	
	return FAIL;
}

UCHAR api_pcd_vap_Check_JSpeedyRefundDecline(void)
{
	UCHAR ridJCB[5]={0xA0,0x00,0x00,0x00,0x65};
	
	if ((!memcmp(glv_tag9F06.Value, ridJCB, 5)) &&
		(glv_tag9C.Value[0] == 0x20) &&
		(JCB_TAA_Result == JCB_TAAR_AAC))
	{
		return SUCCESS;
	}
	
	return FAIL;
}

UCHAR api_pcd_vap_Update_TLVData(UCHAR *iptTLV, UINT *udtLen, UCHAR *udtData)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	lenOfT_ipt=0;
	UCHAR	lenOfL_ipt=0;
	UINT	lenOfV_ipt=0;
	UINT	lenResidual=0;
	UCHAR	tmpBuffer[512]={0};
	UCHAR	*ptrFound=NULLPTR;
	UCHAR	*ptrResidual=NULLPTR;


	UT_Get_TLVLength(iptTLV, &lenOfT_ipt, &lenOfL_ipt, &lenOfV_ipt);
	
	ptrFound=UT_Find_Tag(iptTLV, udtLen[0], udtData);
	if (ptrFound != NULLPTR)
	{
		UT_Get_TLVLength(ptrFound, &lenOfT, &lenOfL, &lenOfV);

		if ((lenOfT == lenOfT_ipt) && (lenOfL == lenOfL_ipt) &&	(lenOfV == lenOfV_ipt))
		{
			if (!memcmp(&ptrFound[lenOfT+lenOfL], &iptTLV[lenOfT_ipt+lenOfL_ipt], lenOfV))
			{
				;	//Same Data, don't update
			}
			else
			{
				memcpy(&ptrFound[lenOfT+lenOfL], &iptTLV[lenOfT_ipt+lenOfL_ipt], lenOfV);

				return TRUE;
			}
		}
		else
		{
			//Copy Residual Data
			ptrResidual=ptrFound+(lenOfT+lenOfL+lenOfV);
			lenResidual=udtLen[0]-(ptrResidual-udtData);	//Residual Len = Total Len - (Len befor Residual Data)
			memcpy(tmpBuffer, ptrResidual, lenResidual);
			
			//Remove Data
			memcpy(ptrFound, tmpBuffer, lenResidual);
			
			//Update Data
			memcpy((ptrFound+lenResidual), iptTLV, (lenOfT_ipt+lenOfL_ipt+lenOfV_ipt));
			udtLen[0]=udtLen[0]-(lenOfT+lenOfL+lenOfV)+(lenOfT_ipt+lenOfL_ipt+lenOfV_ipt);

			return TRUE;
		}
	}
	else
	{
		memcpy(&udtData[udtLen[0]], iptTLV, (lenOfT_ipt+lenOfL_ipt+lenOfV_ipt));
		udtLen[0]+=(lenOfT_ipt+lenOfL_ipt+lenOfV_ipt);
		
		return TRUE;
	}
	
	return FALSE;
}


void api_pcd_vap_Encrypt_Data(
	UINT	iptLen,
	UCHAR	*iptData,
	UINT	*optLen,
	UCHAR	*optData,
	UCHAR	*encKey)
{
	UCHAR	flgPadding=0;
	UCHAR	rsdByte=0;
	UCHAR	desKey[24]={0};
	UCHAR	tmpBuffer[8]={0};
	UINT	encNumber=0;
	UINT	idxNum=0;
	
	memcpy(&desKey[0], encKey, 16);
	memcpy(&desKey[16], encKey, 8);

	rsdByte=iptLen % 8;
	if (rsdByte != 0)
	{
		flgPadding=TRUE;
		optLen[0]=((iptLen/8)+1)*8;

		encNumber=optLen[0]/8;
	}
	else
	{
		encNumber=iptLen/8;	
		optLen[0] = iptLen;
	}

	for (idxNum=0; idxNum < encNumber; idxNum++)
	{
		memset(tmpBuffer, 0, 8);

		if ((flgPadding == TRUE) && ((idxNum + 1) == encNumber))
		{
			memcpy(tmpBuffer, iptData, rsdByte);
		}
		else
		{
			memcpy(tmpBuffer, iptData, 8);
		}
		
		api_3des_encipher(tmpBuffer, optData, desKey);
		
		iptData+=8;
		optData+=8;
	}
}


UCHAR api_pcd_vap_Get_Capability(UCHAR iptScmID, UCHAR *optSupport)
{
	UCHAR	idxNum=0;

	for (idxNum=0; idxNum < VAP_SCHEME_NUMBER; idxNum++)
	{
		if (iptScmID == glv_vap_Scheme[idxNum].ID)
		{
			optSupport[0]=glv_vap_Scheme[idxNum].Support;
			return SUCCESS;
		}
	}

	return FAIL;
}


UCHAR api_pcd_vap_Get_DisplayMessage(UCHAR iptMsgID, UCHAR *optMsgLength, UCHAR *optMessage)
{
	UCHAR	idxNum=0;
	
	for (idxNum=0; idxNum < VAP_MESSAGE_NUMBER; idxNum++)
	{
		if (glv_vap_Message[idxNum].ID == iptMsgID)
		{
			optMsgLength[0]=glv_vap_Message[idxNum].Length;
			memcpy(optMessage, glv_vap_Message[idxNum].Message, glv_vap_Message[idxNum].Length);

			return SUCCESS;
		}
	}
	return FAIL;
}

UCHAR api_pcd_vap_Get_EMVTag(UCHAR *iptTag, UCHAR *optLength, UCHAR *optData)
{
	UCHAR	lenOfT=0;
	UINT	intTag=0;

	UT_Get_TLVLengthOfT(iptTag, &lenOfT);

	if (lenOfT == 1)
	{
		intTag=iptTag[0];
	}
	else if (lenOfT == 2)
	{
		intTag=iptTag[0]*256+iptTag[1];
	}
	else
	{
		return FAIL;
	}

	switch (intTag)
	{
		//Transaction Currency Code
		case 0x5F2A:
			optLength[0]=ETP_PARA_SIZE_5F2A;
			memcpy(optData, glv_par5F2A, ETP_PARA_SIZE_5F2A);
			return SUCCESS;
		
		//Txn Currency Code
		case 0x5F36:	
			optLength[0]=ETP_PARA_SIZE_5F36;
			memcpy(optData,glv_par5F36, ETP_PARA_SIZE_5F36);

			return SUCCESS;	
		
		//Tracsaction Type
		case 0x009C:
			optLength[0]=ETP_PARA_SIZE_9C;
			memcpy(optData, glv_par9C, ETP_PARA_SIZE_9C);
			return SUCCESS;

		//Acquired ID
		case 0x9F01:
			optLength[0]=ETP_PARA_SIZE_9F01;
			memcpy(optData,glv_par9F01, ETP_PARA_SIZE_9F01);

			return SUCCESS;
			
		//Merchant Code
		case 0x9F15:	
			optLength[0]=ETP_PARA_SIZE_9F15;
			memcpy(optData,glv_par9F15, ETP_PARA_SIZE_9F15);

			return SUCCESS;
			
		//Terminal Country Code
		case 0x9F1A:
			optLength[0]=ETP_PARA_SIZE_9F1A;
			memcpy(optData, glv_par9F1A, ETP_PARA_SIZE_9F1A);
			return SUCCESS;

		case 0x9F1B:			
			if(	(glv_par9F1B_VISA[VISA_Purchase_Mode][0] == 0x00)&&
				(glv_par9F1B_VISA[VISA_Purchase_Mode][1] == 0x00)&&
				(glv_par9F1B_VISA[VISA_Purchase_Mode][2] == 0x00)&&
				(glv_par9F1B_VISA[VISA_Purchase_Mode][3] == 0x00))		//absent
			{
				if(glv_par9F1B_Len == 0)
					optLength[0]= 0;
				else
				{
					optLength[0]= ETP_PARA_SIZE_9F1B;
					memcpy(optData, glv_par9F1B_VISA[VISA_Purchase_Mode], ETP_PARA_SIZE_9F1B);
				}
			}
			else
			{
				optLength[0]= ETP_PARA_SIZE_9F1B;
				memcpy(optData, glv_par9F1B_VISA[VISA_Purchase_Mode], ETP_PARA_SIZE_9F1B);
			}
			return SUCCESS;

		//Terminal Capability
		case 0x9F33:
			optLength[0]=ETP_PARA_SIZE_9F33;
			memcpy(optData, glv_par9F33, ETP_PARA_SIZE_9F33);
			return SUCCESS;
		
		//VISA Terminal Type
		case 0x9F35:
			optLength[0]=ETP_PARA_SIZE_9F35;
			memcpy(optData, &glv_par9F35[0], ETP_PARA_SIZE_9F35);
			return SUCCESS;

		//Merchant Name
		case 0x9F4E:	
			if(glv_par9F4ELen)
			{
				optLength[0] = glv_par9F4ELen;
				memcpy(optData,glv_par9F4E, glv_par9F4ELen);
			}
			else
				optLength[0] = 0;

			return SUCCESS;

		case 0x9F66:
			optLength[0]= ETP_PARA_SIZE_9F66;
			memcpy(optData, &glv_par9F66[2], ETP_PARA_SIZE_9F66);
			return SUCCESS;

		//Reader Contactless Transaction floor Limit
		case 0xDF00:
			optLength[0]=glv_parDF00Len[2];
			memcpy(optData, &glv_parDF00_VISA[VISA_Purchase_Mode], glv_parDF00Len[2]);
			return SUCCESS;
			
		//payWAVE Reader CVM Required Limit	
		case 0xDF01:
			optLength[0]=glv_parDF01Len[2];
			memcpy(optData, &glv_parDF01_VISA[VISA_Purchase_Mode], glv_parDF01Len[2]);	
			return SUCCESS;
		
		//payWAVE Reader CL Floor Limit
		case 0xDF02:
			optLength[0]=glv_parDF02Len[2];
			memcpy(optData, &glv_parDF02_VISA[VISA_Purchase_Mode], glv_parDF02Len[2]);	
			return SUCCESS;
				
		case 0xDF03:
			optLength[0]=ETP_PARA_SIZE_DF03;
			memcpy(optData,&glv_parDF03[0], ETP_PARA_SIZE_DF03);
			return SUCCESS;
			
		case 0xDF04:
			optLength[0]=ETP_PARA_SIZE_DF04;
			memcpy(optData, &glv_parDF04[0], ETP_PARA_SIZE_DF04);
			return SUCCESS;

		case 0xDF05:
			optLength[0]=ETP_PARA_SIZE_DF05;
			memcpy(optData, &glv_parDF05[0], ETP_PARA_SIZE_DF05);
			return SUCCESS;

		case 0xDF06:			
			optLength[0]= ETP_PARA_SIZE_DF06;
			memcpy(optData, glv_parDF06_VISA[VISA_Purchase_Mode], ETP_PARA_SIZE_DF06);
			return SUCCESS;


		case 0xDF30:	//DPAS - Contactless Transaction Limit
			optLength[0]=glv_parDF00Len[5];
			memcpy(optData, &glv_parDF00[5], glv_parDF00Len[5]);
			return SUCCESS;
			
		case 0xDF31:	//DPAS - CVM Required Limit
			optLength[0]=glv_parDF01Len[5];
			memcpy(optData, &glv_parDF01[5], glv_parDF01Len[5]);	
			return SUCCESS;
			
		case 0xDF32:	//DPAS - Contactless Floor Limit
			optLength[0]=glv_parDF02Len[5];
			memcpy(optData, &glv_parDF02[5], glv_parDF02Len[5]);	
			return SUCCESS;
			
		case 0xDF33:	//DPAS - Risk Parameter
			optLength[0]=ETP_PARA_SIZE_DF06;
			memcpy(optData, glv_parDF06[5], ETP_PARA_SIZE_DF06);
			return SUCCESS;
			
		case 0xDF34:	//DPAS - Terminal Transaction Qualifier
			optLength[0]=ETP_PARA_SIZE_9F66;
			memcpy(optData, &glv_par9F66[5], ETP_PARA_SIZE_9F66);
			return SUCCESS;

		case 0xDF35:	//DPAS - Terminal Type
			optLength[0]=ETP_PARA_SIZE_9F35;
			memcpy(optData, &glv_par9F35[5], ETP_PARA_SIZE_9F35);
			return SUCCESS;
			
		case 0xDF36:	//DPAS - Terminal Application Version Number
			optLength[0]=ETP_PARA_SIZE_9F09;
			memcpy(optData, &glv_par9F09[5], ECL_LENGTH_9F09);
			return SUCCESS;

		


		//Reader Management Check
		case 0xDF41:
		//mapping DF41 to DF06
		/*
Order	B1 B2									B1 B2
		==DF41==(2 Bytes)						==DF06==(2 Bytes)
		B2b8: RFU								B1b8: Status Check enabled/disabled
		B2b7: RFU								B1b7: Amount, Authorized of Zero Check enabled/disabled
		B2b6: Floor Limit Check						B1b6: Amount, Authorized of Zero Option (1b=Option 1 and 0b=Option 2)
		B2b5: CVM Limit Check						B1b5: Reader Contactless Transaction Limit Check enabled/disabled
		B2b4: Txn Limit Check						B1b4: Reader CVM Required Limit Check enabled/disabled
		B2b3: Zero Check Option 2					B1b3: Reader Contactless Floor Limit Check enabled/disabled
		B2b2: Zero Check Option 1					B1b2: Exception file enabled/disabled
		B2b1: Status Check						B2b8: Terminal Floor Limit Check enabled/disabled
		All other bits are RFU						All other bits are RFU
		*/	
			optLength[0]= ETP_PARA_SIZE_DF06;

			//B2b6: Floor Limit Check		
			if(glv_parDF06_VISA[VISA_Purchase_Mode][0] & 0x04)
				optData[1] |= 0x20;

			//B2b5: CVM Limit Check		
			if(glv_parDF06_VISA[VISA_Purchase_Mode][0] & 0x08)
				optData[1] |= 0x10;

			//B2b4: Txn Limit Check		
			if(glv_parDF06_VISA[VISA_Purchase_Mode][0] & 0x10)
				optData[1] |= 0x08;
			
			if(glv_parDF06_VISA[VISA_Purchase_Mode][0] & 0x40)
			{
				if(glv_parDF06_VISA[VISA_Purchase_Mode][0] & 0x20)
					optData[1] |= 0x02;		//B2b2: Zero Check Option 1
				else
					optData[1] |= 0x04;		//B2b3: Zero Check Option 2	
			}

			//B2b1: Status Check
			if(glv_parDF06_VISA[VISA_Purchase_Mode][0] & 0x80)
			{
				optData[1] |= 0x01;
			}
		
			//All other bits are RFU
			optData[0] = 0x00;		

			return SUCCESS;

		case 0xDF50:	//AMEX - Application version number
			optLength[0]=ETP_PARA_SIZE_9F09;
			memcpy(optData, &glv_par9F09[3], ECL_LENGTH_9F09);
			return SUCCESS;
			
		case 0xDF51:	//AMEX - TAC Default
			optLength[0]=ECL_LENGTH_DF8120;
			memcpy(optData, &glv_parDF8120[3], ECL_LENGTH_DF8120);
			return SUCCESS;
						
		case 0xDF52:	//AMEX - TAC Denial
			optLength[0]=ECL_LENGTH_DF8121;
			memcpy(optData, &glv_parDF8121[3], ECL_LENGTH_DF8121);
			return SUCCESS;
						
		case 0xDF53:	//AMEX - TAC Online
			optLength[0]=ECL_LENGTH_DF8122;
			memcpy(optData, &glv_parDF8122[3], ECL_LENGTH_DF8122);
			return SUCCESS;
			
		case 0xDF54:	//AMEX - Contactless Reader Capabilities
			optLength[0]=1;
			memcpy(optData, &glv_par9F6D[3], 1);
			return SUCCESS;
						
		case 0xDF55:	//AMEX - Enhanced Contactless Reader Capabilities
			optLength[0]=4;
			memcpy(optData, &glv_par9F6E[3], 4);
			return SUCCESS;
			
		case 0xDF56:	//AMEX - Terminal Type
			optLength[0]=ECL_LENGTH_9F35;
			memcpy(optData, &glv_par9F35[3], ECL_LENGTH_9F35);
			return SUCCESS;
			
		case 0xDF57:	//AMEX - Unpredictable Number Range
			optLength[0]=1;
			optData[0]=aex_rngUnpNumber;
			return SUCCESS;

		case 0xDF58:	//AMEX - Deactivation Timer
			optLength[0]=1;
			optData[0]=aex_tmrDeactivation;
			return SUCCESS;
			
		case 0xDF59:	//AMEX - Function Parameter
			optLength[0]=1;
			optData[0]=aex_parFunction;
			return SUCCESS;

		case 0xDF5A:	//AMEX - Contactless Transaction Limit
			optLength[0]=ETP_PARA_SIZE_DF00;
			memcpy(optData, &glv_parDF00[3], ETP_PARA_SIZE_DF00);
			return SUCCESS;

		case 0xDF5B:	//AMEX - CVM Required Limit
			optLength[0]=ETP_PARA_SIZE_DF01;
			memcpy(optData, &glv_parDF01[3], ETP_PARA_SIZE_DF01);
			return SUCCESS;

		case 0xDF5C:	//AMEX - Contactless Floor Limit
			optLength[0]=ETP_PARA_SIZE_DF02;
			memcpy(optData, &glv_parDF02[3], ETP_PARA_SIZE_DF02);
			return SUCCESS;

		case 0xDF5D:	//AMEX - Risk Parameter
			optLength[0]=ETP_PARA_SIZE_DF06;
			memcpy(optData, &glv_parDF06[3], ETP_PARA_SIZE_DF06);
			return SUCCESS;

		//JCB Parameter
		case 0xDF60:	//JCB Contactless Transaction Limit
			if(JCB_Static_Parameter.ClessTxnLimitLen)
			{
				optLength[0]=JCB_Static_Parameter.ClessTxnLimitLen;
				memcpy(optData, JCB_Static_Parameter.ClessTxnLimit, JCB_Static_Parameter.ClessTxnLimitLen);	
			}
			else
				optLength[0]=0;
			
			return SUCCESS;
			
		case 0xDF61:	//JCB Reader CVM Required Limit
			if(JCB_Static_Parameter.ClessCVMLimitLen)
			{
				optLength[0]=JCB_Static_Parameter.ClessCVMLimitLen;
				memcpy(optData, JCB_Static_Parameter.ClessCVMLimit, JCB_Static_Parameter.ClessCVMLimitLen);	
			}
			else
				optLength[0]=0;
			
			return SUCCESS;
			
		case 0xDF62:	//JCB Reader CL Floor Limit

			if(JCB_Static_Parameter.ClessFloorLimitLen)
			{
				optLength[0]=JCB_Static_Parameter.ClessFloorLimitLen;
				memcpy(optData, JCB_Static_Parameter.ClessFloorLimit, JCB_Static_Parameter.ClessFloorLimitLen);	
			}
			else
				optLength[0]=0;
			
			return SUCCESS;
			
		case 0xDF63:	//Enhance DDA Ver Num
			optLength[0]=ETP_PARA_SIZE_DF03;
			memcpy(optData,&glv_parDF03[0], ETP_PARA_SIZE_DF03);
			return SUCCESS;
			
		case 0xDF64:	//CVM Require
			optLength[0]=ETP_PARA_SIZE_DF04;
			memcpy(optData,&glv_parDF04[0], ETP_PARA_SIZE_DF04);
			return SUCCESS;
			
		case 0xDF65:	//Display Offline Funds
			optLength[0]=ETP_PARA_SIZE_DF05;
			memcpy(optData, &glv_parDF05[0], ETP_PARA_SIZE_DF05);
			return SUCCESS;
		
		case 0xDF66:	//JCB Terminal Type
			optLength[0]=ETP_PARA_SIZE_9F35;
			optData[0] = JCB_Static_Parameter.TerminalType;
			return SUCCESS;

		case 0xDF67:	//TTQ
			optLength[0]= ETP_PARA_SIZE_9F66;
			memcpy(optData, &glv_par9F66[0], ETP_PARA_SIZE_9F66);
			return SUCCESS;
			
		case 0xDF68: //Combination Option
			optLength[0] = 2;
			memcpy(optData,JCB_Static_Parameter.CombinationOption,2);
			
			return SUCCESS;

		case 0xDF69:	//Terminal Interchange Profile
			optLength[0] = 3;			
			memcpy(optData,JCB_Static_Parameter.TerminalInterchangeProfile,3);
			
			return SUCCESS;			
			
		case 0xDF6A:	//BRS MAX Target
			optLength[0] = 1;
			optData[0] = JCB_Static_Parameter.MAXTargetPercent_RandomSelection;

			return SUCCESS;
		
		case 0xDF6B:	//BRS Targer
			optLength[0] = 1;
			optData[0] = JCB_Static_Parameter.TargetPercent_RandomSelection;

			return SUCCESS;	
		
		case 0xDF6C:	//BRS Threshold
			optLength[0] = 6;
			memcpy(optData,JCB_Static_Parameter.ThresholdValue_RandomSelection,6);

			return SUCCESS;
		
		case 0xDF6D:	//TAC_Default
			if(JCB_Static_Parameter.TAC_Default_Present)
			{
				optLength[0] = 5;
				memcpy(optData,JCB_Static_Parameter.TAC_Default,5);
			}
			else
				optLength[0] = 0;

			return SUCCESS;

		case 0xDF6E:	//TAC_Denial
			if(JCB_Static_Parameter.TAC_Denial_Present)
			{
				optLength[0] = 5;
				memcpy(optData,JCB_Static_Parameter.TAC_Denial,5);
			}
			else
				optLength[0] = 0;

			return SUCCESS;

		case 0xDF6F:	//TAC_Online
			if(JCB_Static_Parameter.TAC_Online_Present)
			{
				optLength[0] = 5;
				memcpy(optData,JCB_Static_Parameter.TAC_Online,5);
			}
			else
				optLength[0] = 0;

			return SUCCESS;

		case 0xDF70:	//qUICS - Contactless Transaction Limit
			optLength[0]=ETP_PARA_SIZE_DF00;
			memcpy(optData, &glv_parDF00[6], ETP_PARA_SIZE_DF00);
			return SUCCESS;

		case 0xDF71:	//qUICS - CVM Required Limit
			optLength[0]=ETP_PARA_SIZE_DF01;
			memcpy(optData, &glv_parDF01[6], ETP_PARA_SIZE_DF01);
			return SUCCESS;

		case 0xDF72:	//qUICS - Transaction Floor Limit
			optLength[0]=ETP_PARA_SIZE_DF02;
			memcpy(optData, &glv_parDF02[6], ETP_PARA_SIZE_DF02);
			return SUCCESS;

		case 0xDF73:	//qUICS - Reader Risk Parameter
			optLength[0]=ETP_PARA_SIZE_DF06;
			memcpy(optData, &glv_parDF06[6], ETP_PARA_SIZE_DF06);
			return SUCCESS;

		case 0xDF77:	//qUICS - Terminal Transaction Qualifiers
			optLength[0]=ETP_PARA_SIZE_9F66;
			memcpy(optData, &glv_par9F66[6], ETP_PARA_SIZE_9F66);
			return SUCCESS;
	
		default: break;
	}
	
	return FAIL;
}


UINT api_pcd_vap_Get_MessageLength(UCHAR *iptMsgLength)
{
	return (iptMsgLength[0]*256+iptMsgLength[1]);
}


UCHAR api_pcd_vap_Get_Parameter(UCHAR *iptIndex, UCHAR *optLength, UCHAR *optData)
{
	UCHAR	idxNum=0;
	UINT	datLen=0;
	
	for (idxNum=0; idxNum < VAP_PARAMETER_NUMBER; idxNum++)
	{
		if (!memcmp(glv_vap_Parameter[idxNum].Index, iptIndex, 2))
		{
			//Copy Data
			datLen=glv_vap_Parameter[idxNum].Length[0]*256+glv_vap_Parameter[idxNum].Length[1];
			memcpy(optLength, glv_vap_Parameter[idxNum].Length, 2);
			memcpy(optData, glv_vap_Parameter[idxNum].Data, datLen);
			
			return SUCCESS;
		}
	}
	
	return FAIL;
}


UCHAR api_pcd_vap_Set_Capability(UCHAR iptScmID, UCHAR iptSupport)
{
	UCHAR	idxNum=0;

	for (idxNum=0; idxNum < VAP_SCHEME_NUMBER; idxNum++)
	{
		if (iptScmID == glv_vap_Scheme[idxNum].ID)
		{
			if ((iptSupport == TRUE) || (iptSupport == FALSE))
			{
				glv_vap_Scheme[idxNum].Support=iptSupport;

				//20140108  set glv_par_WAVE2_Enable enadle
				if(glv_vap_Scheme[idxNum].ID == VAP_Scheme_Wave2)
				{
					glv_par_WAVE2_Enable = iptSupport;
				}
				
				return SUCCESS;
			}
		}			
	}

	return FAIL;
}


UCHAR api_pcd_vap_Set_DisplayMessage(UCHAR iptMsgID, UCHAR iptMsgLength, UCHAR *iptMessage)
{
	UCHAR	idxNum=0;
	UCHAR	i = 0, C_Word[2]={0x00,0x00};

	for (idxNum=0; idxNum < VAP_MESSAGE_NUMBER; idxNum++)
	{
		if (glv_vap_Message[idxNum].ID == iptMsgID)
		{
			//Reset First
			memset(glv_vap_Message[idxNum].Message,0x00,64);
			glv_vap_Message[idxNum].Length = 0;

			
			//20130823, For Display the Message ID 0, we should clear the Screen (or row 2) first
			if(iptMsgID == 0x01)
			{
				
#ifdef _SCREEN_SIZE_128x64
				UT_ClearScreen();								//For AS210
#endif
			}
		
			if (iptMsgLength <= 64)
			{				
				for(i = 0 ; i < iptMsgLength ; i++)
				{					
					if(iptMessage[i] == 0x80)	//GHL Chinese Character translate to BIG5
					{
						UT_GHL_Trans_BIG5(&iptMessage[i],C_Word);
						
						memcpy(&iptMessage[i],C_Word,2);						

						i+=1;
					}
				}
				
				glv_vap_Message[idxNum].Length=iptMsgLength;
				memcpy(glv_vap_Message[idxNum].Message, iptMessage, iptMsgLength);				
				
				return SUCCESS;
			}
			else
			{
				return FAIL;
			}
		}
	}
	
	return FAIL;
}


UCHAR api_pcd_vap_Set_EMVTag(UCHAR *iptTLV)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	intTag=0;
	UINT 	i=0;
	UCHAR	rspCode=0;
	UINT	cfgLen=0;
	UCHAR	cfgData[FLS_SIZE_CONFIGURATION_DATA-2]={0};
	UCHAR	tmpTLV[9]={0};
	UCHAR	flgUpdate=FALSE;
	UCHAR	*ptrValue=NULLPTR;

	UT_Get_TLVLength(iptTLV, &lenOfT, &lenOfL, &lenOfV);

	if (lenOfT == 1)
	{
		intTag=iptTLV[0];
	}
	else if (lenOfT == 2)
	{
		intTag=iptTLV[0]*256+iptTLV[1];
	}
	else
	{
		return FAIL;
	}

	switch (intTag)
	{
		//Transaction Currency Code
		case 0x5F2A:
			if (lenOfV == ETP_PARA_SIZE_5F2A)
			{
				memcpy(glv_par5F2A, &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_5F2A);

				ETP_Add_TransactionData(iptTLV);

				return SUCCESS;
			}
			
			break;	//20140120 V1, Added

		//Transaction Currency Exponent
		case 0x5F36:	
			if(lenOfV == ETP_PARA_SIZE_5F36)
			{
				JCB_Static_Parameter.TxnCurrencyExponent = iptTLV[lenOfT+lenOfL];

				glv_par5F36[0] = iptTLV[lenOfT+lenOfL];

				return SUCCESS;
			}
			else 
				return FAIL;

		//Tracsaction Type
		case 0x009C:
			if (lenOfV == ETP_PARA_SIZE_9C)
			{
				memcpy(glv_par9C, &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_9C);

				ETP_Add_TransactionData(iptTLV);
				
				return SUCCESS;
			}

			break;	//20140120 V1, Added
		
		//Acquired ID
		case 0x9F01:	
			if(lenOfV == ETP_PARA_SIZE_9F01)
			{
				memcpy(JCB_Static_Parameter.AcquirerID,&iptTLV[lenOfT+lenOfL],lenOfV);

				memcpy(glv_par9F01,&iptTLV[lenOfT+lenOfL],lenOfV);

				return SUCCESS;
			}
			else
				return FAIL;

		//Merchant Category Code
		case 0x9F15:
			if(lenOfV == ETP_PARA_SIZE_9F15)
			{
				memcpy(JCB_Static_Parameter.MerchantCategoryCode,&iptTLV[lenOfT+lenOfL],lenOfV);

				//test
				memcpy(glv_par9F15,&iptTLV[lenOfT+lenOfL],lenOfV);

				return SUCCESS;
			}
			else
				return FAIL;

		//Terminal Country Code
		case 0x9F1A:
			if (lenOfV == ETP_PARA_SIZE_9F1A)
			{
				memcpy(glv_par9F1A, &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_9F1A);

				ETP_Add_TransactionData(iptTLV);
				
				return SUCCESS;
			}

			break;	//20140120 V1, Added

		case 0x9F1B:

			if (lenOfV == ETP_PARA_SIZE_9F1B)
			{
				glv_par9F1B_Len = lenOfV;	//20140120 V1, Added
			
				for(i=0;i<ETP_NUMBER_COMBINATION;i++)
				{
					if(i!=2)
					{
					//1.Reset to 0x00 
						memset(glv_par9F1B[i],0x00,ETP_PARA_SIZE_9F1B);

					//2.Set
						memcpy(&glv_par9F1B[i],&iptTLV[lenOfT+lenOfL],ETP_PARA_SIZE_9F1B);					
					}
					else		
					{
					//1.Reset to 0x00
						memset(glv_par9F1B_VISA[VISA_Purchase_Mode],0x00,ETP_PARA_SIZE_9F1B);
					
						memset(glv_par9F1B_VISA[VISA_Cash_Mode],0x00,ETP_PARA_SIZE_9F1B);
						
						memset(glv_par9F1B_VISA[VISA_Cashback_Mode],0x00,ETP_PARA_SIZE_9F1B);
	
					//2.Set 
						memcpy(&glv_par9F1B_VISA[VISA_Purchase_Mode][0],&iptTLV[lenOfT+lenOfL],ETP_PARA_SIZE_9F1B);	

						memcpy(&glv_par9F1B_VISA[VISA_Cash_Mode][0],&iptTLV[lenOfT+lenOfL],ETP_PARA_SIZE_9F1B);	

						memcpy(&glv_par9F1B_VISA[VISA_Cashback_Mode][0],&iptTLV[lenOfT+lenOfL],ETP_PARA_SIZE_9F1B);	
					}					
				}

				/*
				//20140112 set to DRL Limit Set and Cash Cashback limit set
				//DRL  limit set
				for(i=0;i<ETP_PARA_NUMBER_PID;i++)
				{
					//1. Reset to 0x00
					memset(glv_parDRLLimitSet[i].tertxnLimit,0x00,ETP_PARA_SIZE_9F1B);
					//2. set
					memcpy(glv_parDRLLimitSet[i].tertxnLimit,&iptTLV[lenOfT+lenOfL],ETP_PARA_SIZE_9F1B);	
				}
				*/				
				return SUCCESS;
			}

			//20140112 if lenofV = 0, it means absent
			if(lenOfV == 0)	//absent
			{
				glv_par9F1B_Len = lenOfV;	//20140120 V1, Added
			
				for(i=0;i<ETP_NUMBER_COMBINATION;i++)
				{
					if(i!=2)
					{
					//1.Reset to 0x00 
						memset(glv_par9F1B[i],0x00,ETP_PARA_SIZE_9F1B);
					}
					else		
					{
					//1.Reset to 0x00
						memset(glv_par9F1B_VISA[VISA_Purchase_Mode],0x00,ETP_PARA_SIZE_9F1B);

						memset(glv_par9F1B_VISA[VISA_Cash_Mode],0x00,ETP_PARA_SIZE_9F1B);

						memset(glv_par9F1B_VISA[VISA_Cashback_Mode],0x00,ETP_PARA_SIZE_9F1B);
					}					
				}

				/*
				//20140112 set to DRL Limit Set and Cash Cashback limit set
				//DRL  limit set
				for(i=0;i<ETP_PARA_NUMBER_PID;i++)
				{
					//1. Reset to 0x00
					memset(glv_parDRLLimitSet[i].tertxnLimit,0x00,ETP_PARA_SIZE_9F1B);
				}
				*/				
				return SUCCESS;
			}

			break;	//20140120 V1, Added

		case 0x9F33:
			if (lenOfV == ETP_PARA_SIZE_9F33)
			{
				memcpy(&glv_par9F33, &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_9F33);
				return SUCCESS;
			}
			break;
			
		//VISA Terminal Type
		case 0x9F35:
			if (lenOfV == ETP_PARA_SIZE_9F35)
			{
				memcpy(&glv_par9F35[0], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_9F35);

				//VCPS AP will Get 9F35 from glv_par9F35[0]
				//ETP_Add_TransactionData(iptTLV);
				
				return SUCCESS;
			}

			break;	//20140120 V1, Added

		// Merchant Name Location
		case 0x9F4E:	
			if ((lenOfV <= ETP_PARA_SIZE_9F4E) && (lenOfV <= sizeof(JCB_Static_Parameter.MerchantNameLocation)))
			{
				JCB_Static_Parameter.MerchantNameLocationLen = lenOfV;
				memcpy(JCB_Static_Parameter.MerchantNameLocation,&iptTLV[lenOfT+lenOfL],lenOfV);

				//test
				glv_par9F4ELen = lenOfV;
				memcpy(glv_par9F4E,&iptTLV[lenOfT+lenOfL],lenOfV);

				return SUCCESS;
			}
			else
				return FAIL;

		case 0x9F66:
			if (lenOfV == ETP_PARA_SIZE_9F66)
			{
				memcpy(&glv_par9F66[0], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_9F66);
				memcpy(&glv_par9F66[2], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_9F66);

//				ETP_Add_TransactionData(iptTLV);
				
				return SUCCESS;
			}

			break;	//20140120 V1, Added

		//Reader Contactless Transaction floor Limit
		case 0xDF00:
			if ((lenOfV == ETP_PARA_SIZE_DF00) || (lenOfV == 0))
			{
				for(i=0;i<ETP_NUMBER_COMBINATION;i++)
				{
					if (i > 2)
					{
						continue;	//Set Limit with Kernel's Proprietary Tag
					}
					
					glv_parDF00Len[i]=lenOfV;

					if (lenOfV == 0)
					{
						memset(&glv_parDF00[i], 0, ETP_PARA_SIZE_DF00);
					}
					else
					{
						memcpy(&glv_parDF00[i], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF00);
					}

					if (i == 2)
					{
						if (lenOfV == 0)
						{
							memset(&glv_parDF00_VISA[VISA_Purchase_Mode], 0, ETP_PARA_SIZE_DF00);
						}
						else
						{
							memcpy(&glv_parDF00_VISA[VISA_Purchase_Mode], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF00);
						}
					}
				}
				
				return SUCCESS;
			}
			
			break;	//20140120 V1, Added
			
		//payWAVE Reader CVM Required Limit	
		case 0xDF01:
			if ((lenOfV == ETP_PARA_SIZE_DF01) || (lenOfV == 0))
			{
				for(i=0;i<ETP_NUMBER_COMBINATION;i++)
				{
					if (i > 2)
					{
						continue;	//Set Limit with Kernel's Proprietary Tag
					}
					
					glv_parDF01Len[i]=lenOfV;
					
					if (lenOfV == 0)
					{
						memset(&glv_parDF01[i], 0, ETP_PARA_SIZE_DF01);
					}
					else
					{
						memcpy(&glv_parDF01[i], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF01);
					}


					if (i == 2)
					{
						if (lenOfV == 0)
						{
							memset(&glv_parDF01_VISA[VISA_Purchase_Mode], 0, ETP_PARA_SIZE_DF01);
						}
						else
						{
							memcpy(&glv_parDF01_VISA[VISA_Purchase_Mode], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF01);
						}
					}
				}
				
				return SUCCESS;
			}

			break;	//20140120 V1, Added

		//payWAVE Reader CL Floor Limit
		case 0xDF02:
			if ((lenOfV == ETP_PARA_SIZE_DF02) || (lenOfV == 0))
			{
				for(i=0;i<ETP_NUMBER_COMBINATION;i++)
				{
					if (i > 2)
					{
						continue;	//Set Limit with Kernel's Proprietary Tag
					}

					glv_parDF02Len[i]=lenOfV;
					
					if (lenOfV == 0)
					{
						memset(&glv_parDF02[i], 0, ETP_PARA_SIZE_DF02);
					}
					else
					{
						memcpy(&glv_parDF02[i], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF02);
					}

					if (i == 2)
					{
						if (lenOfV == 0)
						{
							memset(&glv_parDF02_VISA[VISA_Purchase_Mode], 0, ETP_PARA_SIZE_DF02);
						}
						else
						{
							memcpy(&glv_parDF02_VISA[VISA_Purchase_Mode], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF02);
						}
					}
				}
				
				return SUCCESS;
			}

			break;	//20140120 V1, Added
			
		case 0xDF03:
			if (lenOfV == ETP_PARA_SIZE_DF03)
			{
				memcpy(&glv_parDF03[0], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF03);
				return SUCCESS;
			}

			break;	//20140120 V1, Added
			
		case 0xDF04:
			if (lenOfV == ETP_PARA_SIZE_DF04)
			{
				memcpy(&glv_parDF04[0], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF04);
				return SUCCESS;
			}

			break;	//20140120 V1, Added
			
		case 0xDF05:
			if (lenOfV == ETP_PARA_SIZE_DF05)
			{
				memcpy(&glv_parDF05[0], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF05);
				return SUCCESS;
			}

			break;	//20140120 V1, Added

		case 0xDF06:

			if (lenOfV == ETP_PARA_SIZE_DF06)
			{
				for(i=0;i<ETP_NUMBER_COMBINATION;i++)
				{
					if (i > 2)
					{
						continue;	//Set Limit with Kernel's Proprietary Tag
					}
					
					if(i!=2)
					{
						memcpy(&glv_parDF06[i],&iptTLV[lenOfT+lenOfL],ETP_PARA_SIZE_DF06);					
					}
					else		
					{
						memcpy(&glv_parDF06_VISA[VISA_Purchase_Mode][0],&iptTLV[lenOfT+lenOfL],ETP_PARA_SIZE_DF06);	
					}					
				}
				
				return SUCCESS;
			}

			break;	//20140120 V1, Added	
			
		//PayPass Parameter
		case 0xDF23:	//Terminal Type(1) + Terminal Capabilities(3) + Transaction Type(1)
		case 0xDF25:	//Terminal Action Code - Denial
		case 0xDF26:	//Terminal Action Code - Offline
		case 0xDF27:	//Terminal Action Code - Default

			ptrValue=&iptTLV[lenOfT+lenOfL];
			
			rspCode=FLS_Read_PayPassConfigurationData(&cfgLen, cfgData);
			if (rspCode == SUCCESS)
			{
				rspCode=api_pcd_vap_Check_PayPassConfigurationData(cfgLen, cfgData);
				if (rspCode == SUCCESS)
				{
					if (intTag == 0xDF23)
					{
						tmpTLV[0]=0x9F;			//Terminal Type
						tmpTLV[1]=0x35;
						tmpTLV[2]=0x01;			//Length
						tmpTLV[3]=ptrValue[0];	//Value
						rspCode=api_pcd_vap_Update_TLVData(tmpTLV, &cfgLen, cfgData);
						if (rspCode == TRUE)
							flgUpdate=TRUE;

						tmpTLV[0]=0x9F;			//Terminal Capabilities
						tmpTLV[1]=0x33;
						tmpTLV[2]=0x03;			//Length	
						memcpy(&tmpTLV[3], &ptrValue[1], 3);
						rspCode=api_pcd_vap_Update_TLVData(tmpTLV, &cfgLen, cfgData);
						if (rspCode == TRUE)
							flgUpdate=TRUE;

						tmpTLV[0]=0x9C;			//Transaction Type
						tmpTLV[1]=0x01;			//Length
						tmpTLV[2]=ptrValue[4];	//Value
						rspCode=api_pcd_vap_Update_TLVData(tmpTLV, &cfgLen, cfgData);
						if (rspCode == TRUE)
							flgUpdate=TRUE;

						if (flgUpdate == TRUE)
						{
							rspCode=FLS_Write_PayPassConfigurationData(cfgLen, cfgData);
							if (rspCode == SUCCESS)
							{
								ETP_Load_MPP_ConfigurationData();
								return SUCCESS;
							}
						}
					}
					else
					{
						tmpTLV[0]=0xDF;			//Tag
						tmpTLV[1]=0x81;
						
						if (intTag == 0xDF25)
							tmpTLV[2]=0x21;
						else if (intTag == 0xDF26)
							tmpTLV[2]=0x22;
						else	//0xDF27
							tmpTLV[2]=0x20;
						
						tmpTLV[3]=0x05;			//Length
						memcpy(&tmpTLV[4], &ptrValue[0], 5);
						
						rspCode=api_pcd_vap_Update_TLVData(tmpTLV, &cfgLen, cfgData);
						if (rspCode == TRUE)
						{
							rspCode=FLS_Write_PayPassConfigurationData(cfgLen, cfgData);
							if (rspCode == SUCCESS)
							{
								ETP_Load_MPP_ConfigurationData();
								return SUCCESS;
							}
						}
					}

					return SUCCESS;
				}
			}
		
			break;


		case 0xDF30:	//DPAS - Contactless Transaction Limit
			if ((lenOfV == ETP_PARA_SIZE_DF00) || (lenOfV == 0))
			{
				glv_parDF00Len[5]=lenOfV;
				
				if (lenOfV == 0)
				{
					memset(&glv_parDF00[5], 0, ETP_PARA_SIZE_DF00);
				}
				else
				{
					memcpy(&glv_parDF00[5], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF00);
				}
						
				return SUCCESS;
			}

			break;
			
		case 0xDF31:	//DPAS - CVM Required Limit
			if ((lenOfV == ETP_PARA_SIZE_DF01) || (lenOfV == 0))
			{
				glv_parDF01Len[5]=lenOfV;
				
				if (lenOfV == 0)
				{
					memset(&glv_parDF01[5], 0, ETP_PARA_SIZE_DF01);
				}
				else
				{
					memcpy(&glv_parDF01[5], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF01);
				}
				
				return SUCCESS;
			}

			break;

		case 0xDF32:	//DPAS - Contactless Floor Limit
			if ((lenOfV == ETP_PARA_SIZE_DF02) || (lenOfV == 0))
			{
				glv_parDF02Len[5]=lenOfV;
				
				if (lenOfV == 0)
				{
					memset(&glv_parDF02[5], 0, ETP_PARA_SIZE_DF02);
				}
				else
				{
					memcpy(&glv_parDF02[5], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF02);
				}
				
				return SUCCESS;
			}

			break;

		case 0xDF33:	//DPAS - Risk Parameter
			if (lenOfV == ETP_PARA_SIZE_DF06)
			{
				memcpy(&glv_parDF06[5], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF06);					
				
				return SUCCESS;
			}

			break;

		case 0xDF34:	//DPAS - Terminal Transaction Qualifier
			if (lenOfV == ETP_PARA_SIZE_9F66)
			{
				memcpy(&glv_par9F66[5], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_9F66);

				return SUCCESS;
			}

			break;

		case 0xDF35:	//DPAS - Terminal Type
			if (lenOfV == ECL_LENGTH_9F35)
			{
				memcpy(&glv_par9F35[5], &iptTLV[lenOfT+lenOfL], ECL_LENGTH_9F35);
				
				return SUCCESS;
			}
			
			break;

		case 0xDF36:	//DPAS - Terminal Application Version Number
			if (lenOfV == ECL_LENGTH_9F09)
			{
				memcpy(&glv_par9F09[5], &iptTLV[lenOfT+lenOfL], ECL_LENGTH_9F09);
				
				return SUCCESS;
			}
			
			break;


		//Reader Management Check
		case 0xDF41:
		//mapping DF41 to DF06
		/*
		==DF41==								==DF06==
		B2b8: RFU								B1b8: Status Check enabled/disabled
		B2b7: RFU								B1b7: Amount, Authorized of Zero Check enabled/disabled
		B2b6: Floor Limit Check						B1b6: Amount, Authorized of Zero Option (1b=Option 1 and 0b=Option 2)
		B2b5: CVM Limit Check						B1b5: Reader Contactless Transaction Limit Check enabled/disabled
		B2b4: Txn Limit Check						B1b4: Reader CVM Required Limit Check enabled/disabled
		B2b3: Zero Check Option 2					B1b3: Reader Contactless Floor Limit Check enabled/disabled
		B2b2: Zero Check Option 1					B1b2: Exception file enabled/disabled
		B2b1: Status Check						B2b8: Terminal Floor Limit Check enabled/disabled
		All other bits are RFU						All other bits are RFU
		*/	
			if (lenOfV == ETP_PARA_SIZE_DF06)
			{
				for(i=0;i<ETP_NUMBER_COMBINATION;i++)
				{
					if(i!=2)
					{
						memset(glv_parDF06[i],0x00,2);
					
						//Status Check
						if(iptTLV[lenOfT+lenOfL+1] & 0x01)
							glv_parDF06[i][0] |= 0x80;
						else
							glv_parDF06[i][0] &= 0x7F;

						//Zero Check
						if(iptTLV[lenOfT+lenOfL+1] & 0x06)
						{
							glv_parDF06[i][0] |= 0x40;

							//Zero Option 1
							if(iptTLV[lenOfT+lenOfL+1] & 0x02)
							{
								glv_parDF06[i][0] |= 0x20;
							}
							else	//Zero Option 2
							{
								glv_parDF06[i][0] &= 0xDF;
							}
						}
						else
						{
							glv_parDF06[i][0] &= 0xBF;
						}

						//Txn Limit Check
						if(iptTLV[lenOfT+lenOfL+1] & 0x08)
							glv_parDF06[i][0] |= 0x10;


						//CVM Limit Check
						if(iptTLV[lenOfT+lenOfL+1] & 0x10)
							glv_parDF06[i][0] |= 0x08;
						else
							glv_parDF06[i][0] &= 0xF7;

						//Floor Limit Check
						if(iptTLV[lenOfT+lenOfL+1] & 0x20)
							glv_parDF06[i][0] |= 0x04;
						else
							glv_parDF06[i][0] &= 0xFB;									
					}
					else		
					{
						memset(glv_parDF06_VISA[VISA_Purchase_Mode],0x00,2);
						
						//Status Check
						if(iptTLV[lenOfT+lenOfL+1] & 0x01)
							glv_parDF06_VISA[VISA_Purchase_Mode][0] |= 0x80;
						else
							glv_parDF06_VISA[VISA_Purchase_Mode][0] &= 0x7F;

						//Zero Check
						if(iptTLV[lenOfT+lenOfL+1] & 0x06)
						{
							glv_parDF06_VISA[VISA_Purchase_Mode][0] |= 0x40;

							//Zero Option 1
							if(iptTLV[lenOfT+lenOfL+1] & 0x02)
							{
								glv_parDF06_VISA[VISA_Purchase_Mode][0] |= 0x20;
							}
							else	//Zero Option 2
							{
								glv_parDF06_VISA[VISA_Purchase_Mode][0] &= 0xDF;
							}
						}
						else
						{
							glv_parDF06_VISA[VISA_Purchase_Mode][0] &= 0xBF;
						}

						//Txn Limit Check
						if(iptTLV[lenOfT+lenOfL+1] & 0x08)
							glv_parDF06_VISA[VISA_Purchase_Mode][0] |= 0x10;


						//CVM Limit Check
						if(iptTLV[lenOfT+lenOfL+1] & 0x10)
							glv_parDF06_VISA[VISA_Purchase_Mode][0] |= 0x08;
						else
							glv_parDF06_VISA[VISA_Purchase_Mode][0] &= 0xF7;

						//Floor Limit Check
						if(iptTLV[lenOfT+lenOfL+1] & 0x20)
							glv_parDF06_VISA[VISA_Purchase_Mode][0] |= 0x04;
						else
							glv_parDF06_VISA[VISA_Purchase_Mode][0] &= 0xFB;

					}					
				}
				
				return SUCCESS;
			}

			break;	//20140120 V1, Added

		case 0xDF50:	//AMEX - Application version number
			if (lenOfV == ECL_LENGTH_9F09)
			{
				memcpy(&glv_par9F09[3], &iptTLV[lenOfT+lenOfL], ECL_LENGTH_9F09);
				return SUCCESS;
			}
			break;
			
		case 0xDF51:	//AMEX - TAC Default
			if (lenOfV == ECL_LENGTH_DF8120)
			{
				memcpy(&glv_parDF8120[3], &iptTLV[lenOfT+lenOfL], ECL_LENGTH_DF8120);
				return SUCCESS;
			}
			break;
			
		case 0xDF52:	//AMEX - TAC Denial
			if (lenOfV == ECL_LENGTH_DF8121)
			{
				memcpy(&glv_parDF8121[3], &iptTLV[lenOfT+lenOfL], ECL_LENGTH_DF8121);
				return SUCCESS;
			}
			break;
			
		case 0xDF53:	//AMEX - TAC Online
			if (lenOfV == ECL_LENGTH_DF8122)
			{
				memcpy(&glv_parDF8122[3], &iptTLV[lenOfT+lenOfL], ECL_LENGTH_DF8122);
				return SUCCESS;
			}
			break;
			
		case 0xDF54:	//AMEX - Contactless Reader Capabilities
			if (lenOfV == 1)
			{
				memcpy(&glv_par9F6D[3], &iptTLV[lenOfT+lenOfL], 1);
				return SUCCESS;
			}
			break;
			
		case 0xDF55:	//AMEX - Enhanced Contactless Reader Capabilities
			if (lenOfV == 4)
			{
				memcpy(&glv_par9F6E[3], &iptTLV[lenOfT+lenOfL], 4);
				return SUCCESS;
			}
			break;
			
		case 0xDF56:	//AMEX - Terminal Type
			if (lenOfV == ECL_LENGTH_9F35)
			{
				memcpy(&glv_par9F35[3], &iptTLV[lenOfT+lenOfL], ECL_LENGTH_9F35);
				return SUCCESS;
			}
			break;

		case 0xDF57:	//AMEX - Unpredictable Number Range
			if (lenOfV == 1)
			{
				if (iptTLV[lenOfT+lenOfL] >= 60)
				{
					aex_rngUnpNumber=iptTLV[lenOfT+lenOfL];
					return SUCCESS;
				}
			}
			break;

		case 0xDF58:	//AMEX - Deactivation Timer
			if (lenOfV == 1)
			{
				if ((iptTLV[lenOfT+lenOfL] >= 10) && (iptTLV[lenOfT+lenOfL] <= 30))
				{
					aex_tmrDeactivation=iptTLV[lenOfT+lenOfL];
					return SUCCESS;
				}
			}
			break;

		case 0xDF59:	//AMEX - Function Parameter
			if (lenOfV == 1)
			{
				aex_parFunction=iptTLV[lenOfT+lenOfL];
				return SUCCESS;
			}
			break;

		case 0xDF5A:	//AMEX - Contactless Transaction Limit
			if (lenOfV == ETP_PARA_SIZE_DF00)
			{
				glv_parDF00Len[3]=lenOfV;
				memcpy(&glv_parDF00[3], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF00);
				
				return SUCCESS;
			}
			break;

		case 0xDF5B:	//AMEX - CVM Required Limit
			if (lenOfV == ETP_PARA_SIZE_DF01)
			{
				glv_parDF01Len[3]=lenOfV;
				memcpy(&glv_parDF01[3], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF01);
				
				return SUCCESS;
			}
			break;

		case 0xDF5C:	//AMEX - Contactless Floor Limit
			if (lenOfV == ETP_PARA_SIZE_DF02)
			{
				glv_parDF02Len[3]=lenOfV;
				memcpy(&glv_parDF02[3], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF02);
				
				return SUCCESS;
			}
			break;

		case 0xDF5D:	//AMEX - Reader Risk Parameter
			if (lenOfV == ETP_PARA_SIZE_DF06)
			{
				memcpy(&glv_parDF06[3], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF06);
				
				return SUCCESS;
			}

			break;
			
		//New JCB Kernel 5 Parameter
		case 0xDF60:	//JCB Contactless Transaction Limit
			if (lenOfV == 6)
			{
				glv_parDF00Len[4]=lenOfV;
				memcpy(&glv_parDF00[4], &iptTLV[lenOfT+lenOfL], 6);
				
				JCB_Static_Parameter.ClessTxnLimitLen = 6;
				memcpy(JCB_Static_Parameter.ClessTxnLimit,&iptTLV[lenOfT+lenOfL],ETP_PARA_SIZE_DF00);
				
				return SUCCESS;
			}
			else
			{
				if (lenOfV == 0)
				{
					glv_parDF00Len[4]=lenOfV;
					memset(&glv_parDF00[4], 0, 6);
				
					JCB_Static_Parameter.ClessTxnLimitLen = 0;
					memset(JCB_Static_Parameter.ClessTxnLimit,0x00,ETP_PARA_SIZE_DF00);
					
					return SUCCESS;
				}
			}

			break;
			
		case 0xDF61:	//JCB Reader CVM Required Limit
			if (lenOfV == ETP_PARA_SIZE_DF01)
			{
				glv_parDF01Len[4]=lenOfV;
				memcpy(&glv_parDF01[4], &iptTLV[lenOfT+lenOfL], 6);
					
				//for JCB Kernel 5
				JCB_Static_Parameter.ClessCVMLimitLen= ETP_PARA_SIZE_DF01;
				memcpy(JCB_Static_Parameter.ClessCVMLimit,&iptTLV[lenOfT+lenOfL],ETP_PARA_SIZE_DF01); 
				
				return SUCCESS;
			}

			if(lenOfV == 0)	//for JCB Kernel 5
			{
				glv_parDF01Len[4]=lenOfV;
				memset(&glv_parDF01[4], 0, 6);
					
				JCB_Static_Parameter.ClessCVMLimitLen = 0;
				memset(JCB_Static_Parameter.ClessCVMLimit,0x00,ETP_PARA_SIZE_DF01); 
				return SUCCESS;
			}

			break;
		
		case 0xDF62:	//JCB Reader CL Floor Limit
			if (lenOfV == ETP_PARA_SIZE_DF02)
			{
				glv_parDF02Len[4]=lenOfV;
				memcpy(&glv_parDF02[4], &iptTLV[lenOfT+lenOfL], 6);
				
				glv_parDF62_Len = lenOfV;	//20140120 V1, add

				//for JCB Kernel 5
				JCB_Static_Parameter.ClessFloorLimitLen= ETP_PARA_SIZE_DF02;
				memcpy(JCB_Static_Parameter.ClessFloorLimit,&iptTLV[lenOfT+lenOfL],ETP_PARA_SIZE_DF02); 
				
				return SUCCESS;
			}

			//20140120 V1, if DF62 is absent
			if(lenOfV == 0)
			{
				glv_parDF02Len[4]=lenOfV;
				memset(&glv_parDF02[4], 0, 6);
				
				glv_parDF62_Len = lenOfV;	//20140120 V1, add

				//for JCB Kernel 5
				JCB_Static_Parameter.ClessFloorLimitLen= 0;
				memset(JCB_Static_Parameter.ClessFloorLimit,0x00,ETP_PARA_SIZE_DF02); 
				
				return SUCCESS;
			}
			//20140120 V1, if DF62 is absent end

			break;

		case 0xDF63:	//Enhance DDA Ver Num
			if (lenOfV == ETP_PARA_SIZE_DF03)
			{
				//memcpy(&glv_parDF03[0], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF03);
				
				return SUCCESS;
			}

			break;

		case 0xDF64:	//CVM Require
			if (lenOfV == ETP_PARA_SIZE_DF04)
			{
				//memcpy(&glv_parDF04[0], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF04);
				return SUCCESS;
			}

			break;

		case 0xDF65:	//Display Offline Funds
			if (lenOfV == ETP_PARA_SIZE_DF05)
			{
				//memcpy(&glv_parDF05[0], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF05);
				return SUCCESS;
			}

			break;
			
		case 0xDF66:	//JCB Terminal Type
			if (lenOfV == ETP_PARA_SIZE_9F35)
			{
				//For JCB Kernel5
				JCB_Static_Parameter.TerminalType = iptTLV[lenOfT+lenOfL];
				
				return SUCCESS;
			}

			break;

		case 0xDF67:	//TTQ
			if (lenOfV == ETP_PARA_SIZE_9F66)
			{
				memcpy(&glv_par9F66[0], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_9F66);

//				ETP_Add_TransactionData(iptTLV);	//20140116 V1,Removed
				
				return SUCCESS;
			}

			break;
			
		case 0xDF68: //Combination Option
			if(lenOfV == 2)
			{
				memcpy(JCB_Static_Parameter.CombinationOption,&iptTLV[lenOfT+lenOfL],lenOfV);	

				return SUCCESS;
			}
			else
				return FAIL;	

		case 0xDF69:	//Terminal Interchange Profile
			if(lenOfV == 3)
			{					
				memcpy(JCB_Static_Parameter.TerminalInterchangeProfile,&iptTLV[lenOfT+lenOfL],lenOfV);

				return SUCCESS;
			}
			else
				return FAIL;
									
		case 0xDF6A:	//BRS MAX Target
			if(lenOfV == 1)
			{
				JCB_Static_Parameter.MAXTargetPercent_RandomSelection = iptTLV[lenOfT+lenOfL];

				return SUCCESS;
			}
			else
				return FAIL;
		
		case 0xDF6B:	//BRS Target
			if(lenOfV == 1)
			{
				JCB_Static_Parameter.TargetPercent_RandomSelection = iptTLV[lenOfT+lenOfL];

				return SUCCESS;
			}
			else
				return FAIL;
			
		case 0xDF6C:	//BRS Threshold
			if(lenOfV == 6)
			{
				memcpy(JCB_Static_Parameter.ThresholdValue_RandomSelection,&iptTLV[lenOfT+lenOfL],lenOfV);

				return SUCCESS;
			}
			else
				return FAIL;
		
		case 0xDF6D:	//TAC_Default
			if(lenOfV == 0)
			{
				JCB_Static_Parameter.TAC_Default_Present = FALSE;
				memset(JCB_Static_Parameter.TAC_Default,0x00,5);

				return SUCCESS;
			}
			else if(lenOfV == 5)
			{
				
				JCB_Static_Parameter.TAC_Default_Present = TRUE;
				memcpy(JCB_Static_Parameter.TAC_Default,&iptTLV[lenOfT+lenOfL],lenOfV);

				return SUCCESS;
			}
			else
				return FAIL;

		case 0xDF6E:	//TAC_Denial
			if(lenOfV == 0)
			{
				JCB_Static_Parameter.TAC_Denial_Present = FALSE;
				memset(JCB_Static_Parameter.TAC_Denial,0x00,5);

				return SUCCESS;
			}
			else if(lenOfV == 5)
			{
				
				JCB_Static_Parameter.TAC_Denial_Present = TRUE;
				memcpy(JCB_Static_Parameter.TAC_Denial,&iptTLV[lenOfT+lenOfL],lenOfV);

				return SUCCESS;
			}
			else
				return FAIL;

		case 0xDF6F:	//TAC_Online
			if(lenOfV == 0)
			{
				JCB_Static_Parameter.TAC_Online_Present = FALSE;
				memset(JCB_Static_Parameter.TAC_Online,0x00,5);

				return SUCCESS;
			}
			else if(lenOfV == 5)
			{
				
				JCB_Static_Parameter.TAC_Online_Present = TRUE;
				memcpy(JCB_Static_Parameter.TAC_Online,&iptTLV[lenOfT+lenOfL],lenOfV);

				return SUCCESS;
			}
			else
				return FAIL;


		case 0xDF70:	//qUICS - Contactless Transaction Limit
			if (lenOfV == ETP_PARA_SIZE_DF00)
			{
				glv_parDF00Len[6]=lenOfV;
				memcpy(&glv_parDF00[6], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF00);
				
				return SUCCESS;
			}
			break;

		case 0xDF71:	//qUICS - CVM Required Limit
			if (lenOfV == ETP_PARA_SIZE_DF01)
			{
				glv_parDF01Len[6]=lenOfV;
				memcpy(&glv_parDF01[6], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF01);
				
				return SUCCESS;
			}
			break;
			
		case 0xDF72:	//qUICS - Transaction Floor Limit
			if (lenOfV == ETP_PARA_SIZE_DF02)
			{
				glv_parDF02Len[6]=lenOfV;
				memcpy(&glv_parDF02[6], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF02);
				
				return SUCCESS;
			}
			break;

		case 0xDF73:	//qUICS - Reader Risk Parameter
			if (lenOfV == ETP_PARA_SIZE_DF06)
			{
				memcpy(&glv_parDF06[6], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_DF06);
				
				return SUCCESS;
			}

			break;
			
		case 0xDF77:	//qUICS - Terminal Transaction Qualifiers
			if (lenOfV == ETP_PARA_SIZE_9F66)
			{
				memcpy(&glv_par9F66[6], &iptTLV[lenOfT+lenOfL], ETP_PARA_SIZE_9F66);
				
				return SUCCESS;
			}
			break;

		default: break;
	}
	
	return FAIL;
}


void api_pcd_vap_Set_MessageLength(UINT iptMsgLength, UCHAR *optMsgLength)
{
	optMsgLength[0]=(iptMsgLength & 0xFF00) >> 8;
	optMsgLength[1]=iptMsgLength & 0x00FF;
}


UCHAR api_pcd_vap_Set_Parameter(UCHAR *iptIndex, UCHAR *iptLength, UCHAR *iptData)
{
	UCHAR	idxNum=0;
	UINT	parIndex=0;
	UINT	datLen=0;
	
	parIndex=iptIndex[0]*256+iptIndex[1];
	datLen=iptLength[0]*256+iptLength[1];

	for (idxNum=0; idxNum < VAP_PARAMETER_NUMBER; idxNum++)
	{
		if (!memcmp(glv_vap_Parameter[idxNum].Index, iptIndex, 2))
		{
			//Check Length
			switch (parIndex)
			{
				case 0x0005:
				case 0x0006:
					if (datLen != 1)
						return FAIL;
					else
						break;

				case 0x0001:
				case 0x0002:
				case 0x0003:
				case 0x0004:
				case 0x0007:
				case 0x0008:
				case 0x0009:
				case 0x000B:
				case 0x000C:
				case 0x000D:
				case 0x000E:
				case 0x000F:
				case 0x0010:
					if (datLen != 2)
						return FAIL;
					else
						break;

				case 0x000A:
					if (datLen > 16)
						return FAIL;
					else
						break;
			}

			//Copy Data			
			memcpy(glv_vap_Parameter[idxNum].Length, iptLength, 2);
			memcpy(glv_vap_Parameter[idxNum].Data, iptData, datLen);
			
			return SUCCESS;
		}
	}
	
	return FAIL;
}

UCHAR api_pcd_vap_Set_QuickPayMultipleAIDData(UINT iptLen, UCHAR *iptData)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	curLen=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	cntIndex=0;

	ptrData=iptData;

	for (cntIndex=0; cntIndex < FLS_NUMBER_AID_SET_UPI; cntIndex++)
	{
		//AID
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		
		glv_par9F06Len[6+cntIndex]=lenOfV;
		memcpy(glv_par9F06[6+cntIndex], &ptrData[lenOfT+lenOfL], lenOfV);
		
		curLen+=(lenOfT+lenOfL+lenOfV);
		ptrData+=(lenOfT+lenOfL+lenOfV);

		//TTQ
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		memcpy(glv_par9F66[6+cntIndex], &ptrData[lenOfT+lenOfL], lenOfV);

		curLen+=(lenOfT+lenOfL+lenOfV);
		ptrData+=(lenOfT+lenOfL+lenOfV);

		//Risk Parameter
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		memcpy(glv_parDF06[6+cntIndex], &ptrData[lenOfT+lenOfL], lenOfV);

		curLen+=(lenOfT+lenOfL+lenOfV);
		ptrData+=(lenOfT+lenOfL+lenOfV);

		//Transaction Limit
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		glv_parDF00Len[6+cntIndex]=lenOfV;
		memcpy(glv_parDF00[6+cntIndex], &ptrData[lenOfT+lenOfL], lenOfV);

		curLen+=(lenOfT+lenOfL+lenOfV);
		ptrData+=(lenOfT+lenOfL+lenOfV);

		//Check CVM Required Limit
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		glv_parDF01Len[6+cntIndex]=lenOfV;
		memcpy(glv_parDF01[6+cntIndex], &ptrData[lenOfT+lenOfL], lenOfV);

		curLen+=(lenOfT+lenOfL+lenOfV);
		ptrData+=(lenOfT+lenOfL+lenOfV);

		//CL Floor Limit
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		glv_parDF02Len[6+cntIndex]=lenOfV;
		memcpy(glv_parDF02[6+cntIndex], &ptrData[lenOfT+lenOfL], lenOfV);

		curLen+=(lenOfT+lenOfL+lenOfV);
		ptrData+=(lenOfT+lenOfL+lenOfV);

		if (iptLen == curLen)
		{
			return SUCCESS;
		}
	}	
	
	return FAIL;
}

UCHAR api_pcd_vap_Set_DPASMultipleAIDData(UINT iptLen, UCHAR *iptData)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	curLen=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	cntIndex=0;

	ptrData=iptData;

	for (cntIndex=0; cntIndex < FLS_NUMBER_AID_SET_DISCOVER; cntIndex++)
	{
		if (cntIndex == 1) cntIndex+=3;	//DPAS index is 5 & 9
		
		//AID
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		
		glv_par9F06Len[5+cntIndex]=lenOfV;
		memcpy(glv_par9F06[5+cntIndex], &ptrData[lenOfT+lenOfL], lenOfV);
		
		curLen+=(lenOfT+lenOfL+lenOfV);
		ptrData+=(lenOfT+lenOfL+lenOfV);

		//TTQ
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		memcpy(glv_par9F66[5+cntIndex], &ptrData[lenOfT+lenOfL], lenOfV);

		curLen+=(lenOfT+lenOfL+lenOfV);
		ptrData+=(lenOfT+lenOfL+lenOfV);

		//Risk Parameter
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		memcpy(glv_parDF06[5+cntIndex], &ptrData[lenOfT+lenOfL], lenOfV);

		curLen+=(lenOfT+lenOfL+lenOfV);
		ptrData+=(lenOfT+lenOfL+lenOfV);

		//Transaction Limit
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		memcpy(glv_parDF00[5+cntIndex], &ptrData[lenOfT+lenOfL], lenOfV);

		curLen+=(lenOfT+lenOfL+lenOfV);
		ptrData+=(lenOfT+lenOfL+lenOfV);

		//Check CVM Required Limit
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		memcpy(glv_parDF01[5+cntIndex], &ptrData[lenOfT+lenOfL], lenOfV);

		curLen+=(lenOfT+lenOfL+lenOfV);
		ptrData+=(lenOfT+lenOfL+lenOfV);

		//CL Floor Limit
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		memcpy(glv_parDF02[5+cntIndex], &ptrData[lenOfT+lenOfL], lenOfV);

		curLen+=(lenOfT+lenOfL+lenOfV);
		ptrData+=(lenOfT+lenOfL+lenOfV);

		if (iptLen == curLen)
		{
			return SUCCESS;
		}
	}	
	
	return FAIL;
}



//	*****************************************
//	*** VISA Reader Interface Instruction ***
//	*****************************************

void api_pcd_vap_pol_POLL(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;

	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 0)
	{
		iptData=iptData;	//Remove Warning
		
		if (api_pcd_vap_flgMutAuthenticate == TRUE)
		{
			optData[0]=VAP_RIF_RC_POLL_A;
		}
		else
		{
			optData[0]=VAP_RIF_RC_POLL_P;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_POLL_N;
	}
	
	api_pcd_vap_Set_MessageLength(0x0001, optMsgLength);
}


void api_pcd_vap_pol_Echo(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;

	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
	
	optData[0]=VAP_RIF_RC_SUCCESS;
	memcpy(&optData[1], iptData, msgLength);
}


void api_pcd_vap_deb_SetDebugAndOptimizationMode(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	iptMode=0xFF;
	UINT	msgLength=0;

	//20131220 it should perform Mutual Auth first
	if(api_pcd_vap_flgMutAuthenticate == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (msgLength == 1)
		{
			iptMode=iptData[0];
			
			if (iptMode == 0)
			{
				api_pcd_vap_flgDbgAndOptMode=TRUE;

				api_pcd_vap_flgNormalMode=FALSE;		//20131126 add normal mode

				optData[0]=VAP_RIF_RC_SUCCESS;
			}
			else if (iptMode == 1)
			{
				api_pcd_vap_flgDbgAndOptMode=FALSE;

				api_pcd_vap_flgNormalMode=TRUE;		//20131126 add normal mode

				optData[0]=VAP_RIF_RC_SUCCESS;
			}
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_DATA;
		}		
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
	api_pcd_vap_Set_MessageLength(1, optMsgLength);
}


void api_pcd_vap_deb_SetParameters(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	curParNumber=0;
	UCHAR	parLength=0;
	UCHAR	flgError=FALSE;
	UCHAR	rspCode=0;
	UCHAR	*ptrData=NULLPTR;
	UINT	parNumber=0;
	UINT	msgLength=0;
	UINT	Para_Index = 0;
	UINT 	P_LAUGUAGE_Len = 0;
#ifndef _PLATFORM_AS350
	UCHAR	ToFlashParaData[128] = {0};
	UCHAR	*ToFlashPtr=NULLPTR;
	UINT	ToFlashParaLen = 0;
	UINT	tmpLen = 0;
	UCHAR	i = 0;
#endif

	if (api_pcd_vap_flgDbgAndOptMode == TRUE)
	{
		VAP_Reader_Index_Change = FALSE;
	
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);

		parNumber=iptData[0]*256+iptData[1];
		ptrData=&iptData[2];

		do {
			rspCode=api_pcd_vap_Set_Parameter(&ptrData[0], &ptrData[2], &ptrData[4]);
			if (rspCode == SUCCESS)
			{
				Para_Index = ptrData[0]*256 + ptrData[1];
				
				switch (Para_Index)
				{
					case 0x0001:	//P_MSG_TIMEOUT

						VAP_VISA_P_MSG_TIMEOUT = (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10;
						
						break;
						
					case 0x0002:	//P_SALE_TIMEOUT
						//20130829, os_SysTimerFreeCnt is count by a 10ms time unit, ï¿½ï¿½ï¿½ï¿½à´?
						//etp_tmrSale = (ptrData[4]*256 + ptrData[5])/10;
						VAP_VISA_P_SALE_TIMEOUT = (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10;		

						etp_tmrSale = VAP_VISA_P_SALE_TIMEOUT;

						break;

					//20140106 ICTK set 30sec for a unit, so it need transafer the unit to ms
					case 0x0003: 	//P_POLL_MSG

						VAP_VISA_P_POLL_MSG	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])*100;

						break;
					
					case 0x0004: 	//P_BUF_TIMEOUT

						VAP_VISA_P_BUF_TIMEOUT	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10;

						break;
						
					case 0x0005:	//P_ENCRYPTION

						VAP_VISA_P_ENCRYPTION	= glv_vap_Parameter[Para_Index-1].Data[0];

						break;
						
					case 0x0006: 	//P_DISPLAY

						VAP_VISA_P_DISPLAY = glv_vap_Parameter[Para_Index-1].Data[0];
						
						break;
						
					case 0x0007:	//P_MAX_BUF_SIZE

						VAP_VISA_P_MAX_BUF_SIZE = (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1]); 

						break;
						
					case 0x0008:	//P_DOUBLE_DIP

						VAP_VISA_P_DOUBLE_DIP = (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10; 

						break;
						
					case 0x0009:	//P_READER_INDEX

						VAP_Reader_Index_Change = TRUE;
						
						VAP_VISA_P_READER_INDEX	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1]); 

						break;

					case 0x000A:	//P_LAUGUAGE

						P_LAUGUAGE_Len = glv_vap_Parameter[Para_Index-1].Length[0]*256 + glv_vap_Parameter[Para_Index-1].Length[1];

						memcpy(VAP_VISA_P_LAUGUAGE,glv_vap_Parameter[Para_Index-1].Data,P_LAUGUAGE_Len);
						
						break;
						
					case 0x000B:	//P_DISPLAY_S_MSG

						VAP_VISA_P_DISPLAY_S_MSG	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10; 

						break;
						
					case 0x000C:	//P_DISPLAY_L_MSG

						VAP_VISA_P_DISPLAY_L_MSG	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10; 

						break;
						
					case 0x000D:	//P_DISPLAY_SS_MSG

						VAP_VISA_P_DISPLAY_SS_MSG	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10; 

						break;
						
					case 0x000E:	//P_DISPLAY_SR_MSG

						VAP_VISA_P_DISPLAY_SR_MSG	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10; 

						break;
						
					case 0x000F:	//P_DISPLAY_PIN_MSG

						VAP_VISA_P_DISPLAY_PIN_MSG	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10; 

						break;
						
					case 0x0010:	//P_DISPLAY_E_MSG

						VAP_VISA_P_DISPLAY_E_MSG	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10; 

						break;
						
				}
						
				parLength=ptrData[2]*256+ptrData[3];
				ptrData+=(4+parLength);
				curParNumber++;
			}
			else
			{
				flgError=TRUE;
				break;
			}
		} while (curParNumber < parNumber);

		if (flgError == TRUE)
		{
			optData[0]=VAP_RIF_RC_INVALID_DATA;
		}
		else
		{
			optData[0]=VAP_RIF_RC_SUCCESS;

#ifndef _PLATFORM_AS350
			ToFlashPtr = ToFlashParaData;
			//20140106 write to flash
			for(i=0;i<16;i++)
			{
				//handle Index
				memcpy(ToFlashPtr,glv_vap_Parameter[i].Index,2);
				ToFlashPtr+=2;

				//handle Length
				memcpy(ToFlashPtr,glv_vap_Parameter[i].Length,2);
				ToFlashPtr+=2;

				//handle data
				tmpLen = glv_vap_Parameter[i].Length[0]*256 + glv_vap_Parameter[i].Length[1];
				memcpy(ToFlashPtr,glv_vap_Parameter[i].Data,tmpLen);
				ToFlashPtr+=tmpLen;

				ToFlashParaLen += 2+2+tmpLen;
			}
			
#ifdef	_PLATFORM_AS210
			rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
			rspCode = apiOK;
#endif
			if(rspCode == apiOK)
				api_fls_write(FLSID_PARA,FLS_ADDRESS_Parameter,ToFlashParaLen,ToFlashParaData);
#endif
		}		
	}
	else
	{
		optData[0]=VAP_RIF_RC_FAILURE;
	}

	api_pcd_vap_Set_MessageLength(1, optMsgLength);
}


void api_pcd_vap_aut_InitializeCommunication(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	iptKeyType=0;
	UCHAR	iptKeyIndex=0;
	UCHAR	iptRndB[8]={0};
	UCHAR	desKey[24]={0};
	UCHAR	tmpData1[8]={0};
	UCHAR	tmpData2[8]={0};
	UCHAR	tmpKey[16]={0};
	UCHAR	optCryptogram[16]={0};
	UCHAR	rspCode=0;
	UINT	msgLength=0;
	UCHAR	testrndR[8] = {0};

	//20140107
	UCHAR	emptyKey[16] = {0};

	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 10)
	{
		iptKeyType=iptData[0];
		iptKeyIndex=iptData[1];
		memcpy(iptRndB, &iptData[2], 8);
		
		memcpy(api_pcd_vap_rndB, iptRndB, 8);
		//api_sys_random(api_pcd_vap_rndR);
		api_sys_random(testrndR);
		memcpy(api_pcd_vap_rndR,testrndR,8);

		rspCode=FLS_Read_EncryptionKey(iptKeyType, iptKeyIndex, tmpKey);
		if (rspCode == SUCCESS)
		{
			//20140107
			memset(emptyKey,0xFF,16);
			
			if(((iptKeyType == 0x02)||(iptKeyType == 0x06)) && (!memcmp(emptyKey,tmpKey,16)))
			{
				msgLength=1;
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			}
			else
			{
				memcpy(&desKey[0],	tmpKey, 16);
				memcpy(&desKey[16],	tmpKey, 8);

				memcpy(&tmpData1[0], &api_pcd_vap_rndR[4], 4);
				memcpy(&tmpData1[4], &api_pcd_vap_rndB[0], 4);
				memcpy(&tmpData2[0], &api_pcd_vap_rndR[0], 4);
				memcpy(&tmpData2[4], &api_pcd_vap_rndB[4], 4);
				
				//TDEA [IMEK, RND_R(5:8), RND_B(1:4), RND_R(1:4), RND_B(5:8)]
				api_3des_encipher(tmpData1, &api_pcd_vap_autSesKey[0], desKey);
				api_3des_encipher(tmpData2, &api_pcd_vap_autSesKey[8], desKey);

				memcpy(&desKey[0], api_pcd_vap_autSesKey, 16);
				memcpy(&desKey[16],api_pcd_vap_autSesKey, 8);
				
				//TDEA (IMEK authentication session key, RND_B, api_pcd_vap_rndR)
				api_3des_encipher(api_pcd_vap_rndB, &optCryptogram[0], desKey);
				api_3des_encipher(api_pcd_vap_rndR, &optCryptogram[8], desKey);

				msgLength=27;
				optData[0]=VAP_RIF_RC_SUCCESS;
				optData[1]=iptKeyType;
				optData[2]=iptKeyIndex;
				memcpy(&optData[3], api_pcd_vap_rndR, 8);
				memcpy(&optData[11], optCryptogram, 16);
			}
		}
		else
		{
			msgLength=1;
			optData[0]=VAP_RIF_RC_AUTH_FAILURE;
		}
	}
	else
	{
		msgLength=1;
		optData[0]=VAP_RIF_RC_INVALID_DATA;
	}
	
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}


void api_pcd_vap_aut_MutualAauthenticate(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	iptKeyType=0;
	UCHAR	iptKeyIndex=0;
	UCHAR	iptCryptogram[16]={0};
	UCHAR	desKey[24]={0};
	UCHAR	tmpData1[8]={0};
	UCHAR	tmpData2[8]={0};
	UCHAR	rcvRndB[8]={0};
	UCHAR	rcvRndR[8]={0};
	UINT	msgLength=0;

	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 18)
	{
		iptKeyType=iptData[0];
		iptKeyIndex=iptData[1];
		memcpy(iptCryptogram, &iptData[2], 16);
		
		memcpy(&desKey[0],	api_pcd_vap_autSesKey, 16);
		memcpy(&desKey[16],	api_pcd_vap_autSesKey, 8);
		
		//TDEA (IMEK authentication session key, RND_B (5:8), RND_R(1:4), RND_B(1:4), RND_R(5:8))
		api_3des_decipher(tmpData1, &iptCryptogram[0], desKey);
		api_3des_decipher(tmpData2, &iptCryptogram[8], desKey);

		memcpy(&rcvRndB[4], &tmpData1[0], 4);
		memcpy(&rcvRndR[0], &tmpData1[4], 4);
		memcpy(&rcvRndB[0], &tmpData2[0], 4);
		memcpy(&rcvRndR[4], &tmpData2[4], 4);

		if ((!memcmp(rcvRndB, api_pcd_vap_rndB, 8)) &&
			(!memcmp(rcvRndR, api_pcd_vap_rndR, 8)))
		{
			api_pcd_vap_flgMutAuthenticate=TRUE;

			main_clean_screen = TRUE;

			//20140108
			api_pcd_vap_flgAdmMode = FALSE;
			api_pcd_vap_flgDbgAndOptMode = FALSE;
			api_pcd_vap_flgNormalMode = TRUE;
			//20140108 end

			//20140114 V4, add for some display problem
			main_txn_complete = TRUE;
			main_card_remove = TRUE;
			//20140114 V4, add for some display problem end
			
			optData[0]=VAP_RIF_RC_SUCCESS;
		}
		else
		{
			optData[0]=VAP_RIF_RC_AUTH_FAILURE;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_INVALID_DATA;
	}
	
	api_pcd_vap_Set_MessageLength(0x0001, optMsgLength);
}


void api_pcd_vap_aut_GenerateKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	iptKeyType=0;
	UCHAR	iptKeyIndex=0;
	UCHAR	iptEncRND[16]={0};
	UCHAR	desKey[24]={0};
	UCHAR	tmpKey[16]={0};
	UCHAR	rspCode=FAIL;
	UINT	msgLength=0;

	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 18)
	{
		iptKeyType=iptData[0];
		iptKeyIndex=iptData[1];
		memcpy(iptEncRND, &iptData[2], 16);
		
		memcpy(&desKey[0],	api_pcd_vap_autSesKey, 16);
		memcpy(&desKey[16],	api_pcd_vap_autSesKey, 8);
		
		api_3des_decipher(&tmpKey[0], &iptEncRND[0], desKey);
		api_3des_decipher(&tmpKey[8], &iptEncRND[8], desKey);

		rspCode=FLS_Write_EncryptionKey(iptKeyType, iptKeyIndex, tmpKey);
		if (rspCode == SUCCESS)
		{
			//20140107 change IMEKmdk IAEKmdk
			//IMEKmdk will be replace by IMEK
			if(iptKeyType == 0x01)	
			{
				rspCode=FLS_Write_EncryptionKey(0x00, 0x00, tmpKey);
			}
			//IAEKmdk will be replace by IAEK
			if(iptKeyType == 0x05)	
			{
				rspCode=FLS_Write_EncryptionKey(0x04, 0x00, tmpKey);
			}
			optData[0]=VAP_RIF_RC_SUCCESS;
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_KEYINDEX;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_FAILURE;
	}
	
	api_pcd_vap_Set_MessageLength(0x0001, optMsgLength);
}


void api_pcd_vap_aut_InvalidateReader(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;
	UCHAR	MEK_Key[16];
	UCHAR	AEK_Key[16];

	//this command can be perform with out Mutual Authentication
	if((api_pcd_vap_flgMutAuthenticate == TRUE) || (api_pcd_vap_flgMutAuthenticate == FALSE))
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (msgLength == 0)
		{
			iptData=iptData;	//Remove Warning
			
			memset(api_pcd_vap_rndB, 0, 8);
			memset(api_pcd_vap_rndR, 0, 8);
			memset(api_pcd_vap_autSesKey, 16, 8);

	//20131223
			
	//Reset Authenticate and turn clean screen on
			api_pcd_vap_flgMutAuthenticate = FALSE;
			main_clean_screen = TRUE;
			/*api_pcd_vap_flgPayment = FALSE;
			api_pcd_vap_flgAdminCommand = FALSE;		*/ //20140108 changed

			//20140108
			api_pcd_vap_flgAdmMode = FALSE;
			api_pcd_vap_flgDbgAndOptMode = FALSE;

	//Reset Key
			memset(MEK_Key,0xFF,sizeof(MEK_Key));
#ifdef _PLATFORM_AS210
			api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#endif 
			api_fls_write(FLSID_PARA,FLS_ADDRESS_MEK,16,MEK_Key);
			
			memset(VISA_MSession_Key,0x00,sizeof(VISA_MSession_Key));
			
			memset(AEK_Key,0xFF,sizeof(AEK_Key));
#ifdef _PLATFORM_AS210
			api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#endif 
			api_fls_write(FLSID_PARA,FLS_ADDRESS_AEK,16,AEK_Key);
			
			memset(VISA_ASession_Key,0x00,sizeof(VISA_ASession_Key));	

	//Reset Buffer
//Reset Buffer Outside of api_pcd_vap_ is not Logical
//			memset(vap_rif_rcvBuffer,0x00,sizeof(vap_rif_rcvBuffer));
//			memset(vap_rif_sndBuffer,0x00,sizeof(vap_rif_sndBuffer));

	//20140106 turn off card scanning funciton
			ECL_LV1_FIELD_OFF();

	//20140107
			api_InvalidateReader = TRUE;

			optData[0]=VAP_RIF_RC_SUCCESS;
		}
		else
		{
			optData[0]=VAP_RIF_RC_FAILURE;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
	
	api_pcd_vap_Set_MessageLength(0x0001, optMsgLength);
}


void api_pcd_vap_tra_ReadyForSale(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	//20140128 show status command
	UCHAR ShowMsgID32[5] = {0x00,0x03,0x01,0x01,0x20};
	UCHAR ShowMsgID6[5] = {0x00,0x03,0x01,0x01,0x06};
	UCHAR ShowMsgID8[5] = {0x00,0x03,0x01,0x01,0x08};
	UCHAR ShowMsgID7[5] = {0x00,0x03,0x01,0x01,0x07};
	UCHAR ShowMsgID9[5] = {0x00,0x03,0x01,0x01,0x09};
	UCHAR ShowMsgRsp[10] = {0x00};

	//20140114 cashback amout
	UCHAR salAmount[6]={0};
	UCHAR salAmountOther[6]={0};
	
	//20140110
	UCHAR	SchemeErr = TRUE;

	//20140107
	UCHAR 	firstfourbytedata[4] = {0};
	UCHAR	firstfourlen=0;

	UCHAR	rtc[16]={0},rtcDate[3]={0},rtcTime[3]={0};
	UINT	optLen=0;
	UCHAR	tstRsp[1024] = {0},Out_Data[1024] = {0},D4_len=0;
	//UCHAR   *tag_addr = 0;
	UINT	tstRspLen=0,Out_Data_Len = 0,i=0;//,tag_len=0;

	UCHAR   E_Key[16]={0};
	//disp amount
	UCHAR	amt[6]={0},Disp_amt[10]={0},Disp_amt_dot[13] = {0},total_amt_len=0;
	UINT 	j=0;
	//UCHAR 	msg_amount[]={"Amt:NT$"};
	UCHAR rspCode = 0;

	UINT	Input_Length = 0;

	UCHAR	amountcheck[24] = {0x00};
	UCHAR	amountLen = 0;
	UCHAR 	AmountErrFlag=FALSE;

	UINT	lenOfFF8106=0;

	const	UCHAR	zroAmount[6]={0};
	UCHAR	Disp_amt_final[16];
	
#ifdef _ACQUIRER_NCCC
	UCHAR	Disp_amt_com[16];
	UCHAR	*ptrInteger=NULLPTR;
	UCHAR	idxInteger=0;
#endif


	//20140115 V3, if any RFS Command occured, Reset this flag
	api_Issuer_Update_Success_Flag_For_AOSA = FALSE;
	//20140115 V3, if any RFS Command occured, Reset this flag end

	//20140110 check all Scheme
	for(i=0;i<VAP_SCHEME_NUMBER;i++)
	{
		if(glv_vap_Scheme[i].Support == 0x01)
		{
			SchemeErr = FALSE;
			break;
		}
	}	

	//2014010 check the command (amount)	
	amountLen = iptMsgLength[1];

	UT_Split(amountcheck,iptData,amountLen);

	for(i=0;i<amountLen*2;i++)
	{
		if((amountcheck[i] < 0x30) || (amountcheck[i] > 0x39))
		{
			AmountErrFlag = TRUE;
			break;
		}
	}
	
	if((AmountErrFlag == FALSE) && (SchemeErr == FALSE))
	{
		i = 0;
		
		Input_Length = iptMsgLength[0]*256 + iptMsgLength[1];
		
		iptData[0]=iptData[0];
		
		if ((api_pcd_vap_flgMutAuthenticate == TRUE) && (api_pcd_vap_flgNormalMode == TRUE) && (api_InvalidateReader == FALSE))	//20140107 add api_InvalidateReader
		{

#ifdef _PLATFORM_AS350
			apk_tsc_switch(1);
#endif

			if (api_pcd_vap_flgRFSBeep == TRUE)
			{
				
#ifdef _SCREEN_SIZE_128x64
				UT_ClearScreen();
#else

#ifdef _PLATFORM_AS350_LITE
				UT_ClearScreen();
#endif
				// Display Contactless logo
				UT_LED_SingleAct(IID_TAP_LOGO,TRUE);

#ifdef _PLATFORM_AS350_LITE
#ifdef _ACQUIRER_CTCB
				PIC_QShowPicture(10,240,48,0,228);
				PIC_QShowPicture(11,240,44,0,276);
#endif
#endif

#endif

				//Beep
				UT_BUZ_Beep1();
			}
			else
			{
				//Reset Flag
				api_pcd_vap_flgRFSBeep=TRUE;
			}

			//20130923
			main_txn_complete = FALSE;
			printf("Start Load_CAPK\n");
			fflush(stdout);
			rspCode = Load_CAPK();
			// Wayne 2020/09/07
			printf("Finshed Load_CAPK\n");
			fflush(stdout);
	
			if(rspCode == FAIL)
			{
				UT_PutStr(0,0,FONT0,13,(UCHAR *)"Load Key Fail");
				UT_WaitKey();
			}

			//Reset VLP Auth Code 9F74
			api_Tag9F74_Data_Length = 0;
	
			//20140107 reset Available_Offline_Amount
			memset(Available_Offline_Amount,0x00,7);
			memset(Available_Offline_Amount_AP,0x00,7);

			memset(api_tag9F39,0x00,sizeof(api_tag9F39));
			memset(api_tagDF0E,0x00,sizeof(api_tagDF0E));
			memset(api_W3_DDAFail,0x00,sizeof(api_W3_DDAFail));
	
			//20131225 reset SchemeID
			SchemeID = 0;
	
			L3_Response_Code = 0xFF;
	
			//20140014 for cashback transaction, add amount other
			//20131115 Display amount
			if(Input_Length == 12)
			{
				memcpy(salAmount,iptData,6);
				memcpy(salAmountOther,&iptData[6],6);
				UT_bcd_add_bcd(6,salAmount,salAmountOther);
				memcpy(amt,salAmount,6);
			}
			else
			{
				memcpy(amt,iptData,6);
			}

			UT_Split(Disp_amt,amt,5);

			memcpy(Disp_amt_dot,Disp_amt,10);
			Disp_amt_dot[10]=0x2E;
			UT_Split(&Disp_amt_dot[11],&amt[5],1);

			total_amt_len=(amt[5] == 0)?(10):(13);	//Exclude .00

			for(j=0;j<=total_amt_len;j++)
			{
				if((Disp_amt_dot[j] != 0x30) && (Disp_amt_dot[j] != 0x2E))
				{
					break;
				}

				if((Disp_amt_dot[j] == 0x30) && (Disp_amt_dot[j+1] == 0x2E))
				{
					break;
				}
			}

			memset(&Disp_amt_final[0], '0', 16);
			memcpy(&Disp_amt_final[3], Disp_amt_dot, 13);

#ifdef _ACQUIRER_NCCC
			memset(&Disp_amt_com[0], '0', 16);
			memcpy(&Disp_amt_com[13], &Disp_amt_dot[10], 3);

			ptrInteger=&Disp_amt_com[12];

			for (idxInteger=0; idxInteger < (10-j); idxInteger++)
			{
				*ptrInteger--=Disp_amt_dot[9-idxInteger];

				if (((idxInteger+1)%3 == 0) && ((idxInteger+1) != (10-j)))
				{
					*ptrInteger--=',';
				}
			}

			memcpy(Disp_amt_final, Disp_amt_com, 16);
#endif

			total_amt_len=(amt[5] == 0)?(13):(16);	//Exclude .00

			for(j=0;j<=total_amt_len;j++)
			{
				if((Disp_amt_final[j] != 0x30) && (Disp_amt_final[j] != 0x2E))
				{
					break;
				}

				if((Disp_amt_final[j] == 0x30) && (Disp_amt_final[j+1] == 0x2E))
				{
					break;
				}
			}

			memcpy(api_msg_amount,glv_vap_Message[30].Message,glv_vap_Message[30].Length);
			api_msg_amount_len+=glv_vap_Message[30].Length;
			
			api_msg_amount[api_msg_amount_len] = 0x24;
			api_msg_amount_len+=1;
	
			if(!memcmp(amt, zroAmount, 6))	//amt == 0
			{
				api_msg_amount[api_msg_amount_len] = 0x30;
				api_msg_amount_len+=1;
			}
			else	//amt != 0
			{
				memcpy(&api_msg_amount[api_msg_amount_len],&Disp_amt_final[j],(total_amt_len-j)); 
				api_msg_amount_len +=  (total_amt_len-j);				
			}
			//20131115 Display amount end

			// Wayne 2020/09/07
			printf("Start to choose Purchase or Cashback\n");
			fflush(stdout);


			printf("glv_par9c:%02x\n",glv_par9C[0]);
			fflush(stdout);

			//Enter the Entrypoint, L3_Response_Code will change depends on situation
			if(glv_par9C[0] == 0x00) // Purchase or Cashback
			{
				if(Input_Length == 6)
					rspCode = api_pcd_qVSDC_Purchase(iptData,&Out_Data_Len,Out_Data);
				else
					rspCode = api_pcd_qVSDC_Purchase_Cashback(&iptData[0],&iptData[6],&Out_Data_Len,Out_Data);
			}
			else if(glv_par9C[0] == 0x01)	//CASH Mode
			{
				rspCode = api_pcd_qVSDC_Cash(iptData,&Out_Data_Len,Out_Data);
			}
			else if(glv_par9C[0] == 0x09)	//JCB Cashback mode
			{
				rspCode = api_pcd_JCB_Purchase_Cashback(&iptData[0],&iptData[6],&Out_Data_Len,Out_Data);
			}
			else if(glv_par9C[0] == 0x20)	//Refund Mode
			{
				rspCode = api_pcd_qVSDC_Refund(iptData,&Out_Data_Len,Out_Data);
			}
			else
			{
				UT_PutStr(1,0,FONT0,22,(UCHAR *)"Transaction Type Error");
				UT_WaitTime(50);
			}

			// Wayne 2020/09/07
			printf("End to choose Purchase or Cashback\n");
			fflush(stdout);


			if (L3_Response_Code == VAP_RIF_RC_FAILURE)
			{
				rspCode=api_pcd_vap_Check_JSpeedyRefundDecline();
				if ((rspCode == TRUE) && (api_pcd_vap_flgJCBRefResponse == TRUE))
				{
					L3_Response_Code=VAP_RIF_RC_JCB_REFUND_DECLINE;

					VGL_AS210_D1_Track();
					SchemeID=JCB_GET_SchemeID();
				}
			}
		
			//Check the Response Code
			if ((L3_Response_Code == VAP_RIF_RC_DATA) ||
				(L3_Response_Code == VAP_RIF_RC_JCB_REFUND_DECLINE))
			{	
				//20140303 check scheme ID support or not
				if(API_Check_SchemeID(SchemeID) == FAIL)
				{
					L3_Response_Code = VAP_RIF_RC_FAILURE;
				}
				else
				{
					
#ifdef _PLATFORM_AS350
					//VCPS 2.1.3 Update - Increase Transaction Sequence Counter
					api_emvk_IncTransSequenceCounter();
#endif
					tstRsp[i++] = VAP_RIF_RC_DATA;	//RC_DATA

					tstRsp[i++] = SchemeID; //Scheme ID

					tstRsp[i++] = 0x20; //country year

					UT_GetDateTime(rtc);

					//Date			
					UT_Compress(rtcDate,rtc,3);
					memcpy(&tstRsp[i],rtcDate,3);
					i+=3;

					//Time
					UT_Compress(rtcTime,&rtc[6],3);
					memcpy(&tstRsp[i],rtcTime,3);
					i+=3;

					tstRsp[i++] = 0xD1; //D1

					tstRsp[i++] = D1_Data_Length;

					if(D1_Data_Length)					//D1
					{		
						memcpy(&tstRsp[i],D1_Data,D1_Data_Length);
						i+=D1_Data_Length;
					}

					tstRsp[i++] = 0xD2; //D2

					tstRsp[i++] = Tag57_Data_Length;

					if(Tag57_Data_Length)					//D2
					{		
						memcpy(&tstRsp[i],Tag57_Data,Tag57_Data_Length);
						i+=Tag57_Data_Length;
					}

					tstRsp[i++] = 0xD3; //D3

					//20140207 detect the tag DF0F, DF1F.
					//In VISA, JCB, I use DF0F DF1F to represent the transaction is Online or Offline
					if((Out_Data[0] == 0xDF) && ((Out_Data[1] == 0x0F) || (Out_Data[1] == 0x1F)))
					{
						Out_Data_Len -=4;	//minus first four bytes
						firstfourlen = 4;
						memcpy(firstfourbytedata,Out_Data,4);
					}
					else
					{
						Out_Data_Len = Out_Data_Len;	
						firstfourlen = 0;
						memset(firstfourbytedata,0x00,4);
					}

					if(Out_Data_Len < 128)
					{
						//20140107
						tstRsp[i++] = Out_Data_Len;
					}
					else
					{
						if(Out_Data_Len <256)
						{
							tstRsp[i++] = 0x81;
							tstRsp[i++] = Out_Data_Len; 				
						}
						else
						{
							tstRsp[i++] = 0x82;
							tstRsp[i++] = (Out_Data_Len & 0xFF00)>>8;
							tstRsp[i++] = Out_Data_Len & 0x00FF;
						}
					}		

					//20140207 , PayWave, NewJSpeedy Data
					if(firstfourlen)  
					{
						memcpy(&tstRsp[i],&Out_Data[4],Out_Data_Len);
					}
					else	//PayPass data
					{
						memcpy(&tstRsp[i],Out_Data,Out_Data_Len);
					}

					i+=Out_Data_Len;

					//Other data - VLP Issuer Authorization code, Available Offline Spending Amount, Online Transaction PIN indicator, Other CVM
					//			 tag 9F74						tag 9F5D						tag 99						tag 55
					if( api_Tag9F74_Data_Length || 
					Available_Offline_Amount[0] || 
					Available_Offline_Amount_AP[0]	||
					(rspCode == ETP_OCP_CVM_OnlinePIN) || 
					(rspCode == ETP_OCP_CVM_ObtainSignature) ||
					firstfourlen	||
					api_tag9F39[0]	||
					api_tagDF0E[0]	||
					api_W3_DDAFail[0]||
					glv_tagDF8129.Length[0])
					{
						tstRsp[i++] = 0xD4; //D4
//handle length first
						if(firstfourlen)
							D4_len += 4;

						if(api_tag9F39[0])
							D4_len += 3+api_tag9F39[0];

						if(api_tagDF0E[0])
							D4_len += 3+api_tagDF0E[0];

						if(api_W3_DDAFail[0])
							D4_len += 3;

						if(rspCode == ETP_OCP_CVM_ObtainSignature)
							D4_len += 3;									// 3 means 0x55, 0x01, (0x00 or 0x01)

						if(api_Tag9F74_Data_Length)
							D4_len += (3 + api_Tag9F74_Data_Length);			// 3 means Tag 0x9F,0x74 and Data Length(1 byte)

						if(Available_Offline_Amount[0])
							D4_len += (3 + Available_Offline_Amount[0]);	// 3 means Tag 0x9F, 0x5D and Data Length(1 byte)

						if(Available_Offline_Amount_AP[0])
							D4_len += (3 + Available_Offline_Amount_AP[0]); // 3 means Tag 0x9F, 0x5D and Data Length(1 byte)
							
						if(rspCode == ETP_OCP_CVM_OnlinePIN)
							D4_len += 3;									// 3 means 0x99, 0x01, 0x00 ( the Last byte 0x01 means reader support pinpad, 0x00 means not)

						if(glv_tagDF8129.Length[0])	//Set D4 = DF8129 + Value of FF8106
						{
							UT_Get_TLVLengthOfV(glv_tagFF8106.Length, &lenOfFF8106);
							D4_len += (12 + lenOfFF8106);
						}

						tstRsp[i++] = D4_len;								//D4 total Length
//handle Data
						//20140107 first four bytes
						if(firstfourlen)
						{
							tstRsp[i++] = firstfourbytedata[0];
							tstRsp[i++] = firstfourbytedata[1];
							tstRsp[i++] = firstfourbytedata[2]; 														
							tstRsp[i++] = firstfourbytedata[3];
						}

						if(api_tag9F39[0])
						{
							tstRsp[i++] = 0x9F;
							tstRsp[i++] = 0x39;
							tstRsp[i++] = api_tag9F39[0];
							memcpy(&tstRsp[i],&api_tag9F39[1],api_tag9F39[0]);
							i+=api_tag9F39[0];	

							memset(api_tag9F39,0x00,11);
						}

						if(api_tagDF0E[0])
						{
							tstRsp[i++] = 0xDF;
							tstRsp[i++] = 0x0E;
							tstRsp[i++] = api_tagDF0E[0];
							memcpy(&tstRsp[i],&api_tagDF0E[1],api_tagDF0E[0]);
							i+=api_tagDF0E[0];	

							memset(api_tagDF0E,0x00,11);
						}
										
						//20130902 For signature, we always add tag 55.
						if(rspCode == ETP_OCP_CVM_ObtainSignature)
						{
							tstRsp[i++] = 0x55;
							tstRsp[i++] = 0x01;
							tstRsp[i++] = 0x00; 			//Reader ask for Signature	
						//	tstRsp[i++] = 0x01; 				//Reader already has Signature or  need not signature, if reader has Signature, it must be TLV format
						}

						if(api_Tag9F74_Data_Length) 						//9F74
						{
							tstRsp[i++] = 0x9F;
							tstRsp[i++] = 0x74;
							tstRsp[i++] = api_Tag9F74_Data_Length;
							memcpy(&tstRsp[i],api_Tag9F74_Data,api_Tag9F74_Data_Length);
							i+=api_Tag9F74_Data_Length; 		
						}

						if(Available_Offline_Amount[0]) 				//9F5D
						{
							tstRsp[i++] = 0x9F;
							tstRsp[i++] = 0x5D;
							tstRsp[i++] = Available_Offline_Amount[0];
							memcpy(&tstRsp[i],&Available_Offline_Amount[1],6);
							i+=Available_Offline_Amount[0]; 

							//reset

							memset(Available_Offline_Amount,0x00,sizeof(Available_Offline_Amount));
						}

						if(Available_Offline_Amount_AP[0])					//9F79
						{
							tstRsp[i++] = 0x9F;
							tstRsp[i++] = 0x79;
							tstRsp[i++] = Available_Offline_Amount_AP[0];
							memcpy(&tstRsp[i],&Available_Offline_Amount_AP[1],6);
							i+=Available_Offline_Amount_AP[0];	

							//reset

							memset(Available_Offline_Amount_AP,0x00,sizeof(Available_Offline_Amount_AP));
						}

						if(api_W3_DDAFail[0])
						{
							memcpy(&tstRsp[i],api_W3_DDAFail,3);
							i+=3;

							memset(api_W3_DDAFail,0x00,sizeof(api_W3_DDAFail));
						}

						if(rspCode == ETP_OCP_CVM_OnlinePIN)
						{
							tstRsp[i++] = 0x99;
							tstRsp[i++] = 0x01;
							tstRsp[i++] = 0x00; 			//Reader do not support pin pad
						}

						if(glv_tagDF8129.Length[0])
						{
							tstRsp[i++]=0xDF;	
							tstRsp[i++]=0x81;
							tstRsp[i++]=0x29;
							tstRsp[i++]=0x08;
							memcpy(&tstRsp[i], glv_tagDF8129.Value, 8);
							i+=8;

							memcpy(&tstRsp[i], glv_tagFF8106.Value, lenOfFF8106);
							i+=lenOfFF8106;
						}
					}

					tstRspLen = i;

					memcpy(E_Key,VISA_MSession_Key,16);

					//AS350 do not encrypt the transaction result
					if(VAP_VISA_P_ENCRYPTION)
					{
						api_pcd_vap_Encrypt_Data(tstRspLen-1, &tstRsp[1], &optLen, &optData[1], E_Key);
					}
					else
					{
						memcpy(&optData[1],&tstRsp[1],tstRspLen-1);
						optLen = tstRspLen-1;
					}
				}
			}
				
			optLen+=1;	//Add RC
	
			optMsgLength[0]=(optLen & 0xFF00) >> 8;
			optMsgLength[1]=optLen & 0x00FF;
	
			optData[0]=L3_Response_Code;
			
#ifdef _PLATFORM_AS350
			apk_tsc_switch(0);
#endif
		}
		else
		{
			optMsgLength[0]=0;
			optMsgLength[1]=1;
		
			if(api_InvalidateReader == TRUE)
			{
				optData[0]=VAP_RIF_RC_FAILURE;
			}
			else
			{
				optData[0]=VAP_RIF_RC_AUTH_NOT_PERFORMED;
			}		
		}
	}
	else
	{
		if(AmountErrFlag)
		{
			optMsgLength[0]=0;
			optMsgLength[1]=1;
			optData[0]=VAP_RIF_RC_INVALID_DATA;
		}
		else
		{
			optMsgLength[0]=0;
			optMsgLength[1]=1;
			optData[0]=VAP_RIF_RC_FAILURE;
		}
	}
	
	//if((L3_Response_Code == 0xFA)||(L3_Response_Code == 0xFF))
	if(((optData[0] == VAP_RIF_RC_FAILURE) || 
		(optData[0] == VAP_RIF_RC_OTHER_INTERFACE) 	|| 
		(optData[0] == VAP_RIF_RC_OTHER_AP_CARDS)	||
		(optData[0] == VAP_RIF_RC_MORE_CARDS)	||
		(optData[0] == VAP_RIF_RC_NO_CARD)	||
		(optData[0] == VAP_RIF_RC_JCB_REFUND_DECLINE)) && (!VAP_Reset_Command_Occured))
	{
		//20140115 V2, if the transaction is terminate, clean the AOSA display
		memset(VGL_Print_AOSA_Amount,0x00,sizeof(VGL_Print_AOSA_Amount));
		//20140115 V2, if the transaction is terminate, clean the AOSA display end		

#ifdef	_SCREEN_SIZE_240x320
		UT_ClearRow(1, 6, FONT0);		
#else
		UT_ClearScreen();
#endif
		//20140110 Show message
		if(optData[0] == VAP_RIF_RC_FAILURE)			//transaction terminate
		{
			api_pcd_vap_tra_ShowStatus(ShowMsgID32,&ShowMsgID32[2],ShowMsgRsp,&ShowMsgRsp[2]);
		}

		if(optData[0] == VAP_RIF_RC_OTHER_INTERFACE)	//Please insert card
		{
			api_pcd_vap_tra_ShowStatus(ShowMsgID6,&ShowMsgID6[2],ShowMsgRsp,&ShowMsgRsp[2]);
		}

		if(optData[0] == VAP_RIF_RC_OTHER_AP_CARDS)		//International Card Please insert
		{
			api_pcd_vap_tra_ShowStatus(ShowMsgID8,&ShowMsgID8[2],ShowMsgRsp,&ShowMsgRsp[2]);
		}

		if(optData[0] == VAP_RIF_RC_MORE_CARDS)		//Please Select 1 Card
		{
			api_pcd_vap_tra_ShowStatus(ShowMsgID7,&ShowMsgID7[2],ShowMsgRsp,&ShowMsgRsp[2]);
		}

		if(optData[0] == VAP_RIF_RC_NO_CARD)		//Please Try Again
		{
			api_pcd_vap_tra_ShowStatus(ShowMsgID9,&ShowMsgID9[2],ShowMsgRsp,&ShowMsgRsp[2]);
		}

		if(optData[0] == VAP_RIF_RC_JCB_REFUND_DECLINE)	//Transaction Complete
		{
			ETP_UI_Request(ETP_OCP_UIM_CardReadOKRemoveCard, ETP_OCP_UIS_CardReadSuccessfully);
		}

		if (optData[0] != VAP_RIF_RC_JCB_REFUND_DECLINE)
		{
			UT_WaitTime(VAP_VISA_P_DISPLAY_E_MSG);
		}
		//20140110 Show message end
		
		main_txn_complete = TRUE;
		main_card_remove = TRUE;
		main_clean_screen = TRUE;
		
#ifdef _SCREEN_SIZE_128x64
		api_sys_backlight(0,0x000001F4);
#endif
	}

	//20131115 reset amount
	api_msg_amount_len = 0;
	memset(api_msg_amount,0x00,sizeof(api_msg_amount));

	api_pcd_vap_flgJCBRefResponse=FALSE;
}


void api_pcd_vap_tra_Reset(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;

	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 0)
	{
		iptData=iptData;	

		//20140115 V3, if any Reset Command occured, Reset this flag
		api_Issuer_Update_Success_Flag_For_AOSA = FALSE;
		//20140115 V3, if any Reset Command occured, Reset this flag end

		//20140115 V2, if any Reset Command occured, clean the AOSA display
		memset(VGL_Print_AOSA_Amount,0x00,sizeof(VGL_Print_AOSA_Amount));
		//20140115 V2, if any Reset Command occured, clean the AOSA display end		

		//20131128 Switch from either administrative mode or debug/optimization mode to normal transaction mode
		api_pcd_vap_flgNormalMode = TRUE;
		api_pcd_vap_flgDbgAndOptMode = FALSE;
		api_pcd_vap_flgAdmMode = FALSE;

#ifdef _PLATFORM_AS350_LITE
		UT_Set_LED(0);
#endif

		//20130905 Field Off
		ECL_LV1_FIELD_OFF();	
		
		//Clear Reader's Buffer
		optData[0]=VAP_RIF_RC_SUCCESS;
	}
	else
	{
		optData[0]=VAP_RIF_RC_FAILURE;
	}
	api_pcd_vap_Set_MessageLength(0x0001, optMsgLength);
}

void api_pcd_vap_tra_ShowStatus(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0, i = 0, j = 0;	//20140116 V1, use "j" for AOSA display
	UCHAR	curMsgNumber = 0,msgNumber = 0, msgID = 0xFF, *msgPos=0 ;

	UCHAR Message1[64] = {0x00};
	UCHAR Message2[64] = {0x00};

	UCHAR Msg_Len1 = 0;
	UCHAR Msg_Len2 = 0;

	UCHAR F_Message1[32] = {0x00};
	UCHAR F_Message2[32] = {0x00};

	UCHAR F_Msg_Len1 = 0;
	UCHAR F_Msg_Len2 = 0;

	UCHAR S_Message1[32] = {0x00};
	UCHAR S_Message2[32] = {0x00};

	UCHAR S_Msg_Len1 = 0;
	UCHAR S_Msg_Len2 = 0;

	UCHAR disp_line = 0;
	
#ifdef _SCREEN_SIZE_128x64
	UCHAR base_line=0;
#else
	UCHAR base_line=1;
#endif

	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);

	if((iptData[0] == 0x00) || (iptData[0] == 0x01))
	{
		msgNumber = iptData[1];
		msgPos= &iptData[2];
		
		do {
			msgID = *msgPos++;

			for(i=0;i<VAP_MESSAGE_NUMBER;i++)
			{		
				if(glv_vap_Message[i].ID == msgID)
				{
					disp_line = 0;

					Msg_Len1 = Msg_Len2 = F_Msg_Len1 = F_Msg_Len2 = S_Msg_Len1 = S_Msg_Len2 = 0;
					//20131031				
#ifdef _SCREEN_SIZE_240x320
					UT_ClearRow(1, 6, FONT1);
#else
					UT_ClearScreen();
#endif

#ifdef _SCREEN_SIZE_128x64

					//20130911
					if(i == 0)
					{
						//20130911
						//break out directly and display message in main function 
						if(msgNumber == 1)
						{
							main_card_remove = TRUE;
							main_txn_complete = TRUE;
							main_clean_screen = TRUE;
							break;
						}							
					}
#endif

//LED & BUZ
					if(iptData[0] == 0x00)
					{
						UT_Set_LED(IID_LED_GREEN);
						UT_Buz_Option(TRUE);			
					}
					else
					{
						UT_LED_Switch(IID_LED_RED,2);
						UT_Buz_Option(FALSE);			
					}
//Show Message				
					UT_Handle_2Type_Message(glv_vap_Message[i].Message,(UINT)glv_vap_Message[i].Length,Message1,&Msg_Len1,Message2,&Msg_Len2);

					if(Msg_Len1)
						UT_Handle_2Line_Message(Message1,(UINT)Msg_Len1,F_Message1,&F_Msg_Len1,F_Message2,&F_Msg_Len2);
					if(Msg_Len2)
						UT_Handle_2Line_Message(Message2,(UINT)Msg_Len2,S_Message1,&S_Msg_Len1,S_Message2,&S_Msg_Len2);

					if(F_Msg_Len1 && F_Msg_Len2 && S_Msg_Len1 && S_Msg_Len2)
						disp_line = base_line + 0;
					else
						disp_line = base_line + 1;

					if((F_Msg_Len1 > Display_MAX_Num) && (F_Msg_Len2 > Display_MAX_Num)) 
						disp_line = base_line + 0;

					if(F_Msg_Len1)
						UT_Disp_Show_Status(F_Msg_Len1,F_Message1,&disp_line);
										
					if(F_Msg_Len2)
						UT_Disp_Show_Status(F_Msg_Len2,F_Message2,&disp_line);
										
					if(S_Msg_Len1)
						UT_Disp_Show_Status(S_Msg_Len1,S_Message1,&disp_line);
										
					if(S_Msg_Len2)
						UT_Disp_Show_Status(S_Msg_Len2,S_Message2,&disp_line);
					
//20140116 V1, We display AOSA only when transaction result is "Approval Or Decline"
					if((i==3) || (i==4))	
					{
//20140114 V4, PRINT AOSA Amount
						//if((glv_parDF05[0][0] == 0x01) || (glv_parDF05[2][0] == 0x01))	//20140116 V1, Changed
						//{															//20140116 V1, Changed
						if(!api_Issuer_Update_Success_Flag_For_AOSA)
						{
							if(glv_parDF05[0][0] == 0x01)
							{
								if(VGL_Print_AOSA_Amount[0])
								{
									//20140116 V1, change i to j, avoid the i effect
									for(j=1;j<14;j++)
									{
										if(VGL_Print_AOSA_Amount[j] != 0x30)		
											break;
								
										if((VGL_Print_AOSA_Amount[j] == 0x30) &&(VGL_Print_AOSA_Amount[j+1] == 0x2E))
											break;
									}
									UT_PutStr(6,(21-(7+14-j))/2,FONT0,7,(UCHAR *)"AOSA : ");	//20140115 V2 Change the length
									UT_PutStr(6,(21-(7+14-j))/2+7,FONT0,(14-j),&VGL_Print_AOSA_Amount[j]);//20140115 V2 Change the length
									memset(VGL_Print_AOSA_Amount,0x00,14);
								}
							}
						}
						else
						{
							api_Issuer_Update_Success_Flag_For_AOSA=FALSE;
						}
						//}//20140116 V1, Changed
//20140114 V4, PRINT AOSA Amount end			
					}	
					

//Display Delay					
					if((i==3) || (i==4) || (i==8))
					{
						if(!main_txn_complete)
						{
							main_clean_screen = TRUE;	
							main_txn_complete = TRUE;
						}
						
						UT_WaitTime(100);	
					}

					UT_WaitTime(100);

					if(main_txn_complete)
						main_clean_screen = TRUE;

				}
			}
	
			curMsgNumber++;
						
		} while (curMsgNumber < msgNumber); 		
	}
// UT_ClearRow(1, 14, FONT0);
//Test
//Parse iptData
iptData[0]=iptData[0];

	api_pcd_vap_Set_MessageLength(0x0001, optMsgLength);

	optData[0]=VAP_RIF_RC_SUCCESS;
}

void api_pcd_vap_tra_Issuer_Update(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData)
{
	UCHAR rspCode = 0;

	UCHAR OptResult[81] = {0};

	//UCHAR tag9F5B[100]={0x9F,0x5B};

	UINT iptLen = 0;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

	//20140117 V2, change to this concept
	if (api_pcd_vap_flgMutAuthenticate == TRUE)
	{
		//20140304, FIx Issuer Update "More Cards" & "No Card" problem
		L3_Response_Code = 0xFF;
		//20140304, Fix Issuer Update "More Cards" & "No Card" problem end
		
		//20140115 V2, Removed, we remove this flag to VGL_Kernel, if it realy to process issue update, we turn this flag on
		//20140114 V4, If Issue Update, Do not display AOSA
		//api_Issuer_Update_Success_Flag_For_AOSA = TRUE;
		//20140114 V4, If Issue Update, Do not display AOSA end
	
		iptLen = iptMsgLength[0]*256 + iptMsgLength[1];
	
		rspCode = api_pcd_vap_IAD(&iptLen,iptData,OptResult);

		if(rspCode == SUCCESS)
		{
			if(OptResult[0])	//if it has value, it means it has execute Issuer update command
			{
				optMsgLength[0]=0;
				optMsgLength[1]=1+1+OptResult[0]*5;		//RC Code (1 byte) + total number of result(1 byte) + Result
				optData[0]=VAP_RIF_RC_DATA;
				memcpy(&optData[1],OptResult,1+OptResult[0]*5);	//copy ISU Result to optdata, 1 for total number of result
			}
			else				//if only excute External Auth
			{
				optMsgLength[0]=0;
				optMsgLength[1]=1+1;		//RC Code (1 byte) + total number of result, because it only excute Ex_Auth, total number will be 00 (1 byte)
				optData[0]=VAP_RIF_RC_DATA;
				optData[1]=0x00;			//no issuer update excute, so number is 00
			}

			if(VAP_VISA_P_ENCRYPTION)
			{
				OriLen = optMsgLength[1]-1;		//minus RC_Code
				memcpy(OriData,&optData[1],OriLen);
				api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_MSession_Key);	//20140114 V4, Change to Msession Key
				optMsgLength[1]=1+EryptLen;		//add rc_Code
			}	
		}
		else
		{	
			//20140221 V1, Issuer Update "More Cards" & "No Card" problem			
			UT_ClearScreen();
		
			optMsgLength[0]=0;
			optMsgLength[1]=1;
			
			if(L3_Response_Code == VAP_RIF_RC_NO_CARD)
			{
				optData[0]=L3_Response_Code;	
	
				if(glv_vap_Message[8].Length > 16)
				{
					UT_PutStr(1,0,FONT1,16,glv_vap_Message[8].Message);
					UT_PutStr(2,0,FONT1,16,&glv_vap_Message[8].Message[16]);
				}
				else
				{
					UT_PutStr(1,0,FONT1,16,glv_vap_Message[8].Message);
				}
			}
			else if(L3_Response_Code == VAP_RIF_RC_MORE_CARDS)
			{
				optData[0]=L3_Response_Code;

				if(glv_vap_Message[6].Length > 16)
				{
					UT_PutStr(1,0,FONT1,16,glv_vap_Message[6].Message);
					UT_PutStr(2,0,FONT1,16,&glv_vap_Message[6].Message[16]);
				}
				else
				{
					UT_PutStr(1,0,FONT1,16,glv_vap_Message[6].Message);
				}				
			}
			else
			{
				optData[0]=VAP_RIF_RC_FAILURE;	

				if(glv_vap_Message[31].Length > 16)
				{
					UT_PutStr(1,0,FONT1,16,glv_vap_Message[31].Message);
					UT_PutStr(2,0,FONT1,16,&glv_vap_Message[31].Message[16]);
				}
				else
				{
					UT_PutStr(1,0,FONT1,16,glv_vap_Message[31].Message);
				}
			}

			UT_LED_Switch(IID_LED_RED,2);
			UT_Buz_Option(FALSE);

			UT_WaitTime(VAP_VISA_P_DISPLAY_E_MSG);

			main_txn_complete = TRUE;
			main_card_remove = TRUE;
			main_clean_screen = TRUE;
			//20140221 V1, Issuer Update "More Cards" & "No Card" problem end

			/*
			optMsgLength[0]=0;
			optMsgLength[1]=1;
			optData[0]=VAP_RIF_RC_FAILURE;	
			*/
		}
	}
	else
	{
		optMsgLength[0]=0;
		optMsgLength[1]=1;
		optData[0]=VAP_RIF_RC_AUTH_NOT_PERFORMED;
	}
	//20140117 V2, change to this concept end

	//20140117 V2, Removed
	/*
	if (api_pcd_vap_flgMutAuthenticate == TRUE)
	{
		//20140115 V2, Removed, we remove this flag to VGL_Kernel, if it realy to process issue update, we turn this flag on
		//20140114 V4, If Issue Update, Do not display AOSA
		//api_Issuer_Update_Success_Flag_For_AOSA = TRUE;
		//20140114 V4, If Issue Update, Do not display AOSA end
	
		iptLen = iptMsgLength[0]*256 + iptMsgLength[1];
	
		rspCode = api_pcd_vap_IAD(&iptLen,iptData,OptResult);

		if((rspCode == SUCCESS) && (OptResult[0]))
		{
			optMsgLength[0]=0;
			optMsgLength[1]=1+OptResult[0]*5;		//RC Code (1 byte) + Result
			optData[0]=VAP_RIF_RC_DATA;
			memcpy(&optData[1],&OptResult[1],OptResult[0]*5);	//Data

			if(VAP_VISA_P_ENCRYPTION)
			{
				OriLen = optMsgLength[1]-1;		//minus RC_Code
				memcpy(OriData,&optData[1],OriLen);
				api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_MSession_Key);	//20140114 V4, Change to Msession Key
				optMsgLength[1]=1+EryptLen;		//add rc_Code
			}	
		}
		else
		{
			optMsgLength[0]=0;
			optMsgLength[1]=1;
			optData[0]=VAP_RIF_RC_FAILURE;	
		}

	}
	else
	{
		optMsgLength[0]=0;
		optMsgLength[1]=1;
		optData[0]=VAP_RIF_RC_AUTH_NOT_PERFORMED;
	}*/
	//20140117 V2, Removed end
}

void api_pcd_vap_adm_SwitchToAdminMode(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	iptMode=0xFF;
	UCHAR	msgManufacturer[8]={"SYMLINK "};
	UCHAR	msgFirmwareVersion[4]={"1.01"};
	UCHAR	msgRFU[4]={"    "};
	UINT	msgLength=0;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

	if(api_pcd_vap_flgMutAuthenticate == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (msgLength == 1)
		{
			// 20131104
			memcpy(msgFirmwareVersion,&Mifare_Firm_Ver[1],4);
			// 20131104 end
			
			iptMode=iptData[0];
			
			if (iptMode == 0)
			{
				api_pcd_vap_flgAdmMode=TRUE;

				api_pcd_vap_flgNormalMode=FALSE;		//20131126 add normal mode

				OriData[0]=VAP_RIF_RC_SUCCESS;
				
				memcpy(&OriData[1], msgManufacturer, 8);
				memcpy(&OriData[9], msgFirmwareVersion, 4);
				memcpy(&OriData[13],msgRFU, 4);

				OriLen = 0x0011;

				if(VAP_VISA_P_ENCRYPTION)
				{
					optData[0] = VAP_RIF_RC_SUCCESS;
					api_pcd_vap_Encrypt_Data(OriLen-1, &OriData[1], &EryptLen, &optData[1], VISA_ASession_Key);
					api_pcd_vap_Set_MessageLength(EryptLen+1, optMsgLength);	//20131220 set opt len
				}
				else
				{
					memcpy(optData,OriData,OriLen);
					api_pcd_vap_Set_MessageLength(OriLen, optMsgLength);	//20131220 set opt len
				}				
			}
			else if (iptMode == 1)
			{
				api_pcd_vap_flgAdmMode=FALSE;

				api_pcd_vap_flgNormalMode=TRUE;			//20131126 add normal mode

				OriData[0]=VAP_RIF_RC_SUCCESS;
				
				memcpy(&OriData[1], msgManufacturer, 8);
				memcpy(&OriData[9], msgFirmwareVersion, 4);
				memcpy(&OriData[13],msgRFU, 4);

				OriLen = 0x0011;

				if(VAP_VISA_P_ENCRYPTION)
				{
					optData[0] = VAP_RIF_RC_SUCCESS;
					api_pcd_vap_Encrypt_Data(OriLen-1, &OriData[1], &EryptLen, &optData[1], VISA_ASession_Key);
					api_pcd_vap_Set_MessageLength(EryptLen+1, optMsgLength);	//20131220 set opt len
				}
				else
				{
					memcpy(optData,OriData,OriLen);
					api_pcd_vap_Set_MessageLength(OriLen, optMsgLength);	//20131220 set opt len
				}				
			}
			else
			{
				optData[0]=VAP_RIF_RC_INVALID_COMMAND;
				api_pcd_vap_Set_MessageLength(0x0001, optMsgLength);	//20131220 set opt len				
			}
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
			api_pcd_vap_Set_MessageLength(0x0001, optMsgLength);	//20131220 set opt len				
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_FAILURE;
		api_pcd_vap_Set_MessageLength(0x0001, optMsgLength);	//20131220 set opt len				
	}
	
}

void api_pcd_vap_adm_GetCapability(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	numScheme=0;
	UCHAR	numCurrent=0;
	UCHAR	optSupFlag=0;
	UCHAR	rspCode=0;
	UCHAR	*ptrIptData=NULLPTR;
	UCHAR	*ptrOptData=NULLPTR;
	UINT	msgLength=0;
	UINT	i=0,j=0;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

	UCHAR	NotSupportFlg = FALSE;
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		j = api_pcd_vap_Get_MessageLength(iptMsgLength);

		numScheme=iptData[0];
	
		if((numScheme == 0xFF) && (j == 1))	//Requests the reader to list ALL payment schemes
		{
			ptrOptData = &optData[2];
			
			for(i = 0; i< VAP_SCHEME_NUMBER; i++)
			{
				//20140205, Cancel Scheme ID check
				//20140102, In VISA AP we only support "W2, W3, MSD2.0, JCB W2 and JCB W3"
				//if(	(glv_vap_Scheme[i].ID == VAP_Scheme_Wave2)||
				//	(glv_vap_Scheme[i].ID == VAP_Scheme_Wave3)||
				//	(glv_vap_Scheme[i].ID == VAP_Scheme_MSD20)||
				//	(glv_vap_Scheme[i].ID == VAP_Scheme_JCBWave2)||
				//	(glv_vap_Scheme[i].ID == VAP_Scheme_JCBWave3))
				//{
				ptrOptData[2*numCurrent]=glv_vap_Scheme[i].ID;
				ptrOptData[(2*numCurrent)+1]=glv_vap_Scheme[i].Support;				

				numCurrent+=1;
				
				msgLength+=2;				
				//}
			}
			
			msgLength+=2;	//RC(1) + Scheme Number(1)
			optData[0]=VAP_RIF_RC_SCHEME_SUPPORTED;
			optData[1]=numCurrent;
		}
		else
		{
			ptrIptData=&iptData[1];

			ptrOptData=&optData[2];

			do {
				//20140205, cancel Scheme ID check
				//20140102, In VISA AP we only support "W2, W3, MSD2.0, JCB W2 and JCB W3"
				//if(	(*ptrIptData == VAP_Scheme_Wave2)||
				//	(*ptrIptData == VAP_Scheme_Wave3)||
				//	(*ptrIptData == VAP_Scheme_MSD20)||
				//	(*ptrIptData == VAP_Scheme_JCBWave2)||
				//	(*ptrIptData == VAP_Scheme_JCBWave3))
				//{
				rspCode=api_pcd_vap_Get_Capability(ptrIptData[0], &optSupFlag);

				ptrOptData[0]=ptrIptData[0];
				
				if (rspCode == SUCCESS)
				{
					ptrOptData[1]=optSupFlag;
				}
				else
				{
					NotSupportFlg = TRUE;	//20140205 chenaged
					break;					//20140205 chenaged
				}

				ptrIptData++;
				numCurrent++;
				ptrOptData+=2;
				msgLength+=2;
				//}
				//else
				//{
				//	NotSupportFlg = TRUE;
				//	break;
				//}				
			} while (numCurrent < numScheme);

			if(!NotSupportFlg)
			{
				msgLength+=2;	//RC(1) + Scheme Number(1)
				optData[0]=VAP_RIF_RC_SCHEME_SUPPORTED;
				optData[1]=numCurrent;
			}
			else
			{
				msgLength = 1;
				optData[0]=VAP_RIF_RC_INVALID_SCHEME;
			}
		}

		memcpy(OriData,optData,msgLength);
		OriLen = msgLength;

		//20140102
		if((VAP_VISA_P_ENCRYPTION) && (!NotSupportFlg))
		{
			api_pcd_vap_Encrypt_Data(OriLen-1, &OriData[1], &EryptLen, &optData[1], VISA_ASession_Key);
			api_pcd_vap_Set_MessageLength(EryptLen+1, optMsgLength);	//20131220 set opt len	
		}
		else
		{
			api_pcd_vap_Set_MessageLength(OriLen, optMsgLength);	
		}
		
	}
	else
	{
		msgLength=1;
		api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}	 
}


void api_pcd_vap_adm_SetCapability(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	numScheme=0;
	UCHAR	numCurrent=0;
	UCHAR	flgError=0;
	UCHAR	rspCode=0;
	UCHAR	*ptrIptData=NULLPTR;
	UINT	msgLength=0;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

#ifndef _PLATFORM_AS350
	UCHAR	ToFlashSchemeData[VAP_SCHEME_NUMBER*2] = {0};
	UCHAR	*ToFlashSchemeDataPtr = NULLPTR;
	UCHAR	index = 0;
#endif
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);

		numScheme=iptData[0];
		ptrIptData=&iptData[1];

		do {
			//20140205, cancel Scheme ID check
			//if(	(*ptrIptData == VAP_Scheme_Wave2)||
			//	(*ptrIptData == VAP_Scheme_Wave3)||
			//	(*ptrIptData == VAP_Scheme_MSD20)||
			//	(*ptrIptData == VAP_Scheme_JCBWave2)||
			//	(*ptrIptData == VAP_Scheme_JCBWave3))
			//{
			rspCode=api_pcd_vap_Set_Capability(ptrIptData[0], ptrIptData[1]);
			if (rspCode == FAIL)
			{
				flgError=TRUE;
				break;
			}

			ptrIptData+=2;
			numCurrent++;
			//}
			//else
			//{
			//	flgError=TRUE;
			//	break;
			//}			
		} while (numCurrent < numScheme);

		if (flgError == FALSE)
		{
			msgLength++;
			optData[0]=VAP_RIF_RC_SUCCESS;

			if(VAP_VISA_P_ENCRYPTION)
			{
				memcpy(OriData,iptData,msgLength);
				OriLen = msgLength-1;	//minus 1 byte for RC_Code
				
				api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
				msgLength = EryptLen+1;	//add 1 byte for RC_Code
			}
			else
				memcpy(&optData[1], &iptData[0], msgLength);

#ifndef _PLATFORM_AS350

#ifdef _PLATFORM_AS210
			rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
			rspCode = apiOK;
#endif
			if(rspCode == apiOK)
			{
				memset(ToFlashSchemeData,0xFF,VAP_SCHEME_NUMBER*2);

				ToFlashSchemeDataPtr = ToFlashSchemeData;

				for (index=0; index < VAP_SCHEME_NUMBER; index++)
				{
					*ToFlashSchemeDataPtr = glv_vap_Scheme[index].ID ;
					ToFlashSchemeDataPtr++;
					
					*ToFlashSchemeDataPtr = glv_vap_Scheme[index].Support;
					ToFlashSchemeDataPtr++;
				}
				
				api_fls_write(FLSID_PARA,FLS_ADDRESS_Scheme,VAP_SCHEME_NUMBER*2,ToFlashSchemeData);
			}
#endif
		}
		else
		{
			msgLength=1;
			optData[0]=VAP_RIF_RC_INVALID_SCHEME;
		}
	}
	else
	{
		msgLength=1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_GetDateAndTime(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rtcBuffer[14]={0};
	UINT	msgLength=0;
	UCHAR 	rspCode = 0xFF;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

	UINT	OptLen = 0x0007;

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (msgLength == 0)
		{
			iptData=iptData;	//Remove Warning
			
			rtcBuffer[0]='2';
			rtcBuffer[1]='0';
			rspCode = api_rtc_getdatetime(0, &rtcBuffer[2]);

			if(rspCode == apiOK)
			{					
				UT_Compress(&optData[1], rtcBuffer, 7);

				optData[0]=VAP_RIF_RC_SUCCESS;
				
				if(VAP_VISA_P_ENCRYPTION)
				{
					memcpy(OriData,&optData[1],OptLen);
					OriLen = OptLen;
					api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
					OptLen = EryptLen;
				}				
			}
			else
				optData[0]=VAP_RIF_RC_FAILURE;
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	api_pcd_vap_Set_MessageLength(OptLen+1, optMsgLength);
}


void api_pcd_vap_adm_SetDateAndTime(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	ascDatTime[14]={0};
	UCHAR	rtcBuffer[13]={0};
	UCHAR	rspCode=0;
	UINT	msgLength=0;

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (msgLength == 7)
		{
			UT_Split(ascDatTime, iptData, 7);

			rtcBuffer[0]=12;
			memcpy(&rtcBuffer[1], &ascDatTime[2], 12);

			rspCode=UT_CheckDateTime(rtcBuffer);
			if (rspCode == TRUE)
			{
				api_rtc_setdatetime(0, &ascDatTime[2]);
				
				optData[0]=VAP_RIF_RC_SUCCESS;
			}
			else
			{
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			}
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
	
	api_pcd_vap_Set_MessageLength(0x0001, optMsgLength);
}


void api_pcd_vap_adm_GetParameters(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=0;
	UINT	datLen=0;
	UINT	msgLength=0;
	UINT	msgIndex = 0;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (msgLength == 2)
		{
			msgIndex = iptData[0]*256 + iptData[1];

			if((msgIndex > 0) && (msgIndex <= VAP_PARAMETER_NUMBER))
			{
				rspCode=api_pcd_vap_Get_Parameter(iptData, &optData[3], &optData[5]);
				if (rspCode == SUCCESS)
				{
					optData[0]=VAP_RIF_RC_SUCCESS;
					memcpy(&optData[1], iptData, 2);
					datLen=2+2+(optData[3]*256+optData[4]);	//Index(2)+Len(2)+Data(x)
				
					//20140102
					if(VAP_VISA_P_ENCRYPTION)
					{
						OriLen = datLen;
						memcpy(OriData,&optData[1],OriLen);
						api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
						datLen = EryptLen;
					}
				}
				else
				{
					optData[0]=VAP_RIF_RC_FAILURE;
				}
			}
			else
			{
				optData[0]=VAP_RIF_RC_INVALID_PARA;
			}			
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_DATA;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	api_pcd_vap_Set_MessageLength((1+datLen), optMsgLength);
}


void api_pcd_vap_adm_GetEMVTagsValues(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	tagNumber=0;
	UCHAR	curNumber=0;
	UCHAR	lenOfT=0;
	UCHAR	rspCode=0;
	UCHAR	tmpT[2]={0};
	UCHAR	tmpL=0;
	UCHAR	tmpV[256]={0};
	UCHAR	flgError=0;
	UCHAR	*ptrInput=NULLPTR;
	UCHAR	*ptrOutput=NULLPTR;
	UINT	msgLength=0;
	UCHAR	flgDATA = FALSE;
	UCHAR	flgCommand = FALSE;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

	UINT 	Input_Len = 0;

	UCHAR	DefEMVTags[65] = {	0x9F,0x35,0xDF,0x04,0x9F,0x1A,0x5F,0x2A,0x9C,0xDF,0x00,0xDF,0x01,0xDF,0x02,0xDF,
								0x03,0xDF,0x05,0x9F,0x66,0xDF,0x06,0xDF,0x61,0xDF,0x62,0xDF,0x63,0xDF,0x64,0xDF,
								0x65,0xDF,0x66,0xDF,0x67,0x9F,0x1B,0xDF,0x68,0xDF,0x69,0xDF,0x6A,0xDF,0x6B,0xDF,
								0x6C,0xDF,0x6D,0xDF,0x6E,0xDF,0x6F,0xDF,0x70,0xDF,0x71,0xDF,0x72,0xDF,0x73,0xDF,
								0x77};

#ifndef _PLATFORM_AS350
	UCHAR	InFlashEMVData_1[256] = {0};

	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	curSize=0;
#endif
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{

#ifndef _PLATFORM_AS350

#ifdef _PLATFORM_AS210
		rspCode=api_fls_memtype(FLSID_PARA, FLSType_SRAM);
#else
		rspCode=apiOK;
#endif
		if (rspCode == apiOK)
		{
			//20140105 test : read data from flash and write to SDRAM
			rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_EMV_Tag,256,InFlashEMVData_1);
			if(rspCode == apiOK)
			{
				tagNumber = 33;
				ptrInput = InFlashEMVData_1;
					
				do {
					rspCode=UT_Get_TLVLength(ptrInput, &lenOfT, &lenOfL, &lenOfV);
					if (rspCode == SUCCESS)
					{				
						rspCode = api_pcd_vap_Set_EMVTag(ptrInput);
						if(rspCode == SUCCESS)
						{
							//Point to Next Tag
							ptrInput+=(lenOfT+lenOfL+lenOfV);
							curNumber++;
							curSize+=(lenOfT+lenOfL+lenOfV);
						}
					}

					if ((ptrInput[0] == 0x00) || (ptrInput[0] == 0xFF))
					{
						ptrInput++;
						curSize++;
					}

					if (curSize >= 256) break;
				} while (curNumber < tagNumber);

				//reset parameter
				tagNumber = 0;
				lenOfT = 0;
				*ptrInput = 0;
				curNumber = 0;
				curSize=0;
			}
			//20140105 test end
		}
#endif
	
		Input_Len = api_pcd_vap_Get_MessageLength(iptMsgLength);

		if((Input_Len == 1) && (iptData[0] == 0xFF))
		{
			tagNumber = 33;
			ptrInput = DefEMVTags;
			ptrOutput = &optData[2];

			do {
				rspCode=UT_Get_TLVLengthOfT(ptrInput, &lenOfT);
				if (rspCode == SUCCESS)
				{
					memcpy(tmpT, ptrInput, lenOfT);
					
					rspCode = api_pcd_vap_Get_EMVTag(tmpT, &tmpL, tmpV);
					if(rspCode == SUCCESS)
					{
						memcpy(ptrOutput, tmpT, lenOfT);			//Patch T
						ptrOutput[lenOfT]=tmpL;						//Patch L
						memcpy(&ptrOutput[lenOfT+1], tmpV, tmpL);	//Patch V

						//Point to Next Tag
						ptrInput+=lenOfT;
						ptrOutput+=(lenOfT+1+tmpL);
						msgLength+=(lenOfT+1+tmpL);
						curNumber++;
					}
					else
					{
						flgError=TRUE;

						flgDATA=TRUE;

						break;
					}
				}
				else
				{
					flgError=TRUE;

					flgCommand = TRUE;
					
					break;
				}
			} while (curNumber < tagNumber);
		}
		else
		{
			tagNumber=iptData[0];
			ptrInput=&iptData[1];
			ptrOutput=&optData[2];

			do {
				rspCode=UT_Get_TLVLengthOfT(ptrInput, &lenOfT);
				if (rspCode == SUCCESS)
				{
					memcpy(tmpT, ptrInput, lenOfT);
					
					rspCode = api_pcd_vap_Get_EMVTag(tmpT, &tmpL, tmpV);
					if(rspCode == SUCCESS)
					{
						memcpy(ptrOutput, tmpT, lenOfT);			//Patch T
						ptrOutput[lenOfT]=tmpL;						//Patch L
						memcpy(&ptrOutput[lenOfT+1], tmpV, tmpL);	//Patch V

						//Point to Next Tag
						ptrInput+=lenOfT;
						ptrOutput+=(lenOfT+1+tmpL);
						msgLength+=(lenOfT+1+tmpL);
						curNumber++;
					}
					else
					{
						flgError=TRUE;

						flgDATA=TRUE;

						break;
					}
				}
				else
				{
					flgError=TRUE;

					flgCommand = TRUE;
					
					break;
				}
			} while (curNumber < tagNumber);
		}

		if (flgError == TRUE)
		{
			if(flgDATA)
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			else if(flgCommand)
				optData[0]=VAP_RIF_RC_INVALID_DATA;		//20140103 change RC_Code to VAP_RIF_RC_INVALID_DATA
			else
				optData[0]=VAP_RIF_RC_FAILURE;
		}
		else
		{
			if(VAP_VISA_P_ENCRYPTION)
			{
				optData[0]=VAP_RIF_RC_SUCCESS;
				optData[1]=curNumber;

				memcpy(OriData,&optData[1],msgLength+1);
				OriLen = msgLength+1;

				api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
				msgLength = EryptLen;
			}
			else
			{
				optData[0]=VAP_RIF_RC_SUCCESS;
				optData[1]=curNumber;
			}
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	if(VAP_VISA_P_ENCRYPTION)
		api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
	else
		api_pcd_vap_Set_MessageLength((msgLength+2), optMsgLength);
}

void api_pcd_vap_adm_SetEMVTagsValues(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	tagNumber=0;
	UCHAR	curNumber=0;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	rspCode=0;
	UCHAR	flgError=0;
	UCHAR	*ptrInput=NULLPTR;
	UINT	msgLength=0;
	UCHAR	flgNOTag = FALSE;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

#ifndef _PLATFORM_AS350
	UCHAR	DefEMVTags[65] = {	0x9F,0x35,0xDF,0x04,0x9F,0x1A,0x5F,0x2A,0x9C,0xDF,0x00,0xDF,0x01,0xDF,0x02,0xDF,
								0x03,0xDF,0x05,0x9F,0x66,0xDF,0x06,0xDF,0x61,0xDF,0x62,0xDF,0x63,0xDF,0x64,0xDF,
								0x65,0xDF,0x66,0xDF,0x67,0x9F,0x1B,0xDF,0x68,0xDF,0x69,0xDF,0x6A,0xDF,0x6B,0xDF,
								0x6C,0xDF,0x6D,0xDF,0x6E,0xDF,0x6F,0xDF,0x70,0xDF,0x71,0xDF,0x72,0xDF,0x73,0xDF,
								0x77};
	UCHAR	tmpT[2]={0};
	UCHAR	tmpL=0;
	UCHAR	tmpV[256]={0};
	UCHAR	*ptrOutput=NULLPTR;

	UCHAR	InFlashEMVData[256] = {0};
	UCHAR	ToFlashEMVData[256] = {0};
#endif

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		tagNumber=iptData[0];
		ptrInput=&iptData[1];

		do {
			rspCode=UT_Get_TLVLength(ptrInput, &lenOfT, &lenOfL, &lenOfV);
			if (rspCode == SUCCESS)
			{				
				rspCode = api_pcd_vap_Set_EMVTag(ptrInput);

				if(rspCode == SUCCESS)
				{
					//Point to Next Tag
					ptrInput+=(lenOfT+lenOfL+lenOfV);
					curNumber++;
				}
				else
				{
					flgError=TRUE;
					flgNOTag = TRUE;					
					break;
				}
			}
			else
			{
				flgError=TRUE;				
				break;
			}
		} while (curNumber < tagNumber);

		if (flgError == TRUE)
		{
			if(flgNOTag)
				optData[0]=VAP_RIF_RC_INVALID_DATA;	//20140103 change the rc code to VAP_RIF_RC_INVALID_DATA
			else
				optData[0]=VAP_RIF_RC_INVALID_DATA;
		}
		else
		{
			if(VAP_VISA_P_ENCRYPTION)
			{
				msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
				
				OriLen = msgLength;
				memcpy(OriData,iptData,msgLength);
				
				api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
				msgLength = EryptLen;
				optData[0]=VAP_RIF_RC_SUCCESS;
			}
			else
			{
				msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
				memcpy(&optData[1], iptData, msgLength);
				optData[0]=VAP_RIF_RC_SUCCESS;
			}

#ifndef _PLATFORM_AS350

#ifdef _PLATFORM_AS210
			rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
			rspCode = apiOK;
#endif
			if(rspCode == apiOK)
			{
				rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_EMV_Tag,256,InFlashEMVData);
				if(rspCode == apiOK)
				{
					memset(ToFlashEMVData,0xFF,256);
					
					ptrOutput = ToFlashEMVData;
					curNumber = 0;
					tagNumber = 33;

					ptrInput = DefEMVTags;
					
					do {					
						rspCode=UT_Get_TLVLengthOfT(ptrInput, &lenOfT);
						if (rspCode == SUCCESS)
						{
							memcpy(tmpT, ptrInput, lenOfT);
							
							rspCode = api_pcd_vap_Get_EMVTag(tmpT, &tmpL, tmpV);
							if(rspCode == SUCCESS)
							{
								memcpy(ptrOutput, tmpT, lenOfT);			//Patch T
								ptrOutput[lenOfT]=tmpL;						//Patch L
								memcpy(&ptrOutput[lenOfT+1], tmpV, tmpL);	//Patch V

								//Point to Next Tag
								ptrInput+=lenOfT;
								ptrOutput+=(lenOfT+1+tmpL);
								curNumber++;
							}
						}
					} while (curNumber < tagNumber);
				
					if(memcmp(ToFlashEMVData,InFlashEMVData,256))
					{
						rspCode = api_fls_write(FLSID_PARA,FLS_ADDRESS_EMV_Tag,256,ToFlashEMVData);
						if(rspCode != apiOK)
						{
							msgLength = 0;
							optData[0]=VAP_RIF_RC_FAILURE;
						}	
					}				
				}
				else
				{
					msgLength = 0;
					optData[0]=VAP_RIF_RC_FAILURE;
				}	
			}
			else
			{
				msgLength = 0;
				optData[0]=VAP_RIF_RC_FAILURE;
			}					
#endif			
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
}


void api_pcd_vap_adm_GetDisplayMessages(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=0;
	UINT	datLen=0;
	UCHAR 	idxNum=0;
	UCHAR	optMsg[1024] = {0};
	UINT	index = 0;
	UINT	Input_Msg_Len = 0;
	
	//20140105
	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;
	
	UCHAR	MsgNum = 0;

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		Input_Msg_Len = api_pcd_vap_Get_MessageLength(iptMsgLength);

		if(Input_Msg_Len == 1)
		{
			if(iptData[0] == 0xFF)
			{							
				for (idxNum=0,index=0; idxNum < VAP_MESSAGE_NUMBER; idxNum++)
				{	
					//20140105 In Interace debug, we only present message ID 1~20
					if((glv_vap_Message[idxNum].ID > 0) && (glv_vap_Message[idxNum].ID < 21))
					{
						memcpy(&optMsg[index],&glv_vap_Message[idxNum].ID,1);
						index+=1;
						memcpy(&optMsg[index],&glv_vap_Message[idxNum].Length,1);
							index+=1;
						if(glv_vap_Message[idxNum].Length != 0x00)
						{
							memcpy(&optMsg[index],glv_vap_Message[idxNum].Message,glv_vap_Message[idxNum].Length);
							index+=glv_vap_Message[idxNum].Length;
						}
						MsgNum++;
					}					
				}
				memcpy(&optData[2],optMsg, index);
				
				optData[0]=VAP_RIF_RC_SUCCESS;
				optData[1] = MsgNum;

				datLen=index + 1;	// 1 for X (1) ï¿½V No. of messages

				//20140105 encrypt output data
				if(VAP_VISA_P_ENCRYPTION)
				{
					OriLen = datLen;
					memcpy(OriData,&optData[1],OriLen);
					api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
					datLen = EryptLen;
				}
			}
			else
			{	
				if(iptData[0] <= VAP_MESSAGE_NUMBER)
				{	
					//20140105 In Interace debug, we only present message ID 1~20
					if((iptData[0] > 0) && (iptData[0] < 21))
					{						
						rspCode=api_pcd_vap_Get_DisplayMessage(iptData[0], &optData[3], &optData[4]);
						if (rspCode == SUCCESS)
						{
							optData[0]=VAP_RIF_RC_SUCCESS;
							optData[1]=1;
							optData[2]=iptData[0];
						
							datLen=1+1+1+optData[3];	//Number(1)+ID(1)+Length(1)+Message(x)
						}
						else
						{
							optData[0]=VAP_RIF_RC_FAILURE;
						}
					}
					else if((iptData[0] > 30) && (iptData[0] < 36))
					{
						rspCode=api_pcd_vap_Get_DisplayMessage(iptData[0], &optData[3], &optData[4]);
						if (rspCode == SUCCESS)
						{
							optData[0]=VAP_RIF_RC_SUCCESS;
							optData[1]=1;
							optData[2]=iptData[0];
						
							datLen=1+1+1+optData[3];	//Number(1)+ID(1)+Length(1)+Message(x)
						}
						else
						{
							optData[0]=VAP_RIF_RC_FAILURE;
						}
					}
					else
						optData[0]=VAP_RIF_RC_INVALID_DATA;

					//20140105 encrypt output data
					if(VAP_VISA_P_ENCRYPTION)
					{
						OriLen = datLen;
						memcpy(OriData,&optData[1],OriLen);
						api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
						datLen = EryptLen;
					}
				}
				else
				{
					//Input Msg Index error
					optData[0]=VAP_RIF_RC_INVALID_DATA;
				}
			}		
		}
		else
		{
			//Input Msg Len error
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}			
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	api_pcd_vap_Set_MessageLength((datLen+1), optMsgLength);	// add 1 byte for RC_Code
}


void api_pcd_vap_adm_SetDisplayMessages(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=0;
	UCHAR	msgLen=0;
	UCHAR	msgNumber=0;
	UCHAR	curMsgNumber=0;
	UCHAR	flgError=0;
	UCHAR	*ptrData=NULLPTR;

	UINT	IptLen = 0;
	UINT	OptLen = 0;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

#ifndef _PLATFORM_AS350
	UCHAR	ToFlashMessageData[1320] = {0};
	UINT	ToFlashMessageLen = 0;
	UCHAR	*ToFlashMessagePtr = NULLPTR;
	UCHAR	i = 0;
	UCHAR 	tmpLen = 0;
#endif
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		IptLen = api_pcd_vap_Get_MessageLength(iptMsgLength);

		msgNumber=iptData[0];
		ptrData=&iptData[1];

		if(msgNumber <= VAP_MESSAGE_NUMBER)
		{
			do {
				if((*ptrData > 0)&&(*ptrData < 21))	//20140103 if message ID is 1~20, return VAP_RIF_RC_INVALID_DATA
				{
					rspCode=api_pcd_vap_Set_DisplayMessage(ptrData[0], ptrData[1], &ptrData[2]);
					if (rspCode == SUCCESS)
					{
						//20140107 if set message ID 1 successfully, clean screen
						if(ptrData[0] == 1)
							main_clean_screen = TRUE;
					
						msgLen=ptrData[1];
						ptrData+=(1+1+msgLen);	//ID(1)+Length(1)
						curMsgNumber++;
					}
					else
					{
						flgError=TRUE;
						break;
					}
				}
				else if((*ptrData > 30)&&(*ptrData < 36))	//20140112 private message, ID 31~35
				{
					rspCode=api_pcd_vap_Set_DisplayMessage(ptrData[0], ptrData[1], &ptrData[2]);
					if (rspCode == SUCCESS)
					{					
						msgLen=ptrData[1];
						ptrData+=(1+1+msgLen);	//ID(1)+Length(1)
						curMsgNumber++;
					}
					else
					{
						flgError=TRUE;
						break;
					}					
				}
				else
				{
					flgError=TRUE;
					break;
				}
			} while (curMsgNumber < msgNumber);		
		}
		else
		{
			flgError=TRUE;
		}

		if (flgError == TRUE)
		{
			optData[0]=VAP_RIF_RC_INVALID_DATA;
		}
		else
		{
			//20140105 chang the output data
			//OptLen = IptLen+1;	//Add "Number of Messages"
			OptLen = IptLen;
			//20140103 add response data
			//optData[1]=curMsgNumber;
			memcpy(&optData[1],iptData,IptLen);

			if(VAP_VISA_P_ENCRYPTION)
			{
				OriLen = OptLen;
				memcpy(OriData,&optData[1],OptLen);
				api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
				OptLen = EryptLen;
			}

			optData[0]=VAP_RIF_RC_SUCCESS;

#ifndef _PLATFORM_AS350
			//20140106 write to flash, message id 1~20
			ToFlashMessagePtr = ToFlashMessageData;
			for(i=0;i<20;i++)
			{
				*ToFlashMessagePtr = glv_vap_Message[i].ID;
				ToFlashMessagePtr++;

				*ToFlashMessagePtr = glv_vap_Message[i].Length;
				ToFlashMessagePtr++;

				tmpLen = glv_vap_Message[i].Length;
				memcpy(ToFlashMessagePtr,glv_vap_Message[i].Message,tmpLen);
				ToFlashMessagePtr+=tmpLen;

				ToFlashMessageLen+= 1+1+tmpLen;
			}

#ifdef _PLATFORM_AS210
			rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
			rspCode = apiOK;
#endif
			if(rspCode == apiOK)
				api_fls_write(FLSID_PARA,FLS_ADDRESS_Message,ToFlashMessageLen,ToFlashMessageData);	

			//20140112 write to flash,	Private Message, message id 31~35
			memset(ToFlashMessageData,0xFF,1320);
			ToFlashMessageLen = 0;	//20140408 reset Len
			ToFlashMessagePtr = ToFlashMessageData;
			for(i=30;i<35;i++)
			{
				*ToFlashMessagePtr = glv_vap_Message[i].ID;
				ToFlashMessagePtr++;

				*ToFlashMessagePtr = glv_vap_Message[i].Length;
				ToFlashMessagePtr++;

				tmpLen = glv_vap_Message[i].Length;
				memcpy(ToFlashMessagePtr,glv_vap_Message[i].Message,tmpLen);
				ToFlashMessagePtr+=tmpLen;

				ToFlashMessageLen+= 1+1+tmpLen;
			}

#ifdef _PLATFORM_AS210
			rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
			rspCode = apiOK;
#endif
			if(rspCode == apiOK)
				api_fls_write(FLSID_PARA,FLS_ADDRESS_Private_Msg,ToFlashMessageLen,ToFlashMessageData);	
#endif			
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
	
	api_pcd_vap_Set_MessageLength(OptLen+1, optMsgLength);
}

void api_pcd_vap_adm_GetVisaPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	//UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR 	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR 	AID[5] = {0xA0,0x00,0x00,0x00,0x03};
	UINT	i = 0, j = 0;
	UCHAR   Key_Data[300] = {0};
	UINT    Key_Data_Len = 0,Total_Key_Data_Len = 0;
	UCHAR   Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UINT    Num_Keys = 0;		//Number of VSDC CA Public Key
	UCHAR	flgReadFlash = FALSE;

	UINT 	Input_Msg_Len = 0;

	UCHAR	OriData[2048] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;
	
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];	

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if(Input_Msg_Len == 1)
		{
#ifdef _PLATFORM_AS210
			api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#endif
			if(*iptData == 0xFF)
			{
				for(j=0;j<CAPK_NUMBER;j++)
				{								
					rspCode = api_fls_read(FLSID_CAPK,j*CAPK_LEN,CAPK_LEN,(UCHAR *)&tmpCAPK);
					
					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//Reset
						i = 0;
						Key_Data_Len = 0;
						Hash_Input_Len = 0;
						
						//Accumulate the number of VSDC CA Public Key 
						Num_Keys++;
						
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						memmove(&Key_Data[3],Key_Data,i);
						Key_Data[0] = tmpCAPK.Index;
						Key_Data[1] = (i & 0xFF00) >> 8;
						Key_Data[2] = (i & 0x00FF);
						Key_Data_Len = i + 3;

						//Final
						memcpy(&optData[2+Total_Key_Data_Len],Key_Data,Key_Data_Len);
						Total_Key_Data_Len+=Key_Data_Len;

						//20140102
						if((Total_Key_Data_Len + 2) > VAP_VISA_P_MAX_BUF_SIZE)
						{
							Total_Key_Data_Len -= Key_Data_Len;
							Num_Keys -= 1;
							break;
						}
					}
					else
					{
						if(rspCode != apiOK)
						{
							flgReadFlash = TRUE;
							break;
						}
					}
				}

				if(flgReadFlash)						//if read flash error, we delete all data and set the RC code to Failure
				{
					optData[0]=VAP_RIF_RC_FAILURE;
					msgLength = 0;
				}
				else
				{
					//20140102
					optData[0]=VAP_RIF_RC_SUCCESS;
				
					if(VAP_VISA_P_ENCRYPTION)
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
						
						if(msgLength < 2048)
						{
							memcpy(OriData,&optData[1],msgLength);
							OriLen = msgLength;
							
							api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
							msgLength = EryptLen;
						}												
					}
					else
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
					}
				}				
			}
			else
			{	
				rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_VISA,*iptData,&glv_CAPK_Addr);

				if(rspCode == SUCCESS)
				{
					rspCode = api_fls_read(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						Key_Data_Len = i;
						
						//Reset to 1
						i = 1;

						//Final, Packing key Data
						//Index
						optData[i++] = tmpCAPK.Index;
						//Key Length (2 bytes)
						optData[i++] = (Key_Data_Len & 0xFF00) >> 8;
						optData[i++] = (Key_Data_Len & 0x00FF);
						//Key Data
						memcpy(&optData[i],Key_Data,Key_Data_Len);
						i+=Key_Data_Len;

						msgLength = i-1;

						//20140102
						if(VAP_VISA_P_ENCRYPTION)
						{				
							if(msgLength < 2048)
							{
								memcpy(OriData,&optData[1],msgLength);
								OriLen = msgLength;
								
								api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
								msgLength = EryptLen;
							}												
						}
						
						optData[0]=VAP_RIF_RC_SUCCESS;
					}
					else
					{
						optData[0]=VAP_RIF_RC_FAILURE;
					}

				}
				else
				{
					optData[0]=VAP_RIF_RC_INVALID_VISA_CA_KEY;	//20140103 change rc_code
				}
			}
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}				
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
}


void api_pcd_vap_adm_SetVisaPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR 	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR 	AID[5] = {0xA0,0x00,0x00,0x00,0x03};
	UCHAR   exp_1[3] = {0x00,0x00,0x03};
	UCHAR   exp_3[3] = {0x01,0x00,0x01};

	UCHAR   Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UCHAR 	hasherr = 0;

	UINT	lenOfKeyData = 0;
	UCHAR	lenOfKeyModule = 0;
	UCHAR 	lenError = 0;

	UCHAR	flgCommand = FALSE;

	UCHAR	IndexErr = FALSE;		//20140128, In production, we wont check CAPK Index
	
	//UCHAR	i=0;

	UINT 	Input_Msg_Len = 0;
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];

	if (api_pcd_vap_flgAdmMode == TRUE)
	{	
		if(iptData[0] == 0x00)		//Add CAPK
		{	
			//20140128, In production, we wont check CAPK Index
			//check index First
			/*
			for(i=0;i<VAP_VISA_CAPK_Table[0];i++)
			{
				if(*(iptData+1) == VAP_VISA_CAPK_Table[i+1])
				{
					IndexErr = FALSE;
					break;
				}				
			}*/
			//if key index showed in VISA CAPK Index, we took some operation to it, else it return VAP_RIF_RC_INVALID_VISA_CA_KEY
			if(IndexErr == FALSE)
			{
				Input_Msg_Len -= 1;		//minus "key add" or "key del" command
		
				rspCode = API_FIND_ADD_CAPK_Addr(VAP_CAPK_ID_VISA,*(iptData+1),&glv_CAPK_Addr);
				if(rspCode == SUCCESS)
				{		
					memcpy(tmpCAPK.RID,AID,5);

					Input_Msg_Len -= 1;		//minus Key Index
					tmpCAPK.Index = *(iptData+1);

					Input_Msg_Len -= 2;		//minus "Length of Key data"
					lenOfKeyData = (*(iptData+2))*256 + (*(iptData+3));

					if(lenOfKeyData != Input_Msg_Len)
						lenError = TRUE;

					if(!lenError)
					{
						lenOfKeyData -= 1;	//minus "Key CA Hash Algorithm Indicator"
						tmpCAPK.hshIndicator = *(iptData+4);

						lenOfKeyData -= 1;	//minus "Key CAPK Algorithm Indicator"
						tmpCAPK.keyIndicator = *(iptData+5);

						lenOfKeyData -= 1;	//minus "length of Key Modulus Length"
						tmpCAPK.Length = *(iptData+6);
						lenOfKeyModule = *(iptData+6);
						
						lenOfKeyData -= lenOfKeyModule;	//minus "Modulus Length"
						memcpy(tmpCAPK.Modulus,(iptData+7),tmpCAPK.Length);

						if(*(iptData+7+tmpCAPK.Length) == 0x01)
						{
							lenOfKeyData -= 1;	//minus "key Exponent Length"
							lenOfKeyData -= 1;	//minus "key Exponent"
							memcpy(tmpCAPK.Exponent,exp_1,3);

							lenOfKeyData -= 20;	//minus "Key Hash Result"
							memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+1),20);
						}
						else
						{
							lenOfKeyData -= 1;	//minus "key Exponent Length"
							lenOfKeyData -= 3;	//minus "key Exponent"
							memcpy(tmpCAPK.Exponent,exp_3,3);

							lenOfKeyData -= 20;	//minus "Key Hash Result"
							memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+3),20);
						}

						if(lenOfKeyData != 0)
							lenError = TRUE;

						if(!lenError)
						{
							//Second, Calculate the Key Hash Result
							memcpy(Hash_Input_Data,tmpCAPK.RID,5);
							Hash_Input_Len += 5;

							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
							Hash_Input_Len += tmpCAPK.Length;

							if(tmpCAPK.Exponent[0] == 0x00)
							{
								Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
							}
							else
							{
								memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
								Hash_Input_Len += 3;
							}

							api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);

							if(memcmp(tmpCAPK.hshValue,Hash_Result,20))
								hasherr = TRUE;

							if(!hasherr)
							{
#ifdef _PLATFORM_AS210
								rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
								rspCode = apiOK;
#endif
								if(rspCode == apiOK)
								{
									rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

									if(rspCode == apiFailed)
										flgError = TRUE;
								}
								else
									flgError = TRUE;
							}
							else
								flgError = TRUE;
						}
						else
							flgError = TRUE;			
					}		
					else
						flgError = TRUE;
				}
				else
					flgError = TRUE;
			}
			else
				flgError = TRUE;
		}
		else if(iptData[0] == 0x01)						//Del CAPK
		{	
			//In production, we wont check CAPK Index
			//check index First
			/*
			for(i=0;i<VAP_VISA_CAPK_Table[0];i++)
			{
				if(*(iptData+1) == VAP_VISA_CAPK_Table[i+1])
				{
					IndexErr = FALSE;
					break;
				}				
			}*/
			//if key index showed in VISA CAPK Index, we took some operation to it, else it return VAP_RIF_RC_INVALID_VISA_CA_KEY
			if(IndexErr == FALSE)
			{
				rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_VISA,*(iptData+1),&glv_CAPK_Addr);

				if(rspCode == SUCCESS)
				{
#ifdef _PLATFORM_AS210
					rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
					rspCode = apiOK;
#endif
					if(rspCode == apiOK)
					{
						memset((UCHAR*)&tmpCAPK, 0xFF, CAPK_LEN);
						rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);
						
						if(rspCode == apiFailed)
							flgError = TRUE;
					}
					else
						flgError = TRUE;
				}
				else
					flgError = TRUE;
			}
			else
				flgError = TRUE;
		}
		else	//Wrong Command
		{
			flgError = TRUE;
			flgCommand = TRUE;
		}

		if (flgError == TRUE)
		{
			if(lenError)
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			else if(hasherr)
				optData[0]=VAP_RIF_RC_INVALID_VISA_CA_KEY;
			else if(flgCommand)
				optData[0]=VAP_RIF_RC_INVALID_COMMAND;
			else if(IndexErr)
				optData[0]=VAP_RIF_RC_INVALID_VISA_CA_KEY;
			else
				optData[0]=VAP_RIF_RC_FAILURE;
		}
		else
		{
			optData[0]=VAP_RIF_RC_SUCCESS;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
}

void api_pcd_vap_adm_GetMasterPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	//UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR 	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR 	AID[5] = {0xA0,0x00,0x00,0x00,0x04};
	UINT	i = 0, j = 0;
	UCHAR	Key_Data[300] = {0};
	UINT	Key_Data_Len = 0,Total_Key_Data_Len = 0;
	UCHAR	Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UINT	Num_Keys = 0;		//Number of VSDC CA Public Key
	UCHAR	flgReadFlash = FALSE;

	UINT 	Input_Msg_Len = 0;

	UCHAR	OriData[2048] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;
	
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];
	

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if(Input_Msg_Len == 1)
		{
			//20140128, In production, we use Flash
#ifdef _PLATFORM_AS210
			api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#endif
		
			if(*iptData == 0xFF)
			{
				for(j=0;j<CAPK_NUMBER;j++)
				{								
					rspCode = api_fls_read(FLSID_CAPK,j*CAPK_LEN,CAPK_LEN,(UCHAR *)&tmpCAPK);
					
					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//Reset
						i = 0;
						Key_Data_Len = 0;
						Hash_Input_Len = 0;
						
						//Accumulate the number of VSDC CA Public Key 
						Num_Keys++;
						
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						memmove(&Key_Data[3],Key_Data,i);
						Key_Data[0] = tmpCAPK.Index;
						Key_Data[1] = (i & 0xFF00) >> 8;
						Key_Data[2] = (i & 0x00FF);
						Key_Data_Len = i + 3;

						//Final
						memcpy(&optData[2+Total_Key_Data_Len],Key_Data,Key_Data_Len);
						Total_Key_Data_Len+=Key_Data_Len;

						//20140102
						if((Total_Key_Data_Len + 2) > VAP_VISA_P_MAX_BUF_SIZE)
						{
							Total_Key_Data_Len -= Key_Data_Len;
							Num_Keys -= 1;
							break;
						}
					}
					else
					{
						if(rspCode != apiOK)
						{
							flgReadFlash = TRUE;
							break;
						}
					}
				}

				if(flgReadFlash)						//if read flash error, we delete all data and set the RC code to Failure
				{
					optData[0]=VAP_RIF_RC_FAILURE;
					msgLength = 0;
				}
				else
				{
					//20140102
					optData[0]=VAP_RIF_RC_SUCCESS;
				
					if(VAP_VISA_P_ENCRYPTION)
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
						
						if(msgLength < 2048)
						{
							memcpy(OriData,&optData[1],msgLength);
							OriLen = msgLength;
							
							api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
							msgLength = EryptLen;
						}												
					}
					else
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
					}
				}								
			}
			else
			{	
				rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_MASTER,*iptData,&glv_CAPK_Addr);

				if(rspCode == SUCCESS)
				{
					rspCode = api_fls_read(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						Key_Data_Len = i;
						
						//Reset to 1
						i = 1;

						//Final, Packing key Data
						//Index
						optData[i++] = tmpCAPK.Index;
						//Key Length (2 bytes)
						optData[i++] = (Key_Data_Len & 0xFF00) >> 8;
						optData[i++] = (Key_Data_Len & 0x00FF);
						//Key Data
						memcpy(&optData[i],Key_Data,Key_Data_Len);
						i+=Key_Data_Len;

						msgLength = i-1;

						//20140102
						if(VAP_VISA_P_ENCRYPTION)
						{				
							if(msgLength < 2048)
							{
								memcpy(OriData,&optData[1],msgLength);
								OriLen = msgLength;
								
								api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
								msgLength = EryptLen;
							}												
						}
												
						optData[0]=VAP_RIF_RC_SUCCESS;
					}
					else
					{
						optData[0]=VAP_RIF_RC_FAILURE;
					}

				}
				else
				{
					optData[0]=VAP_RIF_RC_FAILURE;
				}
			}
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
}

void api_pcd_vap_adm_SetMasterPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR 	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR 	AID[5] = {0xA0,0x00,0x00,0x00,0x04};
	UCHAR   exp_1[3] = {0x00,0x00,0x03};
	UCHAR   exp_3[3] = {0x01,0x00,0x01};

	UCHAR   Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UCHAR 	hasherr = 0;

	UINT	lenOfKeyData = 0;
	UCHAR	lenOfKeyModule = 0;
	UCHAR 	lenError = 0;

	UCHAR	flgCommand = FALSE;
	
	//UCHAR	i = 0;

	UINT 	Input_Msg_Len = 0;
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];

	if (api_pcd_vap_flgAdmMode == TRUE)
	{	
		if(iptData[0] == 0x00)		//Add CAPK
		{		
			Input_Msg_Len -= 1;		//minus "key add" or "key del" command
		
			rspCode = API_FIND_ADD_CAPK_Addr(VAP_CAPK_ID_MASTER,*(iptData+1),&glv_CAPK_Addr);
			if(rspCode == SUCCESS)
			{		
				memcpy(tmpCAPK.RID,AID,5);

				Input_Msg_Len -= 1;		//minus Key Index
				tmpCAPK.Index = *(iptData+1);

				Input_Msg_Len -= 2;		//minus "Length of Key data"
				lenOfKeyData = (*(iptData+2))*256 + (*(iptData+3));

				if(lenOfKeyData != Input_Msg_Len)
					lenError = TRUE;

				if(!lenError)
				{
					lenOfKeyData -= 1;	//minus "Key CA Hash Algorithm Indicator"
					tmpCAPK.hshIndicator = *(iptData+4);

					lenOfKeyData -= 1;	//minus "Key CAPK Algorithm Indicator"
					tmpCAPK.keyIndicator = *(iptData+5);

					lenOfKeyData -= 1;	//minus "length of Key Modulus Length"
					tmpCAPK.Length = *(iptData+6);
					lenOfKeyModule = *(iptData+6);
					
					lenOfKeyData -= lenOfKeyModule;	//minus "Modulus Length"
					memcpy(tmpCAPK.Modulus,(iptData+7),tmpCAPK.Length);

					if(*(iptData+7+tmpCAPK.Length) == 0x01)
					{
						lenOfKeyData -= 1;	//minus "key Exponent Length"
						lenOfKeyData -= 1;	//minus "key Exponent"
						memcpy(tmpCAPK.Exponent,exp_1,3);

						lenOfKeyData -= 20;	//minus "Key Hash Result"
						memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+1),20);
					}
					else
					{
						lenOfKeyData -= 1;	//minus "key Exponent Length"
						lenOfKeyData -= 3;	//minus "key Exponent"
						memcpy(tmpCAPK.Exponent,exp_3,3);

						lenOfKeyData -= 20;	//minus "Key Hash Result"
						memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+3),20);
					}

					if(lenOfKeyData != 0)
						lenError = TRUE;

					if(!lenError)
					{
						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);

						if(memcmp(tmpCAPK.hshValue,Hash_Result,20))
							hasherr = TRUE;

						if(!hasherr)
						{
#ifdef _PLATFORM_AS210
							rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
							rspCode = apiOK;
#endif
							if(rspCode == apiOK)
							{
								rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

								if(rspCode == apiFailed)
									flgError = TRUE;
							}
							else
								flgError = TRUE;
						}
						else
							flgError = TRUE;
					}
					else
						flgError = TRUE;			
				}		
				else
					flgError = TRUE;
			}
			else
				flgError = TRUE;
		}
		else if(iptData[0] == 0x01)						//Del CAPK
		{		
			rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_MASTER,*(iptData+1),&glv_CAPK_Addr);

			if(rspCode == SUCCESS)
			{
#ifdef _PLATFORM_AS210
				rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
				rspCode = apiOK;
#endif
				if(rspCode == apiOK)
				{
					memset((UCHAR*)&tmpCAPK, 0xFF, CAPK_LEN);
					rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

					if(rspCode == apiFailed)
						flgError = TRUE;
				}
				else
					flgError = TRUE;
			}
			else
				flgError = TRUE;
		}
		else
		{
			flgError = TRUE;
			flgCommand = TRUE;
		}

		if (flgError == TRUE)
		{
			if(lenError)
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			else if(hasherr)
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			else if(flgCommand)
				optData[0]=VAP_RIF_RC_INVALID_COMMAND;
			else
				optData[0]=VAP_RIF_RC_FAILURE;
		}
		else
		{
			optData[0]=VAP_RIF_RC_SUCCESS;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_FAILURE;
	}
	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
}

void api_pcd_vap_adm_GetJCBPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	//UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR 	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR	AID[5] = {0xA0,0x00,0x00,0x00,0x65};
	UINT	i = 0, j = 0;
	UCHAR	Key_Data[300] = {0};
	UINT	Key_Data_Len = 0,Total_Key_Data_Len = 0;
	UCHAR	Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UINT	Num_Keys = 0;		//Number of VSDC CA Public Key
	UCHAR	flgReadFlash = FALSE;

	UINT	Input_Msg_Len = 0;

	UCHAR	OriData[2048] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;
	
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if(Input_Msg_Len == 1)
		{
#ifdef _PLATFORM_AS210
			api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#endif

			
			if(*iptData == 0xFF)
			{
				for(j=0;j<CAPK_NUMBER;j++)
				{								
					rspCode = api_fls_read(FLSID_CAPK,j*CAPK_LEN,CAPK_LEN,(UCHAR *)&tmpCAPK);
					
					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//Reset
						i = 0;
						Key_Data_Len = 0;
						Hash_Input_Len = 0;
						
						//Accumulate the number of VSDC CA Public Key 
						Num_Keys++;
						
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] =  tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						memmove(&Key_Data[3],Key_Data,i);
						Key_Data[0] = tmpCAPK.Index;
						Key_Data[1] = (i & 0xFF00) >> 8;
						Key_Data[2] = (i & 0x00FF);
						Key_Data_Len = i + 3;

						//Final
						memcpy(&optData[2+Total_Key_Data_Len],Key_Data,Key_Data_Len);
						Total_Key_Data_Len+=Key_Data_Len;

						//20140102
						if((Total_Key_Data_Len + 2) > VAP_VISA_P_MAX_BUF_SIZE)
						{
							Total_Key_Data_Len -= Key_Data_Len;
							Num_Keys -= 1;
							break;
						}
					}
					else
					{
						if(rspCode != apiOK)
						{
							flgReadFlash = TRUE;
							break;
						}
					}
				}
				
				if(flgReadFlash)						//if read flash error, we delete all data and set the RC code to Failure
				{
					optData[0]=VAP_RIF_RC_FAILURE;
					msgLength = 0;
				}
				else
				{
					//20140102
					optData[0]=VAP_RIF_RC_SUCCESS;
				
					if(VAP_VISA_P_ENCRYPTION)
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
						
						if(msgLength < 2048)
						{
							memcpy(OriData,&optData[1],msgLength);
							OriLen = msgLength;
							
							api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
							msgLength = EryptLen;
						}												
					}
					else
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
					}
				}					
			}
			else
			{	
				rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_JCB,*iptData,&glv_CAPK_Addr);

				if(rspCode == SUCCESS)
				{
					rspCode = api_fls_read(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						Key_Data_Len = i;
						
						//Reset to 1
						i = 1;

						//Final, Packing key Data
						//Index
						optData[i++] = tmpCAPK.Index;
						//Key Length (2 bytes)
						optData[i++] = (Key_Data_Len & 0xFF00) >> 8;
						optData[i++] = (Key_Data_Len & 0x00FF);
						//Key Data
						memcpy(&optData[i],Key_Data,Key_Data_Len);
						i+=Key_Data_Len;

						msgLength = i-1;

						//20140102
						if(VAP_VISA_P_ENCRYPTION)
						{				
							if(msgLength < 2048)
							{
								memcpy(OriData,&optData[1],msgLength);
								OriLen = msgLength;
								
								api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
								msgLength = EryptLen;
							}												
						}
						
						optData[0]=VAP_RIF_RC_SUCCESS;
					}
					else
					{
						optData[0]=VAP_RIF_RC_FAILURE;
					}

				}
				else
				{
					optData[0]=VAP_RIF_RC_INVALID_JCB_CA_KEY;	//20140103 change rc_code
				}
			}

		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}				
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);

}

void api_pcd_vap_adm_SetJCBPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR	AID[5] = {0xA0,0x00,0x00,0x00,0x65};
	UCHAR	exp_1[3] = {0x00,0x00,0x03};
	UCHAR	exp_3[3] = {0x01,0x00,0x01};

	UCHAR   Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UCHAR 	hasherr = 0;

	UINT	lenOfKeyData = 0;
	UCHAR	lenOfKeyModule = 0;
	UCHAR 	lenError = 0;

	UCHAR	flgCommand = FALSE;

	UCHAR	IndexErr = FALSE;	//20140128, In production, we wont check CAPK Index

	//UCHAR	i = 0;

	UINT 	Input_Msg_Len = 0;
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];

	if (api_pcd_vap_flgAdmMode == TRUE)
	{	
		if(iptData[0] == 0x00)		//Add CAPK
		{		
			//20140128, In production, we wont check CAPK Index
			//check index First
			/*
			for(i=0;i<VAP_JCB_CAPK_Table[0];i++)
			{
				if(*(iptData+1) == VAP_JCB_CAPK_Table[i+1])
				{
					IndexErr = FALSE;
					break;
				}				
			}*/
			//if key index showed in JCB CAPK Index, we took some operations to it, else it return VAP_RIF_RC_INVALID_JCB_CA_KEY
			if(IndexErr == FALSE)
			{
				Input_Msg_Len -= 1;		//minus "key add" or "key del" command
		
				rspCode = API_FIND_ADD_CAPK_Addr(VAP_CAPK_ID_JCB,*(iptData+1),&glv_CAPK_Addr);
				if(rspCode == SUCCESS)
				{		
					memcpy(tmpCAPK.RID,AID,5);

					Input_Msg_Len -= 1;		//minus Key Index
					tmpCAPK.Index = *(iptData+1);

					Input_Msg_Len -= 2;		//minus "Length of Key data"
					lenOfKeyData = (*(iptData+2))*256 + (*(iptData+3));

					if(lenOfKeyData != Input_Msg_Len)
						lenError = TRUE;

					if(!lenError)
					{
						lenOfKeyData -= 1;	//minus "Key CA Hash Algorithm Indicator"
						tmpCAPK.hshIndicator = *(iptData+4);

						lenOfKeyData -= 1;	//minus "Key CAPK Algorithm Indicator"
						tmpCAPK.keyIndicator = *(iptData+5);

						lenOfKeyData -= 1;	//minus "length of Key Modulus Length"
						tmpCAPK.Length = *(iptData+6);
						lenOfKeyModule = *(iptData+6);
						
						lenOfKeyData -= lenOfKeyModule;	//minus "Modulus Length"
						memcpy(tmpCAPK.Modulus,(iptData+7),tmpCAPK.Length);

						if(*(iptData+7+tmpCAPK.Length) == 0x01)
						{
							lenOfKeyData -= 1;	//minus "key Exponent Length"
							lenOfKeyData -= 1;	//minus "key Exponent"
							memcpy(tmpCAPK.Exponent,exp_1,3);

							lenOfKeyData -= 20;	//minus "Key Hash Result"
							memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+1),20);
						}
						else
						{
							lenOfKeyData -= 1;	//minus "key Exponent Length"
							lenOfKeyData -= 3;	//minus "key Exponent"
							memcpy(tmpCAPK.Exponent,exp_3,3);

							lenOfKeyData -= 20;	//minus "Key Hash Result"
							memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+3),20);
						}

						if(lenOfKeyData != 0)
							lenError = TRUE;

						if(!lenError)
						{
							//Second, Calculate the Key Hash Result
							memcpy(Hash_Input_Data,tmpCAPK.RID,5);
							Hash_Input_Len += 5;

							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
							Hash_Input_Len += tmpCAPK.Length;

							if(tmpCAPK.Exponent[0] == 0x00)
							{
								Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
							}
							else
							{
								memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
								Hash_Input_Len += 3;
							}

							api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);

							if(memcmp(tmpCAPK.hshValue,Hash_Result,20))
								hasherr = TRUE;

							if(!hasherr)
							{
#ifdef _PLATFORM_AS210
								rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
								rspCode = apiOK;
#endif
								if(rspCode == apiOK)
								{								
									rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

									if(rspCode == apiFailed)
										flgError = TRUE;
								}
								else
									flgError = TRUE;
							}
							else
								flgError = TRUE;
						}
						else
							flgError = TRUE;			
					}		
					else
						flgError = TRUE;
				}
				else
					flgError = TRUE;
			}
			else
				flgError = TRUE;
		}
		else if(iptData[0] == 0x01)						//Del CAPK
		{
			//20140128, In production, we wont check CAPK Index
			//check index First
			/*
			for(i=0;i<VAP_JCB_CAPK_Table[0];i++)
			{
				if(*(iptData+1) == VAP_JCB_CAPK_Table[i+1])
				{
					IndexErr = FALSE;
					break;
				}				
			}*/
			//if key index showed in JCB CAPK Index, we took some operation to it, else it return VAP_RIF_RC_INVALID_JCB_CA_KEY
			if(IndexErr == FALSE)
			{
				rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_JCB,*(iptData+1),&glv_CAPK_Addr);

				if(rspCode == SUCCESS)
				{
#ifdef _PLATFORM_AS210
					rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
					rspCode = apiOK;
#endif
					if(rspCode == apiOK)
					{
						memset((UCHAR*)&tmpCAPK, 0xFF, CAPK_LEN);
						rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

						if(rspCode == apiFailed)
							flgError = TRUE;
					}
					else
						flgError = TRUE;
				}
				else
					flgError = TRUE;
			}
			else
				flgError = TRUE;
		}
		else
		{
			flgError = TRUE;
			flgCommand = TRUE;
		}

		if (flgError == TRUE)
		{
			if(lenError)
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			else if(hasherr)
				optData[0]=VAP_RIF_RC_INVALID_JCB_CA_KEY;
			else if(flgCommand)
				optData[0]=VAP_RIF_RC_INVALID_COMMAND;
			else if(IndexErr)
				optData[0]=VAP_RIF_RC_INVALID_JCB_CA_KEY;
			else
				optData[0]=VAP_RIF_RC_FAILURE;
		}
		else
		{
			optData[0]=VAP_RIF_RC_SUCCESS;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
}

void api_pcd_vap_adm_GetUPIPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	//UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR 	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR 	AID[5] = {0xA0,0x00,0x00,0x03,0x33};
	UINT	i = 0, j = 0;
	UCHAR	Key_Data[300] = {0};
	UINT	Key_Data_Len = 0,Total_Key_Data_Len = 0;
	UCHAR	Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UINT	Num_Keys = 0;		//Number of VSDC CA Public Key
	UCHAR	flgReadFlash = FALSE;

	UINT 	Input_Msg_Len = 0;

	UCHAR	OriData[2048] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;
	
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];
	

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if(Input_Msg_Len == 1)
		{
			//20140128, In production, we use Flash
#ifdef _PLATFORM_AS210
			api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#endif
		
			if(*iptData == 0xFF)
			{
				for(j=0;j<CAPK_NUMBER;j++)
				{								
					rspCode = api_fls_read(FLSID_CAPK,j*CAPK_LEN,CAPK_LEN,(UCHAR *)&tmpCAPK);
					
					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//Reset
						i = 0;
						Key_Data_Len = 0;
						Hash_Input_Len = 0;
						
						//Accumulate the number of VSDC CA Public Key 
						Num_Keys++;
						
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						memmove(&Key_Data[3],Key_Data,i);
						Key_Data[0] = tmpCAPK.Index;
						Key_Data[1] = (i & 0xFF00) >> 8;
						Key_Data[2] = (i & 0x00FF);
						Key_Data_Len = i + 3;

						//Final
						memcpy(&optData[2+Total_Key_Data_Len],Key_Data,Key_Data_Len);
						Total_Key_Data_Len+=Key_Data_Len;

						//20140102
						if((Total_Key_Data_Len + 2) > VAP_VISA_P_MAX_BUF_SIZE)
						{
							Total_Key_Data_Len -= Key_Data_Len;
							Num_Keys -= 1;
							break;
						}
					}
					else
					{
						if(rspCode != apiOK)
						{
							flgReadFlash = TRUE;
							break;
						}
					}
				}

				if(flgReadFlash)						//if read flash error, we delete all data and set the RC code to Failure
				{
					optData[0]=VAP_RIF_RC_FAILURE;
					msgLength = 0;
				}
				else
				{
					//20140102
					optData[0]=VAP_RIF_RC_SUCCESS;
				
					if(VAP_VISA_P_ENCRYPTION)
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
						
						if(msgLength < 2048)
						{
							memcpy(OriData,&optData[1],msgLength);
							OriLen = msgLength;
							
							api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
							msgLength = EryptLen;
						}												
					}
					else
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
					}
				}								
			}
			else
			{	
				rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_UPI,*iptData,&glv_CAPK_Addr);

				if(rspCode == SUCCESS)
				{
					rspCode = api_fls_read(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						Key_Data_Len = i;
						
						//Reset to 1
						i = 1;

						//Final, Packing key Data
						//Index
						optData[i++] = tmpCAPK.Index;
						//Key Length (2 bytes)
						optData[i++] = (Key_Data_Len & 0xFF00) >> 8;
						optData[i++] = (Key_Data_Len & 0x00FF);
						//Key Data
						memcpy(&optData[i],Key_Data,Key_Data_Len);
						i+=Key_Data_Len;

						msgLength = i-1;

						//20140102
						if(VAP_VISA_P_ENCRYPTION)
						{				
							if(msgLength < 2048)
							{
								memcpy(OriData,&optData[1],msgLength);
								OriLen = msgLength;
								
								api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
								msgLength = EryptLen;
							}												
						}
												
						optData[0]=VAP_RIF_RC_SUCCESS;
					}
					else
					{
						optData[0]=VAP_RIF_RC_FAILURE;
					}

				}
				else
				{
					optData[0]=VAP_RIF_RC_FAILURE;
				}
			}
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
}

void api_pcd_vap_adm_SetUPIPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR 	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR 	AID[5] = {0xA0,0x00,0x00,0x03,0x33};
	UCHAR   exp_1[3] = {0x00,0x00,0x03};
	UCHAR   exp_3[3] = {0x01,0x00,0x01};

	UCHAR   Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UCHAR 	hasherr = 0;

	UINT	lenOfKeyData = 0;
	UCHAR	lenOfKeyModule = 0;
	UCHAR 	lenError = 0;

	UCHAR	flgCommand = FALSE;
	
	//UCHAR	i = 0;

	UINT 	Input_Msg_Len = 0;
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];

	if (api_pcd_vap_flgAdmMode == TRUE)
	{	
		if(iptData[0] == 0x00)		//Add CAPK
		{		
			Input_Msg_Len -= 1;		//minus "key add" or "key del" command
		
			rspCode = API_FIND_ADD_CAPK_Addr(VAP_CAPK_ID_UPI,*(iptData+1),&glv_CAPK_Addr);
			if(rspCode == SUCCESS)
			{		
				memcpy(tmpCAPK.RID,AID,5);

				Input_Msg_Len -= 1;		//minus Key Index
				tmpCAPK.Index = *(iptData+1);

				Input_Msg_Len -= 2;		//minus "Length of Key data"
				lenOfKeyData = (*(iptData+2))*256 + (*(iptData+3));

				if(lenOfKeyData != Input_Msg_Len)
					lenError = TRUE;

				if(!lenError)
				{
					lenOfKeyData -= 1;	//minus "Key CA Hash Algorithm Indicator"
					tmpCAPK.hshIndicator = *(iptData+4);

					lenOfKeyData -= 1;	//minus "Key CAPK Algorithm Indicator"
					tmpCAPK.keyIndicator = *(iptData+5);

					lenOfKeyData -= 1;	//minus "length of Key Modulus Length"
					tmpCAPK.Length = *(iptData+6);
					lenOfKeyModule = *(iptData+6);
					
					lenOfKeyData -= lenOfKeyModule;	//minus "Modulus Length"
					memcpy(tmpCAPK.Modulus,(iptData+7),tmpCAPK.Length);

					if(*(iptData+7+tmpCAPK.Length) == 0x01)
					{
						lenOfKeyData -= 1;	//minus "key Exponent Length"
						lenOfKeyData -= 1;	//minus "key Exponent"
						memcpy(tmpCAPK.Exponent,exp_1,3);

						lenOfKeyData -= 20;	//minus "Key Hash Result"
						memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+1),20);
					}
					else
					{
						lenOfKeyData -= 1;	//minus "key Exponent Length"
						lenOfKeyData -= 3;	//minus "key Exponent"
						memcpy(tmpCAPK.Exponent,exp_3,3);

						lenOfKeyData -= 20;	//minus "Key Hash Result"
						memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+3),20);
					}

					if(lenOfKeyData != 0)
						lenError = TRUE;

					if(!lenError)
					{
						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);

						if(memcmp(tmpCAPK.hshValue,Hash_Result,20))
							hasherr = TRUE;

						if(!hasherr)
						{
#ifdef _PLATFORM_AS210
							rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
							rspCode = apiOK;
#endif
							if(rspCode == apiOK)
							{
								rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

								if(rspCode == apiFailed)
									flgError = TRUE;
							}
							else
								flgError = TRUE;
						}
						else
							flgError = TRUE;
					}
					else
						flgError = TRUE;			
				}		
				else
					flgError = TRUE;
			}
			else
				flgError = TRUE;
		}
		else if(iptData[0] == 0x01)						//Del CAPK
		{		
			rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_UPI,*(iptData+1),&glv_CAPK_Addr);

			if(rspCode == SUCCESS)
			{
#ifdef _PLATFORM_AS210
				rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
				rspCode = apiOK;
#endif
				if(rspCode == apiOK)
				{
					memset((UCHAR*)&tmpCAPK, 0xFF, CAPK_LEN);
					rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

					if(rspCode == apiFailed)
						flgError = TRUE;
				}
				else
					flgError = TRUE;
			}
			else
				flgError = TRUE;
		}
		else
		{
			flgError = TRUE;
			flgCommand = TRUE;
		}

		if (flgError == TRUE)
		{
			if(lenError)
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			else if(hasherr)
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			else if(flgCommand)
				optData[0]=VAP_RIF_RC_INVALID_COMMAND;
			else
				optData[0]=VAP_RIF_RC_FAILURE;
		}
		else
		{
			optData[0]=VAP_RIF_RC_SUCCESS;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_FAILURE;
	}
	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
}

void api_pcd_vap_adm_GetAEPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	//UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR 	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR 	AID[5] = {0xA0,0x00,0x00,0x00,0x25};
	UINT	i = 0, j = 0;
	UCHAR	Key_Data[300] = {0};
	UINT	Key_Data_Len = 0,Total_Key_Data_Len = 0;
	UCHAR	Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UINT	Num_Keys = 0;		//Number of VSDC CA Public Key
	UCHAR	flgReadFlash = FALSE;

	UINT 	Input_Msg_Len = 0;

	UCHAR	OriData[2048] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;
	
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];
	

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if(Input_Msg_Len == 1)
		{
			//20140128, In production, we use Flash
#ifdef _PLATFORM_AS210
			api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#endif
		
			if(*iptData == 0xFF)
			{
				for(j=0;j<CAPK_NUMBER;j++)
				{								
					rspCode = api_fls_read(FLSID_CAPK,j*CAPK_LEN,CAPK_LEN,(UCHAR *)&tmpCAPK);
					
					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//Reset
						i = 0;
						Key_Data_Len = 0;
						Hash_Input_Len = 0;
						
						//Accumulate the number of VSDC CA Public Key 
						Num_Keys++;
						
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						memmove(&Key_Data[3],Key_Data,i);
						Key_Data[0] = tmpCAPK.Index;
						Key_Data[1] = (i & 0xFF00) >> 8;
						Key_Data[2] = (i & 0x00FF);
						Key_Data_Len = i + 3;

						//Final
						memcpy(&optData[2+Total_Key_Data_Len],Key_Data,Key_Data_Len);
						Total_Key_Data_Len+=Key_Data_Len;

						//20140102
						if((Total_Key_Data_Len + 2) > VAP_VISA_P_MAX_BUF_SIZE)
						{
							Total_Key_Data_Len -= Key_Data_Len;
							Num_Keys -= 1;
							break;
						}
					}
					else
					{
						if(rspCode != apiOK)
						{
							flgReadFlash = TRUE;
							break;
						}
					}
				}

				if(flgReadFlash)						//if read flash error, we delete all data and set the RC code to Failure
				{
					optData[0]=VAP_RIF_RC_FAILURE;
					msgLength = 0;
				}
				else
				{
					//20140102
					optData[0]=VAP_RIF_RC_SUCCESS;
				
					if(VAP_VISA_P_ENCRYPTION)
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
						
						if(msgLength < 2048)
						{
							memcpy(OriData,&optData[1],msgLength);
							OriLen = msgLength;
							
							api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
							msgLength = EryptLen;
						}												
					}
					else
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
					}
				}								
			}
			else
			{	
				rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_AE,*iptData,&glv_CAPK_Addr);

				if(rspCode == SUCCESS)
				{
					rspCode = api_fls_read(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						Key_Data_Len = i;
						
						//Reset to 1
						i = 1;

						//Final, Packing key Data
						//Index
						optData[i++] = tmpCAPK.Index;
						//Key Length (2 bytes)
						optData[i++] = (Key_Data_Len & 0xFF00) >> 8;
						optData[i++] = (Key_Data_Len & 0x00FF);
						//Key Data
						memcpy(&optData[i],Key_Data,Key_Data_Len);
						i+=Key_Data_Len;

						msgLength = i-1;

						//20140102
						if(VAP_VISA_P_ENCRYPTION)
						{				
							if(msgLength < 2048)
							{
								memcpy(OriData,&optData[1],msgLength);
								OriLen = msgLength;
								
								api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
								msgLength = EryptLen;
							}												
						}
												
						optData[0]=VAP_RIF_RC_SUCCESS;
					}
					else
					{
						optData[0]=VAP_RIF_RC_FAILURE;
					}

				}
				else
				{
					optData[0]=VAP_RIF_RC_FAILURE;
				}
			}
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
}

void api_pcd_vap_adm_SetAEPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR 	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR 	AID[5] = {0xA0,0x00,0x00,0x00,0x25};
	UCHAR   exp_1[3] = {0x00,0x00,0x03};
	UCHAR   exp_3[3] = {0x01,0x00,0x01};

	UCHAR   Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UCHAR 	hasherr = 0;

	UINT	lenOfKeyData = 0;
	UCHAR	lenOfKeyModule = 0;
	UCHAR 	lenError = 0;

	UCHAR	flgCommand = FALSE;
	
	//UCHAR	i = 0;

	UINT 	Input_Msg_Len = 0;
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];

	if (api_pcd_vap_flgAdmMode == TRUE)
	{	
		if(iptData[0] == 0x00)		//Add CAPK
		{		
			Input_Msg_Len -= 1;		//minus "key add" or "key del" command
		
			rspCode = API_FIND_ADD_CAPK_Addr(VAP_CAPK_ID_AE,*(iptData+1),&glv_CAPK_Addr);
			if(rspCode == SUCCESS)
			{		
				memcpy(tmpCAPK.RID,AID,5);

				Input_Msg_Len -= 1;		//minus Key Index
				tmpCAPK.Index = *(iptData+1);

				Input_Msg_Len -= 2;		//minus "Length of Key data"
				lenOfKeyData = (*(iptData+2))*256 + (*(iptData+3));

				if(lenOfKeyData != Input_Msg_Len)
					lenError = TRUE;

				if(!lenError)
				{
					lenOfKeyData -= 1;	//minus "Key CA Hash Algorithm Indicator"
					tmpCAPK.hshIndicator = *(iptData+4);

					lenOfKeyData -= 1;	//minus "Key CAPK Algorithm Indicator"
					tmpCAPK.keyIndicator = *(iptData+5);

					lenOfKeyData -= 1;	//minus "length of Key Modulus Length"
					tmpCAPK.Length = *(iptData+6);
					lenOfKeyModule = *(iptData+6);
					
					lenOfKeyData -= lenOfKeyModule;	//minus "Modulus Length"
					memcpy(tmpCAPK.Modulus,(iptData+7),tmpCAPK.Length);

					if(*(iptData+7+tmpCAPK.Length) == 0x01)
					{
						lenOfKeyData -= 1;	//minus "key Exponent Length"
						lenOfKeyData -= 1;	//minus "key Exponent"
						memcpy(tmpCAPK.Exponent,exp_1,3);

						lenOfKeyData -= 20;	//minus "Key Hash Result"
						memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+1),20);
					}
					else
					{
						lenOfKeyData -= 1;	//minus "key Exponent Length"
						lenOfKeyData -= 3;	//minus "key Exponent"
						memcpy(tmpCAPK.Exponent,exp_3,3);

						lenOfKeyData -= 20;	//minus "Key Hash Result"
						memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+3),20);
					}

					if(lenOfKeyData != 0)
						lenError = TRUE;

					if(!lenError)
					{
						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);

						if(memcmp(tmpCAPK.hshValue,Hash_Result,20))
							hasherr = TRUE;

						if(!hasherr)
						{
#ifdef _PLATFORM_AS210
							rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
							rspCode = apiOK;
#endif
							if(rspCode == apiOK)
							{
								rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

								if(rspCode == apiFailed)
									flgError = TRUE;
							}
							else
								flgError = TRUE;
						}
						else
							flgError = TRUE;
					}
					else
						flgError = TRUE;			
				}		
				else
					flgError = TRUE;
			}
			else
				flgError = TRUE;
		}
		else if(iptData[0] == 0x01)						//Del CAPK
		{		
			rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_AE,*(iptData+1),&glv_CAPK_Addr);

			if(rspCode == SUCCESS)
			{
#ifdef _PLATFORM_AS210
				rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
				rspCode = apiOK;
#endif
				if(rspCode == apiOK)
				{
					memset((UCHAR*)&tmpCAPK, 0xFF, CAPK_LEN);
					rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

					if(rspCode == apiFailed)
						flgError = TRUE;
				}
				else
					flgError = TRUE;
			}
			else
				flgError = TRUE;
		}
		else
		{
			flgError = TRUE;
			flgCommand = TRUE;
		}

		if (flgError == TRUE)
		{
			if(lenError)
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			else if(hasherr)
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			else if(flgCommand)
				optData[0]=VAP_RIF_RC_INVALID_COMMAND;
			else
				optData[0]=VAP_RIF_RC_FAILURE;
		}
		else
		{
			optData[0]=VAP_RIF_RC_SUCCESS;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_FAILURE;
	}
	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
}

void api_pcd_vap_adm_GetDiscoverPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	//UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR 	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR 	AID[5] = {0xA0,0x00,0x00,0x01,0x52};
	UINT	i = 0, j = 0;
	UCHAR	Key_Data[300] = {0};
	UINT	Key_Data_Len = 0,Total_Key_Data_Len = 0;
	UCHAR	Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UINT	Num_Keys = 0;		//Number of VSDC CA Public Key
	UCHAR	flgReadFlash = FALSE;

	UINT 	Input_Msg_Len = 0;

	UCHAR	OriData[2048] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;
	
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];
	

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if(Input_Msg_Len == 1)
		{
			//20140128, In production, we use Flash
#ifdef _PLATFORM_AS210
			api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#endif
		
			if(*iptData == 0xFF)
			{
				for(j=0;j<CAPK_NUMBER;j++)
				{								
					rspCode = api_fls_read(FLSID_CAPK,j*CAPK_LEN,CAPK_LEN,(UCHAR *)&tmpCAPK);
					
					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//Reset
						i = 0;
						Key_Data_Len = 0;
						Hash_Input_Len = 0;
						
						//Accumulate the number of VSDC CA Public Key 
						Num_Keys++;
						
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						memmove(&Key_Data[3],Key_Data,i);
						Key_Data[0] = tmpCAPK.Index;
						Key_Data[1] = (i & 0xFF00) >> 8;
						Key_Data[2] = (i & 0x00FF);
						Key_Data_Len = i + 3;

						//Final
						memcpy(&optData[2+Total_Key_Data_Len],Key_Data,Key_Data_Len);
						Total_Key_Data_Len+=Key_Data_Len;

						//20140102
						if((Total_Key_Data_Len + 2) > VAP_VISA_P_MAX_BUF_SIZE)
						{
							Total_Key_Data_Len -= Key_Data_Len;
							Num_Keys -= 1;
							break;
						}
					}
					else
					{
						if(rspCode != apiOK)
						{
							flgReadFlash = TRUE;
							break;
						}
					}
				}

				if(flgReadFlash)						//if read flash error, we delete all data and set the RC code to Failure
				{
					optData[0]=VAP_RIF_RC_FAILURE;
					msgLength = 0;
				}
				else
				{
					//20140102
					optData[0]=VAP_RIF_RC_SUCCESS;
				
					if(VAP_VISA_P_ENCRYPTION)
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
						
						if(msgLength < 2048)
						{
							memcpy(OriData,&optData[1],msgLength);
							OriLen = msgLength;
							
							api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
							msgLength = EryptLen;
						}												
					}
					else
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
					}
				}								
			}
			else
			{	
				rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_DISCOVER,*iptData,&glv_CAPK_Addr);

				if(rspCode == SUCCESS)
				{
					rspCode = api_fls_read(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						Key_Data_Len = i;
						
						//Reset to 1
						i = 1;

						//Final, Packing key Data
						//Index
						optData[i++] = tmpCAPK.Index;
						//Key Length (2 bytes)
						optData[i++] = (Key_Data_Len & 0xFF00) >> 8;
						optData[i++] = (Key_Data_Len & 0x00FF);
						//Key Data
						memcpy(&optData[i],Key_Data,Key_Data_Len);
						i+=Key_Data_Len;

						msgLength = i-1;

						//20140102
						if(VAP_VISA_P_ENCRYPTION)
						{				
							if(msgLength < 2048)
							{
								memcpy(OriData,&optData[1],msgLength);
								OriLen = msgLength;
								
								api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
								msgLength = EryptLen;
							}												
						}
												
						optData[0]=VAP_RIF_RC_SUCCESS;
					}
					else
					{
						optData[0]=VAP_RIF_RC_FAILURE;
					}

				}
				else
				{
					optData[0]=VAP_RIF_RC_FAILURE;
				}
			}
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
}

void api_pcd_vap_adm_SetDiscoverPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR 	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR 	AID[5] = {0xA0,0x00,0x00,0x01,0x52};
	UCHAR   exp_1[3] = {0x00,0x00,0x03};
	UCHAR   exp_3[3] = {0x01,0x00,0x01};

	UCHAR   Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UCHAR 	hasherr = 0;

	UINT	lenOfKeyData = 0;
	UCHAR	lenOfKeyModule = 0;
	UCHAR 	lenError = 0;

	UCHAR	flgCommand = FALSE;
	
	//UCHAR	i = 0;

	UINT 	Input_Msg_Len = 0;
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];

	if (api_pcd_vap_flgAdmMode == TRUE)
	{	
		if(iptData[0] == 0x00)		//Add CAPK
		{		
			Input_Msg_Len -= 1;		//minus "key add" or "key del" command
		
			rspCode = API_FIND_ADD_CAPK_Addr(VAP_CAPK_ID_DISCOVER,*(iptData+1),&glv_CAPK_Addr);
			if(rspCode == SUCCESS)
			{		
				memcpy(tmpCAPK.RID,AID,5);

				Input_Msg_Len -= 1;		//minus Key Index
				tmpCAPK.Index = *(iptData+1);

				Input_Msg_Len -= 2;		//minus "Length of Key data"
				lenOfKeyData = (*(iptData+2))*256 + (*(iptData+3));

				if(lenOfKeyData != Input_Msg_Len)
					lenError = TRUE;

				if(!lenError)
				{
					lenOfKeyData -= 1;	//minus "Key CA Hash Algorithm Indicator"
					tmpCAPK.hshIndicator = *(iptData+4);

					lenOfKeyData -= 1;	//minus "Key CAPK Algorithm Indicator"
					tmpCAPK.keyIndicator = *(iptData+5);

					lenOfKeyData -= 1;	//minus "length of Key Modulus Length"
					tmpCAPK.Length = *(iptData+6);
					lenOfKeyModule = *(iptData+6);
					
					lenOfKeyData -= lenOfKeyModule;	//minus "Modulus Length"
					memcpy(tmpCAPK.Modulus,(iptData+7),tmpCAPK.Length);

					if(*(iptData+7+tmpCAPK.Length) == 0x01)
					{
						lenOfKeyData -= 1;	//minus "key Exponent Length"
						lenOfKeyData -= 1;	//minus "key Exponent"
						memcpy(tmpCAPK.Exponent,exp_1,3);

						lenOfKeyData -= 20;	//minus "Key Hash Result"
						memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+1),20);
					}
					else
					{
						lenOfKeyData -= 1;	//minus "key Exponent Length"
						lenOfKeyData -= 3;	//minus "key Exponent"
						memcpy(tmpCAPK.Exponent,exp_3,3);

						lenOfKeyData -= 20;	//minus "Key Hash Result"
						memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+3),20);
					}

					if(lenOfKeyData != 0)
						lenError = TRUE;

					if(!lenError)
					{
						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);

						if(memcmp(tmpCAPK.hshValue,Hash_Result,20))
							hasherr = TRUE;

						if(!hasherr)
						{
#ifdef _PLATFORM_AS210
							rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
							rspCode = apiOK;
#endif
							if(rspCode == apiOK)
							{
								rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

								if(rspCode == apiFailed)
									flgError = TRUE;
							}
							else
								flgError = TRUE;
						}
						else
							flgError = TRUE;
					}
					else
						flgError = TRUE;			
				}		
				else
					flgError = TRUE;
			}
			else
				flgError = TRUE;
		}
		else if(iptData[0] == 0x01)						//Del CAPK
		{		
			rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_DISCOVER,*(iptData+1),&glv_CAPK_Addr);

			if(rspCode == SUCCESS)
			{
#ifdef _PLATFORM_AS210
				rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
				rspCode = apiOK;
#endif
				if(rspCode == apiOK)
				{
					memset((UCHAR*)&tmpCAPK, 0xFF, CAPK_LEN);
					rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

					if(rspCode == apiFailed)
						flgError = TRUE;
				}
				else
					flgError = TRUE;
			}
			else
				flgError = TRUE;
		}
		else
		{
			flgError = TRUE;
			flgCommand = TRUE;
		}

		if (flgError == TRUE)
		{
			if(lenError)
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			else if(hasherr)
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			else if(flgCommand)
				optData[0]=VAP_RIF_RC_INVALID_COMMAND;
			else
				optData[0]=VAP_RIF_RC_FAILURE;
		}
		else
		{
			optData[0]=VAP_RIF_RC_SUCCESS;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_FAILURE;
	}
	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);
}

void api_pcd_vap_adm_GetVisaTestAIDPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	//UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR 	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR	AID[5] = {0xA0,0x00,0x00,0x99,0x99};
	UINT	i = 0, j = 0;
	UCHAR	Key_Data[300] = {0};
	UINT	Key_Data_Len = 0,Total_Key_Data_Len = 0;
	UCHAR	Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UINT	Num_Keys = 0;		//Number of VSDC CA Public Key
	UCHAR	flgReadFlash = FALSE;

	UINT	Input_Msg_Len = 0;

	UCHAR	OriData[2048] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;
	
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if(Input_Msg_Len == 1)
		{
#ifdef _PLATFORM_AS210
			api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#endif		
			if(*iptData == 0xFF)
			{
				for(j=0;j<CAPK_NUMBER;j++)
				{				
					//20140109
					rspCode = api_fls_read(FLSID_CAPK,j*CAPK_LEN,CAPK_LEN,(UCHAR *)&tmpCAPK);
					
					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//Reset
						i = 0;
						Key_Data_Len = 0;
						Hash_Input_Len = 0;
						
						//Accumulate the number of VSDC CA Public Key 
						Num_Keys++;
						
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						memmove(&Key_Data[3],Key_Data,i);
						Key_Data[0] = tmpCAPK.Index;
						Key_Data[1] = (i & 0xFF00) >> 8;
						Key_Data[2] = (i & 0x00FF);
						Key_Data_Len = i + 3;

						//Final
						memcpy(&optData[2+Total_Key_Data_Len],Key_Data,Key_Data_Len);
						Total_Key_Data_Len+=Key_Data_Len;

						//20140102
						if((Total_Key_Data_Len + 2) > VAP_VISA_P_MAX_BUF_SIZE)
						{
							Total_Key_Data_Len -= Key_Data_Len;
							Num_Keys -= 1;
							break;
						}
					}
					else
					{
						if(rspCode != apiOK)
						{
							flgReadFlash = TRUE;
							break;
						}
					}			
				}			

				if(flgReadFlash)						//if read flash error, we delete all data and set the RC code to Failure
				{
					optData[0]=VAP_RIF_RC_FAILURE;
					msgLength = 0;
				}
				else
				{
					//20140102
					optData[0]=VAP_RIF_RC_SUCCESS;
				
					if(VAP_VISA_P_ENCRYPTION)
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
						
						if(msgLength < 2048)
						{
							memcpy(OriData,&optData[1],msgLength);
							OriLen = msgLength;
							
							api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
							msgLength = EryptLen;
						}												
					}
					else
					{
						optData[1] = Num_Keys;
						msgLength = Total_Key_Data_Len+1;	// 1 for num keys
					}
				}			
			}
			else
			{	
				rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_VISA_TEST,*iptData,&glv_CAPK_Addr);

				if(rspCode == SUCCESS)
				{
					rspCode = api_fls_read(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

					if((rspCode == apiOK) && (!memcmp(tmpCAPK.RID,AID,5)))
					{
						//First, We pack Key Data 
						//Key CA Hash Algorithm Indicator
						Key_Data[i++] = tmpCAPK.hshIndicator;
						//Key CA Public Key Algorithm Indicator
						Key_Data[i++] = tmpCAPK.keyIndicator;
						//Key Modulus Length
						Key_Data[i++] = tmpCAPK.Length;

						//Key Modulus
						memcpy(&Key_Data[i],tmpCAPK.Modulus,tmpCAPK.Length);
						i+=tmpCAPK.Length;
						
						if(tmpCAPK.Exponent[0] == 0x00)
						{
							//Key Exponent Length
							Key_Data[i++] = 0x01;
							//Key Exponent
							Key_Data[i++] = 0x03;	
						}
						else
						{
							//Key Exponent Length
							Key_Data[i++] = 0x03;
							//Key Exponent
							Key_Data[i++] = 0x01;	
							Key_Data[i++] = 0x00;	
							Key_Data[i++] = 0x01;	
						}

						//Second, Calculate the Key Hash Result
						memcpy(Hash_Input_Data,tmpCAPK.RID,5);
						Hash_Input_Len += 5;

						Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

						memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
						Hash_Input_Len += tmpCAPK.Length;

						if(tmpCAPK.Exponent[0] == 0x00)
						{
							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Exponent[2];
						}
						else
						{
							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
							Hash_Input_Len += 3;
						}

						api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);


						//Third, we concatenate the hash result, and get key length
						memcpy(&Key_Data[i],Hash_Result,20);
						i+=20;

						Key_Data_Len = i;
						
						//Reset to 1
						i = 1;

						//Final, Packing key Data
						//Index
						optData[i++] = tmpCAPK.Index;
						//Key Length (2 bytes)
						optData[i++] = (Key_Data_Len & 0xFF00) >> 8;
						optData[i++] = (Key_Data_Len & 0x00FF);
						//Key Data
						memcpy(&optData[i],Key_Data,Key_Data_Len);
						i+=Key_Data_Len;

						msgLength = i-1;

						//20140102
						if(VAP_VISA_P_ENCRYPTION)
						{				
							if(msgLength < 2048)
							{
								memcpy(OriData,&optData[1],msgLength);
								OriLen = msgLength;
								
								api_pcd_vap_Encrypt_Data(OriLen, OriData, &EryptLen, &optData[1], VISA_ASession_Key);
								msgLength = EryptLen;
							}												
						}
						
						optData[0]=VAP_RIF_RC_SUCCESS;
					}
					else
					{
						optData[0]=VAP_RIF_RC_FAILURE;
					}

				}
				else
				{
					optData[0]=VAP_RIF_RC_INVALID_VISA_CA_KEY;	//20140103 change rc_code
				}
			}
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}				
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);

}

void api_pcd_vap_adm_SetVisaTestAIDPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	flgError=0;
	UINT	msgLength=0;
	UCHAR 	rspCode = 0;
	CAPK	tmpCAPK;
	UCHAR 	AID[5] = {0xA0,0x00,0x00,0x99,0x99};
	UCHAR   exp_1[3] = {0x00,0x00,0x03};
	UCHAR   exp_3[3] = {0x01,0x00,0x01};

	UCHAR   Hash_Result[20]={0};
	UCHAR	Hash_Input_Data[300]={0};
	UINT	Hash_Input_Len = 0;
	UCHAR 	hasherr = 0;

	UINT	lenOfKeyData = 0;
	UCHAR	lenOfKeyModule = 0;
	UCHAR 	lenError = 0;

	UCHAR	flgCommand = FALSE;

	UCHAR	IndexErr = TRUE;
	
	//UCHAR	i = 0;

	UINT 	Input_Msg_Len = 0;
	Input_Msg_Len = (iptMsgLength[0]*256) + iptMsgLength[1];

	if (api_pcd_vap_flgAdmMode == TRUE)
	{	
		if(iptData[0] == 0x00)		//Add CAPK
		{		
			//20140128, in production, we wont check CAPK Index
			//check index First
			/*
			for(i=0;i<VAP_VISATest_CAPK_Table[0];i++)
			{
				if(*(iptData+1) == VAP_VISATest_CAPK_Table[i+1])
				{
					IndexErr = FALSE;
					break;
				}				
			}*/
			//if key index showed in VISA test CAPK Index, we took some operation to it, else it return VAP_RIF_RC_INVALID_VISA_CA_KEY
			if(IndexErr == FALSE)
			{
				Input_Msg_Len -= 1;		//minus "key add" or "key del" command
		
				rspCode = API_FIND_ADD_CAPK_Addr(VAP_CAPK_ID_VISA_TEST,*(iptData+1),&glv_CAPK_Addr);
				if(rspCode == SUCCESS)
				{		
					memcpy(tmpCAPK.RID,AID,5);

					Input_Msg_Len -= 1;		//minus Key Index
					tmpCAPK.Index = *(iptData+1);

					Input_Msg_Len -= 2;		//minus "Length of Key data"
					lenOfKeyData = (*(iptData+2))*256 + (*(iptData+3));

					if(lenOfKeyData != Input_Msg_Len)
						lenError = TRUE;

					if(!lenError)
					{
						lenOfKeyData -= 1;	//minus "Key CA Hash Algorithm Indicator"
						tmpCAPK.hshIndicator = *(iptData+4);

						lenOfKeyData -= 1;	//minus "Key CAPK Algorithm Indicator"
						tmpCAPK.keyIndicator = *(iptData+5);

						lenOfKeyData -= 1;	//minus "length of Key Modulus Length"
						tmpCAPK.Length = *(iptData+6);
						lenOfKeyModule = *(iptData+6);
						
						lenOfKeyData -= lenOfKeyModule;	//minus "Modulus Length"
						memcpy(tmpCAPK.Modulus,(iptData+7),tmpCAPK.Length);

						if(*(iptData+7+tmpCAPK.Length) == 0x01)
						{
							lenOfKeyData -= 1;	//minus "key Exponent Length"
							lenOfKeyData -= 1;	//minus "key Exponent"
							memcpy(tmpCAPK.Exponent,exp_1,3);

							lenOfKeyData -= 20;	//minus "Key Hash Result"
							memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+1),20);
						}
						else
						{
							lenOfKeyData -= 1;	//minus "key Exponent Length"
							lenOfKeyData -= 3;	//minus "key Exponent"
							memcpy(tmpCAPK.Exponent,exp_3,3);

							lenOfKeyData -= 20;	//minus "Key Hash Result"
							memcpy(tmpCAPK.hshValue,(iptData+7+(tmpCAPK.Length)+1+3),20);
						}

						if(lenOfKeyData != 0)
							lenError = TRUE;

						if(!lenError)
						{
							//Second, Calculate the Key Hash Result
							memcpy(Hash_Input_Data,tmpCAPK.RID,5);
							Hash_Input_Len += 5;

							Hash_Input_Data[Hash_Input_Len++] = tmpCAPK.Index;

							memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Modulus,tmpCAPK.Length);
							Hash_Input_Len += tmpCAPK.Length;

							if(tmpCAPK.Exponent[0] == 0x00)
							{
								Hash_Input_Data[Hash_Input_Len++] = 0x03;
							}
							else
							{
								memcpy(&Hash_Input_Data[Hash_Input_Len],tmpCAPK.Exponent,3);
								Hash_Input_Len += 3;
							}

							api_sys_SHA1(Hash_Input_Len,Hash_Input_Data,Hash_Result);

							if(memcmp(tmpCAPK.hshValue,Hash_Result,20))
								hasherr = TRUE;

							if(!hasherr)
							{
#ifdef _PLATFORM_AS210
								rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
								rspCode = apiOK;
#endif
								if(rspCode == apiOK)
								{
									rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

									if(rspCode == apiFailed)
										flgError = TRUE;
								}
								else
									flgError = TRUE;
							}
							else
								flgError = TRUE;
						}
						else
							flgError = TRUE;			
					}		
					else
						flgError = TRUE;
				}
				else
					flgError = TRUE;
			}
			else
				flgError = TRUE;
		}
		else if(iptData[0] == 0x01)						//Del CAPK
		{		
			//20140128, in production, we wont check CAPK Index
			//check index First
			/*
			for(i=0;i<VAP_VISATest_CAPK_Table[0];i++)
			{
				if(*(iptData+1) == VAP_VISATest_CAPK_Table[i+1])
				{
					IndexErr = FALSE;
					break;
				}				
			}*/
			//if key index showed in VISA CAPK Index, we took some operation to it, else it return VAP_RIF_RC_INVALID_VISA_CA_KEY
			if(IndexErr == FALSE)
			{
				rspCode = API_FIND_CAPK_Addr(VAP_CAPK_ID_VISA_TEST,*(iptData+1),&glv_CAPK_Addr);

				if(rspCode == SUCCESS)
				{
				
					memset((UCHAR*)&tmpCAPK,0xFF,CAPK_LEN);
#ifdef _PLATFORM_AS210
					rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
					rspCode = apiOK;
#endif
					if(rspCode == apiOK)
					{
						rspCode = api_fls_write(FLSID_CAPK,glv_CAPK_Addr,CAPK_LEN,(UCHAR *)&tmpCAPK);

						if(rspCode == apiFailed)
							flgError = TRUE;
					}
					else
						flgError = TRUE;
				}
				else
					flgError = TRUE;	
			}
			else
				flgError = TRUE;
		}
		else
		{	
			flgError = TRUE;
			flgCommand = TRUE;
		}

		if (flgError == TRUE)
		{
			if(lenError)
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			else if(hasherr)
				optData[0]=VAP_RIF_RC_INVALID_VISA_CA_KEY;
			else if(flgCommand)
				optData[0]=VAP_RIF_RC_INVALID_COMMAND;
			else if(IndexErr)
				optData[0]=VAP_RIF_RC_INVALID_VISA_CA_KEY;
			else
				optData[0]=VAP_RIF_RC_FAILURE;
		}
		else
		{
			optData[0]=VAP_RIF_RC_SUCCESS;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
	
	api_pcd_vap_Set_MessageLength((msgLength+1), optMsgLength);

}

void api_pcd_vap_adm_GetCVMCapability(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	CVMList=0;
	UINT	msgLength=0,j=0;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		j=api_pcd_vap_Get_MessageLength(iptMsgLength);

		CVMList=iptData[0];

		if((CVMList == 0xFF) && (j==1))
		{
			msgLength++;
			
			optData[0] = VAP_RIF_RC_SUCCESS;

			msgLength+=3;

			//20140102 Reader do not perform CVM by itself, so return Number of CVM list = 0x00, supported CVM ID = 0x00, state = 0x00
			optData[1] = 0x00;
			optData[2] = glv_vap_CVM[0].ID;
			optData[3] = glv_vap_CVM[0].State;

			if(VAP_VISA_P_ENCRYPTION)
			{
				memcpy(OriData,&optData[1],msgLength-1);	//20140103 minus RC_Code
				OriLen = msgLength-1;						//20140103 minus RC_Code
				api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
				msgLength = EryptLen+1;						//20140103 adds RC_Code
			}
		}
		else
		{
			msgLength=1;
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}
	}
	else
	{
		msgLength=1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);

}

void api_pcd_vap_adm_SetCVMCapability(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	numCVM=0;
	UCHAR	numCurrent=0;
	UCHAR	flgError=TRUE;
	UCHAR	*ptrIptData=NULLPTR;
	UINT	msgLength=0,i=0;
	
#ifndef _PLATFORM_AS350
	UCHAR rspCode = 0xFF;
	UCHAR ToFlashCVMData[8] = {0};
	UCHAR Index = 0;
#endif

	//20140304 add
	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;
	//20140304 add end
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);

		numCVM=iptData[0];
		ptrIptData=&iptData[1];

		do{
			for(i=0;i<VAP_CVM_NUMBER;i++)
			{
				if(glv_vap_CVM[i].ID == *ptrIptData)
				{
					if((*ptrIptData == 0x10) || ( *ptrIptData == 0x11))
					{
						flgError = TRUE;	//not activate "Online PIN" and "Signature"
						break;
					}
					else
					{
						flgError = FALSE;
						glv_vap_CVM[i].State = *(ptrIptData+1);
					}
				}
			}
			
			ptrIptData+=2;
			numCurrent++;
		} while (numCurrent < numCVM);

		if (flgError == FALSE)
		{
			/*
			//20140304 removed
			msgLength++;
			optData[0]=VAP_RIF_RC_SUCCESS;
			memcpy(&optData[1], &iptData[0], msgLength);
			//20140304 removed end
			*/
			
			//20140304 changed			
			optData[0]=VAP_RIF_RC_SUCCESS;
			memcpy(&optData[1], &iptData[0], msgLength);
			msgLength++;
			
			if(VAP_VISA_P_ENCRYPTION)
			{
				OriLen = msgLength-1;
				memcpy(OriData, &optData[1],OriLen);
				api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
				msgLength = EryptLen+1; //add rc_code
			}			
			//20140304 changed end

#ifndef _PLATFORM_AS350

#ifdef _PLATFORM_AS210
			rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
			rspCode = apiOK;
#endif
			if(rspCode == apiOK)
			{
				memset(ToFlashCVMData,0xFF,8);

				for(Index = 0; Index<VAP_CVM_NUMBER; Index++)
				{
					ToFlashCVMData[Index] = glv_vap_CVM[Index].ID;
					ToFlashCVMData[Index+1] = glv_vap_CVM[Index].State;
				}				
				api_fls_write(FLSID_PARA,FLS_ADDRESS_CVM,8,ToFlashCVMData);
			}
#endif
		}
		else
		{
			msgLength=1;
			optData[0]=VAP_RIF_RC_INVALID_SCHEME;
		}
	}
	else
	{
		msgLength=1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);

}

void api_pcd_vap_adm_GetAPID(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR i,j,k,index=0;
	UCHAR tag9F5A[19]={0x9F,0x5A};	//APID	tag(2) + len(1) + value(0~16) 
	UCHAR tagDF06[5]={0xDF,0x06};	//Reader Configuration Parameter	tag(2) + len(1) + value(2)
	UCHAR tagDF00[9]={0xDF,0x00};	//Reader Cless Transaction Limit	tag(2) + len(1) + value(6)
	UCHAR tagDF01[9]={0xDF,0x01};	//Reader CVM Required Limit	tag(2) + len(1) + value(6)
	UCHAR tagDF02[9]={0xDF,0x02};	//Reader Cless Floor Limit		tag(2) + len(1) + value(6)		

	//20140103 encrypt data
	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if((iptData[0] == 0xFF) || ((iptData[0] > 0x00) && (iptData[0] <= ETP_PARA_NUMBER_PID))	) //All or APID 1~4
		{
			if((iptMsgLength[0] == 0x00) && (iptMsgLength[1] == 0x01))		//Check input Length
			{
				index = 2;		//optData[0] is success, optData[1] is total num of APID, so we add the tag data to optData[2]

				if(iptData[0] == 0xFF)	//run all
				{
					k = 0;
					j = ETP_PARA_NUMBER_PID;
				}
				else					//run one
				{
					k = iptData[0]-1;
					j = iptData[0];
				}
				
				for( i = k ; i < j ; i++)
				{
					if(glv_parDRLLimitSet[i].AppProgIDLen)
					{
						//9F5A
						tag9F5A[2] = glv_parDRLLimitSet[i].AppProgIDLen;		//assign the length
						memcpy(&tag9F5A[3],glv_parDRLLimitSet[i].AppProgID,glv_parDRLLimitSet[i].AppProgIDLen);	//add data			
						memmove(&optData[index],tag9F5A,tag9F5A[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
						index+=tag9F5A[2]+3;									//shift index

						//DF06
						tagDF06[2]=2;
						memcpy(&tagDF06[3],glv_parDRLLimitSet[i].Combined,2);
						memmove(&optData[index],tagDF06,tagDF06[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
						index+=tagDF06[2]+3;									//shift index

						//DF00 Reader Contactless Txn Limit
						if (glv_parDRLLimitSet_Len[i].rdrtxnLimit_Len == 0)
						{
							tagDF00[2]=0;
							memmove(&optData[index],tagDF00,tagDF00[2]+3);
							index+=tagDF00[2]+3;
						}
						else
						{
							tagDF00[2]=6;
							memcpy(&tagDF00[3],glv_parDRLLimitSet[i].rdrtxnLimit,6);
							memmove(&optData[index],tagDF00,tagDF00[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
							index+=tagDF00[2]+3;									//shift index
						}

						//DF01 Reader Contactless CVM Limit
						if (glv_parDRLLimitSet_Len[i].rdrcvmLimit_Len == 0)
						{
							tagDF01[2]=0;
							memmove(&optData[index],tagDF01,tagDF01[2]+3);
							index+=tagDF01[2]+3;
						}
						else
						{
							tagDF01[2]=6;
							memcpy(&tagDF01[3],glv_parDRLLimitSet[i].rdrcvmLimit,6);
							memmove(&optData[index],tagDF01,tagDF01[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
							index+=tagDF01[2]+3;									//shift index
						}

						//DF02 Reader Contactless Floor Limit
						if (glv_parDRLLimitSet_Len[i].rdrFlrLimit_Len == 0)
						{
							tagDF02[2]=0;
							memmove(&optData[index],tagDF02,tagDF02[2]+3);
							index+=tagDF02[2]+3;
						}
						else
						{
							tagDF02[2]=6;
							memcpy(&tagDF02[3],glv_parDRLLimitSet[i].rdrFlrLimit,6);
							memmove(&optData[index],tagDF02,tagDF02[2]+3);			//the 3 is tag(2) + len(1) , add to optdata					
							index+=tagDF02[2]+3;									//shift index						
						}
					}
					else
					{
						//9F5A
						tag9F5A[2] = 0;		//assign the length		
						memmove(&optData[index],tag9F5A,tag9F5A[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
						index+=tag9F5A[2]+3;									//shift index

						//DF06
						tagDF06[2] = 0;
						memmove(&optData[index],tagDF06,tagDF06[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
						index+=tagDF06[2]+3;									//shift index

						//DF00
						tagDF00[2] = 0;
						memmove(&optData[index],tagDF00,tagDF00[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
						index+=tagDF00[2]+3;									//shift index

						//DF01
						tagDF01[2] = 0;
						memmove(&optData[index],tagDF01,tagDF01[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
						index+=tagDF01[2]+3;									//shift index

						//DF02
						tagDF02[2] = 0;
						memmove(&optData[index],tagDF02,tagDF02[2]+3);			//the 3 is tag(2) + len(1) , add to optdata					
						index+=tagDF02[2]+3;									//shift index			
					}
				}
				
				optData[0]=VAP_RIF_RC_SUCCESS;
				
				if(iptData[0] == 0xFF)
					optData[1] = ETP_PARA_NUMBER_PID;	//Total number of APID from 0x01 ~ 0xFF	
				else
					optData[1] = 1;	

				optMsgLength[0] = 0;
				optMsgLength[1] = index;			

				//20140103
				if(VAP_VISA_P_ENCRYPTION)
				{
					OriLen = index-1;	//minus RC_Code
					memcpy(OriData,&optData[1],OriLen);
					api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);

					index = EryptLen+1;	//adds RC_Code

					optMsgLength[0] = 0;
					optMsgLength[1] = index;					
				}
			}
			/*else if((iptData[0] > 0x00) && (iptData[0] < 0x05))		//APID 1- 4
			{
				index = 2;		//optData[0] is RC_Success, optData[1] is num of APID, so we add the tag data to optData[2]
			
				i = iptData[0]-1;

				if(glv_parDRLLimitSet[i].AppProgIDLen)
				{
					//9F5A
					tag9F5A[2] = glv_parDRLLimitSet[i].AppProgIDLen;		//assign the length
					memcpy(&tag9F5A[3],glv_parDRLLimitSet[i].AppProgID,glv_parDRLLimitSet[i].AppProgIDLen);	//add data			
					memmove(&optData[index],tag9F5A,tag9F5A[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
					index+=tag9F5A[2]+3;									//shift index

					//DF06
					memcpy(&tagDF06[3],glv_parDRLLimitSet[i].Combined,2);
					memmove(&optData[index],tagDF06,tagDF06[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
					index+=tagDF06[2]+3;									//shift index

					//DF00
					memcpy(&tagDF00[3],glv_parDRLLimitSet[i].rdrtxnLimit,6);
					memmove(&optData[index],tagDF00,tagDF00[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
					index+=tagDF00[2]+3;									//shift index

					//DF01
					memcpy(&tagDF01[3],glv_parDRLLimitSet[i].rdrcvmLimit,6);
					memmove(&optData[index],tagDF01,tagDF01[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
					index+=tagDF01[2]+3;									//shift index

					//DF02
					memcpy(&tagDF02[3],glv_parDRLLimitSet[i].rdrFlrLimit,6);
					memmove(&optData[index],tagDF02,tagDF02[2]+3);			//the 3 is tag(2) + len(1) , add to optdata					
					index+=tagDF02[2]+3;									//shift index	
				
					optData[0]=VAP_RIF_RC_SUCCESS;
					optData[1] = 1;		//Total number of APID from 0x01 ~ 0xFF		

					optMsgLength[0] = 0;
					optMsgLength[1] = index - 3;
				}
				else
				{
					//9F5A
					tag9F5A[2]=0;		//assign the length
					memmove(&optData[index],tag9F5A,tag9F5A[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
					index+=tag9F5A[2]+3;									//shift index

					//DF06
					tagDF06[2]=0;
					memmove(&optData[index],tagDF06,tagDF06[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
					index+=tagDF06[2]+3;									//shift index

					//DF00
					tagDF00[2]=0;
					memmove(&optData[index],tagDF00,tagDF00[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
					index+=tagDF00[2]+3;									//shift index

					//DF01
					tagDF01[2]=0;
					memmove(&optData[index],tagDF01,tagDF01[2]+3);			//the 3 is tag(2) + len(1) , add to optdata
					index+=tagDF01[2]+3;									//shift index

					//DF02
					tagDF02[2]=0;
					memmove(&optData[index],tagDF02,tagDF02[2]+3);			//the 3 is tag(2) + len(1) , add to optdata					
					index+=tagDF02[2]+3;									//shift index	
				
					optData[0]=VAP_RIF_RC_SUCCESS;
					optData[1] = 1;		//Total number of APID from 0x01 ~ 0xFF		

					optMsgLength[0] = 0;
					optMsgLength[1] = index - 3;
				}
			}*/
			else					//out of order
			{
				optMsgLength[0] = 0;
				optMsgLength[1] = 1;
				optData[0]=VAP_RIF_RC_FAILURE;
			}

		}
		else	//length error
		{
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
			optData[0]=VAP_RIF_RC_INVALID_DATA;
		}	
	}
	else
	{
		optMsgLength[0] = 0;
		optMsgLength[1] = 1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
}


void api_pcd_vap_adm_SetAPID(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
#ifndef _PLATFORM_AS350
	UCHAR ToFlashAPIDData[166] = {0};
	UCHAR *ToFlashAPIDDataPtr = NULLPTR;
	UCHAR ToFlashIndex = 0;
#endif
	LIMITSET	tmp_parDRLLimitSet[ETP_PARA_NUMBER_PID]={{0,{0},{0},{0},{0},{0},{0}}};
	LMTSET_LEN	tmp_parDRLLimitSet_Len[ETP_PARA_NUMBER_PID]={{0,0,0,0}};
	
	UCHAR *InputDRLPtr = NULLPTR;
	UCHAR tmpLenT = 0, tmpLenL = 0;
	UINT  tmpLenV = 0;
	UCHAR rspCode = 0;
	UCHAR CheckAPIDNum = 0;
	
	UCHAR index=0,Err_flg=0,CheckLen=0;
	UCHAR tag9F5A[2]={0x9F,0x5A};	//APID	tag(2) + len(1) + value(0~16) 
	UCHAR tagDF06[2]={0xDF,0x06};	//Reader Configuration Parameter	tag(2) + len(1) + value(2)
	UCHAR tagDF00[2]={0xDF,0x00};	//Reader Cless Transaction Limit	tag(2) + len(1) + value(6)
	UCHAR tagDF01[2]={0xDF,0x01};	//Reader CVM Required Limit	tag(2) + len(1) + value(6)
	UCHAR tagDF02[2]={0xDF,0x02};	//Reader Cless Floor Limit		tag(2) + len(1) + value(6)			

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if((iptData[0] > 0x00) && (iptData[0] <= ETP_PARA_NUMBER_PID))
		{
			memset(tmp_parDRLLimitSet,0x00,sizeof(tmp_parDRLLimitSet));
			
			CheckLen = iptMsgLength[0]*256 + iptMsgLength[1];

			CheckLen-=1;		//minus first byte, "Number of [Application Program ID] from 0x01 to 0xFF"

			InputDRLPtr = &iptData[1];
			
			for(index=0;index<iptData[0];index++)
			{
				//handle 9F5A
				if(!memcmp(InputDRLPtr,tag9F5A,2))
				{
					rspCode = UT_Get_TLVLength(InputDRLPtr,&tmpLenT,&tmpLenL,&tmpLenV);
					if(rspCode == SUCCESS)
					{
						//Set length
						tmp_parDRLLimitSet[index].AppProgIDLen = (UCHAR)tmpLenV;

						//shift to the Data, Set Data
						InputDRLPtr+=tmpLenT;
						InputDRLPtr+=tmpLenL;
						memcpy(tmp_parDRLLimitSet[index].AppProgID,InputDRLPtr,tmp_parDRLLimitSet[index].AppProgIDLen);

						//shift to next tag
						InputDRLPtr+=tmpLenV;						

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);

						//check APID Number
						CheckAPIDNum+=1;
					}
					else
					{
						Err_flg = 1;
						break;
					}					
				}
				else
				{
					Err_flg = 1;
					break;
				}

				//handle DF06
				if(!memcmp(InputDRLPtr,tagDF06,2))
				{
					rspCode = UT_Get_TLVLength(InputDRLPtr,&tmpLenT,&tmpLenL,&tmpLenV);
					if(rspCode == SUCCESS)
					{
						//Check format
						if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==2))
						{
							//shift to data, Set Data
							InputDRLPtr+=(tmpLenT+tmpLenL);
							memcpy(tmp_parDRLLimitSet[index].Combined,InputDRLPtr,tmpLenV);

							//shift to next tag
							InputDRLPtr+=tmpLenV;

							//check total legth
							CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
						}
						else
						{
							Err_flg = 1;
							break;
						}
					}
					else
					{
						Err_flg = 1;
						break;
					}
				}
				else
				{
					Err_flg = 1;
					break;
				}

				//handle DF00 Reader Contactless Txn Limit
				if(!memcmp(InputDRLPtr,tagDF00,2))
				{
					rspCode = UT_Get_TLVLength(InputDRLPtr,&tmpLenT,&tmpLenL,&tmpLenV);
					if(rspCode == SUCCESS)
					{
						//Check format
						if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==6))
						{
							//shift to data, Set Data
							InputDRLPtr+=(tmpLenT+tmpLenL);
							memcpy(tmp_parDRLLimitSet[index].rdrtxnLimit,InputDRLPtr,tmpLenV);

							//shift to next tag
							InputDRLPtr+=tmpLenV;

							//check total legth
							CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
						}
						else if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==0))	//20140111 absset 
						{
							InputDRLPtr+=(tmpLenT+tmpLenL);
							memset(tmp_parDRLLimitSet[index].rdrtxnLimit,0x00,6);

							//shift to next tag
							InputDRLPtr+=tmpLenV;

							//check total legth
							CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
						}
						else
						{
							Err_flg = 1;
							break;
						}

						tmp_parDRLLimitSet_Len[index].rdrtxnLimit_Len=tmpLenV;
					}
					else
					{
						Err_flg = 1;
						break;
					}
				}
				else
				{
					Err_flg = 1;
					break;
				}

				//DF01 Reader Contactless CVM Limit
				if(!memcmp(InputDRLPtr,tagDF01,2))
				{
					rspCode = UT_Get_TLVLength(InputDRLPtr,&tmpLenT,&tmpLenL,&tmpLenV);
					if(rspCode == SUCCESS)
					{
						//Check format
						if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==6))
						{
							//shift to data, Set Data
							InputDRLPtr+=(tmpLenT+tmpLenL);
							memcpy(tmp_parDRLLimitSet[index].rdrcvmLimit,InputDRLPtr,tmpLenV);

							//shift to next tag
							InputDRLPtr+=tmpLenV;

							//check total legth
							CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
						}
						else if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==0))	//20140111 absset 
						{
							InputDRLPtr+=(tmpLenT+tmpLenL);
							memset(tmp_parDRLLimitSet[index].rdrcvmLimit,0x00,6);

							//shift to next tag
							InputDRLPtr+=tmpLenV;

							//check total legth
							CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
						}
						else
						{
							Err_flg = 1;
							break;
						}

						tmp_parDRLLimitSet_Len[index].rdrcvmLimit_Len=tmpLenV;
					}
					else
					{
						Err_flg = 1;
						break;
					}
				}
				else
				{
					Err_flg = 1;
					break;
				}

				//DF02 Reader Contactless Floor Limit
				if(!memcmp(InputDRLPtr,tagDF02,2))
				{
					rspCode = UT_Get_TLVLength(InputDRLPtr,&tmpLenT,&tmpLenL,&tmpLenV);
					if(rspCode == SUCCESS)
					{
						//Check format
						if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==6))
						{
							//shift to data, Set Data
							InputDRLPtr+=(tmpLenT+tmpLenL);
							memcpy(tmp_parDRLLimitSet[index].rdrFlrLimit,InputDRLPtr,tmpLenV);

							//shift to next tag
							InputDRLPtr+=tmpLenV;

							//check total legth
							CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
						}
						else if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==0))	//20140111 absset 
						{
							InputDRLPtr+=(tmpLenT+tmpLenL);
							memset(tmp_parDRLLimitSet[index].rdrFlrLimit,0x00,6);

							//shift to next tag
							InputDRLPtr+=tmpLenV;

							//check total legth
							CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
						}
						else
						{
							Err_flg = 1;
							break;
						}

						tmp_parDRLLimitSet_Len[index].rdrFlrLimit_Len=tmpLenV;
					}
					else
					{
						Err_flg = 1;
						break;
					}
				}
				else
				{
					Err_flg = 1;
					break;
				}
			}

			//check length & APID number
			if((CheckLen != 0) && (CheckAPIDNum != iptData[0]))
			{
				Err_flg = 1;
			}

			//check error
			if(Err_flg)
			{
				optMsgLength[0] = 0;
				optMsgLength[1] = 1;
				optData[0]=VAP_RIF_RC_FAILURE;
			}
			else
			{	
				memset(glv_parDRLLimitSet,0x00,sizeof(glv_parDRLLimitSet));
				memset(glv_parDRLLimitSet_Len,0x00,sizeof(glv_parDRLLimitSet_Len));
				
				memcpy(glv_parDRLLimitSet,tmp_parDRLLimitSet,sizeof(tmp_parDRLLimitSet));
				memcpy(glv_parDRLLimitSet_Len,tmp_parDRLLimitSet_Len,sizeof(tmp_parDRLLimitSet_Len));
			
				optMsgLength[0] = 0;
				optMsgLength[1] = 1;
				optData[0]=VAP_RIF_RC_SUCCESS;

#ifndef _PLATFORM_AS350

#ifdef _PLATFORM_AS210
				rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
				rspCode = apiOK;
#endif
				if(rspCode == apiOK)
				{
					memset(ToFlashAPIDData,0xFF,sizeof(ToFlashAPIDData));
					ToFlashAPIDData[0] = CheckAPIDNum;
					ToFlashAPIDData[1] = CheckAPIDNum*LIMITSET_LEN;

					ToFlashAPIDDataPtr = &ToFlashAPIDData[2];

					for(ToFlashIndex=0;ToFlashIndex<CheckAPIDNum;ToFlashIndex++)
					{
						memcpy(ToFlashAPIDDataPtr,&glv_parDRLLimitSet[ToFlashIndex],LIMITSET_LEN);
						ToFlashAPIDDataPtr+=LIMITSET_LEN;
					}

					api_fls_write(FLSID_PARA,FLS_ADDRESS_APID,166,ToFlashAPIDData);
				}
#endif
			}
		}
		else	//out of order
		{
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
			optData[0]=VAP_RIF_RC_INVALID_DATA;
		}
	}
	else
	{
		optMsgLength[0] = 0;
		optMsgLength[1] = 1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
}

void api_pcd_vap_adm_GetExceptionFile(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData)
{
	UCHAR 	paddingFF[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

	UINT	iptLen = 0;
	
	UCHAR 	FileNum = 0;
#ifndef _PLATFORM_AS350
	UCHAR	rspCode = 0xFF; 
	UCHAR	InFlashExpFileData[111] = {0};
	UCHAR	*FilePtr = NULLPTR;
	UCHAR 	FileSize = 0;
#else
	UCHAR	EmptySet[10*11] = {0x00}	;
#endif
	UCHAR 	OptLen = 0;
	UCHAR	*OptPtr = NULLPTR;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

	iptLen = iptMsgLength[0]*256+iptMsgLength[1];
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if((iptLen==1) && (iptData[0] == 0xFF))		//Check length and data
		{
#ifndef _PLATFORM_AS350

#ifdef _PLATFORM_AS210
			rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
			rspCode = apiOK;
#endif
			if(rspCode == apiOK)
			{
				rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_ExceptionFile,111,InFlashExpFileData);
				if(rspCode == apiOK)
				{
					if(((InFlashExpFileData[0] == 0xFF) && (InFlashExpFileData[1] == 0xFF)) || 
						((InFlashExpFileData[0] == 0x00) && (InFlashExpFileData[1] == 0x00)))	//empty, return SUCCESS and Length 0
					{
						//this is 20140117 V2 Version, return RC_Success
						//20140109 changed
						/*
						optMsgLength[0] = 0;
						optMsgLength[1] = 2;
						optData[0]=VAP_RIF_RC_SUCCESS;
						optData[1]=0;
						*/
						
						//this is 20140120 V1 Version
						//20140120 V1, when empty, return RC_Failure
						optMsgLength[0] = 0;
						optMsgLength[1] = 1;
						optData[0]=VAP_RIF_RC_FAILURE; 	
						
						//Both of solutions (20140117 V2, 20140120 V1)are acceptable, because there is no test case for empty situation
					}
					else
					{
						FileNum = InFlashExpFileData[0];
						if(FileNum < 11)	//File Number should be 1~10
						{							
							FilePtr = &InFlashExpFileData[1];
							OptPtr = &optData[1];
							
							while(FileNum--)
							{
								/* 20140117 V2, Removed
								FileSize = *FilePtr;	//Get File Size,	L
								memcpy(OptPtr,FilePtr,FileSize+1);	//Copy L(1 byte) + V(L Bytes)
								FilePtr+=(FileSize+1);	//Shift the pointer to the file
								OptPtr+=(FileSize+1);
								
								OptLen = 1+FileSize;
								*/ //20140117 V2, Removed end
								
								//20140117 Changed
								FileSize = *FilePtr;	//Get File Size,	L
								FilePtr += 1;			//shift to data
								//copy Length and padding to optdata
								memcpy(OptPtr,FilePtr,FileSize);
								OptPtr+=FileSize;
								memcpy(OptPtr,paddingFF,6);		
								OptPtr+=6;
								
								FilePtr+=FileSize;	//Shift the pointer to the next file	

								OptLen+=FileSize+6;
								//20140117 Changed end
							}
	
							optData[0] = VAP_RIF_RC_SUCCESS;	
							
							if(VAP_VISA_P_ENCRYPTION)
							{
								OriLen = OptLen;
								memcpy(OriData,&optData[1],OriLen);
								api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
								OptLen = EryptLen;
							}

							optMsgLength[0] = 0;
							optMsgLength[1] = OptLen + 1;	//data size and RC code
						}
						else
						{
							optMsgLength[0] = 0;
							optMsgLength[1] = 1;
							optData[0]=VAP_RIF_RC_FAILURE;
						}						
					}
				}		
			}
#else
			memset(EmptySet,0x00,sizeof(EmptySet));
			
			if(!memcmp(Exception_File,EmptySet,110))	//Empty
			{
				optMsgLength[0] = 0;
				optMsgLength[1] = 1;
				optData[0]=VAP_RIF_RC_FAILURE; 
			}
			else
			{
				OptPtr = &optData[1];
				while(memcmp(&Exception_File[FileNum][0],EmptySet,10))
				{
					memcpy(OptPtr,&Exception_File[FileNum][0],10);
					OptPtr+=10;
					memcpy(OptPtr,paddingFF,6);	
					OptPtr+=6;
					OptLen+=16;
					FileNum++;
				}		
				
				optData[0] = VAP_RIF_RC_SUCCESS;
				
				if(VAP_VISA_P_ENCRYPTION)
				{
					OriLen = OptLen;
					memcpy(OriData,&optData[1],OriLen);
					api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
					OptLen = EryptLen;
				}

				optMsgLength[0] = 0;
				optMsgLength[1] = OptLen + 1;	//data size and RC code	
			}	

#endif		
		}
		else
		{
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
			optData[0]=VAP_RIF_RC_INVALID_DATA;
		}
	}
	else
	{
		optMsgLength[0] = 0;
		optMsgLength[1] = 1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
}

void api_pcd_vap_adm_SetExceptionFile(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData)
{
	UCHAR emptySet[10] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	UCHAR tmpExFile[10][11] = {{0}};
	
	UCHAR FileNum = 0,NewFileSize = 0,ExFileIndex = 0;
	UCHAR Err_flg = TRUE;
	UINT iptLen = 0;

#ifndef _PLATFORM_AS350
	UCHAR rspCode = 0xFF;
	UCHAR ToFlashExpFileData[111] = {0};
	UCHAR *ToFlashExpFileDataPtr = NULLPTR;
#endif

//20140117 change to this concept
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		iptLen = iptMsgLength[0]*256 + iptMsgLength[1];

		memcpy(tmpExFile,Exception_File,sizeof(Exception_File));

		if(iptLen == 17)
		{
			if(iptData[0] == 0x00)	//add
			{
//Find Space 
				for(ExFileIndex=0;ExFileIndex<10;ExFileIndex++)
				{
					if(!memcmp(tmpExFile[ExFileIndex],emptySet,10))
					{
						Err_flg = FALSE;
						break;
					}
					else
						Err_flg = TRUE;	//No Free space
				}
//excute input data
				if(Err_flg == FALSE)
				{					
					for(NewFileSize=0;NewFileSize<16;NewFileSize++)
					{
						if(iptData[NewFileSize+1] == 0xFF)
							break;
					}
//Store to temp ExFile
					if((NewFileSize<11) && (NewFileSize > 0))
					{
						memcpy(&tmpExFile[ExFileIndex],&iptData[1],10);

						memset(Exception_File,0x00,sizeof(Exception_File));

						memcpy(Exception_File,tmpExFile,sizeof(Exception_File));
						
						Err_flg = FALSE;
					}
					else
						Err_flg = TRUE;
				}
//count the each file size and write to Flash
				if(Err_flg == FALSE)
				{
#ifndef _PLATFORM_AS350			
					memset(ToFlashExpFileData,0xFF,111);

					//set the pointer
					ToFlashExpFileDataPtr = &ToFlashExpFileData[1];
					
					for(ExFileIndex=0;ExFileIndex<10;ExFileIndex++)
					{
						if(memcmp(Exception_File[ExFileIndex],emptySet,10))
						{	
							FileNum++;
							*ToFlashExpFileDataPtr = 10;			//Fixed to 10
							ToFlashExpFileDataPtr+=1;				//shift to data
							memcpy(ToFlashExpFileDataPtr,Exception_File[ExFileIndex],10);
							ToFlashExpFileDataPtr+=10;
						}
						else
							break;
					}
					
					ToFlashExpFileData[0] = FileNum;

#ifdef _PLATFORM_AS210
					rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
					rspCode = apiOK;
#endif
					if(rspCode == apiOK)
					{
						//write to flash
						api_fls_write(FLSID_PARA,FLS_ADDRESS_ExceptionFile,111,ToFlashExpFileData);

						optMsgLength[0] = 0;
						optMsgLength[1] = 1;
						optData[0]=VAP_RIF_RC_SUCCESS;
					}
#else		
					optMsgLength[0] = 0;
					optMsgLength[1] = 1;
					optData[0]=VAP_RIF_RC_SUCCESS;								
#endif					
				}
				else
				{
					optMsgLength[0] = 0;
					optMsgLength[1] = 1;
					optData[0]=VAP_RIF_RC_FAILURE;
				}
			}
			else if(iptData[0] == 0x01)	//delete
			{
				for(NewFileSize=0;NewFileSize<16;NewFileSize++)
				{
					if(iptData[NewFileSize+1] == 0xFF)
						break;
				}

				for(ExFileIndex=0;ExFileIndex<10;ExFileIndex++)
				{
					//if(!memcmp(&tmpExFile[ExFileIndex],&iptData[1],NewFileSize))	//20140304 changed, NewFileSize => 10
					if(!memcmp(&tmpExFile[ExFileIndex],&iptData[1],10))
					{
						memset(&tmpExFile[ExFileIndex],0x00,10);
						Err_flg = FALSE;
					}
				}

				if(Err_flg)
				{
					optMsgLength[0] = 0;
					optMsgLength[1] = 1;
					optData[0]=VAP_RIF_RC_FAILURE;
				}
				else
				{
					optMsgLength[0] = 0;
					optMsgLength[1] = 1;
					optData[0]=VAP_RIF_RC_SUCCESS;

					//re-sort the files
					memset(Exception_File,0x00,sizeof(Exception_File));

					for(ExFileIndex=0;ExFileIndex<10;ExFileIndex++)
					{
						if(memcmp(tmpExFile[ExFileIndex],emptySet,10))
						{
							memcpy(Exception_File[FileNum],tmpExFile[ExFileIndex],10);
							FileNum++;	//Count File Number
						}
					}					

#ifndef _PLATFORM_AS350

#ifdef _PLATFORM_AS210
					rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
					rspCode = apiOK;
#endif
					if(rspCode == apiOK)
					{
						memset(ToFlashExpFileData,0xFF,111);
						//set the pointer
						ToFlashExpFileDataPtr = ToFlashExpFileData;

						//handle the data, total_file_num(1 byte) + (L(1 byte) + V(0~10 bytes))*total_num 
						*ToFlashExpFileDataPtr = FileNum;	//Total Exception File Num
						ToFlashExpFileDataPtr++;	//shift pointer

						//copy the input data
						for(ExFileIndex=0;ExFileIndex<FileNum;ExFileIndex++)
						{
							*ToFlashExpFileDataPtr=10;
							ToFlashExpFileDataPtr+=1;
							memcpy(ToFlashExpFileDataPtr,Exception_File[ExFileIndex],10);				
							ToFlashExpFileDataPtr+=10;
						}

						//write to flash
						api_fls_write(FLSID_PARA,FLS_ADDRESS_ExceptionFile,111,ToFlashExpFileData);
					}
#endif					
				}				
			}
			else	//wrong commmand
			{
				optMsgLength[0] = 0;
				optMsgLength[1] = 1;
				optData[0]=VAP_RIF_RC_INVALID_COMMAND;
			}
		}
		else
		{
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
			optData[0]=VAP_RIF_RC_INVALID_DATA;
		}			
	}
	else
	{
		optMsgLength[0] = 0;
		optMsgLength[1] = 1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
	//20140117 change to this concept end

//20140117 V2, changed 
/*	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		iptLen = iptMsgLength[0]*256 + iptMsgLength[1];

		iptFileLen = iptLen;
		
		for(i = 0 ; i < 10 ; i++)
		{
			//20140106
			if(iptData[index] > 11)	//Check Len
			{
				Err_flg = 1;
				break;
			}

			iptLen -= iptData[index]+1;			//the 1 means length of length, EX : L-V => 0x01,0x00

			if(iptData[index+1] == 0x00)
				memset(&Exception_File[i][0],0x00,10);
			else
				memcpy(&Exception_File[i][0],&iptData[index+1],iptData[index]);
			
			index+=iptData[index]+1;			

			if(iptLen == 0)
				break;
		}

		if(Err_flg)
		{
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
			optData[0]=VAP_RIF_RC_FAILURE;
		}
		else
		{
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
			optData[0]=VAP_RIF_RC_SUCCESS;

			//20140106 write to flash
			rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
			if(rspCode == apiOK)
			{
				memset(ToFlashExpFileData,0xFF,111);
				//set the pointer
				ToFlashExpFileDataPtr = ToFlashExpFileData;

				//handle the data, total_file_num(1 byte) + (L(1 byte) + V(0~10 bytes))*total_num 
				*ToFlashExpFileDataPtr = i+1;	//Total Exception File Num, "i+1" because i count from 0
				ToFlashExpFileDataPtr++;	//shift pointer

				//copy the input data
				memcpy(ToFlashExpFileDataPtr,iptData,iptFileLen);				

				//write to flash
				api_fls_write(FLSID_PARA,FLS_ADDRESS_ExceptionFile,111,ToFlashExpFileData);
			}
		}
			
	}
	else
	{
		optMsgLength[0] = 0;
		optMsgLength[1] = 1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
*/
//20140117 V2, Changed end
}

void api_pcd_vap_adm_GetDRLEnable(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT iptLen = 0;

	UINT OptLen = 0;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

	iptLen = iptMsgLength[0]*256+iptMsgLength[1];

	iptData = iptData;

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if(iptLen == 0x00)
		{
			optData[1]=glv_parFlgDRL;

			OptLen = 1;

			if(VAP_VISA_P_ENCRYPTION)
			{
				OriLen = OptLen;
				memcpy(OriData,&optData[1],OriLen);
				api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
				OptLen = EryptLen;

				optMsgLength[1] = OptLen;
			}
			optData[0]=VAP_RIF_RC_SUCCESS;

			optMsgLength[0] = 0;
			optMsgLength[1] = OptLen+1;				
		}
		else
		{
			optData[0]=VAP_RIF_RC_FAILURE;
			
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
		
		optMsgLength[0] = 0;
		optMsgLength[1] = 1;
	}
}

void api_pcd_vap_adm_SetDRLEnable(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT iptLen = 0;
#ifndef _PLATFORM_AS350
	UCHAR rspCode = 0xFF;
#endif
	iptLen = iptMsgLength[0]*256+iptMsgLength[1];

	iptData = iptData;

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if((iptLen = 0x01) && ((iptData[0]==0x00) || (iptData[0]==0x01)))
		{
			glv_parFlgDRL = iptData[0];

			optData[0]=VAP_RIF_RC_SUCCESS;

			optMsgLength[0] = 0;
			optMsgLength[1] = 1; 	

#ifndef _PLATFORM_AS350

#ifdef _PLATFORM_AS210
			rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
			rspCode = apiOK;
#endif
			if(rspCode == apiOK)
			{
				api_fls_write(FLSID_PARA,FLS_ADDRESS_DRLEnable,1,&glv_parFlgDRL);
			}
#endif			
		}
		else
		{
			optData[0]=VAP_RIF_RC_FAILURE;
			
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
		
		optMsgLength[0] = 0;
		optMsgLength[1] = 1;
	}
}

void api_pcd_vap_adm_GetCashRRP(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT iptLen = 0;
	UCHAR *OutPutDataPtr = NULLPTR;
	UINT optLen = 0;

	UCHAR tagDF06[3] = {0xDF,0x06,0x02};	
	UCHAR tagDF00[3] = {0xDF,0x00,0x06};
	UCHAR tagDF01[3] = {0xDF,0x01,0x06};
	UCHAR tagDF02[3] = {0xDF,0x02,0x06};

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		iptLen = iptMsgLength[0]*256+iptMsgLength[1];

		if((iptLen == 0x01) && (iptData[0] == 0xFF))
		{
			OutPutDataPtr = &optData[1];

			//handle DF06
			memcpy(OutPutDataPtr,tagDF06,3);
			OutPutDataPtr+=3;
			memcpy(OutPutDataPtr,glv_parDF06_VISA[VISA_Cash_Mode],2);
			OutPutDataPtr+=2;

			optLen+=5;
			
			//handle DF00
			memcpy(OutPutDataPtr,tagDF00,3);
			OutPutDataPtr+=3;
			memcpy(OutPutDataPtr,glv_parDF00_VISA[VISA_Cash_Mode],6);
			OutPutDataPtr+=6;
			
			optLen+=9;

			//handle DF01
			memcpy(OutPutDataPtr,tagDF01,3);
			OutPutDataPtr+=3;
			memcpy(OutPutDataPtr,glv_parDF01_VISA[VISA_Cash_Mode],6);
			OutPutDataPtr+=6;
			
			optLen+=9;
			
			//handle DF02
			memcpy(OutPutDataPtr,tagDF02,3);
			OutPutDataPtr+=3;
			memcpy(OutPutDataPtr,glv_parDF02_VISA[VISA_Cash_Mode],6);
			OutPutDataPtr+=6;
			
			optLen+=9;

			//Encrypt data
			if(VAP_VISA_P_ENCRYPTION)
			{
				OriLen = optLen;
				memcpy(OriData,&optData[1],OriLen);
				api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
				optLen = EryptLen;
			}
		
			optData[0]=VAP_RIF_RC_SUCCESS;
				
			optMsgLength[0] = 0;
			optMsgLength[1] = (UCHAR)(optLen+1); //add RC_Code

		}
		else
		{
			optData[0]=VAP_RIF_RC_FAILURE;
				
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
			
		optMsgLength[0] = 0;
		optMsgLength[1] = 1;
	}
}


void api_pcd_vap_adm_SetCashRRP(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT iptLen = 0;

	UCHAR rspCode = 0xFF;

	UCHAR *InputDataPtr = NULLPTR;

	UCHAR tagDF06[2] = {0xDF,0x06}; 
	UCHAR tagDF00[2] = {0xDF,0x00};
	UCHAR tagDF01[2] = {0xDF,0x01};
	UCHAR tagDF02[2] = {0xDF,0x02};

	UCHAR tmpDF06[2] = {0};
	UCHAR tmpDF00[6] = {0};
	UCHAR tmpDF01[6] = {0};
	UCHAR tmpDF02[6] = {0};

	UCHAR tmpLenT=0, tmpLenL=0, CheckLen=0;
	UINT  tmpLenV=0;

	UCHAR ErrFlag = FALSE;
#ifndef _PLATFORM_AS350
	UCHAR ToFlashCashData[20] = {0};
#endif

	iptLen = iptMsgLength[0]*256+iptMsgLength[1];
	CheckLen = iptLen;

	InputDataPtr = iptData;

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if(iptLen <= 32) //DF0602+V, DF0006+V, DF0106+V,DF0206+V //20140717 "==" change to "<="
		{
			//handle DF06
			if(!memcmp(InputDataPtr,tagDF06,2))
			{
				rspCode = UT_Get_TLVLength(InputDataPtr,&tmpLenT,&tmpLenL,&tmpLenV);
				if(rspCode == SUCCESS)
				{
					//Check format
					if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==2))
					{
						//shift to data, Set Data
						InputDataPtr+=(tmpLenT+tmpLenL);
						memcpy(tmpDF06,InputDataPtr,tmpLenV);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else
						ErrFlag = 1;
				}
				else
					ErrFlag = 1;
			}
			else
				ErrFlag = 1;
			
			//handle DF00 Reader Contactless Txn Limit
			if(!memcmp(InputDataPtr,tagDF00,2))
			{
				rspCode = UT_Get_TLVLength(InputDataPtr,&tmpLenT,&tmpLenL,&tmpLenV);
				if(rspCode == SUCCESS)
				{
					//Check format
					if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==6))
					{
						//shift to data, Set Data
						InputDataPtr+=(tmpLenT+tmpLenL);
						memcpy(tmpDF00,InputDataPtr,tmpLenV);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==0))	//20140111 absset 
					{
						InputDataPtr+=(tmpLenT+tmpLenL);
						memset(tmpDF00,0x00,6);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else
						ErrFlag = 1;					
				}
				else
					ErrFlag = 1;
			}
			else
				ErrFlag = 1;
			
			//DF01 Reader Contactless CVM Limit
			if(!memcmp(InputDataPtr,tagDF01,2))
			{
				rspCode = UT_Get_TLVLength(InputDataPtr,&tmpLenT,&tmpLenL,&tmpLenV);
				if(rspCode == SUCCESS)
				{
					//Check format
					if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==6))
					{
						//shift to data, Set Data
						InputDataPtr+=(tmpLenT+tmpLenL);
						memcpy(tmpDF01,InputDataPtr,tmpLenV);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==0))	//20140111 absset 
					{
						InputDataPtr+=(tmpLenT+tmpLenL);
						memset(tmpDF01,0x00,6);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else
						ErrFlag = 1;						
				}
				else
					ErrFlag = 1;
			}
			else
				ErrFlag = 1;

			//DF02 Reader Contactless Floor Limit
			if(!memcmp(InputDataPtr,tagDF02,2))
			{
				rspCode = UT_Get_TLVLength(InputDataPtr,&tmpLenT,&tmpLenL,&tmpLenV);
				if(rspCode == SUCCESS)
				{
					//Check format
					if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==6))
					{
						//shift to data, Set Data
						InputDataPtr+=(tmpLenT+tmpLenL);
						memcpy(tmpDF02,InputDataPtr,tmpLenV);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==0))	//20140111 absset 
					{
						InputDataPtr+=(tmpLenT+tmpLenL);
						memset(tmpDF02,0x00,6);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else
						ErrFlag = 1;					
				}
				else
					ErrFlag = 1;
			}
			else
				ErrFlag = 1;

			//check format and write to flash
			if((ErrFlag == FALSE) && (CheckLen == 0))
			{		
				optData[0]=VAP_RIF_RC_SUCCESS;
			
				optMsgLength[0] = 0;
				optMsgLength[1] = 1;

				memcpy(glv_parDF06_VISA[VISA_Cash_Mode],tmpDF06,2);
				memcpy(glv_parDF00_VISA[VISA_Cash_Mode],tmpDF00,6);
				memcpy(glv_parDF01_VISA[VISA_Cash_Mode],tmpDF01,6);
				memcpy(glv_parDF02_VISA[VISA_Cash_Mode],tmpDF02,6);

#ifndef _PLATFORM_AS350

#ifdef _PLATFORM_AS210
				rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
				rspCode = apiOK;
#endif
				if(rspCode == apiOK)
				{
					memset(ToFlashCashData,0xFF,20);
					memcpy(ToFlashCashData,glv_parDF06_VISA[VISA_Cash_Mode],2);	
					memcpy(&ToFlashCashData[2],glv_parDF00_VISA[VISA_Cash_Mode],6);
					memcpy(&ToFlashCashData[8],glv_parDF01_VISA[VISA_Cash_Mode],6);
					memcpy(&ToFlashCashData[14],glv_parDF02_VISA[VISA_Cash_Mode],6);

					//write to flash
					api_fls_write(FLSID_PARA,FLS_ADDRESS_Cash_Data,20,ToFlashCashData);	//20140701 change the address
				}
#endif
			}
			else
			{
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			
				optMsgLength[0] = 0;
				optMsgLength[1] = 1;
			}			
		}
		else
		{
			optData[0]=VAP_RIF_RC_FAILURE;
			
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
		
		optMsgLength[0] = 0;
		optMsgLength[1] = 1;
	}

}


void api_pcd_vap_adm_GetCashBackRRP(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT iptLen = 0;
	UCHAR *OutPutDataPtr = NULLPTR;
	UINT optLen = 0;

	UCHAR tagDF06[3] = {0xDF,0x06,0x02};	
	UCHAR tagDF00[3] = {0xDF,0x00,0x06};
	UCHAR tagDF01[3] = {0xDF,0x01,0x06};
	UCHAR tagDF02[3] = {0xDF,0x02,0x06};

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		iptLen = iptMsgLength[0]*256+iptMsgLength[1];

		if((iptLen == 0x01) && (iptData[0] == 0xFF))
		{
			OutPutDataPtr = &optData[1];

			//handle DF06
			memcpy(OutPutDataPtr,tagDF06,3);
			OutPutDataPtr+=3;
			memcpy(OutPutDataPtr,glv_parDF06_VISA[VISA_Cashback_Mode],2);
			OutPutDataPtr+=2;

			optLen+=5;
			
			//handle DF00
			memcpy(OutPutDataPtr,tagDF00,3);
			OutPutDataPtr+=3;
			memcpy(OutPutDataPtr,glv_parDF00_VISA[VISA_Cashback_Mode],6);
			OutPutDataPtr+=6;
			
			optLen+=9;

			//handle DF01
			memcpy(OutPutDataPtr,tagDF01,3);
			OutPutDataPtr+=3;
			memcpy(OutPutDataPtr,glv_parDF01_VISA[VISA_Cashback_Mode],6);
			OutPutDataPtr+=6;
			
			optLen+=9;
			
			//handle DF02
			memcpy(OutPutDataPtr,tagDF02,3);
			OutPutDataPtr+=3;
			memcpy(OutPutDataPtr,glv_parDF02_VISA[VISA_Cashback_Mode],6);
			OutPutDataPtr+=6;
			
			optLen+=9;

			//Encrypt data
			if(VAP_VISA_P_ENCRYPTION)
			{
				OriLen = optLen;
				memcpy(OriData,&optData[1],OriLen);
				api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
				optLen = EryptLen;
			}
		
			optData[0]=VAP_RIF_RC_SUCCESS;
				
			optMsgLength[0] = 0;
			optMsgLength[1] = (UCHAR)(optLen+1);	//add RC_Code

		}
		else
		{
			optData[0]=VAP_RIF_RC_FAILURE;
				
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
			
		optMsgLength[0] = 0;
		optMsgLength[1] = 1;
	}
}

void api_pcd_vap_adm_SetCashBackRRP(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT iptLen = 0;

	UCHAR rspCode = 0xFF;

	UCHAR *InputDataPtr = NULLPTR;

	UCHAR tagDF06[2] = {0xDF,0x06};	
	UCHAR tagDF00[2] = {0xDF,0x00};
	UCHAR tagDF01[2] = {0xDF,0x01};
	UCHAR tagDF02[2] = {0xDF,0x02};

	UCHAR tmpDF06[2] = {0};
	UCHAR tmpDF00[6] = {0};
	UCHAR tmpDF01[6] = {0};
	UCHAR tmpDF02[6] = {0};

	UCHAR tmpLenT=0, tmpLenL=0, CheckLen=0;
	UINT  tmpLenV=0;

	UCHAR ErrFlag = FALSE;
#ifndef _PLATFORM_AS350
	UCHAR ToFlashCashBackData[20] = {0};
#endif
	
	iptLen = iptMsgLength[0]*256+iptMsgLength[1];
	CheckLen = iptLen;

	InputDataPtr = iptData;

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if(iptLen <= 32) //DF0602+V, DF0006+V, DF0106+V,DF0206+V //20140717 "==" change to "<="
		{
			//handle DF06
			if(!memcmp(InputDataPtr,tagDF06,2))
			{
				rspCode = UT_Get_TLVLength(InputDataPtr,&tmpLenT,&tmpLenL,&tmpLenV);
				if(rspCode == SUCCESS)
				{
					//Check format
					if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==2))
					{
						//shift to data, Set Data
						InputDataPtr+=(tmpLenT+tmpLenL);
						memcpy(tmpDF06,InputDataPtr,tmpLenV);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else
						ErrFlag = 1;
				}
				else
					ErrFlag = 1;
			}
			else
				ErrFlag = 1;
			
			//handle DF00 Reader Contactless Txn Limit
			if(!memcmp(InputDataPtr,tagDF00,2))
			{
				rspCode = UT_Get_TLVLength(InputDataPtr,&tmpLenT,&tmpLenL,&tmpLenV);
				if(rspCode == SUCCESS)
				{
					//Check format
					if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==6))
					{
						//shift to data, Set Data
						InputDataPtr+=(tmpLenT+tmpLenL);
						memcpy(tmpDF00,InputDataPtr,tmpLenV);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==0))	//20140111 absset 
					{
						InputDataPtr+=(tmpLenT+tmpLenL);
						memset(tmpDF00,0x00,6);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else
						ErrFlag = 1;					
				}
				else
					ErrFlag = 1;
			}
			else
				ErrFlag = 1;
			
			//DF01 Reader Contactless CVM Limit
			if(!memcmp(InputDataPtr,tagDF01,2))
			{
				rspCode = UT_Get_TLVLength(InputDataPtr,&tmpLenT,&tmpLenL,&tmpLenV);
				if(rspCode == SUCCESS)
				{
					//Check format
					if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==6))
					{
						//shift to data, Set Data
						InputDataPtr+=(tmpLenT+tmpLenL);
						memcpy(tmpDF01,InputDataPtr,tmpLenV);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==0))	//20140111 absset 
					{
						InputDataPtr+=(tmpLenT+tmpLenL);
						memset(tmpDF01,0x00,6);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else
						ErrFlag = 1;						
				}
				else
					ErrFlag = 1;
			}
			else
				ErrFlag = 1;

			//DF02 Reader Contactless Floor Limit
			if(!memcmp(InputDataPtr,tagDF02,2))
			{
				rspCode = UT_Get_TLVLength(InputDataPtr,&tmpLenT,&tmpLenL,&tmpLenV);
				if(rspCode == SUCCESS)
				{
					//Check format
					if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==6))
					{
						//shift to data, Set Data
						InputDataPtr+=(tmpLenT+tmpLenL);
						memcpy(tmpDF02,InputDataPtr,tmpLenV);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else if((tmpLenT==2) && (tmpLenL==1) && (tmpLenV==0))	//20140111 absset 
					{
						InputDataPtr+=(tmpLenT+tmpLenL);
						memset(tmpDF02,0x00,6);

						//shift to next tag
						InputDataPtr+=tmpLenV;

						//check total legth
						CheckLen-=(tmpLenT+tmpLenL+tmpLenV);
					}
					else
						ErrFlag = 1;					
				}
				else
					ErrFlag = 1;
			}
			else
				ErrFlag = 1;

			//check format and write to flash
			if((ErrFlag == FALSE) && (CheckLen == 0))
			{		
				optData[0]=VAP_RIF_RC_SUCCESS;
			
				optMsgLength[0] = 0;
				optMsgLength[1] = 1;

				memcpy(glv_parDF06_VISA[VISA_Cashback_Mode],tmpDF06,2);
				memcpy(glv_parDF00_VISA[VISA_Cashback_Mode],tmpDF00,6);
				memcpy(glv_parDF01_VISA[VISA_Cashback_Mode],tmpDF01,6);
				memcpy(glv_parDF02_VISA[VISA_Cashback_Mode],tmpDF02,6);

#ifndef _PLATFORM_AS350

#ifdef _PLATFORM_AS210
				rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
				rspCode = apiOK;
#endif
				if(rspCode == apiOK)
				{
					memset(ToFlashCashBackData,0xFF,20);
					memcpy(ToFlashCashBackData,glv_parDF06_VISA[VISA_Cashback_Mode],2);	
					memcpy(&ToFlashCashBackData[2],glv_parDF00_VISA[VISA_Cashback_Mode],6);
					memcpy(&ToFlashCashBackData[8],glv_parDF01_VISA[VISA_Cashback_Mode],6);
					memcpy(&ToFlashCashBackData[14],glv_parDF02_VISA[VISA_Cashback_Mode],6);

					//write to flash
					api_fls_write(FLSID_PARA,FLS_ADDRESS_CashBack_Data,20,ToFlashCashBackData);
				}
#endif
			}
			else
			{
				optData[0]=VAP_RIF_RC_INVALID_DATA;
			
				optMsgLength[0] = 0;
				optMsgLength[1] = 1;
			}			
		}
		else
		{
			optData[0]=VAP_RIF_RC_FAILURE;
			
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
		
		optMsgLength[0] = 0;
		optMsgLength[1] = 1;
	}

}

void api_pcd_vap_adm_GetKeyRevoList(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	iptLen = 0;

#ifdef _PLATFORM_AS350
	UCHAR	EmptySet[100] = {0x00}	;
#else
	UCHAR	InFlashRevoListData[101] = {0};
	UCHAR	*RevoListPtr = NULLPTR;
	UCHAR	rspCode = 0xFF;	
	UCHAR 	RevoListSize = 0;
#endif
	UCHAR 	OptLen = 0;
	UCHAR	*OptPtr = NULLPTR;
	UCHAR	RevoListTotalNum = 0;
	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

	iptLen = iptMsgLength[0]*256+iptMsgLength[1];
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		if((iptLen==1) && (iptData[0] == 0xFF))		//Check length and data
		{

#ifndef _PLATFORM_AS350

#ifdef _PLATFORM_AS210		
			rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
			rspCode = apiOK;
#endif
			if(rspCode == apiOK)
			{
				rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_RevoList,101,InFlashRevoListData);
				if(rspCode == apiOK)
				{
					if(((InFlashRevoListData[0] == 0xFF) && (InFlashRevoListData[1] == 0xFF)) || 
						((InFlashRevoListData[0] == 0x00) && (InFlashRevoListData[1] == 0x00)))	//empty, return SUCCESS and Length 0
					{
						//20140109 changed
						optMsgLength[0] = 0;
						optMsgLength[1] = 2;
						optData[0]=VAP_RIF_RC_SUCCESS;
						optData[1]=0;
					}
					else
					{
						RevoListTotalNum = InFlashRevoListData[0];
						if(RevoListTotalNum < 11)	//total Revo List Number should be 1~10
						{							
							RevoListPtr = &InFlashRevoListData[1];

							optData[1] = RevoListTotalNum;	//20140304, added total key revo number

							OptLen += 1;	//20140304, added, 1 for key revo list number

							OptPtr = &optData[2];	//20140304, reserve 2 bytes for the "RC_Code" and "key list number"
							
							while(RevoListTotalNum--)
							{
								RevoListSize = *RevoListPtr;	//Get Revo List Size,	L
								RevoListPtr++;					//20140304 shift to the value
								memcpy(OptPtr,RevoListPtr,RevoListSize);	//Copy L(1 byte) + V(L Bytes)
								RevoListPtr+=RevoListSize;	//Shift the pointer to the next List
								OptPtr+=RevoListSize;
								
								OptLen += RevoListSize;	//20140304 changed
							}
							
							if(VAP_VISA_P_ENCRYPTION)
							{
								OriLen = OptLen;
								memcpy(OriData,&optData[1],OriLen);
								api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
								OptLen = EryptLen;
							}

							optData[0] = VAP_RIF_RC_SUCCESS;	

							optMsgLength[0] = 0;
							optMsgLength[1] = OptLen + 1;	//data size and RC code
						}
						else
						{
							optMsgLength[0] = 0;
							optMsgLength[1] = 1;
							optData[0]=VAP_RIF_RC_FAILURE;
						}					
					}
				}		
			}
#else
			memset(EmptySet,0x00,sizeof(EmptySet));
			
			if(!memcmp(EmptySet,Revo_List,90))
			{
				//20140109 changed
				optMsgLength[0] = 0;
				optMsgLength[1] = 2;
				optData[0]=VAP_RIF_RC_SUCCESS;
				optData[1]=0;
			}
			else
			{
				OptPtr = &optData[2];
				while(memcmp(&Revo_List[RevoListTotalNum][0],EmptySet,9))
				{
					memcpy(OptPtr,&Revo_List[RevoListTotalNum][0],9);
					OptPtr+=9;
					OptLen+=9;
					RevoListTotalNum++;
				}		
				
				OptLen+=1;	//Add List total number
				
				optData[0] = VAP_RIF_RC_SUCCESS;
				optData[1] = RevoListTotalNum;
				
				if(VAP_VISA_P_ENCRYPTION)
				{
					OriLen = OptLen;
					memcpy(OriData,&optData[1],OriLen);
					api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
					OptLen = EryptLen;
				}

				optMsgLength[0] = 0;
				optMsgLength[1] = OptLen + 1 ;	//RC code
			}	
#endif			
		}
		else
		{
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
			optData[0]=VAP_RIF_RC_INVALID_DATA;
		}
	}
	else
	{
		optMsgLength[0] = 0;
		optMsgLength[1] = 1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
}

void api_pcd_vap_adm_SetKeyRevoList(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR emptySet[9] = {0x00};
	UCHAR tmpRevoList[10][9] = {{0x00}};
	UCHAR RevoListIndex = 0;
	UCHAR ListNum = 0;
	UCHAR Err_flg = 0;
	UINT iptLen = 0;
	UINT iptFileLen = 0;

#ifndef _PLATFORM_AS350
	UCHAR rspCode = 0xFF;
	UCHAR InListIndex=0;
	UCHAR ToFlashRevoListData[101] = {0};
	UCHAR *ToFlashRevoListDataPtr = NULLPTR;
#endif
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		iptLen = iptMsgLength[0]*256 + iptMsgLength[1];

		iptFileLen = iptLen;

		//20140304 removed
		/*
		for(i = 0 ; i < 10 ; i++)
		{
			//20140109
			if(iptData[index] > 10) //Check Len
			{
				Err_flg = 1;
				break;
			}

			iptLen -= iptData[index]+1; 		//the 1 means length of length, EX : L-V => 0x01,0x00

			if(iptData[index+1] == 0x00)
				memset(&Revo_List[i][0],0x00,9);
			else
				memcpy(&Revo_List[i][0],&iptData[index+1],iptData[index]);
			
			index+=iptData[index]+1;			

			if(iptLen == 0)
				break;
		}
		*/
		//20140304 removed end

		//20140304 added

		memcpy(tmpRevoList,Revo_List,sizeof(Revo_List));
		
		if(iptData[0] == 0x00)	//add
		{
			iptLen-=1;	//minus "add command"
			
			//find free space
			for(RevoListIndex = 0; RevoListIndex<10 ; RevoListIndex++)
			{
				if(!memcmp(tmpRevoList[RevoListIndex],emptySet,9))
				{
					Err_flg = FALSE;
					break;
				}
				else
					Err_flg = TRUE;
			}

			if(Err_flg == FALSE)
			{
				memcpy(tmpRevoList[RevoListIndex],&iptData[1],9);

				memset(Revo_List,0x00,sizeof(Revo_List));

				memcpy(Revo_List,tmpRevoList,sizeof(Revo_List));

				//check the list num for writing to flash
				for(RevoListIndex=0;RevoListIndex<10;RevoListIndex++)
				{
					if(memcmp(tmpRevoList[RevoListIndex],emptySet,9))
					{
						memcpy(Revo_List[ListNum],tmpRevoList[RevoListIndex],9);
						ListNum++;
					}
				}				
			}			
		}
		else if(iptData[0] == 0x01) //delete
		{
			iptLen-=1;

			//find list
			for(RevoListIndex = 0; RevoListIndex<10 ; RevoListIndex++)
			{
				if(!memcmp(tmpRevoList[RevoListIndex],&iptData[1],9))
				{
					Err_flg = FALSE;
					break;
				}
				else
					Err_flg = TRUE;
			}

			if(Err_flg == FALSE)
			{
				memset(tmpRevoList[RevoListIndex],0x00,9);

				//re-sort the list
				memset(Revo_List,0x00,sizeof(Revo_List));

				for(RevoListIndex=0;RevoListIndex<10;RevoListIndex++)
				{
					if(memcmp(tmpRevoList[RevoListIndex],emptySet,9))
					{
						memcpy(Revo_List[ListNum],tmpRevoList[RevoListIndex],9);
						ListNum++;
					}
				}
			}
		}
		else
			Err_flg = TRUE;
		//2014304 added end

		if(Err_flg)
		{
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
			optData[0]=VAP_RIF_RC_FAILURE;
		}
		else
		{
			optMsgLength[0] = 0;
			optMsgLength[1] = 1;
			optData[0]=VAP_RIF_RC_SUCCESS;

#ifndef _PLATFORM_AS350

#ifdef _PLATFORM_AS210
			rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
			rspCode = apiOK;
#endif
			if(rspCode == apiOK)
			{
				memset(ToFlashRevoListData,0xFF,101);
				//set the pointer
				ToFlashRevoListDataPtr = ToFlashRevoListData;

//20140304, removed
/*		
				//handle the data, total_file_num(1 byte) + (L(1 byte) + V(0~10 bytes))*total_num 
				*ToFlashRevoListDataPtr = i+1;	//Total Exception File Num, "i+1" because i count from 0
				ToFlashRevoListDataPtr++;	//shift pointer

				//copy the input data
				memcpy(ToFlashRevoListDataPtr,iptData,iptFileLen);				
*/
//20140304, removed end

//20140304 added
				*ToFlashRevoListDataPtr = ListNum;
				ToFlashRevoListDataPtr++;
				while(ListNum--)
				{
					*ToFlashRevoListDataPtr=9;
					ToFlashRevoListDataPtr++;
					memcpy(ToFlashRevoListDataPtr,Revo_List[InListIndex],9);
					ToFlashRevoListDataPtr+=9;
					
					InListIndex++;
				}
//20140304 added end 			
				//write to flash
				api_fls_write(FLSID_PARA,FLS_ADDRESS_RevoList,101,ToFlashRevoListData);
			}
#endif
		}
			
	}
	else
	{
		optMsgLength[0] = 0;
		optMsgLength[1] = 1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}
}


void api_pcd_vap_adm_GetBaudRate(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData)
{
	UINT	msgLength = 0,OutMsgLen = 0;

	UCHAR	Errflg = FALSE;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

	iptData[0] = iptData[0];
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		
		if(msgLength == 0x0000)
		{			
			switch(glv_vap_BaudRate)
			{
				case 0xC0:		//19200
					OutMsgLen=2;
					optData[0]=VAP_RIF_RC_SUCCESS;
					optData[1]=0x04;
					break;
					
				case 0xE00:		//28800
					OutMsgLen=2;
					optData[0]=VAP_RIF_RC_SUCCESS;
					optData[1]=0x03;					
					break;
					
				case 0xE0:		//38400
					OutMsgLen=2;
					optData[0]=VAP_RIF_RC_SUCCESS;
					optData[1]=0x02;
					break;
					
				case 0x100:		//57600
					OutMsgLen=2;
					optData[0]=VAP_RIF_RC_SUCCESS;
					optData[1]=0x01;
					break;	
					
				case 0x200:		//115200
					OutMsgLen=2;
					optData[0]=VAP_RIF_RC_SUCCESS;
					optData[1]=0x00;
					break;

				default:
					OutMsgLen=1;
					optData[0]=VAP_RIF_RC_INVALID_DATA;
					Errflg = TRUE;
					break;
			}	

			if((!Errflg) && (VAP_VISA_P_ENCRYPTION))
			{
				memcpy(OriData,&optData[1],OutMsgLen-1);	//20140103 minus RC_Code
				OriLen = OutMsgLen-1;						//20140103 minus RC_Code
				api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
				OutMsgLen = EryptLen+1;						//20140103 add RC_Code
			}
		}
		else
		{
			OutMsgLen = 1;
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}
	}
	else
	{
		OutMsgLen = 1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	api_pcd_vap_Set_MessageLength(OutMsgLen, optMsgLength);

}

void api_pcd_vap_adm_SetBaudRate(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData)
{
	UINT	msgLength = 0,OutMsgLen = 0;

#ifndef _PLATFORM_AS350
	UCHAR	rspCode = 0xFF;
	UCHAR	ToFlashBaudData[2] = {0};
#endif
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		
		if(msgLength == 0x0001)
		{			
			OutMsgLen=1;
			
			switch(iptData[0])
			{
				case 0x04:		//19200
					optData[0]=VAP_RIF_RC_SUCCESS;
					glv_vap_BaudRate = COM_19200; 
					break;
					
				case 0x03:		//28800
					optData[0]=VAP_RIF_RC_SUCCESS;
					glv_vap_BaudRate = COM_28800; 
					break;
					
				case 0x02:		//38400
					optData[0]=VAP_RIF_RC_SUCCESS;
					glv_vap_BaudRate = COM_38400; 					
					break;
					
				case 0x01: 	//57600
					optData[0]=VAP_RIF_RC_SUCCESS;
					glv_vap_BaudRate = COM_57600; 
					break;	
					
				case 0x00: 	//115200
					optData[0]=VAP_RIF_RC_SUCCESS;
					glv_vap_BaudRate = COM_115200;
					break;

				default:
					optData[0]=VAP_RIF_RC_INVALID_DATA;

					//Set Default Value
					//glv_vap_BaudRate = 0xE0;
					break;
			}	
		}
		else
		{
			OutMsgLen = 1;
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}
	}
	else
	{
		OutMsgLen = 1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

#ifndef _PLATFORM_AS350
	//write to flash
	if(optData[0] == VAP_RIF_RC_SUCCESS)
	{
#ifdef _PLATFORM_AS210
		rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
		rspCode = apiOK;
#endif
		if(rspCode == apiOK)
		{
			ToFlashBaudData[0] = (glv_vap_BaudRate & 0xFF00) >> 8;
			ToFlashBaudData[1] = glv_vap_BaudRate & 0x00FF;
			api_fls_write(FLSID_PARA,FLS_ADDRESS_BaudRate,2,ToFlashBaudData);
		}
	}
#endif

	api_pcd_vap_Set_MessageLength(OutMsgLen, optMsgLength);

}

void api_pcd_vap_adm_ResetAcquirerKey(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData)
{
	UCHAR rspCode = 0xFF;
	UINT msgLength = 1;
	UINT iptLen = 0;
	UCHAR flgCommand = FALSE;
	UCHAR IndexCommand = FALSE;
	UCHAR MEK_Key[16];
	UCHAR AEK_Key[16];

	if(api_pcd_vap_flgAdmMode == TRUE)
	{
		iptLen = iptMsgLength[0]*256 + iptMsgLength[1];
		if(iptLen == 2)
		{
			if((iptData[0] == 0x02) || (iptData[0] == 0x06))		//MEK or AEK
			{
				if(iptData[1]!=0x00)		//Check Index
				{
					if(iptData[1] == 0x01)
					{
						//20140107
						if(iptData[0] == 0x02)
						{
							memset(MEK_Key,0xFF,16);
#ifdef _PLATFORM_AS210
							rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
							rspCode = apiOK;
#endif
							if(rspCode == apiOK)
							{
								api_fls_write(FLSID_PARA,FLS_ADDRESS_MEK,16,MEK_Key);
							}
						}
						else
						{
							memset(AEK_Key,0xFF,16);							
#ifdef _PLATFORM_AS210
							rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
							rspCode = apiOK;
#endif
							if(rspCode == apiOK)
							{
								api_fls_write(FLSID_PARA,FLS_ADDRESS_AEK,16,AEK_Key);
							}
						}
					}
					else
						IndexCommand = TRUE;					
				}
				else
					flgCommand = TRUE;
			}
			else
				flgCommand = TRUE;
		}
		else
			flgCommand = TRUE;

		if(flgCommand || IndexCommand)
		{
			if(flgCommand)
				optData[0]=VAP_RIF_RC_INVALID_COMMAND;
			else
				optData[0]=VAP_RIF_RC_FAILURE;
		}
		else
			optData[0]=VAP_RIF_RC_SUCCESS;
	}
	else
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);

}

void api_pcd_vap_adm_ReaderRecover(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData)
{	
	UINT msgLength = 1;
	UINT iptLen = 0;

	UCHAR flgCommand = FALSE;

	//Remove compiler warning
	iptData[0] = iptData[0];

	if(api_pcd_vap_flgAdmMode == TRUE)
	{
		iptLen = iptMsgLength[0]*256 + iptMsgLength[1];
		if(iptLen == 0)
		{
			/*
				Reader Recover
			*/
			//20140106 turn on card scanning function
		#ifdef PCD_PLATFORM_CLRC663
			ECL_LV1_FIELD_ON_EMV();
		#else
			ECL_LV1_FIELD_ON();
		#endif

			//20140107
			api_InvalidateReader = FALSE;
		}
		else
			flgCommand = TRUE;

		if(flgCommand)
		{
			optData[0]=VAP_RIF_RC_INVALID_COMMAND;
		}
		else
			optData[0]=VAP_RIF_RC_SUCCESS;
	}
	else
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
	
}


void api_pcd_vap_adm_GetPayPassConfigurationData(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData)
{
	UINT	iptLength=0;
	UINT	msgLength=0;
	UINT	cfgLen=0;
	UCHAR	cfgData[FLS_SIZE_CONFIGURATION_DATA-2]={0};
	UCHAR	rspCode=0;

	//Set Default
	optData[0]=VAP_RIF_RC_FAILURE;
	msgLength=1;
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		iptLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (iptLength == 0)
		{
			rspCode=FLS_Read_PayPassConfigurationData(&cfgLen, cfgData);
			if (rspCode == SUCCESS)
			{
				rspCode=api_pcd_vap_Check_PayPassConfigurationData(cfgLen, cfgData);
				if (rspCode == SUCCESS)
				{
					//Patch Data
					if (VAP_VISA_P_ENCRYPTION)
					{
						api_pcd_vap_Encrypt_Data(cfgLen, cfgData, &msgLength, &optData[1], VISA_ASession_Key);
					}
					else
					{
						msgLength=cfgLen;
						memcpy(&optData[1], cfgData, cfgLen);						
					}

					//Patch Response Code
					optData[0]=VAP_RIF_RC_SUCCESS;
					msgLength++;
				}
			}
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	iptData[0]=iptData[0];
	
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_SetPayPassConfigurationData(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=FALSE;
	UINT	cfgLen=0;
	UCHAR	cfgData[FLS_SIZE_CONFIGURATION_DATA-2]={0};

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);

		rspCode=api_pcd_vap_Check_PayPassConfigurationData(msgLength, iptData);
		if (rspCode == SUCCESS)
		{
			rspCode=FLS_Read_PayPassConfigurationData(&cfgLen, cfgData);
			if ((rspCode == SUCCESS) &&
				((msgLength == cfgLen) && (!memcmp(iptData, cfgData, msgLength))))
			{
				;	//Same Data Do Nothing
			}
			else
			{
				rspCode=FLS_Write_PayPassConfigurationData(msgLength, iptData);
			}
		}

		if (rspCode == SUCCESS)
		{
			ETP_Load_MPP_ConfigurationData();
		}

		optData[0]=(rspCode == SUCCESS)?(VAP_RIF_RC_SUCCESS):(VAP_RIF_RC_FAILURE);
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	msgLength=1;

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_GetPayPassDataExchangeData(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData)
{
	UINT msgLength=0;
	
	iptMsgLength=iptMsgLength;iptData=iptData;	//Remove Warning

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		optData[0]=VAP_RIF_RC_SUCCESS;

		if (VAP_VISA_P_ENCRYPTION)
		{
			api_pcd_vap_Encrypt_Data(mpp_lenDE_xmlBuf, mpp_ptrDE_xmlBuf, &msgLength, &optData[1], VISA_ASession_Key);
		}
		else
		{
			memcpy(&optData[1], mpp_ptrDE_xmlBuf, mpp_lenDE_xmlBuf);
			msgLength=mpp_lenDE_xmlBuf;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	api_pcd_vap_Set_MessageLength((1+msgLength), optMsgLength);
}

void api_pcd_vap_adm_SetPayPassDataExchangeData(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData)
{
	UINT	msgLength=0;
	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);

	mpp_ptrDE_xmlBuf=malloc(msgLength);
	if (mpp_ptrDE_xmlBuf == NULLPTR)
	{
		optData[0]=VAP_RIF_RC_FAILURE;
	}
	else
	{
		mpp_lenDE_xmlBuf=msgLength;
		memcpy(mpp_ptrDE_xmlBuf, iptData, mpp_lenDE_xmlBuf);

		optData[0]=VAP_RIF_RC_SUCCESS;
	}

	api_pcd_vap_Set_MessageLength(1, optMsgLength);
}

void api_pcd_vap_adm_GetQuickPassMultipleAIDData(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	iptLength=0;
	UINT	msgLength=0;
	UINT	datLen=0;
	UCHAR	datBuffer[FLS_SIZE_MULTIAID_DATA-2]={0};
	UCHAR	rspCode=0;

	//Set Default
	optData[0]=VAP_RIF_RC_FAILURE;
	msgLength=1;
		
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		iptLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (iptLength == 0)
		{
			rspCode=FLS_Read_QuickPassMultipleAIDData(&datLen, datBuffer);
			if (rspCode == SUCCESS)
			{
				rspCode=api_pcd_vap_Check_MultipleAIDData(datLen, datBuffer, FLS_NUMBER_AID_SET_UPI);
				if (rspCode == SUCCESS)
				{
					rspCode=api_pcd_vap_Set_QuickPayMultipleAIDData(datLen, datBuffer);
					if (rspCode == SUCCESS)
					{
						//Patch Data
						if (VAP_VISA_P_ENCRYPTION)
						{
							api_pcd_vap_Encrypt_Data(datLen, datBuffer, &msgLength, &optData[1], VISA_ASession_Key);
						}
						else
						{
							msgLength=datLen;
							memcpy(&optData[1], datBuffer, datLen);						
						}

						//Patch Response Code
						optData[0]=VAP_RIF_RC_SUCCESS;
						msgLength++;
					}
				}
			}
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	iptData[0]=iptData[0];
	
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_SetQuickPassMultipleAIDData(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=FALSE;
	UINT	datLen=0;
	UCHAR	datBuffer[FLS_SIZE_MULTIAID_DATA-2]={0};

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);

		rspCode=api_pcd_vap_Check_MultipleAIDData(msgLength, iptData, FLS_NUMBER_AID_SET_UPI);
		if (rspCode == SUCCESS)
		{
			rspCode=FLS_Read_QuickPassMultipleAIDData(&datLen, datBuffer);
			if ((rspCode == SUCCESS) &&
				((msgLength == datLen) && (!memcmp(iptData, datBuffer, msgLength))))
			{
				;	//Same Data Do Nothing				
			}
			else
			{
				rspCode=FLS_Write_QuickPassMultipleAIDData(msgLength, iptData);
			}
		}

		if (rspCode == SUCCESS)
		{
			rspCode=api_pcd_vap_Set_QuickPayMultipleAIDData(msgLength, iptData);
			if (rspCode == SUCCESS)
			{
				//Reload Parameter
				ETP_Initialize_CombinationTable();
			}
		}

		optData[0]=(rspCode == SUCCESS)?(VAP_RIF_RC_SUCCESS):(VAP_RIF_RC_FAILURE);
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	msgLength=1;

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_GetQuickPassMultipleAIDEnable(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;
	UCHAR	flgError=TRUE;

#ifndef _PLATFORM_AS350
	UCHAR	rspCode=apiOK;
	UCHAR	datBuffer[1]={0};
#endif

	iptData=iptData;

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (msgLength == 0)
		{

#ifdef _PLATFORM_AS350
			msgLength=2;
			optData[0]=VAP_RIF_RC_SUCCESS;
			optData[1]=etp_flgMtiAID_UPI;
			flgError=FALSE;
#else

#ifdef _PLATFORM_AS210	
			rspCode=api_fls_memtype(FLSID_PARA, FLSType_SRAM);
#endif

			if (rspCode == apiOK)
			{
				rspCode=api_fls_read(FLSID_PARA, FLS_ADDRESS_MULTIAID_ENABLE, 1, datBuffer);
				if (rspCode == apiOK)
				{
					if ((datBuffer[0] == TRUE) || (datBuffer[0] == FALSE))
					{
						//Set Flag
						etp_flgMtiAID_UPI=datBuffer[0];
						
						//Patch Data
						if (VAP_VISA_P_ENCRYPTION)
						{
							api_pcd_vap_Encrypt_Data(1 ,datBuffer, &msgLength, &optData[1], VISA_ASession_Key);
						}
						else
						{
							msgLength=1;
							optData[1]=datBuffer[0];
						}

						//Patch Response Code
						msgLength++;
						optData[0]=VAP_RIF_RC_SUCCESS;

						flgError=FALSE;
					}
				}
			}
#endif
		}

		if (flgError == TRUE)
		{
			msgLength=1;
			optData[0]=VAP_RIF_RC_FAILURE;
		}
	}
	else
	{
		msgLength=1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_SetQuickPassMultipleAIDEnable(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;

#ifndef _PLATFORM_AS350
	UCHAR	rspCode=apiOK;
	UCHAR	datBuffer[1]={0};
#endif

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		//Set Default
		optData[0]=VAP_RIF_RC_FAILURE;
		
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (msgLength == 1)
		{
			if ((iptData[0] == TRUE) || (iptData[0] == FALSE))
			{

#ifdef _PLATFORM_AS350
				etp_flgMtiAID_UPI=iptData[0];
				optData[0]=VAP_RIF_RC_SUCCESS;
#else

#ifdef _PLATFORM_AS210	
				rspCode=api_fls_memtype(FLSID_PARA, FLSType_SRAM);
#endif

				if (rspCode == apiOK)
				{
					rspCode=api_fls_read(FLSID_PARA, FLS_ADDRESS_MULTIAID_ENABLE, 1, datBuffer);
					if (rspCode == apiOK)
					{
						if (iptData[0] == datBuffer[0])
						{
							;	//Same Data Do Nothing
						}
						else
						{
							rspCode=api_fls_write(FLSID_PARA, FLS_ADDRESS_MULTIAID_ENABLE, 1, iptData);
						}

						if (rspCode == apiOK)
						{
							etp_flgMtiAID_UPI=iptData[0];
							optData[0]=VAP_RIF_RC_SUCCESS;
						}
					}
				}
#endif

				//Reload Parameter
				ETP_Initialize_CombinationTable();
			}
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	msgLength=1;
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_GetDPASMultipleAIDData(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	iptLength=0;
	UINT	msgLength=0;
	UINT	datLen=0;
	UCHAR	datBuffer[FLS_SIZE_MULTIAID_DATA_DPAS-2]={0};
	UCHAR	rspCode=0;

	//Set Default
	optData[0]=VAP_RIF_RC_FAILURE;
	msgLength=1;
		
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		iptLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (iptLength == 0)
		{
			rspCode=FLS_Read_DPASMultipleAIDData(&datLen, datBuffer);
			if (rspCode == SUCCESS)
			{
				rspCode=api_pcd_vap_Check_MultipleAIDData(datLen, datBuffer, FLS_NUMBER_AID_SET_DISCOVER);
				if (rspCode == SUCCESS)
				{
					rspCode=api_pcd_vap_Set_DPASMultipleAIDData(datLen, datBuffer);
					if (rspCode == SUCCESS)
					{
						//Patch Data
						if (VAP_VISA_P_ENCRYPTION)
						{
							api_pcd_vap_Encrypt_Data(datLen, datBuffer, &msgLength, &optData[1], VISA_ASession_Key);
						}
						else
						{
							msgLength=datLen;
							memcpy(&optData[1], datBuffer, datLen);						
						}

						//Patch Response Code
						optData[0]=VAP_RIF_RC_SUCCESS;
						msgLength++;
					}
				}
			}
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	iptData[0]=iptData[0];
	
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_SetDPASMultipleAIDData(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=FALSE;
	UINT	datLen=0;
	UCHAR	datBuffer[FLS_SIZE_MULTIAID_DATA_DPAS-2]={0};

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);

		rspCode=api_pcd_vap_Check_MultipleAIDData(msgLength, iptData, FLS_NUMBER_AID_SET_DISCOVER);
		if (rspCode == SUCCESS)
		{
			rspCode=FLS_Read_DPASMultipleAIDData(&datLen, datBuffer);
			if ((rspCode == SUCCESS) &&
				((msgLength == datLen) && (!memcmp(iptData, datBuffer, msgLength))))
			{
				;	//Same Data Do Nothing				
			}
			else
			{
				rspCode=FLS_Write_DPASMultipleAIDData(msgLength, iptData);
			}
		}

		if (rspCode == SUCCESS)
		{
			rspCode=api_pcd_vap_Set_DPASMultipleAIDData(msgLength, iptData);
			if (rspCode == SUCCESS)
			{
				//Reload Parameter
				ETP_Initialize_CombinationTable();
			}
		}

		optData[0]=(rspCode == SUCCESS)?(VAP_RIF_RC_SUCCESS):(VAP_RIF_RC_FAILURE);
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	msgLength=1;

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_GetDPASMultipleAIDEnable(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;
	UCHAR	flgError=TRUE;

#ifndef _PLATFORM_AS350
	UCHAR	rspCode=apiOK;
	UCHAR	datBuffer[1]={0};
#endif

	iptData=iptData;

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (msgLength == 0)
		{

#ifdef _PLATFORM_AS350
			msgLength=2;
			optData[0]=VAP_RIF_RC_SUCCESS;
			optData[1]=etp_flgMtiAID_Discover;
			flgError=FALSE;
#else

#ifdef _PLATFORM_AS210	
			rspCode=api_fls_memtype(FLSID_PARA, FLSType_SRAM);
#endif

			if (rspCode == apiOK)
			{
				rspCode=api_fls_read(FLSID_PARA, FLS_ADDRESS_MULTIAID_ENABLE_DPAS, 1, datBuffer);
				if (rspCode == apiOK)
				{
					if ((datBuffer[0] == TRUE) || (datBuffer[0] == FALSE))
					{
						//Set Flag
						etp_flgMtiAID_Discover=datBuffer[0];
						
						//Patch Data
						if (VAP_VISA_P_ENCRYPTION)
						{
							api_pcd_vap_Encrypt_Data(1 ,datBuffer, &msgLength, &optData[1], VISA_ASession_Key);
						}
						else
						{
							msgLength=1;
							optData[1]=datBuffer[0];
						}

						//Patch Response Code
						msgLength++;
						optData[0]=VAP_RIF_RC_SUCCESS;

						flgError=FALSE;
					}
				}
			}
#endif
		}

		if (flgError == TRUE)
		{
			msgLength=1;
			optData[0]=VAP_RIF_RC_FAILURE;
		}
	}
	else
	{
		msgLength=1;
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_SetDPASMultipleAIDEnable(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;

#ifndef _PLATFORM_AS350
	UCHAR	rspCode=apiOK;
	UCHAR	datBuffer[1]={0};
#endif

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		//Set Default
		optData[0]=VAP_RIF_RC_FAILURE;
		
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (msgLength == 1)
		{
			if ((iptData[0] == TRUE) || (iptData[0] == FALSE))
			{

#ifdef _PLATFORM_AS350
				etp_flgMtiAID_Discover=iptData[0];
				optData[0]=VAP_RIF_RC_SUCCESS;
#else

#ifdef _PLATFORM_AS210	
				rspCode=api_fls_memtype(FLSID_PARA, FLSType_SRAM);
#endif

				if (rspCode == apiOK)
				{
					rspCode=api_fls_read(FLSID_PARA, FLS_ADDRESS_MULTIAID_ENABLE_DPAS, 1, datBuffer);
					if (rspCode == apiOK)
					{
						if (iptData[0] == datBuffer[0])
						{
							;	//Same Data Do Nothing
						}
						else
						{
							rspCode=api_fls_write(FLSID_PARA, FLS_ADDRESS_MULTIAID_ENABLE_DPAS, 1, iptData);
						}

						if (rspCode == apiOK)
						{
							etp_flgMtiAID_Discover=iptData[0];
							optData[0]=VAP_RIF_RC_SUCCESS;
						}
					}
				}
#endif

				//Reload Parameter
				ETP_Initialize_CombinationTable();
			}
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
	}

	msgLength=1;
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_GetMSDOption(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0xFFFF;

	UCHAR	OriData[1024] = {0};
	UINT	OriLen = 0;
	UINT	EryptLen = 0;

	if(api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (msgLength == 0)
		{
			iptData=iptData;	//Remove Warning
			
			optData[0]=VAP_RIF_RC_SUCCESS;
			optData[1]=Opt_MSD_Constructing_Track1;		//Track1
			optData[2]=Opt_MSD_Formatting_Track2;		//Track2
			optData[3]=Opt_MSD_CVN17_Support;			//MSD 17
			optData[4]=0;								//RFU

			msgLength=5;

			if(VAP_VISA_P_ENCRYPTION)
			{
				OriLen = msgLength-1;	//minus RC_Code
				memcpy(OriData,&optData[1],OriLen);
				api_pcd_vap_Encrypt_Data(OriLen,OriData,&EryptLen,&optData[1],VISA_ASession_Key);
				msgLength = EryptLen+1;	//add RC_Code
			}
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_DATA;
			msgLength=1;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
		msgLength=1;
	}

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_SetMSDOption(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
#ifndef _PLATFORM_AS350
	UCHAR rspCode=0xFF;
	UCHAR ToFlashMSDOption[4] = {0};
#endif
	
	UINT	msgLength=0;
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
		if (msgLength == 4)
		{
			Opt_MSD_Constructing_Track1 = iptData[0];
			Opt_MSD_Formatting_Track2	= iptData[1];
			Opt_MSD_CVN17_Support		= iptData[2];
			
			optData[0]=VAP_RIF_RC_SUCCESS;

			msgLength=1;

#ifndef _PLATFORM_AS350

#ifdef _PLATFORM_AS210
			rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
			rspCode = apiOK;
#endif
			if(rspCode == apiOK)
			{
				memset(ToFlashMSDOption,0xFF,4);
				ToFlashMSDOption[0] = Opt_MSD_Constructing_Track1;
				ToFlashMSDOption[1] = Opt_MSD_Formatting_Track2;
				ToFlashMSDOption[2] = Opt_MSD_CVN17_Support;

				api_fls_write(FLSID_PARA,FLS_ADDRESS_MSDOption,4,ToFlashMSDOption);
			}
#endif
		}
		else
		{
			optData[0]=VAP_RIF_RC_INVALID_DATA;
			msgLength=1;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
		msgLength=1;
	}

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

//Test Disable
#if 0
void api_pcd_vap_adm_GetExpressPayDefaultDRL(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	drlSet[19]={0};

	iptMsgLength=iptMsgLength;iptData=iptData;	//Remove Warning

	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		optData[0]=VAP_RIF_RC_SUCCESS;

		drlSet[0]=((aex_drlDefault.Combined[0] & 0x1C) == 0x1C)?(TRUE):(FALSE);
		memcpy(&drlSet[1], aex_drlDefault.rdrtxnLimit, 6);
		memcpy(&drlSet[7], aex_drlDefault.rdrFlrLimit, 6);
		memcpy(&drlSet[13], aex_drlDefault.rdrcvmLimit, 6);
		
		if (VAP_VISA_P_ENCRYPTION)
		{
			api_pcd_vap_Encrypt_Data(19, drlSet, &msgLength, &optData[1], VISA_ASession_Key);
		}
		else
		{
			memcpy(&optData[1], drlSet, 19);
			msgLength=19;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
		msgLength=1;
	}

	api_pcd_vap_Set_MessageLength((1+msgLength), optMsgLength);
}

void api_pcd_vap_adm_SetExpressPayDefaultDRL(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if ((msgLength == 19) && (iptData[0] < 2))
	{
		aex_drlDefault.Combined[0]=(iptData[0] == TRUE)?(0x1C):(0x00);
		memcpy(aex_drlDefault.rdrtxnLimit, &iptData[1], 6);
		memcpy(aex_drlDefault.rdrFlrLimit, &iptData[7], 6);
		memcpy(aex_drlDefault.rdrcvmLimit, &iptData[13], 6);

		optData[0]=VAP_RIF_RC_SUCCESS;
	}
	else
	{
		optData[0]=VAP_RIF_RC_FAILURE;
	}

	msgLength=1;
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_GetExpressPayDRL(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	cntIndex=0;
	UCHAR	drlSet[19*AEX_DRL_NUMBER]={0};

	iptMsgLength=iptMsgLength;iptData=iptData;	//Remove Warning
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		optData[0]=VAP_RIF_RC_SUCCESS;

		for (cntIndex=0; cntIndex < AEX_DRL_NUMBER; cntIndex++)
		{
			drlSet[cntIndex*19]=((aex_drlSets[cntIndex].Combined[0] & 0x1C) == 0x1C)?(TRUE):(FALSE);
			memcpy(&drlSet[cntIndex*19+1], aex_drlSets[cntIndex].rdrtxnLimit, 6);
			memcpy(&drlSet[cntIndex*19+7], aex_drlSets[cntIndex].rdrFlrLimit, 6);
			memcpy(&drlSet[cntIndex*19+13], aex_drlSets[cntIndex].rdrcvmLimit, 6);
		}
		
		if (VAP_VISA_P_ENCRYPTION)
		{
			api_pcd_vap_Encrypt_Data((19*AEX_DRL_NUMBER), drlSet, &msgLength, &optData[1], VISA_ASession_Key);
		}
		else
		{
			memcpy(&optData[1], drlSet, 304);
			msgLength=304;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
		msgLength=1;
	}

	api_pcd_vap_Set_MessageLength((1+msgLength), optMsgLength);
}

void api_pcd_vap_adm_SetExpressPayDRL(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	cntIndex=0;
	UCHAR	flgError=FALSE;
	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 304)
	{
		for (cntIndex=0; cntIndex < AEX_DRL_NUMBER; cntIndex++)
		{
			if (iptData[cntIndex*19] > 1)
			{
				flgError=TRUE;
				break;
			}
		}

		if (flgError == FALSE)
		{
			for (cntIndex=0; cntIndex < AEX_DRL_NUMBER; cntIndex++)
			{
				aex_drlSets[cntIndex].Combined[0]=(iptData[cntIndex*19+0] == TRUE)?(0x1C):(0x00);
				memcpy(aex_drlSets[cntIndex].rdrtxnLimit, &iptData[cntIndex*19+1], 6);
				memcpy(aex_drlSets[cntIndex].rdrFlrLimit, &iptData[cntIndex*19+7], 6);
				memcpy(aex_drlSets[cntIndex].rdrcvmLimit, &iptData[cntIndex*19+13], 6);
			}

			optData[0]=VAP_RIF_RC_SUCCESS;
		}
		else
		{
			optData[0]=VAP_RIF_RC_FAILURE;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_FAILURE;
	}

	msgLength=1;
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_GetExpressPayKeyRevoList(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;

	iptMsgLength=iptMsgLength;iptData=iptData;	//Remove Warning
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		optData[0]=VAP_RIF_RC_SUCCESS;
		
		if (VAP_VISA_P_ENCRYPTION)
		{
			api_pcd_vap_Encrypt_Data(900, (UCHAR*)&glv_CRL, &msgLength, &optData[1], VISA_ASession_Key);
		}
		else
		{
			memcpy(&optData[1], &glv_CRL, sizeof(glv_CRL));
			msgLength=900;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
		msgLength=1;
	}

	api_pcd_vap_Set_MessageLength((1+msgLength), optMsgLength);
}

void api_pcd_vap_adm_SetExpressPayKeyRevoList(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 900)
	{
		optData[0]=VAP_RIF_RC_SUCCESS;
		memcpy(&glv_CRL, &iptData[0], sizeof(glv_CRL));
	}
	else
	{
		optData[0]=VAP_RIF_RC_FAILURE;
	}

	msgLength=1;
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_adm_GetExpressPayExceptionFile(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;

	iptMsgLength=iptMsgLength;iptData=iptData;	//Remove Warning
	
	if (api_pcd_vap_flgAdmMode == TRUE)
	{
		optData[0]=VAP_RIF_RC_SUCCESS;
		
		if (VAP_VISA_P_ENCRYPTION)
		{
			api_pcd_vap_Encrypt_Data((UINT)sizeof(Exception_File), (UCHAR*)&Exception_File, &msgLength, &optData[1], VISA_ASession_Key);
		}
		else
		{
			memcpy(&optData[1], Exception_File, sizeof(Exception_File));
			msgLength=(UINT)sizeof(Exception_File);
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_ACCESS_NOT_PERFORMED;
		msgLength=1;
	}

	api_pcd_vap_Set_MessageLength((1+msgLength), optMsgLength);
}

void api_pcd_vap_adm_SetExpressPayExceptionFile(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 110)
	{
		optData[0]=VAP_RIF_RC_SUCCESS;
		memcpy(Exception_File, &iptData[0], 110);
	}
	else
	{
		optData[0]=VAP_RIF_RC_FAILURE;
	}

	msgLength=1;
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}
#endif

void api_pcd_vap_pro_GetVLPSupportIndicator(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;
	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 0)
	{
		iptData=iptData;	//Remove Warning
		
		optData[0]=VAP_RIF_RC_SUCCESS;
		optData[1]=glv_par9F7A[0];

		msgLength=2;
	}
	else
	{
		optData[0]=VAP_RIF_RC_INVALID_DATA;
		msgLength=1;
	}

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_pro_SetVLPSupportIndicator(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;
#ifndef _PLATFORM_AS350
	UCHAR rspCode = 0xFF;
#endif	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 1)
	{
		if (iptData[0] == 0x00)
		{
			glv_par9F7A[0]=FALSE;
		
			optData[0]=VAP_RIF_RC_SUCCESS;
		}
		else if (iptData[0] == 0x01)
		{
			glv_par9F7A[0]=TRUE;
		
			optData[0]=VAP_RIF_RC_SUCCESS;
		}
		else
		{
			optData[0]=VAP_RIF_RC_FAILURE;
		}
	}
	else
	{
		optData[0]=VAP_RIF_RC_INVALID_DATA;
	}

	if(optData[0]==VAP_RIF_RC_SUCCESS)
	{
	
#ifndef _PLATFORM_AS350
		
#ifdef _PLATFORM_AS210
		rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
		rspCode = apiOK;
#endif
		if(rspCode == apiOK)
		{
			api_fls_write(FLSID_PARA,FLS_ADDRESS_VLP_Support,1,glv_par9F7A);
		}
#endif			
	}

	api_pcd_vap_Set_MessageLength(1, optMsgLength);
}

void api_pcd_vap_pro_KeyInjection(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT IptLen = 0;

	UCHAR IMEK_MDK_Default[16] = {0};
	UCHAR IAEK_MDK_Default[16] = {0};

	UCHAR rspCode = 0xFF;

	IptLen = api_pcd_vap_Get_MessageLength(iptMsgLength);

	if(IptLen == 32)
	{
		memcpy(IMEK_MDK_Default,&iptData[0],16);
		memcpy(IAEK_MDK_Default,&iptData[16],16);

		rspCode=FLS_Write_EncryptionKey(0x00, 0x00, IMEK_MDK_Default);
		if(rspCode == SUCCESS)
		{
			rspCode=FLS_Write_EncryptionKey(0x04, 0x00, IAEK_MDK_Default);
			if(rspCode == SUCCESS)			
			{
				optData[5] = VAP_RIF_RC_SUCCESS;	
			}
			else
			{
				optData[5] = VAP_RIF_RC_FAILURE;
			}
		}
		else
		{
			optData[5] = VAP_RIF_RC_FAILURE;
		}
	}
	else
	{
		optData[5] = VAP_RIF_RC_FAILURE;
	}

	api_pcd_vap_Set_MessageLength(1, optMsgLength);
}

void api_pcd_vap_pro_GetEMVTags(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=FAIL;
	UINT	msgLength=0;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenEMVTags=0;
	UINT	lenEncrypt=0;
	UINT	recIndex=0;
	UCHAR	*ptrIn=NULLPTR;
	UCHAR	*ptrOut=NULLPTR;
	UCHAR	tmpData[1280];
	

	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength != 0)
	{
		ptrIn=&iptData[0];
		ptrOut=&optData[1];

		do
		{
			rspCode=UT_Get_TLVLengthOfT(ptrIn, &lenOfT);
			if (rspCode == SUCCESS)
			{
				rspCode=UT_Search_Record(lenOfT, ptrIn, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
				if (rspCode == SUCCESS)
				{
					rspCode=UT_Get_TLVLengthOfL((UCHAR*)glv_addTable[recIndex], &lenOfL);
					rspCode=UT_Get_TLVLengthOfV((UCHAR*)glv_addTable[recIndex], &lenOfV);
						
					memcpy(ptrOut, ptrIn, lenOfT);
					ptrOut+=lenOfT;

					memcpy(ptrOut, (UCHAR*)glv_addTable[recIndex], lenOfL);
					ptrOut+=lenOfL;

					memcpy(ptrOut, (UCHAR*)glv_addTable[recIndex]+3, lenOfV);
					ptrOut+=lenOfV;

					lenEMVTags+=(lenOfT+lenOfL+lenOfV);
					ptrIn+=lenOfT;
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		} while ((ptrIn-iptData) < msgLength);

		if ((ptrIn-iptData) != msgLength)
		{
			msgLength=1;
			optData[0]=VAP_RIF_RC_FAILURE;
		}
		else
		{
			optData[0]=VAP_RIF_RC_SUCCESS;

			if (VAP_VISA_P_ENCRYPTION)
			{
				memcpy(tmpData, &optData[1], lenEMVTags);
				api_pcd_vap_Encrypt_Data(lenEMVTags, tmpData, &lenEncrypt, &optData[1], VISA_ASession_Key);
				msgLength=1+lenEncrypt;
			}
			else
			{
				msgLength=1+lenEMVTags;
			}
		}
	}
	else
	{
		msgLength=1;
		optData[0]=VAP_RIF_RC_INVALID_DATA;
	}

	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

void api_pcd_vap_sys_OpenContactlessInterface(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;

	iptData=iptData;	//Remove Warning
	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 0)
	{
		APP_Open_ContactlessInterface();
		optData[0]=VAP_RIF_RC_SUCCESS;
	}
	else
	{
		optData[0]=VAP_RIF_RC_INVALID_DATA;
	}

	msgLength=1;
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}


void api_pcd_vap_sys_CloseContactlessInterface(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;

	iptData=iptData;	//Remove Warning
	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 0)
	{
		APP_Close_ContactlessInterface();
		optData[0]=VAP_RIF_RC_SUCCESS;
	}
	else
	{
		optData[0]=VAP_RIF_RC_INVALID_DATA;
	}

	msgLength=1;
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}


void api_pcd_vap_sys_InitializeContactlessParameter(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT	msgLength=0;

	iptData=iptData;	//Remove Warning
	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 0)
	{
		APP_L3_VISA_ReaderInterface();
		optData[0]=VAP_RIF_RC_SUCCESS;
	}
	else
	{
		optData[0]=VAP_RIF_RC_INVALID_DATA;
	}

	msgLength=1;
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}


void api_pcd_vap_sys_SetReadyForSaleBeep(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;

	iptData=iptData;	//Remove Warning
	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if ((msgLength == 1) && (iptData[0] <= 1))
	{
		api_pcd_vap_flgRFSBeep=iptData[0];
		optData[0]=VAP_RIF_RC_SUCCESS;
	}
	else
	{
		optData[0]=VAP_RIF_RC_INVALID_DATA;
	}

	msgLength=1;
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}


void api_pcd_vap_sys_SetRFRangeUnder4cm(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;

	iptData=iptData;	//Remove Warning
	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 0)
	{
		ECL_LV1_SetRegister(0x39, 0x0C);
		
		optData[0]=VAP_RIF_RC_SUCCESS;
	}
	else
	{
		optData[0]=VAP_RIF_RC_INVALID_DATA;
	}

	msgLength=1;
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}


void api_pcd_vap_sys_SetRFRangeEMV(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;

	iptData=iptData;	//Remove Warning
	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if (msgLength == 0)
	{
		ECL_LV1_InitialCL();

		optData[0]=VAP_RIF_RC_SUCCESS;
	}
	else
	{
		optData[0]=VAP_RIF_RC_INVALID_DATA;
	}

	msgLength=1;
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}


void api_pcd_vap_sys_SetJCBRefundResponse(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;

	iptData=iptData;	//Remove Warning
	
	msgLength=api_pcd_vap_Get_MessageLength(iptMsgLength);
	if ((msgLength == 1) && (iptData[0] <= 1))
	{
		api_pcd_vap_flgJCBRefResponse=iptData[0];
		optData[0]=VAP_RIF_RC_SUCCESS;
	}
	else
	{
		optData[0]=VAP_RIF_RC_INVALID_DATA;
	}

	msgLength=1;
	api_pcd_vap_Set_MessageLength(msgLength, optMsgLength);
}

