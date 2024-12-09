//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : CONEXANT V.92 hardware MODEM (56K).                        **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APK_MDM.C 	                                            **
//**  MODULE   : apk_mdm_open()				                    **
//**		 apk_mdm_close()					    **
//**		 apk_mdm_status()					    **
//**		 apk_mdm_txready()					    **
//**		 apk_mdm_rxready()					    **
//**		 apk_mdm_txstring()					    **
//**		 apk_mdm_rxstring()					    **
//**									    **
//**  FUNCTION : API::MDM (MODEM Module)		    		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2010/06/03                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2010 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "bsp_uart.h"
#include "bsp_tmr.h"
#include "POSAPI.h"
#include "APK_MDM.h"
#include "DEV_AUX.h"
#include "esf.h"
#include "redirtsk.h"
#include "SDLC.h"

extern		API_AUX		os_AUX_Para[];
extern		API_AUX		os_AUX_ParaBak[];
extern		AUX_DATA	os_AUX_RxDataBuffer[];	// Rx interface buffer
extern		AUX_DATA	os_AUX_TxDataBuffer[];	// Tx interface buffer

extern		UCHAR 		*pmdmTxBuf;
extern		UCHAR		*pmdmRxBuf;

extern		unsigned int	mdmCountTX;
extern		unsigned int	mdmCountRX;
extern		UINT		mdmCanTx;
extern		UCHAR		StartSdlcFlag;
extern		UCHAR		SdlcSendCount;			//SDLC Ns
extern		UCHAR		SdlcReceiveCount;		//HDLS Nr
extern		UCHAR		SdlcStartMoveFlag;

extern		BSP_UART	*os_pAUX2;

volatile	UCHAR		mdmEscSequenceFlag = 0;		// "+++"
volatile	UCHAR		mdmDialConFlag = 0; 		//ï¿½Î¨Ó§Pï¿½_ï¿½Oï¿½_ï¿½~ï¿½ò¼·¸ï¿½ï¿½ï¿½ï¿½Xï¿½ï¿½
volatile	UCHAR		mdmTimeoutFlag = 0; 		//ï¿½ï¿½ï¿½ï¿½Timeoutï¿½Oï¿½_ï¿½oï¿½Íªï¿½ï¿½Xï¿½ï¿½
volatile	UCHAR		mdmVtpTimeoutFlag = 0;
volatile	UCHAR		mdmEscOpenFlag = 0; 		//ï¿½ï¿½ï¿½ï¿½ï¿½sï¿½uï¿½Xï¿½ï¿½
volatile	UCHAR 		mdmStatusCode = mdmIdle;	//modemï¿½ï¿½lï¿½ï¿½ï¿½Aï¿½ï¿½modem idle
volatile	UCHAR 		mdmStatusRLSDS = 0;		//ï¿½sï¿½uï¿½ï¿½ï¿½Aï¿½Xï¿½ï¿½
volatile	UCHAR 		mdmMode = mdmREFERRAL;		//ï¿½ï¿½ï¿½Fï¿½ï¿½ï¿½ï¿½ï¿½Ü¾ï¿½ï¿½ï¿½ï¿½A, modem modeï¿½wï¿½]ï¿½ï¿½mdmREFERRAL
volatile	UCHAR		mdmPhonChkFlag = 0;
volatile	UCHAR		mdmReferralFlag = 0;
volatile	UCHAR		mdmOffHookInstrFlag = 0;
volatile	float		mdmVtroff;			//ï¿½ï¿½ï¿½ï¿½qï¿½ï¿½ï¿½Hï¿½ï¿½ï¿½ï¿½ï¿½Oï¿½_ï¿½oï¿½ï¿½OFF-HOOK INTRUSION
volatile	float		mdmVtr;				//ï¿½Ñ¦Ò¹qï¿½ï¿½ï¿½Aï¿½Î¨Ó§Pï¿½_ï¿½bREFERRALï¿½Ò¦ï¿½ï¿½Üµï¿½ï¿½Oï¿½_ï¿½ï¿½ï¿½W
volatile	float		mdmVtpMonit;			//ï¿½ï¿½ï¿½ï¿½monitorï¿½Ò±oTipï¿½MRingï¿½Ð·Ç¹qï¿½ï¿½

UCHAR		SdlcAddress;	//ï¿½ï¿½ï¿½ï¿½SDLC address

UINT		os_DHN_MDM56K = 0;
//BSP_GPIO	*pCOM1_SEL;
BSP_TIMER	*pMdmFlowControl;
BSP_TIMER	*pTryconnectTimer = NULLPTR; //Use for creating a Timer
BSP_TIMER   *pVtpTimer = NULLPTR;
BSP_UART	*pMdmUart = 0;
//NOS_TASK	*pMdmFlowControlTask = NULLPTR;
BSP_TIMER	*pMdmFlowControlTask = NULLPTR;
//NOS_TASK	*pReadFromModemTask  = NULLPTR;
BSP_TIMER	*pReadFromModemTask  = NULLPTR;
//NOS_TASK	*pWriteToModemTask   = NULLPTR;

volatile	ULONG		MDM_FlowControlState = MDM_STATE_IDLE;
volatile	ULONG		MDM_FlowControlNextState = MDM_STATE_IDLE;

UCHAR		MDM_country;
UCHAR		MDM_address;
UINT		MDM_baud;
ULONG		MDM_cdon;
ULONG		MDM_cdon_bak;
ULONG		MDM_rdpi;
ULONG		MDM_rdpi_bak;
ULONG		MDM_cdon1;
ULONG		MDM_cdon2;
UCHAR		MDM_attempt1;
UCHAR		MDM_attempt2;
UCHAR		MDM_len1;
UCHAR		MDM_len2;
UCHAR		MDM_phone1[30];
UCHAR		MDM_phone2[30];

//function declaration
UCHAR		apk_mdm_init( void );
void		*MDM_FlowControlTask( void );
void		MDM_ClearRTS( void );
void		MDM_FlowControlIsr( void );
extern		void		*read_from_modem_task( void );
extern		void		write_to_modem_task( void );
extern		void		MDM_ResetAuxTOB( UINT tob );
extern		UCHAR		MDM_OpenAuxPort( void );
extern	BSP_UART *AUX_GetDHN( UINT32 port );
extern	BSP_UART *AUX_CheckDHN( UINT dhn );
//UCHAR		g_ATH_SW = 0;
//BSP_GPIO	*pMDM_DCD;
ULONG		os_mdm56k_flow_ctrl = FALSE;

struct	mdmqueue mdmTxqueue=
	{
	NULL, NULL, NULL
	};

struct	mdmqueue mdmRxqueue=
	{
	NULL, NULL, NULL
	};

// part of T.35 country code list
/* REMOVED: 2011-08-17, should refer to T.35 country code
UCHAR              countrylist[countrynum]={ 0x26, 0xFE, 0x30, 0x50, 0x61, 0xBC, 0x1C, 0xA9, 0x00, 0x89,
  	                                     0x63, 0x54, 0x6C, 0x1A, 0x53, 0x9C, 0x55, 0x56, 0x62, 0x0C,
  	                                     0x83, 0x8D, 0x98, 0xB3, 0xBF, 0x58, 0x5E, 0x64, 0xA7, 0xAE,
  	                                     0X2D, 0x36, 0xC5, 0xC8, 0xC9, 0x5C, 0x84, 0xD2, 0xEB, 0xC7,
  	                                     0xD1, 0xCA, 0x02, 0x71, 0x67, 0x77, 0xAD, 0x32, 0xEF, 0x9E,
  	                                     0xA2, 0x20, 0xE3, 0xB5, 0x12 };
*/

// ---------------------------------------------------------------------------
//ï¿½wï¿½qï¿½Î¨Ó½Tï¿½{Timeoutï¿½Oï¿½_ï¿½oï¿½Íªï¿½Timer
void mdm_TryconnectTimerLisr (  )
{
	sleep(5);
//  DEBUGPRINTF( "60 secs timeout occurs\r\n" );
	while(1)
	{
		uint64_t COUNT;
		mdmTimeoutFlag = 1;
		BSP_TMR_GetTick(pTryconnectTimer,&COUNT);
	}
}

// ---------------------------------------------------------------------------
//ï¿½wï¿½qï¿½Î¨Ó½Tï¿½{Timeoutï¿½Oï¿½_ï¿½oï¿½Íªï¿½Timer
void* mdm_VtpTimerLisr (  )
{
  ULONG tick=0;
  uint64_t COUNT=0;
  //usleep(400000);//wait 400ms to timeout
  
  while(1)
  {
	if(BSP_TMR_GetTick(pVtpTimer,&COUNT)>0)
		tick+=COUNT;
	
	if(tick>400)//4000ms timeout
	{	
		DEBUGPRINTF( "Vtp monitor timeout occurs\r\n" );
		if(mdmPhonChkFlag)
			mdmVtpTimeoutFlag = 0;  
		else
			mdmVtpTimeoutFlag = 1;  
	}
  }
}

// ---------------------------------------------------------------------------
//******************************************************************************
// ï¿½wï¿½qï¿½Î¨Ó¾Þ§@modem queueï¿½Bï¿½@ï¿½ï¿½function
//******************************************************************************

int mdm_empty ( struct mdmqueue *pq )
{
  return (pq -> front == NULL) ? TRUE : FALSE;
}

MODEM_BUFFER mdm_getnode ( UINT size, UCHAR *api_status )
{
  MODEM_BUFFER q;
  // q = ( MODEM_BUFFER ) BSP_Malloc( ( UINT32 ) sizeof ( struct nodetype ) );
  q = ( MODEM_BUFFER ) malloc( ( UINT32 ) sizeof ( struct nodetype ) );
  if ( q==NULL )
    {
      DEBUGPRINTF( "getnode failure, because memory allocated failure\r\n" );
      *api_status=apiFailed;
    }
  q-> buffer = ( UCHAR* ) malloc( ( UINT32 ) ( size * sizeof( UCHAR ) ) );
  if ( q==NULL )
    {
      DEBUGPRINTF( "getnode failure, because memory allocated failure\r\n" );
      *api_status=apiFailed;
    }
  return q;
}

void mdm_freenode ( MODEM_BUFFER p )
{
  free( p->buffer );
  free( p );
  p->buffer = NULL;
  p = NULL;
  return;
}

void mdm_queue_insert ( struct mdmqueue *pq, UCHAR *data, UINT size, UCHAR *api_status )
{
  MODEM_BUFFER q = mdm_getnode( size, api_status );
  q->buffer_size=size;  //ï¿½Nï¿½ï¿½?¤jï¿½pï¿½ï¿½ï¿½ï¿½Tï¿½xï¿½sï¿½bnode
  memcpy( q->buffer, data, size );
  q->next = NULL;
  if ( pq->rear==NULL )
    {
      pq->front=q;
      pq->trial=q;
    }
  else
    {
      pq->rear->next=q;
    }
  pq->rear=q;
  return;
}

void mdm_txqueue_insert ( struct mdmqueue *pq, UCHAR *data, UINT size, UCHAR *api_status )
{
  MODEM_BUFFER q = mdm_getnode( size, api_status );
  q->buffer_size = size;  //ï¿½Nï¿½ï¿½?¤jï¿½pï¿½ï¿½ï¿½ï¿½Tï¿½xï¿½sï¿½bnode
  memcpy( q->buffer, data, size );
  q->next = NULL;
  if ( pq->rear == NULL )
    {
      pq->front = q;
    }
  else
    {
      pq->rear->next = q;
    }
  pq->rear = q;
  return;
}

void mdm_rxqueue_insert ( struct mdmqueue *pq, unsigned int size, UCHAR *api_status )
{
  MODEM_STATUS   eRetCode;
  MODEM_BUFFER q = mdm_getnode( size, api_status );

  q->buffer_size = size;  //ï¿½Nï¿½ï¿½?¤jï¿½pï¿½ï¿½ï¿½ï¿½Tï¿½xï¿½sï¿½bnode
  eRetCode = cnxt_modem_read( ( char * )q->buffer, &size );//ï¿½Nï¿½ï¿½?¥ï¿½modemï¿½ï¿½ï¿½wï¿½Ä°ï¿½Åªï¿½Jqueueï¿½ï¿½
  if ( eRetCode != MODEM_OK )
    {
      DEBUGPRINTF( "read_from_modem_task: cnxt_modem_read error(%d)\n\r", eRetCode );
    }

  q->next = NULL;
  if ( pq->rear == NULL )
    {
      pq->front = q;
    }
  else
    {
      pq->rear->next = q;
    }
  pq->rear = q;
  return;
}

void mdm_queue_remove( struct mdmqueue *pq )
{
  MODEM_BUFFER q;

  q = pq->front;
  pq->front = q->next;
  mdm_freenode ( q );

  if (pq->front == NULL)
    {
      pq->trial = NULL;
      pq->rear = NULL;
    }
}

//For debug
void mdm_verify( MODEM_BUFFER p )
{
  MODEM_BUFFER q = p;
  int counter = 0;
  int i = 0;
  if ( p == NULL )
    {
      DEBUGPRINTF ( "list or queue is empty!!\r\n" );
    }
  else
    {
      for ( ;q!=NULL; q=q->next, counter++ )
        {
          DEBUGPRINTF( "node size is %d \r\n", q->buffer_size );
          for (i=0; i<q->buffer_size ;i++ )
            {
              DEBUGPRINTF( "%c", *( q->buffer+i ) );
            }
        }
      DEBUGPRINTF( "\nEND\n" );
    }
  DEBUGPRINTF( "Numbers of nodes within this queue is %d\r\n", counter );
  return;
}

// ---------------------------------------------------------------------------
UCHAR	api_mdm_createtask( void )
{
	if( MDM_OpenAuxPort()==apiOK )
	{
		//pMdmFlowControlTask = NosCreate( MDM_FlowControlTask,  PERIODIC_PRIO,    TASK_STACK_SIZE, 0 );
		pMdmFlowControlTask = BSP_TMR_Acquire(20);
		if(BSP_TMR_Start(MDM_FlowControlTask,pMdmFlowControlTask)!=BSP_SUCCESS)
			return( apiFailed );
		// pReadFromModemTask  = NosCreate( read_from_modem_task, MODEM_READ_PRIO,  TASK_STACK_SIZE, 0 );
		pReadFromModemTask =  BSP_TMR_Acquire2(1);//1ms
		if(BSP_TMR_Start(read_from_modem_task,pReadFromModemTask)!=BSP_SUCCESS)
			return( apiFailed );
//		pWriteToModemTask   = NosCreate( write_to_modem_task,  MODEM_WRITE_PRIO, TASK_STACK_SIZE, 0 );
		pMdmFlowControl = BSP_TMR_Acquire(1);//10ms timer
		if( BSP_TMR_Start( (void *)MDM_FlowControlIsr,pMdmFlowControl ) != BSP_SUCCESS )	// start timer thread
			return( apiFailed );
		
		
		
//		if( pMdmFlowControlTask && pReadFromModemTask && pWriteToModemTask )
		if( pMdmFlowControlTask && pReadFromModemTask &&  pMdmFlowControl)
			return( apiOK );
	}

	return( apiFailed );
}

//******************************************************************************
// mdm_005_002
// ï¿½wï¿½q api_mdm_vtpmonitor
// ï¿½\ï¿½ï¿½Gï¿½Ê´ï¿½Tipï¿½MRingï¿½ï¿½ï¿½qï¿½ï¿½
//******************************************************************************

UCHAR api_mdm_vtpmonitor ( float *Vtp )
{
  UCHAR              api_status = apiOK;
  UCHAR              at_cmd[]={"AT-TRV\r"};
  unsigned int       size;
  MODEM_STATUS       eRetCode;

  //ï¿½]ï¿½wTimerï¿½}ï¿½lï¿½pï¿½ï¿½
  //pVtpTimer = BSP_TMR_Acquire( BSP_LARGE_TIMER, mdm_VtpTimerLisr );
  pVtpTimer = BSP_TMR_Acquire( 1 );

  if ( pVtpTimer )
    {
      //ï¿½Ø¥ß½Tï¿½{Timeoutï¿½Oï¿½_ï¿½oï¿½Íªï¿½Timer
      // pVtpTimer->Count = 1;
      // pVtpTimer->Compare = BSP_GetClock( TRUE, 2048/3 );//ï¿½]ï¿½wTimeoutï¿½É¶ï¿½ï¿½ï¿½ï¿½Tï¿½ï¿½
      // pVtpTimer->Control = TMR_PRES_DIV_2048 | TMR_TEN | TMR_MODE_RELOAD;
      // BSP_TMR_Start( pVtpTimer,mdm_VtpTimerLisr );//Timerï¿½}ï¿½lï¿½pï¿½ï¿½
      BSP_TMR_Start( mdm_VtpTimerLisr,pVtpTimer );//Timerï¿½}ï¿½lï¿½pï¿½ï¿½
    }
  else
    {
      DEBUGPRINTF( "Timer required error\r\n" );
      return apiFailed;
    }

  size = ( sizeof ( at_cmd ) )-1;
  eRetCode = cnxt_modem_write( ( char * ) at_cmd, &size );
  if (eRetCode != MODEM_OK)
    {
      DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error(%d)\n\r", eRetCode );
    }
  mdmPhonChkFlag = 1;
  //ï¿½ï¿½ï¿½Ýªï¿½ï¿½ì±µï¿½ï¿½ï¿½ï¿½^ï¿½ï¿½ï¿½Tï¿½ï¿½

  while ( mdmPhonChkFlag && !mdmVtpTimeoutFlag );

  if ( mdmVtpTimeoutFlag )
    {
      api_status = apiFailed;
    }
  //BSP_TMR_Release( pVtpTimer );//ï¿½ï¿½ï¿½ï¿½Timer
  BSP_TMR_Stop( pVtpTimer );
  pVtpTimer = NULLPTR;

  mdmVtpTimeoutFlag = 0;
  mdmPhonChkFlag = 0;
  *Vtp = mdmVtpMonit;

  return api_status;
  //DEBUGPRINTF( "Vtp is %5.2f\r\n", Vtp);
}

// ---------------------------------------------------------------------------
  float              Vtp1;
  float              Vtp2;
// UCHAR	api_mdm_phonecheck( UCHAR *LineStatus )
UCHAR	api_mdm_phonecheck()
{
  UCHAR              api_status = apiOK;
  UCHAR              at_cmd1[]={"ATH1\r"};/*modem off-hook*/
  UCHAR              at_cmd2[]={"ATH0\r"};/*modem on-hook*/
  unsigned int       size;
//  float              Vtp1;
//  float              Vtp2;
  MODEM_STATUS       eRetCode;


  MDM_OpenAuxPort();
  // MDM_ResetAuxTOB(100);
  MDM_ResetAuxTOB(100);

  api_status = api_mdm_vtpmonitor( &Vtp1 );
  if ( api_status == apiFailed )
    {
      DEBUGPRINTF("Vtp monitor failed\n\r" );
      return api_status;
    }
  //Tipï¿½MRingï¿½ï¿½ï¿½qï¿½ï¿½ï¿½pï¿½ï¿½4Vï¿½ï¿½ï¿½ï¿½No Line
  if ( Vtp1 < 4)
    {
      // *LineStatus = mdmNoLine;
      mdmStatusCode = mdmNoLine;
      // DEBUGPRINTF( "modem status is %d\r\n", *LineStatus );
      DEBUGPRINTF( "modem status is %d\r\n", mdmStatusCode );
      return api_status;
    }
  if ( !mdmReferralFlag )
    {
      /*ï¿½oï¿½eATH1
       *modem off-hook*/
      size = ( sizeof ( at_cmd1 ) )-1;
      eRetCode = cnxt_modem_write_atc( ( char * ) at_cmd1, &size, 300 );
      if (eRetCode != MODEM_OK)
        {
          DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error(%d)\n\r", eRetCode );
        }

      //BSP_Delay_n_ms( 150 );//ï¿½ï¿½ï¿½Ý¦ï¿½ATH1ï¿½_ï¿½@ï¿½ï¿½, add
		usleep(150000);//sleep 150 ms
      //ï¿½ï¿½ï¿½ï¿½Vtpï¿½qï¿½ï¿½
      api_status = api_mdm_vtpmonitor( &Vtp2 );
      if ( api_status == apiFailed )
        {
          DEBUGPRINTF("Vtp monitor failed\n\r" );
          return api_status;
        }
      /*ï¿½oï¿½eATH0
       *modem on-hook*/
      size = ( sizeof ( at_cmd2 ) )-1;
      eRetCode = cnxt_modem_write_atc( ( char * ) at_cmd2, &size, 300 );
      if (eRetCode != MODEM_OK)
        {
          DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error(%d)\n\r", eRetCode );
        }
      //ï¿½ï¿½ï¿½Vtp1ï¿½PVtp2ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?
      if ( fabs(Vtp1 - Vtp2)<10 )
        {
          // *LineStatus = mdmLocalLineBusy;
          mdmStatusCode = mdmLocalLineBusy;
        }
      else
        {
          // *LineStatus = mdmIdle ;
          mdmStatusCode = mdmIdle;
          mdmVtr = Vtp1;//ï¿½Nï¿½ï¿½ï¿½É°ï¿½ï¿½ï¿½ï¿½ì¤§ï¿½qï¿½ï¿½ï¿½]ï¿½wï¿½ï¿½ï¿½Ð·Ç¹qï¿½ï¿½
        }

    }
  else
    {
      if ( !mdmOffHookInstrFlag )
        {
          if ( Vtp1 < mdmVtroff * 0.9 )
            {
              DEBUGPRINTF( "OFF-HOOK INTRUSION**************************\r\n" );
              //ï¿½Ï¥ÎªÌ®ï¿½ï¿½_ï¿½Üµï¿½
              mdmOffHookInstrFlag = 1;
              //modem on-hook
              size = ( sizeof ( at_cmd2 ) )-1;
              eRetCode = cnxt_modem_write_atc( ( char * ) at_cmd2, &size, 300 );
              if (eRetCode != MODEM_OK)
                {
                  DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error(%d)\n\r", eRetCode );
                }
              //BSP_Delay_n_ms( 150 );//ï¿½ï¿½ï¿½Ý¦ï¿½ATH1ï¿½_ï¿½@ï¿½ï¿½
              usleep( 150000 );//ï¿½ï¿½ï¿½Ý¦ï¿½ATH1ï¿½_ï¿½@ï¿½ï¿½
              // *LineStatus = mdmLocalLineBusy;
              mdmStatusCode = mdmLocalLineBusy;
            }
          else
            {
              mdmVtroff = Vtp1;//ï¿½ï¿½sï¿½Pï¿½_off-hookï¿½ï¿½ï¿½Ð·Ç¹qï¿½ï¿½
              // *LineStatus = mdmIdle ;
              mdmStatusCode = mdmIdle;
            }
        }
      else
        {
          if ( Vtp1 <= 0.85 * mdmVtr )
            {
              //ï¿½Ï¥ÎªÌ®ï¿½ï¿½_ï¿½Üµï¿½
              // *LineStatus = mdmLocalLineBusy;
              mdmStatusCode = mdmLocalLineBusy;
            }
          else
            {
              DEBUGPRINTF( "PARALLEL PHONE ON-HOOK************************\r\n" );
              //ï¿½Ï¥ÎªÌ±ï¿½ï¿½Wï¿½Üµï¿½
              // *LineStatus = mdmIdle ;
              mdmStatusCode = mdmIdle;
            }
        }
    }
  // DEBUGPRINTF( "Line status is %d\r\n", *LineStatus );
  DEBUGPRINTF( "Line status is %d\r\n", mdmStatusCode );

  return api_status;

}

// ---------------------------------------------------------------------------
void	MDM_HangUp( void )
{
UCHAR	hang_up1[]={"ATH\r"};
UCHAR	hang_up2[]={"+++ATH\r"};
UCHAR	api_status = apiOK;
MODEM_STATUS	eRetCode;


  if (mdmMode == mdmSDLC)
    {
      UCHAR ecs[2];
      ecs[0] = 0x19;
      ecs[1] = 0xBA;
      mdmCountTX = 2;
      eRetCode = cnxt_modem_write_atc( ( char * )ecs, &mdmCountTX, 300 );
      if (eRetCode != MODEM_OK)
        {
          DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error(%d)\n\r", eRetCode );
        }

      DEBUGPRINTF( "!!!!!Send ecs \r\n");
      mdmCountTX = 4;
      eRetCode = cnxt_modem_write_atc( ( char * )hang_up1, &mdmCountTX, 300 );
      if (eRetCode != MODEM_OK)
        {
          DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error(%d)\n\r", eRetCode );
        }

      DEBUGPRINTF( "!!!!!Hang up\r\n");
    }
  else
    {
      mdmCountTX = 7;
      eRetCode = cnxt_modem_write_atc( ( char * )hang_up2, &mdmCountTX, 300 );
      if (eRetCode != MODEM_OK)
        {
          DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error(%d)\n\r", eRetCode );
        }
    }
    
}

// ---------------------------------------------------------------------------
// FUNCTION: Initialize GPIO ports for COM1 port selection.
//		COM1_SEL - GPIO0[13]
//			0 = normal COM1 PORT
//			1 = MODEM PORT
// INPUT   : dhn
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UCHAR	apk_mdm_init( void )
{
	return( TRUE );
	/*
	pCOM1_SEL = BSP_GPIO_Acquire( GPIO_PORT_0,BIT13 );
	if( pCOM1_SEL )
	  {
	  pCOM1_SEL->Mode     = GPIO_MODE_OUTPUT;
	  pCOM1_SEL->IntMode  = GPIO_INT_DISABLE;
	  pCOM1_SEL->Wake     = GPIO_WAKE_DISABLE;
	  pCOM1_SEL->Data     = 0;
	  pCOM1_SEL->pIsrFunc = NULLPTR;
	  BSP_GPIO_Start( pCOM1_SEL );
	  
	  // set default COM1_SEL = 0
	  BSP_GPIO_OUT_0(pCOM1_SEL);
	  
	  pMDM_DCD   = BSP_GPIO_Acquire( GPIO_PORT_0,BIT14 );	// low active
	  if( pMDM_DCD )
	    {
	    pMDM_DCD->Mode     = GPIO_MODE_INPUT;
	    pMDM_DCD->IntMode  = GPIO_INT_DISABLE;
	    pMDM_DCD->Wake     = GPIO_WAKE_DISABLE;
	    pMDM_DCD->Data     = 0;
	    pMDM_DCD->pIsrFunc = NULLPTR;
	    BSP_GPIO_Start( pMDM_DCD );
	    
	    return( TRUE );
	    }
	  }
*/
	return( FALSE );
}

// ---------------------------------------------------------------------------
// 300 BPS
UCHAR	const	V21FC_ASYNC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;+MS=V21;S17=64\r"};
UCHAR	const	B103FC_ASYNC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;+MS=B103;S17=64\r"};
UCHAR	const	V21_ASYNC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;+MS=V21;S17=0\r"};
UCHAR	const	B103_ASYNC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;+MS=B103;S17=0\r"};

// 1200 BPS
UCHAR	const	V22FC_SDLC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;$F2;+MS=V22;+ES=6,,8;+ESA=0,0,,,1,0;S17=13\r"};
UCHAR	const	V22_ASYNC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;+MS=V22;S17=0\r"};
UCHAR	const	BELL212_SDLC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;+MS=B212;+ES=6,,8;+ESA=0,0,,,1,0;S17=7\r"};
UCHAR	const	BELL212_ASYNC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;+MS=B212;S17=6\r"};

// 2400 BPS
UCHAR	const	V22B_SDLC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;+MS=V22B;+ES=6,,8;+ESA=0,0,,,1,0;S17=7\r"};
UCHAR	const	V22B_ASYNC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;+MS=V22B;S17=6\r"};

// 9600 BPS
//UCHAR	const	V29FC_SDLC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;$F4;+ES=6,,8;+ESA=0,0,,,1,0;S17=13\r"};
UCHAR	const	V29FC_SDLC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;$F4;+ES=6,,8;+ESA=0,0,,,1,0;S17=13\rAT+IBC=1,1,1,,,1\r"};
UCHAR	const	V32_ASYNC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;+MS=V32;S17=0\r"};

// 14400 BPS
UCHAR	const	V32B_ASYNC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;+MS=V32B;S17=0\r"};

// 33600 BPS
UCHAR	const	V34_ASYNC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;+MS=V34;S17=0\r"};

// 57600 BPS
UCHAR	const	V90_ASYNC[]	=	{"AT&F;%C0\\N0;+A8E=,,,0;+MS=V90;S17=0\r"};

// ---------------------------------------------------------------------------
// FUNCTION: MODEM flow control state machine process - MDM_STATE_INIT.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	MDM_FCS_Init( void )
{
UCHAR	i;
UCHAR	dial_time1[128];
//UCHAR	dial_time1_1200[] = {"ATZ\rAT&K0\rAT+MS=V22\rAT$F2\rAT%C0\rAT\\N0\rATS9=1\rAT+ES=6,,8\rATS17=7\rATW2\r"};
//UCHAR	dial_time1_2400[] = {"ATZ\rAT&K0\rAT+MS=V22B\rAT$F0\rAT%C0\rAT\\N0\rAT+ES=6,,8\rATS17=7\rATW2\r"};
////UCHAR	dial_time1_9600[] = {"ATZ\rAT&K3\rAT+MS=V32\rAT$F4\rAT%C0\rAT\\N0\rATS9=1\rAT+ES=6,,8\rATS17=7\rATW2\r"};
////UCHAR	dial_time1_9600[] = {"ATZ\rAT&F\rAT%C0\rAT\\N0\rAT+A8E=,,,0\rAT$F4\rAT+ES=6,,8\rAT+ESA=0,0,,,1,0\rATS17=13\rAT+IBC=1,1,1,,,1\r"};
//UCHAR	dial_time1_9600[] = {"ATZ\rAT&F;%C0\\N0;+A8E=,,,0;$F4;+ES=6,,8;+ESA=0,0,,,1,0;S17=13\rAT+IBC=1,1,1,,,1\r"};
UCHAR	dial_time2[] = {"ATZ\r"};
UCHAR	dial_atw2[] =  {"ATW2\r"};
////UCHAR	dial_time3[] = {"AT+ESA=0,0,0,,1,0,255,\r"};
//UCHAR	dial_time3[] = {"AT+ESA=0,0,,,1,0\r"};
UCHAR	dial_atgci[] = {"AT+GCI="};//Command for reconfigurating modem country code
UCHAR	dial_cmd[] = {"ATD"};
UCHAR	CONNECT_str[] = {"\r CONNECT"};
UCHAR	dial_country[15] = {0};
UCHAR	dial_phone[35] = {0};
//UCHAR	dial_ms[]={"AT+MS="};
UCHAR	countryCode;
UCHAR	dial_baud[15] = {0};
UINT	baudRate;
UCHAR	baudLeng=0;
UCHAR	baudMode[4] = {0};
UCHAR	*ATcmd;
UCHAR	countryValidFlag = 0;
MODEM_STATUS	eRetCode;
UCHAR	MDM_PhoneLen;
UCHAR	MDM_PhoneNum[30];
UINT	iLen = 0;
UINT	tmpCount;
UINT	Avail;
UCHAR	temp[128];
//UCHAR	os_mdm56k_temp_buffer[128];
UCHAR	result;

	os_mdm56k_flow_ctrl = FALSE;

	mdmEscOpenFlag = 0;//ï¿½Mï¿½ï¿½ï¿½ï¿½ï¿½Xapi_mdm_openï¿½Xï¿½ï¿½
	
	if( mdmMode == mdmSDLC )
  	  {                                                                           
  	  SdlcStartMoveFlag = 0;	// PATCH: 2010-05-14                             
                                                                              
    	  switch( (MDM_baud) & 0x7FE0 )
        	{
        	case	MDM_1200:	// V.22 Fast Connect
//       		memmove( dial_time1, dial_time1_1200, strlen(dial_time1_1200) );
//        		iLen = sizeof( dial_time1_1200 );

			if( MDM_baud & MDM_BELL )
			  {
       			  memmove( dial_time1, BELL212_SDLC, strlen(BELL212_SDLC) );
        		  iLen = sizeof( BELL212_SDLC );
        		  }
        		else
        		  {
       			  memmove( dial_time1, V22FC_SDLC, strlen(V22FC_SDLC) );
        		  iLen = sizeof( V22FC_SDLC );
        		  }
        		
        		break;
        	                                                                 
        	case	MDM_2400:	// V.22bis Normal
//        		memmove( dial_time1, dial_time1_2400, strlen(dial_time1_2400) );
//        		iLen = sizeof( dial_time1_2400 );

        		memmove( dial_time1, V22B_SDLC, strlen(V22B_SDLC) );
        		iLen = sizeof( V22B_SDLC );

        		break;

        	case	MDM_9600:	// V.29 Fast POS
//        		memmove( dial_time1, dial_time1_9600, strlen(dial_time1_9600) );
//        		iLen = sizeof( dial_time1_9600 );

        		memmove( dial_time1, V29FC_SDLC, strlen(V29FC_SDLC) );
        		iLen = sizeof( V29FC_SDLC );
        		
        		break;
        	
        	default:
        		mdmStatusCode = mdmFailed;
        		return;
        	}
          }

        if( mdmMode == mdmBYPASS )
          {
    	  switch( (MDM_baud) & 0x7FE0 )
        	{
        	case	MDM_300:	// BELL 103	<cr><lf>CONNECT<cr><lf>
        	
        		if( MDM_baud & MDM_BELL )
        		  {
        		  memmove( dial_time1, B103_ASYNC, strlen(B103_ASYNC) );
        		  iLen = sizeof( B103FC_ASYNC );
        		  }
        		else
        		  {
        		  memmove( dial_time1, V21_ASYNC, strlen(V21_ASYNC) );
        		  iLen = sizeof( V21FC_ASYNC );
        		  }
        		  
        		break;
        		
        	case	MDM_1200:	// V.22		<cr><lf>CONNECT 1200<cr><lf>

        		if( MDM_baud & MDM_BELL )
        		  {
        		  memmove( dial_time1, BELL212_ASYNC, strlen(BELL212_ASYNC) );
        		  iLen = sizeof( BELL212_ASYNC );
        		  }
        		else
        		  {
        		  memmove( dial_time1, V22_ASYNC, strlen(V22_ASYNC) );
        		  iLen = sizeof( V22_ASYNC );
        		  }
        		  
        		break;

        	case	MDM_2400:	// V.22bis	<cr><lf>CONNECT 2400<cr><lf>
        		memmove( dial_time1, V22B_ASYNC, strlen(V22B_ASYNC) );
        		iLen = sizeof( V22B_ASYNC );
        		
        		break;

        	case	MDM_9600:	// V.32		<cr><lf>CONNECT 9600<cr><lf>
        		memmove( dial_time1, V32_ASYNC, strlen(V32_ASYNC) );
        		iLen = sizeof( V32_ASYNC );
        		
        		break;

        	case	MDM_14400:	// V.32bis	<cr><lf>CONNECT 14400<cr><lf>
        		memmove( dial_time1, V32B_ASYNC, strlen(V32B_ASYNC) );
        		iLen = sizeof( V32B_ASYNC );
        		break;

        	case	MDM_33600:	// V.34		<cr><lf>CONNECT 33600<cr><lf>
        		memmove( dial_time1, V34_ASYNC, strlen(V34_ASYNC) );
        		iLen = sizeof( V34_ASYNC );
        		
        		break;

        	case	MDM_56000:	// V.90		<cr><lf>CONNECT XXXXX<cr><lf>
        		memmove( dial_time1, V90_ASYNC, strlen(V90_ASYNC) );
        		iLen = sizeof( V90_ASYNC );
        		
        		break;

        	default:
        		mdmStatusCode = mdmFailed;
        		return;
        	}
          }

	SdlcAddress = MDM_address;
	countryCode = MDM_country;	//ï¿½Ë¬dcountry codeï¿½Oï¿½_ï¿½Oï¿½Wï¿½æ¤¤ï¿½ï¿½ï¿½ï¿½a
	baudRate = MDM_baud;		//ï¿½]ï¿½wmodem baud rate

/*
        if( baudRate != 0 )
          {
          if( baudRate == MDM_1200)
            {
            memmove( baudMode, "V22", sizeof( baudMode) );
	    baudLeng = 3;
	    }
          else if( baudRate == MDM_2400)
          	 {
                 memmove( baudMode, "V22B", sizeof( baudMode) );
	         baudLeng = 4;
	         }
          else if( baudRate == MDM_9600)
          	 {
                 memmove( baudMode, "V32", sizeof( baudMode) );
	         baudLeng = 3;
	         }
          else if( baudRate == MDM_57600)
          	 {
                 memmove( baudMode, "V90", sizeof( baudMode) );
	         baudLeng =3;	 
	         }
               else
                 {
                 baudRate = 0;
	         }
          }
*/

/*	REMOVED: 2011-08-17, should refer to T.35 country code
        for( i=0; i<countrynum; i++ )
           {
           if( countryCode == countrylist[i] )
             {
             countryValidFlag = 1;
             } 	
          	
           }
           
        //If the selected country code is not in the country list, the set the coutry code as default (USA: 0xB5)
	if( ! countryValidFlag )
	  {
	  countryCode = 0xB5;	
	  }
*/

	if( mdmMode == mdmSDLC )
	  {
	  StartSdlcFlag = 0;//ï¿½]ï¿½wSDLCï¿½}ï¿½lï¿½Xï¿½{ï¿½ï¿½flagï¿½ï¿½0
	  SdlcSendCount = 0;
	  SdlcReceiveCount = 0;
	  }

	//ï¿½Nqueueï¿½Mï¿½ï¿½
	for( ;mdmRxqueue.front!=NULL; mdm_queue_remove( &mdmRxqueue ) )
           ;
	for( ;mdmTxqueue.front!=NULL; mdm_queue_remove( &mdmTxqueue ) )
	   ;
	// Check attempt1
	if( (MDM_attempt1) && (MDM_len1) )
	  {
	  MDM_attempt1 -= 1;
	  
	  MDM_PhoneLen = MDM_len1;
	  memmove( MDM_PhoneNum, MDM_phone1, MDM_PhoneLen );
	  MDM_cdon = MDM_cdon1;		// in 10ms
	  }
	else
	  {
	  // Check attempt2
	  if( (MDM_attempt2) && (MDM_len2) )
	    {
	    MDM_attempt2 -= 1;
	    
	    MDM_PhoneLen = MDM_len2;
	    memmove( MDM_PhoneNum, MDM_phone2, MDM_PhoneLen );
	    MDM_cdon = MDM_cdon2;	// in 10ms
	    }
	  else
	    {
	    // end of dial-up
	    mdmStatusCode = mdmFailed;
	    return;
	    }
	  }

	// ATZ
	printf("ATZ write_atc\n");
	mdmCountTX = ( sizeof ( dial_time2 ) )-1;	// ATZ<cr> ATW2<cr>
	eRetCode = cnxt_modem_write_atc( ( char * ) dial_time2, &mdmCountTX, 300 );
	if( eRetCode == MODEM_ERROR )
	  {
	  mdmStatusCode = mdmFailed;
	  return;
	  }

	//ï¿½]ï¿½wmodem country code (???)
	printf("set country code \n");
	mdmCountTX = ( sizeof ( dial_atgci ) ) + 2;
	sprintf( (char *) dial_country, "%s%X%c", dial_atgci, countryCode, '\r' );
	printf("dial_country=%s\n",dial_country);
	eRetCode = cnxt_modem_write_atc( ( char * ) dial_country, &mdmCountTX, 3000 );
	if( eRetCode == MODEM_ERROR )
	  {
	  mdmStatusCode = mdmFailed;
	  return;
	  }

	// Normal connection dial
	if( (mdmMode == mdmBYPASS) || (mdmMode == mdmSDLC) )
	  {
	  //ï¿½Ç°e"ATZ\rAT+MS=v22\rAT$F2\rAT%C0\rAT\\N0\rATS9=1\rAT+ES=6,,8\rATS17=9\r"
	  // mdmCountTX = ( sizeof ( dial_time1 ) )-1;
	  //mdmCountTX = iLen;	// 2010-06-09
	  mdmCountTX = iLen-1;	// 2020-08-24
	  printf("Normal connection dial \n");
	  eRetCode = cnxt_modem_write_atc( ( char * ) dial_time1, &mdmCountTX, 300 );
	  if( eRetCode == MODEM_ERROR )
	    {
	    mdmStatusCode = mdmFailed;
	    return;
	    }
	    
	  // ATW2
	  //mdmCountTX = sizeof( dial_atw2 );
	  mdmCountTX = sizeof( dial_atw2 )-1;//2020-08-24
	  printf("ATW2\n");
	  eRetCode = cnxt_modem_write_atc( ( char * ) dial_atw2, &mdmCountTX, 300 );
	  if( eRetCode == MODEM_ERROR )
	    {
	    mdmStatusCode = mdmFailed;
	    return;
	    }
	  }
	else
	  {
	  if( mdmMode == mdmREFERRAL )
	    {
	    mdmReferralFlag = 1;
	    mdmOffHookInstrFlag = 0;
	    mdmVtroff = 0;
	    
//	    mdmCountTX = ( sizeof ( dial_time2 ) )-1-5;	// only ATZ<cr>
//	    eRetCode = cnxt_modem_write_atc( ( char * ) dial_time2, &mdmCountTX, 300 );
//	    if( eRetCode == MODEM_ERROR )
//	      {
//	      mdmStatusCode = mdmFailed;
//	      return;
//	      }
	    }
	  else
	    {
//	    //ï¿½Ç°e"ATZ\r"
//	    mdmCountTX = ( sizeof ( dial_time2 ) )-1;
//	    eRetCode = cnxt_modem_write_atc( ( char * ) dial_time2, &mdmCountTX, 300 );
//	    if( eRetCode == MODEM_ERROR )
//	      {
//	      mdmStatusCode = mdmFailed;
//	      return;
//	      }
	    
	    mdmStatusCode = mdmFailed;
	    return;
	    }
	  }

/*
	//ï¿½]ï¿½wmodem country code (???)
	mdmCountTX = ( sizeof ( dial_atgci ) ) + 2;
	sprintf( (char *) dial_country, "%s%X%c", dial_atgci, countryCode, '\r' );
	eRetCode = cnxt_modem_write_atc( ( char * ) dial_country, &mdmCountTX, 300 );
	if( eRetCode == MODEM_ERROR )
	  {
	  mdmStatusCode = mdmFailed;
	  return;
	  }
*/
	// configure synchronous access submode
//	mdmCountTX = strlen( dial_time3 );
//	eRetCode = cnxt_modem_write_atc( ( char * ) dial_time3, &mdmCountTX, 300 );
//	if( eRetCode == MODEM_ERROR )
//	  {
//	  mdmStatusCode = mdmFailed;
//	  return;
//	  }

/*
	//ï¿½ï¿½ï¿½Ç¿ï¿½Ò¦ï¿½ï¿½ï¿½ASYNCï¿½É³]ï¿½wbaud rate
	if( mdmMode == mdmBYPASS && baudRate !=0  )
	  {
	  mdmCountTX = ( ( sizeof ( dial_ms ) )-1 )+baudLeng+1 ;
	  sprintf( (char *) dial_baud, "%s%.*s%c", dial_ms, baudLeng, baudMode, '\r' );
	  eRetCode = cnxt_modem_write_atc( ( char * )dial_baud, &mdmCountTX, 300 );
	  if( eRetCode == MODEM_ERROR )
	    {
	    mdmStatusCode = mdmFailed;
	    return;
	    }
	  }
*/

	// TEST ONLY
//	mdmStatusRLSDS = 1;
//	SdlcTxSNRM();
//	goto EXIT;
	
	//ï¿½Ì·ï¿½APï¿½ï¿½Jï¿½ï¿½ï¿½qï¿½Ü¸ï¿½ï¿½Xï¿½iï¿½æ¼·ï¿½ï¿½
	//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½rï¿½ï¿½ï¿½ï¿½×¬ï¿?<ATD><kbuf->len1><\r>
	// mdmCountTX = 3 + MDM_PhoneLen + 1;//ï¿½pï¿½ï¿½ï¿½`ï¿½rï¿½ï¿½jï¿½p
	mdmCountTX = 3 + MDM_PhoneLen + 1 + strlen(CONNECT_str);//ï¿½pï¿½ï¿½ï¿½`ï¿½rï¿½ï¿½jï¿½p
	sprintf( (char *) dial_phone, "%s%.*s%c", dial_cmd, MDM_PhoneLen, MDM_PhoneNum, '\r' );
	printf("mdmCountTX=%d\n",mdmCountTX);
	printf("strlen(CONNECT_str)=%d\n",strlen(CONNECT_str));
	eRetCode = cnxt_modem_write_atc( ( char * )dial_phone, &mdmCountTX, 0 );
EXIT:	
	MDM_ResetAuxTOB(3);
	
	if( (mdmMode == mdmSDLC) && (baudRate == MDM_9600) )
	  os_mdm56k_flow_ctrl = TRUE;
}

// ---------------------------------------------------------------------------
// FUNCTION: MODEM interrupt service routine.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	MDM_FlowControlIsr( void )
{
while(1){
	uint64_t COUNT;
	if(BSP_TMR_GetTick(pMdmFlowControl,&COUNT)>0){
	switch( MDM_FlowControlState )
	      {
	      case MDM_STATE_IDLE:
	      case MDM_STATE_INIT:
	           break;
	           	      
	      case MDM_STATE_WAIT_CD:
	      	   
	      	   if( MDM_cdon )
	      	     {
	      	     MDM_cdon--;
		     }
	      	   
	           break;

	      case MDM_STATE_WAIT_TOUT:
	      	   
	      	   if( MDM_rdpi )
	      	     {
	      	     MDM_rdpi--;
		     }
	      	   
	      	   break;
	      }
	  }
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: MODEM flow control ROS task.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	*MDM_FlowControlTask( void )
{
	
	while(1)
	{
		uint64_t COUNT;
		if(BSP_TMR_GetTick(pMdmFlowControlTask,&COUNT)>0)
		{
	switch( MDM_FlowControlState )
	      {
	      case MDM_STATE_IDLE:
	      
	           break;
	           
	      case MDM_STATE_INIT:
		  
		  
		   mdmStatusCode = mdmConnecting;
		   MDM_FCS_Init();	   
		   
		   if( mdmMode != mdmREFERRAL )
		     MDM_FlowControlState = MDM_STATE_WAIT_CD;
		   else
		     MDM_FlowControlState = MDM_STATE_IDLE;
		     
		   break;
	      
	      case MDM_STATE_WAIT_CD:
	      	   
	      	   if( mdmEscOpenFlag )
	      	     {
//	      	     mdmStatusCode = mdmFailed;		// 2011-08-18, keep the status from "read_from_modem_task()"
	      	     
	      	     MDM_FlowControlState = MDM_STATE_IDLE;
	      	     break;
	      	     }
	      	   
	      	   if( mdmStatusCode == mdmConnected )
	      	     MDM_FlowControlState = MDM_STATE_IDLE;
	      	   else
	      	     {
	      	     if( (MDM_cdon == 0) || (mdmDialConFlag) )
	      	       {
	      	       mdmDialConFlag = 0;	// clear redial flag
	      	       
	      	       if( (MDM_attempt1) || (MDM_attempt2) )
	      	         {
	      	         MDM_HangUp();
	      	         
	      	         mdmStatusCode = mdmRedialing;
	      	         
					 MDM_rdpi = MDM_rdpi_bak;	// retrieve RDPI
	      	         MDM_FlowControlState = MDM_STATE_WAIT_TOUT;
	      	         MDM_FlowControlNextState = MDM_STATE_INIT;
	      	         }
	      	       else
	      	         {
	      	         mdmStatusCode = mdmFailed;	// out of retry
	      	         
	      	         MDM_FlowControlState = MDM_STATE_IDLE;
	      	         }
	      	       }
	      	     }
	      	   
	           break;

	      case MDM_STATE_WAIT_TOUT:
	      	   
//	      	   if( MDM_rdpi )
//	      	     {
//	      	     MDM_rdpi--;
	      	     
	      	     if( MDM_rdpi == 0 )
	      	       {
	      	       MDM_FlowControlState = MDM_FlowControlNextState;
	      	       }
//		     }
	      	   
	      	   break;
	      }
	
		//NosSleep(20);
		} // while(1)
	}
}

// ---------------------------------------------------------------------------
void	MDM_ResetAuxTOB( UINT tob )
{
ULONG	IntState;


//	IntState = BSP_DisableInterrupts( BSP_INT_MASK );
	
	os_AUX_Para[COM2].Tob = tob;		// 3*10ms
	os_AUX_ParaBak[COM2].Tob = tob;		//
	
//	BSP_RestoreInterrupts( IntState );
}

// ---------------------------------------------------------------------------
/*
UCHAR	MDM_OpenAuxPort2( API_HOST *ht )
{
ULONG	baud;
ULONG	databits = 0;
ULONG	stopbits;
ULONG	parity;


	switch( ht->baud & 0x03E0 )
	      {
	      case COM_300:
	           baud = 300;
	           break;
	      case COM_600:
	           baud = 600;
	           break;
	      case COM_1200:
	           baud = 1200;
	           break;
	      case COM_2400:
	           baud = 2400;
	           break;
	      case COM_4800:
	           baud = 4800;
	           break;
	      case COM_9600:
	           baud = 9600;
	           break;
	      case COM_19200:
	           baud = 19200;
	           break;
	      case COM_38400:
	           baud = 38400;
	           break;
	      case COM_57600:
	           baud = 57600;
	           break;
	      case COM_115200:
	           baud = 115200;
	           break;
	      default:
	           baud = 9600;
	      }

	switch( ht->baud & 0x0003 )
	      {
	      case COM_CHR7:
	           databits = 7;
	           break;
	      case COM_CHR8:
	           databits = 8;
	           break;
	      defautl:
	           databits = 8;
	      }
	
	switch( ht->baud & 0x0004 )
	      {
	      case COM_STOP1:
	           stopbits = 1;
	           break;
	      case COM_STOP2:
	           stopbits = 2;
	           break;
	      default:
	           stopbits  = 1;
	      }
	      
	switch( ht->baud & 0x0018 )
	      {
	      case COM_NOPARITY:
	           parity = UART_PARITY_NONE;
	           break;
	      case COM_ODDPARITY:
	           parity = UART_PARITY_ODD;
	           break;
	      case COM_EVENPARITY:
	           parity = UART_PARITY_EVEN;
	           break;
	      default:
	           parity = UART_PARITY_NONE;
	      }

	pMdmUart = BSP_UART_Acquire( COM1, NULLPTR );
	if( pMdmUart )
	  {
	  pMdmUart->Mode = UART_MODE_IRQ;
	  pMdmUart->Baud = baud;
	  pMdmUart->DataBits = databits;
	  pMdmUart->StopBits = stopbits;
	  pMdmUart->Parity = parity;
	  pMdmUart->BufferSize = 2048;
	  pMdmUart->FlowControl = UART_FLOW_NONE;
//	  pMdmUart->RxCallLevel = 1;
//	  pMdmUart->TxCallCntrl = TRUE;
	  if( BSP_UART_Start( pMdmUart ) != BSP_SUCCESS )
	    return( FALSE );
	  }
	
	BSP_UART_SetModemControl( pMdmUart, MCTL_DTR | MCTL_RTS );	// enable DTR & RTS
	
	BSP_GPIO_OUT_1(pCOM1_SEL);	// switch to MODEM port
	
	return( TRUE );
}
*/

// ---------------------------------------------------------------------------
UCHAR	MDM_OpenAuxPort( void )
{
API_AUX	pMdmPort;
UINT	dhn_mdm;
UINT	iBaud;


	if( os_DHN_MDM56K != 0 )
	  return( apiOK );	// already opened

	//  open modem port (COM2)
	pMdmPort.Mode = auxBYPASS;
	iBaud = COM_57600 + COM_CHR8 + COM_NOPARITY + COM_STOP1; // using fixed 56K communication speed with MODEM
	pMdmPort.Baud = iBaud;		//
	pMdmPort.Tob = 300;		// 3*10ms
	pMdmPort.Tor = 300;		// 300*10ms
	pMdmPort.Acks = 0;		//
	pMdmPort.Resend = 0;		//
	pMdmPort.BufferSize = 2048;	// buffer size
	dhn_mdm = api_aux_open( COM2, pMdmPort );
	if( dhn_mdm == apiOutOfService )
	  {
	  os_DHN_MDM56K = 0;
	  //BSP_GPIO_OUT_0(pCOM1_SEL);		// switch COM1 port to external device
	  
	  return( apiFailed );
	  }

	//BSP_GPIO_OUT_1(pCOM1_SEL);		// switch COM1 prot to internal MODEM (MARKED: if using external MODEM)
	
	os_DHN_MDM56K = dhn_mdm;
	// printf("os_DHN_MDM56K=%d\n",os_DHN_MDM56K);
	return( apiOK );
}

// ---------------------------------------------------------------------------
UCHAR	MDM_CloseAuxPort( void )
{
UCHAR	result;

	// printf("os_DHN_MDM56K=%d\n",os_DHN_MDM56K);
	result = api_aux_close( os_DHN_MDM56K );
	
	os_DHN_MDM56K = 0;			// reset dhn	
	//BSP_GPIO_OUT_0(pCOM1_SEL);		// switch COM1 port to external device//20200807
																			  //comment out because it's not necessary in AS350+
	return( result );
}

// ---------------------------------------------------------------------------
/*
ULONG	MDM_GetStatus( void )
{
	BSP_UART_GetModemStatus( os_pAUX2 );
	return( os_pAUX2->ModemStatus );
}
*/
// ---------------------------------------------------------------------------
/*
ULONG	MDM_WaitDCD_On( ULONG tenms )
{
	while(1)
	     {
	     // wait DCD ON
	     BSP_GPIO_READ( pMDM_DCD );
	     if( pMDM_DCD->Data == 0 )	// low active
	       {
	       // set RTS ON
	       BSP_UART_SetModemControl( os_pAUX2, MCTL_DTR | MCTL_RTS );
	       
	       do{
	         // wait CTS ON
	         if( !(MDM_GetStatus() & MSR_CTS) )
	           {
	           if( tenms )
	             tenms--;
	           //NosSleep(1);
	           BSP_Delay_n_ms(10);
	           }
	         else
	           return( TRUE );
	         } while( tenms );
	         
	       return( FALSE );
	       }
	     else
	       {
	       if( tenms-- )
	         //NosSleep(1);
	         BSP_Delay_n_ms(10);
	       else
	         return( FALSE );
	       }
	     }
}

// ---------------------------------------------------------------------------
void	MDM_WaitDCD( void )
{
	// wait DCD OFF
	while(1)
	     {
	     BSP_GPIO_READ( pMDM_DCD );
	     if( pMDM_DCD->Data == 0 )	// low active
	       break;
	     };
}
*/
// ---------------------------------------------------------------------------
//void	ICB_SetRTS_ON( void )
//{
//UCHAR	temp[16];
//
//	temp[0] = 2;
//	temp[1] = 0;
//	temp[2] = 0x19;
//	temp[3] = 0x43;
//	api_aux_txstring( os_DHN_MDM56K, temp );
//}

// ---------------------------------------------------------------------------
/*
void	MDM_SetRTS( void )
{
	if( !os_mdm56k_flow_ctrl )
	  return;
	  
	// wait DCD OFF
	while(1)
	     {
	     BSP_GPIO_READ( pMDM_DCD );
	     if( pMDM_DCD->Data != 0 )	// low active
	       break;
	     };
	  
	// set RTS ON
	BSP_UART_SetModemControl( os_pAUX2, MCTL_DTR | MCTL_RTS );
//	ICB_SetRTS_ON();
	
	// wait CTS ON
	while( !(MDM_GetStatus() & MSR_CTS) );
	
//	BSP_Delay_n_ms(150);
}
*/

// ---------------------------------------------------------------------------
/*
void	MDM_ClearRTS( void )
{
	if( !os_mdm56k_flow_ctrl )
	  return;
	
	// make sure that all the data have been sent
//	BSP_Delay_n_ms(20);
	
	// set RTS OFF
	BSP_UART_SetModemControl( os_pAUX2, MCTL_DTR );
	
	// wait DCD ON
	while(1)
	     {
	     BSP_GPIO_READ( pMDM_DCD );
	     if( pMDM_DCD->Data == 0 )	// low active
	       break;
	     };
}
*/
// ---------------------------------------------------------------------------
// FUNCTION: To enable the modem device and establish the link to remote host.
// INPUT   : atcmd
//	     UCHAR length; // length of user-assigned AT-command. (max. 30 bytes)
//	     char  data[length]; // optional, user-assigned AT-command string,
//	                         // if "length" != 0.
//	     sbuf
//	     refer to API_HOST defintion.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT	apk_mdm_open( UCHAR *atcmd, API_HOST *sbuf )
{
//ULONG	FCTL;
API_AUX	pMdmPort;

	//  open modem port (COM2)
	pMdmPort.Mode = auxBYPASS;
	pMdmPort.Baud = sbuf->baud;
	pMdmPort.Tob = 300;		// 300*10ms
	pMdmPort.Tor = 300;		// 300*10ms
	pMdmPort.Acks = 0;
	pMdmPort.Resend = 0;
	pMdmPort.BufferSize = 2048;
	os_DHN_MDM56K = api_aux_open( COM2, pMdmPort );
	printf("os_DHN_MDM56K =%d\n",os_DHN_MDM56K );
	if( os_DHN_MDM56K == apiOutOfService )
	  return( apiFailed );
	
//	BSP_GPIO_OUT_1(pCOM1_SEL);	// switch to MODEM port (TEMP SOLUTION: using external MODEM)
	
	if( MDM_OpenAuxPort() != apiOK )
	  return( apiFailed );  
	mdmStatusRLSDS = 0;
	mdmDialConFlag = 0;

	// MDM_ResetAuxTOB(300);
	MDM_ResetAuxTOB(300);//TOB fix to 3ms to prevent from data read by other timer 
	
	// backup MODEM parameters
	MDM_attempt1 = sbuf->attempt1;
	MDM_attempt2 = sbuf->attempt2;
	if( (MDM_attempt1 == 0) && (MDM_attempt2 == 0) )
	  return( apiFailed );

	MDM_rdpi = (sbuf->rdpi)*100;	// in 10ms
	if( MDM_rdpi < 300 )
	  MDM_rdpi = 300;	// default min 3 sec
	MDM_rdpi_bak = MDM_rdpi;
	
	MDM_cdon1 = (sbuf->cdon1)*100;
	MDM_cdon2 = (sbuf->cdon2)*100;
	
	MDM_country = sbuf->country;
	MDM_address = sbuf->address;
	MDM_baud = sbuf->baud;
	MDM_len1 = sbuf->len1;
	MDM_len2 = sbuf->len2;
	
	if( (MDM_len1 != 0) && (MDM_len1 <= sizeof(MDM_phone1)) )
	  memmove( MDM_phone1, sbuf->phone1, MDM_len1 );
	  
	if( (MDM_len2 != 0) && (MDM_len2 <= sizeof(MDM_phone2)) )
	  memmove( MDM_phone2, sbuf->phone2, MDM_len2 );
	
	mdmMode = sbuf->mode;
	if( ( mdmMode != mdmBYPASS ) && ( mdmMode != mdmSDLC ) && ( mdmMode != mdmREFERRAL ) )
	  return( apiFailed );

	// Create a timer thread to follow up the subsequent processes
/*	
	pMdmFlowControl = BSP_TMR_Acquire( BSP_LARGE_TIMER, (void *)MDM_FlowControlIsr );
	if( pMdmFlowControl == NULLPTR )
	  return( apiFailed );
	  
	pMdmFlowControl->Count    = 1;
	pMdmFlowControl->Compare  = BSP_GetClock( TRUE, 2048 );
	pMdmFlowControl->Compare *= 10; // 10ms;
	pMdmFlowControl->Compare /= 1000;
	pMdmFlowControl->Control  = TMR_PRES_DIV_2048 | TMR_TEN | TMR_MODE_RELOAD;
*/
	
	pMdmFlowControl = BSP_TMR_Acquire(1);//10ms timer
	if( api_mdm_createtask() == apiOK )	//create background task
	  {
	  MDM_FlowControlState = MDM_STATE_INIT;
	  mdmStatusCode = mdmConnecting;
	//   printf("^^^^^^^^^^^^^^^^^^^^os_DHN_MDM56K=%d\n",os_DHN_MDM56K);
	  return( os_DHN_MDM56K );
	  }
	else
	  {
	  mdmStatusCode = mdmFailed;
	  return( apiFailed );
	  }
}

//------------------------------------------------------------------------------

//******************************************************************************
// ï¿½{ï¿½ï¿½ï¿½Ò²Õ½sï¿½ï¿½:mdm_004_001
// ï¿½Æµ{ï¿½ï¿½ï¿½Wï¿½Ù¡G api_mdm_hangup
// ï¿½\ï¿½ï¿½Gï¿½ï¿½ï¿½_ï¿½qï¿½ï¿½
//******************************************************************************

UCHAR api_mdm_hangup( void )
{
  UCHAR           hang_up1[]={"ATH\r"};
  UCHAR           hang_up2[]={"+++\r"};
  UCHAR		  no_carrier[] = {"\r\nNO CARRIER\r\n"};
  UCHAR     	  api_status = apiOK;
  MODEM_STATUS    eRetCode;
  int		  Avail;
  UINT		  iLen;
  UCHAR		  temp[128];


  mdmTimeoutFlag = 0;

  mdmCanTx = FALSE;
  for ( ;mdmRxqueue.front != NULL; mdm_queue_remove( &mdmRxqueue ) )
  ;

  if (mdmMode == mdmSDLC)
    {
      UCHAR ecs[2];
      ecs[0] = 0x19;
      ecs[1] = 0xBA;
      mdmCountTX = 2;
      eRetCode = cnxt_modem_write( ( char * )ecs, &mdmCountTX );
      if (eRetCode != MODEM_OK)
        {
          DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error(%d)\n\r", eRetCode );
        }

//      DEBUGPRINTF( "!!!!!Send ecs \r\n");
//      mdmCountTX = 4;
//      eRetCode = cnxt_modem_write_atc( ( char * )hang_up1, &mdmCountTX, 300 );
//      if (eRetCode != MODEM_OK)
//        {
//          DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error(%d)\n\r", eRetCode );
//        }
//
//      DEBUGPRINTF( "!!!!!Hang up\r\n");
    }
  else
    {
      // issue escape sequence +++
      mdmEscSequenceFlag = 1;
      mdmCountTX = strlen(hang_up2);
      eRetCode = cnxt_modem_write_atc( ( char * )hang_up2, &mdmCountTX, 0xFFFFFFFF );
      mdmEscSequenceFlag = 0;
      if( eRetCode != MODEM_OK )
        goto HANGUP_EXIT;
        
      // issue ATH to hang up
      mdmCountTX = strlen(hang_up1);
      eRetCode = cnxt_modem_write_atc( ( char * )hang_up1, &mdmCountTX, 300 );
      if( eRetCode == MODEM_OK )
	{
	mdmStatusRLSDS = 0;
	mdmStatusCode = mdmLineDropped;
	}
	
      goto HANGUP_EXIT;
    }


  //pTryconnectTimer = BSP_TMR_Acquire( BSP_LARGE_TIMER, mdm_TryconnectTimerLisr );
  pTryconnectTimer = BSP_TMR_Acquire( 1 );

  if ( pTryconnectTimer )
    {
      //ï¿½Ø¥ß½Tï¿½{Timeoutï¿½Oï¿½_ï¿½oï¿½Íªï¿½Timer
      // pTryconnectTimer->Count  =1;
      // pTryconnectTimer->Compare = BSP_GetClock(TRUE, 2048/5);//ï¿½]ï¿½wTimeoutï¿½É¶ï¿½
      // pTryconnectTimer->Control = TMR_PRES_DIV_2048 | TMR_TEN | TMR_MODE_RELOAD;
      //BSP_TMR_Start( pTryconnectTimer );//Timerï¿½}ï¿½lï¿½pï¿½ï¿½
      BSP_TMR_Start( mdm_TryconnectTimerLisr,pTryconnectTimer );//Timerï¿½}ï¿½lï¿½pï¿½ï¿½
    }
  else
    {
      api_status=apiFailed;
      return( api_status );
    }

  //ï¿½ï¿½ï¿½Ýªï¿½ï¿½ì°»ï¿½ï¿½ï¿½ï¿½ï¿½_ï¿½u
//  while ( mdmStatusRLSDS == 1 && mdmTimeoutFlag == 0 )
//    {
//      OS_SLEEP( 1 );
//    }
//  if ( mdmTimeoutFlag == 1 )
//    {
//      api_status = apiFailed;
//      mdmTimeoutFlag = 0;
//    }

	
          // waiting for <cr><lf>NO CARRIER<cr><lf> received in "read_from_modem_task()"
          api_status = apiFailed;
          while( mdmStatusRLSDS == 1 && mdmTimeoutFlag == 0 )
               {
               if( mdmStatusCode != mdmLineDropped )
        	 NosSleep(1);
               else
        	 {
        	 // send ATH<cr>
        	 mdmCountTX = strlen(hang_up1);
        	 if( cnxt_modem_write_atc( ( char * )hang_up1, &mdmCountTX, 300 ) == MODEM_OK )
        	   api_status = apiOK;	// line has been dropped OK
        	     
        	 break;
        	 }
               }
  
  //BSP_TMR_Release( pTryconnectTimer );//ï¿½ï¿½ï¿½ï¿½Timer
  BSP_TMR_Stop( pTryconnectTimer );
  pTryconnectTimer = NULLPTR;

HANGUP_EXIT:

  //ï¿½Nqueueï¿½Mï¿½ï¿½
  for ( ;mdmRxqueue.front != NULL; mdm_queue_remove( &mdmRxqueue ) )
    ;
  for ( ;mdmTxqueue.front != NULL; mdm_queue_remove( &mdmTxqueue ) )
    ;

  return api_status;
}

// ---------------------------------------------------------------------------
// FUNCTION: To disable the modem device and disconnect the link to host.
// INPUT   : dhn
//	     The specified device handle number.
//	     0x00 = to close all opened tasks.
//
//	     delay
//	     Time before disconnecting the link to host.
//	     0x0000 = disconnect immediately.
//	     0xFFFF= ignore the last "delay" setting and keep the link with host if it exists.
//	     Others  = delay time. (unit: depends on system timer resolution, default 10ms.)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	apk_mdm_close( UINT delay )
{
UCHAR	api_status = apiOK;
printf("^^^^^^^^^^^^^apk_mdm_close\n");
	
	mdmReferralFlag = 0;
	
	os_mdm56k_flow_ctrl = FALSE;
	
	if(delay == 0xFFFF)
	  return( apiOK );
	
	if( mdmStatusRLSDS )	// connected?
	  {
		  //¸õ¹L¤â°Ê±¾±¼¹q¸Ü¨ç¦¡api_mdm_hangup()¡A¦]¬°µwÅémodem·|¦Û°Ê±¾±¼¡C
	//   api_status = api_mdm_hangup();
//	  BSP_Delay_n_ms(100);
	  api_status = apiOK;
	  if( api_status == apiOK )
	    {
	    mdmStatusCode = mdmIdle;
	    mdmMode = mdmREFERRAL;
	    }
	  else
	    mdmStatusCode = mdmIdle;
	  }
	else
	  {
		  //¸õ¹L¤â°Ê±¾±¼¹q¸Ü¨ç¦¡api_mdm_hangup()¡A¦]¬°µwÅémodem·|¦Û°Ê±¾±¼¡C
	//   api_status = api_mdm_hangup();
//	  BSP_Delay_n_ms(100);
	  
	  mdmStatusCode = mdmIdle;
	  mdmMode = mdmREFERRAL;
	  }
	int result=0;
	
	result=BSP_TMR_Stop( pMdmFlowControl );	// release flow control task
	printf("TMR_STOP result=%d\n",result);
	result=BSP_TMR_Stop( pMdmFlowControlTask );	// release flow control task
	printf("TMR_STOP result=%d\n",result);
	result=BSP_TMR_Stop( pReadFromModemTask );	// release flow control task
	printf("TMR_STOP result=%d\n",result);
	
	MDM_CloseAuxPort();			// release COM2 port
	
	mdmStatusRLSDS = 0;
	
	return( api_status );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the connection status from modem device.
// INPUT   : dhn
//	     The specified device handle number.
// OUTPUT  : dbuf
//	     Modem status byte.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR debug_mdm_status_bak=0;
UCHAR	apk_mdm_status( UCHAR *dbuf )
{
	*dbuf = mdmStatusCode;
	if(mdmStatusCode!=debug_mdm_status_bak)
		printf("mdm status=%d\n",mdmStatusCode);
	debug_mdm_status_bak=mdmStatusCode;
	return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: To determine whether data is ready for input from the modem device.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//           UINT  length ;     // length of the received data string.
// RETURN  : apiReady
//           apiNotReady
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	apk_mdm_rxready( UCHAR *dbuf )
{
UCHAR	api_status = apiNotReady;
UINT	DataSize;


  //DEBUGPRINTF( "SemAcquire!rxready \r\n" );
  if ( mdm_empty ( &mdmRxqueue )  )//ï¿½Pï¿½_RX queueï¿½Oï¿½_ï¿½ï¿½ï¿½ï¿½
    {
      // DEBUGPRINTF( "Rx queue is empty! \r\n" );
      return api_status;
    }
  else
    {
      MODEM_BUFFER q = mdmRxqueue.front;
      DataSize = 0;
      //ï¿½pï¿½ï¿½Rxqueueï¿½ï¿½ï¿½ï¿½ï¿½ï¿½?¤jï¿½p
      for ( ;q!=NULL; q=q->next )
        {
          DataSize += q->buffer_size;
        }

      dbuf[0] = ( UCHAR )DataSize;
      dbuf[1] = ( UCHAR )( DataSize>>8 );

      //DEBUGPRINTF( "Rx total  size is %d \r\n", dbuf[0]|( dbuf[1]<<8 ) );

      api_status = apiReady;
    }

  return api_status;
}
/*
UCHAR	apk_mdm_rxready( UCHAR *dbuf )
{
UCHAR	api_status = apiNotReady;
UINT	DataSize;
BSP_UART *pUart;

        pUart = AUX_CheckDHN( os_DHN_MDM56K );
	if( pUart == NULLPTR )
	  return( apiFailed );
	DataSize = BSP_UART_Read( pUart, os_AUX_RxDataBuffer[pUart->UartNum].Data, 2048 );
  //DEBUGPRINTF( "SemAcquire!rxready \r\n" );
  if ( DataSize==0 )//ï¿½Pï¿½_RX queueï¿½Oï¿½_ï¿½ï¿½ï¿½ï¿½
    {
	  os_AUX_RxDataBuffer[pUart->UartNum].Len=0;
      //DEBUGPRINTF( "Rx queue is empty! \r\n" );
      return api_status;
    }
  else
    {
      // MODEM_BUFFER q = mdmRxqueue.front;
      // DataSize = 0;
      //ï¿½pï¿½ï¿½Rxqueueï¿½ï¿½ï¿½ï¿½ï¿½ï¿½?¤jï¿½p
      // for ( ;q!=NULL; q=q->next )
        // {
          // DataSize += q->buffer_size;
        // }
	  os_AUX_RxDataBuffer[pUart->UartNum].Len=DataSize;
      dbuf[0] = ( UCHAR )DataSize;
      dbuf[1] = ( UCHAR )( DataSize>>8 );

      //DEBUGPRINTF( "Rx total  size is %d \r\n", dbuf[0]|( dbuf[1]<<8 ) );

      api_status = apiReady;
    }

  return api_status;
}
*/
// ---------------------------------------------------------------------------
// FUNCTION: To read the incoming data string from modem receiving block-stream buffer.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//           UINT  length ;     	// length of the received data string.
//	     UCHAR data[length] ;	// data string.	
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	apk_mdm_rxstring( UCHAR *dbuf )
{
UCHAR	api_status = apiFailed;
UCHAR	*i = &dbuf[2];
UINT	DataSize;

  if ( mdm_empty ( &mdmRxqueue )  )//ï¿½Pï¿½_RX queueï¿½Oï¿½_ï¿½ï¿½ï¿½ï¿½
    {
      //DEBUGPRINTF( "Rx queue is empty! \r\n" );
      dbuf[0] = 0;
      dbuf[1] = 0;


      api_status = apiOK;
      return api_status;
    }
  else
    {
      MODEM_BUFFER q = mdmRxqueue.front;
      DataSize = 0;
      //ï¿½pï¿½ï¿½Rxqueueï¿½ï¿½ï¿½ï¿½ï¿½ï¿½?¤jï¿½p
      for ( ;q != NULL; q = q->next )
        {
          DataSize += q->buffer_size;
        }

      dbuf[0] = (UCHAR)DataSize;
      dbuf[1] = (UCHAR)(DataSize>>8);

      //DEBUGPRINTF( "Rx total  size is %d \r\n", dbuf[0]|( dbuf[1]<<8 ) );

      q = mdmRxqueue.front;

      for (;mdmRxqueue.front != NULL; mdm_queue_remove( &mdmRxqueue ) )
        {
          memcpy( i, mdmRxqueue.front->buffer , mdmRxqueue.front->buffer_size );
          i += (mdmRxqueue.front->buffer_size);
        }

      api_status = apiOK;

    }



  return api_status;
}
/*
UCHAR	apk_mdm_rxstring( UCHAR *dbuf )
{
UCHAR	api_status = apiFailed;
UCHAR	*i = &dbuf[2];
UINT	DataSize;
BSP_UART *pUart;

    pUart = AUX_CheckDHN( os_DHN_MDM56K );
	if( pUart == NULLPTR )
	  return( apiFailed );
  
  if ( os_AUX_RxDataBuffer[pUart->UartNum].Len==0  )//ï¿½Pï¿½_RX queueï¿½Oï¿½_ï¿½ï¿½ï¿½ï¿½
    {
      //DEBUGPRINTF( "Rx queue is empty! \r\n" );
      dbuf[0] = 0;
      dbuf[1] = 0;


      api_status = apiOK;
      return api_status;
    }
  else
    {
      DataSize = os_AUX_RxDataBuffer[pUart->UartNum].Len;
 
      dbuf[0] = (UCHAR)DataSize;
      dbuf[1] = (UCHAR)(DataSize>>8);
	  memmove( &dbuf[2], os_AUX_RxDataBuffer[pUart->UartNum].Data, DataSize );
      api_status = apiOK;

    }



  return api_status;
}
*/
// ---------------------------------------------------------------------------
// FUNCTION: To determine whether modem is ready to transmit the next data string.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : none.
// RETURN  : apiReady
//           apiNotReady
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	apk_mdm_txready( void )
{
UCHAR	api_status = apiFailed;
	printf("mdmCanTx =%d\tmdmStatusRLSDS=%d\n",mdmCanTx,mdmStatusRLSDS);
  if ( mdmCanTx == FALSE || mdmStatusRLSDS==0 )//·í modemªºCTS¬°low©M³s½u©|¥¼«Ø¥ßªº®É­Ô¡ATxª¬ºA¬°not ready
    {
      api_status = apiNotReady;
    }
  else
    {
      api_status = apiReady;
    }
  return api_status;
}

// ---------------------------------------------------------------------------
// FUNCTION: To write the outgoing data string to modem transmitting stream buffer.
// INPUT   : dhn
//           The specified device handle number.
//           sbuf
//           UINT  length ;     	// length of data string to be transmitted.
//	     UCHAR data[length] ;	// data string.	
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	apk_mdm_txstring( UCHAR *sbuf )
{
UCHAR	api_status = apiOK;
MODEM_STATUS   eRetCode;
unsigned int TXLeng;
TXLeng = sbuf[0]|( sbuf[1]<<8 );
  //Add node
  if ( mdmMode == mdmSDLC )
    {
      mdm_queue_insert( &mdmTxqueue, &(sbuf[2]), TXLeng, &api_status );

    }
  else
    {
      mdmCountTX= TXLeng;
      //Add node
      mdm_txqueue_insert( &mdmTxqueue, &( sbuf[2] ), mdmCountTX, &api_status );
      if (api_status == apiFailed)
        {
          return api_status;
        }
      pmdmTxBuf = mdmTxqueue.front->buffer;
	  eRetCode = cnxt_modem_write( ( char * )pmdmTxBuf, &mdmCountTX );
      //memmove( pmdmTxBuf, 0x00, mdmCountTX );//2020-08-13 replace with memset in case of ggc "null argument where non-null required" warning
      memset( pmdmTxBuf, 0x00, mdmCountTX );
      if (eRetCode != MODEM_OK)
        {
          DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error(%d)\n\r", eRetCode );
        }
       
      mdmCountTX = 0;
      
      NosSleep( 2 );
      mdm_queue_remove( &mdmTxqueue );
    }

  return api_status;
}
