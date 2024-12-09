#include <string.h>
#include "POSAPI.h"
#include "ECL_Tag.h"
#include "ECL_Tag_Define.h"
#include "Function.h"
#include "Glv_ReaderConfPara.h"
#include "VAP_ReaderInterface_Define.h"
#include "MPP_Define.h"
#include "MPP_Function.h"
#include "DPS_Kernel_Define.h"
#include "FLSAPI.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Function.h"
#include "ITR_Function.h"
#include "Define.h"

#ifdef _PLATFORM_AS210
#include "xioapi.h"
#else
#include "LCDTFTAPI.h"
#endif

// Add by Wayne 2020/08/21 to avoid compiler warning
// <-------------
#include "UTILS_CTLS.H"
#include "DBG_Function.h"
extern UCHAR MPP_Allocate_DE_LogBuffer(void);
extern void MPP_Free_DE_LogBuffer(void);

// <-------------
//add by West for debug used
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <time.h>
extern struct timeval GetNowTime();
//AIDs
const UCHAR	etp_aidNA[7]	={0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const UCHAR	etp_aidVISA[7]	={0xA0,0x00,0x00,0x00,0x03,0x10,0x10};
const UCHAR	etp_aidMaster[7]={0xA0,0x00,0x00,0x00,0x04,0x10,0x10};
const UCHAR	etp_aidJCB[7]	={0xA0,0x00,0x00,0x00,0x65,0x10,0x10};
const UCHAR	etp_aidUPI[7]	={0xA0,0x00,0x00,0x03,0x33,0x01,0x01};
const UCHAR	etp_aidAMEX[7]	={0xA0,0x00,0x00,0x00,0x25,0x01,0x00};
const UCHAR	etp_aidDPS[7]	={0xA0,0x00,0x00,0x01,0x52,0x30,0x10};

//Element of Entry Point
CMBTABLE	etp_cmbTable[ETP_NUMBER_TRANSACTIONTYPE][ETP_NUMBER_COMBINATION]={{{0,{0},{0}}}};		//Combination Table(0: Payment, 1: Cash, 2: CashBack, 3:Refund) 4:Test
ETPCONFIG	etp_etpConfig[ETP_NUMBER_COMBINATION]={{0,0,0,0,0,{0},0,{0},0,{0},0,{0},0,{0},0,0}};	//Entry Point Configuration
INDICATOR	etp_preIndicator[ETP_NUMBER_COMBINATION]={{0,0,0,0,0,{0}}};								//Pre-Processing Indicator
CDDLIST		etp_cddList[ETP_NUMBER_CANDIDATELIST]={{0,{0},0,{0},0,0,{0},0}};						//Candidate List
OUTCOME		etp_Outcome={0,{0},0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};										//Entry Point Outcome

//Flag
UCHAR		etp_flgAutorun=FALSE;				//Autorun
UCHAR		etp_flgRestart=FALSE;				//Reader Restart (IAD/Issuer Update)
UCHAR		etp_flgComError=FALSE;				//Communication Error
UCHAR 		etp_flgIsuUpdateErr = FALSE;		//0625 ICTK, Issuer Script represent the card
UCHAR 		etp_flgCmdTimeout = FALSE;			//Command Timeout
UCHAR		etp_flgDrlNotAllow=FALSE;			//DRL CL Transaction Limit
UCHAR		etp_flgMtiAID_UPI=FALSE;			//qUICS Muilt AID Mode
UCHAR		etp_flgMtiAID_Discover=FALSE;		//D-PAS Muilt AID Mode
UCHAR		etp_flgUnknowAID=FALSE;				//Unknow AID
UCHAR		etp_flgCDCVM=FALSE;					//Cardholder Device CVM

//Pointer
UCHAR		*etp_ptrData=NULLPTR;				//Pointer of Directory Entry

//Candidate List
UCHAR		etp_cddIndex=0;						//Candidate List Index
UCHAR		etp_cddNumber=0;					//Candidate List Number

//Match AID
UCHAR		etp_mchAIDResult=0;					//Match AID Result

//Examine Kernel ID
UCHAR		etp_exmKIDResult[3]={0};			//Examine KID Result

//Transaction Data Buffer
UCHAR		etp_txnData[ETP_TRANSACTION_DATA_SIZE]={0};	//Transaction Data
UINT		etp_txnDataLen=0;					//Length of Transaction Data

//Kernel Configuration Data Buffer
KRNCONFIG	etp_krnConfig[ETP_NUMBER_KERNELCONFIGURATION]={{0,{0},0,0,{0}}};
UCHAR		etp_krnCfgIndex=0;					//Kernel Configuration Index

//Receive Buffer
UCHAR		etp_rcvData[ETP_BUFFER_SIZE]={0};	//Receive Data Buffer
UINT		etp_rcvLen=0;						//Receive Length

//APDU Response
UCHAR		etp_depRspCode=0;					//L1 Response Code
UCHAR		etp_depSW[2]={0};					//SW12

//Polling
UINT		etp_tmrSale=6500;					//Timer of Sale
ULONG		etp_Polling_Timer=0;				//Timer of Polling
UCHAR		etp_Polling_Restart = FALSE;		//Restart of Polling

//Check ADF
UCHAR 		etp_checkADF_Fail=FALSE;


//	VISA
extern UCHAR				api_msg_amount[32];
extern UCHAR				api_msg_amount_len;
extern UCHAR				main_card_remove;
extern UCHAR				SchemeID;
extern UCHAR				L3_Response_Code;
extern UINT					Online_Data_Length;
extern UCHAR				Online_Data[1024];
extern UCHAR				glv_par_WAVE2_Enable;
extern PREDRLLIMITSET		VGL_PREDRLLimitSet[ETP_PARA_NUMBER_PID];

//	JCB
extern UCHAR				JCB_JCB_FCI_Error;
extern UCHAR				JCB_Out_Status;
extern UCHAR				JCB_Txn_Terminate;
extern UCHAR				JCB_Issue_Update_Flag;	
extern JCB_DTP				JCB_Dynamic_Parameter;
extern JCB_Recover_Context	JCB_ReContext_Par;

//	AE
extern UCHAR				aex_flgRestart;

//	MasterCard
//extern UCHAR				mpp_DE_log[MPP_DE_LOG_SIZE];
extern UCHAR				*mpp_DE_log;



#ifndef _PLATFORM_AS350
extern void		VAP_RIF_TRA_ReadyForSales_TryAgain(void);
#endif

extern UCHAR	api_pcd_vap_Check_PayPassConfigurationData(UINT iptLen, UCHAR * iptData);
extern ULONG	OS_GET_SysTimerFreeCnt( void );

void	ETP_C_Step3(void);
UCHAR	ETP_Parse_FCI(UINT iptLen, UCHAR *iptData);
void	ETP_Update_CandidateList(void);
void	ETP_Update_Timer(ULONG iptTime1, ULONG iptTime2, ULONG * tmrValue);


void ETP_MPP_OutcomeProcess(void)
{
	UINT	lenOfV=0;

	//Status
	//Replaced by Entry Point UI Request

	//Start
	switch (glv_tagDF8129.Value[1])
	{
		case MPP_OPS_START_NA:
			etp_Outcome.Start=ETP_OCP_Start_NA;
			break;
			
		case MPP_OPS_START_B:
			etp_Outcome.Start=ETP_OCP_Start_B;
			etp_flgComError=TRUE;
			break;
			
		case MPP_OPS_START_C:
			etp_Outcome.Start=ETP_OCP_Start_C;
			ETP_Remove_CandidateList();
			break;

		default: break;
	}

	//Always set UI Request
	etp_Outcome.rqtOutcome=TRUE;

	//UI Request on Restart Present
	if (glv_tagDF8129.Value[4] & 0x40)
	{
		etp_Outcome.rqtRestart=TRUE;
	}

	//Check Two Tap Processing
	if ((glv_tagDF8129.Value[0] == MPP_OPS_STATUS_EndApplication) &&
		(glv_tagDF8129.Value[1] == MPP_OPS_START_B) &&
		(glv_tagDF8116.Value[0] == MPP_UIR_MID_SeePhone))
	{
		etp_flgCDCVM=TRUE;
		etp_Outcome.rqtOutcome=FALSE;	//Disable Outcome UI
	}

	//UI Request - Message
	switch (glv_tagDF8129.Value[0])
	{
		case MPP_OPS_STATUS_Approved:				etp_Outcome.ocmMsgID=ETP_OCP_UIM_Approved;					break;
		case MPP_OPS_STATUS_Declined:				etp_Outcome.ocmMsgID=ETP_OCP_UIM_NotAuthorised;				break;
		case MPP_OPS_STATUS_OnlineRequest:			etp_Outcome.ocmMsgID=ETP_OCP_UIM_AuthorisingPleaseWait;		break;
		case MPP_OPS_STATUS_SelectNext:				etp_Outcome.ocmMsgID=ETP_OCP_UIM_NA;						break;
		case MPP_OPS_STATUS_TryAnotherInterface:	etp_Outcome.ocmMsgID=ETP_OCP_UIM_PleaseInsertOrSwipeCard;	break;
		case MPP_OPS_STATUS_TryAgain:				etp_Outcome.ocmMsgID=ETP_OCP_UIM_PresentCardAgain;			break;
		case MPP_OPS_STATUS_NA:						etp_Outcome.ocmMsgID=ETP_OCP_UIM_NA;						break;
		case MPP_OPS_STATUS_EndApplication:			etp_Outcome.ocmMsgID=
			(glv_tagDF8129.Value[1] == MPP_OPS_START_NA)?
			(ETP_OCP_UIM_InsertSwipeOrTryAnotherCard):(ETP_OCP_UIM_PresentCardAgain);							break;
			
		default:									etp_Outcome.ocmMsgID=ETP_OCP_UIM_NA;						break;
	}

	//UI Request - Status
	switch (glv_tagDF8129.Value[0])
	{
		case MPP_OPS_STATUS_Approved:
		case MPP_OPS_STATUS_OnlineRequest:			etp_Outcome.ocmStatus=ETP_OCP_UIS_CardReadSuccessfully;	break;
		case MPP_OPS_STATUS_Declined:				etp_Outcome.ocmStatus=ETP_OCP_UIS_ProcessingError;		break;
		case MPP_OPS_STATUS_SelectNext:
		case MPP_OPS_STATUS_TryAnotherInterface:
		case MPP_OPS_STATUS_TryAgain:				etp_Outcome.ocmStatus=ETP_OCP_UIS_ReadyToRead;			break;
		case MPP_OPS_STATUS_EndApplication:			etp_Outcome.ocmStatus=
			(etp_flgCDCVM == TRUE)?(ETP_OCP_UIS_NotReady):(ETP_OCP_UIS_ProcessingError);					break;

		default:									etp_Outcome.ocmStatus=ETP_OCP_UIS_NA;					break;
	}
	
	//Outcome Status Buzzer
	switch (etp_Outcome.ocmStatus)
	{
		case ETP_OCP_UIS_CardReadSuccessfully:	UT_Buz_Option(1);	break;
		case ETP_OCP_UIS_ProcessingError:		UT_Buz_Option(0);	break;
		default:													break;
	}

	//Copy Outcome Status for L3 Response
	if (etp_Outcome.ocmStatus == ETP_OCP_UIS_CardReadSuccessfully)
	{
		VGL_AS210_D1_Track();
		L3_Response_Code=VAP_RIF_RC_DATA;
	}
	else
	{
		L3_Response_Code=VAP_RIF_RC_FAILURE;
	}
	
	//Copy Data to Online_Data for VISA AP Reader Interface
	UT_Get_TLVLengthOfV(glv_tagFF8105.Length, &lenOfV);
	Online_Data_Length=lenOfV;
	memcpy(Online_Data, glv_tagFF8105.Value, lenOfV);
}

void ETP_Load_MPP_ConfigurationData(void)
{
	UINT	cfgLen=0;
	UCHAR	cfgBuffer[FLS_SIZE_CONFIGURATION_DATA-2]={0};	//Length(2)+Data(512)
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfData=0;
	UINT	lenOfTags=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=0;
	UCHAR	flgLodDefault=FALSE;

	rspCode=FLS_Read_PayPassConfigurationData(&cfgLen, cfgBuffer);
	if (rspCode == SUCCESS)
	{
		rspCode=api_pcd_vap_Check_PayPassConfigurationData(cfgLen, cfgBuffer);
		if (rspCode == SUCCESS)
		{
			;	//Do nothing
		}
		else
		{
			flgLodDefault=TRUE;
		}
	}
	else
	{
		flgLodDefault=TRUE;
	}

	if (flgLodDefault == TRUE)
	{
		cfgLen=(UINT)sizeof(glv_parCfgData);
		memcpy(cfgBuffer, glv_parCfgData, cfgLen);
	}

	//AID
	ptrData=cfgBuffer;
	UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
	etp_krnConfig[0].AIDLen=(UCHAR)lenOfV;
	memcpy(etp_krnConfig[0].AID, &ptrData[lenOfT+lenOfL], lenOfV);

	ptrData+=(lenOfT+lenOfL+lenOfV);
	lenOfData+=(lenOfT+lenOfL+lenOfV);

	//Transaction Type
	UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
	etp_krnConfig[0].txnType=ptrData[lenOfT+lenOfL];

	ptrData+=(lenOfT+lenOfL+lenOfV);
	lenOfData+=(lenOfT+lenOfL+lenOfV);

	//Configuration Data
	do {
		UT_Get_TLVLength(ptrData+lenOfTags, &lenOfT, &lenOfL, &lenOfV);

		lenOfData+=(lenOfT+lenOfL+lenOfV);
		lenOfTags+=(lenOfT+lenOfL+lenOfV);
	} while (lenOfData < cfgLen);

	etp_krnConfig[0].cfgLen=lenOfTags;
	memcpy(etp_krnConfig[0].cfgData, ptrData, lenOfTags);
}

void ETP_Get_UnpredictableNumber(void)
{
	UCHAR	rndBuffer[8]={0};	//Random Number Buffer

	api_sys_random(rndBuffer);

	glv_tag9F37.Length[0] = ECL_LENGTH_9F37;
	memcpy(glv_tag9F37.Value, rndBuffer, ECL_LENGTH_9F37);
}

UCHAR ETP_Get_TransactionTypeIndex(void)
{
	if ((glv_tag9C.Value[0] == 0x00) && (glv_parFlgCashBack == TRUE))
	{
		return ETP_CMBTABLE_CASHBACK;	//VISA
	}
	else if (glv_tag9C.Value[0] == 0x01)
	{
		return ETP_CMBTABLE_CASH;
	}
	else if (glv_tag9C.Value[0] == 0x09)
	{
		return ETP_CMBTABLE_CASHBACK;	//MasterCard
	}
	else if (glv_tag9C.Value[0] == 0x20)
	{
		return ETP_CMBTABLE_REFUND;
	}

	return ETP_CMBTABLE_PURCHASE;
}


UCHAR ETP_Get_KernelConfigurationIndex(void)
{
	UCHAR	idxNum=0;

	for (idxNum=0; idxNum < ETP_NUMBER_KERNELCONFIGURATION; idxNum++)
	{
		if (!memcmp(etp_cddList[etp_cddIndex].adfName, etp_krnConfig[idxNum].AID, etp_krnConfig[idxNum].AIDLen))
		{
			if (glv_tag9C.Value[0] == etp_krnConfig[idxNum].txnType)
			{
				return idxNum;
			}
		}
	}
	
	return 0;
}


UCHAR ETP_Add_TransactionData(UCHAR *iptTLV)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	rspCode=FALSE;

	rspCode=UT_Get_TLVLength(iptTLV, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == SUCCESS)
	{
		if (etp_txnDataLen +(lenOfT+lenOfL+lenOfV) <= ETP_TRANSACTION_DATA_SIZE)
		{
			memcpy(&etp_txnData[etp_txnDataLen], iptTLV, (lenOfT+lenOfL+lenOfV));
			etp_txnDataLen+=(lenOfT+lenOfL+lenOfV);

			return SUCCESS;
		}
	}
	
	return FAIL;
}


void ETP_Add_TransactionDataFromParameter(void)
{
	UCHAR	tmpTLV[64]={0};
//	ULONG	rndNumber=0;		//Random Number
//	UCHAR	rndBuffer[8]={0};	//Random Number Buffer

	//5F2A	Transaction Currency Code
	memset(tmpTLV, 0, 64);
	tmpTLV[0]=0x5F;
	tmpTLV[1]=0x2A;
	tmpTLV[2]=ECL_LENGTH_5F2A;
	memcpy(&tmpTLV[3], glv_par5F2A, ECL_LENGTH_5F2A);
	ETP_Add_TransactionData(tmpTLV);

	//9F1A	Terminal Country Code
	memset(tmpTLV, 0, 64);
	tmpTLV[0]=0x9F;
	tmpTLV[1]=0x1A;
	tmpTLV[2]=ECL_LENGTH_9F1A;
	memcpy(&tmpTLV[3], glv_par9F1A, ECL_LENGTH_9F1A);
	ETP_Add_TransactionData(tmpTLV);
}


void ETP_Load_TransactionData(void)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfData=0;
	UCHAR	*ptrData=NULLPTR;
	UINT	recIndex=0;
	UCHAR	rspCode=FALSE;

	//Set Pointer to Transaction Data Buffer
	ptrData=etp_txnData;

	do {
		//Get Length
		rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		if (rspCode == SUCCESS)
		{
			//Save Tag
			rspCode=UT_Search_Record(lenOfT, ptrData, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
			if (rspCode == SUCCESS)
			{
				memcpy((UCHAR*)glv_addTable[recIndex], ptrData+lenOfT, lenOfL);
				memcpy((UCHAR*)glv_addTable[recIndex]+3, ptrData+lenOfT+lenOfL, lenOfV);
			}
		}
		
		//Point to Next Tag
		ptrData+=(lenOfT+lenOfL+lenOfV);
		lenOfData+=(lenOfT+lenOfL+lenOfV);
	} while (lenOfData < etp_txnDataLen);
}

void ETP_Initialize_CombinationTable(void)
{
	UCHAR	krnNA		=0x80;	//Kernel 1
	UCHAR	krnMaster	=0x40;	//Kernel 2
	UCHAR	krnVISA 	=0x20;	//Kernel 3
	UCHAR	krnAMEX 	=0x10;	//Kernel 4
	UCHAR	krnJCB		=0x08;	//Kernel 5
	UCHAR	krnDiscover	=0x04;	//Kernel 6
	UCHAR	krnUPI		=0x02;	//Kernel 7
	UCHAR	typIndex	=0;

	for (typIndex=0; typIndex < ETP_NUMBER_TRANSACTIONTYPE; typIndex++)
	{
		etp_cmbTable[typIndex][0].aidLen=7;
		memcpy(etp_cmbTable[typIndex][0].cmbAID, etp_aidNA, 7);	//VISA AP
		etp_cmbTable[typIndex][0].lstKernel[0]=krnNA;

		etp_cmbTable[typIndex][1].aidLen=7;
		memcpy(etp_cmbTable[typIndex][1].cmbAID, etp_aidMaster, 7);
		etp_cmbTable[typIndex][1].lstKernel[0]=krnMaster;

		etp_cmbTable[typIndex][2].aidLen=7;
		memcpy(etp_cmbTable[typIndex][2].cmbAID, etp_aidVISA, 7);
		etp_cmbTable[typIndex][2].lstKernel[0]=krnVISA;

		etp_cmbTable[typIndex][3].aidLen=6;
		memcpy(etp_cmbTable[typIndex][3].cmbAID, etp_aidAMEX, 6);
		etp_cmbTable[typIndex][3].lstKernel[0]=krnAMEX;

		etp_cmbTable[typIndex][4].aidLen=7;
		memcpy(etp_cmbTable[typIndex][4].cmbAID, etp_aidJCB, 7);
		etp_cmbTable[typIndex][4].lstKernel[0]=krnJCB;

		
		memset(&etp_cmbTable[typIndex][5], 0, CMBTABLE_LEN);
		memset(&etp_cmbTable[typIndex][6], 0, CMBTABLE_LEN);
		memset(&etp_cmbTable[typIndex][7], 0, CMBTABLE_LEN);
		memset(&etp_cmbTable[typIndex][8], 0, CMBTABLE_LEN);
		memset(&etp_cmbTable[typIndex][9], 0, CMBTABLE_LEN);
		
		if (etp_flgMtiAID_Discover == TRUE)
		{
			etp_cmbTable[typIndex][5].aidLen=glv_par9F06Len[5];
			memcpy(etp_cmbTable[typIndex][5].cmbAID, glv_par9F06[5], glv_par9F06Len[5]);
			etp_cmbTable[typIndex][5].lstKernel[0]=krnDiscover;
			
			etp_cmbTable[typIndex][9].aidLen=glv_par9F06Len[9];
			memcpy(etp_cmbTable[typIndex][9].cmbAID, glv_par9F06[9], glv_par9F06Len[9]);
			etp_cmbTable[typIndex][9].lstKernel[0]=krnDiscover;
		}
		else
		{
			etp_cmbTable[typIndex][5].aidLen=7;
			memcpy(etp_cmbTable[typIndex][5].cmbAID, etp_aidDPS, 7);
			etp_cmbTable[typIndex][5].lstKernel[0]=krnDiscover;
		}

		if (etp_flgMtiAID_UPI == TRUE)
		{
			etp_cmbTable[typIndex][6].aidLen=glv_par9F06Len[6];
			memcpy(etp_cmbTable[typIndex][6].cmbAID, glv_par9F06[6], glv_par9F06Len[6]);
			etp_cmbTable[typIndex][6].lstKernel[0]=krnUPI;

			etp_cmbTable[typIndex][7].aidLen=glv_par9F06Len[7];
			memcpy(etp_cmbTable[typIndex][7].cmbAID, glv_par9F06[7], glv_par9F06Len[7]);
			etp_cmbTable[typIndex][7].lstKernel[0]=krnUPI;

			etp_cmbTable[typIndex][8].aidLen=glv_par9F06Len[8];
			memcpy(etp_cmbTable[typIndex][8].cmbAID, glv_par9F06[8], glv_par9F06Len[8]);
			etp_cmbTable[typIndex][8].lstKernel[0]=krnUPI;
		}
		else
		{
			etp_cmbTable[typIndex][6].aidLen=7;
			memcpy(etp_cmbTable[typIndex][6].cmbAID, etp_aidUPI, 7);
			etp_cmbTable[typIndex][6].lstKernel[0]=krnUPI;
		}
	}
}


void ETP_Initialize_EntryPointConfiguration(void)
{
	UCHAR	cntIndex=0;
	ULONG	int9F1B=0;
	UCHAR	asc9F1B[10]={0};
	UCHAR	bcd9F1B[5]={0};
	
	/*	
	DF06
	B1b8: Status Check enabled/disabled
	B1b7: Amount, Authorized of Zero Check enabled/disabled
	B1b6: Amount, Authorized of Zero Option (1b=Option 1 and 0b=Option 2)
	B1b5: Reader Contactless Transaction Limit Check enabled/disabled
	B1b4: Reader CVM Required Limit Check enabled/disabled
	B1b3: Reader Contactless Floor Limit Check enabled/disabled
	B1b2: Exception file enabled/disabled
	B1b1: DRL enabled/disabled
	B2b8: Terminal Floor Limit Check enabled/disabled	
	*/
	
	for (cntIndex=0; cntIndex < ETP_NUMBER_COMBINATION; cntIndex++)
	{
		//Status Check
		if (glv_parDF06[cntIndex][0] & 0x80)
		{
			etp_etpConfig[cntIndex].pstStaCheck=TRUE;
			etp_etpConfig[cntIndex].flgStaCheck=TRUE;
		}
		else
		{
			etp_etpConfig[cntIndex].pstStaCheck=FALSE;
			etp_etpConfig[cntIndex].flgStaCheck=FALSE;
		}

		//Zero Check
		if (glv_parDF06[cntIndex][0] & 0x40)
		{
			etp_etpConfig[cntIndex].pstZroAllowed=TRUE;
			etp_etpConfig[cntIndex].flgZroAllowed=FALSE;
		}
		else
		{
			etp_etpConfig[cntIndex].pstZroAllowed=FALSE;
			etp_etpConfig[cntIndex].flgZroAllowed=TRUE;
		}

		etp_etpConfig[cntIndex].pstExtSelection=TRUE;
		etp_etpConfig[cntIndex].flgExtSelection=glv_parExtSelSupport[cntIndex];

		//Transaction Limit Check
		if (glv_parDF06[cntIndex][0] & 0x10)
		{
			etp_etpConfig[cntIndex].pstRdrCTL=TRUE;
			memcpy(etp_etpConfig[cntIndex].rdrCTL, glv_parDF00[cntIndex], ETP_PARA_SIZE_DF00);
		}
		else
		{
			etp_etpConfig[cntIndex].pstRdrCTL=FALSE;
			memset(etp_etpConfig[cntIndex].rdrCTL, 0, ETP_PARA_SIZE_DF00);
		}

		//Floor Limit Check
		if ((glv_parDF06[cntIndex][0] & 0x04) && (glv_parDF02Len[cntIndex] != 0))
		{
			etp_etpConfig[cntIndex].pstRdrCFL=TRUE;
			memcpy(etp_etpConfig[cntIndex].rdrCFL, glv_parDF02[cntIndex], ETP_PARA_SIZE_DF02);
		}
		else
		{
			etp_etpConfig[cntIndex].pstRdrCFL=FALSE;
			memset(etp_etpConfig[cntIndex].rdrCFL, 0, ETP_PARA_SIZE_DF02);
		}
		
		if (glv_par9F1B_Len == ECL_LENGTH_9F1B)
		{
			etp_etpConfig[cntIndex].pstTrmFL=TRUE;
			int9F1B=glv_par9F1B[cntIndex][0]*(1<<24)+glv_par9F1B[cntIndex][1]*(1<<16)+glv_par9F1B[cntIndex][2]*(1<<8)+glv_par9F1B[cntIndex][3];
			UT_INT2ASC(int9F1B, asc9F1B);
			UT_Compress(bcd9F1B, asc9F1B, 5);
			memcpy(&etp_etpConfig[cntIndex].trmFL[1], bcd9F1B, 5);
		}
		else
		{
			etp_etpConfig[cntIndex].pstTrmFL=FALSE;
			memset(etp_etpConfig[cntIndex].trmFL, 0, 6);
		}

		//CVM Required Limit Check
		if(glv_parDF06[cntIndex][0] & 0x08)
		{
			etp_etpConfig[cntIndex].pstRdrCRL=TRUE;
			memcpy(etp_etpConfig[cntIndex].rdrCRL, glv_parDF01[cntIndex], ETP_PARA_SIZE_DF01);
		}
		else
		{
			etp_etpConfig[cntIndex].pstRdrCRL=FALSE;
			memset(etp_etpConfig[cntIndex].rdrCRL, 0, ETP_PARA_SIZE_DF01);
		}

		etp_etpConfig[cntIndex].pstTTQ=TRUE;
		memcpy(etp_etpConfig[cntIndex].TTQ, glv_par9F66[cntIndex], ETP_PARA_SIZE_9F66);
	}

	//VISA TTQ B1b6: qVSDC
	if (etp_etpConfig[2].TTQ[0] & 0x20)
	{
		glv_parFlgqVSDC=TRUE;
	}
	else
	{
		glv_parFlgqVSDC=FALSE;
	}
}


void ETP_Initialize(void)
{
	ETP_Initialize_CombinationTable();
	ETP_Load_MPP_ConfigurationData();

	ETP_Initialize_EntryPointConfiguration();
}


void ETP_Clear_Buffer(void)
{
	etp_rcvLen=0;
	memset(etp_rcvData, 0, ETP_BUFFER_SIZE);
}


void ETP_Clear_DirectoryEntryTag(void)
{
	memset(glv_tag4F.Length,	0, 3+ECL_LENGTH_4F);
	memset(glv_tag50.Length,	0, 3+ECL_LENGTH_50);
	memset(glv_tag87.Length,	0, 3+ECL_LENGTH_87);
	memset(glv_tag9F29.Length,	0, 3+ECL_LENGTH_9F29);
	memset(glv_tag9F2A.Length,	0, 3+ECL_LENGTH_9F2A);
}


void ETP_Clear_CandidateList(void)
{
	UCHAR idxNum=0;
	
	etp_cddIndex=0;
	etp_cddNumber=0;
	
	for (idxNum=0; idxNum<ETP_NUMBER_CANDIDATELIST; idxNum++)
		memset(&etp_cddList[idxNum], 0, CDDLIST_LEN);
}


void ETP_Clear_TransactionData(void)
{
	memset(etp_txnData, 0, ETP_TRANSACTION_DATA_SIZE);
	etp_txnDataLen=0;
}


void ETP_Reset_Parameter(void)
{
//	UCHAR	idxNum=0;

//Test
/*	//Pre-Processing Indicator
	for (idxNum=0; idxNum < ETP_NUMBER_COMBINATION; idxNum++)
	{
		memset((UCHAR*)&etp_preIndicator[idxNum], 0, INDICATOR_LEN);
	}

	//Candidate List
	ETP_Clear_CandidateList();
*/	
	//Outcome
	//0514 ICTK
	memset((UCHAR*)&etp_Outcome, 0, OUTCOME_LEN);


	//Pointer of Directory Entry
	etp_ptrData=NULLPTR;	

	//Match AID Result
	etp_mchAIDResult=0;		

	//Examine KID Result
	memset(etp_exmKIDResult, 0, 3);

	//Issuer Update Error Flag
	etp_flgIsuUpdateErr=FALSE;

	//Unkonw AID Flag
	etp_flgUnknowAID=FALSE;

	etp_flgCDCVM=FALSE;

	//Receive Buffer
	ETP_Clear_Buffer();

//Test			
	etp_depRspCode=0;
	memset(etp_depSW, 0, 2);
}


void ETP_Add_CandidateList(CDDLIST * iptData)
{
	if (etp_cddNumber < ETP_NUMBER_CANDIDATELIST)
	{
		memcpy(&etp_cddList[etp_cddIndex], iptData, CDDLIST_LEN);

		etp_cddIndex++;
		etp_cddNumber++;
	}
}


void ETP_Sort_CandidateList(void)
{
	CDDLIST	tmpCDDList[ETP_NUMBER_CANDIDATELIST]={{0,{0},0,{0},0,0,{0},0}};
	UCHAR	cddIndex=0;		//Candidate List Index
	UCHAR	srtIndex=0;		//Sort Index
	UCHAR	priIndex=0;		//Priority Index

	//Copy High Priority Candidate List to Temporary Candidate List
	for (priIndex=1; priIndex <= 15; priIndex++)//Application Priority Indicator is ranging from 1 to 15
	{
		for (cddIndex=0; cddIndex<etp_cddNumber; cddIndex++)
		{
			if (priIndex == etp_cddList[cddIndex].appPriority)
			{
				memcpy(&tmpCDDList[srtIndex], &etp_cddList[cddIndex], CDDLIST_LEN);
				srtIndex++;
			}
		}
	}

	//Copy No Priority Candidate List to Temporary Candidate List
	for (cddIndex=0; cddIndex < etp_cddNumber; cddIndex++)
	{
		if (etp_cddList[cddIndex].appPriority == 0)
		{
			memcpy(&tmpCDDList[srtIndex], &etp_cddList[cddIndex], CDDLIST_LEN);
			srtIndex++;
		}
	}

	//Copy Sorted Temporary Candidate List To Candidate List 
	for (cddIndex=0; cddIndex < etp_cddNumber; cddIndex++)
	{
		memcpy(&etp_cddList[cddIndex], &tmpCDDList[cddIndex], CDDLIST_LEN);
	}
}


void ETP_Remove_CandidateList(void)
{
	CDDLIST	tmpCDDList[ETP_NUMBER_CANDIDATELIST]={{0,{0},0,{0},0,0,{0},0}};
	UCHAR	cddIndex=0;		//Candidate List Index

	if (etp_cddNumber != 0)
	{
		//Copy Candidate List to Temporary Buffer except First Item
		for (cddIndex=1; cddIndex < etp_cddNumber; cddIndex++)
		{
			memcpy(&tmpCDDList[cddIndex-1], &etp_cddList[cddIndex], CDDLIST_LEN);
		}

		//Reset
		for (cddIndex=0; cddIndex < etp_cddNumber; cddIndex++)
		{
			memset(&etp_cddList[cddIndex], 0, CDDLIST_LEN);
		}
		
		//Copy Back to Candidate List
		for (cddIndex=0; cddIndex < (etp_cddNumber-1); cddIndex++)
		{
			memcpy(&etp_cddList[cddIndex], &tmpCDDList[cddIndex], CDDLIST_LEN);
		}

		//Decrease Number
		etp_cddNumber--;
	}
}


void ETP_Copy_TTQ(void)
{
	UCHAR	cmbIndex=0xFF;

	cmbIndex=etp_cddList[etp_cddIndex].cmbIndex;

	//Copy to Tag
	UT_Set_TagLength(ECL_LENGTH_9F66, glv_tag9F66.Length);
	memcpy(glv_tag9F66.Value, etp_preIndicator[cmbIndex].cpyTTQ, ECL_LENGTH_9F66);
}

void ETP_Copy_DFName(UCHAR * ptrTag84)
{
	UCHAR	rspCode=0;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;

	rspCode=UT_Get_TLVLength(ptrTag84, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == SUCCESS)
	{
		UT_Set_TagLength(lenOfV, glv_tag84.Length);
		memcpy(glv_tag84.Value, &ptrTag84[lenOfT+lenOfL], lenOfV);
	}
}

void ETP_Copy_PDOL(UCHAR * ptrTag9F38)
{
	UCHAR	rspCode=0;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;

	rspCode=UT_Get_TLVLength(ptrTag9F38, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == SUCCESS)
	{
		UT_Set_TagLength(lenOfV, glv_tag9F38.Length);
		memcpy(glv_tag9F38.Value, &ptrTag9F38[lenOfT+lenOfL], lenOfV);
	}
}


void ETP_Display_CountdownSecond(UCHAR *iptPrevious, ULONG iptTime)
{
	UCHAR 	secNow;
	UCHAR	secDisplay[3]={0};
	UCHAR	dspRow=0;
	UCHAR	dspColumn=0;
	UCHAR	dspFont=5;
	UCHAR	dspFormat=0;

	secNow=(UCHAR)(iptTime/100);	//Seconds


#ifdef _SCREEN_SIZE_240x320
	#ifdef _PLATFORM_AS350
		#ifdef _ACQUIRER_NCCC
			dspFormat=2;
		#else
			dspFormat=1;
		#endif
	#else
		dspFormat=1;
	#endif
#else
	#ifdef _SCREEN_SIZE_320x240
		dspFormat=3;
	#else
		dspFormat=4;
	#endif
#endif


	switch (dspFont)
	{
		case 1:	dspRow	= 6;	dspColumn	= 11;	dspFont	= FONT0;	break;
		case 2: dspRow	= 8;	dspColumn	= 15;	dspFont	= FONT1;	break;
		case 3: dspRow	= 2;	dspColumn	= 10;	dspFont	= FONT1;	break;
		case 4: dspRow	= 3;	dspColumn	=  2;	dspFont	= FONT1;	break;
		case 5: dspRow	= 6;	dspColumn	=  12;	dspFont	= FONT1;	break;
		default:														break;
	}

	if (dspFormat != 2)
	{
		if ((secNow != 0) && (iptPrevious[0] == 0))
		{
			UT_PutStr(dspRow, (dspColumn-3), dspFont, 13, (UCHAR*)"In    Seconds");
		}
	}

	if (secNow != iptPrevious[0])
	{
		UT_1hexto3asc(secNow, secDisplay);
		UT_PutStr(dspRow, dspColumn, dspFont, 2, &secDisplay[1]);
		
		iptPrevious[0]=secNow;
	}
}

extern char rotation;
void ETP_UI_Request(UCHAR msgID, UCHAR rqtStatus)
{
 
/*	UCHAR	dspStatus[22]={0};		//Display Status
	UCHAR	dspMessage0[21]={0};	//Display Message 0
	UCHAR	dspMessage1[21]={0};	//Display Message 1
	UINT	dspOffset = 0;
	UCHAR	dspStyle = FONT2;
	UINT	dspLen = 15;
*/
#ifdef _SCREEN_SIZE_240x320
	//20131220 increase error message display time
	UCHAR 	delay_flag = FALSE;

	UCHAR	dsp_line_row = 1;
	UCHAR	dsp_same_line_wait = 0;
#else
	UCHAR	dsp_line_row = 0;
#endif	

	UCHAR	dspStatus[64]={0};		//Display Status
	UINT	dspLen = 15;
	UCHAR	dspLine0 = FALSE;

	//Display Message ID
	UCHAR	dspMessage0[64] = {0};
	UCHAR	dsp_Msg_Len0 = 0;

	//display chinese message
	UCHAR 	dsp_C_Msg_Len = 0;
	UCHAR 	dsp_C_Message[32] = {0};

	UCHAR	dspMessage1[64]={0};	//Display Message 1
	UCHAR	dspMessage2[64]={0};	//Display Message 2	
	UCHAR 	dsp_Msg_Len1 = 0;
	UCHAR 	dsp_Msg_Len2 = 0;

	UCHAR 	Message1[32] = {0x00};
	UCHAR 	Message2[32] = {0x00};
	UCHAR 	Message3[32] = {0x00};
	UCHAR 	Message4[32] = {0x00};
	
	UCHAR 	Msg_Len1 = 0;
	UCHAR 	Msg_Len2 = 0;
	UCHAR 	Msg_Len3 = 0;
	UCHAR 	Msg_Len4 = 0;

	UCHAR 	Disp_Direct = FALSE;
	UCHAR	msgTryAgain[10]={"½Ð­«·s·PÀ³"};

	memset(dspMessage0, 0, 64);
	memset(dspStatus, 0, 64);
	
	switch (rqtStatus)
	{
		/*case 0x00:	memcpy(dspStatus, (UCHAR*)"Card Read Successfully",	22); break;	//+y
		case 0x01:	memcpy(dspStatus, (UCHAR*)"Processing Error      ",	22); break;
		case 0x02:	memcpy(dspStatus, (UCHAR*)"Ready to Read         ",	22); break;
		case 0x03:	memcpy(dspStatus, (UCHAR*)"Not Ready             ",	22); break;
		case 0x04:	memcpy(dspStatus, (UCHAR*)"Processing            ",	22); break;
		case 0x05:	memcpy(dspStatus, (UCHAR*)"                      ",	22); break;*/


		case 0x00:	memcpy(dspStatus,glv_vap_Message[18].Message,glv_vap_Message[18].Length); 
					dspLen = glv_vap_Message[18].Length;
					break;
					
		case 0x01:	memcpy(dspStatus, (UCHAR*)"·PÀ³¥¢±Ñ",	8);
					dspLen = 8;
					//dspLine0 = TRUE;
					break;

		case 0x02:	memcpy(dspStatus,glv_vap_Message[17].Message,glv_vap_Message[17].Length); 
					dspLen = glv_vap_Message[17].Length;
					//dspLine0 = TRUE;
					break;
		
		//case 0x03:	memcpy(dspStatus, (UCHAR*)"Not Ready             ",	22); break;

		case 0x04:	memcpy(dspStatus,glv_vap_Message[19].Message,glv_vap_Message[19].Length); 
					dspLen = glv_vap_Message[19].Length;
					break;

		case 0x05:	memcpy(dspStatus, (UCHAR*)"                      ",	22); break;

		default:	break;
	}

 
#ifdef _SCREEN_SIZE_240x320
	//20140127 100y debug add msd 0xFF
	//20131226 add (msdID == 0x07) (Decline), for the DDA fail problem, we don't delay in kernel, we delay message display with L3 Show Status command 
	//20131220 increase error message display time
	//if((msgID == 0x03)||(msgID == 0x07)||(msgID == 0x15)||(msgID == 0x16)||(msgID == 0x17)||(msgID == 0x1B))
	if((msgID == 0x03)||(msgID == 0x07)||(msgID == 0x15)||(msgID == 0x16)||(msgID == 0x17)||(msgID == 0x1A)||(msgID == 0x1B)||(msgID == 0xFF))
		delay_flag = FALSE;
	else
		delay_flag = TRUE;
#endif
printf("msgID=0x%x\n",msgID);
	switch (msgID)
	{
/*		case 0x03:	memcpy(dspMessage0, (UCHAR*)"Approved             ",	21); 
					memcpy(dspMessage1, (UCHAR*)"                     ",	21); break;
		case 0x07:	memcpy(dspMessage0, (UCHAR*)"Decline              ",	21); 
					memcpy(dspMessage1, (UCHAR*)"                     ",	21); break;
		case 0x09:	memcpy(dspMessage0, (UCHAR*)"Please enter your PIN",	21); 
					memcpy(dspMessage1, (UCHAR*)"                     ",	21); break;
		case 0x0F:	memcpy(dspMessage0, (UCHAR*)"Processing error     ",	21); 
					memcpy(dspMessage1, (UCHAR*)"                     ",	21); break;
		case 0x10:	memcpy(dspMessage0, (UCHAR*)"Please remove card   ",	21); 
					memcpy(dspMessage1, (UCHAR*)"                     ",	21); break;
		case 0x14:	memcpy(dspMessage0, (UCHAR*)"Welcome              ",	21); 
					memcpy(dspMessage1, (UCHAR*)"                     ",	21); break;
		case 0x15:	memcpy(dspMessage0, (UCHAR*)"Present Card         ",	21); 
					memcpy(dspMessage1, (UCHAR*)"                     ",	21); break;
		case 0x16:	memcpy(dspMessage0, (UCHAR*)"Processing           ",	21); 
					memcpy(dspMessage1, (UCHAR*)"                     ",	21); break;
		case 0x17:	memcpy(dspMessage0, (UCHAR*)"Card read OK Please  ",	21); 
					memcpy(dspMessage1, (UCHAR*)"remove card          ",	21); break;
		case 0x18:	memcpy(dspMessage0, (UCHAR*)"Please insert or     ",	21); 
					memcpy(dspMessage1, (UCHAR*)"swipe card           ",	21); break;
		case 0x19:	memcpy(dspMessage0, (UCHAR*)"Please present one   ",	21); 
					memcpy(dspMessage1, (UCHAR*)"card only            ",	21); break;
		case 0x1A:	memcpy(dspMessage0, (UCHAR*)"Approved Please sign ",	21); 
					memcpy(dspMessage1, (UCHAR*)"                     ",	21); break;
		case 0x1B:	memcpy(dspMessage0, (UCHAR*)"Authorising Please   ",	21); 
					memcpy(dspMessage1, (UCHAR*)"Wait                 ",	21); break;
		case 0x1C:	memcpy(dspMessage0, (UCHAR*)"Insert, Swipe or Try ",	21); 
					memcpy(dspMessage1, (UCHAR*)"Another Card         ",	21); break;
		case 0x1D:	memcpy(dspMessage0, (UCHAR*)"Please insert card   ",	21); 
					memcpy(dspMessage1, (UCHAR*)"                     ",	21); break;
		case 0x20:	memcpy(dspMessage0, (UCHAR*)"See Phone for        ",	21); 
					memcpy(dspMessage1, (UCHAR*)"Instructions         ",	21); break;
		case 0x21:	memcpy(dspMessage0, (UCHAR*)"Present Card Again   ",	21); 
					memcpy(dspMessage1, (UCHAR*)"                     ",	21); break;
*/					

		case 0x03:	

			dsp_Msg_Len0 = glv_vap_Message[18].Length;
			memcpy(dspMessage0,glv_vap_Message[18].Message,dsp_Msg_Len0);

			break;
		
		case 0x07:	//memcpy(dspMessage0, (UCHAR*)"       Decline       ",	21); 

			dsp_Msg_Len0 = glv_vap_Message[18].Length;
			memcpy(dspMessage0,glv_vap_Message[18].Message,dsp_Msg_Len0);
		
			break;
		
		case 0x09:	//memcpy(dspMessage0, (UCHAR*)"Please enter your PIN",	21); 

			dsp_Msg_Len0 = glv_vap_Message[12].Length;
			memcpy(dspMessage0,glv_vap_Message[12].Message,dsp_Msg_Len0);
			
			break;
		
		case 0x0F:	//memcpy(dspMessage0, (UCHAR*)"  Processing error   ",	21); 

			dsp_Msg_Len0 = glv_vap_Message[8].Length;
			memcpy(dspMessage0,glv_vap_Message[8].Message,dsp_Msg_Len0);

			break;
		
		case 0x10:	//memcpy(dspMessage0, (UCHAR*)" Please remove card  ",	21); 

			dsp_Msg_Len0 = glv_vap_Message[18].Length;
			memcpy(dspMessage0,glv_vap_Message[18].Message,dsp_Msg_Len0);
			
			break;

		case 0x14:	//memcpy(dspMessage0, (UCHAR*)"       Welcome       ",	21); 

			dsp_Msg_Len0 = glv_vap_Message[0].Length;
			memcpy(dspMessage0,glv_vap_Message[0].Message,dsp_Msg_Len0);

			break;

		case 0x15:	//memcpy(dspMessage0, (UCHAR*)"    Present Card     ",	21); 
			
			dsp_Msg_Len0 = glv_vap_Message[17].Length;
			memcpy(dspMessage0,glv_vap_Message[17].Message,dsp_Msg_Len0);
		// 	printf("dspMessage0:\n");
		// for(int g=0;g<dsp_Msg_Len0;g++)
		// 	printf("%c ",dspMessage0[g]);
		// printf("\n");
		UT_LED_SingleAct(IID_TAP_LOGO,TRUE);
		// UT_Set_LED(IID_LED_BLUE);
		UT_Set_LEDSignal(IID_LED_BLUE, 50, 50);
#ifdef _SCREEN_SIZE_128x64
			//AS350 display start from row 1
			dspLine0 = TRUE;
#endif

			break;

		case 0x16:	//memcpy(dspMessage0, (UCHAR*)"     Processing      ",	21); 
			
			dsp_Msg_Len0 = glv_vap_Message[19].Length;
			memcpy(dspMessage0,glv_vap_Message[19].Message,dsp_Msg_Len0);

			break;

		case 0x17:	//memcpy(dspMessage0, (UCHAR*)" Card read OK Please ",	21); 
					//memcpy(dspMessage1, (UCHAR*)"     Remove card     ",	21); 

			if ((etp_cddList[etp_cddIndex].krnID[0] != ETP_KID_MASTER) &&
				(etp_cddList[etp_cddIndex].krnID[0] != ETP_KID_AMEX))
			{
				UT_Buz_Option(TRUE);
				UT_Set_LED(IID_LED_GREEN);
// #ifdef _PLATFORM_AS350_LITE
// 				UT_Set_LED(IID_LED_GREEN);
// #else
// 				UT_LED_F_Off_S_On(IID_LED_YELLOW,IID_LED_GREEN,TRUE);
// #endif
			}
			
			dsp_Msg_Len0 = glv_vap_Message[18].Length;
			memcpy(dspMessage0,glv_vap_Message[18].Message,dsp_Msg_Len0);					
			
			break;

		case 0x18:	//memcpy(dspMessage0, (UCHAR*)"   Please insert or  ",	21); 
					//memcpy(dspMessage1, (UCHAR*)"      swipe card     ",	21); 

			dsp_Msg_Len0 = glv_vap_Message[5].Length;
			memcpy(dspMessage0,glv_vap_Message[5].Message,dsp_Msg_Len0);				

			break;

		case 0x19:	//memcpy(dspMessage0, (UCHAR*)" ï¿½hï¿½dï¿½Ð¿ï¿½Ü¤@ï¿½i",15); 
					//memcpy(dspMessage1, (UCHAR*)" ï¿½Dï¿½ï¿½Ä²ï¿½ï¿½ï¿½Hï¿½Î¥d",15);		
			UT_ClearScreen();
			dsp_Msg_Len0 = glv_vap_Message[6].Length;
			memcpy(dspMessage0,glv_vap_Message[6].Message,dsp_Msg_Len0);												
					
			break;

					
		case 0x1A:	//memcpy(dspMessage0, (UCHAR*)"Approved Please sign",	20); 

			dsp_Msg_Len0 = glv_vap_Message[11].Length;
			memcpy(dspMessage0,glv_vap_Message[11].Message,dsp_Msg_Len0);

			break;
		
		case 0x1B:	//memcpy(dspMessage0, (UCHAR*)"Authorising Please wait",23); 	

			dsp_Msg_Len0 = glv_vap_Message[18].Length;
			memcpy(dspMessage0,glv_vap_Message[18].Message,dsp_Msg_Len0);

			break;

		case 0x1C:	//memcpy(dsp_C_Message, (UCHAR*)"ï¿½Pï¿½ï¿½ï¿½ï¿½ï¿½ï¿½",	8);
					dsp_C_Msg_Len=10;
					memcpy(dsp_C_Message, msgTryAgain, dsp_C_Msg_Len);
					
					memcpy(dspMessage0, (UCHAR*)"Insert, Swipe or",	16); 
					dsp_Msg_Len0 = 16;
					memcpy(dspMessage1, (UCHAR*)"Try Another Card",	16); 
					dsp_Msg_Len1 = 16;

					Disp_Direct = TRUE;
			break;
			
		case 0x1D:	//memcpy(dsp_C_Message, (UCHAR*)"ï¿½Pï¿½ï¿½ï¿½ï¿½ï¿½ï¿½",	8); 
					dsp_C_Msg_Len=10;
					memcpy(dsp_C_Message, msgTryAgain, dsp_C_Msg_Len);
					
					memcpy(dspMessage0, (UCHAR*)"Please Insert",13); 
					dsp_Msg_Len0 = 13;					
					memcpy(dspMessage1, (UCHAR*)"Card",	4); 
					dsp_Msg_Len1 = 4;

					Disp_Direct = TRUE;		
			break;
			
		case 0x20:	//memcpy(dsp_C_Message, (UCHAR*)"ï¿½Pï¿½ï¿½ï¿½ï¿½ï¿½ï¿½",	8); 
					dsp_C_Msg_Len=10;
					memcpy(dsp_C_Message, msgTryAgain, dsp_C_Msg_Len);
					
					memcpy(dspMessage0, (UCHAR*)"See Phone for", 13); 
					dsp_Msg_Len0 = 13;
					memcpy(dspMessage1, (UCHAR*)"Instructions",	12); 
					dsp_Msg_Len1 = 12;

					Disp_Direct = TRUE;
					
			break;
			
		case 0x21:	
					dsp_C_Msg_Len=10;
					memcpy(dsp_C_Message, msgTryAgain, dsp_C_Msg_Len);

					memcpy(dspMessage0, (UCHAR*)"Present Card",	12); 
					dsp_Msg_Len0 = 12;
					memcpy(dspMessage1, (UCHAR*)"Again",5); 
					dsp_Msg_Len1 = 5;

					Disp_Direct = TRUE;		
			break;

		case 0xF0:	//Terminate
					dsp_Msg_Len0 = glv_vap_Message[31].Length;
					memcpy(dspMessage0,glv_vap_Message[31].Message,dsp_Msg_Len0);

			break;

		default:	break;
	}
	// Disp_Direct=0;
// printf("Disp_Direct=%d\n",Disp_Direct);

// printf("dsp_Msg_Len1=%d\n",dsp_Msg_Len1);
// printf("dsp_Msg_Len2=%d\n",dsp_Msg_Len2);

// for(int g=0;g<dsp_Msg_Len0;g++)
// 	if(dspMessage0[g]<0x80)
// 		printf("dspMessage0[%d]=%c\n",g,dspMessage0[g]);
// 	else
// 		printf("dspMessage0[%d]=0x%x\n",g,dspMessage0[g]);
// for(int g=0;g<dsp_C_Msg_Len;g++)
// 	printf("dsp_C_Message[%d]=0x%x\n",g,dsp_C_Message[g]);
	if(Disp_Direct)
	{
	 
#ifdef _SCREEN_SIZE_240x320
		dsp_line_row = 1;
		UT_ClearRow(1,4,FONT2); 
#else
		dsp_line_row = 0;
		UT_ClearRow(0,4,FONT1);
#endif
 
		UT_Handle_2Type_Message(dsp_C_Message,(UINT)dsp_C_Msg_Len,Message1,&Msg_Len1,Message2,&Msg_Len2);
 
#ifdef _SCREEN_SIZE_240x320
		if(Msg_Len1 && Msg_Len2)
			dsp_same_line_wait=1;
		else
			dsp_same_line_wait=0;
#else
		if(Msg_Len1 && Msg_Len2)
			dsp_line_row+=0;
		else
			dsp_line_row+=1;
#endif
// printf("Msg_Len1=%d\n",Msg_Len1);
// printf("Msg_Len2=%d\n",Msg_Len2);
// printf("Msg_Len3=%d\n",Msg_Len3);
// printf("Msg_Len4=%d\n",Msg_Len4);
printf("dsp_line_row=%d\n",dsp_line_row);
		if(Msg_Len1)
		{
			UT_Disp_Show_Status(Msg_Len1,Message1,&dsp_line_row);
			
#ifdef _SCREEN_SIZE_240x320
			if(dsp_same_line_wait)
				UT_WaitTime(70);
#endif
		}
		
		if(Msg_Len2)
		{
			UT_Disp_Show_Status(Msg_Len2,Message2,&dsp_line_row);
		}
		// UT_PutStr(line[0]*FONT2_H,(Display_MAX_Num-disp_len)/2*FONT2_W-12,FONT2,disp_len,disp_str);
#ifdef _SCREEN_SIZE_240x320
		UT_PutStr(2*FONT2_H,((Display_MAX_Num - dsp_Msg_Len0)/2)*FONT12_W,FONT2,dsp_Msg_Len0,dspMessage0);
		UT_PutStr(3*FONT2_H,((Display_MAX_Num - dsp_Msg_Len1)/2)*FONT12_W,FONT2,dsp_Msg_Len1,dspMessage1);
#else
		//20140114 V4, Changed for Issuer Update, Present card again
		UT_PutStr(dsp_line_row++,((Display_MAX_Num - dsp_Msg_Len0)/2),FONT1,dsp_Msg_Len0,dspMessage0);
		UT_PutStr(dsp_line_row,((Display_MAX_Num - dsp_Msg_Len1)/2),FONT1,dsp_Msg_Len1,dspMessage1);
		//20140114 V4, Changed for Issuer Update, Present card again, end
#endif		
	}
	else
	{
		UT_ClearRow(2,13,FONT0);
		UT_Handle_2Type_Message(dspMessage0,(UINT)dsp_Msg_Len0,dspMessage1,&dsp_Msg_Len1,dspMessage2,&dsp_Msg_Len2);
		if(dsp_Msg_Len1)		//First Message			
			UT_Handle_2Line_Message(dspMessage1,(UINT)dsp_Msg_Len1,Message1,&Msg_Len1,Message2,&Msg_Len2);
		if(dsp_Msg_Len2)		//Second Message
			UT_Handle_2Line_Message(dspMessage2,(UINT)dsp_Msg_Len2,Message3,&Msg_Len3,Message4,&Msg_Len4);
 
		dsp_line_row=(dspLine0)?0:1;
		// UT_ClearRow(dsp_line_row, 4, FONT1);
		printf("Msg_Len1=%d\n",Msg_Len1);
		printf("Msg_Len2=%d\n",Msg_Len2);
		printf("Msg_Len3=%d\n",Msg_Len3);
		printf("Msg_Len4=%d\n",Msg_Len4);
		// printf("Display_MAX_Num=%d\n",Display_MAX_Num);
		// for(int g=0;g<Msg_Len3;g++)
		// 	printf("Message3[%d]=%c\n",g,Message3[g]);
		if(Msg_Len1)
		{
			UT_Disp_Show_Status(Msg_Len1,Message1,&dsp_line_row);
		}
		if(Msg_Len2)
		{
			UT_Disp_Show_Status(Msg_Len2,Message2,&dsp_line_row);
		}
		if(Msg_Len3)
		{
			UT_Disp_Show_Status(Msg_Len3,Message3,&dsp_line_row);
		}
		if(Msg_Len4)
		{
			UT_Disp_Show_Status(Msg_Len4,Message4,&dsp_line_row);
		}
	}
 
	if ((etp_cddList[etp_cddIndex].krnID[0] == ETP_KID_MASTER) ||
		(etp_cddList[etp_cddIndex].krnID[0] == ETP_KID_AMEX))
	{
		switch (rqtStatus)
		{
			case ETP_OCP_UIS_CardReadSuccessfully:	UT_Set_LED(IID_LED_GREEN);	break;
			case ETP_OCP_UIS_ProcessingError:		UT_Set_LED(IID_LED_RED);	break;
			case ETP_OCP_UIS_ReadyToRead:			UT_Set_LED(IID_LED_BLUE);	break;
			case ETP_OCP_UIS_NotReady:				UT_Set_LED(0);				break;
			case ETP_OCP_UIS_Processing:			UT_Set_LED(IID_LED_YELLOW);	break;
			default:								UT_Set_LED(0);				break;
		}
	}
	else
	{
		if(msgID == ETP_OCP_UIM_AuthorisingPleaseWait)
		{
			UT_Set_LED(IID_LED_GREEN);	
		}
	}
 
#ifdef _SCREEN_SIZE_240x320
	//20131220 increase error message display time
	if(delay_flag)
		UT_WaitTime(150);
#endif
}

void ETP_Set_DefaultOutcome(UCHAR ocmResult)
{
	printf("ocmResult=%d\n",ocmResult);
	switch (ocmResult)
	{
			
		case ETP_OUTCOME_TryAnotherInterface:
			//20140107
			L3_Response_Code = VAP_RIF_RC_OTHER_INTERFACE;
			
			etp_Outcome.Start			= ETP_OCP_Start_NA;
//			memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
			etp_Outcome.CVM				= ETP_OCP_CVM_NA;
			etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;	//20140112 don't display message
			etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PleaseInsertOrSwipeCard;
			etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
			etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_No;
			etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
			etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
			etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
			etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
			etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
			etp_Outcome.rmvTimeout		= 0;	
			break;

		case ETP_OUTCOME_EndApplication:
		
			//20140107
			if(!glv_parFlgqVSDC)	//20140115 V2, if MSD only and miss PDOL and candidate list is empty, RC_Code should be RC_Failure
				L3_Response_Code = VAP_RIF_RC_FAILURE;
			else if ((etp_depRspCode == ECL_LV1_TIMEOUT_USER) || (etp_depRspCode == ECL_LV1_TIMEOUT_ISO))
				L3_Response_Code = VAP_RIF_RC_FAILURE;	
			else if(etp_checkADF_Fail)					//20140117 V1, test case D.013.01
			{
				L3_Response_Code = VAP_RIF_RC_FAILURE;
				etp_checkADF_Fail=FALSE;

				if (etp_flgUnknowAID == TRUE)
				{
					L3_Response_Code = VAP_RIF_RC_OTHER_INTERFACE;
				}
			}
			else
			{
				if (L3_Response_Code == 0xFF)	//Default
				{
					L3_Response_Code = VAP_RIF_RC_OTHER_INTERFACE;
				}
			}
			
#ifdef _JCB_TA_
			//20140519 JCB test			
			//07	End Application
			JCB_Out_Status = JCB_TxnR_EndApp;
			JCB_Txn_Terminate = TRUE;
#endif		
			etp_Outcome.Start			= ETP_OCP_Start_NA;
//			memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
			etp_Outcome.CVM				= ETP_OCP_CVM_NA;
			etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;	//20140112 don't display message
			etp_Outcome.ocmMsgID		= ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
			etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
			etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_No;
			etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
			etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
			etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
			etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
			etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
			etp_Outcome.rmvTimeout		= 0;				
			break;

		case ETP_OUTCOME_Timeout:
			UT_ClearRow(1,2,FONT0);
			UT_ClearRow(3,2,FONT1);

			UT_LED_Switch(IID_LED_BLUE, 2);	
			
#ifdef _JCB_TA_
			//07	End Application with comm err
			JCB_Out_Status = JCB_TxnR_EndAppWithRst;
			
			etp_Outcome.Start			= ETP_OCP_Start_B;
//			memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
			etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
			etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_Yes;
			etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PresentCardAgain;
			etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
			etp_Outcome.hldTime 		= 13;
			etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_Yes;
			etp_Outcome.rstMsgID		= ETP_OCP_UIM_PresentCardAgain;
			etp_Outcome.rstStatus		= ETP_OCP_UIS_ReadyToRead;
			etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
			etp_Outcome.dscData 		= ETP_OCP_DiscretionaryDataPresent_No;
			etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
			etp_Outcome.Receipt 		= ETP_OCP_Receipt_NA;
			etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
			etp_Outcome.rmvTimeout		= 0;				
#else			
			etp_Outcome.Start			= ETP_OCP_Start_B;
//			memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
			etp_Outcome.CVM				= ETP_OCP_CVM_NA;
			etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;
			etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_No;
			etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
			etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
			etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
			etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
			etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
			etp_Outcome.rmvTimeout		= 0;				
#endif
			break;

		case ETP_OUTCOME_Contact:

			etp_Outcome.Start			= ETP_OCP_Start_NA;
//			memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
			etp_Outcome.CVM				= ETP_OCP_CVM_NA;
			etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;
			etp_Outcome.ocmMsgID		= ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
			etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
			etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_No;
			etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
			etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
			etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
			etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
			etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
			etp_Outcome.rmvTimeout		= 0;				
			break;

		case ETP_OUTCOME_Terminate:
			L3_Response_Code = VAP_RIF_RC_FAILURE;
			
			etp_Outcome.Start			= ETP_OCP_Start_NA;
			etp_Outcome.CVM				= ETP_OCP_CVM_NA;
			etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_Yes;
			etp_Outcome.ocmMsgID		= ETP_OCP_UIM_Terminate;
			etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
			etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_No;
			etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
			etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;
			etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
			etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
			etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
			etp_Outcome.rmvTimeout		= 0;	
			break;

		default:
			break;
	}

}


void ETP_Set_DefaultKID(void)
{
	switch (etp_mchAIDResult)
	{

		case 0x00:	etp_exmKIDResult[0]=ETP_KID_VISAAP;		break;
		case 0x01:	etp_exmKIDResult[0]=ETP_KID_MASTER;		break;
		case 0x02:	etp_exmKIDResult[0]=ETP_KID_VISA;		break;
		case 0x03:	etp_exmKIDResult[0]=ETP_KID_AMEX;		break;
		case 0x04:	etp_exmKIDResult[0]=ETP_KID_JCB;		break;
		case 0x05:	etp_exmKIDResult[0]=ETP_KID_DISCOVER;	break;
		case 0x06:	
		case 0x07:	
		case 0x08:	etp_exmKIDResult[0]=ETP_KID_UPI;		break;

		default:	etp_exmKIDResult[0]=0x00;				break;
	}	
}

//[Contactless D-PAS Figure 10]
UCHAR ETP_Check_DPAS_PDOL(void)
{
	UCHAR	tag9F66[2] = {0x9F, 0x66};
	UCHAR	tag9F02[2] = {0x9F, 0x02};
	UCHAR	tag9F03[2] = {0x9F, 0x03};
	UCHAR	tag9F1A[2] = {0x9F, 0x1A};
	UCHAR	tag5F2A[2] = {0x5F, 0x2A};
	UCHAR	tag9A[1] = {0x9A};
	UCHAR	tag9C[1] = {0x9C};
	UCHAR	tag9F37[2] = {0x9F, 0x37};
	UCHAR	tag9F7D[2] = {0x9F, 0x7D};
	UCHAR	*ptrData = NULLPTR;
	UCHAR	datBuffer[ETP_BUFFER_SIZE+2] = {0};
	UCHAR	*tagPtr = NULLPTR;

	//Check TTQ
	ptrData = UT_Find_TagInDOL(tag9F66, (UINT)glv_tag9F38.Length[0], glv_tag9F38.Value);
	if(ptrData != NULLPTR)
	{
		if(*(ptrData + 2) != 0x04)	//TLV-L
		{
			DPS_DBG_Put_Process(DPS_FIGURE_10, 1, FALSE);

			return FAIL;
		}

		//Copy Current TTQ to Tag 9F66
		ETP_Copy_TTQ();

		DPS_DBG_Put_Process(DPS_FIGURE_10, 1, TRUE);
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_10, 1, FALSE);

		return FAIL;
	}

	//Check Amount Authorized
	ptrData = UT_Find_TagInDOL(tag9F02, (UINT)glv_tag9F38.Length[0], glv_tag9F38.Value);
	if(ptrData != NULLPTR)
	{
		if(*(ptrData + 2) != 0x06)	//TLV-L
		{
			DPS_DBG_Put_Process(DPS_FIGURE_10, 2, FALSE);

			return FAIL;
		}

		DPS_DBG_Put_Process(DPS_FIGURE_10, 2, TRUE);
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_10, 2, FALSE);

		return FAIL;
	}

	//Check Amount Others
	ptrData = UT_Find_TagInDOL(tag9F03, (UINT)glv_tag9F38.Length[0], glv_tag9F38.Value);
	if(ptrData != NULLPTR)
	{
		if(*(ptrData + 2) != 0x06)	//TLV-L
		{
			DPS_DBG_Put_Process(DPS_FIGURE_10, 3, FALSE);

			return FAIL;
		}

		DPS_DBG_Put_Process(DPS_FIGURE_10, 3, TRUE);
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_10, 3, FALSE);

		return FAIL;
	}

	//Check Terminal Country Code
	ptrData = UT_Find_TagInDOL(tag9F1A, (UINT)glv_tag9F38.Length[0], glv_tag9F38.Value);
	if(ptrData != NULLPTR)
	{
		if(*(ptrData + 2) != 0x02)	//TLV-L
		{
			DPS_DBG_Put_Process(DPS_FIGURE_10, 4, FALSE);

			return FAIL;
		}

		DPS_DBG_Put_Process(DPS_FIGURE_10, 4, TRUE);
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_10, 4, FALSE);

		return FAIL;
	}

	//Check Transaction Currency Code
	ptrData = UT_Find_TagInDOL(tag5F2A, (UINT)glv_tag9F38.Length[0], glv_tag9F38.Value);
	if(ptrData != NULLPTR)
	{
		if(*(ptrData + 2) != 0x02)	//TLV-L
		{
			DPS_DBG_Put_Process(DPS_FIGURE_10, 5, FALSE);

			return FAIL;
		}

		DPS_DBG_Put_Process(DPS_FIGURE_10, 5, TRUE);
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_10, 5, FALSE);

		return FAIL;
	}

	//Check Transaction Date
	ptrData = UT_Find_TagInDOL(tag9A, (UINT)glv_tag9F38.Length[0], glv_tag9F38.Value);
	if(ptrData != NULLPTR)
	{
		if(*(ptrData + 1) != 0x03)	//TLV-L
		{
			DPS_DBG_Put_Process(DPS_FIGURE_10, 6, FALSE);

			return FAIL;
		}

		DPS_DBG_Put_Process(DPS_FIGURE_10, 6, TRUE);
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_10, 6, FALSE);

		return FAIL;
	}

	//Check Transaction Type
	ptrData = UT_Find_TagInDOL(tag9C, (UINT)glv_tag9F38.Length[0], glv_tag9F38.Value);
	if(ptrData != NULLPTR)
	{
		if(*(ptrData + 1) != 0x01)	//TLV-L
		{
			DPS_DBG_Put_Process(DPS_FIGURE_10, 7, FALSE);

			return FAIL;
		}

		DPS_DBG_Put_Process(DPS_FIGURE_10, 7, TRUE);
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_10, 7, FALSE);

		return FAIL;
	}

	//Check Unpredictable Number
	ptrData = UT_Find_TagInDOL(tag9F37, (UINT)glv_tag9F38.Length[0], glv_tag9F38.Value);
	if(ptrData != NULLPTR)
	{
		DPS_DBG_Put_Process(DPS_FIGURE_10, 0x8A, TRUE);

		if(glv_tag9F66.Value[0] & 0x80)	//Magnetic stripe mode supported
		{
			datBuffer[0] = (etp_rcvLen & 0x00FF);
			datBuffer[1] = (etp_rcvLen & 0xFF00) >> 8;
			memcpy(&datBuffer[2], etp_rcvData, etp_rcvLen);

//			tagPtr = apk_EMVCL_FindTag(0x9F, 0x7D, &datBuffer[2]);
			tagPtr = UT_Find_Tag(tag9F7D, (etp_rcvLen-2), etp_rcvData);
			if(tagPtr != NULLPTR)	//tag 9F7D present
			{
				DPS_DBG_Put_Process(DPS_FIGURE_10, 0x8B, TRUE);

				if(*(ptrData + 2) != 0x01)	//Check length of UN
				{
					DPS_DBG_Put_Process(DPS_FIGURE_10, 0x8C, FALSE);

					return FAIL;
				}

				DPS_DBG_Put_Process(DPS_FIGURE_10, 0x8C, TRUE);

				//Optional PDOL checks
//				DBG_Put_String(20, (UCHAR*)"Optional PDOL checks");

				return SUCCESS;
			}
		}

		DPS_DBG_Put_Process(DPS_FIGURE_10, 0x8B, FALSE);

		if(*(ptrData + 2) != 0x04)	//Check length of UN
		{
			DPS_DBG_Put_Process(DPS_FIGURE_10, 0x8D, FALSE);

			return FAIL;
		}

		DPS_DBG_Put_Process(DPS_FIGURE_10, 0x8D, TRUE);

		//[Contactless D-PAS Figure 10 - Optional PDOL checks] has already implemented in "DPS_DOL_Get_DOLData" function
//		DBG_Put_String(20, (UCHAR*)"Optional PDOL checks");

		return SUCCESS;
	}
	else
	{
		DPS_DBG_Put_Process(DPS_FIGURE_10, 0x8A, FALSE);

		return FAIL;
	}
}

UCHAR ETP_Check_PDOL(UINT iptLen, UCHAR *iptData)
{
	UCHAR	tag9F38[2]={0x9F,0x38};
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=FAIL;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;

	ptrData=UT_Find_Tag(tag9F38, iptLen, iptData);
	if (ptrData != NULLPTR)
	{
		rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		if (rspCode == SUCCESS)
		{
			if (lenOfV != 0) return SUCCESS;
		}
	}
	
	return FAIL;
}

UCHAR ETP_Check_FCIMandatoryTag(void)
{
	UCHAR	*ptrTag=0;
	UCHAR	tag61[1]={0x61};
	UCHAR	tag84[1]={0x84};
	UCHAR	tag4F[1]={0x4F};
	UCHAR	tagBF0C[2]={0xBF,0x0C};
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	valOf84[14]={'2', 'P', 'A', 'Y', '.', 'S', 'Y', 'S', '.', 'D', 'D', 'F', '0', '1'};

	ptrTag=UT_Find_Tag(tag61, (etp_rcvLen-2), etp_rcvData);
	if (ptrTag == NULLPTR)
	{
		return FAIL;
	}

	ptrTag=UT_Find_Tag(tagBF0C, (etp_rcvLen-2), etp_rcvData);
	if (ptrTag == NULLPTR)
	{
		return FAIL;
	}

	//for JCB test case SELECT PPSE 028, if the length of tag BF0C if zero, kernel shall provide "End Application" result
	UT_Get_TLVLengthOfV((ptrTag+2), &lenOfV);
	if (lenOfV == 0)
	{
		return FAIL;
	}

	ptrTag=UT_Find_Tag(tag84, (etp_rcvLen-2), etp_rcvData);	
	if (ptrTag != NULLPTR)
	{
		UT_Get_TLVLength(ptrTag, &lenOfT, &lenOfL, &lenOfV);
		if ((lenOfV != sizeof(valOf84)) ||
			(memcmp((ptrTag+lenOfT+lenOfL), valOf84, sizeof(valOf84))))
		{
			return FAIL;
		}
	}
	else
	{
		ptrTag=UT_Find_Tag(tag4F, (etp_rcvLen-2), etp_rcvData);
		if (ptrTag != NULLPTR)
		{
			UT_Get_TLVLength(ptrTag, &lenOfT, &lenOfL, &lenOfV);
			if (!memcmp((ptrTag+lenOfT+lenOfL), etp_aidAMEX, 6))
			{
				return FAIL;
			}
		}
	}

	return SUCCESS;
}

//[Contactless D-PAS Figure 9 Format Analysis]
UCHAR ETP_Check_DPAS_FCI(void)
{
	UCHAR	datBuffer[ETP_BUFFER_SIZE + 2] = {0};
	UCHAR	padLen = 0;
	UCHAR	*ptrData;
	UCHAR	*TmpTagPtr = NULLPTR;
	UCHAR	*Tag50Ptr = NULLPTR;
	UCHAR	lenOfL = 0;
	UINT	lenOfV = 0;
	UCHAR	recLen[2] = {0};
	UCHAR	rspCode = 0;
	UCHAR	tmp_value = 0;
	UCHAR	supportOtherInterface = FALSE;
	UCHAR	tag50[1]={0x50};
	UCHAR	tag84[1]={0x84};

	//Check if it supports contact or magnetic stripe
	tmp_value = etp_etpConfig[6].TTQ[0];
	tmp_value &= 0x90;	//TTQ B1b8 : Magnetic stripe mode supported, B1b5 : EMV contact chip supported
	if(tmp_value)
	{
		supportOtherInterface = TRUE;
	}

	if(etp_rcvLen > 2)
	{
		//Check Receive Data is FCI
		if (etp_rcvData[0] == 0x6F)
		{
			//Patch Length to Fit apk_EMVCL_ParseLenFCI Format
			datBuffer[0] = (etp_rcvLen & 0x00FF);
			datBuffer[1] = (etp_rcvLen & 0xFF00) >> 8;
			memcpy(&datBuffer[2], etp_rcvData, etp_rcvLen);
			
//			rspCode = apk_EMVCL_ParseLenFCI(datBuffer, &padLen);
//			if (rspCode == TRUE)
			rspCode=ETP_Parse_FCI((etp_rcvLen-2), etp_rcvData);
			if (rspCode == SUCCESS)
			{
				//Step 1 : Check tag 84
//				ptrData = apk_EMVCL_FindTag(0x84, 0x00, &datBuffer[2]);
				ptrData = UT_Find_Tag(tag84, (etp_rcvLen-2), etp_rcvData);
				if(ptrData == NULLPTR)
				{
					if(supportOtherInterface)
					{
						DPS_Try_Another_Interface_OutCome();
					}
					else
					{
						DPS_End_Application_OutCome();
					}

					DPS_DBG_Put_Process(DPS_FIGURE_9, 1, FALSE);

					return FAIL;
				}
				else  //ptrData point to 84-L
				{
					ptrData++;	//Point to L
					DPS_DBG_Put_Process(DPS_FIGURE_9, 1, TRUE);

					UT_EMVCL_GetBERLEN(ptrData, &lenOfL, &lenOfV);	//Get length of 84-L and 84-V
					if(lenOfV == 0)
					{
						if(supportOtherInterface)
						{
							DPS_Try_Another_Interface_OutCome();
						}
						else
						{
							DPS_End_Application_OutCome();
						}

						DPS_DBG_Put_Process(DPS_FIGURE_9, 2, FALSE);

						return FAIL;
					}
				}

				ptrData += (lenOfL + lenOfV);	//ptrData point to next tag

				//Step 2 : Check tag A5
				if(*ptrData != 0xA5)
				{
					if(supportOtherInterface)
					{
						DPS_Try_Another_Interface_OutCome();
					}
					else
					{
						DPS_End_Application_OutCome();
					}

					DPS_DBG_Put_Process(DPS_FIGURE_9, 2, FALSE);

					return FAIL;
				}

				ptrData += 1;	//Point to A5-L

				//Get A5 Length of TLV-L
				UT_EMVCL_GetBERLEN(ptrData, &lenOfL, &lenOfV);
				if (lenOfV == 0)
				{
					if(supportOtherInterface)
					{
						DPS_Try_Another_Interface_OutCome();
					}
					else
					{
						DPS_End_Application_OutCome();
					}

					DPS_DBG_Put_Process(DPS_FIGURE_9, 2, FALSE);

					return FAIL;
				}
				else
				{
					//Get length of A5-V
					recLen[0] = (lenOfV & 0x00FF);
					recLen[1] = (lenOfV & 0xFF00) >> 8;

					//Point to Next Tag
					ptrData += lenOfL;	//ptrData should point to tag 50

//					rspCode=apk_EMVCL_ParseTLV(recLen, ptrData, padLen);
//					if (rspCode == TRUE)
					rspCode=UT_Check_EMVTagEncodingError(lenOfV, ptrData);
					if (rspCode == FALSE)
					{
						//Step 3 : Check tag 50
//						Tag50Ptr = apk_EMVCL_FindTag(0x50,0x00,ptrData);
						Tag50Ptr = UT_Find_Tag(tag50, lenOfV, ptrData);

						if(!Tag50Ptr)
						{
							if(supportOtherInterface)
							{
								DPS_Try_Another_Interface_OutCome();
							}
							else
							{
								DPS_End_Application_OutCome();
							}

							DPS_DBG_Put_Process(DPS_FIGURE_9, 3, FALSE);

							return FAIL;
						}
						else  //Tag50Ptr point to 50-L
						{
							Tag50Ptr++;	//Point to L
							DPS_DBG_Put_Process(DPS_FIGURE_9, 3, TRUE);

							UT_EMVCL_GetBERLEN(Tag50Ptr, &lenOfL, &lenOfV);
							TmpTagPtr = Tag50Ptr + lenOfL + lenOfV;	//TmpTagPtr point to next tag

							//tag 87 is optional
							if(*TmpTagPtr == 0x87)
							{
								TmpTagPtr++;	//TmpTagPtr point to 87-L
								UT_EMVCL_GetBERLEN(TmpTagPtr, &lenOfL, &lenOfV);	//Get length of 87-L and 87-V
								TmpTagPtr += (lenOfL + lenOfV);	//TmpTagPtr point to next tag

								//Step 4 : Check tag 9F38
								if((*TmpTagPtr == 0x9F) && (*(TmpTagPtr + 1) == 0x38))	//Processing Options Data Object List (PDOL)
								{
									DPS_DBG_Put_Process(DPS_FIGURE_9, 4, TRUE);

									//It can go to PDOL Analysis
									return SUCCESS;
								}
								else
								{
									if(supportOtherInterface)
									{
										DPS_Try_Another_Interface_OutCome();
									}
									else
									{
										DPS_End_Application_OutCome();
									}

									DPS_DBG_Put_Process(DPS_FIGURE_9, 4, FALSE);

									return FAIL;
								}
							}
							//If tag 87 doesn't present, next tag should be tag 9F38
							//Step 4 : Check tag 9F38
							else if((*TmpTagPtr == 0x9F) && (*(TmpTagPtr + 1) == 0x38))
							{
								DPS_DBG_Put_Process(DPS_FIGURE_9, 4, TRUE);

								//It can go to PDOL Analysis
								return SUCCESS;
							}
							else
							{
								if(supportOtherInterface)
								{
									DPS_Try_Another_Interface_OutCome();
								}
								else
								{
									DPS_End_Application_OutCome();
								}

								DPS_DBG_Put_Process(DPS_FIGURE_9, 4, FALSE);

								return FAIL;
							}
						}
					}
					else
					{
						if(supportOtherInterface)
						{
							DPS_Try_Another_Interface_OutCome();
						}
						else
						{
							DPS_End_Application_OutCome();
						}

						return FAIL;
					}
				}
			}
		}
	}
	
	if(supportOtherInterface)
	{
		DPS_Try_Another_Interface_OutCome();
	}
	else
	{
		DPS_End_Application_OutCome();
	}

	return FAIL;
}

UCHAR ETP_Check_JCB_FCI(void)
{
	UCHAR	datBuffer[ETP_BUFFER_SIZE+2]={0};
	UCHAR	padLen=0;
	UCHAR	*ptrData;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	recLen[2]={0};
	UCHAR	rspCode=0;
	UCHAR	*Tag50Ptr = NULLPTR;
	UCHAR	LenOftag50L=0;
	UINT 	LenOftag50V=0;
	UCHAR	*TmpTagPtr = NULLPTR;
	UCHAR	TmpLenOfT=0,TmpLenOfL=0;
	UINT 	TmpLenOfV=0;
	ULONG	TmpIndex=0;
	UINT	TotalLen=0;
	UINT	cntIndex=0;
	UCHAR	tag50[1]={0x50};
	UCHAR	tag84[1]={0x84};
	UCHAR	tag9F4D[2]={0x9F,0x4D};
		
	if (etp_rcvLen > 2)
	{
		//Check Receive Data is FCI
		if (etp_rcvData[0] == 0x6F)
		{
			//JCB CL 1.3 Update - Requirement 5.2.1.4
			rspCode=ETP_Check_PDOL((etp_rcvLen-2), etp_rcvData);
			if (rspCode == FAIL)
			{
				JCB_JCB_FCI_Error = TRUE;	//Kernel will Select Next
				return SUCCESS;
			}
			
			//Patch Length to Fit apk_EMVCL_ParseLenFCI Format
			datBuffer[0]=(etp_rcvLen & 0x00FF);
			datBuffer[1]=(etp_rcvLen & 0xFF00) >> 8;
			memcpy(&datBuffer[2], etp_rcvData, etp_rcvLen);
			
//			rspCode=apk_EMVCL_ParseLenFCI(datBuffer, &padLen);
//			if (rspCode == TRUE)
			rspCode=ETP_Parse_FCI((etp_rcvLen-2), etp_rcvData);
			if (rspCode == SUCCESS)
			{
				//JCB_Test Select DF 027, if tag 84 is missing, kernel shall return Select Next outcome
//				ptrData=apk_EMVCL_FindTag(0x84, 0x00, &datBuffer[2]);
				ptrData=UT_Find_Tag(tag84, (etp_rcvLen-2), etp_rcvData);
				if(ptrData == NULLPTR)
				{
					JCB_JCB_FCI_Error = TRUE;	//JCB select DF 021
					return SUCCESS;
				}
				else
				{
					ptrData++;	//Point to L
					//JCB_Test Select DF 035, if length of tag 84 is zero, kernel shall return Select Next outcome
					UT_EMVCL_GetBERLEN(ptrData, &lenOfL, &lenOfV);
					if(lenOfV == 0)
					{
						JCB_JCB_FCI_Error = TRUE;	//JCB select DF 035
						return SUCCESS;
					}
					//JCB_Test Select DF 035, if length of tag 84 is zero, kernel shall return Select Next outcome end
				}
				//JCB_Test Select DF 027, if tag 84 is missing, kernel shall return Select Next outcome end 
				
				//ptrData=apk_EMVCL_FindTag(0xA5, 0x00, &datBuffer[2]);
				//if(ptrData == NULLPTR)
				ptrData+=(lenOfL+lenOfV);			//SELECT_DF_095 , the format of response of select AID is followed by table45 of EMV book1
				if(*ptrData != 0xA5)
				{
					//JCB CL 1.3 Update - Check Padding Between 84 & A5
					if ((ptrData[0] == 0x00) || (ptrData[0] == 0xFF))
					{
						for (cntIndex=0; cntIndex < ((etp_rcvLen-2)-(1+lenOfL+lenOfV)); cntIndex++)
						{
							if ((ptrData[cntIndex] == 0x00) || (ptrData[cntIndex] == 0xFF))
							{
								continue;
							}
							else
							{
								ptrData+=cntIndex;	//Skip Padding
								break;
							}
						}
					}
					else
					{
						JCB_JCB_FCI_Error = TRUE;	//JCB select DF 035
						return SUCCESS;
					}
				}
				
				ptrData+=1;		//add 1 for length of tag A5
				
				//Get A5 Length of TLV-L
				UT_EMVCL_GetBERLEN(ptrData, &lenOfL, &lenOfV);
				if (lenOfV == 0)
				{
					return SUCCESS;
				}
				else
				{
					recLen[0]=(lenOfV & 0x00FF);
					recLen[1]=(lenOfV & 0xFF00) >> 8;

					//Point to Next Tag
					ptrData+=lenOfL;	

//					rspCode=apk_EMVCL_ParseTLV(recLen, ptrData, padLen);
//					if (rspCode == TRUE)
					rspCode=UT_Check_EMVTagEncodingError(lenOfV, ptrData);
					if (rspCode == FALSE)
					{
						//We follow EMV Book1 table 45: the tag50 is Manadatory in tagA5, and check the tag87 length 
//						Tag50Ptr = apk_EMVCL_FindTag(0x50,0x00,ptrData);
						Tag50Ptr=UT_Find_Tag(tag50, lenOfV, ptrData);
						if(!Tag50Ptr)
						{
							JCB_JCB_FCI_Error = TRUE;	//JCB select DF 045
						}
						else
						{
							Tag50Ptr++;	//Point to L
							UT_EMVCL_GetBERLEN(Tag50Ptr,&LenOftag50L,&LenOftag50V);

							if(	(*(Tag50Ptr+LenOftag50L+LenOftag50V) == 0x87)	||
								((*(Tag50Ptr+LenOftag50L+LenOftag50V) == 0x9F) && (*(Tag50Ptr+LenOftag50L+LenOftag50V+1) == 0x38))	||
								((*(Tag50Ptr+LenOftag50L+LenOftag50V) == 0x5F) && (*(Tag50Ptr+LenOftag50L+LenOftag50V+1) == 0x2D))	||
								((*(Tag50Ptr+LenOftag50L+LenOftag50V) == 0x9F) && (*(Tag50Ptr+LenOftag50L+LenOftag50V+1) == 0x11))	||
								((*(Tag50Ptr+LenOftag50L+LenOftag50V) == 0x9F) && (*(Tag50Ptr+LenOftag50L+LenOftag50V+1) == 0x12))	||
								((*(Tag50Ptr+LenOftag50L+LenOftag50V) == 0xBF) && (*(Tag50Ptr+LenOftag50L+LenOftag50V+1) == 0x0C))	||
								((*(Tag50Ptr+LenOftag50L+LenOftag50V) == 0x90) && (*(Tag50Ptr+LenOftag50L+LenOftag50V+1) == 0x00)))
							{
								lenOfV-=(1+LenOftag50L+LenOftag50V);	//Tag50, T(1)+L(LenOftag50L)+V(LenOftag50V)
								TmpTagPtr = Tag50Ptr+LenOftag50L+LenOftag50V;
								
								while(TotalLen<lenOfV)
								{
									UT_Get_TLVLength(TmpTagPtr,&TmpLenOfT,&TmpLenOfL,&TmpLenOfV);
									
									if(*TmpTagPtr & 0x20)	//Constructed tags, BF0C, tag9F4D format error JCB_Select_DF_091~92
									{
//										if(apk_EMVCL_FindTag(0x9F,0x4D,TmpTagPtr))
										if (UT_Find_Tag(tag9F4D, TmpLenOfV, (TmpTagPtr+TmpLenOfT+TmpLenOfL)))
										{
											if(TmpLenOfV == 0)
											{
												JCB_JCB_FCI_Error = TRUE;
												break;
											}
										}
										TmpTagPtr+=(TmpLenOfT+TmpLenOfL);
										TotalLen += (TmpLenOfT+TmpLenOfL);
									}
									else					//Primitive tags
									{
										rspCode = UT_Search(TmpLenOfT,TmpTagPtr,(UCHAR*)glv_tagTable,ECL_NUMBER_TAG,ECL_SIZE_TAGTABLE,&TmpIndex);
										if(rspCode == SUCCESS)
										{
											if(glv_tagTable[TmpIndex].JCB_Length)	
											{
												if(glv_tagTable[TmpIndex].JCB_Length_Def == JCB_Fixed_Length)	//JCB_Select_DF_060, check Format of tag87
												{
													if(TmpLenOfV != glv_tagTable[TmpIndex].JCB_Length)
													{
														JCB_JCB_FCI_Error = TRUE;
														return SUCCESS;
													}
												}
												else if(glv_tagTable[TmpIndex].JCB_Length_Def == JCB_Var_Length)
												{
													if(TmpLenOfV>glv_tagTable[TmpIndex].JCB_Length)
													{
														JCB_JCB_FCI_Error = TRUE;
														return SUCCESS;
													}
												}
											}	
										}

										TmpTagPtr+=(TmpLenOfT+TmpLenOfL+TmpLenOfV);//shift to next Tag
										TotalLen += (TmpLenOfT+TmpLenOfL+TmpLenOfV);
									}									
								}
								
								return SUCCESS;
							}
							else
								JCB_JCB_FCI_Error = TRUE;	//JCB Select_DF_46	check lenght of tag 50 
						}
						
						return SUCCESS;
					}
					else
					{
						JCB_JCB_FCI_Error = TRUE;	//JCB select DF 021
						return SUCCESS;
					}
				}
			}
			else
			{
				JCB_JCB_FCI_Error = TRUE;	//JCB select DF 021
				return SUCCESS;
			}
		}
		else
		{
			JCB_JCB_FCI_Error = TRUE;	//JCB select DF 017
			return SUCCESS;
		}
	}
	else
	{
		//JCB CL 1.3 Update - Select Next when FCI is Missing
		if((etp_rcvData[0] == 0x90) && (etp_rcvData[1] == 0x00))
		{
			JCB_JCB_FCI_Error = TRUE;
			return SUCCESS;
		}

		return SUCCESS;
	}
	
	return FAIL;
}

UCHAR ETP_Check_FCI(void)
{
	UCHAR	*ptrData;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	rspCode=0;
	UCHAR	tagA5[1]={0xA5};
	
	if (etp_rcvLen > 2)
	{
		//Check Receive Data is FCI
		if (etp_rcvData[0] == 0x6F)
		{
			rspCode=ETP_Parse_FCI((etp_rcvLen-2), etp_rcvData);
			if (rspCode == SUCCESS)
			{
				ptrData=UT_Find_Tag(tagA5, (etp_rcvLen-2), etp_rcvData);
				UT_EMVCL_GetBERLEN((ptrData+1), &lenOfL, &lenOfV);
				if (lenOfV != 0)
				{
					//Point to Value of A5
					ptrData+=(1+lenOfL);	

					rspCode=UT_Check_EMVTagEncodingError(lenOfV, ptrData);
					if (rspCode == TRUE)
					{
						return FAIL;
					}
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
	}
	
	return SUCCESS;
}


UCHAR ETP_Check_ADFName(void)
{
	UINT	lenOfV=0;

	UT_Get_TLVLengthOfV(glv_tag4F.Length, &lenOfV);
	
	//According to [EMV 4.2 Book 1], section 12.2.1
	if (lenOfV >= 5)
	{
		if (!memcmp(glv_tag4F.Value, etp_aidVISA, 5))	return SUCCESS;
		if (!memcmp(glv_tag4F.Value, etp_aidMaster, 5))	return SUCCESS;
		if (!memcmp(glv_tag4F.Value, etp_aidJCB, 5))	return SUCCESS;
		if (!memcmp(glv_tag4F.Value, etp_aidUPI, 5))	return SUCCESS;
		if (!memcmp(glv_tag4F.Value, etp_aidAMEX, 5))	return SUCCESS;
		if (!memcmp(glv_tag4F.Value, etp_aidDPS, 5))	return SUCCESS;
	}
	
	return FAIL;
}


UCHAR ETP_Check_KIDSupport(UCHAR cmbKID, UCHAR rqtKID)
{
	UCHAR bitIndex=0x80;
	UCHAR lstIndex=0;

	for (lstIndex=0; lstIndex<8; lstIndex++, bitIndex>>=1)
	{
		if (cmbKID & bitIndex)			//Combination Support KID
		{
			if ((lstIndex+1) == rqtKID)	//Support KID = Request KID
				return SUCCESS;
		}
	}

	return FAIL;
}


UCHAR ETP_Check_ApplicationNotAllowed(void)
{
	//Check Application Not Allowed in C Step2_E
	
	return SUCCESS;
}


UCHAR ETP_Check_UPI_AID(void)
{
	if (!memcmp(etp_cddList[etp_cddIndex].adfName, etp_aidUPI, 5))
		return SUCCESS;


	return FAIL;
}


UCHAR ETP_Check_VISA_AID(void)
{
	if (!memcmp(etp_cddList[etp_cddIndex].adfName, etp_aidVISA, 5))
		return SUCCESS;

	return FAIL;
}


void ETP_Check_ExtendedAID(void)
{
	UCHAR	flgExtSelect=FALSE;

	if (etp_cddList[etp_cddIndex].extLen != 0)
	{
		if (etp_etpConfig[etp_cddList[etp_cddIndex].krnID[0]].pstExtSelection == TRUE)
		{
			if (etp_etpConfig[etp_cddList[etp_cddIndex].krnID[0]].flgExtSelection == TRUE)
			{
				flgExtSelect=TRUE;
			}
		}
	}

	if (flgExtSelect == FALSE)
	{
		etp_cddList[etp_cddIndex].extLen=0;
	}
}


void ETP_Display_PresentCard(void)
{
	UCHAR	dspMessage1[64]={0};	//Display Message 1
	UCHAR	dspMessage2[64]={0};	//Display Message 2	
	UCHAR 	dsp_Msg_Len1 = 0;
	UCHAR 	dsp_Msg_Len2 = 0;
	UCHAR	dsp_line_row = 1;
	UCHAR 	Message1[32] = {0x00};
	UCHAR 	Message2[32] = {0x00};
	UCHAR 	Message3[32] = {0x00};
	UCHAR 	Message4[32] = {0x00};
	UCHAR 	Msg_Len1 = 0;
	UCHAR 	Msg_Len2 = 0;
	UCHAR 	Msg_Len3 = 0;
	UCHAR 	Msg_Len4 = 0;

	UT_ClearRow(1, 2, FONT1);
	
	UT_Handle_2Type_Message(glv_vap_Message[17].Message, glv_vap_Message[17].Length, dspMessage1, &dsp_Msg_Len1, dspMessage2, &dsp_Msg_Len2);
	
	if(dsp_Msg_Len1)		//First Message			
		UT_Handle_2Line_Message(dspMessage1,(UINT)dsp_Msg_Len1,Message1,&Msg_Len1,Message2,&Msg_Len2);

	if(dsp_Msg_Len2)		//Second Message
		UT_Handle_2Line_Message(dspMessage2,(UINT)dsp_Msg_Len2,Message3,&Msg_Len3,Message4,&Msg_Len4);
	
	dsp_line_row = 9;	//Under Landing Plan
	
	if(Msg_Len1)
	{
		UT_ClearRow(dsp_line_row,1,FONT1);
		UT_Disp_Show_Status(Msg_Len1,Message1,&dsp_line_row);
	}
	
	if(Msg_Len2)
	{
		UT_ClearRow(dsp_line_row,1,FONT1);
		UT_Disp_Show_Status(Msg_Len2,Message2,&dsp_line_row);
	}
	
	if(Msg_Len3)
	{
		UT_ClearRow(dsp_line_row,1,FONT1);
		UT_Disp_Show_Status(Msg_Len3,Message3,&dsp_line_row);
	}
	
	if(Msg_Len4)
	{
		UT_ClearRow(dsp_line_row,1,FONT1);
		UT_Disp_Show_Status(Msg_Len4,Message4,&dsp_line_row);
	}
}

void ETP_Display_AmountMessage(void)
{
	UCHAR 	Msg_len = 0;
	UCHAR 	Amount_Line_Num = 0;
	UCHAR	countCHfont=0;
	UCHAR	colnum=0;
	UT_LED_Switch(IID_LED_BLUE,2);
 
#ifdef _PLATFORM_AS350_LITE
 
#ifdef _ACQUIRER_CTCB
 
	ETP_UI_Request(ETP_OCP_UIM_PresentCard, ETP_OCP_UIS_ReadyToRead);
#else
 
	ETP_Display_PresentCard();
#endif

#else
 
	ETP_UI_Request(ETP_OCP_UIM_PresentCard, ETP_OCP_UIS_ReadyToRead);
#endif

#ifdef _SCREEN_SIZE_128x64
	Amount_Line_Num = 2;
#else

	Amount_Line_Num = 3;
	// Amount_Line_Num = 5;//modify by west
#endif
 //modify by west
	// UT_ClearRow(Amount_Line_Num,1,FONT1);
	UT_ClearRow(Amount_Line_Num,1,FONT2);
	// printf("api_msg_amount_len=%d\n",api_msg_amount_len);
	if((Display_MAX_Num-api_msg_amount_len)>9)
		colnum=3;
	else if((Display_MAX_Num-api_msg_amount_len)>4)
		colnum=2;
	else
		colnum=1;
	if(api_msg_amount_len>=Display_MAX_Num+1)
	{
		if(api_msg_amount[Msg_len] & 0x80)
		{ 
			UT_PutStr(Amount_Line_Num*FONT2_W,((Display_MAX_Num-glv_vap_Message[30].Length)/2)*FONT2_H,FONT2,glv_vap_Message[30].Length,glv_vap_Message[30].Message);	
		}
		else
		{ 
			// UT_PutStr(Amount_Line_Num,((Display_MAX_Num-glv_vap_Message[30].Length)/2),FONT1,glv_vap_Message[30].Length,glv_vap_Message[30].Message);
			UT_PutStr(Amount_Line_Num*FONT2_W,((Display_MAX_Num-glv_vap_Message[30].Length)/2)*FONT2_H,FONT2,glv_vap_Message[30].Length,glv_vap_Message[30].Message);
		}

		UT_WaitTime(100);
 
		// UT_PutStr(Amount_Line_Num,(Display_MAX_Num-(api_msg_amount_len-glv_vap_Message[30].Length))/2,FONT1,(api_msg_amount_len-glv_vap_Message[30].Length),&api_msg_amount[glv_vap_Message[30].Length]);
		UT_PutStr(Amount_Line_Num*FONT2_W,(Display_MAX_Num-(api_msg_amount_len-glv_vap_Message[30].Length))/2*FONT2_H,FONT2,(api_msg_amount_len-glv_vap_Message[30].Length),&api_msg_amount[glv_vap_Message[30].Length]);
	}
	else
	{
		while(1)
		{
			if(api_msg_amount[Msg_len] & 0x80)
			{
				
				// UT_PutStr(Amount_Line_Num*FONT2_H,colnum+(Msg_len/2),FONT2,2,&api_msg_amount[Msg_len]);	
				UT_PutStr(Amount_Line_Num*FONT2_H,(Display_MAX_Num-api_msg_amount_len+Msg_len*2)*FONT12_W/2,FONT2,2,&api_msg_amount[Msg_len]);	
				Msg_len+=2;
				countCHfont++;
				// Msg_len+=1;//modify by west
			}
			else
			{
				UT_PutStr(Amount_Line_Num*FONT2_H,(Display_MAX_Num-api_msg_amount_len+Msg_len*2)*FONT12_W/2,FONT2,1,&api_msg_amount[Msg_len]);	
				Msg_len+=1;
				// Msg_len+=2;//modify by west
			}

			if(Msg_len == api_msg_amount_len)
				break;
		}
	}			
}


UCHAR ETP_LV1_Check_Interrupt(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=ITR_Check_ICC();		if (rspCode == TRUE)	return ECL_LV1_STOP_ICCARD;
	rspCode=ITR_Check_MagStripe();	if (rspCode == TRUE)	return ECL_LV1_STOP_MAGSTRIPE;
	rspCode=ITR_Check_Cancel();		if (rspCode == TRUE)	return ECL_LV1_STOP_CANCEL;
	rspCode=ITR_Check_StopNow();	if (rspCode == TRUE)	return ECL_LV1_STOP_NOW;
	rspCode=ITR_Check_StopLater();	if (rspCode == TRUE)	return ECL_LV1_STOP_LATER;
	
	return ECL_LV1_FAIL;
}


UCHAR ETP_LV1_POLLING(ULONG *iptTimeout)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	ULONG	tmrTick1, tmrTick2;
	UCHAR	cntPrevious=0;

	ECL_LV1_ResetPCDParameter();

	rspCode=ITR_Reset_Flags();

	do
	{
		tmrTick1=OS_GET_SysTimerFreeCnt();

		ETP_Display_CountdownSecond(&cntPrevious, iptTimeout[0]);
		
		rspCode=ECL_LV1_POLLING_TypeA();			if (rspCode == ECL_LV1_SUCCESS)	break;
		rspCode=ECL_LV1_POLLING_CheckCollision();	if (rspCode == ECL_LV1_SUCCESS)	{rspCode=ECL_LV1_COLLISION; break;}
		rspCode=ECL_LV1_POLLING_TypeB();			if (rspCode == ECL_LV1_SUCCESS)	break;
		rspCode=ECL_LV1_POLLING_CheckCollision();	if (rspCode == ECL_LV1_SUCCESS)	{rspCode=ECL_LV1_COLLISION; break;}

		rspCode=ETP_LV1_Check_Interrupt();			if (rspCode != ECL_LV1_FAIL)	break;

		tmrTick2=OS_GET_SysTimerFreeCnt();
		ETP_Update_Timer(tmrTick1, tmrTick2, iptTimeout);
	} while (iptTimeout[0] > 0);

	if (iptTimeout[0] == 0) rspCode=ECL_LV1_TIMEOUT_USER;

	if (rspCode == ECL_LV1_SUCCESS)
	{
		tmrTick2=OS_GET_SysTimerFreeCnt();
		ETP_Update_Timer(tmrTick1, tmrTick2, iptTimeout);
	}

	return rspCode;
}


UCHAR ETP_Parse_FCI(UINT iptLen, UCHAR *iptData)
{
	UCHAR	tagA5[1]={0xA5};
	UCHAR	*ptrData=NULLPTR;
	UCHAR	*ptrA5=NULLPTR;
	UCHAR	rspCode=FAIL;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfV_6F=0;
	UINT	lenOfData=0;
	UINT	lenOfParse=0;
	UINT	lenOfPadding=0;
	UINT	lenOfTLV_A5=0;
	UINT	lenBeforeA5=0;
	UINT	lenAfterA5=0;

	//Check FCI Tag
	if (iptData[0] != 0x6F)
	{
		return FAIL;
	}

	//Check 6F TLV Format
	rspCode=UT_Get_TLVLength(iptData, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == FAIL)
	{
		return FAIL;
	}

	//Check Input Length
	if (iptLen != (lenOfT+lenOfL+lenOfV))
	{
		return FAIL;
	}

	//Point to Value of 6F
	ptrData=iptData+lenOfT+lenOfL;
	lenOfV_6F=lenOfV;

	//Find A5
	ptrA5=UT_Find_Tag(tagA5, lenOfV_6F, ptrData);
	if (ptrA5 == NULLPTR)
	{
		return FAIL;
	}

	//Check Data Before A5
	lenOfParse=ptrA5-ptrData;
	if (lenOfParse > 0)
	{
		do
		{
			if ((ptrData[0] == 0x00) || (ptrData[0] == 0xFF))
			{
				//Padding
				UT_Check_Padding(lenOfParse, lenOfData, ptrData, &lenOfPadding);

				ptrData+=lenOfPadding;
				lenOfData+=lenOfPadding;
				continue;
			}
			else
			{
				//Tag
				rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == FAIL)
				{
					return FAIL;
				}

				ptrData+=(lenOfT+lenOfL+lenOfV);
				lenOfData+=(lenOfT+lenOfL+lenOfV);
			}
		} while (lenOfData < lenOfParse);
	}

	if (lenOfData != lenOfParse)
	{
		return FAIL;
	}

	lenBeforeA5=lenOfParse;

	//Check A5 TLV Format
	rspCode=UT_Get_TLVLength(ptrA5, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == FAIL)
	{
		return FAIL;
	}

	lenOfTLV_A5=(lenOfT+lenOfL+lenOfV);

	//Check Length Before A5
	if ((lenBeforeA5+lenOfTLV_A5) > lenOfV_6F)
	{
		return FAIL;
	}

	//Check Data After A5
	if ((lenBeforeA5+lenOfTLV_A5) != lenOfV_6F)
	{
		//Point to the Tag After A5
		ptrData=ptrA5+lenOfTLV_A5;
		
		lenOfParse=lenOfV_6F-(lenBeforeA5+lenOfTLV_A5);
		lenOfData=0;
		lenOfPadding=0;
		
		do
		{
			if ((ptrData[0] == 0x00) || (ptrData[0] == 0xFF))
			{
				//Padding
				UT_Check_Padding(lenOfParse, lenOfData, ptrData, &lenOfPadding);

				ptrData+=lenOfPadding;
				lenOfData+=lenOfPadding;
				continue;
			}
			else
			{
				//Tag
				rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == FAIL)
				{
					return FAIL;
				}

				ptrData+=(lenOfT+lenOfL+lenOfV);
				lenOfData+=(lenOfT+lenOfL+lenOfV);
			}
		} while (lenOfData < lenOfParse);

		if (lenOfData != lenOfParse)
		{
			return FAIL;
		}

		lenAfterA5=lenOfParse;
	}

	if ((lenBeforeA5+lenOfTLV_A5+lenAfterA5) != lenOfV_6F)
	{
		return FAIL;
	}
	
	return SUCCESS;
}

UCHAR ETP_Parse_DirectoryEntry(void)
{
	UCHAR	rspCode=0;
	UCHAR	lenOfT=0;		//TLV Length of T
	UCHAR	lenOfL=0;		//TLV Length of L
	UINT	lenOfV=0;		//TLV Length of V
	UINT	lenOf61=0;		//Length of Tag 61
	UINT	lenOfData=0;	//Length of Data
	UCHAR	flg4F=0;		//Flag of Tag 4F Present
	UINT	lenOfPadding=0; //Length of Padding Byte
	UCHAR	flgLenError=FALSE;
	UCHAR	cntIndex=0;
	UCHAR	tag4F[1]={0x4F};
	UCHAR	tag50[1]={0x50};
	UCHAR	tag61[1]={0x61};
	UCHAR	tag87[1]={0x87};
	UCHAR	tag9F2A[2]={0x9F,0x2A};
	UCHAR	tag9F29[2]={0x9F,0x29};
	UCHAR	sw9000[2]={0x90,0x00};

	//Check End of Directory Entry
	if (*etp_ptrData != 0x61)
	{
		return 0xFF;
	}

	//Clear Directory Entry Tags
	ETP_Clear_DirectoryEntryTag();

	//Check Directory Entry
	if (!memcmp(etp_ptrData, tag61, 1))	//Tag 61: Directory Entry
	{
		rspCode=UT_Get_TLVLength(etp_ptrData, &lenOfT, &lenOfL, &lenOfV);
		if (rspCode == SUCCESS)
		{
			lenOf61=lenOfV;
			if (lenOf61 == 0)
			{
				return FAIL;
			}

			etp_ptrData+=(lenOfT+lenOfL);	//Point to V of Tag 61
		}
		else
		{
			return FAIL;
		}
	}

	while (1)
	{
		if (!memcmp(etp_ptrData, tag4F, 1))	//Tag 4F: ADF Name
		{
			rspCode=UT_Get_TLVLength(etp_ptrData, &lenOfT, &lenOfL, &lenOfV);
			if (rspCode == SUCCESS)
			{
				if (lenOfV <= ECL_LENGTH_4F)
				{
					UT_Set_TagLength(lenOfV, glv_tag4F.Length);
					memcpy(glv_tag4F.Value, &etp_ptrData[lenOfT+lenOfL], lenOfV);

					etp_ptrData+=(lenOfT+lenOfL+lenOfV);	//Point to Next Tag
					lenOfData+=(lenOfT+lenOfL+lenOfV);

					flg4F=TRUE;

					rspCode=ETP_Check_ADFName();
					if (rspCode == FAIL)
					{
						etp_flgUnknowAID=TRUE;
						
						return FAIL;
					}
					
					if (lenOfData == lenOf61)	//End of 61
					{
						break;
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
		}	
		else if (!memcmp(etp_ptrData, tag50, 1))	//Tag 50: Application Lable
		{
			rspCode=UT_Get_TLVLength(etp_ptrData, &lenOfT, &lenOfL, &lenOfV);
			if (rspCode == SUCCESS)
			{
				if (!memcmp(glv_tag4F.Value, etp_aidJCB, 5))
				{
					//JCB CL 1.3 Update - Check If Tag 50 Length Error
					if (memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV], tag87, 1))
					{
						if (lenOfV == 0)
						{
							for (cntIndex=0; cntIndex < ECL_LENGTH_50; cntIndex++)
							{
								if (!memcmp(&etp_ptrData[lenOfT+lenOfL+cntIndex], tag87, 1))
								{
									lenOfV=cntIndex;
									flgLenError=TRUE;
									
									break;
								}
							}
						}
						else
						{
							if (!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV-1], tag87, 1))
							{
								lenOfV--;
								flgLenError=TRUE;
							}
							
							if (!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV+1], tag87, 1))
							{
								lenOfV++;
								flgLenError=TRUE;
							}
						}
					}
				}

				if (lenOfV <= ECL_LENGTH_50)
				{
					UT_Set_TagLength(lenOfV, glv_tag50.Length);
					memcpy(glv_tag50.Value, &etp_ptrData[lenOfT+lenOfL], lenOfV);
				}
				
				etp_ptrData+=(lenOfT+lenOfL+lenOfV);	//Point to Next Tag
				lenOfData+=(lenOfT+lenOfL+lenOfV);
				
				if (lenOfData == lenOf61)	//End of 61
				{
					break;
				}
			}
			else
			{
				return FAIL;
			}
		}
		else if (!memcmp(etp_ptrData, tag87, 1))	//Tag 87: Priority Indicator
		{
			rspCode=UT_Get_TLVLength(etp_ptrData, &lenOfT, &lenOfL, &lenOfV);
			if (rspCode == SUCCESS)
			{
				if (!memcmp(glv_tag4F.Value, etp_aidJCB, 5))
				{
					//JCB CL 1.3 Update - Check If Tag 87 Length Error
					if ((memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV], tag61, 1)) &&
						(memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV], tag9F2A, 2)) &&
						(memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV], sw9000, 2)))
					{
						if (lenOfV == 0)
						{
							for (cntIndex=0; cntIndex < 2; cntIndex++)
							{
								if ((!memcmp(&etp_ptrData[lenOfT+lenOfL+cntIndex], tag61, 1)) ||
									(!memcmp(&etp_ptrData[lenOfT+lenOfL+cntIndex], sw9000, 2)))
								{
									lenOfV=cntIndex;
									flgLenError=TRUE;
									break;
								}
							}
						}
						else
						{
							if ((!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV-1], tag61, 1)) ||
								(!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV-1], tag9F2A, 2)) ||
								(!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV-1], sw9000, 2)))
							{
								lenOfV--;
								flgLenError=TRUE;
							}
							
							if ((!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV+1], tag61, 1)) ||
								(!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV+1], tag9F2A, 2)) ||
								(!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV+1], sw9000, 2)))
							{
								lenOfV++;
								flgLenError=TRUE;
							}
						}
					}

					if (lenOfV > ECL_LENGTH_87)
					{
						flgLenError=TRUE;
					}
				}
				
				if ((lenOfV <= ECL_LENGTH_87) || (flgLenError == TRUE))
				{
					UT_Set_TagLength(1, glv_tag87.Length);
					glv_tag87.Value[0]=(flgLenError == TRUE)?(0):(etp_ptrData[lenOfT+lenOfL]);
					
					etp_ptrData+=(lenOfT+lenOfL+lenOfV);	//Point to Next Tag
					lenOfData+=(lenOfT+lenOfL+lenOfV);
					
					if (lenOfData == lenOf61)	//End of 61
					{
						break;
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
		}
		else if (!memcmp(etp_ptrData, tag9F2A, 2))	//Tag 9F2A: Kernel Identifier		
		{
			rspCode=UT_Get_TLVLength(etp_ptrData, &lenOfT, &lenOfL, &lenOfV);
			if (rspCode == SUCCESS)
			{
				if (!memcmp(glv_tag4F.Value, etp_aidJCB, 5))
				{
					//JCB CL 1.3 Update - Check If Tag 9F2A Length Error
					if ((memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV], tag61, 1)) &&
						(memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV], tag9F29, 2)) &&
						(memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV], sw9000, 2)))
					{
						if (lenOfV == 0)
						{
							for (cntIndex=0; cntIndex < ECL_LENGTH_9F2A; cntIndex++)
							{
								if ((!memcmp(&etp_ptrData[lenOfT+lenOfL+cntIndex], tag61, 1)) ||
									(!memcmp(&etp_ptrData[lenOfT+lenOfL+cntIndex], sw9000, 2)))
								{
									lenOfV=cntIndex;
									flgLenError=TRUE;
									break;
								}
							}
						}
						else
						{
							if ((!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV-1], tag61, 1)) ||
								(!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV-1], tag9F29, 2)) ||
								(!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV-1], sw9000, 2)))
							{
								lenOfV--;
								//flgLenError=TRUE;
							}
							
							if ((!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV+1], tag61, 1)) ||
								(!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV+1], tag9F29, 2)) ||
								(!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV+1], sw9000, 2)))
							{
								lenOfV++;
								//flgLenError=TRUE;
							}
						}
					}
				}
				
				if (lenOfV <= ECL_LENGTH_9F2A)
				{
					UT_Set_TagLength(lenOfV, glv_tag9F2A.Length);					
					memcpy(glv_tag9F2A.Value, &etp_ptrData[lenOfT+lenOfL], lenOfV);
					
					etp_ptrData+=(lenOfT+lenOfL+lenOfV);	//Point to Next Tag
					lenOfData+=(lenOfT+lenOfL+lenOfV);
					
					if (lenOfData == lenOf61)	//End of 61
					{
						break;
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
		}
		else if (!memcmp(etp_ptrData, tag9F29, 2)) 	//Tag 9F29: Extended Selection
		{
			rspCode=UT_Get_TLVLength(etp_ptrData, &lenOfT, &lenOfL, &lenOfV);
			if (rspCode == SUCCESS)
			{
				if (!memcmp(glv_tag4F.Value, etp_aidJCB, 5))
				{
					//JCB CL 1.3 Update - Check If Tag 9F29 Length Error
					if ((memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV], tag61, 1)) &&
						(memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV], sw9000, 2)))
					{
						if (lenOfV == 0)
						{
							for (cntIndex=0; cntIndex < 255; cntIndex++)
							{
								if ((!memcmp(&etp_ptrData[lenOfT+lenOfL+cntIndex], tag61, 1)) ||
									(!memcmp(&etp_ptrData[lenOfT+lenOfL+cntIndex], sw9000, 2)))
								{
									lenOfV=cntIndex;
									flgLenError=TRUE;
									break;
								}
							}
						}
						else
						{
							if ((!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV-1], tag61, 1)) ||
								(!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV-1], sw9000, 2)))
							{
								lenOfV--;
								flgLenError=TRUE;
							}
							
							if ((!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV+1], tag61, 1)) ||
								(!memcmp(&etp_ptrData[lenOfT+lenOfL+lenOfV+1], sw9000, 2)))
							{
								lenOfV++;
								flgLenError=TRUE;
							}
						}
					}
				}
				
				if (lenOfV <= ECL_LENGTH_9F29)
				{
					UT_Set_TagLength(lenOfV, glv_tag9F29.Length);
					memcpy(glv_tag9F29.Value, &etp_ptrData[lenOfT+lenOfL], lenOfV);
					
					etp_ptrData+=(lenOfT+lenOfL+lenOfV);	//Point to Next Tag
					lenOfData+=(lenOfT+lenOfL+lenOfV);
					
					if (lenOfData == lenOf61)	//End of 61
					{
						break;
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
		}
		else	//Unexpected Tag
		{
			//Check Padding
			if ((etp_ptrData[0] == 0x00) || (etp_ptrData[0] == 0xFF))
			{
				UT_Check_Padding(lenOf61, lenOfData, etp_ptrData, &lenOfPadding);
				lenOfData+=lenOfPadding;

				if (lenOfData == lenOf61)	//End of 61
				{
					break;
				}
			}
			else
			{
				rspCode=UT_Get_TLVLength(etp_ptrData, &lenOfT, &lenOfL, &lenOfV);
				if (rspCode == SUCCESS)
				{
					etp_ptrData+=(lenOfT+lenOfL+lenOfV);	//Point to Next Tag
					lenOfData+=(lenOfT+lenOfL+lenOfV);

					if (lenOfData == lenOf61)	//End of 61
					{
						break;
					}
				}
				else
				{
					return FAIL;
				}
			}
		}
	}

	if (flg4F == TRUE)
	{
		return SUCCESS;
	}
	
	return FAIL;
}


void	ETP_Patch_SelAID(UCHAR *datLen, UCHAR *datBuff)
{
	UCHAR 	cmdSelect[4]={0x00,0xA4,0x04,0x00};
	
	memcpy(datBuff, cmdSelect, 4);

	memcpy(&datBuff[5], etp_cddList[etp_cddIndex].adfName, etp_cddList[etp_cddIndex].adfLen);

	//0514ICTK add the AID to Tag9F06
	glv_tag9F06.Length[0] = etp_cddList[etp_cddIndex].adfLen;
	memcpy(glv_tag9F06.Value,etp_cddList[etp_cddIndex].adfName, etp_cddList[etp_cddIndex].adfLen);
	//0514ICTK add the AID to Tag9F06 end

	datLen[0]=5+etp_cddList[etp_cddIndex].adfLen;
	
	if (etp_cddList[etp_cddIndex].extLen != 0)
	{
		memcpy(&datBuff[datLen[0]], etp_cddList[etp_cddIndex].extSelection, etp_cddList[etp_cddIndex].extLen);		
		datLen[0]+=etp_cddList[etp_cddIndex].extLen;
	}

	datBuff[datLen[0]]=0;
	datLen[0]+=1;

	datBuff[4]=datLen[0]-6;	//Select Header(4) + Lc(1) + Le(1)
}


UCHAR ETP_Select_PPSE(void)
{
	UCHAR	rspCode=0;
	UCHAR 	cmdPPSE[20]={	
				0x00,	//CLA
				0xA4,	//INS
				0x04,	//P1
				0x00,	//P2
				0x0E,	//Length of Data Field
				'2', 'P', 'A', 'Y', '.', 'S', 'Y', 'S', '.', 'D', 'D', 'F', '0', '1',
				0x00};	//Le
	
	ETP_Clear_Buffer();

	rspCode=ECL_LV1_DEP(20, cmdPPSE, &etp_rcvLen, etp_rcvData, 1000);

	//DE: recording 
	MPP_DataExchangeLog((UCHAR*)&"SelPPSE", 7);
	MPP_DataExchangeLog(cmdPPSE, 20);
	MPP_DataExchangeLog((UCHAR*)&"Res-SelPPSE", 11);
	MPP_DataExchangeLog(etp_rcvData, etp_rcvLen);

	return rspCode;
}


UCHAR ETP_Select_AID(UCHAR iptLen, UCHAR *iptData)
{
	UCHAR	rspCode=0;
	
	ETP_Clear_Buffer();

	rspCode=ECL_LV1_DEP((UINT)iptLen, iptData, &etp_rcvLen, etp_rcvData, 1000);

	//DE: recording 
	MPP_DataExchangeLog((UCHAR*)&"SelAID", 6);
	MPP_DataExchangeLog(iptData, (UINT)iptLen);
	MPP_DataExchangeLog((UCHAR*)&"Res-SelAID", 10);
	MPP_DataExchangeLog(etp_rcvData, etp_rcvLen);

	return rspCode;
}


UCHAR ETP_Select_AIDFromCandidateList(void)
{
	UCHAR	rspCode=0;
	UCHAR	selLen=0;
	UCHAR	selCmd[50]={0};
	
	ETP_Patch_SelAID(&selLen, selCmd);
	
	rspCode=ETP_Select_AID(selLen, selCmd);

	return rspCode;
}


void ETP_Select_AIDFromSupportList(void)
{
	UCHAR	idxNumber=0;
	UCHAR	idxTxnType=0;
	UCHAR	rspCode=FAIL;
	UCHAR	*ptrData_84=NULLPTR;
	UCHAR	*ptrData_87=NULLPTR;
	UCHAR	flgNext=FALSE;
	UCHAR	tag84[1]={0x84};
	UCHAR	tag87[1]={0x87};
	UCHAR	lenOfT_84=0;
	UCHAR	lenOfL_84=0;
	UINT	lenOfV_84=0;
	UCHAR	lenOfT_87=0;
	UCHAR	lenOfL_87=0;
	UINT	lenOfV_87=0;
	UCHAR	lenSelect=0;
	UCHAR	priIndicator=0;
	UCHAR 	cmdSelect[4+1+16+1]={0x00,0xA4,0x04,0x00};	//Select APDU + Lc + CMD +Le
	CDDLIST	tmpCDDList={0,{0},0,{0},0,0,{0},0};

	
	idxTxnType=ETP_Get_TransactionTypeIndex();
	
	for (idxNumber=0; idxNumber < ETP_NUMBER_COMBINATION; idxNumber++)
	{
		if (memcmp(etp_cmbTable[idxTxnType][idxNumber].cmbAID, etp_aidNA, 7))
		{
			lenSelect=4+1+etp_cmbTable[idxTxnType][idxNumber].aidLen+1;
			cmdSelect[4]=lenSelect;
			memcpy(&cmdSelect[5], etp_cmbTable[idxTxnType][idxNumber].cmbAID, lenSelect);
			cmdSelect[5+lenSelect]=0x00;

			while (1)
			{
				if (flgNext == TRUE)
				{
					cmdSelect[3]=0x02;	//Select Next
				}
				
				rspCode=ETP_Select_AID(lenSelect, cmdSelect);
				if (rspCode == ECL_LV1_SUCCESS)
				{
					rspCode=UT_Check_SW12(&etp_rcvData[etp_rcvLen-2], STATUSWORD_9000);
					if (rspCode == TRUE)
					{
						rspCode=ETP_Check_FCI();
						if (rspCode == SUCCESS)
						{
							ptrData_84=UT_Find_Tag(tag84, (etp_rcvLen-2), etp_rcvData);
							if (ptrData_84 != NULLPTR)
							{
								UT_Get_TLVLength(ptrData_84, &lenOfT_84, &lenOfL_84, &lenOfV_84);
								
								priIndicator=0;
								ptrData_87=UT_Find_Tag(tag87, (etp_rcvLen-2), etp_rcvData);
								if (ptrData_87 != NULLPTR)
								{
									UT_Get_TLVLength(ptrData_87, &lenOfT_87, &lenOfL_87, &lenOfV_87);

									if (lenOfV_87 > ECL_LENGTH_87)
									{
										break;
									}
									
									priIndicator=(lenOfV_87 == ECL_LENGTH_87)?(ptrData_87[lenOfT_87+lenOfL_87]):(0);
								}

								if (!memcmp(etp_cmbTable[idxTxnType][idxNumber].cmbAID, &ptrData_84[lenOfT_84+lenOfL_84], etp_cmbTable[idxTxnType][idxNumber].aidLen))
								{
									tmpCDDList.adfLen=(UCHAR)lenOfV_84;
									memcpy(tmpCDDList.adfName, &ptrData_84[lenOfT_84+lenOfL_84], lenOfL_84);
									tmpCDDList.krnID[0]=(idxNumber+1);
									tmpCDDList.appPriority=priIndicator;
									ETP_Add_CandidateList(&tmpCDDList);
									
									flgNext=TRUE;

									continue;
								}
							}
						}
					}
				}
				else 
				{
					if (rspCode == ECL_LV1_TIMEOUT_USER)
						etp_flgCmdTimeout=TRUE;
					
					etp_depRspCode=rspCode;

					etp_Outcome.Start=ETP_OCP_Start_B;
					etp_flgComError=TRUE;

					ECL_LV1_RESET();
				}

				break;
			}
		}
	}
}


void ETP_Update_Timer(ULONG iptTime1, ULONG iptTime2, ULONG * tmrValue)
{
	ULONG	ticValue;

	ticValue=iptTime2-iptTime1;
	tmrValue[0]=(tmrValue[0] >= ticValue)?(tmrValue[0]-ticValue):(0);
}


UCHAR ETP_Match_PartialAID(void)
{
	UCHAR rspCode=0xFF;
	UCHAR cmbIndex=0xFF;
	UCHAR typIndex=0;
	UINT  lenOf4F=0;
	UCHAR cmpTable[ETP_NUMBER_COMBINATION]={0};
	UCHAR idxCompare=0;
	UCHAR flgParMatch=FALSE;

	typIndex=ETP_Get_TransactionTypeIndex();
	
	UT_Get_TLVLengthOfV(glv_tag4F.Length, &lenOf4F);

	for (cmbIndex=0; cmbIndex<ETP_NUMBER_COMBINATION; cmbIndex++)
	{
		//ADF Starts with Combination Table AID
		if (lenOf4F >= etp_cmbTable[typIndex][cmbIndex].aidLen)
		{
			if (!memcmp(glv_tag4F.Value, etp_cmbTable[typIndex][cmbIndex].cmbAID, etp_cmbTable[typIndex][cmbIndex].aidLen))
			{
				//Match AID Number
				for (idxCompare=0; idxCompare < ECL_LENGTH_4F; idxCompare++)
				{
					if (glv_tag4F.Value[idxCompare] != etp_cmbTable[typIndex][cmbIndex].cmbAID[idxCompare])
					{
						break;
					}
				}

				//Save Result to cmpTable
				cmpTable[cmbIndex]=idxCompare+1;

				flgParMatch=TRUE;
			}
		}
	}

	//Find Most Likely AID Index for Partial Match
	if (flgParMatch == TRUE)
	{
		idxCompare=0;
		
		for (cmbIndex=1; cmbIndex<ETP_NUMBER_COMBINATION; cmbIndex++)
		{
			if (cmpTable[idxCompare] < cmpTable[cmbIndex])
			{
				idxCompare=cmbIndex;
			}
		}

		rspCode=idxCompare;
	}

	return rspCode;
}


UCHAR ETP_Outcome_Processing(void)
{
	//[Book B Req: 3.5.1.1] UI Request on Outcome Present is Yes
	if (etp_Outcome.rqtOutcome == ETP_OCP_UIOnOutcomePresent_Yes)
	{	
		ETP_UI_Request(etp_Outcome.ocmMsgID, etp_Outcome.ocmStatus);
		UT_WaitTime(etp_Outcome.hldTime * 10);
		
		etp_Outcome.rqtOutcome = ETP_OCP_UIOnOutcomePresent_No;	//20140117 V1, Reset
	}
	
	//[Book B Req: 3.5.1.2] Field Off Request
	if (etp_Outcome.filOffRequest != ETP_OCP_FieldOffRequest_NA)
	{
		ECL_LV1_FIELD_OFF();
		UT_WaitTime(etp_Outcome.filOffRequest * 10);

		etp_Outcome.filOffRequest = ETP_OCP_FieldOffRequest_NA; //20140117 V1, Reset
	}

	//[Book B Req: 3.5.1.3] Entry Point shall return to Start B when Outcom is Try Again
	if (etp_Outcome.Start == ETP_OCP_Start_B)
	{
		if ((etp_flgComError == TRUE) || (etp_flgRestart == TRUE) || (etp_flgCDCVM == TRUE))
		{
			//JCB CL 1.3 Update - Reset Tag Before Start B
			ECL_Reset_Tag();
			ETP_Load_TransactionData();
			
			return FAIL;
		}
	}
	
	//[Book B Req: 3.5.1.4] Entry Point shall return to Start C when Outcom is Select Next
	if (etp_Outcome.Start == ETP_OCP_Start_C)
	{
		return FAIL;
	}

	//[Book B Req: 3.5.1.5] Entry Point shall Provide the Outcome to the reader as a Final Outcome

	//0516, ICTK issuer Update
	//if((etp_Outcome.Start == ETP_OCP_Start_B) && (Opt_qVSDC_ISSUER_Update == TRUE))
	//{
	//	return FAIL;
	//}
	//0516, ICTK issuer Update end

	return SUCCESS;
}

// Add by Wayne for debug

extern UCHAR JCB_Out_Status;

void ETP_D_Kernel_Activation(void)
{
	struct timeval nowtime;
	nowtime=GetNowTime();
	printf("!!ETP_D_Kernel_Activation: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec); 
DBG_Put_Text("D");
	//Get Kernel Configuration Data Index
	etp_krnCfgIndex=ETP_Get_KernelConfigurationIndex();
	nowtime=GetNowTime();
	printf("ETP_Get_KernelConfigurationIndex End: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec);
DBG_Put_UCHAR(etp_cddList[etp_cddIndex].krnID[0]);
//	Kernel Processing
	//printf("(--------)Break point:%s:line%d(---------)\n",__func__,__LINE__);
	//printf("etp_cddList[etp_cddIndex].krnID[0]:%x\n",etp_cddList[etp_cddIndex].krnID[0]);
	nowtime=GetNowTime();
	printf("MPP_Start_Kernel Start: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec);
	switch (etp_cddList[etp_cddIndex].krnID[0])
	{
//		case ETP_KID_VISAAP:	VAP_Start_Kernel(etp_rcvLen, etp_rcvData);		break;
		case ETP_KID_MASTER:	MPP_Start_Kernel(etp_rcvLen, etp_rcvData);		break;
		case ETP_KID_VISA:		VGL_Start_Kernel(etp_rcvLen, etp_rcvData);		break;
		case ETP_KID_AMEX:		AEX_Start_Kernel(etp_rcvLen, etp_rcvData);		break;
		case ETP_KID_JCB:		JCB_Start_Kernel(etp_rcvLen, etp_rcvData);		break;
		case ETP_KID_DISCOVER:	DPS_Start_Kernel(etp_rcvLen, etp_rcvData);		break;
		case ETP_KID_UPI:		UNP_Start_Kernel(etp_rcvLen, etp_rcvData);		break;
	}
	nowtime=GetNowTime();
	printf("MPP_Start_Kernel End: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec);
}

void ETP_C_FinalSelection(void)
{
	UCHAR	flgRmvCddList=FALSE;
	UCHAR	flgNotAllowed=FALSE;
	UCHAR	rspCode=0;
	UCHAR	rspChkSW=0;
	UCHAR	rspChkUPI=FALSE;
	UCHAR	*ptrPDOL=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	tag84[1]={0x84};
	UCHAR	tag9F38[2]={0x9F,0x38};
	UCHAR	tag9F66[2]={0x9F,0x66};
	UCHAR	*ptrDF=NULLPTR;
	UCHAR	*ptrTTQ=NULLPTR;
	
DBG_Put_Text("CF");
	//20131202
	UCHAR 	flg_missPDOL = FALSE;
	
	//DRL ID
	PREDRLLIMITSET DRLID = {{0},0};

	//VCPS 2.1.3 Update - Reset DrlNotAllow Flag
	etp_flgDrlNotAllow=FALSE;
	
	if (etp_flgRestart == FALSE)
	{
		//[Book B Req: 3.3.3.1] If there is only one Combination in the Candidate List, Entry Point shall select the Combination

		//[Book B Req: 3.3.3.2] Sort Priority when there are multiple Combinations in the Candidate List
		if (etp_cddNumber != 1)
		{
			ETP_Sort_CandidateList();
		}

		//Reset Candidate List Index
		etp_cddIndex=0;	
	}

	//[Book B Req: 3.3.3.3] Check Extended Selection
	ETP_Check_ExtendedAID();

	//
	//	Select AID
	//
	//[Book B Req: 3.3.3.4] Select AID
	rspCode=ETP_Select_AIDFromCandidateList();
	if (rspCode == ECL_LV1_SUCCESS)
	{
		if(etp_cddList[etp_cddIndex].krnID[0] == ETP_KID_JCB)
		{
			rspCode=ETP_Check_JCB_FCI();	//check AID with JCB Rule
			if(rspCode == SUCCESS)
			{
				rspChkSW=UT_Check_SW12(&etp_rcvData[etp_rcvLen-2], STATUSWORD_9000);
				if(rspChkSW == SUCCESS)
				{
					//Check PDOL
					ptrPDOL=UT_Find_Tag(tag9F38, (etp_rcvLen-2), etp_rcvData);
					if (ptrPDOL != NULLPTR)
					{
						ETP_Copy_PDOL(ptrPDOL);
					}

					//Copy DF Name
					ptrDF=UT_Find_Tag(tag84, (etp_rcvLen-2), etp_rcvData);
					if (ptrDF != NULLPTR)
					{
						ETP_Copy_DFName(ptrDF);
					}
				}
				else
				{
					//JCB CL 1.3 Update - Requirement 4.3.4.2
					//Restart Flag should be 0
					if (etp_flgRestart == FALSE)
				    {
						flgRmvCddList=TRUE;
				    }
				    else
				    {
						ETP_Set_DefaultOutcome(ETP_OUTCOME_EndApplication);
						etp_flgIsuUpdateErr=TRUE;
				    }		
				}				
			}
			else
			{
				flgRmvCddList=TRUE;
			}
		}
		//Tammy 2017/07/25 [DISCOVER]
		else if(etp_cddList[etp_cddIndex].krnID[0] == ETP_KID_DISCOVER)
		{
			rspChkSW = UT_Check_SW12(&etp_rcvData[etp_rcvLen-2], STATUSWORD_9000);
			if(rspChkSW == SUCCESS)
			{
				//Check which one AID is selected.
				if(!memcmp(glv_tag9F06.Value, etp_aidDPS, 5))	//Select D-PAS application
				{
					rspCode = ETP_Check_DPAS_FCI();

//					DBG_Put_String(8, (UCHAR*)"DPAS Chk");	// ==== [Debug] ====
//					DBG_Put_UCHAR(rspCode);	// ==== [Debug] ====

					if(rspCode == SUCCESS)
					{
						//Check PDOL
//						ptrPDOL=apk_EMVCL_FindTag(0x9F, 0x38, etp_rcvData);
//						if(ptrPDOL != 0)
						ptrPDOL=UT_Find_Tag(tag9F38, (etp_rcvLen-2), etp_rcvData);
						if (ptrPDOL != NULLPTR)
						{
							ptrPDOL+=2;	//Point to L
							UT_Get_TLVLengthOfL(ptrPDOL, &lenOfL);
							UT_Get_TLVLengthOfV(ptrPDOL, &lenOfV);
							UT_Set_TagLength(lenOfV, glv_tag9F38.Length);
							memcpy(glv_tag9F38.Value, &ptrPDOL[lenOfL], lenOfV);

							rspCode = ETP_Check_DPAS_PDOL();
							if(rspCode == FAIL)
							{
//								DBG_Put_Process(DPS_FIGURE_10, 9, 0xFF);

								flgRmvCddList = TRUE;
							}
						}

						//Copy DF Name
						ptrDF=UT_Find_Tag(tag84, (etp_rcvLen-2), etp_rcvData);
						if (ptrDF != NULLPTR)
						{
							ETP_Copy_DFName(ptrDF);
						}
					}
					else
					{
						etp_flgComError = TRUE;	//use for terminate or try another interface
					}
				}				
			}
			else
			{
				//When second tap AID is not correct, end the transaciton
				if (etp_flgRestart == TRUE)
				{
					etp_flgIsuUpdateErr = TRUE;

					//reset etp_outcome
					etp_Outcome.rqtOutcome = ETP_OCP_UIOnOutcomePresent_No;
					etp_Outcome.Start = ETP_OCP_Start_NA;
				}
				else
				{
					flgRmvCddList=TRUE;
				}		
			}	
		}
		else
		{			
			rspCode=ETP_Check_FCI();
			if (rspCode == SUCCESS)
			{
				//[Book B Req: 3.3.3.5] Check Status Word
				rspChkSW=UT_Check_SW12(&etp_rcvData[etp_rcvLen-2], STATUSWORD_9000);
				if (rspChkSW == TRUE)
				{
					if ((glv_parFlgDRL == TRUE) && 
						(glv_parFlgqVSDC == TRUE) &&
						(etp_cddList[etp_cddIndex].krnID[0] == ETP_KID_VISA))
					{
						//VGL_Dynamic_Reader_Limit();
						rspCode = VGL_Dynamic_Reader_Limit(&DRLID);
						if(rspCode == SUCCESS)
						{
							//replace DRL TTQ to etp_preIndicator TTQ
							memcpy(etp_preIndicator[2].cpyTTQ,DRLID.DRL_TTQ,4);

							//if DRL Application Not allow == 1, Reader remove the APP from candidate list
							if(DRLID.DRL_CLAP_Not_Allowed == TRUE)
							{
								//VCPS 2.1.3 Update - Enable etp_flgDrlNotAllow
								flgNotAllowed = TRUE;

								etp_flgDrlNotAllow=TRUE;
							}
						}
						else	//0621 ICTK, Load default Value
						{
							if (etp_preIndicator[2].notAllowed == TRUE)
							{
								//VCPS 2.1.3 Update - Enable etp_flgDrlNotAllow
								flgNotAllowed=TRUE;
								flgRmvCddList=TRUE;

								etp_flgDrlNotAllow=TRUE;
							}
						}
					}

					if (flgNotAllowed == FALSE)
					{
						//Check PDOL
						ptrPDOL=UT_Find_Tag(tag9F38, (etp_rcvLen-2), etp_rcvData);
						if(ptrPDOL != NULLPTR)
						{
							ETP_Copy_PDOL(ptrPDOL);
						}

						//[Book B Req: 3.3.3.6] Check PDOL for VISA
						rspCode=ETP_Check_VISA_AID();
						rspChkUPI=ETP_Check_UPI_AID();
						if ((rspCode == SUCCESS) || (rspChkUPI == SUCCESS))
						{
							if (ptrPDOL != NULLPTR)
							{
								//Copy PDOL
								ETP_Copy_PDOL(ptrPDOL);

								//Check TTQ Present
								ptrTTQ=UT_Find_TagInDOL(tag9F66, lenOfV, glv_tag9F38.Value);
							}
							
							//If VISA AID PDOL Absent or Missing Tag 9F66 in PDOL, Entry Point Should Change to JCB Kernel
							if ((ptrPDOL == NULLPTR) || (ptrTTQ == NULLPTR))
							{
								if (rspChkUPI == SUCCESS)
								{
									flgRmvCddList=TRUE;
								}
								else
								{
									if(!glv_par_WAVE2_Enable)		//wave 3
									{
										flgRmvCddList=TRUE;
									}
									else							//wave 2
									{								
										//Check JCB Kernel Support
										if (etp_preIndicator[0].notAllowed == FALSE)
										{
											etp_cddList[etp_cddIndex].krnID[0]=ETP_KID_VISAAP;	
										}
										else
										{
											flgRmvCddList=TRUE;
										}
									}
								}

								//20131202 for MSD only mode case CLMD.006.0001
								flg_missPDOL = TRUE;
							}

							if (rspChkUPI == SUCCESS)
							{
								ptrDF=UT_Find_Tag(tag84, (etp_rcvLen-2), etp_rcvData);
								if (ptrDF == NULLPTR)
								{
									flgRmvCddList=TRUE;
								}
							}
						}

						//Copy Current TTQ to Tag 9F66
						ETP_Copy_TTQ();

						ptrDF=UT_Find_Tag(tag84, (etp_rcvLen-2), etp_rcvData);
						if (ptrDF != NULLPTR)
						{
							ETP_Copy_DFName(ptrDF);
						}
					}
					else	//20140112  if DRLID.DRL_CLAP_Not_Allowed = true , remove candidate list
					{
						flgRmvCddList=TRUE;
					}
				}
				else
				{
					//2013 0624 ICTK, issuer update, when second tap AID is not correct, go back to "start B" 
					//and show "Re-present Correct card", and wait for timeout
				    if (etp_flgRestart == TRUE)
				    {
				    	etp_flgIsuUpdateErr = TRUE;
#ifndef _PLATFORM_AS210
						UT_ClearRow(1,2,FONT0);
						UT_PutMsg(1,0,FONT0,23,(UCHAR *)"Re-present Correct Card");
						UT_WaitTime(100);
#endif
						//reset etp_outcome
						etp_Outcome.rqtOutcome = ETP_OCP_UIOnOutcomePresent_No;
						//return to start B
						etp_Outcome.Start = ETP_OCP_Start_B;
				    }
				    else
				    {
				    	flgRmvCddList=TRUE;
				    }	
				}
			}
			else
			{
				flgRmvCddList=TRUE;
			}
		}		

		if (flgRmvCddList == TRUE)
		{
			//if(!glv_parFlgqVSDC)		//MSD Only mode
			if((!glv_parFlgqVSDC) && (flg_missPDOL))	//MSD Only mode and miss PDOL	20131202 for case CLMD.006.0001 
			{
				while(etp_cddNumber)
				{
					ETP_Remove_CandidateList();
				}
			}
			else					//qVSDC & MSD Both mode or qVSDC Only Mode
			{
				//Remove Candidate List
				ETP_Remove_CandidateList();
			}
			
			ETP_C_Step3();
		}
	}
	else if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
	{
		//[Book B Req: 3.3.3.7] Entry Point shall return to Start B when communications error
		etp_Outcome.Start=ETP_OCP_Start_B;
		etp_flgComError=TRUE;
		etp_flgCmdTimeout=TRUE;

		//for select AID timeout
		ETP_Remove_CandidateList();
		ETP_Set_DefaultOutcome(ETP_OUTCOME_Timeout);

		ECL_LV1_RESET();
	}
#ifdef _PLATFORM_AS350
	else if ((rspCode == ECL_LV1_STOP_ICCARD) || (rspCode == ECL_LV1_STOP_MAGSTRIPE))
	{
		//[Book B Req: 3.3.3.7] Entry Point shall return to Start B when communications error
		etp_flgComError=TRUE;

		L3_Response_Code = (rspCode == ECL_LV1_STOP_ICCARD)?(VAP_RIF_RC_INSERT):(VAP_RIF_RC_SWIPE);
		
		rspChkUPI=ETP_Check_UPI_AID();
		if (rspChkUPI == SUCCESS)
		{
			ETP_Set_DefaultOutcome(ETP_OUTCOME_EndApplication);
		}
		else
		{
			//for select AID timeout
			ETP_Remove_CandidateList();
			ETP_Set_DefaultOutcome(ETP_OUTCOME_Contact);
		}

		ECL_LV1_RESET();
	}
	else if (rspCode == ECL_LV1_STOP_CANCEL)
	{
		L3_Response_Code = VAP_RIF_RC_KEY_PAD;
		
		ETP_Set_DefaultOutcome(ETP_OUTCOME_EndApplication);

		ECL_LV1_RESET();
	}
#endif
	else
	{
		etp_depRspCode=rspCode;	

		//[Book B Req: 3.3.3.7] Entry Point shall return to Start B when communications error
		etp_Outcome.Start=ETP_OCP_Start_B;
		etp_flgComError=TRUE;

		ECL_LV1_RESET();
	}
}


void ETP_C_Step3(void)
{
DBG_Put_Text("C3");
	//[Book B Req: 3.3.2.6] Entry Point shall retain the Candidate List If the Candidate List contains at least one entry
	if (etp_cddNumber != 0)
	{
		ETP_C_FinalSelection();
	}
	else	//[Book B Req: 3.3.2.7] Set End Application Outcome when Candidate List is Empty
	{
		//0628 ICTK select PPSE timeout
		if(etp_flgComError && etp_flgCmdTimeout)
			ETP_Set_DefaultOutcome(ETP_OUTCOME_Timeout);
		else if(etp_flgComError && (!etp_flgCmdTimeout))
			ETP_Set_DefaultOutcome(ETP_OUTCOME_Timeout);
		else
		{
			ETP_Set_DefaultOutcome(ETP_OUTCOME_EndApplication);

			if (etp_flgDrlNotAllow == FALSE)
			{
				//VCPS 2.1.3 Update - Change UI to Terminate
				etp_Outcome.rqtOutcome=ETP_OCP_UIOnOutcomePresent_No;
				etp_Outcome.ocmMsgID=ETP_OCP_UIM_ProcessingError;
//				UT_PutMsg(1,0,FONT0,22,(UCHAR *)"Transaction terminated");
//				UT_WaitTime(100);
			}
		}
	}
}


UCHAR ETP_C_Step2_E(void)		//Add Candidate List
{
	UCHAR	flgAddCddList=FALSE;
	UINT	lenOfV=0;
	CDDLIST	tmpCDDList={0,{0},0,{0},0,0,{0},0};
DBG_Put_Text("C2E");
	//VISA qVSDC DRL Don't Check Not Allowed Flag
	if ((glv_parFlgqVSDC == TRUE) && (glv_parFlgDRL) && (etp_mchAIDResult == 2))
	{
		flgAddCddList=TRUE;
	}
	else
	{
		if (etp_preIndicator[etp_mchAIDResult].notAllowed == FALSE)
		{
			flgAddCddList=TRUE;
		}
	}
	
	if (flgAddCddList == TRUE)
	{
		//ADF Name
		UT_Get_TLVLengthOfV(glv_tag4F.Length, &lenOfV);
		tmpCDDList.adfLen=(UCHAR)lenOfV;
		memcpy(tmpCDDList.adfName, glv_tag4F.Value, lenOfV);

		//Kernel ID
		memcpy(tmpCDDList.krnID, etp_exmKIDResult, 3);

		//Application Priority Indicator 
		tmpCDDList.appPriority=glv_tag87.Value[0] & 0x0F;

		//Extended Selection
		UT_Get_TLVLengthOfV(glv_tag9F29.Length, &lenOfV);
		if (lenOfV != 0)
		{
			tmpCDDList.extLen=(UCHAR)lenOfV;
			memcpy(tmpCDDList.extSelection, glv_tag9F29.Value, lenOfV);
		}

		//Combination Index
		tmpCDDList.cmbIndex=etp_mchAIDResult;
		
		ETP_Add_CandidateList(&tmpCDDList);

		return SUCCESS;
	}	

	return FAIL;	//CL Application Not Allowed
}


UCHAR ETP_C_Step2_D(void)		//Check Support of KID
{
	UCHAR rspCode=FAIL;
	UCHAR typIndex=0;
DBG_Put_Text("C2D");
	typIndex=ETP_Get_TransactionTypeIndex();

	rspCode=ETP_Check_KIDSupport(etp_cmbTable[typIndex][etp_mchAIDResult].lstKernel[0], etp_exmKIDResult[0]);
	if (rspCode == SUCCESS)
	{
		return SUCCESS;
	}
	
	return FAIL;		
}


UCHAR ETP_C_Step2_C(void)		//Examine KID
{
	UINT	lenOfV=0;
DBG_Put_Text("C2C");
	memset(etp_exmKIDResult, 0, sizeof(etp_exmKIDResult));

	//Get KID Length
	UT_Get_TLVLengthOfV(glv_tag9F2A.Length, &lenOfV);
	
	if (lenOfV == 0)	//KID Absent
	{
		ETP_Set_DefaultKID();
	}
	else
	{
		if (((glv_tag9F2A.Value[0] & 0xC0)==0x00) || ((glv_tag9F2A.Value[0] & 0xC0)==0x40))
		{												
			etp_exmKIDResult[0]=(glv_tag9F2A.Value[0] & 0x3F);	//Set Short KID

			if (etp_exmKIDResult[0] == 0)
			{
				ETP_Set_DefaultKID();
			}
		}
		else
		{
//			if (lenOfV < 3)		//EMV CL
			if (lenOfV == 1)	//DPAS Accepts 9F2A = 2
			{
				return FAIL;
			}
			
			if ((glv_tag9F2A.Value[0] & 0x3F) != 0x00)
			{
				etp_exmKIDResult[0]=(glv_tag9F2A.Value[0] & 0x3F);	//Set Short KID
			}
			else
			{
				return FAIL;	//Out of EMV Spec. Scope
			}
		}
	}

	//For qUICS, ignore KID other than 7
	if (etp_mchAIDResult == 6)
	{
		if (etp_exmKIDResult[0] != 7)
		{
			ETP_Set_DefaultKID();
		}
	}
	
	return SUCCESS;
}


UCHAR ETP_C_Step2_B(void)		//Matching AID
{
	UCHAR cmbIndex=0;
	UCHAR typIndex=0;
	UCHAR flgParMatch=FALSE;	//Flag of Partial Match
	UCHAR flgFulMatch=FALSE;	//Flag of Fully Match
	UINT  lenOf4F=0;
DBG_Put_Text("C2B");
	typIndex=ETP_Get_TransactionTypeIndex();

	etp_mchAIDResult=0xFF;
	
	UT_Get_TLVLengthOfV(glv_tag4F.Length, &lenOf4F);

	for (cmbIndex=0; cmbIndex < ETP_NUMBER_COMBINATION; cmbIndex++)
	{
		//Match First 5 Byte (RID)
		if (!memcmp(glv_tag4F.Value, etp_cmbTable[typIndex][cmbIndex].cmbAID, 5))
		{
			//Set cmbIndex to Match AID Result
			etp_mchAIDResult=cmbIndex;
			
			if ((!memcmp(glv_tag4F.Value, etp_cmbTable[typIndex][cmbIndex].cmbAID, lenOf4F)) &&
				(lenOf4F == etp_cmbTable[typIndex][cmbIndex].aidLen))
			{
				flgFulMatch=TRUE;

				return SUCCESS;
			}
			else
			{
				flgParMatch=TRUE;
			}
		}
	}

	if (flgParMatch == TRUE)
	{
		//Partial Match Process for qUICS when Multiple AID is Enable
		if (((etp_flgMtiAID_UPI == TRUE) &&
			((etp_mchAIDResult == 6) || (etp_mchAIDResult == 7) || (etp_mchAIDResult == 8)))
			||
			((etp_flgMtiAID_Discover == TRUE) &&
			((etp_mchAIDResult == 5) || (etp_mchAIDResult == 9))))
		{
			etp_mchAIDResult=ETP_Match_PartialAID();
		}
		
		return SUCCESS;
	}
	
	return FAIL;
}


UCHAR ETP_C_Step2_A(void)		//Examine ADF Format
{
	UCHAR	*ptrData=NULLPTR;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;

	UCHAR rspCode=0;
DBG_Put_Text("C2A");
	ptrData=etp_ptrData;

	rspCode=ETP_Parse_DirectoryEntry();
	if (rspCode == FAIL)
	{
		//Point to Next FCI
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		etp_ptrData=ptrData+(lenOfT+lenOfL+lenOfV);
		
		return FAIL;
	}
	else if (rspCode == 0xFF)	//End of FCI
	{
		return 0xFF;
	}

	return SUCCESS;	
}

void ETP_C_Step2(void)
{
	UCHAR	rspCode=0;
	UCHAR	flgEndA=0;
	UCHAR	tag61[1]={0x61};
DBG_Put_Text("C2");
	//[Book B - Req. 3.3.2.4] Check Directory Entry
	etp_ptrData=UT_Find_Tag(tag61, (etp_rcvLen-2), etp_rcvData);
	if (etp_ptrData != NULLPTR)
	{
		//[Book B - Req. 3.3.2.5] Process the Directory Entries
		rspCode=ETP_Check_ApplicationNotAllowed();
		if (rspCode == SUCCESS)
		{
			while (1)
			{
				//Examine ADF
				rspCode=ETP_C_Step2_A();
				if 		(rspCode == FAIL)	continue;				//Error
				else if (rspCode == 0xFF)	{flgEndA=TRUE;break;}	//End of FCI
				
				//Matching AID
				rspCode=ETP_C_Step2_B();
				if (rspCode == FAIL) continue;

				//Examine KID
				rspCode=ETP_C_Step2_C();
				if (rspCode == FAIL) continue;

				//Check Kernel Support
				rspCode=ETP_C_Step2_D();
				if (rspCode == FAIL) continue;

				//Add Candidate List
				rspCode=ETP_C_Step2_E();
				if (rspCode == FAIL) continue;
			}

			if (flgEndA == TRUE)
			{
				if (etp_cddNumber == 0)
				{
					//20140117 V1, when ADF Length is 4 bytes, test case D.013.01.00, Response should be RC_Failure
					etp_checkADF_Fail = TRUE;
				}
			}
		}
	}
}


void ETP_C_Step1(void)
{
	UCHAR	rspCode=0;
DBG_Put_Text("C1");
	//
	//	Select PPSE
	//
	//[Book B - Req. 3.3.2.2] Select PPSE
	rspCode=ETP_Select_PPSE();
	if (rspCode == ECL_LV1_SUCCESS)
	{
		//[Book B - Req. 3.3.2.3] Check SW12
		rspCode=UT_Check_SW12(&etp_rcvData[etp_rcvLen-2], STATUSWORD_9000);
		if (rspCode == TRUE)
		{
			//Check FCI
			rspCode=ETP_Check_FCI();
			if (rspCode == SUCCESS)
			{
				rspCode=ETP_Check_FCIMandatoryTag();
				if (rspCode == SUCCESS)
				{
					ETP_C_Step2();
				}
			}
		}
		else
		{
			memcpy(etp_depSW, &etp_rcvData[etp_rcvLen-2], 2);

			rspCode=UT_Check_SW12(&etp_rcvData[etp_rcvLen-2], STATUSWORD_6A82);
			if (rspCode == TRUE)
			{
				ETP_Select_AIDFromSupportList();
			}
		}
	}
#ifdef _PLATFORM_AS350
	else if ((rspCode == ECL_LV1_STOP_CANCEL) || (rspCode == ECL_LV1_STOP_ICCARD) || (rspCode == ECL_LV1_STOP_MAGSTRIPE))
	{
		if (rspCode == ECL_LV1_STOP_CANCEL)
		{
			L3_Response_Code = VAP_RIF_RC_KEY_PAD;
		}
		else
		{
			L3_Response_Code = (rspCode == ECL_LV1_STOP_ICCARD)?(VAP_RIF_RC_INSERT):(VAP_RIF_RC_SWIPE);
		}

		ECL_LV1_RESET();
	}
#endif
	else 
	{
		if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
			etp_flgCmdTimeout=TRUE;
		
		etp_depRspCode=rspCode;

		//[Book B - Req. 3.3.3.7] Entry Point shall return to Start B when communications error
		etp_Outcome.Start=ETP_OCP_Start_B;
		etp_flgComError=TRUE;

		ECL_LV1_RESET();
	}
	
	ETP_C_Step3();
}


UCHAR ETP_C_CombinationSelection(void)
{
struct timeval nowtime;
	nowtime=GetNowTime();
	printf("!!ETP_C_CombinationSelection: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec); 
DBG_Put_Text("C");
	//[Book B - Req. 3.3.2.1] Check Start Point
	if (etp_Outcome.Start == ETP_OCP_Start_B)
	{
		if (etp_flgRestart == TRUE)
		{
			nowtime=GetNowTime();
			printf("ETP_C_FinalSelection Start: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec);
			ETP_C_FinalSelection();
			nowtime=GetNowTime();
			printf("ETP_C_FinalSelection End: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec); 
		}
		else
		{
			nowtime=GetNowTime();
			printf("ETP_C_Step1 0 Start: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec);
			ETP_C_Step1();
			nowtime=GetNowTime();
			printf("ETP_C_Step1 0 End: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec); 
		}

		if(etp_flgIsuUpdateErr)
			return FAIL;
		else 
			return SUCCESS;
	}
	else if (etp_Outcome.Start == ETP_OCP_Start_C)
	{
		nowtime=GetNowTime();
		printf("ETP_C_Step3 Start: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec); 
		ETP_C_Step3();
		nowtime=GetNowTime();
		printf("ETP_C_Step3 End: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec); 
		return SUCCESS;
	}
	else
	{
		nowtime=GetNowTime();
		printf("ETP_C_Step1 Start: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec); 
		ETP_C_Step1();
		nowtime=GetNowTime();
		printf("ETP_C_Step1 End: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec); 
		//ICTK 0627 For Select AID, Time out case
		if(etp_flgComError)
			return FAIL;
		else
			return SUCCESS;
	}
}


UCHAR ETP_B_ProtocolActivation(void)
{
	
	UCHAR	cmbIndex=0;	//Combination Index
	UCHAR	rspCode=0;
	ULONG	tmrStrOfColDetection;	//Start of Collision Detection
	UCHAR	flgCollision=FALSE;
	UCHAR	cntErrIntegrity=0;
	UCHAR	cntErrCollision=0;
	struct timeval nowtime;
	nowtime=GetNowTime();
	printf("!!ETP_B_ProtocolActivation: %d s %d us\n",nowtime.tv_sec,nowtime.tv_usec); 
DBG_Put_Text("B");
	//20130628 ICTK for select AID timeout
	etp_flgCmdTimeout = FALSE;

	//Reset Communication Error Flag
	etp_flgComError=FALSE;
 
	//[Book B - Req. 3.2.1.1] Reset Parameter
	if (etp_flgRestart == FALSE)
	{
		//Exclude Start B of Try Again
		if ((etp_Outcome.Start == ETP_OCP_Start_B) && (etp_flgAutorun == TRUE))
		{
			for (cmbIndex=0; cmbIndex < ETP_NUMBER_COMBINATION; cmbIndex++)
			{
				//Reset Pre-Processing Indicator
				memset(&etp_preIndicator[cmbIndex], 0, INDICATOR_LEN);

				//Copy TTQ
				if (etp_etpConfig[cmbIndex].pstTTQ == TRUE)
				{
					memcpy(etp_preIndicator[cmbIndex].cpyTTQ, etp_etpConfig[cmbIndex].TTQ, 4);
				}
			}
		}
 
		//Clear Candidate List
		ETP_Clear_CandidateList();
		 
	}
 
	//[Book B - Req. 3.2.1.2] Display User Interface Request
	if ((etp_flgRestart == TRUE) && (etp_Outcome.rqtRestart == TRUE))
	{
		 
		//Issue Update Display
		UT_LED_F_Off_S_On(IID_LED_GREEN,IID_LED_BLUE,2);
		 	
		//Issue Update Display end
		ETP_UI_Request(etp_Outcome.rstMsgID, etp_Outcome.rstStatus);	//Retained UI Request
	}
	else
	{	
		if(!etp_Polling_Restart)
		{
			 
			ETP_Display_AmountMessage();
		}
	}
 
	//
	//	Activate Card Processing
	//
	//[Book B - Req. 3.2.1.3] Polling
	while (1)
	{	
		if(!etp_Polling_Restart)			
			etp_Polling_Timer = etp_tmrSale + 99;	//for display reason, we add 99 millisecond to look like correctly 

	#ifdef PCD_PLATFORM_CLRC663
		rspCode=ECL_LV1_FIELD_ON_EMV();
	#else
		rspCode=ECL_LV1_FIELD_ON();
	#endif
		UT_WaitTime(5);

		rspCode=ETP_LV1_POLLING(&etp_Polling_Timer);
		printf("rspCode = %d\n",rspCode );
		if (rspCode == ECL_LV1_SUCCESS)
		{	
#ifdef _SCREEN_SIZE_128x64
			api_sys_backlight(0,(ULONG)(etp_Polling_Timer+1000));
#endif

			tmrStrOfColDetection=OS_GET_SysTimerFreeCnt();

			rspCode=ECL_LV1_COLLISION_DETECTION();
			if (rspCode == ECL_LV1_SUCCESS)
			{								
				rspCode=ECL_LV1_ACTIVATE();
				if (rspCode == ECL_LV1_SUCCESS)
				{
					etp_Polling_Restart = FALSE;
									
					main_card_remove = FALSE;
					//20131031 FOR Production, we change the display order
					//20130724 minus UI Display, the select ppse command timing down form 31ms to 8ms
					//UT_PutMsg(1,0,FONT0,10,(UCHAR *)"Processing");
					
#ifdef _SCREEN_SIZE_240x320
					UT_ClearRow(1,4,FONT1);
	#ifdef _PLATFORM_AS350_LITE
		#ifndef _ACQUIRER_CTCB
					UT_ClearRow(9, 4, FONT1);
		#endif
	#endif
#else					
					UT_ClearRow(0,4,FONT1);
#endif					

					//Blue oFF & Yellow On
#ifdef _PLATFORM_AS350_LITE
					UT_Set_LED(IID_LED_YELLOW);
#else
					UT_LED_F_Off_S_On(IID_LED_BLUE,IID_LED_YELLOW,TRUE);
#endif
					ETP_UI_Request(ETP_OCP_UIM_Processing,ETP_OCP_UIS_Processing);
					 
					return SUCCESS;
				}
				else
				{
					 
					etp_Polling_Restart = TRUE;
					((OS_GET_SysTimerFreeCnt()-tmrStrOfColDetection) > etp_Polling_Timer)?(etp_Polling_Timer=0):(etp_Polling_Timer-=(OS_GET_SysTimerFreeCnt()-tmrStrOfColDetection));
					
					//[Book B - Req. 3.3.3.7] Entry Point shall return to Start B when communications error
					etp_Outcome.Start=ETP_OCP_Start_B;
					etp_flgComError=TRUE;
 
					ECL_LV1_RESET();
					 
					return FAIL;
				}
			}
			else if (rspCode == ECL_LV1_COLLISION)
			{
				if (cntErrCollision < 2)
				{
					cntErrCollision++;
					etp_Polling_Restart = TRUE;
					((OS_GET_SysTimerFreeCnt()-tmrStrOfColDetection) > etp_Polling_Timer)?(etp_Polling_Timer=0):(etp_Polling_Timer-=(OS_GET_SysTimerFreeCnt()-tmrStrOfColDetection));
				}
				else
				{
					flgCollision=TRUE;
				}
			}
			else if (rspCode == ECL_LV1_ERROR_INTEGRITY)
			{
				if (cntErrIntegrity < 3)
				{
					cntErrIntegrity++;

					etp_Polling_Restart = TRUE;
					((OS_GET_SysTimerFreeCnt()-tmrStrOfColDetection) > etp_Polling_Timer)?(etp_Polling_Timer=0):(etp_Polling_Timer-=(OS_GET_SysTimerFreeCnt()-tmrStrOfColDetection));
				}
				else
				{
					flgCollision=TRUE;
				}
			}
			else
			{
				etp_Polling_Restart = TRUE;
				((OS_GET_SysTimerFreeCnt()-tmrStrOfColDetection) > etp_Polling_Timer)?(etp_Polling_Timer=0):(etp_Polling_Timer-=(OS_GET_SysTimerFreeCnt()-tmrStrOfColDetection));
			}
		}
		else if(rspCode == ECL_LV1_STOP_NOW)	//Receive Reset Command
		{
			if(etp_Polling_Restart)
				etp_Polling_Restart = FALSE;

#ifdef _SCREEN_SIZE_240x320
			UT_ClearRow(1,2,FONT1);
#else
			UT_ClearScreen();
#endif
			//20131216 receive a reset command while waiting the card present
			VAP_Reset_Command_Occured = TRUE;			

			//20140115 V2, add this flag to prevent Reset Issuer Update problem
			if(etp_flgRestart)
			{
				etp_flgRestart = FALSE;
				etp_flgIsuUpdateErr = TRUE;	//20140304, if Reset Issuer Update, etp_flgIsuUpateErr = True. 
			}

			etp_Outcome.rqtOutcome = ETP_OCP_UIOnOutcomePresent_No;	//20140304, if Reset Issuer Update , don't display any message
			
			if(etp_flgComError)
				etp_flgComError = FALSE;
			//20140115 V2, add this flag to prevent Reset Issuer Update problem end

#ifdef _PLATFORM_AS350
			L3_Response_Code = VAP_RIF_RC_AUX_INTERRUPTION;
#endif
			
			break;

		}
#ifdef _PLATFORM_AS350
		else if(rspCode == ECL_LV1_STOP_CANCEL)
		{
			memset((UCHAR *)&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));
			memset((UCHAR *)&JCB_Dynamic_Parameter,0x00,sizeof(JCB_Dynamic_Parameter));
			memset((UCHAR *)&etp_Outcome,0x00,sizeof(etp_Outcome));
			JCB_Out_Status = JCB_TxnR_EndApp;
			
			if(etp_Polling_Restart)
				etp_Polling_Restart = FALSE;
		
			//L3 Response Code
			L3_Response_Code = VAP_RIF_RC_KEY_PAD;

			break;
		}
		else if(rspCode == ECL_LV1_STOP_ICCARD)
		{
			if(etp_Polling_Restart)
				etp_Polling_Restart = FALSE;
		
			//L3 Response Code
			L3_Response_Code = VAP_RIF_RC_INSERT;

			break;
		}
		else if(rspCode == ECL_LV1_STOP_MAGSTRIPE)
		{
			if(etp_Polling_Restart)
				etp_Polling_Restart = FALSE;
		
			//L3 Response Code
			L3_Response_Code = VAP_RIF_RC_SWIPE;

			break;
		}
#endif	
		else if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
		{
			if(etp_Polling_Restart)
				etp_Polling_Restart = FALSE;
					
			//L3 Response Code
			L3_Response_Code = VAP_RIF_RC_NO_CARD;	
			
			//20130628 ICTK, when Issuer Update and second tap(the card didn't present)
			if(etp_flgRestart)
				etp_flgIsuUpdateErr = TRUE;
			
			//20130627 reset the etp_outcome to prevent unnecessary data present
			etp_Outcome.rqtOutcome = ETP_OCP_UIOnOutcomePresent_No;
						
			//0625, if (the ISU card response error occur &&  timeout) we reset etp_flgRestart to go to Initial screen
			if(etp_flgIsuUpdateErr)
				etp_flgRestart = FALSE;

			break;
		}
		else if (rspCode == ECL_LV1_FAIL)
		{
			flgCollision=TRUE;
		}

		//[Book B - Req. 3.2.1.4] Collision
		if (flgCollision == TRUE)
		{
			if(etp_Polling_Restart)
				etp_Polling_Restart = FALSE;

			//20140304, Issuer Update "More Cards"
			if(etp_flgRestart)
				etp_flgIsuUpdateErr = TRUE;

			etp_Outcome.rqtOutcome = ETP_OCP_UIOnOutcomePresent_No;
				
			if(etp_flgIsuUpdateErr)
				etp_flgRestart = FALSE;
			//20140304, Issuer Update "More Cards" end
			
			//L3 Response Code
			L3_Response_Code = VAP_RIF_RC_MORE_CARDS;	
		
			ECL_LV1_RESET();
			//20140113 change the display order after ready for sale
			break;
		}
	}
	
	return FAIL;
}

UCHAR ETP_A_PreProcessing(void)
{
	UCHAR	cmbIndex=0;			//Combination Index: [0]JCB  [1]MasterCard  [2]VISA  [3]AMEX
	UCHAR	flgNotAllowed=TRUE;	//Application Not Allowed Flag
	UCHAR	rspCode=0;			//Response of BCD Comparing
	UCHAR	zroAmount[6]={0x00,0x00,0x00,0x00,0x00,0x00};	//Zero Amount for Comparison
	UCHAR	oneAmount[6]={0x00,0x00,0x00,0x00,0x01,0x00};	//Single Unit of Currency
DBG_Put_Text("A");
 
	for (cmbIndex=0; cmbIndex < ETP_NUMBER_COMBINATION; cmbIndex++)
	{
		//[Book B - Req. 3.1.1.1] Reset Pre-Processing Indicators
		memset(&etp_preIndicator[cmbIndex], 0, INDICATOR_LEN);

		//Pre DRL
		if((cmbIndex == 2) && glv_parFlgqVSDC && glv_parFlgDRL)
		{
			VGL_DRL_Preprocess();
		}

		//20130629 ICTK, if MSD only mode, we won't take effect to TTQ byte2 bit8, Chap 5.2.4.2, Req5.16, also because MSD always go online
		if ((cmbIndex == 2) && ((etp_etpConfig[cmbIndex].TTQ[0] & 0xE0) == 0x80) && (!glv_par_WAVE2_Enable))	//ICTK VISA MSD Only Mode, chap 5.2.4.2, Req5.16
		{
			//Copy "TTQ" to "Copy of TTQ"
			memcpy(etp_preIndicator[cmbIndex].cpyTTQ, etp_etpConfig[cmbIndex].TTQ, 4);
			
			continue;
		}

		//[Book B - Req. 3.1.1.2] If TTQ is part of the configuration data for a Combination
		if (etp_etpConfig[cmbIndex].pstTTQ == TRUE)
		{
			//Copy "TTQ" to "Copy of TTQ"
			memcpy(etp_preIndicator[cmbIndex].cpyTTQ, etp_etpConfig[cmbIndex].TTQ, 4);

			//Set "Copy of TTQ" B2b7 & B2b8 = 0b
			etp_preIndicator[cmbIndex].cpyTTQ[1]&=0x3F;
		}

		//[Book B - Req. 3.1.1.3] Check Status Check Support
		if ((etp_etpConfig[cmbIndex].pstStaCheck == TRUE) && //Amount is A Single Unit of Currency
			(etp_etpConfig[cmbIndex].flgStaCheck == TRUE))			
		{
			//Amount is A Single Unit of Currency
			if (memcmp(glv_tag9F02.Value, oneAmount, 6) == 0)
			{
				etp_preIndicator[cmbIndex].staCheck=TRUE;
			}
		}

		//[Book B - Req. 3.1.1.4] Check Zero Amount
		if (memcmp(glv_tag9F02.Value, zroAmount, 6) == 0)
		{
			if ((etp_etpConfig[cmbIndex].pstZroAllowed == TRUE) &&
				(etp_etpConfig[cmbIndex].flgZroAllowed == FALSE))
			{
				etp_preIndicator[cmbIndex].notAllowed=TRUE;
				
				continue;
			}
			else
			{
				etp_preIndicator[cmbIndex].zroAmount=TRUE;
			}
		}

		//[Book B - Req. 3.1.1.5] Check Reader Contactless Transaction Limit 
		if (etp_etpConfig[cmbIndex].pstRdrCTL == TRUE)
		{
			//Transaction Amount >= Reader Contactless Transaction Limit
			rspCode=UT_bcdcmp(glv_tag9F02.Value, etp_etpConfig[cmbIndex].rdrCTL, 6);
			if ((rspCode == 1) || (rspCode == 0))
			{
				etp_preIndicator[cmbIndex].notAllowed=TRUE;
				
				continue;
			}
		}

		//[Req. 3.1.1.6] Check Reader Contactless Floor Limit
		if (etp_etpConfig[cmbIndex].pstRdrCFL == TRUE)
		{
			//VISA TA	: Transaction Amount >  Reader Contactless Floor Limit
			//Production: Transaction Amount >= Reader Contactless Floor Limit
			rspCode=UT_bcdcmp(glv_tag9F02.Value, etp_etpConfig[cmbIndex].rdrCFL, 6);
			if ((rspCode == 1) || (rspCode == 0))
			{
				etp_preIndicator[cmbIndex].rdrCFLExceeded=TRUE;
			}
		}

		//[Book B - Req. 3.1.1.7] Reader Contactless Floor Limit Not Present
		if (etp_etpConfig[cmbIndex].pstRdrCFL == FALSE)
		{
			if (etp_etpConfig[cmbIndex].pstTrmFL == TRUE)
			{
				//VISA TA	: Transaction Amount >  Terminal Floor Limit
				//Production: Transaction Amount >= Terminal Floor Limit
				rspCode=UT_bcdcmp(glv_tag9F02.Value, etp_etpConfig[cmbIndex].trmFL, 6);
				if ((rspCode == 1) || (rspCode == 0))
				{
					etp_preIndicator[cmbIndex].rdrCFLExceeded=TRUE;
				}
			}
			else	//For qUICS, If Terminal Floor Limit Absent, Require Online Cryptograms
			{
				if ((cmbIndex == 6) || (cmbIndex == 7) || (cmbIndex == 8))
				{
					etp_preIndicator[cmbIndex].cpyTTQ[1]|=ETP_TTQ_OnlineCryptogram_Required;
				}
			}
		}

		//[Book B - Req. 3.1.1.8] Check Reader CVM Required Limit
		if (etp_etpConfig[cmbIndex].pstRdrCRL == TRUE)
		{
			//Transaction Amount >= Reader CVM Required Limit
			rspCode=UT_bcdcmp(glv_tag9F02.Value, etp_etpConfig[cmbIndex].rdrCRL, 6);
			if ((rspCode == 1) || (rspCode == 0))
			{
				etp_preIndicator[cmbIndex].rdrCVMExceeded=TRUE;
			}
		}

		//[Book B - Req. 3.1.1.9] Set Online cryptogram required when Contactless Floor Limit Exceeded
		if (etp_preIndicator[cmbIndex].rdrCFLExceeded == TRUE)
		{
			etp_preIndicator[cmbIndex].cpyTTQ[1]|=ETP_TTQ_OnlineCryptogram_Required;
		}

		//[Book B - Req. 3.1.1.10] Set Online cryptogram required when Status Check Requested
		if (etp_preIndicator[cmbIndex].staCheck == TRUE)
		{
			etp_preIndicator[cmbIndex].cpyTTQ[1]|=ETP_TTQ_OnlineCryptogram_Required;
		}

		//[Book B - Req. 3.1.1.11] Configure Parameter when Amount is 0
		if (etp_preIndicator[cmbIndex].zroAmount == TRUE)
		{
			if ((etp_etpConfig[cmbIndex].TTQ[0] & 0x08) == ETP_TTQ_OnlineCapableReader)
			{
				if (cmbIndex == 2)	//VISA Global
				{
					if (glv_parDF06[cmbIndex][0] & 0x20)
					{
						//Option 1: Indicate Online Cryptogram Required
						etp_preIndicator[cmbIndex].cpyTTQ[1]|=ETP_TTQ_OnlineCryptogram_Required;
					}
					else
					{
						//Option 2: Contactless Application Not Allowed
						etp_preIndicator[cmbIndex].notAllowed=TRUE;
						
						continue;
					}
				}
				else
				{
					etp_preIndicator[cmbIndex].cpyTTQ[1]|=ETP_TTQ_OnlineCryptogram_Required;
				}
			}
			else
			{
				etp_preIndicator[cmbIndex].notAllowed=TRUE;
				
				continue;
			}
		}

		//[Book B - Req. 3.1.1.12] Set CVM required when CVM Exceeded
		if (etp_preIndicator[cmbIndex].rdrCVMExceeded == TRUE)
		{
			etp_preIndicator[cmbIndex].cpyTTQ[1]|=ETP_TTQ_CVM_Required;
		}
	}	
 
	//[Book B - Req. 3.1.1.13] Check Every Card Index with Application Not Allowed
	for (cmbIndex=0; cmbIndex < ETP_NUMBER_COMBINATION; cmbIndex++)
	{
		if (etp_preIndicator[cmbIndex].notAllowed == FALSE)
		{
			flgNotAllowed=FALSE;
			break;
		}
	}

	if(flgNotAllowed==TRUE)
	{
		if(glv_parFlgqVSDC && glv_parFlgDRL)
		{
			for(cmbIndex=0; cmbIndex < ETP_PARA_NUMBER_PID; cmbIndex++)
			{
				if(VGL_PREDRLLimitSet[cmbIndex].DRL_CLAP_Not_Allowed == FALSE)
				{
					flgNotAllowed=FALSE;
					break;
				}
			}
		}
	}
printf("flgNotAllowed =%d\n",flgNotAllowed);
	if (flgNotAllowed == FALSE)
	{
		return SUCCESS;
	}
	else
	{
		ETP_Set_DefaultOutcome(ETP_OUTCOME_TryAnotherInterface);
		etp_Outcome.ocmStatus=ETP_OCP_UIS_ProcessingError;
	}
 
	return FAIL;
}


void ETP_EntryPoint(void)
{
	UCHAR rspCode=0;
	UCHAR ocmResult=0;

 
	if (etp_flgRestart == FALSE)
	{
		//Reset Tag
		ECL_Reset_Tag();

		//Reset Entry Point Relevant Parameter
		ETP_Reset_Parameter();
		
		//Load Transaction Data Tag
		ETP_Load_TransactionData();

		//Initial Configuration
		ETP_Initialize_EntryPointConfiguration();

		aex_flgRestart=FALSE;
	}

	 
	if(!JCB_Issue_Update_Flag)	//0605 add
	{
		if ((etp_Outcome.Start == ETP_OCP_Start_NA) ||
			(etp_Outcome.Start == ETP_OCP_Start_A) ||
			(etp_Outcome.Start == ETP_OCP_Start_B))
		{
			ETP_Clear_CandidateList();
		}
	}
 

	//Allocate MPP DE Log
	rspCode=MPP_Allocate_DE_LogBuffer();
 
	while(1)
	{
		ETP_Get_UnpredictableNumber();


		printf("etp_Outcome.Start= %d\n",etp_Outcome.Start);
		switch (etp_Outcome.Start)
		{
			case ETP_OCP_Start_NA:
				

			case ETP_OCP_Start_A:	rspCode=ETP_A_PreProcessing(); 		if (rspCode == FAIL) break;

			case ETP_OCP_Start_B:	 rspCode=ETP_B_ProtocolActivation();	if (rspCode == FAIL) break;

			case ETP_OCP_Start_C:	 rspCode=ETP_C_CombinationSelection();	if (rspCode == FAIL) break;
			
			case ETP_OCP_Start_D:
 #ifdef CTBC_DEBUG_MODE
			return;
			#endif
				ETP_D_Kernel_Activation();

				 
				if(etp_flgCmdTimeout == TRUE)
				{
					etp_Outcome.Start=ETP_OCP_Start_B;
					etp_flgComError=TRUE;
					etp_flgCmdTimeout = FALSE;
				}
				
			default:
				break;
		}
 

		
		ocmResult=ETP_Outcome_Processing();
		printf("ocmResult=0x%x\n",ocmResult);
		if (ocmResult == SUCCESS)
			break;		

		//20140206 for PayPass transaction, if following condition occurs, process "EMV_Reset"
		if((etp_Outcome.Start == ETP_OCP_Start_B) && (glv_tagDF8129.Value[0] == MPP_OPS_STATUS_EndApplication))
			ECL_LV1_RESET();
 
#ifndef _PLATFORM_AS350
		//20140618 add for consumer payment device, we send RFS response with RC_Code "VAP_RIF_RC_TRY_AGAIN" 
		if (etp_flgCDCVM == TRUE)
		{
			VAP_RIF_TRA_ReadyForSales_TryAgain();
		}
#endif			
	}
 
	//20140112 removed
	//20130913
	//EMV_REMOVAL();
	//main_card_remove = TRUE; //20140128 removed, we detect card after "ready for sale"

	//20140707 add for New JCB, Kernel5 
	if(etp_cddList[etp_cddIndex].krnID[0] == ETP_KID_JCB)
	{
		L3_Response_Code = JCB_GET_L3_RCCode();
		if(L3_Response_Code == VAP_RIF_RC_DATA)
		{
			VGL_AS210_D1_Track();

			//Get Scheme ID
			SchemeID = JCB_GET_SchemeID();

			//Update Online Data
			JCB_Update_OnlineData();
		}
	}
 

	MPP_Free_DE_LogBuffer();
 

	//turn off RF signal
	ECL_LV1_FIELD_OFF();
}

