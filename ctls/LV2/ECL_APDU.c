#include <string.h>
#include "POSAPI.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Function.h"




UCHAR ECL_APDU_Select_PPSE(UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	rspCode=0;
	UCHAR 	cmdPPSE[20]={	
				0x00,	//CLA
				0xA4,	//INS
				0x04,	//P1
				0x00,	//P2
				0x0E,	//Length of Data Field
				'2', 'P', 'A', 'Y', '.', 'S', 'Y', 'S', '.', 'D', 'D', 'F', '0', '1',
				0x00};	//Le
	
	rspCode=ECL_LV1_DEP(20, cmdPPSE, rcvLen, rcvData, 0);

	return rspCode;
}

UCHAR ECL_APDU_Get_ProcessingOptions(UINT sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR sndBuffer[1024];
	UCHAR rspCode = 0;
	// sndBuffer = GPO_Header (CLA+INS+P1+P2 )+ Len (Lc) + GPO_Data (Data)+ GPO_Tail (La)
	UCHAR GPO_Header[4]={0x80,0xA8,0x00,0x00};
	UCHAR GPO_Tail = 0x00;
	
	memcpy(sndBuffer,GPO_Header,4);			//Copy header
	sndBuffer[4] = (UCHAR)sndLen ;			//Total length
	memcpy(&sndBuffer[5],sndData,sndLen);	//Copy data
	sndBuffer[sndLen + 5] = GPO_Tail ;		//Copy tail

	rspCode=ECL_LV1_DEP(sndLen + 6 , sndBuffer , rcvLen, rcvData, 1000);

	return rspCode;
}

UCHAR ECL_APDU_Read_Record(UCHAR sndRecNumber, UCHAR sndSFI, UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	rspCode=0;
	UCHAR	cmdReadRecord[5]={
				0x00,	//CLA
				0xB2,	//INS
				0x00,	//P1
				0x00,	//P2
				0x00};	//Le
								
	cmdReadRecord[2]=sndRecNumber;			//P1: Record Number
	cmdReadRecord[3]=(sndSFI<<3) | 0x04;	//P2: SFI | P1 is a record number

	rspCode=ECL_LV1_DEP(5, cmdReadRecord, rcvLen, rcvData, 1000);
			
	return rspCode;
}

UCHAR ECL_APDU_Internal_Authenticate(UINT sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	sndBuffer[1024];
	UCHAR	rspCode=0;
	UCHAR	cmdInternalAuthenticate[4]={0x00,	//CLA
										0x88,	//INS
										0x00,	//P1
										0x00};	//P2

	memset(sndBuffer,0x00,sizeof(sndBuffer));
	memcpy(sndBuffer,cmdInternalAuthenticate,4);	//Command
	sndBuffer[4] = (UCHAR) sndLen;					//Lc, length of Authentication-related data
	memcpy(&sndBuffer[5],sndData,sndLen);			//Data, Authentication-related data
	sndBuffer[5+sndLen] = 0x00;						//Le

	rspCode=ECL_LV1_DEP(6+sndLen, sndBuffer, rcvLen, rcvData, 1000);

	if (rspCode == ECL_LV1_SUCCESS)
		return ECL_LV1_SUCCESS;
		
	return ECL_LV1_FAIL;

}

UCHAR ECL_APDU_Generate_AC(UCHAR Ref_Con_Para, UINT sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	sndBuffer[1024];
	UCHAR	rspCode=0;
	UCHAR	cmd_Generate_AC[4]={0x80,	//CLA
								0xAE,	//INS
								0x00,	//P1
								0x00};	//P2

	cmd_Generate_AC[2] = Ref_Con_Para;			//Reference control parameter	
	
	memset(sndBuffer,0x00,sizeof(sndBuffer));
	memcpy(sndBuffer,cmd_Generate_AC,4);	//Command
	sndBuffer[4] = (UCHAR) sndLen;			//Lc, length of Authentication-related data
	memcpy(&sndBuffer[5],sndData,sndLen);	//Data, Authentication-related data
	sndBuffer[5+sndLen] = 0x00;				//Le

	rspCode=ECL_LV1_DEP(6+sndLen, sndBuffer, rcvLen, rcvData, 1000);

	if (rspCode == ECL_LV1_SUCCESS)
		return ECL_LV1_SUCCESS;
		
	return rspCode;

}

UCHAR ECL_APDU_Get_Magstripe_Data(UCHAR Ref_Con_Para, UINT sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	sndBuffer[1024];
	UCHAR	rspCode=0;
	UCHAR	cmd_Get_Magstripe[4]={	0x80,	//CLA
									0xD0,	//INS
									0x00,	//P1
									0x00};	//P2
	if(Ref_Con_Para == 0x80)	//Online Request
		cmd_Get_Magstripe[2] = 0x80;

	if(Ref_Con_Para == 0x00)	//Decline Request
		cmd_Get_Magstripe[2] = 0x00;
	
	memset(sndBuffer,0x00,sizeof(sndBuffer));
	memcpy(sndBuffer,cmd_Get_Magstripe,4);	//Command
	sndBuffer[4] = (UCHAR) sndLen;			//Lc, length of Authentication-related data
	memcpy(&sndBuffer[5],sndData,sndLen);	//Data, Authentication-related data
	sndBuffer[5+sndLen] = 0x00;				//Le

	rspCode=ECL_LV1_DEP(6+sndLen, sndBuffer, rcvLen, rcvData, 1000);

	if (rspCode == ECL_LV1_SUCCESS)
		return ECL_LV1_SUCCESS;
		
	return ECL_LV1_FAIL;
}

