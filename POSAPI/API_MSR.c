


#include "POSAPI.h"
#include "OS_PROCS.h"
#include "DEV_MSR.h"


UCHAR		os_DHN_MSR = 0;


// ---------------------------------------------------------------------------
// FUNCTION: To check if DHN matched.
// INPUT   : dhn
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UCHAR MSR_CheckDHN( UCHAR dhn )
{
	if( (dhn == 0) || (dhn == os_DHN_MSR) )
	  return( TRUE );
	else
	  return( FALSE );
}


// ---------------------------------------------------------------------------
// FUNCTION: To enable the track(s) of MSR,
//	     all captured data and status will be cleared.
// INPUT   : tracks -- MSR track number.
// OUTPUT  : none.
// RETURN  : DeviceHandleNo
//           apiOutOfService
// ---------------------------------------------------------------------------

UCHAR api_msr_open( UCHAR tracks )
{
	
	if( os_DHN_MSR != 0 )	// already opened?
	  return( apiOutOfService );
	
	if( tracks & (isoTrack1 + isoTrack2 + isoTrack3) )
	  {
	  //os_pMcr = OS_MSR_Open( tracks );
	  if( !OS_MSR_Open( tracks ))
	    return( apiOutOfService );
	  }
	else
	  return( apiOutOfService );	// no any track assigned
	
	os_DHN_MSR = psDEV_MSR + 0x80;
	return( os_DHN_MSR );
}


// ---------------------------------------------------------------------------
// FUNCTION: To disable all tracks of MSR device.
// INPUT   : dhn
//	     The specified device handle number.
//	     0x00 = to close all opened tasks.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
/*
        close related interrupt
*/
UCHAR api_msr_close( UCHAR dhn )
{
	
	if( MSR_CheckDHN( dhn ) == TRUE )
	  {
	  OS_MSR_Close();
	  
	  os_DHN_MSR = 0;
	  return( apiOK );
	  }
	else
	  return( apiFailed );
}




// ---------------------------------------------------------------------------
// FUNCTION: To get the current status of MSR track(s).
// INPUT   : dhn
//	     The specified device handle number.
//	     action
//           0 = clear the status after reading.
//           1 = keep the status after reading.
// OUTPUT  : dbuf
//	     UCHAR  swipe ;            // MSR swiped status.
//	     UCHAR  isoTrk1 ;          // iso track1 status.
//	     UCHAR  isoTrk2 ;          // iso track2 status.
//	     UCHAR  isoTrk3 ;          // iso track3 status.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR api_msr_status( UCHAR dhn, UCHAR action, UCHAR *dbuf )
{
	if( MSR_CheckDHN( dhn ) == TRUE )
	  {
	  OS_MSR_Status( action, dbuf );
	  
	  return( apiOK );
	  }
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To read the captured data from the MSR device.
// INPUT   : dhn
//	     The specified device handle number.
//	     sbuf
//	     UCHAR  action ; 	// 0x00 = system will clear all captured data after reading.
//	                   	// 0x01 = system will keep all captured data after reading.
//	     UCHAR  TrackNo ;	// MSR Track Number
// OUTPUT  : dbuf
//	     struct TRACK  isoTrk1 ;    // contents of iso track1.
//	     struct TRACK  isoTrk2 ;    // contents of iso track2.
//	     struct TRACK  isoTrk3 ;    // contents of iso track3.
//	     struct TRACK {                       // the format of track contents.
//	                  UCHAR  Length ;         // length of the track data.
//	                  UCHAR  StartSentinel ;  // start sentinel of the track.
//	                  UCHAR  Data[Length-2] ; // data in the track.
//                        UCHAR  EndSentinel ;    // end sentinel of the track.
//	                  } ;
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR api_msr_getstring( UCHAR dhn, UCHAR *sbuf, UCHAR *dbuf )
{
	if( MSR_CheckDHN( dhn ) == TRUE )
	  {
	  if( sbuf[1] & (isoTrack1 + isoTrack2 + isoTrack3 ) )
	    {
	    OS_MSR_Read(sbuf[0], sbuf[1], dbuf );
	    return( apiOK );
	    }
	  else
	    return( apiFailed );
	  }
	else
	  return( apiFailed );
}
