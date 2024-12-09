#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "POSAPI.h"
#include "ExtIODev.h"
#include "API_TSC.h"
#include "DEV_TSC.h"
#include "bsp_gpio.h"
#include "fbutils.h"

int		os_DHN_TSC;
UCHAR		os_TSC_ORIENT = FALSE;
UCHAR TSC_CheckDHN( UINT16 dhn )
{
	if( ((dhn > 0) && (dhn == os_DHN_TSC)) ||(dhn ==0))
	  return( TRUE );//true
	else
	  return( FALSE );//false
}

UCHAR api_tsc_open( UCHAR deviceid )
{	
	if( os_DHN_TSC != 0 )		// already opened?
		return( apiOutOfService );	  
	  // return( os_DHN_TSC );//modified 20201221,for ap variable "gl_devtsc" overwrite by multiple open.
	  
	os_DHN_TSC=OS_TSC_Open();
	printf("os_DHN_TSC=%d\n",os_DHN_TSC);
	if( os_DHN_TSC == FALSE )
	  return( apiFailed );
	  
	return( os_DHN_TSC );
}

//UCHAR api_tsc_close( UINT16 dhn )
UCHAR api_tsc_close(UCHAR dhn)
{	
	if( TSC_CheckDHN( dhn ) == TRUE )
	  {
		if(OS_TSC_Close()==TRUE)
		{
			os_DHN_TSC=0;
			return( apiOK );//apiOK
		}
		return( apiFailed );
	  }
	else
	  return( apiFailed );//apiFailed
}

//UCHAR	api_tsc_status( UCHAR dhn, UCHAR *sbuf, UCHAR *status )
UCHAR	api_tsc_status( UCHAR dhn, API_TSC_PARA para, UCHAR *STATUS )
{
//API_TSC_PARA para;

	//memmove( &para, sbuf, sizeof(API_TSC_PARA) );
	
	if( TSC_CheckDHN( dhn ) == TRUE )
	  {	
	  para.fd=os_DHN_TSC;
	  if( OS_TSC_Status( para, STATUS ) )
	  {
	    return( apiOK );
	  }
	  else
	    return( apiFailed );
	  }
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To activate SignPad function.
// INPUT   : dhn
//	     The specified device handle number.
//	     para	- parameters.
//
//		sbuf	- para
//
// OUTPUT  : status	- status byte.
//			  0: not available (abort or timeout)
//			  1: available	   (sign confirmed)
//			  2: change orientation and re-sign
//	     palette	- fixed 3 bytes for RGB.
//			
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_tsc_signpad( UCHAR dhn, API_TSC_PARA para, UCHAR *status, UCHAR *palette )
//UCHAR	api_tsc_signpad( UCHAR dhn, UCHAR *sbuf, UCHAR *status, UCHAR *palette )
{
//API_TSC_PARA para;
	
	//memmove( &para, sbuf, sizeof(API_TSC_PARA) );
	
	os_TSC_ORIENT = para.RFU[1];
	
	if( TSC_CheckDHN( dhn ) == TRUE )
	  {
	  para.fd=dhn;
	  if( (para.RFU[0] == SIGN_STATUS_PROCESSING) || (para.RFU[0] == SIGN_STATUS_CLEAR) )
	    goto RE_SIGN;
SIGN:
	  OS_TSC_ShowSignPad( para, os_TSC_ORIENT, palette );
RE_SIGN:
	  *status = 0;
	  if( OS_TSC_SignPad( para, status, os_TSC_ORIENT ) )
	    {
//	    if( *status == SIGNPAD_STATUS_ROTATE )	// change orientation
//	      {
//	      ggg_cnt++;
//	      
//	      if( os_TSC_ORIENT == 0 )
//	        os_TSC_ORIENT = 1;
//	      else
//	        os_TSC_ORIENT = 0;
//	      goto SIGN;
//	      }
	    
	    if( *status == SIGN_STATUS_PROCESSING )
	      return( apiOK );
	    
	    if( *status == SIGNPAD_STATUS_CLEAR )	// re-sign
	      {
//	      para.RFU[0] = 0;
//	      OS_TSC_ShowRotateButton( TRUE, os_TSC_ORIENT );
//	      goto RE_SIGN;
	      
	      OS_TSC_ShowRotateButton( TRUE, os_TSC_ORIENT );
		  OS_TSC_ClearSignpad( para, os_TSC_ORIENT );
	      *status = SIGN_STATUS_CLEAR;
	      
	      return( apiOK );
	      }
	    
	    *status &= 0x0F;
	    return( apiOK );
	    }
	  else
	    return( apiFailed );
	  }
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get image data of the signature.
// INPUT   : dhn
//	     The specified device handle number.
//	     para	- parameters.
//			  .RFU[0] = 0: the output format is raw data.
//			  .RFU[0] = 1: the output format is Windows BMP.
//			  .RFU[0] = 2: the output format is Windows BMP + raw data.
//			  .RFU[1] = 0: normal direction (CCW_0)
//			  .RFU[1] = 1: reverse direction (CCW_180)
//
//		sbuf	- para
//
// OUTPUT  : status	- status byte.
//			  0: not available (no data)
//			  1: available	   (data available)
//	     sign	- image data of the signature (fixed 320x160 bitmap)
//			  available only when "status" = 1.
//	     length	- length of the image data.
//
//		dbuf	- length(4)+sign(n)
//
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_tsc_getsign( UCHAR dhn, API_TSC_PARA para, UCHAR *status, UCHAR *sign, ULONG *length )
//UCHAR	api_tsc_getsign( UCHAR dhn, UCHAR *sbuf, UCHAR *status, UCHAR *dbuf )
{
//API_TSC_PARA para;
//UCHAR	*sign;
//ULONG	length;
	//memmove( &para, sbuf, sizeof(API_TSC_PARA) );
	//sign	= dbuf;
	
	if( TSC_CheckDHN( dhn ) == TRUE )
	  {	 
	  if( OS_TSC_GetSign( para, status, sign, length ) )
	    return( apiOK );
	  else
	    return( apiFailed );
	  }
	else
	  return( apiFailed );
}
#if 0
#endif

