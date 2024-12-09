//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : L2API_06.C                                                 **
//**  MODULE   : api_emv_ReadApplicationData()                              **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/08                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2002-2018 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "EMVKconfig.h"

#include "POSAPI.h"
#include "EMVDC.h"
#include "GDATAEX.h"
#include "EMVAPI.h"
#include "EMVAPK.h"
#include "TOOLS.h"

extern	UCHAR	SRED_Func_StorePAN( UCHAR *pan, UCHAR length );


// ---------------------------------------------------------------------------
// FUNCTION: scan icc data element table.
// INPUT   : tag1 - the MSB of Tag. (0 if only LSB is present)
//           tag2 - the LSB of Tag.
// OUTPUT  : de   - the data element entry found. (TAG1 TAG2 FORMAT LEN)
// RETURN  : -1     = tag not found.
//           others = the index of the found tag(s) in the table. (0x00..N)
// ---------------------------------------------------------------------------
//UCHAR Scan_IDE( UCHAR tag1, UCHAR tag2, UCHAR *de )
//{
//UCHAR ide[4];
//UCHAR i, j;
//
//      for( i=0; i<MAX_IDE_CNT; i++ )
//         {
//         TL_IDE_GetData( i, ide ); // load 1 icc data element
//
//         if( (tag1 == ide[0]) && (tag2 == ide[1]) )
//           {
//           for( j=0; j<4; j++ )
//              *de++ = ide[j];
//           return( i );
//           }
//         }
//      return(-1);
//}

// ---------------------------------------------------------------------------
// FUNCTION: (1) The terminal shall read the files and records indicated in
//               the AFL.
//           (2) The terminal shall store all the recognized data objects read.
//           (3) If an error prevents the terminal from reading data from the
//               ICC, the transaction shall be terminated.
//           (4) Any SW1 SW2 other than '9000' passed to the application layer
//               shall cause the transaction to be terminated.
//           (5) If any mandatory data objects are not present, the terminal
//               shall terminate the transaction.
//               - Application Expiry Date (Tag=5F 24)
//               - PAN (Tag=5A)
//               - CDOL1 (Tag=8C)
//               - CDOL2 (Tag=8D)
//           (6) If the terminal encounters more than one occurrence of a
//               single primitive data object while reading data from ICC,
//               the terminal shall be terminated.
//               (ie, no duplicated data object is allowed.)
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_ibuf
// RETURN  : emvOK
//           emvFailed (will terminate the transaction)
// NOTE    : AFL
//           byte 1: SFI.
//           byte 2: the first record nbr to be read.
//           byte 3: the last record nbr to be read.
//           byte 4: the nbr of records involved in offline data authen.
// ---------------------------------------------------------------------------
UCHAR api_emv_ReadApplicationData( void )
{
UCHAR i, j;
UCHAR sfi;
UCHAR st_rec_num;
UCHAR end_rec_num;
UCHAR oda_rec_cnt;
UCHAR oda_flag[1];
UCHAR rec_len[2];
UCHAR *ptrrec;
UCHAR *ptrnext;
UCHAR afl[ICC_AFL_TOTAL_LEN]; // application file locator
UCHAR ptrtlv[261];            // Tag1-Tag2-L-L-L-Data[256]
UCHAR de[4];
UCHAR index;
UCHAR icc_rec_num;            // backup rec number
UINT  iTempLen;
UINT  iLen;
UCHAR cnt;
UCHAR padlen[1];
UCHAR *ptrobj;
//UCHAR flag_tk2;               // track 2 equivalent data
UCHAR flag_SKIP = FALSE;	// for SB159


      // clear all ICC-related "Data Elements" & "Records" to 0
      // NOTE: there is a bug in IAR C-compiler for the following expression:
      //       iLen = ADDR_ICC_DE_END - ADDR_ICC_DE_START !!!
      iLen = ADDR_ICC_DE_END;
      iLen -= ADDR_ICC_DE_START;
      apk_ClearRamDataICC( ADDR_ICC_DE_START, iLen, 0 );

      iLen = ADDR_ICC_REC_END;
      iLen -= ADDR_ICC_REC_START;
      apk_ClearRamDataICC( ADDR_ICC_REC_START, iLen, 0 );

      // load all AFL entries
      // format: CNT[1] AFL[4*n]
      apk_ReadRamDataICC( ADDR_ICC_AFL, ICC_AFL_TOTAL_LEN, afl );

      // store AFL & AIP to AFL2 & AIP2
      de[0] = afl[0] << 2;
      de[1] = 0;
      apk_WriteRamDataICC( ADDR_ICC_AFL2, 2, de );
      apk_WriteRamDataICC( ADDR_ICC_AFL2+2, de[0], &afl[1] );

      de[0] = 2;
      de[1] = 0;
      apk_ReadRamDataICC( ADDR_ICC_AIP, 2, &de[2] );
      apk_WriteRamDataICC( ADDR_ICC_AIP2, 4, de );

      icc_rec_num = 0;

//    flag_tk2 = TRUE; // refrenece only

      // read all records in AFL entries from left to right
      for( i=0; i<afl[0]; i++ )
         {
         sfi = afl[1+i*ICC_AFL_LEN];
         sfi &= 0xF8;	// 2016-12-07, 2CA.041.16, SB140: The three least significant bits are RFU.
         st_rec_num = afl[2+i*ICC_AFL_LEN];
         end_rec_num = afl[3+i*ICC_AFL_LEN];
         oda_rec_cnt = afl[4+i*ICC_AFL_LEN];

         sfi = (sfi >> 3) & 0x1f; // sfi = 1..30

         // REMOVED: 2003-06-12
         //          The following verification is moved to "Verify_AFL()"
         //          in "api_emv_InitApplication()".

//       if( (sfi == 0) || ((sfi & 0x07) != 0) || ((sfi & 0xf8) == 0xf8) ||
//           (st_rec_num == 0) || (end_rec_num < st_rec_num) ||
//           ((end_rec_num - st_rec_num + 1) < oda_rec_cnt) )
//         return( emvFailed );
//       else
//         sfi = (sfi >> 3) & 0x1f; // sfi = 1..30

#ifdef  L2_SW_DEBUG
         g_test_read_app_data = 1;
#endif

         // read, check, and store all recognized data objects for the SFI
         for( j=st_rec_num; j<=end_rec_num; j++ )
            {
            // read 1 record data: LEN[2] 70-L-V
            if( apk_ReadRecord( sfi, j, g_ibuf ) != apkOK )
              return( emvFailed );

//    TL_DumpHexData(0,0,g_ibuf[0]+2, g_ibuf);
//    UI_WaitKey();

            // JCB & VIS SPEC: check track 2 equivalent data (Tag=57) in SFI=1, rec=1
// REMOVED  if( (sfi == 1) && (j == 1) )
// by JCB-    {
// amemdment  ptrobj = apk_FindTag( 0x57, 0x00, &g_ibuf[2] );
// v1.2       if( ptrobj == 0 )
//              flag_tk2 = FALSE; // not found
//            }

            // check & save each data element in the record
            // (1) check record LENGTH & template TAG=70
            if( (g_ibuf[3] & 0x80) == 0 )
              {
              iTempLen = g_ibuf[3]; // 1-byte length (1..127)
              ptrrec = &g_ibuf[4];  // pointer to the starting of DO
              cnt = 1;
              }
            else // 2-byte length
              {
              switch( g_ibuf[3] & 0x7f )
                    {
                    case 0x01: // 1-byte length (128..255)
                         iTempLen = g_ibuf[4];
                         ptrrec = &g_ibuf[5];
                         cnt = 2;
                         break;

                    case 0x02: // 2-byte length (256..65535)
                         iTempLen = g_ibuf[4]*256 + g_ibuf[5];
                         ptrrec = &g_ibuf[6];
                         cnt = 3;
                         break;

                    default:   // out of spec
                         return( emvFailed );
                    }
              }

            if( g_ibuf[2] == 0x70 ) // Tag = 70?
              {
        //    iLen = g_ibuf[1]*256 + g_ibuf[0];
        //
        //    if( iLen < (1 + cnt + 2) )
        //      return( emvFailed );
        //
        //    iLen -= (1 + cnt + 2); // 70 LEN SW1 SW2
        //
        //    if( iLen != iTempLen )
        //      return( emvFailed );

              if( apk_CheckIsoPadding_Right( g_ibuf, padlen ) == FALSE )
                return( emvFailed );
              }

            // (2) backup the record data (fODA-T-L-V, excluding SW1 SW2)
            if( oda_rec_cnt != 0 )
              {
              // set ODA record flag
              oda_flag[0] = sfi + 0x80;
              apk_WriteRamDataICC( ADDR_ICC_REC_01+ICC_REC_LEN*icc_rec_num, 1, oda_flag );
              oda_rec_cnt--;
              }
            else
              {
              oda_flag[0] = sfi;
              apk_WriteRamDataICC( ADDR_ICC_REC_01+ICC_REC_LEN*icc_rec_num, 1, oda_flag );
              }

            apk_WriteRamDataICC( ADDR_ICC_REC_01+(ICC_REC_LEN*icc_rec_num)+1, (g_ibuf[1]*256+g_ibuf[0])-2, &g_ibuf[2] );
            icc_rec_num++;
            if( icc_rec_num >= MAX_ICC_REC_CNT )
              return( emvFailed ); // out of memory

            if( g_ibuf[2] != 0x70 ) // Tag = 70?
              {
           // continue; // non Tag-70 record, ignore the data elements in it.
              if( sfi >= 11 )
                continue;
              return( emvFailed ); // PATCH: PBOC2.0, 2006-02-16, 2CL030, if Tag error then reject Trans.
              }

            // store the recognized data elements
            // rec_len[0] = g_ibuf[3];
            // ptrrec = &g_ibuf[4];
            rec_len[0] = iTempLen & 0x00ff;
            rec_len[1] = (iTempLen & 0xff00) >> 8;

            // ignore the data elements with SFI = 11..30 (out of EMV spec)
            if( sfi > 10 )
              continue; // go to next record

            while(1)
                 {
                 // read one data element
                 ptrnext = apk_GetBERTLV( &rec_len[0], ptrrec, ptrtlv );
                 if( ptrnext == (UCHAR *)-1 )
                   return( emvFailed );

                 if( ptrnext != 0 )
                   {

//      UI_ClearScreen();
//      TL_DispHexByte(0,0,ptrtlv[0]);
//      TL_DispHexByte(0,2,ptrtlv[1]);

                   // (3) tag(s) source is "ICC" ?
                   index = apk_ScanIDE( ptrtlv[0], ptrtlv[1], &de[0] );

//      TL_DispHexByte(0,5,index);
//      UI_WaitKey();

        //         TL_DispHexByte(0,0,i);
        //         TL_DispHexByte(1,0,j);
        //         TL_DispHexByte(2,0,icc_rec_num);
        //         TL_DumpHexData(0,3,4, de);
        //         for(;;);

                   if( index != 255 )
                     {

//      TL_DispHexByte(1,0,ptrtlv[0]);
//      TL_DispHexByte(1,2,ptrtlv[1]);
//      TL_DispHexByte(1,5,index);
//      UI_WaitKey();

                     // (4) known tag(s), check redundancy & save the data element
                     apk_ReadRamDataICC( TL_IDE_GetAddr(index), 2, (UCHAR *)&iLen ); // load length value
                     if( iLen == 0 )
                       {
                       flag_SKIP = FALSE;	// 2016-12-28
                                        
                       // JCB SPEC: check legal LENGTH field
                       if( de[3] != 0 ) // skip checking if expected len = 0
                         {
                         iLen = ptrtlv[3]*256 + ptrtlv[2]; // current len
                         if( iLen > (UINT)de[3] ) // current len > expected len?
                           {
                           // 2016-12-07, 2CE.003.12.01, SB159, ignore formatting error for the following data objectes, and as if it is not present.
                           //	Cardholder Name ('5F20')
                           //	Cardholder Name Extended ('9F0B')
                           //	Issuer URL ('5F50')
                           //	Log Entry ('9F4D')
                           //	Log Format ('9F4F')’
                           if( !((ptrtlv[0] == 0x5F) && (ptrtlv[1] == 0x20)) &&
                               !((ptrtlv[0] == 0x9F) && (ptrtlv[1] == 0x0B)) &&
                               !((ptrtlv[0] == 0x5F) && (ptrtlv[1] == 0x50)) &&
                               !((ptrtlv[0] == 0x9F) && (ptrtlv[1] == 0x4D)) &&
                               !((ptrtlv[0] == 0x9F) && (ptrtlv[1] == 0x4F)) )
                             return( emvFailed );   // invalid
                           else
                             flag_SKIP = TRUE;	// 2016-12-28
                           }
                         }

		       if( !flag_SKIP )
		       	 {
                         apk_WriteRamDataICC( TL_IDE_GetAddr(index), ptrtlv[3]*256+ptrtlv[2]+2, &ptrtlv[2] ); // LL DATA
                       
                         // SRED: save encrypted PAN for APP access
//#ifdef	_SRED_ENABLED_
                         if( (ptrtlv[0] == 0x00) && (ptrtlv[1] == 0x5A) )
//                     	   SRED_Func_StorePAN( &ptrtlv[4], (UCHAR)(ptrtlv[3]*256+ptrtlv[2]) );
                       	   api_emv_PutDataElement_SRED( DE_ICC, ADDR_ICC_AP_PAN+2, (UCHAR)(ptrtlv[3]*256+ptrtlv[2]), &ptrtlv[4] );
//#endif
			 }
                       }
                     else
                       return( emvFailed ); // already existed
                     }

                   ptrrec = ptrnext; // update next data object

                   // check redundancy
                   iLen = rec_len[1]*256 + rec_len[0];
                   if( (iLen == 1) || (iLen == 2) )
                   {
                       if( (*ptrrec == 0x00) || (*ptrrec == 0xff) )
                         {
                         if( TL_memcmpc( ptrrec, *ptrrec, (UCHAR)iLen ) != 0 )
                           return( emvFailed );
                         }
                       else
                       {
                      	 if(ptrrec[0]==0x8e)
			   continue;//09-jan-08 charles ,當cvm list的長度為0時，不可直接結束
			 else
			   {
			   // 2018-04-18, Issuer Public Key Remainder? (T=92 L=0)
			   // AMEX: AXP_EMV_016___ODA_Failure
			   if( (iLen == 2) && (ptrrec[0] == 0x92) && (ptrrec[1] == 0x00) )
			     continue;
			   else
  			     return( emvFailed );
  			   }
                       }

                    }
                   }
                 else // end of current record analysis
                   break;
                 } // while(1)
            } // for(j), next record
         } // for(i), next SFI

      // (5) check mandatory data objects
      apk_ReadRamDataICC( ADDR_ICC_AP_EXPIRE_DATE, 2, (UCHAR *)&iLen );
      if( iLen == 0 )
        return( emvFailed );

      apk_ReadRamDataICC( ADDR_ICC_AP_PAN, 2, (UCHAR *)&iLen );
      if( iLen == 0 )
        return( emvFailed );

      apk_ReadRamDataICC( ADDR_ICC_CDOL1, 2, (UCHAR *)&iLen );
      if( iLen == 0 )
        return( emvFailed );

      apk_ReadRamDataICC( ADDR_ICC_CDOL2, 2, (UCHAR *)&iLen );
      if( iLen == 0 )
        return( emvFailed );

      // JCB & VIS SPEC: check track 2 equivalent data (deleted by JCB v1.2 amendments)
//    if( flag_tk2 == FALSE )
//      return( emvFailed );

      // verify "Application Expiration Date"
      apk_ReadRamDataICC( ADDR_ICC_AP_EXPIRE_DATE+2, 3, ptrtlv );
      if( TL_CheckDate( ptrtlv ) == FALSE )
        return( emvFailed );

      // verify "Application Effective Date" if present
      apk_ReadRamDataICC( ADDR_ICC_AP_EFFECT_DATE, 2, (UCHAR *)&iLen );
      if( iLen != 0 )
        {
        apk_ReadRamDataICC( ADDR_ICC_AP_EFFECT_DATE+2, 3, ptrtlv );
        if( TL_CheckDate( ptrtlv ) == FALSE )
          return( emvFailed );
        }

//    for(i=0; i<8; i++)
//       {
//       apk_ReadRamDataICC( ADDR_ICC_REC_01+i*ICC_REC_LEN, 8, g_ibuf);
//       TL_DumpHexData(0,0,8, g_ibuf);
//       }
//    for(;;);

      return( emvOK );
}

