//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : CONEXANT V.92 hardware MODEM (56K).                        **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APK_MDM.H                                                  **
//**  MODULE   : Declaration of MODEM Module.		                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2010/06/04                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2010 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _APK_MDM_H_
#define _APK_MDM_H_

//----------------------------------------------------------------------------
#define		MDM_STATE_IDLE			0
#define		MDM_STATE_INIT			1
#define		MDM_STATE_WAIT_CD		2
#define		MDM_STATE_WAIT_TOUT		3

//Define the number of country in country list 
#define		countrynum			55

//Define struct HOST for API_mdm

struct HOST
  {
    UCHAR  country ;       // modem Country Code 
    UCHAR  address ;       // node address of the station. (only effective in SDLC mode)
    UCHAR  mode ;          // modem data link protocol.
    UINT   baud ;          // modem communication data format.
    UCHAR  rdpi ;          // pause interval (seconds) for redialing.
    UCHAR  attempt1 ;      // no. of attempts to connect host by the primary phone number.
    UCHAR  cdon1 ;         // connection time (seconds) for the primary phone number.

    UCHAR  len1 ;          // length of the primary phone number.
    UCHAR  phone1[30] ;    // primary phone number. ( including  'T', 'P', ',' , '0' to '9' )
    UCHAR  attempt2 ;      // no. of attempts to connect host by the secondary phone number.
    UCHAR  cdon2 ;         // connection time (seconds) for the secondary phone number.
    UCHAR  len2 ;          // length of the secondary phone number.
    UCHAR  phone2[30] ;    // secondary phone number. ( including  'T', 'P', ',' , '0' to '9' )

  };

//Define struct nodetype for modem buffer
struct nodetype
  {
    UCHAR *buffer;
    UINT  buffer_size;
    struct nodetype *next;
  };

typedef struct nodetype *MODEM_BUFFER;

//Define struct mdmqueue for maintaining modem buffer
struct mdmqueue
  {
    MODEM_BUFFER front, rear, trial;
  };

//The flag for recording modem status

extern volatile UCHAR mdmStatusCode;  //一開始的狀態為modem idle
extern volatile UCHAR mdmStatusRLSDS; //連線狀態旗標


extern struct mdmqueue mdmTxqueue;
extern struct mdmqueue mdmRxqueue;


//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	UINT	apk_mdm_open( UCHAR *atcmd, API_HOST *sbuf );
extern	UCHAR	apk_mdm_close( UINT delay );
extern	UCHAR	apk_mdm_status( UCHAR *dbuf );
extern	UCHAR	apk_mdm_rxready( UCHAR *dbuf );
extern	UCHAR	apk_mdm_rxstring( UCHAR *dbuf );
extern	UCHAR	apk_mdm_txready( void );
extern	UCHAR	apk_mdm_txstring( UCHAR *sbuf );
extern	UCHAR	api_mdm_phonecheck();
extern	void	mdm_queue_remove( struct mdmqueue *pq );
extern	void	mdm_queue_insert ( struct mdmqueue *pq, UCHAR *data, UINT size, UCHAR *api_status );


// ---------------------------------------------------------------------------
#endif
