#ifndef _PEDS_API_H_
#define _PEDS_API_H_
//----------------------------------------------------------------------------

#define TRUE                    1
#define FALSE                   0

#define PED_DEV_INT             0               // internal pinpad device
#define PED_DEV_EXT             1               // external pinpad device
#define PED_DEV_EXT_DONGLE      2               // external pinpad device (dongle port)
#define PED_DEV_EXT_IPC         3               // external pinpad device (IPC    port)

#define	DES_MODE_ECB			0x00	// electronic code book
#define	DES_MODE_CBC			0x01	// cipher bolck chaining
#define	DES_MODE_ENCRYPT		0x00	// to encrypt
#define	DES_MODE_DECRYPT		0x80	// to decrypt

// PED Response Code (external)
#define PED_RC_OK                       0x00    //
#define PED_RC_FAILED                   0x01    //

// PED Command (external)
#define PED_CMD_SET_MKEY                '0'
#define PED_CMD_SET_SKEY                '1'
#define PED_CMD_RESET                   '2'
#define PED_CMD_GET_PIN                 '3'
#define PED_CMD_MSKEY_GEN_PIN_BLOCK     '4'
#define PED_CMD_MSKEY_GEN_MAC           '5'
#define PED_CMD_GET_STATUS              '6'
#define PED_CMD_OPEN                    '7'
#define PED_CMD_CLOSE                   '8'
#define PED_CMD_DEL_MKEY                '9'     // delete master key
#define PED_CMD_DEL_SKEY                'A'     // delete session key
#define	PED_CMD_SELECT_MKEY		'B'	// select MKEY slot
#define	PED_CMD_SHOW_BALANCE		'C'	// CUP
#define	PED_CMD_MSKEY_TDES		'D'	// FISC

#define PED_CMD_EXCH_KEK                0x80    // exchange key encryption key
#define PED_CMD_GET_EPIN                0x81    // get encrypted PIN data, EPIN=TDES(PIN,KEK)

// Transaction Type
#define TT_SALE                         '0'
#define TT_VOID                         '1'
#define TT_REFUND                       '2'
#define TT_PREAUTH                      '3'
#define TT_PREAUTH_VOID                 '4'
#define TT_PREAUTH_COMP                 '5'
#define TT_PREAUTH_COMP_CANCEL          '6'

// Internal PINPED State
#define PED_STATE_START                 0
#define PED_STATE_WAIT_KEY              1
#define PED_STATE_PIN_READY             2
#define PED_STATE_END                   3

//#define auxBYPASS_DONGLE                7
//#define auxBYPASS_IPC                   10

//----------------------------------------------------------------------------
//      RSA SAM DATA STRUCTURE DEFINITIONS
//----------------------------------------------------------------------------
//#define KEY_FID_TMK             0x8021                  // PINPAD TMK (Terminal Master Key)
//
//#define OFFSET_SAM_RID          0x0000                  // key file structure
//#define OFFSET_SAM_PKI          0x0005                  //
//#define OFFSET_SAM_EXP_LEN      0x0006                  //
//#define OFFSET_SAM_MOD_LEN      0x0007                  //
//#define OFFSET_SAM_SHA1         0x0009                  //
//#define OFFSET_SAM_EXP          0x001D                  //
//#define OFFSET_SAM_MOD          0x0020                  // for fixed length exponent=3 bytes
//#define OFFSET_SAM_MOD1         0x001E                  // for variable length exponent=2 or 3 (1 byte)
//#define OFFSET_SAM_MOD3         0x0020                  // for variable length exponent=2^16+1 (3 bytes)

//****************************************************************************
//      Data Element Addressing - KEY MANAGEMENT
//                                Location: SRAM page 14
//                                Size    : 32KB
//                                Range   : 0x0000 - 0x7FFF
//                                          0x0000 - 0x77FF (30KB, EMV KEYS)
//                                          0x7800 - 0x7FFF (02KB, PINPAD  )
//****************************************************************************
//#define SRAM_PAGE_KEY                   0x0E
//#define FLASH_PAGE_KEY                  0x8C

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

//----------------------------------------------------------------------------
//      Revocation List of the Issuer Public Key Certificate (signed by CAPK)
//
//      FORMAT: RID[5] + CAPKI[1] + CERTIFICATE_SN[3]
//      NOTE  : If RID = 00 00 00 00 00 -> end of list
//----------------------------------------------------------------------------
//#define MAX_CAPK_REVOC_CNT              50
//#define CAPK_REVOC_LEN                  9
//
//#define ADDR_CAPK_REVOCATION_LIST_01    ADDR_CA_PK_END
//#define ADDR_CAPK_REVOCATION_LIST_02    ADDR_CAPK_REVOCATION_LIST_01+CAPK_REVOC_LEN*1
//#define ADDR_CAPK_REVOCATION_LIST_50    ADDR_CAPK_REVOCATION_LIST_01+CAPK_REVOC_LEN*(MAX_CAPK_REVOC_CNT-1)
//
//#define ADDR_CAPK_REVOCATION_LIST_END   ADDR_CAPK_REVOCATION_LIST_01+CAPK_REVOC_LEN*MAX_CAPK_REVOC_CNT

//----------------------------------------------------------------------------
//      VisaWave
//      IMEK & IAEK         (2*16 bytes)
//
//      FORMAT: L-V
//----------------------------------------------------------------------------
//#define WAVE_KEY_LEN                    16
//
//#define ADDR_WAVE_IMEK                  ADDR_CAPK_REVOCATION_LIST_END
//#define ADDR_WAVE_IAEK                  ADDR_WAVE_IMEK+WAVE_KEY_LEN+1 // L-V[16]

//----------------------------------------------------------------------------
//      PINPAD
//----------------------------------------------------------------------------
//#define PED_PIN_LEN                     16              // actual 4..12 digits
//#define PED_PIN_SLOT_LEN                17              //
//#define ADDR_PED_PIN                    0x7800          // b17, L-V
//#define ADDR_PED_PIN_END                ADDR_PED_PIN+PED_PIN_SLOT_LEN

//----------------------------------------------------------------------------
//      Master-Session Keys (TDES)
//      Master Key Slot# : 10
//      Working Key Slot#: 10 (under one of the 10 master keys)
//----------------------------------------------------------------------------
//#define MAX_MKEY_CNT                    10                      //
//#define MAX_SKEY_CNT                    10                      //
//#define PED_MSKEY_LEN                   16                      //
//#define PED_MSKEY_SLOT_LEN              17                      // L(1)+V(16)
//#define PED_MSKEY_SLOT_LEN2             20                      // L(1)+V(16)+KCV(3)
//
//#define ADDR_PED_MKEY_INDEX             ADDR_PED_PIN_END                        // current master key index  (0..9)
//#define ADDR_PED_SKEY_INDEX             ADDR_PED_MKEY_INDEX+1                   // current session key index (0..9)
//
//#define ADDR_PED_MKEY                   ADDR_PED_SKEY_INDEX+1                   // b17, L-V, master key
//#define ADDR_PED_MKEY_01                ADDR_PED_MKEY                           //
//#define ADDR_PED_MKEY_02                ADDR_PED_MKEY_01+PED_MSKEY_SLOT_LEN2    //
//#define ADDR_PED_MKEY_03                ADDR_PED_MKEY_02+PED_MSKEY_SLOT_LEN2    //
//#define ADDR_PED_MKEY_04                ADDR_PED_MKEY_03+PED_MSKEY_SLOT_LEN2    //
//#define ADDR_PED_MKEY_05                ADDR_PED_MKEY_04+PED_MSKEY_SLOT_LEN2    //
//#define ADDR_PED_MKEY_06                ADDR_PED_MKEY_05+PED_MSKEY_SLOT_LEN2    //
//#define ADDR_PED_MKEY_07                ADDR_PED_MKEY_06+PED_MSKEY_SLOT_LEN2    //
//#define ADDR_PED_MKEY_08                ADDR_PED_MKEY_07+PED_MSKEY_SLOT_LEN2    //
//#define ADDR_PED_MKEY_09                ADDR_PED_MKEY_08+PED_MSKEY_SLOT_LEN2    //
//#define ADDR_PED_MKEY_10                ADDR_PED_MKEY_09+PED_MSKEY_SLOT_LEN2    //

//#define ADDR_PED_SKEY_01                ADDR_PED_MKEY_10+PED_MSKEY_SLOT_LEN2    // b17, L-V, session keys
//#define ADDR_PED_SKEY_02                ADDR_PED_SKEY_01+PED_MSKEY_SLOT_LEN     //
//#define ADDR_PED_SKEY_03                ADDR_PED_SKEY_02+PED_MSKEY_SLOT_LEN     //
//#define ADDR_PED_SKEY_04                ADDR_PED_SKEY_03+PED_MSKEY_SLOT_LEN     //
//#define ADDR_PED_SKEY_05                ADDR_PED_SKEY_04+PED_MSKEY_SLOT_LEN     //
//#define ADDR_PED_SKEY_06                ADDR_PED_SKEY_05+PED_MSKEY_SLOT_LEN     //
//#define ADDR_PED_SKEY_07                ADDR_PED_SKEY_06+PED_MSKEY_SLOT_LEN     //
//#define ADDR_PED_SKEY_08                ADDR_PED_SKEY_07+PED_MSKEY_SLOT_LEN     //
//#define ADDR_PED_SKEY_09                ADDR_PED_SKEY_08+PED_MSKEY_SLOT_LEN     //
//#define ADDR_PED_SKEY_10                ADDR_PED_SKEY_09+PED_MSKEY_SLOT_LEN     //

/*
** ------------------------------------------------------------------ **
**                      API Functions Declaration                     **
** ------------------------------------------------------------------ **
*/
extern  void  api_peds_InitGlobal( void );
extern  UCHAR api_peds_OpenPinPad( UCHAR dev );
extern  UCHAR api_peds_ClosePinPad( UCHAR dev );
extern  UCHAR api_peds_GetStatus( UCHAR *status );
extern  UCHAR api_peds_SetMasterKey( UCHAR index, UCHAR length, UCHAR *key );
extern  UCHAR api_peds_SetSessionKey( UCHAR index, UCHAR length, UCHAR *key );
extern  UCHAR api_peds_ResetPinPad( UCHAR tout, UCHAR type, UCHAR *amt );
extern  UCHAR api_peds_GetPin( UCHAR *keyin );
extern  UCHAR api_peds_MSKEY_GenPinBlock( UCHAR mode, UCHAR index, UCHAR *pan, UCHAR *epb );
extern  UCHAR api_peds_MSKEY_GenMAC( UCHAR mode, UCHAR index, UINT length, UCHAR *data, UCHAR *mac );
extern  UCHAR api_peds_LoadTerminalMasterKey( UCHAR port, UCHAR tout );
extern	UCHAR api_peds_ShowBalance( UCHAR tout, UCHAR *amt );
extern	UCHAR api_peds_SelectMasterKey( UCHAR index );
extern	UCHAR api_peds_MSKEY_TDES( UCHAR mode, UCHAR index, UCHAR *icv, UINT length, UCHAR *idata, UCHAR *odata );

//----------------------------------------------------------------------------
#endif
