//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : DSS (for no file system)                                   **
//**  PRODUCT  : AS320B                                                     **
//**                                                                        **
//**  FILE     : Down.C                                               **
//**  MODULE   : DSS                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/06/15                                                 **
//**  EDITOR   : Richad Hu                                                  **
//**                                                                        **
//**  Copyright(C) 2009 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================

#include "POSAPI.h"
#include "bsp_uart.h"
#include "bsp_tmr.h"
// #include "bsp_mem.h"
#include "DownInterface.h"
#include "Down.h"
#include "redirtsk.h"
//#include "vfs.h"
#include "stdlib.h"
// #include "GlobalVar.h"



BSP_UART     pUartBackUp;//Backup the configuration of UART which will be released
extern BSP_UART	*glpUart;
extern UINT32	os_DSS_COM;
extern ULONG     DownReceiveSize;
extern UINT32	  DSS_recieveData;
UCHAR	DSS_APPbuffer[DSS_MAX_APP_SIZE];//10MB
UCHAR	BSP_UART_Rxreadyflag;//if UART read done
UCHAR	DssTelNum[23+1];		
UCHAR	DssRemoteIP[23+1];	
UCHAR	DssRemotePort[23+1];
UCHAR	DssPort[23+1];
UCHAR   DssIP[23+1];
UCHAR   DssGateway[23+1];
UCHAR   DssSubNetMask[23+1];
// extern BSP_BOOL EthInit();//For testing Lan
// extern BSP_BOOL EthStart();//For testing Lan
// extern BSP_BOOL API_EthTxString2(unsigned long length,unsigned char *string);
// extern BSP_BOOL API_EthConnect(unsigned short port,unsigned short rport,unsigned long rip);
// extern unsigned char* API_EthRxString();
// extern void API_EthDisconnect();

extern	void	LIB_LCD_Cls( void );
extern	void	LIB_LCD_Puts( UINT8 row, UINT8 col, UINT8 font, UINT8 len, UINT8 *str );
extern	UCHAR	api_lan_rxstring2( UCHAR dhn,UCHAR *dbuf );
extern	UCHAR	api_lan_rxstring3( UCHAR dhn,UCHAR *dbuf );
extern	UCHAR	api_lan_txstring2( UCHAR dhn, UCHAR *sbuf );
extern	UCHAR	api_lan_rxready2( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR	api_lan_rxready3( UCHAR dhn, UCHAR *dbuf );
extern	UINT32 BSP_UART_Rxstring( BSP_UART * pUart );
extern	UINT32 BSP_UART_Rxready( BSP_UART * pUart );
UCHAR   DSS_AUXdhn=0xFF;
UCHAR   DSS_LANdhn=0xFF;
UCHAR   DSS_MDMdhn=0xFF;
extern UCHAR		IfOpenedCom0;//to note whether uart port opened.	
extern UCHAR		IfOpenedCom1;	
extern UCHAR		IfOpenedCom2;	
volatile UCHAR MdmOpenTimerFlag =0;//for record the timeout of waitting connection of modem

void UartISR()
{
}

void mdmOpenTimerLisr()
{
  MdmOpenTimerFlag = 1;
}
// ---------------------------------------------------------------------------
// FUNCTION: api_down_open
//           Initialize the device used in download communiction
//           For uart : Release the original uart and require a new one
//           For modem and Lan: Check if the modem status is "Connected" or not
// INPUT   : TransMode ( 0: Uart, 1: modem ,2: lan)
// OUTPUT  : none.
// RETURN  : apiOK
//               apiFailed
// ---------------------------------------------------------------------------

UCHAR api_down_open( UCHAR TransMode )
{
  UCHAR					apiStatus=apiOK;
  UCHAR	sbuf[10];//modem
  UCHAR  dssTelNum[23+1];//For storing the Modem tel number
  UCHAR  dssLanRemoteIP[23+1];//For storing the remote IP
  UCHAR  dssLanRemotePort[23+1];//For storing the remote port
  UCHAR  dssLanPort[23+1];//For storing the port
  UCHAR    const message_modem_connecting[]={"Connecting..."};
  UCHAR    const message_modem_connected[]={"Connected!"};
  ULONG  remoteIPTmp;
  ULONG  remotePortTmp;
  ULONG  portTmp;
  API_LAN DSS_lan;
  UCHAR status;
//  API_HOST ht;//modem
  API_HOST ht[1];

  switch ( TransMode )
    {
    case ( DownUartMode ):
    {

      if ( glpUart )//uart port temporary use COM1
        {
          pUartBackUp.Mode = glpUart->Mode;
          pUartBackUp.Baud = glpUart->Baud;
          pUartBackUp.DataBits = glpUart->DataBits;
	        pUartBackUp.StopBits = glpUart->StopBits;
	        pUartBackUp.Parity = glpUart->Parity;
	        pUartBackUp.BufferSize = glpUart->BufferSize;
	        pUartBackUp.FlowControl = glpUart->FlowControl;
	   
          BSP_UART_Release( glpUart );
          glpUart = NULLPTR;
        }
//    pUart = BSP_UART_Acquire( BSP_UART_0, UartISR );
      glpUart = BSP_UART_Acquire( os_DSS_COM, UartISR );	// 2014-07-23

      glpUart->Mode        = UART_MODE_IRQ;
      glpUart->Baud        = 115200;
      glpUart->DataBits    = 8;
      glpUart->StopBits    = 1;
      glpUart->Parity      = UART_PARITY_NONE;
      glpUart->BufferSize  = 3072;//�C�ӿN���ʥ]�j�p�̤j��(2048+9)bytes
      glpUart->FlowControl = UART_FLOW_NONE;

      apiStatus = BSP_UART_Start( glpUart );
      	  
      break;
    }

#ifdef	_MODEM_ENABLED_

    case ( DownModemMode):
    {
      UCHAR linestate;
      BSP_TIMER * pmdmOpenTimer;


      memmove( dssTelNum, DssTelNum, sizeof(dssTelNum) );//for internal usage


      sbuf[0] = 0;
      ht->mode = mdmBYPASS;
      ht->address = 0x00;
      ht->baud = 0;
      ht->rdpi = 10;
      ht->attempt1 = 3;
      ht->cdon1 = 30;

      ht->len1 = dssTelNum[0]+1;
      ht->phone1[0] = 'T';

      memmove( &ht->phone1[1], &dssTelNum[1], ht->len1 );



      ht->attempt2 = 0;
      ht->country = 0xFE;//for Taiwan setting


      apiStatus = api_mdm_phonecheck( &linestate );
      if ( !linestate && apiStatus == apiOK )
        {
//        api_mdm_open( sbuf, &ht );
	  api_mdm_open( sbuf, (UCHAR *)ht );

          LIB_LCD_Cls();
          LIB_LCD_Puts( 1, 0, FONT0, sizeof(message_modem_connecting), (UINT8 *)message_modem_connecting );

          //wait connection of modem at most 40 sec

          pmdmOpenTimer = BSP_TMR_Acquire( BSP_LARGE_TIMER, mdmOpenTimerLisr );

          if ( pmdmOpenTimer )
            {
              //Set the timer for calculating the waiting packet time
              pmdmOpenTimer->Count  =1;
              pmdmOpenTimer->Compare = BSP_GetClock(TRUE, 2048/40);//Set the timeout occurs in 30 sec
              pmdmOpenTimer->Control=  TMR_PRES_DIV_2048 | TMR_TEN | TMR_MODE_RELOAD;
              BSP_TMR_Start( pmdmOpenTimer  );//Start the timer
            }
          else
            {
              return apiFailed;
            }

          while ( MdmOpenTimerFlag == 0 )
            {             
              apiStatus = api_mdm_status( 0, &linestate );
              if ( linestate == mdmConnected )
                {
                  BSP_TMR_Release(pmdmOpenTimer);
                  LIB_LCD_Cls();
                  LIB_LCD_Puts( 1, 0, FONT0, sizeof(message_modem_connected), (UINT8 *)message_modem_connected );

                  break;
                }
              else if ( linestate == mdmFailed)
                {
                  BSP_TMR_Release(pmdmOpenTimer);
                  return apiConnectFailed;

                }
            }
          if ( MdmOpenTimerFlag )
            {
              BSP_TMR_Release(pmdmOpenTimer);
              MdmOpenTimerFlag = 0 ;
              return apiConnectFailed;
            }

        }
      else
        {

          return apiNoLine;
        }

      break;
    }

#endif

#ifdef	_LAN_ENABLED_

    case ( DownLanMode ):
    {

      memmove( dssLanRemoteIP, DssRemoteIP, sizeof(dssLanRemoteIP) );//for internal usage
      memmove( dssLanRemotePort, DssRemotePort, sizeof(dssLanRemotePort) );//for internal usage
      memmove( dssLanPort, DssPort, sizeof(dssLanPort) );//for internal usage
      

      // IPstr2int( &dssLanRemoteIP[1], dssLanRemoteIP[0], &remoteIPTmp );//comment out by west


      portTmp = atoi(&dssLanPort[1]);
      remotePortTmp =  atoi( &dssLanRemotePort[1] );
      
      DSS_lan.LenType = 0;
      DSS_lan.ClientPort = portTmp; // not sure why need this
      DSS_lan.ServerPort = remotePortTmp;
      DSS_lan.ServerIP_Len = dssLanRemoteIP[0];
      memmove(DSS_lan.ServerIP, &dssLanRemoteIP[1], DSS_lan.ServerIP_Len);
      DSS_LANdhn = api_lan_open(DSS_lan);
      if (DSS_LANdhn==0xFF )
        {
          return apiConnectFailed;
        }

      break;
    }
    
#endif

    default :
    {
      return apiFailed;
    }

    }


  if ( apiStatus == BSP_SUCCESS || apiStatus == apiOK )
    {
      apiStatus= apiOK;
    }
  else
    {
      apiStatus = apiFailed;
    }

  return apiStatus;

}

// ---------------------------------------------------------------------------
// FUNCTION: api_down_txstring
//           Transmit the data
// INPUT   : TransMode ( 0: Uart, 1: modem ,2: lan)
//              size: the size to be transmitted
//              *tbuf: the data to be transmitted
// RETURN  : apiOK
//               apiFailed
// ---------------------------------------------------------------------------

UCHAR api_down_txstring( UCHAR TransMode, UCHAR *tbuf, UINT size )
{
  UCHAR					Status = apiOK;
  UCHAR         temp[MaxUpPackSize];
  temp[0]=(UCHAR)size;
  temp[1]=(UCHAR)(size>>8);
  memcpy( &temp[2], tbuf, size);
  switch ( TransMode )
    {
    case ( DownUartMode ):
    {
      Status = BSP_UART_Write( glpUart, tbuf, size );
      break;
    }

#ifdef	_MODEM_ENABLED_

    case ( DownModemMode ):
    {   
      Status = api_mdm_txstring( 0, temp );

      break;
    }

#endif

#ifdef	_LAN_ENABLED_

    case ( DownLanMode ):
    {
      Status = api_lan_txstring2( DSS_LANdhn, temp);
      
      break;
    }
    
#endif

    default :
    {
      return apiFailed;
    }

    }

  if ( Status == BSP_SUCCESS || Status == apiOK )
    {
      return apiOK;
    }
  else
    {
      return apiFailed;
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: api_down_rxready
//           To check if any data coming
// INPUT   : TransMode ( 0: Uart, 1: modem ,2: lan)
// OUTPUT  : *rbuf
// RETURN  : apiReady
//               apiNotReady
//               apiFailed
// ---------------------------------------------------------------------------

UCHAR api_down_rxready( UCHAR TransMode, ULONG *rbuf )
{
  UCHAR result;
  switch ( TransMode )
    {
    case ( DownUartMode ):
    {
      *rbuf = BSP_UART_Rxready( glpUart);
      if ( BSP_UART_Rxreadyflag != 0)
        {
          return apiReady;
        }
      else
        {
          return apiNotReady;
        }
    }

#ifdef	_MODEM_ENABLED_

    case ( DownModemMode ):
    {
      // OS_SLEEP(10);//for test
      return  api_mdm_rxready( 0, (UCHAR *) rbuf);

    }

#endif

#ifdef	_LAN_ENABLED_

    case ( DownLanMode ):
    {
      result = api_lan_rxready3( DSS_LANdhn, (UCHAR *) rbuf );
      DownReceiveSize=*rbuf;
      if ( result == apiReady)
        {
          return apiReady;
        }
      else
        {
          return apiNotReady;
        }
    }
    
#endif

    default:
    {
      return apiFailed;
    }
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: api_down_rxstring
//                 Receive the data
// INPUT   : TransMode ( 0: Uart, 1: modem ,2: lan)
//
// OUTPUT  : *rbuf - the point pointed to RX temp buffer
//                *Count - received data size
// RETURN  : apiOK
//               apiFailed
// ---------------------------------------------------------------------------

UCHAR api_down_rxstring( UCHAR TransMode, UCHAR *rbuf, ULONG *Count )
{
  UCHAR					Status = apiOK;


  switch ( TransMode )
    {
    case ( DownUartMode ):
    {
      memmove(rbuf,DSS_APPbuffer,*Count);
      // DSS_recieveData=0;
      DownReceiveSize=0;
      break;
    }

#ifdef	_MODEM_ENABLED_

    case ( DownModemMode ):
    {
      UCHAR                 temp[MaxDowPackSize+3];
      UINT                  size;


      Status = api_mdm_rxstring( 0, temp);
      size = temp[0]|( temp[1]<<8 );
      memcpy( (UCHAR *)rbuf, &temp[2], size );
      *Count = size;

      break;
    }

#endif

#ifdef	_LAN_ENABLED_

    case ( DownLanMode ):
    {
      api_lan_rxstring3(DSS_LANdhn,rbuf);

      break;
    }
    
#endif

    default :
    {
      return apiFailed;
    }
    }

  if ( Status == BSP_SUCCESS || Status == apiOK )
    {
      return apiOK;
    }
  else
    {
      return apiFailed;
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: api_down_close
//           Close the requested device
//
// INPUT   : TransMode ( 0: Uart, 1: modem ,2: lan)
//               dhn
// OUTPUT  : none.
// RETURN  : apiOK
//               apiFailed
// ---------------------------------------------------------------------------

UCHAR api_down_close( UCHAR TransMode, UCHAR dhn )
{
  UCHAR					Status = apiOK;

  switch ( TransMode )
    {
    case ( DownUartMode ):
    {
	
      //Status = BSP_UART_Stop( pUart );
      
      //Recover status of UART before DSS 
      BSP_UART_Release( glpUart );
//    glpUart = BSP_UART_Acquire( BSP_UART_0, UartISR );
      glpUart = BSP_UART_Acquire( os_DSS_COM, UartISR );	// 2014-07-23

      glpUart->Mode        = pUartBackUp.Mode;
      glpUart->Baud        = pUartBackUp.Baud;
      glpUart->DataBits    = pUartBackUp.DataBits;
      glpUart->StopBits    = pUartBackUp.StopBits;
      glpUart->Parity      = pUartBackUp.Parity;
      glpUart->BufferSize  = pUartBackUp.BufferSize;//�C�ӿN���ʥ]�j�p�̤j��(2048+9)bytes
      glpUart->FlowControl = pUartBackUp.FlowControl;
	  
	Status = BSP_UART_Start( glpUart );  
	
      break;
    }

#ifdef	_MODEM_ENABLED_

    case ( DownModemMode ):
    {
      Status = api_mdm_close( 0, 0 );

      break;
    }
    
#endif

#ifdef	_LAN_ENABLED_

    case ( DownLanMode ):
    {
      api_lan_close( DSS_LANdhn );

      break;
    }

#endif

    default :
    {
      return apiFailed;
    }
    }

  if ( Status == BSP_SUCCESS || Status == apiOK )
    {
      return apiOK;
    }
  else
    {
      return apiFailed;
    }
}

