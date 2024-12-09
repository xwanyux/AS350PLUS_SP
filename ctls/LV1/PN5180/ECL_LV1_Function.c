#include <string.h>
#include "POSAPI.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Util.h"
#include "EMV_Function.h"
#include "PCD_Function.h"
#include "SPI_Function.h"
#include "ITR_Function.h"
#include "OS_Function.h"
#include "PCD_Define.h"

#ifdef PCD_PLATFORM_CLRC663
#include "NXP_Function.h"
#else
#include "PNQ_Function.h"
#endif


extern UCHAR	spi_flgOpen;

extern UCHAR	nxp_TxAmp_A;
extern UCHAR	nxp_TxAmp_B;
extern UCHAR	nxp_RxAna_A;
extern UCHAR	nxp_RxAna_B;

extern UCHAR	pcd_flgTypeA;
extern UCHAR	pcd_flgTypeB;
extern UCHAR	pcd_parUIDSize;
extern UCHAR	pcd_parUID[PCD_BUFFER_SIZE_UID];
extern UCHAR	pcd_parPUPI[PCD_BUFFER_SIZE_PUPI];

extern UCHAR	emv_rcvATQA[2];
extern UCHAR	emv_rcvSAK[1];


UCHAR ECL_LV1_OpenCL(void)
{
	UCHAR	rspCode=0;

#ifdef PCD_PLATFORM_CLRC663
	rspCode=SPI_Open(8000000);
#else	//PN5180
	rspCode=SPI_Open(7000000);
#endif

	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_Check_SPI_RC663();
#else
	rspCode=PNQ_Check_SPI_PN5180();
#endif

	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_CloseCL(void)
{
	if (spi_flgOpen == TRUE)
	{
		PCD_Switch_RF_Off();
	}
	
	SPI_Close();

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_InitialCL(void)
{
#ifdef PCD_PLATFORM_CLRC663
	NXP_Load_RC663_Parameter_AS350();
	NXP_Initialize_Reader();
#else
	PNQ_Load_RF_Parameter_TA();
	PNQ_Initialize_Reader();
#endif

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_GetChipSN(UCHAR * optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_Get_CLChipSerialNumber(optData);	//SN: 11 bytes
#else
	rspCode=PNQ_Get_CLChipSerialNumber(optData);	//SN: 16 bytes
#endif

	return rspCode;
}


UCHAR ECL_LV1_GetATQA(UCHAR * optATQA)
{
	memcpy(optATQA, emv_rcvATQA, 2);

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_GetUID(UCHAR * optLen, UCHAR * optUID)
{
	if (pcd_parUIDSize == 0)
	{
		return ECL_LV1_FAIL;
	}

	optLen[0]=pcd_parUIDSize;
	memcpy(optUID, pcd_parUID, pcd_parUIDSize);

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_GetPUPI(UCHAR * optPUPI)
{
	UCHAR	lenPUPI=0;

	lenPUPI=(UCHAR)strlen((char*)pcd_parPUPI);
	if (lenPUPI == 0)
	{
		return ECL_LV1_FAIL;
	}

	memcpy(optPUPI, pcd_parPUPI, 4);

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_GetSAK(UCHAR * optSAK)
{
	memcpy(optSAK, emv_rcvSAK, 1);

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_GetCardType(UCHAR * crdType)
{
	PCD_Get_Card_Type(crdType);

	if ((crdType[0] != 'A') && (crdType[0] != 'B'))
	{
		return ECL_LV1_FAIL;
	}

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_GetRegister(UCHAR iptRegister, UCHAR * optData)
{
#ifdef PCD_PLATFORM_CLRC663
	NXP_Read_Register(iptRegister, optData);
#else
	iptRegister=iptRegister;
	optData=optData;
#endif

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_SetCardType(UCHAR crdType)
{
	if ((crdType != 'A') && (crdType != 'B'))
	{
		return ECL_LV1_FAIL;
	}

	if (crdType == 'A')
	{
		pcd_flgTypeA=TRUE;
		pcd_flgTypeB=FALSE;
	}
	else
	{
		pcd_flgTypeA=FALSE;
		pcd_flgTypeB=TRUE;
	}

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_SetRegister(UCHAR iptRegister, UCHAR iptData)
{
#ifdef PCD_PLATFORM_CLRC663
	NXP_Write_Register(iptRegister, iptData);
#else
	iptRegister=iptRegister;
	iptData=iptData;
#endif

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_SetRFParameter(UCHAR iptType, UCHAR iptCard, UCHAR iptData)
{
#ifdef PCD_PLATFORM_CLRC663
	if (iptType == 0)	//TX
	{
		if (iptCard == 'A')
		{
			nxp_TxAmp_A=iptData;
		}
		else
		{
			if (iptCard == 'B')
			{
				nxp_TxAmp_B=iptData;
			}
		}
	}
	else
	{
		if (iptType == 1)	//RX
		{
			if (iptCard == 'A')
			{
				nxp_RxAna_A=iptData;
			}
			else
			{
				if (iptCard == 'B')
				{
					nxp_RxAna_B=iptData;
				}
			}
		}
	}
#else
	iptType=iptType;
	iptCard=iptCard;
	iptData=iptData;
#endif

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_ResetPCDParameter(void)
{
	EMV_Reset_PcdParameter();

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_POLLING_TypeA(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=EMV_POLLING_TypeA();

	return rspCode;
}


UCHAR ECL_LV1_POLLING_TypeB(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=EMV_POLLING_TypeB();

	return rspCode;
}


UCHAR ECL_LV1_POLLING_CheckCollision(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=EMV_POLLING_Check_Collision();
	if (rspCode == TRUE)
	{
		return ECL_LV1_SUCCESS;
	}

	return ECL_LV1_FAIL;
}


UCHAR ECL_LV1_COLLISION_DETECTION(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=EMV_COLLISION_DETECTION();

	return rspCode;
}


UCHAR ECL_LV1_ACTIVATE(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=EMV_ACTIVATE();

	return rspCode;
}


UCHAR ECL_LV1_REMOVAL(ULONG iptTimeout)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	rspRemove=ECL_LV1_FAIL;
	UCHAR	flgTimer=TRUE;
	UCHAR	cntNoRsp=0;	
	ULONG	tmrValue=0xFFFFFFFF;
	ULONG	tmrTick=0;
	ULONG	tmrStart=0;
	UCHAR	crdType=0;
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[13];


	EMV_RESET();
	EMV_WAIT();

	//Configure Timer
	(iptTimeout == 0)?(flgTimer=FALSE):(tmrValue=iptTimeout);
	tmrStart=OS_GET_SysTimerFreeCnt();

	//Get Card Type
	PCD_Get_Card_Type(&crdType);
	
	do
	{
		if (crdType == 'A')
		{
			rspCode=PCD_WUPA(&rcvLen, rcvBuffer);			
			if (rspCode == ECL_LV1_TIMEOUT_ISO)
			{
				cntNoRsp++;
				
				if (cntNoRsp == 3)
				{
					rspRemove=ECL_LV1_SUCCESS;
					break;
				}
			}
			else
			{
				PCD_HLTA();
				EMV_WAIT();

				cntNoRsp=0;
			}
		}
		else
		{
			if (crdType == 'B')
			{
				rspCode=PCD_WUPB(&rcvLen, rcvBuffer);
				if (rspCode == ECL_LV1_TIMEOUT_ISO)
				{
					cntNoRsp++;
					
					if (cntNoRsp == 3)				
					{
						rspRemove=ECL_LV1_SUCCESS;
						break;
					}
				}
				else
				{
					EMV_WAIT();

					cntNoRsp=0;
				}
			}
			else
			{
				break;
			}
		}

		//Check Timer
		if (flgTimer == TRUE) tmrTick=OS_GET_SysTimerFreeCnt()-tmrStart;
	} while (tmrTick < tmrValue);

	if (tmrTick >= tmrValue) rspRemove=ECL_LV1_TIMEOUT_USER;

	return rspRemove;
}


UCHAR ECL_LV1_RESET(void)
{
	EMV_RESET();

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_FIELD_ON(void)
{
	PCD_Switch_RF_On();

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_FIELD_OFF(void)
{
	PCD_Switch_RF_Off();

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_REQA(UINT * rcvLen, UCHAR * rcvATQA)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=PCD_REQA(rcvLen, rcvATQA);

	return rspCode;
}


UCHAR ECL_LV1_WUPA(UINT * rcvLen, UCHAR * rcvATQA)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=PCD_WUPA(rcvLen, rcvATQA);

	return rspCode;
}


UCHAR ECL_LV1_ANTICOLLISION(UCHAR selCL, UINT * rcvLen, UCHAR * rcvUID)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=PCD_ANTICOLLISION(selCL, rcvLen, rcvUID);

	return rspCode;
}


UCHAR ECL_LV1_SELECT(UCHAR selCL, UCHAR * selUID, UINT * rcvLen, UCHAR * rcvSAK)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=PCD_SELECT(selCL, selUID, rcvLen, rcvSAK);

	return rspCode;
}


UCHAR ECL_LV1_RATS(UINT * rcvLen, UCHAR * rcvATS)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=PCD_RATS(rcvLen, rcvATS);

	return rspCode;
}


UCHAR ECL_LV1_HLTA(void)
{
	PCD_HLTA();

	return ECL_LV1_SUCCESS;
}


UCHAR ECL_LV1_WUPB(UINT * rcvLen, UCHAR * rcvATQB)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=PCD_WUPB(rcvLen, rcvATQB);

	return rspCode;
}


UCHAR ECL_LV1_ATTRIB(UCHAR * iptPUPI, UINT * rcvLen, UCHAR * rcvATA)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=PCD_ATTRIB(iptPUPI, rcvLen, rcvATA);

	return rspCode;
}


UCHAR ECL_LV1_DEP(UINT iptLen, UCHAR *iptData, UINT *optLen, UCHAR *optData, ULONG iptTimeout)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PCD_DEP_WithTimeout(iptLen, iptData, optLen, optData, iptTimeout);

	return rspCode;
}


UCHAR ECL_LV1_LOADKEY(UCHAR * iptKey)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_LOADKEY(iptKey);
#else
	rspCode=PNQ_LOADKEY(iptKey);
#endif

	return rspCode;
}


UCHAR ECL_LV1_AUTHENTICATION(UCHAR iptAutType, UCHAR iptAddress, UCHAR * iptUID)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_AUTHENTICATION(iptAutType, iptAddress, iptUID);
#else
	rspCode=PNQ_AUTHENTICATION(iptAutType, iptAddress, iptUID);
#endif

	return rspCode;
}


UCHAR ECL_LV1_READ(UCHAR iptAddress, UCHAR * optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_READ(iptAddress, optData);
#else
	rspCode=PNQ_READ(iptAddress, optData);
#endif

	return rspCode;
}


UCHAR ECL_LV1_WRITE(UCHAR iptAddress, UCHAR * iptData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_WRITE(iptAddress, iptData);
#else
	rspCode=PNQ_WRITE(iptAddress, iptData);
#endif

	return rspCode;
}


UCHAR ECL_LV1_DECREMENT(UCHAR iptAddress, UCHAR * iptValue)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_DECREMENT(iptAddress, iptValue);
#else
	rspCode=PNQ_DECREMENT(iptAddress, iptValue);
#endif

	return rspCode;
}


UCHAR ECL_LV1_INCREMENT(UCHAR iptAddress, UCHAR * iptValue)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_INCREMENT(iptAddress, iptValue);
#else
	rspCode=PNQ_INCREMENT(iptAddress, iptValue);
#endif

	return rspCode;
}


UCHAR ECL_LV1_TRANSFER(UCHAR iptAddress)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_TRANSFER(iptAddress);
#else
	rspCode=PNQ_TRANSFER(iptAddress);
#endif

	return rspCode;
}


UCHAR ECL_LV1_RESTORE(UCHAR iptAddress)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_RESTORE(iptAddress);
#else
	rspCode=PNQ_RESTORE(iptAddress);
#endif

	return rspCode;
}


UCHAR ECL_LV1_AV2_AUTHENTICATION_1ST(UCHAR iptAutType, UCHAR iptAddress, UCHAR * optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_AV2_AUTHENTICATION_1ST(iptAutType, iptAddress, optData);
#else
	iptAutType=iptAutType;
	iptAddress=iptAddress;
	optData=optData;
#endif

	return rspCode;
}


UCHAR ECL_LV1_AV2_AUTHENTICATION_2ND(UCHAR * iptData, UCHAR * optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

#ifdef PCD_PLATFORM_CLRC663	
	rspCode=NXP_AV2_AUTHENTICATION_2ND(iptData, optData);
#else
	iptData=iptData;
	optData=optData;
#endif

	return rspCode;
}


UCHAR ECL_LV1_AV2_TRANSCEIVE(UINT iptLen, UCHAR * iptData, UINT * optLen, UCHAR * optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

#ifdef PCD_PLATFORM_CLRC663	
	rspCode=NXP_AV2_TRANSCEIVE(iptLen, iptData, optLen, optData);
#else
	iptLen=iptLen;
	iptData=iptData;
	optLen=optLen;
	optData=optData;
#endif

	return rspCode;
}
