//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							    **
//**  PRODUCT  : AS350+							    **
//**                                                                        **
//**  FILE     : POSTFUNC.C                                                 **
//**  MODULE   : 							    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/12/07                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
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

#include "bsp_types.h"
#include "bsp_gpio.h"
//#include "bsp_mcr.h"
//#include "bsp_mem.h"

//#include "OS_POST.h"
#include "OS_MSG.h"
#include "OS_LIB.h"
#include "OS_PED.h"
#include "OS_SECM.h"
//#include "OS_FLASH.h"
//#include "DEV_SRAM.h"
//#include "DEV_WDT.h"
//#include "DEV_LCD.h"
//#include "DEV_KBD.h"
#include "POSAPI.h"
//#include "SBOOT.h"

//#include "EMVAPI.h"

//#include "bsp_uart.h"
//#include "flash.h"
//#include "console.h"
//#include "download.h"
//#include "defines.h"

#include "LCDTFTAPI.h"
#include "TSCAPI.h"
#include "USBAPI.h"
#include "LTEAPI.h"
#include "FSAPI.h"

#include "API_EXTIO.h"
#include "ExtIODev.h"

//#include "IAPAPI.h"
#include "FLASHAPI.h"


//const UCHAR FONT2_ASCII_12x24[] = {
//#include "sii_font_12x24.h"
//};

//const UCHAR FONT4_ASCII_8x16[] = {
////#include "prt_font4_8x16.h"
//#include "iso88591_prt_font4_8x16.h"
//};

//extern	UINT8	const	BIOS_RELEASE_DATE[];
//extern	void	OS_SRAM_GetConfig( UINT32 *BaseAddr, UINT32 *MemorySize );
//extern	UINT32	OS_FLASH_PageInit( UINT32 type );	// defined in DEV_FLASH.c

// Global DSS parameters
//UCHAR	DssTelNum[23+1]={0};
//UCHAR	DssRemoteIP[23+1]={0};
//UCHAR	DssRemotePort[23+1]={0};
//UCHAR	DssPort[23+1]={0};

//extern	BSP_GPIO	*pScGpio;		// declared in SC.c of BSP
//extern	UINT32		g_ScSpiErr1;		// declared in SC.c of BSP
//extern	UINT32		g_ScSpiErr2;		// declared in SC.c of BSP

//extern	FLASH_INFO	FlashInfo;
//extern	BSP_UART	*pUart;
//extern	UINT32		FlashSizeMB;
//extern	UINT32		FlashBlockSize;
//extern	UINT32		FlashMaxAddr;
//extern	UINT32		os_ConsoleFlag;
//extern	UINT32		fWarmBoot;

//extern	UINT8		post_dhn_aux;

//extern			BSP_LCD	BspLcd;

//extern	UINT32		os_HpCopFlag;
//extern	UINT32		os_TSC_ENABLED;

//extern	UINT8		os_FLASH_INFO[];	// declared in DEV_FLASH.c

#define	DATA_FLASH_START_PAGE		0			// data flash area
#define	SYST_FLASH_START_PAGE		127			// system flash area

#if 1
// ---------------------------------------------------------------------------
extern	UINT32	os_TSC_RESIST;					// defined in DEV_TSC.c

// DSS parameters defined in Down.c
extern	UCHAR	DssTelNum[23+1];		
extern	UCHAR	DssRemoteIP[23+1];	
extern	UCHAR	DssRemotePort[23+1];
extern	UCHAR	DssPort[23+1];
extern  UCHAR   DssIP[23+1];
extern  UCHAR   DssGateway[23+1];
extern  UCHAR   DssSubNetMask[23+1];


// ---------------------------------------------------------------------------
void	POST_FLASH_PageInit( void )
{
API_FLASH pFls;


	api_flash_TypeSelect( TYPE_NOR );
	
	pFls.StPage = 0;
	pFls.EndPage = SYST_FLASH_START_PAGE-1;
	api_flash_PageLink( pFls, 0 );		// link all pages to one except the last page
}

// ---------------------------------------------------------------------------
// FUNCTION: Show status of MSR.
//	     TRACKn:   OK (or ERROR)
// INPUT   : track  - track no. (1..3)
//	     status - MSR status.
//                    -1 = to clear all statuses.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	DIAG_ShowStatusMSR( UINT8 track, UINT8 status )
{
UINT8	i;
UINT8	row;
UINT8	col;

	row = track;
	col = 10;
	
	if( status == (UINT8)-1 )
	  {
	  for( i=0; i<5; i++ )
	     LIB_LCD_Puts( i, col, FONT0, sizeof(os_msg_5SP), (UINT8 *)os_msg_5SP );
	  return;
	  }
	
	switch( status )
	      {
	      case msrNoData:		// no data
	      
	      	   LIB_LCD_Puts( row, col, FONT0, sizeof(os_msg_5SP), (UINT8 *)os_msg_5SP );
	      	   break;

	      case msrDataOK:		// data ok
	      
	      	   LIB_LCD_Puts( row, col, FONT0, sizeof(os_msg_OK5), (UINT8 *)os_msg_OK5 );
	      	   break;

	      case msrDataError:	// data error
	      
	      	   LIB_LCD_Puts( row, col, FONT0, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR );
	      	   break;
	      }

}

// ---------------------------------------------------------------------------
// FUNCTION: Show contents of MSR Tracks.
//	     TRACKn
//           xx xx xx xx xx xx xx
//	     xx xx xx
//
//	     Track data format.
//	     L-Track1
//	     L-Track2
//	     L-Track3
//
// INPUT   : dhn 
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	DIAG_ShowDataTracks( UINT8 dhn, UINT8 *Status, UINT8 *Data )
{
UINT8	sbuf[2];
UINT16	len;

	
	if( Status[0] == msrSwiped )
	  {
	  // Track 1
	  if( Status[1] == msrDataOK )
	    {
	    LIB_LCD_Puts( 0, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_TRACK1), (UINT8 *)os_msg_TRACK1 );
	    
	    len = *Data++;
	    LIB_DumpHexData( 1, 1, len, Data );
	    Data += len;
	    }
	  
	  // Track 2
	  if( Status[2] == msrDataOK )
	    {
	    LIB_LCD_Puts( 0, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_TRACK2), (UINT8 *)os_msg_TRACK2 );

	    len = *Data++;
	    LIB_DumpHexData( 1, 1, len, Data );
	    Data += len;
	    }
	    
	  // Track 3
	  if( Status[3] == msrDataOK )
	    {
	    LIB_LCD_Puts( 0, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_TRACK3), (UINT8 *)os_msg_TRACK3 );
	    
	    len = *Data++;
	    LIB_DumpHexData( 1, 1, len, Data );
	    }
	  
//	  sbuf[0] = 0; // clear data after reading
//	  sbuf[1] = isoTrack1 + isoTrack2 + isoTrack3;
//	  api_msr_getstring( dhn, sbuf, dbuf );
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: 14 - Test Magnetic Stripe Reader. (3-track)
//		  MSR
//		  TRACK1: OK (or ERROR)
//		  TRACK2: OK (or ERROR)
//		  TRACK3: OK (or ERROR)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
UINT32	DIAG_FUNC_TestMsr( void )
{
UINT32	i;
UINT8	dhn_msr;
UINT8	key;
UINT8	status;
UINT8	flag;
UINT8	MsrStatus[4];
UINT8	BakMsrStatus[4];
UINT8	sbuf[2];
UINT8	dbuf[300];


	// open all tracks
	dhn_msr = api_msr_open( isoTrack1 + isoTrack2 + isoTrack3 );
	
	if( dhn_msr == apiOutOfService)
	  return( FALSE );
	  
	while(1)
	{
	LIB_LCD_Cls();
	
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_MSR), (UINT8 *)os_msg_MSR );
	LIB_LCD_Puts( 1, 0, FONT0, sizeof(os_msg_TRACK1), (UINT8 *)os_msg_TRACK1 );
	LIB_LCD_Puts( 2, 0, FONT0, sizeof(os_msg_TRACK2), (UINT8 *)os_msg_TRACK2 );
	LIB_LCD_Puts( 3, 0, FONT0, sizeof(os_msg_TRACK3), (UINT8 *)os_msg_TRACK3 );
	LIB_LCD_Puts( 7, 0, FONT0, sizeof(os_msg_VIEW_QUIT), (UINT8 *)os_msg_VIEW_QUIT );
	
	i = 0;
	flag = TRUE;
	while(flag)
	     {
	     if( LIB_GetKeyStatus() == apiReady )
	       {
	       key = LIB_WaitKey();
	       switch( key )
	             {
	             case 'y':	// view
	             
	             	  DIAG_ShowDataTracks( dhn_msr, (UINT8 *)BakMsrStatus, (UINT8 *)dbuf );
	             	  
	             	  flag = FALSE;
	             	  continue;
	             	  
	             case 'x':	// quit
	             
	             	  // close all tracks
	             	  api_msr_close( dhn_msr );
	             	  
	             	  LIB_LCD_ClearRow( 7, 1, FONT0 );
	             	  return( TRUE );
	             }
	       }
	     
	     api_msr_status( dhn_msr, 0, MsrStatus );		// get MSR status
	     if( MsrStatus[0] == msrSwiped )
	       {
	       LIB_BUZ_Beep1();
	       
	       memmove( BakMsrStatus, MsrStatus, 4 );
	       
	       // Read MSR data
	       sbuf[0] = 1; // keep data after reading
	       sbuf[1] = isoTrack1 + isoTrack2 + isoTrack3;
	       api_msr_getstring( dhn_msr, sbuf, dbuf );	// get MSR data
	       
	       // Show MSR status
	       for( i=1; i<4; i++ )
	          DIAG_ShowStatusMSR( i, MsrStatus[i] );
	       }
	       
	     } // while(flag)

        } // while(1)
	
	
}

// ---------------------------------------------------------------------------
// FUNCTION: 21 - Test Ethernet module.
//		  
//		  01234567890123456789012345678901
//		  ETHERNET
//		  1=SETUP 2=PING
//		  
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
UINT32	DIAG_FUNC_TestIP( void )
{
//#ifdef	_LAN_ENABLED_

API_IPCONFIG config;
//API_IPCONFIG config[1];
UINT8	result;
UINT8	msg_ENABLE_DHCP[] = {"ENABLE DHCP"};
UINT8	msg_DHCP[]	  = {"DHCP..."};
UINT8	ip[20];	// L-V
UINT32	timeUS;
UINT8	select;
UINT32	int_1;
UINT32	dec_1;
UINT32	dec_2;
UINT32	remainder;
UINT8	len, len1;
UINT8	abuf[20];
UINT8	buffer[20];

UINT8	MAC[6];
UINT32	data;

	
	LIB_LCD_Puts( 0, 0, FONT0, strlen(os_msg_ETHERNET), (UINT8 *)os_msg_ETHERNET );
	LIB_LCD_Puts( 1, 0, FONT0, strlen(os_msg_1SETUP_2PING), (UINT8 *)os_msg_1SETUP_2PING );
	
//	POST_AutoSetMAC();
	
	do{
	  select = LIB_WaitKey();
	  if( select == 'x' )
	    return( TRUE );
	  }while( (select != '1') && (select != '2') && (select != '3') && (select != '9') &&(select != 'y') );

//	if( select == '3' )	// E-SUN BANK TCP DEBUG ONLY
//	  {
//	  ESUN_TCP_DEBUG();
//	  return( FALSE );
//	  }
	  
//	if( select == '9' )	// debug?
//	  {
//	  os_DHCP_DEBUG = TRUE;	// referenced by DHCP driver
//	  select = '1';
//	  }

	if( select == 'y' )
	  select = '2';
	
	if( select == '1' )	// setup IP config
	  {
	  // SETUP
	  // MAC: (internal)
	  // IP:
	  // SUBNET MASK:
	  // GATEWAY:
	  
#ifdef	_DHCP_ENABLED_

	  // show current setting
	  LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, strlen(msg_DHCP), (UINT8 *)msg_DHCP );
	  
	  if( PED_GetStateDHCP() )
	    LIB_LCD_Puts( 1, strlen("msg_DHCP"), FONT0, strlen("ON"), (UINT8 *)"ON" );
	  else
	    LIB_LCD_Puts( 1, strlen("msg_DHCP"), FONT0, strlen("OFF"), (UINT8 *)"OFF" );
	  
	  // enable DHCP?
	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_ENABLE_DHCP), (UINT8 *)msg_ENABLE_DHCP );
	  select = LIB_WaitKeyYesNo( 2, strlen(msg_ENABLE_DHCP) );
	  
	  LIB_LCD_ClearRow( 2, 1, FONT0 );
	  
	  LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, strlen(msg_DHCP), (UINT8 *)msg_DHCP );
	  
	  result = api_lan_setup_DHCP( select );	// force to exec DHCP
	  if( !select )
	    goto DHCP_END;
	    
	  
	  if( result != apiOK )
	    {
	    LIB_LCD_Puts( 1, strlen("msg_DHCP"), FONT0, strlen(os_msg_ERROR), (UINT8 *)os_msg_ERROR );
	    
	    LIB_WaitKey();
	    return( TRUE );
	    }
	  else
	    {
	    data = 600;	// DHCP timeout 60 sec
	    while(1)
	         {
	         if( LIB_GetKeyStatus() == apiReady )
	           {
	           if( LIB_WaitKey() == 'x' )	// abort?
	             {
	             api_lan_setup_DHCP( FALSE );
	             goto DHCP_END;
	             }
	           }
	           
	         if( api_lan_status_DHCP() == apiReady )
	           {
	           LIB_LCD_Puts( 1, strlen("msg_DHCP"), FONT0, sizeof(os_msg_OK), (UINT8 *)os_msg_OK );
	           break;
	           }
	         else
	           {
	           if( data-- )
//	             BSP_Delay_n_ms(100);
	             LIB_WaitTime(10);
	           else
	             {
	             LIB_LCD_Puts( 1, strlen("msg_DHCP"), FONT0, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR );
	             
//	             DIAG_FUNC_DumpLogForDHCP();
	             
	             LIB_WaitKey();
	             return( TRUE );
	             }
	           }
	         }
	    }
	  
	  LIB_WaitTimeAndKey(100);
	  
//	  DIAG_FUNC_DumpLogForDHCP();
	  
DHCP_END:
	  LIB_LCD_ClearRow( 1, 1, FONT0 );
	  
#endif	// _DHCP_ENABLED_

	  
	  // setup default MAC
#if	0
	  data = BSP_RD32(PMU_ID0_REG);	// binary format: CUST_ID(1) + SN(3)
	  
#if	0
	  MAC[0] = 0x00;	// SYMLINK OUI = 00 F0 FF (test only)
	  MAC[1] = 0xF0;	//
	  MAC[2] = 0xFF;	//
	  
	  MAC[3] = (data & 0x00FF0000) >> 16;
	  MAC[4] = (data & 0x0000FF00) >> 8;
	  MAC[5] =  data & 0x000000FF;
#else
	  MAC[0] = 0x6C;	// SYMLINK OUI = 6C 15 24 Dx (formal 28 bits)
	  MAC[1] = 0x15;	//
	  MAC[2] = 0x24;	//
	  MAC[3] = 0xD0;	//
	
	  MAC[3] |= ((data & 0x000F0000) >> 16);
	  MAC[4] =   (data & 0x0000FF00) >> 8;
	  MAC[5] =    data & 0x000000FF;
#endif

	  API_setMAC( MAC );
#endif

	  api_lan_getIPconfig( &config );
	  
	  // enter EDC IP
	  LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_IP), (UINT8 *)os_msg_IP );
	  
	  if( (config.IP != 0) && (config.IP_Len < 16) )
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, config.IP_Len, config.IP );
	    
//	  while(1)
//	  {
	  if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 3, 15, ip ) != TRUE )
	    return( TRUE );
	  
	  if( (ip[0] == 0) && (config.IP != 0) && (config.IP_Len < 16) )
	    {	// old
	    ip[0] = config.IP_Len;
	    memmove( &ip[1], config.IP, ip[0] );
	    }
	  else
	    {	// new
	    config.IP_Len = ip[0];
	    memmove( config.IP, &ip[1], ip[0] );
	    }
//	  }

	  // enter SUBNET_MASK
	  LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_SUBNET_MASK), (UINT8 *)os_msg_SUBNET_MASK );

	  if( (config.SubnetMask_Len != 0) && (config.SubnetMask_Len < 16) )
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, config.SubnetMask_Len, config.SubnetMask );

//	  while(1)
//	  {
	  if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 3, 15, ip ) != TRUE )
	    return( TRUE );
	  
	  if( (ip[0] == 0) && (config.SubnetMask_Len != 0) && (config.SubnetMask_Len < 16) )
	    {	// old
	    ip[0] = config.SubnetMask_Len;
	    memmove( &ip[1], config.SubnetMask, ip[0] );
	    }
	  else
	    {	// new
	    config.SubnetMask_Len = ip[0];
	    memmove( config.SubnetMask, &ip[1], ip[0] );
	    }
//	  }
	  
	  
	  // enter GATEWAY
	  LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_GATEWAY), (UINT8 *)os_msg_GATEWAY );
	  
	  if( (config.Gateway_Len != 0) && (config.Gateway_Len < 16) )
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, config.Gateway_Len, config.Gateway );
	  
//	  while(1)
//	  {
	  if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 3, 15, ip ) != TRUE )
	    return( TRUE );
	  
	  if( (ip[0] == 0) && (config.Gateway_Len != 0) && (config.Gateway_Len < 16) )
	    {	// old
	    ip[0] = config.Gateway_Len;
	    memmove( &ip[1], config.Gateway, ip[0] );
	    }
	  else
	    {	// new
	    config.Gateway_Len = ip[0];
	    memmove( config.Gateway, &ip[1], ip[0] );
	    }
//	  }
	  
	  if( api_lan_setIPconfig( config ) != apiOK )
	    return( FALSE );
	  
	  LIB_LCD_ClearRow( 1, 7, FONT0 );
	  }
	
	
	// PING...
	LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_PING), (UINT8 *)os_msg_PING );
	// enter IP address
	do{
	  if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 3, 15, ip ) != TRUE )
	    return( TRUE );
	  }while( ip[0] == 0 );
	
	while(1)
	  {
	  LIB_LCD_ClearRow( 4, 1, FONT0 );
	  
	  if( api_lan_ping( ip[0], &ip[1], &timeUS ) == apiOK )
	    {
//	    LIB_DispHexWord( 3, 0, (UINT16)timeUS );
	    
	    int_1 = timeUS/1000;
	    remainder = timeUS - int_1*1000;
	    dec_1 = remainder/100;
	    remainder -= (dec_1*100);
	    dec_2 = remainder/10;
	    
	    len = 0;
	    len1 = LIB_itoa( int_1, abuf );
	    memmove( buffer, abuf, len1 );
	    
	    buffer[len1++] = '.';
	    len += len1;
	    
	    len1 = LIB_itoa( dec_1, abuf );
	    memmove( &buffer[len], abuf, len1 );
	    len += len1;
	    
	    len1 = LIB_itoa( dec_2, abuf );
	    memmove( &buffer[len], abuf, len1 );
	    len += len1;
	    
	    buffer[len++] = 'm';
	    buffer[len++] = 's';
	    
	    LIB_LCD_Puts( 4, 0, FONT0, len, buffer );
	    
	    if( LIB_WaitTimeAndKey( 300 ) != 255 )	// repeat every 3 seconds till any key stroke
	      break;
	    }
	  else
	    {
	    LIB_LCD_Puts( 4, 0, FONT0, sizeof(os_msg_ERR_NO_RESPONSE), (UINT8 *)os_msg_ERR_NO_RESPONSE );
	    if( LIB_WaitTimeAndKey( 300 ) != 255 )	// repeat every 3 seconds till any key stroke
	      break;
	    }
	  }

//#endif

	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: read or write TSC type from/to FS.
// INPUT   : flag	- 0=read, 1=write
//	     type	- 0=CAP, 1=REG type (for write)
// OUTPUT  : type	- 0=CAP, 1=REG type (for read)
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	POST_SetTscType( UINT32 flag, UINT32 *type )
{
UINT32	result = FALSE;
UINT32	bytes = 0;
UINT32	current = 0;
FILE	*fh;
UINT8	sbuf[8];
UINT8	dbuf[8];


	api_fs_select( MEDIA_FLASH );
	api_fs_init();
	
	fh = api_fs_open( "tsc.ini", 0 );
	if( !fh )
	  {
	  // 1'st time process...
	  
	  api_fs_create( "tsc.ini", 0 );
	  fh = api_fs_open( "tsc.ini", 0 );
	  if( !fh )
	    return( FALSE );
	  
	  if( flag == 0 )	// read
	    {
	    sbuf[0] = 'C';
	    bytes = api_fs_write( fh, sbuf, 1 );
	    
	    *type = 0;
	    result = TRUE;
	    }
	  else			// write
	    {
	    if( *type == 0 )
	      sbuf[0] = 'C';
	    else
	      sbuf[0] = 'R';

	    bytes = api_fs_write( fh, sbuf, 1 );
	    if( bytes == 1 )
	      result = TRUE;
	    }
	  
	  goto EXIT;
	  }
	
	
	// sub-sequence process...
	
	bytes = api_fs_read( fh, dbuf, 1 );	// read current setting
	if( bytes == 1 )
	  {
//	  LIB_DispHexByte( 1, 0, dbuf[0] );
//	  LIB_WaitKey();
	  switch( dbuf[0] )
	        {
	        case 'C':
	             current = 0;
	             break;
	             
	        case 'R':
	             current = 1;
	             break;
	             
	        default:
	             current = 0;
	             break;
	        }
	  
	  if( flag == 0 )	// read(0) or write(1)?
	    {
//	    LIB_DispHexByte( 5, 0, current );
	    *type = current;
	    result = TRUE;
	    }
	  else
	    {
	    api_fs_seek( fh, 0 );
	    
	    if( *type == 0 )
	      sbuf[0] = 'C';
	    else
	      sbuf[0] = 'R';
	    bytes = api_fs_write( fh, sbuf, 1 );
	    if( bytes == 1 )
	      result = TRUE;
	    }
	  }

EXIT:
	api_fs_close( fh );
	
	return( result );
}

// ---------------------------------------------------------------------------
void	POST_SetDefaultTscType( void )
{
	POST_SetTscType( 0, &os_TSC_RESIST );
}

// ---------------------------------------------------------------------------
void	DelayForCapacitiveTSC( void )
{
UINT32	msx10 = 5;


	if( !os_TSC_RESIST )
	  LIB_WaitTime(msx10);
}

// ---------------------------------------------------------------------------
// FUNCTION: 27 - Test TSC module.
//		  
//		  01234567890123456789012345678901
//		  TSC
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
UINT32	DIAG_FUNC_TestTouchScreen( void )
{
UINT32	result = FALSE;
UINT8	status;
UINT8	dhn_tsc;
API_LCDTFT_RECT	rect;
//API_LCDTFT_RECT	rect[1];
API_TSC_PARA	tscpara;
//API_TSC_PARA	tscpara[1];
UINT8	msg_TSC[] =		{"TOUCH SCREEN"};
UINT8	msg_PLS_TOUCH[] =	{"PLEASE TOUCH..."};
UINT8	info[16];

	
//	TEST_TOUCH();
//	return(1);
	
//	if( !os_TSC_ENABLED )
//	  return( FALSE );
	
//	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_TSC), (UINT8 *)msg_TSC );

//	LIB_BUZ_Beep1();
//	BSP_Delay_n_ms(500);
	
	LIB_LCD_Cls();
	LIB_LCD_Puts( 10-4, 7, FONT0, strlen(msg_PLS_TOUCH), (UINT8 *)msg_PLS_TOUCH );

	dhn_tsc = api_tsc_open( 0 );
	
	// ---------------------------------------------------------------------------
	// fill rectangle : #1 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 0;
	rect.Xend	= 31;
	rect.Ystart	= 0;
	rect.Yend	= 31;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );

	// test rectangle : #1
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 0;
	tscpara.Y	= 0;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );
	  
	// fill rectangle : #1 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 0;
	rect.Xend	= 31;
	rect.Ystart	= 0;
	rect.Yend	= 31;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();

	// ---------------------------------------------------------------------------	
	// fill rectangle : #2 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 104;
	rect.Xend	= 104+32-1;
	rect.Ystart	= 0;
	rect.Yend	= 31;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #2
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 104;
	tscpara.Y	= 0;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #2 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 104;
	rect.Xend	= 104+32-1;
	rect.Ystart	= 0;
	rect.Yend	= 31;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();
	
	// ---------------------------------------------------------------------------	
	// fill rectangle : #3 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 240-32-1;
	rect.Xend	= 240-1;
	rect.Ystart	= 0;
	rect.Yend	= 31;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #3
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 240-32-1;
	tscpara.Y	= 0;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #3 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 240-32-1;
	rect.Xend	= 240-1;
	rect.Ystart	= 0;
	rect.Yend	= 31;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();

	// ---------------------------------------------------------------------------	
	// fill rectangle : #4 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 0;
	rect.Xend	= 31;
	rect.Ystart	= 144;
	rect.Yend	= 144+32-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #4
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 0;
	tscpara.Y	= 144;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #4 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 0;
	rect.Xend	= 31;
	rect.Ystart	= 144;
	rect.Yend	= 144+32-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();

	// ---------------------------------------------------------------------------	
	// fill rectangle : #5 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 104;
	rect.Xend	= 104+32-1;
	rect.Ystart	= 144;
	rect.Yend	= 144+32-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #5
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 104;
	tscpara.Y	= 144;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #5 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 104;
	rect.Xend	= 104+32-1;
	rect.Ystart	= 144;
	rect.Yend	= 144+32-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();

	// ---------------------------------------------------------------------------	
	// fill rectangle : #6 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 240-32-1;
	rect.Xend	= 240-1;
	rect.Ystart	= 144;
	rect.Yend	= 144+32-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #6
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 240-32-1;
	tscpara.Y	= 144;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #6 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 240-32-1;
	rect.Xend	= 240-1;
	rect.Ystart	= 144;
	rect.Yend	= 144+32-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();
	
	// ---------------------------------------------------------------------------	
	// fill rectangle : #7 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 0;
	rect.Xend	= 31;
	rect.Ystart	= 320-32-1;
	rect.Yend	= 320-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #7
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 0;
	tscpara.Y	= 320-32-1;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #7 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 0;
	rect.Xend	= 31;
	rect.Ystart	= 320-32-1;
	rect.Yend	= 320-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();

	// ---------------------------------------------------------------------------	
	// fill rectangle : #8 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 104;
	rect.Xend	= 104+32-1;
	rect.Ystart	= 320-32-1;
	rect.Yend	= 320-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #8
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 104;
	tscpara.Y	= 320-32-1;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #8 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 104;
	rect.Xend	= 104+32-1;
	rect.Ystart	= 320-32-1;
	rect.Yend	= 320-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();
	
	// ---------------------------------------------------------------------------	
	// fill rectangle : #9 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 240-32-1;
	rect.Xend	= 240-1;
	rect.Ystart	= 320-32-1;
	rect.Yend	= 320-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #9
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 240-32-1;
	tscpara.Y	= 320-32-1;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #9 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 240-32-1;
	rect.Xend	= 240-1;
	rect.Ystart	= 320-32-1;
	rect.Yend	= 320-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();
	
	api_tsc_close( dhn_tsc );
	
	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: 61 - SIGN PAD Test.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
UINT32	DIAG_FUNC_TestSignPad( void )
{
API_TSC_PARA tscpara;
API_LCDTFT_RECT	rect;

ULONG	length = 0;
UCHAR	dhn_tsc;
UCHAR	dhn_lcd;
UCHAR	sbuf[64];
UCHAR	dbuf[64];
UCHAR	status;
UCHAR	SIGNPAD_direction = 0;


	dhn_lcd = 0;
	dhn_tsc = api_tsc_open( 0 );
	
SIGNPAD:
	sbuf[0] = 255; // palette
	sbuf[1] = 208; //
	sbuf[2] = 139; //
	// clear sign pad message area
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	if( SIGNPAD_direction == 0 )
	  {
	  rect.Xstart = 40+120;
	  rect.Xend = 240;
	  }
	else
	  {
	  rect.Xstart = 0;
	  rect.Xend = 80;
	  }
	
	rect.Ystart = 0;
	rect.Yend = 320;
	rect.Palette[0] = sbuf[0];
	
	rect.Palette[1] = sbuf[1];
	rect.Palette[2] = sbuf[2];
	api_lcdtft_fillRECT( dhn_lcd, rect );
	
	status = 0; // init signpad status
	
SIGNPAD2:
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	
	tscpara.ID = FID_TSC_SIGNPAD;
	tscpara.X = 0;
	tscpara.Y = 40;
	tscpara.Width = 320;
	tscpara.Height = 120;
	tscpara.RxLen = 1;
	tscpara.RFU[0] = status; // current status
	tscpara.RFU[1] = SIGNPAD_direction; // direction (0 or 1)
	tscpara.Timeout = 100*60; // 60 sec (unit: 10ms)
	status = 0;
	api_tsc_signpad( dhn_tsc, tscpara, &status, sbuf );
//	LIB_DispHexByte( 0, 0, status );
//	LIB_WaitKey();
	
	if( status == SIGN_STATUS_TIMEOUT )
	goto EXIT;
	if( status == SIGN_STATUS_PROCESSING )
	goto SIGNPAD2;
	if(status == SIGN_STATUS_CLEAR)
	goto SIGNPAD2;
	if( status == SIGN_STATUS_ROTATE )
	  {
	  if( SIGNPAD_direction == 0 )
	    SIGNPAD_direction = 1;
	  else
	    SIGNPAD_direction = 0;
	  goto SIGNPAD; // restart sign pad
	  }

EXIT:

	api_tsc_close( dhn_tsc);
	
	LIB_LCD_Cls();
	
	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: 87 - Setup TSC type (capacitive or resistive)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	SYS_FUNC_SetTscType( void )
{
UINT32	result;
UINT32	type[1];
UINT8	msg_SETUP_TSC[]	= {"SETUP TSC"};
UINT8	msg_1_CAP[]	= {"1-CAPACITIVE"};
UINT8	msg_2_RES[]	= {"2-RESISTIVE"};
UINT8	select;

 
	if( LIB_EnterPassWord( 3, 1, "2872" ) != TRUE )	// MAX_PED_PSW_CNT
	  return( FALSE );

	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_SETUP_TSC), (UINT8 *)msg_SETUP_TSC );
	
	result = POST_SetTscType( 0, &os_TSC_RESIST );	// read current TSC setting

	LIB_LCD_Puts( 2, 0, FONT0, sizeof(msg_1_CAP), (UINT8 *)msg_1_CAP );
	LIB_LCD_Puts( 3, 0, FONT0, sizeof(msg_2_RES), (UINT8 *)msg_2_RES );

SELECT:
	if( os_TSC_RESIST == 0 )
	  LIB_LCD_Puts( 0, strlen(msg_SETUP_TSC), FONT0, strlen("  -CAP"), (UINT8 *)"  -CAP" );
	else
	  LIB_LCD_Puts( 0, strlen(msg_SETUP_TSC), FONT0, strlen("  -RES"), (UINT8 *)"  -RES" );
	
	do{
	  select = LIB_WaitKey();
	  if( select == 'x' )
	    {
	    result = POST_SetTscType( 1, &os_TSC_RESIST );	// write new TSC setting
	    return( result );	// end of process
	    }
	  }while( (select != '1') && (select != '2') && (select != 'y') );
	  
	if( select == '1' )
	  os_TSC_RESIST = 0;	// CAP
	else
	  os_TSC_RESIST = 1;	// RES
	
	goto SELECT;
}

// ---------------------------------------------------------------------------
// FUNCTION: Load DSS variables from backup flash area.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
ULONG	POST_LoadDssVariables( ULONG flag )
{
UINT32	result = FALSE;
UINT32	bytes = 0;
UINT32	current = 0;
struct	FILE	*fh;
UINT8	sbuf[7*24];


	api_fs_select( MEDIA_FLASH );
	api_fs_init();
	
	fh = api_fs_open( "dss.ini", 0 );
	if( !fh )
	  {
	  // 1'st time process...
	  api_fs_create( "dss.ini", 0 );
	  fh = api_fs_open( "dss.ini", 0 );
	  if( !fh )
	    {
	    result = FALSE;
	    goto EXIT;
	    }

WRITE_DSS:
	  // write current DSS parameters to file
	  memmove( &sbuf[0*24], DssTelNum, 24 );
	  memmove( &sbuf[1*24], DssRemoteIP, 24 );
	  memmove( &sbuf[2*24], DssRemotePort, 24 );
	  memmove( &sbuf[3*24], DssPort, 24 );
      memmove( &sbuf[4*24], DssIP, 24 );
      memmove( &sbuf[5*24], DssGateway, 24 );
      memmove( &sbuf[6*24], DssSubNetMask, 24 );
	  bytes = api_fs_write( fh, sbuf, sizeof(sbuf) );
	  if( bytes == sizeof(sbuf) )
	    {
	    result = TRUE;
	    goto EXIT0;
	    }
	  else
	    {
	    result = FALSE;
	    goto EXIT0;
	    }
	  }

	if( flag )	// write?
	  goto WRITE_DSS;
	  
	// file already exists
	bytes = api_fs_read( fh, sbuf, sizeof(sbuf) );	// read current setting
	if( bytes == sizeof(sbuf) )
	  {
	  // update DSS parameters from file
	  memmove( DssTelNum,     &sbuf[0*24], 24 );
	  memmove( DssRemoteIP,   &sbuf[1*24], 24 );
	  memmove( DssRemotePort, &sbuf[2*24], 24 );
	  memmove( DssPort,       &sbuf[3*24], 24 );
      memmove( DssIP,         &sbuf[4*24], 24 );
      memmove( DssGateway,    &sbuf[5*24], 24 );
      memmove( DssSubNetMask,  &sbuf[6*24], 24 );
	  
	  result = TRUE;
	  }
	else
	  result = FALSE;
	
EXIT0:
	api_fs_close( fh );
EXIT:
	return( result );
}
#else
extern	UINT32	DIAG_FlashErasePages( UINT32 StartAddr, UINT32 NoEraseSize );
extern	UINT32	DIAG_FlashCheckType( void );
extern	UINT32	DIAG_FlashTestRW( UINT32 StartAddr, UINT32 NoEraseSize );
extern	void	DIAG_ShowStatusATR( UINT8 device, UINT8 status );
extern	void	DIAG_ShowStatusMSR( UINT8 track, UINT8 status );
extern	void	DIAG_ShowDataATR( UINT8 device, UINT8 *atr );
extern	void	DIAG_ShowDataTracks( UINT8 dhn, UINT8 *Status, UINT8 *Data );
extern	void	DIAG_SRAM_Erase( void );
extern	void	DIAG_SRAM_Erase2( void );

extern	UINT32	FLASH_BlankCheckSector( void *SectorBase, UINT32 SectorSize );
extern	UINT32	SYS_FUNC_IspLocal( void );
extern	UINT32	SYS_FUNC_IspRemote( void );
extern	UINT32	SYS_FUNC_IspT2T( void );
extern	UINT32	SYS_FUNC_SwitchBootROM( void );
extern	UINT32	SYS_FUNC_SetMAC( void );
extern	UINT32	SYS_FUNC_SetTSN( void );
extern	UINT32	SYS_FUNC_GetVersion( void );
//extern	UINT32	SYS_FUNC_EventLog( void );

extern	UINT32	DIAG_FUNC_TestSram( void );
extern	UINT32	DIAG_FUNC_TestFlash( void );
extern	UINT32	DIAG_FUNC_TestNandFlash( void );
extern	UINT32	DIAG_FUNC_TestPinPad( void );
extern	UINT32	DIAG_FUNC_TestDisplay( void );
extern	UINT32	DIAG_FUNC_TestScr( void );
extern	UINT32	DIAG_FUNC_TestMsr( void );
extern	UINT32	DIAG_FUNC_TestRtc( void );
extern	UINT32	DIAG_FUNC_TestAux( UINT32 mode, UINT32 *status );
extern	UINT32	DIAG_FUNC_TestPrinter( void );
extern	UINT32	DIAG_FUNC_TestCP( void );
extern	UINT32	DIAG_FUNC_TestBackLight( void );
extern	UINT32	DIAG_FUNC_TestGPRS( void );
extern	UINT32	DIAG_FUNC_TestBattPower( void );
extern	void	DIAG_FUNC_TestHardwareReset( void );
extern	UINT32	DIAG_FUNC_TestSpeaker( void );
extern	UINT32	DIAG_FUNC_TestSDCard( void );
extern	UINT32	DIAG_FUNC_Test2DReader( void );
extern	UINT32	DIAG_FUNC_TestModem( void );
extern	UINT32	DIAG_FUNC_TestIP( void );
extern	UINT32	DIAG_FUNC_TestKbdLED( void );
extern	UINT32	DIAG_FUNC_TestRC663( void );
extern	UINT32	DIAG_FUNC_TestMfg( void );
extern	UINT32	DIAG_FUNC_TestTouchScreen( void );
extern	void	POST_MfgTest( void );
extern	UINT32	POST_BurnInTest( void );
extern	UINT32	DIAG_FUNC_TestUSB_DEV( void );
extern	UINT32	DIAG_FUNC_TestUSB_HOST( void );

extern	UINT32	DIAG_FUNC_TestLcdColor( void );
extern	UINT32	DIAG_FUNC_TestSignPad( void );

extern	UINT32	SYS_FUNC_TestLan( void );

extern	UINT32	SYS_FUNC_SetIEK( void );
extern	UINT32	SYS_FUNC_SetTscType( void );
extern	UINT32	SYS_FUNC_SetSubModelID( void );

extern	UINT32	SYS_FUNC_KeyManagementSystem( void );
extern	UINT32	SYS_FUNC_TamperStatus( void );
extern	UINT32	SYS_FUNC_PinLimit( void );
extern	UINT32	SYS_FUNC_RecoverAP( void );
extern	UINT32	POST_DataRetentionTest2( void );
extern	UINT32	SYS_FUNC_SysMaintenance( void );
extern	UINT32	SYS_FUNC_DetectTamper( void );
extern	UINT32	SYS_FUNC_LoadTMK( void );

extern	UINT32	IdentifyFlash16( UINT32 FlashCS );
extern	UINT32	os_MultiBootMode;

extern	UINT8	api_dss2_init( UINT8 mode );
extern	UINT8	api_dss2_apid( void );
extern	UINT8	api_dss2_run( UINT8 apid );

extern	UINT8	*api_dss_getaddr( void );

extern	void	OS_SEC_UnlockAPB( void );
extern	void	OS_SEC_LockAPB( void );

extern	void	POST_FLASH_PageInit_NOR( void );

extern	UINT32	T2T_TaskManager( void );

extern	float	OS_PMU_GetBattVoltage( void );

extern	UINT32	os_DHCP_DEBUG;					// definded in TCPIP.UDP.c
extern	UINT32	os_UDP_LOG_CNT;					//  max 2 logs (OFFER & ACK)
extern	UINT8	*os_UDP_LOG_PTR;				//  EMV WORKING BUFFER (32KB)
								//  LEN1(4) UDP1(n) LEN2(4) UDP2(n)
								
extern	UINT32	os_TSC_RESIST;					// defined in DEV_TSC.c

// PCD
extern UCHAR 	pcd_flgTypeA;
extern UCHAR 	pcd_flgTypeB;
UINT8		os_PCD_TYPE;		// 0=A, 1=B, 2=BX, FF=none

#define SUCCESS				1
#define FAIL				0
#define PCD_DEP_SUCCESS			0x01U	//Success


#define	FN_ABORT			0xFFFFFFFF
#define	FN_SYS_FUNC_ISP_LOCAL		0
#define	FN_SYS_FUNC_ISP_REMOTE		1
#define	FN_SYS_FUNC_SWITCH_BOOT_ROM	2
#define	FN_SYS_FUNC_SET_MAC		3
#define	FN_SYS_FUNC_ISP_T2T		4
#define	FN_SYS_FUNC_SET_TSN		5
#define	FN_SYS_FUNC_GET_VERSION		6
#define	FN_SYS_FUNC_TEST_LAN		7
#define	FN_SYS_FUNC_SET_AT_CMD		8

#define	FN_SYS_FUNC_SET_IEK		80	// set IMEK & IAEK for contactless reader (CL)
#define	FN_SYS_FUNC_SET_TSC_TYPE	87	// set capacitive or registive TSC type
#define	FN_SYS_FUNC_SET_SUB_MODEL	89	// set sub-model ID

#define	FN_SYS_FUNC_KMS			90
#define	FN_SYS_FUNC_TAMPER_STATUS	91
#define	FN_SYS_FUNC_DATA_RETENTION	92	// SRAM data retention test

#define	FN_SYS_FUNC_ERASE_SYSTEM	93	// erase U-boot & Kernel (shall reload FLASH images by using UUU through USB port)

#define	FN_SYS_FUNC_DETECT_TAMPER	97	// turn on or off TAMPER detection
#define	FN_SYS_FUNC_PIN_LIMIT		98
#define	FN_SYS_FUNC_RECOVER_AP		99

#define	FN_DIAG_FUNC_TEST_SRAM		10
#define	FN_DIAG_FUNC_TEST_FLASH		11
#define	FN_DIAG_FUNC_TEST_KBD		12
#define	FN_DIAG_FUNC_TEST_DISP		13
#define	FN_DIAG_FUNC_TEST_MSR		14
#define	FN_DIAG_FUNC_TEST_SCR		15
#define	FN_DIAG_FUNC_TEST_RTC		16
#define	FN_DIAG_FUNC_TEST_AUX		17
#define	FN_DIAG_FUNC_TEST_MDM		18
#define	FN_DIAG_FUNC_TEST_PRT		19
#define	FN_DIAG_FUNC_TEST_CP		20
#define	FN_DIAG_FUNC_TEST_IP		21

#define	FN_DIAG_FUNC_TEST_BL		22	// TBD
#define	FN_DIAG_FUNC_TEST_GPRS		23	//
#define	FN_DIAG_FUNC_TEST_POWER		24

#define	FN_DIAG_FUNC_TEST_USB_DEV	25	// TBD
#define	FN_DIAG_FUNC_TEST_USB_HOST	26	// TBD
#define	FN_DIAG_FUNC_TEST_TSC		27	// 
#define	FN_DIAG_FUNC_TEST_RC663		28	// 
#define	FN_DIAG_FUNC_TEST_KBD_LED	29	// 

#define	FN_DIAG_FUNC_TEST_NAND_FLASH	31	// new for NAND Flash
#define	FN_DIAG_FUNC_TEST_HWRST		33	//

#define	FN_DIAG_FUNC_TEST_SPEAKER	34	// speaker
#define	FN_DIAG_FUNC_TEST_SD_CARD	35	// SD card

#define	FN_DIAG_FUNC_TEST_2D_READER	39	// 2D barcode reader

#define	FN_SYS_FUNC_EVENT_LOG		40	// show system event logs
#define	FN_SYS_FUNC_LOAD_TMK		48	// load or verify TMKs

#define	FN_DIAG_FUNC_MFG_TEST		50	// TBD
#define	FN_DIAG_FUNC_BURN_IN_TEST	51	//

#define	FN_DIAG_FUNC_TEST_LCD_COLOR	60	// TFT LCD color test
#define	FN_DIAG_FUNC_TEST_SIGNPAD	61	// SIGN PAD test


//#define	FLASH_BASE				HW_FLASH_BASE_ADDRESS
//#define	ADDR_APP1				APP1_FLASH_ST_ADDRESS
//#define	ADDR_APP0				KRNL_FLASH_ST_ADDRESS

//#define	FULL_BIG5_CHAR_NUM			13503
//#define	ADDR_FULL_BIG5_CODE_TABLE		0x10D90000	//0x105D0000
//#define	ADDR_FULL_PRT_FONT2_24X24_BMP		0x10DA0000	//0x105E0000
//#define	ADDR_FULL_PRT_FONT4_16X16_BMP		0x10E90000	//0x106D0000
//#define	ADDR_FONT_VERSION			ADDR_FULL_PRT_FONT4_16X16_BMP + (FULL_BIG5_CHAR_NUM+1)*32 - 2

//#define	FLASH_POST0_AND_DATA_SIZE		(128+896)*1024
//#define	FLASH_NO_ERASE_SIZE			(128+896+1472+64)*1024

// ---------------------------------------------------------------------------
//	PEDS definitions (reference to \INC\PEDSAPI.h)
// ---------------------------------------------------------------------------
//#define MAX_PEDS_MKEY_CNT                20                      //
//#define PEDS_MSKEY_LEN                   16                      //
//#define PEDS_MSKEY_SLOT_LEN              17                      // L(1)+V(16)
//#define PEDS_MSKEY_SLOT_LEN2             20                      // L(1)+V(16)+KCV(3)

//UINT8	*os_pSwap;
//UINT32	os_Config_ESX = FALSE;


// ---------------------------------------------------------------------------
#if	0
void	POST_DisplayTestResult( UINT32 result )
{
	if( result == TRUE )
	  {
	  LIB_BUZ_Beep1();
	  LIB_LCD_Puts( 6, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_OK), (UINT8 *)os_msg_OK );
	  }
	else
	  {
	  LIB_BUZ_Beep2();
	  LIB_LCD_Puts( 6, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR );
	  
	  if( os_MultiBootMode == OP_MODE_MFG )
	    {
	    LIB_WaitKey();
	    return;
	    }
	  }
	
	LIB_WaitTimeAndKey( 100 ); // delay 1.0 sec
}
#endif

// ---------------------------------------------------------------------------
#if	0
void	TEST_POWER_SW( void )
{
UCHAR data;
UCHAR tx[4];
UCHAR rx[4];
//int rv,fd=TSC_fd;
struct timeval timeout;
fd_set set;
struct input_event TSCevent[5];
UCHAR *SHM_data=NULL;
UINT	cnt = 0;
UCHAR	value = 0;


	LIB_LCD_Puts( 0, 0, FONT0, sizeof("POWER SW"), (UINT8 *)"POWER SW" );
	
//	TSC_ExtIOInitialize(DEV_OPEN);
//	InitExtIo();
#if	0
	if(TSC_fd<1)
	{
	LIB_DispHexByte( 1, 0, 1 );
	
		bsp_shm_acquire(SHM_data);
		bsp_shm_IrqOccupied(1,&data);
		usleep(1000);//wait a while for waiting daemon close /dev/input/event2
		TSC_fd = open("/dev/input/event2", O_RDWR);
		if(TSC_fd<0)
		  {
		  LIB_DispHexByte( 1, 6, 0xFF );
		  for(;;);
		  }
	}
	LIB_DispHexByte( 1, 3, 2 );
#endif

#if	1
	while(1)
	{
	LIB_DispHexByte( 2, 0, EXTIO_GPIO_Read_U4( _SW_DETECT_BANK_ADDR, _SW_DETECT_GPIO_PORT, _SW_DETECT_GPIO_NUM, &value ) );
//	LIB_DispHexByte( 1, 0, EXTIO_GPIO_Read( _Power_in_BANK_ADDR, _Power_in_GPIO_PORT, _Power_in_GPIO_NUM, &value ) );
//	LIB_DispHexByte( 1, 0, EXTIO_GPIO_Read( _PAPEREND_BANK_ADDR, _PAPEREND_GPIO_PORT, _PAPEREND_GPIO_NUM, &value ) );
	LIB_DispHexByte( 3, 0, value );

//	LIB_DispHexByte( 1, 0, PMU_PowerSourceState() );
	LIB_WaitTime(50);
	LIB_DispHexWord( 4, 0, cnt++ );
        }
#endif

	
#if	0
ST:
	FD_ZERO(&set); /* clear the set */
	FD_SET(fd, &set); /* add our file descriptor to the set */
	timeout.tv_sec = 0;
	timeout.tv_usec = 500;//0.5ms timeout
//	timeout.tv_usec = 0;
	rv = select(fd+1, &set, NULL, NULL, &timeout);
	
	if(FD_ISSET(fd, &set))
	  rv=read(fd,&TSCevent, sizeof(TSCevent));
	  
	if(rv == -1 || rv == 0 )
	{
		//read error or time out
		LIB_DispHexByte( 2, 0, rv );
		LIB_DispHexWord( 3, 0, cnt++ );
		
		//update interrupt pins status
//		tx[0]=0x45;
//		tx[1]=0x08;//INTCAPA (clear interrupt)
//		tx[2]=0x00;
//		SPI_Transfer(tx,rx,0,EXTIO,4);
	}
	else
	{
		tx[0]=0x45;
		tx[1]=0x09;//GPIOA
		tx[2]=0x00;
		SPI_Transfer(tx,rx,0,EXTIO,4);
		
		if((rx[2]&0x80)>0)
		  {
		  data=0;
		  bsp_shm_ButtPressed(1,&data);
		  
		  LIB_DispHexByte( 5, 0, 0x00 );	// not pressed
		  }
		else
		  {
		  data=1;
		  bsp_shm_ButtPressed(1,&data);
		  
		  LIB_DispHexByte( 5, 0, 0x01 );	// pressed
		  }
	}

	goto ST;
#endif

}
#endif

// ---------------------------------------------------------------------------
void	TEST_SD_CARD_OLD( void )
{
ULONG	i;
ULONG	result;
FILE	*linux_file;
UCHAR	path_buffer[] = {"/media/mmcblk0p1/file.txt"};
UCHAR	sbuf[64];
UCHAR	dbuf[64];



	LIB_LCD_Puts( 0, 0, FONT0, sizeof("SD CARD"), (UINT8 *)"SD CARD" );
	LIB_WaitKey();
	
	linux_file = fopen(path_buffer,"a+");	// read & write
	if( linux_file != NULL )
	  {
	  LIB_LCD_Puts( 1, 0, FONT0, sizeof("OPEN: OK"), (UINT8 *)"OPEN: OK" );
	  LIB_WaitKey();
	  
	  for( i=0; i<sizeof(sbuf); i++ )
	     sbuf[i] = i;
	  memset( dbuf, 0x00, sizeof(dbuf) );
	  
//	  result = fwrite( sbuf, sizeof(char), sizeof(sbuf), linux_file );

	  LIB_LCD_Puts( 2, 0, FONT0, sizeof("READ: "), (UINT8 *)"READ: " );
	  result = fread( dbuf, sizeof(char), sizeof(dbuf), linux_file );
	  LIB_DispHexWord( 2, strlen("READ: "), result );
	  
	  if( LIB_memcmp( sbuf, dbuf, sizeof(dbuf) ) == 0 )
	    LIB_LCD_Puts( 2, strlen("READ: ")+6, FONT0, sizeof("OK"), (UINT8 *)"OK" );
	    
	  LIB_WaitKey();
	  
	  LIB_LCD_Puts( 3, 0, FONT0, sizeof("TELL: "), (UINT8 *)"TELL: " );
	  result = api_fs_tell( linux_file );
	  LIB_DispHexWord( 3, strlen("TELL: "), result );
	  
	  LIB_WaitKey();
	  
	  fclose( linux_file );
	  LIB_LCD_Puts( 7, 0, FONT0, sizeof("CLOSE"), (UINT8 *)"CLOSE" );
	  LIB_WaitKey();
	  }
	else
	  {
	  LIB_LCD_Puts( 1, 0, FONT0, sizeof("OPEN: FAIL"), (UINT8 *)"OPEN: FAIL" );
	  for(;;);
	  }
}

// ---------------------------------------------------------------------------
void	TEST_SD_CARD( void )
{
ULONG	i;
ULONG	result;
FILE	*linux_file;
UCHAR	sbuf[64];
UCHAR	dbuf[64];
FILE	*fp[1];



	LIB_LCD_Puts( 0, 0, FONT0, strlen("SD CARD"), (UINT8 *)"SD CARD" );
	LIB_WaitKey();
	
	// select media to be accessed
	LIB_LCD_Puts( 1, 0, FONT0, strlen("SELECT: "), (UINT8 *)"SELECT: " );
	if( api_fs_select( MEDIA_SD ) == apiOK )
	  LIB_LCD_Puts( 1, strlen("SELECT: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  LIB_LCD_Puts( 1, strlen("SELECT: "), FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	
	LIB_WaitKey();
	
	// initailize
	LIB_LCD_Puts( 2, 0, FONT0, strlen("INIT: "), (UINT8 *)"INIT: " );
	if( api_fs_init() == apiOK )
	  LIB_LCD_Puts( 2, strlen("INIT: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  LIB_LCD_Puts( 2, strlen("INIT: "), FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	  
	LIB_WaitKey();
	
	// create a new file
	LIB_LCD_Puts( 3, 0, FONT0, strlen("CREATE: "), (UINT8 *)"CREATE: " );
	if( api_fs_create( "file1.dat", 0 ) == apiOK )
	  LIB_LCD_Puts( 3, strlen("CREATE: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  LIB_LCD_Puts( 3, strlen("CREATE: "), FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	
	LIB_WaitKey();
	
	// open an existent file
	LIB_LCD_Puts( 4, 0, FONT0, strlen("OPEN: "), (UINT8 *)"OPEN: " );
	
#if	1	
	linux_file = api_fs_open( "file1.dat", 0 );
#else
	if( api_fs_open_EX( strlen("file1.dat"), "file1.dat", fp ) == apiOK )
	  {
//	  LIB_DispHexByte( 4, 10, 0x33 );
//	  LIB_WaitKey();
	  
	  linux_file = *fp;
	  printf("linux_file=%p\n", linux_file);
	  
//	  LIB_DispHexByte( 4, 10, 0x44 );
//	  LIB_WaitKey();
	  }
#endif

	if( linux_file != NULL )
	  LIB_LCD_Puts( 4, strlen("OPEN: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  LIB_LCD_Puts( 4, strlen("OPEN: "), FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	
	LIB_WaitKey();
	
	for( i=0; i<sizeof(sbuf); i++ )
	   sbuf[i] = i;
	
	// write data to the file
	LIB_LCD_Puts( 5, 0, FONT0, strlen("WRITE: "), (UINT8 *)"WRITE: " );
	result = api_fs_write( linux_file, sbuf, sizeof(sbuf) );
	LIB_DispHexWord( 5, strlen("WRITE: "), result );
	if( result == sizeof(sbuf) )
	  LIB_LCD_Puts( 5, strlen("WRITE: ")+6, FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  LIB_LCD_Puts( 5, strlen("WRITE: ")+6, FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	  
	LIB_WaitKey();
	
	// close file
	LIB_LCD_Puts( 6, 0, FONT0, strlen("CLOSE: "), (UINT8 *)"CLOSE: " );
	api_fs_close( linux_file );
	LIB_LCD_Puts( 6, strlen("CLOSE: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	
	LIB_WaitKey();
	
	// open an existent file
	LIB_LCD_Puts( 7, 0, FONT0, strlen("OPEN: "), (UINT8 *)"OPEN: " );
	linux_file = api_fs_open( "file1.dat", 0 );
	if( linux_file != NULL )
	  LIB_LCD_Puts( 7, strlen("OPEN: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  LIB_LCD_Puts( 7, strlen("OPEN: "), FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	
	LIB_WaitKey();
	
	// read and compare contents of the file
	LIB_LCD_Puts( 8, 0, FONT0, strlen("READ: "), (UINT8 *)"READ: " );
	memset( dbuf, 0x00, sizeof(dbuf) );
	result = api_fs_read( linux_file, dbuf, sizeof(dbuf) );
	LIB_DispHexWord( 8, strlen("READ: "), result );
	
	if( LIB_memcmp( sbuf, dbuf, sizeof(dbuf) ) == 0 )
	  LIB_LCD_Puts( 8, strlen("READ: ")+6, FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  LIB_LCD_Puts( 8, strlen("READ: ")+6, FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	
	LIB_WaitKey();
	
	// TELL
	LIB_LCD_Puts( 9, 0, FONT0, sizeof("TELL: "), (UINT8 *)"TELL: " );
	result = api_fs_tell( linux_file );
	LIB_DispHexWord( 9, strlen("TELL: "), result );
	  
	LIB_WaitKey();
	
	// SEEK
	LIB_LCD_Puts( 10, 0, FONT0, sizeof("SEEK: "), (UINT8 *)"SEEK: " );
	result = api_fs_seek( linux_file, 10 );
	if( result == apiOK )
	  LIB_LCD_Puts( 10, strlen("SEEK: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  LIB_LCD_Puts( 10, strlen("SEEK: "), FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	
	// READ
	LIB_LCD_Puts( 11, 0, FONT0, strlen("READ: "), (UINT8 *)"READ: " );
	memset( dbuf, 0x00, sizeof(dbuf) );
	result = api_fs_read( linux_file, dbuf, 8 );
	LIB_DispHexWord( 11, strlen("READ: "), result );
	LIB_DispHexByte( 12, 0, dbuf[0] );
	LIB_DispHexByte( 12, 3, dbuf[1] );
	LIB_DispHexByte( 12, 6, dbuf[2] );
	LIB_DispHexByte( 12, 9, dbuf[3] );
	LIB_DispHexByte( 13, 0, dbuf[4] );
	LIB_DispHexByte( 13, 3, dbuf[5] );
	LIB_DispHexByte( 13, 6, dbuf[6] );
	LIB_DispHexByte( 13, 9, dbuf[7] );
	  
	LIB_WaitKey();
	
	// close file
	LIB_LCD_Puts( 14, 0, FONT0, strlen("CLOSE: "), (UINT8 *)"CLOSE: " );
	api_fs_close( linux_file );
	LIB_LCD_Puts( 14, strlen("CLOSE: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	  
	for(;;);
}

// ---------------------------------------------------------------------------
void	TEST_COM1( void )
{
ULONG	i;
ULONG	result;
FILE	*linux_file;
UCHAR	sbuf[2+128];
UCHAR	dbuf[2+128];
UCHAR	data0, data;
FILE	*fp[1];
UINT	cnt;



	LIB_LCD_Puts( 0, 0, FONT0, strlen("TEST COM1"), (UINT8 *)"TEST COM1" );
	LIB_WaitKey();
	
	LIB_LCD_Puts( 1, 0, FONT0, strlen("OPEN: "), (UINT8 *)"OPEN: " );
	if( LIB_OpenAUX( COM1, auxBYPASS, COM_115200 ) )
	  LIB_LCD_Puts( 1, strlen("OPEN: "), FONT0, strlen("OK"), (UINT8 *)"OK" );
	else
	  LIB_LCD_Puts( 1, strlen("OPEN: "), FONT0, strlen("FAIL"), (UINT8 *)"FAIL" );
	  
	LIB_WaitKey();
	
	
	data0 = 0x20;
	
	dbuf[0] = 2;
	dbuf[1] = 0;
	dbuf[2] = 0x0d;
	dbuf[3] = 0x0a;
	
NEXT:
	cnt = 6;
	
	LIB_LCD_Puts( 2, 0, FONT0, strlen("HIT ANY KEY TO SEND"), (UINT8 *)"HIT ANY KEY TO SEND" );
	LIB_WaitKey();
	
	LIB_LCD_Puts( 3, 0, FONT0, strlen("SEND: "), (UINT8 *)"SEND: " );

NEXT2:
	if( data0 >= 0x7f )
	  data0 = 0x20;
	data = data0++;
	for( i=0; i<sizeof(sbuf)-2; i++ )
	   {
	   sbuf[2+i] = data++;
	   if( data >= 0x7f )
	     data = 0x21;
	   }
	
	sbuf[2+64+0] = 0x0d;
	sbuf[2+64+1] = 0x0a;
	
	sbuf[0] = 64+2;
	sbuf[1] = 0;
	LIB_TransmitAUX( sbuf );
	
	if( --cnt )
	  goto NEXT2;
	
	LIB_TransmitAUX( dbuf );	// CR+LF
	
	LIB_LCD_Puts( 3, strlen("SEND: "), FONT0, strlen("OK"), (UINT8 *)"OK" );
	  
	LIB_WaitKey();
	
	LIB_LCD_ClearRow( 2, 2, FONT0 );
	goto NEXT;
}

// ---------------------------------------------------------------------------
// FUNCTION: Diagnostic test.
// INPUT   : FuncNo	- item number to be tested. (0x00..0xff)
// OUTPUT  : none.
// RETURN  : TRUE / FALSE
// ---------------------------------------------------------------------------
#if	1
UINT32	POST_DiagnosticTest( UINT32 FuncNo )
{
UINT32	i;
//UINT32	FuncNo;
UINT32	Result;
//UINT8	buf[16];
//UINT8	len;

//UINT32	NoEraseSize = FLASH_NO_ERASE_SIZE;
//UINT32	StartAddr = ADDR_APP1;


//	POST_FLASH_PageInit_NOR();
	
//	POST_EnableDHCP();
	
//	os_HpCopFlag = 0;
	
//	while(1)
//	{
#if	0
	POST_DisplayVersionNo();

	LIB_OpenKeyAll();
//	while( LIB_WaitKey() != KEY_FUNC );	// only FUNC key accepted (AS320)
	while( LIB_WaitKey() != KEY_ENTER );	// only FUNC key accepted (AS350)
	
	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_FUNCTION), (UINT8 *)os_msg_FUNCTION );
		
//	i = LIB_GetNumKey( 0, NUM_TYPE_DIGIT, '_', FONT0, 1, 2, buf );
//	LIB_OpenKeyAll();
//	if( i == FALSE )
//	  return;

	if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT, '_', FONT0, 1, 2, buf ) == FALSE )
	  continue;
	
	len = buf[0];
	buf[len+1] = 0x00;
	FuncNo = atoi( (UINT8 *)&buf[1] );
	
	LIB_LCD_Cls();
#endif

	Result = FALSE;
	switch( FuncNo )
	      {
	      case FN_SYS_FUNC_ISP_LOCAL:	// 00
	      	
//		TEST_COM1();
//	      	TEST_SD_CARD();
//	      	Result = TRUE;
//	      	TEST_POWER_SW();
//	      	BSP_IO_Control(5,0);	// POWER-OFF BATT
//	      	for(;;);

//	      	   Result = SYS_FUNC_IspLocal();
	      	   break;
	      	   
	      case FN_SYS_FUNC_ISP_REMOTE:	// 01
	      	
//	      	   Result = SYS_FUNC_IspRemote();
		   Result = api_dss_interface();
	      	   break;
	      	   
	      case FN_SYS_FUNC_SWITCH_BOOT_ROM:	// 02

#if	0
//#ifndef	_BOOT_FROM_POST0_
//	      	   Result = SYS_FUNC_SwitchBootROM();
//#endif
		// === for DEBUG TEST ONLY (all defined in IPC_server.c) ===
		if( LIB_WaitKey() == '1' )
		  {
		  TEST_TSC();
//		  TEST_SIGNPAD();
		  }
		else
//		  TEST_PRINTER();
		  TEST_SIGNPAD();
		  
//		TEST_LCD();
//		TEST_TSC();
//		TEST_SIGNPAD();
//		TEST_TIMER();
//		TEST_TIMER2();
//		TEST_PRINTER();
//		TEST_SYSAPI();
//		TEST_DPA();	// function test OK, only trigger IO is to be verified!

		   Result = TRUE;
#endif
	      	   break;
	      	   
	      case FN_SYS_FUNC_SET_MAC:		// 03
	      	
	      	   Result = SYS_FUNC_SetMAC();
	      	   break;
	      	   
	      case FN_SYS_FUNC_ISP_T2T:		// 04
	      	
//	      	   Result = SYS_FUNC_IspT2T();
	      	   break;

	      case FN_SYS_FUNC_SET_TSN:		// 05
	      	
//	      	   Result = SYS_FUNC_SetTSN();
	      	   break;
      	   
	      case FN_SYS_FUNC_GET_VERSION:	// 06
	      	
//	      	   Result = SYS_FUNC_GetVersion();
	      	   break;
	      	   
	      case FN_SYS_FUNC_TEST_LAN:	// 07
	      	
////	      	   Result = SYS_FUNC_TestLan();
	      	   break;
	      	   
	      case FN_SYS_FUNC_SET_AT_CMD:	// 08
	      	
//	      	   Result = SYS_FUNC_SetAtCmd();
	      	   break;
	      
	      case FN_DIAG_FUNC_TEST_SRAM:	// 10
	      	
//	      	   Result = DIAG_FUNC_TestSram();
	      	   break;
	      	   
	      case FN_DIAG_FUNC_TEST_FLASH:	// 11
	      	
//	      	   Result = DIAG_FUNC_TestFlash();
	      	   break;
	      	   
	      case FN_DIAG_FUNC_TEST_KBD:	// 12
	      	
	      	   Result = DIAG_FUNC_TestPinPad();
	      	   break;
	      	   
	      case FN_DIAG_FUNC_TEST_DISP:	// 13
	      	
	           Result = DIAG_FUNC_TestDisplay();
	           break;
	           
	      case FN_DIAG_FUNC_TEST_MSR:	// 14
	      	
	      	   Result = DIAG_FUNC_TestMsr();
	      	   break;
	      	   
	      case FN_DIAG_FUNC_TEST_SCR:	// 15
	      	
	      	   Result = DIAG_FUNC_TestScr();
	      	   break;
	      	   
	      case FN_DIAG_FUNC_TEST_RTC:	// 16
	      	
	      	   Result = DIAG_FUNC_TestRtc();
	      	   break;

	      case FN_DIAG_FUNC_TEST_AUX:	// 17
	      	
	      	   Result = DIAG_FUNC_TestAux( 0, &i );
	      	   break;
	      	   
	      case FN_DIAG_FUNC_TEST_MDM:	// 18
	      	
//	      	   Result = DIAG_FUNC_TestModem();
	      	   break;
	      	   
	      case FN_DIAG_FUNC_TEST_PRT:	// 19
	      	
	      	   Result = DIAG_FUNC_TestPrinter();
	      	   break;

	      case FN_DIAG_FUNC_TEST_CP:	// 20
	      	
//	      	   Result = DIAG_FUNC_TestCP();
	      	   break;

	      case FN_DIAG_FUNC_TEST_IP:	// 21
	      	
	      	   Result = DIAG_FUNC_TestIP();
//		   Result = DSS_function_LANset();
	      	   break;

	      case FN_DIAG_FUNC_TEST_BL:	// 22
	      	
//	      	   Result = DIAG_FUNC_TestBackLight();
	      	   break;

	      case FN_DIAG_FUNC_TEST_GPRS:	// 23
	      
//	      	   Result = DIAG_FUNC_TestGPRS();
	      	   break;

	      case FN_DIAG_FUNC_TEST_POWER:	// 24

	      	   Result = DIAG_FUNC_TestBattPower();
	      	   break;
	      	   
	      case FN_DIAG_FUNC_TEST_USB_DEV:	// 25
	      	
//	      	   Result = DIAG_FUNC_TestUSB_DEV();
	      	   break;

	      case FN_DIAG_FUNC_TEST_USB_HOST:	// 26
	      	
//	      	   Result = DIAG_FUNC_TestUSB_HOST();
	      	   break;
	      	   
	      case FN_DIAG_FUNC_TEST_TSC:	// 27
	      	
	      	   Result = DIAG_FUNC_TestTouchScreen();
	      	   break;
	      	   
	      case FN_DIAG_FUNC_TEST_RC663:	// 28
	      	
	      	   Result = DIAG_FUNC_TestRC663();
	      	   break;

	      case FN_DIAG_FUNC_TEST_KBD_LED:	// 29
	      	
	      	   Result = DIAG_FUNC_TestKbdLED();
	      	   break;

	      case FN_DIAG_FUNC_TEST_NAND_FLASH: // 31
	      	
//	      	   Result = DIAG_FUNC_TestNandFlash();
	      	   break;

	      case FN_DIAG_FUNC_TEST_HWRST:	// 33
	      	
//	      	   DIAG_FUNC_TestHardwareReset();
	      	   break;
	      	   
	      case FN_DIAG_FUNC_TEST_SPEAKER:	// 34
	      	
	      	   Result = DIAG_FUNC_TestSpeaker();
	      	   break;
	      	   
	      case FN_DIAG_FUNC_TEST_SD_CARD:	// 35
	      	
	      	   Result = DIAG_FUNC_TestSDCard();
	      	   break;

	      case FN_DIAG_FUNC_TEST_2D_READER:	// 39
	      	
	      	   Result = DIAG_FUNC_Test2DReader();
	      	   
	      case FN_SYS_FUNC_EVENT_LOG:	// 40 (show system event logs)
	      	
//	      	   Result = SYS_FUNC_EventLog();
	      	   break;

	      case FN_SYS_FUNC_LOAD_TMK:	// 48
	      	
//		   Result = SYS_FUNC_LoadTMK();
		   break;
		   
	      case FN_DIAG_FUNC_MFG_TEST:	// 50
	      	
//	      	   Result = DIAG_FUNC_TestMfg();
	      	   break;

	      case FN_DIAG_FUNC_BURN_IN_TEST:	// 51
	      	
//	      	   Result = POST_BurnInTest();
	      	   break;

	      case FN_DIAG_FUNC_TEST_LCD_COLOR:	// 60
	      	
//	      	   Result = DIAG_FUNC_TestLcdColor();
	      	   break;
	      	   
	      case FN_DIAG_FUNC_TEST_SIGNPAD:	// 61
	      	
//	      	   Result = DIAG_FUNC_TestSignPad();
	      	   break;
	      	   
	      case FN_SYS_FUNC_SET_IEK:		// 80 (IMEK & IAEK for CL)
	      
//#ifndef	_BOOT_FROM_POST0_
//	      	   Result = SYS_FUNC_SetIEK();
//#endif
	      	   break;
	      
	      case FN_SYS_FUNC_SET_TSC_TYPE:	// 87 set TSC type
	      	   
	      	   Result = SYS_FUNC_SetTscType();
	      	   break;
	      
	      case FN_SYS_FUNC_SET_SUB_MODEL:	// 89 setup sub-model id
	      	
//	      	   Result = SYS_FUNC_SetSubModelID();
	      	   break;
   	   
	      case FN_SYS_FUNC_KMS:		// 90 (Key Management System)
	      	
////	      	   Result = SYS_FUNC_KeyManagementSystem();
	      	   break;

	      case FN_SYS_FUNC_TAMPER_STATUS:	// 91 (Tamper Status)
	      	
//	      	   Result = SYS_FUNC_TamperStatus();
	      	   break;

	      case FN_SYS_FUNC_DATA_RETENTION:	// 92 (SRAM Data Retention)
	      	
//	           Result = POST_DataRetentionTest2();
	           break;

	      case FN_SYS_FUNC_ERASE_SYSTEM:	// 93 (system maintenance functions)
	      	
	      	   Result = SYS_FUNC_SysMaintenance();
	      	   break;

	      case FN_SYS_FUNC_DETECT_TAMPER:	// 97 (turn on or off TAMPER detection)
	      	
//	      	   Result = SYS_FUNC_DetectTamper();
	      	   break;

	      case FN_SYS_FUNC_PIN_LIMIT:	// 98: PIN entry frequency limitation

//#ifndef	_BOOT_FROM_POST0_
//	           Result = SYS_FUNC_PinLimit();
//#endif
	           break;

	      case FN_SYS_FUNC_RECOVER_AP:
//	      	   Result = SYS_FUNC_RecoverAP();
	      	   break;
	      }	// swtich( FuncNo )

//	POST_DisplayTestResult( Result );
//	}

	return( Result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Update COP firmware.
// INPUT   : pAddr - pointer to COP code. (DSS packet format)
// OUTPUT  : none.
// RETURN  : TRUE/FALSE
// ---------------------------------------------------------------------------
#if	0
inline	static	UINT32	POST_COP_IapProcess( UINT8 *pAddr )
{
UINT32	i;
UINT32	result = FALSE;
UINT32	max_length = 512*1024;
UINT32	tot_length = 0;
UINT8	buffer[2+1024];
UINT16	iLen;
UINT16	iLeftLen;
UINT8	status;


	if( !pAddr )
	  return( FALSE );
	
	// verify M3 code
	// offset	0x0003		0x10
	//		0x0007		0x00
	//		0x02FC~0x2FF	0xFF,0xFF,0xFF,0xFF (no CRP)
	if( (*(pAddr+8+0x0003) != 0x10) || (*(pAddr+8+0x0007) != 0x00) ||
	    (*(pAddr+8+0x02FC) != 0xFF) || (*(pAddr+8+0x02FD) != 0xFF) || (*(pAddr+8+0x02FE) != 0xFF) || (*(pAddr+8+0x02FF) != 0xFF) )
	  {
	  return( FALSE );
	  }
	
	// write packets to M3 FLASH memory
	// SOH(1) LEN(2) CMD(1) OFFSET(4) Data(n) LRC(1)
//	LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING );
//	LIB_DispHexByte( 2, 14, 0x00 );
	
//	LIB_WaitKey();
	
	// signal M3 to switch to the 2'nd bootloader
	api_iap_switch();
	
	BSP_Delay_n_ms(3000);
//	LIB_WaitKey();
	
	LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING );
	LIB_DispHexByte( 2, 14, 0x01 );
	
	tot_length = 0;
	while(1)
	     {
	     if( tot_length > max_length )
	       goto EXIT;
	       
	     if( (pAddr[0] == 0x01) && ((pAddr[3] == 0x01) || (pAddr[3] == 0x02) || (pAddr[3] == 0x03)) )
	       {	         
	       iLen = pAddr[1] + pAddr[2]*256;
	       iLen -= 5;	// exclude CMD(1) + OFFSET(4)
	       iLeftLen = 0;
	       
	       if( iLen > 1024 )
	         {
	         iLeftLen = iLen - 1024;
	         iLen = 1024;

	         // send 1'st part
	         buffer[0] = iLen & 0x00FF;
	         buffer[1] = (iLen & 0xFF00) >> 8;
	         pAddr += 8;	// ptr data
	         memmove( &buffer[2], pAddr, iLen );
	         
	         tot_length += iLen;
	         if( api_iap_txstring( buffer ) != apiOK )
	           break;
	         pAddr += iLen;
	         }
	       else
	         {
//	         if( pAddr[3] == 3 )
//	           {
//	           LIB_DispHexWord( 0, 0, iLen );
//	           LIB_DumpHexData( 0, 1, iLen, pAddr );
//	           }
	           	         
	         pAddr += 8;	// ptr data
	         iLeftLen = iLen;
	         }
	       
	       // send 2'nd part
	       if( iLeftLen )
	         {
	         buffer[0] = iLeftLen & 0x00FF;
	         buffer[1] = (iLeftLen & 0xFF00) >> 8;
	         memmove( &buffer[2], pAddr, iLeftLen );
	       
	         tot_length += iLeftLen;	         
	         if( api_iap_txstring( buffer ) != apiOK )
	           break;
	         pAddr += (iLeftLen+1);		// data + lrc
	         }
	       else
	         pAddr += 1;	// lrc
	       }
	     else
	       {
	       if( (pAddr[0] == 0x01) && (pAddr[3] == 0x04) )	// complete
	         {
	         for( i=0; i<16; i++ )
	            {
	            if( pAddr[4+i] != 0xF0 )
	              goto EXIT;
	            }
	              
		 result = TRUE;
		 break;
		 }
	       else
	         break;		// invalid packet
	       }
	     } // while(1)

	// start to burn
	if( result )
	  {
	  LIB_DispHexByte( 2, 14, 0x02 );
			  
	  status = 0;
	  result = api_iap_burn( &status );
//	  LIB_DispHexByte( 5, 0, status );
	  if( result == apiOK )
	    {
	    status = 0;
	    api_iap_status( &status );
	    }
	  
	  LIB_DispHexByte( 2, 17, status );
	  if( status == 0x07 )
	    result = TRUE;
	  else
	    result = FALSE;
	  }

	// signal M3 to switch to the 1'st bootloader
	BSP_Delay_n_ms(1000);
	
	api_iap_switch();
	
	BSP_Delay_n_ms(3000);
	
EXIT:	     
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Update COP firmware.
// INPUT   : pAddr - pointer to COP code. (COP code format)
// OUTPUT  : none.
// RETURN  : TRUE/FALSE
// ---------------------------------------------------------------------------
#if	0
inline	static	UINT32	POST_COP_IapProcess2( UINT32 length, UINT8 *pAddr )
{
UINT32	i;
UINT32	result = FALSE;
UINT32	max_length = 512*1024;
UINT32	tot_length = 0;
UINT8	buffer[2+1024];
UINT16	iLen;
UINT16	iLeftLen;
UINT8	status;

UINT32	cnt = 0;
UINT32	left_bytes = 0;
UINT32	PayloadSize = 1024;


	if( !pAddr )
	  return( FALSE );
	
	// verify M3 code
	// offset	0x0003		0x10
	//		0x0007		0x00
	//		0x02FC~0x2FF	0xFF,0xFF,0xFF,0xFF (no CRP)
	if( (*(pAddr+0+0x0003) != 0x10) || (*(pAddr+0+0x0007) != 0x00) ||
	    (*(pAddr+0+0x02FC) != 0xFF) || (*(pAddr+0+0x02FD) != 0xFF) || (*(pAddr+0+0x02FE) != 0xFF) || (*(pAddr+0+0x02FF) != 0xFF) )
	  {
	  return( FALSE );
	  }
	
	// write packets to M3 FLASH memory
	// SOH(1) LEN(2) CMD(1) OFFSET(4) Data(n) LRC(1)
	
	// signal M3 to switch to the 2'nd bootloader
	api_iap_switch();
	
	BSP_Delay_n_ms(3000);
	
	LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING );
	LIB_DispHexByte( 2, 14, 0x01 );
	
	
	tot_length = length;	// size of COP
        cnt = tot_length / PayloadSize;		// max payload length in one packet, eg, 1024 bytes
        left_bytes = tot_length % PayloadSize;

        buffer[0] = PayloadSize & 0x00FF;
        buffer[1] = (PayloadSize & 0xFF00) >> 8;
        if( cnt )
          {
          // request to send full size packet (1024 bytes)
          for( i=0; i<cnt; i++ )
             {
             memmove( &buffer[2], pAddr, PayloadSize );
             if( api_iap_txstring( buffer ) != apiOK )
               goto EXIT;
               
             pAddr += PayloadSize;
             }
          }

	if( left_bytes )
	  {
          buffer[0] = left_bytes & 0x00FF;
          buffer[1] = (left_bytes & 0xFF00) >> 8;
        
	  memmove( &buffer[2], pAddr, left_bytes );
	  if( api_iap_txstring( buffer ) != apiOK )
	    goto EXIT;
	  }


	result = TRUE;
		
	// start to burn
	if( result )
	  {
	  LIB_DispHexByte( 2, 14, 0x02 );
			  
	  status = 0;
	  result = api_iap_burn( &status );
	  if( result == apiOK )
	    {
	    status = 0;
	    api_iap_status( &status );
	    }
	  
	  LIB_DispHexByte( 2, 17, status );
	  if( status == 0x07 )
	    result = TRUE;
	  else
	    result = FALSE;
	  }

	// signal M3 to switch to the 1'st bootloader
	BSP_Delay_n_ms(1000);
	
	api_iap_switch();
	
	BSP_Delay_n_ms(3000);
	
EXIT:	     
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 00 - Local In-Application-Programming for M3.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE/FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	SYS_FUNC_IspLocal( void )
{
UINT8	msg_M3_IAP[] = {"DSS.COP"};
UINT8	*pAddr;
UINT32	result = FALSE;

#if	0
UINT32	i;
UINT32	result = FALSE;
UINT8	msg_M3_IAP[] = {"DSS.COP"};
UINT32	max_length = 512*1024;
UINT32	tot_length = 0;
UINT8	*pAddr;
UINT8	buffer[2+1024];
UINT16	iLen;
UINT16	iLeftLen;
UINT8	status;
#endif


//	os_HpCopFlag = 0;
	
	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_M3_IAP), (UINT8 *)msg_M3_IAP );
	
//	IspRemote_GetPassword( psw );
//	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, &psw[1] ) != TRUE )
//	  return( FALSE );

	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "1788" ) != TRUE )
	  return( FALSE );
	
	// download firmware via DSS channel  
	if( api_dss_interface2() != apiOK )
	  return( FALSE );

	// retrieve beginning address of packets
	pAddr = api_dss_getaddr();
	
	result = POST_COP_IapProcess( pAddr );
	
#if	0
	if( !pAddr )
	  return( FALSE );
	
	// verify M3 code
	// offset	0x0003		0x10
	//		0x0007		0x00
	//		0x02FC~0x2FF	0xFF,0xFF,0xFF,0xFF (no CRP)
	if( (*(pAddr+8+0x0003) != 0x10) || (*(pAddr+8+0x0007) != 0x00) ||
	    (*(pAddr+8+0x02FC) != 0xFF) || (*(pAddr+8+0x02FD) != 0xFF) || (*(pAddr+8+0x02FE) != 0xFF) || (*(pAddr+8+0x02FF) != 0xFF) )
	  {
	  return( FALSE );
	  }
	
	// write packets to M3 FLASH memory
	// SOH(1) LEN(2) CMD(1) OFFSET(4) Data(n) LRC(1)
//	LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING );
//	LIB_DispHexByte( 2, 14, 0x00 );
	
//	LIB_WaitKey();
	
	// signal M3 to switch to the 2'nd bootloader
	api_iap_switch();
	
	BSP_Delay_n_ms(3000);
//	LIB_WaitKey();
	
	LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING );
	LIB_DispHexByte( 2, 14, 0x01 );
	
	tot_length = 0;
	while(1)
	     {
	     if( tot_length > max_length )
	       goto EXIT;
	       
	     if( (pAddr[0] == 0x01) && ((pAddr[3] == 0x01) || (pAddr[3] == 0x02) || (pAddr[3] == 0x03)) )
	       {	         
	       iLen = pAddr[1] + pAddr[2]*256;
	       iLen -= 5;	// exclude CMD(1) + OFFSET(4)
	       iLeftLen = 0;
	       
	       if( iLen > 1024 )
	         {
	         iLeftLen = iLen - 1024;
	         iLen = 1024;

	         // send 1'st part
	         buffer[0] = iLen & 0x00FF;
	         buffer[1] = (iLen & 0xFF00) >> 8;
	         pAddr += 8;	// ptr data
	         memmove( &buffer[2], pAddr, iLen );
	         
	         tot_length += iLen;
	         if( api_iap_txstring( buffer ) != apiOK )
	           break;
	         pAddr += iLen;
	         }
	       else
	         {
//	         if( pAddr[3] == 3 )
//	           {
//	           LIB_DispHexWord( 0, 0, iLen );
//	           LIB_DumpHexData( 0, 1, iLen, pAddr );
//	           }
	           	         
	         pAddr += 8;	// ptr data
	         iLeftLen = iLen;
	         }
	       
	       // send 2'nd part
	       if( iLeftLen )
	         {
	         buffer[0] = iLeftLen & 0x00FF;
	         buffer[1] = (iLeftLen & 0xFF00) >> 8;
	         memmove( &buffer[2], pAddr, iLeftLen );
	       
	         tot_length += iLeftLen;	         
	         if( api_iap_txstring( buffer ) != apiOK )
	           break;
	         pAddr += (iLeftLen+1);		// data + lrc
	         }
	       else
	         pAddr += 1;	// lrc
	       }
	     else
	       {
	       if( (pAddr[0] == 0x01) && (pAddr[3] == 0x04) )	// complete
	         {
	         for( i=0; i<16; i++ )
	            {
	            if( pAddr[4+i] != 0xF0 )
	              goto EXIT;
	            }
	              
		 result = TRUE;
		 break;
		 }
	       else
	         break;		// invalid packet
	       }
	     } // while(1)

	// start to burn
	if( result )
	  {
	  LIB_DispHexByte( 2, 14, 0x02 );
			  
	  status = 0;
	  result = api_iap_burn( &status );
//	  LIB_DispHexByte( 5, 0, status );
	  if( result == apiOK )
	    {
	    status = 0;
	    api_iap_status( &status );
	    }
	  
	  LIB_DispHexByte( 2, 17, status );
	  if( status == 0x07 )
	    result = TRUE;
	  else
	    result = FALSE;
	  }

	// signal M3 to switch to the 1'st bootloader
	BSP_Delay_n_ms(1000);
	
	api_iap_switch();
	
	BSP_Delay_n_ms(3000);

#endif

EXIT:
	return( result );	
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Update COP firmware.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - updated OK or failed
//           FALSE - no update needed
// ---------------------------------------------------------------------------
#if	0
UINT32	POST_UpdateCOP( void )
{
UINT8	msg_SYSTEM_UPDATE[] = {"SYSTEM UPDATE..."};
UINT8	msg_SYSTEM_UPDATE_OK[]    = {"SYSTEM UPDATE...OK"};
UINT8	msg_SYSTEM_UPDATE_ERROR[] = {"SYSTEM UPDATE...ERROR"};
UINT32	result = FALSE;
UINT8	buffer1[16];
UINT8	buffer2[16];
UINT8	*pAddr;
UINT32	length;


//	os_HpCopFlag = 0;

	length = api_iap_SizeOfCOP();
	if( length == 0 )
	  return( FALSE );
	  
	api_iap_info( buffer1 );		// current COP version
	api_iap_VersionOfCOP( buffer2 );	// new COP version
	
	if( LIB_memcmp( buffer1, buffer2, 5 ) == 0 )
	  return( FALSE );

	BSP_Delay_n_ms( 1000 );
	LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_SYSTEM_UPDATE), (UINT8 *)msg_SYSTEM_UPDATE );
	BSP_Delay_n_ms( 1000 );
	
	// copy COP code to working memory
	pAddr = api_dss_getaddr();
	memmove( pAddr, api_iap_AddressOfCOP(), length );
	
	result = POST_COP_IapProcess2( length, pAddr );
	if( result )
	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_SYSTEM_UPDATE_OK), (UINT8 *)msg_SYSTEM_UPDATE_OK );
	else
	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_SYSTEM_UPDATE_ERROR), (UINT8 *)msg_SYSTEM_UPDATE_ERROR );
	  
	BSP_Delay_n_ms( 1000+2000 );	// 2016-10-28, for COP WDT-reset
	
	LIB_ResetSystem();
	
//	return( result );
}
#endif

// ---------------------------------------------------------------------------
#if	0
void	DIAG_SRAM_Erase( void )
{
UINT8	xDssTelNum[23+1]={0};
UINT8	xDssRemoteIP[23+1]={0};
UINT8	xDssRemotePort[23+1]={0};
UINT8	xDssPort[23+1]={0};

UINT32	SRAM_BaseAddr;
UINT32	SRAM_MemorySize;


	OS_SRAM_GetConfig( &SRAM_BaseAddr, &SRAM_MemorySize );
	
	// backup DSS parameters
	memmove( xDssTelNum, DssTelNum, sizeof(xDssTelNum) );
	memmove( xDssRemoteIP, DssRemoteIP, sizeof(xDssRemoteIP) );
	memmove( xDssRemotePort, DssRemotePort, sizeof(xDssRemotePort) );
	memmove( xDssPort, DssPort, sizeof(xDssPort) );
	
	// this is a temp solution to clear all of SRAM data, including all global data
	memset( (UINT8 *)SRAM_BaseAddr, 0x00, SRAM_MemorySize );

	// restore DSS parameters
	memmove( DssTelNum, xDssTelNum, sizeof(xDssTelNum) );
	memmove( DssRemoteIP, xDssRemoteIP, sizeof(xDssRemoteIP) );
	memmove( DssRemotePort, xDssRemotePort, sizeof(xDssRemotePort) );
	memmove( DssPort, xDssPort, sizeof(xDssPort) );
}
#endif

// ---------------------------------------------------------------------------
#if	0
void	DIAG_SRAM_Erase2( void )
{
UINT8	xDssTelNum[23+1]={0};
UINT8	xDssRemoteIP[23+1]={0};
UINT8	xDssRemotePort[23+1]={0};
UINT8	xDssPort[23+1]={0};

UINT32	SRAM_BaseAddr;
UINT32	SRAM_MemorySize;


	OS_SRAM_GetConfig( &SRAM_BaseAddr, &SRAM_MemorySize );
	
	// backup DSS parameters
	memmove( xDssTelNum, DssTelNum, sizeof(xDssTelNum) );
	memmove( xDssRemoteIP, DssRemoteIP, sizeof(xDssRemoteIP) );
	memmove( xDssRemotePort, DssRemotePort, sizeof(xDssRemotePort) );
	memmove( xDssPort, DssPort, sizeof(xDssPort) );
	
	// this is a temp solution to clear all of SRAM data, including all global data
	SRAM_MemorySize = 192*1024;	// first 192KB of SRAM
	memset( (UINT8 *)SRAM_BaseAddr, 0x00, SRAM_MemorySize );

	// restore DSS parameters
	memmove( DssTelNum, xDssTelNum, sizeof(xDssTelNum) );
	memmove( DssRemoteIP, xDssRemoteIP, sizeof(xDssRemoteIP) );
	memmove( DssRemotePort, xDssRemotePort, sizeof(xDssRemotePort) );
	memmove( DssPort, xDssPort, sizeof(xDssPort) );
}
#endif

// ---------------------------------------------------------------------------
#if	0
UINT8	IspRemote_ChangePassword( void )
{
UINT8	i;
UINT8	len = 12;
UINT8	buf1[16] = {0};
UINT8	buf2[16] = {0};


	LIB_LCD_PutMsg( 1, COL_LEFTMOST, FONT0+attrCLEARWRITE, sizeof(os_msg_NEW_PASSWORD), (UINT8 *)os_msg_NEW_PASSWORD );
	
	if( LIB_GetNumKey( 0, NUM_TYPE_STAR+NUM_TYPE_LEADING_ZERO+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 2, len, buf1 ) == TRUE )
	  {
	  if( buf1[0] < 4 )
	    return( FALSE );
	  }
	else
	  return( FALSE );
	
	
	LIB_LCD_PutMsg( 1, COL_LEFTMOST, FONT0+attrCLEARWRITE, sizeof(os_msg_CONFIRM_PASSWORD), (UINT8 *)os_msg_CONFIRM_PASSWORD );
	
	if( LIB_GetNumKey( 0, NUM_TYPE_LEADING_ZERO+NUM_TYPE_STAR+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 2, len, buf2 ) == TRUE )
	  {
	  if( buf2[0] < 4 )
	    return( FALSE );
	  }
	
	for( i=0; i<buf1[0]+1; i++ )
	   {
	   if( buf1[i] != buf2[i] )
	     return( FALSE );
	   }

	return( OS_FLS_PutData( F_ADDR_ISP_PSW, ISP_PSW_LEN, buf1 ) );	// L(1) + V(n)
}
#endif

// ---------------------------------------------------------------------------
#if	0
UINT8	IspRemote_ResetPassword( void )
{
UINT8	buf[16] = {0};

	
	// set to default password = "2467"
	buf[0] = 4;
	buf[1] = '2';
	buf[2] = '4';
	buf[3] = '6';
	buf[4] = '7';
	return( OS_FLS_PutData( F_ADDR_ISP_PSW, ISP_PSW_LEN, buf ) );	// L(1) + V(n)
}
#endif

// ---------------------------------------------------------------------------
#if	0
void	IspRemote_GetPassword( UCHAR *psw )
{
UINT8	buf[16] = {0};


	OS_FLS_GetData( F_ADDR_ISP_PSW, ISP_PSW_LEN, buf );	// L(1) + V(12)
//	LIB_DumpHexData( 0, 0, 16, buf );
	
	if( (buf[0] < 1) || (buf[0] > 12) )	// reset to default if invalid
	  {
	  IspRemote_ResetPassword();
	  OS_FLS_GetData( F_ADDR_ISP_PSW, ISP_PSW_LEN, buf );
	  }
	
	memmove( psw, buf, ISP_PSW_LEN );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 01 - Remote In-System-Programming.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
UINT32	SYS_FUNC_IspRemote( void )
{
UINT8	key;
UINT8	psw[16];
UINT8	const	msg_1_DOWNLOAD[] =	{"1-DOWNLOAD"};		// item 1
UINT8	const	msg_2_CHANGE_PSW[] =	{"2-CHANGE PASSWORD"};	// item 2
UINT8	const	msg_RESET[] =		{"RESET"};		// item '*' (invisible)


//	os_HpCopFlag = 0;
	
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_DSS), (UINT8 *)os_msg_DSS );
	
	LIB_LCD_Puts( 1, 0, FONT0, sizeof(msg_1_DOWNLOAD), (UINT8 *)msg_1_DOWNLOAD );
	LIB_LCD_Puts( 2, 0, FONT0, sizeof(msg_2_CHANGE_PSW), (UINT8 *)msg_2_CHANGE_PSW );
	
	do{
	  key = LIB_WaitKey();
	  } while( (key != '1') && (key != '2') && (key !='*') && (key != 'x') );
	  
	LIB_LCD_ClearRow( 1, 2, FONT0 );
	  
	switch( key )
	      {
	      case '1':		// download
	           
	           IspRemote_GetPassword( psw );
	           break;
	           
	      case '2':		// change password
	      	   
	      	   IspRemote_GetPassword( psw );
	      	   if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, &psw[1] ) != TRUE )
	      	     return( FALSE );
	      	   else
	      	     return( IspRemote_ChangePassword() );
	           
	      case '*':		// reset (* + OK)
	      
//	           LIB_LCD_Puts( 1, 0, FONT0, sizeof(msg_RESET), (UINT8 *)msg_RESET );
//	           if( LIB_WaitKeyYesNo( 1, sizeof(msg_RESET) ) )
		   if( LIB_WaitKey() == 'y' )
	             return( IspRemote_ResetPassword() );
		   else
		     return( FALSE );
	           
	      case 'x':
	           return( FALSE );
	      }
	
//	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "2467" ) != TRUE )
//	  return;

	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, &psw[1] ) != TRUE )
	  return( FALSE );

	// --- PCI Security Process ---
	OS_SEC_UnlockAPB();	// 2014-11-19, unlock APB
	
	// Reset application & boot status, erase all sensitive data
////	OS_SECM_ResetBootStatus();	// REMOVED: 2012-08-16, 2017-07-27 moved to softreset() of DSS module
////	OS_SECM_ResetAppStatus();	// REMOVED: 2012-08-16, 2017-07-27 moved to softreset() of DSS module
//	OS_SRAM_Erase();	// formal
////	DIAG_SRAM_Erase();	// temp solution, REMOVED: 2012-08-16, 2017-07-27 moved to softreset() of DSS module
//	DIAG_SRAM_Erase2();	// PATCH: 2012-10-15
	
#ifndef	_BOOT_FROM_POST0_	// REMOVED: 2012-08-16
	PED_EraseSensitiveData();
#endif

	// Activate application status
	// 2017-07-27 moved to DSS.RESET
//	OS_SEC_UnlockAPB();	// 2015-01-14, unlock APB
//	OS_SECM_SetAppStatus();
//	OS_SEC_LockAPB();	// 2015-01-14, lock APB

	// 2014-04-08
	if( (DssRemotePort[0] != 0) && (DssRemotePort[0] <= 5) )
	  DssRemotePort[DssRemotePort[0]+1] = 0x00;
	else
	  {
	  if( DssRemotePort[0] > 5 )
	    DssRemotePort[6] = 0x00;
	  }
	
	if( (DssPort[0] != 0) && (DssPort[0] <= 5) )
	  DssPort[DssPort[0]+1] = 0x00;
	else
	  {
	  if( DssPort[0] > 5 )
	    DssPort[6] = 0x00;
	  }
	  
	api_dss_interface();
}
#endif

// ---------------------------------------------------------------------------
//	called by DSS.RESET after successful APP download
// ---------------------------------------------------------------------------
#if	0
void	SYS_FUNC_SetAppStatus( void )
{
	OS_SEC_UnlockAPB();
	
	OS_SECM_SetAppStatus();
	
	OS_SEC_LockAPB();
}
#endif

// ---------------------------------------------------------------------------
#if	0
void	SYS_FUNC_SetAppBootStatus( void )
{
	OS_SEC_UnlockAPB();
	
	OS_SECM_SetAppStatus();
	OS_SECM_SetWarmBootStatus();
	
	OS_SEC_LockAPB();
}
#endif

// ---------------------------------------------------------------------------
#if	0
void	SYS_FUNC_ResetAppBootStatus( void )
{
	OS_SEC_UnlockAPB();
	
	OS_SECM_ResetBootStatus();
	OS_SECM_ResetAppStatus();
	
	OS_SEC_LockAPB();
	
	DIAG_SRAM_Erase();
}
#endif

// ---------------------------------------------------------------------------
#if	0
void	SYS_FUNC_EraseApp( void )
{
UCHAR	apid;
UCHAR	buf[16];


	os_ConsoleFlag = 0;		// close console channel
	DIAG_FlashCheckType();		// check flash type
	
	memset( buf, 0x00, sizeof(buf) );
	OS_FLS_GetData( APP1_FLASH_ST_ADDRESS, sizeof(buf), buf );
	if( LIB_memcmpc( buf, 0xFF, sizeof(buf) != 0 ) )
	  EraseSector( APP1_FLASH_ST_ADDRESS );
	
	memset( buf, 0x00, sizeof(buf) );
	OS_FLS_GetData( APP2_FLASH_ST_ADDRESS, sizeof(buf), buf );
	if( LIB_memcmpc( buf, 0xFF, sizeof(buf) != 0 ) )	  
	  EraseSector( APP2_FLASH_ST_ADDRESS );

	OS_FLS_GetData( F_ADDR_APP_ID, APP_ID_LEN, (UCHAR *)&apid );
	if( apid != 0xFF )
	  {
	  apid = 0xFF;
	  OS_FLS_PutData( F_ADDR_APP_ID, APP_ID_LEN, (UCHAR *)&apid );
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 02 - Setup new BOOT ROM and warm reset.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
#ifndef	_BOOT_FROM_POST0_

UINT32	SYS_FUNC_SwitchBootROM( void )
{
UINT8	buffer[8];
UINT8	id;
UINT8	result;


	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "2467" ) != TRUE )
	  return( FALSE );
	
	LIB_LCD_Cls();
	
	api_dss2_init(1);	// init to double APP platform
	
	while(1)
	{
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_SWITCH_BOOT_ROM), (UINT8 *)os_msg_SWITCH_BOOT_ROM );
	
	buffer[0] = 2;	// starting row number
        buffer[1] = 3;	// max lcd row cnt
        buffer[2] = 2;	// list items
        buffer[3] = 3;	// item length
        buffer[4] = 0;	// offset of LEN field in item
        buffer[5] = FONT0;
        id = api_dss2_apid() & 0x01;
	id = LIB_ListBox( id, &buffer[0], (UINT8 *)&os_list_BOOT_ROM[0], 0 );
	if( id == 0xff )
	  return( FALSE );
	  
	if( LIB_WaitKeyMsgYesNo( 3, COL_LEFTMOST, sizeof(os_msg_Q_RUN_AP), (UINT8 *)os_msg_Q_RUN_AP ) == TRUE )
	  {
	  if( api_dss2_run( id ) == apiFailed )
	    return( FALSE );
	  }
	else
	  LIB_LCD_Cls();
	}
}

#endif
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 04 - Remote In-System-Programming.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	SYS_FUNC_IspT2T( void )
{
	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "8228" ) != TRUE )
	  return( FALSE );
	  
	return( T2T_TaskManager() );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 03 - Auto setup ethernet MAC code.
//		  MAC = OUI(3) + TSN(3)
//		  OUI = 00 F0 00
//		  TSN = MCU S/N
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	SYS_FUNC_SetMAC( void )
{
#ifdef	_LAN_ENABLED_

UINT8	MAC[6];
UINT32	data;


	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_AUTO_SETUP_MAC), (UINT8 *)os_msg_AUTO_SETUP_MAC );

//	api_lan_init();		// init ethernet
	
	memset( MAC, 0x00, sizeof(MAC) );
	
	API_getMAC( MAC );
	LIB_DispHexByte( 1, 0, MAC[0] );
	LIB_DispHexByte( 1, 3, MAC[1] );
	LIB_DispHexByte( 1, 6, MAC[2] );
	LIB_DispHexByte( 1, 9, MAC[3] );
	LIB_DispHexByte( 1, 12, MAC[4] );
	LIB_DispHexByte( 1, 15, MAC[5] );
	
	LIB_WaitKey();

	data = BSP_RD32(PMU_ID0_REG);	// binary format: CUST_ID(1) + SN(3)
	
#if	0
	MAC[0] = 0x00;	// SYMLINK OUI = 00 F0 FF (test only, 24 bits)
	MAC[1] = 0xF0;	//
	MAC[2] = 0xFF;	//

	MAC[3] = (data & 0x00FF0000) >> 16;
	MAC[4] = (data & 0x0000FF00) >> 8;
	MAC[5] =  data & 0x000000FF;
#else
	MAC[0] = 0x6C;	// SYMLINK OUI = 6C 15 24 Dx (formal 28 bits)
	MAC[1] = 0x15;	//
	MAC[2] = 0x24;	//
	MAC[3] = 0xD0;	//
	
	MAC[3] |= ((data & 0x000F0000) >> 16);
	MAC[4] =   (data & 0x0000FF00) >> 8;
	MAC[5] =    data & 0x000000FF;
#endif

	return( API_setMAC( MAC ) );

#endif
}
#endif

// ---------------------------------------------------------------------------
UINT32	SYS_FUNC_SetMAC( void )
{
UINT32	result = FALSE;
UINT8	*pstr;
UINT8	buffer[128];
UINT8	cmd_if_hwaddr[]	= {"ifconfig | grep HWaddr"};
FILE	*file_cmmd;


	LIB_LCD_Puts( 0, 0, FONT0, sizeof("ETH MAC"), (UINT8 *)"ETH MAC" );
	
	file_cmmd = popen( cmd_if_hwaddr, "r" );
	
	// response: "eth0      Link encap:Ethernet  HWaddr 00:f0:ff:3e:3c:40"
	if( fgets( buffer, 56, file_cmmd ) )
	  {
//	  LIB_DumpHexData( 1, 0, 56, buffer );
	  pstr = (char *)strpbrk( buffer, "HW" );
	  if( pstr )
	    {
	    LIB_LCD_Puts( 2, 0, FONT0, 17, pstr+7 );
	    result = TRUE;
	    }
	  }
	
	pclose(file_cmmd);
	
	LIB_WaitKey();
	
	return( result );
}

// ---------------------------------------------------------------------------
void	POST_AutoSetMAC( void )
{
UINT8	mac_b[6];	// 12 34 56 78 9A BC
UINT8	mac_s[17+1];	// 12:23:56:78:9A:BC
UINT8	cmd_if_down[]	= {"ifconfig eth0 down"};
UINT8	cmd_if_up[]	= {"ifconfig eth0 up"};
UINT8	cmd_if_hw[]	= {"ifconfig eth0 hw ether "};
UINT8	buffer[128];
FILE	*file_cmmd;


//	api_sys_genMAC( mac_b, mac_s );		// using SYMLINK OUI (6C 15 24 Dx xx xx)
	api_sys_genMAC_PSEUDO( mac_b, mac_s );	// using PSEUDO  OUI (00 F0 FF xx xx xx)
	mac_s[17] = 0;

	file_cmmd = popen( cmd_if_down, "r" );
	pclose(file_cmmd);
	
	memset( buffer, 0x00, sizeof(buffer) );
	sprintf( buffer, "%s%s", cmd_if_hw, mac_s );
	file_cmmd = popen( buffer, "r" );
	pclose(file_cmmd);
	
	file_cmmd = popen( cmd_if_up, "r" );
	pclose(file_cmmd);
}

// ---------------------------------------------------------------------------
#if	0
void	POST_AutoSetMAC( void )
{
#ifdef	_LAN_ENABLED_

UINT8	MAC[6];
UINT8	buf[DSSGV_LEN];
UINT32	data;


	memset( MAC, 0x00, sizeof(MAC) );
	API_getMAC( MAC );

#if	0
	if( (MAC[0] == 0x00) && (MAC[1] == 0xF0) && (MAC[2] == 0xFF) )
	  return;
#else
	if( (MAC[0] == 0x6C) && (MAC[1] == 0x15) && (MAC[2] == 0x24) && ((MAC[3] &0xF0) == 0xD0) )
	  return;
#endif
	else
	  {
	  data = BSP_RD32(PMU_ID0_REG);	// binary format: CUST_ID(1) + SN(3)

#if	0
	  MAC[0] = 0x00;	// SYMLINK OUI = 00 F0 FF (test only, 24 bits)
	  MAC[1] = 0xF0;	//
	  MAC[2] = 0xFF;	//
	  
	  MAC[3] = (data & 0x00FF0000) >> 16;
	  MAC[4] = (data & 0x0000FF00) >> 8;
	  MAC[5] =  data & 0x000000FF;
#else
	  MAC[0] = 0x6C;	// SYMLINK OUI = 6C 15 24 Dx (formal 28 bits)
	  MAC[1] = 0x15;	//
	  MAC[2] = 0x24;	//
	  MAC[3] = 0xD0;	//
	
	  MAC[3] |= ((data & 0x000F0000) >> 16);
	  MAC[4] =   (data & 0x0000FF00) >> 8;
	  MAC[5] =    data & 0x000000FF;
#endif

	  API_setMAC( MAC );
	  
	  // erase DSS parameters
	  memset( DssTelNum, 0x00, sizeof(DssTelNum) );
	  memset( DssRemoteIP, 0x00, sizeof(DssRemoteIP) );
	  memset( DssRemotePort, 0x00, sizeof(DssRemotePort) );
	  memset( DssPort, 0x00, sizeof(DssPort) );
	  
	  // erase DSS parameters in FLASH
	  memset( buf, 0x00, sizeof(buf) );
	  OS_FLS_PutData( F_ADDR_DSSGV, DSSGV_LEN, buf );
	  }
	  
#endif
}
#endif

// ---------------------------------------------------------------------------
#if	0
void	POST_AutoSetMAC_SYMLINK( void )
{
#ifdef	_LAN_ENABLED_

UINT8	MAC[6];
UINT32	data;


	memset( MAC, 0x00, sizeof(MAC) );
	API_getMAC( MAC );

	if( (MAC[0] == 0x6C) && (MAC[1] == 0x15) && (MAC[2] == 0x24) && ((MAC[3] & 0xF0) == 0xD0) )
	  return;
	else
	  {
	  data = BSP_RD32(PMU_ID0_REG);	// binary format: CUST_ID(1) + SN(3)

	  MAC[0] = 0x6C;	// SYMLINK OUI = 6C 15 24 Dx (formal 28 bits)
	  MAC[1] = 0x15;	//
	  MAC[2] = 0x24;	//
	  MAC[3] = 0xD0;	//
	
	  MAC[3] |= ((data & 0x000F0000) >> 16);
	  MAC[4] =   (data & 0x0000FF00) >> 8;
	  MAC[5] =    data & 0x000000FF;

	  API_setMAC( MAC );
	  }
	  
#endif
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 05 - Setup Terminal Serial Number to FLASH (or EEPROM).
//		  the TSN is the product S/N on the barcode sticker.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	SYS_FUNC_SetTSN( void )
{
UINT8	VerNo[128];
UINT8	ECC_tsn[4];
UINT32	i;
UINT32	status;
UINT32	len;
UINT8	sum;
UINT8	key;


TSN_BEGIN:
	LIB_LCD_Cls();
	
	LIB_LCD_Puts( 0, 0, FONT0, strlen(os_msg_SETUP_TSN), (UINT8 *)os_msg_SETUP_TSN );
	
	LIB_LCD_Puts( 2, 0, FONT0, strlen("1-EDC"), (UINT8 *)"1-EDC" );
	LIB_LCD_Puts( 3, 0, FONT0, strlen("2-ECC"), (UINT8 *)"2-ECC" );
	LIB_LCD_Puts( 4, 0, FONT0, strlen("3-UID"), (UINT8 *)"3-UID" );

TSN_SELECT:
	key = LIB_WaitKey();
	if( key == 'x' )
	  return( TRUE );
	  
	if( key == '1' )
	  {
	  LIB_LCD_Puts( 0, strlen(os_msg_SETUP_TSN), FONT0, strlen("-EDC"), (UINT8 *)"-EDC" );
	  goto TSN_EDC;
	  }
	  
	if( key == '2' )
	  {
	  LIB_LCD_Puts( 0, strlen(os_msg_SETUP_TSN), FONT0, strlen("-ECC"), (UINT8 *)"-ECC" );
	  LIB_LCD_ClearRow( 2, 2+1, FONT0 );
	  
	  // show current TSN
	  OS_FLS_GetData_TSN_ECC( 4, VerNo );
	  LIB_DispHexByte( 2, 0, VerNo[0] );
	  LIB_DispHexByte( 2, 3, VerNo[1] );
	  LIB_DispHexByte( 2, 6, VerNo[2] );
	  LIB_DispHexByte( 2, 9, VerNo[3] );
	  
	  if( LIB_GetAlphaNumDigits( 3, 0, FONT0, 8, VerNo ) )
	    {
	    if( VerNo[0] == 8 )
	      {
	      for( i=0; i<4; i++ )
		 ECC_tsn[i] = LIB_ascw2hexb( &VerNo[1+i*2] ); // convert to hex format
		 
//	      status = OS_FLS_PutData( F_ADDR_TSN_ECC, 4, ECC_tsn );
	      status = OS_FLS_PutData_TSN_ECC( 4, ECC_tsn );	// 2018-01-25
	      return( status );
	      }
	    else
	      {
//	      LIB_LCD_Cls();
	      goto TSN_BEGIN;
	      }
	    }
	  else
	    {
//	    LIB_LCD_Cls();
	    goto TSN_BEGIN;
	    }
	  }

	if( key == '3' )
	  LIB_LCD_Puts( 0, strlen(os_msg_SETUP_TSN), FONT0, strlen("-UID"), (UINT8 *)"-UID" );
	else
	  goto TSN_SELECT;

	// *** Solution 1: ZA9L Chip ID
//	api_sys_info( SID_TerminalSerialNumber2, VerNo );	// LEN[1]+TSN[8]
//	
//	LIB_LCD_Puts( 2, 0, FONT0, 8, &VerNo[1] );
//	
//	LIB_WaitKey();
//	
//	return( TRUE );

TSN_EDC:
	LIB_LCD_ClearRow( 2, 2+1, FONT0 );

	// *** Solution 2: Product TSN (Barcode scan or Manual keyin)
	if( key == '1' )
	  OS_FLS_GetData( F_ADDR_TSN, TSN_LEN, VerNo );	// TSN
	else
	  OS_FLS_GetData( F_ADDR_UID, UID_LEN, VerNo );	// UID
//	LIB_DumpHexData( 0, 0, TSN_LEN, VerNo );
	if( VerNo[0] )
	  {
	  // check data validity
	  len = VerNo[0] & 0x0F;
	  for( i=1; i<len+1; i++ )
	     {
	     if( VerNo[i] >= 0x80 )
	       VerNo[i] = 0x20;
	     }
	  
	  // show current TSN
	  LIB_LCD_Puts( 2, 0, FONT0, len, &VerNo[1] );
	  }
	  
//	if( !LIB_OpenAUX( COM1, auxBARCODE ) )
	if( !LIB_OpenAUX( COM2, auxBARCODE ) )	// 2016-07-04
	  return( FALSE );

	status = FALSE;
	while(1)
	     {
	     // manual keyin
	     if( LIB_GetKeyStatus() == apiReady )
	       {
	       if( LIB_GetNumKey( 0, NUM_TYPE_LEADING_ZERO+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 2, 9, &VerNo[1] ) )
	         {
	         if( VerNo[1] == 0 )
	           {
	           if( key == '3' )
	             {
	             LIB_LCD_Puts( 1, 0, FONT0, strlen("ERASE UID"), (UINT8 *)"ERASE UID" );
	             if( LIB_WaitKeyYesNo( 1, strlen("ERASE UID") ) )
	               {
	               LIB_LCD_ClearRow( 1, 1, FONT0 );
	               if( LIB_EnterPassWord( 1, 1, "6731" ) == TRUE )
	               	 {
	                 memset( VerNo, 0xFF, UID_LEN );
	                 OS_FLS_PutData( F_ADDR_UID, UID_LEN, VerNo );
	                 }
	               }
	             }
	             
	           status = TRUE;
	           goto EXIT;
	           }
	         if( VerNo[1] == 9 )
	           {
	           // check sum by adding all digits
	           sum = 0;
	           for( i=0; i<8; i++ )
	              sum += (VerNo[i+2] & 0x0F);
	           
	           while(1)
	        	{
	        	if( sum >= 10 )
	        	  sum -= 10;
	        	else
	        	  break;
	        	}
	           
	           // compare only the lower nibble
	           if( sum == (VerNo[10] & 0x0F) )
	             {
	             VerNo[1] = 8;
	             status = TRUE;
	             }
	           }
	         }
	       break;
	       }
	     else
	       {
	       // barcode scan
	       if( api_aux_rxready( post_dhn_aux, VerNo ) == apiReady )
	         {
	         LIB_ReceiveAUX( VerNo );
	         
//	         LIB_DumpHexData( 0, 4, VerNo[0]+VerNo[1]*256+2, VerNo );
	         
	         if( VerNo[0] < 8 )
	           break;
	           
	         VerNo[1] = VerNo[0];
	         i = VerNo[0];	// original length
	         if( i > 8 )
	           {
	           VerNo[1] = 8;
	           memmove( &VerNo[2], &VerNo[i-6], 8 ); // adopt only last 8-digi
	           }
	         
	         // show the scanned barcode
	         LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, VerNo[1] & 0x0F, &VerNo[2] );
	         
	         if( LIB_WaitKey() == 'y' )
	           status = TRUE;
	           
	         break;
	         }
	       }
	     }
	
	if( status )
	  {
//	  LIB_DumpHexData( 0, 6, 9, &VerNo[1] );
	  
	  // write new TSN to Flash: L(1)-V(8)
	  if( key == '1' )
	    status = OS_FLS_PutData( F_ADDR_TSN, 9, &VerNo[1] );	// TSN
	  else
	    status = OS_FLS_PutData( F_ADDR_UID, 9, &VerNo[1] );	// UID
	  if( status )
	    {
	    if( key == '1' )
	      OS_SECM_PutData( S_ADDR_TSN, 9, &VerNo[1] );	// 2016-10-19, backup to SECM
	    }
	  
	  goto EXIT;
	  }
	else
	  {
	  LIB_CloseAUX();
	  goto TSN_BEGIN;
	  }

EXIT:
	LIB_CloseAUX();
	
	return( status );
}
#endif

// ---------------------------------------------------------------------------
// This function is called by POST to distinguish ES4 from S4.
// The FONT4 version shall be '3' for ES4.
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_IsESX_Ex( void )
{
UINT8	data[2];


	os_Config_ESX = FALSE;
	
	FLASH_ReadData( (void *)ADDR_FONT_VERSION, data, 2 );	// 00XX
	if( data[1] == 3 )	// AS350-ES4?
	  os_Config_ESX = TRUE;

	if( !os_TSC_ENABLED )
	  os_Config_ESX = TRUE;	// S1NS

	return( os_Config_ESX );
}
#endif

// ---------------------------------------------------------------------------
// This function is called by BSP & POSAPI to distinguish ES4 from S4.
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_IsESX( void )
{
	return( os_Config_ESX );
}
#endif

// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_CheckChineseFont( UINT8 *version )
{
UINT8	data[16];
UINT8	dbuf[16];
UINT8	ver[2];


	// 2012-10-19, get built-in full Chinese font version
	OS_FLS_GetData( ADDR_FONT_VERSION, 2, data );	// 00XX
	dbuf[0] = (data[1] & 0xF0) >> 4;
	if( dbuf[0] < 10 )
	  ver[0] = dbuf[0] + 0x30;
	else
	  ver[0] = (dbuf[0] - 10) + 0x41;

	dbuf[0] = data[1] & 0x0F;
	if( dbuf[0] < 10 )
	  ver[1] = dbuf[0] + 0x30;
	else
	  ver[1] = (dbuf[0] - 10) + 0x41;
	  
	version[0] = 2;		// set default
	version[1] = 'F';
	version[2] = 'F';
	
	if( (ver[0] == 'F') && (ver[1] == 'F') )
	  return( FALSE );
	
	// verify integrity of Chinese Big5 codes
	OS_FLS_GetData( ADDR_FULL_BIG5_CODE_TABLE, 2, data );
	if( (data[0] != 0xA1) || (data[1] != 0x40) )
	  return( FALSE );
	  
	OS_FLS_GetData( ADDR_FULL_BIG5_CODE_TABLE+(FULL_BIG5_CHAR_NUM-1)*2, 2, data );
	if( (data[0] != 0xF9) || (data[1] != 0xFE) )
	  return( FALSE );
	
	// verify integrity of Chinese FONT2 (empty?)
	OS_FLS_GetData( ADDR_FULL_PRT_FONT2_24X24_BMP, 8, data );
	if( (data[0] == 0xFF) && (data[1] == 0xFF) && (data[2] == 0xFF) && (data[3] == 0xFF) &&
	    (data[4] == 0xFF) && (data[5] == 0xFF) && (data[6] == 0xFF) && (data[7] == 0xFF) )
	  return( FALSE );
	  
	// verify integrity of Chinese FONT4 (empty?)
	OS_FLS_GetData( ADDR_FULL_PRT_FONT4_16X16_BMP, 8, data );
	if( (data[0] == 0xFF) && (data[1] == 0xFF) && (data[2] == 0xFF) && (data[3] == 0xFF) &&
	    (data[4] == 0xFF) && (data[5] == 0xFF) && (data[6] == 0xFF) && (data[7] == 0xFF) )
	  return( FALSE );

	version[0] = 2;		// get font version number
	version[1] = ver[0];	//
	version[2] = ver[1];	//

	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 06 - Get system version number.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	SYS_FUNC_GetVersion( void )
{
UINT8	VerNo[16];
UINT8	dbuf[8];
UINT8	ver[2];


	api_sys_info( SID_TerminalSerialNumber, VerNo );	// LEN[1]+CID[3]+SN[8]
	LIB_LCD_Puts( 0, 0, FONT0, 4, "CID:" );
	LIB_LCD_Puts( 0, 4, FONT0, 3, &VerNo[1] );
	
	LIB_LCD_Puts( 1, 0, FONT0, 4, "S/N:" );
	LIB_LCD_Puts( 1, 4, FONT0, 8, &VerNo[4] );
	
	api_sys_info( SID_TerminalSerialNumber2, VerNo );
	LIB_LCD_Puts( 2, 0, FONT0, 4, "TSN:" );
	LIB_LCD_Puts( 2, 4, FONT0, 8, &VerNo[1] );

	LIB_LCD_Puts( 3, 0, FONT0, 8, "TSN_ECC:" );	// TSN_ECC:XX XX XX XX
	OS_FLS_GetData_TSN_ECC( 4, VerNo );
	if( (VerNo[0] != 0xFF) || (VerNo[1] != 0xFF) || (VerNo[2] != 0xFF) || (VerNo[3] != 0xFF) )
	  {
	  LIB_DispHexByte( 3, 8 , VerNo[0] );
	  LIB_DispHexByte( 3, 11, VerNo[1] );
	  LIB_DispHexByte( 3, 14, VerNo[2] );
	  LIB_DispHexByte( 3, 17, VerNo[3] );
	  }

	DIAG_CheckChineseFont( dbuf );
	ver[0] = dbuf[1];
	ver[1] = dbuf[2];

	LIB_LCD_Puts( 4, 0, FONT0, 9, "FONT VER:" );
	LIB_LCD_Puts( 4, 9, FONT0, 2, ver );
	
	LIB_WaitKey();
	LIB_LCD_Cls();

	// 2015-03-03, LIB version number
	api_sys_info( SID_BIOSversion, VerNo );
	LIB_LCD_Puts( 0, 0, FONT0, 6, "BIOS :" );
	LIB_LCD_Puts( 0, 6, FONT0, VerNo[0], &VerNo[1] );
	
	api_sys_info( SID_BSPversion, VerNo );
	LIB_LCD_Puts( 0, 11, FONT0, 6, "BSP  :" );
	LIB_LCD_Puts( 0, 17, FONT0, VerNo[0], &VerNo[1] );

	api_sys_info( SID_MODEMversion, VerNo );
	LIB_LCD_Puts( 1, 0, FONT0, 6, "MODEM:" );
	LIB_LCD_Puts( 1, 6, FONT0, VerNo[0], &VerNo[1] );

	api_sys_info( SID_POSAPIversion, VerNo );
	LIB_LCD_Puts( 1, 11, FONT0, 6, "API  :" );
	LIB_LCD_Puts( 1, 17, FONT0, VerNo[0], &VerNo[1] );
	
	api_sys_info( SID_PRINTERversion, VerNo );
	LIB_LCD_Puts( 2, 0, FONT0, 6, "PRINT:" );
	LIB_LCD_Puts( 2, 6, FONT0, VerNo[0], &VerNo[1] );

	api_sys_info( SID_USBversion, VerNo );
	LIB_LCD_Puts( 2, 11, FONT0, 6, "USB  :" );
	LIB_LCD_Puts( 2, 17, FONT0, VerNo[0], &VerNo[1] );

	api_sys_info( SID_TCPIPversion, VerNo );
	LIB_LCD_Puts( 3, 0, FONT0, 6, "TCPIP:" );
	LIB_LCD_Puts( 3, 6, FONT0, VerNo[0], &VerNo[1] );

	api_sys_info( SID_LCDTFTversion, VerNo );
	LIB_LCD_Puts( 3, 11, FONT0, 6, "TFT  :" );
	LIB_LCD_Puts( 3, 17, FONT0, VerNo[0], &VerNo[1] );

	api_sys_info( SID_FTPversion, VerNo );
	LIB_LCD_Puts( 4, 0, FONT0, 6, "FTP  :" );
	LIB_LCD_Puts( 4, 6, FONT0, VerNo[0], &VerNo[1] );

	api_sys_info( SID_TSCversion, VerNo );
	LIB_LCD_Puts( 4, 11, FONT0, 6, "TSC  :" );
	LIB_LCD_Puts( 4, 17, FONT0, VerNo[0], &VerNo[1] );

	api_sys_info( SID_SSLversion, VerNo );
	LIB_LCD_Puts( 5, 0, FONT0, 6, "SSL  :" );
	LIB_LCD_Puts( 5, 6, FONT0, VerNo[0], &VerNo[1] );

	api_sys_info( SID_CRYPTOversion, VerNo );
	LIB_LCD_Puts( 6, 0, FONT0, 6, "CRYPT:" );
	LIB_LCD_Puts( 6, 6, FONT0, VerNo[0], &VerNo[1] );
	
	// 2015-02-10, BIOS release date
//	OS_FLS_GetData( F_ADDR_BIOS_RELEASE_DATE, BIOS_RELEASE_DATE_LEN, VerNo );	// read stamped release date, YYYYMMDDhhmmss
//	LIB_LCD_Puts( 7, 0, FONT0, 14, VerNo );

	LIB_WaitKey();
	
	LIB_LCD_Cls();
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 40 - Show system event logs.
//		  SYS EVENT LOG
//
//		  index: xxxx
//		  YY/MM/DD hh:mm:ss	(date/time)    - ascii (12 bytes)
//		  xx xx xx xx		(event source) - ascii (4 bytes)
//		  xx xx xx xx		(event id)     - ascii (4 bytes)
//
//		  (more/end)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	SYS_FUNC_EventLog( void )
{
UINT16	i;
UINT8	msg_TITLE[] = {"SYS EVENT LOG"};
UINT8	msg_INDEX[] = {"INDEX:"};
UINT8	msg_more[]  = {"more..."};
UINT8	msg_end[]   = {"-END-"};
UINT8	log[32];
UINT8	buffer[32];


	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(msg_TITLE), (UINT8 *)msg_TITLE );
	
	LIB_LCD_Puts( 2, 0, FONT0, sizeof(msg_INDEX), (UINT8 *)msg_INDEX );
	
	for( i=0; i<8; i++ )
	   {
	   LIB_DispHexWord( 2, sizeof(msg_INDEX), i );
	   api_sys_eventlog( i, log );
	   if( LIB_memcmpc( log, 0xFF, 12 ) == 0 )
	     {
	     LIB_LCD_Puts( 7, 0, FONT0+attrCLEARWRITE, sizeof(msg_end), (UINT8 *)msg_end );
	     break;
	     }
	   else
	     {
	     // date & time
	     buffer[0] = log[0];
	     buffer[1] = log[1];
	     buffer[2] = '-';
	     buffer[3] = log[2];
	     buffer[4] = log[3];
	     buffer[5] = '-';
	     buffer[6] = log[4];
	     buffer[7] = log[5];
	     
	     buffer[8] = ' ';
	     
	     buffer[9] = log[6];
	     buffer[10]= log[7];
	     buffer[11]= ':';
	     buffer[12]= log[8];
	     buffer[13]= log[9];
	     buffer[14]= ':';
	     buffer[15]= log[10];
	     buffer[16]= log[11];
	     LIB_LCD_Puts( 3, 0, FONT0, 17, buffer );
	     
	     // event source
	     LIB_DispHexByte( 4, 0, log[12] );
	     LIB_DispHexByte( 4, 3, log[13] );
	     LIB_DispHexByte( 4, 6, log[14] );
	     LIB_DispHexByte( 4, 9, log[15] );
	     
	     // event id
	     LIB_DispHexByte( 5, 0, log[16] );
	     LIB_DispHexByte( 5, 3, log[17] );
	     LIB_DispHexByte( 5, 6, log[18] );
	     LIB_DispHexByte( 5, 9, log[19] );     
	     
	     LIB_LCD_Puts( 7, 0, FONT0+attrCLEARWRITE, sizeof(msg_more), (UINT8 *)msg_more );
	     LIB_WaitKey();
	     }
	   }
	
	LIB_WaitKey();
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 80 - Setup IMEK & IAEK for contactless reader (CL).
//		  (1) by keyin
//		  (2) by UART download
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
//#ifndef	_BOOT_FROM_POST0_
#if	0
UINT32	SYS_FUNC_SetIEK( void )
{
UINT32	i;
UINT8	msg_ruler[]	= {"................"};
UINT8	msg_SETUP_IEK[]	= {"SETUP IEK"};
UINT8	msg_1_IMEK[]	= {"1-IMEK"};
UINT8	msg_2_IAEK[]	= {"2-IAEK"};
UINT8	msg_IMEK[]	= {"IMEK"};
UINT8	msg_IAEK[]	= {"IAEK"};
UINT8	msg_IMEK1[]	= {"IMEK1:"};	// IMEK = IMEK1 xor IMEK2 (dual control)
UINT8	msg_IMEK2[]	= {"IMEK2:"};
UINT8	msg_IAEK1[]	= {"IAEK1:"};	// IAEK = IAEK1 xor IAEK2 (dual control)
UINT8	msg_IAEK2[]	= {"IAEK2:"};
UINT8	msg_KCV1[]	= {"KCV1="};
UINT8	msg_KCV2[]	= {"KCV2="};
UINT8	msg_KCV[]	= {"KCV="};
//UINT8	msg_CONFIRM[]	= {"ENTER=Confirm"};
UINT8	msg_UPDATE[]	= {"UPDATE"};
UINT8	select;
UINT8	buf[64];
UINT8	imek1[16];
UINT8	imek2[16];
UINT8	iaek1[16];
UINT8	iaek2[16];
UINT8	imek[16];
UINT8	iaek[16];
UINT8	kcv[8];
UINT8	XCSN[16];

	
	// request the same password as defined in ISP download
	IspRemote_GetPassword( buf );
	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, &buf[1] ) != TRUE )
	  return( FALSE );
ST:
	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(msg_SETUP_IEK), (UINT8 *)msg_SETUP_IEK );
	
	LIB_LCD_Puts( 2, 0, FONT0, sizeof(msg_1_IMEK), (UINT8 *)msg_1_IMEK );
	LIB_LCD_Puts( 3, 0, FONT0, sizeof(msg_2_IAEK), (UINT8 *)msg_2_IAEK );
	
	do{
	  select = LIB_WaitKey();
	  if( select == 'x' )
	    return( TRUE );	// end of process
	  }while( (select != '1') && (select != '2') && (select != 'y') );

	// XCSN(16) =	CSN[0], 0x01, CSN[1], 0x23, CSN[2], 0x45, CSN[3], 0x67
	//		CSN[4], 0x89, CSN[5], 0xAB, CSN[6], 0xCD, CSN[7], 0xEF
	api_sys_info( SID_TerminalSerialNumber, buf );
	XCSN[0]  = buf[4];	// CSN[0]
	XCSN[1]  = 0x01;
	XCSN[2]  = buf[5];	// CSN[1]
	XCSN[3]  = 0x23;
	XCSN[4]  = buf[6];	// CSN[2]
	XCSN[5]  = 0x45;
	XCSN[6]  = buf[7];	// CSN[3]
	XCSN[7]  = 0x67;
	XCSN[8]  = buf[8];	// CSN[4]
	XCSN[9]  = 0x89;
	XCSN[10] = buf[9];	// CSN[5]
	XCSN[11] = 0xAB;
	XCSN[12] = buf[10];	// CSN[6]
	XCSN[13] = 0xCD;
	XCSN[14] = buf[11];	// CSN[7]
	XCSN[15] = 0xEF;
//	DIAG_FUNC_GenerateXCSN( XCSN );

	if( select == '1' )
	  {
	  // IMEK1:		->	IMEK2:			->	IMEK
	  // ................		................
	  // xxxxxxxxxxxxxxxx		xxxxxxxxxxxxxxxx
	  // xxxxxxxxxxxxxxxx		xxxxxxxxxxxxxxxx
	  //
	  // KCV1=			KCV2=				KCV=
	  // (Y/N)?			(Y/N)?				(Y/N)?
	  // 
	  
	  // get current KCV of IMEK
	  LIB_LCD_Cls();
	  LIB_LCD_Puts( 0, 0, FONT0, sizeof(msg_IMEK), (UINT8 *)msg_IMEK );
	  LIB_LCD_Puts( 5, 0, FONT0, sizeof(msg_KCV), (UINT8 *)msg_KCV );
	  OS_FLS_GetData( F_ADDR_CL_IMEK_KCV, CL_IMEK_KCV_LEN, kcv );
	  LIB_DispHexByte( 5, 5,  kcv[0] );
	  LIB_DispHexByte( 5, 8,  kcv[1] );
	  LIB_DispHexByte( 5, 11, kcv[2] );
	  LIB_LCD_Puts( 6, 0, FONT0, sizeof(msg_UPDATE), (UINT8 *)msg_UPDATE );
	  if( !LIB_WaitKeyYesNo( 6, 6 ) )
	    goto ST;
	  
//	  OS_FLS_GetData( F_ADDR_CL_IMEK, CL_IMEK_LEN, buf );
//	  PED_TripleDES2( XCSN, 16, buf, imek1 );
//	  LIB_DumpHexData( 0, 0, 16, imek1 );
SET_IMEK1:
	  // --- IMEK1 ---
	  LIB_LCD_Cls();
	  LIB_LCD_Puts( 0, 0, FONT0, sizeof(msg_IMEK1), (UINT8 *)msg_IMEK1 );
	  LIB_LCD_Puts( 1, 0, FONT0, sizeof(msg_ruler), (UINT8 *)msg_ruler );
	  memset( buf, 0x00, sizeof(buf) );
	  while(1)
	       {
	       if( LIB_GetAlphaNumDigits( 2, 0, FONT0, 16, buf ) )
	         {
	         if( buf[0] != 16 )
	           continue;
	         
		 for( i=0; i<8; i++ )
		    imek1[i] = LIB_ascw2hexb( &buf[1+i*2] ); // convert to hex format
		 break;
	         }
	       else
	         return( FALSE );
	       }
	  
	  memset( buf, 0x00, sizeof(buf) );
	  while(1)
	       {
	       if( LIB_GetAlphaNumDigits( 3, 0, FONT0, 16, buf ) )
	         {
	         if( buf[0] != 16 )
	           continue;
	         
		 for( i=0; i<8; i++ )
		    imek1[i+8] = LIB_ascw2hexb( &buf[1+i*2] ); // convert to hex format
		 break;
	         }
	       else
	         return( FALSE );
	       }

	  // KCV1 = TDES( 8*0x00, IMEK1 )
	  LIB_LCD_Puts( 5, 0, FONT0, sizeof(msg_KCV1), (UINT8 *)msg_KCV1 );
	  
	  memset( buf, 0x00, sizeof(buf) );
	  PED_TripleDES( imek1, 8, buf, kcv );
	  LIB_DispHexByte( 5, 5,  kcv[0] );
	  LIB_DispHexByte( 5, 8,  kcv[1] );
	  LIB_DispHexByte( 5, 11, kcv[2] );

	  if( !LIB_WaitKeyYesNo( 6, 0 ) )
	    goto SET_IMEK1;
//	  if( LIB_WaitKey() == 'x' )
//	    return( FALSE );
//	  LIB_DumpHexData( 0, 0, 16, imek1 );

SET_IMEK2:
	  // --- IMEK2 ---
	  LIB_LCD_Cls();
	  LIB_LCD_Puts( 0, 0, FONT0, sizeof(msg_IMEK2), (UINT8 *)msg_IMEK2 );
	  LIB_LCD_Puts( 1, 0, FONT0, sizeof(msg_ruler), (UINT8 *)msg_ruler );
	  memset( buf, 0x00, sizeof(buf) );
	  while(1)
	       {
	       if( LIB_GetAlphaNumDigits( 2, 0, FONT0, 16, buf ) )
	         {
	         if( buf[0] != 16 )
	           continue;
	         
		 for( i=0; i<8; i++ )
		    imek2[i] = LIB_ascw2hexb( &buf[1+i*2] ); // convert to hex format
		 break;
	         }
	       else
	         return( FALSE );
	       }
	  
	  memset( buf, 0x00, sizeof(buf) );
	  while(1)
	       {
	       if( LIB_GetAlphaNumDigits( 3, 0, FONT0, 16, buf ) )
	         {
	         if( buf[0] != 16 )
	           continue;
	         
		 for( i=0; i<8; i++ )
		    imek2[i+8] = LIB_ascw2hexb( &buf[1+i*2] ); // convert to hex format
		 break;
	         }
	       else
	         return( FALSE );
	       }

	  // KCV2 = TDES( 8*0x00, IMEK2 )
	  LIB_LCD_Puts( 5, 0, FONT0, sizeof(msg_KCV2), (UINT8 *)msg_KCV2 );
	  
	  memset( buf, 0x00, sizeof(buf) );
	  PED_TripleDES( imek2, 8, buf, kcv );
	  LIB_DispHexByte( 5, 5,  kcv[0] );
	  LIB_DispHexByte( 5, 8,  kcv[1] );
	  LIB_DispHexByte( 5, 11, kcv[2] );

	  if( !LIB_WaitKeyYesNo( 6, 0 ) )
	    goto SET_IMEK2;
//	  if( LIB_WaitKey() == 'x' )
//	    return( FALSE );
//	  LIB_DumpHexData( 0, 0, 16, imek2 );
	  
	  // final plaintext IMEK = IMEK1 xor IMEK2
	  for( i=0; i<16; i++ )
	     imek[i] = imek1[i]^imek2[i];
	  
	  // final KCV of IMEK
	  LIB_LCD_Cls();
	  LIB_LCD_Puts( 0, 0, FONT0, sizeof(msg_IMEK), (UINT8 *)msg_IMEK );
	  LIB_LCD_Puts( 5, 0, FONT0, sizeof(msg_KCV), (UINT8 *)msg_KCV );
	  
	  memset( buf, 0x00, sizeof(buf) );
	  PED_TripleDES( imek, 8, buf, kcv );
	  LIB_DispHexByte( 5, 5,  kcv[0] );
	  LIB_DispHexByte( 5, 8,  kcv[1] );
	  LIB_DispHexByte( 5, 11, kcv[2] );

	  if( !LIB_WaitKeyYesNo( 6, 0 ) )
	    return( FALSE );
//	  if( LIB_WaitKey() == 'x' )
//	    return( FALSE );
	  
	  // final encrypted eIMEK = TDES( IMEK(16), XCSN(16) )
	  //			     XCSN(16) =	CSN[0], 0x01, CSN[1], 0x23, CSN[2], 0x45, CSN[3], 0x67
	  //					CSN[4], 0x89, CSN[5], 0xAB, CSN[6], 0xCD, CSN[7], 0xEF
	  PED_TripleDES( XCSN, 16, imek, imek1 );	// imek1 = eIMEK
	  
	  memmove( buf, imek1, 16 );			// eIMEK(16)
	  memmove( &buf[16], kcv, 8 );			// KCV(8)
	  OS_FLS_PutData( F_ADDR_CL_IMEK, CL_IMEK_LEN+CL_IMEK_KCV_LEN, buf );	// save (eIMEK+KCV) to FLASH
	  
//	  LIB_DumpHexData( 0, 0, 16, imek );
	  goto ST;
	  }
	else
	  {
	  // IAEK1:		->	IAEK2:			->	IAEK
	  // ................		................
	  // xxxxxxxxxxxxxxxx		xxxxxxxxxxxxxxxx
	  // xxxxxxxxxxxxxxxx		xxxxxxxxxxxxxxxx
	  //
	  // KCV1=			KCV2=				KCV=
	  // (Y/N)?			(Y/N)?				(Y/N)?
	  //

	  // get current KCV of IAEK
	  LIB_LCD_Cls();
	  LIB_LCD_Puts( 0, 0, FONT0, sizeof(msg_IAEK), (UINT8 *)msg_IAEK );
	  LIB_LCD_Puts( 5, 0, FONT0, sizeof(msg_KCV), (UINT8 *)msg_KCV );
	  OS_FLS_GetData( F_ADDR_CL_IAEK_KCV, CL_IAEK_KCV_LEN, kcv );
	  LIB_DispHexByte( 5, 5,  kcv[0] );
	  LIB_DispHexByte( 5, 8,  kcv[1] );
	  LIB_DispHexByte( 5, 11, kcv[2] );
	  LIB_LCD_Puts( 6, 0, FONT0, sizeof(msg_UPDATE), (UINT8 *)msg_UPDATE );
	  if( !LIB_WaitKeyYesNo( 6, 6 ) )
	    goto ST;

//	  OS_FLS_GetData( F_ADDR_CL_IAEK, CL_IAEK_LEN, buf );
//	  PED_TripleDES2( XCSN, 16, buf, iaek1 );
//	  LIB_DumpHexData( 0, 0, 16, iaek1 );
SET_IAEK1:
	  // --- IAEK1 ---
	  LIB_LCD_Cls();
	  LIB_LCD_Puts( 0, 0, FONT0, sizeof(msg_IAEK1), (UINT8 *)msg_IAEK1 );
	  LIB_LCD_Puts( 1, 0, FONT0, sizeof(msg_ruler), (UINT8 *)msg_ruler );
	  memset( buf, 0x00, sizeof(buf) );
	  while(1)
	       {
	       if( LIB_GetAlphaNumDigits( 2, 0, FONT0, 16, buf ) )
	         {
	         if( buf[0] != 16 )
	           continue;
	         
		 for( i=0; i<8; i++ )
		    iaek1[i] = LIB_ascw2hexb( &buf[1+i*2] ); // convert to hex format
		 break;
	         }
	       else
	         return( FALSE );
	       }
	  
	  memset( buf, 0x00, sizeof(buf) );
	  while(1)
	       {
	       if( LIB_GetAlphaNumDigits( 3, 0, FONT0, 16, buf ) )
	         {
	         if( buf[0] != 16 )
	           continue;
	         
		 for( i=0; i<8; i++ )
		    iaek1[i+8] = LIB_ascw2hexb( &buf[1+i*2] ); // convert to hex format
		 break;
	         }
	       else
	         return( FALSE );
	       }

	  // KCV1 = TDES( 8*0x00, IAEK1 )
	  LIB_LCD_Puts( 5, 0, FONT0, sizeof(msg_KCV1), (UINT8 *)msg_KCV1 );
	  
	  memset( buf, 0x00, sizeof(buf) );
	  PED_TripleDES( iaek1, 8, buf, kcv );
	  LIB_DispHexByte( 5, 5,  kcv[0] );
	  LIB_DispHexByte( 5, 8,  kcv[1] );
	  LIB_DispHexByte( 5, 11, kcv[2] );

	  if( !LIB_WaitKeyYesNo( 6, 0 ) )
	    goto SET_IAEK1;
//	  if( LIB_WaitKey() == 'x' )
//	    return( FALSE );
//	  LIB_DumpHexData( 0, 0, 16, imek1 );

SET_IAEK2:
	  // --- IAEK2 ---
	  LIB_LCD_Cls();
	  LIB_LCD_Puts( 0, 0, FONT0, sizeof(msg_IAEK2), (UINT8 *)msg_IAEK2 );
	  LIB_LCD_Puts( 1, 0, FONT0, sizeof(msg_ruler), (UINT8 *)msg_ruler );
	  memset( buf, 0x00, sizeof(buf) );
	  while(1)
	       {
	       if( LIB_GetAlphaNumDigits( 2, 0, FONT0, 16, buf ) )
	         {
	         if( buf[0] != 16 )
	           continue;
	         
		 for( i=0; i<8; i++ )
		    iaek2[i] = LIB_ascw2hexb( &buf[1+i*2] ); // convert to hex format
		 break;
	         }
	       else
	         return( FALSE );
	       }
	  
	  memset( buf, 0x00, sizeof(buf) );
	  while(1)
	       {
	       if( LIB_GetAlphaNumDigits( 3, 0, FONT0, 16, buf ) )
	         {
	         if( buf[0] != 16 )
	           continue;
	         
		 for( i=0; i<8; i++ )
		    iaek2[i+8] = LIB_ascw2hexb( &buf[1+i*2] ); // convert to hex format
		 break;
	         }
	       else
	         return( FALSE );
	       }

	  // KCV2 = TDES( 8*0x00, IAEK2 )
	  LIB_LCD_Puts( 5, 0, FONT0, sizeof(msg_KCV2), (UINT8 *)msg_KCV2 );
	  
	  memset( buf, 0x00, sizeof(buf) );
	  PED_TripleDES( imek2, 8, buf, kcv );
	  LIB_DispHexByte( 5, 5,  kcv[0] );
	  LIB_DispHexByte( 5, 8,  kcv[1] );
	  LIB_DispHexByte( 5, 11, kcv[2] );

	  if( !LIB_WaitKeyYesNo( 6, 0 ) )
	    goto SET_IAEK2;
//	  if( LIB_WaitKey() == 'x' )
//	    return( FALSE );
//	  LIB_DumpHexData( 0, 0, 16, imek2 );

	  // final plaintext IAEK = IAEK1 xor IAEK2
	  for( i=0; i<16; i++ )
	     iaek[i] = iaek1[i]^iaek2[i];

	  // final KCV of IAEK
	  LIB_LCD_Cls();
	  LIB_LCD_Puts( 0, 0, FONT0, sizeof(msg_IMEK), (UINT8 *)msg_IMEK );
	  LIB_LCD_Puts( 5, 0, FONT0, sizeof(msg_KCV), (UINT8 *)msg_KCV );
	  
	  memset( buf, 0x00, sizeof(buf) );
	  PED_TripleDES( iaek, 8, buf, kcv );
	  LIB_DispHexByte( 5, 5,  kcv[0] );
	  LIB_DispHexByte( 5, 8,  kcv[1] );
	  LIB_DispHexByte( 5, 11, kcv[2] );

	  if( !LIB_WaitKeyYesNo( 6, 0 ) )
	    return( FALSE );	  
//	  if( LIB_WaitKey() == 'x' )
//	    return( FALSE );
	  
	  // final encrypted eIAEK = TDES( IAEK(16), XCSN(16) )
	  //			     XCSN(16) =	CSN[0], 0x01, CSN[1], 0x23, CSN[2], 0x45, CSN[3], 0x67
	  //					CSN[4], 0x89, CSN[5], 0xAB, CSN[6], 0xCD, CSN[7], 0xEF
	  PED_TripleDES( XCSN, 16, iaek, iaek1 );	// imek1 = eIMEK
	  
	  memmove( buf, iaek1, 16 );			// eIMEK(16)
	  memmove( &buf[16], kcv, 8 );			// KCV(8)
	  OS_FLS_PutData( F_ADDR_CL_IAEK, CL_IAEK_LEN+CL_IAEK_KCV_LEN, buf );	// save (eIAEK+KCV) to FLASH

//	  LIB_DumpHexData( 0, 0, 16, iaek );
	  goto ST;
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: read or write TSC type from/to FS.
// INPUT   : flag	- 0=read, 1=write
//	     type	- 0=CAP, 1=REG type (for write)
// OUTPUT  : type	- 0=CAP, 1=REG type (for read)
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	POST_SetTscType( UINT32 flag, UINT32 *type )
{
UINT32	result = FALSE;
UINT32	bytes = 0;
UINT32	current = 0;
FILE	*fh;
UINT8	sbuf[8];
UINT8	dbuf[8];


	api_fs_select( MEDIA_FLASH );
	api_fs_init();
	
	fh = api_fs_open( "tsc.ini", 0 );
	if( !fh )
	  {
	  // 1'st time process...
	  
	  api_fs_create( "tsc.ini", 0 );
	  fh = api_fs_open( "tsc.ini", 0 );
	  if( !fh )
	    return( FALSE );
	  
	  if( flag == 0 )	// read
	    {
	    sbuf[0] = 'C';
	    bytes = api_fs_write( fh, sbuf, 1 );
	    
	    *type = 0;
	    result = TRUE;
	    }
	  else			// write
	    {
	    if( *type == 0 )
	      sbuf[0] = 'C';
	    else
	      sbuf[0] = 'R';

	    bytes = api_fs_write( fh, sbuf, 1 );
	    if( bytes == 1 )
	      result = TRUE;
	    }
	  
	  goto EXIT;
	  }
	
	
	// sub-sequence process...
	
	bytes = api_fs_read( fh, dbuf, 1 );	// read current setting
	if( bytes == 1 )
	  {
//	  LIB_DispHexByte( 1, 0, dbuf[0] );
//	  LIB_WaitKey();
	  switch( dbuf[0] )
	        {
	        case 'C':
	             current = 0;
	             break;
	             
	        case 'R':
	             current = 1;
	             break;
	             
	        default:
	             current = 0;
	             break;
	        }
	  
	  if( flag == 0 )	// read(0) or write(1)?
	    {
//	    LIB_DispHexByte( 5, 0, current );
	    *type = current;
	    result = TRUE;
	    }
	  else
	    {
	    api_fs_seek( fh, 0 );
	    
	    if( *type == 0 )
	      sbuf[0] = 'C';
	    else
	      sbuf[0] = 'R';
	    bytes = api_fs_write( fh, sbuf, 1 );
	    if( bytes == 1 )
	      result = TRUE;
	    }
	  }

EXIT:
	api_fs_close( fh );
	
	return( result );
}

// ---------------------------------------------------------------------------
void	POST_SetDefaultTscType( void )
{
	POST_SetTscType( 0, &os_TSC_RESIST );
//	LIB_DispHexByte( 5, 3, os_TSC_RESIST );
//	LIB_WaitTime(200);
}

// ---------------------------------------------------------------------------
// FUNCTION: 87 - Setup TSC type (capacitive or registive)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	SYS_FUNC_SetTscType( void )
{
UINT32	result;
UINT32	type[1];
UINT8	msg_SETUP_TSC[]	= {"SETUP TSC"};
UINT8	msg_1_CAP[]	= {"1-CAPACITIVE"};
UINT8	msg_2_REG[]	= {"2-REGISTIVE"};
UINT8	select;

 
	if( LIB_EnterPassWord( 3, 1, "2872" ) != TRUE )	// MAX_PED_PSW_CNT
	  return( FALSE );

	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_SETUP_TSC), (UINT8 *)msg_SETUP_TSC );
	
	result = POST_SetTscType( 0, &os_TSC_RESIST );	// read current TSC setting

	LIB_LCD_Puts( 2, 0, FONT0, sizeof(msg_1_CAP), (UINT8 *)msg_1_CAP );
	LIB_LCD_Puts( 3, 0, FONT0, sizeof(msg_2_REG), (UINT8 *)msg_2_REG );

SELECT:
	if( os_TSC_RESIST == 0 )
	  LIB_LCD_Puts( 0, strlen(msg_SETUP_TSC), FONT0, strlen("  -CAP"), (UINT8 *)"  -CAP" );
	else
	  LIB_LCD_Puts( 0, strlen(msg_SETUP_TSC), FONT0, strlen("  -REG"), (UINT8 *)"  -REG" );
	
	do{
	  select = LIB_WaitKey();
	  if( select == 'x' )
	    {
	    result = POST_SetTscType( 1, &os_TSC_RESIST );	// write new TSC setting
	    return( result );	// end of process
	    }
	  }while( (select != '1') && (select != '2') && (select != 'y') );
	  
	if( select == '1' )
	  os_TSC_RESIST = 0;	// CAP
	else
	  os_TSC_RESIST = 1;	// RES
	
	goto SELECT;
}

// ---------------------------------------------------------------------------
// FUNCTION: 89 - Setup AS350 sub-model ID. (S1 or S1NS)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	SYS_FUNC_SetSubModelID( void )
{
UINT8	msg_S1[] =		{"S1"};
UINT8	msg_S1NS[] =		{"S1NS"};
UINT8	msg_UNKNOWN[] =		{"UNKNOWN"};
UINT32	status = TRUE;
UINT16	flag;
UINT32	IntState;


	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "9119" ) != TRUE )
	  return( FALSE );
	
	// read flag from backup FLASH
	OS_FLS_GetData( F_ADDR_S1_SUB_MODEL_ID, S1_SUB_MODEL_ID_LEN, (UINT8 *)&flag );
	
	while(1)
	     {
	     if( flag == SMID_S1 )
	       LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_S1), msg_S1 );
	     else if( flag == SMID_S1NS )
	            LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_S1NS), msg_S1NS );
	          else
	            LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_UNKNOWN), msg_UNKNOWN );
	     
	     if( LIB_WaitKey() == 'x' )
	       break;
	     
	     if( flag == SMID_S1 )
	       flag = SMID_S1NS;
	     else if( flag == SMID_S1NS )
	            flag = SMID_S1;
	          else
	            flag = SMID_S1;
	     
	     IntState = BSP_DisableInterrupts( BSP_INT_MASK );
	     
	     status = OS_FLS_PutData( F_ADDR_S1_SUB_MODEL_ID, S1_SUB_MODEL_ID_LEN, (UINT8 *)&flag );
	     
	     BSP_RestoreInterrupts( IntState );
	     }
	
	if( status && (flag == SMID_S1) )
	  os_TSC_ENABLED = TRUE;
	  
	if( status && (flag == SMID_S1NS) )
	  os_TSC_ENABLED = FALSE;
	  
	return( status );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Check sub-model id is S1NS or not.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE	- YES
//	     FALSE	- NO
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_IsSubModel_S1NS( void )
{
UINT16	id;


	OS_FLS_GetData( F_ADDR_S1_SUB_MODEL_ID, S1_SUB_MODEL_ID_LEN, (UINT8 *)&id );
	if( id == SMID_S1NS )
	  return( TRUE );
	else
	  return( FALSE );
}
#endif

// ---------------------------------------------------------------------------
#if	0
inline	static	void  DIAG_InitDefaultKEK( UCHAR *kek )
{
      kek[0] = 0xEA;
      kek[1] = 0xC7;
      kek[2] = 0x49;
      kek[3] = 0x0D;
      kek[4] = 0x5D;
      kek[5] = 0xEC;
      kek[6] = 0x51;
      kek[7] = 0x54;
      kek[8] = 0x16;
      kek[9] = 0x40;
      kek[10]= 0xB6;
      kek[11]= 0xCD;
      kek[12]= 0x08;
      kek[13]= 0x13;
      kek[14]= 0x2C;
      kek[15]= 0xEA;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Decrypt master key by using default KEK.
// INPUT   : emkey - encrypted master key. (V, 16 bytes)
// OUTPUT  : mkey  - decrypted master key. (V, 16 bytes)
// RETURN  : TRUE  - OK.
//           FALSE - failed.
// ---------------------------------------------------------------------------
#if	0
void  DIAG_DecryptMKEY( UCHAR *emkey, UCHAR *mkey )
{
UCHAR kek[16];

      // init default KEK
      DIAG_InitDefaultKEK( kek );

      // Decrypt default MKEY by default Key Encryption Key (KEK)
      PED_TripleDES2( kek, 16, emkey, mkey );
      
      memset( kek, 0x00, sizeof(kek) );
}
#endif

// ---------------------------------------------------------------------------
// pflag = 1, to print out.
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_VerifyTMK( UINT32 pflag, UINT8 dhn )
{
UINT8	i, j;
UINT8	result;
UINT8	buf[PEDS_MSKEY_SLOT_LEN2];
UINT8	mkey[PEDS_MSKEY_SLOT_LEN2];
UINT32	FlashAddr = F_ADDR_PED_ETMK_01;

UINT8	row = 0;
UINT8	col = 0;
UINT8	rr = 0;
UINT8	temp[8];
UINT8	tkcv[8];
UINT8	msg_VERIFY[] =	{"VERIFY TMK"};		// verify
UINT8	msg_DASH[] =	{"-- -- --  -- -- --"};
UINT8	msg_MORE[] =	{"(more)"};
UINT8	msg_END[]  =	{"(end)"};
UINT8	dbuf[64];
UINT32	redraw = TRUE;


	memset( dbuf, 0x20, sizeof(dbuf) );
		
	if( !pflag )
	  {
	  LIB_LCD_Cls();
	  LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_VERIFY), msg_VERIFY );
	  }
	
	row = 2;
	col = 0;
	
	// retrieve ETMK
	for( i=0; i<MAX_PEDS_MKEY_CNT; i++ )
	   {
	   if( (row == 2) || (row >= 6) )
	     {
	     if( row >= 6 )
	       {
	       if( !pflag )
	       	 {
	       	 if( col >= 17 )
	       	   {
	           LIB_LCD_Puts( 7, 0, FONT0, strlen(msg_MORE), msg_MORE );
	           LIB_WaitKey();
	           
	           row = 2;
	           col = 0;
	           redraw = TRUE;
	           }
	         }
	       }
	     
	     if( !pflag && (col == 0) && redraw )
	       {
	       rr = 2;
	       for( j=0; j<5; j++ )
	          LIB_LCD_Puts( rr++, 0, FONT0, strlen(msg_DASH), msg_DASH );
	       
	       redraw = FALSE;
	       }
	     }
	   
	   FLASH_ReadData( (void *)FlashAddr+(i*PEDS_MSKEY_SLOT_LEN2), buf, PEDS_MSKEY_SLOT_LEN2 );
	   if( buf[0] == PEDS_MSKEY_LEN ) // MKEY available? (in FLASH)
	     {
VERIFY:
	     // Restore default MKEY by default Key Encryption Key (KEK)
	     mkey[0] = buf[0];
	     DIAG_DecryptMKEY( &buf[1], &mkey[1] );
	     
	     // Restore KCV
	     memmove( &mkey[PEDS_MSKEY_SLOT_LEN], &buf[PEDS_MSKEY_SLOT_LEN], 3 );
	     
	     // verify KCV
	     memset( temp, 0x00, 8 );
	     PED_TripleDES( &mkey[1], 8, temp, tkcv );
	     if( (tkcv[0] == mkey[17]) && (tkcv[1] == mkey[18]) && (tkcv[2] == mkey[19]) )
	       {
	       if( col >= 17 )
	         {
	         if( pflag )	// print out?
	           {
	           dbuf[0] = 19;	// length
	           dbuf[19] = 0x0a;	// line feed
	           api_prt_putstring( dhn, FONT0, dbuf );
	           
	           memset( dbuf, 0x20, sizeof(dbuf) );
	           }
	         
	         col = 0;
	         row += 1;
	         }
	       
	       if( !pflag )
	         LIB_DispHexByte( row, col, tkcv[0] );
	       else
	       	 {
	       	 LIB_hex2asc( 1, &tkcv[0], temp );
	       	 dbuf[col+1] = temp[1];
	       	 dbuf[col+2] = temp[2];
	       	 }
	       col += 3;
	       
	       if( !pflag )
	         LIB_DispHexByte( row, col, tkcv[1] );
	       else
	       	 {
	       	 LIB_hex2asc( 1, &tkcv[1], temp );
	       	 dbuf[col+1] = temp[1];
	       	 dbuf[col+2] = temp[2];
	       	 }
	       col += 3;
	       
	       if( !pflag )
	         LIB_DispHexByte( row, col, tkcv[2] );
	       else
	       	 {
	       	 LIB_hex2asc( 1, &tkcv[2], temp );
	       	 dbuf[col+1] = temp[1];
	       	 dbuf[col+2] = temp[2];
	       	 }
	       	 
	       col += 4;
	       }
	     }
	   else
	     {
	     // special checking for SINGLE TMK
	     if( i == 0 )
	       {
	       FLASH_ReadData( (void *)F_ADDR_PED_ETMK, buf, PEDS_MSKEY_SLOT_LEN2 );
	       if( buf[0] == PEDS_MSKEY_LEN ) // MKEY available?
	         goto VERIFY;
	       }
	     }
	   }
	
	if( !pflag )
	  {
	  LIB_LCD_Puts( 7, 0, FONT0+attrCLEARWRITE, strlen(msg_END), msg_END );
	  LIB_WaitKey();
	  }
	else
	  {
	  dbuf[0] = 19;		// length
	  dbuf[19] = 0x0a;	// line feed
	  api_prt_putstring( dhn, FONT0, dbuf );
	  
	  dbuf[0] = 7;
	  dbuf[1] = 0x0a;
	  dbuf[2] = '(';
	  dbuf[3] = 'e';
	  dbuf[4] = 'n';
	  dbuf[5] = 'd';
	  dbuf[6] = ')';
	  dbuf[7] = 0x0a;
	  api_prt_putstring( dhn, FONT0, dbuf );
	  }

	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To clear CA Public Key data element to specified pattern.
// INPUT   : address - begin address to write.
//           length  - length of the data element.
//           data    - the data element.
// OUTPUT  : none.
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
#if	0
UCHAR DIAG_ClearRamDataKEY( ULONG address, ULONG length, UCHAR pattern )
{
#if	0
API_SRAM pSram[1];

	pSram->StPage = SRAM_PAGE_KEY;
	pSram->StAddr = address;
	pSram->Len = length;
	return( api_sram_PageClear( (UINT8 *)pSram, pattern ) );
#endif

#if	0
UCHAR	*pSramMemory = (UCHAR *)SRAM_BASE_EMV_KEY;

	return( api_sram_clear( pSramMemory+address, length, pattern ) );
#endif

	OS_SECM_ClearData( address, length, pattern );	// 2015-05-18
	
	return( apiOK );
}
#endif

// ------------------------------------------------------------
// FUNCTION: save TMK to Flash memory in encrypted format.
// INPUT   : etmk  - encrypted terminal master key. (L-V-KCV)
//	     cnt   - total number of keys.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ------------------------------------------------------------
#if	0
UCHAR	DIAG_PutPinPadTMK_RAM( UCHAR *etmk, UCHAR cnt )
{
UCHAR	i;
ULONG	status;
ULONG	DataSize = 64*1024;	// 64KB, sizeof FLASH backup area
ULONG	FlashAddr = FLASH_BACKUP_BASE_ADDR;;

	
	status = apiFailed;
	
	os_pSwap = (UCHAR *)BSP_Malloc( DataSize );	// allocate 64KB for memory swap
	if( os_pSwap )
	  {
	  // read FLASH data to RAM
	  FLASH_ReadData( (void *)FlashAddr, os_pSwap, DataSize );
	  
	  // update ETMKs in RAM
	  memmove( os_pSwap+(F_ADDR_PED_ETMK_01-FLASH_BACKUP_BASE_ADDR), etmk, cnt*PEDS_MSKEY_SLOT_LEN2 );

	  // 2014-09-12, copy 1'st new ETMK to old single ETMK
	  memmove( os_pSwap+(F_ADDR_PED_ETMK-FLASH_BACKUP_BASE_ADDR), etmk, PEDS_MSKEY_SLOT_LEN2 );
	  
	  status = apiOK;
	  }

	return( (UCHAR)status );
}
#endif

// ------------------------------------------------------------
// FUNCTION: save TMK to Flash memory in encrypted format.
// INPUT   : etmk  - encrypted terminal master key. (L-V-KCV)
//	     cnt   - total number of keys.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ------------------------------------------------------------
#if	0
UCHAR	DIAG_PutPinPadTMK_ROM( UCHAR *etmk, UCHAR cnt )
{
ULONG	status;
ULONG	DataSize = 64*1024;	// 64KB, sizeof FLASH backup area
ULONG	FlashAddr = FLASH_BACKUP_BASE_ADDR;

	
	status = apiFailed;
	  
	  // erase FLASH backup sector
	  if( FLASH_EraseSector( FlashAddr ) )
	    {
	    // write RAM data to FLASH
	    if( FLASH_WriteData( (void *)FlashAddr, os_pSwap, DataSize ) )
	      status = apiOK;
	    }
	    
	  BSP_Free( os_pSwap );

	return( (UCHAR)status );
}
#endif

// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_LoadTMK( void )
{
UINT8	msg_DOWNLOAD[] =	{"DOWNLOAD TMK"};	// download
//UINT8	msg_ENTER_TO_START[] =	{"ENTER TO START..."};
UINT8	msg_PROCESSING[] =	{"PROCESSING..."};

UINT8	port = 0;	// COM0
UINT8	tout = 30;	// 30 seconds

UCHAR result;
UCHAR dhn_aux;
UCHAR buf[40];
UCHAR mkey[PEDS_MSKEY_SLOT_LEN2];  // LEN(1) TMK(16) KCV(3)
UINT  len;
UCHAR dhn_tim;
UINT  tick1, tick2;

API_AUX	pAux[1];

UCHAR	i;
UCHAR	index;
UCHAR	temp[8];
UCHAR	kcv[8];
UCHAR	etmk[MAX_PEDS_MKEY_CNT][PEDS_MSKEY_SLOT_LEN2];
UCHAR	tmk_cnt = 0;

UCHAR	AtLeast1Cnt = 0;	// at least one of the keys is updated ok


	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_DOWNLOAD), msg_DOWNLOAD );
//	LIB_LCD_Puts( 3, 0, FONT0, strlen(msg_ENTER_TO_START), msg_ENTER_TO_START );
//	LIB_WaitKey();
//	LIB_LCD_Cls();
	LIB_LCD_Puts( 3, 0, FONT0, strlen(msg_PROCESSING), msg_PROCESSING );
	
      // 1. open CP AUX port

      // --- enable hardware port ---
      api_aux_close( 0 );
      
      pAux->Mode = auxDLL;
      pAux->Baud = COM_9600 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
      pAux->Tob = 10;	// 100ms
      pAux->Tor = 100;	// 1 sec
      pAux->Acks = 1;	// ACKS=1
      pAux->Resend = 0;	// RESEND=0
      dhn_aux = api_aux_open( port, (UINT8 *)pAux );

      if( (dhn_aux == apiOutOfService) || (dhn_aux == apiOutOfLink) )
        return( apiFailed );

      result = apiFailed;

      // clear all current MS keys
      for( i=0; i<MAX_PEDS_MKEY_CNT; i++ )
         {
         DIAG_ClearRamDataKEY( ADDR_PED_MKEY_01+(i*PEDS_MSKEY_SLOT_LEN2), PEDS_MSKEY_SLOT_LEN2, 0x00 );
         DIAG_ClearRamDataKEY( ADDR_PED_SKEY_01+(i*PEDS_MSKEY_SLOT_LEN), PEDS_MSKEY_SLOT_LEN, 0x00 );
         etmk[i][0] = 0;
         }

      dhn_tim = api_tim_open( 100 ); // 1 sec
      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );

      // 2. waiting for KIT download
      while(1)
           {
           if( LIB_GetKeyStatus == apiReady )
             {
             if( LIB_WaitKey() == 'x' )
               {
               result = 0xff;
               break;
               }
             }

           if( AtLeast1Cnt == 1 )	// 2016-12-12, for only ONE TMK solution
             {
             api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
             if( (tick2 - tick1) >= 3 ) // timeout?
               {
               result = apiOK;
               break;
               }
             }

           api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
           if( (tick2 - tick1) >= tout ) // timeout?
             break;

           if( api_aux_rxready( dhn_aux, buf ) == apiReady )
             {
             api_tim_gettick( dhn_tim, (UCHAR *)&tick1 ); // reset time tick
             
             len = buf[0] + buf[1]*256;
             if( len != 34 )
               break; // invalid LEN

             api_aux_rxstring( dhn_aux, buf );
//           if( buf[2] != 0x01 )
	     if( buf[2] > MAX_PEDS_MKEY_CNT )
               break; // invalid ID

	     if( buf[2] == 0x00 )	// EOF?
	       {
	       result = apiOK;
	       break;
	       }
	     
	     tmk_cnt++;
	     
	     index = buf[2] - 1;      // 2011-01-11, CUP TMK key index = 00..nn
             
//             // save encrypted key to Flash key slot
//             buf[2] = PED_MSKEY_LEN;
//             PED_PutPinPadTMK( &buf[2] );

             // Restore EMKEY by default Key Encryption Key (KEK)
             DIAG_DecryptMKEY( &buf[3], &mkey[1] );
             mkey[0] = PEDS_MSKEY_LEN;
             memmove( &mkey[17], &buf[2+17], 3 ); // KCV

//	TL_DumpHexData( 0, 0, 17, mkey );

             // Verify KCV and save TMK
             memset( temp, 0x00, 8 );
             PED_TripleDES( &mkey[1], 8, temp, kcv );

//	TL_DumpHexData( 0, 3, 8, kcv );

             if( (kcv[0] == buf[2+17]) && (kcv[1] == buf[2+18]) && (kcv[2] == buf[2+19]) )
               {
               // temp save to array buffer
               buf[2] = PEDS_MSKEY_LEN;
               memmove( &etmk[index][0], &buf[2], PEDS_MSKEY_SLOT_LEN2 );
               
               // save plaintext TMK to SRAM key slot
//             apk_WriteRamDataKEY( ADDR_PED_MKEY_01+(index*PEDS_MSKEY_SLOT_LEN2), PEDS_MSKEY_SLOT_LEN2, mkey );

               AtLeast1Cnt++;	 // 2016-11-25
               }               
             }
           }

      api_tim_close( dhn_tim );
      api_aux_close( dhn_aux );
      
      // finally save ETMKs to backup FLASH memory
      if( result == apiOK )
        {
        DIAG_PutPinPadTMK_RAM( &etmk[0][0], tmk_cnt );
        DIAG_PutPinPadTMK_ROM( &etmk[0][0], tmk_cnt );
        }

      memset( mkey, 0x00, sizeof(mkey) );
      
      return( result );	
}
#endif

// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_DeleteTMK( void )
{
UINT8	result = apiFailed;
UINT8	msg_DEL_TMK[] =		{"DELETE TMK"};		// delete all TMKs
UINT8	etmk[MAX_PEDS_MKEY_CNT][PEDS_MSKEY_SLOT_LEN2];
UINT8	tmk_cnt = MAX_PEDS_MKEY_CNT;


//	LIB_LCD_Cls();
//	if( LIB_EnterPassWord( 1, 1, "69977978" ) != TRUE )
//	  return( apiFailed );
//	  
//	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_DEL_TMK), msg_DEL_TMK );
//	if( LIB_WaitKeyYesNo( 2, 0 ) )
//	  {
	  memset( etmk, 0xFF, sizeof(etmk) );
	  if( DIAG_PutPinPadTMK_RAM( &etmk[0][0], tmk_cnt ) == apiOK )
	    result = DIAG_PutPinPadTMK_ROM( &etmk[0][0], tmk_cnt );
	  
//	  return( apiOK );
//	  }
//	else
//	  return( apiFailed );

	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 48 - Setup or Verify TMKs.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	SYS_FUNC_LoadTMK( void )
{
UINT32	result;
UINT32	flag;
UINT8	key;
UINT8	msg_TMK[] =		{"TMK"};
UINT8	msg_1_VERIFY[] =	{"1-VERIFY"};		// verify & show KCV
UINT8	msg_2_DOWNLOAD[] =	{"2-DOWNLOAD"};		// inject new TMKs
UINT8	msg_OK[] =		{"OK"};


	if( LIB_EnterPassWord( 1, 1, "87977996" ) != TRUE )
	  return(FALSE);

	while(1)
	{
	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_TMK), msg_TMK );
	LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_1_VERIFY), msg_1_VERIFY );
	LIB_LCD_Puts( 3, 0, FONT0, strlen(msg_2_DOWNLOAD), msg_2_DOWNLOAD );
	
	flag = TRUE;
	while(flag)
	     {
	     if( LIB_GetKeyStatus() == apiReady )
	       {
	       key = LIB_WaitKey();
	       switch( key )
	             {
	             case '1':
	             	  
	             	  result = DIAG_FUNC_VerifyTMK(0,0);
	             	  flag = FALSE;
	             	  break;
	             
	             case '2':
	             
	             	  result = DIAG_FUNC_LoadTMK();
	             	  
	             	  if( result == 0xFF )
	             	    {
	             	    flag = FALSE;
	             	    break;
	             	    }
	             	  if( result == apiOK )
	             	    {
	             	    LIB_BUZ_Beep1();
	             	    
	             	    LIB_LCD_Puts( 7, 0, FONT0, strlen(msg_OK), msg_OK );
	             	    LIB_WaitTime(50);
	             	    
	             	    flag = FALSE;
	             	    break;
	             	    }
	             	  else
	             	    return( FALSE );
#if	0	             
	             case '0':	// delete all TMKs (implicit function)
	             	
	             	  result = DIAG_FUNC_DeleteTMK();

	             	  if( result == 0xFF )
	             	    {
	             	    flag = FALSE;
	             	    break;
	             	    }
	             	  if( result == apiOK )
	             	    return( TRUE );
	             	  else
	             	    return( FALSE );
#endif
	             case 'x':	// quit
	             
	             	  return( TRUE );
	             	  break;
	             }
	       }
	     }
	}
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 90 - Key Management System.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
UINT32	SYS_FUNC_KeyManagementSystem( void )
{
	UINT8	buffer[8];
	UINT8	result;
	UINT8	status;
	UINT8	start;


	// dual control password
	PED_SetSensitiveServiceTime(TRUE);
	if(!PED_DualPasswordControl())
	{
		PED_SetSensitiveServiceTime(FALSE);
		return FALSE;
	}

	start = 0;
	while(1)
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_SELECT), (UINT8 *)os_msg_SELECT);

		buffer[0] = 1;	// starting row number
		buffer[1] = 14;	// max lcd row cnt
		buffer[2] = 13;	// list items
		buffer[3] = 19;	// item length
		buffer[4] = 0;	// offset of LEN field in item
		buffer[5] = FONT1;
		result = LIB_ListBox(start, &buffer[0], (UINT8 *)&os_list_ADMIN_MODE[0], 60); // wait for selection (T.O.=60sec)

		switch(result)
		{
			case 0xff: // aborted
				PED_SetSensitiveServiceTime(FALSE);
				return(TRUE);

			case 0x00: // VERIFY KEY STATUS
				status = PED_VerifyKeyStatus();
				break;

			case 0x01:	// CHANGE KEY MODE
				status = PED_ADMIN_ChangeKeyMode();
				break;

			case 0x02:	// Setup KPK for TDES
				status = PED_ADMIN_SetKeyBlockProtectionKey(1);
				break;

			case 0x03:	// Setup KPK for AES
				status = PED_ADMIN_SetKeyBlockProtectionKey(2);
				break;

			case 0x04:	// MASTER/SESSION KEY
				status = PED_ADMIN_LoadMasterSessionKey();
				break;

			case 0x05:	// DUKPT KEY
				status = PED_ADMIN_LoadDukptKey();
				break;

			case 0x06:	// ISO4_KEY
				status = PED_ADMIN_LoadISO4KEY();
				break;

			case 0x07:	// ACC_DEK
				status = PED_ADMIN_LoadAccDEK();
				break;

			case 0x08:	// FPE Key
				status = PED_ADMIN_LoadFPEKey();
				break;

			case 0x09:	// CAPK
				status = PED_ADMIN_LoadCAPK();
				break;

			case 0x0A:	// DELETE PED KEY
				status = PED_ADMIN_DeletePedKey();
				break;

			case 0x0B:	// SETUP SRED
				status = PED_ADMIN_SetupSRED();
				break;

			case 0x0C:	// RESET SECM
				status = PED_ADMIN_ResetSecureMemory();
				PED_SetSensitiveServiceTime(FALSE);
				return status;
				
			default:
				status = apiFailed;
				break;
		}

		if(status == apiFailed)
		{
			PED_SetSensitiveServiceTime(FALSE);
			return FALSE;
		}

		start = result;
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: 91 - Show Tamper Status.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	0
UINT32	SYS_FUNC_TamperStatus( void )
{
UINT16	cnt=1;;
UINT32	data;
UINT8	dd;

	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "1379" ) != TRUE )
	  return(FALSE);

	LIB_DispHexWord( 0, 0, cnt );
	BSP_Delay_n_ms( 2000 );	// 2014-12-04
	
	do{
	  LIB_LCD_Cls();
	  LIB_DispHexWord( 0, 0, cnt++ );
	
	  data = OS_SEC_Status(0);	// 2014-12-04

	  dd = (data >> 24) & 0xFF;
	  LIB_DispHexByte( 2, 0, dd );
	  dd = (data >> 16) & 0xFF;
	  LIB_DispHexByte( 2, 3, dd );
	  dd = (data >> 8) & 0xFF;
	  LIB_DispHexByte( 2, 6, dd );
	  dd = data & 0xFF;
	  LIB_DispHexByte( 2, 9, dd );
	
	  if( data & 0x01 )
	    {
	    // show error target
	    dd = (data >> 8) & 0x0E;
	    if( dd & 0x02 )
	      LIB_LCD_Puts( 4, 0, FONT0, 3, "SW0" );
	    if( dd & 0x04 )
	      LIB_LCD_Puts( 4, 4, FONT0, 3, "SW1" );
	    if( dd & 0x08 )
	      LIB_LCD_Puts( 4, 8, FONT0, 4, "WIRE" );
	      
	    if( data & 0x11FE )
	       LIB_LCD_Puts( 4, 13, FONT0, 6, "OTHERS" );
	    
	    POST_DisplayTestResult( FALSE );
	    }
	  else
	    POST_DisplayTestResult( TRUE );
	
	  dd= LIB_WaitTimeAndKey( 200 );	// refresh every 1+2 seconds

	  }while( dd == 255 );
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
#if	0
UINT32	SYS_FUNC_TamperStatus( void )
{
UINT16	cnt=1;;
UINT32	data;
UINT32	data2;
UINT8	dd;
UINT8	dd2;


	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "1379" ) != TRUE )
	  return(FALSE);

	LIB_LCD_Cls();
	
	// SecDsbl
	data2 = OS_SEC_Status(1);	// read SecDsbl
	
	dd2 = (data2 >> 24) & 0xFF;
	LIB_DispHexByte( 2, 0, dd2 );
	dd2 = (data2 >> 16) & 0xFF;
	LIB_DispHexByte( 2, 3, dd2 );
	dd2 = (data2 >> 8) & 0xFF;
	LIB_DispHexByte( 2, 6, dd2 );
	dd2 = data2 & 0xFF;
	LIB_DispHexByte( 2, 9, dd2 );

	// SecCfg1
	data2 = OS_SEC_Status(2);	// read SecCfg1
	
	dd2 = (data2 >> 24) & 0xFF;
	LIB_DispHexByte( 3, 0, dd2 );
	dd2 = (data2 >> 16) & 0xFF;
	LIB_DispHexByte( 3, 3, dd2 );
	dd2 = (data2 >> 8) & 0xFF;
	LIB_DispHexByte( 3, 6, dd2 );
	dd2 = data2 & 0xFF;
	LIB_DispHexByte( 3, 9, dd2 );
	
	// SecCfg2
	data2 = OS_SEC_Status(3);	// read SecCfg2
	
	dd2 = (data2 >> 24) & 0xFF;
	LIB_DispHexByte( 3, 13, dd2 );
	dd2 = (data2 >> 16) & 0xFF;
	LIB_DispHexByte( 3, 16, dd2 );
	dd2 = (data2 >> 8) & 0xFF;
	LIB_DispHexByte( 3, 19, dd2 );
	dd2 = data2 & 0xFF;
	LIB_DispHexByte( 3, 22, dd2 );
	  
	do{
//	  LIB_LCD_Cls();
	  LIB_DispHexWord( 0, 0, cnt++ );
	  
	  LIB_LCD_ClearRow( 4, 4, FONT0 );

	  // SecSta
	  data = OS_SEC_Status(0);	// read SecSta

	  dd = (data >> 24) & 0xFF;
	  LIB_DispHexByte( 4, 0, dd );
	  dd = (data >> 16) & 0xFF;
	  LIB_DispHexByte( 4, 3, dd );
	  dd = (data >> 8) & 0xFF;
	  LIB_DispHexByte( 4, 6, dd );
	  dd = data & 0xFF;
	  LIB_DispHexByte( 4, 9, dd );

	  if( data & 0x01 )
	    {
	    // show error target
	    dd = (data >> 8) & 0x0E;
	    if( dd & 0x02 )
	      LIB_LCD_Puts( 5, 0, FONT0, 3, "SW0" );
	    if( dd & 0x04 )
	      LIB_LCD_Puts( 5, 4, FONT0, 3, "SW1" );
	    if( dd & 0x08 )
	      LIB_LCD_Puts( 5, 8, FONT0, 4, "WIRE" );
	      
	    if( data & 0x11FE )
	       LIB_LCD_Puts( 5, 13, FONT0, 6, "OTHERS" );
	    
	    POST_DisplayTestResult( FALSE );
	    }
	  else
	    POST_DisplayTestResult( TRUE );
	
	  dd= LIB_WaitTimeAndKey( 200 );	// refresh every 1+2 seconds

	  }while( dd == 255 );
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: to check detect tamper flag in flash.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - to detect tamper status.
//           FALSE - not to detect tamper status.
// ---------------------------------------------------------------------------
#if	0
UINT32	POST_CheckDetectTamperFlag( void )
{
UINT16	flag;

	
	OS_FLS_GetData( F_ADDR_TAMPER_DETECT, TAMPER_DETECT_LEN, (UINT8 *)&flag );
	if( flag == FLAG_DETECT_TAMPER_ON )
	  return( TRUE );
	else
	  return( FALSE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 97 - Turn on or off TAMPER detection in boot sequence.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	SYS_FUNC_DetectTamper( void )
{
UINT8	msg_ON[] =		{"ON"};
UINT8	msg_OFF[] =		{"OFF"};
UINT32	status = TRUE;
UINT16	flag;
UINT16	flag2;
UINT32	IntState;


	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "9111" ) != TRUE )
	  return( FALSE );
	
	// read flag from backup FLASH
	OS_FLS_GetData( F_ADDR_TAMPER_DETECT, TAMPER_DETECT_LEN, (UINT8 *)&flag );
	
	while(1)
	     {
	     if( flag == FLAG_DETECT_TAMPER_OFF )
	       LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_OFF), msg_OFF );
	     else
	       LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_ON), msg_ON );
	     
	     if( LIB_WaitKey() == 'x' )
	       break;
	     
	     if( flag == FLAG_DETECT_TAMPER_OFF )
	       flag = FLAG_DETECT_TAMPER_ON;
	     else
	       flag = FLAG_DETECT_TAMPER_OFF;
	     
	     IntState = BSP_DisableInterrupts( BSP_INT_MASK );
	     
	     status = OS_FLS_PutData( F_ADDR_TAMPER_DETECT, TAMPER_DETECT_LEN, (UINT8 *)&flag );
	     
	     if( flag == FLAG_DETECT_TAMPER_OFF )
	       {
	       flag2 = FLAG_REF_F97_OFF;
	       status = OS_FLS_PutData( F_ADDR_REF_F97, REF_F97_LEN, (UINT8 *)&flag2 );
	       }
	     else
	       {
	       flag2 = FLAG_REF_F97_ON;
	       status = OS_FLS_PutData( F_ADDR_REF_F97, REF_F97_LEN, (UINT8 *)&flag2 );
	       }
	     
	     BSP_RestoreInterrupts( IntState );
	     }
	
	return( status );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 98 - Enable or disable PIN entry frequency limit.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
#ifndef	_BOOT_FROM_POST0_

UINT32	SYS_FUNC_PinLimit( void )
{
UINT8	flag;
UINT8	key;
UINT8	msg_ON[]	= {"ON "};
UINT8	msg_OFF[]	= {"OFF"};


	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "641308" ) != TRUE )
	  return( FALSE );
	
	OS_SECM_GetData( ADDR_PED_PIN_FREQ_LIMIT, PED_PIN_FREQ_LIMIT_LEN, &flag );
	if( flag == 0 )
	  LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_OFF), msg_OFF );
	else
	  LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_ON), msg_ON );
	
	while(1)
	     {
	     key = LIB_WaitKey();
	     if( key == 'x' )
	       break;
	       
	     if( key == '0' )
	       {
	       flag = 0;
	       OS_SECM_PutData( ADDR_PED_PIN_FREQ_LIMIT, PED_PIN_FREQ_LIMIT_LEN, &flag );
	       LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_OFF), msg_OFF );
	       }
	       
	     if( key == '1' )
	       {
	       flag = 1;
	       OS_SECM_PutData( ADDR_PED_PIN_FREQ_LIMIT, PED_PIN_FREQ_LIMIT_LEN, &flag );
	       LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_ON), msg_ON );
	       }
	     }
	
	return( TRUE );
}
#endif
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 99 - Recover application in case terminal was compromised.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	SYS_FUNC_RecoverAP( void )
{
	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "9111" ) != TRUE )
	  return( FALSE );
	
//	OS_SECM_SetAppStatus();
	SYS_FUNC_SetAppStatus();	// 2018-03-19
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
#if	0
void	DIAG_SramTestWrite( void )
{
UINT32	i;
UINT32	j;
UINT32	PageCnt;
UINT32	PageSize;	// 1 page = 64KB
UINT8	*pAddr;
//UINT8	*pSramMem = 0;
UINT8	OrgData;

UINT32	SRAM_MemorySize;
UINT32	pSramMem;


	OS_SRAM_GetConfig( &pSramMem, &SRAM_MemorySize );

	    PageCnt = SRAM_MemorySize / (64*1024);
	    
	    // Write
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_W), (UINT8 *)os_msg_W );
	    PageSize = 0;
	    pAddr = (UINT8 *)pSramMem;
	    
	    for( i=0; i<PageCnt; i++ )
	       {	       
	       OrgData = i;
	       
	       for( j=0; j<(64*1024); j++ )
	          {
	          if( (PageSize & 0xFFFF) == 0xFFFF )	// show address at 64K boundary
	            LIB_DispHexLong( 2, sizeof(os_msg_RW_ADDRESS)+1, PageSize );	// logical
	          
	          *pAddr++ = OrgData++;
	          PageSize++;
	          }
	       }
}
#endif

// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_SramTestRead( void )
{
UINT32	i;
UINT32	j;
UINT32	PageCnt;
UINT32	PageSize;	// 1 page = 64KB
UINT8	*pAddr;
//UINT8	*pSramMem = 0;
UINT8	OrgData;

UINT32	SRAM_MemorySize;
UINT32	pSramMem;


	OS_SRAM_GetConfig( &pSramMem, &SRAM_MemorySize );

	    PageCnt = SRAM_MemorySize / (64*1024);
//	    PageCnt = (1024*1024) / (64*1024);
	    
	    // Read
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_R), (UINT8 *)os_msg_R );
	    PageSize = 0;
	    pAddr = (UINT8 *)pSramMem;
	    
	    for( i=0; i<PageCnt; i++ )
	       {	       
	       OrgData = i;
	       
	       for( j=0; j<(64*1024); j++ )
	          {
	          if( (PageSize & 0xFFFF) == 0xFFFF )	// show address at 64K boundary
	            LIB_DispHexLong( 2, sizeof(os_msg_RW_ADDRESS)+1, PageSize );	// logical
	          
	          if( *pAddr++ != OrgData++ )
	            {
	            return( FALSE );
	            }
	          PageSize++;
	          }
	       }
	       
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 10 - Test SRAM.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_TestSram( void )
{
UINT32	i;
UINT32	j;
UINT32	PageSize;	// 1 page = 64KB
UINT32	PageCnt;
UINT32	Result = FALSE;
//UINT8	*pSramMem = 0;
UINT8	*pAddr;
UINT8	OrgData;

UINT32	SRAM_MemorySize;
UINT32	pSramMem;

	
	OS_SRAM_GetConfig( &pSramMem, &SRAM_MemorySize );
	
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_SRAM), (UINT8 *)os_msg_SRAM );
	
	if( (os_MultiBootMode == OP_MODE_MFG) || (os_MultiBootMode == OP_MODE_BIN) )
	  goto SRAM_ST;
	
	LIB_LCD_Puts( 2, 0, FONT0, sizeof(os_msg_CLEAR_MEMORY), (UINT8 *)os_msg_CLEAR_MEMORY );
	if( LIB_WaitKeyYesNo( 2, 12 ) == FALSE )	// clear memory?
	  {
	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_RW), (UINT8 *)os_msg_RW );
	  
	  // No, do non-desctructive testing
	  //
	  // Algorithm: 1. writing 0x55
	  //            2. writing 0xAA
	  //            3. recover, byte by byte
	  PageSize = 0;
	  pAddr = (UINT8 *)pSramMem;
	  
	  for( i=0; i<SRAM_MemorySize; i++ )
	     {
	     if( (PageSize & 0xFFFF) == 0xFFFF )	// show address at 64K boundary
	       LIB_DispHexLong( 2, sizeof(os_msg_RW_ADDRESS)+1, PageSize );	// logical
	     
	     OrgData = *pAddr;	// save original data
	     
	     *pAddr = 0x55;		// writing 0x55
	     if( *pAddr != 0x55 )
	       break;
	       
	     *pAddr = 0xAA;		// writing 0xAA
	     if( *pAddr != 0xAA )
	       break;
	       
	     *pAddr++ = OrgData;	// recover original data
	     PageSize++;
	     }
	  
	  Result = TRUE;
	  }
	else
	  {
	  // Yes, do destructive testing
	  //
	  // Algorithm: 1. writing cyclic patterns 0x0N..0xFF to each 64KB page, N=beginning page counter
	  //            2. reading back and comparing patterns
	  //            3. clear all memory to NULL
	  if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "1376" ) == TRUE )
	    {
SRAM_ST:
	    // --- PCI Security Process ---
	    // Reset application & boot status, erase all SRAM data
	    OS_SEC_UnlockAPB();	// 2014-11-19, unlock APB
	    
	    OS_SECM_ResetBootStatus();
	    OS_SECM_ResetAppStatus();	// 2017-07-27
	    
	    OS_SEC_LockAPB();	// 2015-01-14, lock APB
	    
	    DIAG_SramTestWrite();
	    
	    PageCnt = SRAM_MemorySize / (64*1024);
	    
/*    
	    // Write
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_W), (UINT8 *)os_msg_W );
	    PageSize = 0;
	    pAddr = pSramMem;
	    
	    for( i=0; i<PageCnt; i++ )
	       {	       
	       OrgData = i;
	       
	       for( j=0; j<(64*1024); j++ )
	          {
	          if( (PageSize & 0xFFFF) == 0xFFFF )	// show address at 64K boundary
	            LIB_DispHexLong( 2, sizeof(os_msg_RW_ADDRESS)+1, PageSize );	// logical
	          
	          *pAddr++ = OrgData++;
	          PageSize++;
	          }
	       }
*/

	   if( DIAG_SramTestRead() != TRUE )
	     return( FALSE );

/*	       
	    // Read
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_R), (UINT8 *)os_msg_R );
	    PageSize = 0;
	    pAddr = pSramMem;
	    
	    for( i=0; i<PageCnt; i++ )
	       {	       
	       OrgData = i;
	       
	       for( j=0; j<(64*1024); j++ )
	          {
	          if( (PageSize & 0xFFFF) == 0xFFFF )	// show address at 64K boundary
	            LIB_DispHexLong( 2, sizeof(os_msg_RW_ADDRESS)+1, PageSize );	// logical
	          
	          if( *pAddr++ != OrgData++ )
	            {
	            return( FALSE );
	            }
	          PageSize++;
	          }
	       }
*/

	    // Erase
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_E), (UINT8 *)os_msg_E );
	    PageSize = 0;
	    pAddr = (UINT8 *)pSramMem;
	    
	    for( i=0; i<PageCnt; i++ )
	       {
	       for( j=0; j<(64*1024); j++ )
	          {
	          if( (PageSize & 0xFFFF) == 0xFFFF )	// show address at 64K boundary
	            LIB_DispHexLong( 2, sizeof(os_msg_RW_ADDRESS)+1, PageSize );	// logical
	          
	          *pAddr++ = 0;
	          PageSize++;
	          }
	       }
	    
	    Result = TRUE;
	    }
	  }
	
	return( Result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 11 - Test FALSH.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_TestFlash( void )
{
UINT32	IntState = 0;
UINT32	NoEraseSize = FLASH_NO_ERASE_SIZE;	// erase APP & CFONT
UINT32	StartAddr = ADDR_APP1;
UINT8	apid;
UINT16	flag;


	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_FLASH), (UINT8 *)os_msg_FLASH );

	if( os_MultiBootMode == OP_MODE_MFG )
	  goto FLASH_ST;
	
	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "1376" ) == TRUE )
	  {
	  LIB_LCD_Puts( 2, 0, FONT0, strlen("NORMAL TEST"), (UINT8 *)"NORMAL TEST" );
	  if( LIB_WaitKeyYesNo( 2, strlen("NORMAL TEST") ) )
	    {
	    LIB_LCD_ClearRow( 2, 1, FONT0 );
	    goto FLASH_ST;
	    }
	  
	  LIB_LCD_ClearRow( 1, 2, FONT0 );
	  
	  LIB_LCD_Puts( 2, 0, FONT0, strlen("ERASE FONT"), (UINT8 *)"ERASE FONT" );
	  if( LIB_WaitKeyYesNo( 2, strlen("ERASE FONT") ) )
	    {
	    LIB_LCD_ClearRow( 2, 1, FONT0 );
	    
	    if( LIB_EnterPassWord( 1, 1, "6731" ) == TRUE )
	      {
	      StartAddr = ADDR_FULL_BIG5_CODE_TABLE;
	      NoEraseSize = FLASH_POST0_AND_DATA_SIZE;	// erase CFONT only
	      }
	    else
	      return( FALSE );
	    }

	  LIB_LCD_ClearRow( 1, 2, FONT0 );
	  
	  LIB_LCD_Puts( 2, 0, FONT0, strlen("ERASE TMK"), (UINT8 *)"ERASE TMK" );
	  if( LIB_WaitKeyYesNo( 2, strlen("ERASE TMK") ) )
	    {
	    LIB_LCD_ClearRow( 2, 1, FONT0 );
	    
	    if( LIB_EnterPassWord( 1, 1, "6731" ) == TRUE )
	      {
	      if( DIAG_FUNC_DeleteTMK() != apiOK )
	      	return( FALSE );
	      }
	    else
	      return( FALSE );
	    }

	  LIB_LCD_ClearRow( 1, 2, FONT0 );
	  
	  LIB_LCD_Puts( 2, 0, FONT0, strlen("RESET TO MFG"), (UINT8 *)"RESET TO MFG" );
	  if( LIB_WaitKeyYesNo( 2, strlen("RESET TO MFG") ) )
	    {
	    LIB_LCD_ClearRow( 2, 1, FONT0 );
	    
	    if( LIB_EnterPassWord( 1, 1, "6731" ) == TRUE )
	      {
	      LIB_LCD_Puts( 2, 0, FONT0, strlen("PROCESSING..."), (UINT8 *)"PROCESSING..." );
	      
//	      EraseSector( FLASH_BACKUP_BASE_ADDR );	// erase all data in the backup area of FLASH, 2015-10-06
	      
	      LIB_LCD_ClearRow( 2, 1, FONT0 );
	      
	      StartAddr = ADDR_APP0;
	      goto FLASH_ST;
	      }
	    }
	    
FLASH_ST:
	  // --- PCI Security Process ---
	  // Reset application & boot status, erase all SRAM data 
	  if( (StartAddr == ADDR_APP1) || (StartAddr == ADDR_APP0) )
	    {
	    OS_SEC_UnlockAPB();	// 2014-11-19, unlock APB
	    
	    OS_SECM_ResetBootStatus();
	    OS_SECM_ResetAppStatus();
	    
	    OS_SEC_LockAPB();	// 2015-01-14, lock APB
	    }
	  
	  BSP_Delay_n_ms( 100 );
	  
	  IntState = BSP_DisableInterrupts( BSP_INT_MASK );
	  
	  // Erase
	  DCacheOff();			//
	  os_ConsoleFlag = 0;		// close console channel
	  DIAG_FlashCheckType();	// check flash type
	
	  DIAG_FlashErasePages( StartAddr, NoEraseSize );	// erase flash memory
	  
	  // Write & Read
	  ProgramUnlock();		// unlock flash
	  if( DIAG_FlashTestRW( StartAddr, NoEraseSize ) == FALSE )
	    {
	    ProgramLock();
	    DCacheOn();
	    
	    BSP_RestoreInterrupts( IntState );
	    
	    return( FALSE );
	    }
	    
	  // Erase
//	  DCacheOff();			//
//	  os_ConsoleFlag = 0;		// close console channel
//	  DIAG_FlashCheckType();	// check flash type

	  DIAG_FlashErasePages( StartAddr, NoEraseSize );	// erase flash memory
	  }
	else
	  return( FALSE );

	// 2014-05-06, force to reset APID = 0
	OS_FLS_GetData( F_ADDR_APP_ID, APP_ID_LEN, (UCHAR *)&apid );
	if( (apid != 0x00) && (apid != 0xFF) )
	  {
	  apid = 0;
	  OS_FLS_PutData( F_ADDR_APP_ID, APP_ID_LEN, (UCHAR *)&apid );
	  }
#ifdef	_F97_ENABLED_
	OS_FLS_GetData( F_ADDR_TAMPER_DETECT, TAMPER_DETECT_LEN, (UINT8 *)&flag );
	if( flag != FLAG_DETECT_TAMPER_ON )
	  {
	  flag = FLAG_DETECT_TAMPER_ON;
	  OS_FLS_PutData( F_ADDR_TAMPER_DETECT, TAMPER_DETECT_LEN, (UCHAR *)&flag );
	  }
	
	OS_FLS_GetData( F_ADDR_REF_F97, REF_F97_LEN, (UINT8 *)&flag );
	if( flag != FLAG_REF_F97_OFF )
	  {
	  flag = FLAG_REF_F97_OFF;
	  OS_FLS_PutData( F_ADDR_REF_F97, REF_F97_LEN, (UCHAR *)&flag );
	  }
#endif
	ProgramLock();
	DCacheOn();
	
	BSP_RestoreInterrupts( IntState );
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
#if	0
#define	NFL_LEN		2*1024
#define	NFL_CNT		64
#define	NFL_SIZE	NFL_LEN*NFL_CNT
#define	NFL_ST_ADDR	0
#define	NFL_ST_PAGE	0

//UINT8	NandFlashBlockBuffer[2*1024];
UINT8	*NandFlashBlockBuffer	= (UINT8 *)0x21100000;
UINT8	*NandFlashBlockBuffer2	= (UINT8 *)(0x21100000 + NFL_SIZE);
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Erase NAND FLASH pages. (in M3)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_TestNandFlash( void )
{
API_FLASH_ADDR pAddr;
API_FLASH pFls;

UINT32	result;
UINT32	i;
//UINT32	SectorAddr;
//UINT32	SectorStartAddr;
//UINT32	SectorLastAddr;
//UINT32	Sectors;
//UINT32	ByteCount;
UINT32	Size;
UINT8	data;

	
//	LIB_LCD_Cls();
//	LIB_DispHexByte( 0, 0, api_flash_TypeSelect( 1 ) );

	api_flash_TypeSelect( 1 );
	
	pAddr.Type = 1;
	if( api_flash_PageSelect( 0, &pAddr ) == apiOK )
	  {
//	  SectorStartAddr = pAddr.StAddr;
//	  SectorLastAddr  = pAddr.EndAddr;
//	  
//	  LIB_DispHexLong( 2, 0, SectorStartAddr );
//	  LIB_DispHexLong( 3, 0, SectorLastAddr );

	  // ---------------------------------------------------------
	  // Erase block
	  // ---------------------------------------------------------
	  Size = 0;
	  pFls.Type = 1;
	  pFls.StPage = NFL_ST_PAGE;
	  pFls.StAddr = NFL_ST_ADDR;
	  pFls.Len    = NFL_LEN;

	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen("E"), (UINT8 *)"E" );
	   
	  for( i=0; i<NFL_CNT; i++ )
	     {
	     LIB_DispHexLong( 2, sizeof(os_msg_ERASE_FLASH)+1, Size );	// logical
	     
	     result = api_flash_PageClear( pFls, 0xFF );
	     if( result == apiOK )
	       {
	       pFls.StAddr += NFL_LEN;
	       Size += NFL_LEN;
	       }
	     else
	       break;
	     }

	  if( result != apiOK )
	    return( FALSE );

//	  BSP_Delay_n_ms(500);
//	  LIB_DispHexByte( 4, 3, result );
//	  LIB_WaitKey();

	  // ---------------------------------------------------------
	  // Blank Check
	  // ---------------------------------------------------------
	  Size = 0;
	  pFls.Type = 1;
	  pFls.StPage = NFL_ST_PAGE;
	  pFls.StAddr = NFL_ST_ADDR;
	  pFls.Len    = NFL_LEN;
	  
	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen("B"), (UINT8 *)"B" );
	  
	  memset( NandFlashBlockBuffer, 0x00, NFL_SIZE );
	  for( i=0; i<NFL_CNT; i++ )
	     {
	     LIB_DispHexLong( 2, sizeof(os_msg_ERASE_FLASH)+1, Size );	// logical
	     
	     result = api_flash_PageRead( pFls, &NandFlashBlockBuffer[Size] );
	     if( result == apiOK )
	       {
	       if( LIB_memcmpc( &NandFlashBlockBuffer[Size], 0xFF, NFL_LEN ) == 0 )
	       	 {
	       	 pFls.StAddr += NFL_LEN;
	         Size += NFL_LEN;
	         }
	       else
	       	 {
//	       	 LIB_DispHexLong( 5, 0, pFls.StAddr );
//	       	 result = apiFailed;
	         break;
	         }
	       }
	     }

	  if( result != apiOK )
	    return( FALSE );
  
//	  BSP_Delay_n_ms(500);
//	  LIB_DispHexByte( 5, 9, result );
//	  LIB_WaitKey();

	  // ---------------------------------------------------------
	  // Write block
	  // ---------------------------------------------------------
	  Size = 0;
	  pFls.Type = 1;
	  pFls.StPage = NFL_ST_PAGE;
	  pFls.StAddr = NFL_ST_ADDR;
	  pFls.Len    = NFL_LEN;
	  
	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen("W"), (UINT8 *)"W" );
	  
	  data = 0;
	  for( i=0; i<NFL_SIZE; i++ )
	     NandFlashBlockBuffer[i] = data++;
	     
	  for( i=0; i<NFL_CNT; i++ )
	     {
	     LIB_DispHexLong( 2, sizeof(os_msg_ERASE_FLASH)+1, Size );	// logical
	     
	     result = api_flash_PageWrite( pFls, &NandFlashBlockBuffer[Size] );
	     if( result == apiOK )
	       {
	       pFls.StAddr += NFL_LEN;
	       Size += NFL_LEN;
	       }
	     else
	       {
//	       BSP_Delay_n_ms(500);
//	       LIB_DispHexLong( 6, 0, pFls.StAddr );
	       break;
	       }
	     }

	  if( result != apiOK )
	    return( FALSE );

//	  BSP_Delay_n_ms(500);
//	  LIB_DispHexByte( 6, 9, result );
//	  LIB_WaitKey();

	  // ---------------------------------------------------------
	  // Verify block
	  // ---------------------------------------------------------
	  Size = 0;
	  pFls.Type = 1;
	  pFls.StPage = NFL_ST_PAGE;
	  pFls.StAddr = NFL_ST_ADDR;
	  pFls.Len    = NFL_LEN;
	  
	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen("R"), (UINT8 *)"R" );
	  
	  memset( NandFlashBlockBuffer2, 0x00, NFL_SIZE );
	  for( i=0; i<NFL_CNT; i++ )
	     {
	     LIB_DispHexLong( 2, sizeof(os_msg_ERASE_FLASH)+1, Size );	// logical
	     
	     result = api_flash_PageRead( pFls, &NandFlashBlockBuffer2[Size] );
	     if( result == apiOK )
	       {
	       if( LIB_memcmp( &NandFlashBlockBuffer2[Size], &NandFlashBlockBuffer[Size], NFL_LEN ) == 0 )
	         {
	         pFls.StAddr += NFL_LEN;
	         Size += NFL_LEN;
	         }
	       else
	         {
	         result = apiFailed;
	         break;
	         }
	       }
	     else
	       break;
	     }

	  if( result != apiOK )
	    return( FALSE );

	  // ---------------------------------------------------------
	  // Erase block
	  // --------------------------------------------------------- 
	  Size = 0;
	  pFls.Type = 1;
	  pFls.StPage = NFL_ST_PAGE;
	  pFls.StAddr = NFL_ST_ADDR;
	  pFls.Len    = NFL_LEN;

	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen("E"), (UINT8 *)"E" );
	   
	  for( i=0; i<NFL_CNT; i++ )
	     {
	     LIB_DispHexLong( 2, sizeof(os_msg_ERASE_FLASH)+1, Size );	// logical
	     
	     result = api_flash_PageClear( pFls, 0xFF );
	     if( result == apiOK )
	       {
	       pFls.StAddr += NFL_LEN;
	       Size += NFL_LEN;
	       }
	     else
	       break;
	     }

	  if( result != apiOK )
	    return( FALSE );

//	  BSP_Delay_n_ms(500);
//	  LIB_DispHexByte( 4, 3, result );
//	  LIB_WaitKey();
	  
	  // ---------------------------------------------------------
	  // Blank Check
	  // ---------------------------------------------------------
	  Size = 0;
	  pFls.Type = 1;
	  pFls.StPage = NFL_ST_PAGE;
	  pFls.StAddr = NFL_ST_ADDR;
	  pFls.Len    = NFL_LEN;
	  
	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen("B"), (UINT8 *)"B" );
	  
	  memset( NandFlashBlockBuffer, 0x00, NFL_SIZE );
	  for( i=0; i<NFL_CNT; i++ )
	     {
	     LIB_DispHexLong( 2, sizeof(os_msg_ERASE_FLASH)+1, Size );	// logical
	     
	     result = api_flash_PageRead( pFls, &NandFlashBlockBuffer[Size] );
	     if( result == apiOK )
	       {
	       if( LIB_memcmpc( &NandFlashBlockBuffer[Size], 0xFF, NFL_LEN ) == 0 )
	       	 {
	       	 pFls.StAddr += NFL_LEN;
	         Size += NFL_LEN;
	         }
	       else
	       	 {
//	       	 LIB_DispHexLong( 5, 0, pFls.StAddr );
//	       	 result = apiFailed;
	         break;
	         }
	       }
	     }

	  if( result == apiOK )
	    return( TRUE );
	  else
	    return( FALSE );
	  }
}
#endif

// ---------------------------------------------------------------------------
#if	0
UINT32	FUNC_InitNandFlash( UINT32 force )
{
UINT32	i, j;
UINT32	ByteNo;
UINT32	BitNo;
UINT8	buf[NAND_FLASH_BBT_LEN];


	memset( os_FLASH_INFO, 0x00, 4+1024 );
//	goto XXX;
	
	OS_FLS_GetData( F_ADDR_NAND_FLASH_BBT_FLAG, NAND_FLASH_BBT_FLAG_LEN, buf );
	
	if( force || (buf[0] != 0x34) || (buf[1] != 0x12) )
	  {
	  OS_FLASH_PageInit(1);	// init and read ID+BBT
	  
	  // test only
//	  os_FLASH_INFO[4+0] = 1;
//	  os_FLASH_INFO[4+7] = 1;
//	  os_FLASH_INFO[4+10] = 1;
//	  os_FLASH_INFO[4+13] = 1;
//	  os_FLASH_INFO[4+16] = 1;
	  
	  // store 1024 bytes to 128 bytes
	  memset( buf, 0x00, sizeof(buf) );
	  for( i=0; i<1024; i++ )
	     {
	     if( os_FLASH_INFO[4+i] != 0 )
	       {
	       ByteNo = i/8;
	       BitNo  = i%8;
	       
	       buf[ByteNo] |= (1 << BitNo);
	       }
	     }
	  
//	  memmove( g_buf, buf, NAND_FLASH_BBT_LEN );
	  OS_FLS_PutData( F_ADDR_NAND_FLASH_BBT, NAND_FLASH_BBT_LEN, buf );		// save iBBT
	  
	  buf[0] = 0x34;
	  buf[1] = 0x12;
	  OS_FLS_PutData( F_ADDR_NAND_FLASH_BBT_FLAG, NAND_FLASH_BBT_FLAG_LEN, buf );	// set iBBT ready flag
	  }
	else
	  {
//XXX:
	  OS_FLASH_PageInit_NAND0();	// init and read ID only
	  
	  // restore 1024 bytes from 128 bytes
	  OS_FLS_GetData( F_ADDR_NAND_FLASH_BBT, NAND_FLASH_BBT_LEN, buf );
	  
	  // test only
//	  memmove( g_buf, buf, NAND_FLASH_BBT_LEN );
//	  buf[0] = 0x81;
//	  buf[1] = 0x24;
//	  buf[2] = 0x01;
	  
	  for( i=0; i<128; i++ )
	     {
	     for( j=0; j<8; j++ )
	        {
	        if( (buf[i] >> j) & 0x01 )
	          os_FLASH_INFO[4+(i*8)+j] = 0x01;
	        }
	     }
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 31 - Test NAND FALSH (on M3).
//		  os_FLASH_INFO[4+1024] = ID[4] + INVALID_BLOCK_TABLE[1024]
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_TestNandFlash( void )
{
UINT8	msg_1_ID[]	=	{"1-READ ID & BBT"};
UINT8	msg_2_RW_TEST[]	=	{"2-R/W TEST BLOCK 0"};	
UINT8	msg_3_RST_ID[]	=	{"3-RESET BBT"};
UINT32	result;
UINT32	i;
UINT32	flag;
UINT32	len;
UINT8	row;
UINT8	col;
UINT8	abuf[8];


	LIB_LCD_Puts( 0, 0, FONT0, strlen("NAND FLASH"), (UINT8 *)"NAND FLASH" );
	
	LIB_LCD_Puts( 2, 0, FONT0, strlen(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING );
	
	FUNC_InitNandFlash(0);

START:
	LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_1_ID), (UINT8 *)msg_1_ID );
	LIB_LCD_Puts( 3, 0, FONT0, strlen(msg_2_RW_TEST), (UINT8 *)msg_2_RW_TEST );
	LIB_LCD_Puts( 4, 0, FONT0, strlen(msg_3_RST_ID), (UINT8 *)msg_3_RST_ID );
	
	switch( LIB_WaitKey() )
	      {
	      case '1':		// read ID
	      	   
	      	   LIB_LCD_ClearRow( 2, 3, FONT0 );
	      	   goto READ_ID;
	      	   
	      case '2':		// erase, write, read test (destructive test for BLOCK 0)
	      	
	      	   LIB_LCD_ClearRow( 2, 3, FONT0 );
	      	   
	      	   if( LIB_EnterPassWord( 1, 1, "6357" ) != TRUE )
	      	     return( FALSE );
	      	   
	      	   result = DIAG_TestNandFlash();
	      	   api_flash_TypeSelect( 0 );
	      	   
	  	   goto EXIT;
	      
	      case '3':		// reset ID (used only when NAND flash is renewed)
	      	   
	      	   LIB_LCD_ClearRow( 2, 3, FONT0 );
	      	   
	      	   if( LIB_EnterPassWord( 1, 1, "6359" ) != TRUE )
	      	     return( FALSE );
	      	
	      	   LIB_LCD_Puts( 2, 0, FONT0, strlen(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING );
	      	   
	      	   FUNC_InitNandFlash(1);
	      	   
	      	   LIB_LCD_ClearRow( 2, 1, FONT0 );

	      	   break;
	      
	      case 'x':
	      	
	      	   return( TRUE );

	      default:
	      	   
	      	   LIB_LCD_ClearRow( 2, 3, FONT0 );
	      	   goto READ_ID;
	      }

READ_ID:
//	OS_FLASH_PageInit(1);
	
	LIB_LCD_Puts( 2, 0, FONT0, strlen("ID: "), (UINT8 *)"ID: " );
	LIB_DispHexByte( 2, strlen("ID: ")+0, os_FLASH_INFO[0] );
	LIB_DispHexByte( 2, strlen("ID: ")+3, os_FLASH_INFO[1] );
	LIB_DispHexByte( 2, strlen("ID: ")+6, os_FLASH_INFO[2] );
	LIB_DispHexByte( 2, strlen("ID: ")+9, os_FLASH_INFO[3] );
	
	LIB_WaitKey();
	
	result = TRUE;
	if( (os_FLASH_INFO[0] != 0x01) || (os_FLASH_INFO[1] != 0xF1) || (os_FLASH_INFO[2] != 0x00) || (os_FLASH_INFO[3] != 0x1D) )
	  {
	  result = FALSE;
	  goto EXIT;
	  }

	flag = TRUE;
	for( i=0; i<1024; i++ )
	   {
	   if( os_FLASH_INFO[4+i] != 0x00 )
	     {
	     flag = FALSE;
	     break;
	     }
	   }
	
	if( !flag )
	  {
//	  LIB_LCD_Puts( 3, 0, FONT0, strlen("BAD BLOCK:"), (UINT8 *)"BAD BLOCK:" );
//	  LIB_DumpHexData( 0, 4, 1024, &os_FLASH_INFO[4] );

	  LIB_LCD_Puts( 3, 0, FONT0, strlen("BAD BLOCK NUM:"), (UINT8 *)"BAD BLOCK NUM:" );
	  
	  row = 4;
	  col = 0;
	  for( i=0; i<1024; i++ )
	     {
	     if( os_FLASH_INFO[4+i] != 0x00 )
	       {
	       len = LIB_itoa( i, abuf );
	       LIB_LCD_Puts( row, col, FONT0, len, abuf );
	       
	       col += 5;
	       if( col >= 30 )
	       	 {
	       	 col = 0;
	       	 row += 1;
	       	 if( row >= 8 )
	       	   {
	       	   row = 4;
	       	   
	       	   if( LIB_WaitKey() == 'x' )
	       	     break;
	       	   LIB_LCD_ClearRow( 4, 4, FONT0 );
	       	   }
	         }
	       }
	     }
	  
//	  LIB_LCD_ClearRow( 4, 4, FONT0 );
	  LIB_WaitKey();
	  LIB_LCD_ClearRow( 4, 4, FONT0 );	// 2017-08-15
	  }

EXIT:

	if( result )
	  {
	  LIB_BUZ_Beep1();
	  LIB_LCD_Puts( 7, 0, FONT0, sizeof(os_msg_OK), (UINT8 *)os_msg_OK );
	  }
	else
	  {
	  LIB_BUZ_Beep2();
	  LIB_LCD_Puts( 7, 0, FONT0, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR );
	  }
	  	   
	LIB_WaitKey();
	  	   
	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, strlen("NAND FLASH"), (UINT8 *)"NAND FLASH" );
	goto START;
	
//	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: to fill LCM with pattern.
// INPUT   : pattern
//	     method
//	     mode	- 0=fill all, 1=fill even, 2=fill odd.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	DIAG_LCD_FillPattern( UINT8 pattern, UINT8 method, UINT8 mode )
{
UINT32	i, j;
UINT8	image[1];
//API_GRAPH dim;
API_GRAPH dim[1];

	
	for( i=0; i<MAX_LCD_X_NO; i += 8 )
	   {
	   for( j=0; j<LCD_SEGMENT_LEN*2; j++ )
	      {
	      dim->Xleft	 = j; 
	      dim->Ytop	 = i;
	      dim->Width	 = 1;
	      dim->Height = 8;
	      dim->Method = method;
	      image[0] 	 = pattern;
	      if( (mode == 0) || ((j & 1)==0) )
	        api_lcd_putgraphics( 0, (UCHAR *)dim, image );
	      }
	   }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 12 - Test Keypad Module.
//		  to test KEYPAD scan matrix. (row by row)
//		  from top to bottom, form left to right.
//		  Display: RR---- => RR****
//			   RR = row number (01..05)
//			   -  = target key to be pressed, it will turn to '*'
//				if correct stroke.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#define MAX_KBD_SCAN_LINE		5
#define MAX_KBD_RETN_LINE		4

#if	1
UINT32	DIAG_FUNC_TestPinPad( void )
{
UINT32	i, j;
UINT8	row = 0x02;	// D
UINT8	col = 0x00;	// E
UINT32	keyread;
UINT32	keydata;


	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_KEYBOARD), (UINT8 *)os_msg_KEYBOARD );
	
	LIB_OpenKeyAll();
	
	if( LIB_GetKeyStatus() == apiReady )
	  LIB_WaitKey();
	
	for( i=0; i<MAX_KBD_SCAN_LINE; i++ )
	   {
	   LIB_LCD_Puts( 2, 0, FONT0, sizeof(os_msg_KBD_DASH)-1, (UINT8 *)os_msg_KBD_DASH );
	   LIB_DispHexByte( 2, 0, row-1 );	// row #
	   
	   for( j=0; j<(MAX_KBD_RETN_LINE-1); j++ )
	      {
	      switch( row )
	            {
	            case 2:
		         if( j == 0 )
		           while( LIB_WaitKey() != '1' );
	                 if( j == 1 )
	                   while( LIB_WaitKey() != '2' );
	                 if( j == 2 )
	                   while( LIB_WaitKey() != '3' );
	                 break;
	                 
	            case 3:
		         if( j == 0 )
		           while( LIB_WaitKey() != '4' );
	                 if( j == 1 )
	                   while( LIB_WaitKey() != '5' );
	                 if( j == 2 )
	                   while( LIB_WaitKey() != '6' );
	                 break;
	                 
	            case 4:
		         if( j == 0 )
		           while( LIB_WaitKey() != '7' );
	                 if( j == 1 )
	                   while( LIB_WaitKey() != '8' );
	                 if( j == 2 )
	                   while( LIB_WaitKey() != '9' );
	                 break;
	                 
	            case 5:
		         if( j == 0 )
		           while( LIB_WaitKey() != '*' );
	                 if( j == 1 )
	                   while( LIB_WaitKey() != '0' );
	                 if( j == 2 )
	                   while( LIB_WaitKey() != '#' );
	                 break;

	            case 6:
		         if( j == 0 )
		           while( LIB_WaitKey() != 'x' );
	                 if( j == 1 )
	                   while( LIB_WaitKey() != 'n' );
	                 if( j == 2 )
	                   while( LIB_WaitKey() != 'y' );
	                 break;
	                 
	            }
	        
	      LIB_LCD_Putc( 2, col+2, FONT0, '*' );
	      LIB_BUZ_Beep1();
	      col++;	// next column
	      }
	   
	   BSP_Delay_n_ms(200);
	   
	   col = 0;	// reset column
	   row++;	// next row
	   }
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 13 - Test LCD Module.
//		1. fill horizontal line pattern. (0x55)
//		2. fill vertical line pattern.
//		3. fill black pattern. (0xFF)
//		4. fill white pattern. (0x00)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	1
UINT32	DIAG_FUNC_TestDisplay( void )
{
API_LCDTFT_PARA		para;


	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_LCM), (UINT8 *)os_msg_LCM );
	
//	BSP_Delay_n_ms( 800 );
	LIB_WaitTime(100);
	LIB_LCD_Cls();
	
//	BSP_Delay_n_ms( 800 );
	LIB_WaitTime(100);
	
	memset( &para, 0x00, sizeof(API_LCDTFT_PARA) );
	
	para.RGB = 0;
	para.Row = 0xFFFF;
	para.BG_Palette[0] = 0x00;	// BLACK
	para.BG_Palette[1] = 0x00;
	para.BG_Palette[2] = 0x00;
	api_lcdtft_clear( 0, para );

//	BSP_Delay_n_ms( 800 );
	LIB_WaitTime(100);
	
	para.RGB = 0;
	para.Row = 0xFFFF;
	para.BG_Palette[0] = 0xFF;	// RED
	para.BG_Palette[1] = 0x00;
	para.BG_Palette[2] = 0x00;
	api_lcdtft_clear( 0, para );

//	BSP_Delay_n_ms( 800 );
	LIB_WaitTime(100);
	
	para.RGB = 0;
	para.Row = 0xFFFF;
	para.BG_Palette[0] = 0x00;	// RED
	para.BG_Palette[1] = 0xFF;
	para.BG_Palette[2] = 0x00;
	api_lcdtft_clear( 0, para );
	
//	BSP_Delay_n_ms( 800 );
	LIB_WaitTime(100);
	
	para.RGB = 0;
	para.Row = 0xFFFF;
	para.BG_Palette[0] = 0x00;	// BLUE
	para.BG_Palette[1] = 0x00;
	para.BG_Palette[2] = 0xFF;
	api_lcdtft_clear( 0, para );

//	BSP_Delay_n_ms( 800 );
	LIB_WaitTime(100);
	
	LIB_LCD_Cls();
//	BSP_Delay_n_ms( 800 );
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 14 - Test Magnetic Stripe Reader. (3-track)
//		  MSR
//		  TRACK1: OK (or ERROR)
//		  TRACK2: OK (or ERROR)
//		  TRACK3: OK (or ERROR)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	1
UINT32	DIAG_FUNC_TestMsr( void )
{
UINT32	i;
UINT8	dhn_msr;
UINT8	key;
UINT8	status;
UINT8	flag;
UINT8	MsrStatus[4];
UINT8	BakMsrStatus[4];
UINT8	sbuf[2];
UINT8	dbuf[300];


	// open all tracks
	dhn_msr = api_msr_open( isoTrack1 + isoTrack2 + isoTrack3 );
	
	if( dhn_msr == apiOutOfService)
	  return( FALSE );
	  
	while(1)
	{
	LIB_LCD_Cls();
	
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_MSR), (UINT8 *)os_msg_MSR );
	LIB_LCD_Puts( 1, 0, FONT0, sizeof(os_msg_TRACK1), (UINT8 *)os_msg_TRACK1 );
	LIB_LCD_Puts( 2, 0, FONT0, sizeof(os_msg_TRACK2), (UINT8 *)os_msg_TRACK2 );
	LIB_LCD_Puts( 3, 0, FONT0, sizeof(os_msg_TRACK3), (UINT8 *)os_msg_TRACK3 );
	LIB_LCD_Puts( 7, 0, FONT0, sizeof(os_msg_VIEW_QUIT), (UINT8 *)os_msg_VIEW_QUIT );
	
	i = 0;
	flag = TRUE;
	while(flag)
	     {
	     if( LIB_GetKeyStatus() == apiReady )
	       {
	       key = LIB_WaitKey();
	       switch( key )
	             {
	             case 'y':	// view
	             
	             	  DIAG_ShowDataTracks( dhn_msr, (UINT8 *)BakMsrStatus, (UINT8 *)dbuf );
	             	  
	             	  flag = FALSE;
	             	  continue;
	             	  
	             case 'x':	// quit
	             
	             	  // close all tracks
	             	  api_msr_close( dhn_msr );
	             	  
	             	  LIB_LCD_ClearRow( 7, 1, FONT0 );
	             	  return( TRUE );
	             }
	       }
	     
	     api_msr_status( dhn_msr, 0, MsrStatus );		// get MSR status
	     if( MsrStatus[0] == msrSwiped )
	       {
	       LIB_BUZ_Beep1();
	       
	       memmove( BakMsrStatus, MsrStatus, 4 );
	       
	       // Read MSR data
	       sbuf[0] = 1; // keep data after reading
	       sbuf[1] = isoTrack1 + isoTrack2 + isoTrack3;
	       api_msr_getstring( dhn_msr, sbuf, dbuf );	// get MSR data
	       
	       // Show MSR status
	       for( i=1; i<4; i++ )
	          DIAG_ShowStatusMSR( i, MsrStatus[i] );
	       }
	       
	     } // while(flag)

        } // while(1)
	
	
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 15 - Test Smart Card Reader. (ICC*1, SAM*4)
//		  ICC0
//		  SAM1
//		  SAM2
//		  ...
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	1
UINT32	DIAG_FUNC_TestScr( void )
{
UINT8	dhn_icc1;
UINT8	dhn_sam1;
UINT8	dhn_sam2;
UINT8	dhn_sam3;
UINT8	dhn_sam4;
UINT8	key;
UINT8	status;
UINT8	flag;
UINT32	ItemNo;
UINT8	atr_icc1[34] = {0};
UINT8	atr_sam1[34] = {0};
UINT8	atr_sam2[34] = {0};
UINT8	atr_sam3[34] = {0};
UINT8	atr_sam4[34] = {0};


	// Open all devices
	dhn_icc1 = api_ifm_open( ICC1 );
	dhn_sam1 = api_ifm_open( SAM1 );
	dhn_sam2 = api_ifm_open( SAM2 );
	dhn_sam3 = api_ifm_open( SAM3 );
	dhn_sam4 = api_ifm_open( SAM4 );
	
	if( (dhn_icc1 == apiOutOfService) 
	    || (dhn_sam1 == apiOutOfService) || (dhn_sam2 == apiOutOfService)
	    || (dhn_sam3 == apiOutOfService) || (dhn_sam4 == apiOutOfService)
	  )
	  return( FALSE );

	while(1)
	{
	LIB_LCD_Cls();
	
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_ICC0), (UINT8 *)os_msg_ICC0 );	// item 1
	LIB_LCD_Puts( 1, 0, FONT0, sizeof(os_msg_SAM1), (UINT8 *)os_msg_SAM1 );	// item 2
	LIB_LCD_Puts( 2, 0, FONT0, sizeof(os_msg_SAM1), (UINT8 *)os_msg_SAM2 );	// item 3
	LIB_LCD_Puts( 3, 0, FONT0, sizeof(os_msg_SAM1), (UINT8 *)os_msg_SAM3 );	// item 4
	LIB_LCD_Puts( 4, 0, FONT0, sizeof(os_msg_SAM1), (UINT8 *)os_msg_SAM4 );	// item 5
	LIB_LCD_Puts( 7, 0, FONT0, sizeof(os_msg_VIEW_QUIT), (UINT8 *)os_msg_VIEW_QUIT );
	
	ItemNo = 0;	// default testing all devices
	
	flag = TRUE;
	while(flag)
	     {
//	     SAM_ClearATRStatus();	// 2018-09-26
	     
	     if( LIB_GetKeyStatus() == apiReady )
	       {
	       key = LIB_WaitKey();
	       switch( key )
	             {
	             case 'y':	// view
	                  
	                  if( (ItemNo == 0) || (ItemNo == 1) )
	                    DIAG_ShowDataATR( ICC1, atr_icc1 );
	                  if( (ItemNo == 0) || (ItemNo == 2) )
	                    DIAG_ShowDataATR( SAM1, atr_sam1 );
	                  if( (ItemNo == 0) || (ItemNo == 3) )
	                    DIAG_ShowDataATR( SAM2, atr_sam2 );
	                  if( (ItemNo == 0) || (ItemNo == 4) )
	                    DIAG_ShowDataATR( SAM3, atr_sam3 );
	                  if( (ItemNo == 0) || (ItemNo == 5) )
	                    DIAG_ShowDataATR( SAM4, atr_sam4 );
	                  
	                  flag = FALSE;
	                  continue;
	             
	             case 'x':	// quit
	             
	             	  // close all devices
	             	  api_ifm_close( dhn_icc1 );
	             	  api_ifm_close( dhn_sam1 );
	             	  api_ifm_close( dhn_sam2 );
	             	  api_ifm_close( dhn_sam3 );
	             	  api_ifm_close( dhn_sam4 );
	             	  
	             	  LIB_LCD_ClearRow( 7, 1, FONT0 );
	             	  return( TRUE );
	             
	             case '0':	// ICC1
	             case '1':	// SAM1
	             case '2':	// SAM2
	             case '3':	// SAM3
	             case '4':	// SAM4
	             
	                  ItemNo = key - '0' + 1;
	                  break;
	                  
	             default:
	                  ItemNo = 0;
	             }
	       }

	     if( (ItemNo == 0) || (ItemNo == 1) )	// ICC1
	       {
	       if( api_ifm_present( dhn_icc1 ) == apiReady )	// ICC1 in position?
	      	 {
	      	 // ATR
	      	 atr_icc1[0] = 0;
	      	 status = ( api_ifm_reset( dhn_icc1, 0, atr_icc1 ) );
	      	 DIAG_ShowStatusATR( ICC1, status );
	      	 }
	       else
	         continue;
	       }
	       
	     if( (ItemNo == 0) || (ItemNo == 2) )	// SAM1
	       {
	       // ATR
	       atr_sam1[0] = 0;
	       status = ( api_ifm_reset( dhn_sam1, 0, atr_sam1 ) );
	       DIAG_ShowStatusATR( SAM1, status );
	       }
	       
	     if( (ItemNo == 0) || (ItemNo == 3) )	// SAM2
	       {
	       // ATR
	       atr_sam2[0] = 0;
	       status = ( api_ifm_reset( dhn_sam2, 0, atr_sam2 ) );
	       DIAG_ShowStatusATR( SAM2, status );
	       }
	     
	     if( (ItemNo == 0) || (ItemNo == 4) )	// SAM3
	       {
	       // ATR
	       atr_sam3[0] = 0;
	       status = ( api_ifm_reset( dhn_sam3, 0, atr_sam3 ) );
	       DIAG_ShowStatusATR( SAM3, status );
	       }
	     
	     if( (ItemNo == 0) || (ItemNo == 5) )	// SAM4
	       {
	       // ATR
	       atr_sam4[0] = 0;
	       status = ( api_ifm_reset( dhn_sam4, 0, atr_sam4 ) );
	       DIAG_ShowStatusATR( SAM4, status );
	       }
	     
	     LIB_BUZ_Beep1();
	     LIB_WaitTime( 100 ); // wait 1 sec
	     DIAG_ShowStatusATR( -1, -1 );
	       
	     } // while(flag)
	  
	} // while(1)
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 16 - Test RTC module.
//		  RTC
//
//		  99/12/31 23:59:56
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	1
UINT32	DIAG_FUNC_TestRtc( void )
{
UINT8	datetime0[] = {"991231235956"};
UINT8	datetime[12];
UINT8	buffer[32];
UINT8	result;

UINT8	dhn_tim;
UINT8	dhn_rtc;
UINT16 	tick1, tick2;
UINT32	repeat = 2;


LOOP:
	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_RTC), (UINT8 *)os_msg_RTC );
	
	dhn_rtc = api_rtc_open();
//	LIB_DispHexByte( 1, 0, dhn_rtc );
//	LIB_WaitKey();
	
//	api_rtc_setdatetime( 0, datetime0 );
	api_rtc_settimezone ( dhn_rtc, GMTp8 );
	
	dhn_tim = api_tim_open(1); // 1 tick unit = 10 ms
	api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
	
	while(1)
	     {
//	     result = LIB_GetKeyStatus();
//	     if( result != apiReady )
//	       {	       
	       api_rtc_getdatetime( 0, datetime );
	       buffer[0] = datetime[0];
	       buffer[1] = datetime[1];
	       buffer[2] = '-';
	       buffer[3] = datetime[2];
	       buffer[4] = datetime[3];
	       buffer[5] = '-';
	       buffer[6] = datetime[4];
	       buffer[7] = datetime[5];
	       
	       buffer[8] = ' ';
	       
	       buffer[9] = datetime[6];
	       buffer[10]= datetime[7];
	       buffer[11]= ':';
	       buffer[12]= datetime[8];
	       buffer[13]= datetime[9];
	       buffer[14]= ':';
	       buffer[15]= datetime[10];
	       buffer[16]= datetime[11];
	       
	       LIB_LCD_Puts( 2, 0, FONT0, 17, buffer );
	       
	       api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
	       if( (tick2 - tick1) >= 1000 )
	         break;
//	       }
//	     else
//	       {
//	       if( LIB_WaitKey() == 'x' )
//	         break;
//	       }
	       
	     BSP_Delay_n_ms(50);
	     }

	while(1)
	     {
	     if( LIB_GetKeyStatus() != apiReady )
	       break;
	     LIB_WaitKey();
	     }

	api_tim_close( dhn_tim );

	if( (buffer[0]  != '0') || (buffer[1]  != '0') ||
	    (buffer[3]  != '0') || (buffer[4]  != '1') ||
	    (buffer[6]  != '0') || (buffer[7]  != '1') ||
	    (buffer[9]  != '0') || (buffer[10] != '0') ||
	    (buffer[12] != '0') || (buffer[13] != '0') ||
//	    (buffer[15] != '0') || (buffer[16] != '6') )
	    (buffer[15] != '0') || ( (buffer[16] != '5') && (buffer[16] != '6') && (buffer[16] != '7') ) )
//	  return( FALSE );
	  return( TRUE );	// TBI
	else
	  {
	  if( --repeat == 0 )
	    return( TRUE );
	  else
	    goto LOOP;
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Show current MODEM line status.
// INPUT   : dhn.
//	     type = FALSE, skip calling "api_mdm_status"
//		    TRUE , call "api_mdm_status"
//	     status - effective only if type = TRUE
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	DIAG_DispLineStatus( UINT8 dhn, UINT8 type, UINT8 status )
{
//#ifdef	_MODEM_ENABLED_

UINT8	dbuf[1];


//	     while(1)
//	     	  {
//	     	  if( LIB_GetKeyStatus() == apiReady )
//	     	    {
//	     	    LIB_WaitKey();
//	     	    break;
//	     	    }
	     	  
	     	  if( type )
	     	    api_mdm_status( dhn, dbuf );
	     	  else
	     	    dbuf[0] = status;
	     	  
	     	  switch( dbuf[0] )
	     	        {
	     	        case mdmIdle:
	     	             LIB_LCD_Puts( 3, 0, FONT0, 16, "IDLE            ");
	     	             break;
	     	             
	     	        case mdmConnecting:
	     	             LIB_LCD_Puts( 3, 0, FONT0, 16, "CONNECTING...   ");
	     	             break;
	     	             
	     	        case mdmConnected:
	     	             LIB_LCD_Puts( 3, 0, FONT0, 16, "CONNECTED OK    ");
	     	             break;
	     	             
	     	        case mdmLocalLineBusy:
	     	             LIB_LCD_Puts( 3, 0, FONT0, 16, "LOCAL LINE BUSY ");
	     	             break;
	     	             
	     	        case mdmRemoteLineBusy:
	     	             LIB_LCD_Puts( 3, 0, FONT0, 16, "REMOTE LINE BUSY");
	     	             break;
	     	             
	     	        case mdmLineDropped:
	     	             LIB_LCD_Puts( 3, 0, FONT0, 16, "LINE DROPPED    ");
	     	             break;
	     	             
	     	        case mdmFailed:
	     	             LIB_LCD_Puts( 3, 0, FONT0, 16, "FAILED          ");
	     	             break;
	     	             
	     	        case mdmRedialing:
	     	             LIB_LCD_Puts( 3, 0, FONT0, 16, "RE-DIALING...   ");
	     	             break;
	     	             
	     	        case mdmNoLine:
	     	             LIB_LCD_Puts( 3, 0, FONT0, 16, "NO LINE         ");
	     	             break;
	     	        }
//	     	  }

//#endif
}
#endif

// ---------------------------------------------------------------------------
#if	1
UINT32	DIAG_TEST_AuxPort( UINT8 dhn, UINT32 mode )
{
UINT32	i;
UINT8	buf[300];
UINT8	result;
UINT8	dhn_tim;
UINT16	tick1, tick2;


	if( !mode )
	{
	LIB_LCD_Puts( 1, 0, FONT0, sizeof(os_msg_OPEN_TEST), (UINT8 *)os_msg_OPEN_TEST );
	BSP_Delay_n_ms( 1000 );
	
	buf[0] = 1;
	buf[1] = 0;
	buf[2] = 0x55;
	api_aux_txstring( dhn, buf );

	for( i=0; i<20; i++ )
	   {
	   BSP_Delay_n_ms( 100 );
	   if( api_aux_rxready( dhn, buf ) == apiReady )
	     {
	     api_aux_rxstring( dhn, buf );
	     return( FALSE );
	     }
	   }
	api_aux_rxstring( dhn, buf );
	}

	LIB_LCD_Puts( 1, 0, FONT0, sizeof(os_msg_LOOPBACK_TEST), (UINT8 *)os_msg_LOOPBACK_TEST );
	if( !mode )
	  LIB_WaitKey();

#if	0
	buf[0] = 1;
	buf[1] = 0;
	buf[2] = 0x55;
	api_aux_txstring( dhn, buf );

	for( i=0; i<20; i++ )
	   {
	   BSP_Delay_n_ms( 100 );
	   if( api_aux_rxready( dhn, buf ) == apiReady )
	     {
	     memset( buf, 0x00, sizeof(buf) );
	     api_aux_rxstring( dhn, buf );
	     if( (buf[0] == 1) && (buf[1] == 0) && (buf[2] == 0x55) )
	       return( TRUE );
	     else
	       {
	       api_aux_rxstring( dhn, buf );
	       return( FALSE );
	       }
	     }
	   }
#endif

	// 2014-12-04, sending 256 bytes patterns (00..FF)
	buf[0] = 0x00;
	buf[1] = 0x01;
	for( i=0; i<256; i++ )
	   buf[2+i] = i;
	api_aux_txstring( dhn, buf );
	
	dhn_tim = api_tim_open(1); // 1 tick unit = 10 ms
	api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
	
	memset( buf, 0x00, sizeof(buf) );
	result = FALSE;
	do{
          api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
          
          buf[0] = 0x00;
	  buf[1] = 0x01;
	  if( api_aux_rxready( dhn, buf ) == apiReady )
	    {
	    if( (buf[0] == 0x00) && (buf[1] == 0x01) )
	      {
	      api_aux_rxstring( dhn, buf );
	      
	      result = TRUE;
	      for( i=0; i<256; i++ )
	         {
	         if( buf[2+i] != i )
	           {
	           result = FALSE;
	           break;
	           }
	         }
	         
	       break;
	      }
	    }
          } while( tick2 - tick1 < 200 );	// 2 sec timeout
	
	api_tim_close( dhn_tim );
	
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 17 - AUX Port Loopback Test.
//		  
//		  01234567890123456789012345678901
//		  AUX0/AUX1/AUX2
//		  OPEN TEST/LOOPBACK TEST
//
// INPUT   : mode - 0 = normal (wait & quit on error)
//		    1 = special (run all)
// OUTPUT  : status - loopback status of com ports. (0=OK, 1=failed)
//		      bit0 - COM0
//		      bit1 - COM1
//		      bit2 - COM2
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	1
UINT32	DIAG_FUNC_TestAux( UINT32 mode, UINT32 *status )
{
API_AUX	pAux0;
API_AUX	pAux1;
API_AUX pAux2;
UINT8	dhn_aux0;
UINT8	dhn_aux1;
UINT8	dhn_aux2;
UINT8	select;
UINT32	result = FALSE;


	*status = 0;
	
	select = '9';

//	if( (mode == 0) && ((os_MultiBootMode == OP_MODE_POST) || (os_MultiBootMode == OP_MODE_LOCK)) )
	if( mode == 0 )
	  {
	  LIB_LCD_Puts( 0, 0, FONT0, sizeof("SELECT:"), (UINT8 *)"SELECT:" );
	  LIB_LCD_Puts( 2, 0, FONT0, sizeof("0-AUX0"), (UINT8 *)"0-AUX0" );
	  LIB_LCD_Puts( 3, 0, FONT0, sizeof("1-AUX1"), (UINT8 *)"1-AUX1" );
	  LIB_LCD_Puts( 4, 0, FONT0, sizeof("2-AUX2"), (UINT8 *)"2-AUX2" );
	  LIB_LCD_Puts( 5, 0, FONT0, sizeof("9-ALL"), (UINT8 *)"9-ALL" );
	  
	  do{
	    select = LIB_WaitKey();
	    if( select == 'x' )
	      return( FALSE );
	    } while( (select != '0') && (select != '1') && (select != '2') && (select != '9') );
	    
	  LIB_LCD_Cls();
	  }
	  
TEST_AUX0:
	if( (select != '9') && (select != '0') )
	  goto TEST_AUX1;

	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_AUX0), (UINT8 *)os_msg_AUX0 );
	
	pAux0.Mode = auxBYPASS;
	pAux0.Baud = COM_9600 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
	pAux0.Tob = 10;		// 100ms
	pAux0.Tor = 10;		// 100ms
	pAux0.Acks = 0;		// no ack
	pAux0.Resend = 0;	// no retry
	dhn_aux0 = api_aux_open( COM0, pAux0 );	// open COM0
	
	result = DIAG_TEST_AuxPort( dhn_aux0, mode );
	api_aux_close( dhn_aux0 );
	if( result == FALSE )
	  {
	  *status |= 0x01;
	  if( !mode )
	    return( FALSE );
	  }

	LIB_LCD_Cls();

TEST_AUX1:
	if( (select != '9') && (select != '1') )
	  goto TEST_AUX2;
	  
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_AUX1), (UINT8 *)os_msg_AUX1 );
	
	pAux1.Mode = auxBYPASS;
	pAux1.Baud = COM_9600 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
	pAux1.Tob = 10;		// 100ms
	pAux1.Tor = 10;		// 100ms
	pAux1.Acks = 0;		// no ack
	pAux1.Resend = 0;	// no retry
	dhn_aux1 = api_aux_open( COM1, pAux1 );	// open COM1
	
	result = DIAG_TEST_AuxPort( dhn_aux1, mode );
	api_aux_close( dhn_aux1 );
	if( result == FALSE )
	  {
	  *status |= 0x02;
	  if( !mode )
	    return( FALSE );
	  }

	LIB_LCD_Cls();

TEST_AUX2:
	if( (select != '9') && (select != '2') )
	  goto EXIT;
	  
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_AUX2), (UINT8 *)os_msg_AUX2 );
	
	pAux2.Mode = auxBYPASS;
	pAux2.Baud = COM_9600 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
	pAux2.Tob = 10;		// 100ms
	pAux2.Tor = 10;		// 100ms
	pAux2.Acks = 0;		// no ack
	pAux2.Resend = 0;	// no retry
	dhn_aux2 = api_aux_open( COM2, pAux2 );	// open COM2

	result = DIAG_TEST_AuxPort( dhn_aux2, mode );
	api_aux_close( dhn_aux2 );
	if( result == FALSE )
	  *status |= 0x04;

EXIT:  
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 18 - Test MODEM module.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_TestModem( void )
{
//#ifdef	_MODEM_ENABLED_

UINT8	result;
UINT8	select;
UINT8	buffer[8];
UINT8	tel_no[20];	// L-V
UINT8	pabx_code[9];	// L-V
UINT8	tp;		// 0 = tone, 1=pulse
UINT8	mode;		// 0 = sync, 1=async
UINT16	bps;		// 0 = 1200, 1 = 2400, 2 = 9600, 3 = 14400 bps
//API_HOST *ht=0;
API_HOST ht[1];


API_AUX	pAux;
UINT8	dhn_aux;
UINT8	dbuf[256];

	// -----------------------------------------------------------------------------
//	pAux.Mode = auxSOH;
//	pAux.Baud = COM_57600 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
//	pAux.Tob = 100;	// 1 second
//	pAux.Tor = 300;	// 3 seconds
//	pAux.Acks = 1;
//	pAux.Resend = 0;
//	dhn_aux = api_aux_open( COM0, pAux );
//	LIB_DispHexByte( 0, 0, dhn_aux );
//	LIB_WaitKey();
//
//	while(1)
//	{
//	while( api_aux_rxready( dhn_aux, dbuf ) != apiReady );
//	LIB_BUZ_Beep1();
//	api_aux_rxstring( dhn_aux, dbuf );
//	LIB_DumpHexData( 0, 1, dbuf[0]+dbuf[1]*256+2, dbuf );
//	LIB_LCD_Cls();
//	};
	
	// -----------------------------------------------------------------------------
	
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_MODEM), (UINT8 *)os_msg_MODEM );
	
	//-----------------------------------------------
	// 01234567890123456789
	// MODEM
	// 1=AUTO 2=SETUP
	//
	// NOTE:
	// 1. AUTO configuration is as follows: (speed up MFG test)
	//    TEL NO.='2'
	//    PABX CODE = NA
	//    DTMF
	//    SYNC
	//    1200 BPS
	// 2. SETUP runs the original process.
	//-----------------------------------------------
	
	LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_1AUTO_2SETUP), (UINT8 *)os_msg_1AUTO_2SETUP );
	do{
	  select = LIB_WaitKey();
	  if( select == 'x' )
	    return( FALSE );
	  }while( (select != '1') && (select != '2') );
	
	// TEL NO:
	LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_TEL_NO), (UINT8 *)os_msg_TEL_NO );
	do{
	  if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT+NUM_TYPE_LEADING_ZERO, '_', FONT0, 2, 16, tel_no ) != TRUE )	// PATCH: 2010-10-06
	    return( FALSE );
	  }while( tel_no[0] == 0 );

	  
	// PABX CODE:
	LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_PABX_CODE), (UINT8 *)os_msg_PABX_CODE );
	if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT+NUM_TYPE_LEADING_ZERO, '_', FONT0, 2, 8, pabx_code ) != TRUE )	// PATCH: 2010-10-06
	  return( FALSE );

	
	// 0=TONE 1=PULSE
	LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_TONE_PULSE), (UINT8 *)os_msg_TONE_PULSE );
	do{
	  if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT, '_', FONT0, 2, 1, buffer ) != TRUE )
	    return( FALSE );
	  if( buffer[0] == 0 )
	    break;
	  }while( (buffer[1] != '0') && (buffer[1] != '1') );
	
	tp = 0;
	if( (buffer[0] != 0) && (buffer[1] != '0') )
	  tp = 1;

//	LIB_DispHexByte( 6, 0, tp );
//	LIB_WaitKey();
	      
	// 0=SYNC 1=ASYNC
	LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_SYNC_ASYNC), (UINT8 *)os_msg_SYNC_ASYNC );
	do{
	  if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT, '_', FONT0, 2, 1, buffer ) != TRUE )
	    return( FALSE );
	  if( buffer[0] == 0 )
	    break;
	  }while( (buffer[1] != '0') && (buffer[1] != '1') );
	
	mode = 0;
	if( (buffer[0] != 0) && (buffer[1] != '0') )
	  mode = 1;
	
//	LIB_DispHexByte( 7, 0, mode );
//	LIB_WaitKey();
	
	// SPEED: (1200,2400,9600,14400)
	LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_SPEED), (UINT8 *)os_msg_SPEED );

	buffer[0] = 2;	// starting row number
        buffer[1] = 3;	// max lcd row cnt
        buffer[2] = 4;	// list items
        buffer[3] = 7;	// item length
        buffer[4] = 0;	// offset of LEN field in item
        buffer[5] = FONT0;
	bps = LIB_ListBox( 0, &buffer[0], (UINT8 *)&os_list_BPS[0], 0 );
	if( bps == 0xFF )
	  return( FALSE );
	  
	switch( bps )
	      {
	      case 0:
	      	   bps = MDM_1200;
	      	   break;
	      	   
	      case 1:
	      	   bps = MDM_2400;
	      	   break;
	      	   
	      case 2:
	      	   bps = MDM_9600;
	      	   break;
	      	   
	      case 3:
	      	   bps = MDM_14400;
	      	   break;
	      }
	
	LIB_LCD_ClearRow( 2, 1, FONT0 );
	
	
	// START (Y/N)?
	LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_START), (UINT8 *)os_msg_START );
	if( LIB_WaitKeyYesNo( 1, 6 ) == FALSE )
	  return( FALSE );
	
	LIB_LCD_ClearRow( 1, 1, FONT0 );
	
	
	// Check current line stauts
	while(1)
	     {
	     result = api_mdm_phonecheck( buffer );
//	     LIB_DispHexByte( 6, 0, result );
//	     LIB_DispHexByte( 7, 0, buffer[0] );
//	     LIB_WaitKey();
//	     continue;
	     
	     if( buffer[0] == mdmIdle )
	       {
	       LIB_WaitTime( 50 );	// wait 1 sec
	       break;
	       }
	    
	     DIAG_DispLineStatus( 0, FALSE, buffer[0] );
	  
	     if( LIB_GetKeyStatus() == apiReady )
	       {
	       if( LIB_WaitKey() == 'x' )
	         return( FALSE );
	       }
	  }
	
	
	// Setup Dial-up Parameters
	
	if( mode == 0 )
	  {
	  ht->address = 0x30;
	  ht->mode = mdmSDLC;
	  }
	else
	  {
	  ht->address = 0x00;
	  ht->mode = mdmBYPASS;
	  }
	
	ht->baud = bps;
	ht->rdpi = 3;
	ht->attempt1 = 2;
	ht->cdon1 = 30;
	
	ht->country = 0xFE;		// Taiwan
	
	ht->len1 = tel_no[0] + 1;	// including 'T' or 'P'
	if( tp == 0 )
	  ht->phone1[0] = 'T';
	else
	  ht->phone1[0] = 'P';
	memmove( &ht->phone1[1], &tel_no[1], tel_no[0] );
	
//	LIB_DumpHexData( 0, 0, 7, ht->phone1 );
	
	ht->attempt2 = 0;
	ht->len2 = 0;
	
	buffer[0] = 0;
	result = api_mdm_open( buffer, (UINT8 *)ht );
	
	while(1)
	     {
	     DIAG_DispLineStatus( result, TRUE, buffer[0] );
	
	     if( LIB_GetKeyStatus() == apiReady )
	       {
	       if( LIB_WaitKey() == 'x' )
	         {
	         api_mdm_close( result, 0 );
	         break;
	         }
	       }
	     }

//#endif

	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 19 - Test Printer Module.
//
//		  01234567890123456789012345678901
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//
//		  ���ؤ���	(A4BA ABD8 A4A4 A4E5)
//		  �@�A�B�C	(A140 A142 A143 A144)
//		  ��������	(F9FB F9FC F9FD F9FE)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
//#include "BMP_JOIN.h"

UINT32	DIAG_FUNC_TestPrinter( void )
{
UINT8	i;
UINT8	dhn_prn;
UINT8	data;
UINT8	result;
UINT8	status;
UINT8	config[1];
UINT8	dbuf[257];
//UINT8	*pTempBuf;
//API_GRAPH	dim;
//API_PRT_FONT	ft;


	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_PRINTER), (UINT8 *)os_msg_PRINTER );

	// setup ASCII 12X24 font
//	ft.FontID = FONT0;
//	ft.Height = 24;
//	
//	ft.AN_Type    = FT_ASCII;
//	ft.AN_CodeNo  = 1;
//	ft.AN_ByteNo  = 2*24;
//	ft.AN_CharNo  = 96;
//	ft.AN_Width   = 12;
//	ft.AN_BitMap  = (UCHAR *)FONT2_ASCII_12x24;
//	ft.AN_CodeMap = (UCHAR *)0;
//	
//	ft.GC_Type    = FT_UNUSED;
//	ft.GC_CodeNo  = 0;
//	ft.GC_ByteNo  = 0;
//	ft.GC_CharNo  = 0;
//	ft.GC_Width   =  0;
//	ft.GC_BitMap  = (UCHAR *)0;
//	ft.GC_CodeMap = (UCHAR *)0;
//	api_prt_initfont( ft );

//	ft.GC_Type = FT_BIG5;
//	ft.GC_CodeNo = 2;
//	ft.GC_ByteNo = 3*24;
//	ft.GC_CharNo = 83;
//	ft.GC_Width = 24;
//	ft.GC_BitMap = (UCHAR *)FONT2_BIG5_24x24;
//	ft.GC_CodeMap = (UCHAR *)FONT2_BIG5_24_code_list;
//	
//	api_prt_initfont( ft );

	// init printer
	dhn_prn = api_prt_open( prtThermal, 0 );
	if( dhn_prn == apiOutOfService )
	  return( FALSE );
	  
#if	0
	// print graphics (DEBUG ONLY)
	dim.Xleft  = 0;
	dim.Ytop   = 0;
	dim.Width  = 384;
	dim.Height = 250;
	dim.Method = 0;
	api_prt_putgraphics( dhn_prn, dim, (UCHAR *)bmp_JOIN );
	goto PRT_LF;
#endif

//PRT_STATUS:
//	result = api_prt_status( dhn_prn, (UCHAR *)&status );
//	LIB_DispHexByte( 0, 0, result );
//	LIB_DispHexByte( 0, 3, status );
//	LIB_WaitKey();
//	goto PRT_STATUS;
	
	// put LF*1
//	dbuf[0] = 0x01;
//	dbuf[1] = 0x0a;
//	result = api_prt_putstring( dhn_prn, FONT0, dbuf );
//	LIB_DispHexByte( 1, 0, result );
//	LIB_WaitKey();

PRINT_1_LINE:
	// put "0123...01"
	dbuf[0] = 33;
	data = '0';
	for( i=1; i<33; i++ )
	   {
	   dbuf[i] = data++;
	   if( data == ('9' + 1) )
	     data = '0';
	   }
	dbuf[i] = 0x0a;
	result = api_prt_putstring( dhn_prn, FONT0, dbuf );
	
//	api_prt_status( dhn_prn, (UCHAR *)&status );
//	DEBUGPRINTF("\r\nStatus=%x\r\n", status );
//	
//	LIB_WaitKey();
//	goto PRINT_1_LINE;
	
	// put "HHH...HH" for 8 lines
	memset( &dbuf[1], 'H', 32 );
	dbuf[0] = 33;
	dbuf[33] = 0x0a;
	for( i=0; i<8; i++ )
	   result = api_prt_putstring( dhn_prn, FONT0, dbuf );

//	if( !DIAG_CheckChineseFont( dbuf ) )
//	  goto PRT_LF;

	// 2011-03-14, test Chinese built-in FONT2 (24x24)
	dbuf[0] = 21;
	dbuf[1] = 0xA4;
	dbuf[2] = 0xBA;
	dbuf[3] = 0xAB;
	dbuf[4] = 0xD8;
	dbuf[5] = 0xA4;
	dbuf[6] = 0xA4;
	dbuf[7] = 0xA4;
	dbuf[8] = 0xE5;
	dbuf[9] = ' ';
	dbuf[10]= 'F';
	dbuf[11]= 'O';
	dbuf[12]= 'N';
	dbuf[13]= 'T';
	dbuf[14]= '2';
	dbuf[15]= ':';
	dbuf[16]= '2';
	dbuf[17]= '4';
	dbuf[18]= 'x';
	dbuf[19]= '2';
	dbuf[20]= '4';	
	dbuf[21] = 0x0A;
	result = api_prt_putstring( dhn_prn, FONT2, dbuf );
	
	dbuf[0] = 9;
	dbuf[1] = 0xA1;
	dbuf[2] = 0x40;
	dbuf[3] = 0xA1;
	dbuf[4] = 0x41;
	dbuf[5] = 0xA1;
	dbuf[6] = 0x42;
	dbuf[7] = 0xA1;
	dbuf[8] = 0x43;
	dbuf[9] = 0x0A;
	result = api_prt_putstring( dhn_prn, FONT2, dbuf );

	dbuf[0] = 9;
	dbuf[1] = 0xF9;
	dbuf[2] = 0xFB;
	dbuf[3] = 0xF9;
	dbuf[4] = 0xFC;
	dbuf[5] = 0xF9;
	dbuf[6] = 0xFD;
	dbuf[7] = 0xF9;
	dbuf[8] = 0xFE;
	dbuf[9] = 0x0A;
	result = api_prt_putstring( dhn_prn, FONT2, dbuf );

	// DOUBLE
	// 2011-03-14, test Chinese built-in FONT2 (24x24)
	dbuf[0] = 21;
	dbuf[1] = 0xA4;
	dbuf[2] = 0xBA;
	dbuf[3] = 0xAB;
	dbuf[4] = 0xD8;
	dbuf[5] = 0xA4;
	dbuf[6] = 0xA4;
	dbuf[7] = 0xA4;
	dbuf[8] = 0xE5;
	dbuf[9] = ' ';
	dbuf[10]= 'F';
	dbuf[11]= 'O';
	dbuf[12]= 'N';
	dbuf[13]= 'T';
	dbuf[14]= '2';
	dbuf[15]= ':';
	dbuf[16]= '2';
	dbuf[17]= '4';
	dbuf[18]= 'x';
	dbuf[19]= '4';
	dbuf[20]= '8';	
	dbuf[21] = 0x0A;
	result = api_prt_putstring( dhn_prn, FONT2+0x80, dbuf );
	
#if	1
	// 2012-04-16, test Chinese built-in FONT4 (16x16)
	dbuf[0] = 21;
	dbuf[1] = 0xA4;
	dbuf[2] = 0xBA;
	dbuf[3] = 0xAB;
	dbuf[4] = 0xD8;
	dbuf[5] = 0xA4;
	dbuf[6] = 0xA4;
	dbuf[7] = 0xA4;
	dbuf[8] = 0xE5;
	dbuf[9] = ' ';
	dbuf[10]= 'F';
	dbuf[11]= 'O';
	dbuf[12]= 'N';
	dbuf[13]= 'T';
	dbuf[14]= '4';
	dbuf[15]= ':';
	dbuf[16]= '1';
	dbuf[17]= '6';
	dbuf[18]= 'x';
	dbuf[19]= '1';
	dbuf[20]= '6';
	dbuf[21]= 0x0A;
	result = api_prt_putstring( dhn_prn, FONT4, dbuf );
	
	dbuf[0] = 9;
	dbuf[1] = 0xA1;
	dbuf[2] = 0x40;
	dbuf[3] = 0xA1;
	dbuf[4] = 0x41;
	dbuf[5] = 0xA1;
	dbuf[6] = 0x42;
	dbuf[7] = 0xA1;
	dbuf[8] = 0x43;
	dbuf[9] = 0x0A;
	result = api_prt_putstring( dhn_prn, FONT4, dbuf );

	dbuf[0] = 9;
	dbuf[1] = 0xF9;
	dbuf[2] = 0xFB;
	dbuf[3] = 0xF9;
	dbuf[4] = 0xFC;
	dbuf[5] = 0xF9;
	dbuf[6] = 0xFD;
	dbuf[7] = 0xF9;
	dbuf[8] = 0xFE;
	dbuf[9] = 0x0A;
	result = api_prt_putstring( dhn_prn, FONT4, dbuf );
#endif

PRT_LF:
	// put LF*6
	dbuf[0] = 0x07;
	memset( &dbuf[1], 0x0a, 6 );
	dbuf[7] = 0x0C;
	api_prt_putstring( dhn_prn, FONT0, dbuf );
	
//	LIB_WaitKey();
//	goto PRINT_1_LINE;

//PRT_STATUS:
//	result = api_prt_status( dhn_prn, (UCHAR *)&status );
//	LIB_DispHexByte( 1, 0, result );
//	LIB_DispHexByte( 2, 0, status );
//	LIB_WaitKey();
//	goto PRT_STATUS;

	do
	 {
	 result = api_prt_status( dhn_prn, (UINT8 *)&status );
//	 DEBUGPRINTF( "\r\nStatus=%x\r\n", status );
	 if( status == prtPaperEmpty )
	   break;
	 } while( status != prtComplete );
	
	api_prt_close( dhn_prn );
	
	if( result == apiOK )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: 20 - Test all related communication units as well as printer.
//		  
//		  01234567890123456789012345678901
//		  --------------------------------
//		  *     AS320 CP TEST REPORT     *
//		  --------------------------------
//		  AUX0				OK
//		  AUX1				OK
//		  AUX2				OK
//		  - - - - - - - - - - - - - - - -
//		   !"#...........................?
//		  @ABC..........................._
//		  'abc..........................~
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//		  HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_TestCP( void )
{
UINT32	i, j;
UINT32	com_status;
UINT8	status;
UINT8	result;
UINT8	dbuf[256];
UINT8	dhn_prn;
UINT8	data;


	DIAG_FUNC_TestAux( 1, &com_status );
	
	// init printer
	dhn_prn = api_prt_open( prtThermal, 0 );
	if( dhn_prn == apiOutOfService )
	  return( FALSE );
	
	dbuf[0] = 33;
	memset( &dbuf[1], '-', 32 );
	dbuf[33] = 0x0a;
	result = api_prt_putstring( dhn_prn, FONT0, dbuf );
	
	dbuf[0] = 33;
	memmove( &dbuf[1], "*     AS350 CP TEST REPORT     *", 32 );
	dbuf[33] = 0x0a;
	result = api_prt_putstring( dhn_prn, FONT0, dbuf );
	
	dbuf[0] = 33;
	memset( &dbuf[1], '-', 32 );
	dbuf[33] = 0x0a;
	result = api_prt_putstring( dhn_prn, FONT0, dbuf );
	
	// AUX0
	dbuf[0] = 33;
	memset( &dbuf[1], 0x20, 32 );
	memmove( &dbuf[1], "AUX0", 4 );
	if( com_status & 0x01 )
	  {
	  dbuf[28] = 'E';
	  dbuf[29] = 'R';
	  dbuf[30] = 'R';
	  dbuf[31] = 'O';
	  dbuf[32] = 'R';
	  }
	else
	  {
	  dbuf[31] = 'O';
	  dbuf[32] = 'K';
	  }
	dbuf[33] = 0x0a;
	result = api_prt_putstring( dhn_prn, FONT0, dbuf );
	
	// AUX1
	dbuf[0] = 33;
	memset( &dbuf[1], 0x20, 32 );
	memmove( &dbuf[1], "AUX1", 4 );
	if( com_status & 0x02 )
	  {
	  dbuf[28] = 'E';
	  dbuf[29] = 'R';
	  dbuf[30] = 'R';
	  dbuf[31] = 'O';
	  dbuf[32] = 'R';
	  }
	else
	  {
	  dbuf[31] = 'O';
	  dbuf[32] = 'K';
	  }
	dbuf[33] = 0x0a;
	result = api_prt_putstring( dhn_prn, FONT0, dbuf );
	
	// AUX2
	dbuf[0] = 33;
	memset( &dbuf[1], 0x20, 32 );
	memmove( &dbuf[1], "AUX2", 4 );
	if( com_status & 0x04 )
	  {
	  dbuf[28] = 'E';
	  dbuf[29] = 'R';
	  dbuf[30] = 'R';
	  dbuf[31] = 'O';
	  dbuf[32] = 'R';
	  }
	else
	  {
	  dbuf[31] = 'O';
	  dbuf[32] = 'K';
	  }
	dbuf[33] = 0x0a;
	result = api_prt_putstring( dhn_prn, FONT0, dbuf );

	dbuf[0] = 33;
	memset( &dbuf[1], '-', 32 );
	dbuf[33] = 0x0a;
	result = api_prt_putstring( dhn_prn, FONT0, dbuf );

	data = 0x20;
	for( i=0; i<3; i++ )
	   {
	   for( j=0; j<32; j++ )
	      dbuf[1+j] = data++;
	   dbuf[33] = 0x0a;
	   result = api_prt_putstring( dhn_prn, FONT0, dbuf );
	   }
	   
	// put "HHH...HH" for 6 lines
	memset( &dbuf[1], 'H', 32 );
	dbuf[0] = 33;
	dbuf[33] = 0x0a;
	for( i=0; i<6; i++ )
	   result = api_prt_putstring( dhn_prn, FONT0, dbuf );	

	// 2015-01-28, print out KCVs of TMK
	//
	// TMK
	// XX XX XX  XX XX XX
	// XX XX XX  XX XX XX
	// ...
	// XX XX XX  XX XX XX
	// -- -- --  -- -- --
	dbuf[0] = 10;
	dbuf[1] = 0x0a;
	dbuf[2] = 'T';
	dbuf[3] = 'M';
	dbuf[4] = 'K';
	dbuf[5] = ' ';
	dbuf[6] = 'K';
	dbuf[7] = 'C';
	dbuf[8] = 'V';
	dbuf[9] = ':';
	dbuf[10] = 0x0a;
	result = api_prt_putstring( dhn_prn, FONT0, dbuf );
	DIAG_FUNC_VerifyTMK( 1, dhn_prn );	// print out KCVs of TMK
	
	// put LF*6
	dbuf[0] = 0x07;
	memset( &dbuf[1], 0x0a, 6 );
	dbuf[7] = 0x0C;
	api_prt_putstring( dhn_prn, FONT0, dbuf );
	
	do
	 {
	 result = api_prt_status( dhn_prn, (UINT8 *)&status );
	 if( status == prtPaperEmpty )
	   break;
	 } while( status != prtComplete );
	
	api_prt_close( dhn_prn );

	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Output UDP packet log from driver layer (SRAM).
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	DIAG_FUNC_DumpLogForDHCP( void )
{
UINT32	i;
UINT32	loop_cnt = 2;
UINT8	result;
UINT8	status;
UINT8	dhn_prn;
UINT16	cnt;
UINT16	left_bytes;
UINT8	*ptrUDP;
UINT32	len;
UINT8	sbuf[64];
UINT8	dbuf[128];


	if( !os_DHCP_DEBUG )
	  return;
	  
	// DEBUG TEST DATA
#if	0
	ptrUDP = os_UDP_LOG_PTR;
	
	ptrUDP[0] = 8;
	ptrUDP[1] = 0;
	ptrUDP[2] = 0;
	ptrUDP[3] = 0;
	
	ptrUDP[4] = 0x01;
	ptrUDP[5] = 0x23;
	ptrUDP[6] = 0x45;
	ptrUDP[7] = 0x67;
	ptrUDP[8] = 0x89;
	ptrUDP[9] = 0xab;
	ptrUDP[10]= 0xcd;
	ptrUDP[11]= 0xef;
	
	ptrUDP[12] = 0x00;
	ptrUDP[13] = 0x00;
	ptrUDP[14] = 0x00;
	ptrUDP[15] = 0x00;
#endif

	// init printer
	dhn_prn = api_prt_open( prtThermal, 0 );
	
	ptrUDP = os_UDP_LOG_PTR;
	
ST:
	if( loop_cnt == 0 )
	  goto EXIT;
	loop_cnt--;
	
	if( loop_cnt == 1 )
	  {
	  dbuf[0] = strlen("OFFER")+1;
	  memmove( &dbuf[1], "OFFER", strlen("OFFER") );
	  dbuf[1+strlen("OFFER")] = 0x0A;
	  }
	else
	  {
	  dbuf[0] = strlen("ACK")+1;
	  memmove( &dbuf[1], "ACK", strlen("ACK") );
	  dbuf[1+strlen("ACK")] = 0x0A;
	  }
	api_prt_putstring( dhn_prn, FONT0, dbuf );
	
	// 1'st LEN(4) + UDP(n)
	len = ptrUDP[0] + ptrUDP[1]*0x100 + ptrUDP[2]*0x10000 + ptrUDP[3]*0x1000000;
	
	memmove( sbuf, ptrUDP, 4 );		// print packet LEN
	LIB_hex2asc( 4, sbuf, dbuf );		// dbuf=L(1)+V(n)
	dbuf[0] += 1;				//
	dbuf[9] = 0x0A;				//
	api_prt_putstring( dhn_prn, FONT0, dbuf );
	
	ptrUDP += 4;	// ptr 1'st UDP data
	
	if( len )
	  {
	  cnt = len / 16;
	  left_bytes = len % 16;
	  
	  if( cnt )
	    {
	    for( i=0; i<cnt; i++ )
	       {
	       // convert HEX to ASC
	       memmove( sbuf, ptrUDP, 16 );
	       LIB_hex2asc( 16, sbuf, dbuf );		// dbuf=L(1)+V(n)
	       dbuf[0] += 1;
	       dbuf[33] = 0x0A;
	       api_prt_putstring( dhn_prn, FONT0, dbuf );
	       
	       ptrUDP += 16;
	       }
	    }
	    
	  if( left_bytes )
	    {
	    // convert HEX to ASC
	    memmove( sbuf, ptrUDP, left_bytes );
	    LIB_hex2asc( left_bytes, sbuf, dbuf );	// dbuf=L(1)+V(n)
	    dbuf[0] += 1;
	    dbuf[1+(left_bytes*2)] = 0x0A;
	    api_prt_putstring( dhn_prn, FONT0, dbuf );
	    
	    ptrUDP += left_bytes;
	    }

	  // put LF*6
	  dbuf[0] = 0x07;
	  memset( &dbuf[1], 0x0A, 6 );
	  dbuf[7] = 0x0C;
	  api_prt_putstring( dhn_prn, FONT0, dbuf );
	  }

	goto ST;

EXIT:
	os_UDP_LOG_CNT = 0;
	
	do
	 {
	 result = api_prt_status( dhn_prn, (UINT8 *)&status );
	 if( status == prtPaperEmpty )
	   break;
	 } while( status != prtComplete );
	
	api_prt_close( dhn_prn );
	
	os_DHCP_DEBUG = FALSE;
}
#endif

// ---------------------------------------------------------------------------
#if	0
void	ESUN_TCP_DEBUG( void )
{
UINT16	count = 0;
UINT8	dhn_tim, dhn_tim2;
UINT16	tick1, tick2;
UINT16	tick2_new, tick2_old;
UINT16	tick10, tick20;
UINT16	tick20_new, tick20_old;
UINT16	len;
UINT8	abuf[16];
UINT8	bar = '-';
UINT16	data01, data20;


	LIB_LCD_Cls();
	
	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "1379" ) != TRUE )
	  return;
	  
	LIB_LCD_Cls();
	
	LIB_LCD_Puts( 0, 0, FONT0, strlen("TCP/IP TEST"), (UINT8 *)"TCP/IP TEST" );
	LIB_LCD_Puts( 2, 0, FONT0, strlen("IDLE...30SEC RESET"), (UINT8 *)"IDLE...30SEC RESET" );

	dhn_tim = api_tim_open(100);	// unit=1 sec
	dhn_tim2 = api_tim_open(10);	// unit=100ms
	
	len = LIB_itoa( count, abuf );
	LIB_LCD_Puts( 4, 0, FONT0, len, abuf );
	
	LIB_LCD_Putc( 6, 0, FONT0, bar );
	
	api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
	api_tim_gettick( dhn_tim, (UCHAR *)&tick2_old );

	api_tim_gettick( dhn_tim2, (UCHAR *)&tick10 );
	api_tim_gettick( dhn_tim2, (UCHAR *)&tick20_old );
	
	while(1)
	     {
	     // REGISTER DATA
	     api_tim_gettick( dhn_tim2, (UCHAR *)&tick20 );
	     tick20_new = tick20;
	     if( tick20 < tick10 )
	       {
	       api_tim_gettick( dhn_tim2, (UCHAR *)&tick10 );
	       continue;
	       }
	     
	     if( (tick20 - tick10 ) >= 5 )	// every 500 ms
	       {
	       data01 = apk_lan_readPHY(1);
	       data20 = apk_lan_readPHY(20);
	       LIB_DispHexWord( 5, 0, data01 );
	       LIB_DispHexWord( 5, 5, data20 );
	       
	       api_tim_gettick( dhn_tim2, (UCHAR *)&tick10 );
	       }
	     
	     
	     // LINK STATUS
	     api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
	     tick2_new = tick2;
	     if( tick2 < tick1 )
	       {
	       api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
	       continue;
	       }
	       
//	     if( (tick2 - tick1) >= (5*60) )	// 5 min?
	     if( (tick2 - tick1) >= (1*30) )	// 30 sec?
	       {
//	       LIB_BUZ_Beep2();
	       BSP_Delay_n_ms(1000);
	       
	       api_lan_reset();
	       
	       api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
	       
	       count++;
	       
//	       	 data01 = apk_lan_readPHY(1);
//	       	 data20 = apk_lan_readPHY(20);
//	       	 LIB_DispHexWord( 5, 0, data01 );
//	       	 LIB_DispHexWord( 5, 5, data20 );
	       
	       len = LIB_itoa( count, abuf );
	       LIB_LCD_Puts( 4, 0, FONT0, len, abuf );
	       }
	     else
	       {
	       if( tick2_old != tick2_new )
	       	 {
	       	 tick2_old = tick2_new;
	       	 
//	       	 data01 = apk_lan_readPHY(1);
//	       	 data20 = apk_lan_readPHY(20);
//	       	 LIB_DispHexWord( 5, 0, data01 );
//	       	 LIB_DispHexWord( 5, 5, data20 );
	       	 
	       	 if( api_lan_lstatus() == apiOK )
	       	   {
	       	   switch( bar )
	       	         {
	       	         case '-':
	       	              bar = 0x5c;
	       	              break;
	       	               
	       	         case 0x5c:
	       	              bar = '|';
	       	              break;
	       	         
	       	         case '|':
	       	              bar = '/';
	       	              break;
	       	         
	       	         case '/':
	       	              bar = '-';
	       	              break;
	       	         }
	       	         
	       	   LIB_LCD_Putc( 6, 0, FONT0, bar );
	       	   }
	       	 }
	       }
	     }
	
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Show current LAN status.
// INPUT   : row    - row number to show the status string.
//	     status - current status value. (lanXXXX)
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	ShowLanStatus( UCHAR row, UCHAR status )
{
	
	switch( status )
	      {
	      case lanIdle:
	           UTIL_PutStr( row, 0, FONT0, 16, (UCHAR *)"IDLE            ");
	           break;
	           
	      case lanConnecting:
	           UTIL_PutStr( row, 0, FONT0, 16, (UCHAR *)"CONNECTING...   ");
	           break;
	           
	      case lanConnected:
	           UTIL_PutStr( row, 0, FONT0, 16, (UCHAR *)"CONNECTED OK    ");
	           break;
	           
	      case lanProcessing:
	           UTIL_PutStr( row, 0, FONT0, 16, (UCHAR *)"PROCESSING...   ");
	           break;
	      }

}

// ---------------------------------------------------------------------------
const	UCHAR	ISO8583_TRANS_MSG_SAMPLE[] = {

		0x60, 0x02, 0x83, 0x00, 0x00, 				// TPDU
		0x02, 0x00, 						// trans type
		0x30, 0x20, 0x05, 0x80, 0x28, 0xC0, 0x00, 0x16,		// field bit map
		0x00, 0x00, 0x00, 					// process code
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 			// trans amount
		0x00, 0x00, 0x02, 0x60, 0x22, 0x02, 0x83, 0x00,
		0xB7, 0x6C, 0x62, 0xD9, 0xDF, 0x6C, 0xAB, 0x50, 0x1C, 0x6E, 0xFD, 0x81, 0xBD, 0x34, 0xE8, 0xCD,
		0x93, 0x45, 0xA4, 0x33, 0xA8, 0xB9, 0x38, 0x39, 0x52, 0x39, 0x39, 0x39, 0x30, 0x38, 0x35, 0x30,
		0x31, 0x30, 0x30, 0x32, 0x20, 
		0x31, 0x33, 0x39, 0x39, 0x39, 0x30, 0x38, 0x35,		// TID
		0x36, 0x36, 0x30,
		0x31, 0x30, 0x30, 0x30, 0x31, 0x35, 0x39, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x06, 0x30, 0x30,
		0x30, 0x30, 0x30, 0x31, 0x00, 0x06, 0x30, 0x30, 0x30, 0x30, 0x30, 0x32, 0x00, 0x19, 0x00, 0x17,
		0x31, 0x32, 0x31, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x99,
		0x76,
		};
		
// ---------------------------------------------------------------------------
// FUNCTION: 7 - Test Ethernet module.
//		  
//		  01234567890123456789012345678901
//		  ETHERNET
//		  1=SETUP 2=PING
//		  
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
UINT32	SYS_FUNC_TestLan( void )
{
UCHAR	dhn_lan;
UCHAR	sbuf[512];
UCHAR	dbuf[512];

UCHAR	status; 
UCHAR	result;
UINT	Len;

API_IPCONFIG	ipconfig;
API_LAN		lan;

char	msg_TEST_LAN[] =	{"TEST LAN"};
char	msg_CANCEL_TO_EXIT[] =	{"CANCEL TO EXIT"};
char	msg_TX_DATA[] =		{"TX DATA..."};
char	msg_RX_DATA[] =		{"RX DATA..."};
char	msg_OK[] =		{"OK"};
char	msg_ERR[] =		{"ERR"};
char	msg_DISCONNECT[] =	{"DISCONNECT"};
char	msg_NO_LINE[] =		{"NO LINE"};
char	msg_HIT_KEY_TO_SEND[] =	{"HIT ANY KEY TO SEND..."};


//	UTIL_OpenDisplay();
//	UTIL_OpenKey_ALL();
//	UTIL_OpenBuzzer_1S();

//	UTIL_ClearScreen();
	LIB_LCD_Puts( 0, 0, FONT1+attrREVERSE, strlen(msg_TEST_LAN), (UCHAR *)msg_TEST_LAN );
	LIB_LCD_Puts( 7, 0, FONT0, strlen(msg_CANCEL_TO_EXIT), (UCHAR *)msg_CANCEL_TO_EXIT );

	// Check current line status (OPIONAL)
	while(1)
	     {
	     result = api_lan_lstatus();
	     if( result == apiOK )
	       {
	       UTIL_WaitTime( 50 );
	       break;
	       }
	    
	     LIB_LCD_Puts( 4, 0, FONT0, strlen(msg_NO_LINE), (UCHAR *)msg_NO_LINE );
	  
	     if( LIB_GetKeyStatus() == apiReady )
	       {
	       if( UTIL_WaitKey() == 'x' )
	         return;
	       }
	     }

	// setup IPCONFIG (demo only)
#if	0
	ipconfig.IP_Len = 11;
	memmove( ipconfig.IP, "172.16.1.30", 11 );
	
	ipconfig.Gateway_Len = 10;
	memmove( ipconfig.Gateway, "172.16.1.1", 10 );
	
	ipconfig.SubnetMask_Len = 13;
	memmove( ipconfig.SubnetMask, "255.255.255.0", 13 );
	
	if( api_lan_setIPconfig( ipconfig ) != apiOK )
	  {
	  UTIL_PutStr( 5, 0, FONT0, strlen("setIPconfig Failed"), (UCHAR *)"setIPconfig Failed" );
	  UTIL_WaitKey();
	  return;
	  }
#endif

	// open LAN (demo only)
	lan.LenType = 0x01;	// 0x00=HEX, 0x01=BCD, 0xFF=BYPASS
	lan.ClientPort = 65100;
	lan.ServerPort = 8000;
	lan.ServerIP_Len = 12;
	memmove( lan.ServerIP, "172.16.1.170", lan.ServerIP_Len );
	
	LIB_LCD_Puts( 3, 0, FONT0, strlen("OPEN: "), (UCHAR *)"OPEN: " );
	dhn_lan = api_lan_open( lan );
//	LIB_DispHexByte( 0, 20, dhn_lan );
//	LIB_WaitKey();

	// connect to "AS330TCP.dat" on ARCHHOST...
	if( dhn_lan != apiOutOfService )
	  {
	  LIB_LCD_Puts( 3, strlen("OPEN: "), FONT0, strlen(msg_OK), (UCHAR *)msg_OK );
	  while(1)
	       {
	       if( api_lan_status( dhn_lan, (UCHAR *)&status ) == apiOK )
	         ShowLanStatus( 4, status );
          
	       if( status == lanConnected )
	         {
	         LIB_BUZ_Beep1();
	         
	         LIB_LCD_Puts( 7, 0, FONT0, strlen(msg_HIT_KEY_TO_SEND), (UCHAR *)msg_HIT_KEY_TO_SEND );
	         LIB_WaitKey();
	         LIB_LCD_ClearRow( 7, 1, FONT0 );
	         
	         break;
	         }
	       
	       if( LIB_GetKeyStatus() == apiReady )
	         {
	         if( LIB_WaitKey() == 'x' )
	           goto EXIT;
	         }
	       }
	              


#if	1
	  /* --- wait for device ready for transmission --- */ 
	  do{
	    result = api_lan_txready( dhn_lan );
	    
#if	0	// the following key checking is not applicable to M6
	    if( LIB_GetKeyStatus() == apiReady )
	      {
	      if( LIB_WaitKey() == 'x' )
	        goto EXIT;
	      }
#endif
	    } while( result != apiReady );
	  
	  /*--- transmit data to HOST --- */
	  LIB_LCD_Puts( 4, 0, FONT0, strlen(msg_TX_DATA), (UCHAR *)msg_TX_DATA );
	   
	  sbuf[0] = sizeof( ISO8583_TRANS_MSG_SAMPLE ) & 0x00ff;
	  sbuf[1] = sizeof( ISO8583_TRANS_MSG_SAMPLE ) & 0xff00; 
	  memmove( &sbuf[2], ISO8583_TRANS_MSG_SAMPLE, sizeof( ISO8583_TRANS_MSG_SAMPLE ) );
	  api_lan_txstring( dhn_lan, sbuf );
	  
	  LIB_LCD_Puts( 4, strlen(msg_TX_DATA), FONT0, strlen(msg_OK), (UCHAR *)msg_OK );
	  
//	  LIB_WaitKey();
	  
	  /* --- check response form HOST --- */
	  LIB_LCD_Puts( 5, 0, FONT0, strlen(msg_RX_DATA), (UCHAR *)msg_RX_DATA );
	  
	  do{
	    result = api_lan_rxready( dhn_lan, (UCHAR *)&Len );
	    
#if	0	// the following key checking is not applicable to M6
	    if( LIB_GetKeyStatus() == apiReady )
	      {
	      if( LIB_WaitKey() == 'x' )
	        goto EXIT;
	      }
#endif
	    } while( result != apiReady );
	  
	  /* --- get response data --- */ 
	  if(api_lan_rxstring( dhn_lan, dbuf ) == apiOK )
	    {
//	    LIB_WaitKey();
//	    UTIL_DumpHexData( 0, 0, dbuf[0]+dbuf[1]*256+2, dbuf );
	    LIB_LCD_Puts( 5, strlen(msg_RX_DATA), FONT0, strlen(msg_OK), (UCHAR *)msg_OK );
	    }
	  else
	    LIB_LCD_Puts( 5, strlen(msg_RX_DATA), FONT0, strlen(msg_ERR), (UCHAR *)msg_ERR );

	  LIB_WaitKey();
	  LIB_DumpHexData( 0, 0, dbuf[0]+dbuf[1]*256+2, dbuf );
	  
	  /* --- disconnect link --- */
	  LIB_LCD_Puts( 6, 0, FONT0, strlen(msg_DISCONNECT), (UCHAR *)msg_DISCONNECT );
	  goto EXIT;
#endif
	  }
	else
	  {
	  LIB_LCD_Puts( 3, strlen("OPEN: "), FONT0, strlen(msg_ERR), (UCHAR *)msg_ERR );
//	  LIB_WaitKey();
	  goto EXIT2;
	  }
EXIT:
	  api_lan_close( dhn_lan );
EXIT2:  
	  LIB_WaitKey();
	  
	  return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: 21 - Test Ethernet module.
//		  
//		  01234567890123456789012345678901
//		  ETHERNET
//		  1=SETUP 2=PING
//		  
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	1
UINT32	DIAG_FUNC_TestIP( void )
{
//#ifdef	_LAN_ENABLED_

API_IPCONFIG config;
//API_IPCONFIG config[1];
UINT8	result;
UINT8	msg_ENABLE_DHCP[] = {"ENABLE DHCP"};
UINT8	msg_DHCP[]	  = {"DHCP..."};
UINT8	ip[20];	// L-V
UINT32	timeUS;
UINT8	select;
UINT32	int_1;
UINT32	dec_1;
UINT32	dec_2;
UINT32	remainder;
UINT8	len, len1;
UINT8	abuf[20];
UINT8	buffer[20];

UINT8	MAC[6];
UINT32	data;

	
	LIB_LCD_Puts( 0, 0, FONT0, strlen(os_msg_ETHERNET), (UINT8 *)os_msg_ETHERNET );
	LIB_LCD_Puts( 1, 0, FONT0, strlen(os_msg_1SETUP_2PING), (UINT8 *)os_msg_1SETUP_2PING );
	
//	POST_AutoSetMAC();
	
	do{
	  select = LIB_WaitKey();
	  if( select == 'x' )
	    return( TRUE );
	  }while( (select != '1') && (select != '2') && (select != '3') && (select != '9') &&(select != 'y') );

//	if( select == '3' )	// E-SUN BANK TCP DEBUG ONLY
//	  {
//	  ESUN_TCP_DEBUG();
//	  return( FALSE );
//	  }
	  
//	if( select == '9' )	// debug?
//	  {
//	  os_DHCP_DEBUG = TRUE;	// referenced by DHCP driver
//	  select = '1';
//	  }

	if( select == 'y' )
	  select = '2';
	
	if( select == '1' )	// setup IP config
	  {
	  // SETUP
	  // MAC: (internal)
	  // IP:
	  // SUBNET MASK:
	  // GATEWAY:
	  
#ifdef	_DHCP_ENABLED_

	  // show current setting
	  LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, strlen(msg_DHCP), (UINT8 *)msg_DHCP );
	  
	  if( PED_GetStateDHCP() )
	    LIB_LCD_Puts( 1, strlen("msg_DHCP"), FONT0, strlen("ON"), (UINT8 *)"ON" );
	  else
	    LIB_LCD_Puts( 1, strlen("msg_DHCP"), FONT0, strlen("OFF"), (UINT8 *)"OFF" );
	  
	  // enable DHCP?
	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_ENABLE_DHCP), (UINT8 *)msg_ENABLE_DHCP );
	  select = LIB_WaitKeyYesNo( 2, strlen(msg_ENABLE_DHCP) );
	  
	  LIB_LCD_ClearRow( 2, 1, FONT0 );
	  
	  LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, strlen(msg_DHCP), (UINT8 *)msg_DHCP );
	  
	  result = api_lan_setup_DHCP( select );	// force to exec DHCP
	  if( !select )
	    goto DHCP_END;
	    
	  
	  if( result != apiOK )
	    {
	    LIB_LCD_Puts( 1, strlen("msg_DHCP"), FONT0, strlen(os_msg_ERROR), (UINT8 *)os_msg_ERROR );
	    
	    LIB_WaitKey();
	    return( TRUE );
	    }
	  else
	    {
	    data = 600;	// DHCP timeout 60 sec
	    while(1)
	         {
	         if( LIB_GetKeyStatus() == apiReady )
	           {
	           if( LIB_WaitKey() == 'x' )	// abort?
	             {
	             api_lan_setup_DHCP( FALSE );
	             goto DHCP_END;
	             }
	           }
	           
	         if( api_lan_status_DHCP() == apiReady )
	           {
	           LIB_LCD_Puts( 1, strlen("msg_DHCP"), FONT0, sizeof(os_msg_OK), (UINT8 *)os_msg_OK );
	           break;
	           }
	         else
	           {
	           if( data-- )
//	             BSP_Delay_n_ms(100);
	             LIB_WaitTime(10);
	           else
	             {
	             LIB_LCD_Puts( 1, strlen("msg_DHCP"), FONT0, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR );
	             
	             DIAG_FUNC_DumpLogForDHCP();
	             
	             LIB_WaitKey();
	             return( TRUE );
	             }
	           }
	         }
	    }
	  
	  LIB_WaitTimeAndKey(100);
	  
	  DIAG_FUNC_DumpLogForDHCP();
	  
DHCP_END:
	  LIB_LCD_ClearRow( 1, 1, FONT0 );
	  
#endif	// _DHCP_ENABLED_

	  
	  // setup default MAC
#if	0
	  data = BSP_RD32(PMU_ID0_REG);	// binary format: CUST_ID(1) + SN(3)
	  
#if	0
	  MAC[0] = 0x00;	// SYMLINK OUI = 00 F0 FF (test only)
	  MAC[1] = 0xF0;	//
	  MAC[2] = 0xFF;	//
	  
	  MAC[3] = (data & 0x00FF0000) >> 16;
	  MAC[4] = (data & 0x0000FF00) >> 8;
	  MAC[5] =  data & 0x000000FF;
#else
	  MAC[0] = 0x6C;	// SYMLINK OUI = 6C 15 24 Dx (formal 28 bits)
	  MAC[1] = 0x15;	//
	  MAC[2] = 0x24;	//
	  MAC[3] = 0xD0;	//
	
	  MAC[3] |= ((data & 0x000F0000) >> 16);
	  MAC[4] =   (data & 0x0000FF00) >> 8;
	  MAC[5] =    data & 0x000000FF;
#endif

	  API_setMAC( MAC );
#endif

	  api_lan_getIPconfig( &config );
	  
	  // enter EDC IP
	  LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_IP), (UINT8 *)os_msg_IP );
	  
	  if( (config.IP != 0) && (config.IP_Len < 16) )
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, config.IP_Len, config.IP );
	    
//	  while(1)
//	  {
	  if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 3, 15, ip ) != TRUE )
	    return( TRUE );
	  
	  if( (ip[0] == 0) && (config.IP != 0) && (config.IP_Len < 16) )
	    {	// old
	    ip[0] = config.IP_Len;
	    memmove( &ip[1], config.IP, ip[0] );
	    }
	  else
	    {	// new
	    config.IP_Len = ip[0];
	    memmove( config.IP, &ip[1], ip[0] );
	    }
//	  }

	  // enter SUBNET_MASK
	  LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_SUBNET_MASK), (UINT8 *)os_msg_SUBNET_MASK );

	  if( (config.SubnetMask_Len != 0) && (config.SubnetMask_Len < 16) )
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, config.SubnetMask_Len, config.SubnetMask );

//	  while(1)
//	  {
	  if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 3, 15, ip ) != TRUE )
	    return( TRUE );
	  
	  if( (ip[0] == 0) && (config.SubnetMask_Len != 0) && (config.SubnetMask_Len < 16) )
	    {	// old
	    ip[0] = config.SubnetMask_Len;
	    memmove( &ip[1], config.SubnetMask, ip[0] );
	    }
	  else
	    {	// new
	    config.SubnetMask_Len = ip[0];
	    memmove( config.SubnetMask, &ip[1], ip[0] );
	    }
//	  }
	  
	  
	  // enter GATEWAY
	  LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_GATEWAY), (UINT8 *)os_msg_GATEWAY );
	  
	  if( (config.Gateway_Len != 0) && (config.Gateway_Len < 16) )
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, config.Gateway_Len, config.Gateway );
	  
//	  while(1)
//	  {
	  if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 3, 15, ip ) != TRUE )
	    return( TRUE );
	  
	  if( (ip[0] == 0) && (config.Gateway_Len != 0) && (config.Gateway_Len < 16) )
	    {	// old
	    ip[0] = config.Gateway_Len;
	    memmove( &ip[1], config.Gateway, ip[0] );
	    }
	  else
	    {	// new
	    config.Gateway_Len = ip[0];
	    memmove( config.Gateway, &ip[1], ip[0] );
	    }
//	  }
	  
	  if( api_lan_setIPconfig( config ) != apiOK )
	    return( FALSE );
	  
	  LIB_LCD_ClearRow( 1, 7, FONT0 );
	  }
	
	
	// PING...
	LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_PING), (UINT8 *)os_msg_PING );
	// enter IP address
	do{
	  if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 3, 15, ip ) != TRUE )
	    return( TRUE );
	  }while( ip[0] == 0 );
	
	while(1)
	  {
	  LIB_LCD_ClearRow( 4, 1, FONT0 );
	  
	  if( api_lan_ping( ip[0], &ip[1], &timeUS ) == apiOK )
	    {
//	    LIB_DispHexWord( 3, 0, (UINT16)timeUS );
	    
	    int_1 = timeUS/1000;
	    remainder = timeUS - int_1*1000;
	    dec_1 = remainder/100;
	    remainder -= (dec_1*100);
	    dec_2 = remainder/10;
	    
	    len = 0;
	    len1 = LIB_itoa( int_1, abuf );
	    memmove( buffer, abuf, len1 );
	    
	    buffer[len1++] = '.';
	    len += len1;
	    
	    len1 = LIB_itoa( dec_1, abuf );
	    memmove( &buffer[len], abuf, len1 );
	    len += len1;
	    
	    len1 = LIB_itoa( dec_2, abuf );
	    memmove( &buffer[len], abuf, len1 );
	    len += len1;
	    
	    buffer[len++] = 'm';
	    buffer[len++] = 's';
	    
	    LIB_LCD_Puts( 4, 0, FONT0, len, buffer );
	    
	    if( LIB_WaitTimeAndKey( 300 ) != 255 )	// repeat every 3 seconds till any key stroke
	      break;
	    }
	  else
	    {
	    LIB_LCD_Puts( 4, 0, FONT0, sizeof(os_msg_ERR_NO_RESPONSE), (UINT8 *)os_msg_ERR_NO_RESPONSE );
	    if( LIB_WaitTimeAndKey( 300 ) != 255 )	// repeat every 3 seconds till any key stroke
	      break;
	    }
	  }

//#endif

	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 22 - Test LCD back-light.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_TestBackLight( void )
{
#if	0
UINT32	i;


	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_BACK_LIGHT), (UINT8 *)os_msg_BACK_LIGHT );
	
	for( i=0; i<2; i++ )
	   {
	   OS_LCD_BackLight( 0x00000000 );	// turn OFF
	   BSP_Delay_n_ms( 1000 );		// delay 1 sec
	
	   OS_LCD_BackLight( 0xFFFFFFFF );	// turn ON
	   BSP_Delay_n_ms( 1000 );		// delay 1 sec
	   }
  
	return( TRUE );
#endif
	return( FALSE );
}
#endif

// ---------------------------------------------------------------------------
#ifdef	_4G_ENABLED_
void	DIAG_GPRS_CheckOP( UINT8 *buf, UINT8 *act )
{
UINT32	len;


	if( api_lte_checkOP( buf, act ) == apiReady )
	  {
//	  fOP = TRUE;
	  
	  if( buf[0] > 21 )
	    buf[0] = 21;
	  LIB_LCD_Puts( 4, 0, FONT0+attrCLEARWRITE, buf[0], &buf[1] );	// OP name
	  
	  switch( *act )
	        {
	        case RAT_GSM:		// "2G"
	             LIB_LCD_Puts( 5, 0, FONT0+attrCLEARWRITE, strlen("2G"), (UINT8 *)"2G" );
	             break;
	        
	        case RAT_UTRAN:		// "3G"
	             LIB_LCD_Puts( 5, 0, FONT0+attrCLEARWRITE, strlen("3G"), (UINT8 *)"3G" );
	             break;
	             
	        case RAT_GSM_EGRPS:	// "E"	(2.5G)
	             LIB_LCD_Puts( 5, 0, FONT0+attrCLEARWRITE, strlen("E "), (UINT8 *)"E " );
	             break;
	             
	        case RAT_UTRAN_HSDPA:	// "H"	(3.5G)
	             LIB_LCD_Puts( 5, 0, FONT0+attrCLEARWRITE, strlen("H "), (UINT8 *)"H " );
	             break;
	         
	        case RAT_UTRAN_HSDPA_HSUPA:
	             LIB_LCD_Puts( 5, 0, FONT0+attrCLEARWRITE, strlen("H+"), (UINT8 *)"H+" );
	             break;
	        
	        case RAT_E_UTRAN:	// "4G"	(3.9/4G)
	             LIB_LCD_Puts( 5, 0, FONT0+attrCLEARWRITE, strlen("4G"), (UINT8 *)"4G" );
	             break;
	        }
	  }
	else
	  {
	  LIB_LCD_Puts( 4, 0, FONT0+attrCLEARWRITE, strlen("(NA)"), (UINT8 *)"(NA)" );
	  LIB_LCD_Puts( 5, 0, FONT0+attrCLEARWRITE, strlen("(NA)"), (UINT8 *)"(NA)" );
	  }

#if	0
	if( api_wcdma_checkCELL( buf ) == apiOK )
	  {
	  len = buf[0] + buf[1]*256;
	  if( len )
	    {
	    LIB_DumpHexData2( 1, 7, len, &buf[2] );
//	    LIB_DumpHexData2( 1, 7, len-18, &buf[12] );
	    }
	  }
#endif
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 23 - Test GPRS module. (2G: SEIMENS MC-55, 3G: CINTERION EHS5)
//		1=SIM
//		2=SIGNAL
//		3=ONLINE
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#ifdef	_4G_ENABLED_
UINT32	DIAG_FUNC_TestGPRS( void )
{
UINT32	status = FALSE;
UINT32	result;
UINT32	fOP;
UINT32	len;
UINT8	key;
UINT8	key2;
UINT8	quality;
UINT8	buf[300];
UINT8	simPIN[16];
API_LTE_PROFILE profile;

UINT8	msg_POWER_ON[]		= {"POWER ON..."};
UINT8	msg_POWER_OFF[]		= {"POWER OFF..."};
UINT8	msg_1SIM[]		= {"1-SIM"};
UINT8	msg_2SIGNAL[]		= {"2-SIGNAL"};
UINT8	msg_3ONLINE[]		= {"3-UPDATE"};
UINT8	msg_4RAT[]		= {"4-RAT"};
UINT8	msg_5RBAND[]		= {"5-RAT/BAND"};
UINT8	msg_PIN_READY[]		= {"OK: PIN READY"};
UINT8	msg_PIN_REQUIRED[]	= {"OK: PIN REQUIRED"};
UINT8	msg_REMOVE_PIN[]	= {"REMOVE PIN"};
UINT8	msg_OK[]		= {"OK"};
UINT8	msg_ERROR[]		= {"ERROR"};
UINT8	msg_0_31[]		= {"(0~31)"};
UINT8	msg_1_SIM1_2_SIM2[]	= {"1=SIM1, 2=SIM2"};
UINT8	msg_SIM1[]		= {"SIM1"};
UINT8	msg_SIM2[]		= {"SIM2"};
UINT8	msg_ENTER_SIM_PIN[]	= {"ENTER SIM PIN:"};
UINT8	msg_PIN_ERROR[]		= {"PIN ERROR"};
UINT8	msg_SETUP_FAILED[]	= {"SETUP FAILED"};
UINT8	msg_PROCESSING[]	= {"PROCESSING..."};


	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_4G_LTE), (UINT8 *)os_msg_4G_LTE );
	
#if	0
	LIB_LCD_Puts( 1, 0, FONT0, strlen(msg_1_SIM1_2_SIM2), (UINT8 *)msg_1_SIM1_2_SIM2 );
	
	if( os_GPRS_SIM_SLOT == 0 )
	  LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_SIM1), (UINT8 *)msg_SIM1 );
	else
	  LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_SIM2), (UINT8 *)msg_SIM2 );
	  
	do{
	  key2 = LIB_WaitKey();
	  } while( (key2 != '1') && (key2 != '2') && (key2 != 'y') && (key2 != 'x') );

	if( key2 == 'x' )
	  return( TRUE );
	  
	if( key2 == 'y' )
	  key2 = os_GPRS_SIM_SLOT + '1';
	
	if( key2 == '1' )
	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(msg_SIM1), (UINT8 *)msg_SIM1 );
	else
	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(msg_SIM2), (UINT8 *)msg_SIM2 );
	  
	api_lte_selectSIM( key2 - '1' );	// select SIM slot
	
	LIB_LCD_ClearRow( 1, 3+1, FONT0 );
#endif

	LIB_LCD_Puts( 1, 0, FONT0, strlen(msg_POWER_ON)-3, (UINT8 *)msg_POWER_ON );
	
//	apk_dbm_SelectWirelessChannel(0);	// switch COM1 to LTE
	
	if( !LIB_WaitKeyYesNo( 1, strlen(msg_POWER_ON)-3 ) )
	  goto START;
	  
	LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(msg_POWER_ON), (UINT8 *)msg_POWER_ON );
	if( api_lte_powerON() == apiOK )
	  {
START:
	  do{
	    LIB_LCD_Cls();
	    LIB_LCD_Puts( 0, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_4G_LTE), (UINT8 *)os_msg_4G_LTE );
	    LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(msg_1SIM), (UINT8 *)msg_1SIM );
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(msg_2SIGNAL), (UINT8 *)msg_2SIGNAL );
//	    LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, sizeof(msg_3ONLINE), (UINT8 *)msg_3ONLINE );
//	    LIB_LCD_Puts( 4, 0, FONT0+attrCLEARWRITE, sizeof(msg_4RAT), (UINT8 *)msg_4RAT );
//	    LIB_LCD_Puts( 5, 0, FONT0+attrCLEARWRITE, sizeof(msg_5RBAND), (UINT8 *)msg_5RBAND );
	    	  
	    key = LIB_WaitKey();
	    switch( key )
	        {
	        case '1':	// TEST SIM

SIM_STATUS:
	             LIB_LCD_Cls();
	             LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_1SIM)-2, (UINT8 *)msg_1SIM+2 );
            
	             LIB_LCD_Puts( 3, 0, FONT0, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING );
	             
	             status = api_lte_checkPIN();
	             if( status == apiFailed )
	               LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, sizeof(msg_ERROR), (UINT8 *)msg_ERROR );
	             else
	               {
	               if( status == apiReady )
	               	 {
	                 LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, sizeof(msg_PIN_READY), (UINT8 *)msg_PIN_READY );
#if	0
	                 // 2020-08-19, show local IP address
	                 LIB_LCD_Puts( 5, 0, FONT0+attrCLEARWRITE, strlen("IP: "), (UINT8 *)"IP: " );
	                 if( api_lte_checkIP( buf ) == apiOK )
	                   LIB_LCD_Puts( 5, strlen("IP: "), FONT0, buf[0], &buf[1] );
	                 else
	                   LIB_LCD_Puts( 5, strlen("IP: "), FONT0, strlen("(NA)"), (UINT8 *)"(NA)" );
#endif
	                 }
	               else
	               	 {
	                 LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, sizeof(msg_PIN_REQUIRED), (UINT8 *)msg_PIN_REQUIRED );

	                 // 2020-05-04
	                 LIB_LCD_Puts( 4, 0, FONT0+attrCLEARWRITE, sizeof(msg_REMOVE_PIN), (UINT8 *)msg_REMOVE_PIN );
	                 if( LIB_WaitKeyYesNo( 4, strlen(msg_REMOVE_PIN) ) )
	                   {
	                   LIB_LCD_Puts( 4, 0, FONT0+attrCLEARWRITE, strlen(msg_ENTER_SIM_PIN), (UINT8 *)msg_ENTER_SIM_PIN );
	                   if( LIB_GetNumKey( 0, NUM_TYPE_STAR+NUM_TYPE_LEADING_ZERO+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 5, 12, simPIN ) )
	               	     {
			     LIB_LCD_Puts( 6, 0, FONT0, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING );
			     
	               	     memset( &profile, 0x00, sizeof(API_LTE_PROFILE) );
	               	     profile.PIN_len = simPIN[0];
	               	     memmove( profile.PIN, &simPIN[1], simPIN[0] );
	               	     profile.ConProfile[0].ConProfileID = 0xFF;
	               	     profile.SrvProfile[0].SrvProfileID = 0xFF;
	               	     
	               	     result = api_lte_setup( profile );
	                     if( result == apiErrorInput )
	                       {
	                       LIB_LCD_Puts( 7, 0, FONT0, strlen(msg_PIN_ERROR), (UINT8 *)msg_PIN_ERROR );
//	                       LIB_WaitKey();
	                       break;
	                       }
	                     if( result == apiFailed )
	                       {
	                       LIB_LCD_Puts( 7, 0, FONT0, strlen(msg_SETUP_FAILED), (UINT8 *)msg_SETUP_FAILED );
//	                       LIB_WaitKey();
	                       break;
	                       }
#if	0
//			     LIB_LCD_ClearRow( 2, 3, FONT0 );

	               	     memset( buf, 0x00, sizeof(buf) );
	               	     memmove( buf, &simPIN[1], simPIN[0] );
	               	     
	               	     if( apk_wcdma_unlockPIN( buf ) == apiOK )
			       LIB_LCD_Puts( 7, 0, FONT0+attrCLEARWRITE, sizeof(msg_OK), (UINT8 *)msg_OK );
	               	     else
	               	       LIB_LCD_Puts( 7, 0, FONT0+attrCLEARWRITE, sizeof(msg_ERROR), (UINT8 *)msg_ERROR );

			     LIB_WaitKey();
			     goto SIM_STATUS;
#endif
	               	     }
	                   }
	                 else
	                   {
//	                   LIB_LCD_ClearRow( 4, 1, FONT0 );
	                   continue;
	                   }
	                 }
	               }
	             
	             break;
	             
	        case '2':	// TEST SIGNAL
	        
	             LIB_LCD_Cls();
	             LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_2SIGNAL)-2, (UINT8 *)msg_2SIGNAL+2 );
	             LIB_LCD_Puts( 0, 9, FONT0, strlen(msg_0_31), (UINT8 *)msg_0_31 );

	             // 2017-05-11, enter SIM PIN when required
	             status = api_lte_checkPIN();
	             if( status == apiNotReady )
	               {
	               LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_ENTER_SIM_PIN), (UINT8 *)msg_ENTER_SIM_PIN );
	               if( LIB_GetNumKey( 0, NUM_TYPE_STAR+NUM_TYPE_LEADING_ZERO+NUM_TYPE_LEFT_JUSTIFY, '_', FONT0, 3, 12, simPIN ) != TRUE )
	               	 break;
	               
	               LIB_LCD_ClearRow( 2, 2, FONT0 );
	               LIB_LCD_Puts( 2, 0, FONT0, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING );
	               
	               memset( &profile, 0x00, sizeof(API_LTE_PROFILE) );
	               profile.PIN_len = simPIN[0];
	               memmove( profile.PIN, &simPIN[1], simPIN[0] );
	               profile.ConProfile[0].ConProfileID = 0xFF;
	               profile.SrvProfile[0].SrvProfileID = 0xFF;
	               
	               result = api_lte_setup( profile );
	               if( result == apiErrorInput )
	               	 {
	               	 LIB_LCD_Puts( 4, 0, FONT0, strlen(msg_PIN_ERROR), (UINT8 *)msg_PIN_ERROR );
	               	 LIB_WaitKey();
	               	 break;
	               	 }
	               if( result == apiFailed )
	               	 {
	               	 LIB_LCD_Puts( 4, 0, FONT0, strlen(msg_SETUP_FAILED), (UINT8 *)msg_SETUP_FAILED );
	               	 LIB_WaitKey();
	               	 break;
	               	 }
	               	 
	               LIB_LCD_ClearRow( 2, 3, FONT0 );
	               }

		     LIB_WaitTime( 20 );	// io delay between two commands
		     
		     fOP = FALSE;
	             while(1)
	                  {
	                  if( api_lte_checkSIGNAL( &quality ) == apiOK )
	                    {
	                    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(msg_OK), (UINT8 *)msg_OK );
	                    
	                    if( quality != 99 )	// not known or not detectable?
	                      {
	                      memset( buf, 0x00, sizeof(buf) );
	                      len = LIB_itoa( quality, buf );
	                      LIB_LCD_Puts( 2, 10, FONT0, len, buf );	// show signal level
	                      
	                      // 2017-05-11, display OP
//	                      if( !fOP )
//	                      	{
				LIB_WaitTime( 20 );	// io delay between two commands

				DIAG_GPRS_CheckOP( buf, &key );
//	                      	}
	                      }
	                    else
	                      {
	                      LIB_LCD_Puts( 2, 10, FONT0, strlen("SCANNING..."), (UINT8 *)"SCANNING..." );
	                      
	                      DIAG_GPRS_CheckOP( buf, &key );
	                      }
	                      
	                    if( LIB_WaitTimeAndKey( 200 ) != 255 )
	                      break;
//	                    LIB_BUZ_Beep1();
	                    }
	                  else
	                    {
	                    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR );
	                    break;
	                    }
	                  }
	                  
	             break;
	             
	        case '3':	// JAVA UPDATE
	             
//	             DIAG_FUNC_UpdateMeJAVA();
	             break;
	             
	        case '4':
	             
//		     DIAG_FUNC_SelectRAT();
	             break;
	             
//	             LIB_DispHexByte( 0, 0, apk_wcdma_lockPIN( "8888" ) );	// default PIN, CHT="0000", FET="8888"
//	             LIB_WaitKey();	             
//	             break;

		case '5':
		     
//		     DIAG_FUNC_SelectRBAND();
		     break;
	        }
		     
	    if( key == '1' )
	      LIB_WaitKey();
	      
	    }while( key != 'x' );
	    
	  status = TRUE;
	  }
	
	if( status )
	  {
	  LIB_LCD_ClearRow( 1, 3+2, FONT0 );
	  LIB_LCD_Puts( 1, 0, FONT0, strlen(msg_POWER_OFF)-3, (UINT8 *)msg_POWER_OFF );
	  if( LIB_WaitKeyYesNo( 1, strlen(msg_POWER_OFF)-3 ) )
	    {
	    LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, sizeof(msg_POWER_OFF), (UINT8 *)msg_POWER_OFF );
	    if( api_lte_powerOFF() != apiOK )
	      status = FALSE;
	    }
	  }
	
	return( status );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Power saving test.
//	     L0:
//	        (1) IO
//		(2) GPRS/WCDMA
//		(3) LAN (if not in use)
//	     M3:
//		(1) MCU
//		(2) TFTLCD BACKLIGHT
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	DIAG_PowerSaving( void )
{
UINT8	msg_POWER_SAVING[] =	{"POWER SAVING"};
UINT8	msg_1_LCD_BL[] =	{"1-LCD BL"};
UINT8	msg_2_KBD_BL[] =	{"2-KBD BL"};
UINT8	msg_3_3G[]     =	{"3-4G"};
UINT8	msg_4_SAM1[]    =	{"4-SAM1"};
UINT8	msg_0_START[]  =	{"0-START"};
UINT8	msg_WAITING_5SEC[] =	{"WAITING FOR 5 SEC..."};
UINT8	msg_HIT_ANY_KEY[] =	{"HIT ANY KEY TO START"};
UINT8	msg_0_OFF[]	=	{"0-OFF"};
UINT8	msg_1_ON[]	=	{"1-ON"};
UINT8	msg_POWER_ON[]		= {"POWER ON..."};
UINT8	msg_POWER_OFF[]		= {"POWER OFF..."};
UINT8	msg_OK[]		= {"OK"};
UINT8	msg_ERROR[]		= {"ERROR"};
UINT8	msg_OFF_TIME[]		= {"OFF TIME? (unit:10ms)"};
UINT8	msg_ON_TIME[]		= {"ON TIME? (unit:10ms)"};
UINT8	key;
UINT8	result;

UINT32	kbd_bl = 0;
UINT32	lcd_bl = 0xFFFFFFFF;

//UINT8	dhn_sam1 = 0;
UINT8	atr_sam1[34] = {0};
UINT8	buf[8];
UINT8	len;


START:
	if( OS_PMU_DcPowerIn() )	// DC power in?
	  return;	// yes
	  
	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_POWER_SAVING), (UINT8 *)msg_POWER_SAVING );
	
	LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_0_START), (UINT8 *)msg_0_START );
//	LIB_LCD_Puts( 3, 0, FONT0, strlen(msg_1_LCD_BL), (UINT8 *)msg_1_LCD_BL );
//	LIB_LCD_Puts( 4, 0, FONT0, strlen(msg_2_KBD_BL), (UINT8 *)msg_2_KBD_BL );
//	LIB_LCD_Puts( 5, 0, FONT0, strlen(msg_3_3G), (UINT8 *)msg_3_3G );
//	LIB_LCD_Puts( 6, 0, FONT0, strlen(msg_4_SAM1), (UINT8 *)msg_4_SAM1 );
	
	do{
	  key = LIB_WaitKey();
//	  } while( (key != '0') && (key != '1') && (key != '2') && (key != '3') && (key != '4') && (key != 'x') );
	  } while( (key != '0') && (key != 'x') );
	
	switch( key )
	      {
	      case '0':
	      	   
	      	   LIB_LCD_Cls();

	      	   // start...
	      	   LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_WAITING_5SEC), (UINT8 *)msg_WAITING_5SEC );
	      	   
	      	   api_pmu_setup( 0xFFFFFFFF, 5, 0xFFFFFFFF );
	      	   	      	   
	      	   LIB_WaitKey();
	      	   
	      	   api_pmu_setup( 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF );
	      	   
	      	   break;

#if	0
	      case '1':
	      	   
	      	   LIB_LCD_Cls();
	      	   
	      	   LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_1_LCD_BL)-2, (UINT8 *)msg_1_LCD_BL+2 );
	      	   LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_0_OFF), (UINT8 *)msg_0_OFF );
	      	   LIB_LCD_Puts( 3, 0, FONT0, strlen(msg_1_ON), (UINT8 *)msg_1_ON );
	      	   
	      	   do
	      	   {
	      	   key = LIB_WaitKey();
	      	   if( key == '0' )
	      	     {
//	      	     BSP_WR32( (BspLcd.pLcdData->Base + GPIO_OUT_CLR), 0x00000040 );	// turn OFF
		     OS_LCD_BackLight( 0 );
	      	     lcd_bl = 0;
	      	     }
	      	   else
	      	     {
	      	     if( key == '1' )
	      	       {
//	      	       BSP_WR32( (BspLcd.pLcdData->Base + GPIO_OUT_SET), 0x00000040 );	// turn ON
		       OS_LCD_BackLight( 0xFFFFFFFF );
	      	       lcd_bl = 0xFFFFFFFF;
	      	       }
	      	     }
	      	   } while( key != 'x' );
	      	   
	      	   break;
	      
	      case '2':
	      	
	      	   LIB_LCD_Cls();
	      	   
	      	   LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_2_KBD_BL)-2, (UINT8 *)msg_2_KBD_BL+2 );
	      	   LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_0_OFF), (UINT8 *)msg_0_OFF );
	      	   LIB_LCD_Puts( 3, 0, FONT0, strlen(msg_1_ON), (UINT8 *)msg_1_ON );
	      	   
	      	   do
	      	   {
	      	   key = LIB_WaitKey();
	      	   if( key == '0' )
	      	     {
	      	     BSP_WR32( (BspLcd.pLcdData->Base + GPIO_OUT_CLR), 0x00000001 );	// turn OFF
	      	     lcd_bl = 0;
	      	     }
	      	   else
	      	     {
	      	     if( key == '1' )
	      	       {
	      	       BSP_WR32( (BspLcd.pLcdData->Base + GPIO_OUT_SET), 0x00000001 );	// turn ON
	      	       lcd_bl = 0xFFFFFFFF;
	      	       }
	      	     }
	      	   } while( key != 'x' );
	      	   
	      	   break;
	      	   
	      case '3':
	      	   
	      	   while(1)
	      	   {
	      	   LIB_LCD_Cls();
	      	   
	      	   LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_3_3G)-2, (UINT8 *)msg_3_3G+2 );
	      	   LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_0_OFF), (UINT8 *)msg_0_OFF );
	      	   LIB_LCD_Puts( 3, 0, FONT0, strlen(msg_1_ON), (UINT8 *)msg_1_ON );
	      	   
	      	   key = LIB_WaitKey();
	      	   if( key == 'x' )
	      	     break;
	      	     
	      	   if( key == '0' )
	      	     {
	      	     LIB_LCD_ClearRow( 3, 1, FONT0 );
	      	     LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(msg_POWER_OFF), (UINT8 *)msg_POWER_OFF );
	      	     result = api_lte_powerOFF();
	      	     }
	      	   else
	      	     {
	      	     if( key == '1' )
	      	       {
	      	       LIB_LCD_ClearRow( 3, 1, FONT0 );
	      	       LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(msg_POWER_ON), (UINT8 *)msg_POWER_ON );
	      	       result = api_lte_powerON();
	      	       }
	      	     else
	      	       continue;
	      	     }
	      	     
	      	   if( result == apiOK )
	      	     LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, sizeof(msg_OK), (UINT8 *)msg_OK );
	      	   else
	      	     LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, sizeof(msg_ERROR), (UINT8 *)msg_ERROR );
	      	     
	      	   LIB_WaitKey();
	      	   }

		   break;

	      case '4':
	      	   
	      	   while(1)
	      	   {
	      	   LIB_LCD_Cls();
	      	   
	      	   LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_4_SAM1)-2, (UINT8 *)msg_4_SAM1+2 );
	      	   LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_0_OFF), (UINT8 *)msg_0_OFF );
	      	   LIB_LCD_Puts( 3, 0, FONT0, strlen(msg_1_ON), (UINT8 *)msg_1_ON );
	      	   
	      	   key = LIB_WaitKey();
	      	   if( key == 'x' )
	      	     break;
	      	     
	      	   if( key == '0' )
	      	     {
	      	     LIB_LCD_ClearRow( 3, 1, FONT0 );
	      	     LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(msg_POWER_OFF), (UINT8 *)msg_POWER_OFF );
	      	     
	      	     if( POST_dhn_sam1 != 0 )
	      	       {
	      	       result = api_ifm_deactivate( POST_dhn_sam1 );
	      	       result = api_ifm_close( POST_dhn_sam1 );
	      	       POST_dhn_sam1 = 0;
	      	       }
	      	     else
		       result = apiOK;
	      	     }
	      	   else
	      	     {
	      	     if( key == '1' )
	      	       {
	      	       LIB_LCD_ClearRow( 3, 1, FONT0 );
	      	       LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(msg_POWER_ON), (UINT8 *)msg_POWER_ON );
	      	       
	      	       if( POST_dhn_sam1 != 0 )
	      	       	 result = api_ifm_reset( POST_dhn_sam1, 0, atr_sam1 );
	      	       else
	      	       	 {
	      	       	 POST_dhn_sam1 = api_ifm_open( SAM1 );
	      	       	 if( POST_dhn_sam1 != apiOutOfService )
	      	       	   result = api_ifm_reset( POST_dhn_sam1, 0, atr_sam1 );
	      	       	 else
	      	       	   result = apiFailed;
	      	       	 }
	      	       }
	      	     else
	      	       continue;
	      	     }
	      	     
	      	   if( result == apiOK )
	      	     LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, sizeof(msg_OK), (UINT8 *)msg_OK );
	      	   else
	      	     LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, sizeof(msg_ERROR), (UINT8 *)msg_ERROR );
	      	     
	      	   LIB_WaitKey();
	      	   }

		   break;
#endif

	      case 'x':
	      	   return;
	      	   
	      }
	
	LIB_LCD_Cls();
	goto START;
}

// ---------------------------------------------------------------------------
// FUNCTION: 24 - Test Battery Power Status.
//		  
//		  01234567890123456789012345678901
//		  BATTERY
//		  
//		  val1 val2
//
//		  sym1 sym2
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
UINT32	DIAG_FUNC_TestBattPower( void )
{
UINT8	msg_1_STATUS[]		= {"1-STATUS"};
UINT8	msg_2_CALIBRATE[]	= {"2-CALIBRATE"};
UINT8	msg_3_CHARGE[]		= {"3-CHARGE"};
UINT8	msg_4_POWER_SAVING[]	= {"4-POWER SAVING"};
UINT8	msg_CALIBRATE[]		= {"CALIBRATE"};
UINT8	msg_WARNING1[]		= {"NEED TO CALIBRATE!"};
UINT8	msg_WARNING2[]		= {"REMOVE DC POWER!"};
UINT8	msg_PER[]		= {"%"};
UINT8	msg_CHARGE_FULL[]	= {"FULLY CHARGED!"};
UINT8	buffer[128];
UINT8	status;
UINT8	key;
UINT8	symbol1, symbol2;
UINT32	value1, value2;
UINT32	Len;
UINT32	bias = 0;
UINT32	percent = 0;
float	voltage;

//UINT8	buffer[100];
//char	*buffer;
char	*ptrbuf;
int	decimal;
int	sign;


	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_BATTERY), (UINT8 *)os_msg_BATTERY );
	
//	if( os_MultiBootMode != OP_MODE_BIN )
//	  {
	  LIB_LCD_Puts( 2, 0, FONT0, sizeof(msg_1_STATUS), (UINT8 *)msg_1_STATUS );
//	  LIB_LCD_Puts( 3, 0, FONT0, sizeof(msg_2_CALIBRATE), (UINT8 *)msg_2_CALIBRATE );
//	  LIB_LCD_Puts( 4, 0, FONT0, sizeof(msg_3_CHARGE), (UINT8 *)msg_3_CHARGE );
	  LIB_LCD_Puts( 5, 0, FONT0, sizeof(msg_4_POWER_SAVING), (UINT8 *)msg_4_POWER_SAVING );

	   do{
	    key = LIB_WaitKey();
//	    } while( (key != '1') && (key != '2') && (key != '3') && (key != '4') && (key != 'x') );
	    } while( (key != '1') && (key != '4') && (key != 'x') );
//	  }
//	else
//	  key = '1';
	  
	LIB_LCD_ClearRow( 2, 2+2, FONT0 );
	
	if( key == 'x' )
	  return( FALSE );

#if	0
	if( key == '3' )
	  {
	  DIAG_ChargeBattery();
	  return( TRUE );
	  }
#endif

#if	1
	if( key == '4' )
	  {
	  DIAG_PowerSaving();
	  return( TRUE );
	  }
#endif
 
#if	0
	if( key == '2' )
	  {
	  if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "2254" ) != TRUE )
	    return( FALSE );

	  status = OS_PMU_PowerStatus( &value1, &value2, &voltage, &percent );
	  if( status & 0x80 )	// check DC power in?
	    {
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(msg_WARNING2), (UINT8 *)msg_WARNING2 );
	    LIB_WaitTimeAndKey( 300 );
	    return( FALSE );
	    }
	  
	  LIB_LCD_Puts( 2, 0, FONT0, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING );
	  bias = DIAG_CalibrateBattPower( TRUE, &percent );	// re-calibrate bias level
	  }
	else
	  {
	  bias = DIAG_CalibrateBattPower( FALSE, &percent );	// read current bias level
	  if( bias == 0xFFFF )
	    {
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(msg_WARNING1), (UINT8 *)msg_WARNING1 );
	    LIB_WaitTimeAndKey( 300 );
	    return( FALSE );
	    }  
	  }
#endif

	LIB_LCD_ClearRow( 2, 1, FONT0 );
	
	do
	 {
#if	0
	 status = OS_PMU_PowerStatus( &value1, &value2, &voltage, &percent );
	 
	 // show original ADC value
	 LIB_DispHexWord( 2, 0, value1 & 0x3FF );
	 LIB_DispHexWord( 2, 5, value2 & 0x3FF );
#endif

	 status = api_pmu_status( &percent );

	 // show real battery voltage value (the function sprintf() has bug in floating conversion)
//	 Len = sprintf( buffer, "%1.2f V", voltage );
//	 ptrbuf = (char *)fcvt( (double)voltage, 2, &decimal, &sign );
//	 LIB_LCD_Puts( 6, 0, FONT0, Len & 0x0F, buffer );

	 // show graphic symbol relative to the battery status
	 //
	 // 0x00 - battery power is GOOD. (icon 0x9A)                                                      
	 // 0x01 - battery power is POOR. (icon 0x9B)                                                      
	 // 0x02 - battery power is FAIR. (icon 0x96)                                                      
	 // 0x80 - DC power here and battery power is GOOD without charging.	(icon 0x99 + icon 0x9A)
	 // 0x81 - DC power here and battery power is in charging.		(icon 0x99 + icon 0x96)        
	 
	 symbol2 = 0x20;
	 symbol1 = 0x20;
	 switch( status )
	       {
	       case 0x00:
	       
	       	   symbol1 = 0x9A;
	       	   break;
	       	   
	       case 0x01:
	            
	            symbol1 = 0x9B;
	            break;
	            
	       case 0x02:
	       
	            symbol1 = 0x96;
	            break;
	            
	       case 0x80:
	       case 0x85:	// good & in charge
	       
	            symbol1 = 0x9A;
	            symbol2 = 0x99;
	            break;
	            
	       case 0x82:
	       
	       	    symbol1 = 0x9B;
	       	    symbol2 = 0x99;
	       	    break;
	            
	       case 0x81:
	       case 0x83:
	       
	            symbol1 = 0x96;
	            symbol2 = 0x99;
	            break;

	       case 0x88:	// 2016-03-07, charge full
	       	    
	       	    symbol1 = 0x9A;
	       	    symbol2 = 0x99;
	       	    break;
	       }
	 
	 LIB_LCD_Putc( 4, 0, FONT0, symbol1 );
	 LIB_LCD_Putc( 4, 5, FONT0, symbol2 );

	 if( status == 0x88 )
	   LIB_LCD_Puts( 6, 0, FONT0, sizeof(msg_CHARGE_FULL), (UINT8 *)msg_CHARGE_FULL );
	 else
	   LIB_LCD_ClearRow( 6, 1, FONT0 );
	   
	 LIB_DispHexByte( 2, 10, status );
	 
	 // show voltage & percentage
//	 memset( buffer, 0x00, sizeof(buffer) );
//	 sprintf( buffer, "%.2f", voltage );
//	 LIB_LCD_Puts( 6, 0, FONT0, strlen(buffer), buffer );
	 
	 memset( buffer, 0x00, sizeof(buffer) );
	 sprintf( buffer, "%d%s", percent, msg_PER );	// 0~100%
	 LIB_LCD_Puts( 5, 0, FONT0+attrCLEARWRITE, strlen(buffer), buffer );

#if	0
	 ftoa( voltage, buffer, 2);
#else
	 voltage = OS_PMU_GetBattVoltage();
//	 printf("voltage= %f\n", voltage);
	 memset( buffer, 0x00, sizeof(buffer) );
	 int ret = snprintf( buffer, sizeof(buffer), "%f", voltage );
#endif

	 buffer[4+1] = 'V';
	 LIB_LCD_Puts( 5, 5, FONT0, 4+1+1, buffer );
	 
#if	0	  
	 if( os_MultiBootMode == OP_MODE_BIN )
	   {
	   LIB_WaitTime(200);
	   break;
	   }
#endif

	 }while( LIB_WaitTimeAndKey(200) == 255 );
	
	return( TRUE );	
}

// ---------------------------------------------------------------------------
// FUNCTION: 25 - Test USB Device module.
//		  
//		  01234567890123456789012345678901
//		  USB - DEVICE
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_TestUSB_DEV( void )
{
UINT8	msg_USB_DEV[] =		{"USB-DEVICE"};
UINT8	msg_HIT_ANY_KEY[] =	{"HIT ANY KEY"};
UINT8	msg_SENDING[] =		{"SENDING..."};
UINT8	msg_RECEIVING[] =	{"RECEIVING..."};
UINT8	sbuf[64];
UINT8	dbuf[64];
UINT8	dhn_usb;
UINT8	result;
UINT32	i;
UINT32	tx_len;
UINT32	rx_len;
//API_USB_PARA	usbpara;
API_USB_PARA	usbpara[1];


	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_USB_DEV), (UINT8 *)msg_USB_DEV );
	
	memset( (UCHAR *)usbpara, 0x00, sizeof(API_USB_PARA) );
	usbpara->ID = FID_USB_OPEN_DEVICE;
	usbpara->CDC = USB_CDC_RS232;
	usbpara->Mode = auxSOH;
	usbpara->Baud = COM_115200 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
	usbpara->Resend = 0;
	usbpara->Acks = 2;
	usbpara->Tor = 100*3;	// 3sec
	dhn_usb = api_usb_open( USB_PORT_DEVICE, (UCHAR *)usbpara );
	if( dhn_usb == apiOutOfService )
	  return( FALSE );
	
	
	LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_HIT_ANY_KEY), (UINT8 *)msg_HIT_ANY_KEY );
	LIB_WaitKey();
	
	LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_SENDING), (UINT8 *)msg_SENDING );
	
//	BSP_Delay_n_ms( 5000 );	// delay 5sec
	
	while( api_usb_txready( dhn_usb ) != apiReady );
	
	tx_len = 16;
	memset( sbuf, 0x55, sizeof(sbuf) );
	sbuf[0] =  tx_len & 0x000000FF;
	sbuf[1] = (tx_len & 0x0000FF00) >> 8;
	sbuf[2] = (tx_len & 0x00FF0000) >> 16;
	sbuf[3] = (tx_len & 0xFF000000) >> 24;
	result = api_usb_txstring( dhn_usb, sbuf );
	if( result == apiOK )
	  {
	  LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_RECEIVING), (UINT8 *)msg_RECEIVING );
	  while( api_usb_rxready( dhn_usb, dbuf ) != apiReady );
	  
	  memset( dbuf, 0x00, sizeof(dbuf) );
	  if( api_usb_rxstring( dhn_usb, dbuf ) == apiOK )
	    {
	    rx_len = dbuf[0] + dbuf[1]*0x100 + dbuf[2]*0x10000 + dbuf[3]*0x1000000;
	    if( rx_len == tx_len )
	      {
	      for( i=0; i<rx_len; i++ )
	         {
	         if( dbuf[i+4] != sbuf[i+4] )
	           {
	           result = apiFailed;
	           break;
	           }
	         }
	      }
	    }
	  }
	
	api_usb_close( dhn_usb );
	
	if( result == apiOK )
	  return( TRUE );
	else
	  return( FALSE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 26 - Test USB Host module.
//		  
//		  01234567890123456789012345678901
//		  USB - HOST
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_TestUSB_HOST( void )
{
UINT8	msg_USB_HOST[] =	{"USB-HOST"};
UINT8	msg_PLUG_PEN_DRIVE[] =	{"PLUG PEN DRIVE"};
UINT8	dhn_usb;
UINT8	result;
//API_USB_PARA	usbpara;
API_USB_PARA	usbpara[1];


	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_USB_HOST), (UINT8 *)msg_USB_HOST );
	
	memset( (UCHAR *)usbpara, 0x00, sizeof(API_USB_PARA) );
	usbpara->ID = FID_USB_OPEN_HOST;
	usbpara->CDC = USB_CDC_MASS_STORAGE;
//	usbpara.Mode = auxSOH;
//	usbpara.Baud = COM_115200 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
//	usbpara.Resend = 0;
//	usbpara.Acks = 2;
//	usbpara.Tor = 100*3;	// 3sec
	dhn_usb = api_usb_open( USB_PORT_HOST, (UCHAR *)usbpara );
	if( dhn_usb == apiOutOfService )
	  return( FALSE );
	
	LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_PLUG_PEN_DRIVE), (UINT8 *)msg_PLUG_PEN_DRIVE );
	
	result = api_usbhost_mfgtest( dhn_usb, (UCHAR *)usbpara );
	if( result == apiOK )
	  result = TRUE;
	else
	  result = FALSE;
	  
	api_usb_close( dhn_usb );
	
	return( result );
}
#endif

// ---------------------------------------------------------------------------
void	DelayForCapacitiveTSC( void )
{
UINT32	msx10 = 5;


	if( !os_TSC_RESIST )
	  LIB_WaitTime(msx10);
}

// ---------------------------------------------------------------------------
// FUNCTION: 27 - Test TSC module.
//		  
//		  01234567890123456789012345678901
//		  TSC
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	1
UINT32	DIAG_FUNC_TestTouchScreen( void )
{
UINT32	result = FALSE;
UINT8	status;
UINT8	dhn_tsc;
API_LCDTFT_RECT	rect;
//API_LCDTFT_RECT	rect[1];
API_TSC_PARA	tscpara;
//API_TSC_PARA	tscpara[1];
UINT8	msg_TSC[] =		{"TOUCH SCREEN"};
UINT8	msg_PLS_TOUCH[] =	{"PLEASE TOUCH..."};
UINT8	info[16];

	
//	TEST_TOUCH();
//	return(1);
	
//	if( !os_TSC_ENABLED )
//	  return( FALSE );
	
//	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_TSC), (UINT8 *)msg_TSC );

//	LIB_BUZ_Beep1();
//	BSP_Delay_n_ms(500);
	
	LIB_LCD_Cls();
	LIB_LCD_Puts( 10-4, 7, FONT0, strlen(msg_PLS_TOUCH), (UINT8 *)msg_PLS_TOUCH );

	dhn_tsc = api_tsc_open( 0 );
	
	// ---------------------------------------------------------------------------
	// fill rectangle : #1 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 0;
	rect.Xend	= 31;
	rect.Ystart	= 0;
	rect.Yend	= 31;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );

	// test rectangle : #1
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 0;
	tscpara.Y	= 0;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );
	  
	// fill rectangle : #1 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 0;
	rect.Xend	= 31;
	rect.Ystart	= 0;
	rect.Yend	= 31;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();

	// ---------------------------------------------------------------------------	
	// fill rectangle : #2 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 104;
	rect.Xend	= 104+32-1;
	rect.Ystart	= 0;
	rect.Yend	= 31;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #2
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 104;
	tscpara.Y	= 0;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #2 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 104;
	rect.Xend	= 104+32-1;
	rect.Ystart	= 0;
	rect.Yend	= 31;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();
	
	// ---------------------------------------------------------------------------	
	// fill rectangle : #3 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 240-32-1;
	rect.Xend	= 240-1;
	rect.Ystart	= 0;
	rect.Yend	= 31;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #3
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 240-32-1;
	tscpara.Y	= 0;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #3 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 240-32-1;
	rect.Xend	= 240-1;
	rect.Ystart	= 0;
	rect.Yend	= 31;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();

	// ---------------------------------------------------------------------------	
	// fill rectangle : #4 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 0;
	rect.Xend	= 31;
	rect.Ystart	= 144;
	rect.Yend	= 144+32-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #4
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 0;
	tscpara.Y	= 144;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #4 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 0;
	rect.Xend	= 31;
	rect.Ystart	= 144;
	rect.Yend	= 144+32-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();

	// ---------------------------------------------------------------------------	
	// fill rectangle : #5 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 104;
	rect.Xend	= 104+32-1;
	rect.Ystart	= 144;
	rect.Yend	= 144+32-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #5
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 104;
	tscpara.Y	= 144;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #5 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 104;
	rect.Xend	= 104+32-1;
	rect.Ystart	= 144;
	rect.Yend	= 144+32-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();

	// ---------------------------------------------------------------------------	
	// fill rectangle : #6 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 240-32-1;
	rect.Xend	= 240-1;
	rect.Ystart	= 144;
	rect.Yend	= 144+32-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #6
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 240-32-1;
	tscpara.Y	= 144;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #6 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 240-32-1;
	rect.Xend	= 240-1;
	rect.Ystart	= 144;
	rect.Yend	= 144+32-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();
	
	// ---------------------------------------------------------------------------	
	// fill rectangle : #7 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 0;
	rect.Xend	= 31;
	rect.Ystart	= 320-32-1;
	rect.Yend	= 320-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #7
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 0;
	tscpara.Y	= 320-32-1;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #7 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 0;
	rect.Xend	= 31;
	rect.Ystart	= 320-32-1;
	rect.Yend	= 320-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();

	// ---------------------------------------------------------------------------	
	// fill rectangle : #8 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 104;
	rect.Xend	= 104+32-1;
	rect.Ystart	= 320-32-1;
	rect.Yend	= 320-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #8
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 104;
	tscpara.Y	= 320-32-1;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #8 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 104;
	rect.Xend	= 104+32-1;
	rect.Ystart	= 320-32-1;
	rect.Yend	= 320-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();
	
	// ---------------------------------------------------------------------------	
	// fill rectangle : #9 (RED)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 240-32-1;
	rect.Xend	= 240-1;
	rect.Ystart	= 320-32-1;
	rect.Yend	= 320-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0x00;
	rect.Palette[2]	= 0x00;
	api_lcdtft_fillRECT( 0, rect );
	
	// test rectangle : #9
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	tscpara.ID	= FID_TSC_STATUS_BUTTON;
	tscpara.X	= 240-32-1;
	tscpara.Y	= 320-32-1;
	tscpara.Width	= 32;
	tscpara.Height	= 32;
	tscpara.RxLen = 1;
	status = 0;
	do{
	  DelayForCapacitiveTSC();
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  }while( status != 1 );

	// fill rectangle : #9 (WHITE)
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	rect.Xstart	= 240-32-1;
	rect.Xend	= 240-1;
	rect.Ystart	= 320-32-1;
	rect.Yend	= 320-1;
	rect.Palette[0]	= 0xFF;
	rect.Palette[1]	= 0xFF;
	rect.Palette[2]	= 0xFF;
	api_lcdtft_fillRECT( 0, rect );
	
	LIB_BUZ_Beep1();
	
	api_tsc_close( dhn_tsc );
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
UCHAR	NXP_Check_SPI_RC663_EX(void)
{
	UCHAR regData=0;
	UCHAR retry = 3;
	UCHAR result = FALSE;
	
//	UT_PutStr(2, 0, FONT0, 20, (UCHAR*)"Reader Disconnected!");

	while (retry--)
	{
		NXP_Read_Register(0x3B, &regData);	//Serial Speed
//		LIB_DispHexByte( 0, 10, regData );
//		LIB_WaitKey();
		if (regData == 0x7A)
		   {
		   	result = TRUE;
			break;
		   }

		LIB_WaitTime(50);
	}

	return( result );
}

// ---------------------------------------------------------------------------
UCHAR APP_POLLING_ST25TB(void)
{
#if	0
	UCHAR	cmdInitiate[2]={0x06,0x00};
	UCHAR	cmdSelect[2]={0x0E,0x00};
	UCHAR	cmdGetUid[1]={0x0B};
	UCHAR	rspCode=0;
	UCHAR	rcvData[32];
	UINT	rcvLen=0;
	
	EMV_WAIT();

	NXP_B_Send(2, cmdInitiate);
	rspCode=NXP_B_Receive(&rcvLen, rcvData);
	if (rspCode == 1)
	{
		EMV_WAIT();
		
		cmdSelect[1]=rcvData[0];
		NXP_B_Send(2, cmdSelect);
		rspCode=NXP_B_Receive(&rcvLen, rcvData);
		if (rspCode == 1)
		{
			EMV_WAIT();
			
			NXP_B_Send(1, cmdGetUid);
			rspCode=NXP_B_Receive(&rcvLen, rcvData);
			if (rspCode == 1)
			{
				return SUCCESS;
			}
		}
	}
	
	EMV_RESET();	// 2022-01-06
#endif
	return FAIL;
}

// ---------------------------------------------------------------------------
ULONG APP_Test_PCD(void)
{
	ULONG	result = FALSE;
	UCHAR	rspCode = FAIL;
	
	os_PCD_TYPE = 0xFF;

	SPI_Open(8000000);
	
	LIB_DispHexByte( 1, 0, 0x00 );
	
	if( !NXP_Check_SPI_RC663_EX() )
		return( result );
		
	LIB_DispHexByte( 1, 0, 0x01 );
		
//	NXP_Load_RC663_Parameter_AS350();
	NXP_Load_RC663_Parameter_TA();		// 2022-01-06
	NXP_Initialize_Reader();
	
	LIB_DispHexByte( 1, 0, 0x02 );

	NXP_Load_Protocol_A();
	NXP_Switch_FieldOn();
	BSP_Delay_n_ms(10);
	
	LIB_DispHexByte( 1, 0, 0x03 );

	pcd_flgTypeA=FALSE;
	pcd_flgTypeB=FALSE;

//	LIB_WaitKey();
//	return( FALSE );

	while (1)
	{
		EMV_POLLING_TypeA();
		if (pcd_flgTypeA == TRUE)
		{
			rspCode=EMV_COLLISION_DETECTION();
			if (rspCode == SUCCESS)
			{
				os_PCD_TYPE = 0;
				
				result=TRUE;
//UT_PutChar(3, 0, 0, 'A');	//Type A
				break;
			}
		}

		EMV_POLLING_TypeB();
		if (pcd_flgTypeB == TRUE)
		{
			rspCode=EMV_COLLISION_DETECTION();
			if (rspCode == SUCCESS)
			{
				os_PCD_TYPE = 1;
				
				result=TRUE;
//UT_PutChar(3, 0, 0, 'B');	//Type B
				break;
			}
		}
		
		rspCode=APP_POLLING_ST25TB();
		if (rspCode == SUCCESS)
		{
			os_PCD_TYPE = 2;
			
			result=TRUE;
//UT_PutChar(3, 0, 0, 'S');	//ST25TB
			break;
		}
	}

	pcd_flgTypeA=FALSE;
	pcd_flgTypeB=FALSE;

	NXP_Switch_FieldOff();
	BSP_Delay_n_ms(100);
	SPI_Close();

	return result;

}

// ---------------------------------------------------------------------------
// FUNCTION: 28 - Test PCD module.
//		  
//		  01234567890123456789012345678901
//		  PCD
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	1
UINT32	DIAG_FUNC_TestRC663( void )
{
UINT32	result = FALSE;
UINT8	msg_PCD[] =		{"PCD"};
UINT8	msg_RC663[] =		{"RC663"};
UINT8	msg_PRESENT_CARD[] =	{"PRESENT CARD"};
UINT8	msg_OK[] =		{"OK"};
UINT8	msg_ERROR[] =		{"ERROR"};
UINT16	rcvLen=0;
UINT8	rcvBuffer[1024]={0};
UINT8	rspCode=0;
API_PCD_ICON icon;


	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_PCD), (UINT8 *)msg_PCD );
//	APP_Test_PCD();
//	LIB_WaitKey();
	
	
	memset( &icon, 0x00, sizeof(API_PCD_ICON) );
	icon.ID = IID_TAP_LOGO;
	api_lcdtft_showPCD( 0, icon );
	
	icon.ID = IID_LED_BLUE;
	icon.BlinkOn	= 50;
	icon.BlinkOff	= 50;
	api_lcdtft_showPCD( 0, icon );
	
	rspCode=SPI_Open(8000000);	
	if (rspCode == FAIL)
	{
	return( FALSE );
	}
	
	//Check RC663
	LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, strlen(msg_RC663), (UINT8 *)msg_RC663 );
	if( !NXP_Check_SPI_RC663_EX() )
	  return( FALSE );

	//Load Parameter
	NXP_Load_RC663_Parameter_AS350();
	
	NXP_Initialize_Reader();

	LIB_LCD_Puts( 1, 0, FONT0+attrCLEARWRITE, strlen(msg_PRESENT_CARD), (UINT8 *)msg_PRESENT_CARD );
	
//		NXP_Switch_FieldOn();	// DEBUG ONLY
//		LIB_WaitKey();
	
#if	0	// EMV Process
	while(1)
	{
//	if( LIB_GetKeyStatus() == apiReady)
//	  {
//	  if( LIB_WaitKey() == 'x' )
//	    {
//	    SPI_Close();
//	    result = TRUE;
//	    break;
//	    }
//	  }
	  
		rspCode=EMV_POLLING();
		if (rspCode == SUCCESS)
		{
			rspCode=EMV_COLLISION_DETECTION();
			if (rspCode == SUCCESS)
			{
				rspCode=EMV_ACTIVATE();
				if (rspCode == SUCCESS)
				{
					LIB_WaitTime(1000);
					
					rspCode=EMV_Select_PPSE(&rcvLen, rcvBuffer);

					EMV_RESET();
					EMV_WAIT();

					if (rspCode == PCD_DEP_SUCCESS)
					{
					//	LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, strlen(msg_OK), (UINT8 *)msg_OK );
						result = TRUE;
						break;
					}
					else
					{
					//	LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, strlen(msg_ERROR), (UINT8 *)msg_ERROR );
						result = FALSE;
						break;
					}
				}
			}
		}

		if (rspCode == FAIL)
		{
			EMV_RESET();
			
		//	LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, strlen(msg_ERROR), (UINT8 *)msg_ERROR );
			result = FALSE;
			break;
		}
	}
#endif

		while (1)
		{
			rspCode=EMV_POLLING();
			if (rspCode == SUCCESS)
			{
				rspCode=EMV_COLLISION_DETECTION();
				if (rspCode == SUCCESS)
				{
					EMV_RESET();
				
				//	return SUCCESS;
					result = TRUE;
					break;
				}
			}

			if (rspCode != SUCCESS)
			{
				EMV_RESET();
				
				result = FALSE;
				break;
			}
		}

	NXP_Switch_FieldOff();
	BSP_Delay_n_ms(100);
	SPI_Close();
	
	icon.ID = IID_PCD_EXIT;
	api_lcdtft_showPCD( 0, icon );
	
	return( result );
}
#endif

// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_TestRC663( void )
{
UINT32	result = FALSE;
UINT8	msg_NFC[]	=	{"NFC"};
UINT8	msg_OK[]	=	{"OK"};
UINT8	msg_ERROR[]	=	{"ERROR"};


	LIB_LCD_Cls();

	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_NFC), msg_NFC );
	
	if( !APP_Test_PCD() )
	  {
	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_ERROR), (UINT8 *)msg_ERROR );
//	  LIB_BUZ_Beep2();
//	  BSP_Delay_n_ms(500);
	  }
	else
	  {
	  if( os_PCD_TYPE == 0 )
	    {
	    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, strlen(msg_OK), (UINT8 *)msg_OK );
//	    LIB_BUZ_Beep1();
//	    LIB_WaitTimeAndKey(50);
	    }
	  }
	
	LIB_WaitKey();
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 29 - Test LED module.
//		  
//		  01234567890123456789012345678901
//		  LED
//		  
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
UINT32	DIAG_FUNC_TestKbdLED( void )
{
UINT32	i;
UINT8	msg_KBD_LED[] =		{"KBD LED"};

	
	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_KBD_LED), (UINT8 *)msg_KBD_LED );

#if	0
	BspLcd.pLcdData->Data    = 0;
	BSP_WR32( (BspLcd.pLcdData->Base + GPIO_OE_SET), 0x00000001 );
	BSP_WR32( (BspLcd.pLcdData->Base + GPIO_EN_SET), 0x00000001 );

	for( i=0; i<3; i++ )
	   {
	   // --- Turn on KEY_LED_EN at LCD_D0 (GPIO2[0]) ---
	   BSP_WR32( (BspLcd.pLcdData->Base + GPIO_OUT_SET), 0x00000001 );	// HI
	   
	   BSP_Delay_n_ms(1000 );
	   
	   // --- Turn off KEY_LED_EN at LCD_D0 (GPIO2[0]) ---
	   BSP_WR32( (BspLcd.pLcdData->Base + GPIO_OUT_CLR), 0x00000001 );	// LO
	   
	   BSP_Delay_n_ms(1000 );
	   }
#else

	for( i=0; i<3; i++ )
	   {
	   // --- Turn on KEY_LED_EN at LCD_D0 (GPIO2[0]) ---
	   EXTIO_GPIO( _KEY_LED_EN_BANK_ADDR, _KEY_LED_EN_GPIO_PORT, _KEY_LED_EN_GPIO_NUM, 1 );	// HI
	   
//	   BSP_Delay_n_ms(1000 );
	   LIB_WaitTime( 100 );
	   
	   // --- Turn off KEY_LED_EN at LCD_D0 (GPIO2[0]) ---
	   EXTIO_GPIO( _KEY_LED_EN_BANK_ADDR, _KEY_LED_EN_GPIO_PORT, _KEY_LED_EN_GPIO_NUM, 0 );	// HI
	   
//	   BSP_Delay_n_ms(1000 );
	   LIB_WaitTime( 100 );
	   }
#endif

	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: 33 - Test hardware reset circuit.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	DIAG_FUNC_TestHardwareReset( void )
{
	LIB_ResetSystem();
	for(;;);
}
#endif

// ---------------------------------------------------------------------------
void	POST_SpeakerInit( void )
{
	system("amixer set 'Playback' 255");
	system("amixer cset numid=14 1");
	system("amixer cset numid=13 127");
}

// ---------------------------------------------------------------------------
void	POST_SpeakerPlayBuzzer( void )
{
//	system("aplay -D plughw:0,0 /usr/share/sounds/alsa/buzzer.wav -v");
//	system("aplay -D plughw:0,0 /usr/share/sounds/alsa/button.wav -v");
	system("aplay -D plughw:0,0 /usr/share/sounds/alsa/KbdKeyTap.wav -v");
//	system("aplay /usr/share/sounds/alsa/KbdKeyTap.wav");
}

// ---------------------------------------------------------------------------
// FUNCTION: 34 - Test speaker. (TBD)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
UINT32	DIAG_FUNC_TestSpeaker( void )
{
	LIB_LCD_Cls();
	
	LIB_LCD_Puts( 0, 0, FONT0, strlen("TEST SPEAKER"), "TEST SPEAKER" );
	
#if	1
	// === SPEAKER ===
	
//	LIB_LCD_Puts( 2, 0, FONT0, strlen("HIT ANY KEY TO START..."), "HIT ANY KEY TO START..." );
	
	system("amixer set 'Playback' 255");
	system("amixer cset numid=14 1");
	system("amixer cset numid=13 127");
	
//	LIB_LCD_Puts( 2, 0, FONT0, strlen("HIT ANY KEY TO EXIT..."), "HIT ANY KEY TO EXIT..." );
	
	while(1)
	{
//	if( LIB_GetKeyStatus() == apiReady )
//	  {
//	  if( LIB_WaitKey() == 'x' )
//	  return;
//	  }
	  
//	LIB_WaitTime( 200 );
	
	LIB_LCD_Puts( 2, 0, FONT0, strlen("PLAYBACK..."), "PLAYBACK..." );
	
//	LIB_WaitTime( 100 );
	
//	system("aplay -D plughw:0,0 /usr/share/sounds/alsa/sample12s.wav -v");

	system("aplay -D plughw:0,0 /usr/share/sounds/alsa/InsertCard.wav -v");	
	LIB_WaitKey();	
	system("aplay -D plughw:0,0 /usr/share/sounds/alsa/InProcessing.wav -v");
	
//	LIB_BUZ_Beep1();
	LIB_LCD_Puts( 2, 0, FONT0, strlen("HIT ANY KEY TO CONTINUE..."), "HIT ANY KEY TO CONTINUE..." );
	
	if( LIB_WaitKey() == 'x' )
	  break;
	  
	LIB_LCD_Puts( 2, 0, FONT0, strlen("                           "), "                           " );
	}
#else
	// === BUZZER IO Test ===
UINT8	key;


	while(1)
	{
	if( LIB_GetKeyStatus() == apiReady )
	  {
	  key = LIB_WaitKey();
	  if( key == 'x' )
	    break;
	  }
	
	if( key == '0' )
	  LIB_WaitTime(50);
	else
	  DelayMS(50);
	  
	OS_BUZ_TurnOn();
	
	if( key == '0' )
	  LIB_WaitTime(50);
	else
	  DelayMS(50);
	  
	OS_BUZ_TurnOff();
	}
	
#endif
	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: 35 - Test SD Card.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UINT32	DIAG_FUNC_TestSDCard( void )
{
UINT32	result = FALSE;
UINT8	msg_SDC[] =	{"SD CARD"};
UINT32	i;
FILE	*linux_file;
UINT8	sbuf[64];
UINT8	dbuf[64];


	LIB_LCD_Cls();
	
	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_SDC), msg_SDC );
	
	// select media to be accessed
	LIB_LCD_Puts( 1, 0, FONT0, strlen("SELECT: "), (UINT8 *)"SELECT: " );
	if( api_fs_select( MEDIA_SD ) == apiOK )
	  LIB_LCD_Puts( 1, strlen("SELECT: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  {
	  LIB_LCD_Puts( 1, strlen("SELECT: "), FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	  goto ERROR;
	  }
	
//	LIB_WaitKey();
	
	// initailize
	LIB_LCD_Puts( 2, 0, FONT0, strlen("INIT: "), (UINT8 *)"INIT: " );
	if( api_fs_init() == apiOK )
	  LIB_LCD_Puts( 2, strlen("INIT: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  {
	  LIB_LCD_Puts( 2, strlen("INIT: "), FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	  goto ERROR;
	  }

//	LIB_WaitKey();

	// create a new file
	LIB_LCD_Puts( 3, 0, FONT0, strlen("CREATE: "), (UINT8 *)"CREATE: " );
	if( api_fs_create( "file1.dat", 0 ) == apiOK )
	  LIB_LCD_Puts( 3, strlen("CREATE: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  {
	  LIB_LCD_Puts( 3, strlen("CREATE: "), FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	  goto ERROR;
	  }

//	LIB_WaitKey();

	// open an existent file
	LIB_LCD_Puts( 4, 0, FONT0, strlen("OPEN: "), (UINT8 *)"OPEN: " );
	linux_file = api_fs_open( "file1.dat", 0 );
	if( linux_file != NULL )
	  LIB_LCD_Puts( 4, strlen("OPEN: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  {
	  LIB_LCD_Puts( 4, strlen("OPEN: "), FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	  goto ERROR;
	  }

//	LIB_WaitKey();

	for( i=0; i<sizeof(sbuf); i++ )
	   sbuf[i] = i+0x21;
	
	// write data to the file
	LIB_LCD_Puts( 5, 0, FONT0, strlen("WRITE: "), (UINT8 *)"WRITE: " );
	result = api_fs_write( linux_file, sbuf, sizeof(sbuf) );
	LIB_DispHexWord( 5, strlen("WRITE: "), result );
	if( result == sizeof(sbuf) )
	  LIB_LCD_Puts( 5, strlen("WRITE: ")+6, FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  {
	  LIB_LCD_Puts( 5, strlen("WRITE: ")+6, FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	  goto ERROR;
	  }

//	LIB_WaitKey();

	// close file
	LIB_LCD_Puts( 6, 0, FONT0, strlen("CLOSE: "), (UINT8 *)"CLOSE: " );
	api_fs_close( linux_file );
	LIB_LCD_Puts( 6, strlen("CLOSE: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	
//	LIB_WaitKey();
	
	// open an existent file
	LIB_LCD_Puts( 7, 0, FONT0, strlen("OPEN: "), (UINT8 *)"OPEN: " );
	linux_file = api_fs_open( "file1.dat", 0 );
	if( linux_file != NULL )
	  LIB_LCD_Puts( 7, strlen("OPEN: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  {
	  LIB_LCD_Puts( 7, strlen("OPEN: "), FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	  goto ERROR;
	  }

//	LIB_WaitKey();

	// read and compare contents of the file
	LIB_LCD_Puts( 8, 0, FONT0, strlen("READ: "), (UINT8 *)"READ: " );
	memset( dbuf, 0x00, sizeof(dbuf) );
	result = api_fs_read( linux_file, dbuf, sizeof(dbuf) );
	LIB_DispHexWord( 8, strlen("READ: "), result );
	
	if( LIB_memcmp( sbuf, dbuf, sizeof(dbuf) ) == 0 )
	  LIB_LCD_Puts( 8, strlen("READ: ")+6, FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  {
	  LIB_LCD_Puts( 8, strlen("READ: ")+6, FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	  goto ERROR;
	  }

//	LIB_WaitKey();

	// close file
	LIB_LCD_Puts( 9, 0, FONT0, strlen("CLOSE: "), (UINT8 *)"CLOSE: " );
	api_fs_close( linux_file );
	LIB_LCD_Puts( 9, strlen("CLOSE: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );

//	LIB_WaitKey();

#if	1
	// delete an existent file
	LIB_LCD_Puts( 10, 0, FONT0, strlen("DELETE: "), (UINT8 *)"DELETE: " );
	if( api_fs_delete( "file1.dat" ) == apiOK )
	  LIB_LCD_Puts( 10, strlen("DELETE: "), FONT0, sizeof("OK"), (UINT8 *)"OK" );
	else
	  {
	  LIB_LCD_Puts( 10, strlen("DELETE: "), FONT0, sizeof("FAIL"), (UINT8 *)"FAIL" );
	  goto ERROR;
	  }
#endif
//	LIB_WaitTime(200);
	LIB_WaitKey();
	result = TRUE;
	goto EXIT;

ERROR:
	result = FALSE;
	LIB_WaitKey();
	
EXIT:
	LIB_LCD_Cls();
	
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: 39 - Test 2D barcode reader.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
UINT32	DIAG_FUNC_Test2DReader( void )
{
UINT32	result = FALSE;
UINT8	msg_2DBCR[] =		{"2D-BCR"};
UINT8	msg_SELECT_COM[] =	{"SELECT COM PORT?"};
UINT8	msg_0_COM0[] =		{"0-COM0"};
UINT8	msg_1_COM1[] =		{"1-COM1"};
UINT8	msg_1_SETUP[] =		{"1-SETUP"};
UINT8	msg_2_TEST[] =		{"2-TEST"};
UINT8	msg_OK[] =		{"OK"};
UINT8	msg_ERROR[] =		{"ERROR"};
UINT8	key;
UINT8	dhn_aux;
UINT8	dhn_tim;
UINT32	model = 0xFFFFFFFF;
UINT32	flag = FALSE;
UINT8	comport = COM0;	// default


	while(1)
	{
	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_2DBCR), msg_2DBCR );
	
	LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_SELECT_COM), msg_SELECT_COM );
	LIB_LCD_Puts( 3, 0, FONT0, strlen(msg_0_COM0), msg_0_COM0 );
	LIB_LCD_Puts( 4, 0, FONT0, strlen(msg_1_COM1), msg_1_COM1 );
	switch( LIB_WaitKey() )
	      {
	      case '0':
	      	   comport = COM0;
	      	   break;
	      	   
	      case '1':
	      	   comport = COM1;
	      	   break;
	      	   
	      case 'x':
	      	   return( FALSE );
	      }
	
	LIB_LCD_ClearRow( 2, 3, FONT0 );
	
	LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_1_SETUP), msg_1_SETUP );
	LIB_LCD_Puts( 3, 0, FONT0, strlen(msg_2_TEST), msg_2_TEST );
	
	if( !flag )
	  {
	  LIB_LCD_Puts( 5, 0, FONT0, strlen("CHECK READER..."), (UINT8 *)"CHECK READER..." );
	  
	  model = DIAG_Verify2DReader( &dhn_aux, &dhn_tim, comport );
	  if( model == 0xFFFFFFFF )
	    {
	    LIB_BUZ_Beep2();
	    
	    LIB_LCD_Puts( 5, 0, FONT0+attrCLEARWRITE, strlen("READER: NOT FOUND"), (UINT8 *)"READER: NOT FOUND" );
	    LIB_WaitKey();
	    
	    api_aux_close( dhn_aux );
	    api_tim_close( dhn_tim );
	  	   
	    return( TRUE );
	    }
	  else
	    {
	    LIB_BUZ_Beep1();
	    
	    if( model == 0x00 )
	      LIB_LCD_Puts( 5, 0, FONT0+attrCLEARWRITE, strlen("READER: EM3085"), (UINT8 *)"READER: EM3085" );
	    else
	      LIB_LCD_Puts( 5, 0, FONT0+attrCLEARWRITE, strlen("READER: QL1601"), (UINT8 *)"READER: QL1601" );
	    }
	  
	  flag = TRUE;
	  }
	
	key = LIB_WaitKey();
	switch( key )
	      {
	      case '1':
	    	   
	    	   LIB_LCD_ClearRow( 2, 2, FONT0 );
	    	   LIB_LCD_Puts( 2, 0, FONT0, strlen("SETUP..."), "SETUP..." );
	    	   
	    	   if( model == 0 )
	    	     result = DIAG_SetupQRC( dhn_aux, dhn_tim );	// setup default config for QRC (MODEL 1)
	    	   else
	    	     result = DIAG_SetupQRC2( dhn_aux, dhn_tim );	// setup default config for QRC (MODEL 2)

	    	   break;
	    	     
	      case '2':

	    	   LIB_LCD_ClearRow( 2, 2, FONT0 );
	    	   LIB_LCD_Puts( 2, 0, FONT0, strlen("TEST..."), "TEST..." );
	    	   
	      	   result = DIAG_Test2DReader( model, dhn_aux, dhn_tim );
	      	   
	    	   break;
	    	     
	      case 'x':

		   api_aux_close( dhn_aux );
		   api_tim_close( dhn_tim );
		   
	      	   return( TRUE );
	    	   }
	    	   
	if( result )
	  LIB_LCD_Puts( 7, 0, FONT0, strlen(msg_OK), msg_OK );
	else
	  LIB_LCD_Puts( 7, 0, FONT0, strlen(msg_ERROR), msg_ERROR );
	  
	LIB_WaitKey();
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: 50 - MFG Test.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_TestMfg( void )
{
UINT32	result;
UINT32	mode;
UINT32	status;


	mode = os_MultiBootMode;	// store mode
	os_MultiBootMode = OP_MODE_MFG;
		
	// test LCD
	LIB_LCD_Cls();
	DIAG_FUNC_TestDisplay();
	DIAG_FUNC_TestPinPad();
	
	LIB_LCD_Cls();
	DIAG_FUNC_TestKbdLED();
	
	LIB_LCD_Cls();
	result = DIAG_FUNC_TestFlash();
	POST_DisplayTestResult( result );
	
	LIB_LCD_Cls();
	result = DIAG_FUNC_TestSram();
	POST_DisplayTestResult( result );
	
	LIB_LCD_Cls();
	DIAG_FUNC_TestMsr();
	
	LIB_LCD_Cls();
	DIAG_FUNC_TestScr();
	
	LIB_LCD_Cls();
	result = DIAG_FUNC_TestRtc();
	POST_DisplayTestResult( result );
	
	LIB_LCD_Cls();
	result = DIAG_FUNC_TestAux( 0, &status );
	POST_DisplayTestResult( result );
	
	if( os_TSC_ENABLED )
	  {
	  LIB_LCD_Cls();
	  result = DIAG_FUNC_TestTouchScreen();
	  POST_DisplayTestResult( result );
	  }
	
	LIB_LCD_Cls();
	result = DIAG_FUNC_TestRC663();
	POST_DisplayTestResult( result );

	LIB_LCD_Cls();
	
	os_MultiBootMode = mode;	// restore mode
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Manufacturing test.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	POST_MfgTest( void )
{
UINT32	result;
UINT32	status;


	while(1)
	{
	// test LCD
	LIB_LCD_Cls();
	DIAG_FUNC_TestDisplay();
	DIAG_FUNC_TestPinPad();

	LIB_LCD_Cls();
	DIAG_FUNC_TestKbdLED();
	
	LIB_LCD_Cls();
	result = DIAG_FUNC_TestFlash();
	POST_DisplayTestResult( result );
	
	LIB_LCD_Cls();
	result = DIAG_FUNC_TestSram();
	POST_DisplayTestResult( result );
	
	LIB_LCD_Cls();
	DIAG_FUNC_TestMsr();
	
	LIB_LCD_Cls();
	DIAG_FUNC_TestScr();
	
	LIB_LCD_Cls();
	result = DIAG_FUNC_TestRtc();
	POST_DisplayTestResult( result );
	
	LIB_LCD_Cls();
	result = DIAG_FUNC_TestAux( 0, &status );
	POST_DisplayTestResult( result );
	
	if( os_TSC_ENABLED )
	  {
	  LIB_LCD_Cls();
	  result = DIAG_FUNC_TestTouchScreen();
	  POST_DisplayTestResult( result );
	  }
	
	LIB_LCD_Cls();
	result = DIAG_FUNC_TestRC663();
	POST_DisplayTestResult( result );
	
//	LIB_LCD_Cls();
	}
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Burn-in test.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
UINT32	POST_BurnInTest( void )
{
UINT32	result;

	LIB_LCD_Cls();
	
	if( os_MultiBootMode != OP_MODE_BIN )
	  {
	  if( LIB_EnterPassWord( 1, 1, "1379" ) != TRUE )
	    return(FALSE);
	  os_MultiBootMode = OP_MODE_BIN;
	  }

	while(1)
	{
	// test LCD
	DIAG_FUNC_TestDisplay();
	
	LIB_LCD_Cls();
	result = DIAG_FUNC_TestSram();
	POST_DisplayTestResult( result );
	
	LIB_LCD_Cls();
	}
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 60 - TFT LCD Color Test.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_TestLcdColor( void )
{
API_LCDTFT_GRAPH	graph;
API_LCDTFT_ICON		gicon;


	// modem
	graph.ID	= 0;
	graph.RGB	= 0;
	memmove( &graph.Bitmap, &bmcolor_balls_240x320_CCW, sizeof(GUI_BITMAP) );
	api_lcdtft_putgraphics( 0, graph );
	
	memset( (UCHAR *)&gicon, 0x00, sizeof(gicon) );
	gicon.ID	= 0;
	gicon.Xleft	= 0;
	gicon.Ytop	= 0;
	gicon.Method	= 0;
	gicon.Width	= 240;
	gicon.Height	= 320;
	api_lcdtft_showICON( 0, gicon );
	
	LIB_WaitKey();
	LIB_LCD_Cls();
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FUNC_TestLcdColor( void )
{
API_PCD_ICON icon[1];

	
	memset( (UCHAR *)icon, 0x00, sizeof(API_PCD_ICON) );
	
	icon->ID = IID_TAP_LOGO;
	api_lcdtft_showPCD( 0, (UCHAR *)icon );
	
	LIB_LCD_Puts( 0, 0, FONT2, strlen("�Ш�d�Ш�d"), "�Ш�d�Ш�d" );
	LIB_LCD_Puts( 1, 0, FONT2, strlen("�Ш�d�Ш�d"), "�Ш�d�Ш�d" );
	
	LIB_WaitKey();
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 61 - SIGN PAD Test.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - OK
//           FALSE - ERROR
// ---------------------------------------------------------------------------
#if	1
UINT32	DIAG_FUNC_TestSignPad( void )
{
//API_TSC_PARA tscpara[1];
//API_LCDTFT_RECT	rect[1];
API_TSC_PARA tscpara;
API_LCDTFT_RECT	rect;

ULONG	length = 0;
UCHAR	dhn_tsc;
UCHAR	dhn_lcd;
UCHAR	sbuf[64];
UCHAR	dbuf[64];
UCHAR	status;
UCHAR	SIGNPAD_direction = 0;


//	if( !os_TSC_ENABLED )
//	  return( FALSE );

	dhn_lcd = 0;
	dhn_tsc = api_tsc_open( 0 );
	
SIGNPAD:
	sbuf[0] = 255; // palette
	sbuf[1] = 208; //
	sbuf[2] = 139; //
	// clear sign pad message area
	memset( &rect, 0x00, sizeof(API_LCDTFT_RECT) );
	if( SIGNPAD_direction == 0 )
	  {
	  rect.Xstart = 40+120;
	  rect.Xend = 240;
	  }
	else
	  {
	  rect.Xstart = 0;
	  rect.Xend = 80;
	  }
	
	rect.Ystart = 0;
	rect.Yend = 320;
	rect.Palette[0] = sbuf[0];
	
	rect.Palette[1] = sbuf[1];
	rect.Palette[2] = sbuf[2];
	api_lcdtft_fillRECT( dhn_lcd, rect );
	
	status = 0; // init signpad status
	
SIGNPAD2:
	memset( &tscpara, 0x00, sizeof( API_TSC_PARA ) );
	
	tscpara.ID = FID_TSC_SIGNPAD;
	tscpara.X = 0;
	tscpara.Y = 40;
	tscpara.Width = 320;
	tscpara.Height = 120;
	tscpara.RxLen = 1;
	tscpara.RFU[0] = status; // current status
	tscpara.RFU[1] = SIGNPAD_direction; // direction (0 or 1)
	tscpara.Timeout = 100*60; // 60 sec (unit: 10ms)
	status = 0;
	api_tsc_signpad( dhn_tsc, tscpara, &status, sbuf );
//	LIB_DispHexByte( 0, 0, status );
//	LIB_WaitKey();
	
	if( status == SIGN_STATUS_TIMEOUT )
	goto EXIT;
	if( status == SIGN_STATUS_PROCESSING )
	goto SIGNPAD2;
	if(status == SIGN_STATUS_CLEAR)
	goto SIGNPAD2;
	if( status == SIGN_STATUS_ROTATE )
	  {
	  if( SIGNPAD_direction == 0 )
	    SIGNPAD_direction = 1;
	  else
	    SIGNPAD_direction = 0;
	  goto SIGNPAD; // restart sign pad
	  }

EXIT:
//	LIB_DispHexByte( 0, 0, status );
//	LIB_WaitKey();

	api_tsc_close( dhn_tsc);
	
	LIB_LCD_Cls();
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Data retention test.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
UINT32	POST_DataRetentionTest2( void )
{
UINT32	result;

	
	if( LIB_EnterPassWord( 1, 1, "1379" ) != TRUE )
	  return(FALSE);
	
	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_SRAM_RETENTION), (UINT8 *)os_msg_SRAM_RETENTION );
	LIB_LCD_Puts( 1, 0, FONT0, sizeof(os_msg_1WRITE_2READ), (UINT8 *)os_msg_1WRITE_2READ );
	
	while(1)
	     {
	     switch( LIB_WaitKey() )
	           {
	           case '1':	// write
			
			DIAG_SramTestWrite();
	      
	           case '2':	// read
	           
	                result = DIAG_SramTestRead();
	                POST_DisplayTestResult( result );
	                for(;;);
	           }
	     }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: 93 - System maintenance functions.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
// ---------------------------------------------------------------------------
UINT32	SYS_FUNC_SysMaintenance( void )
{
UINT8	key;
UINT8	const	msg_1_KILL_IPC_SERVER[] = {"1-KILL IPC SERVER"};
UINT8	const	msg_2_DISABLE_TTYMXC2[] = {"2-DISABLE TTYMXC2"};
UINT8	const	msg_9_REMOVE_SYSTEM[] =	  {"9-REMOVE SYSTEM"};


	if( LIB_EnterPassWord( MAX_PED_PSW_CNT, 1, "37273" ) != TRUE )
	  return( FALSE );
	
	LIB_LCD_Puts( 1, 0, FONT0, sizeof(msg_1_KILL_IPC_SERVER), (UINT8 *)msg_1_KILL_IPC_SERVER );
	LIB_LCD_Puts( 2, 0, FONT0, sizeof(msg_2_DISABLE_TTYMXC2), (UINT8 *)msg_2_DISABLE_TTYMXC2 );
	LIB_LCD_Puts( 3, 0, FONT0, sizeof(msg_9_REMOVE_SYSTEM), (UINT8 *)msg_9_REMOVE_SYSTEM );
	
	do{
	  key = LIB_WaitKey();
	  } while( (key != '1') && (key != '2') && (key != '9') && (key != 'x') );
	  
	LIB_LCD_ClearRow( 1, 3, FONT0 );
	
	switch( key )
	      {
	      case '1':		// kill IPC_server
	      	   
	      	   system( "killall /home/root/IPC_server" );
	      	   return( TRUE );
	      
	      case '2':		// remove ttymxc2 service, which which is reserved for QR code reader.
	      	   
	      	   system( "systemctl disable serial-getty@ttymxc2.service" );
	      	   return( TRUE );
	      	   
	      case '9':		// kill whole SYSTEM

		   system( "flash_erase /dev/mtd0 0 0" );	// erase U-boot
		   system( "flash_erase /dev/mtd1 0 0" );	// erase Kernel
	
		   LIB_LCD_Puts( 2, 0, FONT0, sizeof("SYSTEM HAS BEEN REMOVED!!!")-1, (UINT8 *)"SYSTEM HAS BEEN REMOVED!!!" );
		   
	      	   for(;;);	// shall reload system by UUU
	      	   
	      case 'x':		// abort
	      	   return( TRUE );
	      	   
	      default:
	      	   return( FALSE );
	      }
}

// ---------------------------------------------------------------------------
// FUNCTION: Data retention test.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	POST_DataRetentionTest( void )
{
UINT32	result;

	
	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_SRAM_RETENTION), (UINT8 *)os_msg_SRAM_RETENTION );
	LIB_LCD_Puts( 1, 0, FONT0, sizeof(os_msg_1WRITE_2READ), (UINT8 *)os_msg_1WRITE_2READ );
	
	while(1)
	     {
	     switch( LIB_WaitKey() )
	           {
	           case '1':	// write
			
			DIAG_SramTestWrite();
	      
	           case '2':	// read
	           
	                result = DIAG_SramTestRead();
	                POST_DisplayTestResult( result );
	                for(;;);
	           }
	     }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Initialize default printer FONT0.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	POST_InitPrinterFont( void )
{
//API_PRT_FONT	ft;
API_PRT_FONT	ft[1];
//API_LCD_FONT	lcdft;
API_LCD_FONT	lcdft[1];


	memset( (UCHAR *)ft, 0, sizeof(API_PRT_FONT) );

	// setup ASCII 12X24 font
	ft->FontID = FONT0;
	ft->Height = 24;
	
	ft->AN_Type    = FT_ASCII;
	ft->AN_CodeNo  = 1;
	ft->AN_ByteNo  = 2*24;
	ft->AN_CharNo  = 96;
	ft->AN_Width   = 12;
	ft->AN_BitMap  = (UCHAR *)FONT2_ASCII_12x24;
	ft->AN_CodeMap = (UCHAR *)0;
		
	api_prt_initfont( (UCHAR *)ft );

	// setup built-in full set 24x24 Chinese FONT2 (pre-burned in FLASH)	
	memset((UCHAR *)ft,0,sizeof(API_PRT_FONT));

	// setup FONT2: ASCII 12X24 & BIG5 24x24
	ft->FontID = FONT2;
	ft->Height = 24;
	
	ft->AN_Type    = FT_ASCII;
	ft->AN_CodeNo  = 1;
	ft->AN_ByteNo  = 2*24;
	ft->AN_CharNo  = 96;
	ft->AN_Width   = 12;
	ft->AN_BitMap  = (UCHAR *)FONT2_ASCII_12x24;	// system default
	ft->AN_CodeMap = (UCHAR *)0;

	ft->GC_Type = FT_BIG5;
	ft->GC_CodeNo = 2;
	ft->GC_ByteNo = 3*24;
	ft->GC_CharNo = FULL_BIG5_CHAR_NUM;
	ft->GC_Width = 24;
	ft->GC_BitMap = (UCHAR *)ADDR_FULL_PRT_FONT2_24X24_BMP;	// user-defined
	ft->GC_CodeMap = (UCHAR *)ADDR_FULL_BIG5_CODE_TABLE;
		
	api_prt_initfont( (UCHAR *)ft );

#if	1
	// setup FONT4: ASCII 8X16 & BIG5 16x16
	memset((UCHAR *)ft,0,sizeof(API_PRT_FONT));
	
	ft->FontID = FONT4;
	ft->Height = 16;
	
	ft->AN_Type    = FT_ASCII;
	ft->AN_CodeNo  = 1;
	ft->AN_ByteNo  = 1*16;
	ft->AN_CharNo  = 96;
	ft->AN_Width   = 8;
	ft->AN_BitMap  = (UCHAR *)FONT4_ASCII_8x16;	// system default
	ft->AN_CodeMap = (UCHAR *)0;

	ft->GC_Type = FT_BIG5;
	ft->GC_CodeNo = 2;
	ft->GC_ByteNo = 2*16;
	ft->GC_CharNo = FULL_BIG5_CHAR_NUM;
	ft->GC_Width = 16;
	ft->GC_BitMap = (UCHAR *)ADDR_FULL_PRT_FONT4_16X16_BMP;	// user-defined
	ft->GC_CodeMap = (UCHAR *)ADDR_FULL_BIG5_CODE_TABLE;
		
	api_prt_initfont( (UCHAR *)ft );
#endif

	// setup default TFT LCD Chinese font (FONT12, 24X24)
	memset( (UCHAR *)lcdft, 0, sizeof(API_LCD_FONT) );

        lcdft->FontID = FONT12; // set to FONT2
        lcdft->ByteNo = 72;
        lcdft->Width  = 24;
        lcdft->Height = 24;
        lcdft->codPageNo  = 0x00;  // RFU
        lcdft->codStAddr  = (UCHAR *)ADDR_FULL_BIG5_CODE_TABLE;
        lcdft->codEndAddr = (UCHAR *)ADDR_FULL_BIG5_CODE_TABLE + 2*FULL_BIG5_CHAR_NUM - 1;
        lcdft->bmpPageNo  = 0x00;  // RFU
        lcdft->bmpStAddr  = (UCHAR *)ADDR_FULL_PRT_FONT2_24X24_BMP;
        lcdft->bmpEndAddr = (UCHAR *)ADDR_FULL_PRT_FONT2_24X24_BMP + 72*FULL_BIG5_CHAR_NUM - 1;
        
	api_lcd_initfont( (UCHAR *)lcdft );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Power on self-test for ICC & SAMs.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	POST_SmartCardModule( void )
{
UINT32	i = 0;
UINT32	status = FALSE;
UINT8	dhn_icc1;
UINT8	dhn_sam1;
UINT8	dhn_sam2;
UINT8	dhn_sam3;
UINT8	dhn_sam4;


	do{
	  BSP_Delay_n_ms( 1000 );	// wait 1 sec
	
	  // Open all devices
	  dhn_icc1 = api_ifm_open( ICC1 );
	  if( g_ScSpiErr1 || g_ScSpiErr2 )
	    {
	    api_ifm_close( dhn_icc1 );
	    status = FALSE;
	    return( status );
	    }
	    
	  dhn_sam1 = api_ifm_open( SAM1 );
	  dhn_sam2 = api_ifm_open( SAM2 );
	  dhn_sam3 = api_ifm_open( SAM3 );
	  dhn_sam4 = api_ifm_open( SAM4 );
	
	  BSP_Delay_n_ms( 100 );	// wait 100 ms
	  
	  // verify SC H/W ready or not by checking Sc_nAlram pin,
	  // if the pin is in low state, it means the SC is not ready,
	  // it may occur when power supply is swiftly turned off and on.
	  if( (BSP_GPIO_READ( pScGpio ) & 0x00800000) != 0 )	// Sc_nAlarm (GPIO_1[23]) = HI? (active)
	    {
	    status = TRUE;
	    break;
	    }
	  } while( ++i < 3 );
	
	// close all devices      
	api_ifm_close( dhn_icc1 );
	api_ifm_close( dhn_sam1 );
	api_ifm_close( dhn_sam2 );
	api_ifm_close( dhn_sam3 );
	api_ifm_close( dhn_sam4 );

	return( status );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Power on self-test for MSR.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	POST_MagneticStripeReaderModule( void )
{
UINT32	i = 0;
UINT32	status = FALSE;
UINT32	data;
UINT8	dhn_msr;

	
	do{
	  dhn_msr = api_msr_open( isoTrack1 + isoTrack2 + isoTrack3 );
	  
	  if( dhn_msr == apiOutOfService )
	    {
	    BSP_Delay_n_ms( 500 );
	    continue;
	    }
	  
	  data = BSP_RD32( MCR_CTRL_REG );	// make sure MODE & DMA can be enabled OK
	  if( data == 0xC7 )
	    {
	    api_msr_close( dhn_msr );
	    
	    status = TRUE;
	    break;
	    }
	    
	  api_msr_close( dhn_msr );
	  
	  BSP_Delay_n_ms( 500 );
	  } while( ++i < 3 );
	  
	return( status );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Check FLASH type by using CFI.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FlashCheckType( void )
{
	if( IdentifyFlash16( 0 ) == FLASH_ERROR )
	  {
	  while(1)
	       {
	       }
	  }
	  
	if( FlashSizeMB == 32 )	// 2013-11-20, temp support max 16MB
	  {
	  FlashSizeMB = 16;
	  FlashMaxAddr = FLASH_BASE + 16*1024*1024 - 1;
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Check FLASH type by using CFI.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FlashCheckType2( void )
{
	if( IdentifyFlash16( 2 ) == FLASH_ERROR )	// CS2
	  {
	  while(1)
	       {
	       }
	  }
	  
//	if( FlashSizeMB == 32 )	// 2013-11-20, temp support max 16MB
//	  {
//	  FlashSizeMB = 16;
//	  FlashMaxAddr = FLASH_BASE + 16*1024*1024 - 1;
//	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Erase FLASH sectors. (only those accessible to applications)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// NOTE    : IdentifyFlash16() shall be called prior to executing this function.
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FlashErasePages( UINT32 StartAddr, UINT32 NoEraseSize )
{
UINT32	i;
UINT32	SectorAddr;
UINT32	SectorStartAddr;
UINT32	SectorLastAddr;
UINT32	Sectors;
UINT32	ByteCount;
UINT32	Size;

	
	// Erase sectors (addr=0x1010_0000..0x10FF_FFFF), 1 sector = 64 or 128 KB
//	SectorStartAddr = 0x10020000;	// start address = 0x1010_0000 or 0x1002_0000
	SectorStartAddr = StartAddr;
	SectorAddr = SectorStartAddr;

//	if( FlashMaxAddr > 0x10400000 )	// check 4MB boundary
//	  FlashMaxAddr = 0x10400000;

//	SectorLastAddr = FlashMaxAddr - 256*1024;			// sizeof(POST0) = last 256KB of Flash
//	SectorLastAddr = FlashMaxAddr - 512*1024;			// sizeof(POST0) = last 512KB of Flash (4MB)
//	SectorLastAddr = FlashMaxAddr - 768*1024;			// sizeof(POST0) = last 768KB of Flash (4MB)
//	SectorLastAddr = FlashMaxAddr - (768+1472)*1024;		// sizeof(POST0+CFONT) = last (768+1472)KB of Flash (4MB)

	FlashMaxAddr = FLASH_BASE+(FlashSizeMB*0x100000)-1;		// 2011-06-13, fixed MAX 8MB
	SectorLastAddr = FlashMaxAddr - NoEraseSize;
	
	Sectors = (SectorLastAddr - SectorAddr + 1) / FlashBlockSize;	// num of sectors to be erased
	
	ByteCount = 0;

	LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_E), (UINT8 *)os_msg_E );
	for( i=0; i<Sectors; i++ )
	   {
//	   LIB_DispHexLong( 2, sizeof(os_msg_ERASE_FLASH)+1, SectorAddr );			// physical
	   LIB_DispHexLong( 2, sizeof(os_msg_ERASE_FLASH)+1, SectorAddr - SectorStartAddr );	// logical
//	   LIB_WaitKey();
	      
	   if( FLASH_BlankCheckSector( (void *)SectorAddr, FlashBlockSize ) == FALSE )
	     {
	     Size = EraseSector( SectorAddr );
	     if( Size == 0 )
	       {
	       break;
	       }
	     }
	   else
	     Size = FlashBlockSize;
	     
	   SectorAddr += Size;
	   ByteCount += Size;
	   }
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Test FLASH memory. (destructive)
//	     Writing cyclic patterns 0x0N..0xFF to each sector. (N=sector#)
//	     R/W test 256 bytes per sector.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// NOTE    : IdentifyFlash16() shall be called prior to executing this function.
// ---------------------------------------------------------------------------
#if	0
UINT32	DIAG_FlashTestRW( UINT32 StartAddr, UINT32 NoEraseSize )
{
UINT32	i, j, k;
UINT32	SectorAddr;
UINT32	SectorStartAddr;
UINT32	SectorLastAddr;
UINT32	Sectors;
UINT32	Cnt;
UINT32	Size;
UINT32	Len;
volatile UINT16	* pAddr;
UINT16	Data;
UINT16	Buf[128];


	// --- Write ---
	LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_W), (UINT8 *)os_msg_W );
	
	Len = sizeof(Buf);	// in bytes
	
//	SectorStartAddr = 0x10020000;	// start address = 0x1010_0000 or 0x1002_0000
	SectorStartAddr = StartAddr;
	SectorAddr = SectorStartAddr; 

//	if( FlashMaxAddr > 0x10400000 )	// check 4MB boundary
//	  FlashMaxAddr = 0x10400000;
	
//	SectorLastAddr = FlashMaxAddr - 256*1024;			// sizeof(POST0) = last 256KB of Flash
//	SectorLastAddr = FlashMaxAddr - 512*1024;			// sizeof(POST0) = last 512KB of Flash (4MB)
//	SectorLastAddr = FlashMaxAddr - 768*1024;			// sizeof(POST0) = last 768KB of Flash (4MB)
//	SectorLastAddr = FlashMaxAddr - (768+1472)*1024;		// sizeof(POST0+CFONT) = last (768+1472)KB of Flash (4MB)
	
	FlashMaxAddr = FLASH_BASE+(FlashSizeMB*0x100000)-1;		// 2011-06-13, fixed MAX 8MB
	SectorLastAddr = FlashMaxAddr - NoEraseSize;

	Sectors = (SectorLastAddr - SectorAddr + 1) / FlashBlockSize;	// num of sectors to be processed
	
	for( i=0; i<Sectors; i++ )	// 1 sector = Len * Cnt (words)
	   {
//	   LIB_DispHexLong( 2, sizeof(os_msg_ERASE_FLASH)+1, SectorAddr );			// physical
	   LIB_DispHexLong( 2, sizeof(os_msg_ERASE_FLASH)+1, SectorAddr - SectorStartAddr );	// logical
//	   LIB_WaitKey();
	   
	   Data = i;	// beginning data pattern per sector
//	   Cnt = FlashBlockSize / Len;	// physical counter
//	   Cnt = 16;			// smaller counter for saving time
	   
//	   for( j=0; j<Cnt; j++ )
//	      {
	      for( k=0; k<(Len/2); k++ )
	         Buf[k] = Data++;	// in words
	      
	      if( FlashWrite_Ex( (void *)SectorAddr, (void *)Buf, Len ) != FLASH_SUCCESS )	// writing per LEN bytes
	        {
	        return( FALSE );
		}
	      
//	      SectorAddr += Len;
//	      } // for(j)
	   SectorAddr += FlashBlockSize;
	   } // for(i)

	
	// --- Read ---
	LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_R), (UINT8 *)os_msg_R );
	
//	SectorAddr = SectorStartAddr;
//	pAddr = (UINT16 *)SectorAddr;

	for( i=0; i<Sectors; i++ )	// 1 sector = Len * Cnt (words)
	   {
	   pAddr = (UINT16 *)(SectorStartAddr + i*FlashBlockSize);
	   
//	   LIB_DispHexLong( 2, sizeof(os_msg_ERASE_FLASH)+1, (ULONG)pAddr );			// physical
	   LIB_DispHexLong( 2, sizeof(os_msg_ERASE_FLASH)+1, (ULONG)pAddr - SectorStartAddr );	// logical
//	   LIB_WaitKey();
	   
	   Data = i;	// beginning data pattern per sector
	   
//	   for( j=0; j<Cnt; j++ )
//	      {
	      for( k=0; k<(Len/2); k++ )
	         {
	         if( *pAddr++ != Data++ )
	           {
	           return( FALSE );
	           }
	         }
	        
//	      } // for(j)
	   } // for(i)
	
	return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Show status of ATR.
//	     ICC0:   OK (or ERROR)
//           SAMx:   OK (or ERROR)
// INPUT   : device - device no. (row no.)
//	     status - ATR status.
//		      -1 = to clear all statuses.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	1
void	DIAG_ShowStatusATR( UINT8 device, UINT8 status )
{
UINT8	i;
UINT8	row;
UINT8	col;

	row = device;
	col = 10;
	
	if( status == (UINT8)-1 )
	  {
	  for( i=0; i<5; i++ )
	     LIB_LCD_Puts( i, col, FONT0, sizeof(os_msg_5SP), (UINT8 *)os_msg_5SP );
	  return;
	  }
	
	if( status == apiOK )
	  LIB_LCD_Puts( row, col, FONT0, sizeof(os_msg_OK), (UINT8 *)os_msg_OK );
	else
	  LIB_LCD_Puts( row, col, FONT0, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR );

}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Show status of MSR.
//	     TRACKn:   OK (or ERROR)
// INPUT   : track  - track no. (1..3)
//	     status - MSR status.
//                    -1 = to clear all statuses.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	1
void	DIAG_ShowStatusMSR( UINT8 track, UINT8 status )
{
UINT8	i;
UINT8	row;
UINT8	col;

	row = track;
	col = 10;
	
	if( status == (UINT8)-1 )
	  {
	  for( i=0; i<5; i++ )
	     LIB_LCD_Puts( i, col, FONT0, sizeof(os_msg_5SP), (UINT8 *)os_msg_5SP );
	  return;
	  }
	
	switch( status )
	      {
	      case msrNoData:		// no data
	      
	      	   LIB_LCD_Puts( row, col, FONT0, sizeof(os_msg_5SP), (UINT8 *)os_msg_5SP );
	      	   break;

	      case msrDataOK:		// data ok
	      
	      	   LIB_LCD_Puts( row, col, FONT0, sizeof(os_msg_OK5), (UINT8 *)os_msg_OK5 );
	      	   break;

	      case msrDataError:	// data error
	      
	      	   LIB_LCD_Puts( row, col, FONT0, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR );
	      	   break;
	      }

}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Show contents of ATR.
//	     ICC0: (or SAMx:)
//           xx xx xx xx xx xx xx
//	     xx xx xx
// INPUT   : device - device no.
//	     atr    - L-V, atr contents.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	1
void	DIAG_ShowDataATR( UINT8 device, UINT8 *atr )
{
UINT8	len;

	len = *atr++;

	switch( device )
	      {
	      case ICC1:
	      
	      	   LIB_LCD_Puts( 0, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_ICC0), (UINT8 *)os_msg_ICC0 );
	      	   break;
	      	   
	      case SAM1:
	      
	      	   LIB_LCD_Puts( 0, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_SAM1), (UINT8 *)os_msg_SAM1 );
	      	   break;
	      	   
	      case SAM2:
	      
	      	   LIB_LCD_Puts( 0, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_SAM2), (UINT8 *)os_msg_SAM2 );
	      	   break;
	      	   
	      case SAM3:
	      
	      	   LIB_LCD_Puts( 0, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_SAM3), (UINT8 *)os_msg_SAM3 );
	      	   break;
	      	   
	      case SAM4:
	      
	      	   LIB_LCD_Puts( 0, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_SAM4), (UINT8 *)os_msg_SAM4 );
	      	   break;
	      }
	      
	LIB_DumpHexData( 0, 1, len, atr );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Show contents of MSR Tracks.
//	     TRACKn
//           xx xx xx xx xx xx xx
//	     xx xx xx
//
//	     Track data format.
//	     L-Track1
//	     L-Track2
//	     L-Track3
//
// INPUT   : dhn 
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	1
void	DIAG_ShowDataTracks( UINT8 dhn, UINT8 *Status, UINT8 *Data )
{
UINT8	sbuf[2];
UINT16	len;

	
	if( Status[0] == msrSwiped )
	  {
	  // Track 1
	  if( Status[1] == msrDataOK )
	    {
	    LIB_LCD_Puts( 0, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_TRACK1), (UINT8 *)os_msg_TRACK1 );
	    
	    len = *Data++;
	    LIB_DumpHexData( 1, 1, len, Data );
	    Data += len;
	    }
	  
	  // Track 2
	  if( Status[2] == msrDataOK )
	    {
	    LIB_LCD_Puts( 0, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_TRACK2), (UINT8 *)os_msg_TRACK2 );

	    len = *Data++;
	    LIB_DumpHexData( 1, 1, len, Data );
	    Data += len;
	    }
	    
	  // Track 3
	  if( Status[3] == msrDataOK )
	    {
	    LIB_LCD_Puts( 0, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_TRACK3), (UINT8 *)os_msg_TRACK3 );
	    
	    len = *Data++;
	    LIB_DumpHexData( 1, 1, len, Data );
	    }
	  
//	  sbuf[0] = 0; // clear data after reading
//	  sbuf[1] = isoTrack1 + isoTrack2 + isoTrack3;
//	  api_msr_getstring( dhn, sbuf, dbuf );
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Stamp BIOS release date and time to the backup flash area.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	POST_StampReleaseDate( void )
{
UINT8	buf[BIOS_RELEASE_DATE_LEN];


	OS_FLS_GetData( F_ADDR_BIOS_RELEASE_DATE, BIOS_RELEASE_DATE_LEN, buf );	// read stamped release date
	
	if( LIB_memcmp( buf, (UINT8 *)BIOS_RELEASE_DATE, BIOS_RELEASE_DATE_LEN ) != 0 )	// write new date stamp
	  return( OS_FLS_PutData( F_ADDR_BIOS_RELEASE_DATE, BIOS_RELEASE_DATE_LEN, (UINT8 *)BIOS_RELEASE_DATE ) );
	else
	  return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Load DSS variables from backup flash area.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	POST_LoadDssVariables( void )
{
	OS_FLS_GetData( F_ADDR_DssTelNum, DssTelNum_LEN, DssTelNum );
	OS_FLS_GetData( F_ADDR_DssRemoteIP, DssRemoteIP_LEN, DssRemoteIP );
	OS_FLS_GetData( F_ADDR_DssRemotePort, DssRemotePort_LEN, DssRemotePort );
	OS_FLS_GetData( F_ADDR_DssPort, DssPort_LEN, DssPort );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Compare current date and time with bios stamped release date.
// INPUT   : date = YYMMDDhhmmss (fixed 12 bytes ASCII).
// OUTPUT  : none.
// RETURN  : TRUE  -- logically correct.  (current date >= release date)
//	     FALSE -- logically incorrect.(current date <  release date)
// ---------------------------------------------------------------------------
#if	0
#ifndef	_BOOT_FROM_POST0_

UINT32	SYS_FUNC_VerifyRTC( UINT8 *date )
{
UINT8	buf[BIOS_RELEASE_DATE_LEN];
UINT32	tseconds1;
UINT32	tseconds2;


	OS_FLS_GetData( F_ADDR_BIOS_RELEASE_DATE, BIOS_RELEASE_DATE_LEN, buf );	// read stamped release date, YYYYMMDDhhmmss
	
	tseconds1 = LIB_UnixTime( &buf[2] );
	tseconds2 = LIB_UnixTime( date );
	
	if( tseconds2 <= tseconds1 )
	  return( FALSE );
}
#endif
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Init flash data area for the 1'st time use. (clear F_ADDR_CL_DATA to 0s)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	POST_InitFlashData( void )
{
UINT8	buf[MAX_CL_DATA_SIZE];


	OS_FLS_GetData( F_ADDR_CL_DATA_INIT_STATE, MAX_CL_DATA_INIT_STATE_SIZE, buf );	// read state
	if( (buf[0] == 0xFF) && (buf[1] == 0xFF) )
	  {
	  // clear DATA to 0s
	  memset( buf, 0x00, sizeof(buf) );
	  OS_FLS_PutData( F_ADDR_CL_DATA, MAX_CL_DATA_SIZE, buf );
	  
	  // reset state
	  buf[0] = 0x12;
	  buf[1] = 0x23;
	  return( OS_FLS_PutData( F_ADDR_CL_DATA_INIT_STATE, MAX_CL_DATA_INIT_STATE_SIZE, buf ) );
	  }
	else
	  return( TRUE );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Verify and/or restore TMK/TSN/F_ADDR_FISC_TMK_STATE.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	POST_EnforceTMK( void )
{
UINT32	i;
UINT32	flag;
UINT16	len;
UINT8	kcv[8];
UINT8	kek[16];
UINT8	buf[PED_MSKEY_SLOT_LEN2];
UINT8	temp[PED_MSKEY_SLOT_LEN2];
UINT8	mkey[PED_MSKEY_SLOT_LEN2];


	// verify or restore TSN
	OS_FLS_GetData( F_ADDR_TSN, TSN_LEN, buf );	// LEN(1)+TSN(15)
	if( buf[0] != 8 )
	  {
	  OS_SECM_GetData( S_ADDR_TSN, TSN_LEN, buf );
	  if( buf[0] == 8 )
	    {
	    flag = TRUE;
	    for( i=1; i<8; i++ )
	       {
	       if( (buf[i] < '0') || (buf[i] > '9') )
	       	 {
	       	 flag = FALSE;
	       	 break;
	       	 }
	       }
	    
	    if( flag )
	      OS_FLS_PutData( F_ADDR_TSN, 9, buf );	// restore TSN in FLASH
	    }
	  }
	else
	  {
	  OS_SECM_GetData( S_ADDR_TSN, TSN_LEN, temp );
	  if( temp[0] != 8 )
	    OS_SECM_PutData( S_ADDR_TSN, 9, buf );	// restroe TSN in SECM
	  }

	api_sys_info( SID_FISCversion, buf );	// FISC?
	if( buf[1] != '(' )	// "(NA)"
	  goto FISC;
	  
	// verify or restore TMK
	for( i=0; i<MAX_MKEY_CNT; i++ )
	   {
//	   DIAG_FLASH_ReadETMK( i, buf );	// L(1)+V(16)+KCV(3)
	   OS_FLS_GetData( F_ADDR_PED_ETMK_01+(i*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, buf );	// L(1)+V(16)+KCV(3)
	   if( buf[0] != PED_MSKEY_LEN ) 	// ETMK available? (in FLASH)
	     {
	     OS_SECM_GetData( ADDR_PED_MKEY_01+(i*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, mkey );
//	     LIB_DumpHexData( 0, 0, PED_MSKEY_SLOT_LEN2, mkey );
	     if( mkey[0] == PED_MSKEY_LEN )
	       {
	       // verify KCV for the old TMK
	       memset( temp, 0x00, sizeof(temp) );
	       PED_TripleDES( &mkey[1], 8, temp, kcv );
	       if( (kcv[0] == mkey[17]) && (kcv[1] == mkey[18]) && (kcv[2] == mkey[19]) )
	       	 {
	       	 // save encrypted TMK to Flash key slot, eTMK = 3DES( KEK, TMK )
	       	 DIAG_InitDefaultKEK( kek );
	       	 PED_TripleDES( kek, 16, &mkey[1], &temp[1] );
	       	 temp[0] = PED_MSKEY_LEN;
	       	 
	       	 memmove( &temp[17], kcv, 3 );	// kcv
//	       	 LIB_DumpHexData( 0, 0, PEDS_MSKEY_SLOT_LEN2, temp );

		 OS_FLS_PutData( (F_ADDR_PED_ETMK_01+(i*PED_MSKEY_SLOT_LEN2)), PED_MSKEY_SLOT_LEN2, temp );	// restore ETMK in FLASH
	       	 }
	       }
	     }
	   else
	     {
	     OS_SECM_GetData( ADDR_PED_MKEY_01+(i*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, temp );
	     if( temp[0] != PED_MSKEY_LEN )
	       {
	       // save decrypted eTMK to SECM key slot, TMK = ~3DES( KEK, eTMK )
	       DIAG_InitDefaultKEK( kek );
	       PED_TripleDES2( kek, 16, &buf[1], &temp[1] );
	       temp[0] = PED_MSKEY_LEN;
	       
	       memmove( &temp[17], &buf[17], 3 );	// kcv
//	       LIB_DumpHexData( 0, 0, PEDS_MSKEY_SLOT_LEN2, temp );
	       
	       OS_SECM_PutData( ADDR_PED_MKEY_01+(i*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, temp );	// restroe TMK in SECM
	       }
	     }
	   }
	goto FISC_END;
	
FISC:
	// verify or restore single TMK (FISC)
	i = 0;
	
	   OS_FLS_GetData( F_ADDR_PED_ETMK+(i*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, buf );	// L(1)+V(16)+KCV(3)
	   if( buf[0] != PED_MSKEY_LEN ) 	// ETMK available? (in FLASH)
	     {
	     OS_SECM_GetData( ADDR_PED_MKEY_01+(i*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, mkey );
	     if( mkey[0] == PED_MSKEY_LEN )
	       {
	       // verify KCV for the old TMK
	       memset( temp, 0x00, sizeof(temp) );
	       PED_TripleDES( &mkey[1], 8, temp, kcv );
	       if( (kcv[0] == mkey[17]) && (kcv[1] == mkey[18]) && (kcv[2] == mkey[19]) )
	       	 {
	       	 // save encrypted TMK to Flash key slot, eTMK = 3DES( KEK, TMK )
	       	 DIAG_InitDefaultKEK( kek );
	       	 PED_TripleDES( kek, 16, &mkey[1], &temp[1] );
	       	 temp[0] = PED_MSKEY_LEN;
	       	 memmove( &temp[17], kcv, 3 );	// kcv

		 OS_FLS_PutData( (F_ADDR_PED_ETMK+(i*PED_MSKEY_SLOT_LEN2)), PED_MSKEY_SLOT_LEN2, temp );	// restore ETMK in FLASH
	       	 }
	       }
	     }
	   else
	     {
	     OS_SECM_GetData( ADDR_PED_MKEY_01+(i*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, temp );
	     if( temp[0] != PED_MSKEY_LEN )
	       {
	       // save decrypted eTMK to SECM key slot, TMK = ~3DES( KEK, eTMK )
	       DIAG_InitDefaultKEK( kek );
	       PED_TripleDES2( kek, 16, &buf[1], &temp[1] );
	       temp[0] = PED_MSKEY_LEN;
	       memmove( &temp[17], &buf[17], 3 );	// kcv
	       
	       OS_SECM_PutData( ADDR_PED_MKEY_01+(i*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, temp );	// restroe TMK in SECM
	       }
	     }

FISC_END:     
	// verify or restore FISC_TMK_STATE
	OS_FLS_GetData( F_ADDR_FISC_TMK_STATE, FISC_TMK_STATE_LEN, buf );	// FISC_TMK_STATE(16)
	if( LIB_memcmpc( buf, 0xFF, FISC_TMK_STATE_LEN ) == 0 )
	  {
	  OS_SECM_GetData( S_ADDR_FISC_TMK_STATE, S_FISC_TMK_STATE_LEN, buf );
	  if( (LIB_memcmpc( buf, 0xFF, FISC_TMK_STATE_LEN ) != 0) && (LIB_memcmpc( buf, 0x00, FISC_TMK_STATE_LEN ) != 0) )
	    OS_FLS_PutData( F_ADDR_FISC_TMK_STATE, FISC_TMK_STATE_LEN, buf );	// restore FISC_TMK_STATE in FLASH
	  }
	else
	  {
	  OS_SECM_GetData( S_ADDR_FISC_TMK_STATE, S_FISC_TMK_STATE_LEN, temp );
	  if( LIB_memcmp( temp, buf, S_FISC_TMK_STATE_LEN ) != 0 )
	    OS_SECM_PutData( S_ADDR_FISC_TMK_STATE, S_FISC_TMK_STATE_LEN, buf );	// restroe FISC_TMK_STATE in SECM
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Check TSC is enabled or not.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE / FALSE
// ---------------------------------------------------------------------------
#if	0
UINT32	IS_TSC_ENABLED( void )
{
	return( os_TSC_ENABLED );
}
#endif

// ---------------------------------------------------------------------------
void	POST_FLASH_PageInit( void )
{
API_FLASH pFls;


#if	1
//	OS_FLASH_PageInit(0);	// init NOR Flash (default flash type)
//	api_flash_init();	// already init in IPC_server
	
	api_flash_TypeSelect( TYPE_NOR );
	
	pFls.StPage = 0;
	pFls.EndPage = SYST_FLASH_START_PAGE-1;
	api_flash_PageLink( pFls, 0 );		// link all pages to one except the last page
#endif

#if	0
//	OS_FLASH_PageInit(1);	// init NAND Flash
	OS_FLASH_PageInit_NAND0();	// init and read ID only
	
	api_flash_TypeSelect( TYPE_NAND );	// 2018-07-23
	
	pFls.StPage = 0;
	pFls.EndPage = SYST_FLASH_START_PAGE-1;
	api_flash_PageLink( pFls, 0 );		// link the first 16MB to one page, the others (128-16 MB, page# 128~1023) are RFU.
	
	api_flash_TypeSelect( TYPE_NOR );	// 2018-08-06
#endif
}
#endif
