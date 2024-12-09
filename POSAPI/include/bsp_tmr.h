#include "bsp_types.h"
#include <stdint.h>        /* Definition of uint64_t */
#define BSP_MAX_TIMERS		9
#define THREAD_EXIT			0
#define THREAD_RUN			1
typedef struct BSP_TIMER_S
{
	UINT16		Timer_FD;//timerfd file description
	BSP_BOOL	THR_flag;//thread id
	UINT8		THR_status;//to control thread exit
	UINT8		Task_Num;//which number of timers
	BSP_HANDLE	Event;//function pointer taht want to be handled 
}BSP_TIMER;
extern BSP_TIMER*	BSP_TMR_Acquire(UINT32 Mic_Sec);
extern BSP_TIMER*	BSP_TMR_Acquire2(UINT32 Mic_Sec);
extern BSP_TIMER*	BSP_TMR_Acquire3(UINT32 Mic_Sec);
extern UINT16		BSP_TMR_GetTick(BSP_TIMER* pTimer,uint64_t *counter);
extern BSP_STATUS	BSP_TMR_Start( void* Event,BSP_TIMER *pTimer );
extern BSP_STATUS	BSP_TMR_Stop( BSP_TIMER * pTimer );