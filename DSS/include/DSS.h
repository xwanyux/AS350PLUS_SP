#ifndef DSS_H_
#define DSS_H_

#include "POSAPI.h"

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

extern UCHAR api_dss_interface();//DSS with user interface (for downloading AP only)
extern UCHAR api_dss_ap ( UCHAR channel, DOWREQPAC *ReqItem );//DSS for download AP without UI
extern UCHAR api_dss_parameter ( UCHAR channel, DOWREQPAC *ReqItem, UCHAR *buffer, UINT bufferSize, UINT *outputSize );//DSS for download parameter

#endif
