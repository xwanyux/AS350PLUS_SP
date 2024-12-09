//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : EMVAPI.H                                                   **
//**  MODULE   : Declaration of related EMV API functions.                  **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/02                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2002-2018 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _EMVAPI_H_
#define _EMVAPI_H_

//----------------------------------------------------------------------------
//      PBOC2.0
//----------------------------------------------------------------------------
//      EMV L2 v4.1e

//extern  void SETUP_config_00( void );
//extern  void SETUP_config_01( void );
//extern  void SETUP_config_02( void );
//extern  void SETUP_config_03( void );
//extern  void SETUP_config_04( void );
//extern  void SETUP_config_05( void );
//extern  void SETUP_config_06( void );
//extern  void SETUP_config_07( void );
//extern  void SETUP_config_08( void );
//extern  void SETUP_config_09( void );
//extern  void SETUP_config_10( void );
//extern  void SETUP_config_11( void );
extern  UCHAR Scan_TDE( UCHAR tag1, UCHAR tag2, UCHAR *de );
extern  UCHAR Scan_IDE( UCHAR tag1, UCHAR tag2, UCHAR *de );

//      PBOC20_2
//extern  void  PBOC_SetAID( void );
//extern  void  PBOC_SetupCardParameters( void );
//extern  UCHAR PBOC_SelectAID( UCHAR cursor );
//extern  UCHAR PBOC_ReadIccLog( void );
//extern  void  PBOC_SetDDOL( void );
//extern  void  PBOC_SetTDOL( void );
//extern  void  PBOC_SetTransType( void );
//extern  UCHAR PBOC_VerifyCardholderLicense( void );
//extern  UCHAR PBOC_CheckTcHash( UCHAR type );

//extern  void  api_emvk_GetPublicKeyInfo( UCHAR *info );

//      DEBUG TEST ONLY
//extern  void  TC_SelectADF_01( void );
//extern  UCHAR apk_IssuerScriptProcessing3( UCHAR tag, UINT addr_ist );
//extern  void TC_IssuerScript( void );
//extern  void TC_TcHash( void );
//extern  void PBOC_TEST();

//----------------------------------------------------------------------------
//      IFM API
//----------------------------------------------------------------------------
//extern  UCHAR api_ifm_open( UCHAR acceptor );
//extern  UCHAR api_ifm_close( UCHAR dhn );
//extern  UCHAR api_ifm_deactivate( UCHAR dhn );
//extern  UCHAR api_ifm_present( UCHAR dhn );
//extern  UCHAR api_ifm_reset( UCHAR dhn, UCHAR mode, UCHAR *dbuf );
//extern  UCHAR api_ifm_exchangeAPDU( UCHAR dhn, UCHAR *c_apdu, UCHAR *r_apdu );

//----------------------------------------------------------------------------
//      DES API
//----------------------------------------------------------------------------
//extern  void  api_des_encipher( UCHAR *plain_text, UCHAR *cipher_text, UCHAR *key );
//extern  void  api_des_decipher( UCHAR *plain_text, UCHAR *cipher_text, UCHAR *key );

//----------------------------------------------------------------------------
//      DUKPT-3D
//----------------------------------------------------------------------------
//extern  UCHAR PED_LoadInitialKey( UCHAR id, UINT CurKeyPtr, UCHAR *Shift_Reg, UCHAR *InitPinEncKey, UCHAR *KeySerialNum );
//extern  UCHAR PED_RequestPinEntry( UCHAR *pindata, UCHAR *pan, UCHAR *ksn, UCHAR *epb );
//extern  UCHAR PED_GetPAN12( UCHAR *pan );

//----------------------------------------------------------------------------
//      EMV L2 API
//----------------------------------------------------------------------------
//      L2API_01
extern  UCHAR api_emv_OpenSession(UCHAR slot);
extern  UCHAR api_emv_CardPresent( UCHAR dhn );
extern  void  api_emv_CloseSession( UCHAR dhn );
extern  void  api_emv_Deactivate( UCHAR dhn );
extern  UCHAR api_emv_ATR( UCHAR dhn, UCHAR *atr );
//extern  UCHAR api_emv_SelectSAM( void );
//extern  UCHAR api_emv_CleanSAM( void );
extern  UCHAR api_emv_RetrievePublicKeyCA( void );
extern  void  api_emv_GetPublicKeyInfo( UCHAR *info );

//      L2API_02
extern  UCHAR api_emv_CreateCandidateList( void );
extern  UCHAR api_emv_CreateCandidateList_PBOC( void );
extern  UCHAR api_emv_GetCandidateList( UCHAR *list );

//      L2API_03
extern  UCHAR api_emv_FinalSelection( UCHAR *appname );
extern  void  api_emv_RemoveApplication( UCHAR index );
extern  UCHAR api_emv_AutoSelectApplication( UCHAR occurrence, UCHAR listcnt, UCHAR *item );
extern	void  api_emv_SetupCardParameters( void );

//      L2API_04
extern  UCHAR api_emv_PutDataElement( UCHAR source, ULONG address, ULONG length, UCHAR *data );
extern  UCHAR api_emv_GetDataElement( UCHAR source, ULONG address, ULONG length, UCHAR *data );
extern  UCHAR api_emv_ClrDataElement( UCHAR source, ULONG address, ULONG length, UCHAR data );

//      L2API_05
extern  UCHAR api_emv_CheckPDOL( UCHAR tag1, UCHAR tag2 );
extern  UCHAR api_emv_ConcatenateDOL( UCHAR type, UCHAR *in_dol, UCHAR *out_dol );
extern  UCHAR api_emv_InitApplication( UCHAR tx_type, UCHAR *tx_amt );

//      L2API_06
extern  UCHAR api_emv_ReadApplicationData( void );

//      L2API_07
extern  UCHAR api_emv_OfflineSDA( UCHAR *sdtba );
extern  UCHAR api_emv_OfflineDDA( UCHAR type, UCHAR *sdtba );
extern  UCHAR api_emv_OfflineDataAuthen( void );
extern  UCHAR api_emv_OfflineCDA( UCHAR phase, UCHAR *gac );

//      L2API_08
extern  UCHAR api_emv_ProcessingRestrictions( void );

//      L2API_09
extern  UCHAR api_emv_CardholderVerification( UINT tout, UCHAR *msg, UCHAR *epb, UCHAR *ksn, UCHAR mod, UCHAR idx );

//      L2API_10
extern  UCHAR api_emv_FloorLimitChecking( UCHAR *amt1, UCHAR *amt2 );
extern  UCHAR api_emv_RandomTransactionSelection( UCHAR *amt1, UCHAR *amt2 );
extern  UCHAR api_emv_VelocityChecking( void );
extern  UCHAR api_emv_TerminalRiskManagment( UCHAR *amt_tx, UCHAR *amt_auth, UCHAR *amt_log );

//      L2API_11
extern  UCHAR api_emv_TAA_Denial( void );
extern  UCHAR api_emv_TAA_Online( void );
extern  UCHAR api_emv_TAA_Default( void );
extern  UCHAR api_emv_TerminalActionAnalysis( UCHAR online, UCHAR arc[] );
extern  void  api_emv_IncTransSequenceCounter( void );
extern  UINT  api_emv_KernelCheckSum( void );

//----------------------------------------------------------------------------
//      Application Kernel
//----------------------------------------------------------------------------
#define	KERNEL_EMV		0
#define	KERNEL_PBOC		1

//----------------------------------------------------------------------------
//      Application Kernel API Definition
//----------------------------------------------------------------------------
#define TRUE                    1
#define FALSE                   0

#define emvOK                   0x00
#define emvFailed               0x01
#define emvReady                0x00
#define emvNotReady             0x02
#define emvAborted              0x03
#define emvAutoSelected         0x04
#define emvNotSupported         0x05
#define emvOutOfService         0xff

#define DE_TERM                 0x00                    // data element TERMINAL
#define DE_ICC                  0x01                    //              ICC
#define DE_ISSUER               0x02                    //              ISSUER
#define DE_KEY                  0x03                    //              PUBLIC KEY

#define DOL_PDOL                0x00                    // GET PROCESSING OPTIONS
#define DOL_CDOL1               0x01                    // GENERATE AC
#define DOL_CDOL2               0x02                    // GENERATE AC
#define DOL_TDOL                0x03                    // TC HASH VALUE
#define DOL_DDOL                0x04                    // INTERNAL AUTHEN

#define SHA1                    0x00                    // hash algorithm
#define MD5                     0x01                    // hash algorithm

#define AC_AAC                  0x00                    // qualifier for APDU-
#define AC_TC                   0x40                    // Generate AC command
#define AC_ARQC                 0x80                    //
#define AC_AAR                  0xC0                    //
#define AC_CDA_REQ              0x10                    // CDA signature request
#define AC_CDA_REQ_MASK         0xEF                    //

#define CID_AC_MASK                     0xC0            //
#define CID_ADVICE_REQUIRED_MASK        0x08            // bit4   advice required mask
#define CID_ADVICE_CODE_MASK            0x07            // bit1-3 advice code mask
#define AVC_NO_INFO                     0x00            //        Advice Codes
#define AVC_SERVICE_NOT_ALLOWED         0x01            //
#define AVC_PIN_TRY_LIMIT_EXCEEDED      0x02            //
#define AVC_ISSUER_AUTH_FAILED          0x03            //

#define ARC_OFFLINE_APPROVED            0x5931          // "Y1"
#define ARC_OFFLINE_DECLINED            0x5A31          // "Z1"
#define ARC_ONLINE_APPROVED             0x5932          // "Y2"
#define ARC_ONLINE_DECLINED             0x5A32          // "Z2"
#define ARC_APPROVED_UNABLE_ONLINE      0x5933          // "Y3"
#define ARC_DECLINED_UNABLE_ONLINE      0x5A33          // "Z3"

                                                        // Issuer Authorization Response Code
#define ISU_ARC_APPROVED                0x3030          // "00"
#define ISU_ARC_REFERRAL1               0x3031          // "01"
#define ISU_ARC_REFERRAL2               0x3032          // "02"
#define ISU_ARC_REFERRAL3               0x3235          // "25"
#define ISU_ARC_MAX                     0x3939          // "99"
#define ISU_ARC_PBOC_APPROVED_10        0x3130          // "10" PBOC ONLY
#define ISU_ARC_PBOC_APPROVED_11        0x3131          // "11" PBOC ONLY

                                                        // POS Entry Mode pos1-2
#define PEM_UNSPECIFIED                 0x00            //
#define PEM_MANUAL                      0x01            //
#define PEM_MSR                         0x02            //
#define PEM_BARCODE                     0x03            //
#define PEM_OCR                         0x04            //
#define PEM_ICC                         0x05            //
#define PEM_FALLBACK                    0x80            // switch icc to msr

//----------------------------------------------------------------------------
//      Data Element Format
//----------------------------------------------------------------------------
#define DEF_AN                  0x00                    // an
#define DEF_ANS                 0x01                    // ans
#define DEF_B                   0x02                    // b
#define DEF_CN                  0x03                    // cn
#define DEF_N                   0x04                    // n
#define DEF_VAR                 0x05                    // var.
#define DEF_A                   0x06                    // a

#define MAX_TDE_CNT             38                      // cf. TDE_Table[]
#define MAX_IDE_CNT             72+1                    // cf. IDE_Table[]

//----------------------------------------------------------------------------
//      Application Interchange Profile
//----------------------------------------------------------------------------
// BYTE 1
#define AIP0_RFU                        0x80            // EMV96: AIP0_INITIATE
#define AIP0_OFFLINE_SDA                0x40            //
#define AIP0_OFFLINE_DDA                0x20            //
#define AIP0_CVM                        0x10            //
#define AIP0_TRM                        0x08            //
#define AIP0_ISSUER_AUTHEN              0x04            //
#define AIP0_RFU1                       0x02            //
#define AIP0_OFFLINE_CDA                0x01            // EMV2000
// BYTE 2
#define AIP1_RFU7                       0x80
#define AIP1_RFU6                       0x40
#define AIP1_RFU5                       0x20
#define AIP1_RFU4                       0x10
#define AIP1_RFU3                       0x08
#define AIP1_RFU2                       0x04
#define AIP1_RFU1                       0x02
#define AIP1_RFU0                       0x01

//----------------------------------------------------------------------------
//      Terminal Capabilities
//----------------------------------------------------------------------------
// BYTE 1
#define CAP0_MANUAL_KEY_ENTRY           0x80
#define CAP0_MAGNETIC_STRIPE            0x40
#define CAP0_IC_WITH_CONTACTS           0x20
#define CAP0_RFU4                       0x10
#define CAP0_RFU3                       0x08
#define CAP0_RFU2                       0x04
#define CAP0_RFU1                       0x02
#define CAP0_RFU0                       0x01
// BYTE 2
#define CAP1_PPIN_ICC_VERIFY            0x80
#define CAP1_EPIN_ONLINE_VERIFY         0x40
#define CAP1_PAPER_SIGN                 0x20
#define CAP1_EPIN_OFFLINE_VERIFY        0x10
#define CAP1_NO_CVM_REQUIRED            0x08            // EMV2000
#define CAP1_RFU2                       0x04
#define CAP1_RFU1                       0x02
#define CAP1_RFU0                       0x01
#define CAP1_CH_LICENSE_VERIFY          0x01            // PBOC2.0 ONLY
// BYTE 3
#define CAP2_SDA                        0x80
#define CAP2_DDA                        0x40
#define CAP2_CARD_CAPTURE               0x20
#define CAP2_RFU4                       0x10
#define CAP2_CDA                        0x08            // EMV2000
#define CAP2_RFU2                       0x04
#define CAP2_RFU1                       0x02
#define CAP2_RFU0                       0x01

//----------------------------------------------------------------------------
//      Additional Terminal Capabilities
//----------------------------------------------------------------------------
// BYTE 1
#define ATC0_CASH                       0x80
#define ATC0_GOODS                      0x40
#define ATC0_SERVICES                   0x20
#define ATC0_CASHBACK                   0x10
#define ATC0_INQUIRY                    0x08
#define ATC0_TRANSFER                   0x04
#define ATC0_PAYMENT                    0x02
#define ATC0_ADMINISTRATIVE             0x01
// BYTE 2
#define ATC1_RFU7                       0x80
#define ATC1_RFU6                       0x40
#define ATC1_RFU5                       0x20
#define ATC1_RFU4                       0x10
#define ATC1_RFU3                       0x08
#define ATC1_RFU2                       0x04
#define ATC1_RFU1                       0x02
#define ATC1_RFU0                       0x01
// BYTE 3
#define ATC2_NUM_KEYS                   0x80
#define ATC2_ASC_KEYS                   0x40
#define ATC2_CMD_KEYS                   0x20
#define ATC2_FUNC_KEYS                  0x10
#define ATC2_RFU3                       0x08
#define ATC2_RFU2                       0x04
#define ATC2_RFU1                       0x02
#define ATC2_RFU0                       0x01
// BYTE 4
#define ATC3_PRINT_ATTENDANT            0x80
#define ATC3_PRINT_CARDHOLDER           0x40
#define ATC3_DISP_ATTENDANT             0x20
#define ATC3_DISP_CARDHOLDER            0x10
#define ATC3_RFU4                       0x08
#define ATC3_RFU3                       0x04
#define ATC3_CODE_TBL_10                0x02
#define ATC3_CODE_TBL_9                 0x01
// BYTE 5
#define ATC4_CODE_TBL_8                 0x80
#define ATC4_CODE_TBL_7                 0x40
#define ATC4_CODE_TBL_6                 0x20
#define ATC4_CODE_TBL_5                 0x10
#define ATC4_CODE_TBL_4                 0x08
#define ATC4_CODE_TBL_3                 0x04
#define ATC4_CODE_TBL_2                 0x02
#define ATC4_CODE_TBL_1                 0x01

//----------------------------------------------------------------------------
//      Terminal Verification Results
//----------------------------------------------------------------------------
// BYTE 1
#define TVR0_OFFLINE_DA_NOT_PERFORMED   0x80
#define TVR0_OFFLINE_SDA_FAILED         0x40
#define TVR0_ICC_DATA_MISSING           0x20
#define TVR0_CARD_APPEARS_ON_TXF        0x10
#define TVR0_OFFLINE_DDA_FAILED         0x08
#define TVR0_OFFLINE_CDA_FAILED         0x04            // EVM2000
#define TVR0_SDA_SELECTED               0x02		// SB113
#define TVR0_RFU0                       0x01
// BYTE 2
#define TVR1_ICC_TERM_DIFF_AP_VER       0x80
#define TVR1_EXPIRED_AP                 0x40
#define TVR1_AP_NOT_YET_EFFECTIVE       0x20
#define TVR1_SERVICE_NOT_ALLOWED        0x10
#define TVR1_NEW_CARD                   0x08
#define TVR1_RFU2                       0x04
#define TVR1_RFU1                       0x02
#define TVR1_RFU0                       0x01
// BYTE 3
#define TVR2_CVM_NOT_SUCCESS            0x80
#define TVR2_UNRECONGNIZED_CVM          0x40
#define TVR2_PIN_TRY_LIMIT_EXCEEDED     0x20
#define TVR2_PIN_REQ_PINPAD_NOT_WORK    0x10
#define TVR2_PIN_REQ_PIN_NOT_ENTERED    0x08
#define TVR2_ONLINE_PIN_ENTERED         0x04
#define TVR2_RFU1                       0x02
#define TVR2_RFU0                       0x01
// BYTE 4
#define TVR3_TX_EXCEEDS_FLOOR_LIMIT     0x80
#define TVR3_LCOFF_LIMIT_EXCEEDED       0x40
#define TVR3_UCOFF_LIMIT_EXCEEDED       0x20
#define TVR3_TX_SELECTED_RAND_ONLINE    0x10
#define TVR3_MERCHANT_FORCED_TX_ONLINE  0x08
#define TVR3_RFU2                       0x04
#define TVR3_RFU1                       0x02
#define TVR3_RFU0                       0x01
// BYTE 5
#define TVR4_DEFAULT_TDOL_USED          0x80
#define TVR4_ISSUER_AUTHEN_UNSUCCESS    0x40
#define TVR4_SP_FAILED_BEFORE_FAC       0x20
#define TVR4_SP_FAILED_AFTER_FAC        0x10
#define TVR4_RFU3                       0x08
#define TVR4_RFU2                       0x04
#define TVR4_RFU1                       0x02
#define TVR4_RFU0                       0x01

//----------------------------------------------------------------------------
//      Terminal Status Information
//----------------------------------------------------------------------------
// BYTE 1
#define TSI0_OFFLINE_DA_PERFORMED       0x80
#define TSI0_CVM_PERFORMED              0x40
#define TSI0_CRM_PERFORMED              0x20
#define TSI0_ISSUER_AUTHEN_PERFORMED    0x10
#define TSI0_TRM_PERFORMED              0x08
#define TSI0_SCRIPT_PROCESS_PERFORMED   0x04
#define TSI0_RFU1                       0x02
#define TSI0_RFU0                       0x01
// BYTE 2
#define TSI1_RFU7                       0x80
#define TSI1_RFU6                       0x40
#define TSI1_RFU5                       0x20
#define TSI1_RFU4                       0x10
#define TSI1_RFU3                       0x08
#define TSI1_RFU2                       0x04
#define TSI1_RFU1                       0x02
#define TSI1_RFU0                       0x01

//----------------------------------------------------------------------------
//      Application Usage Control (ICC)
//----------------------------------------------------------------------------
#define AUC0_VALID_DOMESTIC_CASH_TX     0x80
#define AUC0_VALID_INTN_CASH_TX         0x40
#define AUC0_VALID_DOMESTIC_GOODS       0x20
#define AUC0_VALID_INTN_GOODS           0x10
#define AUC0_VALID_DOMESTIC_SERVICES    0x08
#define AUC0_VALID_INTN_SERVICES        0x04
#define AUC0_VALID_AT_ATM               0x02
#define AUC0_VALID_AT_TERM_NOT_ATM      0x01

#define AUC1_DOMESTIC_CASHBK_ALLOWED    0x80
#define AUC1_INTN_CASHBK_ALLOWED        0x40
#define AUC1_RFU5                       0x20
#define AUC1_RFU4                       0x10
#define AUC1_RFU3                       0x08
#define AUC1_RFU2                       0x04
#define AUC1_RFU1                       0x02
#define AUC1_RFU0                       0x01

//----------------------------------------------------------------------------
//      Cardholder Verification Method & Condition Codes
//----------------------------------------------------------------------------
// BYTE 1
#define CVM_OFFLINE_PIN_MASK            0x07    // offline plaintext or enciphered PIN

#define CVR_APPLY_NEXT_CVR              0x40    // 0=CV failed if this CVM is unsuccessful, 1=apply next
#define CVR_FAIL_CVM_PROCESSING         0x00    // APPLY...
#define CVR_PLAINTEXT_PIN               0x01    // offline
#define CVR_ENCIPHERED_PIN_ONLINE       0x02    // online
#define CVR_PLAINTEXT_PIN_AND_SIGN      0x03    // offline
#define CVR_ENCIPHERED_PIN              0x04    // offline
#define CVR_ENCIPHERED_PIN_AND_SIGN     0x05    // offline
#define CVR_SIGN_PAPER                  0x1E
#define CVR_NO_CVM_REQUIRED             0x1F
#define CVR_VERIFY_CH_LICENSE           0x20    // PBOC2.0 ONLY, cardholder's license
#define CVR_NOT_AVAILABLE               0x3F

// BTYE 2 (OLD)
//#define CVC_ALWAYS                      0x00
//#define CVC_CASH_OR_CASHBACK            0x01    // IF...
//#define CVC_NOT_CASH_OR_CASHBACK        0x02    //
//#define CVC_TERM_SUPPORT_CVM            0x03    //
//#define CVC_RFU4                        0x04    //
//#define CVC_RFU5                        0x05    //
//#define CVC_AMT_UNDER_X                 0x06    //
//#define CVC_AMT_OVER_X                  0x07    //
//#define CVC_AMT_UNDER_Y                 0x08    //
//#define CVC_AMT_OVER_Y                  0x09    //

// BYTE 2 (NEW)
#define CVC_ALWAYS                      0x00	// IF...
#define CVC_UNATTENDED_CASH            	0x01    // CVC_CASH_OR_CASHBACK
#define CVC_NOT_CASH_OR_CASHBACK        0x02    //
#define CVC_TERM_SUPPORT_CVM            0x03    //
#define CVC_MANUAL_CASH                 0x04    // CVC_RFU4
#define CVC_PURCHASE_WITH_CASHBACK      0x05    // CVC_RFU5
#define CVC_AMT_UNDER_X                 0x06    //
#define CVC_AMT_OVER_X                  0x07    //
#define CVC_AMT_UNDER_Y                 0x08    //
#define CVC_AMT_OVER_Y                  0x09    //

//----------------------------------------------------------------------------
//      CVM Result Codes (the 3'rd byte of CVM Results)
//----------------------------------------------------------------------------
#define CVMR_UNKNOWN                    0x00
#define CVMR_FAILED                     0x01
#define CVMR_SUCCESSFUL                 0x02

//----------------------------------------------------------------------------
//      Memory Management
//
//      PAGE 00 (32K): Downloadable Parameters & Fonts
//      PAGE 01 (32K): Transaction Log
//
//      PAGE 12 (32K): Terminal Data Element
//      PAGE 13 (32K): ICC Data Element
//      PAGE 14 (32K): Key Management
//      PAGE 15 (16K): Working Buffer (eg, dir stack, print queue)
//
//----------------------------------------------------------------------------
#define MAX_STACK_QUE                   8

// for PAGE config only
//#ifdef	_SRAM_1MB_
//#define SRAM_PAGE_TERM                  26
//#define SRAM_PAGE_ICC                   27
//#define SRAM_PAGE_KEY                   28
//#define SRAM_PAGE_WORK                  29
//#endif

//#ifdef	_SRAM_4MB_
//#define SRAM_PAGE_TERM                  122
//#define SRAM_PAGE_ICC                   123
//#define SRAM_PAGE_KEY                   124
//#define SRAM_PAGE_WORK                  125
//#endif

// for File System config only
//#define	SRAM_BASE_EMV_WORK		0x00006000	// 8KB
//#define	SRAM_BASE_EMV_KEY		0x00008000	// 32KB
//#define	SRAM_BASE_EMV_TERM		0x00010000	// 32KB
//#define	SRAM_BASE_EMV_ICC		0x00018000	// 32KB

// 2011-11-23, SRAM NEW CONFIG (SRAM PAGE + FS)
#define	SRAM_BASE_EMV_WORK	0x00010000			// 32KB
#define	SRAM_BASE_EMV_KEY	0x00018000			// 32KB
#define	SRAM_BASE_EMV_TERM	0x00020000			// 32KB
#define	SRAM_BASE_EMV_ICC	0x00028000			// 32KB

//****************************************************************************
//      Transaction Log         - TERMINAL
//                                Location: SRAM page 01
//                                Size    : 32KB
//                                Range   : 0x0000 - 0x7FFF
//
//      Field Name                Data Format     Length
//      ------------------------  -----------     ------
//      ROC                       n6              3
//      App PAN                   cn ~19          10
//      App PAN Sequence Number   n2              1
//      Transaction Amount        n10+n2          5+1
//      Transaction Date          n6 (YYMMDD)     3
//      Transaction Time          n6 (HHMMSS)     3
//      RFU                                       6
//      ------------------------  ------------    ------
//      TOTAL                                     32 bytes
//****************************************************************************
//#define MAX_TX_LOG_CNT                  250
//#define TX_LOG_LEN                      32
//
//#define TX_LOG_001                      0x0000
//#define TX_LOG_250                      TX_LOG_001+TX_LOG_LEN*(MAX_TX_LOG_CNT-1)
//
//#define TX_LOG_END                      TX_LOG_001+TX_LOG_LEN*MAX_TX_LOG_CNT

//****************************************************************************
//      Data Element Addressing - TERMINAL
//                                Location: SRAM page 12
//                                Size    : 32KB
//                                Range   : 0x0000 - 0x7FFF
//                                Format  : LEN1 + VALUE
//****************************************************************************

//----------------------------------------------------------------------------
//      Terminal AID
//      FORMAT: LEN[1] ASI[1] AID[16]
//              ASI: Application Selection Indicator
//                   0x00 = exact matching criterion used.
//                   0x01 = partial matching criterion used.
//              LEN: 0x00 = end of AID list.
//----------------------------------------------------------------------------
#define MAX_AID_CNT                     16
#define TERM_AID_LEN                    0x12

#define ADDR_TERM_AID_START             0x0000

#define ADDR_TERM_AID_01                ADDR_TERM_AID_START+TERM_AID_LEN*0  // 0000 - 0011
#define ADDR_TERM_AID_02                ADDR_TERM_AID_START+TERM_AID_LEN*1  // 0012 - 0023
#define ADDR_TERM_AID_03                ADDR_TERM_AID_START+TERM_AID_LEN*2  // 0024 - 0035
#define ADDR_TERM_AID_04                ADDR_TERM_AID_START+TERM_AID_LEN*3  // 0036 - 0047
#define ADDR_TERM_AID_05                ADDR_TERM_AID_START+TERM_AID_LEN*4  // 0048 - 0059
#define ADDR_TERM_AID_06                ADDR_TERM_AID_START+TERM_AID_LEN*5  // 005A - 006B
#define ADDR_TERM_AID_07                ADDR_TERM_AID_START+TERM_AID_LEN*6  // 006C - 007D
#define ADDR_TERM_AID_08                ADDR_TERM_AID_START+TERM_AID_LEN*7  // 007E - 008F

#define ADDR_TERM_AID_09                ADDR_TERM_AID_START+TERM_AID_LEN*8  // 0090 - 00A1
#define ADDR_TERM_AID_10                ADDR_TERM_AID_START+TERM_AID_LEN*9  // 00A2 - 00B3
#define ADDR_TERM_AID_11                ADDR_TERM_AID_START+TERM_AID_LEN*10 // 00B4 - 00C5
#define ADDR_TERM_AID_12                ADDR_TERM_AID_START+TERM_AID_LEN*11 // 00C6 - 00D7
#define ADDR_TERM_AID_13                ADDR_TERM_AID_START+TERM_AID_LEN*12 // 00D8 - 00E9
#define ADDR_TERM_AID_14                ADDR_TERM_AID_START+TERM_AID_LEN*13 // 00EA - 00FB
#define ADDR_TERM_AID_15                ADDR_TERM_AID_START+TERM_AID_LEN*14 // 00FC - 010D
#define ADDR_TERM_AID_16                ADDR_TERM_AID_START+TERM_AID_LEN*15 // 010E - 011F

#define ADDR_TERM_AID_END               ADDR_TERM_AID_START+TERM_AID_LEN*MAX_AID_CNT

//----------------------------------------------------------------------------
//      Candidate List
//      FORMAT: TAG[1] LEN[1] DIR[254]
//              (1) ADF Directory Entry (using PSE DIR)
//                      61-L-[4F-L-(ADF Name)...] -- Table III-3 ADF DIR
//                      or
//              (2) FCI of ADF (using the list of applications in terminal)
//                      6F-L-[84-L-(DF Name)... ] -- similar to Table II-31 ADF FCI
//----------------------------------------------------------------------------
#define MAX_CANDIDATE_CNT               16
#define CANDIDATE_LEN                   0x0100

#define ADDR_CANDIDATE_START            ADDR_TERM_AID_END

#define ADDR_CANDIDATE_01               ADDR_CANDIDATE_START+CANDIDATE_LEN*0  // 0120 - 021F
#define ADDR_CANDIDATE_02               ADDR_CANDIDATE_START+CANDIDATE_LEN*1  // 0220 - 031F
#define ADDR_CANDIDATE_03               ADDR_CANDIDATE_START+CANDIDATE_LEN*2  // 0320 - 041F
#define ADDR_CANDIDATE_04               ADDR_CANDIDATE_START+CANDIDATE_LEN*3  // 0420 - 051F
#define ADDR_CANDIDATE_05               ADDR_CANDIDATE_START+CANDIDATE_LEN*4  // 0520 - 061F
#define ADDR_CANDIDATE_06               ADDR_CANDIDATE_START+CANDIDATE_LEN*5  // 0620 - 071F
#define ADDR_CANDIDATE_07               ADDR_CANDIDATE_START+CANDIDATE_LEN*6  // 0720 - 081F
#define ADDR_CANDIDATE_08               ADDR_CANDIDATE_START+CANDIDATE_LEN*7  // 0820 - 091F

#define ADDR_CANDIDATE_09               ADDR_CANDIDATE_START+CANDIDATE_LEN*8  // 0920 - 0A1F
#define ADDR_CANDIDATE_10               ADDR_CANDIDATE_START+CANDIDATE_LEN*9  // 0A20 - 0B1F
#define ADDR_CANDIDATE_11               ADDR_CANDIDATE_START+CANDIDATE_LEN*10 // 0B20 - 0C1F
#define ADDR_CANDIDATE_12               ADDR_CANDIDATE_START+CANDIDATE_LEN*11 // 0C20 - 0D1F
#define ADDR_CANDIDATE_13               ADDR_CANDIDATE_START+CANDIDATE_LEN*12 // 0D20 - 0E1F
#define ADDR_CANDIDATE_14               ADDR_CANDIDATE_START+CANDIDATE_LEN*13 // 0E20 - 0F1F
#define ADDR_CANDIDATE_15               ADDR_CANDIDATE_START+CANDIDATE_LEN*14 // 0F20 - 101F
#define ADDR_CANDIDATE_16               ADDR_CANDIDATE_START+CANDIDATE_LEN*15 // 1020 - 111F

#define ADDR_CANDIDATE_END              ADDR_CANDIDATE_START+CANDIDATE_LEN*MAX_CANDIDATE_CNT

//----------------------------------------------------------------------------
//      Sorted Candidate Name List (16x18 array)
//      FORMAT: LINK[1]  : link index to the original candidate list. (0..15)
//              LEN[1]   : length of the following name field.
//              NAME[16] : the application prefered name or label name.
//              (1) if the LEN byte equals 0, it is the bottom of list.
//              (2) if the LEN byte equals 255, it is removed by kernel.
//----------------------------------------------------------------------------
#define MAX_CANDIDATE_NAME_CNT          16
#define CANDIDATE_NAME_LEN              0x12

#define ADDR_CANDIDATE_NAME_START       ADDR_CANDIDATE_END

#define ADDR_CANDIDATE_NAME_01          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*0  // 1120 - 1131
#define ADDR_CANDIDATE_NAME_02          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*1  // 1132 - 1143
#define ADDR_CANDIDATE_NAME_03          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*2  // 1144 - 1155
#define ADDR_CANDIDATE_NAME_04          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*3  // 1156 - 1167
#define ADDR_CANDIDATE_NAME_05          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*4  // 1168 - 1179
#define ADDR_CANDIDATE_NAME_06          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*5  // 117A - 118B
#define ADDR_CANDIDATE_NAME_07          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*6  // 118C - 119D
#define ADDR_CANDIDATE_NAME_08          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*7  // 119E - 11AF

#define ADDR_CANDIDATE_NAME_09          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*8  // 11B0 - 11C1
#define ADDR_CANDIDATE_NAME_10          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*9  // 11C2 - 11D3
#define ADDR_CANDIDATE_NAME_11          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*10 // 11D4 - 11E5
#define ADDR_CANDIDATE_NAME_12          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*11 // 11E6 - 11F7
#define ADDR_CANDIDATE_NAME_13          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*12 // 11F8 - 1209
#define ADDR_CANDIDATE_NAME_14          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*13 // 120A - 121B
#define ADDR_CANDIDATE_NAME_15          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*14 // 121C - 122D
#define ADDR_CANDIDATE_NAME_16          ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*15 // 122E - 123F

#define ADDR_CANDIDATE_NAME_END         ADDR_CANDIDATE_NAME_START+CANDIDATE_NAME_LEN*MAX_CANDIDATE_NAME_CNT

//----------------------------------------------------------------------------
//      Application Version Number (relative to ADDR_TERM_AID_XX)
//----------------------------------------------------------------------------
#define MAX_AVN_CNT                     MAX_AID_CNT
#define AVN_LEN                         2

#define ADDR_AVN_START                  ADDR_CANDIDATE_NAME_END

#define ADDR_AVN_01                     ADDR_AVN_START+AVN_LEN*0
#define ADDR_AVN_02                     ADDR_AVN_START+AVN_LEN*1
#define ADDR_AVN_03                     ADDR_AVN_START+AVN_LEN*2
#define ADDR_AVN_04                     ADDR_AVN_START+AVN_LEN*3
#define ADDR_AVN_05                     ADDR_AVN_START+AVN_LEN*4
#define ADDR_AVN_06                     ADDR_AVN_START+AVN_LEN*5
#define ADDR_AVN_07                     ADDR_AVN_START+AVN_LEN*6
#define ADDR_AVN_08                     ADDR_AVN_START+AVN_LEN*7

#define ADDR_AVN_09                     ADDR_AVN_START+AVN_LEN*8
#define ADDR_AVN_10                     ADDR_AVN_START+AVN_LEN*9
#define ADDR_AVN_11                     ADDR_AVN_START+AVN_LEN*10
#define ADDR_AVN_12                     ADDR_AVN_START+AVN_LEN*11
#define ADDR_AVN_13                     ADDR_AVN_START+AVN_LEN*12
#define ADDR_AVN_14                     ADDR_AVN_START+AVN_LEN*13
#define ADDR_AVN_15                     ADDR_AVN_START+AVN_LEN*14
#define ADDR_AVN_16                     ADDR_AVN_START+AVN_LEN*15

#define ADDR_AVN_END                    ADDR_AVN_START+AVN_LEN*MAX_AVN_CNT

//----------------------------------------------------------------------------
//      Terminal Floor Limit (relative to ADDR_TERM_AID_XX)
//----------------------------------------------------------------------------
#define MAX_TFL_CNT                     MAX_AID_CNT
#define TFL_LEN                         6               // n10+(n2)

#define ADDR_TFL_01                     ADDR_AVN_END+TFL_LEN*0
#define ADDR_TFL_02                     ADDR_AVN_END+TFL_LEN*1
#define ADDR_TFL_03                     ADDR_AVN_END+TFL_LEN*2
#define ADDR_TFL_04                     ADDR_AVN_END+TFL_LEN*3
#define ADDR_TFL_05                     ADDR_AVN_END+TFL_LEN*4
#define ADDR_TFL_06                     ADDR_AVN_END+TFL_LEN*5
#define ADDR_TFL_07                     ADDR_AVN_END+TFL_LEN*6
#define ADDR_TFL_08                     ADDR_AVN_END+TFL_LEN*7

#define ADDR_TFL_09                     ADDR_AVN_END+TFL_LEN*8
#define ADDR_TFL_10                     ADDR_AVN_END+TFL_LEN*9
#define ADDR_TFL_11                     ADDR_AVN_END+TFL_LEN*10
#define ADDR_TFL_12                     ADDR_AVN_END+TFL_LEN*11
#define ADDR_TFL_13                     ADDR_AVN_END+TFL_LEN*12
#define ADDR_TFL_14                     ADDR_AVN_END+TFL_LEN*13
#define ADDR_TFL_15                     ADDR_AVN_END+TFL_LEN*14
#define ADDR_TFL_16                     ADDR_AVN_END+TFL_LEN*15

#define ADDR_TFL_FLAG_01                ADDR_AVN_END+TFL_LEN*16 // FLAG=1: TFL present (>=0)
#define ADDR_TFL_FLAG_02                ADDR_TFL_FLAG_01+1      //
#define ADDR_TFL_FLAG_03                ADDR_TFL_FLAG_01+2      //
#define ADDR_TFL_FLAG_04                ADDR_TFL_FLAG_01+3      //
#define ADDR_TFL_FLAG_05                ADDR_TFL_FLAG_01+4      //
#define ADDR_TFL_FLAG_06                ADDR_TFL_FLAG_01+5      //
#define ADDR_TFL_FLAG_07                ADDR_TFL_FLAG_01+6      //
#define ADDR_TFL_FLAG_08                ADDR_TFL_FLAG_01+7      //

#define ADDR_TFL_FLAG_09                ADDR_TFL_FLAG_01+8      //
#define ADDR_TFL_FLAG_10                ADDR_TFL_FLAG_01+9      //
#define ADDR_TFL_FLAG_11                ADDR_TFL_FLAG_01+10     //
#define ADDR_TFL_FLAG_12                ADDR_TFL_FLAG_01+11     //
#define ADDR_TFL_FLAG_13                ADDR_TFL_FLAG_01+12     //
#define ADDR_TFL_FLAG_14                ADDR_TFL_FLAG_01+13     //
#define ADDR_TFL_FLAG_15                ADDR_TFL_FLAG_01+14     //
#define ADDR_TFL_FLAG_16                ADDR_TFL_FLAG_01+15     //

#define ADDR_TFL_FLAG                   ADDR_TFL_FLAG_01+16     //
#define ADDR_TFL                        ADDR_TFL_FLAG+1         //

#define ADDR_TFL_END                    ADDR_TFL+6

//----------------------------------------------------------------------------
//      Transaction Currency Code (relative to ADDR_TERM_AID_XX)
//----------------------------------------------------------------------------
#define MAX_TX_CC_CNT                   MAX_AID_CNT
#define TX_CC_LEN                       2               // n3

#define ADDR_TERM_TX_CC_01              ADDR_TFL_END+TX_CC_LEN*0
#define ADDR_TERM_TX_CC_16              ADDR_TFL_END+TX_CC_LEN*15

#define ADDR_TERM_TX_CC_END             ADDR_TFL_END+TX_CC_LEN*MAX_TX_CC_CNT

//----------------------------------------------------------------------------
//      Transaction Currency Exponent (relative to ADDR_TERM_AID_XX)
//----------------------------------------------------------------------------
#define MAX_TX_CE_CNT                   MAX_AID_CNT
#define TX_CE_LEN                       1               // n1

#define ADDR_TERM_TX_CE_01              ADDR_TERM_TX_CC_END+TX_CE_LEN*0
#define ADDR_TERM_TX_CE_16              ADDR_TERM_TX_CC_END+TX_CE_LEN*15

#define ADDR_TERM_TX_CE_END             ADDR_TERM_TX_CC_END+TX_CE_LEN*MAX_TX_CE_CNT

//----------------------------------------------------------------------------
//      Acquirer Identifier           (relative to ADDR_TERM_AID_XX)
//----------------------------------------------------------------------------
#define MAX_ACQ_ID_CNT                  MAX_AID_CNT
#define ACQ_ID_LEN                      6               // n6-11

#define ADDR_TERM_ACQ_ID_01             ADDR_TERM_TX_CE_END+ACQ_ID_LEN*0
#define ADDR_TERM_ACQ_ID_16             ADDR_TERM_TX_CE_END+ACQ_ID_LEN*15

#define ADDR_TERM_ACQ_ID_END            ADDR_TERM_TX_CE_END+ACQ_ID_LEN*MAX_ACQ_ID_CNT

//----------------------------------------------------------------------------
//      Merchant Category Code        (relative to ADDR_TERM_AID_XX)
//----------------------------------------------------------------------------
#define MAX_MCC_CNT                     MAX_AID_CNT
#define MCC_LEN                         2               // n4

#define ADDR_TERM_MCC_01                ADDR_TERM_ACQ_ID_END+MCC_LEN*0
#define ADDR_TERM_MCC_16                ADDR_TERM_ACQ_ID_END+MCC_LEN*15

#define ADDR_TERM_MCC_END               ADDR_TERM_ACQ_ID_END+MCC_LEN*MAX_MCC_CNT

//----------------------------------------------------------------------------
//      Terminal Capabilities         (relative to ADDR_TERM_AID_XX)
//----------------------------------------------------------------------------
#define MAX_TERM_CAP_CNT                MAX_AID_CNT
#define TERM_CAP_LEN                    3               // b3

#define ADDR_TERM_CAP_01                ADDR_TERM_MCC_END+TERM_CAP_LEN*0
#define ADDR_TERM_CAP_16                ADDR_TERM_MCC_END+TERM_CAP_LEN*15

#define ADDR_TERM_CAP_END               ADDR_TERM_MCC_END+TERM_CAP_LEN*MAX_TERM_CAP_CNT

//----------------------------------------------------------------------------
//      Additional Terminal Capabilities (relative to ADDR_TERM_AID_XX)
//----------------------------------------------------------------------------
#define MAX_TERM_ADD_CAP_CNT            MAX_AID_CNT
#define TERM_ADD_CAP_LEN                5               // b5

#define ADDR_TERM_ADD_CAP_01            ADDR_TERM_CAP_END+TERM_ADD_CAP_LEN*0
#define ADDR_TERM_ADD_CAP_16            ADDR_TERM_CAP_END+TERM_ADD_CAP_LEN*15

#define ADDR_TERM_ADD_CAP_END           ADDR_TERM_CAP_END+TERM_ADD_CAP_LEN*MAX_TERM_ADD_CAP_CNT

//----------------------------------------------------------------------------
//      Threshold Value for BRS       (relative to ADDR_TERM_AID_XX)
//----------------------------------------------------------------------------
#define MAX_TERM_BRS_THRESHOLD          MAX_AID_CNT
#define BRS_THRESHOLD_LEN               6               // n10+(n2)

#define ADDR_TERM_BRS_THRESHOLD_01      ADDR_TERM_ADD_CAP_END+BRS_THRESHOLD_LEN*0
#define ADDR_TERM_BRS_THRESOHLD_16      ADDR_TERM_ADD_CAP_END+BRS_THRESHOLD_LEN*15

#define ADDR_TERM_BRS_THRESHOLD_END     ADDR_TERM_ADD_CAP_END+BRS_THRESHOLD_LEN*MAX_TERM_BRS_THRESHOLD

//----------------------------------------------------------------------------
//      Target Percentage for BR      (relative to ADDR_TERM_AID_XX)
//----------------------------------------------------------------------------
#define MAX_TERM_RS_TP                  MAX_AID_CNT
#define RS_TP_LEN                       1               // n2

#define ADDR_TERM_RS_TP_01              ADDR_TERM_BRS_THRESHOLD_END+RS_TP_LEN*0
#define ADDR_TERM_RS_TP_16              ADDR_TERM_BRS_THRESHOLD_END+RS_TP_LEN*15

#define ADDR_TERM_RS_TP_END             ADDR_TERM_BRS_THRESHOLD_END+RS_TP_LEN*MAX_TERM_RS_TP

//----------------------------------------------------------------------------
//      Max Target Percentage for BRS (relative to ADDR_TERM_AID_XX)
//----------------------------------------------------------------------------
#define MAX_TERM_BRS_MTP                MAX_AID_CNT
#define BRS_MTP_LEN                     1               // n2

#define ADDR_TERM_BRS_MTP_01            ADDR_TERM_RS_TP_END+BRS_MTP_LEN*0
#define ADDR_TERM_BRS_MTP_16            ADDR_TERM_RS_TP_END+BRS_MTP_LEN*15

#define ADDR_TERM_BRS_MTP_END           ADDR_TERM_RS_TP_END+BRS_MTP_LEN*MAX_TERM_BRS_MTP

//----------------------------------------------------------------------------
//      NCCC RFU Function Parameters
//----------------------------------------------------------------------------
#define ADDR_TERM_MAX_DECIMAL           ADDR_TERM_BRS_MTP_END     // 2L-V(1)
#define ADDR_TERM_ENTRY_CAP             ADDR_TERM_MAX_DECIMAL+3   // 2L-V(1)
//#define ADDR_TERM_TMS_EMV               ADDR_TERM_ENTRY_CAP+3     // 1=TMS EMV para are valid

#define ADDR_TERM_ENTRY_CAP_END         ADDR_TERM_ENTRY_CAP+3

//----------------------------------------------------------------------------
//      Terminal-Related Data Object
//      DATA[n]
//
//      FORMAT[1]: 0=an, 1=ans, 2=b, 3=cn, 4=n, 5=var.
//      LEN[1]   : actual length of the following data. (in byte)
//      NOTE     : "--" means no tags supported
//----------------------------------------------------------------------------
#define ADDR_TERM_DDOL                  ADDR_TERM_ENTRY_CAP_END   // b252, 2L-V, terminal DDOL

#define ADDR_TERM_ACQ_ID                ADDR_TERM_DDOL+254        // n6,   acquirer id
#define ADDR_TERM_ADD_CAP               ADDR_TERM_ACQ_ID+6        // b5,   additional terminal capabilities
#define ADDR_TERM_AMT_AUTH_B            ADDR_TERM_ADD_CAP+5       // b4,   amount, authorized (binary)
#define ADDR_TERM_AMT_AUTH_N            ADDR_TERM_AMT_AUTH_B+4    // n6,   amount, authorized (numeric)
#define ADDR_TERM_AMT_OTHER_B           ADDR_TERM_AMT_AUTH_N+6    // b4,   amount, other (binary)
#define ADDR_TERM_AMT_OTHER_N           ADDR_TERM_AMT_OTHER_B+4   // n6,   amount, other (numeric)
#define ADDR_TERM_AMT_RC                ADDR_TERM_AMT_OTHER_N+6   // b4,   amount, reference currency (binary)
#define ADDR_TERM_AID                   ADDR_TERM_AMT_RC+4        // b16,  application id
#define ADDR_TERM_AVN                   ADDR_TERM_AID+16          // b2,   application version number
#define ADDR_TERM_ARC                   ADDR_TERM_AVN+2           // an2,  authorization response code
#define ADDR_TERM_CVMR                  ADDR_TERM_ARC+2           // b3,   CVM result
#define ADDR_TERM_CA_PK_INDEX           ADDR_TERM_CVMR+3          // b1,   CA public key index
#define ADDR_TERM_IFD_SN                ADDR_TERM_CA_PK_INDEX+1   // an8,  IFD serial number
//#define ADDR_TERM_TDOL                              --  // terminal TDOL (if ICC TDOL not present)
//#define ADDR_TERM_MTP_BRS                           --  // max. target percentage to be used for biased random selection
#define ADDR_TERM_MCC                   ADDR_TERM_IFD_SN+8        // n2,   merchant category code
#define ADDR_TERM_MID                   ADDR_TERM_MCC+2           // ans15,merchant id
//#define ADDR_TERM_MSG_TYPE                              // message type
#define ADDR_TERM_PEM                   ADDR_TERM_MID+15          // n1,   POS entry mode
//#define ADDR_TERM_TAC_DEFAULT                       --  // terminal action code - default
//#define ADDR_TERM_TAC_DENIAL                        --  // terminal action code - denial
//#define ADDR_TERM_TAC_ONLINE                        --  // terminal action ocde - online
//#define ADDR_TERM_TP_RS                             --  // target percentage for random selection
#define ADDR_TERM_CAP                   ADDR_TERM_PEM+1           // b3,   terminal capabilities
#define ADDR_TERM_CNTR_CODE             ADDR_TERM_CAP+3           // n2,   terminal country code
#define ADDR_TERM_FL                    ADDR_TERM_CNTR_CODE+2     // b4,   floor limit
#define ADDR_TERM_TID                   ADDR_TERM_FL+4            // an8,  terimal id
#define ADDR_TERM_TRM_DATA              ADDR_TERM_TID+8           // b8,   terminal risk management data
#define ADDR_TERM_TYPE                  ADDR_TERM_TRM_DATA+8      // n1,   terminal type
#define ADDR_TERM_TVR                   ADDR_TERM_TYPE+1          // b5,   terminal verification results
//#define ADDR_TERM_BRS_TV                            --  // threshold value for biased random selection
//#define ADDR_TERM_TX_AMT                            --  //  n6,   transaction amount (including tips & adjustments)
#define ADDR_TERM_TC_SHA1               ADDR_TERM_TVR+5           // b20,  transaction certificate hash value
#define ADDR_TERM_TX_CC                 ADDR_TERM_TC_SHA1+20      // n2,   transaction currency code
#define ADDR_TERM_TX_CE                 ADDR_TERM_TX_CC+2         // n1,   transaction currency exponent
#define ADDR_TERM_TX_DATE               ADDR_TERM_TX_CE+1         // n3,   transaction date
//#define ADDR_TERM_TX_PIN                                // transaction pin data
#define ADDR_TERM_TX_RCC                ADDR_TERM_TX_DATE+3       // n2,   transaction reference currency code
#define ADDR_TERM_TX_RCE                ADDR_TERM_TX_RCC+2        // n1,   transaction reference currency exponent
#define ADDR_TERM_TX_SC                 ADDR_TERM_TX_RCE+1        // n4,   transaction sequence counter
#define ADDR_TERM_TSI                   ADDR_TERM_TX_SC+4         // b2,   transaction status information
#define ADDR_TERM_TX_TIME               ADDR_TERM_TSI+2           // n3,   transaction time
#define ADDR_TERM_TX_TYPE               ADDR_TERM_TX_TIME+3       // n1,   transaction type
#define ADDR_TERM_UPD_NBR               ADDR_TERM_TX_TYPE+1       // b4,   unpredictable number

#define ADDR_TERM_ISR                   ADDR_TERM_UPD_NBR+4       // b81,  CNT+5*16, issuer script results (CNT=0..16)
#define ADDR_TERM_TCC                   ADDR_TERM_ISR+81          // an1,  transaction category code

//#define ADDR_TERM_MNL                   ADDR_TERM_TCC+1           // ans248, 2L-V, merchant name & location

#define ADDR_TERM_END                   ADDR_TERM_TCC+1

//----------------------------------------------------------------------------
//      Terminal Configuratins & Non-tagged Data Objects
//----------------------------------------------------------------------------
#define ADDR_TERM_CONFIG_ID             ADDR_TERM_END             // b1
#define ADDR_TERM_TX_AMT                ADDR_TERM_CONFIG_ID+1     // n6,   transaction amount (including tips & adjustments)
#define ADDR_TERM_UPD_NBR_LEN           ADDR_TERM_TX_AMT+6        // b1,   length of uprediactable number
#define ADDR_TERM_EPIN_DATA             ADDR_TERM_UPD_NBR_LEN+1   // b250, 2L-V, enciphered PIN data
#define	ADDR_TERM_KSN			ADDR_TERM_EPIN_DATA+250	  // b10,  2L-V, key serial number for DUKPT
#define ADDR_TERM_LANG_PREFER           ADDR_TERM_KSN+12          // an8,  2L-V (2*4 language preferences)

#define ADDR_TERM_CONFIG_END            ADDR_TERM_LANG_PREFER+10

//----------------------------------------------------------------------------
//      Payment System Settings
//----------------------------------------------------------------------------
//#define BRS_THRESHOLD_LEN               6                         // n10+(n2)
#define ADDR_TERM_BRS_THRESHOLD         ADDR_TERM_CONFIG_END      // n6, threshold value for BRS
#define ADDR_TERM_BRS_MTP               ADDR_TERM_BRS_THRESHOLD+6 // n1, max target percentage for BRS
#define ADDR_TERM_RS_TP                 ADDR_TERM_BRS_MTP+1       // n1, target percentage for RS

#define ADDR_TERM_TAC_DEFAULT           ADDR_TERM_RS_TP+1         // b5, terminal action code - default
#define ADDR_TERM_TAC_DENIAL            ADDR_TERM_TAC_DEFAULT+5   // b5, terminal action code - denial
#define ADDR_TERM_TAC_ONLINE            ADDR_TERM_TAC_DENIAL+5    // b5, terminal action ocde - online

#define TAC_LEN                         5                         // 5 * 16 AID
#define ADDR_TERM_TAC_DEFAULT_01        ADDR_TERM_TAC_ONLINE+5
#define ADDR_TERM_TAC_DEFAULT_02        ADDR_TERM_TAC_DEFAULT_01+5
#define ADDR_TERM_TAC_DEFAULT_03        ADDR_TERM_TAC_DEFAULT_01+10
#define ADDR_TERM_TAC_DEFAULT_04        ADDR_TERM_TAC_DEFAULT_01+15
#define ADDR_TERM_TAC_DEFAULT_05        ADDR_TERM_TAC_DEFAULT_01+20
#define ADDR_TERM_TAC_DEFAULT_06        ADDR_TERM_TAC_DEFAULT_01+25
#define ADDR_TERM_TAC_DEFAULT_07        ADDR_TERM_TAC_DEFAULT_01+30
#define ADDR_TERM_TAC_DEFAULT_08        ADDR_TERM_TAC_DEFAULT_01+35

#define ADDR_TERM_TAC_DEFAULT_09        ADDR_TERM_TAC_DEFAULT_01+40
#define ADDR_TERM_TAC_DEFAULT_10        ADDR_TERM_TAC_DEFAULT_01+45
#define ADDR_TERM_TAC_DEFAULT_11        ADDR_TERM_TAC_DEFAULT_01+50
#define ADDR_TERM_TAC_DEFAULT_12        ADDR_TERM_TAC_DEFAULT_01+55
#define ADDR_TERM_TAC_DEFAULT_13        ADDR_TERM_TAC_DEFAULT_01+60
#define ADDR_TERM_TAC_DEFAULT_14        ADDR_TERM_TAC_DEFAULT_01+65
#define ADDR_TERM_TAC_DEFAULT_15        ADDR_TERM_TAC_DEFAULT_01+70
#define ADDR_TERM_TAC_DEFAULT_16        ADDR_TERM_TAC_DEFAULT_01+75

#define ADDR_TERM_TAC_DENIAL_01         ADDR_TERM_TAC_DEFAULT_16+5
#define ADDR_TERM_TAC_DENIAL_02         ADDR_TERM_TAC_DENIAL_01+5
#define ADDR_TERM_TAC_DENIAL_03         ADDR_TERM_TAC_DENIAL_01+10
#define ADDR_TERM_TAC_DENIAL_04         ADDR_TERM_TAC_DENIAL_01+15
#define ADDR_TERM_TAC_DENIAL_05         ADDR_TERM_TAC_DENIAL_01+20
#define ADDR_TERM_TAC_DENIAL_06         ADDR_TERM_TAC_DENIAL_01+25
#define ADDR_TERM_TAC_DENIAL_07         ADDR_TERM_TAC_DENIAL_01+30
#define ADDR_TERM_TAC_DENIAL_08         ADDR_TERM_TAC_DENIAL_01+35

#define ADDR_TERM_TAC_DENIAL_09         ADDR_TERM_TAC_DENIAL_01+40
#define ADDR_TERM_TAC_DENIAL_10         ADDR_TERM_TAC_DENIAL_01+45
#define ADDR_TERM_TAC_DENIAL_11         ADDR_TERM_TAC_DENIAL_01+50
#define ADDR_TERM_TAC_DENIAL_12         ADDR_TERM_TAC_DENIAL_01+55
#define ADDR_TERM_TAC_DENIAL_13         ADDR_TERM_TAC_DENIAL_01+60
#define ADDR_TERM_TAC_DENIAL_14         ADDR_TERM_TAC_DENIAL_01+65
#define ADDR_TERM_TAC_DENIAL_15         ADDR_TERM_TAC_DENIAL_01+70
#define ADDR_TERM_TAC_DENIAL_16         ADDR_TERM_TAC_DENIAL_01+75

#define ADDR_TERM_TAC_ONLINE_01         ADDR_TERM_TAC_DENIAL_16+5
#define ADDR_TERM_TAC_ONLINE_02         ADDR_TERM_TAC_ONLINE_01+5
#define ADDR_TERM_TAC_ONLINE_03         ADDR_TERM_TAC_ONLINE_01+10
#define ADDR_TERM_TAC_ONLINE_04         ADDR_TERM_TAC_ONLINE_01+15
#define ADDR_TERM_TAC_ONLINE_05         ADDR_TERM_TAC_ONLINE_01+20
#define ADDR_TERM_TAC_ONLINE_06         ADDR_TERM_TAC_ONLINE_01+25
#define ADDR_TERM_TAC_ONLINE_07         ADDR_TERM_TAC_ONLINE_01+30
#define ADDR_TERM_TAC_ONLINE_08         ADDR_TERM_TAC_ONLINE_01+35

#define ADDR_TERM_TAC_ONLINE_09         ADDR_TERM_TAC_ONLINE_01+40
#define ADDR_TERM_TAC_ONLINE_10         ADDR_TERM_TAC_ONLINE_01+45
#define ADDR_TERM_TAC_ONLINE_11         ADDR_TERM_TAC_ONLINE_01+50
#define ADDR_TERM_TAC_ONLINE_12         ADDR_TERM_TAC_ONLINE_01+55
#define ADDR_TERM_TAC_ONLINE_13         ADDR_TERM_TAC_ONLINE_01+60
#define ADDR_TERM_TAC_ONLINE_14         ADDR_TERM_TAC_ONLINE_01+65
#define ADDR_TERM_TAC_ONLINE_15         ADDR_TERM_TAC_ONLINE_01+70
#define ADDR_TERM_TAC_ONLINE_16         ADDR_TERM_TAC_ONLINE_01+75

#define ADDR_TERM_TDOL                  ADDR_TERM_TAC_ONLINE_16+5    // b252, 2L-V

#define ADDR_TERM_PS_END                ADDR_TERM_TDOL+254

//----------------------------------------------------------------------------
//      Issuer-Related Data Object
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//      RID[5]
//      INDEX[1]
//      Certificate Serial Number of Isser PK Certificate[3]
//----------------------------------------------------------------------------
#define MAX_ISU_CSN_CNT                 50
#define ISU_CSN_LEN                     9

#define ADDR_ISU_CSN_01                 ADDR_TERM_PS_END
#define ADDR_ISU_CSN_50                 ADDR_ISU_CSN_01+ISU_CSN_LEN*(MAX_ISU_CSN_CNT-1)

#define ADDR_ISU_CSN_END                ADDR_ISU_CSN_01+ISU_CSN_LEN*MAX_ISU_CSN_CNT

#define ADDR_ISU_AUTH_CODE              ADDR_ISU_CSN_END          // an6, authorization code (approval code)
#define ADDR_ISU_ARC                    ADDR_ISU_AUTH_CODE+6      // an2, authorization response code
#define ADDR_ISU_AUTH_DATA_LEN          ADDR_ISU_ARC+2            // b2,  2L, length of issuer authentication data
#define ADDR_ISU_AUTH_DATA              ADDR_ISU_AUTH_DATA_LEN+2  // b16, V[8~16], issuer authentication data
#define ADDR_ISU_SCRIPT_TEMP            ADDR_ISU_AUTH_DATA+16     // b1024, 2L-V[1024], issuer script template buffer

#define ADDR_ISU_ARPC                   ADDR_ISU_SCRIPT_TEMP+1026 // b8,  2L-V

//#define ADDR_ISSUER_SCRIPT_ID                           // script id


//****************************************************************************
//      Data Element Addressing - ICC
//                                Location: SRAM page 13
//                                Size    : 32KB
//                                Range   : 0x0000 - 0x7FFF
//                                Format  : LEN1 + VALUE
//****************************************************************************

//----------------------------------------------------------------------------
//      The FCI of the Selected ADF
//      FORMAT: 6F-L-[84-L-(DF Name)... ]
//----------------------------------------------------------------------------
#define MAX_SELECTED_FCI_CNT            1
#define SELECTED_FCI_LEN                0x100

#define ADDR_SELECTED_FCI               0x0000

#define ADDR_SELECTED_FCI_END           ADDR_SELECTED_FCI+MAX_SELECTED_FCI_CNT*SELECTED_FCI_LEN

#define SELECTED_AID_LEN                17                        // LEN[1] RID[5]PIX[0..11]
#define RID_LEN                         5
#define ADDR_SELECTED_AID               ADDR_SELECTED_FCI_END     // AID or DFNAME

#define ADDR_SELECTED_AID_END           ADDR_SELECTED_AID+SELECTED_AID_LEN

//----------------------------------------------------------------------------
//      ICC-Related Data Object
//
//      LEN1[1] LEN2[1] DATA[n]
//      If LEN1+256*LEN2 = 0: no DATA present.
//----------------------------------------------------------------------------
#define ICC_AIP_LEN                     2
#define ADDR_ICC_AIP                    ADDR_SELECTED_AID_END     // b2,   app interchange profile

#define ICC_AFL_LEN                     4
#define ICC_AFL_TOTAL_LEN               253
#define ADDR_ICC_AFL                    ADDR_ICC_AIP+2            // b253, CNT[1] + AFL[4*CNT], CNT=1..63,  app file locator

#define ADDR_ICC_DE_START               ADDR_ICC_AFL+253
                                        // -----------------------------------
#define ADDR_ICC_AP_CC                  ADDR_ICC_DE_START         // n2,   app currency code
#define ADDR_ICC_AP_CE                  ADDR_ICC_AP_CC+4          // n1,   app currency exponent
#define ADDR_ICC_AP_DD                  ADDR_ICC_AP_CE+3          // b32,  app discretionary data
#define ADDR_ICC_AP_EFFECT_DATE         ADDR_ICC_AP_DD+34         // n3,   app effective date
#define ADDR_ICC_AP_EXPIRE_DATE         ADDR_ICC_AP_EFFECT_DATE+5 // n3,   app expiration date
#define ADDR_ICC_AP_PAN                 ADDR_ICC_AP_EXPIRE_DATE+5 // cn10, app PAN
#define ADDR_ICC_AP_PAN_SN              ADDR_ICC_AP_PAN+12        // n1,   app PAN serial nbr
#define ADDR_ICC_AP_RC                  ADDR_ICC_AP_PAN_SN+3      // n8,   app reference currency
#define ADDR_ICC_AP_RCE                 ADDR_ICC_AP_RC+10         // n4,   app reference currency exponent
#define ADDR_ICC_ATC                    ADDR_ICC_AP_RCE+6         // b2,   app transaction counter
#define ADDR_ICC_AUC                    ADDR_ICC_ATC+4            // b2,   app usage control
#define ADDR_ICC_AVN                    ADDR_ICC_AUC+4            // b2,   app version number
#define ADDR_ICC_CDOL1                  ADDR_ICC_AVN+4            // b252, CDOL1
#define ADDR_ICC_CDOL2                  ADDR_ICC_CDOL1+254        // b252, CDOL2
#define ADDR_ICC_CH_NAME                ADDR_ICC_CDOL2+254        // ans26,cardholder name
#define ADDR_ICC_CH_NAME_EX             ADDR_ICC_CH_NAME+28       // ans45,cardholder name extended
#define ADDR_ICC_CVM_LIST               ADDR_ICC_CH_NAME_EX+47    // b252, CVM list
#define ADDR_ICC_CA_PKI                 ADDR_ICC_CVM_LIST+254     // b1,   CA public key index
#define ADDR_ICC_CMD_PERFORM            ADDR_ICC_CA_PKI+3         // b239, command to perform
#define ADDR_ICC_CID                    ADDR_ICC_CMD_PERFORM+241  // b1,   cryptogram info data
#define ADDR_ICC_DAC                    ADDR_ICC_CID+3            // b2,   data authen code
#define ADDR_ICC_DDOL                   ADDR_ICC_DAC+4            // b252, DDOL
#define ADDR_ICC_DYNAMIC_NBR            ADDR_ICC_DDOL+254         // b8,   icc dynamic number
#define ADDR_ICC_PIN_EPKC               ADDR_ICC_DYNAMIC_NBR+10   // b256, icc pin encipherment public key certificate
#define ADDR_ICC_PIN_EPKE               ADDR_ICC_PIN_EPKC+258     // b3                                    exponent
#define ADDR_ICC_PIN_EPKR               ADDR_ICC_PIN_EPKE+5       // b256                                  remainder
#define ADDR_ICC_PKC                    ADDR_ICC_PIN_EPKR+258     // b256, icc public key certificate
#define ADDR_ICC_PKE                    ADDR_ICC_PKC+258          // b3,                  exponent
#define ADDR_ICC_PKR                    ADDR_ICC_PKE+5            // b256,                remainder
#define ADDR_ICC_IAC_DEFAULT            ADDR_ICC_PKR+258          // b5,   issuer action code - default
#define ADDR_ICC_IAC_DENIAL             ADDR_ICC_IAC_DEFAULT+7    // b5,                      - denial
#define ADDR_ICC_IAC_ONLINE             ADDR_ICC_IAC_DENIAL+7     // b5,                      - online
#define ADDR_ICC_ISU_AP_DATA            ADDR_ICC_IAC_ONLINE+7     // b32,   issuer application data
#define ADDR_ICC_ISU_CTI_OLD            ADDR_ICC_ISU_AP_DATA+34   // n1,    issuer code table index
#define ADDR_ICC_ISU_CNTR_CODE          ADDR_ICC_ISU_CTI_OLD+3    // n2,    issuer country code
#define ADDR_ICC_ISU_PKC                ADDR_ICC_ISU_CNTR_CODE+4  // b256,  issuer public key certificate
#define ADDR_ICC_ISU_PKE                ADDR_ICC_ISU_PKC+258      // b3                       exponent
#define ADDR_ICC_ISU_PKR                ADDR_ICC_ISU_PKE+5        // b256,                    remainder
#define ADDR_ICC_LANG_PREFER_OLD        ADDR_ICC_ISU_PKR+258      // an8,   language preference (2*4languages)
#define ADDR_ICC_ATC_LAST_ONLINE        ADDR_ICC_LANG_PREFER_OLD+10   // b2,    last online ATC register
#define ADDR_ICC_LCOL                   ADDR_ICC_ATC_LAST_ONLINE+4 // b1,   lower consecutive offline limit
#define ADDR_ICC_PIN_TRY_CNT            ADDR_ICC_LCOL+3           // b1,    pin try counter
#define ADDR_ICC_SERVICE_CODE           ADDR_ICC_PIN_TRY_CNT+3    // n2,    service code on track 1 and 2
#define ADDR_ICC_SIGNED_DAD             ADDR_ICC_SERVICE_CODE+4   // b256,  signed dynamic app data
#define ADDR_ICC_SIGNED_SAD             ADDR_ICC_SIGNED_DAD+258   // b256,  signed static  app data
#define ADDR_ICC_SDA_TL                 ADDR_ICC_SIGNED_SAD+258   // b256,  static data authen tag list
#define ADDR_ICC_TRK1_DISD              ADDR_ICC_SDA_TL+258       // ans256,track1 discretionary data
#define ADDR_ICC_TRK2_DISD              ADDR_ICC_TRK1_DISD+258    // cn256 ,track2 ...
#define ADDR_ICC_TRK2_EQUD              ADDR_ICC_TRK2_DISD+258    // b19,   track2 equivalent data
#define ADDR_ICC_TDOL                   ADDR_ICC_TRK2_EQUD+21     // b252,  TDOL
#define ADDR_ICC_UCOL                   ADDR_ICC_TDOL+254         // b1,    upper consecutive offline limit
#define ADDR_ICC_AC                     ADDR_ICC_UCOL+3           // b8,    app cryptogram
#define ADDR_ICC_AIP2                   ADDR_ICC_AC+10            // b2,    app interchange profile
#define ADDR_ICC_AFL2                   ADDR_ICC_AIP2+4           // b252,  app file locator

#define ADDR_ICC_ISU_PKM                ADDR_ICC_AFL2+254         // b256, 2L-V, result from SDA or DDA
#define ADDR_ICC_PKM                    ADDR_ICC_ISU_PKM+258      // b256, 2L-V, result from DDA
                                                                  // EMV2000 new ICC data elements:
#define ADDR_ICC_IIN                    ADDR_ICC_PKM+258          // n6,    issuer id number (forms the first part of PAN)
#define ADDR_ICC_ISU_URL                ADDR_ICC_IIN+5            // ans248,2L-URL
#define ADDR_ICC_IBAN                   ADDR_ICC_ISU_URL+250      // b34,   2L-V, international bank account no.
#define ADDR_ICC_BIC                    ADDR_ICC_IBAN+36          // b11,   2L-V, bank id code
#define ADDR_ICC_ISU_CNTR_CODE2         ADDR_ICC_BIC+13           // a2,    issuer country code - using 2 char aplpha code
#define ADDR_ICC_ISU_CNTR_CODE3         ADDR_ICC_ISU_CNTR_CODE2+4 // a3,    issuer country code - using 3 char aplpha code
#define ADDR_ICC_LOG_ENTRY              ADDR_ICC_ISU_CNTR_CODE3+5 // b2,    log entry (FCI.ADF)
#define ADDR_ICC_LOG_FORMAT             ADDR_ICC_LOG_ENTRY+4      // b248,  2L-V, log format
#define ADDR_ICC_CH_LICENSE_ID          ADDR_ICC_LOG_FORMAT+250   // an40,  2L-V, PBOC2.0
#define ADDR_ICC_CH_LICENSE_TYPE        ADDR_ICC_CH_LICENSE_ID+42 // cn1 ,  2L-V, PBOC2.0

#define	ADDR_ICC_CARD_PRODUCT_ID	ADDR_ICC_CH_LICENSE_TYPE+3 // b16,  2L-V, CUP POS

#define ADDR_ICC_DE_END                 ADDR_ICC_CARD_PRODUCT_ID+18

                                        // ------------------------------------
#define ADDR_ICC_FILE_DE_START          ADDR_ICC_DE_END

#define ADDR_ICC_AID                    ADDR_ICC_FILE_DE_START    // b16,   application identifier (ADF name)
#define ADDR_ICC_DFNAME                 ADDR_ICC_AID+18           // b16,   dedicated file name
#define ADDR_ICC_DDFNAME                ADDR_ICC_DFNAME+18        // b16,   directory definition file name
#define ADDR_ICC_AP_LABEL               ADDR_ICC_DDFNAME+18       // ans16, application label
#define ADDR_ICC_AP_PREFER_NAME         ADDR_ICC_AP_LABEL+18      // ans16, application prefered name
#define ADDR_ICC_AP_PI                  ADDR_ICC_AP_PREFER_NAME+18 //b1 ,   application priority indicator
#define ADDR_ICC_FCI_IDD                ADDR_ICC_AP_PI+3          // b222,  FCI issuer discretionary data
#define ADDR_ICC_PDOL                   ADDR_ICC_FCI_IDD+224      // b254,  processing options data object list
#define ADDR_ICC_PDOL_CATS              ADDR_ICC_PDOL+256         // b254,  concatenation of PDOL (83-L-V)
#define ADDR_ICC_CDOL1_CATS             ADDR_ICC_PDOL_CATS+256    // b254,  concatenation of CDOL1
#define ADDR_ICC_CDOL2_CATS             ADDR_ICC_CDOL1_CATS+256   // b254,  concatenation of CDOL2
#define ADDR_ICC_LANG_PREFER            ADDR_ICC_CDOL2_CATS+256   // an8,   language preference (2*4languages)
#define ADDR_ICC_ISU_CTI                ADDR_ICC_LANG_PREFER+10   // n1,    issuer code table index

#define ADDR_ICC_FILE_DE_END            ADDR_ICC_ISU_CTI+3

                                        // -----------------------------------

//#define ADDR_ICC_ISU_PKM                ADDR_ICC_FILE_DE_END      // b256, 2L-V, result from SDA or DDA
//#define ADDR_ICC_PKM                    ADDR_ICC_ISU_PKM+256      // b256, 2L-V, result from DDA

//#define ADDR_ICC_SFI                                    // SFI to AEF or DDF

//----------------------------------------------------------------------------
//      ICC Record Data (for SFI = 1~30)
//      FORMAT: ODA[1] DATA[269]
//                     70-L-V (excluding SW1 SW2)
//              ODA: bit8 = 1 : record for offline data authen.
//                   bit6-7   : RFU
//                   bit1-5   : SFI
//              L  : length 1~3 bytes.
//----------------------------------------------------------------------------
#define ADDR_ICC_REC_START              ADDR_ICC_FILE_DE_END

//#define MAX_ICC_REC_CNT                 80//EMV 42a 2009-03-03 Charles                         // PATCH: 2006-10-04, org = 30
#define MAX_ICC_REC_CNT                 30
#define ICC_REC_LEN                     270

#define ADDR_ICC_REC_01                 ADDR_ICC_REC_START+ICC_REC_LEN*0
#define ADDR_ICC_REC_02                 ADDR_ICC_REC_START+ICC_REC_LEN*1
#define ADDR_ICC_REC_03                 ADDR_ICC_REC_START+ICC_REC_LEN*2
#define ADDR_ICC_REC_04                 ADDR_ICC_REC_START+ICC_REC_LEN*3
#define ADDR_ICC_REC_05                 ADDR_ICC_REC_START+ICC_REC_LEN*4
#define ADDR_ICC_REC_06                 ADDR_ICC_REC_START+ICC_REC_LEN*5
#define ADDR_ICC_REC_07                 ADDR_ICC_REC_START+ICC_REC_LEN*6
#define ADDR_ICC_REC_08                 ADDR_ICC_REC_START+ICC_REC_LEN*7
#define ADDR_ICC_REC_09                 ADDR_ICC_REC_START+ICC_REC_LEN*8
#define ADDR_ICC_REC_10                 ADDR_ICC_REC_START+ICC_REC_LEN*9

#define ADDR_ICC_REC_11                 ADDR_ICC_REC_START+ICC_REC_LEN*10
#define ADDR_ICC_REC_12                 ADDR_ICC_REC_START+ICC_REC_LEN*11
#define ADDR_ICC_REC_13                 ADDR_ICC_REC_START+ICC_REC_LEN*12
#define ADDR_ICC_REC_14                 ADDR_ICC_REC_START+ICC_REC_LEN*13
#define ADDR_ICC_REC_15                 ADDR_ICC_REC_START+ICC_REC_LEN*14
#define ADDR_ICC_REC_16                 ADDR_ICC_REC_START+ICC_REC_LEN*15
#define ADDR_ICC_REC_17                 ADDR_ICC_REC_START+ICC_REC_LEN*16
#define ADDR_ICC_REC_18                 ADDR_ICC_REC_START+ICC_REC_LEN*17
#define ADDR_ICC_REC_19                 ADDR_ICC_REC_START+ICC_REC_LEN*18
#define ADDR_ICC_REC_20                 ADDR_ICC_REC_START+ICC_REC_LEN*19

#define ADDR_ICC_REC_21                 ADDR_ICC_REC_START+ICC_REC_LEN*20
#define ADDR_ICC_REC_22                 ADDR_ICC_REC_START+ICC_REC_LEN*21
#define ADDR_ICC_REC_23                 ADDR_ICC_REC_START+ICC_REC_LEN*22
#define ADDR_ICC_REC_24                 ADDR_ICC_REC_START+ICC_REC_LEN*23
#define ADDR_ICC_REC_25                 ADDR_ICC_REC_START+ICC_REC_LEN*24
#define ADDR_ICC_REC_26                 ADDR_ICC_REC_START+ICC_REC_LEN*25
#define ADDR_ICC_REC_27                 ADDR_ICC_REC_START+ICC_REC_LEN*26
#define ADDR_ICC_REC_28                 ADDR_ICC_REC_START+ICC_REC_LEN*27
#define ADDR_ICC_REC_29                 ADDR_ICC_REC_START+ICC_REC_LEN*28
#define ADDR_ICC_REC_30                 ADDR_ICC_REC_START+ICC_REC_LEN*29

#define ADDR_ICC_REC_END                ADDR_ICC_REC_START+ICC_REC_LEN*MAX_ICC_REC_CNT

//----------------------------------------------------------------------------
//      RSA SAM DATA STRUCTURE DEFINITIONS
//----------------------------------------------------------------------------
//#define MAX_KEY_SLOT_CNT        9                       // total number of key slots
#define MAX_KEY_SLOT_NUM        18-1                      // max key slot number
#define MAX_SAM_BUF_LEN         248                     // max RSA SAM Tx/Rx buffer size

#define KEY_FID_00              0x8000                  // file id for key 00 (WORKING)
#define KEY_FID_01              0x8001                  // file id for key 01 (SLOT# 01)
#define KEY_FID_02              0x8002                  //
#define KEY_FID_03              0x8003                  //
#define KEY_FID_04              0x8004                  //
#define KEY_FID_05              0x8005                  //
#define KEY_FID_06              0x8006                  //
#define KEY_FID_07              0x8007                  //
#define KEY_FID_08              0x8008                  //
#define KEY_FID_09              0x8009                  //
#define KEY_FID_10              0x800A                  //
                                                        //
#define KEY_FID_20              0x8014                  //
#define KEY_FID_50              0x8032                  //
#define KEY_FID_WORK            KEY_FID_00              // working key area

#define OFFSET_SAM_RID          0x0000                  // key file structure
#define OFFSET_SAM_PKI          0x0005                  //
#define OFFSET_SAM_EXP_LEN      0x0006                  //
#define OFFSET_SAM_MOD_LEN      0x0007                  //
#define OFFSET_SAM_SHA1         0x0009                  //
#define OFFSET_SAM_EXP          0x001D                  //
#define OFFSET_SAM_MOD          0x0020                  // for fixed length exponent=3 bytes
#define OFFSET_SAM_MOD1         0x001E                  // for variable length exponent=2 or 3 (1 byte)
#define OFFSET_SAM_MOD3         0x0020                  // for variable length exponent=2^16+1 (3 bytes)

//****************************************************************************
//      Data Element Addressing - KEY MANAGEMENT
//                                Location: SRAM page 14
//                                Size    : 32KB
//                                Range   : 0x0000 - 0x7FFF
//                                Format  : LEN1 + VALUE
//****************************************************************************

//----------------------------------------------------------------------------
//      CA PUBLIC KEY STRUCTURE
//
//      RID                       - 5   bytes
//      INDEX                     - 1   bytes
//      EXPONENT LENGTH           - 1   byte  (exponent length in BYTES)
//      MODULUS LENGTH            - 2   byte  (modulus length in BYTES)
//      SHA-1                     - 20  bytes (hash(RID+INDEX+MODULUS+EXPONENT))
//      EXPONENT                  - 3   bytes (2, 3, or 2^16+1)
//      MODULUS                   - 256 bytes (768, 896, 1024, 1152, or 2048 bits)
//      HASH ALGORITHM INDICATOR  - 1   byte
//      PK ALGORITHM INDICATOR    - 1   byte  (PK: public key)
//      RFU                       - 10  bytes
//
//      Total: 300 bytes
//----------------------------------------------------------------------------
//#define RSA_SAM                         SAM6            // g_dhn_sam
//
#define MAX_CA_PK_CNT                   18-1
#define CA_PK_LEN                       300
#define CA_PK_HEADER_LEN                29

#define ADDR_CA_PK_01                   0x0000
#define ADDR_CA_PK_RID                  0x0000
#define ADDR_CA_PK_INDEX                ADDR_CA_PK_RID+5
#define ADDR_CA_PK_EXP_LEN              ADDR_CA_PK_INDEX+1
#define ADDR_CA_PK_MOD_LEN              ADDR_CA_PK_EXP_LEN+1
#define ADDR_CA_PK_SHA1                 ADDR_CA_PK_MOD_LEN+2
#define ADDR_CA_PK_EXPONENT             ADDR_CA_PK_SHA1+20
#define ADDR_CA_PK_MODULUS              ADDR_CA_PK_EXPONENT+3
#define ADDR_CA_HASH_AI                 ADDR_CA_PK_MODULUS+256
#define ADDR_CA_PK_AI                   ADDR_CA_HASH_AI_01+1
#define ADDR_CA_PK_RFU                  ADDR_CA_PK_AI+1

//#define ADDR_CA_PK_END                  ADDR_CA_PK_01+CA_PK_LEN*MAX_CA_PK_CNT

//----------------------------------------------------------------------------
//      Revocation List of the Issuer Public Key Certificate (signed by CAPK)
//
//      FORMAT: RID[5] + CAPKI[1] + CERTIFICATE_SN[3]
//      NOTE  : If RID = 00 00 00 00 00 -> end of list
//----------------------------------------------------------------------------
#define MAX_CAPK_REVOC_CNT              18-1
#define CAPK_REVOC_LEN                  9

#define ADDR_CAPK_REVOCATION_LIST_01    ADDR_ICC_REC_END
#define ADDR_CAPK_REVOCATION_LIST_02    ADDR_CAPK_REVOCATION_LIST_01+CAPK_REVOC_LEN*1
#define ADDR_CAPK_REVOCATION_LIST_50    ADDR_CAPK_REVOCATION_LIST_01+CAPK_REVOC_LEN*(MAX_CAPK_REVOC_CNT-1)

#define ADDR_CAPK_REVOCATION_LIST_END   ADDR_CAPK_REVOCATION_LIST_01+CAPK_REVOC_LEN*MAX_CAPK_REVOC_CNT

//****************************************************************************
//      Data Element Addressing - WORKING BUFFER
//                                Location: SRAM page 15
//                                Size    : 16KB
//                                Range   : 0x0000 - 0x3FFF
//****************************************************************************
//#define ADDR_DIR_STACK                  0x0000          	// 8KB
//#define ADDR_DIR_STACK_END              0x1FFF

//#define PRINTER_QUE_LEN		  0x2000		// 8KB
//#define ADDR_PRINTER_QUE                0x2000
//#define ADDR_PRINTER_QUE_END            0x3FFF

//----------------------------------------------------------------------------
#endif
