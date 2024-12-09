#include <string.h>
#include "Glv_ReaderConfPara.h"
#include "VAP_ReaderInterface_Define.h"
#include "api_pcd_vap_Command.h"
#include "api_pcd_mfc_Command.h"
#include "Function.h"
#include "MIFARE_Define.h"
#include "FLSAPI.h"
#include "UTILS.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Function.h"


#ifdef _PLATFORM_AS210
#include "xioapi.h"
#endif

#ifdef _PLATFORM_AS350_LITE

// #include "MFC_Function.h"

#ifdef _PAYMENT_IPASS_VERSION_1
#include "IPASS1\IPASS_MP_API.h"

#ifdef _ACQUIRER_NCCC
#include "IPASS\IpassMP.h"
#endif
#endif

#ifdef _PAYMENT_IPASS_VERSION_2
#include "IPASS2\IPASS2_Function.h"
#endif

#endif

//20140113 Fix the reset behavior during Ready for sale
extern UCHAR main_clean_screen;
extern UCHAR main_txn_complete;
extern UCHAR main_card_remove;

//20140105 POLL_A flag 
extern UCHAR Main_VAP_POLL_A_Flag;

extern UINT	etp_tmrSale;	//P_SALE_TIMEOUT


#ifndef _PLATFORM_AS350
extern volatile UCHAR os_WAVE_SEQNO;
extern volatile UCHAR os_WAVE_RXI_H;
extern volatile UCHAR os_WAVE_RXI_L;
extern volatile UCHAR os_WAVE_TXI_H;
extern volatile UCHAR os_WAVE_TXI_L;
#endif

extern ULONG	OS_GET_SysTimerFreeCnt( void );
extern void		OS_SET_SysTimerFreeCnt( ULONG value );
extern UCHAR	Mifare_Burn_File(void);
extern void		Mifare_Check_BlinkLEDTimeout(void);

#ifdef _PLATFORM_AS350_LITE
#ifdef _ACQUIRER_NCCC
extern void		NCCC_Lite_Reset(void);
#endif
#endif

UCHAR	vap_rif_dhnAUX=0;
UINT	vap_rif_rcvLen=0;
UCHAR	vap_rif_rcvBuffer[VAP_RIF_BUFFER_SIZE];
UCHAR	vap_rif_sndBuffer[VAP_RIF_BUFFER_SIZE];

UCHAR	vap_rif_flgMifare=FALSE;	//Flag of Mifare Command

//20140115 Reset Flag For Display
UCHAR	VAP_Reset_Display_Flag = FALSE;

//20140105 Reader Index Changed?
UCHAR	VAP_Reader_Index_Change = FALSE;

//20131216 we create this flag for the situation that when the reader waiting the card present, acquirer prees the cancel button or send a reset command.
UCHAR	VAP_Reset_Command_Occured;

//20131212 VAP Parameter
UINT	VAP_VISA_P_MSG_TIMEOUT;
UINT	VAP_VISA_P_SALE_TIMEOUT;
ULONG	VAP_VISA_P_POLL_MSG;
UINT	VAP_VISA_P_BUF_TIMEOUT;
UCHAR	VAP_VISA_P_ENCRYPTION;
UCHAR	VAP_VISA_P_DISPLAY;
UINT	VAP_VISA_P_MAX_BUF_SIZE;
UINT	VAP_VISA_P_DOUBLE_DIP;
UINT	VAP_VISA_P_READER_INDEX;
UCHAR	VAP_VISA_P_LAUGUAGE[16];
UINT	VAP_VISA_P_DISPLAY_S_MSG;
UINT	VAP_VISA_P_DISPLAY_L_MSG;
UINT	VAP_VISA_P_DISPLAY_SS_MSG;
UINT	VAP_VISA_P_DISPLAY_SR_MSG;
UINT	VAP_VISA_P_DISPLAY_PIN_MSG;
UINT	VAP_VISA_P_DISPLAY_E_MSG;



// ADD by Wayne 2020/08/21 (just for reduce the compiler warning)
// It's also can give read some information about the funciton
// which function is include from outside
// <-------
extern void api_pcd_mfc_buz_Beep_Interval(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void api_pcd_mfc_msr_Open(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void api_pcd_mfc_msr_Close(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void api_pcd_mfc_msr_Status(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void api_pcd_mfc_msr_Read(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
// <-------

#if 0
void VAP_RIF_MFC_RFOff(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_con_RFOff(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;	

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_RFOn(void)
{
	UINT OptLen = 0;
	
	api_pcd_mfc_con_RFOn(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_REQA(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_con_REQA(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_WUPA(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_con_WUPA(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_AntiCol(void)
{
	UINT OptLen = 0;
	
	api_pcd_mfc_con_AntiCol(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_Select(void)
{
	UINT OptLen = 0;
	
	api_pcd_mfc_con_Select(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_PollingCard(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_con_Polling(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_GetCardData(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_con_GetCardData(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_WUPB(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_con_WUPB(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_Authenticate(void)
{
	UINT OptLen = 0;
	
	api_pcd_mfc_con_Authenticate(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_Read(void)
{
	UINT OptLen = 0;
	
	api_pcd_mfc_con_Read(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_Write(void)
{
	UINT OptLen = 0;
	
	api_pcd_mfc_con_Write(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_Value(void)
{
	UINT OptLen = 0;
	
	api_pcd_mfc_con_Value(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_Restore(void)
{
	UINT OptLen = 0;
	
	api_pcd_mfc_con_Restore(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_Transfer(void)
{
	UINT OptLen = 0;
	
	api_pcd_mfc_con_Transfer(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_TypeATransparent(void)
{
	UINT OptLen = 0;
	
	api_pcd_mfc_con_TypeATransparent(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_TypeBTransparent(void)
{
	UINT OptLen = 0;
	
	api_pcd_mfc_con_TypeBTransparent(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}
#endif
void VAP_RIF_MFC_ChangeBaudrate(void)
{
#if	0
	UINT OptLen = 0;
	UINT preBaud = 0;
	
	preBaud = glv_vap_BaudRate;

	api_pcd_mfc_aux_ChangeBaudrate(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_RS232;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

	//20131101
	UT_WaitTime(50);
	
	if(preBaud != glv_vap_BaudRate)
	{
		if(vap_rif_sndBuffer[3] == Mifare_RC_Success)
		{		
			if(VAP_RIF_Close_AUX())
			{
				if(VAP_RIF_Open_AUX())
					UT_BUZ_Beep1();
			}
		}
	}
#endif
}

void VAP_RIF_MFC_GetReaderInformation(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_sys_GetReaderInformation(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_System;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_GetFirmwareInformation(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_sys_GetFirmwareInformation(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_System;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_UpdateFileInformation(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_fil_UpdateFileInformation(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Custom_Command;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_ReceiveFile(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_fil_ReceiveFile(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Custom_Command;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_GetCAPKIndex(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_pro_GetCAPKIndex(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Custom_Command;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_DeleteAllCAPKKey(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_pro_DeleteAllCAPKKey(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Custom_Command;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}
#if 0
void VAP_RIF_MFC_GetContactlessChipSerialNumber(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_sys_GetContactlessChipSerialNumber(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_System;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}
#endif
void VAP_RIF_MFC_GetDateTime(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_sys_GetDateTime(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_System;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_SetDateTime(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_sys_SetDateTime(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_System;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_ClearScreen(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_lcd_ClearScreen(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_ClearLine(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_lcd_ClearLine(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_ClearLineAlphanumeric(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_lcd_ClearLineAlphanumeric(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_WriteLine(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_lcd_WriteLine(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_WriteLineAlphanumeric(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_lcd_WriteLineAlphanumeric(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_BackLight(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_lcd_SwitchBackLight(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_ShowContactlessSymbol(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_lcd_ShowContactlessSymbol(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_SetBGPalette(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_lcd_SetBGPalette(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_SetLED(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_led_SetLED(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_MFC_BlinkLED(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_led_BlinkLED(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_Beep(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_buz_Beep(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_Beep_Interval(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_buz_Beep_Interval(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_ECHOTest(void)
{
#if	0
	UINT OptLen = 0;

	api_pcd_mfc_aux_ECHOTest(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_RS232;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}
#if 0
void VAP_RIF_MFC_ActivateCard(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_iso_ActivateCard(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_GetStatus(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_iso_GetStatus(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_TransmitAPDU(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_iso_TransmitAPDU(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_CardRemoval(void)
{
	UINT OptLen = 0;

	api_pcd_mfc_iso_CardRemoval(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	OptLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(OptLen & 0x00FF);
	vap_rif_sndBuffer[1]=(OptLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_Cless_Card_Interface;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}
#endif

void VAP_RIF_MFC_RestoreFunction(void)
{
#if	0
	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;	
	vap_rif_sndBuffer[2]=vap_rif_rcvBuffer[4];
	vap_rif_sndBuffer[3]=Mifare_RC_Fail;
	vap_rif_sndBuffer[4]=0;
	vap_rif_sndBuffer[5]=0;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}


#ifdef _PLATFORM_AS350
#ifdef _SCREEN_SIZE_320x240

void VAP_RIF_MFC_LCDGraphics(void)
{
#if	0
	UINT optLen=0;

	api_pcd_mfc_lcd_Graphics(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_LCDShow(void)
{
#if	0
	UINT optLen=0;

	api_pcd_mfc_lcd_Show(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=Mifare_Channel_Human_Interface;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_MacroMSR(void)
{
	UINT	optLen=0;
	UCHAR	result;
	
#if 0
	result = EO_MacroMSR();


	optLen = 1+1;

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_ESC;
	vap_rif_sndBuffer[3]=result;	// 0=OK, 1=FAIL
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_MacroICC(void)
{
	UINT optLen=0;
	UCHAR	result;
	
#if 0
	result = EO_MacroICC();


	optLen = 1+1;

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_ESC;
	vap_rif_sndBuffer[3]=result;	// 0=OK, 1=FAIL
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_MacroCTLS(void)
{
	UINT optLen=0;
	UCHAR	result;
#if 0 	
	result = EO_MacroCTLS();


	optLen = 1+1;

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_ESC;
	vap_rif_sndBuffer[3]=result;	// 0=OK, 1=FAIL
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_OpenMSR(void)
{
	UINT optLen=0;
	
	api_pcd_mfc_msr_Open(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_MSR;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_CloseMSR(void)
{
	UINT optLen=0;
	
	api_pcd_mfc_msr_Close(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_MSR;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_GetMSRStatus(void)
{
	UINT optLen=0;
	
	api_pcd_mfc_msr_Status(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_MSR;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_ReadMSR(void)
{
	UINT optLen=0;
	
	api_pcd_mfc_msr_Read(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_MSR;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_OpenKBD(void)
{
#if	0
	UINT optLen=0;
	
	api_pcd_mfc_kbd_Open(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_KBD;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_CloseKBD(void)
{
#if	0
	UINT optLen=0;
	
	api_pcd_mfc_kbd_Close(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_KBD;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_GetKBDStatus(void)
{
#if	0
	UINT optLen=0;
	
	api_pcd_mfc_kbd_Status(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_KBD;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_GetKBD(void)
{
#if	0
	UINT optLen=0;
	
	api_pcd_mfc_kbd_Get(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_KBD;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_GetcKBD(void)
{
#if	0
	UINT optLen=0;
	
	api_pcd_mfc_kbd_Getc(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_KBD;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_MFC_GetsKBD(void)
{
#if	0
	UINT optLen=0;
	
	api_pcd_mfc_kbd_Gets(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_KBD;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

#ifdef	_BCR_
void VAP_RIF_MFC_OpenBCR(void)
{
	UINT optLen=0;
	
	api_pcd_mfc_bcr_Open(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_BCR;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}
#endif

#ifdef	_BCR_
void VAP_RIF_MFC_CloseBCR(void)
{
	UINT optLen=0;
	
	api_pcd_mfc_bcr_Close(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_BCR;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}
#endif

#ifdef	_BCR_
void VAP_RIF_MFC_ReadBCR(void)
{
	UINT optLen=0;
	
	api_pcd_mfc_bcr_Read(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_Channel_BCR;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}
#endif

#endif
#endif


#ifdef _PLATFORM_AS350_LITE
#ifdef _ACQUIRER_NCCC

void VAP_RIF_MFC_NCCC_GetParameterInformation(void)
{
	UINT optLen=0;
	
	api_pcd_mfc_n3c_GetParameterInformation(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_NCCC_PARAMETER_INFORMATION;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_NCCC_ShowError(void)
{
	UINT optLen=0;
	
	api_pcd_mfc_n3c_ShowError(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_NCCC_SHOW_ERROR;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_MFC_NCCC_Reset(void)
{
	UINT optLen=0;
	
	api_pcd_mfc_n3c_Reset(&vap_rif_rcvBuffer[6], &vap_rif_rcvBuffer[8], &vap_rif_sndBuffer[0], &vap_rif_sndBuffer[3]);

	optLen = vap_rif_sndBuffer[0]*256 + vap_rif_sndBuffer[1] + 1;	// 1 for channel	

	vap_rif_sndBuffer[0]=(optLen & 0x00FF);
	vap_rif_sndBuffer[1]=(optLen & 0xFF00)>>8;	
	vap_rif_sndBuffer[2]=Mifare_NCCC_RESET;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

	if (vap_rif_sndBuffer[3] == Mifare_RC_Success)
	{
		NCCC_Lite_Reset();
	}
}

#endif
#endif

void VAP_RIF_Reader_RestoreFunction(void)
{
#if	0
	UINT preBaud = 0;

	preBaud = COM_38400;

	UT_WaitTime(50);
	
	if(preBaud != glv_vap_BaudRate)
	{
		glv_vap_BaudRate = COM_38400;
		
		if(VAP_RIF_Close_AUX())
		{
			if(VAP_RIF_Open_AUX())
				UT_BUZ_Beep1();
		}
	}
#endif
}

void VAP_RIF_LV3_RestoreFunction(void)
{
#if	0
	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	
	vap_rif_sndBuffer[2]=vap_rif_rcvBuffer[2];
	vap_rif_sndBuffer[3]=0;
	vap_rif_sndBuffer[4]=1;
	vap_rif_sndBuffer[5]=VAP_RIF_RC_FAILURE;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

UCHAR VAP_RIF_Close_AUX(void)
{
	UCHAR rspCode = 0;

	rspCode = api_aux_close(vap_rif_dhnAUX);
	if(rspCode == apiOK)
		return SUCCESS;

	return FAIL;
}


UCHAR VAP_RIF_Open_AUX(void)
{
	API_AUX	sbuf;

	sbuf.Mode		=auxVISAWAVE;
	sbuf.Baud 		=glv_vap_BaudRate+COM_CHR8+COM_NOPARITY+COM_STOP1;
	sbuf.Tob		=1000;	// 10; 2020-05-22 change to 10sec for e-Order ESC macro command (may enter command manually via HyperTermial)
	sbuf.Tor		=50;
	sbuf.Acks		=0;
	sbuf.Resend		=0;
	sbuf.BufferSize	=VAP_RIF_BUFFER_SIZE;

#ifdef _PLATFORM_AS350_LITE
	vap_rif_dhnAUX=api_aux_open(COM2, sbuf);
#else
	vap_rif_dhnAUX=api_aux_open(COM0, sbuf);
#endif

	return vap_rif_dhnAUX;
}


void VAP_RIF_Reset_Buffer(void)
{
	vap_rif_rcvLen=0;
	memset(vap_rif_rcvBuffer, 0, VAP_RIF_BUFFER_SIZE);
	memset(vap_rif_sndBuffer, 0, VAP_RIF_BUFFER_SIZE);
}

//Test Disable
#if 0

void VAP_RIF_POL_POLL(void)
{
	api_pcd_vap_pol_POLL(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_POL_POLL;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_POL_Echo(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_pol_Echo(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_POL_Echo;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_DEB_SetDebugAndOptimizationMode(void)
{
	api_pcd_vap_deb_SetDebugAndOptimizationMode(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_DEB_SetDebugAndOptimizationMode;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);	
}


void VAP_RIF_DEB_SetParameters(void)
{
	api_pcd_vap_deb_SetParameters(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_DEB_SetParameters;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

#ifndef _PLATFORM_AS350
	//20140105 set Index after sending response
	if(VAP_Reader_Index_Change)
	{
		os_WAVE_TXI_H = (VAP_VISA_P_READER_INDEX & 0xFF00)>>8;

		os_WAVE_TXI_L = VAP_VISA_P_READER_INDEX & 0x00FF;

		//20140107 reset this flag
		VAP_Reader_Index_Change = FALSE;
	}
#endif	
}


void VAP_RIF_AUT_InitializeCommunication(void)
{
	UINT tmpLen;

	api_pcd_vap_aut_InitializeCommunication(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_AUT_InitializeCommunication;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_AUT_MutualAuthenticate(void)
{
	api_pcd_vap_aut_MutualAauthenticate(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_AUT_MutualAuthenticate;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

	//20140105 make sure VAP_VISA_P_POLL_MSG funciton correctly
	if(vap_rif_sndBuffer[5] == VAP_RIF_RC_SUCCESS)
		Main_VAP_POLL_A_Flag = TRUE;
}


void VAP_RIF_AUT_GenerateKey(void)
{
	api_pcd_vap_aut_GenerateKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_AUT_GenerateKey;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_AUT_InvalidateReader(void)
{
	api_pcd_vap_aut_InvalidateReader(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_AUT_InvalidateReader;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_TRA_ReadyForSalesTransaction(void)
{
	UINT	tmpLen=0;
	
#ifdef _SCREEN_SIZE_128x64
	api_sys_backlight(0,(ULONG)(etp_tmrSale+500));
#endif

	//20140115 V1 "A Reset Flag" for display
	VAP_Reset_Display_Flag = TRUE;
	
	api_pcd_vap_tra_ReadyForSale(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	//20140115 V1 "A Reset Flag" for display end
	VAP_Reset_Display_Flag = FALSE;
	
	//20131216 while receoving a reset command we dont send the ready for sale response to terminal
	if(VAP_Reset_Command_Occured)
	{
		VAP_Reset_Command_Occured = FALSE;
	}
	else
	{
		tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
		
		vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
		vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
		vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_TRA_ReadyForSalesTransaction;
		
		UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

		main_card_remove=TRUE;
		ECL_LV1_FIELD_OFF();
	}
}

//20140618 add for "GPO response 6986"
void VAP_RIF_TRA_ReadyForSales_TryAgain(void)
{
#ifndef _PLATFORM_AS350
	vap_rif_sndBuffer[0]=0x04;
	vap_rif_sndBuffer[1]=0x00;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_TRA_ReadyForSalesTransaction;
	vap_rif_sndBuffer[3]=0x00;
	vap_rif_sndBuffer[4]=0x01;
	vap_rif_sndBuffer[5]=VAP_RIF_RC_TRY_AGAIN;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
}

void VAP_RIF_TRA_Reset(void)
{
	api_pcd_vap_tra_Reset(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_TRA_Reset;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
	main_clean_screen = TRUE;

	//20130911
	main_card_remove = TRUE;
	main_txn_complete = TRUE;
}


void VAP_RIF_TRA_ShowStatus(void)
{
	api_pcd_vap_tra_ShowStatus(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_TRA_ShowStatus;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_TRA_Issuer_Update(void)
{
	UCHAR	rspCode=0;
	UINT	tmpLen=0;
	
	//20140115 V1 "A Reset Flag" for display end
	VAP_Reset_Display_Flag = TRUE;

	api_pcd_vap_tra_Issuer_Update(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	//20140115 V1 "A Reset Flag" for display end
	VAP_Reset_Display_Flag = FALSE;
	
	//20140304 while receoving a reset command we dont send the Issuer Update response to terminal
	if(VAP_Reset_Command_Occured)
	{
		VAP_Reset_Command_Occured = FALSE;
	}
	else
	{
		tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
		
		vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
		vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
		vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_TRA_Issuer_Update;

		UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

#ifdef _PLATFORM_AS350_LITE
		rspCode=ECL_LV1_SUCCESS;
#else
		rspCode=ECL_LV1_REMOVAL(0);
#endif

		if (rspCode == ECL_LV1_SUCCESS)
			main_card_remove=TRUE;

		ECL_LV1_FIELD_OFF();
	}
}

void VAP_RIF_ADM_SwitchToAdministrativeMode(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_SwitchToAdminMode(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SwitchToAdministrativeMode;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_ADM_GetCapability(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_GetCapability(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetCapability;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_ADM_SetCapability(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_SetCapability(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetCapability;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_ADM_GetDateAndTime(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_GetDateAndTime(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetDateAndTime;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_ADM_SetDateAndTime(void)
{
	api_pcd_vap_adm_SetDateAndTime(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetDateAndTime;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_ADM_GetParameters(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_GetParameters(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetParameters;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_ADM_GetEMVTagsValues(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_GetEMVTagsValues(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetEMVTagsValues;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetVisaPublicKey(void)
{
	UINT	tmpLen=0;
		
	api_pcd_vap_adm_SetVisaPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetVisaPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetVisaPublicKey(void)
{
	UINT	tmpLen=0;
		
	api_pcd_vap_adm_GetVisaPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetVisaPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetMasterPublicKey(void)
{
	UINT	tmpLen=0;
		
	api_pcd_vap_adm_SetMasterPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetMasterPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetMasterPublicKey(void)
{
	UINT	tmpLen=0;
		
	api_pcd_vap_adm_GetMasterPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetMasterPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_ADM_SetJCBPublicKey(void)
{
	UINT	tmpLen=0;
		
	api_pcd_vap_adm_SetJCBPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetJCBPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetJCBPublicKey(void)
{
	UINT	tmpLen=0;
		
	api_pcd_vap_adm_GetJCBPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetJCBPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

}


void VAP_RIF_ADM_SetUPIPublicKey(void)
{
	UINT	tmpLen=0;
		
	api_pcd_vap_adm_SetUPIPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetUPIPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetUPIPublicKey(void)
{
	UINT	tmpLen=0;
		
	api_pcd_vap_adm_GetUPIPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetUPIPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_ADM_SetVisaTestAIDPublicKey(void)
{
	UINT	tmpLen=0;
		
	api_pcd_vap_adm_SetVisaTestAIDPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetVisaTestAIDPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

}

void VAP_RIF_ADM_GetVisaTestAIDPublicKey(void)
{
	UINT	tmpLen=0;
		
	api_pcd_vap_adm_GetVisaTestAIDPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetVisaTestAIDPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

}

void VAP_RIF_ADM_SetEMVTagsValues(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_SetEMVTagsValues(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetEMVTagsValues;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_ADM_GetDisplayMessages(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_GetDisplayMessages(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetDisplayMessages;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_ADM_SetDisplayMessages(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_SetDisplayMessages(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetDisplayMessages;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetCVMCapability(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_GetCVMCapability(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetCVMCapability;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetCVMCapability(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_SetCVMCapability(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetCVMCapability;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetAPID(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_GetAPID(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4] + 3;

	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetAPID;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetAPID(void)
{
	UINT tmpLen=0;

	api_pcd_vap_adm_SetAPID(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen = vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4] + 3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetAPID;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetExceptionFile(void)
{
	UINT tmpLen=0;

	api_pcd_vap_adm_GetExceptionFile(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen = vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4] + 3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetExceptionFile;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetExceptionFile(void)
{
	UINT tmpLen=0;

	api_pcd_vap_adm_SetExceptionFile(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen = vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4] + 3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetExceptionFile;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetDRLEnable(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_GetDRLEnable(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetDRLEnable;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

}

void VAP_RIF_ADM_SetDRLEnable(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_SetDRLEnable(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetDRLEnable;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

}

void VAP_RIF_ADM_GetCashRRP(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_GetCashRRP(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetCashRRP;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetCashRRP(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_SetCashRRP(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetCashRRP;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetCashBackRRP(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_GetCashBackRRP(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetCashBackRRP;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetCashBackRRP(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_SetCashBackRRP(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetCashBackRRP;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_ADM_GetKeyRevoList(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_GetKeyRevoList(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetKeyRevoList;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetKeyRevoList(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_SetKeyRevoList(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetKeyRevoList;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_ADM_GetBaudRate(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_GetBaudRate(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetBaudRate;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetBaudRate(void)
{
	UINT	tmpLen=0;
	UCHAR	rspCode=0;
	UINT	preBaud = 0;

	preBaud = glv_vap_BaudRate;
	
	api_pcd_vap_adm_SetBaudRate(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetBaudRate;

	rspCode = UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

	//20140103 decrease the delay time
	//20131101
	UT_WaitTime(10);
	
	if(preBaud != glv_vap_BaudRate)
	{
		if((rspCode == SUCCESS)&&(vap_rif_sndBuffer[5] == VAP_RIF_RC_SUCCESS))
		{		
			if(VAP_RIF_Close_AUX())
			{
				if(VAP_RIF_Open_AUX())
					UT_BUZ_Beep1();
			}
		}
	}
}

void VAP_RIF_ADM_ResetAcquirerKey(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_ResetAcquirerKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_ResetAcquirerKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_ReaderRecover(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_ReaderRecover(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	//20140102 ICTK change the ID
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_ReaderRecovery;
	
	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetPayPassConfigurationData(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_GetPayPassConfigurationData(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetPayPassConfigurationData;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetPayPassConfigurationData(void)
{
	api_pcd_vap_adm_SetPayPassConfigurationData(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetPayPassConfigurationData;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetPayPassDataExchangeData(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_GetPayPassDataExchangeData(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetPayPassDataExchangeData;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetPayPassDataExchangeData(void)
{
	api_pcd_vap_adm_SetPayPassDataExchangeData(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetPayPassDataExchangeData;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetQuickPassMultipleAIDData(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_adm_GetQuickPassMultipleAIDData(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetQuickPassMultipleAIDData;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetQuickPassMultipleAIDData(void)
{
	api_pcd_vap_adm_SetQuickPassMultipleAIDData(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetQuickPassMultipleAIDData;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetQuickPassMultipleAIDEnable(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_GetQuickPassMultipleAIDEnable(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetQuickPassMultipleAIDEnable;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetQuickPassMultipleAIDEnable(void)
{
	api_pcd_vap_adm_SetQuickPassMultipleAIDEnable(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetQuickPassMultipleAIDEnable;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetMSDOption(void)
{
	UINT	tmpLen=0;	

	api_pcd_vap_adm_SetMSDOption(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen = vap_rif_sndBuffer[3]*256 + vap_rif_sndBuffer[4] + 3;		// 3 for TX Len(2 bytes) and command ID(1 byte)

	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetMSDOption;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

}

void VAP_RIF_ADM_GetMSDOption(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_GetMSDOption(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen = vap_rif_sndBuffer[3]*256 + vap_rif_sndBuffer[4] + 3;		// 3 for TX Len(2 bytes) and command ID(1 byte)

	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetMSDOption;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

}

void VAP_RIF_ADM_GetExpressPayDefaultDRL(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_GetExpressPayDefaultDRL(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;

	UT_S2C(tmpLen, vap_rif_sndBuffer);
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetExpressPayDefaultDRL;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetExpressPayDefaultDRL(void)
{
	UINT	tmpLen=0;	

	api_pcd_vap_adm_SetExpressPayDefaultDRL(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	UT_S2C(tmpLen, vap_rif_sndBuffer);
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetExpressPayDefaultDRL;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetExpressPayDRL(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_GetExpressPayDRL(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;

	UT_S2C(tmpLen, vap_rif_sndBuffer);
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetExpressPayDRL;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetExpressPayDRL(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_SetExpressPayDRL(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;

	UT_S2C(tmpLen, vap_rif_sndBuffer);
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetExpressPayDRL;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetExpressPayKeyRevoList(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_GetExpressPayKeyRevoList(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;

	UT_S2C(tmpLen, vap_rif_sndBuffer);
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetExpressPayKeyRevoList;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetExpressPayKeyRevoList(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_SetExpressPayKeyRevoList(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;

	UT_S2C(tmpLen, vap_rif_sndBuffer);
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetExpressPayKeyRevoList;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetExpressPayExceptionFile(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_GetExpressPayExceptionFile(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;

	UT_S2C(tmpLen, vap_rif_sndBuffer);
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetExpressPayExceptionFile;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetExpressPayExceptionFile(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_SetExpressPayExceptionFile(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;

	UT_S2C(tmpLen, vap_rif_sndBuffer);
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetExpressPayExceptionFile;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetAEPublicKey(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_GetAEPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;

	UT_S2C(tmpLen, vap_rif_sndBuffer);
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetAEPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetAEPublicKey(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_SetAEPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;

	UT_S2C(tmpLen, vap_rif_sndBuffer);
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetAEPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_GetDiscoverPublicKey(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_GetDiscoverPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;

	UT_S2C(tmpLen, vap_rif_sndBuffer);
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_GetDiscoverPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_ADM_SetDiscoverPublicKey(void)
{
	UINT	tmpLen=0;

	api_pcd_vap_adm_SetDiscoverPublicKey(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;

	UT_S2C(tmpLen, vap_rif_sndBuffer);
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_ADM_SetDiscoverPublicKey;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_PRO_KeyInjection(void)
{
	api_pcd_vap_pro_KeyInjection(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);
		
	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_PRO_KeyInjection;
	

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}

void VAP_RIF_PRO_SetVLPSupportIndicator(void)
{
	api_pcd_vap_pro_SetVLPSupportIndicator(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	vap_rif_sndBuffer[0]=4;
	vap_rif_sndBuffer[1]=0;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_PRO_SetVLPSupportIndicator;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_PRO_GetVLPSupportIndicator(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_pro_GetVLPSupportIndicator(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_PRO_GetVLPSupportIndicator;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

}


void VAP_RIF_SYS_SetReadyForSaleBeep(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_sys_SetReadyForSaleBeep(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_SYS_SetReadyForSaleBeep;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_SYS_SetRFRangeUnder4cm(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_sys_SetRFRangeUnder4cm(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_SYS_SetRFRangeUnder4cm;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_SYS_SetRFRangeEMV(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_sys_SetRFRangeEMV(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_SYS_SetRFRangeEMV;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}


void VAP_RIF_SYS_SetJCBRefundResponse(void)
{
	UINT	tmpLen=0;
	
	api_pcd_vap_sys_SetJCBRefundResponse(&vap_rif_rcvBuffer[3], &vap_rif_rcvBuffer[5], &vap_rif_sndBuffer[3], &vap_rif_sndBuffer[5]);

	tmpLen=vap_rif_sndBuffer[3]*256+vap_rif_sndBuffer[4]+3;
	
	vap_rif_sndBuffer[0]=tmpLen & 0x00FF;
	vap_rif_sndBuffer[1]=(tmpLen & 0xFF00)>>8;
	vap_rif_sndBuffer[2]=VAP_RIF_INSTRUCTION_SYS_SetJCBRefundResponse;

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
}
#endif

void VAP_RIF_Switch_MifareCommand(UCHAR iptChannel, UCHAR iptCommand)
{
	vap_rif_flgMifare=TRUE;
	
	switch (iptChannel)
	{
		case Mifare_Channel_Human_Interface:
			switch (iptCommand)
			{
#ifdef _PLATFORM_AS350
#ifdef _SCREEN_SIZE_320x240
				case Mifare_HI_Clear_Screen:				VAP_RIF_MFC_ClearScreen();				break;
				case Mifare_HI_Clear_Line:					VAP_RIF_MFC_ClearLine();				break;
				case Mifare_HI_Write_Line:					VAP_RIF_MFC_WriteLine();				break;
				case Mifare_HI_Turn_On_Off_LCD_Backlight:	VAP_RIF_MFC_BackLight();				break;
				case Mifare_HI_Beep:						VAP_RIF_MFC_Beep();						break;
				case Mifare_HI_Set_LED:						VAP_RIF_MFC_SetLED();					break;
				case Mifare_HI_Blink_LED:					VAP_RIF_MFC_BlinkLED();					break;

				case 0x05:									VAP_RIF_MFC_LCDGraphics();				break;
				case 0x06:									VAP_RIF_MFC_LCDShow();					break;
				case 0x11:									VAP_RIF_MFC_Beep_Interval();			break;
				default:	break;

#endif
#else
				case Mifare_HI_Clear_Screen:				VAP_RIF_MFC_ClearScreen();				break;
				case Mifare_HI_Clear_Line:					VAP_RIF_MFC_ClearLine();				break;
				case Mifare_HI_Write_Line:					VAP_RIF_MFC_WriteLine();				break;
				case Mifare_HI_Turn_On_Off_LCD_Backlight:	VAP_RIF_MFC_BackLight();				break;
				case Mifare_HI_Clear_Line_Alphanumeric:		VAP_RIF_MFC_ClearLineAlphanumeric();	break;
				case Mifare_HI_Write_Line_Alphanumeric:		VAP_RIF_MFC_WriteLineAlphanumeric();	break;				
				case Mifare_HI_Beep:						VAP_RIF_MFC_Beep();						break;
				case Mifare_HI_Set_LED:						VAP_RIF_MFC_SetLED();					break;
				case Mifare_HI_Blink_LED:					VAP_RIF_MFC_BlinkLED();					break;
				case Mifare_HI_Show_ContactlessSymbol:		VAP_RIF_MFC_ShowContactlessSymbol();	break;
				case Mifare_HI_Set_BG_Palette:				VAP_RIF_MFC_SetBGPalette();				break;

				default:	break;
#endif

			}
			
			break;
			
		case Mifare_Channel_Contact_Card_Interface:		break;

/*		case Mifare_Channel_Cless_Card_Interface:
			switch(iptCommand)
			{
				case Mifare_Cless_RF_Off:				VAP_RIF_MFC_RFOff();			break;
				case Mifare_Cless_RF_On:				VAP_RIF_MFC_RFOn();				break;
				case Mifare_Cless_REQ_A:				VAP_RIF_MFC_REQA();				break;
				case Mifare_Cless_WUP_A:				VAP_RIF_MFC_WUPA();				break;
				case Mifare_Cless_ANTICOL:				VAP_RIF_MFC_AntiCol();			break;
				case Mifare_Cless_Select:				VAP_RIF_MFC_Select();			break;
				case Mifare_Cless_Polling_Card:			VAP_RIF_MFC_PollingCard();		break;
				case Mifare_Cless_Get_Card_Data:		VAP_RIF_MFC_GetCardData();		break;
				case Mifare_Cless_3PASS_Auth:			VAP_RIF_MFC_Authenticate();		break;								
				case Mifare_Cless_Read:					VAP_RIF_MFC_Read();				break;
				case Mifare_Cless_Write:				VAP_RIF_MFC_Write();			break;
				case Mifare_Cless_Value:				VAP_RIF_MFC_Value();			break;
				case Mifare_Cless_Restore:				VAP_RIF_MFC_Restore();			break;
				case Mifare_Cless_Transfer:				VAP_RIF_MFC_Transfer();			break;
				case Mifare_Cless_Type_A_Transparent:	VAP_RIF_MFC_TypeATransparent();	break;
				case Mifare_Cless_Type_B_Transparent:	VAP_RIF_MFC_TypeBTransparent();	break;
				case Mifare_Cless_Activate_Card:		VAP_RIF_MFC_ActivateCard();		break;
				case Mifare_Cless_Transmit_APDU:		VAP_RIF_MFC_TransmitAPDU();		break;
				case Mifare_Cless_Get_Status:			VAP_RIF_MFC_GetStatus();		break;
				case Mifare_Cless_Card_Removal:			VAP_RIF_MFC_CardRemoval();		break;
				case Mifare_Cless_WUPB:					VAP_RIF_MFC_WUPB();				break;

				default:	break;
			}
		
			break;
*/
		case Mifare_Channel_Crypto_Component:			break;

		case Mifare_Channel_RS232:
			switch(iptCommand)
			{
				case Mifare_RS232_ECHO_TEST:		VAP_RIF_MFC_ECHOTest();			break;
				case Mifare_RS232_Change_Baudrate:	VAP_RIF_MFC_ChangeBaudrate();	break;
				
				default:	break;
			}
		

			break;

		case Mifare_Channel_System:
			switch(iptCommand)
			{
				case Mifare_SYS_Get_Sys_Command:					VAP_RIF_MFC_GetReaderInformation();				break;
				case Mifare_SYS_Inject_Encry_Key:																	break;
				case Mifare_SYS_Enable_Manual_Resource_Control:														break;
				case Mifare_SYS_Replace_Default_LOGO:																break;
				case Mifare_SYS_Disable_Screean_Saver:																break;
				case Mifare_SYS_Get_Firm_Info:						VAP_RIF_MFC_GetFirmwareInformation();			break;
				case Mifare_SYS_Get_Datetime:						VAP_RIF_MFC_GetDateTime();						break;
				case Mifare_SYS_Set_Datetime:						VAP_RIF_MFC_SetDateTime();						break;
//				case Mifare_SYS_Get_ContactlessChipSerialNumber:	VAP_RIF_MFC_GetContactlessChipSerialNumber();	break;

				default:	break;
			}					

			break;

		case Mifare_Channel_Advance_Command:			break;

		case Mifare_Channel_Custom_Command:
			
			switch(iptCommand)
			{
				case Mifare_Custom_File_Info:	
					//20131106
#ifdef _SCREEN_SIZE_128x64
					api_sys_backlight(0,0xFFFFFFFF);
#endif									
					VAP_RIF_MFC_UpdateFileInformation();		

					Mifare_File_Download_Flag = 1;

					OS_SET_SysTimerFreeCnt(0);

					Mifare_File_Timer1 = OS_GET_SysTimerFreeCnt();

					break;
				case Mifare_Custom_Rcv_File:						

					VAP_RIF_MFC_ReceiveFile();		

					if(Mifare_Mem_Offset == Mifare_Total_File_Size)
					{
						Mifare_Burn_File();
					}
					
				break;

				case Mifare_Custom_Get_CAPK_Index:		VAP_RIF_MFC_GetCAPKIndex();		break;
				case Mifare_Custom_Del_ALL_CAPK_Key:	VAP_RIF_MFC_DeleteAllCAPKKey();	break;
				
				default:	break;	
			}	
		
		break;
		
		case	Mifare_Channel_ESC:	// 2020-05-22, demo macro command
			
			switch (iptCommand)
				{
				case	'M':	// MSR
					
				//	LIB_LCD_Putc( 0, 0, FONT0, iptCommand );
					VAP_RIF_MFC_MacroMSR();
					
					break;
					
				case	'I':	// ICC
					
				//	LIB_LCD_Putc( 1, 0, FONT0, iptCommand );
					VAP_RIF_MFC_MacroICC();
					
					break;
					
				case	'C':	// CTLS
					
				//	LIB_LCD_Putc( 2, 0, FONT0, iptCommand );
					VAP_RIF_MFC_MacroCTLS();
					
					break;
				}
				
			break;

#ifdef _PLATFORM_AS350
#ifdef _SCREEN_SIZE_320x240
		case	Mifare_Channel_MSR:
			switch (iptCommand)
			{
				case Mifare_MSR_OPEN:					VAP_RIF_MFC_OpenMSR();			break;
				case Mifare_MSR_CLOSE:					VAP_RIF_MFC_CloseMSR();			break;
				case Mifare_MSR_STATUS:					VAP_RIF_MFC_GetMSRStatus();		break;
				case Mifare_MSR_READ:					VAP_RIF_MFC_ReadMSR();			break;
				
				default:	break;
			}
			
			break;


		case	Mifare_Channel_ICC:
			switch (iptCommand)
			{	//Comment by Wayne 2020/08/21 for EMVK contact function

				// case Mifare_EMVK_INIT_KERNEL:			MFC_emvk_init_kernel();			break;
				// case Mifare_EMVK_OPEN_SESSION:			MFC_emvk_open_session();		break;
				// case Mifare_EMVK_CLOSE_SESSION:			MFC_emvk_close_session();		break;
				// case Mifare_EMVK_DETECT_ICC:			MFC_emvk_detect_icc();			break;
				// case Mifare_EMVK_ENABLE_ICC:			MFC_emvk_enable_icc();			break;
				// case Mifare_EMVK_DISABLE_ICC:			MFC_emvk_disable_icc();			break;
				// case Mifare_EMVK_CREATE_CANDIDATE_LIST:		MFC_emvk_create_candidate_list();	break;
				// case Mifare_EMVK_SELECT_APP:			MFC_emvk_select_application();		break;
				// case Mifare_EMVK_EXEC_1:			MFC_emvk_exec_1();			break;
				// case Mifare_EMVK_CVM:				MFC_emvk_cvm();				break;
				// case Mifare_EMVK_EXEC_2:			MFC_emvk_exec_2();			break;
				// case Mifare_EMVK_COMPLETION:			MFC_emvk_completion();			break;
				// case Mifare_EMVK_SET_PARA:			MFC_emvk_set_para();			break;
				// case Mifare_EMVK_GET_DATA_ELEMENT:		MFC_emvk_get_data_element();		break;
					
				// default:	break;
			}
			
			break;


		case	Mifare_Channel_KBD:
			switch (iptCommand)
			{
				case Mifare_KBD_OPEN:					VAP_RIF_MFC_OpenKBD();			break;
				case Mifare_KBD_CLOSE:					VAP_RIF_MFC_CloseKBD();			break;
				case Mifare_KBD_STATUS:					VAP_RIF_MFC_GetKBDStatus();		break;
				case Mifare_KBD_GET:					VAP_RIF_MFC_GetKBD();			break;
				case Mifare_KBD_GETC:					VAP_RIF_MFC_GetcKBD();			break;
				case Mifare_KBD_GETS:					VAP_RIF_MFC_GetsKBD();			break;
				
				default:	break;
			}
			
			break;

		case	Mifare_Channel_BCR:
#ifdef	_BCR_
			switch (iptCommand)
			{
				case Mifare_BCR_OPEN:					VAP_RIF_MFC_OpenBCR();			break;
				case Mifare_BCR_CLOSE:					VAP_RIF_MFC_CloseBCR();			break;
				case Mifare_BCR_READ:					VAP_RIF_MFC_ReadBCR();			break;
				
				default:	break;
			}
#endif
			break;
#endif
#endif

#ifdef _PLATFORM_AS350_LITE

#ifdef _ACQUIRER_NCCC
		case	Mifare_Channel_NCCC:
			switch(iptCommand)
			{
				case Mifare_NCCC_PARAMETER_INFORMATION:	VAP_RIF_MFC_NCCC_GetParameterInformation();	break;
				case Mifare_NCCC_SHOW_ERROR:			VAP_RIF_MFC_NCCC_ShowError();				break;
				case Mifare_NCCC_RESET:					VAP_RIF_MFC_NCCC_Reset();					break;
					
				default:	break;
			}
			
			break;
#endif

		case	Mifare_Channel_TSC:
			switch(iptCommand)
			{
				case Mifare_TSC_OPEN:		MFC_tsc_open();		break;
				case Mifare_TSC_CLOSE:		MFC_tsc_close();	break;
				case Mifare_TSC_STATUS:		MFC_tsc_status();	break;
				case Mifare_TSC_SIGNPAD:
				{
					api_kbd_close(0);
					MFC_tsc_signpad();
					UT_OpenKeyAll();
					break;
				}
				case Mifare_TSC_GETSIGN:	MFC_tsc_getsign();	break;

				default:	break;
			}
			
			break;
			
		case	Mifare_Channel_PED:
			
			switch(iptCommand)
			{
				case Mifare_PED_ENABLE:		MFC_ped_enable();	break;
				case Mifare_PED_DISABLE:	MFC_ped_disable();	break;

				default:	break;
			}
			
			break;
			
		case	Mifare_Channel_LCDTFT:
			switch(iptCommand)
			{
				case Mifare_LCDTFT_CLEAR:			MFC_lcdtft_clear();				break;
				case Mifare_LCDTFT_PUTS:			MFC_lcdtft_putstring();			break;
				case Mifare_LCDTFT_PUTG_PARA:		MFC_lcdtft_putgraphics_PARA();	break;
				case Mifare_LCDTFT_PUTG_DATA:		MFC_lcdtft_putgraphics_DATA();	break;
				case Mifare_LCDTFT_PUTWB_PARA:		MFC_lcdtft_putwinbmp_PARA();	break;
				case Mifare_LCDTFT_PUTWB_DATA:		MFC_lcdtft_putwinbmp_DATA();	break;
				case Mifare_LCDTFT_SHOWICON:		MFC_lcdtft_showICON();			break;
				case Mifare_LCDTFT_FILLRECT:		MFC_lcdtft_fillRECT();			break;
				case Mifare_LCDTFT_SIGNPAD_PUTS:	MFC_SIGNPAD_lcdtft_putstring();	break;
				case Mifare_LCDTFT_STATUSICON:		MFC_lcdtft_statusICON();		break;
					
				default:	break;
			}
			
			break;
#endif
			
		default:	break;
	}	
}

#if 0
void VAP_RIF_Switch_MifareCommand_DownloadFile(void)
{
	UCHAR rspCode;
	
	while(1)
	{
		rspCode=api_aux_rxready(vap_rif_dhnAUX, (UCHAR*)&vap_rif_rcvLen);
		if (rspCode == apiOK)
		{
			if ((vap_rif_rcvLen != 0) && (vap_rif_rcvLen <= VAP_RIF_BUFFER_SIZE))
			{
				api_aux_rxstring(vap_rif_dhnAUX, vap_rif_rcvBuffer);

				if ((vap_rif_rcvBuffer[2] == 0x47) && 
					(vap_rif_rcvBuffer[3] == 0x53))
				{
					if(	(vap_rif_rcvBuffer[4] == Mifare_Channel_Custom_Command) && 
						(vap_rif_rcvBuffer[5] == Mifare_Custom_Rcv_File))
					{
						VAP_RIF_MFC_ReceiveFile();

						if(vap_rif_sndBuffer[3] == Mifare_RC_Success)
						{
							Mifare_File_Timer1 = OS_GET_SysTimerFreeCnt();

							//Reset Buffer
							vap_rif_rcvLen=0;
							memset(vap_rif_rcvBuffer, 0, VAP_RIF_BUFFER_SIZE);
							memset(vap_rif_sndBuffer, 0, VAP_RIF_BUFFER_SIZE);
							
							if(Mifare_Mem_Offset == Mifare_Total_File_Size)
							{
								if(Mifare_Total_File_Size != 0) 
								{
									rspCode = Mifare_Burn_File();

									if(rspCode == FAIL)
										break;
								}
							}
						}
						else
						{
							break;
						}			
					}
					else
					{
						VAP_RIF_MFC_RestoreFunction();	break;
					}
				}
				else		//Level 3 Command
				{
					VAP_RIF_LV3_RestoreFunction();	break;
				}
			}			
		}

		Mifare_File_Timer2 = OS_GET_SysTimerFreeCnt() - Mifare_File_Timer1;
		
		if(Mifare_File_Timer2 > 500)
			break;			
	}

	//Reset Baud Rate to 38400, and Download flags.
	VAP_RIF_Reader_RestoreFunction();
	
	Mifare_File_Download_Flag = 0;
	Mifare_Seq_Num = 1;
	Mifare_Mem_Offset = 0;		

	//20131106
#ifdef _SCREEN_SIZE_128x64		
	api_sys_backlight(0,0x00000000);
#endif
	UT_ClearRow(1,3,FONT1);
}


/*	[Request]
	|0		|2		|4		|6		|8
	|AUX Len|APID	|CMD	|CMD Len|CMD Data
	
	[Response]
	|0		|2		|4		|6
	|AUX Len|Status	|Rsp Len|Rsp Data
*/
void VAP_RIF_Switch_IPASSMPCommand(UINT iptCommand)
{
#ifdef _PLATFORM_AS350_LITE
#ifdef _PAYMENT_IPASS_VERSION_1
	UINT	rspCode=0xFFFF;
	ULONG	iptLen=0;
	ULONG	optLen=0;
	ULONG	iptPolTime=0;
	UCHAR	*ptrData;

	iptLen=(vap_rif_rcvBuffer[6]<<8) | vap_rif_rcvBuffer[7];
	ptrData=&vap_rif_rcvBuffer[8];


	if ((iptCommand == 0x0101) ||
		(iptCommand == 0x0110) ||
		(iptCommand == 0x0120) ||
		(iptCommand == 0x0130))
	{
		iptLen-=2;
		iptPolTime=(ULONG)UT_C2S(&vap_rif_rcvBuffer[8]);
		ptrData=&vap_rif_rcvBuffer[10];
	}

	switch (iptCommand)
	{
#ifdef _IPASS_VERSION_1_ONLINE
		case 0x1111: rspCode=IPassMP_initLib(IPassMP_DebugPrintFunc, (ULONG)vap_rif_rcvBuffer[8], &vap_rif_rcvBuffer[9]);		break;
#else
		case 0x1111: rspCode=IPassMP_initLib(IPassMP_DebugPrintFunc, (ULONG)vap_rif_rcvBuffer[8]);								break;
#endif
		case 0x0001: rspCode=IPassMP_IsRWCheck(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);					break;
		case 0x0002: rspCode=IPassMP_RequestCheckRW(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);				break;
		case 0x0003: rspCode=IPassMP_ExcuteCheckRW(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);				break;
		case 0x0101: rspCode=IPassMP_RequestQueryTicket(iptPolTime, ptrData, iptLen, &vap_rif_sndBuffer[6], &optLen);			break;
		case 0x0102: rspCode=IPassMP_ExecuteQueryTicket(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);			break;
		case 0x0130: rspCode=IPassMP_RequestDeductValue(iptPolTime, ptrData, iptLen, &vap_rif_sndBuffer[6], &optLen);			break;
		case 0x0131: rspCode=IPassMP_ExecuteDeductValue(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);			break;
		case 0x0110: rspCode=IPassMP_RequestAddValue(iptPolTime, ptrData, iptLen, &vap_rif_sndBuffer[6], &optLen);				break;
		case 0x0111: rspCode=IPassMP_ExecuteAddValue(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);			break;
		case 0x0120: rspCode=IPassMP_RequestCancelAddValue(iptPolTime, ptrData, iptLen, &vap_rif_sndBuffer[6], &optLen);		break;
		case 0x0121: rspCode=IPassMP_ExecuteCancelAddValue(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);		break;
		case 0x010A: rspCode=IPassMP_ExecuteBlockTicket(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);			break;
		case 0x0000: rspCode=IPassMP_GetSystemInfo(&vap_rif_sndBuffer[6], &optLen);												break;
		case 0x0203: rspCode=IPassMP_GetTxLog(UT_C2S(&vap_rif_rcvBuffer[8]), &vap_rif_sndBuffer[6], &optLen);					break;
		case 0x0204: rspCode=IPassMP_GetTxLog(0, &vap_rif_sndBuffer[6], &optLen);												break;
		case 0x0205: rspCode=IPassMP_GetTxnDayCloseRecord(&vap_rif_sndBuffer[6], &optLen);										break;
		case 0xFFFF: rspCode=IPassMP_Debug(vap_rif_rcvBuffer[8]);																break;
		default:																												break;
	}
	
	UT_S2C((UINT)(2+2+optLen), &vap_rif_sndBuffer[0]);
	vap_rif_sndBuffer[2]=(rspCode & 0xFF00) >> 8;
	vap_rif_sndBuffer[3]=(rspCode & 0x00FF);
	vap_rif_sndBuffer[4]=(optLen & 0xFF00) >> 8;
	vap_rif_sndBuffer[5]=(optLen & 0x00FF);

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
#endif

	iptCommand=iptCommand;	//Remove Warning
}


void VAP_RIF_Switch_IPASSLV3Command(UINT iptCommand)
{
#ifdef _PLATFORM_AS350_LITE
#ifdef _ACQUIRER_NCCC
#ifdef _PAYMENT_IPASS_VERSION_1

	UINT	rspCode=0xFFFF;
	UINT	iptLen=0;
	UINT	optLen=0;
	
	iptLen=UT_C2S(&vap_rif_rcvBuffer[6]);

	//Input Length is Big Endian
	//Should not Use UT_C2S Here
	//Leave it Unchanged if Application is OK

	switch (iptCommand)
	{
		case 0x0000: rspCode=IPASS_INITIAL_LIBRARY(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);			break;
		case 0x0001: rspCode=IPASS_GET_SYSTEM_INFO_CMD(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);		break;
		case 0x0002: rspCode=IPASS_RW_Register(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);				break;
		case 0x0003: rspCode=IPASS_SignOn(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);						break;
		case 0x0004: rspCode=IPASS_Manual_SignOn(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);				break;
		case 0x0005: rspCode=IPASS_SaleTrans(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);					break;
		case 0x0006: rspCode=IPASS_DepositTrans(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);				break;
		case 0x0007: rspCode=IPASS_VoidDepositTrans(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);			break;
		case 0x0008: rspCode=IPASS_RefundTrans(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);				break;
		case 0x0009: rspCode=IPASS_InquireTrans(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);				break;
		case 0x000A: rspCode=IPASS_Get_Transaction_Record(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);		break;
		case 0x000B: rspCode=IPASS_SettleTrans(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);					break;
		case 0x000C: rspCode=IPASS_Get_TMS_Parameter(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);			break;
		case 0x000D: rspCode=IPASS_Reprint(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);						break;
		case 0x000E: rspCode=IPASS_Print_Total_Report(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);			break;
		case 0x000F: rspCode=IPASS_Print_Details_Report(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);			break;
		case 0x0010: rspCode=IPASS_UNLOCK_TICKET_CMD(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);		break;
		case 0x0011: rspCode=IPASS_AUTOLOAD_ON_TICKET_CMD(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);	break;
		case 0x0012: rspCode=IPASS_Clear_flash_CMD(&vap_rif_rcvBuffer[8], iptLen, &vap_rif_sndBuffer[6], &optLen);			break;
		default:																											break;
	}

	if (rspCode == 0)
	{
		IPASS_Error_Massage(&vap_rif_sndBuffer[6], &optLen);
	}
	
	UT_S2C((optLen+2+2), &vap_rif_sndBuffer[0]);
	vap_rif_sndBuffer[2]=(UCHAR)((rspCode & 0xFF00)>>8);
	vap_rif_sndBuffer[3]=(UCHAR)(rspCode & 0x00FF);
	vap_rif_sndBuffer[4]=(UCHAR)((optLen & 0xFF00)>>8);
	vap_rif_sndBuffer[5]=(UCHAR)(optLen & 0x00FF);

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);

#endif
#endif
#endif

	iptCommand=iptCommand;	//Remove Warning
}


/*	[Request]
	|0		|2		|4		|6		|8
	|AUX Len|APID	|CMD	|CMD Len|CMD Data
	
	[Response]
	|0		|2		|4		|6
	|AUX Len|Status	|Rsp Len|Rsp Data
*/
void VAP_RIF_Switch_IPASS2Command(UINT iptCommand)
{
#ifdef _PLATFORM_AS350_LITE
#ifdef _PAYMENT_IPASS_VERSION_2
	UINT	rspCode=0xFFFF;
	UINT	iptLen=0;
	UINT	optLen=0;

	iptLen=(vap_rif_rcvBuffer[6]<<8) + vap_rif_rcvBuffer[7];

	switch (iptCommand)
	{
		case 0x0000: rspCode=IPASS2_lib_init(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);				break;
		case 0x0100: rspCode=IPASS2_sign_on_Req(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);				break;
		case 0x0101: rspCode=IPASS2_sign_on_Auth(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);			break;
		case 0x0200: rspCode=IPASS2_query_ticket_Req(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);		break;
		case 0x0201: rspCode=IPASS2_query_ticket_Auth(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);		break;
		case 0x0300: rspCode=IPASS2_purchase_req(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);			break;
		case 0x0301: rspCode=IPASS2_purchase_auth(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);			break;
		case 0x0400: rspCode=IPASS2_add_value_req(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);			break;
		case 0x0401: rspCode=IPASS2_add_value_auth(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);			break;
		case 0x0410: rspCode=IPASS2_cancel_add_value_Req(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);	break;
		case 0x0411: rspCode=IPASS2_cancel_add_value_Auth(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);	break;
		case 0x0500: rspCode=IPASS2_reversal_check(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);			break;
		case 0x0510: rspCode=IPASS2_reversal_Req(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);			break;
		case 0x0511: rspCode=IPASS2_reversal_Auth(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);			break;
		case 0x0600: rspCode=IPASS2_advice_Req(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);				break;
		case 0x0601: rspCode=IPASS2_advice_Auth(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);				break;
		case 0x0900: rspCode=IPASS2_close_batch_Req(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);			break;
		case 0x0901: rspCode=IPASS2_close_batch_Auth(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);		break;
		case 0x0A00: rspCode=IPASS2_record_info_fields_init(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);	break;
		case 0x0A01: rspCode=IPASS2_record_info_set_field(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);	break;
		case 0x0A02: rspCode=IPASS2_record_info_is_field_set(iptLen, &vap_rif_rcvBuffer[8], &optLen, &vap_rif_sndBuffer[6]);break;
		
		default:																											break;
	}
	
	UT_S2C((UINT)(2+2+optLen), &vap_rif_sndBuffer[0]);
	UT_S2C(rspCode, &vap_rif_sndBuffer[2]);
	vap_rif_sndBuffer[4]=(UCHAR)((optLen & 0xFF00)>>8);
	vap_rif_sndBuffer[5]=(UCHAR)(optLen & 0x00FF);

	UT_Tx_AUX(vap_rif_dhnAUX, vap_rif_sndBuffer);
#endif
#endif

	iptCommand=iptCommand;	//Remove Warning
}


void VAP_RIF_Switch_VisaL3Command(UCHAR iptCommand)
{
	vap_rif_flgMifare=FALSE;

	if(	(vap_rif_rcvBuffer[2] != VAP_RIF_INSTRUCTION_AUT_InitializeCommunication)&&
		(vap_rif_rcvBuffer[2] != VAP_RIF_INSTRUCTION_AUT_MutualAuthenticate))
	{
#ifdef _SCREEN_SIZE_128x64					
		api_sys_backlight(0,0x000001F4);
#endif
		
		//20140106 when receive any command, extent the timout
		Main_VAP_POLL_A_Flag = TRUE;
	}
	
	switch (iptCommand)
	{
		//POLL Messages
		case VAP_RIF_INSTRUCTION_POL_POLL:							VAP_RIF_POL_POLL();								break;
		case VAP_RIF_INSTRUCTION_POL_Echo:							VAP_RIF_POL_Echo();								break;

		//Debug and Optimization Messages
		case VAP_RIF_INSTRUCTION_DEB_SetDebugAndOptimizationMode:	VAP_RIF_DEB_SetDebugAndOptimizationMode();		break;
		case VAP_RIF_INSTRUCTION_DEB_SetParameters:					VAP_RIF_DEB_SetParameters();					break;
		
		//Security Messages
		case VAP_RIF_INSTRUCTION_AUT_InitializeCommunication:		VAP_RIF_AUT_InitializeCommunication();			break;
		case VAP_RIF_INSTRUCTION_AUT_MutualAuthenticate:			VAP_RIF_AUT_MutualAuthenticate();				break;
		case VAP_RIF_INSTRUCTION_AUT_GenerateKey:					VAP_RIF_AUT_GenerateKey();						break;
		case VAP_RIF_INSTRUCTION_AUT_InvalidateReader:				VAP_RIF_AUT_InvalidateReader();					break;

		//Transaction Message
		case VAP_RIF_INSTRUCTION_TRA_ReadyForSalesTransaction:		VAP_RIF_TRA_ReadyForSalesTransaction();			break;
		case VAP_RIF_INSTRUCTION_TRA_Reset:							VAP_RIF_TRA_Reset();							break;
		case VAP_RIF_INSTRUCTION_TRA_ShowStatus:					VAP_RIF_TRA_ShowStatus();						break;
		case VAP_RIF_INSTRUCTION_TRA_Issuer_Update:					VAP_RIF_TRA_Issuer_Update();					break;

		//Administrative Messages
		case VAP_RIF_INSTRUCTION_ADM_SwitchToAdministrativeMode:	VAP_RIF_ADM_SwitchToAdministrativeMode();		break;
		case VAP_RIF_INSTRUCTION_ADM_GetCapability:					VAP_RIF_ADM_GetCapability();					break;
		case VAP_RIF_INSTRUCTION_ADM_SetCapability:					VAP_RIF_ADM_SetCapability();					break;
		case VAP_RIF_INSTRUCTION_ADM_GetDateAndTime:				VAP_RIF_ADM_GetDateAndTime();					break;
		case VAP_RIF_INSTRUCTION_ADM_SetDateAndTime:				VAP_RIF_ADM_SetDateAndTime();					break;
		case VAP_RIF_INSTRUCTION_ADM_GetParameters:					VAP_RIF_ADM_GetParameters();					break;
		case VAP_RIF_INSTRUCTION_ADM_GetVisaPublicKey:				VAP_RIF_ADM_GetVisaPublicKey(); 				break;
		case VAP_RIF_INSTRUCTION_ADM_SetVisaPublicKey:				VAP_RIF_ADM_SetVisaPublicKey();					break;
		case VAP_RIF_INSTRUCTION_ADM_GetBaudRate:					VAP_RIF_ADM_GetBaudRate();						break;
		case VAP_RIF_INSTRUCTION_ADM_SetBaudRate:					VAP_RIF_ADM_SetBaudRate();						break;
		case VAP_RIF_INSTRUCTION_ADM_ResetAcquirerKey:				VAP_RIF_ADM_ResetAcquirerKey();					break;
		case VAP_RIF_INSTRUCTION_ADM_ReaderRecovery:				VAP_RIF_ADM_ReaderRecover();					break;
		case VAP_RIF_INSTRUCTION_ADM_GetEMVTagsValues:				VAP_RIF_ADM_GetEMVTagsValues();					break;
		case VAP_RIF_INSTRUCTION_ADM_SetEMVTagsValues:				VAP_RIF_ADM_SetEMVTagsValues();					break;
		case VAP_RIF_INSTRUCTION_ADM_GetDisplayMessages:			VAP_RIF_ADM_GetDisplayMessages();				break;
		case VAP_RIF_INSTRUCTION_ADM_SetDisplayMessages:			VAP_RIF_ADM_SetDisplayMessages();				break;
		case VAP_RIF_INSTRUCTION_ADM_GetCVMCapability:				VAP_RIF_ADM_GetCVMCapability();					break;
		case VAP_RIF_INSTRUCTION_ADM_SetCVMCapability:				VAP_RIF_ADM_SetCVMCapability();					break;
		case VAP_RIF_INSTRUCTION_ADM_GetAPID:						VAP_RIF_ADM_GetAPID();							break;
		case VAP_RIF_INSTRUCTION_ADM_SetAPID:						VAP_RIF_ADM_SetAPID();							break;
		case VAP_RIF_INSTRUCTION_ADM_GetExceptionFile:				VAP_RIF_ADM_GetExceptionFile();					break;
		case VAP_RIF_INSTRUCTION_ADM_SetExceptionFile:				VAP_RIF_ADM_SetExceptionFile();					break;
		case VAP_RIF_INSTRUCTION_ADM_GetKeyRevoList:				VAP_RIF_ADM_GetKeyRevoList();					break;
		case VAP_RIF_INSTRUCTION_ADM_SetKeyRevoList:				VAP_RIF_ADM_SetKeyRevoList();					break;
		case VAP_RIF_INSTRUCTION_ADM_GetDRLEnable:					VAP_RIF_ADM_GetDRLEnable();						break;
		case VAP_RIF_INSTRUCTION_ADM_SetDRLEnable:					VAP_RIF_ADM_SetDRLEnable();						break;
		case VAP_RIF_INSTRUCTION_ADM_GetCashRRP:					VAP_RIF_ADM_GetCashRRP();						break;
		case VAP_RIF_INSTRUCTION_ADM_SetCashRRP:					VAP_RIF_ADM_SetCashRRP();						break;
		case VAP_RIF_INSTRUCTION_ADM_GetCashBackRRP:				VAP_RIF_ADM_GetCashBackRRP();					break;
		case VAP_RIF_INSTRUCTION_ADM_SetCashBackRRP:				VAP_RIF_ADM_SetCashBackRRP();					break;
		case VAP_RIF_INSTRUCTION_ADM_GetVisaTestAIDPublicKey:		VAP_RIF_ADM_GetVisaTestAIDPublicKey();			break;
		case VAP_RIF_INSTRUCTION_ADM_SetVisaTestAIDPublicKey:		VAP_RIF_ADM_SetVisaTestAIDPublicKey();			break;
		case VAP_RIF_INSTRUCTION_ADM_SetMSDOption:					VAP_RIF_ADM_SetMSDOption();						break;
		case VAP_RIF_INSTRUCTION_ADM_GetMSDOption:					VAP_RIF_ADM_GetMSDOption();						break;

		case VAP_RIF_INSTRUCTION_ADM_GetMasterPublicKey:			VAP_RIF_ADM_GetMasterPublicKey();				break;
		case VAP_RIF_INSTRUCTION_ADM_SetMasterPublicKey:			VAP_RIF_ADM_SetMasterPublicKey();				break;
		case VAP_RIF_INSTRUCTION_ADM_GetPayPassConfigurationData:	VAP_RIF_ADM_GetPayPassConfigurationData();		break;
		case VAP_RIF_INSTRUCTION_ADM_SetPayPassConfigurationData:	VAP_RIF_ADM_SetPayPassConfigurationData();		break;
		case VAP_RIF_INSTRUCTION_ADM_GetPayPassDataExchangeData:	VAP_RIF_ADM_GetPayPassDataExchangeData();		break;
		case VAP_RIF_INSTRUCTION_ADM_SetPayPassDataExchangeData:	VAP_RIF_ADM_SetPayPassDataExchangeData();		break;

		case VAP_RIF_INSTRUCTION_ADM_GetQuickPassMultipleAIDData:	VAP_RIF_ADM_GetQuickPassMultipleAIDData();		break;
		case VAP_RIF_INSTRUCTION_ADM_SetQuickPassMultipleAIDData:	VAP_RIF_ADM_SetQuickPassMultipleAIDData();		break;
		case VAP_RIF_INSTRUCTION_ADM_GetQuickPassMultipleAIDEnable:	VAP_RIF_ADM_GetQuickPassMultipleAIDEnable();	break;
		case VAP_RIF_INSTRUCTION_ADM_SetQuickPassMultipleAIDEnable:	VAP_RIF_ADM_SetQuickPassMultipleAIDEnable();	break;
		case VAP_RIF_INSTRUCTION_ADM_GetUPIPublicKey:				VAP_RIF_ADM_GetUPIPublicKey();					break;
		case VAP_RIF_INSTRUCTION_ADM_SetUPIPublicKey:				VAP_RIF_ADM_SetUPIPublicKey();					break;

		case VAP_RIF_INSTRUCTION_ADM_GetJCBPublicKey:				VAP_RIF_ADM_GetJCBPublicKey();					break;
		case VAP_RIF_INSTRUCTION_ADM_SetJCBPublicKey:				VAP_RIF_ADM_SetJCBPublicKey();					break;

		case VAP_RIF_INSTRUCTION_ADM_GetExpressPayDefaultDRL:		VAP_RIF_ADM_GetExpressPayDefaultDRL();			break;
		case VAP_RIF_INSTRUCTION_ADM_SetExpressPayDefaultDRL:		VAP_RIF_ADM_SetExpressPayDefaultDRL();			break;
		case VAP_RIF_INSTRUCTION_ADM_GetExpressPayDRL:				VAP_RIF_ADM_GetExpressPayDRL();					break;
		case VAP_RIF_INSTRUCTION_ADM_SetExpressPayDRL:				VAP_RIF_ADM_SetExpressPayDRL();					break;
		case VAP_RIF_INSTRUCTION_ADM_GetExpressPayKeyRevoList:		VAP_RIF_ADM_GetExpressPayKeyRevoList();			break;
		case VAP_RIF_INSTRUCTION_ADM_SetExpressPayKeyRevoList:		VAP_RIF_ADM_SetExpressPayKeyRevoList();			break;
		case VAP_RIF_INSTRUCTION_ADM_GetExpressPayExceptionFile:	VAP_RIF_ADM_GetExpressPayExceptionFile();		break;
		case VAP_RIF_INSTRUCTION_ADM_SetExpressPayExceptionFile:	VAP_RIF_ADM_SetExpressPayExceptionFile();		break;
		case VAP_RIF_INSTRUCTION_ADM_GetAEPublicKey:				VAP_RIF_ADM_GetAEPublicKey();					break;
		case VAP_RIF_INSTRUCTION_ADM_SetAEPublicKey:				VAP_RIF_ADM_SetAEPublicKey();					break;

		case VAP_RIF_INSTRUCTION_ADM_GetDiscoverPublicKey:			VAP_RIF_ADM_GetDiscoverPublicKey();				break;
		case VAP_RIF_INSTRUCTION_ADM_SetDiscoverPublicKey:			VAP_RIF_ADM_SetDiscoverPublicKey();				break;

		//Proprietary Message
		case VAP_RIF_INSTRUCTION_PRO_KeyInjection:					VAP_RIF_PRO_KeyInjection();						break;
		case VAP_RIF_INSTRUCTION_PRO_SetVLPSupportIndicator:		VAP_RIF_PRO_SetVLPSupportIndicator();			break;
		case VAP_RIF_INSTRUCTION_PRO_GetVLPSupportIndicator:		VAP_RIF_PRO_GetVLPSupportIndicator();			break;

		//System Configuration
		case VAP_RIF_INSTRUCTION_SYS_SetReadyForSaleBeep:			VAP_RIF_SYS_SetReadyForSaleBeep();				break;
		case VAP_RIF_INSTRUCTION_SYS_SetRFRangeUnder4cm:			VAP_RIF_SYS_SetRFRangeUnder4cm();				break;
		case VAP_RIF_INSTRUCTION_SYS_SetRFRangeEMV:					VAP_RIF_SYS_SetRFRangeEMV();					break;
		case VAP_RIF_INSTRUCTION_SYS_SetJCBRefundResponse:			VAP_RIF_SYS_SetJCBRefundResponse();				break;
		
		default: break;
	}
}
#endif

/*	[VISAWave Mode]
	-		-			-			-			2				3			5			5+x		7+x
	|STX	|Sequence	|Sender		|Receiver	|Instruction	|Message	|Variable	|CRC	|ETX	|
	|(1)	|Number(1)	|index(2)	|Index(2)	|Identifier(1)	|Length(2)	|Data(x)	|(2)	|(1)	|
*/	
void VAP_RIF_Get_ReaderInterfaceInstruction(void)
{
	UCHAR	rspCode=0;
	UCHAR	cmdMIFARE[2]={0x47,0x53};
	UCHAR	cmdIPASS_MP[2]={0x49,0x50};
	UCHAR	cmdIPASS_LV3[2]={0x49,0x51};
	UCHAR	cmdIPASS_V2[2]={0x49,0x52};

	if (Mifare_File_Download_Flag == 0)
	{
		rspCode=api_aux_rxready(vap_rif_dhnAUX, (UCHAR*)&vap_rif_rcvLen);
		if (rspCode == apiOK)
		{
			if ((vap_rif_rcvLen != 0) && (vap_rif_rcvLen <= VAP_RIF_BUFFER_SIZE))
			{		
				api_aux_rxstring(vap_rif_dhnAUX, vap_rif_rcvBuffer);		

				if (!memcmp(cmdMIFARE, &vap_rif_rcvBuffer[2], 2))
				{
					VAP_RIF_Switch_MifareCommand(vap_rif_rcvBuffer[4], vap_rif_rcvBuffer[5]);			
				}
/*				else if (!memcmp(cmdIPASS_MP, &vap_rif_rcvBuffer[2], 2))
				{
					VAP_RIF_Switch_IPASSMPCommand((vap_rif_rcvBuffer[4]<<8)+vap_rif_rcvBuffer[5]);
				}
				else if (!memcmp(cmdIPASS_LV3, &vap_rif_rcvBuffer[2], 2))
				{
					VAP_RIF_Switch_IPASSLV3Command((vap_rif_rcvBuffer[4]<<8)+vap_rif_rcvBuffer[5]);
				}
				else if (!memcmp(cmdIPASS_V2, &vap_rif_rcvBuffer[2], 2))
				{
					VAP_RIF_Switch_IPASS2Command((vap_rif_rcvBuffer[4]<<8)+vap_rif_rcvBuffer[5]);
				}
				else
				{
					VAP_RIF_Switch_VisaL3Command(vap_rif_rcvBuffer[2]);
				}
*/
			}
		}

		Mifare_Check_BlinkLEDTimeout();
	}
/*	else
	{
		VAP_RIF_Switch_MifareCommand_DownloadFile();
	}
*/
	VAP_RIF_Reset_Buffer();	
}


UCHAR VAP_RIF_Check_StopPolling(void)
{
	UCHAR	rspCode=0xFF;
	UCHAR	cmdStpPolling_4753[7]={0x47,0x53,0x03,0x72,0x00,0x01,0x00};
	UCHAR	cmdStpPolling_5864[7]={0x58,0x64,0x00,0x01,0x00,0x01,0x00};

	if(Mifare_File_Download_Flag == 0)
	{
		rspCode=api_aux_rxready(vap_rif_dhnAUX, (UCHAR*)&vap_rif_rcvLen);
		if (rspCode == apiOK)
		{
			if ((vap_rif_rcvLen != 0) && (vap_rif_rcvLen <= VAP_RIF_BUFFER_SIZE))
			{		
				api_aux_rxstring(vap_rif_dhnAUX, vap_rif_rcvBuffer);

				if (!memcmp(&vap_rif_rcvBuffer[2], cmdStpPolling_4753, 7))
				{
					vap_rif_flgMifare=TRUE;
					return TRUE;
				}
				else
				{
					if (!memcmp(&vap_rif_rcvBuffer[2], cmdStpPolling_5864, 7))
					{
						vap_rif_flgMifare=TRUE;
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}

UCHAR VAP_RIF_Check_StopRemoval(void)
{
	UCHAR	rspCode=0xFF;

	if(Mifare_File_Download_Flag == 0)
	{
		rspCode=api_aux_rxready(vap_rif_dhnAUX, (UCHAR*)&vap_rif_rcvLen);
		if (rspCode == apiOK)
		{
			if ((vap_rif_rcvLen != 0) && (vap_rif_rcvLen <= VAP_RIF_BUFFER_SIZE))
			{		
				api_aux_rxstring(vap_rif_dhnAUX, vap_rif_rcvBuffer);

				if ((vap_rif_rcvBuffer[2] == 0x47) && (vap_rif_rcvBuffer[3] == 0x53))
				{
					vap_rif_flgMifare=TRUE;

					if (vap_rif_rcvBuffer[4] == Mifare_Channel_Cless_Card_Interface)
					{
						if (vap_rif_rcvBuffer[5] == Mifare_Cless_Card_Removal)
						{
							if ((vap_rif_rcvBuffer[6] == 0x00) && (vap_rif_rcvBuffer[7] == 0x01))	//Length should be 1
							{
								if (vap_rif_rcvBuffer[8] == 0x00)	//Stop Removal
								{
									return TRUE;
								}
							}
						}
					}
				}
			}
		}
	}

	return FALSE;
}

UCHAR VAP_RIF_Check_CloseBCR(void)
{
#if	0
	UCHAR	rspCode=0xFF;

	if(Mifare_File_Download_Flag == 0)
	{
		rspCode=api_aux_rxready(vap_rif_dhnAUX, (UCHAR*)&vap_rif_rcvLen);
		if (rspCode == apiOK)
		{
			if ((vap_rif_rcvLen != 0) && (vap_rif_rcvLen <= VAP_RIF_BUFFER_SIZE))
			{		
				api_aux_rxstring(vap_rif_dhnAUX, vap_rif_rcvBuffer);

				if ((vap_rif_rcvBuffer[2] == 0x47) && (vap_rif_rcvBuffer[3] == 0x53))
				{
					vap_rif_flgMifare=TRUE;

					if (vap_rif_rcvBuffer[4] == Mifare_Channel_BCR)
					{
						if (vap_rif_rcvBuffer[5] == Mifare_BCR_CLOSE)
						{
							if ((vap_rif_rcvBuffer[6] == 0x00) && (vap_rif_rcvBuffer[7] == 0x00))	//Length should be 0
							{
								return TRUE;
							}
						}
					}
				}
			}
		}
	}
#endif
	return FALSE;
}
#if 0
UCHAR VAP_RIF_Check_ResetCommand(void)
{
	UCHAR	rspCode=0xFF;
	
	rspCode=api_aux_rxready(vap_rif_dhnAUX, (UCHAR*)&vap_rif_rcvLen);
	if (rspCode == apiOK)
	{
		if ((vap_rif_rcvLen != 0) && (vap_rif_rcvLen <= VAP_RIF_BUFFER_SIZE))
		{
			api_aux_rxstring(vap_rif_dhnAUX, vap_rif_rcvBuffer);

			if (vap_rif_rcvBuffer[2] == VAP_RIF_INSTRUCTION_TRA_Reset)
			{
				VAP_RIF_TRA_Reset();
				return TRUE;
			}
		}
	}

	return FALSE;
}

//20140305, add, while detecting card, reader can receive show status command
void VAP_RIF_Check_ShowStatusCommand(void)
{
	UCHAR	rspCode=0xFF;
	
	rspCode=api_aux_rxready(vap_rif_dhnAUX, (UCHAR*)&vap_rif_rcvLen);
	if (rspCode == apiOK)
	{
		if ((vap_rif_rcvLen != 0) && (vap_rif_rcvLen <= VAP_RIF_BUFFER_SIZE))
		{
			api_aux_rxstring(vap_rif_dhnAUX, vap_rif_rcvBuffer);

			if (vap_rif_rcvBuffer[2] == VAP_RIF_INSTRUCTION_TRA_ShowStatus)
			{
				VAP_RIF_TRA_ShowStatus();
			}
		}
	}
}
//20140305, add end, while detecting card, reader can receive show status command 
#endif
