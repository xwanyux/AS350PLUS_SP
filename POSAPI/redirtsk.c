//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : redirsk.c                                                  **
//**  MODULE   : modem task                                                 **
//**  VERSION  : V2.00                                                      **
//**  DATE     : 2010/06/08                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2010 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
//#include <stdarg.h>
//#include "bsp_mem.h"
#include "bsp_types.h"
#include "bsp_uart.h"
//#include "dmc20434.h"
//#include "redirtsk.h"
//#include "modem_api.h"
#include "SDLC.h"
//#include "nos.h"
#include "esf.h"
#include "APK_MDM.h"
#include "bsp_tmr.h"


extern	BSP_UART	*pUart;
extern  volatile	UCHAR     mdmDialConFlag;
extern  volatile	UCHAR     mdmEscOpenFlag;
extern	volatile	UCHAR	  mdmMode;
extern	volatile	UCHAR	  mdmPhonChkFlag;
extern	volatile    float	  mdmVtpMonit;
extern	volatile	UCHAR     mdmReferralFlag;
extern	volatile	UCHAR	  mdmEscSequenceFlag;
extern	volatile	UCHAR	  mdmStatusCode;
UCHAR	BACK_mdmStatusCode=0xff;
UCHAR	BACK_mdmPhonChkFlag=0xff;
UCHAR	BACK_MDM_FlowControlState=0xff;

extern	BSP_TIMER	*pReadFromModemTask;
extern	ULONG		MDM_FlowControlState;
// extern	NOS_TASK	*pReadFromModemTask;
// extern	NOS_TASK	*pWriteToModemTask;
extern	UINT		MDM_baud;

extern	void		MDM_ClearRTS_V80(void);
extern	void 		mdm_rxqueue_insert ( struct mdmqueue *pq, unsigned int size, UCHAR *api_status );
extern	void 		mdm_queue_remove( struct mdmqueue *pq );


//OS_TASK			*pPortMonTask = NULLPTR;
//OS_TASK			*pReadFromModemTask;
//OS_TASK			*pWriteToModemTask;

volatile  UCHAR		StartSdlcFlag = 0;
UCHAR			*pmdmTxBuf = NULLPTR;
UCHAR			*pmdmRxBuf = NULLPTR;
UINT 			mdmCanTx = FALSE;
ULONG			LastModemStatus = 0;

unsigned int          	mdmCountTX = 0;
unsigned int          	mdmCountRX = 0;

UCHAR			mdmRxBuffer[2048];

//Function Prototypes
//void port_monitor_task(    TASK_PARAMS );
//void write_to_modem_task(  TASK_PARAMS );
//void read_from_modem_task( TASK_PARAMS );

//#define _DEBUG_PRINTF
//void DEBUGPRINTF( char * Msg, ... );
// #ifdef _DEBUG_PRINTF
#define PRINT_BUF_SIZE 256

char	PrintBuf[PRINT_BUF_SIZE];
void DEBUGPRINTF( char * Msg, ... )
{


  va_list			args;
  int 				Len;

  va_start( args , Msg );
  Len = vsprintf( PrintBuf, Msg, args );
  printf("%s",PrintBuf);
  va_end( args );

}
// #endif


// ---------------------------------------------------------------------------
// FUNCTION: write_to_modem_task
//           One of the modem API task is used for put data in transmitter buffer
//           in modem driver
// INPUT   : Task parameters
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
 

void	write_to_modem_task( void )
{
  MODEM_STATUS				eRetCode;

//  while ( pWriteToModemTask )
//    {
      if ( mdmCanTx == FALSE )
        {
          //DEBUGPRINTF( "Cant send - CTS low\r\n" );
//          NosSleep(2);		// 20 ms
//          continue;
	return;
        }


      if ( mdmCountTX == 0 )
        {
//          NosSleep(2);      	// 20 ms
//          continue;
	return;
        }
      // printf("modem tx:\n");
      // for(int g=0;g<mdmCountTX;g++)
      //   printf(" 0x%02x ",*(pmdmTxBuf+g));
      // printf("\n");
      eRetCode = cnxt_modem_write( ( char * )pmdmTxBuf, &mdmCountTX );
      // memmove( pmdmTxBuf, 0x00, mdmCountTX );//2020-08-13 replace with memset in case of ggc "null argument where non-null required" warning
      memset( pmdmTxBuf, 0x00, mdmCountTX );
      if (eRetCode != MODEM_OK)
        {
          DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error(%d)\n\r", eRetCode );
        }
       
      mdmCountTX = 0;
//    }
}


//******************************************************************************
// �{���Ҳսs��:mdm_003_001
//******************************************************************************

void	*read_from_modem_task( void )
{
    while ( 1 )
    {
int				Avail;
MODEM_STATUS	eRetCode;
UCHAR			api_status = apiOK;
UCHAR 			phonecheckbuffer[20];
UCHAR 			temp[10];
ULONG			i;
u_int64_t			COUNT;
static UCHAR preventCutFlag = 0 ;	
	BSP_TMR_GetTick(pReadFromModemTask,&COUNT);
    if( mdmEscSequenceFlag )
      {
      NosSleep(1);	// 2
      continue;
      }
      
    if( ( !mdmPhonChkFlag ) && (mdmStatusCode != mdmConnected) && ((MDM_FlowControlState == MDM_STATE_IDLE) || (MDM_FlowControlState == MDM_STATE_INIT)) )
      {
      NosSleep(1);	// 2
      continue;
      }	  
	  if((BACK_mdmPhonChkFlag!=mdmPhonChkFlag)
		  ||(BACK_mdmStatusCode!=mdmStatusCode)
			||(BACK_MDM_FlowControlState!=MDM_FlowControlState))
	printf("mdmPhonChkFlag=%d\tmdmStatusCode=%d\tMDM_FlowControlState=%d\n",mdmPhonChkFlag,mdmStatusCode,MDM_FlowControlState);
	
      BACK_mdmPhonChkFlag=mdmPhonChkFlag;
	  BACK_mdmStatusCode=mdmStatusCode;
	  BACK_MDM_FlowControlState=MDM_FlowControlState;
      // if( pUart->TxSem == FALSE )
        // {
         // Wait for last tx to complete

          // OS_SLEEP( 1 );		// 10 ms
          // continue;
        // }
      

//	  Avail = 0;
//	  eRetCode = cnxt_modem_ioctl( MODEM_IOCTL_RX_AVAIL, &Avail );


	if( (mdmStatusRLSDS != 1) || (mdmMode != mdmSDLC) || ((mdmMode == mdmSDLC) && (MDM_baud != MDM_9600)) )
	  {
		Avail = 0;
		eRetCode = cnxt_modem_ioctl( MODEM_IOCTL_RX_AVAIL, &Avail );
		// printf("eRetCode=%x\t",eRetCode);
		// printf("Avail=%d\n",Avail);
	  }
	else
	  {
	  temp[0] = 0xff;
	  temp[1] = 0x80;
	  temp[2] = 2;
	  temp[3] = 0;
	  temp[4] = 0x19;
	  temp[5] = 0x66;
	  eRetCode = cnxt_modem_ioctl( MODEM_IOCTL_RX_AVAIL,(int*) temp );
	  }

	  
      if( eRetCode != MODEM_OK )
        {
        NosSleep(1);
        continue;
	}
	

      mdmCountRX = Avail;
      if (mdmMode == mdmSDLC)
        {

	  pmdmRxBuf = mdmRxBuffer;
          eRetCode = cnxt_modem_read( ( char * )pmdmRxBuf, &mdmCountRX );
          if ( eRetCode != MODEM_OK )
            {
              DEBUGPRINTF( "read_from_modem_task: cnxt_modem_read error(%d)\n\r", eRetCode );
              pmdmRxBuf = NULL;

              continue;
            }
       
          UCHAR *debug_pRxBuf=pmdmRxBuf;
          UCHAR *debug_pRxBuf_bak;
          UINT debug_CountPacket=0;//�????�???????�?�?主�??�??????�度
          UINT debug_CountRX=0;//�????�???????�?�?檢�?��?????�??????�度
          if( mdmStatusRLSDS )
            {
            //   printf("pmdmRxBuf=\n");
            // for(int g=0;g<mdmCountRX;g++)
            // //  if((*(pmdmRxBuf+g)>0x20)&&(*(pmdmRxBuf+g)<0x7f))
            // //     printf(" %c ",*(pmdmRxBuf+g));
            // //  else
            //     printf(" 0x%02x ",*(pmdmRxBuf+g));
            //  printf("\n");
            // printf("*pmdmRxBuf+189=0x%x\tmdmCountRX=%d\n",*(pmdmRxBuf+189),mdmCountRX);
            //??��?��?????�??????????�?
            if(mdmCountRX>20)
            {   
                while(1)
                {
                  if((*debug_pRxBuf==0x19)&&(*(debug_pRxBuf+1)==0xb2))
                  {
                    debug_CountPacket=2;//表�?��?�度
                    debug_pRxBuf_bak=debug_pRxBuf;
                  }
                    
                  if((*debug_pRxBuf==0x19)&&(*(debug_pRxBuf+1)==0xb0)||(debug_CountRX==(mdmCountRX-1)))//?????��??尾�??????????�已�?檢�?��?��?????�????
                  {
                    if(debug_CountRX>20)//�??????�度>20???該�?��?��?�主�?�????�?
                    {
                      debug_CountPacket++;//??�度???�?尾�??0xb0
                      debug_pRxBuf=debug_pRxBuf_bak;//??????�??????????
                      break;
                    }       
                  }                    
                  //�????�?�???�繼�????�?�????�????     
                  debug_pRxBuf++ ;
                  debug_CountRX++;
                  if(debug_CountRX==(mdmCountRX-1))//已�??檢�?��?��?????�????
                    break;
                }
                SdlcRxDataRecombine( debug_pRxBuf,debug_CountRX );
            }             
            else  
              SdlcRxDataRecombine( pmdmRxBuf,mdmCountRX );
            
            goto  WAIT_CD;
            }
          
          //20090507_Richard:?????�SDLC??????0x30???0x93被�????????�?
          if ( preventCutFlag == 1 )
            {
              if ( pmdmRxBuf[0] == 0x93 && mdmCountRX < 10 )
                {
                  StartSdlcFlag = 1;                  
                  temp[0]=0x30;
                  memcpy( &temp[1], pmdmRxBuf, mdmCountRX );
                  mdmCountRX++;
                  memcpy( pmdmRxBuf, temp, mdmCountRX);
                }
              preventCutFlag = 0;
	
            }
            
          //20090507_Richard:??�SDLC???frame??��?��????????�???��?�並????????��?????�????
          if ( StartSdlcFlag == 0 )
            {

              if ( pmdmRxBuf[0] == 0x30 && pmdmRxBuf[1] == 0x93 )
                {
                  StartSdlcFlag = 1;
                  preventCutFlag = 0;
                }
              else if ( preventCutFlag == 0 )
                {

                  if ( pmdmRxBuf[mdmCountRX-1] == 0x30 )
                    {
                    	preventCutFlag = 1;
                    }
                }
            }

          if ( StartSdlcFlag == 1 )
            {
              SdlcRxDataRecombine( pmdmRxBuf,mdmCountRX );
            }
            
WAIT_CD:
	  // Connected?
	  if( !mdmStatusRLSDS && (MDM_FlowControlState == MDM_STATE_WAIT_CD) )
	    {
	    for( i=0; i<mdmCountRX; i++ )
	       {
	       if( *pmdmRxBuf == 0x0D )
	         break;
	       else
	         pmdmRxBuf++;
	       }
	    
	    switch( (MDM_baud) & 0xFFE0 )
	          {
	          case MDM_1200:
	          
	               if( !strncmp( ( char * )( pmdmRxBuf ), "\r\nCONNECT 1200\r\n", 16 ) )
	                 {
	                 mdmStatusRLSDS = 1;
                  //  DEBUGPRINTF( "SDLC CONNECT 1200!!!\r\n" );
	                 continue;
	                 }
	               break;
	               
	          case MDM_2400:
            
	               if( !strncmp( ( char * )( pmdmRxBuf ), "\r\nCONNECT 2400\r\n", 16 ) )
	                 {
	                 mdmStatusRLSDS = 1;
	                 continue;
	                 }
	               break;
	               	        
	          case MDM_9600:
	          
	               if( !strncmp( ( char * )( pmdmRxBuf ), "\r\nCONNECT V29\r\n", 15 ) )
	                 {
//	                 MDM_ClearRTS();
			 MDM_ClearRTS_V80();
	                 
	                 mdmStatusRLSDS = 1;
	                 continue;
	                 }
	               break;
	          }
	    }

          //�P�_modem���A�T���A�ó]��modem���A�X��
          if ( !strncmp( ( char * )( pmdmRxBuf + mdmCountRX - 8 ), "\r\nBUSY\r\n", 8 ) )
            {
              mdmStatusCode = mdmRemoteLineBusy;
              mdmDialConFlag = 1; //�]�߭����X��
              DEBUGPRINTF( "Compare BUSY success!!!\r\n" );
            }
          else if ( !strncmp( ( char * )( pmdmRxBuf + mdmCountRX - 14 ), "\r\nNO CARRIER\r\n" , 14 ) )
            {
              mdmStatusCode = mdmLineDropped;
              mdmDialConFlag = 1;//�]�߭����X��
              DEBUGPRINTF( "Compare NO CARRIER success!!!\r\n" );
            }
          else if ( !strncmp( ( char * )( pmdmRxBuf + mdmCountRX - 13 ), "\r\nNO ANSWER\r\n", 13 ) )
            {
              mdmStatusCode = mdmLineDropped;
              mdmDialConFlag = 1;//�]�߭����X��
              DEBUGPRINTF( "Compare NO ANSWER success!!!\r\n" );
            }
          else if ( !strncmp( ( char * )( pmdmRxBuf + mdmCountRX - 15 ), "\r\nNO DIALTONE\r\n", 15 ) )
            {
//            mdmStatusCode = mdmLineDropped;
	      mdmStatusCode = mdmNoLine;
              DEBUGPRINTF( "Compare NO DIALTONE success!!!\r\n" );
              mdmEscOpenFlag = 1;

            }
          else if ( !strncmp( ( char * )( pmdmRxBuf + mdmCountRX - 15 ), "\r\nLINE IN USE\r\n", 15 ) )
            {
              mdmStatusCode = mdmLocalLineBusy;
              DEBUGPRINTF( "Compare LINE IN USE success!!!\r\n" );
              mdmEscOpenFlag = 1;
            }
          else if ( !strncmp( ( char * )( pmdmRxBuf + mdmCountRX - 11 ), "\r\nNO LINE\r\n", 11 ) )
            {
              mdmStatusCode = mdmNoLine;
              DEBUGPRINTF( "Compare NO LINE success!!!\r\n" );
              mdmEscOpenFlag = 1;
            }
        }
      else	// ASYNC
        {
          mdm_rxqueue_insert( &mdmRxqueue, mdmCountRX, &api_status);
          if ( mdmPhonChkFlag == 1 )
            {
              memcpy ( phonecheckbuffer, mdmRxqueue.rear->buffer, mdmRxqueue.rear->buffer_size );
              UCHAR    i, j=0, temp[6];

              for ( i=0; i<20; i++)
                {
                  if ( phonecheckbuffer[i] == 0x0D && phonecheckbuffer[i+1] == 0x0A )
                    {
                      for ( j = 0; j < 5; j++, i++)
                        {
                          if ( phonecheckbuffer[i+2] == 0x0D )
                            {
                              temp[j] = '\0';
                              break;
                            }
                          temp[j] = phonecheckbuffer[i+2];
                        }
                      break;
                    }
                }

              mdmVtpMonit = atof( ( char * ) temp );//�N�r���ഫ���B�I��
              DEBUGPRINTF( "mdmVtpMonit is %5.2f\r\n", mdmVtpMonit);
              
              mdmPhonChkFlag = 0;
	      continue;
            }
          
          if( mdmReferralFlag )	// PATCH: 2010-06-01
            continue;
          
          //�P�_modem���A�T���A�ó]��modem���A�X��
          if (!strncmp( ( char * )( ( mdmRxqueue.rear->buffer )+( mdmRxqueue.rear->buffer_size )-8 ), "\r\nBUSY\r\n", 8 ) )
            {
              mdmStatusCode = mdmRemoteLineBusy;
              mdmDialConFlag = 1; //�]�߭����X��
              DEBUGPRINTF( "Compare BUSY success!!!\r\n" );
            }
          else if ( !strncmp( ( char * )( ( mdmRxqueue.rear->buffer )+( mdmRxqueue.rear->buffer_size )-14 ), "\r\nNO CARRIER\r\n" , 14 ) )
            {
              mdmStatusCode = mdmLineDropped;
              mdmDialConFlag = 1;//�]�߭����X��
              DEBUGPRINTF( "Compare NO CARRIER success!!!\r\n" );
            }
          else if ( !strncmp( ( char * )( ( mdmRxqueue.rear->buffer )+( mdmRxqueue.rear->buffer_size )-13 ), "\r\nNO ANSWER\r\n", 13 ) )
            {
              mdmStatusCode =  mdmLineDropped;
              mdmDialConFlag = 1;//�]�߭����X��
              DEBUGPRINTF( "Compare NO ANSWER success!!!\r\n" );
            }
          else if ( !strncmp( ( char * )( ( mdmRxqueue.rear->buffer )+( mdmRxqueue.rear->buffer_size )-15 ), "\r\nNO DIALTONE\r\n", 15 ) )
            {
//            mdmStatusCode = mdmLineDropped;
	      mdmStatusCode = mdmNoLine;
              mdmEscOpenFlag = 1;//�]�wtimeout Flag�A�H���Xapi_mdm_open()
              DEBUGPRINTF( "Compare NO DIALTONE success!!!\r\n" );
            }
          else if ( !strncmp( ( char * )( ( mdmRxqueue.rear->buffer )+( mdmRxqueue.rear->buffer_size )-15 ), "\r\nLINE IN USE\r\n", 15 ) )
            {
              mdmStatusCode = mdmLocalLineBusy;
              mdmEscOpenFlag = 1;//�]�wtimeout Flag�A�H���Xapi_mdm_open()
              DEBUGPRINTF( "Compare LINE IN USE success!!!\r\n" );
            }
          else if ( !strncmp( ( char * )( ( mdmRxqueue.rear->buffer )+( mdmRxqueue.rear->buffer_size )-11 ),"\r\nNO LINE\r\n", 11 ) )
            {
              mdmStatusCode = mdmNoLine;
              mdmEscOpenFlag = 1;//�]�wtimeout Flag�A�H���Xapi_mdm_open()
              DEBUGPRINTF( "Compare NO LINE success!!!\r\n" );
            }
               else
                 {
                 // "CONNECT xxxx"?
                 for( i=0; i<mdmRxqueue.rear->buffer_size; i++ )
                    {
                    if( ((mdmRxqueue.rear->buffer_size) - i) >= 7 )
                      {
                      if( strncmp( &mdmRxqueue.rear->buffer[i], "CONNECT", 7 ) == 0 )
                        {
                        //�N�s�u���\�T��"CONNECT"��queue���M��
                        //if( !strncmp( ( char * )mdmRxqueue.rear->buffer, "\r\nCONNECT", 9 ) )
                          mdm_queue_remove( &mdmRxqueue );
                        
//	                MDM_WaitDCD();	// waiting for DCD on
                        
                        // waiting for DCD on for 5 seconds
//	                if( !MDM_WaitDCD_On( 500*2 ) )
//	                  mdmStatusCode = mdmFailed;
//	                else
//	                  {
                          mdmStatusCode = mdmConnected;
                          mdmStatusRLSDS = 1;
                          mdmCanTx = TRUE;
//	                  }
                          
                        break;
                        }
                      }
                    else
                      break;
                    }
                 }

        }
        
#ifdef MDM_DEBUG
		api_status = BSP_SUCCESS;
      //�N�����쪺��?�g��UART�ǰe��e����ܥX��
      if ( mdmMode == mdmSDLC )
        {
          //api_status = BSP_UART_Write( pUart, (UINT8 *)pmdmRxBuf, mdmCountRX );
		  printf("%s",*pmdmRxBuf);
          if ( api_status != BSP_SUCCESS )
            {
              DEBUGPRINTF( "UART write error %X\r\n", api_status );
              if ( mdmMode == mdmSDLC )
                {
//                BSP_Free( pmdmRxBuf );
                  pmdmRxBuf=NULL;
                }
              //continue;
            }
        }
      else
        { 
          //�N�����쪺��?�g��UART�ǰe��e����ܥX��
          //api_status = BSP_UART_Write( pUart, (UINT8 *)mdmRxqueue.rear->buffer, mdmCountRX );
		  printf("%s",*pmdmRxBuf);
          if ( api_status != BSP_SUCCESS )
            {
              DEBUGPRINTF( "UART write error %X\r\n", api_status );
              //continue;
            }
          
        }
#endif

      if ( mdmMode == mdmSDLC )
        {
//        BSP_Free( pmdmRxBuf );
          pmdmRxBuf = NULL;
        }
      else
        {
          //�p�G�s�u�|���إ߫h��node����
          if ( mdmStatusRLSDS == 0 )
            {
              mdm_queue_remove( &mdmRxqueue );
            }
          //�N�s�u���\�T��"CONNECT"��queue���M��
//          else
//            {
//              if ( !strncmp( ( char * )mdmRxqueue.rear->buffer, "\r\nCONNECT", 9 ) )
//                {
//                  DEBUGPRINTF( "Remove connect node\r\n" );
//                  mdm_queue_remove( &mdmRxqueue );
//                  mdmStatusCode = mdmConnected;
//                }
//            }
        }

    }
}
