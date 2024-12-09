//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : MAX32550 (32-bit Platform)				    **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : API_TMR.C                                                  **
//**  MODULE   : api_tim3_open()				            **
//**		 api_tim3_close()					    **
//**		 api_tim3_gettick()					    **
//**									    **
//**  FUNCTION : API::TIM3 (1us Timer Module)				    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/05/09                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"
#include "OS_PROCS.h"
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>        /* Definition of uint64_t */



UCHAR	os_DHN_TIM3 = 0;

static struct itimerspec new_value;
static struct timespec now;
static struct timeval timeout;

static uint64_t count;

static UCHAR FirstTimeFlag = 1;




// ---------------------------------------------------------------------------
// FUNCTION: To enable the timer device.
// INPUT   : unit
//           Time tick resolution unit, incremented in step of this value.
//	     One unit is 1us, i.e., time tick is advanced by 1us if unit=1.
// OUTPUT  : none.
// RETURN  : DeviceHandleNo
//           apiOutOfService
// ---------------------------------------------------------------------------
UCHAR	api_tim3_open( ULONG microSec )
{

	if(os_DHN_TIM3 != 0)
		api_tim3_close(os_DHN_TIM3);

	int fd;

	long SEC = microSec / 1000000;
	long NSEC = (microSec % 1000000) * 1000;

	// make the tick number correct
	FirstTimeFlag = 1;

	if (clock_gettime(CLOCK_REALTIME, &now) == -1) {
        return( apiOutOfService );
    }
	new_value.it_value.tv_sec = now.tv_sec;					//timer start time(sec)
    new_value.it_value.tv_nsec = now.tv_nsec;	//timer start time(nano sec).start counting after 1 unit
	new_value.it_interval.tv_sec = SEC;						//timer execute interval time(sec)
	new_value.it_interval.tv_nsec = NSEC;			//timer execute interval time. 1K nano sec = 1 um
	fd = timerfd_create(CLOCK_REALTIME, O_NONBLOCK);					//create timer object
	if (fd == -1) {
        return( apiOutOfService );
    }
    if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1) { //set timer attribute
        return( apiOutOfService );
    }
	os_DHN_TIM3  = (UCHAR)fd;
	return (UCHAR)fd;

	return( apiOutOfService );
}

// ---------------------------------------------------------------------------
// FUNCTION: To disable the system timer service.
// INPUT   : dhn
//           The specified device handle number.
//	     0x00 = to close all opened tasks.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_tim3_close( UCHAR dhn )
{
	UINT i=0;
	/*
	 *check if timer opened before
	 */
	if(dhn != os_DHN_TIM3)
		return (apiFailed);

	new_value.it_interval.tv_sec = 0;		//timer execute interval time(sec)
	new_value.it_interval.tv_nsec = 0;		//timer execute interval=0 ==>timer not enable	
	if (timerfd_settime(dhn, TFD_TIMER_ABSTIME, &new_value, NULL) == -1) { //set timer attribute
        return( apiFailed );
    }
	if(close(dhn)==-1)
	  return( apiFailed );

	os_DHN_TIM3 = 0;
	return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the contents of tick counter from the specified timer.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//	     ULONG  counter  // current 32-bit up-counter of the specified timer.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_tim3_gettick( UCHAR dhn, ULONG *dbuf )
{
	if(dhn != os_DHN_TIM3)
		return (apiFailed);

	if(read(dhn, &count, sizeof(uint64_t))>0){
		if(count>1){
			(*dbuf)+=count;
		}
		(*dbuf)++;

		// only need to deal with first time flag
		if(FirstTimeFlag){
			(*dbuf)--;
			FirstTimeFlag = 0; 
		}

		return( apiOK );
	}
	return( apiFailed );
}
