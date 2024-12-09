#include <string.h>
#include "Define.h"
#include "Function.h"
#include "Glv_ReaderConfPara.h"
#include "POSAPI.h"
#include "UTILS.h"
#include "FLSAPI.h"
#include "api_pcd_vap_Command.h"
#include "MIFARE_Define.h"
#include "VAP_ReaderInterface_Define.h"
#include "JCB_Kernel_Define.h"
#include "ECL_LV1_Function.h"

#ifdef _PLATFORM_AS210
#include "xioapi.h"
#else
#include "LCDTFTAPI.h"
#endif


//20140107
extern volatile UCHAR os_WAVE_TXI_H;
extern volatile UCHAR os_WAVE_TXI_L;

//20140106
extern UINT		etp_tmrSale;
extern UCHAR	etp_flgMtiAID_UPI;
extern UCHAR	etp_flgMtiAID_Discover;

//20140105
UCHAR Main_VAP_POLL_A_Flag = FALSE;

extern UCHAR Mifare_File_Download_Flag;

//display control
UCHAR main_clean_screen = FALSE;
UCHAR main_card_remove = FALSE;
UCHAR main_txn_complete = FALSE;


//LCD
//Test LCD
UCHAR dhn_lcd;


UCHAR dhn_ICC;

//Buzzer Flag
UCHAR app_flgBuzzer=FALSE;

//Polling Timer
ULONG Timer_Poll_1 = 0, Timer_Poll_2 = 0;


#ifdef _PLATFORM_AS350_LITE
#ifdef _ACQUIRER_NCCC
UCHAR	app_flgActivated=FALSE;
UCHAR	app_flgStaUpdate=FALSE;
#endif
#endif


extern UCHAR vap_rif_flgMifare;


extern ULONG OS_GET_SysTimerFreeCnt( void );

//api_pcd_vap_Command.c
extern UCHAR api_pcd_vap_flgMutAuthenticate;
extern UCHAR api_pcd_vap_flgDbgAndOptMode;
extern UCHAR api_pcd_vap_flgAdmMode;
extern UCHAR api_pcd_vap_Set_Parameter(UCHAR * iptIndex,UCHAR * iptLength,UCHAR * iptData);
extern UCHAR api_pcd_vap_Set_EMVTag(UCHAR *iptTLV);
extern UCHAR api_pcd_vap_Set_DisplayMessage(UCHAR iptMsgID,UCHAR iptMsgLength,UCHAR * iptMessage);
extern UCHAR api_pcd_vap_Check_PayPassConfigurationData(UINT iptLen, UCHAR * iptData);
extern UCHAR api_pcd_vap_Check_MultipleAIDData(UINT iptLen, UCHAR * iptData, UCHAR iptMaxSet);
extern UCHAR api_pcd_vap_Set_QuickPayMultipleAIDData(UINT iptLen, UCHAR * iptData);
extern UCHAR api_pcd_vap_Set_DPASMultipleAIDData(UINT iptLen, UCHAR * iptData);

extern void	 MFC_lcdtft_showSymLinkLogo( void );

#ifdef _PLATFORM_AS350_LITE

#ifdef _ACQUIRER_NCCC
extern void	NCCC_Lite_InputData(void);
extern void	NCCC_Lite_ShowInfo(UCHAR mode, UCHAR * optData);
#endif

#ifdef _ACQUIRER_FISC
extern UCHAR PINPAD_load_key_function( void );
#endif

#endif


#ifdef _PLATFORM_AS350_LITE

#ifdef _ACQUIRER_NCCC

UCHAR APP_Set_NCCC_Parameter(UCHAR * iptData)
{
	UCHAR	rspCode=apiFailed;
	
	rspCode=api_fls_write(FLSID_PARA, FLS_ADDRESS_NCCC_PARAMETER, 22, iptData);
	if (rspCode == apiOK)
	{
		return SUCCESS;
	}

	return FAIL;
}


UCHAR APP_Get_NCCC_Parameter(UCHAR * optData)
{
	UCHAR	rspCode=apiFailed;
	
	rspCode=api_fls_read(FLSID_PARA, FLS_ADDRESS_NCCC_PARAMETER, 22, optData);
	if (rspCode == apiOK)
	{
		return SUCCESS;
	}

	return FAIL;
}


UCHAR APP_Check_NCCC_Activation(void)
{
	UCHAR	flsBuffer[1];
	UCHAR	rspCode;
	
	rspCode=api_fls_read(FLSID_PARA, FLS_ADDRESS_NCCC_PARAMETER ,1 ,flsBuffer);
	if (rspCode == apiOK)
	{
		if (flsBuffer[0] == 2)	//Activated
		{
			return TRUE;
		}
	}
	
	return FALSE;
}


void APP_Check_NCCC_Function(void)
{
	ULONG	tmrStart;
	UCHAR	getKey;
	UCHAR	optBuffer[1500];

	if (api_pcd_vap_flgMutAuthenticate == FALSE)
	{
		if (UT_GetKey() == KEY_OK)
		{
			tmrStart=OS_GET_SysTimerFreeCnt();

			do
			{
				getKey=UT_GetKey();
				if (getKey == KEY_5)
				{
					NCCC_Lite_InputData();
					MFC_lcdtft_showSymLinkLogo();
					app_flgActivated=APP_Check_NCCC_Activation();
					app_flgStaUpdate=TRUE;
					break;
				}
				else if (getKey == KEY_7)
				{
					NCCC_Lite_ShowInfo(0, optBuffer);
					MFC_lcdtft_showSymLinkLogo();
					break;
				}
				else if (getKey == FAIL)
				{
					continue;
				}
				else
				{
					break;
				}
			} while ((OS_GET_SysTimerFreeCnt() - tmrStart) < 300);
		}
	}
}


void APP_Show_NCCC_ActivationStatus(void)
{
	if (api_pcd_vap_flgMutAuthenticate == FALSE)
	{
		if (app_flgStaUpdate == TRUE)
		{
			if (app_flgActivated == FALSE)
			{
				UT_PutStr(2, (Display_MAX_Num-10)/2, FONT2, 10, (UCHAR*)"�]�ƥ��ҥ�");
			}
			else
			{
				UT_PutStr(2, (Display_MAX_Num-10)/2, FONT2, 10, (UCHAR*)"          ");
			}

			app_flgStaUpdate=FALSE;
		}
	}
}
#endif

#ifdef _ACQUIRER_FISC
void APP_Check_FISC_Function(void)
{
	if ( PINPAD_load_key_function() )
	{
		MFC_lcdtft_showSymLinkLogo();
	}
}
#endif

#endif


void APP_SetDefaultEMVTagsProcess(void)
{
	UINT i;
	
	//WAVE 2 Tag
	UCHAR Default_Terminal_Type_VISAAP[ETP_PARA_SIZE_9F35] 	= {0x01};
	UCHAR Default_CVM_Requirement[ETP_PARA_SIZE_DF04]	= {0x01};
	
	//WAVE 2 & 3 Tag
	UCHAR Default_Terminal_Country_Code[ETP_PARA_SIZE_9F1A] = {0x01,0x58};						//20140808 change to 0158
	UCHAR Default_Txn_Corrency_Code[ETP_PARA_SIZE_5F2A] 	= {0x09,0x01};						//20140808 change to 0901
	UCHAR Default_Txn_Type[ETP_PARA_SIZE_9C]				= {0x00};
	UCHAR Default_CL_Txn_Limit[ETP_PARA_SIZE_DF00]			= {0x99,0x99,0x99,0x99,0x99,0x99};	//20140808 change the default value to 150
	UCHAR Default_Rdr_CVM_Required_Limit[ETP_PARA_SIZE_DF01]= {0x00,0x00,0x00,0x00,0x00,0x00};	//20140808 change the default value to 00
	UCHAR Default_Rdr_CL_Floor_Limit[ETP_PARA_SIZE_DF02]	= {0x00,0x00,0x00,0x00,0x00,0x00};	//20140808 change the default value to 00
	UCHAR Default_Enhance_DDA_Ver_Num[ETP_PARA_SIZE_DF03]	= {0x00};
	
	//WAVE 3 Tag
	UCHAR Default_Display_Offline_Funds[ETP_PARA_SIZE_DF05] = {0x00};
	UCHAR Default_payWaveTTQ[ETP_PARA_SIZE_9F66]			= {0xA6,0x00,0xC0,0x00};
	UCHAR Default_Reader_Managerment_Check[ETP_PARA_SIZE_DF06] = {0x1C,0x00};	//20140205 changed to 0x1C, 0x00

	//New JCB Kernel 5
	UCHAR DefaultTerminalType = 0x22;
	UCHAR DefaultCombineOption[2]={0x7B,0x00};
	UCHAR DefaultTIP[3]={0x70,0x80,0x00};
	UCHAR DefaultBRSMAXTar = 0x00;
	UCHAR DefaultBRSTar = 0x00;
	UCHAR DefaultBRSThreshold[6] = {0x00,0x00,0x00,0x00,0x20,0x00};
	UCHAR DefaultTACDefault[5] = {0x90,0x40,0x00,0x80,0x00};
	UCHAR DefaultTACDenial[5] = {0x04,0x10,0x00,0x00,0x00};
	UCHAR DefaultTACOnline[5] = {0x90,0x60,0x00,0x90,0x00};

	//AE
	UCHAR Default_Terminal_Application_Ver_Num_AE[ETP_PARA_SIZE_9F09]	= {0x00,0x01};

	//D-PAS
	UCHAR Default_Terminal_Application_Ver_Num_DPAS[ETP_PARA_SIZE_9F09]	= {0x01,0x00};

	//qUICS
	UCHAR Default_qUICSTTQ[ETP_PARA_SIZE_9F66] = {0x26,0x00,0x00,0x80};

	//Gerneral
	UCHAR Default_Terminal_Type[ETP_PARA_SIZE_9F35] 				= {0x22};
	UCHAR Default_Terminal_Floor_Limit[ETP_PARA_SIZE_9F1B]			= {0x00};
	
	memcpy(&glv_par9F35[0],&Default_Terminal_Type_VISAAP[0],ETP_PARA_SIZE_9F35);	//Terminal Type
	memcpy(&glv_par9F35[3],&Default_Terminal_Type[0],ETP_PARA_SIZE_9F35);
	memcpy(&glv_par9F35[5],&Default_Terminal_Type[0],ETP_PARA_SIZE_9F35);
	memcpy(&glv_par9F09[3],&Default_Terminal_Application_Ver_Num_AE, ETP_PARA_SIZE_9F09);	//Terminal Application Version Number
	memcpy(&glv_par9F09[5],&Default_Terminal_Application_Ver_Num_DPAS, ETP_PARA_SIZE_9F09);
	memcpy(&glv_par9F1A,&Default_Terminal_Country_Code,ETP_PARA_SIZE_9F1A);	//Terminal Country Code
	memcpy(&glv_par5F2A,&Default_Txn_Corrency_Code,ETP_PARA_SIZE_5F2A);		//Transaction Currency Code
	memcpy(&glv_par9C,&Default_Txn_Type,ETP_PARA_SIZE_9C);					//Transaction Type

	for(i=0;i<ETP_NUMBER_COMBINATION;i++)
	{
		glv_parDF00Len[i]=ETP_PARA_SIZE_DF00;
		glv_parDF01Len[i]=ETP_PARA_SIZE_DF01;
		glv_parDF02Len[i]=ETP_PARA_SIZE_DF02;
		
		if(i!=2)
		{
			memcpy(&glv_parDF00[i],&Default_CL_Txn_Limit,ETP_PARA_SIZE_DF00);				//Reader CL Transaction Limit
			memcpy(&glv_parDF01[i],&Default_Rdr_CVM_Required_Limit,ETP_PARA_SIZE_DF01);		//Reader CVM Required Limit
			memcpy(&glv_parDF02[i],&Default_Rdr_CL_Floor_Limit,ETP_PARA_SIZE_DF02);			//Reader CL Floor Limit
			memcpy(&glv_parDF06[i],&Default_Reader_Managerment_Check,ETP_PARA_SIZE_DF06);	//Reader Configuration Parameter
			memcpy(&glv_par9F1B[i],&Default_Terminal_Floor_Limit, ETP_PARA_SIZE_9F1B);		//Terminal Floor Limit
		}
		else
		{
			memcpy(&glv_parDF00_VISA[VISA_Purchase_Mode][0],&Default_CL_Txn_Limit,ETP_PARA_SIZE_DF00);				//Reader CL Transaction Limit
			memcpy(&glv_parDF01_VISA[VISA_Purchase_Mode][0],&Default_Rdr_CVM_Required_Limit,ETP_PARA_SIZE_DF01);	//Reader CVM Required Limit
			memcpy(&glv_parDF02_VISA[VISA_Purchase_Mode][0],&Default_Rdr_CL_Floor_Limit,ETP_PARA_SIZE_DF02);		//Reader CL Floor Limit
			memcpy(&glv_parDF06_VISA[VISA_Purchase_Mode][0],&Default_Reader_Managerment_Check,ETP_PARA_SIZE_DF06);	//Reader Configuration Parameter
			memcpy(&glv_par9F1B_VISA[VISA_Purchase_Mode][0],&Default_Terminal_Floor_Limit,ETP_PARA_SIZE_9F1B);		//Terminal Floor Limit
		}

		memcpy(&glv_parDF03[i],&Default_Enhance_DDA_Ver_Num,ETP_PARA_SIZE_DF03);		//Enhanced DDA Version Number
		memcpy(&glv_parDF04[i],&Default_CVM_Requirement,ETP_PARA_SIZE_DF04); 			//CVM Required
		memcpy(&glv_parDF05[i],&Default_Display_Offline_Funds,ETP_PARA_SIZE_DF05);		//Display Offline Available Fund
		
	}
	
	memcpy(&glv_par9F66[0],&Default_payWaveTTQ,ETP_PARA_SIZE_9F66);		//payWAVE TTQ
	memcpy(&glv_par9F66[2],&Default_payWaveTTQ,ETP_PARA_SIZE_9F66);	
	memcpy(&glv_par9F66[6],&Default_qUICSTTQ,ETP_PARA_SIZE_9F66);

	//JCB Kernel 5
	JCB_Static_Parameter.ClessTxnLimitLen = 6;
	memcpy(JCB_Static_Parameter.ClessTxnLimit,Default_CL_Txn_Limit,6);
	JCB_Static_Parameter.ClessFloorLimitLen = 6;
	memcpy(JCB_Static_Parameter.ClessFloorLimit,Default_Rdr_CL_Floor_Limit,6);
	JCB_Static_Parameter.ClessCVMLimitLen = 6;
	memcpy(JCB_Static_Parameter.ClessCVMLimit,Default_Rdr_CVM_Required_Limit,6);				
	JCB_Static_Parameter.TerminalType = DefaultTerminalType;
	memcpy(JCB_Static_Parameter.CombinationOption,DefaultCombineOption,2);
	memcpy(JCB_Static_Parameter.TerminalInterchangeProfile,DefaultTIP,3);
	JCB_Static_Parameter.MAXTargetPercent_RandomSelection = DefaultBRSMAXTar;
	JCB_Static_Parameter.TargetPercent_RandomSelection = DefaultBRSTar;
	memcpy(JCB_Static_Parameter.ThresholdValue_RandomSelection,DefaultBRSThreshold,6);
	JCB_Static_Parameter.TAC_Default_Present = TRUE;
	memcpy(JCB_Static_Parameter.TAC_Default,DefaultTACDefault,5);
	JCB_Static_Parameter.TAC_Denial_Present= TRUE;
	memcpy(JCB_Static_Parameter.TAC_Denial,DefaultTACDenial,5);
	JCB_Static_Parameter.TAC_Online_Present = TRUE;
	memcpy(JCB_Static_Parameter.TAC_Online,DefaultTACOnline,5);
}


void APP_Set_Default_PayPassCfgData(void)
{
	UCHAR rspCode=0xFF;
	UCHAR flgLodDefault=FALSE;
	UINT  datLen=0;
	UCHAR datBuffer[FLS_SIZE_CONFIGURATION_DATA-2]={0};
	UINT  cfgDatLen=0;

	cfgDatLen=(UINT)sizeof(glv_parCfgData);

	rspCode=FLS_Read_PayPassConfigurationData(&datLen, datBuffer);
	if(rspCode == SUCCESS)
	{
		rspCode=api_pcd_vap_Check_PayPassConfigurationData(datLen, datBuffer);
		if (rspCode == SUCCESS)
		{
			;	//do nothing
		}
		else
		{
			flgLodDefault=TRUE;
		}
	}
	else
	{
		flgLodDefault=TRUE;
	}

	if (flgLodDefault == TRUE)
	{
		//Load Default Value
		rspCode=api_pcd_vap_Check_PayPassConfigurationData(cfgDatLen, (UCHAR*)glv_parCfgData);
		if (rspCode == SUCCESS)
		{
			rspCode=FLS_Write_PayPassConfigurationData(cfgDatLen, (UCHAR*)glv_parCfgData);
		}
	}	
}

void APP_Set_Default_MSDOption(void)
{
#ifndef _PLATFORM_AS350
	UCHAR rspCode = 0xFF;

	UCHAR InFlashMSDOptionData[4] = {0};

#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_MSDOption,4,InFlashMSDOptionData);
		if(rspCode == apiOK)
		{
			if(	((InFlashMSDOptionData[0] == 0xFF) && (InFlashMSDOptionData[1] == 0xFF)) ||
				((InFlashMSDOptionData[0] == 0x00) && (InFlashMSDOptionData[1] == 0x00)))
			{
				//	Do Nothing
			}
			else
			{
				Opt_MSD_Constructing_Track1 = InFlashMSDOptionData[0];
				Opt_MSD_Formatting_Track2	= InFlashMSDOptionData[1];
				Opt_MSD_CVN17_Support		= InFlashMSDOptionData[2];
			}
		}
	}
#endif
}

void APP_Set_Default_CashBack(void)
{
#ifndef _PLATFORM_AS350
	UCHAR rspCode = 0xFF;

	UCHAR InFlashCashBackData[20] = {0};

#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_CashBack_Data,20,InFlashCashBackData);
		if(rspCode == apiOK)
		{
			if(	((InFlashCashBackData[0] == 0xFF) && (InFlashCashBackData[1] == 0xFF))||
				((InFlashCashBackData[0] == 0x00) && (InFlashCashBackData[1] == 0x00)))
			{
				//	Do Nothing
			}
			else
			{
				memcpy(glv_parDF06_VISA[VISA_Cashback_Mode],InFlashCashBackData,2);
				memcpy(glv_parDF00_VISA[VISA_Cashback_Mode],&InFlashCashBackData[2],6);
				memcpy(glv_parDF01_VISA[VISA_Cashback_Mode],&InFlashCashBackData[8],6);
				memcpy(glv_parDF02_VISA[VISA_Cashback_Mode],&InFlashCashBackData[14],6);
			}
		}
	}
#endif
}

void APP_Set_Default_Cash(void)
{
#ifndef _PLATFORM_AS350
	UCHAR rspCode = 0xFF;

	UCHAR InFlashCashData[20] = {0};

#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_Cash_Data,20,InFlashCashData);
		if(rspCode == apiOK)
		{
			if(	((InFlashCashData[0] == 0xFF) && (InFlashCashData[1] == 0xFF)) ||
				((InFlashCashData[0] == 0x00) && (InFlashCashData[1] == 0x00)))
			{
				//	Do Nothing
			}
			else
			{
				memcpy(glv_parDF06_VISA[VISA_Cash_Mode],InFlashCashData,2);
				memcpy(glv_parDF00_VISA[VISA_Cash_Mode],&InFlashCashData[2],6);
				memcpy(glv_parDF01_VISA[VISA_Cash_Mode],&InFlashCashData[8],6);
				memcpy(glv_parDF02_VISA[VISA_Cash_Mode],&InFlashCashData[14],6);
			}
		}
	}
#endif
}

void APP_Set_Default_DRLEnable(void)
{
#ifndef _PLATFORM_AS350
	UCHAR rspCode = 0xFF;

	UCHAR InFlashDRLFlag = 0xFF;

#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_DRLEnable,1,&InFlashDRLFlag);
		if(rspCode == apiOK)
		{
			if((InFlashDRLFlag == 0x00) || (InFlashDRLFlag == 0x01))
				glv_parFlgDRL = InFlashDRLFlag;
			else
				glv_parFlgDRL = FALSE;
		}
	}
#endif
}

void APP_Set_Default_APID(void)
{
#ifndef _PLATFORM_AS350
	UCHAR	rspCode = 0xFF;
	UCHAR	InFlashAPIDData[166]={0};
	UCHAR	*InFlashAPIDDataPtr = NULLPTR;
	UCHAR	Index=0;

#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_APID,166,InFlashAPIDData);
		if(rspCode == apiOK)
		{
			if(	((InFlashAPIDData[0] == 0xFF) && (InFlashAPIDData[1] == 0xFF)) ||
				((InFlashAPIDData[0] == 0x00) && (InFlashAPIDData[1] == 0x00)))//empty
			{
				//do nothing
			}
			else
			{
				InFlashAPIDDataPtr = &InFlashAPIDData[2];

				if (InFlashAPIDData[0] <= ETP_PARA_NUMBER_PID)
				{
					for(Index=0; Index<InFlashAPIDData[0]; Index++)
					{
						memcpy(&glv_parDRLLimitSet[Index],InFlashAPIDDataPtr,LIMITSET_LEN);
						InFlashAPIDDataPtr+=LIMITSET_LEN;
					}
				}
			}
		}
	}
#endif
}


void APP_Set_Default_VLP_Suppoted(void)
{
#ifdef _PLATFORM_AS350
	glv_par9F7A[0] = TRUE;
#else
	UCHAR	rspCode = 0xFF;

	UCHAR	InFlashVLP = 0;

#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_VLP_Support,1,&InFlashVLP);
		if(rspCode == apiOK)
		{		
			if(InFlashVLP == 0xFF)	//empty, set default
			{
				glv_par9F7A[0] = TRUE;
			}
			else
			{
				if(InFlashVLP == 0x00)
					glv_par9F7A[0] = FALSE;
				else
					glv_par9F7A[0] = TRUE;
			}
		}
	}
#endif
}

void APP_Set_Default_Revo_List(void)
{
#ifndef _PLATFORM_AS350
	UCHAR rspCode = 0xFF;

	UCHAR InFlashRevoList[101] = {0};

	UCHAR *RevoListPoint = NULLPTR;
	UCHAR RevoListSize = 0;

	UCHAR Index=0;

#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_RevoList,101,InFlashRevoList);
		if(rspCode == apiOK)
		{
			if(	((InFlashRevoList[0] == 0xFF) && (InFlashRevoList[1] == 0xFF))||
				((InFlashRevoList[0] == 0x00) && (InFlashRevoList[1] == 0x00)))
			{
				//do nothing
			}
			else
			{
				if (InFlashRevoList[0] <= 10)	//Revo_List[10][9]
				{
					RevoListPoint = &InFlashRevoList[1];

					for(Index = 0; Index < InFlashRevoList[0]; Index++)
					{
						RevoListSize = *RevoListPoint;

						if (RevoListSize <= 9)
						{
							memcpy(Revo_List[Index], RevoListPoint+1, RevoListSize);
							RevoListPoint+=(1+RevoListSize);
						}
						else
						{
							if (Index != 0)
							{
								memset(Revo_List, 0, 10*9);
							}

							break;
						}
					}
				}
			}
		}
	}
#endif
}

void APP_Set_Default_Exception_File(void)
{
#ifndef _PLATFORM_AS350
	UCHAR rspCode = 0xFF;

	UCHAR InFlashExpFile[111] = {0};

	UCHAR *FilePoint = NULLPTR;
	UCHAR FileSize = 0;

	UCHAR Index=0;

#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_ExceptionFile,111,InFlashExpFile);
		if(rspCode == apiOK)
		{
			if(	((InFlashExpFile[0] == 0xFF) && (InFlashExpFile[1] == 0xFF)) ||
				((InFlashExpFile[0] == 0x00) && (InFlashExpFile[1] == 0x00)))
			{
				//do nothing
			}
			else
			{
				if (InFlashExpFile[0] <= 10)	//Exception_File[10][10]
				{
					FilePoint = &InFlashExpFile[1];

					for(Index = 0; Index < InFlashExpFile[0];Index++)
					{
						FileSize = *FilePoint;

						if (FileSize <= 10)
						{
							memcpy(Exception_File[Index],FilePoint+1,FileSize);
							FilePoint+=(1+FileSize);
						}
						else
						{
							if (Index != 0)
							{
								memset(Exception_File, 0, 10*11);
							}

							break;
						}
					}
				}
			}
		}
	}
#endif

}

void APP_Set_Default_BaudRate(void)
{
#ifndef _PLATFORM_AS350
	UCHAR rspCode = 0xFF;
	UCHAR flgSetDefault=FALSE;
	UINT  flsBaud;

	UCHAR InFlashBaud[2] = {0xFF,0xFF};

#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_BaudRate,2,InFlashBaud);
		if(rspCode == apiOK)
		{
			if(	((InFlashBaud[0] == 0xFF) && (InFlashBaud[1] == 0xFF))||
				((InFlashBaud[0] == 0x00) && (InFlashBaud[1] == 0x00)))
			{
				flgSetDefault=TRUE;
			}
			else
			{
				flsBaud=InFlashBaud[0]*256 + InFlashBaud[1];
				
				switch(flsBaud)
				{
					case COM_19200:
					case COM_28800:
					case COM_38400:
					case COM_57600:
					case COM_115200:	glv_vap_BaudRate = flsBaud;	break;
					default:			flgSetDefault=TRUE;			break;
				}
			}

			if (flgSetDefault == TRUE)
			{
				glv_vap_BaudRate = COM_38400;
			}
		}
	}
#else
	glv_vap_BaudRate = COM_38400;
#endif
}

void APP_Set_Default_CVM(void)
{
	glv_vap_CVM[0].ID = 0x00;	//Does not support CVM
	glv_vap_CVM[1].ID = 0x10;	//Signature with touch screen
	glv_vap_CVM[2].ID = 0x11;	//online PIN
	glv_vap_CVM[3].ID = 0x12;	//offline PIN

#ifndef _PLATFORM_AS350
	UCHAR rspCode = 0xFF;
	
	UCHAR InFlashCVMData[8]={0};
	UCHAR Index = 0;

#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_CVM,8,InFlashCVMData);
		if(rspCode == apiOK)
		{
			if(	((InFlashCVMData[0] == 0xFF) && (InFlashCVMData[1] == 0xFF))||
				((InFlashCVMData[0] == 0x00) && (InFlashCVMData[1] == 0x00)))	//empty, Load Default
			{				
				glv_vap_CVM[0].State = 0x00;	//Does not support CVM
				glv_vap_CVM[1].State = 0x00;	//Signature with touch screen
				glv_vap_CVM[2].State = 0x00;	//online PIN
				glv_vap_CVM[3].State = 0x00;	//offline PIN				
			}
			else
			{
				for(Index=0; Index < VAP_CVM_NUMBER; Index++)
				{
					glv_vap_CVM[Index].ID = InFlashCVMData[Index];
					glv_vap_CVM[Index].State = InFlashCVMData[Index+1];
				}
			}
		}
	}
#endif
}

void APP_Set_Default_Scheme(void)
{
	glv_vap_Scheme[0].ID=VAP_Scheme_DPAS_Magstripe;	//DPAS Magstripe
	glv_vap_Scheme[1].ID=VAP_Scheme_DPAS_EMV;		//DPAS EMV
	glv_vap_Scheme[2].ID=VAP_Scheme_CDA;			//CDA
	glv_vap_Scheme[3].ID=VAP_Scheme_dCvv;			//dCVV
	glv_vap_Scheme[4].ID=VAP_Scheme_FullVSDC;		//Full VSDC
	glv_vap_Scheme[5].ID=VAP_Scheme_FastDDAVLP;		//Fast DDA + VLP
	glv_vap_Scheme[6].ID=VAP_Scheme_Wave2;			//WAVE2
	glv_vap_Scheme[7].ID=VAP_Scheme_Wave3;			//WAVE3, qVSDC
	glv_vap_Scheme[8].ID=VAP_Scheme_MSD20;			//MSD2.0
	glv_vap_Scheme[9].ID=VAP_Scheme_PP_Magstripe;	//PP Magstripe
	glv_vap_Scheme[10].ID=VAP_Scheme_PP_MChip;		//PP MChip
	glv_vap_Scheme[11].ID=VAP_Scheme_AE_Magstripe;	//AE Magstripe
	glv_vap_Scheme[12].ID=VAP_Scheme_AE_EMV;		//AE EMV
	glv_vap_Scheme[13].ID=VAP_Scheme_JCBWave2;		//JCB Wave2
	glv_vap_Scheme[14].ID=VAP_Scheme_JCBWave3;		//JCB Wave3, qVSDC
	glv_vap_Scheme[15].ID=VAP_Scheme_JCBMag;		//JCB Magstripe
	glv_vap_Scheme[16].ID=VAP_Scheme_qUICS;			//qUICS
	
#ifdef _PLATFORM_AS350

	//Default Scheme Enable
	glv_vap_Scheme[6].Support=TRUE;		//WAVE2	Support
	glv_vap_Scheme[7].Support=TRUE;		//WAVE3, qVSDC Support
	glv_vap_Scheme[8].Support=TRUE;		//MSD2.0 Support
	glv_vap_Scheme[9].Support=TRUE;		//PP Magstripe
	glv_vap_Scheme[10].Support=TRUE;	//PP MChip
	glv_vap_Scheme[13].Support=TRUE;	//JCB Wave2 Support
	glv_vap_Scheme[14].Support=TRUE;	//JCB Wave3, qVSDC Support
	glv_vap_Scheme[15].Support=TRUE;	//JCB Magstripe Support
	
#else

	UCHAR rspCode = 0xFF;
	UCHAR flgSetDefault=FALSE;
	UCHAR InFlashSchemeData[VAP_SCHEME_NUMBER*2]={0};
	UCHAR Index = 0;
	UCHAR IndexInFlash = 0;


#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_Scheme,VAP_SCHEME_NUMBER*2,InFlashSchemeData);
		if(rspCode == apiOK)
		{
			//Check Scheme List
			for (Index=0; Index < VAP_SCHEME_NUMBER; Index++)
			{
				if (InFlashSchemeData[Index*2] != glv_vap_Scheme[Index].ID)
				{
					flgSetDefault=TRUE;
					break;
				}
			}
			
  			if (flgSetDefault == FALSE)
			{
				for(Index = 0,IndexInFlash=0; Index<VAP_SCHEME_NUMBER; Index++,IndexInFlash+=2)
				{
					if ((InFlashSchemeData[IndexInFlash+1] == TRUE) ||
						(InFlashSchemeData[IndexInFlash+1] == FALSE))
					{
						glv_vap_Scheme[Index].ID = InFlashSchemeData[IndexInFlash];
	 					glv_vap_Scheme[Index].Support = InFlashSchemeData[IndexInFlash+1];

						if(glv_vap_Scheme[Index].ID == VAP_Scheme_Wave2)
						{
							glv_par_WAVE2_Enable = glv_vap_Scheme[Index].Support;
						}
					}
					else	//Invalid Data
					{
						flgSetDefault=TRUE;
						break;
					}
				}
			}
			
			if (flgSetDefault == TRUE)
			{
				for (Index=0; Index < VAP_SCHEME_NUMBER; Index++)
				{
					glv_vap_Scheme[Index].Support=FALSE;
				}
				
				glv_vap_Scheme[6].Support=TRUE;				//WAVE2	Support
				glv_vap_Scheme[7].Support=TRUE;				//WAVE3, qVSDC Support
				glv_vap_Scheme[8].Support=TRUE;				//MSD2.0 Support
				glv_vap_Scheme[13].Support=TRUE;			//JCB Wave2 Support
				glv_vap_Scheme[14].Support=TRUE;			//JCB Wave3, qVSDC Support
				glv_vap_Scheme[15].Support=TRUE;			//JCB Magstripe Support

				//If WAVE2 Support, set glv_parWAVE2_Enable = TRUE
				glv_par_WAVE2_Enable = TRUE;
			}
		}
	}
#endif
}

void 	APP_Set_Default_EMV_Tags(void)	
{
	
	
	
#ifdef _PLATFORM_AS350

	APP_SetDefaultEMVTagsProcess();
	
#else

	//20140106 Set EMV tag by flash
	UCHAR InFlashEMVData_2[256] = {0};		
	UCHAR rspCode = 0xFF;
	UCHAR curNumber = 0,tagNumber = 0;
	UCHAR lenOfT = 0,lenOfL = 0;
	UINT lenOfV = 0;
	UCHAR *ptrInput = NULLPTR;
	UCHAR flgSetDefault=FALSE;

	UCHAR i=0;
	UCHAR defRskParameter[2]={0x1C, 0x00};

	
	//20140116 V1, Default EMV tag, for first time power on(Flash is empty)
	UCHAR Default_Emv_Tag[204] = {	0x9F,0x35,0x01,0x01,
									0xDF,0x04,0x01,0x01,
									0x9F,0x1A,0x02,0x01,0x58,						//20140820 change
									0x5F,0x2A,0x02,0x09,0x01,						//20140820 change
									0x9C,0x01,0x00,
									0xDF,0x00,0x06,0x99,0x99,0x99,0x99,0x99,0x99,
									0xDF,0x01,0x06,0x00,0x00,0x00,0x00,0x00,0x00,
									0xDF,0x02,0x06,0x00,0x00,0x00,0x00,0x00,0x00,
									0xDF,0x03,0x01,0x00,
									0xDF,0x05,0x01,0x00,
									0x9F,0x66,0x04,0xA6,0x00,0xC0,0x00,
									0xDF,0x06,0x02,0x1C,0x00,				//20140205 changed to 0x1C, 0x00
									0xDF,0x60,0x06,0x99,0x99,0x99,0x99,0x99,0x99,	//JCB Transaction Limit
									0xDF,0x61,0x06,0x00,0x00,0x00,0x00,0x00,0x00,	//JCB CVM Limit
									0xDF,0x62,0x06,0x00,0x00,0x00,0x00,0x00,0x00,	//JCB Floor Limit
									0xDF,0x66,0x01,0x22,							//JCB Terminal Type
									0xDF,0x67,0x04,0xA6,0x00,0xC0,0x00,
									0x9F,0x1B,0x04,0x00,0x00,0x00,0x00,
									0xDF,0x68,0x02,0x7B,0x00,						//JCB Combination Option
									0xDF,0x69,0x03,0x70,0x80,0x00,					//JCB Terminal Interchange Profile
									0xDF,0x6A,0x01,0x00,							//JCB BRS MAX Target
									0xDF,0x6B,0x01,0x00,							//JCB BRS Target
									0xDF,0x6C,0x06,0x00,0x00,0x00,0x00,0x20,0x00,	//JCB BRS Threeshold
									0xDF,0x6D,0x05,0x90,0x40,0x00,0x80,0x00,		//JCB TAC Default
									0xDF,0x6E,0x05,0x04,0x10,0x00,0x00,0x00,		//JCB TAC Denial
									0xDF,0x6F,0x05,0x90,0x60,0x00,0x90,0x00,		//JCB TAC_Online
									0xDF,0x70,0x06,0x99,0x99,0x99,0x99,0x99,0x99,	//qUICS CL Transaction Limit
									0xDF,0x71,0x06,0x00,0x00,0x00,0x00,0x00,0x00,	//qUICS CVM Required Limit
									0xDF,0x72,0x06,0x00,0x00,0x00,0x00,0x00,0x00,	//qUICS CL Floor Limit
									0xDF,0x73,0x02,0x1C,0x00,						//qUICS Risk Parameter
									0xDF,0x77,0x04,0x26,0x00,0x00,0x80};			//qUICS TTQ

#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif

	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_EMV_Tag,256,InFlashEMVData_2);
		if(rspCode == apiOK)
		{
			if(	((InFlashEMVData_2[0] == 0xFF)||(InFlashEMVData_2[1] == 0xFF)) ||
				((InFlashEMVData_2[0] == 0x00)||(InFlashEMVData_2[1] == 0x00)))	//empty set, load default
			{
				flgSetDefault=TRUE;
			}
			else
			{
				tagNumber = 33;
				ptrInput = InFlashEMVData_2;
				do {
					if(*ptrInput != 0xFF)
					{
						rspCode=UT_Get_TLVLength(ptrInput, &lenOfT, &lenOfL, &lenOfV);
						if (rspCode == SUCCESS)
						{				
							rspCode = api_pcd_vap_Set_EMVTag(ptrInput);
							if(rspCode == SUCCESS)
							{
								//Point to Next Tag
								ptrInput+=(lenOfT+lenOfL+lenOfV);
								curNumber++;
							}
							else
							{
								flgSetDefault=TRUE;
								break;
							}
						}
						else
						{
							flgSetDefault=TRUE;
							break;
						}
					}
					else
					{
						flgSetDefault=TRUE;
						break;
					}					
				} while (curNumber < tagNumber);
			}

			if (flgSetDefault == TRUE)
			{				
				APP_SetDefaultEMVTagsProcess();

#ifdef _PLATFORM_AS210
				rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
				rspCode = apiOK;
#endif
				if(rspCode == apiOK)
				{
					api_fls_write(FLSID_PARA,FLS_ADDRESS_EMV_Tag,204,Default_Emv_Tag);
				}	
			}

			//Enable Risk Parameter by Default
			for (i=0;i<ETP_NUMBER_COMBINATION;i++)
			{
				memcpy(&glv_parDF06[i], defRskParameter, 2);
			}
		}
	}
	
#endif
}

//Read Parameter from flash or set default value
void APP_Set_Default_V3_Parameter()
{
#ifdef _PLATFORM_AS350

	UCHAR	RspBuf[3]={0};
	UCHAR	Def_V3_Parameter[102]={	0x00,0x64,											//Total Size
									0x00,0x10,											//Total Parameter Number
									0x00,0x01,0x00,0x02,0x01,0xF4,						//P_MSG_Timeout, 500ms
									0x00,0x02,0x00,0x02,0x3A,0x98,						//P_Sale_Timeout, 15000ms
									0x00,0x03,0x00,0x02,0xFF,0xFF,						//P_Poll_MSG, //20131230 change the default value to 30000ms = 30 sec	//20140205 changed to maximum value
									0x00,0x04,0x00,0x02,0x13,0x88,						//P_Buf_Timeout, 5000ms
									0x00,0x05,0x00,0x01,0x00,							//P_ENCRYPTION, 00 for disable, 01 for enable	//20131230 change the default value to enable
									0x00,0x06,0x00,0x01,0x00,							//P_Display, 00 for display all response code, 01 for convert all response code	//20131230 change the default value to do not display message
									0x00,0x07,0x00,0x02,0x04,0x00,						//P_MAX_BUF_SIZE, 1024 bytes
									0x00,0x08,0x00,0x02,0x13,0x88,						//P_Double_Dip, 5000ms
									0x00,0x09,0x00,0x02,0x0E,0x01,						//P_Reader_index, 0E01
									0x00,0x0A,0x00,0x06,0x5F,0x2D,0x03,0x7A,0x68,0x01,	//P_LAUGUAGE, 7A68 for chinese	//20131230 change the default value to english
									0x00,0x0B,0x00,0x02,0x07,0xD0,						//P_DISPLAY_S_MSG, simple message display, 2000ms
									0x00,0x0C,0x00,0x02,0x13,0x88,						//P_DISPLAY_L_MSG, long message display, 5000ms
									0x00,0x0D,0x00,0x02,0x27,0x10,						//P_DISPLAY_SS_MSG, signature signing on the screen, 10000ms
									0x00,0x0E,0x00,0x02,0x13,0x88,						//P_DISPLAY_SR_MSG, signature signing on the receipt, 5000ms	
									0x00,0x0F,0x00,0x02,0x27,0x10,						//P_DISPLAY_PIN_MSG, PIN entry, 10000ms
									0x00,0x10,0x00,0x02,0x0B,0xB8};						//P_DISPLAY_E_MSG, error messages display, 3000ms
#else

	UCHAR	curParNumber=0;
	UINT	parNumber=0;
	UCHAR	*ptrData=NULLPTR;
	UCHAR	parLength=0;
	UCHAR	flgSetDefault=FALSE;
	UCHAR	rspCode = 0xFF;
	UCHAR	InFlashParaData[128] = {0};
	UINT	Para_Index = 0,P_LAUGUAGE_Len=0;
	UCHAR	i = 0;
	UCHAR	Def_V3_Parameter[100]={	0x00,0x10,								
									0x00,0x01,0x00,0x02,0x01,0xF4,						//P_MSG_Timeout, 500ms
									0x00,0x02,0x00,0x02,0x3A,0x98,						//P_Sale_Timeout, 15000ms
									0x00,0x03,0x00,0x02,0xFF,0xFF,						//P_Poll_MSG, //20131230 change the default value to 30000ms = 30 sec	//20140205 changed to maximum value
									0x00,0x04,0x00,0x02,0x13,0x88,						//P_Buf_Timeout, 5000ms
									0x00,0x05,0x00,0x01,0x01,							//P_ENCRYPTION, 00 for disable, 01 for enable	//20131230 change the default value to enable
									0x00,0x06,0x00,0x01,0x01,							//P_Display, 00 for display all response code, 01 for convert all response code	//20131230 change the default value to do not display message
									0x00,0x07,0x00,0x02,0x04,0x00,						//P_MAX_BUF_SIZE, 1024 bytes
									0x00,0x08,0x00,0x02,0x13,0x88,						//P_Double_Dip, 5000ms
									0x00,0x09,0x00,0x02,0x0E,0x01,						//P_Reader_index, 0E01
									0x00,0x0A,0x00,0x06,0x5F,0x2D,0x03,0x65,0x6E,0x01,	//P_LAUGUAGE, 7A68 for chinese	//20131230 change the default value to english
									0x00,0x0B,0x00,0x02,0x07,0xD0,						//P_DISPLAY_S_MSG, simple message display, 2000ms
									0x00,0x0C,0x00,0x02,0x13,0x88,						//P_DISPLAY_L_MSG, long message display, 5000ms
									0x00,0x0D,0x00,0x02,0x27,0x10,						//P_DISPLAY_SS_MSG, signature signing on the screen, 10000ms
									0x00,0x0E,0x00,0x02,0x13,0x88,						//P_DISPLAY_SR_MSG, signature signing on the receipt, 5000ms	
									0x00,0x0F,0x00,0x02,0x27,0x10,						//P_DISPLAY_PIN_MSG, PIN entry, 10000ms
									0x00,0x10,0x00,0x02,0x0B,0xB8};						//P_DISPLAY_E_MSG, error messages display, 3000ms
#endif

	
	glv_vap_Parameter[0].Index[1]=1;	//P_MSG_TIMEOUT
	glv_vap_Parameter[1].Index[1]=2;	//P_SALE_TIMEOUT
	glv_vap_Parameter[2].Index[1]=3;	//P_POLL_MSG
	glv_vap_Parameter[3].Index[1]=4;	//P_BUF_TIMEOUT
	glv_vap_Parameter[4].Index[1]=5;	//P_ENCRYPTION
	glv_vap_Parameter[5].Index[1]=6;	//P_DISPLAY
	glv_vap_Parameter[6].Index[1]=7;	//P_MAX_BUF_SIZE
	glv_vap_Parameter[7].Index[1]=8;	//P_DOUBLE_DIP
	glv_vap_Parameter[8].Index[1]=9;	//P_READER_INDEX
	glv_vap_Parameter[9].Index[1]=10;	//P_LAUGUAGE
	glv_vap_Parameter[10].Index[1]=11;	//P_DISPLAY_S_MSG
	glv_vap_Parameter[11].Index[1]=12;	//P_DISPLAY_L_MSG
	glv_vap_Parameter[12].Index[1]=13;	//P_DISPLAY_SS_MSG
	glv_vap_Parameter[13].Index[1]=14;	//P_DISPLAY_SR_MSG
	glv_vap_Parameter[14].Index[1]=15;	//P_DISPLAY_PIN_MSG
	glv_vap_Parameter[15].Index[1]=16;	//P_DISPLAY_E_MSG


#ifdef _PLATFORM_AS350

	api_pcd_vap_flgDbgAndOptMode = TRUE;
	
	api_pcd_vap_deb_SetParameters(Def_V3_Parameter,&Def_V3_Parameter[2],RspBuf,&RspBuf[2]);
	
	api_pcd_vap_flgDbgAndOptMode = FALSE;

	if(RspBuf[2])	// RX_Success = 0x00 
	{
		UT_PutStr(1,0,FONT0,14,(UCHAR *)"Set V3_Par Err");
		UT_WaitKey();
	}

#else

#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_Parameter,128,InFlashParaData);
		if(rspCode == apiOK)
		{
			if(	((InFlashParaData[0] == 0x00) && (InFlashParaData[1] == 0x00)) || 
				((InFlashParaData[0] == 0xFF) && (InFlashParaData[1] == 0xFF))) //empty, Load default Value
			{
				flgSetDefault=TRUE;
			}
			else	//load from flash
			{
				parNumber=16;
				ptrData = InFlashParaData;

				do 
				{		
					rspCode=api_pcd_vap_Set_Parameter(&ptrData[0], &ptrData[2], &ptrData[4]);
					if (rspCode == SUCCESS)
					{
						parLength=ptrData[2]*256+ptrData[3];
						ptrData+=(4+parLength);
						curParNumber++;
					}
					else
					{
						flgSetDefault=TRUE;
						break;
					}
				} while (curParNumber < parNumber);
			}

			if (flgSetDefault == TRUE)
			{
				parNumber = Def_V3_Parameter[0] *256 + Def_V3_Parameter[1];
				ptrData=&Def_V3_Parameter[2];
				
				do 
				{		
					rspCode=api_pcd_vap_Set_Parameter(&ptrData[0], &ptrData[2], &ptrData[4]);
					if (rspCode == SUCCESS)
					{
						parLength=ptrData[2]*256+ptrData[3];
						ptrData+=(4+parLength);
						curParNumber++;
					}
				} while (curParNumber < parNumber);

				api_fls_write(FLSID_PARA,FLS_ADDRESS_Parameter,98,&Def_V3_Parameter[2]);
			}
	
			//activate those parameter
			for(i=0;i<16;i++)
			{
				Para_Index = glv_vap_Parameter[i].Index[0]*256+glv_vap_Parameter[i].Index[1];
				switch (Para_Index)
				{
					case VAP_RIF_PARAMETER_INDEX_P_MSG_TIMEOUT:		VAP_VISA_P_MSG_TIMEOUT		= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10;	break;
					case VAP_RIF_PARAMETER_INDEX_P_SALE_TIMEOUT:	VAP_VISA_P_SALE_TIMEOUT		= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10;
																	etp_tmrSale					= VAP_VISA_P_SALE_TIMEOUT;																		break;
					case VAP_RIF_PARAMETER_INDEX_P_POLL_MSG:		VAP_VISA_P_POLL_MSG			= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])*100;	break;
					case VAP_RIF_PARAMETER_INDEX_P_BUF_TIMEOUT: 	VAP_VISA_P_BUF_TIMEOUT		= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10;	break;
					case VAP_RIF_PARAMETER_INDEX_P_ENCRYPTION:		VAP_VISA_P_ENCRYPTION		= glv_vap_Parameter[Para_Index-1].Data[0];														break;
					case VAP_RIF_PARAMETER_INDEX_P_DISPLAY: 		VAP_VISA_P_DISPLAY			= glv_vap_Parameter[Para_Index-1].Data[0];														break;
					case VAP_RIF_PARAMETER_INDEX_P_MAX_BUF_SIZE:	VAP_VISA_P_MAX_BUF_SIZE		= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1]); 	break;
					case VAP_RIF_PARAMETER_INDEX_P_DOUBLE_DIP:		VAP_VISA_P_DOUBLE_DIP		= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10; 	break;
					case VAP_RIF_PARAMETER_INDEX_P_READER_INDEX:	VAP_VISA_P_READER_INDEX		= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1]); 	break;
					case VAP_RIF_PARAMETER_INDEX_P_LAUGUAGE:		P_LAUGUAGE_Len = glv_vap_Parameter[Para_Index-1].Length[0]*256 + glv_vap_Parameter[Para_Index-1].Length[1];
																	memcpy(VAP_VISA_P_LAUGUAGE,glv_vap_Parameter[Para_Index-1].Data,P_LAUGUAGE_Len);											break;
					case VAP_RIF_PARAMETER_INDEX_P_DISPLAY_S_MSG:	VAP_VISA_P_DISPLAY_S_MSG	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10; 	break;
					case VAP_RIF_PARAMETER_INDEX_P_DISPLAY_L_MSG:	VAP_VISA_P_DISPLAY_L_MSG	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10;	break;
					case VAP_RIF_PARAMETER_INDEX_P_DISPLAY_SS_MSG:	VAP_VISA_P_DISPLAY_SS_MSG	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10; 	break;
					case VAP_RIF_PARAMETER_INDEX_P_DISPLAY_SR_MSG:	VAP_VISA_P_DISPLAY_SR_MSG	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10;	break;
					case VAP_RIF_PARAMETER_INDEX_P_DISPLAY_PIN_MSG:	VAP_VISA_P_DISPLAY_PIN_MSG	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10;	break;
					case VAP_RIF_PARAMETER_INDEX_P_DISPLAY_E_MSG:	VAP_VISA_P_DISPLAY_E_MSG	= (	glv_vap_Parameter[Para_Index-1].Data[0]*256 + glv_vap_Parameter[Para_Index-1].Data[1])/10; 	break;
				}
			}	
		}
	}
#endif
}

void APP_Set_Default_Message(void)
{
	UCHAR Def_Message[511]=	{
	0x01,0xFD,0x14,
	0x01,0x10,0xC5,0x77,0xAA,0xEF,0xA8,0xCF,0xA5,0xCE,0x0A,0x57,0x65,0x6C,0x63,0x6F,0x6D,0x65,
	0x02,0x10,0xC1,0xC2,0xC1,0xC2,0xB1,0x7A,0x0A,0x54,0x68,0x61,0x6E,0x6B,0x20,0x59,0x6F,0x75,
	0x03,0x11,0x54,0x68,0x61,0x6E,0x6B,0x20,0x59,0x6F,0x75,0x2C,0x0A,0x3C,0x35,0x46,0x32,0x30,0x3E,
	0x04,0x1E,0xA5,0xE6,0xA9,0xF6,0xA7,0xB9,0xA6,0xA8,0x0A,0x54,0x72,0x61,0x6E,0x73,0x61,0x63,0x74,0x69,0x6F,0x6E,0x20,0x43,0x6F,0x6D,0x70,0x6C,0x65,0x74,0x65,0x64,
	0x05,0x15,0x50,0x6C,0x65,0x61,0x73,0x65,0x20,0x55,0x73,0x65,0x20,0x4F,0x74,0x68,0x65,0x72,0x20,0x43,0x61,0x72,0x64,
	0x06,0x12,0x50,0x6C,0x65,0x61,0x73,0x65,0x20,0x49,0x6E,0x73,0x65,0x72,0x74,0x20,0x43,0x61,0x72,0x64,
	0x07,0x14,0x50,0x6C,0x65,0x61,0x73,0x65,0x20,0x53,0x65,0x6C,0x65,0x63,0x74,0x20,0x31,0x20,0x43,0x61,0x72,0x64,
	0x08,0x20,0x49,0x6E,0x74,0x65,0x72,0x6E,0x61,0x74,0x69,0x6F,0x6E,0x61,0x6C,0x20,0x43,0x61,0x72,0x64,0x0A,0x50,0x6C,0x65,0x61,0x73,0x65,0x20,0x49,0x6E,0x73,0x65,0x72,0x74,
	0x09,0x10,0x50,0x6C,0x65,0x61,0x73,0x65,0x20,0x54,0x72,0x79,0x20,0x41,0x67,0x61,0x69,0x6E,
	0x0A,0x1F,0x49,0x6E,0x74,0x65,0x72,0x6E,0x61,0x74,0x69,0x6F,0x6E,0x61,0x6C,0x20,0x43,0x61,0x72,0x64,0x0A,0x50,0x6C,0x65,0x61,0x73,0x65,0x20,0x53,0x77,0x69,0x70,0x65,
	0x0B,0x19,0x50,0x6C,0x65,0x61,0x73,0x65,0x20,0x53,0x69,0x67,0x6E,0x20,0x6F,0x6E,0x0A,0x54,0x68,0x65,0x20,0x53,0x63,0x72,0x65,0x65,0x6E,
	0x0C,0x21,0xBD,0xD0,0xC3,0xB1,0xA6,0x57,0x0A,0x50,0x6C,0x65,0x61,0x73,0x65,0x20,0x53,0x69,0x67,0x6E,0x20,0x6F,0x6E,0x0A,0x54,0x68,0x65,0x20,0x72,0x65,0x63,0x65,0x69,0x70,0x74,
	0x0D,0x10,0x50,0x6C,0x65,0x61,0x73,0x65,0x20,0x45,0x6E,0x74,0x65,0x72,0x20,0x50,0x49,0x4E,
	0x0E,0x17,0x4F,0x66,0x66,0x6C,0x69,0x6E,0x65,0x20,0x41,0x76,0x61,0x69,0x6C,0x61,0x62,0x6C,0x65,0x20,0x46,0x75,0x6E,0x64,0x73,
	0x0F,0x2C,0x50,0x49,0x4E,0x20,0x45,0x6E,0x74,0x72,0x79,0x20,0x52,0x65,0x71,0x75,0x69,0x72,0x65,0x64,0x0A,0x54,0x72,0x61,0x6E,0x73,0x61,0x63,0x74,0x69,0x6F,0x6E,0x20,0x4E,0x6F,0x74,0x20,0x43,0x6F,0x6D,0x70,0x6C,0x65,0x74,0x65,0x64,
	0x10,0x2C,0x53,0x69,0x67,0x6E,0x61,0x74,0x75,0x72,0x65,0x20,0x52,0x65,0x71,0x75,0x69,0x72,0x65,0x64,0x0A,0x54,0x72,0x61,0x6E,0x73,0x61,0x63,0x74,0x69,0x6F,0x6E,0x20,0x4E,0x6F,0x74,0x20,0x43,0x6F,0x6D,0x70,0x6C,0x65,0x74,0x65,0x64,
	0x11,0x01,0x2D,
	0x12,0x17,0xBD,0xD0,0xB7,0x50,0xC0,0xB3,0xA5,0x64,0xA4,0xF9,0x0A,0x50,0x72,0x65,0x73,0x65,0x6E,0x74,0x20,0x43,0x61,0x72,0x64,
	0x13,0x16,0xBD,0xD0,0xB2,0xBE,0xB6,0x7D,0xA5,0x64,0xA4,0xF9,0x0A,0x52,0x65,0x6D,0x6F,0x76,0x65,0x20,0x43,0x61,0x72,0x64,
	0x14,0x14,0xB3,0x42,0xB2,0x7A,0xA4,0xA4,0x0A,0x50,0x72,0x6F,0x63,0x65,0x73,0x73,0x69,0x6E,0x67,0x2E,0x2E,0x2E};

#ifdef _PLATFORM_AS210
	UCHAR Def_Private_Message[43]={
	0x00,0x29,0x02,
	0x1F,0x04,0x41,0x6D,0x74,0x3A,
	0x20,0x20,0xA5,0xE6,0xA9,0xF6,0xA4,0xA3,0xA6,0xA8,0xA5,0x5C,0x0A,0x54,0x72,0x61,0x6E,0x73,0x61,0x63,0x74,0x69,0x6F,0x6E,0x0A,0x54,0x65,0x72,0x6D,0x69,0x6E,0x61,0x74,0x65};
#else
	UCHAR Def_Private_Message[46]={
	0x00,0x2C,0x02,
	0x1F,0x07,0xAA,0xF7,0xC3,0x42,0x3A,0x4E,0x54,
	0x20,0x20,0xA5,0xE6,0xA9,0xF6,0xA4,0xA3,0xA6,0xA8,0xA5,0x5C,0x0A,0x54,0x72,0x61,0x6E,0x73,0x61,0x63,0x74,0x69,0x6F,0x6E,0x0A,0x54,0x65,0x72,0x6D,0x69,0x6E,0x61,0x74,0x65};
#endif

	UCHAR msgNumber,*ptrData=0,msgLen,rspCode,curMsgNumber=0;
	UINT i = 0, total_msg_len = 0;
	UCHAR MsgNum=0,MsgID=0;

	
	//set message ID
	for(MsgNum=0,MsgID=1;MsgNum<VAP_MESSAGE_NUMBER;MsgNum++,MsgID++)
	{
		glv_vap_Message[MsgNum].ID = MsgID;
		glv_vap_Message[MsgNum].Length = 0;
		memset(glv_vap_Message[MsgNum].Message,0x00,64);
	}
		
#ifdef _PLATFORM_AS350
	UCHAR flgError=0;	
	UCHAR cntMsgNumber=0;
	
	//Load Default message	
	total_msg_len = Def_Message[0]*256+Def_Message[1] - 1;		//total message ID num
	
	msgNumber=Def_Message[2];
	ptrData=&Def_Message[3];

	do 
	{	
		if( i < total_msg_len )
		{
			if (glv_vap_Message[curMsgNumber].ID == ptrData[i])
			{
				if (ptrData[i+1] <= 64)
				{
					glv_vap_Message[curMsgNumber].Length = ptrData[i+1];
					memcpy(glv_vap_Message[curMsgNumber].Message,  &ptrData[i+2], ptrData[i+1]);

					rspCode = SUCCESS;
				}
				else
				{
					rspCode = FAIL;
				}
			}
		}
		else
			break;
	
		if (rspCode == SUCCESS)
		{
			msgLen=ptrData[i+1];
			i+=(1+1+msgLen);	//ID(1)+Length(1)
			curMsgNumber++;
		}
		else
		{
			flgError=TRUE;
			break;
		}
	} while (curMsgNumber < msgNumber);		

	if(flgError)
	{
		UT_PutStr(1,0,FONT0,11,(UCHAR *)"Set Msg Err");
		UT_WaitKey();
	}

	//Load Private message	
	total_msg_len = Def_Private_Message[0]*256+Def_Private_Message[1] - 1;		//total message ID num
	
	msgNumber=Def_Private_Message[2];
	ptrData=&Def_Private_Message[3];

	i=0;

	do
	{
		//Point to Length
		i++;
		ptrData++;
		msgLen=ptrData[0];

		//Point to Data
		i++;
		ptrData++;

		//Point to Next Index
		i+=msgLen;
		ptrData+=msgLen;
		cntMsgNumber++;
	} while (i < total_msg_len);

	if ((i != total_msg_len) || (cntMsgNumber != msgNumber))
	{
		flgError=TRUE;
	}

	if (flgError == FALSE)
	{
		ptrData=&Def_Private_Message[3];

		for (i=0; i < cntMsgNumber; i++)
		{
			if ((ptrData[0] < (31+VAP_Private_MESSAGE_NUMBER)) &&	//Private Message Starts from 31
				(ptrData[1] < 64))
			{
				MsgID=ptrData[0]-1;	//Message Arrar ID Start from 0
				glv_vap_Message[MsgID].Length = ptrData[1];
				memcpy(glv_vap_Message[MsgID].Message, &ptrData[2], ptrData[1]);
				ptrData+=(1+1+ptrData[1]);	//Point to Next ID (ID+Len+Message Len)
			}
			else
			{
				flgError=TRUE;
				break;
			}
		}
	}
	
	if(flgError)
	{
		UT_PutStr(1,0,FONT0,11,(UCHAR *)"Set Msg Err");
		UT_WaitKey();
	}
#else
	UCHAR InFlashMessageData[1320] = {0};
	UCHAR flgSetDefault_Normal=FALSE;
	UCHAR flgSetDefault_Private=FALSE;

	//Load default message
#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_Message,1320,InFlashMessageData);
		if(rspCode == apiOK)
		{
			if(	((InFlashMessageData[0]==0xFF) && (InFlashMessageData[1]==0xFF)) ||
				((InFlashMessageData[0]==0x00) && (InFlashMessageData[1]==0x00)))	//empty, Load Defaule
			{
				flgSetDefault_Normal=TRUE;
			}
			else
			{
				curMsgNumber = 0;
				msgNumber = 20;
				ptrData = InFlashMessageData;
				do 
				{
					if((*ptrData > 0)&&(*ptrData < 21))	//20140103 if message ID is 1~20, return VAP_RIF_RC_INVALID_DATA
					{
						rspCode=api_pcd_vap_Set_DisplayMessage(ptrData[0], ptrData[1], &ptrData[2]);
						if (rspCode == SUCCESS)
						{
							msgLen=ptrData[1];
							ptrData+=(1+1+msgLen);	//ID(1)+Length(1)
							curMsgNumber++;
						}
						else
						{
							flgSetDefault_Normal=TRUE;
							break;
						}
					}
					else
					{
						flgSetDefault_Normal=TRUE;
						break;
					}
				} while (curMsgNumber < msgNumber);	
			}
		}
	}
	
	if (flgSetDefault_Normal == TRUE)
	{
		total_msg_len = Def_Message[0]*256+Def_Message[1] - 1;		//total message ID num

		msgNumber=Def_Message[2];
		ptrData=&Def_Message[3];

		for(i=0;i<VAP_MESSAGE_NUMBER;i++)
		{
			if(glv_vap_Message[i].ID == *ptrData)
			{
				ptrData++;	//shift to length
				
				glv_vap_Message[i].Length = *ptrData;
				ptrData++;	//shift to value

				memcpy(glv_vap_Message[i].Message,ptrData,glv_vap_Message[i].Length);
				ptrData+=glv_vap_Message[i].Length;

				total_msg_len-=1+1+glv_vap_Message[i].Length;
			}
			if(!total_msg_len)
				break;
		}
	}

	//Load default private message
#ifdef _PLATFORM_AS210
	rspCode = api_fls_memtype(FLSID_PARA,FLSType_SRAM);
#else
	rspCode = apiOK;
#endif
	if(rspCode == apiOK)
	{
		rspCode = api_fls_read(FLSID_PARA,FLS_ADDRESS_Private_Msg,330,InFlashMessageData);
		if(rspCode == apiOK)
		{
			if(	((InFlashMessageData[0]==0xFF) && (InFlashMessageData[1]==0xFF)) ||
				((InFlashMessageData[0]==0x00) && (InFlashMessageData[1]==0x00)))	//empty, Load Defaule
			{
				flgSetDefault_Private=TRUE;
			}
			else
			{
				curMsgNumber = 0;
				msgNumber = 5;
				ptrData = InFlashMessageData;
				do 
				{
					if((*ptrData > 30)&&(*ptrData < 36))	//20140103 if message ID is 1~20, return VAP_RIF_RC_INVALID_DATA
					{
						rspCode=api_pcd_vap_Set_DisplayMessage(ptrData[0], ptrData[1], &ptrData[2]);
						if (rspCode == SUCCESS)
						{
							msgLen=ptrData[1];
							ptrData+=(1+1+msgLen);	//ID(1)+Length(1)
							curMsgNumber++;
						}
						else
						{
							flgSetDefault_Private=TRUE;
							break;
						}
					}
					else
					{
						flgSetDefault_Private=TRUE;
						break;
					}
				} while (curMsgNumber < msgNumber);	
			}
		}
	}

	if (flgSetDefault_Private == TRUE)
	{
		total_msg_len = Def_Private_Message[0]*256+Def_Private_Message[1] - 1;		//total message ID num

		msgNumber=Def_Private_Message[2];
		ptrData=&Def_Private_Message[3];

		for(i=0;i<VAP_MESSAGE_NUMBER;i++)
		{
			if(glv_vap_Message[i].ID == *ptrData)
			{
				ptrData++;	//shift to length
				
				glv_vap_Message[i].Length = *ptrData;
				ptrData++;	//shift to value

				memcpy(glv_vap_Message[i].Message,ptrData,glv_vap_Message[i].Length);
				ptrData+=glv_vap_Message[i].Length;

				total_msg_len-=1+1+glv_vap_Message[i].Length;
			}
			if(!total_msg_len)
				break;
		}
	}

#endif
}


void APP_Set_Default_QuickPassMultipleAIDData(void)
{
#ifndef _PLATFORM_AS350

	UCHAR	rspCode=apiOK;
	UCHAR	datEnable[1]={0xFF};
	UINT	datLen=0;
	UCHAR	datBuffer[FLS_SIZE_MULTIAID_DATA-2]={0};


	//Load Flag
	
#ifdef _PLATFORM_AS210
	rspCode=api_fls_memtype(FLSID_PARA, FLSType_SRAM);
#endif

	if (rspCode == apiOK)
	{
		rspCode=api_fls_read(FLSID_PARA, FLS_ADDRESS_MULTIAID_ENABLE, 1, datEnable);
		if (rspCode == apiOK)
		{
			if ((datEnable[0] == TRUE) || (datEnable[0] == FALSE))
			{
				etp_flgMtiAID_UPI=datEnable[0];
			}
		}
	}

	//Load AID Data
	if (etp_flgMtiAID_UPI == TRUE)
	{
		rspCode=FLS_Read_QuickPassMultipleAIDData(&datLen, datBuffer);
		if (rspCode == SUCCESS)
		{
			rspCode=api_pcd_vap_Check_MultipleAIDData(datLen, datBuffer, FLS_NUMBER_AID_SET_UPI);
			if (rspCode == SUCCESS)
			{
				rspCode=api_pcd_vap_Set_QuickPayMultipleAIDData(datLen, datBuffer);
			}
		}
	}

#endif
}


void APP_Set_Default_DPASMultipleAIDData(void)
{
#ifndef _PLATFORM_AS350

	UCHAR	rspCode=apiOK;
	UCHAR	datEnable[1]={0xFF};
	UINT	datLen=0;
	UCHAR	datBuffer[FLS_SIZE_MULTIAID_DATA_DPAS-2]={0};


	//Load Flag
	
#ifdef _PLATFORM_AS210
	rspCode=api_fls_memtype(FLSID_PARA, FLSType_SRAM);
#endif

	if (rspCode == apiOK)
	{
		rspCode=api_fls_read(FLSID_PARA, FLS_ADDRESS_MULTIAID_ENABLE_DPAS, 1, datEnable);
		if (rspCode == apiOK)
		{
			if ((datEnable[0] == TRUE) || (datEnable[0] == FALSE))
			{
				etp_flgMtiAID_Discover=datEnable[0];
			}
		}
	}

	//Load AID Data
	if (etp_flgMtiAID_Discover == TRUE)
	{
		rspCode=FLS_Read_DPASMultipleAIDData(&datLen, datBuffer);
		if (rspCode == SUCCESS)
		{
			rspCode=api_pcd_vap_Check_MultipleAIDData(datLen, datBuffer, FLS_NUMBER_AID_SET_DISCOVER);
			if (rspCode == SUCCESS)
			{
				rspCode=api_pcd_vap_Set_DPASMultipleAIDData(datLen, datBuffer);
			}
		}
	}

#endif
}


void APP_Check_ProtocolIndex(void)
{
#ifndef _PLATFORM_AS350
	//Check the Index and change it
	if((((VAP_VISA_P_READER_INDEX & 0xFF00)>>8) == 0x0E) && ((VAP_VISA_P_READER_INDEX & 0x00FF) != 0x00))
	{
		os_WAVE_TXI_H = (VAP_VISA_P_READER_INDEX & 0xFF00)>>8;
		os_WAVE_TXI_L = VAP_VISA_P_READER_INDEX & 0x00FF;
	}
	else	//set to default
	{
		os_WAVE_TXI_H = 0x0E;
		os_WAVE_TXI_L = 0x01;
		VAP_VISA_P_READER_INDEX = 0x0E01;
	}
#endif
}


void APP_Open_Buzzer(void)
{
	if (app_flgBuzzer == FALSE)
	{
		UT_OpenBuzzer_1S();
		app_flgBuzzer=TRUE;
	}
}


void APP_Check_PollingTimeout(void)
{
	if ((api_pcd_vap_flgMutAuthenticate == TRUE) &&
		(main_card_remove == TRUE) &&
		(main_txn_complete == TRUE))
	{
		//check Main_VAP_POLL_A_Flag every VAP_VISA_P_POLL_MSG sec
		if(Main_VAP_POLL_A_Flag)
		{
			Main_VAP_POLL_A_Flag = FALSE;
			
			Timer_Poll_1 = OS_GET_SysTimerFreeCnt();
		}
		else
		{
			Timer_Poll_2 = OS_GET_SysTimerFreeCnt();

			if ((Timer_Poll_2 - Timer_Poll_1) > VAP_VISA_P_POLL_MSG)
			{
				main_clean_screen = TRUE;
				api_pcd_vap_flgMutAuthenticate = FALSE;

				//20140108
				api_pcd_vap_flgAdmMode = FALSE;
				api_pcd_vap_flgDbgAndOptMode = FALSE;
				//20140108 end
				Timer_Poll_1 = OS_GET_SysTimerFreeCnt();
			}
		}
	}
}


void APP_Show_Welcome(void)
{
	UCHAR Message1[64] = {0x00};
	UCHAR Message2[64] = {0x00};

	UCHAR Msg_Len1 = 0;
	UCHAR Msg_Len2 = 0;

	UCHAR F_Message1[32] = {0x00};
	UCHAR F_Message2[32] = {0x00};

	UCHAR F_Msg_Len1 = 0;
	UCHAR F_Msg_Len2 = 0;

	UCHAR S_Message1[32] = {0x00};
	UCHAR S_Message2[32] = {0x00};

	UCHAR S_Msg_Len1 = 0;
	UCHAR S_Msg_Len2 = 0;

	UCHAR disp_line = 0;

#ifdef _SCREEN_SIZE_128x64
	UCHAR base_line=0;
#else
	UCHAR base_line=1;
#endif

	if (vap_rif_flgMifare == FALSE)
	{
		//Turn ALL LED off, Blue on
		UT_Set_LED(IID_LED_BLUE);

		//Show Welcome Message
		Msg_Len1 = Msg_Len2 = F_Msg_Len1 = F_Msg_Len2 = S_Msg_Len1 = S_Msg_Len2 = 0;
		
		UT_Handle_2Type_Message(glv_vap_Message[0].Message,(UINT)glv_vap_Message[0].Length,Message1,&Msg_Len1,Message2,&Msg_Len2);

		if(Msg_Len1)
			UT_Handle_2Line_Message(Message1,(UINT)Msg_Len1,F_Message1,&F_Msg_Len1,F_Message2,&F_Msg_Len2);
		if(Msg_Len2)
			UT_Handle_2Line_Message(Message2,(UINT)Msg_Len2,S_Message1,&S_Msg_Len1,S_Message2,&S_Msg_Len2);

		if((F_Msg_Len1 && F_Msg_Len2)&& ((S_Msg_Len1==0) && (S_Msg_Len2==0)))
			disp_line = base_line+1;
		else if((S_Msg_Len1 && S_Msg_Len2)&& ((F_Msg_Len1==0) && (F_Msg_Len2==0)))
			disp_line = base_line+1;
		else if((F_Msg_Len1 && F_Msg_Len2) || (S_Msg_Len1 && S_Msg_Len2))
			disp_line = base_line+0;
		else
			disp_line = base_line+1;

		if((F_Msg_Len1 > 16) && (F_Msg_Len2 > 16)) 
			disp_line = base_line+0;

		if(F_Msg_Len1)
			UT_Disp_Show_Status(F_Msg_Len1,F_Message1,&disp_line);
							
		if(F_Msg_Len2)
			UT_Disp_Show_Status(F_Msg_Len2,F_Message2,&disp_line);
							
		if(S_Msg_Len1)
			UT_Disp_Show_Status(S_Msg_Len1,S_Message1,&disp_line);
							
		if(S_Msg_Len2)
			UT_Disp_Show_Status(S_Msg_Len2,S_Message2,&disp_line);
	}
}


void APP_Show_NotInUse(void)
{
	UCHAR msg_Sorry[]={"Sorry"};
	UCHAR msg_Not_In_Use[]={"Not In Use"};

#ifdef _SCREEN_SIZE_128x64
	UCHAR base_line=1;
#else
	UCHAR base_line=2;
#endif

	if (vap_rif_flgMifare == FALSE)
	{
		//Turn ALL LED off
		UT_Set_LED(0);

		UT_PutStr(base_line, (Display_MAX_Num-5)/2, FONT1, 5, msg_Sorry);
		UT_PutStr(base_line+1, (Display_MAX_Num-10)/2, FONT1, 10, msg_Not_In_Use);

#ifdef _SCREEN_SIZE_128x64
		api_sys_backlight(0,0x00000000);
#endif
	}
}


void APP_Show_IdleStatus(void)
{
	if (main_clean_screen == TRUE)
	{
		UT_ClearScreen();
		main_clean_screen = FALSE;

		if (api_pcd_vap_flgMutAuthenticate == TRUE)
		{
			APP_Show_Welcome();
		}
		else
		{
			APP_Show_NotInUse();
		}
	}
}


void APP_L3_VISA_ReaderInterface(void)
{
	UCHAR	buffer[20] = {0};
	//Load SID
	api_sys_info( SID_TerminalSerialNumber, buffer );
	// modify in order to prevent the wrong empty buffer value
	if(buffer[0] > 3)
		memcpy(Mifare_Rdr_Serial_Num,&buffer[4],buffer[0]-3);

	//Set Parameters
	APP_Set_Default_V3_Parameter();
	APP_Set_Default_EMV_Tags();
	APP_Set_Default_Message();
	APP_Set_Default_Scheme();
	APP_Set_Default_CVM();
	APP_Set_Default_VLP_Suppoted();
//	APP_Set_Default_BaudRate();		// 2020-06-04, e-Order default baud rate = 115200 bps
	APP_Set_Default_PayPassCfgData();
	APP_Set_Default_Exception_File();
	APP_Set_Default_APID();
	APP_Set_Default_DRLEnable();
	APP_Set_Default_Cash();
	APP_Set_Default_CashBack();
	APP_Set_Default_MSDOption();
	APP_Set_Default_Revo_List();
	APP_Set_Default_QuickPassMultipleAIDData();
	APP_Set_Default_DPASMultipleAIDData();

	//Initialize entrypoint combination table
	ETP_Initialize();

#ifdef _PLATFORM_AS350
	api_pcd_vap_flgMutAuthenticate = TRUE;
#else
	APP_Check_ProtocolIndex();
	VAP_RIF_Open_AUX();	
	
	main_clean_screen = TRUE;
	main_card_remove = TRUE;
	main_txn_complete = TRUE;

#ifdef _PLATFORM_AS350_LITE

#ifdef _SCREEN_SIZE_240x320
	UT_ClearRow(6, 3, FONT0);
	MFC_lcdtft_showSymLinkLogo();
#endif

#ifndef _ACQUIRER_CTCB
	UT_OpenKeyAll();
#endif

#ifdef _ACQUIRER_NCCC
	app_flgActivated=APP_Check_NCCC_Activation();
	app_flgStaUpdate=TRUE;
#endif

#endif

	while (1)
	{

#ifdef _SCREEN_SIZE_128x64
		APP_Show_IdleStatus();
#endif

#ifdef _PLATFORM_AS350_LITE
#ifdef _ACQUIRER_NCCC
		APP_Show_NCCC_ActivationStatus();
#endif
#endif

		if (!Mifare_File_Download_Flag)
		{
			APP_Check_PollingTimeout();
		}	

		VAP_RIF_Get_ReaderInterfaceInstruction();

#ifdef _PLATFORM_AS350_LITE

#ifdef _ACQUIRER_NCCC
		APP_Check_NCCC_Function();
#endif

#ifdef _ACQUIRER_FISC
		APP_Check_FISC_Function();
#endif

#endif

	}
#endif
}

void APP_Open_ContactlessInterface(void)
{
	UCHAR rspCode;

	rspCode=ECL_LV1_OpenCL();
	printf("after ECL_LV1_OpenCL\n");
	rspCode=ECL_LV1_InitialCL();
	printf("after ECL_LV1_InitialCL\n");
	//Open Beep Sound
	APP_Open_Buzzer();

#ifndef _PLATFORM_AS210
	dhn_lcd = 0;	// assign the lcd device number directly
	dhn_ICC = 0xE0;
#endif
}

void APP_Close_ContactlessInterface(void)
{
	ECL_LV1_CloseCL();
}


