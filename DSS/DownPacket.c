//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : DSS (for no file system)                                   **
//**  PRODUCT  : AS320B                                                     **
//**                                                                        **
//**  FILE     : DownPacket.C                                               **
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
#include "Down.h"
#include "DownInterface.h"
#include "DownPacket.h"
#include "DownAnaly.h"

// ---------------------------------------------------------------------------
// FUNCTION: DownReq
//           Request host for start transmission
// INPUT   : TransMode
//           0:UART
//           1:Modem
//           2:Lan
//
//           ReqItem:struct DownReqFormat
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
// RETURN  : apiOK
//               apiFailed
// ---------------------------------------------------------------------------

UCHAR DownReq ( UCHAR TransMode, DOWREQPAC *ReqItem)
{
  UCHAR      ApiStatus;
  UCHAR     *pLRC;
  UINT       i;
  static struct DownTxReqPac ReqPacket;

  //ReqPacket.SOH = 0x01;
  //ReqPacket.Address = 0x10100000;

  ReqPacket.SOH = 0x01;

  // ReqPacket.Length = sizeof( ReqPacket )-6;
  ReqPacket.Length = sizeof( ReqPacket )-4;//modified by West to fit PC JAVA program DSS lite.
  ReqPacket.Type = DownRxReq ;//0x80

  memcpy( (UCHAR *) &(ReqPacket.Req), ReqItem, sizeof(ReqPacket.Req) );

  ReqPacket.LRC = 0;

  //Caculate LRC
  pLRC = ( UCHAR * )&(ReqPacket.Length);

  for ( i=0; i < ( sizeof(ReqPacket.Length)+sizeof(ReqPacket.Type)+ sizeof(ReqPacket.Req) ); i++ )
    {
      ReqPacket.LRC ^= *pLRC++;
    }


  ApiStatus = api_down_txstring( TransMode, (UCHAR *)&ReqPacket, sizeof(ReqPacket) );
  printf("DownReq ApiStatus=%d\n",ApiStatus);

  return ApiStatus;
}

// ---------------------------------------------------------------------------
// FUNCTION: DownAck
//           Send "ACK" packet
// INPUT   : TransMode
//           0:UART
//           1:Modem
//           2:Lan
//
// RETURN  : apiOK
//               apiFailed
// ---------------------------------------------------------------------------

UCHAR DownAck( UCHAR TransMode )
{
  UCHAR    ApiStatus;
  UCHAR    *pLRC;
  UINT     i;
  struct DownResPack AckPacket;


  AckPacket.SOH = 0x01;
  AckPacket.Length = 1;
  AckPacket.Type = DownTxAck;//0x06
  AckPacket.LRC = 0;

  //Caculate LRC
  pLRC = ( UCHAR * ) &( AckPacket.Length );
  for ( i=0; i < 3; i++ )
    {
      AckPacket.LRC ^= *pLRC++;
    }
  ApiStatus = api_down_txstring( TransMode, (UCHAR *)&AckPacket, sizeof(AckPacket) );

  return ApiStatus;
}

// ---------------------------------------------------------------------------
// FUNCTION: DownAbort
//           Send "ABORT" packet
// INPUT   : TransMode
//           0:UART
//           1:Modem
//           2:Lan
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
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
/*
UCHAR DownAbort( UCHAR TransMode )
{
  UCHAR    ApiStatus;
  UCHAR    *pLRC;
  UINT     i;
  struct DownResPack AbortPacket;


  AbortPacket.SOH = 0x01;
  AbortPacket.Length = 1;
  AbortPacket.Type = 0xFF;
  AbortPacket.LRC = 0;

  //Caculate LRC
  pLRC = ( UCHAR * ) &( AbortPacket.Length );
  for ( i=0; i < 3; i++ )
    {
      AbortPacket.LRC ^= *pLRC++;
    }
  ApiStatus = api_down_txstring( TransMode, (UCHAR *)&AbortPacket, sizeof(AbortPacket) );

  return ApiStatus;
}
*/
// ---------------------------------------------------------------------------
// FUNCTION: DownNak
//           Send "Nak" packet
// INPUT   : TransMode
//           0:UART
//           1:Modem
//           2:Lan
//
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------

UCHAR DownNak( UCHAR TransMode )
{
  UCHAR    ApiStatus;
  UCHAR    *pLRC;
  UINT     i;
  struct DownResPack NakPacket;


  NakPacket.SOH = 0x01;
  NakPacket.Length = 1;
  NakPacket.Type = DownRxNak;//0x15
  NakPacket.LRC = 0;

  //Caculate LRC
  pLRC = ( UCHAR * ) &( NakPacket.Length );
  for ( i=0; i < 3; i++ )
    {
      NakPacket.LRC ^= *pLRC++;
    }
  ApiStatus = api_down_txstring( TransMode, (UCHAR *)&NakPacket, sizeof(NakPacket) );

  return ApiStatus;
}

// ---------------------------------------------------------------------------
// FUNCTION: DownResend
//           Request host for resend transmission
// INPUT   : TransMode
//           0:UART
//           1:Modem
//           2:Lan
//
// RETURN  : apiOK
//               apiFailed
//
// 
// ---------------------------------------------------------------------------

UCHAR DownRes ( UCHAR TransMode, DOWREQPAC *ReqItem, ULONG offset )
{
  UCHAR      ApiStatus;
  UCHAR     *pLRC;
  UINT       i;
  static struct DownTxReqPac ReqPacket;

  //ReqPacket.SOH = 0x01;
  //ReqPacket.Address = 0x10100000;

  ReqPacket.SOH = 0x01;

  ReqPacket.Length = sizeof( ReqPacket )-4;
  ReqPacket.Type = DownErrResend;//0x81
  ReqPacket.Req.Offset = offset;

  memcpy( (UCHAR *) &(ReqPacket.Req), ReqItem, sizeof(ReqPacket.Req) );

  ReqPacket.LRC = 0;

  //Caculate LRC
  pLRC = ( UCHAR * )&(ReqPacket.Length);

  for ( i=0; i < ( sizeof(ReqPacket.Length)+sizeof(ReqPacket.Type)+ sizeof(ReqPacket.Req) ); i++ )
    {
      ReqPacket.LRC ^= *pLRC++;
    }


  ApiStatus = api_down_txstring( TransMode, (UCHAR *)&ReqPacket, sizeof(ReqPacket) );


  return ApiStatus;
}
// ---------------------------------------------------------------------------
// FUNCTION: DownDisconnect
//           Request "disconnect" packet
// INPUT   : TransMode
//           0:UART
//           1:Modem
//           2:Lan
//
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
/*UCHAR DownDisconnect( UCHAR TransMode )
{
  UCHAR    ApiStatus;
  UCHAR    *pLRC;
  UINT     i;
  struct DownResPack DisconPacket;


  DisconPacket.SOH = 0x01;
  DisconPacket.Length = 1;
  DisconPacket.Type = 0x00;
  DisconPacket.LRC = 0;

  //Caculate LRC
  pLRC = ( UCHAR * ) &( DisconPacket.Length );
  for ( i=0; i < 3; i++ )
    {
      DisconPacket.LRC ^= *pLRC++;
    }
  ApiStatus = api_down_txstring( TransMode, (UCHAR *)&DisconPacket, sizeof(DisconPacket) );

  return ApiStatus;
}*/
