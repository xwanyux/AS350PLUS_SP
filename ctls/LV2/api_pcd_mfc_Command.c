// #ifndef _build_DSS_
#include <string.h>
#include "POSAPI.h"
#include "Function.h"
#include "FLSAPI.h"
#include "Define.h"
#include "Glv_ReaderConfPara.h"
#include "MIFARE_Define.h"
#include "VAP_ReaderInterface_Define.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Function.h"
#include "ITR_Function.h"

#ifdef _PLATFORM_AS210
#include "XIOAPI.h"
#else
#include "LCDTFTAPI.h"
#endif

#ifdef _PLATFORM_AS210
const UCHAR	Mifare_Rdr_HW_ID[8] 		= {"AS210 HW"};
const UCHAR	Mifare_Firm_Name[20] 		= {"AS210SW"};
#endif

#ifdef _PLATFORM_AS350
const UCHAR	Mifare_Rdr_HW_ID[8] 		= {"AS350Lib"};

#ifdef _ACQUIRER_NCCC
const UCHAR	Mifare_Firm_Name[20] 		= {"AS350Lib_NCCC"};
#else
const UCHAR	Mifare_Firm_Name[20] 		= {"AS350Lib"};
#endif

#endif

#ifdef _PLATFORM_AS350_LITE
#ifdef _SCREEN_SIZE_128x64
const UCHAR	Mifare_Rdr_HW_ID[8] 		= {"AS350 UL"};
const UCHAR	Mifare_Firm_Name[20] 		= {"AS350UL"};
#else
// const UCHAR	Mifare_Rdr_HW_ID[8] 		= {"AS350 LT"};
// const UCHAR	Mifare_Firm_Name[20] 		= {"AS350_LITE SW"};
#endif

#ifdef _ACQUIRER_NCCC
const UCHAR	Mifare_Firm_PP_Ver[20] 		= {"NCCC_PP_R159"};
#endif

#ifdef _ACQUIRER_FISC
const UCHAR	Mifare_Firm_PP_Ver[20] 		= {"FISC_PP_R203"};
#endif

#ifdef _ACQUIRER_CTCB
const UCHAR	Mifare_Firm_PP_Ver[20] 		= {"CTCB_PP_R207"};
const UCHAR	Mifare_IPASS_Ver[20]		= {"IPASS 0.10D"};
#endif
#endif


      UCHAR	Mifare_Rdr_Serial_Num[8]	= {"12345678"};	//Serial_Num
const UCHAR	Mifare_Rdr_HW_Ver[4] 		= {"V1.0"};		//HW Ver
const UCHAR	Mifare_Rdr_Firm_ID[8] 		= {"POSAPI"};	//BIOS Name
const UCHAR	Mifare_Rdr_Firm_Ver[4] 		= {"110C"};		//BIOS ID

//AP1
const UCHAR	Mifare_Firm_Ver[20] 		= {"V9XX_18121201"};//General Firmware
const UCHAR	Mifare_Firm_Date[20] 		= {"181212"};		//Firmware Date Time
const UCHAR	Mifare_Firm_VISA_Ver[20] 	= {"3.1.3"};
const UCHAR	Mifare_Firm_MAS_Ver[20] 	= {"3.1.1"};
const UCHAR	Mifare_Firm_Mifare_Ver[20]	= {"V1.0"};

//AP2
UCHAR	Mifare_File_Size[4]={0x00};					//File Size

UCHAR	Mifare_Firm_Name_Update[20] = {0x00};		//Model Name
UCHAR	Mifare_Firm_Ver_Update[20]  = {0x00};		//General Firmware
UCHAR	Mifare_Firm_Date_Update[20] = {0x00};		//Date Time

//Burn File
ULONG	Mifare_Seq_Num = 1;
ULONG	Mifare_Mem_Offset = 0;
ULONG 	Mifare_Total_File_Size = 0;

UCHAR	Mifare_File_Download_Flag = 0;
ULONG	Mifare_File_Timer1 = 0, Mifare_File_Timer2 = 0;

//Blink LED Timeout
ULONG	Mifare_Blink_LED_Timeout[4]={0};
ULONG	Mifare_Blink_LED_StartTime[4]={0};

//DHN of AUX Interrupt
UCHAR	mifare_dhnAUXInterrupt=0;

//DHN of MSR
UCHAR	mifare_dhnMSR=apiOutOfService;

//DHN of KBD
UCHAR	mifare_dhnKBD=apiOutOfService;

//Buffer for 2DR
UCHAR	mifare_2DR_rcvBuffer[MIFARE_2DR_BUFFER];
UINT	mifare_2DR_rcvLength=0;

extern UCHAR Utils_Global_palette[3];

extern ULONG OS_GET_SysTimerFreeCnt( void );

extern UCHAR VAP_RIF_Check_StopPolling(void);
extern UCHAR VAP_RIF_Check_StopRemoval(void);
extern UCHAR VAP_RIF_Check_CloseBCR(void);

// Add by Wayne to remove compiler warning 2020/08/21
// It's also can give read which function is include outside
// <------------
extern UCHAR	api_dss2_apid( void );
extern UCHAR	api_dss2_burn( ULONG apid );
extern UCHAR	api_dss2_run( UCHAR apid );
extern UCHAR	api_dss2_file( ULONG offset, ULONG length, UCHAR *data );
extern UCHAR 	api_2DR_StartScan(void);
extern UCHAR 	api_2DR_ReceiveData(UINT *optLen, UCHAR *optData);
extern UCHAR 	api_2DR_StopScan(void);
extern UCHAR	api_sys_backlight( UCHAR device, ULONG duration );
// <-------------


#ifdef _PLATFORM_AS350_LITE
#ifdef _ACQUIRER_NCCC
extern void		NCCC_Lite_ShowInfo(UCHAR mode, UCHAR * optData);
extern void		NCCC_Lite_ShowError(UCHAR mode);
#endif
#endif


UCHAR Mifare_Check_Interrupt(void)
{
	if (ITR_Check_Cancel()		== TRUE)		return 0xE0;
	if (ITR_Check_ICC()			== TRUE)		return 0xE1;
	if (ITR_Check_MagStripe()	== TRUE)		return 0xE2;

	if (VAP_RIF_Check_StopPolling() == TRUE)	return 0xD0;
	
	return FALSE;
}

UCHAR Mifare_Check_CloseBCR(void)
{
	if (VAP_RIF_Check_CloseBCR() == TRUE)		return TRUE;
	
	return FALSE;
}

UCHAR Mifare_Polling(UINT iptTimeout)
{
	UCHAR	rspCode=0xFF;
	UCHAR	rspChkCollision=0;
	ULONG	tick1, tick2;

	//Timer Setting
	tick1=OS_GET_SysTimerFreeCnt();

	//Reset PCD EMV Parameter
	ECL_LV1_ResetPCDParameter();

	//Reset Interrupt Flags
	ITR_Reset_Flags();
#ifdef PCD_PLATFORM_CLRC663
	//Field On
	rspCode=ECL_LV1_FIELD_ON_EMV();
#else
	rspCode=ECL_LV1_FIELD_ON();
#endif
	UT_WaitTime(5);
	
	do {
		//Type A Polling
		rspCode=ECL_LV1_POLLING_TypeA();
		if (rspCode == ECL_LV1_SUCCESS)
		{
			rspCode=SUCCESS;
			break;
		}
		
		rspChkCollision=ECL_LV1_POLLING_CheckCollision();
		if (rspChkCollision == TRUE)
		{
			rspCode=0xF3;
			break;
		}
		
		//Type B Polling
		rspCode=ECL_LV1_POLLING_TypeB();
		if (rspCode == ECL_LV1_SUCCESS)
		{
			rspCode=SUCCESS;
			break;
		}

		rspChkCollision=ECL_LV1_POLLING_CheckCollision();
		if (rspChkCollision == TRUE)
		{
			rspCode=0xF3;
			break;
		}

		rspCode=Mifare_Check_Interrupt();
		if (rspCode != FALSE)
		{
			break;
		}

		//Get Timer Value
		tick2=OS_GET_SysTimerFreeCnt();
	} while((tick2 - tick1) < iptTimeout);
	
	return rspCode;
}


UCHAR Mifare_Removal(UINT iptTimeout)
{
	UCHAR	rspCode=0;
	UCHAR	rspStop=0;
	UCHAR	rspStatus=FALSE;
	UCHAR	cntNoRsp=0;	//No Response Counter
	ULONG	tick1, tick2;
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[256];
	
	//Timer Setting
	tick1=OS_GET_SysTimerFreeCnt();
	
	ECL_LV1_RESET();

	UT_Wait(5100);

	do
	{
		rspCode=ECL_LV1_WUPA(&rcvLen, rcvBuffer);
		if (rspCode == ECL_LV1_TIMEOUT_ISO)
		{
			rspCode=ECL_LV1_WUPB(&rcvLen, rcvBuffer);
			if (rspCode == ECL_LV1_TIMEOUT_ISO)
			{
				cntNoRsp++;
			}
			else
			{
				UT_Wait(5100);

				cntNoRsp=0;
			}
		}
		else
		{
			ECL_LV1_HLTA();

			UT_Wait(5100);

			cntNoRsp=0;
		}

		if (cntNoRsp == 3)
		{
			rspStatus=TRUE;
			break;
		}

		rspStop=VAP_RIF_Check_StopRemoval();
		if (rspStop == TRUE)
		{
			rspStatus=2;
			break;
		}

		//Get Timer Value
		tick2=OS_GET_SysTimerFreeCnt();
	} while ((tick2 - tick1) < iptTimeout);

	return rspStatus;
}


void Mifare_Check_BlinkLEDTimeout(void)
{
	UCHAR i;
	ULONG tmrTotal;

	for (i=0; i < 4; i++)
	{
		if (Mifare_Blink_LED_Timeout[i] != 0)
		{
			tmrTotal=OS_GET_SysTimerFreeCnt()-Mifare_Blink_LED_StartTime[i];
			if (tmrTotal > Mifare_Blink_LED_Timeout[i])
			{
				Mifare_Blink_LED_Timeout[i]=0;
				UT_Set_LEDSignal(2+i, 0, 0);	//LED ID starts from 2
			}
		}
	}
}

UCHAR Mifare_Check_ParityLength(UINT iptLen)
{
	UINT	cntByte=0;
	UCHAR	cntBits=0;
	UINT	estByte=0;

	if (iptLen < 2)	//At Least 2 Bytes
	{
		return FAIL;
	}

	cntByte=iptLen*8/(8+1);	//8 bits + 1 Parity
	cntBits=iptLen*8%(8+1);	//8 bits + 1 Parity

	if (cntBits == 8)	//Useless Byte
	{
		return FAIL;
	}

	estByte=cntByte+(cntByte/8)+((cntBits == 0)?(0):(1));

	if (estByte != iptLen)	//Estimate Byte Number not Match
	{
		return FAIL;
	}

	return SUCCESS;
}

UCHAR Mifare_Burn_File(void)
{
	UCHAR APP_ID = 0;
	UCHAR rspCode = 0;

	//debug message
	UT_ClearRow(7,1,FONT0);
	UT_PutStr(7,0,FONT0,7,(UCHAR *)"Burning");

//Get APP ID
	APP_ID = api_dss2_apid();

//Burn
	if(APP_ID == 0)
		rspCode = api_dss2_burn(1);
	else
		rspCode = api_dss2_burn(0);

//Reboot
	if(rspCode == apiOK)
	{
		//debug message		
		UT_PutStr(7,8,FONT0,4,(UCHAR *)"PASS");
		
		if(APP_ID == 0)
			rspCode = api_dss2_run(1);
		else
			rspCode = api_dss2_run(0);

		if(rspCode != apiOK)
			return FAIL;
	}	
	else
	{
		// return fail 
		UT_PutStr(7,8,FONT0,4,(UCHAR *)"FAIL");

		UT_BUZ_Beep1();
		UT_WaitTime(10);
		UT_BUZ_Beep1();
		UT_WaitTime(10);
		UT_BUZ_Beep1();

		return FAIL;
	}

	return FAIL;
	
}

void Mifare_TransparentAPDU(UCHAR iptType, UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=Mifare_RC_Fail;
	UCHAR rspDEP=0;
	UCHAR bupFlag=0;
	UINT  msgLen=0;
	UINT  sndLen=0;
	UINT  rcvLen=0;
	ULONG tmr10ms=0;

	if ((iptType != 'A') && (iptType != 'B'))
	{
		return;
	}

	//Back up Flag
	ECL_LV1_GetCardType(&bupFlag);
	ECL_LV1_SetCardType(iptType);
		
	sndLen=iptMsgLength[0]*256+iptMsgLength[1]-1;	//Exclude Timeout
	tmr10ms=iptData[sndLen]*100;
	rspDEP=ECL_LV1_DEP(sndLen, iptData, &rcvLen, &optData[3], tmr10ms);

	switch (rspDEP)
	{
		case ECL_LV1_SUCCESS:		rspCode=Mifare_RC_Success;		break;
		case ECL_LV1_TIMEOUT_ISO:
		case ECL_LV1_TIMEOUT_USER:	rspCode=Mifare_RC_No_Response;	break;
		default:					rspCode=Mifare_RC_Fail;			break;
	}

	//Restore
	ECL_LV1_SetCardType(bupFlag);
	
	msgLen=1+2+rcvLen;	//Response Code(1)+Length(2)+rcvLen
	optMsgLength[0]=(msgLen & 0xFF00) >> 8;
	optMsgLength[1]=(msgLen & 0x00FF);

	optData[0]=rspCode;
	optData[1]=(rcvLen & 0xFF00) >> 8;
	optData[2]=(rcvLen & 0x00FF);
}


void api_pcd_mfc_con_RFOff(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	//Avoid Warning
	iptMsgLength[0] = iptMsgLength[0];
	iptData[0] = iptData[0];

	ECL_LV1_FIELD_OFF();

	//Mifare Response Format
	optData[0] = Mifare_RC_Success;
	optData[1] = 0;
	optData[2] = 0;
	
	optMsgLength[0] = 0;
	optMsgLength[1] = 1 + 2;	//Status + 2 bytes Length
}


void api_pcd_mfc_con_RFOn(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	//Avoid Warning
	iptMsgLength[0] = iptMsgLength[0];
	iptData[0] = iptData[0];

#ifdef PCD_PLATFORM_CLRC663
	ECL_LV1_FIELD_ON_EMV();
#else
	ECL_LV1_FIELD_ON();
#endif

	//Mifare Response Format
	optData[0] = Mifare_RC_Success;
	optData[1] = 0;
	optData[2] = 0;
	
	optMsgLength[0] = 0;
	optMsgLength[1] = 1 + 2;	//Status + 2 bytes Length
}

void api_pcd_mfc_con_REQA(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=0;

	UINT OutLen = 0;
	UCHAR OutATQA[2] = {0};

	//Avoid Warning
	iptMsgLength[0]= iptMsgLength[0];
	iptData[0]= iptData[0];

	//Function start
	rspCode = ECL_LV1_REQA(&OutLen,OutATQA);

	if(rspCode == SUCCESS)
	{
		//Mifare Response Format
		optData[0] = Mifare_RC_Success;
		optData[1] = (OutLen & 0xFF00)>>8;
		optData[2] = (OutLen & 0x00FF);
		memcpy(&optData[3],OutATQA,OutLen);

		optMsgLength[0]=((1 + 2 + OutLen) & 0xFF00) >> 8;	//Status + 2 bytes Length
		optMsgLength[1]=(1 + 2 + OutLen) & 0x00FF;			
	}
	else
	{
		//Mifare Response Format
		optData[0] = Mifare_RC_Fail;
		optData[1] = 0;
		optData[2] = 0;

		optMsgLength[0]=0;
		optMsgLength[1]=1 + 2;					//Status + 2 bytes Length
	}
	
}


void api_pcd_mfc_con_WUPA(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=0;

	UINT OutLen = 0;
	UCHAR OutWUPA[2] = {0};

	//Avoid Warning
	iptMsgLength[0]= iptMsgLength[0];
	iptData[0]= iptData[0];

	//Function start
	rspCode = ECL_LV1_WUPA(&OutLen,OutWUPA);

	if(rspCode == SUCCESS)
	{
		//Mifare Response Format
		optData[0] = Mifare_RC_Success;
		optData[1] = (OutLen & 0xFF00)>>8;
		optData[2] = (OutLen & 0x00FF);
		memcpy(&optData[3],OutWUPA,OutLen);

		optMsgLength[0]=((1 + 2 + OutLen) & 0xFF00) >> 8;	//Status + 2 bytes Length
		optMsgLength[1]=(1 + 2 + OutLen) & 0x00FF;			
	}
	else
	{
		//Mifare Response Format
		optData[0] = Mifare_RC_Fail;
		optData[1] = 0;
		optData[2] = 0;

		optMsgLength[0]=0;
		optMsgLength[1]=1 + 2;					//Status + 2 bytes Length
	}
	
}

void api_pcd_mfc_con_AntiCol(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode = 0;

	UINT IptLen = 0;
	UCHAR IptData[1024];
	
	UINT OutLen = 0;
	UCHAR OutUID[10] = {0};

	IptLen = iptMsgLength[0] * 256 + iptMsgLength[1];

	if(IptLen)
		memcpy(IptData,iptData,IptLen);

	switch(IptData[0])
	{
		case 1:		IptData[0] = 0x93;						break;
		case 2:		IptData[0] = 0x95;						break;
		case 3:		IptData[0] = 0x97;						break;
		default:	optData[0] = Mifare_RC_Invalid_Input;	break;
	}

	if (optData[0] != Mifare_RC_Invalid_Input)
	{		
		rspCode = ECL_LV1_ANTICOLLISION(IptData[0],&OutLen,OutUID);
		if(rspCode == SUCCESS)
		{
			OutLen = 4;
		
			//Mifare Response Format
			optData[0] = Mifare_RC_Success;
			optData[1] = ((OutLen) & 0xFF00)>>8;
			optData[2] = (OutLen & 0x00FF);
			memcpy(&optData[3],OutUID,OutLen);

			optMsgLength[0]=((1 + 2 + OutLen) & 0xFF00)>>8;			//Status + 2 bytes Length
			optMsgLength[1]=((1 + 2 + OutLen) & 0x00FF);
		}
		else
		{
			optData[0] = Mifare_RC_Fail;
		}
	}

	if (optData[0] != Mifare_RC_Success)
	{
		//Mifare Response Format
		optData[1] = 0;
		optData[2] = 0;
		
		optMsgLength[0]=0;
		optMsgLength[1]=1 + 2;	//Status + 2 bytes Length
	}
}


void api_pcd_mfc_con_Select(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode = 0;

	UINT IptLen = 0;
	UCHAR IptData[1024];
	
	UINT OutLen = 0;
	UCHAR OutSAK[10] = {0};

	IptLen = iptMsgLength[0] * 256 + iptMsgLength[1];

	optData[0] = Mifare_RC_Fail;
	
	if (IptLen == 5)
	{
		memcpy(IptData, iptData, IptLen);
		
		switch(IptData[0])
		{
			case 1:		IptData[0] = 0x93;						break;
			case 2:		IptData[0] = 0x95;						break;
			case 3:		IptData[0] = 0x97;						break;
			default:	optData[0] = Mifare_RC_Invalid_Input;	break;
		}

		if (optData[0] != Mifare_RC_Invalid_Input)
		{
			rspCode=ECL_LV1_SELECT(IptData[0], &IptData[1], &OutLen, OutSAK);
			if(rspCode == SUCCESS)
			{
				OutLen = 1;
			
				//Mifare Response Format
				optData[0] = Mifare_RC_Success;
				optData[1] = ((OutLen) & 0xFF00)>>8;
				optData[2] = (OutLen & 0x00FF);
				memcpy(&optData[3], OutSAK, OutLen);

				optMsgLength[0]=((1 + 2 + OutLen) & 0xFF00)>>8;
				optMsgLength[1]=((1 + 2 + OutLen) & 0x00FF);
			}
		}
	}

	if (optData[0] != Mifare_RC_Success)
	{
		optData[1] = 0;
		optData[2] = 0;

		optMsgLength[0]= 0;
		optMsgLength[1]= 1 + 2;	//Status + 2 bytes Length
	}
}


void api_pcd_mfc_con_WUPB(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=0;

	UINT  optLen = 0;
	UCHAR optATQB[64] = {0};

	//Avoid Warning
	iptMsgLength[0]= iptMsgLength[0];
	iptData[0]= iptData[0];

	//Function start
	rspCode=ECL_LV1_WUPB(&optLen, optATQB);
	if(rspCode == SUCCESS)
	{
		//Mifare Response Format
		optData[0] = Mifare_RC_Success;
		optData[1] = (optLen & 0xFF00)>>8;
		optData[2] = (optLen & 0x00FF);
		memcpy(&optData[3], optATQB, optLen);

		optMsgLength[0]=((1 + 2 + optLen) & 0xFF00) >> 8;	//Status + 2 bytes Length
		optMsgLength[1]=(1 + 2 + optLen) & 0x00FF;			
	}
	else
	{
		//Mifare Response Format
		optData[0] = Mifare_RC_Fail;
		optData[1] = 0;
		optData[2] = 0;

		optMsgLength[0]=0;
		optMsgLength[1]=1 + 2;					//Status + 2 bytes Length
	}
	
}

void api_pcd_mfc_con_Authenticate(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=0;

	UINT IptLen = 0;	
	UINT OutLen = 0;

	IptLen = iptMsgLength[0] * 256 + iptMsgLength[1];

	optData[0]=Mifare_RC_Fail;
	
	if (IptLen == 12)
	{
		if ((iptData[0] == 0x60) || (iptData[0] == 0x61))
		{
			rspCode=ECL_LV1_LOADKEY(&iptData[6]);
			rspCode=ECL_LV1_AUTHENTICATION(iptData[0], iptData[1], &iptData[2]);
			if(rspCode == SUCCESS)
			{
				OutLen = 0;
			
				//Mifare Response Format
				optData[0] = Mifare_RC_Success;
				optData[1] = ((OutLen) & 0xFF00)>>8;
				optData[2] = (OutLen & 0x00FF);

				optMsgLength[0]=((1 + 2 + OutLen) & 0xFF00)>>8;
				optMsgLength[1]=((1 + 2 + OutLen) & 0x00FF);
			}
		}
		else
		{
			optData[0] = Mifare_RC_Invalid_Input;
		}
	}

	if (optData[0] != Mifare_RC_Success)
	{
		optData[1] = 0;
		optData[2] = 0;

		optMsgLength[0]= 0;
		optMsgLength[1]= 1 + 2;	//Status + 2 bytes Length
	}
}

void api_pcd_mfc_con_Read(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=0;

	UINT IptLen = 0;	
	UINT OutLen = 0;

	IptLen = iptMsgLength[0] * 256 + iptMsgLength[1];

	optData[0]=Mifare_RC_Fail;
	
	if (IptLen == 1)
	{
		rspCode=ECL_LV1_READ(iptData[0], &optData[3]);
		if(rspCode == SUCCESS)
		{
			OutLen = 16;
		
			//Mifare Response Format
			optData[0] = Mifare_RC_Success;
			optData[1] = ((OutLen) & 0xFF00)>>8;
			optData[2] = (OutLen & 0x00FF);

			optMsgLength[0]=((1 + 2 + OutLen) & 0xFF00)>>8;
			optMsgLength[1]=((1 + 2 + OutLen) & 0x00FF);
		}
	}

	if (optData[0] != Mifare_RC_Success)
	{
		optData[1] = 0;
		optData[2] = 0;

		optMsgLength[0]= 0;
		optMsgLength[1]= 1 + 2;	//Status + 2 bytes Length
	}
}

void api_pcd_mfc_con_Write(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=0;

	UINT IptLen = 0;	
	UINT OutLen = 0;

	IptLen = iptMsgLength[0] * 256 + iptMsgLength[1];

	optData[0]=Mifare_RC_Fail;
	
	if (IptLen == 17)
	{
		rspCode=ECL_LV1_WRITE(iptData[0], &iptData[1]);
		if(rspCode == SUCCESS)
		{
			OutLen = 0;
		
			//Mifare Response Format
			optData[0] = Mifare_RC_Success;
			optData[1] = ((OutLen) & 0xFF00)>>8;
			optData[2] = (OutLen & 0x00FF);

			optMsgLength[0]=((1 + 2 + OutLen) & 0xFF00)>>8;
			optMsgLength[1]=((1 + 2 + OutLen) & 0x00FF);
		}
	}

	if (optData[0] != Mifare_RC_Success)
	{
		optData[1] = 0;
		optData[2] = 0;

		optMsgLength[0]= 0;
		optMsgLength[1]= 1 + 2;	//Status + 2 bytes Length
	}
}

void api_pcd_mfc_con_Value(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=0;

	UINT IptLen = 0;	
	UINT OutLen = 0;

	IptLen = iptMsgLength[0] * 256 + iptMsgLength[1];

	optData[0]=Mifare_RC_Fail;

	if ((iptData[0] == 0xC0) || (iptData[0] == 0xC1))
	{
		if (IptLen == 6)
		{
			if (iptData[0] == 0xC0)
				rspCode=ECL_LV1_DECREMENT(iptData[1], &iptData[2]);
			else
				rspCode=ECL_LV1_INCREMENT(iptData[1], &iptData[2]);
			
			if(rspCode == SUCCESS)
			{
				OutLen = 0;
			
				//Mifare Response Format
				optData[0] = Mifare_RC_Success;
				optData[1] = ((OutLen) & 0xFF00)>>8;
				optData[2] = (OutLen & 0x00FF);

				optMsgLength[0]=((1 + 2 + OutLen) & 0xFF00)>>8;
				optMsgLength[1]=((1 + 2 + OutLen) & 0x00FF);
			}
		}
	}
	else
	{
		optData[0]=Mifare_RC_Invalid_Input;
	}
	
	if (optData[0] != Mifare_RC_Success)
	{
		optData[1] = 0;
		optData[2] = 0;

		optMsgLength[0]= 0;
		optMsgLength[1]= 1 + 2;	//Status + 2 bytes Length
	}
}

void api_pcd_mfc_con_Restore(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=0;

	UINT IptLen = 0;	
	UINT OutLen = 0;

	IptLen = iptMsgLength[0] * 256 + iptMsgLength[1];

	optData[0]=Mifare_RC_Fail;
	
	if (IptLen == 1)
	{
		rspCode=ECL_LV1_RESTORE(iptData[0]);
		if(rspCode == SUCCESS)
		{
			OutLen = 0;
		
			//Mifare Response Format
			optData[0] = Mifare_RC_Success;
			optData[1] = ((OutLen) & 0xFF00)>>8;
			optData[2] = (OutLen & 0x00FF);

			optMsgLength[0]=((1 + 2 + OutLen) & 0xFF00)>>8;
			optMsgLength[1]=((1 + 2 + OutLen) & 0x00FF);
		}
	}

	if (optData[0] != Mifare_RC_Success)
	{
		optData[1] = 0;
		optData[2] = 0;

		optMsgLength[0]= 0;
		optMsgLength[1]= 1 + 2;	//Status + 2 bytes Length
	}
}

void api_pcd_mfc_con_Transfer(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=0;

	UINT IptLen = 0;	
	UINT OutLen = 0;

	IptLen = iptMsgLength[0] * 256 + iptMsgLength[1];

	optData[0]=Mifare_RC_Fail;
	
	if (IptLen == 1)
	{
		rspCode=ECL_LV1_TRANSFER(iptData[0]);
		if(rspCode == SUCCESS)
		{
			OutLen = 0;
		
			//Mifare Response Format
			optData[0] = Mifare_RC_Success;
			optData[1] = ((OutLen) & 0xFF00)>>8;
			optData[2] = (OutLen & 0x00FF);

			optMsgLength[0]=((1 + 2 + OutLen) & 0xFF00)>>8;
			optMsgLength[1]=((1 + 2 + OutLen) & 0x00FF);
		}
	}

	if (optData[0] != Mifare_RC_Success)
	{
		optData[1] = 0;
		optData[2] = 0;

		optMsgLength[0]= 0;
		optMsgLength[1]= 1 + 2;	//Status + 2 bytes Length
	}
}

void api_pcd_mfc_con_TypeATransparent(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	Mifare_TransparentAPDU('A', iptMsgLength, iptData, optMsgLength, optData);
}

void api_pcd_mfc_con_TypeBTransparent(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	Mifare_TransparentAPDU('B', iptMsgLength, iptData, optMsgLength, optData);
}

void api_pcd_mfc_con_Polling(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=Mifare_RC_Fail;
	UCHAR rspPCD=0;
	UINT  tmr10ms=0;
	
	//Set Card Number to 0
	optData[3]=0;
	
	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 1)
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	else
	{
		if (iptData[0] == 0)	//0 Sec.: Terminate Activation
		{
			rspCode=Mifare_RC_Success;
		}
		else
		{
			tmr10ms=iptData[0]*100;
			rspPCD=Mifare_Polling(tmr10ms);
			if (rspPCD == SUCCESS)
			{
				rspPCD=ECL_LV1_COLLISION_DETECTION();

				if (rspPCD == ECL_LV1_COLLISION)
				{
					optData[3]=2;
				}
			}
			else
			{
				if (rspPCD == 0xF3)
				{
					optData[3]=2;
				}
			}
			
			if (rspPCD == SUCCESS)
			{
				optData[3]=1;
				rspCode=Mifare_RC_Success;
			}
			else
			{
				ECL_LV1_RESET();

				if (rspPCD == 0xD0)
				{
					rspCode=Mifare_RC_Success;
				}
				else
				{
					if ((rspPCD == 0xE0) ||
						(rspPCD == 0xE1) ||
						(rspPCD == 0xE2))
					{
						rspCode=rspPCD;
					}
					else
					{
						rspCode=Mifare_RC_Fail;
					}
				}
			}
		}
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2+1;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=1;	
}


void api_pcd_mfc_con_GetCardData(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=Mifare_RC_Fail;
	UCHAR msgLen=0;
	UCHAR crdType=0;
	UCHAR lenID=0;
	UCHAR bufID[10]={0};
	UCHAR bufATQA[2]={0};
	UCHAR bufSAK[1]={0};

	iptData=iptData;	//Prevent warning
	
	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 0)
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		rspCode=ECL_LV1_GetCardType(&crdType);
		if (rspCode == ECL_LV1_SUCCESS)
		{
			if (crdType == 'A')
			{
				rspCode=ECL_LV1_GetUID(&lenID, bufID);
				if (rspCode == ECL_LV1_SUCCESS)
				{
					rspCode=ECL_LV1_GetATQA(bufATQA);
					rspCode=ECL_LV1_GetSAK(bufSAK);
					optData[3]=0x02;
					memcpy(&optData[4], bufATQA, 2);
					memcpy(&optData[6], bufSAK, 1);
					optData[7]=lenID;
					memcpy(&optData[8], bufID, lenID);
					msgLen=1+2+1+1+lenID;
					
					rspCode=Mifare_RC_Success;
				}
				else
				{
					rspCode=Mifare_RC_Fail;
				}
			}
			else
			{
				rspCode=ECL_LV1_GetPUPI(bufID);
				if (rspCode == ECL_LV1_SUCCESS)
				{
					optData[3]=0x03;
					optData[4]=4;
					memcpy(&optData[5], bufID, 4);
					msgLen=1+1+4;
					
					rspCode=Mifare_RC_Success;
				}
				else
				{
					rspCode=Mifare_RC_Fail;
				}
			}
		}
		else
		{
			rspCode=Mifare_RC_Fail;
		}
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2+msgLen;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=msgLen;
}


//20131025
void api_pcd_mfc_sys_GetReaderInformation(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
//Avoid Warning
	iptMsgLength[0]=iptMsgLength[0];
	iptData[0]=iptData[0];

//concatenate data
	optData[0] = Mifare_RC_Success;
	optData[1] = 0;
	optData[2] = 32;
	memcpy(&optData[3],Mifare_Rdr_Serial_Num,8);
	memcpy(&optData[11],Mifare_Rdr_HW_ID,8);
	memcpy(&optData[19],Mifare_Rdr_HW_Ver,4);
	memcpy(&optData[23],Mifare_Rdr_Firm_ID,8);
	memcpy(&optData[31],Mifare_Rdr_Firm_Ver,4);

	optMsgLength[0] = 0x00;
	optMsgLength[1] = 1 + 2 + 32;	//Status + 2 bytes Length
	

}

void api_pcd_mfc_sys_GetFirmwareInformation(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT OptLen = 0;

//Avoid warning

	iptMsgLength[0] = iptMsgLength[0] ;
	iptData[0] = iptData[0] ;

//BER-TLV Format
	UCHAR tag61[22]={0x61};		
	UCHAR tag62[22]={0x62};		
	UCHAR tag63[22]={0x63};		
	UCHAR tag71[12]={0x71};
	UCHAR tag72[12]={0x72};
	UCHAR tag75[12]={0x75};

//handle each tag
	tag61[1] = strlen((char *)(Mifare_Firm_Name));
	memcpy(&tag61[2],Mifare_Firm_Name,tag61[1]);

	tag62[1] = strlen((char *)(Mifare_Firm_Ver));
	memcpy(&tag62[2],Mifare_Firm_Ver,tag62[1]);

	tag63[1] = strlen((char *)(Mifare_Firm_Date));
	memcpy(&tag63[2],Mifare_Firm_Date,tag63[1]);

	tag71[1] = strlen((char *)(Mifare_Firm_VISA_Ver));
	memcpy(&tag71[2],Mifare_Firm_VISA_Ver,tag71[1]);

	tag72[1] = strlen((char *)(Mifare_Firm_MAS_Ver));
	memcpy(&tag72[2],Mifare_Firm_MAS_Ver,tag72[1]);

	tag75[1] = strlen((char *)(Mifare_Firm_Mifare_Ver));
	memcpy(&tag75[2],Mifare_Firm_Mifare_Ver,tag75[1]);

//concatenate each tag, add the length	
	memcpy(&optData[OptLen+3],tag61,2+tag61[1]);
	OptLen+=(2+tag61[1]);
	
	memcpy(&optData[OptLen+3],tag62,2+tag62[1]);
	OptLen+=(2+tag62[1]);

	memcpy(&optData[OptLen+3],tag63,2+tag63[1]);
	OptLen+=(2+tag63[1]);

	memcpy(&optData[OptLen+3],tag71,2+tag71[1]);
	OptLen+=(2+tag71[1]);

	memcpy(&optData[OptLen+3],tag72,2+tag72[1]);
	OptLen+=(2+tag72[1]);

	memcpy(&optData[OptLen+3],tag75,2+tag75[1]);
	OptLen+=(2+tag75[1]);

	optData[0] = Mifare_RC_Success;
	optData[1] = (OptLen & 0xFF00) >> 8;
	optData[2] = (OptLen & 0x00FF);

//Calculate the length
	optMsgLength[0] = ((1 + 2 + OptLen) & 0xFF00) >> 8;	//Status + 2 bytes Length
	optMsgLength[1] = ((1 + 2 + OptLen) & 0x00FF) ;
	
}

void api_pcd_mfc_fil_UpdateFileInformation(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR flgErr = 0;

	UINT	IptLen = 0;
	UINT	index = 0;

	UCHAR	LenOfT= 0;
	UCHAR	LenOfL= 0;
	UINT	LenOfV= 0;

	//20131105
	UT_PutStr(1,0,FONT1,16,(UCHAR *)"File Updating...");
	UT_PutStr(2,0,FONT1,16,(UCHAR *)"DO NOT POWER OFF");
	//20131105 end

	IptLen = iptMsgLength[0]*256 + iptMsgLength[1];

	while(IptLen - index)
	{	
		LenOfT = LenOfL = LenOfV = 0;
	
		UT_Get_TLVLength(&iptData[index],&LenOfT,&LenOfL,&LenOfV);

		if(LenOfT && LenOfL && LenOfV)
		{
			if(iptData[index] == 0x81) 
			{	
				memset(Mifare_Firm_Name_Update,0x00,sizeof(Mifare_Firm_Name_Update));
				memcpy(Mifare_Firm_Name_Update,&iptData[index+LenOfT+LenOfL],LenOfV);
			}
			else if(iptData[index] == 0x82)
			{	
				memset(Mifare_Firm_Ver_Update,0x00,sizeof(Mifare_Firm_Ver_Update));
				memcpy(Mifare_Firm_Ver_Update,&iptData[index+LenOfT+LenOfL],LenOfV);
			}
			else if(iptData[index] == 0x83)
			{
				memset(Mifare_Firm_Date_Update,0x00,sizeof(Mifare_Firm_Date_Update));
				memcpy(Mifare_Firm_Date_Update,&iptData[index+LenOfT+LenOfL],LenOfV);
			}
			else if(iptData[index] == 0x84)
			{
				// 1. Reset
				memset(Mifare_File_Size,0x00,sizeof(Mifare_File_Size));
				Mifare_Total_File_Size = 0;

				// 2. Copy to the Buffer
				memcpy(&Mifare_File_Size[4-LenOfV],&iptData[index+LenOfT+LenOfL],LenOfV);

				// 3. Calculate the Size
				Mifare_Total_File_Size = Mifare_File_Size[0] * 16777216 + Mifare_File_Size[1] * 65536 + Mifare_File_Size[2] * 256 + Mifare_File_Size[3];
			}
			else
			{	
				flgErr = 1;
				break;
			}
		}
		else
		{
			flgErr = 1;
			break;
		}		
		
		index += (LenOfT+LenOfL+LenOfV);
	}

	if(flgErr)
	{
		optData[0] = Mifare_RC_Fail;
	}
	else
	{
		optData[0] = Mifare_RC_Success;
	}

	optData[1] = 0;
	optData[2] = 0;

	optMsgLength[0] = 0;
	optMsgLength[1] = 1 + 2;	//Ststus + 2 bytes Length
	
}

void api_pcd_mfc_fil_ReceiveFile(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR Update_Percent_Char = 0;
	UCHAR Update_Percent[3] = {0};

	ULONG Current_Seq_Num = 0;

	ULONG Local_TOTAL_FILE_SIZE = 0;
	ULONG Local_RIGHTNOW_FILE_SIZE = 0;

	UCHAR rspCode = 0;

	UINT Packet_Len = 0;

	UINT iptLen = 0;

	iptLen = iptMsgLength[0] * 256 + iptMsgLength[1];

	memcpy((UCHAR *)&Current_Seq_Num,iptData,4);

	Packet_Len = iptLen - 4;	

	//Check the Sequence Num
	if(Mifare_Seq_Num == Current_Seq_Num)
	{
		//debug message
		//UT_PutStr(6,0,FONT0,17,(UCHAR *)"Checking Seq Pass");
			
		rspCode = api_dss2_file(Mifare_Mem_Offset,(ULONG)Packet_Len,&iptData[4]);
		if(rspCode == apiOK)
		{
			Mifare_Mem_Offset += Packet_Len;

			Local_RIGHTNOW_FILE_SIZE = Mifare_Mem_Offset;
			Local_TOTAL_FILE_SIZE = Mifare_Total_File_Size;

			Update_Percent_Char = (Local_RIGHTNOW_FILE_SIZE / (Local_TOTAL_FILE_SIZE/100));
			UT_1hexto3asc(Update_Percent_Char,Update_Percent);
			
			//debug message
			UT_PutStr(6,0,FONT0,11,(UCHAR *)"Downloading");
			
			if(Update_Percent_Char < 10) 
				UT_PutStr(6,15,FONT0,1,&Update_Percent[2]);
			else if(( 9 < Update_Percent_Char ) && (Update_Percent_Char < 100))
				UT_PutStr(6,14,FONT0,2,&Update_Percent[1]);
			else
				UT_PutStr(6,13,FONT0,3,Update_Percent);
				
			UT_PutStr(6,17,FONT0,1,(UCHAR *)"%");
		
			optData[0] = Mifare_RC_Success;		
			optData[1] = 0;
			optData[2] = 0;

			optMsgLength[0] = 0;
			optMsgLength[1] = 1 + 2;//Ststus + 2 bytes Length

			Mifare_Seq_Num+=1;
		}
		else
		{
			//debug message
			//UT_ClearRow(6,1,FONT0);
			UT_PutStr(7,13,FONT0,5,(UCHAR *)"Error");
			
			optData[0] = Mifare_RC_Fail;
			optData[1] = 0;
			optData[2] = 4;
			memcpy(&optData[3],(UCHAR *)&Mifare_Seq_Num,4);	//Copy Seq to Response Data

			optMsgLength[0] = 0;
			optMsgLength[1] = 1 + 2 + 4;//Ststus + 2 bytes Length + Seq Num
		}	
	}
	else
	{
		//debug message
		//UT_PutStr(6,0,FONT0,17,(UCHAR *)"Checking Seq Fail");
		
		optData[0] = Mifare_RC_Fail;
		optData[1] = 0;
		optData[2] = 4;
		memcpy(&optData[3],(UCHAR *)&Mifare_Seq_Num,4);	//Copy Seq to Response Data

		optMsgLength[0] = 0;
		optMsgLength[1] = 1 + 2 + 4;//Ststus + 2 bytes Length + Seq Num
	}	
}

//CAPK debug_CAPK;

void api_pcd_mfc_pro_GetCAPKIndex(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT iptLen = 0;
	UCHAR OutLen = 0;
	UCHAR ErrFlg = 0;
	UCHAR rspCode = 0;

	UCHAR j = 0,VISA_Num_Keys = 0,MAS_Num_Keys = 0,JCB_Num_Keys = 0,AE_Num_Keys = 0,UPI_Num_Keys = 0,Discover_Num_Keys = 0;
	UCHAR VISA_AID[5]		= {0xA0,0x00,0x00,0x00,0x03};
	UCHAR Master_AID[5]		= {0xA0,0x00,0x00,0x00,0x04};
	UCHAR JCB_AID[5]		= {0xA0,0x00,0x00,0x00,0x65};
	UCHAR AE_AID[7]			= {0xA0,0x00,0x00,0x00,0x25};
	UCHAR UPI_AID[7]		= {0xA0,0x00,0x00,0x03,0x33};
	UCHAR Discover_AID[7]	= {0xA0,0x00,0x00,0x01,0x52};

	UCHAR VISA_addr = 2,MAS_addr = 2,JCB_addr = 2, AE_addr = 2,UPI_addr = 2,Discover_addr = 2;
	CAPK tmpCAPK;
	
	UCHAR tag40[1+1+CAPK_NUMBER]={0x40};	//AE
	UCHAR tag50[1+1+CAPK_NUMBER]={0x50};	//VISA
	UCHAR tag80[1+1+CAPK_NUMBER]={0x80};	//Master
	UCHAR tag90[1+1+CAPK_NUMBER]={0x90};	//UPI
	UCHAR tagC0[1+1+CAPK_NUMBER]={0xC0};	//JCB
	UCHAR tagD0[1+1+CAPK_NUMBER]={0xD0};	//Discover
	

//20140820 force to change mem type to flash
#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);	
#else
	rspCode = apiOK;
#endif

	iptLen = iptMsgLength[0]*256 + iptMsgLength[1];

	if((iptLen == 0x01) && (rspCode == apiOK))
	{
		// 1. Scan Flash and calculate whole key index 
		for(j=0;j<CAPK_NUMBER;j++)
		{								
			rspCode = api_fls_read(FLSID_CAPK,j*CAPK_LEN,CAPK_LEN,(UCHAR *)&tmpCAPK);
			//memcpy((UCHAR *)&debug_CAPK,(UCHAR *)&tmpCAPK,CAPK_LEN);

			if(rspCode == apiOK)
			{
				if ((!memcmp(tmpCAPK.RID,AE_AID,5)) &&
					((iptData[0] == 0x40) || (iptData[0] == 0xFF)))
				{
					AE_Num_Keys++;
					
					tag40[AE_addr++] = tmpCAPK.Index;
					tag40[1] = AE_Num_Keys;
				}

				if ((!memcmp(tmpCAPK.RID,VISA_AID,5)) &&
					((iptData[0] == 0x50) || (iptData[0] == 0xFF)))
				{
					VISA_Num_Keys++;
					
					tag50[VISA_addr++] = tmpCAPK.Index;
					tag50[1] = VISA_Num_Keys;
				}
				
				if ((!memcmp(tmpCAPK.RID,Master_AID,5)) &&
					((iptData[0] == 0x80) || (iptData[0] == 0xFF)))
				{
					MAS_Num_Keys++;

					tag80[MAS_addr++] = tmpCAPK.Index;
					tag80[1] = MAS_Num_Keys;
				}

				if ((!memcmp(tmpCAPK.RID,UPI_AID,5)) &&
					((iptData[0] == 0x90) || (iptData[0] == 0xFF)))
				{
					UPI_Num_Keys++;

					tag90[UPI_addr++] = tmpCAPK.Index;
					tag90[1] = UPI_Num_Keys;
				}
				
				if ((!memcmp(tmpCAPK.RID,JCB_AID,5)) &&
					((iptData[0] == 0xC0) || (iptData[0] == 0xFF)))
				{
					JCB_Num_Keys++;

					tagC0[JCB_addr++] = tmpCAPK.Index;
					tagC0[1] = JCB_Num_Keys;
				}

				if ((!memcmp(tmpCAPK.RID,Discover_AID,5)) &&
					((iptData[0] == 0xD0) || (iptData[0] == 0xFF)))
				{
					Discover_Num_Keys++;

					tagD0[Discover_addr++] = tmpCAPK.Index;
					tagD0[1] = Discover_Num_Keys;
				}
			}
			else
			{
				ErrFlg = 1;
			}
		}

		// 2. add number of key and key index to array
		if(!ErrFlg)
		{
			if(iptData[0] == 0x40)
			{
				OutLen = tag40[1]+ 1 + 1;	// "len of tag" and "len of len"
				memcpy(&optData[3],tag40,OutLen);
			}
			else if(iptData[0] == 0x50)
			{
				OutLen = tag50[1]+ 1 + 1;	// "len of tag" and "len of len"
				memcpy(&optData[3],tag50,OutLen);
			}
			else if(iptData[0] == 0x80)
			{
				OutLen = tag80[1]+ 1 + 1;	// "len of tag" and "len of len"
				memcpy(&optData[3],tag80,OutLen);
			}
			else if(iptData[0] == 0x90)
			{
				OutLen = tag90[1]+ 1 + 1;	// "len of tag" and "len of len"
				memcpy(&optData[3],tag90,OutLen);
			}
			else if(iptData[0] == 0xC0)
			{
				OutLen = tagC0[1]+ 1 + 1;	// "len of tag" and "len of len"
				memcpy(&optData[3],tagC0,OutLen);
			}
			else if(iptData[0] == 0xD0)
			{
				OutLen = tagD0[1]+ 1 + 1;	// "len of tag" and "len of len"
				memcpy(&optData[3],tagD0,OutLen);
			}
			else if(iptData[0] == 0xFF)
			{
				OutLen = tag40[1]+tag50[1]+tag80[1]+tag90[1]+tagC0[1]+tagD0[1]+12;
				memcpy(&optData[3],tag40,tag40[1]+2);
				memcpy(&optData[3+tag40[1]+2],tag50,tag50[1]+2);
				memcpy(&optData[3+tag40[1]+2+tag50[1]+2],tag80,tag80[1]+2);
				memcpy(&optData[3+tag40[1]+2+tag50[1]+2+tag80[1]+2],tag90,tag90[1]+2);
				memcpy(&optData[3+tag40[1]+2+tag50[1]+2+tag80[1]+2+tag90[1]+2],tagC0,tagC0[1]+2);
				memcpy(&optData[3+tag40[1]+2+tag50[1]+2+tag80[1]+2+tag90[1]+2+tagC0[1]+2],tagD0,tagD0[1]+2);
			}
			else
			{
				ErrFlg = 1;
			}
		}
	}
	else
	{
		ErrFlg = 1;
	}

// handle output data
	if(ErrFlg)	//Fail
	{
		optData[0] = Mifare_RC_Fail;
		optData[1] = 0;
		optData[2] = 0;

		optMsgLength[0] = 0;
		optMsgLength[1] = 1 + 2;	//Ststus + 2 bytes Length
	}
	else	//Success
	{
		optData[0] = Mifare_RC_Success;
		optData[1] = 0;
		optData[2] = OutLen ;

		optMsgLength[0] = 0;
		optMsgLength[1] = 1 + 2 + OutLen;	//Ststus + 2 bytes Length
	}	
}

void api_pcd_mfc_pro_DeleteAllCAPKKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT iptLen = 0;
	UCHAR ErrFlg = 0;
	UCHAR i = 0;
	CAPK tmpCAPK;
	UCHAR rspCode = 0;
	UCHAR DelOneKey = TRUE;
	UCHAR bufCAPK[CAPK_LEN*10];
	
	UCHAR AID[5] = {0};
	UCHAR VISA_AID[5] = {0xA0,0x00,0x00,0x00,0x03};
	UCHAR MAS_AID[5] = {0xA0,0x00,0x00,0x00,0x04};
	UCHAR JCB_AID[5] = {0xA0,0x00,0x00,0x00,0x65};

	iptLen = iptMsgLength[0] * 256 + iptMsgLength[1];

//Check Format
	if(iptLen == 0x01)
	{
		if(iptData[0] == 0xFF)
		{
			DelOneKey = FALSE;
			
#ifdef _PLATFORM_AS210			
			rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
			rspCode=apiOK;
#endif

			if(rspCode == apiOK)
			{
				//Delete All key				
				memset(bufCAPK, 0xFF, CAPK_LEN*10);

				//Delete 10 CAPK at Once to Reduce Write Time
				for(i = 0; i < (CAPK_NUMBER/10) ; i++)
				{
					rspCode = api_fls_write(FLSID_CAPK, CAPK_LEN*10*i, CAPK_LEN*10, bufCAPK);
					if(rspCode == apiFailed)
					{
						ErrFlg = TRUE;
					}
				}
			}
			else
			{
				ErrFlg = 1;
			}			
		}
		else if(iptData[0] == 0x50)
		{
			memcpy(AID,VISA_AID,5);
		}
		else if(iptData[0] == 0x80)
		{
			memcpy(AID,MAS_AID,5);
		}
		else if(iptData[0] == 0xC0)
		{
			memcpy(AID,JCB_AID,5);
		}
		else
			ErrFlg = 1;

//Delete One type Key
		if((!ErrFlg) && DelOneKey)
		{
#ifdef _PLATFORM_AS210		
			rspCode = api_fls_memtype(FLSID_CAPK,FLSType_Flash);
#else
			rspCode = apiOK;
#endif
			if(rspCode == apiOK)
			{
				for(i = 0; i<CAPK_NUMBER ; i++)
				{
					api_fls_read(FLSID_CAPK,CAPK_LEN*i,CAPK_LEN,(UCHAR *)&tmpCAPK);

					if(memcmp(tmpCAPK.RID,AID,5) == 0)
					{
						memset((UCHAR*)&tmpCAPK,0xFF,CAPK_LEN);

						rspCode = api_fls_write(FLSID_CAPK,CAPK_LEN*i,CAPK_LEN,(UCHAR *)&tmpCAPK);

						if(rspCode == apiFailed)
						{
							ErrFlg = TRUE;
						}
					}
				}
			}
			else
			{
				ErrFlg = 1;	
			}
		}	
	}
	else
		ErrFlg = 1;

// Handle output data
	if(ErrFlg)	//Fail
	{
		optData[0] = Mifare_RC_Fail;
	}
	else		//Success
	{
		optData[0] = Mifare_RC_Success;
	}
	
	optData[1] = 0;
	optData[2] = 0;

	optMsgLength[0] = 0;
	optMsgLength[1] = 1 + 2;	//Ststus + 2 bytes Length
}


void api_pcd_mfc_pro_SetAuxInterruptDeviceHandleNumber(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;

	if ((iptMsgLength[0]*256+iptMsgLength[1]) == 1)
	{
		mifare_dhnAUXInterrupt=iptData[0];
		rspCode=Mifare_RC_Success;
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1;
	optData[0]=rspCode;
}


void api_pcd_mfc_sys_GetContactlessChipSerialNumber(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	serNumber[11]={0};

	iptData=iptData;
	
	if ((iptMsgLength[0]*256+iptMsgLength[1]) == 0)
	{
		rspCode=ECL_LV1_GetChipSN(serNumber);
		if (rspCode == SUCCESS)
		{
			optMsgLength[0]=0;
			optMsgLength[1]=1+11;
			optData[0]=Mifare_RC_Success;
			memcpy(&optData[1], serNumber, 11);
		}
		else
		{
			optMsgLength[0]=0;
			optMsgLength[1]=1;
			optData[0]=Mifare_RC_Fail;
		}
	}
	else
	{
		optMsgLength[0]=0;
		optMsgLength[1]=1;
		optData[0]=Mifare_RC_Invalid_Command;
	}
}


void api_pcd_mfc_sys_GetDateTime(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	ascDatetime[16]={0};
	UCHAR	bcdDatetime[8]={0};

	iptData=iptData;

	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 0)
	{
		rspCode=Mifare_RC_Invalid_Command;

		optMsgLength[0]=0;
		optMsgLength[1]=1+2;

		optData[1]=0;
		optData[2]=0;
	}
	else
	{
		ascDatetime[0]='2';
		ascDatetime[1]='0';
		api_rtc_getdatetime(0, &ascDatetime[2]);
		UT_Compress(bcdDatetime, ascDatetime, 8);

		rspCode=Mifare_RC_Success;
		
		optMsgLength[0]=0;
		optMsgLength[1]=1+2+7;
		memcpy(&optData[1], bcdDatetime, 7);
	}	

	optData[0]=rspCode;
}


void api_pcd_mfc_sys_SetDateTime(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	rstCheck=FALSE;
	UCHAR	ascDatetime[14]={0};
	UCHAR	bufDatetime[15]={0};

	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 7)
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		UT_Split(ascDatetime, iptData, 7);

		//Modify format for UT_CheckDateTime
		bufDatetime[0]=12;
		memcpy(&bufDatetime[1], &ascDatetime[2], 12);
		rstCheck=UT_CheckDateTime(bufDatetime);
		if (rstCheck == TRUE)
		{
			api_rtc_setdatetime(0, &ascDatetime[2]);

			rspCode=Mifare_RC_Success;
		}
		else
		{
			rspCode=Mifare_RC_Invalid_Input;
		}
	}	

	optMsgLength[0]=0;
	optMsgLength[1]=1+2;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=0;
}


void api_pcd_mfc_lcd_ClearScreen(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;

	iptData=iptData;
	
	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 0)
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		UT_ClearScreen();

		rspCode=Mifare_RC_Success;
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=0;
}


void api_pcd_mfc_lcd_ClearLine(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;

	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 1)
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		if (iptData[0] < Mifare_LCD_MAX_LINES_FONT1)
		{
			UT_ClearRow(iptData[0], 1, FONT1);
			rspCode=Mifare_RC_Success;
		}
		else
		{
			if (iptData[0] == 0xFF)
			{
				UT_ClearScreen();
				rspCode=Mifare_RC_Success;
			}
			else
			{
				rspCode=Mifare_RC_Invalid_Input;
			}
		}
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=0;
}


void api_pcd_mfc_lcd_ClearLineAlphanumeric(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;

	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 1)
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		if (iptData[0] < Mifare_LCD_MAX_LINES_FONT0)
		{
			UT_ClearRow(iptData[0], 1, FONT0);
			rspCode=Mifare_RC_Success;
		}
		else
		{
			rspCode=Mifare_RC_Invalid_Input;
		}
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=0;
}


void api_pcd_mfc_lcd_WriteLine(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	txtLength=0;
	UCHAR	idxLine=0;
	UCHAR	txtAlignment=0;
	UCHAR	strPosition=0;

	if (((iptMsgLength[0]*256+iptMsgLength[1]) < 2) ||
		((iptMsgLength[0]*256+iptMsgLength[1]) > (2+Mifare_LCD_MAX_WORDS_FONT1)))
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		if ((iptData[0] < Mifare_LCD_MAX_LINES_FONT1) && (iptData[1] <= 2))
		{
			idxLine=iptData[0];
			txtAlignment=iptData[1];
			txtLength=(iptMsgLength[0]*256+iptMsgLength[1])-2;

			switch (txtAlignment)
			{
				case 0:	strPosition=(Mifare_LCD_MAX_WORDS_FONT1-txtLength)/2;	break;	//Center
				case 1: strPosition=0;											break;	//Left
				case 2: strPosition=Mifare_LCD_MAX_WORDS_FONT1-txtLength;		break;	//Right
			}

			UT_PutStr(idxLine, strPosition, FONT2, txtLength, &iptData[2]);

			rspCode=Mifare_RC_Success;
		}
		else
		{
			rspCode=Mifare_RC_Invalid_Input;
		}
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=0;
}


void api_pcd_mfc_lcd_WriteLineAlphanumeric(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	txtLength=0;
	UCHAR	idxLine=0;
	UCHAR	txtAlignment=0;
	UCHAR	strPosition=0;

	if (((iptMsgLength[0]*256+iptMsgLength[1]) < 2) ||
		((iptMsgLength[0]*256+iptMsgLength[1]) > (2+Mifare_LCD_MAX_WORDS_FONT0)))
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		if ((iptData[0] < Mifare_LCD_MAX_LINES_FONT0) && (iptData[1] <= 2))
		{
			idxLine=iptData[0];
			txtAlignment=iptData[1];
			txtLength=(iptMsgLength[0]*256+iptMsgLength[1])-2;

			switch (txtAlignment)
			{
				case 0:	strPosition=(Mifare_LCD_MAX_WORDS_FONT0-txtLength)/2;	break;	//Center
				case 1: strPosition=0;											break;	//Left
				case 2: strPosition=Mifare_LCD_MAX_WORDS_FONT0-txtLength;		break;	//Right
			}

			UT_PutStr(idxLine, strPosition, FONT0, txtLength, &iptData[2]);

			rspCode=Mifare_RC_Success;
		}
		else
		{
			rspCode=Mifare_RC_Invalid_Input;
		}
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=0;
}


void api_pcd_mfc_lcd_SwitchBackLight(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;

	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 3)
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		if (iptData[0] <= 2)
		{
#ifndef _SCREEN_SIZE_240x320
			switch (iptData[0])
			{
				case 0:	api_sys_backlight(0, 0x00000000);					break;
				case 1:	api_sys_backlight(0, 0xFFFFFFFF);					break;
				case 2:	api_sys_backlight(0, iptData[1]*256+iptData[2]);	break;
			}

			rspCode=Mifare_RC_Success;
#else
			rspCode=Mifare_RC_Invalid_Command;
#endif
		}
		else
		{
			rspCode=Mifare_RC_Invalid_Input;
		}
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=0;
}


void api_pcd_mfc_lcd_ShowContactlessSymbol(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;

	iptData=iptData;
	
	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 0)
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		
#ifdef _SCREEN_SIZE_128x64
		rspCode=Mifare_RC_Invalid_Command;
#else
		UT_Set_LEDSignal(IID_TAP_LOGO, 0xFFFF, 0x0000);
		rspCode=Mifare_RC_Success;
#endif

	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=0;
}


void api_pcd_mfc_lcd_SetBGPalette(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;

#ifdef _SCREEN_SIZE_128x64
	rspCode=Mifare_RC_Invalid_Command;
#else
	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 3)
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		Utils_Global_palette[0]=iptData[0];
		Utils_Global_palette[1]=iptData[1];
		Utils_Global_palette[2]=iptData[2];

		rspCode=Mifare_RC_Success;
	}
#endif

	optMsgLength[0]=0;
	optMsgLength[1]=1+2;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=0;
}


void api_pcd_mfc_led_SetLED(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	i=0;
	UINT	blkOn=0;
	UCHAR	flgOneByOne=TRUE;
	UCHAR	cntOn=0;
	UCHAR	cntOff=0;

	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 4)
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		for (i=0; i < 4; i++)
		{
			switch (iptData[i])
			{
				case 0x00:										break;
				case 0x80:	cntOff++;							break;
				case 0x81:	cntOn++;							break;
				default:	rspCode=Mifare_RC_Invalid_Input;	break;
			}

			if (rspCode == Mifare_RC_Invalid_Input) break;
		}

		if (rspCode != Mifare_RC_Invalid_Input)
		{

#ifdef _PLATFORM_AS350_LITE
			if ((cntOff == 4) ||
				((cntOn == 1) && (cntOff == 3)))
			{
				flgOneByOne=FALSE;
			}
#endif

			if (flgOneByOne == TRUE)
			{
				for (i=0; i < 4; i++)
				{
					if (iptData[i] != 0x00)
					{
						(iptData[i] == 0x80)?(blkOn=0x0000):(blkOn=0xFFFF);
												
						UT_Set_LEDSignal(2+i, blkOn, 0);	//LED ID Starts from 2
					}
				}
			}
			else
			{
				if (cntOff == 4)
				{
					UT_Set_LED(0);
				}
				else
				{
					for (i=0; i < 4; i++)
					{
						if (iptData[i] == 0x81)
						{
							UT_Set_LED(2+i);
							break;
						}
					}
				}
			}

			for (i=0; i < 4; i++) Mifare_Blink_LED_Timeout[i]=0;	//Disable Blink Timeout
			rspCode=Mifare_RC_Success;
		}
	}	

	optMsgLength[0]=0;
	optMsgLength[1]=1+2;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=0;
}


void api_pcd_mfc_led_BlinkLED(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	idxLED=0;

	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 3)
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		if ((iptData[0] >= 1) && (iptData[0] <= 4))
		{
			idxLED=iptData[0]-1;
			
			Mifare_Blink_LED_Timeout[idxLED]=iptData[1]*100;	//Convert to Timeout(10 ms)

			if (iptData[1] == 0)
			{
				//Turn Off LED
				UT_Set_LEDSignal(2+idxLED, 0, 0);	//LED ID Starts from 2
			}
			else
			{
				//Turn On and Blink
				Mifare_Blink_LED_StartTime[idxLED]=OS_GET_SysTimerFreeCnt();
				UT_Set_LEDSignal(2+idxLED, iptData[2], iptData[2]);
			}
			
			rspCode=Mifare_RC_Success;
		}
		else
		{
			rspCode=Mifare_RC_Invalid_Input;
		}
	}	

	optMsgLength[0]=0;
	optMsgLength[1]=1+2;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=0;
}


void api_pcd_mfc_buz_Beep(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=Mifare_RC_Fail;

	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 3)
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		switch (iptData[0])
		{
			case 0:
				//Do Nothing
				rspCode=Mifare_RC_Success;
				break;
				
			case 1:
				UT_BUZ_Beep1();
				rspCode=Mifare_RC_Success;
				break;
				
			case 2:
				UT_BUZ_Beep1();
				UT_WaitTime(10);
				UT_BUZ_Beep1();
				rspCode=Mifare_RC_Success;
				break;
				
			case 3:
				UT_BUZ_Beep_Delay(iptData[1]*256+iptData[2]);
				rspCode=Mifare_RC_Success;
				break;
				
			default:
				rspCode=Mifare_RC_Invalid_Input;
				break;
		}
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=0;	
}

void api_pcd_mfc_buz_Beep_Interval(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=Mifare_RC_Fail;
	UCHAR dhnBUZ;
    UCHAR bufBUZ[6];

	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 3)
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
        bufBUZ[0]=iptData[0];	// cycle
        bufBUZ[1]=iptData[1];	// on
        bufBUZ[2]=iptData[2];	// off
        bufBUZ[3]=0;			// frequency (UINT)
        bufBUZ[4]=0;			//
        bufBUZ[5]=0x5A;			// auto close after sound

        dhnBUZ=api_buz_open(bufBUZ);
        if (dhnBUZ != apiOutOfService)
        {
        	api_buz_sound(dhnBUZ);

        	rspCode=Mifare_RC_Success;
        }
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=0;
}

void api_pcd_mfc_aux_ECHOTest(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT msgLength=0;

	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	
	optMsgLength[0]=((msgLength+1+2) & 0xFF00)>>8;
	optMsgLength[1]=((msgLength+1+2) & 0x00FF);
	
	optData[0]=Mifare_RC_Success;
	memcpy(&optData[1], iptMsgLength, 2);		
	memcpy(&optData[3], iptData, msgLength);
}

void api_pcd_mfc_aux_ChangeBaudrate(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UINT iptLen = 0;
	UCHAR ErrFlg = 0;

	iptLen = iptMsgLength[0] * 256 + iptMsgLength[1];

	if(iptLen == 1)
	{		
		if(iptData[0] == 0x04)
		{
			glv_vap_BaudRate = COM_19200;
		}
		else if(iptData[0] == 0x03)
		{
			glv_vap_BaudRate = COM_28800;
		}
		else if(iptData[0] == 0x02)			
		{
			glv_vap_BaudRate = COM_38400;
		}
		else if(iptData[0] == 0x01) 		
		{
			glv_vap_BaudRate = COM_57600;
		}
		else if(iptData[0] == 0x00)
		{
			glv_vap_BaudRate = COM_115200;
		}
		else
		{
			ErrFlg = TRUE;
		}
	}
	else
	{
		ErrFlg = TRUE;
	}

	if(ErrFlg)
	{
		optData[0] = Mifare_RC_Fail;
	}
	else
	{	
		optData[0] = Mifare_RC_Success;
	}

	optData[1] = 0;
	optData[2] = 0;
	
	optMsgLength[0] = 0;
	optMsgLength[1] = 1 + 2;	//Status + 2 bytes Length

}

void api_pcd_mfc_iso_ActivateCard(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=Mifare_RC_Fail;
	UCHAR rspPCD=0;
	UINT  tmr10ms=0;
	
	//Set Card Number to 0
	optData[3]=0;
	
	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 1)
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	else
	{
		if (iptData[0] == 0)	//0 Sec.: Terminate Activation
		{
			rspCode=Mifare_RC_Success;
		}
		else
		{
			tmr10ms=iptData[0]*100;
			rspPCD=Mifare_Polling(tmr10ms);
			if (rspPCD == SUCCESS)
			{
				rspPCD=ECL_LV1_COLLISION_DETECTION();
				if (rspPCD == SUCCESS)
				{
					rspPCD=ECL_LV1_ACTIVATE();
				}
				else
				{
					if (rspPCD == ECL_LV1_COLLISION)
					{
						optData[3]=2;
					}
				}
			}
			else
			{
				if (rspPCD == 0xF3)
				{
					optData[3]=2;
				}
			}
			
			if (rspPCD == SUCCESS)
			{
				optData[3]=1;
				rspCode=Mifare_RC_Success;
			}
			else
			{
				ECL_LV1_RESET();

				if (rspPCD == 0xD0)
				{
					rspCode=Mifare_RC_Success;
				}
				else
				{
					if ((rspPCD == 0xE0) ||
						(rspPCD == 0xE1) ||
						(rspPCD == 0xE2))
					{
						rspCode=rspPCD;
					}
					else
					{
						rspCode=Mifare_RC_Fail;
					}
				}
			}
		}
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2+1;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=1;	
}


void api_pcd_mfc_iso_GetStatus(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=Mifare_RC_Fail;
	UCHAR msgLen=0;
	UCHAR crdType=0;
	UCHAR lenID=0;
	UCHAR bufID[10]={0};

	iptData=iptData;	//Prevent warning
	
	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 0)
	{
		rspCode=Mifare_RC_Invalid_Command;
	}
	else
	{
		rspCode=ECL_LV1_GetCardType(&crdType);
		if (rspCode == SUCCESS)
		{
			if (crdType == 'A')
			{
				rspCode=ECL_LV1_GetUID(&lenID, bufID);
				if (rspCode == SUCCESS)
				{
					optData[3]=0x02;
					optData[4]=lenID;
					memcpy(&optData[5], bufID, lenID);
					msgLen=1+1+lenID;
					
					rspCode=Mifare_RC_Success;
				}
				else
				{
					rspCode=Mifare_RC_Fail;
				}
			}
			else
			{
				rspCode=ECL_LV1_GetPUPI(bufID);
				if (rspCode == SUCCESS)
				{
					optData[3]=0x03;
					optData[4]=4;
					memcpy(&optData[5], bufID, 4);
					msgLen=1+1+4;
					
					rspCode=Mifare_RC_Success;
				}
				else
				{
					rspCode=Mifare_RC_Fail;
				}
			}
		}
		else
		{
			rspCode=Mifare_RC_Fail;
		}
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2+msgLen;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=msgLen;
}


void api_pcd_mfc_iso_TransmitAPDU(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=Mifare_RC_Fail;
	UCHAR rspDEP=0;
	UINT  msgLen=0;
	UINT  sndLen=0;
	UINT  rcvLen=0;
	UCHAR crdType=0;

	rspCode=ECL_LV1_GetCardType(&crdType);
	if (rspCode == SUCCESS)
	{
		sndLen=iptMsgLength[0]*256+iptMsgLength[1];
		rspDEP=ECL_LV1_DEP(sndLen, iptData, &rcvLen, &optData[3], 0);

		switch (rspDEP)
		{
			case ECL_LV1_SUCCESS:		rspCode=Mifare_RC_Success;		break;
			case ECL_LV1_TIMEOUT_ISO:	rspCode=Mifare_RC_No_Response;	break;
			default:					rspCode=Mifare_RC_Fail;			break;
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Command;
	}

	msgLen=1+2+rcvLen;	//Response Code(1)+Length(2)+rcvLen
	optMsgLength[0]=(msgLen & 0xFF00) >> 8;
	optMsgLength[1]=(msgLen & 0x00FF);

	optData[0]=rspCode;
	optData[1]=(rcvLen & 0xFF00) >> 8;
	optData[2]=(rcvLen & 0x00FF);
}


void api_pcd_mfc_iso_CardRemoval(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=Mifare_RC_Fail;
	UCHAR rspRemoval=0;
	UINT  tmr10ms=0;
	
	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 1)
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	else
	{
		if (iptData[0] == 0)
		{
			optData[3]=0x01;	//Terminate
		}
		else
		{
			tmr10ms=iptData[0]*100;
			rspRemoval=Mifare_Removal(tmr10ms);
			if (rspRemoval == TRUE)
			{
				ECL_LV1_FIELD_OFF();
				optData[3]=0x00;
			}
			else
			{
				optData[3]=(rspRemoval == 0x02) ? 0x01 : 0xFF;
			}
		}
		
		rspCode=Mifare_RC_Success;
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2+1;

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=1;
}


void api_pcd_mfc_av2_Authenticate_1st(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=Mifare_RC_Fail;
	UINT  rcvLen=0;
	
	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 2)
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	else
	{
		if ((iptData[0] != 0x60) && (iptData[0] != 0x61))
		{
			rspCode=Mifare_RC_Invalid_Input;
		}
		else
		{
			rspCode=ECL_LV1_AV2_AUTHENTICATION_1ST(iptData[0], iptData[1], &optData[3]);
			if (rspCode == SUCCESS)
			{
				rspCode=Mifare_RC_Success;
				rcvLen=4;
			}
		}
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2+rcvLen;

	optData[0]=rspCode;
	optData[1]=(rcvLen & 0xFF00)>>8;
	optData[2]=(rcvLen & 0x00FF);
}


void api_pcd_mfc_av2_Authenticate_2nd(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=Mifare_RC_Fail;
	UINT  rcvLen=0;
	
	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 9)
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	else
	{
		rspCode=ECL_LV1_AV2_AUTHENTICATION_2ND(iptData, &optData[3]);
		if (rspCode == SUCCESS)
		{
			rspCode=Mifare_RC_Success;
			rcvLen=5;
		}
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2+rcvLen;

	optData[0]=rspCode;
	optData[1]=(rcvLen & 0xFF00)>>8;
	optData[2]=(rcvLen & 0x00FF);
}


void api_pcd_mfc_av2_Transceive(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR rspCode=Mifare_RC_Fail;
	UINT  sndLen=0;
	UINT  rcvLen=0;
	
	sndLen=(UCHAR)(iptMsgLength[0]*256+iptMsgLength[1]);

	rspCode=Mifare_Check_ParityLength(sndLen);
	if (rspCode == SUCCESS)
	{
		rspCode=ECL_LV1_AV2_TRANSCEIVE(sndLen, iptData, &rcvLen, &optData[3]);
		if (rspCode == SUCCESS)
		{
			rspCode=Mifare_RC_Success;
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}

	optMsgLength[0]=0;
	optMsgLength[1]=1+2+rcvLen;

	optData[0]=rspCode;
	optData[1]=(rcvLen & 0xFF00)>>8;
	optData[2]=(rcvLen & 0x00FF);
}


#ifdef _PLATFORM_AS350
#ifdef _SCREEN_SIZE_320x240

void api_pcd_mfc_msr_Open(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=Mifare_RC_Fail;

	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	if (msgLength == 2)
	{
		mifare_dhnMSR=api_msr_open(iptData[1]);
		if (mifare_dhnMSR != apiOutOfService)
		{
			rspCode=Mifare_RC_Success;
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	
	optMsgLength[0]=0x00;
	optMsgLength[1]=0x03;
	
	optData[0]=rspCode;
	optData[1]=0x00;
	optData[2]=0x00;
}


void api_pcd_mfc_msr_Close(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	rspAPI=apiFailed;

	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	if (msgLength == 0)
	{
		rspAPI=api_msr_close(mifare_dhnMSR);
		if (rspAPI == apiOK)
		{
			mifare_dhnMSR=apiOutOfService;
			rspCode=Mifare_RC_Success;
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	
	optMsgLength[0]=0x00;
	optMsgLength[1]=0x03;
	
	optData[0]=rspCode;
	optData[1]=0x00;
	optData[2]=0x00;
}


void api_pcd_mfc_msr_Status(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	rspAPI=apiFailed;
	
	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	if (msgLength == 1)
	{
		if (mifare_dhnMSR != apiOutOfService)
		{
			rspAPI=api_msr_status(mifare_dhnMSR, iptData[0], &optData[3]);
			if ((rspAPI == apiOK) && (optData[3] == msrSwiped))
			{
				msgLength=4;
				rspCode=Mifare_RC_Success;
			}
			else
			{
				rspCode=Mifare_RC_Fail;
			}
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	
	optMsgLength[0]=(UCHAR)(((0x03+msgLength) & 0xFF00)>>8);
	optMsgLength[1]=(UCHAR)((0x03+msgLength) & 0x00FF);
	
	optData[0]=rspCode;
	optData[1]=(UCHAR)((msgLength & 0xFF00)>>8);
	optData[2]=(UCHAR)(msgLength & 0x00FF);
}

	
void api_pcd_mfc_msr_Read(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	rspAPI=apiFailed;
	ULONG	tmrStart, tmrNow;
	UCHAR	msrStatus[4]={0};
	UCHAR	cntTrack=0;
	UCHAR	numTrack;
	UCHAR	lenTrack;
	UCHAR	idxTrack=1;
	UCHAR	*ptrData=NULLPTR;
	
	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	if (msgLength == 3)
	{
		numTrack=iptData[2];
		if ((numTrack & 0xF8) == 0)
		{
			if (mifare_dhnMSR != apiOutOfService)
			{
				tmrStart=OS_GET_SysTimerFreeCnt();
				
				do
				{
					rspAPI=api_msr_status(mifare_dhnMSR, 0, msrStatus);
					if ((rspAPI == apiOK) && (msrStatus[0] == msrSwiped))
					{
						rspAPI=api_msr_getstring(mifare_dhnMSR, &iptData[1], &optData[3]);
						if (rspAPI == apiOK)
						{
							ptrData=&optData[3];
							msgLength=0;
							
							for (cntTrack=0; cntTrack < 3; cntTrack++, idxTrack<<=1)
							{
								if ((numTrack & idxTrack) &&
									(msrStatus[1+cntTrack] == msrDataOK))
								{
									lenTrack=ptrData[0]+1;
									msgLength+=lenTrack;
									ptrData+=lenTrack;
								}
								else
								{
									continue;
								}
							}

							rspCode=Mifare_RC_Success;
						}

						break;
					}


					tmrNow=OS_GET_SysTimerFreeCnt();
				} while ((tmrNow - tmrStart) < (ULONG)(iptData[0] * 100));
			}
		}
		else
		{
			rspCode=Mifare_RC_Invalid_Input;
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}

	if (rspCode != Mifare_RC_Success)
	{
		msgLength=0;
	}
	
	optMsgLength[0]=(UCHAR)(((0x03+msgLength) & 0xFF00)>>8);
	optMsgLength[1]=(UCHAR)((0x03+msgLength) & 0x00FF);
	
	optData[0]=rspCode;
	optData[1]=(UCHAR)((msgLength & 0xFF00)>>8);
	optData[2]=(UCHAR)(msgLength & 0x00FF);
}

void api_pcd_mfc_kbd_Open(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=Mifare_RC_Fail;

	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	if (msgLength == 5)
	{
		mifare_dhnKBD=api_kbd_open(0, iptData);
		if (mifare_dhnKBD != apiOutOfService)
		{
			rspCode=Mifare_RC_Success;
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	
	optMsgLength[0]=0x00;
	optMsgLength[1]=0x03;
	
	optData[0]=rspCode;
	optData[1]=0x00;
	optData[2]=0x00;
}


void api_pcd_mfc_kbd_Close(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	rspAPI=apiFailed;

	iptData=iptData;

	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	if (msgLength == 0)
	{
		if (mifare_dhnKBD != apiOutOfService)
		{
			rspAPI=api_kbd_close(mifare_dhnKBD);
			if (rspAPI == apiOK)
			{
				mifare_dhnKBD=apiOutOfService;
				rspCode=Mifare_RC_Success;
			}
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	
	optMsgLength[0]=0x00;
	optMsgLength[1]=0x03;
	
	optData[0]=rspCode;
	optData[1]=0x00;
	optData[2]=0x00;
}


void api_pcd_mfc_kbd_Status(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	rspAPI=apiFailed;

	iptData=iptData;

	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	if (msgLength == 0)
	{
		if (mifare_dhnKBD != apiOutOfService)
		{
			rspAPI=api_kbd_status(mifare_dhnKBD, &optData[3]);
			if (rspAPI == apiReady)
			{
				msgLength=1;
				rspCode=Mifare_RC_Success;
			}
			else
			{
				rspCode=Mifare_RC_Fail;
			}
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	
	optMsgLength[0]=(UCHAR)(((0x03+msgLength) & 0xFF00)>>8);
	optMsgLength[1]=(UCHAR)((0x03+msgLength) & 0x00FF);
	
	optData[0]=rspCode;
	optData[1]=(UCHAR)((msgLength & 0xFF00)>>8);
	optData[2]=(UCHAR)(msgLength & 0x00FF);
}


void api_pcd_mfc_kbd_Get(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	rspAPI=apiFailed;
	ULONG	tmrStart, tmrNow;

	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	if (msgLength == 1)
	{
		if (mifare_dhnKBD != apiOutOfService)
		{
			tmrStart=OS_GET_SysTimerFreeCnt();
			
			do
			{
				rspAPI=api_kbd_status(mifare_dhnKBD, &optData[3]);
				if (rspAPI == apiReady)
				{
					rspAPI=api_kbd_getchar(mifare_dhnKBD, &optData[3]);
					
					break;
				}
				
				tmrNow=OS_GET_SysTimerFreeCnt();
			} while ((tmrNow - tmrStart) < (ULONG)(iptData[0] * 100));
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}

	if (rspAPI == apiOK)
	{
		msgLength=1;
		rspCode=Mifare_RC_Success;
	}
	else
	{
		msgLength=0;
	}
	
	optMsgLength[0]=(UCHAR)(((0x03+msgLength) & 0xFF00)>>8);
	optMsgLength[1]=(UCHAR)((0x03+msgLength) & 0x00FF);
	
	optData[0]=rspCode;
	optData[1]=(UCHAR)((msgLength & 0xFF00)>>8);
	optData[2]=(UCHAR)(msgLength & 0x00FF);
}


void api_pcd_mfc_kbd_Getc(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	rspAPI=apiFailed;
	ULONG	tmrStart, tmrNow;

	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	if (msgLength == 3)
	{
		if ((iptData[1] < Mifare_LCD_MAX_LINES_FONT1) &&
			(iptData[2] < Mifare_LCD_MAX_WORDS_FONT1))
		{
			if (mifare_dhnKBD != apiOutOfService)
			{
				tmrStart=OS_GET_SysTimerFreeCnt();
				
				do
				{
					rspAPI=api_kbd_status(mifare_dhnKBD, &optData[3]);
					if (rspAPI == apiReady)
					{
						rspAPI=api_kbd_getchar(mifare_dhnKBD, &optData[3]);
						
						break;
					}
					
					tmrNow=OS_GET_SysTimerFreeCnt();
				} while ((tmrNow - tmrStart) < (ULONG)(iptData[0] * 100));

				
			}
		}
		else
		{
			rspCode=Mifare_RC_Invalid_Input;
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}

	if (rspAPI == apiOK)
	{
		UT_PutStr(iptData[1], iptData[2], FONT2, 1, &optData[3]);
		msgLength=1;
		rspCode=Mifare_RC_Success;
	}
	else
	{
		msgLength=0;
	}
	
	optMsgLength[0]=(UCHAR)(((0x03+msgLength) & 0xFF00)>>8);
	optMsgLength[1]=(UCHAR)((0x03+msgLength) & 0x00FF);
	
	optData[0]=rspCode;
	optData[1]=(UCHAR)((msgLength & 0xFF00)>>8);
	optData[2]=(UCHAR)(msgLength & 0x00FF);
}


void api_pcd_mfc_kbd_Gets(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	rspAPI=apiFailed;
	ULONG	tmrStart, tmrNow;
	UCHAR	keyBuffer[1];
	UCHAR	*ptrData=NULLPTR;
	UINT	cntString=0;

	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	if (msgLength == 1)
	{
		if (mifare_dhnKBD != apiOutOfService)
		{
			tmrStart=OS_GET_SysTimerFreeCnt();
			ptrData=&optData[3];
			
			do
			{
				rspAPI=api_kbd_status(mifare_dhnKBD, keyBuffer);
				if (rspAPI == apiReady)
				{
					rspAPI=api_kbd_getchar(mifare_dhnKBD, keyBuffer);
					if (rspAPI == apiOK)
					{
						if ((keyBuffer[0] == 'y') || (keyBuffer[0] == 'x'))
						{
							break;
						}
						else
						{
							*ptrData++=keyBuffer[0];
							cntString++;

							if (cntString < Mifare_LCD_MAX_WORDS_FONT1)
							{
								UT_PutStr((Mifare_LCD_MAX_LINES_FONT1-1), (Mifare_LCD_MAX_WORDS_FONT1-cntString-1), FONT2, cntString, &optData[3]);
							}
						}
					}
				}
				
				tmrNow=OS_GET_SysTimerFreeCnt();
			} while ((tmrNow - tmrStart) < (ULONG)(iptData[0] * 100));
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}

	if ((rspAPI == apiOK) && (keyBuffer[0] == 'y'))
	{
		msgLength=cntString;
		rspCode=Mifare_RC_Success;
	}
	else
	{
		msgLength=0;
	}
	
	optMsgLength[0]=(UCHAR)(((0x03+msgLength) & 0xFF00)>>8);
	optMsgLength[1]=(UCHAR)((0x03+msgLength) & 0x00FF);
	
	optData[0]=rspCode;
	optData[1]=(UCHAR)((msgLength & 0xFF00)>>8);
	optData[2]=(UCHAR)(msgLength & 0x00FF);
}


void api_pcd_mfc_bcr_Open(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=Mifare_RC_Fail;
	UCHAR	rspAPI=apiFailed;
	ULONG	tmrStart, tmrNow;

	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	if (msgLength == 1)
	{
		rspAPI=api_2DR_StartScan();
		if (rspAPI == apiOK)
		{
			mifare_2DR_rcvLength=0;
			tmrStart=OS_GET_SysTimerFreeCnt();
			
			do
			{
				rspAPI=api_2DR_ReceiveData(&mifare_2DR_rcvLength, mifare_2DR_rcvBuffer);
				if (rspAPI == apiOK)
				{
					rspCode=Mifare_RC_Success;
					break;
				}

				rspAPI=Mifare_Check_CloseBCR();
				if (rspAPI == TRUE)
				{
					rspAPI=api_2DR_StopScan();

					rspCode=Mifare_RC_Success;
					break;
				}

				tmrNow=OS_GET_SysTimerFreeCnt();
			} while ((tmrNow - tmrStart) < (ULONG)(iptData[0] * 100));
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}

	if ((tmrNow - tmrStart) >= (ULONG)(iptData[0] * 100))
	{
		rspAPI=api_2DR_StopScan();
	}

	optMsgLength[0]=0x00;
	optMsgLength[1]=0x03;
	
	optData[0]=rspCode;
	optData[1]=0x00;
	optData[2]=0x00;
}


void api_pcd_mfc_bcr_Close(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=Mifare_RC_Fail;

	iptData=iptData;

	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	if (msgLength == 0)
	{
		rspCode=Mifare_RC_Success;
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}

	optMsgLength[0]=0x00;
	optMsgLength[1]=0x03;
	
	optData[0]=rspCode;
	optData[1]=0x00;
	optData[2]=0x00;
}


void api_pcd_mfc_bcr_Read(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData)
{
	UINT	msgLength=0;
	UCHAR	rspCode=Mifare_RC_Fail;

	iptData=iptData;

	msgLength=(iptMsgLength[0]*256+iptMsgLength[1]);
	if (msgLength == 0)
	{
		if ((mifare_2DR_rcvLength != 0) && (mifare_2DR_rcvLength <= MIFARE_2DR_BUFFER))
		{
			msgLength=mifare_2DR_rcvLength;
			memcpy(&optData[3], mifare_2DR_rcvBuffer, mifare_2DR_rcvLength);

			mifare_2DR_rcvLength=0;

			rspCode=Mifare_RC_Success;
		}
	}
	else
	{
		rspCode=Mifare_RC_Invalid_Input;
	}

	optMsgLength[0]=(UCHAR)(((0x03+msgLength) & 0xFF00)>>8);
	optMsgLength[1]=(UCHAR)((0x03+msgLength) & 0x00FF);
	
	optData[0]=rspCode;
	optData[1]=(UCHAR)((msgLength & 0xFF00)>>8);
	optData[2]=(UCHAR)(msgLength & 0x00FF);
}

#endif
#endif


#ifdef _PLATFORM_AS350_LITE
#ifdef _ACQUIRER_NCCC

void api_pcd_mfc_n3c_GetParameterInformation(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;
	UINT	msgLength=0;
	UCHAR	optBuffer[1500];

	iptData=iptData;
	
	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 0)
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	else
	{	
		NCCC_Lite_ShowInfo(1, optBuffer);	//Len(1) + Data(x)
		msgLength=(UINT)optBuffer[0];
		memcpy(&optData[3], &optBuffer[1], msgLength);

		rspCode=Mifare_RC_Success;
	}

	optMsgLength[0]=((msgLength+1+2) & 0xFF00)>>8;
	optMsgLength[1]=((msgLength+1+2) & 0x00FF);

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=(UCHAR)msgLength;
}

void api_pcd_mfc_n3c_ShowError(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;
	UINT	msgLength=0;

	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 1)
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	else
	{	
		NCCC_Lite_ShowError(iptData[0]);

		rspCode=Mifare_RC_Success;
	}

	optMsgLength[0]=((msgLength+1+2) & 0xFF00)>>8;
	optMsgLength[1]=((msgLength+1+2) & 0x00FF);

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=(UCHAR)msgLength;
}

void api_pcd_mfc_n3c_Reset(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData)
{
	UCHAR	rspCode=Mifare_RC_Fail;
	UINT	msgLength=0;

	iptData=iptData;

	if ((iptMsgLength[0]*256+iptMsgLength[1]) != 0)
	{
		rspCode=Mifare_RC_Invalid_Input;
	}
	else
	{
		rspCode=Mifare_RC_Success;
	}

	optMsgLength[0]=((msgLength+1+2) & 0xFF00)>>8;
	optMsgLength[1]=((msgLength+1+2) & 0x00FF);

	optData[0]=rspCode;
	optData[1]=0;
	optData[2]=(UCHAR)msgLength;
}

#endif
#endif
// #endif

