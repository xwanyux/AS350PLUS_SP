#ifndef _PMUAPI_H_
#define _PMUAPI_H_
#include "POSAPI.h"

//battery power status
//		 0x00 - battery power is GOOD. 80%~99%
//	     0x01 - battery power is POOR. 0%~39%
//	     0x02 - battery power is FAIR. 40%~79%
//	     0x80 - DC power here and battery power is GOOD without charging.	
//	     0x82 - DC power here and battery power is UNKNOW without charging. - BATT may be not connected or bad
//	     0x81 - DC power here and battery power is POOR and in charging.	0%~39%
//	     0x83 - DC power here and battery power is FAIR and in charging.	40%~79%
//	     0x84 - DC power here and battery power is GOOD and in chargeing.	80%~99%
#define BATTGOOD		    0x00
#define BATTPOOR		    0x01
#define BATTFAIR		    0x02
#define DC_BATTCOMPLETE		0x80
#define DC_BATTUNKNOW		0x82
#define DC_BATTPOOR		    0x81
#define DC_BATTFAIR		    0x83
#define DC_BATTGOOD		    0x84


//function
UCHAR api_pmu_status( UCHAR *BattEnergyPercent );
void  api_pmu_setup( ULONG BackLitTime, ULONG PowerSavingTime, ULONG PowerOffTime );
void  api_pmu_poweroff( void );
void  api_pmu_poweron( void );
#endif