#include <string.h>
#include "POSAPI.h"
#include "MPP_Define.h"

// Add by Wayne 2020/08/21 to avoid compiler warning
#include "DBG_Function.h"

extern	UCHAR	mpp_flgDBG;

//		AUX Device Handle Number
UCHAR	mpp_dbg_dhnAUX=0;


UCHAR MPP_DBG_Open_AUX(UCHAR iptPrtNo)
{
	if (mpp_flgDBG == TRUE)
	{
		mpp_dbg_dhnAUX=DBG_Open_AUX(iptPrtNo);
	}

	return mpp_dbg_dhnAUX;
}


void MPP_DBG_Put_String(UCHAR iptLen, UCHAR *iptString)
{
	if (mpp_flgDBG == TRUE)
	{
		DBG_Put_String(iptLen, iptString);
	}
}


void MPP_DBG_Put_Process(UCHAR iptState, UCHAR iptID, UCHAR iptRspCode)
{
	UCHAR	datBuffer[32]={0};
	UCHAR	msgState[3]={' ',' ',' '};
	UCHAR	msgID[3]={' ',' ',' '};
	UCHAR	msgRsp[5]={' ',' ',' ',' ',' '};

	if (mpp_flgDBG == TRUE)
	{
		switch (iptState)
		{
			case MPP_STATE_1:							msgState[0]='1';	break;
			case MPP_STATE_2:							msgState[0]='2';	break;
			case MPP_STATE_3:							msgState[0]='3';	break;
			case MPP_STATE_4:							msgState[0]='4';	break;
			case MPP_STATE_5:							msgState[0]='5';	break;
			case MPP_STATE_6:							msgState[0]='6';	break;
			case MPP_STATE_7:							msgState[0]='7';	break;
			case MPP_STATE_8:							msgState[0]='8';	break;
			case MPP_STATE_9:							msgState[0]='9';	break;
			case MPP_STATE_4Apostrophes:				msgState[0]='4';
														msgState[1]='\'';	break;
			case MPP_STATE_78_A:						msgState[0]='7';
														msgState[1]='8';	break;
			case MPP_STATE_10:							msgState[0]='1';
														msgState[1]='0';	break;
			case MPP_STATE_11:							msgState[0]='1';
														msgState[1]='1';	break;
			case MPP_STATE_12:							msgState[0]='1';
														msgState[1]='2';	break;
			case MPP_STATE_13:							msgState[0]='1';
														msgState[1]='3';	break;
			case MPP_STATE_14:							msgState[0]='1';
														msgState[1]='4';	break;
			case MPP_STATE_15:							msgState[0]='1';
														msgState[1]='5';	break;
			case MPP_STATE_16:							msgState[0]='1';
														msgState[1]='6';	break;
			case MPP_STATE_17:							msgState[0]='1';
														msgState[1]='7';	break;
			case MPP_STATE_R1:							msgState[0]='R';
														msgState[1]='1';	break;
			case MPP_STATE_3R1_D:						msgState[0]='3';
														msgState[1]='R';
														msgState[2]='1';	break;
			case MPP_STATE_51:							msgState[0]='5';
														msgState[1]='1';	break;
			case MPP_STATE_52:							msgState[0]='5';
														msgState[1]='2';	break;
			case MPP_STATE_53:							msgState[0]='5';
														msgState[1]='3';	break;
			case MPP_STATE_456_A:						msgState[0]='4';
														msgState[1]='5';
														msgState[2]='6';	break;
			case MPP_STATE_910_A:						msgState[0]='9';
														msgState[1]='1';
														msgState[2]='0';	break;
			case MPP_PROCEDURE_PreGACBalanceReading:	msgState[0]='B';
														msgState[1]='R';
														msgState[2]='1';	break;
			case MPP_PROCEDURE_PostGACBalanceReading:	msgState[0]='B';
														msgState[1]='R';
														msgState[2]='2';	break;
			case MPP_PROCEDURE_CVMSelection:			msgState[0]='C';
														msgState[1]='V';
														msgState[2]='M';	break;
			case MPP_PROCEDURE_PrepareGACCommand:		msgState[0]='G';
														msgState[1]='A';
														msgState[2]='C';	break;
			case MPP_PROCEDURE_ProcessingRestrictions:	msgState[0]='P';
														msgState[1]='R';
														msgState[2]='E';	break;
			case MPP_PROCEDURE_TerminalActionAnalysis:	msgState[0]='T';
														msgState[1]='A';
														msgState[2]='A';	break;
			case 0xDE:	msgState[0] = 'D';
						msgState[1] = 'E';
						msgState[2] = 'T';	break;
		}

		if (iptID < 10)
		{
			msgID[2]=iptID+'0';
		}
		else if (iptID < 100)
		{
			msgID[1]=(iptID / 10) + '0';
			msgID[2]=(iptID % 10) + '0';
		}
		else
		{
			msgID[0]=(iptID / 100) + '0';
			msgID[1]=((iptID / 10) % 10) + '0';
			msgID[2]=(iptID % 10) + '0';
		}

		if (iptRspCode == 1)
		{
			msgRsp[0]='T';
			msgRsp[1]='R';
			msgRsp[2]='U';
			msgRsp[3]='E';
		}
		else if (iptRspCode == 0)
		{
			msgRsp[0]='F';
			msgRsp[1]='A';
			msgRsp[2]='L';
			msgRsp[3]='S';
			msgRsp[4]='E';
		}
		else if (iptRspCode == 0xDE)
		{
			msgRsp[0] = 'D';
			msgRsp[1] = 'E';
			msgRsp[2] = 'K';
		}
		else if (iptRspCode == 0xDD)
		{
			msgRsp[0] = 'U';
			msgRsp[1] = 'p';
			msgRsp[2] = 'd';
		}

		//Length
		datBuffer[0]=15;

		//State
		memcpy(&datBuffer[2], msgState, 3);
		datBuffer[5]='.';

		//ID
		memcpy(&datBuffer[6], msgID, 3);
		datBuffer[9]=' ';

		//Response Code
		memcpy(&datBuffer[10], msgRsp, 5);

		//New Line
		datBuffer[15]=0x0D;
		datBuffer[16]=0x0A;

//		UT_Tx_AUX(mpp_dbg_dhnAUX, datBuffer);
		DBG_Put_String(13, &datBuffer[2]);
	}
}

