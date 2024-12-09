#include <string.h>
#include <stdlib.h>
#include "POSAPI.h"
#include "Datastructure.h"
#include "Define.h"
#include "ECL_Tag.h"
#include "Function.h"
#include "MPP_Define.h"
#include "MPP_Tag.h"
#include "MPP_TagOf.h"
#include "ECL_LV1_Define.h"

// Add by Wayne 2020/08/21 to avoid compiler warning
#include "UTILS_CTLS.H"

//		CAPK
extern	CAPK	glv_CAPK[CAPK_NUMBER];

//		CRL
extern	CRL		glv_CRL[CRL_NUMBER];

//		Transaction Data
extern	UCHAR	etp_txnData[ETP_TRANSACTION_DATA_SIZE];
extern	UINT	etp_txnDataLen;

//		Kernel Configuration Data
extern	KRNCONFIG	etp_krnConfig[ETP_NUMBER_KERNELCONFIGURATION];
extern	UCHAR		etp_krnCfgIndex;


//		Debug Flag
extern	UCHAR	mpp_flgTA;

//		PayPass TLV DB
extern ECL_TAG	mpp_lstPresent;
extern ECL_TAG	mpp_lstTlvDB;

//		MPP DEP Buffer
extern	UINT	mpp_depRcvLen;
//extern	UCHAR	mpp_depRcvData[ETP_BUFFER_SIZE];
extern UCHAR *mpp_depRcvData; //Wayne modify

//		Verify SDAD Buffer
extern	UINT	mpp_vfyDataLen;
//extern	UCHAR	mpp_vfyData[ETP_BUFFER_SIZE];
extern UCHAR	*mpp_vfyData; // Wayne modify




//		Flag
extern	UCHAR	mpp_flgSigned;

//		PayPass Queue
extern	UCHAR	mpp_Queue[MPP_QUEUE_SIZE];
extern	UCHAR	mpp_queSigNumber;

//		PayPass Signal
extern	UCHAR	mpp_Signal;

//		Torn Transaction Record
extern	TORNREC	mpp_trnRec[MPP_TORN_AIDNUMBER][MPP_TORN_RECORDNUMBER];
extern	UCHAR	mpp_trnRecNum[MPP_TORN_AIDNUMBER];

//		Process Flag
extern	UCHAR	mpp_flgMisPDOL;
extern	UCHAR	mpp_flgParResult;
extern	UCHAR	mpp_flgSigned;
extern	UCHAR	mpp_flgVfySDAD;
extern	UCHAR	mpp_flgOddCVM;

//		DataExchange
//extern UCHAR	DE_xmlBuf[DE_sizeXML];	//william
extern UCHAR	*mpp_ptrDE_xmlBuf;	//william
extern UINT		mpp_lenDE_xmlBuf;
//extern UCHAR	mpp_DE_log[MPP_DE_LOG_SIZE];
extern UCHAR   *mpp_DE_log;
extern UCHAR   *mpp_ptrDE_log;
extern UINT		mpp_lenDETRcvBuff;
//extern UCHAR	mpp_DETRcvBuff[1024];
extern UCHAR   *mpp_DETRcvBuff;



extern void		MPP_DBG_Put_Process(UCHAR iptState, UCHAR iptID, UCHAR iptRspCode);
extern void		MPP_DBG_Put_String(UCHAR iptLen, UCHAR * iptString);


UCHAR	MPP_AddToList(UCHAR * iptLstItem, UCHAR * iptList, UCHAR iptLstType);
UCHAR	MPP_IsPresent(UCHAR * iptTag);
void	MPP_RemoveFromList(UCHAR * iptLstItem, UCHAR * iptList, UCHAR iptLstType);
void	MPP_MSG(UCHAR *iptTag);
void	MPP_Store_Tag(UCHAR lenOfL, UCHAR lenOfV, UCHAR * iptData, UINT iptTagIndex);
UCHAR	MPP_Store_TLV(UINT lenOfTLV, UCHAR * iptData, UCHAR * iptList);



UCHAR MPP_Allocate_DE_LogBuffer(void)
{
#ifdef MPP_DE_LOG
	mpp_DE_log = malloc(sizeof(UCHAR) * MPP_DE_LOG_SIZE);
	if (mpp_DE_log == NULLPTR)
	{
		return  FAIL;
	}
	
	mpp_ptrDE_log = mpp_DE_log;
#endif

	return SUCCESS;
}

void MPP_Free_DE_LogBuffer(void)
{
#ifdef MPP_DE_LOG
	free(mpp_DE_log);
	mpp_ptrDE_log = NULLPTR;
#endif
}

void MPP_DataExchangeLog(UCHAR* buff, UINT len)
{
#ifdef MPP_DE_LOG
	if (mpp_ptrDE_log != NULLPTR)
	{
		if (((mpp_ptrDE_log+len)-mpp_DE_log) <= MPP_DE_LOG_SIZE)
		{
			memcpy(mpp_ptrDE_log, buff, len);
			mpp_ptrDE_log += len;
		}
	}
#else
	buff=buff;len=len;
#endif
}

void MPP_Load_TransactionData(void)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfData=0;
	UCHAR	*ptrData=NULLPTR;
	
	ptrData=etp_txnData;

	do {
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		//Add to Present List
		MPP_AddToList(ptrData, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

		//Point to Next Tag
		ptrData+=(lenOfT+lenOfL+lenOfV);
		lenOfData+=(lenOfT+lenOfL+lenOfV);
	} while (lenOfData < etp_txnDataLen);
}


void MPP_Load_KernelConfiguration(void)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfCfg=0;
	UINT	lenOfData=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=0;
	UINT	recIndex=0;
	UCHAR	sysInfo[20]={0};

	if (etp_krnCfgIndex == 0xFF)	//Configuration not Found
	{
		return;
	}

	lenOfCfg=etp_krnConfig[etp_krnCfgIndex].cfgLen;
	ptrData=etp_krnConfig[etp_krnCfgIndex].cfgData;

	do {
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		//Add to Present List
		MPP_AddToList(ptrData, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

		//Save Tag
		rspCode=UT_Search_Record(lenOfT, ptrData, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
		if (rspCode == SUCCESS)
		{
			MPP_Store_Tag(lenOfL, lenOfV, ptrData+lenOfT, recIndex);
		}
		else
		{
			MPP_AddToList(ptrData, mpp_lstTlvDB.Value, MPP_LISTTYPE_TLV);
		}

		//Point to Next Tag
		ptrData+=(lenOfT+lenOfL+lenOfV);
		lenOfData+=(lenOfT+lenOfL+lenOfV);
	} while (lenOfData < lenOfCfg);

	//Retrieve Interface Device Serial Number
	rspCode=MPP_IsPresent(mpp_tof9F1E);
	if (rspCode == TRUE)
	{
		api_sys_info(SID_TerminalSerialNumber, sysInfo);
		UT_Set_TagLength(ECL_LENGTH_9F1E, glv_tag9F1E.Length);
		memcpy(glv_tag9F1E.Value, &sysInfo[4], ECL_LENGTH_9F1E);
	}
}


void MPP_Reset_PresentList(void)
{
	memset(mpp_lstPresent.Length, 0, 3+MPP_TLVLIST_BUFFERSIZE);
}


void MPP_Reset_TLVDB(void)
{
	memset(mpp_lstTlvDB.Length, 0, 3+MPP_TLVLIST_BUFFERSIZE);
}


void MPP_Reset_SelfDefineTag(void)
{
	memset(mpp_tagActiveAFL.Length,							0, 3+MPP_LENGTH_ActiveAFL);
	memset(mpp_tagActiveTag.Length,							0, 3+MPP_LENGTH_ActiveTag);
	memset(mpp_tagACType.Length,							0, 3+MPP_LENGTH_ACType);
//	memset(mpp_tagFailedMSCntr.Length,						0, 3+MPP_LENGTH_FailedMSCntr);
	memset(mpp_tagNextCmd.Length,							0, 3+MPP_LENGTH_NextCmd);

	memset(mpp_tagnUN.Length,								0, 3+MPP_LENGTH_nUN);
	memset(mpp_tagODAStatus.Length,							0, 3+MPP_LENGTH_ODAStatus);
	memset(mpp_tagReaderContactlessTransactionLimit.Length,	0, 3+MPP_LENGTH_ReaderContactlessTransactionLimit);
	memset(mpp_tagStaticDataToBeAuthenticated.Length,		0, 3+MPP_LENGTH_StaticDataToBeAuthenticated);
	memset(mpp_tagTagsToReadYet.Length,						0, 3+MPP_TLVLIST_BUFFERSIZE);

	memset(mpp_tagTagsToWriteYetAfterGenAC.Length,			0, 3+MPP_TLVLIST_BUFFERSIZE);
	memset(mpp_tagTagsToWriteYetBeforeGenAC.Length,			0, 3+MPP_TLVLIST_BUFFERSIZE);
	memset(mpp_tagTornEntry.Length,							0, 3+MPP_LENGTH_TornEntry);
	memset(mpp_tagTornTempRecord.Length,					0, 3+MPP_LENGTH_TornTempRecord);
}


void MPP_Reset_Flag(void)
{
	mpp_flgMisPDOL=FALSE;				//Flag of Missing PDOL Data
	mpp_flgParResult=FALSE;				//Flag of Parsing Result
	mpp_flgSigned=FALSE;				//Flag of Signed Active AFL
	mpp_flgVfySDAD=FALSE;				//Flag of Verify Signed Dynamic Application Data
	mpp_flgOddCVM=FALSE;				//Flag of Odd CVM List
}


void MPP_Reset_Parameter(void)
{
	MPP_Reset_Flag();
	MPP_Reset_SelfDefineTag();
	MPP_Reset_PresentList();
	MPP_Reset_TLVDB();
}


void MPP_Clear_DEPBuffer(void)
{
	mpp_depRcvLen=0;
//Test
//	memset(mpp_depRcvData, 0, ETP_BUFFER_SIZE);
}


void MPP_Clear_Signal(void)
{
	mpp_queSigNumber=0;
	memset(mpp_Queue, 0, MPP_QUEUE_SIZE);
}


UCHAR MPP_Check_PayPassTransaction(void)
{
	UCHAR rspCode=0;

	//If it is a PayPass Transaction, DF811A should be Always Present
	rspCode=MPP_IsPresent(mpp_tofDF811A);

	return rspCode;
}


void MPP_Check_api_rsa_recover(UCHAR iptLen, UCHAR *recData)
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


UCHAR MPP_Check_Separator(UCHAR iptLen, UCHAR *iptData, UCHAR iptTrkNo)
{
	UCHAR	idxNum=0;
	UCHAR	flgSeparator=FALSE;
	UCHAR	datBuffer[ECL_LENGTH_56]={0};
	UCHAR	*ptrData=NULLPTR;

	ptrData=datBuffer;

	if (iptTrkNo == MPP_TRACK_1)
	{
		memcpy(datBuffer, iptData, iptLen);

		//Find Separator '^' 2 Times
		for (idxNum=0; idxNum < iptLen; idxNum++)
		{
			if (ptrData[0] == '^')
			{
				if (flgSeparator == TRUE)
				{
					return SUCCESS;
				}
				else
				{
					flgSeparator=TRUE;
				}
			}

			ptrData++;
		}
	}
	else if (iptTrkNo == MPP_TRACK_2)
	{
		//Convert to ASCII
		UT_Split(datBuffer, iptData, iptLen);

		//Find Separator 'D'
		for (idxNum=0; idxNum < (iptLen*2); idxNum++)
		{
			if (ptrData[0] == 'D')
			{
				return SUCCESS;
			}

			ptrData++;
		}
	}

	return FAIL;
}


UCHAR MPP_Check_TornTransactionExpire(UCHAR *iptTrnDatetime)
{
	ULONG	rtcDatTime=0;
	ULONG	trnDatTime=0;
	ULONG	expTime=0;
	UCHAR	ascDatTime[12]={0};

	//Get RTC Datetime
	api_rtc_getdatetime(0, ascDatTime);
	rtcDatTime=UT_Get_UnixTime(ascDatTime);
	
	//Get Torn Log Datetime
	trnDatTime=UT_Get_UnixTime(iptTrnDatetime);

	//Get Max Lifetime of Torn Transaction Log Record
	expTime=glv_tagDF811C.Value[0]*256+glv_tagDF811C.Value[1];

	if ((rtcDatTime-trnDatTime) > expTime)
		return TRUE;
	
	return FALSE;
}


UCHAR MPP_Compare_Date(UCHAR *iptDate1, UCHAR *iptDate2)
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


UCHAR MPP_Check_PhoneMessageTable(UCHAR *optMessage, UCHAR *optStatus)
{
	UCHAR	idxNum=0;
	UCHAR	idxTmp=0;
	UCHAR	rspCode=0xFF;
	UCHAR	tmpDF4B[3]={0};
	UINT	lenOfV=0;
	UINT	lenOfData=0;
	
	UT_Get_TLVLengthOfV(glv_tagDF8131.Length, &lenOfV);
	
	do {
		//tmpDF4B=[(Phone Message Table PCII Mask) & (POS Cardholder Interaction Information)]
		for (idxTmp=0; idxTmp < 3; idxTmp++)
		{
			tmpDF4B[idxTmp]=(glv_tagDF8131.Value[idxNum*8+idxTmp] & glv_tagDF4B.Value[idxTmp]);
		}

		rspCode=UT_bcdcmp(tmpDF4B, &glv_tagDF8131.Value[idxNum*8+3], 3);
		if (rspCode == 0)
		{
			optMessage[0]=glv_tagDF8131.Value[idxNum*8+6];
			optStatus[0]=glv_tagDF8131.Value[idxNum*8+7];
			
			return TRUE;
		}

		//Point to Next Entry
		idxNum++;
		lenOfData+=8;
	} while (lenOfData < lenOfV);

	return FALSE;
}


UCHAR MPP_Check_CRL(UCHAR *iptRID, UCHAR iptIndex, UCHAR *iptSerNumber)
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


UCHAR MPP_Check_CDOL1(UCHAR *iptTag)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfCDOL1=0;
	UCHAR	lenOfCheck=0;
	UINT	lenOfData=0;
	UCHAR	*ptrData=NULLPTR;
	
	UT_Get_TLVLengthOfT(iptTag, &lenOfT);
	lenOfCheck=lenOfT;

	UT_Get_TLVLengthOfV(glv_tag8C.Length, &lenOfV);
	lenOfCDOL1=lenOfV;

	//Set Pointer to CDOL1
	ptrData=glv_tag8C.Value;
	
	do {
		if (!memcmp(ptrData, iptTag, lenOfCheck))
		{
			return TRUE;
		}
		
		//Point to Next Tag
		UT_Get_TLVLengthOfT(ptrData, &lenOfT);
		lenOfL=1;
		ptrData+=(lenOfT+lenOfL);
		lenOfData+=(lenOfT+lenOfL);
	} while (lenOfData < lenOfCDOL1);
		
	return FALSE;
}


UCHAR MPP_Check_CVMMethod_IsRecognized(UCHAR iptMedCode)
{
	UCHAR	medCode=0;

	medCode=iptMedCode & 0xBF;	//Mask Off b7
	
	switch (medCode)
	{
		case MPP_CVM_METHOD_FailCardholderVerificationIfThisCVMIsUnsuccessful:
		case MPP_CVM_METHOD_ApplySucceedingCVRuleIfThisCVMIsUnsuccessful:
		case MPP_CVM_METHOD_PlaintextPINVerificationPerformedByICC:
		case MPP_CVM_METHOD_EncipheredPINVerifiedOnline:
		case MPP_CVM_METHOD_PlaintextPINVerificationPerformedByICCAndSignature_Paper:
		case MPP_CVM_METHOD_EncipheredPINVerificationPerformedByICC:
		case MPP_CVM_METHOD_EncipheredPINVerificationPerformedByICCAndSignature_Paper:
		case MPP_CVM_METHOD_Signature_Paper:
		case MPP_CVM_METHOD_NoCVMRequired:
					return TRUE;

		default:	break;
	}

	return FALSE;
}


UCHAR MPP_Check_CVMMethod_IsSupported(UCHAR iptMedCode)
{
	UCHAR	medCode=0;
	UCHAR	rspCode=FALSE;
	
	medCode=iptMedCode & 0xBF;	//Mask Off b7
	
	switch (medCode)
	{
		case MPP_CVM_METHOD_FailCardholderVerificationIfThisCVMIsUnsuccessful:
			rspCode=TRUE;

			break;
			
		case MPP_CVM_METHOD_PlaintextPINVerificationPerformedByICC:
		case MPP_CVM_METHOD_PlaintextPINVerificationPerformedByICCAndSignature_Paper:
			if (glv_tag9F33.Value[1] & MPP_TRC_PlaintextPINforICCVerification)
				rspCode=TRUE;

			break;

		case MPP_CVM_METHOD_EncipheredPINVerifiedOnline:
			if (glv_tag9F33.Value[1] & MPP_TRC_EncipheredPINForOnlineVerification)
				rspCode=TRUE;

			break;

		case MPP_CVM_METHOD_Signature_Paper:
			if (glv_tag9F33.Value[1] & MPP_TRC_Signature_Paper)
				rspCode=TRUE;

			break;

		case MPP_CVM_METHOD_EncipheredPINVerificationPerformedByICC:
		case MPP_CVM_METHOD_EncipheredPINVerificationPerformedByICCAndSignature_Paper:
			if (glv_tag9F33.Value[1] & MPP_TRC_EncipheredPINForOfflineVerification)
				rspCode=TRUE;

			break;
		
		case MPP_CVM_METHOD_NoCVMRequired:
			if (glv_tag9F33.Value[1] & MPP_TRC_NoCVMRequired)
				rspCode=TRUE;

			break;

		default: break;
	}
	
	return rspCode;
}


UCHAR MPP_Check_CVMCondition_IsUnderstood(UCHAR iptCndCode)
{
	switch (iptCndCode)
	{
		case MPP_CVM_CONDITION_Always:
		case MPP_CVM_CONDITION_IfUnattendedCash:
		case MPP_CVM_CONDITION_IfNotUnattendedCashAndNotManualCashAndNotPurchaseWithCashback:
		case MPP_CVM_CONDITION_IfTerminalSupportsTheCVM:
		case MPP_CVM_CONDITION_IfManualCash:
		case MPP_CVM_CONDITION_IfPurchaseWithCashback:
		case MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsUnderXValue:
		case MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsOverXValue:
		case MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsUnderYValue:
		case MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsOverYValue:
					return TRUE;
					
		default:	break;			
	}

	return FALSE;
}


UCHAR MPP_Check_CVMCondition_IsSatisfied(UCHAR iptCvmCode, UCHAR iptCndCode)
{
	UCHAR	flgIsAppCurrency=FALSE;
	UCHAR	binAmount[4]={0};
	UCHAR	rspCode=0;

	if ((iptCndCode == MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsUnderXValue) ||
		(iptCndCode == MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsOverXValue) ||
		(iptCndCode == MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsUnderYValue) ||
		(iptCndCode == MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsOverYValue))
	{
		//Check Transaction Currency = Application Currency
		if (!memcmp(glv_tag5F2A.Value, glv_tag9F42.Value, 2))
			flgIsAppCurrency=TRUE;

		//Convert BCD Amount to Binary Amount
		UT_bcd2hex(5, &glv_tag9F02.Value[1], binAmount);
	}
	
	switch (iptCndCode)
	{
		case MPP_CVM_CONDITION_Always:
			return TRUE;
			
		case MPP_CVM_CONDITION_IfUnattendedCash:
			if (glv_tag9C.Value[0] == MPP_TXT_Cash)
				return TRUE;
			else
				break;
			
		case MPP_CVM_CONDITION_IfNotUnattendedCashAndNotManualCashAndNotPurchaseWithCashback:
			if (glv_tag9C.Value[0] == MPP_TXT_Purchase)
				return TRUE;
			else
				break;
			
		case MPP_CVM_CONDITION_IfTerminalSupportsTheCVM:
			rspCode=MPP_Check_CVMMethod_IsSupported(iptCvmCode);
			if (rspCode == TRUE)
				return TRUE;
			else
				break;

		case MPP_CVM_CONDITION_IfManualCash:
			//if (glv_tag9C.Value[0] == MPP_TXT_ManualCash)
			if(glv_tag9C.Value[0] == MPP_TXT_CashDisbursement)	//Tammy 2017/11/13
				return TRUE;
			else
				break;
			
		case MPP_CVM_CONDITION_IfPurchaseWithCashback:
			if (glv_tag9C.Value[0] == MPP_TXT_CashBack)
				return TRUE;
			else
				break;
			
		case MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsUnderXValue:
			if (flgIsAppCurrency == FALSE)
				break;

			if (UT_memcmp(binAmount, glv_tag8E.Value, 4) <= 0)
				return TRUE;
			else
				break;
			
		case MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsOverXValue:
			if (flgIsAppCurrency == FALSE)
				break;

			if (UT_memcmp(binAmount, glv_tag8E.Value, 4) > 0)
				return TRUE;
			else
				break;
			
		case MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsUnderYValue:
			if (flgIsAppCurrency == FALSE)
				break;

			if (UT_memcmp(binAmount, &glv_tag8E.Value[4], 4) <= 0)
				return TRUE;
			else
				break;
			
		case MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsOverYValue:
			if (flgIsAppCurrency == FALSE)
				break;

			if (UT_memcmp(binAmount, &glv_tag8E.Value[4], 4) > 0)
				return TRUE;
			else
				break;
	}
	
	return FALSE;
}


UCHAR MPP_Check_CVMList_IsLastCVR(UCHAR iptLstIndex)
{
	UINT	lenOfV=0;

	UT_Get_TLVLengthOfV(glv_tag8E.Length, &lenOfV);

	//CVM List = X(4) + Y(4) + (Index+1) * 2
	//Index Start from 0, So Add 1 to Adjust
	if ((8 + (iptLstIndex + 1) * 2) == lenOfV)
		return TRUE;
	
	return FALSE;
}


void MPP_Get_CVMList_CVR(UCHAR iptLstIndex, UCHAR *optCVR)
{
	//CVR Address = X(4) + Y(4) + Index * 2
	memcpy(optCVR, &glv_tag8E.Value[8+iptLstIndex*2], 2);
}


UCHAR MPP_Get_ErrorIndication_L1(UCHAR iptRspCode)
{
	switch (iptRspCode)
	{
		case ECL_LV1_SUCCESS:				return MPP_EID_LV1_OK;
		case ECL_LV1_ERROR_TRANSMISSION:	return MPP_EID_LV1_TransmissionError;
		case ECL_LV1_ERROR_PROTOCOL:		return MPP_EID_LV1_ProtocolError;
		case ECL_LV1_TIMEOUT_ISO:			return MPP_EID_LV1_TimeOutError;

		default: break;
	}
	
	return ERROR;
}


UINT MPP_Get_NumberOfNonZeroBits(UINT iptLen, UCHAR *iptData)
{
	UINT	cntNumber=0;
	UCHAR	idxBit=0;
	UINT	idxByte=0;
	UCHAR	mskBit=0;

	for (idxByte=0; idxByte < iptLen; idxByte++)
	{
		mskBit=1;
		
		for (idxBit=0; idxBit < 8; idxBit++)
		{
			if (iptData[idxByte] & mskBit)
				cntNumber++;
			
			mskBit<<=1;
		}
	}
	
	return cntNumber;
}


ULONG MPP_Get_BCDRandomNumber_4Byte(void)
{
	ULONG	rndNumber=0;
	UCHAR	rndBuffer[8]={0};
	
	while (1)
	{
		api_sys_random_len(rndBuffer,4);

		//Convert to Integer
		rndNumber=(rndBuffer[0] << 24)+(rndBuffer[1] << 16)+(rndBuffer[2] << 8)+(rndBuffer[3]);

		if (rndNumber <= 99999999)	//A 4 Byte BCD is Under 99999999
			break;
	}

	return rndNumber;
}


UCHAR MPP_Get_TornRecordAIDIndex(void)
{
	UINT	lenOfV=0;
	UCHAR	idxNum=0;
	UCHAR	rspCode=FAIL;
	UCHAR	trnAID[3][ECL_LENGTH_9F06]=
	{
		{0xA0,0x00,0x00,0x00,0x04,0x10,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
		{0xA0,0x00,0x00,0x00,0x04,0x30,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
		{0xB0,0x12,0x34,0x56,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
	};

	rspCode=UT_Get_TLVLengthOfV(glv_tag9F06.Length, &lenOfV);
	if (rspCode == SUCCESS)
	{
		for (idxNum=0; idxNum < MPP_TORN_AIDNUMBER; idxNum++)
		{
			if (!memcmp(trnAID[idxNum], glv_tag9F06.Value, lenOfV))
				return idxNum;
		}
	}

	return 0xFF;
}

void MPP_Insert_Signal(UCHAR iptSignal)	//william 2017/11/06
{
	UCHAR	tmpQueue[MPP_QUEUE_SIZE] = { 0 };

	if ((mpp_queSigNumber + 1) <= MPP_QUEUE_SIZE)
	{
		memcpy(tmpQueue, mpp_Queue, mpp_queSigNumber);
		mpp_Queue[0] = iptSignal;
		memcpy(&mpp_Queue[1], tmpQueue, mpp_queSigNumber);
		mpp_queSigNumber++;
	}

}

void MPP_Get_Signal(void)
{
	UCHAR	tmpQueue[MPP_QUEUE_SIZE]={0};
	
	//Reset
	mpp_Signal=0;
	
	if (mpp_queSigNumber != 0)
	{
		mpp_Signal=mpp_Queue[0];

		//Remove First Signal
		mpp_queSigNumber--;
		memcpy(tmpQueue, &mpp_Queue[1], mpp_queSigNumber);
		memset(mpp_Queue, 0, MPP_QUEUE_SIZE);
		memcpy(mpp_Queue, tmpQueue, mpp_queSigNumber);
	}
}


void MPP_Add_Signal(UCHAR iptSignal)
{
	if ((mpp_queSigNumber+1) <= MPP_QUEUE_SIZE)
	{
		mpp_Queue[mpp_queSigNumber]=iptSignal;
		mpp_queSigNumber++;
	}
}


UCHAR MPP_Add_TornRecord(UINT iptRecLen, UCHAR *iptRecData)
{
	UCHAR	idxAID=0xFF;
	UCHAR	idxNum=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	rtcBuffer[12]={0};

	idxAID=MPP_Get_TornRecordAIDIndex();
	if (idxAID != 0xFF)
	{
		for (idxNum=0; idxNum < MPP_TORN_RECORDNUMBER; idxNum++)
		{
			if (mpp_trnRec[idxAID][idxNum].PANLen == 0)	//Empty
			{
				ptrData=UT_Find_Tag(mpp_tof5A, iptRecLen, iptRecData);
				if (ptrData != NULLPTR)
				{
					UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

					mpp_trnRec[idxAID][idxNum].PANLen=lenOfV;
					memcpy(mpp_trnRec[idxAID][idxNum].PAN, ptrData+lenOfT+lenOfL, lenOfV);
				}

				ptrData=UT_Find_Tag(mpp_tof5F34, iptRecLen, iptRecData);
				if (ptrData != NULLPTR)
				{
					UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

					mpp_trnRec[idxAID][idxNum].PSN=ptrData[lenOfT+lenOfL];
				}

				UT_GetDateTime(rtcBuffer);
				UT_Compress(mpp_trnRec[idxAID][idxNum].DateTime, rtcBuffer, 6);
				
				if (iptRecLen <= MPP_TORN_BUFFERSIZE)
				{
					mpp_trnRec[idxAID][idxNum].RecLen=iptRecLen;
					memcpy(mpp_trnRec[idxAID][idxNum].Record, iptRecData, iptRecLen);
				}

				mpp_trnRecNum[idxAID]+=1;
				
				return SUCCESS;
			}
		}
	}
	
	return FAIL;
}


void MPP_Set_DefaultCRL(void)
{
	UCHAR	idxNum=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	numCRL=60;
	UCHAR	tstCRL[540]=
	{
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x00, 0x00, 0x10,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x00, 0x00, 0x11,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x00, 0x01, 0x01,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x00, 0x01, 0x10,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x00, 0x01, 0x11,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x00, 0x10, 0x00,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x00, 0x10, 0x01,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x00, 0x10, 0x10,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x00, 0x10, 0x11,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x00, 0x11, 0x00,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x00, 0x11, 0x01,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x00, 0x11, 0x10,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x00, 0x11, 0x11,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x00, 0x00,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x00, 0x01,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x00, 0x10,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x00, 0x11,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x01, 0x00,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x01, 0x01,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x01, 0x11,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x10, 0x00,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x10, 0x01,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x10, 0x10,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x10, 0x11,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x11, 0x00,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x11, 0x01,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x11, 0x10,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x01, 0x11, 0x11,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x10, 0x00, 0x00,
		0xA0, 0x00, 0x00, 0x00, 0x04, 0xF8, 0x10, 0x00, 0x01,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x00, 0x00, 0x10,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x00, 0x00, 0x11,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x00, 0x01, 0x01,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x00, 0x01, 0x10,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x00, 0x01, 0x11,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x00, 0x10, 0x00,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x00, 0x10, 0x01,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x00, 0x10, 0x10,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x00, 0x10, 0x11,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x00, 0x11, 0x00,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x00, 0x11, 0x01,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x00, 0x11, 0x10,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x00, 0x11, 0x11,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x00, 0x00,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x00, 0x01,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x00, 0x10,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x00, 0x11,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x01, 0x00,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x01, 0x01,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x01, 0x11,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x10, 0x00,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x10, 0x01,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x10, 0x10,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x10, 0x11,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x11, 0x00,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x11, 0x01,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x11, 0x10,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x01, 0x11, 0x11,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x10, 0x00, 0x00,
		0xB0, 0x12, 0x34, 0x56, 0x78, 0xF8, 0x10, 0x00, 0x01
	};

	ptrData=tstCRL;

	for (idxNum=0; idxNum < numCRL; idxNum++)
	{
		//Copy RID
		memcpy(glv_CRL[idxNum].RID, ptrData, 5);
		ptrData+=5;

		//Copy Index
		glv_CRL[idxNum].Index=ptrData[0];
		ptrData+=1;

		//Copy Serial Number
		memcpy(glv_CRL[idxNum].serNumber, ptrData, 3);
		ptrData+=3;
	}
}


UCHAR *MPP_Find_DD_Track2(UCHAR *optLen, UCHAR *optStrPosition)
{
	UCHAR	ascTrack2[ECL_LENGTH_9F6B*2]={0};
	UCHAR	datIndex=0;
	UINT	lenOf9F6B=0;
	UCHAR	flgFound=FALSE;
	UCHAR	*ptrData=NULLPTR;

	//Point to Track 2
	ptrData=glv_tag9F6B.Value;

	//Get Length of Track 2
	UT_Get_TLVLengthOfV(glv_tag9F6B.Length, &lenOf9F6B);

	//Split to Ascii for Processing
	UT_Split(ascTrack2, glv_tag9F6B.Value, lenOf9F6B);

	//Find Field Separator
	for (datIndex=0; datIndex < (lenOf9F6B*2); datIndex++)
	{
		if (ascTrack2[datIndex] == 'D')
		{
			flgFound=TRUE;

			if (datIndex & 1)
				optStrPosition[0]=MPP_DD_START_LSB;
			else
				optStrPosition[0]=MPP_DD_START_MSB;

			break;
		}
	}

	if (flgFound == FALSE)
		return NULLPTR;

	//Move Pointer
	datIndex+=1;	//Point to Start of Expiry Date
	datIndex+=4;	//Point to Start of Service Code
	datIndex+=3;	//Point to Start of Discretionary Data

	//Check Last Padding Word
	if (ascTrack2[lenOf9F6B*2-1] == 'F')
		optLen[0]=(lenOf9F6B*2-1)-datIndex;	//Exclude Padding 'F'
	else
		optLen[0]=lenOf9F6B*2-datIndex;

	ptrData+=(datIndex/2);

	return ptrData;
}


UCHAR *MPP_Find_DD_Track1(UCHAR *optLenDD)
{
	UINT	lenOfTag56=0;
	UCHAR	datIndex=0;
	UCHAR	flgFound=FALSE;
	UCHAR	*ptrData=NULLPTR;

	//Set Pointer to Track 1 Data
	ptrData=glv_tag56.Value;

	UT_Get_TLVLengthOfV(glv_tag56.Length, &lenOfTag56);

	//Find First Field Separator
	for (datIndex=0; datIndex < lenOfTag56; datIndex++)
	{
		if (*(ptrData+datIndex) == '^')
		{
			flgFound=TRUE;
			break;
		}
	}

	if (flgFound == FALSE)
		return NULLPTR;

	datIndex+=1;	//Point to Start of Name
	flgFound=FALSE;

	//Find Second Field Separator
	for (datIndex=datIndex; datIndex < lenOfTag56; datIndex++)
	{
		if (*(ptrData+datIndex) == '^')
		{
			flgFound=TRUE;
			break;
		}
	}

	if (flgFound == FALSE)
		return NULLPTR;

	//Move Pointer
	datIndex+=1;	//Point to Start of Expiry Date
	datIndex+=4;	//Point to Start of Service Code
	datIndex+=3;	//Point to Start of Discretionary Data

	optLenDD[0]=lenOfTag56-datIndex;
	ptrData+=datIndex;

	return ptrData;
}


void MPP_Move(UINT iptLen, UCHAR *iptData, UCHAR movDirection, UCHAR movBits)
{
	UCHAR	bitIdx=0;
	UINT	cntIdx=0;
	UCHAR	movMask=0;

	if ((movBits == 0) || (movBits > 8))
		return;

	if (movDirection == MPP_MOVE_LEFT)
	{
		//Count Move Mask
		bitIdx=0x80;
		for (cntIdx=0; cntIdx < movBits; cntIdx++)
		{
			movMask|=bitIdx;
			bitIdx>>=1;
		}

		//Move Data
		for (cntIdx=0; cntIdx < iptLen; cntIdx++)
		{
			if (cntIdx != 0)
			{
				iptData[cntIdx-1]|=((iptData[cntIdx] & movMask) >> (8-movBits));
			}

			iptData[cntIdx]<<=movBits;
		}
	}
	else if (movDirection == MPP_MOVE_RIGHT)
	{
		//Count Move Mask
		bitIdx=0x01;
		for (cntIdx=0; cntIdx < movBits; cntIdx++)
		{
			movMask|=bitIdx;
			bitIdx<<=1;
		}

		//Move Data Start from Last Byte
		cntIdx=iptLen-1;

		while (1)
		{
			iptData[cntIdx]>>=movBits;

			if (cntIdx == 0)
			{
				break;
			}
			else
			{
				iptData[cntIdx]|=((iptData[cntIdx-1] & movMask) << (8-movBits));

				cntIdx--;
			}
		}
	}
}


void MPP_Remove_FirstRecordFromActiveAFL(void)
{
	UINT	lenOfV=0;
	UCHAR	tmpAFL[MPP_LENGTH_ActiveAFL]={0};
	
	if (mpp_tagActiveAFL.Value[1] == mpp_tagActiveAFL.Value[2])
	{
		UT_Get_TLVLengthOfV(mpp_tagActiveAFL.Length, &lenOfV);
		lenOfV-=4;
		
		UT_Set_TagLength(lenOfV, mpp_tagActiveAFL.Length);

		memcpy(tmpAFL, &mpp_tagActiveAFL.Value[4], lenOfV);
		memset(mpp_tagActiveAFL.Value, 0, MPP_LENGTH_ActiveAFL);
		memcpy(mpp_tagActiveAFL.Value, tmpAFL, lenOfV);
	}
	else if (mpp_tagActiveAFL.Value[1] > mpp_tagActiveAFL.Value[2])
	{
		mpp_tagActiveAFL.Length[0]=0;
	}
	else
	{
		mpp_tagActiveAFL.Value[1]+=1;

		if (mpp_flgSigned == TRUE)
		{
			mpp_tagActiveAFL.Value[3]-=1;
		}
	}
}


void MPP_Remove_TornRecord(UCHAR iptRecIndex)
{
	UCHAR	idxAID=0;

	idxAID=MPP_Get_TornRecordAIDIndex();
	memset(&mpp_trnRec[idxAID][iptRecIndex].PANLen, 0, TORNREC_LEN);

	mpp_trnRecNum[idxAID]-=1;
}


void MPP_OWHF2(UCHAR *iptPD, UCHAR *optR)
{
	//Parameter Naming by PayPass
	UINT	PL;
	UCHAR	DSPKL[6]={0};
	UCHAR	DSPKR[6]={0};
	UCHAR	OID[8]={0};
	UCHAR	KL[8]={0};		//DES Key
	UCHAR	KR[8]={0};		//DES Key
	UCHAR	i=0;
	UCHAR	desKey[24]={0};

	//Process Parameter
	UINT	lenOfV=0;
	
	//Compute DSPKL & DSPKR
	UT_Get_TLVLengthOfV(glv_tag9F5E.Length, &lenOfV);
	PL=lenOfV;	//Length of DS ID

	for (i=1; i<7; i++)		//i = 1..6
	{
		DSPKL[i-1]=((glv_tag9F5E.Value[i-1]/16)*10+(glv_tag9F5E.Value[i-1]%16))*2;
		DSPKR[i-1]=((glv_tag9F5E.Value[PL-6+i-1]/16)*10+(glv_tag9F5E.Value[PL-6+i-1]%16))*2;
	}

	//Compute OID
	UT_Get_TLVLengthOfV(glv_tag9F6F.Length, &lenOfV);
//	if ((lenOfV != 0) && (glv_tag9F6F.Value[0] & 0xC0))	//DS Slot Management Control
	UCHAR flagIsNotEmpty, flag9F6FPermanent, flagDF62Volatile;
	flagIsNotEmpty = lenOfV;
	flag9F6FPermanent = (glv_tag9F6F.Value[0] & 0x80);
	flagDF62Volatile = (glv_tagDF62.Value[0] & 0x40);

	if(flagIsNotEmpty && flag9F6FPermanent && flagDF62Volatile)
		memset(OID, 0, 8);
	else
		memcpy(OID, glv_tag9F5C.Value, 8);	//DS Requested Operator ID

	//Genernate DES Key
	memcpy(KL, DSPKL, 6);
	memcpy(&KL[6], &OID[4], 2);	//Start from 5th
	memcpy(KR, DSPKR, 6);
	memcpy(&KR[6], &OID[6], 2);	//Start from 7th
	
	//Compute R
	for (i=0; i<8; i++)
		OID[i]^=iptPD[i];

	memcpy(desKey, KL, 8);
	memcpy(&desKey[8], KR, 8);
	memcpy(&desKey[16], KL, 8);

	api_3des_encipher(OID, optR, desKey);

	for (i=0; i<8; i++)
		optR[i]^=iptPD[i];
}


void MPP_OWHF2AES(UCHAR* iptC, UCHAR *optR)
{
	//Parameter Naming by PayPass
	UCHAR	OID[8]={0};
	UCHAR	M[16]={0};		//Message
	UCHAR	Y[11]={0};
	UCHAR	K[16]={0};		//AES Key
	UCHAR	T[16]={0};
	UCHAR	i=0;

	//Process Parameter
	UINT	lenOfV=0;
	
	
	//Compute OID
	UT_Get_TLVLengthOfV(glv_tag9F6F.Length, &lenOfV);
	//	if ((lenOfV != 0) && (glv_tag9F6F.Value[0] & 0xC0))	//DS Slot Management Control
	UCHAR flagIsNotEmpty, flag9F6FPermanent, flagDF62Volatile;
	flagIsNotEmpty = lenOfV;
	flag9F6FPermanent = (glv_tag9F6F.Value[0] & 0x80);
	flagDF62Volatile = (glv_tagDF62.Value[0] & 0x40);

	if (flagIsNotEmpty && flag9F6FPermanent && flagDF62Volatile)
		memset(OID, 0, 8);
	else
		memcpy(OID, glv_tag9F5C.Value, 8);	//DS Requested Operator ID

	//Create Message
	memcpy(M, iptC, 8);
	memcpy(&M[8], OID, 8);

	//Padding DS ID
	UT_Get_TLVLengthOfV(glv_tag9F5E.Length, &lenOfV);
	memcpy(&Y[11 - lenOfV], glv_tag9F5E.Value, lenOfV);	//DS ID

	//Create AES Key
	memcpy(K, Y, 11);
	memcpy(&K[11], &OID[4], 4);
	K[15]=0x3F;

	//Compute T
	api_aes_encipher(M, T, K, 16);

	for (i=0; i<16; i++)
		T[i]^=M[i];

	//Compute R
	memcpy(optR, T, 8);
}


void MPP_Generate_DD_CVC3(UCHAR datLen, UCHAR *disData, UCHAR trkNum)
{
	UCHAR	bmpCVC3[6]={0};		//Bitmap of CVC3
	UCHAR	bmpBytNum=0;		//Byte Number of Bitmap CVC3
	UINT	numCVC3=0;			//Number of non-zero bits of CVC3
	UCHAR	ascCVC3[48]={0};	//Ascii format of CVC3
	UINT	intCVC3=0;			//Integer format of CVC3

	UCHAR	*ptrAscii;
	UCHAR	idxNum=0;
	UCHAR	idxBit=1;
	UINT	cntNum=0;


	//Initial Data
	if (trkNum == MPP_TRACK_1)
	{
		bmpBytNum=ECL_LENGTH_9F62;
		memcpy(bmpCVC3, glv_tag9F62.Value, bmpBytNum);
		intCVC3=glv_tag9F60.Value[0]*256+glv_tag9F60.Value[1];
	}
	else if (trkNum == MPP_TRACK_2)
	{
		bmpBytNum=ECL_LENGTH_9F65;
		memcpy(&bmpCVC3[4], glv_tag9F65.Value, bmpBytNum);	//Align to Rigth
		intCVC3=glv_tag9F61.Value[0]*256+glv_tag9F61.Value[1];
	}

	//Convert CVC3 to Ascii
	memset(ascCVC3, 0x30, 48);
	UT_INT2ASC(intCVC3, &ascCVC3[38]);

	//Set Pointer to Least Significant Byte of Ascii
	ptrAscii=&ascCVC3[47];

	//Count Number of non-zero bits in PCVC3
	numCVC3=MPP_Get_NumberOfNonZeroBits(6, bmpCVC3);

	//Copy CVC3
	for (idxNum=0; idxNum < (bmpBytNum*8); idxNum++)
	{
		if (bmpCVC3[5-(idxNum/8)] & idxBit)
		{
			disData[(datLen-1)-idxNum]=*ptrAscii;

			ptrAscii--;
			cntNum++;

			if (cntNum == numCVC3)
				break;
		}

		(idxBit == 128)?(idxBit=1):(idxBit<<=1);

		if ((idxNum+1) == datLen)
			break;
	}
}


void MPP_Generate_DD_UN(UCHAR datLen, UCHAR *disData, UCHAR trkNum)
{
	UCHAR	bmpUNATC[6]={0};	//Bitmap of UNATC
	UCHAR	bmpBytNum=0;		//Byte Number of Bitmap UNATC
	UCHAR	numNATC=0;			//Number of ATC in PUNATC
	UINT	numUNATC=0;			//Number of Non-zero bits in PUNATC
	UCHAR	ascUN[8]={0};		//Ascii format of UN

	UCHAR	*ptrAscii;
	UCHAR	idxNum=0;
	UCHAR	idxBit=1;
	UCHAR	cntNum=0;


	//Initial Data
	if (trkNum == MPP_TRACK_1)
	{
		bmpBytNum=ECL_LENGTH_9F63;
		memcpy(bmpUNATC, glv_tag9F63.Value, bmpBytNum);
		numNATC=glv_tag9F64.Value[0];
	}
	else if (trkNum == MPP_TRACK_2)
	{
		bmpBytNum=2;	//Length of 9F66 VISA=4, MasterCard=2.
		memcpy(&bmpUNATC[4], glv_tag9F66.Value, bmpBytNum);	//Align to Rigth
		numNATC=glv_tag9F67.Value[0];
	}

	//Convert UN
	UT_Split(ascUN, glv_tag9F6A.Value, ECL_LENGTH_9F6A);

	//Set Pointer to Ascii Data
	ptrAscii=&ascUN[7];

	//Count Number of non-zero bits in PUNATC
	numUNATC=MPP_Get_NumberOfNonZeroBits(6, bmpUNATC);

	//Count nUN
	mpp_tagnUN.Value[0]=numUNATC-numNATC;
	if (mpp_tagnUN.Value[0] == 0)
		return;

	//Copy UN
	for (idxNum=0; idxNum < (bmpBytNum*8); idxNum++)
	{
		if (bmpUNATC[5-(idxNum/8)] & idxBit)
		{
			disData[(datLen-1)-idxNum]=*ptrAscii;

			ptrAscii--;
			cntNum++;

			if (cntNum == mpp_tagnUN.Value[0])
				break;
		}

		(idxBit == 128)?(idxBit=1):(idxBit<<=1);

		if ((idxNum+1) == datLen)
			break;
	}
}


void MPP_Generate_DD_ATC(UCHAR datLen, UCHAR *disData, UCHAR trkNum)
{
	UCHAR	bmpUNATC[6]={0};	//Bitmap of UNATC
	UCHAR	bmpBytNum=0;		//Byte Number of Bitmap UNATC
	UCHAR	numNATC=0;			//Number of non-zero bits of NATC
	UCHAR	ascATC[10]={0};		//Ascii format of ATC
	UINT	intATC=0;			//Integer format of ATC
	UCHAR	flgATC=0;			//Flag of ATC Coping

	UCHAR	*ptrAscii;
	UCHAR	idxNum=0;
	UCHAR	idxBit=1;
	UCHAR	cntNum=0;


	//Initial Data
	if (trkNum == MPP_TRACK_1)
	{
		bmpBytNum=ECL_LENGTH_9F63;
		memcpy(bmpUNATC, glv_tag9F63.Value, bmpBytNum);
		numNATC=glv_tag9F64.Value[0];
	}
	else if (trkNum == MPP_TRACK_2)
	{
		bmpBytNum=2;	//Length of 9F66 VISA=4, MasterCard=2.
		memcpy(&bmpUNATC[4], glv_tag9F66.Value, bmpBytNum);	//Align to Rigth
		numNATC=glv_tag9F67.Value[0];
	}

	if (numNATC == 0)
		return;

	//Convert ATC
	intATC=glv_tag9F36.Value[0]*256+glv_tag9F36.Value[1];
	UT_INT2ASC(intATC, ascATC);

	//Set Pointer to Ascii Data
	ptrAscii=&ascATC[9];

	//Skip UN bits and Start Coping ATC
	if (mpp_tagnUN.Value[0] == 0)
		flgATC=TRUE;

	for (idxNum=0; idxNum < (bmpBytNum*8); idxNum++)	//Length of bmpCVC3 * Bits in 1 Byte
	{
		if (bmpUNATC[5-(idxNum/8)] & idxBit)
		{
			if (flgATC == TRUE)
			{
				disData[(datLen-1)-idxNum]=*ptrAscii;

				ptrAscii--;
				cntNum++;	//Count ATC bits

				if (cntNum == numNATC)
					break;
			}
			else
			{
				cntNum++;	//Count UN bits

				if (cntNum == mpp_tagnUN.Value[0])
				{
					flgATC=TRUE;	//Start Coping ATC
					cntNum=0;
				}
			}
		}

		(idxBit == 128)?(idxBit=1):(idxBit<<=1);

		if ((idxNum+1) == datLen)
			break;
	}
}


void MPP_Generate_DD_nUN(UCHAR datLen, UCHAR *disData, UCHAR numUN)
{
	if (numUN < 10)
	{
		disData[datLen-1]=numUN + '0';	//Convert to Ascii
	}
}


UCHAR MPP_Generate_DD(UCHAR datLen, UCHAR *disData, UCHAR numUN, UCHAR trkNum)
{
	UCHAR	tmpDisLen=0;
	UCHAR	tmpDisData[48]={0};

	//Length of DD Can't Exceed Max. Length of PCVC3 * 8
	if (datLen > (6 * 8))
		return FAIL;

	if (trkNum == MPP_TRACK_1)
	{
		memcpy(tmpDisData, disData, datLen);
		tmpDisLen=datLen;
	}
	else if (trkNum == MPP_TRACK_2)
	{
		UT_Split(tmpDisData, disData, datLen);

		//Exclude Padding Word
		if (tmpDisData[datLen*2-1] == 'F')
			tmpDisLen=datLen*2-1;
		else
			tmpDisLen=datLen*2;
	}
	else
	{
		return FAIL;
	}

	MPP_DBG_Put_String(3, (UCHAR*)"RAW");
	MPP_DBG_Put_String(tmpDisLen, tmpDisData);

	MPP_Generate_DD_CVC3(tmpDisLen, tmpDisData, trkNum);

	MPP_DBG_Put_String(3, (UCHAR*)"CVC");
	MPP_DBG_Put_String(tmpDisLen, tmpDisData);

	MPP_Generate_DD_UN(tmpDisLen, tmpDisData, trkNum);

	MPP_DBG_Put_String(3, (UCHAR*)"UN ");
	MPP_DBG_Put_String(tmpDisLen, tmpDisData);

	MPP_Generate_DD_ATC(tmpDisLen, tmpDisData, trkNum);

	MPP_DBG_Put_String(3, (UCHAR*)"ATC");
	MPP_DBG_Put_String(tmpDisLen, tmpDisData);

	MPP_Generate_DD_nUN(tmpDisLen, tmpDisData, numUN);

	MPP_DBG_Put_String(3, (UCHAR*)"nUN");
	MPP_DBG_Put_String(tmpDisLen, tmpDisData);	

	if (trkNum == MPP_TRACK_1)
	{
		memcpy(disData, tmpDisData, tmpDisLen);
	}
	else if (trkNum == MPP_TRACK_2)
	{
		if (tmpDisLen & 1)
			tmpDisLen=tmpDisLen/2+1;
		else
			tmpDisLen=tmpDisLen/2;

		UT_Compress(disData, tmpDisData, tmpDisLen);
	}

	return SUCCESS;
}


void MPP_Generate_Y(UCHAR *optLen, UCHAR *optData)
{
	UCHAR	ascPAN[ECL_LENGTH_5A*2]={0};
	UCHAR	ascSequence[2]={0};
	UCHAR	ascY[ECL_LENGTH_5A*2+2]={0};
	UINT	lenOfV=0;
	UCHAR	lenPAN=0;
	UCHAR	idxNum=0;
	UCHAR	rspCode=FALSE;

	//Convert to ASCII
	UT_Get_TLVLengthOfV(glv_tag5A.Length, &lenOfV);
	UT_Split(ascPAN, glv_tag5A.Value, lenOfV);

	//Count Application PAN Length
	if (lenOfV != 0)
	{
		while (glv_tag5A.Value[lenOfV-1] == 0xFF)	//william	2017/11/22  case:  PAN = (7 byte)11 22 33 44 55 6F FF => actual 6 byte
		{
			lenOfV--;
		}

		if (ascPAN[lenOfV*2-1] == 'F')
		{
			lenPAN=lenOfV*2-1;
		}
		else
		{
			lenPAN=lenOfV*2;
		}
	}

	//Check Application PAN Sequence Number
	rspCode=MPP_IsPresent(mpp_tof5F34);
	if (rspCode == TRUE)
	{
		//memset(ascSequence, 0, 2);						//william 2017/11/04  reverse 
		UT_Split(ascSequence, glv_tag5F34.Value, 1);
	}
	else
	{
		//UT_Split(ascSequence, glv_tag5F34.Value, 1);
		//memset(ascSequence, 0, 2);
		memset(ascSequence, 0x30, 2);		// william 2017/11/22		shold be 0x30 = '0' not 'null'
	}

	if ((lenPAN+2) < (16-1))	//(lenPAN+lenSequence) < (8*2 Byte - 1 Odd Padding)
	{
		//Pad 0x00 to Left
		for (idxNum=0; idxNum < (16-(lenPAN+2)); idxNum++)
		{
			ascY[idxNum]='0';
		}

		memcpy(&ascY[idxNum], ascPAN, lenPAN);
		memcpy(&ascY[idxNum+lenPAN], ascSequence, 2);

		optLen[0]=8;
	}
	else
	{
		//Don't Pad 0x00 to Left
		if (lenPAN & 1)
		{
			ascY[0]='0';
			memcpy(&ascY[1], ascPAN, lenPAN);
			memcpy(&ascY[1+lenPAN], ascSequence, 2);

			optLen[0]=(1+lenPAN+2)/2;
		}
		else
		{
			memcpy(&ascY[0], ascPAN, lenPAN);
			memcpy(&ascY[lenPAN], ascSequence, 2);

			optLen[0]=(lenPAN+2)/2;
		}
	}
	UT_Compress(optData, ascY, optLen[0]);
}


void MPP_Get_DDLength_Track2(UCHAR iptLen, UCHAR *optLen, UCHAR disPosition)
{
	if ((disPosition == MPP_DD_START_MSB) && ((iptLen & 1) == 0))
	{
		optLen[0]=(iptLen/2);
	}
	else 
	{
		optLen[0]=(iptLen/2)+1;
	}
}


void MPP_Copy_DD_Track2(UCHAR *disTrack2, UCHAR disLen, UCHAR *disData, UCHAR disPosition)
{
	UCHAR	tmpByte=0;
	
	if (disPosition == MPP_DD_START_MSB)
	{
		if (disLen & 1)
			memcpy(disTrack2, disData, (disLen/2+1));
		else
			memcpy(disTrack2, disData, (disLen/2));
	}
	if (disPosition == MPP_DD_START_LSB)
	{
		tmpByte=disTrack2[0];

		memcpy(disTrack2, disData, (disLen/2+1));

		disTrack2[0]&=0x0F;
		disTrack2[0]|=(tmpByte & 0xF0);
	}
}


UCHAR *MPP_Search_ListItem(UCHAR *iptTag, UCHAR *lstData, UINT *optRmdDatLen, UCHAR lstType)
{
	UCHAR	lenOfT=0;			//Length of TLV-T
	UCHAR	lenOfL=0;			//Length of TLV-L
	UINT	lenOfV=0;			//Length of TLV-V
	UINT	lenOfList=0;		//Length of List
	UINT	lenOfItem=0;		//Length of Item
	UINT	lenOfData=0;		//Length of Processing Data
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=0;

	rspCode=UT_Get_TLVLengthOfV(lstData-3, &lenOfList);	//Decrease 3 to Point to Length
	if (rspCode == FAIL)
		return NULLPTR;

	//Empty List
	if (lenOfList == 0)
		return NULLPTR;

	//Set Pointer to Start of List
	ptrData=lstData;

	do {
		if (lstType == MPP_LISTTYPE_TAG)
		{
			rspCode=UT_Get_TLVLengthOfT(ptrData, &lenOfT);
			lenOfItem=lenOfT;
		}
		else if (lstType == MPP_LISTTYPE_TLV)
		{
			rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
			lenOfItem=lenOfT+lenOfL+lenOfV;
		}

		if (rspCode == FAIL)
			return NULLPTR;

		if (!memcmp(ptrData, iptTag, lenOfT))
		{
			optRmdDatLen[0]=lenOfList-lenOfData;

			return ptrData;
		}

		//Point to Next Tag
		ptrData+=lenOfItem;
		lenOfData+=lenOfItem;
	} while (lenOfData < lenOfList);

	return NULLPTR;
}


UCHAR *MPP_Search_GetDataTag(UCHAR *lstData, UCHAR iptLstType)
{
	UCHAR	lenOfT=0;			//Length of TLV-T
	UCHAR	lenOfL=0;			//Length of TLV-L
	UINT	lenOfV=0;			//Length of TLV-V
	UINT	lenOfList=0;		//Length of List
	UINT	lenOfItem=0;		//Length of Item
	UINT	lenOfData=0;		//Length of Processing Data
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=FALSE;

	//Get List Length
	rspCode=UT_Get_TLVLengthOfV(lstData-3, &lenOfList);	//Decrease 3 to Point to Length
	if (rspCode == FAIL)
		return NULLPTR;

	//Set Pointer to Start of Data
	ptrData=lstData;

	do {
		//Get List Tag Length
		if (iptLstType == MPP_LISTTYPE_TAG)
		{
			UT_Get_TLVLengthOfT(lstData, &lenOfT);
			lenOfItem=lenOfT;
		}
		else if (iptLstType == MPP_LISTTYPE_TLV)
		{
			UT_Get_TLVLength(lstData, &lenOfT, &lenOfL, &lenOfV);
			lenOfItem=lenOfT+lenOfL+lenOfV;
		}

		if (lenOfT == 2)			// william 2017/11/20  add 9F50
			if(lstData[0] == 0x9F)
				if (    ((lstData[1] >= 0x70) && (lstData[1] <= 0x79))    ||     (lstData[1] == 0x50)  )
					return lstData;

		//Point to Next Tag
		lstData+=lenOfItem;
		lenOfData+=lenOfItem;
	} while (lenOfData < lenOfList);

	return NULLPTR;
}


UCHAR MPP_Search_TornRecord_PAN(UCHAR *optRecIndex)

{
	UINT	lenOfPAN=0;
	UCHAR	idxNum=0;
	UCHAR	idxAID=0xFF;
	UCHAR	rtcDatTime[12]={0};
	UCHAR	trnDatTime[12]={0};
	ULONG	rtcUnxTime=0;
	ULONG	trnUnxTime=0;
	ULONG	difUnxTime=0;
	UCHAR	flgMatch=FALSE;

	api_rtc_getdatetime(0, rtcDatTime);
	rtcUnxTime=UT_Get_UnixTime(rtcDatTime);
	
	UT_Get_TLVLengthOfV(glv_tag5A.Length, &lenOfPAN);

	idxAID=MPP_Get_TornRecordAIDIndex();

	if (idxAID != 0xFF)
	{
		for (idxNum=0; idxNum < MPP_TORN_RECORDNUMBER; idxNum++)
		{
			if (mpp_trnRec[idxAID][idxNum].PANLen != 0)
			{
				UT_Split(trnDatTime, mpp_trnRec[idxAID][idxNum].DateTime, 6);
				trnUnxTime=UT_Get_UnixTime(trnDatTime);

				//Check if Application PAN Sequence Number is not empty
				//if (mpp_trnRec[idxAID][idxNum].PSN != 0)		// william 2017/11/28	
				if(glv_tag5F34.Length[0] != 0)
				{
					if ((mpp_trnRec[idxAID][idxNum].PANLen == lenOfPAN) &&
						(!memcmp(mpp_trnRec[idxAID][idxNum].PAN, glv_tag5A.Value, lenOfPAN)) &&
						(mpp_trnRec[idxAID][idxNum].PSN == glv_tag5F34.Value[0]))
					{
						//Find Recent Record
						if (flgMatch == FALSE)
						{
							optRecIndex[0]=idxNum;
							difUnxTime=rtcUnxTime-trnUnxTime;

							flgMatch=TRUE;
						}
						else
						{
							if ((rtcUnxTime - trnUnxTime) < difUnxTime)
							{
								optRecIndex[0]=idxNum;
								difUnxTime=rtcUnxTime-trnUnxTime;
							}
						}
					}
				}
				else
				{
					if((mpp_trnRec[idxAID][idxNum].PANLen == lenOfPAN) &&
					   (!memcmp(mpp_trnRec[idxAID][idxNum].PAN, glv_tag5A.Value, lenOfPAN)) &&
					   (mpp_trnRec[idxAID][idxNum].PSN == 0))
					{
						//Find Recent Record
						if (flgMatch == FALSE)
						{
							optRecIndex[0]=idxNum;
							difUnxTime=rtcUnxTime-trnUnxTime;

							flgMatch=TRUE;
						}
						else
						{
							if ((rtcUnxTime - trnUnxTime) < difUnxTime)
							{
								optRecIndex[0]=idxNum;
								difUnxTime=rtcUnxTime-trnUnxTime;
							}
						}
					}
				}
			}
		}
	}

	if (flgMatch == FALSE)
		return FAIL;

	return SUCCESS;
}


UCHAR MPP_Search_TornRecord_Oldest(UCHAR *optRecIndex)
{
	UCHAR	idxNum=0;
	UCHAR	idxAID=0xFF;
	UCHAR	rtcDatTime[12]={0};
	UCHAR	trnDatTime[12]={0};
	ULONG	rtcUnxTime=0;
	ULONG	trnUnxTime=0;
	ULONG	difUnxTime=0;
	UCHAR	flgMatch=FALSE;

	api_rtc_getdatetime(0, rtcDatTime);
	rtcUnxTime=UT_Get_UnixTime(rtcDatTime);
	
	idxAID=MPP_Get_TornRecordAIDIndex();
	if (idxAID != 0xFF)
	{
		for (idxNum=0; idxNum < MPP_TORN_RECORDNUMBER; idxNum++)
		{
			if (mpp_trnRec[idxAID][idxNum].PANLen != 0)
			{
				UT_Split(trnDatTime, mpp_trnRec[idxAID][idxNum].DateTime, 6);
				trnUnxTime=UT_Get_UnixTime(trnDatTime);

				//Find Oldest Record
				if (flgMatch == FALSE)
				{
					optRecIndex[0]=idxNum;
					difUnxTime=rtcUnxTime-trnUnxTime;

					flgMatch=TRUE;
				}
				else
				{
					if ((rtcUnxTime - trnUnxTime) > difUnxTime)
					{
						optRecIndex[0]=idxNum;
						difUnxTime=rtcUnxTime-trnUnxTime;
					}
				}
			}
		}
	}

	if (flgMatch == FALSE)
		return FAIL;

	return SUCCESS;
}


UCHAR MPP_Retrieve_PK_CA(
	UCHAR	*iptRID, 
	UCHAR	iptIndex, 
	UCHAR	*optModLen,
	UCHAR	*optModulus,
	UCHAR	*optExponent)
{
	UCHAR	idxNum=0;

	MPP_DBG_Put_String(5, (UCHAR*)"RtvCA");
	
	//[Book 2 6.2]
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


UCHAR MPP_Retrieve_PK_Issuer(
	UCHAR	iptModLen, 
	UCHAR	*iptModulus, 
	UCHAR	*iptExponent, 
	UCHAR	*optModLen, 
	UCHAR	*optModulus,
	UCHAR	*optExponent)
{
	UINT	lenOfV=0;
	UINT	lenOfIPKC=0;				//Length of Issuer Public Key Certificate
	UINT	lenOfIPKR=0;				//Length of Issuer Public Key Remainder
	UINT	lenOfIPKL=0;				//Length of Issuer Public Key Length
	UINT	lenOfIPKE=0;				//Length of Issuer Public Key Exponent
	UINT	lenOfLMD=0;					//Length of Leftmost Digits of the Issuer Public Key
	UCHAR	bufCertificate[2+256]={0};	//Len(2)+Data(256)
	UCHAR	bufHash[512]={0};			//Hash Buffer
	UCHAR	bufModulus[2+256]={0};		//Modulus Buffer
	UCHAR	rstHash[20]={0};			//Result of Hash
	UINT	lenOfHash=0;				//Length of Hash Data
	UCHAR	rspCode=FALSE;

	MPP_DBG_Put_String(6, (UCHAR*)"RtrISS");

	//[Book 2 6.3]
	//Step 1: Check Issuer Public Key Certificate Length = CAPK Modulus Length
	UT_Get_TLVLengthOfV(glv_tag90.Length, &lenOfV);
	lenOfIPKC=lenOfV;
	
	if (iptModLen != lenOfIPKC)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR *)"Step 1");

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
	memcpy(&bufCertificate[2], glv_tag90.Value, lenOfIPKC);

	rspCode=api_rsa_recover(bufCertificate, bufCertificate);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	MPP_Check_api_rsa_recover(bufCertificate[0], &bufCertificate[2]);

	if (bufCertificate[2+lenOfIPKC-1] != 0xBC)	//Decrease 1 to Point to Trailer
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR *)"Step 2");

	//Step 3: Check Header = 0x6A
	if (bufCertificate[2+0] != 0x6A)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR *)"Step 3");

	//Step 4: Check Certificate Format = 0x02
	if (bufCertificate[2+1] != 0x02)	//Add 1 to Point to Certificate Format
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR *)"Step 4");

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
			return FAIL;
		}
	}

	UT_Get_TLVLengthOfV(glv_tag9F32.Length, &lenOfV);
	lenOfIPKE=lenOfV;

	if (lenOfIPKE == 0)
	{
		return FAIL;
	}

	memcpy(&bufHash[lenOfHash], glv_tag9F32.Value, lenOfIPKE);
	lenOfHash+=lenOfIPKE;

	MPP_DBG_Put_String(6, (UCHAR *)"Step 5");

	//Step 6: Apply Hash Algorithm
	rspCode=api_sys_SHA1(lenOfHash, bufHash, rstHash);
	if (rspCode == apiFailed)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR *)"Step 6");

	//Step 7: Compare Hash Result
	rspCode=UT_bcdcmp(&bufCertificate[2+lenOfIPKC-(20+1)], rstHash, 20); //Hash Result(20)+Trailer(1)
	if (rspCode != 0)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR *)"Step 7");

	//Step 8: Verify the Issuer Identifier = PAN
	rspCode=UT_CNcmp(&bufCertificate[2+2], glv_tag5A.Value, 4);	//Add 2 to Point to Issuer Identifier
	if (rspCode == FALSE)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR *)"Step 8");

	//Step 9: Verify Certificate Expiration Date
	rspCode=UT_VerifyCertificateExpDate(&bufCertificate[2+6]);	//Add 6 to Point to Certificate Expiration Date
	if (rspCode == FALSE)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR *)"Step 9");

	//Step 10: Verify Certification Revocation List
	rspCode=MPP_Check_CRL(glv_tag9F06.Value, glv_tag8F.Value[0], &bufCertificate[2+8]);	//Add 8 to Point to Certificate Serial Number
	if (rspCode == TRUE)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(7, (UCHAR *)"Step 10");

	//Step 11: Check Issuer Public Key Algorithm Indicator = 0x01
	if (bufCertificate[2+12] != 0x01)	//Add 12 to Point to Issuer Public Key Algorithm Indicator
	{
		return FAIL;
	}

	MPP_DBG_Put_String(7, (UCHAR *)"Step 11");

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

	MPP_DBG_Put_String(7, (UCHAR *)"Step 12");

	return SUCCESS;
}


UCHAR MPP_Retrieve_PK_ICC(
	UCHAR	iptModLen, 
	UCHAR	*iptModulus, 
	UCHAR	*iptExponent, 
	UCHAR	*optModLen, 
	UCHAR	*optModulus, 
	UCHAR	*optExponent)
{
	UINT	lenOfV=0;
	UINT	lenOfIPKC=0;				//Length of ICC Public Key Certificate
	UINT	lenOfIPKR=0;				//Length of ICC Public Key Remainder
	UINT	lenOfIPKL=0;				//Length of ICC Public Key Length
	UINT	lenOfIPKE=0;				//Length of ICC Public Key Exponent
	UINT	lenOfLMD=0;					//Length of Leftmost Digits of the ICC Public Key
	UCHAR	bufCertificate[2+256]={0};	//Len(2)+Data(256)
	UCHAR	bufHash[512+MPP_LENGTH_StaticDataToBeAuthenticated]={0};			//Hash Buffer
	UCHAR	bufModulus[2+256]={0};		//Modulus Buffer
	UCHAR	bufPAN[10]={0};				//PAN Buffer
	UCHAR	rstHash[20]={0};			//Result of Hash
	UINT	lenOfHash=0;				//Length of Hash Data
	UCHAR	rspCode=FALSE;


	MPP_DBG_Put_String(6, (UCHAR*)"RtvICC");

	//[Book 2 6.4]
	//Step 1: Check ICC Public Key Certificate Length = Issuer Public Key Modulus Length
	UT_Get_TLVLengthOfV(glv_tag9F46.Length, &lenOfV);
	lenOfIPKC=lenOfV;
	
	if (lenOfIPKC != iptModLen)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 1");

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

	MPP_Check_api_rsa_recover(bufCertificate[0], &bufCertificate[2]);

	if (bufCertificate[2+lenOfIPKC-1] != 0xBC)	//Decrease 1 to Point to Trailer
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 2");

	//Step 3: Check Header = 0x6A
	if (bufCertificate[2+0] != 0x6A)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 3");

	//Step 4: Check Certificate Format = 0x04
	if (bufCertificate[2+1] != 0x04)	//Add 1 to Point to Certificate Format
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 4");

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
		return FAIL;
	}
	else
	{
		memcpy(&bufHash[lenOfHash], glv_tag9F47.Value, lenOfIPKE);
		lenOfHash+=lenOfIPKE;
	}

	UT_Get_TLVLengthOfV(mpp_tagStaticDataToBeAuthenticated.Length, &lenOfV);
	memcpy(&bufHash[lenOfHash], mpp_tagStaticDataToBeAuthenticated.Value, lenOfV);
	lenOfHash+=lenOfV;

	MPP_DBG_Put_String(6, (UCHAR*)"Step 5");

	//Step 6: Apply Hash Algorithm
	rspCode=api_sys_SHA1(lenOfHash, bufHash, rstHash);
	if (rspCode == apiFailed)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 6");

	//Step 7: Compare Hash Result
	rspCode=UT_bcdcmp(&bufCertificate[2+lenOfIPKC-(20+1)], rstHash, 20); //Hash Result(20)+Trailer(1)
	if (rspCode != 0)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 7");

	//Step 8: Compare PAN
	memset(bufPAN, 0xFF, 10);
	UT_Get_TLVLengthOfV(glv_tag5A.Length, &lenOfV);
	memcpy(bufPAN, glv_tag5A.Value, lenOfV);

	rspCode=UT_CNcmp2(&bufCertificate[2+2], bufPAN, 10);	//Add 2 to Point to Application PAN
	if (rspCode == FALSE)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 8");

	//Step 9: Verify Certificate Expiration Date
	rspCode=UT_VerifyCertificateExpDate(&bufCertificate[2+12]);	//Add 12 to Point to Certificate Expiration Date
	if (rspCode == FALSE)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 9");

	//Step 10: Check ICC Public Key Algorithm Indicator = 0x01
	if (bufCertificate[2+18] != 0x01)	//Add 18 to Point to ICC Public Key Algorithm Indicator
	{
		return FAIL;
	}

	MPP_DBG_Put_String(7, (UCHAR*)"Step 10");

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

	MPP_DBG_Put_String(7, (UCHAR*)"Step 11");

	return SUCCESS;
}


UCHAR MPP_Verify_DynamicSignature(
	UCHAR	iptModLen,
	UCHAR	*iptModulus,
	UCHAR	*iptExponent,
	UCHAR	flgIDSRead,
	UCHAR	flgRRP)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfTag77=0;
	UINT	lenOfRemainder=0;			
	UINT	lenOfData=0;				
	UINT	lenOfIDDL=0;				//Length of ICC Dynamic Data Length
	UINT	lenOfHash=0;				//Length of Hash Data
	UINT	lenOfRmvPadding=0;			//Length of Padding Removed Buffer
	UINT	lenOfSDAD=0;				//Length of SDAD
	UCHAR	lenOfIDNL=0;				//ICC Dynamic Number Length
	UCHAR	bufSDAD[2+256]={0};			//Len(2)+Data(256)
	UCHAR	bufHash[512]={0};			//Hash Buffer
	UCHAR	bufRmvPadding[512]={0};		//Padding Removed Buffer
	UCHAR	bufModulus[2+256]={0};		//Modulus Buffer
	UCHAR	rstHash[20]={0};			//Result of Hash	
	UCHAR	*ptrStart=NULLPTR;
	UCHAR	*ptrSDAD=NULLPTR;
	UCHAR	*ptrAftSDAD=NULLPTR;
	UCHAR	rspCode=FALSE;
	UCHAR	*ptrRelayData = NULLPTR;
	UCHAR	flagDSS2 = FALSE, flagDSS3 = FALSE;
	UINT	lenOfDSS1 = 0;

	MPP_DBG_Put_String(7, (UCHAR*)"VrfSDAD");

	//[Book 2 6.6.2]
	//Step 1: Check Signed Dynamic Application Data Length = ICC Public Key Modulus Length
	UT_Get_TLVLengthOfV(glv_tag9F4B.Length, &lenOfV);
	lenOfSDAD=lenOfV;
	
	if (lenOfSDAD != iptModLen)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 1");

	//Step 2: Recover SDAD and Check Trailer = 0xBC
	bufModulus[0]=iptModLen;
	memcpy(&bufModulus[2], iptModulus, iptModLen);
	rspCode=api_rsa_loadkey(bufModulus, iptExponent);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	bufSDAD[0]=lenOfSDAD & 0x00FF;
	bufSDAD[1]=(lenOfSDAD & 0xFF00) >> 8;
	memcpy(&bufSDAD[2], glv_tag9F4B.Value, lenOfSDAD);

	rspCode=api_rsa_recover(bufSDAD, bufSDAD);
	if (rspCode != apiOK)
	{
		return FAIL;
	}

	MPP_Check_api_rsa_recover(bufSDAD[0], &bufSDAD[2]);

	if (bufSDAD[2+lenOfSDAD-1] != 0xBC)	//Decrease 1 to Point to Trailer
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 2");

	//Step 3: Check Header = 0x6A
	if (bufSDAD[2+0] != 0x6A)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 3");

	//Step 4: Check Signed Data Format = 0x05
	if (bufSDAD[2+1] != 0x05)	//Add 1 to Point to Signed Data Format
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 4");

	//Step 5: Retrieve ICC Dynamic Data
	lenOfIDNL=bufSDAD[2+4];	//Add 4 to Point to ICC Dynamic Number Length

	MPP_DBG_Put_String(6, (UCHAR*)"Step 5");

	//Step 6: Check CID retrieved from the ICC Dynamic Data = CID obtained from the response to the GENERATE AC command
	if (bufSDAD[2+(5+lenOfIDNL)] != glv_tag9F27.Value[0])	//Add (5+lenOfIDNL) to Point to Cryptogram Information Data
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 6");

	//Step 7: Concatenation
	rspCode=MPP_Check_CDOL1(mpp_tof9F37);
	if (rspCode == FALSE)
	{
		return FAIL;
	}
	
	lenOfHash=lenOfSDAD-(1+20+1);				//Header(1)+Hash Result(20)+Trailer(1)
	memcpy(bufHash, &bufSDAD[2+1], lenOfHash);	//Start from Signed Data Format

	memcpy(&bufHash[lenOfHash], glv_tag9F37.Value, 4);
	lenOfHash+=4;

	MPP_DBG_Put_String(6, (UCHAR*)"Step 7");

	//Step 8: Apply Hash Algorithm
	rspCode=api_sys_SHA1(lenOfHash, bufHash, rstHash);
	if (rspCode == apiFailed)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 8");

	//Step 9: Compare Hash Result
	rspCode=UT_bcdcmp(&bufSDAD[2+lenOfSDAD-(20+1)], rstHash, 20); //Hash Result(20)+Trailer(1)
	if (rspCode != 0)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(6, (UCHAR*)"Step 9");

	//Step 10: Concatenation
	memset(bufHash, 0, 512);
	lenOfHash=0;

	//remarked by Tammy 201711/28
	//Copy PDOL Related Data
	//UT_Get_TLVLengthOfV(glv_tagDF8111.Length, &lenOfV);
	//memcpy(bufHash, glv_tagDF8111.Value, lenOfV);
	//lenOfHash+=lenOfV;

	//Copy PDOL	Tammy 2017/11/28
	UT_Get_TLVLengthOfL(&glv_tagDF8111.Value[1], &lenOfL);
	UT_Get_TLVLengthOfV(&glv_tagDF8111.Value[1], &lenOfV);
	memcpy(bufHash, &glv_tagDF8111.Value[1 + lenOfL], lenOfV);
	lenOfHash += lenOfV;

	//Copy CDOL1 Related Data
	UT_Get_TLVLengthOfV(glv_tagDF8107.Length, &lenOfV);
	memcpy(&bufHash[lenOfHash], glv_tagDF8107.Value, lenOfV);
	lenOfHash+=lenOfV;

	//Get Generate AC Response Record TLV Length
	UT_Get_TLVLength(mpp_vfyData, &lenOfT, &lenOfL, &lenOfV);
	lenOfTag77=lenOfV;
	ptrStart=mpp_vfyData+(lenOfT+lenOfL);	//Point to TLV-V

	//Exclude Signed Dynamic Application Data
	ptrSDAD=UT_Find_Tag(mpp_tof9F4B, lenOfV, ptrStart);
	if (ptrSDAD != NULLPTR)
	{
		lenOfData=ptrSDAD-ptrStart;
		if (lenOfData != 0)
		{
			rspCode=UT_Remove_PaddingData((ptrSDAD-ptrStart), ptrStart, &lenOfRmvPadding, bufRmvPadding);
			if (rspCode == FAIL)
				return FAIL;

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
				return FAIL;
			
			memcpy(&bufHash[lenOfHash], bufRmvPadding, lenOfRmvPadding);
			lenOfHash+=lenOfRmvPadding;
		}
	}
	else
	{
		return FAIL;
	}
	
	MPP_DBG_Put_String(7, (UCHAR*)"Step 10");

	//Step 11: Apply Hash Algorithm
	memset(rstHash, 0, 20);
	rspCode=api_sys_SHA1(lenOfHash, bufHash, rstHash);
	if (rspCode == apiFailed)
	{
		return FAIL;
	}

	MPP_DBG_Put_String(7, (UCHAR*)"Step 11");

	//Step 12: Compare Transaction Data Hash Code
	rspCode=UT_bcdcmp(&bufSDAD[2+5+lenOfIDNL+9], rstHash, 20);	//Add 14+lenOfIDNL to Point to Transaction Data Hash Code
	if (rspCode != 0)
	{
		return FAIL;
	}

	if ((lenOfIDNL >= 2) && (lenOfIDNL <= 8))
	{
		UT_Set_TagLength(lenOfIDNL, glv_tag9F4C.Length);
		memcpy(glv_tag9F4C.Value, &bufSDAD[2+5], lenOfIDNL);	//Add 5 to Point to ICC Dynamic Number
		MPP_AddToList(mpp_tof9F4C, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	UT_Set_TagLength(8, glv_tag9F26.Length);
	memcpy(glv_tag9F26.Value, &bufSDAD[2+5+lenOfIDNL+1], 8);	//Add 6+lenOfIDNL to Point to TC or ARQC
	MPP_AddToList(mpp_tof9F26, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	lenOfIDDL = bufSDAD[2 + 3];

	//Retrieve from the ICC Dynamic Data the DS Summary 2 and DS Summary 3
	if(flgIDSRead == TRUE)
	{
		if(flgRRP == TRUE)	//ICC Dynamic Data includes DS Summary 2 and DS Summary 3
		{
			//S910.3.1 and S11.42.1
			//if (glv_tag9F5D.Value[0] == MPP_ACI_DSVersionNumber_Version1)
			if(glv_tag9F5D.Value[0] & MPP_ACI_DSVersionNumber_Version1)
			{
				if(lenOfIDDL < (60 + lenOfIDNL))
				{
					return FAIL;
				}

				//ICC Dynamic Data includes DS Summary 2 and DS Summary 3
				flagDSS2 = TRUE;
				flagDSS3 = TRUE;
				UT_Set_TagLength(8, glv_tagDF8101.Length);
				memcpy(glv_tagDF8101.Value, &bufSDAD[2 + 4 + 30 + lenOfIDNL], 8);

				UT_Set_TagLength(8, glv_tagDF8102.Length);
				memcpy(glv_tagDF8102.Value, &bufSDAD[2 + 4 + 30 + lenOfIDNL + 8], 8);

				ptrRelayData = &bufSDAD[2 + 4 + 30 + lenOfIDNL + 8 + 8];	//Point to start of relay resistance related data objects
			}
			//else if(glv_tag9F5D.Value[0] == MPP_ACI_DSVersionNumber_Version2)
			else if (glv_tag9F5D.Value[0] & MPP_ACI_DSVersionNumber_Version2)
			{
				if(lenOfIDDL < (76 + lenOfIDNL))
				{
					return FAIL;
				}

				//ICC Dynamic Data includes DS Summary 2 and DS Summary 3
				flagDSS2 = TRUE;
				flagDSS3 = TRUE;
				UT_Set_TagLength(16, glv_tagDF8101.Length);
				memcpy(glv_tagDF8101.Value, &bufSDAD[2 + 4 + 30 + lenOfIDNL], 16);

				UT_Set_TagLength(16, glv_tagDF8102.Length);
				memcpy(glv_tagDF8102.Value, &bufSDAD[2 + 4 + 30 + lenOfIDNL + 16], 16);

				ptrRelayData = &bufSDAD[2 + 4 + 30 + lenOfIDNL + 16 + 16];	//Point to start of relay resistance related data objects
			}
		}
		else  //ICC Dynamic Data may include DS Summary 2 and DS Summary 3
		{
			//S910.3 and S11.42
			if(lenOfIDDL < (30 + lenOfIDNL))
			{
				return FAIL;
			}

			lenOfRemainder = lenOfIDDL - (30 + lenOfIDNL);
			UT_Get_TLVLengthOfV(glv_tag9F7D.Length, &lenOfDSS1);

			if (lenOfRemainder != 0)
			{
				//if (glv_tag9F5D.Value[0] == MPP_ACI_DSVersionNumber_Version1)
				if (glv_tag9F5D.Value[0] & MPP_ACI_DSVersionNumber_Version1)
				{
					if (lenOfRemainder < 16)
					{
						//ICC Dynamic Data does not include DS Summary 3

						if (lenOfRemainder < 8)
						{
							;//ICC Dynamic Data does not include DS Summary 2
						}
						else
						{
							flagDSS2 = TRUE;
							UT_Set_TagLength(8, glv_tagDF8101.Length);
							memcpy(glv_tagDF8101.Value, &bufSDAD[2 + 4 + 30 + lenOfIDNL], 8);
						}
					}
					else
					{	
						flagDSS2 = TRUE;
						flagDSS3 = TRUE;

						//if (lenOfRemainder >= 32)
						//{
							UT_Set_TagLength(lenOfDSS1, glv_tagDF8101.Length);
							memcpy(glv_tagDF8101.Value, &bufSDAD[2 + 4 + 30 + lenOfIDNL], lenOfDSS1);
							UT_Set_TagLength(lenOfDSS1, glv_tagDF8102.Length);
							memcpy(glv_tagDF8102.Value, &bufSDAD[2 + 4 + 30 + lenOfIDNL + lenOfDSS1], lenOfDSS1);
						//}
						/*else if (16 <= lenOfRemainder && lenOfRemainder < 32)
						{
							UT_Set_TagLength(8, glv_tagDF8101.Length);
							memcpy(glv_tagDF8101.Value, &bufSDAD[2 + 4 + 30 + lenOfIDNL], 8);
							UT_Set_TagLength(8, glv_tagDF8102.Length);
							memcpy(glv_tagDF8102.Value, &bufSDAD[2 + 4 + 30 + lenOfIDNL + 8], 8);
						}*/
					}
				}
				else
				{
					//if (glv_tag9F5D.Value[0] == MPP_ACI_DSVersionNumber_Version2)
					if (glv_tag9F5D.Value[0] & MPP_ACI_DSVersionNumber_Version2)
					{
						if (lenOfRemainder < 32)
						{
							//ICC Dynamic Data does not include DS Summary 3

							if (lenOfRemainder < 16)
							{
								;//ICC Dynamic Data does not include DS Summary 2
							}
							else
							{
								flagDSS2 = TRUE;
								UT_Set_TagLength(16, glv_tagDF8101.Length);
								memcpy(glv_tagDF8101.Value, &bufSDAD[2 + 4 + 30 + lenOfIDNL], 16);
							}
						}
						else
						{
							flagDSS2 = TRUE;
							flagDSS3 = TRUE;
							UT_Set_TagLength(16, glv_tagDF8101.Length);
							memcpy(glv_tagDF8101.Value, &bufSDAD[2 + 4 + 30 + lenOfIDNL], 16);

							UT_Set_TagLength(16, glv_tagDF8102.Length);
							memcpy(glv_tagDF8102.Value, &bufSDAD[2 + 4 + 30 + lenOfIDNL + 16], 16);
						}
					}
				}
			}
		}
	}
	else
	{
		if(flgRRP == TRUE)
		{
			ptrRelayData = &bufSDAD[2 + 4 + 30 + lenOfIDNL];	//Point to start of relay resistance related data objects
		}
	}

	// william 2017/11/21	add DSS2/3 in TLV_DB & Presentlst
	if (flagDSS2)
		MPP_AddToList(mpp_tofDF8101, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	if (flagDSS3)
		MPP_AddToList(mpp_tofDF8102, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	
	//Check if the relay resistance related data objects in the ICC Dynamic Data match the corresponding data objects in the TLV Database
	if(flgRRP == TRUE)
	{
		//S910.3.1, S910.4.1, S11.42.1 and S11.43.1

		//Terminal Relay Resistance Entropy
		if(memcmp(glv_tagDF8301.Value, ptrRelayData, 4))
		{
			return FAIL;
		}
		ptrRelayData += 4;

		//Device Relay Resistance Entropy
		if(memcmp(glv_tagDF8302.Value, ptrRelayData, 4))
		{
			return FAIL;
		}
		ptrRelayData += 4;

		//Min Time For Processing Relay Resistance APDU
		if(memcmp(glv_tagDF8303.Value, ptrRelayData, 2))
		{
			return FAIL;
		}
		ptrRelayData += 2;

		//Max Time For Processing Relay Resistance APDU
		if(memcmp(glv_tagDF8304.Value, ptrRelayData, 2))
		{
			return FAIL;
		}
		ptrRelayData += 2;

		//Device Estimated Transmission Time For Relay Resistance R-APDU
		if(memcmp(glv_tagDF8305.Value, ptrRelayData, 2))
		{
			return FAIL;
		}
	}

	MPP_DBG_Put_String(7, (UCHAR*)"Step 12");

	return SUCCESS;
}


void MPP_Store_Tag(UCHAR lenOfL, UCHAR lenOfV, UCHAR *iptData, UINT iptTagIndex)
{
	//Store Length
	memcpy((UCHAR*)glv_addTable[iptTagIndex], iptData, lenOfL);

	//Store Value
	iptData+=lenOfL;	//Point to V of Tag
	memcpy((UCHAR*)glv_addTable[iptTagIndex]+3, iptData, lenOfV);
}


UCHAR MPP_Store_TLV(UINT lenOfTLV, UCHAR *iptData, UCHAR *iptList)
{
	UINT	lenOfV=0;
	UCHAR	rspCode=FAIL;

	rspCode=UT_Get_TLVLengthOfV(iptList-3, &lenOfV);		//Decrease 3 to Point to Length
	if (rspCode == SUCCESS)
	{
		if ((lenOfV+lenOfTLV) <= MPP_TLVLIST_BUFFERSIZE)
		{
			memcpy(iptList+lenOfV, iptData, lenOfTLV);		//Add Length of List to Point to End of List
			UT_Set_TagLength(lenOfV+lenOfTLV, iptList-3);	//Update Length

			return SUCCESS;
		}
	}

	return FAIL;
}


void MPP_Add_ListItem(UINT iptLstLen, UCHAR *iptLstItem, UCHAR *iptList)
{
	UINT	lenOfList=0;
	UCHAR	rspCode=FAIL;

	rspCode=UT_Get_TLVLengthOfV(iptList-3, &lenOfList);		//Decrease 3 to Point to Length
	if (rspCode == SUCCESS)
	{
		memcpy(iptList+lenOfList, iptLstItem, iptLstLen);	//Add Length of List to Point to End of List
		UT_Set_TagLength(lenOfList+iptLstLen, iptList-3);	//Update Length
	}
}


void MPP_Update_ListItem(UCHAR *iptLstItem, UCHAR *iptList)
{
    UCHAR	lenOfT=0;	//Length of TLV-T
	UCHAR	lenOfL=0;	//Length of TLV-L
	UINT	lenOfV=0;	//Length of TLV-V

	//Only TLV List Has to Update ListItem
	MPP_RemoveFromList(iptLstItem, iptList, MPP_LISTTYPE_TLV);

    UT_Get_TLVLength(iptLstItem, &lenOfT, &lenOfL, &lenOfV);
	MPP_Add_ListItem(lenOfT+lenOfL+lenOfV, iptLstItem, iptList);
}

UCHAR MPP_Update_ListItem2(UCHAR *iptLstItem, UCHAR *iptList, UCHAR lstType)	// with the order of tags unchange 2017/11/09
{
	UCHAR	tempList[3 + MPP_TLVLIST_BUFFERSIZE] = { 0 };
	ECL_TAG tempListBuff = { tempList,	3 + tempList };

	UCHAR	lenOfT = 0;			//Length of TLV-T
	UCHAR	lenOfL = 0;			//Length of TLV-L
	UINT	lenOfV = 0;			//Length of TLV-V
	UCHAR	lenOfT_Target = 0;			//Length of TLV-T
	UCHAR	lenOfL_Target = 0;			//Length of TLV-L
	UINT	lenOfV_Target = 0;			//Length of TLV-V
	UINT	lenOfList = 0;		//Length of List
	UINT	lenOfItem = 0;		//Length of Item
	UINT	lenOfData = 0;		//Length of Processing Data
	UCHAR	rspCode = FALSE;
	UCHAR	*ptrData = NULLPTR;
	UCHAR	flagFound = 0;

	//Get List Length
	rspCode = UT_Get_TLVLengthOfV(iptList - 3, &lenOfList);	//Decrease 3 to Point to Length
	if (rspCode == FAIL)
		return FAIL;

	////Empty List
	//if (lenOfList == 0)
	//	return FAIL;

	//Set Pointer to Start of List
	ptrData = iptList;
	while(lenOfData < lenOfList)
	{
		if (lstType == MPP_LISTTYPE_TAG)
		{
			rspCode = UT_Get_TLVLengthOfT(ptrData, &lenOfT);
			lenOfItem = lenOfT;
		}
		else if (lstType == MPP_LISTTYPE_TLV)
		{
			rspCode = UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
			lenOfItem = lenOfT + lenOfL + lenOfV;
		}
		if (rspCode == FAIL)
			return FAIL;

		if (!memcmp(ptrData, iptLstItem, lenOfT))		//if match, means we find the tag we want to update
		{
			UT_Get_TLVLength(iptLstItem, &lenOfT_Target, &lenOfL_Target, &lenOfV_Target);
			MPP_Add_ListItem(lenOfT_Target + lenOfL_Target + lenOfV_Target, iptLstItem, tempListBuff.Value);
			flagFound = TRUE;
		}
		else
		{
			//add current tag to new list
			MPP_Add_ListItem(lenOfItem, ptrData, tempListBuff.Value);
		}


		//Point to Next Tag
		ptrData += lenOfItem;
		lenOfData += lenOfItem;
	}
	
	//
	if (flagFound)
		;
	else
	{
		UT_Get_TLVLength(iptLstItem, &lenOfT_Target, &lenOfL_Target, &lenOfV_Target);
		MPP_Add_ListItem(lenOfT_Target + lenOfL_Target + lenOfV_Target, iptLstItem, tempListBuff.Value);
	}

	// Memmove NewList to OriginalList
	rspCode = UT_Get_TLVLengthOfV(tempListBuff.Length, &lenOfList);	//Decrease 3 to Point to Length
	memmove(iptList - 3, tempListBuff.Length, lenOfList + 3);

	return SUCCESS;
}

/*******************************************************************************
*	Initializes a List. This creates the List structure if it does not
*	exist, and initializes its contents to be empty, i.e. the List contains
*	no ListItems. This method can be called at any time during the operation
*	of the Kernel in order to clear and reset a list.
*******************************************************************************/
void MPP_Initialize_List(UCHAR *iptList)
{
	memset(iptList-3, 0, 3+MPP_TLVLIST_BUFFERSIZE);	//Decrease 3 to Point to Length
}


/*******************************************************************************
*	If ListItem is not included in List, then adds ListItem to the end of
*	List. Updates ListItem if it is already included in the List.
*******************************************************************************/
UCHAR MPP_AddToList(UCHAR *iptLstItem, UCHAR *iptList, UCHAR iptLstType)
{
	UCHAR	lenOfT=0;				//Length of TLV-T
	UCHAR	lenOfL=0;				//Length of TLV-L
	UINT	lenOfV=0;				//Length of TLV-V
	UINT	lenOfList=0;			//Length of List
	UINT	lenOfItem=0;			//Length of Item
	UINT	rmdDatLength=0;			//Length of Remainder Datat
	UCHAR	*ptrLstItem=NULLPTR;	
	UCHAR	rspCode=FALSE;

	//Get List Length
	rspCode=UT_Get_TLVLengthOfV(iptList-3, &lenOfList);	//Decrease 3 to Point to Length
	if (rspCode == FAIL)
		return FAIL;

	ptrLstItem=MPP_Search_ListItem(iptLstItem, iptList, &rmdDatLength, iptLstType);

	if (ptrLstItem != NULLPTR)
	{
		if (iptLstType == MPP_LISTTYPE_TLV)
			MPP_Update_ListItem(iptLstItem, iptList);

		return SUCCESS;
	}

	if (iptLstType == MPP_LISTTYPE_TAG)
	{
		rspCode=UT_Get_TLVLengthOfT(iptLstItem, &lenOfT);
		lenOfItem=lenOfT;
	}
	else if (iptLstType == MPP_LISTTYPE_TLV)
	{
		rspCode=UT_Get_TLVLength(iptLstItem, &lenOfT, &lenOfL, &lenOfV);
		lenOfItem=lenOfT+lenOfL+lenOfV;
	}

	if (rspCode == SUCCESS)
	{
		if ((lenOfList+lenOfItem) > MPP_TLVLIST_BUFFERSIZE)
			return FAIL;

		MPP_Add_ListItem(lenOfItem, iptLstItem, iptList);

		return SUCCESS;
	}

	return FAIL;
}


/*******************************************************************************
*	Removes ListItem from the List if ListItem is present in List. Ignores
*	otherwise.
*******************************************************************************/
void MPP_RemoveFromList(UCHAR *iptLstItem, UCHAR *iptList, UCHAR iptLstType)
{
	UINT	lenOfDelete=0;			//Length of Tag Deleted
	UCHAR	lenOfT=0;				//Length of TLV-T
	UCHAR	lenOfL=0;				//Length of TLV-L
	UINT	lenOfV=0;				//Length of TLV-V
	UINT	lenOfList=0;			//Length of List
	UINT	rmdDatLength=0;			//Remainder Data Length
	UCHAR	tmpBuffer[2048]={0};	//Temporary Buffer
	UCHAR	*ptrStrOfDelete=NULLPTR;//Start of Delete Pointer
	UCHAR	rspCode=0;

	//Get List Length
	UT_Get_TLVLengthOfV(iptList-3, &lenOfList);	//Decrease 3 to Point to Length

	//Save Start of Delete Position
	ptrStrOfDelete=MPP_Search_ListItem(iptLstItem, iptList, &rmdDatLength, iptLstType);
	if (ptrStrOfDelete == NULLPTR)
		return;

	//Get Length of Tag Going to Be Deleted
	if (iptLstType == MPP_LISTTYPE_TAG)
	{
		rspCode=UT_Get_TLVLengthOfT(ptrStrOfDelete, &lenOfT);
		lenOfDelete=lenOfT;
	}
	else
	{
		rspCode=UT_Get_TLVLength(ptrStrOfDelete, &lenOfT, &lenOfL, &lenOfV);
		lenOfDelete=lenOfT+lenOfL+lenOfV;
	}

	//Copy Remainder Tag to Temporary Buffer
	memcpy(tmpBuffer, &ptrStrOfDelete[lenOfDelete], rmdDatLength-lenOfDelete);

	//Reset Exisit Buffer
	memset(ptrStrOfDelete, 0, rmdDatLength);

	//Copy Remainder List Item
	memcpy(ptrStrOfDelete, tmpBuffer, rmdDatLength-lenOfDelete);

	//Update Length
	UT_Set_TagLength(lenOfList-lenOfDelete, iptList-3);	//Decrease 3 to Point to Length
}


/*******************************************************************************
*	Adds the ListItems in List1 that are not yet included in List2 to the
*	end of List2. Updates ListItems that are already included in List2.
*******************************************************************************/
UCHAR MPP_AddListToList(UCHAR *iptList1, UCHAR *iptList2, UCHAR iptLstType)
{
	UCHAR	*ptrList1=NULLPTR;	//Pointer of List1
	UCHAR	lenOfT=0;			//Length of TLV-T
	UCHAR	lenOfL=0;			//Length of TLV-L
	UINT	lenOfV=0;			//Length of TLV-V
	UINT	lenOfList1=0;		//Lenght of List1
	UINT	lenOfItem=0;		//Length of Item
	UINT	lenOfData=0;		//Length of Processing Data
	UCHAR	rspCode=0;

	//Get List Length
	rspCode=UT_Get_TLVLengthOfV(iptList1-3, &lenOfList1);	//Decrease 3 to Point to Length
	if (rspCode == FAIL)
		return FAIL;

	//Set Pointer to Start of List1
	ptrList1=iptList1;

	do {
		if (iptLstType == MPP_LISTTYPE_TAG)
		{
			rspCode=UT_Get_TLVLengthOfT(ptrList1, &lenOfT);
			lenOfItem=lenOfT;
		}
		else if (iptLstType == MPP_LISTTYPE_TLV)
		{
			rspCode=UT_Get_TLVLength(ptrList1, &lenOfT, &lenOfL, &lenOfV);
			lenOfItem=lenOfT+lenOfL+lenOfV;
		}

		if (rspCode == FAIL)
			return FAIL;

		rspCode=MPP_AddToList(ptrList1, iptList2, iptLstType);
		if (rspCode == FAIL)
			return FAIL;

		//Point to Next Tag
		ptrList1+=lenOfItem;
		lenOfData+=lenOfItem;
	} while (lenOfData < lenOfList1);

	return SUCCESS;
}


/*******************************************************************************
*	Removes and returns the first ListItem from List. Returns NULL if List
*	is empty.
*******************************************************************************/
UCHAR MPP_GetAndRemoveFromList(UCHAR *iptList, UCHAR *optLstItem, UCHAR iptLstType)
{
	UCHAR	lenOfT=0;	//Length of TLV-T
	UCHAR	lenOfL=0;	//Length of TLV-L
	UINT	lenOfV=0;	//Length of TLV-V
	UINT	lenOfList=0;//Lenght of List
	UINT	lenOfItem=0;//Length of Item
	UCHAR	rspCode=0;

	//Get List Length
	rspCode=UT_Get_TLVLengthOfV(iptList-3, &lenOfList);	//Decrease 3 to Point to Length
	if (rspCode == FAIL)
		return FAIL;

	if (lenOfList != 0)
	{
		if (iptLstType == MPP_LISTTYPE_TAG)
		{
			rspCode=UT_Get_TLVLengthOfT(iptList, &lenOfT);
			lenOfItem=lenOfT;
		}
		else if (iptLstType == MPP_LISTTYPE_TLV)
		{
			rspCode=UT_Get_TLVLength(iptList, &lenOfT, &lenOfL, &lenOfV);
			lenOfItem=lenOfT+lenOfL+lenOfV;
		}

		if (rspCode == SUCCESS)
			memcpy(optLstItem, iptList, lenOfItem);
		else
			return FAIL;

		MPP_RemoveFromList(optLstItem, iptList, iptLstType);
		
		return SUCCESS;
	}

	return FAIL;
}


/*******************************************************************************
*	Removes and returns the first tag from a list of tags that is
*	categorized as being available from the Card using a GET DATA command.
*	If no tag is found, NULL is returned.
*******************************************************************************/
UCHAR MPP_GetNextGetDataTagFromList(UCHAR *iptList, UCHAR *optLstItem, UCHAR iptLstType)
{
	UCHAR	lenOfT=0;			//Length of TLV-T
	UCHAR	lenOfL=0;			//Length of TLV-L
	UINT	lenOfV=0;			//Length of TLV-V
	UINT	lenOfList=0;		//Lenght of List
	UINT	lenOfItem=0;		//Length of Item
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=0;

	//Get List Length
	rspCode=UT_Get_TLVLengthOfV(iptList-3, &lenOfList);	//Decrease 3 to Point to Length
	if (rspCode == FAIL)
		return FAIL;

	if (lenOfList != 0)
	{
		ptrData=MPP_Search_GetDataTag(iptList, iptLstType);
		if (ptrData != NULLPTR)
		{
			if (iptLstType == MPP_LISTTYPE_TAG)
			{
				rspCode=UT_Get_TLVLengthOfT(ptrData, &lenOfT);
				lenOfItem=lenOfT;
			}
			else if (iptLstType == MPP_LISTTYPE_TLV)
			{
				rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
				lenOfItem=lenOfT+lenOfL+lenOfV;
			}

			if (rspCode == SUCCESS)
			{
				memcpy(optLstItem, ptrData, lenOfItem);
				UT_Set_TagLength(lenOfItem, optLstItem-3);		//william 2017/10/26
			}
			else
				return FAIL;

			MPP_RemoveFromList(ptrData, iptList, iptLstType);
			
			return SUCCESS;
		}
	}

	return FAIL;
}


/*******************************************************************************
*	Returns TRUE if List contains no ListItems.
*******************************************************************************/
UCHAR MPP_IsEmptyList(UCHAR *iptList)
{
	UINT	lenOfList=0;
	UCHAR	rspCode=FAIL;

	//Get List Length
	rspCode=UT_Get_TLVLengthOfV(iptList-3, &lenOfList);	//Decrease 3 to Point to Length
	if (rspCode == FAIL)
		return ERROR;

	if (lenOfList == 0)
		return TRUE;

	return FALSE;
}


/*******************************************************************************
*	Returns TRUE if List contains ListItems.
*******************************************************************************/
UCHAR MPP_IsNotEmptyList(UCHAR *iptList)
{
	UINT	lenOfList=0;
	UCHAR	rspCode=FAIL;

	//Get List Length
	rspCode=UT_Get_TLVLengthOfV(iptList-3, &lenOfList);	//Decrease 3 to Point to Length
	if (rspCode == FAIL)
		return ERROR;

	if (lenOfList != 0)
		return TRUE;

	return FALSE;
}


/*******************************************************************************
*	Returns TRUE if tag T is defined in the data dictionary of the Kernel as
*	defined in Annex A.
*******************************************************************************/
UCHAR MPP_IsKnown(UCHAR *iptTag)
{
	UCHAR	rspCode=FALSE;
	UCHAR	lenOfT=0;
	UINT	recIndex=0;

	//Get Tag Length
	rspCode=UT_Get_TLVLengthOfT(iptTag, &lenOfT);
	if (rspCode == FAIL)
		return ERROR;

	//Search EMVCL Tag Table
	rspCode=UT_Search_Record(lenOfT, iptTag, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
	if (rspCode == SUCCESS)
	{
		if (glv_tagTable[recIndex].MASTER_IsKnown == MASTER_KNOWN)
			return TRUE;
	}

	return FALSE;
}


/*******************************************************************************
*	Returns TRUE if the TLV Database includes a data object with tag T. Note
*	that the length of the data object may be zero. Note also that
*	proprietary data objects that are not known can be present if they have
*	been provided in the TLV Database at Kernel instantiation. In this case
*	the IsKnown() service returns FALSE and the IsPresent() service returns
*	TRUE.
*******************************************************************************/
UCHAR MPP_IsPresent(UCHAR *iptTag)
{
	UCHAR	*rspPTR=NULLPTR;
	UINT	rmdDatLength=0;

	//Search TLV DB List
	rspPTR=MPP_Search_ListItem(iptTag, mpp_lstPresent.Value, &rmdDatLength, MPP_LISTTYPE_TAG);
	if (rspPTR != NULLPTR)
		return TRUE;

	return FALSE;
}


/*******************************************************************************
*	Returns TRUE if the TLV Database does not include a data object with
*	tag T.
*******************************************************************************/
UCHAR MPP_IsNotPresent(UCHAR *iptTag)
{
	UCHAR	rspCode=FALSE;

	//Reverse from MPP_IsPresent
	rspCode=MPP_IsPresent(iptTag);
	if (rspCode == TRUE)
		return FALSE;

	return TRUE;
}


/*******************************************************************************
*	Returns TRUE if all the following are true: The TLV Database includes a
*	data object with tag T. The length of the data object is zero.
*******************************************************************************/
UCHAR MPP_IsEmpty(UCHAR *iptTag)
{
	UCHAR	rspCode=FALSE;
	UCHAR	tagLen=0;
	UINT	recIndex=0;
	UINT	rmdDatLength=0;
	UCHAR	*ptrTag;
	UCHAR	lenOfT=0;	//Length of TLV-T
	UCHAR	lenOfL=0;	//Length of TLV-L
	UINT	lenOfV=0;	//Length of TLV-V

	//The TLV Database includes a data object with tag T
	rspCode=MPP_IsPresent(iptTag);
	if (rspCode == FALSE)
		return ERROR;

	//Check the length of the data object is zero
	//Get Tag Length
	rspCode=UT_Get_TLVLengthOfT(iptTag, &tagLen);
	if (rspCode == FAIL)
		return ERROR;

	//Search EMVCL Tag Table
	rspCode=UT_Search_Record(tagLen, iptTag, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
	if (rspCode == SUCCESS)
	{
		UT_Get_TLVLengthOfV((UCHAR*)glv_addTable[recIndex], &lenOfV);
		if (lenOfV == 0)
			return TRUE;
		else
			return FALSE;
	}

	//If Tag Not Found, Search Temporary TLV DB
	ptrTag=MPP_Search_ListItem(iptTag, mpp_lstTlvDB.Value, &rmdDatLength, MPP_LISTTYPE_TLV);
	if (ptrTag != NULLPTR)
	{
		rspCode=UT_Get_TLVLength(ptrTag, &lenOfT, &lenOfL, &lenOfV);
		if (rspCode == FAIL)
			return ERROR;

		if (lenOfV == 0)
			return TRUE;
		else
			return FALSE;
	}
	
	return ERROR;
}


/*******************************************************************************
*	Returns TRUE if all of the following are true: The TLV Database includes
*	a data object with tag T. The length of the data object is different
*	from zero.
*******************************************************************************/
UCHAR MPP_IsNotEmpty(UCHAR *iptTag)
{
	UCHAR	rspCode=FALSE;

	//Reverse from MPP_IsEmpty
	rspCode=MPP_IsEmpty(iptTag);
	if (rspCode == TRUE)
		return FALSE;
	else if (rspCode == FALSE)
		return TRUE;

	return ERROR;
}


/*******************************************************************************
*	Returns the tag of the data object with name DataObjectName.
*******************************************************************************/
void MPP_TagOf(UCHAR *DataObjectName)
{
	/*
	*	Useless Function.
	*/
	DataObjectName[0]=DataObjectName[0];
}


/*******************************************************************************
*	Initializes the data object with tag T with a zero length. After
*	initialization the data object is present in the TLV Database.
*******************************************************************************/
void MPP_Initialize_Tag(UCHAR *iptTag)
{
	UCHAR	rspCode=FALSE;
	UCHAR	*rspPTR=NULLPTR;
	UCHAR	lenOfT=0;		//Length of TLV-T
	UCHAR	lenOfL=0;		//Length of TLV-L
	UINT	lenOfV=0;		//Length of TLV-V
	UINT	recIndex=0;
	UINT	rmdDatLength=0;
	//Tammy 2017/10/26
	//UCHAR	lenOfTable=0;
	UINT	lenOfTable = 0;
	UCHAR	tmpBuffer[32]={0};

	//Get Tag Length
	rspCode=UT_Get_TLVLengthOfT(iptTag, &lenOfT);
	if (rspCode != SUCCESS)
		return;

	//Search EMVCL Tag Table
	rspCode=UT_Search_Record(lenOfT, iptTag, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
	if (rspCode == SUCCESS)
	{
		//Initialize Tag
		//Tammy 2017/10/26
		//lenOfTable=glv_tagTable[recIndex].MASTER_Length;
		lenOfTable=glv_tagTable[recIndex].MASTER_MaxLength[0] * 256 + glv_tagTable[recIndex].MASTER_MaxLength[1];
		memset((UCHAR*)glv_addTable[recIndex], 0, (3 + lenOfTable));	//Reset L(3) + V(lenOfTable)
	}
	else
	{
		//Initialize Tag in TLV DB
		memcpy(tmpBuffer, iptTag, lenOfT);
		tmpBuffer[lenOfT]=0;	//Set Length = 0

		rspPTR=MPP_Search_ListItem(iptTag, mpp_lstTlvDB.Value, &rmdDatLength, MPP_LISTTYPE_TLV);
		if (rspPTR != NULLPTR)
		{
			MPP_Update_ListItem(tmpBuffer, mpp_lstTlvDB.Value);
		}
		else
		{
			//Add Tag in TLV DB
		    rspCode=UT_Get_TLVLength(tmpBuffer, &lenOfT, &lenOfL, &lenOfV);
			if (rspCode == SUCCESS)
			{
			    MPP_Add_ListItem(lenOfT+lenOfL+lenOfV, tmpBuffer, mpp_lstTlvDB.Value);
			}
		}
	}

	rspCode=MPP_IsPresent(iptTag);
	if (rspCode == FALSE)
	{
		rspCode=MPP_AddToList(iptTag, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}
}


/*******************************************************************************
*	Retrieves the TLV encoded data object with tag T from the TLV Database.
*	Returns NULL if the TLV Database does not include a data object with
*	tag T.
*******************************************************************************/
UCHAR MPP_GetTLV(UCHAR *iptTag, UCHAR *optDatObject)
{
	UCHAR	rspCode=FALSE;
	UCHAR	lenOfT=0;		//Length of TLV-T
	UCHAR	lenOfL=0;		//Length of TLV-L
	UINT	lenOfV=0;		//Length of TLV-V
	UINT	recIndex=0;
	UINT	rmdDatLength=0;
	UCHAR	*rspPTR=NULLPTR;

	rspCode=MPP_IsPresent(iptTag);
	if (rspCode == TRUE)
	{
		rspCode=UT_Get_TLVLengthOfT(iptTag, &lenOfT);
		if (rspCode == FAIL)
			return FAIL;

		//Search Tag Data
		rspCode=UT_Search_Record(lenOfT, iptTag, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
		if (rspCode == SUCCESS)	//Tag is in glv_tag
		{
			rspCode=UT_EMVCL_GetBERLEN((UCHAR*)glv_addTable[recIndex], &lenOfL, &lenOfV);
			
			//Patch TLV
			memcpy(optDatObject, glv_tagTable[recIndex].Tag, lenOfT);			
			memcpy(&optDatObject[lenOfT], glv_addTable[recIndex], lenOfL);			
			memcpy(&optDatObject[lenOfT+lenOfL], glv_addTable[recIndex]+3, lenOfV);

			return SUCCESS;
		}
		else	//Tag is in Temporary TLV DB
		{
			//Search TLV DB
			rspPTR=MPP_Search_ListItem(iptTag, mpp_lstTlvDB.Value, &rmdDatLength, MPP_LISTTYPE_TLV);
			if (rspPTR != NULLPTR)
			{
				rspCode=UT_Get_TLVLength(rspPTR, &lenOfT, &lenOfL, &lenOfV);

				//Patch TLV
				memcpy(optDatObject, rspPTR, lenOfT+lenOfL+lenOfV);

				return SUCCESS;
			}
		}
	}

	return FAIL;
}


/*******************************************************************************
*	Retrieves from the TLV Database the length of the data object with
*	tag T. Returns NULL if the TLV Database does not include a data object
*	with tag T.
*******************************************************************************/
UCHAR MPP_GetLength(UCHAR *iptTag, UINT *optLength)
{
	UCHAR	rspCode=FALSE;
	UCHAR	*rspPTR=NULLPTR;
	UCHAR	lenOfT=0;		//Length of TLV-T
	UCHAR	lenOfL=0;		//Length of TLV-L
	UINT	lenOfV=0;		//Length of TLV-V
	UINT	recIndex=0;
	UINT	rmdDatLength=0;

	rspCode=MPP_IsPresent(iptTag);
	if (rspCode == TRUE)
	{
		rspCode=UT_Get_TLVLengthOfT(iptTag, &lenOfT);

		rspCode=UT_Search_Record(lenOfT, iptTag, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
		if (rspCode == SUCCESS)
		{
			UT_Get_TLVLengthOfV((UCHAR*)glv_addTable[recIndex], &lenOfV);

			optLength[0]=lenOfV;

			return SUCCESS;
		}
		else
		{
			rspPTR=MPP_Search_ListItem(iptTag, mpp_lstTlvDB.Value, &rmdDatLength, MPP_LISTTYPE_TLV);
			if (rspPTR != NULLPTR)
			{
				rspCode=UT_Get_TLVLength(rspPTR, &lenOfT, &lenOfL, &lenOfV);

				optLength[0]=lenOfV;

				return SUCCESS;
			}
		}
	}

	return FAIL;
}


UCHAR MPP_ParseAndStoreCardResponse(UINT iptDatLen, UCHAR *iptData)
{
	UCHAR	flgConTag=FALSE;		//Flag of Constructed Tag
	UCHAR	flgEncError=FALSE;		//Flag of TLV Encoding Error
	UCHAR	flgIsKnown=FALSE;		//Flag of PayPass IsKnown
	UCHAR	flgIsPresent=FALSE;		//Flag of PayPass IsPresent
	UCHAR	flgIsNotPresent=FALSE;	//Flag of PayPass IsNotPresent
	UCHAR	flgIsEmpty=FALSE;		//Flag of PayPass IsEmpty
	UCHAR	flgPrvClass=FALSE;		//Flag of Tag is Private Class
	UCHAR	flgPrimitive=FALSE;		//Flag of Tag is Primitive Tag
	UCHAR	flgNotRA=FALSE;			//Flag of Update Conditions Not Include RA Signal
	UCHAR	flgInTagTable=FALSE;	//Flag of Tag is in Tag Table
	UCHAR	lenOfT=0;				//Length of TLV-T
	UCHAR	lenOfL=0;				//Length of TLV-L
	UINT	lenOfV=0;				//Length of TLV-V
	UINT	lenOfTLV=0;				//Length of TLV
	UINT	tagIndex=0;				//Index of Tag
	UINT	datLen=0;				//Length of Constructed Tag
	UINT	parLen=0;				//Length of Parse Data
	UINT	padLen=0;				//Length of Padding Word
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=0;

	//Tammy 2017/10/25
	UCHAR	*ptrData2 = NULLPTR;	//Point of template tag
	UCHAR	lenOfT2 = 0;			//Length of TLV-T
	UCHAR	lenOfL2 = 0;			//Length of TLV-L
	UINT	lenOfV2 = 0;			//Length of TLV-V
	UINT	searchLen = 0;			//Length of search data
	UINT	minLen = 0;				//Minimum length of TLV-V
	UINT	maxLen = 0;				//Maximum length of TLV-V
	UINT	templateLen = 0;		//Length of template tag TLV-V
	UINT	checkLen = 0;			//How long the length has been searched
	UCHAR	flgInLenRange = FALSE;	//Flag of TLV-L is within the range specified by Length field of the data object with tag T in the data dictionary in Annex A
	UCHAR	flgChkTemplate = FALSE;	//Flag of TLV is included in the correct template


	//Point to Start of Data
	ptrData=iptData;

	rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == FAIL)
		return ERROR;

	searchLen = (lenOfT + lenOfL + lenOfV);	//Tammy 2017/10/25

	//Check If the TLV String is a Single Consructed Tag
//	flgConTag=apk_EMVCL_CheckConsTag(ptrData[0]);
	flgConTag=UT_Check_ConstructedTag(ptrData);
	if (flgConTag == TRUE)
	{
		ptrData+=(lenOfT+lenOfL);	//Point to Constructed Tag Value
		datLen=lenOfV;

		//Check Receive Length = Tag Length + SW12
		if (iptDatLen != (lenOfT+lenOfL+lenOfV+2))
		{
			return FALSE;
		}

		//Check Length = 0
		if (lenOfV == 0)
		{
			return TRUE;
		}
	}
	else
	{
		datLen=lenOfT+lenOfL+lenOfV;
	}
	
	//Check BER Coding, Single Constructed Format or Primitive Tag
	//remarked by Tammy 2017/11/17
	//flgEncError=MPP_Check_EncodingError(datLen, ptrData);
	flgEncError = UT_Check_EMVTagEncodingError(datLen, ptrData);	//Tammy 2017/11/17
	if (flgEncError == TRUE)
	{
		return FALSE;
	}
	else
	{
		do {
			//Reset Flag
			flgConTag=FALSE;
			flgEncError=FALSE;
			flgIsKnown=FALSE;
			flgIsPresent=FALSE;
			flgIsNotPresent=FALSE;
			flgIsEmpty=FALSE;
			flgPrvClass=FALSE;
			flgPrimitive=FALSE;
			flgNotRA=FALSE;
			flgInTagTable=FALSE;

			//Tammy 2017/10/25
			flgInLenRange = FALSE;
			flgChkTemplate = FALSE;

			//Get TLV Length
			UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

			//Check Padding
			if ((ptrData[0] == 0x00) || (ptrData[0] == 0xFF))
			{
				UT_Check_Padding(datLen, parLen, ptrData, &padLen);

				ptrData+=padLen;
				parLen+=padLen;
				continue;
			}

			//Check Primitive Tag
			flgPrimitive=UT_Check_PrimitiveTag(ptrData);
			if (flgPrimitive == TRUE)
			{
				//Check IsKnown
				flgIsKnown=MPP_IsKnown(ptrData);

				//Check Private Class
				flgPrvClass=UT_Check_PrivateClassTag(ptrData);

				//Check Update Condition Not Include RA Signal
				flgInTagTable=UT_Search_Record(lenOfT, ptrData, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &tagIndex);

				if (flgIsKnown == TRUE)
				{
					if ((glv_tagTable[tagIndex].MASTER_UC & MASTER_UC_RA) == 0)
					{
						flgNotRA=TRUE;
					}
				}

				if (!((flgIsKnown == TRUE) && (flgPrvClass == TRUE) && (flgNotRA == TRUE)))
				{
					//Check IsPresent
					flgIsPresent=MPP_IsPresent(ptrData);
					flgIsNotPresent=!flgIsPresent;

					//Check IsEmpty
					flgIsEmpty=MPP_IsEmpty(ptrData);

					//Tammy 2017/10/25
					//Check L is within the range specified by Length field of the data object with tag T in the data dictionary in Annex A
					minLen = glv_tagTable[tagIndex].MASTER_MinLength[0] * 256 + glv_tagTable[tagIndex].MASTER_MinLength[1];
					maxLen = glv_tagTable[tagIndex].MASTER_MaxLength[0] * 256 + glv_tagTable[tagIndex].MASTER_MaxLength[1];

					if((lenOfV >= minLen) && (lenOfV <= maxLen))
					{
						//Check AFL is Multiply of 4
						if(!memcmp(ptrData, mpp_tof94, lenOfT))
						{
							if((lenOfV % 4) == 0)
							{
								flgInLenRange = TRUE;
							}
						}
						else
						{
							flgInLenRange = TRUE;
						}
					}

					//Tammy 2017/10/25
					//Check TLV is included in the correct template (if any) within TLV String
					if(glv_tagTable[tagIndex].MASTER_Template != MASTER_TMP_UNDEFINED)	//Find template
					{
						ptrData2 = UT_Find_Tag(mpp_tof70, searchLen, iptData);
						if((ptrData2 != NULLPTR) && ((glv_tagTable[tagIndex].MASTER_Template & MASTER_TMP_70) != 0))
						{
							;
						}
						else
						{
							ptrData2 = UT_Find_Tag(mpp_tof77, searchLen, iptData);
							if((ptrData2 != NULLPTR) && ((glv_tagTable[tagIndex].MASTER_Template & MASTER_TMP_77) != 0))
							{
								;
							}
							else
							{
								ptrData2 = UT_Find_Tag(mpp_tofA5, searchLen, iptData);
								if((ptrData2 != NULLPTR) && ((glv_tagTable[tagIndex].MASTER_Template & MASTER_TMP_A5) != 0))
								{
									;
								}
								else
								{
									ptrData2 = UT_Find_Tag(mpp_tofBF0C, searchLen, iptData);
									if((ptrData2 != NULLPTR) && ((glv_tagTable[tagIndex].MASTER_Template & MASTER_TMP_BF0C) != 0))
									{
										;
									}
									else
									{
										ptrData2 = UT_Find_Tag(mpp_tof6F, searchLen, iptData);
										if((ptrData2 != NULLPTR) && ((glv_tagTable[tagIndex].MASTER_Template & MASTER_TMP_6F) != 0))
										{
											;
										}
										else
										{
											flgChkTemplate = FALSE;
										}
									}
								}
							}
						}

						UT_Get_TLVLength(ptrData2, &lenOfT2, &lenOfL2, &lenOfV2);	//Get template tag length
						templateLen = lenOfV2;
						checkLen = 0;	//reset
						ptrData2 += (lenOfT2 + lenOfL2);	//Point to template tag TLV-V

						do
						{
							if(UT_Check_PrimitiveTag(ptrData2) == TRUE)	//TLV which in the template is primitive tag
							{
								UT_Get_TLVLength(ptrData2, &lenOfT2, &lenOfL2, &lenOfV2);	//Get TLV Length

								if(!memcmp(ptrData, ptrData2, lenOfT2))	//TLV is included in the correct template
								{
									flgChkTemplate = TRUE;
					
									break;
								}
								else
								{
									ptrData2 += (lenOfT2+lenOfL2+lenOfV2);	//Point to next tag
									checkLen += (lenOfT2+lenOfL2+lenOfV2);
								}
							}
							else  //Constructed tag
							{
								UT_Get_TLVLength(ptrData2, &lenOfT2, &lenOfL2, &lenOfV2);	//Get TLV Length

								if((ptrData - ptrData2) < (lenOfT2 + lenOfL2 + lenOfV2))	//TLV is in this constructed tag
								{
									flgChkTemplate = FALSE;
									break;
								}
								else  //Point to next tag
								{
									ptrData2 += (lenOfT2 + lenOfL2 + lenOfV2);
									checkLen += (lenOfT2 + lenOfL2 + lenOfV2);
								}
							}
						} while(checkLen < templateLen);
					}
					else
					{
						//Data objects for which no template is indicated ("�V") must not be returned in a template from the card.
						ptrData2 = iptData;	//Check from the beginning of card response

						do
						{
							if(UT_Check_PrimitiveTag(ptrData2) == TRUE)	//Primitive tag
							{
								UT_Get_TLVLength(ptrData2, &lenOfT2, &lenOfL2, &lenOfV2);	//Get TLV Length

								if(!memcmp(ptrData, ptrData2, lenOfT2))
								{
									flgChkTemplate = TRUE;
					
									break;
								}
								else
								{
									ptrData2 += (lenOfT2+lenOfL2+lenOfV2);	//Point to next tag
									checkLen += (lenOfT2+lenOfL2+lenOfV2);
								}
							}
							else  //Constructed tag
							{
								flgChkTemplate = FALSE;

								break;
							}
						} while(checkLen < iptDatLen);
					}

					if (flgIsKnown == TRUE)
					{
						//Tammy 2017/10/25 [MCL V3.1.1] Added check on length and template
						if (((flgIsNotPresent == TRUE) || (flgIsEmpty == TRUE)) && 
							(flgNotRA == FALSE) && 
							(flgInLenRange == TRUE) && 
							(flgChkTemplate == TRUE))
						{
							//Store to glv_tag
							MPP_Store_Tag(lenOfL, lenOfV, ptrData+lenOfT, tagIndex);

							if (flgIsNotPresent == TRUE)
							{
								MPP_AddToList(ptrData, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
							}
						}
						else
						{
							return FALSE;
						}
					}
					else
					{
						if (flgIsPresent == TRUE)
						{
							if ((flgIsEmpty == TRUE) && (flgNotRA == FALSE))
							{
								//Store LV in the TLV Database for tag T
								if (flgInTagTable == TRUE)
								{
									//Store to glv_tag
									MPP_Store_Tag(lenOfL, lenOfV, ptrData+lenOfT, tagIndex);
								}
								else
								{
									//Store to Temporary TLV DB
									MPP_AddToList(ptrData, mpp_lstTlvDB.Value, MPP_LISTTYPE_TLV);
								}
							}
							else
							{		
								return FALSE;
							}
						}
					}
				}

				//Count Tag Length
				lenOfTLV=lenOfT+lenOfL+lenOfV;
			}
			else
			{
				//Count Non Primitive Tag Length
				lenOfTLV=lenOfT+lenOfL;
			}

			//Update Parse Length
			parLen+=lenOfTLV;
			ptrData+=lenOfTLV;
		} while (parLen < datLen);

		return TRUE;
	}

	return FALSE;
}


/*******************************************************************************
*	Copies all incoming data (Terminal Sent Data) to the Kernel TLV Database
*	if update conditions allow.
*******************************************************************************/
void MPP_UpdateWithDetData(UCHAR *iptData)
{
	UCHAR	flgIsKnown=FALSE;		//Flag of PayPass IsKnown
	UCHAR	flgIsPresent=FALSE;		//Flag of PayPass IsPresent
	UCHAR	flgUC_DET=FALSE;		//Flag of PayPass Update Condition Include DET
	UCHAR	flgInTagTable=FALSE;	//Flag of Tag is in Tag Table
	UCHAR	flgEmpty = FALSE;
	UINT	lenOfData=0;			//Counter of Parse Data Length
	UCHAR	*ptrData;				//Pointer of Input Data
	UCHAR	lenOfT=0;				//Length of TLV-T
	UCHAR	lenOfL=0;				//Length of TLV-L
	UINT	lenOfV=0;				//Length of TLV-V
	UINT	lenOfDET=0;				//Length of DET Data
	UINT	recIndex=0;				//Record Index
	UCHAR	rspCode=FALSE;

	rspCode=UT_Get_TLVLength(iptData, &lenOfT, &lenOfL, &lenOfDET);
	if (rspCode == FAIL)
		return;

	//Set Data Pointer to TLV-V
	ptrData = iptData;

	do {
		//Reset Flag
		flgIsKnown=FALSE;
		flgIsPresent=FALSE;
		flgUC_DET=FALSE;
		flgInTagTable=FALSE;

		//Get Condition Flags
		flgIsKnown=MPP_IsKnown(ptrData);
		flgIsPresent=MPP_IsPresent(ptrData);
		flgEmpty = MPP_IsEmpty(ptrData);

		UT_Get_TLVLengthOfT(ptrData, &lenOfT);
		flgInTagTable=UT_Search_Record(lenOfT, ptrData, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
		if (glv_tagTable[recIndex].MASTER_UC & MASTER_UC_DET)
			flgUC_DET=TRUE;

		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		//Store LV in the TLV Database for tag T
		if (((flgIsKnown == TRUE) || (flgIsPresent == TRUE)) && (flgUC_DET == TRUE))
		{

			MPP_AddToList(ptrData, mpp_lstTlvDB.Value, MPP_LISTTYPE_TLV);
			MPP_AddToList(ptrData, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			if (flgInTagTable == TRUE)	//Store to glv_tag
			{				
				MPP_Store_Tag(lenOfL, lenOfV, ptrData + lenOfT, recIndex);   // 2017/10/31
			}
			//else		// william  2017/11/23  �ק令�@�w�|�[�JTLV_DB
			//{
			//	//Store to Temporary TLV DB
			//	MPP_AddToList(ptrData, mpp_lstTlvDB.Value, MPP_LISTTYPE_TLV);
			//}
		}
		else if (flgIsKnown == FALSE)	// william 2017/11/23  �w��proprietary tag�P�_��UC�O: RA or DET+ACT
		{
			if (flgEmpty == TRUE)					// means UC = RA, do nothing
				;
			else if (flgEmpty == FALSE)				// means UC = DET+ACT, update
				MPP_Update_ListItem(ptrData, mpp_lstTlvDB.Value);
		}

		if (lenOfT == 3)		// move out   2017/10/31
		{						// MPP_Store_Tag���ץѩ�ϥ�UCHAR �ҥH����s�W�L256����
			if ((ptrData[0] == 0xDF) && (ptrData[1] == 0x81) && (ptrData[2] == 0x12))
			{
				memcpy((UCHAR*)glv_addTable[recIndex], ptrData + lenOfT, lenOfL);	//Store Length				
				memcpy((UCHAR*)glv_addTable[recIndex] + 3, ptrData + lenOfT + lenOfL, lenOfV);	//Store Value
				MPP_AddListToList(glv_tagDF8112.Value, mpp_tagTagsToReadYet.Value, MPP_LISTTYPE_TAG);
			}
			else if ((ptrData[0] == 0xFF) && (ptrData[1] == 0x81) && (ptrData[2] == 0x02))
			{
				memcpy((UCHAR*)glv_addTable[recIndex]	 , ptrData + lenOfT			, lenOfL);	//Store Length				
				memcpy((UCHAR*)glv_addTable[recIndex] + 3, ptrData + lenOfT + lenOfL, lenOfV);	//Store Value
				MPP_AddListToList(glv_tagFF8102.Value, mpp_tagTagsToWriteYetBeforeGenAC.Value, MPP_LISTTYPE_TLV);
			}
			else if ((ptrData[0] == 0xFF) && (ptrData[1] == 0x81) && (ptrData[2] == 0x03))
			{
				memcpy((UCHAR*)glv_addTable[recIndex], ptrData + lenOfT, lenOfL);	//Store Length				
				memcpy((UCHAR*)glv_addTable[recIndex] + 3, ptrData + lenOfT + lenOfL, lenOfV);	//Store Value
				MPP_AddListToList(glv_tagFF8103.Value, mpp_tagTagsToWriteYetAfterGenAC.Value, MPP_LISTTYPE_TLV);
			}
		}
		//Skip to Next Tag
		ptrData+=(lenOfT+lenOfL+lenOfV);
		lenOfData+=(lenOfT+lenOfL+lenOfV);
	} while (lenOfData < mpp_lenDETRcvBuff);   //lenOfDET); 2017/10/31 parse without whole complete
}


void MPP_Create_List(UINT lstLength, UCHAR *lstData, UCHAR *iptList)
{
	UCHAR	*ptrData;
	UINT	parLength=0;
	UCHAR	rspCode=0;
	UCHAR	lenOfT=0;
	UCHAR	tmpTLV[1024]={0};

	//Initialize List
	memset(iptList-3, 0, 3+MPP_TLVLIST_BUFFERSIZE);	//Decrease 3 to Point to Length

	//Set Pointer to Start of Data
	ptrData=lstData;

	//For Ever Tag in List, If Tag Present in TLV DB, AddToList(GetTLV(Tag), List)
	do {
		rspCode=MPP_IsPresent(ptrData);
		if (rspCode == TRUE)
		{
			rspCode=MPP_GetTLV(ptrData, tmpTLV);
			if (rspCode == SUCCESS)
				MPP_AddToList(tmpTLV, iptList, MPP_LISTTYPE_TLV);
		}

		//Point to Next Tag
		UT_Get_TLVLengthOfT(ptrData, &lenOfT);
		parLength+=lenOfT;
		ptrData+=lenOfT;
	} while (parLength < lstLength);
}


void MPP_CreateEMVDataRecord(void)
{
	UINT	lstLength=50;
	UCHAR	lstDataRecord[50]={
			0x9F,0x02,	//Amount, Authorized (Numeric)
			0x9F,0x03,	//Amount, Other (Numeric)
			0x9F,0x26,	//Application Cryptogram
			0x5F,0x24,	//Application Expiration Date
			0x82,		//Application Interchange Profile
			0x50,		//Application Label
			0x5A,		//Application PAN
			0x5F,0x34,	//Application PAN Sequence Number
			0x9F,0x12,	//Application Preferred Name
			0x9F,0x36,	//Application Transaction Counter
			0x9F,0x07,	//Application Usage Control
			0x9F,0x09,	//Application Version Number (Reader)
			0x9F,0x27,	//Cryptogram Information Data
			0x9F,0x34,	//CVM Results
			0x84,		//DF Name
			0x9F,0x1E,	//Interface Device Serial Number
			0x9F,0x10,	//Issuer Application Data
			0x9F,0x11,	//Issuer Code Table Index
			0x9F,0x24,	//Payment Account Reference
			0x9F,0x33,	//Terminal Capabilities
			0x9F,0x1A,	//Terminal Country Code
			0x9F,0x35,	//Terminal Type
			0x95,		//Terminal Verification Results
			0x57,		//Track 2 Equivalent Data
			0x9F,0x53,	//Transaction Category Code
			0x5F,0x2A,	//Transaction Currency Code
			0x9A,		//Transaction Date
			0x9C,		//Transaction Type
			0x9F,0x37	//Unpredictable Number
	};

	MPP_Create_List(lstLength, lstDataRecord, glv_tagFF8105.Value);

	MPP_AddToList(mpp_tofFF8105, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
}


void MPP_CreateMSDataRecord(void)
{
	UINT	lstLength=15;
	UCHAR	lstDataRecord[15]={
			0x50,		//Application Label
			0x9F,0x12,	//Application Preferred Name
			0x84,		//DF Name
			0x9F,0x1E,	//Interface Device Serial Number
			0x9F,0x11,	//Issuer Code Table Index
			0x9F,0x6D,	//Mag-stripe Application Version Number (Reader)
			0x9F,0x24,	//Payment Account Reference
			0x56,		//Track 1 Data
			0x9F,0x6B,	//Track 2 Data
	};

	MPP_Create_List(lstLength, lstDataRecord, glv_tagFF8105.Value);

	MPP_AddToList(mpp_tofFF8105, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
}


void MPP_CreateEMVDiscretionaryData(void)
{
	UINT	lstLength=30;
	UCHAR	lstDiscretionaryData[30]={
		    0x9F,0x5D,		//Application Capabilities Information
			0x9F,0x42,		//Application Currency Code
			0xDF,0x81,0x05,	//Balance Read After Gen AC
			0xDF,0x81,0x04,	//Balance Read Before Gen AC
			0xDF,0x81,0x02,	//DS Summary 3
			0xDF,0x81,0x0B,	//DS Summary Status
			0xDF,0x81,0x15,	//Error Indication
			0xDF,0x81,0x0E,	//Post-Gen AC Put Data Status
			0xDF,0x81,0x0F,	//Pre-Gen AC Put Data Status
			0x9F,0x6E,		//Third Party Data
			0xFF,0x81,0x01	//Torn Record
	};

	MPP_Create_List(lstLength, lstDiscretionaryData, glv_tagFF8106.Value);

	MPP_AddToList(mpp_tofFF8106, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
}


void MPP_CreateMSDiscretionaryData(void)
{
	UINT	lstLength=13;
	UCHAR	lstDiscretionaryData[13]={
			0x9F,0x5D,		//Application Capabilities Information
			0xDF,0x81,0x2A,	//DD Card (Track1)
			0xDF,0x81,0x2B,	//DD Card (Track2)
			0xDF,0x81,0x15,	//Error Indication
			0x9F,0x6E		//Third Party Data
	};

	MPP_Create_List(lstLength, lstDiscretionaryData, glv_tagFF8106.Value);

	MPP_AddToList(mpp_tofFF8106, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
}


void MPP_Set_DefaultConfigurationData(void)
{
	UCHAR	rspCode=FALSE;
	UCHAR	dftPhoMsgTable[40]={
			//PCII Mask(3)	PCII Value(3)	MID(1)	Status(1)
			0x00,0x08,0x00,	0x00,0x08,0x00,	0x20, 	0x00,
			0x00,0x04,0x00,	0x00,0x04,0x00, 0x20, 	0x00,
			0x00,0x01,0x00,	0x00,0x01,0x00, 0x20, 	0x00,
			0x00,0x02,0x00,	0x00,0x02,0x00, 0x20, 	0x00,
			0x00,0x00,0x00,	0x00,0x00,0x00, 0x07, 	0x00};

	rspCode=MPP_IsNotPresent(mpp_tof9F40);
	if (rspCode == TRUE)
	{
		//Additional Terminal Capabilities '0000000000'
		glv_tag9F40.Length[0]=5;
		glv_tag9F40.Value[0]=0x00;
		glv_tag9F40.Value[1]=0x00;
		glv_tag9F40.Value[2]=0x00;
		glv_tag9F40.Value[3]=0x00;
		glv_tag9F40.Value[4]=0x00;

		MPP_AddToList(mpp_tof9F40, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tof9F09);
	if (rspCode == TRUE)
	{
		//Application Version Number (Reader) '0002'
		glv_tag9F09.Length[0]=2;
		glv_tag9F09.Value[0]=0x00;
		glv_tag9F09.Value[1]=0x02;

		MPP_AddToList(mpp_tof9F09, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF8117);
	if (rspCode == TRUE)
	{
		//Card Data Input Capability '00'
		glv_tagDF8117.Length[0]=1;
		glv_tagDF8117.Value[0]=0x00;

		MPP_AddToList(mpp_tofDF8117, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF8118);
	if (rspCode == TRUE)
	{
		//CVM Capability - CVM Required	'00'
		glv_tagDF8118.Length[0]=1;
		glv_tagDF8118.Value[0]=0x00;

		MPP_AddToList(mpp_tofDF8118, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF8119);
	if (rspCode == TRUE)
	{
		//CVM Capability - No CVM Required '00'
		glv_tagDF8119.Length[0]=1;
		glv_tagDF8119.Value[0]=0x00;

		MPP_AddToList(mpp_tofDF8119, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF811A);
	if (rspCode == TRUE)
	{
		//Default UDOL '9F6A04'
		glv_tagDF811A.Length[0]=3;
		glv_tagDF811A.Value[0]=0x9F;
		glv_tagDF811A.Value[1]=0x6A;
		glv_tagDF811A.Value[2]=0x04;

		MPP_AddToList(mpp_tofDF811A, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF8130);
	if (rspCode == TRUE)
	{
		//Hold Time Value '0D'
		glv_tagDF8130.Length[0]=1;
		glv_tagDF8130.Value[0]=0x0D;

		MPP_AddToList(mpp_tofDF8130, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF811B);
	if (rspCode == TRUE)
	{
		//Kernel Configuration '00'
		glv_tagDF811B.Length[0]=1;
		glv_tagDF811B.Value[0]=0x00;

		MPP_AddToList(mpp_tofDF811B, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF810C);
	if (rspCode == TRUE)
	{
		//Kernel ID '02'
		glv_tagDF810C.Length[0]=1;
		glv_tagDF810C.Value[0]=0x02;

		MPP_AddToList(mpp_tofDF810C, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tof9F6D);
	if (rspCode == TRUE)	
	{
		//Mag-stripe Application Version Number (Reader) '0001'
		glv_tag9F6D.Length[0]=2;
		glv_tag9F6D.Value[0]=0x00;
		glv_tag9F6D.Value[1]=0x01;

		MPP_AddToList(mpp_tof9F6D, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF811E);
	if (rspCode == TRUE)
	{
		//Mag-stripe CVM Capability - CVM Required 'F0'
		glv_tagDF811E.Length[0]=1;
		glv_tagDF811E.Value[0]=0xF0;

		MPP_AddToList(mpp_tofDF811E, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF812C);
	if (rspCode == TRUE)
	{
		//Mag-stripe CVM Capability - No CVM Required 'F0'
		glv_tagDF812C.Length[0]=1;
		glv_tagDF812C.Value[0]=0xF0;

		MPP_AddToList(mpp_tofDF812C, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF811C);
	if (rspCode == TRUE)
	{
		//Max Lifetime of Torn Transaction Log Record '012C'
		glv_tagDF811C.Length[0]=2;
		glv_tagDF811C.Value[0]=0x01;
		glv_tagDF811C.Value[1]=0x2C;

		MPP_AddToList(mpp_tofDF811C, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF811D);
	if (rspCode == TRUE)
	{
		//Max Number of Torn Transaction Log Records '00'
		glv_tagDF811D.Length[0]=1;
		glv_tagDF811D.Value[0]=0x00;

		MPP_AddToList(mpp_tofDF811D, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF812D);
	if (rspCode == TRUE)
	{
		//Message Hold Time '000013'
		glv_tagDF812D.Length[0]=3;
		glv_tagDF812D.Value[0]=0x00;
		glv_tagDF812D.Value[1]=0x00;
		glv_tagDF812D.Value[2]=0x13;

		MPP_AddToList(mpp_tofDF812D, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode = MPP_IsNotPresent(mpp_tofDF8133);
	if(rspCode == TRUE)
	{
		//Maximum Relay Resistance Grace Period '0032'
		glv_tagDF8133.Length[0] = 2;
		glv_tagDF8133.Value[0] = 0x00;
		glv_tagDF8133.Value[1] = 0x32;

		MPP_AddToList(mpp_tofDF8133, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode = MPP_IsNotPresent(mpp_tofDF8132);
	if(rspCode == TRUE)
	{
		//Minimum Relay Resistance Grace Period '0014'
		glv_tagDF8132.Length[0] = 2;
		glv_tagDF8132.Value[0] = 0x00;
		glv_tagDF8132.Value[1] = 0x14;

		MPP_AddToList(mpp_tofDF8132, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF8131);
	if (rspCode == TRUE)
	{
		//Default Phone Message Table
		glv_tagDF8131.Length[0]=40;
		memcpy(&glv_tagDF8131.Value[0], dftPhoMsgTable, 40);

		MPP_AddToList(mpp_tofDF8131, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF8123);
	if (rspCode == TRUE)
	{
		//Reader Contactless Floor Limit '000000000000'
		glv_tagDF8123.Length[0]=6;
		glv_tagDF8123.Value[0]=0x00;
		glv_tagDF8123.Value[1]=0x00;
		glv_tagDF8123.Value[2]=0x00;
		glv_tagDF8123.Value[3]=0x00;
		glv_tagDF8123.Value[4]=0x00;
		glv_tagDF8123.Value[5]=0x00;

		MPP_AddToList(mpp_tofDF8123, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF8124);
	if (rspCode == TRUE)
	{
		//Reader Contactless Transaction Limit (No On-device CVM) '000000000000'
		glv_tagDF8124.Length[0]=6;
		glv_tagDF8124.Value[0]=0x00;
		glv_tagDF8124.Value[1]=0x00;
		glv_tagDF8124.Value[2]=0x00;
		glv_tagDF8124.Value[3]=0x00;
		glv_tagDF8124.Value[4]=0x00;
		glv_tagDF8124.Value[5]=0x00;

		MPP_AddToList(mpp_tofDF8124, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF8125);
	if (rspCode == TRUE)
	{
		//Reader Contactless Transaction Limit (On-device CVM) '000000000000'
		glv_tagDF8125.Length[0]=6;
		glv_tagDF8125.Value[0]=0x00;
		glv_tagDF8125.Value[1]=0x00;
		glv_tagDF8125.Value[2]=0x00;
		glv_tagDF8125.Value[3]=0x00;
		glv_tagDF8125.Value[4]=0x00;
		glv_tagDF8125.Value[5]=0x00;

		MPP_AddToList(mpp_tofDF8125, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF8126);
	if (rspCode == TRUE)
	{
		//Reader CVM Required Limit '000000000000'
		glv_tagDF8126.Length[0]=6;
		glv_tagDF8126.Value[0]=0x00;
		glv_tagDF8126.Value[1]=0x00;
		glv_tagDF8126.Value[2]=0x00;
		glv_tagDF8126.Value[3]=0x00;
		glv_tagDF8126.Value[4]=0x00;
		glv_tagDF8126.Value[5]=0x00;

		MPP_AddToList(mpp_tofDF8126, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode = MPP_IsNotPresent(mpp_tofDF8136);
	if(rspCode == TRUE)
	{
		//Relay Resistance Accuracy Threshold '012C'
		glv_tagDF8136.Length[0] = 2;
		glv_tagDF8136.Value[0] = 0x01;
		glv_tagDF8136.Value[1] = 0x2C;

		MPP_AddToList(mpp_tofDF8136, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode = MPP_IsNotPresent(mpp_tofDF8137);
	if(rspCode == TRUE)
	{
		//Relay Resistance Transmission Time Mismatch Threshold '32'
		glv_tagDF8137.Length[0] = 1;
		glv_tagDF8137.Value[0] = 0x32;

		MPP_AddToList(mpp_tofDF8137, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF811F);
	if (rspCode == TRUE)
	{
		//Security Capability '00'
		glv_tagDF811F.Length[0]=1;
		glv_tagDF811F.Value[0]=0x00;

		MPP_AddToList(mpp_tofDF811F, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF8120);
	if (rspCode == TRUE)
	{
		//Terminal Action Code - Default '840000000C'
		glv_tagDF8120.Length[0]=5;
		glv_tagDF8120.Value[0]=0x84;
		glv_tagDF8120.Value[1]=0x00;
		glv_tagDF8120.Value[2]=0x00;
		glv_tagDF8120.Value[3]=0x00;
		glv_tagDF8120.Value[4]=0x0C;

		MPP_AddToList(mpp_tofDF8120, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF8121);
	if (rspCode == TRUE)
	{
		//Terminal Action Code - Denial '840000000C'
		glv_tagDF8121.Length[0]=5;
		glv_tagDF8121.Value[0]=0x84;
		glv_tagDF8121.Value[1]=0x00;
		glv_tagDF8121.Value[2]=0x00;
		glv_tagDF8121.Value[3]=0x00;
		glv_tagDF8121.Value[4]=0x0C;

		MPP_AddToList(mpp_tofDF8121, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF8122);
	if (rspCode == TRUE)
	{
		//Terminal Action Code - Online '840000000C'
		glv_tagDF8122.Length[0]=5;
		glv_tagDF8122.Value[0]=0x84;
		glv_tagDF8122.Value[1]=0x00;
		glv_tagDF8122.Value[2]=0x00;
		glv_tagDF8122.Value[3]=0x00;
		glv_tagDF8122.Value[4]=0x0C;

		MPP_AddToList(mpp_tofDF8122, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tof9F1A);
	if (rspCode == TRUE)
	{
		//Terminal Country Code '0000'
		glv_tag9F1A.Length[0]=2;
		glv_tag9F1A.Value[0]=0x00;
		glv_tag9F1A.Value[1]=0x00;

		MPP_AddToList(mpp_tof9F1A, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode = MPP_IsNotPresent(mpp_tofDF8134);
	if(rspCode == TRUE)
	{
		//Terminal Expected Transmission Time For Relay Resistance C-APDU '0012'
		glv_tagDF8134.Length[0] = 2;
		glv_tagDF8134.Value[0] = 0x00;
		glv_tagDF8134.Value[1] = 0x12;

		MPP_AddToList(mpp_tofDF8134, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode = MPP_IsNotPresent(mpp_tofDF8135);
	if(rspCode == TRUE)
	{
		//Terminal Expected Transmission Time For Relay Resistance R-APDU '0018'
		glv_tagDF8135.Length[0] = 2;
		glv_tagDF8135.Value[0] = 0x00;
		glv_tagDF8135.Value[1] = 0x18;

		MPP_AddToList(mpp_tofDF8135, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tof9F35);
	if (rspCode == TRUE)
	{
		//Terminal Type '00'
		glv_tag9F35.Length[0]=1;
		glv_tag9F35.Value[0]=0x00;

		MPP_AddToList(mpp_tof9F35, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tofDF8127);
	if (rspCode == TRUE)
	{
		//Time Out Value '01F4'
		glv_tagDF8127.Length[0]=2;
		glv_tagDF8127.Value[0]=0x01;
		glv_tagDF8127.Value[1]=0xF4;

		MPP_AddToList(mpp_tofDF8127, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	rspCode=MPP_IsNotPresent(mpp_tof9C);
	if (rspCode == TRUE)
	{
		//Transaction Type '00'
		glv_tag9C.Length[0]=1;
		glv_tag9C.Value[0]=0x00;

		MPP_AddToList(mpp_tof9C, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}	
}


void MPP_OUT(UCHAR *iptTag)
{
	UCHAR	tlvBuffer[3+3+1024]={0};	//Max. Tag + Length + Value
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	rspCode=FALSE;

	if (mpp_flgTA == TRUE)
	{
		//Success Tone for Performance
		if ((!memcmp(iptTag, mpp_tofDF8129, 3)) &&
			((glv_tagDF8129.Value[0] == MPP_OPS_STATUS_Approved) ||
			(glv_tagDF8129.Value[0] == MPP_OPS_STATUS_OnlineRequest)))
		{
			UT_BUZ_Success();
		}
		
		rspCode=MPP_GetTLV(iptTag, tlvBuffer);
		if (rspCode == SUCCESS)
		{
			rspCode=UT_Get_TLVLength(tlvBuffer, &lenOfT, &lenOfL, &lenOfV);
			if (rspCode == SUCCESS)
			{
				TEI_Upload_Data(lenOfT+lenOfL+lenOfV, tlvBuffer);
			}
		}
	}
}


UCHAR MPP_Convert_MPP2EMV_Message(UCHAR iptMsgID)
{
	switch (iptMsgID)
	{
		case MPP_UIR_MID_CardReadOK:				return 0x17;
		case MPP_UIR_MID_TryAgain:					return 0x21;
		case MPP_UIR_MID_Approved:					return 0x03;
		case MPP_UIR_MID_Approved_Sign:				return 0x1A;
		case MPP_UIR_MID_Declined:					return 0x07;
		case MPP_UIR_MID_Error_OtherCard:			return 0x0F;
		case MPP_UIR_MID_InsertCard:				return 0x18;
		case MPP_UIR_MID_SeePhone:					return 0x20;
		case MPP_UIR_MID_Authorising_PleaseWait:	return 0x1B;
		case MPP_UIR_MID_ClearDisplay:				return 0xFF;
		case MPP_UIR_MID_NA:						return 0xFF;
		default: break;
	}

	return 0xFF;
}


UCHAR MPP_Convert_MPP2EMV_Status(UCHAR iptStatus)
{
	switch (iptStatus)
	{
		case MPP_UIR_STATUS_NotReady:				return 0x03;
		case MPP_UIR_STATUS_Idle:					return 0x05;
		case MPP_UIR_STATUS_ReadyToRead:			return 0x02;
		case MPP_UIR_STATUS_Processing:				return 0x04;
		case MPP_UIR_STATUS_CardReadSuccessfully:	return 0x00;
		case MPP_UIR_STATUS_ProcessingError:		return 0x01;
		case MPP_UIR_STATUS_NA:						return 0x05;
		default: break;
	}

	return 0xFF;
}


void MPP_MSG_Display_Message(void)
{
	UCHAR	emvMsgId=0;
	UCHAR	emvStatus=0;

	emvMsgId=MPP_Convert_MPP2EMV_Message(glv_tagDF8116.Value[0]);
	emvStatus=MPP_Convert_MPP2EMV_Status(glv_tagDF8116.Value[1]);

	ETP_UI_Request(emvMsgId, emvStatus);	
}


void MPP_MSG(UCHAR *iptTag)
{
	if (mpp_flgTA == TRUE)
	{
		MPP_OUT(iptTag);
	}
	else
	{
		MPP_MSG_Display_Message();
	}
}

//	Send DEK(Data To Send, Data Needed) signal & Receieve DET signal (inner search, not actual)
UCHAR MPP_DEK(UINT option)
{
	UCHAR	DEKBuff[1024] = { 0 };
	UCHAR	tagFF8104[3] = { 0xFF,0x81,0x04 };
	UCHAR	tagDF8106[3] = { 0xDF,0x81,0x06 };
	UCHAR	hexDEK[3] = { 0x44,0x45,0x4B };
	UCHAR	hexDET[3] = { 0x44,0x45,0x54 };
	UINT	BuffCnt = 0, SearchCnt = 0, lenV, lenL, DETBufCnt = 0, DETCnt = 0;
	UINT	LeftSetDEKT = 0, RightSetDEKT = 0;
	UINT	LeftEachDET = 0, RightEachDET = 0;
	UCHAR	rspCode = FALSE;
	UCHAR	flagSetEnd = FALSE;

	// 1.Build DEK Signal from DataToSend & DataNeeded
	memcpy(&DEKBuff[BuffCnt], tagFF8104, 3);
	BuffCnt += 3;

	UCHAR	lenOfL = 0;
	UT_EMVCL_GetBERLEN(glv_tagFF8104.Length, &lenOfL, &lenV); 
	if (lenV != 0)
	{
		memcpy(&DEKBuff[BuffCnt], glv_tagFF8104.Length, lenOfL);
		BuffCnt += lenOfL;
		memcpy(&DEKBuff[BuffCnt], glv_tagFF8104.Value, lenV);
		BuffCnt += lenV;
	}
	else	//empty
		DEKBuff[BuffCnt++] = 0x00;

	if (option == 2)
	{
		memcpy(&DEKBuff[BuffCnt], tagDF8106, 3);
		BuffCnt += 3;
		lenL = UT_Get_TLVLengthOfV(glv_tagDF8106.Length, &lenV);
		if (lenV != 0)
		{
			memcpy(&DEKBuff[BuffCnt], glv_tagDF8106.Length, lenL);
			BuffCnt += lenL;
			memcpy(&DEKBuff[BuffCnt], glv_tagDF8106.Value, lenV);
			BuffCnt += lenV;
		}
		else	//empty
			DEKBuff[BuffCnt++] = 0x00;
	}
	// Record in log: 
	MPP_DataExchangeLog((UCHAR*)&"DEK:", BuffCnt);
	MPP_DataExchangeLog(DEKBuff, BuffCnt);
	
	// **1.5  if there is tag 9F37(UN), change all its value into 0xFF to match .xml file
	int i;
	for (i = 0; i < BuffCnt; i++)
		if ( DEKBuff[i] == 0x9F && DEKBuff[i + 1] == 0x37 && DEKBuff[i + 2] == 0x04 )
		{
			DEKBuff[i + 3] = 0xFF; DEKBuff[i + 4] = 0xFF; DEKBuff[i + 5] = 0xFF; DEKBuff[i + 6] = 0xFF;
			break;
		}

	// 2. Search.xml for same DEK in whole DE_xmlBuf (0 ~ mpp_lenDE_xmlBuf)
	do 
	{
		if (!memcmp(&mpp_ptrDE_xmlBuf[SearchCnt], hexDEK, 3))
		{
			if (LeftSetDEKT == 0)
			{
				SearchCnt += 3;
				if (!memcmp(&mpp_ptrDE_xmlBuf[SearchCnt], DEKBuff, BuffCnt))
				{
					LeftSetDEKT = SearchCnt;
					rspCode = TRUE;		// our DEK exist 
				}
			}
			else			//LeftSetDEKT != 0
			{				
				break;
			}
		}
		SearchCnt++;
	} while (SearchCnt < mpp_lenDE_xmlBuf);
	RightSetDEKT = SearchCnt;

	if (rspCode != TRUE)	// cant find the same DEK in .xml
	{
		MPP_DataExchangeLog((UCHAR*)&"UnexpDEK", BuffCnt);
		return FALSE;
	}
	
	// 3. Search corresponding DET signal in DE_xmlBuf[our set] (numDET >= 0)
	SearchCnt = LeftSetDEKT;
	mpp_lenDETRcvBuff = 0;
	while(1)
	{
		if (!memcmp(&mpp_ptrDE_xmlBuf[SearchCnt], hexDEK, 3) || SearchCnt == RightSetDEKT)
		{
			flagSetEnd = TRUE;
		}

		if (!memcmp(&mpp_ptrDE_xmlBuf[SearchCnt], hexDET, 3) || flagSetEnd)
		{			
			if (LeftEachDET == 0 )		// numDET == 0 so far
			{
				if (flagSetEnd)
					break;

				SearchCnt += 3;
				LeftEachDET = SearchCnt;			
			}
			else if(LeftEachDET != 0 || flagSetEnd)
			{
				RightEachDET = SearchCnt;
				DETCnt++;

				memcpy(&mpp_DETRcvBuff[DETBufCnt], &mpp_ptrDE_xmlBuf[LeftEachDET], RightEachDET - LeftEachDET);
				mpp_lenDETRcvBuff += RightEachDET - LeftEachDET;
				DETBufCnt = DETBufCnt + (RightEachDET - LeftEachDET);
				
				// Record in log: 
				MPP_DataExchangeLog((UCHAR*)&"DET:", BuffCnt);
				MPP_DataExchangeLog(&mpp_ptrDE_xmlBuf[LeftEachDET], RightEachDET - LeftEachDET);

				if (flagSetEnd)
					break;

				LeftEachDET = SearchCnt + 3;
			}
		}
		SearchCnt++;
	} 

	// 4. Already get DET, set Signal, Keep DET buffer until allow to update in TLV-DB (ex: S5_2 )
	if (DETCnt != 0)
		MPP_Insert_Signal(MPP_SIGNAL_DET); // flagDE = 1; //MPP_Add_Signal(MPP_SIGNAL_DET);

	return TRUE;
}
