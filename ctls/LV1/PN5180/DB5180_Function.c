#include <string.h>
#include "POSAPI.h"
#include "ECL_LV1_Function.h"
#include "ECL_LV1_Util.h"
#include "ECL_LV1_Define.h"
#include "HIC_Define.h"
#include "HIC_Function.h"
#include "PCD_Function.h"
#include "PNQ_Function.h"
#include "DBG_Function.h"
#include "PN5180Function.h"




void DB5180_Check_Bitmask_Technology_TX(UINT iptData)
{	
	if (iptData & 0x0001) DBG_Put_Text("A 106");
	if (iptData & 0x0002) DBG_Put_Text("A 212");
	if (iptData & 0x0004) DBG_Put_Text("A 424");
	if (iptData & 0x0008) DBG_Put_Text("A 848");
	if (iptData & 0x0010) DBG_Put_Text("B 106");
	if (iptData & 0x0020) DBG_Put_Text("B 212");
	if (iptData & 0x0040) DBG_Put_Text("B 424");
	if (iptData & 0x0080) DBG_Put_Text("B 848");
	if (iptData & 0x0100) DBG_Put_Text("F 212");
	if (iptData & 0x0200) DBG_Put_Text("F 424");
	if (iptData & 0x0400) DBG_Put_Text("ISO/IEC 15693 ASK10");
	if (iptData & 0x0800) DBG_Put_Text("ISO/IEC 15693 ASK100");
	if (iptData & 0x1000) DBG_Put_Text("ISO/IEC 18000m3_TARI_18_88 us");
	if (iptData & 0x2000) DBG_Put_Text("ISO/IEC 18000m3_TARI_9_44 us");
	if (iptData & 0x4000) DBG_Put_Text("N/A");
	if (iptData & 0x8000) DBG_Put_Text("N/A");

}


void DB5180_Check_Bitmask_Technology_RX(UINT iptData)
{
	if (iptData & 0x0001) DBG_Put_Text("A 106");
	if (iptData & 0x0002) DBG_Put_Text("A 212");
	if (iptData & 0x0004) DBG_Put_Text("A 424");
	if (iptData & 0x0008) DBG_Put_Text("A 848");
	if (iptData & 0x0010) DBG_Put_Text("B 106");
	if (iptData & 0x0020) DBG_Put_Text("B 212");
	if (iptData & 0x0040) DBG_Put_Text("B 424");
	if (iptData & 0x0080) DBG_Put_Text("B 848");
	if (iptData & 0x0100) DBG_Put_Text("F 212");
	if (iptData & 0x0200) DBG_Put_Text("F 424");
	if (iptData & 0x0400) DBG_Put_Text("ISO/IEC 15693 ASK10_53");
	if (iptData & 0x0800) DBG_Put_Text("ISO/IEC 15693 ASK100_26");
	if (iptData & 0x1000) DBG_Put_Text("ISO/IEC 18000m3_Manch424_4");
	if (iptData & 0x2000) DBG_Put_Text("ISO/IEC 18000m3_Manch424_2_212");
	if (iptData & 0x4000) DBG_Put_Text("ISO/IEC 18000m3_Manch848_4_212");
	if (iptData & 0x8000) DBG_Put_Text("ISO/IEC 18000m3_Manch848_2");
}


void DB5180_Check_ParameterField_RX(UCHAR iptData)
{
	char	ascMessage[3];

	ascMessage[0]=(iptData & 0x04)?('-'):('+');
	ascMessage[1]='0'+(iptData&0x03);
	ascMessage[2]=0;

	DBG_Put_Text(ascMessage);
}


void DB5180_Check_ParameterField_TX(UCHAR iptData)
{
	char	ascMessage[3];

	ascMessage[0]=(iptData & 0x08)?('-'):('+');
	ascMessage[1]='0'+(iptData&0x07);
	ascMessage[2]=0;

	DBG_Put_Text(ascMessage);
}


void DB5180_Parse_EEPROM_DPC_AWC_DRC_LUT(void)
{
	UINT	i;
	UCHAR	cntTX=0;
	UCHAR	cntRX=0;
	UCHAR	rspCode=0;
	ULONG	intData=0;
	ULONG	intValue=0;
	UCHAR	datEEPROM[80];

	//Default data
/*	0x91,0x99,0x07,0x08,0x91,0x99,0x08,0x00,0x91,0x99,0xA0,0x37,0x91,0xF9,0x40,0x00,
	0x92,0xB9,0x10,0x00,0xA2,0x9B,0x20,0x00,0xA2,0xE9,0x40,0x00,0x92,0x9A,0x08,0x00,
	0x92,0xF9,0x80,0x00,0x93,0x90,0x47,0x08,0xA7,0x99,0x07,0x08,0x97,0x99,0x08,0x00,
	0x97,0x99,0x30,0x37,0x81,0x16,0x01,0x00,0x81,0x02,0x00,0x0C,0x81,0x02,0x00,0x30,
	0xD1,0x02,0x00,0x40,0x85,0x1A,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
*/

	rspCode=HIC_READ_EEPROM(HIC_EEPROM_PCD_AWC_DRC_LUT_SIZE, 1, datEEPROM);
	DBG_Put_Text("<HIC_EEPROM_PCD_AWC_DRC_LUT_SIZE>");
	DBG_Put_UCHAR(datEEPROM[0]);

	cntTX=datEEPROM[0] & 0x0F;
	cntRX=(datEEPROM[0] & 0xF0) >> 4;

	rspCode=HIC_READ_EEPROM(HIC_EEPROM_PCD_AWC_DRC_LUT, 80, datEEPROM);
	DBG_Put_Text("<HIC_EEPROM_PCD_AWC_DRC_LUT>");
	DBG_Put_Hex(80, datEEPROM);


	for (i=0; i < cntTX; i++)
	{
		DBG_Put_Text("");
		DBG_Put_Text("No.:");
		DBG_Put_UCHAR(i);
		DBG_Put_Text("TX Data:");
		DBG_Put_Hex(4, &datEEPROM[i*4]);

	 	intData=(datEEPROM[i*4+0]+(datEEPROM[i*4+1]<<8)+(datEEPROM[i*4+2]<<16)+(datEEPROM[i*4+3]<<24));


		DBG_Put_Text("DPC Gear:");
		intValue=ECL_LV1_UTI_Get_BitRangeValue(intData, 3, 0);
		DBG_Put_UCHAR((UCHAR)intValue);


		DBG_Put_Text("TAU_MOD_FALLING:");
		intValue=ECL_LV1_UTI_Get_BitRangeValue(intData, 7, 4);
		DB5180_Check_ParameterField_TX((UCHAR)intValue);


		DBG_Put_Text("TAU_MOD_RISING:");
		intValue=ECL_LV1_UTI_Get_BitRangeValue(intData, 11, 8);
		DB5180_Check_ParameterField_TX((UCHAR)intValue);


		DBG_Put_Text("RESIDUAL_CARRIER:");
		intValue=ECL_LV1_UTI_Get_BitRangeValue(intData, 15, 12);
		DB5180_Check_ParameterField_TX((UCHAR)intValue);


		DBG_Put_Text("Technology_TX:");
		intValue=ECL_LV1_UTI_Get_BitRangeValue(intData, 31, 16);
		DB5180_Check_Bitmask_Technology_TX((UINT)intValue);
	}

	for (i=cntTX; i < (cntTX+cntRX); i++)
	{
		DBG_Put_Text("");
		DBG_Put_Text("No.:");
		DBG_Put_UCHAR(i);
		DBG_Put_Text("RX Data:");
		DBG_Put_Hex(4, &datEEPROM[i*4]);

	 	intData=(datEEPROM[i*4+0]+(datEEPROM[i*4+1]<<8)+(datEEPROM[i*4+2]<<16)+(datEEPROM[i*4+3]<<24));


		DBG_Put_Text("DPC Gear:");
		intValue=ECL_LV1_UTI_Get_BitRangeValue(intData, 3, 0);
		DBG_Put_UCHAR((UCHAR)intValue);
		

		DBG_Put_Text("RX_HPCF:");
		intValue=ECL_LV1_UTI_Get_BitRangeValue(intData, 6, 4);
		DB5180_Check_ParameterField_RX((UCHAR)intValue);


		DBG_Put_Text("RX_GAIN:");
		intValue=ECL_LV1_UTI_Get_BitRangeValue(intData, 9, 7);
		DB5180_Check_ParameterField_RX((UCHAR)intValue);


		DBG_Put_Text("MIN_LEVEL:");
		intValue=ECL_LV1_UTI_Get_BitRangeValue(intData, 12, 10);
		DB5180_Check_ParameterField_RX((UCHAR)intValue);

		
		DBG_Put_Text("MIN_LEVELP:");
		intValue=ECL_LV1_UTI_Get_BitRangeValue(intData, 15, 13);
		DB5180_Check_ParameterField_RX((UCHAR)intValue);


		DBG_Put_Text("Technology_RX:");
		intValue=ECL_LV1_UTI_Get_BitRangeValue(intData, 31, 16);
		DB5180_Check_Bitmask_Technology_RX((UINT)intValue);
	}
}


void DB5180_Parse_EEPROM_DPC_CONTROL(void)
{
	UCHAR	rspCode;
	UCHAR	optData[32];
	ULONG	intData;

	
	DBG_Put_Text("<DPC_CONTROL>");
	rspCode=HIC_READ_EEPROM(HIC_EEPROM_DPC_CONTROL, 1, &optData[0]);	
	DBG_Put_UCHAR(optData[0]);

	intData=ECL_LV1_UTI_Get_BitRangeValue((ULONG)optData[0], 0, 0);
	if (intData)
		DBG_Put_Text("DPC is Enabled");
	else
		DBG_Put_Text("DPC is Disabled");

	DBG_Put_Text("Step Size:");
	intData=ECL_LV1_UTI_Get_BitRangeValue((ULONG)optData[0], 3, 1);
	DBG_Put_ULONG(intData);

	DBG_Put_Text("Start Gear:");
	intData=ECL_LV1_UTI_Get_BitRangeValue((ULONG)optData[0], 7, 4);
	DBG_Put_ULONG(intData);
}


void DB5180_Parse_EEPROM_AGC_CONTROL(void)
{
	UCHAR	rspCode;
	UCHAR	optData[32];
	ULONG	intData;

	
	DBG_Put_Text("<AGC_CONTROL>");
	rspCode=HIC_READ_EEPROM(HIC_EEPROM_AGC_CONTROL, 2, &optData[0]);
	DBG_Put_Hex(2, &optData[0]);

	intData=ECL_LV1_UTI_Get_BitRangeValue((optData[0]+optData[1]*256), 10, 10);
	if (intData)
		DBG_Put_Text("Duration is Enabled");
	else
		DBG_Put_Text("Duration is Disabled");

	DBG_Put_Text("Duration:");
	intData=ECL_LV1_UTI_Get_BitRangeValue((optData[0]+optData[1]*256), 9, 0);
	DBG_Put_ULONG(intData);

	intData=ECL_LV1_UTI_Get_BitRangeValue((optData[0]+optData[1]*256), 13, 13);
	if (intData)
		DBG_Put_Text("Step Size is Enabled");
	else
		DBG_Put_Text("Step Size is Disabled");

	DBG_Put_Text("Step Size:");
	intData=ECL_LV1_UTI_Get_BitRangeValue((optData[0]+optData[1]*256), 12, 11);
	DBG_Put_UCHAR(intData);
}


void DB5180_Parse_REGISTER_EMD_CONTROL(void)
{
	ULONG	optRegister;
	ULONG	valRegister;
	
	DBG_Put_Text("");
	DBG_Put_Text("<EMD_CONTROL>");

	HIC_READ_REGISTER(HIC_REGISTER_EMD_CONTROL, (UCHAR*)&optRegister);

	DBG_Put_ULONG(optRegister);

	valRegister=ECL_LV1_UTI_Get_BitRangeValue(optRegister, 0, 0);
	if (valRegister)
		DBG_Put_Text("EMD is Enabled");
	else
		DBG_Put_Text("EMD is Disabled");

	valRegister=ECL_LV1_UTI_Get_BitRangeValue(optRegister, 1, 1);
	if (valRegister)
		DBG_Put_Text("Tran. Error Above Threshold is No EMD: TRUE");
	else
		DBG_Put_Text("Tran. Error Above Threshold is No EMD: FALSE");

	DBG_Put_Text("Noise Bytes Threshold:");
	valRegister=ECL_LV1_UTI_Get_BitRangeValue(optRegister, 5, 2);
	DBG_Put_ULONG(valRegister);

	DBG_Put_Text("Transmission Timer Used:");
	valRegister=ECL_LV1_UTI_Get_BitRangeValue(optRegister, 9, 8);
	DBG_Put_ULONG(valRegister);
}


void DB5180_Print_DPC_Information(void)
{
	UCHAR	rspCode;
	UCHAR	optData[32];

	DBG_Put_Text("");
	DBG_Put_Text("[PN5180_DPC_Information]");


	DB5180_Parse_EEPROM_DPC_CONTROL();
	
	DB5180_Parse_EEPROM_AGC_CONTROL();
	
	DBG_Put_Text("<DPC_THRSH_HIGH>");
	rspCode=HIC_READ_EEPROM(HIC_EEPROM_DPC_THRSH_HIGH, 30, &optData[0]);
	DBG_Put_Hex(30, &optData[0]);
	
	
	DBG_Put_Text("<DPC_AGC_GEAR_LUT_SIZE>");
	rspCode=HIC_READ_EEPROM(HIC_EEPROM_DPC_AGC_GEAR_LUT_SIZE, 1, &optData[0]);
	DBG_Put_UCHAR(optData[0]);
	
	
	DBG_Put_Text("<DPC_AGC_GEAR_LUT>");
	rspCode=HIC_READ_EEPROM(HIC_EEPROM_DPC_AGC_GEAR_LUT, 15, &optData[0]);
	DBG_Put_Hex(15, &optData[0]);
}


void DB5180_Test_DPC(void)
{
	UCHAR rspCode=0;
	UCHAR optData[64]={0};
	

	//0x59 DPC_CONTROL(1) - Enables DPC and configures DPC gears
//	optData[0]=0x73;	//Enable DPC
//	optData[0]=0x72;	//Disable DPC
//	HIC_WRITE_EEPROM(HIC_EEPROM_DPC_CONTROL, 1, optData);
	HIC_READ_EEPROM(HIC_EEPROM_DPC_CONTROL, 1, optData);

	UT_PutStr(0, 0, FONT0, 4, (UCHAR*)"DPC:");
	if (optData[0] & 0x01)
		UT_PutStr(0, 4, FONT0, 3, (UCHAR*)"On ");
	else
		UT_PutStr(0, 4, FONT0, 3, (UCHAR*)"Off");

	//0x81 DPC_AGC_GEAR_LUT_SIZE(1) - Defines the number of gears
	optData[1]=4;
	HIC_WRITE_EEPROM(HIC_EEPROM_DPC_AGC_GEAR_LUT_SIZE, 1, &optData[1]);
	UT_PutStr(1, 0, FONT0, 10, (UCHAR*)"Gear size:");
	HIC_READ_EEPROM(HIC_EEPROM_DPC_AGC_GEAR_LUT_SIZE, 1, &optData[1]);
	UT_DispHexByte(1, 10, optData[1]);

	//0x82 DPC_AGC_GEAR_LUT(15) - Defines the Setting(DPC_CONFIG) for each step size
	optData[0]=0xF9;
	optData[1]=0xF7;
	optData[2]=0xF0;
	optData[3]=0xF4;
	HIC_WRITE_EEPROM(HIC_EEPROM_DPC_AGC_GEAR_LUT, 4, &optData[0]);
	UT_PutStr(2, 0, FONT0, 3, (UCHAR*)"TX:");
	HIC_READ_EEPROM(HIC_EEPROM_DPC_AGC_GEAR_LUT, 15, &optData[2]);
	for (rspCode=0; rspCode < 5; rspCode++)
	{
		UT_DispHexByte(3, rspCode*3, optData[2+rspCode]);
		UT_DispHexByte(4, rspCode*3, optData[2+rspCode+5]);
		UT_DispHexByte(5, rspCode*3, optData[2+rspCode+10]);
	}

	//0x5F DPC_THRSH_HIGH(30) - Defines the AGC high threshold for each gear
/*	optData[0]=0x20;
	optData[1]=0x01;
	optData[2]=0x90;
	optData[3]=0x01;
*/
	optData[0]=0x70;
	optData[1]=0x02;
	optData[2]=0x5A;
	optData[3]=0x02;
	optData[4]=0x2A;
	optData[5]=0x02;
	optData[6]=0x5A;
	optData[7]=0x03;
		
	HIC_WRITE_EEPROM(HIC_EEPROM_DPC_THRSH_HIGH, 8, &optData[0]);
	UT_PutStr(6, 0, FONT0, 7, (UCHAR*)"AGC_TH:");
	HIC_READ_EEPROM(HIC_EEPROM_DPC_THRSH_HIGH, 30, &optData[20]);
	for (rspCode=0; rspCode < 5; rspCode++)
	{
		UT_DispHexByte(7, rspCode*5,	optData[20+rspCode*2]);
		UT_DispHexByte(7, rspCode*5+2,	optData[20+rspCode*2+1]);
		UT_DispHexByte(8, rspCode*5,	optData[20+rspCode*2+10]);
		UT_DispHexByte(8, rspCode*5+2,	optData[20+rspCode*2+11]);
		UT_DispHexByte(9, rspCode*5,	optData[20+rspCode*2+20]);
		UT_DispHexByte(9, rspCode*5+2,	optData[20+rspCode*2+21]);
	}

	//0x5C DPC_XI - AGC trim value for production trimming
//	optData[0]=0;
//	HIC_WRITE_EEPROM(HIC_EEPROM_DPC_XI, 1, optData);
	HIC_READ_EEPROM(HIC_EEPROM_DPC_XI, 1, optData);
	UT_PutStr(10, 0, FONT0, 7, (UCHAR*)"DPC_XI:");
	UT_DispHexByte(10, 7, optData[0]);
	

	HIC_RF_ON();

/*	HIC_WRITE_REGISTER(HIC_REGISTER_DPC_CONFIG, UT_L2P(valTX));
	HIC_READ_REGISTER(HIC_REGISTER_DPC_CONFIG, optData);
	UT_PutStr(2, 0, FONT0, 3, "TX:");
	UT_DispHexByte(2, 3, valTX & 0x000000FF);
*/
	while (1)
	{
//		HIC_READ_EEPROM(HIC_EEPROM_DPC_CONTROL, 1, optData);
//		UT_DispHexByte(1, 0, optData[0]);

//		HIC_WRITE_REGISTER(HIC_REGISTER_DPC_CONFIG, UT_L2P(valTX));
		HIC_READ_REGISTER(HIC_REGISTER_DPC_CONFIG, optData);
		UT_PutStr(2, 0, FONT0, 3, (UCHAR*)"TX:");
		UT_DispHexByte(2, 3, optData[0]);
		UT_WaitTime(100);


		UT_WaitTime(100);
//		HIC_WRITE_REGISTER(HIC_REGISTER_DPC_CONFIG, UT_L2P(0x0F0FFFF2));
//		UT_WaitTime(500);

		HIC_READ_REGISTER(HIC_REGISTER_RF_STATUS, &optData[8]);
		UT_PutStr(11, 0, FONT0, 4, (UCHAR*)"AGC:");
		UT_DispHexWord(11, 4, (optData[9]&0x03)*256+optData[8]);	//Current AGC value
		UT_PutStr(12, 0, FONT0, 5, (UCHAR*)"Gear:");
		UT_DispHexByte(12, 5, (optData[10]&0xF0) >>	4);				//Current gear
//		UT_WaitTime(100);

	}
}


void DB5180_Test_MIFARE(void)
{
	UCHAR	rspCode;
	UINT	rcvLen;
	UCHAR	rcvBuffer[256];
	UCHAR	UID[10];
	
	while (1)
	{

DBG_Put_Text("");
DBG_Put_DateTime();

		//
		//Type A
		//
		HIC_RF_ON();
		UT_WaitTime(10);

		rspCode=PCD_WUPA(&rcvLen, rcvBuffer);

DBG_Put_Text("WUPA:");
DBG_Put_UCHAR(rspCode);

		if (rspCode == ECL_LV1_SUCCESS)
		{
DBG_Put_UINT(rcvLen);
DBG_Put_Hex(rcvLen, rcvBuffer);

			UT_WaitTime(10);

			PCD_HLTA();

DBG_Put_Text("HLTA:");

			UT_WaitTime(10);

			rspCode=PCD_WUPA(&rcvLen, rcvBuffer);

DBG_Put_Text("WUPA:");
DBG_Put_UCHAR(rspCode);

			if (rspCode == ECL_LV1_SUCCESS)
			{
DBG_Put_UINT(rcvLen);
DBG_Put_Hex(rcvLen, rcvBuffer);

				UT_WaitTime(10);

				rspCode=PCD_ANTICOLLISION(0x93, &rcvLen, rcvBuffer);

DBG_Put_Text("Anti-Col:");
DBG_Put_UCHAR(rspCode);

				if (rspCode == ECL_LV1_SUCCESS)
				{
DBG_Put_UINT(rcvLen);
DBG_Put_Hex(rcvLen, rcvBuffer);

memcpy(UID, rcvBuffer, rcvLen);

					UT_WaitTime(10);

					rspCode=PCD_SELECT(0x93, &rcvBuffer[0], &rcvLen, rcvBuffer);

DBG_Put_Text("Select:");
DBG_Put_UCHAR(rspCode);

//Test
//UCHAR keyA[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
//UCHAR keyB[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
//UCHAR keyA[6]={0x1E,0x7B,0x08,0x3F,0x57,0x14};
UCHAR keyB[6]={0xD3,0xA3,0xA2,0x9A,0x96,0xE5};
//UCHAR rspStatus=0xFF;

rspCode=PNQ_LOADKEYandAUTHENTICATION(keyB, 0x61, 12, UID);
//rspCode=PNQ_LOADKEY(keyA);
//rspCode=PNQ_AUTHENTICATION(0x60, 12, UID);

DBG_Put_Text("MIFARE Authenticate:");
DBG_Put_UCHAR(rspCode);

rspCode=PNQ_READ(12, rcvBuffer);

DBG_Put_Text("MIFARE Read (12):");
DBG_Put_UCHAR(rspCode);
if (rspCode == ECL_LV1_SUCCESS) DBG_Put_Hex(16, rcvBuffer);
/*
memset(rcvBuffer, 0x00, 16);
rspCode=PNQ_WRITE(12, rcvBuffer);
DBG_Put_Text("MIFARE Write (12):");
DBG_Put_UCHAR(rspCode);
*/


UCHAR valDecrement[4]={0x01,0x00,0x00,0x00};
rspCode=PNQ_INCREMENT(12, valDecrement);
DBG_Put_Text("MIFARE Increment:");
DBG_Put_UCHAR(rspCode);

rspCode=PNQ_TRANSFER(12);
DBG_Put_Text("MIFARE Transfer:");
DBG_Put_UCHAR(rspCode);


rspCode=PNQ_READ(12, rcvBuffer);

DBG_Put_Text("MIFARE Read (12):");
DBG_Put_UCHAR(rspCode);
if (rspCode == ECL_LV1_SUCCESS) DBG_Put_Hex(16, rcvBuffer);

				}
			}
		}

		HIC_RF_OFF();
		UT_WaitTime(200);
	}
}


