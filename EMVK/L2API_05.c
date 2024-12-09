/*
============================================================================
****************************************************************************
**                                                                        **
**  PROJECT  : EMV L2                                                     **
**  PRODUCT  : AS320-A                                                    **
**                                                                        **
**  FILE     : L2API_04.C                                                 **
**  MODULE   : api_emv_FindPDOL() -- RFU                                  **
               Build_DOL()                                                **
**             api_emv_CheckPDOL()                                        **
**             api_emv_ConcatenateDOL()                                   **
**             api_emv_InitApplication()                                  **
**  VERSION  : V1.00                                                      **
**  DATE     : 2002/12/19                                                 **
**  EDITOR   : James Hsieh                                                **
**                                                                        **
**  Copyright(C) 2002-2007 SymLink Corporation. All rights reserved.      **
**                                                                        **
****************************************************************************
============================================================================
HISTORY:
	2008-dec-22		Charles tsai
					�[�J�ˬdformat 2��R-APDU�����׬�0��TAG
----------------------------------------------------------------------------
*/
#include <string.h>

#include "POSAPI.h"
//#include <EMVDC.h>
#include "GDATAEX.h"
#include "EMVAPI.h"
#include "EMVAPK.h"
#include "EMVCONST.H"
#include <TOOLS.h>

UCHAR Build_DOL( UCHAR source, UCHAR *ptrobj, UCHAR *length, UCHAR *ptrdol );
UCHAR Build_DOL2( UCHAR source, UCHAR *ptrobj, UCHAR *length, UCHAR *ptrdol );

// ---------------------------------------------------------------------------
// FUNCTION: Find PDOL in the given FCI.
// INPUT   : fci - T-L-V, the FCI. (RFU)
// OUTPUT  : none.
// REF     : g_ibuf
// RETURN  : NULL  - not found.
//           other - pointer to the length field of PDOL (L-PDOL)
// ---------------------------------------------------------------------------
//UCHAR *api_emv_FindPDOL( void )
//{
//UCHAR *fci;
//
//      apk_ReadRamDataICC( ADDR_SELECTED_FCI, SELECTED_FCI_LEN, g_ibuf );
//
//      return( apk_FindTag( 0x9f, 0x38, g_ibuf ) ); // Tag=9F38 (PDOL)?
//}

// ---------------------------------------------------------------------------
// FUNCTION: check the specifed tag (sigle- or double-byte tag) in PDOL of FCI.
// INPUT   : tag1 - the MSB of Tag. (0 if only LSB is present)
//           tag2 - the LSB of Tag.
// OUTPUT  : none.
// REF     : g_ibuf
// RETURN  : emvOK     - found.
//           emvFailed - not found.
// ---------------------------------------------------------------------------
UCHAR api_emv_CheckPDOL( UCHAR tag1, UCHAR tag2 )
{
UCHAR *ptrobj;
UCHAR pdol_len;
UCHAR t1, t2;
UCHAR cnt;

      apk_ReadRamDataICC( ADDR_SELECTED_FCI, SELECTED_FCI_LEN, g_ibuf );
      ptrobj = apk_FindTag( 0x9f, 0x38, g_ibuf );

      if( ptrobj == 0) // Tag=9F38 (PDOL)?
        return( emvFailed ); // no PDOL in FCI
      else
        {
        // format: LEN, T1-L1, T2-L2, T3-L3....
        //  where: LEN = total length, T1 = 1 or 2 bytes

        if( apk_FindTagDOL( tag1, tag2, ptrobj ) == TRUE )
          return( emvOK );
        else
          return( emvFailed );

//      pdol_len = *ptrobj++; // total length of the T-L pairs
//
//      // scan each T-L pair through the end of DOL
//      do {
//         t1 = *ptrobj++;    // MSG of tag
//
//         if( apk_CheckWordTag( t1 ) == TRUE ) // double-byte tag?
//           {
//           t2 = *ptrobj++;  // LSB of tag
//
//           apk_GetBERLEN( ptrobj, &cnt ); // retrieve length field
//           ptrobj += cnt; // advance offset address to next T-L pair
//
//           pdol_len -= (2 + cnt);
//           }
//         else // single-byte tag
//           {
//           t2 = t1;
//           t1 = 0;
//
//           apk_GetBERLEN( ptrobj, &cnt ); // retrieve length field
//           ptrobj += cnt; // advance offset address to next T-L pair
//
//           pdol_len -= (1 + cnt);
//           }
//
//         if( (t1 == tag1) && (t2 == tag2) )
//           return( emvOK ); // found
//
//         } while( pdol_len > 0 );
//
//      return( emvFailed ); // not found
        }
}

// ---------------------------------------------------------------------------
// FUNCTION: scan terminal data element table.
// INPUT   : tag1 - the MSB of Tag. (0 if only LSB is present)
//           tag2 - the LSB of Tag.
// OUTPUT  : de   - the data element entry found. (TAG1 TAG2 FORMAT LEN)
// RETURN  : -1     = tag not found.
//           others = the index of the found tag(s) in the table. (0x00..N)
// ---------------------------------------------------------------------------
UCHAR Scan_TDE( UCHAR tag1, UCHAR tag2, UCHAR *de )
{
UCHAR tde[4];
UCHAR i, j;

      for( i=0; i<MAX_TDE_CNT; i++ )
         {
         TL_TDE_GetData( i, tde ); // load 1 term data element

         if( (tag1 == tde[0]) && (tag2 == tde[1]) )
           {
           for( j=0; j<4; j++ )
              *de++ = tde[j];
           return( i );
           }
         }
      return(-1);
}

// ---------------------------------------------------------------------------
// FUNCTION: scan icc data element table.
// INPUT   : tag1 - the MSB of Tag. (0 if only LSB is present)
//           tag2 - the LSB of Tag.
// OUTPUT  : de   - the data element entry found. (TAG1 TAG2 FORMAT LEN)
// RETURN  : -1     = tag not found.
//           others = the index of the found tag(s) in the table. (0x00..N)
// ---------------------------------------------------------------------------
UCHAR Scan_IDE( UCHAR tag1, UCHAR tag2, UCHAR *de )
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
// FUNCTION: check & build special DOL which is built immediately prior to
//           issing the GENERATE AC command.
//           (1) 9A  : Transaction Date(n6)
//           (2) 9F21: Transaction Time(n6)
//           (3) 98  : TC Hash Value(b20)
//           (4) 9F37: Unpredictable Number(b4)
// INPUT   : tag1   - the MSB of Tag. (0 if only LSB is present)
//           tag2   - the LSB of Tag.
// OUTPUT  : none.
// REF     : g_ibuf
//           g_obuf
//           g_cdol_updn
// RETURN  : none.
// ---------------------------------------------------------------------------
void Build_Special_DOL( UCHAR tag1, UCHAR tag2 )
{
UINT  iTag;
UCHAR i;
UCHAR buf1[22];
UCHAR buf2[22];
UCHAR length[1];

      iTag = tag1*256 + tag2;
      switch( iTag )
            {
            case 0x009A: // transaction date

                 apk_ReadRamDataTERM( ADDR_TERM_TX_DATE, 3, buf1 );
                 for( i=0; i<3; i++ )
                    {
                    if( buf1[i] != 0xff ) // ever been set
                      return;
                    }

                 // setup current value
                 TL_GetDateTime( &buf1[1] );
                 buf1[0]=6;
                 TL_asc2bcd( 3, buf2, buf1 ); // convert to BCD, buf2: YYMMDD

                 apk_WriteRamDataTERM( ADDR_TERM_TX_DATE, 3, buf2 );

                 break;

            case 0x9F21: // transaction time

                 apk_ReadRamDataTERM( ADDR_TERM_TX_TIME, 3, buf1 );
                 for( i=0; i<3; i++ )
                    {
                    if( buf1[i] != 0xff ) // ever been set
                      return;
                    }

                 // setup current value
                 TL_GetDateTime( &buf1[1] );
                 buf1[5]=6;
                 TL_asc2bcd( 3, buf2, &buf1[5] ); // convert to BCD, buf2: HHMMSS

                 apk_WriteRamDataTERM( ADDR_TERM_TX_TIME, 3, buf2 );

                 break;

            case 0x0098: // TC hash value = HASH(TDOL)

                 apk_ReadRamDataICC( ADDR_ICC_TDOL, 254, g_ibuf );

//  g_ibuf[0] = 10;
//  g_ibuf[1] = 0;
//  g_ibuf[2] = 0x5a;
//  g_ibuf[3] = 0x03;
//  g_ibuf[4] = 0x95;
//  g_ibuf[5] = 0x01;
//  g_ibuf[6] = 0x5f;
//  g_ibuf[7] = 0x2d;
//  g_ibuf[8] = 0x06;
//  g_ibuf[9] = 0x5f;
//  g_ibuf[10] = 0x20;
//  g_ibuf[11] = 0x01;

                 if( (g_ibuf[1]*256+g_ibuf[0]) != 0 )
                   {
                   // (1) using ICC TDOL (from card issuer )
                   g_ibuf[1] = g_ibuf[0]; // convert to berLEN format
                   g_ibuf[0] = 0x81;      //
                   Build_DOL2( 0, g_ibuf, length, g_obuf );
                   apk_HASH( SHA1, (UINT)length[0], g_obuf, buf1 ); // generate hash value
                   }
                 else
                   {
                   // (2) using TERM default TDOL (from payment system)
                   //     - if a default TDOL is required but is not present
                   //       in the terminal (because the payment system does not
                   //       support), a default TDOL with no data objects in the
                   //       list shall be assumed.
                   //     - set TVR.default_TDOL_used = 1

                   apk_ReadRamDataTERM( ADDR_TERM_TDOL, 254, g_ibuf );
                   if( (g_ibuf[1]*256+g_ibuf[0]) != 0 )
                     {
                     g_ibuf[1] = g_ibuf[0]; // convert to berLEN format
                     g_ibuf[0] = 0x81;      //
                     Build_DOL2( 0, g_ibuf, length, g_obuf );
                     apk_HASH( SHA1, (UINT)length[0], g_obuf, buf1 ); // generate hash value

                     g_term_TVR[4] |= TVR4_DEFAULT_TDOL_USED; // set TVR bit
                     api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+4, 1, &g_term_TVR[4] );
                     }
                   else // neither ICC TDOL nor default TDOL is present
                     {
                //   apk_HASH( SHA1, 0, g_obuf, buf1 ); // generate hash value
                //   NOTE: according to HASH algorithm, put empty message to
                //         calculate the HASH value, ie, length = 0,
                //         the default result is as follows: (IFM cannot support this case)

                     buf1[0]  = 0xDA;
                     buf1[1]  = 0x39;
                     buf1[2]  = 0xA3;
                     buf1[3]  = 0xEE;
                     buf1[4]  = 0x5E;
                     buf1[5]  = 0x6B;
                     buf1[6]  = 0x4B;
                     buf1[7]  = 0x0D;
                     buf1[8]  = 0x32;
                     buf1[9]  = 0x55;
                     buf1[10] = 0xBF;
                     buf1[11] = 0xEF;
                     buf1[12] = 0x95;
                     buf1[13] = 0x60;
                     buf1[14] = 0x18;
                     buf1[15] = 0x90;
                     buf1[16] = 0xAF;
                     buf1[17] = 0xD8;
                     buf1[18] = 0x07;
                     buf1[19] = 0x09;
                     }
                   }

                 // setup current value
                 apk_WriteRamDataTERM( ADDR_TERM_TC_SHA1, 20, buf1 );

                 break;

            case 0x9F37: // unpredictable number

            //   g_cdol_updn = 1; // CDA: set flag on

            //   apk_ReadRamDataTERM( ADDR_TERM_UPD_NBR, 4, buf1 );
            //   for( i=0; i<4; i++ )
            //      {
            //      if( buf1[i] != 0xff ) // ever been set
            //        return;
            //      }

                 apk_ReadRamDataTERM( ADDR_TERM_UPD_NBR_LEN, 1, buf1 );
                 if( buf1[0] != 0 )
                   return;

                 // setup current value
                 apk_GetChallengeTERM( buf1 );
                 apk_WriteRamDataTERM( ADDR_TERM_UPD_NBR, 4, buf1 );
                 buf1[0] = 4;
                 apk_WriteRamDataTERM( ADDR_TERM_UPD_NBR_LEN, 1, buf1 );

                 break;
            }
}

// ---------------------------------------------------------------------------
// FUNCTION: build DOL.
// INPUT   : ptrobj - pointer to the requested DOL.
//                    berLEN T1-L1 T2-L2 T3-L3...
//           source - 0: no limitation.
//                    1: only TERMINAL as source.
//                    2: only ICC as source.
// OUTPUT  : length - length of the data elements concatenation of DOL.
//           ptrdol - concatenation of all related data elements of the DOL.
// REF     : g_ibuf
// RETURN  : TRUE  - OK
//           FALSE - something wrong in TLV format (reference only)
// ---------------------------------------------------------------------------
UCHAR Build_DOL( UCHAR source, UCHAR *ptrobj, UCHAR *length, UCHAR *ptrdol )
{
UCHAR dol_len;
UCHAR de[4];
UCHAR i;
UCHAR tag1, tag2;
UCHAR len;
UCHAR index;
UCHAR buf[2];
UCHAR cnt;
UCHAR flag;


      flag = TRUE; // PATCH: 2005/08/09, for bad CDOL2 format in COSMOS VSDC card

      *length = 0; // reset output length

      // format: LEN[n], T1-L1, T2-L2, T3-L3....
      //         where: LEN = total length, T1 = 1 or 2 bytes, Ln = 1 byte.

      dol_len = apk_GetBERLEN( ptrobj, &cnt ); // get total length
      if( dol_len == 0 ) // empty DOL?
        return( flag );

      ptrobj += cnt;

      do{
        tag1 = *ptrobj++;    // MSB of tag
        tag2 = *ptrobj++;    // LSB of tag

        // check constructed data element
        if( apk_CheckConsTag( tag1 ) == TRUE )
          {
          len = (UCHAR)apk_GetBERLEN( --ptrobj, &cnt );
          ptrobj += cnt;
          if( len == 0 )
            flag = FALSE;

          dol_len -= (1+cnt);
          goto BDOL_UNKNOWN;
          }

        if( apk_CheckWordTag( tag1 ) == TRUE ) // double-byte tag?
          {
          len = (UCHAR)apk_GetBERLEN( ptrobj, &cnt );
          ptrobj += cnt;
          if( len == 0 )
            flag = FALSE;

          dol_len -= (2+cnt);
          }
        else // single-byte tag
          {
          len = (UCHAR)apk_GetBERLEN( --ptrobj, &cnt );
          ptrobj += cnt;
          if( len == 0 )
            flag = FALSE;

          tag2 = tag1;
          tag1 = 0;

          dol_len -= (1+cnt);

          if( apk_CheckTermTag( tag2 ) == FALSE )
            {
            if( apk_CheckIccTag( tag2 ) == FALSE )
              {
              if( apk_CheckIssuerTag( tag2 ) == FALSE )
                // a single-byte tag, but its source is neither TERMINAL, ICC or ISSUER
                goto BDOL_UNKNOWN;
              }
            }
          }

        // ------ tag(s) source is "TERMINAL" ? ------
        index = Scan_TDE( tag1, tag2, &de[0] );

        if( index != 255 )
          {
          // Check specical tags for:
          // (1) 9A  : Transaction Date(n6)
          // (2) 9F21: Transaction Time(n6)
          // (3) 98  : TC Hash Value(b20)
          // (4) 9F37: Unpredictable Number(b4)
          Build_Special_DOL( tag1, tag2 );

          // Yes, retrieve the specified data element
          apk_ReadRamDataTERM( TL_TDE_GetAddr(index), de[3], g_ibuf );

          // check length
          if( len == de[3] )
            {
            // same length
            memmove( ptrdol, g_ibuf, len );
            ptrdol += len;
            }
          else
            {
            // different length
            if( len < de[3] )
              {
              // 1. the length specified in DOL is less than the actual
              // 1.1 format='n': truncate leftmost bytes
              // 1.2 others    : truncate rightmost bytes
              if( de[2] == DEF_N )
                {
                // case 1.1
                i = de[3] - len;
                memmove( ptrdol, &g_ibuf[i], len );
                ptrdol += len;
                }
              else
                {
                // case 1.2
                memmove( ptrdol, g_ibuf, len );
                ptrdol += len;
                }
              }
            else
              {
              // 2. the length specified in DOL is greater than the actual
              // 2.1 format='n' : paded with leading hex  0x00
              // 2.2 format='cn': paded with leading hex  0xff (changed to trailing hex 0xff)
              // 2.3 others     : paded with trailing hex 0x00
              switch( de[2] )
                    {
                    case DEF_N:  // case 2.1

                         i = len - de[3];
                         for( i=0; i<(len-de[3]); i++ )
                            *ptrdol++ = 0x00;

                         memmove( ptrdol, g_ibuf, de[3] );
                         ptrdol += de[3];

                         break;

                    case DEF_CN: // case 2.2

                //       i = len - de[3];
                //       for( i=0; i<(len-de[3]); i++ )
                //          *ptrdol++ = 0xff;
                //
                //       memmove( ptrdol, g_ibuf, de[3] );
                //       ptrdol += de[3];

                         i = len - de[3];
                         memmove( ptrdol, g_ibuf, de[3] );
                         ptrdol += de[3];

                         for( i=0; i<(len-de[3]); i++ )
                            *ptrdol++ = 0xff;

                         break;

                    default:     // case 2.3

                         i = len - de[3];
                         memmove( ptrdol, g_ibuf, de[3] );
                         ptrdol += de[3];

                         for( i=0; i<(len-de[3]); i++ )
                            *ptrdol++ = 0x00;
                    }
              }
            }

          goto BDOL_NEXT;
          }

        if( source == 1 ) // terminal as source only?
          goto BDOL_UNKNOWN;

        // ------ tag(s) source is "ICC" ? ------
        index = Scan_IDE( tag1, tag2, &de[0] );

        if( index != 255 )
          {
          // Yes, retrieve the specified data element
          apk_ReadRamDataICC( TL_IDE_GetAddr(index), 1, &de[3] ); // get length
          if( de[3] == 0 )
            memset( g_ibuf, 0x00, 255 );
          else
            apk_ReadRamDataICC( TL_IDE_GetAddr(index)+2, de[3], g_ibuf ); // get data

          // check length
          if( len == de[3] )
            {
            // same length
            memmove( ptrdol, g_ibuf, len );
            ptrdol += len;
            }
          else
            {
            // different length
            if( len < de[3] )
              {
              // 1. the length specified in DOL is less than the actual
              // 1.1 format='n': truncate leftmost bytes
              // 1.2 others    : truncate rightmost bytes
              if( de[2] == DEF_N )
                {
                // case 1.1
                i = de[3] - len;
                memmove( ptrdol, &g_ibuf[i], len );
                ptrdol += len;
                }
              else
                {
                // case 1.2
                memmove( ptrdol, g_ibuf, len );
                ptrdol += len;
                }
              }
            else
              {
              // 2. the length specified in DOL is greater than the actual
              // 2.1 format='n' : paded with leading hex  0x00
              // 2.2 format='cn': paded with leading hex  0xff (changed to trailing hex 0xff)
              // 2.3 others     : paded with trailing hex 0x00
              switch( de[2] )
                    {
                    case DEF_N:  // case 2.1

                         i = len - de[3];
                         for( i=0; i<(len-de[3]); i++ )
                            *ptrdol++ = 0x00;

                         memmove( ptrdol, g_ibuf, de[3] );
                         ptrdol += de[3];

                         break;

                    case DEF_CN: // case 2.2

                //       i = len - de[3];
                //       for( i=0; i<(len-de[3]); i++ )
                //          *ptrdol++ = 0xff;
                //
                //       memmove( ptrdol, g_ibuf, de[3] );
                //       ptrdol += de[3];

                         i = len - de[3];
                         memmove( ptrdol, g_ibuf, de[3] );
                         ptrdol += de[3];

                         for( i=0; i<(len-de[3]); i++ )
                            *ptrdol++ = 0xff;

                         break;

                    default:     // case 2.3

                         i = len - de[3];
                         memmove( ptrdol, g_ibuf, de[3] );
                         ptrdol += de[3];

                         for( i=0; i<(len-de[3]); i++ )
                            *ptrdol++ = 0x00;
                    }
              }
            }
          }
        else // tag(s) source is not found
          {

BDOL_UNKNOWN:
          // fill with hex 0
          for( i=0; i<len; i++ )
             *ptrdol++ = 0x00;
          }

BDOL_NEXT:
        *length += len; // accumulate data element length
        } while( dol_len > 0 );

      return( flag );
}

// ---------------------------------------------------------------------------
// FUNCTION: build DOL. (used only for build TC HASH)
// INPUT   : ptrobj - pointer to the requested DOL.
//                    berLEN T1-L1 T2-L2 T3-L3...
//           source - 0: no limitation.
//                    1: only TERMINAL as source.
//                    2: only ICC as source.
// OUTPUT  : length - length of the data elements concatenation of DOL.
//           ptrdol - concatenation of all related data elements of the DOL.
// RETURN  : TRUE  - OK
//           FALSE - something wrong in TLV format (reference only)
// ---------------------------------------------------------------------------
UCHAR Build_DOL2( UCHAR source, UCHAR *ptrobj, UCHAR *length, UCHAR *ptrdol )
{
UCHAR dol_len;
UCHAR de[4];
UCHAR i;
UCHAR tag1, tag2;
UCHAR len;
UCHAR index;
UCHAR buf[2];
UCHAR cnt;
UCHAR flag;
UCHAR g_ibuf[300]; // local buffer

      flag = TRUE; // PATCH: 2005/08/09, for bad CDOL2 format in COSMOS VSDC card

      *length = 0; // reset output length

      // format: LEN[n], T1-L1, T2-L2, T3-L3....
      //         where: LEN = total length, T1 = 1 or 2 bytes, Ln = 1 byte.

      dol_len = apk_GetBERLEN( ptrobj, &cnt ); // get total length
      if( dol_len == 0 ) // empty DOL?
        return( flag );

      ptrobj += cnt;

      do{
        tag1 = *ptrobj++;    // MSB of tag
        tag2 = *ptrobj++;    // LSB of tag

        // check constructed data element
        if( apk_CheckConsTag( tag1 ) == TRUE )
          {
          len = (UCHAR)apk_GetBERLEN( --ptrobj, &cnt );
          ptrobj += cnt;
          if( len == 0 )
            flag = FALSE;

          dol_len -= (1+cnt);
          goto BDOL_UNKNOWN;
          }

        if( apk_CheckWordTag( tag1 ) == TRUE ) // double-byte tag?
          {
          len = (UCHAR)apk_GetBERLEN( ptrobj, &cnt );
          ptrobj += cnt;
          if( len == 0 )
            flag = FALSE;

          dol_len -= (2+cnt);
          }
        else // single-byte tag
          {
          len = (UCHAR)apk_GetBERLEN( --ptrobj, &cnt );
          ptrobj += cnt;
          if( len == 0 )
            flag = FALSE;

          tag2 = tag1;
          tag1 = 0;

          dol_len -= (1+cnt);

//  TL_DispHexByte(0,0,tag1);
//  TL_DispHexByte(1,0,tag2);
//  TL_DispHexByte(2,0,dol_len);
//  UI_WaitKey();

          if( apk_CheckTermTag( tag2 ) == FALSE )
            {
            if( apk_CheckIccTag( tag2 ) == FALSE )
              {
              if( apk_CheckIssuerTag( tag2 ) == FALSE )
                // a single-byte tag, but its source is neither TERMINAL, ICC or ISSUER
                goto BDOL_UNKNOWN;
              }
            }
          }

        // ------ tag(s) source is "TERMINAL" ? ------
        index = Scan_TDE( tag1, tag2, &de[0] );

        if( index != 255 )
          {

//  TL_DumpHexData(0,0,4,de);
          // Check specical tags for:
          // (1) 9A  : Transaction Date(n6)
          // (2) 9F21: Transaction Time(n6)
          // (3) 98  : TC Hash Value(b20)
          // (4) 9F37: Unpredictable Number(b4)
//        Build_Special_DOL( tag1, tag2 );

          // Yes, retrieve the specified data element
          apk_ReadRamDataTERM( TL_TDE_GetAddr(index), de[3], g_ibuf );

//  TL_DispHexByte(1,0,len);
//  TL_DumpHexData(0,2,de[3], g_ibuf);

          // check length
          if( len == de[3] )
            {
            // same length
            memmove( ptrdol, g_ibuf, len );
            ptrdol += len;
            }
          else
            {
            // different length
            if( len < de[3] )
              {
              // 1. the length specified in DOL is less than the actual
              // 1.1 format='n': truncate leftmost bytes
              // 1.2 others    : truncate rightmost bytes
              if( de[2] == DEF_N )
                {
                // case 1.1
                i = de[3] - len;
                memmove( ptrdol, &g_ibuf[i], len );
                ptrdol += len;
                }
              else
                {
                // case 1.2
                memmove( ptrdol, g_ibuf, len );
                ptrdol += len;
                }
              }
            else
              {
              // 2. the length specified in DOL is greater than the actual
              // 2.1 format='n' : paded with leading hex  0x00
              // 2.2 format='cn': paded with leading hex  0xff (changed to trailing hex 0xff)
              // 2.3 others     : paded with trailing hex 0x00
              switch( de[2] )
                    {
                    case DEF_N:  // case 2.1

                         i = len - de[3];
                         for( i=0; i<(len-de[3]); i++ )
                            *ptrdol++ = 0x00;

                         memmove( ptrdol, g_ibuf, de[3] );
                         ptrdol += de[3];

                         break;

                    case DEF_CN: // case 2.2

                //       i = len - de[3];
                //       for( i=0; i<(len-de[3]); i++ )
                //          *ptrdol++ = 0xff;
                //
                //       memmove( ptrdol, g_ibuf, de[3] );
                //       ptrdol += de[3];

                         i = len - de[3];
                         memmove( ptrdol, g_ibuf, de[3] );
                         ptrdol += de[3];

                         for( i=0; i<(len-de[3]); i++ )
                            *ptrdol++ = 0xff;

                         break;

                    default:     // case 2.3

                         i = len - de[3];
                         memmove( ptrdol, g_ibuf, de[3] );
                         ptrdol += de[3];

                         for( i=0; i<(len-de[3]); i++ )
                            *ptrdol++ = 0x00;
                    }
              }
            }

          goto BDOL_NEXT;
          }

        if( source == 1 ) // terminal as source only?
          goto BDOL_UNKNOWN;

//  TL_DispHexByte(0,0,tag1);
//  TL_DispHexByte(1,0,tag2);
//  UI_WaitKey();

        // ------ tag(s) source is "ICC" ? ------
        index = Scan_IDE( tag1, tag2, &de[0] );

//  if( (tag1 == 0x5f) && (tag2 == 0x2d) )
//    {
//    apk_ReadRamDataICC( ADDR_ICC_LANG_PREFER, 10, g_temp);
//    TL_DumpHexData(0,0,10,g_temp);
//    apk_ReadRamDataICC( ADDR_ICC_ISU_CTI, 10, g_temp);
//    TL_DumpHexData(0,2,10,g_temp);
//    }

        if( index != 255 )
          {
          // Yes, retrieve the specified data element
          apk_ReadRamDataICC( TL_IDE_GetAddr(index), 1, &de[3] ); // get length
          if( de[3] == 0 )
            memset( g_ibuf, 0x00, 255 );
          else
            apk_ReadRamDataICC( TL_IDE_GetAddr(index)+2, de[3], g_ibuf ); // get data

//TL_DumpHexData(0,0,de[3], g_ibuf);

          // check length
          if( len == de[3] )
            {
            // same length
            memmove( ptrdol, g_ibuf, len );
            ptrdol += len;
            }
          else
            {
            // different length
            if( len < de[3] )
              {
              // 1. the length specified in DOL is less than the actual
              // 1.1 format='n': truncate leftmost bytes
              // 1.2 others    : truncate rightmost bytes
              if( de[2] == DEF_N )
                {
                // case 1.1
                i = de[3] - len;
                memmove( ptrdol, &g_ibuf[i], len );
                ptrdol += len;
                }
              else
                {
                // case 1.2
                memmove( ptrdol, g_ibuf, len );
                ptrdol += len;
                }
              }
            else
              {
              // 2. the length specified in DOL is greater than the actual
              // 2.1 format='n' : paded with leading hex  0x00
              // 2.2 format='cn': paded with leading hex  0xff (changed to trailing hex 0xff)
              // 2.3 others     : paded with trailing hex 0x00
              switch( de[2] )
                    {
                    case DEF_N:  // case 2.1

                         i = len - de[3];
                         for( i=0; i<(len-de[3]); i++ )
                            *ptrdol++ = 0x00;

                         memmove( ptrdol, g_ibuf, de[3] );
                         ptrdol += de[3];

                         break;

                    case DEF_CN: // case 2.2

                //       i = len - de[3];
                //       for( i=0; i<(len-de[3]); i++ )
                //          *ptrdol++ = 0xff;
                //
                //       memmove( ptrdol, g_ibuf, de[3] );
                //       ptrdol += de[3];

                         i = len - de[3];
                         memmove( ptrdol, g_ibuf, de[3] );
                         ptrdol += de[3];

                         for( i=0; i<(len-de[3]); i++ )
                            *ptrdol++ = 0xff;

                         break;

                    default:     // case 2.3

                         i = len - de[3];
                         memmove( ptrdol, g_ibuf, de[3] );
                         ptrdol += de[3];

                         for( i=0; i<(len-de[3]); i++ )
                            *ptrdol++ = 0x00;
                    }
              }
            }
          }
        else // tag(s) source is not found
          {

BDOL_UNKNOWN:
          // fill with hex 0
          for( i=0; i<len; i++ )
             *ptrdol++ = 0x00;
          }

BDOL_NEXT:
        *length += len; // accumulate data element length
        } while( dol_len > 0 );

      return( flag );
}

// ---------------------------------------------------------------------------
// FUNCTION: check if Unpredictable Number contained in CDOL1 or CDOL2.
// INPUT   : ptrobj - pointer to the requested DOL.
//                    berLEN T1-L1 T2-L2 T3-L3...
// OUTPUT  : none.
// REF     : g_ibuf
// RETURN  : TRUE  - yes.
//           FALSE - no.
// ---------------------------------------------------------------------------
UCHAR Check_UnpredictNumInCDOL( UCHAR *ptrobj )
{
UCHAR dol_len;
UCHAR de[4];
UCHAR i;
UCHAR tag1, tag2;
UCHAR len;
UCHAR index;
UCHAR buf[2];
UCHAR cnt;

      // format: LEN[n], T1-L1, T2-L2, T3-L3....
      //         where: LEN = total length, T1 = 1 or 2 bytes, Ln = 1 byte.

      dol_len = apk_GetBERLEN( ptrobj, &cnt ); // get total length
      if( dol_len == 0 ) // empty DOL?
        return( FALSE );

      ptrobj += cnt;

      do{
        tag1 = *ptrobj++;    // MSB of tag
        tag2 = *ptrobj++;    // LSB of tag

        // check constructed data element
        if( apk_CheckConsTag( tag1 ) == TRUE )
          {
          len = (UCHAR)apk_GetBERLEN( --ptrobj, &cnt );
          ptrobj += cnt;

          dol_len -= (1+cnt);
       // goto BDOL_NEXT;
          continue;
          }

        if( apk_CheckWordTag( tag1 ) == TRUE ) // double-byte tag?
          {
          len = (UCHAR)apk_GetBERLEN( ptrobj, &cnt );
          ptrobj += cnt;

          dol_len -= (2+cnt);
          }
        else // single-byte tag
          {
          len = (UCHAR)apk_GetBERLEN( --ptrobj, &cnt );
          ptrobj += cnt;

          tag2 = tag1;
          tag1 = 0;

          dol_len -= (1+cnt);

          if( apk_CheckTermTag( tag2 ) == FALSE )
            {
            if( apk_CheckIccTag( tag2 ) == FALSE )
              {
              if( apk_CheckIssuerTag( tag2 ) == FALSE )
                // a single-byte tag, but its source is neither TERMINAL, ICC or ISSUER
             // goto BDOL_NEXT;
                continue;
              }
            }
          }

        // ------ tag(s) source is "TERMINAL" ? ------
        index = Scan_TDE( tag1, tag2, &de[0] );

        if( index != 255 )
          {
          // Check specical tags for:
          // 9F37: Unpredictable Number(b4)
          if( (tag1 == 0x9F) && (tag2 == 0x37) )
            return( TRUE );
          }

//BDOL_NEXT:
        } while( dol_len > 0 );

      return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: build PDOL.
//           (1) terminal as source.
//           (2) non-constructive.
//           (3) available at this moment.
// INPUT   : ptrobj - pointer to orignal PDOL. (berLEN, T1-L1, T2-L2,...)
// OUTPUT  : pdol - (Tag(83)-L-V), (V=data elements)
//                  concatenation of all related data elements of the DOL type.
// REF     : g_ibuf
// RETURN  : TRUE  = dol built OK.
//           FALSE = dol not found. (PDOL is empty)
// ---------------------------------------------------------------------------
UCHAR Build_PDOL( UCHAR *ptrobj, UCHAR *pdol )
{
//UCHAR *ptrobj;
UCHAR *ptrdol;
//UINT  pdol_len;
//UCHAR de[4];
//UCHAR i;
//UCHAR tag1, tag2;
//UCHAR len;
//UCHAR index;
//UCHAR fci[SELECTED_FCI_LEN];
UCHAR buf[2];
//UCHAR cnt;
UCHAR length[1];
UINT  iLen;

      // retrieve the selected FCI
//    apk_ReadRamDataICC( ADDR_SELECTED_FCI, SELECTED_FCI_LEN, fci );
//    ptrobj = apk_FindTag( 0x9f, 0x38, fci ); // Tag=9F38 (PDOL)?

      // reset default empty PDOL
      pdol[0] = 0x83; // tag
      pdol[1] = 0;    // total data elements length
      ptrdol = &pdol[2];

      if( ptrobj != 0 ) // PDOL found
        {
        // format: LEN[n], T1-L1, T2-L2, T3-L3....
        //         where: LEN = total length, T1 = 1 or 2 bytes, Ln = 1 byte.

        Build_DOL( 1, ptrobj, length, ptrdol );

//        pdol_len = apk_GetBERLEN( ptrobj, &cnt ); // get total length
//        ptrobj += cnt;
//
//        do{
//          tag1 = *ptrobj++;    // MSB of tag
//          tag2 = *ptrobj++;    // LSB of tag
//
//          // check constructed data element
//          if( apk_CheckConsTag( tag1 ) == TRUE )
//            {
//            len = (UCHAR)apk_GetBERLEN( --ptrobj, &cnt );
//            ptrobj += cnt;
//
//            pdol_len -= (1+cnt);
//            goto BPDOL_UNKNOWN;
//            }
//
//          if( apk_CheckWordTag( tag1 ) == TRUE ) // double-byte tag?
//            {
//            len = (UCHAR)apk_GetBERLEN( ptrobj, &cnt );
//            ptrobj += cnt;
//
//            pdol_len -= (2+cnt);
//            }
//          else // single-byte tag
//            {
//            len = (UCHAR)apk_GetBERLEN( --ptrobj, &cnt );
//            ptrobj += cnt;
//
//            tag2 = tag1;
//            tag1 = 0;
//
//            pdol_len -= (1+cnt);
//
//            if( apk_CheckTermTag( tag2 ) == FALSE )
//              // a single-byte tag, but its source is not TERMINAL
//              goto BPDOL_UNKNOWN;
//            }
//
////        TL_DispHexByte(0,0,tag1);
////        TL_DispHexByte(1,0,tag2);
////        TL_DispHexByte(2,0,len);
////        TL_DispHexByte(3,0,pdol_len);
////        UI_WaitKey();
//
//          // tag(s) source is "TERMINAL" ?
//          index = Scan_TDE( tag1, tag2, &de[0] );
//
//          if( index != -1 )
//            {
//            // Yes, retrieve the specified data element
//            apk_ReadRamDataTERM( TL_TDE_GetAddr(index), de[3], g_ibuf );
//
//            // check length
//            if( len == de[3] )
//              {
//              // same length
//              memmove( ptrdol, g_ibuf, len );
//              ptrdol += len;
//              }
//            else
//              {
//              // different length
//              if( len < de[3] )
//                {
//                // 1. the length specified in DOL is less than the actual
//                // 1.1 format='n': truncate leftmost bytes
//                // 1.2 others    : truncate rightmost bytes
//                if( de[2] == DEF_N )
//                  {
//                  // case 1.1
//                  i = de[3] - len;
//                  memmove( ptrdol, &g_ibuf[i], len );
//                  ptrdol += len;
//                  }
//                else
//                  {
//                  // case 1.2
//                  memmove( ptrdol, g_ibuf, len );
//                  ptrdol += len;
//                  }
//                }
//              else
//                {
//                // 2. the length specified in DOL is greater than the actual
//                // 2.1 format='n' : paded with leading hex  0x00
//                // 2.2 format='cn': paded with leading hex  0xff
//                // 2.3 others     : paded with trailing hex 0x00
//                switch( de[2] )
//                      {
//                      case DEF_N:  // case 2.1
//
//                           i = len - de[3];
//                           for( i=0; i<(len-de[3]); i++ )
//                              *ptrdol++ = 0x00;
//
//                           memmove( ptrdol, g_ibuf, de[3] );
//                           ptrdol += de[3];
//
//                           break;
//
//                      case DEF_CN: // case 2.2
//
//                           i = len - de[3];
//                           for( i=0; i<(len-de[3]); i++ )
//                              *ptrdol++ = 0xff;
//
//                           memmove( ptrdol, g_ibuf, de[3] );
//                           ptrdol += de[3];
//
//                           break;
//
//                      default:     // case 2.3
//
//                           i = len - de[3];
//                           memmove( ptrdol, g_ibuf, de[3] );
//                           ptrdol += de[3];
//
//                           for( i=0; i<(len-de[3]); i++ )
//                              *ptrdol++ = 0x00;
//                      }
//                }
//              }
//            }
//          else // tag(s) source is not "TERMINAL"
//            {
//BPDOL_UNKNOWN:
//            // fill with hex 0
//            for( i=0; i<len; i++ )
//               *ptrdol++ = 0x00;
//            }
//
//          pdol[1] += len; // accumulate data element length
//          } while( pdol_len > 0 );
//
        pdol[1] = length[0]; // put final length

        iLen = 2;
        if( apk_SetBERLEN( pdol[1], buf ) == 2 ) // convert to BER length
          {
          // T-L-V --> T-L-L-V
          memmove( g_ibuf, &pdol[2], pdol[1] );
          memmove( &pdol[3], g_ibuf, pdol[1] );

          pdol[1] = buf[0];
          pdol[2] = buf[1];

          iLen = 3;
          }

        // backup PDOL concatenation results (2L 83-L-V)
        iLen += length[0];
        apk_WriteRamDataICC( ADDR_ICC_PDOL_CATS, 2, (UCHAR *)&iLen );
        apk_WriteRamDataICC( ADDR_ICC_PDOL_CATS+2, iLen, pdol );

        return( TRUE );  // PDOL built OK
        }
      else
        return( FALSE ); // PDOL not found
}

// ---------------------------------------------------------------------------
// FUNCTION: build DDOL.
// INPUT   : ptrobj - pointer to orignal DDOL. (berLEN, T1-L1, T2-L2,...)
// OUTPUT  : ddol - (L-V), (V=data elements)
//                  L = total length of the concatenated data elements.
//                  concatenation of all related data elements of the DOL type.
// REF     : g_ibuf
// RETURN  : TRUE.
// ---------------------------------------------------------------------------
UCHAR Build_DDOL( UCHAR *ptrobj, UCHAR *ddol )
{
UCHAR length[1];

      Build_DOL( 0, ptrobj, length, &ddol[1] ); // build dol
      ddol[0] = length[0]; // set final length of concatenation

      return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: build CDOL.
// INPUT   : ptrobj - pointer to orignal CDOL. (berLEN, T1-L1, T2-L2,...)
// OUTPUT  : cdol - (L-V), (V=data elements)
//                  L = total length of the concatenated data elements.
//                  concatenation of all related data elements of the DOL type.
// REF     : g_ibuf
//           g_obuf
// RETURN  : TRUE.
// ---------------------------------------------------------------------------
UCHAR Build_CDOL( UCHAR *ptrobj, UCHAR *cdol )
{
UCHAR length[1];
UCHAR flag;

      flag = Build_DOL( 0, ptrobj, length, &cdol[1] ); // build dol
      cdol[0] = length[0]; // set final length of concatenation

      return( flag );
}

// ---------------------------------------------------------------------------
// FUNCTION: update transaction amount to terminal-related data objects.
// INPUT   : amt - the current transaction amount (n10+n2).
// OUTPUT  : none.
// REF     :
// RETURN  : none.
// ---------------------------------------------------------------------------
void InitAP_UpdateTxAmt( UCHAR *amt )
{
UCHAR buf[4];

      apk_WriteRamDataTERM( ADDR_TERM_AMT_AUTH_N, 6, amt );
      apk_WriteRamDataTERM( ADDR_TERM_TX_AMT, 6, amt );

      TL_bcd2hex( 5, &amt[1], buf ); // convert to binary format
      apk_WriteRamDataTERM( ADDR_TERM_AMT_AUTH_B, 4, buf );
}

// ---------------------------------------------------------------------------
// FUNCTION: Find & Concatenate Data Object List (DOL).
// INPUT   : type - type of DOL (DOL_XXX).
//           in_dol  - the dol list. (berLEN-V)
// OUTPUT  : out_dol - (1) T-L-V, (V = the data field of the DOL) or
//                     (2) L-V
//                     concatenation of all related data elements of the DOL type.
// REF     : g_obuf - the final DOL data elements.
// RETURN  : emvOK
//           emvFailed (DOL not found) -- no actual effect in application.
// ---------------------------------------------------------------------------
UCHAR api_emv_ConcatenateDOL( UCHAR type, UCHAR *in_dol, UCHAR *out_dol )
{
UCHAR result=0;

      switch( type )
            {
            case DOL_PDOL:

                 result = Build_PDOL( in_dol, out_dol );
                 break;

            case DOL_CDOL1:
            case DOL_CDOL2:

                 result = Build_CDOL( in_dol, out_dol );
                 break;

            case DOL_TDOL:

                 break;

            case DOL_DDOL:

                 result = Build_DDOL( in_dol, out_dol );
                 break;
            }

      if( result == TRUE )
        return( emvOK );
      else
        return( emvFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: verify the contents of all AFL entries.
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_ibuf
// RETURN  : TURE  - correct AFL.
//           FALSE - incorrect AFL.
// ---------------------------------------------------------------------------
UCHAR Verify_AFL( void )
{
UCHAR i;
UCHAR sfi;
UCHAR st_rec_num;
UCHAR end_rec_num;
UCHAR oda_rec_cnt;

      apk_ReadRamDataICC( ADDR_ICC_AFL, ICC_AFL_TOTAL_LEN, g_ibuf ); // n AFL[4n]

      // check all AFL entries
      for( i=0; i<g_ibuf[0]; i++ )
         {
         sfi = g_ibuf[1+i*ICC_AFL_LEN];
         st_rec_num = g_ibuf[2+i*ICC_AFL_LEN];
         end_rec_num = g_ibuf[3+i*ICC_AFL_LEN];
         oda_rec_cnt = g_ibuf[4+i*ICC_AFL_LEN];

//       if( (sfi == 0) || ((sfi & 0x07) != 0) || ((sfi & 0xf8) == 0xf8) ||
	 if( (sfi == 0) || ((sfi & 0xf8) == 0xf8) ||		// 2016-12-07, 2CA.041.16, SB140: The three least significant bits are RFU.
             (st_rec_num == 0) || (end_rec_num < st_rec_num) ||
             ((end_rec_num - st_rec_num + 1) < oda_rec_cnt) )

           return( FALSE );
         }

      return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: initiate application.
// INPUT   : tx_type - current transaction type.   (n2)
//           tx_amt  - current transaction amount. (n10+n2, n2 is RFU)
//                     this amount will be used to update terminal DO.
// OUTPUT  : none.
// REF     : g_ibuf, g_obuf
// RETURN  : emvOK
//           emvNotReady (means this application cannot be applied) -> select next one
//           emvFailed   (init failed) -> terminate transaction.
//           emvFallBack (JCB SPEC)    -> Fall back transaction.
// ---------------------------------------------------------------------------
UCHAR api_emv_InitApplication( UCHAR tx_type, UCHAR *tx_amt )
{
UCHAR i;
UINT  iLen;
UCHAR cnt;
UCHAR *ptrobj;
UCHAR result;
UCHAR fci[SELECTED_FCI_LEN];
UCHAR buf1[16];
UCHAR buf2[16];


      // setup current transaction Date & Time + Clear CVMR
      // PATCH: PBOC2.0, 2006-02-21, move get_date_time prior to GET_PROCESSING_OPTIONS
      //        because PDOL may request transaction date, time, and CVMR
      TL_GetDateTime( &buf1[1] );

      buf1[0]=6;
      TL_asc2bcd( 3, buf2, buf1 );     // convert to BCD, buf2: YYMMDD
      apk_WriteRamDataTERM( ADDR_TERM_TX_DATE, 3, buf2 );

      buf1[6]=6;
      TL_asc2bcd( 3, buf2, &buf1[6] ); // convert to BCD, buf2: HHMMSS
      apk_WriteRamDataTERM( ADDR_TERM_TX_TIME, 3, buf2 );

      buf1[0] = 0x00;
      buf1[1] = 0x00;
      buf1[2] = 0x00;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_CVMR, 3, buf1 );


      // setup transaction amount
      InitAP_UpdateTxAmt( tx_amt );

      // clear TVR & TSI to 0
      for( i=0; i<5; i++ )
         g_term_TVR[i] = 0;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR, 5, &g_term_TVR[0] );

      for( i=0; i<2; i++ )
         g_term_TSI[i] = 0;
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TSI, 2, &g_term_TSI[0] );

      // setup transction type
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TX_TYPE, 1, (UCHAR *)&tx_type );

      // retrieve PDOL from the selected FCI
      apk_ReadRamDataICC( ADDR_SELECTED_FCI, SELECTED_FCI_LEN, fci );
      ptrobj = apk_FindTag( 0x9f, 0x38, fci ); // Tag=9F38 (PDOL)

      // prepare data elements for PDOL
      api_emv_ConcatenateDOL( DOL_PDOL, ptrobj, g_obuf ); // 83-L-V

      // issue "GET PROCESSING OPTIONS" command
      // g_obuf: the PDOL.
      // g_ibuf: the response of the command.
      //         (1) primitive  : 2L 80-L-AIP[2]-AFL[4n] SW1 SW2, n=1..63
      //             or
      //         (2) constructed: 2L 77-L-[...(82-L-V)..(94-L-V)...] SW1 SW2
      //             where: Tag 82=AIP, 94=AFL.

      result = apk_InitApp( g_obuf, g_ibuf );

//    if( result == apkFallBack ) // JCB SEPC
//      return( emvFallBack );

      if( result == apkOK )
        {
        iLen = apk_GetBERLEN( &g_ibuf[3], &cnt ); // get total length of data elements

        // check legal length
//      if( ((g_ibuf[1]*256 + g_ibuf[0]) < 10) || (iLen < 6) )
//        return( emvFailed );
        if( iLen < 6 )
          return( emvFailed );

        // check response format (1)
        if( g_ibuf[2] == 0x80 )
          {
          // save AIP[2]
          g_icc_AIP[0] = g_ibuf[3+cnt]; // 4+(cnt-1)
          g_icc_AIP[1] = g_ibuf[4+cnt]; // 5+(cnt-1)
          apk_WriteRamDataICC( ADDR_ICC_AIP, 2, &g_ibuf[3+cnt] );

          // save AFL[4n]
          g_ibuf[4+cnt] = (iLen-2) / 4; // the value 'n'
          apk_WriteRamDataICC( ADDR_ICC_AFL, iLen-1, &g_ibuf[4+cnt] ); // n AFL[4n]

          // PATCH: 2003-06-12 JCB SPEC (also good for EMV SPEC)
          //        To verify AFL before actual reading records.
          if( Verify_AFL() == FALSE )
            return( emvFailed );

//        InitAP_UpdateTxAmt( tx_amt );
          return( emvOK ); // done
          }

        // check response format (2)
        if( (g_ibuf[2] == 0x77) || (g_ibuf[2] == 0x80) )
          {
          // find AIP[2]
          ptrobj = apk_FindTag( 0x82, 0x00, &g_ibuf[2] );
          if( ptrobj != 0 )
            {
            iLen = apk_GetBERLEN( ptrobj, &cnt );
            if(iLen==0)//08-dec-22	charles update
            	return ( emvFailed ); // AIP not present
            // save AIP[2]
            g_icc_AIP[0] = *(ptrobj+cnt);
            g_icc_AIP[1] = *(ptrobj+cnt+1);
            apk_WriteRamDataICC( ADDR_ICC_AIP, 2, ptrobj+cnt );
            }
          else
            return( emvFailed ); // AIP not present

          // find AFL[4n]
          ptrobj = apk_FindTag( 0x94, 0x00, &g_ibuf[2] );
          if( ptrobj != 0 )
            {
            iLen = apk_GetBERLEN( ptrobj, &cnt );

            // save AFL[4n]
            *(ptrobj+cnt-1) = iLen / 4; // the value 'n'
            apk_WriteRamDataICC( ADDR_ICC_AFL, iLen+1, ptrobj+cnt-1 ); // n AFL[4n]
            }
          else
            return( emvFailed ); // AFL not present

          // PATCH: 2003-06-12 JCB SPEC (also good for EMV SPEC)
          //        To verify AFL before actual reading records.
          if( Verify_AFL() == FALSE )
            return( emvFailed );

//        InitAP_UpdateTxAmt( tx_amt );
          return( emvOK ); // done
          }
        else
          return( emvFailed ); // invalid response tag format
        }
      else
        {
        if( result == apkCondNotSatisfied )
          return( emvNotReady); // not available
        else
          return( emvFailed );  // init app failed
        }
}

