#ifndef DOWN_H_
#define DOWN_H_

#include "POSAPI.h"



UCHAR api_down_open( UCHAR);
UCHAR api_down_txstring( UCHAR, UCHAR *, UINT );
UCHAR api_down_rxready( UCHAR, ULONG * );
UCHAR api_down_rxstring( UCHAR, UCHAR * , ULONG *);
UCHAR api_down_close( UCHAR, UCHAR );

#endif /*DOWN_H_*/
