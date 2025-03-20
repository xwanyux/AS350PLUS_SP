#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bsp_tmr.h"
#include "bsp_types.h"
#include "POSAPI.h"
#include "DownPacket.h"
#include "OS_LIB.h"
#include "OS_FLASH.h"
#include "OS_MSG.h"
#include "OS_SECM.h"
#include "OS_PED.h"
#if 0
#include "LTEAPI.h"
#endif


extern UCHAR DownShowSaveInput( const UCHAR *showMessage, UCHAR messageLen, UCHAR *saveBuffer, UCHAR leadZero, UCHAR setDeciPoint );


UCHAR printSECMdata(UINT32	addr, UINT32 len)
{
UCHAR buf[len];
  OS_SECM_GetData( addr, len, buf );
  printf("data=\n");
  for(int g=0;g<len;g++)
  printf("%c ",buf[g]);
  printf("\n");
  return(apiReady);
}

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
    UINT8   authStatus;
    UINT8   msg_unauth[] = {"Unauthenticated device!"};


	// dual control password
	PED_SetSensitiveServiceTime(TRUE);
	// if(!PED_DualPasswordControl())   //Remarked by Tammy
    if(!PED_DualPasswordControl(1)) //Modified by Tammy
	{
		PED_SetSensitiveServiceTime(FALSE);
		return FALSE;
	}

    OS_SECM_GetData(ADDR_DEVICE_AUTH_STATUS, 1, &authStatus);
    if(authStatus == 0)
    {
        PED_SetSensitiveServiceTime(FALSE);

        //This device has not been authenticated yet
        LIB_LCD_Cls();
        LIB_LCD_Puts(2, 8, FONT0, sizeof(msg_unauth) - 1, (UINT8 *)msg_unauth);
        LIB_WaitTime(300);
        return FALSE;
    }

	start = 0;
	while(1)
	{
		LIB_LCD_Cls();
		LIB_LCD_Puts(0, 0, FONT0, sizeof(os_msg_SELECT), (UINT8 *)os_msg_SELECT);

		buffer[0] = 1;	// starting row number
		buffer[1] = 16;	// max lcd row cnt
		buffer[2] = 15;	// list items
		buffer[3] = 25;	// item length
		buffer[4] = 0;	// offset of LEN field in item
		buffer[5] = FONT0;
		result = LIB_ListBox(start, &buffer[0], (UINT8 *)&os_list_ADMIN_MODE[0], 60); // wait for selection (T.O.=60sec)

		switch(result)
		{
			case 0xff: // aborted
				PED_SetSensitiveServiceTime(FALSE);
				return TRUE;

			case 0x00: // VERIFY KEY STATUS
				status = PED_VerifyKeyStatus();
				break;

			case 0x01:	// CHANGE KEY MODE
				status = PED_ADMIN_ChangeKeyMode();
				break;

			case 0x02:	// SETUP 128-bit TDES KPK
                status = PED_ADMIN_SetTDESKeyBlockProtectionKey(TDES_128);
				break;

			case 0x03:	// SETUP 192-bit TDES KPK
                status  = PED_ADMIN_SetTDESKeyBlockProtectionKey(TDES_192);
				break;

            case 0x04:  // SETUP 128-bit AES KPK
                status = PED_ADMIN_SetAESKeyBlockProtectionKey(AES_128);
                break;

            case 0x05:  // SETUP 192-bit AES KPK
                status = PED_ADMIN_SetAESKeyBlockProtectionKey(AES_192);
                break;

            case 0x06:  // SETUP 256-bit AES KPK
                status = PED_ADMIN_SetAESKeyBlockProtectionKey(AES_256);
                break;
                
			case 0x07:	// PEK MASTER/SESSION KEY
                status = PED_ADMIN_LoadPEKMasterSessionKey();
				break;

			case 0x08:	// ACC_DEK MASTER/SESSION KEY
				status = PED_ADMIN_LoadAccDEKMasterSessionKey();
				break;

			case 0x09:	// FPE MASTER/SESSION KEY
				status = PED_ADMIN_LoadFPEMasterSessionKey();
				break;
            
            case 0x0A:	// AES DUKPT KEY
				status = PED_ADMIN_LoadAesDukptKey();
				break;

			case 0x0B:	// CAPK
				status = PED_ADMIN_LoadCAPK();
				break;

			case 0x0C:	// DELETE PED KEY
				status = PED_ADMIN_DeletePedKey();
				break;

			case 0x0D:	// SETUP SRED
				status = PED_ADMIN_SetupSRED();
				break;

			case 0x0E:	// RESET SECM
				status = PED_ADMIN_ResetSecureMemory();
				PED_SetSensitiveServiceTime(FALSE);
				return status;

            // case 0x0F:  // DELETE U-BOOT AND KERNEL
            //     system( "flash_erase /dev/mtd0 0 0" );	// erase U-Boot
            //     system( "flash_erase /dev/mtd1 0 0" );	// erase Kernel
            //     LIB_LCD_Cls();
            //     LIB_LCD_Puts(2, 6, FONT1, sizeof("SYSTEM HAS BEEN REMOVED!!!") - 1, (UINT8 *)"SYSTEM HAS BEEN REMOVED!!!");
            //     for(;;);
            //     break;
				
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

UCHAR DSS_function_LANset()
{
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
  LIB_LCD_Cls();
  LIB_LCD_Puts( 0, 0, FONT1, sizeof(os_msg_ETHERNET), (UINT8 *)os_msg_ETHERNET );
	LIB_LCD_Puts( 1, 0, FONT1, sizeof(os_msg_1SETUP_2PING), (UINT8 *)os_msg_1SETUP_2PING );
  do{
	  select = LIB_WaitKey();
	  if( select == 'x' )
	    return( FALSE );
	  }while( (select != '1') && (select != '2') && (select != 'y') );

	if( select == 'y' )
	  select = '2';
	
	if( select == '1' )	// setup IP config
	{
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
      return apiReady;
  }//set TCP/IP
  else
  {
    // PING...
	LIB_LCD_Puts( 1, 0, FONT1+attrCLEARWRITE, sizeof(os_msg_PING), (UINT8 *)os_msg_PING );
	// enter IP address
	do{
	  if( LIB_GetNumKey( 0, NUM_TYPE_DIGIT+NUM_TYPE_LEFT_JUSTIFY, '_', FONT1, 3, 15, ip ) != TRUE )
	    return( FALSE );
	  }while( ip[0] == 0 );
	
	while(1)
	  {
	  LIB_LCD_ClearRow( 4, 1, FONT1 );
	  
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
	    
	    LIB_LCD_Puts( 4, 0, FONT1, len, buffer );
	    
	    if( LIB_WaitTimeAndKey( 300 ) != 255 )	// repeat every 3 seconds till any key stroke
	      break;
	    }
	  else
	    {
	    LIB_LCD_Puts( 4, 0, FONT1, sizeof(os_msg_ERR_NO_RESPONSE), (UINT8 *)os_msg_ERR_NO_RESPONSE );
	    if( LIB_WaitTimeAndKey( 300 ) != 255 )	// repeat every 3 seconds till any key stroke
	      break;
	    }
	  }
  }
}

#if 0
UINT32	DIAG_FUNC_TestGPRS( void )
{
UINT32	status = FALSE;
UINT32	result;
UINT32	fOP;
UINT32	len;
UINT8	key;
UINT8	key2;
UINT8	quality;
UINT8	buf[32];
UINT8	simPIN[16];
API_LTE_PROFILE profile[1];

UINT8	msg_POWER_ON[]		= {"POWER ON..."};
UINT8	msg_POWER_OFF[]		= {"POWER OFF..."};
UINT8	msg_1SIM[]		= {"1-SIM"};
UINT8	msg_2SIGNAL[]		= {"2-SIGNAL"};
UINT8	msg_3ONLINE[]		= {"3-ONLINE"};
UINT8	msg_PIN_READY[]		= {"OK: PIN READY"};
UINT8	msg_PIN_REQUIRED[]	= {"OK: PIN REQUIRED"};
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
UINT8	os_msg_4G_LTE[]	= {"4G LTE test"};


	LIB_LCD_Puts( 0, 0, FONT0, sizeof(os_msg_4G_LTE), (UINT8 *)os_msg_4G_LTE );
	
	LIB_LCD_Puts( 1, 0, FONT0, strlen(msg_1_SIM1_2_SIM2), (UINT8 *)msg_1_SIM1_2_SIM2 );
	
	LIB_LCD_Puts( 2, 0, FONT0, strlen(msg_SIM1), (UINT8 *)msg_SIM1 );
	  
	do{
	  key2 = LIB_WaitKey();
	  } while( (key2 != '1') && (key2 != '2') && (key2 != 'y') && (key2 != 'x') );

	if( key2 == 'x' )
	  return( TRUE );
	  
	if( key2 == 'y' )
	  key2 ='1';
	
	if( key2 == '1' )
	  LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(msg_SIM1), (UINT8 *)msg_SIM1 );
	  
	LIB_LCD_ClearRow( 1, 3, FONT0 );

	LIB_LCD_Puts( 1, 0, FONT0, strlen(msg_POWER_ON)-3, (UINT8 *)msg_POWER_ON );
	
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
	    LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, sizeof(msg_3ONLINE), (UINT8 *)msg_3ONLINE );
	  
	    key = LIB_WaitKey();
	    switch( key )
	        {
	        case '1':	// TEST SIM
	             
	             LIB_LCD_Cls();
	             LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_1SIM)-2, (UINT8 *)msg_1SIM+2 );
            
	             LIB_LCD_Puts( 3, 0, FONT0, sizeof(os_msg_PROCESSING), (UINT8 *)os_msg_PROCESSING );
	             
	             status = api_lte_checkPIN();
	             if( status == apiFailed )
	               LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, sizeof(msg_ERROR), (UINT8 *)msg_ERROR );
	             else
	               {
	               if( status == apiReady )
	                 LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, sizeof(msg_PIN_READY), (UINT8 *)msg_PIN_READY );
	               else
	                 LIB_LCD_Puts( 3, 0, FONT0+attrCLEARWRITE, sizeof(msg_PIN_REQUIRED), (UINT8 *)msg_PIN_REQUIRED );
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
	               
	               memset( (UINT8 *)profile, 0x00, sizeof(API_LTE_PROFILE) );
	               profile->PIN_len = simPIN[0];
	               memmove( profile->PIN, &simPIN[1], simPIN[0] );
	               profile->ConProfile[0].ConProfileID = 0xFF;
	               profile->SrvProfile[0].SrvProfileID = 0xFF;
	               
	               result = api_lte_setup( profile[0] );
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
				
	                      	if( api_lte_checkOP( buf, &key ) == apiReady )
	                      	  {
	                      	  fOP = TRUE;
	                      	  
	                      	  if( buf[0] > 21 )
	                      	    buf[0] = 21;
	                      	  LIB_LCD_Puts( 4, 0, FONT0+attrCLEARWRITE, buf[0], &buf[1] );	// OP name
	                      	  
	                      	  switch( key )
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
//	                      	}
	                      }
	                    else
	                      LIB_LCD_Puts( 2, 10, FONT0, strlen("SCANNING..."), (UINT8 *)"SCANNING..." );
	                      
	                    if( LIB_WaitTimeAndKey( 200 ) != 255 )
	                      break;
	                    LIB_BUZ_Beep1();
	                    }
	                  else
	                    {
	                    LIB_LCD_Puts( 2, 0, FONT0+attrCLEARWRITE, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR );
	                    break;
	                    }
	                  }
	                  
	             break;
	             
	        case '3':	// TEST ONLINE
	             break;
	        }
	    
	    if( key == '1' )
	      LIB_WaitKey();
	      
	    }while( key != 'x' );
	    
	  status = TRUE;
	  }
	
	if( status )
	  {
	  LIB_LCD_ClearRow( 1, 3, FONT0 );
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
