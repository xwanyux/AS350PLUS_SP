//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APK_SPSE.C                                                 **
//**  MODULE   : apk_SelectPSE()                                            **
//**             apk_SelectDDF()                                            **
//**             apk_SelectADF()                                            **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2002/12/06                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2002-2007 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
//#include "bsp_mem.h"
#include "POSAPI.h"
//#include <EMVDC.h>
#include "GDATAEX.h"
#include "EMVAPI.h"
#include "EMVAPK.h"
//#include <APDU.H>
//#include <TOOLS.h>
UCHAR SPSE_SelectFile( UCHAR type, UCHAR *name, UCHAR occurrence, UCHAR *fci );
//UCHAR SPSE_SelectFile2( UCHAR type, UCHAR *name, UCHAR occurrence, UCHAR *fci );

// ---------------------------------------------------------------------------
// FUNCTION: Terminal begins with an explicit selection of the
//           Payment System Environment.
// INPUT   : none.
// OUTPUT  : fci - 2L-V type, the file control information.
// RETURN  : apkOK               (9000, 61La)
//           apkFileInvalidated  (6283) -- application blocked
//           apkFuncNotSupported (6A81) -- card blocked
//           apkFileNotFound     (6A82)
//           apkUnknown          (any other values) -- EMV2000
//           apkFailed
//           apkIncorrect        (mandatory data objects missing or not parse correctly
// ---------------------------------------------------------------------------
UCHAR apk_SelectPSE( UCHAR *fci )
{
UCHAR buf[16] = " 1PAY.SYS.DDF01";

buf[0] = 14;
UCHAR rec_len[2];
UINT  iLen;
UCHAR result;
UCHAR *ptrobj;
UCHAR *ptrnext;
UCHAR cnt;
UCHAR padlen[1];

      // select DDFNAME = "1PAY.SYS.DDF01"

//      buf[1] =  '1';
//      buf[2] =  'P';
//      buf[3] =  'A';
//      buf[4] =  'Y';
//      buf[5] =  '.';
//      buf[6] =  'S';
//      buf[7] =  'Y';
//      buf[8] =  'S';
//      buf[9] =  '.';
//      buf[10] = 'D';
//      buf[11] = 'D';
//      buf[12] = 'F';
//      buf[13] = '0';
//      buf[14] = '1';

      result = SPSE_SelectFile( 0, buf, 0, fci );

//    ptrobj = apk_FindTag( 0xA5, 0x00, &fci[2] );
//    iLen = apk_GetBERLEN( ptrobj, &cnt );
//    rec_len[0] = iLen & 0x00ff;
//    rec_len[1] = (iLen & 0xff00) >> 8;
//    ptrobj += cnt; // ptr to the 1'st DO of FCI Proprietary Template

      if( result == apkOK )
        {

        ptrobj = apk_FindTag( 0xA5, 0x00, &fci[2] );
        iLen = apk_GetBERLEN( ptrobj, &cnt );
        rec_len[0] = iLen & 0x00ff;
        rec_len[1] = (iLen & 0xff00) >> 8;
        ptrobj += cnt; // ptr to the 1'st DO of FCI Proprietary Template


        // parse all data objects
        if( apk_ParseLenFCI( fci, padlen ) == FALSE )
       // return( apkFailed );
          return( apkIncorrect ); // PATCH: PBOC2.0, 2006-02-15, 2CL.029.00.01

        if( apk_ParseTLV( &rec_len[0], ptrobj, padlen[0] ) == FALSE )
        //return( apkFailed );
          return( apkIncorrect ); // EMV2000

        // check mandatory data object: SFI (1~10)
        ptrobj = apk_FindTag( 0x88, 0x00, &fci[2] );

        if( ptrobj == 0 )
        //return( apkFailed );
          return( apkIncorrect ); // EMV2000
        else
          {
          // check length
          iLen = apk_GetBERLEN( ptrobj, &cnt );
          if( iLen != 1 )
            return( apkFailed );

          // check range
          ptrobj += cnt;
          if( (*ptrobj == 0) || (*ptrobj > 10) )
          //return( apkFailed );
            return( apkIncorrect ); // EMV2000
          }

        // check optional data object: Language Preference
        ptrobj = apk_FindTag( 0x5F, 0x2D, &fci[2] );
        if( ptrobj != 0 )
          {
          iLen = apk_GetBERLEN( ptrobj, &cnt );
          if( iLen != 0 )	// 2018-04-18, AMEX test case: AXP_EMV_017_2___Magstripe_Fallback_Empty_Candidate_List
            {
            if( (iLen < 2) || (iLen > 8) )
              return( apkFailed );
            }
          }

        // check optional data object: Issuer Code Table Index (1~10)
        ptrobj = apk_FindTag( 0x9F, 0x11, &fci[2] );
        if( ptrobj != 0 )
          {
          // check length
          iLen = apk_GetBERLEN( ptrobj, &cnt );
          if( iLen != 1 )
          //return( apkFailed );
            return( apkIncorrect ); // EMV2000
          else
            {
            ptrobj += cnt;
            apk_WriteRamDataICC( ADDR_ICC_ISU_CTI, 2, (UCHAR *)&iLen );
            apk_WriteRamDataICC( ADDR_ICC_ISU_CTI+2, iLen, ptrobj );
            }

          // check range
          // PATCH: 2005/02/25, EMV2000: BOOK1-12.2.5
//        ptrobj += cnt;
//        if( (*ptrobj == 0) || (*ptrobj > 10) )
//          return( apkFailed );
          }
        }

      return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: retrieve one BER-TLV from a specified template record.
// INPUT   : reclen - length of record. (2-byte integer)
//           ptrobj - pointer to the current data object. (T-L-V)
//           padlen - length of padding data objects. (0=no padding present)
// OUTPUT  : reclen - remaining length of record after retrieving one TLV.
//           tlv    - the current data object retrieved. (Tag1-Tag2-L1-L2-V)
//                    if single byte tag only, then Tag1=0, Tag2 codes the tag.
//                    L1=LSB, L2=MSB.
// RETURN  : Found  - the pointer to the next tag's body (T-L-V).
//           Error  - a NULL pointer (-1)
//           EOF    - a NULL pointer (0).
// NOTE    : supporting ISO padding bytes (0x00 or 0xff) between data objects,
//           i.e., the following cases will appear in "ptrobj".
//           (1) 0x00...0x00 TAG-L-V.
//           (2) 0xFF...0xFF TAG-L-V.
// ---------------------------------------------------------------------------
//UCHAR *apk_GetBERTLV3( UCHAR reclen[], UCHAR *ptrobj, UCHAR *tlv, UCHAR padlen  )
//{
//UCHAR i;
//UCHAR len1, len2, len3;
//UINT  iPileLen;
//UCHAR tag1, tag2;
//UINT  iLen;
//
//      iLen = reclen[1]*256 + reclen[0];
//
//      if( iLen < 2 )
//        return( (UCHAR *)0 ); // end of record
//
//      tag1 = *ptrobj++;
//
//      // check ISO padding
//      len1 = 0;
//      if( (tag1 == 0x00) || (tag1 == 0xff) )
//        {
//        len1 = 1;
//
//        while(1)
//             {
//             if( tag1 == *ptrobj )
//               {
//               len1++;
//               ptrobj++;
//               }
//             else
//               break;
//             }
//
//        if( len1 != 0 )
//          {
//          iLen -= len1;
//          reclen[0] = iLen & 0x00ff;
//          reclen[1] = (iLen & 0xff00) >> 8;
//
//          tag1 = *ptrobj++;
//          }
//        }
//
//      if( apk_CheckWordTag( tag1 ) == TRUE ) // double-byte tag?
//        {
//        tag2 = *ptrobj++;
//        len1 = *ptrobj++;
//        len2 = 0;
//        len3 = 3;
//
//        if( (len1 & 0x80) != 0 ) // chained length field?
//          {
//          switch( len1 & 0x7f )
//                {
//                case 0x01: // 1-byte length
//                     len1 = *ptrobj++;
//                     len3 += 1;
//                     break;
//
//                case 0x02: // 2-byte length
//                     len2 = *ptrobj++;
//                     len1 = *ptrobj++;
//                     len3 += 2;
//                     break;
//
//                default:   // out of spec
//                     return( (UCHAR *)0 );
//                }
//          }
//        }
//      else // single-byte tag
//        {
//        tag2 = tag1;
//        tag1 = 0;
//        len1 = *ptrobj++;
//        len2 = 0;
//        len3 = 2;
//
//        if( (len1 & 0x80) != 0 ) // chained length field?
//          {
//          switch( len1 & 0x7f )
//                {
//                case 0x01: // 1-byte length
//                     len1 = *ptrobj++;
//                     len3 += 1;
//                     break;
//
//                case 0x02: // 2-byte length
//                     len2 = *ptrobj++;
//                     len1 = *ptrobj++;
//                     len3 += 2;
//                     break;
//
//                default:   // out of spec
//                     return( (UCHAR *)0 );
//                }
//          }
//        }
//
//      iPileLen = len1 + len2 + len3;
//      if( iLen < iPileLen )
//        {
//        if( (iLen + padlen) < iPileLen )
//          return( (UCHAR *)-1 ); // invalid length
//        else
//          return( (UCHAR *)0 );  // within the padding data objects, seems ok
//        }
//
//      // pick up current TLV
//      *tlv++ = tag1;
//      *tlv++ = tag2;
//      *tlv++ = len1;
//      *tlv++ = len2;
//      for( i=0; i<(len1+len2); i++ )
//         *tlv++ = *ptrobj++; // DATA[]
//
//      // update the remaining length
//      iLen -= iPileLen;
//      reclen[0] = iLen & 0x00ff;
//      reclen[1] = (iLen & 0xff00) >> 8;
//
//      return( ptrobj ); // ok
//}
//
// ---------------------------------------------------------------------------
//UCHAR apk_ParseTLV2( UCHAR reclen[], UCHAR *ptrobj, UCHAR padlen )
//{
//UCHAR *ptrnext;
//UCHAR *ptrdo;
//UCHAR tlv[261];
//UCHAR len[2];
//UINT  iLen;
//
//      len[0] = reclen[0];
//      len[1] = reclen[1];
//      ptrdo = ptrobj;
//
//// TL_DispHexByte(0,0,padlen);
//// TL_DumpHexData(0,1,2, len);
//// TL_DumpHexData(0,0,16, ptrobj);
//
//      while(1)
//           {
//           // read one data element
//           ptrnext = apk_GetBERTLV2( &len[0], ptrdo, tlv, padlen );
//           if( ptrnext == (UCHAR *)-1 )
//             return( FALSE );
//
//           if( ptrnext != 0 )
//             {
//             ptrdo = ptrnext; // update next data object
//
//             iLen = len[1]*256 + len[0]; // remaining length
//             if( (iLen == 1) || (iLen == 2) )
//               return( FALSE );
//             }
//           else
//             return( TRUE );
//           }
//}
//
// ---------------------------------------------------------------------------
// FUNCTION: check length and iso right padding. (0x00 or 0xFF)
// INPUT   : rec    - 2L1-V1-SW1-SW2, the template record read.
//                    V1 = T-L2-V..., T = template tag (1-byte)
//                    the legal template shall be one of the followings:
//                    (1) L1 - 2 = L2 + 1 + C1, C1=sizeof(L2) or
//                    (2) (L1 - 2) - (L2 + 1 + C1) > 0 with right padding.
// OUTPUT  : padlen - length of padding data objects.
// RETURN  : TRUE  - legal data structure.
//           FALSE - illegal data structure.
// ---------------------------------------------------------------------------
//UCHAR apk_CheckIsoPadding_Right2( UCHAR *rec, UCHAR *padlen )
//{
//UINT  i;
//UCHAR c1;
//UCHAR padding;
//UCHAR *ptrpad;
//UINT  iL1, iL2, iL3;
//
//// TL_DumpHexData(0,0,18, rec);
//
//      iL1 = rec[1]*256 + rec[0];           // record length
//      iL2 = apk_GetBERLEN( &rec[3], &c1 ); // template length
//
//      *padlen = 0; // PATCH: 2005/04/11
//
//      if( iL1 < (1 + c1 + 2) ) // T-L-SW1-SW2
//        return( FALSE );
//
//// UI_ClearScreen();
//// TL_DispHexWord(0,0,iL1);
//// TL_DispHexWord(0,5,c1);
//// TL_DispHexWord(1,0,iL2);
//// TL_DispHexWord(2,0,iL1-2);
//// TL_DispHexWord(2,5,iL2+1+c1);
//// UI_WaitKey();
//
//      if( (iL1 - 2) != (iL2 + 1 + c1) )
//        {
//        if( (iL1 - 2) > (iL2 + 1 + c1) )
//          {
//          iL3 = (iL1 - 2) - (iL2 + 1 + c1); // padding length
//          *padlen = (UCHAR)iL3;
//
//          ptrpad = rec + 3 + iL2 + c1;
//          padding = *ptrpad;
//
//// TL_DispHexWord(3,0,iL3);
//// TL_DispHexWord(3,5,padding);
//// UI_WaitKey();
//
//          if( (padding != 0x00) && (padding != 0xff) )
//            return( FALSE );
//
//          for( i=0; i<iL3; i++ )
//             {
//             if( *(ptrpad + i) != padding )
//               return( FALSE );
//             }
//          return( TRUE );
//          }
//        else
//          return( FALSE );
//        }
//      else
//        return( TRUE );
//}
//
// ---------------------------------------------------------------------------
//UCHAR apk_ParseLenFCI2( UCHAR *ptrfci, UCHAR *padlen )
//{
//UCHAR i;
//UCHAR c1, c2, c3;
//UINT  iL1, iL2, iL3, iL4;
//UINT  lenfci;
//UCHAR padding;
//UCHAR *ptrobj;
//UCHAR *ptrrec;
//
//      ptrrec = ptrfci;
//      lenfci = ptrfci[1]*256 + ptrfci[0];
//      ptrfci += 2; // pointer to T-L-V
//
//      iL1 = apk_GetBERLEN( ptrfci+1, &c1 );       // FCI Template
//
//      ptrobj = apk_FindTag( 0x84, 0x00, ptrfci ); // DF Name
//      iL2 = apk_GetBERLEN( ptrobj, &c2 );
//
//      // check if "A5" is just following "84"
//      iL4 = 0; // length of unexpected data objects btw A5 & 84
//      if( *(ptrobj + iL2 + c2) != 0xA5 )
//        {
//        while( *(ptrobj + iL2 + c2) != 0xA5 )
//             {
//             iL4++;
//             ptrobj++;
//             }
//        }
//
//// TL_DispHexWord(0,0,iL4);
//
//      ptrobj = apk_FindTag( 0xA5, 0x00, ptrfci ); // FCI Prorietary Template
//      iL3 = apk_GetBERLEN( ptrobj, &c3 );
//
//// TL_DispHexWord(1,0,iL3);
//// TL_DispHexByte(1,5,c3);
//// TL_DumpHexData(0,2,8, ptrobj);
//
//      // (1) lenfci - 2 = L1 + 1 + C1 or
//      // (2) ISO rigth padding (0x00 or 0xFF)
//
//      if( apk_CheckIsoPadding_Right2( ptrrec, padlen ) == FALSE )
//        return( FALSE );
//
////    if( (lenfci - 2) != (iL1 + 1 + c1) )
////      {
////      if( (lenfci - 2) > (iL1 + 1 + c1) )
////        {
////        iL4 = (lenfci - 2) - (iL1 + 1 + c1);
////        padding = *(ptrfci + 2 + iL1);
////
//// TL_DispHexWord(0,0,iL4);
//// TL_DispHexByte(1,0,padding);
//// UI_WaitKey();
////
////        if( (padding != 0x00) && (padding != 0xff) )
////          return( FALSE );
////
////        for( i=0; i<iL4; i++ )
////           {
////           if( *(ptrfci + 2 + iL1 + i) != padding )
////             return( FALSE );
////           }
////        return( TRUE );
////        }
////      else
////        return( FALSE );
////      }
//
//      // L1 - (1 + L2 + C2) = L3 + 1 +C3
//
//      if( (iL1 - (1 + iL2 + c2) - iL4) != (iL3 + 1 + c3) )
//        return( FALSE );
//
//      return( TRUE );
//}
// ---------------------------------------------------------------------------
// FUNCTION: Terminal selects a DDF.
// INPUT   : ddfname - 1L-V, the DDF name.
// OUTPUT  : fci     - 2L-V, the file control information.
// RETURN  : apkOK               (9000, 61La)
//           apkFileInvalidated  (6283) -- application blocked
//           apkFuncNotSupported (6A81) -- card blocked
//           apkFileNotFound     (6A82)
//           apkFailed
//           apkIncorrect        (mandatory data objects missing)
//           apkNotReady         (DDF entry failed)
// ---------------------------------------------------------------------------
UCHAR apk_SelectDDF( UCHAR *fci, UCHAR *ddfname )
{
UCHAR buf[15];
UCHAR len;
UCHAR i;
UCHAR result;
UCHAR *ptrobj;
UCHAR rec_len[2];
UCHAR cnt;
UCHAR padlen[1];
UINT  iLen;

      // --- DEBUG ONLY ---
#ifdef  L2_SW_DEBUG

UCHAR rapdu[]={ 0x6f, 0x15,
                0x84, 0x0e, 0x32, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59, 0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31,
                0xa5, 0x03,
                0x88, 0x01, 0x01,
                0x90, 0x00};

      fci[0] = 25;
      fci[1] = 0;

      memmove( &fci[2], rapdu, 25 );
      return( 0 );

#endif
      // ------------------
//    fci[0] = 74;
//    fci[1] = 0;
//    fci[2] = 0x6F;
//    fci[3] = 0x46;
//    fci[4] = 0x84;
//    fci[5] = 0x05;
//    fci[6] = 0xA0;
//    fci[7] = 0x00;
//    fci[8] = 0x11;
//    fci[9] = 0x11;
//    fci[10] = 0x11;
//    fci[11] = 0xA5;
//    fci[12] = 0x3D;
//    fci[13] = 0x88;
//    fci[14] = 0x01;
//    fci[15] = 0x01;
//    fci[16] = 0xBF;
//    fci[17] = 0x0C;
//    fci[18] = 0x37;
//    fci[19] = 0x5F;
//    fci[20] = 0x54;
//    fci[21] = 0x0B;
//    fci[22] = 0x42;
//    fci[23] = 0x41;
//    fci[24] = 0x4E;
//    fci[25] = 0x4B;
//    fci[26] = 0x55;
//    fci[27] = 0x53;
//    fci[28] = 0x32;
//    fci[29] = 0x31;
//    fci[30] = 0x4F;
//    fci[31] = 0x4E;
//    fci[32] = 0x45;
//    fci[33] = 0x5F;
//    fci[34] = 0x53;
//    fci[35] = 0x16;
//    fci[36] = 0x55;
//    fci[37] = 0x53;
//    fci[38] = 0x30;
//    fci[39] = 0x30;
//    fci[40] = 0x31;
//    fci[41] = 0x31;
//    fci[42] = 0x32;
//    fci[43] = 0x32;
//    fci[44] = 0x33;
//    fci[45] = 0x33;
//    fci[46] = 0x34;
//    fci[47] = 0x34;
//    fci[48] = 0x35;
//    fci[49] = 0x35;
//    fci[50] = 0x36;
//    fci[51] = 0x36;
//    fci[52] = 0x37;
//    fci[53] = 0x37;
//    fci[54] = 0x38;
//    fci[55] = 0x38;
//    fci[56] = 0x39;
//    fci[57] = 0x39;
//    fci[58] = 0x5F;
//    fci[59] = 0x55;
//    fci[60] = 0x02;
//    fci[61] = 0x55;
//    fci[62] = 0x53;
//    fci[63] = 0x5F;
//    fci[64] = 0x56;
//    fci[65] = 0x03;
//    fci[66] = 0x55;
//    fci[67] = 0x53;
//    fci[68] = 0x41;
//    fci[69] = 0x42;
//    fci[70] = 0x03;
//    fci[71] = 0x09;
//    fci[72] = 0x18;
//    fci[73] = 0x27;
//    fci[74] = 0x90;
//    fci[75] = 0x00;
//
//    return(apkOK);
      // ------------------

      len = *ddfname++;
      buf[0] = len;

      for( i=0; i<len; i++ )
         buf[i+1] = *ddfname++;

      result = SPSE_SelectFile( 2, buf, 0, fci );

//    ptrobj = apk_FindTag( 0xA5, 0x00, &fci[2] );
//    iLen = apk_GetBERLEN( ptrobj, &cnt );
//    rec_len[0] = iLen & 0x00ff;
//    rec_len[1] = (iLen & 0xff00) >> 8;
//    ptrobj += cnt; // ptr to the 1'st DO of FCI Proprietary Template

      if( result == apkOK )
        {
        ptrobj = apk_FindTag( 0xA5, 0x00, &fci[2] );
        iLen = apk_GetBERLEN( ptrobj, &cnt );
        rec_len[0] = iLen & 0x00ff;
        rec_len[1] = (iLen & 0xff00) >> 8;
        ptrobj += cnt; // ptr to the 1'st DO of FCI Proprietary Template

        // parse all data objects
        if( apk_ParseLenFCI( fci, padlen ) == FALSE )
       // return( apkFailed );
          return( apkIncorrect ); // PATCH: PBOC2.0, 2006-02-15, 2CL.029.01.01

        if( apk_ParseTLV( &rec_len[0], ptrobj, padlen[0] ) == FALSE )
          return( apkIncorrect ); // EMV2000

        // check mandatory data object: SFI (1~10)
        ptrobj = apk_FindTag( 0x88, 0x00, &fci[2] );

        if( ptrobj == 0 )
          return( apkIncorrect ); // EMV2000
        else
          {
          // check length
          iLen = apk_GetBERLEN( ptrobj, &cnt );
          if( iLen != 1 )
            return( apkFailed );

          // check range
          ptrobj += cnt;
          if( (*ptrobj == 0) || (*ptrobj > 10) )
          //return( apkFailed );
            return( apkIncorrect ); // EMV2000
          }
        }

      return( result );
}
//return is TRUE -> have repeat
//remove inline Wayne
static UCHAR apk_checkRepeat( UCHAR tag1, UCHAR tag2, UCHAR *data )
//UCHAR *apk_FindTag( UCHAR tag1, UCHAR tag2, UCHAR *data )
{
UCHAR tmptag;
UCHAR tmptag2;
UCHAR tag;    		// current tag value
UINT  tmplen=0;		// length of template object
UINT  taglen;		// length of current tag object
UCHAR len_offset;
UCHAR len1;
int count = 0;
      // check constructed template
FIND_NEXT_TAG:

      while(1)
      {
           tag = *data++;
           // check ISO padding
           len1 = 0;
           if( (tag == 0x00) || (tag == 0xff) )
           {
             len1 = 1;

             while(1)
             {
                  if( tag == *data )
                  {
                    len1++;
                    data++;
                  }
                  else
                    break;
             }

             if( len1 != 0 )
             {
               tag = *data++;
             }
          }

           // check template TAG
           if( (tag == 0x61) || (tag == 0x6f) || (tag == 0x70) ||
               (tag == 0x77) || (tag == 0x80) || (tag == 0xa5) ||
               ((tag & 0x20) && ((tag & 0x1f) != 0x1f)) )
           {
             tmptag = tag;   // template tag
             tmplen = *data; // template length
             len_offset = 1;

             if( tmplen >= 0x80 )
             {
               switch( tmplen & 0x7f )
               {
                     case 0x01: // 1-byte length
                          tmplen = *(data+1);
                          len_offset = 2;
                          break;

                     case 0x02: // 2-byte length
                          tmplen = (*(data+1))*256; // msb
                          tmplen += *(data+2);       // lsb
                          len_offset = 3;
                          break;

                     default:   // out of spec
                          return( 0xff );
                }
             }

             if( (tmptag == tag1) && (tag2 == 0) )
               //return( data );
            	 count++;
             else
               data += len_offset;
           }
           else if((tag==0xBF)&&(*data==0x0c))
           {
               tmptag2 = *data++;
               tmptag = tag;   // template tag
               tmplen = *data; // template length
               len_offset = 1;

               if( tmplen >= 0x80 )
               {
                 switch( tmplen & 0x7f )
                 {
                       case 0x01: // 1-byte length
                            tmplen = *(data+1);
                            len_offset = 2;
                            break;

                       case 0x02: // 2-byte length
                            tmplen = (*(data+1))*256; // msb
                            tmplen += *(data+2);       // lsb
                            len_offset = 3;
                            break;

                       default:   // out of spec
                            return( 0xff );
                  }
               }

               if( (tmptag == tag1) && (tag2 == tmptag2) )
                 //return( data );
              	 count++;
               else
                 data += len_offset;
           }
           else// check template TAG
             break;
           }

      // matching the specified tag with current single or word tag.
      if( (tag == 0x5f) || (tag == 0x9f) || (tag == 0xbf) || ((tag & 0x1f) == 0x1f) ) // meet a word tag?
        {  // match word tag
        tmptag2 = *data++;
        len_offset = 3;

        if( tag2 != 0 )
          {
          if( (tag == tag1) && (tmptag2 == tag2) )
            //return( data ); // found
        	  count++;
          }
        }
      else // meet a single tag
        {
        len_offset = 2;

        if( tag2 == 0 ) // to find single tag
          {
          if( tag == tag1 )
            //return( data ); // found
        	  count++;
          }
        }

      taglen = *data++;

      if( (taglen & 0x80) != 0 )
        {
        switch( taglen & 0x7f )
              {
              case 0x01: // 1-byte length
                   taglen = *data++;
                   len_offset += 1;
                   break;

              case 0x02: // 2-byte length
                   taglen = (*data)*256; // msb
                   data++;
                   taglen += *data++;    // lsb
                   len_offset += 2;
                   break;

              default:   // out of spec
                   return( 0xff );
              }
        }

      data += taglen; // ptr to next tag

      if( tmplen <= (taglen + len_offset) )
        //return ( (UCHAR *)0 );  // not found
    	  if(count>1)
    		  return TRUE;
    	  else
    		  return FALSE;

      tmplen -= (taglen + len_offset);

      goto FIND_NEXT_TAG;

}

// ---------------------------------------------------------------------------
// FUNCTION: Terminal selects a ADF by name.
// INPUT   : dfname     - 1L-V, the DF name.
//           occurrence - 0=1'st, 1=next.
// OUTPUT  : fci        - 2L-V, the file control information.
// RETURN  : apkOK               (9000, 61La)
//           apkFileInvalidated  (6283) -- application blocked
//           apkFuncNotSupported (6A81) -- card blocked
//           apkFileNotFound     (6A82)
//           apkFailed
//           apkIncorrect        (mandatory data objects missing)
// ---------------------------------------------------------------------------
UCHAR apk_SelectADF( UCHAR *fci, UCHAR *dfname, UCHAR occurrence )
{
UCHAR i;
UCHAR buf[17];
UCHAR rec_len[2];
UCHAR len;
UINT  iLen;//20090326_Charles:Modifying the data type of iLen from UCHAR to UINT
UCHAR result;
UCHAR *ptrobj;
UCHAR *ptrnext;
UCHAR cnt;
UCHAR padlen[1];

        //--- DEBUG ONLY ---
#ifdef  L2_SW_DEBUG

// without PDOL
//UCHAR rapdu[]={ 0x6f, 0x20,
//                0x84, 0x07, 0xa0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10,
//                0xa5, 0x15,
//                0x50, 0x0b, 0x56, 0x49, 0x53, 0x41, 0x20, 0x43, 0x52, 0x45, 0x44, 0x49, 0x54,
//                0x87, 0x01, 0x01,
//                0x5f, 0x2d, 0x02, 0x7a, 0x68,
//                0x90, 0x00};
//
//      fci[0] = 36;
//      fci[1] = 0;
//
//      memmove( &fci[2], rapdu, 36 );
//      return( 0 );

// with PDOL
UCHAR rapdu[]={ 0x6f, 0x28,
                0x84, 0x07, 0xa0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10,
                0xa5, 0x1d,
                0x50, 0x0b, 0x56, 0x49, 0x53, 0x41, 0x20, 0x43, 0x52, 0x45, 0x44, 0x49, 0x54,
                0x87, 0x01, 0x01,
                0x5f, 0x2d, 0x02, 0x7a, 0x68,
                0x9f, 0x38, 0x05, 0x9f, 0x02, 0x06, 0x95, 0x05,
                0x90, 0x00};

      fci[0] = 36+8;
      fci[1] = 0;

      memmove( &fci[2], rapdu, 36+8 );
      return( 0 );

#endif
        //------------------

      len = *dfname++;
      buf[0] = len;

      for( i=0; i<len; i++ )
         buf[i+1] = *dfname++;

      result = SPSE_SelectFile( 1, buf, occurrence, fci );

// result = apkFileInvalidated;
// fci[0] = 2;
// fci[1] = 0;
// fci[2] = 0x62;
// fci[3] = 0x83;

//    ptrobj = apk_FindTag( 0xA5, 0x00, &fci[2] );
//    iLen = apk_GetBERLEN( ptrobj, &cnt );
//    rec_len[0] = iLen & 0x00ff;
//    rec_len[1] = (iLen & 0xff00) >> 8;
//    ptrobj += cnt; // ptr to the 1'st DO of FCI Proprietary Template

      if( result == apkOK )
        {
          // parse all data objects
          if( apk_ParseLenFCI( fci, padlen ) == FALSE )
            //return( apkFailed );
          	return apkIncorrect;//2009-03-03 EMV4.2a Charles 2ce.003.08 case1

        ptrobj = apk_FindTag( 0xA5, 0x00, &fci[2] );
        iLen = apk_GetBERLEN( ptrobj, &cnt );
        rec_len[0] = iLen & 0x00ff;
        rec_len[1] = (iLen & 0xff00) >> 8;
        ptrobj += cnt; // ptr to the 1'st DO of FCI Proprietary Template
        if(iLen!=0)
        {//EMV 42a 2009-03-06 Charles 2ca.098.00
        if( apk_ParseTLV( &rec_len[0], ptrobj, padlen[0] ) == FALSE )
          return( apkIncorrect );//2008-DEC-25 Charles
        }
		//if(apk_ParseTemplate(fci)== FALSE)//2009-1-8 charles
        if(FCI_Parse(2,fci)==FALSE)//EMV 42a 2009-03-07 Charles 2ca.099.00 2cL.032.00
			return apkIncorrect;

		if(apk_checkRepeat(0x9f,0x38,&fci[2])==TRUE)//2008-DEC-25 Charles
			return apkIncorrect;
		if(apk_checkRepeat(0x9f,0x4d,&fci[2])==TRUE)//EMV 42a 2009-03-03 Charles 2ce.003.08 case 7
			return apkIncorrect;
        // check optional data object: Application Priority Indicator
        ptrobj = apk_FindTag( 0x87, 0x00, &fci[2] );
        if( ptrobj != 0 )
          {
          iLen = apk_GetBERLEN( ptrobj, &cnt );
          if( iLen != 1 )
        //  return( apkFailed );       // EMV    SPEC
            return( apkFileNotFound ); // JSmart SPEC
          }

        // check optional data object: Language Preference
        ptrobj = apk_FindTag( 0x5F, 0x2D, &fci[2] );
        if( ptrobj != 0 )
          {
          iLen = apk_GetBERLEN( ptrobj, &cnt );
          if( (iLen < 2) || (iLen > 8) )
            return( apkFailed );
          }

        // check optional data object: Issuer Code Table Index
        ptrobj = apk_FindTag( 0x9F, 0x11, &fci[2] );
        if( ptrobj != 0 )
          {
          iLen = apk_GetBERLEN( ptrobj, &cnt );
          if( iLen != 1 )
            return( apkFailed );
          else
            {
            ptrobj += cnt;
            apk_WriteRamDataICC( ADDR_ICC_ISU_CTI, 2, (UCHAR *)&iLen );
            apk_WriteRamDataICC( ADDR_ICC_ISU_CTI+2, iLen, ptrobj );
            }
          }
        }

      return( result );
}

// ---------------------------------------------------------------------------
// type = 0 -- select PSE file
//      = 1 -- select ADF
//      = 2 -- select DDF
// ---------------------------------------------------------------------------
UCHAR SPSE_SelectFile( UCHAR type, UCHAR *name, UCHAR occurrence, UCHAR *fci )
{
UCHAR result;
UINT  iLength;
UINT  iLen;
UCHAR cnt;
UCHAR *ptrobj;


      g_emv_SW1 = 0;
      g_emv_SW2 = 0;
      
      if( apdu_SELECT( name, occurrence, fci ) != apkOK )
        return( apkFailed );

      iLength = fci[1]*256 + fci[0]; // template length

      if( iLength > 2 )
        {
        // check mandatory data object: FCI Template
        if( fci[2] != 0x6F )
        //return( apkFailed );
          return( apkIncorrect ); // EMV2000

        // check mandatory data object: DF Name
        ptrobj = apk_FindTag( 0x84, 0x00, &fci[2] );
        if( ptrobj == 0 )
        //return( apkFailed );
          return( apkIncorrect ); // EMV2000
        else
          {
          iLen = apk_GetBERLEN( ptrobj, &cnt );
          if( (iLen < 5) || (iLen > 16) )
            return( apkIncorrect );//08-DEC-25 Charles

          // PATCH: 2003-06-11, JSmart CAP07
          // check the repetitive DF names in FCI
          // ABNORMAL FORMAT: 6F 84 XX...XX 84 XX...XX A5...
          ptrobj += (iLen + cnt); // ptr to next TLV
          if( *ptrobj == 0x84 )
            return( apkFileNotFound ); // invalid: repetitive DF name
          }

        // check mandatory data object: FCI Proprietary Template
        if( apk_FindTag( 0xA5, 0x00, &fci[2] ) == 0 )
        //return( apkFailed );
          return( apkIncorrect ); // EMV2000

	g_emv_SW1 = fci[iLength+0];
	g_emv_SW2 = fci[iLength+1];
	
        // check returned status word
        if( ( fci[iLength] == 0x90 ) && ( fci[iLength+1] == 0x00 ) )
          return( apkOK );

        if( fci[iLength] == 0x61 )
          return( apkOK );

        if( ( fci[iLength] == 0x62 ) && ( fci[iLength+1] == 0x83 ) )
          return( apkFileInvalidated );
        if( ( fci[iLength] == 0x6a ) && ( fci[iLength+1] == 0x81 ) )
          return( apkFuncNotSupported );
        else
          {
//        return( apkFailed );
//        return( apkFuncNotSupported );

          if( type == 0 )         // PATCH: 2005/03/21, EMV2000
            //return( apkUnknown ); // select PSE, eg.2CB.014.63C0
        	  return apkIncorrect;// 2009-03-02 charles tsai 2cb.022.00 case 3~10 2ca.033.02 CASE 11


          if( type == 2 ) // select DDF
            return( apkNotReady );
          else
//          return( apkFuncNotSupported ); // select ADF
            return( apkUnknown ); // PATCH: 2006-10-09
          }
        }
      else // return only SW1 SW2
        {
        if( iLength != 2 )
          return( apkFailed );

	g_emv_SW1 = fci[2];
	g_emv_SW2 = fci[3];
	
        if( ( fci[2] == 0x62 ) && ( fci[3] == 0x83 ) )  // PATCH: PBOC2.0, 2006-02-15, application blocked
          return( apkFileInvalidated );                 //

        if( ( fci[2] == 0x6a ) && ( fci[3] == 0x81 ) )
          return( apkFuncNotSupported );

        if( ( fci[2] == 0x6a ) && ( fci[3] == 0x82 ) )
          return( apkFileNotFound );

        if( ( fci[2] == 0x90 ) && ( fci[3] == 0x00 ) )  // PATCH: 2006-10-05
          return( apkFciMissing );
        else
          {
        	if(type==0||type==2)
        		return apkIncorrect;//09-3-3 charles 2ca.033.02 CASE 11
      //  return( apkFailed );  // EMV SPEC
          return( apkUnknown ); // JCB SPEC
          }
        }
}

// ---------------------------------------------------------------------------
//UCHAR apdu_SELECT2( UCHAR *filename, UCHAR occurrence, UCHAR *fci )
//{
//UCHAR i;
//UCHAR c_apdu[24];
//
//      c_apdu[0] = filename[0] + 6;      // length of APDU
//      c_apdu[1] = 0x00;                 //
//      c_apdu[2] = 0x00;                     // CLA
//      c_apdu[3] = 0xa4;                     // INS
//      c_apdu[4] = 0x04;                     // P1
//      c_apdu[5] = (occurrence << 1) & 0x03; // P2
//      c_apdu[6] = filename[0];              // Lc
//      for(i=0; i<filename[0]; i++)          // Data=filename (5~16 bytes)
//         c_apdu[7+i] = filename[1+i];       //
//      c_apdu[7+i] = 0x00;                   // Le
//
//    return(emvOK);
//
//      return( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, fci ) );
//}
// ---------------------------------------------------------------------------
//UCHAR SPSE_SelectFile2( UCHAR type, UCHAR *name, UCHAR occurrence, UCHAR *fci )
//{
//UCHAR result;
//UINT  iLength;
//UINT  iLen;
//UCHAR cnt;
//UCHAR *ptrobj;
//
////  return(apkOK);
//
//      if( apdu_SELECT2( name, occurrence, fci ) != apkOK )
//        return( apkFailed );
//
//    return(apkOK);
//
//      iLength = fci[1]*256 + fci[0]; // template length
//
//      if( iLength > 2 )
//        {
//        // check mandatory data object: FCI Template
//        if( fci[2] != 0x6F )
//        //return( apkFailed );
//          return( apkIncorrect ); // EMV2000
//
//        // check mandatory data object: DF Name
//        ptrobj = apk_FindTag( 0x84, 0x00, &fci[2] );
//        if( ptrobj == 0 )
//        //return( apkFailed );
//          return( apkIncorrect ); // EMV2000
//        else
//          {
//          iLen = apk_GetBERLEN( ptrobj, &cnt );
//          if( (iLen < 5) || (iLen > 16) )
//            return( apkFailed );
//
//          // PATCH: 2003-06-11, JSmart CAP07
//          // check the repetitive DF names in FCI
//          // ABNORMAL FORMAT: 6F 84 XX...XX 84 XX...XX A5...
//          ptrobj += (iLen + cnt); // ptr to next TLV
//          if( *ptrobj == 0x84 )
//            return( apkFileNotFound ); // invalid: repetitive DF name
//          }
//
//        // check mandatory data object: FCI Proprietary Template
//        if( apk_FindTag( 0xA5, 0x00, &fci[2] ) == 0 )
//        //return( apkFailed );
//          return( apkIncorrect ); // EMV2000
//
//        // check returned status word
//        if( ( fci[iLength] == 0x90 ) && ( fci[iLength+1] == 0x00 ) )
//          return( apkOK );
//
//        if( fci[iLength] == 0x61 )
//          return( apkOK );
//
//        if( ( fci[iLength] == 0x62 ) && ( fci[iLength+1] == 0x83 ) )
//          return( apkFileInvalidated );
//        if( ( fci[iLength] == 0x6a ) && ( fci[iLength+1] == 0x81 ) )
//          return( apkFuncNotSupported );
//        else
//          {
////        return( apkFailed );
////        return( apkFuncNotSupported );
//
//          if( type == 0 )         // PATCH: 2005/03/21, EMV2000
//            return( apkUnknown ); // select PSE, eg.2CB.014.63C0
//          else
//            return( apkFuncNotSupported ); // select ADF or DDF
//          }
//        }
//      else // return only SW1 SW2
//        {
//        if( iLength != 2 )
//          return( apkFailed );
//
//        if( ( fci[2] == 0x6a ) && ( fci[3] == 0x81 ) )
//          return( apkFuncNotSupported );
//
//        if( ( fci[2] == 0x6a ) && ( fci[3] == 0x82 ) )
//          return( apkFileNotFound );
//        else
//      //  return( apkFailed );  // EMV SPEC
//          return( apkUnknown ); // JCB SPEC
//        }
//}

