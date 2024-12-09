#ifndef DOWNINERFACE_H_
#define DOWNINERFACE_H_

#include "POSAPI.h"
#include "DSS.h"

#define MaxDowPackSize DSS_MAX_APP_SIZE
#define MaxUpPackSize 100

#define DownUartMode  0x00
#define DownModemMode 0x01
#define DownLanMode   0x02

#define DownDataStart 0x01
#define DownData      0x02
#define DownDataEnd   0x03

#define DownContinue 0x02

#define apiFileNotFound			0x03
#define apiTimeOut                        0x04
#define apiConnectFailed               0x05
#define apiNoLine                          0x06
#define apiFileError                       0x07
#define apiOffsetError                   0x08
#define apiEraseFailed                  0x09
#define apiVerifyFailed                 0x10
#define apiWriteFailed                 0x11
#define __AP                 0x80
#define __SP                 0x81
#define __OT                 0x82

// #ifdef	_SDRAM_8MB_
// #define DownSdramStartAddr 0x205F0000	//  8MB SDRAM: 0x205F0000~0x207FFFFF, the space is 3MB-512KB
// #endif

//#ifdef	_SDRAM_16MB_
//#define DownSdramStartAddr 0x20DF0000	// 16MB SDRAM: 0x20DF0000~0x20FFFFFF, the space is 2M+64K
//#endif

// #ifdef	_SDRAM_16MB_
// #define DownSdramStartAddr 0x21100000	// 16MB SDRAM: 0x21100000~0x215FFFFF, the space is 5MB
// #endif

/*
struct DownReqFormat
{
	UCHAR TID[8];
	UCHAR BID[4];
	UCHAR Reserv[12];
	UCHAR AID[8];
	UCHAR FID[32];
	UCHAR VER[8];
	UCHAR DATE[8];
	ULONG Offset;
}__attribute__((packed));;

typedef struct DownReqFormat DOWREQPAC;
*/

struct DownMsgFormat
  {
    UCHAR Type;
    ULONG Offset;
    UCHAR Data;
  }__attribute__((packed));;

typedef struct DownMsgFormat DOWNMSG;

struct DownDataFormat
  {
    UCHAR SOH;
    UINT  Len;
    DOWNMSG Msg;
  }__attribute__((packed));;


typedef struct DownDataFormat DOWNDATAPAC;

extern DOWNDATAPAC glpSeek;
#define DownSdramStartAddr &glpSeek
UCHAR api_dss_download ( UCHAR channel, DOWREQPAC *ReqItem );//called by AP

UCHAR DownCom ( UCHAR TransMode, DOWREQPAC *ReqItem );
UCHAR DownReceive ( UCHAR TransMode, UCHAR *Reaction, UCHAR *pDownRxBuf );
UCHAR DownReGet ( UCHAR TransMode, DOWREQPAC *ReqItem, ULONG *Offset, ULONG *sdramAddr, UINT resendCount );
ULONG DownToSdram ( const void *pSource, ULONG size );
UCHAR DownVerifyData( UCHAR TransMode, DOWREQPAC *ReqItem );
UCHAR DownProgramProcess( DOWREQPAC *ReqItem );
UCHAR DownBurnToFlash( ULONG* nextOffset );
UCHAR DownParaToFile( UCHAR *buffer, UINT bufferSize, UINT *outputSize );
UCHAR DownMain ( UCHAR channel, DOWREQPAC *ReqItem, UCHAR *buffer, UINT bufferSize, UINT *outputSize );
UCHAR DownUi ( UCHAR channel, DOWREQPAC *ReqItem );
void DownRefreshRTC();

void Down_monitor_burn_task(void);

#endif /*DOWNINERFACE_H_*/
