#include <string.h>
#include <stdlib.h>
#include "Glv_ReaderConfPara.h"
#include "Datastructure.h"
#include "Function.h"
#include "Define.h"
#include "ECL_Tag.h"
#include "VGL_Function.h"
#include "VGL_Kernel_Define.h"
#include "VAP_ReaderInterface_Define.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Function.h"

#ifndef _PLATFORM_AS210
#include "LCDTFTAPI.h"
#else
#include "xioapi.h"
#endif

// Add by Wayne 2020/08/21 to avoid compiler warning
// <------
#include "UTILS_CTLS.H"
#include "DBG_Function.h"
// <-------


extern ULONG OS_GET_SysTimerFreeCnt( void );


//ETP_entrypoint.c
extern UCHAR 	etp_flgIsuUpdateErr;
extern UCHAR	etp_rcvData[ETP_BUFFER_SIZE];	//Receive Data Buffer
extern UINT		etp_rcvLen;
extern OUTCOME 	etp_Outcome;
extern UCHAR 	etp_flgRestart;
extern UCHAR 	etp_flgComError;
extern UCHAR	etp_flgCmdTimeout;
extern UCHAR	etp_flgCDCVM;

//api_pcd_Vap_Command.c 
extern UCHAR D1_Data[150];		//Used for Ready For Sale D1 Part, D2 Part
extern UCHAR Tag57_Data[39];
extern UINT  D1_Data_Length;
extern UCHAR Tag57_Data_Length;
extern UCHAR L3_Response_Code;	//Used for Ready For Sale D3 Part
extern UCHAR SchemeID;
extern UCHAR api_tag9F39[11];	//Used for Ready For Sale D4 Part
extern UCHAR api_tagDF0E[11];
extern UCHAR Available_Offline_Amount[7];

//20140110 V2, Double Dip
ULONG 	VGL_Double_DIP_Timer=0;
UCHAR 	VGL_LastTxnPAN[21] = {0};
ULONG 	VGL_LastTxnPANTime = 0;

//Use this flag for deciding to process CVN17 transaction or not
UCHAR 	VGL_Kernel_CVN17=FALSE;

//CVM Result
UCHAR 	VGL_CVMOutcome = FALSE;

//Transaction Result :Offline_Approval
UCHAR	VGL_Offline_Approval=FALSE;

//VISA VCPS Global PRE DRL Reader Limit Set
PREDRLLIMITSET	VGL_PREDRLLimitSet[ETP_PARA_NUMBER_PID]={{{0},0}};

//Mandatory tag check
UCHAR 	VGL_tag57	=0x00;	//Track2 Equivalant Data			//GPO or Read Record
UCHAR	VGL_tag82	=0x00;	//Application Interchange Profile		//GPO
UCHAR 	VGL_tag9F10	=0x00;	//Issuer Application Data			//GPO
UCHAR	VGL_tag9F26	=0x00;	//Application Cryptogram			//GPO
UCHAR	VGL_tag9F36	=0x00;	//Application Transaction Counter	//GPO

//Receive Buffer
//UCHAR	VGL_rcvBuff[DEP_BUFFER_SIZE_RECEIVE]={0};	//Receive Data Buffer
UCHAR	*VGL_rcvBuff;
UINT	VGL_rcvLen=0;				//Receive Length
//Send Buffer
//UCHAR	VGL_sendBuff[DEP_BUFFER_SIZE_SEND]={0};	//Send Data Buffer
UCHAR	*VGL_sendBuff;
UINT	VGL_sendLen=0;				//Send Length


//Cryptogram Type
UCHAR 	VGL_AAC = 0;
UCHAR 	VGL_TC = 0;
UCHAR 	VGL_ARQC = 0;

//Path Flag
UCHAR 	VGL_MSD_Path = FALSE;
UCHAR 	VGL_qVSDC_Path = FALSE;

//qVSDC Mode - Indicator
UCHAR	VGL_Decline_Required_by_Reader = 0;
UCHAR	VGL_Online_Required_by_Reader  = 0;

//Track1,Track2 Data
UCHAR 	VGL_MSD_Track2_Data[150]={0},VGL_MSD_Track1_Data[150]={0};		//ICTK 0513, Change the VGL_MSD_Track2_Data and VGL_MSD_Track1_Data buff size 79 ==> 150
UCHAR 	VGL_MSD_Track2_Data_Length=0,VGL_MSD_Track1_Data_Length=0;

//Print AOSA, PAN, Format is L (1 byte)+ V
UCHAR	VGL_Print_AOSA_Amount[14] = {0};
UCHAR	VGL_Print_PAN[21]={0};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

UCHAR VGL_Allocate_Buffer(void)
{
	VGL_sendBuff=malloc(DEP_BUFFER_SIZE_SEND);
	VGL_rcvBuff=malloc(DEP_BUFFER_SIZE_RECEIVE);

	if ((VGL_sendBuff == NULLPTR) || (VGL_rcvBuff == NULLPTR))
	{
		return FAIL;
	}

	return SUCCESS;
}

void VGL_Free_Buffer(void)
{
	free(VGL_sendBuff);
	free(VGL_rcvBuff);
}

void VGL_GPO_Terminate_OutCome(void)
{	
	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= FALSE;
	etp_Outcome.dscData 		= FALSE;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt 		= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

void VGL_End_Application(void)
{
// we display message, LED andd beep sound by show status command

#ifndef _PLATFORM_AS210
	//20131023	
	//accroding to AS350 support more interface, the rc_code shall be RC_Other_interface
	//else the RC_Code shall be RC_Failure
	L3_Response_Code = VAP_RIF_RC_OTHER_INTERFACE;
#endif	

	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= FALSE;			//20140110 V3 Removed
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= FALSE;
	etp_Outcome.dscData			= FALSE;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

void VGL_RR_MSD_Online_Req(void)
{
	//20140114 V1 MSD Online transaction, assign the Response code
	L3_Response_Code = VAP_RIF_RC_DATA;

	VGL_RR_MSD_Online_Data();
	
	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;	//20140114 V1, Remove the Display
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_AuthorisingPleaseWait;
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= TRUE;
	etp_Outcome.dscData 		= FALSE;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt 		= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;

}


void VGL_Try_Again_OutCome(void)
{
	etp_flgComError = TRUE;
	etp_flgCDCVM = TRUE;

	etp_Outcome.Start			= ETP_OCP_Start_B;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= TRUE;
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_SeePhoneForInstructions;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
	etp_Outcome.hldTime			= 13;
	etp_Outcome.rqtRestart		= TRUE;
	etp_Outcome.rstStatus		= ETP_OCP_UIS_ReadyToRead;
	etp_Outcome.datRecord		= FALSE;
	etp_Outcome.dscData			= FALSE;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= 10;
	etp_Outcome.rmvTimeout		= 0;	
}

void VGL_Select_Next_OutCome(void)
{
	//test
	ETP_Remove_CandidateList();
	//test end
	etp_Outcome.Start			= ETP_OCP_Start_C;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM 			= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= FALSE;
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= FALSE;
	etp_Outcome.dscData			= FALSE;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
}

void VGL_FDDA_Try_Another_Interface(void)
{
#ifndef _PLATFORM_AS210
	//20131023	
	//accroding to AS350 support more interface, the rc_code shall be RC_Other_interface
	//else the RC_Code shall be RC_Failure
	L3_Response_Code = VAP_RIF_RC_OTHER_INTERFACE;
#endif	

	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM				= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= FALSE;			////20140110 V3 Removed
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PleaseInsertCard;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= FALSE;
	etp_Outcome.dscData			= FALSE;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_ContactChip;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
	
}

void VGL_GPO_Try_Another_Interface(void)
{	

#ifndef _PLATFORM_AS210
	//20131023	
	//accroding to AS350 support more interface, the rc_code shall be RC_Other_interface
	//else the RC_Code shall be RC_Failure
	L3_Response_Code = VAP_RIF_RC_OTHER_INTERFACE;
#endif	

	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM				= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= FALSE;
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PleaseInsertCard;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= FALSE;
	etp_Outcome.dscData			= FALSE;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_ContactChip;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
	
}

void VGL_Try_Another_Interface(void)
{

#ifndef _PLATFORM_AS210
	//20131023	
	//accroding to AS350 support more interface, the rc_code shall be RC_Other_interface
	//else the RC_Code shall be RC_Failure
	L3_Response_Code = VAP_RIF_RC_OTHER_INTERFACE;
#endif	


	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));
	etp_Outcome.CVM				= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= FALSE;			////20140110 V3 Removed
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PleaseInsertOrSwipeCard;
	etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
	etp_Outcome.rqtRestart		= FALSE;
	etp_Outcome.datRecord		= FALSE;
	etp_Outcome.dscData			= FALSE;
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;	
	
}

UCHAR VGL_Chip_Data(UCHAR Tag_Total_Len,UCHAR *taglist)
{
	UINT i = 0,tmpLEN = 0,VLength=0;
	UCHAR TLength = 0, LLength = 0, rspCode = 0,*Len_Addr = 0, *Value_Addr = 0;
	ULONG Index=0;
	UCHAR TEC[2]={0xDF,0x0E};
	UCHAR Track1Tag[2]={0xDF,0x10};
	UCHAR Track2Tag[2]={0xDF,0x11};

	//20140107 handle Pos Entry Mode and Terminal Entry Capability
	api_tag9F39[0] = glv_tag9F39.Length[0];
	memcpy(&api_tag9F39[1],glv_tag9F39.Value,api_tag9F39[0]);

	//VCPS 2.1.3 Update - Terminal Entry Capability should not Set by TTQ
	api_tagDF0E[0] = 1;
/*	if(glv_tag9F66.Value[0]&0x10)		//ICTK 0627 support qVSDC & contact	
		api_tagDF0E[1]=5;	
	else
		api_tagDF0E[1]=8;	
*/	//20140107 handle Pos Entry Mode and Terminal Entry Capability end
#ifdef _PLATFORM_AS350
	api_tagDF0E[1]=0x05;
#else
	api_tagDF0E[1]=0x08;
#endif

	////////////Mandatory//////////////
	UT_Get_TLVLengthOfV(glv_tag9F03.Length,&tmpLEN);	
	if(!tmpLEN)
	{
		glv_tag9F03.Length[0] = 0x06;
	}

	UT_Get_TLVLengthOfV(glv_tag95.Length,&tmpLEN);		
	if(!tmpLEN)
	{
		glv_tag95.Length[0] = 0x05;
	}
	///////////Mandatory///////////////

	
	if(VGL_MSD_Path)
	{
		//MSD always go Online
		Online_Data[i++]=0xDF;
		Online_Data[i++]=0x0F;
		Online_Data[i++]=0x01;
		Online_Data[i++]=0x02;			//VISA MSD online Data
	}
	if(VGL_qVSDC_Path)
	{
		if(VGL_Decline_Required_by_Reader)
		{
			Online_Data[i++]=0xDF;
			Online_Data[i++]=0x1F;
			Online_Data[i++]=0x01;
			Online_Data[i++]=0x01;		//decline
		}
		else
		{
			if(VGL_Online_Required_by_Reader)
			{
				Online_Data[i++]=0xDF;
				Online_Data[i++]=0x0F;
				Online_Data[i++]=0x01;
				Online_Data[i++]=0x01;			//VISA qVSDC Online Data
			}
			else
			{
				Online_Data[i++]=0xDF;
				Online_Data[i++]=0x1F;
				Online_Data[i++]=0x01;
				Online_Data[i++]=0x00;		//approval
			}
		}		
	}
	
	while(Tag_Total_Len)
	{
		TLength = LLength = VLength = Index = 0;
		UT_Get_TLVLengthOfT(taglist,&TLength);
		rspCode = UT_Search(TLength,taglist,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);
		if(rspCode == SUCCESS)
		{
			//UT_DumpHex(0,0,TLength,taglist);
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
			if(memcmp(taglist,TEC,2) == 0)
			{
				TLength = 2;
				memcpy(&Online_Data[i],taglist,TLength);
				i+=TLength;
				Online_Data[i++] = 0x01;

				//VCPS 2.1.3 Update - Terminal Entry Capability should not Set by TTQ
				/*
				if(glv_tag9F66.Value[0]&0x10)		//ICTK 0627 support qVSDC & contact	
					Online_Data[i++]=0x05;	
				else
					Online_Data[i++]=0x08;	
				*/
#ifdef _PLATFORM_AS350
				Online_Data[i++]=0x05;
#else
				Online_Data[i++]=0x08;
#endif
			}
			else if(memcmp(taglist,Track1Tag,2) == 0)
			{
				TLength = 2;
				if(Opt_MSD_Constructing_Track1)
				{
					memcpy(&Online_Data[i],taglist,TLength);
					i+=TLength;

					Online_Data[i++] = VGL_MSD_Track1_Data_Length;
					
					memcpy(&Online_Data[i],VGL_MSD_Track1_Data,VGL_MSD_Track1_Data_Length);
					i+=VGL_MSD_Track1_Data_Length;
				}
			}
			else if(memcmp(taglist,Track2Tag,2) == 0)
			{
				TLength = 2;
				if(Opt_MSD_Formatting_Track2)
				{
					memcpy(&Online_Data[i],taglist,TLength);
					i+=TLength;

					Online_Data[i++] = VGL_MSD_Track2_Data_Length;
					
					memcpy(&Online_Data[i],VGL_MSD_Track2_Data,VGL_MSD_Track2_Data_Length);
					i+=VGL_MSD_Track2_Data_Length;
				}
			}
			else
				return FAIL;
		}
		
		taglist += TLength;
		Tag_Total_Len -= TLength;
	}

	Online_Data_Length = i;
	
	return SUCCESS;

}

void VGL_RR_MSD_Online_Data(void)
{
	//UINT i=0,tag_len=0;
	UCHAR MSD_Online_Data_CVN17[50]={0x9F,0x02,0x9F,0x26,0x5F,0x34,0x9F,0x36,0x9F,0x7C,
									 0x9F,0x6E,0x9F,0x10,0x9F,0x39,0xDF,0x0E,0x9F,0x66,
									 0x9F,0x37,0x9A,0x9F,0x21,0xDF,0x10,0xDF,0x11,0x57,
									 0x5F,0x28,0x9F,0x57};
	
	UCHAR MSD_Online_Data[50]={	0x9F,0x39,0xDF,0x0E,0xDF,0x10,0xDF,0x11,0x57,0x5F,
								0x28,0x9F,0x57};

	UCHAR rspCode = 0;
		
	//if(Opt_MSD_CVN17_Support)
	//20140122 100V case CLM.R.004.0104, CLMG.0070005 , if tag 9F26 is missing, we process CVN142 transaction.
	//we use this flag "VGL_Kernel_CVN17" for deciding to process CVN17 transaction or not.
	//20131204 for case CLMG0070005 and CLM.R.004.0104, if tag 9F26 is missing we perform CVN 142 transaction
	if((Opt_MSD_CVN17_Support)&&(VGL_Kernel_CVN17))
	{
		rspCode = VGL_Chip_Data(34,MSD_Online_Data_CVN17);

		if(rspCode == FAIL)
		{
			UT_PutMsg(1,0,FONT0,21,(UCHAR *)"MSD Online Data Error");
			UT_WaitKey();
		}
	}
	else
	{
		rspCode = VGL_Chip_Data(13,MSD_Online_Data);		//20140115 v2 reduce the length

		if(rspCode == FAIL)
		{
			UT_PutMsg(1,0,FONT0,21,(UCHAR *)"MSD Online Data Error");
			UT_WaitKey();
		}
	}

	memset(VGL_MSD_Track1_Data,0x00,VGL_MSD_Track1_Data_Length);
	VGL_MSD_Track1_Data_Length=0;
	memset(VGL_MSD_Track2_Data,0x00,VGL_MSD_Track2_Data_Length);
	VGL_MSD_Track2_Data_Length=0;
}

UCHAR VGL_LRC( UCHAR *buf, UINT len )
{
	UINT i;
	UCHAR lrc = 0;		 
	 
	for( i = 0; i < len; i++ )
	{
		lrc = lrc + buf[i];
	}
	 
	return ((UCHAR)(-((char)lrc)));
}

UCHAR VGL_PR_Cashback_Exam_CTQ(UCHAR *CTQ)
{
#ifdef _PLATFORM_AS350
	if(CTQ[0] & 0x02)
		return TRUE;	//	VGL_Try_Another_Interface();
	else
		return FALSE;
#else
	UCHAR TTQ[4]={0x00};
	memcpy(TTQ,glv_tag9F66.Value,4);
	
	if((CTQ[0] & 0x02) && (TTQ[0] & 0x10))
		return TRUE;	//	VGL_Try_Another_Interface();
	else
		return FALSE;
#endif
}

UCHAR VGL_PR_Cash_Exam_CTQ(UCHAR *CTQ)
{
#ifdef _PLATFORM_AS350
	if(CTQ[0] & 0x04)
		return TRUE;	//	VGL_Try_Another_Interface();
	else
		return FALSE;
#else
	UCHAR TTQ[4]={0x00};
	memcpy(TTQ,glv_tag9F66.Value,4);
	
	if((CTQ[0] & 0x04) && (TTQ[0] & 0x10))
		return TRUE;	//	VGL_Try_Another_Interface();
	else
		return FALSE;
#endif
}


//FUNCTION : Handle Req 5.5 "Processing Restrictions"  EMV Mode Path Processing
// INPUT   : none.
// OUTPUT  : VGL_Online_Required_by_Reader, VGL_Decline_Required_by_Reader indicator
// RETURN  : none.

UCHAR VGL_Processing_Restriction(void)
{
	UCHAR Tmp_CTQ[2] = {0},Tmp_AUC[2] = {0},Card_Date[3] = {0},Card_Tmp_Date[2] = {0},TTQ[4] = {0};
	UCHAR PAN_list[10] = {0},result=0,rspCode=0;
	UINT Tmp_Len=0,Tmp_Len2=0,i=0;
	UCHAR 	Cash_Transactions_Allow = 0 ;
	UCHAR 	Cashback_Transactions_Allow = 0;

	//Handle Req 5.5.1.1 "Application Expired Check"
	if(VGL_TC)
	{
		UT_Get_TLVLengthOfV(glv_tag5F24.Length,&Tmp_Len);
		if(Tmp_Len != 0)	
		{
			memcpy(Card_Date,glv_tag5F24.Value,Tmp_Len);	//Get Application Expired Date
			//Transfer the YYMMDD to MMYY
			Card_Tmp_Date[0] = Card_Date[1];
			Card_Tmp_Date[1] = Card_Date[0];
			
			if(UT_VerifyCertificateExpDate(Card_Tmp_Date) == FALSE)
			{	
				UT_Get_TLVLengthOfV(glv_tag9F6C.Length,&Tmp_Len);
				if(Tmp_Len != 0)	
				{
					memcpy(Tmp_CTQ,glv_tag9F6C.Value,Tmp_Len);
					
					//20131205
					memcpy(TTQ,glv_tag9F66.Value,glv_tag9F66.Length[0]);
					if((Tmp_CTQ[0] & 0x08) && (!(TTQ[0] & 0x08)))
						VGL_Online_Required_by_Reader = 1;
					else
						VGL_Decline_Required_by_Reader = 1;
				}
				else
					VGL_Decline_Required_by_Reader = 1;
			}
		}
		else		//if it is a Offline_Transactions and "Application Expiration Date" is not returned we assume FDDA fail and check the CTQ
		{
			UT_Get_TLVLengthOfV(glv_tag9F6C.Length,&Tmp_Len);
			if(Tmp_Len != 0)	
			{
				memcpy(Tmp_CTQ,glv_tag9F6C.Value,Tmp_Len);

				//20131205
				memcpy(TTQ,glv_tag9F66.Value,glv_tag9F66.Length[0]);
				if((Tmp_CTQ[0] & 0x08) && (!(TTQ[0] & 0x08)))
					VGL_Online_Required_by_Reader = 1;
				else
					VGL_Decline_Required_by_Reader = 1;
			}
			else
				VGL_Decline_Required_by_Reader = 1;
		}

	}

//	UT_DispHexByte(2,0,VGL_Online_Required_by_Reader);
//	UT_DispHexByte(3,0,VGL_Decline_Required_by_Reader);
//	UT_WaitKey();

	
	//Handle Req 5.5.1.2 "Terminal Exception File Check"
	if(Opt_qVSDC_Ter_Except_File)
	{
		//we should make a Exception_File_List First, and contains with APNs
		/*
		Acquirer-Merchant-Optional:
		If the Terminal Exception File Check is implemented,
		then it shall be acquirer-merchant configurable to be enabled or
		disabled.
		If the card application returns a Transaction Certificate (TC),
		and the Application PAN is present on the Terminal Exception
		File,
		then the kernel shall set the Decline Required by Reader Indicator
		to 1.
		*/
		memset(PAN_list,0xFF,10);
		UT_Get_TLVLengthOfV(glv_tag5A.Length,&Tmp_Len);

		if(Tmp_Len != 0)
		{
			memcpy(PAN_list,glv_tag5A.Value,Tmp_Len);
			for( i=0; i<10; i++ )
			{	
				if( UT_memcmp( &Exception_File[i][0], PAN_list, Tmp_Len ) == 0 )
			    {
			    	result = TRUE;	// matched
			    	break;
				}
			}
		}
		if(result == TRUE)
			VGL_Decline_Required_by_Reader = 1;
	}

	//Handle Req 5.5.1.3 "Application Usage Control - Cash Transactions"
	if((glv_tag9C.Value[0] == 0x01)&& (Opt_Cash_Transactions == 1))
	{	
		
		//Application Usage Control (AUC)
		UT_Get_TLVLengthOfV(glv_tag9F07.Length,&Tmp_Len);
		if(Tmp_Len != 0)	
		{
			memcpy(Tmp_AUC,glv_tag9F07.Value,Tmp_Len);

			UT_Get_TLVLengthOfV(glv_tag9F57.Length,&Tmp_Len);	//Issuer Country Code return?
			UT_Get_TLVLengthOfV(glv_tag5F28.Length,&Tmp_Len2);	//Issuer Country Code return?
			if((Tmp_Len != 0) || (Tmp_Len2 != 0))
			{
				if(Tmp_Len != 0)
				{
					if((memcmp(glv_tag9F1A.Value,glv_tag9F57.Value,2)==0) && (Tmp_AUC[0] & 0x80))	//for domestic cash
						Cash_Transactions_Allow = 1;
					else if((memcmp(glv_tag9F1A.Value,glv_tag9F57.Value,2)!=0) && (Tmp_AUC[0] & 0x40))	//for Internation cash
						Cash_Transactions_Allow = 1;
					else
						Cash_Transactions_Allow = 0;
						//rspCode = VGL_PR_Cash_Exam_CTQ(Tmp_CTQ);
				}
				else
				{
					if((memcmp(glv_tag9F1A.Value,glv_tag5F28.Value,2)==0) && (Tmp_AUC[0] & 0x80))	//for domestic cash
						Cash_Transactions_Allow = 1;
					else if((memcmp(glv_tag9F1A.Value,glv_tag5F28.Value,2)!=0) && (Tmp_AUC[0] & 0x40))	//for Internation cash
						Cash_Transactions_Allow = 1;
					else
						Cash_Transactions_Allow = 0;
						//rspCode = VGL_PR_Cash_Exam_CTQ(Tmp_CTQ);
				}
			}
			else
				Cash_Transactions_Allow = 0;
				//rspCode = VGL_PR_Cash_Exam_CTQ(Tmp_CTQ);
		}
		else
			Cash_Transactions_Allow = 0;
			//rspCode = VGL_PR_Cash_Exam_CTQ(Tmp_CTQ);

		//Check CASH Allow or not
		if(Cash_Transactions_Allow == 0)
		{
			//if CTQ present , process continue else Decline = 1
			UT_Get_TLVLengthOfV(glv_tag9F6C.Length,&Tmp_Len);
			if(Tmp_Len != 0)	
			{
				memcpy(Tmp_CTQ,glv_tag9F6C.Value,Tmp_Len);
				
				rspCode = VGL_PR_Cash_Exam_CTQ(Tmp_CTQ);
				
				//20130926 Change the  switch Interface logic				
				if(rspCode == TRUE)
				{
					return VGL_Switch_Interface;
				}
				else
					VGL_Decline_Required_by_Reader = 1;
				
			}
			else
				VGL_Decline_Required_by_Reader = 1;
			
		}
	}

//	UT_DispHexByte(4,0,VGL_Online_Required_by_Reader);
//	UT_DispHexByte(5,0,VGL_Decline_Required_by_Reader);
//	UT_WaitKey();
	
	//Handle Req 5.5.1.4 "Application Usage Control - Cashback Transactions"
	if((glv_tag9C.Value[0] == 0x00) && (Opt_Cashback_Transactions == 1))
	{
		
		//Application Usage Control (AUC)
		UT_Get_TLVLengthOfV(glv_tag9F07.Length,&Tmp_Len);
		if(Tmp_Len != 0)	
		{
			memcpy(Tmp_AUC,glv_tag9F07.Value,Tmp_Len);
			
			UT_Get_TLVLengthOfV(glv_tag9F57.Length,&Tmp_Len);	//Issuer Country Code return?
			UT_Get_TLVLengthOfV(glv_tag5F28.Length,&Tmp_Len2);	//Issuer Country Code return?
			if((Tmp_Len != 0) || (Tmp_Len2 != 0))	
			{
				if(Tmp_Len != 0)
				{
					if((memcmp(glv_tag9F1A.Value,glv_tag9F57.Value,2)==0) && (Tmp_AUC[1] & 0x80))	//for domestic cashback
						Cashback_Transactions_Allow = 1;
					else if((memcmp(glv_tag9F1A.Value,glv_tag9F57.Value,2)!=0) && (Tmp_AUC[1] & 0x40))	//for Internation cashback
						Cashback_Transactions_Allow = 1;
					else
						Cashback_Transactions_Allow = 0;
				}
				else
				{
					if((memcmp(glv_tag9F1A.Value,glv_tag5F28.Value,2)==0) && (Tmp_AUC[1] & 0x80))	//for domestic cash
						Cashback_Transactions_Allow = 1;
					else if((memcmp(glv_tag9F1A.Value,glv_tag5F28.Value,2)!=0) && (Tmp_AUC[1] & 0x40))	//for Internation cash
						Cashback_Transactions_Allow = 1;
					else 
						Cashback_Transactions_Allow = 0;
				}
				
			}
			else
				Cashback_Transactions_Allow = 0;
			
		}
		else
			Cashback_Transactions_Allow = 0;			
		
		//Check CASHBACK Allow or not
		if(Cashback_Transactions_Allow == 0)
		{
			//if CTQ present, else Decline = 1
			UT_Get_TLVLengthOfV(glv_tag9F6C.Length,&Tmp_Len);
			if(Tmp_Len != 0)	
			{
				memcpy(Tmp_CTQ,glv_tag9F6C.Value,Tmp_Len);
				rspCode = VGL_PR_Cashback_Exam_CTQ(Tmp_CTQ);
				
				if(rspCode == TRUE)
				{
					return VGL_Switch_Interface;
				}	
				else
					VGL_Decline_Required_by_Reader = 1; 
			}
			else
				VGL_Decline_Required_by_Reader = 1; 
		}
		
	}	

//	UT_DispHexByte(6,0,VGL_Online_Required_by_Reader);
//	UT_DispHexByte(7,0,VGL_Decline_Required_by_Reader);
//	UT_WaitKey();

	return SUCCESS;
}


//VCPS 2.1.3 Update - Create Track 1 and Track 2 Function
UCHAR VGL_Create_MSDTrack(void)
{
	UINT lenOfV=0;
	UINT lenOf57=0;
	UINT lenOf5F20=0;
	UCHAR lenData=0;
	UCHAR lenPadding=0;
	UCHAR lenMaxT1DD=0;
	UCHAR lenPAN=0;
	UCHAR lenFollowing=0;
	UCHAR numIndex=0;
	UCHAR *ptrData;
	UCHAR flgPadding=FALSE;
	UCHAR flgPVV=FALSE;
	UCHAR flgError=FALSE;
	UCHAR ofsCI=0;
	UCHAR ofsATC1=0;
	UCHAR ofsATC2=0;
	UCHAR ofsdCVV=0;

	//Reset Data
	VGL_MSD_Track1_Data_Length = 0;
	memset(VGL_MSD_Track1_Data,0x00,150);
	VGL_MSD_Track2_Data_Length = 0;
	memset(VGL_MSD_Track2_Data,0x00,150);
	

	//Create Track 2
	//Start Sentinel
	VGL_MSD_Track2_Data[0]=';';

	//Track 2 Equivalent Data
	UT_Get_TLVLengthOfV(glv_tag57.Length, &lenOf57);
	
	if (lenOf57 == 0)
	{
		return FAIL;
	}
		
	UT_Split(&VGL_MSD_Track2_Data[1], glv_tag57.Value, (char)lenOf57);

	//Field Separator
	for (numIndex=0; numIndex < (19+1); numIndex++)	//PAN(19) + Separator(1)
	{
		if (VGL_MSD_Track2_Data[1+numIndex] == 'D')	//First Byte Starts from Track2_Data[1]
		{
			VGL_MSD_Track2_Data[1+numIndex]='=';
			lenPAN=numIndex;
			break;
		}
	}

	if (numIndex == (19+1))	//Separator Not Found
	{
		return FAIL;
	}
	
	//End Sentinel
	lenData=1+lenOf57*2;	//Start Sentinel + Track 2 Equivalent Data
	
	if (VGL_MSD_Track2_Data[lenData-1] == 'F')	//Check Padding
	{
		flgPadding=TRUE;
		lenData--;
	}
		
	VGL_MSD_Track2_Data[lenData++]='?';
	
	//LRC
	VGL_MSD_Track2_Data_Length=lenData;
	VGL_MSD_Track2_Data[lenData]=VGL_LRC(VGL_MSD_Track2_Data, VGL_MSD_Track2_Data_Length);
	
	VGL_MSD_Track2_Data_Length+=1;


	//Create Track 1
	//Field 1 - Start Sentinel
	lenData=0;
	VGL_MSD_Track1_Data[lenData++]='%';

	//Field 2 - Format Code
	VGL_MSD_Track1_Data[lenData++]='B';

	//Field 3 - PAN
	ptrData=&VGL_MSD_Track2_Data[1];
	memcpy(&VGL_MSD_Track1_Data[lenData], ptrData, lenPAN);
	lenData+=lenPAN;

	//Field 4 - Field Separator
	VGL_MSD_Track1_Data[lenData++]='^';

	//Field 5 - Cardholder Name
	UT_Get_TLVLengthOfV(glv_tag5F20.Length, &lenOf5F20);
	if (lenOf5F20 != 0)
	{
		memcpy(&VGL_MSD_Track1_Data[lenData], glv_tag5F20.Value, lenOf5F20);
		lenData+=lenOf5F20;
	}
	else
	{
		//Set Default Value
		lenOf5F20=2;
		VGL_MSD_Track1_Data[lenData++]=0x20;
		VGL_MSD_Track1_Data[lenData++]=0x2F;
	}

	//Field 6 - Field Separator
	VGL_MSD_Track1_Data[lenData++]='^';

	//Field 7 - Card Expiration Date
	ptrData=&VGL_MSD_Track2_Data[1+lenPAN+1];	//Point to First Position after Separator
	if (UT_CheckMonth(ptrData+2))		//+2 to Month
	{
		memcpy(&VGL_MSD_Track1_Data[lenData], ptrData, 4);
		lenData+=4;
	}
	else
	{
		return FAIL;
	}

	//Field 8 - Service Code
	ptrData+=4;	//Point to Service Code
	memcpy(&VGL_MSD_Track1_Data[lenData], ptrData, 3);
	lenData+=3;

	//Field 9 - PVKI & PVV
	lenPadding=(flgPadding == TRUE)?(1):(0);

	//If the number of digits following separator 'D' is 20, the PVV is present	
	lenFollowing=lenOf57*2-(lenPAN+1+lenPadding);	//Following=Total-(PAN+Separator+Padding)
	
	if (lenFollowing == 20)
	{
		flgPVV=TRUE;
	}
	else
	{
		if (lenFollowing != 15)
		{
			flgError=TRUE;
		}
	}

	if (flgPVV == TRUE)
	{
		ptrData+=3;	//Point to PVKI & PVV
		memcpy(&VGL_MSD_Track1_Data[lenData], ptrData, 5);
		lenData+=5;
	}

	//Field 10.1 - Track1 Discretionary Data
	UT_Get_TLVLengthOfV(glv_tag9F1F.Length, &lenOfV);
	if (lenOfV != 0)
	{
		lenMaxT1DD=79-30-lenPAN-lenOf5F20;	//Max=79-30-PAN-Cardholder
		
		if (lenOfV > lenMaxT1DD)
		{
			memcpy(&VGL_MSD_Track1_Data[lenData], glv_tag9F1F.Value, lenMaxT1DD);
			lenData+=lenMaxT1DD;
		}
		else
		{
			memcpy(&VGL_MSD_Track1_Data[lenData], glv_tag9F1F.Value, lenOfV);
			lenData+=lenOfV;
		}
	}

	//Field 10.2 - Contactless Indicator
	//Field 11.1 - ATC Part 1
	//Field 11.2 - dCVV
	//Field 11.3 - ATC Part 2
	if (flgError == FALSE)
	{
		if (flgPVV == TRUE)
		{
			ofsCI=20;
			ofsATC1=16;
			ofsdCVV=13;
			ofsATC2=18;
		}
		else
		{
			ofsCI=15;
			ofsATC1=11;
			ofsdCVV=8;
			ofsATC2=13;
		}		
		
		VGL_MSD_Track1_Data[lenData++]=VGL_MSD_Track2_Data[1+numIndex+ofsCI];
		
		memcpy(&VGL_MSD_Track1_Data[lenData], &VGL_MSD_Track2_Data[1+numIndex+ofsATC1], 2);
		lenData+=2;
		
		memcpy(&VGL_MSD_Track1_Data[lenData], &VGL_MSD_Track2_Data[1+numIndex+ofsdCVV], 3);
		lenData+=3;
		
		memcpy(&VGL_MSD_Track1_Data[lenData], &VGL_MSD_Track2_Data[1+numIndex+ofsATC2], 2);
		lenData+=2;
	}
	else
	{
		//Set Default Value
		VGL_MSD_Track1_Data[lenData++]='1';	//Contactless Indicator
		VGL_MSD_Track1_Data[lenData++]='0';	//ATC Part 1
		VGL_MSD_Track1_Data[lenData++]='0';
		VGL_MSD_Track1_Data[lenData++]='0';	//dCVV
		VGL_MSD_Track1_Data[lenData++]='0';
		VGL_MSD_Track1_Data[lenData++]='0';
		VGL_MSD_Track1_Data[lenData++]='0';	//ATC Part 2
		VGL_MSD_Track1_Data[lenData++]='0';
	}

	//Field 11.4 - VISA Reserved
	//Field 11.5 - VISA Reserved
	VGL_MSD_Track1_Data[lenData++]='0';
	VGL_MSD_Track1_Data[lenData++]='0';
	VGL_MSD_Track1_Data[lenData++]='0';
	VGL_MSD_Track1_Data[lenData++]='0';
	
	//Field 12 - End Sentinel
	VGL_MSD_Track1_Data[lenData++]='?';

	//Field 13 - LRC
	VGL_MSD_Track1_Data_Length=lenData;
	VGL_MSD_Track1_Data[lenData]=VGL_LRC(VGL_MSD_Track1_Data, VGL_MSD_Track1_Data_Length);
	
	VGL_MSD_Track1_Data_Length+=1;
	
	return SUCCESS;
}

UCHAR VGL_AS210_D1_Track(void)
{
	
//	UCHAR *tmp_tag57=0;
	UCHAR tmp_tag57[19];
	UINT IndexOfD=0,tmp_len=0,i=0,j=0;
	
	//Prepare for Copy the Track2 data (PAN, CardHolderName, ExpiryDate, ServiceCode, PVV, DCVV, ATC, CL) to the Array for future use
	UCHAR	PAN[19]={0}, ExpiryDate[4]={0}, ServiceCode[3]={0}, PVV[5]={0},
			DCVV[3]={0x30,0x30,0x30}, ATC12[2]={0x30,0x30}, ATC34[2]={0x30,0x30},
			CL[1]={0x31}, CHName[26]={0},T1DD[49]={0};

	UCHAR D2_Data[150]={0};
	UCHAR D2_Data_Length=0;
	//Card Expiry Date
	UCHAR Card_ExpiryDate[4] = {0};
	
	//For Fixed Size
	UINT ExpiryDate_Len = 4, ServiceCode_Len = 3, DCVV_Len = 3, ATC12_Len = 2, ATC34_Len = 2 , CL_Len = 1;
	//For VAR Size
	UINT	PAN_Len = 0, PVV_Len = 0,CH_Len = 0, T1DD_Len = 0, T1DD_Tmp_Len =0 ,afterD = 0;


	//Reset Whole Data
	D1_Data_Length = 0;
	memset(D1_Data,0x00,150);
	Tag57_Data_Length = 0;
	memset(Tag57_Data,0x00,sizeof(Tag57_Data));
	
	//Ready to Construct Track1, Track2
	if (SchemeID == 0x20)	//PayPass Mag-Stripe
	{
		UT_Get_TLVLengthOfV(glv_tag9F6B.Length,&tmp_len);
	
		Tag57_Data_Length = (UCHAR)tmp_len;
		memcpy(Tag57_Data,glv_tag9F6B.Value,Tag57_Data_Length);
	
		memcpy(tmp_tag57,glv_tag9F6B.Value,tmp_len);
	}
	else
	{
		UT_Get_TLVLengthOfV(glv_tag57.Length,&tmp_len);

		Tag57_Data_Length = (UCHAR)tmp_len;
		memcpy(Tag57_Data,glv_tag57.Value,Tag57_Data_Length);
			
		memcpy(tmp_tag57,glv_tag57.Value,tmp_len);
	}

	//len of tag57 = 0 problem
	if(tmp_len == 0)
		return FAIL;

	D2_Data[0] = ';';
		
	UT_Split(&D2_Data[1],tmp_tag57,(char)tmp_len);

	while(1)
	{
		if(D2_Data[IndexOfD] == 'D')
		{
			D2_Data[IndexOfD] = '=';
			break;
		}
		IndexOfD++;
	}
	D2_Data[(tmp_len)*2+1] = '?';	
	
	D2_Data_Length = ((UCHAR)tmp_len+1)*2;

//0513ICTK add LRC
	D2_Data[D2_Data_Length] = VGL_LRC(D2_Data,D2_Data_Length);

	D2_Data_Length += 1 ;
//0513ICTK add LRC end
	
	//Handle PAN
	PAN_Len = IndexOfD-1;
	memcpy(PAN,&D2_Data[1],PAN_Len);
	
	//Handle ExpiryDate
	memcpy(ExpiryDate,&D2_Data[IndexOfD+1],4);
	if(UT_CheckMonth(ExpiryDate))
	{
		memcpy(Card_ExpiryDate,ExpiryDate,4);
	}

	//Handle ServiceCode
	memcpy(ServiceCode,&D2_Data[IndexOfD+1+4],3);

	//Handle PVV, if the digits after separator 'D' is 20 we have PVV, else We don't
	afterD = (tmp_len)*2 - IndexOfD;
	if(afterD == 0x15)  //means we have PIN Verification Field 
	{
		if(D2_Data[D2_Data_Length-3] == 0x46)	//minus LRC and "?" ,get the last byte padding with F
		{
			PVV_Len = 5;
			memcpy(PVV,&D2_Data[IndexOfD+1+4+3],5);
			memcpy(DCVV,&D2_Data[IndexOfD+1+4+3+5],3);
			memcpy(ATC12,&D2_Data[IndexOfD+1+4+3+5+3],2);
			memcpy(ATC34,&D2_Data[IndexOfD+1+4+3+5+3+2],2);
			memcpy(CL,&D2_Data[IndexOfD+1+4+3+5+3+2+2],1);
		}
			
	}
	else if(afterD == 0x14)  //means we have PIN Verification Field 
	{
		PVV_Len = 5;
		memcpy(PVV,&D2_Data[IndexOfD+1+4+3],5);
		memcpy(DCVV,&D2_Data[IndexOfD+1+4+3+5],3);
		memcpy(ATC12,&D2_Data[IndexOfD+1+4+3+5+3],2);
		memcpy(ATC34,&D2_Data[IndexOfD+1+4+3+5+3+2],2);
		memcpy(CL,&D2_Data[IndexOfD+1+4+3+5+3+2+2],1);		
	}	
	//Handle DCVV, if the digits after separator 'D' is 16 and padding with F 
	else if(afterD == 0x10)	
	{
		if(D2_Data[D2_Data_Length-3] == 0x46)	//last byte padding with F
		{
			PVV_Len = 0;
			memcpy(DCVV,&D2_Data[IndexOfD+1+4+3],3);
			memcpy(ATC12,&D2_Data[IndexOfD+1+4+3+3],2);
			memcpy(ATC34,&D2_Data[IndexOfD+1+4+3+3+2],2);
			memcpy(CL,&D2_Data[IndexOfD+1+4+3+3+2+2],1);
		}
		else											//last byte padding with F, that is Wrong Format
		{
			PVV_Len = 0;
		}
	}
	//Handle DCVV, if the digits after separator 'D' is 15
	else if(afterD == 0x0F)	
	{
		if(D2_Data[D2_Data_Length-3] != 0x46)	//last byte padding without F
		{
			PVV_Len = 0;
			memcpy(DCVV,&D2_Data[IndexOfD+1+4+3],3);
			memcpy(ATC12,&D2_Data[IndexOfD+1+4+3+3],2);
			memcpy(ATC34,&D2_Data[IndexOfD+1+4+3+3+2],2);
			memcpy(CL,&D2_Data[IndexOfD+1+4+3+3+2+2],1);
		}
		else											//last byte padding with F, that is Wrong Format
		{
			PVV_Len = 0;
		}
	}
	//Wrong Format
	/*
	If an incorrectly personalized card does not include dCVV, ATC, or Contactless Indicator in Track 2 Equivalent Data,
	the reader shall nevertheless build Track 1 using the following default values for the data that was not included: 000 for dCVV, 
	0000 for ATC and 1 for Contactless Indicator.	
	*/
	else 
	{
		PVV_Len = 0;
		//UT_PutStr(1, 0, FONT0, 19, (UCHAR *)"Track2 Wrong Format");
		//UT_WaitKey();
	}
//First, We Check tag5F20 (CardHolder Name), tag9F1F (Track1 Discretionary Data) 

	//Handle CardHolderName
	UT_Get_TLVLengthOfV(glv_tag5F20.Length,&CH_Len);
	if(CH_Len)
	{
		//CH_Len = ((UINT)*glv_tag5F20.Length)*2;
		//UT_Split(CHName,glv_tag5F20.Value,(char)*glv_tag5F20.Length);
		memcpy(CHName,glv_tag5F20.Value,CH_Len);
	}
	else	//Set default value
	{
		CH_Len = 2;
		CHName[0] = 0x20;
		CHName[1] = 0x2F;
	}

	//Handle Track1 Discretionary Data
	UT_Get_TLVLengthOfV(glv_tag9F1F.Length,&T1DD_Len);
	if(T1DD_Len != 0)
	{
		T1DD_Tmp_Len = T1DD_Len;
		//T1DD_Len = ((49-PAN_Len-CH_Len))*2;		// qVSDC Spec	
		//UT_Split(T1DD,glv_tag9F1F.Value,(char)*glv_tag9F1F.Length);
		memcpy(T1DD,glv_tag9F1F.Value,T1DD_Len);

		//ICTK , T1DD_Len has max lenght
		T1DD_Tmp_Len = 79-30-PAN_Len-CH_Len;

		if(T1DD_Tmp_Len >= T1DD_Len)
			T1DD_Len = T1DD_Len;
		else
			T1DD_Len = T1DD_Tmp_Len;
		//ICTK , T1DD_Len has max lenght end
	}

//Second, We store "PAN, CHName, ExpiryDate, ServiceCode, PVV, T1DD, CL, ATC12, DCVV, ATC34 " to Track1
	
	D1_Data[i++] = 'B';

	//PAN
	while(PAN_Len--)
	{
		D1_Data[i++] = PAN[j++];
	}
	j=0;

	D1_Data[i++] = '^';

	//CH Name
	while(CH_Len--)
	{
		D1_Data[i++] = CHName[j++];
	}
	j=0;
	
	D1_Data[i++] = '^';

	//ExpiryDate
	while(ExpiryDate_Len--)
	{
		D1_Data[i++] = ExpiryDate[j++];
	}
	j=0;

	//ServiceCode
	while(ServiceCode_Len--)
	{
		D1_Data[i++] = ServiceCode[j++];
	}
	j=0;
	
	//PVV
	while(PVV_Len--)
	{
		D1_Data[i++] = PVV[j++];
	}
	j=0;
	
	//T1DD	//Maximum length of field 10.1 = 79 �V 30 �V length of field 3(PAN) �V length of field 5(CHName)
	while(T1DD_Len--)
	{
		D1_Data[i++] = T1DD[j++];
	}
	j=0;

	//CL
	while(CL_Len--)
	{
		D1_Data[i++] = CL[j++];
	}
	j=0;

	//ATC12
	while(ATC12_Len--)
	{
		D1_Data[i++] = ATC12[j++];
	}
	j=0;

	//DCVV
	while(DCVV_Len--)
	{
		D1_Data[i++] = DCVV[j++];
	}
	j=0;

	//ATC34
	while(ATC34_Len--)
	{
		D1_Data[i++] = ATC34[j++];
	}
	j=0;

	D1_Data[i++] = '0' ;
	D1_Data[i++] = '0' ;
	D1_Data[i++] = '0' ;
	D1_Data[i++] = '0' ;

	D1_Data_Length = i ;

	//len of tag57 = 0 problem
	return SUCCESS;

}


UCHAR VGL_RR_MSD_Track(void)
{
//	UT_DumpHex(0,1,(UINT)*glv_tag57.Length,glv_tag57.Value);
	UCHAR *tmp_tag57=0;
	//UCHAR *VGL_MSD_Track1_Data = Track1;
	UINT IndexOfD=0,tmp_len=0,i=0,j=0;

	
	//Prepare for Copy the Track2 data (PAN, CardHolderName, ExpiryDate, ServiceCode, PVV, DCVV, ATC, CL) to the Array for future use
	UCHAR	PAN[19]={0}, ExpiryDate[4]={0}, ServiceCode[3]={0}, PVV[5]={0}
	, DCVV[3]={0x30,0x30,0x30}, ATC12[2]={0x30,0x30}, ATC34[2]={0x30,0x30}, CL[1]={0x31}, CHName[26]={0},T1DD[49]={0};
	//For Fixed Size
	UINT ExpiryDate_Len = 4, ServiceCode_Len = 3, DCVV_Len = 3, ATC12_Len = 2, ATC34_Len = 2 , CL_Len = 1;
	//For VAR Size
	UINT	PAN_Len = 0, PVV_Len = 0,CH_Len = 0, T1DD_Len = 0, T1DD_Tmp_Len =0 ,afterD = 0;

	//Card Expiry Date
	UCHAR Card_ExpiryDate[4] = {0};

	//Reset Whole Data
	VGL_MSD_Track2_Data_Length = 0;
	memset(VGL_MSD_Track2_Data,0x00,150);
	VGL_MSD_Track1_Data_Length = 0;
	memset(VGL_MSD_Track1_Data,0x00,150);
	
	//Ready to Construct Track1, Track2

	//Req 5.4.4.1 "Formatting_Track2"
	//if(Opt_MSD_Formatting_Track2)
	//{
	UT_Get_TLVLengthOfV(glv_tag57.Length,&tmp_len);

	//len of tag57 = 0 problem
	if(tmp_len == 0)
		return FAIL;
	
	memcpy(tmp_tag57,glv_tag57.Value,tmp_len);

	VGL_MSD_Track2_Data[0] = ';';
		
	UT_Split(&VGL_MSD_Track2_Data[1],tmp_tag57,(char)tmp_len);

	while(1)
	{
		if(VGL_MSD_Track2_Data[IndexOfD] == 'D')
		{
			VGL_MSD_Track2_Data[IndexOfD] = '=';
			break;
		}
		IndexOfD++;
	}
	VGL_MSD_Track2_Data[(tmp_len)*2+1] = '?';	
	
	//UT_DumpHex(0,0,(tmp_len+1)*2,VGL_MSD_Track2_Data);

	VGL_MSD_Track2_Data_Length = (tmp_len+1)*2;

//0513ICTK add LRC
	VGL_MSD_Track2_Data[VGL_MSD_Track2_Data_Length] = VGL_LRC(VGL_MSD_Track2_Data,VGL_MSD_Track2_Data_Length);

	VGL_MSD_Track2_Data_Length += 1 ;
//0513ICTK add LRC end

	//UT_DumpHex(0,0,VGL_MSD_Track2_Data_Length,VGL_MSD_Track2_Data);

	//UT_DispHexWord(6,0,VGL_MSD_Track2_Data_Length);
	//UT_WaitKey();
	//}	
//	UT_DumpHex(0,0,39,VGL_MSD_Track2_Data);


	//Req 5.4.4.2 "Construction_Track1"
	//if(Opt_MSD_Constructing_Track1)
	//{	
	
	//Handle PAN
	PAN_Len = IndexOfD-1;
	memcpy(PAN,&VGL_MSD_Track2_Data[1],PAN_Len);
	
	//Handle ExpiryDate
	memcpy(ExpiryDate,&VGL_MSD_Track2_Data[IndexOfD+1],4);
	if(UT_CheckMonth(ExpiryDate))
	{
		memcpy(Card_ExpiryDate,ExpiryDate,4);
	}

	//Handle ServiceCode
	memcpy(ServiceCode,&VGL_MSD_Track2_Data[IndexOfD+1+4],3);

	//Handle PVV, if the digits after separator 'D' is 20 we have PVV, else We don't
	afterD = (tmp_len)*2 - IndexOfD;
	if(afterD == 0x15)  //means we have PIN Verification Field 
	{
		if(VGL_MSD_Track2_Data[VGL_MSD_Track2_Data_Length-3] == 0x46)	//minus LRC and "?" ,get the last byte padding with F
		{
			PVV_Len = 5;
			memcpy(PVV,&VGL_MSD_Track2_Data[IndexOfD+1+4+3],5);
			memcpy(DCVV,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+5],3);
			memcpy(ATC12,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+5+3],2);
			memcpy(ATC34,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+5+3+2],2);
			memcpy(CL,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+5+3+2+2],1);
		}
			
	}
	else if(afterD == 0x14)  //means we have PIN Verification Field 
	{
		PVV_Len = 5;
		memcpy(PVV,&VGL_MSD_Track2_Data[IndexOfD+1+4+3],5);
		memcpy(DCVV,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+5],3);
		memcpy(ATC12,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+5+3],2);
		memcpy(ATC34,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+5+3+2],2);
		memcpy(CL,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+5+3+2+2],1);		
	}	
	//Handle DCVV, if the digits after separator 'D' is 16 and padding with F 
	else if(afterD == 0x10)	
	{
		if(VGL_MSD_Track2_Data[VGL_MSD_Track2_Data_Length-3] == 0x46)	//last byte padding with F
		{
			PVV_Len = 0;
			memcpy(DCVV,&VGL_MSD_Track2_Data[IndexOfD+1+4+3],3);
			memcpy(ATC12,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+3],2);
			memcpy(ATC34,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+3+2],2);
			memcpy(CL,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+3+2+2],1);
		}
		else											//last byte padding with F, that is Wrong Format
		{
			PVV_Len = 0;
		}
	}
	//Handle DCVV, if the digits after separator 'D' is 15
	else if(afterD == 0x0F)	
	{
		if(VGL_MSD_Track2_Data[VGL_MSD_Track2_Data_Length-3] != 0x46)	//last byte padding without F
		{
			PVV_Len = 0;
			memcpy(DCVV,&VGL_MSD_Track2_Data[IndexOfD+1+4+3],3);
			memcpy(ATC12,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+3],2);
			memcpy(ATC34,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+3+2],2);
			memcpy(CL,&VGL_MSD_Track2_Data[IndexOfD+1+4+3+3+2+2],1);
		}
		else											//last byte padding with F, that is Wrong Format
		{
			PVV_Len = 0;
		}
	}
	//Wrong Format
	/*
	If an incorrectly personalized card does not include dCVV, ATC, or Contactless Indicator in Track 2 Equivalent Data,
	the reader shall nevertheless build Track 1 using the following default values for the data that was not included: 000 for dCVV, 
	0000 for ATC and 1 for Contactless Indicator.	
	*/
	else 
	{
		PVV_Len = 0;
		//UT_PutStr(1, 0, FONT0, 19, (UCHAR *)"Track2 Wrong Format");
		//UT_WaitKey();
	}
//First, We Check tag5F20 (CardHolder Name), tag9F1F (Track1 Discretionary Data) 

	//Handle CardHolderName
	UT_Get_TLVLengthOfV(glv_tag5F20.Length,&CH_Len);
	if(CH_Len)
	{
		//CH_Len = ((UINT)*glv_tag5F20.Length)*2;
		//UT_Split(CHName,glv_tag5F20.Value,(char)*glv_tag5F20.Length);
		memcpy(CHName,glv_tag5F20.Value,CH_Len);
	}
	else	//Set default value
	{
		CH_Len = 2;
		CHName[0] = 0x20;
		CHName[1] = 0x2F;
	}

	//Handle Track1 Discretionary Data
	UT_Get_TLVLengthOfV(glv_tag9F1F.Length,&T1DD_Len);
	if(T1DD_Len != 0)
	{
		T1DD_Tmp_Len = T1DD_Len;
		//T1DD_Len = ((49-PAN_Len-CH_Len))*2;		// qVSDC Spec	
		//UT_Split(T1DD,glv_tag9F1F.Value,(char)*glv_tag9F1F.Length);
		memcpy(T1DD,glv_tag9F1F.Value,T1DD_Len);

		//ICTK , T1DD_Len has max lenght
		T1DD_Tmp_Len = 79-30-PAN_Len-CH_Len;

		if(T1DD_Tmp_Len >= T1DD_Len)
			T1DD_Len = T1DD_Len;
		else
			T1DD_Len = T1DD_Tmp_Len;
		//ICTK , T1DD_Len has max lenght end
	}

//Second, We store "PAN, CHName, ExpiryDate, ServiceCode, PVV, T1DD, CL, ATC12, DCVV, ATC34 " to Track1
	
	VGL_MSD_Track1_Data[i++] = '%';
	VGL_MSD_Track1_Data[i++] = 'B';

	//PAN
	while(PAN_Len--)
	{
		VGL_MSD_Track1_Data[i++] = PAN[j++];
	}
	j=0;

	VGL_MSD_Track1_Data[i++] = '^';

	//CH Name
	while(CH_Len--)
	{
		VGL_MSD_Track1_Data[i++] = CHName[j++];
	}
	j=0;
	
	VGL_MSD_Track1_Data[i++] = '^';

	//ExpiryDate
	while(ExpiryDate_Len--)
	{
		VGL_MSD_Track1_Data[i++] = ExpiryDate[j++];
	}
	j=0;

	//ServiceCode
	while(ServiceCode_Len--)
	{
		VGL_MSD_Track1_Data[i++] = ServiceCode[j++];
	}
	j=0;
	
	//PVV
	while(PVV_Len--)
	{
		VGL_MSD_Track1_Data[i++] = PVV[j++];
	}
	j=0;
	
	//T1DD	//Maximum length of field 10.1 = 79 �V 30 �V length of field 3(PAN) �V length of field 5(CHName)
	while(T1DD_Len--)
	{
		VGL_MSD_Track1_Data[i++] = T1DD[j++];
	}
	j=0;

	//CL
	while(CL_Len--)
	{
		VGL_MSD_Track1_Data[i++] = CL[j++];
	}
	j=0;

	//ATC12
	while(ATC12_Len--)
	{
		VGL_MSD_Track1_Data[i++] = ATC12[j++];
	}
	j=0;

	//DCVV
	while(DCVV_Len--)
	{
		VGL_MSD_Track1_Data[i++] = DCVV[j++];
	}
	j=0;

	//ATC34
	while(ATC34_Len--)
	{
		VGL_MSD_Track1_Data[i++] = ATC34[j++];
	}
	j=0;

	VGL_MSD_Track1_Data[i++] = '0' ;
	VGL_MSD_Track1_Data[i++] = '0' ;
	VGL_MSD_Track1_Data[i++] = '0' ;
	VGL_MSD_Track1_Data[i++] = '0' ;
	VGL_MSD_Track1_Data[i++] = '?' ;

	VGL_MSD_Track1_Data_Length = i ;
	
	//0513ICTK Track1 LRC
	VGL_MSD_Track1_Data[VGL_MSD_Track1_Data_Length] = VGL_LRC(VGL_MSD_Track1_Data,VGL_MSD_Track1_Data_Length);

	VGL_MSD_Track1_Data_Length += 1;
	//0513 ICTK Track1 LRC end
	
	//UT_DispHexWord(0,0,VGL_MSD_Track1_Data_Length);
	//UT_DumpHex(0,1,VGL_MSD_Track1_Data_Length,VGL_MSD_Track1_Data);

	return SUCCESS;
}

// Req5.4.3.1 "Cryptogram Information Data"
void VGL_RR_qVSDC_Handle_CID(void)
{
	UCHAR tmp_value = 0,tmp_value1 = 0;
	UINT Tag97_VLen = 0;

	UT_Get_TLVLengthOfV(glv_tag9F27.Length,&Tag97_VLen);
	if(Tag97_VLen == 0)	//Estimate CID
	{
		//UT_DumpHex(0,2,32,glv_tag9F10.Value);
		//UT_WaitKey();
		memset(glv_tag9F27.Value,0x00,1);		//reset to 0x00
		tmp_value = glv_tag9F10.Value[4];	
		glv_tag9F27.Length[0] = 0x01;		//initiate 9F27 Length
		glv_tag9F27.Value[0] = ((tmp_value & 0x30) << 2);	//copy Issue App Data(9F10) byte5 bit5,6 to CID(9F27) byte1 bit8,7
	}

	//Reg5.4.3.2 "Cryptogram Type Transaction Disposition" - Compute the CID(9F27) byte1 bit8,7
	tmp_value1 = glv_tag9F66.Value[1];		//TTQ byte2 
	tmp_value = glv_tag9F27.Value[0];
	tmp_value &= 0xC0;
	if(tmp_value == 0x00)					//Application Authentication Cryptogram(AAC) check
	{
		VGL_AAC = 1;
		//Refund Always Approval
		if(glv_tag9C.Value[0] != 0x20)	
			VGL_Decline_Required_by_Reader = 1;
	}
	else if(tmp_value == 0x40)				//Transaction Certificate(TC) 
	{
		VGL_TC = 1;
		VGL_Decline_Required_by_Reader = 0;
		VGL_Online_Required_by_Reader = 0;
	}
	else if((tmp_value == 0x80) || (tmp_value1 & 0x80))	//Authorisation Request Cryptogram(ARQC) & TTQ byte2 bit8 check
	{
		VGL_ARQC = 1;
		VGL_Online_Required_by_Reader = 1;
	}
	else								//Can't determine
		VGL_Decline_Required_by_Reader = 1;			

	
}


//Reg 5.4.2.1 "Mandatory Data"
UCHAR VGL_Check_M_Tag(void)
{
	
	//UT_DispHexByte(2,0,VGL_tag57);
	//UT_DispHexByte(3,0,VGL_tag82);
	//UT_DispHexByte(4,0,VGL_tag9F10);
	//UT_DispHexByte(5,0,VGL_tag9F26);
	//UT_DispHexByte(6,0,VGL_tag9F36);
				
	//UT_WaitKey();


	if(VGL_qVSDC_Path)
	{
		if(VGL_tag57 && VGL_tag82 && VGL_tag9F10 && VGL_tag9F26 && VGL_tag9F36)
			return SUCCESS;
		else
			return FAIL;
	}
	/*	//20131204 case CLM.R.004.0104, CLMG.0070005 , if tag 9F26 is missing, we process CVN142 transaction.
	if(VGL_MSD_Path)
	{
		if(VGL_tag9F26)
		{
			if(VGL_tag57 && VGL_tag82 && VGL_tag9F10 && VGL_tag9F36)
				return SUCCESS;
			else
				return FAIL;
		}
		else
		{
			if(VGL_tag57 && VGL_tag82)
				return SUCCESS;
			else
			return FAIL;
		}
	}*/
	//20131204 case CLM.R.004.0104, CLMG.0070005 , if tag 9F26 is missing, we process CVN142 transaction. Start
	if(VGL_MSD_Path)
	{
		if(VGL_tag9F26)
		{
			if(VGL_tag57 && VGL_tag82 && VGL_tag9F10 && VGL_tag9F36)
			{
				VGL_Kernel_CVN17 = TRUE;	//add this flag to decide Perform CVN17 transaction or not
				return SUCCESS;
			}
			else
				return FAIL;
		}
		else
		{
			if(VGL_tag57 && VGL_tag82)
			{
				VGL_Kernel_CVN17 = FALSE;	//add this flag to decide Perform CVN17 transaction or not
				return SUCCESS;
			}
			else
				return FAIL;
		}
	}

	//20131204 case CLM.R.004.0104, CLMG.0070005 , if tag 9F26 is missing, we process CVN142 transaction. end
	return FAIL;
}

UCHAR VGL_Check_PAN_Consistency(void)
{
	UINT	lenOf5A;
	UINT	lenOf57;
	UCHAR	bufPAN[ECL_LENGTH_5A*2];
	UCHAR	bufTr2[ECL_LENGTH_5A*2];
	UCHAR	numPAN=0;
	UCHAR	numTr2=0;
	UCHAR	i;
	
	UT_Get_TLVLengthOfV(glv_tag5A.Length, &lenOf5A);
	if (lenOf5A != 0)
	{
		UT_Get_TLVLengthOfV(glv_tag57.Length, &lenOf57);
		if (lenOf57 != 0)
		{
			//Convert to ASCII
			UT_Split(bufPAN, glv_tag5A.Value, ECL_LENGTH_5A);
			UT_Split(bufTr2, glv_tag57.Value, ECL_LENGTH_5A);

			//Get PAN Number
			for (i=0; i < (ECL_LENGTH_5A*2); i++)	
			{
				if ((i == (lenOf5A*2)) || (bufPAN[i] == 'F'))
				{
					numPAN=i;
					break;
				}
			}

			//Get Account Number
			for (i=0; i < (ECL_LENGTH_5A*2); i++)
			{
				//Max. PAN is 19, 'D' must be included in (ECL_LENGTH_5A*2)
				if (bufTr2[i] == 'D')
				{
					numTr2=i;
					break;
				}
			}

			//Check Consistency
			if ((numPAN == numTr2) && (!memcmp(bufPAN, bufTr2, numPAN)))
			{
				return SUCCESS;
			}
		}
		else
		{
			return SUCCESS;
		}
	}
	else
	{
		return SUCCESS;
	}
	
	return FAIL;
}

//FUNCTION : HandleReq 5.4 "Card Read Complete"
UCHAR VGL_RR_Complete(void)
{
	UCHAR rspCode;
//	UCHAR Temp_PAN[20] = {0};
	UINT tag_len=0;

	//0622 For AOSA 
	UCHAR Damount[13]={0},amount[6]={0};
	UINT tag_VLen=0,i=0;

	//0622 ICTK Copy the AOSA to Terminal and print, if it present
	memset(VGL_Print_AOSA_Amount,0x00,sizeof(VGL_Print_AOSA_Amount));
	UT_Get_TLVLengthOfV(glv_tag9F5D.Length,&tag_VLen);
	if(tag_VLen)
	{
		memcpy(amount,glv_tag9F5D.Value,glv_tag9F5D.Length[0]);
		UT_Split(Damount,amount,6);

		Damount[12] = Damount[11];
		Damount[11] = Damount[10];
		Damount[10] = 0x2E;
		
		VGL_Print_AOSA_Amount[0] = (UCHAR)(tag_VLen*2);
		memcpy(&VGL_Print_AOSA_Amount[1],Damount,13);

		//for Other data in Reader Interface 
		Available_Offline_Amount[0] = (UCHAR)tag_VLen;
		memcpy(&Available_Offline_Amount[1],glv_tag9F5D.Value,6);
	}

	tag_VLen = 0;

	//0622 ICTK Copy the PAN to Terminal and print, if it present
	memset(VGL_Print_PAN,0x00,sizeof(VGL_Print_PAN));
	UT_Get_TLVLengthOfV(glv_tag5A.Length,&tag_VLen);
	if(tag_VLen)
	{
		VGL_Print_PAN[0] = (UCHAR)(tag_VLen*2);
		for(i=0;i<tag_VLen;i++)
		{
			UT_hexb2ascw(glv_tag5A.Value[i],&VGL_Print_PAN[i*2+1],&VGL_Print_PAN[(i+1)*2]);
		}
	}	
	//VCPS 2.1.3 Update No. 34
/*	else
	{
		UT_Get_TLVLengthOfV(glv_tag57.Length,&tag_VLen);
		if(tag_VLen)
		{	
			UT_Split(Temp_PAN,glv_tag57.Value,10);
			
			for(i=0;i<20;i++)
			{
				if(Temp_PAN[i] == 'D')
				{
					VGL_Print_PAN[0] = (UCHAR)(i);
					memcpy(&VGL_Print_PAN[1],Temp_PAN,i);
					break;
				}
			}
		}
	}
*/
	
	//0702 ICTK FOR FFI , if the TTQ setting is qVSDC setting, the FFI last bytes last 4 bits will change to zero
	rspCode = UT_Get_TLVLengthOfV(glv_tag9F6E.Length,&tag_len);
	if(tag_len)
	{
		if(glv_tag9F66.Value[0] & 0x20)
		{
			glv_tag9F6E.Value[tag_len-1] &= 0xF0;
		}
	}

	//0514 ICTK
	ETP_UI_Request(ETP_OCP_UIM_CardReadOKRemoveCard,00);

	if(!memcmp(VGL_LastTxnPAN,VGL_Print_PAN,VGL_Print_PAN[0]+1))
	{
		VGL_Double_DIP_Timer=OS_GET_SysTimerFreeCnt();
		//check time
		if((VGL_Double_DIP_Timer - VGL_LastTxnPANTime) < VAP_VISA_P_DOUBLE_DIP)
			return FAIL;
	}		
	//20131224 change handle D1 D2 logic
	//Build D1, D2 Data
	rspCode = VGL_AS210_D1_Track();
	if(rspCode == FAIL)
		return FAIL;
	
	//Req 5.4.2 is Done when Receiving Read Record Requirement
	rspCode = VGL_Check_M_Tag();

	if(rspCode == FAIL)
	{
		return FAIL;
	}

	//VCPS 2.1.3 Update No. 65
	rspCode=VGL_Check_PAN_Consistency();
	if (rspCode == FAIL)
	{
		return FAIL;
	}
	
	//Req 5.4.3 "Determine Card Transaction Disposition"
	if(VGL_qVSDC_Path)
	{
		//Req 5.4.3.1 "Cryptogram Information Data"
		VGL_RR_qVSDC_Handle_CID();
	
		return VGL_qVSDC_Process;
	}

	//Req 5.4.4 Mag-Stripe Mode Path Processing ?Online Disposition
	if(VGL_MSD_Path)
	{	
		//VCPS 2.1.3 Update - Modify Create MSD Track Function
		//rspCode = VGL_RR_MSD_Track();
		rspCode = VGL_Create_MSDTrack();

		//20131205 tag57 problem
		if(rspCode == FAIL)
			return FAIL;
		
		return VGL_MSD_Process;		
	}
	
	return FAIL;

}

//Req 5.4.2.2 "Redundant Data" & store tag70
UCHAR VGL_Check_RR_R_Tag(UCHAR SFINUMBER,UINT RRR_Len,UCHAR *RRR_Buf)
{
	UCHAR redundant=0,tag9F02[4] = {0x9F,0x02,0x00,0x00};
	UCHAR TLength,LLength,tmplen=0,*value_addr=0,*len_addr=0,rspCode;//,*ptrrec,tmptag;
	UINT tmp_tag_len,VLength;
	//UINT iTempLen;
	ULONG Index;

	//ICTK check Read Record Wrong Format		
	//Check the Format First
	UT_Get_TLVLengthOfT(RRR_Buf,&TLength);
	UT_Get_TLVLengthOfL(&RRR_Buf[TLength],&LLength);
	UT_Get_TLVLengthOfV(&RRR_Buf[TLength],&VLength);
	
	//UT_DispHexByte(0,0,RRR_Buf[TLength+LLength+VLength]);
	//UT_DispHexByte(0,2,RRR_Buf[TLength+LLength+VLength+1]);
	//UT_WaitKey();
	if(!((RRR_Buf[TLength+LLength+VLength] == 0x90) && (RRR_Buf[TLength+LLength+VLength+1] == 0x00)))
		return FAIL;

	if(RRR_Buf[0] != 0x70)
	{
		//ICTK 0627 test case CLQ.N.076
		if(SFINUMBER<11)
		{
			return FAIL;
		}
	}
	//ICTK check Read Record Wrong Format	end
	
	while(tmplen != (RRR_Len-2))
	{
		//tmptag = *RRR_Buf;	//get first byte tag to check
		TLength = LLength = tmp_tag_len = 0;
		Index = 0;

		//is tag a Primitive data element ?
		//if(tmptag & 0x20)		//it's a Constructed data element
		if((*RRR_Buf & 0x20) && (*RRR_Buf != 0xFF))
		{	
			UT_Get_TLVLengthOfT(RRR_Buf,&TLength);	//get tag's Length
//20130306
//			UT_DumpHex(0,0,TLength,RRR_Buf);
			
			tmplen += TLength;			
			RRR_Buf += TLength;			//shift buf pointer, and now we are in "length" field
						
			UT_Get_TLVLengthOfL(RRR_Buf,&LLength);	//get length's Length
		
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
//			UT_DumpHex(0,0,2,RRR_Buf);
			UT_Get_TLVLengthOfT(RRR_Buf,&TLength);	//get tag's Length

//20130306
			//UT_DumpHex(0,1,TLength,RRR_Buf);
			
			rspCode = UT_Search(TLength,RRR_Buf,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);	//search glv_tagTable and get the index	
			if(rspCode == SUCCESS)
			{						
				if(*glv_addTable[Index] != 0)		//test if it is redundant?
				{	
					//UT_DispHexWord(0,0,Index);
					//UT_DumpHex(0,1,4,glv_tagTable[Index].Tag);
					//0507 ICTK, when card return 9F02 , Ignore it
					//if((*RRR_Buf != 0x9F)&&((*RRR_Buf+1) != 0x02))
					if(memcmp(glv_tagTable[Index].Tag,tag9F02,4))
					{
						//UT_DispHexWord(0,0,Index);
						//UT_WaitKey();
						//UT_DumpHex(0,1,14,(UCHAR*)glv_addTable[Index]);
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
//					UT_DumpHex(0,1,(RRR_Len-tmplen),RRR_Buf);
			}
			else			//can't recognize, ignore it
			{
				//UT_ClearScreen();
				//UT_DumpHex(0,2,TLength,RRR_Buf);
				//UT_PutStr(1, 0, FONT0, 16, (UCHAR *)"CAN NOT FIND TAG");
				//UT_WaitKey();
				
				tmplen += TLength;
				RRR_Buf += TLength;			//shift buf pointer, and now we are in "length" field

				UT_Get_TLVLengthOfL(RRR_Buf,&LLength);	//get length's Length
				tmplen += LLength;
				UT_Get_TLVLengthOfV(RRR_Buf,&tmp_tag_len);
				tmplen += tmp_tag_len;	
				//0605 VISA 
				RRR_Buf += LLength;				
				RRR_Buf += tmp_tag_len;	
			}
		}
	}
	if(redundant == 0x01)
	{
		//UT_ClearScreen();
		//UT_DumpHex(0,0,TLength,RRR_Buf);
		//UT_PutStr(1, 0, FONT0, 16, (UCHAR *)"redundant RR TAG");
		//UT_WaitKey();
		return FAIL;
	}	
	else
		return SUCCESS;
}


UCHAR VGL_RAD_SAD_Data(UCHAR SFINUM,UINT RECNUM,UINT RRR_Len,UCHAR *RRR_Buf)
{
	//We only store the record that depends on AFL Byte4 
	//Store whole Tag70 for OFDA
	if(RRR_Buf[0] == 0x70)
	{		
		// (1) check record LENGTH & template TAG=70
		if( (RRR_Buf[1] & 0x80) == 0 )
		{
			//iTempLen = RRR_Buf[1]; // 1-byte length (1..127)
		  	//ptrrec = &RRR_Buf[2];	// pointer to the starting of DO
		  	memcpy((ADDR_VGL_REC_START+VGL_REC_LEN*(RECNUM)),&SFINUM,1);
		  	memcpy((ADDR_VGL_REC_START+VGL_REC_LEN*(RECNUM))+1,RRR_Buf,RRR_Len-2);	//minus SW1,SW2
		}
		else // 2-byte length
		{
			switch( RRR_Buf[1] & 0x7f )
			{
				case 0x01: // 1-byte length (128..255)
					//iTempLen = RRR_Buf[2];
					//ptrrec = &RRR_Buf[3];
					memcpy((ADDR_VGL_REC_START+VGL_REC_LEN*(RECNUM)),&SFINUM,1);
		  			memcpy((ADDR_VGL_REC_START+VGL_REC_LEN*(RECNUM))+1,RRR_Buf,RRR_Len-2);	//minus SW1,SW2
					break;
					
				case 0x02: // 2-byte length (256..65535)
					//iTempLen = RRR_Buf[2]*256 + RRR_Buf[3];
					//ptrrec = &RRR_Buf[4];				
					memcpy((ADDR_VGL_REC_START+VGL_REC_LEN*(RECNUM)),&SFINUM,1);
		  			memcpy((ADDR_VGL_REC_START+VGL_REC_LEN*(RECNUM))+1,RRR_Buf,RRR_Len-2);	//minus SW1,SW2
					break;
				default:   // out of spec
					return FAIL;
			}
		}
		//UT_DumpHex(0,0,RRR_Len+1-2,(ADDR_VGL_REC_START+VGL_REC_LEN*RECNUM));
		//if( apk_CheckIsoPadding_Right( RRR_Buf, padlen ) == FALSE )
	       //	return( emvFailed );
	}
	return SUCCESS;
}

//5.3.2 Read Application Data,  if AFL returned
UCHAR VGL_RAD_Process(void)
{
	UCHAR SFINUM=0,rspCode,AFL_B4=0,j,temcnt;
	UINT tag94_len=0,i,RECNUM=0;

	//20130626 ICTK, Reset the ADDR_VGL_REC_Buf , for case n085 and s016, for n085, it has remain data stored in ADDR_VGL_REC_Buf, 
	// so the s016 will save the remain data and api_sys_SHA1 will recover wrong data to examine
	ODA_Clear_Record();

	//UT_DumpHex(0,0,30,glv_tag94.Value);
	UT_EMVCL_GetBERLEN(glv_tag94.Length,&temcnt,&tag94_len);

	//UT_Get_TLVLengthOfV(glv_tag94.Length,&tag94_len);
	//tag94_len = glv_tag94.Length[0];

	for(i=0;i<tag94_len;i+=4)
	{
		SFINUM = glv_tag94.Value[i];
		SFINUM = SFINUM>>3;
		AFL_B4 = glv_tag94.Value[i+3];
						
		for(j=glv_tag94.Value[i+1];j<=glv_tag94.Value[i+2];j++)
		{
			//reset the Read Record Buff
			VGL_rcvLen = 0;
			memset(VGL_rcvBuff,0,DEP_BUFFER_SIZE_RECEIVE);
			//send RR Command and Receive
			rspCode = ECL_APDU_Read_Record(j,SFINUM,&VGL_rcvLen,VGL_rcvBuff);
			//UT_DumpHex(0,0,VGL_rcvLen,VGL_rcvBuff);				
			
			if(rspCode == ECL_LV1_SUCCESS)
			{
				//UT_DumpHex(0,0,VGL_rcvLen,VGL_rcvBuff);
								
				rspCode = UT_Check_SW12(&VGL_rcvBuff[VGL_rcvLen-2], STATUSWORD_9000);
				if(rspCode != TRUE)
				{
					//UT_DispHexWord(0,0,VGL_rcvLen);
					//UT_DumpHex(0,1,VGL_rcvLen,VGL_rcvBuff);
					//UT_PutStr(0,5,FONT0,1,(UCHAR *)"a");
					//UT_WaitKey();
					return FAIL;
				}
				//ICTK
				//if(VGL_MSD_Path)
				//{
				//if(apk_EMVCL_FindTag(0x57,0x00,VGL_rcvBuff) != (UCHAR *)0)
				//	VGL_tag57 = 1;
				//}

				VGL_M_Tag(VGL_rcvBuff);
				
				if(AFL_B4)
				{
					VGL_RAD_SAD_Data(SFINUM,RECNUM,VGL_rcvLen,VGL_rcvBuff);
					AFL_B4--;
					RECNUM++;
				}
			
				rspCode = VGL_Check_RR_R_Tag(SFINUM,VGL_rcvLen,VGL_rcvBuff);
				if(rspCode == FAIL)
				{
					return FAIL;
				}
					
				//UT_ClearScreen();
				//UT_DumpHex(0,0,VGL_rcvLen,VGL_rcvBuff);
						
				//UT_WaitKey();
			}
			//20130627 Timeout 
			else if((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
			{
				etp_flgCmdTimeout=TRUE;
				return 0xFE;
			}
			//20130626 contact interrupt
			else if(rspCode == ECL_LV1_STOP_ICCARD)
			{
				return 0xFF;
			}
			else
			{
				return rspCode;
			}
		}
	}
	return SUCCESS;
}


// ---------------------------------------------------------------------------
// FUNCTION: verify the contents of all AFL entries.
// INPUT   : none.
// OUTPUT  : none.
// REF     : glv_tag94.Value
// RETURN  : TURE  - correct AFL.
//           FALSE - incorrect AFL.
// ---------------------------------------------------------------------------
UCHAR VGL_Verify_AFL( UCHAR *AFL_Data,UINT AFL_Length)
{
UCHAR i;
UCHAR sfi;
UCHAR st_rec_num;
UCHAR end_rec_num;
UCHAR oda_rec_cnt;

	 //UT_DumpHex(0,0,AFL_Length,AFL_Data);

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

         {	//UT_DispHexByte(4,0,sfi);
			//UT_DispHexByte(4,3,st_rec_num);
			//UT_DispHexByte(4,6,st_rec_num);
			//UT_DispHexByte(4,9,oda_rec_cnt);
			//UT_WaitKey();
           return FALSE ;
         }
      }

      return TRUE ;
}


UCHAR VGL_AIP_AFL_Process (void)
{	
	//Application Interchang Prefile 
	UCHAR *data_point = 0;
	UCHAR AIP[2] = {0};
	UCHAR cnt,AFL_Data[252]={0};
	UINT Value_Len=0,AFL_Length=0;
	UCHAR tag82[1]={0x82};
	UCHAR tag94[1]={0x94};

    UT_EMVCL_GetBERLEN( &VGL_rcvBuff[1], &cnt ,&Value_Len); // get total length of data elements
//	UT_DumpHex(0,0,VGL_rcvLen,VGL_rcvBuff);
	// check legal length
    if( Value_Len < 6 )
         return FAIL;

    // check response format (1)
    if( VGL_rcvBuff[0] == 0x80 )
    {
       	// save AIP[2]
        AIP[0] = VGL_rcvBuff[1+cnt]; // 2+(cnt-1)
        AIP[1] = VGL_rcvBuff[2+cnt]; // 3+(cnt-1)
        glv_tag82.Length[0] = 0x02;
        memcpy(glv_tag82.Value,AIP,2);
		VGL_tag82 = 1;

        // save AFL[4n]
	    if((Value_Len-2)%4 == 0)
	    {
	    	AFL_Length = Value_Len-2;
        	memcpy(AFL_Data,&VGL_rcvBuff[3+cnt],Value_Len-2); 	//0514 ICTK tag length is 2 bytes
	    }
		else
			return FAIL;

// PATCH: 2003-06-12 JCB SPEC (also good for EMV SPEC)
//        To verify AFL before actual reading records.
       	if( VGL_Verify_AFL(AFL_Data,AFL_Length) == FALSE )
       		return	FAIL;
		else	//ICTK
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

	// check response format (2)
	if( (VGL_rcvBuff[0] == 0x77))
    {
		// find AIP[2]
//	    data_point= apk_EMVCL_FindTag( 0x82, 0x00, &VGL_rcvBuff[0] );
//		if( data_point != 0 )
		data_point=UT_Find_Tag(tag82, (VGL_rcvLen-2), VGL_rcvBuff);
	    if ( data_point != NULLPTR )
	    {
			data_point++;	//Point to L
        	UT_EMVCL_GetBERLEN( data_point, &cnt ,&Value_Len);
            if(Value_Len==0)
            	return FAIL; // AIP not present

			//ICTK, AIP length = 3, it should be terminated
			if(Value_Len != 2)
				return FAIL;
            // save AIP[2]
            AIP[0] = *(data_point+cnt);
            AIP[1] = *(data_point+cnt+1);
			//*glv_tag82.Length = 0x02;
           	//memcpy(glv_tag82.Value,AIP,2);
		}
        else
            return FAIL; // AIP not present

        // find AFL[4n]
//        data_point = apk_EMVCL_FindTag( 0x94, 0x00, &VGL_rcvBuff[0] );
//        if( data_point != 0 )
		data_point=UT_Find_Tag(tag94, (VGL_rcvLen-2), VGL_rcvBuff);
	    if ( data_point != NULLPTR )
        {
			data_point++;	//Point to L
        	UT_EMVCL_GetBERLEN( data_point, &cnt ,&Value_Len);

            // save AFL[4n]
            if((Value_Len % 4)==0)
            {
            	AFL_Length = Value_Len;
				memcpy(AFL_Data,&data_point[cnt],AFL_Length);
            }
        }
		//else
		//	return FAIL; // AFL not present

        // PATCH: 2003-06-12 JCB SPEC (also good for EMV SPEC)
        //        To verify AFL before actual reading records.
        if( VGL_Verify_AFL(AFL_Data,AFL_Length) == FALSE )
            return FAIL;
    }
//	UT_DumpHex(0,0,2,glv_tag82.Value);
//	UT_DumpHex(0,1,40,glv_tag94.Value);

	return SUCCESS; // done
		
}

UCHAR VGL_Initiate_Choose_Path_Process (void)
{	
	if(((glv_tag9F66.Value[0] & 0x20) == 0x00) && (glv_tag9F66.Value[0] & 0x80))
	{
		//Req 5.4.4
		VGL_MSD_Path = TRUE;
		//0524
		glv_tag9F39.Length[0] = 0x01;
		glv_tag9F39.Value[0] = 0x91;
		return SUCCESS;
	}
	else if((glv_tag9F66.Value[0] & 0x20)&&((glv_tag9F66.Value[0] & 0x80) == 0x00))
	{
		//Req 5.4.3
		VGL_qVSDC_Path = TRUE;
		//0524
		glv_tag9F39.Length[0] = 0x01;
		glv_tag9F39.Value[0] = 0x07;
		return SUCCESS;
	}
	else if(glv_tag9F66.Value[0] & 0xA0)	//qVSDC && MSD both support
	{
		//check AIP
		if(glv_tag82.Value[1] & 0x80 )
		{
			//Req 5.4.4
			VGL_MSD_Path = TRUE;
			//0524
			glv_tag9F39.Length[0] = 0x01;
			glv_tag9F39.Value[0] = 0x91;	
			return SUCCESS;
		}
		else if((glv_tag82.Value[1] & 0x80) == 0x00)
		{
			//Req 5.4.3
			VGL_qVSDC_Path = TRUE;
			//0524
			glv_tag9F39.Length[0] = 0x01;
			glv_tag9F39.Value[0] = 0x07;
			return SUCCESS;
		}
		else	//Wrong Format
		{
			return FAIL;			
		}
	}
	else
		return FAIL;
}


UCHAR VGL_Check_SW12(UCHAR *datSW)
{
	if ((datSW[0] == 0x90) && (datSW[1] == 0x00))
		return VGL_SW9000;

	else if ((datSW[0] == 0x69) && (datSW[1] == 0x84))
		return VGL_SW6984;

	else if ((datSW[0] == 0x69) && (datSW[1] == 0x85))
		return VGL_SW6985;

	else if ((datSW[0] == 0x69) && (datSW[1] == 0x86))
		return VGL_SW6986;

	else
		return FAIL;
}


UCHAR VGL_Check_GPO_R_Data (UINT GPOR_Len,UCHAR *GPOR_Buf)
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
		else				//this tag is a Primitive data element
		{
			UT_Get_TLVLengthOfT(GPOR_Buf,&TLength);	//get tag's Length

			rspCode = UT_Search(TLength,GPOR_Buf,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);	//search glv_tagTable and get the index 
			if(rspCode == SUCCESS)
			{				
				len_addr = (UCHAR *)(glv_addTable[Index]);	//point to glv_addtable length address
				
				if(*len_addr != 0)
				{
					//ICTK 0513 GPO response 9F02, just ignore it
					//if((GPOR_Buf[0] != 0x9F)&&(GPOR_Buf[1] != 0x02))
					if(memcmp(glv_tagTable[Index].Tag,tag9F02,4))
					{					
						//UT_DumpHex(0,0,2,&GPOR_Buf[0]);
						//UT_DumpHex(0,1,10,(UCHAR *)(glv_addTable[Index]));
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
					//ICTK 0513 GPO response 9F02, just ignore it,end
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
			
				
				//20131210	CLM.S.002.00
				tmplen += TLength;
				GPOR_Buf += TLength;			//shift buf pointer, and now we are in "length" field

				UT_Get_TLVLengthOfL(GPOR_Buf,&LLength);				//get length's Length
				UT_Get_TLVLengthOfV(GPOR_Buf,&VLength);				//get Value's Length
				tmplen += LLength;
				tmplen += VLength;

				GPOR_Buf += LLength;
				GPOR_Buf += VLength;
				//20131210 end
				//UT_PutStr(1, 0, FONT0, 17, (UCHAR *)"GPO CANT FIND TAG");
				//UT_WaitKey();
			}
		}		
	}

	if(redundant == 0x01)
		return FAIL;
	else 
		return SUCCESS;
		
}

void VGL_M_Tag(UCHAR *Buf)
{
/*	if(apk_EMVCL_FindTag(0x57,0x00,Buf))
		VGL_tag57 = 0x01;
	if(apk_EMVCL_FindTag(0x82,0x00,Buf))
		VGL_tag82 = 0x01;
	if(apk_EMVCL_FindTag(0x9F,0x10,Buf))
		VGL_tag9F10 = 0x01;
	if(apk_EMVCL_FindTag(0x9F,0x26,Buf))
		VGL_tag9F26 = 0x01;
	if(apk_EMVCL_FindTag(0x9F,0x36,Buf))
		VGL_tag9F36 = 0x01;
*/
	UCHAR	*ptrData=NULLPTR;
	UCHAR	tag57[1]={0x57};
	UCHAR	tag82[1]={0x82};
	UCHAR	tag9F10[2]={0x9F,0x10};
	UCHAR	tag9F26[2]={0x9F,0x26};
	UCHAR	tag9F36[2]={0x9F,0x36};

	ptrData=UT_Find_Tag(tag57, (VGL_rcvLen-2), VGL_rcvBuff);
	if (ptrData != NULLPTR) VGL_tag57 = TRUE;

	ptrData=UT_Find_Tag(tag82, (VGL_rcvLen-2), VGL_rcvBuff);
	if (ptrData != NULLPTR) VGL_tag82 = TRUE;

	ptrData=UT_Find_Tag(tag9F10, (VGL_rcvLen-2), VGL_rcvBuff);
	if (ptrData != NULLPTR) VGL_tag9F10 = TRUE;

	ptrData=UT_Find_Tag(tag9F26, (VGL_rcvLen-2), VGL_rcvBuff);
	if (ptrData != NULLPTR) VGL_tag9F26 = TRUE;

	ptrData=UT_Find_Tag(tag9F36, (VGL_rcvLen-2), VGL_rcvBuff);
	if (ptrData != NULLPTR) VGL_tag9F36 = TRUE;
}


UCHAR VGL_Initiate_Process_GPO (void)
{
	UCHAR rspCode;
	UINT  PDOL_Len;

	//PDOL = glv_tag9F38.Value;
	UT_Get_TLVLengthOfV(glv_tag9F38.Length,&PDOL_Len);
	
	//	Handle PDOL Data
	rspCode = DOL_Get_DOLData(PDOL_Len,glv_tag9F38.Value,(UCHAR *)&VGL_sendLen,VGL_sendBuff);

	if(rspCode == SUCCESS)
	{	
		//send GPO Command
		rspCode = ECL_APDU_Get_ProcessingOptions(VGL_sendLen,VGL_sendBuff,&VGL_rcvLen,VGL_rcvBuff);

		if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
			etp_flgCmdTimeout=TRUE;
		
		if(rspCode == ECL_LV1_SUCCESS)
		{
			//rspCode = UT_Check_SW12(&VGL_rcvBuff[VGL_rcvLen-2], STATUSWORD_9000);
			//if(rspCode != TRUE)
				
			//UT_DumpHex(0,0,VGL_rcvLen,VGL_rcvBuff);
			//load data to GPOBuff

			VGL_M_Tag(VGL_rcvBuff);
		}
		else	//20130626 ICTK contact Interrupt
		{
			return rspCode;
		}
	}
	else
		return FAIL;	//20140805 add if handle PDOL fail, it should return fail.

	return SUCCESS;
}


UCHAR VGL_Initiate_App_Process(void)
{
	UCHAR rspCode = 0;

	//reset Buffer
	VGL_rcvLen = 0;
	memset(VGL_rcvBuff,0x00,DEP_BUFFER_SIZE_RECEIVE);

	//Send GPO command
	rspCode = VGL_Initiate_Process_GPO();

	//Contact occur
	if(rspCode == ECL_LV1_STOP_ICCARD)
		return 0xFF;

	//Timeout occur
	if((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
		return 0xFE;

	rspCode = VGL_Check_SW12(&VGL_rcvBuff[VGL_rcvLen-2]);
	
	if(rspCode == VGL_SW9000)
	{
		//Check GPO response
		if((VGL_rcvBuff[0] != 0x77) && (VGL_rcvBuff[0] != 0x80))
		{
			return FAIL;
		}
	
		rspCode = VGL_AIP_AFL_Process();
		if(rspCode == SUCCESS)
		{
			rspCode = VGL_Check_GPO_R_Data(VGL_rcvLen,VGL_rcvBuff);
			if(rspCode == SUCCESS)
			{
				rspCode = VGL_Initiate_Choose_Path_Process();	//check Both MSD-enabled and qVSDC-enabled
				if(rspCode == SUCCESS)
				{
					//20131224 choose SchemeID in here
					if(VGL_qVSDC_Path)
						SchemeID = 0x17;	//qVSDC
					else
						SchemeID = 0x18;	//MSD
					
					return SUCCESS;
				}
				else 
					return FAIL;
			}
			else
				return FAIL;
		}
		else		
			return FAIL;
	}
	else if(rspCode == VGL_SW6984)	//Try Another Interface
	{
		//20130702 ICTK, For msd only mode, when GPO return 6984, 6986 just terminate it
		if(glv_tag9F66.Value[0] & 0x20)		//is qVSDC support?
			return VGL_Try_Another;
		else										//MSD Only Mode
			return VGL_GPO_Terminate;
	}
	else if(rspCode == VGL_SW6985)	//Select Next Outcome
	{
		return VGL_Select_Next;
	}
	else if(rspCode == VGL_SW6986)	//Try Again
	{
		//20130702 ICTK, For msd only mode, when GPO return 6984, 6986 just terminate it
		//Display message, power down CL interface, and wait 1000ms ?1500ms
		if(glv_tag9F66.Value[0] & 0x20)		//is qVSDC support?
			return VGL_Try_Again;
		else										//MSD Only Mode
			return VGL_GPO_Terminate;
	}	
	else
	{
		return FAIL;
	}
	
}

void VGL_Start_Kernel(UINT lenFCI, UCHAR *datFCI)
{
	UINT	tempLEN=0,i=0;
	UCHAR	rspCode=0,temcnt=0;

	lenFCI=lenFCI;
	datFCI=datFCI;

	rspCode=VGL_Allocate_Buffer();
	if (rspCode == FAIL)
	{
DBG_Put_Text("malloc fail");
		return;
	}

	VGL_Offline_Approval = FALSE;

	VGL_Kernel_CVN17 = FALSE;	//Reset this flag
	
	if(((etp_Outcome.Start == ETP_OCP_Start_NA) && (etp_flgRestart == FALSE)) || 
		((etp_Outcome.Start == ETP_OCP_Start_B) && (etp_flgRestart == FALSE)) ||
		((etp_Outcome.Start == ETP_OCP_Start_C) && (etp_flgRestart == FALSE)))
	{		
		//reset the indicator
		VGL_Decline_Required_by_Reader = 0;
		VGL_Online_Required_by_Reader = 0;
		// reset the parameter
		memset(Online_Data,0x00,1024);
		Online_Data_Length = 0;

		//0621 reset CVM
		VGL_CVMOutcome = 0x00;
	
		rspCode = VGL_Initiate_App_Process();
		
		switch(rspCode)
		{
			case SUCCESS:

				UT_EMVCL_GetBERLEN(glv_tag94.Length,&temcnt,&tempLEN);
								
				if(tempLEN)		// if the Application File Locator was returned
				{
					rspCode = VGL_RAD_Process();
				}
	
				if(rspCode == SUCCESS)
				{				
					rspCode = VGL_RR_Complete();
														
					switch(rspCode)
					{
						case VGL_MSD_Process:
							VGL_RR_MSD_Online_Req();
							break;
							
						case VGL_qVSDC_Process:
							if(glv_tag9F66.Value[1]&0x80)
								VGL_Online_Required_by_Reader = TRUE;
							
							rspCode = VGL_Processing_Restriction();
							switch(rspCode)
							{
								case SUCCESS:
													
									if(	(VGL_Online_Required_by_Reader == 0)  	&& 
										(VGL_Decline_Required_by_Reader == 0) 	&& 
										(glv_tag9C.Value[0] != 0x20))
									{
										rspCode = VGL_EMV_OFDA();

										if(rspCode == VGL_OFDA_Try_Another)
										{
											VGL_FDDA_Try_Another_Interface();
											break;		
										}
									}
									
									if((VGL_Decline_Required_by_Reader == 0) && (glv_tag9C.Value[0] != 0x20))
										VGL_CVM();

									if(	(VGL_Online_Required_by_Reader == 1)	&&
										(VGL_Decline_Required_by_Reader == 0) 	&& 
										(glv_tag9C.Value[0] != 0x20))
									{
										//20140113 V5, if reader is offline only reader(byte 1 b 4), it should be offline decline
										if(glv_tag9F66.Value[0]&0x08)
										{
											VGL_Decline_Required_by_Reader = 1;
											VGL_Offline_Completion();
										}//20140113 V5, if reader is offline only reader(byte 1 b 4), it should be offline decline end
										else
										{
											//for layer 3
											L3_Response_Code = VAP_RIF_RC_DATA;

											//20140110 V2 For Double DIP Problem
											memcpy(VGL_LastTxnPAN,VGL_Print_PAN,VGL_Print_PAN[0]+1);
											VGL_LastTxnPANTime=OS_GET_SysTimerFreeCnt();
											//20140110 V2 For Double DIP Problem end
											
											VGL_Online_Process();
										}
									}
									else
									{
										//VCPS 2.1.3 Case CLQ.A.027.01
										if ((VGL_Online_Required_by_Reader == 1)	&&
											(VGL_Decline_Required_by_Reader == 0) 	&& 
											(glv_tag9C.Value[0] == 0x20))
										{
											UT_LED_Switch(IID_LED_GREEN,2);
											L3_Response_Code = VAP_RIF_RC_DATA;
											VGL_Online_Process();
										}
										else
										{
											if(!VGL_Decline_Required_by_Reader)
											{																				
												VGL_Offline_Approval = 1;
												//for layer 3
												L3_Response_Code = VAP_RIF_RC_DATA;
												
												//20140110 V2 For Double DIP Problem
												memcpy(VGL_LastTxnPAN,VGL_Print_PAN,VGL_Print_PAN[0]+1);
												VGL_LastTxnPANTime=OS_GET_SysTimerFreeCnt();
												//20140110 V2 For Double DIP Problem end											
											}
											
											VGL_Offline_Completion();
										}
									}

									break;
	
								case VGL_Switch_Interface:
									VGL_Try_Another_Interface();
									break;						
							}
							break;
							
						case FAIL:
							VGL_End_Application();
							break;
							
					}
				}	
				else if(rspCode == 0xFE)		//Read Record timeout
				{
					//2013062 ICTK
					//if time out, reset the data that receive from GPO Response, Read Record Response
					for(i=0;i<ECL_NUMBER_TAG;i++)
					{
						if(glv_tagTable[i].VISA_R & (VISA_R_GPO|VISA_R_READ_RECORD))
						{
							memset((UCHAR *)glv_addTable[i],0x00,3+((UINT)(glv_tagTable[i].VISA_Length)));
						}
					}

					//VCPS 2.1.3 Update - Set Timeout Outcome
					ETP_Set_DefaultOutcome(ETP_OUTCOME_Timeout);
					
					ECL_LV1_RESET();
					
					break;
				}
				else if(rspCode == 0xFF)				//20130626 Contact interrupt
				{
					//VCPS 2.1.3 Update - Disable LED
					UT_Set_LEDSignal(IID_LED_YELLOW, 0, 0);
					
					etp_Outcome.ocmMsgID = ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
					break;
				}
				else
				{
					VGL_End_Application();
				}
				break;
						
			case VGL_Try_Another:
				VGL_GPO_Try_Another_Interface();
				break;
	
			case VGL_Select_Next:
				VGL_Select_Next_OutCome();
				break;
	
			case VGL_Try_Again:
				VGL_Try_Again_OutCome();
				break;
	
			case FAIL:
				VGL_End_Application();
				break;	

			//timeout
			case 0xFE:

				//VCPS 2.1.3 Update - Set Timeout Outcome
				ETP_Set_DefaultOutcome(ETP_OUTCOME_Timeout);

				ECL_LV1_RESET();
				
				break;

			case 0xFF:	//Contact interrupt
				
				etp_Outcome.ocmMsgID = ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
				
				break;

			case VGL_GPO_Terminate:

				VGL_GPO_Terminate_OutCome();

				break;
				
		}

		//reset tag flag
		VGL_tag57	=0x00;	
		VGL_tag82	=0x00;	
		VGL_tag9F10	=0x00;	
		VGL_tag9F26	=0x00;	
		VGL_tag9F36	=0x00;

		//reset Cryptogram Type		
		VGL_AAC = 0;
		VGL_TC = 0;
		VGL_ARQC = 0;

		//reset the path
		VGL_MSD_Path = 0;
		VGL_qVSDC_Path = 0;

		//reset the indicator
		VGL_Decline_Required_by_Reader = 0;
		VGL_Online_Required_by_Reader = 0;
	
		// 20131204 reset this flag
		VGL_Kernel_CVN17 = FALSE;

		//20131205  buffer reset
		memset(VGL_rcvBuff,0x00,DEP_BUFFER_SIZE_RECEIVE);
		VGL_rcvLen = 0;

		memset(VGL_sendBuff,0x00,DEP_BUFFER_SIZE_SEND);
		VGL_sendLen = 0;
		//20131205 end
		
	}

	if((etp_Outcome.Start == ETP_OCP_Start_B )&& Opt_qVSDC_ISSUER_Update)
	{
		VGL_Issuer_Update();

		//20131211
		ETP_UI_Request(ETP_OCP_UIM_CardReadOKRemoveCard,00);		
		UT_LED_F_Off_S_On(IID_LED_YELLOW,IID_LED_GREEN,TRUE);
		UT_BUZ_Beep1();
	}

	if((etp_Outcome.Start == ETP_OCP_Start_D) && (etp_flgRestart == TRUE))
	{
		//0627 ICTK , reset the CVMOutcome for restart 
		VGL_CVMOutcome = 0x00;
		//0627 ICTK , Issuer Update, case CLQ.U.005
		
		if((glv_tag9F6C.Value[1]&0x40) && (Opt_qVSDC_ISSUER_Update) && (glv_tag9F66.Value[2]&0x80))	//20131120_PM change issuer update logic CLQ.U.005.01
			VGL_Online_Process();
		else
		{
			//20140115 V2, We add this flag to decide Issuer update Success or not
			etp_flgIsuUpdateErr = TRUE;
			//20140115 V2, We add this flag to decide Issuer update Success or not end
			VGL_Offline_Completion();
		}
	}
	
	VGL_Free_Buffer();
}

UCHAR VGL_Dynamic_Reader_Limit(PREDRLLIMITSET *ID)
{
	
	UCHAR *APID=0,lenOfL = 0,table1[ETP_PARA_NUMBER_PID][2],FullFlag = 0;
	UINT lenOfV,ReaderSet,CardAPIDIndex,MaxMatchReaderSet=0,MatchNum=0,MaxMatch=0;
	UCHAR tag9F5A[2]={0x9F,0x5A};
	
	memset(table1,0x00,sizeof(table1));		//store {Full match(2) or partial match(1) or not match(0)},{match Len}
	
	//Req5.1.2.1 Find 9F5A Tag	
//	APID = apk_EMVCL_FindTag(0x9F, 0x5A, etp_rcvData);
//	if (*APID != 0)		//Find 9F5A
	APID=UT_Find_Tag(tag9F5A, (etp_rcvLen-2), etp_rcvData);
	if (APID != NULLPTR)
	{
		//Check tag 9F5A length's length
		APID++;	//Point to L
		UT_EMVCL_GetBERLEN(APID,&lenOfL,&lenOfV);
		APID+=lenOfL;

		/*
		If the card Application Program ID has the same length and
		value as the reader Application Program ID (full match),
�h 		
		or the card Application Program ID begins with the
		entire reader Application Program ID (partial match)
		*/
		
		//Scan Reader Limit Set
		for(ReaderSet=0;ReaderSet<ETP_PARA_NUMBER_PID;ReaderSet++)		//Terminal Reader Set
		{
			MatchNum = 0;
			
			if(lenOfV >= glv_parDRLLimitSet[ReaderSet].AppProgIDLen)	//the APID from Card must bigger or equal than Reader's
			{
				for(CardAPIDIndex=0;CardAPIDIndex<glv_parDRLLimitSet[ReaderSet].AppProgIDLen;CardAPIDIndex++)	//depand on Reader APID Len
				{
					if(glv_parDRLLimitSet[ReaderSet].AppProgID[CardAPIDIndex] == APID[CardAPIDIndex])
					{
						MatchNum++;
						
						if(MatchNum == lenOfV)						//Full
						{
							table1[ReaderSet][0] = 2;				//Full match
							table1[ReaderSet][1] = MatchNum;		//match len
							FullFlag = TRUE;						//if we had a "Full Match", We will choose the Full match Reader set, regardless its length
						}
						else
						{
							table1[ReaderSet][0] = 1;				//Partial match
							table1[ReaderSet][1] = MatchNum;		//match len
						}
					}
					else
					{
						table1[ReaderSet][0] = 0;					//Not match
						table1[ReaderSet][1] = MatchNum;			//match len
						break;
					}
				}
			}
			else													//No match
			{
				table1[ReaderSet][0] = 0;							//Not match
				table1[ReaderSet][1] = 0;							//Match len = 0
			}
		}
	
		for(ReaderSet=0;ReaderSet<ETP_PARA_NUMBER_PID;ReaderSet++)
		{
			if(table1[ReaderSet][0] != 0)			//not no match
			{
				if(table1[ReaderSet][0] == 2)		//Full match
				{
					MaxMatch = table1[ReaderSet][1];
					MaxMatchReaderSet = ReaderSet;
					break;
				}
				else								//partial match
				{
					if(table1[ReaderSet][1] > MaxMatch)
					{
						MaxMatch = table1[ReaderSet][1];
						MaxMatchReaderSet = ReaderSet;
					}	
				}
			}
		}

		//finally, we use MaxMatch to determine 
		if(MaxMatch != 0)
		{
			memcpy(ID,&VGL_PREDRLLimitSet[MaxMatchReaderSet],PREDRLLIMITSET_LEN);
			return SUCCESS;
		}	
		else
			return FAIL;
	}
	else
		return FAIL;
}	

void VGL_DRL_Preprocess(void)
{
	UCHAR zeroset[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
	
	UINT i=0;
	UCHAR rspCode ;
	UCHAR tmp9F21[4]={0};
	UCHAR oneAmount[6]={0x00,0x00,0x00,0x00,0x01,0x00};
	UCHAR zroAmount[6]={0};

	for(i=0;i<ETP_PARA_NUMBER_PID;i++)
	{
		memcpy(glv_parDRLLimitSet[i].tertxnLimit,glv_par9F1B_VISA[VISA_Purchase_Mode],4);
		
		memset(&VGL_PREDRLLimitSet[i], 0x00, PREDRLLIMITSET_LEN);
		
		memcpy(VGL_PREDRLLimitSet[i].DRL_TTQ,glv_par9F66[2],ETP_PARA_SIZE_9F66);

		VGL_PREDRLLimitSet[i].DRL_TTQ[1] &= 0x3F;	//Reset TTQ B2b7 and B2b8 to "0"

		//Status Check
		if (glv_parDRLLimitSet[i].Combined[0] & 0x80) 
		{
			//Amount is A Single Unit of Currency
			if (memcmp(glv_tag9F02.Value, oneAmount, 6) == 0)
			{
				VGL_PREDRLLimitSet[i].DRL_TTQ[1] |= 0x80;			//TTQ B2b8 Online Cryptogram Required
			}
		}

		//Zero Amount
		if (memcmp(glv_tag9F02.Value, zroAmount, 6) == 0)			//Transaction Amount = 0
		{
			if(glv_parDRLLimitSet[i].Combined[0] & 0x40)			//Amount, Authorized of Zero Check
			{
				if((VGL_PREDRLLimitSet[i].DRL_TTQ[0] & 0x08)==0x00)	//not offline only reader
				{
					if (glv_parDRLLimitSet[i].Combined[0] & 0x20)	//Option 1	
					{
						//Option 1: Indicate Online Cryptogram Required
						VGL_PREDRLLimitSet[i].DRL_TTQ[1]|= 0x80;	//TTQ B2b8 Online Cryptogram Required
					}
					else											//Option 2
					{
						//Option 2: Contactless Application Not Allowed
						VGL_PREDRLLimitSet[i].DRL_CLAP_Not_Allowed = TRUE;
					}
				}
				else												//offline only Reader
				{
					VGL_PREDRLLimitSet[i].DRL_CLAP_Not_Allowed = TRUE;
				}
			}
		}

		//Reader Contactless Transaction Limit (RCTL) Check
		if(glv_parDRLLimitSet[i].Combined[0] & 0x10)
		{
			//Transaction Amount >= Reader Contactless Transaction Limit
			rspCode = UT_bcdcmp(glv_tag9F02.Value, glv_parDRLLimitSet[i].rdrtxnLimit, 6);
			if((rspCode == 1) || (rspCode == 0))
			{
				VGL_PREDRLLimitSet[i].DRL_CLAP_Not_Allowed = TRUE;
			}
		}

		//Reader CVM Required Limit Check
		if(glv_parDRLLimitSet[i].Combined[0] & 0x08)
		{
			//Transaction Amount >= Reader Contactless Floor Limit
			rspCode = UT_bcdcmp(glv_tag9F02.Value, glv_parDRLLimitSet[i].rdrcvmLimit, 6);
			if((rspCode == 1) || (rspCode == 0))
			{
				VGL_PREDRLLimitSet[i].DRL_TTQ[1] |= 0x40;		//TTQ B2b7, CVM required
			}
		}

		//Reader Contactless Floor Limit Check  or Terminal Floor Limit Check
		if(glv_parDRLLimitSet[i].Combined[0] & 0x04)			//Reader Floor Limit Flag
		{
			if (glv_parDRLLimitSet_Len[i].rdrFlrLimit_Len == 6)
			{
				//TA: Transaction Amount > Reader Contactless Floor Limit
				//Production: Transaction Amount >= Reader Contactless Floor Limit
				rspCode = UT_bcdcmp(glv_tag9F02.Value, glv_parDRLLimitSet[i].rdrFlrLimit, 6);
				if((rspCode == 1) || (rspCode == 0))
				{
					VGL_PREDRLLimitSet[i].DRL_TTQ[1] |= 0x80;		//TTQ B2b8, Online Cryptogram Required
				}
			}
			else
			{
				if(memcmp(glv_parDRLLimitSet[i].tertxnLimit,zeroset,4))
				{
					UT_bcd2hex(5,&glv_tag9F02.Value[1],tmp9F21);
				
					rspCode = UT_bcdcmp(tmp9F21, glv_parDRLLimitSet[i].tertxnLimit,4);
					if((rspCode == 1) || (rspCode == 0))
					{
						VGL_PREDRLLimitSet[i].DRL_TTQ[1] |= 0x80;		//TTQ B2b8, Online Cryptogram Required
					}
				}
			}
		}
	}
}



