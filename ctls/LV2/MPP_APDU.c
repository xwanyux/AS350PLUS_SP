#include <string.h>
#include "POSAPI.h"
#include "Define.h"
#include "Function.h"
#include "MPP_Define.h"
#include "MPP_Function.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Function.h"


void MPP_APDU_Add_Signal(UCHAR iptRspCode)
{
	switch (iptRspCode)
	{
		case ECL_LV1_SUCCESS:		MPP_Add_Signal(MPP_SIGNAL_RA);		break;
		case ECL_LV1_STOP_LATER:	MPP_Add_Signal(MPP_SIGNAL_STOP);	break;
		default:					MPP_Add_Signal(MPP_SIGNAL_L1RSP);	break;
	}
}


UCHAR MPP_APDU_GET_PROCESSING_OPTIONS(UINT sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	rspCode=0;
	UCHAR	sndBuffer[MPP_APDU_BUFFERSIZE]={0};
	UCHAR	cmdGPO[7]={
				0x80,	//CLA
				0xA8,	//INS
				0x00,	//P1
				0x00,	//P2
				0x00,	//Lc
				0x00,	//Data
				0x00};	//Le
	
	//Command Header
	memcpy(sndBuffer, cmdGPO, 4);

	//Command Length
	sndBuffer[4]=sndLen;

	//Command Data
	memcpy(&sndBuffer[5], sndData, sndLen);

	//Command Tail
	sndBuffer[5+sndLen]=0x00;

	//Send Command
	rspCode=ECL_LV1_DEP(6+sndLen, sndBuffer, rcvLen, rcvData, 0);

	//DE: recording APDU
	 MPP_DataExchangeLog((UCHAR*)&"APDU-GPO", 8);
	 MPP_DataExchangeLog(sndBuffer, sndLen + 6);
	 MPP_DataExchangeLog((UCHAR*)&"APDURes-GPO", 11);
	 MPP_DataExchangeLog( rcvData, *rcvLen);

	//Add to Queue
	MPP_APDU_Add_Signal(rspCode);

	return rspCode;
}


UCHAR MPP_APDU_READ_RECORD(UCHAR sndRecNumber, UCHAR sndSFI, UINT * rcvLen, UCHAR * rcvData)
{
	UCHAR	rspCode=0;
	UCHAR	cmdReadRecord[5]={
				0x00,	//CLA
				0xB2,	//INS
				0x00,	//P1
				0x00,	//P2
				0x00};	//Le

	//Record Number
	cmdReadRecord[2]=sndRecNumber;			//P1: Record Number

	//SFI
	cmdReadRecord[3]=(sndSFI<<3) | 0x04;	//P2: SFI | P1 is a record number

	//Send Command
	rspCode=ECL_LV1_DEP(5, cmdReadRecord, rcvLen, rcvData, 0);

	//DE: recording APDU
	 MPP_DataExchangeLog((UCHAR*)&"APDU- RR", 8);
	 MPP_DataExchangeLog(cmdReadRecord, 5);

	//Add to Queue
	MPP_APDU_Add_Signal(rspCode);

	return rspCode;
}


UCHAR MPP_APDU_COMPUTE_CRYPTOGRAPHIC_CHECKSUM(UCHAR sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	rspCode=0;
	UCHAR	sndBuffer[MPP_APDU_BUFFERSIZE]={0};
	UCHAR 	cmdCCC[7]={	
				0x80,	//CLA
				0x2A,	//INS
				0x8E,	//P1
				0x80,	//P2
				0x00,	//Lc
				0x00,	//Data
				0x00};	//Le

	//Command Header
	memcpy(sndBuffer, cmdCCC, 4);

	//Command Length
	sndBuffer[4]=sndLen;

	//Command Data
	memcpy(&sndBuffer[5], sndData, sndLen);

	//Command Tail
	sndBuffer[5+sndLen]=0x00;

	//Send Command
	rspCode=ECL_LV1_DEP(6+sndLen, sndBuffer, rcvLen, rcvData, 0);

	//DE: recording APDU
	 MPP_DataExchangeLog((UCHAR*)&"APDU-CCC", 8);
	 MPP_DataExchangeLog(sndBuffer, sndLen + 6);
	 MPP_DataExchangeLog((UCHAR*)&"APDURes-CCC", 11);
	 MPP_DataExchangeLog(rcvData, *rcvLen);

	//Add to Queue
	if (rspCode == ECL_LV1_STOP_LATER)	//Disregard STOP
		rspCode=ECL_LV1_SUCCESS;
	
	MPP_APDU_Add_Signal(rspCode);
		
	return rspCode;
}


UCHAR MPP_APDU_GENERATE_AC(UCHAR iptP1, UCHAR sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	rspCode=0;
	UCHAR	sndBuffer[MPP_APDU_BUFFERSIZE]={0};
	UCHAR 	cmdGenAC[7]={	
				0x80,	//CLA
				0xAE,	//INS
				0x00,	//P1
				0x00,	//P2
				0x00,	//Lc
				0x00,	//Data
				0x00};	//Le

	//Command Header
	cmdGenAC[2]=iptP1;
	memcpy(sndBuffer, cmdGenAC, 4);

	//Command Length
	sndBuffer[4]=sndLen;

	//Command Data
	memcpy(&sndBuffer[5], sndData, sndLen);

	//Command Tail
	sndBuffer[5+sndLen]=0x00;

	//Send Command
	rspCode=ECL_LV1_DEP(6+sndLen, sndBuffer, rcvLen, rcvData, 0);

	//DE: recording APDU
	 MPP_DataExchangeLog((UCHAR*)&"APDU-GAC", 8);
	 MPP_DataExchangeLog(sndBuffer, sndLen + 6);
	 MPP_DataExchangeLog((UCHAR*)&"APDURes-GAC", 11);
	 MPP_DataExchangeLog(rcvData, *rcvLen);

	//Add to Queue
	if (rspCode == ECL_LV1_STOP_LATER)	//Disregard STOP
		rspCode=ECL_LV1_SUCCESS;
	
	MPP_APDU_Add_Signal(rspCode);
		
	return rspCode;
}


UCHAR MPP_APDU_GET_DATA(UCHAR iptP1, UCHAR iptP2, UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	rspCode=0;
	UCHAR 	cmdGETDATA[5]={	
				0x80,	//CLA
				0xCA,	//INS
				0x00,	//P1
				0x00,	//P2
				0x00};	//Le

	//Command Header
	cmdGETDATA[2]=iptP1;
	cmdGETDATA[3]=iptP2;

	//Send Command
	rspCode=ECL_LV1_DEP(5, cmdGETDATA, rcvLen, rcvData, 0);

	//DE: recording APDU
	 MPP_DataExchangeLog((UCHAR*)&"APDU- GD", 8);
	 MPP_DataExchangeLog(cmdGETDATA, 5);

	//Add to Queue
	if (rspCode == ECL_LV1_STOP_LATER)	//Disregard STOP
		rspCode=ECL_LV1_SUCCESS;
	
	MPP_APDU_Add_Signal(rspCode);
		
	return rspCode;
}


UCHAR MPP_APDU_PUT_DATA(UCHAR iptP1, UCHAR iptP2, UCHAR sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	rspCode=0;
	UCHAR	sndBuffer[MPP_APDU_BUFFERSIZE]={0};
	UCHAR 	cmdPUTDATA[6]={	
				0x80,	//CLA
				0xDA,	//INS
				0x00,	//P1
				0x00,	//P2
				0x00,	//Lc
				0x00};	//Data

	//Command Header
	cmdPUTDATA[2]=iptP1;
	cmdPUTDATA[3]=iptP2;
	memcpy(sndBuffer, cmdPUTDATA, 4);

	//Command Length
	sndBuffer[4]=sndLen;

	//Command Data
	memcpy(&sndBuffer[5], sndData, sndLen);

	//Send Command
	rspCode=ECL_LV1_DEP(5+sndLen, sndBuffer, rcvLen, rcvData, 0);

	//DE: recording APDU
	 MPP_DataExchangeLog((UCHAR*)&"APDU- PD", 8);
	 MPP_DataExchangeLog(sndBuffer, sndLen + 5);
	 MPP_DataExchangeLog((UCHAR*)&"APDURes- PD", 11);
	 MPP_DataExchangeLog(rcvData, *rcvLen);

	//Add to Queue
	if (rspCode == ECL_LV1_STOP_LATER)	//Disregard STOP
		rspCode=ECL_LV1_SUCCESS;
	
	MPP_APDU_Add_Signal(rspCode);
		
	return rspCode;
}


UCHAR MPP_APDU_RECOVER_AC(UCHAR sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	rspCode=0;
	UCHAR	sndBuffer[MPP_APDU_BUFFERSIZE]={0};
	UCHAR 	cmdRcoAC[7]={	
				0x80,	//CLA
				0xD0,	//INS
				0x00,	//P1
				0x00,	//P2
				0x00,	//Lc
				0x00,	//Data
				0x00};	//Le

	//Command Header
	memcpy(sndBuffer, cmdRcoAC, 4);

	//Command Length
	sndBuffer[4]=sndLen;

	//Command Data
	memcpy(&sndBuffer[5], sndData, sndLen);

	//Command Tail
	sndBuffer[5+sndLen]=0x00;

	//Send Command
	rspCode=ECL_LV1_DEP(6+sndLen, sndBuffer, rcvLen, rcvData, 0);

	//DE: recording APDU
	 MPP_DataExchangeLog((UCHAR*)&"APDU-RAC", 8);
	 MPP_DataExchangeLog(sndBuffer, sndLen + 6);
	 MPP_DataExchangeLog((UCHAR*)&"APDURes-RAC", 11);
	 MPP_DataExchangeLog(rcvData, *rcvLen);

	//Add to Queue
	if (rspCode == ECL_LV1_STOP_LATER)	//Disregard STOP
		rspCode=ECL_LV1_SUCCESS;
	
	MPP_APDU_Add_Signal(rspCode);
		
	return rspCode;
}


UCHAR MPP_APDU_EXCHANGE_RELAY_RESISTANCE_DATA(UCHAR sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	rspCode=0;
	UCHAR	sndBuffer[MPP_APDU_BUFFERSIZE]={0};
	UCHAR	cmdExchRRdata[7]={
				0x80,	//CLA
				0xEA,	//INS
				0x00,	//P1
				0x00,	//P2
				0x00,	//Lc
				0x00,	//Data
				0x00};	//Le
	
	//Command Header
	memcpy(sndBuffer, cmdExchRRdata, 4);

	//Command Length
	sndBuffer[4]=sndLen;

	//Command Data
	memcpy(&sndBuffer[5], sndData, sndLen);

	//Command Tail
	sndBuffer[5+sndLen]=0x00;

	//Send Command
	rspCode=ECL_LV1_DEP(6+sndLen, sndBuffer, rcvLen, rcvData, 0);

	//DE: recording APDU
	 MPP_DataExchangeLog((UCHAR*)&"APDU-ERR", 8);
	 MPP_DataExchangeLog(sndBuffer, sndLen + 6);
	 MPP_DataExchangeLog((UCHAR*)&"APDURes-ERR", 11);
	 MPP_DataExchangeLog(rcvData, *rcvLen);

	//Add to Queue
	MPP_APDU_Add_Signal(rspCode);

	return rspCode;
}
