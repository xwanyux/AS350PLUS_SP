#ifndef DOWNANALY_H_
#define DOWNANALY_H_

#include "POSAPI.h"
//Define the receive type
#define DownRxReq           0x80
//#define DownDiscon          0x00
#define DownRxStart         0x01
#define DownRxData          0x02
#define DownRxEnd           0x03
#define DownRxAbort         0XFF
#define DownRxNak           0x15
#define DownErrResend       0x81

//Define the behavior of reaction

#define DownTxAck           0x06
//#define DownTxAbort         0XFF
#define DownTxNak           0x10
#define DownStop            0x20
#define DownABORT            0x25
#define DownSuccess         0x30
#define DownReqResend          0x40
#define DownFailed          0x50
#define DownTxReq           0x60
#define DownAlwaysReceive   0x70
#define DownPacketToSdram  0x75


//extern ULONG     DownFileSize;
//extern ULONG     DownReceiveSize;

UCHAR DownAnalysis( UCHAR TransMode, const UCHAR *, ULONG , UCHAR *);



#endif /*DOWNANALY_H_*/
