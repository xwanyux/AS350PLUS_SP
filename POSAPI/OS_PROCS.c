//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : OS_PROCS.C                                                 **
//**  MODULE   : PS_ClearAllProcess()                                       **
//**             PS_CheckLRC()						    **
//**             PS_RenewLRC()						    **
//**             PS_UpKillProcess()					    **
//**             PS_KillProcess()					    **
//**             PS_SeekProcess()					    **
//**             PS_ExecProcess()					    **
//**             PS_ForkProcess()					    **
//**             PS_SignalProcess()					    **
//**             PS_CloseAllPID()					    **
//**             PS_NormalCloseDevice()					    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2007/07/17                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2007 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "OS_PROCS.h"

OS_PST		ProcessStateTable[MAX_PST_NO];
volatile	UINT32	fPS_Exec;	// a signal to external caller that this function has been executed

// ---------------------------------------------------------------------------
// FUNCTION: To clear all processes in the ProcessStateTable[].
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void PS_ClearAllProcess( void )
{
UINT32	i;
	
	for( i=0; i<MAX_PST_NO; i++ )
	   ProcessStateTable[i].State = PS_END;
}

// ---------------------------------------------------------------------------
// FUNCTION: Validate the given process by using LRC method.
// INPUT   : pPST -- the current PST.
// OUTPUT  : none.
// RETURN  : TRUE  -- valid
//           FALSE -- invalid
// ---------------------------------------------------------------------------
BSP_BOOL PS_CheckLRC( UINT8 *pPST )
{
UINT32	i;
UINT8	lrc;

	lrc = 0;
	for( i=0; i<PST_CHKSUM_LEN; i++ )
	   lrc ^= *pPST++;
	
	if( lrc == *pPST )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Renew LRC for the given process.
// INPUT   : pPST -- the current PST.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void PS_RenewLRC( UINT8 *pPST )
{
UINT32	i;
UINT8	lrc;

	lrc = 0;
	for( i=0; i<PST_CHKSUM_LEN; i++ )
	   lrc ^= *pPST++;
	
	*pPST = lrc;
}

// ---------------------------------------------------------------------------
// FUNCTION: To kill the process upwardly with given process id.
// INPUT   : pPST  -- the current PST.
//           index -- item number.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void PS_UpKillProcess( UINT8 *pPST, UINT32 index )
{
UINT32	i;

	for( i=index; i>0; i-- )
	   {
	   pPST -= PST_LEN;	// ptr to the previous one
	   
	   if( *pPST == PS_KILLED )
	     *pPST = PS_END;
	   }
}

// ---------------------------------------------------------------------------
// FUNCTION: To kill the process with given process id.
// INPUT   : pid -- process id.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void PS_KillProcess( UINT8 pid )
{
UINT32	i;

	for( i=0; i<MAX_PST_NO; i++ )
	   {

	   if( ProcessStateTable[i].PID == pid )
	     {
	     ProcessStateTable[i].State = PS_KILLED;
	     ProcessStateTable[i].Signal = SIG_IGNORE;
	     
	     if( (i == MAX_PST_NO - 1) || (ProcessStateTable[i+1].State == PS_END) )	// last one or no task in the next PST
	       {
	       ProcessStateTable[i].State = PS_END;	// it's the last process
	       PS_UpKillProcess( (UINT8 *)&ProcessStateTable[i], i );	
	       }
	     break;  
	     }
	   }	   
}

// ---------------------------------------------------------------------------
// FUNCTION: To kill the process with given process id.
// INPUT   : pid  -- process id.
// OUTPUT  : none.
// RETURN  : pPST    -- pointer to the found PST entry.
//           NULLPTR -- not found.
// ---------------------------------------------------------------------------
OS_PST *PS_SeekProcess( UINT8 pid )
{
OS_PST	*pPST;
UINT32	i;
	
	for( i=0; i<MAX_PST_NO; i++ )
	   {
	   pPST = &ProcessStateTable[i]; // ptr to n'th entry
	   if( pPST->PID == pid )
	     {
	     if( (pPST->State == PS_RUNNING) || (pPST->State == PS_SLEEPING) )
	       return( pPST ); // found
	     }
	    }
	return( NULLPTR ); // not found
}

// ---------------------------------------------------------------------------
// FUNCTION: Run the processes in the ProcessStateTable. (called by OS timer task)
// INPUT   : pPST -- the current PST.
// OUTPUT  : none.
// RETURN  : TRUE  -- valid
//           FALSE -- invalid
// ---------------------------------------------------------------------------
void PS_ExecProcess( void )
{
UINT32	i;
OS_PST	*pPST;

	fPS_Exec = FALSE;	// reset flag
	
	for( i=0; i<MAX_PST_NO; i++ )
	   {
	   pPST = &ProcessStateTable[i];	// ptr to n'th entry
	   
	   if( PS_CheckLRC( (UINT8 *)&ProcessStateTable[i] ) == TRUE )
	     {
	     switch( pPST->State )
	           {
	           case PS_END:
	           	return;
	           	
	           case PS_SLEEPING:
	           
	           	if( pPST->Signal == SIG_INT )	// signal happens
	           	  {
	           	  pPST->Signal = SIG_IGNORE;	// clear signal
	           	  pPST->State = PS_RUNNING;	// wake up the process
	           	  
	           	  pPST->pIsrFunc( pPST );	// callback the target process
	           	  PS_RenewLRC( (UINT8 *)&ProcessStateTable[i] );	// renew LRC for the process
	           	  }
	           	else
	           	  {
	           	  if( pPST->Signal == SIG_EXIT )
	           	    {
	           	    PS_KillProcess( pPST->PID );       	    
	           	    }
	           	  }
	           	break;
	           	
	           case PS_RUNNING:
	           
	           	if( pPST->Xtime == 0xFFFF )	// always run?
	           	  {
	           	  pPST->pIsrFunc( pPST );	// callback the target process
	           	  break;
	           	  }

	           	if( pPST->Xtime == 0x0000 )	// quit process?
	           	  {
	           	  if( pPST->Signal == SIG_EXIT )
	           	    PS_KillProcess( pPST->PID );
	           	  else
	           	    {
	           	    pPST->State = PS_SLEEPING;	// put to sleeping
	           	    PS_RenewLRC( (UINT8 *)&ProcessStateTable[i] );	// renew LRC for the process
	           	    }
	           	  }
	           	  
	           	break;
	           	
	           }
	     } // if(LRC)
	   
	   } // for(MAX_PST_NO)
	
}

// ---------------------------------------------------------------------------
// FUNCTION: To create a new process. (called by device API)
// INPUT   : devid -- device id.
//           pPPST -- the PST parameters.
//		      the caller shall setup the related fields before calling.
// OUTPUT  : dhn   -- device handle number.
// RETURN  : TRUE  -- OK
//           FALSE -- failed
// ---------------------------------------------------------------------------
BSP_BOOL PS_ForkProcess( UINT8 devid, OS_PST *pPPST, UINT8 *dhn )
{
OS_PST	*pPST;
UINT8	n;
UINT8	did_old;
UINT8	did_new;
UINT8	n1_targetPST;
UINT8	n2_sameDID;
UINT8	tid;


	did_old = devid & 0x78;
	n1_targetPST = 0x0FF;
	n2_sameDID = 0;
	tid = 0;
	
	for( n=0; n<MAX_PST_NO; n++ )
	   {
	   pPST = &ProcessStateTable[n];	// ptr to n'th entry
	   
	   did_new = pPST->PID;
	   did_new &= 0x78;
	   
	   switch( pPST->State )
	   	 {
	   	 case PS_END:
	   	 case PS_KILLED:
	   	      
	   	      if( n1_targetPST == 0x0FF )
	   	      	n1_targetPST = n;
	   	      	
	   	      break;
	   	 
	   	 default: // PS_RUNNING or PS_SLEEPING
	   	      
	   	      if( did_new == did_old )
	   	        {
	   	        if( ((pPST->PID) & 0x07) == tid )
	   	          {
	   	          tid++;
	   	          tid &= 0x07;
	   	          }
	   	        n2_sameDID++;
	   		}
	   		
	   	 } // switch(state)
	   } // for(n)
	   
	// *** Create a new process ***
	
	if( n1_targetPST == 0x0FF )
	  return( FALSE );	// PST full
	
	if( n2_sameDID >= MAX_DID_NO )	// out of service
	  {
	  *dhn = 0xFF;	// out of service
	  return( FALSE );
	  }
	  
	pPST = &ProcessStateTable[n1_targetPST]; // ptr to N1'th entry
	
	*dhn = tid | 0x80 | did_old;		// output dhn
	pPST->PID = *dhn;			// new PID
	
	// *** Setup parameters to new PST entry
	
	pPST->Xtime = pPPST->Xtime;
	pPST->pIsrFunc = pPPST->pIsrFunc;
	pPST->Signal = pPPST->Signal;
	pPST->RFU_0 = pPPST->RFU_0;
	pPST->RFU_1 = pPPST->RFU_1;
	pPST->RFU_2 = pPPST->RFU_2;
	pPST->RFU_3 = pPPST->RFU_3;
	pPST->RFU_4 = pPPST->RFU_4;
	pPST->RFU_5 = pPPST->RFU_5;
	
	pPST->RFU_6 = pPPST->RFU_6;
	pPST->RFU_7 = pPPST->RFU_7;
	pPST->RFU_8 = pPPST->RFU_8;
	pPST->RFU_9 = pPPST->RFU_9;
	
	pPST->State = PS_RUNNING;		// process is registered
	
	PS_RenewLRC( (UINT8 *)&ProcessStateTable[n1_targetPST] );
	return( TRUE );
	
}

// ---------------------------------------------------------------------------
// FUNCTION: This function is used to send a signal to an existent process, 
//           usually to quit a process.
// INPUT   : pid    -- process id. (same as DHN in API)
//           signal -- a signal to the process.
//           delay  -- exit the process after DELAY time, 0x0000=quit immediately.
//           pIsr   -- callback function, set NULLPTR to it if not used. 
// OUTPUT  : none.
// RETURN  : TRUE  -- ok
//           FALSE -- failed
// ---------------------------------------------------------------------------
BSP_BOOL PS_SignalProcess( UINT8 pid, UINT8 signal, UINT16 delay, OS_PST *pIsr )
{
OS_PST *pPST;

	pPST = PS_SeekProcess( pid );
	if( pPST )
	  {
	  pPST->Signal = signal;	// put signal
	  pPST->Xtime = delay;		// put exec time
	  
	  if( signal == SIG_EXIT )	// quit process?
	    {
	    if( pIsr == NULLPTR )
	      {
	      pPST->RFU_0 = 0x00;
	      pPST->RFU_1 = 0x00;
	      }
	    else    
	      pPST->pIsrFunc( pIsr );	// callback function
	    }
	  
	  PS_RenewLRC( (UINT8 *)pPST );
	  
	  return( TRUE );
	  }
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To kill all the processes related to the given process id.
// INPUT   : did  -- device id.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void PS_CloseAllPID( UINT8 pid )
{
UINT8	did;	// device id
	
	pid |= 0x80;	// dhn
	for( did=0; did<MAX_DID_NO; did++ )
	   PS_SignalProcess( pid | did, SIG_EXIT, 0x0000, NULLPTR );

}

// ---------------------------------------------------------------------------
// FUNCTION: To close task(s) with the given PID.
// INPUT   : dhn -- device handle number.
//           pid -- process id.
// OUTPUT  : none.
// RETURN  : TRUE  -- OK
//           FALSE -- failed
// ---------------------------------------------------------------------------
BSP_BOOL PS_NormalCloseDevice( UINT8 dhn, UINT8 pid )
{

	// wait till the process has been killed
//	fPS_Exec = TRUE;	// PATCH: 2010-01-15, moved to "api_tim_close()" to avoid optimizing problem for the volatile variable.
	
	if( dhn == 0x00 )
	  PS_CloseAllPID( pid );
	else
	  {
	  if( PS_SignalProcess( dhn, SIG_EXIT, 0x0000, NULLPTR ) == FALSE )
	    return( FALSE );
	  }
	  
//	// wait till the process has been killed
//	fPS_Exec = TRUE;
				// PATCH: 2010-01-15, restore checking flag, otherwise some timer may be not executed.
	while( fPS_Exec );	// Unkonwn: this instruction will cause CPU loop here forever! (because of compiler's optimization)
//	BSP_Delay_n_ms( 12 );

//	do{			// PATCH: 2008/08/28, alternative solution
//	  BSP_Delay_n_ms( 1 );
//	  } while( fPS_Exec );
	
	return( TRUE );
}
