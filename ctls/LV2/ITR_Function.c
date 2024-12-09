#include <string.h>
#include "POSAPI.h"
#include "Define.h"
#include "Function.h"


extern UCHAR dhn_ICC;

extern UCHAR mifare_dhnAUXInterrupt;



extern ULONG	OS_GET_KbdEventFlag( void );
extern void		OS_SET_KbdEventFlag( ULONG value );

extern ULONG	OS_GET_MsrEventFlag( void );
extern void		OS_SET_MsrEventFlag( ULONG value );

UCHAR ITR_Reset_Flags(void)
{
#ifdef _PLATFORM_AS350
	UCHAR msrStatus[4]={0};
	
	OS_SET_KbdEventFlag(FALSE);
	OS_SET_MsrEventFlag(FALSE);

	//Reset MSR data
	api_msr_status(0x00, 1, msrStatus);
#endif

	return TRUE;
}

UCHAR ITR_Check_MagStripe(void)
{

	// Wayne add for testing only 
//   return FALSE;
  
#ifdef _PLATFORM_AS350
	if (OS_GET_MsrEventFlag() == TRUE)
	{
		UT_BUZ_Beep1();
		OS_SET_MsrEventFlag(FALSE);
		
		return TRUE;
	}

#endif

	return FALSE;
}

UCHAR ITR_Check_ICC(void)
{
  // Wayne add for testing only 
//   return FALSE;

#ifdef _PLATFORM_AS350
	if (api_ifm_present(dhn_ICC) == 0x00)
	{
		UT_BUZ_Beep1();
		
		return TRUE;
	}
#endif

	return FALSE;
}

UCHAR ITR_Check_Cancel(void)
{
	// Wayne add for testing only 
//   return FALSE;

#ifdef _PLATFORM_AS350
	if (OS_GET_KbdEventFlag() == TRUE)
	{
		UT_BUZ_Beep1();
		OS_SET_KbdEventFlag(FALSE);

		return TRUE;
	}
#endif

	return FALSE;
}


UCHAR ITR_Check_StopNow(void)
{
	UCHAR rspCode=FALSE;

//Test
/*
#ifdef _PLATFORM_AS350
	UINT  rcvLen=0;

	if (mifare_dhnAUXInterrupt != 0)
	{
		rspCode=api_aux_rxready(mifare_dhnAUXInterrupt, (UCHAR*)&rcvLen);
		if (rspCode == apiOK)
		{
			rspCode=TRUE;
		}
		else
		{
			rspCode=FALSE;
		}
	}
#else
	rspCode=VAP_RIF_Check_ResetCommand();
#endif
*/
	return rspCode;
}

UCHAR ITR_Check_StopLater(void){return FALSE;}

