//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : API_PEDS.C                                                 **
//**  MODULE   : api_peds_OpenPinPad()			                    **
//**		 api_peds_ClosePinPad()					    **
//**		 api_peds_GetStatus()					    **
//**		 api_peds_SetMasterKey()				    **
//**		 api_peds_SetSessionKey()				    **
//**		 api_peds_ResetPinPad()					    **
//**		 api_peds_GetPin()					    **
//**		 api_peds_MSKEY_GenPinBlock()				    **
//**		 api_peds_MSKEY_GenMAC					    **
//**		 api_peds_LoadTerminalMasterKey()			    **
//**		 api_peds_ShowBalance()					    **
//**		 api_peds_SelectMasterKey()				    **
//**		 api_peds_MSKEY_TDES()					    **
//**									    **
//**  FUNCTION : API::PEDS (SYMLINK internal used only)			    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/08/17                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2009-2011 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------

#include <string.h>
//#include "bsp_mem.h"
#include "POSAPI.h"
#include "PEDSAPI.h"
#include "OS_FLASH.h"
#include "OS_SECM.h"	// 2018-05-02, for SES1 model with ECC memory mapping

#define	PEDS_FLASH_BACKUP_SIZE		128*1024	// 128KB for AS350 (64KB for AS320)

UCHAR   g_dhn_ped;
UCHAR   g_ped_dev;
UCHAR   g_ped_tout;
UCHAR   g_ped_state;
UCHAR   g_ped_dhn_tim;
UCHAR   g_ped_dhn_kbd;
UINT    g_ped_tick1;
UINT    g_ped_tick2;
UCHAR	g_ped_mkey_index = 0;

UCHAR	*g_pSwap;

extern	void		UTIL_DumpHexData( UCHAR mode, UCHAR row, UINT length, UCHAR *data );
extern	BSP_HANDLE 	BSP_Malloc_EX( UINT32 Size );		// alien format of BSP_Malloc()
extern	void 		BSP_Free_EX( BSP_HANDLE pData );	// alien format of BSP_Free()


// ---------------------------------------------------------------------------
// FUNCTION: To read CA Public Key data element from SRAM page memory.
// INPUT   : address - begin address to read.
//           length  - length of the data element.
// OUTPUT  : data    - the data element read.
// RETURN  : apkOK     - matched.
//           apkFailed - not matched.
// ---------------------------------------------------------------------------
UCHAR	PEDS_ReadRamDataKEY( UINT address, UINT length, UCHAR *data )
{
	OS_SECM_GetData( address, length, data );
	return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: To write CA Public Key data element to SRAM page memory.
// INPUT   : address - begin address to write.
//           length  - length of the data element.
//           data    - the data element.
// OUTPUT  : none.
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
UCHAR	PEDS_WriteRamDataKEY( UINT address, UINT length, UCHAR *data )
{
	OS_SECM_PutData( address, length, data );
	return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: To clear CA Public Key data element to specified pattern.
// INPUT   : address - begin address to write.
//           length  - length of the data element.
//           data    - the data element.
// OUTPUT  : none.
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
UCHAR	PEDS_ClearRamDataKEY( UINT address, UINT length, UCHAR pattern )
{
	OS_SECM_ClearData( address, length, pattern );
	return( apiOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: Open keyboard - numeric keys, cancel, clear, backspace, OK.
// INPUT   : none.
// OUTPUT  : g_dhn_kbd
// RETURN  : none.
// ---------------------------------------------------------------------------
void PEDS_OpenKeyNum()
{
UCHAR buf[5];

      buf[0]=0x01c;
      buf[1]=0x03c;
      buf[2]=0x03c;
      buf[3]=0x02c;
      buf[4]=0x000;
      g_ped_dhn_kbd = api_kbd_open( 0, buf );
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait for N*10ms.
// INPUT   : ten_ms - unit in step of ten mini-second. (1 sec = 100 unit)
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void PEDS_WaitTime( UINT tenms )
{
UCHAR dhn_tim;
UINT  tick1, tick2;

      dhn_tim = api_tim_open(1); // 1 tick unit = 10 ms
      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );

      do{
        api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );
        } while( tick2 - tick1 < tenms );

      api_tim_close( dhn_tim );
}

// ---------------------------------------------------------------------------
// FUNCTION: Send MSG to PINPAD.
//           Format: SOH LEN(1) DATA(n) LRC
// INPUT   : dhn.
//           len  - length of data.
//           data - data to be sent. ( CMD(1)+Data(n) )
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR PEDS_SendData( UCHAR dhn, UINT len, UCHAR *data )
{
UCHAR sbuf[510];


      sbuf[0] = len & 0x00FF;
      sbuf[1] = (len & 0xFF00) >> 8;
      memmove( &sbuf[2], data, len );
      
      return( api_aux_txstring( dhn, sbuf ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: Receive TEXT from CP.
// INPUT   : dhn.
//           tout - timeout in seconds.
// OUTPUT  : data - data received (LL-V).
// RETURN  : apiOK
//           apiFailed (timeout)
// ---------------------------------------------------------------------------
UCHAR PEDS_ReceiveData( UCHAR dhn, UCHAR tout, UCHAR *data )
{
UINT  i;
UCHAR lrc;
UCHAR dbuf[500];
UCHAR result;
UCHAR tim_dhn;
UINT  tick1, tick2;
UINT  timeout;
UINT  len;

      result = apiFailed;

      // check response
      tim_dhn=api_tim_open( 10 ); // 100ms
      api_tim_gettick(tim_dhn, (UCHAR *)&tick1);

      if( tout != 0 )
        {
        if( tout != 0xFF )
          tout++; // one more second than external PINPAD timeout
        timeout = tout * 10;
        }
      else
        timeout = 3; // 300ms for special case

      do
       {
       if( api_aux_rxready( dhn, dbuf ) == apiReady )
         {
         if( api_aux_rxstring( dhn, dbuf ) == apiOK )
           {
           // Verify package: PACKLEN(2) SOH LEN(1) Data(n) LRC
           len = dbuf[0] + dbuf[1]*256;
           if( len > sizeof(dbuf) )
             break;

           if( len != 0 )
             {
             memmove( data, dbuf, len+2 );
             result = apiOK; // done
             }
           }

         break;
         }

       api_tim_gettick(tim_dhn, (UCHAR *)&tick2);
       } while( (tick2 - tick1) < timeout ); // timout?

      api_tim_close( tim_dhn );

      return( result );
}

// ---------------------------------------------------------------------------
void  PEDS_InitDefaultKEK( UCHAR *kek )
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

// ---------------------------------------------------------------------------
// FUNCTION: Decrypt master key by using default KEK.
// INPUT   : emkey - encrypted master key. (V, 16 bytes)
// OUTPUT  : mkey  - decrypted master key. (V, 16 bytes)
// RETURN  : TRUE  - OK.
//           FALSE - failed.
// ---------------------------------------------------------------------------
void  PED_DecryptMKEY( UCHAR *emkey, UCHAR *mkey )
{
UCHAR kek[16];

      // init default KEK
      PEDS_InitDefaultKEK( kek );

      // Decrypt default MKEY by default Key Encryption Key (KEK)
      PED_TripleDES2( kek, 16, emkey, mkey );
}

// ---------------------------------------------------------------------------
// FUNCTION: Encrypt master key by using default KEK.
// INPUT   : mkey  - decrypted master key. (V, 16 bytes)
//	     length- length of key. (16 or 24)
// OUTPUT  : emkey - encrypted master key. (V, 16 bytes)
// RETURN  : TRUE  - OK.
//           FALSE - failed.
// ---------------------------------------------------------------------------
void  PED_EncryptMKEY( UCHAR length, UCHAR *mkey, UCHAR *emkey )
{
UCHAR kek[PED_MSKEY_SLOT_LEN];

      // init default KEK
      PEDS_InitDefaultKEK( kek );

      // Encrypt default MKEY by default Key Encryption Key (KEK)
      PED_TripleDES( kek, length, mkey, emkey );
}

// ---------------------------------------------------------------------------
// FUNCTION: Encrypt master key by using default KEK. (current TMK)
// INPUT   : mkey  - decrypted master key. (V, 16 bytes)
// OUTPUT  : emkey - encrypted master key. (V, 16 bytes)
// RETURN  : none.
// ---------------------------------------------------------------------------
//void  PED_EncryptMKEY_FISC( UCHAR *mkey, UCHAR *emkey )
//{
//UCHAR kek[PED_MSKEY_SLOT_LEN];
//
//      // default KEK = current TMK
//      apk_ReadRamDataKEY( ADDR_PED_MKEY_01+(g_ped_mkey_index*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN, kek );
//
//      // Encrypt default MKEY by default Key Encryption Key (KEK)
//      PED_TripleDES( kek, 16, mkey, emkey );
//}

// ---------------------------------------------------------------------------
// FUNCTION: Decrypt master key by using default KEK. (current TMK)
// INPUT   : emkey  - encrypted master key. (V, 16 bytes)
// OUTPUT  : mkey   - decrypted master key. (V, 16 bytes)
// RETURN  : none.
// ---------------------------------------------------------------------------
//void  PED_DecryptMKEY_FISC( UCHAR *emkey, UCHAR *mkey )
//{
//UCHAR kek[PED_MSKEY_SLOT_LEN];
//
//      // default KEK = current TMK
//      apk_ReadRamDataKEY( ADDR_PED_MKEY_01+(g_ped_mkey_index*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN, kek );
//
//      // Encrypt default MKEY by default Key Encryption Key (KEK)
//      PED_TripleDES2( kek, 16, emkey, mkey );
//}

// ------------------------------------------------------------
// FUNCTION: To enable PIN pad device.
// INPUT   : dev   - PIN pad device, 0 = internal, 1 = external.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// NOTE    : This function must be called once before requesting
//           any other PIN pad functions.
//
//           EDC                PINPAD
//           ===                ======
//           CMD_OPEN  ->
//                              <- CMD_OPEN + RC_XXX
// ------------------------------------------------------------
UCHAR PEDS_OpenPinPad( void )
{
UCHAR	i;
UCHAR	result;
UCHAR	buf[PED_MSKEY_SLOT_LEN2];
UCHAR	mkey[PED_MSKEY_SLOT_LEN2];
ULONG	FlashAddr = F_ADDR_PED_ETMK_01;


	// 2012-11-28, internal multiple TMKs
	// If current TMSs are not available, retrieve them from FLASH
        for( i=0; i<MAX_MKEY_CNT; i++ )
           {
           memset( mkey, 0x00, sizeof(mkey) );
           result = PEDS_ReadRamDataKEY( ADDR_PED_MKEY_01+(i*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, mkey ); 
           if( result != apiOK )
             goto OPEN_EXIT;
             
           if( mkey[0] != PED_MSKEY_LEN ) // MKEY available? (in SRAM)
             {          
             // retrieve ETMK
//           result = apk_ReadFlashData( ADDR_PED_MKEY, PED_MSKEY_SLOT_LEN, buf );
             FLASH_ReadData( (void *)FlashAddr+(i*PED_MSKEY_SLOT_LEN2), buf, PED_MSKEY_SLOT_LEN2 );
             if( buf[0] == PED_MSKEY_LEN ) // MKEY available? (in FLASH)
               {
               // Restore default MKEY by default Key Encryption Key (KEK)
               mkey[0] = buf[0];
               PED_DecryptMKEY( &buf[1], &mkey[1] );
               
               // Restore KCV
               memmove( &mkey[PED_MSKEY_SLOT_LEN], &buf[PED_MSKEY_SLOT_LEN], 3 );
             
               // save it to SRAM key slot (L-V-KCV)
               result = PEDS_WriteRamDataKEY( ADDR_PED_MKEY_01+(i*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, mkey );
               }
             }
	   }
	   
        g_ped_tout = 0;

        if( g_ped_dhn_tim != 0 )
          {
          api_tim_close( g_ped_dhn_tim );
          g_ped_dhn_tim = 0;
          }
          
OPEN_EXIT:

	memset( mkey, 0x00, sizeof(mkey) );
	memset( buf, 0x00, sizeof(buf) );
	
        return( result );
}

// ------------------------------------------------------------
void api_peds_InitGlobal( void )
{
      g_ped_dev = 0xFF;
      g_ped_tout = 0;
      g_ped_state = PED_STATE_END;
      g_ped_dhn_tim = 0;
      g_ped_mkey_index = 0;
}

// ------------------------------------------------------------
// FUNCTION: To enable PIN pad device.
// INPUT   : dev   - PIN pad device, 0 = internal, 1 = external.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// NOTE    : This function must be called once before requesting
//           any other PIN pad functions.
//
//           EDC                PINPAD
//           ===                ======
//           CMD_OPEN  ->
//                              <- CMD_OPEN + RC_XXX
// ------------------------------------------------------------
UCHAR api_peds_OpenPinPad( UCHAR dev )
{
UCHAR mode;
UCHAR result;
UCHAR buf[16];
UCHAR mkey[17];
UCHAR port;

API_AUX	pAux;


      g_ped_dev = dev;

      if( dev == PED_DEV_INT )  // internal PINPAD?
        return( PEDS_OpenPinPad() );

      if( dev == PED_DEV_EXT )  // external PINPAD?
        {
        mode = auxDLL;	// single LEN
//	mode = auxSOH;	// double LEN, 2011-01-06 for CUP
        port = COM1;
        }
      else
        {
        return( apiFailed );
        }

      // --- external PINPAD ---
      result = apiFailed;

      // --- enable hardware port ---
      pAux.Mode = mode;
      pAux.Baud = COM_9600 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
      pAux.Tob = 10;	// 100ms
      pAux.Tor = 100;	// 1 sec
      pAux.Acks = 0;	// no ack to avoid PINPAD resending
      pAux.Resend = 0;	// no retry
      g_dhn_ped = api_aux_open( port, pAux );

      if( (g_dhn_ped != apiOutOfService) && (g_dhn_ped != apiOutOfLink) )
        {
        PEDS_ReceiveData( g_dhn_ped, 0, buf ); // flush garbage

        // --- send command to PINPAD ---
        buf[0] = PED_CMD_OPEN;
        if( PEDS_SendData( g_dhn_ped, 1, buf ) == apiOK )
          {
          if( PEDS_ReceiveData( g_dhn_ped, 1, buf ) == apiOK )
//	  if( PEDS_ReceiveData( g_dhn_ped, 0, buf ) == apiOK )	// PATCH: 2010-06-14
            {
            if( (buf[0] == 2) && (buf[1] == 0) && (buf[2] == PED_CMD_OPEN) && (buf[3] == PED_RC_OK) )
              {
//            PEDS_WaitTime( 20 );	// delay 200ms
              result = apiOK;
              }
            }
          }
        }

      return( result );
}

// ------------------------------------------------------------
// FUNCTION: To disable PIN pad device.
// INPUT   : dev   - PIN pad device, 0 = internal, 1 = external.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// NOTE    : This function must be called after end of PIN pad
//           processes.
//
//           EDC                PINPAD
//           ===                ======
//           CMD_CLOSE ->
//                              <- CMD_CLOSE+ RC_XXX
// ------------------------------------------------------------
UCHAR api_peds_ClosePinPad( UCHAR dev )
{
UCHAR buf[10];
UCHAR result;

      // erase TMK from SRAM key slot
//    apk_ClearRamDataKEY( ADDR_PED_MKEY, PED_MSKEY_SLOT_LEN, 0x00 );

      // check device? (internal or external PINPAD)
      if( dev != g_ped_dev )
        return( apiFailed );
      else
        {
        if( dev == PED_DEV_INT )
          {
          g_ped_dev = 0xFF;
          g_ped_tout = 0;

          if( g_ped_dhn_tim != 0 )
            {
            api_tim_close( g_ped_dhn_tim );
            g_ped_dhn_tim = 0;
            }

          return( apiOK );
          }
        }

      // --- send command to external PINPAD ---
      result = apiFailed;

      buf[0] = PED_CMD_CLOSE;
      if( PEDS_SendData( g_dhn_ped, 1, buf ) == apiOK )
        {
//      if( PEDS_ReceiveData( g_dhn_ped, 1, buf ) == apiOK )
	if( PEDS_ReceiveData( g_dhn_ped, 0, buf ) == apiOK )	// PATCH: 2010-06-14
          {
          if( (buf[0] == 2) && (buf[1] == 0) && (buf[2] == PED_CMD_CLOSE) && (buf[3] == PED_RC_OK) )
            result = apiOK;
          }
        }

      api_aux_close( g_dhn_ped ); // PATCH: 2008/07/29
      PEDS_WaitTime( 20 );         // wait 200ms for restoring comm btw TU & CP

      return( result );
}

// ------------------------------------------------------------
// FUNCTION: To reset PIN pad device.
// INPUT   : tout  - timeout in seconds used to wait for PIN entry.
//           type  - transaction type. (RFU)
//           amt   - amount to be confirmed on display. (external)
//                   format: LEN(1) + ASCII(n)
//                           LEN = 0: no confirmation.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// NOTE    : This function must be called before requesting PIN
//           entry (api_ped_GetPin()).
//
//           EDC                PINPAD
//           ===                ======
//           CMD_RESET ->
//                              <- CMD_RESET + RC_XXX
// ------------------------------------------------------------
UCHAR api_peds_ResetPinPad( UCHAR tout, UCHAR type, UCHAR *amt )
{
UCHAR result;
UCHAR buf[32];
UINT  len;

      g_ped_tout = tout;

      // --- internal PINPAD ---
      if( g_ped_dev == PED_DEV_INT )
        {
        PEDS_OpenKeyNum();

        g_ped_state = PED_STATE_START;
        PEDS_ClearRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );
//	OS_SECM_ClearData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );

        return( apiOK );
        }

      // --- external PINPAD ---
      result = apiFailed;

      len = *amt++;

      buf[0] = PED_CMD_RESET;
      buf[1] = tout;
      buf[2] = type;
      buf[3] = len;
      if( len != 0 )
        memmove( &buf[4], amt, len );
      len += 4;

      if( PEDS_SendData( g_dhn_ped, len, buf ) == apiOK )
        {
        if( PEDS_ReceiveData( g_dhn_ped, 1, buf ) == apiOK )
          {
          if( (buf[0] == 2) && (buf[1] == 0) && (buf[2] == PED_CMD_RESET) && (buf[3] == PED_RC_OK) )
            result = apiOK;
          }
        }

      return( result );
}

// ------------------------------------------------------------
// FUNCTION: To reset PIN pad device.
// INPUT   : tout  - timeout in seconds used to show balance.
//           amt   - amount to be confirmed on display. (external)
//                   format: LEN(1) + ASCII(n)
//                           LEN = 0: no confirmation.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// NOTE    : This function must be called before requesting PIN
//           entry (api_ped_GetPin()).
//
//           EDC                PINPAD
//           ===                ======
//           CMD_RESET ->
//                              <- CMD_RESET + RC_XXX
// ------------------------------------------------------------
#if	0

UCHAR api_peds_ShowBalance( UCHAR tout, UCHAR *amt )
{
UCHAR result;
UCHAR buf[32];
UINT  len;

      g_ped_tout = tout;

      // --- internal PINPAD ---
      if( g_ped_dev == PED_DEV_INT )
        {
        return( apiFailed );	// not support internal PINPAD
        }

      // --- external PINPAD ---
      result = apiFailed;

      len = *amt++;

      buf[0] = PED_CMD_SHOW_BALANCE;
      buf[1] = tout;
      buf[2] = len;
      if( len != 0 )
        memmove( &buf[3], amt, len );
      len += 3;

      if( PEDS_SendData( g_dhn_ped, len, buf ) == apiOK )
        {
        if( PEDS_ReceiveData( g_dhn_ped, 1, buf ) == apiOK )
          {
          if( (buf[0] == 2) && (buf[1] == 0) && (buf[2] == PED_CMD_SHOW_BALANCE) && (buf[3] == PED_RC_OK) )
            result = apiOK;
          }
        }

      return( result );
}

#endif

// ---------------------------------------------------------------------------
// FUNCTION: Get key status.
// INPUT   : g_dhn_kbd
// OUTPUT  : none.
// RETURN  : apiReady
//           apiNotReady
// ---------------------------------------------------------------------------
UCHAR PEDS_GetKeyStatus(void)
{
UCHAR buf[1];

      return( api_kbd_status( g_ped_dhn_kbd, buf) );
}

// ---------------------------------------------------------------------------
// FUNCTION: Wait key-stroke.
// INPUT   : g_dhn_kbd
// OUTPUT  : none.
// RETURN  : the key code data.
// ---------------------------------------------------------------------------
UCHAR PEDS_WaitKey(void)
{
UCHAR buf[1];

      api_kbd_getchar( g_ped_dhn_kbd, buf );
//    api_buz_sound( g_ped_dhn_buz ); // 1 short beep
      return(buf[0]);
}

// ---------------------------------------------------------------------------
// FUNCTION: To request PIN entry from PIN pad device.
// INPUT   : none.
// OUTPUT  : keyin - value of the key-in digit.
//                   For INTERNAL PIN pad.
//                   Only the following characters will be
//                   returned to AP.
//                   (1) '*' for '0' to '9' keys.
//                   (2) CLEAR, CANCEL, BACK-SPACE or ENTER key.
//                   (3) NULL if only ENTER key is pressed.
//                   (4) 0xFF if PIN length less than 4 or over 12 digits.
//                   For EXTERNAL PIN pad.
//                   Only CANCEL or ENTER key will be
//                   returned to AP.
// RETURN  : apiOK         (key-in digit available)
//           apiFailed     (timeout or invalid PIN length)
// NOTE    : The "api_ped_ResetPinPad()" shall be called prior to
//           invoking this function.
// REF     : g_ped_tout
//           g_ped_state
// ---------------------------------------------------------------------------
UCHAR PEDS_GetPin( UCHAR *keyin )
{
UCHAR kek[16];
UCHAR pin[17];
UCHAR temp[17];
UCHAR key;
UCHAR pinlen;


      // init default KEK
      PEDS_InitDefaultKEK( kek );

      switch( g_ped_state )
            {
            case PED_STATE_START:

                 PEDS_ClearRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );

                 if( g_ped_dhn_tim != 0 )
                   {
                   api_tim_close( g_ped_dhn_tim );
                   g_ped_dhn_tim = 0;
                   }

                 g_ped_dhn_tim = api_tim_open(100); // time tick = 1sec
                 if( g_ped_dhn_tim != apiOutOfService )
                   api_tim_gettick( g_ped_dhn_tim, (UCHAR *)&g_ped_tick1 );
                 else
                   {
                   api_tim_close( g_ped_dhn_tim );
                   g_ped_dhn_tim = 0;

//                 memset( g_ped_temp, 0x00, sizeof( g_ped_temp ) );

                   g_ped_state = PED_STATE_END;
                   return( apiFailed );
                   }

                 // go throuth next state for waiting keyin

            case PED_STATE_WAIT_KEY:

                 // wait key & check time out
                 do{
                   api_tim_gettick( g_ped_dhn_tim, (UCHAR *)&g_ped_tick2 );
                   if( (g_ped_tick2 - g_ped_tick1) >= g_ped_tout )
                     {
                     api_tim_close( g_ped_dhn_tim );
                     g_ped_dhn_tim = 0;

                     PEDS_ClearRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );

                     g_ped_state = PED_STATE_END;
                     return( apiFailed );
                     }
                   } while( PEDS_GetKeyStatus() != apiReady );

                 // key stroked
                 key = PEDS_WaitKey();

                 if( (key >= 0x30) && (key <= 0x39) ) // '0'..'9'
                   {
                   PEDS_ReadRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, temp );
                   memset( pin, 0x00, sizeof(pin) );

                   pinlen = temp[0];
                   if( pinlen < 12 )
                     {
                     if( g_ped_state == PED_STATE_WAIT_KEY )
                       {
                       // decrypt PIN by using default KEK (excluding PINLEN)
                       if( pinlen )
                //       PED_TripleDES2( kek, 16, (UCHAR *)&temp[1], &pin[1] ); // encrypted format
                         memmove( &pin[1], &temp[1], 16 ); // plaintext format for speed-up
                       }

                     pinlen++;
                     temp[0] = pinlen;
                     pin[0] = pinlen;
                     pin[pinlen] = key;

                //   disp_hex_byte( 5, 0, pin[0] );
                //   disp_hex_byte( 6, 0, pin[1] );
                //   disp_hex_byte( 6, 3, pin[2] );
                //   disp_hex_byte( 6, 6, pin[3] );
                //   disp_hex_byte( 6, 9, pin[4] );
                //   disp_hex_byte( 6, 12, pin[5] );
                //   disp_hex_byte( 6, 15, pin[6] );
                //   disp_hex_byte( 6, 18, pin[7] );
                //   disp_hex_byte( 7, 0, pin[8] );
                //   disp_hex_byte( 7, 3, pin[9] );
                //   disp_hex_byte( 7, 6, pin[10] );
                //   disp_hex_byte( 7, 9, pin[11] );
                //   disp_hex_byte( 7, 12, pin[12] );

                     // encrypt PIN by using default KEK (excluding PINLEN)
                //   PED_TripleDES( kek, 16, &pin[1], (UCHAR *)&temp[1] ); // encrypted format
                     memmove( &temp[1], &pin[1], 16 ); // plaintext format for speed-up
                     PEDS_WriteRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, temp );

                     g_ped_state = PED_STATE_WAIT_KEY;
                     *keyin = 0x2A; // asterisk
                     return( apiOK );
                     }
                   else
                     {
                     // PIN length over 12 digits
                     g_ped_state = PED_STATE_WAIT_KEY;
                     *keyin = 0xFF;
                     return( apiOK );

                     // OLD
////                 memset( g_ped_temp, 0x00, sizeof( g_ped_temp ) );
//                   apk_ClearRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );
//
//                   g_ped_state = PED_STATE_END;
//                   return( apiFailed ); // invalid length (>12)
                     }
                   }

                 if( key == 'n' ) // CLEAR
                   {
//                 g_ped_temp[0] = 0;
                   PEDS_ClearRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );

                   *keyin = key;
                   return( apiOK );
                   }

                 if( key == 'x' ) // CANCEL
                   {
                   api_tim_close( g_ped_dhn_tim );
                   g_ped_dhn_tim = 0;

//                 memset( g_ped_temp, 0x00, sizeof( g_ped_temp ) );
                   PEDS_ClearRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );

                   *keyin = key;
                   return( apiOK );
                   }

                 if( key == '#' ) // BACK-SPACE
                   {
//                 if( g_ped_temp[0] != 0 )
//                   g_ped_temp[0] -= 1;

                   PEDS_ReadRamDataKEY( ADDR_PED_PIN, 1, temp );
                   if( temp[0] != 0 )
                     {
                     temp[0] -= 1;
                     PEDS_WriteRamDataKEY( ADDR_PED_PIN, 1, temp );
                     }

                   *keyin = key;
                   return( apiOK );
                   }

                 if( key == 'y' ) // ENTER
                   {
                   PEDS_ReadRamDataKEY( ADDR_PED_PIN, 1, temp );

//                 disp_hex_byte( 5, 10, temp[0] );
//                 wait_1_key(kbd_dhn);

                   if( temp[0] == 0 ) // bypass PIN
                     {
                     api_tim_close( g_ped_dhn_tim );
                     g_ped_dhn_tim = 0;

                     g_ped_state = PED_STATE_END;
                     *keyin = 0x00;   // NULL
                     return( apiOK ); // done
                     }

//                 if( g_ped_temp[0] < 4 )
                   if( temp[0] < 4 )	// ISO
//		   if( temp[0] < 6 )	// FISC
                     {
                     // PIN length less than 4 digits
                     g_ped_state = PED_STATE_WAIT_KEY;
                     *keyin = 0xFF;
                     return( apiOK );

                     // OLD
////                 memset( g_ped_temp, 0x00, sizeof( g_ped_temp ) );
//                   apk_ClearRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );
//
//                   g_ped_state = PED_STATE_END;
//                   *keyin = key;
//                   return( apiFailed ); // invalid length (<4)
                     }
                   else
                     {
                     // encrypt plaintext PIN by using default KEK (excluding PINLEN) at final step
                     PEDS_ReadRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, pin );
                     PED_TripleDES( kek, 16, &pin[1], (UCHAR *)&temp[1] );
                     temp[0] = pin[0];
                     PEDS_WriteRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, temp );

                     api_tim_close( g_ped_dhn_tim );
                     g_ped_dhn_tim = 0;

                     g_ped_state = PED_STATE_PIN_READY; // ready to generate PIN block
                     *keyin = key;

                     return( apiOK ); // done
                     }
                   }

            default: // PED_STATE_END or PED_STATE_PIN_READY

//               memset( g_ped_temp, 0x00, sizeof( g_ped_temp ) );
                 PEDS_ClearRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 );
                 g_ped_state = PED_STATE_END;
                 return( apiFailed );
            }
}

// ------------------------------------------------------------
// FUNCTION: To request PIN entry from PIN pad device.
// INPUT   : none.
// OUTPUT  : keyin - value of the key-in digit.
//                   For INTERNAL PIN pad.
//                   Only the following characters will be
//                   returned to AP.
//                   (1) '*' for '0' to '9' keys.
//                   (2) CLEAR, CANCEL, BACK-SPACE or ENTER key.
//                   (3) NULL if only ENTER key is pressed.
//                   For EXTERNAL PIN pad.
//                   Only CANCEL or ENTER key will be
//                   returned to AP.
// RETURN  : apiOK         (key-in digit available)
//           apiFailed     (timeout or invalid PIN length)
// NOTE    : The "api_ped_ResetPinPad()" shall be called prior to
//           invoking this function.
//
//           EDC                PINPAD
//           ===                ======
//           CMD_GET_PIN ->
//                              <- CMD_GET_PIN + RC_XXX + [KEYIN]
// ------------------------------------------------------------
UCHAR api_peds_GetPin( UCHAR *keyin )
{
UCHAR result;
UCHAR buf[32];

      // --- internal PINPAD ---
      if( g_ped_dev == PED_DEV_INT )
        return( PEDS_GetPin( keyin ) );

      // --- external PINPAD ---
      result = apiFailed;

      buf[0] = PED_CMD_GET_PIN;

      if( PEDS_SendData( g_dhn_ped, 1, buf ) == apiOK )
        {
        if( PEDS_ReceiveData( g_dhn_ped, g_ped_tout, buf ) == apiOK ) // 2008-06-30
          {
          *keyin = buf[4]; // keyin data

          if( (buf[0] == 3) && (buf[1] == 0) && (buf[2] == PED_CMD_GET_PIN) && (buf[3] == PED_RC_OK) )
            result = apiOK;
          }
        }

      return( result );
}

// ------------------------------------------------------------
// FUNCTION: save TMK to Flash memory in encrypted format.
// INPUT   : etmk  - encrypted terminal master key. (L-V-KCV)
//	     index - index number of key slots. (00..09)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ------------------------------------------------------------
UCHAR	PED_PutPinPadTMK( UCHAR *etmk, UCHAR index )
{
ULONG	status;
UCHAR	*pSwap;
ULONG	DataSize = PEDS_FLASH_BACKUP_SIZE;	// 64KB, sizeof FLASH backup area
ULONG	FlashAddr = FLASH_BACKUP_BASE_ADDR;;

	
	status = apiFailed;
	
	pSwap = (UCHAR *)BSP_Malloc_EX( DataSize );	// allocate 64KB for memory swap
	if( pSwap )
	  {
	  // read FLASH data to RAM
//	  FLASH_ReadData( (void *)FlashAddr+index*PED_MSKEY_SLOT_LEN2, pSwap, DataSize );
	  FLASH_ReadData( (void *)FlashAddr, pSwap, DataSize );	// 2018-04-30
	  
	  // update ETMK in RAM
	  memmove( pSwap+((F_ADDR_PED_ETMK_01+(index*PED_MSKEY_SLOT_LEN2))-FLASH_BACKUP_BASE_ADDR), etmk, PED_MSKEY_SLOT_LEN2 );
	  
	  // erase FLASH backup sector
	  if( FLASH_EraseSector( FlashAddr ) )
	    {
	    // write RAM data to FLASH
	    status = FLASH_WriteData( (void *)FlashAddr, pSwap, DataSize );
	    }
	    
	  BSP_Free_EX( pSwap );	// PATCH: 2010-06-14
	  }

	return( (UCHAR)status );
}

// ------------------------------------------------------------
// FUNCTION: save TMK to Flash memory in encrypted format.
// INPUT   : etmk  - encrypted terminal master key. (L-V-KCV)
//	     cnt   - total number of keys.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ------------------------------------------------------------
UCHAR	PED_PutPinPadTMK_RAM( UCHAR *etmk, UCHAR cnt )
{
UCHAR	i;
ULONG	status;
ULONG	DataSize = PEDS_FLASH_BACKUP_SIZE;	// 64KB, sizeof FLASH backup area
ULONG	FlashAddr = FLASH_BACKUP_BASE_ADDR;;

	
	status = apiFailed;
	
	g_pSwap = (UCHAR *)BSP_Malloc_EX( DataSize );	// allocate 64KB for memory swap
	if( g_pSwap )
	  {
	  // read FLASH data to RAM
	  FLASH_ReadData( (void *)FlashAddr, g_pSwap, DataSize );
	  
//	  UTIL_DumpHexData( 0, 3, 128, g_pSwap );
//	  for(;;);
	  
	  // update ETMKs in RAM
	  memmove( g_pSwap+(F_ADDR_PED_ETMK_01-FLASH_BACKUP_BASE_ADDR), etmk, cnt*PED_MSKEY_SLOT_LEN2 );
	  
	  status = apiOK;
	  }

	return( (UCHAR)status );
}

// ------------------------------------------------------------
// FUNCTION: save TMK to Flash memory in encrypted format.
// INPUT   : etmk  - encrypted terminal master key. (L-V-KCV)
//	     cnt   - total number of keys.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ------------------------------------------------------------
UCHAR	PED_PutPinPadTMK_ROM( UCHAR *etmk, UCHAR cnt )
{
ULONG	status;
UCHAR	*pSwap;
ULONG	DataSize = PEDS_FLASH_BACKUP_SIZE;	// 64KB, sizeof FLASH backup area
ULONG	FlashAddr = FLASH_BACKUP_BASE_ADDR;;

	
	status = apiFailed;
	  
	  // erase FLASH backup sector
	  if( FLASH_EraseSector( FlashAddr ) )
	    {
//	UTIL_DispHexByte( 3, 0, 0x11 );
//	for(;;);
	
	    // write RAM data to FLASH
	    if( FLASH_WriteData( (void *)FlashAddr, g_pSwap, DataSize ) )
	      status = apiOK;
	    }
	    
//	UTIL_DispHexByte( 3, 3, 0x22 );
//	for(;;);
	  
	  BSP_Free_EX( g_pSwap );

	return( (UCHAR)status );
}

// ------------------------------------------------------------
// FUNCTION: To setup one master key at the specified key slot.
// INPUT   : index  - key slot index. (default 0)
//           length - length of master key data. (MUST be 8 or 16)
//           key    - master key data.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ------------------------------------------------------------
UCHAR PEDS_SetMasterKey( UCHAR index, UCHAR length, UCHAR *key )
{
UCHAR buf[PED_MSKEY_SLOT_LEN2];
UCHAR temp[8];
UCHAR tkcv[8];


      if( index < MAX_MKEY_CNT )
        {
        // save encrypted key to Flash key slot
        buf[0] = length;
        PED_EncryptMKEY( length, key, &buf[1] ); // encrypted by default KEK
        PED_PutPinPadTMK( buf, index );

        // save plaintext key to SRAM key slot
        buf[0] = length;
        memmove( &buf[1], key, length );
        PEDS_WriteRamDataKEY( ADDR_PED_MKEY_01+index*PED_MSKEY_SLOT_LEN2, PED_MSKEY_SLOT_LEN2, buf );
        
        // 2018-05-02, setup KCV[3] for api_peds_GetStatus()
        memset( temp, 0x00, 8 );
        PED_TripleDES( &buf[1], 8, temp, tkcv );
        PEDS_WriteRamDataKEY( ADDR_PED_MKEY_01+index*PED_MSKEY_SLOT_LEN2+PED_MSKEY_SLOT_LEN, 3, tkcv );

        return( apiOK );
        }
      else
        return( apiFailed );
}

// ------------------------------------------------------------
// FUNCTION: To setup one master key at the specified key slot.
// INPUT   : index  - key slot index. (default 0)
//           length - length of master key data. (MUST be 8 or 16)
//           key    - master key data.
//	     kcv    - key check value (4 bytes)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           EDC                PINPAD
//           ===                ======
//           CMD_SET_MKEY + INDEX(1) + MKEY_LEN(1) + MKEY(n) + KCV(4) ->
//                              <- CMD_SET_MKEY + RC_XXX
// ------------------------------------------------------------
UCHAR api_peds_SetMasterKey( UCHAR index, UCHAR length, UCHAR *key )
{
UCHAR result;
UCHAR buf[64];
UINT  len;


      if( (length != 8) && (length != 16) && (length != 24) )
        return( apiFailed );

      // --- internal PINPAD ---
      if( g_ped_dev == PED_DEV_INT )
        return( PEDS_SetMasterKey( index, length, key ) );

      // --- external PINPAD ---
      result = apiFailed;

      buf[0] = PED_CMD_SET_MKEY;
      buf[1] = index;
      buf[2] = length;
      memmove( &buf[3], key, length );
      len = length + 3;

      if( PEDS_SendData( g_dhn_ped, len, buf ) == apiOK )
        {
        if( PEDS_ReceiveData( g_dhn_ped, 10, buf ) == apiOK )
          {
          if( (buf[0] == 2) && (buf[1] == 0) && (buf[2] == PED_CMD_SET_MKEY) && (buf[3] == PED_RC_OK) )
            result = apiOK;
          }
        }

      return( result );
}

// ------------------------------------------------------------
// FUNCTION: To setup one session key at the specified key slot.
// INPUT   : index  - session key slot index.
//                    0 = TPK, 1 = MAC Key.
//           length - length of session key data. (MUST be 8 or 16)
//           key    - session key data, encrypted by master key.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ------------------------------------------------------------
UCHAR PEDS_SetSessionKey( UCHAR index, UCHAR length, UCHAR *key )
{
UCHAR buf[PED_MSKEY_SLOT_LEN];
UCHAR skey[PED_MSKEY_SLOT_LEN];
UCHAR mkey[PED_MSKEY_SLOT_LEN];


      if( index < MAX_SKEY_CNT )
        {
        // retrive default MKEY
        PEDS_ReadRamDataKEY( ADDR_PED_MKEY_01+(g_ped_mkey_index*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, mkey );
//      TL_DumpHexData(0,0,17,mkey);

        // recover ESKEY by using MKEY
        memset( buf, 0x00, sizeof(buf) );
        buf[0] = length;
        memmove( &buf[1], key, length );
        PED_TripleDES2( &mkey[1], PED_MSKEY_LEN, &buf[1], &skey[1] );

        // save it
        skey[0] = length;
        return( PEDS_WriteRamDataKEY( ADDR_PED_SKEY_01+index*PED_MSKEY_SLOT_LEN, PED_MSKEY_SLOT_LEN, skey ) );
        }
      else
        return( apiFailed );
}

// ------------------------------------------------------------
// FUNCTION: To setup one session key at the specified key slot.
// INPUT   : index  - session key slot index.
//                    0 = TPK, 1 = MAC Key.
//           length - length of session key data. (MUST be 8 or 16)
//           key    - session key data, encrypted by master key.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           EDC                PINPAD
//           ===                ======
//           CMD_SET_SKEY + INDEX(1) + SKEY_LEN(1) + SKEY(n) ->
//                              <- CMD_SET_SKEY + RC_XXX
// ------------------------------------------------------------
UCHAR api_peds_SetSessionKey( UCHAR index, UCHAR length, UCHAR *key )
{
UCHAR result;
UCHAR buf[32];
UINT  len;


      if( (length != 8) && (length != 16) )
        return( apiFailed );

      // --- internal PINPAD ---
      if( g_ped_dev == PED_DEV_INT )
        return( PEDS_SetSessionKey( index, length, key ) );

      // --- external PINPAD ---
      result = apiFailed;

      buf[0] = PED_CMD_SET_SKEY;
      buf[1] = index;
      buf[2] = length;
      memmove( &buf[3], key, length );
      len = length + 3;

      if( PEDS_SendData( g_dhn_ped, len, buf ) == apiOK )
        {
        if( PEDS_ReceiveData( g_dhn_ped, 1, buf ) == apiOK )
          {
          if( (buf[0] == 2) && (buf[1] == 0) && (buf[2] == PED_CMD_SET_SKEY) && (buf[3] == PED_RC_OK) )
            result = apiOK;
          }
        }

      return( result );
}

// ------------------------------------------------------------
// FUNCTION: To generate PIN block by using Master/Session key
//           algorithm.
// INPUT   : mode  - algorithm of PIN block. (default 0)
//           index - session key index used to encrypt the PIN block.
//                   (default 0, using TPK)
//           pan   - full PAN.digits. (format: LEN(1) + ASCII(n))
// OUTPUT  : epb   - encrypted pin block (fixed 8 bytes).
// RETURN  : apiOK
//           apiFailed
// ------------------------------------------------------------
UCHAR PEDS_MSKEY_GenPinBlock( UCHAR mode, UCHAR index, UCHAR *pan, UCHAR *epb )
{
UCHAR pin[PED_PIN_SLOT_LEN];
UCHAR temp[PED_PIN_SLOT_LEN];
UCHAR kek[16];
UCHAR pinblock[8];
UCHAR skey[PED_MSKEY_SLOT_LEN];


      // init default KEK
      PEDS_InitDefaultKEK( kek );

      // check SKEY index and size
      if( index < MAX_SKEY_CNT )
        {
        // restore PIN data
        PEDS_ReadRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, temp ); // L-V
        pin[0] = temp[0];
        PED_TripleDES2( kek, 16, (UCHAR *)&temp[1], &pin[1] );
//      TL_DumpHexData( 0, 0, 17, pin );
//      TL_DumpHexData( 0, 3, 17, pan );

        // generate plaintext PIN Block
        PED_GenPinBlock_ISO0( pin, pan, pinblock );
//      TL_DumpHexData( 0, 0, 8, pinblock );

        // generate EPB (Encrypted PIN Block) by using SKEY
        PEDS_ReadRamDataKEY( ADDR_PED_SKEY_01+index*PED_MSKEY_SLOT_LEN, PED_MSKEY_SLOT_LEN, skey );
//      TL_DumpHexData(0,0,17,skey);

        PED_TripleDES( &skey[1], 8, pinblock, epb );
//      TL_DumpHexData(0,3,8, epb);

        PEDS_ClearRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 ); // clear PIN data
        return( apiOK );
        }
      else
        {
        PEDS_ClearRamDataKEY( ADDR_PED_PIN, PED_PIN_SLOT_LEN, 0 ); // clear PIN data
        return( apiFailed );
        }
}

// ------------------------------------------------------------
// FUNCTION: To generate PIN block by using Master/Session key
//           algorithm.
// INPUT   : mode  - algorithm of PIN block. (default 0)
//           index - session key index used to encrypt the PIN block.
//                   (default 0, using TPK)
//           pan   - full PAN.digits. (format: LEN(1) + ASCII(n))
// OUTPUT  : epb   - encrypted pin block (fixed 8 bytes).
// RETURN  : apiOK
//           apiFailed
// NOTE    : The "api_ped_GetPin()" shall be invoked prior to
//           calling this function.
//
//           EDC                PINPAD
//           ===                ======
//           CMD_MSKEY_GEN_PIN_BLOCK + MODE(1) + INDEX(1) + PAN(n) ->
//                              <- CMD_SET_MSKEY_GEN_PIN_BLOCK + RC_XXX + EPB(8)
// ------------------------------------------------------------
UCHAR api_peds_MSKEY_GenPinBlock( UCHAR mode, UCHAR index, UCHAR *pan, UCHAR *epb )
{
UCHAR result;
UCHAR buf[32];
UINT  len;


      // --- internal PINPAD ---
      if( g_ped_dev == PED_DEV_INT )
        return( PEDS_MSKEY_GenPinBlock( mode, index, pan, epb ) );

      // --- external PINPAD ---
      result = apiFailed;

      buf[0] = PED_CMD_MSKEY_GEN_PIN_BLOCK;
      buf[1] = mode;
      buf[2] = index;
      memmove( &buf[3], pan, pan[0]+1 );
      len = pan[0] + 4;

      if( PEDS_SendData( g_dhn_ped, len, buf ) == apiOK )
        {
        if( PEDS_ReceiveData( g_dhn_ped, 3, buf ) == apiOK )
          {
          if( (buf[0] == 10) && (buf[1] == 0) && (buf[2] == PED_CMD_MSKEY_GEN_PIN_BLOCK) && (buf[3] == PED_RC_OK) )
            {
            memmove( epb, &buf[4], 8 );
            result = apiOK;
            }
          }
        }

      return( result );

}

// ------------------------------------------------------------
// FUNCTION: To generate PIN block by using Master/Session key
//           algorithm.
// INPUT   : mode   - algorithm of MAC. (default 0)
//           index  - session key index used to encrypt the PIN block.
//                    (default 1, using MAC Key)
//           length - length of data used to generate MAC value,
//                    and it must be multiple of 8.
//           data   - data used to generate MAC value,
//                    and the padding data shall be included.
// OUTPUT  : mac    - MAC value (fixed 8 bytes).
// RETURN  : apiOK
//           apiFailed
// ------------------------------------------------------------
#if 0 
UCHAR PEDS_MSKEY_GenMAC( UCHAR mode, UCHAR index, UINT length, UCHAR *data, UCHAR *mac )
{
UCHAR icv[8];
UCHAR skey[PED_MSKEY_SLOT_LEN];


      // check SKEY index and size
      if( index < MAX_SKEY_CNT )
        {
        // generate MAC by using SKEY
        PEDS_ReadRamDataKEY( ADDR_PED_SKEY_01+index*PED_MSKEY_SLOT_LEN, PED_MSKEY_SLOT_LEN, skey ); // L-V
//      TL_DumpHexData( 0, 0, 17, skey );

        memset( icv, 0x00, 8 ); // icv
//      PED_GenMAC_X9D19( &skey[1], icv, length, data, mac );
        PED_GenMAC_BASE24( &skey[1], icv, length, data, mac );			// CTCB algorithm
//	PED_GenMAC_ISO16609( 0x10, &skey[1], icv, length, data, mac );		// NCCC algorithm
//	PED_GenMAC_CUP( 0x10, &skey[1], icv, length, data, mac );		// CUP  algorithm
//      TL_DumpHexData(0,0,8,mac);

        return( apiOK );
        }
      else
        return( apiFailed );
}
#endif

// ------------------------------------------------------------
// FUNCTION: To generate PIN block by using Master/Session key
//           algorithm.
// INPUT   : mode   - algorithm of MAC. (default 0)
//           index  - session key index used to encrypt the PIN block.
//                    (default 1, using MAC Key)
//           length - length of data used to generate MAC value,
//                    and it must be multiple of 8.
//           data   - data used to generate MAC value,
//                    and the padding data shall be included.
// OUTPUT  : mac    - MAC value (fixed 8 bytes).
// RETURN  : apiOK
//           apiFailed
// NOTE    : 1. The "api_ped_GetPin()" shall be called prior to
//              invoking this function.
//           2. Current limitation: max data length = 250 bytes.
//
//           EDC                PINPAD
//           ===                ======
//           CMD_MSKEY_GEN_MAC + MODE(1) + INDEX(1) + LEN(2) + Data(n) ->
//                              <- CMD_SET_MSKEY_GEN_MAC + RC_XXX + MAC(8)
// ------------------------------------------------------------
#if 0 
UCHAR api_peds_MSKEY_GenMAC( UCHAR mode, UCHAR index, UINT length, UCHAR *data, UCHAR *mac )
{
UCHAR result;
UCHAR buf[105];
UINT  len;

      // check max length and MOD-8
      if( length > (sizeof(buf) - 5) )
        return( apiFailed );

     // --- internal PIN PAD ---
      if( g_ped_dev == PED_DEV_INT )
        return( PEDS_MSKEY_GenMAC( mode, index, length, data, mac ) );

     // --- external PIN PAD ---
      result = apiFailed;

      buf[0] = PED_CMD_MSKEY_GEN_MAC;
      buf[1] = mode;
      buf[2] = index;
      buf[3] = length & 0x00FF;
      buf[4] = (length & 0xFF00) >> 8;
      memmove( &buf[5], data, length );
      len = length + 5;

      if( PEDS_SendData( g_dhn_ped, len, buf ) == apiOK )
        {
        if( PEDS_ReceiveData( g_dhn_ped, 10, buf ) == apiOK )
          {
          if( (buf[0] == 10) && (buf[1] == 0) && (buf[2] == PED_CMD_MSKEY_GEN_MAC) && (buf[3] == PED_RC_OK) )
            {
            memmove( mac, &buf[4], 8 );
            result = apiOK;
            }
          }
        }

      return( result );
}
#endif

// ------------------------------------------------------------
// FUNCTION: To get the status of PIN Pad.
// INPUT   : none.
// OUTPUT  : status - status of PIN Pad. (total 32 bytes)
//           OLD>>    Model           : ASCII(12)
//                    Version         : ASCII(4)
//                    MKEY Slot Status: HEX(2)
//                    SKEY Slot Status: HEX(2)
//                    KCV             : HEX(1*3)  -> 1 TMK (NCCC)
//                    RFU             : HEX(9)
//
//                                    ;; total 60 bytes
//           NEW>>    Model           : ASCII(12)
//                    Version         : ASCII(4)
//                    MKEY Slot Status: HEX(2)
//                    SKEY Slot Status: HEX(2)
//                    KCV             : HEX(10*3) -> 10 TMKs (CTCB)
//                    TSN             : ASCII(8)
//                    RFU             : HEX(2)
//
// RETURN  : apiOK
//           apiFailed  (no response)
// ------------------------------------------------------------
UCHAR PEDS_GetStatus( UCHAR *status )
{
UCHAR i;
UCHAR result;
UINT  len;
UCHAR buf[80];
UCHAR key[PED_MSKEY_SLOT_LEN2];
UINT  mkeymap, skeymap;
UCHAR msg_model[] =             {"AS300 PINPAD"};
UCHAR msg_version[] =           {"V205"};
UCHAR kcv[10][3] = {0};
UCHAR tsn[16];
UCHAR temp[8];
UCHAR tkcv[8];


//    api_sys_info( SID_TerminalSerialNumber, tsn );	// LEN(1) + CUST_ID(N3) + SN(N8)
      api_sys_info( SID_TerminalSerialNumber2, tsn );	// LEN(1) + TSN(N8)

      // Model (12)
      memmove( &buf[2], (UCHAR *)&msg_model, 12 );

      // Version (4)
      memmove( &buf[2+12], (UCHAR *)&msg_version, 4 );

      // MKEY Status (2)
      mkeymap = 0;
      for( i=0; i<MAX_MKEY_CNT; i++ )
         {
         PEDS_ReadRamDataKEY( ADDR_PED_MKEY_01+i*PED_MSKEY_SLOT_LEN2, PED_MSKEY_SLOT_LEN2, key );
//       if( i == 0 )
//         //memmove( kcv, &key[17], sizeof(kcv) );
//           memmove( &kcv[i][0], &key[17], 3 );

         if( key[0] == PED_MSKEY_LEN ) // PATCH: 2009/01/03
           {
           // verify KCV
           memset( temp, 0x00, 8 );
           PED_TripleDES( &key[1], 8, temp, tkcv );
           if( (tkcv[0] == key[17]) && (tkcv[1] == key[18]) && (tkcv[2] == key[19]) )
             {
             memmove( &kcv[i][0], &key[17], 3 );
             mkeymap |= ((UINT)1 << i);
             }
           }
         }
      buf[2+12+4] = (mkeymap & 0x00FF);
      buf[2+12+5] = (mkeymap & 0xFF00) >> 8;

      // SKEY Status (2)
      skeymap = 0;
      for( i=0; i<MAX_SKEY_CNT; i++ )
         {
         PEDS_ReadRamDataKEY( ADDR_PED_SKEY_01+i*PED_MSKEY_SLOT_LEN, PED_MSKEY_SLOT_LEN, key );

         if( key[0] == PED_MSKEY_LEN ) // PATCH: 2009/01/03
           skeymap |= ((UINT)1 << i);
         }
      buf[2+12+6] = (skeymap & 0x00FF);
      buf[2+12+7] = (skeymap & 0xFF00) >> 8;

      // RFU (12)
//    memset( &buf[2+12+8], 0x00, 12 );
//    memmove( &buf[2+12+8], kcv, 3 );

      memset( &buf[2+12+8], 0x00, 40 ); // KCV(10*3) + TSN(8) + RFU(2)

      // 10*KCV(3)
      memmove( &buf[2+12+8], &kcv[0][0], sizeof(kcv) );

      // TSN(8)
//    memmove( &buf[2+12+8+30], &tsn[4], 8 );
      memmove( &buf[2+12+8+30], &tsn[1], 8 );
      
      memmove( status, &buf[2], 60 );

      return( apiOK );
}

// ------------------------------------------------------------
// FUNCTION: To get the status of PIN Pad.
// INPUT   : none.
// OUTPUT  : status - status of PIN Pad. (total 32 bytes)
//           OLD>>    Model           : ASCII(12)
//                    Version         : ASCII(4)
//                    MKEY Slot Status: HEX(2)
//                    SKEY Slot Status: HEX(2)
//                    KCV             : HEX(1*3)  -> 1 TMK (NCCC)
//                    RFU             : HEX(9)
//
//                                    ;; total 60 bytes
//           NEW>>    Model           : ASCII(12)
//                    Version         : ASCII(4)
//                    MKEY Slot Status: HEX(2)
//                    SKEY Slot Status: HEX(2)
//                    KCV             : HEX(10*3) -> 10 TMKs (CTCB)
//                    TSN             : ASCII(8)
//                    RFU             : HEX(2)
//
// RETURN  : apiOK
//           apiFailed  (no response)
//
//           EDC                PINPAD
//           ===                ======
//           CMD_GET_STATUS ->
//                              <- CMD_GET_STATUS + RC_XXX + STATUS(32)
// ------------------------------------------------------------
UCHAR api_peds_GetStatus( UCHAR *status )
{
UCHAR result;
UCHAR buf[80];


     // --- internal PIN PAD ---
      if( g_ped_dev == PED_DEV_INT )
        return( PEDS_GetStatus( status ) );

      // --- external PINPAD ---
      result = apiFailed;

      buf[0] = PED_CMD_GET_STATUS;

      if( PEDS_SendData( g_dhn_ped, 1, buf ) == apiOK )
        {
        if( PEDS_ReceiveData( g_dhn_ped, 1, buf ) == apiOK )
          {
          if( (buf[0] == 34) && (buf[1] == 0) && (buf[2] == PED_CMD_GET_STATUS) && (buf[3] == PED_RC_OK) )
            {
            memmove( status, &buf[4], 32 );     // old format
            result = apiOK;
            }
          else
           {
           if( (buf[0] == 62) && (buf[1] == 0) && (buf[2] == PED_CMD_GET_STATUS) && (buf[3] == PED_RC_OK) )
             {
             memmove( status, &buf[4], 60 );    // new format
             result = apiOK;
             }
           }
          }
        }

      return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Decrypt master key by using default KEK.
// INPUT   : emkey - encrypted master key. (V, 16 bytes)
// OUTPUT  : mkey  - decrypted master key. (V, 16 bytes)
// RETURN  : TRUE  - OK.
//           FALSE - failed.
// ---------------------------------------------------------------------------
void  PEDS_DecryptMKEY( UCHAR *emkey, UCHAR *mkey )
{
UCHAR kek[16];

      // init default KEK
      PEDS_InitDefaultKEK( kek );

      // Decrypt default MKEY by default Key Encryption Key (KEK)
      PED_TripleDES2( kek, 16, emkey, mkey );
}

// ------------------------------------------------------------
// FUNCTION: To download default master key from KIT
//           (Key Injection Tool) to the key slot in SAM.
// INPUT   : port - serial port ID. (default 0)
//           tout - timeout in seconds used to wait for download.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           KIT                EDC
//           ===                ===
//           SOH LEN(1) ID(1) EMKEY(16) NULL(17) LRC ->
//           SOH LEN(1) ID(1) EMKEY(16) KCV(3) NULL(14) LRC -> (NEW)
//                              <- ACK (if valid)
//                              <- NAK (if invalid)
//           where: ID = 0x01, NULL = 0x00.
//                  EMKEY = TDES( DefaultKEK, MKEY )
//                  DefaultKEK = (EA, C7, 49, 0D, 5D, EC, 51, 54, 16, 40, B6, CD, 08, 13, 2C, EA)
// ------------------------------------------------------------
UCHAR api_peds_LoadTerminalMasterKey( UCHAR port, UCHAR tout )
{
UCHAR result;
UCHAR dhn_aux;
UCHAR buf[40];
UCHAR mkey[PED_MSKEY_SLOT_LEN2];  // LEN(1) TMK(16) KCV(3)
UINT  len;
UCHAR dhn_tim;
UINT  tick1, tick2;

API_AUX	pAux;

UCHAR	i;
UCHAR	index;
UCHAR	temp[8];
UCHAR	kcv[8];
UCHAR	etmk[MAX_MKEY_CNT][PED_MSKEY_SLOT_LEN2];
UCHAR	tmk_cnt = 0;


      // 1. open CP AUX port

      // --- enable hardware port ---
      api_aux_close( 0 );
      
      pAux.Mode = auxDLL;
      pAux.Baud = COM_9600 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
      pAux.Tob = 10;	// 100ms
      pAux.Tor = 100;	// 1 sec
      pAux.Acks = 1;	// ACKS=1
      pAux.Resend = 0;	// RESEND=0
      dhn_aux = api_aux_open( port, pAux );

      if( (dhn_aux == apiOutOfService) || (dhn_aux == apiOutOfLink) )
        return( apiFailed );

      result = apiFailed;

      // clear all current MS keys
      for( i=0; i<MAX_MKEY_CNT; i++ )
         {
         PEDS_ClearRamDataKEY( ADDR_PED_MKEY_01+(i*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, 0x00 );
         PEDS_ClearRamDataKEY( ADDR_PED_SKEY_01+(i*PED_MSKEY_SLOT_LEN), PED_MSKEY_SLOT_LEN, 0x00 );
         etmk[i][0] = 0;
         }

      dhn_tim = api_tim_open( 100 ); // 1 sec
      api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );

      // 2. waiting for KIT download
      while(1)
           {
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
	     if( buf[2] > MAX_MKEY_CNT )
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
             PEDS_DecryptMKEY( &buf[3], &mkey[1] );
             mkey[0] = PED_MSKEY_LEN;
             memmove( &mkey[17], &buf[2+17], 3 ); // KCV

//	TL_DumpHexData( 0, 0, 17, mkey );

             // Verify KCV and save TMK
             memset( temp, 0x00, 8 );
             PED_TripleDES( &mkey[1], 8, temp, kcv );

//	TL_DumpHexData( 0, 3, 8, kcv );

             if( (kcv[0] == buf[2+17]) && (kcv[1] == buf[2+18]) && (kcv[2] == buf[2+19]) )
               {
               // temp save to array buffer
               buf[2] = PED_MSKEY_LEN;
               memmove( &etmk[index][0], &buf[2], PED_MSKEY_SLOT_LEN2 );
               
               // save plaintext TMK to SRAM key slot
               PEDS_WriteRamDataKEY( ADDR_PED_MKEY_01+(index*PED_MSKEY_SLOT_LEN2), PED_MSKEY_SLOT_LEN2, mkey );
               }               
             }
           }

      api_tim_close( dhn_tim );
      api_aux_close( dhn_aux );
      
      // finally save ETMKs to backup FLASH memory
      if( result == apiOK )
        {
        PED_PutPinPadTMK_RAM( &etmk[0][0], tmk_cnt );
        PED_PutPinPadTMK_ROM( &etmk[0][0], tmk_cnt );
        }

      return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: setup index of TMK.
// INPUT   : index  - key slot index. (00..09)
// OUTPUT  : none.
// RETURN  : apiOK     - OK.
//           apiFailed - failed.
// ---------------------------------------------------------------------------
UCHAR 	PED_SelectMasterKey_CUP( UCHAR index )
{
	if( index < MAX_MKEY_CNT )
	  {
	  g_ped_mkey_index = index;
	  return( apiOK );
	  }
	else
	  return( apiFailed );
}

// ------------------------------------------------------------
// FUNCTION: To select one master key from the specified key slot
//           for the succeeding operation.
// INPUT   : index  - key slot index. (00..09)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
//
//           EDC                PINPAD
//           ===                ======
//           CMD_SELECT_MKEY + INDEX(1) ->
//                              <- CMD_SELECT_MKEY + RC_XXX
// ------------------------------------------------------------
UCHAR api_peds_SelectMasterKey( UCHAR index )
{
UCHAR result;
UCHAR buf[32];
UINT  len;


      // --- internal PINPAD ---
      if( g_ped_dev == PED_DEV_INT )
        return( PED_SelectMasterKey_CUP( index ) );

      // --- external PINPAD ---
      result = apiFailed;

      buf[0] = PED_CMD_SELECT_MKEY;
      buf[1] = index;
      len = 2;

      if( PEDS_SendData( g_dhn_ped, len, buf ) == apiOK )
        {
        if( PEDS_ReceiveData( g_dhn_ped, 10, buf ) == apiOK )
          {
          if( (buf[0] == 2) && (buf[1] == 0) && (buf[2] == PED_CMD_SELECT_MKEY) && (buf[3] == PED_RC_OK) )
            result = apiOK;
          }
        }

      return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Perform data encryption by using TDES with Master/Session key.
// INPUT   : mode   - operation mode. (ECB, CBC, encrypt, decrypt)
//		      DES_MODE_xxx.
// 	     index  - session key slot index.
//	     icv    - initial chained vector.
//	     length - size of input data.
//	     idata  - data to be encrypted or decrypted.
//		      format: V(8*n)
// OUTPUT  : odata  - the encrypted or decrypted data.
//		      format: V(8*n)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	PED_MSKEY_CipherTDES( UINT8 mode, UINT8 index, UINT8 *icv, UINT16 length, UINT8 *idata, UINT8 *odata )
{
UCHAR	result;
UCHAR	skey[PED_MSKEY_SLOT_LEN];


	result = apiFailed;
	
	if( index < MAX_SKEY_CNT )
	  {
	  // retrieve SKEY
	  PEDS_ReadRamDataKEY( ADDR_PED_SKEY_01+index*PED_MSKEY_SLOT_LEN, PED_MSKEY_SLOT_LEN, skey ); // L-V
	  if( (skey[0] != 8) && (skey[0] != 16) && (skey[0] != 24) )
	    return( apiFailed );
	  
	  switch( mode & 0x0F )
	        {
	        case DES_MODE_ECB:
	        
	             if( mode & DES_MODE_DECRYPT )
	               PED_TripleDES2( &skey[1], length, idata, odata );
	             else
	               PED_TripleDES( &skey[1], length, idata, odata );
	             
	             result = apiOK;
	             break;
	             
	        case DES_MODE_CBC:
	  
	             if( mode & DES_MODE_DECRYPT )
	               PED_CBC_TripleDES2( &skey[1], icv, length, idata, odata );
	             else
	               PED_CBC_TripleDES( &skey[1], icv, length, idata, odata );
	             
	             result = apiOK;
	             break;
	        }
	  }

	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Perform data encryption by using TDES with Master/Session key.
// INPUT   : mode   - operation mode. (ECB, CBC, encrypt, decrypt)
//		      DES_MODE_xxx.
// 	     index  - session key slot index.
//	     icv    - initial chained vector.
//	     length - size of input data.
//	     idata  - data to be encrypted or decrypted.
//		      format: V(8*n)
// OUTPUT  : odata  - the encrypted or decrypted data.
//		      format: V(8*n)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_peds_MSKEY_TDES( UCHAR mode, UCHAR index, UCHAR *icv, UINT length, UCHAR *idata, UCHAR *odata )
{
UCHAR	buf[500];
UINT	len;
UCHAR	result;
UCHAR	skey[PED_MSKEY_SLOT_LEN];


	// --- internal PINPAD ---
	if( g_ped_dev == PED_DEV_INT )
	  return( PED_MSKEY_CipherTDES( mode, index, icv, length, idata, odata ) );

	// --- external PINPAD ---
	result = apiFailed;
	
	buf[0] = PED_CMD_MSKEY_TDES;
	buf[1] = mode;
	buf[2] = index;
	memmove( &buf[3], icv, 8 );
	buf[11] = length & 0x00FF;
	buf[12] = (length & 0xFF00) >> 8;
	memmove( &buf[13], idata, length );
	len = length + 13;
	
	if( PEDS_SendData( g_dhn_ped, len, buf ) == apiOK )
	  {
	  if( PEDS_ReceiveData( g_dhn_ped, 10, buf ) == apiOK )
	    {
	    if( (buf[2] == PED_CMD_MSKEY_TDES) && (buf[3] == PED_RC_OK) )
	      {
	      memmove( odata, &buf[4], length );
	      result = apiOK;
	      }
	    }
	  }
	
	return( result );
}


