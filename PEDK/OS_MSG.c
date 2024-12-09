//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL  				                                    **
//**  PRODUCT  : AS350-X6						                            **
//**                                                                        **
//**  FILE     : OS_MSG.C                                                   **
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
#include "POSAPI.h"

//----------------------------------------------------------------------------
UINT8	const	os_msg_BSP[] =				{"BSP 1.0.0"};

UINT8	const	os_msg_Ready[] =			{"READY"};
UINT8	const	os_msg_FUNCTION[] =			{"FUNCTION:"};
UINT8	const	os_msg_DOWNLOAD[] =			{"DOWNLOAD"};
UINT8	const	os_msg_3DOTS[] =			{"..."};
UINT8	const	os_msg_END_OF_DOWNLOAD[] =	{"END OF DOWNLOAD"};
UINT8	const	os_msg_DOWNLOAD_FAILED[] =	{"DOWNLOAD FAILED"};
UINT8	const	os_msg_ERASE_FLASH[] =		{"ERASE FLASH:"};
UINT8	const	os_msg_RW_ADDRESS[] =		{"R/W ADDRESS:"};
UINT8	const	os_msg_R[] =				{"R"};			// read
UINT8	const	os_msg_W[] =				{"W"};			// write
UINT8	const	os_msg_E[] =				{"E"};			// erase
UINT8	const	os_msg_RW[] =				{"RW"};			// read & write
UINT8	const	os_msg_PROCESSING[] =		{"PROCESSING..."};
UINT8	const	os_msg_PROCESSING_ERR[] =	{"PROCESSING ERR"};
UINT8	const	os_msg_LOCAL_DOWNLOAD[] =	{"LOCAL DOWNLOAD"};
UINT8	const	os_msg_PLEASE_RESTART[] =	{"PLEASE RESTART..."};
UINT8	const	os_msg_AS320[] =			{"AS-320"};
UINT8	const	os_msg_SRAM[] =				{"SRAM"};
UINT8	const	os_msg_FLASH[] =			{"FLASH"};
UINT8	const	os_msg_LCM[] =				{"LCM"};
UINT8	const	os_msg_KEYBOARD[] =			{"KEYBOARD"};
UINT8	const	os_msg_KBD_DASH[] =			{"  ----"};
UINT8	const	os_msg_RTC[] =				{"RTC"};
UINT8	const	os_msg_CLEAR_MEMORY[] =		{"CLEAR MEMORY"};
UINT8	const	os_msg_OK[] =				{"OK"};
UINT8	const	os_msg_OK5[] =				{"OK   "};
UINT8	const	os_msg_ERROR[] =			{"ERROR"};
UINT8	const	os_msg_5SP[] =				{"     "};
UINT8	const	os_msg_ICC0[] =				{"ICC0:"};
UINT8	const	os_msg_SAM1[] =				{"SAM1:"};
UINT8	const	os_msg_SAM2[] =				{"SAM2:"};
UINT8	const	os_msg_SAM3[] =				{"SAM3:"};
UINT8	const	os_msg_SAM4[] =				{"SAM4:"};
UINT8	const	os_msg_MSR[] =				{"MSR"};
UINT8	const	os_msg_TRACK1[] =			{"TRACK1:"};
UINT8	const	os_msg_TRACK2[] =			{"TRACK2:"};
UINT8	const	os_msg_TRACK3[] =			{"TRACK3:"};
UINT8	const	os_msg_VIEW_QUIT[] =		{"ENTER=View  CANL=Quit"};
UINT8	const	os_msg_AUTO_SETUP_MAC[] = 	{"AUTO SETUP MAC"};
UINT8	const	os_msg_SETUP_TSN[] = 		{"SETUP TSN"};
UINT8	const	os_msg_DSS[] =				{"DSS"};
UINT8	const	os_msg_MODEM[] =			{"MODEM"};
UINT8	const	os_msg_1AUTO_2SETUP[] =		{"1=AUTO 2=SETUP"};
UINT8	const	os_msg_TEL_NO[] =			{"TEL NO:"};
UINT8	const	os_msg_PABX_CODE[] =		{"PABX CODE:"};
UINT8	const	os_msg_TONE_PULSE[] =		{"0=TONE 1=PULSE"};
UINT8	const	os_msg_SYNC_ASYNC[] =		{"0=SYNC 1=ASYNC"};
UINT8	const	os_msg_SPEED[] =			{"SPEED:"};
UINT8	const	os_msg_START[] =			{"START"};
UINT8	const	os_msg_PRINTER[] =			{"PRINTER"};
UINT8	const	os_msg_ETHERNET[] =			{"ETHERNET"};
UINT8	const	os_msg_BATTERY[] =			{"BATTERY"};
UINT8	const	os_msg_PING[] =				{"PING:"};
UINT8	const	os_msg_ERR_NO_RESPONSE[] =	{"Err: no response"};
UINT8	const	os_msg_1SETUP_2PING[] =		{"1=SETUP 2=PING"};
UINT8	const	os_msg_SETUP[] =			{"SETUP"};
UINT8	const	os_msg_IP[] =				{"IP:"};
UINT8	const	os_msg_SUBNET_MASK[]=		{"SUBNET MASK:"};
UINT8	const	os_msg_GATEWAY[] =			{"GATEWAY:"};
UINT8	const	os_msg_TEST[] =				{"SELF_TEST..."};
UINT8	const	os_msg_SRAM_RETENTION[] =	{"SRAM RETENTION"};
UINT8	const	os_msg_1WRITE_2READ[] =		{"1=WRITE 2=READ"};
UINT8	const	os_msg_AUX0[] =				{"AUX0"};
UINT8	const	os_msg_AUX1[] =				{"AUX1"};
UINT8	const	os_msg_AUX2[] =				{"AUX2"};
UINT8	const	os_msg_OPEN_TEST[] =		{"OPEN TEST"};
UINT8	const	os_msg_LOOPBACK_TEST[] =	{"LOOPBACK TEST"};
UINT8	const	os_msg_GPRS[] =				{"GPRS"};
UINT8	const	os_msg_4G_LTE[] =			{"4G-LTE"};
UINT8	const	os_msg_SWITCH_BOOT_ROM[] =	{"SWITCH BOOT ROM (0/1)"};
UINT8	const	os_msg_VERIFY_APP[] =		{"VERIFY APP..."};
UINT8	const	os_msg_HAL[] =				{"HAL:"};
UINT8	const	os_msg_SSL[] =				{"SSL"};

UINT8	const	os_msg_KMS[] =				{"KEY MANAGEMENT SYSTEM"};

UINT8	const	os_list_OP_MODE[] =
				{ 13, 'A','D','M','I','N','I','S','T','R','A','T','O','R',' ',' ',' ',' ',  // 00: ADMINISTRATOR
				  04, 'U','S','E','R',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 01: USER
//				  05, 'R','E','S','E','T',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 02: RESET
				};
				
UINT8	const	os_list_ADMIN_MODE[] =
				{
				  17, 'V','E','R','I','F','Y',' ','K','E','Y',' ','S','T','A','T','U','S',' ',' ',' ',' ',' ',' ',' ',  // 00: VERIFY KEY STATUS
				  17, 'C','H','A','N','G','E',' ','K','E','Y',' ','S','C','H','E','M','E',' ',' ',' ',' ',' ',' ',' ',	// 01: CHANGE KEY SCHEME (MODE)
				  22, 'S','E','T','U','P',' ','1','2','8','-','b','i','t',' ','T','D','E','S',' ','K','P','K',' ',' ',	// 02: SETUP 128-bit TDES KPK
				  22, 'S','E','T','U','P',' ','1','9','2','-','b','i','t',' ','T','D','E','S',' ','K','P','K',' ',' ',	// 03: SETUP 192-bit TDES KPK
                  21, 'S','E','T','U','P',' ','1','2','8','-','b','i','t',' ','A','E','S',' ','K','P','K',' ',' ',' ',	// 04: SETUP 128-bit AES KPK
				  21, 'S','E','T','U','P',' ','1','9','2','-','b','i','t',' ','A','E','S',' ','K','P','K',' ',' ',' ',	// 05: SETUP 192-bit AES KPK
                  21, 'S','E','T','U','P',' ','2','5','6','-','b','i','t',' ','A','E','S',' ','K','P','K',' ',' ',' ',	// 06: SETUP 256-bit AES KPK
				  20, 'L','O','A','D',' ','M','/','S',' ','K','E','Y',' ','F','O','R',' ','P','E','K',' ',' ',' ',' ',	// 07: LOAD M/S KEY FOR PEK
				  24, 'L','O','A','D',' ','M','/','S',' ','K','E','Y',' ','F','O','R',' ','A','C','C','_','D','E','K',	// 08: LOAD M/S KEY FOR ACC_DEK
                  24, 'L','O','A','D',' ','M','/','S',' ','K','E','Y',' ','F','O','R',' ','F','P','E',' ','K','E','Y',	// 09: LOAD M/S KEY FOR FPE KEY
                  18, 'L','O','A','D',' ','A','E','S',' ','D','U','K','P','T',' ','K','E','Y',' ',' ',' ',' ',' ',' ',	// 0A: LOAD AES DUKPT KEY
				   9, 'L','O','A','D',' ','C','A','P','K',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 0B: LOAD CAPK
				  14, 'D','E','L','E','T','E',' ','P','E','D',' ','K','E','Y',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 0C: DELETE PED KEY
				  10, 'S','E','T','U','P',' ','S','R','E','D',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 0D: SETUP SRED
				  10, 'R','E','S','E','T',' ','S','E','C','M',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 0E: RESET SECM
                //   24, 'D','E','L','E','T','E',' ','U','-','B','O','O','T',' ','A','N','D',' ','K','E','R','N','E','L',  // 0F: DELETE U-BOOT AND KERNEL

                //   17, 'V','E','R','I','F','Y',' ','K','E','Y',' ','S','T','A','T','U','S',' ',  // 00: VERIFY KEY STATUS
				//   17, 'C','H','A','N','G','E',' ','K','E','Y',' ','S','C','H','E','M','E',' ',	// 01: CHANGE KEY SCHEME (MODE)
				//   18, 'S','E','T','U','P',' ','K','P','K',' ','F','O','R',' ','T','D','E','S',	// 02: SETUP KPK FOR TDES
				//   17, 'S','E','T','U','P',' ','K','P','K',' ','F','O','R',' ','A','E','S',' ',	// 03: SETUP KPK FOR AES
				//   12, 'L','O','A','D',' ','M','/','S',' ','K','E','Y',' ',' ',' ',' ',' ',' ',	// 04: LOAD M/S KEY
				//   15, 'L','O','A','D',' ','D','U','K','P','T',' ','K','E','Y',' ',' ',' ',' ',	// 05: LOAD DUKPT KEY
				//   13, 'L','O','A','D',' ','I','S','O','4','_','K','E','Y',' ',' ',' ',' ',' ',	// 06: LOAD ISO4_KEY
				//   12, 'L','O','A','D',' ','A','C','C','_','D','E','K',' ',' ',' ',' ',' ',' ',	// 07: LOAD ACC_DEK
				//   12, 'L','O','A','D',' ','F','P','E',' ','K','E','Y',' ',' ',' ',' ',' ',' ',	// 08: LOAD FPE KEY
				//    9, 'L','O','A','D',' ','C','A','P','K',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 09: LOAD CAPK
				//   14, 'D','E','L','E','T','E',' ','P','E','D',' ','K','E','Y',' ',' ',' ',' ',  // 0A: DELETE PED KEY
				//   10, 'S','E','T','U','P',' ','S','R','E','D',' ',' ',' ',' ',' ',' ',' ',' ',  // 0B: SETUP SRED
				//   10, 'R','E','S','E','T',' ','S','E','C','M',' ',' ',' ',' ',' ',' ',' ',' ',  // 0C: RESET SECM
				};

UINT8	const	os_list_USER_MODE[] =
				{
				  15, 'C','H','A','N','G','E',' ','P','A','S','S','W','O','R','D',' ',' ',  // 00: CHANGE PASSWORD
				  17, 'V','E','R','I','F','Y',' ','K','E','Y',' ','S','T','A','T','U','S',  // 01: VERIFY KEY STATUS
				  17, 'C','H','A','N','G','E',' ','K','E','Y',' ','S','C','H','E','M','E',  // 02: CHANGE KEY SCHEME (MODE)
//				  12, 'S','H','O','W',' ','T','E','R','M',' ','S','N',' ',' ',' ',' ',' ',  // 03: SHOW TERM SN
				};

UINT8	const	os_list_KEY_MODE[] = 
				{ 
				  04, '(','N','A',')',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 00: (NA)
                  14, 'M','A','S','T','E','R','/','S','E','S','S','I','O','N',' ',' ',' ',  // 01: MASTER/SESSION
                  05, 'D','U','K','P','T',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 02: DUKPT
                   8, 'I','S','O','4','_','K','E','Y',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 03: ISO4_KEY
		        };

UINT8	const	os_list_KEYS[] = 
				{ 
                  15, 'M','/','S',' ','K','E','Y',' ','F','O','R',' ','P','E','K',' ',' ',' ',' ',  // 00: M/S KEY FOR PEK
                  19, 'M','/','S',' ','K','E','Y',' ','F','O','R',' ','A','C','C','_','D','E','K',  // 01: M/S KEY FOR ACC_DEK
                  19, 'M','/','S',' ','K','E','Y',' ','F','O','R',' ','F','P','E',' ','K','E','Y',  // 02: M/S KEY FOR FPE KEY
                   9, 'A','E','S',' ','D','U','K','P','T',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 03: AES DUKPT
				  04, 'C','A','P','K',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 04: CAPK

                //   14, 'M','A','S','T','E','R','/','S','E','S','S','I','O','N',' ',' ',' ',  // 00: MASTER/SESSION
                //   05, 'D','U','K','P','T',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 01: DUKPT
				//    8, 'I','S','O','4','_','K','E','Y',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 02: ISO4_KEY
                //   07, 'A','C','C','_','D','E','K',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 03: ACC_DEK
                //   07, 'F','P','E',' ','K','E','Y',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 04: FPE KEY
				//   04, 'C','A','P','K',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',  // 05: CAPK
		                };

UINT8	const	os_list_BPS[] =
				{
				  04, '1','2','0','0',' ',' ',
				  04, '2','4','0','0',' ',' ',
				  04, '9','6','0','0',' ',' ',
				  05, '1','4','4','0','0',' ',
				};

UINT8	const	os_list_BOOT_ROM[] =
				{
				  01, '0',' ',
				  01, '1',' ',
				};
				
	                
UINT8	const	os_msg_NEW_PASSWORD[] =		{"NEW PASSWORD:"};
UINT8	const	os_msg_CONFIRM_PASSWORD[] =	{"CONFIRM PASSWORD:"};

UINT8	const	os_msg_ADMIN[] =			{"ADMINISTRATOR"};
UINT8	const	os_msg_ADMIN2[] =			{"[ADMIN]"};
UINT8	const	os_msg_USER[] =				{"USER"};
UINT8	const	os_msg_USER2[] =			{"[USER]"};
UINT8	const	os_msg_TERM_ID[] =			{"TERM ID:"};	 
UINT8	const	os_msg_Q_UPDATE[] =			{"UPDATE (Y/N)?"};
UINT8	const	os_msg_Q_DELETE[] =			{"DELETE (Y/N)?"};
UINT8	const	os_msg_Q_RESET[] =			{"RESET (Y/N)?"};
UINT8	const	os_msg_Q_RUN_AP[] =			{"RUN AP (Y/N)?"};
UINT8	const	os_msg_LOAD_KEY_OK[] =		{"LOAD KEY OK"};
UINT8	const	os_msg_LOAD_KEY_FAILED[] =	{"LOAD KEY FAILED"};
UINT8	const	os_msg_UPDATE_OK[] =		{"UPDATE OK"};
UINT8	const	os_msg_SELECT[] =			{"SELECT:"};
UINT8	const	os_msg_DUKPT[] =			{"DUKPT"};
UINT8	const	os_msg_AES_DUKPT[] =		{"AES DUKPT"};
UINT8	const	os_msg_MSKEY[] =			{"MASTER/SESSION"};
UINT8	const	os_msg_PEK[] =			    {"PEK"};
UINT8	const	os_msg_FKEY[] =				{"FIXED"};
UINT8	const	os_msg_CAPK[] =				{"CAPK"};
UINT8	const	os_msg_ISO4KEY[] =			{"ISO4_KEY"};
UINT8	const	os_msg_ACCDEK[] =			{"ACC_DEK"};
UINT8	const	os_msg_FPEKEY[] =			{"FPE KEY"};
UINT8	const	os_msg_SET_TDES_KPK[] =		{"SET TDES KPK"};
UINT8	const	os_msg_SET_AES_KPK[] =		{"SET AES KPK"};
UINT8	const	os_msg_LOAD_PED_KEY[] =		{"LOAD PED KEY"};
UINT8	const	os_msg_DEL_PED_KEY[] =		{"DELETE PED KEY"};
UINT8	const	os_msg_RESET_SECM[] =		{"RESET SECURE MEMORY"};
UINT8	const	os_msg_LOAD_IKEK[] =		{"LOAD IKEK"};
UINT8	const	os_msg_LOAD_MSKEY[] =		{"LOAD MASTER/SESSION KEY"};
UINT8	const	os_msg_LOAD_DUKPT[] =		{"LOAD DUKPT"};
UINT8	const	os_msg_LOAD_ISO4KEY[] =		{"LOAD ISO4_KEY"};
UINT8	const	os_msg_LOAD_ISO4KEK[] =		{"LOAD ISO4_KEK"};
UINT8	const	os_msg_LOAD_ACCDEK[] =		{"LOAD ACC_DEK"};
UINT8	const	os_msg_LOAD_FPEKEY[] =		{"LOAD FPE KEY"};
UINT8	const	os_msg_LOAD_CAPK[] =		{"LOAD CA PUBLIC KEY"};
UINT8	const	os_msg_PED_KEY_STATUS[] =	{"PED KEY STATUS"};
UINT8	const	os_msg_TERM_SN[] =			{"TERM SN"};
UINT8	const	os_msg_INIT[] =				{"INIT..."};
UINT8	const	os_msg_SETUP_SRED[] =		{"SETUP SRED"};
UINT8	const	os_msg_DISABLE[] =			{"0 = DISABLE"};
UINT8	const	os_msg_ENABLE[] =			{"1 = ENABLE"};
UINT8	const	os_msg_WRITE_OK[] =			{"WRITE OK"};
UINT8	const	os_msg_READ_OK[] =			{"READ OK"};
UINT8   const   os_msg_GEN_RSA_KEY_PAIR[] = {"GENERATE RSA KEY PAIR"};
UINT8   const   os_msg_DEV_AUTH_1[] =       {"DEVICE AUTH PHASE 1"};
UINT8   const   os_msg_DEV_AUTH_2[] =       {"DEVICE AUTH PHASE 2"};
UINT8	const	os_msg_VERIFY_OK[] =		{"VERIFY OK"};
UINT8	const	os_msg_VERIFY_FAILED[] =	{"VERIFY FAILED"};

//----------------------------------------------------------------------------

