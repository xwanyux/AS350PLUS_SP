//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : APK_CVML.C                                                 **
//**  MODULE   : apk_CVM_PlaintextPIN()                                     **
//**		 apk_CVM_EncipheredPIN()				    **
//**		 apk_CVM_OnlineEncipheredPIN()				    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/07                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "POSAPI.h"
#include "GDATAEX.h"
#include "EMVAPI.h"
#include "EMVAPK.h"
#include "APDU.h"
#include "TOOLS.h"
#include "PINPAD.h"


// ---------------------------------------------------------------------------
// FUNCTION: Retrieve & Verify ICC PIN Enciphered Public Key by using
//           the Issuer Public Key.
// INPUT   : pkm - issuer public key modulus.  (n) -- LEN[2]+n[256] bytes
//           pke - issuer public key exponent. (e) -- e[3] integer
//           pkc - PIN enciphered public key certificate.
// OUTPUT  : pkm - the ICC PIN EPK (2L-V)
//           pke - the ICC PIN EPE (V)
// REF     : g_ibuf
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UCHAR CVM_RetrieveICC_PIN_EPK( UCHAR *pkm, UCHAR *pke, UCHAR *pkc )
{
UINT  i, j;
UINT  iLen;
UINT  iPKcLen;
UINT  iModLen;
UINT  iHashLen;
UINT  iLeftMostLen;
UCHAR temp[20];
UCHAR result;

// UI_ClearScreen();
// TL_DispHexByte(0,0,0xAA);
// UI_WaitKey();

      iPKcLen = pkc[1]*256 + pkc[0]; // length of the certificate
      iLeftMostLen = iPKcLen - 42; // length of "ICC PIN EPK" or "leftmost digits of the ICC PIN EPK"

      // Recover the certificate
      if( apk_LoadExternalPK( KEY_FID_WORK, pkm, pke ) != apkOK )
        return( FALSE );
      if( apk_RecoverPKC( pkm, pke, pkc ) != apkOK ) // pkc=the recovered data (2L-V)
        return( FALSE );

      iModLen = pkc[2+20-1]; // total length of "ICC PIN EPK"

      // Verification 2: check recovered data trailer
      if( pkc[ (iPKcLen+2)-1 ] != 0xBC )
        return( FALSE );

      // Verification 3: check recovered data header
      if( pkc[2+0] != 0x6A )
        return( FALSE );

      // Verification 4: check certificate format
      if( pkc[2+1] != 0x04 )
        return( FALSE );

      // Concatenation
      // (1) Recovered Data[2'nd..10'th] +
      // (2) ICC PIN Encipherment PK Remainder +
      // (3) ICC PIN Encipherment PK Exponent

      iHashLen = iPKcLen - 22; // header[1], HashResult[20], Trailer[1]
      for( i=0; i<iHashLen; i++ )
         g_ibuf[i] = pkc[i+3]; // from "Certificate Format" to "ICC Public Key"

      apk_ReadRamDataICC( ADDR_ICC_PIN_EPKR, 2, (UCHAR *)&iLen ); // load icc EPIN public key remainder length
      if( iLen != 0 )
        {
        apk_ReadRamDataICC( ADDR_ICC_PIN_EPKR+2, iLen, &g_ibuf[i] ); // cat icc EPIN public key remainder data
        i += iLen;
        iHashLen += iLen;
        }

      apk_ReadRamDataICC( ADDR_ICC_PIN_EPKE, 2, (UCHAR *)&iLen ); // load icc EPIN public key exponent length
      if( iLen == 0 ) // exponent present?
        {
        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] ); // PATCH: 2006-10-02

        return( FALSE );
        }

      apk_ReadRamDataICC( ADDR_ICC_PIN_EPKE+2, iLen, &g_ibuf[i] ); // cat icc EPIN public key exponent data
      i += iLen;
      iHashLen += iLen;

      // Verification 5: calculate & compare SHA1 result
      if( apk_HASH( SHA1, iHashLen, g_ibuf, temp ) == apkFailed )
        return( FALSE );

      for( i=0; i<20; i++ )
         {
         if( temp[i] != pkc[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
           return( FALSE );
         }

      // Verification 6: check Application PAN
      apk_ReadRamDataICC( ADDR_ICC_AP_PAN+2, 10, temp ); // load application PAN

//    if( TL_CNcmp( &pkc[2+2], temp, 10 ) == FALSE )
//      return( FALSE ); // PATCH: 2006-10-04
      result = TL_CNcmp( &pkc[2+2], temp, 10 );	// 2018-08-09, clear PAN working buffer after use
      memset( temp, 0x00, sizeof(temp) );	//
      if( result == FALSE )
      	return( FALSE );

      // Verification 7: check the certificate expiration date MMYY
      if( TL_VerifyCertificateExpDate( &pkc[2+12] ) == FALSE )
        return( FALSE );

      // Verification 8: check the icc PIN EPK algorithm indicator
      if( pkc[2+18] != 0x01 )
        return( FALSE );

      // ICC PIN EPK Modulus (stored in pkm[iModLen] array) = 2L-V
      //     (1) Leftmost Digits of the ICC PIN EPK +
      //     (2) ICC PIN EPK Remainder (if present)
      for( i=0; i<iLeftMostLen; i++ )
         pkm[i+2] = pkc[i+2+21];

      apk_ReadRamDataICC( ADDR_ICC_PIN_EPKR, 2, (UCHAR *)&iLen ); // load icc PIN EPK remainder length
      if( iLen != 0 )
        {
        apk_ReadRamDataICC( ADDR_ICC_PIN_EPKR+2, iLen, g_ibuf ); // load icc PIN EPK remainder data
        for( j=0; j<iLen; j++ )
           pkm[i+2+j] = g_ibuf[j];
        }

//    iModLen += iLen;
      pkm[0] = iModLen & 0x00ff;
      pkm[1] = (iModLen & 0xff00) >> 8;

      // icc ICC PIN EPK exponent (stored in pke[])
      apk_ReadRamDataICC( ADDR_ICC_PIN_EPKE, 5, g_ibuf ); // load icc PIN EPK exponent (2L-V)
      iLen = g_ibuf[1]*256 + g_ibuf[0];

      // the ICC PIN EPKE must be 2, 3 or 2^16+1 (value 2 will be removed in the future)
      switch( iLen )
            {
            case 0x0001:

                 if( (g_ibuf[2] != 2) && (g_ibuf[2] != 3) )
                   return( FALSE );
                 break;

            case 0x0003:

//               if( (g_ibuf[2] != 0x01) || (g_ibuf[3] != 0x00) || (g_ibuf[4] != 0x01) )

		 // 2016-12-07, 2CC.095.00.03, EXP=(00 00 03)
		 if( ((g_ibuf[2] == 0x01) && (g_ibuf[3] == 0x00) && (g_ibuf[4] == 0x01)) ||
		     ((g_ibuf[2] == 0x00) && (g_ibuf[3] == 0x00) && (g_ibuf[4] == 0x02)) ||
		     ((g_ibuf[2] == 0x00) && (g_ibuf[3] == 0x00) && (g_ibuf[4] == 0x03)) )
		   break;
		 else
                   return( FALSE );

            default:

                 return( FALSE );
            }

      pke[0] = 0;
      pke[1] = 0;
      pke[2] = 0;
      memmove( pke, &g_ibuf[2], iLen );

      return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Retrieve & Verify Public Key for ICC PIN Encipherment.
// INPUT   : none.
// OUTPUT  : pkm - the public key modulus  for ICC PIN Encipherment.
//           pke - the public key exponent for ICC PIN Encipherment.
// REF     : g_ibuf
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UCHAR CVM_RetrievePublicKey_EPIN( UCHAR *pkm, UCHAR *pke )
{
UINT  i;
UCHAR pkc[258];         // public key certificate (2L-V[256])
UINT  iLen1;
UINT  iLen2;


        // =======================================
        //  Enciphered PIN format
        // =======================================

        // 1. Check necessary data objects
        //    (1) RID (already here)

        //    (2) CA PKI
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
        apk_ReadRamDataICC( ADDR_ICC_CA_PKI, 2, (UCHAR *)&iLen1 );
        if( iLen1 == 0 )
          {
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
          g_term_TVR[0] |= TVR0_ICC_DATA_MISSING; // PATCH: 2006-10-02, EMV2000 V4.1a - 2CC.043.02
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );

          return( FALSE );
          }

        //    (3) Issuer PKC
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
        apk_ReadRamDataICC( ADDR_ICC_ISU_PKC, 2, (UCHAR *)&iLen1 );
        if( iLen1 == 0 )
          {
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
          g_term_TVR[0] |= TVR0_ICC_DATA_MISSING; // PATCH: 2006-10-02, EMV2000 V4.1a - 2CC.044.02
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );

          return( FALSE );
          }

        //    (4) Issuer PKR (optional)

        //    (5) Issuer PKE
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
        apk_ReadRamDataICC( ADDR_ICC_ISU_PKE, 2, (UCHAR *)&iLen1 );
        if( iLen1 == 0 )
          {
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
          g_term_TVR[0] |= TVR0_ICC_DATA_MISSING; // PATCH: 2006-10-02, EMV2000 V4.1a - 2CC.045.02
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );

          return( FALSE );
          }

        // --------------------------------------------------------
        //  Method 1 (using ICC PIN Encipherment Public Key)
        // --------------------------------------------------------
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
        apk_ReadRamDataICC( ADDR_ICC_PIN_EPKC, 2, (UCHAR *)&iLen1 ); // load icc EPIN PKC length
        apk_ReadRamDataICC( ADDR_ICC_PIN_EPKE, 2, (UCHAR *)&iLen2 ); // load icc EPIN PKE length

        if( (iLen1 != 0) && (iLen2 != 0) )
          {
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
          // Verification using ICC "PIN Enciphered PKC"
          apk_ReadRamDataICC( ADDR_ICC_PIN_EPKC, iLen1+2, pkc ); // load icc EPIN PKC (2L-V)

          apk_ReadRamDataICC( ADDR_ICC_ISU_PKM, iLen1+2, pkm ); // load issuer PKM (2L-n)
          if( (pkm[1]*256 + pkm[0]) == 0 )  // issuer PKM available?
            {
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
            if( api_emv_RetrievePublicKeyISU( pkm, pke ) != emvOK ) // retrive issuer public key (n,e)
              return( FALSE );	// PATCH: 2009-01-08, 2CC.014.03
            }
          else
            apk_ReadRamDataICC( ADDR_ICC_ISU_PKE+2, 3, pke );   // load issuer PKE (e)
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
	  // PATCH: 2009-03-07, BCTC EMV L2: 2CC.061.03
	  // sizeof(ICC_EPIN_PKC) == sizeof(ISSUER_PKM)?
	  if( iLen1 != (pkm[1]*256 + pkm[0]) )
	    return( FALSE );
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
          // Retrieve ICC "PIN Enciphered Public Key" using "Issuer Public Key (n,e)"
          if( CVM_RetrieveICC_PIN_EPK( pkm, pke, pkc ) == FALSE )
            {
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
            return( FALSE );
            }
          else
            {
printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
printf("\nMETHOD 1:");
printf("\nPKM:\n");
	for( i=0; i<(pkm[0]+pkm[1]*256); i++ )
   		printf("%02x ", pkm[2+i] );
   	    printf("\n");
printf("\nPKE:\n");
	for( i=0; i<3; i++ )
   		printf("%02x ", pke[i] );
   	    printf("\n");
            return( TRUE );
            }
          }

        // --------------------------------------------------------
        //  Method 2 (using ICC Public Key for DDA )
        // --------------------------------------------------------
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
        apk_ReadRamDataICC( ADDR_ICC_PKM, 2, (UCHAR *)&iLen1 ); // load icc public key modulus length
        apk_ReadRamDataICC( ADDR_ICC_PKE, 2, (UCHAR *)&iLen2 ); // load icc public key exponent length
        apk_ReadRamDataICC( ADDR_ICC_PKE+2, 3, pke ); // load icc public key exponent

        if( (iLen1 != 0) && (iLen2 != 0) )
          {
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
          // Verification using "ICC PKC"
          apk_ReadRamDataICC( ADDR_ICC_PKM, iLen1+2, pkm ); // load icc PKM (2L-V)
          apk_ReadRamDataICC( ADDR_ICC_PKE+2, 3, pke );     // load icc PKE (V)
          }
        else
          {
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
          // Retrieve ICC Public Key by using DDA
          if( apk_OFDA_BuildInputList( g_obuf ) == apkFailed )
            return( FALSE );
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
          if( api_emv_OfflineDDA( 1, g_obuf ) != emvOK ) // PATCH: 2006-10-04, retrieve ICC PK process
            return( FALSE ); // both methods are not satisfied for EPIN
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
          apk_ReadRamDataICC( ADDR_ICC_PKM, 2, (UCHAR *)&iLen1 ); // load icc public key modulus length
          apk_ReadRamDataICC( ADDR_ICC_PKE, 2, (UCHAR *)&iLen2 ); // load icc public key exponent length
          apk_ReadRamDataICC( ADDR_ICC_PKE+2, 3, pke ); // load icc public key exponent

          // Verification using "ICC PKC"
          apk_ReadRamDataICC( ADDR_ICC_PKM, iLen1+2, pkm ); // load icc PKM (2L-V)
          apk_ReadRamDataICC( ADDR_ICC_PKE+2, 3, pke );     // load icc PKE (V)
          }
printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
printf("\nMETHOD 2:");
printf("\nPKM:\n");
	for( i=0; i<(pkm[0]+pkm[1]*256); i++ )
   		printf("%02x ", pkm[2+i] );
   	    printf("\n");
printf("\nPKE:\n");
	for( i=0; i<3; i++ )
   		printf("%02x ", pke[i] );
   	    printf("\n");
   	    
        return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: issue VERIFY command to ICC for the current PIN entry.
// INPUT   : pindata - L-V, the plaintext pin in ASCII.
//           mode    - 0 = plaintext PIN format.
//                     1 = enciphered PIN format.
//           pkm     - public key modulus  (2L-V, available for mode1)
//           pke     - public key exponent (V,    available for mode1)
// OUTPUT  : none.
// REF     :
// RETURN  :  apiOK
//            apiNotReady     (try again)
//            apiFailed       (verify failed, set TVR.PIN_Try_Limit_exceeded=1)
//            apiErrorInput   (no CVMs satisfied)
//            apiOutOfService (invalid response or not supported)
// ---------------------------------------------------------------------------
UCHAR CVM_VerifyPIN( UCHAR mode, UCHAR *pindata, UCHAR *pkm, UCHAR *pke )
{
UINT  i, j;
UCHAR pinblock[16];
UCHAR buf[16];
UCHAR pkc[258];         // public key certificate (2L-V[256])
UCHAR *ptrdata;
UCHAR *ptrpindata;
UCHAR qualifier;
UINT  iLen1;
UINT  iLen2;


      ptrdata = pindata;

      // 1. Generate plaintext PIN block

//    PP_GenPinBlock( pinblock );
      PED_GenPinBlock( pinblock );	// 2014-10-29
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__); 
printf("\nMODE=%x\n", mode);
      if( mode == 0 )
        {
        ptrpindata = pinblock;
        qualifier = 0x80;
        iLen1 = 8;

        // =======================================
        //  Plaintext PIN format
        // =======================================

        // Verify PIN

VERIFY_PINDATA:
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__); 
	// -------------------------------------------------------------------
	// Here shows the PIN Data on display for PCI offline testing purpose
	// -------------------------------------------------------------------
	// UI_ClearScreen();      
	// UI_PutStr( 0, 0, FONT0, 9, "PIN DATA:" );
	// TL_DumpHexData( 0, 1, iLen1, ptrpindata );
	// -------------------------------------------------------------------

        if( apdu_VERIFY( qualifier, (UCHAR)iLen1, ptrpindata, buf ) == apiOK )
          {
          if( (buf[1]*256 + buf[0]) == 2 )
            {
            if( (buf[2] == 0x90) && (buf[3] == 0x00) )
              {
              PP_show_pin_ok(); // "PIN OK"
              
              TL_WaitTime(100);	// wait 1 sec
              return( apiOK );
              }

            if( (buf[2] == 0x69) && (buf[3] == 0x83) )
              {
              return( apiFailed );
              }

            if( (buf[2] == 0x69) && (buf[3] == 0x84) )
              {
              return( apiFailed );
              }

            if( (buf[2] == 0x63) && ((buf[3] & 0xf0) == 0xc0) )
              {
              if( (buf[3] & 0xcf) == 0xc0 )
                {
                return( apiFailed );
                }
              else
                {
                PP_show_incorrect_pin(); // "INCORRECT PIN"

                if( (buf[3] & 0xcf) == 0xc1 )
                  {
                  TL_WaitTimeAndKey(100);
                  PP_show_last_pin_try(); // "LAST PIN TRY"
                  }

                UI_WaitKey();

                return( apiNotReady); // try again
                }
              }
            else // unknown respose of VERIFY
              {
              return( apiOutOfService );
              }
            }
          else // illegal response length
            {
            return( apiOutOfService );
            }
          }
        else // no response
          {
          return( apiOutOfService );
          }
        }
      else
        {
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__); 
printf("\nPKM:\n");
	for( i=0; i<(pkm[0]+pkm[1]*256); i++ )
   		printf("%02x ", pkm[2+i] );
   	    printf("\n");
printf("\nPKE:\n");
	for( i=0; i<3; i++ )
   		printf("%02x ", pke[i] );
   	    printf("\n");
   	    
        // =======================================
        //  Enciphered PIN format
        // =======================================

EPIN_VERIFY:

        // 2. Terminal issues a "GET CHALLENGE" command to ICC
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__); 
        if( apk_GetChallengeICC( buf ) == apkFailed ) // buf[]: 2L-RAN[8]
          return( apiOutOfService );
        if( ((buf[1]*256 + buf[0]) != 10) || (buf[10] != 0x90) || (buf[11] != 0x00) )
       // return( apiOutOfService );
          return( apiErrorInput ); // PATCH: EMV2000

        // 3. Build "Enciphered PIN Data"
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__); 
        iLen1 = (pkm[1]*256 + pkm[0]) - 17; // sizeof( Random_Pad_Pattern[N-17] )

        i = iLen1 / 8;     // PATCH: EMV2000
        iLen2 = iLen1 % 8; // rem of mod 8
        iLen1 = i;         // quo of mod 8

        pkc[0] = pkm[0];
        pkc[1] = pkm[1];

        pkc[2] = 0x7F;                     // data header
        memmove( &pkc[2+1], pinblock, 8 ); // PIN block
        memmove( &pkc[2+9], &buf[2], 8 );  // unpredictable number

        qualifier = 1;
        for( i=0; i<iLen1; i++ )           // random pad pattern
           {
        // pkc[2+17+i] = i;

           // PATCH: EMV2000
           TL_GetRandomNumber( buf );
           for( j=0; j<8; j++ )
              {
              buf[j] *= qualifier++;
              buf[j] ^= (j*(i+1));
              }

           memmove( &pkc[2+17+i*8], buf, 8 );
           }

        if( iLen2 ) // PATCH: EMV2000
          {
          TL_GetRandomNumber( buf );
          memmove( &pkc[2+17+i*8], buf, iLen2 );
          }

        // 4. Generate the Enciphered PIN Data by using (pkm, pke) --
        //    "PIN Encipherment Public Key" or "ICC Public Key"
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__); 
        if( apk_LoadExternalPK( KEY_FID_WORK, pkm, pke ) != apkOK )
          return( apiOutOfService );
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__); 
        if( apk_RecoverPKC( pkm, pke, pkc ) != apkOK )
          return( apiOutOfService );
printf("SP@@@@@@@@@@@@%s %d\n",__func__,__LINE__); 
        ptrpindata = &pkc[2];
        iLen1 = pkc[1]*256 + pkc[0];
        qualifier = 0x88;

        // 5. Issue VERIFY command for Enciphered PIN
        goto VERIFY_PINDATA;

        }
}

// ---------------------------------------------------------------------------
// FUNCTION: CVM - Offline PIN Processing. (for plaintext or enciphered PIN)
// INPUT   : tout - time out for PIN entry in seconds. (RFU)
//           msg  - message to be shown on PIN PAD prior to enter PIN.
//                  if no PIN required or LEN=0, the following messages
//                  will be ignored. (generally, it's the total amount)
//                  Structure: ROW[1] COL[1] FONT[1] LEN[1] MSG1[20] <- 1'st
//                             ROW[1] COL[1] FONT[1] LEN[1] MSG2[20] <- 2'nd
//                             ROW[1] COL[1] FONT[1] LEN[1] MSG3[20] <- 3'rd
//                             ROW[1] COL[1] FONT[1] LEN[1] MSG4[20] <- 4'th
//           mode - 0 = plaintext PIN.
//                  1 = enciphered PIN.
// OUTPUT  : none.
// REF     :
// RETURN  : apkOK
//           apkNotReady     ( PIN entry bypassed )
//           apkFailed       ( PIN Try Limit exceeded --> try next CVM entry )
//           apkIncorrect    ( Enciphered PIN retrieved failed or incorrect )
//           apkDeviceError  ( PIN pad is malfuntioning )
//           apkOutOfService (terminate transaction)
// ---------------------------------------------------------------------------
UCHAR CVM_OfflinePIN( UINT tout, UCHAR *msg, UCHAR mode )
{
UCHAR pkm[258];         // public key modulus (2L-V[256])
UCHAR pke[3];           // public key exponent (0x02, 0x03, or 0x10001)
UCHAR buf[16];
UCHAR result;
UINT  iLen;


      if( mode == 1 ) // enciphered PIN
        {
        if( CVM_RetrievePublicKey_EPIN( pkm, pke ) == FALSE )
    //    return( apkFailed );
          return( apkIncorrect ); // PATCH: 2003-06-18,
                                  // the PTC exceeded bit shall not be set in this case.
        }

      // get "PIN Try Counter" from ICC ( 2L [9F17 1 CNT 90 00] )
      if( apdu_GET_DATA( 0x9f, 0x17, buf ) == apiFailed )
        // command not supported or device error --> get PIN
        goto GET_PIN;

      iLen = buf[1]*256 + buf[0];
      if( (iLen != 6) || (buf[2] != 0x9f) || (buf[3] != 0x17) ||
          (buf[6] != 0x90) || (buf[7] != 0x00) )
    	  goto GET_PIN;//08-dec-29 charles
/*
        {
        // PATCH: 2006-10-19, 2CM.013.00 & 2CM.013.01 for PTC LEN=0
        if( (buf[iLen] == 0x90) && (buf[iLen+1] == 0x00) )
//      if( (iLen == 5) && (buf[2] == 0x9f) && (buf[3] == 0x17) && (buf[4] == 0x00) && (buf[5] == 0x90) && (buf[6] == 0x00) )
          goto GET_PIN;

        // PATCH: 2006-10-02
        if( ((buf[iLen] == 0x6a) && (buf[iLen+1] == 0x81)) || ((buf[iLen] == 0x6a) && (buf[iLen+1] == 0x88)) )
          // "PIN Try Counter" is not retrievable --> get PIN
          goto GET_PIN;
        else
          return( apkOutOfService ); // invalid status word
        }
*/

      // check counter remaining value
      if( buf[5] == 0 )
        return( apkFailed ); // PIN Try Limit exceeded

GET_PIN:
      // show "message" from IFD to inform cardholder
      // to confirm it befor entering PIN.
      UI_ClearScreen();

      // get PIN, with TOTAL amount
      result = PP_GetPIN( tout, msg );

      if( result == apiDeviceError )
        return( apkDeviceError );     // pinpad malfunctioning

      if( result == apiOutOfService )
        return( apkOutOfService );    // timeout

      if( result == apiFailed )
        return( apkNotReady );        // PIN bypassed

      // show "PLEASE WAIT"
      PP_show_please_wait();

      // PIN ready, start to verify PIN
      result = CVM_VerifyPIN( mode, buf, pkm, pke );
      switch( result )
            {
            case apiOK:
                 return( apkOK );
                 break;

            case apiFailed:
                 return( apkFailed );
                 break;

            case apiNotReady: // try again
                 goto GET_PIN;

            case apiOutOfService:
                 return( apkOutOfService );

            case apiErrorInput: // PATCH: EMV2000, CASE: 2CC.100.01
                 return( apkIncorrect );
            }
}

// ---------------------------------------------------------------------------
// FUNCTION: CVM - Offline Plaintext PIN Processing.
// INPUT   : tout  - time out for PIN entry in seconds.
// OUTPUT  : none.
// REF     :
// RETURN  : apkOK
//           apkNotReady     ( PIN entry bypassed )
//           apkFailed       ( PIN Try Limit exceeded --> try next CVM entry )
//           apkOutOfService (terminate transaction)
// ---------------------------------------------------------------------------
UCHAR apk_CVM_PlaintextPIN( UINT tout, UCHAR *msg )
{
      return( CVM_OfflinePIN( tout, msg, 0 ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: CVM - Offline Enciphered PIN Processing.
// INPUT   : tout  - time out for PIN entry in seconds.
// OUTPUT  : none.
// REF     :
// RETURN  : apkOK
//           apkNotReady     ( PIN entry bypassed )
//           apkFailed       ( PIN Try Limit exceeded --> try next CVM entry )
//           apkOutOfService (terminate transaction)
// ---------------------------------------------------------------------------
UCHAR apk_CVM_EncipheredPIN( UINT tout, UCHAR *msg )
{

      // 1. Check & Retrieve PK for ICC PIN Encipherment
      // 2. Get PTC (PIN Try Counter)
      // 3. Get PIN
      // 4. Get CHALLENGE
      // 5. VERIFY

      return( CVM_OfflinePIN( tout, msg, 1 ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: CVM - Online Enciphered PIN Processing.
// INPUT   : tout      - time out for PIN entry in seconds. (RFU)
//           msg       - the total amount to be confirmed. (L-V)
// OUTPUT  : epb       - LL-V, the enciphered PIN block data for online (ISO 9564-1).
//                             if LL=0, no online PIN data. (fixed 8 bytes if available)
//	     ksn       - LL-V, key serial number for online.  (fixed 10 bytes if available)
// REF     :
// RETURN  : apkOK
//           apkNotReady     ( PIN entry bypassed )
//           apkFailed       ( PIN Try Limit exceeded --> try next CVM entry )
//           apkOutOfService (terminate transaction)
//
// REF     : ksn       - format2: V(10)
//                       fix 10-byte with leading hex "F's".
// ---------------------------------------------------------------------------
UCHAR apk_CVM_OnlineEncipheredPIN( UINT tout, UCHAR *msg, UCHAR *epb, UCHAR *ksn, UCHAR mod, UCHAR idx )
{
//UCHAR temp[16];
UCHAR bpan[12];
UCHAR pan[20];
UCHAR result;
UINT  iLen;


      // clear EPB & KSN data
      apk_ClearRamDataTERM( ADDR_TERM_EPIN_DATA, 10, 0x00 );
      apk_ClearRamDataTERM( ADDR_TERM_KSN, 12, 0x00 );
      
      memset( epb, 0x00, 10 );
      memset( ksn, 0x00, 12 );

      // get PIN, with TOTAL amount
      result = PP_GetPIN( tout, msg );

      if( result == apiDeviceError )
        return( apkDeviceError );     // pinpad malfunctioning

      if( result == apiOutOfService )
        return( apkOutOfService );    // timeout

      if( result == apiFailed )
        return( apkNotReady );        // PIN bypassed

      // get full PAN data
      apk_ReadRamDataICC( ADDR_ICC_AP_PAN, sizeof(bpan), bpan );  // bpan: LL-V(CN)
      TL_bcd2asc( bpan[0], &bpan[2], pan );	// pan: L-V(AN)
      
      result = PP_GenEncrypedPinBlock( pan, epb, ksn, mod, idx );
      memset( pan, 0x00, sizeof(pan) );		// 2018-08-09, clear PAN working buffer after use
      memset( bpan, 0x00, sizeof(bpan) );	//
      if( result == apiOK )
        {
        // save EPB & KSN for online transmission
        apk_WriteRamDataTERM( ADDR_TERM_EPIN_DATA, 10, epb );	// LL-V(8)
	apk_WriteRamDataTERM( ADDR_TERM_KSN, 12, ksn );		// LL-V(10)
	
	    iLen = epb[0]+epb[1]*256;
	    if( iLen != 0 )
	      {
	      // ----------------------------------------------------------------------------------
	      // Here shows the Encryped PIN Block (EPB) on display for PCI online testing purpose
	      // ----------------------------------------------------------------------------------
	    //   UI_ClearScreen();
	      
	    //   UI_PutStr( 0, 0, FONT0, 4, "EPB:" );
	    //   TL_DumpHexData( 0, 1, iLen, epb+2 );
	      }
	    
	    if( (ksn[0]+ksn[1]*256) != 0 )
	      {
	      // ----------------------------------------------------------------------------------
	      // Here shows the Key Serial Number (KSN) on display for PCI online testing purpose
	      // ----------------------------------------------------------------------------------
	    //   UI_PutStr( 4, 0, FONT0, 4, "KSN:" );
	    //   TL_DumpHexData( 0, 5, 10, ksn+2 );
	      }
	
	}
      
      return( result );
}
