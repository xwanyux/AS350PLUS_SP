#include <string.h>
#include <stdlib.h>
#include "Glv_ReaderConfPara.h"
#include "ECL_Tag.h"
#include "Function.h"
#include "AEX_Define.h"
#include "ODA_Record.h"
#include "DBG_Function.h"
#include "VAP_ReaderInterface_Define.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Function.h"

// Add by Wayne 2020/08/21 to avoid compiler warning
#include "UTILS_CTLS.H"

extern UCHAR		etp_flgRestart;
extern UCHAR		etp_flgComError;
extern ETPCONFIG	etp_etpConfig[ETP_NUMBER_COMBINATION];
extern OUTCOME		etp_Outcome;
extern CAPK			glv_CAPK[CAPK_NUMBER];
extern CRL			glv_CRL[CRL_NUMBER];
extern UCHAR		L3_Response_Code;
extern UCHAR		D1_Data[150];
extern UINT			D1_Data_Length;
extern UCHAR		Tag57_Data[39];
extern UCHAR		Tag57_Data_Length;
extern UCHAR		SchemeID;

//			Data Buffer
UINT		aex_sndLen=0;
//UCHAR		aex_sndData[ETP_BUFFER_SIZE];
UCHAR		*aex_sndData;

UINT		aex_rcvLen=0;
//UCHAR		aex_rcvData[ETP_BUFFER_SIZE];
UCHAR		*aex_rcvData;

//			Function Parameter
UCHAR		aex_parFunction=AEX_FUNCTION_PARAMETER_GO_ONLINE;

//			Configuration Flag
UCHAR		aex_flgGoOnline=FALSE;
UCHAR		aex_flgDelayedAuthorization=FALSE;

//			Restart Flag
UCHAR		aex_flgRestart=FALSE;

//			Mode
UCHAR		aex_modTransaction=AEX_MODE_TRANSACTION_MAGSTRIPE;
UCHAR		aex_modInterface=AEX_MODE_INTERFACE_CARD;
UCHAR		aex_modODA=AEX_MODE_ODA_NONE;

UCHAR		aex_typAC=AEX_ACT_AAC;
	
UCHAR		aex_tag9F35_Modified=0;

//			CV Rules
//UCHAR		aex_cvrList[ECL_LENGTH_8E];
UCHAR		*aex_cvrList;
UCHAR		aex_cvrLen=0;
UCHAR		aex_cvrIndex=0;

//			DRL
LIMITSET	aex_drlDefault={0,{0},{0},{0},{0},{0},{0}};
LIMITSET	aex_drlSets[AEX_DRL_NUMBER]={{0,{0},{0},{0},{0},{0},{0}}};
UCHAR		aex_rstDRL=AEX_DRL_RESULT_NONE;

//			Pseudo Magnetic Stripe Track
UCHAR		aex_pmsTrkOne[AEX_PMS_LENGTH_TRACK_ONE];
UCHAR		aex_pmsTrkTwo[AEX_PMS_LENGTH_TRACK_TWO];

//			Unpredictable Number Range
UCHAR		aex_rngUnpNumber=AEX_RANGE_UNPREDICTABLE_NUMBER;

//			Deactivation Timer (100 ms)
UCHAR		aex_tmrDeactivation=AEX_TIMER_DEACTIVATION;


UCHAR AEX_Check_PresenceOfTagList(UINT iptLen, UCHAR *iptData);
UCHAR AEX_Compare_Date(UCHAR *iptDate1, UCHAR *iptDate2);
UCHAR AEX_OfflineDataAuthentication_SDA(void);
UCHAR AEX_Retrieve_PK_CA(UCHAR * iptRID, UCHAR iptIndex, UCHAR * optModLen, UCHAR * optModulus, UCHAR * optExponent);
UCHAR AEX_Retrieve_PK_ICC(UCHAR iptModLen, UCHAR * iptModulus, UCHAR * iptExponent, UCHAR * optModLen, UCHAR * optModulus, UCHAR * optExponent);
UCHAR AEX_Retrieve_PK_Issuer(UCHAR iptModLen, UCHAR * iptModulus, UCHAR * iptExponent, UCHAR * optModLen, UCHAR * optModulus, UCHAR * optExponent);
void  AEX_Set_CVMResults(UCHAR iptMethod, UCHAR iptCondition, UCHAR iptResult);
void  AEX_Subtract_Date(UCHAR iptYear, UCHAR iptMonth, UCHAR iptSubMonth, UCHAR *optYear, UCHAR *optMonth);
void  AEX_Subtract_Year(UCHAR iptYear, UCHAR iptSubYear, UCHAR *optYear);
UCHAR AEX_Verify_DynamicSignature(UCHAR iptModLen, UCHAR * iptModulus, UCHAR * iptExponent);
UCHAR AEX_Verify_SignedStaticApplicationData(UCHAR iptModLen, UCHAR * iptModulus, UCHAR * iptExponent);

extern UCHAR AEX_DOL_Get_DOLData(UINT iptLen, UCHAR * iptDOL, UCHAR * optLen, UCHAR * optData);
extern UCHAR AEX_DOL_Patch_DOLData(UINT iptLen, UCHAR * iptDOL, UCHAR * optLen, UCHAR * optData);


UCHAR AEX_Allocate_Buffer(void)
{
	aex_sndData=malloc(ETP_BUFFER_SIZE);
	aex_rcvData=malloc(ETP_BUFFER_SIZE);
	aex_cvrList=malloc(ECL_LENGTH_8E);

	if ((aex_sndData == NULLPTR) || (aex_rcvData == NULLPTR) || (aex_cvrList == NULLPTR))
	{
		return AEX_RESULT_TERMINATE;
	}

	return AEX_RESULT_SUCCESS;
}

void AEX_Check_api_rsa_recover(UCHAR iptLen, UCHAR *recData)
{
	UCHAR	idxNum=0;
	UCHAR	sftBytes=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	recBuffer[256]={0};
	
	ptrData=recData;

	//Calculate Shift Bytes
	for (idxNum=0; idxNum < 3; idxNum++)
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


UCHAR AEX_Check_AC(UINT iptLen, UCHAR *iptData)
{
	UCHAR	tag9F27[2]={0x9F,0x27};
	UCHAR	rsp9F27[1]={0};
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	*ptrData=NULLPTR;

	UT_Get_TLVLength(iptData, &lenOfT, &lenOfL, &lenOfV);
	
	if (iptData[0] == 0x80)
	{
		rsp9F27[0]=iptData[lenOfT+lenOfL];
	}
	else
	{
		ptrData=UT_Find_Tag(tag9F27, iptLen, iptData);

		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		rsp9F27[0]=ptrData[lenOfT+lenOfL];
	}

	if ((rsp9F27[0] & 0xC0) == 0xC0)	//RFU
	{
		return FAIL;
	}
	
	if ((rsp9F27[0] & 0xC0) == AEX_ACT_TC)
	{
		if (aex_typAC != AEX_ACT_TC)
		{
			return FAIL;
		}
	}
	else
	{
		if ((rsp9F27[0] & 0xC0) == AEX_ACT_AAC)
		{
			return FAIL;
		}
		else
		{
			if ((rsp9F27[0] & 0xC0) == AEX_ACT_ARQC)
			{
				if (aex_typAC == AEX_ACT_AAC)
				{
					return FAIL;
				}

				if (aex_flgGoOnline == FALSE)
				{
					return FAIL;
				}
			}
		}
	}
	
	return SUCCESS;
}


UCHAR AEX_Check_AFL(UINT iptLen, UCHAR *iptData)
{
	UCHAR	idxNumber=0;
	UCHAR	recSFI=0;
	UCHAR	recStart=0;
	UCHAR	recEnd=0;
	UCHAR	recODA=0;

	for (idxNumber=0; idxNumber < iptLen; idxNumber+=4)
	{
		recSFI=iptData[idxNumber];
		recStart=iptData[idxNumber+1];
		recEnd=iptData[idxNumber+2];
		recODA=iptData[idxNumber+3];

		if ((recSFI == 0) ||
			((recSFI & 0x07) != 0) || 
			((recSFI & 0xF8) == 0xF8) ||
			(recStart == 0) ||
			(recEnd < recStart) ||
			((recEnd - recStart + 1) < recODA))
		{
			return FAIL;
		}
	}
	
	return SUCCESS;
}


void AEX_Check_ApplicationUsageControl(void)
{
	UINT	lenOfV=0;
	UCHAR	flgDomestic=0xFF;
	UCHAR	rspCode=FALSE;
	UCHAR	lstConCode[4]=
	{
		0x5F,0x28,	//Issuer Country Code
		0x9F,0x1A	//Terminal Country Code
	};

	UT_Get_TLVLengthOfV(glv_tag9F07.Length, &lenOfV);
	if (lenOfV == 0)
	{
		return;
	}

	//Domestic/International Usage Check
	rspCode=AEX_Check_PresenceOfTagList((UINT)sizeof(lstConCode), lstConCode);
	if (rspCode == SUCCESS)
	{
		rspCode=UT_bcdcmp(glv_tag9F1A.Value, glv_tag5F28.Value, 2);
		flgDomestic=(rspCode == 0)?(TRUE):(FALSE);

		if (glv_tag9C.Value[0] == AEX_TXT_PURCHASE)
		{
			if (flgDomestic == TRUE)
			{
				if (((glv_tag9F07.Value[0] & AEX_AUC_VALID_FOR_DOMESTIC_GOODS) == 0) && 
					((glv_tag9F07.Value[0] & AEX_AUC_VALID_FOR_DOMESTIC_SERVICES) == 0))
				{
					glv_tag95.Value[1]|=AEX_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;
				}
			}
			else
			{
				if (((glv_tag9F07.Value[0] & AEX_AUC_VALID_FOR_INTERNATIONAL_GOODS) == 0) &&
					((glv_tag9F07.Value[0] & AEX_AUC_VALID_FOR_INTERNATIONAL_SERVICES) == 0))
				{
					glv_tag95.Value[1]|=AEX_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;
				}
			}
		}
		else
		{
			if (glv_tag9C.Value[0] == AEX_TXT_CASH)
			{
				if (flgDomestic == TRUE)
				{
					if ((glv_tag9F07.Value[0] & AEX_AUC_VALID_FOR_DOMESTIC_CASH_TRANSACTIONS) == 0)
					{
						glv_tag95.Value[1]|=AEX_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;
					}
				}
				else
				{
					if ((glv_tag9F07.Value[0] & AEX_AUC_VALID_FOR_INTERNATIONAL_CASH_TRANSACTIONS) == 0)
					{
						glv_tag95.Value[1]|=AEX_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;
					}
				}
			}
			else
			{
				if (glv_tag9C.Value[0] == AEX_TXT_CASHBACK)
				{
					if (flgDomestic == TRUE)
					{
						if ((glv_tag9F07.Value[1] & AEX_AUC_DOMESTIC_CASHBACK_ALLOWED) == 0)
						{
							glv_tag95.Value[1]|=AEX_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;
						}
					}
					else
					{
						if ((glv_tag9F07.Value[1] & AEX_AUC_INTERNATIONAL_CASHBACK_ALLOWED) == 0)
						{
							glv_tag95.Value[1]|=AEX_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;
						}
					}
				}
			}
		}
	}
	
	//Transaction Environment Check
	if (((glv_tag9F35.Value[0] == 0x14) ||
		(glv_tag9F35.Value[0] == 0x15) ||
		(glv_tag9F35.Value[0] == 0x16)) &&
		(glv_tag9F40.Value[0] & 0x80))
	{
		if ((glv_tag9F07.Value[0] & AEX_AUC_VALID_AT_ATMS) == 0)
		{
			glv_tag95.Value[1]|=AEX_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;
		}
	}
	else
	{
		if ((glv_tag9F07.Value[0] & AEX_AUC_VALID_AT_TERMINALS_OTHER_THAN_ATMS) == 0)
		{
			glv_tag95.Value[1]|=AEX_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;
		}
	}
}

void AEX_Check_ApplicationVersionNumber(void)
{
	UINT	lenOfV=0;

	UT_Get_TLVLengthOfV(glv_tag9F08.Length, &lenOfV);
	if (lenOfV != 0)
	{
		if (memcmp(glv_tag9F08.Value, glv_tag9F09.Value, 2))
		{
			glv_tag95.Value[1]|=AEX_TVR_ICC_AND_TERMINAL_HAVE_DIFFERENT_APPLICATION_VERSIONS;
		}
	}
}


void AEX_Check_CDAMandatoryTag(void)
{
	UCHAR	rspCode=FAIL;
	UCHAR	lstTag[]=
	{
		0x90,		//Issuer Public Key Certificate
		0x9F,0x32,	//Issuer Public Key Exponent
		0x9F,0x46,	//ICC Public Key Certificate                                                 
		0x9F,0x47	//ICC Public Key Exponent
	};

	rspCode=AEX_Check_PresenceOfTagList((UINT)sizeof(lstTag), lstTag);
	if (rspCode == FAIL)
	{
		glv_tag95.Value[0]|=AEX_TVR_ICC_DATA_MISSING;
	}
}


UCHAR AEX_Check_CRL(UCHAR *iptRID, UCHAR iptIndex, UCHAR *iptSerNumber)
{
	UCHAR	idxNum=0;
	UCHAR	endOfList[9]={0};

	for (idxNum=0; idxNum < CRL_NUMBER; idxNum++)
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


UCHAR AEX_Check_CVM_ConditionIsSatisfied(UCHAR iptCondition)
{
	UCHAR	rspCode=FALSE;
	UCHAR	binAmount[4]={0};
	UCHAR	lstTag[4]=
	{
		0x5F,0x2A,	//Transaction Currency Code
		0x9F,0x42	//Application Currency Code
	};
	
	if ((iptCondition == AEX_CVM_CONDITION_ALWAYS) ||
		(iptCondition == AEX_CVM_CONDITION_IF_NOT_UNATTENDED_CASH_AND_NOT_MANUAL_CASH_AND_NOT_PURCHASE_WITH_CASHBACK) ||
		(iptCondition == AEX_CVM_CONDITION_IF_TERMINAL_SUPPORTS_THE_CVM))
	{
		rspCode=TRUE;
	}
	else
	{
		if ((iptCondition == AEX_CVM_CONDITION_IF_TRANSACTION_IS_IN_THE_APPLICATION_CURRENCY_AND_IS_UNDER_X_VALUE) ||
			(iptCondition == AEX_CVM_CONDITION_IF_TRANSACTION_IS_IN_THE_APPLICATION_CURRENCY_AND_IS_OVER_X_VALUE) ||
			(iptCondition == AEX_CVM_CONDITION_IF_TRANSACTION_IS_IN_THE_APPLICATION_CURRENCY_AND_IS_UNDER_Y_VALUE) ||
			(iptCondition == AEX_CVM_CONDITION_IF_TRANSACTION_IS_IN_THE_APPLICATION_CURRENCY_AND_IS_OVER_Y_VALUE))
		{
			//Check Transaction Currency = Application Currency
			rspCode=AEX_Check_PresenceOfTagList((UINT)sizeof(lstTag), lstTag);
			if (rspCode == SUCCESS)
			{
				if (!memcmp(glv_tag5F2A.Value, glv_tag9F42.Value, 2))
				{
					//Convert BCD Amount to Binary Amount
					UT_bcd2hex(5, &glv_tag9F02.Value[1], binAmount);

					if (iptCondition == AEX_CVM_CONDITION_IF_TRANSACTION_IS_IN_THE_APPLICATION_CURRENCY_AND_IS_UNDER_X_VALUE)
					{
						if (UT_memcmp(binAmount, glv_tag8E.Value, 4) <= 0)
						{
							rspCode=TRUE;
						}
					}
					else if (iptCondition == AEX_CVM_CONDITION_IF_TRANSACTION_IS_IN_THE_APPLICATION_CURRENCY_AND_IS_OVER_X_VALUE)
					{
						if (UT_memcmp(binAmount, glv_tag8E.Value, 4) > 0)
						{
							rspCode=TRUE;
						}
					}
					else if (iptCondition == AEX_CVM_CONDITION_IF_TRANSACTION_IS_IN_THE_APPLICATION_CURRENCY_AND_IS_UNDER_Y_VALUE)
					{
						if (UT_memcmp(binAmount, &glv_tag8E.Value[4], 4) <= 0)
						{
							rspCode=TRUE;
						}
					}
					else //(iptCondition == AEX_CVM_CONDITION_IF_TRANSACTION_IS_IN_THE_APPLICATION_CURRENCY_AND_IS_OVER_Y_VALUE)
					{
						if (UT_memcmp(binAmount, &glv_tag8E.Value[4], 4) > 0)
						{
							rspCode=TRUE;
						}
					}
				}
			}
		}
	}
	
	return rspCode;
}


UCHAR AEX_Check_CVM_MethodIsRecognized(UCHAR iptMethod)
{
	UCHAR	medCode=0;

	medCode=iptMethod & 0xBF;
	
	switch (medCode)
	{
		case AEX_CVM_METHOD_FAIL_CVM_PROCESSING:
		case AEX_CVM_METHOD_PLAINTEXT_PIN_VERIFICATION_PERFORMED_BY_ICC:
		case AEX_CVM_METHOD_ENCIPHERED_PIN_VERIFIED_ONLINE:
		case AEX_CVM_METHOD_PLAINTEXT_PIN_VERIFICATION_PERFORMED_BY_ICC_AND_SIGNATURE_PAPER:
		case AEX_CVM_METHOD_ENCIPHERED_PIN_VERIFICATION_PERFORMED_BY_ICC:
		case AEX_CVM_METHOD_ENCIPHERED_PIN_VERIFICATION_PERFORMED_BY_ICC_AND_SIGNATURE_PAPER:
		case AEX_CVM_METHOD_SIGNATURE_PAPER:
		case AEX_CVM_METHOD_NO_CVM_REQUIRED:
			return TRUE;

		default:	break;
	}

	return FALSE;
}


UCHAR AEX_Check_CVM_MethodIsSupported(UCHAR iptMethod)
{
	UCHAR	rspCode=FALSE;
	UINT	lenOfV=0;

	if ((iptMethod & 0xBF) == AEX_CVM_METHOD_ENCIPHERED_PIN_VERIFIED_ONLINE)
	{
		if (glv_tag9F6E.Value[1] & AEX_ECRC_ONLINE_PIN_SUPPORTED)
		{
			rspCode=TRUE;
		}
		else
		{
			glv_tag95.Value[2]|=AEX_TVR_PIN_ENTRY_REQUIRED_AND_PIN_PAD_NOT_PRESENT_OR_NOT_WORKING;
		}
	}
	else if ((iptMethod & 0xBF) == AEX_CVM_METHOD_SIGNATURE_PAPER)
	{
		if (glv_tag9F6E.Value[1] & AEX_ECRC_SIGNATURE)
		{
			rspCode=TRUE;
		}
	}
	else if ((iptMethod & 0xBF) == AEX_CVM_METHOD_NO_CVM_REQUIRED)
	{
		if ((glv_tag9F6E.Value[2] & AEX_ECRC_CVM_REQUIRED) == 0x00)
		{
			rspCode=TRUE;
		}
	}
	else if ((iptMethod & 0xBF) == AEX_CVM_METHOD_PLAINTEXT_PIN_VERIFICATION_PERFORMED_BY_ICC)
	{
		if (aex_modInterface == AEX_MODE_INTERFACE_MOBILE)
		{
			if (glv_tag9F6E.Value[1] & AEX_ECRC_MOBILE_CVM_SUPPORTED)
			{
				rspCode=TRUE;
			}
		}
		else
		{
			UT_Get_TLVLengthOfV(glv_tag9F70.Length, &lenOfV);

			if ((lenOfV == 0) ||
				((glv_tag9F70.Value[0] & AEX_CIPC_MOBILE_INTERFACE_SUPPORTED) == 0))
			{
				glv_tag95.Value[2]|=AEX_TVR_PIN_ENTRY_REQUIRED_AND_PIN_PAD_NOT_PRESENT_OR_NOT_WORKING;
			}
		}
	}
	
	return rspCode;
}


UCHAR AEX_Check_DataElementsForMagstripeMode(void)
{
	UCHAR rspCode=FAIL;
	UCHAR lstManTags_Mag[2]=
	{
		0x9F,0x36
	};
	
	if (aex_modTransaction == AEX_MODE_TRANSACTION_MAGSTRIPE)
	{
		rspCode=AEX_Check_PresenceOfTagList((UINT)sizeof(lstManTags_Mag), lstManTags_Mag);
		if (rspCode == FAIL)
		{
			return FAIL;
		}
	}

	return SUCCESS;
}


UCHAR AEX_Check_DynamicReaderLimits(void)
{
	UINT		lenOfV=0;
	UCHAR		idxDRL=0xFF;
	UCHAR		rspDRL=AEX_DRL_RESULT_NONE;
	UCHAR		rspCode=0xFF;
	UCHAR		rdrLimit_MAX[6]={0x99,0x99,0x99,0x99,0x99,0x99};
	UCHAR		rdrLimit_NA[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	LIMITSET	drlData={0,{0},{0},{0},{0},{0},{0}};

	//Check Default DRL Configuration
	if (((aex_drlDefault.Combined[0] & AEX_DRL_ENABLE_TRANSACTION_LIMIT) == 0) &&
		((aex_drlDefault.Combined[0] & AEX_DRL_ENABLE_CVM_LIMIT) == 0) &&
		((aex_drlDefault.Combined[0] & AEX_DRL_ENABLE_FLOOR_LIMIT) == 0))
	{
		
DBG_Put_Text("Apply AID Reader Limits");

		if ((etp_etpConfig[3].pstRdrCTL == TRUE) && (glv_parDF00Len[3] == 6))
		{
			memcpy(drlData.rdrtxnLimit, etp_etpConfig[3].rdrCTL, 6);
		}
		else
		{
			memcpy(drlData.rdrtxnLimit, rdrLimit_NA, 6);
		}
		
		if (etp_etpConfig[3].pstRdrCRL == TRUE)
		{
			if (glv_parDF01Len[3] == 6)
			{
				memcpy(drlData.rdrcvmLimit, etp_etpConfig[3].rdrCRL, 6);
			}
			else
			{
				memcpy(drlData.rdrcvmLimit, rdrLimit_MAX, 6);
			}
		}
		else
		{
			memcpy(drlData.rdrcvmLimit, rdrLimit_NA, 6);
		}

		if ((etp_etpConfig[3].pstRdrCFL == TRUE) && (glv_parDF02Len[3] == 6))
		{
			memcpy(drlData.rdrFlrLimit, etp_etpConfig[3].rdrCFL, 6);
		}
		else
		{
			memcpy(drlData.rdrFlrLimit, rdrLimit_NA, 6);
		}
	}
	else
	{

DBG_Put_Text("Apply Dynamic Reader Limits");

		UT_Get_TLVLengthOfV(glv_tag9F70.Length, &lenOfV);
		
		idxDRL=(lenOfV == 0)?(0xFF):(glv_tag9F70.Value[1] & 0x0F);

DBG_Put_Text("DRL Index");
DBG_Put_UCHAR(idxDRL);

		if (idxDRL == 0xFF)
		{

DBG_Put_Text("Apply Default DRL");

			memcpy(&drlData, &aex_drlDefault, LIMITSET_LEN);
		}
		else
		{
			if (((aex_drlSets[idxDRL].Combined[0] & AEX_DRL_ENABLE_TRANSACTION_LIMIT) == 0) &&
				((aex_drlSets[idxDRL].Combined[0] & AEX_DRL_ENABLE_CVM_LIMIT) == 0) &&
				((aex_drlSets[idxDRL].Combined[0] & AEX_DRL_ENABLE_FLOOR_LIMIT) == 0))
			{

DBG_Put_Text("Apply Default DRL");

				memcpy(&drlData, &aex_drlDefault, LIMITSET_LEN);
			}
			else
			{

DBG_Put_Text("Apply Sets of DRL");

				memcpy(&drlData, &aex_drlSets[idxDRL], LIMITSET_LEN);
			}
		}
	}

DBG_Put_Text("TXN Limit");
DBG_Put_Hex(6, drlData.rdrtxnLimit);
DBG_Put_Text("CVM Limit");
DBG_Put_Hex(6, drlData.rdrcvmLimit);
DBG_Put_Text("FLR Limit");
DBG_Put_Hex(6, drlData.rdrFlrLimit);

	rspCode=UT_bcdcmp(glv_tag9F02.Value, drlData.rdrtxnLimit, ECL_LENGTH_9F02);
	if (rspCode == 1)
	{
		rspDRL|=AEX_DRL_RESULT_EXCEED_TRANSACTION_LIMIT;
	}

	rspCode=UT_bcdcmp(glv_tag9F02.Value, drlData.rdrcvmLimit, ECL_LENGTH_9F02);
	if (rspCode < 2)
	{
		rspDRL|=AEX_DRL_RESULT_REQUIRE_CVM;
	}

	rspCode=UT_bcdcmp(glv_tag9F02.Value, drlData.rdrFlrLimit, ECL_LENGTH_9F02);
	if (rspCode == 1)
	{
		rspDRL|=AEX_DRL_RESULT_EXCEED_FLOOR_LIMIT;
	}
	
	return rspDRL;
}


void AEX_Check_EffectiveAndExpirationDate(void)
{
	UINT	lenOfV=0;
	UCHAR	rspCode=0xFF;

	//Check Application Effective Date
	UT_Get_TLVLengthOfV(glv_tag5F25.Length, &lenOfV);
	if (lenOfV != 0)
	{
		rspCode=AEX_Compare_Date(glv_tag9A.Value, glv_tag5F25.Value);
		if (rspCode == 2)
		{
			glv_tag95.Value[1]|=AEX_TVR_APPLICATION_NOT_YET_EFFECTIVE;
		}
	}

	//Check Application Expiration Date
	UT_Get_TLVLengthOfV(glv_tag5F24.Length, &lenOfV);
	if (lenOfV != 0)
	{
		rspCode=AEX_Compare_Date(glv_tag9A.Value, glv_tag5F24.Value);
		if (rspCode == 1)
		{
			glv_tag95.Value[1]|=AEX_TVR_EXPIRED_APPLICATION;
		}
	}
}


UCHAR AEX_Check_EMVProcessingRestrictions(void)
{
	AEX_Check_ApplicationVersionNumber();

	AEX_Check_ApplicationUsageControl();

	AEX_Check_EffectiveAndExpirationDate();

	return SUCCESS;
}


UCHAR AEX_Check_ExceptionFile(void)
{
	UCHAR i=0;
	UCHAR j=0;
	UCHAR ascExpFile[ECL_LENGTH_5A*2];
	UCHAR ascPAN[ECL_LENGTH_5A*2];
	UCHAR lenOfAscPAN=0;
	UINT  lenOf5F34;
	UINT  lenOf5A;

	UT_Get_TLVLengthOfV(glv_tag5F34.Length, &lenOf5F34);
	UT_Get_TLVLengthOfV(glv_tag5A.Length, &lenOf5A);
	
	if ((lenOf5A == 0) || (lenOf5F34 == 0))
	{
		;	//Don't Check
	}
	else
	{
		memset(ascPAN, 'F', (ECL_LENGTH_5A*2));
		UT_Split(ascPAN, glv_tag5A.Value, lenOf5A);

		lenOfAscPAN=(ascPAN[lenOf5A*2-1] == 'F')?(lenOf5A*2-1):(lenOf5A*2);
		
		for (i=0; i < 10; i++)	//Number of Exception File
		{				
			UT_Split(ascExpFile, Exception_File[i], 10);

			for (j=0; j < (ECL_LENGTH_5A*2); j++)	//Length of PAN in Exception File
			{
				if (ascExpFile[j] == 'F')
				{
					break;			
				}
			}

			if ((!memcmp(ascPAN, ascExpFile, j)) && (lenOfAscPAN == j))
			{
				if ((lenOf5F34 == ECL_LENGTH_5F34) && 
					(glv_tag5F34.Value[0] == Exception_File[i][10]))
				{
					return FAIL;
				}
			}
		}
	}

	return SUCCESS;
}


UCHAR AEX_Check_GACResponseFormat(UINT iptLen, UCHAR *iptData)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	lenOfT_Template=0;
	UCHAR	lenOfL_Template=0;
	UINT	lenOfV_Template=0;
	UINT	lenOfPadding=0;
	UCHAR	*ptr9F26=NULLPTR;
	UCHAR	*ptr9F27=NULLPTR;
	UCHAR	*ptr9F36=NULLPTR;
	UCHAR	*ptr9F4B=NULLPTR;
	UCHAR	tag9F26[2]={0x9F,0x26};
	UCHAR	tag9F27[2]={0x9F,0x27};
	UCHAR	tag9F36[2]={0x9F,0x36};
	UCHAR	tag9F4B[2]={0x9F,0x4B};
	UCHAR	rsp9F27[1]={0};
	UCHAR	rspCode=FAIL;

	UT_Get_TLVLength(iptData, &lenOfT_Template, &lenOfL_Template, &lenOfV_Template);

	if (iptLen != (lenOfT_Template+lenOfL_Template+lenOfV_Template))
	{
		return FAIL;
	}

	if ((iptData[0] == 0x80) || (iptData[0] == 0x77))
	{
		ptr9F26=UT_Find_Tag(tag9F26, iptLen, iptData);
		ptr9F27=UT_Find_Tag(tag9F27, iptLen, iptData);
		ptr9F36=UT_Find_Tag(tag9F36, iptLen, iptData);
		
		if (iptData[0] == 0x80)
		{
			//Check Format 1
			if (lenOfV_Template < (ECL_LENGTH_9F27+ECL_LENGTH_9F36+ECL_LENGTH_9F26))
			{
				return FAIL;
			}

			if (ptr9F27 != NULLPTR)
			{
				rspCode=UT_Get_TLVLength(ptr9F27, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == SUCCESS)
				{
					return FAIL;
				}
			}

			if (ptr9F36 != NULLPTR)
			{
				rspCode=UT_Get_TLVLength(ptr9F36, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == SUCCESS)
				{
					return FAIL;
				}
			}

			if (ptr9F26 != NULLPTR)
			{
				rspCode=UT_Get_TLVLength(ptr9F26, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == SUCCESS)
				{
					return FAIL;
				}
			}

			if (((iptData[lenOfT_Template+lenOfL_Template]) == 0x00) ||
				((iptData[lenOfT_Template+lenOfL_Template]) == 0xFF))
			{
				UT_Check_Padding(lenOfV_Template, 0, &iptData[lenOfT_Template+lenOfL_Template], &lenOfPadding);

				if (lenOfPadding > 1)
				{
					return FAIL;
				}
			}

			if (aex_modODA == AEX_MODE_ODA_CDA)
			{
				rsp9F27[0]=iptData[lenOfT_Template+lenOfL_Template];
				if (((rsp9F27[0] & 0xC0) == AEX_ACT_TC) ||
					((rsp9F27[0] & 0xC0) == AEX_ACT_ARQC))
				{
					return FAIL;
				}
			}
		}
		else
		{
			//Check Format 2
			if (ptr9F27 == NULLPTR)
			{
				return FAIL;
			}

			UT_Get_TLVLength(ptr9F27, &lenOfT, &lenOfL, &lenOfV);
			if (lenOfV == ECL_LENGTH_9F27)
			{
				rsp9F27[0]=ptr9F27[lenOfT+lenOfL];
			}

			if (ptr9F36 == NULLPTR)
			{
				return FAIL;
			}

			if ((aex_modODA == AEX_MODE_ODA_CDA) &&
				(((rsp9F27[0] & 0xC0) == AEX_ACT_TC) || ((rsp9F27[0] & 0xC0) == AEX_ACT_ARQC)))
			{
				ptr9F4B=UT_Find_Tag(tag9F4B, iptLen, iptData);
				if (ptr9F4B == NULLPTR)
				{
					return FAIL;
				}
			}
			else
			{
				if (ptr9F26 == NULLPTR)
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


UCHAR AEX_Check_GPOResponseFormat(UINT iptLen, UCHAR *iptData)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	tag82[1]={0x82};
	UCHAR	tag94[1]={0x94};
	UCHAR	*ptr82=NULLPTR;
	UCHAR	*ptr94=NULLPTR;
	UCHAR	rspCode=FAIL;

	iptLen=iptLen;
	
DBG_Put_Text("1.7.1 Chk GPO");

	UT_Get_TLVLength(iptData, &lenOfT, &lenOfL, &lenOfV);
	if (lenOfV < 6)
	{
		return FAIL;
	}

	if ((iptData[0] == 0x80) || (iptData[0] == 0x77))
	{
		ptr82=UT_Find_Tag(tag82, lenOfV, &iptData[lenOfT+lenOfL]);
		ptr94=UT_Find_Tag(tag94, lenOfV, &iptData[lenOfT+lenOfL]);
		
		if (iptData[0] == 0x80)
		{
			
DBG_Put_Text("1.7.2 Format 1");

			UT_Get_TLVLength(iptData, &lenOfT, &lenOfL, &lenOfV);
			if (lenOfV < (2+4))	//AIP + AFL
			{

DBG_Put_Text("1.7.3 Min. Length Error");

				return FAIL;
			}

			if (((lenOfV-2) % 4) != 0)
			{

DBG_Put_Text("1.7.4 AFL Length Error");

				return FAIL;
			}

			rspCode=AEX_Check_AFL((lenOfV-2), &iptData[lenOfT+lenOfL+2]);
			if (rspCode == FAIL)
			{

DBG_Put_Text("1.7.5 Chk AFL Fail");

				return FAIL;
			}

			//Check TLV
			if (ptr82 != NULLPTR)
			{
				rspCode=UT_Get_TLVLength(ptr82, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == TRUE)
				{

DBG_Put_Text("1.7.6 AIP TLV Present");

					return FAIL;
				}
			}

			if (ptr94 != NULLPTR)
			{
				rspCode=UT_Get_TLVLength(ptr94, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == TRUE)
				{

DBG_Put_Text("1.7.7 AFL TLV Present");

					return FAIL;
				}
			}
		}
		else
		{
			
DBG_Put_Text("1.7.8 Format 2");

			if (ptr82 != NULLPTR)
			{

DBG_Put_Text("1.7.9 AIP Present");

				UT_Get_TLVLength(ptr82, &lenOfT, &lenOfL, &lenOfV);
				if (lenOfV != 2)
				{

DBG_Put_Text("1.7.10 AIP Length != 2");

					return FAIL;
				}
			}
			else
			{

DBG_Put_Text("1.7.11 AIP Missing");

				return FAIL;
			}

			if (ptr94 != NULLPTR)
			{

DBG_Put_Text("1.7.12 AFL Present");

				UT_Get_TLVLength(ptr94, &lenOfT, &lenOfL, &lenOfV);
				if ((lenOfV % 4) != 0)
				{

DBG_Put_Text("1.7.13 Not a Multiplier of 4");

					return FAIL;
				}

				rspCode=AEX_Check_AFL(lenOfV, &ptr94[lenOfT+lenOfL]);
				if (rspCode == FAIL)
				{

DBG_Put_Text("1.7.14 Chk AFL Fail");

					return FAIL;
				}
			}
			else
			{

DBG_Put_Text("1.7.15 AFL is Absent");

				return FAIL;
			}
		}
	}
	else
	{

DBG_Put_Text("1.7.16 Template Error");

		return FAIL;
	}

	return SUCCESS;
}


void AEX_Check_ODAKey(void)
{
	UCHAR	rspCode=FAIL;
	UINT	lenOfV=0;

	UT_Get_TLVLengthOfV(glv_tag8F.Length, &lenOfV);
	if (lenOfV == 0)
	{
		glv_tag95.Value[0]|=AEX_TVR_ICC_DATA_MISSING;
		glv_tag95.Value[0]|=AEX_TVR_CDA_FAILED;
	}
	else
	{
		rspCode=AEX_Retrieve_PK_CA(glv_tag9F06.Value, glv_tag8F.Value[0], &aex_sndData[0], &aex_sndData[4], &aex_sndData[1]);
		if (rspCode == FAIL)
		{
			glv_tag95.Value[0]|=AEX_TVR_CDA_FAILED;
		}
	}
}


UCHAR AEX_Check_PresenceOfTagList(UINT iptLen, UCHAR *iptData)
{
	UCHAR	lenOfT=0;
	UINT	lenOfV=0;
	UINT	idxTag=0;
	UCHAR	rspCode=FAIL;
	UCHAR	*ptrData=NULLPTR;

	ptrData=iptData;
	
	do
	{
		UT_Get_TLVLengthOfT(ptrData, &lenOfT);

		rspCode=UT_Search_Record(lenOfT, ptrData, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &idxTag);
		if (rspCode == FAIL)
		{
			return FAIL;
		}

		UT_Get_TLVLengthOfV((UCHAR*)glv_addTable[idxTag], &lenOfV);
		if (lenOfV == 0)
		{
			return FAIL;
		}

		ptrData+=lenOfT;
	} while ((ptrData-iptData) < iptLen);
	
	return SUCCESS;
}


UCHAR AEX_Check_ReadRecordDataFormat(void)
{
	UCHAR	rspCode=FALSE;
	UINT	lenOfV=0;

	UT_Get_TLVLengthOfV(glv_tag5F24.Length, &lenOfV);
	if (lenOfV > 0)
	{
		if (lenOfV != 3)
		{
			return FAIL;
		}
		
		rspCode=UT_CheckYearMonthDate(glv_tag5F24.Value);
		if (rspCode == FALSE)
		{
			return FAIL;
		}
	}

	UT_Get_TLVLengthOfV(glv_tag5F25.Length, &lenOfV);
	if (lenOfV > 0)
	{
		if (lenOfV != 3)
		{
			return FAIL;
		}
		
		rspCode=UT_CheckYearMonthDate(glv_tag5F25.Value);
		if (rspCode == FALSE)
		{
			return FAIL;
		}
	}

	UT_Get_TLVLengthOfV(glv_tag8E.Length, &lenOfV);
	if (lenOfV > 0)
	{
		if ((lenOfV < 8) || (lenOfV & 1))
		{			
			return FAIL;
		}
	}

	//Don't Save Merchant Category Code Returned from Card
	UT_Get_TLVLengthOfV(glv_tag9F15.Length, &lenOfV);
	if (lenOfV > 0)
	{
		if ((glv_par9F15[0] == 0) && (glv_par9F15[1] == 0))
		{
			UT_Set_TagLength(0, glv_tag9F15.Length);
		}
	}

	//Don't Save Merchant Name and Location Returned from Card
	UT_Get_TLVLengthOfV(glv_tag9F4E.Length, &lenOfV);
	if (lenOfV > 0)
	{
		if (glv_par9F4ELen == 0)
		{
			UT_Set_TagLength(0, glv_tag9F4E.Length);
		}
	}
	
	return SUCCESS;
}


UCHAR AEX_Check_ReadRecordMandatoryData(void)
{
	UCHAR	rspCode=FAIL;
	UCHAR	lstManTags[4]=
	{
		0x5A,		//Application Primary Account Number
		0x5F,0x24,	//Application Expiration Date
		0x8C		//CDOL1
	};
	UCHAR	lstManTags_Mag[5]=
	{
		0x5F,0x25,	//Application Effective Date
		0x5F,0x20,	//Cardholder Name
		0x57		//Track 2 Equivalent Data
	};

	rspCode=AEX_Check_PresenceOfTagList((UINT)sizeof(lstManTags), lstManTags);
	if (rspCode == FAIL)
	{
		return FAIL;
	}

	if (aex_modTransaction == AEX_MODE_TRANSACTION_MAGSTRIPE)
	{
		rspCode=AEX_Check_PresenceOfTagList((UINT)sizeof(lstManTags_Mag), lstManTags_Mag);
		if (rspCode == FAIL)
		{
			return FAIL;
		}
	}

	return SUCCESS;
}


UCHAR AEX_Check_SupplementaryProcessingRestrictions(void)
{
	UINT	lenOfV=0;
	UCHAR	flgDomestic=0xFF;
	UCHAR	rspCode=FALSE;
	UCHAR	lstConCode[4]=
	{
		0x5F,0x28,	//Issuer Country Code
		0x9F,0x1A	//Terminal Country Code
	};

	UT_Get_TLVLengthOfV(glv_tag9F70.Length, &lenOfV);
	if (lenOfV != 0)
	{
		if (aex_flgDelayedAuthorization == TRUE)
		{
			if (glv_tag9F70.Value[1] & AEX_CIPC_DELAYED_AUTHORIZATION_USAGE_INFORMATION_PRESENT)
			{
				rspCode=AEX_Check_PresenceOfTagList((UINT)sizeof(lstConCode), lstConCode);
				if (rspCode == SUCCESS)
				{
					rspCode=UT_bcdcmp(glv_tag9F1A.Value, glv_tag5F28.Value, 2);

					flgDomestic=(rspCode == 0)?(TRUE):(FALSE);
					if (flgDomestic == TRUE)
					{
						if ((glv_tag9F70.Value[1] & AEX_CIPC_VALID_AT_DOMESTIC_TERMINALS_PERFORMING_DELAYED_AUTHORIZATION) == 0)
						{
							glv_tag95.Value[1]|=AEX_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;
						}
					}
					else
					{
						if ((glv_tag9F70.Value[1] & AEX_CIPC_VALID_AT_INTERNATIONAL_TERMINALS_PERFORMING_DELAYED_AUTHORIZATION) == 0)
						{
							glv_tag95.Value[1]|=AEX_TVR_REQUESTED_SERVICE_NOT_ALLOWED_FOR_CARD_PRODUCT;
						}
					}
				}
			}
		}
	}
	
	return SUCCESS;
}


UCHAR AEX_Check_SupportForAlternativeInterface(void)
{
	UINT	lenOfV=0;

	UT_Get_TLVLengthOfV(glv_tag9F70.Length, &lenOfV);
	if (lenOfV != 0)
	{
		if ((glv_tag9F70.Value[0] & AEX_CIPC_CONTACT_EMV_INTERFACE_SUPPORTED) &&
			(glv_tag9F33.Value[0] & AEX_TRC_IC_WITH_CONTACTS))
		{
			return SUCCESS;
		}
	}
	else
	{
		return SUCCESS;	//Assume an alternative interface using contact EMV is supported by the Card
	}
	
	return FAIL;
}


void AEX_Clear_Parameter(void)
{
	aex_flgGoOnline=FALSE;
	aex_flgDelayedAuthorization=FALSE;
	
	aex_modTransaction=AEX_MODE_TRANSACTION_MAGSTRIPE;
	aex_modInterface=AEX_MODE_INTERFACE_CARD;
	aex_modODA=AEX_MODE_ODA_NONE;
	
	aex_typAC=AEX_ACT_AAC;
	
	aex_tag9F35_Modified=0;
}


UCHAR AEX_Compare_Date(UCHAR *iptDate1, UCHAR *iptDate2)
{
	UINT	Year1=0;
	UINT	Year2=0;
	UCHAR	Month1=0;
	UCHAR	Month2=0;
	UCHAR	Day1=0;
	UCHAR	Day2=0;
	UCHAR	tmpCentury=0;

	//Compare Year
	tmpCentury=UT_SetCentury(iptDate1[0]);
	Year1=tmpCentury*256+iptDate1[0];
	
	tmpCentury=UT_SetCentury(iptDate2[0]);
	Year2=tmpCentury*256+iptDate2[0];

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
		Month1=iptDate1[1];
		Month2=iptDate2[1];
		
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
			Day1=iptDate1[2];
			Day2=iptDate2[2];

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


UCHAR AEX_Copy_CardResponse_TLV(UINT iptLen, UCHAR *iptData)
{
	UCHAR	flgConTag=FALSE;
	UCHAR	flgEncError=FALSE;
	UCHAR	flgPrimitive=FALSE;
	UCHAR	flgInTagTable=FALSE;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfV_Table=0;
	UINT	lenInTerminal=0;
	UCHAR	rspCode=FAIL;
	UCHAR	*ptrData=NULLPTR;
	UINT	datLen=0;
	UINT	parLen=0;
	UINT	padLen=0;
	UINT	idxTag=0;
	

	//Point to Start of Data
	ptrData=iptData;

	rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == FAIL)
	{

DBG_Put_Text("Get TLV Length Error");

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

DBG_Put_Text("Input Length != TLV Length");

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

DBG_Put_Text("EMV Tag Encoding Error");

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

DBG_Put_Text("TLV Error");

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

DBG_Put_Text("Redundant Tag");

					return FAIL;	//Redundant
				}

				lenOfV_Table=glv_tagTable[idxTag].MASTER_MaxLength[0]*256+glv_tagTable[idxTag].MASTER_MaxLength[1];
				if (lenOfV <= lenOfV_Table)
				{
					//Save Card Data
					UT_Set_TagLength(lenOfV, (UCHAR*)glv_addTable[idxTag]);
					memcpy((char*)(glv_addTable[idxTag]+3), &ptrData[lenOfT+lenOfL], lenOfV);
				}
			}
		}

		ptrData+=(lenOfT+lenOfL+lenOfV);
		parLen+=(lenOfT+lenOfL+lenOfV);
	} while (parLen < datLen);

	return SUCCESS;
}


UCHAR AEX_Copy_CardResponse_GACFM1(UINT iptLen, UCHAR *iptData)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfV_Check=0;
	UINT	lenOf9F10=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=FALSE;

	//Get Template TLV
	rspCode=UT_Get_TLVLength(iptData, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == FAIL)
	{
		return FAIL;
	}

	ptrData=iptData+lenOfT+lenOfL;	//Point to Value of 80

	//Cryptogram Information Data
	UT_Get_TLVLengthOfV(glv_tag9F27.Length, &lenOfV_Check);
	if (lenOfV_Check != 0)
	{
		return FAIL;
	}

	UT_Set_TagLength(ECL_LENGTH_9F27, glv_tag9F27.Length);
	memcpy(glv_tag9F27.Value, ptrData, ECL_LENGTH_9F27);
	ptrData+=ECL_LENGTH_9F27;

	//Application Transaction Counter
	UT_Get_TLVLengthOfV(glv_tag9F36.Length, &lenOfV_Check);
	if (lenOfV_Check != 0)
	{
		return FAIL;
	}

	UT_Set_TagLength(ECL_LENGTH_9F36, glv_tag9F36.Length);
	memcpy(glv_tag9F36.Value, ptrData, ECL_LENGTH_9F36);
	ptrData+=ECL_LENGTH_9F36;

	//Application Cryptogram
	UT_Get_TLVLengthOfV(glv_tag9F26.Length, &lenOfV_Check);
	if (lenOfV_Check != 0)
	{
		return FAIL;
	}

	UT_Set_TagLength(ECL_LENGTH_9F26, glv_tag9F26.Length);
	memcpy(glv_tag9F26.Value, ptrData, ECL_LENGTH_9F26);
	ptrData+=ECL_LENGTH_9F26;

	//Issuer Application Data
	if (lenOfV > (ECL_LENGTH_9F27+ECL_LENGTH_9F36+ECL_LENGTH_9F26))
	{
		lenOf9F10=lenOfV-(ECL_LENGTH_9F27+ECL_LENGTH_9F36+ECL_LENGTH_9F26);
		if (lenOf9F10 <= ECL_LENGTH_9F10)
		{
			UT_Get_TLVLengthOfV(glv_tag9F10.Length, &lenOfV_Check);
			if (lenOfV_Check != 0)
			{
				return FAIL;
			}

			UT_Set_TagLength(lenOf9F10, glv_tag9F10.Length);
			memcpy(glv_tag9F10.Value, ptrData, lenOf9F10);
		}
		else
		{
			return FAIL;
		}
	}

	iptLen=iptLen;	//Remove Warning

	return SUCCESS;
}


UCHAR AEX_Copy_CardResponse_GPOFM1(UINT iptLen, UCHAR *iptData)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfV_Check=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=FALSE;

	//Get Template TLV
	rspCode=UT_Get_TLVLength(iptData, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == FAIL)
	{
		return FAIL;
	}

	ptrData=iptData+lenOfT+lenOfL;

	//Application Interchange Profile
	UT_Get_TLVLengthOfV(glv_tag82.Length, &lenOfV_Check);
	if (lenOfV_Check != 0)
	{
		return FAIL;
	}
	
	UT_Set_TagLength(2, glv_tag82.Length);
	memcpy(glv_tag82.Value, ptrData, 2);
	ptrData+=2;

	//Application File Locator
	UT_Get_TLVLengthOfV(glv_tag94.Length, &lenOfV_Check);
	if (lenOfV_Check != 0)
	{
		return FAIL;
	}
	
	if ((lenOfV-2) <= ECL_LENGTH_94)
	{
		UT_Set_TagLength((lenOfV-2), glv_tag94.Length);
		memcpy(glv_tag94.Value, ptrData, (lenOfV-2));
		ptrData+=(lenOfV-2);
	}

	iptLen=iptLen;	//Remove Warning
	
	return SUCCESS;
}


void AEX_Free_Buffer(void)
{
	free(aex_sndData);
	free(aex_rcvData);
	free(aex_cvrList);
}


UCHAR AEX_Get_Data(void)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	rspCode=0;
	UCHAR 	cmdGETDATA[5]={	
				0x80,	//CLA
				0xCA,	//INS
				0x00,	//P1
				0x00,	//P2
				0x00};	//Le

	//Command Header
	cmdGETDATA[2]=0x9F;
	cmdGETDATA[3]=0x36;

	//Send Command
	rspCode=ECL_LV1_DEP(5, cmdGETDATA, &aex_rcvLen, aex_rcvData, 1000);
	if (rspCode == ECL_LV1_SUCCESS)
	{
		//Check SW12
		rspCode=UT_Check_SW12(&aex_rcvData[aex_rcvLen-2], STATUSWORD_9000);
		if (rspCode == TRUE)
		{
			//Copy ATC
			UT_Get_TLVLength(aex_rcvData, &lenOfT, &lenOfL, &lenOfV);
			if (((lenOfT == 2) && (!memcmp(aex_rcvData, &cmdGETDATA[2], 2))) &&
				(lenOfV == ECL_LENGTH_9F36))
			{
				UT_Set_TagLength(lenOfV, glv_tag9F36.Length);
				memcpy(glv_tag9F36.Value, &aex_rcvData[lenOfT+lenOfL], lenOfV);
			}
			else
			{
				return FAIL;
			}
		}
		else
		{
			return FAIL;
		}
	}
	else
	{
		return FAIL;
	}

	return SUCCESS;
}


UCHAR AEX_Get_OfflineDataAuthenticationMode(void)
{
	if ((glv_tag82.Value[0] & AEX_AIP_CDA_SUPPORTED) &&
		(glv_tag9F33.Value[2] & AEX_TRC_CDA))
	{
		return AEX_MODE_ODA_CDA;
	}
	else
	{
		if ((glv_tag82.Value[0] & AEX_AIP_SDA_SUPPORTED) &&
			(glv_tag9F33.Value[2] & AEX_TRC_SDA))
		{
			return AEX_MODE_ODA_SDA;
		}
	}

	return AEX_MODE_ODA_NONE;
}


UCHAR AEX_Get_PseudoMagneticStripe(void)
{
	UINT	lenOfV=0;
	UCHAR	lenOfPadding=0;
	UCHAR	ascATC[10]={0};
	UCHAR	ascExpDate[4]={0};
	UCHAR	ascSerCode[4]={0};
	UCHAR	ascUnpNumber[4]={0};
	UCHAR	ascAC[10]={0};
	ULONG	intAC=0;

	memset(aex_pmsTrkOne, 0, AEX_PMS_LENGTH_TRACK_ONE);
	memset(aex_pmsTrkTwo, 0, AEX_PMS_LENGTH_TRACK_TWO);

	//PAN of Magstripe Mode Must be 15 Digits
	UT_Get_TLVLengthOfV(glv_tag5A.Length, &lenOfV);
	if ((lenOfV != 8) || ((glv_tag5A.Value[7] & 0x0F) != 0x0F))
	{
		return FAIL;
	}

	//Get ATC
	UT_INT2ASC((glv_tag9F36.Value[0]*256+glv_tag9F36.Value[1]), ascATC);

	//Get Application Expiration Date
	UT_Split(ascExpDate, glv_tag5F24.Value, 2);

	//Get Service Code
	UT_Split(ascSerCode, &glv_tag57.Value[10], 2);

	//Get Unpredictable Number
	UT_Split(ascUnpNumber, &glv_tag9F37.Value[2], 2);

	//Get Application Cryptogram
	UT_Get_TLVLengthOfV(glv_tag9F26.Length, &lenOfV);
	if (lenOfV != 0)
	{
		intAC=(glv_tag9F26.Value[5]<<16) | (glv_tag9F26.Value[6]<<8) | (glv_tag9F26.Value[7]);
		UT_INT2ASC(intAC, ascAC);
	}

	//Track 1
	//Start Sentinel
	aex_pmsTrkOne[AEX_PMS_OFFSET_TRACK_ONE_START_SENTINEL]='%';

	//Format Code
	aex_pmsTrkOne[AEX_PMS_OFFSET_TRACK_ONE_FORMAT_CODE]='B';

	//PAN
	UT_Split(&aex_pmsTrkOne[AEX_PMS_OFFSET_TRACK_ONE_PAN], glv_tag5A.Value, 8);

	//Field Separator 1
	aex_pmsTrkOne[AEX_PMS_OFFSET_TRACK_ONE_FIELD_SEPARATOR_1]='^';

	//Cardholder Name
	UT_Get_TLVLengthOfV(glv_tag5F20.Length, &lenOfV);
	(lenOfV >= 21)?(lenOfV=21):(lenOfPadding=21-lenOfV);
	memcpy(&aex_pmsTrkOne[AEX_PMS_OFFSET_TRACK_ONE_CARDHOLDER_NAME], glv_tag5F20.Value, lenOfV);

	if (lenOfPadding != 0)
	{
		memset(&aex_pmsTrkOne[AEX_PMS_OFFSET_TRACK_ONE_CARDHOLDER_NAME+lenOfV], ' ', lenOfPadding);
	}

	//ATC
	memcpy(&aex_pmsTrkOne[AEX_PMS_OFFSET_TRACK_ONE_ATC], &ascATC[5], AEX_PMS_LENGTH_TRACK_ONE_ATC);

	//Field Separator 2
	aex_pmsTrkOne[AEX_PMS_OFFSET_TRACK_ONE_FIELD_SEPARATOR_2]='^';

	//Application Expiration Date
	memcpy(&aex_pmsTrkOne[AEX_PMS_OFFSET_TRACK_ONE_APPLICATION_EXPIRATION_DATE], ascExpDate, AEX_PMS_LENGTH_TRACK_ONE_APPLICATION_EXPIRATION_DATE);

	//Service Code
	memcpy(&aex_pmsTrkOne[AEX_PMS_OFFSET_TRACK_ONE_SERVICE_CODE], ascSerCode, AEX_PMS_LENGTH_TRACK_ONE_SERVICE_CODE);

	//Unpredictable Number
	memcpy(&aex_pmsTrkOne[AEX_PMS_OFFSET_TRACK_ONE_UNPREDICTABLE_NUMBER], ascUnpNumber, AEX_PMS_LENGTH_TRACK_ONE_UNPREDICTABLE_NUMBER);
	
	//Cryptogram
	memcpy(&aex_pmsTrkOne[AEX_PMS_OFFSET_TRACK_ONE_CRYPTOGRAM], &ascAC[5], AEX_PMS_LENGTH_TRACK_ONE_CRYPTOGRAM);

	//End Sentinel
	aex_pmsTrkOne[AEX_PMS_OFFSET_TRACK_ONE_END_SENTINEL]='?';

	
	//Track 2
	//Start Sentinel
	aex_pmsTrkTwo[AEX_PMS_OFFSET_TRACK_TWO_START_SENTINEL]=';';
	
	//PAN
	UT_Split(&aex_pmsTrkTwo[AEX_PMS_OFFSET_TRACK_TWO_PAN], glv_tag5A.Value, 8);
	
	//Field Separator
	aex_pmsTrkTwo[AEX_PMS_OFFSET_TRACK_TWO_FIELD_SEPARATOR]='=';
	
	//Application Expiration Date
	memcpy(&aex_pmsTrkTwo[AEX_PMS_OFFSET_TRACK_TWO_APPLICATION_EXPIRATION_DATE], ascExpDate, AEX_PMS_LENGTH_TRACK_TWO_APPLICATION_EXPIRATION_DATE);
	
	//Service Code
	memcpy(&aex_pmsTrkTwo[AEX_PMS_OFFSET_TRACK_TWO_SERVICE_CODE], ascSerCode, AEX_PMS_LENGTH_TRACK_TWO_SERVICE_CODE);
	
	//Unpredictable Number
	memcpy(&aex_pmsTrkTwo[AEX_PMS_OFFSET_TRACK_TWO_UNPREDICTABLE_NUMBER], ascUnpNumber, AEX_PMS_LENGTH_TRACK_TWO_UNPREDICTABLE_NUMBER);
	
	//Cryptogram
	memcpy(&aex_pmsTrkTwo[AEX_PMS_OFFSET_TRACK_TWO_CRYPTOGRAM], &ascAC[5], AEX_PMS_LENGTH_TRACK_TWO_CRYPTOGRAM);
	
	//ATC
	memcpy(&aex_pmsTrkTwo[AEX_PMS_OFFSET_TRACK_TWO_ATC], &ascATC[5], AEX_PMS_LENGTH_TRACK_TWO_ATC);
	
	//End Sentinel
	aex_pmsTrkTwo[AEX_PMS_OFFSET_TRACK_TWO_END_SENTINEL]='?';
	
	return SUCCESS;
}


UCHAR AEX_Get_RandomNumber(UCHAR *optRndNumber)
{
	UCHAR	bufRandom[8]={0};
	UCHAR	rndFloor=0;
	UCHAR	idxTry=0;
	UCHAR	idxNumber=0;
	
	//Get Random Number
	rndFloor=255-255%(aex_rngUnpNumber+1);

	for (idxTry=0; idxTry < 8; idxTry++)
	{
		api_sys_random(bufRandom);

		for (idxNumber=0; idxNumber < 8; idxNumber++)
		{
			if (bufRandom[idxNumber] < rndFloor)
			{
				optRndNumber[0]=bufRandom[idxNumber]%(aex_rngUnpNumber+1);

				return SUCCESS;
			}
		}
	}

	return FAIL;
}


UCHAR AEX_Get_UnpredictableNumber(UCHAR *optUnpNumber)
{
	UINT	lenOfV=0;
	UCHAR	cntYear=0;
	UCHAR	cntMonth=0;
	UCHAR	rndMonth=0;
	UCHAR	rstYear=0;
	UCHAR	rstMonth=0;

	memset(optUnpNumber, 0, 4);
	
	//Get Application Effective Date
	UT_Get_TLVLengthOfV(glv_tag5F25.Length, &lenOfV);
	if (lenOfV == 0)
	{
		return FAIL;
	}

	cntYear=((glv_tag5F25.Value[0] & 0xF0)>>4)*10 + (glv_tag5F25.Value[0] & 0x0F);
	cntMonth=((glv_tag5F25.Value[1] & 0xF0)>>4)*10 + (glv_tag5F25.Value[1] & 0x0F);

	AEX_Get_RandomNumber(&rndMonth);
	AEX_Subtract_Date(cntYear, cntMonth, rndMonth, &rstYear, &rstMonth);

	optUnpNumber[2]=((rstYear/10)<<4) | (rstYear%10);
	optUnpNumber[3]=((rstMonth/10)<<4) | (rstMonth%10);
	
	return SUCCESS;
}


void AEX_Load_Configuration(void)
{
	//Terminal Capabilities
	UT_Set_TagLength(ECL_LENGTH_9F33, glv_tag9F33.Length);
	memcpy(glv_tag9F33.Value, &glv_par9F33, ECL_LENGTH_9F33);
	
	//Terminal Type
	UT_Set_TagLength(ECL_LENGTH_9F35, glv_tag9F35.Length);
	memcpy(glv_tag9F35.Value, &glv_par9F35[3], ECL_LENGTH_9F35);
	
	//Application Version Number (Reader)
	UT_Set_TagLength(ECL_LENGTH_9F09, glv_tag9F09.Length);
	memcpy(glv_tag9F09.Value, &glv_par9F09[3], ECL_LENGTH_9F09);

	//Terminal Action Code �VDefault
	UT_Set_TagLength(ECL_LENGTH_DF8120, glv_tagDF8120.Length);
	memcpy(glv_tagDF8120.Value, &glv_parDF8120[3], ECL_LENGTH_DF8120);

	//Terminal Action Code �VDenial
	UT_Set_TagLength(ECL_LENGTH_DF8121, glv_tagDF8121.Length);
	memcpy(glv_tagDF8121.Value, &glv_parDF8121[3], ECL_LENGTH_DF8121);

	//Terminal Action Code �VOnline
	UT_Set_TagLength(ECL_LENGTH_DF8122, glv_tagDF8122.Length);
	memcpy(glv_tagDF8122.Value, &glv_parDF8122[3], ECL_LENGTH_DF8122);

	//Contactless Reader Capabilities
	UT_Set_TagLength(ECL_LENGTH_9F6D, glv_tag9F6D.Length);
	memcpy(glv_tag9F6D.Value, &glv_par9F6D[3], 1);

	//Enhanced Contactless Reader Capabilities
	UT_Set_TagLength(ECL_LENGTH_9F6E, glv_tag9F6E.Length);
	memcpy(glv_tag9F6E.Value, &glv_par9F6E[3], 4);

	//Merchant Category Code
	UT_Set_TagLength(ECL_LENGTH_9F15, glv_tag9F15.Length);
	memcpy(glv_tag9F15.Value, &glv_par9F15, ECL_LENGTH_9F15);

	//Merchant Name and Location
	UT_Set_TagLength((UINT)glv_par9F4ELen, glv_tag9F4E.Length);
	memcpy(glv_tag9F4E.Value, &glv_par9F4E, glv_par9F4ELen);

	//Check Function Parameter
	if (aex_parFunction & AEX_FUNCTION_PARAMETER_GO_ONLINE)				aex_flgGoOnline=TRUE;
	if (aex_parFunction & AEX_FUNCTION_PARAMETER_DELAYED_AUTHORIZATION)	aex_flgDelayedAuthorization=TRUE;
}


void AEX_Patch_MagStripeTrack(void)
{
	AEX_Get_PseudoMagneticStripe();

	D1_Data_Length=AEX_PMS_LENGTH_TRACK_ONE;
	memcpy(D1_Data, aex_pmsTrkOne, AEX_PMS_LENGTH_TRACK_ONE);
	Tag57_Data_Length=AEX_PMS_LENGTH_TRACK_TWO;
	memcpy(Tag57_Data, aex_pmsTrkTwo, AEX_PMS_LENGTH_TRACK_TWO);
}


void AEX_Patch_OnlineData(void)
{
	UCHAR	rspCode = 0;
	UINT	lenTag=0;
	UINT	lenData=0;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	recIndex=0;
	UCHAR	*ptrTag=NULLPTR;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	tagData[]={	0x57,0x5A,0x5F,0x20,0x5F,0x24,0x5F,0x2A,0x5F,0x34,
						0x82,0x95,0x9A,0x9C,0x9F,0x02,0x9F,0x03,0x9F,0x06,
						0x9F,0x09,0x9F,0x0D,0x9F,0x0E,0x9F,0x0F,0x9F,0x10,
						0x9F,0x15,0x9F,0x1A,0x9F,0x26,0x9F,0x27,0x9F,0x33,
						0x9F,0x34,0x9F,0x35,0x9F,0x36,0x9F,0x37,0x9F,0x4E,
						0x9F,0x6D,0x9F,0x6E};

	Online_Data_Length=0;
	
	if (aex_flgDelayedAuthorization == TRUE)
	{
		Online_Data[Online_Data_Length++]=0xDF;
		Online_Data[Online_Data_Length++]=0x1F;
		Online_Data[Online_Data_Length++]=0x01;
		Online_Data[Online_Data_Length++]=0x01;
	}
	else
	{
		if ((glv_tag9F27.Value[0] & 0xC0) == AEX_ACT_TC)
		{
			Online_Data[Online_Data_Length++]=0xDF;
			Online_Data[Online_Data_Length++]=0x1F;
			Online_Data[Online_Data_Length++]=0x01;
			Online_Data[Online_Data_Length++]=0x00;
		}
		else	//AEX_ACT_ARQC
		{
			Online_Data[Online_Data_Length++]=0xDF;
			Online_Data[Online_Data_Length++]=0x0F;
			Online_Data[Online_Data_Length++]=0x01;
			Online_Data[Online_Data_Length++]=0x08;
		}
	}	

	
	lenData=0;
	ptrData=&Online_Data[Online_Data_Length];
	lenTag=sizeof(tagData);
	ptrTag=tagData;
	
	while (lenTag > 0)
	{
		rspCode=UT_Get_TLVLengthOfT(ptrTag, &lenOfT);

		rspCode=UT_Search_Record(lenOfT, ptrTag, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
		if (rspCode == SUCCESS)
		{
			UT_Get_TLVLengthOfL((UCHAR*)glv_addTable[recIndex], &lenOfL);
			UT_Get_TLVLengthOfV((UCHAR*)glv_addTable[recIndex], &lenOfV);

			memcpy(ptrData, ptrTag, lenOfT);
			memcpy((ptrData+lenOfT), glv_addTable[recIndex], lenOfL);
			memcpy((ptrData+lenOfT+lenOfL), glv_addTable[recIndex]+3, lenOfV);

			lenData+=(lenOfT+lenOfL+lenOfV);
			ptrData+=(lenOfT+lenOfL+lenOfV);
		}

		lenTag-=lenOfT;
		ptrTag+=lenOfT;
	};

	Online_Data_Length+=lenData;
}


UCHAR AEX_Perform_CVM_CheckNoCVMRequiredValid(UINT iptLen, UCHAR *iptData, UCHAR *optIndex)
{
	UINT	idxNumber=0;
	UCHAR	rspCode=FALSE;

	for (idxNumber=0; idxNumber < (iptLen/2); idxNumber++)
	{
		if ((iptData[idxNumber*2] & 0xBF) == AEX_CVM_METHOD_NO_CVM_REQUIRED)
		{
			rspCode=AEX_Check_CVM_ConditionIsSatisfied(iptData[idxNumber*2+1]);
			if (rspCode == TRUE)
			{
				optIndex[0]=idxNumber;
				
				return SUCCESS;
			}
		}
	}
	
	return FAIL;
}


void AEX_Perform_CVM_CreateCVMList(void)
{
//	UINT	idxNumber=0;
	UINT	lenOfV=0;
	UCHAR	*ptrData=NULLPTR;
		
	aex_cvrLen=0;
	aex_cvrIndex=0;
	UT_Get_TLVLengthOfV(glv_tag8E.Length, &lenOfV);
	ptrData=&glv_tag8E.Value[8];

//Check Support List in AEX_Perform_CVM_ListProcessing
/*	for (idxNumber=0; idxNumber < ((lenOfV-8)/2); idxNumber++, ptrData+=2)
	{
		if ((ptrData[0] & 0xBF) == AEX_CVM_METHOD_NO_CVM_REQUIRED)
		{
			if (glv_tag9F6E.Value[2] & AEX_ECRC_CVM_REQUIRED)
			{
				continue;	//Skip CVR
			}
		}

		if ((ptrData[0] & 0xBF) == AEX_CVM_METHOD_ENCIPHERED_PIN_VERIFIED_ONLINE)
		{
			if ((glv_tag9F6E.Value[1] & AEX_ECRC_ONLINE_PIN_SUPPORTED) == 0	)
			{
				glv_tag95.Value[2]|=AEX_TVR_PIN_ENTRY_REQUIRED_AND_PIN_PAD_NOT_PRESENT_OR_NOT_WORKING;

				continue;	//Skip CVR
			}
		}

		memcpy(&aex_cvrList[aex_cvrLen], ptrData, 2);
		aex_cvrLen+=2;
	}
*/
aex_cvrLen=lenOfV-8;
memcpy(aex_cvrList, ptrData, aex_cvrLen);

}


UCHAR AEX_Perform_CVM_LimitNotExceeded(void)
{

DBG_Put_Text("5.4 CVM Limit Not Exceeded");

	if (aex_modInterface == AEX_MODE_INTERFACE_MOBILE)
	{
		return AEX_CVM_RESULT_CVM_LIMIT_NOT_EXCEEDED_MOBILE;
	}

	return AEX_CVM_RESULT_CVM_LIMIT_NOT_EXCEEDED_CARD;
}


UCHAR AEX_Perform_CVM_LimitNotExceeded_Mobile(void)
{
	UINT	lenOfV=0;
	UCHAR	rspCode=FAIL;
	UCHAR	lstIndex=0;

DBG_Put_Text("5.4.1 CVM Limit Not Exceeded for Mobile");

	UT_Get_TLVLengthOfV(glv_tag9F71.Length, &lenOfV);
	if (lenOfV != 0)
	{
		if (glv_tag9F71.Value[0] == AEX_MCR_MOBILE_CVM_PERFORMED)
		{
			if (glv_tag9F71.Value[2] == AEX_MCR_MOBILE_CVM_FAILED)
			{
				if (aex_flgRestart == FALSE)
				{
					return AEX_CVM_RESULT_TRY_AGAIN;
				}
			}
		}
	}

	if (glv_tag82.Value[0] & AEX_AIP_CARDHOLDER_VERIFICATION_SUPPORTED)
	{
		UT_Get_TLVLengthOfV(glv_tag8E.Length, &lenOfV);
		
		if ((lenOfV == 0) || (lenOfV == 8))	//No CVM List or Rules
		{
			glv_tag95.Value[0]|=AEX_TVR_ICC_DATA_MISSING;
		}
		else
		{
			rspCode=AEX_Perform_CVM_CheckNoCVMRequiredValid((lenOfV-8), (glv_tag8E.Value+8), &lstIndex);
			if (rspCode == SUCCESS)
			{
				AEX_Set_CVMResults(glv_tag8E.Value[8+lstIndex*2], glv_tag8E.Value[8+lstIndex*2+1], 0x02);
			}
			else
			{
				AEX_Perform_CVM_CreateCVMList();
				
				return AEX_CVM_RESULT_CVM_LIST_PROCESSING;
			}
		}
	}
	
	return AEX_CVM_RESULT_GO_TO_TERMINAL_RISK_MANAGEMENT;
}


UCHAR AEX_Perform_CVM_LimitNotExceeded_Card(void)
{
	UINT	lenOfV=0;
	UCHAR	rspCode=FAIL;
	UCHAR	lstIndex=0;

DBG_Put_Text("5.4.2 CVM Limit Not Exceeded for Card");

	if (glv_tag82.Value[0] & AEX_AIP_CARDHOLDER_VERIFICATION_SUPPORTED)
	{
		UT_Get_TLVLengthOfV(glv_tag8E.Length, &lenOfV);
		
		if ((lenOfV == 0) || (lenOfV == 8))	//No CVM List or Rules
		{
			glv_tag95.Value[0]|=AEX_TVR_ICC_DATA_MISSING;
		}
		else
		{
			rspCode=AEX_Perform_CVM_CheckNoCVMRequiredValid((lenOfV-8), (glv_tag8E.Value+8), &lstIndex);
			if (rspCode == SUCCESS)
			{
				AEX_Set_CVMResults(glv_tag8E.Value[8+lstIndex*2], glv_tag8E.Value[8+lstIndex*2+1], 0x02);
			}
			else
			{
				AEX_Perform_CVM_CreateCVMList();
				
				return AEX_CVM_RESULT_CVM_LIST_PROCESSING;
			}
		}
	}
	
	return AEX_CVM_RESULT_GO_TO_TERMINAL_RISK_MANAGEMENT;
}


UCHAR AEX_Perform_CVM_ListProcessing(void)
{
	UCHAR	rspCode=FAIL;
	UCHAR	defResult[3]={0x3F, 0x00, 0x00};	//Default Result

DBG_Put_Text("5.2 CVM List Processing");

	for (; aex_cvrIndex < (aex_cvrLen/2); aex_cvrIndex++)
	{
		rspCode=AEX_Check_CVM_ConditionIsSatisfied(aex_cvrList[aex_cvrIndex*2+1]);
		if (rspCode == TRUE)
		{
			rspCode=AEX_Check_CVM_MethodIsRecognized(aex_cvrList[aex_cvrIndex*2]);
			if (rspCode == TRUE)
			{
				rspCode=AEX_Check_CVM_MethodIsSupported(aex_cvrList[aex_cvrIndex*2]);
				if (rspCode == TRUE)
				{
					AEX_Set_CVMResults(aex_cvrList[aex_cvrIndex*2], aex_cvrList[aex_cvrIndex*2+1], 0x00);	//Set Result Later
					
					switch (aex_cvrList[aex_cvrIndex*2] & 0xBF)
					{
						case AEX_CVM_METHOD_PLAINTEXT_PIN_VERIFICATION_PERFORMED_BY_ICC:	return AEX_CVM_RESULT_MOBILE_CVM;
						case AEX_CVM_METHOD_ENCIPHERED_PIN_VERIFIED_ONLINE:					return AEX_CVM_RESULT_ONLINE_PIN;
						case AEX_CVM_METHOD_SIGNATURE_PAPER:								return AEX_CVM_RESULT_SIGNATURE;
						case AEX_CVM_METHOD_NO_CVM_REQUIRED:								return AEX_CVM_RESULT_NO_CVM_REQUIRED;
					}
				}

				if ((aex_cvrList[aex_cvrIndex*2] & 0xBF) == AEX_CVM_METHOD_FAIL_CVM_PROCESSING)
				{
					AEX_Set_CVMResults(aex_cvrList[aex_cvrIndex*2], aex_cvrList[aex_cvrIndex*2+1], 0x01);
						
					break;
				}
			}
			else
			{
				glv_tag95.Value[2]|=AEX_TVR_UNRECOGNISED_CVM;
			}

//This Process Fails Some CVM Test Cases
//			if ((aex_cvrList[aex_cvrIndex*2] & 0x40) == 0x00)	//Fail cardholder verification if this CVM is unsuccessful
//			{
//				break;
//			}
		}
	}
	
	if (!memcmp(glv_tag9F34.Value, defResult, 3))
	{
		AEX_Set_CVMResults(0x3F, 0x00, 0x01);
	}

	return AEX_CVM_RESULT_CARDHOLDER_VERIFICATION_UNABLE_TO_COMPLETE;
}


UCHAR AEX_Perform_CVM_MobileCVMProcessing(void)
{
	UINT	lenOfV=0;
	UCHAR	flgUnsuccessful=FALSE;
	UCHAR	flgChkRestart=TRUE;

DBG_Put_Text("5.2.2 CVM Mobile CVM Processing");

	if (glv_tag9F6E.Value[1] & AEX_ECRC_MOBILE_CVM_SUPPORTED)
	{
		UT_Get_TLVLengthOfV(glv_tag9F71.Length, &lenOfV);
		if (lenOfV != 0)
		{
			if (glv_tag9F71.Value[2] == AEX_MCR_MOBILE_CVM_BLOCKED)
			{
				flgUnsuccessful=TRUE;
				flgChkRestart=FALSE;
			}
			else
			{
				if (glv_tag9F71.Value[0] == AEX_MCR_NO_CVM_PERFORMED)
				{
					flgUnsuccessful=TRUE;
				}
				else
				{
					if (glv_tag9F71.Value[0] == AEX_MCR_MOBILE_CVM_PERFORMED)
					{
						if (glv_tag9F71.Value[2] == AEX_MCR_MOBILE_CVM_SUCCESSFUL)
						{
							glv_tag9F34.Value[2]=0x02;
						}
						else
						{
							flgUnsuccessful=TRUE;
						}
					}
					else
					{
						flgUnsuccessful=TRUE;
					}
				}
			}
		}
		else
		{
			flgUnsuccessful=TRUE;
			flgChkRestart=FALSE;
		}
	}
	else
	{
		return AEX_CVM_RESULT_CVM_LIST_PROCESSING;
	}

	if (flgUnsuccessful == TRUE)
	{
		glv_tag9F34.Value[2]=0x01;
		
		if (flgChkRestart == TRUE)
		{
			if (aex_flgRestart == FALSE)
			{
				return AEX_CVM_RESULT_TRY_AGAIN;
			}
		}
		
		return AEX_CVM_RESULT_CVM_LIST_PROCESSING;
	}
			
	return AEX_CVM_RESULT_GO_TO_TERMINAL_RISK_MANAGEMENT;
}


void AEX_Perform_CVM_OnlinePIN(void)
{

DBG_Put_Text("5.2.1 CVM Online PIN");

	glv_tag95.Value[2]|=AEX_TVR_ONLINE_PIN_ENTERED;

	glv_tag9F34.Value[2]=0x00;
}


UCHAR AEX_Perform_CVM_Processing(void)
{
	UINT	lenOfV=0;

DBG_Put_Text("5.1 CVM Processing");

	UT_Get_TLVLengthOfV(glv_tag8E.Length, &lenOfV);

	if ((lenOfV == 0) || (lenOfV == 8))	//No CVM List or Rules
	{
		glv_tag95.Value[0]|=AEX_TVR_ICC_DATA_MISSING;

		return AEX_CVM_RESULT_GO_TO_TERMINAL_RISK_MANAGEMENT;
	}

	AEX_Perform_CVM_CreateCVMList();
	
	return AEX_CVM_RESULT_CVM_LIST_PROCESSING;
}


UCHAR AEX_Perform_CVM_UnableToComplete(void)
{
	UCHAR	rspCode=FAIL;

DBG_Put_Text("5.3 CVM Unable To Complete");

	glv_tag95.Value[2]|=AEX_TVR_CARDHOLDER_VERIFICATION_WAS_NOT_SUCCESSFUL;

	if ((aex_modTransaction == AEX_MODE_TRANSACTION_EMV) &&
		(glv_tag9F33.Value[0] & AEX_TRC_IC_WITH_CONTACTS))
	{
		rspCode=AEX_Check_SupportForAlternativeInterface();			
		if (rspCode == SUCCESS)
		{
			return AEX_CVM_RESULT_TRY_ANOTHER_INTERFACE;
		}
	}

	return AEX_CVM_RESULT_GO_TO_TERMINAL_RISK_MANAGEMENT;
}


void AEX_Reset_ReceiveBuffer(void)
{
	aex_rcvLen=0;
	memset(aex_rcvData, 0, ETP_BUFFER_SIZE);
}


UCHAR AEX_Retrieve_PK_CA(
	UCHAR	*iptRID, 
	UCHAR	iptIndex, 
	UCHAR	*optModLen,
	UCHAR	*optModulus,
	UCHAR	*optExponent)
{
	UCHAR	idxNum=0;

DBG_Put_Text("Retrieve CA PK");
	
	//[Book 2 5.2/6.2]
	for (idxNum=0; idxNum < CAPK_NUMBER; idxNum++)
	{
		if (!memcmp(glv_CAPK[idxNum].RID, iptRID, 5))
		{
			if (glv_CAPK[idxNum].Index == iptIndex)
			{
				optModLen[0]=glv_CAPK[idxNum].Length;
				memcpy(optModulus, glv_CAPK[idxNum].Modulus, glv_CAPK[idxNum].Length);
				memcpy(optExponent, glv_CAPK[idxNum].Exponent, 3);
				
				return SUCCESS;
			}
		}
	}

	return FAIL;
}


UCHAR AEX_Retrieve_PK_Issuer(
	UCHAR	iptModLen, 
	UCHAR	*iptModulus, 
	UCHAR	*iptExponent, 
	UCHAR	*optModLen, 
	UCHAR	*optModulus,
	UCHAR	*optExponent)
{
	UCHAR	rspCode=FALSE;
	UINT	lenOfV=0;
	UINT	lenOfIPKC=0;				//Length of Issuer Public Key Certificate
	UINT	lenOfIPKR=0;				//Length of Issuer Public Key Remainder
	UINT	lenOfIPKL=0;				//Length of Issuer Public Key Length
	UINT	lenOfIPKE=0;				//Length of Issuer Public Key Exponent
	UINT	lenOfLMD=0;					//Length of Leftmost Digits of the Issuer Public Key
	UINT	lenOfHash=0;				//Length of Hash Data
	UCHAR	bufCertificate[2+256]={0};	//Buffer of Certificate [Len(2)+Data(256)]
	UCHAR	bufHash[512]={0};			//Buffer of Hash
	UCHAR	bufModulus[2+256]={0};		//Buffer of Modulus
	UCHAR	rstHash[20]={0};			//Result of Hash


DBG_Put_Text("Retrieve Issuer PK");

	//[Book 2 5.3/6.3]
	//Step 1: Check Issuer Public Key Certificate Length = CAPK Modulus Length
	UT_Get_TLVLengthOfV(glv_tag90.Length, &lenOfV);
	lenOfIPKC=lenOfV;

	if (lenOfIPKC == 0)
	{
		glv_tag95.Value[0]|=AEX_TVR_ICC_DATA_MISSING;
	}
	
	if (iptModLen != lenOfIPKC)
	{
		return FAIL;
	}

DBG_Put_Text("Step 1");

	//Step 2: Recover Certificate and Check Trailer = 0xBC
	bufModulus[0]=iptModLen;
	memcpy(&bufModulus[2], iptModulus, iptModLen);

	rspCode=api_rsa_loadkey(bufModulus, iptExponent);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	UT_S2C(lenOfIPKC, &bufCertificate[0]);
	memcpy(&bufCertificate[2], glv_tag90.Value, lenOfIPKC);

	rspCode=api_rsa_recover(bufCertificate, bufCertificate);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	AEX_Check_api_rsa_recover(bufCertificate[0], &bufCertificate[2]);

	if (bufCertificate[2+lenOfIPKC-1] != 0xBC)	//Decrease 1 to Point to Trailer
	{
		return FAIL;
	}

DBG_Put_Text("Step 2");

	//Step 3: Check Header = 0x6A
	if (bufCertificate[2+0] != 0x6A)
	{
		return FAIL;
	}

DBG_Put_Text("Step 3");

	//Step 4: Check Certificate Format = 0x02
	if (bufCertificate[2+1] != 0x02)	//Add 1 to Point to Certificate Format
	{
		return FAIL;
	}

DBG_Put_Text("Step 4");

	//Step 5: Concatenation
	lenOfHash=lenOfIPKC-(1+20+1);						//Header(1)+Hash Result(20)+Trailer(1)
	memcpy(bufHash, &bufCertificate[2+1], lenOfHash);	//Add 1 to Point to Certificate Format

	lenOfIPKL=bufCertificate[2+13];
	lenOfLMD=lenOfIPKC-36;

	UT_Get_TLVLengthOfV(glv_tag92.Length, &lenOfV);
	lenOfIPKR=lenOfV;

	if (lenOfIPKR != 0)
	{
		memcpy(&bufHash[lenOfHash], glv_tag92.Value, lenOfIPKR);
		lenOfHash+=lenOfIPKR;
	}
	else
	{
		if (lenOfIPKL > lenOfLMD)
		{
			glv_tag95.Value[0]|=AEX_TVR_ICC_DATA_MISSING;
			
			return FAIL;
		}
	}

	UT_Get_TLVLengthOfV(glv_tag9F32.Length, &lenOfV);
	lenOfIPKE=lenOfV;

	if (lenOfIPKE == 0)
	{
		glv_tag95.Value[0]|=AEX_TVR_ICC_DATA_MISSING;
		
		return FAIL;
	}

	memcpy(&bufHash[lenOfHash], glv_tag9F32.Value, lenOfIPKE);
	lenOfHash+=lenOfIPKE;

DBG_Put_Text("Step 5");

	//Step 6: Apply Hash Algorithm
	rspCode=api_sys_SHA1(lenOfHash, bufHash, rstHash);
	if (rspCode == apiFailed)
	{
		return FAIL;
	}

DBG_Put_Text("Step 6");

	//Step 7: Compare Hash Result
	rspCode=UT_bcdcmp(&bufCertificate[2+lenOfIPKC-(20+1)], rstHash, 20); //Hash Result(20)+Trailer(1)
	if (rspCode != 0)
	{
		return FAIL;
	}

DBG_Put_Text("Step 7");

	//Step 8: Verify the Issuer Identifier = PAN
	rspCode=UT_CNcmp(&bufCertificate[2+2], glv_tag5A.Value, 4);	//Add 2 to Point to Issuer Identifier
	if (rspCode == FALSE)
	{
		return FAIL;
	}

DBG_Put_Text("Step 8");

	//Step 9: Verify Certificate Expiration Date
	rspCode=UT_VerifyCertificateExpDate(&bufCertificate[2+6]);	//Add 6 to Point to Certificate Expiration Date
	if (rspCode == FALSE)
	{
		return FAIL;
	}

DBG_Put_Text("Step 9");

	//Step 10: Verify Certification Revocation List
	rspCode=AEX_Check_CRL(glv_tag9F06.Value, glv_tag8F.Value[0], &bufCertificate[2+8]);	//Add 8 to Point to Certificate Serial Number
	if (rspCode == TRUE)
	{
		return FAIL;
	}

DBG_Put_Text("Step 10");

	//Step 11: Check Issuer Public Key Algorithm Indicator = 0x01
	if (bufCertificate[2+12] != 0x01)	//Add 12 to Point to Issuer Public Key Algorithm Indicator
	{
		return FAIL;
	}

DBG_Put_Text("Step 11");

	//Step 12: Concatenate Issuer Public Key Modulus
	memcpy(optModulus, &bufCertificate[2+15], lenOfLMD);	//Add 15 to Point to Issuer Public Key or Leftmost Digits of the Issuer Public Key

	if (lenOfIPKR != 0)
	{
		memcpy(&optModulus[lenOfLMD], glv_tag92.Value, lenOfIPKR);
	}

	optModLen[0]=lenOfIPKL;

	if (lenOfIPKE == 1)
	{
		memset(optExponent, 0, 3);
		optExponent[2]=glv_tag9F32.Value[0];
	}
	else if (lenOfIPKE == 3)
	{
		memcpy(optExponent, glv_tag9F32.Value, 3);
	}

DBG_Put_Text("Step 12");

	return SUCCESS;
}


UCHAR AEX_Retrieve_PK_ICC(
	UCHAR	iptModLen, 
	UCHAR	*iptModulus, 
	UCHAR	*iptExponent, 
	UCHAR	*optModLen, 
	UCHAR	*optModulus, 
	UCHAR	*optExponent)
{
	UCHAR	rspCode=FALSE;
	UINT	lenOfV=0;
	UINT	lenOfIPKC=0;				//Length of ICC Public Key Certificate
	UINT	lenOfIPKR=0;				//Length of ICC Public Key Remainder
	UINT	lenOfIPKL=0;				//Length of ICC Public Key Length
	UINT	lenOfIPKE=0;				//Length of ICC Public Key Exponent
	UINT	lenOfLMD=0;					//Length of Leftmost Digits of the ICC Public Key
	UINT	lenOfHash=0;				//Length of Hash Data
	UINT	lenOfSDTBA=0;				//Length of Static Data To Be Authenticate
	UCHAR	bufCertificate[2+256]={0};	//Buffer of Certificate [Len(2)+Data(256)]
	UCHAR	bufHash[512+2048]={0};		//Buffer of Hash (2048 for SDTBA)			//Hash Buffer
	UCHAR	bufModulus[2+256]={0};		//Buffer of Modulus
	UCHAR	bufPAN[10]={0};				//Buffer of PAN
	UCHAR	rstHash[20]={0};			//Result of Hash


DBG_Put_Text("Retrieve ICC PK");

	//[Book 2 6.4]
	//Step 1: Check ICC Public Key Certificate Length = Issuer Public Key Modulus Length
	UT_Get_TLVLengthOfV(glv_tag9F46.Length, &lenOfV);
	lenOfIPKC=lenOfV;

	if (lenOfIPKC == 0)
	{
		glv_tag95.Value[0]|=AEX_TVR_ICC_DATA_MISSING;
	}

	if (lenOfIPKC != iptModLen)
	{
		return FAIL;
	}

DBG_Put_Text("Step 1");

	//Step 2: Recover Certificate and Check Trailer = 0xBC
	bufModulus[0]=iptModLen;
	memcpy(&bufModulus[2], iptModulus, iptModLen);

	rspCode=api_rsa_loadkey(bufModulus, iptExponent);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	bufCertificate[0]=lenOfIPKC & 0x00FF;
	bufCertificate[1]=(lenOfIPKC & 0xFF00) >> 8;
	memcpy(&bufCertificate[2], glv_tag9F46.Value, lenOfIPKC);

	rspCode=api_rsa_recover(bufCertificate, bufCertificate);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	AEX_Check_api_rsa_recover(bufCertificate[0], &bufCertificate[2]);

	if (bufCertificate[2+lenOfIPKC-1] != 0xBC)	//Decrease 1 to Point to Trailer
	{
		return FAIL;
	}

DBG_Put_Text("Step 2");

	//Step 3: Check Header = 0x6A
	if (bufCertificate[2+0] != 0x6A)
	{
		return FAIL;
	}

DBG_Put_Text("Step 3");

	//Step 4: Check Certificate Format = 0x04
	if (bufCertificate[2+1] != 0x04)	//Add 1 to Point to Certificate Format
	{
		return FAIL;
	}

DBG_Put_Text("Step 4");

	//Step 5: Concatenation
	lenOfHash=lenOfIPKC-(1+20+1);						//Header(1)+Hash Result(20)+Trailer(1)
	memcpy(bufHash, &bufCertificate[2+1], lenOfHash);	//Add 1 to Point to Certificate Format

	lenOfIPKL=bufCertificate[2+19];
	lenOfLMD=lenOfIPKC-42;

	UT_Get_TLVLengthOfV(glv_tag9F48.Length, &lenOfV);
	lenOfIPKR=lenOfV;

	if (lenOfIPKR != 0)
	{
		memcpy(&bufHash[lenOfHash], glv_tag9F48.Value, lenOfIPKR);
		lenOfHash+=lenOfIPKR;
	}
	else
	{
		if (lenOfIPKL > lenOfLMD)
		{
			return FAIL;
		}
	}

	UT_Get_TLVLengthOfV(glv_tag9F47.Length, &lenOfV);
	lenOfIPKE=lenOfV;

	if (lenOfIPKE == 0)
	{
		glv_tag95.Value[0]|=AEX_TVR_ICC_DATA_MISSING;
		
		return FAIL;
	}
	else
	{
		memcpy(&bufHash[lenOfHash], glv_tag9F47.Value, lenOfIPKE);
		lenOfHash+=lenOfIPKE;
	}

	lenOfSDTBA=UT_C2S(&oda_bufRecord[0]);
	if (lenOfSDTBA != 0)
	{
		if ((lenOfHash+lenOfSDTBA) > sizeof(bufHash))
		{
			return FAIL;
		}

		memcpy(&bufHash[lenOfHash], &oda_bufRecord[2], lenOfSDTBA);
		lenOfHash+=lenOfSDTBA;
	}

DBG_Put_Text("Step 5");

	//Step 6: Apply Hash Algorithm
	rspCode=api_sys_SHA1(lenOfHash, bufHash, rstHash);
	if (rspCode == apiFailed)
	{
		return FAIL;
	}

DBG_Put_Text("Step 6");

	//Step 7: Compare Hash Result
	rspCode=UT_bcdcmp(&bufCertificate[2+lenOfIPKC-(20+1)], rstHash, 20); //Hash Result(20)+Trailer(1)
	if (rspCode != 0)
	{
		return FAIL;
	}

DBG_Put_Text("Step 7");

	//Step 8: Compare PAN
	memset(bufPAN, 0xFF, 10);
	UT_Get_TLVLengthOfV(glv_tag5A.Length, &lenOfV);
	memcpy(bufPAN, glv_tag5A.Value, lenOfV);

	rspCode=UT_CNcmp2(&bufCertificate[2+2], bufPAN, 10);	//Add 2 to Point to Application PAN
	if (rspCode == FALSE)
	{
		return FAIL;
	}

DBG_Put_Text("Step 8");

	//Step 9: Verify Certificate Expiration Date
	rspCode=UT_VerifyCertificateExpDate(&bufCertificate[2+12]);	//Add 12 to Point to Certificate Expiration Date
	if (rspCode == FALSE)
	{
		return FAIL;
	}

DBG_Put_Text("Step 9");

	//Step 10: Check ICC Public Key Algorithm Indicator = 0x01
	if (bufCertificate[2+18] != 0x01)	//Add 18 to Point to ICC Public Key Algorithm Indicator
	{
		return FAIL;
	}

DBG_Put_Text("Step 10");

	//Step 11: Concatenate ICC Public Key Modulus
	memcpy(optModulus, &bufCertificate[2+21], lenOfLMD);	//Add 21 to Point to ICC Public Key or Leftmost Digits of the Issuer Public Key

	if (lenOfIPKR != 0)
	{
		memcpy(&optModulus[lenOfLMD], glv_tag9F48.Value, lenOfIPKR);
	}

	optModLen[0]=lenOfIPKL;

	if (lenOfIPKE == 1)
	{
		memset(optExponent, 0, 3);
		optExponent[2]=glv_tag9F47.Value[0];
	}
	else if (lenOfIPKE == 3)
	{
		memcpy(optExponent, glv_tag9F47.Value, 3);
	}

DBG_Put_Text("Step 11");

	return SUCCESS;
}


UCHAR AEX_Save_ODARecord(UCHAR iptSFI, UINT iptLen, UCHAR *iptData)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfRecord=0;
	UCHAR	*ptrData=NULLPTR;
	
	lenOfRecord=UT_C2S(&oda_bufRecord[0]);

	if (iptSFI <= 10)
	{
		UT_Get_TLVLength(iptData, &lenOfT, &lenOfL, &lenOfV);
		ptrData=&iptData[lenOfT+lenOfL];
	}
	else
	{
		lenOfV=iptLen;
		ptrData=&iptData[0];
	}

	if ((lenOfRecord+lenOfV) > (ODA_BUFFER_SIZE_RECORD-2))
	{
		return FAIL;
	}

	UT_S2C((lenOfRecord+lenOfV), &oda_bufRecord[0]);
	memcpy(&oda_bufRecord[2+lenOfRecord], ptrData, lenOfV);
	
	return SUCCESS;
}


UCHAR AEX_Save_ODARecord_StaticDataAuthenticationTagList(void)
{
	UINT	lenOfV=0;
	
	UT_Get_TLVLengthOfV(glv_tag9F4A.Length, &lenOfV);
	if (lenOfV != 0)
	{
		if ((lenOfV == 1) && (glv_tag9F4A.Value[0] == 0x82))
		{
			lenOfV=UT_C2S(&oda_bufRecord[0]);
			if ((lenOfV+2) <= (ODA_BUFFER_SIZE_RECORD-2))
			{
				UT_S2C((lenOfV+2), &oda_bufRecord[0]);
				memcpy(&oda_bufRecord[2+lenOfV], glv_tag82.Value, 2);

				return SUCCESS;
			}
		}

		glv_tag95.Value[0]|=AEX_TVR_SDA_FAILED;
	}

	return FAIL;
}


void AEX_Set_CVMResults(UCHAR iptMethod, UCHAR iptCondition, UCHAR iptResult)
{
	glv_tag9F34.Value[0]=iptMethod;
	glv_tag9F34.Value[1]=iptCondition;
	glv_tag9F34.Value[2]=iptResult;
}


void AEX_Subtract_Date(UCHAR iptYear, UCHAR iptMonth, UCHAR iptSubMonth, UCHAR *optYear, UCHAR *optMonth)
{
	UCHAR	subYear=0;
	UCHAR	subMonth=0;

	subYear=iptSubMonth/12;
	subMonth=iptSubMonth%12;

	if (subMonth < iptMonth)
	{
		AEX_Subtract_Year(iptYear, subYear, optYear);
		optMonth[0]=iptMonth-subMonth;
	}
	else
	{
		AEX_Subtract_Year(iptYear, (subYear+1), optYear);
		optMonth[0]=(12+iptMonth)-subMonth;
	}
}


void AEX_Subtract_Year(UCHAR iptYear, UCHAR iptSubYear, UCHAR *optYear)
{
	optYear[0]=(iptSubYear <= iptYear)?(iptYear-iptSubYear):((100+iptYear)-iptSubYear);
}


UCHAR AEX_Verify_DynamicSignature(
	UCHAR	iptModLen,
	UCHAR	*iptModulus,
	UCHAR	*iptExponent)
{
	UCHAR	rspCode=FALSE;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfTag77=0;
	UINT	lenOfRemainder=0;
	UINT	lenOfData=0;
	UINT	lenOfHash=0;			//Length of Hash Data
	UINT	lenOfRmvPadding=0;		//Length of Padding Removed Buffer
	UINT	lenOfSDAD=0;			//Length of SDAD
	UCHAR	lenOfIDN=0;				//Length of ICC Dynamic Number
	UCHAR	bufSDAD[2+256]={0};		//Buffer of SDAD [Len(2)+Data(256)]
	UCHAR	bufHash[2048]={0};		//Buffer of Hash
	UCHAR	bufRmvPadding[512]={0};	//Buffer of Padding Removed
	UCHAR	bufModulus[2+256]={0};	//Buffer of Modulus
	UCHAR	rstHash[20]={0};		//Result of Hash	
	UCHAR	*ptrStart=NULLPTR;
	UCHAR	*ptrSDAD=NULLPTR;
	UCHAR	*ptrAftSDAD=NULLPTR;
	UCHAR	tag9F4B[2]={0x9F,0x4B};
	

DBG_Put_Text("Verify SDAD");

	//[Book 2 6.6.2]
	//Step 1: Check Signed Dynamic Application Data Length = ICC Public Key Modulus Length
	UT_Get_TLVLengthOfV(glv_tag9F4B.Length, &lenOfV);
	lenOfSDAD=lenOfV;
	
	if (lenOfSDAD != iptModLen)
	{
		return FAIL;
	}

DBG_Put_Text("Step 1");

	//Step 2: Recover SDAD and Check Trailer = 0xBC
	bufModulus[0]=iptModLen;
	memcpy(&bufModulus[2], iptModulus, iptModLen);
	rspCode=api_rsa_loadkey(bufModulus, iptExponent);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	UT_S2C(lenOfSDAD, &bufSDAD[0]);
	memcpy(&bufSDAD[2], glv_tag9F4B.Value, lenOfSDAD);

	rspCode=api_rsa_recover(bufSDAD, bufSDAD);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	AEX_Check_api_rsa_recover(bufSDAD[0], &bufSDAD[2]);

	if (bufSDAD[2+lenOfSDAD-1] != 0xBC)	//Decrease 1 to Point to Trailer
	{
		return FAIL;
	}

DBG_Put_Text("Step 2");

	//Step 3: Check Header = 0x6A
	if (bufSDAD[2+0] != 0x6A)
	{
		return FAIL;
	}

DBG_Put_Text("Step 3");

	//Step 4: Check Signed Data Format = 0x05
	if (bufSDAD[2+1] != 0x05)	//Add 1 to Point to Signed Data Format
	{
		return FAIL;
	}

	if (bufSDAD[2+2] != 0x01)	//Add 2 to Point to Hash Algorithm Indicator
	{
		return FAIL;
	}

DBG_Put_Text("Step 4");

	//Step 5: Retrieve ICC Dynamic Data
	lenOfIDN=bufSDAD[2+4];	//Add 4 to Point to ICC Dynamic Number Length

DBG_Put_Text("Step 5");

	//Step 6: Check CID retrieved from the ICC Dynamic Data = CID obtained from the response to the GENERATE AC command
	if (bufSDAD[2+(5+lenOfIDN)] != glv_tag9F27.Value[0])	//Add (5+lenOfIDNL) to Point to Cryptogram Information Data
	{
		return FAIL;
	}

DBG_Put_Text("Step 6");

	//Step 7: Concatenation
	lenOfHash=lenOfSDAD-(1+20+1);				//Header(1)+Hash Result(20)+Trailer(1)
	memcpy(bufHash, &bufSDAD[2+1], lenOfHash);	//Start from Signed Data Format

	memcpy(&bufHash[lenOfHash], glv_tag9F37.Value, 4);
	lenOfHash+=4;

DBG_Put_Text("Step 7");

	//Step 8: Apply Hash Algorithm
	rspCode=api_sys_SHA1(lenOfHash, bufHash, rstHash);
	if (rspCode == apiFailed)
	{
		return FAIL;
	}

DBG_Put_Text("Step 8");

	//Step 9: Compare Hash Result
	rspCode=UT_bcdcmp(&bufSDAD[2+lenOfSDAD-(20+1)], rstHash, 20); //Hash Result(20)+Trailer(1)
	if (rspCode != 0)
	{
		return FAIL;
	}

DBG_Put_Text("Step 9");

	//Step 10: Concatenation
	memset(bufHash, 0, sizeof(bufHash));
	lenOfHash=0;

	//Copy PDOL Related Data
	UT_Get_TLVLengthOfV(glv_tagDF8111.Length, &lenOfV);
	memcpy(bufHash, glv_tagDF8111.Value, lenOfV);
	lenOfHash+=lenOfV;

	//Copy CDOL1 Related Data
	UT_Get_TLVLengthOfV(glv_tagDF8107.Length, &lenOfV);
	memcpy(&bufHash[lenOfHash], glv_tagDF8107.Value, lenOfV);
	lenOfHash+=lenOfV;

	//Get Generate AC Response Record TLV Length
	UT_Get_TLVLength(oda_bufRspGAC, &lenOfT, &lenOfL, &lenOfV);
	lenOfTag77=lenOfV;
	ptrStart=oda_bufRspGAC+(lenOfT+lenOfL);	//Point to TLV-V

	//Exclude Signed Dynamic Application Data
	ptrSDAD=UT_Find_Tag(tag9F4B, lenOfV, ptrStart);
	if (ptrSDAD != NULLPTR)
	{
		lenOfData=ptrSDAD-ptrStart;
		if (lenOfData != 0)
		{
			rspCode=UT_Remove_PaddingData((ptrSDAD-ptrStart), ptrStart, &lenOfRmvPadding, bufRmvPadding);
			if (rspCode == FAIL)
			{
				return FAIL;
			}

			memcpy(&bufHash[lenOfHash], bufRmvPadding, lenOfRmvPadding);
			lenOfHash+=lenOfRmvPadding;
		}

		UT_Get_TLVLength(ptrSDAD, &lenOfT, &lenOfL, &lenOfV);
		ptrAftSDAD=ptrSDAD+(lenOfT+lenOfL+lenOfV);	//Skip SDAD TLV

		lenOfRemainder=lenOfTag77-(lenOfData+(lenOfT+lenOfL+lenOfV));	//Remainder Length = Total - (Data Before SDAD + SDAD)
		if (lenOfRemainder != 0)	
		{
			rspCode=UT_Remove_PaddingData(lenOfRemainder, ptrAftSDAD, &lenOfRmvPadding, bufRmvPadding);
			if (rspCode == FAIL)
			{
				return FAIL;
			}
			
			memcpy(&bufHash[lenOfHash], bufRmvPadding, lenOfRmvPadding);
			lenOfHash+=lenOfRmvPadding;
		}
	}
	else
	{
		return FAIL;
	}
	
DBG_Put_Text("Step 10");

	//Step 11: Apply Hash Algorithm
	memset(rstHash, 0, 20);
	rspCode=api_sys_SHA1(lenOfHash, bufHash, rstHash);
	if (rspCode == apiFailed)
	{
		return FAIL;
	}

DBG_Put_Text("Step 11");

	//Step 12: Compare Transaction Data Hash Code
	rspCode=UT_bcdcmp(&bufSDAD[2+5+lenOfIDN+9], rstHash, 20);	//Add 14+lenOfIDNL to Point to Transaction Data Hash Code
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

DBG_Put_Text("Step 12");

	return SUCCESS;
}


UCHAR AEX_Verify_SignedStaticApplicationData(
	UCHAR	iptModLen,
	UCHAR	*iptModulus,
	UCHAR	*iptExponent)
{
	UCHAR	rspCode=apiFailed;
	UINT	lenOfV=0;
	UINT	lenOfSSAD=0;			//Length of Signed Static Application Data
	UINT	lenOfSDTBA=0;			//Length of Static Data To Be Authenticate
	UINT	lenOfHash=0;			//Length of Hash Data
	UCHAR	bufSSAD[2+256]={0};		//Buffer of Signed Static Application Data Length [Len(2)+Data(256)]
	UCHAR	bufHash[512+2048]={0};	//Buffer of Hash (2048 for SDTBA)
	UCHAR	bufModulus[2+256]={0};	//Buffer of Modulus
	UCHAR	rstHash[20]={0};		//Result of Hash

DBG_Put_Text("Verify SSAD");

	//[Book 2 5.4]
	//Step 1: Check Signed Static Application Data Length = Issuer Public Key Modulus Length
	UT_Get_TLVLengthOfV(glv_tag93.Length, &lenOfV);
	lenOfSSAD=lenOfV;

	if (lenOfSSAD == 0)
	{
		glv_tag95.Value[0]|=AEX_TVR_ICC_DATA_MISSING;
	}
	
	if (iptModLen != lenOfSSAD)
	{
		return FAIL;
	}

DBG_Put_Text("Step 1");

	//Step 2: Recover SSAD and Check Trailer = 0xBC
	bufModulus[0]=iptModLen;
	memcpy(&bufModulus[2], iptModulus, iptModLen);
	rspCode=api_rsa_loadkey(bufModulus, iptExponent);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	UT_S2C(lenOfSSAD, &bufSSAD[0]);
	memcpy(&bufSSAD[2], glv_tag93.Value, lenOfSSAD);

	rspCode=api_rsa_recover(bufSSAD, bufSSAD);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	AEX_Check_api_rsa_recover(bufSSAD[0], &bufSSAD[2]);

	if (bufSSAD[2+lenOfSSAD-1] != 0xBC)	//Decrease 1 to Point to Trailer
	{
		return FAIL;
	}

DBG_Put_Text("Step 2");

	//Step 3: Check Header = 0x6A
	if (bufSSAD[2+0] != 0x6A)
	{
		return FAIL;
	}

DBG_Put_Text("Step 3");

	//Step 4: Check Signed Data Format = 0x03
	if (bufSSAD[2+1] != 0x03)	//Add 1 to Point to Signed Data Format
	{
		return FAIL;
	}

DBG_Put_Text("Step 4");

	//Step 5: Concatenation
	lenOfHash=lenOfSSAD-(1+20+1);				//Header(1)+Hash Result(20)+Trailer(1)
	memcpy(bufHash, &bufSSAD[2+1], lenOfHash);	//Add 1 to Point to Certificate Format

	lenOfSDTBA=UT_C2S(&oda_bufRecord[0]);
	if (lenOfSDTBA != 0)
	{
		if ((lenOfHash+lenOfSDTBA) > sizeof(bufHash))
		{
			return FAIL;
		}

		memcpy(&bufHash[lenOfHash], &oda_bufRecord[2], lenOfSDTBA);
		lenOfHash+=lenOfSDTBA;

		UT_Get_TLVLengthOfV(glv_tag9F4A.Length, &lenOfV);
		if (lenOfV != 0)
		{
			if ((lenOfV != 1) || (glv_tag9F4A.Value[0] != 0x82))
			{
				return FAIL;
			}
		}
	}

DBG_Put_Text("Step 5");

	//Step 6: Apply Hash Algorithm
	rspCode=api_sys_SHA1(lenOfHash, bufHash, rstHash);
	if (rspCode == apiFailed)
	{
		return FAIL;
	}

DBG_Put_Text("Step 6");

	//Step 7: Compare Hash Result
	rspCode=UT_bcdcmp(&bufSSAD[2+lenOfSSAD-(20+1)], rstHash, 20); //Hash Result(20)+Trailer(1)
	if (rspCode != 0)
	{
		return FAIL;
	}

DBG_Put_Text("Step 7");

	UT_Set_TagLength(2, glv_tag9F45.Length);
	memcpy(glv_tag9F45.Value, &bufSSAD[2+1+1+1], 2);

	return SUCCESS;
}


UCHAR AEX_InitiateApplication(void)
{
	UCHAR	tag9F35[2]={0x9F,0x35};
	UCHAR	tag9F38[2]={0x9F,0x38};
	UCHAR	tag9F6E[2]={0x9F,0x6E};
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=0;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	lenOfDolData=0;
	UCHAR	tag9F35_Backup=0;
	UCHAR	flg9F35=FALSE;
	UCHAR	flg9F6E=FALSE;
	UCHAR	flgModify=FALSE;
	UCHAR	rdrCRL_MAX[6]={0x99,0x99,0x99,0x99,0x99,0x99};
	UCHAR	rdrCRL[6]={0x00,0x00,0x00,0x00,0x00,0x00};

DBG_Put_Text("1 Initiate Application");

	//Pre-PDOL Processing
	UT_Set_TagLength(1, glv_tag9F6D.Length);
	glv_tag9F6D.Value[0]&=0xF7;

	UT_Set_TagLength(4, glv_tag9F6E.Length);
	glv_tag9F6E.Value[2]=0x00;

	UT_Set_TagLength(ECL_LENGTH_95, glv_tag95.Length);

	if (etp_etpConfig[3].pstRdrCRL == TRUE)	//CRL is Present
	{
		if (glv_parDF01Len[3] == 0)	//CRL is not Defined
		{
			memcpy(rdrCRL, rdrCRL_MAX, 6);
		}
		else
		{
			memcpy(rdrCRL, etp_etpConfig[3].rdrCRL, 6);
		}

		rspCode=UT_bcdcmp(glv_tag9F02.Value, rdrCRL, 6);
		if (rspCode < 2)
		{
			glv_tag9F6D.Value[0]|=AEX_CRC_CVM_REQUIRED;
			glv_tag9F6E.Value[2]|=AEX_ECRC_CVM_REQUIRED;
		}
	}

	if ((((glv_tag9F35.Value[0] & 0x0F) == 0x03) || ((glv_tag9F35.Value[0] & 0x0F) == 0x06)) ||
		(aex_flgGoOnline == FALSE))
	{
		glv_tag9F6E.Value[2]|=AEX_ECRC_TERMINAL_IS_OFFLINE_ONLY;
	}

	aex_tag9F35_Modified=glv_tag9F35.Value[0] | glv_tag9F6D.Value[0];

	//Handle Entry Point Processing Tags
	UT_Get_TLVLengthOfV(glv_tag9F06.Length, &lenOfV);
	UT_Set_TagLength(lenOfV, glv_tag84.Length);
	memcpy(glv_tag84.Value, glv_tag9F06.Value, lenOfV);
	UT_Set_TagLength(0, glv_tag9F66.Length);

DBG_Put_Text("1.1 PDOL Processing");

	//PDOL Processing
	ptrData=UT_Find_Tag(tag9F38, aex_rcvLen, aex_rcvData);
	if (ptrData != NULLPTR)
	{

DBG_Put_Text("1.2 PDOL Found");

		rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		UT_Set_TagLength(lenOfV, glv_tag9F38.Length);
		memcpy(glv_tag9F38.Value, &ptrData[lenOfT+lenOfL], lenOfV);
		
		//Find 9F35
		ptrData=UT_Find_TagInDOL(tag9F35, lenOfV, glv_tag9F38.Value);
		if (ptrData != NULLPTR)
		{
			flg9F35=TRUE;
		}

		//Find 9F6E
		ptrData=UT_Find_TagInDOL(tag9F6E, lenOfV, glv_tag9F38.Value);
		if (ptrData != NULLPTR)
		{
			flg9F6E=TRUE;
		}

		//Modify Terminal Type if 9F6E is Absent in PDOL
		if ((flg9F35 == TRUE) && (flg9F6E == FALSE))
		{
			tag9F35_Backup=glv_tag9F35.Value[0];
			glv_tag9F35.Value[0]=aex_tag9F35_Modified;

			flgModify=TRUE;
		}

DBG_Put_Text("1.3 Patch PDOL");

		//Patch PDOL Related Data
		rspCode=AEX_DOL_Get_DOLData(lenOfV, glv_tag9F38.Value, &lenOfDolData, aex_sndData);
		aex_sndLen=(UINT)lenOfDolData;

		if (rspCode == SUCCESS)
		{
			//Save PDOL Related Data
			UT_Get_TLVLength(aex_sndData, &lenOfT, &lenOfL, &lenOfV);
			UT_Set_TagLength(lenOfV, glv_tagDF8111.Length);
			memcpy(glv_tagDF8111.Value, &aex_sndData[lenOfT+lenOfL], lenOfV);

			//Recover Terminal Type
			if (flgModify == TRUE)
			{
				glv_tag9F35.Value[0]=tag9F35_Backup;
			}
		}
		else
		{
			return AEX_RESULT_TERMINATE;
		}
	}
	else
	{

DBG_Put_Text("1.4 PDOL is not Present");

		aex_sndLen=2;
		aex_sndData[0]=0x83;
		aex_sndData[1]=0x00;
	}

DBG_Put_Text("1.5 Send GPO");

	//Send GPO
	AEX_Reset_ReceiveBuffer();
	rspCode=ECL_APDU_Get_ProcessingOptions(aex_sndLen, aex_sndData, &aex_rcvLen, aex_rcvData);
	if (rspCode != ECL_LV1_SUCCESS)
	{
		return AEX_RESULT_TERMINATE;
	}

DBG_Put_Text("1.6 Check SW12");

	//Check SW12
	rspCode=UT_Check_SW12(&aex_rcvData[aex_rcvLen-2], STATUSWORD_9000);
	if (rspCode == FALSE)
	{
		return AEX_RESULT_TERMINATE;
	}

DBG_Put_Text("1.7 Check GPO Response");

	//Check Response Format
	rspCode=AEX_Check_GPOResponseFormat((aex_rcvLen-2), aex_rcvData);
	if (rspCode == FALSE)
	{
		return AEX_RESULT_TERMINATE;
	}

DBG_Put_Text("1.8 Copy Card Response");

	//Copy GPO Response
	if (aex_rcvData[0] == 0x80)
	{
		rspCode=AEX_Copy_CardResponse_GPOFM1((aex_rcvLen-2), aex_rcvData);
	}
	else
	{
		rspCode=AEX_Copy_CardResponse_TLV((aex_rcvLen-2), aex_rcvData);
	}

	if (rspCode == FALSE)
	{
		return AEX_RESULT_TERMINATE;
	}

	//Check Transaction Mode
	if ((glv_tag9F6D.Value[0] & AEX_CRC_EMV_AND_MAGSTRIPE) &&
		(glv_tag82.Value[1] & AEX_AIP_EMV_AND_MAGSTRIPE_MODES_SUPPORTED))
	{

DBG_Put_Text("1.9 EMV Mode");

		aex_modTransaction=AEX_MODE_TRANSACTION_EMV;
	}

	//Check Interface
	if (glv_tag82.Value[1] & AEX_AIP_EXPRESSPAY_MOBILE_SUPPORTED)
	{

DBG_Put_Text("1.10 Mobile Interface");
	
		aex_modInterface=AEX_MODE_INTERFACE_MOBILE;

		UT_Get_TLVLengthOfV(glv_tag9F71.Length, &lenOfV);

		if (lenOfV != 0)
		{
			if (glv_tag9F71.Value[2] == AEX_MCR_MOBILE_CVM_BLOCKED)
			{
				glv_tag95.Value[2]|=AEX_TVR_PIN_TRY_LIMIT_EXCEEDED;	//Mobile CVM Try Limit Exceeded
			}
		}
	}

	//Check ODA Mode
	aex_modODA=AEX_Get_OfflineDataAuthenticationMode();
	if (aex_modODA == AEX_MODE_ODA_NONE)
	{
		glv_tag95.Value[0]|=AEX_TVR_OFFLINE_DATA_AUTHENTICATION_WAS_NOT_PERFORMED;
	}

DBG_Put_Text("1.11 ODA Mode");
DBG_Put_UCHAR(aex_modODA);

	return AEX_RESULT_SUCCESS;
}


UCHAR AEX_ReadApplicationData(void)
{
	UINT	lenOfAFL=0;
	UCHAR	idxAFL=0;
	UCHAR	idxRecord=0;
	UCHAR	recSFI=0;
	UCHAR	recStart=0;
	UCHAR	recEnd=0;
	UCHAR	recODA=0;
	UCHAR	rspCode=FAIL;

DBG_Put_Text("2 Read Record");
UCHAR msgRec[2]={0};

	UT_Get_TLVLengthOfV(glv_tag94.Length, &lenOfAFL);

	for (idxAFL=0; idxAFL < (lenOfAFL/4); idxAFL++)
	{
		recSFI=glv_tag94.Value[idxAFL*4]>>3;
		recStart=glv_tag94.Value[idxAFL*4+1];
		recEnd=glv_tag94.Value[idxAFL*4+2];
		recODA=glv_tag94.Value[idxAFL*4+3];

		for (idxRecord=recStart; idxRecord <= recEnd; idxRecord++)
		{

DBG_Put_Text("2.1 Send Read Record");
DBG_Put_Text("RecNo/SFI");
msgRec[0]=idxRecord;
msgRec[1]=recSFI;
DBG_Put_Hex(2, msgRec);

			AEX_Reset_ReceiveBuffer();
			rspCode=ECL_APDU_Read_Record(idxRecord, recSFI, &aex_rcvLen, aex_rcvData);
			if (rspCode == ECL_LV1_SUCCESS)
			{

DBG_Put_Text("2.2 Read Success");
//DBG_Put_Hex(aex_rcvLen, aex_rcvData);

DBG_Put_Text("2.3 Check SW");

				rspCode=UT_Check_SW12(&aex_rcvData[aex_rcvLen-2], STATUSWORD_9000);
				if (rspCode != TRUE)
				{
					return AEX_RESULT_TERMINATE;
				}

DBG_Put_Text("2.4 Copy Record");

				rspCode=AEX_Copy_CardResponse_TLV((aex_rcvLen-2), aex_rcvData);
				if (rspCode == FAIL)
				{
					return AEX_RESULT_TERMINATE;
				}

				if (recODA != 0)
				{

DBG_Put_Text("2.5 Save ODA Rec");

					rspCode=AEX_Save_ODARecord(recSFI, (aex_rcvLen-2), aex_rcvData);
					if (rspCode == FAIL)
					{
						return AEX_RESULT_TERMINATE;
					}

					recODA--;
				}
			}
			else
			{

DBG_Put_Text("2.6 Read Fail");

				return AEX_RESULT_TERMINATE;
			}
		}
	}

DBG_Put_Text("2.7 Save SDA Tag List");

	//Patch SDA Tag List
	rspCode=AEX_Save_ODARecord_StaticDataAuthenticationTagList();

DBG_Put_Text("2.8 Check Mandatory Data");

	//Check Mandatory Tag
	rspCode=AEX_Check_ReadRecordMandatoryData();
	if (rspCode == FAIL)
	{
		return AEX_RESULT_TERMINATE;
	}

DBG_Put_Text("2.9 Check Data Format");

	//Check Data Format
	rspCode=AEX_Check_ReadRecordDataFormat();
	if (rspCode == FAIL)
	{
		return AEX_RESULT_TERMINATE;
	}

DBG_Put_Text("2.10 Check ODA Key");

	//Check ODA Key
	if (aex_modODA != AEX_MODE_ODA_NONE)
	{
		AEX_Check_ODAKey();
	}

	//Get Data
	if (aex_modTransaction == AEX_MODE_TRANSACTION_MAGSTRIPE)
	{

DBG_Put_Text("2.11 Get Data");

		rspCode=AEX_Get_Data();
		if (rspCode == FAIL)
		{
			return AEX_RESULT_TERMINATE;
		}
	}

	return AEX_RESULT_SUCCESS;
}


UCHAR AEX_OfflineDataAuthentication(void)
{
	UCHAR	rspCode=AEX_RESULT_SUCCESS;

DBG_Put_Text("3. Offline Data Authentication");

	if (aex_modODA == AEX_MODE_ODA_SDA)
	{
		rspCode=AEX_OfflineDataAuthentication_SDA();
	}

	return AEX_RESULT_SUCCESS;
}


UCHAR AEX_OfflineDataAuthentication_CDA(void)
{
	UCHAR	cauModLen=0;
	UCHAR	issModLen=0;
	UCHAR	iccModLen=0;
	UCHAR	cauModulus[255]={0};
	UCHAR	issModulus[255]={0};
	UCHAR	iccModulus[255]={0};
	UCHAR	cauExponent[3]={0};
	UCHAR	issExponent[3]={0};
	UCHAR	iccExponent[3]={0};
	UCHAR	rspCode=FAIL;

DBG_Put_Text("3.2 OfflineDataAuthentication CDA");

	rspCode=AEX_Retrieve_PK_CA(glv_tag9F06.Value, glv_tag8F.Value[0], &cauModLen, cauModulus, cauExponent);
	if (rspCode == SUCCESS)
	{
		rspCode=AEX_Retrieve_PK_Issuer(cauModLen, cauModulus, cauExponent, &issModLen, issModulus, issExponent);
		if (rspCode == SUCCESS)
		{
			rspCode=AEX_Retrieve_PK_ICC(issModLen, issModulus, issExponent, &iccModLen, iccModulus, iccExponent);
			if (rspCode == SUCCESS)
			{
				rspCode=AEX_Verify_DynamicSignature(iccModLen, iccModulus, iccExponent);
				if (rspCode == SUCCESS)
				{
					return SUCCESS;
				}
			}
		}
	}

	glv_tag95.Value[0]|=AEX_TVR_CDA_FAILED;

	return FAIL;
}


UCHAR AEX_OfflineDataAuthentication_SDA(void)
{
	UCHAR	cauModLen=0;
	UCHAR	issModLen=0;
	UCHAR	cauModulus[255]={0};
	UCHAR	issModulus[255]={0};
	UCHAR	cauExponent[3]={0};
	UCHAR	issExponent[3]={0};	
	UCHAR	rspCode=FAIL;

DBG_Put_Text("3.1 OfflineDataAuthentication SDA");

	glv_tag95.Value[0]|=AEX_TVR_SDA_SELECTED;

	rspCode=AEX_Retrieve_PK_CA(glv_tag9F06.Value, glv_tag8F.Value[0], &cauModLen, cauModulus, cauExponent);
	if (rspCode == SUCCESS)
	{
		rspCode=AEX_Retrieve_PK_Issuer(cauModLen, cauModulus, cauExponent, &issModLen, issModulus, issExponent);
		if (rspCode == SUCCESS)
		{
			rspCode=AEX_Verify_SignedStaticApplicationData(issModLen, issModulus, issExponent);
			if (rspCode == SUCCESS)
			{
				return SUCCESS;
			}
		}
	}

	glv_tag95.Value[0]|=AEX_TVR_SDA_FAILED;

	return FAIL;
}


UCHAR AEX_ProcessingRestrictions(void)
{
	UCHAR	rspCode=FAIL;
	UCHAR	flgAltInterface=FALSE;

DBG_Put_Text("4 Processing Restrictions");

	//Determine Support for Alternative Interface
	rspCode=AEX_Check_SupportForAlternativeInterface();
	if (rspCode == SUCCESS)
	{
		flgAltInterface=TRUE;
	}

DBG_Put_Text("4.1 DRL Process");

	//Apply Dynamic Reader Limits
	aex_rstDRL=AEX_Check_DynamicReaderLimits();

DBG_Put_Text("Result:");
DBG_Put_UCHAR(aex_rstDRL);

	if (aex_rstDRL & AEX_DRL_RESULT_EXCEED_TRANSACTION_LIMIT)
	{
		if ((flgAltInterface == TRUE) && (aex_modTransaction == AEX_MODE_TRANSACTION_EMV))
		{
			return AEX_RESULT_TRY_ANOTHER_INTERFACE;
		}
		else
		{
			return AEX_RESULT_REQUEST_ANOTHER_PAYMENT;
		}
	}

DBG_Put_Text("4.2 EMV Processing Restriction Process");

	//EMV Processing Restrictions
	rspCode=AEX_Check_EMVProcessingRestrictions();

DBG_Put_Text("4.3 Supplementary Processing Restrictions");

	//Supplementary Processing Restrictions
	rspCode=AEX_Check_SupplementaryProcessingRestrictions();

DBG_Put_Text("4.4 Check Mag. Stripe Data");
	
	//Data Elements for Magstripe Mode
	rspCode=AEX_Check_DataElementsForMagstripeMode();
	if (rspCode == FAIL)
	{
		return AEX_RESULT_TERMINATE;
	}
	
	return AEX_RESULT_SUCCESS;
}


UCHAR AEX_CardholderVerification(void)
{
	UCHAR	rspCode=0xFF;
	UCHAR	flgListProcessing=FALSE;
	UCHAR	flgUnableToComplete=FALSE;

DBG_Put_Text("5 Cardholder Verification");

	//Set Default Result
	UT_Set_TagLength(ECL_LENGTH_9F34, glv_tag9F34.Length);
	AEX_Set_CVMResults(0x3F, 0x00, 0x00);
	
	if (aex_rstDRL & AEX_DRL_RESULT_REQUIRE_CVM)
	{
		glv_tag9F6D.Value[0]|=AEX_CRC_CVM_REQUIRED;
		glv_tag9F6E.Value[2]|=AEX_ECRC_CVM_REQUIRED;
		
		if (glv_tag82.Value[0] & AEX_AIP_CARDHOLDER_VERIFICATION_SUPPORTED)
		{
			rspCode=AEX_Perform_CVM_Processing();
			if (rspCode == AEX_CVM_RESULT_CVM_LIST_PROCESSING)
			{
				flgListProcessing=TRUE;
			}
		}
		else
		{
			flgUnableToComplete=TRUE;
		}
	}
	else
	{
		glv_tag9F6D.Value[0]&=(~AEX_CRC_CVM_REQUIRED);
		glv_tag9F6E.Value[2]&=(~AEX_ECRC_CVM_REQUIRED);
		
		rspCode=AEX_Perform_CVM_LimitNotExceeded();
		if (rspCode == AEX_CVM_RESULT_CVM_LIMIT_NOT_EXCEEDED_MOBILE)
		{
			rspCode=AEX_Perform_CVM_LimitNotExceeded_Mobile();
		}
		else
		{
			rspCode=AEX_Perform_CVM_LimitNotExceeded_Card();
		}

		if (rspCode == AEX_CVM_RESULT_CARDHOLDER_VERIFICATION_UNABLE_TO_COMPLETE)
		{
			flgUnableToComplete=TRUE;
		}
		else if (rspCode == AEX_CVM_RESULT_CVM_LIST_PROCESSING)
		{
			flgListProcessing=TRUE;
		}
		else if (rspCode == AEX_CVM_RESULT_TRY_AGAIN)
		{
			return AEX_RESULT_TRY_AGAIN;
		}
		else	//AEX_CVM_RESULT_GO_TO_TERMINAL_RISK_MANAGEMENT
		{
			;
		}
	}

	if (flgListProcessing == TRUE)
	{

DBG_Put_Text("CVM List");
DBG_Put_Hex(aex_cvrLen, aex_cvrList);

		while (1)
		{
			rspCode=AEX_Perform_CVM_ListProcessing();
			
			if (rspCode == AEX_CVM_RESULT_ONLINE_PIN)
			{
				AEX_Perform_CVM_OnlinePIN();

				etp_Outcome.CVM=ETP_OCP_CVM_OnlinePIN;
			}
			else if (rspCode == AEX_CVM_RESULT_MOBILE_CVM)
			{
				rspCode=AEX_Perform_CVM_MobileCVMProcessing();
				if (rspCode == AEX_CVM_RESULT_CVM_LIST_PROCESSING)
				{
					aex_cvrIndex++;	//Next CVM

					continue;
				}
				else
				{
					if (rspCode == AEX_CVM_RESULT_TRY_AGAIN)
					{
						return AEX_RESULT_TRY_AGAIN;
					}
				}
			}
			else if (rspCode == AEX_CVM_RESULT_CARDHOLDER_VERIFICATION_UNABLE_TO_COMPLETE)
			{
				flgUnableToComplete=TRUE;
			}
			else if (rspCode == AEX_CVM_RESULT_NO_CVM_REQUIRED)
			{
				glv_tag9F34.Value[2]=0x02;
			}
			else	//AEX_CVM_RESULT_SIGNATURE
			{
				etp_Outcome.CVM=ETP_OCP_CVM_ObtainSignature;
			}

			break;
		}
	}

	if (flgUnableToComplete == TRUE)
	{
		rspCode=AEX_Perform_CVM_UnableToComplete();
		if (rspCode == AEX_CVM_RESULT_TRY_ANOTHER_INTERFACE)
		{
			return AEX_RESULT_TRY_ANOTHER_INTERFACE;
		}
	}
		
	return AEX_RESULT_SUCCESS;
}


UCHAR AEX_TerminalRiskManagement(void)
{
	UCHAR	rspCode=0xFF;

DBG_Put_Text("6 Terminal Risk Management");

	if (aex_rstDRL & AEX_DRL_RESULT_EXCEED_FLOOR_LIMIT)
	{
		glv_tag95.Value[3]|=AEX_TVR_TRANSACTION_EXCEEDS_FLOOR_LIMIT;
	}

	rspCode=AEX_Check_ExceptionFile();
	if (rspCode == FAIL)
	{
		glv_tag95.Value[0]|=AEX_TVR_CARD_APPEARS_ON_TERMINAL_EXCEPTION_FILE;
	}

	return AEX_RESULT_SUCCESS;
}


UCHAR AEX_TerminalActionAnalysis(void)
{
	UINT	lenOfV=0;
	UCHAR	cmpBuffer[5]={0};
	UCHAR	cmpZero[5]={0};
	UCHAR	idxNumber=0;
	UCHAR	lenDolRelData=0;
	UCHAR	cmdP1=0;
	UCHAR	rspCode=0;

DBG_Put_Text("7 Terminal Action Analysis");

	//Offline Only
	if (((glv_tag9F35.Value[0] & 0x07) == 0x03) ||
		((glv_tag9F35.Value[0] & 0x07) == 0x06))
	{

DBG_Put_Text("7.1 Offline Only");

		for (idxNumber=0; idxNumber < 5; idxNumber++)
		{
			cmpBuffer[idxNumber]=glv_tagDF8121.Value[idxNumber]|glv_tag9F0E.Value[idxNumber];
			cmpBuffer[idxNumber]&=glv_tag95.Value[idxNumber];
		}

		if (!memcmp(cmpBuffer, cmpZero, 5))
		{

DBG_Put_Text("7.2 Set TC");

			aex_typAC=AEX_ACT_TC;
		}
	}
	else
	{
		//Online Only
		if (((glv_tag9F35.Value[0] & 0x07) == 0x01) ||
			((glv_tag9F35.Value[0] & 0x07) == 0x04))
		{

DBG_Put_Text("7.3 Online Only");

			if (aex_flgGoOnline == TRUE)
			{
				for (idxNumber=0; idxNumber < 5; idxNumber++)
				{
					cmpBuffer[idxNumber]=glv_tagDF8121.Value[idxNumber]|glv_tag9F0E.Value[idxNumber];
					cmpBuffer[idxNumber]&=glv_tag95.Value[idxNumber];
				}

				if (!memcmp(cmpBuffer, cmpZero, 5))
				{

DBG_Put_Text("7.4 Set ARQC");

					aex_typAC=AEX_ACT_ARQC;
				}
			}
		}
		else
		{
			//Offline with Online Capability
			if (((glv_tag9F35.Value[0] & 0x07) == 0x02) ||
				((glv_tag9F35.Value[0] & 0x07) == 0x05))
			{

DBG_Put_Text("7.5 Offline with Online Capability");

				if (aex_flgDelayedAuthorization == FALSE)
				{

DBG_Put_Text("7.6 Not Delayed Authorization");

					for (idxNumber=0; idxNumber < 5; idxNumber++)
					{
						cmpBuffer[idxNumber]=glv_tagDF8121.Value[idxNumber]|glv_tag9F0E.Value[idxNumber];
						cmpBuffer[idxNumber]&=glv_tag95.Value[idxNumber];
					}

					if (!memcmp(cmpBuffer, cmpZero, 5))
					{
						for (idxNumber=0; idxNumber < 5; idxNumber++)
						{
							cmpBuffer[idxNumber]=glv_tagDF8122.Value[idxNumber]|glv_tag9F0F.Value[idxNumber];
							cmpBuffer[idxNumber]&=glv_tag95.Value[idxNumber];
						}

						if (!memcmp(cmpBuffer, cmpZero, 5))
						{

DBG_Put_Text("7.7 Set TC");

							aex_typAC=AEX_ACT_TC;
						}
						else
						{

DBG_Put_Text("7.8 Set ARQC");

							aex_typAC=AEX_ACT_ARQC;

							if (aex_flgGoOnline == FALSE)
							{

DBG_Put_Text("7.9 Set AAC");

								aex_typAC=AEX_ACT_AAC;

								for (idxNumber=0; idxNumber < 5; idxNumber++)
								{
									cmpBuffer[idxNumber]=glv_tagDF8120.Value[idxNumber]|glv_tag9F0D.Value[idxNumber];
									cmpBuffer[idxNumber]&=glv_tag95.Value[idxNumber];
								}

								if (!memcmp(cmpBuffer, cmpZero, 5))
								{

DBG_Put_Text("7.10 Set TC");

									aex_typAC=AEX_ACT_TC;
								}
							}
						}
					}
				}
				else
				{
					
DBG_Put_Text("7.11 Delayed Authorization");

					for (idxNumber=0; idxNumber < 5; idxNumber++)
					{
						cmpBuffer[idxNumber]=glv_tagDF8121.Value[idxNumber]|glv_tag9F0E.Value[idxNumber];
						cmpBuffer[idxNumber]&=glv_tag95.Value[idxNumber];
					}

					if (!memcmp(cmpBuffer, cmpZero, 5))
					{
						for (idxNumber=0; idxNumber < 5; idxNumber++)
						{
							cmpBuffer[idxNumber]=glv_tagDF8122.Value[idxNumber]|glv_tag9F0F.Value[idxNumber];
							cmpBuffer[idxNumber]&=glv_tag95.Value[idxNumber];
						}

						if (!memcmp(cmpBuffer, cmpZero, 5))
						{
							
DBG_Put_Text("7.12 Set TC");

							aex_typAC=AEX_ACT_TC;
						}
						else
						{

DBG_Put_Text("7.13 Set ARQC");

							aex_typAC=AEX_ACT_ARQC;
						}
					}
				}
			}
		}
	}

DBG_Put_Text("7.14 Patch CDOL");

	if ((aex_typAC == AEX_ACT_TC) || (aex_typAC == AEX_ACT_ARQC))
	{
		if (aex_modODA == AEX_MODE_ODA_CDA)
		{
			AEX_Check_CDAMandatoryTag();
		}

		if (aex_typAC == AEX_ACT_TC)
		{
			UT_Set_TagLength(2, glv_tag8A.Length);
			memcpy(glv_tag8A.Value, "Y1", 2);
		}
	}
	else	//AEX_ACT_AAC
	{
		UT_Set_TagLength(2, glv_tag8A.Length);
		memcpy(glv_tag8A.Value, "Z1", 2);
	}

	if (aex_modTransaction == AEX_MODE_TRANSACTION_MAGSTRIPE)
	{
		AEX_Get_UnpredictableNumber(glv_tag9F37.Value);
	}

	UT_Get_TLVLengthOfV(glv_tag8C.Length, &lenOfV);
	rspCode=AEX_DOL_Patch_DOLData(lenOfV, glv_tag8C.Value, &lenDolRelData, aex_sndData);
	if (rspCode != SUCCESS)
	{

DBG_Put_Text("7.15 Patch CDOL1 FAIL");

		return AEX_RESULT_TERMINATE;
	}

	//Save CDOL1 Related Data
	UT_Set_TagLength(lenDolRelData, glv_tagDF8107.Length);
	memcpy(glv_tagDF8107.Value, aex_sndData, lenDolRelData);

DBG_Put_Text("7.16 Send GAC");

	aex_sndLen=(UINT)lenDolRelData;
	AEX_Reset_ReceiveBuffer();
	
	cmdP1=aex_typAC;
	if (aex_modODA == AEX_MODE_ODA_CDA)
	{
		if ((glv_tag95.Value[0] & AEX_TVR_CDA_FAILED) == 0)
		{
			cmdP1|=AEX_RCP_CDA_SIGNATURE_REQUESTED;
		}
	}
	
	rspCode=ECL_APDU_Generate_AC(cmdP1, aex_sndLen, aex_sndData, &aex_rcvLen, aex_rcvData);
	if (rspCode != ECL_LV1_SUCCESS)
	{
		if (rspCode == ECL_LV1_TIMEOUT_ISO)
		{

DBG_Put_Text("7.17 GAC Timeout");

			return AEX_RESULT_RESTART;
		}

DBG_Put_Text("7.18 GAC FAIL");

		return AEX_RESULT_TERMINATE;
	}

	return AEX_RESULT_SUCCESS;
}


UCHAR AEX_CardActionAnalysis(void)
{
	UCHAR	rspCode=0;
	UCHAR	flgDecline=FALSE;
	UCHAR	tag9F36[ECL_LENGTH_9F36]={0};

DBG_Put_Text("8 Card Action Analysis");
DBG_Put_Text("8.1 Check SW12");

	rspCode=UT_Check_SW12(&aex_rcvData[aex_rcvLen-2], STATUSWORD_9000);
	if (rspCode == TRUE)
	{

DBG_Put_Text("8.2 Check Response Format");

		rspCode=AEX_Check_GACResponseFormat((aex_rcvLen-2), aex_rcvData);
		if (rspCode == SUCCESS)
		{

DBG_Put_Text("8.3 Check AC");

			rspCode=AEX_Check_AC((aex_rcvLen-2), aex_rcvData);
			if (rspCode == SUCCESS)
			{
				if (aex_modTransaction == AEX_MODE_TRANSACTION_MAGSTRIPE)
				{
					memcpy(tag9F36, glv_tag9F36.Value, ECL_LENGTH_9F36);

					UT_Set_TagLength(0, glv_tag9F36.Length);	//Prevent Redudant Tag
				}

DBG_Put_Text("8.4 Copy Response");

				if (aex_rcvData[0] == 0x80)
				{
					rspCode=AEX_Copy_CardResponse_GACFM1((aex_rcvLen-2), aex_rcvData);
				}
				else
				{
					rspCode=AEX_Copy_CardResponse_TLV((aex_rcvLen-2), aex_rcvData);
				}

				if (rspCode == SUCCESS)
				{

DBG_Put_Text("8.5 Check CDA");

					if (aex_modODA == AEX_MODE_ODA_CDA)
					{
						//Save Generate AC Response
						memcpy(oda_bufRspGAC, aex_rcvData, (aex_rcvLen-2));
						
						rspCode=AEX_OfflineDataAuthentication_CDA();
						if (rspCode == FAIL)
						{
							flgDecline=TRUE;
						}
					}

					if (flgDecline == FALSE)
					{

DBG_Put_Text("8.6 Not Decline");

						if (aex_modTransaction == AEX_MODE_TRANSACTION_MAGSTRIPE)
						{
							
DBG_Put_Text("8.7 Check Mag. Stripe");

							if (!memcmp(tag9F36, glv_tag9F36.Value, ECL_LENGTH_9F36))
							{
								if ((glv_tag9F27.Value[0] & 0xC0) == AEX_ACT_ARQC)
								{
									return AEX_RESULT_SUCCESS;
								}
							}
							else
							{
								return AEX_RESULT_TERMINATE;
							}
						}
						else
						{
							return AEX_RESULT_SUCCESS;
						}
					}
				}
				else
				{
					return AEX_RESULT_TERMINATE;
				}
			}
		}
		else
		{

DBG_Put_Text("8.8 Check Other Interface");

			rspCode=AEX_Check_SupportForAlternativeInterface();
			if (rspCode == SUCCESS)
			{
				if (aex_modTransaction == AEX_MODE_TRANSACTION_EMV)
				{
					return AEX_RESULT_TRY_ANOTHER_INTERFACE;
				}

				return AEX_RESULT_REQUEST_ANOTHER_PAYMENT;
			}

			return AEX_RESULT_TERMINATE;
		}
	}
	else
	{

DBG_Put_Text("8.9 Check Mobile Interface");

		if (aex_modInterface == AEX_MODE_INTERFACE_MOBILE)
		{

DBG_Put_Text("8.10 Check SW12");

			rspCode=UT_Check_SW12(&aex_rcvData[aex_rcvLen-2], STATUSWORD_6984);
			if (rspCode == TRUE)
			{

DBG_Put_Text("8.11 Check Restart");

				if (aex_flgRestart == FALSE)
				{
					return AEX_RESULT_RESTART;
				}
			}
		}

		return AEX_RESULT_TERMINATE;
	}
	
	return AEX_RESULT_DECLINE;
}


void AEX_OutcomeProcessing(UCHAR iptRspCode)
{
	//Set Scheme ID
	SchemeID=(aex_modTransaction == AEX_MODE_TRANSACTION_MAGSTRIPE)?(VAP_Scheme_AE_Magstripe):(VAP_Scheme_AE_EMV);

	//Outcome
	if (iptRspCode == AEX_RESULT_SUCCESS)
	{
		L3_Response_Code			= VAP_RIF_RC_DATA;
		etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_Yes;
		etp_Outcome.ocmStatus		= ETP_OCP_UIS_CardReadSuccessfully;
		etp_Outcome.hldTime			= 10;

		if (((glv_tag9F27.Value[0] & 0xC0) == AEX_ACT_TC) ||
			(aex_flgDelayedAuthorization == TRUE))
		{
			etp_Outcome.ocmMsgID = (etp_Outcome.CVM == ETP_OCP_CVM_ObtainSignature)?(ETP_OCP_UIM_ApprovedPleaseSign):(ETP_OCP_UIM_Approved);	
		}
		else
		{
			etp_Outcome.ocmMsgID = ETP_OCP_UIM_AuthorisingPleaseWait;
		}

		if (aex_modTransaction == AEX_MODE_TRANSACTION_MAGSTRIPE)
		{
			AEX_Patch_MagStripeTrack();
		}
		else
		{
			VGL_AS210_D1_Track();
		}

		AEX_Patch_OnlineData();
	}
	else if (iptRspCode == AEX_RESULT_DECLINE)
	{
		L3_Response_Code			= VAP_RIF_RC_FAILURE;
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_NotAuthorised;
		etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
		etp_Outcome.rqtOutcome		= TRUE;
		etp_Outcome.hldTime			= 10;
	}
	else if (iptRspCode == AEX_RESULT_TERMINATE)
	{
		L3_Response_Code			= VAP_RIF_RC_FAILURE;
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_Terminate;
		etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
		etp_Outcome.rqtOutcome		= TRUE;
		etp_Outcome.hldTime			= 3;
	}
	else if (iptRspCode == AEX_RESULT_TRY_AGAIN)
	{
		L3_Response_Code			= VAP_RIF_RC_TRY_AGAIN;
		etp_flgComError				= TRUE;	//Enable Flag to Start Entry Point Again
		etp_Outcome.Start			= ETP_OCP_Start_B;
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_SeePhoneForInstructions;
		etp_Outcome.ocmStatus		= ETP_OCP_UIS_NotReady;
		etp_Outcome.rqtOutcome		= TRUE;
		etp_Outcome.hldTime			= 10;
		etp_Outcome.filOffRequest	= (UINT)aex_tmrDeactivation;

		aex_flgRestart=TRUE;
	}
	else if (iptRspCode == AEX_RESULT_RESTART)
	{
		L3_Response_Code			= VAP_RIF_RC_TRY_AGAIN;
		etp_flgComError				= TRUE;	//Enable Flag to Start Entry Point Again
		etp_Outcome.Start			= ETP_OCP_Start_B;
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PresentCardAgain;
		etp_Outcome.ocmStatus		= ETP_OCP_UIS_NotReady;
		etp_Outcome.rqtOutcome		= TRUE;
		etp_Outcome.hldTime			= 10;
		etp_Outcome.filOffRequest	= (UINT)aex_tmrDeactivation;

		aex_flgRestart=TRUE;
	}
	else if (iptRspCode == AEX_RESULT_TRY_ANOTHER_INTERFACE)
	{
		UT_Buz_Option(0);
		ETP_UI_Request(ETP_OCP_UIM_Terminate, ETP_OCP_UIS_ProcessingError);
			
		L3_Response_Code			= VAP_RIF_RC_OTHER_INTERFACE;
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PleaseInsertCard;
		etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
		etp_Outcome.rqtOutcome		= TRUE;
		etp_Outcome.hldTime			= 10;
	}
	else if (iptRspCode == AEX_RESULT_REQUEST_ANOTHER_PAYMENT)
	{
		UT_Buz_Option(0);
		ETP_UI_Request(ETP_OCP_UIM_Terminate, ETP_OCP_UIS_ProcessingError);
			
		L3_Response_Code			= VAP_RIF_RC_FAILURE;
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
		etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
		etp_Outcome.rqtOutcome		= TRUE;
		etp_Outcome.hldTime			= 10;
	}

	//Outcome Status Buzzer
	switch (etp_Outcome.ocmStatus)
	{
		case ETP_OCP_UIS_CardReadSuccessfully:	UT_Buz_Option(1);	break;
		case ETP_OCP_UIS_ProcessingError:		UT_Buz_Option(0);	break;
		default:													break;
	}
}


void AEX_Start_Kernel(UINT lenFCI, UCHAR * datFCI)
{
	UCHAR	rspCode=AEX_RESULT_TERMINATE;
	
	rspCode=AEX_Allocate_Buffer();
	if (rspCode == AEX_RESULT_TERMINATE)
	{
		return;
	}

	memcpy(aex_rcvData, datFCI, lenFCI);
	aex_rcvLen=lenFCI;

	ODA_Clear_Record();
	ODA_Clear_GACResponse();

	AEX_Clear_Parameter();
	AEX_Load_Configuration();

	rspCode=AEX_InitiateApplication();
	if (rspCode == AEX_RESULT_SUCCESS)
	{
		rspCode=AEX_ReadApplicationData();
		if (rspCode == AEX_RESULT_SUCCESS)
		{
			rspCode=AEX_OfflineDataAuthentication();
			if (rspCode == AEX_RESULT_SUCCESS)
			{
				rspCode=AEX_ProcessingRestrictions();
				if (rspCode == AEX_RESULT_SUCCESS)
				{
					rspCode=AEX_CardholderVerification();
					if (rspCode == AEX_RESULT_SUCCESS)
					{
						rspCode=AEX_TerminalRiskManagement();
						if (rspCode == AEX_RESULT_SUCCESS)
						{
							rspCode=AEX_TerminalActionAnalysis();
							if (rspCode == AEX_RESULT_SUCCESS)
							{
								rspCode=AEX_CardActionAnalysis();

								ETP_UI_Request(ETP_OCP_UIM_CardReadOKRemoveCard, ETP_OCP_UIS_CardReadSuccessfully);
								UT_WaitTime(30);
							}
						}
					}
				}
			}
		}
	}

	AEX_OutcomeProcessing(rspCode);

	AEX_Free_Buffer();
}

