#include <string.h>
#include "POSAPI.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Util.h"
#include "PCD_Define.h"
#include "EMV_Define.h"
#include "DTE_Define.h"
#include "NXP_Function.h"
#include "PCD_Function.h"
#include "DTE_Function.h"


//NXP_Function.c
extern UCHAR nxp_TxAmp_C;
extern UCHAR nxp_flgNoise;

//PCD_Function.c
extern UCHAR pcd_flgTypeA;		//Type A Card
extern UCHAR pcd_flgTypeB;		//Type B Card
extern UCHAR pcd_flgCompliant;	//Compliant with ISO/IEC 14443-4
extern UCHAR pcd_blkNumber;		//Block Number



//APDU Command
const UCHAR emv_cmdPPSE[20]={0x00,0xA4,0x04,0x00,0x0E,0x32,0x50,0x41,0x59,0x2E,0x53,0x59,0x53,0x2E,0x44,0x44,0x46,0x30,0x31,0x00};
const UCHAR	emv_cmdSend[18]={0x00,0xA4,0x04,0x00,0x0C,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x00};

//Function
UCHAR	emv_appFunction=EMV_FUNCTION_NONE;	//Application Function
UCHAR	emv_appMode=EMV_MODE_NONE;			//Application Mode

//Type
UCHAR	emv_typSndApp=0;					//Type of Transaction Send Application

//Flag
UCHAR	emv_flgLodModulation=FALSE;			//Load Modulation Test

//Index
UCHAR	emv_idxPCDFun=EMV_PCD_FUNCTION_NONE;//Index of PCD Function

//Card Return Parameters
UCHAR	emv_rcvATQA[2];						//ATQA
UCHAR	emv_rcvSAK[1];						//SAK
UCHAR	emv_rcvPUPI[4];						//PUPI


void EMV_Reset_PcdParameter(void)
{
	//Reset PCD EMV Parameter
	PCD_Reset_EmvParameter();
	
	//Reset Card Type Flag
	pcd_flgTypeA=FALSE;
	pcd_flgTypeB=FALSE;
}


void EMV_Indicator_Success(void)
{
	ECL_LV1_UTI_Beep_1S();
}


void EMV_Indicator_Error(void)
{
	ECL_LV1_UTI_Beep_2S();
}


void EMV_WAIT(void)
{
	ECL_LV1_UTI_Wait(EMV_DELAY_T_P);
}


void EMV_WAIT_RETRANSMISSION(void)
{
	ECL_LV1_UTI_Wait(EMV_DELAY_T_RETRANSMISSION);
}


void EMV_RESET(void)
{
	PCD_Switch_RF_Off();
	
	ECL_LV1_UTI_Wait(EMV_DELAY_T_RESET);

	PCD_Switch_RF_On();
}


void EMV_POWEROFF(void)
{
	PCD_Switch_RF_Off();
	
	ECL_LV1_UTI_Wait(EMV_DELAY_T_POWEROFF);

	PCD_Switch_RF_On();
}


UCHAR EMV_POLLING_TypeA(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	rspPolling=ECL_LV1_FAIL;
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[64];

	if (pcd_flgTypeA == TRUE)
	{
		rspPolling=ECL_LV1_SUCCESS;
	}
	else
	{
		EMV_WAIT();

		rspCode=PCD_WUPA(&rcvLen, rcvBuffer);
		if (rspCode != ECL_LV1_TIMEOUT_ISO)
		{
			pcd_flgTypeA=TRUE;

			PCD_HLTA();
		}
	}

	return rspPolling;
}


UCHAR EMV_POLLING_TypeB(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	rspPolling=ECL_LV1_FAIL;
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[64];

	if (pcd_flgTypeB == TRUE)
	{
		rspPolling=ECL_LV1_SUCCESS;
	}
	else
	{
		EMV_WAIT();
		
		rspCode=PCD_WUPB(&rcvLen, rcvBuffer);
		if (rspCode != ECL_LV1_TIMEOUT_ISO)
		{
			pcd_flgTypeB=TRUE;
		}
	}

	return rspPolling;
}


UCHAR EMV_POLLING_Check_Collision(void)
{
	if (pcd_flgTypeA && pcd_flgTypeB)
	{
		return TRUE;
	}

	return FALSE;
}


UCHAR EMV_POLLING(void)
{
	UCHAR rspCode=ECL_LV1_FAIL;

	EMV_Reset_PcdParameter();

	while (1)
	{
		//Type A
		rspCode=EMV_POLLING_TypeA();			if (rspCode == ECL_LV1_SUCCESS)	return ECL_LV1_SUCCESS;
		rspCode=EMV_POLLING_Check_Collision();	if (rspCode == TRUE)			return ECL_LV1_FAIL;

		//Type B
		rspCode=EMV_POLLING_TypeB();			if (rspCode == ECL_LV1_SUCCESS)	return ECL_LV1_SUCCESS;
		rspCode=EMV_POLLING_Check_Collision();	if (rspCode == TRUE)			return ECL_LV1_FAIL;

#ifdef DTE_FUNCTION_ENABLE
		rspCode=DTE_Get_AUXCommand();			if (rspCode == TRUE)			return ECL_LV1_FAIL;
#endif
	}

	return ECL_LV1_FAIL;
}


UCHAR EMV_COLLISION_DETECTION_TypeA(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	rcvUID[4]={0};
	UCHAR	cntCL=0;
	UCHAR	cntRetransmit=0;
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[64];

		
	EMV_WAIT();
	
	//WUPA
	for (cntRetransmit=0; cntRetransmit < 3; cntRetransmit++)
	{
		rspCode=PCD_WUPA(&rcvLen, rcvBuffer);
		if (rspCode != ECL_LV1_TIMEOUT_ISO)
		{
			break;
		}

		EMV_WAIT_RETRANSMISSION();
	}

	if (rspCode != ECL_LV1_SUCCESS)
	{
		return rspCode;
	}

	memcpy(emv_rcvATQA, rcvBuffer, 2);

	for (cntCL=0; cntCL < 3; cntCL++)
	{
		//Reset Retransmit Counter
		cntRetransmit=0;

		//ANTICOLLISION
		for (cntRetransmit=0; cntRetransmit < 3; cntRetransmit++)
		{
			rspCode=PCD_ANTICOLLISION((PCD_CASCADE_LEVEL1+cntCL*2), &rcvLen, rcvBuffer);
			if (rspCode != ECL_LV1_TIMEOUT_ISO)
			{
				break;
			}

			EMV_WAIT_RETRANSMISSION();
		}

		if (rspCode != ECL_LV1_SUCCESS)
		{
			return rspCode;
		}
		
		//Reset Retransmit Counter
		cntRetransmit=0;
		
		//SELECT
		for (cntRetransmit=0; cntRetransmit < 3; cntRetransmit++)
		{
			memcpy(rcvUID, rcvBuffer, 4);
			
			rspCode=PCD_SELECT((PCD_CASCADE_LEVEL1+cntCL*2), rcvUID, &rcvLen, rcvBuffer);
			if (rspCode != ECL_LV1_TIMEOUT_ISO)
			{
				break;
			}

			EMV_WAIT_RETRANSMISSION();
		}

		if (rspCode != ECL_LV1_SUCCESS)
		{
			return rspCode;
		}
		
		if (rcvUID[0] != PCD_CASCADE_TAG)
		{
			memcpy(emv_rcvSAK, rcvBuffer, 1);
			
			return ECL_LV1_SUCCESS;
		}
	}

	return ECL_LV1_FAIL;
}


UCHAR EMV_COLLISION_DETECTION_TypeB(void)
{
	UCHAR 	rspCode=ECL_LV1_FAIL;
	UCHAR 	cntRetransmit=0;
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[64];

	EMV_WAIT();

	for (cntRetransmit=0; cntRetransmit < 3; cntRetransmit++)
	{
		rspCode=PCD_WUPB(&rcvLen, rcvBuffer);
		if (rspCode != ECL_LV1_TIMEOUT_ISO)
		{
			break;
		}

		EMV_WAIT_RETRANSMISSION();
	}

	if (rspCode == ECL_LV1_SUCCESS)
	{
		memcpy(emv_rcvPUPI, &rcvBuffer[1], 4);
	}

	return rspCode;
}


UCHAR EMV_COLLISION_DETECTION(void)
{
	UCHAR rspCode=0;

	//Reset PCD EMV Parameter to Avoid Different Data From Polling Process
	PCD_Reset_EmvParameter();

	if (pcd_flgTypeA == TRUE)
	{
		rspCode=EMV_COLLISION_DETECTION_TypeA();
	}
	else
	{
		if (pcd_flgTypeB == TRUE)
		{
			rspCode=EMV_COLLISION_DETECTION_TypeB();
		}
	}

	return rspCode;
}


UCHAR EMV_ACTIVATE_TypeA(void)
{
	UCHAR	rspCode=0;
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[256];
	
	rspCode=PCD_RATS(&rcvLen, rcvBuffer);

	return rspCode;
}


UCHAR EMV_ACTIVATE_TypeB(void)
{
	UCHAR	rspCode=0;
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[256];
	
	rspCode=PCD_ATTRIB(emv_rcvPUPI, &rcvLen, rcvBuffer);

	return rspCode;
}


UCHAR EMV_ACTIVATE(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cntRetransmit=0;

	for (cntRetransmit=0; cntRetransmit < 3; cntRetransmit++)
	{
		if (pcd_flgTypeA == TRUE)
		{
			rspCode=EMV_ACTIVATE_TypeA();
		}
		else
		{
			if (pcd_flgTypeB == TRUE)
			{
				rspCode=EMV_ACTIVATE_TypeB();
			}
		}

		if (rspCode != ECL_LV1_TIMEOUT_ISO)
		{
			break;
		}
	}
	
	return rspCode;
}


void EMV_REMOVAL_TypeA(void)
{
	UCHAR	rspCode=0;
	UCHAR	cntNoRsp=0;	//No Response Counter
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[64];
	
	do
	{
		rspCode=PCD_WUPA(&rcvLen, rcvBuffer);
		if (rspCode == ECL_LV1_TIMEOUT_ISO)
		{
			cntNoRsp++;
		}
		else
		{
			PCD_HLTA();
			EMV_WAIT();

			cntNoRsp=0;
		}
	} while (cntNoRsp < 3);
}


void EMV_REMOVAL_TypeB(void)
{
	UCHAR	rspCode=0;
	UCHAR	cntNoRsp=0;	//No Response Counter
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[64];
	
	do
	{
		rspCode=PCD_WUPB(&rcvLen, rcvBuffer);
		if (rspCode == ECL_LV1_TIMEOUT_ISO)
		{
			cntNoRsp++;
		}
		else
		{
			EMV_WAIT();

			cntNoRsp=0;
		}
	} while (cntNoRsp < 3);
}


void EMV_REMOVAL(void)
{
	EMV_RESET();
	EMV_WAIT();

	if (pcd_flgTypeA == TRUE)
	{
		EMV_REMOVAL_TypeA();
	}
	else
	{
		if (pcd_flgTypeB == TRUE)
		{
			EMV_REMOVAL_TypeB();
		}
	}
}


void EMV_CollisionDetectionToActivate(UCHAR iptType)
{
	UCHAR rspCode=0;
	
	if (iptType == 'A')
	{
		pcd_flgTypeA=TRUE;
	}
	else if (iptType == 'B')
	{
		pcd_flgTypeB=TRUE;
	}

	rspCode=EMV_COLLISION_DETECTION();
	if (rspCode == ECL_LV1_SUCCESS)
	{
		rspCode=EMV_ACTIVATE();
	}		
}


UCHAR EMV_PreValidation(void)
{
	UCHAR	rspCode=0;
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[EMV_BUFFER_SIZE_RECEIVE];

	ECL_LV1_UTI_Wait(1000);	//Delay to Meet Spec.

	//Select PPSE
	rspCode=PCD_DEP(20, (UCHAR*)emv_cmdPPSE, &rcvLen, rcvBuffer);
	if (rspCode == ECL_LV1_SUCCESS)
	{
		rspCode=ECL_LV1_FAIL;
		
		if ((rcvLen >= 2) &&
			(((rcvBuffer[rcvLen-2] & 0xF0) == 0x60) || (rcvBuffer[rcvLen-2] == 0x90)))
		{
			rspCode=ECL_LV1_SUCCESS;
		}
	}

	emv_appFunction=EMV_FUNCTION_NONE;

	if (rspCode == ECL_LV1_SUCCESS)
	{
		EMV_Indicator_Success();
		
		return ECL_LV1_SUCCESS;
	}

	EMV_Indicator_Error();

	return ECL_LV1_FAIL;
}


UCHAR EMV_LoopBack(void)
{
	UCHAR	rspCode=0;
	UINT	sndLen=0;
	UINT	rcvLen=0;
	UCHAR	sndBuffer[EMV_BUFFER_SIZE_SEND];
	UCHAR	rcvBuffer[EMV_BUFFER_SIZE_RECEIVE];


	ECL_LV1_UTI_Wait(1000);	//Delay to Meet Spec.
	
	//Select PPSE
	rspCode=PCD_DEP(20, (UCHAR*)emv_cmdPPSE, &rcvLen, rcvBuffer);

	while (1)
	{
		if (rspCode == ECL_LV1_SUCCESS)
		{
			//Check Second Byte
			if 		(rcvBuffer[1] == 0x70)
			{
				EMV_Indicator_Success();
				
				return ECL_LV1_SUCCESS;
			}
			else if (rcvBuffer[1] == 0x72)
			{
				return ECL_LV1_POWEROFF;
			}

			//Convert R-APDU into Next C-APDU to be Sent
			sndLen=rcvLen-2;	//Stripping the Status Word
			memcpy(sndBuffer, rcvBuffer, sndLen);


			//Send New C-APDU
			rspCode=PCD_DEP(sndLen, sndBuffer, &rcvLen, rcvBuffer);
		}
		else
		{
			break;
		}
	}

	EMV_Indicator_Error();
	
	return ECL_LV1_FAIL;
}


UCHAR EMV_ApplicationCommandList_TypeA(UCHAR iptIndex)
{
	UCHAR	iptUID[4]={0x27,0xE9,0x3B,0x11};
	UCHAR	flgEnd=FALSE;


	if ((iptIndex == 8) || (iptIndex == 9))
	{
		pcd_blkNumber=(iptIndex == 8)?(0):(1);

		PCD_Allocate_Memory();
	}
	
	switch (iptIndex)
	{
		case 1: PCD_WUPA_Send();											break;
		case 2: PCD_HLTA();													break;
		case 3: PCD_WUPB_Send();											break;
		case 4: PCD_WUPA_Send();											break;
		case 5: PCD_ANTICOLLISION_Send(PCD_CASCADE_LEVEL1);					break;
		case 6: PCD_SELECT_Send(PCD_CASCADE_LEVEL1, iptUID);				break;
		case 7: PCD_RATS_Send();											break;
		case 8:	PCD_DEP_Send(PCD_BLOCK_TYPE_I, 20, (UCHAR*)emv_cmdPPSE);	break;
		case 9:	PCD_DEP_Send(PCD_BLOCK_TYPE_I, 18, (UCHAR*)emv_cmdSend);	break;
		default: flgEnd=TRUE;												break;
	}

	if (iptIndex == 2)
	{
		//Add 0.1 ms Delay for HLTA. Because HLTA don't Receive Response.
		ECL_LV1_UTI_Wait(100);
	}

	if ((iptIndex == 8) || (iptIndex == 9))
	{
		PCD_Free_Memory();

		//Add Delay for DEP. It takes longer to complete transmission.
		ECL_LV1_UTI_Wait(500);
	}

	if (flgEnd == TRUE)
	{
		return ECL_LV1_FAIL;
	}

	return ECL_LV1_SUCCESS;
}


UCHAR EMV_ApplicationCommandList_TypeB(UCHAR iptIndex)
{
	UCHAR	iptPUPI[4]={0x46,0xB5,0xC7,0xA0};
	UCHAR	flgEnd=FALSE;
	

	pcd_flgCompliant=TRUE;
	
	if ((iptIndex == 5) || (iptIndex == 6))
	{
		pcd_blkNumber=(iptIndex == 5)?(0):(1);

		PCD_Allocate_Memory();
	}	
	
	switch (iptIndex)
	{
		case 1: PCD_WUPB_Send();											break;
		case 2: PCD_WUPA_Send();											break;
		case 3: PCD_WUPB_Send();											break;
		case 4: PCD_ATTRIB_Send(iptPUPI);									break;
		case 5:	PCD_DEP_Send(PCD_BLOCK_TYPE_I, 20, (UCHAR*)emv_cmdPPSE);	break;
		case 6:	PCD_DEP_Send(PCD_BLOCK_TYPE_I, 18, (UCHAR*)emv_cmdSend);	break;
		default: flgEnd=TRUE;												break;
	}

	if ((iptIndex == 5) || (iptIndex == 6))
	{
		PCD_Free_Memory();

		//Add Delay for DEP. It takes longer to complete transmission.
		ECL_LV1_UTI_Wait(500);
	}
	
	if (flgEnd == TRUE)
	{
		return ECL_LV1_FAIL;
	}

	return ECL_LV1_SUCCESS;
}


void EMV_TransactionSendApplication(void)
{
	UCHAR	cntIndex=1;
	UCHAR	rspCode=ECL_LV1_FAIL;

	if (emv_appFunction == EMV_FUNCTION_TXNSENDAPP)
	{
		PCD_Switch_RF_On();
		ECL_LV1_UTI_Wait(100000);
		
		do
		{
			if (emv_typSndApp == EMV_TYPE_APPLICATION_A)
			{
				rspCode=EMV_ApplicationCommandList_TypeA(cntIndex);
			}
			else if (emv_typSndApp == EMV_TYPE_APPLICATION_B)
			{
				rspCode=EMV_ApplicationCommandList_TypeB(cntIndex);
			}

			if (rspCode == ECL_LV1_SUCCESS)
			{
				ECL_LV1_UTI_Wait(2000);
			}

			cntIndex++;
		} while (rspCode == ECL_LV1_SUCCESS);
	}
}


void EMV_MainLoop(void)
{
	UCHAR rspCode=0;
	UCHAR rspApp=0;

	while (emv_appFunction == EMV_FUNCTION_APPLICATION)
	{
		PCD_Switch_RF_On();

		rspCode=EMV_POLLING();
		if (rspCode == ECL_LV1_SUCCESS)
		{
			rspCode=EMV_COLLISION_DETECTION();
			if (rspCode == ECL_LV1_SUCCESS)
			{
				rspCode=EMV_ACTIVATE();
				if (rspCode == ECL_LV1_SUCCESS)
				{
					if		(emv_appMode == EMV_MODE_PREVALIDATION)	rspApp=EMV_PreValidation();
					else if (emv_appMode == EMV_MODE_LOOPBACK)		rspApp=EMV_LoopBack();

					if		(rspApp == ECL_LV1_SUCCESS)		EMV_REMOVAL();
					else if	(rspApp == ECL_LV1_POWEROFF)	EMV_POWEROFF();
					
					EMV_RESET();
					EMV_WAIT();
				}
			}
		}

		if (rspCode != ECL_LV1_SUCCESS) EMV_RESET();
	}
}


void EMV_PcdFunction(void)
{
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[64];
	
	if (emv_appFunction == EMV_FUNCTION_PCD)
	{
		switch (emv_idxPCDFun)
		{
			case EMV_PCD_FUNCTION_RESET:		EMV_RESET();							break;
			case EMV_PCD_FUNCTION_POLLING:		EMV_POLLING();							break;
			case EMV_PCD_FUNCTION_WUPA:			PCD_WUPA(&rcvLen, rcvBuffer);			break;
			case EMV_PCD_FUNCTION_RATS:			EMV_CollisionDetectionToActivate('A');	break;
			case EMV_PCD_FUNCTION_WUPB:			PCD_WUPB(&rcvLen, rcvBuffer);			break;
			case EMV_PCD_FUNCTION_ATTRIB:		EMV_CollisionDetectionToActivate('B');	break;
			case EMV_PCD_FUNCTION_CARRIERON:	PCD_Switch_RF_On();						break;
			case EMV_PCD_FUNCTION_CARRIEROFF:	PCD_Switch_RF_Off();					break;
			case EMV_PCD_FUNCTION_POWEROFF:		EMV_POWEROFF();							break;

			default:																	break;
		}

		emv_appFunction=EMV_FUNCTION_NONE;
	}
}


