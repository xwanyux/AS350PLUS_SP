//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : MAX32550 (32-bit Platform)				    **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : API_TMR.C                                                  **
//**  MODULE   : api_tim2_open()				            **
//**		 api_tim2_close()					    **
//**		 api_tim2_gettick()					    **
//**									    **
//**  FUNCTION : API::TIM2 (1us Timer Module)				    **
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



UCHAR	os_DHN_TIM2 = 0;

static struct itimerspec new_value;
static struct timespec now;
static struct timeval timeout;


static uint64_t count;

static UCHAR TickCount = 0;





// ---------------------------------------------------------------------------
// FUNCTION: To enable the timer device.
// INPUT   : unit
//           Time tick resolution unit, incremented in step of this value.
//	     One unit is 1us, i.e., time tick is advanced by 1us if unit=1.
// OUTPUT  : none.
// RETURN  : DeviceHandleNo
//           apiOutOfService
// ---------------------------------------------------------------------------
UCHAR	api_tim2_open( ULONG microSec )
{
	if(os_DHN_TIM2 != 0)
		api_tim2_close(os_DHN_TIM2);

	int fd;

	TickCount = 0;

	long SEC = microSec / 1000000;
	long NSEC = (microSec % 1000000) * 1000;

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
	os_DHN_TIM2  = (UCHAR)fd;
	//printf("os dhn = %d\n",os_DHN_TIM2);
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
UCHAR	api_tim2_close( UCHAR dhn )
{
	/*
	 *check if timer opened before
	 */
	if(dhn != os_DHN_TIM2)
		return (apiFailed);

	new_value.it_interval.tv_sec = 0;		//timer execute interval time(sec)
	new_value.it_interval.tv_nsec = 0;		//timer execute interval=0 ==>timer not enable	
	if (timerfd_settime(dhn, TFD_TIMER_ABSTIME, &new_value, NULL) == -1) { //set timer attribute
        return( apiFailed );
    }
	if(close(dhn)==-1)
	  return( apiFailed );

	os_DHN_TIM2 = 0;
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
UCHAR	api_tim2_gettick( UCHAR dhn, ULONG *dbuf )
{
	
	// if(dhn != os_DHN_TIM2)
	// 	return (apiFailed);

	// if(read(dhn, &count, sizeof(uint64_t))>0){
	// 	if(count>1){
	// 		(*dbuf)+=count;
	// 	}
	// 	(*dbuf)++;		
	// 	return( apiOK );
	// }
	return( apiFailed );	
}

// ---------------------------------------------------------------------------
// FUNCTION: To check the status of timeout.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : none.
// RETURN  : apiReady	 - reaches timeout
//	     apiNotReady - not yet timeout
//           apiFailed   - invalid check
// ---------------------------------------------------------------------------
UCHAR	api_tim2_status( UCHAR dhn )
{
	uint64_t count;
	
	if(dhn != os_DHN_TIM2)
		return (apiFailed);

	if( read(dhn, &count, sizeof(uint64_t)) >0 ){
		TickCount += count;
	}

	if(TickCount >= 2)
		return (apiReady);
	
	return (apiNotReady);


}


/**
 * 	this function is used to implement delay in ms (by micro sec timer)
 *  @param[in] ms		     time delay in mini-sec
 *  @note not sure where to put this function (is implement for EMVK and ped)
 */  
void BSP_Delay_n_ms(ULONG ms){
   UCHAR dhn;
   ULONG micro_sec;
   micro_sec = ms * 1000; 
   dhn = api_tim2_open(micro_sec);
   if(dhn == apiOutOfService)
		return;
	
	while(api_tim2_status(dhn) != apiReady);

	return;
}





