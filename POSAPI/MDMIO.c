//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : CONEXANT V.92 hardware MODEM (56K).                        **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : ESF.C	                                            	    **
//**  MODULE   : cnxt_modem_read()				            **
//**		 cnxt_modem_write()					    **
//**		 cnxt_modem_ioctl()					    **
//**									    **
//**  FUNCTION : H/W MODEM Module			    		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2010/06/07                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2010 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "POSAPI.h"
#include "esf.h"
#include "APK_MDM.h"
#include "bsp_tmr.h"


// ---------------------------------------------------------------------------
extern	UINT		os_DHN_MDM56K; 		// modem UART port
extern	volatile	ULONG	MDM_FlowControlState;
extern	ULONG		os_mdm56k_flow_ctrl;

UCHAR	os_mdm56k_temp_buffer[2048+2];
UCHAR	os_mdm56k_atc_buffer[128];
UINT	os_mdm56k_atc_length;


// ---------------------------------------------------------------------------
// FUNCTION: Read byte(s) from the modem port.
// INPUT   : buf    - pointer to buffer holding the data read from the modem.
//           pcount - pointer to the number of bytes of "buf".
// OUTPUT  : pcount - pointer to the actual number of bytes read from the modem.
// RETURN  : MODEM_OK
//	     MODEM_ERROR
// ---------------------------------------------------------------------------
MODEM_STATUS	cnxt_modem_read( char *buf, unsigned int *pcount )
{
UCHAR	result;
UINT	iLen;

	
	result = api_aux_rxstring( os_DHN_MDM56K, os_mdm56k_temp_buffer );
	if( result == apiOK )
	  result = MODEM_OK;
	else
	  result = MODEM_ERROR;
	  
	iLen = os_mdm56k_temp_buffer[0] + os_mdm56k_temp_buffer[1]*256;
	
	memmove( buf, &os_mdm56k_temp_buffer[2], iLen );
	*pcount = iLen;
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Perform modem control functions.
// INPUT   : code   - ioctl function call for the modem.
//           pctl   - 
// OUTPUT  : pctl   - 
// RETURN  : MODEM_OK
//	     MODEM_ERROR
// ---------------------------------------------------------------------------
MODEM_STATUS	cnxt_modem_ioctl2( const MODEM_IOCTL_CODE code, int *pctl )
{
	
}

// ---------------------------------------------------------------------------
// FUNCTION: Perform modem control functions.
// INPUT   : code   - ioctl function call for the modem.
//           pctl   - 
// OUTPUT  : pctl   - 
// RETURN  : MODEM_OK
//	     MODEM_ERROR
// ---------------------------------------------------------------------------
MODEM_STATUS	cnxt_modem_ioctl( const MODEM_IOCTL_CODE code, int *pctl )
{
UCHAR	result;

	
	result = api_aux_rxready( os_DHN_MDM56K, (UCHAR *)pctl );
	// printf("pctl=%d\n",*pctl);
	if( result == apiReady ){
	  return( MODEM_OK );
	}
	if( result == apiNotReady )
	  return( MODEM_OTHERS );
	else
	  return( MODEM_ERROR );
	  
//	if( result == apiFailed )
//	  result = MODEM_ERROR;
//	else
//	  result = MODEM_OK;
//	  
//	return( result );
}

// ---------------------------------------------------------------------------
void	MDM_WaitString_V80( UINT len, UCHAR *str )
{
UCHAR	buf[300];

	
	do{
	  NosSleep(1);
	  
	  buf[0] = 0xFF;
	  buf[1] = 0x80;
	  buf[2] = len & 0x00FF;
	  buf[3] = (len & 0xFF00) >> 8;
	  memmove( &buf[4], str, len );
	  } while( api_aux_rxready(  os_DHN_MDM56K, buf ) != apiReady );
	  
	api_aux_rxstring( os_DHN_MDM56K, buf );
}

// ---------------------------------------------------------------------------
void	MDM_SetRTS_V80(void)
{
UCHAR	buf[128];


	if( !os_mdm56k_flow_ctrl )
	  return;

	if( MDM_FlowControlState == MDM_STATE_INIT )
	  return;
	
	while( api_aux_txready( os_DHN_MDM56K ) == apiNotReady );
	
//	buf[0] = 0;
//	buf[1] = 0;
//	while( api_aux_rxready( os_DHN_MDM56K, buf ) != apiReady );	// wait for DCD OFF (19 66)
//	api_aux_rxstring( os_DHN_MDM56K, buf );

//	BSP_Delay_n_ms(20);
//	NosSleep(2);
//	buf[0] = 0x19;
//	buf[1] = 0x66;
//	MDM_WaitString_V80( 2, buf );
	
	os_mdm56k_temp_buffer[0] = 2;
	os_mdm56k_temp_buffer[1] = 0;
	os_mdm56k_temp_buffer[2] = 0x19;
	os_mdm56k_temp_buffer[3] = 0x43;
	api_aux_txstring( os_DHN_MDM56K, os_mdm56k_temp_buffer );
	
//	buf[0] = 2;
//	buf[1] = 0;
//	while( api_aux_rxready( os_DHN_MDM56K, buf ) != apiReady );	// wait for CTS ON (19 63)
//	api_aux_rxstring( os_DHN_MDM56K, buf );
	
//	BSP_Delay_n_ms(20);
//	NosSleep(10);

	buf[0] = 0x19;
	buf[1] = 0x63;
	MDM_WaitString_V80( 2, buf );
	
	NosSleep(10);	// mandatory delay for IEN switching
}

// ---------------------------------------------------------------------------
void	MDM_ClearRTS_V80(void)
{
UCHAR	buf[128];


	if( !os_mdm56k_flow_ctrl )
	  return;
	  
	if( MDM_FlowControlState == MDM_STATE_INIT )
	  return;
	
	while( api_aux_txready( os_DHN_MDM56K ) == apiNotReady );	// wait for end of previous transmission
	
	os_mdm56k_temp_buffer[0] = 2;
	os_mdm56k_temp_buffer[1] = 0;
	os_mdm56k_temp_buffer[2] = 0x19;
	os_mdm56k_temp_buffer[3] = 0x42;
	api_aux_txstring( os_DHN_MDM56K, os_mdm56k_temp_buffer );
	
//	buf[0] = 2;
//	buf[1] = 0;
//	while( api_aux_rxready( os_DHN_MDM56K, buf ) != apiReady );	// wait for CTS OFF (19 62)
//	api_aux_rxstring( os_DHN_MDM56K, buf );
}

// ---------------------------------------------------------------------------
//void	MDM_ClearRTS_V80_CONNECT( void )
//{
//UCHAR	buf[4];
//
//
//	buf[0] = 0x19;
//	buf[1] = 0xBE;
//	buf[2] = 0x24;
//	buf[3] = 0x24;
//	MDM_WaitString_V80( 4, buf );	// wait for "19 BE 24 24"
//	
//	MDM_ClearRTS_V80();		// RTS OFF
//}

// ---------------------------------------------------------------------------
// FUNCTION: Write byte(s) to the modem port.
// INPUT   : buf    - pointer to buffer holding the data to be written to the modem.
//           pcount - pointer to the number of bytes of "buf".
// OUTPUT  : pcount - pointer to the actual number of bytes written to the modem.
// RETURN  : MODEM_OK
//	     MODEM_ERROR
// ---------------------------------------------------------------------------
MODEM_STATUS	cnxt_modem_write( const char *buf, unsigned int *pcount )
{
UCHAR	dhn_tim;
UCHAR	result;
UINT	tick1, tick2;
UINT	iLen;

	
//	MDM_SetRTS();
	MDM_SetRTS_V80();
	
//	result = apiFailed;
//	while(1)
//	  {	
//	  if( api_aux_txready( os_DHN_MDM56K ) == apiReady )
//	    {
	    iLen = *pcount;
	    os_mdm56k_temp_buffer[0] = iLen & 0x00FF;
	    os_mdm56k_temp_buffer[1] = (iLen & 0xFF00) >> 8;
	    memmove( &os_mdm56k_temp_buffer[2], buf, iLen );
		
	    result = api_aux_txstring( os_DHN_MDM56K, os_mdm56k_temp_buffer );
//	    break;
//	    }
//	  }
	
	
//	MDM_ClearRTS();
	MDM_ClearRTS_V80();
	
	if( result == apiOK )
	  return( MODEM_OK);
	else
	  {
	  *pcount = 0;
	  return( MODEM_ERROR );
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: Write AT commands to the modem port.
// INPUT   : buf    - pointer to buffer holding the data to be written to the modem.
//           pcount - pointer to the number of bytes of "buf".
//	     delay  - delay time in 10 ms after sending one AT command end with <cr>.
// OUTPUT  : pcount - pointer to the actual number of bytes written to the modem.
// RETURN  : MODEM_OK
//	     MODEM_ERROR
// ---------------------------------------------------------------------------
MODEM_STATUS	cnxt_modem_write_atc( const char *buf, unsigned int *pcount, ULONG delay )
{
UCHAR	buffer[128];
UCHAR	temp[128];
int	Avail;
unsigned int i, j;
unsigned int iLen;
unsigned int Count;
unsigned int bakCount = 0;
unsigned int tmpCount;

UCHAR	dhn_tim;
UCHAR	result;
UINT	tick1=0, tick2=0;
UINT	iDelay;


	iLen = *pcount;
	iDelay = delay;
	
	j = 0;
	for( i=0; i<iLen; i++ )
	   {
	   buffer[j++] = buf[i];
	   if( buf[i] == 0x0D )
	     {
	     if( delay != 0xFFFFFFFF )
	       Count = j;
	     else
	       Count = j-1;	// for case "+++"
	       
	     memmove( os_mdm56k_atc_buffer, buffer, Count );	// backup current AT command & length
	     os_mdm56k_atc_length = Count;			//
	     // send AT command
//	     if( cnxt_modem_write( buffer, &Count ) != MODEM_OK )
//	       return( MODEM_ERROR );

	     os_mdm56k_temp_buffer[0] = Count & 0x00FF;
	     os_mdm56k_temp_buffer[1] = (Count & 0xFF00) >> 8;
	     memmove( &os_mdm56k_temp_buffer[2], buffer, Count );
	     if( api_aux_txstring( os_DHN_MDM56K, os_mdm56k_temp_buffer ) != apiOK )
		 {
			 printf("aux tx fail\n");
	       return( MODEM_ERROR );
	     }
	     // Method 1: io delay
//	     if( delay )
//	       NosSleep(delay);
//	     else
//	       NosSleep(1);

	     // special check
	     if( (delay == 0x00000000) || (delay == 0xFFFFFFFF) )
	       iDelay = 300;

	     // Method 2: waiting for the response "command<cr><lf>OK<cr><lf>" from MODEM
	     dhn_tim = api_tim_open(1); // 1 tick unit = 10 ms
      	     api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
      
	     while(1)
		  {
			  
		  api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
		  if( (tick2 - tick1) >= iDelay )
		    {
				printf(">= iDelay\n");
		    api_tim_close( dhn_tim );
		    return( MODEM_ERROR );	// time out
		    }
		  
		  if( delay == 0 )
		    Avail = os_mdm56k_atc_length;	// expected response length without <cr><lf>OK<cr><lf>, dialup command "ATDxxx<cr>"
		  else
		    {
		    if( delay == 0xFFFFFFFF )
		      {
		      os_mdm56k_atc_length = 0;
		      Avail = 6;			// only <cr><lf>OK<cr><lf>
		      }
		    else
		      Avail = os_mdm56k_atc_length + 6;	// expected response length with <cr><lf>OK<cr><lf>
		    }
		  
//		  if( delay != 0 )
//		    Avail = os_mdm56k_atc_length + 6;	// expected response length with <cr><lf>OK<cr><lf>
//		  else
//		    Avail = os_mdm56k_atc_length;	// expected response length without <cr><lf>OK<cr><lf>, dialup command "ATDxxx<cr>"
	
	     	  //if( cnxt_modem_ioctl( MODEM_IOCTL_RX_AVAIL, &Avail ) != MODEM_OK )
				  // Avail=iLen+6;
	     	  if( cnxt_modem_ioctl( MODEM_IOCTL_RX_AVAIL, &Avail ) != MODEM_OK )
	     	    {
	       	    NosSleep(1);
		    }
	     	  else
	       	    {
	       	    api_tim_close( dhn_tim );
	       	    
	       	    // receive the response of AT command
	       	    cnxt_modem_read( temp, &tmpCount );
	       	    sprintf( (char *)&os_mdm56k_atc_buffer[os_mdm56k_atc_length], "%s", "\r\nOK\r\n" );
	       	    
	       	    if( delay == 0 )
	       	      tmpCount = os_mdm56k_atc_length;
				int g=0;
				// while(os_mdm56k_atc_buffer[g])
				// {
	       	    //  printf("temp[%d]=%c\tos_mdm56k_atc_buffer[%d]=%c\n",g,temp[g],g,os_mdm56k_atc_buffer[g]);
				//  g++;
				// }
	       	    if( strncmp( os_mdm56k_atc_buffer, temp, tmpCount ) != 0 )	// OK?
				   {
					   	printf("strncmp fail\n");
						return( MODEM_ERROR );
				   }
	              
	            else
	              break;
	       	    }
	       	  }
	     
//	     if( delay )
//	       NosSleep(1);
	     
	     bakCount += Count;
	     j = 0;
	     }
	   }
	
	*pcount = bakCount;
	return( MODEM_OK );
}
