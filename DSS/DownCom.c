//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : DSS (for no file system)                                   **
//**  PRODUCT  : AS320B                                                     **
//**                                                                        **
//**  FILE     : DownCom.C						    **
//**  MODULE   : DSS                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/06/15                                                 **
//**  EDITOR   : Richad Hu                                                  **
//**                                                                        **
//**  Copyright(C) 2009 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================

//#include "za9_pmu.h"
// #include "bsp_mem.h"
#include "bsp_uart.h"

#include "bsp_tmr.h"
#include "POSAPI.h"
#include "DownAnaly.h"
#include "Down.h"
#include "DownInterface.h"
#include "DownPacket.h"
#include "BurnFlash.h"
#include "OS_LIB.h"
#include "DSS2API.h"

#include "esf.h"
#include "redirtsk.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef	_FS_ENABLED_
#include "FSAPI.h"
#endif

extern BSP_UART			* pUart ;//for debug
extern BSP_TIMER	    * pShowTask;


ULONG SdramBurnOfs = 0;//The offset value to SDRAM starting writing address
ULONG DownburnSize = 0;//Record the burning size 
ULONG DownBaseAddr=0;//Record the address captured from request response packet
ULONG SystemRollback = 0;	// 2015-02-13, 1=rollback to OLD_PLATFORM
ULONG DownReceiveSize_bak = 0;	// record if rx szie growing
UINT  PacketLength=0;
UCHAR DownRxTimerFlag = 0;//Record the timeout when waiting packet for a long time

UCHAR	DSS_fid = 0;
UCHAR	DSS_file_name[16];
struct	FILE *DSS_pFile;
DOWNDATAPAC glpSeek;
UCHAR dataBuffer[DSS_MAX_APP_SIZE]; //Added by Tammy

extern UCHAR  DownDataStartFlag;//20090508_Richard
extern ULONG DownFileSize;
extern UCHAR DownTime[12];
extern ULONG		DSS_APlength;
extern UCHAR		DSS_APPbuffer[DSS_MAX_APP_SIZE];//10MB
extern	void	UT_DumpHexData( UCHAR mode, UCHAR row, UINT length, UCHAR *data );
extern	UCHAR DownDataToFile( ULONG* pnextOffset );
BSP_TIMER * pDownRxTimer;//used to require a timer
extern void getnowtime();
extern  void Tamper_SetupSecureDevice(); //Added by Tammy
void* DownRxTimerLisr()
{
UCHAR COUNT=0;
UCHAR FirstTimeCall=0;
  while(1)
  {
	  if(BSP_TMR_GetTick(pDownRxTimer,&COUNT)>0)
    {      
      // printf("DownRxTimerLisr timeout\n");
      // getnowtime();
      if(FirstTimeCall)
        DownRxTimerFlag = 1;
      FirstTimeCall=1;
    }
	  		
  }
  
}
/**
 * 
 * 20220324 add by west
 * I know it's dirty way to remove packet header and LRC.
 * 
 */
UCHAR Header_LRC_Eraser()
{
ULONG DataCursor=0,i=1;
ULONG EraserCursor=0;
UCHAR HeaderIdentifier[]="\x01\x05\x04\x02\x00";
  if(PacketLength==0)
  {
    printf("PacketLength=0\n");
    return 0;
  }
  
  HeaderIdentifier[2]=PacketLength>>8;
  printf("HeaderIdentifier[2]=%02x\n",HeaderIdentifier[2]);  
  if(DownReceiveSize_bak<=DownFileSize)
    return 0;
  while(1)
  {
    DataCursor=i*PacketLength+8;
    EraserCursor=i*PacketLength+9*(i)+8;
    if(memcmp(&DSS_APPbuffer[EraserCursor-8],HeaderIdentifier,5)==0)
    {
      memmove(&DSS_APPbuffer[DataCursor],&DSS_APPbuffer[EraserCursor],PacketLength);
      // DownReceiveSize_bak-=(i*1024+9);
      i++;
    }
    else
    {
       
      // printf("i=%d\n",i);
      // printf("DSS_APPbuffer[%d]: ",i*PacketLength+1+8);
      // for(int g=0;g<100;g++)
      //   printf("%02x",DSS_APPbuffer[i*PacketLength+1+8+g]);
      // printf("\n");
      memmove(&DSS_APPbuffer[DataCursor],&DSS_APPbuffer[EraserCursor],PacketLength);
      break;
    }
  }
}
// ---------------------------------------------------------------------------
// FUNCTION: DownReceive
//                 To receive and recombine the incoming packet
//           --------------------------------------------
//          | SOH | Length | Type | Address | Data | LRC |
//           --------------------------------------------
//            1 B    2B       1B      4B    2KB(opt)  1B
// INPUT   : TransMode ( 0: Uart, 1: modem )
//
// OUTPUT  : Reaction : the reaction after receiving
//                pDownRxBuf : the temp buffer for putting received data
//
// RETURN  : apiOK
//               apiFailed
// ---------------------------------------------------------------------------


UCHAR DownReceive ( UCHAR TransMode, UCHAR *Reaction, UCHAR *pDownRxBuf )
{

  UCHAR 	ApiStatus;
  UCHAR 	LenFlag = 0; //After getting the imcoming packet size, set the flag
  //UCHAR 	TestLrc = 0; //to calculate the LRC
  ULONG  	Size = 0;    //the data size in this RX
  ULONG  	Count = 0;   //record the received data size of a packet
  ULONG  	PackLen = 5; //record the getting packet length
  UCHAR 	TestLrc = 0;
  UINT  	i;

  


  //Mode 1: Reassemble the incoming data to a whole packet and recognize the type of packet
  if ( DownDataStartFlag == 0 )
    {

      while ( Count < PackLen )
        {

          // pDownRxTimer = BSP_TMR_Acquire( BSP_LARGE_TIMER, DownRxTimerLisr );
			pDownRxTimer = BSP_TMR_Acquire(5000);//5sec
			
          if ( pDownRxTimer )
            {
              BSP_TMR_Start( (void *)DownRxTimerLisr,pDownRxTimer );//Start the timer
            }
          else
            {
              return apiFailed;
            }
          DownRxTimerFlag=0;//init flag
          printf("DownRxTimerFlag=%d\n",DownRxTimerFlag);
          while ( DownRxTimerFlag==0 )
            {
              ApiStatus = api_down_rxready( TransMode ,&Size );
              if ( ApiStatus == apiReady )
                {
                  printf("ApiStatus == apiReady\n");
                  break;
                }
            }
            printf("DownRxTimerFlag=%d\n",DownRxTimerFlag);
#ifdef DEBUG
          DEBUGPRINTF( "Rxready response size is %lu\r\n", Size );
#endif

          // BSP_TMR_Release( pDownRxTimer );
			BSP_TMR_Stop( pDownRxTimer );
          if ( DownRxTimerFlag==1 )
            {
              DownRxTimerFlag = 0;

#ifdef DEBUG
              DEBUGPRINTF( "Timeout occur \r\n" );
#endif
              //Nak
              *Reaction = DownStop;

              return apiOK;
            }

          printf("Count=%d\n",Count);

          if((Size > MaxDowPackSize) || ((Count + Size) > MaxDowPackSize))
            return apiFailed;
          
          ApiStatus = api_down_rxstring( TransMode, pDownRxBuf+Count, &Size );

          Count += Size;
          Size = 0;


          //If the received first byte isn't "SOH"
          printf("pDownRxBuf[0]=%d\n",pDownRxBuf[0]);
          if ( pDownRxBuf[0]!=0x01 )
            {
#ifdef DEBUG
              DEBUGPRINTF( "Error SOH  \r\n" );
#endif
              *Reaction = DownTxNak;

              return apiOK;
            }

          //Only getting the packet lenth in a period of receiving a packet
          if ( Count >=3 || LenFlag == 0  )
            {
              //The total packet length is : Length + Length(2B)+ SOH(1B) + LRC(1B)
              PackLen = ( pDownRxBuf[1] | ((UINT16)pDownRxBuf[2]<<8) )+4;//SOH+Len+LRC=4
              printf("Count =%d PackLen =%d\n",Count,PackLen);
              LenFlag = 1;
            }

        }//end of while
      // printf("pDownRxBuf[]=\n");
      // for(int i=0;i<Count;i++)
      //   printf("%02x",pDownRxBuf[i]);
      printf("\nCount >= PackLen\n");
      //Verify the LRC value(SOH and LRC don't need to be checked )
      for ( ULONG i=0; i < PackLen-2; i++ )
        {
          TestLrc ^= pDownRxBuf[i+1];
        }
        printf("TestLrc =%d pDownRxBuf[ PackLen-1 ]=%d\n",TestLrc,pDownRxBuf[ PackLen-1 ]);
      if ( TestLrc == pDownRxBuf[ PackLen-1 ] )
        {
          //Throw the packet starting with "Type" byte into analysis
          //eliminate "SOH", "Length", "LRC"
          //          ---------------
          //          | Type | Data |
          //          ---------------
#ifdef DEBUG
          DEBUGPRINTF( "A complete packet income! \r\n" );
#endif
          ApiStatus = DownAnalysis( TransMode, pDownRxBuf, PackLen, Reaction );
          // ApiStatus = apiOK;
          // *Reaction=DownSuccess;//20200224 by west. jump through analysis
          return ApiStatus;
        }
      else
        {
#ifdef DEBUG
          DEBUGPRINTF( "LRC error \r\n" );
#endif
          *Reaction = DownTxNak;

          return apiOK;
        }

    }

  //Mode 2: Check if the incoming data is the "last complete packet", if not, save it to SRAM without any checking

  //��DownDataStartFlag == 1
  else
    {
      pDownRxTimer = BSP_TMR_Acquire(500);//5sec			
      if ( pDownRxTimer )
        {
          BSP_TMR_Start( (void *)DownRxTimerLisr,pDownRxTimer );//Start the timer
        }
      else
        {
          return apiFailed;
        }
printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
      while ( 1 )
        {
          ApiStatus = api_down_rxready( TransMode ,&Size );

          if ( ApiStatus == apiReady )
            {
              break;
            }
        }
      BSP_TMR_Stop( pDownRxTimer );
      printf("Size=%d\n",Size);
      if(DownReceiveSize_bak!=Size)//if DownReceiveSize growing, means rx not timeout yet.
        DownRxTimerFlag=0;
      if ( DownRxTimerFlag==1 )
        {
          DownRxTimerFlag = 0;
#ifdef DEBUG
          DEBUGPRINTF( "Timeout occur \r\n" );

#endif
          //Download stop and close the communication device
          *Reaction = DownStop;

          DownDataStartFlag = 0;

          return apiOK;
        }
      ApiStatus = api_down_rxstring( TransMode, pDownRxBuf, &Size );
      // printf("pDownRxBuf[]=\n");
      // for(int i=0;i<100;i++)
      //   printf("%02x",pDownRxBuf[i]);
      PacketLength=pDownRxBuf[2]<<8;
      // printf("\n");
      DownReceiveSize_bak=Size;
      ApiStatus = DownAnalysis( TransMode, pDownRxBuf, Size, Reaction );
      printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
      return ApiStatus;
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: The program for handle the action and reaction of DSS
//           Handle the reaction for receiving and transmitting data
//
// INPUT   : TransMode ( 0: Uart, 1: modem ,2:lan)
//              ReqItem:struct DownReqFormat
//                             {
//                                 UCHAR TID[8];
//                                 UCHAR BID[4];
//                                 UCHAR Reserv[12];
//                                 UCHAR AID[8];
//                                 UCHAR FID[32];
//                                 UCHAR VER[8];
//                                 UCHAR DATE[8];
//                                 ULONG Offset;
//                               }__attribute__((packed));;
//
// OUTPUT  : none.
// RETURN  : apiOK
//               apiFailed
//               apiFileNotFound
//               apiTimeOut
//               apiConnectFailed
//               apiNoLine
//               apiOffsetError
// ---------------------------------------------------------------------------


UCHAR DownCom ( UCHAR TransMode, DOWREQPAC *ReqItem )
{
  UCHAR    apiStatus;
  UCHAR    Reaction = DownTxReq;//Reaction after receiving
  UCHAR    RxFlag = 0; //To mark "receiving" needed for next communication or not
  UCHAR    FileNotFoundFlag = 0;
  UCHAR    *pDownRxBuf = NULLPTR;//for debug


  //Initialize global value used in downloading
  DownDataStartFlag = 0;
  SdramBurnOfs = 0; //Indicate the start address of SRAM


  //Verify the 'Mode of Communication'
  if ( TransMode > DownLanMode )
    {
      return apiFailed;
    }
  if(TransMode==DownUartMode)//20200224 by west.UART mode need to send request byte to PC first
    RxFlag = 0;              //LAN mode can just get data after connected.
    
  //Open the communication device
  apiStatus = api_down_open( TransMode );
  if ( apiStatus != apiOK )
    {
      return apiStatus;
    }

  pDownRxBuf = ( UCHAR* ) malloc( ( UINT32 ) ( MaxDowPackSize ));

  while ( 1 )
    {
      //In "Resend" mode, we don't need to receive the packet
      //It doesn't need to 'Receive' in first time
      if ( RxFlag )
        {
          printf("RxFlag\n");
          //Receive the packet
          apiStatus = DownReceive ( TransMode, &Reaction, pDownRxBuf );
          if ( apiStatus == apiFailed )
            {
              free( pDownRxBuf );
              return apiFailed;
            }
        }
      /*
            if ( ResendFlag )
              {
                //Resent the sendtype as the last transmission
                Reaction = SendType;
              }
      */
     printf("Reaction=%d\n",Reaction);
      switch ( Reaction )
        {

          //Request packet
        case ( DownTxReq ):
        {
          //ResendCount = 0;

          apiStatus = DownReq ( TransMode, ReqItem );//�ШDserver�ݵo�e���

          if ( apiStatus == apiOK )
            {
              //Record the send type in this time.
              //When the "Resend" occurs, it can resend the last packet with the last sent type
#ifdef DEBUG
              DEBUGPRINTF( "1. Send Request packet \r\n" );
#endif
              //SendType = DownTxReq;
              RxFlag = 1;
              //ResendFlag = 0;

              continue;
            }
          else
            {
              free( pDownRxBuf );
              return apiFailed;
            }
        }

        //20090508_Richard:Always receive mode
        case ( DownAlwaysReceive ):
        {
          RxFlag = 1;
          continue;
        }

        //transmit "ACK"
        case ( DownTxAck ):
        {
          //ResendCount = 0;

          apiStatus = DownAck( TransMode );
          if ( apiStatus == apiOK )
            {
#ifdef DEBUG
              DEBUGPRINTF( "3. Transmit ACK \r\n" );
#endif

              //SendType = DownTxAck;
              RxFlag = 1;
              //ResendFlag = 0;

              continue;
            }
          else
            {
              free( pDownRxBuf );
              return apiFailed;
            }
        }

        //transmit "NAK"
        case ( DownTxNak ):
        {

#ifdef DEBUG
          DEBUGPRINTF( "ERROR SOH or ERROR LRC \r\n" );
#endif

          free( pDownRxBuf );
          return apiFailed;
        }

        case ( DownABORT ):
        {
	    free( pDownRxBuf );
          return apiOffsetError;
        }

        //Receiving NAK means file not found
        case ( DownRxNak ):
        {
          FileNotFoundFlag = 1;
        }


        //"Stop" the process and return error to AP
        case ( DownStop ):
        {
          //Release the communication device
          api_down_close( TransMode, 0 );

          free( pDownRxBuf );

          if ( FileNotFoundFlag == 1 )
            {

              return apiFileNotFound;

            }
          else
            {

              return apiTimeOut;
            }
        }

        case ( DownSuccess ):
        {
          //Downloading data succeeds

          //Release the communication device
          api_down_close( TransMode, 0 );

          free( pDownRxBuf );

          return apiOK;
        }
        default :
        {
          free( pDownRxBuf );

          return apiFailed;
        }
        }
    }
}
// ---------------------------------------------------------------------------
// FUNCTION: DownReGet
//               The function for error continued downloading
// INPUT   : TransMode ( 0: Uart, 1: modem ,2:lan)
//              ReqItem:struct DownReqFormat
//                             {
//                                 UCHAR TID[8];
//                                 UCHAR BID[4];
//                                 UCHAR Reserv[12];
//                                 UCHAR AID[8];
//                                 UCHAR FID[32];
//                                 UCHAR VER[8];
//                                 UCHAR DATE[8];
//                                 ULONG Offset;
//                               }__attribute__((packed));;
//              Offset: the file offset which will be transmitted to remote host
//              sdramAddr: SDRAM address to be written
//              resendCount: the error number of packets
// OUTPUT  : none.
// RETURN  : apiOK
//               apiFailed
// PS. Not yet implement
// ---------------------------------------------------------------------------

UCHAR DownReGet ( UCHAR TransMode, DOWREQPAC *ReqItem, ULONG *Offset, ULONG *sdramAddr, UINT resendCount )
{
  UCHAR    ApiStatus;
  UCHAR    Reaction = DownReqResend;//Reaction after receiving
  UCHAR    RxFlag = 0; //To mark "receiving" needed for next communication or not
  UCHAR    FileNotFoundFlag = 0;
  UCHAR    *pDownRxBuf = NULLPTR;
  UINT      i = 0;


  while (resendCount)
    {

      Reaction = DownReqResend;
      RxFlag = 0;
      //Open the communication device
      if ( TransMode == DownLanMode )
        {
          ApiStatus = api_down_open( TransMode );
          if ( ApiStatus != apiOK )
            {
              return apiFailed;
            }
        }
      //Verify the 'Mode of Communication'
      if ( TransMode > DownLanMode )
        {
          return apiFailed;
        }

      pDownRxBuf = ( UCHAR* ) malloc( ( UINT32 ) ( MaxDowPackSize ));

      while ( 1 )
        {
          //In "Resend" mode, we don't need to receive the packet
          //It doesn't need to 'Receive' in first time
          if ( RxFlag )
            {
              //Receive the packet
              ApiStatus = DownReceive ( TransMode, &Reaction, pDownRxBuf );
              if ( ApiStatus == apiFailed )
                {
                  free( pDownRxBuf );
                  return apiFailed;
                }
            }

          switch ( Reaction )
            {

              //Request resend packet
            case ( DownReqResend ):
            {
              //ResendCount = 0;

              ApiStatus = DownRes ( TransMode, ReqItem, Offset[i] );//�ШDserver�ݵo�e���?

              if ( ApiStatus == apiOK )
                {
                  //Record the send type in this time.
                  //When the "Resend" occurs, it can resend the last packet with the last sent type
#ifdef DEBUG
                  DEBUGPRINTF( "1. Send Request Resend packet \r\n" );
#endif
                  RxFlag = 1;

                  continue;
                }
              else
                {
                  free( pDownRxBuf );
                  return apiFailed;
                }
            }

            //"Stop" the process and return error to AP
            case ( DownStop ):
            {
              //Release the communication device
              api_down_close( TransMode, 0 );

              free( pDownRxBuf );

              if ( FileNotFoundFlag == 1 )
                {
                  return apiFileNotFound;

                }
              else
                {
                  return apiFailed;
                }
            }
            case ( DownPacketToSdram ):
            {
              //Release the communication device
              api_down_close( TransMode, 0 );
              //Write data to SDRAM
              memmove( ( unsigned long * )(sdramAddr[i]), pDownRxBuf, (pDownRxBuf[1]|pDownRxBuf[2]<<8)+4 );
              resendCount--;

              free( pDownRxBuf );

              break;
            }
            default :
            {
              free( pDownRxBuf );

              return apiFailed;
            }

            }
        }
    }
  return apiOK;
}

// ---------------------------------------------------------------------------
// FUNCTION: DownToSdram
//           Download data into SDRAM for temporary storage
// INPUT   : pSource : The data source address
//              size : The data size will be written
// OUTPUT  : none.
// RETURN  :the last written address of SDRAM
// ---------------------------------------------------------------------------

ULONG  DownToSdram ( const void *pSource, ULONG size )
{
  // memmove( &DSS_APPbuffer[SdramBurnOfs] , pSource , size );
  SdramBurnOfs += size;

  // return (SdramBurnOfs);
  return (&DSS_APPbuffer[SdramBurnOfs]);
}

// ---------------------------------------------------------------------------
// FUNCTION: DownVerifyData()
//                 Verify received data stored in SDRAM
// INPUT   : TransMode ( 0: Uart, 1: modem ,2:lan)
//              ReqItem:struct DownReqFormat
//                             {
//                                 UCHAR TID[8];
//                                 UCHAR BID[4];
//                                 UCHAR Reserv[12];
//                                 UCHAR AID[8];
//                                 UCHAR FID[32];
//                                 UCHAR VER[8];
//                                 UCHAR DATE[8];
//                                 ULONG Offset;
//                               }__attribute__((packed));;
//
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//	     apiOutOfService	- special value for normal DATA file download.
// ---------------------------------------------------------------------------

UCHAR DownVerifyData( UCHAR TransMode, DOWREQPAC *ReqItem )
{
UINT   i;
UINT   j = 0;
UCHAR   packetLrc;//for test
UCHAR 	testLrc = 0; //to calculate the LRC
UCHAR   firstDataFlag = 1;
UCHAR   lastDataFlag = 0;
UCHAR   errorFlag = 0;
UCHAR   apiStatus = apiOK;
UINT     resendCount = 0;
UINT debugCount=0;//for test
ULONG   address = 0;
ULONG   resendOffset[100]={0};//if error occurs, request the offset to get the correct data
ULONG   reWriteSdramAddr[100]={0};
DOWNDATAPAC *pseek = NULLPTR;
UCHAR   *pStart;

//Remarked by Tammy
//pseek = ( DOWNDATAPAC *) DownSdramStartAddr;
pseek = (DOWNDATAPAC *)DSS_APPbuffer;   //Added by Tammy

BEGIN:
	
  pStart = &(pseek->Msg.Data);
  
NEXT:

//  UT_DumpHexData( 0, 0, 8, &(pseek->Msg.Data) + 12 );

  address = pseek->Msg.Offset;//Get the initial address

  while (1)
    {
      debugCount++;//for test

      if ( pseek->SOH != 0x01 )
        {
#ifdef DEBUG
          DEBUGPRINTF( "Error SOH: %d  \r\n", debugCount );
#endif
          resendOffset[j]=address;
          reWriteSdramAddr[j] = (ULONG) pseek;
          errorFlag = 1;

        }

      if ( firstDataFlag )
        {
          firstDataFlag = 0;

          if ( pseek->Msg.Type != DownDataStart )
            {
#ifdef DEBUG
              DEBUGPRINTF( "Error type 01: %d \r\n", debugCount );
#endif
              resendOffset[j]=address;
              reWriteSdramAddr[j] = (ULONG) pseek;
              errorFlag = 1;

            }
        }
      else
        {
        	
          if ( (pseek->Msg.Type == DownDataEnd) || (DownFileSize < 1024) )
            {
              lastDataFlag = 1;
#ifdef DEBUG
              DEBUGPRINTF( "Find last data: %d  \r\n", debugCount );
#endif
            }

          if ( ( pseek->Msg.Type != DownData ) &&  ( pseek->Msg.Type != DownDataEnd ) )
            {
#ifdef DEBUG
              DEBUGPRINTF( "Error type 02 or 03: %d \r\n", debugCount );
#endif
              resendOffset[j]=address;
              reWriteSdramAddr[j] = (ULONG) pseek;
              errorFlag = 1;

            }

        }

      if ( address != pseek->Msg.Offset )
        {
#ifdef DEBUG
          DEBUGPRINTF( "Error Offset: %d \r\n", debugCount );
#endif
          resendOffset[j]=address;
          reWriteSdramAddr[j] = (ULONG) pseek;
          errorFlag = 1;

        }


      //Verify the LRC value(SOH and LRC don't need to be checked )
      //LRC:	XOR from LEN to end of MSG
      testLrc = 0;

      for ( i=0; i < ( pseek->Len+2 ) ; i++ )
        {
          testLrc ^=   *( (UCHAR *)&( pseek->Len ) + i  );
        }

      packetLrc = *( (UCHAR *)&( pseek->Msg ) + pseek->Len  );


      if ( testLrc == packetLrc )
        {
          testLrc = 0;
          address = address + (pseek->Len - 5);
          //pseek = ( DOWNDATAPAC *)( (UCHAR *)&( pseek->Msg ) + pseek->Len + 1 );

#ifdef DEBUG
          //DEBUGPRINTF( "Correct packet: %d \r\n", debugCount );
#endif
          if ( lastDataFlag == 1)
            {
              firstDataFlag = 1;
              lastDataFlag = 0;
              break;
            }
        }
      else
        {
#ifdef DEBUG
          DEBUGPRINTF( "Error LRC:%d  \r\n", debugCount );
#endif
          resendOffset[j]=address;
          reWriteSdramAddr[j] = (ULONG) pseek;
          errorFlag = 1;

          //return apiFailed;
        }
      //pseek2 = pseek;//for debug
      pseek = ( DOWNDATAPAC *)( (UCHAR *)&( pseek->Msg ) + pseek->Len + 1 );

      if ( errorFlag )
        {
          errorFlag = 0;
          resendCount++;
          j++;
          if (j >= 100 )
            {
              return apiFailed;
            }
        }

    }	// while(1)
    
  if (resendCount != 0)
    {
      apiStatus = DownReGet ( TransMode, ReqItem, resendOffset, reWriteSdramAddr, resendCount );
      if ( apiStatus == apiFailed )
        {
#ifdef DEBUG
          DEBUGPRINTF( "Resend Failed \r\n" );
#endif
          return apiStatus;
        }

    }

#ifdef DEBUG
  DEBUGPRINTF( "6. Verification success \r\n" );
#endif
  
//  *( pStart + ECB_OFFSET_MID ) = ECB_RESTORE_VALUE;	// 2012-02-16, restore ECB default values
  
  return apiOK;
}

// ---------------------------------------------------------------------------
// FUNCTION: SPorAP()
//           Both AP and SP are ELF file, the only way to distinguish them are
//           check trailer of incomming file
//           File Format: APP.bin[n]+TRAILER[64]+SIGNATURE[256]
//           TRAILER Format: (fixed 64 bytes)
//           ------  ---------------  -------------------------------------
//           OFFSET      REMARK                    VALUE 
//           ------  ---------------  -------------------------------------
//           00~01      File Type         "AP", "SP", "UB", "KN"      
//           02~05      Model ID          as defined on AS350
//                                        ES1: 0x4B,0x00,0x00,0x00    
//                                        ES2: 0x4C,0x00,0x00,0x00    
//                                        ES3: 0x4A,0x00,0x00,0x00    
//                                        ES4: 0x4D,0x00,0x00,0x00    
//           06..63      (RFU)            0x00's                      
//           ------  ---------------  -------------------------------------
// INPUT   : None.
// OUTPUT  : None.
// RETURN  : file type - SP, AP, or OTHER
// ---------------------------------------------------------------------------
UCHAR SPorAP()  //Modified by Tammy
{
    UCHAR ASCII_SP[]={'S','P'};
    UCHAR ASCII_AP[]={'A','P'};
    DOWNDATAPAC *pseek = ( DOWNDATAPAC *)DSS_APPbuffer;
    ULONG i = 0;
    UCHAR *dataSeek = NULLPTR;
    ULONG index = 0;


    while(1)
    {
        dataSeek = (UCHAR *)&(pseek->Msg) + 5;
        memcpy(&dataBuffer[index], dataSeek, (pseek->Len - 5));
        index += (pseek->Len - 5);

        if(pseek->Msg.Type == DownDataEnd)
            break;
        else
            pseek = (DOWNDATAPAC *)((UCHAR *)&(pseek->Msg) + pseek->Len + 1);
    }

    if(memcmp(&dataBuffer[DownFileSize - (64 + 256)], ASCII_SP, sizeof(ASCII_SP)) == 0)
        return __SP;
    if(memcmp(&dataBuffer[DownFileSize - (64 + 256)], ASCII_AP, sizeof(ASCII_AP)) == 0)
        return __AP;
    else
        return __OT;
}

void CheckHeader(DOWREQPAC *ReqItem)    //Modified by Tammy
{
    UCHAR elfHeader[4] = {0x7f, 0x45, 0x4c, 0x46};
    UCHAR ca_certificate_header[] = {"-----BEGIN CERTIFICATE-----"};
    UCHAR client_certificate_header[] = {"Certificate:"};
    UCHAR client_private_key_header[] = {"-----BEGIN RSA PRIVATE KEY-----"};  
    DOWNDATAPAC *pseek = NULL;


    pseek = ( DOWNDATAPAC *)DSS_APPbuffer;
    if(memcmp(&(pseek->Msg.Data), elfHeader, sizeof(elfHeader)) == 0)
    {
        printf("ELF header detected\n");

        switch(SPorAP())
        {
            case __SP:
                memcpy(&ReqItem->AID, "SP", sizeof("SP"));
                break;
            case __AP:
                memcpy(&ReqItem->AID, "APP", sizeof("APP"));
                break;
            case __OT:
                memcpy(&ReqItem->AID, "OT", sizeof("OT"));
                break;
            default:
                break;
        }
    }
    else if(memcmp(&(pseek->Msg.Data), ca_certificate_header, sizeof(ca_certificate_header)-1) == 0)
    {
        printf("CA certificate header detected\n");
        memcpy ( &ReqItem->AID, "CA_CRT", sizeof("CA_CRT") );
    }
    else if(memcmp(&(pseek->Msg.Data), client_certificate_header, sizeof(client_certificate_header)-1) == 0)
    {
        printf("client certificate header detected\n");
        memcpy ( &ReqItem->AID, "C_CRT", sizeof("C_CRT") );
    }
    else if(memcmp(&(pseek->Msg.Data), client_private_key_header, sizeof(client_private_key_header)-1) == 0)
    {
        printf("client private key header detected\n");
        memcpy ( &ReqItem->AID, "C_PRV", sizeof("C_PRV") );
    }
    else
    {
        memcpy(&ReqItem->AID, "OT", sizeof("OT"));
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: DownDataProcess()
//               Deal with the data after downloading  for different AID
// INPUT   :  ReqItem:struct DownReqFormat
//                             {
//                                 UCHAR TID[8];
//                                 UCHAR BID[4];
//                                 UCHAR Reserv[12];
//                                 UCHAR AID[8];
//                                 UCHAR FID[32];
//                                 UCHAR VER[8];
//                                 UCHAR DATE[8];
//                                 ULONG Offset;
//                               }__attribute__((packed));;
//
//                bufferSize:size of user's buffer
//
// OUTPUT  : *buffer: store the received data
//                *outputSize: the size of data stored in user's buffer
// RETURN  : apiOK
//               apiFailed
// ---------------------------------------------------------------------------
UCHAR DownDataProcess( DOWREQPAC *ReqItem, UCHAR *buffer, UINT bufferSize, UINT *outputSize )
{
  UCHAR apiStatus = apiOK;
  ULONG nextOffset;//for debug
  UCHAR    down_message_eraseflash[]={" Erasing Flash... "};

  CheckHeader(ReqItem);
// #ifdef	_FS_ENABLED_
//         if( DSS_fid == 1 )
//           {
        printf("ReqItem-> AID = ");
        for(int i = 0 ; i < 6 ; i++)
          printf("%c",ReqItem-> AID[i]);
        printf("\n");
      //     apiStatus = api_dss2_burn2( 0 );
      //  DSS_fid = 0;
      //     return( apiStatus );
//           }

// #endif
  if ( ! memcmp ( &ReqItem-> AID[0], "APP", sizeof("APP") ))
  {
    apiStatus = api_dss2_burn2( 0 );
    DSS_fid = 0;
    return( apiStatus );
  }
  else if ( ! memcmp ( &ReqItem-> AID[0], "SP", sizeof("SP") ))
  {
    //Modified by Tammy
    apiStatus = api_dss2_burnSP();
    return apiStatus;
  }
  else if ( ! memcmp ( &ReqItem-> AID[0], "CA_CRT", sizeof("CA_CRT") )) //Added by Tammy
  {
    apiStatus = api_dss2_burn_CA_Certificate();
    return apiStatus;
  }
  else if ( ! memcmp ( &ReqItem-> AID[0], "C_CRT", sizeof("C_CRT") ))   //Added by Tammy
  {
    apiStatus = api_dss2_burn_client_Certificate();
    return apiStatus;
  }
  else if ( ! memcmp ( &ReqItem-> AID[0], "C_PRV", sizeof("C_PRV") ))   //Added by Tammy
  {
    apiStatus = api_dss2_burn_client_privateKey();
    return apiStatus;
  }

  if ( ! memcmp ( &ReqItem-> AID[0], "APP", sizeof("APP") ) || ! memcmp ( &ReqItem-> AID[0], "SP", sizeof("SP") ))
    {
      if ( DownBaseAddr == 0 )
        {
          LIB_LCD_Cls();
          LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_message_eraseflash)-1, (UINT8 *)down_message_eraseflash );

          // apiStatus = EraseFlashArea ();
          if ( apiStatus == apiFailed )
            {

              return apiEraseFailed;
            }
        }
        

      return apiStatus;
    }
    //20210220 temperary comment out by west 
/*
  else if ( ! memcmp ( &ReqItem-> AID[0], "TMSPARA", sizeof("TMSPARA") )  ||  ! memcmp ( &ReqItem-> AID[0], "EMVPARA", sizeof("EMVPARA") ) ||  ! memcmp ( &ReqItem-> AID[0], "CAPK", sizeof("CAPK") ))
    {
      apiStatus = DownParaToFile( buffer, bufferSize, outputSize );

      return apiStatus;
    }


  else
    {
      return apiFailed;
    }
*/
}

// ---------------------------------------------------------------------------
// FUNCTION: DownBurnToFlash()
//                 burning data to flash ( the starting address of writting will be extract form the sig.bin file )
// INPUT   :  none.
//
// OUTPUT  :  pnextOffset: Record the next offset for continueous download
// RETURN  : apiOK
//               apiFailed: 1. Burning failed
//                             2. The burning size is bigger than file size
// ---------------------------------------------------------------------------
#if 0
UCHAR DownBurnToFlash( ULONG* pnextOffset )
{

  UCHAR apiStatus = apiOK;
  DOWNDATAPAC *pseek = NULLPTR;


  //Extract the starting address from "sig.bin" file

  pseek = ( DOWNDATAPAC *) DownSdramStartAddr;

  if ( DownBaseAddr == 0 )//Download round 1
    {
      // offset 0x20 of sig.bin file is the location to save the address
      // the address minus 0x24 will be the real address
      DownBaseAddr = *( ( ULONG * )( &( pseek->Msg.Data ) + 0x20 ) )- 0x24 ;
    }


  while (1)
    {
      //baseAddr = baseAddr + ( pseek->Msg.Offset );
      apiStatus = api_flash_write( (void *)DownBaseAddr + ( pseek->Msg.Offset ), (void *)&(pseek->Msg.Data), pseek->Len-5 );
      if ( apiStatus == apiOK )
        {
//#ifdef DEBUG
          //DEBUGPRINTF( "Burning..." );
//#endif
          DownburnSize = DownburnSize + pseek->Len-5;

          /*
          #ifdef DEBUG
                        DEBUGPRINTF( "Receive Offset=%d\r\n", ( pseek->Msg.Offset ) );
                        DEBUGPRINTF( "DownburnSize=%d\r\n", DownburnSize );
          #endif
          */

          if ( pseek->Msg.Type == DownDataEnd )
            {
#ifdef DEBUG
              DEBUGPRINTF( "Burning Flash Successes!" );
#endif

              if ( DownburnSize == DownFileSize )
                {
#ifdef DEBUG
                  DEBUGPRINTF( " DownburnSize == DownFileSize" );
#endif
                  return apiOK;
                }

              else  if ( DownburnSize < DownFileSize )
                {
                  *pnextOffset = pseek->Msg.Offset + ( pseek->Len-5 );

#ifdef DEBUG
                  DEBUGPRINTF( " DownburnSize <= DownFileSize" );
#endif
                  return DownContinue;
                }

              else
                {
                  return apiFailed;
                }
            }
          else
            {
              pseek = ( DOWNDATAPAC *)( (UCHAR *)&( pseek->Msg ) + pseek->Len + 1 );

            }
        }
      else
        {
#ifdef DEBUG
          DEBUGPRINTF( "Burning Flash Failed!" );
#endif
          return apiFailed;
        }
    }

}
#endif
// ---------------------------------------------------------------------------
// FUNCTION: DownBurnToFlash()
//                 burning data to flash ( the starting address of writting will be extract form the sig.bin file )
// INPUT   :  none.
//
// OUTPUT  :  pnextOffset: Record the next offset for continueous download
// RETURN  : apiOK
//               apiFailed: 1. Burning failed
//                             2. The burning size is bigger than file size
// ---------------------------------------------------------------------------
UCHAR DownDataToFile( ULONG* pnextOffset )
{

  UCHAR apiStatus = apiOK;
  DOWNDATAPAC *pseek = NULLPTR;
  UCHAR	first = 1;


  //Extract the starting address from "sig.bin" file

  pseek = ( DOWNDATAPAC *) DownSdramStartAddr;

//  if ( DownBaseAddr == 0 )//Download round 1
//    {
//      // offset 0x20 of sig.bin file is the location to save the address
//      // the address minus 0x24 will be the real address
//      DownBaseAddr = *( ( ULONG * )( &( pseek->Msg.Data ) + 0x20 ) )- 0x24 ;
//    }


  while (1)
    {
      //baseAddr = baseAddr + ( pseek->Msg.Offset );
      if( first )
        {
        api_fs_write( DSS_pFile, (void *)&(pseek->Msg.Data)+32, (pseek->Len-5)-32 );
        first = 0;
        }
      else
        api_fs_write( DSS_pFile, (void *)&(pseek->Msg.Data), pseek->Len-5 );
        
//      numwritten = api_fs_write( DSS_pFile, (void *)DownBaseAddr + ( pseek->Msg.Offset ), pseek->Len-5 );
//      apiStatus = api_flash_write( (void *)DownBaseAddr + ( pseek->Msg.Offset ), (void *)&(pseek->Msg.Data), pseek->Len-5 );
      apiStatus = apiOK;
      if ( apiStatus == apiOK )
        {
          DownburnSize = DownburnSize + pseek->Len-5;

          if ( (pseek->Msg.Type == DownDataEnd) || (DownFileSize < 1024) )
            {
              if ( DownburnSize == DownFileSize )
                {
                  api_fs_close( DSS_pFile );
                  return apiOK;
                }

              else  if ( DownburnSize < DownFileSize )
                {
                  *pnextOffset = pseek->Msg.Offset + ( pseek->Len-5 );

                  return DownContinue;
                }

              else
                {
                  return apiFailed;
                }
            }
          else
            {
              pseek = ( DOWNDATAPAC *)( (UCHAR *)&( pseek->Msg ) + pseek->Len + 1 );

            }
        }
      else
        {
          return apiFailed;
        }
    }

}


// ---------------------------------------------------------------------------
// FUNCTION:  DownParaToFile
//
// INPUT:       bufferSize:size of user's buffer
//
// OUTPUT  : *buffer: store the received data
//                *outputSize: the size of data stored in user's buffer
//
// RETURN  : apiOK
//               apiFailed: file operation error
// ---------------------------------------------------------------------------


UCHAR DownParaToFile( UCHAR *buffer, UINT bufferSize, UINT *outputSize )
{

  UCHAR apiStatus = apiOK;

  ULONG writeCount = 0;
  //ULONG writeTotal = 0;
  DOWNDATAPAC *pseek = NULLPTR;
  //struct FILE* pfileID;

  pseek = ( DOWNDATAPAC *) DownSdramStartAddr;

  //Extract the starting address from "sig.bin" file
  LIB_LCD_Cls();
  /*
    pfileID = api_fs_open( filename, 0 );
    if ( pfileID == NULLPTR )
      {
        apiStatus = api_fs_create( filename, 0x02);
        if ( apiStatus != apiOK )
          {
            return apiFailed;
          }
      }
    else
      {
        api_fs_close( pfileID );
        apiStatus = api_fs_delete(filename);
        if ( apiStatus == apiOK )
          {
            apiStatus = api_fs_create( filename, 0x02);
            if ( apiStatus != apiOK )
              {
                return apiFailed;
              }
          }
        pfileID = api_fs_open( filename, 0 );

      }
  */
  if ( DownFileSize > bufferSize )
    {
      *outputSize = DownFileSize;

      return apiFailed;
    }



  while (1)
    {
      //writeCount = api_fs_write( pfileID, (void *)&(pseek->Msg.Data), pseek->Len-5 );
      memmove( buffer+writeCount, (void *)&(pseek->Msg.Data), pseek->Len-5 );
      writeCount = writeCount + (pseek->Len-5);
      //writeTotal += writeCount;

#ifdef DEBUG
      DEBUGPRINTF( "Saving..." );
#endif
      //saveSize = saveSize + (pseek->Len-5);

      if ( pseek->Msg.Type == DownDataEnd )
        {

          //api_fs_close( pfileID );

#ifdef DEBUG
          DEBUGPRINTF( "Save Successes!" );
#endif


          if ( writeCount == DownFileSize )
            {
#ifdef DEBUG
              DEBUGPRINTF( " writeTotal == DownFileSize" );
#endif
              *outputSize = DownFileSize;

              return apiOK;
            }

#ifdef DEBUG
          DEBUGPRINTF( " writeTotal != DownFileSize" );
#endif
          return apiFailed;
        }
      else
        {
          pseek = ( DOWNDATAPAC *)( (UCHAR *)&( pseek->Msg ) + pseek->Len + 1 );

        }

    }

}

// ---------------------------------------------------------------------------
// FUNCTION: DownRefreshRTC
//
// INPUT:       none
//
// OUTPUT  : none
//
// RETURN  : none
// ---------------------------------------------------------------------------

void DownRefreshRTC(void)
{
  UCHAR     rtc_dhn;
  UCHAR rtcafter[13];

  rtc_dhn = api_rtc_open();
  /*
      #ifdef DEBUG
    UCHAR rtcbefore[13]={0x30, 0x30, 0x30, 0x31, 0x30, 0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0};
             UCHAR rtcafter[13]={0};
   api_rtc_setdatetime( rtc_dhn, rtcbefore);
        api_rtc_getdatetime( rtc_dhn, rtcbefore);

  DEBUGPRINTF( "Time is : %s\r\n", rtcbefore );


          #endif
          */
  api_rtc_setdatetime( rtc_dhn, DownTime);
  api_rtc_close(rtc_dhn);

#ifdef DEBUG
  DEBUGPRINTF( "Refresh RTC! \r\n" );
  memmove( rtcafter, DownTime, 12);//for debug
  api_rtc_getdatetime( rtc_dhn, rtcafter);
  DEBUGPRINTF( "Time is : %s\r\n", rtcafter );
#endif


}


