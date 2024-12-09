//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : EMVDC.h                                                    **
//**  MODULE   : Global data definition.                                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/02                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _EMVDC_H_
#define _EMVDC_H_

//----------------------------------------------------------------------------

//#define L2_SW_DEBUG                     1       // mask it for REAL ICC TASK
//#define USE_RSA_SAM		          1	  // open it for using RSA SAM
//#define PLATFORM_16BIT                  1       // open it to support 16bit platform
//#define PRT_ENABLED                     1       // open it to support printer

//#define TRUE                    1
//#define FALSE                   0
//
//#define MAX_DSP_WIDTH                   128
//#define MAX_DSP_CHAR                    20+1
//#define MAX_DSP_FONT0_COL               19+1
//#define MAX_DSP_FONT0_CNT               20+1
//#define MAX_DSP_FONT1_COL               14+1
//#define MAX_DSP_FONT1_CNT               15+1
//#define MAX_DSP_ROW_CNT                 4
//#define MAX_AMT_CNT                     12
//
//#define COL_LEFTMOST                    0       // display column position
//#define COL_MIDWAY                      1
//#define COL_RIGHTMOST                   2
//
//#define KEY_0                           '0'
//#define KEY_1                           '1'
//#define KEY_2                           '2'
//#define KEY_3                           '3'
//#define KEY_4                           '4'
//#define KEY_5                           '5'
//#define KEY_6                           '6'
//#define KEY_7                           '7'
//#define KEY_8                           '8'
//#define KEY_9                           '9'
//#define KEY_F1                          'a'     // VOID
//#define KEY_F2                          'b'     // ADJUST
//#define KEY_F3                          'c'     // SETTLE
//#define KEY_F4                          'd'     // FUNC
//#define KEY_CANCEL                      'x'
//#define KEY_CLEAR                       'n'
//#define KEY_ALPHA                       'z'
//#define KEY_BACKSPACE                   '#'
//#define KEY_OK                          'y'
//
#define AMT_INT_SIZE                    5       // amount, integer part (n10)
#define AMT_DEC_SIZE                    1       // amount, decimal part (n2)
//
//#define NUM_TYPE_DIGIT                  0x01    // pure digit (RFU)
//#define NUM_TYPE_COMMA                  0x02    // insert thousand comma
//#define NUM_TYPE_DEC                    0x04    // insert decimal point
//#define NUM_TYPE_STAR                   0x08    // special prompt ('*')
//#define NUM_TYPE_LEADING_ZERO           0x10    // accept leading '0'

//----------------------------------------------------------------------------
//      Message Type ID (iso 8583)
//----------------------------------------------------------------------------
#define MSGID_FINANCIAL_REQ             0x0200
#define MSGID_FINANCIAL_CONFIRM         0x0202
#define MSGID_FINANCIAL_ADVICE          0x0220  // upload BDC (batch data captured)
#define MSGID_REVERSAL_REQ              0x0400
#define MSGID_SETTLE_REQ                0x0500
#define MSGID_LOAD_CAPK_GOOD            0x0A00

//----------------------------------------------------------------------------
//      Transaction Type (the 1'st two digits of Processing Code)
//----------------------------------------------------------------------------
//      Debits
#define TT_GOODS_AND_SERVICE            0x00
#define TT_FINANCIAL_CONFIRM            0x90
#define TT_FINANCIAL_REVERSAL           0x91
#define TT_FINANCIAL_ADVICE             0x92
#define TT_SETTLEMENT                   0x93
#define TT_LOAD_CAPK                    0x94
#define TT_PRE_AUTH                     0x95
#define TT_BDC                          0x96

#define TT_GOODS                        0x00
#define TT_SERVICES                     0x01

//      Credits

//----------------------------------------------------------------------------
//      Online Processing Result (referenced by EMVDC_OnlineProcessing)
//----------------------------------------------------------------------------
#define ONL_Completed                   0
#define ONL_Failed                      1

//----------------------------------------------------------------------------
//      Referral Processing Result (referenced by EMVDC_ReferralProcessing)
//----------------------------------------------------------------------------
#define REF_Approved                    0
#define REF_Declined                    1
#define REF_ForcedOnline                2
#define REF_ForcedAccept                3
#define REF_ForcedDecline               4
#define REF_ForcedAbort                 5

//----------------------------------------------------------------------------
//      Financial Transaction Request / Response (message block format)
//----------------------------------------------------------------------------
                                                // LEN1 LEN2 P/F DATA[]
#define TX_TID                          0 +3    // an8, terminal identifier
#define TX_DATE                         8 +3    // n6,  transaction date
#define TX_TIME                         11+3    // n6,  transaction time
#define TX_ARQC                         14+3    // b1,  authorization request cryptogram
#define TX_TYPE                         15+3    // n2,  transaction type
#define TX_AMT_AUTH                     16+3    // n12, amount, authorized
#define TX_AMT_OTHER                    22+3    // n12, amount, other
#define TX_AMT                          28+3    // n12, transaction amount
#define TX_CC                           34+3    // n3,  transaction currency code
#define TX_PAN                          36+3    // cn19,application PAN
#define TX_PAN_SN                       46+3    // n2,  application PAN sequence number
#define TX_AIP                          47+3    // b2,  application interchange profile
#define TX_ATC                          49+3    // b2,  application transaction counter
#define TX_TVR                          51+3    // b5,  terminal verification results
#define TX_CNTR_CODE                    56+3    // n3,  terminal country code
#define TX_UPD_NBR                      58+3    // b4,  2L-V, unpredictable number
#define TX_ISU_AP_DATA                  64+3    // b32, 2L-V, issuer application data
#define TX_CID                          98+3    // b1,  cryptogram information data
#define TX_AC                           99+3    // b8,  application cryptogram
#define TX_PEM                          107+3   // n2,  POS entry mode
#define TX_REV_ISR                      108+3   // b82, 2L+ISR[5*16] for reversal only
//#define TX_EPIN                         190+3   // b10, 2L+EPIN[8]
//#define TX_KSN                          200+3   // b12, 2L+KSN[10]

#define TX_ISR                          16+3    // n,   CNT-V[5*16]

                                                // LEN1 LEN2 P/F DATA[]
#define RX_PF                           0 +2    // poll or final flag
#define RX_TID                          0 +3    // an8, terminal identifier
#define RX_DATE                         8 +3    // n6,  transaction date
#define RX_TIME                         11+3    // n6,  transaction time
#define RX_ARC                          14+3    // an2, authorization response code
#define RX_AUTH_CODE                    16+3    // an6, 2L-V, authorization code
#define RX_IAD                          24+3    // b16, 2L-V, issuer authen data
#define RX_IST                          42+3    // var, 2L-V, issuer script template

//----------------------------------------------------------------------------
//      Function Table
//----------------------------------------------------------------------------
#define MAX_EMVDC_FUNC_CNT              20    // cf. EMVDC_Func_Table[cnt]
#define EMVDC_FUNC_LEN                  18      //     defined in XCONSTP1.C
#define EMVDC_ITEM_LEN                  17      //     defined in XCONSTP1.C

#define FN_SETUP_CONFIG                 0x00
#define FN_SET_DATE_TIME                0x01
#define	FN_SET_ISO_FORMAT		        0x02
#define	FN_SET_KEY_INDEX		        0x03
#define FN_SET_MAC_KEY_INDEX            0X04
#define FN_SET_AID                      0x05
#define FN_SET_CONTRY_CODE              0x06
#define FN_SET_FLOOR_LIMIT              0x07
#define FN_SET_BRS_SHRESHOLD            0x08
#define FN_SET_BRS_MTP                  0x09
#define FN_SET_RS_TP                    0x0A
#define FN_SET_TAC                      0x0B
#define FN_TRANS_LOG                    0x0C
#define FN_SETTLE                       0x0D
#define FN_LOAD_CAPK                    0x0E
#define FN_LOAD_PED_IKEY                0x0F
#define FN_KERNEL_CHECKSUM              0x10
#define FN_SET_DDOL                     0x11
#define FN_SET_TDOL                     0x12
#define FN_GET_CAPK_INDEX               0x13

                                        // -----------------------------------
#define MAX_TAC_FUNC_CNT                3       // cf. TAC_Func_Table[cnt]
#define TAC_FUNC_LEN                    18      //     defined in XCONSTP1.C
#define TAC_ITEM_LEN                    17      //     defined in XCONSTP1.C

#define FN_TAC_DENIAL                   0x00
#define FN_TAC_ONLINE                   0x01
#define FN_TAC_DEFAULT                  0x02

//----------------------------------------------------------------------------
//      SRAM Page Allocation
//----------------------------------------------------------------------------
#define SRAM_PAGE_PARA                  0
#define SRAM_PAGE_LOG                   1

//****************************************************************************
//      Transaction Log         - TERMINAL
//                                Location: SRAM page 01
//                                Size    : 32KB
//                                Range   : 0x0000 - 0x7FFF
//
//      Field Name                Data Format     Length
//      ------------------------  -----------     --------
//      Transaction Seq Counter   n8              4
//      App PAN                   cn ~19          10
//      App PAN Sequence Number   n2              1
//      Transaction Amount        n10+n2          5+1
//      Transaction Date          n6 (YYMMDD)     3
//      Transaction Time          n6 (HHMMSS)     3
//      Authorization Code        an6             6
//      ARC                       an2             2
//      Message Type              an2             2
//
//      AIP                       b               2
//      ATC                       b               2
//      AUC                       b               2
//      ARQC                      b               1
//      CID                       b               1
//      CVM List                  b               2+20
//      CVMR                      b               3
//      IAC-DEN                   b               5
//      IAC-ONL                   b               5
//      IAC-DEF                   b               5
//      Issuer App Data           b               2+32
//      TVR                       b               5
//      Unpredictable Nbr         b               4
//      Amount, Authorized        n12             6
//      Amount, Other             n12             6
//      App Effective Date        n6              3
//      App Expiration Date       n6              3
//      Issuer Country Code       n3              2
//      POS Entry Mode            n2              1
//      Trans Currency Code       n3              2
//      Trans Type                n2              1
//      ISR                       b               1+5*4
//
//      IFD Serial Number         an8             8       (RFU)
//      TERM CAP                  b               3       (RFU)
//      TERM Type                 n2              1       (RFU)
//      Acquirer ID               n6-11           6       (RFU)
//      Merchant Category Code    n4              2       (RFU)
//      Merchant ID               ans15           15      (RFU)
//      Terminal Country Code     n3              2       (RFU)
//      Terminal ID               an8             8       (RFU)
//
//      RFU                                       38
//      ------------------------  ------------    --------
//      TOTAL                                     256 bytes
//****************************************************************************
#define LOG_TX_SC                       0
#define LOG_AP_PAN                      4
#define LOG_AP_PAN_SN                   14
#define LOG_TX_AMT                      15
#define LOG_TX_DATE                     21
#define LOG_TX_TIME                     24
#define LOG_AUTH_CODE                   27
#define LOG_ARC                         33
#define LOG_MSG_TYPE                    35
#define LOG_AIP                         37
#define LOG_ATC                         39
#define LOG_AUC                         41
#define LOG_ARQC                        43
#define LOG_CID                         44
#define LOG_CVML                        45
#define LOG_CVMR                        67
#define LOG_IAC_DEN                     70
#define LOG_IAC_ONL                     75
#define LOG_IAC_DEF                     80
#define LOG_ISU_AP_DATA                 85
#define LOG_TVR                         119
#define LOG_UPD_NBR                     124
#define LOG_AMT_AUTH                    128
#define LOG_AMT_OTHER                   134
#define LOG_AP_EFFECT_DATE              140
#define LOG_AP_EXPIRE_DATE              143
#define LOG_ISU_CNTR_CODE               146
#define LOG_PEM                         148
#define LOG_TX_CC                       149
#define LOG_TX_TYPE                     151
#define LOG_ISR                         152
#define LOG_AC                          173
#define LOG_TSI                         181

//#define LOG_IFD_SN                      70
//#define LOG_TERM_CAP                    127
//#define LOG_TERM_TYPE                   130
//#define LOG_ACQ_ID                      140
//#define LOG_MCC                         166
//#define LOG_MID                         168
//#define LOG_TERM_CNTR_CODE              184
//#define LOG_TID                         186

#define MAX_TX_LOG_CNT                  128     // 128*256=32KB
#define TX_LOG_LEN                      255+1

#define TX_LOG_001                      0x0000
#define TX_LOG_128                      TX_LOG_001+TX_LOG_LEN*(MAX_TX_LOG_CNT-1)

#define TX_LOG_END                      TX_LOG_001+TX_LOG_LEN*MAX_TX_LOG_CNT

//----------------------------------------------------------------------------
//      EMV D/C Functions Prototypes
//----------------------------------------------------------------------------
//      EMVDCDSP
//extern  void SETUP_config_00( void );

//      EMVDC_01
extern  UCHAR EMVDC_startup( UCHAR *atr );
extern  void  EMVDC_close_session( UCHAR wait );
extern  UCHAR EMVDC_RetrievePublicKeyCA( void );
extern  void  EMVDC_SetupPosEntryMode( UCHAR mode );

//      EMVDC_02
extern  UCHAR EMVDC_select_application( UCHAR occurrence, UCHAR *candid );

//      EMVDC_03
extern  UCHAR EMVDC_initiate_application( void );

//      EMVDC_04
extern  UCHAR EMVDC_read_application_data( void );

//      EMVDC_05
extern  UCHAR EMVDC_offline_data_authen( void );

//      EMVDC_06
extern  UCHAR EMVDC_processing_restrictions( void );

//      EMVDC_07
extern  UCHAR EMVDC_cardholder_verification( void );

//      EMVDC_08
extern  UCHAR EMVDC_terminal_risk_management( void );

//      EMVDC_09
extern  UCHAR EMVDC_terminal_action_analysis( void );

//      EMVDC_10
extern  UCHAR EMVDC_OnlineProcessing( UINT msgid, UCHAR ct, UCHAR *arc, UCHAR *iad, UINT *ist );
extern  UCHAR EMVDC_OnlineReconciliation( UINT msgid, UCHAR *log );
extern  UCHAR EMVDC_ReferralProcessing( UCHAR *authcode );
extern  UCHAR EMVDC_PrintReceipt( UCHAR condition, UCHAR *arc );
extern  UCHAR EMVDC_close_transaction( void );
extern  UCHAR EMVDC_MsrProcessing( UCHAR mode, UCHAR *status );
extern  UCHAR EMVDC_FallBackTransaction( void );
extern  UCHAR EMVDC_LoadCaPublicKey( void );
extern  void  EMVDC_PrintLogDetails( void );
extern  UCHAR EMVDC_UploadLastBDC( void );

//----------------------------------------------------------------------------
#endif
