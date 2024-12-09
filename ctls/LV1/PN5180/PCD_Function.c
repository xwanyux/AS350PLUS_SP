#include <string.h>
#include <stdlib.h>
#include "POSAPI.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Util.h"
#include "PCD_Define.h"
#include "DTE_Define.h"
#include "DTE_Function.h"
#include "ITR_Function.h"
#include "OS_Function.h"


#ifdef PCD_PLATFORM_CLRC663
#include "NXP_Function.h"
#else
#include "PNQ_Function.h"
#endif


//EMV Parameter(Check PCD_Reset_EmvParameter)
UINT	pcd_parFSC=0;							//Frame Size Card
UCHAR	pcd_parFSCI=0;							//Frame Size Card Index
ULONG	pcd_parFWT=0;							//Frame Waiting Time
UCHAR	pcd_parFWI=0;							//Frame Waiting Time Integer
ULONG	pcd_parFWT_TEMP=0;						//Frame Waiting Time Temporary
ULONG	pcd_parSFGT=0;							//Start-up Frame Guard Time
ULONG	pcd_parSFGT_DELTA=0;					//Start-up Frame Guard Time Delta
UCHAR	pcd_parSFGI=0;							//Start-up Frame Guard Time Integer
UCHAR	pcd_parWTXM=0;							//Waiting Time Extention Multiplier
UINT	pcd_parFrameSize_Send=0;				//Frame Size to Send
UCHAR	pcd_parUIDSize=0;						//UID Size - Single/Double/Triple(4/7/10)
UCHAR	pcd_parUID[PCD_BUFFER_SIZE_UID];		//UID
UCHAR	pcd_parPUPI[PCD_BUFFER_SIZE_PUPI];		//Pseudo-Unique PICC Identifier

//EMV Flag
UCHAR 	pcd_flgTypeA=FALSE;						//Type A Card
UCHAR 	pcd_flgTypeB=FALSE;						//Type B Card
UCHAR 	pcd_flgCompliant=FALSE;					//Compliant with ISO/IEC 14443-4
UCHAR	pcd_flgWTX=FALSE;						//Waiting Time Extention

//DEP Parameter
UCHAR	pcd_blkNumber=0;						//Block Number
UINT	pcd_sndSize=0;							//Total Send Size
UINT	pcd_rcvSize=0;							//Total Receive Size
//UCHAR	pcd_sndData[PCD_BUFFER_SIZE_SEND];		//Send Data Buffer
//UCHAR	pcd_rcvData[PCD_BUFFER_SIZE_RECEIVE];	//Receive Data Buffer
UCHAR	*pcd_sndData=NULLPTR;					//Allocate send data buffer in PCD_DEP_WithTimeout
UCHAR	*pcd_rcvData=NULLPTR;					//Allocate receive data buffer in PCD_DEP_WithTimeout

//Data Pointer
UCHAR	*pcd_ptrSnd=NULLPTR;					//Pointer of Send Data
UCHAR	*pcd_ptrRcv=NULLPTR;					//Pointer of Receive Data

//DEP Flag
UCHAR 	pcd_flgChaining=0;						//Chaining Flag
UCHAR 	pcd_flgTraError=0;						//Transmission Error
UCHAR 	pcd_flgProError=0;						//Protocol Error
UCHAR 	pcd_flgTimeout=0;						//Timeout 

//DEP Counter
UCHAR	pcd_cntResend=0;						//Resend Counter
UCHAR 	pcd_cntNAK=0;							//NAK Counter




UCHAR PCD_Allocate_Memory(void)
{
	pcd_sndData=malloc(PCD_BUFFER_SIZE_SEND);
	if (pcd_sndData == NULLPTR)
	{
		return ECL_LV1_FAIL;
	}
	
	pcd_rcvData=malloc(PCD_BUFFER_SIZE_RECEIVE);
	if (pcd_rcvData == NULLPTR)
	{
		free(pcd_sndData);
		
		return ECL_LV1_FAIL;
	}

	return ECL_LV1_SUCCESS;
}


void PCD_Free_Memory(void)
{
	free(pcd_sndData);
	free(pcd_rcvData);

	pcd_sndData=NULLPTR;
	pcd_rcvData=NULLPTR;
}


UCHAR PCD_Check_Interrupt(void)
{
	if (ITR_Check_ICC()			== TRUE)	return ECL_LV1_STOP_ICCARD;
	if (ITR_Check_MagStripe()	== TRUE)	return ECL_LV1_STOP_MAGSTRIPE;
	if (ITR_Check_Cancel()		== TRUE)	return ECL_LV1_STOP_CANCEL;
	if (ITR_Check_StopNow()		== TRUE)	return ECL_LV1_STOP_NOW;
	if (ITR_Check_StopLater()	== TRUE)	return ECL_LV1_STOP_LATER;
	
	return ECL_LV1_FAIL;
}


UCHAR PCD_Check_ATQA(UINT rcvLen)
{
	if (rcvLen != 2)	return ECL_LV1_FAIL;	//ATQA Length Error

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_Check_ATQB(UINT *rcvLen, UCHAR *rcvATQB)
{
	if ((rcvLen[0] < 12) || (rcvLen[0] > 13))	return ECL_LV1_FAIL;	//ATQB Length Error
	if (rcvATQB[0] != 0x50)						return ECL_LV1_FAIL;	//ATQB Format Error
	if (rcvATQB[10] & 0x08)						return ECL_LV1_FAIL;	//Protocol Error

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_Check_OddParity(UCHAR iptData)
{
	UCHAR	idxNumber=128;
	UCHAR	cntBit=0;
	UCHAR	i=0;

	for (i=0; i < 8; i++)
	{
		if (iptData & idxNumber)
		{
			cntBit++;
		}

		idxNumber>>=1;
	}

	if (cntBit & 1)
	{
		return ECL_LV1_SUCCESS;
	}

	return ECL_LV1_FAIL;
}


UCHAR PCD_Get_BCC(UCHAR datLen, UCHAR *calData)
{
	UCHAR cntIdx=0;
	UCHAR calBCC=0;

	for (cntIdx=0; cntIdx < datLen; cntIdx++)
		calBCC^=calData[cntIdx];

	return calBCC;
}


UINT PCD_Get_FrameSize(UCHAR iptFSI)
{
	UINT frmSize=0;

	switch (iptFSI)	//Frame Size Index
	{
		case 0:		frmSize=16;		break;
		case 1:		frmSize=24;		break;
		case 2:		frmSize=32;		break;
		case 3:		frmSize=40;		break;
		case 4:		frmSize=48;		break;
		case 5:		frmSize=64;		break;
		case 6:		frmSize=96;		break;
		case 7:		frmSize=128;	break;
		case 8:		frmSize=256;	break;
		case 9:		frmSize=512;	break;
		case 10:	frmSize=1024;	break;
		case 11:	frmSize=2048;	break;
		case 12:	frmSize=4096;	break;
		default:	frmSize=4096;	break;
	}

	return frmSize;
}


UCHAR PCD_Get_BitRateCode(UCHAR idxBitRate)
{
	UCHAR rspCode=0;
	
	switch (idxBitRate)
	{
		case 1:		rspCode=0;	break;
		case 2:		rspCode=1;	break;
		case 4:		rspCode=2;	break;
		case 8:		rspCode=3;	break;
		default:	rspCode=0;	break;
	}

	return rspCode;
}


void PCD_Get_Card_Type(UCHAR *crdType)
{
	if (pcd_flgTypeA == TRUE)
	{
		*crdType='A';
	}
	else if (pcd_flgTypeB == TRUE)
	{
		*crdType='B';
	}
}


UCHAR PCD_Get_FWI(UCHAR iptData)
{
	UCHAR	parFWI=0;
	
	parFWI=iptData & 0x0F;

	if (parFWI > PCD_PARAMETER_FWI_MAX)
	{
		parFWI=4;
	}

	return parFWI;
}


ULONG PCD_Get_FWT(UCHAR iptFWI)
{
	return ((256*16)*ECL_LV1_UTI_pow(2, iptFWI));
}


ULONG PCD_Get_FWT_TEMP(ULONG iptFWT, UCHAR iptWTXM)
{
	ULONG	parFWT_TEMP=0;
	
	pcd_flgWTX=TRUE;

	parFWT_TEMP=iptFWT*iptWTXM;
	
	if (parFWT_TEMP > PCD_PARAMETER_FWT_MAX)
	{
		parFWT_TEMP=PCD_PARAMETER_FWT_MAX;
	}

	return parFWT_TEMP;
}


UCHAR PCD_Get_SFGI(UCHAR iptData)
{
	UCHAR	parSFGI=0;
	
	parSFGI=iptData & 0x0F;

	if (parSFGI > PCD_PARAMETER_SFGI_MAX)
	{
		parSFGI=0;
	}

	return parSFGI;
}


ULONG PCD_Get_SFGT(UCHAR iptSFGI)
{
	return ((256*16)*ECL_LV1_UTI_pow(2, iptSFGI));
}


ULONG PCD_Get_SFGT_DELTA(UCHAR iptSFGI)
{
	return (384*ECL_LV1_UTI_pow(2, iptSFGI));
}


void PCD_Get_ParameterFromATQB(UINT *rcvLen, UCHAR *rcvATQB)
{
	//0x50(1) + PUPI(4) + Application Data(4) + Protocol Info(3 or 4)

	//PUPI
	memcpy(pcd_parPUPI, &rcvATQB[1] ,4);

	//Application Data
	//The PCD shall disregard any value returned by the PICC in the Application Data

	//Protocol Info = Byte 1 + Byte 2 + Byte 3 + Byte 4(optional)
	//Byte 1:Bit Rate Capability(Default:106 kbit/s in both directions)

	//Byte 2:Max Frame Size(4 bits) +Protocol Type(4 bits)
	pcd_parFSCI=rcvATQB[10] >> 4;
	pcd_parFSC=PCD_Get_FrameSize(pcd_parFSCI);

	//Protocol Type
	
	//PCD shall disregard any value returned by the PICC in Minimum TR2
	if (rcvATQB[10] & 0x01)	pcd_flgCompliant=TRUE;	//PICC compliant with ISO/IEC 14443-4

	//Byte 3:FWI(4 bits) + ADC(2 bits) + FO(2 bits)
	//FWI
	pcd_parFWI=PCD_Get_FWI(rcvATQB[11] >> 4);
	pcd_parFWT=PCD_Get_FWT(pcd_parFWI);

	//ADC:PCD shall disregard the value of b3 returned by the PICC in the ADC field
	//FO:PCD shall disregard any value returned by the PICC in the FO field

	//Byte 4:SFGI(4 bits) + RFU(4 bits)
	//Extented ATQB
	if (rcvLen[0] == 13)
	{
		pcd_parSFGI=PCD_Get_SFGI(rcvATQB[12] >> 4);
		if (pcd_parSFGI != 0)		
		{
			pcd_parSFGT_DELTA=PCD_Get_SFGT_DELTA(pcd_parSFGI);
			pcd_parSFGT=PCD_Get_SFGT(pcd_parSFGI);
		}
	}
}


void PCD_Reset_EmvParameter(void)
{
	pcd_parFSC=0;				//Frame Size Card
	pcd_parFSCI=0;				//Frame Size Card Index
	pcd_parFWT=0;				//Frame Waiting Time
	pcd_parFWI=0;				//Frame Waiting Time Integer
	pcd_parFWT_TEMP=0;			//Frame Waiting Time Temporary
	pcd_parSFGT=0;				//Start-up Frame Guard Time
	pcd_parSFGT_DELTA=0;		//Start-up Frame Guard Time Delta
	pcd_parSFGI=0;				//Start-up Frame Guard Time Integer
	pcd_parWTXM=0;				//Waiting Time Extention Multiplier
	pcd_parFrameSize_Send=0;	//Frame Size to Send
	pcd_parUIDSize=0;			//UID Size
	memset(pcd_parUID, 0, 10);	//UID
	memset(pcd_parPUPI, 0, 4);	//Pseudo-Unique PICC Identifier	

	pcd_flgCompliant=FALSE;
	pcd_flgWTX=FALSE;
}


void PCD_Switch_RF_On(void)
{
#ifdef PCD_PLATFORM_CLRC663
	NXP_Load_Protocol_A();
	NXP_Switch_FieldOn();
#else
	PNQ_Load_Protocol_A();
	PNQ_Switch_FieldOn();
#endif
}


void PCD_Switch_RF_Off(void)
{
#ifdef PCD_PLATFORM_CLRC663
	NXP_Switch_FieldOff();
#else
	PNQ_Switch_FieldOff();
#endif
}


void PCD_REQA_Send(void)
{
	//Send REQA Process
#ifdef PCD_PLATFORM_CLRC663
	NXP_REQA_Send(PCD_PARAMETER_FDT_A_MIN_LB0);
#else
	PNQ_REQA_Send(PCD_PARAMETER_FDT_A_MIN_LB0);
#endif
}


UCHAR PCD_REQA_Check(UINT rcvLen)
{
	UCHAR rspCode=ECL_LV1_FAIL;
	
	rspCode=PCD_Check_ATQA(rcvLen);
	
	return rspCode;
}


UCHAR PCD_REQA_Receive(UINT *rcvLen, UCHAR *rcvATQA)
{
	UCHAR rspCode=0;
	
	//Receive Process
#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_REQA_Receive(rcvLen, rcvATQA);
#else
	rspCode=PNQ_REQA_Receive(rcvLen, rcvATQA);
#endif
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;
	
	//Check EMV Protocol Error
	rspCode=PCD_REQA_Check(rcvLen[0]);
	if (rspCode == ECL_LV1_FAIL)	//Protocol Error
		return ECL_LV1_FAIL;

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_REQA(UINT *rcvLen, UCHAR *rcvATQA)
{
	UCHAR	rspCode=0;

	PCD_REQA_Send();

	rspCode=PCD_REQA_Receive(rcvLen, rcvATQA);

	return rspCode;
}


void PCD_WUPA_Send(void)
{
	//Send WUPA Process
#ifdef PCD_PLATFORM_CLRC663
	NXP_WUPA_Send(PCD_PARAMETER_FDT_A_MIN_LB1);
#else
	PNQ_WUPA_Send(PCD_PARAMETER_FDT_A_MIN_LB1);
#endif
}


UCHAR PCD_WUPA_Check(UINT rcvLen)
{
	UCHAR rspCode=ECL_LV1_FAIL;
	
	rspCode=PCD_Check_ATQA(rcvLen);

	return rspCode;
}


UCHAR PCD_WUPA_Receive(UINT *rcvLen, UCHAR *rcvATQA)
{
	UCHAR rspCode=0;
	
	//Receive Process
#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_WUPA_Receive(rcvLen, rcvATQA);
#else
	rspCode=PNQ_WUPA_Receive(rcvLen, rcvATQA);
#endif
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;
	
	//Check EMV Protocol Error
	rspCode=PCD_WUPA_Check(rcvLen[0]);
	if (rspCode == ECL_LV1_FAIL)	//Protocol Error
		return ECL_LV1_FAIL;

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_WUPA(UINT *rcvLen, UCHAR *rcvATQA)
{
	UCHAR	rspCode=0;

	PCD_WUPA_Send();

	rspCode=PCD_WUPA_Receive(rcvLen, rcvATQA);

	return rspCode;
}


void PCD_ANTICOLLISION_Send(UCHAR selCL)
{
	//Send ANTICOLLISION Process
#ifdef PCD_PLATFORM_CLRC663
	NXP_ANTICOLLISION_Send(selCL, PCD_PARAMETER_FDT_A_MIN_LB0);
#else
	PNQ_ANTICOLLISION_Send(selCL, PCD_PARAMETER_FDT_A_MIN_LB0);
#endif
}


UCHAR PCD_ANTICOLLISION_Check(UINT rcvLen, UCHAR *rcvCLn)
{
	UCHAR tmpByte=0;

	if (rcvLen != 5)
		return ECL_LV1_FAIL;

	//Calculate BCC
	tmpByte=PCD_Get_BCC(4, rcvCLn);
	if (tmpByte != rcvCLn[4])	//BCC Error
		return ECL_LV1_FAIL;

	return ECL_LV1_SUCCESS;	//Correct
}


UCHAR PCD_ANTICOLLISION_Receive(UCHAR selCL, UINT *rcvLen, UCHAR *rcvCLn)
{
	UCHAR rspCode=0;

	//Receive Process
#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_ANTICOLLISION_Receive(rcvLen, rcvCLn);
#else
	rspCode=PNQ_ANTICOLLISION_Receive(rcvLen, rcvCLn);
#endif
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;

	//Check EMV Prtocol Error
	rspCode=PCD_ANTICOLLISION_Check(rcvLen[0], rcvCLn);
	if (rspCode == ECL_LV1_FAIL)		//Transmission Error
		return ECL_LV1_FAIL;

	//Save UID CLn(1/2/3)
	if (rcvCLn[0] == PCD_CASCADE_TAG)	//Cascade Tag
	{
		if (selCL == PCD_CASCADE_LEVEL1)
		{
			memcpy(&pcd_parUID[0], &rcvCLn[1], 3);
		}
		else if (selCL == PCD_CASCADE_LEVEL2)
		{
			memcpy(&pcd_parUID[3], &rcvCLn[1], 3);
		}
	}
	else
	{
		if (selCL == PCD_CASCADE_LEVEL1)
		{
			memcpy(&pcd_parUID[0], &rcvCLn[0], 4);
			pcd_parUIDSize=PCD_UID_SINGLE;
		}
		else if (selCL == PCD_CASCADE_LEVEL2)
		{
			memcpy(&pcd_parUID[3], &rcvCLn[0], 4);
			pcd_parUIDSize=PCD_UID_DOUBLE;
		}
		else if (selCL == PCD_CASCADE_LEVEL3)
		{
			memcpy(&pcd_parUID[6], &rcvCLn[0], 4);
			pcd_parUIDSize=PCD_UID_TRIPLE;
		}
	}

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_ANTICOLLISION(UCHAR selCL, UINT *rcvLen, UCHAR *rcvUID)
{
	UCHAR rspCode=0;
	UCHAR rcvCLn[5]={0};

	PCD_ANTICOLLISION_Send(selCL);

	rspCode=PCD_ANTICOLLISION_Receive(selCL, rcvLen, rcvCLn);
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;

	rcvLen[0]-=1;				//Remove BCC
	memcpy(rcvUID, rcvCLn, 4);	//Copy CL to UID

	return ECL_LV1_SUCCESS;
}


void PCD_SELECT_Send(UCHAR selCL, UCHAR *selUID)
{
	UCHAR cmdBuffer[9];
	UCHAR rspCode=ECL_LV1_FAIL;
	ULONG tmrSELECT=0;

	//Calculate Parity Bit for Timeout
	cmdBuffer[0]=selCL;						//SEL
	cmdBuffer[1]=0x70;						//NVB
	memcpy(&cmdBuffer[2], selUID, 4);		//UID
	cmdBuffer[6]=PCD_Get_BCC(4, selUID);	//BCC

	ECL_LV1_UTI_ComputeCrc('A', (char*)cmdBuffer, 7, &cmdBuffer[7], &cmdBuffer[8]);

	rspCode=PCD_Check_OddParity(cmdBuffer[8]);
	tmrSELECT=(rspCode == ECL_LV1_SUCCESS)?(PCD_PARAMETER_FDT_A_MIN_LB0):(PCD_PARAMETER_FDT_A_MIN_LB1);
	
	//Send Select Process
#ifdef PCD_PLATFORM_CLRC663
	NXP_SELECT_Send(selCL, selUID, cmdBuffer[6], tmrSELECT);
#else
	PNQ_SELECT_Send(selCL, selUID, cmdBuffer[6], tmrSELECT);
#endif
}


UCHAR PCD_SELECT_Check(UINT rcvLen)
{
	if (rcvLen != 1)
		return ECL_LV1_FAIL;

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_SELECT_Receive(UINT *rcvLen, UCHAR *rcvSAK)
{
	UCHAR rspCode=0;

	//Receive Process
#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_SELECT_Receive(rcvLen, rcvSAK);
#else
	rspCode=PNQ_SELECT_Receive(rcvLen, rcvSAK);
#endif
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;

	//Check EMV Prtocol Error
	rspCode=PCD_SELECT_Check(rcvLen[0]);
	if (rspCode == ECL_LV1_FAIL)
		return ECL_LV1_FAIL;

	//Receive Data Processing
	if ((rcvSAK[0] & 0x04) == 0x00)	//UID Complete
	{
		if (rcvSAK[0] & 0x20)		//PICC compliant with ISO/IEC 14443-4
			pcd_flgCompliant=TRUE;
	}

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_SELECT(UCHAR selCL, UCHAR *selUID, UINT *rcvLen, UCHAR *rcvSAK)
{
	UCHAR rspCode=0;

	PCD_SELECT_Send(selCL, selUID);

	rspCode=PCD_SELECT_Receive(rcvLen, rcvSAK);

	return rspCode;
}


void PCD_RATS_Send(void)
{
	UCHAR ratPar=0;		//RATS PARAM

	//PARAM(1 Byte)=FSDI(4 bit)+CID(4 bit)
	ratPar=PCD_PARAMETER_FSDI<<4;

	//Send RATS Process
#ifdef PCD_PLATFORM_CLRC663
	NXP_RATS_Send(ratPar, (PCD_PARAMETER_FWT_ACTIVATION+PCD_PARAMETER_T_DELTA));
#else
	PNQ_RATS_Send(ratPar, (PCD_PARAMETER_FWT_ACTIVATION+PCD_PARAMETER_T_DELTA));
#endif
}


UCHAR PCD_RATS_Check(UINT rcvLen, UCHAR *rcvATS)
{
	UCHAR cntTx=0;

	if ((rcvLen) != rcvATS[0])	return ECL_LV1_FAIL;	//Receive length not complient with TL
	if (rcvATS[0] == 1)			return ECL_LV1_SUCCESS;	//TL=1, There are no other Bytes
	if (rcvATS[1] & 0x80)		return ECL_LV1_FAIL;	//RFU Coding Error
	if (rcvATS[1] & 0x40)		cntTx++;				//TC is transmitted
	if (rcvATS[1] & 0x20)		cntTx++;				//TB is transmitted
	if (rcvATS[1] & 0x10)		cntTx++;				//TA is transmitted
	if (rcvATS[0] < (2+cntTx))	return ECL_LV1_FAIL;	//Length Error

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_RATS_Receive(UINT *rcvLen, UCHAR *rcvATS)
{
	UCHAR	rspCode=0;
	UCHAR	flgTA=0;
	UCHAR	flgTB=0;
	UCHAR	ptrItrByt=0;

	//Receive Process
#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_RATS_Receive(rcvLen, rcvATS);
#else
	rspCode=PNQ_RATS_Receive(rcvLen, rcvATS);
#endif
	if (rspCode != ECL_LV1_SUCCESS)
	{
		return rspCode;
	}

	//Check EMV Protocol Error
	rspCode=PCD_RATS_Check(rcvLen[0], rcvATS);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//T0:Interface Byte & FSCI
	if (rcvATS[1] & 0x10) flgTA=TRUE;
	if (rcvATS[1] & 0x20) flgTB=TRUE;

	//Set FSC
	pcd_parFSCI=(rcvATS[0] < 2)?(2):(rcvATS[1] & 0x0F);	//Default FSCI:2
	pcd_parFSC=PCD_Get_FrameSize(pcd_parFSCI);

	//TA:Bit Rate Capabilities
	//Default:106 kbit/s in both directions
	if (flgTA == TRUE) ptrItrByt++;

	//TB:FWI & SFGI
	if (flgTB == TRUE) ptrItrByt++;
	
	pcd_parFWI=PCD_Get_FWI(rcvATS[1+ptrItrByt] >> 4);
	pcd_parFWT=PCD_Get_FWT(pcd_parFWI);

	pcd_parSFGI=PCD_Get_SFGI(rcvATS[1+ptrItrByt] & 0x0F);
	if (pcd_parSFGI != 0)		
	{
		pcd_parSFGT_DELTA=PCD_Get_SFGT_DELTA(pcd_parSFGI);
		pcd_parSFGT=PCD_Get_SFGT(pcd_parSFGI);
	}
	
	//TC:CID & NAD
	//Disregard any value returned by the PICC in b1-b2 of TC

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_RATS(UINT *rcvLen, UCHAR *rcvATS)
{
	UCHAR rspCode=0;
	ULONG mcrSFGT=0;	//Microsecond SFGT

	//Reset DEP Block Number
	pcd_blkNumber=0;
	
	PCD_RATS_Send();

	rspCode=PCD_RATS_Receive(rcvLen, rcvATS);

	if ((rspCode == ECL_LV1_SUCCESS) && (pcd_parSFGI != 0))
	{
		mcrSFGT=(pcd_parSFGT+pcd_parSFGT_DELTA) / 13;	//13.568 MHz

		ECL_LV1_UTI_Wait(mcrSFGT);
	}
	
	return rspCode;
}

void PCD_HLTA_Send(void)
{
	//Send HLTA Process
#ifdef PCD_PLATFORM_CLRC663
	NXP_HLTA_Send(PCD_PARAMETER_FDT_A_MIN_LB0);
#else
	PNQ_HLTA_Send(PCD_PARAMETER_FDT_A_MIN_LB0);	//There's no FDT for HLTA
#endif
}


void PCD_HLTA(void)
{
	PCD_HLTA_Send();

	//PCD shall always consider the HLTA command as acknowledged
	//PICC should not response to a HLTA command
}


void PCD_REQB_Send(void)
{
	//Send WUPB Process
#ifdef PCD_PLATFORM_CLRC663
	NXP_REQB_Send(PCD_PARAMETER_FWT_ATQB);
#else
	PNQ_REQB_Send(PCD_PARAMETER_FWT_ATQB);
#endif
}


UCHAR PCD_REQB_Check(UINT *rcvLen, UCHAR *rcvATQB)
{
	UCHAR rspCode=ECL_LV1_FAIL;
	
	rspCode=PCD_Check_ATQB(rcvLen, rcvATQB);

	return rspCode;
}


UCHAR PCD_REQB_Receive(UINT *rcvLen, UCHAR *rcvATQB)
{
	UCHAR rspCode=0;

	//Receive Process
#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_REQB_Receive(rcvLen, rcvATQB);
#else
	rspCode=PNQ_REQB_Receive(rcvLen, rcvATQB);
#endif
	if (rspCode != ECL_LV1_SUCCESS)
	{
		return rspCode;
	}

	//Check EMV Protocol Error
	rspCode=PCD_REQB_Check(rcvLen, rcvATQB);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//Get Parameter Settings
	PCD_Get_ParameterFromATQB(rcvLen, rcvATQB);

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_REQB(UINT *rcvLen, UCHAR *rcvATQB)
{
	UCHAR rspCode=0;

	PCD_REQB_Send();

	rspCode=PCD_REQB_Receive(rcvLen, rcvATQB);

	return rspCode;
}


void PCD_WUPB_Send(void)
{
	//Send WUPB Process
#ifdef PCD_PLATFORM_CLRC663
	NXP_WUPB_Send(PCD_PARAMETER_FWT_ATQB);
#else
	PNQ_WUPB_Send(PCD_PARAMETER_FWT_ATQB);
#endif
}


UCHAR PCD_WUPB_Check(UINT *rcvLen, UCHAR *rcvATQB)
{
	UCHAR rspCode=ECL_LV1_FAIL;
	
	rspCode=PCD_Check_ATQB(rcvLen, rcvATQB);

	return rspCode;
}


UCHAR PCD_WUPB_Receive(UINT *rcvLen, UCHAR *rcvATQB)
{
	UCHAR rspCode=0;

	//Receive Process
#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_WUPB_Receive(rcvLen, rcvATQB);
#else
	rspCode=PNQ_WUPB_Receive(rcvLen, rcvATQB);
#endif

	if (rspCode != ECL_LV1_SUCCESS)
	{
		return rspCode;
	}

	//Check EMV Protocol Error
	rspCode=PCD_WUPB_Check(rcvLen, rcvATQB);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//Get Parameter Settings
	PCD_Get_ParameterFromATQB(rcvLen, rcvATQB);

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_WUPB(UINT *rcvLen, UCHAR *rcvATQB)
{
	UCHAR rspCode=0;

	PCD_WUPB_Send();

	rspCode=PCD_WUPB_Receive(rcvLen, rcvATQB);

	return rspCode;
}


void PCD_ATTRIB_Send(UCHAR *iptPUPI)
{
	UCHAR cmdATTRIB[9]={0};
	UCHAR tmpByte=0;

	//'1D' + PUPI + Param 1 + Param 2 + Param 3 + Param 4
	//Byte 1:0x1D
	cmdATTRIB[0]=0x1D;

	//Byte 2~5:PUPI
	memcpy(&cmdATTRIB[1], iptPUPI, 4);

	//Byte 6:Param 1(TR0 + TR1 + Support suppression)
	//The PCD shall set default minimum value of TR0 & TR1
	//It does not support suppression of SoS/EoS
	cmdATTRIB[5]=0x00;

	//Byte 7:Param 2(Bit rate + FSDI)
	tmpByte=PCD_Get_BitRateCode(PCD_PARAMETER_PICC2PCD);
	cmdATTRIB[6]=tmpByte<<6;

	tmpByte=PCD_Get_BitRateCode(PCD_PARAMETER_PCD2PICC);
	tmpByte<<=4;
	cmdATTRIB[6]|=tmpByte;

	cmdATTRIB[6]|=PCD_PARAMETER_FSDI;

	//Byte 8:Param 3
	cmdATTRIB[7]=(pcd_flgCompliant == TRUE)?(0x01):(0x00);

	//Byte 9:Param 4
	//The PCD shall not use CID
	cmdATTRIB[8]=0x00;

	//Send Command Process
#ifdef PCD_PLATFORM_CLRC663
	NXP_ATTRIB_Send(cmdATTRIB, (pcd_parFWT+PCD_PARAMETER_FWT_DELTA+PCD_PARAMETER_T_DELTA));
#else
	PNQ_ATTRIB_Send(cmdATTRIB, (pcd_parFWT+PCD_PARAMETER_FWT_DELTA+PCD_PARAMETER_T_DELTA));
#endif
}


UCHAR PCD_ATTRIB_Check(UCHAR rcvATA)
{
	UCHAR tmpByte=0;

	tmpByte=rcvATA & 0x0F;	//CID
	if (tmpByte == 0)
		return ECL_LV1_SUCCESS;

	return ECL_LV1_FAIL;
}


UCHAR PCD_ATTRIB_Receive(UINT *rcvLen, UCHAR *rcvATA)
{
	UCHAR rspCode=0;
	
	//Receive Process
#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_ATTRIB_Receive(rcvLen, rcvATA);
#else
	rspCode=PNQ_ATTRIB_Receive(rcvLen, rcvATA);
#endif
	if (rspCode != ECL_LV1_SUCCESS)
	{
		return rspCode;
	}

	//Check EMV Protocol Error
	rspCode=PCD_ATTRIB_Check(rcvATA[0]);

	//Byte 1:MBLI + CID
	//The PCD shall disregard any value returned in the MBLI field

	return rspCode;
}


UCHAR PCD_ATTRIB(UCHAR *iptPUPI, UINT *rcvLen, UCHAR *rcvATA)
{
	UCHAR rspCode=0;
	ULONG mcrSFGT=0;	//Microsecond SFGT

	//Reset DEP Block Number
	pcd_blkNumber=0;
	
	PCD_ATTRIB_Send(iptPUPI);

	rspCode=PCD_ATTRIB_Receive(rcvLen, rcvATA);

	if ((rspCode == ECL_LV1_SUCCESS) && (pcd_parSFGI != 0))
	{
		mcrSFGT=(pcd_parSFGT + pcd_parSFGT_DELTA) / 13;	//13.568 MHz

		ECL_LV1_UTI_Wait(mcrSFGT);
	}
	
	return rspCode;
}


void PCD_HLTB_Send(UCHAR *iptPUPI)
{
#ifdef PCD_PLATFORM_CLRC663
	NXP_HLTB_Send(iptPUPI, 10240);
#else
	PNQ_HLTB_Send(iptPUPI, 10240);
#endif
}


UCHAR PCD_HLTB_Check(UINT *rcvLen, UCHAR *rcvData)
{
	if ((rcvLen[0] == 1) && (rcvData[0] == 0x00))
	{
		return ECL_LV1_SUCCESS;
	}

	return ECL_LV1_FAIL;
}


UCHAR PCD_HLTB_Receive(UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR rspCode=ECL_LV1_FAIL;

#ifdef PCD_PLATFORM_CLRC663
	rspCode=NXP_HLTB_Receive(rcvLen, rcvData);
#else
	rspCode=PNQ_HLTB_Receive(rcvLen, rcvData);
#endif

	if (rspCode != ECL_LV1_SUCCESS)
	{
		return rspCode;
	}

	rspCode=PCD_HLTB_Check(rcvLen, rcvData);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_HLTB(UCHAR *iptPUPI)
{
	UCHAR rspCode=ECL_LV1_FAIL;
	UINT  rcvSize=0;
	UCHAR rcvData[32];
	
	PCD_HLTB_Send(iptPUPI);

	rspCode=PCD_HLTB_Receive(&rcvSize, rcvData);

	return rspCode;
}


UCHAR PCD_DESELECT_Send(void)
{
	UCHAR	sndData[1];
	UCHAR	crdType=0;
	
	//Configure PCB
	sndData[0]=PCD_BLOCK_S_DESELECT;

	//Send DEP Process
	PCD_Get_Card_Type(&crdType);

#ifdef PCD_PLATFORM_CLRC663
	NXP_DEP_Send(crdType, 1, sndData, (PCD_PARAMETER_FWT_DESELECT+PCD_PARAMETER_T_DELTA));
#else
	PNQ_DEP_Send(crdType, 1, sndData, (PCD_PARAMETER_FWT_DESELECT+PCD_PARAMETER_T_DELTA));
#endif

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_DESELECT_Check(UINT iptLen, UCHAR * iptData)
{
	if ((iptLen == 1) && ((iptData[0] & 0xF7) == PCD_BLOCK_S_DESELECT))
	{
		return ECL_LV1_SUCCESS;
	}
	
	return ECL_LV1_FAIL;
}


UCHAR PCD_DESELECT_Receive(void)
{
	UINT	rcvSize=0;
	UCHAR	rdrBuff[PCD_BUFFER_SIZE_TEMPORARY]={0};
	UCHAR	rspCode=0;
	UCHAR	crdType=0;
	
	//Clear Receive Data Buffer
	memset(rdrBuff, 0, PCD_BUFFER_SIZE_TEMPORARY);

	//Check Receive Data
#ifdef PCD_PLATFORM_CLRC663
	PCD_Get_Card_Type(&crdType);
	rspCode=NXP_DEP_Receive(crdType, &rcvSize, rdrBuff);
#else
	crdType=crdType;
	rspCode=PNQ_DEP_Receive(&rcvSize, rdrBuff);
#endif
	if (rspCode == ECL_LV1_SUCCESS)
	{
		rspCode=PCD_DESELECT_Check(rcvSize, rdrBuff);
		if (rspCode == ECL_LV1_SUCCESS)
		{
			return ECL_LV1_SUCCESS;
		}
	}

	return ECL_LV1_FAIL;
}


UCHAR PCD_DESELECT(void)
{
	UCHAR rspCode=0;
	
	PCD_DESELECT_Send();

	rspCode=PCD_DESELECT_Receive();
	
	return rspCode;
}


void PCD_DEP_Reset_Counter(void)
{
	pcd_cntResend=0;
	pcd_cntNAK=0;	
}


void PCD_DEP_Reset_Error(void)
{
	pcd_flgTraError=0;
	pcd_flgProError=0;
	pcd_flgTimeout=0;
}


void PCD_DEP_Reset_Pointer(UINT sndLen, UCHAR *sndData)
{
	pcd_sndSize=sndLen;
	pcd_ptrSnd=sndData;
	pcd_rcvSize=0;
	pcd_ptrRcv=pcd_rcvData;
}


void PCD_DEP_Reset(UINT sndLen, UCHAR *sndData)
{
	PCD_DEP_Reset_Counter();
	PCD_DEP_Reset_Error();
	PCD_DEP_Reset_Pointer(sndLen, sndData);
}


void PCD_DEP_Update_BlockNumber(void)
{
	pcd_blkNumber++;
	pcd_blkNumber&=0x01;
}


void PCD_DEP_Send_I_Normal(UINT datLen, UCHAR *datBuffer)
{
	UCHAR	crdType=0;
	
	//Configure PCB
	pcd_sndData[0]=PCD_BLOCK_I_NORMAL | pcd_blkNumber;

	//Copy Data to Information Field
	memcpy(&pcd_sndData[1], datBuffer, datLen);

	//Send DEP Process
	PCD_Get_Card_Type(&crdType);

#ifdef PCD_PLATFORM_CLRC663
	NXP_DEP_Send(crdType, (datLen+1), pcd_sndData, (pcd_parFWT+PCD_PARAMETER_FWT_DELTA+PCD_PARAMETER_T_DELTA));	//datLen Add 1 Byte PCB
#else
	PNQ_DEP_Send(crdType, (datLen+1), pcd_sndData, (pcd_parFWT+PCD_PARAMETER_FWT_DELTA+PCD_PARAMETER_T_DELTA));	//datLen Add 1 Byte PCB
#endif
}


void PCD_DEP_Send_I_Chaining(UINT datLen, UCHAR *datBuffer)
{
	UCHAR	crdType=0;
	
	//Configure PCB
	pcd_sndData[0]=PCD_BLOCK_I_CHAINING | pcd_blkNumber;

	//Copy Data to Information Field
	memcpy(&pcd_sndData[1], datBuffer, datLen);

	//Send DEP Process
	PCD_Get_Card_Type(&crdType);

#ifdef PCD_PLATFORM_CLRC663
	NXP_DEP_Send(crdType, (datLen+1), pcd_sndData, (pcd_parFWT+PCD_PARAMETER_FWT_DELTA+PCD_PARAMETER_T_DELTA));	//datLen Add 1 Byte PCB
#else
	PNQ_DEP_Send(crdType, (datLen+1), pcd_sndData, (pcd_parFWT+PCD_PARAMETER_FWT_DELTA+PCD_PARAMETER_T_DELTA));	//datLen Add 1 Byte PCB
#endif
}


UCHAR PCD_DEP_Send_I(UINT datSize, UCHAR *datBuffer)
{
	//Calculate Available Transmit Size
	pcd_parFrameSize_Send=(pcd_parFSC > PCD_PARAMETER_FSC)?(PCD_PARAMETER_FSC):(pcd_parFSC);
	pcd_parFrameSize_Send-=3;	//PCB(1)+CRC(2)

	if (pcd_cntResend < 3)
	{
		if (datSize <= pcd_parFrameSize_Send)	//Tx Data <= Frame Available Tx Size
		{
			PCD_DEP_Send_I_Normal(datSize, datBuffer);

			pcd_flgChaining=PCD_CHAINING_NONE;
		}
		else
		{
			PCD_DEP_Send_I_Chaining(pcd_parFrameSize_Send, datBuffer);

			pcd_flgChaining=PCD_CHAINING_PCD;
		}

		return ECL_LV1_SUCCESS;
	}

	pcd_flgProError=PCD_ERROR_PROTOCOL;

	return ECL_LV1_FAIL;
}


void PCD_DEP_Send_R_ACK(void)
{
	UCHAR	crdType=0;
	
	//Configure PCB
	pcd_sndData[0]=PCD_BLOCK_R_ACK | pcd_blkNumber;

	//Send DEP Process
	PCD_Get_Card_Type(&crdType);

#ifdef PCD_PLATFORM_CLRC663
	NXP_DEP_Send(crdType, 1, pcd_sndData, (pcd_parFWT+PCD_PARAMETER_FWT_DELTA+PCD_PARAMETER_T_DELTA));
#else
	PNQ_DEP_Send(crdType, 1, pcd_sndData, (pcd_parFWT+PCD_PARAMETER_FWT_DELTA+PCD_PARAMETER_T_DELTA));
#endif
}


void PCD_DEP_Send_R_NAK(void)
{
	UCHAR	crdType=0;
	ULONG	rcvTimeout=0;
	
	//Configure PCB
	pcd_sndData[0]=PCD_BLOCK_R_NAK | pcd_blkNumber;

	//Send DEP Process
	PCD_Get_Card_Type(&crdType);

	//Set Timeout
	rcvTimeout=(pcd_flgWTX)?(pcd_parFWT_TEMP):(pcd_parFWT);

#ifdef PCD_PLATFORM_CLRC663
	NXP_DEP_Send(crdType, 1, pcd_sndData, (rcvTimeout+PCD_PARAMETER_FWT_DELTA+PCD_PARAMETER_T_DELTA));
#else
	PNQ_DEP_Send(crdType, 1, pcd_sndData, (rcvTimeout+PCD_PARAMETER_FWT_DELTA+PCD_PARAMETER_T_DELTA));
#endif
}


UCHAR PCD_DEP_Send_R(void)
{
	//Check Error
	if ((pcd_flgTimeout != 0) || (pcd_flgTraError != 0) || (pcd_flgProError != 0))	//Any Error
	{ 	
		if (pcd_cntNAK < 2)	//Resend NAK < 2
		{
			if (pcd_flgChaining == PCD_CHAINING_PICC)	//PICC Chaining
			{
				PCD_DEP_Send_R_ACK();
			}
			else
			{
				PCD_DEP_Send_R_NAK();
			}

			pcd_cntNAK++;	//Update NAK Counter

			return ECL_LV1_SUCCESS;
		}
	}
	else	//Correct
	{
		PCD_DEP_Send_R_ACK();
		
		return ECL_LV1_SUCCESS;
	}
	
	return ECL_LV1_FAIL;
}


UCHAR PCD_DEP_Send_S(void)
{
	UCHAR	crdType=0;
	
	//Configure PCB
	pcd_sndData[0]=PCD_BLOCK_S_WTX;

	//Information Field
	pcd_sndData[1]=pcd_parWTXM;

	//Send DEP Process
	PCD_Get_Card_Type(&crdType);

#ifdef PCD_PLATFORM_CLRC663
	NXP_DEP_Send(crdType, 2, pcd_sndData, (pcd_parFWT_TEMP+PCD_PARAMETER_FWT_DELTA+PCD_PARAMETER_T_DELTA));
#else
	PNQ_DEP_Send(crdType, 2, pcd_sndData, (pcd_parFWT_TEMP+PCD_PARAMETER_FWT_DELTA+PCD_PARAMETER_T_DELTA));
#endif

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_DEP_Send(UCHAR blkType, UINT sndLen, UCHAR *sndData)
{
	UCHAR rspCode=0;

	//Clear Send Data Buffer
	memset(pcd_sndData, 0, PCD_BUFFER_SIZE_SEND);
	
	switch (blkType)
	{
		case PCD_BLOCK_TYPE_I:	rspCode=PCD_DEP_Send_I(sndLen, sndData);	break;
		case PCD_BLOCK_TYPE_R:	rspCode=PCD_DEP_Send_R();					break;
		case PCD_BLOCK_TYPE_S:	rspCode=PCD_DEP_Send_S();					break;
		default:				rspCode=ECL_LV1_FAIL;						break;
	}

#ifdef DTE_FUNCTION_ENABLE
	DTE_Upload_Data(sndLen, sndData, 'C');
#endif

	return rspCode;
}


UCHAR PCD_DEP_Check_Protocol_BlockSize(UINT rcvSize)
{
	if ((rcvSize+2) > PCD_PARAMETER_FSD)	//Block Size=[PCB+INF](rcvSize)+CRC(2)
		return ECL_LV1_FAIL;
	
	return ECL_LV1_SUCCESS;
}


UCHAR PCD_DEP_Check_Protocol_PcbFormat(UCHAR rcvPCB)
{
	UCHAR tmpPCB=0;		//Temporary PCB
	UCHAR mskI=0xEE;	//I-block Mask
	UCHAR mskR=0xEE;	//R-block Mask
	UCHAR mskS=0xC7;	//S-block Mask

	//Get PCB Type
	tmpPCB=rcvPCB & 0xC0;

	switch (tmpPCB)
	{
		case PCD_BLOCK_I_BLOCK:
			tmpPCB=rcvPCB & mskI;
			if (tmpPCB != 0x02)	//I-block Format Error
				return ECL_LV1_FAIL;

			break;

		case PCD_BLOCK_R_BLOCK:
			tmpPCB=rcvPCB & mskR;
			if (tmpPCB != 0xA2)	//R-block Format Error
				return ECL_LV1_FAIL;

			break;

		case PCD_BLOCK_S_BLOCK:
			tmpPCB=rcvPCB & mskS;
			if (tmpPCB != 0xC2)	//S-block Format Error
				return ECL_LV1_FAIL;

			break;

		default:
			return ECL_LV1_FAIL;
	}

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_DEP_Check_Protocol_BlockNumber(UCHAR rcvPCB)
{
	UCHAR tmpPCB=0;	//Temporary PCB
	UCHAR sndPCB=0;	//Send PCB

	//Get PCB Type
	tmpPCB=rcvPCB & 0xC0;

	if (tmpPCB == PCD_BLOCK_S_BLOCK)		//S-block don't Check Block Number
	{
		return ECL_LV1_SUCCESS;
	}
	else if (tmpPCB == PCD_BLOCK_R_BLOCK)
	{
		//Get Last Send Block PCB
		sndPCB=pcd_sndData[0] & 0xFE;

		//Get R-block Type
		tmpPCB=rcvPCB & 0xFE;

		if ((sndPCB == PCD_BLOCK_R_NAK) && (tmpPCB == PCD_BLOCK_R_ACK))
		{
			if (pcd_flgChaining == PCD_CHAINING_NONE)
			{
				tmpPCB=rcvPCB & 0x01;
				if (tmpPCB == pcd_blkNumber)
					return ECL_LV1_FAIL;
			}
			
			return ECL_LV1_SUCCESS;
		}
	}

	//Get Receive Block Number
	tmpPCB=rcvPCB & 0x01;
	if (tmpPCB != pcd_blkNumber)	//Block Number Error
		return ECL_LV1_FAIL;

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_DEP_Check_Protocol_ReceivePolicy(UCHAR rcvPCB)
{
	UCHAR tmpPCB=0;
	UCHAR sndPCB=0;	//Send PCB

	//Check Receive Block
	tmpPCB=rcvPCB & 0xF2;
	if (tmpPCB == PCD_BLOCK_S_WTX)	//Don't Check S_WTX
		return ECL_LV1_SUCCESS;

	//Get Last Send PCB
	sndPCB=pcd_sndData[0] & 0xFE;

	switch (sndPCB)
	{
		case PCD_BLOCK_I_NORMAL:
			tmpPCB=rcvPCB & 0xC0;
			if (tmpPCB == PCD_BLOCK_R_BLOCK)
				return ECL_LV1_FAIL;

			break;

		case PCD_BLOCK_I_CHAINING:
			tmpPCB=rcvPCB & 0xF2;
			if (tmpPCB != PCD_BLOCK_R_ACK)
				return ECL_LV1_FAIL;
			
			break;

		case PCD_BLOCK_R_ACK:
			tmpPCB=rcvPCB & 0xC0;
			if (tmpPCB != PCD_BLOCK_I_BLOCK)
				return ECL_LV1_FAIL;

			break;			

		case PCD_BLOCK_S_WTX:
		case PCD_BLOCK_R_NAK:
			tmpPCB=rcvPCB & 0xF2;
			if (tmpPCB == PCD_BLOCK_R_NAK)
				return ECL_LV1_FAIL;

			break;

		default:
			return ECL_LV1_FAIL;
	}

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_DEP_Check_Protocol(UINT rcvSize, UCHAR *rcvBuff)
{
	UCHAR rspCode=0;

	rspCode=PCD_DEP_Check_Protocol_BlockSize(rcvSize);
	if (rspCode == ECL_LV1_FAIL)
	{
		pcd_flgProError=PCD_ERROR_BLOCKSIZE;
		return ECL_LV1_FAIL;
	}

	rspCode=PCD_DEP_Check_Protocol_PcbFormat(rcvBuff[0]);
	if (rspCode == ECL_LV1_FAIL)
	{
		pcd_flgProError=PCD_ERROR_PCBFORMAT;
		return ECL_LV1_FAIL;
	}

	rspCode=PCD_DEP_Check_Protocol_BlockNumber(rcvBuff[0]);
	if (rspCode == ECL_LV1_FAIL)
	{
		pcd_flgProError=PCD_ERROR_BLOCKNUMBER;
		return ECL_LV1_FAIL;
	}

	rspCode=PCD_DEP_Check_Protocol_ReceivePolicy(rcvBuff[0]);
	if (rspCode == ECL_LV1_FAIL)
	{
		pcd_flgProError=PCD_ERROR_RECEIVEPOLICY;
		return ECL_LV1_FAIL;
	}

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_DEP_Check(UINT rcvSize, UCHAR *rcvBuff)
{
	UCHAR rspCode=0;

//Transmission Error Has Been Checked in Receiving Process
/*	rspCode=PCD_DEP_Check_Transmission();
	if (rspCode == ECL_LV1_FAIL)
		return ECL_LV1_FAIL;
*/
	rspCode=PCD_DEP_Check_Protocol(rcvSize, rcvBuff);
	if (rspCode == ECL_LV1_FAIL)
		return ECL_LV1_FAIL;

	return ECL_LV1_SUCCESS;
}


UCHAR PCD_DEP_Receive_I(UINT rcvSize, UCHAR *rcvBuff)
{
	UCHAR tmpPCB=0;	//Temporary PCB
	UCHAR rspCode=0;

	//Configure Chaining Flag
	tmpPCB=rcvBuff[0] & 0x10;
	if (tmpPCB == 0x10)
	{
		pcd_flgChaining=PCD_CHAINING_PICC;	//PICC Chaining
		rspCode=PCD_BLOCK_TYPE_R;			//Response
	}
	else
	{
		pcd_flgChaining=PCD_CHAINING_NONE;	//Non-Chaining
		rspCode=ECL_LV1_FAIL;				//End DEP
	}

	//Update Receive Pointer
	if ((pcd_rcvSize+(rcvSize-1)) <= PCD_BUFFER_SIZE_RECEIVE)
	{
		pcd_rcvSize+=(rcvSize-1);			//Delete One Byte PCB
		memcpy(pcd_ptrRcv, &rcvBuff[1], (rcvSize-1));
		pcd_ptrRcv+=(rcvSize-1);
	}
	else
	{
		pcd_flgProError=PCD_ERROR_OVERFLOW;	//Receive Data Exceed Buffer
		return ECL_LV1_FAIL;
	}

	//Update Block Number & Reset Counter
	PCD_DEP_Update_BlockNumber();
	PCD_DEP_Reset_Counter();

	//Check WTX
	if (pcd_flgWTX == TRUE)
	{
		pcd_flgWTX=FALSE;
	}

	return rspCode;
}


UCHAR PCD_DEP_Receive_R(UCHAR *rcvBuff)
{
	UCHAR tmpPCB=0;
	UCHAR rspCode=0;

	tmpPCB=rcvBuff[0] & 0xFE;
	if (tmpPCB == PCD_BLOCK_R_ACK)
	{
		tmpPCB=rcvBuff[0] & 0x01;		//Get PCB Block Number

		if (pcd_blkNumber == tmpPCB)	//Current BlkNo = Receive BlkNo
		{	
			if (pcd_flgChaining == PCD_CHAINING_PCD)
			{
				pcd_sndSize-=pcd_parFrameSize_Send;
				pcd_ptrSnd+=pcd_parFrameSize_Send;
			}
			
			//Update Block Number & Reset Counter
			PCD_DEP_Update_BlockNumber();
			PCD_DEP_Reset_Counter();			
		}
		else
		{
			pcd_cntResend++;	//Update Resend Counter
			pcd_cntNAK=0;		//Reset NAK Counter
		}

		rspCode=PCD_BLOCK_TYPE_I;
	}

	return rspCode;
}


UCHAR PCD_DEP_Receive_S(UCHAR *rcvBuff)
{
	UCHAR rspCode=ECL_LV1_FAIL;
	UCHAR tmpWTXM=0;

	if ((rcvBuff[0] & 0x30) == 0x30)	//WTX
	{
		if ((rcvBuff[1] & 0xC0) == 0)
		{
			tmpWTXM=rcvBuff[1] & 0x3F;
			if ((tmpWTXM >= 1) && (tmpWTXM <= 59))
			{
				pcd_parWTXM=tmpWTXM;
				pcd_parFWT_TEMP=PCD_Get_FWT_TEMP(pcd_parFWT, pcd_parWTXM);

				rspCode=PCD_BLOCK_TYPE_S;
			}
			else
			{
				pcd_flgProError=PCD_ERROR_PROTOCOL;
			}
		}
		else
		{
			pcd_flgProError=PCD_ERROR_PROTOCOL;
		}
	}
	else
	{
		pcd_flgProError=PCD_ERROR_PROTOCOL;
	}

	return rspCode;
}


UCHAR PCD_DEP_Receive(void)
{
	UINT	rcvSize=0;
	UCHAR	rdrBuff[PCD_BUFFER_SIZE_TEMPORARY]={0};
	UCHAR	blkType=0;		//Block Type
	UCHAR	rspCode=0;
	UCHAR	crdType=0;

	//Reset Error
	PCD_DEP_Reset_Error();
	
	//Clear Receive Data Buffer
	memset(rdrBuff, 0, PCD_BUFFER_SIZE_TEMPORARY);

	//Check Receive Data
#ifdef PCD_PLATFORM_CLRC663
	PCD_Get_Card_Type(&crdType);
	rspCode=NXP_DEP_Receive(crdType, &rcvSize, rdrBuff);
#else
	crdType=crdType;
	rspCode=PNQ_DEP_Receive(&rcvSize, rdrBuff);
#endif
	if (rspCode == ECL_LV1_SUCCESS)
	{
		if (rcvSize <= PCD_BUFFER_SIZE_TEMPORARY)
		{
			rspCode=PCD_DEP_Check(rcvSize, rdrBuff);
			if (rspCode == ECL_LV1_SUCCESS)
			{
				switch (rdrBuff[0] & 0xC0)
				{
					case PCD_BLOCK_I_BLOCK:	blkType=PCD_DEP_Receive_I(rcvSize, rdrBuff);	break;
					case PCD_BLOCK_R_BLOCK:	blkType=PCD_DEP_Receive_R(rdrBuff);				break;
					case PCD_BLOCK_S_BLOCK:	blkType=PCD_DEP_Receive_S(rdrBuff);				break;
					default:				blkType=ECL_LV1_FAIL;							break;
				}

#ifdef DTE_FUNCTION_ENABLE
				DTE_Upload_Data(rcvSize, rdrBuff, 'R');
#endif
			}
			else		//Protocol or Transmission Error
			{
				blkType=ECL_LV1_FAIL;
			}
		}
		else
		{
			pcd_flgTraError=PCD_ERROR_OVERFLOW;	//Receive Data Exceed Buffer
			blkType=ECL_LV1_FAIL;
		}
	}
	else if (rspCode == ECL_LV1_TIMEOUT_ISO)
	{
		pcd_flgTimeout=TRUE;

		blkType=PCD_BLOCK_TYPE_R;
	}
	else if (rspCode == ECL_LV1_ERROR_TRANSMISSION)
	{
		pcd_flgTraError=TRUE;

		blkType=PCD_BLOCK_TYPE_R;
	}
	else	//Unknown Error
	{
		pcd_flgTraError=TRUE;
		
		blkType=ECL_LV1_FAIL;
	}

	return blkType;	//Indicate Next Send Block Type
}


UCHAR PCD_DEP_WithTimeout(UINT sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData, ULONG iptTimeout)
{
	UCHAR rspCode=0;				//Response Code
	UCHAR blkType=PCD_BLOCK_TYPE_I;	//Block Type: I-block(Default)
	UCHAR flgStop=FALSE;			//Stop Flag
	UCHAR flgTimer=TRUE;			//Timer Flag
	ULONG tmrTick=0;
	ULONG tmrValue=0xFFFFFFFF;
	ULONG tmrStart=0;
	UCHAR tmpRcvData[PCD_BUFFER_SIZE_RECEIVE];

	//Allocate Memory
	rspCode=PCD_Allocate_Memory();
	if (rspCode == ECL_LV1_FAIL) return ECL_LV1_ERROR;

	//Reset Parameter
	PCD_DEP_Reset(sndLen, sndData);
	rspCode=ITR_Reset_Flags();

	//Configure Timer
	(iptTimeout == 0)?(flgTimer=FALSE):(tmrValue=iptTimeout);
	tmrStart=OS_GET_SysTimerFreeCnt();

	do
	{
		rspCode=PCD_DEP_Send(blkType, pcd_sndSize, pcd_ptrSnd);
		if (rspCode == ECL_LV1_FAIL) break;

		blkType=PCD_DEP_Receive();
		if (blkType == ECL_LV1_FAIL) break;

		if (flgStop == ECL_LV1_FAIL)
		{
			flgStop=PCD_Check_Interrupt();
			if ((flgStop != ECL_LV1_FAIL) && (flgStop != ECL_LV1_STOP_LATER)) break;
		}

		if (flgTimer == TRUE) tmrTick=OS_GET_SysTimerFreeCnt()-tmrStart;
	} while (tmrTick < tmrValue);

	//Replicate data
	memcpy(tmpRcvData, pcd_rcvData, pcd_rcvSize);
	
	//Free Memory
	PCD_Free_Memory();

	//Return Stop response code
	if (flgStop != ECL_LV1_FAIL)
	{
		if (flgStop == ECL_LV1_STOP_LATER)
		{
			rcvLen[0]=pcd_rcvSize;
			memcpy(rcvData, tmpRcvData, pcd_rcvSize);
		}

		return flgStop;
	}
	
	//Check Error
	if (tmrTick >= tmrValue)		return ECL_LV1_TIMEOUT_USER;
	if (pcd_flgTimeout == TRUE)		return ECL_LV1_TIMEOUT_ISO;
	if (pcd_flgTraError == TRUE)	return ECL_LV1_ERROR_TRANSMISSION;
	if (pcd_flgProError != FALSE)	return ECL_LV1_ERROR_PROTOCOL;
	
	//Return Data
	rcvLen[0]=pcd_rcvSize;
	memcpy(rcvData, tmpRcvData, pcd_rcvSize);

	return ECL_LV1_SUCCESS;	
}


UCHAR PCD_DEP(UINT sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData)
{
	return (PCD_DEP_WithTimeout(sndLen, sndData, rcvLen, rcvData, 0));
}

