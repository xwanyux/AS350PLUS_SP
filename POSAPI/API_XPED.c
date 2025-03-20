//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							                                **
//**  PRODUCT  : AS350 PLUS						                            **
//**                                                                        **
//**  FILE     : API_XPED.C                                                 **
//**  MODULE   : api_xped_xxx()				                                **
//**									                                    **
//**  FUNCTION : API::XPED (External Pinpad Entry Device)		            **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2025/01/02                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2025 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"
#include "OS_SECM.h"
#include "OS_PROCS.h"
//#include "DEV_XPED.h"
#include "OS_FLASH.h"

UCHAR	g_dhn_xped = COM1 + psDEV_AUX + 0x80;	// fixed COM1

//extern	UCHAR apk_WriteRamDataKEY( ULONG address, ULONG length, UCHAR *data );

#define ADDR_PED_PIN2                   0x7800          // b17, L-V

//typedef	struct API_GENEPB_S
//{
//	UCHAR			Mode;		// algorithm and padding method
//	UCHAR			Index;		// key slot index (not used for DUKPT)
//	UCHAR			Algo;		// algorithm
//} __attribute__((packed)) API_GENEPB;


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
UCHAR	XPED_SendData( UCHAR dhn, UINT len, UCHAR *data )
{
UCHAR	sbuf[510];


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
UCHAR	XPED_ReceiveData( UCHAR dhn, UCHAR tout, UCHAR *data )
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
// FUNCTION: receive data from AUX port.
// INPUT   : none.
// OUTPUT  : data - 2L-V, the data receivd.
// REF     : g_dhn_aux
// RETURN  : TRUE  - data ready.
//           FALSE - device error or timeout before data ready.
// ---------------------------------------------------------------------------
UCHAR	XPED_ReceiveAUX( UCHAR *data )
{
	if( XPED_ReceiveData( g_dhn_xped, 20, data ) == apiOK )	// timeout = 20sec
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: receive data from AUX port.
// INPUT   : none.
// OUTPUT  : data - 2L-V, the data receivd.
// REF     : g_dhn_aux
// RETURN  : TRUE  - data ready.
//           FALSE - device error or timeout before data ready.
// ---------------------------------------------------------------------------
UCHAR	XPED_ReceiveAUX2( UCHAR *data, UINT tout )
{
	if( XPED_ReceiveData( g_dhn_xped, tout, data ) == apiOK )	// timeout = 20sec
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: transmit data to AUX port.
// INPUT   : data - 2L-V, the data to be transmitted.
// OUTPUT  : none.
// REF     : g_dhn_aux
// RETURN  : TRUE  - data ready.
//           FALSE - device error.
// ---------------------------------------------------------------------------
UCHAR	XPED_TransmitAUX( UCHAR *data )
{
UINT	iLen;

	iLen = data[0] + data[1]*256;
	if( XPED_SendData( g_dhn_xped, iLen, &data[2] ) == apiOK )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To read CA Public Key data element from SRAM page memory.
// INPUT   : address - begin address to read.
//           length  - length of the data element.
// OUTPUT  : data    - the data element read.
// RETURN  : apkOK     - matched.
//           apkFailed - not matched.
// ---------------------------------------------------------------------------
UCHAR	XPED_ReadRamDataKEY( UINT address, UINT length, UCHAR *data )
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
UCHAR	XPED_WriteRamDataKEY( UINT address, UINT length, UCHAR *data )
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
UCHAR	XPED_ClearRamDataKEY( UINT address, UINT length, UCHAR pattern )
{
	OS_SECM_ClearData( address, length, pattern );
	return( apiOK );
}

// ---------------------------------------------------------------------------
void  XPED_InitDefaultKEK( UCHAR *kek )
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
// FUNCTION: To request PIN entry from external PIN pad device.
// INPUT   : none.
// OUTPUT  : amt   - amount to be confirmed on display. (external)
//                   format: LEN(1) + ASCII(n)
//                           LEN = 0: no confirmation.
//	     tout  - PIN entry timeout in seconds.
//	     type  - message type for PIN prompts.
//		     0x00 = show "ENTER PIN or ENTER"
//		     0x01 = show "ENTER PIN" only
// RETURN  : apiOK         (key-in digit available)
//           apiFailed     (timeout or invalid request)
//	     apiNotReady   (bypass PIN)
// NOTE    : the COM port shall be initialized in advance by AP.
// ---------------------------------------------------------------------------
UCHAR	XPED_GetPin( UINT tout, UCHAR *amt, UCHAR type )
{
UCHAR	ikek[16];
UCHAR	temp[32];
UCHAR	buf1[32];
UCHAR	buf2[40];
UCHAR	key[16];
UCHAR	rnd_b[8];
UCHAR	ernd[8];
UCHAR	result;


	// 0. Open AUX port -- testing ONLY, it shall be called by AP.
//	api_peds_OpenPinPad( 1 );

	// -----------------------------------------------------------------------------
	// 1. Exchange Key Encryption Key (KEK) between EDC and PINPAD.
	//
	// EDC		-> PED_CMD_EXCH_KEK(1) ERND(8)
	// PINPAD	-> PED_CMD_EXCH_KEK(1) RC(1) CRYPTOGRAM(24)
	//
	// where:	ERND(8) = TDES(RND_B(8), Kp)
	//		CRYPTORAM(24) = TDES(KEK(16)+RND_R(8), Kp), available only if RC(1) = RC_OK.
	//
	//			RND_B(8) = random number, generated by EDC
	//			RND_R(8) = random number, generated by PINPAD
	//			KEK(16)  = RND_R[5:8]+RND_B[1:4]+RND_R[1:4]+RND_B[5:8]
	//
	// 2. Transfer enciphered PIN data from PINPAD to EDC.
	//
	// EDC		-> PED_CMD_GET_EPIN(1)
	// PINPAD	-> PED_CMD_GET_EPIN(1) RC(1) EPIN(16)
	//
	// where:	
	//		RC(1)      = RC_OK
	//			     RC_FAILED (timeout or bypass PIN)
	//		EPIN(16)   = enciphered PIN data, available only if RC(1) = RC_OK.
	//			   = TDES(PIN_LEN(1)+PIN(15), KEK)
	// -----------------------------------------------------------------------------

	// 1. Exchange KEK
	
	// Initial KEK for AS300 PINPAD (fixed)
	XPED_InitDefaultKEK( ikek );
//	ikek[0] = 0xEA;
//	ikek[1] = 0xC7;
//	ikek[2] = 0x49;
//	ikek[3] = 0x0D;
//	ikek[4] = 0x5D;
//	ikek[5] = 0xEC;
//	ikek[6] = 0x51;
//	ikek[7] = 0x54;
//    
//	ikek[8] = 0x16;
//	ikek[9] = 0x40;
//	ikek[10]= 0xB6;
//	ikek[11]= 0xCD;
//	ikek[12]= 0x08;
//	ikek[13]= 0x13;
//	ikek[14]= 0x2C;
//	ikek[15]= 0xEA;

	// Random Number (RND_B)
	api_sys_random( rnd_b );
//	rnd_b[0] = 0x88;
//	rnd_b[1] = 0x99;
//	rnd_b[2] = 0xAA;
//	rnd_b[3] = 0xBB;
//	rnd_b[4] = 0xCC;
//	rnd_b[5] = 0xDD;
//	rnd_b[6] = 0xEE;
//	rnd_b[7] = 0xFF;

	// ERND = TDES( RND_B, IKEK )
	PED_TripleDES( ikek, 8, rnd_b, ernd );
	
	buf1[0] = 9;
	buf1[1] = 0;
	buf1[2] = 0x80;	// PED_CMD_EXCH_KEK;
	memmove( &buf1[3], ernd, 8 );
		 
      // process command
      result = apiFailed;
      
      if( XPED_TransmitAUX( buf1 ) == TRUE )
        {
        if( XPED_ReceiveAUX( buf2 ) == TRUE )
          {
          if( (buf2[0] == 26) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]) && (buf2[3] == 0x00) )
            {
            // recover, check, and save KEK(16)
            memmove( temp, &buf2[4], 24 );
            PED_TripleDES2( ikek, 24, temp, buf2 );
//            LIB_DumpHexData( 0, 0, 24, buf2 );
            memmove( &buf1[1], buf2, 24 );

            if( (LIB_memcmp( &buf1[1], &buf1[21], 4 ) != 0) ||
                (LIB_memcmp( &buf1[9], &buf1[17], 4 ) != 0) ||
                (LIB_memcmp( &buf1[5], &rnd_b[0], 4 ) != 0) ||
                (LIB_memcmp( &buf1[13], &rnd_b[4], 4 ) != 0) )
              result = apiFailed;
            else
              {
	      memmove( ikek, &buf1[1], 16 );
              result = apiOK;
              }
            }
          }
        }

	LIB_WaitTime( 20 );	// ACK delay 200ms

	// 2.1 Issue PED_CMD_RESET
	if( result != apiOK )
	  {
	  result = apiDeviceError;	// 2012-11-23, pinpad not work
	  goto PROC_EXIT;
	  }
	  
	buf1[0] = 4 + amt[0];
	buf1[1] = 0;
	buf1[2] = '2';	// PED_CMD_RESET;
	buf1[3] = tout;	// TIMEOUT = 20 sec, 2013-12-07, add "tout"
//	buf1[4] = 0;	// TRANS_TYPE
	buf1[4] = type;	// 2015-06-22
	memmove( &buf1[5], amt, amt[0]+1 );	// LEN + AMT

      // process command
      result = apiFailed;
      
      if( XPED_TransmitAUX( buf1 ) == TRUE )
        {
        if( XPED_ReceiveAUX( buf2 ) == TRUE )
          {
          if( (buf2[0] == 2) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]) && (buf2[3] == 0x00) )
            {
	    result = apiOK;
            }
          }
        }	  
	
	LIB_WaitTime( 20 );	// ACK delay 200ms
	 
	// 2.2 Issue PED_CMD_GET_PIN
	if( result != apiOK )
	  {
	  result = apiDeviceError;	// 2012-11-23, pinpad not work
	  goto PROC_EXIT;
	  }
	  
	buf1[0] = 1;
	buf1[1] = 0;
	buf1[2] = '3';	// PED_CMD_GET_PIN
	
     // process command
      result = apiFailed;
      
      if( XPED_TransmitAUX( buf1 ) == TRUE )
        {
        if( XPED_ReceiveAUX2( buf2, tout ) == TRUE )	// 2013-12-17, add "tout"
          {
          if( (buf2[0] == 3) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]) && (buf2[3] == 0x00) )
            {
            // check retur key stroke
	    if( buf2[4] == 'y' )
	      result = apiOK;		// PIN entered
	    else
	      {
	      if( buf2[4] == 'x' )	// 2010-03-22, aborted by CANCEL key
	        result = apiFailed;
	      else
	        result = apiNotReady;	// PIN bypassed
	      }
            }
          }
        else
          result = apiDeviceError;	// 2012-11-23, pinpad not work
        }             
      else
        result = apiDeviceError;	// 2012-11-23, pinpad not work
	
	
//	LIB_DumpHexData( 0, 0, 5, buf2 );
//	for(;;);
	
	LIB_WaitTime( 20 );	// ACK delay 200ms
	
	// 2.3 Issue PED_CMD_GET_EPIN
	if( result != apiOK )
	  goto PROC_EXIT;
	  
	buf1[0] = 1;
	buf1[1] = 0;
	buf1[2] = 0x81;	// PED_CMD_GET_EPIN
	
     // process command
      result = apiFailed;
      
      if( XPED_TransmitAUX( buf1 ) == TRUE )
        {
        if( XPED_ReceiveAUX( buf2 ) == TRUE )
          {
          if( (buf2[0] == 18) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]) && (buf2[3] == 0x00) )
            {
            // retrieve and save PIN data
            memmove( temp, &buf2[4], 16 );
            memset( buf2, 0x00, sizeof(buf2) );
	    PED_TripleDES2( ikek, 16, temp, buf2 );	// buf2 = PIN_LEN(1) + PIN(n)
//	    LIB_DumpHexData( 0, 0, 16, buf2 );

	    OS_SECM_PutData( ADDR_PEDS_PIN, PEDS_PIN_SLOT_LEN, buf2 );	// save PIN data to SECM
	    
	    result = apiOK;
            }
          }
        else
          result = apiDeviceError;	// 2012-11-23, pinpad not work
        }
      else
        result = apiDeviceError;	// 2012-11-23, pinpad not work		

      // PATHC: 2009-04-07, clear sensitive data
PROC_EXIT:
      memset( ikek, 0x00, sizeof(ikek) );
      memset( temp, 0x00, sizeof(temp) );
      memset( buf1, 0x00, sizeof(buf1) ); 
      memset( buf2, 0x00, sizeof(buf2) );
      memset( key, 0x00, sizeof(key) );

      return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate encrypted PIN block by using Master/Session key algorithm.
// INPUT   : mode   - algorithm of PIN block.
//		      0: ISO 9564 Format 0 (ANSI X9.8)
//		      1: ISO 9564 Format 1
//		      2: ISO 9564 Format 2
//		      3: ISO 9564 Format 3
// 	     index  - session key index used to encrypt the PIN block.
//	     pan    - full PAN digits or transaction field for ISO3. (format: L-V)
// OUTPUT  : epb    - encrypted pin block (LL-V(8))
//	     ksn    - key serial number   (LL-V(10) effective only for DUKPT).
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT8	XPED_GenEncryptedPinBlock( UCHAR algo, UINT8 mode, UINT8 index, UINT8 *pan, UINT8 *epb, UINT8 *ksn )
{
UINT8	buf1[64];
UINT8	buf2[64];
UINT8	result;


	// 0. Open AUX port -- testing ONLY
//	if( LIB_OpenAUX( COM1, auxDLL ) == FALSE )
//	  return( apiFailed );
	  
	// -----------------------------------------------------------------------------
	// 1. XPED_GetPin() has been called in advance.
	//
	// 2. Issue PED_CMD_MSKEY_GEN_PIN_BLOCK ('4').
	// EDC		-> PED_CMD_MSKEY_GEN_PIN_BLOCK(1) MODE(1) INDEX(1) PAN_LEN(1) PAN(n)
	// PINPAD	-> PED_CMD_MSKEY_GEN_PIN_BLOCK(1) RC(1) EPB(8) KSN(10)
	//		   
	//		Where: EPB(8)  = encrypted PIN block.
	//		       KSN(10) = key sequence number for DUKPT. (will not be returned if using old AS300PP)
	// -----------------------------------------------------------------------------
	LIB_WaitTime( 20 );	// ACK delay 200ms
	
	buf1[0] = 4 + pan[0];
	buf1[1] = 0;
	buf1[2] = '4';	// PED_CMD_GEN_PIN_BLOCK
	buf1[3] = mode;
	buf1[4] = index;
	memmove( &buf1[5], pan, pan[0]+1 );
	
      // process command
      result = apiFailed;
      
      if( XPED_TransmitAUX( buf1 ) == TRUE )
        {
        if( XPED_ReceiveAUX( buf2 ) == TRUE )
          {
          if( (buf2[0] == 10) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]) && (buf2[3] == 0x00) )
            {	// with EPB only
            memmove( &epb[2], &buf2[4], 8 );
            epb[0] = 8;
            epb[1] = 0;
            
	    result = apiOK;
            }
          else
            {	// with both EPB & KSN
            if( (buf2[0] == 20) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]) && (buf2[3] == 0x00) )
              {
              memmove( &epb[2], &buf2[4], 8 );
              epb[0] = 8;
              epb[1] = 0;
              
              memmove( &ksn[2], &buf2[12], 10 );
              ksn[0] = 10;
              ksn[1] = 0;
              
              result = apiOK;
              }
            }
          }
        }

	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: To request PIN entry from external PIN pad device.
// INPUT   : none.
// OUTPUT  : amt   - amount to be confirmed on display. (external)
//                   format: LEN(1) + ASCII(n)
//                           LEN = 0: no confirmation.
//	     tout  - PIN entry timeout in seconds.
// RETURN  : apiOK         (key-in digit available)
//           apiFailed     (timeout or invalid request)
//	     apiNotReady   (bypass PIN)
// ---------------------------------------------------------------------------
UCHAR	api_xped_GetPin( UINT tout, UCHAR *amt )
{	
	return( XPED_GetPin( tout, amt, 0 ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To request PIN entry from external PIN pad device.
// INPUT   : none.
// OUTPUT  : amt   - amount to be confirmed on display. (external)
//                   format: LEN(1) + ASCII(n)
//                           LEN = 0: no confirmation.
//	     tout  - PIN entry timeout in seconds.
// RETURN  : apiOK         (key-in digit available)
//           apiFailed     (timeout or invalid request)
//	     apiNotReady   (bypass PIN)
// ---------------------------------------------------------------------------
UCHAR	api_xped_GetPin2( UINT tout, UCHAR *amt )
{	
	return( XPED_GetPin( tout, amt, 1 ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate online encrypted PIN block from external PIN pad device.
// INPUT   : algo   - key algorithm.
// 	     mode   - pin block format.
// 	     index  - key index.
//	     pan    - PAN. (format: L-V)
// OUTPUT  : epb    - encrypted pin block (fixed  8 bytes).
//	     ksn    - key sequence number (fixed 10 bytes).
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_xped_GenEncryptedPinBlock( UCHAR algo, UCHAR mode, UCHAR index, UCHAR *pan, UCHAR *epb, UCHAR *ksn )
{
	return( XPED_GenEncryptedPinBlock( algo, mode, index, pan, epb, ksn ) );
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
UCHAR XPED_MSKEY_GenPinBlock( UCHAR mode, UCHAR index, UCHAR *pan, UCHAR *epb )
{
UCHAR pin[PEDS_PIN_SLOT_LEN];
UCHAR temp[PEDS_PIN_SLOT_LEN];
UCHAR kek[16];
UCHAR pinblock[8];
UCHAR skey[PED_MSKEY2_SLOT_LEN];


      // init default KEK
      XPED_InitDefaultKEK( kek );

      // check SKEY index and size
      if( index < MAX_SKEY2_CNT )
        {
        // restore PIN data
        XPED_ReadRamDataKEY( ADDR_PEDS_PIN, PEDS_PIN_SLOT_LEN, temp ); // L-V
        pin[0] = temp[0];
        PED_TripleDES2( kek, 16, (UCHAR *)&temp[1], &pin[1] );
//      TL_DumpHexData( 0, 0, 17, pin );
//      TL_DumpHexData( 0, 3, 17, pan );

        // generate plaintext PIN Block
        PED_GenPinBlock_ISO0( pin, pan, pinblock );	// same as IPC_server: OS_PED3.c
//      TL_DumpHexData( 0, 0, 8, pinblock );

        // generate EPB (Encrypted PIN Block) by using SKEY
        XPED_ReadRamDataKEY( ADDR_PED_SKEY2_01+index*PED_MSKEY2_SLOT_LEN, PED_MSKEY2_SLOT_LEN, skey );
//      TL_DumpHexData(0,0,17,skey);

        PED_TripleDES( &skey[1], 8, pinblock, epb );
//      TL_DumpHexData(0,3,8, epb);

        XPED_ClearRamDataKEY( ADDR_PEDS_PIN, PEDS_PIN_SLOT_LEN, 0 ); // clear PIN data
        return( apiOK );
        }
      else
        {
        XPED_ClearRamDataKEY( ADDR_PEDS_PIN, PEDS_PIN_SLOT_LEN, 0 ); // clear PIN data
        return( apiFailed );
        }
}

// ------------------------------------------------------------
// FUNCTION: To generate PIN block by using Master/Session.
//	     This function is used to simulate non-PCI spec to generate EMV online PIN block
//	     according to NCCC/CTCB spec.
//
// NOTE    : non-PCI application  : the PIN data is encrypted by IKEK and saved in SRAM key slot. (NCCC/CTCB)
//	     PCI & EMV application: the PIN data is saved in SECM key slot by plaintext format.
//
// INPUT   : mode  - algorithm of PIN block. (default 0)
//           index - session key index used to encrypt the PIN block.
//                   (default 0, using TPK)
//           pan   - full PAN.digits. (format: LEN(1) + ASCII(n))
// OUTPUT  : epb   - encrypted pin block (fixed 8 bytes).
// RETURN  : apiOK
//           apiFailed
// ------------------------------------------------------------
UCHAR	api_xped_MSKEY_GenPinBlock( UCHAR mode, UCHAR index, UCHAR *pan, UCHAR *epb )
{
UCHAR pin[PED_PIN_SLOT_LEN];
UCHAR temp[PED_PIN_SLOT_LEN];
UCHAR kek[16];


	// restore plaintext PIN data from SECM
	XPED_ReadRamDataKEY( ADDR_PEDS_PIN, PEDS_PIN_SLOT_LEN, pin );

	// encrypt plaintext PIN by using default KEK and save back to SRAM
	XPED_InitDefaultKEK( kek );
	PED_TripleDES( kek, 16, &pin[1], (UCHAR *)&temp[1] );
	temp[0] = pin[0];
	XPED_WriteRamDataKEY( ADDR_PEDS_PIN, PEDS_PIN_SLOT_LEN, temp );
        
        memset( pin, 0x00, sizeof(pin) );
        memset( kek, 0x00, sizeof(kek) );

	if( temp[0] != 0 )	// 2010-03-22
	  {
	  memset( temp, 0x00, sizeof( temp ) );
	  return( XPED_MSKEY_GenPinBlock( mode, index, pan, epb ) );
	  }
	else
	  return( apiFailed );	// no PIN
}

// ---------------------------------------------------------------------------
// FUNCTION: To show PIN entry message with ID number. (English & Chinese)
// INPUT   : id. (0..n)
//		0 = "LAST PIN TRY"
//		1 = "PIN ERROR" (INCORRECT PIN)
//		2 = "PIN_TRY_LIMIT_EXCEEDED"
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_xped_show_MsgID( UCHAR id )
{
UCHAR	result;
UCHAR	buf1[64];
UCHAR	buf2[64];


	if( id > 2 )
	  return( apiFailed );
	
	buf1[0] = 2;
	buf1[1] = 0;
	buf1[2] = 'Z';	// PED_CMD_SHOW_MSGID
	buf1[3] = id;
	
	// process command
	result = apiFailed;
	
	if( XPED_TransmitAUX( buf1 ) == TRUE )
	  {
	  if( XPED_ReceiveAUX( buf2 ) == TRUE )
	    {
	    if( (buf2[0] == 0x02) && (buf2[1] == 0x00) && (buf2[2] == buf1[2]) && (buf2[3] == 0x00) )
	      result = apiOK;
	    }
	  }
	  
	return( result );
}
