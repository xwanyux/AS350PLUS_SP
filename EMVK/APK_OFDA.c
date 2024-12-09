//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APK_OFDA.C                                                 **
//**  MODULE   : apk_OFDA_BuildInputList()                                  **
//**             apk_RetrievePublicKeyCA()                                  **
//**             apk_RecoverPKC()                                           **
//**             apk_HASH()                                                 **
//**             apk_GetChallengeTERM()                                     **
//**             apk_GetChallengeICC()                                      **
//**             apk_InternalAuthen()                                       **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2003/01/04                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2003-2007 SymLink Corporation. All rights reserved.      **
//**History:                                                                **
//**	     2008-12-30 Richard    modify apk_OFDA_BuildInputList()         **
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
#include "APDU.H"
#include "TOOLS.h"

// ---------------------------------------------------------------------------
// FUNCTION: building of the input list for offline data authentication.
//           (ie, "static data to be authenticated")
// FORMAT  : [records identified by the AFL] +
//           [data elements identified by the optional Static Data Authentication Tag List]
// INPUT   : none.
// OUTPUT  : outbuf - 2L-V, the result concatenation of data list.
// REF     : g_ibuf
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
UCHAR apk_OFDA_BuildInputList( UCHAR *outbuf )
{
UINT  i, j;
UCHAR *ptrobj;
UCHAR *ptrlist;
int   iTotalLen;
UINT  iLen;
UINT  iTagListLen;
UCHAR tag1, tag2;
UCHAR cnt;
UCHAR de[4];
UCHAR index;


      ptrobj = outbuf;
      *ptrobj++ = 0; // clear output length
      *ptrobj++ = 0;
      iTotalLen = 0;

      // -----------------------------------------------
      // Processing of the records identified by the AFL
      // -----------------------------------------------
      // The fields within [] are included.
      // (1) SFI=1..10,  70 LL [Data] SW1 SW2
      // (2) SFI=11..30, [70 LL Data] SW1 SW2
      // (3) If the record is a non Tag-70, ODA has failed.
      // -----------------------------------------------
      for( i=0; i<MAX_ICC_REC_CNT; i++ )
         {
         // read 1 icc record: ODA[1] 70-L-V
         apk_ReadRamDataICC( ADDR_ICC_REC_01+i*ICC_REC_LEN, ICC_REC_LEN, g_ibuf );

         // check ODA flag byte
         if( g_ibuf[0] == 0 )
           break; // end of records read
         else
           {
           iLen = apk_GetBERLEN( &g_ibuf[2], &cnt ); // retrieve length of this data element

           if( (g_ibuf[0] & 0x80) != 0 ) // the selected record for ODA?
             {
             if( g_ibuf[1] != 0x70 ) // Tag = 70?
               return( apkFailed );  // fail to process ODA

             // SFI=1..10
             if( (g_ibuf[0] & 0x1f) < 11 )
               {
               	/*2008-12-30_Richard- 2CC.144.00~02
               	 *                    1. increasing the buffer size of g_ibuf and g_obuf to 1k bytes
               	 *                    2. the check boundary increases to 997 bytes
               	 */
               if( (iTotalLen + iLen) < 1497 )//EMV 42a Charles 2009-03-09 2CC.144.01
                 {
                 for( j=0; j<iLen; j++ )
                    *ptrobj++ = g_ibuf[j+2+cnt]; // store [Data]
                 iTotalLen += iLen;
                 }
               else
                 return( apkFailed ); // out of range
               }
             else
               {
               // SFI=11..30
               if( (g_ibuf[0] & 0x1f) < 31 )
                 {
                 /*2008-12-30_Richard- 2CC.144.00~02
                  *                    1. increasing the buffer size of g_ibuf and g_obuf to 1k bytes
               	  *                    2. the check boundary increases to 997 bytes
               	  */
                 if( (iTotalLen + iLen) < 1497 )//EMV 42a Charles 2009-03-09 2CC.144.01
                   {
                   for( j=0; j<iLen+cnt+1; j++ )
                      *ptrobj++ = g_ibuf[j+1];
                   iTotalLen += (iLen+cnt+1);
                   }
                 else
                   return( apkFailed ); // out of range
                 }
               }
             }
           }
         } // for( MAX_ICC_REC_CNT )

      // ------------------------------------------------------------
      // Processing of the Static Data Authentication Tag List
      // ------------------------------------------------------------
      // Static Data Authentication Tag List: 9F4A-L-Tag1-Tag2...TagN
      // Value field of each Tag shall be included according to:
      // (1) ICC as the source.
      // (2) All tags must represent data elements available in the
      //     current transaction.
      // (3) Tags must not refer to a constructed data object.
      // (4) The value field of the data object identified by the tag
      //     is to be concatenated to the current end of the input
      //     string. Tags and Lengths of the data objects are not
      //     included in the concatenation.
      // ------------------------------------------------------------

#ifdef  L2_SW_DEBUG

      // assumption for ICC Static Data Authentication Tag List
      g_ibuf[0] = 3;
      g_ibuf[1] = 0;

      g_ibuf[2] = 0x9f; // ap version number
      g_ibuf[3] = 0x08;

      g_ibuf[4] = 0x5a; // ap PAN
      apk_WriteRamDataICC( ADDR_ICC_SDA_TL, 0x05, g_ibuf );

      // CA public key index
      g_ibuf[0] = 0x01;
      g_ibuf[1] = 0x00;

      g_ibuf[2] = 1;
      apk_WriteRamDataICC( ADDR_ICC_CA_PKI, 0x03, g_ibuf );

      // ISSUER public key certificate
      TL_Debug_PutIssuerPKC( g_ibuf );
      apk_WriteRamDataICC( ADDR_ICC_ISU_PKC, 130, g_ibuf );

      // ISSUER public key remainder
      TL_Debug_PutIssuerPKR( g_ibuf );
      apk_WriteRamDataICC( ADDR_ICC_ISU_PKR, 22, g_ibuf );

      // ISSUER public key exponent
      TL_Debug_PutIssuerPKE( g_ibuf );
      apk_WriteRamDataICC( ADDR_ICC_ISU_PKE, 3, g_ibuf );

      // Signed Static Application Data
      TL_Debug_PutSSAD( g_ibuf );
      apk_WriteRamDataICC( ADDR_ICC_SIGNED_SAD, 114, g_ibuf );

#endif

      // ------------------------------------------------------------

      apk_ReadRamDataICC( ADDR_ICC_SDA_TL, 258, g_ibuf ); // LEN[2] Tag1..TagN
      iTagListLen = g_ibuf[1]*256+g_ibuf[0]; // length of all tags
      ptrlist = &g_ibuf[2]; // pointer to the tag

      if( iTagListLen != 0 )
        {
        // tag list is present
        do{
          tag1 = *ptrlist++;
          iTagListLen--;

          // check constructed DO
          if( apk_CheckConsTag( tag1 ) == TRUE )
            return( apkFailed );

          // check word tag
          if( apk_CheckWordTag( tag1 ) == TRUE )
            {
            tag2 = *ptrlist++;
            iTagListLen--;
            }
          else
            {
            tag2 = tag1;
            tag1 = 0;
            }

          // check ICC source
          index = apk_ScanIDE( tag1, tag2, de );
          if( index == 255 )
            return( apkFailed );

          i = de[3];  // max length of the data element
          if( i == 0 )
            i = 256;

          // load data element & check if available
          apk_ReadRamDataICC( TL_IDE_GetAddr(index), i+2, g_temp ); // including LL

          iLen = g_temp[1]*256+g_temp[0]; // actual length of the requested data element
          if( (iLen != 0) && ((iTotalLen + iLen) < 257) )
            {
            // concatenated to the current end of the input string
            for( j=0; j<iLen; j++ )
               *ptrobj++ = g_temp[j+2];
            iTotalLen += iLen;
            }
          else
            return( apkFailed );

          } while( iTagListLen > 0 ); // next tag
        }

      // put final total length of input string
      outbuf[0] = iTotalLen & 0x00ff;
      outbuf[1] = (iTotalLen & 0xff00) >> 8;

      return( apkOK ); // done
}

// ---------------------------------------------------------------------------
// FUNCTION: retrieve & verify CA Public Key.
// INPUT   : pki   - CA public key index.
//           rid   - registered application provider id.
// OUTPUT  : pkm   - public key modulus.  (n) -- LEN[2]+n[256] bytes
//           pke   - public key exponent. (e) -- e[3] integer
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
UCHAR apk_RetrievePublicKeyCA( UCHAR pki, UCHAR *rid, UCHAR *pkm, UCHAR *pke )
{
#ifdef	USE_RSA_SAM

      if( apdu_SAM_GET_PUBLIC_KEY( pki, rid, pkm, pke ) == apiOK )
        return( apkOK );
      else
        return( apkFailed );

#endif

#ifndef	USE_RSA_SAM

ULONG i;
UCHAR r_apdu[CA_PK_LEN+2];
UCHAR pkh[CA_PK_HEADER_LEN];
UINT  iFID;
UCHAR index;


	goto EMVL2_TEST;	// 2009-10-23, non-PCI & page solution

	// --- PCI PED ---
#if	0
	if( api_ped_SelectKey_CAPK( pki, rid, pkh, (UCHAR *)&index ) == apiOK )
	  {
	  pkm[0] = pkh[OFFSET_SAM_MOD_LEN+1]; // length of modulus
          pkm[1] = pkh[OFFSET_SAM_MOD_LEN+0]; //
	  g_key_fid = index + KEY_FID_01;

	  return( apiOK );
	  }
	else
	  return( apiFailed );
#endif

EMVL2_TEST:

      // --- TEST ONLY ---
      iFID = KEY_FID_01; // initial key file id
      g_capk_cnt = MAX_CA_PK_CNT;	// 2009-10-23, assume max
      for( i=0; i<g_capk_cnt; i++ )
         {
         apk_ReadRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*i, CA_PK_LEN, r_apdu );  // read CAPK components
//       TL_DumpHexData( 0, 0, CA_PK_LEN, r_apdu );

         // matching RID & INDEX
         if( (TL_memcmp( rid, &r_apdu[OFFSET_SAM_RID], 5 ) == 0) && (pki == r_apdu[OFFSET_SAM_PKI]) )
           break;
         else
           iFID++;
         }

      if( iFID >= (KEY_FID_01 + g_capk_cnt) )
        return( apkFailed ); // public key not found
        
//      TL_DumpHexData( 0, 0, CA_PK_LEN, r_apdu );

      pkm[0] = r_apdu[OFFSET_SAM_MOD_LEN+1]; // length of modulus
      pkm[1] = r_apdu[OFFSET_SAM_MOD_LEN+0]; //

      if( r_apdu[OFFSET_SAM_EXP_LEN] == 1 )    
        memmove( &pkm[2], &r_apdu[OFFSET_SAM_MOD-2], pkm[0]+pkm[1]*256 );	// modulus
      else
        memmove( &pkm[2], &r_apdu[OFFSET_SAM_MOD], pkm[0]+pkm[1]*256 );		// modulus
        
      memmove( pke, &r_apdu[OFFSET_SAM_EXP], 3 );	// exponent
      
//      TL_DispHexByte( 0, 0, 1, OFFSET_SAM_MOD );
//      TL_DumpHexData( 0, 1, 0x90, &r_apdu[OFFSET_SAM_MOD] );
//      TL_DumpHexData( 0, 0, 0x90+2, pkm );
//      TL_DumpHexData( 0, 0, 3, pke );

      if( api_rsa_loadkey( pkm, pke ) == apiOK )
        {
        g_key_fid = iFID; // set file id for the selected key
        return( apkOK );
        }
      else
        return( apkFailed );

#endif

}

// ---------------------------------------------------------------------------
// FUNCTION: check the data objects required for offline SDA.
//           (1) RID
//           (2) CA Public Key Index
//           (3) Issuer Public Key Certificate
//           (4) Issuer Public Key Remainder (if present)
//           (5) Issuer Public Key Exponent
//           (6) Signed Static Application Data
//           (7) Static data to be authenticated
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
//UCHAR apk_SDA_CheckRequiredDO( void )
//{
//UCHAR len;
//
//      // CA public key index
//      apk_ReadRamDataICC( ADDR_ICC_CA_PKI, 1, &len );
//      if( len == 0 )
//        return( apkFailed );
//
//      // issuer public key certificate
//      apk_ReadRamDataICC( ADDR_ICC_ISU_PKC, 1, &len );
//      if( len == 0 )
//        return( apkFailed );
//
//      // issuer public key exponent
//      apk_ReadRamDataICC( ADDR_ICC_ISU_PKE, 1, &len );
//      if( len == 0 )
//        return( apkFailed );
//
//      // signed static application data
//      apk_ReadRamDataICC( ADDR_ICC_SIGNED_SAD, 1, &len );
//      if( len == 0 )
//        return( apkFailed );
//
//     return( apkOK );
//}

// ---------------------------------------------------------------------------
// FUNCTION: retrieve & verify Issuer Public Key.
// INPUT   : ca_pkm -- CA public key modulus.
//           ca_pke -- CA public key exponent.
// OUTPUT  : none.
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
//UCHAR apk_RetrievePublicKeyISSUER( UCHAR *ca_pkm, UCHAR *ca_pke )
//{
//UCHAR ISSUER_pkc[257];  // ISSUER public key certificate (L-V, L=0: 256 bytes)
//UCHAR ISSUER_rpkc[256]; // recovered ISSUER public key certificate
//UINT  iLen;
//UINT  iHASH_len;
//UINT  i;
//
//      // Verification 1: check certificate length
//      apk_ReadRamDataICC( ADDR_ICC_ISU_PKC, 257, ISSUER_pkc ); // load issuer public key certificate
//      if( ISSUER_pkc[0] != CA_pkm[0] )
//        return( apkFailed );
//
//      iLen = ISSUER_pkc[0];
//      if( iLen == 0 )
//        iLen = 256;
//
//      if( apk_RecoverIssuerPKC( ca_pkm, ca_pke, ISSUER_pkc, ISSUER_rpkc ) != apkOK )
//        return( apkFailed );
//
//      // Verification 2: check recovered data trailer
//      if( ISSUER_rpkc[ iLen-1 ] != 0xBC )
//        return( apkFailed );
//
//      // Verification 3: check recovered data header
//      if( ISSUER_rpkc[0] != 0x6A )
//        return( apkFailed );
//
//      // Verification 4: check certificate format
//      if( ISSUER_rpkc[1] != 0x02 )
//        return( apkFailed );
//
//      // Concatenation (Recovered Data + Remainder + Exponent)
//      iHASH_len = iLen - 22;
//      for( i=0; i<iHASH_len; i++ )
//         g_ibuf[i] = ISSUER_rpkc[i+1]; // from "Certificate Format" to "Issuer Public Key"
//
//      apk_ReadRamDataICC( ADDR_ICC_ISU_PKR, 1, iLen );            // load issuer public key remainder length
//      apk_ReadRamDataICC( ADDR_ICC_ISU_PKR+1, iLen, &g_ibuf[i] ); // load issuer public key remainder data
//      i += iLen;
//      iHASH_len += iLen;
//
//      apk_ReadRamDataICC( ADDR_ICC_ISU_PKE, 1, iLen );            // load issuer public key exponent length
//      apk_ReadRamDataICC( ADDR_ICC_ISU_PKE+1, iLen, &g_ibuf[i] ); // load issuer public key exponent data
//      iHASH_len += iLen;
//
//      apk_SHA1( iHASH_len, g_ibuf );
//
//
//}

// ---------------------------------------------------------------------------
// FUNCTION: load external working key to RSA SAM (eg. ISSUER or ICC PK),
//           for RECOVER function.
// INPUT   : pkm - source public key modulus.  (n) -- LEN[2]+n[256] bytes
//           pke - source public key exponent. (e) -- e[3] integer
//           slot- the key file ID. (eg, 0x8032)
// OUTPUT  : none.
// REF     : g_key_fid, g_temp
// RETURN  : apkOK
//           apkFailed
// NOTE    : this function will auto pad leading zeor's for x8 moduli.
// ---------------------------------------------------------------------------
UCHAR apk_LoadExternalPK( UINT slot, UCHAR *pkm, UCHAR *pke )
{
#ifdef	USE_RSA_SAM

UINT  iLen;
UINT  pad;
UCHAR *ptrpkm;

      ptrpkm = pkm;

      // auto pad leading zero's for x8 moduli padding
      iLen = pkm[1]*256 + pkm[0];
      pad = iLen % 8;

      if( pad != 0 )
        {
        pad = 8 - pad; // the nearest x8 moduli
        memset( &g_temp[2], 0x00, pad );
        memmove( &g_temp[pad+2], &pkm[2], iLen );

        iLen += pad;
        g_temp[0] = iLen & 0x00ff;
        g_temp[1] = (iLen & 0xff00) >> 8;

        ptrpkm = g_temp;
        }

      // load external public key to SAM
      if( apdu_SAM_LOAD_EXTERNAL_PK( slot, ptrpkm, pke ) == apiOK )
        {
        g_key_fid = slot; // set current key file id
        return( apkOK );
        }
      else
        return( apkFailed );

#endif

#ifndef	USE_RSA_SAM

UINT  iLen;
UINT  pad;
UCHAR *ptrpkm;

      ptrpkm = pkm;

      // auto pad leading zero's for x8 moduli padding
      iLen = pkm[1]*256 + pkm[0];
      pad = iLen % 4;	// 32-bit boundary

      if( pad != 0 )
        {
        pad = 4 - pad; // the nearest x8 moduli
        memset( &g_temp[2], 0x00, pad );
        memmove( &g_temp[pad+2], &pkm[2], iLen );

        iLen += pad;
        g_temp[0] = iLen & 0x00ff;
        g_temp[1] = (iLen & 0xff00) >> 8;

        ptrpkm = g_temp;
        }

      // load external public key
      if( api_rsa_loadkey( ptrpkm, pke ) == apiOK )
        {
        g_key_fid = slot; // set current key file id
        return( apkOK );
        }
      else
        return( apkFailed );

#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: Force to release the BSP RSA function.
//	     (RSA Public Key Encryption - certificate verification)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// NOTE    : This function is used by EMV L2 kernel only, especially when
//           the procedure exits between "api_rsa_loadkey()" and "api_rsa_recover()".
// ---------------------------------------------------------------------------
void	apk_ReleaseRSA( void )
{
	api_rsa_release();
}

// ---------------------------------------------------------------------------
// FUNCTION: using RSA recover function to the specified certificate to get
//           the original data before signature. (GENERAL)
// INPUT   : pkm - source public key modulus.  (n) -- LEN[2]+n[256] bytes
//           pke - source public key exponent. (e) -- e[3] integer
//           pkc - target public key certificate.
//                 source: eg., CA
//                 target: eg., Issuer or ICC
// OUTPUT  : pkc - the recovered data from the target certificate.
// REF     : g_key_fid, g_temp
// RETURN  : apkOK
//           apkFailed
// NOTE    : this function will auto pad/remove leading zeor's for x8 moduli.
// ---------------------------------------------------------------------------
UCHAR apk_RecoverPKC( UCHAR *pkm, UCHAR *pke, UCHAR *pkc )
{
#ifdef	USE_RSA_SAM

UINT  slot; // key slot number
UINT  iLen;
UINT  pad;
UCHAR *ptrpkc;

      ptrpkc = pkc;

      // auto pad leading zero's for x8 moduli
      iLen = pkc[1]*256 + pkc[0];
      pad = iLen % 8;

      if( pad != 0 )
        {
        pad = 8 - pad; // the nearest x8 moduli padding
        memset( &g_temp[2], 0x00, pad );
        memmove( &g_temp[pad+2], &pkc[2], iLen );

        iLen += pad;
        g_temp[0] = iLen & 0x00ff;
        g_temp[1] = (iLen & 0xff00) >> 8;

        ptrpkc = g_temp;
        }

      slot = g_key_fid;  // get current key file ID

      // X = Recover(PK)[Sign(SK)[X]], pkc=Sign(SK)[X]
      if( apdu_SAM_RSA_RECOVER( slot, ptrpkc ) == apiOK )
        {
        // auto remove leading zero's from the recoverd data X
        if( pad != 0 )
          {
          iLen -= pad;
          memmove( &pkc[2], ptrpkc+pad+2, iLen );
          pkc[0] = iLen & 0x00ff;
          pkc[1] = (iLen & 0xff00) >> 8;
          }

        return( apkOK );
        }
      else
        return( apkFailed );

#endif

#ifndef	USE_RSA_SAM

UINT  iLen;
UINT  pad;
UCHAR *ptrpkc;

      ptrpkc = pkc;

      // auto pad leading zero's for x8 moduli
      iLen = pkc[1]*256 + pkc[0];
      pad = iLen % 4;	// 32-bit boundary

      if( pad != 0 )
        {
        pad = 4 - pad; // the nearest x8 moduli padding
        memset( &g_temp[2], 0x00, pad );
        memmove( &g_temp[pad+2], &pkc[2], iLen );

        iLen += pad;
        g_temp[0] = iLen & 0x00ff;
        g_temp[1] = (iLen & 0xff00) >> 8;

        ptrpkc = g_temp;
        }

      // X = Recover(PK)[Sign(SK)[X]], pkc=Sign(SK)[X]
      if( api_rsa_recover( ptrpkc, ptrpkc ) == apiOK )
        {
        // auto remove leading zero's from the recoverd data X
        if( pad != 0 )
          {
          iLen -= pad;
          memmove( &pkc[2], ptrpkc+pad+2, iLen );
          pkc[0] = iLen & 0x00ff;
          pkc[1] = (iLen & 0xff00) >> 8;
          }

        return( apkOK );
        }
      else
        return( apkFailed );

#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: using RSA recover function to the specified certificate to get
//           the original data before signature. (for ISSUER PKC only)
// INPUT   : pkm - source public key modulus.  (n) -- LEN[2]+n[256] bytes
//           pke - source public key exponent. (e) -- e[3] integer
//           pkc - target public key certificate.
//                 source: eg., CA
//                 target: eg., Issuer or ICC
// OUTPUT  : pkc - the recovered data from the target certificate.
// REF     : g_key_fid
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
UCHAR apk_RecoverIPKC( UCHAR *pkm, UCHAR *pke, UCHAR *pkc )
{
#ifdef	USE_RSA_SAM

UINT  slot; // key slot number

      slot = g_key_fid;  // get current key file ID

      // X = Recover(PK)[Sign(SK)[X]], pkc=Sign(SK)[X]
      if( apdu_SAM_RSA_RECOVER( slot, pkc ) == apiOK )
        return( apkOK );
      else
        return( apkFailed );

#endif

#ifndef	USE_RSA_SAM

	if( api_rsa_recover( pkc, pkc ) == apiOK )
	  return( apkOK );
	else
	  return( apkFailed );

#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: using SHA-1 algorithm to hash the input data
//           to generate a 20-byte digest.
// INPUT   : algorithm - SHA1 or MD5.
//           length    - length of the input data.
//           data      - the data to be hashed.
// OUTPUT  : digest    - 20 bytes digest.
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
UCHAR apk_HASH( UCHAR algorithm, UINT length, UCHAR *data, UCHAR *digest )
{
#ifdef	USE_RSA_SAM

      if( apdu_SAM_HASH( algorithm, length, data, digest ) == apiOK )
        return( apkOK );
      else
        return( apkFailed );

#endif

#ifndef	USE_RSA_SAM

	if( api_sys_SHA1( length, data, digest ) == apiOK )
	  return( apkOK );
	else
	  return( apkFailed );

#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: to obtain an unpredictable number (8-byte) from terminal.
// INPUT   : none.
// OUTPUT  : random - the random number got.
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
UCHAR apk_GetChallengeTERM( UCHAR *random )
{
    // source: AS300   (test)
    TL_GetRandomNumber( random );
    return( apkOK );

    // source: RSA SAM (formal)
//  if( apdu_SAM_GET_CHALLENGE( random ) == apiOK )
//    return( apkOK );
//  else
//    return( apkFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: to obtain an unpredictable number (8-byte) from icc.
// INPUT   : none.
// OUTPUT  : random - 2L-V, the random number got (0A 00 RANDOM[8] 90 00).
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
UCHAR apk_GetChallengeICC( UCHAR *random )
{
      if( apdu_GET_CHALLENGE( random ) == apiOK )
        return( apkOK );
      else
        return( apkFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: issuing command to icc to genearte dynamic signature.
// INPUT   : ddol
// OUTPUT  : sdad
// RETURN  : apkOK
//           apkFailed
//           apkNotReady - SDAD is not present (only returning 90 00)
// ---------------------------------------------------------------------------
UCHAR apk_InternalAuthen( UCHAR *ddol, UCHAR *sdad )
{
UINT  iLen;
UCHAR *ptrobj;

      if( apdu_INTERNAL_AUTHENTICATE( ddol, sdad ) == apiOK )
        {
        iLen = sdad[1]*256 + sdad[0];

        if( (iLen == 2) && (sdad[2] == 0x90) && (sdad[3] == 0x00) ) // PATCH: 2006-10-04
          return( apkNotReady );

        if(iLen<=4)//09-01-08 charles tsai Int Auth must response//if( iLen <= 2 )
          return( apkFailed );

        if( (sdad[iLen] == 0x90) && (sdad[iLen+1] == 0x00) )
          {
          // PATCH: 2006-10-07
          if( (sdad[2] != 0x80) && (sdad[2] != 0x77) )
            return( apkFailed );

          if( sdad[2] == 0x77 )
            {
            if( sdad[3] == 0x00 )    // PATCH: 2006-10-11, sdad not present
              return( apkNotReady );
            ptrobj = apk_FindTag( 0x9F, 0x4B, &sdad[2] );
            if( ptrobj == 0 )
              return( apkFailed );
            }

          return( apkOK );
          }
        else
          return( apkFailed );
        }
      else
        return( apkFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: select SAM with AID="A0 00 00 00 00 00 02".
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
UCHAR apk_SelectSAM( void )
{
#ifdef	USE_RSA_SAM

UCHAR buf[8];
UCHAR fci[16];

      buf[0] = 0x07;

      buf[1] = 0xA0;
      buf[2] = 0x00;
      buf[3] = 0x00;
      buf[4] = 0x00;
      buf[5] = 0x00;
      buf[6] = 0x00;
      buf[7] = 0x02;
      if( apdu_SAM_SELECT( buf, 0, fci ) != apiOK )
        return( apkFailed );
      if( ((fci[1]*256 + fci[0]) != 2) || (fci[2] != 0x90) || (fci[3] != 0x00) )
        return( apkFailed );
      else
        return( apkOK );
#endif

#ifndef	USE_RSA_SAM
	return( apkOK );
#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: clear up OCS-SAM to avoid from error code 6611.
// INPUT   : none.
// OUTPUT  : response - 2L-V.
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
UCHAR apk_CleanSAM( void )
{
#ifdef	USE_RSA_SAM

UCHAR buf[8];

      if( apdu_SAM_CLEAN( buf ) != apiOK )
        return( apkFailed );

      if( ((buf[1]*256 + buf[0]) != 2) || (buf[2] != 0x90) || (buf[3] != 0x00) )
        return( apkFailed );
      else
        return( apkOK );
#endif

#ifndef	USE_RSA_SAM
	return( apkOK );
#endif
}

// ---------------------------------------------------------------------------
