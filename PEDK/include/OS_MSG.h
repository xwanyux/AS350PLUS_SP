//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL  				                                    **
//**  PRODUCT  : AS350-X6						                            **
//**                                                                        **
//**  FILE     : OS_MSG.H						                            **
//**  MODULE   : Declaration of POST Messages.		                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _OS_MSG_H_
#define _OS_MSG_H_

#include "POSAPI.h"

//----------------------------------------------------------------------------
extern	UINT8	const	os_msg_BSP[9];
extern	UINT8	const	os_msg_Ready[5];
extern	UINT8	const	os_msg_FUNCTION[9];
extern	UINT8	const	os_msg_DOWNLOAD[8];
extern	UINT8	const	os_msg_3DOTS[3];
extern	UINT8	const	os_msg_END_OF_DOWNLOAD[15];
extern	UINT8	const	os_msg_DOWNLOAD_FAILED[15];
extern	UINT8	const	os_msg_ERASE_FLASH[12];
extern	UINT8	const	os_msg_RW_ADDRESS[12];
extern	UINT8	const	os_msg_R[1];
extern	UINT8	const	os_msg_W[1];
extern	UINT8	const	os_msg_E[1];
extern	UINT8	const	os_msg_RW[2];
extern	UINT8	const	os_msg_PROCESSING[13];
extern	UINT8	const	os_msg_PROCESSING_ERR[14];
extern	UINT8	const	os_msg_LOCAL_DOWNLOAD[14];
extern	UINT8	const	os_msg_PLEASE_RESTART[17];
extern	UINT8	const	os_msg_AS320[6];
extern	UINT8	const	os_msg_SRAM[4];
extern	UINT8	const	os_msg_FLASH[5];
extern	UINT8	const	os_msg_LCM[3];
extern	UINT8	const	os_msg_KEYBOARD[8];
extern	UINT8	const	os_msg_KBD_DASH[6];
extern	UINT8	const	os_msg_RTC[3];
extern	UINT8	const	os_msg_CLEAR_MEMORY[12];
extern	UINT8	const	os_msg_OK[2];
extern	UINT8	const	os_msg_OK5[5];
extern	UINT8	const	os_msg_ERROR[5];
extern	UINT8	const	os_msg_5SP[5];
extern	UINT8	const	os_msg_ICC0[5];
extern	UINT8	const	os_msg_SAM1[5];
extern	UINT8	const	os_msg_SAM2[5];
extern	UINT8	const	os_msg_SAM3[5];
extern	UINT8	const	os_msg_SAM4[5];
extern	UINT8	const	os_msg_MSR[3];
extern	UINT8	const	os_msg_TRACK1[7];
extern	UINT8	const	os_msg_TRACK2[7];
extern	UINT8	const	os_msg_TRACK3[7];
extern	UINT8	const	os_msg_VIEW_QUIT[21];
extern	UINT8	const	os_msg_AUTO_SETUP_MAC[14];
extern	UINT8	const	os_msg_SETUP_TSN[9];
extern	UINT8	const	os_msg_DSS[3];
extern	UINT8	const	os_msg_MODEM[5];
extern	UINT8	const	os_msg_1AUTO_2SETUP[14];
extern	UINT8	const	os_msg_TEL_NO[7];
extern	UINT8	const	os_msg_PABX_CODE[10];
extern	UINT8	const	os_msg_TONE_PULSE[14];
extern	UINT8	const	os_msg_SYNC_ASYNC[14];
extern	UINT8	const	os_msg_SPEED[6];
extern	UINT8	const	os_msg_START[5];
extern	UINT8	const	os_msg_PRINTER[7];
extern  UINT8	const	os_msg_ETHERNET[8];
extern	UINT8	const	os_msg_BATTERY[7];
extern  UINT8	const	os_msg_PING[5];
extern	UINT8	const	os_msg_ERR_NO_RESPONSE[16];
extern	UINT8	const	os_msg_1SETUP_2PING[14];
extern	UINT8	const	os_msg_SETUP[5];
extern	UINT8	const	os_msg_IP[3];
extern	UINT8	const	os_msg_SUBNET_MASK[12];
extern	UINT8	const	os_msg_GATEWAY[8];
extern	UINT8	const	os_msg_TEST[12];
extern	UINT8	const	os_msg_SRAM_RETENTION[14];
extern	UINT8	const	os_msg_1WRITE_2READ[14];
extern	UINT8	const	os_msg_AUX0[4];
extern	UINT8	const	os_msg_AUX1[4];
extern	UINT8	const	os_msg_AUX2[4];
extern	UINT8	const	os_msg_OPEN_TEST[9];
extern	UINT8	const	os_msg_LOOPBACK_TEST[13];
extern	UINT8	const	os_msg_GPRS[4];
extern	UINT8	const	os_msg_4G_LTE[6];
extern	UINT8	const	os_msg_SWITCH_BOOT_ROM[21];
extern	UINT8	const	os_msg_VERIFY_APP[13];
extern	UINT8	const	os_msg_HAL[4];
extern	UINT8	const	os_msg_SSL[3];

extern	UINT8	const	os_msg_KMS[21];

extern	UINT8	const	os_list_OP_MODE[];
extern	UINT8	const	os_list_ADMIN_MODE[];
extern	UINT8	const	os_list_USER_MODE[];
extern	UINT8	const	os_list_KEYS[];
extern	UINT8	const	os_list_KEY_MODE[];
extern	UINT8	const	os_list_BPS[];
extern	UINT8	const	os_list_BOOT_ROM[];

extern	UINT8	const	os_msg_NEW_PASSWORD[13];
extern	UINT8	const	os_msg_CONFIRM_PASSWORD[17];

extern	UINT8	const	os_msg_ADMIN[13];
extern	UINT8	const	os_msg_ADMIN2[7];
extern	UINT8	const	os_msg_USER[4];
extern	UINT8	const	os_msg_USER2[6];
extern	UINT8	const	os_msg_TERM_ID[9];
extern	UINT8	const	os_msg_Q_UPDATE[13];
extern	UINT8	const	os_msg_Q_DELETE[13];
extern	UINT8	const	os_msg_Q_RESET[12];
extern	UINT8	const	os_msg_Q_RUN_AP[13];
extern	UINT8	const	os_msg_LOAD_KEY_OK[11];
extern	UINT8	const	os_msg_LOAD_KEY_FAILED[15];
extern	UINT8	const	os_msg_UPDATE_OK[9];
extern	UINT8	const	os_msg_SELECT[7];
extern	UINT8	const	os_msg_DUKPT[5];
extern  UINT8   const   os_msg_AES_DUKPT[9];
extern	UINT8	const	os_msg_MSKEY[14];
extern  UINT8	const	os_msg_PEK[3];
extern	UINT8	const	os_msg_FKEY[5];
extern	UINT8	const	os_msg_CAPK[4];
extern	UINT8	const	os_msg_ISO4KEY[8];
extern	UINT8	const	os_msg_ACCDEK[7];
extern	UINT8	const	os_msg_FPEKEY[7];
extern	UINT8	const	os_msg_SET_TDES_KPK[12];
extern	UINT8	const	os_msg_SET_AES_KPK[11];
extern	UINT8	const	os_msg_LOAD_PED_KEY[12];
extern	UINT8	const	os_msg_DEL_PED_KEY[14];
extern	UINT8	const	os_msg_RESET_SECM[19];
extern	UINT8	const	os_msg_LOAD_IKEK[9];
extern	UINT8	const	os_msg_LOAD_MSKEY[23];
extern	UINT8	const	os_msg_LOAD_DUKPT[10];
extern	UINT8	const	os_msg_LOAD_ISO4KEY[13];
extern	UINT8	const	os_msg_LOAD_ISO4KEK[13];
extern	UINT8	const	os_msg_LOAD_ACCDEK[12];
extern	UINT8	const	os_msg_LOAD_FPEKEY[12];
extern	UINT8	const	os_msg_LOAD_CAPK[18];
extern	UINT8	const	os_msg_PED_KEY_STATUS[14];
extern	UINT8	const	os_msg_TERM_SN[7];
extern	UINT8	const	os_msg_INIT[7];
extern	UINT8	const	os_msg_SETUP_SRED[10];
extern	UINT8	const	os_msg_DISABLE[11];
extern	UINT8	const	os_msg_ENABLE[10];
extern	UINT8	const	os_msg_WRITE_OK[8];
extern	UINT8	const	os_msg_READ_OK[7];
extern  UINT8   const   os_msg_GEN_RSA_KEY_PAIR[21];
extern  UINT8   const   os_msg_DEV_AUTH_1[19];
extern  UINT8   const   os_msg_DEV_AUTH_2[19];
extern  UINT8	const	os_msg_VERIFY_OK[9];
extern  UINT8	const	os_msg_VERIFY_FAILED[13];

//----------------------------------------------------------------------------
#endif
