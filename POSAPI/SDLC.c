
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : SDLC.c                                                     **
//**  MODULE   : SDLC protocol for modem sync transmission and receive      **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2008/08/21                                                 **
//**  EDITOR   : Richard Hu                                                 **
//**                                                                        **
//**  Copyright(C) 2008 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================

#include "POSAPI.h"
#include "SDLC.h"
#include "esf.h"
#include "APK_MDM.h"
#include "bsp_types.h"
#include "redirtsk.h"
#include <stdio.h>
extern struct mdmqueue      mdmTxqueue;
extern UCHAR 		    *pmdmTxBuf;
extern UCHAR                SdlcAddress;
extern unsigned int	    mdmCountTX;
extern UINT 		    mdmCanTx;
extern void	write_to_modem_task( void );
UCHAR                     *RxSdlcBuf = NULLPTR;
UCHAR			  RxSdlcBuffer[2048];
UCHAR			  TxSdlcBuffer[2048];
UCHAR			  SdlcWorkBuffer[2048];
UCHAR			  SdlcTempBuffer[4096];

UCHAR                     SdlcStartMoveFlag=0;
UCHAR                     SdlcStopSymbolFlag=0;
UCHAR 			  SdlcSendCount = 0;
UCHAR 			  SdlcReceiveCount = 0;
UCHAR 			  SdlcNRPre = 0;

//******************************************************************************
// 程式模組編號:SDLC_001_001
// 副程式名稱： SdlcRxDataRecombine
// 功能：將接收進來被分割的封包重新組合起來
//******************************************************************************

void SdlcRxDataRecombine(UCHAR *Dtemp,unsigned int leng)
{
  unsigned int i;
  static int j;
  static unsigned int CountLength;
  // printf("SdlcAddress=0x%x\n",SdlcAddress);
  for (i=0; i<leng; i++)
    {
      if (SdlcStartMoveFlag==0)
        {
          if (Dtemp[i]== SdlcAddress)
            {

//            RxSdlcBuf=(UCHAR*) BSP_Malloc((INT32) (1024*sizeof(UCHAR)) );
//	      RxSdlcBuf=(UCHAR*) BSP_Malloc((INT32) (2048*sizeof(UCHAR)) );	// PATCH: 2010-03-15
	      RxSdlcBuf = RxSdlcBuffer;
	      
              j=0;
              CountLength=0;
              RxSdlcBuf[j]=Dtemp[i];
              CountLength++;
              j++;
              SdlcStartMoveFlag=1;
              continue;
            }
        }
      else
        {
          if ( SdlcStopSymbolFlag==1 && (Dtemp[i]==0xB1 || Dtemp[i]==0xB2) )
            {
              RxSdlcBuf[j]=Dtemp[i];
              CountLength++;
              j++;
              SdlcStartMoveFlag=0;
              SdlcStopSymbolFlag=0;

              /*
              //for testing
              int x;
              for ( x=0; x< CountLength; x++,y++  )
                {
                  Test[y]=RxSdlcBuf[x];

                }

              Test[y]='#';
              y++;
              Test[y]=CountLength;
              y++;
              */
              //SdlcRXPacket (RxSdlcBuf, CountLength);//解析封包
              SdlcRxSyncConversion(RxSdlcBuf, CountLength);//將資料從v.80的格式轉成SDLC
//            BSP_Free(RxSdlcBuf);
              RxSdlcBuf=NULL;
              break;
            }
          if (SdlcStopSymbolFlag==1)
            {
              SdlcStopSymbolFlag=0;
            }
          if (Dtemp[i]==0x19 )
            {
              RxSdlcBuf[j]=Dtemp[i];
              CountLength++;
              j++;
              SdlcStopSymbolFlag=1;
              continue;
            }
          RxSdlcBuf[j]=Dtemp[i];
          CountLength++;
          j++;
        }
    }
}

//******************************************************************************
// 程式模組編號:SDLC_001_002
// 副程式名稱： SdlcRxSyncConversion
// 功能：將接收進來的資料由v.80格式轉換成至SDLC
//******************************************************************************

void SdlcRxSyncConversion(UCHAR *temp , unsigned int length)
{
  UINT i=0;
  UINT j=0;
  unsigned int len;

  UCHAR *AnalyTemp;

//AnalyTemp = (UCHAR*) BSP_Malloc( (UINT32) (length * sizeof (UCHAR)));
  AnalyTemp = SdlcWorkBuffer;

  //陣列的最後兩個字元19 B1不轉換
  while(i<length)
    {
      if (i<length-2)
        {
          if (temp[i]==0x19)
            {
              if (temp[i+1]==0x5C)
                {
                  AnalyTemp[j]=0x19;
                  j++;
                  i+=2;
                }
              else if (temp[i+1]==0x76)
                {
                  AnalyTemp[j]=0x99;
                  j++;
                  i+=2;
                }
              else if (temp[i+1]==0xA0)
                {
                  AnalyTemp[j]=0x11;
                  j++;
                  i+=2;
                }
              else if (temp[i+1]==0xA1)
                {
                  AnalyTemp[j]=0x13;
                  j++;
                  i+=2;
                }
              else if (temp[i+1]==0x5D)
                {
                  AnalyTemp[j]=0x19;
                  AnalyTemp[j+1]=0x19;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0x77)
                {
                  AnalyTemp[j]=0x99;
                  AnalyTemp[j+1]=0x99;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xA2)
                {
                  AnalyTemp[j]=0x11;
                  AnalyTemp[j+1]=0x11;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xA3)
                {
                  AnalyTemp[j]=0x13;
                  AnalyTemp[j+1]=0x13;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xA4)
                {
                  AnalyTemp[j]=0x19;
                  AnalyTemp[j+1]=0x99;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xA5)
                {
                  AnalyTemp[j]=0x19;
                  AnalyTemp[j+1]=0x11;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xA6)
                {
                  AnalyTemp[j]=0x19;
                  AnalyTemp[j+1]=0x13;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xA7)
                {
                  AnalyTemp[j]=0x99;
                  AnalyTemp[j+1]=0x19;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xA8)
                {
                  AnalyTemp[j]=0x99;
                  AnalyTemp[j+1]=0x11;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xA9)
                {
                  AnalyTemp[j]=0x99;
                  AnalyTemp[j+1]=0x13;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xAA)
                {
                  AnalyTemp[j]=0x11;
                  AnalyTemp[j+1]=0x19;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xAB)
                {
                  AnalyTemp[j]=0x11;
                  AnalyTemp[j+1]=0x99;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xAC)
                {
                  AnalyTemp[j]=0x11;
                  AnalyTemp[j+1]=0x13;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xAD)
                {
                  AnalyTemp[j]=0x13;
                  AnalyTemp[j+1]=0x19;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xAE)
                {
                  AnalyTemp[j]=0x13;
                  AnalyTemp[j+1]=0x99;
                  j+=2;
                  i+=2;
                }
              else if (temp[i+1]==0xAF)
                {
                  AnalyTemp[j]=0x13;
                  AnalyTemp[j+1]=0x11;
                  j+=2;
                  i+=2;
                }
              else
                {
                  AnalyTemp[j]=temp[i];
                  j++;
                  i++;
                }
            }
          else
            {
              AnalyTemp[j]=temp[i];
              j++;
              i++;
            }
        }
      else
        {
          AnalyTemp[j]=temp[i];
          j++;
          i++;
        }
    }
  len=j;
  // printf("AnalyTemp=\n");
  // for(int g=0;g<len;g++)
  //   printf(" 0x%02x ",AnalyTemp[g]);
  SdlcRXPacket (AnalyTemp, len);//解析封包
//BSP_Free(AnalyTemp);
  AnalyTemp=NULL;
}

//******************************************************************************
// 程式模組編號:SDLC_001_003
// 副程式名稱： SdlcRXPacket
// 功能：解析接收進來的封包
//******************************************************************************

void SdlcRXPacket (UCHAR *Vtemp, unsigned int len)
{

  if (Vtemp[0]==SdlcAddress)
    {
      printf("Vtemp[0]==SdlcAddress,Vtemp[0]=0x%x\n",Vtemp[0]);
      printf("Vtemp[1]=0x%x\n",Vtemp[1]);
      if (!(Vtemp[1] & 0x01))
        {
          //封包格式為 I-frame
          DEBUGPRINTF( "!!!!!I-frame detected\r\n");
          struct Packet ipacket;
          ipacket.length=len;
          ipacket.packetHead.IHeader = (struct IFrameHeader *) &Vtemp[0];
          ipacket.Data = &Vtemp[2];
          ipacket.packetTail = (struct Tailer *) &Vtemp[len-2];
          printf("rx data=\n");
          for(int g=0;g<len;g++)
            printf(" 0x%x ",*(ipacket.Data+g));
          //呼叫處理I-frame的函式
          SdlcRxIframeResponse(&ipacket);

          //DEBUGPRINTF( "I-frame detected\r\n");
        }
      else if (Vtemp[1] & 0x02)
        {
          //封包格式為 U-frame
          struct Packet upacket;
          upacket.length=len;
          upacket.packetHead.UHeader = (struct UFrameHeader *) &Vtemp[0];
          upacket.packetTail = (struct Tailer *) &Vtemp[len-2];

          // #ifdef MDM_DEBUG
          // DEBUGPRINTF( "!!!!!U-frame detected\r\n");
          // #endif
          DEBUGPRINTF( "!!!!!U-frame Vtemp[1] & (~0x10)=0x%x\r\n",Vtemp[1] & (~0x10));
          switch (Vtemp[1] & (~0x10))
            {
            case 0x03:
              {
                //UI
                break;
              }
            case 0x43:
              {
                //DISC
                break;
              }
            case 0x83:
              {
                //SNRM
                SdlcSendCount=0;
                SdlcReceiveCount=0;
                SdlcNRPre=0;
                SdlcTxUASend();
                
                break;
              }
            case 0xE3:
              {
                //TEST
                break;
              }
            case 0x07:
              {
                //SIM
                SdlcSendCount=0;
                SdlcReceiveCount=0;
                SdlcNRPre=0;
                break;
              }
            case 0xC7:
              {
                //CFGR
                break;
              }
            case 0xAF:
              {
                //XID
                break;
              }
            default:
              {
                //Unknown U-frame
                break;
              }
            }
        }
      else
        {
          //封包格式為 S-frame

          struct Packet spacket;
          spacket.length=len;
          spacket.packetHead.SHeader = (struct SFrameHeader *) &Vtemp[0];
          spacket.packetTail = (struct Tailer *) &Vtemp[len-2];

          DEBUGPRINTF( "!!!!!S-frame signal detected\r\n");
          // DEBUGPRINTF( "!!!!!S-frame Vtemp[1] & 0x0f=0x%x\r\n",Vtemp[1] & 0x0f);
          switch (Vtemp[1] & 0x0f)
            {
            case 0x01:
              {
                /*
                RR
                 當接收到RR時代表SDLC連線成功
                */
                mdmStatusCode = mdmConnected;
                mdmCanTx = TRUE;
                // DEBUGPRINTF( "!!!!!S-frame SDLC mdmConnected\r\n");
                SdlcRxRRResponse(&spacket);
                break;
              }
            case 0x05:
              {
                //RNR
                SdlcRxRRResponse(&spacket);
                break;
              }
            case 0x09:
              {
                //REJ
                SdlcRxRRResponse(&spacket);
                break;
              }
            default:
              {
                //Unknown S-frame
                break;
              }

            }
        }
    }
}

//******************************************************************************
// 程式模組編號:SDLC_001_004
// 副程式名稱： SdlcRxRRResponse
// 功能：處理收到RR封包的後續動作
//******************************************************************************

void SdlcRxRRResponse(struct Packet *Spacket)
{

  // #ifdef MDM_DEBUG
  DEBUGPRINTF( "!!!!!Receive RR Nr:%d \r\n", Spacket->packetHead.SHeader->NR );
  // #endif

  //Check 收進來的S-frame的Nr
  //當接收到的NR比前一次接收的NR要大，此差值代表對方實際收到的I-frame數

  int RemoveCount=(Spacket->packetHead.SHeader->NR)-SdlcNRPre;

  if ((Spacket->packetHead.SHeader->NR==0) && (SdlcSendCount != Spacket->packetHead.SHeader->NR))
    {
      RemoveCount=RemoveCount+8;
    }
  //把對方實際接收到的frame從queue移除
  while( RemoveCount != 0 && mdmTxqueue.front != NULL)
    {
      mdm_queue_remove(&mdmTxqueue);
      RemoveCount--;
    }

  SdlcNRPre=Spacket->packetHead.SHeader->NR;//紀錄此次接收到NR以便與下一次接收到的NR作比較

  //當trial與front所指到的node不同，代表這些差距的node對方沒有接收到，要重送
  if (mdmTxqueue.front != mdmTxqueue.trial)
    {
       DEBUGPRINTF( "!!!!!Receive RR Need to resend \r\n");
      mdmTxqueue.trial=mdmTxqueue.front;
      SdlcSendCount=Spacket->packetHead.SHeader->NR;
    }

  //準備回傳的封包
  //如果RR中的P/F bit不為0，才回傳
  if (Spacket->packetHead.SHeader->PF ==1)
    {
      //檢查TX queue中有無資料，有則回傳I-frame，無則回丟RR
      if (mdmTxqueue.trial != NULL)
        {
          printf("SdlcTxIframeSend();\n");
          //TX queue中有資料
          //呼叫打包TX I-frame的函式
          //送出I-frame
          SdlcTxIframeSend();
        }
      else
        {
          SdlcTxRRSend();
        }
    }

}


//******************************************************************************
// 程式模組編號:SDLC_001_005
// 副程式名稱： SdlcRxIframeResponse
// 功能：處理收到I-frame封包的後續動作
//******************************************************************************

void SdlcRxIframeResponse(struct Packet *Ipacket)
{
  UCHAR                     api_status = apiOK;
  
  #ifdef MDM_DEBUG
  DEBUGPRINTF( "!Receive I-frame Ns:%d \r\n", Ipacket->packetHead.IHeader->NS );
  #endif
  //1.判斷收進來的I-frame，如果Flag為0xB1則收進RX queue中
  //2.Check 收進來的I-frame的Nr，如果正確則釋放RX queue中的資料
  //3.如果Nr不等於(SdlcSendCount+1)，則資料要重傳
  if (Ipacket->packetTail->Flag ==0xB1)
    {
      SdlcReceiveCount++;
      if (SdlcReceiveCount==8)
        {
          SdlcReceiveCount=0;
        }
      //將收進I-frame的data放到RX queue中
//    mdm_queue_insert( &mdmRxqueue, Ipacket->Data, (Ipacket->length)-6*sizeof(UCHAR), &api_status);	//把表頭和結尾拿掉, CU(1)+CTRL(1)+CRC(2)+EM(1)+FLAG(1)
      mdm_queue_insert( &mdmRxqueue, Ipacket->Data, (Ipacket->length)-4*sizeof(UCHAR), &api_status);	// 2010-07-23, CU(1)+CTRL(1)+EM(1)+FLAG(1)

      //Check 收進來的I-frame的Nr
      //當接收到的NR比前一次接收的NR要大，此差值代表對方實際收到的I-frame數
      int RemoveCount=(Ipacket->packetHead.IHeader->NR)-SdlcNRPre;

      if ((Ipacket->packetHead.IHeader->NR==0) && (SdlcSendCount != Ipacket->packetHead.IHeader->NR))
        {
          RemoveCount=RemoveCount+8;
        }
      //把對方實際接收到的frame從queue移除
      while( RemoveCount != 0 && mdmTxqueue.front!=NULL)
        {
          mdm_queue_remove(&mdmTxqueue);
          RemoveCount--;
        }
      SdlcNRPre=Ipacket->packetHead.IHeader->NR;//紀錄此次接收到NR以便與下一次接收到的NR作比較

      //當trial與front所指到的node不同，代表這些差距的node對方沒有接收到，要重送
      if (mdmTxqueue.front != mdmTxqueue.trial)
        {
          mdmTxqueue.trial=mdmTxqueue.front;
          SdlcSendCount=Ipacket->packetHead.IHeader->NR;
        }
    }

  //當I-frame的flag為0xB2的情況
  //Nr不要加代表有error


  //準備回傳的封包
  //如果I-frame中的P/F bit為0，則不回傳
  //如果Nr不等於(SdlcSendCount+1)，則資料要重傳
  //檢查RX queue中有無資料，有則回傳I-frame，無則回丟RR
  if (Ipacket->packetHead.IHeader->PF==1)
    {
      //檢查TX queue中有沒有資料，有則準備送出
      if (mdmTxqueue.trial != NULL)
        {
          SdlcTxIframeSend();
        }

      else
        {
          SdlcTxRRSend();
        }
    }
}

//******************************************************************************
// 程式模組編號:SDLC_001_006
// 副程式名稱： SdlcTxRRSend
// 功能：傳送RR封包
//******************************************************************************

void SdlcTxRRSend()
{
  struct Packet RRpacket;
  MODEM_STATUS				eRetCode;
  UCHAR  *pTXRRTemp;
  UCHAR	 TXRRTemp[16];
  UCHAR	 TXRRTemp2[16];
  ULONG	 i;
  
//pTXRRTemp=(UCHAR *) BSP_Malloc( (UINT32) 4*sizeof (UCHAR) );
  pTXRRTemp=TXRRTemp;

  RRpacket.length=4;
  RRpacket.packetHead.SHeader = (struct SFrameHeader *) pTXRRTemp;
  RRpacket.packetTail = (struct Tailer *) &pTXRRTemp[2];

  RRpacket.packetHead.SHeader->address=SdlcAddress;
  RRpacket.packetHead.SHeader->NR=SdlcReceiveCount;
  RRpacket.packetHead.SHeader->PF=1;
  RRpacket.packetHead.SHeader->code=0x00;
  RRpacket.packetHead.SHeader->type=0x01;
  RRpacket.packetTail->EM=0x19;
  RRpacket.packetTail->Flag=0xB1;

  // PATCH: 2010-06-17, check and convert for "Control" byte of SDLC RR-frame
  TXRRTemp2[0] = TXRRTemp[0];	// address
  switch( pTXRRTemp[1] )
        {
        case 0x19:
        
             TXRRTemp2[1] = 0x19;
             TXRRTemp2[2] = 0x5C;
             TXRRTemp2[3] = 0x19;
             TXRRTemp2[4] = 0xB1;
             RRpacket.length = 5;
             break;
             
        case 0x99:
        
             TXRRTemp2[1] = 0x19;
             TXRRTemp2[2] = 0x76;
             TXRRTemp2[3] = 0x19;
             TXRRTemp2[4] = 0xB1;
             RRpacket.length = 5;
             break;
             
        case 0x11:
        
             TXRRTemp2[1] = 0x19;
             TXRRTemp2[2] = 0xA0;
             TXRRTemp2[3] = 0x19;
             TXRRTemp2[4] = 0xB1;
             RRpacket.length = 5;
             break;
             
        case 0x13:

             TXRRTemp2[1] = 0x19;
             TXRRTemp2[2] = 0xA1;
             TXRRTemp2[3] = 0x19;
             TXRRTemp2[4] = 0xB1;
             RRpacket.length = 5;
             break;
             
        default:
	     
	     memmove( TXRRTemp2, TXRRTemp, 4 );
             break;
        }

  eRetCode = cnxt_modem_write( ( char * )&TXRRTemp2, &(RRpacket.length) );
  if (eRetCode != MODEM_OK)
    {
      DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error_RR(%d)\n\r", eRetCode );
    }
    
  // #ifdef MDM_DEBUG  
  DEBUGPRINTF( "!!!!!Send RR Nr:%d \r\n", RRpacket.packetHead.SHeader->NR );
  // #endif
  // printf("RR packet:\n");
  for(int g=0;g<RRpacket.length;g++)
    printf(" 0x%02x ",TXRRTemp2[g]);
  printf("\n");
//BSP_Free(pTXRRTemp);
  pTXRRTemp=NULL;
}

//******************************************************************************
// 程式模組編號:SDLC_001_007
// 副程式名稱： SdlcTxUASend
// 功能：傳送UA封包
//******************************************************************************

void SdlcTxUASend()
{
  struct Packet UApacket;
  MODEM_STATUS				eRetCode;
  UCHAR  *pTXUATemp;
  UCHAR  TXUATemp[16];

  
//pTXUATemp=(UCHAR *) BSP_Malloc( (UINT32) 4*sizeof (UCHAR) );
  pTXUATemp=TXUATemp;

  UApacket.length=4;
  UApacket.packetHead.UHeader = (struct UFrameHeader *) pTXUATemp;
  UApacket.packetTail = (struct Tailer *) &pTXUATemp[2];

  UApacket.packetHead.UHeader->address=SdlcAddress;
  UApacket.packetHead.UHeader->code1=0x03;
  UApacket.packetHead.UHeader->PF=0x01;
  UApacket.packetHead.UHeader->code2=0x00;
  UApacket.packetHead.UHeader->type=0x03;
  UApacket.packetTail->EM=0x19;
  UApacket.packetTail->Flag=0xB1;

/*
	UApacket.length=6;
	TXUATemp[0]  = 0x19;
	TXUATemp[1]  = 0xB2;
	TXUATemp[2]  = 0x30;
	TXUATemp[3]  = 0x73;
	TXUATemp[4]  = 0x19;
	TXUATemp[5]  = 0xB1;
*/

  eRetCode = cnxt_modem_write( ( char * ) pTXUATemp, &(UApacket.length));
  if (eRetCode != MODEM_OK)
    {
      DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error(%d)\n\r", eRetCode );
    }
//BSP_Free(pTXUATemp);
  pTXUATemp=NULL;
}

void SdlcTxSNRM()
{
  struct Packet UApacket;
  MODEM_STATUS				eRetCode;
  UCHAR  *pTXUATemp;
  UCHAR  TXUATemp[16];
  
//pTXUATemp=(UCHAR *) BSP_Malloc( (UINT32) 4*sizeof (UCHAR) );
  pTXUATemp=TXUATemp;

  UApacket.length=4;
  UApacket.packetHead.UHeader = (struct UFrameHeader *) pTXUATemp;
  UApacket.packetTail = (struct Tailer *) &pTXUATemp[2];

  UApacket.packetHead.UHeader->address=SdlcAddress;
  UApacket.packetHead.UHeader->code1=0x03;
  UApacket.packetHead.UHeader->PF=0x01;
  UApacket.packetHead.UHeader->code2=0x00;
  UApacket.packetHead.UHeader->type=0x03;
  UApacket.packetTail->EM=0x19;
  UApacket.packetTail->Flag=0xB1;

/*  
	UApacket.length=10;
	TXUATemp[0]  = 0x19;
	TXUATemp[1]  = 0xB2;
	TXUATemp[2]  = 0x30;
	TXUATemp[3]  = 0x93;
	TXUATemp[4]  = 0x19;
	TXUATemp[5]  = 0xB1;
	TXUATemp[6]  = 0x19;
	TXUATemp[7]  = 0xB0;
	TXUATemp[8]  = 0x19;
	TXUATemp[9]  = 0xB2;
*/

	UApacket.length=4;
	TXUATemp[0]  = 0x30;
	TXUATemp[1]  = 0x93;
	TXUATemp[2]  = 0x19;
	TXUATemp[3]  = 0xB1;	

  eRetCode = cnxt_modem_write( ( char * ) pTXUATemp, &(UApacket.length));
  if (eRetCode != MODEM_OK)
    {
      DEBUGPRINTF("write_to_modem_task: cnxt_modem_write error(%d)\n\r", eRetCode );
    }
//BSP_Free(pTXUATemp);
  pTXUATemp=NULL;
}

//******************************************************************************
// 程式模組編號:SDLC_001_008
// 副程式名稱： SdlcTxIframeSend
// 功能：傳送information封包
//******************************************************************************

//傳送information封包
void SdlcTxIframeSend()
{
  /*將TX queue的資料先打包成I-frame
  P/F值的設定取決於TX queue中還有沒有資料要傳輸
  計數Ns*/

  UCHAR  *pTxPacketTemp;
  struct Packet TxIPacket;

//pTxPacketTemp=(UCHAR *) BSP_Malloc( (UINT32) ( ((mdmTxqueue.trial->buffer_size)+4)*sizeof (UCHAR) ));
  pTxPacketTemp=TxSdlcBuffer;
  TxIPacket.length=(mdmTxqueue.trial->buffer_size)+4;
  memcpy( &pTxPacketTemp[2], mdmTxqueue.trial->buffer, mdmTxqueue.trial->buffer_size);
  TxIPacket.packetHead.IHeader = (struct IFrameHeader *) pTxPacketTemp;
  TxIPacket.Data = &pTxPacketTemp[2];
  TxIPacket.packetTail = (struct Tailer *) &pTxPacketTemp[(TxIPacket.length)-2];

  TxIPacket.packetHead.IHeader->address=SdlcAddress;
  TxIPacket.packetHead.IHeader->NR=SdlcReceiveCount;
  TxIPacket.packetHead.IHeader->NS=SdlcSendCount;
  TxIPacket.packetHead.IHeader->type=0x00;
  TxIPacket.packetTail->EM=0x19;
  TxIPacket.packetTail->Flag=0xB1;

  //如果TX queue的下個node不為空，則P/F為0,代表還要繼續傳送I-frame

  if (mdmTxqueue.trial->next != NULL)
    {
      TxIPacket.packetHead.IHeader->PF=0;
    }
  else
    {
      TxIPacket.packetHead.IHeader->PF=1;
    }
  //轉換封包，以避免封包內的資料被v.80轉換
  // #ifdef MDM_DEBUG
  DEBUGPRINTF( "!Send I-frame Ns:%d \r\n", TxIPacket.packetHead.IHeader->NS );
  // #endif
  
  SdlcTxSyncConversion(pTxPacketTemp , TxIPacket.length);


//BSP_Free(pTxPacketTemp);
  pTxPacketTemp=NULL;

}

//******************************************************************************
// 程式模組編號:SDLC_001_009
// 副程式名稱： SdlcTxSyncConversion
// 功能：將欲傳送的資料預先轉換，以避免欲傳輸的資料被v.80層轉換
// 20090902_Richard:將0x19皆換為0x19 0x5c
//******************************************************************************

void SdlcTxSyncConversion(UCHAR *DataTemp , unsigned int DataLen)
{
  UINT i=0;
  UINT j=0;
  unsigned int len;

  UCHAR *TxDataTemp;

//TxDataTemp = (UCHAR*) BSP_Malloc( (UINT32) ((DataLen+2048) * sizeof (UCHAR)));
  TxDataTemp = SdlcTempBuffer;
  while(i<DataLen)
    {
      if (i<DataLen-2)
        {
          if( DataTemp[i] == 0x19 )
            {
            TxDataTemp[j] = 0x19;
            TxDataTemp[j+1] = 0x5C;
            j+=2;
            i++;   
            }
          else
            {
            if( DataTemp[i] == 0x99 )	// PATCH: 2009/09/03
              {
              TxDataTemp[j]= 0x99;
              TxDataTemp[j+1]= 0x76;
              j+=2;
              i++;
              }
            else
              {
              TxDataTemp[j]=DataTemp[i];
              j++;
              i++;
              }
            }
        }
      else
        {
          TxDataTemp[j]=DataTemp[i];
          j++;
          i++;
        }
    }
  len=j;

  //將封包send出去
  mdmCountTX=len;
  pmdmTxBuf=TxDataTemp;
  write_to_modem_task();	// 2010-07-26

  SdlcSendCount++;
  if (SdlcSendCount==8)
    {
      SdlcSendCount=0;
    }

//  OS_SLEEP(1);
  NosSleep(1);
//BSP_Free(TxDataTemp);
  TxDataTemp = NULL;
  mdmTxqueue.trial=mdmTxqueue.trial->next;

  //如果Tx queue中還有資料，則不間斷，繼續發送下個I-frame
  if (mdmTxqueue.trial != NULL)
    {
//      OS_SLEEP(1);
      NosSleep(1);
      SdlcTxIframeSend();
    }
}