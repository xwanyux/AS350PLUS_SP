#ifndef DOWNPACKET_H_
#define DOWNPACKET_H_

#include "POSAPI.h"
#include "bsp_types.h"
#include "DownInterface.h"
#include "DSS.h"
//struct DownReqPacket
struct DownTxReqPac
  {
    UCHAR    SOH;
    UINT     Length;
    UCHAR    Type;//MSG = Type + Data
    //UCHAR    Data[84];
    DOWREQPAC Req;
    UINT8    LRC;
  }__attribute__((packed));;

//Size is 80 bytes
struct DownDataField
 {
    UCHAR TID[8];
    UCHAR BID[4];
    UCHAR TIME[12];
    UCHAR AID[8];
    UCHAR FID[32];
    UCHAR VER[8];
    UCHAR DATE[8];
  };



struct DownRxReqPac
  {
    UCHAR    SOH;
    UINT     Length;
    UCHAR    Type;//MSG = Type + Data + Size
    //UCHAR    Data[80];
    struct DownDataField Data;
    ULONG    Size;//file size
    UCHAR    LRC;
  }__attribute__((packed));;




/*ACK, Abort, NAK(Resend), Disconnect*/
//struct DownResPacket
struct DownResPack
  {
    UCHAR    SOH;
    UINT     Length;
    UCHAR    Type;
    UCHAR    LRC;
  }__attribute__((packed));;

//struct DownDataPacket
struct DownDataPac
  {
    UCHAR    SOH;
    UINT     Length;
    UCHAR    Type;//MSG = Type + Offset + Data
    ULONG    Offset;
    UCHAR    *Data;
    UCHAR    LRC;
  }__attribute__((packed));;

UCHAR DownReq ( UCHAR TransMode, DOWREQPAC *ReqItem );
UCHAR DownAck( UCHAR TransMode );
UCHAR DownAbort( UCHAR TransMode );
UCHAR DownNak( UCHAR TransMode );
UCHAR DownRes ( UCHAR TransMode, DOWREQPAC *ReqItem, ULONG offset );
UCHAR DownDisconnect( UCHAR TransMode );

#endif /*DOWNPACKET_H_*/
