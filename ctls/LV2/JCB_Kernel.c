#include <string.h>
#include <stdlib.h>
#include "ECL_Tag.h"
#include "Function.h"
#include "JCB_Kernel_Define.h"
#include "VAP_ReaderInterface_Define.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Function.h"
// Add by Wayne 2020/08/21 to avoid compiler warninig
#include "UTILS_CTLS.H"

#ifndef _PLATFORM_AS210
#include "LCDTFTAPI.h"
#else
#include "xioapi.h"
#endif

//UCHAR JCB_GPO_Buf_Tags[100]={0};
UCHAR *JCB_GPO_Buf_Tags;
UINT JCB_GPO_Buf_Tags_Length = 0;

//UCHAR JCB_RR_Buf_Tags[100]={0};
UCHAR *JCB_RR_Buf_Tags;
UINT JCB_RR_Buf_Tags_Length = 0;

//UCHAR JCB_GenAC_Buf_Tags[100]={0};
UCHAR *JCB_GenAC_Buf_Tags;
UINT JCB_GenAC_Buf_Tags_Length = 0;

UCHAR JCB_RFS_Result=0;
UCHAR JCB_Txn_Terminate=FALSE;
UCHAR JCB_Out_Status=0;

//extern UCHAR pcd_flgJCBStop;		//20140611 add
extern UCHAR etp_flgRestart;
extern UCHAR etp_flgComError;
extern UCHAR etp_flgCmdTimeout;
extern UCHAR etp_flgCDCVM;
extern UCHAR L3_Response_Code;
extern OUTCOME etp_Outcome;
extern UCHAR Online_Data[1024];
extern UINT Online_Data_Length;
extern CAPK glv_CAPK[CAPK_NUMBER];


extern void JCB_Add_Buf(UCHAR Type, UCHAR TLength, UCHAR *Tag);
extern UCHAR JCB_Check_GenerateAC_M_Data(UCHAR TxnMode, UCHAR GenACType);
extern UCHAR JCB_Ker_Initiate_App_Process(void);
extern UCHAR JCB_Ker_Read_Application_Data(void);
extern UCHAR JCB_CDA_Verify(void);
extern UCHAR JCB_CVM_Process(UCHAR TxnMode);
extern UCHAR JCB_Pack_Online_Data(UCHAR TxnMode, UCHAR TxnResult);


UCHAR JCB_Allocate_Buffer(void)
{
	JCB_Snd_Buf=malloc(DEP_BUFFER_SIZE_SEND);
	JCB_Rcv_Buf=malloc(DEP_BUFFER_SIZE_RECEIVE);
	JCB_GenAC_Response=malloc(512);
	JCB_GPO_Buf_Tags=malloc(100);
	JCB_RR_Buf_Tags=malloc(100);
	JCB_GenAC_Buf_Tags=malloc(100);

	if ((JCB_Snd_Buf == NULLPTR) ||
		(JCB_Rcv_Buf == NULLPTR) ||
		(JCB_GenAC_Response == NULLPTR) ||
		(JCB_GPO_Buf_Tags == NULLPTR) ||
		(JCB_RR_Buf_Tags == NULLPTR) ||
		(JCB_GenAC_Buf_Tags == NULLPTR))
	{
		return FAIL;
	}

	return SUCCESS;
}

void JCB_Free_Buffer(void)
{
	free(JCB_Snd_Buf);
	free(JCB_Rcv_Buf);
	free(JCB_GenAC_Response);
	free(JCB_GPO_Buf_Tags);
	free(JCB_RR_Buf_Tags);
	free(JCB_GenAC_Buf_Tags);
}

UCHAR JCB_Cancel_Occur(void)
{
/*	if(pcd_flgJCBStop)
	{
		pcd_flgJCBStop = FALSE;
		return TRUE;
	}
*/	
	return FALSE;
}

void JCB_Handle_GenAC_Rep_CDA(UCHAR *RspData)
{
	UCHAR TmpTL=0,TmpLL=0;
	UINT TmpVL=0;

	UCHAR *Tag9F4BPtr=NULLPTR;
	UCHAR tag9F4B[2]={0x9F,0x4B};

	//Reset
	JCB_GenAC_ResponseLen = 0;
	memset(JCB_GenAC_Response,0x00,sizeof(JCB_GenAC_Response));

	//Handle Constructed Tag "0x77"
	UT_Get_TLVLength(RspData,&TmpTL,&TmpLL,&TmpVL);
	JCB_GenAC_ResponseLen = (TmpTL+TmpLL+TmpVL);
	memcpy(JCB_GenAC_Response,RspData,JCB_GenAC_ResponseLen);

	//if tag9F4B (Signed Dynamic Application Data)Present, Remove it.
//	Tag9F4BPtr = apk_EMVCL_FindTag_withLen(0x9F,0x4B,JCB_GenAC_Response,JCB_GenAC_ResponseLen);
	Tag9F4BPtr=UT_Find_Tag(tag9F4B, JCB_GenAC_ResponseLen, JCB_GenAC_Response);
	if(Tag9F4BPtr)
	{
//		Tag9F4BPtr-=2;	//back to the begin of tag9F4B
		UT_Get_TLVLength(Tag9F4BPtr,&TmpTL,&TmpLL,&TmpVL);
		memmove(Tag9F4BPtr,&Tag9F4BPtr[TmpTL+TmpLL+TmpVL],(JCB_GenAC_ResponseLen-(Tag9F4BPtr-JCB_GenAC_Response)-(TmpTL+TmpLL+TmpVL)));
		JCB_GenAC_ResponseLen-=(TmpTL+TmpLL+TmpVL);
	}

	//Remove the Constructed Tag "0x77" and its Length
	UT_Get_TLVLength(JCB_GenAC_Response,&TmpTL,&TmpLL,&TmpVL);
	JCB_GenAC_ResponseLen-=(TmpTL+TmpLL);
	memcpy(JCB_GenAC_Response,&JCB_GenAC_Response[TmpTL+TmpLL],JCB_GenAC_ResponseLen);

}

void JCB_Remove_Tags(UCHAR Type)
{
	UCHAR TLength = 0,rspCode=0;
	ULONG Index=0;
	UCHAR cursor=0;
	UINT ClearSize=0;
	
	if(Type == JCB_GPO_Tag)
	{
		while(JCB_GPO_Buf_Tags_Length)
		{
			UT_Get_TLVLengthOfT(&JCB_GPO_Buf_Tags[cursor],&TLength);

			rspCode = UT_Search(TLength,&JCB_GPO_Buf_Tags[cursor],(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);	//search glv_tagTable and get the index 
			if(rspCode == SUCCESS)
			{
				if(glv_tagTable[Index].JCB_Presence != UNDEFINED)
				{
					if(glv_tagTable[Index].JCB_Length == 0x00)
						ClearSize = 3+1024;
					else
						ClearSize = 3+glv_tagTable[Index].JCB_Length;
					
					memset((UCHAR *)glv_addTable[Index],0x00,ClearSize);
				}
			}

			cursor+=TLength;
			JCB_GPO_Buf_Tags_Length-=TLength;
		}		
	}
	else if(Type == JCB_RR_Tag)
	{
		while(JCB_RR_Buf_Tags_Length)
		{
			UT_Get_TLVLengthOfT(&JCB_RR_Buf_Tags[cursor],&TLength);

			rspCode = UT_Search(TLength,&JCB_RR_Buf_Tags[cursor],(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);	//search glv_tagTable and get the index 
			if(rspCode == SUCCESS)
			{
				if(glv_tagTable[Index].JCB_Presence != UNDEFINED)
				{
					if(glv_tagTable[Index].JCB_Length == 0x00)
						ClearSize = 3+1024;
					else
						ClearSize = 3+glv_tagTable[Index].JCB_Length;
					
					memset((UCHAR *)glv_addTable[Index],0x00,ClearSize);
				}
			}

			cursor+=TLength;
			JCB_RR_Buf_Tags_Length-=TLength;
		}		
	}
	else if(Type == JCB_GenAC_Tag)
	{
		while(JCB_GenAC_Buf_Tags_Length)
		{
			UT_Get_TLVLengthOfT(&JCB_GenAC_Buf_Tags[cursor],&TLength);

			rspCode = UT_Search(TLength,&JCB_GenAC_Buf_Tags[cursor],(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);	//search glv_tagTable and get the index 
			if(rspCode == SUCCESS)
			{
				if(glv_tagTable[Index].JCB_Presence != UNDEFINED)
				{
					if(glv_tagTable[Index].JCB_Length == 0x00)
						ClearSize = 3+1024;
					else
						ClearSize = 3+glv_tagTable[Index].JCB_Length;
					
					memset((UCHAR *)glv_addTable[Index],0x00,ClearSize);
				}
			}

			cursor+=TLength;
			JCB_GenAC_Buf_Tags_Length-=TLength;
		}		
	}
}

void JCB_Add_Buf(UCHAR Type, UCHAR TLength, UCHAR *Tag)
{
	if(Type == JCB_GPO_Tag)
	{
		memcpy(&JCB_GPO_Buf_Tags[JCB_GPO_Buf_Tags_Length],Tag,TLength);
		JCB_GPO_Buf_Tags_Length+=TLength;
	}
	else if(Type == JCB_RR_Tag)
	{
		memcpy(&JCB_RR_Buf_Tags[JCB_RR_Buf_Tags_Length],Tag,TLength);
		JCB_RR_Buf_Tags_Length+=TLength;
	}
	else if(Type == JCB_GenAC_Tag)
	{
		memcpy(&JCB_GenAC_Buf_Tags[JCB_GenAC_Buf_Tags_Length],Tag,TLength);
		JCB_GenAC_Buf_Tags_Length+=TLength;
	}
}

void JCB_Txn_Outcome_Select_Next(void)
{
	//20140617 add
	memset((UCHAR *)&etp_Outcome,0x00,sizeof(etp_Outcome));

	ETP_Remove_CandidateList();
	//10
	JCB_Out_Status = JCB_TxnR_SelectNext;

	etp_Outcome.Start			= ETP_OCP_Start_C;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= FALSE; 		
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
	etp_Outcome.dscData 		= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt 		= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

void JCB_Txn_Outcome_End_Application_With_Restart_OnDeviceCVM(void)
{
	//20140617 add
	memset((UCHAR *)&etp_Outcome,0x00,sizeof(etp_Outcome));
	//09
	JCB_Out_Status = JCB_TxnR_EndAppWithRstOnDev;

	etp_flgComError = TRUE;	//20140603 add
	etp_flgCDCVM = TRUE;

	etp_Outcome.Start			= ETP_OCP_Start_B;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= TRUE; 		
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_SeePhoneForInstructions;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
	etp_Outcome.hldTime			= 0x0D;
	etp_Outcome.rqtRestart		= TRUE;
	etp_Outcome.rstMsgID		= ETP_OCP_UIM_PresentCardAgain;
	etp_Outcome.rstStatus		= ETP_OCP_UIS_ReadyToRead;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
	etp_Outcome.dscData 		= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt 		= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= 0x0D;
	etp_Outcome.rmvTimeout		= 0;	
}

void JCB_Txn_Outcome_End_Application_With_Restart_Comm_Err(void)
{
	//20140617 add
	memset((UCHAR *)&etp_Outcome,0x00,sizeof(etp_Outcome));
	//08
	JCB_Out_Status = JCB_TxnR_EndAppWithRst;

	etp_flgComError = TRUE;	//20140603 add

	etp_Outcome.Start			= ETP_OCP_Start_B;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= TRUE; 		
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PresentCardAgain;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
	etp_Outcome.hldTime			= 0x0D;
	etp_Outcome.rqtRestart		= TRUE;
	etp_Outcome.rstMsgID		= ETP_OCP_UIM_PresentCardAgain;
	etp_Outcome.rstStatus		= ETP_OCP_UIS_ReadyToRead;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
	etp_Outcome.dscData 		= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt 		= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

//20140620 add
void JCB_Txn_Outcome_End_Application_WithARC(void)
{
	memset((UCHAR *)&etp_Outcome,0x00,sizeof(etp_Outcome));
	//07
	JCB_Out_Status = JCB_TxnR_EndApp;

	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= FALSE; 		
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
	etp_Outcome.dscData 		= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt 		= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

void JCB_Txn_Outcome_End_Application(void)
{
	//20140617 add
	memset((UCHAR *)&etp_Outcome,0x00,sizeof(etp_Outcome));
	//07
	JCB_Out_Status = JCB_TxnR_EndApp;

	JCB_Txn_Terminate = TRUE;

	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= FALSE; 		
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
	etp_Outcome.dscData 		= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt 		= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

void JCB_Txn_Outcome_Try_Another(void)
{
	//20140617 add
	memset((UCHAR *)&etp_Outcome,0x00,sizeof(etp_Outcome));
	//06
	JCB_Out_Status = JCB_TxnR_TryAnother;

	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= FALSE;									//20140710 the kernel does not display any message except "Card Read Successfully"
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PleaseInsertCard;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_ReadyToRead;
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
	etp_Outcome.dscData 		= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_ContactChip;
	etp_Outcome.Receipt 		= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

void JCB_Txn_Outcome_Decline(void)
{
	//20140617 add
	memset((UCHAR *)&etp_Outcome,0x00,sizeof(etp_Outcome));
	//05
	JCB_Out_Status = JCB_TxnR_Decline;

	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= FALSE;									//20140710 the kernel does not display any message except "Card Read Successfully"
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_NotAuthorised;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_CardReadSuccessfully;
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_Yes;
	etp_Outcome.dscData 		= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt 		= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

void JCB_Txn_Outcome_Online_Req_Present_and_Hold(void)
{
	//20140617 add
	memset((UCHAR *)&etp_Outcome,0x00,sizeof(etp_Outcome));
	//04
	JCB_Out_Status = JCB_TxnR_OnlineReq_Hold;

	etp_Outcome.Start			= ETP_OCP_Start_D;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= JCB_CVM_Result;
	etp_Outcome.rqtOutcome		= TRUE; 		
	if(JCB_CVM_Result == ETP_OCP_CVM_OnlinePIN)
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PleaseEnterYourPIN;
	else
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_AuthorisingPleaseWait;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_Processing;
	etp_Outcome.rqtRestart		= TRUE;
	etp_Outcome.rstMsgID		= ETP_OCP_UIM_Processing;
	etp_Outcome.rstStatus		= ETP_OCP_UIS_Processing;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_Yes;
	etp_Outcome.dscData 		= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt 		= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	memcpy((UCHAR*)&etp_Outcome.rmvTimeout,JCB_Static_Parameter.Remove_Timeout,2);
	
}

void JCB_Txn_Outcome_Online_Req_Two_Presentments(void)
{
	//20140617 add
	memset((UCHAR *)&etp_Outcome,0x00,sizeof(etp_Outcome));
	//03
	JCB_Out_Status = JCB_TxnR_OnlineReq_Two_Present;

	etp_Outcome.Start			= ETP_OCP_Start_B;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= JCB_CVM_Result;
	etp_Outcome.rqtOutcome		= TRUE; 		
	if(JCB_CVM_Result == ETP_OCP_CVM_OnlinePIN)
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PleaseEnterYourPIN;
	else
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_AuthorisingPleaseWait;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_CardReadSuccessfully;
	etp_Outcome.rqtRestart		= TRUE;
	etp_Outcome.rstMsgID		= ETP_OCP_UIM_PresentCardAgain;
	etp_Outcome.rstStatus		= ETP_OCP_UIS_ReadyToRead;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_Yes;
	etp_Outcome.dscData 		= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt 		= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

void JCB_Txn_Outcome_Online_Req(void)
{
	//20140617 add
	memset((UCHAR *)&etp_Outcome,0x00,sizeof(etp_Outcome));
	//02
	JCB_Out_Status = JCB_TxnR_OnlineReq;

	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= JCB_CVM_Result;
	etp_Outcome.rqtOutcome		= FALSE;									//20140710 the kernel does not display any message except "Card Read Successfully" 		
	if(JCB_CVM_Result == ETP_OCP_CVM_OnlinePIN)
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PleaseEnterYourPIN;
	else
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_AuthorisingPleaseWait;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_CardReadSuccessfully;
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_Yes;
	etp_Outcome.dscData 		= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt 		= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

void JCB_Txn_Outcome_Approved(void)
{
	//20140617 add
	memset((UCHAR *)&etp_Outcome,0x00,sizeof(etp_Outcome));
	//01
	JCB_Out_Status = JCB_TxnR_Approval;

	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= JCB_CVM_Result;
	etp_Outcome.rqtOutcome		= FALSE;									//20140710 the kernel does not display any message except "Card Read Successfully"
	if(JCB_CVM_Result == ETP_OCP_CVM_ObtainSignature)
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_ApprovedPleaseSign;
	else
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_Approved;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_CardReadSuccessfully;
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_Yes;
	etp_Outcome.dscData 		= ETP_OCP_DiscretionaryDataPresent_No;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt 		= ETP_OCP_Receipt_Yes;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

UCHAR JCB_ISU_GenAC2_CheckLen(UCHAR *ConstructTag, UINT ConstructLen)
{
	UCHAR TLength=0,LLength=0;
	UINT VLength=0;
	ULONG Index=0;
	UCHAR *TagPtr=NULLPTR;
	UCHAR rspCode=0;

	UINT tmplen=0;

	TagPtr = ConstructTag;

	while(tmplen != (ConstructLen - 2))
	{			
		TLength = LLength = VLength = 0;
		Index = 0;

		if((*TagPtr & 0x20) && (*TagPtr != 0xFF))
		{
			UT_Get_TLVLengthOfT(TagPtr,&TLength);
			tmplen += TLength;
			TagPtr+= TLength;

			UT_Get_TLVLengthOfL(TagPtr,&LLength);
			tmplen += LLength;
			TagPtr+= LLength;
		}
		else if((*TagPtr == 0x00) || (*TagPtr ==0xFF))
		{
			tmplen += 1;
			TagPtr += 1;
		}
		else
		{
			UT_Get_TLVLengthOfT(TagPtr,&TLength);

			rspCode = UT_Search(TLength,TagPtr,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);
			if(rspCode == SUCCESS)
			{
				tmplen += TLength;
				TagPtr += TLength; 		//shift buf pointer, and now we are in "length" field

				UT_Get_TLVLengthOfL(TagPtr,&LLength);	//get length's Length
				tmplen += LLength;
				UT_Get_TLVLengthOfV(TagPtr,&VLength);
				tmplen += VLength;

				if((glv_tagTable[Index].JCB_Length)&&(VLength)) 
				{
					if(glv_tagTable[Index].JCB_Length_Def == JCB_Fixed_Length)	//fixed length
					{
						if(VLength != glv_tagTable[Index].JCB_Length)
							return JCB_FAIL;
					}
					else if(glv_tagTable[Index].JCB_Length_Def == JCB_Var_Length)	//var length
					{
						if(VLength>glv_tagTable[Index].JCB_Length)
							return JCB_FAIL;
					}
				}	
				TagPtr+=(LLength+VLength);
			}
			else
			{
				if(((*TagPtr) & 0x1F) == 0x1F)
				{
					tmplen+=TLength;
					TagPtr+=TLength;

					if(UT_Get_TLVLengthOfL(TagPtr,&LLength))
						tmplen += LLength;
					else
						return JCB_FAIL;

					if(UT_Get_TLVLengthOfV(TagPtr,&VLength))
						tmplen += VLength;
					else
						return JCB_FAIL;

					TagPtr += (LLength+VLength);					
				}
				else
					return JCB_FAIL;				
			}
		}
		if(tmplen > (ConstructLen-2))	
			return JCB_FAIL;
	}	

	return JCB_SUCCESS;
}

UCHAR JCB_Issuer_Update_Process(void)
{
	UCHAR Script_tag_len=0;
	UCHAR Script_len_len=0;
	UINT Script_value_len=0;
	UINT tmp_Script_value_len=0;
	
	UCHAR tag_len=0;
	UCHAR len_len=0;
	UINT value_len=0;

	UCHAR *ISU_ptr = 0;
	UCHAR *tag71_ptr = 0;

	UCHAR GenAC_Type=0;

	UCHAR *tmp_New_tag_ptr = 0;
	UCHAR *tmp_tag_ptr = 0;

	UCHAR *tag72_ptr = 0;
	UCHAR rspCode=0;

	UCHAR	tag95[1]={0x95};
	UCHAR	tag9F10[2]={0x9F, 0x10};
	UCHAR	tag9F26[2]={0x9F, 0x26};
	UCHAR	tag9F27[2]={0x9F, 0x27};
	UCHAR	tag9F36[2]={0x9F, 0x36};
	UCHAR	tag9F5F[2]={0x9F, 0x5F};
	

	//5.11.1 Issuer Update Initialisation
	//5.11.1.1
	//JCB CL 1.3 Update - Requirement 5.11.1.1
	if(JCB_JCB_FCI_Error == TRUE)	//FCI is not parsed correctly
	{
		JCB_JCB_FCI_Error = FALSE;	//Reset

		JCB_Txn_Outcome_End_Application();
		return JCB_FAIL;
	}
	
	//5.11.2 Critical Script Processing
	if(JCB_Dynamic_Parameter.JCB_IST_1Len)
	{
		ISU_ptr = JCB_Dynamic_Parameter.JCB_IST_1;
		
		while(JCB_Dynamic_Parameter.JCB_IST_1Len)
		{
			if(*ISU_ptr == 0x71)
			{
				tag71_ptr = ISU_ptr;
				
				UT_Get_TLVLength(tag71_ptr,&Script_tag_len,&Script_len_len,&Script_value_len);

				tmp_Script_value_len = Script_value_len;
				
				ISU_ptr+=(Script_tag_len+Script_len_len);
				JCB_Dynamic_Parameter.JCB_IST_1Len -= (Script_tag_len+Script_len_len);
			}
			
			//Check format
			if((*ISU_ptr == 0x9F) && (*(ISU_ptr+1) == 0x18))
			{
				UT_Get_TLVLength(ISU_ptr,&tag_len,&len_len,&value_len);
				ISU_ptr += (tag_len+len_len+value_len); //for JCB Issuer Update, it will ignore Script ID	
				JCB_Dynamic_Parameter.JCB_IST_1Len -= (tag_len+len_len+value_len);
				tmp_Script_value_len -= (tag_len+len_len+value_len);
			}
			else if(*ISU_ptr == 0x86)
			{
				UT_Get_TLVLength(ISU_ptr,&tag_len,&len_len,&value_len);

				//check tag86 length, ISU_TEST_045
				if( (*(ISU_ptr+tag_len+len_len+value_len)==0x86) || 
					(*(ISU_ptr+tag_len+len_len+value_len)==0x71) ||
					((*(ISU_ptr+tag_len+len_len+value_len)==0x00) && (*(ISU_ptr+tag_len+len_len+value_len+1)==0x00)))
				{
					rspCode=ECL_LV1_DEP(value_len,ISU_ptr+tag_len+len_len,&JCB_Rcv_Buf_Len,JCB_Rcv_Buf, 1000);

					if ((rspCode == ECL_LV1_TIMEOUT_USER) || (rspCode == ECL_LV1_TIMEOUT_ISO))
						etp_flgCmdTimeout=TRUE;

					if( (JCB_Rcv_Buf[JCB_Rcv_Buf_Len-2] != 0x90) &&
						(JCB_Rcv_Buf[JCB_Rcv_Buf_Len-2] != 0x62) &&
						(JCB_Rcv_Buf[JCB_Rcv_Buf_Len-2] != 0x63))
					{
						//5.11.1.1 terminate the delivery of commands

						glv_tag95.Value[4] |= 0x20;				//5.11.1.1 set TVR Byte 5, bit 6 to 1

						//shift to next 71						
						tag71_ptr+=(Script_tag_len+Script_len_len+Script_value_len);
						ISU_ptr = tag71_ptr;
						JCB_Dynamic_Parameter.JCB_IST_1Len -= tmp_Script_value_len; 
						
					}
					else
					{
						ISU_ptr+=(tag_len+len_len+value_len);
						JCB_Dynamic_Parameter.JCB_IST_1Len -= (tag_len+len_len+value_len);
						tmp_Script_value_len -= (tag_len+len_len+value_len);
					}
				}
				else
				{
					glv_tag95.Value[4] |= 0x20;

					//shift to next 71		
					tag71_ptr+=(Script_tag_len+Script_len_len+Script_value_len);
					ISU_ptr = tag71_ptr;
					JCB_Dynamic_Parameter.JCB_IST_1Len -= tmp_Script_value_len; 
				}				
			}
			else	//format error, shift to next tag 71
			{
				glv_tag95.Value[4] |= 0x20;

				//shift to next 71	
				tag71_ptr+=(Script_tag_len+Script_len_len+Script_value_len);
				ISU_ptr = tag71_ptr;
				JCB_Dynamic_Parameter.JCB_IST_1Len -= tmp_Script_value_len; 										
			}			
		}

		//Reset JCB_Dynamic_Parameter.JCB_IST_1
		memset(JCB_Dynamic_Parameter.JCB_IST_1,0x00,sizeof(JCB_Dynamic_Parameter.JCB_IST_1));

		if((JCB_Dynamic_Parameter.JCB_IADLen == 0) && (JCB_Dynamic_Parameter.JCB_IST_2Len == 0))
		{

			JCB_Txn_Outcome_End_Application_WithARC();
			return JCB_EndApplicationWithARC;
		}
	}
	
	//5.11.3 Second GENERATE AC Command
	//5.11.3.1
	if(JCB_Online_Context.CDOL2_Length)
	{
		memset(JCB_Snd_Buf,0x00,sizeof(JCB_Snd_Buf));
		JCB_Snd_Buf_Len = 0;

		memset(JCB_Rcv_Buf,0x00,sizeof(JCB_Rcv_Buf));
		JCB_Rcv_Buf_Len = 0;
		
		//5.11.3.2
		DOL_Patch_DOLData(JCB_Online_Context.CDOL2_Length,JCB_Online_Context.CDOL2_Value,(UCHAR *) &JCB_Snd_Buf_Len,JCB_Snd_Buf);

		if( ((glv_tag8A.Value[0] == 0x30)&&(glv_tag8A.Value[1] == 0x30))	||
			((glv_tag8A.Value[0] == 0x31)&&(glv_tag8A.Value[1] == 0x30))	|| 
			((glv_tag8A.Value[0] == 0x31)&&(glv_tag8A.Value[1] == 0x31))	||
			((glv_tag8A.Value[0] == 0x30)&&(glv_tag8A.Value[1] == 0x31))	||
			((glv_tag8A.Value[0] == 0x30)&&(glv_tag8A.Value[1] == 0x32)))			
			GenAC_Type = 0x40;
		else
			GenAC_Type = 0x00;

		rspCode=ECL_APDU_Generate_AC(GenAC_Type,JCB_Snd_Buf_Len,JCB_Snd_Buf,&JCB_Rcv_Buf_Len,JCB_Rcv_Buf);

		if ((rspCode == ECL_LV1_TIMEOUT_USER) || (rspCode == ECL_LV1_TIMEOUT_ISO))
			etp_flgCmdTimeout=TRUE;

		//5.11.3.6
		if((JCB_Rcv_Buf[JCB_Rcv_Buf_Len-2] != 0x90) || (JCB_Rcv_Buf[JCB_Rcv_Buf_Len-1] != 0x00))
		{
			JCB_Txn_Outcome_End_Application_WithARC();
			return JCB_EndApplicationWithARC;
		}
		else
		{
			//5.11.3.7
			if(JCB_Rcv_Buf[0] == 0x77)
			{
				UT_Get_TLVLength(JCB_Rcv_Buf,&tag_len,&len_len,&value_len);
				if((JCB_Rcv_Buf[tag_len+len_len+value_len]==0x90) &&(JCB_Rcv_Buf[tag_len+len_len+value_len+1]==0x00))
				{
					//20140530 check len
					if(!JCB_ISU_GenAC2_CheckLen(JCB_Rcv_Buf,JCB_Rcv_Buf_Len))
					{
						JCB_Txn_Outcome_End_Application_WithARC();
						return JCB_EndApplicationWithARC;
					}
				
					//Check Mandatory Data
//					tmp_New_tag_ptr = apk_EMVCL_FindTag_withLen(0x9F,0x27,JCB_Rcv_Buf,JCB_Rcv_Buf_Len);
					tmp_New_tag_ptr = UT_Find_Tag(tag9F27, JCB_Rcv_Buf_Len, JCB_Rcv_Buf);
					if(!tmp_New_tag_ptr)
					{
						JCB_Txn_Outcome_End_Application_WithARC();
						return JCB_EndApplicationWithARC;
					}
					else
					{					
//						tmp_tag_ptr = apk_EMVCL_FindTag_withLen(0x9F,0x27,JCB_Online_Context.Txn_Record,JCB_Online_Context.Txn_Record_Length);
						tmp_tag_ptr = UT_Find_Tag(tag9F27, JCB_Online_Context.Txn_Record_Length, JCB_Online_Context.Txn_Record);
						memcpy(tmp_tag_ptr,tmp_New_tag_ptr,2);					
					}

//					tmp_New_tag_ptr = apk_EMVCL_FindTag_withLen(0x9F,0x36,JCB_Rcv_Buf,JCB_Rcv_Buf_Len);
					tmp_New_tag_ptr = UT_Find_Tag(tag9F36, JCB_Rcv_Buf_Len, JCB_Rcv_Buf);
					if(!tmp_New_tag_ptr)
					{
						JCB_Txn_Outcome_End_Application_WithARC();
						return JCB_EndApplicationWithARC;
					}
					else
					{						
//						tmp_tag_ptr = apk_EMVCL_FindTag_withLen(0x9F,0x36,JCB_Online_Context.Txn_Record,JCB_Online_Context.Txn_Record_Length);
						tmp_tag_ptr = UT_Find_Tag(tag9F36, JCB_Online_Context.Txn_Record_Length, JCB_Online_Context.Txn_Record);
						memcpy(tmp_tag_ptr,tmp_New_tag_ptr,3);					
					}

//					tmp_New_tag_ptr = apk_EMVCL_FindTag_withLen(0x9F,0x26,JCB_Rcv_Buf,JCB_Rcv_Buf_Len);
					tmp_New_tag_ptr = UT_Find_Tag(tag9F26, JCB_Rcv_Buf_Len, JCB_Rcv_Buf);
					if(!tmp_New_tag_ptr)
					{
						JCB_Txn_Outcome_End_Application_WithARC();
						return JCB_EndApplicationWithARC;	
					}
					else
					{						
//						tmp_tag_ptr = apk_EMVCL_FindTag_withLen(0x9F,0x26,JCB_Online_Context.Txn_Record,JCB_Online_Context.Txn_Record_Length);
						tmp_tag_ptr = UT_Find_Tag(tag9F26, JCB_Online_Context.Txn_Record_Length, JCB_Online_Context.Txn_Record);
						memcpy(tmp_tag_ptr,tmp_New_tag_ptr,9);					
					}

					//Check Option data
//					tmp_New_tag_ptr = apk_EMVCL_FindTag_withLen(0x9F,0x10,JCB_Rcv_Buf,JCB_Rcv_Buf_Len);
					tmp_New_tag_ptr = UT_Find_Tag(tag9F10, JCB_Rcv_Buf_Len, JCB_Rcv_Buf);
					if(!tmp_New_tag_ptr)
					{
						//do nothing
					}
					else
					{						
//						tmp_tag_ptr = apk_EMVCL_FindTag_withLen(0x9F,0x10,JCB_Online_Context.Txn_Record,JCB_Online_Context.Txn_Record_Length);
						tmp_tag_ptr = UT_Find_Tag(tag9F10, JCB_Online_Context.Txn_Record_Length, JCB_Online_Context.Txn_Record);

						if(tmp_tag_ptr)
							memcpy(tmp_tag_ptr,tmp_New_tag_ptr,((*tmp_New_tag_ptr)+1)); 
						else	//update JCB_Online_Context.Txn_Record
						{
							UT_Get_TLVLength((tmp_New_tag_ptr-2),&tag_len,&len_len,&value_len);
							memcpy(&JCB_Online_Context.Txn_Record[JCB_Online_Context.Txn_Record_Length],(tmp_New_tag_ptr-2),(tag_len+len_len+value_len));
							JCB_Online_Context.Txn_Record_Length+=(tag_len+len_len+value_len);
						}
							
					}

//					tmp_New_tag_ptr = apk_EMVCL_FindTag_withLen(0x9F,0x5F,JCB_Rcv_Buf,JCB_Rcv_Buf_Len);
					tmp_New_tag_ptr = UT_Find_Tag(tag9F5F, JCB_Rcv_Buf_Len, JCB_Rcv_Buf);
					if(!tmp_New_tag_ptr)
					{
						//do nothing
					}
					else
					{						
//						tmp_tag_ptr = apk_EMVCL_FindTag_withLen(0x9F,0x5F,JCB_Online_Context.Txn_Record,JCB_Online_Context.Txn_Record_Length);
						tmp_tag_ptr = UT_Find_Tag(tag9F5F, JCB_Online_Context.Txn_Record_Length, JCB_Online_Context.Txn_Record);
						
						if(tmp_tag_ptr)
							memcpy(tmp_tag_ptr,tmp_New_tag_ptr,((*tmp_New_tag_ptr)+1)); 
						else	//update JCB_Online_Context.Txn_Record
						{
							UT_Get_TLVLength((tmp_New_tag_ptr-2),&tag_len,&len_len,&value_len);
							memcpy(&JCB_Online_Context.Txn_Record[JCB_Online_Context.Txn_Record_Length],(tmp_New_tag_ptr-2),(tag_len+len_len+value_len));
							JCB_Online_Context.Txn_Record_Length+=(tag_len+len_len+value_len);
						}
					}
					
					//update tag95
//					tmp_tag_ptr = apk_EMVCL_FindTag_withLen(0x95,0x00,JCB_Online_Context.Txn_Record,JCB_Online_Context.Txn_Record_Length);
					tmp_tag_ptr = UT_Find_Tag(tag95, JCB_Online_Context.Txn_Record_Length, JCB_Online_Context.Txn_Record);
					memcpy(tmp_tag_ptr+1,glv_tag95.Value,5);
				}
				else
				{
					JCB_Txn_Outcome_End_Application_WithARC();
					return JCB_EndApplicationWithARC;	
				}				
			}
			else
			{
				JCB_Txn_Outcome_End_Application_WithARC();
				return JCB_EndApplicationWithARC;
			}
		}
	}
	else
	{
		JCB_Txn_Outcome_End_Application_WithARC();
		return JCB_EndApplicationWithARC;	
	}

	//5.11.5
	if(JCB_Dynamic_Parameter.JCB_IST_2Len)
	{
		ISU_ptr = JCB_Dynamic_Parameter.JCB_IST_2;
		
		while(JCB_Dynamic_Parameter.JCB_IST_2Len)
		{
			if(*ISU_ptr == 0x72)
			{
				tag72_ptr = ISU_ptr;
				
				UT_Get_TLVLength(tag72_ptr,&Script_tag_len,&Script_len_len,&Script_value_len);

				tmp_Script_value_len = Script_value_len;
				
				ISU_ptr+=(Script_tag_len+Script_len_len);
				JCB_Dynamic_Parameter.JCB_IST_2Len -= (Script_tag_len+Script_len_len);
			}
			
			//Check format
			if((*ISU_ptr == 0x9F) && (*(ISU_ptr+1) == 0x18))
			{
				UT_Get_TLVLength(ISU_ptr,&tag_len,&len_len,&value_len);
				ISU_ptr += (tag_len+len_len+value_len); //for JCB Issuer Update, it will ignore Script ID	
				JCB_Dynamic_Parameter.JCB_IST_2Len -= (tag_len+len_len+value_len);
				tmp_Script_value_len -= (tag_len+len_len+value_len);
			}
			else if(*ISU_ptr == 0x86)
			{
				UT_Get_TLVLength(ISU_ptr,&tag_len,&len_len,&value_len);

				if( (*(ISU_ptr+tag_len+len_len+value_len)==0x86) || 
					(*(ISU_ptr+tag_len+len_len+value_len)==0x72) || 
					((*(ISU_ptr+tag_len+len_len+value_len) == 0x00) && (*(ISU_ptr+tag_len+len_len+value_len+1) == 0x00)))
				{
					rspCode=ECL_LV1_DEP(value_len,ISU_ptr+tag_len+len_len,&JCB_Rcv_Buf_Len,JCB_Rcv_Buf, 1000);

					if ((rspCode == ECL_LV1_TIMEOUT_USER) || (rspCode == ECL_LV1_TIMEOUT_ISO))
						etp_flgCmdTimeout=TRUE;

					if( (JCB_Rcv_Buf[JCB_Rcv_Buf_Len-2] != 0x90) &&
						(JCB_Rcv_Buf[JCB_Rcv_Buf_Len-2] != 0x62) &&
						(JCB_Rcv_Buf[JCB_Rcv_Buf_Len-2] != 0x63))
					{
						//5.11.5.1	terminate the delivery of commands

						glv_tag95.Value[4] |= 0x10;				//5.11.4.1	update TVR Byte 5, bit 5 to 1

						//shift to next 72
						tag72_ptr+=(Script_tag_len+Script_len_len+Script_value_len);
						ISU_ptr = tag72_ptr;
						JCB_Dynamic_Parameter.JCB_IST_2Len -= tmp_Script_value_len; 
						
					}
					else
					{
						ISU_ptr+=(tag_len+len_len+value_len);
						JCB_Dynamic_Parameter.JCB_IST_2Len -= (tag_len+len_len+value_len);
						tmp_Script_value_len -= (tag_len+len_len+value_len);
					}
				}
				else
				{
					glv_tag95.Value[4] |= 0x10;
					
					//shift to next 72
					tag72_ptr+=(Script_tag_len+Script_len_len+Script_value_len);
					ISU_ptr = tag72_ptr;
					JCB_Dynamic_Parameter.JCB_IST_2Len -= tmp_Script_value_len; 
				}				
			}
			else	//format error, shift to next tag 72
			{
				glv_tag95.Value[4] |= 0x10;
				
				//shift to next 72
				tag72_ptr+=(Script_tag_len+Script_len_len+Script_value_len);
				ISU_ptr = tag72_ptr;
				JCB_Dynamic_Parameter.JCB_IST_2Len -= tmp_Script_value_len; 
			}			
		}

		//Reset JCB_Dynamic_Parameter.JCB_IST_2
		memset(JCB_Dynamic_Parameter.JCB_IST_2,0x00,sizeof(JCB_Dynamic_Parameter.JCB_IST_2));

		//Update tag95
//		tmp_tag_ptr = apk_EMVCL_FindTag_withLen(0x95,0x00,JCB_Online_Context.Txn_Record,JCB_Online_Context.Txn_Record_Length);
		tmp_tag_ptr = UT_Find_Tag(tag95, JCB_Online_Context.Txn_Record_Length, JCB_Online_Context.Txn_Record);
		memcpy(tmp_tag_ptr+1,glv_tag95.Value,5);
	}

	//5.11.4
	//JCB CL 1.3 Update - Check CID returned from 2nd GAC
//	tmp_tag_ptr = apk_EMVCL_FindTag_withLen(0x9F,0x27,JCB_Online_Context.Txn_Record,JCB_Online_Context.Txn_Record_Length);
	tmp_tag_ptr = UT_Find_Tag(tag9F27, JCB_Online_Context.Txn_Record_Length, JCB_Online_Context.Txn_Record);
	if(tmp_tag_ptr)
	{
		if((*(tmp_tag_ptr+1) & 0xC0) == 0x00)	//5.11.4.1
		{
			JCB_Txn_Outcome_Decline();
			return JCB_FAIL;	
		}
		else if((*(tmp_tag_ptr+1) & 0xC0) == 0x40)	//5.11.4.2
		{
			JCB_CVM_Result = JCB_Online_Context.CVM_Result;

			//5.11.4.3 20140617 change. JCB_Combined 22
			if(JCB_CVM_Result == ETP_OCP_CVM_OnlinePIN)
				JCB_CVM_Result = ETP_OCP_CVM_NA;
			
			Online_Data_Length = JCB_Online_Context.Txn_Record_Length;
			memcpy(Online_Data,JCB_Online_Context.Txn_Record,Online_Data_Length);
			JCB_Txn_Outcome_Approved();
		}
		else
		{
			JCB_Txn_Outcome_End_Application();
			return JCB_FAIL;
		}
	}
	
	return JCB_SUCCESS;
}

UCHAR JCB_Check_Echo_SW12(UCHAR *datSW)
{
	if ((datSW[0] == 0x90) && (datSW[1] == 0x00))
		return JCB_SW9000;
	else if ((datSW[0] == 0x69) && (datSW[1] == 0x85))
		return JCB_SW6985;
	else 
		return JCB_FAIL;	
}


UCHAR JCB_Echo_Command(UINT *OutLen, UCHAR * OutBuf)
{	
	UCHAR	rspCode=0;
	UCHAR	cmd_Echo[4]={	0x80,	//CLA
							0xDF,	//INS
							0x00,	//P1
							0x00};	//P2
	
	JCB_Snd_Buf_Len = 0;
	memset(JCB_Snd_Buf,0x00,DEP_BUFFER_SIZE_SEND);
	
	memcpy(JCB_Snd_Buf,cmd_Echo,4);//Command
	JCB_Snd_Buf[4] = 0x00; 				//Le

	rspCode=ECL_LV1_DEP(5, JCB_Snd_Buf, OutLen, OutBuf, 1000);

	if ((rspCode == ECL_LV1_TIMEOUT_USER) || (rspCode == ECL_LV1_TIMEOUT_ISO))
		etp_flgCmdTimeout=TRUE;

	if (rspCode == ECL_LV1_SUCCESS)
		return JCB_SUCCESS;
		
	return JCB_FAIL;
}


UCHAR JCB_Txn_Recover_Process(void)
{
	UINT lenOfV=0;
	UCHAR rspCode = 0xFF;
	UCHAR tag9F52[2]={0x9F,0x52};
	UCHAR *ptrData=NULLPTR;

	UCHAR ECHO_Rsp[512] = {0};
	UINT ECHO_Rsp_Len = 0;


	//5.14.1 Select Response Analysis
	//5.14.1.1
	//JCB CL 1.3 Update - Requirement 5.14.1.1
	if (JCB_JCB_FCI_Error == TRUE)
	{
		JCB_JCB_FCI_Error = FALSE;	//Reset
		memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));
		return JCB_EndApplication;
	}

	//5.14.1.2
	//JCB CL 1.3 Update - Requirement 5.14.1.2
	UT_Get_TLVLengthOfV(glv_tag9F38.Length,&lenOfV);
	if(lenOfV != 0)
	{
		ptrData=UT_Find_TagInDOL(tag9F52, lenOfV, glv_tag9F38.Value); 
		if (ptrData == NULLPTR)
		{
			memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));
			return JCB_EndApplication;	//Legacy Mode
		}		
	}
	
	//5.14.2 Echo Command
	//5.14.2.1
	JCB_Rcv_Buf_Len = 0;
	memset(JCB_Rcv_Buf,0x00,DEP_BUFFER_SIZE_RECEIVE);

	rspCode = JCB_Echo_Command(&JCB_Rcv_Buf_Len,JCB_Rcv_Buf);

	//5.14.2.3
	if(rspCode != JCB_SUCCESS)
	{
		memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));
		return JCB_EndAppWithRestartComm;
	}

	//5.14.2.2
	rspCode = JCB_Check_Echo_SW12(&JCB_Rcv_Buf[JCB_Rcv_Buf_Len-2]);
	if(rspCode != JCB_SW9000)
	{
		memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));		
		return JCB_Recover_Break_1;
	}

	//20140610 add
	ECHO_Rsp_Len = JCB_Rcv_Buf_Len;
	if(ECHO_Rsp_Len<512)
		memcpy(ECHO_Rsp,JCB_Rcv_Buf,JCB_Rcv_Buf_Len);
	
	//5.14.3 	Transaction Initialisation
	//5.14.3.1
	//5.2.1.5	reset TVR tag95 to zero, set tag9F52 to 01 
	memset(glv_tag95.Value,0x00,5);

	glv_tag9F52.Length[0]= 0x01;
	glv_tag9F52.Value[0] = 0x01;
	
	//5.2.1.6	reset Online Transaction Context, Transaction Mode to 'Undefined Mode'
	memset(&JCB_Online_Context,0x00,sizeof(JCB_Online_Context));
	JCB_TxnMode = JCB_Txn_Undefined;

	//5.2.1.7 
	if((JCB_Kernel_Support & JCB_Txn_EMV) && (JCB_Static_Parameter.CombinationOption[0] & 0x02))
	{
		glv_tag9F52.Value[0] |= 0x02;
	}

	//5.2.1.8 Terminal Interchange Profile
	//JCB CL 1.3 Update - Requirement 5.2.1.8
	glv_tag9F53.Length[0] = 0x03;
	memcpy(glv_tag9F53.Value,JCB_Static_Parameter.TerminalInterchangeProfile,3);
	glv_tag9F53.Value[0] &= 0x7F;
	
	if(JCB_Kernel_Support & JCB_Txn_ISUSupport)
	{
		glv_tag9F53.Value[1] |= 0x80;
	}
	else
	{
		glv_tag9F53.Value[1] &= 0x7F;
	}
	
	//5.14.4 Initiate Application Processing
	//5.14.4.1
	//20140521 test
	JCB_Remove_Tags(JCB_GPO_Tag);	
	rspCode = JCB_Ker_Initiate_App_Process();
	if(rspCode == JCB_SelectNext)	//5.14.4.3
	{
		//JCB CL 1.3 Update - Requirement 5.14.4.3
		memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));
		return JCB_EndApplication;
	}
	else if ((rspCode == JCB_EndAppWithRestartComm) || (rspCode == JCB_FAIL))	//5.14.4.2
	{
		memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));	//20140610 add
		return JCB_EndAppWithRestartComm;
	}

	//5.14.4.4
	//JCB CL 1.3 Update - Requirement 5.14.4.4
	if (JCB_TxnMode == JCB_Txn_Mag)
	{
		memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));	
		return JCB_EndApplication;
	}
	
	//5.14.5 Read Application Data
	//5.14.5.1
	//20140521 test
	JCB_Remove_Tags(JCB_RR_Tag);	
	rspCode = JCB_Ker_Read_Application_Data();
	if(rspCode == JCB_SelectNext)	//5.14.5.3
	{
		//JCB CL 1.3 Update - Requirement 5.14.5.3
		memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));
		return JCB_EndApplication;
	}
	else if((rspCode == JCB_FAIL) || (rspCode == JCB_EndAppWithRestartComm))	//5.14.5.2
	{
		memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));
		return JCB_EndAppWithRestartComm;
	}
	else if (rspCode == JCB_EndApplication)
	{
		memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));
		return JCB_EndApplication;
	}

//20140610 swap the ECHO data to  "JCB_Rcv_Buf" and check the manadatory data
	JCB_Rcv_Buf_Len = 0;
	memset(JCB_Rcv_Buf,0x00,DEP_BUFFER_SIZE_RECEIVE);
	JCB_Rcv_Buf_Len = ECHO_Rsp_Len;
	memcpy(JCB_Rcv_Buf,ECHO_Rsp,ECHO_Rsp_Len);
//the Echo response will be last Generate AC response, so we check the response format in here.
	rspCode = JCB_Check_GenerateAC_M_Data(JCB_ReContext_Par.LastTxnType,JCB_ReContext_Par.LastGenACType);
	if(rspCode != JCB_SUCCESS)
	{
		memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));		
		return JCB_Decline;
	}
	else
	{
		JCB_Handle_GenAC_Rep_CDA(JCB_Rcv_Buf);		//20140528 we add this for CDA Process

		//20140625 add Show Card Read OK 
		ETP_UI_Request(ETP_OCP_UIM_CardReadOKRemoveCard,00);
	}
	
	//5.14.6 Transaction Recovery Completion
	rspCode = JCB_CDA_Verify();
	if(rspCode != JCB_SUCCESS)
	{
		glv_tag95.Value[0] |= 0x04; //CDA Failed //20140707 add
		memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));
		return JCB_Decline;	//Decline the transaction 5.13.5
	}
	else
	{
		memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));
		//5.8.3	CVM Processing
		if(glv_tag9F50.Length[0])
		{
			rspCode = JCB_CVM_Process(JCB_TxnMode);
			if(rspCode == JCB_FAIL)
			{
				return JCB_Decline;
			}
			else
			{
				//5.8.4	Transaction Outcome
				//5.8.4.1
				if((glv_tag9F27.Value[0] & 0xC0) == 0x40)	//TC
				{
					JCB_Txn_Outcome_Approved();	//Approved Outcome 5.13.1
					JCB_Pack_Online_Data(JCB_Txn_EMV, JCB_TxnApprovalOrOnline);	//20140616 add
				}

				//5.8.4.2~4
				if((glv_tag9F27.Value[0] & 0xC0) == 0x80)	//ARQC
				{					
					//JCB CL 1.3 Update - Copy CDOL2 Like Normal EMV Mode Process
					UT_Get_TLVLengthOfV(glv_tag8D.Length,&lenOfV);
					if(lenOfV)
					{
						JCB_Online_Context.CDOL2_Length = lenOfV;
						memcpy(JCB_Online_Context.CDOL2_Value,glv_tag8D.Value,lenOfV);
					}
					else
					{	
						JCB_Online_Context.CDOL2_Length = 0;
						memset(JCB_Online_Context.CDOL2_Value,0x00,252)	;
					}

					JCB_Online_Context.CVM_Result = JCB_CVM_Result;
					
					//5.8.4.2
					if(	((JCB_Static_Parameter.TerminalInterchangeProfile[1] & 0x80)==0x00 ) ||	//we can check TIP B2b8
						(glv_tag9F60.Length[0] == 0x00)||	//Issuer Update Parameter is absent
						(glv_tag9F60.Value[0] == 0x00))
					{
						JCB_Txn_Outcome_Online_Req();//Online Request 5.13.2
					}

					//5.8.4.3
					if(	(JCB_Static_Parameter.TerminalInterchangeProfile[1] & 0x80)	&&
						(glv_tag9F60.Value[0] == 0x01))
					{
						JCB_Txn_Outcome_Online_Req_Present_and_Hold();//Online Request 5.13.4 "Present and Hold"
					}

					//5.8.4.4
					if((JCB_Static_Parameter.TerminalInterchangeProfile[1] & 0x80)	&&
						(glv_tag9F60.Value[0] == 0x02))
					{
						JCB_Txn_Outcome_Online_Req_Two_Presentments();//Online Request 5.13.3 "Two Presentments"
					}

					JCB_Pack_Online_Data(JCB_Txn_EMV, JCB_TxnApprovalOrOnline);	//20140616 change
				}	
			}
		}
		return JCB_SUCCESS;
	}

}

UCHAR JCB_Check_Mag_M_Data(void)
{
	UCHAR	redundant = 0, tmplen = 0, TLength = 0,LLength = 0, *value_addr = 0, *len_addr = 0, *Mag_buf = 0, rspCode=0,
			tag9F02[4] = {0x9F,0x02,0x00,0x00},tag57[4] = {0x57,0x00,0x00,0x00}; 
	UINT 	VLength = 0;
	ULONG	Index = 0;
	
//	if(!apk_EMVCL_FindTag_withLen(0x57,0x00,JCB_Rcv_Buf,JCB_Rcv_Buf_Len))
	if(!UT_Find_Tag(tag57, JCB_Rcv_Buf_Len, JCB_Rcv_Buf))
		return JCB_SelectNext;

	Mag_buf = JCB_Rcv_Buf;
		
	while(tmplen != (JCB_Rcv_Buf_Len - 2))
	{			
		TLength = LLength = VLength = 0;
		Index = 0;

		if(*Mag_buf & 0x20)	
		{
			if(*Mag_buf != 0xFF)
			{
				UT_Get_TLVLengthOfT(Mag_buf,&TLength);
				tmplen += TLength;
				Mag_buf+= TLength;

				UT_Get_TLVLengthOfL(Mag_buf,&LLength);
				tmplen += LLength;
				Mag_buf+= LLength;
			}
		}
		else if((*Mag_buf == 0x00) || (*Mag_buf ==0xFF))
		{
			tmplen += 1;
			Mag_buf += 1;
		}
		else
		{
			UT_Get_TLVLengthOfT(Mag_buf,&TLength);

			rspCode = UT_Search(TLength,Mag_buf,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);
			if(rspCode == SUCCESS)
			{
				if(glv_addTable[Index][0] != 0) 	//redundent
				{
					if(memcmp(glv_tagTable[Index].Tag,tag9F02,4))
					{
						if(memcmp(glv_tagTable[Index].Tag,tag57,4))
						{
							redundant = 0x01;
							break;
						}
						else
						{
							tmplen += TLength;
							Mag_buf += TLength; 		//shift buf pointer, and now we are in "length" field

							UT_Get_TLVLengthOfL(Mag_buf,&LLength);	//get length's Length
							tmplen += LLength;
							UT_Get_TLVLengthOfV(Mag_buf,&VLength);
							tmplen += VLength;
						
							len_addr = (UCHAR *)(glv_addTable[Index]);	//point to glv_addtable length address
							while(LLength--)
							{
								*len_addr++ = *Mag_buf++;		//shift buf pointer, and now we're in "value" field
							}
							value_addr = (UCHAR *)(glv_addTable[Index]+3);	//point to glv_addtable value address
							while(VLength--)	//copy data to glv_addTable
							{
								*value_addr++ = *Mag_buf++ ;		//shift buf pointer, and now we're in "tag" field
							}
						}
					}
					else
					{
						tmplen += TLength;
						Mag_buf += TLength; 		//shift buf pointer, and now we are in "length" field

						UT_Get_TLVLengthOfL(Mag_buf,&LLength);				//get length's Length
						UT_Get_TLVLengthOfV(Mag_buf,&VLength);				//get Value's Length
						tmplen += (LLength+VLength);
						
						Mag_buf += (LLength+VLength);
					}
				}
				else
				{
					tmplen += TLength;
					Mag_buf += TLength; 		//shift buf pointer, and now we are in "length" field

					UT_Get_TLVLengthOfL(Mag_buf,&LLength);	//get length's Length
					tmplen += LLength;
					UT_Get_TLVLengthOfV(Mag_buf,&VLength);
					tmplen += VLength;
				
					len_addr = (UCHAR *)(glv_addTable[Index]);	//point to glv_addtable length address
					while(LLength--)
					{
						*len_addr++ = *Mag_buf++;		//shift buf pointer, and now we're in "value" field
					}
					value_addr = (UCHAR *)(glv_addTable[Index]+3);	//point to glv_addtable value address
					while(VLength--)	//copy data to glv_addTable
					{
						*value_addr++ = *Mag_buf++ ;		//shift buf pointer, and now we're in "tag" field
					}
				}
			}
			else
			{
				tmplen+=TLength;
				Mag_buf+=TLength;

				UT_Get_TLVLengthOfL(Mag_buf,&LLength);
				tmplen += LLength;

				UT_Get_TLVLengthOfV(Mag_buf,&VLength);
				tmplen += VLength;

				Mag_buf += (LLength+VLength);				
			}
		}
	}

	if(redundant == 0x01)
		return JCB_FAIL;
	else
		return JCB_SUCCESS;
}

UCHAR JCB_Check_Mag_SW12(UCHAR *datSW)
{
	if ((datSW[0] == 0x90) && (datSW[1] == 0x00))
		return JCB_SW9000;
	else if ((datSW[0] == 0x69) && (datSW[1] == 0x86))
		return JCB_SW6986;
	else if ((datSW[0] == 0x63) && (datSW[1] == 0x00))
		return JCB_SW6300;
	else 
		return JCB_FAIL;	
}

UCHAR JCB_Ker_Mag_Command(void)
{
	UCHAR Def_MDOL[19] = {0x9F,0x02,0x06,0x9F,0x1A,0x02,0x5F,0x2A,0x02,0x9A,0x03,0x9C,0x01,0x9F,0x53,0x03,0x9F,0x4E,0x14};

	UINT tmp_Len = 0;

	UCHAR *MDOL_Pointer = 0;
	UINT MDOL_Len = 0;

	UCHAR rspCode = 0xFF;

	UCHAR CommandType = 0x00;		//0x80 for Online, 0x00 for Decline. Reference Control Parameter

	memset(JCB_Snd_Buf,0x00,sizeof(JCB_Snd_Buf));
	JCB_Snd_Buf_Len = 0;

	memset(JCB_Rcv_Buf,0x00,sizeof(JCB_Rcv_Buf));
	JCB_Rcv_Buf_Len = 0;
	
	//5.9.1.1
	UT_Get_TLVLengthOfV(glv_tag9F5C.Length,&tmp_Len);
	if(tmp_Len)
	{
		MDOL_Len = tmp_Len;
		MDOL_Pointer = glv_tag9F5C.Value;
	}
	else
	{
		MDOL_Len = 19;
		MDOL_Pointer = Def_MDOL;
	}

	if(JCB_TAA_Result == JCB_TAAR_AAC)
		 CommandType = 0x00;

	if(JCB_TAA_Result == JCB_TAAR_ARQC)
		 CommandType = 0x80;

	//5.9.1.2
	rspCode = DOL_Patch_DOLData(MDOL_Len,MDOL_Pointer,(UCHAR *)&JCB_Snd_Buf_Len,JCB_Snd_Buf);
	if(rspCode == SUCCESS)
	{
		rspCode = ECL_APDU_Get_Magstripe_Data(CommandType,JCB_Snd_Buf_Len,JCB_Snd_Buf,&JCB_Rcv_Buf_Len,JCB_Rcv_Buf);	//commandtype correspond to the result of Terminal Action Analysis

		if ((rspCode == ECL_LV1_TIMEOUT_USER) || (rspCode == ECL_LV1_TIMEOUT_ISO))
			etp_flgCmdTimeout=TRUE;
		
		if(rspCode == ECL_LV1_SUCCESS)
		{
			rspCode = JCB_Check_Mag_SW12(&JCB_Rcv_Buf[JCB_Rcv_Buf_Len-2]);
			if(rspCode == JCB_SW9000)
			{
				//5.9.1.7
				rspCode = JCB_Check_Mag_M_Data();
				return rspCode;
			}
			else if(rspCode == JCB_SW6986)
			{
				return JCB_EndAppWithRestart;//End application 5.13.9 (with restart, On-device CVM)
			}
			else if(rspCode == JCB_SW6300)
			{
				return JCB_Decline;//decline the transaction 5.13.5
			}
			else	//5.9.1.6
			{
				return JCB_SelectNext;
			}
		}		
		else
			return JCB_FAIL;
	}
	else
		return JCB_FAIL;
}

UCHAR JCB_Pack_Online_Data(UCHAR TxnMode,UCHAR TxnResult)
{
	UINT i = 0,VLength=0;
	UCHAR TLength = 0, LLength = 0, rspCode = 0,*Len_Addr = 0, *Value_Addr = 0;
	ULONG Index=0;
	UCHAR Tag_Total_Len;
	UCHAR *taglist = 0;

	UCHAR JCB_Mag_Online_Data[29] = {	0x9F,0x02,0x9F,0x03,0x5F,0x20,0x9F,0x39,0x9F,0x1A,
										0x9F,0x1F,0x57,0x5F,0x2A,0x9A,0x9F,0x21,0x9C,0x9F,
										0x50,0x8D,0x9F,0x34,0x84,0x5F,0x28,0x9F,0x57};
	
	UCHAR JCB_EMV_Legacy_Online_Data[43] =	{	0x9F,0x02,0x9F,0x03,0x9F,0x26,0x82,0x5F,0x34,0x9F,
												0x36,0x5F,0x20,0x9F,0x27,0x9F,0x10,0x9F,0x39,0x9F,
												0x1A,0x95,0x9F,0x1F,0x57,0x5F,0x2A,0x9A,0x9F,0x21,
												0x9C,0x9F,0x37,0x9F,0x50,0x8D,0x9F,0x34,0x84,0x5F,
												0x28,0x9F,0x57};
	UCHAR JCB_Decline_Data[4] = {0x57,0x95,0x9F,0x53};

	if((TxnMode == JCB_Txn_Mag) && (!TxnResult))
	{
		taglist = JCB_Mag_Online_Data;
		Tag_Total_Len = (UCHAR)sizeof(JCB_Mag_Online_Data);
	}
	else if(((TxnMode == JCB_Txn_EMV) || (TxnMode == JCB_Txn_Legacy)) && (!TxnResult))
	{
		taglist = JCB_EMV_Legacy_Online_Data;
		Tag_Total_Len = (UCHAR)sizeof(JCB_EMV_Legacy_Online_Data);
	}
	else
	{
		taglist = JCB_Decline_Data;
		Tag_Total_Len = (UCHAR)sizeof(JCB_Decline_Data);
	}

	while(Tag_Total_Len)
	{
		TLength = LLength = VLength = Index = 0;
		UT_Get_TLVLengthOfT(taglist,&TLength);
		rspCode = UT_Search(TLength,taglist,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);
		if(rspCode == SUCCESS)
		{
			Len_Addr = (UCHAR *)(glv_addTable[Index]);
			Value_Addr = (UCHAR *)(glv_addTable[Index]+3);
			
			UT_Get_TLVLengthOfV(Len_Addr,&VLength);
			if(VLength)
			{
				memcpy(&Online_Data[i],taglist,TLength);
				i+=TLength;

				UT_Get_TLVLengthOfL(Len_Addr,&LLength);
				memcpy(&Online_Data[i],Len_Addr,LLength);
				i+=LLength;

				UT_Get_TLVLengthOfV(Len_Addr,&VLength);
				memcpy(&Online_Data[i],Value_Addr,VLength);
				i+=VLength;
			}
		}
		else
		{
			return FAIL;
		}
		
		taglist += TLength;
		Tag_Total_Len -= TLength;
	}

	Online_Data_Length = i;

//	JCB_Online_Context.Txn_Record[0] = (Online_Data_Length & 0xFF00)>>8;
//	JCB_Online_Context.Txn_Record[1] = (Online_Data_Length & 0x00FF);

	JCB_Online_Context.Txn_Record_Length = Online_Data_Length;
	memcpy(JCB_Online_Context.Txn_Record,Online_Data,Online_Data_Length);
	
	return SUCCESS;

}

// ---------------------------------------------------------------------------
// FUNCTION: check CRL (Certificate Revocation List).
// INPUT   : rid   - RID [5]
//           capki - CAPK index[1]
//           csn   - certificate serial number [3] 
// OUTPUT  : none.
// RETURN  : TRUE  - matched
//           FALSE - not matched
// ---------------------------------------------------------------------------
UCHAR	JCB_OFDA_CheckCRL( UCHAR *rid, UCHAR capki, UCHAR *csn )
{
	UINT	i;
	UCHAR	result = FALSE;

	
	for( i=0; i<JCB_MAX_CAPK_REVOC_CNT; i++ )
	{	
		if( UT_memcmp(rid,JCB_RevocationList[i].RID, 5 ) == 0 )
	    {
	    	if(UT_memcmp(csn,JCB_RevocationList[i].SN, 3 ) == 0 )
	    	{
	    		if(capki == JCB_RevocationList[i].CAPKIndex)
	    		{
					result = TRUE;	// matched
			    	break;
	    		}
			}
   		}
	}	
	return result;
}

UCHAR JCB_CVM_Process(UCHAR TxnMode)
{
	UCHAR JCB_Mag_CVM_Method = 0xFF;
	UCHAR *JCB_Legacy_CVM_List=0;
	UINT JCB_Legacy_CVM_List_Len=0;
	UCHAR lstBytOf57;

//Reset JCB_CVM_Result
	JCB_CVM_Result = ETP_OCP_CVM_NA;

	//JCB CL 1.3 Update - Add CVM Result
	UT_Set_TagLength(ECL_LENGTH_9F34, glv_tag9F34.Length);
	memset(glv_tag9F34.Value, 0, ECL_LENGTH_9F34);

	//5.9.2.1
	if(TxnMode == JCB_Txn_Mag)
	{
		//JCB CL 1.3 Update - Requirement 5.9.2.1
		lstBytOf57=glv_tag57.Value[glv_tag57.Length[0]-1];
		if ((lstBytOf57 & 0x0F) == 0x0F)	//Exclude Padding
			JCB_Mag_CVM_Method=((lstBytOf57 & 0xF0) >> 4);
		else
			JCB_Mag_CVM_Method=(lstBytOf57 & 0x0F);
	}

	//5.10.2.1
	if(TxnMode == JCB_Txn_Legacy)
	{
		UT_Get_TLVLengthOfV(glv_tag8E.Length,&JCB_Legacy_CVM_List_Len);
		if(JCB_Legacy_CVM_List_Len)
		{
			JCB_Legacy_CVM_List_Len -= 8;
			JCB_Legacy_CVM_List = &glv_tag8E.Value[8];	//the first 4 bytes are amount field, and the second 4 bytes are second amount field
		}
	}

//Check CVM logic
	//if(JCB_Static_Parameter.TerminalInterchangeProfile[0] & 0x80)
	if(glv_tag9F53.Value[0] & 0x80)
	{
		if(TxnMode == JCB_Txn_EMV)
		{
			//5.8.3.2
			if(glv_tag9F50.Value[0] == 0x00)
				return JCB_FAIL;	//Decline the transaction 5.13.5
		}
		else if(TxnMode == JCB_Txn_Mag)
		{
			//5.9.2.2
			if(JCB_Mag_CVM_Method == 1)
				return JCB_FAIL;
		}
		else
		{
			//5.10.2.2
			if(!JCB_Legacy_CVM_List_Len)
			{
				//JCB CL 1.3 Update - Apply Own Policy on Legacy Mode no CVM Preference
				JCB_CVM_Result = ETP_OCP_CVM_ObtainSignature;
				glv_tag9F34.Value[0]=0x1E;	//Obtain signature
				glv_tag9F34.Value[2]=0x00;	//Unknown
				return JCB_SUCCESS;
			}
		}
	}
	else
	{
		if(TxnMode == JCB_Txn_EMV)
		{
			if(glv_tag9F50.Value[0] == 0x00)
			{
				JCB_CVM_Result = ETP_OCP_CVM_NoCVM;
				glv_tag9F34.Value[0]=0x1F;	//No CVM Performed
				glv_tag9F34.Value[2]=0x02;	//Successful
				return JCB_SUCCESS;
			}
		}
		else if(TxnMode == JCB_Txn_Mag)
		{
			if(JCB_Mag_CVM_Method == 1)
			{
				JCB_CVM_Result = ETP_OCP_CVM_NoCVM;
				glv_tag9F34.Value[0]=0x1F;	//No CVM Performed
				glv_tag9F34.Value[2]=0x02;	//Successful
				return JCB_SUCCESS;
			}
		}
		else
		{
			JCB_CVM_Result = ETP_OCP_CVM_NoCVM;
			glv_tag9F34.Value[0]=0x1F;	//No CVM Performed
			glv_tag9F34.Value[2]=0x02;	//Successful
			return JCB_SUCCESS;			
		}
	}	

//Perform CVM Result

	if(TxnMode == JCB_Txn_EMV)	//5.8.3.3
	{		
		if(glv_tag9F50.Value[0] == 0x10)	//Signature
		{
			if(JCB_Static_Parameter.TerminalInterchangeProfile[0] & 0x40)
			{
				JCB_CVM_Result = ETP_OCP_CVM_ObtainSignature;
				glv_tag9F34.Value[0]=0x1E;	//Obtain signature
				glv_tag9F34.Value[2]=0x00;	//Unknown
				return JCB_SUCCESS;
			}
			else
				return JCB_FAIL;	//Decline the transaction 5.13.5
		}

		if(glv_tag9F50.Value[0] == 0x20)	//Online PIN
		{
			if(JCB_Static_Parameter.TerminalInterchangeProfile[0] & 0x20)
			{
				JCB_CVM_Result = ETP_OCP_CVM_OnlinePIN;
				glv_tag9F34.Value[0]=0x02;	//Online PIN
				glv_tag9F34.Value[2]=0x00;	//Unknown
				return JCB_SUCCESS;
			}
			else
				return JCB_FAIL;	//Decline the transaction 5.13.5
		}

		if((glv_tag9F50.Value[0] & 0xF0) == 0x30)//On-Device CVM
		{
			if(JCB_Static_Parameter.TerminalInterchangeProfile[0] & 0x10)
			{
				JCB_CVM_Result = ETP_OCP_CVM_ConfirmationCodeVerified;
				glv_tag9F34.Value[0]=0x01;	//Confirmation Code Verified
				glv_tag9F34.Value[2]=0x02;	//Successful
				return JCB_SUCCESS;
			}
			else
				return JCB_FAIL;	//Decline the transaction 5.13.5	
		}

		//JCB CL 1.3 Update - Apply Own Policy on EMV Mode no CVM Preference
		JCB_CVM_Result = ETP_OCP_CVM_ObtainSignature;
		glv_tag9F34.Value[0]=0x1E;	//Obtain signature
		glv_tag9F34.Value[2]=0x00;	//Unknown
		return JCB_SUCCESS;
	}
	else if(TxnMode == JCB_Txn_Mag)	//5.9.2.3
	{
		if(JCB_Mag_CVM_Method == 2)	//Signature
		{
			if(JCB_Static_Parameter.TerminalInterchangeProfile[0] & 0x40)
			{
				JCB_CVM_Result = ETP_OCP_CVM_ObtainSignature;
				glv_tag9F34.Value[0]=0x1E;	//Obtain signature
				glv_tag9F34.Value[2]=0x00;	//Unknown
				return JCB_SUCCESS;
			}
			else
				return JCB_FAIL;	//Decline the transaction 5.13.5
		}

		if(JCB_Mag_CVM_Method == 3)	//Online PIN
		{
			if(JCB_Static_Parameter.TerminalInterchangeProfile[0] & 0x20)
			{
				JCB_CVM_Result = ETP_OCP_CVM_OnlinePIN;
				glv_tag9F34.Value[0]=0x02;	//Online PIN
				glv_tag9F34.Value[2]=0x00;	//Unknown
				return JCB_SUCCESS;
			}
			else
				return JCB_FAIL;	//Decline the transaction 5.13.5
		}

		if(JCB_Mag_CVM_Method == 4)//On-Device CVM
		{
			if(JCB_Static_Parameter.TerminalInterchangeProfile[0] & 0x10)
			{
				JCB_CVM_Result = ETP_OCP_CVM_ConfirmationCodeVerified;
				glv_tag9F34.Value[0]=0x01;	//Confirmation Code Verified
				glv_tag9F34.Value[2]=0x02;	//Successful
				return JCB_SUCCESS;
			}
			else
				return JCB_FAIL;	//Decline the transaction 5.13.5	
		}

		//JCB CL 1.3 Update - Apply Own Policy on Mag. Stripe Mode no CVM Preference
		JCB_CVM_Result = ETP_OCP_CVM_ObtainSignature;
		glv_tag9F34.Value[0]=0x1E;	//Obtain signature
		glv_tag9F34.Value[2]=0x00;	//Unknown
		return JCB_SUCCESS;
	}
	else	//5.10.2.3
	{
		while(JCB_Legacy_CVM_List_Len)
		{
			if((JCB_Legacy_CVM_List[0] == 0x1F) && (JCB_Legacy_CVM_List[1] == 0x00))
			{
				//JCB CL 1.3 Update - Decline No CVM
				return JCB_FAIL;
			}
			if((JCB_Legacy_CVM_List[0] & 0xBF) == 0x1E)			//Signature (paper)		
			{
				if(JCB_Static_Parameter.TerminalInterchangeProfile[0] & 0x40)
				{
					JCB_CVM_Result = ETP_OCP_CVM_ObtainSignature;
					glv_tag9F34.Value[0]=0x1E;	//Obtain signature
					glv_tag9F34.Value[2]=0x00;	//Unknown
					return JCB_SUCCESS;
				}
				/*
				//20140612 removed	 CVM_PROCESS_078
				else
				{
					if((JCB_Legacy_CVM_List[0]&0x40)==0x00)	//added because CVM_PROCESS_078 
						return JCB_FAIL;
				}*/
				//else				
				//	return JCB_FAIL;	//Removed because CVM_PROCESS_077  
			}
			if((JCB_Legacy_CVM_List[0] & 0xBF) == 0x02)			//Enciphered PIN verified online
			{
				if(JCB_Static_Parameter.TerminalInterchangeProfile[0] & 0x20)
				{
					JCB_CVM_Result = ETP_OCP_CVM_OnlinePIN;
					glv_tag9F34.Value[0]=0x02;	//Online PIN
					glv_tag9F34.Value[2]=0x00;	//Unknown
					return JCB_SUCCESS;
				}
				/*
				//20140612 removed	 CVM_PROCESS_078
				else
				{
					if((JCB_Legacy_CVM_List[0]&0x40)==0x00)	//added because CVM_PROCESS_078 
						return JCB_FAIL;
				}*/
				//else
				//	return JCB_FAIL;	//Removed because CVM_PROCESS_077  
			}
			
			if((JCB_Legacy_CVM_List[0] & 0xBF) == 0x04)			//Enciphered PIN verification performed by ICC
			{
				/*if(JCB_Static_Parameter.TerminalInterchangeProfile[0] & 0x10)
				{
					JCB_CVM_Result = ETP_OCP_CVM_ConfirmationCodeVerified;
					return JCB_SUCCESS;
				}
				else*/	//CVM_Legacy_062
					return JCB_FAIL;
			}
			
			JCB_Legacy_CVM_List += 2;
			JCB_Legacy_CVM_List_Len -= 2;
		}
		
		//return ETP_OCP_CVM_NoCVM;		
	}

	return JCB_FAIL;

}

UCHAR JCB_CDA_Verify(void)
{
	UINT i,j,iTagListLen;
	UINT Re_len=0,iPKcLen=0,iModLen=0,iLeftMostLen=0,iHashLen=0,Tmp_Len1=0,Tag70_Tmp_Len=0;
	UCHAR capki;
	UCHAR rid[JCB_Rid_Len];
	UCHAR Index_not_found = 0;
	UCHAR CAPKI[1]={0}; 	
	UCHAR ISSPK[250] = {0};
	UCHAR pkm[250]={0};
	UCHAR ICCPK[250] = {0}; //Retrieval of the ICC Public Key					tag9f46
	UCHAR pkc[250] = {0};	//Retrieval of the ICC Public Key
	UCHAR JCB_tag70_Data[270];
	UCHAR temp[20];
	UCHAR Tag_Len,*ptrlist;
	UCHAR rspCode;
	ULONG Index;

	UCHAR JCB_ICC_Dynamic_Data[38] = {0};	//Table 19 in EMV book2
	UCHAR JCB_CDA_TmpLen = 0;
	UINT JCB_CDA_TotalLen = 0;
	UCHAR *JCB_CDA_Buf = 0;

	UCHAR g_ibuf[1500];
	UCHAR g_obuf[1500];

	//20140206 for WAVE2 DDA process, we do not check AIP
	//AIP byte 1 bits6 is 0b?
	/*
	rspCode = UT_Get_TLVLengthOfV(glv_tag82.Length,&Tmp_Len1);
	if(Tmp_Len1 != 0)
	{
		if(!(glv_tag82.Value[0] & 0x20))
			return JCB_OFDA_Fail;
	}
	*/

//EMV 4.3 Book 2 Section 6.2-Retrieve the Certification Authority Public Key Index		tag8f
	memcpy(CAPKI,glv_tag8F.Value,1);
	
	for(i=0;i<CAPK_NUMBER;i++)
	{
		if((glv_CAPK[i].Index == CAPKI[0]) && (!memcmp(glv_CAPK[i].RID,glv_tag9F06.Value,5)))
		{
			Index_not_found = 0;
			memcpy(&pkm[2],glv_CAPK[i].Modulus,(UINT)glv_CAPK[i].Length);
			break;
		}
		else
			Index_not_found = 1;
	}
	
	if(Index_not_found)
		return JCB_OFDA_Fail ;

	

//EMV 4.3 Book 2 Section 6.3 -Retrieval of the Issuer Public Key						tag90
	//Issuer Public Key Certificate length check 
	
	rspCode = UT_Get_TLVLengthOfV(glv_tag90.Length, &Tmp_Len1);

	pkm[0]=Tmp_Len1 & 0x00FF;
	pkm[1]=(Tmp_Len1 & 0xFF00) >> 8;
				
	if(glv_CAPK[i].Length == Tmp_Len1)
	{
		ISSPK[0]=Tmp_Len1 & 0x00FF;
		ISSPK[1]=(Tmp_Len1 & 0xFF00) >> 8;
		memcpy(&ISSPK[2],glv_tag90.Value,Tmp_Len1);
	}
	else
		return JCB_OFDA_Fail ;


	//recover function on the Issuer Public Key Certificate using the Certification Authority Public Key	
	//Load external public key
	if( api_rsa_loadkey( pkm, glv_CAPK[i].Exponent) != apiOK )
		return JCB_OFDA_Fail ;
	
	iPKcLen = Tmp_Len1;
	iLeftMostLen = Tmp_Len1 - 36;
	
	//recover	
	if( api_rsa_recover( ISSPK, ISSPK ) != apiOK )
		return JCB_OFDA_Fail ;

	//20140603 for ram_gcc_nos_xx6.ld
	for(i=2;i<ISSPK[0];i++)
	{
		if(ISSPK[i] != 0x00)
		{
			memmove(&ISSPK[2],&ISSPK[i],ISSPK[0]);
			break;
		}
	}


	//Identifies the length of the Issuer Public Key Modulus in bytes
	iModLen = ISSPK[2+14-1];
		
	//vertify the recover data, trailer 0xBC
	if(ISSPK[Tmp_Len1+2-1] != 0xBC)
		return JCB_OFDA_Fail ;

	//vertify the recover data, Recovered Data Header 0x6A
	if(ISSPK[2] != 0x6A)
		return JCB_OFDA_Fail ;

	// vertify the recover data, check certificate format 0x02
	if(ISSPK[2+1] != 0x02)
		return JCB_OFDA_Fail ;

	//vertify the recover data, Table 13 filed2-10, Issuer Public Key Remainder, Issuer Public Key Exponent (Step5,6,7)
	Re_len = (Tmp_Len1 - 22);
	for(i=0;i<Re_len;i++)
		g_ibuf[i] = ISSPK[i+3]; // filed2-10

	rspCode = UT_Get_TLVLengthOfV(glv_tag92.Length,&Tmp_Len1);
	if(Tmp_Len1 != 0)	//Issuer Public Key Remainder
	{
		memcpy(&g_ibuf[i],glv_tag92.Value,Tmp_Len1);	
		i+= Tmp_Len1;
		Re_len+= Tmp_Len1;
	}
	else
	{
		// Ni <= Nca - 36 ?
		if( iModLen > iLeftMostLen )
			return JCB_OFDA_Fail;
	}


	rspCode = UT_Get_TLVLengthOfV(glv_tag9F32.Length,&Tmp_Len1);	
	if(Tmp_Len1 != 0)	//Issuer Public Key exponent
	{
		memcpy(&g_ibuf[i],glv_tag9F32.Value,Tmp_Len1);	
		i+=Tmp_Len1;
		Re_len+=Tmp_Len1;
	}
	else
		return JCB_OFDA_Fail;


	// Verification 6
	if( api_sys_SHA1( Re_len, g_ibuf, temp) != apiOK )
		return JCB_OFDA_Fail;


	// Verification 7
	for( i=0; i<20; i++ )
	{
		if( temp[i] != ISSPK[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
			return JCB_OFDA_Fail;
	}


	// Verification 8: check Issuer ID Number, Leftmost 3-8 digits from the PAN (padded to the right with Hex 'F's)
	rspCode = UT_Get_TLVLengthOfV(glv_tag5A.Length,&Tmp_Len1);	
	if(Tmp_Len1 != 0)
		memcpy(temp,glv_tag5A.Value,Tmp_Len1);// load application PAN

	if( UT_CNcmp( &ISSPK[2+2], temp, 4 ) == FALSE )
		return JCB_OFDA_Fail;
	


	// Verification 9: check the certificate expiration date MMYY
	if( UT_VerifyCertificateExpDate( &ISSPK[2+6] ) == FALSE )	
		return JCB_OFDA_Fail;


	// Verification 10: RID[5] + INDEX[1] + Certificate Serial Number[3] 
	memcpy(&capki,glv_tag8F.Value,1);
	memcpy(rid,glv_tag9F06.Value,5);
	if( JCB_OFDA_CheckCRL( rid, capki, &ISSPK[2+8] ) )
		return JCB_OFDA_Fail;
		
	// Verification 11: check the issuer public key algorithm indicator 0x01
	  if( ISSPK[2+12] != 0x01 )
		return JCB_OFDA_Fail;

	// Verification 12: concatenate the Leftmost Digits of the Issuer Public Key and the Issuer Public Key Remainder
	// Issuer Public Key Modulus (stored in pkm[iModLen] array) = 2L-V
	//		  (1) Leftmost Digits of the Issuer Public Key +
	//		  (2) Issuer Public Key Remainder (if present)			tag92
	for(i=0;i<iLeftMostLen;i++)
		pkm[i+2] = ISSPK[i+2+15];

	rspCode = UT_Get_TLVLengthOfV(glv_tag92.Length,&Tmp_Len1);
	if(Tmp_Len1 != 0)
		memcpy(&pkm[i+2],glv_tag92.Value,Tmp_Len1);
	

//EMV 4.3 Book 2  6.4.1 ICC Public Key Certificate has a length different from the length of the Issuer Public Key Modulus		
	rspCode = UT_Get_TLVLengthOfV(glv_tag9F46.Length,&Tmp_Len1);
	if(iModLen == Tmp_Len1) 	
	{
		pkm[0]=Tmp_Len1 & 0x00FF;
		pkm[1]=(Tmp_Len1 & 0xFF00)>>8;
	}	
	else
		return JCB_OFDA_Fail;

	
	//6.4.2 Obtain the recovered data
	iPKcLen = Tmp_Len1;
	iLeftMostLen = Tmp_Len1 - 42;

	//Load external public key
	if( api_rsa_loadkey( pkm, glv_tag9F32.Value) != apiOK )
		return JCB_OFDA_Fail ;


	ICCPK[0]=Tmp_Len1 & 0x00FF;
	ICCPK[1]=(Tmp_Len1 & 0xFF00)>>8;
	memcpy(&ICCPK[2],glv_tag9F46.Value,Tmp_Len1);
	
	//recover	
	if( api_rsa_recover( ICCPK, ICCPK ) != apiOK )
		return JCB_OFDA_Fail ;
	

	//20130625 for ram_gcc_nos_xx6.ld
	for(i=2;i<ICCPK[0];i++)
	{
		if(ICCPK[i] != 0x00)
		{
			memmove(&ICCPK[2],&ICCPK[i],ICCPK[0]);
			break;
		}
	}

	
	iModLen = ICCPK[2+20-1];

	// Verification 2: check recovered data trailer
	if( ICCPK[ (iPKcLen+2)-1 ] != 0xBC )
		return JCB_OFDA_Fail ;

	// Verification 3: check recovered data header
	if( ICCPK[2+0] != 0x6A )
		return JCB_OFDA_Fail ;

	// Verification 4: check certificate format
	if( ICCPK[2+1] != 0x04 )
		return JCB_OFDA_Fail ;
	
	// Verification 5: Concatenation
	   // (1) Recovered Data[2'nd..10'th] +
	   // (2) ICC Remainder +
	   // (3) ICC Exponent +
	   // (4) Static data to be authenticated
	Re_len = iPKcLen - 22; // header[1], HashResult[20], Trailer[1]
	
	for( i=0; i<Re_len; i++ )
		g_ibuf[i] = ICCPK[i+3]; // from "Certificate Format" to "ICC Public Key"

	rspCode = UT_Get_TLVLengthOfV(glv_tag9F48.Length,&Tmp_Len1);
	if(Tmp_Len1 != 0)	//ICC Public Key Remainder
	{
		memcpy(&g_ibuf[i],glv_tag9F48.Value,Tmp_Len1);	
		i+=Tmp_Len1;
		Re_len+=Tmp_Len1;
	}
	else
	{
		// Nic <= Ni - 42 ?
		if( iModLen > iLeftMostLen )
		{
			//JCB CL 1.3 Update - Set TVR ICC Data Missing
			glv_tag95.Value[0] |= 0x20;
			
			return JCB_OFDA_Fail;
		}
	}
		
	rspCode = UT_Get_TLVLengthOfV(glv_tag9F47.Length,&Tmp_Len1);	
	if(Tmp_Len1 != 0) //ICC public key Exponent
	{
		memcpy(&g_ibuf[i],glv_tag9F47.Value,Tmp_Len1);	//ICC Public Key  exponent
		i+=Tmp_Len1;
		Re_len+=Tmp_Len1;
	}
	else
		return JCB_OFDA_Fail;
	
/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Get STATIC DATA
	// The fields within [] are included.
	// (1) SFI=1..10,  70 LL [Data] SW1 SW2
		// (2) SFI=11..30, [70 LL Data] SW1 SW2
		// (3) If the record is a non Tag-70, ODA has failed.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	for(j=0;j<MAX_JCB_REC_CNT;j++)
	{
		memcpy(JCB_tag70_Data,ADDR_JCB_REC_01+j*JCB_REC_LEN,JCB_REC_LEN);

		//UT_DumpHex(0,0,270,VAP_tag70_Data);
		
		if(JCB_tag70_Data[0]==0x00)
			break;
		else
		{
			if( (JCB_tag70_Data[0] & 0x1f) < 11 )	// SFI=1..10, only store data
			{
				//get length and length's length
				if((JCB_tag70_Data[2] & 0x80) == 0x00)	//length = VAP_tag70_Data[2],	length's length = 1
				{					
					Tag70_Tmp_Len = JCB_tag70_Data[2];
					
					memcpy(&g_ibuf[i],&JCB_tag70_Data[1+1+1],Tag70_Tmp_Len);	
					i+=Tag70_Tmp_Len;
					Re_len+=Tag70_Tmp_Len;
				}
				else									//lenght = VAP_tag70_Data[3][4], length's length = 2 or 3
				{
					switch( JCB_tag70_Data[2] & 0x7f )
					{
						case	0x00: // 1-byte length (128..255)
								Tag70_Tmp_Len = JCB_tag70_Data[2];

								memcpy(&g_ibuf[i],&JCB_tag70_Data[1+1+1],Tag70_Tmp_Len);	
								i+=Tag70_Tmp_Len;
								Re_len+=Tag70_Tmp_Len;
								break;
								
						case	0x01: // 1-byte length (128..255)
								Tag70_Tmp_Len = JCB_tag70_Data[3];

								memcpy(&g_ibuf[i],&JCB_tag70_Data[1+1+2],Tag70_Tmp_Len);	
								i+=Tag70_Tmp_Len;
								Re_len+=Tag70_Tmp_Len;
								break;

						case	0x02: // 2-byte length (256..65535)
								Tag70_Tmp_Len = JCB_tag70_Data[3]*256 + JCB_tag70_Data[4];

								memcpy(&g_ibuf[i],&JCB_tag70_Data[1+1+3],Tag70_Tmp_Len);	
								i+=Tag70_Tmp_Len;
								Re_len+=Tag70_Tmp_Len;
								break;

						default:   // out of spec
								return JCB_OFDA_Fail;
					}
				}
			}
			else	// SFI=11..30, store tag length data
			{
				if( (JCB_tag70_Data[0] & 0x1f) < 31 )
				{
					if((JCB_tag70_Data[2] & 0x80) == 0x00)	//length = VAP_tag70_Data[2],	length's length = 1
					{					
						Tag70_Tmp_Len = JCB_tag70_Data[2];
						
						memcpy(&g_ibuf[i],&JCB_tag70_Data[1],Tag70_Tmp_Len+1+1);	
						i+=Tag70_Tmp_Len+1+1;
						Re_len+=Tag70_Tmp_Len+1+1;
					}
					else									//lenght = VAP_tag70_Data[3][4], length's length = 2 or 3
					{
						switch( JCB_tag70_Data[2] & 0x7f )
						{
							case	0x00: // 1-byte length (128..255)
									Tag70_Tmp_Len = JCB_tag70_Data[2];

									memcpy(&g_ibuf[i],&JCB_tag70_Data[1],Tag70_Tmp_Len+1+1);	
									i+=Tag70_Tmp_Len+1+1;
									Re_len+=Tag70_Tmp_Len+1+1;
									break;
						
							case	0x01: // 1-byte length (128..255)
									Tag70_Tmp_Len = JCB_tag70_Data[3];

									memcpy(&g_ibuf[i],&JCB_tag70_Data[1],Tag70_Tmp_Len+1+2);		//tag length 1, length length 2
									i+=Tag70_Tmp_Len+1+2;
									Re_len+=Tag70_Tmp_Len+1+2;
									break;

							case	0x02: // 2-byte length (256..65535)
									Tag70_Tmp_Len = JCB_tag70_Data[3]*256 + JCB_tag70_Data[4];

									memcpy(&g_ibuf[i],&JCB_tag70_Data[1],(UINT)Tag70_Tmp_Len+1+3);		//tag length 1, length length 3
									i+=Tag70_Tmp_Len+1+3;
									Re_len+=Tag70_Tmp_Len+1+3;
									break;

							default:   // out of spec
									return JCB_OFDA_Fail;
						}
					}		 
				}
				else 
					return JCB_OFDA_Fail;
			}
		}
	}

	// tag 9F4A list is present "Static Data Authentication Tag List"
	rspCode = UT_Get_TLVLengthOfV(glv_tag9F4A.Length,&Tmp_Len1);
	if(Tmp_Len1 != 0)
	{
		//get value's pointer
		ptrlist = glv_tag9F4A.Value;

		if(*ptrlist != 0x82)
			return JCB_OFDA_Fail;

		iTagListLen = Tmp_Len1;

		// concatenated to the current end of the input string
		do
		{
			// check constructed DO
//			if( apk_EMVCL_CheckConsTag( *ptrlist ) == TRUE )
			if (UT_Check_ConstructedTag(ptrlist) == TRUE)
				return JCB_OFDA_Fail;

			// check word tag
//			if( apk_EMVCL_CheckWordTag( *ptrlist ) == TRUE )
			if( UT_Check_WordTag( ptrlist ) == TRUE )
			{
				Tag_Len = 2;		//Word tag
				UT_Search(Tag_Len,ptrlist,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);
				ptrlist+=2;
				iTagListLen-=2;
			}
			else
			{
				Tag_Len = 1;		//single tag
				UT_Search(Tag_Len,ptrlist,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);
				ptrlist++;
				iTagListLen--;
			}

			//check length
			if(*glv_addTable[Index] & 0x80)
			{
				if(*glv_addTable[Index] & 0x01)
				{
					Tmp_Len1 = *(glv_addTable[Index]+1);
				}
				else if(*glv_addTable[Index] & 0x02) 
				{
					Tmp_Len1 = ((*(glv_addTable[Index]+1))*256)+(*(glv_addTable[Index]+2));
				}
				else
					Tmp_Len1 = *glv_addTable[Index];
			}
			else
			{
				Tmp_Len1 = *glv_addTable[Index];
			}

			// concatenated to the current end of the input string
			if( Tmp_Len1 != 0) 
			{
				memcpy(&g_ibuf[i],glv_addTable[Index]+3,Tmp_Len1);
				i+=Tmp_Len1;
				Re_len+=Tmp_Len1;
			}
			else
				return JCB_OFDA_Fail;
		} while( iTagListLen > 0 ); // next tag  
	}

	// Verification 6: calculate & compare SHA1 result
	if( api_sys_SHA1(Re_len, g_ibuf, temp) != apiOK )
		return JCB_OFDA_Fail;

	//Verification 7: Compare the calculated hash result from the previous step with the recovered Hash Result
	for( i=0; i<20; i++ )
	{
		if( temp[i] != ICCPK[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
			return JCB_OFDA_Fail;
	}

	//Verification 8 : Compare the recovered PAN
	memset(temp, 0xff, 10 ); // padded with 'F'

	rspCode = UT_Get_TLVLengthOfV(glv_tag5A.Length,&Tmp_Len1);
	memcpy(temp,glv_tag5A.Value,Tmp_Len1);
	if( UT_CNcmp2(&ICCPK[2+2],temp,10) == FALSE)
		return JCB_OFDA_Fail;

	//Verification 9 : Verify that the last day
	if( UT_VerifyCertificateExpDate( &ICCPK[2+12] ) == FALSE )
		return	JCB_OFDA_Fail;
  	
	//Verification 10 : Check ICC Public Key Algorithm Indicator
	if( ICCPK[2+18] != 0x01 )
		return JCB_OFDA_Fail;

	//Verification 11 :  Concatenate the Leftmost Digits of the ICC Public Key and the ICC Public Key Remainder (if present) to 
	//				obtain the ICC Public Key Modulus
	for( i=0; i<iLeftMostLen; i++ )
		pkm[i+2] = ICCPK[i+2+21];

	rspCode = UT_Get_TLVLengthOfV(glv_tag9F48.Length,&Tmp_Len1);
	if(Tmp_Len1 != 0 )
	{
		memcpy(&pkm[i+2],glv_tag9F48.Value,Tmp_Len1);
		i+=Tmp_Len1;
	}
////////////////////////////JCB_CDA////////////////////////////////////////

//6.6.2 Dynamic Signature Verification

	//Verification 1 : compare the length between "Signed Dynamic Application Data(9F4B)" & "ICC Public key module" 

	rspCode = UT_Get_TLVLengthOfV(glv_tag9F4B.Length,&Tmp_Len1);
	if(Tmp_Len1 != 0)
	{
		if(iModLen == Tmp_Len1) 	
		{
			pkm[0]=Tmp_Len1 & 0x00FF;
			pkm[1]=(Tmp_Len1 & 0xFF00)>>8;
			iPKcLen = Tmp_Len1;
		}	
		else
			return JCB_OFDA_Fail;
	}

	//Verification 2 : check recover data tail
	//Load external public key
	if( api_rsa_loadkey( pkm, glv_tag9F47.Value) != apiOK )
		return JCB_OFDA_Fail ;

	pkc[0]=Tmp_Len1 & 0x00FF;
	pkc[1]=(Tmp_Len1 & 0xFF00)>>8;
	memcpy(&pkc[2],glv_tag9F4B.Value,Tmp_Len1);
	
	//recover	
	if( api_rsa_recover( pkc, pkc ) != apiOK )
		return JCB_OFDA_Fail ;

	//20130625 for ram_gcc_nos_xx6.ld
	for(i=2;i<pkc[0];i++)
	{
		if(pkc[i] != 0x00)
		{
			memmove(&pkc[2],&pkc[i],pkc[0]);
			break;
		}
	}

	if(pkc[iPKcLen+2-1] != 0xBC)
		return JCB_OFDA_Fail ;

	//Verification 3:	check recover data header
	if(pkc[2+0] != 0x6A)
		return JCB_OFDA_Fail;

	//Verification 4:	Check Signed Data Format
	if(pkc[2+1] != 0x05)
		return JCB_OFDA_Fail;

	if(pkc[2+2] != 0x01)	//JCB_Test_Case CDA_VERIFICATION_047, check Hash Algorithm Indicator
		return JCB_OFDA_Fail;

	//Verification 5:	Retrieve from the ICC Dynamic Data
	memcpy(JCB_ICC_Dynamic_Data,&pkc[6],pkc[5]);

	//Verification 6:	Check that the Cryptogram Information Data retrieved from the ICC Dynamic Data is equal to the 
	//				Cryptogram Information Data obtained from the response to the GENERATE AC command.
	i = JCB_ICC_Dynamic_Data[0];
	if(JCB_ICC_Dynamic_Data[i+1] != glv_tag9F27.Value[0])
		return JCB_OFDA_Fail;

	//Verification 7:	Concatenate the second to the sixth data elements, followed by Unpredictable Number
	//20140521
	if(JCB_ReContext_Par.RecoverEMVFlag)
	{
		i=0;
		memcpy(&g_obuf[1+i],JCB_ReContext_Par.UnPredNum,sizeof(JCB_ReContext_Par.UnPredNum));
		i+=sizeof(JCB_ReContext_Par.UnPredNum);

		g_obuf[0] = i;
	}
	else
	{
		UT_Get_TLVLengthOfV(glv_tag9F37.Length,&Tmp_Len1);
		if(Tmp_Len1)
		{
			i=0;
			memcpy(&g_obuf[1+i],glv_tag9F37.Value,Tmp_Len1);
			i+=Tmp_Len1;

			g_obuf[0] = i;
		}

	}
				
	iHashLen = iPKcLen - 22; // header[1], HashResult[20], Trailer[1]

	for( i=0; i<iHashLen; i++ )
		g_ibuf[i] = pkc[i+3]; // from "Signed Data Format" to "Pad pattern"

	memcpy( &g_ibuf[i], &g_obuf[1], g_obuf[0] ); // Concatenate from left to right the second to the sixth data. followed by "Unpredicatable Number"
	iHashLen += g_obuf[0];

	//Verification 8: 	Apply the indicated hash algorithm
	if( api_sys_SHA1(iHashLen, g_ibuf, temp) != apiOK )
		return JCB_OFDA_Fail;

	//Verification 9: 	Compare the calculated hash result
	for( i=0; i<20; i++ )
	{
		if( temp[i] != pkc[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
			return JCB_OFDA_Fail;
	}

	//Verification 10: 	Concatenate from left to right the values of the following data elements
	memset(g_ibuf,0x00,sizeof(g_ibuf));
	JCB_CDA_Buf = g_ibuf;		//assign the pointer

	//20140521
	if(JCB_ReContext_Par.RecoverEMVFlag)
	{
		memcpy(JCB_CDA_Buf,JCB_ReContext_Par.TornCDAHashDataBuf,JCB_ReContext_Par.TornCDAHashDataBufSize);
		JCB_CDA_TotalLen+=JCB_ReContext_Par.TornCDAHashDataBufSize;
		JCB_CDA_Buf +=JCB_ReContext_Par.TornCDAHashDataBufSize;
	}
	else
	{
		UT_Get_TLVLengthOfV(glv_tag9F38.Length,&Tmp_Len1);	//PDOL
		if(Tmp_Len1)
		{
			DOL_Patch_DOLData(Tmp_Len1,glv_tag9F38.Value, &JCB_CDA_TmpLen, JCB_CDA_Buf);
			JCB_CDA_TotalLen+=JCB_CDA_TmpLen;
			JCB_CDA_Buf +=JCB_CDA_TmpLen;
			
			Tmp_Len1 = JCB_CDA_TmpLen = 0;
		}

		UT_Get_TLVLengthOfV(glv_tag8C.Length,&Tmp_Len1);	//CDOL
		if(Tmp_Len1)
		{
			DOL_Patch_DOLData(Tmp_Len1,glv_tag8C.Value, &JCB_CDA_TmpLen, JCB_CDA_Buf);
			JCB_CDA_TotalLen+=JCB_CDA_TmpLen;
			JCB_CDA_Buf +=JCB_CDA_TmpLen;
			
			Tmp_Len1 = JCB_CDA_TmpLen = 0;
		}
	}
///////////////////////////////////handle GenAC Response///////////////////////////////////////
////////////////////////////////with the exception of the SDAD/////////////////////////////////////
	/*UT_Get_TLVLengthOfV(glv_tag9F27.Length,&Tmp_Len1);	//CID
	if(Tmp_Len1)
	{
		//T
		JCB_CDA_Buf[0] = 0x9F;
		JCB_CDA_Buf[1] = 0x27;
		JCB_CDA_TotalLen+=2;
		JCB_CDA_Buf +=2;
		
		//L
		JCB_CDA_Buf[0] = Tmp_Len1;
		JCB_CDA_TotalLen+=1;
		JCB_CDA_Buf +=1;
		
		//V
		memcpy(JCB_CDA_Buf,glv_tag9F27.Value,Tmp_Len1);
		JCB_CDA_TotalLen+=Tmp_Len1;
		JCB_CDA_Buf +=Tmp_Len1;
		
		Tmp_Len1 = 0;
	}

	UT_Get_TLVLengthOfV(glv_tag9F36.Length,&Tmp_Len1);	//Application Transaction Counter
	if(Tmp_Len1)
	{
		//T
		JCB_CDA_Buf[0] = 0x9F;
		JCB_CDA_Buf[1] = 0x36;
		JCB_CDA_TotalLen+=2;
		JCB_CDA_Buf +=2;
		
		//L
		JCB_CDA_Buf[0] = Tmp_Len1;
		JCB_CDA_TotalLen+=1;
		JCB_CDA_Buf +=1;
		
		//V
		memcpy(JCB_CDA_Buf,glv_tag9F36.Value,Tmp_Len1);
		JCB_CDA_TotalLen+=Tmp_Len1;
		JCB_CDA_Buf +=Tmp_Len1;
		
		Tmp_Len1 = 0;
	}

	UT_Get_TLVLengthOfV(glv_tag9F50.Length,&Tmp_Len1);	//Cardholder Verification Status
	if(Tmp_Len1)
	{
		//T
		JCB_CDA_Buf[0] = 0x9F;
		JCB_CDA_Buf[1] = 0x50;
		JCB_CDA_TotalLen+=2;
		JCB_CDA_Buf +=2;
		
		//L
		JCB_CDA_Buf[0] = Tmp_Len1;
		JCB_CDA_TotalLen+=1;
		JCB_CDA_Buf +=1;
		
		//V
		memcpy(JCB_CDA_Buf,glv_tag9F50.Value,Tmp_Len1);
		JCB_CDA_TotalLen+=Tmp_Len1;
		JCB_CDA_Buf +=Tmp_Len1;
		
		Tmp_Len1 = 0;
	}

	UT_Get_TLVLengthOfV(glv_tag9F5F.Length,&Tmp_Len1);	//offline Balance
	if(Tmp_Len1)
	{
		//T
		JCB_CDA_Buf[0] = 0x9F;
		JCB_CDA_Buf[1] = 0x5F;
		JCB_CDA_TotalLen+=2;
		JCB_CDA_Buf +=2;
		
		//L
		JCB_CDA_Buf[0] = Tmp_Len1;
		JCB_CDA_TotalLen+=1;
		JCB_CDA_Buf +=1;
		
		//V
		memcpy(JCB_CDA_Buf,glv_tag9F5F.Value,Tmp_Len1);
		JCB_CDA_TotalLen+=Tmp_Len1;
		JCB_CDA_Buf +=Tmp_Len1;
		
		Tmp_Len1 = 0;
	}

	UT_Get_TLVLengthOfV(glv_tag9F10.Length,&Tmp_Len1);	//Issuer Application Data
	if(Tmp_Len1)
	{
	
		//T
		JCB_CDA_Buf[0] = 0x9F;
		JCB_CDA_Buf[1] = 0x10;
		JCB_CDA_TotalLen+=2;
		JCB_CDA_Buf +=2;
		
		//L
		JCB_CDA_Buf[0] = Tmp_Len1;
		JCB_CDA_TotalLen+=1;
		JCB_CDA_Buf +=1;
		
		//V
		memcpy(JCB_CDA_Buf,glv_tag9F10.Value,Tmp_Len1);
		JCB_CDA_TotalLen+=Tmp_Len1;
		JCB_CDA_Buf +=Tmp_Len1;
		
		Tmp_Len1 = 0;
	}

	UT_Get_TLVLengthOfV(glv_tag9F60.Length,&Tmp_Len1);	//Issuer Update Parameter
	if(Tmp_Len1)
	{
		//T
		JCB_CDA_Buf[0] = 0x9F;
		JCB_CDA_Buf[1] = 0x60;
		JCB_CDA_TotalLen+=2;
		JCB_CDA_Buf +=2;
		
		//L
		JCB_CDA_Buf[0] = Tmp_Len1;
		JCB_CDA_TotalLen+=1;
		JCB_CDA_Buf +=1;
		
		//V
		memcpy(JCB_CDA_Buf,glv_tag9F60.Value,Tmp_Len1);
		JCB_CDA_TotalLen+=Tmp_Len1;
		JCB_CDA_Buf +=Tmp_Len1;
		
		Tmp_Len1 = 0;
	}*/
	
	memcpy(JCB_CDA_Buf,JCB_GenAC_Response,JCB_GenAC_ResponseLen);
	JCB_CDA_TotalLen+=JCB_GenAC_ResponseLen;
///////////////////////////////////handle GenAC Response///////////////////////////////////////	

	//Verification 11: 	Apply the indicated hash algorithm
	if( api_sys_SHA1(JCB_CDA_TotalLen, g_ibuf, temp) != apiOK )
		return JCB_OFDA_Fail;

	//Verification 12: 	Compare the calculated hash result
	for( i=0; i<20; i++ )
	{
		Tmp_Len1 = JCB_ICC_Dynamic_Data[0];
		if( temp[i] != JCB_ICC_Dynamic_Data[1+Tmp_Len1+9+i] ) // offset address of Hash: (iPKcLen+2)-1-20
			return JCB_OFDA_Fail;
	}
	
	// Save "ICC Dynamic Number" (Tag=9F4C)
	Tmp_Len1 = JCB_ICC_Dynamic_Data[0];			 // length
	if( (Tmp_Len1> 1) && (Tmp_Len1 < 9) ) // valid only 2~8 bytes
	{
		glv_tag9F4C.Length[0] = (UCHAR)Tmp_Len1;
		memcpy(glv_tag9F4C.Value,&JCB_ICC_Dynamic_Data[1],Tmp_Len1);
	}

	// Save "Application Cryptogram" (Tag=9F26)
	glv_tag9F26.Length[0] = 8;
	memcpy(glv_tag9F26.Value,&JCB_ICC_Dynamic_Data[1+Tmp_Len1+1],8);
	
	return SUCCESS;
   
}


UCHAR JCB_Check_GenerateAC_M_Data(UCHAR TxnMode, UCHAR GenACType)
{
	UCHAR	redundant = 0, tmplen = 0, TLength = 0,LLength = 0, *value_addr = 0, *len_addr = 0, *gac_buf = 0, rspCode=0,
			tag9F02[4] = {0x9F,0x02,0x00,0x00}; 
	UINT iLEN, VLength = 0;
	ULONG	Index = 0;

	//Check Format

	if(TxnMode == JCB_Txn_Legacy)
	{
		if(JCB_Rcv_Buf[0] != 0x80)
			return JCB_Decline;
	}
	else
	{
		if(JCB_Rcv_Buf[0] != 0x77)
			return JCB_Decline;
	}
	
	if(JCB_Rcv_Buf[0] == 0x80) 	//primitive
	{
		//Check Length
		UT_Get_TLVLength(JCB_Rcv_Buf,&TLength,&LLength,&VLength);

		if((JCB_Rcv_Buf[TLength+LLength+VLength]==0x90) && (JCB_Rcv_Buf[TLength+LLength+VLength+1]==0x00))
		{
			iLEN = VLength;
			
			if(iLEN>=11)
			{
				//Check Manadatory Tag and save
				// save "Cryptogram Information Data" (CID)
				if(glv_tag9F27.Length[0] == 0x00)
				{
					glv_tag9F27.Length[0] = 0x01;
					memcpy(glv_tag9F27.Value,&JCB_Rcv_Buf[1+1],1);

					JCB_Add_Buf(JCB_GenAC_Tag,2,(UCHAR *)"9F27");
				}
				else
					return JCB_FAIL;	//redundent
				
				// save "Application Transaction Counter" (ATC)
				if(glv_tag9F36.Length[0] == 0x00)
				{
					glv_tag9F36.Length[0] = 0x02;
					memcpy(glv_tag9F36.Value,&JCB_Rcv_Buf[1+1+1],2);

					JCB_Add_Buf(JCB_GenAC_Tag,2,(UCHAR *)"9F36");
				}
				else
					return JCB_FAIL;	//redundent

				// save "Application Cryptogram (AC)"
				if(glv_tag9F26.Length[0] == 0x00)
				{
					glv_tag9F26.Length[0] = 0x08;	//Application Cryptogram (ARQC)
					memcpy(glv_tag9F26.Value,&JCB_Rcv_Buf[1+1+1+2],8);

					JCB_Add_Buf(JCB_GenAC_Tag,2,(UCHAR *)"9F26");
				}
				else
					return JCB_FAIL;	//redundent

				if(iLEN > 11)	//Issuer Application Data (IAD) Present
				{
					iLEN -= 11;
					
					if(iLEN > 32)
					{
						//JCB CL 1.3 Update - Decline Transaction when Length more than 43 bytes (CID+ATC+ARQC+IAD)
						return JCB_Decline;
					}
					
					//save "Issuer Application Data (IAD)"
					if(glv_tag9F10.Length[0] == 0x00)
					{
						glv_tag9F10.Length[0] = iLEN;
						memcpy(glv_tag9F10.Value,&JCB_Rcv_Buf[1+1+1+2+8],iLEN);

						JCB_Add_Buf(JCB_GenAC_Tag,2,(UCHAR *)"9F10");
					}
					else 
						return FAIL;	//redundent					
				}
			}
			else
				return JCB_Decline;	
		}
		else
		{
			//JCB CL 1.3 Update - Decline Transaction when Template Length Error
			return JCB_Decline;
		}
	}
	else		//constructed
	{
		//Check Length
		UT_Get_TLVLength(JCB_Rcv_Buf,&TLength,&LLength,&VLength);

		//JCB CL 1.3 Update - Check Template Length
		if ((TLength+LLength+VLength) != (JCB_Rcv_Buf_Len - 2))
			return JCB_Decline;

		if(!((JCB_Rcv_Buf[TLength+LLength+VLength] == 0x90) && (JCB_Rcv_Buf[TLength+LLength+VLength+1] == 0x00)))
			return JCB_SelectNext;

		/*if((GenACType & 0x40) || (GenACType == 0x90))	//[TC returned] or [ARQC returned, CDA requested], handle table 6-3
		{
			//check Manadatory Tag
			if(!apk_EMVCL_FindTag_withLen(0x9F,0x27,JCB_Rcv_Buf,JCB_Rcv_Buf_Len))
				return JCB_Decline;
			if(!apk_EMVCL_FindTag_withLen(0x9F,0x36,JCB_Rcv_Buf,JCB_Rcv_Buf_Len))
				return JCB_Decline;
			if(!apk_EMVCL_FindTag_withLen(0x9F,0x4B,JCB_Rcv_Buf,JCB_Rcv_Buf_Len))
				return JCB_Decline;	
			if(!apk_EMVCL_FindTag_withLen(0x9F,0x50,JCB_Rcv_Buf,JCB_Rcv_Buf_Len))
				return JCB_Decline;	
		}
		else if(GenACType == 0x80)	//[ARQC returned, CDA not requested]
		{
			//check Manadatory Tag
			if(!apk_EMVCL_FindTag_withLen(0x9F,0x27,JCB_Rcv_Buf,JCB_Rcv_Buf_Len))
				return JCB_Decline;
			if(!apk_EMVCL_FindTag_withLen(0x9F,0x36,JCB_Rcv_Buf,JCB_Rcv_Buf_Len))
				return JCB_Decline;
			if(!apk_EMVCL_FindTag_withLen(0x9F,0x26,JCB_Rcv_Buf,JCB_Rcv_Buf_Len))
				return JCB_Decline;	
			if(!apk_EMVCL_FindTag_withLen(0x9F,0x50,JCB_Rcv_Buf,JCB_Rcv_Buf_Len))
				return JCB_Decline;	
		}
		else if((GenACType == 0x00) || (GenACType == 0x10))	//[AAC returned]
		{
			//check Manadatory Tag
			if(!apk_EMVCL_FindTag_withLen(0x9F,0x27,JCB_Rcv_Buf,JCB_Rcv_Buf_Len))
				return JCB_Decline;
			if(!apk_EMVCL_FindTag_withLen(0x9F,0x36,JCB_Rcv_Buf,JCB_Rcv_Buf_Len))
				return JCB_Decline;
			if(!apk_EMVCL_FindTag_withLen(0x9F,0x26,JCB_Rcv_Buf,JCB_Rcv_Buf_Len))
				return JCB_Decline;	
		}
		else
			return JCB_FAIL;
		*/
		gac_buf = &JCB_Rcv_Buf[0];

		while(tmplen != (JCB_Rcv_Buf_Len - 2))
		{
			TLength = LLength = VLength = 0;
			Index = 0;

			if((*gac_buf & 0x20) && (*gac_buf != 0xFF))
			{
				//JCB CL 1.3 Update - Check Response Code
				rspCode=UT_Get_TLVLengthOfT(gac_buf,&TLength);
				if (rspCode == FAIL)
					return JCB_Decline;
				
				tmplen += TLength;
				gac_buf+= TLength;

				rspCode=UT_Get_TLVLengthOfL(gac_buf,&LLength);
				if (rspCode == FAIL)
					return JCB_Decline;
				
				tmplen += LLength;
				gac_buf+= LLength;
			}
			else if((*gac_buf == 0x00) || (*gac_buf ==0xFF))
			{
				tmplen += 1;
				gac_buf += 1;
			}
			else
			{
				//JCB CL 1.3 Update - Check Response Code
				rspCode = UT_Get_TLVLengthOfT(gac_buf,&TLength);
				if (rspCode == SUCCESS)
				{
					rspCode = UT_Search(TLength,gac_buf,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);
					if(rspCode == SUCCESS)
					{
						JCB_Add_Buf(JCB_GenAC_Tag,TLength,gac_buf);
						
						if(glv_addTable[Index][0] != 0) 	//redundent
						{
							if(memcmp(glv_tagTable[Index].Tag,tag9F02,4))
							{
								redundant = 0x01;
								break;
							}
							else
							{
								tmplen += TLength;
								gac_buf += TLength; 		//shift buf pointer, and now we are in "length" field

								rspCode = UT_Get_TLVLengthOfL(gac_buf,&LLength);				//get length's Length
								if (rspCode == FAIL)
									return JCB_Decline;
								
								rspCode = UT_Get_TLVLengthOfV(gac_buf,&VLength);				//get Value's Length
								if (rspCode == FAIL)
									return JCB_Decline;
								
								tmplen += (LLength+VLength);
								
								gac_buf += (LLength+VLength);
							}
						}
						else
						{
							tmplen += TLength;
							gac_buf += TLength; 		//shift buf pointer, and now we are in "length" field

							rspCode = UT_Get_TLVLengthOfL(gac_buf,&LLength);	//get length's Length
							if (rspCode == FAIL)
								return JCB_Decline;
							
							tmplen += LLength;
							
							rspCode = UT_Get_TLVLengthOfV(gac_buf,&VLength);
							if (rspCode == FAIL)
								return JCB_Decline;
							
							tmplen += VLength;

							if((glv_tagTable[Index].JCB_Length)&&(VLength))	//JCB Gen AC 79 , we add check "VLength" for JCB_GenAC_80
							{
								if(glv_tagTable[Index].JCB_Length_Def == JCB_Fixed_Length)	//fixed length
								{
									if(VLength != glv_tagTable[Index].JCB_Length)
										return JCB_Decline;
								}
								else if(glv_tagTable[Index].JCB_Length_Def == JCB_Var_Length)	//var length
								{
									if(VLength>glv_tagTable[Index].JCB_Length)
										return JCB_Decline;
								}
							}
						
							len_addr = (UCHAR *)(glv_addTable[Index]);	//point to glv_addtable length address
							while(LLength--)
							{
								*len_addr++ = *gac_buf++;		//shift buf pointer, and now we're in "value" field
							}
							value_addr = (UCHAR *)(glv_addTable[Index]+3);	//point to glv_addtable value address
							while(VLength--)	//copy data to glv_addTable
							{
								*value_addr++ = *gac_buf++ ;		//shift buf pointer, and now we're in "tag" field
							}
						}
					}
					else
					{
						//if(((*gac_buf) & 0x1F) == 0x1F)
						if(((*gac_buf) & 0xC0)==0xC0)	//CDA 145
						{
							tmplen+=TLength;
							gac_buf+=TLength;

							if(UT_Get_TLVLengthOfL(gac_buf,&LLength))
								tmplen += LLength;
							else
								return JCB_SelectNext;

							if(UT_Get_TLVLengthOfV(gac_buf,&VLength))
								tmplen += VLength;
							else
								return JCB_SelectNext;

							gac_buf += (LLength+VLength);					
						}
						else
						{
							return JCB_Decline;
						}
						
					}
				}
				else
				{
					return JCB_Decline;
				}
			}

			if(tmplen > (JCB_Rcv_Buf_Len-2))	//JCB case Gan 93,
				return JCB_Decline;
		}

		if(redundant == 0x01)
		{
			return JCB_FAIL;
		}
		else
		{
			/*
			if((GenACType & 0x40) || (GenACType == 0x90))	//[TC returned] or [ARQC returned, CDA requested], handle table 6-3
			{
				//check Manadatory Tag
				//20140606, JCB change the SPEC, when madatory data missing, go to End Application, not Decline
				if(!glv_tag9F27.Length[0])
					return JCB_EndApplication;	
				if(!glv_tag9F36.Length[0])
					return JCB_EndApplication;
				if(!glv_tag9F4B.Length[0])
					return JCB_EndApplication;	
				if(!glv_tag9F50.Length[0])
					return JCB_EndApplication;	
			}
			else if(GenACType == 0x80)	//[ARQC returned, CDA not requested]
			{
				//check Manadatory Tag
				if(!glv_tag9F27.Length[0])
					return JCB_EndApplication;
				if(!glv_tag9F36.Length[0])
					return JCB_EndApplication;
				if(!glv_tag9F26.Length[0])
					return JCB_EndApplication;	
				if(!glv_tag9F50.Length[0])
					return JCB_EndApplication;	
			}
			else if((GenACType == 0x00) || (GenACType == 0x10))	//[AAC returned]
			{
				//check Manadatory Tag
				if(!glv_tag9F27.Length[0])
					return JCB_EndApplication;
				if(!glv_tag9F36.Length[0])
					return JCB_EndApplication;
				if(!glv_tag9F26.Length[0])
					return JCB_EndApplication;	
			}
			else
				return JCB_FAIL;		
			*/
			//20140612 change
			if(glv_tag9F27.Length[0])
			{
				if(((glv_tag9F27.Value[0] & 0xC0)==0x40) || (((glv_tag9F27.Value[0] & 0xC0)==0x80) && (GenACType & 0x10)))	//[TC returned] or [ARQC returned, CDA requested], handle table 6-3
				{
					//check Manadatory Tag
					//20140606, JCB change the SPEC, when madatory data missing, go to End Application, not Decline
					if(!glv_tag9F36.Length[0])
						return JCB_Decline;		//20140617 change

					//JCB CL 1.3 Update - Check 9F4B if Card Support CDA
					if (glv_tag82.Value[0] & 0x01)
					{
						if(!glv_tag9F4B.Length[0])
							return JCB_Decline;		//20140617 change
					}
					
					if(!glv_tag9F50.Length[0])
						return JCB_Decline;		//20140617 change
				}
				else if(((glv_tag9F27.Value[0] & 0xC0)==0x80)&&((GenACType & 0x10)==0x00))	//[ARQC returned, CDA not requested]
				{
					//check Manadatory Tag
					if(!glv_tag9F36.Length[0])
						return JCB_Decline;		//20140617 change
					if(!glv_tag9F26.Length[0])
						return JCB_Decline;		//20140617 change
					if(!glv_tag9F50.Length[0])
						return JCB_Decline;		//20140617 change
				}
				else if((glv_tag9F27.Value[0] & 0xC0)==0x00)	//[AAC returned]
				{
					//check Manadatory Tag
					if(!glv_tag9F36.Length[0])
						return JCB_Decline;		//20140617 change
					if(!glv_tag9F26.Length[0])
						return JCB_Decline;		//20140617 change
				}
				else
					return JCB_FAIL;		
			}
			else
				return JCB_Decline;		//20140606, JCB change the SPEC, when madatory data missing, go to End Application, not Decline //20140617 change to decline
		}
	}
	return JCB_SUCCESS;
}

UCHAR JCB_Check_RR_SW12(UCHAR *datSW)
{
	/*if ((datSW[0] == 0x90) && (datSW[1] == 0x00))
		return JCB_SUCCESS;
	else if((datSW[0] == 0x69) && (datSW[1] == 0x85))
		return JCB_SelectNext;
	else
		return JCB_FAIL;*/
	if ((datSW[0] == 0x90) && (datSW[1] == 0x00))
		return JCB_SUCCESS;
	else 
		return JCB_SelectNext;
}

UCHAR JCB_Check_GenerateAC_SW12(UCHAR *datSW)
{
	if ((datSW[0] == 0x90) && (datSW[1] == 0x00))
		return JCB_SW9000;
	else if ((datSW[0] == 0x69) && (datSW[1] == 0x84))
		return JCB_SW6984;
	else if ((datSW[0] == 0x69) && (datSW[1] == 0x86))
		return JCB_SW6986;
	else 
		return JCB_SelectNext;	
}

UCHAR JCB_Check_GPO_SW12(UCHAR *datSW)
{
	/*if ((datSW[0] == 0x90) && (datSW[1] == 0x00))
		return JCB_SUCCESS;
	else if ((datSW[0] == 0x69) && (datSW[1] == 0x85))
		return JCB_SW6985;
	else if ((datSW[0] == 0x6A) && (datSW[1] == 0x80))
		return JCB_SW6A80;
	else
		return JCB_FAIL;*/

	if ((datSW[0] == 0x90) && (datSW[1] == 0x00))
		return JCB_SUCCESS;
	else 
		return JCB_SelectNext;
}

UCHAR JCB_Ker_Generate_AC(UCHAR TxnType,UCHAR TAA_Result)	
{
	UCHAR rspCode = 0;
	UCHAR Type = 0;
	UINT tmpLen=0;
	UCHAR TmpRecoverTextLen = 0;
	
	memset(JCB_Snd_Buf,0x00,sizeof(JCB_Snd_Buf));
	JCB_Snd_Buf_Len = 0;

	memset(JCB_Rcv_Buf,0x00,sizeof(JCB_Rcv_Buf));
	JCB_Rcv_Buf_Len = 0;

	//5.8.1.1 / 5.10.1.1
	UT_Get_TLVLengthOfV(glv_tag8C.Length,&tmpLen);
	if(tmpLen)
	{
		rspCode = DOL_Patch_DOLData(tmpLen,glv_tag8C.Value,(UCHAR *) &JCB_Snd_Buf_Len,JCB_Snd_Buf);
	}
	else
	{
		return JCB_FAIL;
	}

	//5.8.1.4
	//JCB CL 1.3 Update - Requirement 5.8.1.4
	if(rspCode == SUCCESS)
	{
		if(TxnType == JCB_Txn_Legacy)
		{
			Type = 0x80;
		}
		else
		{
			if (TAA_Result == JCB_TAAR_AAC)		Type = 0x00;
			if (TAA_Result == JCB_TAAR_TC)		Type = 0x40;
			if (TAA_Result == JCB_TAAR_ARQC)	Type = 0x80;

			if ((JCB_Static_Parameter.CombinationOption[0] & 0x20) &&
				(glv_tag82.Value[0] & 0x01))
			{
				Type |= 0x10;
			}
		}

		//5.8.1.3 / 5.10.1.3
		rspCode = ECL_APDU_Generate_AC(Type,JCB_Snd_Buf_Len,JCB_Snd_Buf,&JCB_Rcv_Buf_Len,JCB_Rcv_Buf);	

		if ((rspCode == ECL_LV1_TIMEOUT_USER) || (rspCode == ECL_LV1_TIMEOUT_ISO))
			etp_flgCmdTimeout=TRUE;

		if ((rspCode == ECL_LV1_STOP_LATER) || (rspCode == ECL_LV1_STOP_CANCEL))
			return JCB_EndApplication;
		
		if(rspCode == ECL_LV1_SUCCESS)
		{			
			rspCode = JCB_Check_GenerateAC_SW12(&JCB_Rcv_Buf[JCB_Rcv_Buf_Len-2]);
			if(rspCode == JCB_SW9000)
			{
				//5.8.1.8 / 5.10.1.5
				rspCode = JCB_Check_GenerateAC_M_Data(TxnType, Type);
				if(rspCode != JCB_SUCCESS)
					return rspCode;
				else
				{
					if(TxnType == JCB_Txn_Legacy)
					{
						return JCB_SUCCESS;
					}
					else
					{	
						//20140528 we add this for CDA Process
						JCB_Handle_GenAC_Rep_CDA(JCB_Rcv_Buf);
					
						//5.8.1.9
						if((glv_tag9F27.Value[0] & 0xC0) == 0x00)
							return JCB_Decline; //Decline the transaction 5.13.5
	
						//5.8.1.10
						if(JCB_TAA_Result == JCB_TAAR_ARQC) //Request ARQC
						{
							if((glv_tag9F27.Value[0] & 0xC0) == 0x40) //Response TC
								return JCB_Decline; //Decline the transaction 5.13.5
						}
	
						//5.8.1.11
						//JCB CL 1.3 Update - Requirement 5.8.1.11
						if ((JCB_Static_Parameter.CombinationOption[0] & 0x20) && 
							(glv_tag82.Value[0] & 0x01))
						{
							if(glv_tag9F4B.Length[0] == 0x00)
								return JCB_Decline; //Decline the transaction 5.13.5
						}
	
						//5.8.1.12
						if(glv_tag9F4B.Length[0])
						{
							if(((glv_tag9F60.Length[0] == 0x00) || (glv_tag9F60.Value[0] == 0x02)) || ((JCB_Static_Parameter.TerminalInterchangeProfile[1] & 0x80)==0x00 ))
							{
								return JCB_SUCCESS;
							}									
						}
					}
					
					return JCB_SUCCESS;
				}
			}
			else if(rspCode == JCB_SW6986)	//5.8.1.5
			{
				if(TxnType == JCB_Txn_EMV)
					return JCB_EndAppWithRestart;
				else	//Legacy MOde
					return JCB_SelectNext;
			}
			else if(rspCode == JCB_SW6984)	//5.8.1.6
			{
				if(TxnType == JCB_Txn_EMV)
					return JCB_TryAnother;
				else
					return JCB_SelectNext;
			}
			else	//5.8.1.7
			{
				return JCB_SelectNext;
			}			
		}		
		else
		{
			if(	(rspCode == ECL_LV1_ERROR_TRANSMISSION) ||
				(rspCode == ECL_LV1_ERROR_PROTOCOL)	||
				(rspCode == ECL_LV1_TIMEOUT_USER) ||
				(rspCode == ECL_LV1_TIMEOUT_ISO))
			{
				JCB_ReContext_Par.LastTxnType = TxnType;
				JCB_ReContext_Par.LastGenACType = Type;
			
				JCB_ReContext_Par.RecoverEMVFlag = TRUE;

				//Store the card Track 2 Equivalent Data
				UT_Get_TLVLengthOfV(glv_tag57.Length,&tmpLen);				
				if(tmpLen)
				{
					memcpy(JCB_ReContext_Par.TornTrack2Data,glv_tag57.Value,tmpLen);
					tmpLen = 0;
				}

				//If CDA has been requested
				if(Type & 0x10)
				{
					UT_Get_TLVLengthOfV(glv_tag9F38.Length,&tmpLen);

					//Cat PDOL
					DOL_Patch_DOLData(tmpLen,glv_tag9F38.Value,&TmpRecoverTextLen,JCB_ReContext_Par.TornCDAHashDataBuf);

					//Cat CDOL1
					memcpy(&JCB_ReContext_Par.TornCDAHashDataBuf[TmpRecoverTextLen],JCB_Snd_Buf,JCB_Snd_Buf_Len);

					TmpRecoverTextLen+=(UCHAR)JCB_Snd_Buf_Len;
					JCB_ReContext_Par.TornCDAHashDataBufSize = TmpRecoverTextLen;

					//20140521 test
					memcpy(JCB_ReContext_Par.UnPredNum,glv_tag9F37.Value,glv_tag9F37.Length[0]);
				}
					
				return JCB_EndAppWithRestartComm;
			}
	
			return JCB_FAIL;
		}
	}
	else
		return JCB_FAIL;
}

UCHAR JCB_TAA_CheckBitsTVR( UCHAR *iac, UCHAR *tac )
{
	UCHAR i;
	UCHAR byteno;
	UCHAR bitno;
	UCHAR JCB_tmp_TVR[5] = {0x00};

	memcpy(JCB_tmp_TVR,glv_tag95.Value,5);




	for( i=0; i<40; i++ ) // from "msb" to "lsb"
	{
		byteno = i / 8;
		bitno = (i % 8);

		if( (JCB_tmp_TVR[byteno] << bitno) & 0x80  ) // TVR bit value = '1'
		{
			// check the corresponding bit in IAC and TAC
			if( ( (iac[byteno] << bitno) & 0x80 ) || ( (tac[byteno] << bitno) & 0x80 ) )
			return	TRUE;
		}
	}
	return	FALSE;
}

UCHAR JCB_TAA_Check(UCHAR Mode)
{
	UCHAR iac[5];
	UCHAR tac[5];

	switch(Mode)
	{
		case JCB_TAA_Default:	
			memcpy(iac,glv_tag9F0D.Value,5);	
			memcpy(tac,JCB_TAC_Default,5);
		break;
	
		case JCB_TAA_Denial:	
			memcpy(iac,glv_tag9F0E.Value,5);	
			memcpy(tac,JCB_TAC_Decline,5);
		break;

		case JCB_TAA_Online:	
			memcpy(iac,glv_tag9F0F.Value,5);	
			memcpy(tac,JCB_TAC_Online,5);
		break;	
	}

	
	// check corresponding bits in both TVR and IAC or TAC
	if( JCB_TAA_CheckBitsTVR( iac, tac ) == TRUE )
		return JCB_SUCCESS;
	else
		return JCB_FAIL;	
}

UCHAR JCB_Terminal_Action_Analysis(void)
{
	UCHAR rspCode = 0xFF;

	//Denial
	rspCode = JCB_TAA_Check(JCB_TAA_Denial);
	if(rspCode == JCB_SUCCESS)
	{
		JCB_TAA_Result = JCB_TAAR_AAC;
		return JCB_TAAR_Denial;
	}

	//Online
	if(	((glv_tag9F35.Value[0] & 0x0F) == 0x01)	||
		((glv_tag9F35.Value[0] & 0x0F) == 0x02)	||
		((glv_tag9F35.Value[0] & 0x0F) == 0x04)	||
		((glv_tag9F35.Value[0] & 0x0F) == 0x05)	)
	{
		rspCode = JCB_TAA_Check(JCB_TAA_Online);
		if(rspCode == JCB_SUCCESS)
		{
			JCB_TAA_Result = JCB_TAAR_ARQC;
			return JCB_TAAR_Online;		
		}
		else
		{
			JCB_TAA_Result = JCB_TAAR_TC;
			return JCB_TAAR_Online;		
		}
	}
		
	//Default
	if(	((glv_tag9F35.Value[0] & 0x0F) == 0x03)	||
		((glv_tag9F35.Value[0] & 0x0F) == 0x06)	)
	{
		rspCode = JCB_TAA_Check(JCB_TAA_Default);
		if(rspCode == JCB_SUCCESS)
		{
			JCB_TAA_Result = JCB_TAAR_AAC;	
			return JCB_TAAR_Default;
		}
		else
		{
			JCB_TAA_Result = JCB_TAAR_TC;
			return JCB_TAAR_Default;
		}
	}
	return JCB_TAAR_Denial;
}

UCHAR JCB_Check_IAC_Present(void)
{
	UINT tmpLen1=0, tmpLen2=0, tmpLen3=0;
	
	UT_Get_TLVLengthOfV(glv_tag9F0D.Length,&tmpLen1);
	UT_Get_TLVLengthOfV(glv_tag9F0E.Length,&tmpLen2);
	UT_Get_TLVLengthOfV(glv_tag9F0F.Length,&tmpLen3);

	if( tmpLen1 && tmpLen2 && tmpLen3)
			return JCB_SUCCESS;

	return JCB_FAIL;
}

void JCB_Load_Default_IAC(void)
{
	UINT tmpLen = 0;

	UCHAR IAC_Decline[5] = {0x00,0x00,0x00,0x00,0x00};
	UCHAR IAC_Online[5] = {0xFF,0xFF,0xFF,0xFF,0xFF};
	UCHAR IAC_Default[5] = {0xFF,0xFF,0xFF,0xFF,0xFF};

	if(JCB_TxnMode == JCB_Txn_EMV)
	{		
		UT_Get_TLVLengthOfV(glv_tag9F0D.Length,&tmpLen);
		if(tmpLen == 0)
		{
			glv_tag9F0D.Length[0] = 5;
			memcpy(glv_tag9F0D.Value,IAC_Default,5);
		}
	
		tmpLen = 0;
	
		UT_Get_TLVLengthOfV(glv_tag9F0E.Length,&tmpLen);
		if(tmpLen == 0)
		{
			glv_tag9F0E.Length[0] = 5;
			memcpy(glv_tag9F0E.Value,IAC_Decline,5);
		}
	
		tmpLen = 0;
	
		UT_Get_TLVLengthOfV(glv_tag9F0F.Length,&tmpLen);
		if(tmpLen == 0)
		{
			glv_tag9F0F.Length[0] = 5;
			memcpy(glv_tag9F0F.Value,IAC_Online,5);
		}
	}
	else
	{
		glv_tag9F0D.Length[0] = 5;
		memcpy(glv_tag9F0D.Value,IAC_Default,5);

		glv_tag9F0E.Length[0] = 5;
		memcpy(glv_tag9F0E.Value,IAC_Decline,5);

		glv_tag9F0F.Length[0] = 5;
		memcpy(glv_tag9F0F.Value,IAC_Online,5);
	}
}

UCHAR JCB_Check_TAC_Present(void)
{
	if(	(JCB_Static_Parameter.TAC_Denial_Present)&&
		(JCB_Static_Parameter.TAC_Online_Present)&&
		(JCB_Static_Parameter.TAC_Default_Present))
		return JCB_SUCCESS;

	return JCB_FAIL;

}

void JCB_Load_Default_TAC(void)
{
	UCHAR TAC_Decline[5] = {0x04,0x10,0x00,0x00,0x00};
	UCHAR TAC_Online[5] = {0x90,0x60,0x00,0x90,0x00};
	UCHAR TAC_Default[5] = {0x90,0x40,0x00,0x80,0x00};

	if(!JCB_Static_Parameter.TAC_Denial_Present)
		memcpy(JCB_TAC_Decline,TAC_Decline,5);
	else
		memcpy(JCB_TAC_Decline,JCB_Static_Parameter.TAC_Denial,5);

	if(!JCB_Static_Parameter.TAC_Online_Present)
		memcpy(JCB_TAC_Online,TAC_Online,5);
	else
		memcpy(JCB_TAC_Online,JCB_Static_Parameter.TAC_Online,5);

	if(JCB_Static_Parameter.TAC_Default_Present)
		memcpy(JCB_TAC_Default,TAC_Default,5);
	else
		memcpy(JCB_TAC_Default,JCB_Static_Parameter.TAC_Default,5);
}

UCHAR JCB_Ker_Terminal_Action_Analysis(void)
{
	UCHAR rspCode = 0xFF;

	UCHAR TAAR_Decline = FALSE;	//Terminal Action Analysis results 

	//5.7.1.1
	if(glv_tag9C.Value[0] == 0x20)	
	{
		TAAR_Decline = TRUE;
		JCB_TAA_Result = JCB_TAAR_AAC;	//20140528 add, JCB_MAG_CVM100 101
	}
	else
		TAAR_Decline = FALSE;
	
	if(TAAR_Decline == FALSE)
	{
	//5.7.1.2 Issuer Action Code (IAC) Values
		rspCode = JCB_Check_IAC_Present();
	
		if(!((JCB_TxnMode == JCB_Txn_EMV) && (rspCode == JCB_SUCCESS)))
			JCB_Load_Default_IAC();
				
	//5.7.1.3 Terminal Action Code (TAC) Values
		rspCode = JCB_Check_TAC_Present();
	
		if(rspCode != JCB_SUCCESS)
			JCB_Load_Default_TAC();
		else
		{
			memcpy(JCB_TAC_Decline,JCB_Static_Parameter.TAC_Denial,5);
			memcpy(JCB_TAC_Online,JCB_Static_Parameter.TAC_Online,5);
			memcpy(JCB_TAC_Default,JCB_Static_Parameter.TAC_Default,5);
		}
		
	//5.7.1.4 Terminal Action Analysis
		rspCode = JCB_Terminal_Action_Analysis();
		if(rspCode == JCB_TAAR_Denial)
			TAAR_Decline = TRUE;
		else
			TAAR_Decline = FALSE;
	}

	//5.7.1.5 Terminal Action Analysis Completion
	if(JCB_TxnMode == JCB_Txn_Mag)
	{
		return JCB_SUCCESS;	//perform 5.9
	}
	else
	{
		if(TAAR_Decline)
			return JCB_Decline;	//5.13.5
		else
		{
			return JCB_SUCCESS;	//perform 5.8(EMV), or 5.9(Legacy)
		}
	}
}

void JCB_Ker_Processing_Restrictions(void)
{
	UINT Tmp_Len = 0, Tmp_Len2 = 0;
	UCHAR Tmp_AUC[2] = {0x00};						//Application Usage Control

	UCHAR Terminal_Country_Code_Value[2] = {0};		//Terminal Country Code
	UCHAR Issuer_Country_Code_Value[2] = {0};		//Issuer Country Code
	UCHAR Txn_Allow = FALSE;

	UCHAR App_Expired = FALSE, App_Effectived = FALSE;
	UCHAR Card_Date[3] = {0},Card_Tmp_Date[2] = {0};
	UCHAR Card_Eff_Date[4]={0},RTC_Date[20] = {0};

//5.6.1 Application Usage Control Check
	//5.6.1.1 Application Usage Control
	UT_Get_TLVLengthOfV(glv_tag9F07.Length,&Tmp_Len);
	
	if((JCB_TxnMode == JCB_Txn_EMV) && Tmp_Len)
	{	
		memcpy(Tmp_AUC,glv_tag9F07.Value,Tmp_Len);	//Application Usage Control (AUC)	

		if(Tmp_AUC[0] & 0x01)	//JCB_PR_020, follow EMV book3 10.4.2
		{
			UT_Get_TLVLengthOfV(glv_tag9F1A.Length,&Tmp_Len);
			memcpy(Terminal_Country_Code_Value,glv_tag9F1A.Value,Tmp_Len);

			Tmp_Len = 0;	//reset to 0
			
			UT_Get_TLVLengthOfV(glv_tag9F57.Length,&Tmp_Len);	//Issuer Country Code return?
			UT_Get_TLVLengthOfV(glv_tag5F28.Length,&Tmp_Len2);	//Issuer Country Code return?
			if(Tmp_Len || Tmp_Len2)
			{
				if(Tmp_Len)
					memcpy(Issuer_Country_Code_Value, glv_tag9F57.Value,Tmp_Len);
				else
					memcpy(Issuer_Country_Code_Value, glv_tag5F28.Value,Tmp_Len2);
						
				if(memcmp(Terminal_Country_Code_Value,Issuer_Country_Code_Value,2)==0)
				{
					if(glv_tag9C.Value[0] == 0x00)	//Purchase Mode	
					{
						if((Tmp_AUC[0] & 0x20) || (Tmp_AUC[0] & 0x08))		//AUC for domestic goods	or Service	,modify for test case "PROCESSING_RESTRICTION_007"
							Txn_Allow = TRUE;
						else
							Txn_Allow = FALSE;
					}
					else if(glv_tag9C.Value[0] == 0x01)	//Cash Mode
					{
						if(Tmp_AUC[0] & 0x80)			//AUC for domestic cash
							Txn_Allow = TRUE;
						else
							Txn_Allow = FALSE;
					}
					else if(glv_tag9C.Value[0] == 0x09)	//Cashback Mode
					{
						if(Tmp_AUC[1] & 0x80)			//AUC for domestic cashback
							Txn_Allow = TRUE;
						else
							Txn_Allow = FALSE;
					}
					else	//Out of spec
					{
						Txn_Allow = FALSE;
					}
				}
				else
				{
					if(glv_tag9C.Value[0] == 0x00)	//Purchase Mode
					{
						if((Tmp_AUC[0] & 0x10) || (Tmp_AUC[0] & 0x04))		//AUC for International goods or Service	,modify for test case "PROCESSING_RESTRICTION_017"
							Txn_Allow = TRUE;
						else
							Txn_Allow = FALSE;
					}
					else if(glv_tag9C.Value[0] == 0x01)	//Cash Mode
					{
						if(Tmp_AUC[0] & 0x40)			//AUC for International cash
							Txn_Allow = TRUE;
						else
							Txn_Allow = FALSE;
					}
					else if(glv_tag9C.Value[0] == 0x09)	//Cashback Mode
					{
						if(Tmp_AUC[1] & 0x40)			//AUC for International cashback
							Txn_Allow = TRUE;
						else
							Txn_Allow = FALSE;
					}
					else	//Out of spec
					{
						Txn_Allow = FALSE;
					}
				}
			}
			else
				Txn_Allow = FALSE;
		}
		else
			Txn_Allow = FALSE;

		//5.6.1.2
		if(Txn_Allow == FALSE)
			glv_tag95.Value[1] |= 0x10; 	//Requested Service Not Allowed for Card Product	

	}
	
//5.6.2 Application Expiration Date Check
	//5.6.2.1
	if((JCB_TxnMode == JCB_Txn_EMV) || (JCB_TxnMode == JCB_Txn_Legacy))
	{
		Tmp_Len = 0;
		
		UT_Get_TLVLengthOfV(glv_tag5F24.Length,&Tmp_Len);
		if(Tmp_Len)	
		{
			memcpy(Card_Date,glv_tag5F24.Value,Tmp_Len);	//Get Application Expired Date
			//Transfer the YYMMDD to MMYY
			Card_Tmp_Date[0] = Card_Date[1];
			Card_Tmp_Date[1] = Card_Date[0];
			
			if(UT_VerifyCertificateExpDate(Card_Tmp_Date) == FALSE)
				App_Expired = TRUE;
			else
				App_Expired = FALSE;

			//5.6.2.2
			if(App_Expired == TRUE)
				glv_tag95.Value[1] |= 0x40; 	//Expired Application
		}
	}	

//5.6.3 Application Effective Date Check
	//5.6.3.1	
	if((JCB_TxnMode == JCB_Txn_EMV) || (JCB_TxnMode == JCB_Txn_Legacy))
	{
		Tmp_Len = 0;
		
		UT_Get_TLVLengthOfV(glv_tag5F25.Length,&Tmp_Len);
		if(Tmp_Len)	
		{
			//Transfer the YYMMDD to CCYYMMDD
			memcpy(&Card_Eff_Date[1],glv_tag5F25.Value,Tmp_Len);	//Get Application Effectative Date
			Card_Eff_Date[0] = UT_SetCentury((signed char)Card_Eff_Date[1]);

			UT_GetDateTime(RTC_Date);
			memmove(&RTC_Date[1],RTC_Date,19);
			RTC_Date[0] = UT_SetCentury((signed char)RTC_Date[1]);
			
			/*if(UT_VerifyCertificateExpDate(Card_Tmp_Date) == TRUE)
			{
				App_Effectived = FALSE;
			}
			else
			{
				App_Effectived = TRUE;
			}*/
			if(UT_CompareDate(Card_Eff_Date,RTC_Date)>0)
				App_Effectived = FALSE;
			else
				App_Effectived = TRUE;				

			//5.6.3.2	
			if(App_Effectived == FALSE)
				glv_tag95.Value[1] |= 0x20; 	//Application Not Yet Effective
		}	
	}
}

UCHAR JCB_Random_Transaction_Selection(void)
{
	UCHAR rspCode = 0xFF;
//	UCHAR Terminal_Ram_Num = 0;
	UCHAR Terminal_Ram_Num[8] = {0};

	UCHAR Txn_TargetPercent = 0;
	UCHAR Tmp_Percent = 0;

	ULONG txn_amt = 0;
	ULONG tmp_Cless_Floor_Limit = 0;
	ULONG tmp_thresh_hold = 0;

	ULONG MAX_Target=0;
	ULONG Target=0;
	
	while((Terminal_Ram_Num[0] == 0) || (Terminal_Ram_Num[0] > 100))	//The terminal shall generate a random number in the range of 1 to 99
	{
		api_sys_random(Terminal_Ram_Num);
	}	

	//UT_DispHexByte(7,0,Terminal_Ram_Num);

	rspCode = UT_bcdcmp(glv_tag9F02.Value,JCB_Static_Parameter.ThresholdValue_RandomSelection,6);
	if(rspCode == 2)	//Case 1, transaction amount less than the Threshold Value for Biased Random Selection
	{
		rspCode = UT_bcdcmp(Terminal_Ram_Num,&JCB_Static_Parameter.TargetPercent_RandomSelection,1);
		if((rspCode == 0) || (rspCode == 2))
		{
			return JCB_SUCCESS;	//the transaction shall be selected
		}
		else
			return JCB_FAIL;
	}
	else
	{
		rspCode = UT_bcdcmp(glv_tag9F02.Value,JCB_Static_Parameter.ClessFloorLimit,6);
		if(rspCode == 2)
		{
			UT_bcd2hex(5,&glv_tag9F02.Value[1],(UCHAR *)&txn_amt);
			UT_SwapData(4,(UCHAR *)&txn_amt);
			
			UT_bcd2hex(5,&JCB_Static_Parameter.ClessFloorLimit[1],(UCHAR *)&tmp_Cless_Floor_Limit);
			UT_SwapData(4,(UCHAR *)&tmp_Cless_Floor_Limit);
			
			UT_bcd2hex(5,&JCB_Static_Parameter.ThresholdValue_RandomSelection[1],(UCHAR *)&tmp_thresh_hold);
			UT_SwapData(4,(UCHAR *)&tmp_thresh_hold);

			txn_amt -= tmp_thresh_hold;
			tmp_Cless_Floor_Limit -= tmp_thresh_hold;	

			UT_bcd2hex(1,&JCB_Static_Parameter.MAXTargetPercent_RandomSelection,(UCHAR*)&MAX_Target);
			UT_SwapData(4,(UCHAR *)&MAX_Target);
			
			UT_bcd2hex(1,&JCB_Static_Parameter.TargetPercent_RandomSelection,(UCHAR*)&Target);
			UT_SwapData(4,(UCHAR *)&Target);
			Tmp_Percent = MAX_Target - Target;

			if(Tmp_Percent != 0)
			{
				if(tmp_Cless_Floor_Limit == 0)
					return JCB_SUCCESS;
				else
				{
					Txn_TargetPercent = ((Tmp_Percent * txn_amt)/tmp_Cless_Floor_Limit) + Target;
					//UT_DispHexByte(7,3,Txn_TargetPercent);
				}

				if(Terminal_Ram_Num[0] < Txn_TargetPercent)
					return JCB_SUCCESS;
				else
					return JCB_FAIL;
			}	
			else
				return JCB_FAIL;
		}
		else
			return JCB_FAIL;
	}
	return JCB_SUCCESS;
}

UCHAR JCB_Ker_Terminal_Risk_Management(void)
{
	UCHAR tmp95[5]={0x00};
	
	UCHAR rspCode = 0xFF;
	UCHAR SingleAmount[6] = {0x00,0x00,0x00,0x00,0x01,0x00};
	UCHAR result = 0;
	UCHAR PAN_list[10] = {0x00};
	UINT TmpLen=0;
	UCHAR FileIndex = 0;
	
//5.5.1 Contactless Limit Check
	if(JCB_Static_Parameter.ClessTxnLimitLen)
	{
		rspCode=UT_bcdcmp(JCB_Dynamic_Parameter.JCB_AmountAuth, JCB_Static_Parameter.ClessTxnLimit, 6);
		if ((rspCode == 1) || (rspCode == 0))
			return JCB_SelectNext;
	}

//5.5.2 CVM Required Limit Check
	if((JCB_Static_Parameter.ClessCVMLimitLen) && (glv_tag9C.Value[0] != 0x20))
	{
		rspCode=UT_bcdcmp(JCB_Dynamic_Parameter.JCB_AmountAuth, JCB_Static_Parameter.ClessCVMLimit, 6);
		if ((rspCode == 1) || (rspCode == 0))
		{
			glv_tag9F53.Value[0] |= 0x80;
		}		
	}

//5.5.3 Floor Limit Check
	//5.5.3.1
	if(	(JCB_TxnMode == JCB_Txn_Mag)	||
		(JCB_TxnMode == JCB_Txn_Legacy)	||
		(JCB_Static_Parameter.TerminalType & 0x05))
		glv_tag95.Value[3] |= 0x80;

	//Status Check
	if(JCB_Static_Parameter.CombinationOption[0] & 0x40)
	{
		rspCode=UT_bcdcmp(JCB_Dynamic_Parameter.JCB_AmountAuth, SingleAmount, 6);
		if(rspCode == 0)
			glv_tag95.Value[3] |= 0x80;
	}
	
	//5.5.3.2
	if(	(JCB_TxnMode == JCB_Txn_EMV) && JCB_Static_Parameter.ClessFloorLimitLen)
	{
		rspCode=UT_bcdcmp(JCB_Dynamic_Parameter.JCB_AmountAuth, JCB_Static_Parameter.ClessFloorLimit, 6);
		if((rspCode == 1) || (rspCode == 0))
			glv_tag95.Value[3] |= 0x80;		
	}

//5.5.4 Random Transaction Selection
	if((JCB_TxnMode == JCB_Txn_EMV) && (JCB_Static_Parameter.CombinationOption[0] & 0x08) && ((glv_tag95.Value[3] & 0x80) == 0x00))
	{
		rspCode = JCB_Random_Transaction_Selection();	//perform Random Transaction Selection
		if(rspCode == JCB_SUCCESS)
		{
			//glv_tag95.Value[3] |= 0x10;		//while testing PROCESSING_RESTRICTION_010, reader will crush by this command "glv_tag95.Value[3] |= 0x10"
			memcpy(tmp95,glv_tag95.Value,5);
			tmp95[3]|=0x10;
			memcpy(glv_tag95.Value,tmp95,5);
		}
	}
	
//5.5.5 Exception File Check
	if(	(JCB_TxnMode == JCB_Txn_EMV) && 
		(JCB_Static_Parameter.CombinationOption[0] & 0x10))
	{
		memset(PAN_list,0xFF,10);
		UT_Get_TLVLengthOfV(glv_tag5A.Length,&TmpLen);

		if(TmpLen != 0)
		{
			memcpy(PAN_list,glv_tag5A.Value,TmpLen);
			for( FileIndex=0; FileIndex<10; FileIndex++ )
			{	
				if( UT_memcmp( JCB_ExceptionFile[FileIndex].PAN, PAN_list, TmpLen ) == 0 )
			    {
			    	if(glv_tag5F34.Length[0])
			    	{
						if(JCB_ExceptionFile[FileIndex].SN == glv_tag5F34.Value[0])
						{
							result = TRUE;	// matched
					    	break;
						}
					}
				}
			}
		}
		if(result == TRUE)
			glv_tag95.Value[0] |= 0x10;
	
	}

	return JCB_SUCCESS;
}

UCHAR JCB_Check_RR_R_Tag(UINT RRR_Len,UCHAR *RRR_Buf)
{
	UCHAR redundant=0,tag9F02[4] = {0x9F,0x02,0x00,0x00},tag5F20[4] = {0x5F,0x20,0x00,0x00};
	UCHAR TLength,LLength,tmplen=0,*value_addr=0,*len_addr=0,rspCode;
	UINT tmp_tag_len,VLength;
	ULONG Index;

	//Check the Format First
	UT_Get_TLVLength(RRR_Buf,&TLength,&LLength,&VLength);

	if(!((RRR_Buf[TLength+LLength+VLength] == 0x90) && (RRR_Buf[TLength+LLength+VLength+1] == 0x00)))
		return JCB_SelectNext;

	if(RRR_Buf[0] != 0x70)
		return JCB_SelectNext;
	
	while(tmplen != (RRR_Len-2))
	{
		TLength = LLength = tmp_tag_len = 0;
		Index = 0;

		//is tag a Primitive data element ?
		if((*RRR_Buf & 0x20) && (*RRR_Buf != 0xFF))			//it's a Constructed data element
		{	
			if(*RRR_Buf & 0xC0)	//JCB_RR_140,In EMV Book3, tag34 is considered as a Constructed tag, but this tag is not allowed in JCB .
			{
				if(UT_Get_TLVLengthOfT(RRR_Buf,&TLength))	
				{
					tmplen += TLength;			
					RRR_Buf += TLength;			//shift to "length" field
				}
				else
					return JCB_SelectNext;	
				
				if(UT_Get_TLVLengthOfL(RRR_Buf,&LLength))	//RR107 tag65 
				{
					tmplen += LLength;
					RRR_Buf += LLength;			//shift buf pointer, and now we are in "Constructed data" field
				}
				else
					return JCB_SelectNext;	
			}
			else
			{
				return JCB_SelectNext;
			}			
		}
		else if((*RRR_Buf == 0x00) || (*RRR_Buf == 0xFF))	//ICTK, padding with 0x00,oxFF
		{
			tmplen += 1;
			RRR_Buf += 1;
		}
		else				//this tag is a Primitive data element
		{
			UT_Get_TLVLengthOfT(RRR_Buf,&TLength);	//get tag's Length

			rspCode = UT_Search(TLength,RRR_Buf,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);	//search glv_tagTable and get the index	
			if(rspCode == SUCCESS)
			{			
				JCB_Add_Buf(JCB_RR_Tag,TLength,RRR_Buf);
				
				if(*glv_addTable[Index] != 0)		//test if it is redundant?
				{	
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
				else	//copy "Read Record Response" to TAG table
				{
					tmplen += TLength;
					RRR_Buf += TLength;			//shift buf pointer, and now we are in "length" field

					if(UT_Get_TLVLengthOfL(RRR_Buf,&LLength))	//get length's Length		
						tmplen += LLength;
					else
						return JCB_SelectNext;
					
					if(UT_Get_TLVLengthOfV(RRR_Buf,&tmp_tag_len))
						tmplen += tmp_tag_len;
					else
						return JCB_SelectNext;

					//JCB RR 054 , length of 5F24 is bigger than the defination of spec, 20140527, add "tmp_tag_len" checking for JCB_RR_100
					if((glv_tagTable[Index].JCB_Length) && (tmp_tag_len))	
					{
						if(glv_tagTable[Index].JCB_Length_Def == JCB_Fixed_Length)	//fixed length
						{
							if(tmp_tag_len != glv_tagTable[Index].JCB_Length)
								return JCB_SelectNext;
						}
						else if(glv_tagTable[Index].JCB_Length_Def == JCB_Var_Length)	//var length
						{
							if(!memcmp(glv_tagTable[Index].Tag,tag5F20,4))
							{
								if(tmp_tag_len>glv_tagTable[Index].JCB_Length)
									return JCB_SelectNext;

								if(tmp_tag_len<2)	//JCB READ_RECORD_158 , the length of tag5F20 should between 2~26
									return JCB_SelectNext;
							}
							else
							{
								if(tmp_tag_len>glv_tagTable[Index].JCB_Length)
									return JCB_SelectNext;
							}
						}
					}
					
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

				if(UT_Get_TLVLengthOfL(RRR_Buf,&LLength))	//get length's Length
					tmplen += LLength;
				else
					return JCB_SelectNext;	//JCB READ_RECORD_088
	
				if(UT_Get_TLVLengthOfV(RRR_Buf,&tmp_tag_len))
					tmplen += tmp_tag_len;	
				else
					return JCB_SelectNext;	//JCB READ_RECORD_088

				RRR_Buf += LLength;				
				RRR_Buf += tmp_tag_len;
			}
		}

		if(tmplen > (RRR_Len-2))	//JCB case Read Record 101, TLV format of tag 8F is wrong, JCB case Read Record 103, TLV format of tag 9F32 is wrong
			return JCB_SelectNext;
	}
	if(redundant == 0x01)
	{
		return JCB_SelectNext;
	}	
	else
		return JCB_SUCCESS;
}


UCHAR JCB_RAD_SAD_Data(UCHAR SFINUM,UINT RECNUM,UINT RRR_Len,UCHAR *RRR_Buf)
{
	//We only store the record that depends on AFL Byte4 
	//Store whole Tag70 for OFDA
	if(RRR_Buf[0] == 0x70)
	{		
		// (1) check record LENGTH & template TAG=70
		if( (RRR_Buf[1] & 0x80) == 0 )
		{
		  	memcpy((ADDR_JCB_REC_START+JCB_REC_LEN*(RECNUM)),&SFINUM,1);
		  	memcpy((ADDR_JCB_REC_START+JCB_REC_LEN*(RECNUM))+1,RRR_Buf,RRR_Len-2);	//minus SW1,SW2
		}
		else // 2-byte length
		{
			switch( RRR_Buf[1] & 0x7f )
			{
				case 0x01: // 1-byte length (128..255)
					memcpy((ADDR_JCB_REC_START+JCB_REC_LEN*(RECNUM)),&SFINUM,1);
		  			memcpy((ADDR_JCB_REC_START+JCB_REC_LEN*(RECNUM))+1,RRR_Buf,RRR_Len-2);	//minus SW1,SW2
					break;
					
				case 0x02: // 2-byte length (256..65535)
					memcpy((ADDR_JCB_REC_START+JCB_REC_LEN*(RECNUM)),&SFINUM,1);
		  			memcpy((ADDR_JCB_REC_START+JCB_REC_LEN*(RECNUM))+1,RRR_Buf,RRR_Len-2);	//minus SW1,SW2
					break;
				default:   // out of spec
					return JCB_FAIL;
			}
		}
	}
	return JCB_SUCCESS;
}


UCHAR JCB_RAD_Process(void)
{
	UCHAR SFINUM=0,rspCode,AFL_B4=0,j;
	UINT tag94_len=0,i,RECNUM=0;

	UINT lenOfV;

	UT_Get_TLVLengthOfV(glv_tag94.Length,&tag94_len);

	ODA_Clear_Record();

	for(i=0;i<tag94_len;i+=4)
	{
		SFINUM = glv_tag94.Value[i];
		SFINUM = SFINUM>>3;

		AFL_B4 = glv_tag94.Value[i+3];
						
		for(j=glv_tag94.Value[i+1];j<=glv_tag94.Value[i+2];j++)
		{
			JCB_Rcv_Buf_Len= 0;
			memset(JCB_Rcv_Buf,0,DEP_BUFFER_SIZE_RECEIVE);

			rspCode = ECL_APDU_Read_Record(j,SFINUM,&JCB_Rcv_Buf_Len,JCB_Rcv_Buf);		

			if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
				etp_flgCmdTimeout=TRUE;

			if ((rspCode == ECL_LV1_STOP_LATER) || (rspCode == ECL_LV1_STOP_CANCEL))
				return JCB_EndApplication;
			
			if(rspCode == ECL_LV1_SUCCESS)
			{				
				rspCode = JCB_Check_RR_SW12(&JCB_Rcv_Buf[JCB_Rcv_Buf_Len-2]);
				if(rspCode != JCB_SUCCESS)
				{
					return rspCode;
				}
				
				if(AFL_B4)
				{
					rspCode = JCB_RAD_SAD_Data(SFINUM,RECNUM,JCB_Rcv_Buf_Len,JCB_Rcv_Buf);
					if(rspCode == JCB_SUCCESS)
					{
						AFL_B4--;
						RECNUM++;
					}
					else
					{
						return JCB_SelectNext;
					}
				}
			
				rspCode = JCB_Check_RR_R_Tag(JCB_Rcv_Buf_Len,JCB_Rcv_Buf);
				if(rspCode == JCB_SelectNext)
				{
					return JCB_SelectNext;
				}

				//5.14.5.4
				//JCB CL 1.3 Update - Check Torn PAN Consistency
				if (JCB_ReContext_Par.RecoverEMVFlag == TRUE)
				{
					UT_Get_TLVLengthOfV(glv_tag57.Length, &lenOfV);
					if (lenOfV)
					{
						if (memcmp(JCB_ReContext_Par.TornTrack2Data, glv_tag57.Value, lenOfV))
						{
							memset(&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));
							return JCB_EndApplication;
						}
					}
				}
			}
			else
			{
				if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
					return JCB_EndAppWithRestartComm;
				else
					return JCB_FAIL;
			}
		}
	}
	return JCB_SUCCESS;
}

UCHAR JCB_Ker_Read_Application_Data(void)
{
	UINT tempLen = 0,NI_Length=0,NCA_Length=0;
	UCHAR rspCode = 0xFF;
	UCHAR i,Index_Not_Found = 0;
	
	UT_Get_TLVLengthOfV(glv_tag94.Length,&tempLen);

	if(tempLen)	
	{
		//5.4.1.1
		rspCode = JCB_RAD_Process();
	}
	else	//Didn't perform "Read Record" Command
		rspCode = JCB_SUCCESS;

	if(rspCode == JCB_SUCCESS)
	{
		//Test, We check 5F24, 5F25 format in here	
		if(glv_tag5F24.Length[0])
		{
			if(!UT_CheckYearMonthDate(glv_tag5F24.Value))
				return JCB_SelectNext;
		}

		if(glv_tag5F25.Length[0])
		{
			if(!UT_CheckYearMonthDate(glv_tag5F25.Value))
				return JCB_SelectNext;
		}
		//Test end	
	
		//5.4.1.2
		if((JCB_TxnMode == JCB_Txn_EMV) || ((JCB_TxnMode == JCB_Txn_Legacy)))
		{
			if(!((glv_tag8C.Length[0]) && (glv_tag57.Length[0]) && (glv_tag5F24.Length[0])))
				return JCB_SelectNext;		
		}

		//5.4.1.3
		//JCB CL 1.3 Update - Requirememt 5.4.1.3
		if ((JCB_TxnMode == JCB_Txn_EMV) && 
			(JCB_Static_Parameter.CombinationOption[0] & 0x20) &&
			(glv_tag82.Value[0] & 0x01))
		{
			/*if(!((glv_tag8F.Length[0]) && 
				(glv_tag90.Length[0]) && 
				(glv_tag9F32.Length[0]) && 
				(glv_tag9F46.Length[0])	&&
				(glv_tag9F47.Length[0])	&& 
				(glv_tag92.Length[0])))	
			{
				glv_tag95.Value[0] |= 0x24;	
			}*/
			//20140612 change CDA_VERIFICATION_069 070 
			if(!((glv_tag8F.Length[0]) && 
				(glv_tag90.Length[0]) && 
				(glv_tag9F32.Length[0]) && 
				(glv_tag9F46.Length[0])	&&
				(glv_tag9F47.Length[0])))	
			{
				glv_tag95.Value[0] |= 0x24;	
			}
			//20140612 add	CDA_VERIFICATION_069 070 
			if(glv_tag9F46.Length[0] && glv_tag90.Length[0])
			{
				UT_Get_TLVLengthOfV(glv_tag9F46.Length,&NI_Length);
				UT_Get_TLVLengthOfV(glv_tag90.Length,&NCA_Length);

				if((NI_Length-NCA_Length+36)>0)
				{
					if(!glv_tag92.Length[0])
						glv_tag95.Value[0] |= 0x24;	
				}
			}
		}

		//5.4.1.4
		//JCB CL 1.3 Update - Requirement 5.4.1.4
		if((JCB_TxnMode == JCB_Txn_EMV) && (JCB_Static_Parameter.CombinationOption[0] & 0x20) && (glv_tag8F.Length[0]))
		{
			for(i=0;i<CAPK_NUMBER;i++)
			{
				if((glv_CAPK[i].Index == glv_tag8F.Value[0]) && (!memcmp(glv_CAPK[i].RID,glv_tag9F06.Value,5)))
				{
					Index_Not_Found = FALSE;
					break;
				}
				else
					Index_Not_Found = TRUE;				
			}
			if(Index_Not_Found)
				glv_tag95.Value[0] |= 0x04;
		}
		
		return JCB_SUCCESS;
	}
	else
		return rspCode;
}

UCHAR JCB_Check_GPO_R_Data (UINT GPOR_Len,UCHAR *GPOR_Buf)
{
	UCHAR TLength,LLength=0,*value_addr=0,*len_addr=0,rspCode,redundant=0,tag9F02[4] = {0x9F,0x02,0x00,0x00};
	ULONG Index;
	UINT  tmplen=0,VLength=0;
	
	while(tmplen != (GPOR_Len-2))
	{
		TLength = LLength = VLength = 0;
		Index = 0;
		
		//Get first byte tag to check
		//is tag a Primitive data element ?
		if((*GPOR_Buf & 0x20) && (*GPOR_Buf != 0xFF))		//it's a Constructed data element
		{	
			UT_Get_TLVLengthOfT(GPOR_Buf,&TLength);	//get tag's Length
			UT_Get_TLVLengthOfL(&GPOR_Buf[TLength],&LLength);	//get length's Length
			UT_Get_TLVLengthOfV(&GPOR_Buf[TLength],&VLength);	//get Value's Length

			//JCB CL 1.3 Update - SW12 of Torn GPO Response is 6200 or 9000
			if (JCB_ReContext_Par.RecoverEMVFlag == TRUE)
			{
				if ((!((GPOR_Buf[TLength+LLength+VLength] == 0x62) && (GPOR_Buf[TLength+LLength+VLength+1] == 0x00))) &&
					(!((GPOR_Buf[TLength+LLength+VLength] == 0x90) && (GPOR_Buf[TLength+LLength+VLength+1] == 0x00))))
					return JCB_FAIL;
			}
			else
			{
				if(!((GPOR_Buf[TLength+LLength+VLength] == 0x90) && (GPOR_Buf[TLength+LLength+VLength+1] == 0x00)))
					return JCB_FAIL;
			}

			if(GPOR_Buf[0] != 0x77)			//JCB GPO 056, BF0D present, just ignore it
			{
				tmplen += TLength;	
				GPOR_Buf += TLength;						
				tmplen += LLength;
				GPOR_Buf += LLength;			
				tmplen += VLength;
				GPOR_Buf += VLength;		
			}
			else
			{
				tmplen += TLength;	
				GPOR_Buf += TLength;						
				tmplen += LLength;
				GPOR_Buf += LLength;			//shift buf pointer, and now we are in "Constructed data" field
			}
		}
		else if((*GPOR_Buf == 0x00)||(*GPOR_Buf == 0xFF))			//ICTK, padding with 0x00
		{
			tmplen += 1;	
			GPOR_Buf += 1;
		}
		else				//this tag is a Primitive data element
		{
			UT_Get_TLVLengthOfT(GPOR_Buf,&TLength);	//get tag's Length

			rspCode = UT_Search(TLength,GPOR_Buf,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);	//search glv_tagTable and get the index 
			if(rspCode == SUCCESS)
			{
				JCB_Add_Buf(JCB_GPO_Tag,TLength,GPOR_Buf);
					
				len_addr = (UCHAR *)(glv_addTable[Index]);	//point to glv_addtable length address
				
				if(*len_addr != 0)
				{
					if(memcmp(glv_tagTable[Index].Tag,tag9F02,4))
					{					
						redundant = 0x01;
						break;
					}
					else
					{
						tmplen += TLength;
						GPOR_Buf += TLength;			//shift buf pointer, and now we are in "length" field

						UT_Get_TLVLengthOfL(GPOR_Buf,&LLength);				//get length's Length
						UT_Get_TLVLengthOfV(GPOR_Buf,&VLength);				//get Value's Length
						tmplen += LLength;
						tmplen += VLength;

						GPOR_Buf += LLength;
						GPOR_Buf += VLength;
					}
				}
				else
				{
					tmplen += TLength;
					GPOR_Buf += TLength;			//shift buf pointer, and now we are in "length" field

					UT_Get_TLVLengthOfL(GPOR_Buf,&LLength);				//get length's Length
					UT_Get_TLVLengthOfV(GPOR_Buf,&VLength);				//get Value's Length
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
			}
			else
			{
				if(((*GPOR_Buf)&0x1F) == 0x1F)
				{
					tmplen += TLength;
					GPOR_Buf += TLength;			//shift buf pointer, and now we are in "length" field

					if(UT_Get_TLVLengthOfL(GPOR_Buf,&LLength))				//get length's Length
						tmplen += LLength;
					else
						return JCB_SelectNext;
					
					if(UT_Get_TLVLengthOfV(GPOR_Buf,&VLength))				//get Value's Length
						tmplen += VLength;
					else
						return JCB_SelectNext;					

					GPOR_Buf += (LLength+VLength);
				}
				else
					return JCB_SelectNext;	//JCB GPO 111,when tag can't recongize return Select Next
			}
		}		

		if(tmplen > (GPOR_Len-2))	
			return JCB_SelectNext;
	}

	if(redundant == 0x01)
		return JCB_FAIL;
	else 
		return JCB_SUCCESS;
		
}

// ---------------------------------------------------------------------------
// FUNCTION: verify the contents of all AFL entries.
// INPUT   : none.
// OUTPUT  : none.
// REF     : glv_tag94.Value
// RETURN  : TURE  - correct AFL.
//           FALSE - incorrect AFL.
// ---------------------------------------------------------------------------
UCHAR JCB_Verify_AFL( UCHAR *AFL_Data,UINT AFL_Length)
{
	UCHAR i;
	UCHAR sfi;
	UCHAR st_rec_num;
	UCHAR end_rec_num;
	UCHAR oda_rec_cnt;

	// check all AFL entries
	for( i=0; i<AFL_Length ; i+=4)
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


/*
JCB_AIP_AFL_Process()

Input : None
Output : None
Return : JCB_Success, JCB_SelectNext

If there any error occurs, kernel will send a "Select Next" Outcome.
*/
UCHAR JCB_AIP_AFL_Process (void)
{	
	UCHAR *data_point = 0;
	UCHAR AIP[2] = {0};
	UCHAR cnt,AFL_Data[252]={0};
	UINT Value_Len=0,AFL_Length=0;
	UCHAR tag82[1]={0x82};
	UCHAR tag94[1]={0x94};

    if(JCB_TxnMode == JCB_Txn_Legacy)
    {
		// Check response format (1)
    	if( JCB_Rcv_Buf[0] == 0x80 )
	    {
			UT_EMVCL_GetBERLEN( &JCB_Rcv_Buf[1], &cnt ,&Value_Len); // Get total length of data elements
			// Check legal length
		    if( Value_Len < 6 )
		    	return JCB_SelectNext;

		
	       	// save AIP[2]
	        AIP[0] = JCB_Rcv_Buf[1+cnt]; // 2+(cnt-1)
	        AIP[1] = JCB_Rcv_Buf[2+cnt]; // 3+(cnt-1)

			glv_tag82.Length[0] = 0x02;
	        memcpy(glv_tag82.Value,AIP,2);
		  
	        // save AFL[4n]
		    if((Value_Len-2)%4 == 0)
		    {
		    	AFL_Length = Value_Len-2;
	        	memcpy(AFL_Data,&JCB_Rcv_Buf[3+cnt],AFL_Length); 

				//To verify AFL before actual reading records.
				if( JCB_Verify_AFL(AFL_Data,AFL_Length) == FALSE )
	    			return	JCB_SelectNext;
				else
				{			
					if(AFL_Length < 128)
						glv_tag94.Length[0] = AFL_Length;
					else if( 128 <= AFL_Length )
					{
						glv_tag94.Length[0] = 0x81;
						glv_tag94.Length[1] = AFL_Length;
					}
					else
					{
						glv_tag94.Length[0] = 0x82;
						glv_tag94.Length[1] = (AFL_Length & 0xFF00)>>8;
						glv_tag94.Length[2] = AFL_Length&0x00FF;
					}
			
					memcpy(glv_tag94.Value,AFL_Data,AFL_Length)	;
				}
		    }
			else
				return JCB_SelectNext;
		}
		else
			return JCB_SelectNext;
	}
	else
	{		
		// check response format (2)
		if( (JCB_Rcv_Buf[0] == 0x77))
		{
			//5.3.1.3
			// find AIP[2]
			//data_point = apk_EMVCL_FindTag_withLen( 0x82, 0x00, JCB_Rcv_Buf, JCB_Rcv_Buf_Len);
			data_point = UT_Find_Tag( tag82, JCB_Rcv_Buf_Len, JCB_Rcv_Buf);
			if( data_point != NULLPTR)
			{
				/**
				 * Because the Find_Tag retrun pointer start from tag, but
				 * the GetBERLEN is work assume the first byte is len, need to 
				 * add 1, to point to the length
				 */ 
				data_point++;
				UT_EMVCL_GetBERLEN( data_point , &cnt ,&Value_Len);
				if((Value_Len==0) || (Value_Len != 2))
					return JCB_SelectNext;
	
				// save AIP[2]
				AIP[0] = *(data_point+cnt);
				AIP[1] = *(data_point+cnt+1);

	
				//5.3.1.4
				if(JCB_TxnMode == JCB_Txn_Undefined)
				{
					if(AIP[1] & 0x80)
						JCB_TxnMode = JCB_Txn_EMV;
					else
						JCB_TxnMode = JCB_Txn_Mag;
				}
			}
			else
				return JCB_SelectNext; // AIP not present

			//5.3.1.6
			// find AFL[4n]
//			data_point = apk_EMVCL_FindTag_withLen( 0x94, 0x00, JCB_Rcv_Buf, JCB_Rcv_Buf_Len);
			data_point = UT_Find_Tag( tag94, JCB_Rcv_Buf_Len, JCB_Rcv_Buf);
			if( data_point != NULLPTR)
			{
				/**
				 * Because the Find_Tag retrun pointer start from tag, but
				 * the GetBERLEN is work assume the first byte is len, need to 
				 * add 1
				 */ 
				data_point++;
				UT_EMVCL_GetBERLEN( data_point, &cnt ,&Value_Len);
	
				if(!Value_Len)	//JCB_GPO_111
				{
					if((JCB_TxnMode == JCB_Txn_EMV) || (JCB_TxnMode == JCB_Txn_Legacy))
						return JCB_SelectNext;
				}
				
				// save AFL[4n]
				if((Value_Len % 4)==0)
				{
					AFL_Length = Value_Len;
					memcpy(AFL_Data,&data_point[cnt],AFL_Length);
				}
				else
					return JCB_SelectNext;	//JCB_GPO_110
	
				//To verify AFL before actual reading records.
				if( JCB_Verify_AFL(AFL_Data,AFL_Length) == FALSE )
					return	JCB_SelectNext;
			}
			else	//5.3.1.5
			{
				if((JCB_TxnMode == JCB_Txn_EMV) || (JCB_TxnMode == JCB_Txn_Legacy))
					return JCB_SelectNext;
			}
		}
		else
			return JCB_SelectNext;
	} 
	return JCB_SUCCESS; // done		
}

//GPO Response
UCHAR JCB_GPO (void)
{
	UCHAR rspCode = 0;
	UINT  PDOL_Len = 0;

	JCB_Snd_Buf_Len = 0;
	memset(JCB_Snd_Buf,0x00,DEP_BUFFER_SIZE_SEND);

	JCB_Rcv_Buf_Len = 0;
	memset(JCB_Rcv_Buf,0x00,DEP_BUFFER_SIZE_RECEIVE);
	
	//5.3.1.1	
	//JCB CL 1.3 Update - Requirement 5.3.1.1
	UT_Get_TLVLengthOfV(glv_tag9F38.Length,&PDOL_Len);
	rspCode = DOL_Get_DOLData(PDOL_Len,glv_tag9F38.Value,(UCHAR *)&JCB_Snd_Buf_Len,JCB_Snd_Buf);
	if(rspCode == SUCCESS)
	{	
		//send GPO Command
		rspCode = ECL_APDU_Get_ProcessingOptions(JCB_Snd_Buf_Len,JCB_Snd_Buf,&JCB_Rcv_Buf_Len,JCB_Rcv_Buf);
		
		if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
			etp_flgCmdTimeout=TRUE;
			
		if ((rspCode == ECL_LV1_STOP_LATER) || (rspCode == ECL_LV1_STOP_CANCEL))
			return JCB_EndApplication;
	}
	else
		return rspCode;
	
	if(rspCode == ECL_LV1_SUCCESS)
		return JCB_SUCCESS;
	else
		return rspCode;
}

//Issuing GPO command and handle the response
UCHAR JCB_Ker_Initiate_App_Process(void)
{
	UCHAR rspCode = 0xFF;
	
	//5.3.1.1 ~ 2
	rspCode = JCB_GPO();
	if(rspCode != JCB_SUCCESS)
	{
		//JCB CL 1.3 Update - Restart Transaction
		if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
			return JCB_EndAppWithRestartComm;
		else
			return JCB_FAIL;
	}

	//Check Response Status
	rspCode = JCB_Check_GPO_SW12(&JCB_Rcv_Buf[JCB_Rcv_Buf_Len-2]);
	if(rspCode != JCB_SUCCESS)
	{
		//JCB CL 1.3 Update - Requirement 5.14.4.1
		if (JCB_ReContext_Par.RecoverEMVFlag == TRUE)
		{
			rspCode=UT_Check_SW12(&JCB_Rcv_Buf[JCB_Rcv_Buf_Len-2], 0x6200);
			if (rspCode == FALSE)
				return JCB_SelectNext;
		}
		else
		{
			return JCB_SelectNext;
		}
	}
		
	//Check GPO response
	if((JCB_Rcv_Buf[0] != 0x77) && (JCB_Rcv_Buf[0] != 0x80)){
		return JCB_SelectNext;
	}

	//5.3.1.3 ~ 5.3.1.6
	rspCode = JCB_AIP_AFL_Process();
	if(rspCode == JCB_SUCCESS)
	{
		//Check length and Record the Data element
		if(JCB_Rcv_Buf[0] == 0x77)
		{
			rspCode = JCB_Check_GPO_R_Data(JCB_Rcv_Buf_Len,JCB_Rcv_Buf);
			if(rspCode != JCB_SUCCESS)
				return JCB_SelectNext;
		}
		else
		{
			JCB_Add_Buf(JCB_GPO_Tag,1,(UCHAR *)"0x82");
			JCB_Add_Buf(JCB_GPO_Tag,1,(UCHAR *)"0x94");
		}
	}
	else{	//Select Next
		return JCB_SelectNext;
	}

	//5.3.1.7
	//JCB CL 1.3 Update - Requirement 5.3.1.7
	if((JCB_TxnMode == JCB_Txn_Mag) || 
		(JCB_TxnMode == JCB_Txn_Legacy) || 
		((JCB_TxnMode == JCB_Txn_EMV) && ((JCB_Static_Parameter.CombinationOption[0] & 0x20)==0x00)) ||
		((JCB_TxnMode == JCB_Txn_EMV) && ((glv_tag82.Value[0] & 0x01) == 0x00)))
		glv_tag95.Value[0] |= 0x80; //offline Data Authentication was not performed

	return JCB_SUCCESS;
}

UCHAR	JCB_Ker_Txn_Initialisation(void)
{
	UCHAR *Tag_TCI = 0;	//Terminal Compatibility Indicator
	UINT PDOL_Len = 0;
	UCHAR tag9F52[2]={0x9F,0x52};

	//5.2.1.1
	if(JCB_ReContext_Par.RecoverEMVFlag)
	{
		//5.14 Torn Transaction Recovery
		return JCB_TornTransactionRecovery;
	}
	else
	{
		//20140521 test for "Torn Recover Txn"
		JCB_GPO_Buf_Tags_Length = 0;
		JCB_RR_Buf_Tags_Length = 0;
		JCB_GenAC_Buf_Tags_Length = 0;
	
		//5.2.1.2
		if(JCB_Dynamic_Parameter.AuthRspCodeLen)
		{
			glv_tag8A.Length[0] = JCB_Dynamic_Parameter.AuthRspCodeLen;
			memcpy(glv_tag8A.Value,JCB_Dynamic_Parameter.AuthRspCode,JCB_Dynamic_Parameter.AuthRspCodeLen);
		
			if(	(JCB_Dynamic_Parameter.JCB_IADLen) ||
				(JCB_Dynamic_Parameter.JCB_IST_1Len)||
				(JCB_Dynamic_Parameter.JCB_IST_2Len))
			{
				//20140529 Add
				if(JCB_Dynamic_Parameter.JCB_IADLen)
				{
					glv_tag91.Length[0] = JCB_Dynamic_Parameter.JCB_IADLen;
					memcpy(glv_tag91.Value,JCB_Dynamic_Parameter.JCB_IAD,JCB_Dynamic_Parameter.JCB_IADLen);
				}
				
				//5.2.1.3	 Issuer Update
				return JCB_IssuerUpdate;
			}
			else
			{
				//20140620 add for COMBINED_TEST_027 , display the ARC result
				if(	((JCB_Dynamic_Parameter.AuthRspCode[0] == 0x30) && (JCB_Dynamic_Parameter.AuthRspCode[1] == 0x30)) ||
					((JCB_Dynamic_Parameter.AuthRspCode[0] == 0x31) && (JCB_Dynamic_Parameter.AuthRspCode[1] == 0x30)) ||
					((JCB_Dynamic_Parameter.AuthRspCode[0] == 0x31) && (JCB_Dynamic_Parameter.AuthRspCode[1] == 0x31)))
				{
					api_pcd_JCB_Approval();
				}
				else
				{
					api_pcd_JCB_Decline();
				}
				//20140620 add for COMBINED_TEST_027 , display the ARC result end
			
				//20140611 add
				JCB_Dynamic_Parameter.AuthRspCodeLen = 0;
				memset(JCB_Dynamic_Parameter.AuthRspCode,0x00,sizeof(JCB_Dynamic_Parameter.AuthRspCode));
				return JCB_EndApplicationWithARC;	//20140620 change  for COMBINED_TEST_027
			}
		}
		else
		{
			//5.2.1.4 Check FCI format
			if(JCB_JCB_FCI_Error == TRUE)	//FCI is not parsed correctly
			{
				JCB_JCB_FCI_Error = FALSE;	//Reset
				//Select Next Outcome
				return JCB_SelectNext;
			}
			else
			{
				//5.2.1.5	reset TVR tag95 to zero, set tag9F52 to 01 
				memset(glv_tag95.Value,0x00,5);
				glv_tag95.Length[0] = 0x05;

				glv_tag9F52.Length[0]= 0x01;
				glv_tag9F52.Value[0] = 0x01;	//Magstripe Mode
				
				//5.2.1.6	reset Online Transaction Context, Transaction Mode to 'Undefined Mode'
				memset(&JCB_Online_Context,0x00,sizeof(JCB_Online_Context));
				JCB_TxnMode = JCB_Txn_Undefined;

				//5.2.1.7 
				if((JCB_Kernel_Support & JCB_Txn_EMV) && (JCB_Static_Parameter.CombinationOption[0] & 0x02))
				{
					glv_tag9F52.Value[0] |= 0x02;
				}

				//5.2.1.8 Terminal Interchange Profile
				//JCB CL 1.3 Update - Requirement 5.2.1.8
				glv_tag9F53.Length[0] = 0x03;
				memcpy(glv_tag9F53.Value,JCB_Static_Parameter.TerminalInterchangeProfile,3);
				glv_tag9F53.Value[0] &= 0x7F;
				
				if(JCB_Kernel_Support & JCB_Txn_ISUSupport)
				{
					glv_tag9F53.Value[1] |= 0x80;
				}
				else
				{
					glv_tag9F53.Value[1] &= 0x7F;
				}
				
				//5.2.1.9 Legacy Mode Detection
				//JCB CL 1.3 Update - Requirement 5.2.1.9
				UT_Get_TLVLengthOfV(glv_tag9F38.Length, &PDOL_Len);
//				Tag_TCI = apk_EMVCL_FindTag_withLen(0x9F,0x52,glv_tag9F38.Value,PDOL_Len);
				Tag_TCI = UT_Find_Tag(tag9F52, PDOL_Len, glv_tag9F38.Value);
				if(Tag_TCI)
				{
					return JCB_SUCCESS;	//5.3 Initiate Application Processing
				}
				else	//legacy mode
				{
					//5.2.1.10
					if((JCB_Kernel_Support & JCB_Txn_Legacy) && (JCB_Static_Parameter.CombinationOption[0] & 0x01))
					{
						JCB_TxnMode |= JCB_Txn_Legacy;
						return JCB_SUCCESS;	//5.3 Initiate Application Processing
					}
					else
					{
						return JCB_SelectNext;	//Select Next
					}
				}
			}			
		}
	}
}

void 	JCB_Start_Kernel(UINT lenFCI,UCHAR * datFCI)
{
	UCHAR rspCode = 0xFF;
	UINT tmp_len = 0;

	//avoid compiler warning
	lenFCI = lenFCI;
	datFCI[0] = datFCI[0];

	rspCode=JCB_Allocate_Buffer();
	if (rspCode == FAIL)
	{
		return;
	}


	if(JCB_Issue_Update_Flag)
	{	
//show Processing	
		ETP_UI_Request(ETP_OCP_UIM_Processing,ETP_OCP_UIS_Processing);
	}
//test
	JCB_Kernel_Support = 0x0F;

//Load Terminal Type
	glv_tag9F35.Length[0] = 0x01;
	glv_tag9F35.Value[0] = JCB_Static_Parameter.TerminalType;

//Load Dynamic Data
	memcpy(JCB_Dynamic_Parameter.JCB_AmountAuth,glv_tag9F02.Value,6);
	memcpy(JCB_Dynamic_Parameter.JCB_AmountAuthOther,glv_tag9F03.Value,6);
	memcpy(JCB_Dynamic_Parameter.TxnDate,glv_tag9A.Value,3);
	memcpy(JCB_Dynamic_Parameter.Txntime,glv_tag9F21.Value,3);
	JCB_Dynamic_Parameter.TxnType = glv_tag9C.Value[0];
	memcpy(JCB_Dynamic_Parameter.JCB_UnpredictNum,glv_tag9F37.Value,4);

//20140530	Compatible with VISA
	if(JCB_Dynamic_Parameter.TxnType != 0x09)
	{
		memset(JCB_Dynamic_Parameter.JCB_AmountAuthOther,0x00,6);
		memset(glv_tag9F03.Value,0x00,6);
	}

	//5.2
Recover_Break1:

	rspCode = JCB_Ker_Txn_Initialisation();

	switch(rspCode)	
	{
		case JCB_SUCCESS:			
			//5.3
			rspCode = JCB_Ker_Initiate_App_Process();
			if(rspCode == JCB_SUCCESS)
			{
				//5.4
Recover_Break2:				
				rspCode = JCB_Ker_Read_Application_Data();
				if(rspCode == JCB_SUCCESS)
				{
					//5.5
Recover_Break3:					
					rspCode = JCB_Ker_Terminal_Risk_Management();
					if(rspCode == JCB_SUCCESS)
					{
						//5.6
						JCB_Ker_Processing_Restrictions();

						//5.7
						rspCode = JCB_Ker_Terminal_Action_Analysis();
						if(rspCode == JCB_SUCCESS)
						{
							//assign transaction mode
							glv_tag9F39.Length[0] = 0x01;
							glv_tag9F39.Value[0] = JCB_TxnMode;							
						
							if(JCB_TxnMode == JCB_Txn_EMV)	//5.8
							{	
								//5.8.1
								rspCode = JCB_Ker_Generate_AC(JCB_TxnMode,JCB_TAA_Result);
								if(rspCode == JCB_FAIL){
									JCB_Txn_Outcome_End_Application();
								}	
								else if(rspCode == JCB_SelectNext){			
									JCB_Txn_Outcome_Select_Next();
								}
								else if(rspCode == JCB_TryAnother)
									JCB_Txn_Outcome_Try_Another();
								else if(rspCode == JCB_EndAppWithRestart)
									JCB_Txn_Outcome_End_Application_With_Restart_OnDeviceCVM();
								else if(rspCode == JCB_Decline)
								{
									JCB_Txn_Outcome_Decline();
									JCB_Pack_Online_Data(JCB_TxnMode, JCB_TxnDecline);
								}
								else if(rspCode == JCB_EndAppWithRestartComm)
									JCB_Txn_Outcome_End_Application_With_Restart_Comm_Err();
								else if(rspCode == JCB_EndApplication){	//20140606, JCB change the SPEC, when madatory data missing, go to End Application, not Decline
									JCB_Txn_Outcome_End_Application();
								}
								else
								{
									//Show Card Read OK
									ETP_UI_Request(ETP_OCP_UIM_CardReadOKRemoveCard,00);

									//5.8.2 Offline Data Authentication
									//JCB CL 1.3 - Apply CDA when Card Support CDA
									if ((glv_tag9F4B.Length[0]) && (glv_tag82.Value[0] & 0x01))
									{
										//5.8.2.1
										rspCode = JCB_CDA_Verify();
										if(rspCode != JCB_SUCCESS)
										{	
											glv_tag95.Value[0] |= 0x04;	//modify by test case "CDA_VERIFICATION_014 "
											JCB_Txn_Outcome_Decline(); //Decline the transaction 5.13.5
											JCB_Pack_Online_Data(JCB_TxnMode, JCB_TxnDecline);

											break;
										}
									}
										
									//5.8.3	CVM Processing
									if(glv_tag9F50.Length[0])
									{
										rspCode = JCB_CVM_Process(JCB_TxnMode);
										if(rspCode == JCB_FAIL)
										{
											JCB_Txn_Outcome_Decline(); //Decline the transaction 5.13.5
											JCB_Pack_Online_Data(JCB_TxnMode, JCB_TxnDecline);

											break;
										}
									}
									
									//5.8.4	Transaction Outcome
									//5.8.4.1
									if((glv_tag9F27.Value[0] & 0xC0) == 0x40)	//TC
									{
										JCB_Txn_Outcome_Approved();	//Approved Outcome 5.13.1

										JCB_Pack_Online_Data(JCB_TxnMode, JCB_TxnApprovalOrOnline);
									}

									//5.8.4.2~4
									if((glv_tag9F27.Value[0] & 0xC0) == 0x80)	//ARQC
									{
										UT_Get_TLVLengthOfV(glv_tag8D.Length,&tmp_len);
										if(tmp_len)
										{
											JCB_Online_Context.CDOL2_Length = tmp_len;
											memcpy(JCB_Online_Context.CDOL2_Value,glv_tag8D.Value,tmp_len);
										}
										else
										{	
											JCB_Online_Context.CDOL2_Length = 0;
											memset(JCB_Online_Context.CDOL2_Value,0x00,252)	;
										}

										JCB_Online_Context.CVM_Result = JCB_CVM_Result;
									
										JCB_Pack_Online_Data(JCB_TxnMode, JCB_TxnApprovalOrOnline);
										
										//5.8.4.2
										if(	((JCB_Static_Parameter.TerminalInterchangeProfile[1] & 0x80)==0x00 ) ||	//we can check TIP B2b8
											(glv_tag9F60.Length[0] == 0x00)||	//Issuer Update Parameter is absent
											(glv_tag9F60.Value[0] == 0x00))	
										{
											JCB_Txn_Outcome_Online_Req();//Online Request 5.13.2
										}

										//5.8.4.3
										if(	(JCB_Static_Parameter.TerminalInterchangeProfile[1] & 0x80)	&&
											(glv_tag9F60.Value[0] == 0x01))
										{
											JCB_Txn_Outcome_Online_Req_Present_and_Hold();//Online Request 5.13.4 "Present and Hold"
										}

										//5.8.4.4
										if((JCB_Static_Parameter.TerminalInterchangeProfile[1] & 0x80)	&&
											(glv_tag9F60.Value[0] == 0x02))
										{
											JCB_Txn_Outcome_Online_Req_Two_Presentments();//Online Request 5.13.3 "Two Presentments"
										}
									}
								}
							}
							else if(JCB_TxnMode == JCB_Txn_Mag)	//5.9
							{
								//5.9.1
								rspCode = JCB_Ker_Mag_Command();
								if(rspCode == JCB_SUCCESS)
								{
									//Show Card Read Ok
									ETP_UI_Request(ETP_OCP_UIM_CardReadOKRemoveCard,00);
									//5.9.2 CVM Processing
									rspCode = JCB_CVM_Process(JCB_TxnMode);
									if(rspCode == JCB_SUCCESS)
									{
										//5.9.3
										//Transaction Outcome
										JCB_Pack_Online_Data(JCB_TxnMode, JCB_TxnApprovalOrOnline);
										JCB_Txn_Outcome_Online_Req();
									}
									else
									{
										JCB_Txn_Outcome_Decline();	
										JCB_Pack_Online_Data(JCB_TxnMode, JCB_TxnDecline);
									}
								}
								else if(rspCode == JCB_SelectNext){
									JCB_Txn_Outcome_Select_Next();
								}
								else if(rspCode == JCB_EndAppWithRestart)
									JCB_Txn_Outcome_End_Application_With_Restart_OnDeviceCVM();
								else if(rspCode == JCB_Decline)
								{
						
									JCB_Txn_Outcome_Decline();
									JCB_Pack_Online_Data(JCB_TxnMode, JCB_TxnDecline);
								}
								else{
									JCB_Txn_Outcome_End_Application();
								}								
							}
							else if(JCB_TxnMode == JCB_Txn_Legacy)	//5.10
							{
								//5.10.1.1~5
								rspCode = JCB_Ker_Generate_AC(JCB_TxnMode,JCB_TAA_Result);
								if(rspCode == JCB_FAIL)
								{
									JCB_Txn_Outcome_End_Application();//End application the transaction 5.13.7
								}
								else if(rspCode == JCB_SelectNext){
									JCB_Txn_Outcome_Select_Next();
								}
								else if(rspCode == JCB_Decline)
								{
									JCB_Txn_Outcome_Decline();
									JCB_Pack_Online_Data(JCB_TxnMode, JCB_TxnDecline);
								}
								else
								{
									//Show Card Read Ok
									ETP_UI_Request(ETP_OCP_UIM_CardReadOKRemoveCard,00);
									//5.10.1.6
									if((glv_tag9F27.Value[0] & 0xC0) != 0x80)	//ARQC
									{
										JCB_Txn_Outcome_Decline();//Decline the transaction 5.13.5
										JCB_Pack_Online_Data(JCB_TxnMode, JCB_TxnDecline);
									}
									else
									{
										//5.10.2 CVM Processing
										rspCode = JCB_CVM_Process(JCB_TxnMode);
										if(rspCode == JCB_SUCCESS)
										{
											//Transaction Outcome
											JCB_Txn_Outcome_Online_Req();//5.10.3 
											JCB_Pack_Online_Data(JCB_TxnMode, JCB_TxnApprovalOrOnline);
										}
										else
										{
											
											JCB_Txn_Outcome_Decline();//5.10.2.4 Decline the transaction, 5.13.5
											JCB_Pack_Online_Data(JCB_TxnMode, JCB_TxnDecline);
										}
									}
								}								
							}
							else
							{
								JCB_Txn_Outcome_End_Application();	
							}
						}
						else		//5.13 end application
						{
							JCB_Txn_Outcome_Decline();	
							JCB_Pack_Online_Data(JCB_TxnMode, JCB_TxnDecline);
						}
					}
					else //if(rspCode == JCB_SelectNext)
					{
						JCB_Txn_Outcome_Select_Next();
				}
				}
				else if(rspCode == JCB_SelectNext){
					JCB_Txn_Outcome_Select_Next();
				}
				else if(rspCode == JCB_EndAppWithRestartComm)
					JCB_Txn_Outcome_End_Application_With_Restart_Comm_Err();
				else{
					JCB_Txn_Outcome_End_Application();	
				}				
			}
			else if(rspCode == JCB_SelectNext){
				JCB_Txn_Outcome_Select_Next();
			}
			else if(rspCode == JCB_EndAppWithRestartComm)
				JCB_Txn_Outcome_End_Application_With_Restart_Comm_Err();
			else{
				JCB_Txn_Outcome_End_Application();
			}
			
			break;

		case JCB_IssuerUpdate:
			rspCode = JCB_Issuer_Update_Process();//5.11
			if(rspCode == JCB_EndApplicationWithARC)
			{
				//20140627 add
				if( ((JCB_Dynamic_Parameter.AuthRspCode[0] == 0x30) && (JCB_Dynamic_Parameter.AuthRspCode[1] == 0x30)) ||
					((JCB_Dynamic_Parameter.AuthRspCode[0] == 0x31) && (JCB_Dynamic_Parameter.AuthRspCode[1] == 0x30)) ||
					((JCB_Dynamic_Parameter.AuthRspCode[0] == 0x31) && (JCB_Dynamic_Parameter.AuthRspCode[1] == 0x31)))
				{
					api_pcd_JCB_Approval();
				}
				else
				{
					api_pcd_JCB_Decline();
				}
				//20140627 add
			}

			//20150529 when finish the issuer update process, reset the paramenter
			memset((UCHAR *)&JCB_Dynamic_Parameter,0x00,sizeof(JCB_Dynamic_Parameter));	//test
			break;
			
		case JCB_TornTransactionRecovery:
			rspCode = JCB_Txn_Recover_Process();//5.14
			if(rspCode == JCB_Recover_Break_1)
			{
				JCB_Remove_Tags(JCB_GPO_Tag);
				JCB_Remove_Tags(JCB_RR_Tag);
				JCB_Remove_Tags(JCB_GenAC_Tag);
				
				goto Recover_Break1;
			}
			else if(rspCode == JCB_EndAppWithRestartComm)
			{
				//20140610 add
				JCB_Remove_Tags(JCB_GPO_Tag);
				JCB_Remove_Tags(JCB_RR_Tag);
				JCB_Remove_Tags(JCB_GenAC_Tag);
			
				JCB_Txn_Outcome_End_Application_With_Restart_Comm_Err();
			}
			/*else if(rspCode == JCB_FAIL)
			{
				JCB_Txn_Outcome_End_Application();
			}*/
			else if(rspCode == JCB_Decline)
			{
				JCB_Txn_Outcome_Decline();
				JCB_Pack_Online_Data(JCB_Txn_EMV, JCB_TxnDecline);
			}
			else if(rspCode == JCB_SelectNext)
			{
				//20140610 add
				JCB_Remove_Tags(JCB_GPO_Tag);
				JCB_Remove_Tags(JCB_RR_Tag);
				JCB_Remove_Tags(JCB_GenAC_Tag);
				JCB_Txn_Outcome_Select_Next();
			}
			else if(rspCode == JCB_Recover_Break_2)
			{
				//20140610 add
				JCB_Remove_Tags(JCB_RR_Tag);
				JCB_Remove_Tags(JCB_GenAC_Tag);
				
				goto Recover_Break2;
			}
			else if(rspCode == JCB_Recover_Break_3)
			{
				//20140610 add
				JCB_Remove_Tags(JCB_GenAC_Tag);
			
				goto Recover_Break3;
			}
			else if(rspCode == JCB_EndApplication)
			{
				JCB_Txn_Outcome_End_Application();
				break;
			}
			else	//success
			{
				break;
			}
			
			break;
		
		case JCB_SelectNext:
			JCB_Txn_Outcome_Select_Next();
			break;

		case JCB_EndApplication:
			JCB_Txn_Outcome_End_Application();
			break;

		case JCB_EndApplicationWithARC:
			JCB_Txn_Outcome_End_Application_WithARC();
			break;

		default: //End application
			JCB_Txn_Outcome_End_Application();
			break;
	}
	JCB_Free_Buffer();
}

UCHAR JCB_Check_ResetCommand(void)
{
	UCHAR rspCode = 0xFF;
	UINT iLen = 0;
	UCHAR cmd = 0;
	UCHAR RcvBuf[1024] = {0};

	rspCode = UT_CheckAUX();
	if(rspCode == TRUE)
	{
		UT_ReceiveAUX( RcvBuf );
		iLen = RcvBuf[0] + RcvBuf[1]*256 - 1;
		cmd = RcvBuf[2];
	
		if((iLen == 0) && (cmd == 0xA3))	//CMD_Reset
		{
			memset((UCHAR *)&JCB_ReContext_Par,0x00,sizeof(JCB_ReContext_Par));	//test
			memset((UCHAR *)&JCB_Dynamic_Parameter,0x00,sizeof(JCB_Dynamic_Parameter));	//test
			memset((UCHAR *)&etp_Outcome,0x00,sizeof(etp_Outcome));//test 0530
			JCB_Out_Status = JCB_TxnR_EndApp;//test 0530
			ECL_LV1_RESET();	//0530 add
			return TRUE;
		}
	}

	return FALSE;
}

UCHAR JCB_Check_Cancel(void)
{
	UCHAR rspCode = 0xFF;
	UINT iLen = 0;
	UCHAR cmd = 0;
	UCHAR RcvBuf[1024] = {0};

	rspCode = UT_CheckAUX();
	if(rspCode == TRUE)
	{
		UT_ReceiveAUX( RcvBuf );
		iLen = RcvBuf[0] + RcvBuf[1]*256 - 1;
		cmd = RcvBuf[2];
	
		if((iLen == 0) && (cmd == 0xA3))	//CMD_Reset
		{
			return TRUE;
		}
	}

	return FALSE;
}

UCHAR	JCB_GET_L3_RCCode(void)
{
	switch(JCB_Out_Status)
	{
		case JCB_TxnR_Approval:
		case JCB_TxnR_OnlineReq:
		case JCB_TxnR_OnlineReq_Hold:
		case JCB_TxnR_OnlineReq_Two_Present:
			return VAP_RIF_RC_DATA;
			
		default:	//20140820 Return General Fail for L3 Interface
			return VAP_RIF_RC_FAILURE;
	}
}

UCHAR	JCB_GET_SchemeID(void)
{
	switch(JCB_TxnMode)
	{
		case 0x01:
			return VAP_Scheme_JCBWave3;

		case 0x02:
			return VAP_Scheme_JCBMag;
			
		case 0x04:
			return VAP_Scheme_JCBWave2;

		default:
			return FAIL;
	}
}

void	JCB_Update_OnlineData(void)
{
	UCHAR OnlineTag[4] = {0xDF,0x0F,0x01};
	UCHAR OfflineTag[4] = {0xDF,0x1F,0x01};

	UINT UpdateLength=0;
	UCHAR UpdateData[1024]={0x00};

	switch(JCB_Out_Status)
	{
		case JCB_TxnR_Approval:
			OfflineTag[3] = 0x00;

			UpdateLength = Online_Data_Length+4;			
			memcpy(UpdateData,OfflineTag,4);
			memcpy(&UpdateData[4],Online_Data,Online_Data_Length);			
		break;
		
		case JCB_TxnR_OnlineReq:
		case JCB_TxnR_OnlineReq_Hold:
		case JCB_TxnR_OnlineReq_Two_Present:

			if(JCB_TxnMode == JCB_Txn_EMV)
				OnlineTag[3] = 0x04;
			else if(JCB_TxnMode == JCB_Txn_Mag)
				OnlineTag[3] = 0x05;
			else //JCB_TxnMode == JCB_Txn_Legacy
				OnlineTag[3] = 0x06;

			UpdateLength = Online_Data_Length+4;			
			memcpy(UpdateData,OnlineTag,4);
			memcpy(&UpdateData[4],Online_Data,Online_Data_Length);	

		break;

		//20140820 for offline decline transaction, do not set L3_Response_Code = VAP_RIF_RC_DATA
		/*
		case JCB_TxnR_Decline:
			OfflineTag[3] = 0x01;

			UpdateLength = Online_Data_Length+4;			
			memcpy(UpdateData,OfflineTag,4);
			memcpy(&UpdateData[4],Online_Data,Online_Data_Length);		
		break;
		*/
		
		default:
			UpdateLength=Online_Data_Length;
			memcpy(UpdateData,Online_Data,Online_Data_Length);
		break;
	}

	Online_Data_Length = UpdateLength;
	memcpy(Online_Data,UpdateData,Online_Data_Length);
}


