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
