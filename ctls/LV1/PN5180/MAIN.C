//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :                                                            **
//**  PRODUCT  : AS320                                                      **
//**                                                                        **
//**  FILE     : MAIN.C                                                     **
//**  MODULE   : APP_main()                                                 **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/04/20                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2010 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------

#include <string.h>
#include "POSAPI.h"
#include "PN5180Function.h"
#include "GlobalVar.h"
#include "PN5180Define.h"
#include "UTILS.H"
#include "LCDTFTAPI.H"
#include "EMV_Define.h"
#include "DTE_Define.h"
#include "EMV_Function.h"
#include "PCD_Function.h"
#include "DTE_Function.h"
#include "ECL_LV1_Function.h"
#include "ECL_LV1_Util.h"
#include "ECL_LV1_Define.h"


#define APP_VERION_NAME		"EMV CL Level 1"
#define APP_VERION_NUMBER	"v3.00 22051001"
#define APP_CHECKSUM		"9C7268A4683B8AF2"
#define APP_VERION_NO_DTE	"        NO DTE"

extern UCHAR dte_flgTA;
extern UCHAR emv_appFunction;
extern UCHAR emv_appMode;


extern volatile	ULONG os_SysTimerFreeCnt;


void APP_Show_ApplicationVersion(void)
{
	UCHAR	iptKey=0;
	ULONG	tmrTick1, tmrTick2;
	
	UT_PutStr(0, 0, FONT0, 14, (UCHAR*)APP_VERION_NAME);

#ifdef DTE_FUNCTION_ENABLE
	UT_PutStr(0, 16, FONT0, 14, (UCHAR*)APP_VERION_NUMBER);
#else
	UT_PutStr(0, 16, FONT0, 14, (UCHAR*)APP_VERION_NO_DTE);
#endif

	UT_PutStr(1, 0, FONT0,  9, (UCHAR*)"Checksum?");

	tmrTick1=os_SysTimerFreeCnt;

	do
	{
		iptKey=UT_GetKey();
		if (iptKey == 'y')
		{
			UT_PutStr(1, 0, FONT0,  9, (UCHAR*)"Checksum:");
			UT_PutStr(1, 9, FONT0, 16, (UCHAR*)APP_CHECKSUM);
			return;
		}
		
		tmrTick2=os_SysTimerFreeCnt-tmrTick1;
	} while (tmrTick2 < 100);

	UT_ClearRow(1, 1, FONT0);
}


void APP_L1_EMV_Contactless(void)
{
	UCHAR	rspCode=FALSE;
	
	//Display Landing Plane
	UT_Set_LEDSignal(IID_TAP_LOGO, 0xFFFF, 0x0000);
	UT_WaitTime(8);
	UT_ClearRow(0, 2, FONT0);
	UT_ClearRow(9, 4, FONT2);

	//Enable Device Test Environment
	dte_flgTA=TRUE;
	DTE_Open_AUX();	//COM0

	APP_Show_ApplicationVersion();
	
	while (1)
	{
		rspCode=DTE_Get_AUXCommand();

		EMV_MainLoop();

		EMV_PcdFunction();

		EMV_TransactionSendApplication();

		if (rspCode == TRUE)
		{
			if ((emv_appFunction == EMV_FUNCTION_NONE) && (emv_appMode == EMV_MODE_PREVALIDATION))
			{
				UT_ClearRow(1, 1, FONT0);
			}

			if ((emv_appFunction == EMV_FUNCTION_APPLICATION) && (emv_appMode == EMV_MODE_LOOPBACK))
			{
				UT_PutStr(1, 0, FONT0, 14, (UCHAR*)"LoopBack Start");
			}
		}
	}
}


void APP_main( void )
{
	UCHAR	rspCode=0;

	rspCode=ECL_LV1_OpenCL();
	rspCode=ECL_LV1_InitialCL();


	//Open Buzzer
	ECL_LV1_UTI_Open_Buzzer_1S();
	ECL_LV1_UTI_Open_Buzzer_2S();
	ECL_LV1_UTI_Beep_1S();

	APP_L1_EMV_Contactless();
}

