//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							    **
//**  PRODUCT  : AS350 Plus						    **
//**                                                                        **
//**  FILE     : DEV_BUZ.C                                                  **
//**  MODULE   : OS_TaskBuzzer()					    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/11/30                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
//#include "bsp_buz.h"
//#include "bsp_tmr.h"
//#include "bsp_gpio.h"

//#include "OS_PROCS.h"
//#include "DEV_BUZ.h"

#include<stdint.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<linux/input.h>
#include<stdlib.h>
#include <string.h>
#include "POSAPI.h"
#include "OS_PROCS.h"
//UCHAR	Buzz_apiBuf[10][6];
UCHAR	Buzz_dhn=0;
UCHAR	Input_EventName[50];//get input event description,to check if open event is right event
const UCHAR	Buzz_EventName[]="pwm-beeper";

int Buzz_fd=0;

//extern	BSP_TIMER		*pBuzTmr;
extern	volatile	UINT8	os_DHN_BUZ_5A;
extern	volatile	UINT32	os_BUZ_5A_START;
extern	UINT32		os_BUZ_MODE;	// 0x00 = default PCI mode, 0x01 = user-defined mode

//BSP_GPIO	*pGpio12;		// io port for BUZZER (SDK)
//BSP_GPIO	*pGpio27;		// io port for BUZZER (TARGET)

UINT32		os_BUZ_IO_STATE = FALSE;
volatile	UINT32	os_flag_buz_io = 0;

#define	BUZ_DEFAULT_ON_CNT	5
static	UINT32	os_BUZ_ON_CNT = BUZ_DEFAULT_ON_CNT;


// ---------------------------------------------------------------------------
// FUNCTION: Initialize BUZZER device.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// NOTE    : Assume the trigger signal is GPIO0_12, it shall be changed for real target.
//           Real Target: GPIO0_27
// ---------------------------------------------------------------------------
UINT32	OS_BUZ_Init( void )
{
	if(Buzz_fd==0){
		Buzz_fd = open("/dev/input/event3", O_RDWR);
	}
	ioctl(Buzz_fd, EVIOCGNAME(sizeof(Input_EventName)), Input_EventName);
	//check whether opened event are beeper event.If not,close event3 and open event4
	if(strcmp(Buzz_EventName,Input_EventName)!=0)
	{
		// printf("!!!!!!!!!!%s\n",Input_EventName);
		close(Buzz_fd);
		Buzz_fd = open("/dev/input/event4", O_RDWR);
	}

	if(Buzz_fd < 0 )
          return( FALSE );
	else
	  {
	  //BSP_TMR_Stop( pBuzTmr );
	  return( TRUE );
	  }
}

// ---------------------------------------------------------------------------
void	OS_BUZ_ResetCnt( void )
{
	os_BUZ_ON_CNT = BUZ_DEFAULT_ON_CNT;
}

// ---------------------------------------------------------------------------
// FUNCTION: Turn ON/OFF buzzer FLAG.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// NOTE    : Assume the trigger signal is GPIO12, it shall be changed for real target.
// ---------------------------------------------------------------------------
void	OS_BUZ_TurnFlag( UINT32 flag )
{
	os_flag_buz_io = flag;
}

// ---------------------------------------------------------------------------
// FUNCTION: Turn ON buzzer.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// NOTE    : Assume the trigger signal is GPIO12, it shall be changed for real target.
// ---------------------------------------------------------------------------
void	OS_BUZ_TurnOn( void )
{
#if	0
UINT32 	FreqInHz = 2500;


	if( !DIAG_IsESX() )	// 2017-05-09
	  {
	  BSP_WR32( (pGpio27->Base + GPIO_OUT_SET), pGpio27->Mask );
	  return;
	  }
	  
	  
	pBuzTmr->Count      = 1;
	pBuzTmr->Compare    = BSP_GetClock( TRUE, (FreqInHz * 16) );
	pBuzTmr->PwmTrigger = pBuzTmr->Compare >> 1;
	pBuzTmr->Control    = TMR_PRES_DIV_16 | TMR_TEN | TMR_MODE_PWM;
	
	if( BSP_TMR_Start( pBuzTmr ) != BSP_SUCCESS )
	  {
	  BSP_TMR_Stop( pBuzTmr );
	  BSP_TMR_Start( pBuzTmr );
	  }
#else

int	ret=0;
UINT32 	FreqInHz = 2500;
struct	input_event event;


	  /*
	   * Use a 50% duty cycle
	   */
	  event.type = EV_SND;
	  event.code = SND_TONE;
	  
//	  FreqInHz = pPST->RFU_6 + pPST->RFU_7*256;
	  event.value = FreqInHz;
	  
	  ret = write(Buzz_fd, &event, sizeof(struct input_event));
	  
#endif
}

// ---------------------------------------------------------------------------
void	OS_BUZ_CloseRunningTask( OS_PST *pPST )
{
UINT8	i;
UINT8	pid;
OS_PST *pst;


	pid = psDEV_BUZ + 0x80;
	for( i=0; i<MAX_DID_NO; i++ )
	   {
	   pst = PS_SeekProcess( pid + i );
	   if( pst && (pst == pPST) )
	     {
	     OS_BUZ_TurnOff();
	     PS_SignalProcess( pid + i, SIG_EXIT, 0x0000, NULLPTR );
	     }
	   }
}

// ---------------------------------------------------------------------------
void	OS_BUZ_StopRunningTask( OS_PST *pPST )
{
UINT8	i;
UINT8	pid;
OS_PST *pst;


	pid = psDEV_BUZ + 0x80;
	for( i=0; i<MAX_DID_NO; i++ )
	   {
	   pst = PS_SeekProcess( pid + i );
	   if( pst && (pst != pPST) && (pst->State == PS_RUNNING) && (pst->RFU_0) )
	     {
	     pst->RFU_0 = 0;	// psBUZ_CYCLE
	     pst->RFU_8 = 0;	// psBUZ_START_FLAG
	     PS_RenewLRC( (UINT8 *)pst );
	     }
	   }
}

// ---------------------------------------------------------------------------
void	OS_BUZ_TurnOn_EX( OS_PST *pPST )
{
int	ret=0;
UINT32 	FreqInHz;
struct	input_event event;


	if( pPST->RFU_8 == 0 )	// psBUZ_START_FLAG = 0?
	  {
	  /*
	   * Use a 50% duty cycle
	   */
	  event.type = EV_SND;
	  event.code = SND_TONE;
	  
	  FreqInHz = pPST->RFU_6 + pPST->RFU_7*256;
	  event.value = FreqInHz;
	  
	  ret = write(Buzz_fd, &event, sizeof(struct input_event));

	  pPST->RFU_8 = 1;
	  }
	else
	  {
	  if( !os_BUZ_MODE )	// PCI mode
	    {
	    if( os_BUZ_ON_CNT )
	      {
	      os_BUZ_ON_CNT--;
//	      printf("\r\nos_BUZ_ON_CNT=%x\r\n", os_BUZ_ON_CNT);
	      if( os_BUZ_ON_CNT == 1 )
	      	{
	      	OS_BUZ_TurnOff_EX(pPST);
//	      	printf("\r\nOS_BUZ_TurnOff()\r\n");
		}
	      }
	    }
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: Turn OFF buzzer.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// NOTE    : Assume the trigger signal is GPIO12, it shall be changed for real target.
// ---------------------------------------------------------------------------
void	OS_BUZ_TurnOff( void )
{
#if	0
	if( !DIAG_IsESX() )	// 2017-05-09
	  BSP_WR32( (pGpio27->Base + GPIO_OUT_CLR), pGpio27->Mask );
	else
	  BSP_TMR_Stop( pBuzTmr );
#endif

int	ret=0;
UINT32 	FreqInHz;
struct	input_event event;


//	if( pPST->RFU_8 == 1 )	// psBUZ_START_FLAG = 1?
//	  {
//	  BSP_TMR_Stop( pBuzTmr );

	  event.type = EV_SND;
	  event.code = SND_TONE;
	  
	  FreqInHz = 0;
	  event.value = FreqInHz;
	  
	  ret = write(Buzz_fd, &event, sizeof(struct input_event));
	    
//	  pPST->RFU_8 = 0;
//	  }
}

// ---------------------------------------------------------------------------
void	BUZ_RandomDelay( void )
{
volatile UINT32	i = 0;
UINT8	cnt;


	api_sys_random_len( &cnt, 1 );
	
	for ( ; cnt > 0; cnt--)
		for (i = 0; i < 1600; i++);
}

// ---------------------------------------------------------------------------
void	OS_BUZ_TurnOff_EX( OS_PST *pPST )
{
int	ret=0;
UINT32 	FreqInHz;
struct	input_event event;


	if( pPST->RFU_8 == 1 )	// psBUZ_START_FLAG = 1?
	  {
//	  BSP_TMR_Stop( pBuzTmr );

	  BUZ_RandomDelay();	// 2024-04-10

	  event.type = EV_SND;
	  event.code = SND_TONE;
	  
	  FreqInHz = 0;
	  event.value = FreqInHz;
	  
	  ret = write(Buzz_fd, &event, sizeof(struct input_event));
	    
	  pPST->RFU_8 = 0;
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: Kernel task for device BUZZER.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_TaskBuzzer( OS_PST *pPST )
{	
	if( pPST->RFU_0 == 0 )	// psBUZ_CYCLE=0?
	  {
	  pPST->Signal = SIG_IGNORE;
	  pPST->Xtime = 0;		// put to sleep
	  PS_RenewLRC( (UINT8 *)pPST );
	  
	  pPST->RFU_0 = pPST->RFU_3;	// reset psBUZ_CYCLE
	  
	  pPST->RFU_1 = pPST->RFU_4;	// reload psBUZ_ON & psBUZ_OFF counter
	  pPST->RFU_2 = pPST->RFU_5;
	  
	  OS_BUZ_TurnOff_EX( pPST );	// turn off buzzer
	  }
	else // psBUZ_CYCLE != 0
	  {
	  if( pPST->RFU_1 != 0 )  // psBUZ_ON=0?
	    {
	    pPST->RFU_1 -= 1;	// psBUZ_ON-1
	    OS_BUZ_TurnOn_EX( pPST );	// turn on buzzer
	    }
	  else
	    {
	    if( pPST->RFU_2 != 0 ) // psBUZ_OFF=0?
	      {
	      pPST->RFU_2 -= 1;	// psBUZ_OFF-1
	      OS_BUZ_TurnOff_EX( pPST );	// turn off buzzer
	      }
	    else // psBUZ_ON & psBUZ_OFF = 0
	      {
	      pPST->RFU_0 -= 1;	// psBUZ_CYCLE-1
	      
	      if( pPST->RFU_0 == 0 ) // psBUZ_CYCLE=0?
	        {
	        if( pPST->RFU_9 == 0x5A )	// auto close?
	          {
	          OS_BUZ_CloseRunningTask( pPST );
	          
	          os_DHN_BUZ_5A = 0;	// 2020-03-03
	          os_BUZ_5A_START = FALSE;	// 2020-03-04
	          return;
	          }
	          
	        pPST->Signal = SIG_IGNORE;
	  	pPST->Xtime = 0;	   // put to sleep
	  	PS_RenewLRC( (UINT8 *)pPST );
	  
	  	pPST->RFU_0 = pPST->RFU_3; // reset psBUZ_CYCLE
		}
		
	      pPST->RFU_1 = pPST->RFU_4;   // reload psBUZ_ON & psBUZ_OFF counter
	      pPST->RFU_2 = pPST->RFU_5;
	      }
	    }
	  }

}
