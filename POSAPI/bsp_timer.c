#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>        /* Definition of uint64_t */
#include <pthread.h>
#include "bsp_types.h"
#include "bsp_tmr.h"
#include "DEV_AUX.h"
#include "printer.h"//to know if printer running
#include "OS_PROCS.h"
#include <syslog.h>
pthread_t 	OS_Timer_Thread=0;//OS pthread flag
pthread_t 	MDM_Timer_Thread;//MDM pthread flag
BSP_TIMER	Timer_Task[BSP_MAX_TIMERS];
UINT8		Task_Number=0;
UINT8		OStimerFlag=0;
struct itimerspec new_value;
struct itimerspec curr_value;
struct timespec now;
int OS_fd=0;
int MDM_fd=0;

// ADD by Wayne 2020/08/12
// volatile	UINT32	os_SysTimerFreeCnt=0;
UINT32	os_SysTimerFreeCnt=0;
UINT32	os_SysTimer_TscDownCnt=0;
// ADD by Wayne 2020/09/04
extern ULONG global_counter_pmu_poweroff;
extern ULONG global_counter_pmu_backlite_off;
extern ULONG global_PowerOffTime;
extern void PowerSavingAndPowerOff(ULONG PowerOffTime);
extern UCHAR api_sys_backlight( UCHAR device, ULONG duration );
extern UCHAR global_powerSaving_on;
extern UCHAR global_Backlight_off_LCD;

extern void	OS_PCD_LED_Task();
extern void	OS_PED_Task( void );
extern void	OS_LAN_ConnectionTimeTask( void );

extern UCHAR PRN_status;//to know if printer running
#ifdef _build_SP_
extern UCHAR *shm_timer;
#endif

extern void TAMPER_Handler();   //Added by Tammy

void OS_Timer1Int( uint64_t count )
{
	//OS_KBD_Scan();			// normal keypad scan
//	OS_KBD_SecureScan();		// secure keypad scan
	
	// OS_AUX_FlowControl();		// AUX flow control
	
	OS_PED_Task();				//from OS_PED2.c. Timer for PED service 
	//OS_KBD_BL_Task();		// KBD backlight

	//OS_PMU_SwitchScan();		// scan AGM power switch
	
	//OS_PMU_TimeTick_Task();		//
	
	//OS_LCD_BL_Task();
	
	//OS_DHCP_Lease_Time_Task();	// LAN.DHCP
	
	OS_LAN_ConnectionTimeTask();	// API_LAN.c
	
	OS_PCD_LED_Task();		// 2022-12-08, JAMES
	PS_ExecProcess();		//
	
	// ADD by Wayne 2020/08/12
	// os_SysTimerFreeCnt++;
	os_SysTimerFreeCnt+=count;//20210830 modified bt west
	os_SysTimer_TscDownCnt+=count;

	//ADD by Wayne 2020/09/04
	// if(global_counter_pmu_poweroff > 0)
	// 	global_counter_pmu_poweroff--;
	//20210830 modified bt west
	if(global_counter_pmu_poweroff > 0)
	{
		if(count>global_counter_pmu_poweroff)
			global_counter_pmu_poweroff=0;
		else
			global_counter_pmu_poweroff-=count;
	}
	// ADD by Wayne 2020/10/29
	// if(global_counter_pmu_backlite_off > 0)
	// 	global_counter_pmu_backlite_off--;
	//20210830 modified bt west
	if(global_counter_pmu_backlite_off > 0)
	{
		if(count>global_counter_pmu_backlite_off)
			global_counter_pmu_backlite_off=0;
		else
			global_counter_pmu_backlite_off-=count;
	}
		
		
	
	//ADD by Wayne 2020/09/04
	if(global_counter_pmu_poweroff == 0 && global_powerSaving_on == TRUE){
		global_powerSaving_on = FALSE;
		PowerSavingAndPowerOff(global_PowerOffTime);
		
	}
	
	if(global_counter_pmu_backlite_off == 0 && global_Backlight_off_LCD == TRUE){
		global_Backlight_off_LCD = FALSE;
		api_sys_backlight(0, 0x00000000);
	}
	// #ifdef _build_SP_
	// memmove(shm_timer,os_SysTimerFreeCnt,4);
	// #endif
}

void *Timer1polling()
{
uint64_t count=0;
	while(1){
		if(OStimerFlag==0)
			pthread_exit(NULL);//finish thread
		// if((PRN_status==MOT_STOP)||(PRN_status==MOT_TURNOFF))
		if(read(OS_fd, &count, sizeof(uint64_t))>0)
		// if(BSP_TMR_GetTick(BSP_TIMER* pTimer,&count)>0)
		{
			OS_Timer1Int(count);
			// OS_PCD_LED_Task();
		}
		else
			usleep(100);
	};
		
}

#if	0
void *PCDtimer(){
	while(1){
		
	uint64_t count;
		if(read(OS_fd, &count, sizeof(uint64_t))>0){
			OS_PCD_LED_Task();
		}
	};
		
}
#endif

/*
**setup and start timerfd
**
**INPUT:
**		Mic_Sec:10ms unit delay time
**RETURN:
**		BSP_FAILURE
**		OS_fd:file description
 */
BSP_TIMER* BSP_TMR_Acquire(UINT32 Mic_Sec)
{
int fd;
UINT32 SEC=0;
UINT32 NSEC=Mic_Sec*10000000;//unit*10ms
	//to prevent from overflow
	if(NSEC>1000000000)
	{
		SEC=NSEC/1000000000;
		NSEC=NSEC%1000000000;
	}
	fd = timerfd_create(CLOCK_MONOTONIC, O_NONBLOCK);					//create timer object
	if(fd<0){
#ifdef	_OUTPUT_DEBUG_
		printf("BSP_TMR_Acquire fd fail\n");
#endif
		return NULL;
	}
	clock_gettime(CLOCK_MONOTONIC, &now);
	new_value.it_value.tv_sec = now.tv_sec;					//timer start time(sec)
    new_value.it_value.tv_nsec = now.tv_nsec;	//timer start time(nano sec).start counting after 1 unit
	new_value.it_interval.tv_sec = SEC;						//timer execute interval time(sec)
	new_value.it_interval.tv_nsec = NSEC;			//timer execute interval time. 10M nano sec = 10 millisec
	
	if(timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL)<0){
#ifdef	_OUTPUT_DEBUG_
		printf("BSP_TMR_Acquire set timer fail\n");
#endif
		return NULL;
	}
	//if this timer is running,allocate next timer to user
	for(UINT8 i=0;i<BSP_MAX_TIMERS;i++){
		if(Timer_Task[i].THR_status!=THREAD_RUN){
			Task_Number=i;
			break;
		}
		if(i==BSP_MAX_TIMERS-1)//if all timer in used
			return NULL;
	}
	memset(&Timer_Task[Task_Number],0,sizeof(BSP_TIMER));
	Timer_Task[Task_Number].Timer_FD=fd;
	Timer_Task[Task_Number].Task_Num=Task_Number;
#ifdef	_OUTPUT_DEBUG_
	printf("↓↓↓↓↓↓↓↓↓↓↓↓↓↓read task Task_Number=%d\tfd=%d\n",Task_Number,Timer_Task[Task_Number].Timer_FD);
#endif
	return(&Timer_Task[Task_Number]);
}
/*
**setup and start timerfd
**
**INPUT:
**		Mic_Sec:1ms unit delay time
**RETURN:
**		BSP_FAILURE
**		OS_fd:file description
 */
BSP_TIMER* BSP_TMR_Acquire2(UINT32 Mic_Sec)
{
int fd;
UINT32 SEC=0;
UINT32 NSEC=Mic_Sec*1000000;//unit*1ms
	//to prevent from overflow
	if(NSEC>1000000000)
	{
		SEC=NSEC/1000000000;
		NSEC=NSEC%1000000000;
	}
	
	fd = timerfd_create(CLOCK_MONOTONIC, O_NONBLOCK);					//create timer object
	if(fd<0){
#ifdef	_OUTPUT_DEBUG_
		printf("fd fail\n");
#endif
		return NULL;
	}
	clock_gettime(CLOCK_MONOTONIC, &now);
	new_value.it_value.tv_sec = now.tv_sec;					//timer start time(sec)
    new_value.it_value.tv_nsec = now.tv_nsec;	//timer start time(nano sec).start counting after 1 unit
	new_value.it_interval.tv_sec = SEC;						//timer execute interval time(sec)
	new_value.it_interval.tv_nsec = NSEC;			//timer execute interval time
	
	if(timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL)<0){
#ifdef	_OUTPUT_DEBUG_
		printf("set timer fail\n");
#endif
		return NULL;
	}
	//if this timer is running,allocate next timer to user
	for(UINT8 i=0;i<BSP_MAX_TIMERS;i++){
		if(Timer_Task[i].THR_status!=THREAD_RUN){
			Task_Number=i;
			break;
		}
		if(i==BSP_MAX_TIMERS-1)//if all timer in used
			return NULL;
	}
	memset(&Timer_Task[Task_Number],0,sizeof(BSP_TIMER));
	Timer_Task[Task_Number].Timer_FD=fd;
	Timer_Task[Task_Number].Task_Num=Task_Number;
#ifdef	_OUTPUT_DEBUG_
	printf("^^^^^^^^^^^^^read task Task_Number=%d\tfd=%d\n",Task_Number,Timer_Task[Task_Number].Timer_FD);
#endif
	return(&Timer_Task[Task_Number]);
}
BSP_TIMER* BSP_TMR_Acquire3(UINT32 Mic_Sec)
{
int fd;
UINT32 SEC=0;
UINT32 NSEC=Mic_Sec*10000;//unit*10us
	//to prevent from overflow
	if(NSEC>1000000000)
	{
		SEC=NSEC/1000000000;
		NSEC=NSEC%1000000000;
	}
	
	fd = timerfd_create(CLOCK_MONOTONIC, O_NONBLOCK);					//create timer object
	if(fd<0){
#ifdef	_OUTPUT_DEBUG_
		printf("fd fail\n");
#endif
		return NULL;
	}
	clock_gettime(CLOCK_MONOTONIC, &now);
	new_value.it_value.tv_sec = now.tv_sec;					//timer start time(sec)
    new_value.it_value.tv_nsec = now.tv_nsec;	//timer start time(nano sec).start counting after 1 unit
	new_value.it_interval.tv_sec = SEC;						//timer execute interval time(sec)
	new_value.it_interval.tv_nsec = NSEC;			//timer execute interval time
	
	if(timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL)<0){
#ifdef	_OUTPUT_DEBUG_
		printf("set timer fail\n");
#endif
		return NULL;
	}
	//if this timer is running,allocate next timer to user
	for(UINT8 i=0;i<BSP_MAX_TIMERS;i++){
		if(Timer_Task[i].THR_status!=THREAD_RUN){
			Task_Number=i;
			break;
		}
		if(i==BSP_MAX_TIMERS-1)//if all timer in used
			return NULL;
	}
	memset(&Timer_Task[Task_Number],0,sizeof(BSP_TIMER));
	Timer_Task[Task_Number].Timer_FD=fd;
	Timer_Task[Task_Number].Task_Num=Task_Number;
	return(&Timer_Task[Task_Number]);
}
void getnowtime()
{
	long            us; // Microconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    us = spec.tv_nsec/1000; // Convert nanoseconds to milliseconds
    if (us > 999999) {
        s++;
        us = 0;
    }
#ifdef	_OUTPUT_DEBUG_
    printf("Current time: %ld.%06ld seconds since the Epoch\n",
           s, us);
#endif
}
/*
**Get time out times of specific timer 
**
**INPUT:
**		dhn:timerfd file description
**		counter:time out times
**OUTPUT:
		counter:time out times
**RETURN:
**		BSP_FAILURE
**		UINT ReadByte:>0 if time out
 */
UINT16 BSP_TMR_GetTick(BSP_TIMER* pTimer,uint64_t *counter){
int ReadByte;
int s;
uint64_t COUNT=0;
	if(pTimer)
	{
		if(pTimer->THR_status==THREAD_EXIT){
			pthread_exit(NULL);//finish thread
		}
		// timerfd_gettime(pTimer->Timer_FD,&curr_value);
		// clock_gettime(CLOCK_MONOTONIC, &now);
		// ReadByte=read(pTimer->Timer_FD, &counter, sizeof(uint64_t));
		//getnowtime();
		// s=read(pTimer->Timer_FD, &COUNT, sizeof(uint64_t));
		s=read(pTimer->Timer_FD, counter, sizeof(uint64_t));//20210830 modified by west to pass value out
		if(s>0)
			return(1);
	}
	
	return(0);
}
/*
**Create pthread to run input function poiter 
**
**INPUT:
**		void* Event:function pointer which you want to run on new thread
**OUTPUT:
**		
**RETURN:
**		BSP_FAILURE
**		UINT32	TMR_Thread:Thread flag 
 */
BSP_STATUS  
BSP_TMR_Start
( 
	void*			Event,
	BSP_TIMER 		*pTimer
	
)
{
	if(pthread_create(&pTimer->THR_flag, NULL,Event,NULL)!=0)
		return(BSP_FAILURE);
	
	pthread_detach(pTimer->THR_flag);
	pTimer->Event=Event;
	pTimer->THR_status=THREAD_RUN;
	return(BSP_SUCCESS);
}

BSP_STATUS  BSP_TMR_Stop( BSP_TIMER * pTimer )
{	
UINT16 		dhn;
BSP_BOOL 	flag;
UINT8		TskNum;
	if(pTimer==NULLPTR)
		return( BSP_FAILURE );
	
	TskNum=pTimer->Task_Num;
	dhn=pTimer->Timer_FD;
	flag=pTimer->THR_flag;
	pTimer->THR_flag=THREAD_EXIT;
	pthread_join(flag, NULL);//wait pthread exit
#ifdef	_OUTPUT_DEBUG_
	printf("^^^^^^^^^^^^^BSP_TMR_Stop Task_Number=%d\tfd=%d\n",TskNum,dhn);
#endif
	if(close(dhn)==-1)//close timerfd
	  return( BSP_FAILURE );
	
	//initialize timer
	memset(&Timer_Task[TskNum],0,sizeof(BSP_TIMER));
	return( BSP_SUCCESS );
}
BSP_BOOL OS_EnableTimer1( void )
{
	if((OStimerFlag!=0)&&(OS_fd!=0)&&(OS_Timer_Thread!=0))//if OS timer is already set
		return(BSP_SUCCESS);
int res;
	//timer set
	clock_gettime(CLOCK_MONOTONIC, &now);
	new_value.it_value.tv_sec = now.tv_sec+1;					//timer start time(sec)
    new_value.it_value.tv_nsec = now.tv_nsec;	//timer start time(nano sec).start counting after 1 unit
	new_value.it_interval.tv_sec = 0;						//timer execute interval time(sec)
	new_value.it_interval.tv_nsec = 10000000;			//timer execute interval time. 10M nano sec = 10 millisec
	if(OS_fd==0)
	{
		OS_fd = timerfd_create(CLOCK_MONOTONIC, O_NONBLOCK);					//create timer object
		if(OS_fd<0){
#ifdef	_OUTPUT_DEBUG_
			printf("fd fail\n");
#endif
			return 0;
		}
		if(timerfd_settime(OS_fd, TFD_TIMER_ABSTIME, &new_value, NULL)<0){
#ifdef	_OUTPUT_DEBUG_
			printf("set timer fail\n");
#endif
			return 0;
		}
	}
	
	PS_ClearAllProcess();	// JAMES, reset system process
	
	OStimerFlag=1;
	res=pthread_create(&OS_Timer_Thread, NULL, Timer1polling,NULL);
	if(res!=0)
	{
		openlog("slog", LOG_PID|LOG_CONS, LOG_USER);
 		syslog(LOG_EMERG, "pthread_create fail res=%d\n",res);
 		closelog();
	}
	return(BSP_SUCCESS);
}

BSP_BOOL OS_EnableTimer2( void )
{
	//timer set
	clock_gettime(CLOCK_MONOTONIC, &now);
	new_value.it_value.tv_sec = now.tv_sec+1;					//timer start time(sec)
    new_value.it_value.tv_nsec = now.tv_nsec;	//timer start time(nano sec).start counting after 1 unit
	new_value.it_interval.tv_sec = 0;						//timer execute interval time(sec)
	new_value.it_interval.tv_nsec = 10000000;			//timer execute interval time. 10M nano sec = 10 millisec

	MDM_fd = timerfd_create(CLOCK_MONOTONIC, O_NONBLOCK);					//create timer object
	if(OS_fd<0){
#ifdef	_OUTPUT_DEBUG_
		printf("fd fail\n");
#endif
		return 0;
	}
	if(timerfd_settime(MDM_fd, TFD_TIMER_ABSTIME, &new_value, NULL)<0){
#ifdef	_OUTPUT_DEBUG_
		printf("set timer fail\n");
#endif
		return 0;
	}
	pthread_create(&MDM_Timer_Thread, NULL, Timer1polling,NULL);
	return(BSP_SUCCESS);
}