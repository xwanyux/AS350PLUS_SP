/*
*	Test Environment Interface Function for PayPass L2
*/

#include <string.h>
#include "Glv_ReaderConfPara.h"
#include "Function.h"
#include "ECL_Tag.h"
#include "MPP_Define.h"
#include <stdlib.h>

//#define TEI_BUFFER_SIZE				4096	//3072	// 2017/10/11  william enlarge buffer for kernel configuration
#define TEI_BUFFER_SIZE				10240	//william	2017/10/27	huge .xml

#define TEI_TIMEOUT_AUTHORIZATION	300

#define TEI_SET_TRANSACTION_DATA	0xA0U
#define TEI_SET_CONFIGURATION_DATA	0xA1U
#define TEI_SET_CRL					0xA2U
#define TEI_SET_EXCEPTION_FILE		0xA3U
#define TEI_SET_DATETIME			0xA4U
#define TEI_SET_COMBINATION_TABLE	0xA5U
#define TEI_SET_DATAEXCHANGE_XML	0xA6U
#define TEI_SET_RECEIPT				0xA7U
#define TEI_SIGNAL_ACT				0xB0U
#define TEI_SIGNAL_CLEAN			0xB1U
#define TEI_SIGNAL_STOP				0xB2U
#define TEI_SIGNAL_CLEANALL			0xB3U
#define TEI_SIGNAL_CLEANXML			0xB4U
#define TEI_UPLOAD_DATA				0xC0U
#define TEI_UPLOAD_DE_DATA			0xC1U
#define TEI_UPLOAD_RRP_TIME			0xC2U
#define TEI_CLEAR_DISPLAY			0xD0U
#define TEI_AUTORUN_TRANSACTION		0xE0U
#define TEI_AUTORUN_CLEARDISPLAY	0xE1U
#define TEI_AUTHORIZE_APPROVE		0xF0U
#define TEI_AUTHORIZE_DECLINE		0xF1U


extern volatile	ULONG os_SysTimerFreeCnt;


extern UCHAR		etp_txnData[ETP_TRANSACTION_DATA_SIZE];
extern UINT			etp_txnDataLen;
extern KRNCONFIG	etp_krnConfig[ETP_NUMBER_TRANSACTIONTYPE];

extern void	ETP_Initialize_MPPCombinationTable(void);
extern void	ETP_Initialize_MPPCombinationTable_PPS_Select1(void);

//extern UCHAR	DE_xmlBuf[1024 * 20];	//william
extern UCHAR	*mpp_ptrDE_xmlBuf;
extern UINT		mpp_lenDE_xmlBuf;

extern UCHAR	mpp_flagCleanAll;		//Tammy 2017/11/09

UCHAR tei_dhnAUX=0xff; // modify by Wayne
UCHAR tei_flgACT=FALSE;
UCHAR tei_flgCLEAN=FALSE;
UCHAR tei_flgCleanAll = FALSE;	//Tammy 2017/11/09
UCHAR tei_flgAtoRun=FALSE;
UCHAR tei_flgAtoClear=TRUE;


UCHAR TEI_Open_AUX(void)
{
	API_AUX	sbuf;

	sbuf.Mode		=auxSOH;	
	sbuf.Baud		=COM_115200+COM_CHR8+COM_NOPARITY+COM_STOP1;
	sbuf.Tob		=10;
	sbuf.Tor		=50;
	sbuf.Acks		=0;
	sbuf.Resend		=0;
	sbuf.BufferSize	=TEI_BUFFER_SIZE;
	
	tei_dhnAUX=api_aux_open(COM0, sbuf);

	return tei_dhnAUX;
}


UCHAR TEI_Get_AUXCommand_STOP(void)
{
	UCHAR	rspCode=0;
	UINT	rcvLen=0;
	UCHAR	rcvData[512]={0};

	rspCode=api_aux_rxready(tei_dhnAUX, (UCHAR*)&rcvLen);
	if (rspCode == apiOK)
	{
		if (rcvLen != 0)
		{
			api_aux_rxstring(tei_dhnAUX, rcvData);
			switch (rcvData[2])
			{
				case TEI_SIGNAL_STOP:
					UT_BUZ_Beep1();
					return TRUE;

				case TEI_SIGNAL_CLEAN:
					mpp_flagCleanAll=FALSE;
					tei_flgCLEAN=TRUE;
					UT_BUZ_Beep1();
					return TRUE;

				case TEI_SIGNAL_CLEANALL:
					mpp_flagCleanAll=TRUE;
					tei_flgCLEAN=TRUE;
					UT_BUZ_Beep1();
					return TRUE;
					
				default: break;
			}
		}
	}

	return FALSE;
}


void TEI_Get_AUXCommand(void)
{
	UCHAR	rspCode=0;
	UINT	rcvLen=0;
	UCHAR	rcvData[TEI_BUFFER_SIZE]={0};
	UINT	idxNum=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfData=0;
	UINT	lenOfDatSet=0;
	UCHAR	ascDateTime[12]={0};
	UCHAR	tof9A[3]={0x9A,0x00,0x00};
	UCHAR	tof9F21[3]={0x9F,0x21,0x00};

	rspCode=api_aux_rxready(tei_dhnAUX, (UCHAR*)&rcvLen);
	if (rspCode == apiOK)
	{
		if (rcvLen != 0)
		{
			api_aux_rxstring(tei_dhnAUX, rcvData);
			switch (rcvData[2])
			{
				case TEI_SET_DATAEXCHANGE_XML:				// 2017/10/24 william   .xml file				
					if (mpp_ptrDE_xmlBuf != NULLPTR)
					{
						mpp_lenDE_xmlBuf = 0;
						free(mpp_ptrDE_xmlBuf);
					}
					// ex: 0x0D00(len) + A6 + "44454BFF8104 444554DF8112"
					mpp_lenDE_xmlBuf = rcvData[0] + rcvData[1] * 256 - 1;	//Decrease Command Length(1)
					// ex: 44454BFF8104 444554DF8112
					ptrData = &rcvData[3];
					if (mpp_lenDE_xmlBuf == 0)
						break;

					mpp_ptrDE_xmlBuf = malloc(sizeof(UCHAR) * mpp_lenDE_xmlBuf);
					memcpy(mpp_ptrDE_xmlBuf, ptrData, mpp_lenDE_xmlBuf);
					//memcpy(DE_xmlBuf, ptrData, mpp_lenDE_xmlBuf);		
					UT_BUZ_Beep1();
					break;

				case TEI_SET_TRANSACTION_DATA:
					etp_txnDataLen=rcvData[0]+rcvData[1]*256-1;	//Decrease Command Length(1)
					if (etp_txnDataLen <= ETP_TRANSACTION_DATA_SIZE)
					{
						memset(etp_txnData, 0, ETP_TRANSACTION_DATA_SIZE);
						memcpy(etp_txnData, &rcvData[3], etp_txnDataLen);

						//Set RTC if Date Time is avaliable
						ptrData=UT_Find_Tag(tof9A, etp_txnDataLen, etp_txnData);
						if (ptrData != NULLPTR)
						{
							UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
							UT_Split(&ascDateTime[0], ptrData+lenOfT+lenOfL, 3);

							ptrData=UT_Find_Tag(tof9F21, etp_txnDataLen, etp_txnData);
							if (ptrData != NULLPTR)
							{
								UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
								UT_Split(&ascDateTime[6], ptrData+lenOfT+lenOfL, 3);
							
								api_rtc_setdatetime(0, ascDateTime);
							}
						}
						
						UT_BUZ_Beep1();
					}
					
					break;

				case TEI_SET_CONFIGURATION_DATA:
					rcvLen=rcvData[0]+rcvData[1]*256-1;	//Decrease Command Length(1)
					ptrData=&rcvData[3];

					for (idxNum=0; idxNum < ETP_NUMBER_KERNELCONFIGURATION; idxNum++)
					{
						memset(&etp_krnConfig[idxNum], 0, KRNCONFIG_LEN);
					}

					idxNum=0;

					do {
						//Save AID
						UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);

						etp_krnConfig[idxNum].AIDLen=lenOfV;
						memcpy(etp_krnConfig[idxNum].AID, ptrData+lenOfT+lenOfL, lenOfV);

						ptrData+=(lenOfT+lenOfL+lenOfV);
						lenOfData+=(lenOfT+lenOfL+lenOfV);

						//Save Transaction Type
						UT_Get_TLVLength(ptrData, &lenOfT, &lenOfL, &lenOfV);
						etp_krnConfig[idxNum].txnType=*(ptrData+lenOfT+lenOfL);

						ptrData+=(lenOfT+lenOfL+lenOfV);
						lenOfData+=(lenOfT+lenOfL+lenOfV);

						lenOfDatSet=0;

						//Parse TLV
						while (1)
						{
							UT_Get_TLVLength(ptrData+lenOfDatSet, &lenOfT, &lenOfL, &lenOfV);

							lenOfDatSet+=(lenOfT+lenOfL+lenOfV);							

							if (((*(ptrData+lenOfDatSet) == 0x9F) && (*(ptrData+lenOfDatSet+1) == 0x06))
								||
								((lenOfData+lenOfDatSet) == rcvLen))
							{
								if (lenOfDatSet <= 512)
								{
									if ((idxNum+1) <= ETP_NUMBER_KERNELCONFIGURATION)
									{
										etp_krnConfig[idxNum].cfgLen=lenOfDatSet;
										memcpy(etp_krnConfig[idxNum].cfgData, ptrData, lenOfDatSet);

										idxNum++;
									}
								}

								ptrData+=lenOfDatSet;
								lenOfData+=lenOfDatSet;

								break;
							}
						}
					} while (lenOfData < rcvLen);


					UT_BUZ_Beep1();

					break;

				case TEI_SET_CRL:
					rcvLen=rcvData[0]+rcvData[1]*256-1;	//Decrease Command Length(1)
					ptrData=&rcvData[3];

					if ((rcvLen / 9) <= CRL_NUMBER)
					{
						for (idxNum=0; idxNum < (rcvLen / 9); idxNum++)
						{
							//Copy RID
							memcpy(glv_CRL[idxNum].RID, ptrData, 5);
							ptrData+=5;

							//Copy Index
							glv_CRL[idxNum].Index=ptrData[0];
							ptrData+=1;

							//Copy Serial Number
							memcpy(glv_CRL[idxNum].serNumber, ptrData, 3);
							ptrData+=3;
						}
					}

					UT_BUZ_Beep1();
					break;

				case TEI_SET_EXCEPTION_FILE:
					break;

				case TEI_SET_DATETIME:
					api_rtc_setdatetime(0, &rcvData[3]);

					UT_BUZ_Beep1();
					break;

				case TEI_SET_COMBINATION_TABLE:
//MCL TA Only
/*					if (rcvData[3] == 0)
					{
						ETP_Initialize_MPPCombinationTable();
					}
					else if (rcvData[3] == 1)
					{
						ETP_Initialize_MPPCombinationTable_PPS_Select1();
					}
*/					
					UT_BUZ_Beep1();
					break;

				case TEI_SIGNAL_ACT:
					tei_flgACT=TRUE;
					break;
				case TEI_SIGNAL_CLEANXML:	// 2017/11/4
					//memset(DE_xmlBuf, 0, DE_sizeXML);
					//memset(mpp_ptrDE_xmlBuf, 0, mpp_lenDE_xmlBuf);
					mpp_lenDE_xmlBuf = 0;
					free(mpp_ptrDE_xmlBuf);
					break;

				case TEI_SIGNAL_CLEAN:
					mpp_flagCleanAll = FALSE;	//Tammy 2017/11/09
					tei_flgCLEAN=TRUE;
					break;

				case TEI_SIGNAL_CLEANALL:
					mpp_flagCleanAll = TRUE;	//Tammy 2017/11/09
					tei_flgCLEAN = TRUE;
					break;

				case TEI_AUTORUN_TRANSACTION:
					if (rcvData[3] == 0)
					{
						tei_flgAtoRun=FALSE;
					}
					else if (rcvData[3] == 1)
					{
						tei_flgAtoRun=TRUE;
					}

					UT_BUZ_Beep1();
					break;

				case TEI_AUTORUN_CLEARDISPLAY:
					if (rcvData[3] == 0)
					{
						tei_flgAtoClear=FALSE;
					}
					else if (rcvData[3] == 1)
					{
						tei_flgAtoClear=TRUE;
					}

					UT_BUZ_Beep1();
					break;

				default: break;
			}
		}
	}
}
void TEI_Upload_Receipt(UINT sndLen, UCHAR *sndData)
{
	UCHAR	datBuffer[2 + 1 + 1024] = { 0 };

	datBuffer[0] = (sndLen + 1) & 0x00FF;
	datBuffer[1] = ((sndLen + 1) & 0xFF00) >> 8;
	datBuffer[2] = TEI_SET_RECEIPT;
	memcpy(&datBuffer[3], sndData, sndLen);

	UT_Tx_AUX(tei_dhnAUX, datBuffer);
}

void TEI_Upload_DE_Data(UINT sndLen, UCHAR *sndData)
{
	UCHAR	datBuffer[2 + 1 + 1024] = { 0 };

	datBuffer[0] = (sndLen + 1) & 0x00FF;
	datBuffer[1] = ((sndLen + 1) & 0xFF00) >> 8;
	datBuffer[2] = TEI_UPLOAD_DE_DATA;
	memcpy(&datBuffer[3], sndData, sndLen);

	UT_Tx_AUX(tei_dhnAUX, datBuffer);
}

void TEI_Upload_Data(UINT sndLen, UCHAR *sndData)
{
	UCHAR	datBuffer[2+1+1024]={0};

	datBuffer[0]=(sndLen+1) & 0x00FF;
	datBuffer[1]=((sndLen+1) & 0xFF00) >> 8;
	datBuffer[2]=TEI_UPLOAD_DATA;
	memcpy(&datBuffer[3], sndData, sndLen);
	
	UT_Tx_AUX(tei_dhnAUX, datBuffer);
}

void TEI_Upload_RRP_Data(UINT sndLen, UCHAR *sndData)	//Tammy 2017/11/22
{
	UCHAR datBuffer[2 + 1 + 2] = {0};

	datBuffer[0] = (sndLen + 1) & 0x00FF;
	datBuffer[1] = ((sndLen + 1) & 0xFF00) >> 8;
	datBuffer[2] = TEI_UPLOAD_RRP_TIME;
	memcpy(&datBuffer[3], sndData, sndLen);
	UT_Tx_AUX(tei_dhnAUX, datBuffer);
}


void TEI_Clear_Display(void)
{
	UCHAR	datBuffer[2+1]={0};

	datBuffer[0]=1;
	datBuffer[1]=0;
	datBuffer[2]=TEI_CLEAR_DISPLAY;
	
	UT_Tx_AUX(tei_dhnAUX, datBuffer);
}

//MCL TA Only
/*
UCHAR TEI_Check_OnlineAuthorization(UCHAR *optAutCode)
{
	ULONG	tmrStart;
	ULONG	tmrNow;
	UCHAR	rspCode;
	UINT	rcvLen=0;
	UCHAR	rcvData[512]={0};

	if (glv_tagDF8129.Value[0] == 0x30)	//MPP_OPS_STATUS_OnlineRequest
	{
		tmrStart=os_SysTimerFreeCnt;
		
		do
		{
			rspCode=api_aux_rxready(tei_dhnAUX, (UCHAR*)&rcvLen);
			if (rspCode == apiOK)
			{
				if (rcvLen != 0)
				{
					api_aux_rxstring(tei_dhnAUX, rcvData);
					switch (rcvData[2])
					{
						case TEI_AUTHORIZE_APPROVE:
							optAutCode[0]=0;
							return TRUE;

						case TEI_AUTHORIZE_DECLINE:
							optAutCode[0]=1;
							return TRUE;

						default: break;
					}
				}
			}

			tmrNow=os_SysTimerFreeCnt;
		} while ((tmrNow-tmrStart) < TEI_TIMEOUT_AUTHORIZATION);

		optAutCode[0]=0;
		
		return TRUE;
	}
	
	return FALSE;
}
*/

