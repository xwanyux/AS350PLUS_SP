/*
 *	Host interface commands for PN5180
 */

#include <string.h>
#include "POSAPI.h"
#include "ECL_LV1_Define.h"
#include "HIC_Define.h"
#include "HIC_Function.h"
#include "SPI_Function.h"


UCHAR HIC_Transmit_SPI(ULONG datLen, UCHAR *datBuffer)
{
	UCHAR	rspCode=0;
	UINT	datBuffer_UINT[HIC_BUFFER_MAXIMUM_SEND];
	UINT	i;
	
	if (datLen > HIC_BUFFER_MAXIMUM_SEND)
	{
		printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
		return ECL_LV1_FAIL;
	}

	// for (i=0; i<datLen; i++)
	// {
	// 	datBuffer_UINT[i]=(datBuffer[i]<<8);
	// }
	
	rspCode=SPI_Transmit_DebugMode(datLen, datBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
		return ECL_LV1_FAIL;
	}
	
	for (i=0; i<datLen; i++)
	{
		datBuffer[i]=(UCHAR)datBuffer_UINT[i];
	}
	printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_LOAD_RF_CONFIG(UCHAR iptConTX, UCHAR iptConRX)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[3];

	cmdBuffer[0]=HIC_COMMAND_LOAD_RF_CONFIG;
	cmdBuffer[1]=iptConTX;
	cmdBuffer[2]=iptConRX;
	
	rspCode=HIC_Transmit_SPI(3, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//No response

	return ECL_LV1_SUCCESS;
}


UCHAR HIC_MIFARE_AUTHENTICATE(UCHAR * iptKey, UCHAR iptAutType, UCHAR iptBlkAddress, UCHAR * iptUID, UCHAR * optStatus)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[13];

	cmdBuffer[0]=HIC_COMMAND_MIFARE_AUTHENTICATE;
	memcpy(&cmdBuffer[1], iptKey, 6);
	cmdBuffer[7]=iptAutType;
	cmdBuffer[8]=iptBlkAddress;
	memcpy(&cmdBuffer[9], iptUID, 4);
	
	rspCode=HIC_Transmit_SPI(13, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	rspCode=HIC_Transmit_SPI(1, optStatus);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_READ_DATA(UINT iptLen, UCHAR *optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[2];

	if (iptLen > HIC_BUFFER_MAXIMUM_RECEIVE)
	{
		return ECL_LV1_FAIL;
	}

	cmdBuffer[0]=HIC_COMMAND_READ_DATA;
	cmdBuffer[1]=0x00;
	
	rspCode=HIC_Transmit_SPI(2, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	rspCode=HIC_Transmit_SPI((ULONG)iptLen, optData);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_READ_EEPROM(UCHAR iptAddEEPROM, UINT iptLen, UCHAR *optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[4];

	if (iptLen > HIC_BUFFER_MAXIMUM_EEPROM)
	{
		return ECL_LV1_FAIL;
	}
	cmdBuffer[0]=HIC_COMMAND_READ_EEPROM;
	cmdBuffer[1]=iptAddEEPROM;
	cmdBuffer[2]=(UCHAR)iptLen;
	rspCode=HIC_Transmit_SPI(3, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
		return ECL_LV1_FAIL;
	}

	// rspCode=HIC_Transmit_SPI((ULONG)iptLen, optData);
	// if (rspCode == ECL_LV1_FAIL)
	// {
	// 	printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
	// 	return ECL_LV1_FAIL;
	// }
	// printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_READ_REGISTER(UCHAR iptAddRegister, UCHAR *optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[2];

	cmdBuffer[0]=HIC_COMMAND_READ_REGISTER;
	cmdBuffer[1]=iptAddRegister;
	
	rspCode=HIC_Transmit_SPI(2, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	rspCode=HIC_Transmit_SPI(4, optData);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_READ_REGISTER_MULTIPLE(UINT iptLen, UCHAR *iptData, UINT *optLen, UCHAR *optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[19];

	if ((iptLen == 0) || (iptLen > 18))
	{
		return ECL_LV1_FAIL;
	}

	cmdBuffer[0]=HIC_COMMAND_READ_REGISTER_MULTIPLE;
	memcpy(&cmdBuffer[1], iptData, iptLen);
	
	rspCode=HIC_Transmit_SPI(19, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	rspCode=HIC_Transmit_SPI((iptLen*4), optData);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	optLen[0]=iptLen*4;
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_RF_ON(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[2];

	cmdBuffer[0]=HIC_COMMAND_RF_ON;
	cmdBuffer[1]=0x00;
	
	rspCode=HIC_Transmit_SPI(2, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//No response
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_RF_OFF(void)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[2];

	cmdBuffer[0]=HIC_COMMAND_RF_OFF;
	cmdBuffer[1]=0x00;
	
	rspCode=HIC_Transmit_SPI(2, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//No response
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_SEND_DATA_BIT_ORIENTED(UINT iptLen, UCHAR *iptData, UCHAR iptBitsValid)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[2+HIC_BUFFER_MAXIMUM_SEND];

	if (iptLen > HIC_BUFFER_MAXIMUM_SEND)
	{
		return ECL_LV1_FAIL;
	}

	cmdBuffer[0]=HIC_COMMAND_SEND_DATA;
	cmdBuffer[1]=iptBitsValid;
	memcpy(&cmdBuffer[2], iptData, iptLen);
	
	rspCode=HIC_Transmit_SPI((2+iptLen), cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//No response
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_SEND_DATA(UINT iptLen, UCHAR *iptData)
{
	return (HIC_SEND_DATA_BIT_ORIENTED(iptLen, iptData, 0));
}


UCHAR HIC_UPDATE_RF_CONFIG(UINT iptLen, UCHAR *iptData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[1+HIC_BUFFER_MAXIMUM_UPDATE_RF];

	if ((iptLen < 6) || (iptLen > HIC_BUFFER_MAXIMUM_UPDATE_RF))
	{
		return ECL_LV1_FAIL;
	}

	if ((iptLen % 6) != 0)
	{
		return ECL_LV1_FAIL;
	}

	cmdBuffer[0]=HIC_COMMAND_UPDATE_RF_CONFIG;
	memcpy(&cmdBuffer[1], iptData, iptLen);
	
	rspCode=HIC_Transmit_SPI((1+iptLen), cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//No response
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_WRITE_EEPROM(UCHAR iptAddEEPROM, UINT iptLen, UCHAR *iptData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[2+HIC_BUFFER_MAXIMUM_EEPROM];

	if (iptLen > HIC_BUFFER_MAXIMUM_EEPROM)
	{
		return ECL_LV1_FAIL;
	}

	cmdBuffer[0]=HIC_COMMAND_WRITE_EEPROM;
	cmdBuffer[1]=iptAddEEPROM;
	memcpy(&cmdBuffer[2], iptData, iptLen);

	rspCode=HIC_Transmit_SPI((2+iptLen), cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//No response

	return ECL_LV1_SUCCESS;
}


UCHAR HIC_WRITE_REGISTER(UCHAR iptAddRegister, UCHAR *iptData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[6];

	cmdBuffer[0]=HIC_COMMAND_WRITE_REGISTER;
	cmdBuffer[1]=iptAddRegister;
	memcpy(&cmdBuffer[2], iptData, 4);
	
	rspCode=HIC_Transmit_SPI(6, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//No response
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_WRITE_REGISTER_OR_MASK(UCHAR iptAddRegister, UCHAR *iptData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[6];

	cmdBuffer[0]=HIC_COMMAND_WRITE_REGISTER_OR_MASK;
	cmdBuffer[1]=iptAddRegister;
	memcpy(&cmdBuffer[2], iptData, 4);
	
	rspCode=HIC_Transmit_SPI(6, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//No response
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_WRITE_REGISTER_AND_MASK(UCHAR iptAddRegister, UCHAR *iptData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[6];

	cmdBuffer[0]=HIC_COMMAND_WRITE_REGISTER_AND_MASK;
	cmdBuffer[1]=iptAddRegister;
	memcpy(&cmdBuffer[2], iptData, 4);
	
	rspCode=HIC_Transmit_SPI(6, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//No response
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_WRITE_REGISTER_MULTIPLE(UINT iptLen, UCHAR *iptData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[1+HIC_BUFFER_MAXIMUM_WRITE_MULTIPLE];

	if ((iptLen < 1) || (iptLen > HIC_BUFFER_MAXIMUM_WRITE_MULTIPLE))
	{
		return ECL_LV1_FAIL;
	}

	if ((iptLen % 6) != 0)
	{
		return ECL_LV1_FAIL;
	}

	cmdBuffer[0]=HIC_COMMAND_WRITE_REGISTER_MULTIPLE;
	memcpy(&cmdBuffer[1], iptData, iptLen);
	
	rspCode=HIC_Transmit_SPI((1+iptLen), cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//No response
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_WRITE_TX_DATA(UINT iptLen, UCHAR *iptData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[1+HIC_BUFFER_MAXIMUM_SEND];

	if ((iptLen < 1) || (iptLen > HIC_BUFFER_MAXIMUM_SEND))
	{
		return ECL_LV1_FAIL;
	}

	cmdBuffer[0]=HIC_COMMAND_WRITE_TX_DATA;
	memcpy(&cmdBuffer[1], iptData, iptLen);
	
	rspCode=HIC_Transmit_SPI((1+iptLen), cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//No response
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_RETRIEVE_RF_CONFIG_SIZE(UCHAR iptID, UCHAR *optNumRegister)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[2];

	cmdBuffer[0]=HIC_COMMAND_RETRIEVE_RF_CONFIG_SIZE;
	cmdBuffer[1]=iptID;
	
	rspCode=HIC_Transmit_SPI(2, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	rspCode=HIC_Transmit_SPI(1, optNumRegister);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_RETRIEVE_RF_CONFIG_PARAMETERS(UCHAR iptID, UCHAR iptNumRegister, UCHAR *optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[2];

	cmdBuffer[0]=HIC_COMMAND_RETRIEVE_RF_CONFIG;
	cmdBuffer[1]=iptID;
	
	rspCode=HIC_Transmit_SPI(2, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	rspCode=HIC_Transmit_SPI((ULONG)iptNumRegister*5, optData);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}
	
	return ECL_LV1_SUCCESS;
}


UCHAR HIC_CONFIGURE_TESTBUS_ANALOG(UCHAR iptPara1, UCHAR iptPara2)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cmdBuffer[1+HIC_BUFFER_MAXIMUM_SEND];


	cmdBuffer[0]=HIC_COMMAND_CONFIGURE_TESTBUS_ANALOG;
	cmdBuffer[1]=iptPara1;
	cmdBuffer[2]=iptPara2;
	
	rspCode=HIC_Transmit_SPI(3, cmdBuffer);
	if (rspCode == ECL_LV1_FAIL)
	{
		return ECL_LV1_FAIL;
	}

	//No response
	
	return ECL_LV1_SUCCESS;
}
