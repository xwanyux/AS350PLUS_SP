//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : L2API_07.C                                                 **
//**  MODULE   : api_emv_OfflineDataAuthen()                                **
//**             api_emv_OfflineSDA()                                       **
//**             api_emv_OfflineDDA()                                       **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2003/01/03                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2003-2007 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "POSAPI.h"
//#include "EMVDC.h"
#include "GDATAEX.h"
#include "EMVAPI.h"
#include "EMVAPK.h"
//#include "UI.H"
#include "TOOLS.h"

// ---------------------------------------------------------------------------
// FUNCTION: offline Static Data Authentication.
// INPUT   : sdtba - 2L-V, static data to be authenticated.
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR api_emv_OfflineSDA( UCHAR *sdtba )
{
UCHAR capki;
UCHAR rid[RID_LEN];
UCHAR pkm[258];         // public key modulus (2L-V[256])
UCHAR pke[3];           // public key exponent (0x02, 0x03, or 0x10001)
UCHAR pkc[258];         // public key certificate (2L-V[256])
//UCHAR rpkc[258];        // recovered public key certificate (2L-V[256])
UCHAR temp[20];
UINT  iLen;
UINT  iHashLen;
UINT  iModLen;
UINT  iPKcLen;
UINT  iLeftMostLen;
UINT  i, j;

#ifdef  L2_SW_DEBUG
      g_test_flag = 0;
#endif

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x11);
// UI_WaitKey();

      // -------------------------------------------
      // Build the "static data to be authenticated"
      // -------------------------------------------
//    if( apk_OFDA_BuildInputList( g_obuf ) == apkFailed )
//      return( emvFailed );

      // check data objects required for SDA
// // if( apk_SDA_CheckRequiredDO() == apkFailed )
// //   return( emvFailed );

      // ---------------------------------
      // 1. Retrieval of the CA public key
      // ---------------------------------

      apk_ReadRamDataICC( ADDR_ICC_CA_PKI, 2, (UCHAR *)&iLen ); // load length of CA pki
      if( iLen == 0 ) // pki present?
        {
        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        return( emvFailed );
        }

      apk_ReadRamDataICC( ADDR_ICC_CA_PKI+2, 1, &capki ); // load value of CA pki
      apk_ReadRamDataICC( ADDR_SELECTED_AID+1, RID_LEN, rid );  // load RID
      if( apk_RetrievePublicKeyCA( capki, rid, pkm, pke ) != apkOK ) // load (n,e) = PK(rid, pki)
        return( emvFailed );

      // ------------------------------------------
      // 2. Retrieve & Verify the ISSUER public key
      // ------------------------------------------

      // Verification 1: check issuer PK certificate length = CA PK modulus
      apk_ReadRamDataICC( ADDR_ICC_ISU_PKC, 258, pkc ); // load issuer public key certificate

      iLen = pkc[1]*256 + pkc[0];
      if( iLen == 0 ) // issuer PKC present?
        {
        apk_ReleaseRSA(); // PATCH: 2009-01-07

        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        return( emvFailed );
        }
      if( iLen != (pkm[1]*256 + pkm[0]) )
        {
        apk_ReleaseRSA(); // PATCH: 2009-01-07
        return( emvFailed ); // length not equal to CA pkm
        }

      iPKcLen = iLen;      // store PKC length
      iLeftMostLen = iLen - 36; // length of "issuer PK" or "leftmost digits of the issuer PK"

      if( apk_RecoverIPKC( pkm, pke, pkc ) != apkOK ) // pkc=2L-V
        return( emvFailed );

      iModLen = pkc[2+14-1]; // total length of "issuer PK"

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x66);
// UI_WaitKey();
// TL_DumpHexData(0,0,256, pkc);

      // Verification 2: check recovered data trailer
      if( pkc[ (iPKcLen+2)-1 ] != 0xBC )
        return( emvFailed );

      // Verification 3: check recovered data header
      if( pkc[2+0] != 0x6A )
        return( emvFailed );

      // Verification 4: check certificate format
      if( pkc[2+1] != 0x02 )
        return( emvFailed );

      // Concatenation (Recovered Data[2'nd..10'th] + Remainder + Exponent)
      iHashLen = iPKcLen - 22;
      for( i=0; i<iHashLen; i++ )
         g_ibuf[i] = pkc[i+3]; // from "Certificate Format" to "Issuer Public Key"

      apk_ReadRamDataICC( ADDR_ICC_ISU_PKR, 2, (UCHAR *)&iLen ); // load issuer public key remainder length
      if( iLen != 0 )
        {
        apk_ReadRamDataICC( ADDR_ICC_ISU_PKR+2, iLen, &g_ibuf[i] ); // cat issuer public key remainder data
        i += iLen;
        iHashLen += iLen;
        }
      else
        {
        // Ni <= Nca - 36 ?
        if( iModLen > iLeftMostLen )
          {
          g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
          return( emvFailed );
          }
        }

      apk_ReadRamDataICC( ADDR_ICC_ISU_PKE, 2, (UCHAR *)&iLen ); // load issuer public key exponent length
      if( iLen == 0 ) // exponent present?
        {
        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        return( emvFailed );
        }

      apk_ReadRamDataICC( ADDR_ICC_ISU_PKE+2, iLen, &g_ibuf[i] ); // cat issuer public key exponent data
      iHashLen += iLen;

      // Verification 5: calculate & compare SHA1 result
      if( apk_HASH( SHA1, iHashLen, g_ibuf, temp ) == apkFailed )
        return( emvFailed );

      for( i=0; i<20; i++ )
         {
         if( temp[i] != pkc[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
           return( emvFailed );
         }

      // Verification 6: check Issuer ID Number (leftmost 3-8 digits)
      apk_ReadRamDataICC( ADDR_ICC_AP_PAN+2, 10, temp ); // load application PAN

      if( TL_CNcmp( &pkc[2+2], temp, 4 ) == FALSE )
        return( emvFailed );

// TL_DumpHexData(0,0,10, &pkc[2]);

      // Verification 7: check the certificate expiration date MMYY
      if( TL_VerifyCertificateExpDate( &pkc[2+6] ) == FALSE )
        {
// UI_ClearScreen();
// TL_DispHexByte(0,0,0xFF);
// UI_WaitKey();
// TL_DumpHexData(0,0,2, &pkc[8]);
        return( emvFailed );
        }

//    TL_GetDateTime( temp ); // get current date (temp[0..11]="YYMMDDhhmmss")
//    temp[12] = TL_SetCentury( temp[0] );
//    temp[13] = ((temp[0] & 0x0f) << 4) + (temp[1] & 0x0f); // convert to BCD
//    i = temp[12]*256 + temp[13]; // today year CCYY
//
//    temp[14] = TL_SetCentury( pkc[2+7] );
//    j = temp[14]*256 + pkc[2+7]; // expiry year CCYY
//
//    if( j < i )  // compare Year
//      return( emvFailed );
//
//    if( j == i ) // compare Month
//      {
//      temp[12] = ((temp[2] & 0x0f) << 4) + (temp[3] & 0x0f); // convert to BCD
//      if( pkc[2+6] < temp[12] ) // compare month
//        return( emvFailed );
//      }

      // Verification 8: RID + INDEX + Certificate Serial Number ???

      // Verification 9: check the issuer public key algorithm indicator
      if( pkc[2+12] != 0x01 )
        return( emvFailed );

      // Issuer Public Key Modulus (stored in pkm[iModLen] array) = 2L-V
      //        (1) Leftmost Digits of the Issuer Public Key +
      //        (2) Issuer Public Key Remainder (if present)
      for( i=0; i<iLeftMostLen; i++ )
         pkm[i+2] = pkc[i+2+15];

      apk_ReadRamDataICC( ADDR_ICC_ISU_PKR, 2, (UCHAR *)&iLen ); // load issuer public key remainder length
      if( iLen != 0 )
        {
        apk_ReadRamDataICC( ADDR_ICC_ISU_PKR+2, iLen, g_ibuf ); // load issuer public key remainder data
        for( j=0; j<iLen; j++ )
           pkm[i+2+j] = g_ibuf[j];
        }

//    iModLen += iLen;
      pkm[0] = iModLen & 0x00ff;
      pkm[1] = (iModLen & 0xff00) >> 8;

      // issuer public key exponent (stored in pke[])
      apk_ReadRamDataICC( ADDR_ICC_ISU_PKE+2, 3, pke ); // load issuer public key exponent data

      // backup complete issuer public key modulus
      apk_WriteRamDataICC( ADDR_ICC_ISU_PKM, pkm[1]*256+pkm[0]+2, pkm );

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x67);
// UI_WaitKey();
// TL_DumpHexData(0,0,3, pke);
// TL_DumpHexData(0,0,256, pkm);

      // -----------------------------------------------------
      // 3. Verification of the Signed Static Application Data
      // -----------------------------------------------------

#ifdef  L2_SW_DEBUG
      g_test_flag = 1;
#endif

      // Verification 1: check length of the SSAD = Issuer PKM
      apk_ReadRamDataICC( ADDR_ICC_SIGNED_SAD, 2, (UCHAR *)&iLen );

//  TL_DispHexWord(0,0,iLen);
//  TL_DispHexWord(1,0,iModLen);
//  UI_WaitKey();
//  apk_ReadRamDataICC( ADDR_ICC_SIGNED_SAD, iLen+2, pkc );
//  TL_DumpHexData(0,0,256,pkc);
//  for(;;);

      if( iLen == 0 ) // SSAD present?
        {
        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        return( emvFailed );
        }

      if( iLen != iModLen )
        return( emvFailed );
      apk_ReadRamDataICC( ADDR_ICC_SIGNED_SAD, iLen+2, pkc ); // 2L-V

      iPKcLen = iLen;      // store SSAD length

      if( apk_LoadExternalPK( KEY_FID_WORK, pkm, pke ) != apkOK )
        return( emvFailed );
      if( apk_RecoverPKC( pkm, pke, pkc ) != apkOK )
        return( emvFailed );

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x77);
// UI_WaitKey();
// TL_DumpHexData(0,0,256, pkc);

      // Verification 2: check recovered data trailer
      if( pkc[ (iPKcLen+2)-1 ] != 0xBC )
        return( emvFailed );

      // Verification 3: check recovered data header
      if( pkc[2+0] != 0x6A )
        return( emvFailed );

      // Verification 4: check signed format
      if( pkc[2+1] != 0x03 )
        return( emvFailed );

      // Concatenation (Recovered Data + "Static data to be authenticated")
      iHashLen = iPKcLen - 22;
      for( i=0; i<iHashLen; i++ )
         g_ibuf[i] = pkc[i+3]; // from "Signed Format" to "Pad Pattern"

      iLen = sdtba[1]*256 + sdtba[0]; // length of SDTBA
      memmove( &g_ibuf[i], &sdtba[2], iLen ); // cat SDTBA
      iHashLen += iLen;

      // PATCH: 2005/02/23, EMV2000: If SDATL is present and contains tags
      //        other than '82'(AIP), then SDA has failed
      apk_ReadRamDataICC( ADDR_ICC_SDA_TL, 3, temp ); // LEN[2] Tag1..TagN

      iLen = temp[1]*256 + temp[0];
      if( iLen != 0 )
        {
        if( (iLen != 1) || (temp[2] != 0x82) )
          return( emvFailed );
        }

      // Verification 5: calculate & compare SHA1 result
      if( apk_HASH( SHA1, iHashLen, g_ibuf, temp ) == apkFailed )
        return( emvFailed );

//  TL_DumpHexData(0,0,20, temp);
//  TL_DumpHexData(0,0,20, &pkc[iPKcLen-19]);

      for( i=0; i<20; i++ )
         {
         if( temp[i] != pkc[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
           return( emvFailed );
         }

      // Save "Data Authentication Code" (Tag=9F45)
      i = 2;
      api_emv_PutDataElement( DE_ICC, ADDR_ICC_DAC, 2, (UCHAR *)&i );
      api_emv_PutDataElement( DE_ICC, ADDR_ICC_DAC+2, 2, &pkc[2+3] );

// api_emv_GetDataElement( DE_ICC, ADDR_ICC_DAC, 4, temp );
// TL_DumpHexData(0,0,4, temp);

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x22);
// UI_WaitKey();

      return( emvOK ); // SDA done
}

// ---------------------------------------------------------------------------
// FUNCTION: offline Dynamic Data Authentication.
// INPUT   : sdtba - signed data to be authenticated.
//           type  - 0 = perform DDA
//                   1 = perform CDA or Retrieve ICC Public Key only for EPIN process
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed
//           emvOutOfService
// ---------------------------------------------------------------------------
UCHAR api_emv_OfflineDDA( UCHAR type, UCHAR *sdtba )
{
UCHAR capki;
UCHAR rid[RID_LEN];
UCHAR pkm[558];         // public key modulus (2L-V[256])
UCHAR pke[3];           // public key exponent (0x02, 0x03, or 0x10001)
UCHAR pkc[558];         // public key certificate (2L-V[256])
//UCHAR rpkc[258];        // recovered public key certificate (2L-V[256])
UCHAR temp[20];
UINT  iLen;
UINT  iHashLen;
UINT  iModLen;
UINT  iPKcLen;
UINT  iLeftMostLen;
UINT  i, j;
UCHAR cnt;
UCHAR *ptrobj;
UINT  totalLen;

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x33);
// UI_WaitKey();

// goto TC_2CL035;

      // ---------------------------------
      // 1. Retrieval of the CA public key
      // ---------------------------------

      apk_ReadRamDataICC( ADDR_ICC_CA_PKI, 2, (UCHAR *)&iLen ); // load length of CA pki
      if( iLen == 0 ) // pki present?
        {
        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        return( emvFailed );
        }

      apk_ReadRamDataICC( ADDR_ICC_CA_PKI+2, 1, &capki ); // load value of CA pki
      apk_ReadRamDataICC( ADDR_SELECTED_AID+1, RID_LEN, rid );  // load RID
      if( apk_RetrievePublicKeyCA( capki, rid, pkm, pke ) != apkOK ) // load (n,e) = PK(rid, pki)
        return( emvFailed );

// TL_DispHexByte(0,0,capki);
// TL_DumpHexData(0,1,RID_LEN,rid);
//
// TL_DumpHexData(0,2,3,pke);
// TL_DumpHexData(0,3,254,pkm);
// TL_DispHexWord(0,0,g_key_fid);
// UI_WaitKey();

      // ------------------------------------------
      // 2. Retrieve & Verify the ISSUER public key
      // ------------------------------------------

      // Verification 1: check issuer PK certificate length = CA PK modulus
      apk_ReadRamDataICC( ADDR_ICC_ISU_PKC, 258, pkc ); // load issuer public key certificate

      iLen = pkc[1]*256 + pkc[0];
      if( iLen == 0 ) // issuer PKC present?
        {
        apk_ReleaseRSA(); // PATCH: 2009-01-07

        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        return( emvFailed );
        }

      if( iLen != (pkm[1]*256 + pkm[0]) )
        {
        apk_ReleaseRSA(); // PATCH: 2009-01-07
        return( emvFailed ); // length not equal to CA pkm
        }

      iPKcLen = iLen;      // store PKC length
      iLeftMostLen = iLen - 36; // length of "issuer PK" or "leftmost digits of the issuer PK"

      if( apk_RecoverIPKC( pkm, pke, pkc ) != apkOK ) // pkc=2L-V
        return( emvFailed );

      iModLen = pkc[2+14-1]; // total length of "issuer PK"

// TL_OpenAUX();
// UI_ClearScreen();
// TL_DispHexByte(0,0,0x81);
// UI_WaitKey();
// TL_DumpHexData(0,0,256, pkc);
// TL_TransmitAUX( pkc );
// UI_WaitKey();

      // Verification 2: check recovered data trailer
      if( pkc[ (iPKcLen+2)-1 ] != 0xBC )
        return( emvFailed );

      // Verification 3: check recovered data header
      if( pkc[2+0] != 0x6A )
        return( emvFailed );

      // Verification 4: check certificate format
      if( pkc[2+1] != 0x02 )
        return( emvFailed );

      // Concatenation
      // (1) Recovered Data[2'nd..10'th] +
      // (2) Issuer Remainder +
      // (3) Issuer Exponent
      iHashLen = iPKcLen - 22; // header[1], HashResult[20], Trailer[1]
      for( i=0; i<iHashLen; i++ )
         g_ibuf[i] = pkc[i+3]; // from "Certificate Format" to "Issuer Public Key"

      apk_ReadRamDataICC( ADDR_ICC_ISU_PKR, 2, (UCHAR *)&iLen ); // load issuer public key remainder length
      if( iLen != 0 )
        {
        apk_ReadRamDataICC( ADDR_ICC_ISU_PKR+2, iLen, &g_ibuf[i] ); // cat issuer public key remainder data
        i += iLen;
        iHashLen += iLen;
        }
      else
        {
        // Ni <= Nca - 36 ?
        if( iModLen > iLeftMostLen )
          {
          g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
          return( emvFailed );
          }
        }

      apk_ReadRamDataICC( ADDR_ICC_ISU_PKE, 2, (UCHAR *)&iLen ); // load issuer public key exponent length
      if( iLen == 0 ) // exponent present?
        {
        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        return( emvFailed );
        }

      apk_ReadRamDataICC( ADDR_ICC_ISU_PKE+2, iLen, &g_ibuf[i] ); // cat issuer public key exponent data
      iHashLen += iLen;


// UI_ClearScreen();
// TL_DispHexByte(0,0,0x55);
// UI_WaitKey();
// TL_OpenAUX();
// memmove( &pkc[2], g_ibuf, 200 );
// pkc[0] = 200;
// pkc[1] = 0;
// TL_TransmitAUX( pkc );
// UI_WaitKey();
//
// memmove( &pkc[2], &g_ibuf[200], 61 );
// pkc[0] = 61;
// pkc[1] = 0;
// TL_TransmitAUX( pkc );
// UI_WaitKey();


      // Verification 5: calculate & compare SHA1 result
      if( apk_HASH( SHA1, iHashLen, g_ibuf, temp ) == apkFailed )
        return( emvFailed );

// pkc[0] = 20;
// pkc[1] = 0;
// memmove( &pkc[2], temp, 20 );
// TL_TransmitAUX( pkc );
// UI_WaitKey();


      for( i=0; i<20; i++ )
         {
         if( temp[i] != pkc[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
           return( emvFailed );
         }

      // Verification 6: check Issuer ID Number (leftmost 3-8 digits)
      apk_ReadRamDataICC( ADDR_ICC_AP_PAN+2, 10, temp ); // load application PAN

      if( TL_CNcmp( &pkc[2+2], temp, 4 ) == FALSE )
        return( emvFailed );

// TL_DumpHexData(0,0,10, &pkc[2]);

      // Verification 7: check the certificate expiration date MMYY
      if( TL_VerifyCertificateExpDate( &pkc[2+6] ) == FALSE )
        {
// UI_ClearScreen();
// TL_DispHexByte(0,0,0xFF);
// UI_WaitKey();
// TL_DumpHexData(0,0,2, &pkc[8]);

        return( emvFailed );
        }

//    TL_GetDateTime( temp ); // get current date (temp[0..11]="YYMMDDhhmmss")
//    temp[12] = TL_SetCentury( temp[0] );
//    temp[13] = ((temp[0] & 0x0f) << 4) + (temp[1] & 0x0f); // convert to BCD
//    i = temp[12]*256 + temp[13]; // today year CCYY
//
//    temp[14] = TL_SetCentury( pkc[2+7] );
//    j = temp[14]*256 + pkc[2+7]; // expiry year CCYY
//
//    if( j < i )  // compare Year
//      return( emvFailed );
//
//    if( j == i ) // compare Month
//      {
//      temp[12] = ((temp[2] & 0x0f) << 4) + (temp[3] & 0x0f); // convert to BCD
//      if( pkc[2+6] < temp[12] ) // compare month
//        return( emvFailed );
//      }

      // Verification 8: RID + INDEX + Certificate Serial Number ???

      // Verification 9: check the issuer public key algorithm indicator
      if( pkc[2+12] != 0x01 )
        return( emvFailed );

      // Issuer Public Key Modulus (stored in pkm[iModLen] array) = 2L-V
      //        (1) Leftmost Digits of the Issuer Public Key +
      //        (2) Issuer Public Key Remainder (if present)
      for( i=0; i<iLeftMostLen; i++ )
         pkm[i+2] = pkc[i+2+15];

      apk_ReadRamDataICC( ADDR_ICC_ISU_PKR, 2, (UCHAR *)&iLen ); // load issuer public key remainder length
      if( iLen != 0 )
        {
        apk_ReadRamDataICC( ADDR_ICC_ISU_PKR+2, iLen, g_ibuf ); // load issuer public key remainder data
        for( j=0; j<iLen; j++ )
           pkm[i+2+j] = g_ibuf[j];
        }

//    iModLen += iLen;
      pkm[0] = iModLen & 0x00ff;
      pkm[1] = (iModLen & 0xff00) >> 8;

      // issuer public key exponent (stored in pke[])
      apk_ReadRamDataICC( ADDR_ICC_ISU_PKE+2, 3, pke ); // load issuer public key exponent data

      // backup complete issuer public key modulus
      apk_WriteRamDataICC( ADDR_ICC_ISU_PKM, pkm[1]*256+pkm[0]+2, pkm );

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x82);
// UI_WaitKey();
// TL_DumpHexData(0,1,3, pke);
// TL_DumpHexData(0,2,pkm[0]+pkm[1]*256+2, pkm);

      // ---------------------------------------
      // 3. Retrieve & Verify the ICC public key
      // ---------------------------------------

      // Verification 1: check ICC PK certificate length = Issuer PK modulus
      apk_ReadRamDataICC( ADDR_ICC_PKC, 258, pkc ); // load issuer public key certificate

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x83);
// UI_WaitKey();
// TL_DumpHexData(0,0,256, pkc);

      iLen = pkc[1]*256 + pkc[0];
      if( iLen == 0 ) // certificate present?
        {
        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        return( emvFailed );
        }
      if( iLen != (pkm[1]*256 + pkm[0]) )
        return( emvFailed ); // length not equal to CA pkm

      iPKcLen = iLen;      // store PKC length
      iLeftMostLen = iLen - 42; // length of "ICC PK" or "leftmost digits of the ICC PK"

      if( apk_LoadExternalPK( KEY_FID_WORK, pkm, pke ) != apkOK )
        return( emvFailed );
      if( apk_RecoverPKC( pkm, pke, pkc ) != apkOK ) // pkc=2L-V
        return( emvFailed );

      iModLen = pkc[2+20-1]; // total length of "ICC PK"

// UI_ClearScreen();
// TL_DispHexByte(0,0,0xEE);
// UI_WaitKey();
// TL_DispHexWord(0,0,iModLen);
// TL_DumpHexData(0,1,256, pkc);

      // Verification 2: check recovered data trailer
      if( pkc[ (iPKcLen+2)-1 ] != 0xBC )
        return( emvFailed );

      // Verification 3: check recovered data header
      if( pkc[2+0] != 0x6A )
        return( emvFailed );

      // Verification 4: check certificate format
      if( pkc[2+1] != 0x04 )
        return( emvFailed );

      // Concatenation
      // (1) Recovered Data[2'nd..10'th] +
      // (2) ICC Remainder +
      // (3) ICC Exponent +
      // (4) Static data to be authenticated
      iHashLen = iPKcLen - 22; // header[1], HashResult[20], Trailer[1]
      for( i=0; i<iHashLen; i++ )
         g_ibuf[i] = pkc[i+3]; // from "Certificate Format" to "ICC Public Key"

      apk_ReadRamDataICC( ADDR_ICC_PKR, 2, (UCHAR *)&iLen ); // load icc public key remainder length
      if( iLen != 0 )
        {
        apk_ReadRamDataICC( ADDR_ICC_PKR+2, iLen, &g_ibuf[i] ); // cat icc public key remainder data
        i += iLen;
        iHashLen += iLen;
        }
      else
        {
        // Nic <= Ni - 42 ?
        if( iModLen > iLeftMostLen )
          {
          g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
          return( emvFailed );
          }
        }

      apk_ReadRamDataICC( ADDR_ICC_PKE, 2, (UCHAR *)&iLen ); // load icc public key exponent length
      if( iLen == 0 ) // exponent present?
        {
        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        return( emvFailed );
        }

      apk_ReadRamDataICC( ADDR_ICC_PKE+2, iLen, &g_ibuf[i] ); // cat icc public key exponent data
      i += iLen;
      iHashLen += iLen;

      iLen = sdtba[1]*256 + sdtba[0]; // length of SDTBA
      memmove( &g_ibuf[i], &sdtba[2], iLen ); // cat SDTBA
      iHashLen += iLen;

      // PATCH: 2005/02/23, EMV2000: If SDATL is present and contains tags
      //        other than '82'(AIP), then SDA has failed
      apk_ReadRamDataICC( ADDR_ICC_SDA_TL, 3, temp ); // LEN[2] Tag1..TagN
      iLen = temp[1]*256 + temp[0];
      if( iLen != 0 )
        {
        if( (iLen != 1) || (temp[2] != 0x82) )
          return( emvFailed );
        }

      // Verification 5: calculate & compare SHA1 result
      if( apk_HASH( SHA1, iHashLen, g_ibuf, temp ) == apkFailed )
        return( emvFailed );

      for( i=0; i<20; i++ )
         {
         if( temp[i] != pkc[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
           return( emvFailed );
         }

      // Verification 6: check Application PAN
      memset( temp, 0xff, 10 ); // padded with 'F'
      apk_ReadRamDataICC( ADDR_ICC_AP_PAN, 2, (UCHAR *)&iLen ); // load application PAN LEN
      apk_ReadRamDataICC( ADDR_ICC_AP_PAN+2, iLen, temp );      // load application PAN

      if( TL_CNcmp2( &pkc[2+2], temp, 10 ) == FALSE )
        return( emvFailed );

// TL_DumpHexData(0,2,14, &pkc[2]);

      // Verification 7: check the certificate expiration date MMYY
      if( TL_VerifyCertificateExpDate( &pkc[2+12] ) == FALSE )
        {
// UI_ClearScreen();
// TL_DispHexByte(0,0,0xFF);
// UI_WaitKey();
// TL_DumpHexData(0,0,2, &pkc[14]);

        return( emvFailed );
        }

//    TL_GetDateTime( temp ); // get current date (temp[0..11]="YYMMDDhhmmss")
//
//    temp[12] = ((temp[0] & 0x0f) << 4) + (temp[1] & 0x0f); // convert to BCD
//    if( pkc[2+13] < temp[12] ) // year
//      return( emvFailed );
//
//    temp[12] = ((temp[2] & 0x0f) << 4) + (temp[3] & 0x0f); // convert to BCD
//    if( pkc[2+12] < temp[12] ) // month
//      return( emvFailed );

      // Verification 8: check the icc public key algorithm indicator
      if( pkc[2+18] != 0x01 )
        return( emvFailed );

      // ICC Public Key Modulus (stored in pkm[iModLen] array) = 2L-V
      //     (1) Leftmost Digits of the ICC Public Key +
      //     (2) ICC Public Key Remainder (if present)
      for( i=0; i<iLeftMostLen; i++ )
         pkm[i+2] = pkc[i+2+21];

      apk_ReadRamDataICC( ADDR_ICC_PKR, 2, (UCHAR *)&iLen ); // load icc public key remainder length
      if( iLen != 0 )
        {
        apk_ReadRamDataICC( ADDR_ICC_PKR+2, iLen, g_ibuf ); // load icc public key remainder data
        for( j=0; j<iLen; j++ )
           pkm[i+2+j] = g_ibuf[j];
        }

//    iModLen += iLen;
      pkm[0] = iModLen & 0x00ff;
      pkm[1] = (iModLen & 0xff00) >> 8;

      // icc public key exponent (stored in pke[])
      apk_ReadRamDataICC( ADDR_ICC_PKE+2, 3, pke ); // load icc public key exponent data

      // backup complete icc public key modulus
      apk_WriteRamDataICC( ADDR_ICC_PKM, pkm[1]*256+pkm[0]+2, pkm );

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x84);
// TL_DispHexWord(1,0,iModLen);
// UI_WaitKey();
// TL_DumpHexData(0,1,3, pke);
// TL_DumpHexData(0,0,256, pkm);

      if( type == 1 )     // EMV2000: perform CDA ? or Retrieve ICC PK only?
        return( emvOK );  // yes, ICC PK is retrieved OK

      // -------------------------------
      // 4. Dynamic Signature Geneartion
      // -------------------------------

      // check the source of DDOL
      apk_ReadRamDataICC( ADDR_ICC_DDOL, 2, (UCHAR *)&iLen ); // load icc DDOL length
      if( iLen != 0 )   // using icc DDOL
        apk_ReadRamDataICC( ADDR_ICC_DDOL, iLen+2, pkc ); // load icc DDOL value
      else
        {
        // PATCH: PBOC20, 2006-02-08, remove the missing flag
//      g_term_TVR[0] |= TVR0_ICC_DATA_MISSING; // AIP indicating DDA but DDOL is not present in ICC
//      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );

        apk_ReadRamDataTERM( ADDR_TERM_DDOL, 2, (UCHAR *)&iLen ); // load term DDOL length
        if( iLen != 0 ) // using terminal DDOL
          apk_ReadRamDataTERM( ADDR_TERM_DDOL, iLen+2, pkc ); // load term DDOL to pkc[]=2L-V
        else
          return( emvNotReady ); // both icc & terminal do not contain a DDOL
        }

      if( pkc[1] != 0 )
        return( emvFailed ); // out of spec (ddol max. 252 bytes)

      // check DDOL contains the "Unpredictable Number" (Tag=9F37)
      pkc[1] = pkc[0]; // convert to berLEN type 0x81-L-V
      pkc[0] = 0x81;

      if( apk_FindTagDOL( 0x9f, 0x37, pkc ) == FALSE ) // find tag=9F37
        return( emvFailed );

      if( apk_GetChallengeTERM( temp ) == apkFailed ) // generate random nbr
        return( emvFailed );

//    apk_WriteRamDataTERM( ADDR_TERM_UPD_NBR, 4, temp ); // save random nbr

      // internal authentication
      api_emv_ConcatenateDOL( DOL_DDOL, pkc, g_obuf ); // g_obuf=concatenation of DDOL
      cnt = apk_InternalAuthen( g_obuf, g_ibuf ); // g_ibuf=icc SDAD
      if( cnt == apkNotReady ) // PATCH: 2006-10-04, SDAD not present
        return( apiFailed );
      if( cnt  != apkOK )
        return( emvOutOfService );

      // g_ibuf: the Signed Dynamic Application Data (SDAD)
      //         (1) primitive  : 2L 80-L-SDAD[] SW1 SW2
      //             or
      //         (2) constructed: 2L 77-L-[...(9F4B-L-V)....] SW1 SW2

      // check response format (1)
      if( g_ibuf[2] == 0x80 )
        {
        iLen = apk_GetBERLEN( &g_ibuf[3], &cnt ); // iLen = SDAD length
        ptrobj = &g_ibuf[3] + cnt; // ptrobj = pointer to SDAD

        // PATCH: PBOC2.0, 2006-02-11, TC_2CL_035
        if( ((g_ibuf[0]+g_ibuf[1]*256) - 2) != (iLen + cnt + 1) )
          {
//  TL_DispHexByte(0,0,0x11);
//  UI_WaitKey();
          return( emvOutOfService );
          }

//  TL_DispHexByte(0,0,0x22);
//  UI_WaitKey();
//
//  TL_DispHexByte(0,0,cnt);
//  TL_DispHexWord(1,0, iLen);
//  TL_DumpHexData(0,2,iLen, ptrobj);
        }
      else
        {
        // check response format (2)
        if( g_ibuf[2] == 0x77 )
          {
          i = apk_GetBERLEN( &g_ibuf[3], &cnt ); // i = template length
          if( i == 0 )
            return( emvOutOfService );
          totalLen = g_ibuf[0]+g_ibuf[1]*256;
          //total length -tag77(1) - cnt(?) - SW(2) = i
          if((totalLen-1-cnt-2)!=i)//EMV 42a Charles 2009-03-11 2CL.035.00 case 1
        	  return emvOutOfService;
          // find SDAD (Tag=9F4B)
          ptrobj = apk_FindTag( 0x9f, 0x4b, &g_ibuf[2] );
          if( ptrobj != 0 )
            {
            iLen = apk_GetBERLEN( ptrobj, &cnt ); // iLen = SDAD length
//  TL_DispHexWord(0,0,i);
//  TL_DispHexWord(1,0,iLen);
//  TL_DispHexWord(2,0,iLen+2+cnt);
//  TL_DispHexWord(3,0,iModLen);
//  UI_WaitKey();
            if( iLen == 0 )
              return( emvOutOfService );

            if( (iLen + 2 + cnt) > i )//if( (iLen + 2 + cnt) != i ) // check redundancy //EMV 42a 2009-03-05 Charles 2ca.084.01 case 1,2
            {

   /*         	unsigned long tmpLen = i-(iLen+2+cnt);
            	unsigned long ii;
            	unsigned char *temp = ptrobj+cnt+iLen;
            	for(ii=0;ii<tmpLen;ii++)
            	{//check pading
            		if((temp[ii]!=0)&&(temp[ii]!=0xff))
            			break;
            	}
            	if(ii<tmpLen)*/
            		return( emvOutOfService );
            }


            ptrobj += cnt; // ptrobj = pointer to SDAD

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x86);
// TL_DispHexWord(1,0,iLen);
// UI_WaitKey();
// TL_DumpHexData(0,0, 256, ptrobj);

            }
          else
            return( emvFailed ); // SDAD not present
          }
        else
          return( emvFailed ); // unknown SDAD tag
        }

      // ---------------------------------
      // 5. Dynamic Signature Verification
      // ---------------------------------

      // Verification 1: check SDAD length = ICC PK modulus
      if( iLen != iModLen )
        return( emvFailed );

      iPKcLen = iLen;      // store PKC length

      pkc[0] = iLen & 0x00ff;           // setup dynamic signature "pkc"
      pkc[1] = (iLen & 0xff00) >> 8;    // to 2L-V
      memmove( &pkc[2], ptrobj, iLen ); //

      if( apk_LoadExternalPK( KEY_FID_WORK, pkm, pke ) != apkOK )
        return( emvFailed );
      if( apk_RecoverPKC( pkm, pke, pkc ) != apkOK ) // pkc=2L-V
        return( emvFailed );

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x87);
// UI_WaitKey();
// TL_DumpHexData(0,0, 256, pkc);

      // Verification 2: check recovered data trailer
      if( pkc[ (iPKcLen+2)-1 ] != 0xBC )
        return( emvFailed );

      // Verification 3: check recovered data header
      if( pkc[2+0] != 0x6A )
        return( emvFailed );

      // Verification 4: check signed data format
      if( pkc[2+1] != 0x05 )
        return( emvFailed );

      // Concatenation
      // (1) Recovered Data[2'nd..6'th] +
      // (2) DDOL data elements
      iHashLen = iPKcLen - 22; // header[1], HashResult[20], Trailer[1]
      for( i=0; i<iHashLen; i++ )
         g_ibuf[i] = pkc[i+3]; // from "Signed Data Format" to "Pad pattern"

      memmove( &g_ibuf[i], &g_obuf[1], g_obuf[0] ); // cat DDOL
      iHashLen += g_obuf[0];

      // Verification 5: calculate & compare SHA1 result
      if( apk_HASH( SHA1, iHashLen, g_ibuf, temp ) == apkFailed )
        return( emvFailed );

      for( i=0; i<20; i++ )
         {
         if( temp[i] != pkc[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
           return( emvFailed );
         }

      // Save "ICC Dynamic Number" (Tag=9F4C)
      i = pkc[2+4];            // length
      if( (i > 1) && (i < 9) ) // valid only 2~8 bytes
        {
        api_emv_PutDataElement( DE_ICC, ADDR_ICC_DYNAMIC_NBR, 2, (UCHAR *)&i );
        api_emv_PutDataElement( DE_ICC, ADDR_ICC_DYNAMIC_NBR+2, i, &pkc[2+5] );
        }

// api_emv_GetDataElement( DE_ICC, ADDR_ICC_DYNAMIC_NBR, 10, temp );
// TL_DumpHexData(0,0,10, temp);

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x44);
// UI_WaitKey();

      return( emvOK ); // DDA done
}

// ---------------------------------------------------------------------------
// FUNCTION: offline Combined DDA/Application Cryptogram Generation (CDA)
//           used to recover the Signed Dynamic Application Data (SDAD).
// PREREQ  : ICC Public Key has successfully retrieved.
// INPUT   : phase - 1 = for the 1'st GENERATE AC
//                   2 = for the 2'nd GENERATE AC
//           gac   - the response of GENERATE AC (2L 77-L-V)
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed
//           emvAborted (if CID of LT(ARQC) and RECOVER(TC))
// ---------------------------------------------------------------------------
UCHAR api_emv_OfflineCDA( UCHAR phase, UCHAR *gac )
{
UCHAR pkm[258];         // public key modulus (2L-V[256])
UCHAR pke[3];           // public key exponent (0x02, 0x03, or 0x10001)
UCHAR pkc[258];         // public key certificate (2L-V[256])
UCHAR temp[20];
UINT  iLen;
UINT  iLen1;
UINT  iLen2;
UINT  iHashLen;
UINT  iModLen;
UINT  iPKcLen;
UINT  i, j;
UCHAR cnt;
UCHAR *ptrobj;
UCHAR *ptrfirst;

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x55);
// UI_WaitKey();

      // ---------------------------------
      // 5. Dynamic Signature Verification
      // ---------------------------------

      // read complete icc public key modulus (2L-V) & exponent
      apk_ReadRamDataICC( ADDR_ICC_PKM, 258, pkm ); // modulus
      iModLen = pkm[1]*256+pkm[0];

      apk_ReadRamDataICC( ADDR_ICC_PKE+2, 3, pke ); // exponent

      // read the SDAD (2L-V)
      apk_ReadRamDataICC( ADDR_ICC_SIGNED_DAD, 258, pkc );
      iLen = pkc[1]*256+pkc[0];

      // Verification 1: check SDAD length = ICC PK modulus
      if( iLen != iModLen )
        return( emvFailed );

      iPKcLen = iLen;      // store PKC length

      // recover SDAD using ICC PK
      if( apk_LoadExternalPK( KEY_FID_WORK, pkm, pke ) != apkOK )
        return( emvFailed );
      if( apk_RecoverPKC( pkm, pke, pkc ) != apkOK ) // pkc=2L-V
        return( emvFailed );

// TL_DispHexByte(0,0,0x11);
// UI_WaitKey();

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x87);
// UI_WaitKey();
// TL_DumpHexData(0,0, 256, pkc);

      // Verification 2: check recovered data trailer
      if( pkc[ (iPKcLen+2)-1 ] != 0xBC )
        return( emvFailed );

// TL_DispHexByte(0,0,0x22);
// UI_WaitKey();

      // Verification 3: check recovered data header
      if( pkc[2+0] != 0x6A )
        return( emvFailed );

// TL_DispHexByte(0,0,0x33);
// UI_WaitKey();

      // Verification 4: check signed data format
      if( pkc[2+1] != 0x05 )
        return( emvFailed );

// TL_DispHexByte(0,0,0x44);
// UI_WaitKey();

      // retrieve leftmost bytes of ICC Dynamic Data
      i = pkc[2+4]; // icc dynamic number length
      i = i + 5;    // offset to CID
      j = i + 9;    // offset to TDHC (Transaction Data Hash Code)

      // Verification 6: check CID
      apk_ReadRamDataICC( ADDR_ICC_CID+2, 1, (UCHAR *)&cnt );

//TL_DispHexByte(0,0,cnt);
//TL_DispHexByte(1,0,pkc[2+i]);
//UI_WaitKey();

      if( pkc[2+i] != cnt )
        {
        if( (cnt == AC_TC ) && ( pkc[2+i] == AC_ARQC ) )
          return( emvAborted ); // to generate 2nd AC

        if( (cnt == AC_ARQC ) && ( pkc[2+i] == AC_TC ) )
          return( emvAborted ); // to deactivate
        else
          return( emvFailed );
        }

// TL_DispHexByte(0,0,0x55);
// UI_WaitKey();

//TL_DispHexByte(2,0,0x55);
//UI_WaitKey();

      // Concatenation
      // (1) Recovered Data[2'nd..6'th] +
      // (2) Unpredictable Number.

      //20090307_Richard: EMV 4.2a(BCTC), 2CC.133.01, CDA flow change
      //                  prevent the case that the hash result verification succeeds
      //                  but the terminal doesn't issue unpredictable number

      if( ( Check_UnpredictNumInCDOL1() == FALSE ) && ( Check_UnpredictNumInCDOL2() == FALSE ) )
        {
          return( emvFailed );
        }


      iHashLen = iPKcLen - 22; // header[1], HashResult[20], Trailer[1]
      for( i=0; i<iHashLen; i++ )
         g_ibuf[i] = pkc[i+3]; // from "Signed Data Format" to "Pad pattern"

      apk_ReadRamDataTERM( ADDR_TERM_UPD_NBR, 4, g_obuf );

      memmove( &g_ibuf[i], g_obuf, 4 ); // cat Unpredictable Number (tag=9F37)
      iHashLen += 4;

// TL_DispHexWord(0,0,iHashLen);
// TL_DumpHexData(0,1,iHashLen, g_ibuf);

      // Verification 8: calculate & compare SHA1 result
      if( apk_HASH( SHA1, iHashLen, g_ibuf, temp ) == apkFailed )
        return( emvFailed );

// TL_DispHexByte(0,0,0x66);
// UI_WaitKey();

// TL_DumpHexData(0,0,20, temp);

      for( i=0; i<20; i++ )
         {
         if( temp[i] != pkc[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
           return( emvFailed );
         }

// TL_DispHexByte(0,0,0x77);
// UI_WaitKey();

// api_emv_GetDataElement( DE_ICC, ADDR_ICC_DYNAMIC_NBR, 10, temp );
// TL_DumpHexData(0,0,10, temp);

      // Concatenation
      // (1)   PDOL data elements (exclude 83-L)
      // (2.1) CDOL1 data elements - in the case of the 1'st GENERATE AC (exclude 77-L)
      // (2.2) CDOL2 data elements - in the case of the 2'nd GENERATE AC (exclude 77-L)
      // (3)   T-L-V in the response to the GENERATE AC, except SDAD
      apk_ReadRamDataICC( ADDR_ICC_PDOL_CATS, 2, (UCHAR *)&iLen );
      apk_ReadRamDataICC( ADDR_ICC_PDOL_CATS+2, iLen, pkm );

      iLen = apk_GetBERLEN( &pkm[1], &cnt ); // actual length of data objects
      iHashLen = iLen;

      for( i=0; i<iLen; i++ )
         g_ibuf[i] = pkm[i+1+cnt]; // cat PDOL

//UI_ClearScreen();
//TL_DispHexWord(0,0,iLen);

      apk_ReadRamDataICC( ADDR_ICC_CDOL1_CATS, 2, (UCHAR *)&iLen );
      apk_ReadRamDataICC( ADDR_ICC_CDOL1_CATS+2, iLen, g_obuf );

      memmove( &g_ibuf[i], g_obuf, iLen ); // cat CDOL1
      iHashLen += iLen;
      i += iLen;

//TL_DispHexWord(1,0,iLen);

      if( phase == 2 ) // 2'nd GENERATE AC?
        {
        apk_ReadRamDataICC( ADDR_ICC_CDOL2_CATS, 2, (UCHAR *)&iLen );
        apk_ReadRamDataICC( ADDR_ICC_CDOL2_CATS+2, iLen, g_obuf );

        memmove( &g_ibuf[i], g_obuf, iLen ); // cat CDOL2
        iHashLen += iLen;
        i += iLen;
        }

      // concatenate T-L-V in response to the GAC, with the exception of SDAD
      // 77-L-[9F27-L-V, 9F36-L-V, 9F4B-L-V, 9F10-L-V,...]
      apk_GetBERLEN( &gac[3], &cnt ); // actual length of data objects
      ptrfirst = &gac[2] + 1 + cnt;   // ptr to the 1'st TLV

      iLen = gac[1]*256 + gac[0];     // total length of gac
      iLen -= ( ptrfirst - &gac[2] ); // exclude 77-L
      iLen -= 2;                      // exclude SW1 SW2

      // find SDAD (M)
      ptrobj = apk_FindTag( 0x9f, 0x4b, &gac[2] );
      iLen2 = apk_GetBERLEN( ptrobj, &cnt ); // length of SDAD

      iLen1 = ptrobj - ptrfirst; // data length before SDAD
      if( iLen1 != 0 )
        {
        iLen1 -= 2; // exclude Tag 9F4B
        memmove( &g_ibuf[i], ptrfirst, iLen1 ); // cat DE before SDAD
        iHashLen += iLen1;
        i += iLen1;
        }

//    i += iLen2; // skip SDAD
      ptrobj += (iLen2 + cnt); // skip SDAD

//  TL_DumpHexData(0,0,7, ptrobj);

//  TL_DispHexWord(0,0,iLen);
//  TL_DispHexWord(0,6,i);
//  TL_DispHexWord(0,11,iHashLen);
//  TL_DispHexWord(1,0,iLen1);
//  TL_DispHexWord(2,0,iLen2);

      iLen2 = iLen - iLen1 - iLen2 - 2 - cnt; // remainding DE's

//  TL_DispHexWord(3,0,iLen2);
//  UI_WaitKey();

      if( iLen2 )
        {
        memmove( &g_ibuf[i], ptrobj, iLen2 );   // cat DE after SDAD
        iHashLen += iLen2;
        }

//  TL_DispHexWord(0,0,iHashLen);
//  TL_DumpHexData(0,1,iHashLen, g_ibuf);

      // Verification 11: calculate & compare SHA1 result
      if( apk_HASH( SHA1, iHashLen, g_ibuf, temp ) == apkFailed )
        return( emvFailed );

// TL_DispHexByte(0,0,0x88);
// UI_WaitKey();

//  TL_DispHexWord(0,0,0x55);
//  TL_DumpHexData(0,1,20, temp);
//  TL_DispHexByte(0,6,j);
//  TL_DumpHexData(0,1,20, &pkc[j+2]);

      for( i=0; i<20; i++ )
         {
         if( temp[i] != pkc[i+j+2] ) // offset address of TDHC
           return( emvFailed );
         }

// TL_DispHexByte(0,0,0x99);
// UI_WaitKey();

      // Save "ICC Dynamic Number" (Tag=9F4C)
      i = pkc[2+4];            // length
      if( (i > 1) && (i < 9) ) // valid only 2~8 bytes
        {
        api_emv_PutDataElement( DE_ICC, ADDR_ICC_DYNAMIC_NBR, 2, (UCHAR *)&i );
        api_emv_PutDataElement( DE_ICC, ADDR_ICC_DYNAMIC_NBR+2, i, &pkc[2+5] );
        }
// api_emv_GetDataElement( DE_ICC, ADDR_ICC_DYNAMIC_NBR, 16, temp );
// TL_DumpHexData(0,0,16, temp);

      // Save "AC (TC or ARQC)" (Tag=9F26)
      i = 8;
      api_emv_PutDataElement( DE_ICC, ADDR_ICC_AC, 2, (UCHAR *)&i );
      api_emv_PutDataElement( DE_ICC, ADDR_ICC_AC+2, 8, &pkc[2+j-8] );

// api_emv_GetDataElement( DE_ICC, ADDR_ICC_AC, 10, temp );
// TL_DumpHexData(0,2,16, temp);

// UI_ClearScreen();
// TL_DispHexByte(0,0,0x66);
// UI_WaitKey();

      return( emvOK ); // DDA done

}

// ---------------------------------------------------------------------------
// FUNCTION: retrieve issuer public key for ICC PIN Encipherment.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR api_emv_RetrievePublicKeyISU( UCHAR *pkm, UCHAR *pke )
{
UCHAR capki;
UCHAR rid[RID_LEN];
//UCHAR pkm[258];         // public key modulus (2L-V[256])
//UCHAR pke[3];           // public key exponent (0x02, 0x03, or 0x10001)
UCHAR pkc[258];         // public key certificate (2L-V[256])
UCHAR temp[20];
UINT  iLen;
UINT  iHashLen;
UINT  iModLen;
UINT  iPKcLen;
UINT  iLeftMostLen;
UINT  i, j;
UCHAR cnt;
UCHAR *ptrobj;

      // ---------------------------------
      // 1. Retrieval of the CA public key
      // ---------------------------------

      apk_ReadRamDataICC( ADDR_ICC_CA_PKI, 2, (UCHAR *)&iLen ); // load length of CA pki
      if( iLen == 0 ) // pki present?
        {
        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        return( emvFailed );
        }

      apk_ReadRamDataICC( ADDR_ICC_CA_PKI+2, 1, &capki ); // load value of CA pki
      apk_ReadRamDataICC( ADDR_SELECTED_AID+1, RID_LEN, rid );  // load RID
      if( apk_RetrievePublicKeyCA( capki, rid, pkm, pke ) != apkOK ) // load (n,e) = PK(rid, pki)
        return( emvFailed );

      // ------------------------------------------
      // 2. Retrieve & Verify the ISSUER public key
      // ------------------------------------------

      // Verification 1: check issuer PK certificate length = CA PK modulus
      apk_ReadRamDataICC( ADDR_ICC_ISU_PKC, 258, pkc ); // load issuer public key certificate

      iLen = pkc[1]*256 + pkc[0];
      if( iLen == 0 ) // issuer PKC present?
        {
        apk_ReleaseRSA(); // PATCH: 2009-01-07

        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        return( emvFailed );
        }

      if( iLen != (pkm[1]*256 + pkm[0]) )
        {
        apk_ReleaseRSA(); // PATCH: 2009-01-07
        return( emvFailed ); // length not equal to CA pkm
        }

      iPKcLen = iLen;      // store PKC length
      iLeftMostLen = iLen - 36; // length of "issuer PK" or "leftmost digits of the issuer PK"

      if( apk_RecoverIPKC( pkm, pke, pkc ) != apkOK ) // pkc=2L-V
        return( emvFailed );

      iModLen = pkc[2+14-1]; // total length of "issuer PK"

//  TL_DumpHexData(0,0,256, pkc);

      // Verification 2: check recovered data trailer
      if( pkc[ (iPKcLen+2)-1 ] != 0xBC )
        return( emvFailed );

      // Verification 3: check recovered data header
      if( pkc[2+0] != 0x6A )
        return( emvFailed );

      // Verification 4: check certificate format
      if( pkc[2+1] != 0x02 )
        return( emvFailed );

      // Concatenation
      // (1) Recovered Data[2'nd..10'th] +
      // (2) Issuer Remainder +
      // (3) Issuer Exponent
      iHashLen = iPKcLen - 22; // header[1], HashResult[20], Trailer[1]
      for( i=0; i<iHashLen; i++ )
         g_ibuf[i] = pkc[i+3]; // from "Certificate Format" to "Issuer Public Key"

      apk_ReadRamDataICC( ADDR_ICC_ISU_PKR, 2, (UCHAR *)&iLen ); // load issuer public key remainder length
      if( iLen != 0 )
        {
        apk_ReadRamDataICC( ADDR_ICC_ISU_PKR+2, iLen, &g_ibuf[i] ); // cat issuer public key remainder data
        i += iLen;
        iHashLen += iLen;
        }

      apk_ReadRamDataICC( ADDR_ICC_ISU_PKE, 2, (UCHAR *)&iLen ); // load issuer public key exponent length
      if( iLen == 0 ) // exponent present?
        {
        g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
        return( emvFailed );
        }

      apk_ReadRamDataICC( ADDR_ICC_ISU_PKE+2, iLen, &g_ibuf[i] ); // cat issuer public key exponent data
      iHashLen += iLen;

      // Verification 5: calculate & compare SHA1 result
      if( apk_HASH( SHA1, iHashLen, g_ibuf, temp ) == apkFailed )
        return( emvFailed );

      for( i=0; i<20; i++ )
         {
         if( temp[i] != pkc[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
           return( emvFailed );
         }

      // Verification 6: check Issuer ID Number (leftmost 3-8 digits)
      apk_ReadRamDataICC( ADDR_ICC_AP_PAN+2, 10, temp ); // load application PAN

      if( TL_CNcmp( &pkc[2+2], temp, 4 ) == FALSE )
        return( emvFailed );

      // Verification 7: check the certificate expiration date MMYY
      if( TL_VerifyCertificateExpDate( &pkc[2+6] ) == FALSE )
        {
// UI_ClearScreen();
// TL_DispHexByte(0,0,0xFF);
// UI_WaitKey();
// TL_DumpHexData(0,0,2, &pkc[8]);

        return( emvFailed );
        }

//    TL_GetDateTime( temp ); // get current date (temp[0..11]="YYMMDDhhmmss")
//    temp[12] = TL_SetCentury( temp[0] );
//    temp[13] = ((temp[0] & 0x0f) << 4) + (temp[1] & 0x0f); // convert to BCD
//    i = temp[12]*256 + temp[13]; // today year CCYY
//
//    temp[14] = TL_SetCentury( pkc[2+7] );
//    j = temp[14]*256 + pkc[2+7]; // expiry year CCYY
//
//    if( j < i )  // compare Year
//      return( emvFailed );
//
//    if( j == i ) // compare Month
//      {
//      temp[12] = ((temp[2] & 0x0f) << 4) + (temp[3] & 0x0f); // convert to BCD
//      if( pkc[2+6] < temp[12] ) // compare month
//        return( emvFailed );
//      }

      // Verification 8: RID + INDEX + Certificate Serial Number ???

      // Verification 9: check the issuer public key algorithm indicator
      if( pkc[2+12] != 0x01 )
        return( emvFailed );

      // Issuer Public Key Modulus (stored in pkm[iModLen] array) = 2L-V
      //        (1) Leftmost Digits of the Issuer Public Key +
      //        (2) Issuer Public Key Remainder (if present)
      for( i=0; i<iLeftMostLen; i++ )
         pkm[i+2] = pkc[i+2+15];

      apk_ReadRamDataICC( ADDR_ICC_ISU_PKR, 2, (UCHAR *)&iLen ); // load issuer public key remainder length
      if( iLen != 0 )
        {
        apk_ReadRamDataICC( ADDR_ICC_ISU_PKR+2, iLen, g_ibuf ); // load issuer public key remainder data
        for( j=0; j<iLen; j++ )
           pkm[i+2+j] = g_ibuf[j];
        }

//    iModLen += iLen;
      pkm[0] = iModLen & 0x00ff;
      pkm[1] = (iModLen & 0xff00) >> 8;

      // issuer public key exponent (stored in pke[])
      apk_ReadRamDataICC( ADDR_ICC_ISU_PKE+2, 3, pke ); // load issuer public key exponent data

      // backup complete issuer public key modulus
      apk_WriteRamDataICC( ADDR_ICC_ISU_PKM, pkm[1]*256+pkm[0]+2, pkm );

// TL_DumpHexData(0,0,3, pke);
// TL_DumpHexData(0,0,256, pkm);

      return( emvOK );	// PATCH: 2009-01-08
}

// ---------------------------------------------------------------------------
// FUNCTION: Check if both Terminal and ICC support CDA.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - yes
//           FALSE - no
// ---------------------------------------------------------------------------
UCHAR CDA_CheckCondition( void )
{
UCHAR aip[2];
UCHAR cap[3];

      // get icc_AIP[0..1]
      api_emv_GetDataElement( DE_ICC, ADDR_ICC_AIP, 2, aip );

      // get term_CAP[0..2]
      api_emv_GetDataElement( DE_TERM, ADDR_TERM_CAP, 3, cap );

      if( (aip[0] & AIP0_OFFLINE_CDA) && (cap[2] & CAP2_CDA) )
        {
        // get TVR0
        api_emv_GetDataElement( DE_TERM, ADDR_TERM_TVR, 1, cap );

        if( cap[0] & TVR0_OFFLINE_CDA_FAILED )
          return( FALSE ); // CDA had failed during recovering ICC PK, seems not support CDA
        else
          return( TRUE );  // CDA verification shall be performed
        }
      else
        return( FALSE );   // both terminal & icc not support CDA
}

// ---------------------------------------------------------------------------
// FUNCTION: Offline Data Authentication.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed
//           emvOutOfService - terminate the transaction.
// ---------------------------------------------------------------------------
UCHAR api_emv_OfflineDataAuthen( void )
{
UCHAR aip[2];
UCHAR cap[3];
UCHAR result;

      // clear temp buffer for Issuer & ICC PK Modulus Length
      aip[0] = 0;
      aip[1] = 0;
      api_emv_PutDataElement( DE_ICC, ADDR_ICC_ISU_PKM, 2, aip );
      
      api_emv_PutDataElement( DE_ICC, ADDR_ICC_PKM, 2, aip );

      // get icc_AIP[0..1]
      api_emv_GetDataElement( DE_ICC, ADDR_ICC_AIP, 2, aip );

      // get term_CAP[0..2]
      api_emv_GetDataElement( DE_TERM, ADDR_TERM_CAP, 3, cap );

      if( (aip[0] & AIP0_OFFLINE_CDA) && (cap[2] & CAP2_CDA) )
        {
        g_term_TSI[0] |= TSI0_OFFLINE_DA_PERFORMED;
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );

        // -----------------------
        //  EMV2000: Perform CDA
        // -----------------------
        if( apk_OFDA_BuildInputList( g_obuf ) == apkOK )
          {
          if( api_emv_OfflineDDA( 1, g_obuf ) == emvOK )
            return( emvOK ); // ICC PK recovered OK for CDA
          }

        g_term_TVR[0] |= TVR0_OFFLINE_CDA_FAILED;
        api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );

        return( emvOK ); // process done but CDA has failed
        }

      // --------------------------------------------------------------------
      if( (aip[0] & AIP0_OFFLINE_DDA) && (cap[2] & CAP2_DDA) )
        {
        // -----------------------
        //       Perform DDA
        // -----------------------
        if( apk_OFDA_BuildInputList( g_obuf ) == apkFailed )
          {
          g_term_TSI[0] |= TSI0_OFFLINE_DA_PERFORMED;
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );

          g_term_TVR[0] |= TVR0_OFFLINE_DDA_FAILED;
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );

          return( emvOK );
          }

        result = api_emv_OfflineDDA( 0, g_obuf );
        if( result == emvOutOfService )
          return( emvOutOfService );

        if( result != emvOK )
          {
          g_term_TVR[0] |= TVR0_OFFLINE_DDA_FAILED;
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );

          if( result == emvNotReady ) // DDOL not present
            {
            g_term_TVR[0] |= TVR0_ICC_DATA_MISSING;
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );

            g_term_TSI[0] |= TSI0_OFFLINE_DA_PERFORMED;					// PATCH: 2013-06-18, 2CC.076.00
	    api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );	//

            return( emvOK );
            }
          }
        }
      else
        {
        if( (aip[0] & AIP0_OFFLINE_SDA) && (cap[2] & CAP2_SDA) &&
            ( ((aip[0] & AIP0_OFFLINE_DDA) == 0) || ((cap[2] & CAP2_DDA) == 0) ) )
          {
          // -----------------------
          //       Perform SDA
          // -----------------------

	  // SB113
	  g_term_TVR[0] |= TVR0_SDA_SELECTED;	// 2016-01-04
	  api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );
	  
          if( apk_OFDA_BuildInputList( g_obuf ) == apkFailed )
            {
            g_term_TSI[0] |= TSI0_OFFLINE_DA_PERFORMED;
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );

            g_term_TVR[0] |= TVR0_OFFLINE_SDA_FAILED;
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );

            return( emvOK );
            }

          if( api_emv_OfflineSDA( g_obuf ) == emvFailed )
            {
            g_term_TVR[0] |= TVR0_OFFLINE_SDA_FAILED;
            api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );
            }
          }
        else
          {
          // CDA, DDA and SDA are not performed

          g_term_TVR[0] |= TVR0_OFFLINE_DA_NOT_PERFORMED;
          api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 1, &g_term_TVR[0] );

          return( emvOK );
          }
        }

      g_term_TSI[0] |= TSI0_OFFLINE_DA_PERFORMED;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 1, &g_term_TSI[0] );

      return( emvOK );
}

