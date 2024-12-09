#include <string.h>
#include "POSAPI.h"
#include "FLSAPI.h"
#include "Function.h"
#include "Define.h"


extern UCHAR VISA_MSession_Key[16];
extern UCHAR VISA_ASession_Key[16];

/*	
	[FLSID_IMEK] (16 Bytes)
	Address	Type	Index	Note
	================================
	0		01		00		IMEK

	[FLSID_IAEK] (16 Bytes)
	Address	Type	Index	Note
	================================
	0		05		00		IAEK

	[FLSID_PARA] (10K Bytes)
	Address	Type	Index	Note
	================================
	0		00		00		IMEK_MDK
	16		02		01		MEK
	32		03		01		Msession
	48		04		00		IAEK_MDK
	64		06		01		AEK
	80		07		01		Asession
*/


UCHAR FLS_Get_EncryptionKeyAddress(UCHAR iptKeyType, UCHAR iptKeyIndex, UCHAR *optID, ULONG *optAddress)
{
	UINT	keyValue=0;

	keyValue=iptKeyType*256+iptKeyIndex;

	switch (keyValue)
	{
		case 0x0000:	optID[0]=FLSID_PARA;
						optAddress[0]=FLS_ADDRESS_IMEK_MDK;	break;
		case 0x0100:	optID[0]=FLSID_IMEK;
						optAddress[0]=FLS_ADDRESS_IMEK;		break;
		case 0x0201:	optID[0]=FLSID_PARA;
						optAddress[0]=FLS_ADDRESS_MEK;		break;

		case 0x0400:	optID[0]=FLSID_PARA;
						optAddress[0]=FLS_ADDRESS_IAEK_MDK;	break;
		case 0x0500:	optID[0]=FLSID_IAEK;
						optAddress[0]=FLS_ADDRESS_IAEK;		break;	
		case 0x0601:	optID[0]=FLSID_PARA;
						optAddress[0]=FLS_ADDRESS_AEK;		break;				
						
		default:		return FAIL;
	}
	
	return SUCCESS;

}


UCHAR FLS_Read_EncryptionKey(UCHAR iptKeyType, UCHAR iptKeyIndex, UCHAR *optKey)
{
	UCHAR	flsID=0;
	ULONG	keyAddress=0;
	UCHAR	rspCode=FAIL;
	ULONG	rspCodeAPI=apiFailed;

	if(	(iptKeyType == 0x00) || (iptKeyType == 0x04) ||
		(iptKeyType == 0x01) || (iptKeyType == 0x05) || 
		(iptKeyType == 0x02) || (iptKeyType == 0x06) )	//IMEKmdk, IAEKmdk, IMEK, IAEK, MEK, AEK
	{
		rspCode=FLS_Get_EncryptionKeyAddress(iptKeyType, iptKeyIndex, &flsID, &keyAddress);
		if (rspCode == SUCCESS)
		{	
#ifdef _PLATFORM_AS350
			rspCodeAPI = apiOK;			
#else
			//20140207 for IMEK IAEK change memtype to Flash
			if((flsID == FLSID_IMEK) || (flsID == FLSID_IAEK))
				rspCodeAPI = api_fls_memtype(flsID,FLSType_Flash);
			else	//else memtype to Internal SRAM
				rspCodeAPI = api_fls_memtype(flsID,FLSType_SRAM);
#endif			
			//20140105 change memtype
			//rspCodeAPI = api_fls_memtype(flsID,FLSType_SRAM);	//20140207 removed
			if(rspCodeAPI == apiOK)
			{
				rspCodeAPI=api_fls_read(flsID, keyAddress, 16, optKey);

#ifdef _PLATFORM_AS350_LITE
				api_fls_memtype(flsID, FLSType_Flash);	//Reset to Flash
#endif
				if (rspCodeAPI == apiOK)
					return SUCCESS;
				else
					return FAIL;
			}
			else
				return FAIL;		
		}
	}
	else if(iptKeyType == 0x03)		//Msession
	{
		memcpy(optKey,VISA_MSession_Key,16);
		return SUCCESS;
	}
	else if(iptKeyType == 0x07)		//Asession
	{
		memcpy(optKey,VISA_ASession_Key,16);
		return SUCCESS;
	}
	else							//other Case
	{
		return FAIL;
	}
	
	return FAIL;
}

UCHAR FLS_Write_EncryptionKey(UCHAR iptKeyType, UCHAR iptKeyIndex, UCHAR *iptKey)
{
	UCHAR	flsID=0;
	ULONG	keyAddress=0;
	UCHAR	rspCode=FAIL;
	ULONG	rspCodeAPI=apiFailed;
	UCHAR	tmpKey[16];

	if(	(iptKeyType == 0x00) || (iptKeyType == 0x04) ||
		(iptKeyType == 0x01) || (iptKeyType == 0x05) || 
		(iptKeyType == 0x02) || (iptKeyType == 0x06) )	//IMEKmdk, IAEKmdk, IMEK, IAEK, MEK, AEK
	{	
		rspCode=FLS_Get_EncryptionKeyAddress(iptKeyType, iptKeyIndex, &flsID, &keyAddress);
		if (rspCode == SUCCESS)
		{					
#ifdef _PLATFORM_AS350
			rspCodeAPI = apiOK;			
#else
			//20140207 for IMEK IAEK change memtype to Flash
			if((flsID == FLSID_IMEK) || (flsID == FLSID_IAEK))
				rspCodeAPI = api_fls_memtype(flsID,FLSType_Flash);
			else	//else memtype to Internal SRAM
				rspCodeAPI = api_fls_memtype(flsID,FLSType_SRAM);
#endif

 			if (rspCodeAPI == apiOK)
			{
 				rspCodeAPI=api_fls_read(flsID, keyAddress, 16, tmpKey);
				if ((rspCodeAPI == apiOK) && (memcmp(tmpKey, iptKey, 16)))
				{
					rspCodeAPI=api_fls_write(flsID, keyAddress, 16, iptKey);
				}
				
#ifdef _PLATFORM_AS350_LITE
				api_fls_memtype(flsID, FLSType_Flash);	//Reset to Flash
#endif

				if (rspCodeAPI == apiOK)
				{
					return SUCCESS;
				}
				else
				{
					return FAIL;
				}
				
			}
			else
			{
				return FAIL;
			}			
		}		
	}
	else if(iptKeyType == 0x03)		//Msession
	{
		memcpy(VISA_MSession_Key,iptKey,16);
		return SUCCESS;
	}
	else if(iptKeyType == 0x07)		//Asession
	{
		memcpy(VISA_ASession_Key,iptKey,16);
		return SUCCESS;
	}
	else							//other case
	{
		return FAIL;
	}

	return FAIL;
}

UCHAR FLS_Read_PayPassConfigurationData(UINT *optLength, UCHAR *optData)
{
	UCHAR rspCode=apiOK;
	UCHAR cfgLen[2]={0};

	//Configuration Data = Data Length(2) + Data(x)

#ifdef _PLATFORM_AS210	
	rspCode=api_fls_memtype(FLSID_PARA, FLSType_SRAM);
#endif

	if (rspCode == apiOK)
	{
		rspCode=api_fls_read(FLSID_PARA, FLS_ADDRESS_CONFIGURATION_DATA, 2, cfgLen);
		if ((rspCode == apiOK) && (UT_C2S(&cfgLen[0]) <= (FLS_SIZE_CONFIGURATION_DATA-2)))
		{
			rspCode=api_fls_read(FLSID_PARA, (FLS_ADDRESS_CONFIGURATION_DATA+2), UT_C2S(&cfgLen[0]), optData);
			if (rspCode == apiOK)
			{
				optLength[0]=UT_C2S(&cfgLen[0]);
				return SUCCESS;
			}
		}
	}

	return FAIL;
}

UCHAR FLS_Write_PayPassConfigurationData(UINT iptLength, UCHAR *iptData)
{
	UCHAR rspCode=apiOK;
	UCHAR tmpBuffer[FLS_SIZE_CONFIGURATION_DATA]={0};

#ifdef _PLATFORM_AS210	
	rspCode=api_fls_memtype(FLSID_PARA, FLSType_SRAM);
#endif

	if (rspCode == apiOK)
	{
		if (iptLength <= (FLS_SIZE_CONFIGURATION_DATA-2))
		{
			UT_S2C(iptLength, &tmpBuffer[0]);
			memcpy(&tmpBuffer[2], iptData, iptLength);
			
			rspCode=api_fls_write(FLSID_PARA, FLS_ADDRESS_CONFIGURATION_DATA, (2+iptLength), tmpBuffer);
			if (rspCode == apiOK)
			{
				return SUCCESS;
			}
		}
	}

	return FAIL;
}

UCHAR FLS_Read_QuickPassMultipleAIDData(UINT *optLength, UCHAR *optData)
{
	UCHAR rspCode=apiOK;
	UCHAR datLen[2]={0};

	//Data = Data Length(2) + Data(x)

#ifdef _PLATFORM_AS210	
	rspCode=api_fls_memtype(FLSID_PARA, FLSType_SRAM);
#endif

	if (rspCode == apiOK)
	{
		rspCode=api_fls_read(FLSID_PARA, FLS_ADDRESS_MULTIAID_DATA, 2, datLen);
		if ((rspCode == apiOK) && (UT_C2S(&datLen[0]) <= (FLS_SIZE_MULTIAID_DATA-2)))
		{
			rspCode=api_fls_read(FLSID_PARA, (FLS_ADDRESS_MULTIAID_DATA+2), UT_C2S(&datLen[0]), optData);
			if (rspCode == apiOK)
			{
				optLength[0]=UT_C2S(&datLen[0]);
				return SUCCESS;
			}
		}
	}

	return FAIL;
}

UCHAR FLS_Write_QuickPassMultipleAIDData(UINT iptLength, UCHAR *iptData)
{
	UCHAR rspCode=apiOK;
	UCHAR tmpBuffer[FLS_SIZE_MULTIAID_DATA]={0};

#ifdef _PLATFORM_AS210	
	rspCode=api_fls_memtype(FLSID_PARA, FLSType_SRAM);
#endif

	if (rspCode == apiOK)
	{
		if (iptLength <= (FLS_SIZE_MULTIAID_DATA-2))
		{
			UT_S2C(iptLength, &tmpBuffer[0]);
			memcpy(&tmpBuffer[2], iptData, iptLength);
			
			rspCode=api_fls_write(FLSID_PARA, FLS_ADDRESS_MULTIAID_DATA, (2+iptLength), tmpBuffer);
			if (rspCode == apiOK)
			{
				return SUCCESS;
			}
		}
	}

	return FAIL;
}

UCHAR FLS_Read_DPASMultipleAIDData(UINT *optLength, UCHAR *optData)
{
	UCHAR rspCode=apiOK;
	UCHAR datLen[2]={0};

	//Data = Data Length(2) + Data(x)

#ifdef _PLATFORM_AS210	
	rspCode=api_fls_memtype(FLSID_PARA, FLSType_SRAM);
#endif

	if (rspCode == apiOK)
	{
		rspCode=api_fls_read(FLSID_PARA, FLS_ADDRESS_MULTIAID_DATA_DPAS, 2, datLen);
		if ((rspCode == apiOK) && (UT_C2S(&datLen[0]) <= (FLS_SIZE_MULTIAID_DATA_DPAS-2)))
		{
			rspCode=api_fls_read(FLSID_PARA, (FLS_ADDRESS_MULTIAID_DATA_DPAS+2), UT_C2S(&datLen[0]), optData);
			if (rspCode == apiOK)
			{
				optLength[0]=UT_C2S(&datLen[0]);
				return SUCCESS;
			}
		}
	}

	return FAIL;
}

UCHAR FLS_Write_DPASMultipleAIDData(UINT iptLength, UCHAR *iptData)
{
	UCHAR rspCode=apiOK;
	UCHAR tmpBuffer[FLS_SIZE_MULTIAID_DATA_DPAS]={0};

#ifdef _PLATFORM_AS210	
	rspCode=api_fls_memtype(FLSID_PARA, FLSType_SRAM);
#endif

	if (rspCode == apiOK)
	{
		if (iptLength <= (FLS_SIZE_MULTIAID_DATA_DPAS-2))
		{
			UT_S2C(iptLength, &tmpBuffer[0]);
			memcpy(&tmpBuffer[2], iptData, iptLength);
			
			rspCode=api_fls_write(FLSID_PARA, FLS_ADDRESS_MULTIAID_DATA_DPAS, (2+iptLength), tmpBuffer);
			if (rspCode == apiOK)
			{
				return SUCCESS;
			}
		}
	}

	return FAIL;
}

