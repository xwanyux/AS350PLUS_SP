/*
============================================================================
****************************************************************************
**                                                                        **
**  PROJECT  : EMV L2                                                     **
**  PRODUCT  : AS320-A                                                    **
**                                                                        **
**  FILE     : APK_FTAG.C                                                 **
**  MODULE   : apk_FindTag()                                              **
**             apk_FindTagDOL()                                           **
**             apk_CheckConsTag()                                         **
**             apk_CheckWordTag()                                         **
**             apk_CheckTermTag()                                         **
**             apk_CheckIccTag()                                          **
**             apk_CheckIssuerTag()                                       **
**             apk_GetBERLEN()                                            **
**             apk_SetBERLEN()                                            **
**             apk_GetBERTLV()                                            **
**             apk_ScanIDE()                                              **
**  VERSION  : V1.00                                                      **
**  DATE     : 2002/12/07                                                 **
**  EDITOR   : James Hsieh                                                **
**                                                                        **
**  Copyright(C) 2002-2007 SymLink Corporation. All rights reserved.      **
**                                                                        **
**HIESTORY: 2008-dec-26 charles �s�W�ˬdtemplate���禡
**
****************************************************************************
============================================================================
HISTORY:
08-dec-23	charles tsai
			modify function apk_ParseLenPSEDIR()
----------------------------------------------------------------------------*/
#include <string.h>

#include "POSAPI.h"
//#include "EMVDC.h"
#include "GDATAEX.h"
#include "EMVAPI.h"
//#include <EMVAPK.h>
//#include <APDU.H>
//#include <TOOLS.h>

// ---------------------------------------------------------------------------
// FUNCTION: Locate the specified tag in a data object.
// INPUT   : tag1 - the tag (or HI byte of word tag) to be found.
//           tag2 - the LOW byte of word tag. (NULL if not used)
//           data - T-L-V, the "constructed" data object.
// OUTPUT  : none.
// RETURN  : Found     - the pointer to the specified tag's body (L-V).
//           Not Found - a NULL pointer (0).
//
// NOTE    : the ICC constructed template:
//           61, 6F, 70, 77, 80, A5.
//
//           61    -> 4F, 50, 52, 73, 9D, 9F12
//           6F    -> 84, [A5]
//           70/77 -> 57, 5A, 5F20, 5F24, 5F25, 5F28, 5F30, 5F34, [61], 8C, 8D,
//                    8E, 8F, 90, 92, 93, 94, 97, 9F05, 9F07, 9F08, 9F0B, 9F0D,
//                    9F0E, 9F0F, 9F14, 9F1F, 9F20, 9F23, 9F2D, 9F2E, 9F2F,
//                    9F32, 9F3B, 9F42, 9F43, 9F44, 9F46, 9F47, 9F48, 9F49,
//                    9F4A
//           77/80 -> 82, 94, 9F10, 9F26, 9F27, 9F36
//           A5    -> 50, 5F2D, 88, 9F11, 9F38, BF0C
//
//           word tag: 5Fxx, 9Fxx, BFxx.
// ---------------------------------------------------------------------------
UCHAR *apk_FindTag( UCHAR tag1, UCHAR tag2, UCHAR *data )
{
UCHAR tmptag;
UCHAR tmptag2;
UCHAR tag;    		// current tag value
UINT  tmplen=0;		// length of template object
UINT  taglen;		// length of current tag object
UCHAR len_offset;
UCHAR len1;

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

//     UI_ClearScreen();
//     TL_DispHexByte(0,0,tag);
//     TL_DispHexByte(1,0,len1);
//     TL_DispHexByte(3,0,0x22);
//     UI_WaitKey();
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
                          return( (UCHAR *)0 );
                     }
               }

             if( (tmptag == tag1) && (tag2 == 0) )
               return( data );
             else
               data += len_offset;
             }
           else // non-constructed tag
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
            return( data ); // found
          }
        else
          {
          if( (tag & 0x1f) == 0x1f )	// PATCH: 2010-04-09, the 2'nd tag is 0 for the word tag, such as DF00
            {
            if( (tag == tag1) && (tmptag2 == tag2) )
              return( data ); // found
            }
          }
        }
      else // meet a single tag
        {
        len_offset = 2;

        if( tag2 == 0 ) // to find single tag
          {
          if( tag == tag1 )
            return( data ); // found
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
                   return( (UCHAR *)0 );
              }
        }

      data += taglen; // ptr to next tag

      if( tmplen <= (taglen + len_offset) )
        return ( (UCHAR *)0 );  // not found

      tmplen -= (taglen + len_offset);

      goto FIND_NEXT_TAG;

}

// ---------------------------------------------------------------------------
// FUNCTION: check if the 1'st tag is a template tag.
//           (bit6 of the first byte tag = 1)
// INPUT   : tag - the tag to be checked.
// OUTPUT  : none.
// RETURN  : TRUE  - yes.
//           FLASE - no.
//
// NOTE    : the ICC constructed template:
//           61, 6F, 70, 77, A5.
//
//           EMV2000: 73 is added
// ---------------------------------------------------------------------------
UCHAR apk_CheckConsTag( UCHAR tag )
{
      if( (tag & 0x20) == 0 )
        return( FALSE );

      if( (tag == 0x61) || (tag == 0x6f) || (tag == 0x70) ||
          (tag == 0x77) || (tag == 0xa5) || (tag == 0x73) )
        return( TRUE );
      else
        return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: check if the 1'st tag is the MSB of word tag.
//           (bit1~5 = 1 of the 1st byte tag)
// INPUT   : tag - the tag to be checked.
// OUTPUT  : none.
// RETURN  : TRUE  - yes.
//           FLASE - no.
//
// NOTE    : word tag: 5Fxx, 9Fxx, BFxx.
// ---------------------------------------------------------------------------
UCHAR apk_CheckWordTag( UCHAR tag )
{
      if( (tag & 0x1f) == 0x1f )
        return( TRUE );
      else
        return( FALSE );

//    if( (tag == 0x5f) || (tag == 0x9f) || (tag == 0xbf) )
//      return( TRUE );
//    else
//      return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: check if the source of a sigle tag is TERMINAL.
// INPUT   : tag - the tag to be checked.
// OUTPUT  : none.
// RETURN  : TRUE  - yes.
//           FLASE - no.
//
// NOTE    : TAG = 81, 8A, 95, 98, 9A, 9B, 9C.
// ---------------------------------------------------------------------------
UCHAR apk_CheckTermTag( UCHAR tag )
{
      if( (tag == 0x81) || (tag == 0x8a) || (tag == 0x95) ||
          (tag == 0x98) || (tag == 0x9a) || (tag == 0x9b) ||
          (tag == 0x9c) )
        return( TRUE );
      else
        return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: check if the source of a sigle tag is ICC.
// INPUT   : tag - the tag to be checked.
// OUTPUT  : none.
// RETURN  : TRUE  - yes.
//           FLASE - no.
//
// NOTE    : TAG = 52, 57, 5A, 8C, 8D, 8E, 8F, 90, 92, 93, 97.
//                 4F, 84, 9D, 50, 87, 82, 94, 42
// ---------------------------------------------------------------------------
UCHAR apk_CheckIccTag( UCHAR tag )
{
      if( (tag == 0x52) || (tag == 0x57) || (tag == 0x5a) ||
          (tag == 0x8c) || (tag == 0x8d) || (tag == 0x8e) ||
          (tag == 0x8f) || (tag == 0x90) || (tag == 0x92) ||
          (tag == 0x93) || (tag == 0x97) || (tag == 0x4f) ||
          (tag == 0x84) || (tag == 0x9d) || (tag == 0x50) ||
          (tag == 0x87) || (tag == 0x82) || (tag == 0x94) ||
          (tag == 0x42) )
        return( TRUE );
      else
        return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: check if the source of a sigle tag is ISSUER.
// INPUT   : tag - the tag to be checked.
// OUTPUT  : none.
// RETURN  : TRUE  - yes.
//           FLASE - no.
//
// NOTE    : TAG = 91 -- IAD.
// ---------------------------------------------------------------------------
UCHAR apk_CheckIssuerTag( UCHAR tag )
{
      if( tag == 0x91 )
        return( TRUE );
      else
        return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: convert UCHAR length to BER length.
// INPUT   : orgLen - orginal length to be converted.
// OUTPUT  : berLen - the BER-TLV format for the "length.
//                    1 or 2 bytes, 1~255. (length > 255 is out of EMV spec)
//                    1~127  : the 1'st byte is same as original value.
//                    128~255: 0x81 NN, NN=original value.
// RETURN  : number of bytes for the BER length.
// ---------------------------------------------------------------------------
UCHAR apk_SetBERLEN( UCHAR orgLen, UCHAR berLen[] )
{
      if( orgLen < 128 )
        {
        berLen[0] = orgLen;
        return( 1 );
        }
      else
        {
        berLen[0] = 0x81;
        berLen[1] = orgLen;
        return( 2 );
        }
}

// ---------------------------------------------------------------------------
// FUNCTION: retrieve the actual length of the specified data element.
// INPUT   : de -  a data element. (L-V, where L is coded according to BER-TLV)
// OUTPUT  : cnt - number of bytes of the L field.
// RETURN  : an integer for the length L.
//           0 = invalid length.
// ---------------------------------------------------------------------------
UINT apk_GetBERLEN( UCHAR *de, UCHAR *cnt )
{
UCHAR len1, len2;

      *cnt = 1;
      len1 = *de++;

      if( (len1 & 0x80) != 0 ) // chained length field?
        {
        switch( len1 & 0x7f )
              {
              case 0x01: // 1-byte length
                   len1 = *de++;
                   len2 = 0;
                   *cnt = 2;
                   break;

              case 0x02: // 2-byte length
                   len2 = *de++;
                   len1 = *de++;
                   *cnt = 3;
                   break;

              default:   // out of spec
                   return( 0 );
              }
        }
      else
        len2 = 0;

      return( len2*256+len1 );
}

// ---------------------------------------------------------------------------
// FUNCTION: retrieve one BER-TLV from a specified template record.
// INPUT   : reclen - length of record. (2-byte integer)
//           ptrobj - pointer to the current data object. (T-L-V)
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
UCHAR *apk_GetBERTLV( UCHAR reclen[], UCHAR *ptrobj, UCHAR *tlv )
{
UCHAR i;
UCHAR len1, len2, len3;
UINT  iPileLen;
UCHAR tag1, tag2;
UINT  iLen;

      iLen = reclen[1]*256 + reclen[0];

      if( iLen < 2 )
        return( (UCHAR *)0 ); // end of record

      tag1 = *ptrobj++;

      // check ISO padding
      len1 = 0;
      if( (tag1 == 0x00) || (tag1 == 0xff) )
        {
        len1 = 1;

        while(1)
             {
             if( tag1 == *ptrobj )
               {
               len1++;

               if( len1 == (UCHAR)iLen )  // PATCH: PBOC2.0, 2006-02-10
                 {
//        TL_DispHexByte(0,0,len1);
//        UI_WaitKey();
            	   reclen[0] = 0;//EMV 42a Charles 2009-03-05 2CA.055.01 case 2

            	   reclen[1] = 0;
                 return( (UCHAR *)0 ); // CASE: V2CI002, TLV..TLV 0..0 90 00
                 }

               ptrobj++;
               }
             else
               break;
             }

        if( len1 != 0 )
          {
          iLen -= len1;
          reclen[0] = iLen & 0x00ff;
          reclen[1] = (iLen & 0xff00) >> 8;

          tag1 = *ptrobj++;

//  UI_ClearScreen();
//  TL_DispHexByte(0,0,tag1);
//  TL_DispHexByte(1,0,len1);
//  TL_DispHexByte(3,0,0x11);
//  UI_WaitKey();
          }
        }

      if( apk_CheckWordTag( tag1 ) == TRUE ) // double-byte tag?
        {
        tag2 = *ptrobj++;
        len1 = *ptrobj++;
        len2 = 0;
        len3 = 3;

        if( (len1 & 0x80) != 0 ) // chained length field?
          {
          switch( len1 & 0x7f )
                {
                case 0x01: // 1-byte length
                     len1 = *ptrobj++;
                     len3 += 1;
                     break;

                case 0x02: // 2-byte length
                     len2 = *ptrobj++;
                     len1 = *ptrobj++;
                     len3 += 2;
                     break;

                default:   // out of spec
                     return( (UCHAR *)0 );
                }
          }
        }
      else // single-byte tag
        {
        tag2 = tag1;
        tag1 = 0;
        len1 = *ptrobj++;
        len2 = 0;
        len3 = 2;

        if( (len1 & 0x80) != 0 ) // chained length field?
          {
          switch( len1 & 0x7f )
                {
                case 0x01: // 1-byte length
                     len1 = *ptrobj++;
                     len3 += 1;
                     break;

                case 0x02: // 2-byte length
                     len2 = *ptrobj++;
                     len1 = *ptrobj++;
                     len3 += 2;
                     break;

                default:   // out of spec
                     return( (UCHAR *)0 );
                }
          }
        }

      iPileLen = len1 + len2 + len3;
      if( iLen < iPileLen )
        return( (UCHAR *)-1 ); // invalid length

      // pick up current TLV
      *tlv++ = tag1;
      *tlv++ = tag2;
      *tlv++ = len1;
      *tlv++ = len2;
      for( i=0; i<(len1+len2); i++ )
         *tlv++ = *ptrobj++; // DATA[]

      // update the remaining length
      iLen -= iPileLen;
      reclen[0] = iLen & 0x00ff;
      reclen[1] = (iLen & 0xff00) >> 8;

//    TL_DispHexByte(0,0,tag1);
//    TL_DispHexByte(1,0,tag2);
//    TL_DispHexByte(2,0,len1);
//    TL_DispHexByte(3,0,len2);
//    UI_WaitKey();

      return( ptrobj ); // ok
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
UCHAR *apk_GetBERTLV2( UCHAR reclen[], UCHAR *ptrobj, UCHAR *tlv, UCHAR padlen  )
{
UCHAR i;
UCHAR len1, len2, len3;
UINT  iPileLen;
UCHAR tag1, tag2;
UINT  iLen;

      iLen = reclen[1]*256 + reclen[0];

      if( iLen < 2 )
        return( (UCHAR *)0 ); // end of record

      tag1 = *ptrobj++;

      // check ISO padding
      len1 = 0;
      if( (tag1 == 0x00) || (tag1 == 0xff) )
        {
        len1 = 1;

        while(1)
             {
             if( tag1 == *ptrobj )
               {
               len1++;
               ptrobj++;
               }
             else
               break;
             }

        if( len1 != 0 )
          {
          iLen -= len1;
          reclen[0] = iLen & 0x00ff;
          reclen[1] = (iLen & 0xff00) >> 8;

          tag1 = *ptrobj++;
          }
        }

      if( apk_CheckWordTag( tag1 ) == TRUE ) // double-byte tag?
        {
        tag2 = *ptrobj++;
        len1 = *ptrobj++;
        len2 = 0;
        len3 = 3;

        if( (len1 & 0x80) != 0 ) // chained length field?
          {
          switch( len1 & 0x7f )
                {
                case 0x01: // 1-byte length
                     len1 = *ptrobj++;
                     len3 += 1;
                     break;

                case 0x02: // 2-byte length
                     len2 = *ptrobj++;
                     len1 = *ptrobj++;
                     len3 += 2;
                     break;

                default:   // out of spec
                     return( (UCHAR *)0 );
                }
          }
        }
      else // single-byte tag
        {
        tag2 = tag1;
        tag1 = 0;
        len1 = *ptrobj++;
        len2 = 0;
        len3 = 2;

        if( (len1 & 0x80) != 0 ) // chained length field?
          {
          switch( len1 & 0x7f )
                {
                case 0x01: // 1-byte length
                     len1 = *ptrobj++;
                     len3 += 1;
                     break;

                case 0x02: // 2-byte length
                     len2 = *ptrobj++;
                     len1 = *ptrobj++;
                     len3 += 2;
                     break;

                default:   // out of spec
                     return( (UCHAR *)0 );
                }
          }
        }

      iPileLen = len1 + len2 + len3;

//if( g_test_flag == 1 )
//  {
//  TL_DispHexWord(0,0,iLen);
//  TL_DispHexWord(0,5,iPileLen);
//  UI_WaitKey();
//  }

      if( iLen < iPileLen )
        {
        return( (UCHAR *)0 ); // invalid length
        
//        if( (iLen + padlen) < iPileLen )
//          return( (UCHAR *)-1 ); // invalid length
//        else
//          return( (UCHAR *)0 );  // within the padding data objects, seems ok
        }

      // pick up current TLV
      *tlv++ = tag1;
      *tlv++ = tag2;
      *tlv++ = len1;
      *tlv++ = len2;
      for( i=0; i<(len1+len2); i++ )
         *tlv++ = *ptrobj++; // DATA[]

      // update the remaining length
      iLen -= iPileLen;
      reclen[0] = iLen & 0x00ff;
      reclen[1] = (iLen & 0xff00) >> 8;

//if( g_test_flag == 1 )
//  {
//  TL_DispHexByte(2,0,reclen[0]);
//  TL_DispHexByte(2,2,reclen[1]);
//  UI_WaitKey();
//  UI_ClearScreen();
//  }

      return( ptrobj ); // ok
}

// ---------------------------------------------------------------------------
// FUNCTION: scan icc data element table.
// INPUT   : tag1 - the MSB of Tag. (0 if only LSB is present)
//           tag2 - the LSB of Tag.
// OUTPUT  : de   - the data element entry found. (TAG1 TAG2 FORMAT LEN)
// RETURN  : -1     = tag not found.
//           others = the index of the found tag(s) in the table. (0x00..N)
// ---------------------------------------------------------------------------
UCHAR apk_ScanIDE( UCHAR tag1, UCHAR tag2, UCHAR *de )
{
UCHAR ide[4];
UCHAR i, j;

      for( i=0; i<MAX_IDE_CNT; i++ )
         {
         TL_IDE_GetData( i, ide ); // load 1 icc data element

         if( (tag1 == ide[0]) && (tag2 == ide[1]) )
           {
           for( j=0; j<4; j++ )
              *de++ = ide[j];
           return( i );
           }
         }
      return(-1);
}

// ---------------------------------------------------------------------------
// FUNCTION: Locate the specified tag(s) in the specified DOL.
// INPUT   : tag1 - the MSB of Tag. (0 if only LSB is present)
//           tag2 - the LSB of Tag.
//           dol  - data object list. (berLEN T-L T-L...)
// OUTPUT  : none.
// RETURN  : TRUE  - found.
//           FLASE - not found.
// ---------------------------------------------------------------------------
UCHAR apk_FindTagDOL( UCHAR tag1, UCHAR tag2, UCHAR *dol )
{
UINT  dol_len;
UCHAR cnt;
UCHAR t1, t2;

      dol_len = apk_GetBERLEN( dol, &cnt ); // total length of the DOL
      dol += cnt; // pointer to the 1'st T-L pair

      // scan each T-L pair through the end of DOL
      do {
         t1 = *dol++;    // MSG of tag

         if( apk_CheckWordTag( t1 ) == TRUE ) // double-byte tag?
           {
           t2 = *dol++;  // LSB of tag

           apk_GetBERLEN( dol, &cnt ); // retrieve length field
           dol += cnt; // advance offset address to next T-L pair

           dol_len -= (2 + cnt);
           }
         else // single-byte tag
           {
           t2 = t1;
           t1 = 0;

           apk_GetBERLEN( dol, &cnt ); // retrieve length field
           dol += cnt; // advance offset address to next T-L pair

           dol_len -= (1 + cnt);
           }

         if( (t1 == tag1) && (t2 == tag2) )
           return( TRUE ); // found

         } while( dol_len > 0 );

      return( FALSE ); // not found
}

// ---------------------------------------------------------------------------
// FUNCTION: parse all TLV's in one record.
// INPUT   : reclen - length of record. (2-byte integer)
//           ptrobj - pointer to the current data object. (T-L-V)
//           padlen - length of padding data objects. (0=no padding present)
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE - invalid data structure.
// ---------------------------------------------------------------------------
UCHAR apk_ParseTLV( UCHAR reclen[], UCHAR *ptrobj, UCHAR padlen )
{
UCHAR *ptrnext;
UCHAR *ptrdo;
UCHAR tlv[261];
UCHAR len[2];
UINT  iLen;

      len[0] = reclen[0];
      len[1] = reclen[1];
      ptrdo = ptrobj;

      while(1)
           {
           // read one data element
           ptrnext = apk_GetBERTLV2( &len[0], ptrdo, tlv, padlen );
//           if( ptrnext == (UCHAR *)0 )
//             return( FALSE );

           if( ptrnext != 0 )
             {
             ptrdo = ptrnext; // update next data object

             iLen = len[1]*256 + len[0]; // remaining length
             if( (iLen == 1) || (iLen == 2) )
             {
            	 if(*ptrnext==0x00||*ptrnext==0xff)
            	 {
            		 ptrnext++;
            		 if(*ptrnext==0x00||*ptrnext==0xff)
            			 return TRUE;
            	 }
               return( FALSE );
             }
             else if(iLen==0)//EMV 42a 2009-03-04 charles
            	 return TRUE;
             }
           else
             return( FALSE );
           }
}

// ---------------------------------------------------------------------------
// FUNCTION: check length and iso left padding. (0x00 or 0xFF)
// INPUT   : rec    - 2L1-V1-SW1-SW2, the template record read.
//                    V1 = T-L2-V..., T = template tag (1-byte)
//                    the legal template shall be one of the followings:
//                    (1) 0x00...0x00 V1 or
//                    (2) 0xFF...0xFF V1
// OUTPUT  : rec   - the trimed template record (without left paddings) or
//                   the original template record (because of no paddings).
// RETURN  : none.
// ---------------------------------------------------------------------------
void  apk_CheckIsoPadding_Left( UCHAR *rec )
{
UINT  iL1;
UCHAR cnt;
UCHAR padding;
UCHAR *ptrrec;

      iL1 = rec[1]*256 + rec[0];           // record length
      ptrrec = &rec[2];

// TL_DumpHexData(0,0,16, ptrrec);

      padding = rec[2];
      if( (padding != 0x00) && (padding != 0xff) )
        return; // no padding

      cnt = 0; // number of paddings to be trimed
      while(1)
           {
           if( *ptrrec == padding )
             {
             cnt++;
             ptrrec++;
             }
           else
             break;
           }

// UI_ClearScreen();
// TL_DispHexByte(0,0,padding);
// TL_DispHexByte(1,0,cnt);
// UI_WaitKey();

      // trim left paddings
      iL1 -= cnt;
      rec[0] = iL1 & 0x00ff;
      rec[1] = (iL1 & 0xff00) >> 8;
      memmove( &rec[2], ptrrec, iL1 );

// TL_DumpHexData(0,0, iL1+2, rec);
}

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
UCHAR apk_CheckIsoPadding_Right( UCHAR *rec, UCHAR *padlen )
{
UINT  i;
UCHAR c1;
UCHAR padding;
UCHAR *ptrpad;
UINT  iL1, iL2, iL3;

      iL1 = rec[1]*256 + rec[0];           // record length
      iL2 = apk_GetBERLEN( &rec[3], &c1 ); // template length

      *padlen = 0; // PATCH: 2005/04/11

      if( iL1 < (1 + c1 + 2) ) // T-L-SW1-SW2
        return( FALSE );

// UI_ClearScreen();
// TL_DispHexWord(0,0,iL1);
// TL_DispHexWord(1,0,iL2);
// TL_DispHexWord(2,0,iL1-2);
// TL_DispHexWord(2,5,iL2+1+c1);
// UI_WaitKey();

      if( (iL1 - 2) != (iL2 + 1 + c1) )
        {
        if( (iL1 - 2) > (iL2 + 1 + c1) )
          {
          iL3 = (iL1 - 2) - (iL2 + 1 + c1); // padding length
          *padlen = (UCHAR)iL3;

          ptrpad = rec + 3 + iL2 + c1;
          padding = *ptrpad;

// TL_DispHexWord(3,0,iL3);
// TL_DispHexWord(3,5,padding);
// UI_WaitKey();

          if( (padding != 0x00) && (padding != 0xff) )
            return( FALSE );

          for( i=0; i<iL3; i++ )
             {
             if( *(ptrpad + i) != padding )
               return( FALSE );
             }
          return( TRUE );
          }
        else
          return( FALSE );
        }
      else
        return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: parse the length of FCI. (constructed data structure)
// INPUT   : lenfic - total length of the record to be parsed. (including SW1 SW2)
//           ptrfci - pointer to the current FCI template (Tag = 6F).
//                    6F-L1-84-L2...A5-L3-...
//                    (1) lenfci - 2 = L1 + 1 + C1
//                    (2) L1 - (1 + L2 + C2) = L3 + 1 +C3
// OUTPUT  : padlen - length of padding data objects.
// RETURN  : TRUE
//           FALSE - invalid data structure.
// ---------------------------------------------------------------------------
UCHAR apk_ParseLenFCI( UCHAR *ptrfci, UCHAR *padlen )
{
UCHAR i;
UCHAR c1, c2, c3;
UINT  iL1, iL2, iL3, iL4;
UINT  lenfci;
UCHAR padding;
UCHAR *ptrobj;
UCHAR *ptrrec;

      ptrrec = ptrfci;
      lenfci = ptrfci[1]*256 + ptrfci[0];
      ptrfci += 2; // pointer to T-L-V

      iL1 = apk_GetBERLEN( ptrfci+1, &c1 );       // FCI Template

      ptrobj = apk_FindTag( 0x84, 0x00, ptrfci ); // DF Name
      iL2 = apk_GetBERLEN( ptrobj, &c2 );

      // check if "A5" is just following "84"
      iL4 = 0; // length of unexpected data objects btw A5 & 84
      if( *(ptrobj + iL2 + c2) != 0xA5 )
        {
        while( *(ptrobj + iL2 + c2) != 0xA5 )
             {
             iL4++;
             ptrobj++;

             if( iL4 >= iL1 )    // PATCH: PBOC2.0, 2006-02-16, 2CL0290101b
               return( FALSE );  //
             }
        }

      ptrobj = apk_FindTag( 0xA5, 0x00, ptrfci ); // FCI Prorietary Template
      iL3 = apk_GetBERLEN( ptrobj, &c3 );

      // (1) lenfci - 2 = L1 + 1 + C1 or
      // (2) ISO rigth padding (0x00 or 0xFF)

      if( apk_CheckIsoPadding_Right( ptrrec, padlen ) == FALSE )
        return( FALSE );

//    if( (lenfci - 2) != (iL1 + 1 + c1) )
//      {
//      if( (lenfci - 2) > (iL1 + 1 + c1) )
//        {
//        iL4 = (lenfci - 2) - (iL1 + 1 + c1);
//        padding = *(ptrfci + 2 + iL1);
//
// TL_DispHexWord(0,0,iL4);
// TL_DispHexByte(1,0,padding);
// UI_WaitKey();
//
//        if( (padding != 0x00) && (padding != 0xff) )
//          return( FALSE );
//
//        for( i=0; i<iL4; i++ )
//           {
//           if( *(ptrfci + 2 + iL1 + i) != padding )
//             return( FALSE );
//           }
//        return( TRUE );
//        }
//      else
//        return( FALSE );
//      }

      // L1 - (1 + L2 + C2) = L3 + 1 +C3

      if( (iL1 - (1 + iL2 + c2) - iL4) != (iL3 + 1 + c3) )
        {
       if( (lenfci - 2) != (iL1 + 1 + c1) ) // PATCH: PBOC2.0, 2006-02-16, 2CA.099, accept if total length is OK even A5 length is wrong.
         return( FALSE );
        }

      return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: parse the length of PSE DIR. (constructed data structure)
// INPUT   : ptrdir - pointer to the current DIR template (Tag = 70).
//                    70-L1-61-L2...61-L3...61-L4......61-Ln... or other valid TLV elements
//                    L0 = (1 + C1 + L1) + (1 + C2 + L2) +...+ (1+Cn+Ln) + ...others TLV
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE - invalid data structure.
//
// REF     : V4.1a -- 2CB.005.00-02 and 2CL.029.05-01
// ---------------------------------------------------------------------------
UCHAR apk_ParseLenPSEDIR( UCHAR *ptrdir )
{
UCHAR cnt;
UCHAR cnt2;
UCHAR *ptrrec;
UINT  iL0, iL1;


      ptrrec = ptrdir;

      iL0 = apk_GetBERLEN( ptrrec+1, &cnt ); // length of 71 template
      if(*(ptrrec+1+cnt)!=0x61)	//08-dec-23 charles
          	  return 0;
      ptrrec += (2 + cnt); // ptr to 1'st length of Tag 61
      cnt2 = 1; // length of the 1'st 61 tag
      while(1)
           {
           iL1 = apk_GetBERLEN( ptrrec, &cnt );   // length of 61

           if( iL0 >= (iL1 + cnt + cnt2) )
             iL0 -= (iL1 + cnt + cnt2);
           else
             return( FALSE );

           if( iL0 == 0 )
             return( TRUE );

           if( iL0 <= 2 )//if( iL0 < 2 ) EMV 42a 2009-03-03 charles 2CL.029.05 case1
             return( FALSE );

           ptrrec += (iL1 + cnt); // ptr to next Tag 61

           cnt2 = 1; // single tag length
           if( *ptrrec != 0x61 )
             {
             // PATCH: 2006-10-16, maybe other good TLV (known or unknown), shall be ignored
             if( apk_CheckWordTag( *ptrrec ) == TRUE )
               {
               ptrrec++;
               cnt2 = 2; // word tag length
               }
             }
           ptrrec++; // ptr to next length of Tag 61
           }
}
/*
enum APDU_TEMPLETE
{
	APP = 0x61,==>	0x4f,
	FCI = 0x6f,
	a =0x70,
	SCRIPT1 = 0x71,
	SCRIPT2 = 0x72,
	DD = 0x73,
	RMF1 = 0x77,
	RMF2 = 0x80,
	FCIP = 0xa5,
	FCIIDD = 0xBF0C
};*/
//2008-dec-26 charles
#define TEMPLATE61SIZE 6
UINT TEMPLATE_61[TEMPLATE61SIZE] = {0X4F,0X50,0X73,0X87,0X9D,0X9F12};
#define TEMPLATE6FSIZE 2
UINT TEMPLATE_6F[TEMPLATE6FSIZE] = {0x84,0xA5};
#define TEMPLATE7077SIZE 41
UINT TEMPLATE_70OR77[TEMPLATE7077SIZE] = {0X57,0X5A,0X61,0X8C,0X8D,0X8E,0X8F,0X90,0X92,0X93
							,0X97,0X5F20,0X5F24,0X5F25,0X5F28,0X5F30,0X5F34,0X9F05,0X9F07,0X9F08
							,0X9F0B,0X9F0D,0X9F0E,0X9F0F,0X9F14,0X9F1F,0X9F20,0X9F23,0X9F2D,0X9F2E
							,0X9F2F,0X9F32,0X9F3B,0X9F42,0X9F43,0X9F44,0X9F46,0X9F47,0X9F48,0X9F49
							,0X9F4A};
#define TEMPLATE7172SIZE 2
UINT TEMPLATE_71OR72[TEMPLATE7172SIZE] = {0X86,0X9F18};
#define TEMPLATE73BF0CSIZE 7
UINT TEMPLATE_73ORBF0C[TEMPLATE73BF0CSIZE] ={0X42,0X5F50,0X5F53,0X5F54,0X5F55,0X5F56,0X9F4D};
#define TEMPLATE7780SIZE 6
UINT TEMPLATE_77OR80[TEMPLATE7780SIZE] = {0X82,0X94,0X9F10,0X9F26,0X9F27,0X9F36};
#define TEMPLATEA5SIZE 8
UINT TEMPLATE_A5[TEMPLATEA5SIZE] = {0X50,0X87,0X88,0X5F2D,0X9F11,0X9F12,0X9F38,0XBF0C};

UINT Uneed[69] = {0x42,0x4f,0x50,0x57,0x5a,0x61,0x73,0x82,0x84,0x86
				 ,0x87,0x88,0x8c,0x8d,0x8e,0x8f,0x90,0x92,0x93,0x94
				 ,0x97,0x9d,0xa5
				 ,0x5f20,0x5f24,0x5f25,0x5f28,0x5f2d,0x5f30,0x5f34,0x5f50,0x5f53,0x5f54
				 ,0x5f55,0x5f56,0x9f05,0x9f07,0x9f08,0x9f0b,0x9f0d,0x9f0e,0x9f0f,0x9f10
				 ,0x9f11,0x9f12,0x9f14,0x9f18,0x9f1f,0x9f20,0x9f23,0x9f26,0x9f27,0x9f2d
				 ,0x9f2e,0x9f2f,0x9f32,0x9f36,0x9f38,0x9f3b,0x9f42,0x9f43,0x9f44,0x9f46
				 ,0x9f47,0x9f48,0x9f49,0x9f4a,0x9f4d,0xbf0c
				 };
#include <stdlib.h>
// ---------------------------------------------------------------------------
// remove inline Wayne
static int apk_BSearchComp(const void *Template,const void *Tag)
{
	UINT temp,tag;
	temp = (*(UINT*)Template);//|(*((UINT*)(Template+1)))<<8;
	tag =  (*(UINT*)Tag);//|(*((UINT*)(Tag+1)))<<8;
	if(temp>tag)
		return 1;
	else if(temp<tag)
		return -1;
	else
			return 0;
}

// ---------------------------------------------------------------------------
// remove inline Wayne
UCHAR apk_needTemplate(UINT Key)
{
	if(bsearch(&Key,Uneed,69,2,apk_BSearchComp)==0)
		return FALSE;
	return TRUE;//�䤣��
}

// ---------------------------------------------------------------------------
//remove inline Wayne
UCHAR apk_BSearch(UINT Key,UINT Template)
{
	//bf0c��tag���ˬd
	if(bsearch(&Key,TEMPLATE_73ORBF0C,TEMPLATE73BF0CSIZE,2,apk_BSearchComp)!=0)
		return TRUE;
	switch(Template)
	{
		case 0x61:
			if(bsearch(&Key,TEMPLATE_61,TEMPLATE61SIZE,2,apk_BSearchComp)!=0)
				return TRUE;
			break;
		case 0x6F:
			if(bsearch(&Key,TEMPLATE_6F,TEMPLATE6FSIZE,2,apk_BSearchComp)!=0)
				return TRUE;
			break;
		case 0x70:
			if(bsearch(&Key,TEMPLATE_70OR77,TEMPLATE7077SIZE,2,apk_BSearchComp)!=0)
				return TRUE;
			break;
		case 0x71:
		case 0X72:
			if(bsearch(&Key,TEMPLATE_71OR72,TEMPLATE7172SIZE,2,apk_BSearchComp)!=0)
				return TRUE;
			break;
		case 0x73:
		case 0XBF0C:
			if(bsearch(&Key,TEMPLATE_73ORBF0C,TEMPLATE73BF0CSIZE,2,apk_BSearchComp)!=0)
				return TRUE;
			break;
		case 0x77:
			if(bsearch(&Key,TEMPLATE_70OR77,TEMPLATE7077SIZE,2,apk_BSearchComp)!=0)
				return TRUE;
			break;
			if(bsearch(&Key,TEMPLATE_77OR80,TEMPLATE7780SIZE,2,apk_BSearchComp)!=0)
				return TRUE;
			break;
		case 0x80:
			if(bsearch(&Key,TEMPLATE_77OR80,TEMPLATE7780SIZE,2,apk_BSearchComp)!=0)
				return TRUE;
			break;
		case 0xA5:
			if(bsearch(&Key,TEMPLATE_A5,TEMPLATEA5SIZE,2,apk_BSearchComp)!=0)
				return TRUE;
			break;
//		case 0xBF0C:
//			return TRUE;
		default:
			return FALSE;


	}
	return FALSE;
}

// ---------------------------------------------------------------------------
//remove inline Wayne
static UCHAR apk_isTemplate(UCHAR tag)
{
	//�ˬdtag��bit6�O�_��1�A�Y��1�h��constructed data object�A�Y�� template
	//���Ftag 80�O�ҥ~
	if(((tag&0x20)!=0)||(tag==0x80))
			return (TRUE);
	else
			return (FALSE);

}

// ---------------------------------------------------------------------------
UCHAR apk_checkTemplate(UCHAR *apdu,UINT Template,UINT TempLen)
{
	UCHAR TAG[2];
	UINT LEN;
	UCHAR cnt;
	UINT index = 0;
	UCHAR response = TRUE;
	while((index<TempLen)&&(response==TRUE))
	{
		TAG[0] = *(apdu+index);
		if((TAG[0]==0)||(TAG[0]==0xff))
		{
			index ++;
			continue;
		}
		if((TAG[0]&0x1f)==0x1f)
		{
			TAG[1] = *(apdu+1+index);
			LEN = apk_GetBERLEN( apdu+2+index, &cnt );
			if(apk_needTemplate((TAG[0]<<8|TAG[1]))==TRUE)
				response = apk_BSearch((TAG[0]<<8|TAG[1]),Template);
			if((TAG[0]==0xbf)&&(response==TRUE))
			{
				response = apk_checkTemplate((apdu+2+cnt+index),0xbf0c,LEN);
			}
			index+=LEN+cnt+2;
		}
		else
		{
	//		TAG[1] = 0;
			LEN = apk_GetBERLEN( apdu+1+index, &cnt );
			if(apk_needTemplate((TAG[0]))==TRUE)
				response = apk_BSearch(TAG[0],Template);
			if((response==TRUE)&&(apk_isTemplate(TAG[0])==TRUE))
			{
				response = apk_checkTemplate((apdu+1+cnt+index),TAG[0],LEN);
			}
			index+=LEN+cnt+1;

		}
	}
	return response;

}

// ---------------------------------------------------------------------------
//before call this function all TLV's length is correction.
//remove inline Wayne
UCHAR apk_ParseTemplate( UCHAR *apdu )
{
	UCHAR TAG[2];
	UCHAR LEN;
	UCHAR cnt;
	UCHAR *index = apdu;
	UINT apdu_len = (index[1]<<8)|(index[0]);
	int i = 2;
	UCHAR response = TRUE;


	apdu+=2;
	TAG[0] = index[2];	
	
	while((TAG[0]==0)||(TAG[0]==0xff))
	{
		if(i<apdu_len+2)
		{
			i++;
			TAG[0] = index[i];
		}
		else
			return FALSE;
	}
	if((TAG[0]&0x1f)==0x1f)
	{
		TAG[1] = *(apdu+1);
		LEN = apk_GetBERLEN( apdu+2, &cnt );
		if(TAG[0]==0xbf)
		{
			response = apk_checkTemplate((apdu+2+cnt),0xbf0c,LEN);
		}
	}
	else
	{
//		TAG[1] = 0;
		LEN = apk_GetBERLEN( apdu+1, &cnt );
		if(apk_isTemplate(TAG[0])==TRUE)
		{
			response = apk_checkTemplate((apdu+1+cnt),TAG[0],LEN);
		}

	}
	return response;
}
