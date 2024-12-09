#include <string.h>
#include <stdlib.h>
#include "POSAPI.h"
#include "Define.h"
#include "ECL_Tag.h"
#include "Function.h"
#include "UNP_Define.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Function.h"
#include "ITR_Function.h"
#include "ODA_Record.h"
#include "VAP_ReaderInterface_Define.h"
#include "Datastructure.h"

//Test
#include "DBG_Function.h"


UINT	unp_rcvLen=0;
//UCHAR	unp_rcvData[ETP_BUFFER_SIZE];
UCHAR	*unp_rcvData;

UINT	unp_lstLen;
//UCHAR	unp_lstData[UNP_LIST_BUFFER_SIZE];
UCHAR	*unp_lstData;

UCHAR	unp_flgNewCard=FALSE;

UCHAR	unp_txnResult;	//Transaction Result
UCHAR	unp_addMessage;	//Additional Message


//		L3 Parameters
extern	OUTCOME	etp_Outcome;
extern	UCHAR	etp_flgComError;
extern	UCHAR	etp_flgCDCVM;

extern	CAPK	glv_CAPK[CAPK_NUMBER];
extern	UCHAR	Revo_List[10][9];
extern	UCHAR	Exception_File[10][11];
extern	UCHAR	SchemeID;
extern	UCHAR	L3_Response_Code;
extern	UCHAR	Online_Data[1024];
extern	UINT	Online_Data_Length;

extern	void	api_pcd_mfc_iso_ActivateCard(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	UCHAR	VGL_AS210_D1_Track(void);
extern	UCHAR	VGL_qVSDC_Online_Data(void);


// Add by Wayne 2020/08/21 to avoid compiler warning
#include "UTILS_CTLS.H"


#if 0
UCHAR UNP_Get_TransactionLog(void)
{
	UCHAR	rspCode=0;
	UCHAR	cntRecNo=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	tag61[1]={0x61};
	UCHAR	tagBF0C[2]={0xBF,0x0C};
	UCHAR	tag4F[1]={0x4F};
	UCHAR	tag9F4D[2]={0x9F,0x4D};
	UCHAR	tag9F4F[2]={0x9F,0x4F};
	UCHAR	ridUPI[5]={0xA0,0x00,0x00,0x03,0x33};
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	sndData[32];
	UCHAR 	cmdPPSE[20]={	
				0x00,0xA4,0x04,0x00,
				0x0E,
				'2','P','A','Y','.','S','Y','S','.','D','D','F','0','1',
				0x00};
	UCHAR 	cmdSelect[16]={
				0x00,0xA4,0x04,0x00,
				0x00,
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	UCHAR	cmdGet[5]={
				0x80,0xCA,0x9F,0x4F,
				0x00};
	UCHAR	cmdRead[5]={
				0x00,0xB2,0x00,0x00,
				0x00};					


DBG_Put_Text("");
DBG_Put_DateTime();


#ifdef _PLATFORM_AS350
	ITR_Reset_Flags();
#endif

	//	Display Landing Plan
#ifdef _SCREEN_SIZE_240x320
	UT_Set_LEDSignal(1, 0xFFFF, 0x0000);
	UT_WaitTime(50);
	UT_PutStr(2, 4, FONT1, 12, (UCHAR*)"Present Card");
	UT_PutStr(3, 7, FONT1, 6, (UCHAR*)"Please");
#else
	UT_ClearScreen();
	UT_PutStr(1, 2, FONT1, 12, (UCHAR*)"Present Card");
	UT_PutStr(2, 5, FONT1, 6, (UCHAR*)"Please");
#endif

	//
	//	Activate Card
	//
	sndData[0]=0x00;
	sndData[1]=0x01;
	sndData[2]=0x08;	//Polling: 8 Seconds
	api_pcd_mfc_iso_ActivateCard(&sndData[0], &sndData[2], &unp_rcvData[0], &unp_rcvData[2]);
	if (unp_rcvData[2] != 0x00) return FAIL;

	//
	//	Select PPSE
	//
	unp_rcvLen=0;
	rspCode=ECL_LV1_DEP(20, cmdPPSE, &unp_rcvLen, unp_rcvData, 1000);
	if (rspCode != ECL_LV1_SUCCESS) return FAIL;

//DBG_Put_Dialog(0, 20, cmdPPSE);
//DBG_Put_Dialog(1, unp_rcvLen, unp_rcvData);

	//Check SW12
	if ((unp_rcvData[unp_rcvLen-2] != 0x90) || (unp_rcvData[unp_rcvLen-1] != 0x00)) return FAIL;

	//Check Mandatory
	ptrData=UT_Find_Tag(tag61, (unp_rcvLen-2), unp_rcvData);
	if (ptrData == NULLPTR) return FAIL;
	
	ptrData=UT_Find_Tag(tagBF0C, (unp_rcvLen-2), unp_rcvData);
	if (ptrData == NULLPTR) return FAIL;

	ptrData=UT_Find_Tag(tag4F, (unp_rcvLen-2), unp_rcvData);
	if (ptrData == NULLPTR) return FAIL;

	//
	//	Select Application
	//
	UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
	if (memcmp(&ptrData[lenOfT+lenOfL], ridUPI, 5)) return FAIL;

	cmdSelect[4]=lenOfV;
	memcpy(&cmdSelect[5], &ptrData[lenOfT+lenOfL], lenOfV);

	unp_rcvLen=0;
	rspCode=ECL_LV1_DEP((6+lenOfV), cmdSelect, &unp_rcvLen, unp_rcvData, 1000);
	if (rspCode != ECL_LV1_SUCCESS) return FAIL;

//DBG_Put_Dialog(0, (6+lenOfV), cmdSelect);
//DBG_Put_Dialog(1, unp_rcvLen, unp_rcvData);

	//Check SW12
	if ((unp_rcvData[unp_rcvLen-2] != 0x90) || (unp_rcvData[unp_rcvLen-1] != 0x00))
	{
		//Log format and transaction log record can be still accessed after the application is blocked
		if ((unp_rcvData[unp_rcvLen-2] != 0x62) || (unp_rcvData[unp_rcvLen-1] != 0x83))
		{
			return FAIL;
		}
	}

	//Find 9F4D
	ptrData=UT_Find_Tag(tag9F4D, (unp_rcvLen-2), unp_rcvData);
	if (ptrData == NULLPTR) return FAIL;

	//Save 9F4D
	UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
	UT_Set_TagLength(lenOfV, glv_tag9F4D.Length);
	memcpy(glv_tag9F4D.Value, &ptrData[lenOfT+lenOfL], lenOfV);

	
	//
	//	Get Data
	//
	unp_rcvLen=0;
	rspCode=ECL_LV1_DEP(5, cmdGet, &unp_rcvLen, unp_rcvData, 1000);
	if (rspCode != ECL_LV1_SUCCESS) return FAIL;

//DBG_Put_Dialog(0, 5, cmdGet);
//DBG_Put_Dialog(1, unp_rcvLen, unp_rcvData);

	//Check SW12
	if ((unp_rcvData[unp_rcvLen-2] != 0x90) || (unp_rcvData[unp_rcvLen-1] != 0x00)) return FAIL;
	
	//Find 9F4F
	ptrData=UT_Find_Tag(tag9F4F, (unp_rcvLen-2), unp_rcvData);
	if (ptrData == NULLPTR) return FAIL;

	//Save 9F4F
	UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
	UT_Set_TagLength(lenOfV, glv_tag9F4F.Length);
	memcpy(glv_tag9F4F.Value, &ptrData[lenOfT+lenOfL], lenOfV);

	
	//
	//	Read Log
	//
	for (cntRecNo=1; cntRecNo <= glv_tag9F4D.Value[1]; cntRecNo++)
	{
		unp_rcvLen=0;
		cmdRead[2]=cntRecNo;							//P1: Record Number
		cmdRead[3]=(glv_tag9F4D.Value[0] << 3) | 0x04;	//P2: SFI | P1 is a record number

		rspCode=ECL_LV1_DEP(5, cmdRead, &unp_rcvLen, unp_rcvData, 1000);
		if (rspCode != ECL_LV1_SUCCESS) return FAIL;

//DBG_Put_Dialog(0, 5, cmdRead);
//DBG_Put_Dialog(1, unp_rcvLen, unp_rcvData);

		//Check SW12
		if ((unp_rcvData[unp_rcvLen-2] != 0x90) || (unp_rcvData[unp_rcvLen-1] != 0x00)) return FAIL;

		//Save Log
		ptrData=oda_bufRecord+ODA_RECORD_SIZE*(cntRecNo-1);
		ptrData[0]=(UCHAR)((unp_rcvLen-2) & 0xFF00) >> 8;
		ptrData[1]=(UCHAR)((unp_rcvLen-2) & 0x00FF);
		memcpy(&ptrData[2], unp_rcvData, (ptrData[0]*256+ptrData[1]));
	}

	return SUCCESS;
}


#ifdef _PLATFORM_AS350
void UNP_Parse_TransactionLog(void)
{
	UCHAR	rspKey=0xFF;
	UCHAR	dspRecNo=0;
	UCHAR	ascRecNo[10];
	UCHAR	ascTxnDate[10];
	UCHAR	ascTxnTime[8];
	UCHAR	ascAutAmount[13];
	UCHAR	ascOthAmount[13];
	UCHAR	ascCouCode[4];
	UCHAR	ascCurCode[4];
	UCHAR	ascMerName[20];
//	UCHAR	ascTxnType[2];
	UCHAR	ascTxnCounter[4];
	UCHAR	cntZero=0;
	UCHAR	cntRecNo=0;
	UCHAR	flgExit=FALSE;
	UCHAR	*ptrData=NULLPTR;
	
	
	//
	//	Parse Log
	//
	UT_ClearScreen();
	UT_WaitTime(50);
	UT_PutStr(6, 0, FONT0, 10, (UCHAR*)"Parse Log?");

	do
	{
		if (UT_GetKeyStatus() == apiReady)
	    {
			rspKey=UT_WaitKey();
			if (rspKey == 'y')
			{
				UT_ClearScreen();

				cntRecNo=1;
				dspRecNo=0;

				do
				{
					//Display Log
					if (cntRecNo != dspRecNo)
					{
						UT_ClearScreen();
						ptrData=oda_bufRecord+ODA_RECORD_SIZE*(cntRecNo-1)+2;	//Skip Len(2)

						UT_INT2ASC((ULONG)cntRecNo, ascRecNo);
						UT_PutStr(0, 0, FONT0, 6, (UCHAR*)"Rec.: ");
						UT_PutStr(0, 6, FONT0, 2, &ascRecNo[8]);

						UT_PutStr(2, 0, FONT0, 16, (UCHAR*)"Transaction Date");
						ascTxnDate[0]='2';
						ascTxnDate[1]='0';
						ascTxnDate[4]='-';
						ascTxnDate[7]='-';
						UT_Split(&ascTxnDate[2], &ptrData[0], 1);						
						UT_Split(&ascTxnDate[5], &ptrData[1], 1);
						UT_Split(&ascTxnDate[8], &ptrData[2], 1);						
						UT_PutStr(3, 0, FONT0, 10, ascTxnDate);
						ptrData+=3;
						
						UT_PutStr(4, 0, FONT0, 16, (UCHAR*)"Transaction Time");
						ascTxnTime[2]='-';
						ascTxnTime[5]='-';
						UT_Split(&ascTxnTime[0], &ptrData[0], 1);
						UT_Split(&ascTxnTime[3], &ptrData[1], 1);
						UT_Split(&ascTxnTime[6], &ptrData[2], 1);
						UT_PutStr(5, 0, FONT0, 8, ascTxnTime);
						ptrData+=3;
						
						UT_PutStr(6, 0, FONT0, 18, (UCHAR*)"Amount, Authorised");
						UT_Split(ascAutAmount, ptrData, 6);
						ascAutAmount[12]=ascAutAmount[11];
						ascAutAmount[11]=ascAutAmount[10];
						ascAutAmount[10]='.';

						//Exclude Leading Zeros
						for (cntZero=0; cntZero < 9; cntZero++)
						{
							if (ascAutAmount[cntZero] != '0') break;

							if (cntZero == 9) break;
						}
						UT_PutStr(7, 0, FONT0, (13-cntZero), &ascAutAmount[cntZero]);
						ptrData+=6;
						
						UT_PutStr(8, 0, FONT0, 13, (UCHAR*)"Amount, Other");
						UT_Split(ascOthAmount, ptrData, 6);
						ascOthAmount[12]=ascOthAmount[11];
						ascOthAmount[11]=ascOthAmount[10];
						ascOthAmount[10]='.';

						//Exclude Leading Zeros
						for (cntZero=0; cntZero < 9; cntZero++)
						{
							if (ascOthAmount[cntZero] != '0') break;

							if (cntZero == 9) break;
						}
						UT_PutStr(9, 0, FONT0, (13-cntZero), &ascOthAmount[cntZero]);
						ptrData+=6;
						
						UT_PutStr(10, 0, FONT0, 21, (UCHAR*)"Terminal Country Code");
						UT_Split(ascCouCode, ptrData, 2);
						UT_PutStr(11, 0, FONT0, 4, ascCouCode);
						ptrData+=2;
						
						UT_PutStr(12, 0, FONT0, 25, (UCHAR*)"Transaction Currency Code");
						UT_Split(ascCurCode, ptrData, 2);
						UT_PutStr(13, 0, FONT0, 4, ascCurCode);
						ptrData+=2;
						
						UT_PutStr(14, 0, FONT0, 26, (UCHAR*)"Merchant Name and Location");
						memcpy(ascMerName, ptrData, 20);
						UT_PutStr(15, 0, FONT0, 20, ascMerName);
						ptrData+=20;
						
						UT_PutStr(16, 0, FONT0, 16, (UCHAR*)"Transaction Type");
						//UT_Split(ascTxnType, ptrData, 1);
						//UT_PutStr(17, 0, FONT0, 2, ascTxnType);
						if (ptrData[0] == 0x00)
						{
							UT_PutStr(17, 0, FONT0, 8, (UCHAR*)"Purchase");
						}
						ptrData+=1;
						
						UT_PutStr(18, 0, FONT0, 30, (UCHAR*)"Application Transaction Counter");
						UT_Split(ascTxnCounter, ptrData, 2);
						UT_PutStr(19, 0, FONT0, 4, ascTxnCounter);
						ptrData+=2;
						
						dspRecNo=cntRecNo;
					}
					
					if (UT_GetKeyStatus() == apiReady)
				    {
						rspKey=UT_WaitKey();
						switch (rspKey)
						{
							case '#': cntRecNo=((cntRecNo - 1) != 0)?(cntRecNo-1):(glv_tag9F4D.Value[1]);	break;
							case '*': cntRecNo=((cntRecNo + 1) > glv_tag9F4D.Value[1])?(1):(cntRecNo+1);	break;
							case 'x': flgExit=TRUE;															break;
							default:																		break;
						}
					}
				} while (flgExit == FALSE);
			}
			else
			{
				if (rspKey == 'x') flgExit=TRUE;
			}
		}
	} while (flgExit == FALSE);
}
#endif
#endif


UCHAR UNP_Allocate_Buffer(void)
{
	unp_rcvData=malloc(ETP_BUFFER_SIZE);
	unp_lstData=malloc(UNP_LIST_BUFFER_SIZE);

	if ((unp_rcvData == NULLPTR) || (unp_lstData == NULLPTR))
	{
		return UNP_RESULT_TERMINATE;
	}

	return UNP_RESULT_SUCCESS;
}

void UNP_Free_Buffer(void)
{
	free(unp_rcvData);
	free(unp_lstData);
}

UCHAR UNP_Get_PAN(UCHAR *optLen, UCHAR *optData)
{
	UCHAR asc57[ECL_LENGTH_57*2]={0};
	UINT  lenOf57=0;
	UCHAR lenPAN=0;
	UCHAR numIndex=0;

	//qUICS: Extract PAN from Track 2 Equivalent Data
	UT_Get_TLVLengthOfV(glv_tag57.Length, &lenOf57);
	
	if ((lenOf57 == 0) || (lenOf57 > ECL_LENGTH_57)) return FAIL;

	UT_Split(asc57, glv_tag57.Value, (char)lenOf57);

	//Field Separator
	for (numIndex=0; numIndex < (ECL_LENGTH_57+1); numIndex++)	//PAN(19) + Separator(1)
	{
		if (asc57[numIndex] == 'D')
		{
			lenPAN=numIndex;
			break;
		}
	}

	//Separator Not Found
	if (numIndex == (ECL_LENGTH_57+1))
	{
		return FAIL;
	}

	if (lenPAN & 1)
	{
		asc57[lenPAN]='F';
		optLen[0]=(lenPAN+1)/2;
	}
	else
	{
		optLen[0]=lenPAN/2;
	}
	
	UT_Compress(optData, asc57, optLen[0]);

	return SUCCESS;
}


void UNP_Add_TagList(UCHAR *iptData)
{
	UCHAR rspCode=0;
	UCHAR lenOfT=0;

	rspCode=UT_Get_TLVLengthOfT(iptData, &lenOfT);
	if (rspCode == SUCCESS)
	{
		if ((unp_lstLen + lenOfT) <= UNP_LIST_BUFFER_SIZE)
		{
			memcpy(&unp_lstData[unp_lstLen], iptData, lenOfT);
			unp_lstLen+=lenOfT;
		}
	}
}


//
//	Copy of VGL_Verify_AFL
//
UCHAR UNP_Check_AFL( UCHAR *AFL_Data,UINT AFL_Length)
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


UCHAR UNP_Check_ExpirationDate(void)
{
	UINT	lenOfV=0;
	UCHAR	ascRTC[12];
	UCHAR	rtcBuffer[1+6];	
	UCHAR	expBuffer[1+3];
	int		cmpResult=0;
	
	UT_Get_TLVLengthOfV(glv_tag5F24.Length, &lenOfV);

	if (lenOfV != 0)
	{
		memcpy(&expBuffer[1], glv_tag5F24.Value, 3);
		expBuffer[0]=UT_SetCentury((signed char)expBuffer[1]);
		
		UT_GetDateTime(ascRTC);
		UT_Compress(&rtcBuffer[1], ascRTC, 6);
		rtcBuffer[0]=UT_SetCentury((signed char)rtcBuffer[1]);

		cmpResult=UT_CompareDate(rtcBuffer, expBuffer);
		if (cmpResult == 1)
		{
			unp_addMessage=UNP_MESSAGE_CARD_EXPIRED;
			
			UT_Get_TLVLengthOfV(glv_tag9F6C.Length, &lenOfV);
			if (lenOfV != 0)
			{
				if ((glv_tag9F6C.Value[0] & UNP_CTQ_IF_APPLICATION_HAS_EXPIRED_THEN_ONLINE_TRANSACTION) &&
					((glv_tag9F66.Value[0] & UNP_TTQ_OFFLINE_TERMINAL) == 0x00))
				{
					return UNP_RESULT_ONLINE_REQUEST;
				}
			}			

			return UNP_RESULT_DECLINE;
		}
	}

	return UNP_RESULT_SUCCESS;
}


UCHAR UNP_Check_TagList(UCHAR *iptData, UINT iptLstLen, UCHAR*iptLstTag)
{
	UCHAR rspCode=0;
	UCHAR lenOfT=0;
	UCHAR lenOfT_List=0;
	UCHAR *ptrData=NULLPTR;
	UINT  curLen=0;

	if (iptLstLen == 0) return SUCCESS;
	
	rspCode=UT_Get_TLVLengthOfT(iptData, &lenOfT);
	if (rspCode == SUCCESS)
	{
		ptrData=iptLstTag;
		
		do
		{
			rspCode=UT_Get_TLVLengthOfT(ptrData, &lenOfT_List);
			
			if ((lenOfT == lenOfT_List) &&
				(!memcmp(iptData, ptrData, lenOfT_List)))
			{
				return FAIL;
			}

			ptrData+=lenOfT_List;
			curLen+=lenOfT_List;
		} while (curLen < iptLstLen);

		return SUCCESS;
	}
	
	return FAIL;
}


UCHAR UNP_Check_TerminalTag(UCHAR *iptData)
{
	UCHAR rspCode;
	UCHAR lstTerTag[6]=
	{
		0x9F,0x37,
		0x9F,0x02,
		0x5F,0x2A,
	};

	rspCode=UNP_Check_TagList(iptData, 6, lstTerTag);
		
	return rspCode;
}


//
//	Modify Function of VGL_Check_RR_R_Tag
//
UCHAR UNP_Copy_ReadRecordResponseData(UCHAR SFINUMBER,UINT RRR_Len,UCHAR *RRR_Buf)
{
	UCHAR redundant=0;
	UCHAR TLength,LLength,tmplen=0,*len_addr=0,rspCode;//,*ptrrec,tmptag;
	UINT tmp_tag_len,VLength;
	//UINT iTempLen;
	ULONG Index;
	UINT  lenOfV_Table=0;

	//ICTK check Read Record Wrong Format		
	//Check the Format First
	UT_Get_TLVLength(RRR_Buf, &TLength, &LLength, &VLength);
	
DBG_Put_Text("2.1.4.1. Check Length");
	
	if(!((RRR_Buf[TLength+LLength+VLength] == 0x90) && (RRR_Buf[TLength+LLength+VLength+1] == 0x00)))
		return UNP_RESULT_TERMINATE;

DBG_Put_Text("2.1.4.2. Check SFI");

	if(RRR_Buf[0] != 0x70)
	{
		//ICTK 0627 test case CLQ.N.076
		if(SFINUMBER<11)
		{
			return UNP_RESULT_TERMINATE;
		}
	}
	//ICTK check Read Record Wrong Format	end

DBG_Put_Text("2.1.4.3. Copy");
	
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

DBG_Put_Hex((UINT)TLength, RRR_Buf);

			rspCode = UT_Search(TLength,RRR_Buf,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);	//search glv_tagTable and get the index	
			if(rspCode == SUCCESS)
			{
				if ((RRR_Buf[0] == 0x5F) && (RRR_Buf[1] == 0x34) &&
					(TLength == 2))
				{
					//qUICS R-CLQZ10501 Don't Check 5F34
					rspCode=SUCCESS;
				}
				else
				{
					rspCode=UNP_Check_TagList(RRR_Buf, unp_lstLen, unp_lstData);
				}
				
				if (rspCode == FAIL)
				{	

DBG_Put_Text("2.1.4.4. Redundant");

					redundant = 0x01;
					break;
				}
				//copy "Read Record Response" to TAG table
				else
				{
					rspCode=UNP_Check_TerminalTag(RRR_Buf);
					if (rspCode == SUCCESS)
					{
						UNP_Add_TagList(RRR_Buf);
						
						tmplen += TLength;
						RRR_Buf += TLength;			//shift buf pointer, and now we are in "length" field

						UT_Get_TLVLengthOfL(RRR_Buf,&LLength);	//get length's Length
						tmplen += LLength;
						UT_Get_TLVLengthOfV(RRR_Buf,&tmp_tag_len);
						tmplen += tmp_tag_len;
					
						len_addr = (UCHAR *)(glv_addTable[Index]);	//point to glv_addtable length address

						lenOfV_Table=glv_tagTable[Index].UPI_Length[0]*256+glv_tagTable[Index].UPI_Length[1];
						if (tmp_tag_len <= lenOfV_Table)
						{
							//Save Card Data
							UT_Set_TagLength(tmp_tag_len, len_addr);
							memcpy((len_addr+3), &RRR_Buf[LLength], tmp_tag_len);
							RRR_Buf+=(LLength+tmp_tag_len);
						}
					}
					else
					{
						//Skip to Next Tag
						tmplen += TLength;
						RRR_Buf += TLength;

						UT_Get_TLVLengthOfL(RRR_Buf,&LLength);
						UT_Get_TLVLengthOfV(RRR_Buf,&VLength);
						tmplen += LLength;
						tmplen += VLength;

						RRR_Buf += LLength;
						RRR_Buf += VLength;
					}
				}
			}
			else			//can't recognize, ignore it
			{
				//UT_ClearScreen();
				//UT_DumpHex(0,2,TLength,RRR_Buf);
				//UT_PutStr(1, 0, FONT0, 16, (UCHAR *)"CAN NOT FIND TAG");
				//UT_WaitKey();

				//Skip to Next Tag
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
		return UNP_RESULT_TERMINATE;
	}	
	else
		return UNP_RESULT_SUCCESS;
}


//
//	Modify Function of VGL_Check_GPO_R_Data
//
UCHAR UNP_Copy_GPOResponseData(UINT iptLen, UCHAR *iptData)
{
	UCHAR TLength,LLength=0,*len_addr=0,rspCode,redundant=0;
	ULONG Index;
	UINT  tmplen=0,VLength=0;
	UINT  lenOfV_Table=0;

DBG_Put_Text("1.3.1. Copy GPO");

	while(tmplen != iptLen)
	{
		TLength = LLength = VLength = 0;
		Index = 0;
		
		//Get first byte tag to check
		//is tag a Primitive data element ?
		if((*iptData & 0x20) && (*iptData != 0xFF))		//it's a Constructed data element
		{	
			UT_Get_TLVLength(iptData, &TLength, &LLength, &VLength);
			
			//ICTK check GPO Length
			if(!((iptData[TLength+LLength+VLength] == 0x90) && (iptData[TLength+LLength+VLength+1] == 0x00)))
			{
				return FAIL;
			}
			
			tmplen += TLength;	
			iptData += TLength;			
		
			tmplen += LLength;
			iptData += LLength;			//shift buf pointer, and now we are in "Constructed data" field
		}
		else if((*iptData == 0x00)||(*iptData == 0xFF))			//ICTK, padding with 0x00
		{
			tmplen += 1;	
			iptData += 1;
		}
		else				//this tag is a Primitive data element
		{
			UT_Get_TLVLengthOfT(iptData,&TLength);	//get tag's Length

DBG_Put_Hex(TLength, iptData);

			rspCode = UT_Search(TLength,iptData,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);	//search glv_tagTable and get the index 
			if(rspCode == SUCCESS)
			{				
				len_addr = (UCHAR *)(glv_addTable[Index]);	//point to glv_addtable length address
				
				rspCode=UNP_Check_TagList(iptData, unp_lstLen, unp_lstData);
				if (rspCode == FAIL)
				{					
					redundant = 0x01;
					break;
				}
				else
				{
					rspCode=UNP_Check_TerminalTag(iptData);
					if (rspCode == SUCCESS)
					{
						UNP_Add_TagList(iptData);
						
						tmplen += TLength;
						iptData += TLength;			//shift buf pointer, and now we are in "length" field

						UT_Get_TLVLengthOfL(iptData,&LLength);				//get length's Length
						UT_Get_TLVLengthOfV(iptData,&VLength);				//get Value's Length
						tmplen += LLength;
						tmplen += VLength;

						lenOfV_Table=glv_tagTable[Index].UPI_Length[0]*256+glv_tagTable[Index].UPI_Length[1];
						if (VLength <= lenOfV_Table)
						{
							//Save Card Data
							UT_Set_TagLength(VLength, len_addr);
							memcpy((len_addr+3), &iptData[LLength], VLength);
							iptData+=(LLength+VLength);
						}
					}
					else
					{
						//Skip to Next Tag
						tmplen += TLength;
						iptData += TLength;						

						UT_Get_TLVLengthOfL(iptData,&LLength);
						UT_Get_TLVLengthOfV(iptData,&VLength);
						tmplen += LLength;
						tmplen += VLength;

						iptData += LLength;
						iptData += VLength;
					}
				}
			}
			else
			{
				//Skip to Next Tag
				//20131210	CLM.S.002.00
				tmplen += TLength;
				iptData += TLength;			//shift buf pointer, and now we are in "length" field

				UT_Get_TLVLengthOfL(iptData,&LLength);				//get length's Length
				UT_Get_TLVLengthOfV(iptData,&VLength);				//get Value's Length
				tmplen += LLength;
				tmplen += VLength;

				iptData += LLength;
				iptData += VLength;
				//20131210 end
				//UT_PutStr(1, 0, FONT0, 17, (UCHAR *)"GPO CANT FIND TAG");
				//UT_WaitKey();
			}
		}		
	}

	if(redundant == 0x01)
	{

DBG_Put_Text("1.3.2. Redundant");

		return FAIL;
	}
	else 
	{
		return SUCCESS;
	}
}


//
//	Modify Function of VGL_AIP_AFL_Process
//
UCHAR UNP_Check_GPOResponseFormat(UINT iptLen, UCHAR *iptData)
{
	//Application Interchang Prefile 
	UCHAR *data_point = 0;
	UCHAR AIP[2] = {0};
	UCHAR cnt,AFL_Data[252]={0};
	UINT Value_Len=0,AFL_Length=0;
	UCHAR tag82[1]={0x82};
	UCHAR tag94[1]={0x94};
	UCHAR lenOfT=0;

DBG_Put_Text("1.2.1. Chk GPO");

    UT_EMVCL_GetBERLEN( &iptData[1], &cnt ,&Value_Len); // get total length of data elements
	// check legal length
    if( Value_Len < 6 )
         return FAIL;

	// check response format (2)
	if( (iptData[0] == 0x77))
    {

DBG_Put_Text("1.2.2. Fm2");

		// find AIP[2]
		data_point=UT_Find_Tag(tag82, iptLen, iptData);
	    if( data_point != NULLPTR )
	    {

DBG_Put_Text("1.2.3. AIP Present");

			UT_Get_TLVLengthOfT(data_point, &lenOfT);
			data_point+=lenOfT;	//Point to L
        	UT_EMVCL_GetBERLEN( data_point, &cnt ,&Value_Len);
            if(Value_Len==0)
            	return FAIL; // AIP not present

DBG_Put_Text("1.2.4. Chk AIP = 0");

			//ICTK, AIP length = 3, it should be terminated
			if(Value_Len != 2)
				return FAIL;

DBG_Put_Text("1.2.5. Chk AIP = 2");

            // save AIP[2]
            AIP[0] = *(data_point+cnt);
            AIP[1] = *(data_point+cnt+1);
			//*glv_tag82.Length = 0x02;
           	//memcpy(glv_tag82.Value,AIP,2);
		}
        else
        {

DBG_Put_Text("1.2.6. AIP Missing");

            return FAIL; // AIP not present
        }

        // find AFL[4n]
        data_point = UT_Find_Tag(tag94, iptLen, iptData);
        if( data_point != NULLPTR )
        {

DBG_Put_Text("1.2.7. AFL Present");

			UT_Get_TLVLengthOfT(data_point, &lenOfT);
			data_point+=lenOfT;	//Point to L
        	UT_EMVCL_GetBERLEN( data_point, &cnt ,&Value_Len);

            // save AFL[4n]
            if((Value_Len % 4)==0)
            {
            	AFL_Length = Value_Len;
				memcpy(AFL_Data,&data_point[cnt],AFL_Length);
            }
			else
			{
				
DBG_Put_Text("1.2.8. Not a Multiplier of 4");

				return FAIL;
			}
        }
		//else
		//	return FAIL; // AFL not present

		
        // PATCH: 2003-06-12 JCB SPEC (also good for EMV SPEC)
        //        To verify AFL before actual reading records.
        if( UNP_Check_AFL(AFL_Data,AFL_Length) == FALSE )
        {

DBG_Put_Text("1.2.9. Chk AFL Fail");

            return FAIL;
        }

    }
	else
	{

DBG_Put_Text("1.2.10. Fmt Error");

		return FAIL;
	}

	return SUCCESS;
}


UCHAR UNP_Check_GPOMandatoryTags(void)
{
	UINT lenOfV;

	lenOfV=0;
	UT_Get_TLVLengthOfV(glv_tag9F10.Length, &lenOfV);
	if (lenOfV != 0)
	{
		lenOfV=0;
		UT_Get_TLVLengthOfV(glv_tag9F27.Length, &lenOfV);
		if (lenOfV == 0)
		{
			//Form CID from 9F10 B5b6-5
			glv_tag9F27.Value[0]=(glv_tag9F10.Value[4] & 0x30) << 2;
		}

		if ((glv_tag9F27.Value[0] & 0xC0) == UNP_CID_TC)
		{
			lenOfV=0;
			UT_Get_TLVLengthOfV(glv_tag82.Length, &lenOfV);
			if (lenOfV == 0) return UNP_RESULT_TERMINATE;

			lenOfV=0;
			UT_Get_TLVLengthOfV(glv_tag94.Length, &lenOfV);
			if (lenOfV == 0) return UNP_RESULT_TERMINATE;

			lenOfV=0;
			UT_Get_TLVLengthOfV(glv_tag9F36.Length, &lenOfV);
			if (lenOfV == 0) return UNP_RESULT_TERMINATE;

			lenOfV=0;
			UT_Get_TLVLengthOfV(glv_tag9F26.Length, &lenOfV);
			if (lenOfV == 0) return UNP_RESULT_TERMINATE;

			lenOfV=0;
			UT_Get_TLVLengthOfV(glv_tag9F10.Length, &lenOfV);
			if (lenOfV == 0) return UNP_RESULT_TERMINATE;
		}
		else if (((glv_tag9F27.Value[0] & 0xC0) == UNP_CID_ARQC) ||
				((glv_tag9F27.Value[0] & 0xC0) == UNP_CID_AAC))
		{
			lenOfV=0;
			UT_Get_TLVLengthOfV(glv_tag82.Length, &lenOfV);
			if (lenOfV == 0) return UNP_RESULT_TERMINATE;

			lenOfV=0;
			UT_Get_TLVLengthOfV(glv_tag9F36.Length, &lenOfV);
			if (lenOfV == 0) return UNP_RESULT_TERMINATE;

			lenOfV=0;
			UT_Get_TLVLengthOfV(glv_tag57.Length, &lenOfV);
			if (lenOfV == 0) return UNP_RESULT_TERMINATE;

			lenOfV=0;
			UT_Get_TLVLengthOfV(glv_tag9F10.Length, &lenOfV);
			if (lenOfV == 0) return UNP_RESULT_TERMINATE;
			
			lenOfV=0;
			UT_Get_TLVLengthOfV(glv_tag9F26.Length, &lenOfV);
			if (lenOfV == 0) return UNP_RESULT_TERMINATE;
		}
		else
		{
			return UNP_RESULT_DECLINE;
		}
	}
	else
	{
		return UNP_RESULT_TERMINATE;
	}

	return UNP_RESULT_SUCCESS;
}


UCHAR UNP_Check_CVMRequest(void)
{
	UINT lenOfV=0;

	UT_Get_TLVLengthOfV(glv_tag9F6C.Length, &lenOfV);

	if (lenOfV != 0)
	{

DBG_Put_Text("2.2.1. 9F6C Present");

		if ((glv_tag9F6C.Value[0] & UNP_CTQ_REQUIRES_ONLINE_PIN) &&
			(glv_tag9F66.Value[0] & UNP_TTQ_SUPPORT_ONLINE_PIN))
		{

DBG_Put_Text("2.2.2. Request Online PIN");
	
			etp_Outcome.CVM=ETP_OCP_CVM_OnlinePIN;

			return UNP_RESULT_SUCCESS;
		}
		else
		{

DBG_Put_Text("2.2.3. Online PIN Not Match");

			if (((glv_tag9F6C.Value[0] & UNP_CTQ_REQUIRES_ONLINE_PIN) == 0) ||
				((glv_tag9F66.Value[0] & UNP_TTQ_SUPPORT_ONLINE_PIN) == 0))
			{
				if (glv_tag9F6C.Value[1] & UNP_CTQ_CDCVM_PERFORMED)
				{

DBG_Put_Text("2.2.4. CDCVM Performed");

					lenOfV=0;
					UT_Get_TLVLengthOfV(glv_tag9F69.Length, &lenOfV);

					if (lenOfV != 0)
					{

DBG_Put_Text("2.2.5. 9F69 Present");

						if (!memcmp(&glv_tag9F69.Value[5], &glv_tag9F6C.Value[0], 2))
						{

DBG_Put_Text("2.2.6. 9F69 Match 9F6C");

							return UNP_RESULT_SUCCESS;	//CVM is Complete
						}
						else
						{
							;	//Decline
						}
					}
					else
					{

DBG_Put_Text("2.2.7. 9F69 Absent");

						if ((glv_tag9F27.Value[0] & 0xC0) == UNP_CID_ARQC)
						{

DBG_Put_Text("2.2.8. 9F27 Request ARQC");

							return UNP_RESULT_SUCCESS;	//CVM is Complete
						}
						else
						{
							;	//Decline
						}
					}
				}
				else
				{

DBG_Put_Text("2.2.9. CDCVM Not Performed");

					if ((glv_tag9F6C.Value[0] & UNP_CTQ_REQUIRES_ONLINE_PIN) == 0)
					{
						if (glv_tag9F6C.Value[0] & UNP_CTQ_REQUIRES_SIGNATURE)
						{
							if (glv_tag9F66.Value[0] & UNP_TTQ_SUPPORT_SIGNATURE)
							{

DBG_Put_Text("2.2.10. Request Signature");

								etp_Outcome.CVM=ETP_OCP_CVM_ObtainSignature;

								return UNP_RESULT_SUCCESS;
							}
						}
						else
						{

DBG_Put_Text("2.2.11. CVM not Set in CTQ");

							if ((glv_tag9F66.Value[1] & UNP_TTQ_REQUIRES_CVM) == 0)
							{

DBG_Put_Text("2.2.12. Under CVM Limit");

								return UNP_RESULT_SUCCESS;
							}
						}
					}
				}
			}
		}
	}
	else
	{

DBG_Put_Text("2.2.13. 9F6C Absent");

		if (glv_tag9F66.Value[1] & UNP_TTQ_REQUIRES_CVM)
		{

DBG_Put_Text("2.2.14. TTQ Requires CVM");

			if (glv_tag9F66.Value[0] & UNP_TTQ_SUPPORT_SIGNATURE)
			{

DBG_Put_Text("2.2.15. Request Signature");

				etp_Outcome.CVM=ETP_OCP_CVM_ObtainSignature;

				return UNP_RESULT_SUCCESS;
			}
			else
			{
				if (glv_tag9F66.Value[0] & UNP_TTQ_SUPPORT_ONLINE_PIN)
				{

DBG_Put_Text("2.2.16. Request Online PIN");

					etp_Outcome.CVM=ETP_OCP_CVM_OnlinePIN;

					return UNP_RESULT_SUCCESS;
				}
				else
				{
					;	//Decline
				}
			}
		}
		else
		{

DBG_Put_Text("2.2.17. Under CVM Limit");

			return UNP_RESULT_SUCCESS;
		}
	}


	return UNP_RESULT_DECLINE;
}


UCHAR UNP_Check_CTQDecision(void)
{
	UCHAR	rspCode=UNP_RESULT_SUCCESS;
	UINT	lenOfV;
	
	UT_Get_TLVLengthOfV(glv_tag9F6C.Length, &lenOfV);
	if (lenOfV != 0)
	{
		if ((glv_tag9F6C.Value[0] & UNP_CTQ_IF_ODA_FAILS_AND_TERMINAL_CAN_GO_ONLINE_THEN_REQUEST_ONLINE) &&
			((glv_tag9F66.Value[0] & UNP_TTQ_OFFLINE_TERMINAL) == 0x00))
		{
			rspCode=UNP_RESULT_ONLINE_REQUEST;
		}
		else
		{
			if ((glv_tag9F6C.Value[0] & UNP_CTQ_IF_ODA_FAILS_AND_TERMINAL_SUPPORTS_STANDARD_DEBIT_CREDIT_FLOW_THEN_TERMINATE) &&
				(glv_tag9F66.Value[0] & UNP_TTQ_SUPPORT_CONTACT_DEBIT_CREDIT_APPLICATION))
			{
				unp_addMessage=UNP_MESSAGE_TERMINATE;
				rspCode=UNP_RESULT_TRY_ANOTHER_INTERFACE;
			}
			else
			{
				rspCode=UNP_RESULT_DECLINE;
			}
		}
	}
	else
	{
		rspCode=UNP_RESULT_DECLINE;
	}

	return rspCode;
}


UCHAR UNP_Check_ExceptionFile(void)
{
	UCHAR i=0;
	UCHAR j=0;
	UCHAR ascExpFile[ECL_LENGTH_5A*2];
	UCHAR ascPAN[ECL_LENGTH_5A*2];
	UCHAR lenOfAscPAN=0;
	UINT  lenOf5F34;
	UINT  lenOf5A;

	UT_Get_TLVLengthOfV(glv_tag5F34.Length, &lenOf5F34);
	UT_Get_TLVLengthOfV(glv_tag5A.Length, &lenOf5A);
	
	if ((lenOf5A == 0) || (lenOf5F34 == 0))
	{
		;	//Don't Check
	}
	else
	{
		memset(ascPAN, 'F', (ECL_LENGTH_5A*2));
		UT_Split(ascPAN, glv_tag5A.Value, lenOf5A);

		lenOfAscPAN=(ascPAN[lenOf5A*2-1] == 'F')?(lenOf5A*2-1):(lenOf5A*2);
		
		for (i=0; i < 10; i++)	//Number of Exception File
		{				
			UT_Split(ascExpFile, Exception_File[i], 10);

			for (j=0; j < (ECL_LENGTH_5A*2); j++)	//Length of PAN in Exception File
			{
				if (ascExpFile[j] == 'F')
				{
					break;			
				}
			}

			if ((!memcmp(ascPAN, ascExpFile, j)) && (lenOfAscPAN == j))
			{
				if ((lenOf5F34 == ECL_LENGTH_5F34) && 
					(glv_tag5F34.Value[0] == Exception_File[i][10]))
				{
					return FAIL;
				}
			}
		}
	}

	return SUCCESS;
}


//
//	Modify of VGL_RAD_SAD_Data
//
void UNP_Save_DDARecord(UCHAR SFINUM,UINT RECNUM,UINT RRR_Len,UCHAR *RRR_Buf)
{
  	memcpy((oda_bufRecord+ODA_RECORD_SIZE*(RECNUM)),&SFINUM,1);
  	memcpy((oda_bufRecord+ODA_RECORD_SIZE*(RECNUM))+1,RRR_Buf,RRR_Len-2);	//minus SW1,SW2
}


//
//	Modify of VGL_Chip_Data
//
UCHAR UNP_Chip_Data(UCHAR Tag_Total_Len,UCHAR *taglist)
{
	UINT i = 0,tmpLEN = 0,VLength=0;
	UCHAR TLength = 0, LLength = 0, rspCode = 0,*Len_Addr = 0, *Value_Addr = 0;
	ULONG Index=0;
	

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

	//Online Request
	Online_Data[i++]=0xDF;
	Online_Data[i++]=0x0F;
	Online_Data[i++]=0x01;
	Online_Data[i++]=0x07;
/*
	//Offline Approve
	Online_Data[i++]=0xDF;
	Online_Data[i++]=0x1F;
	Online_Data[i++]=0x01;
	Online_Data[i++]=0x00;

	//Decline
	Online_Data[i++]=0xDF;
	Online_Data[i++]=0x1F;
	Online_Data[i++]=0x01;
	Online_Data[i++]=0x01;
*/	
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
			return FAIL;
		}
		
		taglist += TLength;
		Tag_Total_Len -= TLength;
	}

	Online_Data_Length = i;
	
	return SUCCESS;

}


//
//	Copy of VGL_qVSDC_Online_Data
//
UCHAR UNP_Patch_OnlineData(void)
{
	UCHAR qUICS_Data[50] = {0x5A,0x5F,0x20,0x5F,0x24,0x5F,0x2A,0x5F,0x34,0x82,
							0x95,0x9A,0x9C,0x9F,0x10,0x9F,0x1A,0x9F,0x26,0x9F,
							0x36,0x9F,0x37,0x9F,0x6E,0x9F,0x7C,0x9F,0x27,0x57,
							0x9F,0x66,0x9F,0x02,0x9F,0x03,0x9F,0x5D,0x9F,0x63,
							0x9F,0x06,0x9F,0x21,0x9F,0x4E,0x9F,0x24};
	
	UCHAR rspCode = 0;

	rspCode = UNP_Chip_Data(48, qUICS_Data);

	if(rspCode == FAIL)
	{
		UT_PutMsg(1,0,FONT0,17,(UCHAR *)"Online Data Error");
		UT_WaitKey();
	}
	
	return SUCCESS;
	
}


UCHAR UNP_ApplicationInitialization(void)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UCHAR	lenOfDolData=0;
	UCHAR	rspCode=FAIL;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	tag9F38[2]={0x9F,0x38};
	UCHAR	tag9F66[2]={0x9F,0x66};
	UCHAR	tagDF61[2]={0xDF,0x61};
	UINT	sndLen=0;
	UCHAR	sndData[512];

DBG_Put_Text("1. APP Initial");

	//Find Card Additional Function Indicator
	ptrData=UT_Find_Tag(tagDF61, (unp_rcvLen-2), unp_rcvData);
	if (ptrData != NULLPTR)
	{
		rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
		if (rspCode == SUCCESS)
		{
			unp_flgNewCard=(ptrData[lenOfT+lenOfL] & UNP_CAFI_ODA_FOR_ONLINE_AUTHORIZATION_SUPPORTED)?(TRUE):(FALSE);
		}
	}

	//Find PDOL
	ptrData=UT_Find_Tag(tag9F38, (unp_rcvLen-2), unp_rcvData);
	if (ptrData == NULLPTR)
	{
		return UNP_RESULT_SELECT_NEXT;
	}
	
	rspCode=UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
	UT_Set_TagLength(lenOfV, glv_tag9F38.Length);
	memcpy(glv_tag9F38.Value, &ptrData[lenOfT+lenOfL], lenOfV);
	
	//Find TTQ
	ptrData=UT_Find_TagInDOL(tag9F66, lenOfV, glv_tag9F38.Value);
	if (ptrData == NULLPTR)
	{
		return UNP_RESULT_SELECT_NEXT;
	}

	//Get PDOL Related Data
	rspCode=DOL_Get_DOLData(lenOfV, glv_tag9F38.Value, &lenOfDolData, sndData);
	sndLen=(UINT)lenOfDolData;

	if (rspCode == FAIL)
	{
		return UNP_RESULT_TERMINATE;
	}

	//Send GPO
	rspCode=ECL_APDU_Get_ProcessingOptions(sndLen, sndData, &unp_rcvLen, unp_rcvData);

//DBG_Put_Dialog(0, sndLen, sndData);
//DBG_Put_Dialog(1, unp_rcvLen, unp_rcvData);

	if (rspCode != ECL_LV1_SUCCESS)
	{
		if (rspCode == ECL_LV1_STOP_ICCARD)
		{
			return UNP_RESULT_INTERRUPT_ICC;
		}
		else if (rspCode == ECL_LV1_STOP_MAGSTRIPE)
		{
			return UNP_RESULT_INTERRUPT_MAG_STRIPE;
		}

		return UNP_RESULT_TRY_AGAIN;
	}

DBG_Put_Text("1.1. Check SW");

	//Check SW12 = 9000
	rspCode=UT_Check_SW12(&unp_rcvData[unp_rcvLen-2], STATUSWORD_9000);
	if (rspCode == FALSE)
	{
		rspCode=UT_Check_SW12(&unp_rcvData[unp_rcvLen-2], STATUSWORD_6986);
		if (rspCode == TRUE)
		{
			return UNP_RESULT_PERFORM_CDCVM;
		}
		else
		{
			if (glv_tag9F66.Value[0] & UNP_TTQ_SUPPORT_CONTACT_DEBIT_CREDIT_APPLICATION)
			{
				unp_addMessage=UNP_MESSAGE_TERMINATE;
				return UNP_RESULT_TRY_ANOTHER_INTERFACE;
			}
			else
			{
				return UNP_RESULT_TERMINATE;
			}
		}
	}

DBG_Put_Text("1.2. Check GPO Response");

	//Check GPO Response Format
	rspCode=UNP_Check_GPOResponseFormat((unp_rcvLen-2), unp_rcvData);
	if (rspCode == FAIL)
	{
		return UNP_RESULT_TERMINATE;
	}

DBG_Put_Text("1.3. Copy GPO Response");

	//Copy Data
	rspCode=UNP_Copy_GPOResponseData((unp_rcvLen-2), unp_rcvData);
	if (rspCode == FAIL)
	{
		return UNP_RESULT_TERMINATE;
	}

DBG_Put_Text("1.4. Check GPO Tag");

	//Check Mandatory Tags
	rspCode=UNP_Check_GPOMandatoryTags();
	if (rspCode != UNP_RESULT_SUCCESS)
	{
		return rspCode;
	}

DBG_Put_Text("1.5. Check CID");

	//Check Cryptogram Information Data
	if ((glv_tag9F27.Value[0] & 0xC0) == UNP_CID_AAC)
	{
		return UNP_RESULT_DECLINE;
	}
	
	return UNP_RESULT_SUCCESS;
}


UCHAR UNP_ReadApplicationData(void)
{
	UCHAR SFINUM=0,rspCode,AFL_B4=0,j,temcnt;
	UINT tag94_len=0,i,RECNUM=0;

DBG_Put_Text("2. Read Record");

	ODA_Clear_Record();

	UT_EMVCL_GetBERLEN(glv_tag94.Length,&temcnt,&tag94_len);

DBG_Put_Text("2.1. Read");

	for (i=0; i<tag94_len; i+=4)
	{
		SFINUM = glv_tag94.Value[i]>>3;
		AFL_B4 = glv_tag94.Value[i+3];
						
		for (j=glv_tag94.Value[i+1]; j<=glv_tag94.Value[i+2]; j++)
		{
			//reset the Read Record Buff
			unp_rcvLen=0;
			memset(unp_rcvData, 0, ETP_BUFFER_SIZE);

DBG_Put_Text("2.1.1. RecNo/SFI");
UCHAR msgRec[2]={0};
msgRec[0]=j;
msgRec[1]=SFINUM;
DBG_Put_Hex(2, msgRec);


			//send RR Command and Receive
			rspCode=ECL_APDU_Read_Record(j, SFINUM, &unp_rcvLen, unp_rcvData);
			if (rspCode == ECL_LV1_SUCCESS)
			{

DBG_Put_Text("2.1.2. Read Success");
//DBG_Put_Hex(unp_rcvLen, unp_rcvData);

DBG_Put_Text("2.1.3. Check SW");

				rspCode=UT_Check_SW12(&unp_rcvData[unp_rcvLen-2], STATUSWORD_9000);
				if (rspCode != TRUE)
				{
					return UNP_RESULT_TERMINATE;
				}

DBG_Put_Text("2.1.4. Copy Record");

				rspCode=UNP_Copy_ReadRecordResponseData(SFINUM, unp_rcvLen, unp_rcvData);
				if (rspCode != UNP_RESULT_SUCCESS)
				{
					return rspCode;
				}

DBG_Put_Text("2.1.5. Check Expiration Date");

				rspCode=UNP_Check_ExpirationDate();
				if (rspCode != UNP_RESULT_SUCCESS)
				{
					return rspCode;
				}
				
DBG_Put_Text("2.1.6. Check Exception File");

				rspCode=UNP_Check_ExceptionFile();
				if (rspCode == FAIL)
				{
					unp_addMessage=UNP_MESSAGE_EXCEPTION_FILE;
					return UNP_RESULT_DECLINE;
				}
				
				if (AFL_B4 != 0)
				{

DBG_Put_Text("2.1.7. Save DDA Rec");

					UNP_Save_DDARecord(SFINUM, RECNUM, unp_rcvLen, unp_rcvData);

					AFL_B4--;
					RECNUM++;
				}
			}
			else
			{
				if (rspCode == ECL_LV1_STOP_ICCARD)
				{
					return UNP_RESULT_INTERRUPT_ICC;
				}
				else if (rspCode == ECL_LV1_STOP_MAGSTRIPE)
				{
					return UNP_RESULT_INTERRUPT_MAG_STRIPE;
				}

DBG_Put_Text("2.1.8. Read Fail");

				return UNP_RESULT_TRY_AGAIN;
			}
		}
	}

DBG_Put_Text("2.2. CVM Request");

	//CVM Request
	rspCode=UNP_Check_CVMRequest();
	if (rspCode == UNP_RESULT_DECLINE)
	{
		return UNP_RESULT_DECLINE;
	}

DBG_Put_Text("2.3. Check CID");

	//Check CID
	if ((glv_tag9F27.Value[0] & 0xC0) == UNP_CID_ARQC)
	{
		return UNP_RESULT_ONLINE_REQUEST;
	}
				
	return UNP_RESULT_SUCCESS;
}


//
//	Modify of VGL_OFDA_FDDA
//
UCHAR UNP_OfflineDataAuthentication(void)
{
	UINT i=0,j=0,iTagListLen=0;
	UINT Re_len=0,iPKcLen=0,iModLen=0,iLeftMostLen=0,iHashLen=0,Tmp_Len1=0,Tag70_Tmp_Len=0;
	UCHAR Index_not_found = 0;
	UCHAR CAPKI[1]={0};		
	UCHAR ISSPK[250] = {0};
	UCHAR pkm[1024]={0};
	UCHAR ICCPK[250] = {0};	//Retrieval of the ICC Public Key					tag9f46
	UCHAR pkc[250] = {0};	//Retrieval of the ICC Public Key
	UCHAR tag70_Data[ODA_RECORD_SIZE]={0};
	UCHAR temp[20]={0};
	UCHAR Tag_Len=0,*ptrlist=0;
	UCHAR rspCode=0;
	ULONG Index={0};
	UCHAR tmpRevList[9]={0};
	UCHAR verFDDA=0x01;
	UINT  lenOf9F08=0;
	UINT  lenOf9F69=0;
	UINT  valOf9F08=0;
	UCHAR g_ibuf[1500];
	UCHAR g_obuf[1500];


DBG_Put_Text("3. ODA");

DBG_Put_Text("3.1. Check DDA Support");

	memset(g_ibuf,0x00,sizeof(g_ibuf));


	//AIP byte 1 bits6 is 0b?	DDA support? 1 for yes , 0 for none
	rspCode = UT_Get_TLVLengthOfV(glv_tag82.Length,&Tmp_Len1);
	if(Tmp_Len1 != 0)
	{
		if(!(glv_tag82.Value[0] & 0x20))
			return FAIL;
	}

DBG_Put_Text("3.2. Check DDA Version");

	//qUICS Check DDA Version Process
	rspCode = UT_Get_TLVLengthOfV(glv_tag9F69.Length, &lenOf9F69);

	if(lenOf9F69 != 0)
	{
		if ((lenOf9F69 < 8) || (lenOf9F69 > 16)) return FAIL;

		if (glv_tag9F69.Value[0] > 0x01) return FAIL;

		verFDDA=glv_tag9F69.Value[0];
	}
	else
	{
		verFDDA=0x00;
	}

	rspCode = UT_Get_TLVLengthOfV(glv_tag9F08.Length, &lenOf9F08);

	valOf9F08=(lenOf9F08 != 0)?(glv_tag9F08.Value[0]*256+glv_tag9F08.Value[1]):(valOf9F08=0x0020);

	if (valOf9F08 >= 0x0030)
	{
		if (verFDDA == 0x00) return FAIL;
	}
	else
	{
		if (verFDDA == 0x01) return FAIL;
	}

DBG_Put_Text("3.3. Retrieve CAPK");

//EMV 4.3 Book 2 Section 6.2-Retrieve the Certification Authority Public Key Index		tag8f
	memcpy(CAPKI,glv_tag8F.Value,1);
	
	for(i=0;i<CAPK_NUMBER;i++)
	{
		if((glv_CAPK[i].Index == CAPKI[0]) && (!memcmp(glv_CAPK[i].RID,glv_tag9F06.Value,5)))
		{
			Index_not_found = 0;
			memcpy(&pkm[2],glv_CAPK[i].Modulus,(UINT)glv_CAPK[i].Length);

			break;
		}
		else
		{
			Index_not_found = 1;
		}
	}
	
	if(Index_not_found)
		return FAIL ;
	
DBG_Put_Text("3.4. Retrieve Issuer PK");

//EMV 4.3 Book 2 Section 6.3 -Retrieval of the Issuer Public Key						tag90
	//Issuer Public Key Certificate length check 
	
	rspCode = UT_Get_TLVLengthOfV(glv_tag90.Length, &Tmp_Len1);

//	modulus with 248 bytes, 2L = 0xF8, 0x00

	pkm[0]=Tmp_Len1 & 0x00FF;
	pkm[1]=(Tmp_Len1 & 0xFF00) >> 8;
				
	if(glv_CAPK[i].Length == Tmp_Len1)
	{
		ISSPK[0]=Tmp_Len1 & 0x00FF;
		ISSPK[1]=(Tmp_Len1 & 0xFF00) >> 8;
//		UT_DumpHex(0,0,(UINT)Tmp_Len1,glv_tag90.Value);
		memcpy(&ISSPK[2],glv_tag90.Value,Tmp_Len1);
	}
	else
	{
		return FAIL ;
	}

DBG_Put_Text("3.4.1. Step1");

	//recover function on the Issuer Public Key Certificate using the Certification Authority Public Key
	
	//Load external public key
    if( api_rsa_loadkey( pkm, glv_CAPK[i].Exponent) != apiOK )
		return FAIL ;
	
	iPKcLen = Tmp_Len1;
	iLeftMostLen = Tmp_Len1 - 36;
	
	//recover	
	if( api_rsa_recover( ISSPK, ISSPK ) != apiOK )
		return FAIL ;


	iModLen = ISSPK[2+14-1];
		
	//vertify the recover data, trailer 0xBC
	if(ISSPK[Tmp_Len1+2-1] != 0xBC)
		return FAIL ;

DBG_Put_Text("3.4.2. Step2");

	//vertify the recover data, Recovered Data Header 0x6A
	if(ISSPK[2] != 0x6A)
		return FAIL ;

DBG_Put_Text("3.4.3. Step3");

	// vertify the recover data, check certificate format 0x02
    if(ISSPK[2+1] != 0x02)
     	return FAIL ;

DBG_Put_Text("3.4.4. Step4");

	//vertify the recover data, filed2-10, Issuer Public Key Remainder, Issuer Public Key Exponent (Step5,6,7)
	Re_len = (Tmp_Len1 - 22);
	for(i=0;i<Re_len;i++)
		g_ibuf[i] = ISSPK[i+3];	// filed2-10

	rspCode = UT_Get_TLVLengthOfV(glv_tag92.Length,&Tmp_Len1);

	if(Tmp_Len1 != 0)	//Issuer Public Key Remainder
	{
		memcpy(&g_ibuf[i],glv_tag92.Value,Tmp_Len1);	
		i+= Tmp_Len1;
		Re_len+= Tmp_Len1;
	}
	else
	{
		// Ni <= Nca - 36 ?
		if( iModLen > iLeftMostLen )
			return FAIL;
	}

	rspCode = UT_Get_TLVLengthOfV(glv_tag9F32.Length,&Tmp_Len1);
	
	if(Tmp_Len1 != 0)	//Issuer Public Key exponent
	{
		memcpy(&g_ibuf[i],glv_tag9F32.Value,Tmp_Len1);	
		i+=Tmp_Len1;
		Re_len+=Tmp_Len1;
	}
	else
	{
		return FAIL;
	}

DBG_Put_Text("3.4.5. Step5");

	if( api_sys_SHA1( Re_len, g_ibuf, temp) != apiOK )
		return FAIL;

DBG_Put_Text("3.4.6. Step6");

	for( i=0; i<20; i++ )
    {
        if( temp[i] != ISSPK[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
        	return FAIL;
    }

DBG_Put_Text("3.4.7. Step7");

	// Verification 8: check Issuer ID Number, Leftmost 3-8 digits from the PAN (padded to the right with Hex 'F's)
	rspCode = UT_Get_TLVLengthOfV(glv_tag5A.Length,&Tmp_Len1);
	
	if(Tmp_Len1 != 0)
		memcpy(temp,glv_tag5A.Value,Tmp_Len1);// load application PAN

	if( UT_CNcmp( &ISSPK[2+2], temp, 4 ) == FALSE )
        return FAIL;

DBG_Put_Text("3.4.8. Step8");

	// Verification 9: check the certificate expiration date MMYY
	if( UT_VerifyCertificateExpDate( &ISSPK[2+6] ) == FALSE )	
		return FAIL;

DBG_Put_Text("3.4.9. Step9");

	// Verification 10: RID[5] + INDEX[1] + Certificate Serial Number[3] 
	memcpy(&tmpRevList[0], glv_tag9F06.Value, 5);
	memcpy(&tmpRevList[5], glv_tag8F.Value, 1);
	memcpy(&tmpRevList[6], &ISSPK[2+8], 3);

	for ( i=0; i < 10; i++ )
	{	
		if ( UT_memcmp(tmpRevList, &Revo_List[i][0], 9 ) == 0 )
	    {
	    	return FAIL;
   		}
	}
	
DBG_Put_Text("3.4.10. Step10");
		
	// Verification 11: check the issuer public key algorithm indicator 0x01
      if( ISSPK[2+12] != 0x01 )
        return FAIL;

DBG_Put_Text("3.4.11. Step11");

	// Verification 12: concatenate the Leftmost Digits of the Issuer Public Key and the Issuer Public Key Remainder
	// Issuer Public Key Modulus (stored in pkm[iModLen] array) = 2L-V
    //        (1) Leftmost Digits of the Issuer Public Key +
    //        (2) Issuer Public Key Remainder (if present)			tag92
	for(i=0;i<iLeftMostLen;i++)
		pkm[i+2] = ISSPK[i+2+15];

	//UT_DumpHex(0,0,iLeftMostLen,&ISSPK[17]);
	rspCode = UT_Get_TLVLengthOfV(glv_tag92.Length,&Tmp_Len1);
	if(Tmp_Len1 != 0)
		memcpy(&pkm[i+2],glv_tag92.Value,Tmp_Len1);

DBG_Put_Text("3.4.12. Step12");


DBG_Put_Text("3.5. Retrieve ICC PK");

	rspCode = UT_Get_TLVLengthOfV(glv_tag9F46.Length,&Tmp_Len1);

	//6.4.1 ICC Public Key Certificate has a length different from the length of the Issuer Public Key Modulus	
	if(iModLen == Tmp_Len1)		
	{
		pkm[0]=Tmp_Len1 & 0x00FF;
		pkm[1]=(Tmp_Len1 & 0xFF00)>>8;
	}	
	else
	{
		return FAIL;
	}

DBG_Put_Text("3.5.1. Step1");

	//6.4.2 Obtain the recovered data
	iPKcLen = Tmp_Len1;
	iLeftMostLen = Tmp_Len1 - 42;
		
	//Load external public key
	if( api_rsa_loadkey( pkm, glv_tag9F32.Value) != apiOK )
		return FAIL ;

	ICCPK[0]=Tmp_Len1 & 0x00FF;
	ICCPK[1]=(Tmp_Len1 & 0xFF00)>>8;
	memcpy(&ICCPK[2],glv_tag9F46.Value,Tmp_Len1);
	
	//recover	
	if( api_rsa_recover( ICCPK, ICCPK ) != apiOK )
		return FAIL ;

	//20130625 for ram_gcc_nos_xx6.ld

	for(i=2;i<ICCPK[0];i++)
	{
		if(ICCPK[i] != 0x00)
		{
			memmove(&ICCPK[2],&ICCPK[i],ICCPK[0]);
			break;
		}
	}

	iModLen = ICCPK[2+20-1];	

	// Verification 2: check recovered data trailer
    if( ICCPK[ (iPKcLen+2)-1 ] != 0xBC )
    	return FAIL ;

DBG_Put_Text("3.5.2. Step2");

    // Verification 3: check recovered data header
    if( ICCPK[2+0] != 0x6A )
        return FAIL ;

DBG_Put_Text("3.5.3. Step3");

    // Verification 4: check certificate format
    if( ICCPK[2+1] != 0x04 )
      	return FAIL ;

DBG_Put_Text("3.5.4. Step4");
	
	// Verification 5: Concatenation
	   // (1) Recovered Data[2'nd..10'th] +
	   // (2) ICC Remainder +
	   // (3) ICC Exponent +
	   // (4) Static data to be authenticated
	Re_len = iPKcLen - 22; // header[1], HashResult[20], Trailer[1]
	
	for( i=0; i<Re_len; i++ )
	  	g_ibuf[i] = ICCPK[i+3]; // from "Certificate Format" to "ICC Public Key"

	rspCode = UT_Get_TLVLengthOfV(glv_tag9F48.Length,&Tmp_Len1);

	if(Tmp_Len1 != 0)	//ICC Public Key Remainder
	{
		memcpy(&g_ibuf[i],glv_tag9F48.Value,Tmp_Len1);	
		i+=Tmp_Len1;
		Re_len+=Tmp_Len1;
	}
	else
	{
		// Nic <= Ni - 42 ?
		if( iModLen > iLeftMostLen )
			return FAIL;
	}

	
	rspCode = UT_Get_TLVLengthOfV(glv_tag9F47.Length,&Tmp_Len1);
	
	if(Tmp_Len1 != 0) //ICC public key Exponent
	{
		memcpy(&g_ibuf[i],glv_tag9F47.Value,Tmp_Len1);	//ICC Public Key  exponent
		i+=Tmp_Len1;
		Re_len+=Tmp_Len1;
	}
	else
	{
		return FAIL;
	}
	
	
/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Get STATIC DATA
	// The fields within [] are included.
	// (1) SFI=1..10,  70 LL [Data] SW1 SW2
      	// (2) SFI=11..30, [70 LL Data] SW1 SW2
      	// (3) If the record is a non Tag-70, ODA has failed.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


	for(j=0;j<ODA_RECORD_NUMBER;j++)
	{
		memcpy(tag70_Data, oda_bufRecord+ODA_RECORD_SIZE*j, ODA_RECORD_SIZE);
		
		if(tag70_Data[0]==0x00)
		{
			break;
		}
		else
        {
			if (tag70_Data[1] != 0x70)	//CLQR01300
			{
				return FAIL;
			}
			
	    	if( (tag70_Data[0] & 0x1f) < 11 )	// SFI=1..10, only store data
	        {
	        	//get length and length's length
	        	if((tag70_Data[2] & 0x80) == 0x00)	//length = tag70_Data[2],	length's length = 1
	        	{	        		
	        		Tag70_Tmp_Len = tag70_Data[2];
					
	    			memcpy(&g_ibuf[i],&tag70_Data[1+1+1],Tag70_Tmp_Len);	
					i+=Tag70_Tmp_Len;
					Re_len+=Tag70_Tmp_Len;
	        	}
				else									//lenght = tag70_Data[3][4], length's length = 2 or 3
				{
					switch( tag70_Data[2] & 0x7f )
                    {
                     	case	0x00: // 1-byte length (128..255)
	                         	Tag70_Tmp_Len = tag70_Data[2];

	                         	memcpy(&g_ibuf[i],&tag70_Data[1+1+1],Tag70_Tmp_Len);	
							 	i+=Tag70_Tmp_Len;
							 	Re_len+=Tag70_Tmp_Len;
	                         	break;
								
	                    case	0x01: // 1-byte length (128..255)
	                         	Tag70_Tmp_Len = tag70_Data[3];

	                         	memcpy(&g_ibuf[i],&tag70_Data[1+1+2],Tag70_Tmp_Len);	
							 	i+=Tag70_Tmp_Len;
							 	Re_len+=Tag70_Tmp_Len;
	                         	break;

	                    case 	0x02: // 2-byte length (256..65535)
	                         	Tag70_Tmp_Len = tag70_Data[3]*256 + tag70_Data[4];

								memcpy(&g_ibuf[i],&tag70_Data[1+1+3],Tag70_Tmp_Len);	
							 	i+=Tag70_Tmp_Len;
							 	Re_len+=Tag70_Tmp_Len;
	                         	break;

	                    default:   // out of spec
	                        	return FAIL;
                    }
				}
	        }
	        else	// SFI=11..30, store tag length data
	        {
	        	if( (tag70_Data[0] & 0x1f) < 31 )
	            {
	            	if((tag70_Data[2] & 0x80) == 0x00)	//length = tag70_Data[2],	length's length = 1
		        	{	        		
		        		Tag70_Tmp_Len = tag70_Data[2];
						
		    			memcpy(&g_ibuf[i],&tag70_Data[1],Tag70_Tmp_Len+1+1);	
						i+=Tag70_Tmp_Len+1+1;
						Re_len+=Tag70_Tmp_Len+1+1;
		        	}
					else									//lenght = tag70_Data[3][4], length's length = 2 or 3
					{
						switch( tag70_Data[2] & 0x7f )
	                    {
							case	0x00: // 1-byte length (128..255)
	                         		Tag70_Tmp_Len = tag70_Data[2];

		                         	memcpy(&g_ibuf[i],&tag70_Data[1],Tag70_Tmp_Len+1+1);	
									i+=Tag70_Tmp_Len+1+1;
									Re_len+=Tag70_Tmp_Len+1+1;
		                         	break;
						
		                    case 	0x01: // 1-byte length (128..255)
		                        	Tag70_Tmp_Len = tag70_Data[3];

		                         	memcpy(&g_ibuf[i],&tag70_Data[1],Tag70_Tmp_Len+1+2);		//tag length 1, length length 2
								 	i+=Tag70_Tmp_Len+1+2;
								 	Re_len+=Tag70_Tmp_Len+1+2;
		                         	break;

		                    case 	0x02: // 2-byte length (256..65535)
		                         	Tag70_Tmp_Len = tag70_Data[3]*256 + tag70_Data[4];

		                         	memcpy(&g_ibuf[i],&tag70_Data[1],(UINT)Tag70_Tmp_Len+1+3);		//tag length 1, length length 3
									i+=Tag70_Tmp_Len+1+3;
								 	Re_len+=Tag70_Tmp_Len+1+3;
		                         	break;

		                    default:   // out of spec
		                         	return FAIL;
	                    }
					}        
	         	}
				else 
				{
					return FAIL;
				}
			}
   		}
	}


	// tag 9F4A list is present "Static Data Authentication Tag List"
	rspCode = UT_Get_TLVLengthOfV(glv_tag9F4A.Length,&Tmp_Len1);

	if(Tmp_Len1 != 0)
	{
		//get value's pointer
		ptrlist = glv_tag9F4A.Value;

		if(*ptrlist != 0x82)
			return FAIL;

		iTagListLen = Tmp_Len1;

		// concatenated to the current end of the input string
        do
		{
	   	    // check constructed DO
//        	if( apk_EMVCL_CheckConsTag( *ptrlist ) == TRUE )
			if (UT_Check_ConstructedTag(ptrlist) == TRUE)
            	return FAIL;

          	// check word tag
//         	if( apk_EMVCL_CheckWordTag( *ptrlist ) == TRUE )
			if (UT_Check_WordTag(ptrlist) == TRUE)
          	{
	            Tag_Len = 2;		//Word tag
				UT_Search(Tag_Len,ptrlist,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);
				ptrlist+=2;
				iTagListLen-=2;
          	}
          	else
          	{
	            Tag_Len = 1;		//single tag
				UT_Search(Tag_Len,ptrlist,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);
				ptrlist++;
				iTagListLen--;
	        }

			//check length
			if(*glv_addTable[Index] & 0x80)
			{
				if(*glv_addTable[Index] & 0x01)
				{
					Tmp_Len1 = *(glv_addTable[Index]+1);
				}
				else if(*glv_addTable[Index] & 0x02) 
				{
					Tmp_Len1 = ((*(glv_addTable[Index]+1))*256)+(*(glv_addTable[Index]+2));
				}
				else
				{
					Tmp_Len1 = *glv_addTable[Index];
				}
			}
			else
			{
				Tmp_Len1 = *glv_addTable[Index];
			}

			// concatenated to the current end of the input string
          	if( Tmp_Len1 != 0) 
            {
	          	memcpy(&g_ibuf[i],glv_addTable[Index]+3,Tmp_Len1);
				i+=Tmp_Len1;
				Re_len+=Tmp_Len1;
            }
	        else
	        {
    	        return FAIL;
	        }
		} while( iTagListLen > 0 ); // next tag  	
	}

DBG_Put_Text("3.5.5. Step5");

	// Verification 6: calculate & compare SHA1 result
	if( api_sys_SHA1(Re_len, g_ibuf, temp) != apiOK )
		return FAIL;

DBG_Put_Text("3.5.6. Step6");

	for( i=0; i<20; i++ )
	{
		if( temp[i] != ICCPK[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
			return FAIL;
	}
	
DBG_Put_Text("3.5.7. Step7");

	//Verification 8 : Compare the recovered PAN
	memset(temp, 0xff, 10 ); // padded with 'F'

	rspCode = UT_Get_TLVLengthOfV(glv_tag5A.Length,&Tmp_Len1);
	memcpy(temp,glv_tag5A.Value,Tmp_Len1);
	
	if(	UT_CNcmp2(&ICCPK[2+2],temp,10) == FALSE)
		return FAIL;

DBG_Put_Text("3.5.8. Step8");

	//Verification 9 : Verify that the last day
	if( UT_VerifyCertificateExpDate( &ICCPK[2+12] ) == FALSE )
    	return	FAIL;

DBG_Put_Text("3.5.9. Step9");

	//Verification 10 : Check ICC Public Key Algorithm Indicator
	if( ICCPK[2+18] != 0x01 )
        return FAIL;

DBG_Put_Text("3.5.10. Step10");

	//Verification 11 :  Concatenate the Leftmost Digits of the ICC Public Key and the ICC Public Key Remainder (if present) to 
	//				obtain the ICC Public Key Modulus
	for( i=0; i<iLeftMostLen; i++ )
		pkm[i+2] = ICCPK[i+2+21];

	rspCode = UT_Get_TLVLengthOfV(glv_tag9F48.Length,&Tmp_Len1);

	if(Tmp_Len1 != 0 )
	{
		memcpy(&pkm[i+2],glv_tag9F48.Value,Tmp_Len1);
		i+=Tmp_Len1;
	}

DBG_Put_Text("3.5.11. Step11");


DBG_Put_Text("3.6. DS Verification");

	//////////////////////////////////////////

		//6.5.2 Dynamic Signature Verification//

	//////////////////////////////////////////

	//Verification 1 : compare the length between "Signed Dynamic Application Data(9F4B)" & "ICC Public key module" 

	rspCode = UT_Get_TLVLengthOfV(glv_tag9F4B.Length,&Tmp_Len1);

	if(Tmp_Len1 != 0)
	{
		if(iModLen == Tmp_Len1)		
		{
			pkm[0]=Tmp_Len1 & 0x00FF;
			pkm[1]=(Tmp_Len1 & 0xFF00)>>8;
			iPKcLen = Tmp_Len1;
		}	
		else
		{
			return FAIL;
		}
	}

DBG_Put_Text("3.6.1. Step1");

	//Verification 2 : check recover data tail
	//Load external public key
	if( api_rsa_loadkey( pkm, glv_tag9F47.Value) != apiOK )
		return FAIL ;


	pkc[0]=Tmp_Len1 & 0x00FF;
	pkc[1]=(Tmp_Len1 & 0xFF00)>>8;
	memcpy(&pkc[2],glv_tag9F4B.Value,Tmp_Len1);
	
	//recover	
	if( api_rsa_recover( pkc, pkc ) != apiOK )
		return FAIL ;

	//20130625 for ram_gcc_nos_xx6.ld
	for(i=2;i<pkc[0];i++)
	{
		if(pkc[i] != 0x00)
		{
			memmove(&pkc[2],&pkc[i],pkc[0]);
			break;
		}
	}

	if(pkc[iPKcLen+2-1] != 0xBC)
		return FAIL ;

DBG_Put_Text("3.6.2. Step2");

	//Verification 3:	check recover data header
	if(pkc[2+0] != 0x6A)
		return FAIL;

DBG_Put_Text("3.6.3. Step3");

	//Verification 4:	check Signed Data Format
	if(pkc[2+1] != 0x05)
		return FAIL;

DBG_Put_Text("3.6.4. Step4");

	i=0;
	memcpy(&g_obuf[1+i],glv_tag9F37.Value,4);
	i+=4;

	if (verFDDA == 0x01)
	{
		memcpy(&g_obuf[1+i],glv_tag9F02.Value,6);
		i+=6;
		memcpy(&g_obuf[1+i],glv_tag5F2A.Value,2);
		i+=2;
		
		rspCode = UT_Get_TLVLengthOfV(glv_tag9F69.Length,&Tmp_Len1);
		memcpy(&g_obuf[1+i],glv_tag9F69.Value,Tmp_Len1);
		i+=Tmp_Len1;
	}

	g_obuf[0] = i;
		
	iHashLen = iPKcLen - 22; // header[1], HashResult[20], Trailer[1]
	for( i=0; i<iHashLen; i++ )
		g_ibuf[i] = pkc[i+3]; // from "Signed Data Format" to "Pad pattern"

	memcpy( &g_ibuf[i], &g_obuf[1], g_obuf[0] ); // cat DDOL
	iHashLen += g_obuf[0];

DBG_Put_Text("3.6.5. Step5");

	if( api_sys_SHA1(iHashLen, g_ibuf, temp) != apiOK )
			return FAIL;

DBG_Put_Text("3.6.6. Step6");

	for( i=0; i<20; i++ )
	{
		if( temp[i] != pkc[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
			return FAIL;
	}

DBG_Put_Text("3.6.7. Step7");

	//The ICC Dynamic Number contained in the ICC Dynamic Data recovered in Table 17 shall be stored in tag '9F4C'
	if ((pkc[2+4] >= 2) && (pkc[2+4] <= 8))	//Add 4 to Point to length of the ICC Dynamic Number
	{
		UT_Set_TagLength((UINT)pkc[2+4], glv_tag9F4C.Length);
		memcpy(glv_tag9F4C.Value, &pkc[2+5], pkc[2+4]);	//Add 5 to Point to ICC Dynamic Number
	}
	
	return SUCCESS;
   
}


UCHAR UNP_ODAforOnlineAuthorization(void)
{

DBG_Put_Text("4. ODA for Online Authorization");

DBG_Put_Text("4.1. Check ARQC");

	//Check ARQC
	if ((glv_tag9F27.Value[0] & 0xC0) != UNP_CID_ARQC)				
	{
		return UNP_RESULT_DECLINE;
	}

DBG_Put_Text("4.2. Check New Card");

	//Check New Card
	if (unp_flgNewCard == FALSE)
	{
		return UNP_RESULT_DECLINE;
	}
	
	//Black List?
	//Declines the transaction if the PAN is in the black list
	
	//Other Check?
	
	return UNP_RESULT_ODA_FOR_ONLINE_APPROVE;
}


void UNP_OutcomeProcess(UCHAR iptRspCode)
{
	//qUICS Scheme ID
	SchemeID=VAP_Scheme_qUICS;

	unp_txnResult=iptRspCode;
	
	switch (iptRspCode)
	{
		case UNP_RESULT_INTERRUPT_ICC:
			L3_Response_Code=VAP_RIF_RC_INSERT;
			break;

		case UNP_RESULT_INTERRUPT_MAG_STRIPE:
			L3_Response_Code=VAP_RIF_RC_SWIPE;
			break;
			
		case UNP_RESULT_TRY_AGAIN:
			etp_flgComError				= TRUE;
			
			etp_Outcome.Start			= ETP_OCP_Start_B;
			etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PresentCardAgain;
			etp_Outcome.ocmStatus		= ETP_OCP_UIS_ReadyToRead;

			L3_Response_Code=VAP_RIF_RC_FAILURE;

			break;

		case UNP_RESULT_PERFORM_CDCVM:
			etp_flgCDCVM				= TRUE;
			
			etp_Outcome.Start			= ETP_OCP_Start_B;
			etp_Outcome.rqtOutcome 		= ETP_OCP_UIOnOutcomePresent_Yes;
			etp_Outcome.ocmMsgID		= ETP_OCP_UIM_PresentCardAgain;
			etp_Outcome.ocmStatus		= ETP_OCP_UIS_ReadyToRead;
			etp_Outcome.filOffRequest	= 10;
			etp_Outcome.hldTime			= 10;

			L3_Response_Code=VAP_RIF_RC_TRY_AGAIN;
		
			break;

		case UNP_RESULT_TRY_ANOTHER_INTERFACE:
			etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;
			
			L3_Response_Code=VAP_RIF_RC_OTHER_INTERFACE;

			break;
		
		case UNP_RESULT_TERMINATE:
		case UNP_RESULT_SELECT_NEXT:
		case UNP_RESULT_DECLINE:
			etp_Outcome.ocmStatus		= ETP_OCP_UIS_ProcessingError;

			L3_Response_Code=VAP_RIF_RC_FAILURE;

			break;
			
		case UNP_RESULT_ONLINE_REQUEST:
		case UNP_RESULT_OFFLINE_APPROVE:
		case UNP_RESULT_ODA_FOR_ONLINE_APPROVE:
			etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;
			etp_Outcome.ocmMsgID		= ETP_OCP_UIM_AuthorisingPleaseWait;
			etp_Outcome.ocmStatus		= ETP_OCP_UIS_CardReadSuccessfully;

			
			L3_Response_Code=VAP_RIF_RC_DATA;
			VGL_AS210_D1_Track();
			UNP_Patch_OnlineData();

			break;
		
		default:	break;
	}	
}


void UNP_Start_Kernel(UINT lenFCI, UCHAR *datFCI)
{
	UCHAR	rspCode=UNP_RESULT_TERMINATE;
	UCHAR	flgDisSuccess=FALSE;

	rspCode=UNP_Allocate_Buffer();
	if (rspCode == UNP_RESULT_TERMINATE)
	{
		return;
	}

	memcpy(unp_rcvData, datFCI, lenFCI);
	unp_rcvLen=lenFCI;

	etp_flgComError=FALSE;
	etp_flgCDCVM=FALSE;

	unp_flgNewCard=FALSE;
	unp_txnResult=UNP_RESULT_TERMINATE;
	unp_addMessage=UNP_MESSAGE_NA;

	unp_lstLen=0;
	memset(unp_lstData, 0, UNP_LIST_BUFFER_SIZE);

	rspCode=UNP_ApplicationInitialization();
	if (rspCode == UNP_RESULT_SUCCESS)
	{
		rspCode=UNP_ReadApplicationData();
		if (rspCode == UNP_RESULT_SUCCESS)
		{
			ETP_UI_Request(ETP_OCP_UIM_CardReadOKRemoveCard, ETP_OCP_UIS_CardReadSuccessfully);
			flgDisSuccess=TRUE;

			rspCode=UNP_OfflineDataAuthentication();

			if (glv_tag9F66.Value[0] & UNP_TTQ_SUPPORT_ODA_FOR_ONLINE_AUTHORIZATION)
			{
				if (rspCode == SUCCESS)
				{
					rspCode=UNP_ODAforOnlineAuthorization();
				}
				else
				{
					rspCode=UNP_RESULT_DECLINE;
				}
			}
			else
			{
				if (rspCode == SUCCESS)
				{
					rspCode=UNP_RESULT_OFFLINE_APPROVE;
				}
				else
				{
					rspCode=UNP_Check_CTQDecision();
				}
			}
		}
	}

	if ((rspCode == UNP_RESULT_ONLINE_REQUEST) && (flgDisSuccess == FALSE))
	{
		ETP_UI_Request(ETP_OCP_UIM_CardReadOKRemoveCard, ETP_OCP_UIS_CardReadSuccessfully);
	}
	
	UNP_OutcomeProcess(rspCode);

	UNP_Free_Buffer();
}

