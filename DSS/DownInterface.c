//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : DSS (for no file system)                                   **
//**  PRODUCT  : AS320B                                                     **
//**                                                                        **
//**  FILE     : DownInterface.C					    **
//**  MODULE   : DSS                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/06/15                                                 **
//**  EDITOR   : Richad Hu                                                  **
//**                                                                        **
//**  Copyright(C) 2009 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================

// #include "za9_pmu.h"
#include "bsp_tmr.h"
#include "bsp_types.h"
#include "POSAPI.h"
#include "DownPacket.h"
#include "OS_LIB.h"
#include "OS_FLASH.h"

#ifdef	_LAN_ENABLED_
// #include "EEPROM.h"
#endif

#include "esf.h"
#include "redirtsk.h"
#include "stdio.h"
// #include "GlobalVar.h"
#include "bsp_uart.h"

#define		DSS_APP1_BASE		0x10580000		// APP1
#define		DSS_APP2_BASE		0x10980000		// APP2 (new)
#define		DSS_APP2_BASE0		0x10700000		// APP2 (old)
extern UCHAR DssTelNum[23];		//alex 2011/02/22 
extern UCHAR DssRemoteIP[23];	//alex 2011/02/22 
extern UCHAR DssRemotePort[23];	//alex 2011/02/22 
extern UCHAR DssPort[23];		//alex 2011/02/22 
extern ULONG     DownburnSize;
ULONG     DownFileSize;
ULONG     DownReceiveSize;
extern ULONG     DownBaseAddr;
extern ULONG	    SystemRollback;
ULONG	 os_LAN_ENABLED=1;
extern UCHAR	DSS_fid;

extern	void	LIB_ResetSystem( void );	// defined in OS_LIB.c

BSP_TIMER	    * pShowTask = NULLPTR;

UINT32		os_DSS_FW	= 0;
UINT32		os_DSS_COM	= BSP_UART_0;

// ---------------------------------------------------------------------------
// FUNCTION: softreset
//
// INPUT   : none.
//
// OUTPUT  : none.
//
// RETURN  : none.
//
// ---------------------------------------------------------------------------

void softreset()
{
UINT8	apid;
UINT16	flag;

printf("softreset\n");
	// 2014-05-06, force to reset APID = 0 and "XT" state
	// BSP_DisableInterrupts( BSP_INT_MASK );
	
	// OS_FLS_GetData( F_ADDR_APP_ID, APP_ID_LEN, (UCHAR *)&apid );
	// if( (apid != 0) && (apid != 0xFF) )
	  // {
	  // apid = 0;
	  // OS_FLS_PutData( F_ADDR_APP_ID, APP_ID_LEN, (UCHAR *)&apid );
	  // }
// #ifdef	_F97_ENABLED_	// for AS350 only		
	// OS_FLS_GetData( F_ADDR_REF_F97, REF_F97_LEN, (UCHAR *)&flag );
	// if( flag == FLAG_REF_F97_ON )
	  // goto EXIT;

	// OS_FLS_GetData( F_ADDR_TAMPER_DETECT, TAMPER_DETECT_LEN, (UCHAR *)&flag );
	// if( flag != FLAG_DETECT_TAMPER_OFF )
	  // {
	  // flag = FLAG_DETECT_TAMPER_OFF;
	  // OS_FLS_PutData( F_ADDR_TAMPER_DETECT, TAMPER_DETECT_LEN, (UCHAR *)&flag );
	  // }
// #endif
// EXIT:
	
	// SYS_FUNC_ResetAppBootStatus();	// 2017-07-27
	// SYS_FUNC_SetAppStatus();	//
	
	LIB_ResetSystem();	// 2013-12-27
	// for(;;);
}

// ---------------------------------------------------------------------------
// FUNCTION: show digit string with right-justified. (max 20 digits)
// INPUT   : row  - row position to show.
//           buf  - L-V, the digit string.
//           type -
//           idle - character to be shown when idle. (eg. '0','-'...)
//           font - the font to be showed
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void ShowKey( UCHAR type, UCHAR idle, UCHAR font, UCHAR row, UCHAR *buf )
{
  UCHAR i;
  UCHAR len;
  UCHAR index;
  UCHAR max_dsp_cnt;
  UCHAR newbuf[30];

  memmove( newbuf, buf, buf[0]+1 );

  // TL_insert_decimal_point( type, newbuf ); // put decimal point if necessary

  LIB_LCD_ClearRow( row, 1, font );

  if ( (font & 0x0f) == FONT1 )
    max_dsp_cnt = MAX_DSP_FONT0_CNT;
  else
    max_dsp_cnt = MAX_DSP_FONT1_CNT;

  // check non-zero idle prompt
  if ( (buf[0] == 0) && (idle != '0') )
    {
      newbuf[1] = idle;
      LIB_LCD_Puts( row, max_dsp_cnt-1, font, 1, &newbuf[1] );
      return;
    }

  if ( newbuf[0] == 0 ) // no data in buffer, show '0'
    {
      newbuf[1] = idle;
      LIB_LCD_Puts( row, max_dsp_cnt-1, font, 1, &newbuf[1] );
    }
  else
    { // check special prompt
      if ( (type & NUM_TYPE_STAR) != 0 )
        {
          type &= 0xf7;

          for ( i=0; i<newbuf[0]; i++ )
            newbuf[i+1] = '*';
        }

      if ( (type & NUM_TYPE_COMMA) == 0 )
        { // NNNNNNNNNNNNNNNN...N
SHOW_NORM:
          if ( (max_dsp_cnt-newbuf[0]) > 0 )
            LIB_LCD_Puts( row, max_dsp_cnt-newbuf[0], font, newbuf[0], &newbuf[1] );
          else
            {
              index = newbuf[0]-max_dsp_cnt + 1;
              LIB_LCD_Puts( row, 0, font, max_dsp_cnt, &newbuf[index] );
            }
        }  //  9   6   3   0   7   4
      else // NN,NNN,NNN,NNN,NNN,NNN,NNN
        {
          len = newbuf[0];
          if ( len < 4 )
            goto SHOW_NORM;
          // to be implemented!
        }
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: get keys & show numeric digits.
// INPUT   : tout - time out before ENTER key pressed. (in seconds, 0=always)
//           row  - row position to show.
//           len  - max. length allowed.
//           type - bit 7 6 5 4 3 2 1 0
//                            | | | | |_ pure digits (NNNNNN)
//                            | | | |___ with sperator (NN,NNN)
//                            | | |_____ with decimal point (NNN.NN) cf. currency exponent.
//                            | |_______ special prompt (eg. ****) cf.
//                            |_________ accept leading '0' (eg. "0123", rather "123")
//
//           idle - character to be shown when idle. (eg. '0','-'...)
//           decipoint - press '*' to input '.' ( 0:disable  1:enable )
//           font - the font to be showed
// OUTPUT  : buf - the final result. (sizeof(buf) = len+1)
//                 format: LEN[1] DIGIT[n]
// REF     :
// RETURN  : TRUE  = confirmed. (by entering OK)
//           FALSE = aborted or timeout.   (by entering CANCEL)
// ---------------------------------------------------------------------------

UCHAR GetNumKey( UINT tout, UCHAR type, UCHAR idle, UCHAR font, UCHAR row, UCHAR len, UCHAR *buf, const UCHAR decipoint )
{
  UCHAR i = 0;
  UCHAR key;
  UCHAR dhn_tim;
  UCHAR dhn_tim2;
  UINT  tick1, tick2;
  UINT  tick3, tick4;

  if ( !decipoint )
    {
      LIB_OpenKeyNum(); // enable numeric keypad
    }
  else
    {
      // enable numeric keypad + '*' key

      UCHAR kdf[5];

      kdf[0]=0x03c;
      kdf[1]=0x03c;
      kdf[2]=0x03c;
      kdf[3]=0x02c;
      kdf[4]=0x000;
      api_kbd_open(0, kdf);
    }

  dhn_tim = api_tim_open(100); // time tick = 1sec
  api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );

  if( (buf[0] != 0) && (buf[0] <= len) )	// 2015-01-14, to keep old data at first time
    {
    i = buf[0]+1;
    ShowKey( type, idle, font, row, buf );
    goto KEYIN_WAIT;
    }
    
KEYIN_CLEAR:

  for ( i=0; i<=len; i++ )
    buf[i] = 0;  // clear output buffer

  i = 1;
  buf[0] = 0;
  key = 0;

BEGIN:

  while (1)
    {


      if (i == 1)
        ShowKey( type, idle, font, row, buf );


KEYIN_WAIT:
      if ( tout != 0 )
        {
          // wait key
          do
            {
              // check timeout
              api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
              if ( (tick2 - tick1) >= tout )
                {
                  api_tim_close( dhn_tim );
                  return( FALSE );
                }
            }
          while ( LIB_GetKeyStatus() == apiNotReady );
        }
      key = LIB_WaitKey();

      switch (key)
        {
        case 'x': // cancel
          for ( i=0; i<=len; i++ )
            buf[i] = 0;

          api_tim_close( dhn_tim );
          return( FALSE );
KEYIN_NO:
        case 'n': // clear
          LIB_LCD_ClearRow(row, 1, FONT2);
          i = 1;
          buf[0] = 0;
          goto KEYIN_CLEAR;
KEYIN_YES:
        case 'y': // ok
          //    if( i == 1 )
          //      continue;

          if ( buf[0] == 0 )
            {
              buf[0] = 1;
              buf[1] = '0';
            }

          api_tim_close( dhn_tim );
          return( TRUE );

        case '#': // backspace
          if ( i != 1 )
            {
              buf[--i] = 0;
              buf[0] -= 1;
              ShowKey( type, idle, font, row, buf );
              continue;
            }
          else
            goto KEYIN_CLEAR;

        case '*'://'.'

          buf[0] = i; // length of current keyin
          key='.';
          buf[i++] = key;
          ShowKey( type, idle, font, row, buf );
          continue;

          
        default: // '0'~'9'
          if ( i > len ) // overflow?
            continue;

          //    if( (type & NUM_TYPE_STAR) == 0 )
          if ( (type & NUM_TYPE_LEADING_ZERO) == 0 )
            {
              if ( (key == '0') && (buf[0] == 0) )
                {
                  buf[0] = 1;
                  buf[1] = '0';
                  ShowKey( type, idle, font, row, buf );
                  buf[0] = 0;
                  goto KEYIN_WAIT;
                }
            }

          buf[0] = i; // length of current keyin

          //    if( (type & 0x08) != 0 ) // check special prompt
          //      {
          //      type &= 0xf7;
          //      key = '*';
          //      }

          buf[i++] = key;
          ShowKey( type, idle, font, row, buf );

        } // switch(key)
    } // while(1)
}

// ---------------------------------------------------------------------------
// FUNCTION: Show and save the result of input
//
// INPUT   : showMessage - the string to be showed on title
//              messageLen - length of showMessage
//              filename - the name for file
//              leadZero - turn on "lead Zero" or not (0:off, 1:on)
//              setDeciPoint - turn on "press '0' twice will input '.' "  (0:off, 1:on)
// OUTPUT  : none.
// RETURN  : apiOK = save successfully
//                apiFailed = press cancel
// ---------------------------------------------------------------------------

UCHAR DownShowSaveInput( const UCHAR *showMessage, UCHAR messageLen, UCHAR *saveBuffer, UCHAR leadZero, UCHAR setDeciPoint )
{

  UCHAR    tempBuf[23];
  UCHAR    apiStatus;


  memmove( tempBuf, saveBuffer, sizeof( tempBuf ) );//For internal usage

  LIB_LCD_Cls();
  LIB_LCD_Puts( 0, 0, FONT2, messageLen, (UINT8 *)showMessage );

  if ( tempBuf[0] !=0 )
    {

      //sprintf( ( char * ) &DssTelNum[1], "%.*s", DssTelNum[0], &DssTelNum[1] );

      LIB_LCD_Puts( 3, 21-tempBuf[0], FONT1, tempBuf[0], (UINT8 *)&tempBuf[1] );


      if ( LIB_WaitKeyYesNo( 4, 0) )
        {
          return apiOK;
        }
      else
        {
//        memset( tempBuf, 0, sizeof( tempBuf ) );
          goto ENTER_PHONE;
        }

    }
  else
    {
ENTER_PHONE:

      LIB_LCD_Cls();
      printf("LCD cls\n");
      LIB_LCD_Puts( 0, 0, FONT2, messageLen, (UINT8 *)showMessage );

      if  ( GetNumKey( 0, leadZero+NUM_TYPE_DEC, '_', FONT1, 3, 21, tempBuf, setDeciPoint ) == FALSE )
        {
          return apiFailed;
        }
      else
        {
          memmove( saveBuffer, tempBuf, sizeof( tempBuf ) );//For internal usage

          return apiOK;
        }
    }

}

// ---------------------------------------------------------------------------
// FUNCTION: The menu for configuring the TEL number in MODEM mode
//
// INPUT   : none
//
// OUTPUT  : none.
//
// RETURN  : apiOK = save successfully
//                apiFailed = press cancel
// ---------------------------------------------------------------------------

UCHAR DownModemMenu()
{
  UCHAR	   result;
  UCHAR    const down_modem_menu[]={"Tel Number:"};

  result = DownShowSaveInput( down_modem_menu, sizeof(down_modem_menu)-1, DssTelNum, NUM_TYPE_LEADING_ZERO, 0 );
  // if( result == apiOK )
    // OS_FLS_PutData( F_ADDR_DssTelNum, DssTelNum_LEN, DssTelNum );	// 2015-08-05
    
  return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: The menu for configuring the remote IP, remote port, port in LAN mode
//
// INPUT   : none
//
// OUTPUT  : none.
//
// RETURN  : apiOK = save successfully
//                apiFailed = press cancel
// ---------------------------------------------------------------------------

UCHAR DownLanMenu()
{
#ifdef	_LAN_ENABLED_

  UCHAR    const down_lan_menu_localip[]={"Local IP:"};
  UCHAR    const down_lan_menu_gateway[]={"Gateway:"};
  UCHAR    const down_lan_menu_subnetmask[]={"SubNetMask:"};
  UCHAR    const down_lan_menu_remoteip[]={"Remote IP:"};
  UCHAR    const down_lan_menu_remoteport[]={"Remote Port:"};
  UCHAR    const down_lan_menu_port[]={"Local Port:"};

  UCHAR	msg_any_key[] = {"(Press any key)"};
  UCHAR  remote_IP[]={"remoteIP"};//file name
  UCHAR  remote_port[]={"remotePort"};//file name
  UCHAR  port[]={"port"};//file name
  UCHAR  remoteIPTemp[22];
  UCHAR  remotePortTemp[22];
  UCHAR  portTemp[22];
  UCHAR    apiStatus = apiFailed;
  UCHAR  ipsetFlag=0;
  
  ULONG	ipTemp[21];
  ULONG  ipLen;
  ULONG	gatewayTemp[21];
  ULONG  gatewayLen;
  ULONG  subnetmaskTemp[21];
  ULONG  subnetmaskLen;

  UCHAR dssLocalIP[23+1];
  UCHAR dssGateway[23+1];
  UCHAR dssSubNetMask[23+1];
//API_IPCONFIG ipconfig;
  API_IPCONFIG ipconfig[1];

  apiStatus = api_lan_getIPconfig( ipconfig );
  if ( apiStatus == apiFailed )
    {
      return apiFailed;
    }

  dssLocalIP[0] = ipconfig->IP_Len;
  memmove( &dssLocalIP[1], ipconfig->IP, ipconfig->IP_Len );

  dssGateway[0] = ipconfig->Gateway_Len;
  memmove( &dssGateway[1], ipconfig->Gateway, ipconfig->Gateway_Len );

  dssSubNetMask[0] = ipconfig->SubnetMask_Len;
  memmove( &dssSubNetMask[1], ipconfig->SubnetMask, ipconfig->SubnetMask_Len);

  //Show Local IP
  apiStatus =  DownShowSaveInput( down_lan_menu_localip, sizeof(down_lan_menu_localip)-1, dssLocalIP, NUM_TYPE_LEADING_ZERO, 1 );
  if (apiStatus != apiOK)
    {
      return apiStatus;
    }

  //Show gateway
  apiStatus =  DownShowSaveInput( down_lan_menu_gateway, sizeof(down_lan_menu_gateway)-1, dssGateway, NUM_TYPE_LEADING_ZERO, 1 );
  if (apiStatus != apiOK)
    {
      return apiStatus;
    }


  //Show subnetmask
  apiStatus =  DownShowSaveInput( down_lan_menu_subnetmask, sizeof(down_lan_menu_subnetmask)-1, dssSubNetMask, NUM_TYPE_LEADING_ZERO, 1 );
  if (apiStatus != apiOK)
    {
      return apiStatus;
    }


  if ( dssLocalIP[0]== ipconfig->IP_Len )
    {
      if ( memcmp( &dssLocalIP[1], ipconfig->IP, dssLocalIP[0]) )
        {
          ipsetFlag = 1;
        }
    }
  else
    {
      ipsetFlag = 1;
    }


  if ( dssGateway[0]== ipconfig->Gateway_Len )
    {
      if ( memcmp( &dssGateway[1], ipconfig->Gateway, dssGateway[0]) )
        {
          ipsetFlag = 1;
        }
    }
  else
    {
      ipsetFlag = 1;
    }

  if ( dssSubNetMask[0]== ipconfig->SubnetMask_Len )
    {
      if ( memcmp( &dssSubNetMask[1], ipconfig->SubnetMask, dssSubNetMask[0]) )
        {
          ipsetFlag = 1;
        }
    }
  else
    {
      ipsetFlag = 1;
    }

  if ( ipsetFlag==1 )
    {

      ipconfig->IP_Len = dssLocalIP[0];
      //ipconfig.IP = &DssLocalIP[1];
      memmove( ipconfig->IP, &dssLocalIP[1], dssLocalIP[0] );

      ipconfig->Gateway_Len = dssGateway[0] ;
      // ipconfig.Gateway = &DssGateway[1];
      memmove( ipconfig->Gateway, &dssGateway[1], dssGateway[0] );


      ipconfig->SubnetMask_Len = dssSubNetMask[0];
      // ipconfig.SubnetMask = &DssSubNetMask[1];
      memmove( ipconfig->SubnetMask, &dssSubNetMask[1], dssSubNetMask[0] );


      apiStatus = api_lan_setIPconfig( ipconfig[0] );
      if ( apiStatus == apiFailed )
        {
          return apiFailed;
        }
    }

  //Enter remote IP
  if( (DssRemoteIP[0] == 0) || (DssRemoteIP[0] > 15) )		// PATCH: 2011-03-03
    {
    DssRemoteIP[0] = 12;		// temp default setting
    DssRemoteIP[1] = '1';
    DssRemoteIP[2] = '7';
    DssRemoteIP[3] = '2';
    DssRemoteIP[4] = '.';
    DssRemoteIP[5] = '1';
    DssRemoteIP[6] = '6';
    DssRemoteIP[7] = '.';
    DssRemoteIP[8] = '1';
    DssRemoteIP[9] = '.';
    DssRemoteIP[10]= '1';
    DssRemoteIP[11]= '3';
    DssRemoteIP[12]= '7';
    // DssRemoteIP[13]= '1';
    // DssRemoteIP[14]= '2';
    }
  apiStatus =  DownShowSaveInput( down_lan_menu_remoteip, sizeof(down_lan_menu_remoteip)-1, DssRemoteIP, NUM_TYPE_LEADING_ZERO, 1 );
  if (apiStatus != apiOK)
    {
      return apiStatus;
    }
  // OS_FLS_PutData( F_ADDR_DssRemoteIP, DssRemoteIP_LEN, DssRemoteIP );	// 2015-08-05

  //Enter remote port
  if( (DssRemotePort[0] == 0) || (DssRemotePort[0] > 5) )	// PATCH: 2011-03-03	
    {
    DssRemotePort[0] = 5;		// temp default setting
    DssRemotePort[1] = '1';
    DssRemotePort[2] = '1';
    DssRemotePort[3] = '0';
    DssRemotePort[4] = '0';
    DssRemotePort[5] = '0';
    // DssRemotePort[5] = '0';
    }
  apiStatus = DownShowSaveInput( down_lan_menu_remoteport, sizeof(down_lan_menu_remoteport)-1, DssRemotePort, NUM_TYPE_LEADING_ZERO, 0 );
  if (apiStatus != apiOK)
    {
      return apiStatus;
    }
  // OS_FLS_PutData( F_ADDR_DssRemotePort, DssRemotePort_LEN, DssRemotePort );	// 2015-08-05

  //Enter port
  if( (DssPort[0] == 0) || (DssPort[0] > 5) )			// PATCH: 2011-03-03
    {
    DssPort[0] = 5;			// temp default setting
    DssPort[1] = '6';
    DssPort[2] = '5';
    DssPort[3] = '1';
    DssPort[4] = '0';
    DssPort[5] = '0';
    }
  apiStatus = DownShowSaveInput( down_lan_menu_port, sizeof(down_lan_menu_port)-1, DssPort, NUM_TYPE_LEADING_ZERO, 0 ) ;
  if (apiStatus != apiOK)
    {
      return apiStatus;
    }
  // OS_FLS_PutData( F_ADDR_DssPort, DssPort_LEN, DssPort );	// 2015-08-05

#endif

  return apiOK;
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase the first sector of APP1 and APP2 on NEW_PLATFORM.
//
// INPUT   : none.
//
// OUTPUT  : none.
//
// RETURN  : none.
// ---------------------------------------------------------------------------
void	SystemRollbackProcess( void )
{
#if 0	
UCHAR	apid;
//UCHAR	const msg_processing[] = {"Processing..."};
  

	if( SystemRollback )
	  {
	  SystemRollback = 0;
	  
	  // erase the first sector of APP1 & APP2 on NEW_PLATFORM
	  // erase the first sector of APP2 on OLD_PLATFORM
//	  LIB_LCD_Puts( 3, 0, FONT0, sizeof(msg_processing), (UINT8 *)msg_processing );
//	  
//	  api_flash_erase( DSS_APP1_BASE,  64*1024 );	// APP1 new
	  api_flash_erase( DSS_APP2_BASE,  64*1024 );	// APP2 new

	  apid = 0;	// force APID = 0
	  OS_FLS_PutData( F_ADDR_APP_ID, 1, (UCHAR *)&apid );
	  
	  api_flash_erase( DSS_APP2_BASE0, 64*1024 );	// APP2 old
	  }
#endif  
}

// ---------------------------------------------------------------------------
// FUNCTION: The main user interface for DSS( for download AP only )
//
// INPUT   : none.
//
// OUTPUT  : none.
//
// RETURN  : apiOK:
//               apiFailed: 1. getting ip configuration failed
//                             2. saving ip configuration failed
// ---------------------------------------------------------------------------

UCHAR api_dss_interface()
{
  UCHAR    const dss_menu[]={"======   DSS   ======"};
  UCHAR    const dss_func1[]={"[1] UART "};
  UCHAR    const dss_func2[]={"[2] MODEM "};
  UCHAR    const dss_func3[]={"[3] ETHERNET "};
  UCHAR    const dss_message_successful[]={"Successfully!"};
  UCHAR    const dss_message_reboot[]={"Reboot..."};
  UCHAR    const dss_message_format1[]={"Please format the"};
  UCHAR    const dss_message_format2[]={"file system before!"};
  UCHAR	   const dss_format[] = {"FORMAT(Y/N)?"};
  UCHAR	   const dss_com0[] = {"[1] COM0"};
  UCHAR	   const dss_com2[] = {"[2] COM1"};
  UCHAR	   const msg_any_key[] = {"(Press any key)"};
  UCHAR	   sbuf[256];
  UCHAR	   dbuf[256];
  UCHAR    apiStatus;
  UCHAR	   first = 1;
  UCHAR	result;
  UCHAR LineStatus;
  DOWREQPAC ReqItem;


  os_DSS_FW = 0;

  memset( &ReqItem, 0, sizeof( ReqItem ) );
  // memcpy ( &ReqItem.AID, "APP", sizeof("APP") );
	
MENU:
  LIB_LCD_Cls();
  LIB_LCD_Puts( 0, 0, FONT2+attrREVERSE, sizeof( dss_menu ), (UINT8 *)dss_menu );
  // printf("sizeof( dss_menu )=%d  ",sizeof( dss_menu ));

SEL_CHANNEL:

  LIB_LCD_ClearRow( 1, 2, FONT2 );

  LIB_LCD_Puts( 1, 0, FONT2, sizeof(dss_func1), (UINT8 *)dss_func1 );

#ifdef	_MODEM_ENABLED_
  LIB_LCD_Puts( 2, 0, FONT2, sizeof(dss_func2), (UINT8 *)dss_func2 );
#endif

#ifdef	_LAN_ENABLED_
  if( os_LAN_ENABLED )	// 216-07-06
    LIB_LCD_Puts( 3, 0, FONT2, sizeof(dss_func3), (UINT8 *)dss_func3 );
#endif

ENTER_MENU:
  switch ( LIB_WaitKey() )
    {

    case ( '1' ):
    {
      LIB_LCD_ClearRow( 1, 3, FONT2 );
      LIB_LCD_Puts( 1, 0, FONT2, sizeof(dss_com0), (UINT8 *)dss_com0 );
      LIB_LCD_Puts( 2, 0, FONT2, sizeof(dss_com2), (UINT8 *)dss_com2 );
      do{
        result = LIB_WaitKey();
        switch( result )
              {
              case '1':
                   os_DSS_COM = COM0;
                   break;
                   
              case '2':
                   os_DSS_COM = COM1;
                   break;
                   
              case 'x':
              	  //  SYS_FUNC_SetAppBootStatus();	// 20210219 comment out by west because don't know doing what
                   return( apiOK );
              }
        } while( (result != '1') && (result != '2') );


      apiStatus = DownUi( DownUartMode, &ReqItem );
      if ( apiStatus == apiOK )
        {
          if( DSS_fid )
            {
            LIB_WaitKey();
            goto MENU;
            }
            
          LIB_LCD_Cls();
          LIB_LCD_Puts( 2, 0, FONT1, sizeof(dss_message_successful), (UINT8 *)dss_message_successful );
          
          SystemRollbackProcess();
          
          if(memcmp(&ReqItem.AID[0], "CA_CRT", sizeof("CA_CRT")) && 
             memcmp(&ReqItem.AID[0], "C_CRT", sizeof("C_CRT")) &&
             memcmp(&ReqItem.AID[0], "C_PRV", sizeof("C_PRV")))
          {
            LIB_LCD_Puts( 3, 0, FONT1+attrCLEARWRITE, sizeof(dss_message_reboot), (UINT8 *)dss_message_reboot );
            softreset();
          }
        }

      //  SYS_FUNC_SetAppBootStatus();	// 20210219 comment out by west because don't know doing what
      return apiFailed;
    }

#ifdef	_MODEM_ENABLED_

    case ( '2' ):
    {
      //modem connection
      apiStatus = DownModemMenu();
      if ( apiStatus != apiOK )
        {
          if ( apiStatus == apiFileError )
            {
              LIB_LCD_Cls();
              LIB_LCD_Puts( 1, 0, FONT1, sizeof(dss_message_format1)-1,(UINT8 *) dss_message_format1 );
              LIB_LCD_Puts( 2, 0, FONT1, sizeof(dss_message_format2)-1, (UINT8 *)dss_message_format2 );
              LIB_LCD_Puts( 5, 0, FONT1, sizeof(msg_any_key)-1, (UINT8 *)msg_any_key );
              LIB_WaitKey();
            }
          //  SYS_FUNC_SetAppBootStatus();	// 20210219 comment out by west because don't know doing what
          return apiFailed;
        }

      LIB_LCD_Cls();
      apiStatus = DownUi( DownModemMode, &ReqItem );

      if ( apiStatus == apiOK )
        {
          if( DSS_fid )
            {
            LIB_WaitKey();
            goto MENU;
            }
            
          LIB_LCD_Cls();
          LIB_LCD_Puts( 2, 0, FONT1, sizeof(dss_message_successful), (UINT8 *)dss_message_successful );
          
          SystemRollbackProcess();
          
          if(memcmp(&ReqItem.AID[0], "CA_CRT", sizeof("CA_CRT")) && 
             memcmp(&ReqItem.AID[0], "C_CRT", sizeof("C_CRT")) &&
             memcmp(&ReqItem.AID[0], "C_PRV", sizeof("C_PRV")))
          {
            LIB_LCD_Puts( 3, 0, FONT1+attrCLEARWRITE, sizeof(dss_message_reboot), (UINT8 *)dss_message_reboot );
            softreset();
          }
        }

      //  SYS_FUNC_SetAppBootStatus();	// 20210219 comment out by west because don't know doing what
      return apiFailed;
    }

#endif

#ifdef	_LAN_ENABLED_

    case ( '3' ):
    {
      if( !os_LAN_ENABLED )	// 2016-07-06
      	break;
      	
      apiStatus =  DownLanMenu();
      if ( apiStatus != apiOK )
        {
          if ( apiStatus == apiFileError )
            {
              LIB_LCD_Cls();
              LIB_LCD_Puts( 1, 0, FONT1, sizeof(dss_message_format1)-1, (UINT8 *)dss_message_format1 );
              LIB_LCD_Puts( 2, 0, FONT1, sizeof(dss_message_format2)-1, (UINT8 *)dss_message_format2 );
              LIB_LCD_Puts( 5, 0, FONT1, sizeof(msg_any_key)-1, (UINT8 *)msg_any_key );
              LIB_WaitKey();
            }
          //  SYS_FUNC_SetAppBootStatus();	// 20210219 comment out by west because don't know doing what
          return apiFailed;
        }
      LIB_LCD_Cls();
      apiStatus = DownUi( DownLanMode, &ReqItem );

      if ( apiStatus == apiOK )
        {
          if( DSS_fid )
            {
            LIB_WaitKey();
            goto MENU;
            }
            
          LIB_LCD_Cls();
          LIB_LCD_Puts( 2, 0, FONT1, sizeof(dss_message_successful), (UINT8 *)dss_message_successful );
          
          SystemRollbackProcess();
          
          if(memcmp(&ReqItem.AID[0], "CA_CRT", sizeof("CA_CRT")) && 
             memcmp(&ReqItem.AID[0], "C_CRT", sizeof("C_CRT")) &&
             memcmp(&ReqItem.AID[0], "C_PRV", sizeof("C_PRV")))
          {
            LIB_LCD_Puts( 3, 0, FONT1+attrCLEARWRITE, sizeof(dss_message_reboot), (UINT8 *)dss_message_reboot );
            softreset();
          }
        }

      //  SYS_FUNC_SetAppBootStatus();	// 20210219 comment out by west because don't know doing what
      return apiFailed;
    }
    
#endif

    case ('x')://cancel
    {
      //  SYS_FUNC_SetAppBootStatus();	// 20210219 comment out by west because don't know doing what
      return apiFailed;
    }

    default:
    {
      goto ENTER_MENU;

    }
    }



}

// ---------------------------------------------------------------------------
// FUNCTION: The main user interface for DSS( for download M3 firmware only )
//
// INPUT   : none.
//
// OUTPUT  : none.
//
// RETURN  : apiOK:
//               apiFailed: 1. getting ip configuration failed
//                             2. saving ip configuration failed
// ---------------------------------------------------------------------------
UCHAR api_dss_interface2()
{
  UCHAR    const dss_menu[]={"====  DSS  ===="};
  UCHAR    const dss_func1[]={"[1] UART "};
  UCHAR    const dss_func2[]={"[2] MODEM "};
  UCHAR    const dss_func3[]={"[3] ETHERNET "};
  UCHAR    const dss_message_successful[]={"Successfully!"};
  UCHAR    const dss_message_reboot[]={"Reboot..."};
  UCHAR    const dss_message_format1[]={"Please format the"};
  UCHAR    const dss_message_format2[]={"file system before!"};
  UCHAR	   const msg_any_key[] = {"(Press any key)"};
  UCHAR	   sbuf[256];
  UCHAR	   dbuf[256];
  UCHAR    apiStatus;
  UCHAR	result;
  UCHAR LineStatus;
  DOWREQPAC ReqItem;


  os_DSS_FW = 1;

  memset( &ReqItem, 0, sizeof( ReqItem ) );
  memcpy ( &ReqItem.AID, "APP", sizeof("APP") );

	
  LIB_LCD_Cls();
  LIB_LCD_Puts( 0, 0, FONT2+attrREVERSE, sizeof( dss_menu ), (UINT8 *)dss_menu );
  LIB_LCD_Puts( 1, 0, FONT2, sizeof(dss_func1), (UINT8 *)dss_func1 );

#ifdef	_MODEM_ENABLED_
  LIB_LCD_Puts( 2, 0, FONT2, sizeof(dss_func2), (UINT8 *)dss_func2 );
#endif

#ifdef	_LAN_ENABLED_
  if( os_LAN_ENABLED )	// 2016-07-06
    LIB_LCD_Puts( 3, 0, FONT2, sizeof(dss_func3), (UINT8 *)dss_func3 );
#endif

ENTER_MENU:
  switch ( LIB_WaitKey() )
    {

    case ( '1' ):
    {
      apiStatus = DownUi( DownUartMode, &ReqItem );
      if ( apiStatus == apiOK )
        {
          LIB_LCD_Cls();
//        LIB_LCD_Puts( 2, 0, FONT0, sizeof(dss_message_successful), (UINT8 *)dss_message_successful );
//        LIB_LCD_Puts( 3, 0, FONT0, sizeof(dss_message_reboot), (UINT8 *)dss_message_reboot );
//        softreset();
	  return( apiOK );
        }

      return apiFailed;
    }

#ifdef	_MODEM_ENABLED_

    case ( '2' ):
    {
      //modem connection
      apiStatus = DownModemMenu();
      if ( apiStatus != apiOK )
        {
          if ( apiStatus == apiFileError )
            {
              LIB_LCD_Cls();
              LIB_LCD_Puts( 1, 0, FONT1, sizeof(dss_message_format1)-1,(UINT8 *) dss_message_format1 );
              LIB_LCD_Puts( 2, 0, FONT1, sizeof(dss_message_format2)-1, (UINT8 *)dss_message_format2 );
              LIB_LCD_Puts( 5, 0, FONT1, sizeof(msg_any_key)-1, (UINT8 *)msg_any_key );
              LIB_WaitKey();
            }
          return apiFailed;
        }

      LIB_LCD_Cls();
      apiStatus = DownUi( DownModemMode, &ReqItem );

      if ( apiStatus == apiOK )
        {
          LIB_LCD_Cls();
//        LIB_LCD_Puts( 2, 0, FONT0, sizeof(dss_message_successful), (UINT8 *)dss_message_successful );
//        LIB_LCD_Puts( 3, 0, FONT0, sizeof(dss_message_reboot), (UINT8 *)dss_message_reboot );
//        softreset();
	  return( apiOK );
        }

      return apiFailed;
    }

#endif

#ifdef	_LAN_ENABLED_

    case ( '3' ):
    {
      if( !os_LAN_ENABLED )	// 2016-07-06
      	break;
      	
      apiStatus =  DownLanMenu();
      if ( apiStatus != apiOK )
        {
          if ( apiStatus == apiFileError )
            {
              LIB_LCD_Cls();
              LIB_LCD_Puts( 1, 0, FONT1, sizeof(dss_message_format1)-1, (UINT8 *)dss_message_format1 );
              LIB_LCD_Puts( 2, 0, FONT1, sizeof(dss_message_format2)-1, (UINT8 *)dss_message_format2 );
              LIB_LCD_Puts( 5, 0, FONT1, sizeof(msg_any_key)-1, (UINT8 *)msg_any_key );
              LIB_WaitKey();
            }
          return apiFailed;
        }
      LIB_LCD_Cls();
      apiStatus = DownUi( DownLanMode, &ReqItem );

      if ( apiStatus == apiOK )
        {
          LIB_LCD_Cls();
//        LIB_LCD_Puts( 2, 0, FONT0, sizeof(dss_message_successful), (UINT8 *)dss_message_successful );
//        LIB_LCD_Puts( 3, 0, FONT0, sizeof(dss_message_reboot), (UINT8 *)dss_message_reboot );
//        softreset();
	  return( apiOK );
        }

      return apiFailed;
    }
    
#endif

    case ('x')://cancel
    {
      return apiFailed;
    }

    default:
    {
      goto ENTER_MENU;

    }
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: The main user interface for DSS( for download 4G LTE61 firmware only )
//
// INPUT   : none.
//
// OUTPUT  : none.
//
// RETURN  : apiOK:
//               apiFailed: 1. getting ip configuration failed
//                             2. saving ip configuration failed
// ---------------------------------------------------------------------------
UCHAR api_dss_interface3()
{
  UCHAR    const dss_menu[]={"====  DSS  ===="};
//  UCHAR    const dss_func3[]={"[3] ETHERNET "};
  UCHAR    const dss_message_successful[]={"Successfully!"};
  UCHAR    const dss_message_reboot[]={"Reboot..."};
  UCHAR    const dss_message_format1[]={"Please format the"};
  UCHAR    const dss_message_format2[]={"file system before!"};
  UCHAR	   const msg_any_key[] = {"(Press any key)"};
  UCHAR	   sbuf[256];
  UCHAR	   dbuf[256];
  UCHAR    apiStatus;
  UCHAR	result;
  UCHAR LineStatus;
  DOWREQPAC ReqItem;


  os_DSS_FW = 1;

  memset( &ReqItem, 0, sizeof( ReqItem ) );
  memcpy ( &ReqItem.AID, "APP", sizeof("APP") );


//  LIB_LCD_Cls();
//  LIB_LCD_Puts( 0, 0, FONT1+attrREVERSE, sizeof( dss_menu ), (UINT8 *)dss_menu );


//#ifdef	_LAN_ENABLED_
//  if( os_LAN_ENABLED )	// 2016-07-06
//    LIB_LCD_Puts( 3, 0, FONT1, sizeof(dss_func3), (UINT8 *)dss_func3 );
//#endif

ENTER_MENU:
//  switch ( LIB_WaitKey() )
//    {

#ifdef	_LAN_ENABLED_

//    case ( '3' ):
//    {
//      if( !os_LAN_ENABLED )	// 2016-07-06
//      	break;
      	
      apiStatus =  DownLanMenu();
      if ( apiStatus != apiOK )
        {
          if ( apiStatus == apiFileError )
            {
              LIB_LCD_Cls();
              return( apiOK );
            }
          else
            return( apiFailed );
        }
      LIB_LCD_Cls();
      apiStatus = DownUi( DownLanMode, &ReqItem );

      if ( apiStatus == apiOK )
        {
          LIB_LCD_Cls();
	  return( apiOK );
        }

      return apiFailed;
//    }
    
#endif

//    case ('x')://cancel
//    {
//      return apiFailed;
//    }

//    default:
//    {
//      goto ENTER_MENU;
//
//    }
//    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Get beginning address of DSS working buffer.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : NULLPTR
//	     address
// ---------------------------------------------------------------------------
UINT8	*api_dss_getaddr( void )
{
	return( DownSdramStartAddr );
}

// ---------------------------------------------------------------------------
// FUNCTION: The DSS function called by AP (for downloading AP, OS, BIOS, TMSPARA, EMVPARA)
//
// INPUT   : channel:  0:UART, 1:MODEM, 2:Lan
//              ReqItem:struct DownReqFormat
//                             {
//                                 UCHAR TID[8];
//                                 UCHAR BID[4];
//                                 UCHAR Reserv[12];
//                                 UCHAR AID[8];
//                                 UCHAR FID[32];
//                                 UCHAR VER[8];
//                                 UCHAR DATE[8];
//                                 ULONG Offset;
//                               }__attribute__((packed));;
//
// OUTPUT  : none.
//
// RETURN  : apiOK
//               apiFailed
//
// ---------------------------------------------------------------------------
UCHAR api_dss_ap ( UCHAR channel, DOWREQPAC *ReqItem )
{
  UCHAR apiStatus;

  if ( ! memcmp ( &ReqItem-> AID[0], "APP", sizeof("APP") ) || ! memcmp ( &ReqItem-> AID[0], "OS", sizeof("OS") ) ||  ! memcmp ( &ReqItem-> AID[0], "BIOS", sizeof("BIOS") ) )
    {
      apiStatus = DownMain( channel, ReqItem, NULLPTR, 0, NULLPTR );
      if (apiStatus == apiOK )
        {
          return apiOK;
        }
      else
        {
          return apiFailed;
        }
    }
  else
    {
      return apiFailed;
    }

}

// ---------------------------------------------------------------------------
// FUNCTION: The DSS function called by AP (for downloading AP, OS, BIOS, TMSPARA, EMVPARA)
//
// INPUT   : channel:  0:UART, 1:MODEM, 2:Lan
//              ReqItem:struct DownReqFormat
//                             {
//                                 UCHAR TID[8];
//                                 UCHAR BID[4];
//                                 UCHAR Reserv[12];
//                                 UCHAR AID[8];
//                                 UCHAR FID[32];
//                                 UCHAR VER[8];
//                                 UCHAR DATE[8];
//                                 ULONG Offset;
//                               }__attribute__((packed));;
//
//               bufferSize: the size of buffer to put parameters
//
// OUTPUT  : *buffer: the place to put parameters
//                *outputSize: the size of received parameter
//
// RETURN  : apiOK
//               apiFailed
//
// ---------------------------------------------------------------------------

UCHAR api_dss_parameter ( UCHAR channel, DOWREQPAC *ReqItem, UCHAR *buffer, UINT bufferSize, UINT *outputSize )
{
  UCHAR apiStatus;

  if ( ! memcmp ( &ReqItem-> AID[0], "TMSPARA", sizeof("TMSPARA") )  ||  ! memcmp ( &ReqItem-> AID[0], "EMVPARA", sizeof("EMVPARA") ) ||  ! memcmp ( &ReqItem-> AID[0], "CAPK", sizeof("CAPK") ))
    {
      apiStatus = DownMain( channel, ReqItem, buffer, bufferSize, outputSize );
      if (apiStatus == apiOK )
        {
          return apiOK;
        }
      else
        {
          return apiFailed;
        }
    }
  else
    {
      return apiFailed;
    }
}
// ---------------------------------------------------------------------------
// FUNCTION: The task used for monitoring download percentage
//
// INPUT   : none.
//
// OUTPUT  : none.
//
// RETURN  : none.
// ---------------------------------------------------------------------------

void* Down_monitor_download_task(void)
{
  UINT      precent;
  UCHAR    down_message_present1[]={" Download "};
  UCHAR    down_message_present2[]={" Percentage: "};
  UCHAR  oneDigit = 0;
  UCHAR  twoDigit = 0;
  UCHAR  showDigit[2];
  UCHAR  temp[21];//for test
  ULONG	 COUNT = 0;

          
//        LIB_LCD_Cls();
          LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_message_present1)-1, (UINT8 *)down_message_present1 );
          LIB_LCD_Puts( 2, 0, FONT1, sizeof(down_message_present2)-1, (UINT8 *)down_message_present2 );
  printf("DownFileSize=%d\n",DownFileSize);        
  while ( 1 )
    {
      if(BSP_TMR_GetTick(pShowTask,&COUNT)>0)
      {
          if (  DownFileSize !=0 && (DownReceiveSize <= DownFileSize) )
          {
              precent = (DownReceiveSize*100)/DownFileSize ;

              memset( showDigit, 0, sizeof( showDigit ) );
              memset( temp, 0, sizeof(temp) );

              if ( ( UCHAR )( precent / 10 ) == 0 )
                {
                  showDigit[0] = ' ';
                }
              else
                {
                  showDigit[0] = ( UCHAR )( precent / 10 ) + 0x30;
                }

              showDigit[1] = ( UCHAR )( precent % 10 ) + 0x30;

//            sprintf( ( char * ) temp, "%s%.*s%c", down_message_present2, sizeof( showDigit ) , showDigit, '%' );
              sprintf( ( char * ) temp, "%.*s%c", sizeof( showDigit ) , showDigit, '%' );

//              LIB_LCD_Cls();
//              LIB_LCD_Puts( 1, 0, FONT0, sizeof(down_message_present1)-1, (UINT8 *)down_message_present1 );
              LIB_LCD_Puts( 2, sizeof(down_message_present2)-1, FONT1, sizeof(temp)-1, (UINT8 *)temp );

              // NosSleep(50);

          }
          else
          {
              // NosSleep(50);
          }
      }
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: The task used for monitoring burning percentage
//
// INPUT   : none.
//
// OUTPUT  : none.
//
// RETURN  : none.
// ---------------------------------------------------------------------------

void Down_monitor_burn_task(void)
{
  UINT      precent;
//UCHAR    down_message_present1[]={" Burning "};
  UCHAR    down_message_present1[]={" Programming "};
  UCHAR    down_message_present2[]={" Percentage: "};
  UCHAR  oneDigit = 0;
  UCHAR  twoDigit = 0;
  UCHAR  showDigit[2];
  UCHAR  temp[21];//for test

  while ( 1 )
    {
      if ( DownFileSize !=0 )
        {
          precent = (DownburnSize*100)/DownFileSize ;
          memset( showDigit, 0, sizeof( showDigit ) );
          memset( temp, 0, sizeof(temp) );

          if ( ( UCHAR )( precent / 10 ) == 0 )
            {
              showDigit[0] = ' ';
            }
          else
            {
              showDigit[0] = ( UCHAR )( precent / 10 ) + 0x30;
            }

          showDigit[1] = ( UCHAR )( precent % 10 ) + 0x30;



          sprintf( ( char * ) temp, "%s%.*s%c", down_message_present2, sizeof( showDigit ) , showDigit, '%' );

          LIB_LCD_Cls();
          LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_message_present1)-1, (UINT8 *)down_message_present1 );
          LIB_LCD_Puts( 2, 0, FONT1, sizeof(temp)-1, (UINT8 *)temp );

          // NosSleep(100);

        }
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: DownUi
//              The function is used to show error message
//
// INPUT   : channel: 0:UART, 1:MODEM, 2:TCP/IP
//              ReqItem:struct DownReqFormat
//                             {
//                                 UCHAR TID[8];
//                                 UCHAR BID[4];
//                                 UCHAR Reserv[12];
//                                 UCHAR AID[8];
//                                 UCHAR FID[32];
//                                 UCHAR VER[8];
//                                 UCHAR DATE[8];
//                                 ULONG Offset;
//                               }__attribute__((packed));;
// OUTPUT  : none.
//
// RETURN  : apiOK
//		      apiFailed
// ---------------------------------------------------------------------------

UCHAR DownUi ( UCHAR channel, DOWREQPAC *ReqItem )
{
  UCHAR apiStatus = apiOK;
  UCHAR    const down_verify_failed[]= {"Verify failed"};
  UCHAR    const down_File_not_found1[]= {"The file does't "};
  UCHAR    const down_File_not_found2[]= {"exist!"};
  UCHAR    const down_timeout[]= {"Timeout occurs"};
  UCHAR    const down_offset[]= {"Offset occurs"};
  UCHAR    const down_connect_fail[]={"Connect failed!"};
  UCHAR    const down_noline[]={"No line!"};
  UCHAR    const down_failed[]={"Download failed"};
  UCHAR    const down_write_failed[]={"Writing failed"};
  UCHAR    const down_erase_failed[]={"Erasing flash failed"};
  UCHAR	  const msg_any_key[] = {"(Press any key)"};


  apiStatus = DownMain( channel, ReqItem, NULLPTR, 0, NULLPTR );
  switch ( apiStatus )
    {
    case ( apiOK ):
    {
      return apiOK;
    }
    case ( apiVerifyFailed ):
    {
      if( os_DSS_FW )	// 2013-10-22, download M3 firmware
        return( apiOK );
      
      LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_verify_failed)-1, (UINT8 *)down_verify_failed );
      LIB_LCD_Puts( 5, 0, FONT1, sizeof(msg_any_key)-1, (UINT8 *)msg_any_key );
      LIB_WaitKey();

      return apiFailed;
    }
    case (apiEraseFailed):
    {
      LIB_LCD_Cls();
      LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_erase_failed)-1, (UINT8 *)down_erase_failed );
      LIB_LCD_Puts( 5, 0, FONT1, sizeof(msg_any_key)-1, (UINT8 *)msg_any_key );
      LIB_WaitKey();

      return apiFailed;
    }
    case (apiFileNotFound):
    {
      LIB_LCD_Cls();
      LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_File_not_found1)-1, (UINT8 *)down_File_not_found1 );
      LIB_LCD_Puts( 2, 0, FONT1, sizeof(down_File_not_found2)-1, (UINT8 *)down_File_not_found2 );
      LIB_LCD_Puts( 5, 0, FONT1, sizeof(msg_any_key)-1, (UINT8 *)msg_any_key );
      LIB_WaitKey();

      return apiFailed;
    }
    case (apiTimeOut ):
    {
      LIB_LCD_Cls();
      LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_timeout)-1, (UINT8 *)down_timeout );
      LIB_LCD_Puts( 5, 0, FONT1, sizeof(msg_any_key)-1, (UINT8 *)msg_any_key );
      LIB_WaitKey();

      return apiFailed;
    }
    case (apiOffsetError):
    {
      LIB_LCD_Cls();
      LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_offset)-1, (UINT8 *)down_offset );
      LIB_LCD_Puts( 5, 0, FONT1, sizeof(msg_any_key)-1, (UINT8 *)msg_any_key );
      LIB_WaitKey();

      return apiFailed;
    }
    case (apiConnectFailed):
    {
      LIB_LCD_Cls();
      LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_connect_fail)-1, (UINT8 *)down_connect_fail );
      LIB_LCD_Puts( 5, 0, FONT1, sizeof(msg_any_key)-1, (UINT8 *)msg_any_key );
      LIB_WaitKey();


      return apiFailed;
    }
    case (apiNoLine):
    {
      LIB_LCD_Cls();
      LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_noline)-1, (UINT8 *)down_noline );
      LIB_LCD_Puts( 5, 0, FONT1, sizeof(msg_any_key)-1,(UINT8 *) msg_any_key );
      LIB_WaitKey();

      return apiFailed;
    }
    case (apiWriteFailed):
    {
      LIB_LCD_Cls();
      LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_write_failed)-1, (UINT8 *)down_write_failed );
      LIB_LCD_Puts( 5, 0, FONT1, sizeof(msg_any_key)-1, (UINT8 *)msg_any_key );
      LIB_WaitKey();

      return apiFailed;
    }

    default:
    {
      LIB_LCD_Cls();
      LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_failed)-1, (UINT8 *)down_failed );
      LIB_LCD_Puts( 5, 0, FONT1, sizeof(msg_any_key)-1, (UINT8 *)msg_any_key );
      LIB_WaitKey();

    }

    }

}


// ---------------------------------------------------------------------------
// FUNCTION: DownMain
//              The main body of DSS
//
// INPUT   : channel: 0:UART, 1:MODEM, 2:TCP/IP
//              
//              *ReqItem: struct DownReqFormat
//                             {
//                                 UCHAR TID[8];
//                                 UCHAR BID[4];
//                                 UCHAR Reserv[12];
//                                 UCHAR AID[8];
//                                 UCHAR FID[32];
//                                 UCHAR VER[8];
//                                 UCHAR DATE[8];
//                                 ULONG Offset;
//                               }__attribute__((packed));;
//             
//              bufferSize: size of user's buffer
//
//
// OUTPUT  : *buffer : user's buffer to place download data
//                *outputSize: size of data to be placed into buffer 
//
// RETURN  : apiOK
//		      apiFailed
//               apiVerifyFailed
//               apiEraseFailed
//               apiWriteFailed
//               apiFileNotFound
//               apiTimeOut
//               apiOffsetError
//               apiConnectFailed
//               apiNoLine
// ---------------------------------------------------------------------------
extern UCHAR Header_LRC_Eraser();
UCHAR DownMain ( UCHAR channel, DOWREQPAC *ReqItem, UCHAR *buffer, UINT bufferSize, UINT *outputSize )
{
  UCHAR apiStatus=apiOK;
  UCHAR    const down_connecting[]={"Connecting..."};
  UCHAR    const down_complete[]={"Download completely!"};
  UCHAR    const down_verify_success[]={"Verify successfully!"};



  DownFileSize = 0;
  DownReceiveSize = 0;
  DownburnSize = 0;
  DownBaseAddr = 0;


ContinDownload:


  LIB_LCD_Cls();
  LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_connecting)-1, (UINT8 *)down_connecting );

  //Create monitor download percentage task
  // pShowTask = NosCreate( Down_monitor_download_task, 7, 0x4000, 0 );
  pShowTask = BSP_TMR_Acquire(20);
	BSP_TMR_Start( (void *)Down_monitor_download_task,pShowTask );
  apiStatus = DownCom ( channel, ReqItem );
  // printf("ReqItem-> AID= ");
  //       for(int i=0;i<3;i++)
  //         printf("%c",ReqItem-> AID[i]);
  //       printf("\n");
  if ( apiStatus == apiOK )
    {
      BSP_TMR_Stop( pShowTask );
      LIB_LCD_Cls();
      LIB_LCD_Puts( 1, 0, FONT1, sizeof(down_complete)-1, (UINT8 *)down_complete );
      apiStatus = DownVerifyData( channel, ReqItem );
      if ( apiStatus == apiFailed )
        {
          return apiVerifyFailed;
        }
      else
        {
          LIB_LCD_Puts( 2, 0, FONT1, sizeof(down_verify_success)-1, (UINT8 *)down_verify_success );
        }
      //Refresh RTC
//    DownRefreshRTC();		// 2012-10-05
      //The process depends on the parameter passed by AP
      //Header_LRC_Eraser();    //Remarked by Tammy, this will make the download fail ====
      apiStatus = DownDataProcess( ReqItem, buffer, bufferSize, outputSize );
      if ( apiStatus == apiOK )
        {
          return apiOK;
        }
      else if ( apiStatus == DownContinue )
        {
          goto ContinDownload;
        }
      else if (apiStatus == apiEraseFailed )
        {
          return apiEraseFailed;
        }
      else if ( apiStatus == apiWriteFailed )
        {
          return apiWriteFailed;
        }
      else
        {
          return apiFailed;
        }

    }
  else if ( apiStatus == apiFileNotFound )
    {
      BSP_TMR_Stop( pShowTask );


      return apiFileNotFound;
    }
  else if ( apiStatus == apiTimeOut )
    {
      BSP_TMR_Stop( pShowTask );

      return apiTimeOut;
    }
  else if ( apiStatus == apiOffsetError )
    {
      BSP_TMR_Stop( pShowTask );

      return apiOffsetError;
    }
  else if (apiStatus == apiConnectFailed )
    {
      BSP_TMR_Stop( pShowTask );

      return apiConnectFailed;
    }
  else if ( apiStatus == apiNoLine )
    {
      BSP_TMR_Stop( pShowTask );


      return apiNoLine;
    }
  else
    {
      BSP_TMR_Stop( pShowTask );


      return apiFailed;
    }

}



