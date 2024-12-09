//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : DSS (for no file system)                                   **
//**  PRODUCT  : AS320B                                                     **
//**                                                                        **
//**  FILE     : DownAnaly.C                                               **
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
#include "DownAnaly.h"
#include "DownInterface.h"
#include "DownPacket.h"
#include "bsp_types.h"




extern unsigned long SdramBurnOfs;
extern ULONG DownBaseAddr;

UCHAR     DownDataStartFlag = 0;//The flag to mark the incoming of data
extern ULONG     DownFileSize = 0; //Record the information of file size from "request response packet"
extern ULONG     DownReceiveSize=0;//Record the received size
UCHAR     DownTime[12];//Record the time captured from "request response packet"

// ---------------------------------------------------------------------------
// FUNCTION: DownAnalysis
//                Analyze incoming packet 
//          -------------------------
//          | Type | Address | Data |
//          -------------------------
//             1B      4B     2KB(opt) 1B
//
// INPUT   : TransMode : the packet starting with "type"
//              *pDownBuf : the buffer placed incoming data
//              Size : the packet length after eliminating "SOH", "Length", "LRC"
// OUTPUT  : *Reaction : the reaction after receiving
//
// RETURN  : apiOK
//               apiFailed
// ---------------------------------------------------------------------------

UCHAR DownAnalysis( UCHAR TransMode, const UCHAR *pDownBuf, ULONG Size, UCHAR *Reaction)
{
  UCHAR    ApiStatus = apiOK;
  //static UINT32   lastAddrOfPacket = 0;//checking if the received packet is a resending packet
  //UINT32       addrOfPacket = 0;
  //UCHAR    *pDownBuf = type;//20090508_Richard: Used for when "DownDataStartFlag == 1"
  //20080508_Richard: Complete packet is 18 bytes, '15' is CRC
//  UCHAR    completePack[] = { 0x04, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x15};
  UCHAR    completePack[21]; // = { 0x01, 0x11, 0x00, 0x04, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x15};
  UCHAR    lanCompletePack[] = { 0x04, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x15, 0x01, 0x01, 0x02, 0x04};
  UCHAR    lanModeCompleteSignal[] = { 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0 };
  UCHAR *    lanTracePoint=NULLPTR;//for debug
  UCHAR *    packetSize = NULLPTR;//for test
  ULONG     sdramEndAddr;//for debug
  
  completePack[0] = 0x01;
  completePack[1] = 0x11;
  completePack[2] = 0x00;
  completePack[3] = 0x04;
  memset( &completePack[4], 0xF0, 16 );
  completePack[sizeof(completePack)-1] = 0x15;
  

  struct DownRxReqPac *ReqPac;//for test

  //Mode 1: Recognize the type of the packet
  if ( DownDataStartFlag == 0 )
    {
      //struct DownRxReqPac *ReqPac = (struct DownRxReqPac *) pDownBuf;//for test
      ReqPac = (struct DownRxReqPac *) pDownBuf;//for test
      printf("ReqPac->Type=0x%x\n",ReqPac->Type);
      switch ( ReqPac->Type)
        {
          
          //20090508_Richard:new add, 0x80
        case ( DownRxReq ) :
        {
          packetSize = (UCHAR *)&( ReqPac->Size );
          
          if ( DownBaseAddr == 0 )//for first download
            {

              //Save the file size in the 1st round transmission
              DownFileSize =  *packetSize | *(packetSize+1)<<8  | *(packetSize+2)<<16 | *(packetSize+3)<<24;
              printf("DownFileSize=%d\n",DownFileSize);
              memmove( DownTime, &ReqPac->Data.TIME[0], 12);
            }


          *Reaction = DownTxAck;

#ifdef DEBUG
          DEBUGPRINTF( "2. Receive Request packet \r\n" );
#endif

          DownDataStartFlag = 1;//20090508_Richard：下一次預定收到的封包為資料封包
          /*
          	   //Create monitor download percentage task
          	   pShowTask = NosCreate( Down_show_task, 9, 0x4000, 0 );
          */

          break;
        }
        //0x81
        case( DownErrResend ):
        {
          *Reaction = DownPacketToSdram;
          break;
        }

        //Abort - 0xFF
        case ( DownRxAbort ):
        {
          *Reaction = DownABORT;

          break;
        }

        //NAK - 0x15
        //Receive 'NAK' means the remote host doesn't find the file.
        //EDC must stop the process and disconnect the communication
        case ( DownRxNak ):
        {
          *Reaction = DownRxNak;

          break;
        }

        default :
        {
#ifdef DEBUG
          DEBUGPRINTF( "Invalid packet incoming! \r\n" );
#endif
          *Reaction = DownTxNak;

          break;
        }
        }
      return ApiStatus;

    }

  //20090508_Richard:When "DownDataStartFlag==1"
  //Mode 2: Receive and store any input data to SDRAM
  else
    {
      sdramEndAddr = DownToSdram ( pDownBuf, Size );//20090517_Richard: Prevent from LRC error of  the last packet
printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
      //Check tail of incoming data to tell if the packet is the "last complete packet"
      if ( TransMode == DownLanMode )
        {
          // printf("sdramEndAddr=\n");
          // for(int i=0;i<=1+sizeof(lanModeCompleteSignal);i++)
          //   printf("%02x",*(UCHAR *)(sdramEndAddr-1-sizeof(lanModeCompleteSignal)+i));
          // printf("\n");
          //"sdramEndAddr-1" is for ignoring the last byte LRC ('0x07')
          if (  ! memcmp( (UCHAR *)(sdramEndAddr-1-sizeof(lanModeCompleteSignal)), lanModeCompleteSignal, sizeof( lanModeCompleteSignal ) ) )
            {
              lanTracePoint = (UCHAR *)(sdramEndAddr-1-sizeof(lanModeCompleteSignal));

              while (*lanTracePoint==0xF0 )
                {
                  if (*(--lanTracePoint)==0x04)
                    {
#ifdef DEBUG
                      DEBUGPRINTF( "Find 0x04 \r\n" );

#endif

                      if ( ! memcmp( (UCHAR *)(lanTracePoint+1-sizeof(lanCompletePack)), lanCompletePack, sizeof( lanCompletePack ) ))
                        {
                          //Find Complete packet，and start to handle the data in SDRAM
                          DownDataStartFlag = 0;
                          *Reaction = DownSuccess;

#ifdef DEBUG
                          DEBUGPRINTF( "5. Receive end packet \r\n" );

#endif
                          printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
                          return ApiStatus;
                        }
                    }
                }
            }
	    else if ( ! memcmp( (UCHAR *)(sdramEndAddr-sizeof(completePack)), completePack, sizeof( completePack ) ))
	    {
               DownDataStartFlag = 0;
              *Reaction = DownSuccess;

#ifdef DEBUG
              DEBUGPRINTF( "5. Receive end packet \r\n" );

#endif     
              printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
              return ApiStatus;
	     }
printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
          DownReceiveSize += Size;

          *Reaction = DownAlwaysReceive;

        }
      else
        {  
          if ( ! memcmp( (UCHAR *)(sdramEndAddr-sizeof(completePack)), completePack, sizeof( completePack ) ) )//When checking the end packet, discards 'LRC'.
            {
              //此封包為Complete，開始處理SDRAM中的資料
              DownDataStartFlag = 0;
              *Reaction = DownSuccess;
              DownReceiveSize= Size;//20211001 added by West,
#ifdef DEBUG
              DEBUGPRINTF( "5. Receive end packet \r\n" );

#endif
            }
          else
            {
printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
              DownReceiveSize += Size;

              //#ifdef DEBUG
              //DEBUGPRINTF( "4. Receive size : %d\r\n", DownReceiveSize  );
              //#endif

              *Reaction = DownAlwaysReceive;
            }
        }
        printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
      return ApiStatus;
    }

}

