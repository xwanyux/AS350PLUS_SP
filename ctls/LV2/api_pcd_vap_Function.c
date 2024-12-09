#include <string.h>
#include "POSAPI.h"
#include "Define.h"
#include "ECL_Tag_Define.h"
#include "Function.h"
#include "Datastructure.h"
#include "Glv_ReaderConfPara.h"
#include "VAP_ReaderInterface_Define.h"

#ifdef _PLATFORM_AS350
#include "EMVAPI.h"
#endif

extern UCHAR Mifare_Rdr_Serial_Num[8];

extern UCHAR L3_Response_Code;
extern UCHAR api_Issuer_Update_Success_Flag_For_AOSA;
extern UCHAR api_Tag9F74_Data_Length;

extern UCHAR	etp_flgRestart;			//Reader Restart
extern UCHAR	etp_flgIsuUpdateErr;	//Issuer Update
extern OUTCOME 	etp_Outcome;			//Use outcome and restart flag to active kernel

UCHAR	Issuer_Script_Data[1024]={0};
UINT	Script_Data_Length = 0;
UCHAR	Issuer_Script_Result[81]={0};		// b81,  CNT+5*16, issuer script results (CNT=0..16)
UCHAR	*Issuer_Script_Result_Length = 0;
UCHAR	IAD[16]={0};
UCHAR	IAD_Len = 0;

void	api_pcd_JCB_Approval(void)
{
	ETP_UI_Request(ETP_OCP_UIM_Approved,ETP_OCP_UIS_CardReadSuccessfully);

	UT_WaitTime(200);
}

void	api_pcd_JCB_Decline(void)
{
	ETP_UI_Request(ETP_OCP_UIM_NotAuthorised,ETP_OCP_UIS_CardReadSuccessfully);

	UT_WaitTime(200);
}
//-------------------------------------------------------------------------------------------
//FUNCTION:Normal Purchase 
//Purchase Transaction type "00"
//Input : salAmount
//Output : rspLength, rspData
//Return : Success(Approval), Fail(Decline)
//-------------------------------------------------------------------------------------------
UCHAR api_pcd_qVSDC_Purchase(UCHAR *salAmount, UINT *rspLength, UCHAR *rspData)
{
	UCHAR idxNum=0;
	UCHAR lenOfL=0;
	UCHAR rtcBuffer[14]={0};
	UCHAR CashbackAmount[ECL_LENGTH_9F03] = {0x00,0x00,0x00,0x00,0x00,0x00};

	UCHAR tag5F2A[3+ECL_LENGTH_5F2A]	={0x5F,0x2A,0x02};
	UCHAR tag5F36[3+ECL_LENGTH_5F36]	={0x5F,0x36,0x01};
	UCHAR tag81[2+ECL_LENGTH_81]		={0x81,0x04};
	UCHAR tag9A[2+ECL_LENGTH_9A]		={0x9A,0x03};
	UCHAR tag9C[2+ECL_LENGTH_9C]		={0x9C,0x01};
	UCHAR tag9F00[3+ECL_LENGTH_9F00]	={0x9F,0x00,0x06};	//20140609 add as 9F02
	UCHAR tag9F01[3+ECL_LENGTH_9F01]	={0x9F,0x01,0x06};
	UCHAR tag9F02[3+ECL_LENGTH_9F02]	={0x9F,0x02,0x06};
	UCHAR tag9F03[3+ECL_LENGTH_9F03]	={0x9F,0x03,0x06};
	UCHAR tag9F15[3+ECL_LENGTH_9F15]	={0x9F,0x15,0x02};
	UCHAR tag9F1A[3+ECL_LENGTH_9F1A]	={0x9F,0x1A,0x02};
	UCHAR tag9F1B[3+ECL_LENGTH_9F1B]	={0x9F,0x1B,0x04};
	UCHAR tag9F1E[3+ECL_LENGTH_9F1E]	={0x9F,0x1E,0x08};
	UCHAR tag9F21[3+ECL_LENGTH_9F21]	={0x9F,0x21,0x03};
	UCHAR tag9F4E[5+ETP_PARA_SIZE_9F4E]	={0x9F,0x4E};		//Variable Length

#ifdef _PLATFORM_AS350
	//VCPS 2.1.3 Update - Add Contact Tags
	UCHAR tag9F33[3+ECL_LENGTH_9F33]	={0x9F,0x33,0x03,0x00,0x00,0x00};
	UCHAR tag9F40[3+ECL_LENGTH_9F40]	={0x9F,0x40,0x05,0x00,0x00,0x00,0x00,0x00};
	UCHAR tag9F41[3+ECL_LENGTH_9F41]	={0x9F,0x41,0x04,0x00,0x00,0x00,0x00};

	//qUICS
	UCHAR tag9F16[3+ECL_LENGTH_9F16]	={0x9F,0x16,0x0F,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x37,0x30,0x34,0x35,0x30,0x30,0x38,0x33};
	UCHAR tag9F35[3+ECL_LENGTH_9F35]	={0x9F,0x35,0x01,0x22};
#endif

	//Load Configure Parameter and active it, Purchase is type 0 
	memcpy(glv_parDF00[2],glv_parDF00_VISA[VISA_Purchase_Mode],ETP_PARA_SIZE_DF00);	// index 2 for Visa, and the para are for pre-process
	memcpy(glv_parDF01[2],glv_parDF01_VISA[VISA_Purchase_Mode],ETP_PARA_SIZE_DF01);
	memcpy(glv_parDF02[2],glv_parDF02_VISA[VISA_Purchase_Mode],ETP_PARA_SIZE_DF02);
	memcpy(glv_parDF06[2],glv_parDF06_VISA[VISA_Purchase_Mode],ETP_PARA_SIZE_DF06);
	memcpy(glv_par9F1B[2],glv_par9F1B_VISA[VISA_Purchase_Mode],ETP_PARA_SIZE_9F1B);

		
	ETP_Clear_TransactionData();
	 
	//20140113 check visa wave 2 directly
	//20140112 check WAVE2 support or not first
	//20140108  set glv_par_WAVE2_Enable enadle
	for(idxNum=0;idxNum<VAP_SCHEME_NUMBER;idxNum++)
	{
		if(	(glv_vap_Scheme[idxNum].ID == VAP_Scheme_Wave2) ||
			(glv_vap_Scheme[idxNum].ID == VAP_Scheme_JCBWave2))
		{
			if(glv_vap_Scheme[idxNum].Support == 0x01)
			{
				glv_par_WAVE2_Enable = TRUE;
				break;		//20140113 if one of "visa wave 2" and "JCB wave 2" support, turn glv_par_WAVE2_Enable on
			}
			else
				glv_par_WAVE2_Enable = FALSE;
		}
	}

	
	//20140609 Assign Amount, Auth (tag 9F00)
	memmove(&tag9F00[3],salAmount,6);
	ETP_Add_TransactionData(tag9F00);
	 
	//20130820 Assign Transaction type
	tag9C[2] = glv_par9C[0];
	ETP_Add_TransactionData(tag9C);

	//Assign Amount, Auth (tag 9F02)
	memmove(&tag9F02[3],salAmount,6);
	ETP_Add_TransactionData(tag9F02);

	//Assign Amount, Auth (tag 81)
	UT_bcd2hex(5,&salAmount[1],&tag81[2]);
	ETP_Add_TransactionData(tag81);

	//Transaction Time
	api_rtc_getdatetime(0, rtcBuffer);
 
	//Assign Transaction Time(9F21)
	UT_Compress(&tag9F21[3],&rtcBuffer[6],3);
	ETP_Add_TransactionData(tag9F21);

	//Assign Transaction Date(9A)	//turn ascii to expiry date (MMYY), ex : 31 33 30 33 31 35  =>  13 03 05
	UT_Compress(&tag9A[2],&rtcBuffer[0],3);
	ETP_Add_TransactionData(tag9A);

	//Add Terminal Transaction Floor Limit 9F1B
	memcpy(&tag9F1B[3],glv_par9F1B[2],4);
	ETP_Add_TransactionData(tag9F1B);

	//20130718
	//Add 5F2A Transaction Currency Code 
	memcpy(&tag5F2A[3],glv_par5F2A,2);
	ETP_Add_TransactionData(tag5F2A);

	//Add 9F1A Terminal Country Code
	memcpy(&tag9F1A[3],glv_par9F1A,2);
	ETP_Add_TransactionData(tag9F1A);

	//Assign Auth, Amount, Others(tag 9F03)
	memmove(&tag9F03[3],CashbackAmount,6);
	ETP_Add_TransactionData(tag9F03);

	//Add 9F15
	memmove(&tag9F15[3],glv_par9F15,2);
	ETP_Add_TransactionData(tag9F15);

	//Add 9F4E
	lenOfL=UT_Set_TagLength((UINT)glv_par9F4ELen, &tag9F4E[2]);
	memmove(&tag9F4E[2+lenOfL], glv_par9F4E, glv_par9F4ELen);
	ETP_Add_TransactionData(tag9F4E);

	//Add 5F36
	tag5F36[3] = glv_par5F36[0];
	ETP_Add_TransactionData(tag5F36);

	//Add 9F01
	memcpy(&tag9F01[3],glv_par9F01,6);
	ETP_Add_TransactionData(tag9F01);

	memcpy(&tag9F1E[3], Mifare_Rdr_Serial_Num, 8);
	ETP_Add_TransactionData(tag9F1E);
	
#ifdef _PLATFORM_AS350
	//VCPS 2.1.3 Update - Add Contact Tags
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_CAP, 3, &tag9F33[3]);
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_ADD_CAP, 5, &tag9F40[3]);
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_TX_SC, 4, &tag9F41[3]);
	ETP_Add_TransactionData(tag9F33);
	ETP_Add_TransactionData(tag9F40);
	ETP_Add_TransactionData(tag9F41);

	//qUICS
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_MID, 15, &tag9F16[3]);
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_TYPE, 1, &tag9F35[3]);
	ETP_Add_TransactionData(tag9F16);
	ETP_Add_TransactionData(tag9F35);
#endif


	//20140113 Opt_qVSDC_Key_Revocation and Exception file enabled/disabled
	Opt_qVSDC_Key_Revocation = TRUE;

	//B1b2: Exception file enabled/disabled
	if(glv_parDF06_VISA[VISA_Purchase_Mode][0] & 0x02)
		Opt_qVSDC_Ter_Except_File=TRUE;
	else
		Opt_qVSDC_Ter_Except_File=FALSE;
	//20140113 Opt_qVSDC_Key_Revocation and Exception file enabled/disabled end

	//ICTK, We should clean the Online Data to Avoid the transaction abort from entrypoint
	memset(Online_Data,0x00,1024);
	Online_Data_Length = 0;

	//0514 ICTK
	memset((UCHAR*)&etp_Outcome, 0, OUTCOME_LEN);


	etp_Outcome.Start = ETP_OCP_Start_NA;
 
	ETP_EntryPoint();
 
	if(Online_Data_Length != 0)
	{
		*rspLength = Online_Data_Length;
		memcpy(rspData,Online_Data,Online_Data_Length);
	}
	else
	{
		*rspLength = Online_Data_Length;
		memset(rspData,0x00,1024);
	}
 

	if(	(etp_Outcome.ocmMsgID == ETP_OCP_UIM_InsertSwipeOrTryAnotherCard) ||
		(etp_Outcome.ocmMsgID == ETP_OCP_UIM_PleaseInsertOrSwipeCard) ||
		(etp_Outcome.ocmMsgID == ETP_OCP_UIM_PleaseInsertCard))
		return ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
	else if((etp_Outcome.CVM == ETP_OCP_CVM_ObtainSignature) || (etp_Outcome.CVM == ETP_OCP_CVM_OnlinePIN))
		return etp_Outcome.CVM;
	else
		return TransactionFinish;
}
//-------------------------------------------------------------------------------------------
//FUNCTION:Purchase With Cashback
//Purchase with Cashback Transaction type  "00"
//Input : salAmount, CashbackAmount
//Output : rspLength, rspData
//Return : Success(Approval), Fail(Decline)
//-------------------------------------------------------------------------------------------
UCHAR api_pcd_qVSDC_Purchase_Cashback(UCHAR *salAmount, UCHAR *CashbackAmount ,UINT *rspLength, UCHAR *rspData)
{
	UCHAR lenOfL=0;
	UCHAR rtcBuffer[14]={0};

	UCHAR tag5F2A[3+ECL_LENGTH_5F2A]	={0x5F,0x2A,0x02};
	UCHAR tag5F36[3+ECL_LENGTH_5F36]	={0x5F,0x36,0x01};
	UCHAR tag81[2+ECL_LENGTH_81]		={0x81,0x04};
	UCHAR tag9A[2+ECL_LENGTH_9A]		={0x9A,0x03};
	UCHAR tag9C[2+ECL_LENGTH_9C]		={0x9C,0x01};
	UCHAR tag9F00[3+ECL_LENGTH_9F00]	={0x9F,0x00,0x06};	//20140609 add as 9F02
	UCHAR tag9F01[3+ECL_LENGTH_9F01]	={0x9F,0x01,0x06};
	UCHAR tag9F02[3+ECL_LENGTH_9F02]	={0x9F,0x02,0x06};
	UCHAR tag9F03[3+ECL_LENGTH_9F03]	={0x9F,0x03,0x06};
	UCHAR tag9F04[3+ECL_LENGTH_9F04]	={0x9F,0x04,0x04};
	UCHAR tag9F15[3+ECL_LENGTH_9F15]	={0x9F,0x15,0x02};
	UCHAR tag9F1A[3+ECL_LENGTH_9F1A]	={0x9F,0x1A,0x02};
	UCHAR tag9F1B[3+ECL_LENGTH_9F1B]	={0x9F,0x1B,0x04};
	UCHAR tag9F1E[3+ECL_LENGTH_9F1E]	={0x9F,0x1E,0x08};
	UCHAR tag9F21[3+ECL_LENGTH_9F21]	={0x9F,0x21,0x03};
	UCHAR tag9F4E[5+ETP_PARA_SIZE_9F4E]	={0x9F,0x4E};		//Variable Length
	
#ifdef _PLATFORM_AS350
	//VCPS 2.1.3 Update - Add Contact Tags
	UCHAR tag9F33[3+ECL_LENGTH_9F33]	={0x9F,0x33,0x03,0x00,0x00,0x00};
	UCHAR tag9F40[3+ECL_LENGTH_9F40]	={0x9F,0x40,0x05,0x00,0x00,0x00,0x00,0x00};
	UCHAR tag9F41[3+ECL_LENGTH_9F41]	={0x9F,0x41,0x04,0x00,0x00,0x00,0x00};

	//qUICS
	UCHAR tag9F16[3+ECL_LENGTH_9F16]	={0x9F,0x16,0x0F,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x37,0x30,0x34,0x35,0x30,0x30,0x38,0x33};
	UCHAR tag9F35[3+ECL_LENGTH_9F35]	={0x9F,0x35,0x01,0x22};
#endif

	//Load Configure Parameter and active it, cashback is type 2 
	memcpy(glv_parDF00[2],glv_parDF00_VISA[VISA_Cashback_Mode],ETP_PARA_SIZE_DF00);	//index 2 for Visa, and the para are for pre-process
	memcpy(glv_parDF01[2],glv_parDF01_VISA[VISA_Cashback_Mode],ETP_PARA_SIZE_DF01);
	memcpy(glv_parDF02[2],glv_parDF02_VISA[VISA_Cashback_Mode],ETP_PARA_SIZE_DF02);
	memcpy(glv_parDF06[2],glv_parDF06_VISA[VISA_Cashback_Mode],ETP_PARA_SIZE_DF06);
	memcpy(glv_par9F1B[2],glv_par9F1B_VISA[VISA_Cashback_Mode],ETP_PARA_SIZE_9F1B);

	
	UCHAR others[6]={0};
	ETP_Clear_TransactionData();
	ETP_Add_TransactionDataFromParameter();

	memcpy(others,CashbackAmount,6);
	UT_bcd_add_bcd( 6, salAmount, others );

	//20140609 Assign Amount, Auth (tag 9F00)
	memmove(&tag9F00[3],salAmount,6);
	ETP_Add_TransactionData(tag9F00);

	//Assign Transaction type
	tag9C[2] = 0x00;
	ETP_Add_TransactionData(tag9C);


	//Assign Amount, Auth (tag 9F02)
	memmove(&tag9F02[3],salAmount,6);
	ETP_Add_TransactionData(tag9F02);

	//Assign Auth, Amount, Others(tag 9F03)
	memmove(&tag9F03[3],CashbackAmount,6);
	ETP_Add_TransactionData(tag9F03);

	//Assign Amount, Auth (tag 81)
	UT_bcd2hex(5,&salAmount[1],&tag81[2]);
	ETP_Add_TransactionData(tag81);

	//Assign Amount, other (tag 9F04)
	UT_bcd2hex(5,&CashbackAmount[1],&tag9F04[3]);
	ETP_Add_TransactionData(tag81);


	//Assign Transaction Time(9F21)
	api_rtc_getdatetime(0, rtcBuffer);
	UT_Compress(&tag9F21[3],&rtcBuffer[6],3);
	ETP_Add_TransactionData(tag9F21);

	//Assign Transaction Date(9A)
	UT_Compress(&tag9A[2],&rtcBuffer[0],3);
	ETP_Add_TransactionData(tag9A);


	//Add Terminal Transaction Floor Limit 9F1B
	memcpy(&tag9F1B[3],glv_par9F1B[2],4);
	ETP_Add_TransactionData(tag9F1B);

	//2014011 the Transaction currency code, Terminal Country Codeshould be added by setting value
	//Add 5F2A Transaction Currency Code 
	memcpy(&tag5F2A[3],glv_par5F2A,2);
	ETP_Add_TransactionData(tag5F2A);

	//Add 9F1A Terminal Country Code
	memcpy(&tag9F1A[3],glv_par9F1A,2);
	ETP_Add_TransactionData(tag9F1A);

	//Add 9F15
	memmove(&tag9F15[3],glv_par9F15,2);
	ETP_Add_TransactionData(tag9F15);

	//Add 9F4E
	lenOfL=UT_Set_TagLength((UINT)glv_par9F4ELen, &tag9F4E[2]);
	memmove(&tag9F4E[2+lenOfL], glv_par9F4E, glv_par9F4ELen);
	ETP_Add_TransactionData(tag9F4E);
	
	//Add 5F36
	tag5F36[3] = glv_par5F36[0];
	ETP_Add_TransactionData(tag5F36);

	//Add 9F01
	memcpy(&tag9F01[3],glv_par9F01,6);
	ETP_Add_TransactionData(tag9F01);

	memcpy(&tag9F1E[3], Mifare_Rdr_Serial_Num, 8);
	ETP_Add_TransactionData(tag9F1E);


#ifdef _PLATFORM_AS350
	//VCPS 2.1.3 Update - Add Contact Tags
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_CAP, 3, &tag9F33[3]);
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_ADD_CAP, 5, &tag9F40[3]);
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_TX_SC, 4, &tag9F41[3]);
	ETP_Add_TransactionData(tag9F33);
	ETP_Add_TransactionData(tag9F40);
	ETP_Add_TransactionData(tag9F41);

	//qUICS
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_MID, 15, &tag9F16[3]);
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_TYPE, 1, &tag9F35[3]);
	ETP_Add_TransactionData(tag9F16);
	ETP_Add_TransactionData(tag9F35);
#endif

	//ICTK, We should clean the Online Data to Avoid the transaction abort from entrypoint
	memset(Online_Data,0x00,1024);
	Online_Data_Length = 0;
	
	etp_Outcome.Start = ETP_OCP_Start_NA;
	
	//turn Cashback_Transactions bits ON
	Opt_Cashback_Transactions = 1;


			
	ETP_EntryPoint();

	//turn Cashback_Transactions bits OFF
	Opt_Cashback_Transactions = 0;
			
	*rspLength = Online_Data_Length;
	memcpy(rspData,Online_Data,Online_Data_Length);	

	if(	(etp_Outcome.ocmMsgID == ETP_OCP_UIM_InsertSwipeOrTryAnotherCard) ||
		(etp_Outcome.ocmMsgID == ETP_OCP_UIM_PleaseInsertOrSwipeCard) ||
		(etp_Outcome.ocmMsgID == ETP_OCP_UIM_PleaseInsertCard))
		return ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
	else if((etp_Outcome.CVM == ETP_OCP_CVM_ObtainSignature) || (etp_Outcome.CVM == ETP_OCP_CVM_OnlinePIN))
		return etp_Outcome.CVM;
	else
		return TransactionFinish;
}
//-------------------------------------------------------------------------------------------
//FUNCTION:Cash Transaction
//Cash Transaction type "01"
//Input : salAmount
//Output : rspLength, rspData
//return : Success(Approval), Fail(Decline)
//-------------------------------------------------------------------------------------------
UCHAR api_pcd_qVSDC_Cash(UCHAR *salAmount,UINT *rspLength, UCHAR *rspData)
{
	UCHAR lenOfL=0;
	UCHAR CashbackAmount[ECL_LENGTH_9F03] = {0x00,0x00,0x00,0x00,0x00,0x00};
	UCHAR rtcBuffer[14]={0};

	UCHAR tag5F2A[3+ECL_LENGTH_5F2A]	={0x5F,0x2A,0x02};
	UCHAR tag5F36[3+ECL_LENGTH_5F36]	={0x5F,0x36,0x01};
	UCHAR tag81[2+ECL_LENGTH_81]		={0x81,0x04};
	UCHAR tag9A[2+ECL_LENGTH_9A]		={0x9A,0x03};
	UCHAR tag9C[2+ECL_LENGTH_9C]		={0x9C,0x01};
	UCHAR tag9F00[3+ECL_LENGTH_9F00]	={0x9F,0x00,0x06};	//20140609 add as 9F02
	UCHAR tag9F01[3+ECL_LENGTH_9F01]	={0x9F,0x01,0x06};
	UCHAR tag9F02[3+ECL_LENGTH_9F02]	={0x9F,0x02,0x06};
	UCHAR tag9F03[3+ECL_LENGTH_9F03]	={0x9F,0x03,0x06};
	UCHAR tag9F15[3+ECL_LENGTH_9F15]	={0x9F,0x15,0x02};
	UCHAR tag9F1A[3+ECL_LENGTH_9F1A]	={0x9F,0x1A,0x02};
	UCHAR tag9F1E[3+ECL_LENGTH_9F1E]	={0x9F,0x1E,0x08};
	UCHAR tag9F21[3+ECL_LENGTH_9F21]	={0x9F,0x21,0x03};
	UCHAR tag9F1B[3+ECL_LENGTH_9F1B]	={0x9F,0x1B,0x04};
	UCHAR tag9F4E[5+ETP_PARA_SIZE_9F4E]	={0x9F,0x4E};		//Variable Length
	
#ifdef _PLATFORM_AS350
	//VCPS 2.1.3 Update - Add Contact Tags
	UCHAR tag9F33[3+ECL_LENGTH_9F33]	={0x9F,0x33,0x03,0x00,0x00,0x00};
	UCHAR tag9F40[3+ECL_LENGTH_9F40]	={0x9F,0x40,0x05,0x00,0x00,0x00,0x00,0x00};
	UCHAR tag9F41[3+ECL_LENGTH_9F41]	={0x9F,0x41,0x04,0x00,0x00,0x00,0x00};

	//qUICS
	UCHAR tag9F16[3+ECL_LENGTH_9F16]	={0x9F,0x16,0x0F,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x37,0x30,0x34,0x35,0x30,0x30,0x38,0x33};
	UCHAR tag9F35[3+ECL_LENGTH_9F35]	={0x9F,0x35,0x01,0x22};
#endif

	//Load Configure Parameter and active it, cash is type 1 
	memcpy(glv_parDF00[2],glv_parDF00_VISA[VISA_Cash_Mode],ETP_PARA_SIZE_DF00);	//index 2 for Visa, and the para are for pre-process
	memcpy(glv_parDF01[2],glv_parDF01_VISA[VISA_Cash_Mode],ETP_PARA_SIZE_DF01);
	memcpy(glv_parDF02[2],glv_parDF02_VISA[VISA_Cash_Mode],ETP_PARA_SIZE_DF02);
	memcpy(glv_parDF06[2],glv_parDF06_VISA[VISA_Cash_Mode],ETP_PARA_SIZE_DF06);
	memcpy(glv_par9F1B[2],glv_par9F1B_VISA[VISA_Cash_Mode],ETP_PARA_SIZE_9F1B);
	
	ETP_Clear_TransactionData();
	ETP_Add_TransactionDataFromParameter();

	//Assign Auth, Amount
	memmove(&tag9F00[3],salAmount,6);
	ETP_Add_TransactionData(tag9F00);
	
	//Assign Transaction type
	tag9C[2] = 0x01;
	ETP_Add_TransactionData(tag9C);

	//Assign Auth, Amount
	memmove(&tag9F02[3],salAmount,6);
	ETP_Add_TransactionData(tag9F02);

	//Assign Amount, Auth (tag 81)
	UT_bcd2hex(5,&salAmount[1],&tag81[2]);
	ETP_Add_TransactionData(tag81);

	//Assign Transaction Time(9F21)
	api_rtc_getdatetime(0, rtcBuffer);
	UT_Compress(&tag9F21[3],&rtcBuffer[6],3);
	ETP_Add_TransactionData(tag9F21);

	//Assign Transaction Date(9A)
	UT_Compress(&tag9A[2],&rtcBuffer[0],3);
	ETP_Add_TransactionData(tag9A);

	//Add Terminal Transaction Floor Limit 9F1B
	memcpy(&tag9F1B[3],glv_par9F1B[2],4);
	ETP_Add_TransactionData(tag9F1B);

	//Assign Auth, Amount, Others(tag 9F03)
	memmove(&tag9F03[3],CashbackAmount,6);
	ETP_Add_TransactionData(tag9F03);

	//2014011 the Transaction currency code, Terminal Country Codeshould be added by setting value
	//Add 5F2A Transaction Currency Code 
	memcpy(&tag5F2A[3],glv_par5F2A,2);
	ETP_Add_TransactionData(tag5F2A);

	//Add 9F1A Terminal Country Code
	memcpy(&tag9F1A[3],glv_par9F1A,2);
	ETP_Add_TransactionData(tag9F1A);

	//Add 9F15
	memmove(&tag9F15[3],glv_par9F15,2);
	ETP_Add_TransactionData(tag9F15);

	//Add 9F4E
	lenOfL=UT_Set_TagLength((UINT)glv_par9F4ELen, &tag9F4E[2]);
	memmove(&tag9F4E[2+lenOfL], glv_par9F4E, glv_par9F4ELen);
	ETP_Add_TransactionData(tag9F4E);
	
	//Add 5F36
	tag5F36[3] = glv_par5F36[0];
	ETP_Add_TransactionData(tag5F36);

	//Add 9F01
	memcpy(&tag9F01[3],glv_par9F01,6);
	ETP_Add_TransactionData(tag9F01);

	memcpy(&tag9F1E[3], Mifare_Rdr_Serial_Num, 8);
	ETP_Add_TransactionData(tag9F1E);

#ifdef _PLATFORM_AS350
	//VCPS 2.1.3 Update - Add Contact Tags
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_CAP, 3, &tag9F33[3]);
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_ADD_CAP, 5, &tag9F40[3]);
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_TX_SC, 4, &tag9F41[3]);
	ETP_Add_TransactionData(tag9F33);
	ETP_Add_TransactionData(tag9F40);
	ETP_Add_TransactionData(tag9F41);

	//qUICS
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_MID, 15, &tag9F16[3]);
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_TYPE, 1, &tag9F35[3]);
	ETP_Add_TransactionData(tag9F16);
	ETP_Add_TransactionData(tag9F35);
#endif

	//ICTK, We should clean the Online Data to Avoid the transaction abort from entrypoint
	memset(Online_Data,0x00,1024);
	Online_Data_Length = 0;

	etp_Outcome.Start = ETP_OCP_Start_NA;

	//turn on
	Opt_Cash_Transactions = 1;
	//APP_main();
	ETP_EntryPoint();

	//turn off
	Opt_Cash_Transactions = 0;
	
	*rspLength = Online_Data_Length;
	memcpy(rspData,Online_Data,Online_Data_Length);	

	if(	(etp_Outcome.ocmMsgID == ETP_OCP_UIM_InsertSwipeOrTryAnotherCard) ||
		(etp_Outcome.ocmMsgID == ETP_OCP_UIM_PleaseInsertOrSwipeCard) ||
		(etp_Outcome.ocmMsgID == ETP_OCP_UIM_PleaseInsertCard))
		return ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
	else if((etp_Outcome.CVM == ETP_OCP_CVM_ObtainSignature) || (etp_Outcome.CVM == ETP_OCP_CVM_OnlinePIN))
		return etp_Outcome.CVM;
	else
		return TransactionFinish;
}

//-------------------------------------------------------------------------------------------
//FUNCTION:Refund Transaction
//Refund Transaction type "20"
//Input : salAmount
//Output : rspLength, rspData
//return : Success(Approval), Fail(Decline)
//-------------------------------------------------------------------------------------------
UCHAR api_pcd_qVSDC_Refund(UCHAR *salAmount,UINT *rspLength, UCHAR *rspData)
{
	UCHAR lenOfL=0;
	UCHAR CashbackAmount[ECL_LENGTH_9F03] = {0x00,0x00,0x00,0x00,0x00,0x00};
	UCHAR rtcBuffer[14]={0};

	UCHAR tag5F2A[3+ECL_LENGTH_5F2A]	={0x5F,0x2A,0x02};
	UCHAR tag5F36[3+ECL_LENGTH_5F36]	={0x5F,0x36,0x01};
	UCHAR tag81[2+ECL_LENGTH_81]		={0x81,0x04};
	UCHAR tag9A[2+ECL_LENGTH_9A]		={0x9A,0x03};
	UCHAR tag9C[2+ECL_LENGTH_9C]		={0x9C,0x01};
	UCHAR tag9F00[3+ECL_LENGTH_9F00]	={0x9F,0x00,0x06};	//20140609 add as 9F02
	UCHAR tag9F01[3+ECL_LENGTH_9F01]	={0x9F,0x01,0x06};
	UCHAR tag9F02[3+ECL_LENGTH_9F02]	={0x9F,0x02,0x06};
	UCHAR tag9F03[3+ECL_LENGTH_9F03]	={0x9F,0x03,0x06};
	UCHAR tag9F15[3+ECL_LENGTH_9F15]	={0x9F,0x15,0x02};
	UCHAR tag9F1A[3+ECL_LENGTH_9F1A]	={0x9F,0x1A,0x02};
	UCHAR tag9F1B[3+ECL_LENGTH_9F1B]	={0x9F,0x1B,0x04};
	UCHAR tag9F1E[3+ECL_LENGTH_9F1E]	={0x9F,0x1E,0x08};
	UCHAR tag9F21[3+ECL_LENGTH_9F21]	={0x9F,0x21,0x03};
	UCHAR tag9F4E[5+ETP_PARA_SIZE_9F4E]	={0x9F,0x4E};		//Variable Length

#ifdef _PLATFORM_AS350
	//VCPS 2.1.3 Update - Add Contact Tags
	UCHAR tag9F33[6]	={0x9F,0x33,0x03,0x00,0x00,0x00};
	UCHAR tag9F40[8]	={0x9F,0x40,0x05,0x00,0x00,0x00,0x00,0x00};
	UCHAR tag9F41[7]	={0x9F,0x41,0x04,0x00,0x00,0x00,0x00};

	//qUICS
	UCHAR tag9F16[18]	={0x9F,0x16,0x0F,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x37,0x30,0x34,0x35,0x30,0x30,0x38,0x33};
	UCHAR tag9F35[4]	={0x9F,0x35,0x01,0x22};
#endif	
	
	//Load Configure Parameter and active it, Purchase is type 0 
	memcpy(glv_parDF00[2],glv_parDF00_VISA[VISA_Purchase_Mode],ETP_PARA_SIZE_DF00);	//index  2 for Visa, and the para are for pre-process
	memcpy(glv_parDF01[2],glv_parDF01_VISA[VISA_Purchase_Mode],ETP_PARA_SIZE_DF01);
	memcpy(glv_parDF02[2],glv_parDF02_VISA[VISA_Purchase_Mode],ETP_PARA_SIZE_DF02);
	memcpy(glv_parDF06[2],glv_parDF06_VISA[VISA_Purchase_Mode],ETP_PARA_SIZE_DF06);
	memcpy(glv_par9F1B[2],glv_par9F1B_VISA[VISA_Purchase_Mode],ETP_PARA_SIZE_9F1B);
		
	ETP_Clear_TransactionData();
	ETP_Add_TransactionDataFromParameter();

	//Assign Auth, Amount
	memmove(&tag9F00[3],salAmount,6);
	ETP_Add_TransactionData(tag9F00);

	//Assign Transaction type
	tag9C[2] = 0x20;
	ETP_Add_TransactionData(tag9C);

	//Assign Amount, Auth (tag 9F02)
	memmove(&tag9F02[3],salAmount,6);
	ETP_Add_TransactionData(tag9F02);

	//Assign Auth, Amount, Others(tag 9F03)
	memmove(&tag9F03[3],CashbackAmount,6);
	ETP_Add_TransactionData(tag9F03);

	//Assign Amount, Auth (tag 81)
	UT_bcd2hex(5,&salAmount[1],&tag81[2]);
	ETP_Add_TransactionData(tag81);

	//Assign Transaction Time(9F21)
	api_rtc_getdatetime(0, rtcBuffer);
	UT_Compress(&tag9F21[3],&rtcBuffer[6],3);
	ETP_Add_TransactionData(tag9F21);

	//Assign Transaction Date(9A)
	UT_Compress(&tag9A[2],&rtcBuffer[0],3);
	ETP_Add_TransactionData(tag9A);
	
	//Add Terminal Transaction Floor Limit 9F1B
	memcpy(&tag9F1B[3],glv_par9F1B[2],4);
	ETP_Add_TransactionData(tag9F1B);

	//2014011 the Transaction currency code, Terminal Country Codeshould be added by setting value
	//Add 5F2A Transaction Currency Code 
	memcpy(&tag5F2A[3],glv_par5F2A,2);
	ETP_Add_TransactionData(tag5F2A);

	//Add 9F1A Terminal Country Code
	memcpy(&tag9F1A[3],glv_par9F1A,2);
	ETP_Add_TransactionData(tag9F1A);

	//Add 9F15
	memmove(&tag9F15[3],glv_par9F15,2);
	ETP_Add_TransactionData(tag9F15);

	//Add 9F4E
	lenOfL=UT_Set_TagLength((UINT)glv_par9F4ELen, &tag9F4E[2]);
	memmove(&tag9F4E[2+lenOfL], glv_par9F4E, glv_par9F4ELen);
	ETP_Add_TransactionData(tag9F4E);
	
	//Add 5F36
	tag5F36[3] = glv_par5F36[0];
	ETP_Add_TransactionData(tag5F36);

	//Add 9F01
	memcpy(&tag9F01[3],glv_par9F01,6);
	ETP_Add_TransactionData(tag9F01);

	memcpy(&tag9F1E[3], Mifare_Rdr_Serial_Num, 8);
	ETP_Add_TransactionData(tag9F1E);

#ifdef _PLATFORM_AS350
	//VCPS 2.1.3 Update - Add Contact Tags
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_CAP, 3, &tag9F33[3]);
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_ADD_CAP, 5, &tag9F40[3]);
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_TX_SC, 4, &tag9F41[3]);
	ETP_Add_TransactionData(tag9F33);
	ETP_Add_TransactionData(tag9F40);
	ETP_Add_TransactionData(tag9F41);

	//qUICS
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_MID, 15, &tag9F16[3]);
	api_emv_GetDataElement(DE_TERM, ADDR_TERM_TYPE, 1, &tag9F35[3]);
	ETP_Add_TransactionData(tag9F16);
	ETP_Add_TransactionData(tag9F35);
#endif

	etp_Outcome.Start = ETP_OCP_Start_NA;

	//ICTK, We should clean the Online Data to Avoid the transaction abort from entrypoint
	memset(Online_Data,0x00,1024);
	Online_Data_Length = 0;
	
	//APP_main();
	ETP_EntryPoint();
		
	*rspLength = Online_Data_Length;
	memcpy(rspData,Online_Data,Online_Data_Length);	

	if(etp_Outcome.ocmMsgID == ETP_OCP_UIM_InsertSwipeOrTryAnotherCard)
		return ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
	else if((etp_Outcome.CVM == ETP_OCP_CVM_ObtainSignature) || (etp_Outcome.CVM == ETP_OCP_CVM_OnlinePIN))
		return etp_Outcome.CVM;
	else
		return TransactionFinish;
}

//-------------------------------------------------------------------------------------------
//FUNCTION:Purchase With Cashback
//Purchase with Cashback Transaction type  "09"
//Input : salAmount, CashbackAmount
//Output : rspLength, rspData
//Return : Success(Approval), Fail(Decline)
//-------------------------------------------------------------------------------------------
UCHAR api_pcd_JCB_Purchase_Cashback(UCHAR *salAmount, UCHAR *CashbackAmount ,UINT *rspLength, UCHAR *rspData)
{
	UCHAR rtcBuffer[14]={0};

	UCHAR tag5F2A[3+ECL_LENGTH_5F2A]	={0x5F,0x2A,0x02};
	UCHAR tag5F36[3+ECL_LENGTH_5F36]	={0x5F,0x36,0x01};
	UCHAR tag81[2+ECL_LENGTH_81]		={0x81,0x04};
	UCHAR tag9C[2+ECL_LENGTH_9C]		={0x9C,0x01};
	UCHAR tag9F00[3+ECL_LENGTH_9F00]	={0x9F,0x00,0x06};	//20140609 add as 9F02
	UCHAR tag9F01[3+ECL_LENGTH_9F01]	={0x9F,0x01,0x06};
	UCHAR tag9F02[3+ECL_LENGTH_9F02]	={0x9F,0x02,0x06};
	UCHAR tag9F03[3+ECL_LENGTH_9F03]	={0x9F,0x03,0x06};
	UCHAR tag9F04[3+ECL_LENGTH_9F04]	={0x9F,0x04,0x04};
	UCHAR tag9F15[3+ECL_LENGTH_9F15]	={0x9F,0x15,0x02};
	UCHAR tag9F1A[3+ECL_LENGTH_9F1A]	={0x9F,0x1A,0x02};
	UCHAR tag9F1B[3+ECL_LENGTH_9F1B]	={0x9F,0x1B,0x04};
	UCHAR tag9F21[3+ECL_LENGTH_9F21]	={0x9F,0x21,0x03};
	UCHAR tag9F4E[5+ETP_PARA_SIZE_9F4E]	={0x9F,0x4E};		//Variable Length
	
	UCHAR others[6]={0};
	ETP_Clear_TransactionData();
	ETP_Add_TransactionDataFromParameter();

	//Assign Auth, Amount
	memmove(&tag9F00[3],salAmount,6);
	ETP_Add_TransactionData(tag9F00);

	memcpy(others,CashbackAmount,6);
	UT_bcd_add_bcd( 6, salAmount, others );

	//Assign Transaction type
	tag9C[2] = 0x09;
	ETP_Add_TransactionData(tag9C);

	//Assign Amount, Auth (tag 9F02)
	memmove(&tag9F02[3],salAmount,6);
	ETP_Add_TransactionData(tag9F02);

	//Assign Auth, Amount, Others(tag 9F03)
	memmove(&tag9F03[3],CashbackAmount,6);
	ETP_Add_TransactionData(tag9F03);

	//Assign Amount, Auth (tag 81)
	UT_bcd2hex(5,&salAmount[1],&tag81[2]);
	ETP_Add_TransactionData(tag81);

	//Assign Amount, other (tag 9F04)
	UT_bcd2hex(5,&CashbackAmount[1],&tag9F04[3]);
	ETP_Add_TransactionData(tag81);

	//Assign Transaction Time(9F21)
	api_rtc_getdatetime(0, rtcBuffer);
	UT_Compress(&tag9F21[3],&rtcBuffer[6],3);
	ETP_Add_TransactionData(tag9F21);

	//Add Terminal Transaction Floor Limit 9F1B
	memcpy(&tag9F1B[3],glv_par9F1B[2],4);
	ETP_Add_TransactionData(tag9F1B);

	//2014011 the Transaction currency code, Terminal Country Codeshould be added by setting value
	//Add 5F2A Transaction Currency Code 
	memcpy(&tag5F2A[3],glv_par5F2A,2);
	ETP_Add_TransactionData(tag5F2A);

	//Add 9F1A Terminal Country Code
	memcpy(&tag9F1A[3],glv_par9F1A,2);
	ETP_Add_TransactionData(tag9F1A);

	//Add 9F15
	memmove(&tag9F15[3],glv_par9F15,2);
	ETP_Add_TransactionData(tag9F15);

	//Add 9F4E
	tag9F4E[2] = glv_par9F4ELen;
	memmove(&tag9F4E[3],glv_par9F4E,glv_par9F4ELen);
	ETP_Add_TransactionData(tag9F4E);
	
	//Add 5F36
	tag5F36[3] = glv_par5F36[0];
	ETP_Add_TransactionData(tag5F36);

	//Add 9F01
	memcpy(&tag9F01[3],glv_par9F01,6);
	ETP_Add_TransactionData(tag9F01);

	//ICTK, We should clean the Online Data to Avoid the transaction abort from entrypoint
	memset(Online_Data,0x00,1024);
	Online_Data_Length = 0;
	
	etp_Outcome.Start = ETP_OCP_Start_NA;
			
	ETP_EntryPoint();
		
	*rspLength = Online_Data_Length;
	memcpy(rspData,Online_Data,Online_Data_Length);	

	if(	(etp_Outcome.ocmMsgID == ETP_OCP_UIM_InsertSwipeOrTryAnotherCard) ||
		(etp_Outcome.ocmMsgID == ETP_OCP_UIM_PleaseInsertOrSwipeCard) ||
		(etp_Outcome.ocmMsgID == ETP_OCP_UIM_PleaseInsertCard))
		return ETP_OCP_UIM_InsertSwipeOrTryAnotherCard;
	else if((etp_Outcome.CVM == ETP_OCP_CVM_ObtainSignature) || (etp_Outcome.CVM == ETP_OCP_CVM_OnlinePIN))
		return etp_Outcome.CVM;
	else
		return TransactionFinish;
}

UCHAR	api_pcd_vap_IAD(UINT *HostScriptLen,UCHAR *ScriptData,UCHAR *HostScriptResult)
{
	UCHAR *tag91_exist = 0;
	UCHAR tag91[1]={0x91};
	UINT i;

	//assign the entrypoint, and turn on the restart flag
	etp_Outcome.Start = ETP_OCP_Start_D;
	etp_flgRestart = TRUE;

	//0625 ICTK
	//etp_flgIsuUpdate = TRUE;

	//0516 ICTK , Issuer Update
	//assign the kernel flag
	Opt_qVSDC_ISSUER_Update = 1;
	//0516 ICTK , Issuer Update end

	for(i=0;i<(*HostScriptLen);i++)
	{
		if((ScriptData[i] == 0x71) || (ScriptData[i] == 0x72))
		{
			break;
		}
	}

//20131015
//	tag91_exist = apk_EMVCL_FindTag(0x91,0x00,ScriptData);
	tag91_exist=UT_Find_Tag(tag91, HostScriptLen[0], ScriptData);
	if(tag91_exist)
	{
		IAD_Len = *tag91_exist;			//minus tag 91 (1 byte) length (1 byte)
		memcpy(IAD,&tag91_exist[1],IAD_Len);
	}
	else
	{	
		IAD_Len = 0;			//minus tag 91 (1 byte) length (1 byte)
		memset(IAD,0x00,16);
	}
//20131015 end

	//copy Issuer Update Script and give it length ==> this is 2L-V Format
	Script_Data_Length = (*HostScriptLen) - i;
	Issuer_Script_Data[0] = Script_Data_Length & 0x00FF;
	Issuer_Script_Data[1] = (Script_Data_Length  & 0xFF00) >> 8;
	memcpy(&Issuer_Script_Data[2],&ScriptData[i],Script_Data_Length);

	//Copy to Outcome rspdata
	//2013 0623 ICTK, if host only response "IAD" without and Issuer script
	if(Issuer_Script_Data[0] == 0)
		memcpy(etp_Outcome.rspData,IAD,IAD_Len);
	else
		memcpy(etp_Outcome.rspData,Issuer_Script_Data,Issuer_Script_Data[0]);

	//UT_DumpHex(0,2,(UINT)Issuer_Script_Data[0],Issuer_Script_Data);

	ETP_EntryPoint();

	//20140115 V2 we add this flag in here if it really to process issue update successfully, we turn this flag on
	if(etp_flgIsuUpdateErr)
		api_Issuer_Update_Success_Flag_For_AOSA = FALSE;
	else
		api_Issuer_Update_Success_Flag_For_AOSA = TRUE;
	//20140115 V2, we add this flag in here if it really to process issue update successfully, we turn this flag on end
	
	//0516 ICTK , Issuer Update
	//Reset the flag
	Opt_qVSDC_ISSUER_Update = 0;
	//0516 ICTK , Issuer Update end


	//20140106 change the length
	memcpy(HostScriptResult,Issuer_Script_Result,Issuer_Script_Result[0]*5+1);	// 20140106 add 1 for result length

	//Reset those data
	IAD_Len = 0;
	memset(IAD,0x00,sizeof(IAD));
	Script_Data_Length = 0;
	memset(Issuer_Script_Data,0x00,sizeof(Issuer_Script_Data));
	//*Issuer_Script_Result_Length = 0;
	memset(Issuer_Script_Result,0,sizeof(Issuer_Script_Result));

	etp_flgRestart = FALSE;

	//0625 ICTK
	etp_flgIsuUpdateErr = FALSE;

	//20140304, Issuer Update "More Cards" & "No Card"
	if((L3_Response_Code == VAP_RIF_RC_NO_CARD) || (L3_Response_Code == VAP_RIF_RC_MORE_CARDS))
		return FAIL;
	else
		return SUCCESS;
	//20140304, Issuer Update "More Cards" & "No Card" end
	
	//return SUCCESS;
}

