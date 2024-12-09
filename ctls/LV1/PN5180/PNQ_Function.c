#include <string.h>
#include "POSAPI.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Util.h"
#include "HIC_Function.h"
#include "HIC_Define.h"
#include "OS_Function.h"



UCHAR	pnq_parTX_UNDERSHOOT_PATTERN_LEN_A;	//TX_UNDERSHOOT_CONFIG[04:01]
UCHAR	pnq_parTX_UNDERSHOOT_PATTERN_A;		//TX_UNDERSHOOT_CONFIG[31:16]

UCHAR	pnq_parTX_OVERSHOOT_PATTERN_LEN_A;	//TX_OVERSHOOT_CONFIG[04:01]
UCHAR	pnq_parTX_OVERSHOOT_PATTERN_A;		//TX_OVERSHOOT_CONFIG[31:16]

UCHAR	pnq_parRX_MIN_LEVELP_A;				//SIGPRO_RM_CONFIG[11:08]
UCHAR	pnq_parRX_MIN_LEVEL_A;				//SIGPRO_RM_CONFIG[15:12]

UCHAR	pnq_parTX_TAU_MOD_RISING_A;			//RF_CONTROL_TX[03:00]
UCHAR	pnq_parTX_TAU_MOD_FALLING_A;		//RF_CONTROL_TX[07:04]
UCHAR	pnq_parTX_RESIDUAL_CARRIER_A;		//RF_CONTROL_TX[17:13]

UCHAR	pnq_parRX_HPCF_A;					//RF_CONTROL_RX[01:00]
UCHAR	pnq_parRX_GAIN_A;					//RF_CONTROL_RX[03:02]


UCHAR	pnq_parTX_UNDERSHOOT_PATTERN_LEN_B;
UCHAR	pnq_parTX_UNDERSHOOT_PATTERN_B;

UCHAR	pnq_parTX_OVERSHOOT_PATTERN_LEN_B;
UCHAR	pnq_parTX_OVRRSHOOT_PATTERN_B;

UCHAR	pnq_parRX_MIN_LEVELP_B;
UCHAR	pnq_parRX_MIN_LEVEL_B;

UCHAR	pnq_parTX_TAU_MOD_RISING_B;
UCHAR	pnq_parTX_TAU_MOD_FALLING_B;
UCHAR	pnq_parTX_RESIDUAL_CARRIER_B;

UCHAR	pnq_parRX_HPCF_B;
UCHAR	pnq_parRX_GAIN_B;


ULONG	pnq_tmrReceive=0;


static UCHAR pnq_bufData[16];

extern volatile	ULONG os_SysTimerFreeCnt;


UCHAR PNQ_Receive_Process(UINT *optLen, UCHAR *optData);
void  PNQ_Set_IRQ(UCHAR *iptData);
void  PNQ_Set_SystemConfig(UCHAR iptData);
void  PNQ_Set_Timer(ULONG iptData);



void PNQ_Load_RF_Parameter_TA(void)
{
	pnq_parTX_TAU_MOD_RISING_A=0x03;
	pnq_parTX_TAU_MOD_FALLING_A=0x04;
	pnq_parTX_RESIDUAL_CARRIER_A=0x1E;
	
	pnq_parRX_GAIN_A=0x02;
	pnq_parRX_HPCF_A=0x03;
	
	
	pnq_parTX_TAU_MOD_RISING_B=0x03;
	pnq_parTX_TAU_MOD_FALLING_B=0x02;
	pnq_parTX_RESIDUAL_CARRIER_B=0x10;

	pnq_parRX_GAIN_B=0x03;
	pnq_parRX_HPCF_B=0x03;

	pnq_parRX_MIN_LEVEL_B=0x05;
	pnq_parRX_MIN_LEVELP_B=0x0A;
}


UCHAR PNQ_Get_CLChipSerialNumber(UCHAR * optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	bufData[16];

	rspCode=HIC_READ_EEPROM(HIC_EEPROM_DIE_IDENTIFIER, 16, bufData);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	memcpy(optData, bufData, 16);

	return ECL_LV1_SUCCESS;
}


UCHAR PNQ_Check_SPI_PN5180(void)
{
	UCHAR bufEEPROM[8]={0};
	UCHAR rspCode=0;
	UCHAR cntRetry=0;

	do
	{
		rspCode=HIC_READ_EEPROM(HIC_EEPROM_FIRMWARE_VERSION, 2, bufEEPROM);
		if ((rspCode == ECL_LV1_SUCCESS) &&
			((bufEEPROM[1] == 0x03) || (bufEEPROM[1] == 0x04)))
		{
			printf("ECL_LV1_SUCCESS\n");
			return ECL_LV1_SUCCESS;
		}
		
		ECL_LV1_UTI_Wait(50000);
	} while (0);
	printf("ECL_LV1_FAIL bufEEPROM[1] =0x%02x\n",bufEEPROM[1]);
	return ECL_LV1_FAIL;
}


void PNQ_Clear_IRQ(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	regData[4]={0xFF,0xFF,0xFF,0xFF};
	
	rspCode=HIC_WRITE_REGISTER(HIC_REGISTER_IRQ_CLEAR, regData);
}


UCHAR PNQ_Perform_ValueBlockCommand_Phase1(UCHAR iptCommand, UCHAR iptAddress)
{
	UCHAR	cmdValue[2];
	UCHAR	rspCode=ECL_LV1_FAIL;
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[16]={0};
	ULONG	intRegister=0;
	
	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_A+HIC_MIFARE_TIMEOUT_5MS);
	
	//Enable TX CRC, Disable RX CRC
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_CRC_TX_CONFIG, ECL_LV1_UTI_L2P(HIC_TX_CRC_ENABLE));
	HIC_WRITE_REGISTER_AND_MASK(HIC_REGISTER_CRC_RX_CONFIG, ECL_LV1_UTI_L2P(~HIC_RX_CRC_ENABLE));

	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);

	cmdValue[0]=iptCommand;
	cmdValue[1]=iptAddress;
	HIC_SEND_DATA(2, cmdValue);

	rspCode=PNQ_Receive_Process(&rcvLen, rcvBuffer);
	if ((rspCode == ECL_LV1_SUCCESS) && (rcvLen == 1))
	{
		HIC_READ_REGISTER(HIC_REGISTER_RX_STATUS, (UCHAR*)&intRegister);
		if ((intRegister & 0x0000E000) == 0x00008000)	//RX_NUM_LAST_BITS = 4 bits
		{
			HIC_READ_DATA(rcvLen, rcvBuffer);
			if (rcvBuffer[0] != HIC_MIFARE_ACK)
			{
				return ECL_LV1_FAIL;
			}
		}
		else
		{
			return ECL_LV1_FAIL;
		}
	}
	else
	{
		return ECL_LV1_FAIL;
	}

	return ECL_LV1_SUCCESS;
}


UCHAR PNQ_Perform_ValueBlockCommand_Phase2(UCHAR *iptValue)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[16]={0};
	ULONG	intRegister=0;

	
	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_A+HIC_MIFARE_TIMEOUT_10MS);
	
	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);
	HIC_SEND_DATA(4, iptValue);

	rspCode=PNQ_Receive_Process(&rcvLen, rcvBuffer);
	if (rspCode != ECL_LV1_TIMEOUT_ISO)
	{
		if ((rspCode == ECL_LV1_SUCCESS) && (rcvLen == 1))
		{
			HIC_READ_REGISTER(HIC_REGISTER_RX_STATUS, (UCHAR*)&intRegister);
			if ((intRegister & 0x0000E000) == 0x00008000)	//RX_NUM_LAST_BITS = 4 bits
			{
				;	//NAK
			}
		}

		return ECL_LV1_FAIL;
	}

	//Timeout means SUCCESS for this Command
	return ECL_LV1_SUCCESS;
}


UCHAR PNQ_Receive_Process(UINT *optLen, UCHAR *optData)
#if 0
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	ULONG	intRegIRQ=0;
	ULONG	intRegRX=0;
	

	PNQ_Clear_IRQ();
	
	//Set RX & Timer 0 IRQ
	PNQ_Set_IRQ(ECL_LV1_UTI_L2P(HIC_IRQ_STAT_RX|HIC_IRQ_STAT_TIMER0));

	//Waiting for IRQ
	do
	{
		HIC_READ_REGISTER(HIC_REGISTER_RX_STATUS, (UCHAR*)&intRegRX);
		if (intRegRX & HIC_RX_PROTOCOL_ERROR)
		{
			rspCode=ECL_LV1_ERROR_PROTOCOL;

			//Delay to prevent error
			ECL_LV1_UTI_Wait(300);

			break;
		}

		HIC_READ_REGISTER(HIC_REGISTER_IRQ_STATUS, (UCHAR*)&intRegIRQ);
	} while ((!(intRegIRQ & HIC_IRQ_STAT_RX)) && (!(intRegIRQ & HIC_IRQ_STAT_TIMER0)));

	if (intRegIRQ & HIC_IRQ_STAT_RX)
	{
		HIC_READ_REGISTER(HIC_REGISTER_RX_STATUS, (UCHAR*)&intRegIRQ);

		if (intRegIRQ & HIC_RX_COLLISION_DETECTED)
		{
			rspCode=ECL_LV1_COLLISION;
		}
		else if (intRegIRQ & HIC_RX_PROTOCOL_ERROR)
		{
			rspCode=ECL_LV1_ERROR_PROTOCOL;
		}
		else if (intRegIRQ & HIC_RX_DATA_INTEGRITY_ERROR)
		{
			rspCode=ECL_LV1_ERROR_INTEGRITY;
		}
		else
		{
			optLen[0]=(UINT)(intRegIRQ & 0x000001FF);

			HIC_READ_DATA(optLen[0], optData);

			rspCode=ECL_LV1_SUCCESS;
		}
	}
	else if (intRegIRQ & HIC_IRQ_STAT_TIMER0)
	{
		rspCode=ECL_LV1_TIMEOUT_ISO;
	}

	PNQ_Clear_IRQ();
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_IDLE);
	
	return rspCode;
}
#else
{
	ULONG	intRegIRQ=0;
	ULONG	intRegRX=0;
	UINT	cntTimeout=0;
	UINT	cntIteration=0;
	ULONG	tmrResidual=0;
	UCHAR	flgRX=FALSE;
	UCHAR	rspCode=ECL_LV1_FAIL;


	PNQ_Clear_IRQ();
	
	//Set IRQ
	PNQ_Set_IRQ(ECL_LV1_UTI_L2P(HIC_IRQ_STAT_RX|HIC_IRQ_STAT_TIMER0));


	cntIteration=pnq_tmrReceive/HIC_TIMER_MAX_RELOAD_VALUE;
	tmrResidual=pnq_tmrReceive-cntIteration*HIC_TIMER_MAX_RELOAD_VALUE;
	
	while (cntTimeout < cntIteration)
	{
		HIC_READ_REGISTER(HIC_REGISTER_IRQ_STATUS, (UCHAR*)&intRegIRQ);
		if (intRegIRQ & HIC_IRQ_STAT_TIMER0)
		{
			cntTimeout++;
			PNQ_Clear_IRQ();
		}
		else if (intRegIRQ & HIC_IRQ_STAT_RX)
		{
			flgRX=TRUE;
			break;
		}

		HIC_READ_REGISTER(HIC_REGISTER_RX_STATUS, (UCHAR*)&intRegRX);
		if (intRegRX & HIC_RX_PROTOCOL_ERROR)
		{
			break;
		}
	}

	if ((flgRX == FALSE) && ((intRegRX & HIC_RX_PROTOCOL_ERROR) == 0))
	{
		if ((cntIteration != 0) || (tmrResidual != 0))
		{
			if (cntIteration != 0)
			{
				//Stop Timer, Set Reload value & Re-Start Timer
				HIC_WRITE_REGISTER(HIC_REGISTER_TIMER0_CONFIG, ECL_LV1_UTI_L2P(0));
				HIC_WRITE_REGISTER(HIC_REGISTER_TIMER0_RELOAD, ECL_LV1_UTI_L2P(tmrResidual));
				HIC_WRITE_REGISTER(HIC_REGISTER_TIMER0_CONFIG, ECL_LV1_UTI_L2P(HIC_T0_ENABLE | HIC_T0_START_NOW | HIC_T0_STOP_ON_RX_STARTED));
			}
			
			do
			{
				HIC_READ_REGISTER(HIC_REGISTER_RX_STATUS, (UCHAR*)&intRegRX);
				if (intRegRX & HIC_RX_PROTOCOL_ERROR)
				{
					break;
				}
				
				HIC_READ_REGISTER(HIC_REGISTER_IRQ_STATUS, (UCHAR*)&intRegIRQ);
			} while ((!(intRegIRQ & HIC_IRQ_STAT_RX)) && (!(intRegIRQ & HIC_IRQ_STAT_TIMER0)));

			flgRX=(intRegIRQ & HIC_IRQ_STAT_RX)?(TRUE):(FALSE);
		}
	}

	if (flgRX == TRUE)
	{
		HIC_READ_REGISTER(HIC_REGISTER_RX_STATUS, (UCHAR*)&intRegRX);

		if (intRegRX & HIC_RX_COLLISION_DETECTED)
		{
			rspCode=ECL_LV1_COLLISION;
		}
		else if (intRegRX & HIC_RX_PROTOCOL_ERROR)
		{
			rspCode=ECL_LV1_ERROR_TRANSMISSION;

			//Delay to prevent error
			ECL_LV1_UTI_Wait(300);
		}
		else if (intRegRX & HIC_RX_DATA_INTEGRITY_ERROR)
		{
			rspCode=ECL_LV1_ERROR_TRANSMISSION;
		}
		else
		{
			optLen[0]=(UINT)(intRegRX & 0x000001FF);

			HIC_READ_DATA(optLen[0], optData);

			rspCode=ECL_LV1_SUCCESS;
		}
	}
	else if (intRegRX & HIC_RX_PROTOCOL_ERROR)
	{
		rspCode=ECL_LV1_ERROR_TRANSMISSION;

		//Delay to prevent error
		ECL_LV1_UTI_Wait(300);
	}
	else
	{
		rspCode=ECL_LV1_TIMEOUT_ISO;
	}


	PNQ_Clear_IRQ();
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_IDLE);
	
	return rspCode;
}
#endif

void PNQ_Set_IRQ(UCHAR *iptData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_IRQ_ENABLE, iptData);
}


void PNQ_Set_SystemConfig(UCHAR iptData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	//Clear Command Field = Idle
	rspCode=HIC_WRITE_REGISTER_AND_MASK(HIC_REGISTER_SYSTEM_CONFIG, (UCHAR*)"\xF8\xFF\xFF\xFF");

	//Set Command
	if (iptData == HIC_SYS_CONFIG_COMMAND_TRANSCEIVE)
	{
		rspCode=HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_SYSTEM_CONFIG, ECL_LV1_UTI_L2P(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE));
	}
}


void PNQ_Set_Timer(ULONG iptData)
{
	ULONG	regValue=(HIC_T0_ENABLE|HIC_T0_START_ON_TX_ENDED|HIC_T0_STOP_ON_RX_STARTED);
	ULONG	tmrValue=0;
	
	pnq_tmrReceive=iptData;

	if (pnq_tmrReceive > HIC_TIMER_MAX_RELOAD_VALUE)
	{
		regValue|=HIC_T0_RELOAD_ENABLE;
		tmrValue=HIC_TIMER_MAX_RELOAD_VALUE;
	}
	else
	{
		
		tmrValue=pnq_tmrReceive;
	}
	
	HIC_WRITE_REGISTER(HIC_REGISTER_TIMER0_CONFIG, ECL_LV1_UTI_L2P(regValue));
	HIC_WRITE_REGISTER(HIC_REGISTER_TIMER0_RELOAD, ECL_LV1_UTI_L2P(tmrValue));
}


UCHAR PNQ_Wait_IRQ(UCHAR *iptData)
{
	UCHAR rspCode=ECL_LV1_FAIL;
	ULONG tmrStart;
	ULONG tmrTick;
	UCHAR bufRegister[4]={0};

	tmrStart=OS_GET_SysTimerFreeCnt();
	
	do
	{
		rspCode=HIC_READ_REGISTER(HIC_REGISTER_IRQ_STATUS, bufRegister);
		
		if ((bufRegister[0] & iptData[0]) ||
			(bufRegister[1] & iptData[1]) ||
			(bufRegister[2] & iptData[2]) ||
			(bufRegister[3] & iptData[3]))
		{
			return ECL_LV1_SUCCESS;
		}
		
		tmrTick=OS_GET_SysTimerFreeCnt()-tmrStart;
	} while (tmrTick < HIC_TIMER_GLOBAL_IRQ);

	return ECL_LV1_FAIL;
}


void PNQ_Switch_FieldOn(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=HIC_RF_ON();
}


void PNQ_Switch_FieldOff(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	
	rspCode=HIC_RF_OFF();
}


void PNQ_Initialize_Reader(void)
{
	UCHAR	optData[8];
	UCHAR	valDPC_CONTROL[1]={0x00|0x02|0x01};	//START_GEAR:0 | STEP_SIZE:1 | DPC:On
//	UCHAR	valDPC_CONTROL[1]={0x00|0x02|0x00};	//START_GEAR:0 | STEP_SIZE:1 | DPC:Off

	//Set DPC Control
	HIC_WRITE_EEPROM(HIC_EEPROM_DPC_CONTROL, 1, valDPC_CONTROL);


	//Disable PCD_AWC_DRC_LUT
	HIC_READ_EEPROM(HIC_EEPROM_PCD_AWC_DRC_LUT_SIZE, 1, optData);
	if (optData != 0)
	{
		HIC_WRITE_EEPROM(HIC_EEPROM_PCD_AWC_DRC_LUT_SIZE, 1, ECL_LV1_UTI_L2P(0));
	}

	//Set TX_CW_TO_MAX_RM (Max. TX Power)
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_DPC_CONFIG, ECL_LV1_UTI_L2P(0x00000008));


	//Configure DPC & AGC setting when DPC is On

}


void PNQ_Load_Protocol_A(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	ULONG	tmpValue=0;
	
	rspCode=HIC_LOAD_RF_CONFIG(HIC_PROTOCOL_TX_ISO14443_A_106, HIC_PROTOCOL_RX_ISO14443_A_106);

	//Customisation
	tmpValue=0;
	HIC_READ_REGISTER(HIC_REGISTER_RF_CONTROL_TX, (UCHAR*)&tmpValue);
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue,  3,  0, pnq_parTX_TAU_MOD_RISING_A);
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue,  7,  4, pnq_parTX_TAU_MOD_FALLING_A);
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 17, 13, pnq_parTX_RESIDUAL_CARRIER_A);
	HIC_WRITE_REGISTER(HIC_REGISTER_RF_CONTROL_TX, ECL_LV1_UTI_L2P(tmpValue));

	tmpValue=0;
	HIC_READ_REGISTER(HIC_REGISTER_RF_CONTROL_RX, (UCHAR*)&tmpValue);
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 1, 0, pnq_parRX_GAIN_A);
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 3, 2, pnq_parRX_HPCF_A);
	HIC_WRITE_REGISTER(HIC_REGISTER_RF_CONTROL_RX, ECL_LV1_UTI_L2P(tmpValue));

	tmpValue=0;
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue,  7, 0, 0x7F);	//Prescaler
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 27, 8, 0x08);	//RX wait value
	HIC_WRITE_REGISTER(HIC_REGISTER_RX_WAIT_CONFIG, ECL_LV1_UTI_L2P(tmpValue));

	tmpValue=0;
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 0, 0, 0);	//Disable EMD
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 1, 1, 1);	//ERROR > THRESHOLD is not EMD
//	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 5, 2, 4);	//Threshold: 4 bytes
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 5, 2, 1);	//PN5180 data sheet: 1 byte
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 9, 8, 0);	//Timer 0
	HIC_WRITE_REGISTER(HIC_REGISTER_EMD_CONTROL, ECL_LV1_UTI_L2P(tmpValue));
}


void PNQ_Load_Protocol_B(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	ULONG	tmpValue=0;
	
	rspCode=HIC_LOAD_RF_CONFIG(HIC_PROTOCOL_TX_ISO14443_B_106, HIC_PROTOCOL_RX_ISO14443_B_106);

	//Customisation
	tmpValue=0;
	HIC_READ_REGISTER(HIC_REGISTER_RF_CONTROL_TX, (UCHAR*)&tmpValue);
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue,  3,  0, pnq_parTX_TAU_MOD_RISING_B);
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue,  7,  4, pnq_parTX_TAU_MOD_FALLING_B);
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 17, 13, pnq_parTX_RESIDUAL_CARRIER_B);
	HIC_WRITE_REGISTER(HIC_REGISTER_RF_CONTROL_TX, ECL_LV1_UTI_L2P(tmpValue));

	tmpValue=0;
	HIC_READ_REGISTER(HIC_REGISTER_RF_CONTROL_RX, (UCHAR*)&tmpValue);
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 1, 0, pnq_parRX_GAIN_B);
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 3, 2, pnq_parRX_HPCF_B);
	HIC_WRITE_REGISTER(HIC_REGISTER_RF_CONTROL_RX, ECL_LV1_UTI_L2P(tmpValue));

	tmpValue=0;
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue,  7, 0, 0x7F);	//Prescaler
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 27, 8, 0x08);	//RX wait value
	HIC_WRITE_REGISTER(HIC_REGISTER_RX_WAIT_CONFIG, ECL_LV1_UTI_L2P(tmpValue));

	tmpValue=0;
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 0, 0, 0);	//Disable EMD
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 1, 1, 1);	//ERROR > THRESHOLD is not EMD
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 5, 2, 4);	//Threshold: 4 bytes
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 9, 8, 0);	//Timer 0
	HIC_WRITE_REGISTER(HIC_REGISTER_EMD_CONTROL, ECL_LV1_UTI_L2P(tmpValue));

	tmpValue=0;
	HIC_READ_REGISTER(HIC_REGISTER_SIGPRO_RM_CONFIG, (UCHAR*)&tmpValue);
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 15, 12, pnq_parRX_MIN_LEVEL_B);
	tmpValue=ECL_LV1_UTI_Set_BitRangeValue(tmpValue, 11,  8, pnq_parRX_MIN_LEVELP_B);
	HIC_WRITE_REGISTER(HIC_REGISTER_SIGPRO_RM_CONFIG, ECL_LV1_UTI_L2P(tmpValue));
}


void PNQ_REQA_Send(ULONG rcvTimeout)
{
	UCHAR cmdREQA[1]={0x26};

	PNQ_Load_Protocol_A();

	//Disable MFC Crypto
	HIC_WRITE_REGISTER_AND_MASK(HIC_REGISTER_SYSTEM_CONFIG, ECL_LV1_UTI_L2P(~HIC_SYS_CONFIG_MFC_CRYPTO_ON));

	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_A+rcvTimeout);

	//Disable CRC
	HIC_WRITE_REGISTER_AND_MASK(HIC_REGISTER_CRC_TX_CONFIG, ECL_LV1_UTI_L2P(~HIC_TX_CRC_ENABLE));
	HIC_WRITE_REGISTER_AND_MASK(HIC_REGISTER_CRC_RX_CONFIG, ECL_LV1_UTI_L2P(~HIC_RX_CRC_ENABLE));

	//Disable Parity
	HIC_WRITE_REGISTER(HIC_REGISTER_TX_CONFIG, ECL_LV1_UTI_L2P(HIC_TX_DATA_ENABLE|0x000003C0));

	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);
	HIC_SEND_DATA_BIT_ORIENTED(1, cmdREQA, 7);
}


UCHAR PNQ_REQA_Receive(UINT *rcvLen, UCHAR *rcvATQA)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PNQ_Receive_Process(rcvLen, rcvATQA);

	return rspCode;
}


void PNQ_WUPA_Send(ULONG rcvTimeout)
{
	UCHAR cmdWUPA[1]={0x52};


	PNQ_Load_Protocol_A();

	//Disable MFC Crypto
	HIC_WRITE_REGISTER_AND_MASK(HIC_REGISTER_SYSTEM_CONFIG, ECL_LV1_UTI_L2P(~HIC_SYS_CONFIG_MFC_CRYPTO_ON));

	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_A+rcvTimeout);

	//Disable CRC
	HIC_WRITE_REGISTER_AND_MASK(HIC_REGISTER_CRC_TX_CONFIG, ECL_LV1_UTI_L2P(~HIC_TX_CRC_ENABLE));
	HIC_WRITE_REGISTER_AND_MASK(HIC_REGISTER_CRC_RX_CONFIG, ECL_LV1_UTI_L2P(~HIC_RX_CRC_ENABLE));

	//Disable Parity
	HIC_WRITE_REGISTER(HIC_REGISTER_TX_CONFIG, ECL_LV1_UTI_L2P(HIC_TX_DATA_ENABLE|0x000003C0));

	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);
	HIC_SEND_DATA_BIT_ORIENTED(1, cmdWUPA, 7);
}


UCHAR PNQ_WUPA_Receive(UINT *rcvLen, UCHAR *rcvATQA)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PNQ_Receive_Process(rcvLen, rcvATQA);

	return rspCode;
}


void PNQ_ANTICOLLISION_Send(UCHAR selCL, ULONG rcvTimeout)
{
	UCHAR	cmdSelCL[2];

	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_A+rcvTimeout);
	
	//Disable CRC
	HIC_WRITE_REGISTER_AND_MASK(HIC_REGISTER_CRC_TX_CONFIG, ECL_LV1_UTI_L2P(~HIC_TX_CRC_ENABLE));
	HIC_WRITE_REGISTER_AND_MASK(HIC_REGISTER_CRC_RX_CONFIG, ECL_LV1_UTI_L2P(~HIC_RX_CRC_ENABLE));

	//Enable Parity, Transmit whole byte (Default TX_CONFIG: 0x00001FC0)
	HIC_WRITE_REGISTER(HIC_REGISTER_TX_CONFIG, ECL_LV1_UTI_L2P(HIC_TX_PARITY_TYPE_ODD|HIC_TX_PARITY_ENABLE|HIC_TX_DATA_ENABLE|0x000003C0));
	
	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);

	cmdSelCL[0]=selCL;
	cmdSelCL[1]=0x20;
	
	HIC_SEND_DATA(2, cmdSelCL);
}


UCHAR PNQ_ANTICOLLISION_Receive(UINT *rcvLen, UCHAR *rcvCLn)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PNQ_Receive_Process(rcvLen, rcvCLn);

	return rspCode;
}


void PNQ_SELECT_Send(UCHAR selCL, UCHAR *selUID, UCHAR uidBCC, ULONG rcvTimeout)
{
	UCHAR	cmdSelCL[7];
	
	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_A+rcvTimeout);
	
	//Enable CRC
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_CRC_TX_CONFIG, ECL_LV1_UTI_L2P(HIC_TX_CRC_ENABLE));
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_CRC_RX_CONFIG, ECL_LV1_UTI_L2P(HIC_RX_CRC_ENABLE));

	//Enable Parity, Transmit whole byte (Default TX_CONFIG: 0x00001FC0)
	HIC_WRITE_REGISTER(HIC_REGISTER_TX_CONFIG, ECL_LV1_UTI_L2P(HIC_TX_PARITY_TYPE_ODD|HIC_TX_PARITY_ENABLE|HIC_TX_DATA_ENABLE|0x000003C0));

	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);

	cmdSelCL[0]=selCL;
	cmdSelCL[1]=0x70;
	memcpy(&cmdSelCL[2], selUID, 4);
	cmdSelCL[6]=uidBCC;
	
	HIC_SEND_DATA(7, cmdSelCL);
}


UCHAR PNQ_SELECT_Receive(UINT *rcvLen, UCHAR *rcvSAK)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PNQ_Receive_Process(rcvLen, rcvSAK);

	return rspCode;
}


void PNQ_RATS_Send(UCHAR ratPARAM, ULONG rcvTimeout)
{
	UCHAR	cmdRATS[2];


	//Config EMD_CONTROL
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_EMD_CONTROL, ECL_LV1_UTI_L2P(HIC_EMD_ENABLE));

	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_A+rcvTimeout);

	//Enable CRC
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_CRC_TX_CONFIG, ECL_LV1_UTI_L2P(HIC_TX_CRC_ENABLE));
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_CRC_RX_CONFIG, ECL_LV1_UTI_L2P(HIC_RX_CRC_ENABLE));

	//Enable Parity, Transmit whole byte (Default TX_CONFIG: 0x00001FC0)
	HIC_WRITE_REGISTER(HIC_REGISTER_TX_CONFIG, ECL_LV1_UTI_L2P(HIC_TX_PARITY_TYPE_ODD|HIC_TX_PARITY_ENABLE|HIC_TX_DATA_ENABLE|0x000003C0));

	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);

	cmdRATS[0]=0xE0;
	cmdRATS[1]=ratPARAM;

	HIC_SEND_DATA(2, cmdRATS);
}


UCHAR PNQ_RATS_Receive(UINT *rcvLen, UCHAR *rcvATS)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PNQ_Receive_Process(rcvLen, rcvATS);

	return rspCode;
}


void PNQ_PPS_Send(ULONG rcvTimeout)
{
	//To be implemented
	rcvTimeout=rcvTimeout;
}


UCHAR PNQ_PPS_Receive(void)
{
	//To be implemented

	return ECL_LV1_SUCCESS;
}


void PNQ_HLTA_Send(ULONG rcvTimeout)
{
	UCHAR	cmdHLTA[2]={0x50,0x00};
	ULONG	intRegister=0;
	
	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_A+rcvTimeout);
	
	//Enable CRC
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_CRC_TX_CONFIG, ECL_LV1_UTI_L2P(HIC_TX_CRC_ENABLE));
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_CRC_RX_CONFIG, ECL_LV1_UTI_L2P(HIC_RX_CRC_ENABLE));

	//Enable Parity, Transmit whole byte (Default TX_CONFIG: 0x00001FC0)
	HIC_WRITE_REGISTER(HIC_REGISTER_TX_CONFIG, ECL_LV1_UTI_L2P(HIC_TX_PARITY_TYPE_ODD|HIC_TX_PARITY_ENABLE|HIC_TX_DATA_ENABLE|0x000003C0));

	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);
	HIC_SEND_DATA(2, cmdHLTA);

	//Set TX IRQ
	PNQ_Set_IRQ(ECL_LV1_UTI_L2P(HIC_IRQ_STAT_TX));

	//Wait IRQ
	do
	{
		HIC_READ_REGISTER(HIC_REGISTER_IRQ_STATUS, (UCHAR*)&intRegister);
	} while ((!(intRegister & HIC_IRQ_STAT_TX)));

	PNQ_Clear_IRQ();
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_IDLE);
}


void PNQ_REQB_Send(ULONG rcvTimeout)
{
	UCHAR cmdREQB[3]={0x05,0x00,0x00};

	PNQ_Load_Protocol_B();

	//Disable MFC Crypto
	HIC_WRITE_REGISTER_AND_MASK(HIC_REGISTER_SYSTEM_CONFIG, ECL_LV1_UTI_L2P(~HIC_SYS_CONFIG_MFC_CRYPTO_ON));

	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_B+rcvTimeout);
	
	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);
	HIC_SEND_DATA(3, cmdREQB);
}


UCHAR PNQ_REQB_Receive(UINT *rcvLen, UCHAR *rcvATQB)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PNQ_Receive_Process(rcvLen, rcvATQB);

	return rspCode;
}


void PNQ_WUPB_Send(ULONG rcvTimeout)
{
	UCHAR cmdWUPB[3]={0x05,0x00,0x08};

	PNQ_Load_Protocol_B();

	//Disable MFC Crypto
	HIC_WRITE_REGISTER_AND_MASK(HIC_REGISTER_SYSTEM_CONFIG, ECL_LV1_UTI_L2P(~HIC_SYS_CONFIG_MFC_CRYPTO_ON));

	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_B+rcvTimeout);
	
	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);
	HIC_SEND_DATA(3, cmdWUPB);
}


UCHAR PNQ_WUPB_Receive(UINT *rcvLen, UCHAR *rcvATQB)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PNQ_Receive_Process(rcvLen, rcvATQB);

	return rspCode;
}


void PNQ_ATTRIB_Send(UCHAR *cmdATTRIB, ULONG rcvTimeout)
{
	//Config EMD_CONTROL
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_EMD_CONTROL, ECL_LV1_UTI_L2P(HIC_EMD_ENABLE));

	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_B+rcvTimeout);
	
	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);
	HIC_SEND_DATA(9, cmdATTRIB);
}


UCHAR PNQ_ATTRIB_Receive(UINT *rcvLen, UCHAR *rcvATA)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PNQ_Receive_Process(rcvLen, rcvATA);

	return rspCode;
}


void PNQ_HLTB_Send(UCHAR * iptPUPI, ULONG rcvTimeout)
{
	UCHAR cmdHLTB[5];

	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_B+rcvTimeout);
	
	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);

	cmdHLTB[0]=0x50;
	memcpy(&cmdHLTB[1], iptPUPI, 4);
	HIC_SEND_DATA(5, cmdHLTB);
}


UCHAR PNQ_HLTB_Receive(UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PNQ_Receive_Process(rcvLen, rcvData);

	return rspCode;
}


void PNQ_DEP_Send(UCHAR crdType, UINT datLen, UCHAR *datBuffer, ULONG rcvTimeout)
{
	ULONG	tmrProTime;
	
	crdType=crdType;

	tmrProTime=(crdType == 'A')?(HIC_PROCESSING_TIME_A):(HIC_PROCESSING_TIME_B);

	//Configure Timer
	PNQ_Set_Timer(tmrProTime+rcvTimeout);

	//Enable CRC
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_CRC_TX_CONFIG, ECL_LV1_UTI_L2P(HIC_TX_CRC_ENABLE));
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_CRC_RX_CONFIG, ECL_LV1_UTI_L2P(HIC_RX_CRC_ENABLE));

	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);
	HIC_SEND_DATA(datLen, datBuffer);

	//Add delay for DEP receiving process to prevent EMD error
	ECL_LV1_UTI_Wait(300);
}


UCHAR PNQ_DEP_Receive(UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PNQ_Receive_Process(rcvLen, rcvData);
	
	return rspCode;
}


UCHAR PNQ_LOADKEYandAUTHENTICATION(UCHAR *iptKey, UCHAR iptAutType, UCHAR iptAddress, UCHAR * iptUID)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	rspStatus=0xFF;
	ULONG	intRegister=0;

	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_IDLE);
	rspCode=HIC_MIFARE_AUTHENTICATE(iptKey, iptAutType, iptAddress, iptUID, &rspStatus);

	PNQ_Clear_IRQ();
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_IDLE);
	
	if (rspCode == ECL_LV1_SUCCESS)
	{
		if (rspStatus == 0)	//Authentication successful
		{
			//Check SYSTEM_CONFIG register MFC_CRYPTO_ON
			rspCode=HIC_READ_REGISTER(HIC_REGISTER_SYSTEM_CONFIG, (UCHAR*)&intRegister);
			if (rspCode == ECL_LV1_SUCCESS)
			{
				if (intRegister & HIC_SYS_CONFIG_MFC_CRYPTO_ON)
				{
					return ECL_LV1_SUCCESS;
				}
			}
		}
	}

	return ECL_LV1_FAIL;
}


UCHAR PNQ_LOADKEY(UCHAR *iptKey)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	ULONG	rspRND=0;
	UCHAR	key[16];
	UCHAR	iptData[16];

	rspRND=api_sys_random(&iptData[0]);
	rspRND=api_sys_random(&iptData[8]);
	
	memcpy(&iptData[10], iptKey, 6);

	rspCode=HIC_READ_EEPROM(HIC_EEPROM_DIE_IDENTIFIER, 16, key);

	rspCode=api_aes_encipher(iptData, pnq_bufData, key, 16);

	return ECL_LV1_SUCCESS;
}


UCHAR PNQ_AUTHENTICATION(UCHAR iptAutType, UCHAR iptAddress, UCHAR *iptUID)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	key[16];
	UCHAR	optData[16];

	rspCode=HIC_READ_EEPROM(HIC_EEPROM_DIE_IDENTIFIER, 16, key);

	rspCode=api_aes_decipher(optData, pnq_bufData, key, 16);

	rspCode=PNQ_LOADKEYandAUTHENTICATION(&optData[10], iptAutType, iptAddress, iptUID);
	if (rspCode == ECL_LV1_SUCCESS)
	{
		return ECL_LV1_SUCCESS;
	}

	return ECL_LV1_FAIL;
}


UCHAR PNQ_READ(UCHAR iptAddress, UCHAR *optData)
{
	UCHAR	cmdRead[2];
	UCHAR	rspCode=ECL_LV1_FAIL;
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[16]={0};

	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_A+HIC_MIFARE_TIMEOUT_5MS);
	
	//Enable CRC
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_CRC_TX_CONFIG, ECL_LV1_UTI_L2P(HIC_TX_CRC_ENABLE));
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_CRC_RX_CONFIG, ECL_LV1_UTI_L2P(HIC_RX_CRC_ENABLE));

	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);

	cmdRead[0]=0x30;
	cmdRead[1]=iptAddress;
	HIC_SEND_DATA(2, cmdRead);

	rspCode=PNQ_Receive_Process(&rcvLen, rcvBuffer);
	if ((rspCode == ECL_LV1_SUCCESS) && (rcvLen == 16))
	{
		memcpy(optData, rcvBuffer, 16);

		return ECL_LV1_SUCCESS;
	}

	//Error handle (NAK ?)

	return ECL_LV1_FAIL;
}


UCHAR PNQ_WRITE(UCHAR iptAddress, UCHAR *iptData)
{
	UCHAR	cmdWrite[2];
	UCHAR	rspCode=ECL_LV1_FAIL;
	UINT	rcvLen=0;
	UCHAR	rcvBuffer[16]={0};
	ULONG	intRegister=0;

	//
	//	Phase 1
	//
	
	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_A+HIC_MIFARE_TIMEOUT_5MS);
	
	//Enable TX CRC, Disable RX CRC
	HIC_WRITE_REGISTER_OR_MASK(HIC_REGISTER_CRC_TX_CONFIG, ECL_LV1_UTI_L2P(HIC_TX_CRC_ENABLE));
	HIC_WRITE_REGISTER_AND_MASK(HIC_REGISTER_CRC_RX_CONFIG, ECL_LV1_UTI_L2P(~HIC_RX_CRC_ENABLE));

	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);

	cmdWrite[0]=0xA0;
	cmdWrite[1]=iptAddress;
	HIC_SEND_DATA(2, cmdWrite);

	rspCode=PNQ_Receive_Process(&rcvLen, rcvBuffer);
	if ((rspCode == ECL_LV1_SUCCESS) && (rcvLen == 1))
	{
		HIC_READ_REGISTER(HIC_REGISTER_RX_STATUS, (UCHAR*)&intRegister);
		if ((intRegister & 0x0000E000) == 0x00008000)	//RX_NUM_LAST_BITS = 4 bits
		{
			HIC_READ_DATA(rcvLen, rcvBuffer);
			if (rcvBuffer[0] != HIC_MIFARE_ACK)
			{
				return ECL_LV1_FAIL;
			}
		}
		else
		{
			return ECL_LV1_FAIL;
		}
	}
	else
	{
		return ECL_LV1_FAIL;
	}

	//
	//	Phase 2
	//
	
	//Configure Timer
	PNQ_Set_Timer(HIC_PROCESSING_TIME_A+HIC_MIFARE_TIMEOUT_10MS);
	
	//Transceive
	PNQ_Set_SystemConfig(HIC_SYS_CONFIG_COMMAND_TRANSCEIVE);
	HIC_SEND_DATA(16, iptData);

	rspCode=PNQ_Receive_Process(&rcvLen, rcvBuffer);
	if ((rspCode == ECL_LV1_SUCCESS) && (rcvLen == 1))
	{
		HIC_READ_REGISTER(HIC_REGISTER_RX_STATUS, (UCHAR*)&intRegister);
		if ((intRegister & 0x0000E000) == 0x00008000)	//RX_NUM_LAST_BITS = 4 bits
		{
			HIC_READ_DATA(1, rcvBuffer);
			if (rcvBuffer[0] == HIC_MIFARE_ACK)
			{
				return ECL_LV1_SUCCESS;
			}
		}
	}

	return ECL_LV1_FAIL;
}


UCHAR PNQ_DECREMENT(UCHAR iptAddress, UCHAR *iptValue)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PNQ_Perform_ValueBlockCommand_Phase1(0xC0, iptAddress);
	if (rspCode == ECL_LV1_SUCCESS)
	{
		rspCode=PNQ_Perform_ValueBlockCommand_Phase2(iptValue);
	}

	return rspCode;
}

UCHAR PNQ_INCREMENT(UCHAR iptAddress, UCHAR *iptValue)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PNQ_Perform_ValueBlockCommand_Phase1(0xC1, iptAddress);
	if (rspCode == ECL_LV1_SUCCESS)
	{
		rspCode=PNQ_Perform_ValueBlockCommand_Phase2(iptValue);
	}

	return rspCode;
}

UCHAR PNQ_RESTORE(UCHAR iptAddress)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	iptValue[4];	//Dummy data

	rspCode=PNQ_Perform_ValueBlockCommand_Phase1(0xC2, iptAddress);
	if (rspCode == ECL_LV1_SUCCESS)
	{
		rspCode=PNQ_Perform_ValueBlockCommand_Phase2(iptValue);
	}

	return rspCode;
}


UCHAR PNQ_TRANSFER(UCHAR iptAddress)
{
	UCHAR	rspCode=ECL_LV1_FAIL;

	rspCode=PNQ_Perform_ValueBlockCommand_Phase1(0xB0, iptAddress);

	return rspCode;
}

