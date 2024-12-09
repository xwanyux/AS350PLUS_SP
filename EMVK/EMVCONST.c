//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : EMVCONST.C                                                 **
//**  MODULE   : Declaration of related message string.                     **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2008/11/27                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2002-2008 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"
#include "EMVAPI.h"

//----------------------------------------------------------------------------
//      Terminal Related Data Element
//
//      Format: TAG LEN ATTRIBUTE
//      NOTE  : MAX_TDE_CNT defined in EMVAPI.h
//              LEN=0, means 256 bytes.
//----------------------------------------------------------------------------
//                            Tag        Attr    Len
//                            ---------  -----   ----
const UCHAR TDE_Tags_Table[]= {
                              0x9f,0x01, DEF_N,  0x06,      // TERM_ACQ_ID
                              0x9f,0x40, DEF_B,  0x05,      // TERM_ADD_CAP
                              0x00,0x81, DEF_B,  0x04,      // TERM_AMT_AUTH_B
                              0x9f,0x02, DEF_N,  0x06,      // TERM_AMT_AUTH_N
                              0x9f,0x04, DEF_B,  0x04,      // TERM_AMT_OTHER_B
                              0x9f,0x03, DEF_N,  0x06,      // TERM_AMT_OTHER_N
                              0x9f,0x3a, DEF_B,  0x04,      // TERM_AMT_RC
                              0x9f,0x06, DEF_B,  0x10,      // TERM_AID
                              0x9f,0x09, DEF_B,  0x02,      // TERM_AVN
                              0x00,0x8a, DEF_AN, 0x02,      // TERM_ARC
                              0x9f,0x34, DEF_B,  0x03,      // TERM_CVMR
                              0x9f,0x22, DEF_B,  0x01,      // TERM_CA_PK_INDEX
                              0x9f,0x1e, DEF_AN, 0x08,      // TERM_IFD_SN
                              0x9f,0x15, DEF_N,  0x02,      // TERM_MCC
                              0x9f,0x16, DEF_ANS,0x0f,      // TERM_MID
                              0x9f,0x39, DEF_N,  0x01,      // TERM_PEM
                              0x9f,0x33, DEF_B,  0x03,      // TERM_CAP
                              0x9f,0x1a, DEF_N,  0x02,      // TERM_CNTR_CODE
                              0x9f,0x1b, DEF_B,  0x04,      // TERM_FL
                              0x9f,0x1c, DEF_AN, 0x08,      // TERM_TID
                              0x9f,0x1d, DEF_B,  0x08,      // TERM_TRM_DATA
                              0x9f,0x35, DEF_N,  0x01,      // TERM_TYPE
                              0x00,0x95, DEF_B,  0x05,      // TERM_TVR
                              0x00,0x98, DEF_B,  0x14,      // TERM_TC_SHA1
                              0x5f,0x2a, DEF_N,  0x02,      // TERM_TX_CC
                              0x5f,0x36, DEF_N,  0x01,      // TERM_TX_CE
                              0x00,0x9a, DEF_N,  0x03,      // TERM_TX_DATE
                              0x9f,0x3c, DEF_N,  0x02,      // TERM_TX_RCC
                              0x9f,0x3d, DEF_N,  0x01,      // TERM_TX_RCE
                              0x9f,0x41, DEF_N,  0x04,      // TERM_TX_SC
                              0x00,0x9b, DEF_B,  0x02,      // TERM_TSI
                              0x9f,0x21, DEF_N,  0x03,      // TERM_TX_TIME
                              0x00,0x9c, DEF_N,  0x01,      // TERM_TX_TYPE
                              0x9f,0x37, DEF_B,  0x04,      // TERM_UPD_NBR
                              0x00,0x89, DEF_AN, 0x06,      // ISU_AUTH_CODE
                              0x00,0x8a, DEF_AN, 0x02,      // ISU_ARC
                              0x00,0x91, DEF_B,  0x10,      // ISU_AUTH_DATA
                              0x9f,0x53, DEF_AN, 0x01       // TERM_TCC
                              };

const UINT TDE_Addr_Table[]=  { // absolute address table
                              ADDR_TERM_ACQ_ID       ,      // TERM_ACQ_ID
                              ADDR_TERM_ADD_CAP      ,      // TERM_ADD_CAP
                              ADDR_TERM_AMT_AUTH_B   ,      // TERM_AMT_AUTH_B
                              ADDR_TERM_AMT_AUTH_N   ,      // TERM_AMT_AUTH_N
                              ADDR_TERM_AMT_OTHER_B  ,      // TERM_AMT_OTHER_B
                              ADDR_TERM_AMT_OTHER_N  ,      // TERM_AMT_OTHER_N
                              ADDR_TERM_AMT_RC       ,      // TERM_AMT_RC
                              ADDR_TERM_AID          ,      // TERM_AID
                              ADDR_TERM_AVN          ,      // TERM_AVN
                              ADDR_TERM_ARC          ,      // TERM_ARC
                              ADDR_TERM_CVMR         ,      // TERM_CVMR
                              ADDR_TERM_CA_PK_INDEX  ,      // TERM_CA_PK_INDEX
                              ADDR_TERM_IFD_SN       ,      // TERM_IFD_SN
                              ADDR_TERM_MCC          ,      // TERM_MCC
                              ADDR_TERM_MID          ,      // TERM_MID
                              ADDR_TERM_PEM          ,      // TERM_PEM
                              ADDR_TERM_CAP          ,      // TERM_CAP
                              ADDR_TERM_CNTR_CODE    ,      // TERM_CNTR_CODE
                              ADDR_TERM_FL           ,      // TERM_FL
                              ADDR_TERM_TID          ,      // TERM_TID
                              ADDR_TERM_TRM_DATA     ,      // TERM_TRM_DATA
                              ADDR_TERM_TYPE         ,      // TERM_TYPE
                              ADDR_TERM_TVR          ,      // TERM_TVR
                              ADDR_TERM_TC_SHA1      ,      // TERM_TC_SHA1
                              ADDR_TERM_TX_CC        ,      // TERM_TX_CC
                              ADDR_TERM_TX_CE        ,      // TERM_TX_CE
                              ADDR_TERM_TX_DATE      ,      // TERM_TX_DATE
                              ADDR_TERM_TX_RCC       ,      // TERM_TX_RCC
                              ADDR_TERM_TX_RCE       ,      // TERM_TX_RCE
                              ADDR_TERM_TX_SC        ,      // TERM_TX_SC
                              ADDR_TERM_TSI          ,      // TERM_TSI
                              ADDR_TERM_TX_TIME      ,      // TERM_TX_TIME
                              ADDR_TERM_TX_TYPE      ,      // TERM_TX_TYPE
                              ADDR_TERM_UPD_NBR      ,      // TERM_UPD_NBR
                              ADDR_ISU_AUTH_CODE     ,      // ISU_AUTH_CODE
                              ADDR_ISU_ARC           ,      // ISU_ARC
                              ADDR_ISU_AUTH_DATA     ,      // ISU_AUTH_DATA
                              ADDR_TERM_TCC                 // TERM_TCC
                              };

//----------------------------------------------------------------------------
//      ICC Related Data Element
//
//      Format: TAG LEN ATTRIBUTE
//      NOTE  : MAX_TDE_CNT defined in EMVAPI.h
//              LEN=0, means 256 bytes.
//----------------------------------------------------------------------------
//                            Tag        Attr    Len
//                            ---------  -----   ----
const UCHAR IDE_Tags_Table[]= {
                              0x9f,0x42, DEF_N,  0x02,      // ICC_AP_CC
                              0x9f,0x44, DEF_N,  0x01,      // ICC_AP_CE
                              0x9f,0x05, DEF_B,  0x20,      // ICC_AP_DD
                              0x5f,0x25, DEF_N,  0x03,      // ICC_AP_EFFECT_DATE
                              0x5f,0x24, DEF_N,  0x03,      // ICC_AP_EXPIRE_DATE
                              0x00,0x5a, DEF_CN, 0x0a,      // ICC_AP_PAN
                              0x5f,0x34, DEF_N,  0x01,      // ICC_AP_PAN_SN
                              0x9f,0x3b, DEF_N,  0x08,      // ICC_AP_RC
                              0x9f,0x43, DEF_N,  0x04,      // ICC_AP_RCE
                              0x9f,0x36, DEF_B,  0x02,      // ICC_ATC
                              0x9f,0x07, DEF_B,  0x02,      // ICC_AUC
                              0x9f,0x08, DEF_B,  0x02,      // ICC_AVN
                              0x00,0x8c, DEF_B,  0xfc,      // ICC_CDOL1
                              0x00,0x8d, DEF_B,  0xfc,      // ICC_CDOL2
                              0x5f,0x20, DEF_ANS,0x1a,      // ICC_CH_NAME
                              0x9f,0x0b, DEF_ANS,0x2d,      // ICC_CH_NAME_EX
                              0x00,0x8e, DEF_B,  0xfc,      // ICC_CVM_LIST
                              0x00,0x8f, DEF_B,  0x01,      // ICC_CA_PK_INDEX
                              0x00,0x52, DEF_B,  0xef,      // ICC_CMD_PERFORM
                              0x9f,0x27, DEF_B,  0x01,      // ICC_CID
                              0x9f,0x45, DEF_B,  0x02,      // ICC_DAC
                              0x9f,0x49, DEF_B,  0xfc,      // ICC_DDOL
                              0x9f,0x4c, DEF_B,  0x08,      // ICC_DYNAMIC_NBR
                              0x9f,0x2d, DEF_B,  0x00,      // ICC_PIN_EPKC
                              0x9f,0x2e, DEF_B,  0x03,      // ICC_PIN_EPKE
                              0x9f,0x2f, DEF_B,  0x00,      // ICC_PIN_EPKR
                              0x9f,0x46, DEF_B,  0x00,      // ICC_PKC
                              0x9f,0x47, DEF_B,  0x03,      // ICC_PKE
                              0x9f,0x48, DEF_B,  0x00,      // ICC_PKR
                              0x9f,0x0d, DEF_B,  0x05,      // ICC_IAC_DEFAULT
                              0x9f,0x0e, DEF_B,  0x05,      // ICC_IAC_DENIAL
                              0x9f,0x0f, DEF_B,  0x05,      // ICC_IAC_ONLINE
                              0x9f,0x10, DEF_B,  0x20,      // ICC_ISU_AP_DATA
                              0x9f,0x11, DEF_N,  0x01,      // ICC_ISU_CTI
                              0x5f,0x28, DEF_N,  0x02,      // ICC_ISU_CNTR_CODE
                              0x00,0x90, DEF_B,  0x00,      // ICC_ISU_PKC
                              0x9f,0x32, DEF_B,  0x03,      // ICC_ISU_PKE
                              0x00,0x92, DEF_B,  0x00,      // ICC_ISU_PKR
                              0x5f,0x2d, DEF_AN, 0x08,      // ICC_LANG_PREFER
                              0x9f,0x13, DEF_B,  0x02,      // ICC_ATC_LAST_ONLINE
                              0x9f,0x14, DEF_B,  0x01,      // ICC_LCOL
                              0x9f,0x17, DEF_ANS,0x0f,      // ICC_PIN_TRY_CNT
                              0x5f,0x30, DEF_N,  0x02,      // ICC_SERVICE_CODE
                              0x9f,0x4b, DEF_B,  0x00,      // ICC_SIGNED_DAD
                              0x00,0x93, DEF_B,  0x00,      // ICC_SIGNED_SAD
                              0x9f,0x4a, DEF_B,  0x00,      // ICC_STATIC_DATL
                              0x9f,0x1f, DEF_ANS,0x00,      // ICC_TRK1_DISD
                              0x9f,0x20, DEF_CN, 0x00,      // ICC_TRK2_DISD
                              0x00,0x57, DEF_B,  0x13,      // ICC_TRK2_EQUD
                              0x00,0x97, DEF_B,  0xfc,      // ICC_TDOL
                              0x9f,0x23, DEF_B,  0x01,      // ICC_UCOL
                              0x9f,0x26, DEF_B,  0x08,      // ICC_AC
                              0x00,0x4f, DEF_B,  0x10,      // ICC_AID
                              0x00,0x84, DEF_B,  0x10,      // ICC_DFNAME
                              0x00,0x9d, DEF_B,  0x10,      // ICC_DDFNAME
                              0x00,0x50, DEF_ANS,0x10,      // ICC_AP_LABEL
                              0x9f,0x12, DEF_ANS,0x10,      // ICC_AP_PREFER_NAME
                              0x00,0x87, DEF_B,  0x01,      // ICC_AP_PI
                              0xbf,0x0c, DEF_B,  0xde,      // ICC_FCI_IDD
                              0x9f,0x38, DEF_B,  0x00,      // ICC_PDOL
                              0x00,0x82, DEF_B,  0x02,      // ICC_AIP2
                              0x00,0x94, DEF_B,  0xfc,      // ICC_AFL2

                              0x00,0x42, DEF_N,  0x03,      // ICC_IIN
                              0x5f,0x50, DEF_ANS,0xf8,      // ICC_ISU_URL
                              0x5f,0x53, DEF_B,  0x22,      // ICC_ISBN
                              0x5f,0x54, DEF_B,  0x0b,      // ICC_BIC
                              0x5f,0x55, DEF_A,  0x02,      // ICC_ISU_CNTR_CODE2
                              0x5f,0x56, DEF_A,  0x03,      // ICC_ISU_CNTR_CODE3
                              0x9f,0x4d, DEF_B,  0x02,      // ICC_LOG_ENTRY
                              0x9f,0x4f, DEF_B,  0xf8,      // ICC_LOG_FORMAT

                              0x9f,0x61, DEF_AN, 0x28,      // ICC_CH_LICENSE_ID    PBOC20
                              0x9f,0x62, DEF_CN, 0x01,      // ICC_CH_LICENSE_TYPE  PBOC20
                              0x9f,0x63, DEF_B,  0x10,	    // ICC_CARD_PRODUCT_ID  PBOC20
                              };

const UINT IDE_Addr_Table[]=  { // absolute address table
                              ADDR_ICC_AP_CC           ,    // ICC_AP_CC
                              ADDR_ICC_AP_CE           ,    // ICC_AP_CE
                              ADDR_ICC_AP_DD           ,    // ICC_AP_DD
                              ADDR_ICC_AP_EFFECT_DATE  ,    // ICC_AP_EFFECT_DATE
                              ADDR_ICC_AP_EXPIRE_DATE  ,    // ICC_AP_EXPIRE_DATE
                              ADDR_ICC_AP_PAN          ,    // ICC_AP_PAN
                              ADDR_ICC_AP_PAN_SN       ,    // ICC_AP_PAN_SN
                              ADDR_ICC_AP_RC           ,    // ICC_AP_RC
                              ADDR_ICC_AP_RCE          ,    // ICC_AP_RCE
                              ADDR_ICC_ATC             ,    // ICC_ATC
                              ADDR_ICC_AUC             ,    // ICC_AUC
                              ADDR_ICC_AVN             ,    // ICC_AVN
                              ADDR_ICC_CDOL1           ,    // ICC_CDOL1
                              ADDR_ICC_CDOL2           ,    // ICC_CDOL2
                              ADDR_ICC_CH_NAME         ,    // ICC_CH_NAME
                              ADDR_ICC_CH_NAME_EX      ,    // ICC_CH_NAME_EX
                              ADDR_ICC_CVM_LIST        ,    // ICC_CVM_LIST
                              ADDR_ICC_CA_PKI          ,    // ICC_CA_PK_INDEX
                              ADDR_ICC_CMD_PERFORM     ,    // ICC_CMD_PERFORM
                              ADDR_ICC_CID             ,    // ICC_CID
                              ADDR_ICC_DAC             ,    // ICC_DAC
                              ADDR_ICC_DDOL            ,    // ICC_DDOL
                              ADDR_ICC_DYNAMIC_NBR     ,    // ICC_DYNAMIC_NBR
                              ADDR_ICC_PIN_EPKC        ,    // ICC_PIN_EPKC
                              ADDR_ICC_PIN_EPKE        ,    // ICC_PIN_EPKE
                              ADDR_ICC_PIN_EPKR        ,    // ICC_PIN_EPKR
                              ADDR_ICC_PKC             ,    // ICC_PKC
                              ADDR_ICC_PKE             ,    // ICC_PKE
                              ADDR_ICC_PKR             ,    // ICC_PKR
                              ADDR_ICC_IAC_DEFAULT     ,    // ICC_IAC_DEFAULT
                              ADDR_ICC_IAC_DENIAL      ,    // ICC_IAC_DENIAL
                              ADDR_ICC_IAC_ONLINE      ,    // ICC_IAC_ONLINE
                              ADDR_ICC_ISU_AP_DATA     ,    // ICC_ISU_AP_DATA
                              ADDR_ICC_ISU_CTI         ,    // ICC_ISU_CTI
                              ADDR_ICC_ISU_CNTR_CODE   ,    // ICC_ISU_CNTR_CODE
                              ADDR_ICC_ISU_PKC         ,    // ICC_ISU_PKC
                              ADDR_ICC_ISU_PKE         ,    // ICC_ISU_PKE
                              ADDR_ICC_ISU_PKR         ,    // ICC_ISU_PKR
                              ADDR_ICC_LANG_PREFER     ,    // ICC_LANG_PREFER
                              ADDR_ICC_ATC_LAST_ONLINE ,    // ICC_ATC_LAST_ONLINE
                              ADDR_ICC_LCOL            ,    // ICC_LCOL
                              ADDR_ICC_PIN_TRY_CNT     ,    // ICC_PIN_TRY_CNT
                              ADDR_ICC_SERVICE_CODE    ,    // ICC_SERVICE_CODE
                              ADDR_ICC_SIGNED_DAD      ,    // ICC_SIGNED_DAD
                              ADDR_ICC_SIGNED_SAD      ,    // ICC_SIGNED_SAD
                              ADDR_ICC_SDA_TL          ,    // ICC_SDA_TL
                              ADDR_ICC_TRK1_DISD       ,    // ICC_TRK1_DISD
                              ADDR_ICC_TRK2_DISD       ,    // ICC_TRK2_DISD
                              ADDR_ICC_TRK2_EQUD       ,    // ICC_TRK2_EQUD
                              ADDR_ICC_TDOL            ,    // ICC_TDOL
                              ADDR_ICC_UCOL            ,    // ICC_UCOL
                              ADDR_ICC_AC              ,    // ICC_AC
                              ADDR_ICC_AID             ,    // ICC_AID
                              ADDR_ICC_DFNAME          ,    // ICC_DFNAME
                              ADDR_ICC_DDFNAME         ,    // ICC_DDFNAME
                              ADDR_ICC_AP_LABEL        ,    // ICC_AP_LABEL
                              ADDR_ICC_AP_PREFER_NAME  ,    // ICC_AP_PREFER_NAME
                              ADDR_ICC_AP_PI           ,    // ICC_AP_PI
                              ADDR_ICC_FCI_IDD         ,    // ICC_FCI_IDD
                              ADDR_ICC_PDOL            ,    // ICC_PDOL
                              ADDR_ICC_AIP2            ,    // ICC_AIP2
                              ADDR_ICC_AFL2            ,    // ICC_AFL2

                              ADDR_ICC_IIN             ,    // ICC_IIN
                              ADDR_ICC_ISU_URL         ,    // ICC_ISU_URL
                              ADDR_ICC_IBAN            ,    // ICC_ISBN
                              ADDR_ICC_BIC             ,    // ICC_BIC
                              ADDR_ICC_ISU_CNTR_CODE2  ,    // ICC_ISU_CNTR_CODE2
                              ADDR_ICC_ISU_CNTR_CODE3  ,    // ICC_ISU_CNTR_CODE3
                              ADDR_ICC_LOG_ENTRY       ,    // ICC_LOG_ENTRY
                              ADDR_ICC_LOG_FORMAT      ,    // ICC_LOG_FORMAT

                              ADDR_ICC_CH_LICENSE_ID   ,    // ICC_CH_LICENSE_ID
                              ADDR_ICC_CH_LICENSE_TYPE ,    // ICC_CH_LICENSE_TYPE
                              ADDR_ICC_CARD_PRODUCT_ID ,    // ICC_CARD_PRODUCT_ID
                              };
