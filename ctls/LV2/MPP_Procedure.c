#include <string.h>
#include "POSAPI.h"
#include "Define.h"
#include "ECL_Tag.h"
#include "Function.h"
#include "MPP_Define.h"
#include "MPP_Function.h"
#include "MPP_Tag.h"
#include "MPP_TagOf.h"


//		MPP DEP Buffer
extern	UINT	mpp_depRcvLen;
//extern	UCHAR	mpp_depRcvData[ETP_BUFFER_SIZE];
extern	UCHAR	*mpp_depRcvData;
extern	UCHAR	mpp_depRspCode;

//		MPP Present List
extern	ECL_TAG	mpp_lstPresent;

//		GAC buffer: CDOL + DSDOL
extern	ECL_TAG mpp_tempSend;

//		MPP CVM Parameter
UCHAR	mpp_cvmCVR[2]={0};	//Cardholder Verification Rule
UCHAR	mpp_cvmCode=0;		//CVM Code(First byte of a CVR)
UCHAR	mpp_cvmCndCode=0;	//CVM Condition Code(Second byte of a CVR)
UCHAR	mpp_cvmLstIndex=0;	//CVM List Index

	
UCHAR MPP_BR1_1(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_IsNotEmpty(mpp_tof9F5D);
	if ((rspCode == TRUE) && (glv_tag9F5D.Value[1] & 0x02))
	{
		rspCode=TRUE;
	}
	else
	{
		rspCode=FALSE;
	}

	MPP_DBG_Put_Process(MPP_PROCEDURE_PreGACBalanceReading, 1, rspCode);
	
	return rspCode;
}

UCHAR MPP_BR1_2(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsPresent(mpp_tofDF8104);

	MPP_DBG_Put_Process(MPP_PROCEDURE_PreGACBalanceReading, 2, rspCode);
	
	return rspCode;
}

void MPP_BR1_3(void)
{
	//Prepare GET DATA command for '9F50'

	MPP_DBG_Put_Process(MPP_PROCEDURE_PreGACBalanceReading, 3, 0xFF);
}

void MPP_BR1_4(void)
{
	MPP_Clear_DEPBuffer();
	
	mpp_depRspCode=MPP_APDU_GET_DATA(0x9F, 0x50, &mpp_depRcvLen, mpp_depRcvData);

	MPP_DBG_Put_Process(MPP_PROCEDURE_PreGACBalanceReading, 4, 0xFF);
}

UCHAR MPP_BR2_1(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_IsNotEmpty(mpp_tof9F5D);
	if ((rspCode == TRUE) && (glv_tag9F5D.Value[1] & 0x02))
	{
		rspCode=TRUE;
	}
	else
	{
		rspCode=FALSE;
	}

	MPP_DBG_Put_Process(MPP_PROCEDURE_PostGACBalanceReading, 1, rspCode);
		
	return rspCode;
}

UCHAR MPP_BR2_2(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_IsPresent(mpp_tofDF8105);

	MPP_DBG_Put_Process(MPP_PROCEDURE_PostGACBalanceReading, 2, rspCode);
	
	return rspCode;
}

void MPP_BR2_3(void)
{
	//Prepare GET DATA command for '9F50'

	MPP_DBG_Put_Process(MPP_PROCEDURE_PostGACBalanceReading, 3, 0xFF);
}

void MPP_BR2_4(void)
{
	MPP_Clear_DEPBuffer();
	
	mpp_depRspCode=MPP_APDU_GET_DATA(0x9F, 0x50, &mpp_depRcvLen, mpp_depRcvData);

	MPP_DBG_Put_Process(MPP_PROCEDURE_PostGACBalanceReading, 4, 0xFF);
}

UCHAR MPP_CVM_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((glv_tag82.Value[0] & MPP_AIP_OnDeviceCardholderVerificationIsSupported) &&
		(glv_tagDF811B.Value[0] & MPP_KCF_OnDeviceCardholderVerificationSupported))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 1, rspCode);

	return rspCode;
}

UCHAR MPP_CVM_2(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=UT_bcdcmp(glv_tag9F02.Value, glv_tagDF8126.Value, 6);
	if (rspCode == 1)	//Amount, Authorized (Numeric) > Reader CVM Required Limit
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 2, rspCode);

	return rspCode;
}

void MPP_CVM_3(void)
{
	glv_tagDF8129.Value[3]=MPP_OPS_CVM_NoCVM;	//'CVM' in Outcome Parameter Set := NO CVM

	glv_tag9F34.Value[0]=0x3F;	//'CVM Performed' in CVM Results := '3F' (No CVM performed)
	glv_tag9F34.Value[1]=0x00;	//'CVM Condition' in CVM Results := '00'
	glv_tag9F34.Value[2]=0x02;	//'CVM Result' in CVM Results := '02' (successful)

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 3, 0xFF);
}

void MPP_CVM_4(void)
{
	glv_tagDF8129.Value[3]=MPP_OPS_CVM_ConfirmationCodeVerified;	//'CVM' in Outcome Parameter Set := CONFIRMATION CODE VERIFIED

	glv_tag9F34.Value[0]=0x01;	//'CVM Performed' in CVM Results := '01' (Plaintext PIN verification performed by ICC)
	glv_tag9F34.Value[1]=0x00;	//'CVM Condition' in CVM Results := '00'
	glv_tag9F34.Value[2]=0x02;	//'CVM Result' in CVM Results := '02' (successful)

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 4, 0xFF);
}

UCHAR MPP_CVM_5(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tag82.Value[0] & MPP_AIP_CardholderVerificationIsSupported)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 5, rspCode);
	
	return rspCode;
}

void MPP_CVM_6(void)
{
	glv_tagDF8129.Value[3]=MPP_OPS_CVM_NoCVM;	//'CVM' in Outcome Parameter Set := NO CVM

	glv_tag9F34.Value[0]=0x3F;	//'CVM Performed' in CVM Results := '3F' (No CVM performed)
	glv_tag9F34.Value[1]=0x00;	//'CVM Condition' in CVM Results := '00'
	glv_tag9F34.Value[2]=0x00;	//'CVM Result' in CVM Results := '00' (unknown)

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 6, 0xFF);
}

UCHAR MPP_CVM_7(void)
{
	UCHAR	flgIsNotPresent = FALSE;
	UCHAR	flgIsEmpty = FALSE;
	UCHAR	rspCode=FALSE;
	
	flgIsNotPresent = MPP_IsNotPresent(mpp_tof8E);
	flgIsEmpty = MPP_IsEmpty(mpp_tof8E);

	if((flgIsNotPresent == TRUE) || (flgIsEmpty == TRUE))
		rspCode = TRUE;
	else
		rspCode = FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 7, rspCode);

	return rspCode;
}

void MPP_CVM_8(void)
{
	glv_tagDF8129.Value[3]=MPP_OPS_CVM_NoCVM;	//'CVM' in Outcome Parameter Set := NO CVM

	glv_tag9F34.Value[0]=0x3F;	//'CVM Performed' in CVM Results := '3F' (No CVM performed)
	glv_tag9F34.Value[1]=0x00;	//'CVM Condition' in CVM Results := '00'
	glv_tag9F34.Value[2]=0x00;	//'CVM Result' in CVM Results := '00' (unknown)

	glv_tag95.Value[0]|=MPP_TVR_ICCDataMissing;	//SET 'ICC data missing' in Terminal Verification Results

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 8, 0xFF);
}

void MPP_CVM_9(void)
{
	memcpy(mpp_cvmCVR, &glv_tag8E.Value[8], 2);		//CVM List = Amount(4) + Second Amount(4) + CVR(2)...	
	mpp_cvmCode=mpp_cvmCVR[0];
	mpp_cvmCndCode=mpp_cvmCVR[1];

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 9, 0xFF);
}

UCHAR MPP_CVM_10(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_Check_CVMCondition_IsUnderstood(mpp_cvmCndCode);
	
	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 10, rspCode);
	
	return rspCode;
}

UCHAR MPP_CVM_11(void)
{
	UCHAR	rspCode=FALSE;

	if ((mpp_cvmCndCode == MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsUnderXValue) ||
		(mpp_cvmCndCode == MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsOverXValue) ||
		(mpp_cvmCndCode == MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsUnderYValue) ||
		(mpp_cvmCndCode == MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsOverYValue))
	{
		rspCode=MPP_IsPresent(mpp_tof9F42);
		if (rspCode == TRUE)
			rspCode=TRUE;
		else
			rspCode=FALSE;
	}
	else
	{
		rspCode=TRUE;
	}
		
	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 11, rspCode);
	
	return rspCode;
}

UCHAR MPP_CVM_12(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_Check_CVMCondition_IsSatisfied(mpp_cvmCode, mpp_cvmCndCode);

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 12, rspCode);
	
	return rspCode;
}

UCHAR MPP_CVM_13(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_Check_CVMList_IsLastCVR(mpp_cvmLstIndex);

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 13, rspCode);

	return rspCode;
}

void MPP_CVM_14(void)
{
	glv_tagDF8129.Value[3]=MPP_OPS_CVM_NoCVM;	//'CVM' in Outcome Parameter Set := NO CVM

	glv_tag9F34.Value[0]=0x3F;	//'CVM Performed' in CVM Results := '3F' (No CVM performed)
	glv_tag9F34.Value[1]=0x00;	//'CVM Condition' in CVM Results := '00'
	glv_tag9F34.Value[2]=0x01;	//'CVM Result' in CVM Results := '01' (failed)

	glv_tag95.Value[2]|=MPP_TVR_CardholderVerificationWasNotSuccessful;	//SET 'Cardholder verification was not successful' in Terminal Verification Results

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 14, 0xFF);
}

UCHAR MPP_CVM_15(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_Check_CVMMethod_IsRecognized(mpp_cvmCode);

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 15, rspCode);

	return rspCode;
}

void MPP_CVM_16(void)
{
	glv_tag95.Value[2]|=MPP_TVR_UnrecognisedCVM;	//SET 'Unrecognised CVM' in Terminal Verification Results

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 16, 0xFF);
}

UCHAR MPP_CVM_17(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_Check_CVMMethod_IsSupported(mpp_cvmCode);
	if ((rspCode == TRUE) && ((mpp_cvmCode & 0x3F) != 0))
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 17, rspCode);

	return rspCode;
}

void MPP_CVM_18(void)
{
	if ((mpp_cvmCode & 0x3F) == 0x02)
	{
		glv_tagDF8129.Value[3]=MPP_OPS_CVM_OnlinePin;	//'CVM' in Outcome Parameter Set := ONLINE PIN

		glv_tag9F34.Value[2]=0x00;						//'CVM Result' in CVM Results := '00' (unknown)

		glv_tag95.Value[2]|=MPP_TVR_OnlinePINEntered;	//SET 'Online PIN entered' in Terminal Verification Results
	}
	else
	{
		if ((mpp_cvmCode & 0x3F) == 0x1E)
		{
			glv_tagDF8129.Value[3]=MPP_OPS_CVM_ObtainSignature;	//'CVM' in Outcome Parameter Set := OBTAIN SIGNATURE

			glv_tag9F34.Value[2]=0x00;							//'CVM Result' in CVM Results := '00' (unknown)

			glv_tagDF8129.Value[4]|=MPP_OPS_Receipt_Yes;		//'Receipt' in Outcome Parameter Set := YES
		}
		else
		{
			if ((mpp_cvmCode & 0x3F) == 0x1F)
			{
				glv_tagDF8129.Value[3]=MPP_OPS_CVM_NoCVM;	//'CVM' in Outcome Parameter Set := NO CVM

				glv_tag9F34.Value[2]=0x02;					//'CVM Result' in CVM Results := '02' (successful)
			}
			else
			{
				glv_tagDF8129.Value[3]=0x4F;	//Set 'CVM' in Outcome Parameter Set to proprietary value

				glv_tag9F34.Value[2]=0x00;		//'CVM Result' in CVM Results := '00' or '02'
			}
		}
	}

	glv_tag9F34.Value[0]=mpp_cvmCode;		//'CVM Performed' in CVM Results := CVM Code
	glv_tag9F34.Value[1]=mpp_cvmCndCode;	//'CVM Condition' in CVM Results := CVM Condition Code

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 18, 0xFF);
}

UCHAR MPP_CVM_19(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_cvmCode & MPP_CVM_METHOD_ApplySucceedingCVRuleIfThisCVMIsUnsuccessful)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 19, rspCode);

	return rspCode;
}

UCHAR MPP_CVM_20(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_Check_CVMList_IsLastCVR(mpp_cvmLstIndex);
	
	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 20, rspCode);
	
	return rspCode;
}

void MPP_CVM_21(void)
{
	//Get Next CVR
	mpp_cvmLstIndex++;
	
	MPP_Get_CVMList_CVR(mpp_cvmLstIndex, mpp_cvmCVR);
	mpp_cvmCode=mpp_cvmCVR[0];
	mpp_cvmCndCode=mpp_cvmCVR[1];

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 21, 0xFF);
}

void MPP_CVM_22(void)
{
	glv_tagDF8129.Value[3]=MPP_OPS_CVM_NoCVM;							//'CVM' in Outcome Parameter Set := NO CVM

	glv_tag95.Value[2]|=MPP_TVR_CardholderVerificationWasNotSuccessful;	//SET 'Cardholder verification was not successful' in Terminal Verification Results

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 22, 0xFF);
}

UCHAR MPP_CVM_23(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((mpp_cvmCode & 0x3F) == 0x00)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 23, rspCode);

	return rspCode;		
}

void MPP_CVM_24(void)
{
	glv_tag9F34.Value[0]=mpp_cvmCode;	//'CVM Performed' in CVM Results := CVM Code
	glv_tag9F34.Value[1]=mpp_cvmCndCode;//'CVM Condition' in CVM Results := 'CVM Condition Code
	glv_tag9F34.Value[2]=0x01;			//'CVM Result' in CVM Results := '01' (failed)

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 24, 0xFF);
}

void MPP_CVM_25(void)
{
	glv_tag9F34.Value[0]=0x3F;	//'CVM Performed' in CVM Results := '3F'
	glv_tag9F34.Value[1]=0x00;	//'CVM Condition' in CVM Results := '00'
	glv_tag9F34.Value[2]=0x01;	//'CVM Result' in CVM Results := '01' (failed)

	MPP_DBG_Put_Process(MPP_PROCEDURE_CVMSelection, 25, 0xFF);
}

UCHAR MPP_GAC_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8128.Value[0] & MPP_IDS_Read)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 1, rspCode);
	
	return rspCode;
}

UCHAR MPP_GAC_2(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tag95.Value[0] & MPP_TVR_CDAFailed)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 2, rspCode);
	
	return rspCode;
}

UCHAR MPP_GAC_3(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tofDF62);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 3, rspCode);
	
	return rspCode;
}

UCHAR MPP_GAC_4(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof9F5B);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 4, rspCode);
	
	return rspCode;
}

UCHAR MPP_GAC_5(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tofDF8108);
	if (rspCode == TRUE)
	{
		rspCode=MPP_IsNotEmpty(mpp_tofDF810A);
		if (rspCode == TRUE)
			rspCode=TRUE;
		else
			rspCode=FALSE;
	}
	else
	{
		rspCode=FALSE;
	}

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 5, rspCode);
	
	return rspCode;
}

void MPP_GAC_6(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_IdsDataError;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 6, 0xFF);
}

UCHAR MPP_GAC_7(void)
{
	UCHAR	rspCode=FALSE;
	
	if (((glv_tagDF8108.Value[0] & 0xC0) == MPP_ACT_AAC) ||											//('AC type' in DS AC Type = AAC) OR
		((mpp_tagACType.Value[0] & 0xC0) == (glv_tagDF8108.Value[0] & 0xC0)) ||									//('AC type' in AC Type = 'AC type' in DS AC Type) OR
		(((glv_tagDF8108.Value[0] & 0xC0) == MPP_ACT_ARQC) && ((mpp_tagACType.Value[0] & 0xC0) == MPP_ACT_TC)))	//(('AC type' in DS AC Type = ARQC) AND ('AC type' in AC Type = TC))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 7, rspCode);
	
	return rspCode;
}

void MPP_GAC_8(void)
{
	mpp_tagACType.Value[0]&=0x3F;	//Reset AC Type
	mpp_tagACType.Value[0]|=(glv_tagDF8108.Value[0] & 0xC0);

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 8, 0xFF);
}

UCHAR MPP_GAC_9(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((((mpp_tagACType.Value[0] & 0xC0) == MPP_ACT_AAC) && (glv_tagDF810A.Value[0] & MPP_DOI_UsableForAAC)) ||	//(('AC type' in AC Type = AAC) AND 'Usable for AAC' in DS ODS Info For Reader is set) OR
		(((mpp_tagACType.Value[0] & 0xC0) == MPP_ACT_ARQC) && (glv_tagDF810A.Value[0] & MPP_DOI_UsableForARQC)))	//(('AC type' in AC Type = ARQC) AND 'Usable for ARQC' in DS ODS Info For Reader is set)
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 9, rspCode);
	
	return rspCode;
}

UCHAR MPP_GAC_10(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF810A.Value[0] & MPP_DOI_StopIfNoDsOdsTerm)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 10, rspCode);
	
	return rspCode;
}

void MPP_GAC_11(void)
{
	//glv_tagDF8115.Value[2] = MPP_EID_LV2_IdsNoMatchingAc;
	glv_tagDF8115.Value[1] = MPP_EID_LV2_IdsNoMatchingAc;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 11, 0xFF);
}

void MPP_GAC_12(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	//glv_tagDF8116.Value[0] = MPP_UIR_STATUS_NotReady;
	glv_tagDF8116.Value[1] = MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 12, 0xFF);
}

void MPP_GAC_13(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_UIR_MID_Error_OtherCard;

	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;
	
	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 13, 0xFF);
}

UCHAR MPP_GAC_20(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_tagODAStatus.Value[0] & MPP_ODS_CDA)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 20, rspCode);
	
	return rspCode;
}

UCHAR MPP_GAC_21(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tag95.Value[0] & MPP_TVR_CDAFailed)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 21, rspCode);
	
	return rspCode;
}

UCHAR MPP_GAC_22(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((glv_tag82.Value[0] & MPP_AIP_OnDeviceCardholderVerificationIsSupported) &&
		(glv_tagDF811B.Value[0] & MPP_KCF_OnDeviceCardholderVerificationSupported))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 22, rspCode);
	
	return rspCode;
}

void MPP_GAC_23(void)
{
	mpp_tagACType.Value[0]=MPP_ACT_AAC;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 23, 0xFF);
}

UCHAR MPP_GAC_24(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_tagACType.Value[0] == MPP_ACT_AAC)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 24, rspCode);
	
	return rspCode;
}

UCHAR MPP_GAC_25(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof9F5D);
	if ((rspCode == TRUE) && 
		(glv_tag9F5D.Value[1] & MPP_ACI_CDAIndicator_CDASupportedOverTC_ARQC_AAC))
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 25, rspCode);

	return rspCode;
}

void MPP_GAC_26(void)
{
	glv_tagDF8114.Length[0]=ECL_LENGTH_DF8114;
	glv_tagDF8114.Value[0]=0x00;	//Reference Control Parameter
	glv_tagDF8114.Value[0]|=(mpp_tagACType.Value[0] & 0xC0);

	MPP_AddToList(mpp_tofDF8114, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 26, 0xFF);
}

void MPP_GAC_27(void)
{
	glv_tagDF8114.Length[0]=ECL_LENGTH_DF8114;
	glv_tagDF8114.Value[0]=0x00;	//Reference Control Parameter
	glv_tagDF8114.Value[0]|=(mpp_tagACType.Value[0] & 0xC0);
	glv_tagDF8114.Value[0]|=MPP_RCP_CDASignatureRequested;

	MPP_AddToList(mpp_tofDF8114, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 27, 0xFF);
}

void MPP_GAC_29(void)
{
	UINT	lenOfV=0, total=0;
	UCHAR	lenDolRelData=0;
	UCHAR	bufDolRelData[255]={0};

	UT_Get_TLVLengthOfV(glv_tag8C.Length, &lenOfV);

	MPP_DOL_Get_DOLRelatedData(lenOfV, glv_tag8C.Value, &lenDolRelData, bufDolRelData);

	UT_Set_TagLength(lenDolRelData, glv_tagDF8107.Length);
	memcpy(glv_tagDF8107.Value, bufDolRelData, lenDolRelData);
	MPP_AddToList(mpp_tofDF8107, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	// Save CDOL in "tempSend"			
	UT_Get_TLVLengthOfV(glv_tagDF8107.Length, &lenOfV);
	memcpy(mpp_tempSend.Value, glv_tagDF8107.Value, lenOfV);
	total += lenOfV;
	UT_Set_TagLength(total, mpp_tempSend.Length);

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 29, 0xFF);
}

UCHAR MPP_GAC_40(void)
{
	UCHAR	rspCode=FALSE;
	UINT	lenOfV=0;
	UINT	i=0;

	UT_Get_TLVLengthOfV(glv_tag9F5B.Length, &lenOfV);
		
	while (i < lenOfV)
	{
		if (!memcmp(mpp_tofDF61, (glv_tag9F5B.Value) + i, 2))
		{
			rspCode = TRUE;
			break;
		}
		i++;
	}

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 40, rspCode);
	
	return rspCode;
}

UCHAR MPP_GAC_41(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsPresent(mpp_tofDF8109);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 41, rspCode);
	
	return rspCode;
}

UCHAR MPP_GAC_42(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tag9F5D.Value[0] & MPP_ACI_DSVersionNumber_Version1)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 42, rspCode);
	
	return rspCode;
}

void MPP_GAC_43(void)
{
	MPP_OWHF2(glv_tagDF8109.Value, glv_tagDF61.Value);	//DS Digest H := OWHF2(DS Input (Term))
	UT_Set_TagLength(8, glv_tagDF61.Length);
	MPP_AddToList(mpp_tofDF61, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 43, 0xFF);
}

void MPP_GAC_44(void)
{
	MPP_OWHF2AES(glv_tagDF8109.Value, glv_tagDF61.Value);	//DS Digest H := OWHF2AES(DS Input (Term))
	UT_Set_TagLength(8, glv_tagDF61.Length);
	MPP_AddToList(mpp_tofDF61, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 44, 0xFF);
}

void MPP_GAC_45(void)
{
	glv_tagDF8114.Length[0]=ECL_LENGTH_DF8114;
	glv_tagDF8114.Value[0]=0x00;	//Reference Control Parameter
	glv_tagDF8114.Value[0]|=(mpp_tagACType.Value[0] & 0xC0);
	glv_tagDF8114.Value[0]|=MPP_RCP_CDASignatureRequested;

	MPP_AddToList(mpp_tofDF8114, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 45, 0xFF);
}

void MPP_GAC_47(void)
{
//	Prepare GENERATE AC command as specified in section 6.3.2. Use CDOL1 and DSDOL to create CDOL1 Related Data and DSDOL related data as concatenated lists of data objects without tags or lengths following the rules specified in section 5.1.4.
	// 1. CDOL
	UINT	lenOfV = 0, total = 0;
	UCHAR	lenDolRelData = 0;
	UCHAR	bufDolRelData[255] = { 0 };

	UT_Get_TLVLengthOfV(glv_tag8C.Length, &lenOfV);

	MPP_DOL_Get_DOLRelatedData(lenOfV, glv_tag8C.Value, &lenDolRelData, bufDolRelData);
	UT_Set_TagLength(lenDolRelData, glv_tagDF8107.Length);
	memcpy(glv_tagDF8107.Value, bufDolRelData, lenDolRelData);
	MPP_AddToList(mpp_tofDF8107, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	// 2. DSDOL
	lenDolRelData = 0;
	memset(bufDolRelData, 0, 255);
	UT_Get_TLVLengthOfV(glv_tag9F5B.Length, &lenOfV);

	MPP_DSDOL_Get_DOLRelatedData(lenOfV, glv_tag9F5B.Value, &lenDolRelData, bufDolRelData);

	UT_Set_TagLength(lenDolRelData, glv_tag9F5B.Length);
	memcpy(glv_tag9F5B.Value, bufDolRelData, lenDolRelData);

	// Save CDOL + DSDOL in "tempSend"
	UT_Get_TLVLengthOfV(glv_tagDF8107.Length, &lenOfV);
	memcpy(mpp_tempSend.Value, glv_tagDF8107.Value, lenOfV);
	total += lenOfV;
	UT_Get_TLVLengthOfV(glv_tag9F5B.Length, &lenOfV);
	memcpy(&mpp_tempSend.Value[total], glv_tag9F5B.Value, lenOfV);
	total += lenOfV;
	UT_Set_TagLength(total, mpp_tempSend.Length);

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 47, 0xFF);
}

void MPP_GAC_48(void)
{
	glv_tagDF8128.Value[0]|=MPP_IDS_Write;

	MPP_DBG_Put_Process(MPP_PROCEDURE_PrepareGACCommand, 48, 0xFF);
}

UCHAR MPP_PRE_1(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_IsNotEmpty(mpp_tof9F08);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 1, rspCode);
	
	return rspCode;
}

UCHAR MPP_PRE_2(void)
{
	UCHAR	rspCode=FALSE;

	if (!memcmp(glv_tag9F08.Value, glv_tag9F09.Value, 2))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 2, rspCode);
	
	return rspCode;
}

void MPP_PRE_3(void)
{
	glv_tag95.Value[1]|=MPP_TVR_ICCAndTerminalHaveDifferentApplicationVersions;	//SET 'ICC and terminal have different application versions' in Terminal Verification Results

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 3, 0xFF);
}

UCHAR MPP_PRE_4(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_IsNotEmpty(mpp_tof5F25);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 4, rspCode);
	
	return rspCode;
}

UCHAR MPP_PRE_5(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_Compare_Date(glv_tag9A.Value, glv_tag5F25.Value);
	if (rspCode == 2)	//Transaction Date is before Application Effective Date
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 5, rspCode);

	return rspCode;
}

void MPP_PRE_6(void)
{
	glv_tag95.Value[1]|=MPP_TVR_ApplicationNotYetEffective;	//SET 'Application not yet effective' in Terminal Verification Results

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 6, 0xFF);
}

UCHAR MPP_PRE_7(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_Compare_Date(glv_tag9A.Value, glv_tag5F24.Value);
	if (rspCode == 1)	//Transaction Date is after Application Expiration Date
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 7, rspCode);

	return rspCode;
}

void MPP_PRE_8(void)
{
	glv_tag95.Value[1]|=MPP_TVR_ExpiredApplication;	//SET 'Expired application' in Terminal Verification Results

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 8, 0xFF);
}

UCHAR MPP_PRE_9(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_IsNotEmpty(mpp_tof9F07);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 9, rspCode);
	
	return rspCode;
}

UCHAR MPP_PRE_10(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((	(glv_tag9F35.Value[0] == 0x14) ||	//[(Terminal Type = '14')  OR
			(glv_tag9F35.Value[0] == 0x15) ||	// (Terminal Type = '15')  OR
			(glv_tag9F35.Value[0] == 0x16)) &&	// (Terminal Type = '16')] AND
			(glv_tag9F40.Value[0] & 0x80))		//'Cash' in Additional Terminal Capabilities is set
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 10, rspCode);
	
	return rspCode;
}

UCHAR MPP_PRE_11(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tag9F07.Value[0] & MPP_AUC_ValidAtTerminalsOtherThanATMs)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 11, rspCode);
	
	return rspCode;
}

UCHAR MPP_PRE_12(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tag9F07.Value[0] & MPP_AUC_ValidAtATMs)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 12, rspCode);
	
	return rspCode;
}

void MPP_PRE_13(void)
{
	glv_tag95.Value[1]|=MPP_TVR_RequestedServiceNotAllowedForCardProduct;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 13, 0xFF);
}

UCHAR MPP_PRE_14(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_IsNotEmpty(mpp_tof5F28);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 14, rspCode);
		
	return rspCode;
}

UCHAR MPP_PRE_15(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((glv_tag9C.Value[0] == MPP_TXT_Cash) || (glv_tag9C.Value[0] == MPP_TXT_CashDisbursement))	//Transaction Type indicates cash transaction
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 15, rspCode);
	
	return rspCode;
}

UCHAR MPP_PRE_16(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=UT_bcdcmp(glv_tag9F1A.Value, glv_tag5F28.Value, 2);
	if (rspCode == 0)	//Terminal Country Code = Issuer Country Code
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 16, rspCode);
	
	return rspCode;
}

UCHAR MPP_PRE_17(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tag9F07.Value[0] & MPP_AUC_ValidForDomesticCashTransactions)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 17, rspCode);
	
	return rspCode;
}

UCHAR MPP_PRE_18(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tag9F07.Value[0] & MPP_AUC_ValidForInternationalCashTransactions)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 18, rspCode);
	
	return rspCode;
}

void MPP_PRE_19(void)
{
	glv_tag95.Value[1]|=MPP_TVR_RequestedServiceNotAllowedForCardProduct;	//SET 'Requested service not allowed for card product' in Terminal Verification Results

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 19, 0xFF);
}

UCHAR MPP_PRE_20(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((glv_tag9C.Value[0] == MPP_TXT_Purchase) || (glv_tag9C.Value[0] == MPP_TXT_CashBack))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 20, rspCode);
	
	return rspCode;
}

UCHAR MPP_PRE_21(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=UT_bcdcmp(glv_tag9F1A.Value, glv_tag5F28.Value, 2);
	if (rspCode == 0)	//Terminal Country Code = Issuer Country Code
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 21, rspCode);
	
	return rspCode;
}

UCHAR MPP_PRE_22(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((glv_tag9F07.Value[0] & MPP_AUC_ValidForDomesticGoods) || 
		(glv_tag9F07.Value[0] & MPP_AUC_ValidForDomesticServices))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 22, rspCode);
	
	return rspCode;
}

UCHAR MPP_PRE_23(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((glv_tag9F07.Value[0] & MPP_AUC_ValidForInternationalGoods) ||
		(glv_tag9F07.Value[0] & MPP_AUC_ValidForInternationalServices))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 23, rspCode);
	
	return rspCode;
}

void MPP_PRE_24(void)
{
	glv_tag95.Value[1]|=MPP_TVR_RequestedServiceNotAllowedForCardProduct;	//SET 'Requested service not allowed for card product' in Terminal Verification Results

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 24, 0xFF);
}

UCHAR MPP_PRE_25(void)
{
	UCHAR	zroAmount[6]={0};		//Zero Amount for Compare
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_IsPresent(mpp_tof9F03);
	if (rspCode == TRUE)
	{
		if (memcmp(glv_tag9F03.Value, zroAmount, 6) != 0)
			rspCode=TRUE;
		else
			rspCode=FALSE;
	}
	else
	{
		rspCode=FALSE;
	}

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 25, rspCode);
	
	return rspCode;
}

UCHAR MPP_PRE_26(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=UT_bcdcmp(glv_tag9F1A.Value, glv_tag5F28.Value, 2);
	if (rspCode == 0)	//Terminal Country Code = Issuer Country Code
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 26, rspCode);
	
	return rspCode;
}

UCHAR MPP_PRE_27(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tag9F07.Value[1] & MPP_AUC_DomesticCashbackAllowed)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 27, rspCode);

	return rspCode;
}

UCHAR MPP_PRE_28(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tag9F07.Value[1] & MPP_AUC_InternationalCashbackAllowed)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 28, rspCode);
	
	return rspCode;
}

void MPP_PRE_29(void)
{
	glv_tag95.Value[1]|=MPP_TVR_RequestedServiceNotAllowedForCardProduct;	//SET 'Requested service not allowed for card product' in Terminal Verification Results

	MPP_DBG_Put_Process(MPP_PROCEDURE_ProcessingRestrictions, 29, 0xFF);
}

UCHAR MPP_TAA_1(void)
{	
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_IsNotEmpty(mpp_tof9F0E);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 1, rspCode);
	
	return rspCode;
}

UCHAR MPP_TAA_2(void)
{
	UCHAR	zroTVR[5]={0};		//Zero TVR for Compare
	UCHAR	tmpTVR[5]={0};
	UCHAR	idxNum=0;
	UCHAR	rspCode=FALSE;
	
	for (idxNum=0; idxNum < 5; idxNum++)
	{
		tmpTVR[idxNum]=glv_tagDF8121.Value[idxNum] & glv_tag95.Value[idxNum];
	}

	if (!memcmp(tmpTVR, zroTVR, 5))
		return TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 2, rspCode);
	
	return rspCode;
}

void MPP_TAA_3(void)
{
	mpp_tagACType.Value[0]=MPP_ACT_AAC;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 3, 0xFF);
}

UCHAR MPP_TAA_4(void)
{
	UCHAR	zroTVR[5]={0};		//Zero TVR for Compare
	UCHAR	tmpTVR1[5]={0};
	UCHAR	tmpTVR2[5]={0};
	UCHAR	idxNum=0;
	UCHAR	rspCode=FALSE;

	for (idxNum=0; idxNum < 5; idxNum++)
	{
		tmpTVR1[idxNum]=glv_tagDF8121.Value[idxNum] | glv_tag9F0E.Value[idxNum];
		tmpTVR2[idxNum]=tmpTVR1[idxNum] & glv_tag95.Value[idxNum];
	}	
	
	if (!memcmp(tmpTVR2, zroTVR, 5))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 4, rspCode);
	
	return rspCode;
}

UCHAR MPP_TAA_4_1(void)
{
	UCHAR	rspCode = FALSE;

	if ((glv_tag9F35.Value[0] == 0x11) ||	//Terminal Type
		(glv_tag9F35.Value[0] == 0x21) ||
		(glv_tag9F35.Value[0] == 0x14) ||
		(glv_tag9F35.Value[0] == 0x24) ||
		(glv_tag9F35.Value[0] == 0x34))
	{
		rspCode = TRUE;
	}

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 4, rspCode);

	return rspCode;
}

void MPP_TAA_4_2(void)
{
	mpp_tagACType.Value[0] = MPP_ACT_ARQC;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 4, 0xFF);
}

void MPP_TAA_5(void)
{
	mpp_tagACType.Value[0]=MPP_ACT_AAC;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 5, 0xFF);
}

UCHAR MPP_TAA_6(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((glv_tag9F35.Value[0] == 0x23) ||	//Terminal Type
		(glv_tag9F35.Value[0] == 0x26) ||
		(glv_tag9F35.Value[0] == 0x36) ||
		(glv_tag9F35.Value[0] == 0x13) ||
		(glv_tag9F35.Value[0] == 0x16))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 6, rspCode);
	
	return rspCode;
}

UCHAR MPP_TAA_7(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_IsNotEmpty(mpp_tof9F0F);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 7, rspCode);
	
	return rspCode;
}

UCHAR MPP_TAA_8(void)
{
	UCHAR	zroTVR[5]={0};		//Zero TVR for Compare
	UCHAR	rspCode=FALSE;
	
	if (!memcmp(glv_tag95.Value, zroTVR, 5))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 8, rspCode);
	
	return rspCode;
}

void MPP_TAA_9(void)
{
	mpp_tagACType.Value[0]=MPP_ACT_TC;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 9, 0xFF);
}

UCHAR MPP_TAA_10(void)
{
	UCHAR	zroTVR[5]={0};		//Zero TVR for Compare
	UCHAR	tmpTVR1[5]={0};
	UCHAR	tmpTVR2[5]={0};
	UCHAR	idxNum=0;
	UCHAR	rspCode=FALSE;

	for (idxNum=0; idxNum < 5; idxNum++)
	{
		tmpTVR1[idxNum]=glv_tagDF8122.Value[idxNum] | glv_tag9F0F.Value[idxNum];
		tmpTVR2[idxNum]=tmpTVR1[idxNum] & glv_tag95.Value[idxNum];
	}	
	
	if (!memcmp(tmpTVR2, zroTVR, 5))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 10, rspCode);
	
	return rspCode;
}

void MPP_TAA_11(void)
{
	mpp_tagACType.Value[0]=MPP_ACT_ARQC;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 11, 0xFF);
}

void MPP_TAA_12(void)
{
	mpp_tagACType.Value[0]=MPP_ACT_TC;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 12, 0xFF);
}

UCHAR MPP_TAA_13(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_IsNotEmpty(mpp_tof9F0D);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 13, rspCode);
	
	return rspCode;
}

UCHAR MPP_TAA_14(void)
{
	UCHAR	zroTVR[5]={0};		//Zero TVR for Compare
	UCHAR	rspCode=FALSE;
	
	if (!memcmp(glv_tag95.Value, zroTVR, 5))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 14, rspCode);
	
	return rspCode;
}

void MPP_TAA_15(void)
{
	mpp_tagACType.Value[0]=MPP_ACT_TC;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 15, 0xFF);
}

UCHAR MPP_TAA_16(void)
{
	UCHAR	zroTVR[5]={0};		//Zero TVR for Compare
	UCHAR	tmpTVR1[5]={0};
	UCHAR	tmpTVR2[5]={0};
	UCHAR	idxNum=0;
	UCHAR	rspCode=FALSE;

	for (idxNum=0; idxNum < 5; idxNum++)
	{
		tmpTVR1[idxNum]=glv_tagDF8120.Value[idxNum] | glv_tag9F0D.Value[idxNum];
		tmpTVR2[idxNum]=tmpTVR1[idxNum] & glv_tag95.Value[idxNum];
	}	
	
	if (!memcmp(tmpTVR2, zroTVR, 5))
		return TRUE;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 16, rspCode);
			
	return rspCode;
}

void MPP_TAA_17(void)
{
	mpp_tagACType.Value[0]=MPP_ACT_AAC;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 17, 0xFF);
}

void MPP_TAA_18(void)
{
	mpp_tagACType.Value[0]=MPP_ACT_TC;

	MPP_DBG_Put_Process(MPP_PROCEDURE_TerminalActionAnalysis, 18, 0xFF);
}


UCHAR MPP_Procedure_PreGenACBalanceReading(void)
{
	UCHAR	rspCode=FALSE;

	//BR1
	rspCode=MPP_BR1_1();
	if (rspCode == TRUE)
	{
		rspCode=MPP_BR1_2();
		if (rspCode == TRUE)
		{
			MPP_BR1_3();
			MPP_BR1_4();
			
			return MPP_STATE_16;
		}
	}
	
	return MPP_STATE_RETURN;
}


UCHAR MPP_Procedure_PostGenACBalanceReading(void)
{
	UCHAR	rspCode=FALSE;

	//BR2
	rspCode=MPP_BR2_1();
	if (rspCode == TRUE)
	{
		rspCode=MPP_BR2_2();
		if (rspCode == TRUE)
		{
			MPP_BR2_3();
			MPP_BR2_4();
			
			return MPP_STATE_17;
		}
	}
	
	return MPP_STATE_RETURN;
}


UCHAR MPP_Procedure_CVMSelection(void)
{
	UCHAR	rspCode=0;		//Response Code
	UCHAR	flg13=FALSE;	//Flag of GOTO CVM.13

	//Reset CVM List Index
	mpp_cvmLstIndex=0;
	
	//CVM
	rspCode=MPP_CVM_1();
	if (rspCode == TRUE)
	{
		rspCode=MPP_CVM_2();
		if (rspCode == TRUE)
		{
			MPP_CVM_4();
		}
		else
		{
			MPP_CVM_3();
		}

		return MPP_STATE_RETURN;
	}
	else
	{
		rspCode=MPP_CVM_5();
		if (rspCode == TRUE)
		{
			rspCode=MPP_CVM_7();
			if (rspCode == TRUE)
			{
				MPP_CVM_8();

				return MPP_STATE_RETURN;
			}
			else
			{
				MPP_CVM_9();
			}
		}
		else
		{
			MPP_CVM_6();

			return MPP_STATE_RETURN;
		}
	}

	while (1)
	{
		rspCode=MPP_CVM_10();
		if (rspCode == TRUE)
		{
			rspCode=MPP_CVM_11();
			if (rspCode == TRUE)
			{
				rspCode=MPP_CVM_12();
				if (rspCode == TRUE)
				{
					rspCode=MPP_CVM_15();
					if (rspCode == TRUE)
					{
						rspCode=MPP_CVM_17();
						if (rspCode == TRUE)
						{
							MPP_CVM_18();

							return MPP_STATE_RETURN;
						}
					}
					else
					{
						MPP_CVM_16();
					}

					rspCode=MPP_CVM_19();
					if (rspCode == TRUE)
					{
						rspCode=MPP_CVM_20();
						if (rspCode == TRUE)
						{
							break;	//GOTO CVM.22
						}
					}
					else
					{
						break;	//GOTO CVM.22
					}
				}
				else
				{
					flg13=TRUE;
				}
			}
			else
			{
				flg13=TRUE;
			}
		}
		else
		{
			flg13=TRUE;
		}

		if (flg13 == TRUE)
		{
			rspCode=MPP_CVM_13();
			if (rspCode == TRUE)
			{
				MPP_CVM_14();
				
				return MPP_STATE_RETURN;
			}
		}
		else
		{
			flg13=FALSE;
		}

		MPP_CVM_21();
	}

	MPP_CVM_22();

	rspCode=MPP_CVM_23();
	if (rspCode == TRUE)
	{
		MPP_CVM_24();
	}
	else
	{
		MPP_CVM_25();
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_Procedure_PrepareGenACCommand(void)
{
	UCHAR	flwControl=0;	//Flow Control
	UCHAR	flg22=FALSE;	//Flag of GOTO 22
	UCHAR	flg24=FALSE;	//Flag of GOTO 24
	UCHAR	flg27=FALSE;	//Flag of GOTO 27
	UCHAR	rspCode=FALSE;
	
	//GAC
	rspCode=MPP_GAC_1();
	if (rspCode == TRUE)
	{
		rspCode=MPP_GAC_2();
		if (rspCode == TRUE)
		{
			flwControl='D';	//CDA Failed
		}
		else
		{
			rspCode=MPP_GAC_3();
			if (rspCode == TRUE)
			{
				rspCode=MPP_GAC_4();
				if (rspCode == TRUE)
				{
					rspCode=MPP_GAC_5();
					if (rspCode == TRUE)
					{
						rspCode=MPP_GAC_7();
						if (rspCode == TRUE)
						{
							MPP_GAC_8();

							flwControl='A';	//IDS Write
						}
						else
						{
							rspCode=MPP_GAC_9();
							if (rspCode == TRUE)
							{
								flwControl='A';	//IDS Write
							}
							else
							{
								rspCode=MPP_GAC_10();
								if (rspCode == TRUE)
								{
									MPP_GAC_11();
								}
								else
								{
									flwControl='C';	//IDS Read Only
								}
							}
						}
					}
					else
					{
						MPP_GAC_6();
					}
				}
				else
				{
					flwControl='C';	//IDS Read Only
				}
			}
			else
			{
				flwControl='C';	//IDS Read Only
			}
		}
	}
	else
	{
		flwControl='B';	//No IDS
	}

	switch (flwControl)
	{
		case 0:
			MPP_GAC_12();
			MPP_GAC_13();

			return MPP_STATE_EXITKERNEL;

		case 'A':
			rspCode=MPP_GAC_40();
			if (rspCode == TRUE)
			{
				rspCode=MPP_GAC_41();
				if (rspCode == TRUE)
				{
					rspCode=MPP_GAC_42();
					if (rspCode == TRUE)
					{
						MPP_GAC_43();
					}
					else
					{
						MPP_GAC_44();
					}
				}
			}

			MPP_GAC_45();
			MPP_GAC_47();
			MPP_GAC_48();

			break;

		case 'B':
			rspCode=MPP_GAC_20();
			if (rspCode == TRUE)
			{
				rspCode=MPP_GAC_21();
				if (rspCode == TRUE)
				{
					flg22=TRUE;
				}
				else
				{
					flg24=TRUE;
				}
			}
			else
			{
				flg22=TRUE;
			}

		case 'C':
		case 'D':
			if (flwControl == 'C')
				flg27=TRUE;
			else if (flwControl == 'D')
				flg22=TRUE;

			if (flg22)
			{
				rspCode=MPP_GAC_22();
				if (rspCode == TRUE)
				{
					MPP_GAC_23();
				}
			}

			if (flg24)
			{
				rspCode=MPP_GAC_24();
				if (rspCode == TRUE)
				{
					rspCode=MPP_GAC_25();
					if (rspCode == TRUE)
					{
						flg27=TRUE;
					}
				}
				else
				{
					flg27=TRUE;
				}
			}

			if (flg27)
			{
				MPP_GAC_27();
			}
			else
			{
				MPP_GAC_26();
			}

			MPP_GAC_29();

		default: break;
	}
	
	return MPP_STATE_RETURN;
}


UCHAR MPP_Procedure_ProcessingRestrictions(void)
{
	UCHAR	flg13=FALSE;				//Flag of GOTO PRE.13
	UCHAR	rspCode=FALSE;				//Response Code

	//PRE
	//Application Version Number Checking
	rspCode=MPP_PRE_1();
	if (rspCode == TRUE)
	{
		rspCode=MPP_PRE_2();
		if (rspCode == TRUE)
		{
			;	//GOTO PRE.4
		}
		else
		{
			MPP_PRE_3();
		}
	}

	//Application Effective/Expiration Date Checking
	rspCode=MPP_PRE_4();
	if (rspCode == TRUE)
	{
		rspCode=MPP_PRE_5();
		if (rspCode == TRUE)
		{
			MPP_PRE_6();
		}
	}

	rspCode=MPP_PRE_7();
	if (rspCode == TRUE)
	{
		MPP_PRE_8();
	}
	
	//Application Usage Control Checking
	rspCode=MPP_PRE_9();
	if (rspCode == TRUE)
	{
		;	//GOTO PRE.10
	}
	else
	{
		return MPP_STATE_RETURN;
	}

	rspCode=MPP_PRE_10();
	if (rspCode == TRUE)
	{
		rspCode=MPP_PRE_12();
		if (rspCode == TRUE)
		{
			;	//GOTO PRE.14
		}
		else
		{
			flg13=TRUE;
		}
	}
	else
	{
		rspCode=MPP_PRE_11();
		if (rspCode == TRUE)
		{
			;	//GOTO PRE.14
		}
		else
		{
			flg13=TRUE;
		}
	}

	if (flg13 == TRUE)
	{
		MPP_PRE_13();
		
		return MPP_STATE_RETURN;
	}

	rspCode=MPP_PRE_14();
	if (rspCode == TRUE)
	{
		;
	}
	else
	{
		return MPP_STATE_RETURN;
	}

	rspCode=MPP_PRE_15();
	if (rspCode == TRUE)
	{
		rspCode=MPP_PRE_16();
		if (rspCode == TRUE)
		{
			rspCode=MPP_PRE_17();
			if(rspCode == FALSE)
			{
				MPP_PRE_19();
			}
		}
		else
		{
			rspCode=MPP_PRE_18();
			if(rspCode == FALSE)
			{
				MPP_PRE_19();
			}
		}
	}

	rspCode=MPP_PRE_20();
	if (rspCode == TRUE)
	{
		rspCode=MPP_PRE_21();
		if (rspCode == TRUE)
		{
			rspCode=MPP_PRE_22();
			if(rspCode == FALSE)
			{
				MPP_PRE_24();
			}
		}
		else
		{
			rspCode=MPP_PRE_23();
			if(rspCode == FALSE)
			{
				MPP_PRE_24();
			}
		}
	}

	rspCode=MPP_PRE_25();
	if (rspCode == TRUE)
	{
		rspCode=MPP_PRE_26();
		if (rspCode == TRUE)
		{
			rspCode=MPP_PRE_27();
			if (rspCode == TRUE)
			{
				return MPP_STATE_RETURN;
			}
		}
		else
		{
			rspCode=MPP_PRE_28();
			if (rspCode == TRUE)
			{
				return MPP_STATE_RETURN;
			}
		}
	}
	else
	{
		return MPP_STATE_RETURN;
	}

	MPP_PRE_29();

	return MPP_STATE_RETURN;
}


UCHAR MPP_Procedure_TerminalActionAnalysis(void)
{
	UCHAR	rspCode=FALSE;			//Response Code

	//TAA
	rspCode=MPP_TAA_1();
	if (rspCode == TRUE)
	{
		rspCode=MPP_TAA_4();
		if (rspCode == TRUE)
		{
			;	//GOTO TAA.4.1
		}
		else
		{
			MPP_TAA_5();

			return MPP_STATE_RETURN;
		}
	}
	else
	{
		rspCode=MPP_TAA_2();
		if (rspCode == TRUE)
		{
			;	//GOTO TAA.4.1
		}
		else
		{
			MPP_TAA_3();

			return MPP_STATE_RETURN;
		}
	}
	
	rspCode = MPP_TAA_4_1();
	if(rspCode == TRUE)
	{
		MPP_TAA_4_2();

		return MPP_STATE_RETURN;
	}

	rspCode=MPP_TAA_6();
	if (rspCode == TRUE)
	{
		rspCode=MPP_TAA_13();
		if (rspCode == TRUE)
		{
			rspCode=MPP_TAA_16();
			if (rspCode == TRUE)
			{
				MPP_TAA_18();

				return MPP_STATE_RETURN;
			}
		}
		else
		{
			rspCode=MPP_TAA_14();
			if (rspCode == TRUE)
			{
				MPP_TAA_15();

				return MPP_STATE_RETURN;
			}
		}

		MPP_TAA_17();
	}
	else
	{
		rspCode=MPP_TAA_7();
		if (rspCode == TRUE)
		{
			rspCode=MPP_TAA_10();
			if (rspCode == TRUE)
			{
				MPP_TAA_12();

				return MPP_STATE_RETURN;
			}
		}
		else
		{
			rspCode=MPP_TAA_8();
			if (rspCode == TRUE)
			{
				MPP_TAA_9();

				return MPP_STATE_RETURN;
			}
		}

		MPP_TAA_11();
	}

	return MPP_STATE_RETURN;
}

