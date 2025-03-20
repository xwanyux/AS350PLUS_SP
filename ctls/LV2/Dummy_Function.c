/*
 * Dummy_Function.c
 */

#include "POSAPI.h"
#include "LCDTFTAPI.h"
#include "USBAPI.h"



// Add by Wayne to Pass the compiler of LV2()
// <------ 

//UCHAR	api_ifm_open( UCHAR acceptor ){return apiOK;}
//UCHAR	api_ifm_present( UCHAR dhn ){return apiOK;}


// UCHAR	api_msr_open( UCHAR tracks ){return apiOK;}
// UCHAR	api_msr_status( UCHAR dhn, UCHAR action, UCHAR *dbuf ){return apiOK;}
// UCHAR	api_msr_close( UCHAR dhn ){return apiOK;}
// UCHAR	api_msr_getstring( UCHAR dhn, UCHAR *sbuf, UCHAR *dbuf ){return apiOK;}

//UCHAR	api_sys_backlight( UCHAR device, ULONG duration ){return apiOK;}
//ULONG	api_sys_info( UCHAR id, UCHAR *info ){return apiOK;}

//ULONG	api_rsa_loadkey( UCHAR *modulus, UCHAR *exponent ){return apiOK;}
//ULONG	api_rsa_recover( UCHAR *pIn, UCHAR *pOut ){return apiOK;};
//ULONG	api_rsa_encrypt( API_RSA cipher ){return apiOK;}
void	api_rsa_release( void ){return;}

//UCHAR	api_emv_GetDataElement( UCHAR source, ULONG address, ULONG length, UCHAR *data ){return apiOK;}

// UCHAR	api_lcdtft_showPCD( UCHAR dhn, API_PCD_ICON icon ){return apiOK;}
#ifdef PCD_PLATFORM_CLRC663
UCHAR	api_usb_open( UCHAR port, API_USB_PARA para ){return apiOK;}
UCHAR	api_usb_close( UCHAR dhn ){return apiOK;}
UCHAR	api_usb_rxready( UCHAR dhn, UCHAR *dbuf ){return apiOK;}
UCHAR	api_usb_rxstring( UCHAR dhn, UCHAR *dbuf ){return apiOK;}
UCHAR	api_usb_txready( UCHAR dhn ){return apiOK;}
UCHAR	api_usb_txstring( UCHAR dhn, UCHAR *sbuf ){return apiOK;}
#endif
// <-------





//ULONG	api_3des_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey ){return apiOK;}
//ULONG	api_3des_decipher( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey ){return apiOK;}
//ULONG	api_aes_decipher( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey, UCHAR KeySize ){return apiOK;}
//ULONG	api_aes_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize ){return apiOK;}
#ifndef _build_DSS_
UCHAR	api_dss2_apid( void ){return apiOK;}
UCHAR	api_dss2_burn( ULONG apid ){return apiOK;}
UCHAR	api_dss2_file( ULONG offset, ULONG length, UCHAR *data ){return apiOK;}
UCHAR	api_dss2_init( UCHAR mode ){return apiOK;}
UCHAR	api_dss2_run( UCHAR apid ){return apiOK;}
#endif
//UCHAR	api_emv_GetDataElement( UCHAR source, ULONG address, ULONG length, UCHAR *data ){return apiOK;}
//void	api_emvk_IncTransSequenceCounter( void ){;}
//ULONG	api_fls_read( UCHAR id, ULONG addr, ULONG len, UCHAR *buf ){return apiOK;}
//ULONG	api_fls_write( UCHAR id, ULONG addr, ULONG len, UCHAR *buf ){return apiOK;}
//UCHAR	api_ifm_present( UCHAR dhn ){return apiOK;}
//UCHAR	api_lcdtft_showPCD( UCHAR dhn, API_PCD_ICON icon ){return apiOK;}
//ULONG	api_rsa_loadkey( UCHAR *modulus, UCHAR *exponent ){return apiOK;}
//ULONG	api_rsa_recover( UCHAR *pIn, UCHAR *pOut ){return apiOK;}
//UCHAR	api_rtc_getdatetime( UCHAR dhn, UCHAR *dbuf ){return apiOK;}
//UCHAR	api_rtc_setdatetime( UCHAR dhn, UCHAR *sbuf ){return apiOK;}
//ULONG	api_sys_random( UCHAR *dbuf ){dbuf[0]=1;return apiOK;}
//ULONG	api_sys_SHA1( ULONG length, UCHAR *data, UCHAR *digest ){return apiOK;}

UCHAR api_2DR_StartScan(void){return apiOK;}
UCHAR api_2DR_ReceiveData(UINT *optLen, UCHAR *optData){return apiOK;}
UCHAR api_2DR_StopScan(void){return apiOK;}

// Wayne modify
// UCHAR	apk_tsc_switch( UCHAR flag )
// {
// 	api_tsc_close(0);

// 	return apiOK;}

//ULONG	OS_GET_KbdEventFlag( void ){return 0;}
//void	OS_SET_KbdEventFlag( ULONG value ){;}



UCHAR dumy_dhn_timer=0;
UCHAR dumy_dhn_msr=0;
UCHAR dumy_dhn_ifm=0;

void DUMY_Open_Timer10MS(void)
{
	dumy_dhn_timer = api_tim_open(1); // 1 tick unit = 10 ms
}


void DUMY_Open_MSR(void)
{
	dumy_dhn_msr=api_msr_open( isoTrack1 + isoTrack2 );
}


void DUMY_Open_IFM(void)
{
	dumy_dhn_ifm=api_ifm_open(ICC1);
}


#if	0
ULONG OS_GET_SysTimerFreeCnt( void )
{
	ULONG tmrTick=0;

	api_tim_gettick( dumy_dhn_timer, (UCHAR *)&tmrTick );

	return tmrTick;
}
#endif

#if	0
void OS_SET_SysTimerFreeCnt( ULONG value )
{
	;
}
#endif

#if	0
ULONG OS_GET_MsrEventFlag( void )
{
	UCHAR msrStatus[4]={0};

	api_msr_status( dumy_dhn_msr, 0, msrStatus );
	if ( msrStatus[0] == msrSwiped )
	{
		return TRUE;
	}

	return FALSE;
}
#endif

#if	0
void OS_SET_MsrEventFlag( ULONG value )
{
	;
}
#endif
