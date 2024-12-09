#include <string.h>
#include <stdlib.h>
#include "POSAPI.h"
#include "Function.h"
#include "Datastructure.h"
#include "Define.h"
#include "ECL_Tag.h"
#include "MPP_Define.h"
#include "MPP_Function.h"
#include "MPP_Tag.h"
#include "MPP_TagOf.h"
#include "ECL_LV1_Function.h"

// Add by Wayne 2020/08/21 to avoid compiler warning
#include "DBG_Function.h"

//		VISA L3 Parameter
extern	UCHAR	SchemeID;

//		ECL_CAPK.c
extern	CAPK	glv_CAPK[CAPK_NUMBER];

//		ETP_EntryPoint.c
extern	UCHAR	etp_depRspCode;
extern	UCHAR	etp_depSW[2];


//		Proprietary Flag
UCHAR	mpp_flgTA			=FALSE;		//Flag of TA Debug
UCHAR	mpp_flgDBG			=FALSE;		//Flag of MPP_DBG_Function Debug

//		Process Flag
UCHAR	mpp_flgMisPDOL		=FALSE;		//Flag of Missing PDOL Data
UCHAR	mpp_flgParResult	=FALSE;		//Flag of Parsing Result
UCHAR	mpp_flgSigned		=FALSE;		//Flag of Signed Active AFL
UCHAR	mpp_flgVfySDAD		=FALSE;		//Flag of Verify Signed Dynamic Application Data
UCHAR	mpp_flgOddCVM		=FALSE;		//Flag of Odd CVM List

//		PayPass Queue
UCHAR	mpp_Queue[MPP_QUEUE_SIZE]={0};	//Queue
UCHAR	mpp_queSigNumber=0;				//Number of Signal in Queue

//		PayPass Signal
UCHAR	mpp_strSignal=MPP_SIGNAL_ACT;	//Kernel Start Signal
UCHAR	mpp_Signal=0;					//Signal

//		PayPass Present List & TLV DB
//UCHAR	mpp_bufPresent[3+MPP_TLVLIST_BUFFERSIZE]={0};
//UCHAR	mpp_bufTlvDB[3+MPP_TLVLIST_BUFFERSIZE]={0};
//ECL_TAG mpp_lstPresent	={mpp_bufPresent,	3+mpp_bufPresent};
//ECL_TAG mpp_lstTlvDB	={mpp_bufTlvDB,		3+mpp_bufTlvDB};
UCHAR	*mpp_bufPresent=NULLPTR;
UCHAR	*mpp_bufTlvDB=NULLPTR;
ECL_TAG mpp_lstPresent;
ECL_TAG mpp_lstTlvDB;

//		GAC buffer: CDOL + DSDOL
//UCHAR mpp_tempBuff[ECL_LENGTH_DF8107 + ECL_LENGTH_9F5B + 3] = { 0 };
UCHAR	*mpp_tempBuff=NULLPTR;
ECL_TAG mpp_tempSend;// = { mpp_tempBuff, mpp_tempBuff + 3 };

//		Read Record Parameter
UCHAR	mpp_rrcNumber=0;					//Record Number
UCHAR	mpp_rrcSFI=0;						//SFI

//		MPP DEP Buffer
UINT	mpp_depRcvLen=0;
//UCHAR	mpp_depRcvData[ETP_BUFFER_SIZE] = { 0 };
UCHAR	*mpp_depRcvData=NULLPTR;
UCHAR	mpp_depRspCode=0;

//		M/Chip Read Record Buffer
UINT	mpp_rrcRcvLen=0;
//UCHAR	mpp_rrcRcvData[ETP_BUFFER_SIZE]={0};
UCHAR	*mpp_rrcRcvData=NULLPTR;

//		M/Chip Get Data Buffer
UINT	mpp_gdcRcvLen=0;
//UCHAR	mpp_gdcRcvData[ETP_BUFFER_SIZE]={0};
UCHAR	*mpp_gdcRcvData=NULLPTR;

//		Verify SDAD Buffer
UINT	mpp_vfyDataLen=0;					//Length of Data for Verify Signed Dynamic Application Data
//UCHAR	mpp_vfyData[ETP_BUFFER_SIZE]={0};	//Data for Verify Signed Dynamic Application Data
UCHAR	*mpp_vfyData=NULLPTR;

//		Torn Transaction Record
//TORNREC	mpp_trnRec[MPP_TORN_AIDNUMBER][MPP_TORN_RECORDNUMBER]={{{0,{0},0,{0},0,{0}}}};
TORNREC	mpp_trnRec[MPP_TORN_AIDNUMBER][MPP_TORN_RECORDNUMBER];
UCHAR	mpp_trnRecNum[MPP_TORN_AIDNUMBER]={0};		//Number of records in Torn Transaction Log

//		Sync Data (Assume It Is a TLV Format)
UINT	mpp_synDatLen=0;							//Length of Sync Data
UCHAR	*mpp_synData=NULLPTR;						//Sync Data

//		Timer3
UCHAR	mpp_dhn_tim3;
ULONG	mpp_tick1 = 0, mpp_tick2 = 0;

//		Time Taken from CA(Exchange Relay Resistance Data) to RA(Exchange Relay Resistance Data)
ULONG	mpp_timeTaken = 0;	//timeTaken is expressed in units of tens of microseconds

//		Clean All Flag
UCHAR	mpp_flagCleanAll = FALSE;		//Tammy 2017/11/09

// DataExchange Test william 2017/10/02
//UCHAR	DE_xmlBuf[DE_sizeXML];		// .xml file can up to 34KB => estimate a single DEK 1024 btye
UCHAR	*mpp_ptrDE_xmlBuf=NULLPTR;	//Buffer of DE XML data
UINT	mpp_lenDE_xmlBuf = 0;		//Length of DE XML data

//UCHAR	mpp_DETRcvBuff[MPP_DET_BUFFER_RECEIVE];
UCHAR	*mpp_DETRcvBuff=NULLPTR;	//Buffer of DET receive data
UINT	mpp_lenDETRcvBuff = 0;		//Length of DET receive data

// DataExchange log buffer
//UCHAR	mpp_DE_log[MPP_DE_LOG_SIZE] = { 0 };
UCHAR	*mpp_DE_log=NULLPTR;		//Buffer of DE log
UCHAR   *mpp_ptrDE_log=NULLPTR;		//Pointer of DE log


extern	void	ETP_MPP_OutcomeProcess(void);


UCHAR MPP_S1_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_ACT)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_1, 1, rspCode);
	
	return rspCode;
}


UCHAR MPP_S1_2(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_1, 2, rspCode);
	
	return rspCode;
}


void MPP_S1_3(void)
{
	UCHAR	tlvDF8115[3+3+ECL_LENGTH_DF8115]={0};
	
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_Stop;

	MPP_Initialize_Tag(mpp_tofFF8106);

	MPP_GetTLV(mpp_tofDF8115, tlvDF8115);
	MPP_AddToList(tlvDF8115, glv_tagFF8106.Value, MPP_LISTTYPE_TLV);

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_1, 3, 0xFF);
}


UCHAR MPP_S1_4(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_CLEAN)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_1, 4, rspCode);
	
	return rspCode;
}


void MPP_S1_5(void)
{
	UCHAR	flgIsKnown=FALSE;			//Flag of IsKnown
	UCHAR	flgIsPresent=FALSE;			//Flag of IsPresent
	UCHAR	flgIncACT=FALSE;			//Flag of Update Conditions of Include ACT signal 
	UINT	recIndex=0;					//Record Index
	UCHAR	bcdDatTime[6]={0};			//BCD Format of Date Time(YYMMDDHHMMSS)
	UCHAR	ascDatTime[12]={0};			//Ascii Format of Date Time
	UINT	lenOfSyncData=0;			//Length of Sync Data
	UINT	lenOfData=0;				//Length of Processing Data
	UCHAR	lenOfT=0;					//Length of TLV-T
	UCHAR	lenOfL=0;					//Length of TLV-L
	UINT	lenOfV=0;					//Length of TLV-V
	UCHAR	idxAID=0xFF;				//Index of Torn Record AID
	UCHAR	*ptrData=NULLPTR;			//Pointer of Processing Data
	UCHAR	tlvFF8101[3+3+1024]={0};	//TLV Buffer of FF8101
	UCHAR	idxNum=0;
	UCHAR	rspCode=FALSE;


	//Get Sync Data TLV Length
	rspCode=UT_Get_TLVLength(mpp_synData, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == SUCCESS)
	{
		//Set Pointer to Sync Data TLV-V
		ptrData=mpp_synData+lenOfT+lenOfL;
		lenOfSyncData=lenOfV;
		
		do {
			//Reset Flag
			flgIsKnown=FALSE;
			flgIsPresent=FALSE;
			flgIncACT=FALSE;

			UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

			flgIsKnown=MPP_IsKnown(ptrData);
			flgIsPresent=MPP_IsPresent(ptrData);		

			rspCode=UT_Search_Record(lenOfT, ptrData, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
			if (rspCode == TRUE)
			{
				if (glv_tagTable[recIndex].MASTER_UC & MASTER_UC_ACT)
					flgIncACT=TRUE;
			}

			if (((flgIsKnown == TRUE) || (flgIsPresent == TRUE)) &&	(flgIncACT == TRUE))
			{
				if (rspCode == TRUE)
				{
					//Store Tag Data
					MPP_Store_Tag(lenOfL, lenOfV, ptrData+lenOfT, recIndex);
				}
				else
				{
					//Store to Temporary TLV DB
					MPP_AddToList(ptrData, mpp_lstTlvDB.Value, MPP_LISTTYPE_TLV);
				}
			}
			
			//Point to Next Tag
			ptrData+=(lenOfT+lenOfL+lenOfV);
			lenOfData+=(lenOfT+lenOfL+lenOfV);
		} while (lenOfData < lenOfSyncData);
	}

	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;

	//Remove Old Records From Torn TransactionLog
	idxAID=MPP_Get_TornRecordAIDIndex();

//EVAL 3M50-6042(Managing_CLEAN_Signal)
//The default Torn AID is MasterCard
idxAID=0;	//MasterCard AID

	if (idxAID != 0xFF)
	{
		for (idxNum=0; idxNum < MPP_TORN_RECORDNUMBER; idxNum++)
		{
			if (mpp_trnRec[idxAID][idxNum].PANLen != 0)
			{
				memcpy(bcdDatTime, mpp_trnRec[idxAID][idxNum].DateTime, 6);
				UT_Split(ascDatTime, bcdDatTime, 6);

				rspCode=MPP_Check_TornTransactionExpire(ascDatTime);
				if ((rspCode == TRUE) || (mpp_flagCleanAll == TRUE))	//Tammy 2017/11/09
				{
					//Copy Record to Torn Record					
					if (mpp_trnRec[idxAID][idxNum].RecLen <= ECL_LENGTH_FF8101)
					{
						UT_Set_TagLength(mpp_trnRec[idxAID][idxNum].RecLen, glv_tagFF8101.Length);
						memcpy(glv_tagFF8101.Value, mpp_trnRec[idxAID][idxNum].Record, mpp_trnRec[idxAID][idxNum].RecLen);
					}

					MPP_Initialize_Tag(mpp_tofFF8106);	

					lenOfT=3;
					UT_Get_TLVLengthOfL(glv_tagFF8101.Length, &lenOfL);
					UT_Get_TLVLengthOfV(glv_tagFF8101.Length, &lenOfV);
					memcpy(tlvFF8101, mpp_tofFF8101, lenOfT);
					memcpy(&tlvFF8101[lenOfT], glv_tagFF8101.Length, lenOfL);
					memcpy(&tlvFF8101[lenOfT+lenOfL], glv_tagFF8101.Value, lenOfV);
	
					MPP_AddToList(tlvFF8101, glv_tagFF8106.Value, MPP_LISTTYPE_TLV);

					MPP_Remove_TornRecord(idxNum);

					MPP_OUT(mpp_tofDF8129);
					MPP_OUT(mpp_tofFF8106);
				}
			}
		}
	}

	//Tammy 2017/11/09	Reset Clean All Flag
	mpp_flagCleanAll = FALSE;

	MPP_DBG_Put_Process(MPP_STATE_1, 5, 0xFF);
}


void MPP_S1_6(void)
{
	MPP_Initialize_Tag(mpp_tofFF8106);

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_1, 6, 0xFF);
}


UCHAR MPP_S1_7(void)
{
	UCHAR	flgIsKnown=FALSE;			//Flag of IsKnown
	UCHAR	flgIsPresent=FALSE;			//Flag of IsPresent
	UCHAR	flgIncACT=FALSE;			//Flag of Update Conditions of Include ACT signal 
	UCHAR	flgIsNotPresent=FALSE;		//Flag of IsNotPresent
	UCHAR	flgIsEmpty=FALSE;			//Flag of IsEmpty
	UCHAR	flgIsNotEmpty=FALSE;		//Flag of IsNotEmpty
	UINT	recIndex=0;					//Record Index
	UINT	lenOfData=0;				//Length of Processing Data
	UCHAR	lenOfT=0;					//Length of TLV-T
	UCHAR	lenOfL=0;					//Length of TLV-L
	UINT	lenOfV=0;					//Length of TLV-V
	UCHAR	*ptrData=NULLPTR;			//Pointer of Processing Data
	UCHAR	flgFail=FALSE;
	UCHAR	rspCode=FALSE;
	UINT	padLen = 0;
	UINT	index = 0;

	//Set Pointer to Sync Data
	ptrData=mpp_synData;

	do {
		rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		if (rspCode == SUCCESS)
		{
			if (ptrData[0] == 0x6F)	//File Control Information Template
			{
				rspCode=MPP_ParseAndStoreCardResponse(mpp_synDatLen+2, ptrData);
				if (rspCode != TRUE)
				{
					glv_tagDF8115.Value[1]=MPP_EID_LV2_ParsingError;

					flgFail=TRUE;
				}
			}
			else
			{
				//Reset Flag
				flgIsKnown=FALSE;
				flgIsPresent=FALSE;
				flgIncACT=FALSE;
				
				flgIsKnown=MPP_IsKnown(ptrData);
				flgIsPresent=MPP_IsPresent(ptrData);

				rspCode=UT_Search_Record(lenOfT, ptrData, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
				if (rspCode == TRUE)
				{
					if (glv_tagTable[recIndex].MASTER_UC & MASTER_UC_ACT)
					{
						flgIncACT=TRUE;
					}
				}

				if (((flgIsKnown == TRUE) || (flgIsPresent == TRUE)) && (flgIncACT == TRUE))
				{
					if (rspCode == TRUE)
					{
						//Store Tag Data
						MPP_Store_Tag(lenOfL, lenOfV, ptrData+lenOfT, recIndex);
					}
					else
					{
						//Store to Temporary TLV DB
						MPP_AddToList(ptrData, mpp_lstTlvDB.Value, MPP_LISTTYPE_TLV);
					}

					if (flgIsNotPresent == TRUE)
					{
						MPP_AddToList(ptrData, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
					}
				}
			}
		}
		else
		{
			flgFail=TRUE;
		}

		if (flgFail == TRUE)
		{
			break;
		}

		//Point to Next Tag
		ptrData+=(lenOfT+lenOfL+lenOfV);
		lenOfData+=(lenOfT+lenOfL+lenOfV);
	} while (lenOfData < mpp_synDatLen);

	if (flgFail == FALSE)
	{
		rspCode=MPP_IsNotEmpty(mpp_tof5F2D);
		if (rspCode == TRUE)
		{
			UT_Get_TLVLengthOfV(glv_tag5F2D.Length, &lenOfV);
			memcpy(&glv_tagDF8116.Value[5], glv_tag5F2D.Value, lenOfV);	//'Language Preference' in User Interface Request Data := Language Preference
			
			//If the length of Language Preference is less than 8 bytes, then pad 'Language Preference' in User Interface Request Data with trailing hexadecimal zeroes to 8 bytes.
			if(lenOfV < 8)
			{
				padLen = 8 - lenOfV;
				index = 5 + lenOfV;
				do
				{
					glv_tagDF8116.Value[index++] = 0x00;
					padLen--;
				} while(padLen != 0);
			}
		}
		
		flgIsNotPresent=MPP_IsNotPresent(mpp_tof84);
		flgIsEmpty=MPP_IsEmpty(mpp_tof84);

		if ((flgIsNotPresent == TRUE) || (flgIsEmpty == TRUE))
		{
			glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

			flgFail=TRUE;
		}

		if (flgFail == FALSE)
		{
			flgIsNotEmpty=MPP_IsNotEmpty(mpp_tof9F5D);
			if (flgIsNotEmpty == TRUE)
			{
				if (glv_tag9F5D.Value[1] & MPP_ACI_SupportForFieldOffDetection)
				{
					glv_tagDF8129.Value[6]=glv_tagDF8130.Value[0];	//'Field Off Request' in Outcome Parameter Set := Hold Time Value
				}
			}
		}
	}

	if (flgFail == TRUE)
	{
		rspCode=FALSE;
	}
	else
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_1, 7, rspCode);
	
	return rspCode;
}


void MPP_S1_8(void)
{
	UCHAR	tlvDF8115[3+3+ECL_LENGTH_DF8115]={0};
	
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_SelectNext;
	glv_tagDF8129.Value[1]=MPP_OPS_START_C;

	MPP_Initialize_Tag(mpp_tofFF8106);

	MPP_GetTLV(mpp_tofDF8115, tlvDF8115);
	MPP_AddToList(tlvDF8115, glv_tagFF8106.Value, MPP_LISTTYPE_TLV);

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_1, 8, 0xFF);
}


void MPP_S1_9(void)
{
	ULONG	rndNumber=0;
	UCHAR	rndBuffer[8]={0};

	//CVM Results := '000000'
	glv_tag9F34.Length[0]=ECL_LENGTH_9F34;
	memset(glv_tag9F34.Value, 0, ECL_LENGTH_9F34);
	MPP_AddToList(mpp_tof9F34, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	mpp_tagACType.Value[0]=MPP_ACT_TC;

	//Terminal Verification Results := '0000000000'
	glv_tag95.Length[0]=ECL_LENGTH_95;
	memset(glv_tag95.Value, 0, ECL_LENGTH_95);
	MPP_AddToList(mpp_tof95, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	//ODA Status := '00'
	mpp_tagODAStatus.Value[0]=0x00;

	//RRP Counter := '00'
	glv_tagDF8307.Length[0] = ECL_LENGTH_DF8307;
	glv_tagDF8307.Value[0] = 0x00;
	MPP_AddToList(mpp_tofDF8307, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	glv_tag9F33.Length[0]=ECL_LENGTH_9F33;
	glv_tag9F33.Value[0]=glv_tagDF8117.Value[0];	//Terminal Capabilities[1] := Card Data Input Capability
	glv_tag9F33.Value[1]=0x00;						//Terminal Capabilities[2] := '00'
	glv_tag9F33.Value[2]=glv_tagDF811F.Value[0];	//Terminal Capabilities[3] := Security Capability

	//Initialize(Static Data To Be Authenticated)
	memset(mpp_tagStaticDataToBeAuthenticated.Length, 0, 3+MPP_LENGTH_StaticDataToBeAuthenticated);	//Initialize(Static Data To Be Authenticated)

	//Generate Unpredictable Number
	rndNumber=api_sys_random(rndBuffer);
	glv_tag9F37.Length[0]=ECL_LENGTH_9F37;
	memcpy(glv_tag9F37.Value, rndBuffer, ECL_LENGTH_9F37);
	MPP_AddToList(mpp_tof9F37, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	MPP_DBG_Put_Process(MPP_STATE_1, 9, 0xFF);
}


void MPP_S1_10(void)
{
	UCHAR	rspCode = FALSE, flagIsPresent = FALSE;
	
	MPP_Initialize_Tag(mpp_tofDF8106);
	MPP_Initialize_Tag(mpp_tofFF8104);
	memset(mpp_tagTagsToReadYet.Value-3, 0, 3+MPP_TLVLIST_BUFFERSIZE);	//Decrease 3 to Point to Length

	rspCode=MPP_IsNotEmptyList(glv_tagDF8112.Value);
	if (rspCode == TRUE)
	{
		MPP_AddListToList(glv_tagDF8112.Value, mpp_tagTagsToReadYet.Value, MPP_LISTTYPE_TAG);
	}


	flagIsPresent = MPP_IsPresent(mpp_tofDF8112);		
	rspCode = MPP_IsEmptyList(glv_tagDF8112.Value);		
	if (rspCode == TRUE && flagIsPresent)		// 2017/11/15 william
	{
		MPP_AddToList(mpp_tofDF8112, glv_tagDF8106.Value, MPP_LISTTYPE_TAG);
	}

	MPP_DBG_Put_Process(MPP_STATE_1, 10, 0xFF);
}


void MPP_S1_11(void)
{
	mpp_flgMisPDOL=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_1, 11, 0xFF);
}


UCHAR MPP_S1_12(void)
{
	UCHAR	lenOfT=0;			//Length of TLV-T
	UCHAR	lenOfL=0;			//Length of TLV-L
	UINT	lenOfPDOL=0;		//Length of PDOL
	UINT	lenOfData=0;		//Length of Processing Data
	UCHAR	*ptrData=NULLPTR;	//Pointer of Processing Data
	UCHAR	rspCode=FALSE;

	//Tammy 2017/11/10
	UCHAR	flgInTagTable = FALSE;	//Flag of Tag is in Tag Table
	UCHAR	flgIncDET = FALSE;		//Flag of Update Conditions of Include DET signal
	UINT	tagIndex = 0;			//Index of Tag

	//Get PDOL Length
	UT_Get_TLVLengthOfV(glv_tag9F38.Length, &lenOfPDOL);

	//Set Pointer to PDOL
	ptrData=glv_tag9F38.Value;

	do {
		flgIncDET = FALSE;	//Reset Flag

		UT_Get_TLVLengthOfT(ptrData, &lenOfT);
		lenOfL=1;

		rspCode=MPP_IsEmpty(ptrData);

		//Tammy 2017/11/10	[SB-195-Errata for EMV Book C-2(Version 2.6)]
		flgInTagTable = UT_Search_Record(lenOfT, ptrData, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &tagIndex);
		if(flgInTagTable == TRUE)
		{
			if(glv_tagTable[tagIndex].MASTER_UC & MASTER_UC_DET)
			{
				flgIncDET = TRUE;
			}
		}

		//Tammy 2017/11/10	[SB-195-Errata for EMV Book C-2(Version 2.6)]
		if((rspCode == TRUE) && (flgIncDET == TRUE))
		{
			mpp_flgMisPDOL=TRUE;

			MPP_AddToList(ptrData, glv_tagDF8106.Value, MPP_LISTTYPE_TAG);
		}

		//Point to Next Tag
		ptrData+=(lenOfT+lenOfL);
		lenOfData+=(lenOfT+lenOfL);
	} while (lenOfData < lenOfPDOL);
	
	if (mpp_flgMisPDOL == TRUE)
	{
		rspCode=TRUE;
	}
	else
	{
		rspCode=FALSE;
	}

	MPP_DBG_Put_Process(MPP_STATE_1, 12, rspCode);

	return rspCode;
}


void MPP_S1_13(void)
{

//	Same As MPP_S2_8

	UINT	lenOfPDOL=0;
	UCHAR	lenDolRelData=0;
	UCHAR	bufDolRelData[MPP_DOL_BUFFERSIZE]={0};

	UT_Get_TLVLengthOfV(glv_tag9F38.Length, &lenOfPDOL);

	MPP_DOL_Get_DOLRelatedData(lenOfPDOL, glv_tag9F38.Value, &lenDolRelData, bufDolRelData);

	UT_Set_TagLength(lenDolRelData, glv_tagDF8111.Length);
	memcpy(glv_tagDF8111.Value, bufDolRelData, lenDolRelData);

	MPP_AddToList(mpp_tofDF8111, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	MPP_DBG_Put_Process(MPP_STATE_1, 13, 0xFF);
}


void MPP_S1_14(void)
{

//	Same As MPP_S2_10

	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	tlv83[1+3+1024]={0};

	UT_Get_TLVLengthOfL(glv_tagDF8111.Length, &lenOfL);
	UT_Get_TLVLengthOfV(glv_tagDF8111.Length, &lenOfV);

	//Construct Tag 83
	tlv83[0]=0x83;
	memcpy(&tlv83[1], glv_tagDF8111.Length, lenOfL);
	memcpy(&tlv83[1+lenOfL], glv_tagDF8111.Value, lenOfV);

	//Tammy 2017/11/29 Update PDOL Related Data with Command data field of the GET PROCESSING OPTIONS command
	UT_Set_TagLength((1 + lenOfL + lenOfV), glv_tagDF8111.Length);
	memcpy(glv_tagDF8111.Value, tlv83, (1 + lenOfL + lenOfV));

	MPP_Clear_DEPBuffer();
	mpp_depRspCode=MPP_APDU_GET_PROCESSING_OPTIONS((1+lenOfL+lenOfV), tlv83, &mpp_depRcvLen, mpp_depRcvData);

	MPP_DBG_Put_Process(MPP_STATE_1, 14, 0xFF);
}


void MPP_S1_15(void)
{
	UCHAR	rspCode=FALSE;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	lenOfT=0;
	UINT	lenOfList=0;
	UINT	lenOfData=0;
	UCHAR	tlvBuffer[1024]={0};
	
	//Set Pointer
	ptrData=mpp_tagTagsToReadYet.Value;

	do {
		UT_Get_TLVLengthOfV(mpp_tagTagsToReadYet.Length, &lenOfList);

		rspCode=MPP_IsNotEmpty(ptrData);
		if (rspCode == TRUE)
		{
			memset(tlvBuffer, 0, 1024);
			MPP_GetTLV(ptrData, tlvBuffer);			
			MPP_AddToList(tlvBuffer, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);		
			MPP_RemoveFromList(ptrData, mpp_tagTagsToReadYet.Value, MPP_LISTTYPE_TAG);
			continue;							
		}

		UT_Get_TLVLengthOfT(ptrData, &lenOfT);

		//Point to Next ListItem
		ptrData+=lenOfT;
		lenOfData+=lenOfT;
	} while (lenOfData < lenOfList);

	MPP_DBG_Put_Process(MPP_STATE_1, 15, 0xFF);
}


void MPP_S1_16(void)
{
	UCHAR	rspCode=FALSE;
	
	glv_tagDF8128.Value[0]=0x00;	//IDS Status := '00'
	glv_tagDF810B.Value[0]=0x00;	//DS Summary Status := '00'
	glv_tagDF810E.Value[0]=0x00;	//Post-Gen AC Put Data Status := '00'
	glv_tagDF810F.Value[0]=0x00;	//Pre-Gen AC Put Data Status := '00'

	// william 2017/11/20 add in present list
	UT_Set_TagLength(ECL_LENGTH_DF8128, glv_tagDF8128.Length);
	UT_Set_TagLength(ECL_LENGTH_DF810B, glv_tagDF810B.Length);
	UT_Set_TagLength(ECL_LENGTH_DF810E, glv_tagDF810E.Length);
	UT_Set_TagLength(ECL_LENGTH_DF810F, glv_tagDF810F.Length);
	MPP_AddToList(mpp_tofDF8128, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	MPP_AddToList(mpp_tofDF810B, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	MPP_AddToList(mpp_tofDF810E, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	MPP_AddToList(mpp_tofDF810F, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);


	memset(glv_tagDF61.Value, 0, ECL_LENGTH_DF61);	//DS Digest H := '0000000000000000'

	memset(mpp_tagTagsToWriteYetAfterGenAC.Value-3, 0, 3+MPP_TLVLIST_BUFFERSIZE);	//Decrease 3 to Point to Length
	memset(mpp_tagTagsToWriteYetBeforeGenAC.Value-3, 0, 3+MPP_TLVLIST_BUFFERSIZE);	//Decrease 3 to Point to Length

	rspCode=MPP_IsNotEmpty(mpp_tofFF8102);
	if (rspCode == TRUE)
	{
		MPP_AddListToList(glv_tagFF8102.Value, mpp_tagTagsToWriteYetBeforeGenAC.Value, MPP_LISTTYPE_TLV);
	}
	
	rspCode=MPP_IsNotEmpty(mpp_tofFF8103);
	if (rspCode == TRUE)
	{
		MPP_AddListToList(glv_tagFF8103.Value, mpp_tagTagsToWriteYetAfterGenAC.Value, MPP_LISTTYPE_TLV);
	}

	rspCode=MPP_IsEmpty(mpp_tofFF8102);
	if (rspCode == TRUE)
	{
		MPP_AddToList(mpp_tofFF8102, glv_tagDF8106.Value, MPP_LISTTYPE_TAG);
	}
	
	rspCode=MPP_IsEmpty(mpp_tofFF8103);
	if (rspCode == TRUE)
	{
		MPP_AddToList(mpp_tofFF8103, glv_tagDF8106.Value, MPP_LISTTYPE_TAG);
	}
	
	MPP_DBG_Put_Process(MPP_STATE_1, 16, 0xFF);
}


UCHAR MPP_S1_17(void)
{
	UCHAR	flgIsNotEmpty=FALSE;
	UCHAR	flgIsPresent=FALSE;
	UCHAR	rspCode=FALSE;

	flgIsNotEmpty=MPP_IsNotEmpty(mpp_tofDF810D);
	flgIsPresent=MPP_IsPresent(mpp_tof9F5C);
	
	if ((flgIsNotEmpty == TRUE) && (flgIsPresent == TRUE))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_1, 17, rspCode);
	
	return rspCode;
}


void MPP_S1_18(void)
{
	UCHAR	tlv9F5D[2+3+ECL_LENGTH_9F5D]={0};
	UCHAR	tlv9F5E[2+3+ECL_LENGTH_9F5E]={0};
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsPresent(mpp_tof9F5E);
	if (rspCode == TRUE)
	{
		MPP_GetTLV(mpp_tof9F5E, tlv9F5E);
		//MPP_AddToList(tlv9F5E, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
		MPP_Update_ListItem2(tlv9F5E, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
	}
	else
	{
		//Add empty DS ID to Data To Send
		memcpy(tlv9F5E, mpp_tof9F5E, 2);
		tlv9F5E[2]=0x00;
		//MPP_AddToList(tlv9F5E, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
		MPP_Update_ListItem2(tlv9F5E, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
	}
	
	rspCode=MPP_IsPresent(mpp_tof9F5D);
	if (rspCode == TRUE)
	{
		MPP_GetTLV(mpp_tof9F5D, tlv9F5D);			// 2017/11/02   all tlv9F5E => tlv9F5D
		//MPP_AddToList(tlv9F5D, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
		MPP_Update_ListItem2(tlv9F5D, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
	}
	else
	{
		//Add empty Application Capabilities Information to Data To Send
		memcpy(tlv9F5D, mpp_tof9F5D, 2);
		tlv9F5D[2]=0x00;
		//MPP_AddToList(tlv9F5D, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
		MPP_Update_ListItem2(tlv9F5D, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
	}

	MPP_DBG_Put_Process(MPP_STATE_1, 18, 0xFF);
}


UCHAR MPP_S1_19(void)
{
	UCHAR	flgIsNotEmpty_ACI=FALSE;
	UCHAR	flgIsNotEmpty_DSID=FALSE;
	UCHAR	flgVersion1=FALSE;
	UCHAR	flgVersion2=FALSE;
	UCHAR	rspCode=FALSE;

	flgIsNotEmpty_ACI=MPP_IsNotEmpty(mpp_tof9F5D);
	flgIsNotEmpty_DSID=MPP_IsNotEmpty(mpp_tof9F5E);

	if ((glv_tag9F5D.Value[0] & 0x0F) == MPP_ACI_DSVersionNumber_Version1)
	{
		flgVersion1=TRUE;
	}

	if ((glv_tag9F5D.Value[0] & 0x0F) == MPP_ACI_DSVersionNumber_Version2)
	{
		flgVersion1=TRUE;
	}

	if ((flgIsNotEmpty_ACI == TRUE) &&						//IsNotEmpty (TagOf (Application Capabilities Information)) AND
		((flgVersion1 == TRUE) || (flgVersion2 == TRUE)) &&	//(('Data Storage Version Number' in Application Capabilities Information = VERSION 1) OR ('Data Storage Version Number' in Application Capabilities Information = VERSION 2)) AND 
		(flgIsNotEmpty_DSID == TRUE))						//IsNotEmpty (TagOf (DS ID))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_1, 19, rspCode);

	return rspCode;
}


void MPP_S1_20(void)
{
	glv_tagDF8128.Value[0]|=MPP_IDS_Read;

	MPP_DBG_Put_Process(MPP_STATE_1, 20, 0xFF);
}


UCHAR MPP_S1_21(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_flgMisPDOL == TRUE)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_1, 21, rspCode);
	
	return rspCode;
}


void MPP_S1_22(void)
{
//	Send DEK(Data To Send, Data Needed) signal
	MPP_DEK(2);


	MPP_Initialize_Tag(mpp_tofFF8104);
	MPP_Initialize_Tag(mpp_tofDF8106);

	MPP_DBG_Put_Process(MPP_STATE_1, 22, 0xDE);
}


void MPP_S1_23(void)
{
//	Start Timer (Time Out Value)

	MPP_DBG_Put_Process(MPP_STATE_1, 23, 0xFF);
}


UCHAR MPP_S2_1(void)
{
	//if (mpp_Signal == MPP_SIGNAL_TIMEOUT)
	if (mpp_Signal != MPP_SIGNAL_DET  &&  mpp_Signal != MPP_SIGNAL_STOP)
		return TRUE;
	
	return FALSE;
}


UCHAR MPP_S2_2(void)
{
	if (mpp_Signal == MPP_SIGNAL_STOP)
		return TRUE;
	
	return FALSE;
}


void MPP_S2_3(void)
{
	UCHAR	tlvDF8115[3+3+ECL_LENGTH_DF8115]={0};
	
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_TimeOut;

	MPP_Initialize_Tag(mpp_tofFF8106);

	MPP_GetTLV(mpp_tofDF8115, tlvDF8115);
	MPP_AddToList(tlvDF8115, glv_tagFF8106.Value, MPP_LISTTYPE_TLV);

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
}


void MPP_S2_4(void)
{
	UCHAR	tlvDF8115[3+3+ECL_LENGTH_DF8115]={0};
	
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_Stop;

	MPP_Initialize_Tag(mpp_tofFF8106);

	MPP_GetTLV(mpp_tofDF8115, tlvDF8115);
	MPP_AddToList(tlvDF8115, glv_tagFF8106.Value, MPP_LISTTYPE_TLV);

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
}


UCHAR MPP_S2_5(void)
{
	if (mpp_Signal == MPP_SIGNAL_DET)
		return TRUE;

	return FALSE;
}


void MPP_S2_6(void)
{
	MPP_UpdateWithDetData(mpp_DETRcvBuff);


	MPP_DBG_Put_Process(MPP_STATE_2, 6, 0xDD);
}


UCHAR MPP_S2_7(void)
{
	UINT	lenOfData=0;		//Length of Processing Data
	UCHAR	lenOfT=0;			//Length of TLV-T
	UCHAR	lenOfL=0;			//Length of TLV-L
	UINT	lenOfPDOL=0;		//Length of PDOL
	UCHAR	*ptrPDOL=NULLPTR;	//Pointer of PDOL
	UCHAR	rspCode=FAIL;

	//Tammy 2017/11/10
	UCHAR	flgInTagTable = FALSE;	//Flag of Tag is in Tag Table
	UCHAR	flgIncDET = FALSE;		//Flag of Update Conditions of Include DET signal
	UINT	tagIndex = 0;			//Index of Tag

	//CLEAR Missing PDOL Data Flag
	mpp_flgMisPDOL=FALSE;

	//Set Pointer to PDOL
	ptrPDOL=glv_tag9F38.Value;
	UT_Get_TLVLengthOfV(glv_tag9F38.Length, &lenOfPDOL);

	do {
		flgIncDET = FALSE;	//Reset flag

		UT_Get_TLVLengthOfT(ptrPDOL, &lenOfT);
		lenOfL=1;

		rspCode=MPP_IsEmpty(ptrPDOL);

		//Tammy 2017/11/10	[SB-195-Errata for EMV Book C-2(Version 2.6)]
		flgInTagTable = UT_Search_Record(lenOfT, ptrPDOL, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &tagIndex);
		if(flgInTagTable == TRUE)
		{
			if(glv_tagTable[tagIndex].MASTER_UC & MASTER_UC_DET)
			{
				flgIncDET = TRUE;
			}
		}

		//Tammy 2017/11/10	[SB-195-Errata for EMV Book C-2(Version 2.6)]
		if((rspCode == TRUE) && (flgIncDET == TRUE))
		{
			//SET Missing PDOL Data Flag
			mpp_flgMisPDOL=TRUE;
		}

		//Point to Next Tag
		ptrPDOL+=(lenOfT+lenOfL);
		lenOfData+=(lenOfT+lenOfL);
	} while (lenOfData < lenOfPDOL);

	if (mpp_flgMisPDOL == TRUE)
		return TRUE;

	return FALSE;
}


void MPP_S2_8(void)
{

//	Same As MPP_S1_13

	UINT	lenOfPDOL=0;
	UCHAR	lenDolRelData=0;
	UCHAR	bufDolRelData[MPP_DOL_BUFFERSIZE]={0};

	UT_Get_TLVLengthOfV(glv_tag9F38.Length, &lenOfPDOL);

	MPP_DOL_Get_DOLRelatedData(lenOfPDOL, glv_tag9F38.Value, &lenDolRelData, bufDolRelData);

	UT_Set_TagLength(lenDolRelData, glv_tagDF8111.Length);
	memcpy(glv_tagDF8111.Value, bufDolRelData, lenDolRelData);
	MPP_AddToList(mpp_tofDF8111, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	MPP_DBG_Put_Process(MPP_STATE_2, 8, 0xFF);
}


void MPP_S2_9(void)
{
//	Stop Timer
}


void MPP_S2_10(void)
{

//	Same As S1_14

	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	tlv83[1+3+1024]={0};

	UT_Get_TLVLengthOfL(glv_tagDF8111.Length, &lenOfL);
	UT_Get_TLVLengthOfV(glv_tagDF8111.Length, &lenOfV);

	//Construct Tag 83
	tlv83[0]=0x83;
	memcpy(&tlv83[1], glv_tagDF8111.Length, lenOfL);
	memcpy(&tlv83[1+lenOfL], glv_tagDF8111.Value, lenOfV);

	//Tammy 2017/11/29 Update PDOL Related Data with Command data field of the GET PROCESSING OPTIONS command
	UT_Set_TagLength((1 + lenOfL + lenOfV), glv_tagDF8111.Length);
	memcpy(glv_tagDF8111.Value, tlv83, (1 + lenOfL + lenOfV));
	
	MPP_Clear_DEPBuffer();
	mpp_depRspCode=MPP_APDU_GET_PROCESSING_OPTIONS((1+lenOfL+lenOfV), tlv83, &mpp_depRcvLen, mpp_depRcvData);

	MPP_DBG_Put_Process(MPP_STATE_2, 10, 0xFF);
}


UCHAR MPP_S3_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_RA)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3, 1, rspCode);
	
	return rspCode;
}


UCHAR MPP_S3_2(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_DET)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3, 2, rspCode);

	return rspCode;
}


void MPP_S3_3(void)
{
	MPP_UpdateWithDetData(mpp_DETRcvBuff);


	MPP_DBG_Put_Process(MPP_STATE_3, 3, 0xDD);
}


UCHAR MPP_S3_4(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3, 4, rspCode);

	return rspCode;
}


void MPP_S3_5(void)
{
	UCHAR	tlvDF8115[3+3+ECL_LENGTH_DF8115]={0};
	
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_TryAgain;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;
	glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(mpp_depRspCode);

	MPP_Initialize_Tag(mpp_tofFF8106);
	
	MPP_GetTLV(mpp_tofDF8115, tlvDF8115);
	MPP_AddToList(tlvDF8115, glv_tagFF8106.Value, MPP_LISTTYPE_TLV);

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_3, 5, 0xFF);
}


UCHAR MPP_S3_6(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3, 6, rspCode);

	return rspCode;
}


void MPP_S3_7(void)
{
	UCHAR	tlvDF8115[3+3+ECL_LENGTH_DF8115]={0};
	
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_Stop;
	
	MPP_Initialize_Tag(mpp_tofFF8106);

	MPP_GetTLV(mpp_tofDF8115, tlvDF8115);
	MPP_AddToList(tlvDF8115, glv_tagFF8106.Value, MPP_LISTTYPE_TLV);

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_3, 7, 0xFF);
}


UCHAR MPP_S3_8(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=UT_Check_SW12(&mpp_depRcvData[mpp_depRcvLen-2], STATUSWORD_9000);

	MPP_DBG_Put_Process(MPP_STATE_3, 8, rspCode);
	
	return rspCode;
}


void MPP_S3_9_1(UCHAR *iptSW)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_StatusBytes;

	//'SW12' in Error Indication := SW12
	memcpy(&glv_tagDF8115.Value[3], iptSW, 2);

	MPP_DBG_Put_Process(MPP_STATE_3, 9, 0xFF);
}


void MPP_S3_9_2(void)
{
	UCHAR	tlvDF8115[3+3+ECL_LENGTH_DF8115]={0};
	
	glv_tagDF8129.Value[6]=MPP_OPS_FieldOffRequest_NA;
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_SelectNext;
	glv_tagDF8129.Value[1]=MPP_OPS_START_C;

	MPP_Initialize_Tag(mpp_tofFF8106);
	
	MPP_GetTLV(mpp_tofDF8115, tlvDF8115);
	MPP_AddToList(tlvDF8115, glv_tagFF8106.Value, MPP_LISTTYPE_TLV);

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_3, 9, 0xFF);
}


void MPP_S3_10(void)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	flgEncError=FALSE;
	UCHAR	rspCode=FALSE;

	//Parsing Result := FALSE
	mpp_flgParResult=FALSE;

	if ((mpp_depRcvLen > 0) && (mpp_depRcvData[0] == 0x77))
	{
		mpp_flgParResult=MPP_ParseAndStoreCardResponse(mpp_depRcvLen, mpp_depRcvData);
	}
	else
	{
		if ((mpp_depRcvLen > 0) && (mpp_depRcvData[0] == 0x80))
		{
			UT_Get_TLVLength(mpp_depRcvData, &lenOfT, &lenOfL, &lenOfV);

			//Check Receive Length = Tag Length + SW12
			if (mpp_depRcvLen != (lenOfT+lenOfL+lenOfV+2))
			{
				flgEncError=TRUE;
			}
			else
			{
				//Store Application Interchange Profile
				if (lenOfV < 6)	//At Least AIP(2) + AFL(4)
				{
					flgEncError=TRUE;
				}
				else
				{
					rspCode=MPP_IsNotEmpty(mpp_tof82);
					if (rspCode == TRUE)
					{
						flgEncError=TRUE;
					}
					else
					{
						glv_tag82.Length[0]=2;
						memcpy(glv_tag82.Value, &mpp_depRcvData[lenOfT+lenOfL], 2);	//Copy from Tag80.Value[0]

						MPP_AddToList(mpp_tof82, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

						//Store Application File Locator
						rspCode=MPP_IsNotEmpty(mpp_tof94);
						if (rspCode == TRUE)
						{
							flgEncError=TRUE;
						}
						else
						{
							//Check AFL is Multiply of 4
							if (((lenOfV-2) % 4) != 0)
							{
								flgEncError=TRUE;
							}
							else
							{
								UT_Set_TagLength(lenOfV-2, glv_tag94.Length);	//Exclude AIP
								memcpy(glv_tag94.Value, &mpp_depRcvData[lenOfT+lenOfL+2], lenOfV-2);	//Copy from Tag80.Value[2]

								MPP_AddToList(mpp_tof94, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
							}
						}
					}
				}
			}
						
			if (flgEncError == FALSE)
			{
				mpp_flgParResult=TRUE;
			}
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_3, 10, 0xFF);
}


UCHAR MPP_S3_11(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_flgParResult == TRUE)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3, 11, rspCode);
	
	return rspCode;
}


void MPP_S3_12(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_ParsingError;

	MPP_DBG_Put_Process(MPP_STATE_3, 12, 0xFF);
}


UCHAR MPP_S3_13(void)
{
	UCHAR	flgIsNotEmpty_AFL=FALSE;
	UCHAR	flgIsNotEmpty_AIP=FALSE;
	UCHAR	rspCode=FALSE;
	
	flgIsNotEmpty_AFL=MPP_IsNotEmpty(mpp_tof94);
	flgIsNotEmpty_AIP=MPP_IsNotEmpty(mpp_tof82);

	if ((flgIsNotEmpty_AFL == TRUE) && (flgIsNotEmpty_AIP == TRUE))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3, 13, rspCode);
	
	return rspCode;
}


void MPP_S3_14(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_DBG_Put_Process(MPP_STATE_3, 14, 0xFF);
}


UCHAR MPP_S3_15(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF811B.Value[0] & MPP_KCF_EMVModeContactlessTransactionsNotSupported)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3, 15, rspCode);
	
	return rspCode;
}


UCHAR MPP_S3_16(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tag82.Value[1] & MPP_AIP_EMVModeIsSupported)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3, 16, rspCode);

	return rspCode;
}


UCHAR MPP_S3_17(void)
{
	UCHAR	rspCode=FALSE;

	if (glv_tagDF811B.Value[0] & MPP_KCF_MagStripeModeContactlessTransactionsNotSupported)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3, 17, rspCode);

	return rspCode;
}


void MPP_S3_18(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_MagStripeNotSupported;

	MPP_DBG_Put_Process(MPP_STATE_3, 18, 0xFF);
}


UCHAR MPP_S3_30(void)
{
	UINT	len94=0;
	UCHAR	dat94[4]={0x08,0x01,0x01,0x00};
	UCHAR	rspCode=FALSE;
	
	MPP_GetLength(mpp_tof94, &len94);

	if ((len94 >= 4) &&																//(GetLength(TagOf(Application File Locator)) >= 4) AND
		(!memcmp(glv_tag94.Value, dat94, 4)) &&										//(Application File Locator[1:4] = '08010100') AND
		((glv_tagDF811B.Value[0] & MPP_KCF_MagStripeModeContactlessTransactionsNotSupported) == 0))	//'Mag-stripe mode contactless transactions not supported' in Kernel Configuration is not set
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_3, 30, rspCode);
	
	return rspCode;
}


void MPP_S3_31(void)
{
	UINT	len94=0;

	MPP_GetLength(mpp_tof94, &len94);

	memcpy(mpp_tagActiveAFL.Length, glv_tag94.Length, 3);
	memcpy(mpp_tagActiveAFL.Value, glv_tag94.Value, len94);

	MPP_DBG_Put_Process(MPP_STATE_3, 31, 0xFF);
}


void MPP_S3_32(void)
{
	UINT	len94=0;
	
	MPP_GetLength(mpp_tof94, &len94);

	UT_Set_TagLength((len94-4), mpp_tagActiveAFL.Length);
	memcpy(mpp_tagActiveAFL.Value, &glv_tag94.Value[4], (len94-4));

	MPP_DBG_Put_Process(MPP_STATE_3, 32, 0xFF);
}


UCHAR MPP_S3_33(void)
{
	UCHAR	rspCode=FALSE;

	if ((glv_tag82.Value[0] & MPP_AIP_OnDeviceCardholderVerificationIsSupported) &&	//'On device cardholder verification is supported' in Application Interchange Profile is set AND
		(glv_tagDF811B.Value[0] & MPP_KCF_OnDeviceCardholderVerificationSupported))	//'On device cardholder verification supported' in Kernel Configuration is set
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_3, 33, rspCode);
	
	return rspCode;
}


void MPP_S3_34(void)
{
	mpp_tagReaderContactlessTransactionLimit.Length[0]=ECL_LENGTH_DF8124;
	memcpy(mpp_tagReaderContactlessTransactionLimit.Value, glv_tagDF8124.Value, ECL_LENGTH_DF8124);

	MPP_DBG_Put_Process(MPP_STATE_3, 34, 0xFF);
}


void MPP_S3_35(void)
{
	mpp_tagReaderContactlessTransactionLimit.Length[0]=ECL_LENGTH_DF8125;
	memcpy(mpp_tagReaderContactlessTransactionLimit.Value, glv_tagDF8125.Value, ECL_LENGTH_DF8125);

	MPP_DBG_Put_Process(MPP_STATE_3, 35, 0xFF);
}

UCHAR MPP_S3_60(void)
{
	UCHAR	rspCode = FALSE;

	if((glv_tagDF811B.Value[0] & MPP_KCF_RelayResistanceProtocolSupported) &&
	   (glv_tag82.Value[1] & MPP_AIP_RelayResistanceProtocolIsSupported))
	{
		rspCode = TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_3, 60, rspCode);
	
	return rspCode;
}


void MPP_S3_61(void)
{
	ULONG	rndNumber=0;
	UCHAR	rndBuffer[8]={0};

	//Generate Unpredictable Number
	rndNumber=api_sys_random(rndBuffer);
	glv_tag9F37.Length[0]=ECL_LENGTH_9F37;
	memcpy(glv_tag9F37.Value, rndBuffer, ECL_LENGTH_9F37);
	MPP_AddToList(mpp_tof9F37, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	UT_Set_TagLength(ECL_LENGTH_DF8301, glv_tagDF8301.Length);
	memcpy(glv_tagDF8301.Value, glv_tag9F37.Value, ECL_LENGTH_DF8301);
	MPP_AddToList(mpp_tofDF8301, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	MPP_DBG_Put_Process(MPP_STATE_3, 61, 0xFF);
}


void MPP_S3_62(void)
{
	//Prepare EXCHANGE RELAY RESISTANCE DATA command
	MPP_DBG_Put_Process(MPP_STATE_3, 62, 0xFF);
}


void MPP_S3_63(void)
{
	//Start Timer

	//Reset Tick
	mpp_tick1 = 0;
	mpp_tick2 = 0;

	mpp_dhn_tim3 = api_tim3_open(10); //1 tick unit = 10 us
	api_tim3_gettick(mpp_dhn_tim3, &mpp_tick1);

	MPP_DBG_Put_Process(MPP_STATE_3, 63, 0xFF);
}


void MPP_S3_64(void)
{
	UINT	lenOfV;
	
	UT_Get_TLVLengthOfV(glv_tagDF8301.Length, &lenOfV);
	
	MPP_Clear_DEPBuffer();
	mpp_depRspCode = MPP_APDU_EXCHANGE_RELAY_RESISTANCE_DATA(lenOfV, glv_tagDF8301.Value, &mpp_depRcvLen, mpp_depRcvData);
	
	MPP_DBG_Put_Process(MPP_STATE_3, 64, 0xFF);
}


void MPP_S3_65(void)
{
	glv_tag95.Value[4] |= MPP_TVR_RRPNotPerformed;

	MPP_DBG_Put_Process(MPP_STATE_3, 65, 0xFF);
}


UCHAR MPP_S3_70(void)
{
	UINT	len94=0;
	UCHAR	dat94[4]={0x08,0x01,0x01,0x00};
	UCHAR	rspCode=FALSE;
	
	MPP_GetLength(mpp_tof94, &len94);

	if ((len94 >= 4) &&	(!memcmp(glv_tag94.Value, dat94, 4)))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3, 70, rspCode);

	return rspCode;
}


void MPP_S3_71(void)
{
	UINT	len94=0;

	MPP_GetLength(mpp_tof94, &len94);

	memcpy(mpp_tagActiveAFL.Length, glv_tag94.Length, 3);
	memcpy(mpp_tagActiveAFL.Value, glv_tag94.Value, len94);

	MPP_DBG_Put_Process(MPP_STATE_3, 71, 0xFF);
}


void MPP_S3_72(void)
{
	mpp_tagActiveAFL.Length[0]=4;
	memcpy(mpp_tagActiveAFL.Value, glv_tag94.Value, 4);

	MPP_DBG_Put_Process(MPP_STATE_3, 72, 0xFF);
}


UCHAR MPP_S3_73(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((glv_tag82.Value[0] & MPP_AIP_OnDeviceCardholderVerificationIsSupported) &&
		(glv_tagDF811B.Value[0] & MPP_KCF_OnDeviceCardholderVerificationSupported))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_3, 73, rspCode);

	return rspCode;
}


void MPP_S3_74(void)
{
	mpp_tagReaderContactlessTransactionLimit.Length[0]=ECL_LENGTH_DF8124;
	memcpy(mpp_tagReaderContactlessTransactionLimit.Value, glv_tagDF8124.Value, ECL_LENGTH_DF8124);

	MPP_DBG_Put_Process(MPP_STATE_3, 74, 0xFF);
}


void MPP_S3_75(void)
{
	mpp_tagReaderContactlessTransactionLimit.Length[0]=ECL_LENGTH_DF8125;
	memcpy(mpp_tagReaderContactlessTransactionLimit.Value, glv_tagDF8125.Value, ECL_LENGTH_DF8125);

	MPP_DBG_Put_Process(MPP_STATE_3, 75, 0xFF);
}


void MPP_S3_76(void)
{
	UCHAR	rspCode=FALSE;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	lenOfT=0;
	UINT	lenOfList=0;
	UINT	lenOfData=0;
	UCHAR	tlvBuffer[1024]={0};
	
	//Set Pointer
	ptrData=mpp_tagTagsToReadYet.Value;

	do {
		UT_Get_TLVLengthOfV(mpp_tagTagsToReadYet.Length, &lenOfList);	

		rspCode=MPP_IsNotEmpty(ptrData);
		if (rspCode == TRUE)
		{
			memset(tlvBuffer, 0, 1024);
			MPP_GetTLV(ptrData, tlvBuffer);			
			MPP_AddToList(tlvBuffer, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
			MPP_RemoveFromList(ptrData, mpp_tagTagsToReadYet.Value, MPP_LISTTYPE_TAG);
			continue;
		}

		UT_Get_TLVLengthOfT(ptrData, &lenOfT);

		//Point to Next ListItem
		ptrData+=lenOfT;
		lenOfData+=lenOfT;
	} while (lenOfData < lenOfList);

	MPP_DBG_Put_Process(MPP_STATE_3, 76, 0xFF);
}


UCHAR MPP_S3_77(void)
{
	UCHAR	flgIsNotEmpty_DataNeeded=FALSE;
	UCHAR	flgIsNotEmpty_DataToSend=FALSE;
	UCHAR	flgIsEmpty_TagsToReadYet=FALSE;
	UCHAR	rspCode=FALSE;
	
	flgIsNotEmpty_DataNeeded=MPP_IsNotEmptyList(glv_tagDF8106.Value);
	flgIsNotEmpty_DataToSend=MPP_IsNotEmptyList(glv_tagFF8104.Value);
	flgIsEmpty_TagsToReadYet=MPP_IsEmptyList(mpp_tagTagsToReadYet.Value);

	if ((flgIsNotEmpty_DataNeeded == TRUE)
		||
		((flgIsNotEmpty_DataToSend == TRUE) && 
		(flgIsEmpty_TagsToReadYet == TRUE)))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_3, 77, rspCode);
	
	return rspCode;
}


void MPP_S3_78(void)
{
//	Send DEK(Data To Send, Data Needed) signal
	MPP_DEK(2);

	MPP_Initialize_Tag(mpp_tofFF8104);
	MPP_Initialize_Tag(mpp_tofDF8106);

	MPP_DBG_Put_Process(MPP_STATE_3, 78, 0xDE);
}


void MPP_S3_80(void)
{
	//Build command data for READ RECORD for the first record indicated by Active AFL
	mpp_rrcSFI=(mpp_tagActiveAFL.Value[0] & 0xF8) >> 3;
	mpp_rrcNumber=mpp_tagActiveAFL.Value[1];

	MPP_DBG_Put_Process(MPP_STATE_3, 80, 0xFF);
}


void MPP_S3_81(void)
{
	MPP_Clear_DEPBuffer();
	mpp_depRspCode=MPP_APDU_READ_RECORD(mpp_rrcNumber, mpp_rrcSFI, &mpp_depRcvLen, mpp_depRcvData);

	MPP_DBG_Put_Process(MPP_STATE_3, 81, 0xFF);
}


void MPP_S3_90_1(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_3, 90, 0xFF);
}


void MPP_S3_90_2(void)
{
	UCHAR	tlvDF8115[3+1+ECL_LENGTH_DF8115]={
		0xDF,0x81,0x15,
		0x06,
		0x00,0x00,0x00,0x00,0x00,0x00};
	
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;

	MPP_Initialize_Tag(mpp_tofFF8106);

	memcpy(&tlvDF8115[4], glv_tagDF8115.Value, ECL_LENGTH_DF8115);
	MPP_AddToList(tlvDF8115, glv_tagFF8106.Value, MPP_LISTTYPE_TLV);

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_3, 90, 0xFF);
}


UCHAR MPP_SR1_1(void)
{
	UCHAR	rspCode = FALSE;
	
	if(mpp_Signal == MPP_SIGNAL_DET)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_R1, 1, rspCode);

	return rspCode;
}


void MPP_SR1_2(void)
{
	MPP_UpdateWithDetData(mpp_DETRcvBuff);

	MPP_DBG_Put_Process(MPP_STATE_R1, 2, 0xDD);
}


UCHAR MPP_SR1_3(void)
{
	UCHAR	rspCode = FALSE;
	
	if(mpp_Signal == MPP_SIGNAL_L1RSP)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_R1, 3, rspCode);

	return rspCode;
}


void MPP_SR1_4(void)
{
	//	Stop Timer
	api_tim3_close(mpp_dhn_tim3);

	MPP_DBG_Put_Process(MPP_STATE_R1, 4, 0xFF);
}


void MPP_SR1_5_1(void)
{
	glv_tagDF8116.Value[0] = MPP_UIR_MID_TryAgain;
	glv_tagDF8116.Value[1] = MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_DBG_Put_Process(MPP_STATE_R1, 5, 0xFF);
}


void MPP_SR1_5_2(void)
{
	glv_tagDF8129.Value[0] = MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1] = MPP_OPS_START_B;
	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnRestartPresent;
	glv_tagDF8115.Value[0] = MPP_Get_ErrorIndication_L1(mpp_depRspCode);
	glv_tagDF8115.Value[5] = MPP_EID_MOE_TryAgain;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_R1, 5, 0xFF);
}


UCHAR MPP_SR1_6(void)
{
	UCHAR	rspCode = FALSE;
	
	if(mpp_Signal == MPP_SIGNAL_STOP)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_R1, 6, rspCode);

	return rspCode;
}


void MPP_SR1_7(void)
{
	//	Stop Timer
	api_tim3_close(mpp_dhn_tim3);

	MPP_DBG_Put_Process(MPP_STATE_R1, 7, 0xFF);
}


void MPP_SR1_8(void)
{
	glv_tagDF8129.Value[0] = MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2] = MPP_EID_LV3_Stop;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_R1, 8, 0xFF);
}


UCHAR MPP_SR1_9(void)
{
	UCHAR	rspCode = FALSE;
	
	if(mpp_Signal == MPP_SIGNAL_RA)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_R1, 9, rspCode);
	
	return rspCode;
}


void MPP_SR1_10(void)
{
	//Stop Timer, Compute Time Taken
	api_tim3_gettick(mpp_dhn_tim3, &mpp_tick2);
	mpp_timeTaken = mpp_tick2 - mpp_tick1;	//timeTaken is expressed in units of tens of microseconds
	api_tim3_close(mpp_dhn_tim3);

	MPP_DBG_Put_Process(MPP_STATE_R1, 10, 0xFF);
}


UCHAR MPP_SR1_11(void)
{
	UCHAR	rspCode = FALSE;
	
	rspCode = UT_Check_SW12(&mpp_depRcvData[mpp_depRcvLen - 2], STATUSWORD_9000);
	
	MPP_DBG_Put_Process(MPP_STATE_R1, 11, rspCode);
	
	return rspCode;
}


void MPP_SR1_12(void)
{
	glv_tagDF8116.Value[0] = MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1] = MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_R1, 12, 0xFF);
}


void MPP_SR1_13(UCHAR *iptSW)
{
	glv_tagDF8129.Value[0] = MPP_OPS_STATUS_EndApplication;
	
	glv_tagDF8115.Value[5] = MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8115.Value[1] = MPP_EID_LV2_StatusBytes;
	memcpy(&glv_tagDF8115.Value[3], iptSW, 2);

	MPP_CreateEMVDiscretionaryData();
	
	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_R1, 13, 0xFF);
}


void MPP_SR1_14(void)
{
	UCHAR	lenOfT = 0;
	UCHAR	lenOfL = 0;
	UINT	lenOfV = 0;
	UCHAR	*ptrData = NULLPTR;

	//Parsing Result := FALSE
	mpp_flgParResult = FALSE;

	UT_Get_TLVLength(mpp_depRcvData, &lenOfT, &lenOfL, &lenOfV);

	if((mpp_depRcvLen > 11) && (mpp_depRcvData[0] == 0x80) && (lenOfV == 10))
	{
		ptrData = &mpp_depRcvData[lenOfT + lenOfL];	//Point to the value field of Response Message Data Field

		//Store Device Relay Resistance Entropy
		glv_tagDF8302.Length[0] = 4;
		memcpy(glv_tagDF8302.Value, ptrData, 4);
		ptrData += 4;

		MPP_AddToList(mpp_tofDF8302, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

		//Store Min Time For Processing Relay Resistance APDU
		glv_tagDF8303.Length[0] = 2;
		memcpy(glv_tagDF8303.Value, ptrData, 2);
		ptrData += 2;

		MPP_AddToList(mpp_tofDF8303, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

		//Store Max Time For Processing Relay Resistance APDU
		glv_tagDF8304.Length[0] = 2;
		memcpy(glv_tagDF8304.Value, ptrData, 2);
		ptrData += 2;

		MPP_AddToList(mpp_tofDF8304, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

		//Store Device Estimated Transmission Time For Relay Resistance R-APDU
		glv_tagDF8305.Length[0] = 2;
		memcpy(glv_tagDF8305.Value, ptrData, 2);

		MPP_AddToList(mpp_tofDF8305, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

		mpp_flgParResult = TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_R1, 14, 0xFF);
}


UCHAR MPP_SR1_15(void)
{
	UCHAR	rspCode = FALSE;
	
	if(mpp_flgParResult == TRUE)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_R1, 15, rspCode);
	
	return rspCode;
}


void MPP_SR1_16(void)
{
	glv_tagDF8116.Value[0] = MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1] = MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_R1, 16, 0xFF);
}


void MPP_SR1_17(void)
{
	glv_tagDF8129.Value[0] = MPP_OPS_STATUS_EndApplication;

	glv_tagDF8115.Value[5] = MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8115.Value[1] = MPP_EID_LV2_ParsingError;

	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_R1, 17, 0xFF);
}

void MPP_SR1_18(void)
{
	int		measuredRRProcTime = 0;			//Measured Relay Resistance Processing Time
	int		terminalExpectedTimeCA = 0;		//Terminal Expected Transmission Time For Relay Resistance C-APDU
	int		devicedExpectedTimeRA = 0;		//Device Estimated Transmission Time For Relay Resistance R-APDU
	int		terminalExpectedTimeRA = 0;		//Terminal Expected Transmission Time For Relay Resistance R-APDU
	int		min = 0;

	terminalExpectedTimeCA = (int)(glv_tagDF8134.Value[0] * 256 + glv_tagDF8134.Value[1]);
	devicedExpectedTimeRA = (int)(glv_tagDF8305.Value[0] * 256 + glv_tagDF8305.Value[1]);
	terminalExpectedTimeRA = (int)(glv_tagDF8135.Value[0] * 256 + glv_tagDF8135.Value[1]);

	if(devicedExpectedTimeRA < terminalExpectedTimeRA)
	{
		min = devicedExpectedTimeRA;
	}
	else
	{
		min = terminalExpectedTimeRA;
	}

	//measuredRRProcTime = (timeTaken / 100) - terminalExpectedTimeCA - min;	//timeTaken is expressed in microseconds.
	//Tammy 2017/11/14	[SB-195-Errata for EMV Book C-2(Version 2.6)]
	if(0 < (((int)mpp_timeTaken / 10) - terminalExpectedTimeCA - min))
	{
		measuredRRProcTime = (((int)mpp_timeTaken / 10) - terminalExpectedTimeCA - min);	//timeTaken is expressed in units of tens of microseconds
	}
	else
	{
		measuredRRProcTime = 0;
	}
	UT_Set_TagLength(ECL_LENGTH_DF8306, glv_tagDF8306.Length);
	glv_tagDF8306.Value[0] = (measuredRRProcTime & 0xFF00) >> 8;
	glv_tagDF8306.Value[1] = measuredRRProcTime & 0x00FF;
	MPP_AddToList(mpp_tofDF8306, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	MPP_DBG_Put_Process(MPP_STATE_R1, 18, 0xFF);
}


UCHAR MPP_SR1_19(void)
{
	UCHAR	rspCode = FALSE;
	int		measuredRRProcTime = 0;			//Measured Relay Resistance Processing Time
	int		minTimeProcRR = 0;				//Minimum Time For Processing Relay Resistance APDU
	int		minRRGracePeriod = 0;			//Minimum Relay Resistance Grace Period
	int		max = 0;

	measuredRRProcTime = (int)(glv_tagDF8306.Value[0] * 256 + glv_tagDF8306.Value[1]);
	minTimeProcRR = (int)(glv_tagDF8303.Value[0] * 256 + glv_tagDF8303.Value[1]);
	minRRGracePeriod = (int)(glv_tagDF8132.Value[0] * 256 + glv_tagDF8132.Value[1]);

	if(0 < (minTimeProcRR - minRRGracePeriod))
	{
		max = minTimeProcRR - minRRGracePeriod;
	}
	else
	{
		max = 0;
	}

	//if(measuredRRProcTime < (minTimeProcRR - minRRGracePeriod))
	//Tammy 2017/11/14	[SB-195-Errata for EMV Book C-2(Version 2.6)]
	if(measuredRRProcTime < max)
	{
		rspCode = TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_R1, 19, rspCode);

	return rspCode;
}


void MPP_SR1_20(void)
{
	glv_tagDF8116.Value[0] = MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1] = MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_R1, 20, 0xFF);
}


void MPP_SR1_21(void)
{
	glv_tagDF8129.Value[0] = MPP_OPS_STATUS_EndApplication;

	glv_tagDF8115.Value[5] = MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8115.Value[1] = MPP_EID_LV2_CardDataError;

	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_R1, 21, 0xFF);
}


UCHAR MPP_SR1_22(void)
{
	UCHAR	rspCode = FALSE;
	int		measuredRRProcTime = 0;			//Measured Relay Resistance Processing Time
	int		maxTimeProcRR = 0;				//Max Time For Processing Relay Resistance APDU
	int		maxRRGracePeriod = 0;			//Maximum Relay Resistance Grace Period

	measuredRRProcTime = (int)(glv_tagDF8306.Value[0] * 256 + glv_tagDF8306.Value[1]);
	maxTimeProcRR = (int)(glv_tagDF8304.Value[0] * 256 + glv_tagDF8304.Value[1]);
	maxRRGracePeriod = (int)(glv_tagDF8133.Value[0] * 256 + glv_tagDF8133.Value[1]);

	if((glv_tagDF8307.Value[0] < 2) && (measuredRRProcTime > (maxTimeProcRR + maxRRGracePeriod)))
	{
		rspCode = TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_R1, 22, rspCode);

	return rspCode;
}


void MPP_SR1_23(void)
{
	ULONG	rndNumber=0;
	UCHAR	rndBuffer[8]={0};

	//Generate Unpredictable Number
	rndNumber=api_sys_random(rndBuffer);
	UT_Set_TagLength(ECL_LENGTH_9F37, glv_tag9F37.Length);
	memcpy(glv_tag9F37.Value, rndBuffer, ECL_LENGTH_9F37);
	MPP_AddToList(mpp_tof9F37, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	UT_Set_TagLength(ECL_LENGTH_DF8301, glv_tagDF8301.Length);
	memcpy(glv_tagDF8301.Value, glv_tag9F37.Value, ECL_LENGTH_DF8301);
	MPP_AddToList(mpp_tofDF8301, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	MPP_DBG_Put_Process(MPP_STATE_R1, 23, 0xFF);
}


void MPP_SR1_24(void)
{
	glv_tagDF8307.Value[0]++;

	MPP_AddToList(mpp_tofDF8307, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	MPP_DBG_Put_Process(MPP_STATE_R1, 24, 0xFF);
}


void MPP_SR1_25(void)
{
	//Prepare EXCHANGE RELAY RESISTANCE DATA command
	MPP_DBG_Put_Process(MPP_STATE_R1, 25, 0xFF);
}


void MPP_SR1_26(void)
{
	//Start Timer

	//Reset Tick
	mpp_tick1 = 0;
	mpp_tick2 = 0;

	mpp_dhn_tim3 = api_tim3_open(10); //1 tick unit = 10 us
	api_tim3_gettick(mpp_dhn_tim3, &mpp_tick1);

	MPP_DBG_Put_Process(MPP_STATE_R1, 26, 0xFF);
}


void MPP_SR1_27(void)
{
	UINT	lenOfV;

	UT_Get_TLVLengthOfV(glv_tagDF8301.Length, &lenOfV);

	MPP_Clear_DEPBuffer();
	mpp_depRspCode = MPP_APDU_EXCHANGE_RELAY_RESISTANCE_DATA(lenOfV, glv_tagDF8301.Value, &mpp_depRcvLen, mpp_depRcvData);

	MPP_DBG_Put_Process(MPP_STATE_R1, 27, 0xFF);
}


UCHAR MPP_SR1_28(void)
{
	UCHAR	rspCode = FALSE;
	int		measuredRRProcTime = 0;			//Measured Relay Resistance Processing Time
	int		maxTimeProcRR = 0;				//Max Time For Processing Relay Resistance APDU
	int		maxRRGracePeriod = 0;			//Maximum Relay Resistance Grace Period

	measuredRRProcTime = (int)(glv_tagDF8306.Value[0] * 256 + glv_tagDF8306.Value[1]);
	maxTimeProcRR = (int)(glv_tagDF8304.Value[0] * 256 + glv_tagDF8304.Value[1]);
	maxRRGracePeriod = (int)(glv_tagDF8133.Value[0] * 256 + glv_tagDF8133.Value[1]);
	
	if(measuredRRProcTime > (maxTimeProcRR + maxRRGracePeriod))
	{
		rspCode = TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_R1, 28, rspCode);

	return rspCode;
}


void MPP_SR1_29(void)
{
	glv_tag95.Value[4] |= MPP_TVR_RelayResistanceTimeLimitsExceeded;

	MPP_DBG_Put_Process(MPP_STATE_R1, 29, 0xFF);
}


UCHAR MPP_SR1_30(void)
{
	UCHAR	rspCode = FALSE;
	int		devicedExpectedTimeRA = 0;		//Device Estimated Transmission Time For Relay Resistance R-APDU
	int		terminalExpectedTimeRA = 0;		//Terminal Expected Transmission Time For Relay Resistance R-APDU
	int		measuredRRProcTime = 0;			//Measured Relay Resistance Processing Time
	int		minTimeProcRR = 0;				//Min Time For Processing Relay Resistance APDU
	int		RRAccuracyThreshold = 0;		//Relay Resistance Accuracy Threshold
	int		max = 0;

	devicedExpectedTimeRA = (int)(glv_tagDF8305.Value[0] * 256 + glv_tagDF8305.Value[1]);
	terminalExpectedTimeRA = (int)(glv_tagDF8135.Value[0] * 256 + glv_tagDF8135.Value[1]);
	measuredRRProcTime = (int)(glv_tagDF8306.Value[0] * 256 + glv_tagDF8306.Value[1]);
	minTimeProcRR = (int)(glv_tagDF8303.Value[0] * 256 + glv_tagDF8303.Value[1]);
	RRAccuracyThreshold = (int)(glv_tagDF8136.Value[0] * 256 + glv_tagDF8136.Value[1]);

	if(0 < (measuredRRProcTime - minTimeProcRR))
	{
		max = measuredRRProcTime - minTimeProcRR;
	}
	else
	{
		max = 0;
	}

	/*if((((devicedExpectedTimeRA / terminalExpectedTimeRA) * 100) < glv_tagDF8137.Value[0]) ||
		(((terminalExpectedTimeRA / devicedExpectedTimeRA) * 100) < glv_tagDF8137.Value[0]) ||
		((measuredRRProcTime - minTimeProcRR) > RRAccuracyThreshold))*/
	//Tammy 2017/11/14	[SB-195-Errata for EMV Book C-2(Version 2.6)]
	if((((devicedExpectedTimeRA * 100) / terminalExpectedTimeRA) < glv_tagDF8137.Value[0]) ||
		(((terminalExpectedTimeRA * 100) / devicedExpectedTimeRA) < glv_tagDF8137.Value[0]) ||
		(max > RRAccuracyThreshold))
	{
		rspCode = TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_R1, 30, rspCode);

	return rspCode;
}


void MPP_SR1_31(void)
{
	glv_tag95.Value[4] |= MPP_TVR_RelayResistanceThresholdExceeded;

	MPP_DBG_Put_Process(MPP_STATE_R1, 31, 0xFF);
}


void MPP_SR1_32(void)
{
	glv_tag95.Value[4] |= MPP_TVR_RRPPerformed;

	MPP_DBG_Put_Process(MPP_STATE_R1, 32, 0xFF);
}


UCHAR MPP_S3R1_1(void)
{
	UCHAR	rspCode = FAIL;
	
	rspCode = MPP_GetNextGetDataTagFromList(mpp_tagTagsToReadYet.Value, mpp_tagActiveTag.Value, MPP_LISTTYPE_TAG);
	if(rspCode == FAIL)	//Active Tag = NULL
	{
		rspCode = FALSE;
	}
	else
	{
		rspCode = TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 1, rspCode);
	
	return rspCode;
}


void MPP_S3R1_2(void)
{
//	Build GET DATA command for Active Tag

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 2, 0xFF);
}


void MPP_S3R1_3(void)
{
	MPP_Clear_DEPBuffer();
	mpp_depRspCode = MPP_APDU_GET_DATA(mpp_tagActiveTag.Value[0], mpp_tagActiveTag.Value[1], &mpp_depRcvLen, mpp_depRcvData);

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 3, 0xFF);
}


void MPP_S3R1_4(void)
{
	mpp_tagNextCmd.Value[0] = MPP_NXC_GetData;

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 4, 0xFF);
}


UCHAR MPP_S3R1_5(void)
{
	UINT	lenOfV = 0;
	UCHAR	rspCode = FALSE;
	
	UT_Get_TLVLengthOfV(mpp_tagActiveAFL.Length, &lenOfV);
	
	if(lenOfV == 0)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 5, rspCode);
	
	return rspCode;
}


void MPP_S3R1_6(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataError;

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 6, 0xFF);
}


void MPP_S3R1_7(void)
{
	//Build READ RECORD command for the first record indicated by Active AFL
	mpp_rrcSFI = (mpp_tagActiveAFL.Value[0] & 0xF8) >> 3;
	mpp_rrcNumber = mpp_tagActiveAFL.Value[1];

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 7, 0xFF);
}


void MPP_S3R1_8(void)
{
	MPP_Clear_DEPBuffer();
	mpp_depRspCode = MPP_APDU_READ_RECORD(mpp_rrcNumber, mpp_rrcSFI, &mpp_depRcvLen, mpp_depRcvData);

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 8, 0xFF);
}


void MPP_S3R1_9(void)
{
	mpp_tagNextCmd.Value[0] = MPP_NXC_ReadRecord;

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 9, 0xFF);
}


UCHAR MPP_S3R1_10(void)
{
	UCHAR	rspCode = FALSE;
	
	if(glv_tagDF8128.Value[0] & MPP_IDS_Read)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 10, rspCode);
	
	return rspCode;
}


void MPP_S3R1_11(void)
{
	UCHAR	tlv9F37[2 + 3 + ECL_LENGTH_9F37] = {0};
	UCHAR	tlv9F54[2 + 3 + ECL_LENGTH_9F54] = {0};
	UCHAR	tlv9F5F[2 + 3 + ECL_LENGTH_9F5F] = {0};
	UCHAR	tlv9F6F[2 + 3 + ECL_LENGTH_9F6F] = {0};
	UCHAR	tlv9F7D[2 + 3 + ECL_LENGTH_9F7D] = {0};
	UCHAR	tlv9F7F[2 + 3 + ECL_LENGTH_9F7F] = {0};
	UCHAR	rspCode=FALSE;

	rspCode = MPP_IsNotEmpty(mpp_tof9F5F);	//DS Slot Availability
	if(rspCode == TRUE)
	{
		MPP_GetTLV(mpp_tof9F5F, tlv9F5F);
		MPP_AddToList(tlv9F5F, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
	}

	rspCode=MPP_IsNotEmpty(mpp_tof9F7D);	//DS Summary 1
	if(rspCode == TRUE)
	{
		MPP_GetTLV(mpp_tof9F7D, tlv9F7D);
		MPP_AddToList(tlv9F7D, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
	}

	rspCode = MPP_IsNotEmpty(mpp_tof9F7F);	//DS Unpredictable Number
	if(rspCode == TRUE)
	{
		MPP_GetTLV(mpp_tof9F7F, tlv9F7F);
		MPP_AddToList(tlv9F7F, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
	}

	rspCode = MPP_IsNotEmpty(mpp_tof9F6F);	//DS Slot Management Control
	if(rspCode == TRUE)
	{
		MPP_GetTLV(mpp_tof9F6F, tlv9F6F);
		MPP_AddToList(tlv9F6F, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
	}

	rspCode = MPP_IsPresent(mpp_tof9F54);	//DS ODS Card			// 2017/11/03  NotEmpty => IsPresent
	if(rspCode == TRUE)
	{
		MPP_GetTLV(mpp_tof9F54, tlv9F54);
		MPP_AddToList(tlv9F54, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
	}

	MPP_GetTLV(mpp_tof9F37, tlv9F37);	//Unpredictable Number
	MPP_AddToList(tlv9F37, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 11, 0xFF);
}


UCHAR MPP_S3R1_12(void)
{
	UCHAR	flgIsNotPresent_9F54 = FALSE;
	UCHAR	flgIsPresent_9F54 = FALSE;
	UCHAR	flgIsNotEmpty_9F5F = FALSE;
	UCHAR	flgIsNotEmpty_9F7D = FALSE;
	UCHAR	flgIsNotEmpty_9F7F = FALSE;
	UCHAR	rspCode=FALSE;

	flgIsNotEmpty_9F5F = MPP_IsNotEmpty(mpp_tof9F5F);
	flgIsNotEmpty_9F7D = MPP_IsNotEmpty(mpp_tof9F7D);
	flgIsNotEmpty_9F7F = MPP_IsNotEmpty(mpp_tof9F7F);
	flgIsNotPresent_9F54 = MPP_IsNotPresent(mpp_tof9F54);
	flgIsPresent_9F54 = MPP_IsPresent(mpp_tof9F54);

	if(((flgIsNotEmpty_9F5F == TRUE) &&
		(flgIsNotEmpty_9F7D == TRUE) &&
		(flgIsNotEmpty_9F7F == TRUE) &&
		(flgIsNotPresent_9F54 == TRUE))
		||
		((flgIsNotEmpty_9F7D == TRUE) &&
		(flgIsPresent_9F54 == TRUE)))
	{
		rspCode = TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 12, rspCode);
	
	return rspCode;
}


void MPP_S3R1_13(void)
{
	//CLEAR 'Read' in IDS Status
	glv_tagDF8128.Value[0] &= (~MPP_IDS_Read);

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 13, 0xFF);
}


void MPP_S3R1_14(void)
{
	UCHAR	lenOfT = 0;
	UINT	lenOfData = 0;
	UINT	lenOfTagsToReadYet = 0;
	UCHAR	*ptrData = NULLPTR;
	UCHAR	rspCode = FALSE;
	UCHAR	tlvBuffer[3 + 3 + 1024] = {0};	//Max. T + Max. L + Max. V
	UCHAR	lstBuffer[MPP_TLVLIST_BUFFERSIZE] = {0};

	//Copy Tags To Read Yet to Avoid RemoveFromList(Tags To Read Yet) Error in Loop	
	memcpy(lstBuffer, mpp_tagTagsToReadYet.Value, MPP_TLVLIST_BUFFERSIZE);
	UT_Get_TLVLengthOfV(mpp_tagTagsToReadYet.Length, &lenOfTagsToReadYet);
	
	//Set Pointer
	ptrData = lstBuffer;
	
	do {
		UT_Get_TLVLengthOfT(ptrData, &lenOfT);

		rspCode = MPP_IsNotEmpty(ptrData);
		if (rspCode == TRUE)
		{
			MPP_GetTLV(ptrData, tlvBuffer);
			MPP_AddToList(tlvBuffer, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
			MPP_RemoveFromList(ptrData, mpp_tagTagsToReadYet.Value, MPP_LISTTYPE_TAG);
		}

		//Point to Next Tag
		ptrData += lenOfT;
		lenOfData += lenOfT;
	} while(lenOfData < lenOfTagsToReadYet);

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 14, 0xFF);
}


UCHAR MPP_S3R1_15(void)
{
	UCHAR	flgIsNotEmpty_DataNeeded = FALSE;
	UCHAR	flgIsNotEmpty_DataToSend = FALSE;
	UCHAR	flgIsEmpty_TagsToReadYet = FALSE;
	UCHAR	rspCode = FALSE;
	
	flgIsNotEmpty_DataNeeded = MPP_IsNotEmptyList(glv_tagDF8106.Value);
	flgIsNotEmpty_DataToSend = MPP_IsNotEmptyList(glv_tagFF8104.Value);
	flgIsEmpty_TagsToReadYet = MPP_IsEmptyList(mpp_tagTagsToReadYet.Value);

	if ((flgIsNotEmpty_DataNeeded == TRUE)
		||
		((flgIsNotEmpty_DataToSend == TRUE) && 
		(flgIsEmpty_TagsToReadYet == TRUE)))
	{
		rspCode = TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 15, flgIsNotEmpty_DataNeeded);
	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 15, flgIsNotEmpty_DataToSend);
	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 15, flgIsEmpty_TagsToReadYet);
	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 15, rspCode);
	
	return rspCode;
}


void MPP_S3R1_16(void)
{
//	Send DEK(Data To Send, Data Needed) signal
	MPP_DEK(2);
	

	MPP_Initialize_Tag(mpp_tofFF8104);
	MPP_Initialize_Tag(mpp_tofDF8106);

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 16, 0xDE);
}


UCHAR MPP_S3R1_17(void)
{
	UCHAR	rspCode = FALSE;
	
	if((glv_tag82.Value[0] & MPP_AIP_CDASupported) &&
		(glv_tag9F33.Value[2] & MPP_TRC_CDA))
	{
		rspCode = TRUE;
	}
	
	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 17, rspCode);

	return rspCode;
}


UCHAR MPP_S3R1_18(void)
{
	UCHAR	rspCode = FALSE;
	
	if(glv_tagDF8128.Value[0] & MPP_IDS_Read)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 18, rspCode);
	
	return rspCode;
}


void MPP_S3R1_19(void)
{
	mpp_tagODAStatus.Value[0] = MPP_ODS_CDA;

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 19, 0xFF);
}


void MPP_S3R1_20(void)
{
	glv_tag95.Value[0] |= MPP_TVR_OfflineDataAuthenticationWasNotPerformed;

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 20, 0xFF);
}


UCHAR MPP_S3R1_21(void)
{
	UCHAR	rspCode = FALSE;
	
	if((mpp_tagNextCmd.Value[0] & 0xC0) == MPP_NXC_ReadRecord)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_3R1_D, 21, rspCode);

	return rspCode;
}


UCHAR MPP_S4_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_DET)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_4, 1, rspCode);

	return rspCode;
}


void MPP_S4_2(void)
{

	MPP_UpdateWithDetData(mpp_DETRcvBuff);


	MPP_DBG_Put_Process(MPP_STATE_4, 2, 0xDD);
}


UCHAR MPP_S4_3(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_RA)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_4, 3, rspCode);
	
	return rspCode;
}


UCHAR MPP_S4_4(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_4, 4, rspCode);
	
	return rspCode;
}


void MPP_S4_5(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_TryAgain;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_DBG_Put_Process(MPP_STATE_4, 5, 0xFF);
}


void MPP_S4_6(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;
	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;
	glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(mpp_depRspCode);
	glv_tagDF8115.Value[5]=MPP_EID_MOE_TryAgain;
	
	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_4, 6, 0xFF);
}


UCHAR MPP_S4_7(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_4, 7, rspCode);

	return rspCode;
}


void MPP_S4_8(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_Stop;
	
	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_4, 8, 0xFF);
}


UCHAR MPP_S4_9(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=UT_Check_SW12(&mpp_depRcvData[mpp_depRcvLen-2], STATUSWORD_9000);

	//Copy DEP Data to Read Record Buffer
	if (rspCode == TRUE)
	{
		mpp_rrcRcvLen=mpp_depRcvLen;
		memcpy(mpp_rrcRcvData, mpp_depRcvData, mpp_rrcRcvLen);

		MPP_DataExchangeLog((UCHAR*)&"APDURes- RR", 11);
		MPP_DataExchangeLog(mpp_depRcvData, mpp_rrcRcvLen);
	}

	MPP_DBG_Put_Process(MPP_STATE_4, 9, rspCode);
	
	return rspCode;
}


void MPP_S4_10_1(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_4, 10, 0xFF);
}


void MPP_S4_10_2(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8115.Value[1]=MPP_EID_LV2_StatusBytes;
	memcpy(&glv_tagDF8115.Value[3], &mpp_depRcvData[mpp_depRcvLen-2], 2);
	
	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_4, 10, 0xFF);
}


UCHAR MPP_S4_11(void)
{
	UCHAR	rspCode=FALSE;

	if (mpp_tagActiveAFL.Value[3] != 0)
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_4, 11, rspCode);

	return rspCode;
}


void MPP_S4_12(void)
{
	mpp_flgSigned=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_4, 12, 0xFF);
}


void MPP_S4_13(void)
{
	mpp_flgSigned=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_4, 13, 0xFF);
}


void MPP_S4_14(void)
{
	mpp_rrcSFI=(mpp_tagActiveAFL.Value[0] & 0xF8) >> 3;
	MPP_Remove_FirstRecordFromActiveAFL();

	MPP_DBG_Put_Process(MPP_STATE_4, 14, 0xFF);
}


UCHAR MPP_S4_15(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_GetNextGetDataTagFromList(mpp_tagTagsToReadYet.Value, mpp_tagActiveTag.Value, MPP_LISTTYPE_TAG);

	if (rspCode == TRUE)
	{
		rspCode=TRUE;
	}
	
	MPP_DBG_Put_Process(MPP_STATE_4, 15, rspCode);
	
	return rspCode;
}


void MPP_S4_16(void)
{
//	Prepare GET DATA command for Active Tag

	MPP_DBG_Put_Process(MPP_STATE_4, 16, 0xFF);
}


void MPP_S4_17(void)
{
	MPP_Clear_DEPBuffer();
	mpp_depRspCode=MPP_APDU_GET_DATA(mpp_tagActiveTag.Value[0], mpp_tagActiveTag.Value[1], &mpp_depRcvLen, mpp_depRcvData);

	MPP_DBG_Put_Process(MPP_STATE_4, 17, 0xFF);
}


void MPP_S4_18(void)
{
	mpp_tagNextCmd.Value[0]=MPP_NXC_GetData;

	MPP_DBG_Put_Process(MPP_STATE_4, 18, 0xFF);
}


UCHAR MPP_S4_19(void)
{
	UINT	lenOfV=0;
	UCHAR	rspCode=FALSE;
	
	UT_Get_TLVLengthOfV(mpp_tagActiveAFL.Length, &lenOfV);

	if (lenOfV == 0)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_4, 19, rspCode);

	return rspCode;
}


void MPP_S4_20(void)
{
	mpp_tagNextCmd.Value[0]=MPP_NXC_None;

	MPP_DBG_Put_Process(MPP_STATE_4, 20, 0xFF);
}


void MPP_S4_21(void)
{
	//Prepare READ RECORD command for first record in Active AFL

	MPP_DBG_Put_Process(MPP_STATE_4, 21, 0xFF);
}


void MPP_S4_22(void)
{
	UCHAR	rrcNumber=0;
	UCHAR	rrcSFI=0;

	rrcSFI=(mpp_tagActiveAFL.Value[0] & 0xF8) >> 3;
	rrcNumber=mpp_tagActiveAFL.Value[1];
	
	MPP_Clear_DEPBuffer();
	mpp_depRspCode=MPP_APDU_READ_RECORD(rrcNumber, rrcSFI, &mpp_depRcvLen, mpp_depRcvData);

	MPP_DBG_Put_Process(MPP_STATE_4, 22, 0xFF);
}


void MPP_S4_23(void)
{
	mpp_tagNextCmd.Value[0]=MPP_NXC_ReadRecord;

	MPP_DBG_Put_Process(MPP_STATE_4, 23, 0xFF);
}


void MPP_S4_24(void)
{
	if (mpp_rrcSFI <= 10)
	{
		if ((mpp_rrcRcvLen > 0) && (mpp_rrcRcvData[0] == 0x70))
		{
			mpp_flgParResult=MPP_ParseAndStoreCardResponse(mpp_rrcRcvLen, mpp_rrcRcvData);
		}
		else
		{
			mpp_flgParResult=FALSE;
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_4, 24, 0xFF);
}


UCHAR MPP_S4_25(void)
{
	UCHAR	rspCode=FALSE;
	UCHAR	flgIsNotEmpty=FALSE;
	UINT	lenOfV=0;
	
	if (mpp_flgParResult == TRUE)
		rspCode=TRUE;

//EVAL 3M21-9082(CVMList_OddNumBytes)
//Check CVM list length
flgIsNotEmpty=MPP_IsNotEmpty(mpp_tof8E);
if (flgIsNotEmpty == TRUE)
{
	UT_Get_TLVLengthOfV(glv_tag8E.Length, &lenOfV);
	if (lenOfV & 1)
	{
		mpp_flgOddCVM=TRUE;
		rspCode=FALSE;
	}
}

	MPP_DBG_Put_Process(MPP_STATE_4, 25, rspCode);
	
	return rspCode;
}


UCHAR MPP_S4_26(void)
{
	UCHAR	rspCode=FALSE;

//EVAL 3M21-9082(CVMList_OddNumBytes)
//Force process go to S4_E27_1
if (mpp_flgOddCVM == TRUE)
{
	mpp_tagNextCmd.Value[0]=MPP_NXC_None;
}

	if (mpp_tagNextCmd.Value[0] == MPP_NXC_None)
	{
		rspCode=TRUE;
	}
	
	MPP_DBG_Put_Process(MPP_STATE_4, 26, rspCode);
	
	return rspCode;
}


void MPP_S4_27_1(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_4, 27, 0xFF);
}


void MPP_S4_27_2(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;

//EVAL 3M21-9082(CVMList_OddNumBytes)
//Set Card Data Error in Error Indication
if (mpp_flgOddCVM == TRUE)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataError;
}
else
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_ParsingError;
}

	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_4, 27, 0xFF);
}


UCHAR MPP_S4_28(void)
{
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=FALSE;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;

	UT_Get_TLVLength(mpp_rrcRcvData, &lenOfT, &lenOfL, &lenOfV);
	ptrData=UT_Find_Tag(mpp_tof8C, lenOfV, &mpp_rrcRcvData[lenOfT+lenOfL]);
	if (ptrData != NULLPTR)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_4, 28, rspCode);
	
	return rspCode;
}


void MPP_S4_29(void)
{
	UCHAR	*ptrData=NULLPTR;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfData=0;
	UINT	lenOfCDOL1=0;
	UCHAR	rspCode=FALSE;

	//Find CDOL1
	UT_Get_TLVLength(mpp_rrcRcvData, &lenOfT, &lenOfL, &lenOfV);		// 2017/10/27 dep => rrc
	ptrData=UT_Find_Tag(mpp_tof8C, lenOfV, &mpp_rrcRcvData[lenOfT+lenOfL]);

	//Get CDOL1 Length
	UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
	lenOfCDOL1=lenOfV;

	//Point to TLV-V of CDOL1
	ptrData+=(lenOfT+lenOfL);
	
	do {
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		
		rspCode=MPP_IsEmpty(ptrData);
		if (rspCode == TRUE)
		{
			MPP_AddToList(ptrData, glv_tagDF8106.Value, MPP_LISTTYPE_TAG);
		}
		
		//Point to Next Tag
		ptrData+=(lenOfT+lenOfL);
		lenOfData+=(lenOfT+lenOfL);
	} while (lenOfData < lenOfCDOL1);

	MPP_DBG_Put_Process(MPP_STATE_4, 29, 0xFF);
}


UCHAR MPP_S4_30(void)
{
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=FALSE;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;

	UT_Get_TLVLength(mpp_rrcRcvData, &lenOfT, &lenOfL, &lenOfV);
	ptrData=UT_Find_Tag(mpp_tof9F5B, lenOfV, &mpp_rrcRcvData[lenOfT+lenOfL]);
	if (ptrData != NULLPTR)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_4, 30, rspCode);
	
	return rspCode;
}


UCHAR MPP_S4_31(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8128.Value[0] & MPP_IDS_Read)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_4, 31, rspCode);
	
	return rspCode;
}


UCHAR MPP_S4_32(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof9F6F);

	if ((rspCode == TRUE) && (glv_tag9F6F.Value[0] & MPP_DSM_LockedSlot))
	{
		rspCode=TRUE;
	}
	else
	{
		rspCode=FALSE;
	}

	MPP_DBG_Put_Process(MPP_STATE_4, 32, rspCode);
	
	return rspCode;
}


void MPP_S4_33(void)
{
	UCHAR	*ptrData=NULLPTR;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfData=0;
	UINT	lenOfDSDOL=0;
	UCHAR	rspCode=FALSE;

	//Find DSDOL
	UT_Get_TLVLength(mpp_rrcRcvData, &lenOfT, &lenOfL, &lenOfV);
	ptrData=UT_Find_Tag(mpp_tof9F5B, lenOfV, &mpp_rrcRcvData[lenOfT+lenOfL]);

	//Get DSDOL Length
	UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
	lenOfDSDOL=lenOfV;

	//Point to TLV-V of CDOL1
	ptrData+=(lenOfT+lenOfL);
	
	do {
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		
		rspCode=MPP_IsEmpty(ptrData);
		if (rspCode == TRUE)
		{
			MPP_AddToList(ptrData, glv_tagDF8106.Value, MPP_LISTTYPE_TAG);
		}
		
		//Point to Next Tag
		ptrData+=(lenOfT+lenOfL);
		lenOfData+=(lenOfT+lenOfL);
	} while (lenOfData < lenOfDSDOL);

	MPP_DBG_Put_Process(MPP_STATE_4, 33, 0xFF);
}


UCHAR MPP_S4_34(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((mpp_flgSigned == TRUE) && (mpp_tagODAStatus.Value[0] & MPP_ODS_CDA))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_4, 34, rspCode);
	
	return rspCode;
}


void MPP_S4_35(void)
{
	UCHAR	lenOfT_70=0;
	UCHAR	lenOfL_70=0;
	UINT	lenOfV_70=0;
	UINT	lenOfV_SDA=0;
	UCHAR	rspCode=FALSE;
	
	if (mpp_rrcSFI <= 10)
	{
		UT_Get_TLVLength(mpp_rrcRcvData, &lenOfT_70, &lenOfL_70, &lenOfV_70);
		UT_Get_TLVLengthOfV(mpp_tagStaticDataToBeAuthenticated.Length, &lenOfV_SDA);

		if ((lenOfV_SDA+lenOfV_70) <= MPP_LENGTH_StaticDataToBeAuthenticated)
		{
			UT_Set_TagLength((lenOfV_SDA+lenOfV_70), mpp_tagStaticDataToBeAuthenticated.Length);
			memcpy(&mpp_tagStaticDataToBeAuthenticated.Value[lenOfV_SDA], &mpp_rrcRcvData[lenOfT_70+lenOfL_70], lenOfV_70);
		}
		else
		{
			glv_tag95.Value[0]|=MPP_TVR_CDAFailed;
		}
	}
	else
	{
		rspCode=UT_Get_TLVLength(mpp_rrcRcvData, &lenOfT_70, &lenOfL_70, &lenOfV_70);
		UT_Get_TLVLengthOfV(mpp_tagStaticDataToBeAuthenticated.Length, &lenOfV_SDA);
		
		if ((mpp_rrcRcvData[0] == 0x70) && 
			(rspCode == TRUE) &&
			((lenOfV_SDA+lenOfT_70+lenOfL_70+lenOfV_70) <= MPP_LENGTH_StaticDataToBeAuthenticated))
		{
			UT_Set_TagLength((lenOfV_SDA+lenOfT_70+lenOfL_70+lenOfV_70), mpp_tagStaticDataToBeAuthenticated.Length);
			memcpy(&mpp_tagStaticDataToBeAuthenticated.Value[lenOfV_SDA], mpp_rrcRcvData, (lenOfT_70+lenOfL_70+lenOfV_70));
		}
		else
		{
			glv_tag95.Value[0]|=MPP_TVR_CDAFailed;
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_4, 35, 0xFF);
}


UCHAR MPP_S4Apostrophes_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_RA)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_4Apostrophes, 1, rspCode);
	
	return rspCode;
}


UCHAR MPP_S4Apostrophes_2(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_4Apostrophes, 2, rspCode);
	
	return rspCode;
}


UCHAR MPP_S4Apostrophes_3(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_4Apostrophes, 3, rspCode);
	
	return rspCode;
}


void MPP_S4Apostrophes_4_1(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_4Apostrophes, 4, 0xFF);
}


void MPP_S4Apostrophes_4_2(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8115.Value[1]=MPP_EID_LV2_ParsingError;
	
	MPP_CreateEMVDiscretionaryData();
	
	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;
	
	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_4Apostrophes, 4, 0xFF);
}


void MPP_S4Apostrophes_5(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_Stop;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_4Apostrophes, 5, 0xFF);
}


UCHAR MPP_S5_1(void)
{
	if (mpp_Signal == MPP_SIGNAL_DET)
		return TRUE;
	
	MPP_DBG_Put_Process(MPP_STATE_5, 1, 0xFF);
	return FALSE;
}


void MPP_S5_2(void)
{
	MPP_UpdateWithDetData(mpp_DETRcvBuff); 


	MPP_DBG_Put_Process(MPP_STATE_5, 2, 0xDD);
}


UCHAR MPP_S5_3(void)
{
	if (mpp_Signal == MPP_SIGNAL_RA)
		return TRUE;
	
	return FALSE;
}


UCHAR MPP_S5_4(void)
{
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		return TRUE;
	
	return FALSE;
}


void MPP_S5_5(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_TryAgain;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_DBG_Put_Process(MPP_STATE_5, 5, 0xFF);
}


void MPP_S5_6(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;
	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;
	glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(mpp_depRspCode);
	glv_tagDF8115.Value[5]=MPP_EID_MOE_TryAgain;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_5, 6, 0xFF);
}


UCHAR MPP_S5_7(void)
{
	if (mpp_Signal == MPP_SIGNAL_STOP)
		return TRUE;
	
	return FALSE;
}


void MPP_S5_8(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_Stop;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_5, 8, 0xFF);
}


void MPP_S5_9(UCHAR *curTag)
{
	UINT	lenOfV=0;

	UT_Get_TLVLengthOfV(mpp_tagActiveTag.Length, &lenOfV);
	
	memcpy(curTag, mpp_tagActiveTag.Value, lenOfV);
	MPP_DBG_Put_Process(MPP_STATE_5, 9, 0xFF);
}


UCHAR MPP_S5_10(void)
{
	UCHAR	rspCode=FALSE;

	//Tammy 2017/10/23
	//Copy DEP Data to Get Data Buffer
	mpp_gdcRcvLen = mpp_depRcvLen;
	memcpy(mpp_gdcRcvData, mpp_depRcvData, mpp_gdcRcvLen);
	 MPP_DataExchangeLog((UCHAR*)&"APDURes- GD", 11);
	 MPP_DataExchangeLog(mpp_gdcRcvData, mpp_gdcRcvLen);

	rspCode=MPP_GetNextGetDataTagFromList(mpp_tagTagsToReadYet.Value, mpp_tagActiveTag.Value, MPP_LISTTYPE_TAG);

	if (rspCode == TRUE)
	{
		return TRUE;
	}
	
	MPP_DBG_Put_Process(MPP_STATE_5, 10, 0xFF);
	return FALSE;
}


void MPP_S5_11(void)
{
//	Prepare GET DATA command for Active Tag
}


void MPP_S5_12(void)
{
	MPP_Clear_DEPBuffer();
	mpp_depRspCode=MPP_APDU_GET_DATA(mpp_tagActiveTag.Value[0], mpp_tagActiveTag.Value[1], &mpp_depRcvLen, mpp_depRcvData);
	
	MPP_DBG_Put_Process(MPP_STATE_5, 12, 0xFF);
}


void MPP_S5_13(void)
{
	mpp_tagNextCmd.Value[0]=MPP_NXC_GetData;

	MPP_DBG_Put_Process(MPP_STATE_5, 13, 0xFF);
}


UCHAR MPP_S5_14(void)
{
	UINT	lenOfV=0;

	UT_Get_TLVLengthOfV(mpp_tagActiveAFL.Length, &lenOfV);
	if (lenOfV == 0)
		return TRUE;

	return FALSE;
}


void MPP_S5_15(void)
{
	mpp_tagNextCmd.Value[0]=MPP_NXC_None;
}


void MPP_S5_16(void)
{
	mpp_rrcSFI=(mpp_tagActiveAFL.Value[0] & 0xF8) >> 3;
	mpp_rrcNumber=mpp_tagActiveAFL.Value[1];
}


void MPP_S5_17(void)
{
	MPP_Clear_DEPBuffer();
	mpp_depRspCode=MPP_APDU_READ_RECORD(mpp_rrcNumber, mpp_rrcSFI, &mpp_depRcvLen, mpp_depRcvData);
}


void MPP_S5_18(void)
{
	mpp_tagNextCmd.Value[0]=MPP_NXC_ReadRecord;

	MPP_DBG_Put_Process(MPP_STATE_5, 18, 0xFF);
}


UCHAR MPP_S5_19(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=UT_Check_SW12(&mpp_gdcRcvData[mpp_gdcRcvLen-2], STATUSWORD_9000);


	if (rspCode == TRUE)
		return TRUE;
	
	return FALSE;
}


void MPP_S5_20(void)
{
	mpp_flgParResult=MPP_ParseAndStoreCardResponse(mpp_gdcRcvLen, mpp_gdcRcvData);


	MPP_DBG_Put_Process(MPP_STATE_5, 20, mpp_flgParResult);
}


UCHAR MPP_S5_21(void)
{
	if (mpp_flgParResult == TRUE)
		return TRUE;
	
	return FALSE;
}


UCHAR MPP_S5_22(UCHAR *curTag)
{
	UCHAR	lenOfT=0;

	UT_Get_TLVLengthOfT(mpp_gdcRcvData, &lenOfT);
	
	if (!memcmp(curTag, mpp_gdcRcvData, lenOfT))
		return TRUE;
	
	return FALSE;
}


void MPP_S5_23(void)
{
	MPP_AddToList(mpp_gdcRcvData, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
}


void MPP_S5_24(UCHAR *curTag)
{
	UCHAR	tmpTLV[3+1]={0};
	UCHAR	lenOfT=0;
	
	UT_Get_TLVLengthOfT(curTag, &lenOfT);
	
	memcpy(tmpTLV, curTag, lenOfT);
	tmpTLV[lenOfT]=0x00;

	MPP_AddToList(tmpTLV, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
	MPP_DBG_Put_Process(MPP_STATE_5, 24, 0xFF);
}


UCHAR MPP_S6_1(void)
{
	if (mpp_Signal != MPP_SIGNAL_DET  &&  mpp_Signal != MPP_SIGNAL_STOP)
		return TRUE;
	
	return FALSE;
}


UCHAR MPP_S6_2(void)
{
	if (mpp_Signal == MPP_SIGNAL_STOP)
		return TRUE;
	
	return FALSE;
}


void MPP_S6_3(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_TimeOut;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_6, 3, 0xFF);
}


void MPP_S6_4(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_Stop;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
}


UCHAR MPP_S6_5(void)
{
	if (mpp_Signal == MPP_SIGNAL_DET)		
		return TRUE;
	
	return FALSE;
}


void MPP_S6_6(void)
{
	MPP_UpdateWithDetData(mpp_DETRcvBuff);




	MPP_DBG_Put_Process(MPP_STATE_6, 6, 0xDD);
}


void MPP_S6_7(void)
{
//	Stop Timer
}


UCHAR MPP_S6_8(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_GetNextGetDataTagFromList(mpp_tagTagsToReadYet.Value, mpp_tagActiveTag.Value, MPP_LISTTYPE_TAG);
	if (rspCode == TRUE)
	{
		return TRUE;
	}
	
	return FALSE;
}


void MPP_S6_9(void)
{
//	Prepare GET DATA command for Active Tag
}


void MPP_S6_10(void)
{
	MPP_Clear_DEPBuffer();
	mpp_depRspCode=MPP_APDU_GET_DATA(mpp_tagActiveTag.Value[0], mpp_tagActiveTag.Value[1], &mpp_depRcvLen, mpp_depRcvData);
	MPP_DBG_Put_Process(MPP_STATE_6, 10, 0xFF);
}


void MPP_S6_11(void)
{
	mpp_tagNextCmd.Value[0]=MPP_NXC_GetData;

	MPP_DBG_Put_Process(MPP_STATE_6, 11, 0xFF);
}


void MPP_S6_12(void)
{
	mpp_tagNextCmd.Value[0]=MPP_NXC_None;

	MPP_DBG_Put_Process(MPP_STATE_6, 12, 0xFF);
}

UCHAR MPP_S456_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((mpp_tagNextCmd.Value[0] & 0xC0) == MPP_NXC_ReadRecord)
	{
		rspCode='R';	//READ RECORD
	}
	else
	{
		if ((mpp_tagNextCmd.Value[0] & 0xC0) == MPP_NXC_GetData)
		{
			rspCode='G';	//GET DATA
		}
		else
		{
			rspCode='N';	//NONE
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_456_A, 1, 0xFF);
	
	return rspCode;
}


void MPP_S456_2(void)
{
	UCHAR	rspCode=FALSE;
	UCHAR	lenOfT=0;
	UINT	lenOfData=0;
	UINT	lenOfTagsToReadYet=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	tmpTLV[3+3+1024]={0};

	//Get Total Length
	//UT_Get_TLVLengthOfV(mpp_tagTagsToReadYet.Length, &lenOfTagsToReadYet);	
	//Set Pointer
	ptrData=mpp_tagTagsToReadYet.Value;
	
	do {
		UT_Get_TLVLengthOfV(mpp_tagTagsToReadYet.Length, &lenOfTagsToReadYet);		//same as s1.15

		rspCode=MPP_IsNotEmpty(ptrData);
		if (rspCode == TRUE)
		{
			MPP_GetTLV(ptrData, tmpTLV);
			MPP_AddToList(tmpTLV, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
			MPP_RemoveFromList(ptrData, mpp_tagTagsToReadYet.Value, MPP_LISTTYPE_TAG);
			continue;
		}

		//Point to Next Tag
		UT_Get_TLVLengthOfT(ptrData, &lenOfT);
		ptrData+=lenOfT;
		lenOfData+=lenOfT;
	} while (lenOfData < lenOfTagsToReadYet);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 2, 0xFF);
}


UCHAR MPP_S456_3(void)
{
	UCHAR	flgIsNotEmptyList_FF8104=FALSE;
	UCHAR	flgIsEmptyList_TagsToReadYet=FALSE;
	UCHAR	rspCode=FALSE;

	flgIsNotEmptyList_FF8104=MPP_IsNotEmptyList(glv_tagFF8104.Value);
	flgIsEmptyList_TagsToReadYet=MPP_IsEmptyList(mpp_tagTagsToReadYet.Value);

	if ((flgIsNotEmptyList_FF8104 == TRUE) && (flgIsEmptyList_TagsToReadYet == TRUE))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 3, rspCode);
	
	return rspCode;
}


void MPP_S456_4(void)
{
//	Send DEK(Data To Send, Data Needed) signal
	MPP_DEK(2);

	MPP_Initialize_Tag(mpp_tofFF8104);
	MPP_Initialize_Tag(mpp_tofDF8106);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 4, 0xDE);
}


UCHAR MPP_S456_5(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsEmpty(mpp_tofDF8110);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 5, rspCode);
	
	return rspCode;
}


void MPP_S456_6(void)
{
	MPP_AddToList(mpp_tofDF8110, glv_tagDF8106.Value, MPP_LISTTYPE_TAG);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 6, 0xFF);
}


void MPP_S456_7(void)
{
	UCHAR	rspCode=FALSE;
	UCHAR	lenOfT=0;
	UINT	lenOfData=0;
	UINT	lenOfTagsToReadYet=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	tmpTLV[3+3+1024]={0};

	//Get Total Length
	//UT_Get_TLVLengthOfV(mpp_tagTagsToReadYet.Length, &lenOfTagsToReadYet);

	//Set Pointer
	ptrData=mpp_tagTagsToReadYet.Value;
	
	do {
		UT_Get_TLVLengthOfV(mpp_tagTagsToReadYet.Length, &lenOfTagsToReadYet);	// 2017/11/02  same as s1.15

		rspCode=MPP_IsNotEmpty(ptrData);
		if (rspCode == TRUE)
		{
			MPP_GetTLV(ptrData, tmpTLV);
			MPP_AddToList(tmpTLV, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
			MPP_RemoveFromList(ptrData, mpp_tagTagsToReadYet.Value, MPP_LISTTYPE_TAG);
			continue;
		}

		//Point to Next Tag
		UT_Get_TLVLengthOfT(ptrData, &lenOfT);
		ptrData+=lenOfT;
		lenOfData+=lenOfT;
	} while (lenOfData < lenOfTagsToReadYet);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 7, 0xFF);
}


UCHAR MPP_S456_8(void)
{
	UCHAR	flgIsNotEmptyList_DF8106=FALSE;
	UCHAR	flgIsNotEmptyList_FF8104=FALSE;
	UCHAR	flgIsEmptyList_TagsToReadYet=FALSE;
	UCHAR	rspCode=FALSE;
	
	flgIsNotEmptyList_DF8106=MPP_IsNotEmptyList(glv_tagDF8106.Value);
	flgIsNotEmptyList_FF8104=MPP_IsNotEmptyList(glv_tagFF8104.Value);
	flgIsEmptyList_TagsToReadYet=MPP_IsEmptyList(mpp_tagTagsToReadYet.Value);

	if ((flgIsNotEmptyList_DF8106 == TRUE) ||
		((flgIsNotEmptyList_FF8104 == TRUE) && (flgIsEmptyList_TagsToReadYet == TRUE)))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_456_A, 8, rspCode);

	return rspCode;
}


void MPP_S456_9(void)
{
//	Send DEK(Data To Send, Data Needed) signal
	MPP_DEK(2);

	MPP_Initialize_Tag(mpp_tofFF8104);
	MPP_Initialize_Tag(mpp_tofDF8106);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 9, 0xDE);
}


void MPP_S456_10(void)
{
//	Start Timer (Time Out Value)

	MPP_DBG_Put_Process(MPP_STATE_456_A, 10, 0xFF);
}


UCHAR MPP_S456_11(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsPresent(mpp_tofDF8110);
	if ((rspCode == TRUE) && (glv_tagDF8110.Value[0] == 0x00))
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 11, rspCode);
	
	return rspCode;
}


UCHAR MPP_S456_12(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof9F02);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 12, rspCode);

	return rspCode;
}


void MPP_S456_13(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_AmountNotPresent;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 13, 0xFF);
}


UCHAR MPP_S456_14(void)
{
	UCHAR	rspCode=0xFF;

	rspCode=UT_bcdcmp(glv_tag9F02.Value, mpp_tagReaderContactlessTransactionLimit.Value, ECL_LENGTH_9F02);
	if (rspCode == 1)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 14, rspCode);
	
	return rspCode;
}


void MPP_S456_15(void)
{
	glv_tagDF8129.Value[6]=MPP_OPS_FieldOffRequest_NA;
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_SelectNext;
	glv_tagDF8129.Value[1]=MPP_OPS_START_C;
	
	glv_tagDF8115.Value[1]=MPP_EID_LV2_MaxLimitExceeded;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 15, 0xFF);
}


UCHAR MPP_S456_16(void)
{
	UCHAR	flgIsNotEmpty_5F24=FALSE;
	UCHAR	flgIsNotEmpty_5A=FALSE;
	UCHAR	flgIsNotEmpty_8C=FALSE;
	UCHAR	rspCode=FALSE;
	
	flgIsNotEmpty_5F24=MPP_IsNotEmpty(mpp_tof5F24);	//Application Expiration Date
	flgIsNotEmpty_5A=MPP_IsNotEmpty(mpp_tof5A);		//Application PAN
	flgIsNotEmpty_8C=MPP_IsNotEmpty(mpp_tof8C);		//CDOL1

	if ((flgIsNotEmpty_5F24 == TRUE) &&
		(flgIsNotEmpty_5A == TRUE) &&
		(flgIsNotEmpty_8C == TRUE))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_456_A, 16, rspCode);
	
	return rspCode;
}


void MPP_S456_17_1(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 17, 0xFF);
}


void MPP_S456_17_2(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_CreateEMVDiscretionaryData();
	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 17, 0xFF);
}


UCHAR MPP_S456_18(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8128.Value[0] & MPP_IDS_Read)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 18, rspCode);

	return rspCode;
}


UCHAR MPP_S456_19(void)
{
	UCHAR	datY[11]={0};
	UCHAR	lenY=0;
	UINT	len9F5E=0;
	UCHAR	rspCode=0xFF;
	
	MPP_Generate_Y(&lenY, datY);

	UT_Get_TLVLengthOfV(glv_tag9F5E.Length, &len9F5E);

	if (len9F5E == lenY)
	{
		rspCode=UT_bcdcmp(glv_tag9F5E.Value, datY, len9F5E);
		if (rspCode == 0)
			rspCode=TRUE;
		else
			rspCode=FALSE;
	}
	else
	{
		rspCode=FALSE;
	}

	MPP_DBG_Put_Process(MPP_STATE_456_A, 19, rspCode);
	
	return rspCode;
}


void MPP_S456_20_1(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 20, 0xFF);
}


void MPP_S456_20_2(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataError;

	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 20, 0xFF);
}


void MPP_S456_21(void)
{
	UCHAR	rspCode=FALSE;
	UCHAR	lenOfT=0;
	UINT	lenOfData=0;
	UINT	lenOfTagsToReadYet=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	tmpTLV[3+3+1024]={0};

	//Get Total Length
	UT_Get_TLVLengthOfV(mpp_tagTagsToReadYet.Length, &lenOfTagsToReadYet);

	//Set Pointer
	ptrData=mpp_tagTagsToReadYet.Value;
	
	do {
		UT_Get_TLVLengthOfT(ptrData, &lenOfT);
		
		rspCode=MPP_IsPresent(ptrData);
		if (rspCode == TRUE)
		{
			MPP_GetTLV(ptrData, tmpTLV);
			MPP_AddToList(tmpTLV, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
		}
		else
		{
			rspCode = MPP_IsKnown(ptrData);		//william 2017/10/24    missing this if
			if (rspCode == TRUE)
			{
				memcpy(tmpTLV, ptrData, lenOfT);
				tmpTLV[lenOfT]=0;
	
				MPP_AddToList(tmpTLV, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
			}
		}

		MPP_RemoveFromList(ptrData, mpp_tagTagsToReadYet.Value, MPP_LISTTYPE_TAG);

		//Point to Next Tag
		lenOfData+=lenOfT;
	} while (lenOfData < lenOfTagsToReadYet);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 21, 0xFF);
}


UCHAR MPP_S456_22(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsEmptyList(glv_tagFF8104.Value);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 22, rspCode);
	
	return rspCode;
}


void MPP_S456_23(void)
{
//	Send DEK(Data To Send) signal
	MPP_DEK(1);

	MPP_Initialize_Tag(mpp_tofFF8104);
	MPP_Initialize_Tag(mpp_tofDF8106);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 23, 0xDE);
}


UCHAR MPP_S456_24(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_tagODAStatus.Value[0] & MPP_ODS_CDA)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 24, rspCode);

	return rspCode;
}


void MPP_S456_25(void)
{
	UCHAR	flgIsNotEmpty_8F=FALSE;
	UCHAR	flgIsNotEmpty_90=FALSE;
	UCHAR	flgIsNotEmpty_9F32=FALSE;
	UCHAR	flgIsNotEmpty_9F46=FALSE;
	UCHAR	flgIsNotEmpty_9F47=FALSE;
	UCHAR	flgIsNotEmpty_9F4A=FALSE;
	UCHAR	optModLen=0;
	UCHAR	optModulus[255]={0};
	UCHAR	optExponent[3]={0};
	UCHAR	rspCode=FAIL;
	
	flgIsNotEmpty_8F=MPP_IsNotEmpty(mpp_tof8F);		//CA Public Key Index (Card)
	flgIsNotEmpty_90=MPP_IsNotEmpty(mpp_tof90);		//Issuer Public Key Certificate
	flgIsNotEmpty_9F32=MPP_IsNotEmpty(mpp_tof9F32);	//Issuer Public Key Exponent
	flgIsNotEmpty_9F46=MPP_IsNotEmpty(mpp_tof9F46);	//ICC Public Key Certificate
	flgIsNotEmpty_9F47=MPP_IsNotEmpty(mpp_tof9F47);	//ICC Public Key Exponent
	flgIsNotEmpty_9F4A=MPP_IsNotEmpty(mpp_tof9F4A);	//Static Data Authentication Tag List
	
	if (!((flgIsNotEmpty_8F == TRUE) &&
		(flgIsNotEmpty_90 == TRUE) &&
		(flgIsNotEmpty_9F32 == TRUE) &&
		(flgIsNotEmpty_9F46 == TRUE) &&
		(flgIsNotEmpty_9F47 == TRUE) &&
		(flgIsNotEmpty_9F4A == TRUE)))
	{
		glv_tag95.Value[0]|=MPP_TVR_ICCDataMissing;
		glv_tag95.Value[0]|=MPP_TVR_CDAFailed;
	}

	rspCode=MPP_Retrieve_PK_CA(glv_tag9F06.Value, glv_tag8F.Value[0], &optModLen, optModulus, optExponent);
	if (rspCode == FAIL)
	{
		glv_tag95.Value[0]|=MPP_TVR_CDAFailed;
	}

	MPP_DBG_Put_Process(MPP_STATE_456_A, 25, 0xFF);
}


UCHAR MPP_S456_26(void)
{
	UCHAR	rspCode=FALSE;
	UINT	lenOfV=0;
	
	rspCode=MPP_IsNotEmpty(mpp_tof9F4A);
	if (rspCode == TRUE)
	{
		UT_Get_TLVLengthOfV(glv_tag9F4A.Length, &lenOfV);
		if ((lenOfV == 1) && (glv_tag9F4A.Value[0] == 0x82))
		{
			rspCode=TRUE;
		}
		else
		{
			rspCode=FALSE;
		}
	}
	else
	{
		rspCode=FALSE;
	}

	MPP_DBG_Put_Process(MPP_STATE_456_A, 26, rspCode);
	
	return rspCode;
}


void MPP_S456_27_1(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 27, 0xFF);
}


void MPP_S456_27_2(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataError;

	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 27, 0xFF);
}


void MPP_S456_28(void)
{
	UINT	lenOfV_SDA=0;

	UT_Get_TLVLengthOfV(mpp_tagStaticDataToBeAuthenticated.Length, &lenOfV_SDA);

	if ((lenOfV_SDA+2) <= MPP_LENGTH_StaticDataToBeAuthenticated)
	{
		UT_Set_TagLength((lenOfV_SDA+2), mpp_tagStaticDataToBeAuthenticated.Length);
		memcpy(&mpp_tagStaticDataToBeAuthenticated.Value[lenOfV_SDA], glv_tag82.Value, 2);
	}
	else
	{
		glv_tag95.Value[0]|=MPP_TVR_CDAFailed;
	}
	
	MPP_DBG_Put_Process(MPP_STATE_456_A, 28, 0xFF);
}


UCHAR MPP_S456_30(void)
{
	UCHAR	rspCode=0xFF;

	rspCode=UT_bcdcmp(glv_tag9F02.Value, glv_tagDF8126.Value, ECL_LENGTH_9F02);
	if (rspCode == 1)	//Amount, Authorized (Numeric) > Reader CVM Required Limit
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 30, rspCode);

 	return rspCode;
}


void MPP_S456_31(void)
{
	glv_tagDF8129.Value[4]|=MPP_OPS_Receipt_Yes;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 31, 0xFF);
}


void MPP_S456_32(void)
{
	glv_tag9F33.Value[1]=glv_tagDF8118.Value[0];

	MPP_DBG_Put_Process(MPP_STATE_456_A, 32, 0xFF);
}


void MPP_S456_33(void)
{
	glv_tag9F33.Value[1]=glv_tagDF8119.Value[0];

	MPP_DBG_Put_Process(MPP_STATE_456_A, 33, 0xFF);
}


void MPP_S456_34(void)
{
	UCHAR	rspCode=0;
	
	rspCode=MPP_Procedure_PreGenACBalanceReading();
	if (rspCode == MPP_STATE_16)
	{
		MPP_State_16_WaitingForPreGenACBalance();
	}

	MPP_DBG_Put_Process(MPP_STATE_456_A, 34, 0xFF);
}


void MPP_S456_35(void)
{
	MPP_Procedure_ProcessingRestrictions();

	MPP_DBG_Put_Process(MPP_STATE_456_A, 35, 0xFF);
}


void MPP_S456_36(void)
{
	MPP_Procedure_CVMSelection();

	MPP_DBG_Put_Process(MPP_STATE_456_A, 36, 0xFF);
}


UCHAR MPP_S456_37(void)
{
	UCHAR	rspCode=0xFF;

	rspCode=UT_bcdcmp(glv_tag9F02.Value, glv_tagDF8123.Value, ECL_LENGTH_9F02);
	if (rspCode == 1)	//Amount, Authorized (Numeric) > Reader Contactless Floor Limit
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 37, rspCode);
	
	return rspCode;
}


void MPP_S456_38(void)
{
	glv_tag95.Value[3]|=MPP_TVR_TransactionExceedsFloorLimit;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 38, 0xFF);
}


void MPP_S456_39(void)
{
	MPP_Procedure_TerminalActionAnalysis();

	MPP_DBG_Put_Process(MPP_STATE_456_A, 39, 0xFF);
}


UCHAR MPP_S456_42(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmptyList(glv_tagFF8102.Value);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 42, rspCode);
	
	return rspCode;
}


UCHAR MPP_S456_43(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsPresent(mpp_tof9F51);
	if ((rspCode == TRUE) && (glv_tagDF811D.Value[0] != 0))
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_456_A, 43, rspCode);

	return rspCode;
}


UCHAR MPP_S456_44(void)
{

//	Same As MPP_S12_14
	
	UCHAR	recIndex=0;
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_Search_TornRecord_PAN(&recIndex);
	if (rspCode == SUCCESS)
	{
		mpp_tagTornEntry.Value[0]=recIndex;
		rspCode=TRUE;
	}
	else
	{
		rspCode=FALSE;
	}

	MPP_DBG_Put_Process(MPP_STATE_456_A, 44, rspCode);

	return rspCode;
}


UCHAR MPP_S456_45(void)
{
	UCHAR	rspCode = FALSE;
	rspCode = MPP_Procedure_PrepareGenACCommand();


	MPP_DBG_Put_Process(MPP_STATE_456_A, 45, rspCode);
	return rspCode;	
}


void MPP_S456_46(void)
{
	UINT	lenOfV=0;
	
	UT_Get_TLVLengthOfV(mpp_tempSend.Length, &lenOfV);
	
	MPP_Clear_DEPBuffer();
	//mpp_depRspCode=MPP_APDU_GENERATE_AC(glv_tagDF8114.Value[0], lenOfV, glv_tagDF8107.Value, &mpp_depRcvLen, mpp_depRcvData);
	mpp_depRspCode=MPP_APDU_GENERATE_AC(glv_tagDF8114.Value[0], lenOfV, mpp_tempSend.Value, &mpp_depRcvLen, mpp_depRcvData);

	mpp_vfyDataLen=mpp_depRcvLen;
	memcpy(mpp_vfyData, mpp_depRcvData, mpp_depRcvLen);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 46, 0xFF);
}


void MPP_S456_47(void)
{

//	Same As MPP_S12_17

	UCHAR	idxAID=0xFF;
	
	idxAID=MPP_Get_TornRecordAIDIndex();
	
	UT_Set_TagLength(mpp_trnRec[idxAID][mpp_tagTornEntry.Value[0]].RecLen, mpp_tagTornTempRecord.Length);
	memcpy(mpp_tagTornTempRecord.Value, mpp_trnRec[idxAID][mpp_tagTornEntry.Value[0]].Record, mpp_trnRec[idxAID][mpp_tagTornEntry.Value[0]].RecLen);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 47, 0xFF);
}


void MPP_S456_48(void)
{

//	Same As MPP_S12_18

	UCHAR	tmpTLV[3+3+MPP_LENGTH_TornTempRecord]={0};
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	*ptrData=NULLPTR;

	//Patch FF8101 TLV-T
	memcpy(tmpTLV, mpp_tofFF8101, 3);
	lenOfT=3;

	//Patch FF8101 TLV-L
	UT_Get_TLVLengthOfV(mpp_tagTornTempRecord.Length, &lenOfV);
	lenOfL=UT_Set_TagLength(lenOfV, &tmpTLV[3]);

	//Patch FF8101 TLV-V
	memcpy(&tmpTLV[lenOfT+lenOfL], mpp_tagTornTempRecord.Value, lenOfV);

	//Find DRDOL Related Data
	ptrData=UT_Find_Tag(mpp_tofDF8113, lenOfV, &tmpTLV[lenOfT+lenOfL]);
	if (ptrData != NULLPTR)
	{
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		UT_Set_TagLength(lenOfV, glv_tagDF8113.Length);
		memcpy(glv_tagDF8113.Value, (ptrData+lenOfT+lenOfL), lenOfV);
	}

	MPP_DBG_Put_Process(MPP_STATE_456_A, 48, 0xFF);
}


void MPP_S456_49(void)
{
	UINT	lenOfV=0;

	UT_Get_TLVLengthOfV(glv_tagDF8113.Length, &lenOfV);
	
	MPP_Clear_DEPBuffer();
	mpp_depRspCode=MPP_APDU_RECOVER_AC(lenOfV, glv_tagDF8113.Value, &mpp_depRcvLen, mpp_depRcvData);

	mpp_vfyDataLen=mpp_depRcvLen;
	memcpy(mpp_vfyData, mpp_depRcvData, mpp_depRcvLen);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 49, 0xFF);
}


void MPP_S456_50(UCHAR *tlvBuffer)
{
	MPP_GetAndRemoveFromList(mpp_tagTagsToWriteYetBeforeGenAC.Value, tlvBuffer, MPP_LISTTYPE_TLV);

	MPP_DBG_Put_Process(MPP_STATE_456_A, 50, 0xFF);
}


void MPP_S456_51(UCHAR *tlvBuffer)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	rspCode=FALSE;

	rspCode=UT_Get_TLVLength(tlvBuffer, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == SUCCESS)
	{
		if (lenOfV <= 255)
		{
			if (lenOfT == 1)
			{
				MPP_Clear_DEPBuffer();
				mpp_depRspCode=MPP_APDU_PUT_DATA(0, tlvBuffer[0], lenOfV, &tlvBuffer[lenOfT+ lenOfL], &mpp_depRcvLen, mpp_depRcvData);  //2017/10/31
			}
			else if (lenOfT == 2)
			{
				MPP_Clear_DEPBuffer();
				mpp_depRspCode=MPP_APDU_PUT_DATA(tlvBuffer[0], tlvBuffer[1], lenOfV, &tlvBuffer[lenOfT+ lenOfL], &mpp_depRcvLen, mpp_depRcvData);
			}
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_456_A, 51, mpp_depRspCode);
}


UCHAR MPP_S7_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_DET)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_7, 1, rspCode);
	
	return rspCode;
}


void MPP_S7_2(void)
{
	MPP_UpdateWithDetData(mpp_DETRcvBuff);

	MPP_DBG_Put_Process(MPP_STATE_7, 2, 0xDD);
}


UCHAR MPP_S7_3(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_RA)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_7, 3, rspCode);

	return rspCode;
}


UCHAR MPP_S7_4(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_7, 4, rspCode);

	return rspCode;
}


void MPP_S7_5(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_TryAgain;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0, 3);

	MPP_DBG_Put_Process(MPP_STATE_7, 5, 0xFF);
}


void MPP_S7_6(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;
	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;
	
	glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(mpp_depRspCode);
	glv_tagDF8115.Value[5]=MPP_EID_MOE_TryAgain;

	MPP_CreateMSDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_7, 6, 0xFF);
}


UCHAR MPP_S7_7(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_7, 7, rspCode);
	
	return rspCode;
}


void MPP_S7_8(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_Stop;

	MPP_CreateMSDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_7, 8, 0xFF);
}

UCHAR MPP_S7_9(UCHAR *iptSW)
{
	UCHAR	rspCode=FALSE;

	rspCode=UT_Check_SW12(iptSW, STATUSWORD_9000);

	MPP_DBG_Put_Process(MPP_STATE_7, 9, rspCode);
	
	return rspCode;
}


void MPP_S7_10_1(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_7, 10, 0xFF);
}


void MPP_S7_10_2(UCHAR *iptSW)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8115.Value[1]=MPP_EID_LV2_StatusBytes;
	memcpy(&glv_tagDF8115.Value[3], iptSW, 2);

	MPP_CreateMSDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;
	
	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_7, 10, 0xFF);
}


void MPP_S7_11(void)
{
	if (mpp_rrcSFI <= 10)
	{
		if ((mpp_depRcvLen > 0) && (mpp_depRcvData[0] == 0x70))
		{
			mpp_flgParResult=MPP_ParseAndStoreCardResponse(mpp_depRcvLen, mpp_depRcvData);
		}
		else
		{
			mpp_flgParResult=FALSE;
		}
	}
//	else
//	{
//		//Processing of records in proprietary files is beyond the scope of this specification
//	}

	MPP_DBG_Put_Process(MPP_STATE_7, 11, 0xFF);
}


UCHAR MPP_S7_12(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_flgParResult == TRUE)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_7, 12, rspCode);
	
	return rspCode;
}


void MPP_S7_13_1(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;
	
	//MPP_MSG(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_7, 13, 0xFF);
}


void MPP_S7_13_2(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;

	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8115.Value[1]=MPP_EID_LV2_ParsingError;

	MPP_CreateMSDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_7, 13, 0xFF);
}


UCHAR MPP_S7_14(void)
{
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=FALSE;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;

	UT_Get_TLVLength(mpp_depRcvData, &lenOfT, &lenOfL, &lenOfV);
	ptrData=UT_Find_Tag(mpp_tof9F69, lenOfV, &mpp_depRcvData[lenOfT+lenOfL]);
	if (ptrData != NULLPTR)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_7, 14, rspCode);
	
	return rspCode;
}


void MPP_S7_15(void)
{
	UCHAR	*ptrData=NULLPTR;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfUDOL=0;
	UINT	lenOfData=0;
	UCHAR	rspCode=FALSE;

	UT_Get_TLVLength(mpp_depRcvData, &lenOfT, &lenOfL, &lenOfV);
	ptrData=UT_Find_Tag(mpp_tof9F69, lenOfV, &mpp_depRcvData[lenOfT+lenOfL]);

	//Get UDOL TLV
	UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
	lenOfUDOL=lenOfV;
	
	//Point to UDOL Value
	ptrData+=(lenOfT+lenOfL);

	do {
		rspCode=MPP_IsEmpty(ptrData);
		if (rspCode == TRUE)
		{
			MPP_AddToList(ptrData, glv_tagDF8106.Value, MPP_LISTTYPE_TAG);
		}

		//Point to Next Tag
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		ptrData+=(lenOfT+lenOfL);
		lenOfData+=(lenOfT+lenOfL);
	} while (lenOfData <= lenOfUDOL);

	MPP_DBG_Put_Process(MPP_STATE_7, 15, 0xFF);
}


void MPP_S7_16(void)
{
	MPP_Remove_FirstRecordFromActiveAFL();

	MPP_DBG_Put_Process(MPP_STATE_7, 16, 0xFF);
}


UCHAR MPP_S7_17(void)
{
	UINT	lenOfV=0;
	UCHAR	rspCode=FALSE;
	
	UT_Get_TLVLengthOfV(mpp_tagActiveAFL.Length, &lenOfV);
	
	if (lenOfV == 0)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_7, 17, rspCode);
	
	return rspCode;
}


void MPP_S7_18(void)
{
	mpp_rrcSFI=(mpp_tagActiveAFL.Value[0] & 0xF8) >> 3;
	mpp_rrcNumber=mpp_tagActiveAFL.Value[1];

	MPP_DBG_Put_Process(MPP_STATE_7, 18, 0xFF);
}


void MPP_S7_19(void)
{
	MPP_Clear_DEPBuffer();
	mpp_depRspCode=MPP_APDU_READ_RECORD(mpp_rrcNumber, mpp_rrcSFI, &mpp_depRcvLen, mpp_depRcvData);

	MPP_DBG_Put_Process(MPP_STATE_7, 19, 0xFF);
}


UCHAR MPP_S7_20(void)
{
	UCHAR	flgIsNotEmpty_9F65=FALSE;
	UCHAR	flgIsNotEmpty_9F66=FALSE;
	UCHAR	flgIsNotEmpty_9F67=FALSE;
	UCHAR	flgIsNotEmpty_9F6B=FALSE;
	UCHAR	rspCode=FALSE;

	flgIsNotEmpty_9F65=MPP_IsNotEmpty(mpp_tof9F65);
	flgIsNotEmpty_9F66=MPP_IsNotEmpty(mpp_tof9F66);
	flgIsNotEmpty_9F67=MPP_IsNotEmpty(mpp_tof9F67);
	flgIsNotEmpty_9F6B=MPP_IsNotEmpty(mpp_tof9F6B);

	if ((flgIsNotEmpty_9F65 == TRUE) &&
		(flgIsNotEmpty_9F66 == TRUE) &&
		(flgIsNotEmpty_9F67 == TRUE) &&
		(flgIsNotEmpty_9F6B == TRUE))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_7, 20, rspCode);
	
	return rspCode;
}


void MPP_S7_21_1(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_7, 21, 0xFF);
}


void MPP_S7_21_2(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;
	
	MPP_CreateMSDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;
	
	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_7, 21, 0xFF);
}


UCHAR MPP_S7_22(void)
{
	UINT	nzo9F66=0;
	UINT	nzo9F63=0;
	UCHAR	flgIsNotEmpty_56=FALSE;
	UCHAR	flgIsNotPresent_9F62=FALSE;
	UCHAR	flgIsEmpty_9F62=FALSE;
	UCHAR	flgIsNotPresent_9F63=FALSE;
	UCHAR	flgIsEmpty_9F63=FALSE;
	UCHAR	flgIsNotPresent_9F64=FALSE;
	UCHAR	flgIsEmpty_9F64=FALSE;
	UCHAR	rspCode=0xFF;
	UINT	lenOfV=0;
	
	nzo9F66=MPP_Get_NumberOfNonZeroBits(2, glv_tag9F66.Value);
	mpp_tagnUN.Length[0]=MPP_LENGTH_nUN;
	mpp_tagnUN.Value[0]=nzo9F66-glv_tag9F67.Value[0];

	if (mpp_tagnUN.Value[0] > 8)
	{
		rspCode=FALSE;
	}

	if (rspCode != FALSE)
	{
		flgIsNotEmpty_56=MPP_IsNotEmpty(mpp_tof56);
		if (flgIsNotEmpty_56 == TRUE)
		{
			flgIsNotPresent_9F62=MPP_IsNotPresent(mpp_tof9F62);
			flgIsEmpty_9F62=MPP_IsEmpty(mpp_tof9F62);
			flgIsNotPresent_9F63=MPP_IsNotPresent(mpp_tof9F63);
			flgIsEmpty_9F63=MPP_IsEmpty(mpp_tof9F63);
			flgIsNotPresent_9F64=MPP_IsNotPresent(mpp_tof9F64);
			flgIsEmpty_9F64=MPP_IsEmpty(mpp_tof9F64);

			nzo9F63=MPP_Get_NumberOfNonZeroBits(ECL_LENGTH_9F63, glv_tag9F63.Value);

			if (((flgIsNotPresent_9F64 == TRUE) || (flgIsEmpty_9F64 == TRUE))
				||
				((flgIsNotPresent_9F62 == TRUE) || (flgIsEmpty_9F62 == TRUE))
				||
				((flgIsNotPresent_9F63 == TRUE) || (flgIsEmpty_9F63 == TRUE))
				||
				((nzo9F63 - glv_tag9F64.Value[0]) != mpp_tagnUN.Value[0]))
			{
				rspCode=FALSE;
			}
			else
			{
				UT_Get_TLVLengthOfV(glv_tag56.Length, &lenOfV);
				rspCode=MPP_Check_Separator(lenOfV, glv_tag56.Value, MPP_TRACK_1);
				if (rspCode == SUCCESS)
				{
					rspCode=TRUE;
				}
				else
				{
					rspCode=FALSE;
				}
			}
		}
		else
		{
			UT_Get_TLVLengthOfV(glv_tag9F6B.Length, &lenOfV);
			rspCode=MPP_Check_Separator(lenOfV, glv_tag9F6B.Value, MPP_TRACK_2);
			if (rspCode == SUCCESS)
			{
				rspCode=TRUE;
			}
			else
			{
				rspCode=FALSE;
			}
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_7, 22, rspCode);
	
	return rspCode;
}


void MPP_S7_23(void)
{
	UCHAR	rspCode=FALSE;
	UCHAR	lenDD_Track1=0;
	UCHAR	lenDD_Track2=0;
	UCHAR	lenDD_Temp=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	posDD=0;
	
	ptrData=MPP_Find_DD_Track2(&lenDD_Track2, &posDD);
	if (ptrData != NULLPTR)
	{
		//Convert Track2 BCD Length to Byte Length
		if (lenDD_Track2 & 1)
		{
			lenDD_Temp=lenDD_Track2/2+1;
		}
		else
		{
			lenDD_Temp=lenDD_Track2/2;
		}

		if (lenDD_Temp <= ECL_LENGTH_DF812B)
		{
			MPP_AddToList(mpp_tofDF812B, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

			glv_tagDF812B.Length[0]=lenDD_Temp;
			memcpy(glv_tagDF812B.Value, ptrData, lenDD_Temp);
		}
	}

	rspCode=MPP_IsNotEmpty(mpp_tof56);
	if (rspCode == TRUE)
	{
		ptrData=MPP_Find_DD_Track1(&lenDD_Track1);
		if (ptrData != NULLPTR)
		{
			if (lenDD_Track1 <= ECL_LENGTH_DF812A)
			{
				MPP_AddToList(mpp_tofDF812A, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

				glv_tagDF812A.Length[0]=lenDD_Track1;
				memcpy(glv_tagDF812A.Value, ptrData, lenDD_Track1);
			}			
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_7, 23, 0xFF);
}


void MPP_S7_24_1(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_7, 24, 0xFF);
}


void MPP_S7_24_2(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataError;

	MPP_CreateMSDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_7, 24, 0xFF);
}


UCHAR MPP_S8_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal != MPP_SIGNAL_DET  &&  mpp_Signal != MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_8, 1, rspCode);
	
	return rspCode;
}


void MPP_S8_2(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_TimeOut;
	
	MPP_CreateMSDiscretionaryData();
	
	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_8, 2, 0xFF);
}


UCHAR MPP_S8_3(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_8, 3, rspCode);

	return rspCode;
}


void MPP_S8_4(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_Stop;
	
	MPP_CreateMSDiscretionaryData();
	
	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_8, 4, 0xFF);
}


UCHAR MPP_S8_5(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_DET)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_8, 5, rspCode);
	
	return rspCode;
}


void MPP_S8_6(void)
{
	MPP_UpdateWithDetData(mpp_DETRcvBuff);

	MPP_DBG_Put_Process(MPP_STATE_8, 6, 0xDD);
}


void MPP_S8_7(void)
{
//	Stop Timer

	MPP_DBG_Put_Process(MPP_STATE_8, 7, 0xFF);
}


UCHAR MPP_S78_1(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsEmpty(mpp_tofDF8110);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_78_A, 1, rspCode);

	return rspCode;
}


void MPP_S78_2(void)
{
	MPP_AddToList(mpp_tofDF8110, glv_tagDF8106.Value, MPP_LISTTYPE_TAG);

	MPP_DBG_Put_Process(MPP_STATE_78_A, 2, 0xFF);
}


void MPP_S78_3(void)
{
	UINT	lenOfData=0;
	UCHAR	lenOfT=0;
	UINT	lenOfList=0;
	UCHAR	rspCode=FALSE;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	tlvBuffer[3+3+1024]={0};

	//Set Pointer
	ptrData=mpp_tagTagsToReadYet.Value;

	//UT_Get_TLVLengthOfV(mpp_tagTagsToReadYet.Length, &lenOfList);
	
	do {
		UT_Get_TLVLengthOfV(mpp_tagTagsToReadYet.Length, &lenOfList);		//same as s1.15

		rspCode=MPP_IsNotEmpty(ptrData);
		if (rspCode == TRUE)
		{
			MPP_GetTLV(ptrData, tlvBuffer);
			MPP_AddToList(tlvBuffer, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
			MPP_RemoveFromList(ptrData, mpp_tagTagsToReadYet.Value, MPP_LISTTYPE_TAG);
			continue;
		}

		//Point to Next Tag
		UT_Get_TLVLengthOfT(ptrData, &lenOfT);
		ptrData+=lenOfT;
		lenOfData+=lenOfT;
	} while (lenOfData < lenOfList);

	MPP_DBG_Put_Process(MPP_STATE_78_A, 3, 0xFF);
}


UCHAR MPP_S78_4(void)
{
	UCHAR	flgIsNotEmpty_DataNeeded=FALSE;
	UCHAR	flgIsNotEmpty_DataToSend=FALSE;
	UCHAR	flgIsEmpty_TagsToReadYet=FALSE;
	UCHAR	rspCode=FALSE;
	
	flgIsNotEmpty_DataNeeded=MPP_IsNotEmptyList(glv_tagDF8106.Value);
	flgIsNotEmpty_DataToSend=MPP_IsNotEmptyList(glv_tagFF8104.Value);
	flgIsEmpty_TagsToReadYet=MPP_IsEmptyList(mpp_tagTagsToReadYet.Value);

	if ((flgIsNotEmpty_DataNeeded == TRUE)
		||
		((flgIsNotEmpty_DataToSend == TRUE) && (flgIsEmpty_TagsToReadYet == TRUE)))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_78_A, 4, rspCode);
	
	return rspCode;
}


void MPP_S78_5(void)
{
//	Send DEK(Data To Send, Data Needed) signal

	MPP_DEK(2);

	MPP_Initialize_Tag(mpp_tofFF8104);
	MPP_Initialize_Tag(mpp_tofDF8106);

	MPP_DBG_Put_Process(MPP_STATE_78_A, 5, 0xDE);
}


void MPP_S78_6(void)
{
//	Start Timer (Time Out Value)

	MPP_DBG_Put_Process(MPP_STATE_78_A, 6, 0xFF);
}


UCHAR MPP_S78_7(void)
{
	UCHAR	rspCode = FALSE;		
	rspCode=MPP_IsPresent(mpp_tofDF8110);
	if ((rspCode == TRUE) && (glv_tagDF8110.Value[0] == 0x00))
		rspCode=TRUE;
	else
		rspCode = FALSE;

	MPP_DBG_Put_Process(MPP_STATE_78_A, 7, rspCode);

	return rspCode;
}


UCHAR MPP_S78_8(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof9F02);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_78_A, 8, rspCode);
	
	return rspCode;
}


void MPP_S78_9(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_AmountNotPresent;
	
	MPP_CreateMSDiscretionaryData();
	
	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_78_A, 9, 0xFF);
}


UCHAR MPP_S78_10(void)
{
	UCHAR	rspCode=0xFF;

	rspCode=UT_bcdcmp(glv_tag9F02.Value, mpp_tagReaderContactlessTransactionLimit.Value, ECL_LENGTH_9F02);
	if (rspCode == 1)	//Amount, Authorized (Numeric) > Reader Contactless Transaction Limit
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_78_A, 10, rspCode);
	
	return rspCode;
}


void MPP_S78_11(void)
{
	glv_tagDF8129.Value[6]=MPP_OPS_FieldOffRequest_NA;
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_SelectNext;
	glv_tagDF8129.Value[1]=MPP_OPS_START_C;

	glv_tagDF8115.Value[1]=MPP_EID_LV2_MaxLimitExceeded;
	
	MPP_CreateMSDiscretionaryData();
	
	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_78_A, 11, 0xFF);
}


void MPP_S78_12(void)		// 2017/11/13 add flagIsKnow in 3.1.11
{
	UINT	lenOfData=0;
	UCHAR	lenOfT=0;
	UINT	lenOfList=0;
	//UCHAR	rspCode=FALSE;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	tlvBuffer[3+3+1024]={0};
	UCHAR	flagIsPresent = FALSE, flagIsKnown = FALSE;

	//Set Pointer
	ptrData=mpp_tagTagsToReadYet.Value;

	UT_Get_TLVLengthOfV(mpp_tagTagsToReadYet.Length, &lenOfList);
	
	do {
		UT_Get_TLVLengthOfT(ptrData, &lenOfT);
		
		flagIsPresent = MPP_IsPresent(ptrData);
		flagIsKnown = MPP_IsKnown(ptrData);
		if (flagIsPresent == TRUE)
		{
			MPP_GetTLV(ptrData, tlvBuffer);
			MPP_AddToList(tlvBuffer, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
		}
		else
		{
			if(flagIsKnown)
			{
				memset(tlvBuffer, 0, lenOfT+1);
				memcpy(tlvBuffer, ptrData, lenOfT);
				tlvBuffer[lenOfT]=0x00;	//Patch '00'
	
				MPP_AddToList(tlvBuffer, glv_tagFF8104.Value, MPP_LISTTYPE_TLV);
			}
		}

		MPP_RemoveFromList(ptrData, mpp_tagTagsToReadYet.Value, MPP_LISTTYPE_TAG);

		//Point to Next Tag
		//ptrData+=lenOfT;		// 2017/11/13  cause inf.loop
		lenOfData+=lenOfT;
	} while (lenOfData < lenOfList);

	MPP_DBG_Put_Process(MPP_STATE_78_A, 12, 0xFF);
}


UCHAR MPP_S78_13(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsEmptyList(glv_tagFF8104.Value);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_78_A, 13, rspCode);

	return rspCode;
}


void MPP_S78_14(void)
{
//	Send DEK(Data To Send) signal
	MPP_DEK(1);

	MPP_Initialize_Tag(mpp_tofFF8104);
	MPP_Initialize_Tag(mpp_tofDF8106);

	MPP_DBG_Put_Process(MPP_STATE_78_A, 14, 0xDE);
}


void MPP_S78_15(void)
{
	ULONG	rndNumber=0;
	UCHAR	ascBuffer[10]={0};
	UCHAR	cntIndex=0;

	rndNumber=MPP_Get_BCDRandomNumber_4Byte();

	UT_INT2ASC(rndNumber, ascBuffer);

	for (cntIndex=0; cntIndex < (8-mpp_tagnUN.Value[0]); cntIndex++)
	{
		ascBuffer[2+cntIndex]='0';	//8 Digits Start from ascBuffer[2]
	}

	glv_tag9F6A.Length[0]=ECL_LENGTH_9F6A;
	UT_Compress(glv_tag9F6A.Value, &ascBuffer[2], ECL_LENGTH_9F6A);

	MPP_DBG_Put_Process(MPP_STATE_78_A, 15, 0xFF);
}


UCHAR MPP_S78_16(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((glv_tag82.Value[0] & MPP_AIP_OnDeviceCardholderVerificationIsSupported) &&
		(glv_tagDF811B.Value[0] & MPP_KCF_OnDeviceCardholderVerificationSupported))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_78_A, 16, rspCode);
	
	return rspCode;
}


void MPP_S78_17(UCHAR *optLen, UCHAR *optData)
{
	UINT	lenOfV=0;
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsPresent(mpp_tof9F69);
	if (rspCode == TRUE)
	{
		UT_Get_TLVLengthOfV(glv_tag9F69.Length, &lenOfV);
		MPP_DOL_Get_DOLRelatedData(lenOfV, glv_tag9F69.Value, optLen, optData);
	}
	else
	{
		//Use Default UDOL
		UT_Get_TLVLengthOfV(glv_tagDF811A.Length, &lenOfV);
		MPP_DOL_Get_DOLRelatedData(lenOfV, glv_tagDF811A.Value, optLen, optData);
	}

	MPP_DBG_Put_Process(MPP_STATE_78_A, 17, 0xFF);
}


void MPP_S78_18(UCHAR iptLen, UCHAR *iptData)
{
	MPP_Clear_DEPBuffer();
	mpp_depRspCode=MPP_APDU_COMPUTE_CRYPTOGRAPHIC_CHECKSUM(iptLen, iptData, &mpp_depRcvLen, mpp_depRcvData);

	MPP_DBG_Put_Process(MPP_STATE_78_A, 18, 0xFF);
}


UCHAR MPP_S78_19(void)
{
	UCHAR	rspCode=0xFF;

	rspCode=UT_bcdcmp(glv_tag9F02.Value, glv_tagDF8126.Value, 6);
	if (rspCode == 1)	//Amount, Authorized (Numeric) > Reader CVM Required Limit
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_78_A, 19, rspCode);
	
	return rspCode;
}


void MPP_S78_20(void)
{
	glv_tag9F7E.Value[0]|=MPP_MSI_ODCVMRequired;
	glv_tagDF8129.Value[3]=MPP_OPS_CVM_ConfirmationCodeVerified;

	MPP_DBG_Put_Process(MPP_STATE_78_A, 20, 0xFF);
}


void MPP_S78_21(UCHAR *optLen, UCHAR *optData)
{
	UINT	lenOfV=0;
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsPresent(mpp_tof9F69);
	if (rspCode == TRUE)
	{
		UT_Get_TLVLengthOfV(glv_tag9F69.Length, &lenOfV);
		MPP_DOL_Get_DOLRelatedData(lenOfV, glv_tag9F69.Value, optLen, optData);
	}
	else
	{
		//Use Default UDOL
		UT_Get_TLVLengthOfV(glv_tagDF811A.Length, &lenOfV);
		MPP_DOL_Get_DOLRelatedData(lenOfV, glv_tagDF811A.Value, optLen, optData);
	}

	MPP_DBG_Put_Process(MPP_STATE_78_A, 21, 0xFF);
}


void MPP_S78_22(UCHAR iptLen, UCHAR *iptData)
{
	MPP_Clear_DEPBuffer();
	mpp_depRspCode=MPP_APDU_COMPUTE_CRYPTOGRAPHIC_CHECKSUM(iptLen, iptData, &mpp_depRcvLen, mpp_depRcvData);

	MPP_DBG_Put_Process(MPP_STATE_78_A, 22, 0xFF);
}


UCHAR MPP_S9_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_9, 1, rspCode);
	
	return rspCode;
}


UCHAR MPP_S9_2(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_RA)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_9, 2, rspCode);
	
	return rspCode;
}


UCHAR MPP_S9_3(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_9, 3, rspCode);
	
	return rspCode;
}


UCHAR MPP_S9_4(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_DET)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_9, 4, rspCode);
	
	return rspCode;
}


UCHAR MPP_S9_5(void)
{
	UCHAR	rspCode=FALSE;

//EVAL 3M50-0003(02-No_Entry_In_Torn_Tranx)
//DRDOL should not be empty
rspCode=MPP_IsNotEmpty(mpp_tof9F51);

	if ((glv_tagDF811D.Value[0] > 0) && (rspCode == TRUE))
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_9, 5, rspCode);

	return rspCode;
}


UCHAR MPP_S9_6(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8128.Value[0] & MPP_IDS_Write)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_9, 6, rspCode);

	return rspCode;
}


void MPP_S9_7(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	MPP_DBG_Put_Process(MPP_STATE_9, 7, 0xFF);
}


void MPP_S9_8(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(mpp_depRspCode);
	glv_tagDF8129.Value[4]|=MPP_OPS_DataRecordPresent;

	MPP_CreateEMVDataRecord();
	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_9, 8, 0xFF);
}


void MPP_S9_9(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_TryAgain;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_DBG_Put_Process(MPP_STATE_9, 9, 0xFF);
}


void MPP_S9_10(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;
	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;
	glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(mpp_depRspCode);
	glv_tagDF8115.Value[5]=MPP_EID_MOE_TryAgain;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_9, 10, 0xFF);
}


void MPP_S9_11(void)
{

//	Same As MPP_S11_13

	UINT	lenOfDRDOL=0;
	UCHAR	lenDolRelData=0;
	UCHAR	bufDolRelData[MPP_DOL_BUFFERSIZE]={0};
	UCHAR	tlvBuffer[3+3+MPP_LENGTH_TornTempRecord]={0};
	UCHAR	lenOfT=0;
	UCHAR	rspCode=FALSE;
	UCHAR	*ptrData=NULLPTR;
	UINT	lenOfData=0;
	UINT	lstLength=69;
	UCHAR	lstTorn[69]={
			0x9F, 0x02,			//Amount, Authorized (Numeric)
			0x9F, 0x03,			//Amount, Other (Numeric)
			0x5A,				//Application PAN
			0x5F, 0x34,			//Application PAN Sequence Number
			0xDF, 0x81, 0x04,	//Balance Read Before Gen AC
			0xDF, 0x81, 0x07,	//CDOL1 Related Data
			0x9F, 0x34,			//CVM Results
			0xDF, 0x81, 0x13,	//DRDOL Related Data
			0x9F, 0x7D,			//DS Summary 1
			0xDF, 0x81, 0x28,	//IDS Status
			0x9F, 0x1E,			//Interface Device Serial Number
			0xDF, 0x81, 0x11,	//PDOL Related Data
			0xDF, 0x81, 0x14,	//Reference Control Parameter
			0x9F, 0x33,			//Terminal Capabilities
			0x9F, 0x1A,			//Terminal Country Code
			0x9F, 0x35,			//Terminal Type
			0x95,				//Terminal Verification Results
			0x9F, 0x53,			//Transaction Category Code
			0x5F, 0x2A,			//Transaction Currency Code
			0x9A,				//Transaction Date
			0x9F, 0x21,			//Transaction Time
			0x9C,				//Transaction Type
			0x9F, 0x37,			//Unpredictable Number
			0xDF, 0x83,	0x01,	//Terminal Relay Resistance Entropy
			0xDF, 0x83,	0x02,	//Device Relay Resistance Entropy
			0xDF, 0x83, 0x03,	//Min Time For Processing Relay Resistance APDU
			0xDF, 0x83,	0x04,	//Max Time For Processing Relay Resistance APDU
			0xDF, 0x83, 0x05,	//Device Estimated Transmission Time For Relay Resistance R-APDU
			0xDF, 0x83,	0x06,	//Measured Relay Resistance Processing Time
			0xDF, 0x83, 0x07	//RRP Counter
	};		

	UT_Get_TLVLengthOfV(glv_tag9F51.Length, &lenOfDRDOL);
	
	MPP_DOL_Get_DOLRelatedData(lenOfDRDOL, glv_tag9F51.Value, &lenDolRelData, bufDolRelData);

	UT_Set_TagLength(lenDolRelData, glv_tagDF8113.Length);
	memcpy(glv_tagDF8113.Value, bufDolRelData, lenDolRelData);
	MPP_AddToList(mpp_tofDF8113, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	//Initialize(Torn Temp Record)
	memset(mpp_tagTornTempRecord.Length, 0, 3+MPP_LENGTH_TornTempRecord);
	
	//Set Pointer
	ptrData=lstTorn;
	
	do {
		UT_Get_TLVLengthOfT(ptrData, &lenOfT);

		rspCode=MPP_IsNotEmpty(ptrData);
		if (rspCode == TRUE)
		{
			memset(tlvBuffer, 0, 3+3+MPP_LENGTH_TornTempRecord);
			MPP_GetTLV(ptrData, tlvBuffer);

			MPP_AddToList(tlvBuffer, mpp_tagTornTempRecord.Value, MPP_LISTTYPE_TLV);
		}

		//Point to Next Tag
		ptrData+=lenOfT;
		lenOfData+=lenOfT;
	} while (lenOfData < lstLength);

	MPP_DBG_Put_Process(MPP_STATE_9, 11, 0xFF);
}


void MPP_S9_13(void)
{

//	Same As MPP_S11_15

	UCHAR	idxAID=0;
	UCHAR	idxRecord=0xFF;
	UCHAR	rspCode=FAIL;
	UINT	lenOfV=0;
	
	idxAID=MPP_Get_TornRecordAIDIndex();

	if (mpp_trnRecNum[idxAID] == glv_tagDF811D.Value[0])
	{
		rspCode=MPP_Search_TornRecord_Oldest(&idxRecord);
		if (rspCode == SUCCESS)
		{
			//Copy oldest record of Torn Transaction Log to Torn Record
			if (mpp_trnRec[idxAID][idxRecord].RecLen <= ECL_LENGTH_FF8101)
			{
				UT_Set_TagLength(mpp_trnRec[idxAID][idxRecord].RecLen, glv_tagFF8101.Length);
				memcpy(glv_tagFF8101.Value, mpp_trnRec[idxAID][idxRecord].Record, mpp_trnRec[idxAID][idxRecord].RecLen);

				MPP_AddToList(mpp_tofFF8101, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			}
			
			//Replace oldest record of Torn Transaction Log with Torn Temp Record
			MPP_Remove_TornRecord(idxRecord);

			UT_Get_TLVLengthOfV(mpp_tagTornTempRecord.Length, &lenOfV);
			MPP_Add_TornRecord(lenOfV, mpp_tagTornTempRecord.Value);
		}
	}
	else
	{
		UT_Get_TLVLengthOfV(mpp_tagTornTempRecord.Length, &lenOfV);
		MPP_Add_TornRecord(lenOfV, mpp_tagTornTempRecord.Value);
	}

	MPP_DBG_Put_Process(MPP_STATE_9, 13, 0xFF);
}


void MPP_S9_14(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_TryAgain;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_DBG_Put_Process(MPP_STATE_9, 14, 0xFF);
}


void MPP_S9_15(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;
	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;
	glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(mpp_depRspCode);
	glv_tagDF8115.Value[5]=MPP_EID_MOE_TryAgain;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_9, 15, 0xFF);
}


UCHAR MPP_S9_16(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=UT_Check_SW12(&mpp_depRcvData[mpp_depRcvLen-2], STATUSWORD_9000);

	MPP_DBG_Put_Process(MPP_STATE_9, 16, rspCode);
	
	return rspCode;
}


void MPP_S9_17(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_StatusBytes;
	memcpy(&glv_tagDF8115.Value[3], &mpp_depRcvData[mpp_depRcvLen-2], 2);

	MPP_DBG_Put_Process(MPP_STATE_9, 17, 0xFF);
}


void MPP_S9_18(void)
{

//	Same As MPP_S11_8

	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOf9F10=0;
	UCHAR	flgIsPresent=FALSE;
	UCHAR	flgIsNotEmpty=FALSE;
	UCHAR	*ptrData=NULLPTR;
	
	mpp_flgParResult=FALSE;
	
	if ((mpp_depRcvLen > 0) && (mpp_depRcvData[0] == 0x77))
	{
		mpp_flgParResult=MPP_ParseAndStoreCardResponse(mpp_depRcvLen, mpp_depRcvData);
	}
	else
	{
		if ((mpp_depRcvLen > 0) && (mpp_depRcvData[0] == 0x80))
		{
			UT_Get_TLVLength(mpp_depRcvData, &lenOfT, &lenOfL, &lenOfV);

			//Check Receive Length = Tag Length + SW12
			if ((mpp_depRcvLen != (lenOfT+lenOfL+lenOfV+2)) || (lenOfV == 0))
			{
				mpp_flgParResult=FALSE;
			}
			else
			{
				if (lenOfV < (ECL_LENGTH_9F27+ECL_LENGTH_9F36+ECL_LENGTH_9F26))
				{
					mpp_flgParResult=FALSE;
				}
				else
				{
					//Point to TLV-V of Tag 80
					ptrData=mpp_depRcvData+lenOfT+lenOfL;
					
					//Cryptogram Information Data
					flgIsPresent=MPP_IsPresent(mpp_tof9F27);
					flgIsNotEmpty=MPP_IsNotEmpty(mpp_tof9F27);

					if ((flgIsPresent == TRUE) || (flgIsNotEmpty == TRUE))
					{
						mpp_flgParResult=FALSE;
					}
					else
					{
						glv_tag9F27.Length[0]=ECL_LENGTH_9F27;
						memcpy(glv_tag9F27.Value, ptrData, ECL_LENGTH_9F27);
						MPP_AddToList(mpp_tof9F27, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
						ptrData+=ECL_LENGTH_9F27;

						//Application Transaction Counter
						flgIsPresent=MPP_IsPresent(mpp_tof9F36);
						flgIsNotEmpty=MPP_IsNotEmpty(mpp_tof9F36);

						if ((flgIsPresent == TRUE) || (flgIsNotEmpty == TRUE))
						{
							mpp_flgParResult=FALSE;
						}
						else
						{
							glv_tag9F36.Length[0]=ECL_LENGTH_9F36;
							memcpy(glv_tag9F36.Value, ptrData, ECL_LENGTH_9F36);
							MPP_AddToList(mpp_tof9F36, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
							ptrData+=ECL_LENGTH_9F36;

							//Application Cryptogram
							flgIsPresent=MPP_IsPresent(mpp_tof9F26);
							flgIsNotEmpty=MPP_IsNotEmpty(mpp_tof9F26);

							if ((flgIsPresent == TRUE) || (flgIsNotEmpty == TRUE))
							{
								mpp_flgParResult=FALSE;
							}
							else
							{
								glv_tag9F26.Length[0]=ECL_LENGTH_9F26;
								memcpy(glv_tag9F26.Value, ptrData, ECL_LENGTH_9F26);
								MPP_AddToList(mpp_tof9F26, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
								ptrData+=ECL_LENGTH_9F26;

								//Issuer Application Data
								flgIsNotEmpty = MPP_IsNotEmpty(mpp_tof9F10);

								if(((lenOfV > (ECL_LENGTH_9F27 + ECL_LENGTH_9F36 + ECL_LENGTH_9F26)) && (flgIsNotEmpty == TRUE)) ||
								   (lenOfV > (ECL_LENGTH_9F27 + ECL_LENGTH_9F36 + ECL_LENGTH_9F26 + ECL_LENGTH_9F10)))
								{
									mpp_flgParResult = FALSE;
								}
								else
								{
									if(lenOfV > (ECL_LENGTH_9F27 + ECL_LENGTH_9F36 + ECL_LENGTH_9F26))
									{
										lenOf9F10 = lenOfV - (ECL_LENGTH_9F27 + ECL_LENGTH_9F36 + ECL_LENGTH_9F26);

										if(lenOf9F10 <= ECL_LENGTH_9F10)
										{
											glv_tag9F10.Length[0] = lenOf9F10;
											memcpy(glv_tag9F10.Value, ptrData, lenOf9F10);
											MPP_AddToList(mpp_tof9F10, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
										}
									}
								}
								
								mpp_flgParResult = TRUE;
							}
						}
					}
				}
			}
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_9, 18, 0xFF);
}


UCHAR MPP_S9_19(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_flgParResult == TRUE)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_9, 19, rspCode);
	
	return rspCode;
}


void MPP_S9_20(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_ParsingError;

	MPP_DBG_Put_Process(MPP_STATE_9, 20, 0xFF);
}


UCHAR MPP_S9_21(void)
{
	UCHAR	flgIsNotEmpty_9F36=FALSE;
	UCHAR	flgIsNotEmpty_9F27=FALSE;
	UCHAR	rspCode=FALSE;
	
	flgIsNotEmpty_9F36=MPP_IsNotEmpty(mpp_tof9F36);
	flgIsNotEmpty_9F27=MPP_IsNotEmpty(mpp_tof9F27);

	if ((flgIsNotEmpty_9F36 == TRUE) && (flgIsNotEmpty_9F27 == TRUE))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_9, 21, rspCode);
	
	return rspCode;
}


void MPP_S9_22(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_DBG_Put_Process(MPP_STATE_9, 22, 0xFF);
}


UCHAR MPP_S9_23(void)
{

//	Same As MPP_S10_17
//	Same As MPP_S11_20

	UCHAR	rspCode=FALSE;

	if ((((glv_tag9F27.Value[0] & 0xC0) == 0x40) && ((glv_tagDF8114.Value[0] & 0xC0) == MPP_ACT_TC))
		||
		(((glv_tag9F27.Value[0] & 0xC0) == 0x80) && (((glv_tagDF8114.Value[0] & 0xC0) == MPP_ACT_TC) || ((glv_tagDF8114.Value[0] & 0xC0) == MPP_ACT_ARQC)))
		||
		((glv_tag9F27.Value[0] & 0xC0) == 0x00))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_9, 23, rspCode);

	return rspCode;
}


void MPP_S9_24(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataError;

	MPP_DBG_Put_Process(MPP_STATE_9, 24, 0xFF);
}


void MPP_S9_25(void)
{
	UCHAR	rspCode=0;
	
	rspCode=MPP_Procedure_PostGenACBalanceReading();
	if (rspCode == MPP_STATE_17)
	{
		while (1)
		{
			rspCode=MPP_State_17_WaitingForPostGenACBalance();
			if (rspCode == MPP_STATE_RETURN)
				break;
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_9, 25, 0xFF);
}


UCHAR MPP_S9_26(void)
{
	UCHAR	rspCode=FALSE;

	//rspCode=MPP_IsNotEmpty(mpp_tofFF8103);
	rspCode = MPP_IsNotEmptyList(glv_tagFF8103.Value);		// william 2017/10/31	fix the consistency of the judgement  
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_9, 26, rspCode);

	return rspCode;
}


void MPP_S9_27(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_ClearDisplay;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_CardReadSuccessfully;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_MSG(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_9, 27, 0xFF);
}


UCHAR MPP_S9_28(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof9F4B);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_9, 28, rspCode);

	return rspCode;
}


UCHAR MPP_S10_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_10, 1, rspCode);
	
	return rspCode;
}


UCHAR MPP_S10_2(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_RA)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_10, 2, rspCode);
	
	return rspCode;
}


UCHAR MPP_S10_3(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_10, 3, rspCode);
	
	return rspCode;
}


UCHAR MPP_S10_4(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_DET)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_10, 4, rspCode);
	
	return rspCode;
}


void MPP_S10_5(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_TryAgain;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_DBG_Put_Process(MPP_STATE_10, 5, 0xFF);
}


void MPP_S10_6(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;
	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;
	glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(mpp_depRspCode);
	glv_tagDF8115.Value[5]=MPP_EID_MOE_TryAgain;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_10, 6, 0xFF);
}


UCHAR MPP_S10_7(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=UT_Check_SW12(&mpp_depRcvData[mpp_depRcvLen-2], STATUSWORD_9000);

	MPP_DBG_Put_Process(MPP_STATE_10, 7, rspCode);
	
	return rspCode;
}


void MPP_S10_8(void)
{
	MPP_Procedure_PrepareGenACCommand();

	MPP_DBG_Put_Process(MPP_STATE_10, 8, 0xFF);
}


void MPP_S10_9(void)
{
	UINT	lenOfV=0;

	UT_Get_TLVLengthOfV(mpp_tempSend.Length, &lenOfV);

	MPP_Clear_DEPBuffer();
	//mpp_depRspCode=MPP_APDU_GENERATE_AC(glv_tagDF8114.Value[0], lenOfV, glv_tagDF8107.Value, &mpp_depRcvLen, mpp_depRcvData);
	mpp_depRspCode = MPP_APDU_GENERATE_AC(glv_tagDF8114.Value[0], lenOfV, mpp_tempSend.Value, &mpp_depRcvLen, mpp_depRcvData);

	mpp_vfyDataLen=mpp_depRcvLen;
	memcpy(mpp_vfyData, mpp_depRcvData, mpp_depRcvLen);

	MPP_DBG_Put_Process(MPP_STATE_10, 9, 0xFF);
}


void MPP_S10_10(void)
{
	MPP_Remove_TornRecord(mpp_tagTornEntry.Value[0]);

	MPP_DBG_Put_Process(MPP_STATE_10, 10, 0xFF);
}


void MPP_S10_11(void)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfObject=0;
	UINT	lenOfData=0;
	UINT	recIndex=0;
	UCHAR	*ptrObject=NULLPTR;
	//UCHAR	*ptrRelData=NULLPTR;
	UCHAR	tmpTLV[3+3+1024]={0};

	//Torn Temp Record
	UT_Get_TLVLengthOfV(mpp_tagTornTempRecord.Length, &lenOfObject);

	ptrObject=mpp_tagTornTempRecord.Value;
	lenOfData=0;

	do {
		UT_Get_TLVLength(ptrObject, &lenOfT, &lenOfL, &lenOfV);

		//Copy TLV from Torn Temp Record
		memset(tmpTLV, 0, 3+3+1024);
		memcpy(tmpTLV, ptrObject, lenOfT+lenOfL+lenOfV);

		//Get Tag Index in Tag Table
		UT_Search_Record(lenOfT, ptrObject, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);

		//Store Tag TLV-L & TLV-V
		MPP_Store_Tag(lenOfL, lenOfV, &tmpTLV[lenOfT], recIndex);

		//Add to Present List
		MPP_AddToList(ptrObject, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

		//Point to Next Tag
		ptrObject+=(lenOfT+lenOfL+lenOfV);
		lenOfData+=(lenOfT+lenOfL+lenOfV);
	} while (lenOfData < lenOfObject);

	//Tammy 2017//11/30	Ignore to update TLV-L and TLV-V from PDOL Related Data, CDOL1 Related Data and DRDOL Related Data
	/*
	//PDOL
	UT_Get_TLVLengthOfV(glv_tag9F38.Length, &lenOfObject);

	ptrObject=glv_tag9F38.Value;
	ptrRelData=glv_tagDF8111.Value;
	lenOfData=0;

	do {
		//Get TLV Length of Data Object
		UT_Get_TLVLengthOfT(ptrObject, &lenOfT);
		lenOfL=1;
		lenOfV=ptrObject[lenOfT];

		//Copy TLV-T & TLV-L from PDOL and Copy TLV-V from PDOL Related Data
		memset(tmpTLV, 0, 3+3+1024);
		memcpy(tmpTLV, ptrObject, lenOfT+lenOfL);
		memcpy(&tmpTLV[lenOfT+lenOfL], ptrRelData, lenOfV);

		//Get Tag Index in Tag Table
		UT_Search_Record(lenOfT, ptrObject, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);

		//Store Tag TLV-L & TLV-V
		MPP_Store_Tag(lenOfL, lenOfV, &tmpTLV[lenOfT], recIndex);

		//Add to Present List
		MPP_AddToList(ptrObject, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

		//Point to Next Tag
		ptrObject+=(lenOfT+lenOfL);
		ptrRelData+=lenOfV;
		lenOfData+=(lenOfT+lenOfL);
	} while (lenOfData < lenOfObject);

	//CDOL1
	UT_Get_TLVLengthOfV(glv_tag8C.Length, &lenOfObject);

	ptrObject=glv_tag8C.Value;
	ptrRelData=glv_tagDF8107.Value;
	lenOfData=0;

	do {
		//Get TLV Length of Data Object
		UT_Get_TLVLengthOfT(ptrObject, &lenOfT);
		lenOfL=1;
		lenOfV=ptrObject[lenOfT];

		//Copy TLV-T & TLV-L from CDOL1 and Copy TLV-V from CDOL1 Related Data
		memset(tmpTLV, 0, 3+3+1024);
		memcpy(tmpTLV, ptrObject, lenOfT+lenOfL);
		memcpy(&tmpTLV[lenOfT+lenOfL], ptrRelData, lenOfV);

		//Get Tag Index in Tag Table
		UT_Search_Record(lenOfT, ptrObject, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);

		//Store Tag TLV-L & TLV-V
		MPP_Store_Tag(lenOfL, lenOfV, &tmpTLV[lenOfT], recIndex);

		//Add to Present List
		MPP_AddToList(ptrObject, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

		//Point to Next Tag
		ptrObject+=(lenOfT+lenOfL);
		ptrRelData+=lenOfV;
		lenOfData+=(lenOfT+lenOfL);
	} while (lenOfData < lenOfObject);

	//DRDOL
	UT_Get_TLVLengthOfV(glv_tag9F51.Length, &lenOfObject);

	ptrObject=glv_tag9F51.Value;
	ptrRelData=glv_tagDF8113.Value;
	lenOfData=0;

	do {
		//Get TLV Length of Data Object
		UT_Get_TLVLengthOfT(ptrObject, &lenOfT);
		lenOfL=1;
		lenOfV=ptrObject[lenOfT];

		//Copy TLV-T & TLV-L from DRDOL and Copy TLV-V from DRDOL Related Data
		memset(tmpTLV, 0, 3+3+1024);
		memcpy(tmpTLV, ptrObject, lenOfT+lenOfL);
		memcpy(&tmpTLV[lenOfT+lenOfL], ptrRelData, lenOfV);

		//Get Tag Index in Tag Table
		UT_Search_Record(lenOfT, ptrObject, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);

		//Store Tag TLV-L & TLV-V
		MPP_Store_Tag(lenOfL, lenOfV, &tmpTLV[lenOfT], recIndex);

		//Add to Present List
		MPP_AddToList(ptrObject, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

		//Point to Next Tag
		ptrObject+=(lenOfT+lenOfL);
		ptrRelData+=lenOfV;
		lenOfData+=(lenOfT+lenOfL);
	} while (lenOfData < lenOfObject);
	*/

	MPP_DBG_Put_Process(MPP_STATE_10, 11, 0xFF);
}


void MPP_S10_12(void)
{
	mpp_flgParResult=FALSE;

	if ((mpp_depRcvLen > 0) && (mpp_depRcvData[0] == 0x77))
	{
		mpp_flgParResult=MPP_ParseAndStoreCardResponse(mpp_depRcvLen, mpp_depRcvData);
	}

	MPP_DBG_Put_Process(MPP_STATE_10, 12, 0xFF);
}


UCHAR MPP_S10_13(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_flgParResult == TRUE)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_10, 13, rspCode);
	
	return rspCode;
}


void MPP_S10_14(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_ParsingError;

	MPP_DBG_Put_Process(MPP_STATE_10, 14, 0xFF);
}


UCHAR MPP_S10_15(void)
{
	UCHAR	flgIsNotEmpty_9F36=FALSE;
	UCHAR	flgIsNotEmpty_9F27=FALSE;
	UCHAR	rspCode=FALSE;
	
	flgIsNotEmpty_9F36=MPP_IsNotEmpty(mpp_tof9F36);
	flgIsNotEmpty_9F27=MPP_IsNotEmpty(mpp_tof9F27);

	if ((flgIsNotEmpty_9F36 == TRUE) && (flgIsNotEmpty_9F27 == TRUE))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_10, 15, rspCode);

	return rspCode;
}


void MPP_S10_16(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_DBG_Put_Process(MPP_STATE_10, 16, 0xFF);
}


UCHAR MPP_S10_17(void)
{

//	Same As MPP_S9_23
//	Same As MPP_S11_20

	UCHAR	rspCode=FALSE;
	
	if ((((glv_tag9F27.Value[0] & 0xC0) == 0x40) && ((glv_tagDF8114.Value[0] & 0xC0) == MPP_ACT_TC))
		||
		(((glv_tag9F27.Value[0] & 0xC0) == 0x80) && (((glv_tagDF8114.Value[0] & 0xC0) == MPP_ACT_TC) || ((glv_tagDF8114.Value[0] & 0xC0) == MPP_ACT_ARQC)))
		||
		((glv_tag9F27.Value[0] & 0xC0) == 0x00))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_10, 17, rspCode);

	return rspCode;
}


void MPP_S10_18(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataError;

	MPP_DBG_Put_Process(MPP_STATE_10, 18, 0xFF);
}


void MPP_S10_19(void)
{
	UCHAR	rspCode=0;
	
	rspCode=MPP_Procedure_PostGenACBalanceReading();
	if (rspCode == MPP_STATE_17)
	{
		while (1)
		{
			rspCode=MPP_State_17_WaitingForPostGenACBalance();
			if (rspCode == MPP_STATE_RETURN)
				break;
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_10, 19, 0xFF);
}


UCHAR MPP_S10_20(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tofFF8103);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_10, 20, rspCode);
	
	return rspCode;
}


void MPP_S10_21(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_ClearDisplay;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_CardReadSuccessfully;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_MSG(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_10, 21, 0xFF);
}


UCHAR MPP_S10_22(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof9F4B);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_10, 22, rspCode);

	return rspCode;
}


UCHAR MPP_S910_1(UCHAR *optModLen, UCHAR *optModulus, UCHAR *optExponent)
{
	UCHAR	cauModLen=0;
	UCHAR	issModLen=0;
	UCHAR	cauModulus[255]={0};
	UCHAR	issModulus[255]={0};
	UCHAR	cauExponent[3]={0};
	UCHAR	issExponent[3]={0};	
	UCHAR	rspCode=FALSE;

	rspCode=MPP_Retrieve_PK_CA(glv_tag9F06.Value, glv_tag8F.Value[0], &cauModLen, cauModulus, cauExponent);
	if (rspCode == SUCCESS)
	{
		rspCode=MPP_Retrieve_PK_Issuer(cauModLen, cauModulus, cauExponent, &issModLen, issModulus, issExponent);
		if (rspCode == SUCCESS)
		{
			rspCode=MPP_Retrieve_PK_ICC(issModLen, issModulus, issExponent, optModLen, optModulus, optExponent);
			if (rspCode == SUCCESS)
			{
				rspCode=TRUE;
			}
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_910_A, 1, rspCode);

	return rspCode;
}


UCHAR MPP_S910_2(void)
{
	UCHAR	rspCode=FALSE;

	if (glv_tagDF8128.Value[0] & MPP_IDS_Read)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 2, rspCode);

	return rspCode;
}


UCHAR MPP_S910_2_1(void)
{
	UCHAR	rspCode = FALSE;

	if(glv_tag95.Value[4] & MPP_TVR_RRPPerformed)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 2, rspCode);

	return rspCode;
}


UCHAR MPP_S910_2_2(void)
{
	UCHAR	rspCode = FALSE;

	if(glv_tag95.Value[4] & MPP_TVR_RRPPerformed)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 2, rspCode);

	return rspCode;
}


void MPP_S910_3(UCHAR iptModLen, UCHAR *iptModulus, UCHAR *iptExponent, UCHAR flgIDSRead, UCHAR flgRRP)
{
	mpp_flgVfySDAD = MPP_Verify_DynamicSignature(iptModLen, iptModulus, iptExponent, flgIDSRead, flgRRP);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 3, 0xFF);
}


void MPP_S910_3_1(UCHAR iptModLen, UCHAR *iptModulus, UCHAR *iptExponent, UCHAR flgIDSRead, UCHAR flgRRP)
{
	mpp_flgVfySDAD=MPP_Verify_DynamicSignature(iptModLen, iptModulus, iptExponent, flgIDSRead, flgRRP);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 3, 0xFF);
}


void MPP_S910_4(UCHAR iptModLen, UCHAR *iptModulus, UCHAR *iptExponent, UCHAR flgIDSRead, UCHAR flgRRP)
{
	mpp_flgVfySDAD=MPP_Verify_DynamicSignature(iptModLen, iptModulus, iptExponent, flgIDSRead, flgRRP);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 4, 0xFF);
}


void MPP_S910_4_1(UCHAR iptModLen, UCHAR *iptModulus, UCHAR *iptExponent, UCHAR flgIDSRead, UCHAR flgRRP)
{
	mpp_flgVfySDAD=MPP_Verify_DynamicSignature(iptModLen, iptModulus, iptExponent, flgIDSRead, flgRRP);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 4, 0xFF);
}


UCHAR MPP_S910_5(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_flgVfySDAD == TRUE)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 5, rspCode);
	
	return rspCode;
}


UCHAR MPP_S910_6(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_flgVfySDAD == TRUE)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 6, rspCode);
	
	return rspCode;
}


void MPP_S910_7(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CamFailed;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 7, 0xFF);
}


void MPP_S910_7_1(void)
{
	glv_tag95.Value[0] |= MPP_TVR_CDAFailed;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 7, 0xFF);
}


UCHAR MPP_S910_8(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsPresent(mpp_tofDF8101);
	
	MPP_DBG_Put_Process(MPP_STATE_910_A, 8, rspCode);
	
	return rspCode;
}


void MPP_S910_9(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 9, 0xFF);
}


UCHAR MPP_S910_10(void)
{
	UINT	lenOf9F7D=0;
	UINT	lenOfDF8101=0;
	UCHAR	rspCode=0xFF;

	UT_Get_TLVLengthOfV(glv_tag9F7D.Length, &lenOf9F7D);
	UT_Get_TLVLengthOfV(glv_tagDF8101.Length, &lenOfDF8101);

	if (lenOf9F7D == lenOfDF8101)
	{
		rspCode=UT_bcdcmp(glv_tag9F7D.Value, glv_tagDF8101.Value, lenOf9F7D);
		if (rspCode == 0)
		{
			rspCode=TRUE;
		}
		else
		{
			rspCode=FALSE;
		}
	}
	else
	{
		rspCode=FALSE;
	}

	MPP_DBG_Put_Process(MPP_STATE_910_A, 10, rspCode);
	
	return rspCode;
}


void MPP_S910_11(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_IdsReadError;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 11, 0xFF);
}


void MPP_S910_12(void)
{
	glv_tagDF810B.Value[0]|=MPP_DSS_SuccessfulRead;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 12, 0xFF);
}


UCHAR MPP_S910_13(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8128.Value[0] & MPP_IDS_Write)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 13, rspCode);

	return rspCode;
}


UCHAR MPP_S910_14(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsPresent(mpp_tofDF8102);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 14, rspCode);

	return rspCode;
}


void MPP_S910_15(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 15, 0xFF);
}


UCHAR MPP_S910_16(void)
{
	UINT	lenOfDF8101=0;
	UINT	lenOfDF8102=0;
	UCHAR	rspCode=0xFF;

	UT_Get_TLVLengthOfV(glv_tagDF8101.Length, &lenOfDF8101);
	UT_Get_TLVLengthOfV(glv_tagDF8102.Length, &lenOfDF8102);

	if (lenOfDF8101 == lenOfDF8102)
	{
		rspCode=UT_bcdcmp(glv_tagDF8101.Value, glv_tagDF8102.Value, lenOfDF8101);
		if (rspCode == 0)
		{
			rspCode=TRUE;
		}
		else
		{
			rspCode=FALSE;
		}
	}
	else
	{
		rspCode=FALSE;
	}

	MPP_DBG_Put_Process(MPP_STATE_910_A, 16, rspCode);

	return rspCode;
}


void MPP_S910_17(void)
{
	glv_tagDF810B.Value[0]|=MPP_DSS_SuccessfulWrite;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 17, 0xFF);
}


UCHAR MPP_S910_18(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF810A.Value[0] & MPP_DOI_StopIfWriteFailed)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 18, rspCode);

	return rspCode;
}


void MPP_S910_19(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_IdsWriteError;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 19, 0xFF);
}


UCHAR MPP_S910_30(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof9F26);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 30, rspCode);

	return rspCode;
}


void MPP_S910_31(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 31, 0xFF);
}


UCHAR MPP_S910_32(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((glv_tag9F27.Value[0] & 0xC0) == 0x00)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 32, rspCode);

	return rspCode;
}


UCHAR MPP_S910_33(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8128.Value[0] & MPP_IDS_Read)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 33, rspCode);

	return rspCode;
}


UCHAR MPP_S910_34(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8114.Value[0] & MPP_RCP_CDASignatureRequested)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 34, rspCode);

	return rspCode;
}


UCHAR MPP_S910_35(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((glv_tagDF8114.Value[0] & 0xC0) == MPP_RCP_ACTYPE_AAC)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 35, rspCode);

	return rspCode;
}


UCHAR MPP_S910_36(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8114.Value[0] & MPP_RCP_CDASignatureRequested)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 36, rspCode);

	return rspCode;
}


void MPP_S910_37(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataError;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 37, 0xFF);
}


UCHAR MPP_S910_38(void)
{
	UCHAR	rspCode = FALSE;

	if(glv_tag95.Value[4] & MPP_TVR_RRPPerformed)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 38, rspCode);

	return rspCode;
}


void MPP_S910_39(void)
{
	//	Same As MPP_S11_79

	UCHAR	flgIsNotEmpty = FALSE;
	UCHAR	*ptrData = NULLPTR;
	UINT	lenOf57 = 0;
	UCHAR	ascTrack2[ECL_LENGTH_57 * 2] = {0};
	UCHAR	datIndex = 0;
	UCHAR	lenOfPAN = 0;
	UCHAR	flgIsNotEmpty_8F = FALSE;
	UCHAR	*ptrDD = NULLPTR;
	UINT	twoLSB = 0;
	UCHAR	ascBuf[5];
	UCHAR	lenOfAscBuf = 0;
	UINT	lenOf5A = 0;
	UCHAR	thirdLSB = 0;
	UINT	millisecond_DF8306 = 0;

	flgIsNotEmpty = MPP_IsNotEmpty(mpp_tof57);
	if(flgIsNotEmpty == TRUE)
	{
		//Point to Track 2
		ptrData = glv_tag57.Value;

		//Get Length of Track 2
		UT_Get_TLVLengthOfV(glv_tag57.Length, &lenOf57);

		//Split to Ascii for Processing
		UT_Split(ascTrack2, glv_tag57.Value, lenOf57);

		//Find Number of Digits in 'Primary Account Number'
		for(datIndex = 0 ; datIndex < (lenOf57 * 2) ; datIndex++)
		{
			if(ascTrack2[datIndex] == 'D')
			{
				lenOfPAN = datIndex;
				break;
			}
		}

		//Move Pointer
		datIndex += 1;	//Point to Start of Expiry Date
		datIndex += 4;	//Point to Start of Service Code
		datIndex += 3;	//Point to Start of Discretionary Data
		
		ptrDD = &ascTrack2[datIndex];	//Point to the most significant digit of the Discretionary Data

		if(lenOfPAN <= 16)
		{
			memset(&ascTrack2[datIndex], '0', 13);
			datIndex += 13;
			if((datIndex % 2) != 0)
			{
				memcpy(&ascTrack2[datIndex], (UCHAR *)'F', 1);
				datIndex++;
			}
		}
		else
		{
			memset(&ascTrack2[datIndex], '0', 10);
			datIndex += 10;
			if((datIndex % 2) != 0)
			{
				memcpy(&ascTrack2[datIndex], (UCHAR *)'F', 1);
				datIndex++;
			}
		}

		flgIsNotEmpty_8F = MPP_IsNotEmpty(mpp_tof8F);		//CA Public Key Index (Card)
		if((flgIsNotEmpty_8F == TRUE) && (glv_tag8F.Value[0] < 0x0A))
		{
			/*
			  Replace the most significant digit of the 'Discretionary Data' in Track 2 Equivalent Data with a digit representing CA Public 
			  Key Index (Card).
			*/
			*ptrDD = glv_tag8F.Value[0] | '0';
		}
		ptrDD++;

		//Replace the second most significant digit of the 'Discretionary Data' in Track 2 Equivalent Data with a digit representing RRP Counter.
		*ptrDD = glv_tagDF8307.Value[0] | '0';
		ptrDD++;

		/*
		  Convert the two least significant bytes of the Device Relay Resistance Entropy from 2 byte binary to 5 digit decimal by 
		  considering the two bytes as an integer in the range 0 to 65535. Replace the 5 digits of 'Discretionary Data' in Track 2 
		  Equivalent Data that follow the RRP Counter digit with that value.
		*/
		twoLSB = glv_tagDF8302.Value[2] * 256 + glv_tagDF8302.Value[3];
		memset(ascBuf, '0', 5);
		lenOfAscBuf = UT_itoa(twoLSB, ascBuf);
		
		if(lenOfAscBuf < 5)
		{
			memset(ptrDD, '0', (5 - lenOfAscBuf));
			ptrDD += (5 - lenOfAscBuf);
		}

		memcpy(ptrDD, ascBuf, lenOfAscBuf);
		ptrDD += lenOfAscBuf;

		MPP_GetLength(mpp_tof5A, &lenOf5A);
		if(lenOfPAN <= 16)	//Tammy 2017/11/10	[SB-195-Errata for EMV Book C-2(Version 2.6)]
		{
			/*
			  Convert the third least significant byte of Device Relay Resistance Entropy from binary to 3 digit decimal in the range 0 to 255.
			  Replace the next 3 digits of 'Discretionary Data' in Track 2 Equivalent Data with that value.
			*/
			thirdLSB = glv_tagDF8302.Value[1];
			memset(ascBuf, '0', 5);
			lenOfAscBuf = UT_itoa((UINT)thirdLSB, ascBuf);
			
			if(lenOfAscBuf < 3)
			{
				memset(ptrDD, '0', (3 - lenOfAscBuf));
				ptrDD += (3 - lenOfAscBuf);
			}

			memcpy(ptrDD, ascBuf, lenOfAscBuf);
			ptrDD += lenOfAscBuf;
		}

		/*
		  Divide the Measured Relay Resistance Processing Time by 10 using the div operator to give a count in milliseconds.
		  If the value exceeds '03E7' (999), then set the value to '03E7'.
		*/
		millisecond_DF8306  = (glv_tagDF8306.Value[0] * 256 + glv_tagDF8306.Value[1]) / 10;
		if(millisecond_DF8306 > 0x03E7)	//999
		{
			millisecond_DF8306 = 0x03E7;
		}

		/*
		  Convert this value from 2 byte binary to 3 digit decimal by considering the 2 bytes as an integer. 
		  Replace the 3 least significant digits of 'Discretionary Data' in Track 2 Equivalent Data with this 3 digit decimal value. 
		*/
		memset(ascBuf, '0', 5);
		lenOfAscBuf = UT_itoa(millisecond_DF8306, ascBuf);

		if(lenOfAscBuf < 3)
		{
			memset(ptrDD, '0', (3 - lenOfAscBuf));
			ptrDD += (3 - lenOfAscBuf);
		}

		memcpy(ptrDD, ascBuf, lenOfAscBuf);
		ptrDD += lenOfAscBuf;

		UT_Set_TagLength((datIndex / 2), glv_tag57.Length);
		UT_Compress(glv_tag57.Value, ascTrack2, ECL_LENGTH_57);
	}

	MPP_DBG_Put_Process(MPP_STATE_910_A, 39, 0xFF);
}


void MPP_S910_50(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;
	
	memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 50, 0xFF);
}


UCHAR MPP_S910_51(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8128.Value[0] & MPP_IDS_Write)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 51, rspCode);

	return rspCode;
}


void MPP_S910_52(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8129.Value[4]|=MPP_OPS_DataRecordPresent;

	MPP_CreateEMVDataRecord();
	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 52, 0xFF);
}


void MPP_S910_53(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;

	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 53, 0xFF);
}


void MPP_S910_61(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 61, 0xFF);
}


void MPP_S910_62(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;

	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 62, 0xFF);
}


void MPP_S910_70(void)
{
	glv_tagDF8129.Value[4]|=MPP_OPS_DataRecordPresent;

	MPP_CreateEMVDataRecord();

	MPP_DBG_Put_Process(MPP_STATE_910_A, 70, 0xFF);
}


UCHAR MPP_S910_71(void)
{
	UCHAR	flgIsNotEmpty=FALSE;
	UCHAR	flgIsNotZero=FALSE;
	UCHAR	tmpDF4B[ECL_LENGTH_DF4B]={0};
	UCHAR	mskDF4B[ECL_LENGTH_DF4B]={0x00,0x03,0x0F};
	UCHAR	zroDF4B[ECL_LENGTH_DF4B]={0};
	UCHAR	idxNum=0;
	UCHAR	rspCmp=0xFF;
	UCHAR	rspCode=FALSE;

	flgIsNotEmpty=MPP_IsNotEmpty(mpp_tofDF4B);

	for (idxNum=0; idxNum < 3; idxNum++)
	{
		tmpDF4B[idxNum]=(glv_tagDF4B.Value[idxNum] & mskDF4B[idxNum]);
	}

	rspCmp=UT_bcdcmp(tmpDF4B, zroDF4B, 3);
	if (rspCmp != 0)
	{
		flgIsNotZero=TRUE;
	}

	if ((flgIsNotEmpty == TRUE) && (flgIsNotZero == TRUE))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_910_A, 71, rspCode);
	
	return rspCode;
}


void MPP_S910_72(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 72, 0xFF);
}


void MPP_S910_73(void)
{
	UCHAR	optMessage=0;
	UCHAR	optStatus=0;
	UCHAR	rspCode=FALSE;

	rspCode=MPP_Check_PhoneMessageTable(&optMessage, &optStatus);
	if (rspCode == TRUE)
	{
		memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);
		glv_tagDF8116.Value[0]=optMessage;
		glv_tagDF8116.Value[1]=optStatus;
	}

	MPP_DBG_Put_Process(MPP_STATE_910_A, 73, 0xFF);
}


void MPP_S910_74(void)
{
	//Same As MPP_S11_114
	
	UCHAR	tmpUniID[2]={0x00,0x00};
	UCHAR	mskUniID[2]={0x80,0x00};
	UCHAR	zroUniID[2]={0x00,0x00};
	UCHAR	cmpDevType[2]={0x30,0x30};
	UCHAR	flgIsNotEmpty=FALSE;
	UCHAR	flgUniID=FALSE;
	UCHAR	flgDevType=FALSE;
	UCHAR	rspCode=0xFF;

	if ((glv_tag9F27.Value[0] & 0xC0) == 0x40)
	{
		glv_tagDF8129.Value[0]=MPP_OPS_STATUS_Approved;
	}
	else
	{
		if ((glv_tag9F27.Value[0] & 0xC0) == 0x80)
		{
			glv_tagDF8129.Value[0]=MPP_OPS_STATUS_OnlineRequest;
		}
		else
		{
//			if ((glv_tag9C.Value[0] == MPP_TXT_Purchase) ||
//				(glv_tag9C.Value[0] == MPP_TXT_CashBack) ||
//				(glv_tag9C.Value[0] == MPP_TXT_Cash))
			if((glv_tag9C.Value[0] == MPP_TXT_Cash) ||
			   (glv_tag9C.Value[0] == MPP_TXT_CashDisbursement) ||
			   (glv_tag9C.Value[0] == MPP_TXT_Purchase) ||
			   (glv_tag9C.Value[0] == MPP_TXT_CashBack))
			{
				flgIsNotEmpty=MPP_IsNotEmpty(mpp_tof9F6E);
				
				tmpUniID[0]=glv_tag9F6E.Value[2] & mskUniID[0];
				tmpUniID[1]=glv_tag9F6E.Value[3] & mskUniID[1];

				if (!memcmp(tmpUniID, zroUniID, 2))
				{
					flgUniID=TRUE;
				}

				//'Device Type' is present when the most significant bit of byte 1 of 'Unique Identifier' is set to 0b. 
				if ((glv_tag9F6E.Value[2] & 0x80) == 0)
				{
					rspCode=UT_bcdcmp(&glv_tag9F6E.Value[4], cmpDevType, 2);
					if (rspCode != 0)
					{
						flgDevType=TRUE;
					}
				}

				if (((flgIsNotEmpty == TRUE) &&
					(flgUniID == TRUE) &&
					(flgDevType == TRUE))
					||
					((glv_tag9F33.Value[0] & MPP_TRC_ICWithContacts) == 0))
				{
					glv_tagDF8129.Value[0]=MPP_OPS_STATUS_Declined;
				}
				else
				{
					glv_tagDF8129.Value[0]=MPP_OPS_STATUS_TryAnotherInterface;
				}
			}
			else
			{
				glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
			}
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_910_A, 74, 0xFF);
}


void MPP_S910_75(void)
{
	//Same As MPP_S11_115
	
	UCHAR	flgIsNotEmpty_DF8105=FALSE;
	UCHAR	flgIsNotEmpty_9F42=FALSE;
	UCHAR	flgIsNotEmpty_9F6E=FALSE;
	UCHAR	flgUniID=FALSE;
	UCHAR	flgDevType=FALSE;
	UCHAR	tmpUniID[2]={0x00,0x00};
	UCHAR	mskUniID[2]={0x80,0x00};
	UCHAR	zroUniID[2]={0x00,0x00};
	UCHAR	cmpDevType[2]={0x30,0x30};
	UCHAR	rspCode=0xFF;
	
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	if ((glv_tag9F27.Value[0] & 0xC0) == 0x40)
	{
		memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, ECL_LENGTH_DF812D);

		flgIsNotEmpty_DF8105=MPP_IsNotEmpty(mpp_tofDF8105);
		if (flgIsNotEmpty_DF8105 == TRUE)
		{
			glv_tagDF8116.Value[13]=MPP_UIR_QUALIFIER_Balance;
			memcpy(&glv_tagDF8116.Value[14], glv_tagDF8105.Value, ECL_LENGTH_DF8105);

			flgIsNotEmpty_9F42=MPP_IsNotEmpty(mpp_tof9F42);
			if (flgIsNotEmpty_9F42 == TRUE)
			{
				memcpy(&glv_tagDF8116.Value[20], glv_tag9F42.Value, ECL_LENGTH_9F42);
			}
		}

		if (glv_tagDF8129.Value[3] == MPP_OPS_CVM_ObtainSignature)
		{
			glv_tagDF8116.Value[0]=MPP_UIR_MID_Approved_Sign;
		}
		else
		{
			glv_tagDF8116.Value[0]=MPP_UIR_MID_Approved;
		}
	}
	else
	{
		if ((glv_tag9F27.Value[0] & 0xC0) == 0x80)
		{
			memset(&glv_tagDF8116.Value[2], 0x00, 3);
			glv_tagDF8116.Value[0]=MPP_UIR_MID_Authorising_PleaseWait;
		}
		else
		{
//			if ((glv_tag9C.Value[0] == MPP_TXT_Purchase) ||
//				(glv_tag9C.Value[0] == MPP_TXT_CashBack) ||
//				(glv_tag9C.Value[0] == MPP_TXT_Cash))
			if((glv_tag9C.Value[0] == MPP_TXT_Cash) ||
			   (glv_tag9C.Value[0] == MPP_TXT_CashDisbursement) ||
			   (glv_tag9C.Value[0] == MPP_TXT_Purchase) ||
			   (glv_tag9C.Value[0] == MPP_TXT_CashBack))
			{
				memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, ECL_LENGTH_DF812D);

				flgIsNotEmpty_9F6E=MPP_IsNotEmpty(mpp_tof9F6E);

				tmpUniID[0]=glv_tag9F6E.Value[2] & mskUniID[0];
				tmpUniID[1]=glv_tag9F6E.Value[3] & mskUniID[1];

				if (!memcmp(tmpUniID, zroUniID, 2))
				{
					flgUniID=TRUE;
				}

				//'Device Type' is present when the most significant bit of byte 1 of 'Unique Identifier' is set to 0b. 
				if ((glv_tag9F6E.Value[2] & 0x80) == 0)
				{
					rspCode=UT_bcdcmp(&glv_tag9F6E.Value[4], cmpDevType, 2);
					if (rspCode != 0)
					{
						flgDevType=TRUE;
					}
				}

				if (((flgIsNotEmpty_9F6E == TRUE) &&
					(flgUniID == TRUE) &&
					(flgDevType == TRUE))
					||
					((glv_tag9F33.Value[0] & MPP_TRC_ICWithContacts) == 0))
				{
					glv_tagDF8116.Value[0]=MPP_UIR_MID_Declined;
				}
				else
				{
					glv_tagDF8116.Value[0]=MPP_UIR_MID_InsertCard;
				}
			}
			else
			{
				memset(&glv_tagDF8116.Value[2], 0x00, 3);
				glv_tagDF8116.Value[0]=MPP_UIR_MID_ClearDisplay;
			}
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_910_A, 75, 0xFF);
}


UCHAR MPP_S910_76(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmptyList(glv_tagFF8103.Value);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_910_A, 76, rspCode);

	return rspCode;
}


void MPP_S910_77(UCHAR *tlvBuffer)
{
	MPP_GetAndRemoveFromList(mpp_tagTagsToWriteYetAfterGenAC.Value, tlvBuffer, MPP_LISTTYPE_TLV);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 77, 0xFF);
}


void MPP_S910_78(UCHAR *tlvBuffer)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	rspCode=FALSE;
	
	rspCode=UT_Get_TLVLength(tlvBuffer, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == SUCCESS)
	{
		if (lenOfV <= 255)
		{
			if (lenOfT == 1)
			{
				MPP_Clear_DEPBuffer();
				mpp_depRspCode=MPP_APDU_PUT_DATA(0, tlvBuffer[0], lenOfV, &tlvBuffer[lenOfT+lenOfL], &mpp_depRcvLen, mpp_depRcvData); //2017/10/31
			}
			else if (lenOfT == 2)
			{
				MPP_Clear_DEPBuffer();
				mpp_depRspCode=MPP_APDU_PUT_DATA(tlvBuffer[0], tlvBuffer[1], lenOfV, &tlvBuffer[lenOfT+lenOfL], &mpp_depRcvLen, mpp_depRcvData);
			}
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_910_A, 78, 0xFF);
}


UCHAR MPP_S910_78_1(void)
{
	UCHAR	flgIsNotEmpty = FALSE;
	UCHAR	flgIsNotZero = FALSE;
	UCHAR	tmpDF4B[ECL_LENGTH_DF4B] = {0};
	UCHAR	mskDF4B[ECL_LENGTH_DF4B] = {0x00,0x03,0x0F};
	UCHAR	zroDF4B[ECL_LENGTH_DF4B] = {0};
	UCHAR	idxNum = 0;
	UCHAR	rspCmp = 0xFF;
	UCHAR	rspCode = FALSE;

	flgIsNotEmpty = MPP_IsNotEmpty(mpp_tofDF4B);

	for(idxNum = 0; idxNum < 3; idxNum++)
	{
		tmpDF4B[idxNum] = (glv_tagDF4B.Value[idxNum] & mskDF4B[idxNum]);
	}

	rspCmp = UT_bcdcmp(tmpDF4B, zroDF4B, 3);
	if(rspCmp != 0)
	{
		flgIsNotZero=TRUE;
	}

	if((flgIsNotEmpty == TRUE) && (flgIsNotZero == TRUE))
	{
		rspCode = TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_910_A, 78, rspCode);
	
	return rspCode;
}


void MPP_S910_79(void)
{
	MPP_MSG(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 79, 0xFF);
}


void MPP_S910_80(void)
{
	//Same As MPP_S11_120
		
	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 80, 0xFF);
}


void MPP_S910_81(void)
{
	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_910_A, 81, 0xFF);
}


UCHAR MPP_S11_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 1, rspCode);
	
	return rspCode;
}


UCHAR MPP_S11_2(void)
{
	UCHAR	rspCode=FALSE;

	if (mpp_Signal == MPP_SIGNAL_RA)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 2, rspCode);
	
	return rspCode;
}


UCHAR MPP_S11_3(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 3, rspCode);
	
	return rspCode;
}


UCHAR MPP_S11_4(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_DET)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 4, rspCode);
	
	return rspCode;
}


void MPP_S11_5(void)
{
	MPP_Remove_TornRecord(mpp_tagTornEntry.Value[0]);

	MPP_DBG_Put_Process(MPP_STATE_11, 5, 0xFF);
}


UCHAR MPP_S11_6(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=UT_Check_SW12(&mpp_depRcvData[mpp_depRcvLen-2], STATUSWORD_9000);

	MPP_DBG_Put_Process(MPP_STATE_11, 6, rspCode);
	
	return rspCode;
}


void MPP_S11_7(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_StatusBytes;
	memcpy(&glv_tagDF8115.Value[3], &mpp_depRcvData[mpp_depRcvLen-2], 2);

	MPP_DBG_Put_Process(MPP_STATE_11, 7, 0xFF);
}


void MPP_S11_8(void)
{

//	Same As MPP_S9_18

	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOf9F10=0;
	UCHAR	flgIsPresent=FALSE;
	UCHAR	flgIsNotEmpty=FALSE;
	UCHAR	*ptrData=NULLPTR;
	
	mpp_flgParResult=FALSE;
	
	if ((mpp_depRcvLen > 0) && (mpp_depRcvData[0] == 0x77))
	{
		mpp_flgParResult=MPP_ParseAndStoreCardResponse(mpp_depRcvLen, mpp_depRcvData);
	}
	else
	{
		if ((mpp_depRcvLen > 0) && (mpp_depRcvData[0] == 0x80))
		{
			UT_Get_TLVLength(mpp_depRcvData, &lenOfT, &lenOfL, &lenOfV);

			//Check Receive Length = Tag Length + SW12
			if ((mpp_depRcvLen != (lenOfT+lenOfL+lenOfV+2)) || (lenOfV == 0))
			{
				mpp_flgParResult=FALSE;
			}
			else
			{
				if (lenOfV < (ECL_LENGTH_9F27+ECL_LENGTH_9F36+ECL_LENGTH_9F26))
				{
					mpp_flgParResult=FALSE;
				}
				else
				{
					//Point to TLV-V of Tag 80
					ptrData=mpp_depRcvData+lenOfT+lenOfL;
					
					//Cryptogram Information Data
					flgIsPresent=MPP_IsPresent(mpp_tof9F27);
					flgIsNotEmpty=MPP_IsNotEmpty(mpp_tof9F27);

					if ((flgIsPresent == TRUE) || (flgIsNotEmpty == TRUE))
					{
						mpp_flgParResult=FALSE;
					}
					else
					{
						glv_tag9F27.Length[0]=ECL_LENGTH_9F27;
						memcpy(glv_tag9F27.Value, ptrData, ECL_LENGTH_9F27);
						MPP_AddToList(mpp_tof9F27, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
						ptrData+=ECL_LENGTH_9F27;

						//Application Transaction Counter
						flgIsPresent=MPP_IsPresent(mpp_tof9F36);
						flgIsNotEmpty=MPP_IsNotEmpty(mpp_tof9F36);

						if ((flgIsPresent == TRUE) || (flgIsNotEmpty == TRUE))
						{
							mpp_flgParResult=FALSE;
						}
						else
						{
							glv_tag9F36.Length[0]=ECL_LENGTH_9F36;
							memcpy(glv_tag9F36.Value, ptrData, ECL_LENGTH_9F36);
							MPP_AddToList(mpp_tof9F36, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
							ptrData+=ECL_LENGTH_9F36;

							//Application Cryptogram
							flgIsPresent=MPP_IsPresent(mpp_tof9F26);
							flgIsNotEmpty=MPP_IsNotEmpty(mpp_tof9F26);

							if ((flgIsPresent == TRUE) || (flgIsNotEmpty == TRUE))
							{
								mpp_flgParResult=FALSE;
							}
							else
							{
								glv_tag9F26.Length[0]=ECL_LENGTH_9F26;
								memcpy(glv_tag9F26.Value, ptrData, ECL_LENGTH_9F26);
								MPP_AddToList(mpp_tof9F26, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
								ptrData+=ECL_LENGTH_9F26;

								//Issuer Application Data
								flgIsNotEmpty = MPP_IsNotEmpty(mpp_tof9F10);

								if(((lenOfV > (ECL_LENGTH_9F27 + ECL_LENGTH_9F36 + ECL_LENGTH_9F26)) && (flgIsNotEmpty == TRUE)) ||
								   (lenOfV > (ECL_LENGTH_9F27 + ECL_LENGTH_9F36 + ECL_LENGTH_9F26 + ECL_LENGTH_9F10)))
								{
									mpp_flgParResult = FALSE;
								}
								else
								{
									if(lenOfV > (ECL_LENGTH_9F27 + ECL_LENGTH_9F36 + ECL_LENGTH_9F26))
									{
										lenOf9F10 = lenOfV - (ECL_LENGTH_9F27 + ECL_LENGTH_9F36 + ECL_LENGTH_9F26);

										if(lenOf9F10 <= ECL_LENGTH_9F10)
										{
											glv_tag9F10.Length[0] = lenOf9F10;
											memcpy(glv_tag9F10.Value, ptrData, lenOf9F10);
											MPP_AddToList(mpp_tof9F10, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
										}
									}
								}
								
								mpp_flgParResult = TRUE;
							}
						}
					}
				}
			}
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 8, 0xFF);
}


UCHAR MPP_S11_9(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_flgParResult == TRUE)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 9, rspCode);
	
	return rspCode;
}


void MPP_S11_10(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_ParsingError;

	MPP_DBG_Put_Process(MPP_STATE_11, 10, 0xFF);
}


UCHAR MPP_S11_11(void)
{
	UCHAR	tmpTLV[3+3+MPP_LENGTH_TornTempRecord]={0};
	UCHAR	tmpTag[3]={0xFF,0x81,0x7F};	//Dummy Tag
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=FALSE;

	//Patch TLV-T
	lenOfT=3;
	memcpy(tmpTLV, tmpTag, lenOfT);

	//Patch TLV-L
	UT_Get_TLVLengthOfL(mpp_tagTornTempRecord.Length, &lenOfL);
	memcpy(&tmpTLV[3], mpp_tagTornTempRecord.Length, lenOfL);

	//Patch TLV-V
	UT_Get_TLVLengthOfV(mpp_tagTornTempRecord.Length, &lenOfV);
	memcpy(&tmpTLV[3+lenOfL], mpp_tagTornTempRecord.Value, lenOfV);

	//Find IDS Status in Torn Temp Record
	ptrData=UT_Find_Tag(mpp_tofDF8128, lenOfV, &tmpTLV[3+lenOfL]);
	if (ptrData != NULLPTR)
	{
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		//Point to TLV-V
		ptrData+=(lenOfT+lenOfL);

		if (ptrData[0] & MPP_IDS_Write)
		{
			rspCode=TRUE;
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 11, rspCode);
		
	return rspCode;
}


void MPP_S11_12(void)
{
	MPP_Remove_TornRecord(mpp_tagTornEntry.Value[0]);

	MPP_DBG_Put_Process(MPP_STATE_11, 12, 0xFF);
}


void MPP_S11_13(void)
{

//	Same As MPP_S9_11

	UINT	lenOfDRDOL=0;
	UCHAR	lenDolRelData=0;
	UCHAR	bufDolRelData[MPP_DOL_BUFFERSIZE]={0};
	UCHAR	tlvBuffer[3+3+1024]={0};
	UCHAR	lenOfT=0;
	UCHAR	rspCode=FALSE;
	UCHAR	*ptrData=NULLPTR;
	UINT	lenOfData=0;
	UINT	lstLength=69;
	UCHAR	lstTorn[69]={
			0x9F, 0x02,			//Amount, Authorized (Numeric)
			0x9F, 0x03,			//Amount, Other (Numeric)
			0x5A,				//Application PAN
			0x5F, 0x34,			//Application PAN Sequence Number
			0xDF, 0x81, 0x04,	//Balance Read Before Gen AC
			0xDF, 0x81, 0x07,	//CDOL1 Related Data
			0x9F, 0x34,			//CVM Results
			0xDF, 0x81, 0x13,	//DRDOL Related Data
			0x9F, 0x7D,			//DS Summary 1
			0xDF, 0x81, 0x28,	//IDS Status
			0x9F, 0x1E,			//Interface Device Serial Number
			0xDF, 0x81, 0x11,	//PDOL Related Data
			0xDF, 0x81, 0x14,	//Reference Control Parameter
			0x9F, 0x33,			//Terminal Capabilities
			0x9F, 0x1A,			//Terminal Country Code
			0x9F, 0x35,			//Terminal Type
			0x95,				//Terminal Verification Results
			0x9F, 0x53,			//Transaction Category Code
			0x5F, 0x2A,			//Transaction Currency Code
			0x9A,				//Transaction Date
			0x9F, 0x21,			//Transaction Time
			0x9C,				//Transaction Type
			0x9F, 0x37,			//Unpredictable Number
			0xDF, 0x83,	0x01,	//Terminal Relay Resistance Entropy
			0xDF, 0x83,	0x02,	//Device Relay Resistance Entropy
			0xDF, 0x83, 0x03,	//Min Time For Processing Relay Resistance APDU
			0xDF, 0x83,	0x04,	//Max Time For Processing Relay Resistance APDU
			0xDF, 0x83, 0x05,	//Device Estimated Transmission Time For Relay Resistance R-APDU
			0xDF, 0x83,	0x06,	//Measured Relay Resistance Processing Time
			0xDF, 0x83, 0x07	//RRP Counter
	};		

	UT_Get_TLVLengthOfV(glv_tag9F51.Length, &lenOfDRDOL);
	
	MPP_DOL_Get_DOLRelatedData(lenOfDRDOL, glv_tag9F51.Value, &lenDolRelData, bufDolRelData);

	UT_Set_TagLength(lenDolRelData, glv_tagDF8113.Length);
	memcpy(glv_tagDF8113.Value, bufDolRelData, lenDolRelData);
	MPP_AddToList(mpp_tofDF8113, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

	//Initialize(Torn Temp Record)
	memset(mpp_tagTornTempRecord.Length, 0, 3+MPP_LENGTH_TornTempRecord);

	//Set Pointer
	ptrData=lstTorn;
	
	do {
		UT_Get_TLVLengthOfT(ptrData, &lenOfT);

		rspCode=MPP_IsNotEmpty(ptrData);
		if (rspCode == TRUE)
		{
			memset(tlvBuffer, 0, 3+3+1024);
			MPP_GetTLV(ptrData, tlvBuffer);

			MPP_AddToList(tlvBuffer, mpp_tagTornTempRecord.Value, MPP_LISTTYPE_TLV);
		}

		//Point to Next Tag
		ptrData+=lenOfT;
		lenOfData+=lenOfT;
	} while (lenOfData < lstLength);

	MPP_DBG_Put_Process(MPP_STATE_11, 13, 0xFF);
}


void MPP_S11_15(void)
{

//	Same As MPP_S9_13

	UCHAR	idxAID=0;
	UCHAR	idxRecord=0xFF;
	UCHAR	rspCode=FAIL;
	UINT	lenOfV=0;
	
	idxAID=MPP_Get_TornRecordAIDIndex();

	if (mpp_trnRecNum[idxAID] == glv_tagDF811D.Value[0])
	{
		rspCode=MPP_Search_TornRecord_Oldest(&idxRecord);
		if (rspCode == SUCCESS)
		{
			//Copy oldest record of Torn Transaction Log to Torn Record
			if (mpp_trnRec[idxAID][idxRecord].RecLen <= ECL_LENGTH_FF8101)
			{
				UT_Set_TagLength(mpp_trnRec[idxAID][idxRecord].RecLen, glv_tagFF8101.Length);
				memcpy(glv_tagFF8101.Value, mpp_trnRec[idxAID][idxRecord].Record, mpp_trnRec[idxAID][idxRecord].RecLen);

				MPP_AddToList(mpp_tofFF8101, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			}

			//Replace oldest record of Torn Transaction Log with Torn Temp Record
			MPP_Remove_TornRecord(idxRecord);

			UT_Get_TLVLengthOfV(mpp_tagTornTempRecord.Length, &lenOfV);
			MPP_Add_TornRecord(lenOfV, mpp_tagTornTempRecord.Value);
		}
	}
	else
	{
		UT_Get_TLVLengthOfV(mpp_tagTornTempRecord.Length, &lenOfV);
		MPP_Add_TornRecord(lenOfV, mpp_tagTornTempRecord.Value);
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 15, 0xFF);
}


void MPP_S11_16(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_TryAgain;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_DBG_Put_Process(MPP_STATE_11, 16, 0xFF);
}


void MPP_S11_17(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;
	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;

	glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(mpp_depRspCode);
	glv_tagDF8115.Value[5]=MPP_EID_MOE_TryAgain;
	
	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_11, 17, 0xFF);
}


UCHAR MPP_S11_18(void)
{
	UCHAR	flgIsNotEmpty_9F36=FALSE;
	UCHAR	flgIsNotEmpty_9F27=FALSE;
	UCHAR	rspCode=FALSE;

	flgIsNotEmpty_9F36=MPP_IsNotEmpty(mpp_tof9F36);
	flgIsNotEmpty_9F27=MPP_IsNotEmpty(mpp_tof9F27);

	if ((flgIsNotEmpty_9F36 == TRUE) && (flgIsNotEmpty_9F27 == TRUE))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 18, rspCode);
	
	return rspCode;
}


void MPP_S11_19(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_DBG_Put_Process(MPP_STATE_11, 19, 0xFF);
}


UCHAR MPP_S11_20(void)
{

//	Same As MPP_S9_23
//	Same As MPP_S10_17

	UCHAR	rspCode=FALSE;
	
	if ((((glv_tag9F27.Value[0] & 0xC0) == 0x40) && ((glv_tagDF8114.Value[0] & 0xC0) == MPP_ACT_TC))
		||
		(((glv_tag9F27.Value[0] & 0xC0) == 0x80) && (((glv_tagDF8114.Value[0] & 0xC0) == MPP_ACT_TC) || ((glv_tagDF8114.Value[0] & 0xC0) == MPP_ACT_ARQC)))
		||
		((glv_tag9F27.Value[0] & 0xC0) == 0x00))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 20, rspCode);

	return rspCode;
}


void MPP_S11_21(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataError;

	MPP_DBG_Put_Process(MPP_STATE_11, 21, 0xFF);
}


void MPP_S11_22(void)
{
	UCHAR	rspCode=0;
	
	rspCode=MPP_Procedure_PostGenACBalanceReading();
	if (rspCode == MPP_STATE_17)
	{
		while (1)
		{
			rspCode=MPP_State_17_WaitingForPostGenACBalance();
			if (rspCode == MPP_STATE_RETURN)
				break;
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 22, 0xFF);
}


UCHAR MPP_S11_23(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmptyList(mpp_tagTagsToWriteYetAfterGenAC.Value);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_11, 23, rspCode);
	
	return rspCode;
}


void MPP_S11_24(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_ClearDisplay;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_CardReadSuccessfully;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_MSG(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_11, 24, 0xFF);
}


UCHAR MPP_S11_25(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof9F4B);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_11, 25, rspCode);
	
	return rspCode;
}


UCHAR MPP_S11_40(UCHAR *optModLen, UCHAR *optModulus, UCHAR *optExponent)
{
	UCHAR	cauModLen=0;
	UCHAR	issModLen=0;
	UCHAR	cauModulus[255]={0};
	UCHAR	issModulus[255]={0};
	UCHAR	cauExponent[3]={0};
	UCHAR	issExponent[3]={0};	
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_Retrieve_PK_CA(glv_tag9F06.Value, glv_tag8F.Value[0], &cauModLen, cauModulus, cauExponent);
	if (rspCode == SUCCESS)
	{
		rspCode=MPP_Retrieve_PK_Issuer(cauModLen, cauModulus, cauExponent, &issModLen, issModulus, issExponent);
		if (rspCode == SUCCESS)
		{
			rspCode=MPP_Retrieve_PK_ICC(issModLen, issModulus, issExponent, optModLen, optModulus, optExponent);
			if (rspCode == SUCCESS)
			{
				rspCode=TRUE;
			}
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 40, rspCode);

	return rspCode;
}


UCHAR MPP_S11_41(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8128.Value[0] & MPP_IDS_Read)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 41, rspCode);

	return rspCode;
}


UCHAR MPP_S11_41_1(void)
{
	UCHAR	rspCode = FALSE;

	if(glv_tag95.Value[4] & MPP_TVR_RRPPerformed)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 41, rspCode);

	return rspCode;
}


UCHAR MPP_S11_41_2(void)
{
	UCHAR	rspCode = FALSE;

	if(glv_tag95.Value[4] & MPP_TVR_RRPPerformed)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 41, rspCode);

	return rspCode;
}


void MPP_S11_42(UCHAR iptModLen, UCHAR *iptModulus, UCHAR *iptExponent, UCHAR flgIDSRead, UCHAR flgRRP)
{
	mpp_flgVfySDAD=MPP_Verify_DynamicSignature(iptModLen, iptModulus, iptExponent, flgIDSRead, flgRRP);

	MPP_DBG_Put_Process(MPP_STATE_11, 42, 0xFF);
}


void MPP_S11_42_1(UCHAR iptModLen, UCHAR *iptModulus, UCHAR *iptExponent, UCHAR flgIDSRead, UCHAR flgRRP)
{
	mpp_flgVfySDAD=MPP_Verify_DynamicSignature(iptModLen, iptModulus, iptExponent, flgIDSRead, flgRRP);

	MPP_DBG_Put_Process(MPP_STATE_11, 42, 0xFF);
}


void MPP_S11_43(UCHAR iptModLen, UCHAR *iptModulus, UCHAR *iptExponent, UCHAR flgIDSRead, UCHAR flgRRP)
{
	mpp_flgVfySDAD=MPP_Verify_DynamicSignature(iptModLen, iptModulus, iptExponent, flgIDSRead, flgRRP);

	MPP_DBG_Put_Process(MPP_STATE_11, 43, 0xFF);
}


void MPP_S11_43_1(UCHAR iptModLen, UCHAR *iptModulus, UCHAR *iptExponent, UCHAR flgIDSRead, UCHAR flgRRP)
{
	mpp_flgVfySDAD=MPP_Verify_DynamicSignature(iptModLen, iptModulus, iptExponent, flgIDSRead, flgRRP);

	MPP_DBG_Put_Process(MPP_STATE_11, 43, 0xFF);
}


UCHAR MPP_S11_44(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_flgVfySDAD == TRUE)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 44, rspCode);
	
	return rspCode;
}


UCHAR MPP_S11_45(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_flgVfySDAD == TRUE)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 45, rspCode);
	
	return rspCode;
}


void MPP_S11_46(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CamFailed;

	MPP_DBG_Put_Process(MPP_STATE_11, 46, 0xFF);
}


void MPP_S11_46_1(void)
{
	glv_tag95.Value[0] |= MPP_TVR_CDAFailed;

	MPP_DBG_Put_Process(MPP_STATE_11, 46, 0xFF);
}


UCHAR MPP_S11_47(void)
{
	UCHAR	tmpTLV[3+3+MPP_LENGTH_TornTempRecord]={0};
	UCHAR	tmpTag[3]={0xFF,0x81,0x7F};	//Dummy Tag
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=FALSE;
	
	//Patch TLV-T
	lenOfT=3;
	memcpy(tmpTLV, tmpTag, lenOfT);

	//Patch TLV-L
	UT_Get_TLVLengthOfL(mpp_tagTornTempRecord.Length, &lenOfL);
	memcpy(&tmpTLV[3], mpp_tagTornTempRecord.Length, lenOfL);

	//Patch TLV-V
	UT_Get_TLVLengthOfV(mpp_tagTornTempRecord.Length, &lenOfV);
	memcpy(&tmpTLV[3+lenOfL], mpp_tagTornTempRecord.Value, lenOfV);

	//Find IDS Status in Torn Temp Record
	ptrData=UT_Find_Tag(mpp_tofDF8128, lenOfV, &tmpTLV[3+lenOfL]);
	if (ptrData != NULLPTR)
	{
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		//Point to TLV-V
		ptrData+=(lenOfT+lenOfL);

		if (ptrData[0] & MPP_IDS_Write)
		{
			rspCode=TRUE;
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 47, rspCode);
		
	return rspCode;
}


UCHAR MPP_S11_48(void)
{
	UCHAR	tmpTLV[3+3+MPP_LENGTH_TornTempRecord]={0};
	UCHAR	tmpTag[3]={0xFF,0x81,0x7F};	//Dummy Tag
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfV9F7D = 0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=FALSE;

	//Patch TLV-T
	lenOfT=3;
	memcpy(tmpTLV, tmpTag, lenOfT);

	//Patch TLV-L
	UT_Get_TLVLengthOfL(mpp_tagTornTempRecord.Length, &lenOfL);
	memcpy(&tmpTLV[3], mpp_tagTornTempRecord.Length, lenOfL);

	//Patch TLV-V
	UT_Get_TLVLengthOfV(mpp_tagTornTempRecord.Length, &lenOfV);
	memcpy(&tmpTLV[3+lenOfL], mpp_tagTornTempRecord.Value, lenOfV);

	//Find DS Summary 1 in Torn Temp Record
	//ptrData=UT_Find_Tag(mpp_tofDF8128, lenOfV, &tmpTLV[3+lenOfL]);		//william 2017/11/27
	ptrData = UT_Find_Tag(mpp_tof9F7D, lenOfV, &tmpTLV[3 + lenOfL]);
	if (ptrData != NULLPTR)
	{
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		//Point to TLV-V
		ptrData+=(lenOfT+lenOfL);
		UT_Get_TLVLengthOfV(glv_tag9F7D.Length, &lenOfV9F7D);
		if (lenOfV9F7D != lenOfV)
			return FAIL;

		if (!memcmp(ptrData, glv_tag9F7D.Value, lenOfV))		
		{
			rspCode=TRUE;
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 48, rspCode);
		
	return rspCode;
}


void MPP_S11_49(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_IdsReadError;

	MPP_DBG_Put_Process(MPP_STATE_11, 49, 0xFF);
}


UCHAR MPP_S11_50(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsPresent(mpp_tofDF8101);
	
	MPP_DBG_Put_Process(MPP_STATE_11, 50, rspCode);
	
	return rspCode;
}


void MPP_S11_51(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_DBG_Put_Process(MPP_STATE_11, 51, 0xFF);
}


UCHAR MPP_S11_52(void)
{
	UINT	lenOfV_9F7D=0;
	UINT	lenOfV_DF8101=0;
	UCHAR	rspCode=FALSE;
	
	UT_Get_TLVLengthOfV(glv_tag9F7D.Length, &lenOfV_9F7D);
	UT_Get_TLVLengthOfV(glv_tagDF8101.Length, &lenOfV_DF8101);

	if (lenOfV_9F7D == lenOfV_DF8101)
	{
		rspCode = UT_bcdcmp(glv_tag9F7D.Value, glv_tagDF8101.Value, lenOfV_9F7D);
		if (rspCode == 0)
		{
			rspCode = TRUE;
		}
		else
		{
			rspCode = FALSE;
		}
	}
	else
	{
		rspCode = FALSE;
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 52, rspCode);
		
	return rspCode;
}


void MPP_S11_53(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_IdsReadError;

	MPP_DBG_Put_Process(MPP_STATE_11, 53, 0xFF);
}


void MPP_S11_54(void)
{
	glv_tagDF810B.Value[0]|=MPP_DSS_SuccessfulRead;

	MPP_DBG_Put_Process(MPP_STATE_11, 54, 0xFF);
}


UCHAR MPP_S11_55(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8128.Value[0] & MPP_IDS_Write)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 55, rspCode);
	
	return rspCode;
}


UCHAR MPP_S11_56(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsPresent(mpp_tofDF8102);
	
	MPP_DBG_Put_Process(MPP_STATE_11, 56, rspCode);
	
	return rspCode;
}


void MPP_S11_57(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_DBG_Put_Process(MPP_STATE_11, 57, 0xFF);
}


UCHAR MPP_S11_58(void)
{
	UINT	lenOfV_DF8101=0;
	UINT	lenOfV_DF8102=0;
	UCHAR	rspCode=FALSE;
	
	UT_Get_TLVLengthOfV(glv_tagDF8101.Length, &lenOfV_DF8101);
	UT_Get_TLVLengthOfV(glv_tagDF8102.Length, &lenOfV_DF8102);
	
	if (lenOfV_DF8101 == lenOfV_DF8102)
	{
		rspCode = UT_bcdcmp(glv_tagDF8101.Value, glv_tagDF8102.Value, lenOfV_DF8101);
		if (rspCode == 0)
		{
			rspCode = TRUE;
		}
		else
		{
			rspCode = FALSE;
		}
	}
	else
	{
		rspCode = FALSE;
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 58, rspCode);
		
	return rspCode;
}


void MPP_S11_59(void)
{
	glv_tagDF810B.Value[0]|=MPP_DSS_SuccessfulWrite;

	MPP_DBG_Put_Process(MPP_STATE_11, 59, 0xFF);
}


UCHAR MPP_S11_60(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF810A.Value[0] & MPP_DOI_StopIfWriteFailed)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 60, rspCode);

	return rspCode;
}


void MPP_S11_61(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_IdsWriteError;

	MPP_DBG_Put_Process(MPP_STATE_11, 61, 0xFF);
}


UCHAR MPP_S11_70(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof9F26);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_11, 70, rspCode);
	
	return rspCode;
}


void MPP_S11_71(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_DBG_Put_Process(MPP_STATE_11, 71, 0xFF);
}


UCHAR MPP_S11_72(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((glv_tag9F27.Value[0] & 0xC0) == 0x00)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 72, rspCode);

	return rspCode;
}


UCHAR MPP_S11_73(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8128.Value[0] & MPP_IDS_Read)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 73, rspCode);

	return rspCode;
}


UCHAR MPP_S11_74(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8114.Value[0] & MPP_RCP_CDASignatureRequested)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 74, rspCode);

	return rspCode;
}


UCHAR MPP_S11_75(void)
{
	UCHAR	rspCode=FALSE;
	
	if ((glv_tagDF8114.Value[0] & 0xC0) == MPP_RCP_ACTYPE_AAC)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 75, rspCode);

	return rspCode;
}


UCHAR MPP_S11_76(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8114.Value[0] & MPP_RCP_CDASignatureRequested)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 76, rspCode);

	return rspCode;
}


void MPP_S11_77(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataError;

	MPP_DBG_Put_Process(MPP_STATE_11, 77, 0xFF);
}


UCHAR MPP_S11_78(void)
{
	UCHAR	rspCode = FALSE;

	if(glv_tag95.Value[4] & MPP_TVR_RRPPerformed)
		rspCode = TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 78, rspCode);

	return rspCode;
}


void MPP_S11_79(void)
{
	//	Same As MPP_S910_39

	UCHAR	flgIsNotEmpty = FALSE;
	UCHAR	*ptrData = NULLPTR;
	UINT	lenOf57 = 0;
	UCHAR	ascTrack2[ECL_LENGTH_57 * 2] = {0};
	UCHAR	datIndex = 0;
	UCHAR	lenOfPAN = 0;
	UCHAR	flgIsNotEmpty_8F = FALSE;
	UCHAR	*ptrDD = NULLPTR;
	UINT	twoLSB = 0;
	UCHAR	ascBuf[5];
	UCHAR	lenOfAscBuf = 0;
	UCHAR	thirdLSB = 0;
	UINT	millisecond_DF8306 = 0;

	flgIsNotEmpty = MPP_IsNotEmpty(mpp_tof57);
	if(flgIsNotEmpty == TRUE)
	{
		//Point to Track 2
		ptrData = glv_tag57.Value;

		//Get Length of Track 2
		UT_Get_TLVLengthOfV(glv_tag57.Length, &lenOf57);

		//Split to Ascii for Processing
		UT_Split(ascTrack2, glv_tag57.Value, lenOf57);

		//Find Number of Digits in 'Primary Account Number'
		for(datIndex = 0 ; datIndex < (lenOf57 * 2) ; datIndex++)
		{
			if(ascTrack2[datIndex] == 'D')
			{
				lenOfPAN = datIndex;
				break;
			}
		}

		//Move Pointer
		datIndex += 1;	//Point to Start of Expiry Date
		datIndex += 4;	//Point to Start of Service Code
		datIndex += 3;	//Point to Start of Discretionary Data
		
		ptrDD = &ascTrack2[datIndex];	//Point to the most significant digit of the Discretionary Data

		if(lenOfPAN <= 16)	//Tammy 2017/11/10	[SB-195-Errata for EMV Book C-2(Version 2.6)]
		{
			memset(&ascTrack2[datIndex], '0', 13);
			datIndex += 13;
			if((datIndex % 2) != 0)
			{
				memcpy(&ascTrack2[datIndex], (UCHAR *)'F', 1);
				datIndex++;
			}
		}
		else
		{
			memset(&ascTrack2[datIndex], '0', 10);
			datIndex += 10;
			if((datIndex % 2) != 0)
			{
				memcpy(&ascTrack2[datIndex], (UCHAR *)'F', 1);
				datIndex++;
			}
		}

		flgIsNotEmpty_8F = MPP_IsNotEmpty(mpp_tof8F);		//CA Public Key Index (Card)
		if((flgIsNotEmpty_8F == TRUE) && (glv_tag8F.Value[0] < 0x0A))
		{
			/*
			  Replace the most significant digit of the 'Discretionary Data' in Track 2 Equivalent Data with a digit representing CA Public 
			  Key Index (Card).
			*/
			*ptrDD = glv_tag8F.Value[0] | '0';
		}
		ptrDD++;

		//Replace the second most significant digit of the 'Discretionary Data' in Track 2 Equivalent Data with a digit representing RRP Counter.
		*ptrDD = glv_tagDF8307.Value[0] | '0';
		ptrDD++;

		/*
		  Convert the two least significant bytes of the Device Relay Resistance Entropy from 2 byte binary to 5 digit decimal by 
		  considering the two bytes as an integer in the range 0 to 65535. Replace the 5 digits of 'Discretionary Data' in Track 2 
		  Equivalent Data that follow the RRP Counter digit with that value.
		*/
		twoLSB = glv_tagDF8302.Value[2] * 256 + glv_tagDF8302.Value[3];
		memset(ascBuf, '0', 5);
		lenOfAscBuf = UT_itoa(twoLSB, ascBuf);
		
		if(lenOfAscBuf < 5)
		{
			memset(ptrDD, '0', (5 - lenOfAscBuf));
			ptrDD += (5 - lenOfAscBuf);
		}

		memcpy(ptrDD, ascBuf, lenOfAscBuf);
		ptrDD += lenOfAscBuf;

		if(lenOfPAN <= 16)	//Tammy 2017/11/10	[SB-195-Errata for EMV Book C-2(Version 2.6)]
		{
			/*
			  Convert the third least significant byte of Device Relay Resistance Entropy from binary to 3 digit decimal in the range 0 to 255.
			  Replace the next 3 digits of 'Discretionary Data' in Track 2 Equivalent Data with that value.
			*/
			thirdLSB = glv_tagDF8302.Value[1];
			memset(ascBuf, '0', 5);
			lenOfAscBuf = UT_itoa((UINT)thirdLSB, ascBuf);
			
			if(lenOfAscBuf < 3)
			{
				memset(ptrDD, '0', (3 - lenOfAscBuf));
				ptrDD += (3 - lenOfAscBuf);
			}

			memcpy(ptrDD, ascBuf, lenOfAscBuf);
			ptrDD += lenOfAscBuf;
		}

		/*
		  Divide the Measured Relay Resistance Processing Time by 10 using the div operator to give a count in milliseconds.
		  If the value exceeds '03E7' (999), then set the value to '03E7'.
		*/
		millisecond_DF8306  = (glv_tagDF8306.Value[0] * 256 + glv_tagDF8306.Value[1]) / 10;
		if(millisecond_DF8306 > 0x03E7)	//999
		{
			millisecond_DF8306 = 0x03E7;
		}

		/*
		  Convert this value from 2 byte binary to 3 digit decimal by considering the 2 bytes as an integer. 
		  Replace the 3 least significant digits of 'Discretionary Data' in Track 2 Equivalent Data with this 3 digit decimal value. 
		*/
		memset(ascBuf, '0', 5);
		lenOfAscBuf = UT_itoa(millisecond_DF8306, ascBuf);

		if(lenOfAscBuf < 3)
		{
			memset(ptrDD, '0', (3 - lenOfAscBuf));
			ptrDD += (3 - lenOfAscBuf);
		}

		memcpy(ptrDD, ascBuf, lenOfAscBuf);
		ptrDD += lenOfAscBuf;

		UT_Set_TagLength((datIndex / 2), glv_tag57.Length);
		UT_Compress(glv_tag57.Value, ascTrack2, ECL_LENGTH_57);
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 79, 0xFF);
}


void MPP_S11_90(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);

	MPP_DBG_Put_Process(MPP_STATE_11, 90, 0xFF);
}


UCHAR MPP_S11_91(void)
{
	UCHAR	tmpTLV[3+3+MPP_LENGTH_TornTempRecord]={0};
	UCHAR	tmpTag[3]={0xFF,0x81,0x7F};	//Dummy Tag
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	rspCode=FALSE;
	
	//Patch TLV-T
	lenOfT=3;
	memcpy(tmpTLV, tmpTag, lenOfT);

	//Patch TLV-L
	UT_Get_TLVLengthOfL(mpp_tagTornTempRecord.Length, &lenOfL);
	memcpy(&tmpTLV[3], mpp_tagTornTempRecord.Length, lenOfL);

	//Patch TLV-V
	UT_Get_TLVLengthOfV(mpp_tagTornTempRecord.Length, &lenOfV);
	memcpy(&tmpTLV[3+lenOfL], mpp_tagTornTempRecord.Value, lenOfV);

	//Find IDS Status in Torn Temp Record
	ptrData=UT_Find_Tag(mpp_tofDF8128, lenOfV, &tmpTLV[3+lenOfL]);
	if (ptrData != NULLPTR)
	{
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		//Point to TLV-V
		ptrData+=(lenOfT+lenOfL);

		if (ptrData[0] & MPP_IDS_Write)
		{
			rspCode=TRUE;
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 91, rspCode);
		
	return rspCode;
}


void MPP_S11_92(void)
{
	UINT	lenOfV=0;

	UT_Get_TLVLengthOfV(mpp_tagTornTempRecord.Length, &lenOfV);

	if (lenOfV <= 1024)
	{
		UT_Set_TagLength(lenOfV, glv_tagFF8101.Length);
		memcpy(glv_tagFF8101.Value, mpp_tagTornTempRecord.Value, lenOfV);

		MPP_AddToList(mpp_tofFF8101, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}
	
	MPP_DBG_Put_Process(MPP_STATE_11, 92, 0xFF);
}


UCHAR MPP_S11_93(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF8128.Value[0] & MPP_IDS_Write)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_11, 93, rspCode);

	return rspCode;
}


void MPP_S11_94(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	glv_tagDF8129.Value[4]|=MPP_OPS_DataRecordPresent;
	
	MPP_CreateEMVDataRecord();
	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_11, 94, 0xFF);
}


void MPP_S11_95(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	
	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_11, 95, 0xFF);
}


void MPP_S11_101(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);

	MPP_DBG_Put_Process(MPP_STATE_11, 101, 0xFF);
}


void MPP_S11_102(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
	
	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_11, 102, 0xFF);
}


void MPP_S11_110(void)
{
	glv_tagDF8129.Value[4]|=MPP_OPS_DataRecordPresent;

	MPP_CreateEMVDataRecord();

	MPP_DBG_Put_Process(MPP_STATE_11, 110, 0xFF);
}


UCHAR MPP_S11_111(void)
{
	UCHAR	flgIsNotEmpty=FALSE;
	UCHAR	flgIsNotZero=FALSE;
	UCHAR	tmpDF4B[ECL_LENGTH_DF4B]={0};
	UCHAR	mskDF4B[ECL_LENGTH_DF4B]={0x00,0x03,0x0F};
	UCHAR	zroDF4B[ECL_LENGTH_DF4B]={0};
	UCHAR	idxNum=0;
	UCHAR	rspCode=FALSE;

	flgIsNotEmpty=MPP_IsNotEmpty(mpp_tofDF4B);

	for (idxNum=0; idxNum < 3; idxNum++)
	{
		tmpDF4B[idxNum]=(glv_tagDF4B.Value[idxNum] & mskDF4B[idxNum]);
	}

	rspCode=UT_bcdcmp(tmpDF4B, zroDF4B, 3);
	if (rspCode != 0)
	{
		flgIsNotZero=TRUE;
	}

	if ((flgIsNotEmpty == TRUE) && (flgIsNotZero == TRUE))
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_11, 111, rspCode);

	return rspCode;
}


void MPP_S11_112(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;

	MPP_DBG_Put_Process(MPP_STATE_11, 112, 0xFF);
}


void MPP_S11_113(void)
{
	UCHAR	optMessage=0;
	UCHAR	optStatus=0;
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_Check_PhoneMessageTable(&optMessage, &optStatus);
	if (rspCode == TRUE)
	{
		glv_tagDF8116.Value[0]=optMessage;
		glv_tagDF8116.Value[1]=optStatus;
		memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 113, 0xFF);
}


void MPP_S11_114(void)
{
	//Same As MPP_S910_74
	
	UCHAR	tmpUniID[2]={0x00,0x00};
	UCHAR	mskUniID[2]={0x80,0x00};
	UCHAR	zroUniID[2]={0x00,0x00};
	UCHAR	cmpDevType[2]={0x30,0x30};
	UCHAR	flgIsNotEmpty=FALSE;
	UCHAR	flgUniID=FALSE;
	UCHAR	flgDevType=FALSE;
	UCHAR	rspCode=0xFF;
	
	if ((glv_tag9F27.Value[0] & 0xC0) == 0x40)
	{
		glv_tagDF8129.Value[0]=MPP_OPS_STATUS_Approved;
	}
	else
	{
		if ((glv_tag9F27.Value[0] & 0xC0) == 0x80)
		{
			glv_tagDF8129.Value[0]=MPP_OPS_STATUS_OnlineRequest;
		}
		else
		{
			if((glv_tag9C.Value[0] == MPP_TXT_Cash) ||
			   (glv_tag9C.Value[0] == MPP_TXT_CashDisbursement) ||
			   (glv_tag9C.Value[0] == MPP_TXT_Purchase) ||
			   (glv_tag9C.Value[0] == MPP_TXT_CashBack))
			{
				flgIsNotEmpty=MPP_IsNotEmpty(mpp_tof9F6E);
				
				tmpUniID[0]=glv_tag9F6E.Value[2] & mskUniID[0];
				tmpUniID[1]=glv_tag9F6E.Value[3] & mskUniID[1];

				if (!memcmp(tmpUniID, zroUniID, 2))
				{
					flgUniID=TRUE;
				}

				//'Device Type' is present when the most significant bit of byte 1 of 'Unique Identifier' is set to 0b. 
				if ((glv_tag9F6E.Value[2] & 0x80) == 0)
				{
					rspCode=UT_bcdcmp(&glv_tag9F6E.Value[4], cmpDevType, 2);
					if (rspCode != 0)
					{
						flgDevType=TRUE;
					}
				}
				
				if (((flgIsNotEmpty == TRUE) &&
					(flgUniID == TRUE) &&
					(flgDevType == TRUE))
					||
					((glv_tag9F33.Value[0] & MPP_TRC_ICWithContacts) == 0))
				{
					glv_tagDF8129.Value[0]=MPP_OPS_STATUS_Declined;
				}
				else
				{
					glv_tagDF8129.Value[0]=MPP_OPS_STATUS_TryAnotherInterface;
				}
			}
			else
			{
				glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
			}
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 114, 0xFF);
}


void MPP_S11_115(void)
{
	//Same As MPP_S910_75
	
	UCHAR	flgIsNotEmpty_DF8105=FALSE;
	UCHAR	flgIsNotEmpty_9F42=FALSE;
	UCHAR	flgIsNotEmpty_9F6E=FALSE;
	UCHAR	flgUniID=FALSE;
	UCHAR	flgDevType=FALSE;
	UCHAR	tmpUniID[2]={0x00,0x00};
	UCHAR	mskUniID[2]={0x80,0x00};
	UCHAR	zroUniID[2]={0x00,0x00};
	UCHAR	cmpDevType[2]={0x30,0x30};
	UCHAR	rspCode=0xFF;
	
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;

	if ((glv_tag9F27.Value[0] & 0xC0) == 0x40)
	{
		memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, ECL_LENGTH_DF812D);

		flgIsNotEmpty_DF8105=MPP_IsNotEmpty(mpp_tofDF8105);
		if (flgIsNotEmpty_DF8105 == TRUE)
		{
			glv_tagDF8116.Value[13]=MPP_UIR_QUALIFIER_Balance;
			memcpy(&glv_tagDF8116.Value[14], glv_tagDF8105.Value, ECL_LENGTH_DF8105);

			flgIsNotEmpty_9F42=MPP_IsNotEmpty(mpp_tof9F42);
			if (flgIsNotEmpty_9F42 == TRUE)
			{
				memcpy(&glv_tagDF8116.Value[20], glv_tag9F42.Value, ECL_LENGTH_9F42);
			}
		}

		if (glv_tagDF8129.Value[3] == MPP_OPS_CVM_ObtainSignature)
		{
			glv_tagDF8116.Value[0]=MPP_UIR_MID_Approved_Sign;
		}
		else
		{
			glv_tagDF8116.Value[0]=MPP_UIR_MID_Approved;
		}
	}
	else
	{
		if ((glv_tag9F27.Value[0] & 0xC0) == 0x80)
		{
			memset(&glv_tagDF8116.Value[2], 0x00, 3);
			glv_tagDF8116.Value[0]=MPP_UIR_MID_Authorising_PleaseWait;
		}
		else
		{
			if((glv_tag9C.Value[0] == MPP_TXT_Cash) ||
			   (glv_tag9C.Value[0] == MPP_TXT_CashDisbursement) ||
			   (glv_tag9C.Value[0] == MPP_TXT_Purchase) ||
			   (glv_tag9C.Value[0] == MPP_TXT_CashBack))
			{
				memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, ECL_LENGTH_DF812D);

				flgIsNotEmpty_9F6E=MPP_IsNotEmpty(mpp_tof9F6E);

				tmpUniID[0]=glv_tag9F6E.Value[2] & mskUniID[0];
				tmpUniID[1]=glv_tag9F6E.Value[3] & mskUniID[1];

				if (!memcmp(tmpUniID, zroUniID, 2))
				{
					flgUniID=TRUE;
				}

				//'Device Type' is present when the most significant bit of byte 1 of 'Unique Identifier' is set to 0b. 
				if ((glv_tag9F6E.Value[2] & 0x80) == 0)
				{
					rspCode=UT_bcdcmp(&glv_tag9F6E.Value[4], cmpDevType, 2);
					if (rspCode != 0)
					{
						flgDevType=TRUE;
					}
				}
				
				if (((flgIsNotEmpty_9F6E == TRUE) &&
					(flgUniID == TRUE) &&
					(flgDevType == TRUE))
					||
					((glv_tag9F33.Value[0] & MPP_TRC_ICWithContacts) == 0))
				{
					glv_tagDF8116.Value[0]=MPP_UIR_MID_Declined;
				}
				else
				{
					glv_tagDF8116.Value[0]=MPP_UIR_MID_InsertCard;
				}
			}
			else
			{
				memset(&glv_tagDF8116.Value[2], 0x00, 3);
				glv_tagDF8116.Value[0]=MPP_UIR_MID_ClearDisplay;
			}
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 115, 0xFF);
}


UCHAR MPP_S11_116(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmptyList(glv_tagFF8103.Value);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_11, 116, rspCode);

	return rspCode;
}


void MPP_S11_117(UCHAR *tlvBuffer)
{
	MPP_GetAndRemoveFromList(mpp_tagTagsToWriteYetAfterGenAC.Value, tlvBuffer, MPP_LISTTYPE_TLV);

	MPP_DBG_Put_Process(MPP_STATE_11, 117, 0xFF);
}


void MPP_S11_118(UCHAR *tlvBuffer)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	rspCode=FALSE;
	
	rspCode=UT_Get_TLVLength(tlvBuffer, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == SUCCESS)
	{
		if (lenOfV <= 255)
		{
			if (lenOfT == 1)
			{
				MPP_Clear_DEPBuffer();
				mpp_depRspCode = MPP_APDU_PUT_DATA(0, tlvBuffer[0], lenOfV, &tlvBuffer[lenOfT + lenOfL], &mpp_depRcvLen, mpp_depRcvData); //2017/10/31
			}
			else if (lenOfT == 2)
			{
				MPP_Clear_DEPBuffer();
				mpp_depRspCode = MPP_APDU_PUT_DATA(tlvBuffer[0], tlvBuffer[1], lenOfV, &tlvBuffer[lenOfT + lenOfL], &mpp_depRcvLen, mpp_depRcvData);
			}
		}
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 118, 0xFF);
}


UCHAR MPP_S11_118_1(void)
{
	UCHAR	flgIsNotEmpty = FALSE;
	UCHAR	flgIsNotZero = FALSE;
	UCHAR	tmpDF4B[ECL_LENGTH_DF4B] = {0};
	UCHAR	mskDF4B[ECL_LENGTH_DF4B] = {0x00,0x03,0x0F};
	UCHAR	zroDF4B[ECL_LENGTH_DF4B] = {0};
	UCHAR	idxNum = 0;
	UCHAR	rspCmp = 0xFF;
	UCHAR	rspCode = FALSE;

	flgIsNotEmpty = MPP_IsNotEmpty(mpp_tofDF4B);

	for(idxNum = 0; idxNum < 3; idxNum++)
	{
		tmpDF4B[idxNum] = (glv_tagDF4B.Value[idxNum] & mskDF4B[idxNum]);
	}

	rspCmp = UT_bcdcmp(tmpDF4B, zroDF4B, 3);
	if(rspCmp != 0)
	{
		flgIsNotZero=TRUE;
	}

	if((flgIsNotEmpty == TRUE) && (flgIsNotZero == TRUE))
	{
		rspCode = TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_11, 118, rspCode);
	
	return rspCode;
}


void MPP_S11_119(void)
{
	MPP_MSG(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_11, 119, 0xFF);
}


void MPP_S11_120(void)
{
	//Same As MPP_S910_80
		
	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);
	
	MPP_DBG_Put_Process(MPP_STATE_11, 120, 0xFF);
}


void MPP_S11_121(void)
{
	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_11, 121, 0xFF);
}


UCHAR MPP_S12_1(void)
{
	MPP_DBG_Put_Process(MPP_STATE_12, 1, 0xFF);
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		return TRUE;
	
	return FALSE;
}


UCHAR MPP_S12_2(void)
{
	MPP_DBG_Put_Process(MPP_STATE_12, 2, 0xFF);
	if (mpp_Signal == MPP_SIGNAL_RA)
		return TRUE;
	
	return FALSE;
}


UCHAR MPP_S12_3(void)
{
	MPP_DBG_Put_Process(MPP_STATE_12, 3, 0xFF);
	if (mpp_Signal == MPP_SIGNAL_STOP)
		return TRUE;
	
	return FALSE;
}


UCHAR MPP_S12_4(void)
{
	MPP_DBG_Put_Process(MPP_STATE_12, 4, 0xFF);
	if (mpp_Signal == MPP_SIGNAL_DET)
		return TRUE;
	
	return FALSE;
}


void MPP_S12_5(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_TryAgain;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_DBG_Put_Process(MPP_STATE_12, 5, 0xFF);
}


void MPP_S12_6(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;
	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;

	glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(mpp_depRspCode);
	glv_tagDF8115.Value[5]=MPP_EID_MOE_TryAgain;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_12, 6, 0xFF);
}


void MPP_S12_7(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_Stop;

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_12, 7, 0xFF);
}


UCHAR MPP_S12_8(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=UT_Check_SW12(&mpp_depRcvData[mpp_depRcvLen-2], STATUSWORD_9000);
	
	MPP_DBG_Put_Process(MPP_STATE_12, 8, rspCode);			//2017/10/31
	return rspCode;
}


UCHAR MPP_S12_9(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsEmptyList(mpp_tagTagsToWriteYetBeforeGenAC.Value);
		
	MPP_DBG_Put_Process(MPP_STATE_12, 9, rspCode);			//2017/10/31
	return rspCode;
}


void MPP_S12_10(UCHAR *tlvBuffer)
{
	MPP_GetAndRemoveFromList(mpp_tagTagsToWriteYetBeforeGenAC.Value, tlvBuffer, MPP_LISTTYPE_TLV);

	MPP_DBG_Put_Process(MPP_STATE_12, 10, 0xFF);			
}


void MPP_S12_11(UCHAR *tlvBuffer)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	rspCode=FALSE;

	rspCode=UT_Get_TLVLength(tlvBuffer, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == SUCCESS)
	{
		if (lenOfV <= 255)
		{
			if (lenOfT == 1)
			{
				MPP_Clear_DEPBuffer();
				mpp_depRspCode=MPP_APDU_PUT_DATA(0, tlvBuffer[0], lenOfV, &tlvBuffer[lenOfT+lenOfL], &mpp_depRcvLen, mpp_depRcvData);	//2017/10/31
			}
			else if (lenOfT == 2)
			{
				MPP_Clear_DEPBuffer();
				mpp_depRspCode=MPP_APDU_PUT_DATA(tlvBuffer[0], tlvBuffer[1], lenOfV, &tlvBuffer[lenOfT+lenOfL], &mpp_depRcvLen, mpp_depRcvData);
			}
		}
	}
	MPP_DBG_Put_Process(MPP_STATE_12, 11, 0xFF);		
}


void MPP_S12_12(void)
{
	glv_tagDF810F.Value[0]|=MPP_PRS_Completed;

	MPP_DBG_Put_Process(MPP_STATE_12, 12, 0xFF);
}


UCHAR MPP_S12_13(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsPresent(mpp_tof9F51);

	if ((rspCode == TRUE) && (glv_tagDF811D.Value[0] != 0))
		return TRUE;
	
	return FALSE;
}


UCHAR MPP_S12_14(void)
{

//	Same As MPP_S456_44
	
	UCHAR	recIndex=0;
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_Search_TornRecord_PAN(&recIndex);
	if (rspCode == SUCCESS)
	{
		mpp_tagTornEntry.Value[0]=recIndex;
		rspCode=TRUE;
	}
	else
	{
		rspCode=FALSE;
	}

	MPP_DBG_Put_Process(MPP_STATE_12, 14, rspCode);
	return rspCode;
}


void MPP_S12_15(void)
{
	MPP_Procedure_PrepareGenACCommand();

	MPP_DBG_Put_Process(MPP_STATE_12, 15, 0xFF);
}


void MPP_S12_16(void)
{
	UINT	lenOfV=0;

	UT_Get_TLVLengthOfV(mpp_tempSend.Length, &lenOfV);

	MPP_Clear_DEPBuffer();
	//mpp_depRspCode=MPP_APDU_GENERATE_AC(glv_tagDF8114.Value[0], lenOfV, glv_tagDF8107.Value, &mpp_depRcvLen, mpp_depRcvData);
	mpp_depRspCode = MPP_APDU_GENERATE_AC(glv_tagDF8114.Value[0], lenOfV, mpp_tempSend.Value, &mpp_depRcvLen, mpp_depRcvData);

	mpp_vfyDataLen=mpp_depRcvLen;
	memcpy(mpp_vfyData, mpp_depRcvData, mpp_depRcvLen);

	MPP_DBG_Put_Process(MPP_STATE_12, 16, 0xFF);
}


void MPP_S12_17(void)
{

//	Same As MPP_S456_47

	UCHAR	idxAID=0xFF;
	
	idxAID=MPP_Get_TornRecordAIDIndex();
	
	UT_Set_TagLength(mpp_trnRec[idxAID][mpp_tagTornEntry.Value[0]].RecLen, mpp_tagTornTempRecord.Length);
	memcpy(mpp_tagTornTempRecord.Value, mpp_trnRec[idxAID][mpp_tagTornEntry.Value[0]].Record, mpp_trnRec[idxAID][mpp_tagTornEntry.Value[0]].RecLen);
	
	MPP_DBG_Put_Process(MPP_STATE_12, 17, 0xFF);
}


void MPP_S12_18(void)
{

//	Same As S456_48

	UCHAR	tmpTLV[3+3+MPP_LENGTH_TornTempRecord]={0};
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	*ptrData=NULLPTR;

	//Patch FF8101 TLV-T
	memcpy(tmpTLV, mpp_tofFF8101, 3);
	lenOfT=3;

	//Patch FF8101 TLV-L
	UT_Get_TLVLengthOfV(mpp_tagTornTempRecord.Length, &lenOfV);
	lenOfL=UT_Set_TagLength(lenOfV, &tmpTLV[3]);

	//Patch FF8101 TLV-V
	memcpy(&tmpTLV[lenOfT+lenOfL], mpp_tagTornTempRecord.Value, lenOfV);

	//Find DRDOL Related Data
	ptrData=UT_Find_Tag(mpp_tofDF8113, lenOfV, &tmpTLV[lenOfT+lenOfL]);
	if (ptrData != NULLPTR)
	{
		UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

		UT_Set_TagLength(lenOfV, glv_tagDF8113.Length);
		memcpy(glv_tagDF8113.Value, (ptrData+lenOfT+lenOfL), lenOfV);
	}

	MPP_DBG_Put_Process(MPP_STATE_12, 18, 0xFF);
}


void MPP_S12_19(void)
{
	UINT	lenOfV=0;

	UT_Get_TLVLengthOfV(glv_tagDF8113.Length, &lenOfV);
	
	MPP_Clear_DEPBuffer();
	mpp_depRspCode=MPP_APDU_RECOVER_AC(lenOfV, glv_tagDF8113.Value, &mpp_depRcvLen, mpp_depRcvData);

	mpp_vfyDataLen=mpp_depRcvLen;
	memcpy(mpp_vfyData, mpp_depRcvData, mpp_depRcvLen);

	MPP_DBG_Put_Process(MPP_STATE_12, 19, 0xFF);
}


UCHAR MPP_S13_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_13, 1, rspCode);
	
	return rspCode;
}


void MPP_S13_2(void)
{

//EVAL 3G10-9010(Tr1_CCC)
//Add RESET as trigger of start measuring
ECL_LV1_RESET();

	//Wait for (2^Failed MS Cntr * 300) ms
	UT_Wait(UT_pow(2, mpp_tagFailedMSCntr.Value[0]) * 300000);

	MPP_DBG_Put_Process(MPP_STATE_13, 2, 0xFF);
}


void MPP_S13_3(void)
{
	mpp_tagFailedMSCntr.Value[0]=UT_min(mpp_tagFailedMSCntr.Value[0]+1, 5);

	MPP_DBG_Put_Process(MPP_STATE_13, 3, 0xFF);
}


void MPP_S13_4(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_TryAgain;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_DBG_Put_Process(MPP_STATE_13, 4, 0xFF);
}


void MPP_S13_5(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;
	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;
	glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(mpp_depRspCode);
	glv_tagDF8115.Value[5]=MPP_EID_MOE_TryAgain;

	MPP_CreateMSDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_13, 5, 0xFF);
}


UCHAR MPP_S13_6(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_RA)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_13, 6, rspCode);
	
	return rspCode;
}


UCHAR MPP_S13_7(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_13, 7, rspCode);
	
	return rspCode;
}


UCHAR MPP_S13_8(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_DET)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_13, 8, rspCode);

	return rspCode;
}


UCHAR MPP_S13_9(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=UT_Check_SW12(&mpp_depRcvData[mpp_depRcvLen-2], STATUSWORD_9000);

	MPP_DBG_Put_Process(MPP_STATE_13, 9, rspCode);
	
	return rspCode;
}


void MPP_S13_10(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_StatusBytes;
	memcpy(&glv_tagDF8115.Value[3], &mpp_depRcvData[mpp_depRcvLen-2], 2);

	MPP_DBG_Put_Process(MPP_STATE_13, 10, 0xFF);
}


void MPP_S13_11(void)
{
	if ((mpp_depRcvLen > 0) && (mpp_depRcvData[0] == 0x77))
	{
		mpp_flgParResult=MPP_ParseAndStoreCardResponse(mpp_depRcvLen, mpp_depRcvData);
	}
	else
	{
		mpp_flgParResult=FALSE;
	}

	MPP_DBG_Put_Process(MPP_STATE_13, 11, 0xFF);
}


UCHAR MPP_S13_12(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_flgParResult == TRUE)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_13, 12, rspCode);
	
	return rspCode;
}


void MPP_S13_12_1(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_ClearDisplay;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_CardReadSuccessfully;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_MSG(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_13, 12, 0xFF);
}


void MPP_S13_13(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_ParsingError;

	MPP_DBG_Put_Process(MPP_STATE_13, 13, 0xFF);
}


UCHAR MPP_S13_14_1(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof9F36);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_13, 14, rspCode);
	
	return rspCode;
}


UCHAR MPP_S13_14_2(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof9F61);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_13, 14, rspCode);
	
	return rspCode;
}


UCHAR MPP_S13_14_3(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tofDF4B);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_13, 14, rspCode);

	return rspCode;
}


void MPP_S13_14_4(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_DBG_Put_Process(MPP_STATE_13, 14, 0xFF);
}


UCHAR MPP_S13_14_5(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tofDF4B);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_13, 14, rspCode);

	return rspCode;
}


UCHAR MPP_S13_14_6(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF4B.Value[1] & MPP_PCI_OfflinePINVerificationSuccessful)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_13, 14, rspCode);

	return rspCode;
}


void MPP_S13_14_7(UCHAR *nUN)
{
	nUN[0]=mpp_tagnUN.Value[0];

	MPP_DBG_Put_Process(MPP_STATE_13, 14, 0xFF);
}


void MPP_S13_14_8(UCHAR *nUN)
{
	nUN[0]=(mpp_tagnUN.Value[0] + 5) % 10;

	MPP_DBG_Put_Process(MPP_STATE_13, 14, 0xFF);
}


UCHAR MPP_S13_15(void)
{
	UCHAR	flgIsNotEmpty=FALSE;
	UCHAR	flgIsNotPresent=FALSE;
	UCHAR	flgIsEmpty=FALSE;
	UCHAR	rspCode=FALSE;
	
	flgIsNotEmpty=MPP_IsNotEmpty(mpp_tof56);
	flgIsNotPresent=MPP_IsNotPresent(mpp_tof9F60);
	flgIsEmpty=MPP_IsEmpty(mpp_tof9F60);

	if ((flgIsNotEmpty == TRUE)
		&&
		((flgIsNotPresent == TRUE) || (flgIsEmpty == TRUE)))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_13, 15, rspCode);

	return rspCode;
}


void MPP_S13_16(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_DBG_Put_Process(MPP_STATE_13, 16, 0xFF);
}


void MPP_S13_17(void)
{
	mpp_tagFailedMSCntr.Value[0]=0;

	MPP_DBG_Put_Process(MPP_STATE_13, 17, 0xFF);
}


void MPP_S13_18(UCHAR nUN)
{
	UCHAR	q=0;	//Number of CVC3 digits to be copied in the DD
	UCHAR	t=0;	//Number of ATC digits to be copied in the DD
	UINT	nzoBit=0;
	UCHAR	disLen=0;
	UCHAR	disData[ECL_LENGTH_9F6B]={0};
	UCHAR	getlen=0;
	UCHAR	disPosition=0;
	UCHAR	*ptrData=NULLPTR;

	nzoBit=MPP_Get_NumberOfNonZeroBits(ECL_LENGTH_9F65, glv_tag9F65.Value);

	q=nzoBit & 0x00FF;
	t=glv_tag9F67.Value[0];

	ptrData=MPP_Find_DD_Track2(&disLen, &disPosition);
	if (ptrData != NULLPTR)
	{
		//Get Discretionary Data Length in Data
		MPP_Get_DDLength_Track2(disLen, &getlen, disPosition);

		memcpy(disData, ptrData, getlen);

		//Adjust Track 2 Discretionary Data Alient to Left
		if (disPosition == MPP_DD_START_MSB)
		{
			memcpy(disData, ptrData, getlen);
		}
		else if (disPosition == MPP_DD_START_LSB)
		{
			MPP_Move(getlen, disData, MPP_MOVE_LEFT, 4);
		}

		//Generate Discretionary Data
		MPP_Generate_DD(getlen, disData, nUN, MPP_TRACK_2);

		if (disPosition == MPP_DD_START_LSB)
		{
			MPP_Move(getlen, disData, MPP_MOVE_RIGHT, 4);
		}

		//Copy Discretionary Data to Track 2
		MPP_Copy_DD_Track2(ptrData, disLen, disData, disPosition);
	}

	MPP_DBG_Put_Process(MPP_STATE_13, 18, 0xFF);
}


void MPP_S13_19(void)
{
//	Copy nUN' into the least significant digit of the 'Discretionary Data' in Track 2 Data
//	Already Done in S13_18 MPP_Generate_DD

	MPP_DBG_Put_Process(MPP_STATE_13, 19, 0xFF);
}


UCHAR MPP_S13_20(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof56);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_13, 20, rspCode);
	
	return rspCode;
}


void MPP_S13_21(UCHAR nUN)
{
	UCHAR	q=0;	//Number of CVC3 digits to be copied in the DD
	UCHAR	t=0;	//Number of ATC digits to be copied in the DD
	UINT	nzoBit=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	disLen=0;
	
	nzoBit=MPP_Get_NumberOfNonZeroBits(ECL_LENGTH_9F62, glv_tag9F62.Value);

	q=nzoBit & 0x00FF;
	t=glv_tag9F64.Value[0];

	ptrData=MPP_Find_DD_Track1(&disLen);
	if (ptrData != NULLPTR)
	{
		MPP_Generate_DD(disLen, ptrData, nUN, MPP_TRACK_1);
	}

	MPP_DBG_Put_Process(MPP_STATE_13, 21, 0xFF);
}


void MPP_S13_22(void)
{
//	Convert nUN' into the ASCII format
//	Copy the ASCII encoded nUN' character into the least significant position of the 'Discretionary Data' in Track 1 Data
//	Already Done in S13_21 MPP_Generate_DD

	MPP_DBG_Put_Process(MPP_STATE_13, 22, 0xFF);
}


UCHAR MPP_S13_24(void)
{
	UCHAR	rspCode=0xFF;

	rspCode=UT_bcdcmp(glv_tag9F02.Value, glv_tagDF8126.Value, ECL_LENGTH_9F02);
	if (rspCode == 1)	//Amount, Authorized (Numeric) > Reader CVM Required Limit
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_13, 24, rspCode);
	
	return rspCode;
}


void MPP_S13_25(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_OnlineRequest;
	glv_tagDF8129.Value[3]=glv_tagDF812C.Value[0] & 0xF0;

	if ((glv_tagDF812C.Value[0] & 0xF0) == MPP_MCC_CVM_ObtainSignature)
	{
		glv_tagDF8129.Value[4]|=MPP_OPS_Receipt_Yes;
	}

	glv_tagDF8129.Value[4]|=MPP_OPS_DataRecordPresent;

	MPP_CreateMSDataRecord();
	MPP_CreateMSDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_13, 25, 0xFF);
}


void MPP_S13_26(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_OnlineRequest;
	glv_tagDF8129.Value[3]=glv_tagDF811E.Value[0] & 0xF0;
	glv_tagDF8129.Value[4]|=MPP_OPS_Receipt_Yes;
	glv_tagDF8129.Value[4]|=MPP_OPS_DataRecordPresent;

	MPP_CreateMSDataRecord();
	MPP_CreateMSDiscretionaryData();
	
	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_13, 26, 0xFF);
}


void MPP_S13_30(void)
{
	//Wait for (2^Failed MS Cntr * 300) ms
	UT_Wait(UT_pow(2, mpp_tagFailedMSCntr.Value[0]) * 300000);

	MPP_DBG_Put_Process(MPP_STATE_13, 30, 0xFF);
}


void MPP_S13_31(void)
{
	mpp_tagFailedMSCntr.Value[0]=UT_min(mpp_tagFailedMSCntr.Value[0]+1, 5);

	MPP_DBG_Put_Process(MPP_STATE_13, 31, 0xFF);
}


void MPP_S13_32(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;
	
	memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);

	MPP_DBG_Put_Process(MPP_STATE_13, 32, 0xFF);
}


void MPP_S13_33(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=glv_tagDF8116.Value[0];

	MPP_CreateMSDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;
	
	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_13, 33, 0xFF);
}


UCHAR MPP_S13_41(void)
{
	UCHAR	mskPCII[3]={0x00,0x03,0x0F};
	UCHAR	tmpPCII[3]={0};
	UCHAR	zroPCII[3]={0};
	UCHAR	idxNum=0;
	UCHAR	rspCode=0xFF;

	for (idxNum=0; idxNum < 3; idxNum++)
	{
		tmpPCII[idxNum]=(glv_tagDF4B.Value[idxNum] & mskPCII[idxNum]);
	}

	rspCode=UT_bcdcmp(tmpPCII, zroPCII, 3);
	if (rspCode != 0)
	{
		rspCode=TRUE;
	}
	else
	{
		rspCode=FALSE;
	}

	MPP_DBG_Put_Process(MPP_STATE_13, 41, rspCode);
		
	return rspCode;
}


void MPP_S13_42(void)
{
	memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, ECL_LENGTH_DF812D);
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Declined;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;
	
	MPP_MSG(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_13, 42, 0xFF);
}


void MPP_S13_42_1(void)
{
	//Wait for (2^Failed MS Cntr * 300) ms
	UT_Wait(UT_pow(2, mpp_tagFailedMSCntr.Value[0]) * 300000);

	MPP_DBG_Put_Process(MPP_STATE_13, 42, 0xFF);
}


void MPP_S13_42_2(void)
{
	mpp_tagFailedMSCntr.Value[0]=UT_min(mpp_tagFailedMSCntr.Value[0]+1, 5);

	MPP_DBG_Put_Process(MPP_STATE_13, 42, 0xFF);
}


void MPP_S13_43(void)
{
	memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);
	glv_tagDF8116.Value[0] = MPP_UIR_MID_Declined;
	glv_tagDF8116.Value[1] = MPP_UIR_STATUS_NotReady;

	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_Declined;
	glv_tagDF8129.Value[4]|=MPP_OPS_DataRecordPresent;
	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;
	
	MPP_CreateMSDiscretionaryData();
	MPP_CreateMSDataRecord();
	
	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_13, 43, 0xFF);
}


void MPP_S13_44(void)
{
	UCHAR	optMessage=0;
	UCHAR	optStatus=0;
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_Check_PhoneMessageTable(&optMessage, &optStatus);
	if (rspCode == TRUE)
	{
		glv_tagDF8116.Value[0]=optMessage;
		glv_tagDF8116.Value[1]=optStatus;

		MPP_MSG(mpp_tofDF8116);
	}

	MPP_DBG_Put_Process(MPP_STATE_13, 44, 0xFF);
}


void MPP_S13_44_1(void)
{
	//Wait for (2^Failed MS Cntr * 300) ms
	UT_Wait(UT_pow(2, mpp_tagFailedMSCntr.Value[0]) * 300000);

	MPP_DBG_Put_Process(MPP_STATE_13, 44, 0xFF);
}


void MPP_S13_44_2(void)
{
	mpp_tagFailedMSCntr.Value[0]=UT_min(mpp_tagFailedMSCntr.Value[0]+1, 5);

	MPP_DBG_Put_Process(MPP_STATE_13, 44, 0xFF);
}


void MPP_S13_45(void)
{
	memset(&glv_tagDF8116.Value[2], 0x00, 3);
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;
	glv_tagDF8129.Value[4]|=MPP_OPS_DataRecordPresent;
	
	MPP_CreateMSDataRecord();
	MPP_CreateMSDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_13, 45, 0xFF);
}


UCHAR MPP_S14_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_14, 1, rspCode);

	return rspCode;
}


void MPP_S14_2(void)
{

//EVAL 3G10-9010(Tr1_CCC)
//Add RESET as trigger of start measuring
ECL_LV1_RESET();

	//Wait for (2^Failed MS Cntr * 300) ms
	UT_Wait(UT_pow(2, mpp_tagFailedMSCntr.Value[0]) * 300000);

	MPP_DBG_Put_Process(MPP_STATE_14, 2, 0xFF);
}


void MPP_S14_3(void)
{
	mpp_tagFailedMSCntr.Value[0]=UT_min(mpp_tagFailedMSCntr.Value[0]+1, 5);

	MPP_DBG_Put_Process(MPP_STATE_14, 3, 0xFF);
}


void MPP_S14_4(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_TryAgain;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_DBG_Put_Process(MPP_STATE_14, 4, 0xFF);
}


void MPP_S14_5(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;
	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;
	glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(mpp_depRspCode);
	glv_tagDF8115.Value[5]=MPP_EID_MOE_TryAgain;
	
	MPP_CreateMSDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_14, 5, 0xFF);
}


UCHAR MPP_S14_6(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_RA)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_14, 6, rspCode);
	
	return rspCode;
}


UCHAR MPP_S14_7(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_14, 7, rspCode);

	return rspCode;
}


UCHAR MPP_S14_8(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_DET)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_14, 8, rspCode);

	return rspCode;
}


UCHAR MPP_S14_9(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=UT_Check_SW12(&mpp_depRcvData[mpp_depRcvLen-2], STATUSWORD_9000);

	MPP_DBG_Put_Process(MPP_STATE_14, 9, rspCode);
	
	return rspCode;
}


void MPP_S14_10(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_StatusBytes;
	memcpy(&glv_tagDF8115.Value[3], &mpp_depRcvData[mpp_depRcvLen-2], 2);

	MPP_DBG_Put_Process(MPP_STATE_14, 10, 0xFF);
}


void MPP_S14_11(void)
{
	if ((mpp_depRcvLen > 0) && (mpp_depRcvData[0] == 0x77))
	{
		mpp_flgParResult=MPP_ParseAndStoreCardResponse(mpp_depRcvLen, mpp_depRcvData);
	}
	else
	{
		mpp_flgParResult=FALSE;
	}

	MPP_DBG_Put_Process(MPP_STATE_14, 11, 0xFF);
}


UCHAR MPP_S14_12(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_flgParResult == TRUE)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_14, 12, rspCode);
	
	return rspCode;
}


void MPP_S14_12_1(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_ClearDisplay;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_CardReadSuccessfully;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_MSG(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_14, 12, 0xFF);
}


void MPP_S14_13(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_ParsingError;

	MPP_DBG_Put_Process(MPP_STATE_14, 13, 0xFF);
}


UCHAR MPP_S14_14(void)
{
	UCHAR	flgIsNotEmpty_9F36=FALSE;
	UCHAR	flgIsNotEmpty_DF4B=FALSE;
	UCHAR	rspCode=FALSE;

	flgIsNotEmpty_9F36=MPP_IsNotEmpty(mpp_tof9F36);
	flgIsNotEmpty_DF4B=MPP_IsNotEmpty(mpp_tofDF4B);

	if ((flgIsNotEmpty_9F36 == TRUE) && (flgIsNotEmpty_DF4B == TRUE))
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_14, 14, rspCode);
	
	return rspCode;
}


UCHAR MPP_S14_15(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof9F61);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_14, 15, rspCode);

	return rspCode;
}


UCHAR MPP_S14_16(void)
{
	UCHAR	flgIsNotEmpty_56=FALSE;
	UCHAR	flgIsNotPresent_9F60=FALSE;
	UCHAR	flgIsEmpty_9F60=FALSE;
	UCHAR	rspCode=FALSE;
	
	flgIsNotEmpty_56=MPP_IsNotEmpty(mpp_tof56);
	flgIsNotPresent_9F60=MPP_IsNotPresent(mpp_tof9F60);
	flgIsEmpty_9F60=MPP_IsEmpty(mpp_tof9F60);

	if ((flgIsNotEmpty_56 == TRUE) &&
		((flgIsNotPresent_9F60 == TRUE) || (flgIsEmpty_9F60 == TRUE)))
	{
		rspCode=TRUE;
	}

	MPP_DBG_Put_Process(MPP_STATE_14, 16, rspCode);
	
	return rspCode;
}


void MPP_S14_17(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataMissing;

	MPP_DBG_Put_Process(MPP_STATE_14, 17, 0xFF);
}


UCHAR MPP_S14_19_1(void)
{
	UCHAR	tmpDF4B[3]={0x00,0x00,0x00};
	UCHAR	mskDF4B[3]={0x00,0x03,0x0F};
	UCHAR	datDF4B[3]={0x00,0x00,0x00};
	UCHAR	cntIdx=0;
	UCHAR	rspCode=FALSE;

	for (cntIdx=0; cntIdx < 3; cntIdx++)
	{
		tmpDF4B[cntIdx]=glv_tagDF4B.Value[cntIdx] & mskDF4B[cntIdx];
	}

	rspCode=UT_bcdcmp(tmpDF4B, datDF4B, 3);
	if (rspCode != 0)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_14, 19, rspCode);
	
	return rspCode;
}


void MPP_S14_19_2(void)
{
	memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, ECL_LENGTH_DF812D);
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Declined;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;
	
	MPP_MSG(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_14, 19, 0xFF);
}


void MPP_S14_19_2_1(void)
{
	//Wait for (2^Failed MS Cntr * 300) ms
	UT_Wait(UT_pow(2, mpp_tagFailedMSCntr.Value[0]) * 300000);

	MPP_DBG_Put_Process(MPP_STATE_14, 19, 0xFF);
}


void MPP_S14_19_2_2(void)
{
	mpp_tagFailedMSCntr.Value[0]=UT_min(mpp_tagFailedMSCntr.Value[0]+1, 5);

	MPP_DBG_Put_Process(MPP_STATE_14, 19, 0xFF);
}


void MPP_S14_19_3(void)
{
	memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);
	glv_tagDF8116.Value[0] = MPP_UIR_MID_Declined;
	glv_tagDF8116.Value[1] = MPP_UIR_STATUS_NotReady;

	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_Declined;
	glv_tagDF8129.Value[4]|=MPP_OPS_DataRecordPresent;
	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;
	
	MPP_CreateMSDiscretionaryData();
	MPP_CreateMSDataRecord();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_14, 19, 0xFF);
}


UCHAR MPP_S14_20(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF4B.Value[1] & MPP_PCI_OfflinePINVerificationSuccessful)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_14, 20, rspCode);
	
	return rspCode;
}


UCHAR MPP_S14_21(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=UT_bcdcmp(glv_tag9F02.Value, glv_tagDF8126.Value, ECL_LENGTH_9F02);
	if (rspCode == 1)	//Amount, Authorized (Numeric) > Reader CVM Required Limit
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_14, 21, rspCode);
	
	return rspCode;
}


void MPP_S14_21_1(void)
{
	glv_tagDF8115.Value[1]=MPP_EID_LV2_CardDataError;

	MPP_DBG_Put_Process(MPP_STATE_14, 21, 0xFF);
}


void MPP_S14_22(void)
{
	UCHAR	optMessage=0;
	UCHAR	optStatus=0;
	UCHAR	rspCode=FALSE;
	
	rspCode=MPP_Check_PhoneMessageTable(&optMessage, &optStatus);
	if (rspCode == TRUE)
	{
		glv_tagDF8116.Value[0]=optMessage;
		glv_tagDF8116.Value[1]=optStatus;

		MPP_MSG(mpp_tofDF8116);
	}

	MPP_DBG_Put_Process(MPP_STATE_14, 22, 0xFF);
}


void MPP_S14_22_1(void)
{
	//Wait for (2^Failed MS Cntr * 300) ms
	UT_Wait(UT_pow(2, mpp_tagFailedMSCntr.Value[0]) * 300000);

	MPP_DBG_Put_Process(MPP_STATE_14, 22, 0xFF);
}


void MPP_S14_22_2(void)
{
	mpp_tagFailedMSCntr.Value[0]=UT_min(mpp_tagFailedMSCntr.Value[0]+1, 5);

	MPP_DBG_Put_Process(MPP_STATE_14, 22, 0xFF);
}


void MPP_S14_23(void)
{
	memset(&glv_tagDF8116.Value[2], 0x00, 3);
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;
	glv_tagDF8129.Value[4]|=MPP_OPS_DataRecordPresent;

	MPP_CreateMSDataRecord();
	MPP_CreateMSDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_14, 23, 0xFF);
}


void MPP_S14_24(UCHAR *nUN)
{
	nUN[0]=(mpp_tagnUN.Value[0] + 5) % 10;

	MPP_DBG_Put_Process(MPP_STATE_14, 24, 0xFF);
}


void MPP_S14_25(UCHAR *nUN)
{
	nUN[0]=mpp_tagnUN.Value[0];

	MPP_DBG_Put_Process(MPP_STATE_14, 25, 0xFF);
}


void MPP_S14_25_1(void)
{
	mpp_tagFailedMSCntr.Value[0]=0;

	MPP_DBG_Put_Process(MPP_STATE_14, 25, 0xFF);
}


void MPP_S14_26(UCHAR nUN)
{
	UCHAR	q=0;	//Number of CVC3 digits to be copied in the DD
	UCHAR	t=0;	//Number of ATC digits to be copied in the DD
	UINT	nzoBit=0;
	UCHAR	disLen=0;
	UCHAR	disData[ECL_LENGTH_9F6B]={0};
	UCHAR	getlen=0;
	UCHAR	disPosition=0;
	UCHAR	*ptrData=NULLPTR;

	nzoBit=MPP_Get_NumberOfNonZeroBits(ECL_LENGTH_9F65, glv_tag9F65.Value);

	q=nzoBit & 0x00FF;
	t=glv_tag9F67.Value[0];

	ptrData=MPP_Find_DD_Track2(&disLen, &disPosition);
	if (ptrData != NULLPTR)
	{
		//Get Discretionary Data Length in Data
		MPP_Get_DDLength_Track2(disLen, &getlen, disPosition);

		memcpy(disData, ptrData, getlen);

		//Adjust Track 2 Discretionary Data Alient to Left
		if (disPosition == MPP_DD_START_MSB)
		{
			memcpy(disData, ptrData, getlen);
		}
		else if (disPosition == MPP_DD_START_LSB)
		{
			MPP_Move(getlen, disData, MPP_MOVE_LEFT, 4);
		}

		//Generate Discretionary Data
		MPP_Generate_DD(getlen, disData, nUN, MPP_TRACK_2);

		if (disPosition == MPP_DD_START_LSB)
		{
			MPP_Move(getlen, disData, MPP_MOVE_RIGHT, 4);
		}

		//Copy Discretionary Data to Track 2
		MPP_Copy_DD_Track2(ptrData, disLen, disData, disPosition);
	}

	MPP_DBG_Put_Process(MPP_STATE_14, 26, 0xFF);
}


void MPP_S14_27(void)
{
//	Copy nUN' into the least significant digit of the 'Discretionary Data' in Track 2 Data
//	Already Done in MPP_S14_26 MPP_Generate_DD

	MPP_DBG_Put_Process(MPP_STATE_14, 27, 0xFF);
}


UCHAR MPP_S14_28(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsNotEmpty(mpp_tof56);
	if (rspCode == TRUE)
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_14, 28, rspCode);
	
	return rspCode;
}


void MPP_S14_29(UCHAR nUN)
{
	UCHAR	q=0;	//Number of CVC3 digits to be copied in the DD
	UCHAR	t=0;	//Number of ATC digits to be copied in the DD
	UINT	nzoBit=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	disLen=0;
	
	nzoBit=MPP_Get_NumberOfNonZeroBits(ECL_LENGTH_9F62, glv_tag9F62.Value);

	q=nzoBit & 0x00FF;
	t=glv_tag9F64.Value[0];

	ptrData=MPP_Find_DD_Track1(&disLen);
	if (ptrData != NULLPTR)
	{
		MPP_Generate_DD(disLen, ptrData, nUN, MPP_TRACK_1);
	}

	MPP_DBG_Put_Process(MPP_STATE_14, 29, 0xFF);
}


void MPP_S14_30(void)
{
//	Copy the ASCII encoded nUN' character into the least significant position of the 'Discretionary Data' in Track 1 Data
//	Already Done in MPP_S14_29 MPP_Generate_DD

	MPP_DBG_Put_Process(MPP_STATE_14, 30, 0xFF);
}


UCHAR MPP_S14_32(void)
{
	UCHAR	rspCode=FALSE;
	
	if (glv_tagDF4B.Value[1] & MPP_PCI_OfflinePINVerificationSuccessful)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_14, 32, rspCode);

	return rspCode;
}


void MPP_S14_33(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_OnlineRequest;
	glv_tagDF8129.Value[3]=MPP_OPS_CVM_NoCVM;
	glv_tagDF8129.Value[4]|=MPP_OPS_DataRecordPresent;
	
	MPP_CreateMSDataRecord();
	MPP_CreateMSDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_14, 33, 0xFF);
}


void MPP_S14_34(void)
{
	UCHAR	rspCode=FALSE;
	
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_OnlineRequest;
	glv_tagDF8129.Value[3]=MPP_OPS_CVM_ConfirmationCodeVerified;
	
	rspCode=UT_bcdcmp(glv_tag9F02.Value, glv_tagDF8126.Value, ECL_LENGTH_9F02);
	if (rspCode == 1)	//Amount, Authorized (Numeric) > Reader CVM Required Limit
	{
		glv_tagDF8129.Value[4]|=MPP_OPS_Receipt_Yes;
	}

	glv_tagDF8129.Value[4]|=MPP_OPS_DataRecordPresent;
	
	MPP_CreateMSDataRecord();
	MPP_CreateMSDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_14, 34, 0xFF);
}


void MPP_S14_40(void)
{
	//Wait for (2^Failed MS Cntr * 300) ms
	UT_Wait(UT_pow(2, mpp_tagFailedMSCntr.Value[0]) * 300000);
	
	MPP_DBG_Put_Process(MPP_STATE_14, 40, 0xFF);
}


void MPP_S14_41(void)
{
	mpp_tagFailedMSCntr.Value[0]=UT_min(mpp_tagFailedMSCntr.Value[0]+1, 5);

	MPP_DBG_Put_Process(MPP_STATE_14, 41, 0xFF);
}


void MPP_S14_42(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;
	
	memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);

	MPP_DBG_Put_Process(MPP_STATE_14, 42, 0xFF);
}


void MPP_S14_43(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
	glv_tagDF8115.Value[5]=glv_tagDF8116.Value[0];
	
	MPP_CreateMSDiscretionaryData();
	
	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_14, 43, 0xFF);
}


UCHAR MPP_S15_1(void)
{
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		return TRUE;
	
	return FALSE;
}


UCHAR MPP_S15_2(void)
{
	if (mpp_Signal == MPP_SIGNAL_RA)
		return TRUE;
	
	return FALSE;
}


UCHAR MPP_S15_3(void)
{
	if (mpp_Signal == MPP_SIGNAL_STOP)
		return TRUE;
	
	return FALSE;
}


UCHAR MPP_S15_4(void)
{
	if (mpp_Signal == MPP_SIGNAL_DET)
		return TRUE;
	
	return FALSE;
}


UCHAR MPP_S15_5(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=UT_Check_SW12(&mpp_depRcvData[mpp_depRcvLen-2], STATUSWORD_9000);
	
	MPP_DBG_Put_Process(MPP_STATE_15, 5, rspCode);
	return rspCode;
}


UCHAR MPP_S15_6(void)
{
	UCHAR	rspCode=FALSE;

	rspCode=MPP_IsEmptyList(mpp_tagTagsToWriteYetAfterGenAC.Value);
	
	MPP_DBG_Put_Process(MPP_STATE_15, 6, rspCode);
	return rspCode;
}


void MPP_S15_7(UCHAR *tlvBuffer)
{
	MPP_GetAndRemoveFromList(mpp_tagTagsToWriteYetAfterGenAC.Value, tlvBuffer, MPP_LISTTYPE_TLV);

	MPP_DBG_Put_Process(MPP_STATE_15, 7, 0xFF);
}


void MPP_S15_8(UCHAR *tlvBuffer)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	rspCode=FALSE;

	rspCode=UT_Get_TLVLength(tlvBuffer, &lenOfT, &lenOfL, &lenOfV);
	if (rspCode == SUCCESS)
	{
		if (lenOfV <= 255)
		{
			if (lenOfT == 1)
			{
				MPP_Clear_DEPBuffer();
				mpp_depRspCode=MPP_APDU_PUT_DATA(0, tlvBuffer[0], lenOfV, &tlvBuffer[lenOfT+ lenOfL], &mpp_depRcvLen, mpp_depRcvData);	//2017/10/31
			}
			else if (lenOfT == 2)
			{
				MPP_Clear_DEPBuffer();
				mpp_depRspCode=MPP_APDU_PUT_DATA(tlvBuffer[0], tlvBuffer[1], lenOfV, &tlvBuffer[lenOfT+ lenOfL], &mpp_depRcvLen, mpp_depRcvData);
			}
		}
	}
	MPP_DBG_Put_Process(MPP_STATE_15, 8, 0xFF);
}


void MPP_S15_9(void)
{
	glv_tagDF810E.Value[0]|=MPP_POS_Completed;
}


//Add new step
UCHAR MPP_S15_9_1(void)
{
	UCHAR	flgIsNotEmpty=FALSE;
	UCHAR	flgIsNotZero=FALSE;
	UCHAR	tmpDF4B[ECL_LENGTH_DF4B]={0};
	UCHAR	mskDF4B[ECL_LENGTH_DF4B]={0x00,0x03,0x0F};
	UCHAR	zroDF4B[ECL_LENGTH_DF4B]={0};
	UCHAR	idxNum=0;
	UCHAR	rspCode=FALSE;

	flgIsNotEmpty=MPP_IsNotEmpty(mpp_tofDF4B);

	for (idxNum=0; idxNum < 3; idxNum++)
	{
		tmpDF4B[idxNum]=(glv_tagDF4B.Value[idxNum] & mskDF4B[idxNum]);
	}

	rspCode=UT_bcdcmp(tmpDF4B, zroDF4B, 3);
	if (rspCode != 0)
	{
		flgIsNotZero=TRUE;
	}

	if ((flgIsNotEmpty == TRUE) && (flgIsNotZero == TRUE))
		rspCode=TRUE;
	else
		rspCode=FALSE;

	MPP_DBG_Put_Process(MPP_STATE_15, 9, rspCode);

	return rspCode;
}


void MPP_S15_10(void)
{
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_CardReadSuccessfully;

	MPP_MSG(mpp_tofDF8116);
	MPP_DBG_Put_Process(MPP_STATE_15, 10, 0xFF);
}


void MPP_S15_11(void)
{
	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);
}


//Add new step
void MPP_S15_12(void)
{
	UCHAR tmpDF8116[ECL_LENGTH_DF8116] = {0};

	memcpy(tmpDF8116, glv_tagDF8116.Value, ECL_LENGTH_DF8116);

	memset(glv_tagDF8116.Value, 0, ECL_LENGTH_DF8116);

	glv_tagDF8116.Value[0]=MPP_UIR_MID_ClearDisplay;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_CardReadSuccessfully;
	memset(&glv_tagDF8116.Value[2], 0x00, 3);

	MPP_MSG(mpp_tofDF8116);

	memcpy(glv_tagDF8116.Value, tmpDF8116, ECL_LENGTH_DF8116);

	MPP_DBG_Put_Process(MPP_STATE_15, 12, 0xFF);
}


//Add new step
void MPP_S15_13(void)
{
	MPP_CreateEMVDiscretionaryData();

	glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;
	
	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8105);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);
}


UCHAR MPP_S16_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_16, 1, rspCode);
	
	return rspCode;
}


void MPP_S16_2(void)
{
	glv_tagDF8116.Value[0]=MPP_UIR_MID_TryAgain;			//'Message Identifier' in User Interface Request Data := TRY AGAIN
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_ReadyToRead;		//'Status' in User Interface Request Data := READY TO READ
	glv_tagDF8116.Value[2]=0;								//'Hold Time' in User Interface Request Data := '000000'
	glv_tagDF8116.Value[3]=0;
	glv_tagDF8116.Value[4]=0;

	MPP_DBG_Put_Process(MPP_STATE_16, 2, 0xFF);
}


void MPP_S16_3(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;		//'Status' in Outcome Parameter Set := END APPLICATION
	glv_tagDF8129.Value[1]=MPP_OPS_START_B;						//'Start' in Outcome Parameter Set := B
	glv_tagDF8129.Value[4]|=MPP_OPS_UIRequestOnRestartPresent;	//SET 'UI Request on Restart Present' in Outcome Parameter Set

	glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(mpp_depRspCode);//'L1' in Error Indication := Return Code
	glv_tagDF8115.Value[5]=MPP_EID_MOE_TryAgain;				//'Msg On Error' in Error Indication:= TRY AGAIN

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);
	MPP_OUT(mpp_tofDF8116);

	MPP_DBG_Put_Process(MPP_STATE_16, 3, 0xFF);
}


UCHAR MPP_S16_4(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_RA)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_16, 4, rspCode);
	
	return rspCode;
}


UCHAR MPP_S16_5(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_DET)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_16, 5, rspCode);
	
	return rspCode;
}


UCHAR MPP_S16_6(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_16, 6, rspCode);

	return rspCode;
}


void MPP_S16_7(void)
{
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;		//'Status' in Outcome Parameter Set := END APPLICATION
	glv_tagDF8115.Value[2]=MPP_EID_LV3_Stop;					//'L3' in Error Indication := STOP

	MPP_CreateEMVDiscretionaryData();

	MPP_OUT(mpp_tofDF8129);
	MPP_OUT(mpp_tofFF8106);

	MPP_DBG_Put_Process(MPP_STATE_16, 7, 0xFF);
}


UCHAR MPP_S16_8(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=UT_Check_SW12(&mpp_depRcvData[mpp_depRcvLen-2], STATUSWORD_9000);
	 MPP_DataExchangeLog((UCHAR*)&"APDURes- GD", 11);
	 MPP_DataExchangeLog(mpp_depRcvData, mpp_depRcvLen);

	MPP_DBG_Put_Process(MPP_STATE_16, 8, rspCode);
	
	return rspCode;
}


void MPP_S16_9(void)
{
	if (((mpp_depRcvLen-2) == 9) &&										//Length of Response Message Data Field = 9 (Exclude SW12)
		((mpp_depRcvData[0] == 0x9F) && (mpp_depRcvData[1] == 0x50)) &&	//Response Message Data Field[1:2] = '9F50'
		(mpp_depRcvData[2] == 6))										//Response Message Data Field[3] = '06'
	{
		//Balance Read Before Gen AC := Response Message Data Field
		glv_tagDF8104.Length[0]=6;
		memcpy(glv_tagDF8104.Value, &mpp_depRcvData[3], 6);

		MPP_AddToList(mpp_tofDF8104, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	MPP_DBG_Put_Process(MPP_STATE_16, 9, 0xFF);
}


UCHAR MPP_S17_1(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_L1RSP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_17, 1, rspCode);
	
	return rspCode;
}


UCHAR MPP_S17_2(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_RA)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_17, 2, rspCode);
	
	return rspCode;
}


UCHAR MPP_S17_3(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_STOP)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_17, 3, rspCode);
	
	return rspCode;
}


UCHAR MPP_S17_4(void)
{
	UCHAR	rspCode=FALSE;
	
	if (mpp_Signal == MPP_SIGNAL_DET)
		rspCode=TRUE;

	MPP_DBG_Put_Process(MPP_STATE_17, 4, rspCode);

	return rspCode;
}


UCHAR MPP_S17_5(void)
{
	UCHAR	rspCode=FALSE;
	
	rspCode=UT_Check_SW12(&mpp_depRcvData[mpp_depRcvLen-2], STATUSWORD_9000);
	 MPP_DataExchangeLog((UCHAR*)&"APDURes- GD", 11);
	 MPP_DataExchangeLog(mpp_depRcvData, mpp_depRcvLen);

	MPP_DBG_Put_Process(MPP_STATE_17, 5, rspCode);
	
	return rspCode;
}


void MPP_S17_6(void)
{
	if (((mpp_depRcvLen-2) == 9) &&										//Length of Response Message Data Field = 9 (Exclude SW12)
		((mpp_depRcvData[0] == 0x9F) && (mpp_depRcvData[1] == 0x50)) &&	//Response Message Data Field[1:2] = '9F50'
		(mpp_depRcvData[2] == 6))										//Response Message Data Field[3] = '06'
	{
		//Balance Read After Gen AC := Response Message Data Field
		glv_tagDF8105.Length[0]=6;
		memcpy(glv_tagDF8105.Value, &mpp_depRcvData[3], 6);

		MPP_AddToList(mpp_tofDF8105, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
	}

	MPP_DBG_Put_Process(MPP_STATE_17, 6, 0xFF);
}


void MPP_ProcessS_Started(void)
{
	/*
	*	Reset Tag Process which are Remarked Have Been Done At the Start of Entry Point
	*/
	
	//Initialize the Candidate List by emptying it of all entries
	ETP_Clear_CandidateList();

	//Initialize in the internal data store
//	memset(glv_tag4F.Length,		0, 3+ECL_LENGTH_4F		);	//ADF Name
//	memset(glv_tag6F.Length,		0, 3+ECL_LENGTH_6F		);	//File Control Information Template
//	memset(glv_tagDF810C.Length,	0, 3+ECL_LENGTH_DF810C	);	//Kernel ID
//	memset(glv_tag9F2A.Length,		0, 3+ECL_LENGTH_9F2A	);	//Kernel Identifier
//	memset(glv_tagDF812E.Length,	0, 3+ECL_LENGTH_DF812E	);	//Selected Combination
//	memset(glv_tagDF812F.Length,	0, 3+ECL_LENGTH_DF812F	);	//Status Bytes
	
	//Initialize Outcome Parameter Set
//	memset(glv_tagDF8129.Length,	0, 3+ECL_LENGTH_DF8129);	//Outcome Parameter Set
	glv_tagDF8129.Length[0]=ECL_LENGTH_DF8129;
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_NA;
	glv_tagDF8129.Value[1]=MPP_OPS_START_NA;
	glv_tagDF8129.Value[2]=MPP_OPS_OnlineResponseData_NA;
	glv_tagDF8129.Value[3]=MPP_OPS_CVM_NA;
	glv_tagDF8129.Value[4] &= (~MPP_OPS_UIRequestOnOutcomePresent);	//CLEAR UI Request on Outcome Present
	glv_tagDF8129.Value[4] &= (~MPP_OPS_UIRequestOnRestartPresent);	//CLEAR UI Request on Restart Present
	glv_tagDF8129.Value[4] &= (~MPP_OPS_DataRecordPresent);			//CLEAR Data Record Present
	glv_tagDF8129.Value[4] |= MPP_OPS_DiscretionaryDataPresent;
	glv_tagDF8129.Value[4] |= MPP_OPS_Receipt_NA;
	glv_tagDF8129.Value[5]=MPP_OPS_AlternateInterfacePreference_NA;
	glv_tagDF8129.Value[6]=MPP_OPS_FieldOffRequest_NA;
	glv_tagDF8129.Value[7]=0x00;									//Removal Timeout=0

	//Initialize User Interface Request Data
//	memset(glv_tagDF8116.Length, 0, 3+ECL_LENGTH_DF8116);	//User Interface Request Data
	glv_tagDF8116.Length[0]=ECL_LENGTH_DF8116;
	glv_tagDF8116.Value[0]=MPP_UIR_MID_NA;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NA;
	memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);//Hold Time=Message Hold Time
	memset(&glv_tagDF8116.Value[5], 0, 8);					//Language Preference='0000000000000000'
	glv_tagDF8116.Value[13]=MPP_UIR_QUALIFIER_None;
	memset(&glv_tagDF8116.Value[14], 0, 6);					//Value='000000000000'
	memset(&glv_tagDF8116.Value[20], 0, 2);					//Currency Code='0000'

	//Initialize Error Indication
//	memset(glv_tagDF8115.Length, 0, 3+ECL_LENGTH_DF8115);	//Error Indication
	glv_tagDF8115.Length[0]=ECL_LENGTH_DF8115;
	glv_tagDF8115.Value[0]=MPP_EID_LV1_OK;
	glv_tagDF8115.Value[1]=MPP_EID_LV2_OK;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_OK;
	memset(&glv_tagDF8115.Value[3], 0, 2);					//SW12='0000'
	glv_tagDF8115.Value[5]=MPP_EID_MOE_NA;
}


void MPP_Kernel_Started(void)
{
	//KS.1 
	MPP_Set_DefaultConfigurationData();

	//KS.2
	//Mobile Support Indicator
	glv_tag9F7E.Length[0]=ECL_LENGTH_9F7E;
	glv_tag9F7E.Value[0]=MPP_MSI_MobileSupported;

	//Initialize Outcome Parameter Set
	MPP_Initialize_Tag(mpp_tofDF8129);
	glv_tagDF8129.Length[0]=ECL_LENGTH_DF8129;
	glv_tagDF8129.Value[0]=MPP_OPS_STATUS_NA;
	glv_tagDF8129.Value[1]=MPP_OPS_START_NA;
	glv_tagDF8129.Value[3]=MPP_OPS_CVM_NA;
	glv_tagDF8129.Value[4] &= (~MPP_OPS_UIRequestOnOutcomePresent);	//CLEAR UI Request on Outcome Present
	glv_tagDF8129.Value[4] &= (~MPP_OPS_UIRequestOnRestartPresent);	//CLEAR UI Request on Restart Present
	glv_tagDF8129.Value[4] &= (~MPP_OPS_DataRecordPresent);			//CLEAR Data Record Present
	glv_tagDF8129.Value[4] |= MPP_OPS_DiscretionaryDataPresent;		
	glv_tagDF8129.Value[4] |= MPP_OPS_Receipt_NA;					
	glv_tagDF8129.Value[5]=MPP_OPS_AlternateInterfacePreference_NA;
	glv_tagDF8129.Value[6]=MPP_OPS_FieldOffRequest_NA;
	glv_tagDF8129.Value[7]=0x00;									//Removal Timeout=0
	glv_tagDF8129.Value[2]=MPP_OPS_OnlineResponseData_NA;
	
	//Initialize User Interface Request Data
	MPP_Initialize_Tag(mpp_tofDF8116);
	glv_tagDF8116.Length[0]=ECL_LENGTH_DF8116;
	glv_tagDF8116.Value[0]=MPP_UIR_MID_NA;
	glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NA;
	memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);//Hold Time=Message Hold Time
	memset(&glv_tagDF8116.Value[5], 0, 8);					//Language Preference='0000000000000000'
	glv_tagDF8116.Value[13]=MPP_UIR_QUALIFIER_None;
	memset(&glv_tagDF8116.Value[14], 0, 6);					//Value='000000000000'
	memset(&glv_tagDF8116.Value[20], 0, 2);					//Currency Code='0000'

	//Initialize Error Indication
	MPP_Initialize_Tag(mpp_tofDF8115);
	glv_tagDF8115.Length[0]=ECL_LENGTH_DF8115;
	glv_tagDF8115.Value[0]=MPP_EID_LV1_OK;
	glv_tagDF8115.Value[1]=MPP_EID_LV2_OK;
	glv_tagDF8115.Value[2]=MPP_EID_LV3_OK;
	memset(&glv_tagDF8115.Value[3], 0, 2);					//SW12='0000'
	glv_tagDF8115.Value[5]=MPP_EID_MOE_NA;
}


UCHAR MPP_State_1_Idle(void)
{
	UCHAR	flg13=FALSE;
	UCHAR	rspCode=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S1_1();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S1_7();
		if (rspCode == TRUE)
		{
			;	//GOTO Next Step
		}
		else
		{
			MPP_S1_8();

			return MPP_STATE_EXITKERNEL;
		}
	}
	else
	{
		rspCode=MPP_S1_2();
		if (rspCode == TRUE)
		{
			MPP_S1_3();

			return MPP_STATE_EXITKERNEL;
		}
		else
		{
			rspCode=MPP_S1_4();
			if (rspCode == TRUE)
			{
				MPP_S1_5();
				MPP_S1_6();

				return MPP_STATE_EXITKERNEL;
			}

			return MPP_STATE_EXITKERNEL;
		}
	}

	MPP_S1_9();

	MPP_S1_10();
	MPP_S1_11();

	rspCode=MPP_S1_12();
	if (rspCode == TRUE)
	{
		;	//GOTO Next Step
	}
	else
	{
		flg13=TRUE;
	}

	if (flg13 == TRUE)
	{
		MPP_S1_13();
		MPP_S1_14();
	}

	MPP_S1_15();

	MPP_S1_16();

	rspCode=MPP_S1_17();
	if (rspCode == TRUE)
	{
		MPP_S1_18();

		rspCode=MPP_S1_19();
		if (rspCode == TRUE)
		{
			MPP_S1_20();
		}
	}

	rspCode=MPP_S1_21();
	if (rspCode == TRUE)
	{
		MPP_S1_22();
		MPP_S1_23();

		return MPP_STATE_2;
	}
	else
	{
		return MPP_STATE_3;
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_State_2_WaitingForPDOLData(void)
{
	UCHAR	rspCode=FAIL;

	MPP_Get_Signal();
	
	rspCode=MPP_S2_1();
	if (rspCode == TRUE)
	{
		MPP_S2_3();

		return MPP_STATE_EXITKERNEL;
	}
	
	rspCode=MPP_S2_2();
	if (rspCode == TRUE)
	{
		MPP_S2_4();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S2_5();
	if (rspCode == TRUE)
	{
		MPP_S2_6();

		rspCode=MPP_S2_7();
		if (rspCode == TRUE)
		{
			return MPP_STATE_2;
		}
		else
		{
			MPP_S2_8();
			MPP_S2_9();
			MPP_S2_10();

			return MPP_STATE_3;
		}
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_State_3_WaitingForGPOResponse(void)
{
	UCHAR	flwControl=0;	//Flow Control
	UCHAR	flg17=FALSE;
	UCHAR	rspCode=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S3_1();
	if (rspCode == TRUE)
	{
		;	//GOTO S3.8
	}
	else
	{
		rspCode=MPP_S3_2();
		if (rspCode == TRUE)
		{
			MPP_S3_3();

			return MPP_STATE_3;
		}

		rspCode=MPP_S3_4();
		if (rspCode == TRUE)
		{
			MPP_S3_5();

			return MPP_STATE_EXITKERNEL;
		}
		else
		{
			rspCode=MPP_S3_6();
			if (rspCode == TRUE)
			{
				MPP_S3_7();

				return MPP_STATE_EXITKERNEL;
			}

			return MPP_STATE_RETURN;
		}
	}

	rspCode=MPP_S3_8();
	if (rspCode == TRUE)
	{
		MPP_S3_10();

		rspCode=MPP_S3_11();
		if (rspCode == TRUE)
		{
			rspCode=MPP_S3_13();
			if (rspCode == TRUE)
			{
				rspCode=MPP_S3_15();
				if (rspCode == TRUE)
				{
					flg17=TRUE;
				}
				else
				{
					rspCode=MPP_S3_16();
					if (rspCode == TRUE)
					{
						flwControl='A';	//EMV Mode
					}
					else
					{
						flg17=TRUE;
					}
				}

				if (flg17 == TRUE)
				{
					rspCode=MPP_S3_17();
					if (rspCode == TRUE)
					{
						MPP_S3_18();

						flwControl='C';	//Invalid Response
					}
					else
					{
						flwControl='B';	//Mag-stripe Mode
					}
				}
			}
			else
			{
				MPP_S3_14();

				flwControl='C';	//Invalid Response
			}
		}
		else
		{
			MPP_S3_12();

			flwControl='C';	//Invalid Response
		}
	}
	else
	{
		MPP_S3_9_1(&mpp_depRcvData[mpp_depRcvLen-2]);
		MPP_S3_9_2();

		return MPP_STATE_EXITKERNEL;
	}

	//EMV Mode
	if (flwControl == 'A')
	{
		//MasterCard M/Chip Scheme
		SchemeID = 0x21;
		
		rspCode=MPP_S3_30();
		if (rspCode == TRUE)
		{
			MPP_S3_32();
		}
		else
		{
			MPP_S3_31();
		}

		rspCode=MPP_S3_33();
		if (rspCode == TRUE)
		{
			MPP_S3_35();
		}
		else
		{
			MPP_S3_34();
		}

		rspCode = MPP_S3_60();
		if(rspCode == TRUE)
		{
			;	//GOTO S3.61
		}
		else
		{
			MPP_S3_65();

			return MPP_STATE_3R1_D;
		}

		MPP_S3_61();
		MPP_S3_62();
		MPP_S3_63();	//Start Timer
		MPP_S3_64();

		return MPP_STATE_R1;
	}

	//Mag-Stripe Mode
	if (flwControl == 'B')
	{
		//MasterCard Mag-Stripe Scheme
		SchemeID = 0x20;
		
		rspCode=MPP_S3_70();
		if (rspCode == TRUE)
		{
			MPP_S3_72();
		}
		else
		{
			MPP_S3_71();
		}

		rspCode=MPP_S3_73();
		if (rspCode == TRUE)
		{
			MPP_S3_75();
		}
		else
		{
			MPP_S3_74();
		}

		MPP_S3_76();

		rspCode=MPP_S3_77();
		if (rspCode == TRUE)
		{
			MPP_S3_78();
		}

		MPP_S3_80();
		MPP_S3_81();

		return MPP_STATE_7;
	}

	//Invalid Response
	if (flwControl == 'C')
	{
		MPP_S3_90_1();
		MPP_S3_90_2();
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_State_R1_WaitingForExchangeRelayResistanceDataResponse(void)
{
	UCHAR rspCode = FALSE;

	MPP_Get_Signal();

	rspCode = MPP_SR1_1();
	if(rspCode == TRUE)
	{
		MPP_SR1_2();

		return MPP_STATE_R1;
	}

	rspCode = MPP_SR1_9();
	if(rspCode == TRUE)
	{
		;	//GOTO SR1.10
	}
	else
	{
		rspCode = MPP_SR1_3();
		if(rspCode == TRUE)
		{
			MPP_SR1_4();	//Stop Timer
			MPP_SR1_5_1();
			MPP_SR1_5_2();
		}
		else
		{
			rspCode = MPP_SR1_6();
			if(rspCode == TRUE)
			{
				MPP_SR1_7();	//Stop Timer
				MPP_SR1_8();
			}
			else
			{
				return MPP_STATE_RETURN;
			}
		}

		return MPP_STATE_EXITKERNEL;
	}

	MPP_SR1_10();	//Stop timer, compute time taken
	rspCode = MPP_SR1_11();
	if(rspCode == TRUE)
	{
		;	//GOTO SR1.14
	}
	else
	{
		MPP_SR1_12();
		MPP_SR1_13(&mpp_depRcvData[mpp_depRcvLen-2]);

		return MPP_STATE_EXITKERNEL;
	}

	MPP_SR1_14();
	rspCode = MPP_SR1_15();
	if(rspCode == TRUE)
	{
		;	//GOTO SR1.18
	}
	else
	{
		MPP_SR1_16();
		MPP_SR1_17();

		return MPP_STATE_EXITKERNEL;
	}

	MPP_SR1_18();	//Calculate Measured RRP Time
	rspCode = MPP_SR1_19();
	if(rspCode == TRUE)
	{
		MPP_SR1_20();
		MPP_SR1_21();

		return MPP_STATE_EXITKERNEL;
	}
	else
	{
		rspCode = MPP_SR1_22();
		if(rspCode == TRUE)
		{
			MPP_SR1_23();
			MPP_SR1_24();
			MPP_SR1_25();
			MPP_SR1_26();	//Start Timer
			MPP_SR1_27();

			return MPP_STATE_R1;
		}
		else
		{
			rspCode = MPP_SR1_28();
			if(rspCode == TRUE)
			{
				MPP_SR1_29();
			}

			rspCode = MPP_SR1_30();
			if(rspCode == TRUE)
			{
				MPP_SR1_31();
			}

			MPP_SR1_32();

			return MPP_STATE_3R1_D;
		}
	}
}


UCHAR MPP_STATE_3R1_D_CommonProcessing(void)
{
	UCHAR	rspCode = FALSE;

	rspCode = MPP_S3R1_1();
	if(rspCode == TRUE)
	{
		MPP_S3R1_2();
		MPP_S3R1_3();
		MPP_S3R1_4();
	}
	else
	{
		rspCode = MPP_S3R1_5();
		if(rspCode == TRUE)
		{
			MPP_S3R1_6();

			//Invalid Response
			MPP_S3_90_1();
			MPP_S3_90_2();

			return MPP_STATE_EXITKERNEL;
		}
		else
		{
			MPP_S3R1_7();
			MPP_S3R1_8();
			MPP_S3R1_9();
		}
	}
	
	rspCode = MPP_S3R1_10();
	if(rspCode == TRUE)
	{
		MPP_S3R1_11();
		rspCode = MPP_S3R1_12();
		if(rspCode == TRUE)
		{
			;	//GOTO S3R1.14
		}
		else
		{
			MPP_S3R1_13();
		}
	}

	MPP_S3R1_14();

	rspCode = MPP_S3R1_15();
	if(rspCode == TRUE)
	{
		MPP_S3R1_16();
	}

	rspCode = MPP_S3R1_17();
	if(rspCode == TRUE)
	{
		MPP_S3R1_19();
	}
	else
	{
		rspCode = MPP_S3R1_18();
		if(rspCode == TRUE)
		{
			MPP_S3R1_19();
		}
		else
		{
			MPP_S3R1_20();
		}
	}

	rspCode = MPP_S3R1_21();
	if(rspCode == TRUE)
	{
		return MPP_STATE_4;
	}
	else
	{
		return MPP_STATE_5;
	}
}


UCHAR MPP_State_4_WaitingForEMVReadRecordResponse(void)
{
	UCHAR	rspCode=FALSE;
	UCHAR	flg19=FALSE;

	MPP_Get_Signal(); 
	
	rspCode=MPP_S4_1();
	if (rspCode == TRUE)
	{
		MPP_S4_2();

		return MPP_STATE_4;
	}
	//MPP_Get_Signal();		//2017/11/06
	rspCode=MPP_S4_3();
	if (rspCode == TRUE)
	{
		;	//GOTO S4.9
	}
	else
	{
		rspCode=MPP_S4_4();
		if (rspCode == TRUE)
		{
			MPP_S4_5();
			MPP_S4_6();
		}
		else
		{
			rspCode=MPP_S4_7();
			if (rspCode == TRUE)
			{
				MPP_S4_8();
			}
			else
			{
				return MPP_STATE_RETURN;
			}
		}

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S4_9();
	if (rspCode == TRUE)
	{
		;	//GOTO S4.11
	}
	else
	{
		MPP_S4_10_1();
		MPP_S4_10_2();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S4_11();
	if (rspCode == TRUE)
	{
		MPP_S4_12();
	}
	else
	{
		MPP_S4_13();
	}

	MPP_S4_14();

	rspCode=MPP_S4_15();
	if (rspCode == TRUE)
	{
		MPP_S4_16();
		MPP_S4_17();
		MPP_S4_18();
	}
	else
	{
		flg19=TRUE;
	}

	if (flg19 == TRUE)
	{
		rspCode=MPP_S4_19();
		if (rspCode == TRUE)
		{
			MPP_S4_20();
		}
		else
		{
			MPP_S4_21();
			MPP_S4_22();
			MPP_S4_23();
		}
	}

	MPP_S4_24();

	rspCode=MPP_S4_25();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S4_28();
		if (rspCode == TRUE)
		{
			MPP_S4_29();
		}

		rspCode=MPP_S4_30();
		if (rspCode == TRUE)
		{
			rspCode=MPP_S4_31();
			if (rspCode == TRUE)
			{
				rspCode=MPP_S4_32();
				if (rspCode == TRUE)
				{
					;	//GOTO S4.34
				}
				else
				{
					MPP_S4_33();
				}
			}
		}
	}
	else
	{
		rspCode=MPP_S4_26();
		if (rspCode == TRUE)
		{
			MPP_S4_27_1();
			MPP_S4_27_2();

			return MPP_STATE_EXITKERNEL;
		}
		else
		{
			return MPP_STATE_4Apostrophes;
		}
	}

	rspCode=MPP_S4_34();
	if (rspCode == TRUE)
	{
		MPP_S4_35();
	}

	return MPP_STATE_456_A;
}


UCHAR MPP_State_4Apostrophes_TerminateOnNextRA(void)
{
	UCHAR	flg4_1=FALSE;
	UCHAR	rspCode=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S4Apostrophes_1();
	if (rspCode == TRUE)
	{
		flg4_1=TRUE;
	}
	else
	{
		rspCode=MPP_S4Apostrophes_2();
		if (rspCode == TRUE)
		{
			flg4_1=TRUE;
		}
		else
		{
			rspCode=MPP_S4Apostrophes_3();
			if (rspCode == TRUE)
			{
				MPP_S4Apostrophes_5();
			}
		}
	}

	if (flg4_1 == TRUE)
	{
		MPP_S4Apostrophes_4_1();
		MPP_S4Apostrophes_4_2();
	}

	return MPP_STATE_EXITKERNEL;
}


UCHAR MPP_State_5_WaitingForGetDataResponse(void)
{
	UCHAR	curTag[3]={0};	//Current Tag
	UCHAR	rspCode=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S5_1();
	if (rspCode == TRUE)
	{
		MPP_S5_2();

		return MPP_STATE_5;
	}
	//MPP_Get_Signal();	//2017/11/06
	rspCode=MPP_S5_3();
	if (rspCode == TRUE)
	{
		;
	}
	else
	{
		rspCode=MPP_S5_4();
		if (rspCode == TRUE)
		{
			MPP_S5_5();
			MPP_S5_6();
		}
		else
		{
			rspCode=MPP_S5_7();
			if (rspCode == TRUE)
			{
				MPP_S5_8();
			}
			else
			{
				return MPP_STATE_RETURN;
			}
		}

		return MPP_STATE_EXITKERNEL;
	}

	MPP_S5_9(curTag);

	rspCode=MPP_S5_10();
	if (rspCode == TRUE)
	{
		MPP_S5_11();
		MPP_S5_12();
		MPP_S5_13();
	}
	else
	{
		rspCode=MPP_S5_14();
		if (rspCode == TRUE)
		{
			MPP_S5_15();
		}
		else
		{
			MPP_S5_16();
			MPP_S5_17();
			MPP_S5_18();
		}
	}

	rspCode=MPP_S5_19();
	if (rspCode == TRUE)
	{
		MPP_S5_20();

		rspCode=MPP_S5_21();
		if (rspCode == TRUE)
		{
			rspCode=MPP_S5_22(curTag);
			if (rspCode == TRUE)
			{
				MPP_S5_23();

				return MPP_STATE_456_A;
			}
		}
	}

	MPP_S5_24(curTag);
	
	return MPP_STATE_456_A;
}


UCHAR MPP_State_6_WaitingForEMVModeFirstWriteFlag(void)
{
	UCHAR	rspCode=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S6_1();
	if (rspCode == TRUE)
	{
		MPP_S6_3();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S6_2();
	if (rspCode == TRUE)
	{
		MPP_S6_4();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S6_5();
	if (rspCode == TRUE)
	{
		MPP_S6_6();
		MPP_S6_7();

		rspCode=MPP_S6_8();
		if (rspCode == TRUE)
		{
			MPP_S6_9();
			MPP_S6_10();
			MPP_S6_11();
		}
		else
		{
			MPP_S6_12();
		}

		return MPP_STATE_456_A;
	}
	
	return MPP_STATE_RETURN;
}


UCHAR MPP_State_456_CommonProcessing(void)
{
	UCHAR	rspCode=FALSE;
	UCHAR	flg7=FALSE;
	UCHAR	tlvBuffer[3+3+1024]={0};

	rspCode=MPP_S456_1();
	if (rspCode == 'G')
	{
		return MPP_STATE_5;
	}
	else if (rspCode == 'R')
	{
		MPP_S456_2();

		rspCode=MPP_S456_3();
		if (rspCode == TRUE)
		{
			MPP_S456_4();
		}

		return MPP_STATE_4;
	}
	else if (rspCode == 'N')
	{
		;
	}

	rspCode=MPP_S456_5();
	if (rspCode == TRUE)
	{
		MPP_S456_6();

		flg7=TRUE;
	}
	else
	{
		rspCode=MPP_S456_11();
		if (rspCode == TRUE)
		{
			flg7=TRUE;
		}
	}

	if (flg7 == TRUE)
	{
		MPP_S456_7();

		rspCode=MPP_S456_8();
		if (rspCode == TRUE)
		{
			MPP_S456_9();
		}

		MPP_S456_10();

		return MPP_STATE_6;
	}

	rspCode=MPP_S456_12();
	if (rspCode == TRUE)
	{
		;
	}
	else
	{
		MPP_S456_13();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S456_14();
	if (rspCode == TRUE)
	{
		MPP_S456_15();

		return MPP_STATE_EXITKERNEL;
	}
	else
	{
		rspCode=MPP_S456_16();
		if (rspCode == TRUE)
		{
			;
		}
		else
		{
			MPP_S456_17_1();
			MPP_S456_17_2();

			return MPP_STATE_EXITKERNEL;
		}
	}

	rspCode=MPP_S456_18();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S456_19();
		if (rspCode == TRUE)
		{
			;
		}
		else
		{
			MPP_S456_20_1();
			MPP_S456_20_2();

			return MPP_STATE_EXITKERNEL;
		}
	}

	MPP_S456_21();

	rspCode=MPP_S456_22();
	if (rspCode == TRUE)
	{
		;
	}
	else
	{
		MPP_S456_23();
	}

	rspCode=MPP_S456_24();
	if (rspCode == TRUE)
	{
		MPP_S456_25();

		rspCode=MPP_S456_26();
		if (rspCode == TRUE)
		{
			MPP_S456_28();
		}
		else
		{
			MPP_S456_27_1();
			MPP_S456_27_2();

			return MPP_STATE_EXITKERNEL;
		}
	}

	rspCode=MPP_S456_30();
	if (rspCode == TRUE)
	{
		MPP_S456_31();
		MPP_S456_32();
	}
	else
	{
		MPP_S456_33();
	}

	MPP_S456_34();	//Procedure - Pre Gen AC Balance Reading
	MPP_S456_35();	//Procedure - Processing Restrictions
	MPP_S456_36();	//Procedure - CVM Selection

	rspCode=MPP_S456_37();
	if (rspCode == TRUE)
	{
		MPP_S456_38();
	}

	MPP_S456_39();	//Procedure - Terminal Action Analysis

	rspCode=MPP_S456_42();
	if (rspCode == TRUE)
	{
		MPP_S456_50(tlvBuffer);
		MPP_S456_51(tlvBuffer);

		return MPP_STATE_12;
	}

	rspCode=MPP_S456_43();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S456_44();
		if (rspCode == TRUE)
		{
			MPP_S456_47();
			MPP_S456_48();
			MPP_S456_49();

			return MPP_STATE_10;
		}
	}

	rspCode = MPP_S456_45();	//Procedure - PrepareGenACCommand
	if (rspCode == MPP_STATE_EXITKERNEL)
		return MPP_STATE_EXITKERNEL;
	MPP_S456_46();

	return MPP_STATE_9;
}


UCHAR MPP_State_7_WaitingForMagStripeReadRecordResponse(void)
{
	UCHAR	rspCode=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S7_1();
	if (rspCode == TRUE)
	{
		MPP_S7_2();

		return MPP_STATE_7;
	}

	rspCode=MPP_S7_3();
	if (rspCode == TRUE)
	{
		;	//GOTO S7.9
	}
	else
	{
		rspCode=MPP_S7_4();
		if (rspCode == TRUE)
		{
			MPP_S7_5();
			MPP_S7_6();

			return MPP_STATE_EXITKERNEL;
		}
		else
		{
			rspCode=MPP_S7_7();
			if (rspCode == TRUE)
			{
				MPP_S7_8();

				return MPP_STATE_EXITKERNEL;
			}
			else
			{
				return MPP_STATE_RETURN;
			}
		}
	}

	rspCode=MPP_S7_9(&mpp_depRcvData[mpp_depRcvLen-2]);
	 MPP_DataExchangeLog((UCHAR*)&"APDURes- RR", 11);
	 MPP_DataExchangeLog(mpp_depRcvData, mpp_depRcvLen);

	if (rspCode == TRUE)
	{
		MPP_S7_11();
	}
	else
	{
		MPP_S7_10_1();
		MPP_S7_10_2(&mpp_depRcvData[mpp_depRcvLen-2]);

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S7_12();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S7_14();
		if (rspCode == TRUE)
		{
			MPP_S7_15();
		}
	}
	else
	{
		MPP_S7_13_1();
		MPP_S7_13_2();

		return MPP_STATE_EXITKERNEL;
	}

	MPP_S7_16();

	rspCode=MPP_S7_17();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S7_20();
		if (rspCode == TRUE)
		{
			rspCode=MPP_S7_22();
			if (rspCode == TRUE)
			{
				MPP_S7_23();

				return MPP_STATE_78_A;
			}
			else
			{
				MPP_S7_24_1();
				MPP_S7_24_2();

				return MPP_STATE_EXITKERNEL;
			}
		}
		else
		{
			MPP_S7_21_1();
			MPP_S7_21_2();

			return MPP_STATE_EXITKERNEL;
		}
	}
	else
	{
		MPP_S7_18();
		MPP_S7_19();

		return MPP_STATE_7;
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_State_8_WaitingForMagStripeFirstWriteFlag(void)
{
	UCHAR	rspCode=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S8_1();
	if (rspCode == TRUE)
	{
		MPP_S8_2();

		return MPP_STATE_EXITKERNEL;
	}
	else
	{
		rspCode=MPP_S8_3();
		if (rspCode == TRUE)
		{
			MPP_S8_4();

			return MPP_STATE_EXITKERNEL;
		}
		else
		{
			rspCode=MPP_S8_5();
			if (rspCode == TRUE)
			{
				MPP_S8_6();
				MPP_S8_7();

				return MPP_STATE_78_A;
			}
		}
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_State_78_CommonProcessing(void)
{
	UCHAR	dolDatLen=0;
	UCHAR	dolData[MPP_DOL_BUFFERSIZE]={0};
	UCHAR	rspCode=FALSE;
	UCHAR	flg3=FALSE;

	rspCode=MPP_S78_1();
	if (rspCode == TRUE)
	{
		MPP_S78_2();

		flg3=TRUE;
	}
	else
	{
		rspCode=MPP_S78_7();
		if (rspCode == TRUE)
		{
			flg3=TRUE;
		}
	}

	if (flg3 == TRUE)
	{
		MPP_S78_3();

		rspCode=MPP_S78_4();
		if (rspCode == TRUE)
		{
			MPP_S78_5();
		}

		MPP_S78_6();

		return MPP_STATE_8;
	}

	rspCode=MPP_S78_8();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S78_10();
		if (rspCode == TRUE)
		{
			MPP_S78_11();

			return MPP_STATE_EXITKERNEL;
		}
		else
		{
			MPP_S78_12();

			rspCode=MPP_S78_13();
			if (rspCode == TRUE)
			{
				;
			}
			else
			{
				MPP_S78_14();
			}
		}
	}
	else
	{
		MPP_S78_9();

		return MPP_STATE_EXITKERNEL;
	}

	MPP_S78_15();

	rspCode=MPP_S78_16();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S78_19();
		if (rspCode == TRUE)
		{
			MPP_S78_20();
		}

		MPP_S78_21(&dolDatLen, dolData);
		MPP_S78_22(dolDatLen, dolData);

		return MPP_STATE_14;
	}
	else
	{
		MPP_S78_17(&dolDatLen, dolData);
		MPP_S78_18(dolDatLen, dolData);

		return MPP_STATE_13;
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_State_9_WaitingForGenerateACResponse1(void)
{
	UCHAR	rspCode=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S9_1();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S9_5();
		if (rspCode == TRUE)
		{
			MPP_S9_11();
			MPP_S9_13();
			MPP_S9_14();
			MPP_S9_15();

			return MPP_STATE_EXITKERNEL;
		}
		else
		{
			rspCode=MPP_S9_6();
			if (rspCode == TRUE)
			{
				MPP_S9_7();
				MPP_S9_8();

				return MPP_STATE_EXITKERNEL;
			}

			MPP_S9_9();
			MPP_S9_10();

			return MPP_STATE_EXITKERNEL;
		}
	}

	rspCode=MPP_S9_2();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S9_16();
		if (rspCode == TRUE)
		{
			MPP_S9_18();

			rspCode=MPP_S9_19();
			if (rspCode == TRUE)
			{
				rspCode=MPP_S9_21();
				if (rspCode == TRUE)
				{
					rspCode=MPP_S9_23();
					if (rspCode == TRUE)
					{
						MPP_S9_25();

						rspCode=MPP_S9_26();
						if (rspCode == TRUE)
						{
							;
						}
						else
						{
							MPP_S9_27();
						}

						rspCode=MPP_S9_28();
						if (rspCode == TRUE)
						{
							return MPP_STATE_910_A;
						}
						else
						{
							return MPP_STATE_910_B;
						}
					}
					else
					{
						MPP_S9_24();
					}
				}
				else
				{
					MPP_S9_22();
				}
			}
			else
			{
				MPP_S9_20();
			}
		}
		else
		{
			MPP_S9_17();
		}

		return MPP_STATE_910_C;
	}

	rspCode=MPP_S9_3();
	if (rspCode == TRUE)
	{
		return MPP_STATE_9;
	}

	rspCode=MPP_S9_4();
	if (rspCode == TRUE)
	{
		return MPP_STATE_9;
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_State_10_WaitingForRecoverACResponse(void)
{
	UCHAR	rspCode=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S10_1();
	if (rspCode == TRUE)
	{
		MPP_S10_5();
		MPP_S10_6();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S10_2();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S10_7();
		if (rspCode == TRUE)
		{
			MPP_S10_10();
			MPP_S10_11();
			MPP_S10_12();
		}
		else
		{
			MPP_S10_8();
			MPP_S10_9();

			return MPP_STATE_11;
		}

		rspCode=MPP_S10_13();
		if (rspCode == TRUE)
		{
			rspCode=MPP_S10_15();
			if (rspCode == TRUE)
			{
				rspCode=MPP_S10_17();
				if (rspCode == TRUE)
				{
					MPP_S10_19();

					rspCode=MPP_S10_20();
					if (rspCode == TRUE)
					{
						;
					}
					else
					{
						MPP_S10_21();
					}

					rspCode=MPP_S10_22();
					if (rspCode == TRUE)
					{
						return MPP_STATE_910_A;
					}
					else
					{
						return MPP_STATE_910_B;
					}
				}
				else
				{
					MPP_S10_18();
				}
			}
			else
			{
				MPP_S10_16();
			}
		}
		else
		{
			MPP_S10_14();
		}

		return MPP_STATE_910_C;
	}

	rspCode=MPP_S10_3();
	if (rspCode == TRUE)
	{
		return MPP_STATE_10;
	}

	rspCode=MPP_S10_4();
	if (rspCode == TRUE)
	{
		return MPP_STATE_10;
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_State_910_CommonProcessing(UCHAR iptStart)
{
	UCHAR	tlvBuffer[3 + 3 + 1024] = {0};
	UCHAR	optModLen = 0;
	UCHAR	optModulus[255] = {0};
	UCHAR	optExponent[3] = {0};
	UCHAR	flwStart = 0;		//Flow Control
	UCHAR	flg7 = FALSE;
	UCHAR	flg35 = FALSE;
	UCHAR	flg37 = FALSE;
	UCHAR	rspCode = FALSE;
	UCHAR	flgIDSRead = FALSE;
	UCHAR	flgRRP = FALSE;

	switch (iptStart)
	{
		case MPP_STATE_910_A: flwStart='A'; break;
		case MPP_STATE_910_B: flwStart='B'; break;
		case MPP_STATE_910_C: flwStart='C'; break;
		default: return MPP_STATE_RETURN;
	}

	if(flwStart == 'A')
	{
		rspCode = MPP_S910_1(&optModLen, optModulus, optExponent);
		if(rspCode == TRUE)
		{
			rspCode = MPP_S910_2();
			if(rspCode == TRUE)
			{
				flgIDSRead = TRUE;

				rspCode = MPP_S910_2_2();
				if(rspCode == TRUE)
				{
					flgRRP = TRUE;

					MPP_S910_3_1(optModLen, optModulus, optExponent, flgIDSRead, flgRRP);
				}
				else
				{
					MPP_S910_3(optModLen, optModulus, optExponent, flgIDSRead, flgRRP);
				}

				rspCode = MPP_S910_5();
				if(rspCode == TRUE)
				{
					rspCode = MPP_S910_8();
					if(rspCode == TRUE)
					{
						rspCode = MPP_S910_10();
						if(rspCode == TRUE)
						{
							;
						}
						else
						{
							MPP_S910_11();

							flwStart = 'C';
						}
					}
					else
					{
						MPP_S910_9();

						flwStart = 'C';
					}
				}
				else
				{
					flg7 = TRUE;
				}
			}
			else
			{
				rspCode = MPP_S910_2_1();
				if(rspCode == TRUE)
				{
					flgRRP = TRUE;

					MPP_S910_4_1(optModLen, optModulus, optExponent, flgIDSRead, flgRRP);
				}
				else
				{
					MPP_S910_4(optModLen, optModulus, optExponent, flgIDSRead, flgRRP);
				}

				rspCode = MPP_S910_6();
				if(rspCode == TRUE)
				{
					flwStart = 'E';
				}
				else
				{
					flg7 = TRUE;
				}
			}
		}
		else
		{
			flg7 = TRUE;
		}

		if(flg7 == TRUE)
		{
			MPP_S910_7();

			MPP_S910_7_1();

			flwStart = 'C';
		}

		if(flwStart == 'A')
		{
			MPP_S910_12();

			rspCode = MPP_S910_13();
			if(rspCode == TRUE)
			{
				rspCode = MPP_S910_14();
				if(rspCode == TRUE)
				{
					rspCode = MPP_S910_16();
					if(rspCode == TRUE)
					{
						rspCode = MPP_S910_18();
						if(rspCode == TRUE)
						{
							MPP_S910_19();

							flwStart = 'D';
						}
						else
						{
							flwStart = 'E';
						}
					}
					else
					{
						MPP_S910_17();

						flwStart = 'E';
					}
				}
				else
				{
					MPP_S910_15();

					flwStart = 'C';
				}
			}
			else
			{
				flwStart = 'E';
			}
		}
	}

	if(flwStart == 'B')
	{
		rspCode = MPP_S910_30();
		if(rspCode == TRUE)
		{
			rspCode = MPP_S910_32();
			if(rspCode == TRUE)
			{
				rspCode = MPP_S910_33();
				if(rspCode == TRUE)
				{
					flgIDSRead = TRUE;

					flg37 = TRUE;
				}
				else
				{
					flg35 = TRUE;
				}

				if(flg35 == TRUE)
				{
					rspCode = MPP_S910_35();
					if(rspCode == TRUE)
					{
						rspCode = MPP_S910_36();
						if(rspCode == TRUE)
						{
							flg37 = TRUE;
						}
						else
						{
							flwStart = 'E';
						}
					}
					else
					{
						flwStart = 'E';
					}
				}
			}
			else
			{
				rspCode = MPP_S910_34();
				if(rspCode == TRUE)
				{
					flg37 = TRUE;
				}
				else
				{
					rspCode = MPP_S910_38();
					if(rspCode == TRUE)
					{
						flgRRP = TRUE;

						MPP_S910_39();
					}

					flwStart='E';
				}
			}

			if(flg37 == TRUE)
			{
				MPP_S910_37();

				flwStart = 'C';
			}
		}
		else
		{
			MPP_S910_31();

			flwStart = 'C';
		}
	}

	if(flwStart == 'C')
	{
		MPP_S910_50();

		rspCode = MPP_S910_51();
		if(rspCode == TRUE)
		{
			MPP_S910_52();
		}
		else
		{
			MPP_S910_53();
		}

		return MPP_STATE_EXITKERNEL;
	}
	else if(flwStart == 'D')
	{
		MPP_S910_61();
		MPP_S910_62();

		return MPP_STATE_EXITKERNEL;
	}
	else if(flwStart == 'E')
	{
		MPP_S910_70();

		rspCode = MPP_S910_71();
		if(rspCode == TRUE)
		{
			MPP_S910_72();
			MPP_S910_73();
		}
		else
		{
			MPP_S910_74();
			MPP_S910_75();
		}

		rspCode = MPP_S910_76();
		if(rspCode == TRUE)
		{
			MPP_S910_77(tlvBuffer);
			MPP_S910_78(tlvBuffer);

			return MPP_STATE_15;
		}

		rspCode = MPP_S910_78_1();
		if(rspCode == TRUE)
		{
			MPP_S910_79();
			MPP_S910_80();

			return MPP_STATE_EXITKERNEL;
		}
		else
		{
			MPP_S910_81();

			return MPP_STATE_EXITKERNEL;
		}
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_State_11_WaitingForGenerateACResponse2(void)
{
	UCHAR	tlvBuffer[3+3+1024]={0};
	UCHAR	optModLen=0;
	UCHAR	optModulus[255]={0};
	UCHAR	optExponent[3]={0};
	UCHAR	flwControl=0;
	UCHAR	flg46=FALSE;
	UCHAR	flg48=FALSE;
	UCHAR	flg50=FALSE;
	UCHAR	flg75=FALSE;
	UCHAR	flg77=FALSE;
	UCHAR	rspCode=FALSE;
	UCHAR	flgIDSRead = FALSE;
	UCHAR	flgRRP = FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S11_1();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S11_11();
		if (rspCode == TRUE)
		{
			;
		}
		else
		{
			MPP_S11_12();
		}

		MPP_S11_13();
		MPP_S11_15();
		MPP_S11_16();
		MPP_S11_17();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S11_2();
	if (rspCode == TRUE)
	{
		MPP_S11_5();

		rspCode=MPP_S11_6();
		if (rspCode == TRUE)
		{
			MPP_S11_8();
		}
		else
		{
			MPP_S11_7();

			flwControl='C';
		}

		if (flwControl == 0)
		{
			rspCode=MPP_S11_9();
			if (rspCode == TRUE)
			{
				rspCode=MPP_S11_18();
				if (rspCode == TRUE)
				{
					rspCode=MPP_S11_20();
					if (rspCode == TRUE)
					{
						MPP_S11_22();

						rspCode=MPP_S11_23();
						if (rspCode == TRUE)
						{
							;
						}
						else
						{
							MPP_S11_24();
						}

						rspCode=MPP_S11_25();
						if (rspCode == TRUE)
						{
							flwControl='A';
						}
						else
						{
							flwControl='B';
						}
					}
					else
					{
						MPP_S11_21();

						flwControl='C';
					}
				}
				else
				{
					MPP_S11_19();

					flwControl='C';
				}
			}
			else
			{
				MPP_S11_10();

				flwControl='C';
			}
		}
	}
	else
	{
		rspCode=MPP_S11_3();
		if (rspCode == TRUE)
		{
			return MPP_STATE_11;
		}
		else
		{
			rspCode=MPP_S11_4();
			if (rspCode == TRUE)
			{
				return MPP_STATE_11;
			}

			return MPP_STATE_RETURN;
		}
	}

	if (flwControl == 'A')
	{
		rspCode=MPP_S11_40(&optModLen, optModulus, optExponent);
		if (rspCode == TRUE)
		{
			rspCode=MPP_S11_41();
			if (rspCode == TRUE)
			{
				flgIDSRead = TRUE;

				rspCode = MPP_S11_41_2();
				if(rspCode == TRUE)
				{
					flgRRP = TRUE;

					MPP_S11_42_1(optModLen, optModulus, optExponent, flgIDSRead, flgRRP);
				}
				else
				{
					MPP_S11_42(optModLen, optModulus, optExponent, flgIDSRead, flgRRP);
				}

				rspCode=MPP_S11_45();
				if (rspCode == TRUE)
				{
					;
				}
				else
				{
					flg46=TRUE;
				}
			}
			else
			{
				rspCode = MPP_S11_41_1();
				if(rspCode == TRUE)
				{
					flgRRP = TRUE;

					MPP_S11_43_1(optModLen, optModulus, optExponent, flgIDSRead, flgRRP);
				}
				else
				{
					MPP_S11_43(optModLen, optModulus, optExponent, flgIDSRead, flgRRP);
				}
				rspCode=MPP_S11_44();
				if (rspCode == TRUE)
				{
					flwControl='E';
				}
				else
				{
					flg46=TRUE;
				}
			}
		}
		else
		{
			flg46=TRUE;
		}

		if (flg46 == TRUE)
		{
			MPP_S11_46();
			MPP_S11_46_1();

			flwControl='C';
		}

		if (flwControl == 'A')
		{
			rspCode=MPP_S11_47();
			if (rspCode == TRUE)
			{
				flg48=TRUE;
			}
			else
			{
				flg50=TRUE;
			}

			if (flg48 == TRUE)
			{
				rspCode=MPP_S11_48();
				if (rspCode == TRUE)
				{
					flg50=TRUE;
				}
				else
				{
					MPP_S11_49();

					flwControl='C';
				}
			}

			if (flg50 == TRUE)
			{
				rspCode=MPP_S11_50();
				if (rspCode == TRUE)
				{
					rspCode=MPP_S11_52();
					if (rspCode == TRUE)
					{
						;
					}
					else
					{
						MPP_S11_53();

						flwControl='C';
					}
				}
				else
				{
					MPP_S11_51();

					flwControl='C';
				}
			}

			if (flwControl == 'A')
			{
				MPP_S11_54();

				rspCode=MPP_S11_55();
				if (rspCode == TRUE)
				{
					rspCode=MPP_S11_56();
					if (rspCode == TRUE)
					{
						rspCode=MPP_S11_58();
						if (rspCode == TRUE)
						{
							rspCode=MPP_S11_60();
							if (rspCode == TRUE)
							{
								MPP_S11_61();

								flwControl='D';
							}
							else
							{
								flwControl='E';
							}
						}
						else
						{
							MPP_S11_59();

							flwControl='E';
						}
					}
					else
					{
						MPP_S11_57();

						flwControl='C';
					}
				}
				else
				{
					flwControl='E';
				}
			}
		}
	}

	if (flwControl == 'B')
	{
		rspCode=MPP_S11_70();
		if (rspCode == TRUE)
		{
			rspCode=MPP_S11_72();
			if (rspCode == TRUE)
			{
				rspCode=MPP_S11_73();
				if (rspCode == TRUE)
				{
					flgIDSRead = TRUE;

					flg77=TRUE;
				}
				else
				{
					flg75=TRUE;
				}

				if (flg75 == TRUE)
				{
					rspCode=MPP_S11_75();
					if (rspCode == TRUE)
					{
						rspCode=MPP_S11_76();
						if (rspCode == TRUE)
						{
							flg77=TRUE;
						}
						else
						{
							flwControl='E';
						}
					}
					else
					{
						flwControl='E';
					}
				}
			}
			else
			{
				rspCode=MPP_S11_74();
				if (rspCode == TRUE)
				{
					flg77=TRUE;
				}
				else
				{
					rspCode = MPP_S11_78();
					if(rspCode == TRUE)
					{
						flgRRP = TRUE;

						MPP_S11_79();
					}

					flwControl='E';
				}
			}

			if (flg77 == TRUE)
			{
				MPP_S11_77();

				flwControl='C';
			}
		}
		else
		{
			MPP_S11_71();

			flwControl='C';
		}
	}

	if (flwControl == 'C')
	{
		MPP_S11_90();

		rspCode=MPP_S11_91();
		if (rspCode == TRUE)
		{
			MPP_S11_92();
		}

		rspCode=MPP_S11_93();
		if (rspCode == TRUE)
		{
			MPP_S11_94();
		}
		else
		{
			MPP_S11_95();
		}

		return MPP_STATE_EXITKERNEL;
	}

	if (flwControl == 'D')
	{
		MPP_S11_101();
		MPP_S11_102();

		return MPP_STATE_EXITKERNEL;
	}

	if (flwControl == 'E')
	{
		MPP_S11_110();

		rspCode=MPP_S11_111();
		if (rspCode == TRUE)
		{
			MPP_S11_112();
			MPP_S11_113();
		}
		else
		{
			MPP_S11_114();
			MPP_S11_115();
		}

		rspCode=MPP_S11_116();
		if (rspCode == TRUE)
		{
			MPP_S11_117(tlvBuffer);
			MPP_S11_118(tlvBuffer);

			return MPP_STATE_15;
		}

		rspCode = MPP_S11_118_1();
		if(rspCode == TRUE)
		{
			MPP_S11_119();
			MPP_S11_120();
		}
		else
		{
			MPP_S11_121();
		}
		
		return MPP_STATE_EXITKERNEL;
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_State_12_WaitingForPutDataResponseBeforeGenerateAC(void)
{
	UCHAR	tlvBuffer[3+3+1024]={0};
	UCHAR	rspCode=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S12_1();
	if (rspCode == TRUE)
	{
		MPP_S12_5();
		MPP_S12_6();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S12_2();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S12_8();
		if (rspCode == TRUE)
		{
			rspCode=MPP_S12_9();
			if (rspCode == TRUE)
			{
				MPP_S12_12();
			}
			else
			{
				MPP_S12_10(tlvBuffer);
				MPP_S12_11(tlvBuffer);

				return MPP_STATE_12;
			}
		}

		rspCode=MPP_S12_13();
		if (rspCode == TRUE)
		{
			rspCode=MPP_S12_14();
			if (rspCode == TRUE)
			{
				MPP_S12_17();
				MPP_S12_18();
				MPP_S12_19();

				return MPP_STATE_10;
			}
		}

		MPP_S12_15();
		MPP_S12_16();

		return MPP_STATE_9;
	}

	rspCode=MPP_S12_3();
	if (rspCode == TRUE)
	{
		MPP_S12_7();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S12_4();
	if (rspCode == TRUE)
	{
		return MPP_STATE_12;
	}
	
	return MPP_STATE_RETURN;
}


UCHAR MPP_State_13_WaitingForCCCResponse1(void)
{
	UCHAR	nUN=0;			//PayPass nUN'
	UCHAR	rspCode=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S13_1();
	if (rspCode == TRUE)
	{
		MPP_S13_2();
		MPP_S13_3();
		MPP_S13_4();
		MPP_S13_5();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S13_6();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S13_9();
		if (rspCode == TRUE)
		{
			MPP_S13_11();

			rspCode=MPP_S13_12();
			if (rspCode == TRUE)
			{
				MPP_S13_12_1();
				rspCode=MPP_S13_14_1();
				if (rspCode == TRUE)
				{
					rspCode=MPP_S13_14_2();
					if (rspCode == TRUE)
					{
						rspCode=MPP_S13_14_5();
						if (rspCode == TRUE)
						{
							rspCode=MPP_S13_14_6();
							if (rspCode == TRUE)
							{
								MPP_S13_14_8(&nUN);
							}
							else
							{
								MPP_S13_14_7(&nUN);
							}
						}
						else
						{
							MPP_S13_14_7(&nUN);
						}

						rspCode=MPP_S13_15();
						if (rspCode == TRUE)
						{
							MPP_S13_16();

							//GOTO A
						}
						else
						{
							MPP_S13_17();
							MPP_S13_18(nUN);
							MPP_S13_19();

							rspCode=MPP_S13_20();
							if (rspCode == TRUE)
							{
								MPP_S13_21(nUN);
								MPP_S13_22();
							}

							rspCode=MPP_S13_24();
							if (rspCode == TRUE)
							{
								MPP_S13_26();
							}
							else
							{
								MPP_S13_25();
							}

							return MPP_STATE_EXITKERNEL;
						}
					}
					else
					{
						rspCode=MPP_S13_14_3();
						if (rspCode == TRUE)
						{
							rspCode=MPP_S13_41();
							if (rspCode == TRUE)
							{
								MPP_S13_44();
								MPP_S13_44_1();
								MPP_S13_44_2();
								MPP_S13_45();
							}
							else
							{
								MPP_S13_42_1();
								MPP_S13_42_2();
								MPP_S13_43();
							}

							return MPP_STATE_EXITKERNEL;
						}
						else
						{
							MPP_S13_14_4();

							//GOTO A
						}
					}
				}
				else
				{
					MPP_S13_14_4();

					//GOTO A
				}
			}
			else
			{
				MPP_S13_13();

				//GOTO A
			}
		}
		else
		{
			MPP_S13_10();

			//GOTO A
		}

		//A Invalid Response
		MPP_S13_30();
		MPP_S13_31();
		MPP_S13_32();
		MPP_S13_33();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S13_7();
	if (rspCode == TRUE)
	{
		return MPP_STATE_13;
	}

	rspCode=MPP_S13_8();
	if (rspCode == TRUE)
	{
		return MPP_STATE_13;
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_State_14_WaitingForCCCResponse2(void)
{
	UCHAR	nUN=0;			//PayPass nUN'
	UCHAR	rspCode=FALSE;
	UCHAR	flg25_1=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S14_1();
	if (rspCode == TRUE)
	{
		MPP_S14_2();
		MPP_S14_3();
		MPP_S14_4();
		MPP_S14_5();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S14_6();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S14_9();
		if (rspCode == TRUE)
		{
			MPP_S14_11();

			rspCode=MPP_S14_12();
			if (rspCode == TRUE)
			{
				MPP_S14_12_1();
				rspCode=MPP_S14_14();
				if (rspCode == TRUE)
				{
					rspCode=MPP_S14_15();
					if (rspCode == TRUE)
					{
						rspCode=MPP_S14_16();
						if (rspCode == TRUE)
						{
							MPP_S14_17();

							//GOTO A
						}
						else
						{
							rspCode=MPP_S14_20();
							if (rspCode == TRUE)
							{
								MPP_S14_24(&nUN);

								flg25_1=TRUE;
							}
							else
							{
								rspCode=MPP_S14_21();
								if (rspCode == TRUE)
								{
									MPP_S14_21_1();
								}
								else
								{
									MPP_S14_25(&nUN);

									flg25_1=TRUE;
								}
							}

							if (flg25_1 == TRUE)
							{
								MPP_S14_25_1();
								MPP_S14_26(nUN);
								MPP_S14_27();

								rspCode=MPP_S14_28();
								if (rspCode == TRUE)
								{
									MPP_S14_29(nUN);
									MPP_S14_30();
								}

								rspCode=MPP_S14_32();
								if (rspCode == TRUE)
								{
									MPP_S14_34();
								}
								else
								{
									MPP_S14_33();
								}

								return MPP_STATE_EXITKERNEL;
							}
						}
					}
					else
					{
						rspCode=MPP_S14_19_1();
						if (rspCode == TRUE)
						{
							MPP_S14_22();
							MPP_S14_22_1();
							MPP_S14_22_2();
							MPP_S14_23();
						}
						else
						{
							MPP_S14_19_2_1();
							MPP_S14_19_2_2();
							MPP_S14_19_3();
						}

						return MPP_STATE_EXITKERNEL;
					}
				}
				else
				{
					MPP_S14_17();

					//GOTO A
				}
			}
			else
			{
				MPP_S14_13();

				//GOTO A
			}
		}
		else
		{
			MPP_S14_10();

			//GOTO A
		}

		//A Invalid Response
		MPP_S14_40();
		MPP_S14_41();
		MPP_S14_42();
		MPP_S14_43();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S14_7();
	if (rspCode == TRUE)
	{
		return MPP_STATE_14;
	}

	rspCode=MPP_S14_8();
	if (rspCode == TRUE)
	{
		return MPP_STATE_14;
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_State_15_WaitingForPutDataResponseAfterGenerateAC(void)
{
	UCHAR	rspCode=FALSE;
	UCHAR	tlvBuffer[3+3+1024]={0};

	MPP_Get_Signal();
	
	rspCode=MPP_S15_1();
	if (rspCode == TRUE)
	{
		;	//GOTO S15.9.1
	}
	else
	{
		rspCode=MPP_S15_2();
		if (rspCode == TRUE)
		{
			rspCode=MPP_S15_5();
			if (rspCode == TRUE)
			{
				rspCode=MPP_S15_6();
				if (rspCode == TRUE)
				{
					MPP_S15_9();
				}
				else
				{
					MPP_S15_7(tlvBuffer);
					MPP_S15_8(tlvBuffer);

					return MPP_STATE_15;
				}
			}
		}
		else
		{
			rspCode=MPP_S15_3();
			if (rspCode == TRUE)
			{
				return MPP_STATE_15;
			}
			else
			{
				rspCode=MPP_S15_4();
				if (rspCode == TRUE)
				{
					return MPP_STATE_15;
				}
				else
				{
					return MPP_STATE_RETURN;
				}
			}
		}
	}

	rspCode = MPP_S15_9_1();
	if(rspCode == TRUE)
	{
		MPP_S15_10();
		MPP_S15_11();
	}
	else
	{
		MPP_S15_12();
		MPP_S15_13();
	}
	
	return MPP_STATE_RETURN;
}


UCHAR MPP_State_16_WaitingForPreGenACBalance(void)
{
	UCHAR	rspCode=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S16_1();
	if (rspCode == TRUE)
	{
		MPP_S16_2();
		MPP_S16_3();

		return MPP_STATE_EXITKERNEL;
	}

	rspCode=MPP_S16_4();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S16_8();
		if (rspCode == TRUE)
		{
			MPP_S16_9();
		}

		return MPP_STATE_RETURN;
	}

	rspCode=MPP_S16_5();
	if (rspCode == TRUE)
	{
		return MPP_STATE_16;
	}

	rspCode=MPP_S16_6();
	if (rspCode == TRUE)
	{
		MPP_S16_7();

		return MPP_STATE_EXITKERNEL;
	}

	return MPP_STATE_RETURN;
}


UCHAR MPP_State_17_WaitingForPostGenACBalance(void)
{
	UCHAR	rspCode=FALSE;

	MPP_Get_Signal();
	
	rspCode=MPP_S17_1();
	if (rspCode == TRUE)
	{
		return MPP_STATE_RETURN;
	}

	rspCode=MPP_S17_2();
	if (rspCode == TRUE)
	{
		rspCode=MPP_S17_5();
		if (rspCode == TRUE)
		{
			MPP_S17_6();	
		}
		
		return MPP_STATE_RETURN;
	}

	rspCode=MPP_S17_3();
	if (rspCode == TRUE)
	{
		return MPP_STATE_17;
	}

	rspCode=MPP_S17_4();
	if (rspCode == TRUE)
	{
		return MPP_STATE_17;
	}

	return MPP_STATE_RETURN;
}


void MPP_State_51_Idle(UCHAR symNum)
{
	/*
	*	Reset Tag Process which are Remarked Have Been Done At the Start of Entry Point
	*/
	
	switch (symNum)
	{
		case 1:	break;	//S51.1	Receive STOP signal
		case 2:	break;	//S51.2	Receive ACT signal with Start Code

		case 3:	//S51.3	Initialize in the internal data store
//			memset(glv_tag4F.Length,		0, 3+ECL_LENGTH_4F		);	//ADF Name
//			memset(glv_tag6F.Length,		0, 3+ECL_LENGTH_6F		);	//File Control Information Template
//			memset(glv_tagDF810C.Length,	0, 3+ECL_LENGTH_DF810C	);	//Kernel ID
//			memset(glv_tag9F2A.Length,		0, 3+ECL_LENGTH_9F2A	);	//Kernel Identifier
//			memset(glv_tagDF812E.Length,	0, 3+ECL_LENGTH_DF812E	);	//Selected Combination
//			memset(glv_tagDF812F.Length,	0, 3+ECL_LENGTH_DF812F	);	//Status Bytes

			glv_tagDF8129.Value[0]=MPP_OPS_STATUS_NA;
			glv_tagDF8129.Value[1]=MPP_OPS_START_NA;

			glv_tagDF8116.Value[0]=MPP_UIR_MID_NA;
			glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NA;
			memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);	//Hold Time=Message Hold Time

			glv_tagDF8115.Value[0]=MPP_EID_LV1_OK;
			glv_tagDF8115.Value[1]=MPP_EID_LV2_OK;
			glv_tagDF8115.Value[2]=MPP_EID_LV3_OK;
			memset(&glv_tagDF8115.Value[3], 0, 2);						//SW12='0000'
			glv_tagDF8115.Value[5]=MPP_EID_MOE_NA;
			break;

		case 4:		break;	//S51.4 IF [(Start Code = 'A') OR (Start Code = 'B')]
		case 5:		break;	//S51.5 Initialize the Candidate List Prepare SELECT command
		case 6:		break;	//S51.6 Send CA(SELECT) signal
		case 7:		break;	//S51.7 IF [Candidate List is empty]
		case 8:		break;	//S51.8 Selected Combination
		case 9:		break;	//S51.9 Prepare SELECT command
		case 10:	break;	//S51.10	Send CA(SELECT) signal

		case 11:	//S51.11
			glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
			glv_tagDF8116.Value[1]=MPP_UIR_STATUS_NotReady;
//			memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);		//Hold Time=Message Hold Time
			glv_tagDF8116.Value[2]=0x00;
			glv_tagDF8116.Value[3]=0x00;
			glv_tagDF8116.Value[4]=0x13;
			MPP_AddToList(mpp_tofDF8116, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

			MPP_MSG(mpp_tofDF8116);
			break;

		case 12:	//S51.12
			glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;
			glv_tagDF8115.Value[1]=MPP_EID_LV2_EmptyCandidateList;
			MPP_AddToList(mpp_tofDF8115, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			
			glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
			glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;


//EVAL 3B02-0001(second_trx_2_mutualappls_fail)
//Reset Start
glv_tagDF8129.Value[1]=MPP_OPS_START_NA;

			MPP_AddToList(mpp_tofDF8129, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofFF8106, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			
			MPP_Create_List(3, mpp_tofDF8115, glv_tagFF8106.Value);

			MPP_OUT(mpp_tofDF8129);
			MPP_OUT(mpp_tofFF8106);
			MPP_OUT(mpp_tofDF8116);
			break;

		case 13:	//S51.13
			glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;

			MPP_OUT(mpp_tofDF8129);
			MPP_OUT(mpp_tofFF8106);
			break;

		default:	break;
	}

//	MPP_DBG_Put_Process(MPP_STATE_51, symNum, 0xFF);
}


void MPP_State_52_WaitingForPPSEResponse(UCHAR symNum)
{
	switch (symNum)
	{
		case 1:	break;	//S52.1 Receive STOP signal
		case 2:	break;	//S52.2 Receive L1RSP signal with Return Code
		case 3:	break;	//S52.3 Receive RA signal with File Control Information Template and Status Bytes

		case 4:	//S52.4
			glv_tagDF8115.Length[0]=ECL_LENGTH_DF8115;
			glv_tagDF8115.Value[2]=MPP_EID_LV3_Stop;
			
			glv_tagDF8129.Length[0]=ECL_LENGTH_DF8129;
			glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;

			MPP_AddToList(mpp_tofDF8115, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofDF8129, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofFF8106, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			
			MPP_Create_List(3, mpp_tofDF8115, glv_tagFF8106.Value);

			MPP_OUT(mpp_tofDF8129);
			MPP_OUT(mpp_tofFF8106);
			break;

		case 5:	//S52.5
			glv_tagDF8129.Length[0]=ECL_LENGTH_DF8129;
			glv_tagDF8129.Value[0]=MPP_OPS_STATUS_TryAgain;
			glv_tagDF8129.Value[1]=MPP_OPS_START_B;

			glv_tagDF8115.Length[0]=ECL_LENGTH_DF8115;
			glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(etp_depRspCode);

			MPP_AddToList(mpp_tofDF8115, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofDF8129, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofFF8106, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			
			MPP_Create_List(3, mpp_tofDF8115, glv_tagFF8106.Value);
			
			MPP_OUT(mpp_tofDF8129);
			MPP_OUT(mpp_tofFF8106);
			break;

		case 6:	break;	//S52.6 IF [Status Bytes = '9000']

		case 7:	//S52.7
			glv_tagDF8115.Value[1]=MPP_EID_LV2_StatusBytes;	
			memcpy(&glv_tagDF8115.Value[3], etp_depSW, 2);
			break;

		case 8:	//S52.8
			glv_tagDF8116.Length[0]=ECL_LENGTH_DF8116;
			glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
//			memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);
			glv_tagDF8116.Value[2]=0x00;
			glv_tagDF8116.Value[3]=0x00;
			glv_tagDF8116.Value[4]=0x13;
			
			glv_tagDF8116.Value[1] = MPP_UIR_STATUS_NotReady;
			break;

		case 9:	//S52.9
			glv_tagDF8115.Length[0]=ECL_LENGTH_DF8115;
			glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;

			glv_tagDF8129.Length[0]=ECL_LENGTH_DF8129;
			glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
			glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

			MPP_AddToList(mpp_tofDF8115, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofDF8129, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofFF8106, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofDF8116, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			
			MPP_Create_List(3, mpp_tofDF8115, glv_tagFF8106.Value);
			
			MPP_OUT(mpp_tofDF8129);
			MPP_OUT(mpp_tofFF8106);
			MPP_OUT(mpp_tofDF8116);
			break;

		case 10:	break;	//S52.10 IF [Parsing successful with no error]

		case 11:	//S52.11
			glv_tagDF8115.Value[1]=MPP_EID_LV2_ParsingError;
			break;

		case 12:	break;	//S52.12 Build Candidate List
		case 13:	break;	//S52.13 IF [Candidate List is empty]

		case 14:	//S52.14
			glv_tagDF8115.Value[1]=MPP_EID_LV2_EmptyCandidateList;
			break;

		case 15:	break;	//S52.15	Selected Combination
		case 16:	break;	//S52.16	Prepare SELECT command
		case 17:	break;	//S52.17	Send CA(SELECT) signal

		case 18:	//S52.18
			glv_tagDF8116.Length[0]=ECL_LENGTH_DF8116;
			glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
//			memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);
			glv_tagDF8116.Value[2]=0x00;
			glv_tagDF8116.Value[3]=0x00;
			glv_tagDF8116.Value[4]=0x13;
			glv_tagDF8116.Value[1] = MPP_UIR_STATUS_NotReady;
			break;

		case 19:	//S52.19
			glv_tagDF8115.Length[0]=ECL_LENGTH_DF8115;
			glv_tagDF8115.Value[5]=MPP_EID_MOE_Error_OtherCard;

			glv_tagDF8129.Length[0]=ECL_LENGTH_DF8129;
			glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
			glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

			MPP_AddToList(mpp_tofDF8115, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofDF8129, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofFF8106, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofDF8116, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			
			MPP_Create_List(3, mpp_tofDF8115, glv_tagFF8106.Value);

			MPP_OUT(mpp_tofDF8129);
			MPP_OUT(mpp_tofFF8106);
			MPP_OUT(mpp_tofDF8116);
			break;

		default:	break;
	}

//	MPP_DBG_Put_Process(MPP_STATE_52, symNum, 0xFF);
}


void MPP_State_53_WaitingForAIDResponse(UCHAR symNum)
{
	switch (symNum)
	{
		case 1:	break;	//S53.1 Receive STOP signal
		case 2:	break;	//S53.2 Receive L1RSP signal with Return Code
		case 3:	break;	//S53.3 Receive RA signal 

		case 4:	//S53.4
			glv_tagDF8129.Length[0]=ECL_LENGTH_DF8129;
			glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
			MPP_AddToList(mpp_tofDF8129, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

			glv_tagDF8115.Length[0]=ECL_LENGTH_DF8115;
			glv_tagDF8115.Value[2]=MPP_EID_LV3_Stop;
			MPP_AddToList(mpp_tofDF8115, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			
			MPP_AddToList(mpp_tofFF8106, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			
			MPP_Create_List(3, mpp_tofDF8115, glv_tagFF8106.Value);

			MPP_OUT(mpp_tofDF8129);
			MPP_OUT(mpp_tofFF8106);
			break;

		case 5:	//S53.5
			glv_tagDF8129.Length[0]=ECL_LENGTH_DF8129;
			glv_tagDF8129.Value[0]=MPP_OPS_STATUS_TryAgain;
			glv_tagDF8129.Value[1]=MPP_OPS_START_B;
			MPP_AddToList(mpp_tofDF8129, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);

			glv_tagDF8115.Length[0]=ECL_LENGTH_DF8115;
			glv_tagDF8115.Value[0]=MPP_Get_ErrorIndication_L1(etp_depRspCode);
			MPP_AddToList(mpp_tofDF8115, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofFF8106, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			
			MPP_Create_List(3, mpp_tofDF8115, glv_tagFF8106.Value);
		
			MPP_OUT(mpp_tofDF8129);
			MPP_OUT(mpp_tofFF8106);
			break;

		case 6:	break;	//S53.6 IF [Status Bytes = '9000']

		case 7:	//S53.7
			glv_tagDF8129.Value[0]=MPP_OPS_STATUS_Approved;
			break;

		case 8:	//S53.8
/*			MPP_AddToList(mpp_tofDF8129, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofDF8115, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofFF8106, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_Create_List(3, mpp_tofDF8115, glv_tagFF8106.Value);
			
			MPP_OUT(mpp_tofDF8129);
			MPP_OUT(mpp_tof6F);
			MPP_OUT(mpp_tofDF8121);
			MPP_OUT(mpp_tofFF8106);
			MPP_OUT(mpp_tofDF812F);
*/
			break;

		case 9:		break;	//S53.9 IF [Candidate List is empty]
		case 10:	break;	//S53.10
		case 11:	break;	//S53.11 Prepare SELECT command according to section 3.3.2
		case 12:	break;	//S53.12 Send CA(SELECT)

		case 13:	//S53.13
			glv_tagDF8115.Value[1]=MPP_EID_LV2_EmptyCandidateList;
			MPP_AddToList(mpp_tofDF8115, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			
			break;

		case 14:	//S53.14
			glv_tagDF8116.Length[0]=ECL_LENGTH_DF8116;
			glv_tagDF8116.Value[0]=MPP_UIR_MID_Error_OtherCard;
			glv_tagDF8116.Value[1] = MPP_UIR_STATUS_NotReady;
//			memcpy(&glv_tagDF8116.Value[2], glv_tagDF812D.Value, 3);
			glv_tagDF8116.Value[2]=0x00;
			glv_tagDF8116.Value[3]=0x00;
			glv_tagDF8116.Value[4]=0x13;
			break;

		case 15:	//S53.15
			glv_tagDF8129.Length[0]=ECL_LENGTH_DF8129;
			glv_tagDF8129.Value[0]=MPP_OPS_STATUS_EndApplication;
			glv_tagDF8115.Value[5] = MPP_EID_MOE_Error_OtherCard;
			glv_tagDF8129.Value[4] |= MPP_OPS_UIRequestOnOutcomePresent;

			MPP_AddToList(mpp_tofDF8129, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofFF8106, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofDF8115, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			MPP_AddToList(mpp_tofDF8116, mpp_lstPresent.Value, MPP_LISTTYPE_TAG);
			
			MPP_Create_List(3, mpp_tofDF8115, glv_tagFF8106.Value);

			MPP_OUT(mpp_tofDF8129);
			MPP_OUT(mpp_tofFF8106);
			MPP_OUT(mpp_tofDF8116);
			break;

		default:	break;
	}

//	MPP_DBG_Put_Process(MPP_STATE_53, symNum, 0xFF);
}


void MPP_StateProcess_EMVMode(void)
{
	UCHAR	strS910=0;	//Start of State 910
	UCHAR	rspCode=0;

	rspCode=MPP_State_1_Idle();
	if (rspCode == MPP_STATE_3)
	{
		rspCode=MPP_State_3_WaitingForGPOResponse();
		if (rspCode == MPP_STATE_4)
		{
		//MasterCard M/Chip Scheme
		SchemeID = 0x21;

		while(1)
		{
			rspCode = MPP_State_4_WaitingForEMVReadRecordResponse();
			if(rspCode == MPP_STATE_456_A)
			{
				rspCode = MPP_State_456_CommonProcessing();
				if(rspCode != MPP_STATE_4)
				{
					break;
				}
			}
			else
			{
				break;
			}
		}

		if(rspCode == MPP_STATE_4Apostrophes)
		{
			rspCode = MPP_State_4Apostrophes_TerminateOnNextRA();
		}

		if(rspCode == MPP_STATE_9)
		{
			rspCode = MPP_State_9_WaitingForGenerateACResponse1();
		}
		else if(rspCode == MPP_STATE_10)
		{
			rspCode = MPP_State_10_WaitingForRecoverACResponse();
		}

		if((rspCode == MPP_STATE_910_A) ||
			(rspCode == MPP_STATE_910_B) ||
			(rspCode == MPP_STATE_910_C))
		{
			strS910 = rspCode;

			rspCode = MPP_State_910_CommonProcessing(strS910);
		}
		else if(rspCode == MPP_STATE_11)
		{
			rspCode = MPP_State_11_WaitingForGenerateACResponse2();
		}
		}
		else if(rspCode == MPP_STATE_7)
		{
			//MasterCard Mag-Stripe Scheme
			SchemeID = 0x20;

			while(1)
			{
				rspCode = MPP_State_7_WaitingForMagStripeReadRecordResponse();
				if(rspCode != MPP_STATE_7)
				{
					break;
				}
			}

			if(rspCode == MPP_STATE_78_A)
			{
				rspCode = MPP_State_78_CommonProcessing();
				if(rspCode == MPP_STATE_13)
				{
					rspCode = MPP_State_13_WaitingForCCCResponse1();
				}
				else if(rspCode == MPP_STATE_14)
				{
					rspCode = MPP_State_14_WaitingForCCCResponse2();
				}
			}
		}
	}
}

void MPP_UploadReceipt(void)
{
	 UINT lenValue;
	 UCHAR bufRcv[512] = { 0 };
	 UCHAR *ptrRcv = bufRcv;

	 //AID(var)
	 memcpy(ptrRcv, (UCHAR*)&"AID:", 4);
	 ptrRcv += 4;
	 UT_Get_TLVLengthOfV(glv_tag9F06.Length, &lenValue);
	 memcpy(ptrRcv, glv_tag9F06.Value, lenValue);
	 ptrRcv += lenValue;

	 //DF Name(var)
	 memcpy(ptrRcv, (UCHAR*)&"DFN:", 4);
	 ptrRcv += 4;
	 UT_Get_TLVLengthOfV(glv_tag84.Length, &lenValue);
	 memcpy(ptrRcv, glv_tag84.Value, lenValue);
	 ptrRcv += lenValue;

	 //PAN(var)
	 UT_Get_TLVLengthOfV(glv_tag5A.Length, &lenValue);
	 if (lenValue != 0)
	 {
		  memcpy(ptrRcv, (UCHAR*)&"PAN:", 4);
		  ptrRcv += 4;
		  memcpy(ptrRcv, glv_tag5A.Value, lenValue);
		  ptrRcv += lenValue;
	 }
	 else
	 {
		  memcpy(ptrRcv, (UCHAR*)&"PAN2:", 5);
		  ptrRcv += 5;
		  UT_Get_TLVLengthOfV(glv_tag9F6B.Length, &lenValue);
		  memcpy(ptrRcv, glv_tag9F6B.Value, lenValue);
		  ptrRcv += lenValue;
	 }

	 //Transaction Type(1)
	 memcpy(ptrRcv, (UCHAR*)&"TransType:", 10);
	 ptrRcv += 10;
	 memcpy(ptrRcv, glv_tag9C.Value, ECL_LENGTH_9C);
	 ptrRcv += ECL_LENGTH_9C;

	 //Date(3)
	 memcpy(ptrRcv, (UCHAR*)&"Date:", 5);
	 ptrRcv += 5;
	 memcpy(ptrRcv, glv_tag9A.Value, ECL_LENGTH_9A);
	 ptrRcv += ECL_LENGTH_9A;

	 //Time(3)
	 memcpy(ptrRcv, (UCHAR*)&"Time:", 5);
	 ptrRcv += 5;
	 memcpy(ptrRcv, glv_tag9F21.Value, ECL_LENGTH_9F21);
	 ptrRcv += ECL_LENGTH_9F21;

	 //Amount(6)
	 memcpy(ptrRcv, (UCHAR*)&"Amount:", 7);
	 ptrRcv += 7;
	 memcpy(ptrRcv, glv_tag9F02.Value, ECL_LENGTH_9F02);
	 ptrRcv += ECL_LENGTH_9F02;

	 //TVR(5)

	 TEI_Upload_Receipt(ptrRcv - bufRcv, bufRcv);
}


//Kernel state process is modified by William
void MPP_KernelStateProcess2(void)
{
	UCHAR	rspCode = MPP_STATE_1, Exit = FALSE;
	
	while (!Exit)
	{
		switch (rspCode)
		{
			case MPP_STATE_1:				rspCode = MPP_State_1_Idle();											break;
			case MPP_STATE_2:				rspCode = MPP_State_2_WaitingForPDOLData();								break;
			case MPP_STATE_3:				rspCode = MPP_State_3_WaitingForGPOResponse();							break;
			case MPP_STATE_3R1_D:			rspCode = MPP_STATE_3R1_D_CommonProcessing();							break;
			case MPP_STATE_R1:				rspCode = MPP_State_R1_WaitingForExchangeRelayResistanceDataResponse();	break;
			case MPP_STATE_4:				rspCode = MPP_State_4_WaitingForEMVReadRecordResponse();				break;
			case MPP_STATE_4Apostrophes:	rspCode = MPP_State_4Apostrophes_TerminateOnNextRA();					break;
			case MPP_STATE_5:				rspCode = MPP_State_5_WaitingForGetDataResponse();						break;
			case MPP_STATE_6:				rspCode = MPP_State_6_WaitingForEMVModeFirstWriteFlag();				break;
			case MPP_STATE_456_A:			rspCode = MPP_State_456_CommonProcessing();								break;
			case MPP_STATE_7:				rspCode = MPP_State_7_WaitingForMagStripeReadRecordResponse();			break;
			case MPP_STATE_8:				rspCode = MPP_State_8_WaitingForMagStripeFirstWriteFlag();				break;
			case MPP_STATE_78_A:			rspCode = MPP_State_78_CommonProcessing();								break;
			case MPP_STATE_9:				rspCode = MPP_State_9_WaitingForGenerateACResponse1();					break;
			case MPP_STATE_10:				rspCode = MPP_State_10_WaitingForRecoverACResponse();					break;
			case MPP_STATE_910_A:
			case MPP_STATE_910_B:
			case MPP_STATE_910_C:			rspCode = MPP_State_910_CommonProcessing(rspCode);						break;
			case MPP_STATE_11:				rspCode = MPP_State_11_WaitingForGenerateACResponse2();					break;
			case MPP_STATE_12:				rspCode = MPP_State_12_WaitingForPutDataResponseBeforeGenerateAC();		break;
			case MPP_STATE_13:				rspCode = MPP_State_13_WaitingForCCCResponse1();						break;
			case MPP_STATE_14:				rspCode = MPP_State_14_WaitingForCCCResponse2();						break;
			case MPP_STATE_15:				rspCode = MPP_State_15_WaitingForPutDataResponseAfterGenerateAC();		break;
			case MPP_STATE_16:				rspCode = MPP_State_16_WaitingForPreGenACBalance();						break;
			case MPP_STATE_17:				rspCode = MPP_State_17_WaitingForPostGenACBalance();					break;

			case MPP_STATE_EXITKERNEL:
			case MPP_STATE_RETURN:			Exit = TRUE;															break;
			default:																								break;
		}

		if (glv_tag95.Value[4] & MPP_TVR_RRPPerformed)	//Tammy 2017/11/22
		{
			// (Wayne comment , seem it not for trsaction) 21/01/15
			//TEI_Upload_RRP_Data(ECL_LENGTH_DF8306, glv_tagDF8306.Value);
		}
	}

	// william	2017/11/20	prevent Sending Buffer too large			
	int TEI_upCnt = 0, TEI_totalCnt, MaxSend = 100;
	TEI_totalCnt = mpp_ptrDE_log - mpp_DE_log;
	while (TEI_upCnt < TEI_totalCnt)
	{
		if (TEI_totalCnt - TEI_upCnt >= MaxSend)
		{
			// (Wayne comment , seem it not for trsaction) 21/01/15
			//TEI_Upload_DE_Data(MaxSend, mpp_DE_log + TEI_upCnt);
			TEI_upCnt += MaxSend;
		}
		else	//TEI_totalCnt - TEI_upCnt < MaxSend
		{
			// (Wayne comment , seem it not for trsaction) 21/01/15
			//TEI_Upload_DE_Data(TEI_totalCnt - TEI_upCnt, mpp_DE_log + TEI_upCnt);
			TEI_upCnt += TEI_totalCnt - TEI_upCnt;
		}
	}


	// william 2017/12/4	Upload Receipt
	// (Wayne comment , seem it not for trsaction) 21/01/15
	//MPP_UploadReceipt();


}

void MPP_KernelStateProcess(void)
{
	UCHAR	rspCode = 0;
	UCHAR	flgS3 = FALSE;
	UCHAR	flgS3R1 = FALSE;
	UCHAR	flgS456 = FALSE;
	UCHAR	flgS9 = FALSE;
	UCHAR	flgS10 = FALSE;
	UCHAR	flgS910 = FALSE;
	UCHAR	strS910 = 0;	//Start of State 910

	rspCode = MPP_State_1_Idle();
	if(rspCode == MPP_STATE_2)
	{
		while(1)
		{
			rspCode = MPP_State_2_WaitingForPDOLData();
			if(rspCode != MPP_STATE_2)
			{
				break;
			}
		}
		
		if(rspCode == MPP_STATE_3)
		{
			flgS3 = TRUE;
		}
	}
	else if(rspCode == MPP_STATE_3)
	{
		flgS3 = TRUE;
	}

	if(flgS3 == TRUE)
	{
		while(1)
		{
			rspCode = MPP_State_3_WaitingForGPOResponse();
			if(rspCode != MPP_STATE_3)
			{
				break;
			}
		}
		
		if(rspCode == MPP_STATE_R1)
		{
			while(1)
			{
				rspCode = MPP_State_R1_WaitingForExchangeRelayResistanceDataResponse();
				if(rspCode != MPP_STATE_R1)
				{
					break;
				}
			}

			if(rspCode == MPP_STATE_3R1_D)
			{
				flgS3R1 = TRUE;
			}
		}
		else if(rspCode == MPP_STATE_3R1_D)
		{
			flgS3R1 = TRUE;
		}
		else if(rspCode == MPP_STATE_7)
		{
			while(1)
			{
				rspCode = MPP_State_7_WaitingForMagStripeReadRecordResponse();
				if(rspCode != MPP_STATE_7)
				{
					break;
				}
			}
			
			if(rspCode == MPP_STATE_78_A)
			{
				while(1)
				{
					rspCode = MPP_State_78_CommonProcessing();
					if(rspCode == MPP_STATE_8)
					{
						rspCode = MPP_State_8_WaitingForMagStripeFirstWriteFlag();
						if(rspCode != MPP_STATE_78_A)
						{
							break;
						}
					}
					else
					{
						break;
					}
				}

				if(rspCode == MPP_STATE_13)
				{
					while(1)
					{
						rspCode = MPP_State_13_WaitingForCCCResponse1();
						if(rspCode != MPP_STATE_13)
						{
							break;
						}
					}
				}
				else if(rspCode == MPP_STATE_14)
				{
					while(1)
					{
						rspCode = MPP_State_14_WaitingForCCCResponse2();
						if(rspCode != MPP_STATE_14)
						{
							break;
						}
					}
				}
			}
		}

		if(flgS3R1 == TRUE)
		{
			rspCode = MPP_STATE_3R1_D_CommonProcessing();
			if(rspCode == MPP_STATE_4)
			{
				while(1)
				{
					rspCode = MPP_State_4_WaitingForEMVReadRecordResponse();
					if(rspCode == MPP_STATE_456_A)
					{
						rspCode = MPP_State_456_CommonProcessing();
						if(rspCode != MPP_STATE_4)
						{
							break;
						}
					}
					else if(rspCode != MPP_STATE_4)
					{
						break;
					}
				}

				if(rspCode == MPP_STATE_4Apostrophes)
				{
					rspCode = MPP_State_4Apostrophes_TerminateOnNextRA();
				}
				
				if(rspCode == MPP_STATE_5)
				{
					while(1)
					{
						rspCode = MPP_State_5_WaitingForGetDataResponse();
						if(rspCode != MPP_STATE_5)
						{
							break;
						}
					}

					if(rspCode == MPP_STATE_456_A)
					{
						flgS456 = TRUE;
					}
				}
				else if(rspCode == MPP_STATE_6)
				{
					rspCode = MPP_State_6_WaitingForEMVModeFirstWriteFlag();
					if(rspCode == MPP_STATE_456_A)
					{
						flgS456 = TRUE;
					}
				}
				else if(rspCode == MPP_STATE_9)
				{
					flgS9 = TRUE;
				}
				else if(rspCode == MPP_STATE_10)
				{
					flgS10 = TRUE;
				}
				else if(rspCode == MPP_STATE_12)
				{
					while(1)
					{
						rspCode = MPP_State_12_WaitingForPutDataResponseBeforeGenerateAC();
						if(rspCode != MPP_STATE_12)
						{
							break;
						}
					}

					if(rspCode == MPP_STATE_9)
					{
						flgS9 = TRUE;
					}
					else if(rspCode == MPP_STATE_10)
					{
						flgS10 = TRUE;
					}
				}
			}
			else if(rspCode == MPP_STATE_5)
			{
				while(1)
				{
					rspCode = MPP_State_5_WaitingForGetDataResponse();
					if(rspCode != MPP_STATE_5)
					{
						break;
					}
				}

				if(rspCode == MPP_STATE_456_A)
				{
					flgS456 = TRUE;
				}
			}

			if(flgS456 == TRUE)
			{
				while(1)
				{
					MPP_DBG_Put_String(10, (UCHAR*)&"??????????");
					rspCode = MPP_State_456_CommonProcessing();
					if(rspCode == MPP_STATE_4)
					{
						while(1)
						{
							rspCode = MPP_State_4_WaitingForEMVReadRecordResponse();
							if(rspCode != MPP_STATE_4)
							{
								break;
							}
						}
						
						if(rspCode == MPP_STATE_4Apostrophes)
						{
							rspCode = MPP_State_4Apostrophes_TerminateOnNextRA();

							break;
						}
						else if(rspCode == MPP_STATE_456_A)
						{
							;	//GOTO S456_A
						}
						else
						{
							break;
						}
					}
					else if(rspCode == MPP_STATE_5)
					{
						while(1)
						{
							rspCode = MPP_State_5_WaitingForGetDataResponse();
							if(rspCode != MPP_STATE_5)
							{
								break;
							}
						}

						if(rspCode == MPP_STATE_456_A)
						{
							;	//GOTO S456_A
						}
						else
						{
							break;
						}
					}
					else if(rspCode == MPP_STATE_6)
					{
						rspCode = MPP_State_6_WaitingForEMVModeFirstWriteFlag();
						if(rspCode == MPP_STATE_456_A)
						{
							;	//GOTO S456_A
						}
						else
						{
							break;
						}
					}
					else if(rspCode == MPP_STATE_9)
					{
						flgS9 = TRUE;

						break;
					}
					else if(rspCode == MPP_STATE_10)
					{
						flgS10 = TRUE;

						break;
					}
					else if(rspCode == MPP_STATE_12)
					{
						while(1)
						{
							rspCode = MPP_State_12_WaitingForPutDataResponseBeforeGenerateAC();
							if(rspCode != MPP_STATE_12)
							{
								break;
							}
						}

						if(rspCode == MPP_STATE_9)
						{
							flgS9 = TRUE;

							break;
						}
						else if(rspCode == MPP_STATE_10)
						{
							flgS10 = TRUE;

							break;
						}

					}
					else
					{
						break;
					}
				}
			}

			if(flgS9 == TRUE)
			{
				while(1)
				{
					rspCode = MPP_State_9_WaitingForGenerateACResponse1();
					if(rspCode != MPP_STATE_9)
					{
						break;
					}
				}

				if((rspCode == MPP_STATE_910_A) ||
				   (rspCode == MPP_STATE_910_B) ||
				   (rspCode == MPP_STATE_910_C))
				{
					strS910 = rspCode;
					
					flgS910 = TRUE;
				}
			}

			if(flgS10 == TRUE)
			{
				while(1)
				{
					rspCode = MPP_State_10_WaitingForRecoverACResponse();
					if(rspCode != MPP_STATE_10)
					{
						break;
					}
				}

				if(rspCode == MPP_STATE_11)
				{
					while(1)
					{
						rspCode = MPP_State_11_WaitingForGenerateACResponse2();
						if(rspCode != MPP_STATE_11)
						{
							break;
						}
					}
					
					if(rspCode == MPP_STATE_15)
					{
						while(1)
						{
							rspCode = MPP_State_15_WaitingForPutDataResponseAfterGenerateAC();
							if(rspCode != MPP_STATE_15)
							{
								break;
							}
						}
					}
				}
				else if((rspCode == MPP_STATE_910_A) ||
						(rspCode == MPP_STATE_910_B) ||
						(rspCode == MPP_STATE_910_C))
				{
					strS910 = rspCode;
					
					flgS910 = TRUE;
				}
			}

			if(flgS910 == TRUE)
			{
				rspCode = MPP_State_910_CommonProcessing(strS910);
				if(rspCode == MPP_STATE_15)
				{
					while(1)
					{
						rspCode = MPP_State_15_WaitingForPutDataResponseAfterGenerateAC();
						if(rspCode != MPP_STATE_15)
						{
							break;
						}
					}
				}
			}
		}

		if(glv_tag95.Value[4] & MPP_TVR_RRPPerformed)	//Tammy 2017/11/22
		{
			TEI_Upload_RRP_Data(ECL_LENGTH_DF8306, glv_tagDF8306.Value);
		}
	}

	// william	2017/11/20	prevent Sending Buffer too large			
	int TEI_upCnt = 0, TEI_totalCnt, MaxSend = 100;
	TEI_totalCnt = mpp_ptrDE_log - mpp_DE_log;
	while (TEI_upCnt < TEI_totalCnt)
	{
		if (TEI_totalCnt - TEI_upCnt >= MaxSend)
		{
			TEI_Upload_DE_Data(MaxSend, mpp_DE_log + TEI_upCnt);
			TEI_upCnt += MaxSend;
		}
		else	//TEI_totalCnt - TEI_upCnt < MaxSend
		{
			TEI_Upload_DE_Data(TEI_totalCnt - TEI_upCnt, mpp_DE_log + TEI_upCnt);
			TEI_upCnt += TEI_totalCnt - TEI_upCnt;
		}
	}

	// william 2017/12/4	Upload Receipt
	MPP_UploadReceipt();
}

UCHAR MPP_Allocate_Memory(void)
{
	UCHAR i, j;
	
	mpp_tempBuff = malloc(sizeof(unsigned char) * (ECL_LENGTH_DF8107 + ECL_LENGTH_9F5B + 3));
	if (mpp_tempBuff == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tempSend.Length = mpp_tempBuff;
	mpp_tempSend.Value = mpp_tempBuff + 3;

	mpp_bufPresent = malloc(sizeof(unsigned char) * (3+MPP_TLVLIST_BUFFERSIZE));
	if (mpp_bufPresent == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_lstPresent.Length = mpp_bufPresent;
	mpp_lstPresent.Value = mpp_bufPresent + 3;

	mpp_bufTlvDB = malloc(sizeof(unsigned char) * (3+MPP_TLVLIST_BUFFERSIZE));
	if (mpp_bufTlvDB == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_lstTlvDB.Length = mpp_bufTlvDB;
	mpp_lstTlvDB.Value = mpp_bufTlvDB + 3;

	mpp_depRcvData = malloc(sizeof(unsigned char) * ETP_BUFFER_SIZE);
	if (mpp_depRcvData == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_DETRcvBuff = malloc(sizeof(unsigned char) * MPP_DET_BUFFER_RECEIVE);
	if (mpp_DETRcvBuff == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_gdcRcvData = malloc(sizeof(unsigned char) * ETP_BUFFER_SIZE);
	if (mpp_gdcRcvData == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_rrcRcvData = malloc(sizeof(unsigned char) * ETP_BUFFER_SIZE);
	if (mpp_rrcRcvData == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_vfyData = malloc(sizeof(unsigned char) * ETP_BUFFER_SIZE);
	if (mpp_vfyData == NULLPTR)
	{
		return FAIL;
	}

	

	for (i=0; i < MPP_TORN_AIDNUMBER; i++)
	{
		for (j=0; j < MPP_TORN_RECORDNUMBER; j++)
		{
			mpp_trnRec[i][j].Record=malloc(sizeof(unsigned char) * MPP_TORN_BUFFERSIZE);
			if (mpp_trnRec[i][j].Record == NULLPTR)
			{
				return FAIL;
			}
			
		}
	}

	//MPP_Tag
	mpp_bufActiveAFL = malloc(sizeof(unsigned char) * (3 + MPP_LENGTH_ActiveAFL));
	if (mpp_bufActiveAFL == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagActiveAFL.Length = mpp_bufActiveAFL;
	mpp_tagActiveAFL.Value = mpp_bufActiveAFL + 3;

	mpp_bufActiveTag = malloc(sizeof(unsigned char) * (3 + MPP_LENGTH_ActiveTag));
	if (mpp_bufActiveTag == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagActiveTag.Length = mpp_bufActiveTag;
	mpp_tagActiveTag.Value = mpp_bufActiveTag + 3;

	mpp_bufACType = malloc(sizeof(unsigned char) * (3 + MPP_LENGTH_ACType));
	if (mpp_bufACType == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagACType.Length = mpp_bufACType;
	mpp_tagACType.Value = mpp_bufACType + 3;

	mpp_bufFailedMSCntr = malloc(sizeof(unsigned char) * (3 + MPP_LENGTH_FailedMSCntr));
	if (mpp_bufFailedMSCntr == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagFailedMSCntr.Length = mpp_bufFailedMSCntr;
	mpp_tagFailedMSCntr.Value = mpp_bufFailedMSCntr + 3;

	mpp_bufNextCmd = malloc(sizeof(unsigned char) * (3 + MPP_LENGTH_NextCmd));
	if (mpp_bufNextCmd == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagNextCmd.Length = mpp_bufNextCmd;
	mpp_tagNextCmd.Value = mpp_bufNextCmd + 3;

	mpp_bufnUN = malloc(sizeof(unsigned char) * (3 + MPP_LENGTH_nUN));
	if (mpp_bufnUN == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagnUN.Length = mpp_bufnUN;
	mpp_tagnUN.Value = mpp_bufnUN + 3;

	mpp_bufODAStatus = malloc(sizeof(unsigned char) * (3 + MPP_LENGTH_ODAStatus));
	if (mpp_bufODAStatus == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagODAStatus.Length = mpp_bufODAStatus;
	mpp_tagODAStatus.Value = mpp_bufODAStatus + 3;

	mpp_bufReaderContactlessTransactionLimit = malloc(sizeof(unsigned char) * (3 + MPP_LENGTH_ReaderContactlessTransactionLimit));
	if (mpp_bufReaderContactlessTransactionLimit == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagReaderContactlessTransactionLimit.Length = mpp_bufReaderContactlessTransactionLimit;
	mpp_tagReaderContactlessTransactionLimit.Value = mpp_bufReaderContactlessTransactionLimit + 3;

	mpp_bufStaticDataToBeAuthenticated = malloc(sizeof(unsigned char) * (3 + MPP_LENGTH_StaticDataToBeAuthenticated));
	if (mpp_bufStaticDataToBeAuthenticated == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagStaticDataToBeAuthenticated.Length = mpp_bufStaticDataToBeAuthenticated;
	mpp_tagStaticDataToBeAuthenticated.Value = mpp_bufStaticDataToBeAuthenticated + 3;

	mpp_bufTagsToReadYet = malloc(sizeof(unsigned char) * (3 + MPP_TLVLIST_BUFFERSIZE));
	if (mpp_bufTagsToReadYet == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagTagsToReadYet.Length = mpp_bufTagsToReadYet;
	mpp_tagTagsToReadYet.Value = mpp_bufTagsToReadYet + 3;

	mpp_bufTagsToWriteYetAfterGenAC = malloc(sizeof(unsigned char) * (3 + MPP_TLVLIST_BUFFERSIZE));
	if (mpp_bufTagsToWriteYetAfterGenAC == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagTagsToWriteYetAfterGenAC.Length = mpp_bufTagsToWriteYetAfterGenAC;
	mpp_tagTagsToWriteYetAfterGenAC.Value = mpp_bufTagsToWriteYetAfterGenAC + 3;

	mpp_bufTagsToWriteYetBeforeGenAC = malloc(sizeof(unsigned char) * (3 + MPP_TLVLIST_BUFFERSIZE));
	if (mpp_bufTagsToWriteYetBeforeGenAC == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagTagsToWriteYetBeforeGenAC.Length = mpp_bufTagsToWriteYetBeforeGenAC;
	mpp_tagTagsToWriteYetBeforeGenAC.Value = mpp_bufTagsToWriteYetBeforeGenAC + 3;

	mpp_bufTornEntry = malloc(sizeof(unsigned char) * (3 + MPP_LENGTH_TornEntry));
	if (mpp_bufTornEntry == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagTornEntry.Length = mpp_bufTornEntry;
	mpp_tagTornEntry.Value = mpp_bufTornEntry + 3;

	mpp_bufTornTempRecord = malloc(sizeof(unsigned char) * (3 + MPP_LENGTH_TornTempRecord));
	if (mpp_bufTornTempRecord == NULLPTR)
	{
		return FAIL;
	}
	
	mpp_tagTornTempRecord.Length = mpp_bufTornTempRecord;
	mpp_tagTornTempRecord.Value = mpp_bufTornTempRecord + 3;

	return SUCCESS;
}

void MPP_Free_Memory(void)
{
	UCHAR i, j;

	free(mpp_tempBuff);
	free(mpp_bufPresent);
	free(mpp_bufTlvDB);
	free(mpp_depRcvData);
	free(mpp_DETRcvBuff);
	free(mpp_gdcRcvData);
	free(mpp_rrcRcvData);
	free(mpp_vfyData);

	for (i=0; i < MPP_TORN_AIDNUMBER; i++)
	{
		for (j=0; j < MPP_TORN_RECORDNUMBER; j++)
		{
			free(mpp_trnRec[i][j].Record);
		}
	}

	//MPP_Tag
	free(mpp_bufActiveAFL);
	free(mpp_bufActiveTag);
	free(mpp_bufACType);
	free(mpp_bufFailedMSCntr);
	free(mpp_bufNextCmd);
	free(mpp_bufnUN);
	free(mpp_bufODAStatus);
	free(mpp_bufReaderContactlessTransactionLimit);
	free(mpp_bufStaticDataToBeAuthenticated);
	free(mpp_bufTagsToReadYet);
	free(mpp_bufTagsToWriteYetAfterGenAC);
	free(mpp_bufTagsToWriteYetBeforeGenAC);
	free(mpp_bufTornEntry);
	free(mpp_bufTornTempRecord);
}

void MPP_Start_Kernel(UINT lenFCI, UCHAR *datFCI)
{
	UCHAR	rspCode=FAIL;
	
	if (lenFCI < 2)	return;	//Communication Error

	//Copy FCI to Sync Data
	mpp_synDatLen=lenFCI-2;	//Exclude SW12
	mpp_synData=datFCI;

	//Add Signal
	MPP_Add_Signal(mpp_strSignal);

	//Memory Allocate Buffer
	rspCode=MPP_Allocate_Memory();
	if (rspCode == FAIL)
	{
DBG_Put_Text("malloc fail");
		return;
	}

	//Reset Parameter
	MPP_Reset_Parameter();

	//Load Transaction Data Tag
	ETP_Load_TransactionData();

	//Load Configuration
	MPP_Load_TransactionData();
	MPP_Load_KernelConfiguration();

	//Main Process
	MPP_Kernel_Started();
//EMV mode for [MCL V3.0.2] remarked by Tammy 2017/10/18
//	MPP_StateProcess_EMVMode();
//	MPP_KernelStateProcess();	//Tammy 2017/10/18	[MCL V3.1.1]
//Test
MPP_KernelStateProcess2();

	//Clear Present List & Signal
	MPP_Reset_PresentList();
	MPP_Clear_Signal();

	// Clear/Free malloc buffer
	MPP_Free_Memory();

	//Outcome Process
	ETP_MPP_OutcomeProcess();
}


