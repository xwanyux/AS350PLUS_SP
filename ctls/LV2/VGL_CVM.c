#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ECL_Tag.h"
#include "Datastructure.h"
#include "Define.h"
#include "Function.h"
#include "VGL_Function.h"
#include "VAP_ReaderInterface_Define.h"	//20140110 add RC_Code for offline transaction
#ifndef _PLATFORM_AS210
#include "LCDTFTAPI.h"
#else
#include "xioapi.h"
#endif

//api_pcd_vap_Command.c
extern UCHAR L3_Response_Code;

//ETP_Entrypoint.c
extern OUTCOME etp_Outcome;
extern ETPCONFIG etp_etpConfig[ETP_NUMBER_COMBINATION];
extern UCHAR etp_flgRestart;

//VGL_Kernel.c
extern UCHAR VGL_Decline_Required_by_Reader;
extern UCHAR VGL_Online_Required_by_Reader;
extern UCHAR VGL_ARQC;
extern UCHAR VGL_CVMOutcome;

UCHAR VGL_qVSDC_Online_Data(void)
{
	//UINT i=0,tag_len;
	UCHAR qVSDC_Data[50] = {0x5A,0x5F,0x20,0x5F,0x24,0x5F,0x2A,0x5F,0x34,0x82,
							0x95,0x9A,0x9C,0x9F,0x10,0x9F,0x1A,0x9F,0x26,0x9F,
							0x36,0x9F,0x37,0x9F,0x5D,0x9F,0x6E,0x9F,0x7C,0x9F,
							0x27,0x57,0x9F,0x66,0x9F,0x02,0x9F,0x03,0x9F,0x39,
							0xDF,0x0E,0x9F,0x21,0x5F,0x28,0x9F,0x57};
	
	UCHAR rspCode = 0;

	rspCode = VGL_Chip_Data(48,qVSDC_Data);

	if(rspCode == FAIL)
	{
		UT_PutMsg(1,0,FONT0,17,(UCHAR *)"Online Data Error");
		UT_WaitKey();
	}
	
	return SUCCESS;
	
}

void VGL_Offline_Completion(void)
{
	UCHAR rspCode;
	
	//Requirements 5.9.1.1
	if((VGL_Online_Required_by_Reader == 0) && (VGL_Decline_Required_by_Reader == 0))
	{
		if(etp_flgRestart == FALSE)
		{
			//20140110 V2, add response Code is "VAP_RIF_RC_DATA"
			L3_Response_Code = VAP_RIF_RC_DATA;
			
			rspCode = VGL_qVSDC_Online_Data();

			if(rspCode == SUCCESS)					
			{
				if(VGL_CVMOutcome == ETP_OCP_CVM_OnlinePIN)
				{
					etp_Outcome.Start			= ETP_OCP_Start_NA;
					memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));	
					etp_Outcome.CVM				= VGL_CVMOutcome;
					etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;	//20140110 V1 changed, kernel do not display 	
					etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PleaseEnterYourPIN;	
					etp_Outcome.rstStatus		= ETP_OCP_UIS_CardReadSuccessfully;
					etp_Outcome.rqtRestart		= FALSE;
					etp_Outcome.datRecord		= TRUE;
					etp_Outcome.dscData			= TRUE;			//As defined in requirement 4.3.1.1, AOSA may be present in Discretionary Data.
					etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
					etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
					etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
					etp_Outcome.rmvTimeout		= 0;
				}
				else
				{
					etp_Outcome.Start			= ETP_OCP_Start_NA;
					memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));	
					etp_Outcome.CVM				= VGL_CVMOutcome;
					etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;	//20140110 V1 changed, kernel do not display 
					etp_Outcome.ocmMsgID		= ETP_OCP_UIM_Approved;	
					etp_Outcome.rstStatus		= ETP_OCP_UIS_CardReadSuccessfully;
					etp_Outcome.rqtRestart		= FALSE;
					etp_Outcome.datRecord		= TRUE;
					etp_Outcome.dscData			= TRUE;			//As defined in requirement 4.3.1.1, AOSA may be present in Discretionary Data.
					etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
					etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
					etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
					etp_Outcome.rmvTimeout		= 0;
				}
			}
			
		}
		else
		{
			etp_Outcome.Start			= ETP_OCP_Start_NA;
			memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));	
			etp_Outcome.CVM				= VGL_CVMOutcome;
			etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;	//20140110 V1 changed, kernel do not display 
			etp_Outcome.ocmMsgID		= ETP_OCP_UIM_Approved;	
			etp_Outcome.rstStatus		= ETP_OCP_UIS_CardReadSuccessfully;
			etp_Outcome.rqtRestart		= FALSE;
			etp_Outcome.datRecord		= TRUE;
			etp_Outcome.dscData			= TRUE;			//As defined in requirement 4.3.1.1, AOSA may be present in Discretionary Data.
			etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
			etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
			etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
			etp_Outcome.rmvTimeout		= 0;
		}

	}

	if(VGL_Decline_Required_by_Reader)
	{
		// 20140820 for offline decline transaction, do not set L3_Response_Code = VAP_RIF_RC_DATA
		/*
		if(L3_Response_Code != VAP_RIF_RC_DDA_AUTH_FAILURE)
		{
			L3_Response_Code = VAP_RIF_RC_DATA;	

			if(etp_flgRestart == FALSE)		
				VGL_qVSDC_Online_Data();	
		}
		*/
		
		etp_Outcome.Start			= ETP_OCP_Start_NA;
		memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));	
		etp_Outcome.CVM				= ETP_OCP_CVM_NoCVM;
		etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;
		etp_Outcome.ocmMsgID		= ETP_OCP_UIM_NotAuthorised;	
		etp_Outcome.ocmStatus		= 0x05;
		etp_Outcome.rqtRestart		= FALSE;
		etp_Outcome.datRecord		= FALSE;
		etp_Outcome.dscData			= TRUE;			//As defined in requirement 4.3.1.1, AOSA may be present in Discretionary Data.
		etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
		etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
		etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
		etp_Outcome.rmvTimeout		= 0;

	}
}

void VGL_Online_Process(void)
{	
	UCHAR rspCode;

	if((etp_Outcome.rspData[0] != 0x00) && (glv_tag9F66.Value[2] & 0x80) && (glv_tag9F6C.Value[1] & 0x40))	//it means restart
	{
		etp_Outcome.Start			= ETP_OCP_Start_B;	//Start from B
		memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));	
		etp_Outcome.rqtRestart		= TRUE;								//UI Request on Restart Present
		etp_Outcome.rstMsgID		= ETP_OCP_UIM_PresentCardAgain;		//Message ID "21"
		etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;		//field off request
	}
	else
	{	
		//first, we cat all require data and store in Reader Data
		rspCode = VGL_qVSDC_Online_Data();

		if(rspCode == SUCCESS)
		{	
			etp_Outcome.Start			= ETP_OCP_Start_NA;
			memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));	
			etp_Outcome.rqtRestart		= FALSE;		//UI Request on Restart Present
			etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
		}
	}
	
	etp_Outcome.CVM				= VGL_CVMOutcome;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;	//20140110 changed, kernel do not display
	etp_Outcome.ocmMsgID		= ETP_OCP_UIM_AuthorisingPleaseWait;
	etp_Outcome.rstStatus		= ETP_OCP_UIS_ReadyToRead;
	etp_Outcome.datRecord		= TRUE;
	etp_Outcome.dscData			= TRUE;		
	etp_Outcome.Receipt			= TRUE;
	etp_Outcome.rmvTimeout		= 0;
	
}
void VGL_CVM(void)
{
	UCHAR R_TTQ[4],C_CTQ[2];//,rspCode=0;
	UINT tmp_len;
	memcpy(R_TTQ,glv_tag9F66.Value,4);

	//VCPS 2.1.3 Update - Remove Consumer Devices CVM Support Check
//	if(R_TTQ[2] & 0x40)	//Reader CVM Support ?
//	{
		//Req 5.79 (CTQ Not Returned by Card)		
		UT_Get_TLVLengthOfV(glv_tag9F6C.Length,&tmp_len);
		
		if(tmp_len == 0)		//card return CTQ?
		{
			//0621 ICTK, if the amount over the CVM Limit, we process CVM
			if(R_TTQ[1] & 0x40)		//If the kernel requires a CVM,
			{
				while(1)
				{
					if(R_TTQ[0] & 0x02)			//Reader signature support
					{		
						VGL_CVMOutcome = ETP_OCP_CVM_ObtainSignature;
						break;
					}
					else if(R_TTQ[0] & 0x04)			//Reader online pin support
					{
						VGL_CVMOutcome = ETP_OCP_CVM_OnlinePIN;

						//20140113 VCPS2.1 update list V.2
						VGL_Online_Required_by_Reader = 1;
						//20140113 VCPS2.1 update list V.2 end
						break;
					}
					else 			//Consuner Device CVM
					{
						VGL_Decline_Required_by_Reader = 1;
						break;
					}
				}
			}
		}
		else
		{
			memcpy(C_CTQ,glv_tag9F6C.Value,tmp_len);
			
			while(1)
			{
				if((C_CTQ[0] & 0x80) && (R_TTQ[0] & 0x04))	//Online PIN Required by card (CTQ byte 1 bit 8 is 1b) and Online PIN supported by reader,
				{
					VGL_Online_Required_by_Reader = 1;
					VGL_CVMOutcome = ETP_OCP_CVM_OnlinePIN;
					break;
				}
				else if(((C_CTQ[0] & 0x80) == 0x00) || ((R_TTQ[0] & 0x04) == 0x00))	//(Online PIN not required or not supported)
				{
					if(C_CTQ[1] & 0x80)	//Consumer Device CVM Performed by card
					{
						UT_Get_TLVLengthOfV(glv_tag9F69.Length,&tmp_len);
						if(tmp_len != 0)	//Card Authentication Related Data was returned
						{
							//VCPS 2.1.3 Update No. 49
							if ((tmp_len >= 7) && (memcmp(&glv_tag9F69.Value[5],C_CTQ,2) == 0))
							{
								VGL_CVMOutcome = ETP_OCP_CVM_ConfirmationCodeVerified;
								break;
							}
							else
							{
								VGL_Decline_Required_by_Reader = 1;
								break;
							}
						}
						else
						{
							if(VGL_ARQC)//ARQC
							{
								//0515 ICTK test CVM
								VGL_Online_Required_by_Reader = 1;
								//0515 ICTK test CVM end
								VGL_CVMOutcome = ETP_OCP_CVM_ConfirmationCodeVerified;
								break;
							}
							else//not ARQC
							{
								VGL_Decline_Required_by_Reader = 1;
								break;
							}
						}
					}
					else//(((C_CTQ[0] & 0x80) == 0x00) && ((C_CTQ[1] & 0x80) == 0x00)) 
					{
						if((C_CTQ[0] & 0x40) && (R_TTQ[0] & 0x02))
						{
							VGL_CVMOutcome = ETP_OCP_CVM_ObtainSignature;
							break;
						}
						//0515 ICTK CVM test
						else //CVM not perform
						{	
							if(R_TTQ[1] & 0x40)
							{
								VGL_Decline_Required_by_Reader = 1;
								break;
							}
							else
							{
								//None of above, No CVM Perform
								break;
							}
						}
						//0515 ICTK CVM test end
					}
				}
			}
		}
//	}
}

