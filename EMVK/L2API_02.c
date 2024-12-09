//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : L2API_02.C                                                 **
//**  MODULE   : api_emv_CreateCandidateList()                              **
//**             api_emv_GetCandidateList()                                 **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2002/12/05                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2002-2009 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "POSAPI.h"
//#include <EMVDC.h>
#include "GDATAEX.h"
#include "EMVAPI.h"
#include "EMVAPK.h"

UCHAR CCL_SelectFromTermList( void );
UCHAR CCL_SelectFromTermList_PBOC( void );

// ---------------------------------------------------------------------------
// FUNCTION: Create a list of ICC applications that are supported by the term.
// INPUT   : none.
// OUTPUT  : none.
// REF     : none.
// RETURN  : emvOK
//           emvFailed
//           emvNotReady - no mutually supported application.
// ---------------------------------------------------------------------------
UCHAR api_emv_CreateCandidateList( void )
{
UCHAR result;
UCHAR *ptrobj;
UCHAR *ptrdir;
UCHAR *bak_ptrdir;
UCHAR sfi;          // short file id
UCHAR rec_num;      // record number
UINT  rec_len;
UINT  dir_len;
UINT  iLen;
UCHAR stktop[1];
UINT  temp;
UCHAR cnt;
UCHAR reclen[2];
UCHAR fSelectDDF;

     // clear "Candidate List"
     iLen = ADDR_CANDIDATE_END;
     iLen -= ADDR_CANDIDATE_START;
     apk_ClearRamDataICC( ADDR_CANDIDATE_START, iLen, 0 );
     iLen = ADDR_CANDIDATE_NAME_END;
     iLen -= ADDR_CANDIDATE_NAME_START;
     apk_ClearRamDataICC( ADDR_CANDIDATE_NAME_START, iLen, 0 );
     // select PSE
     result = apk_SelectPSE( g_ibuf );
CCL_SELECT_PSE:
     switch( result )
           {
           case apkOK:
                goto CCL_PSE_FOUND;

           case apkUnknown:
                                                    // PATCH: 2005/01/21
                                                    // MCHIP LITE test card returns "67 00"
                if( ( g_ibuf[2] & 0xf0 ) == 0x60 )  // SW1 = 6x ?
                  {
                  if( CCL_SelectFromTermList() == emvOK )
                    goto  CCL_FINAL;
                  else
                    return( emvFailed );
                  }
                else
                  return( emvFailed );
 
           case apkFciMissing:       // 9000, fci missing
           case apkFileNotFound:     // 6A82, file not found
           case apkFileInvalidated:  // 6283, application blocked
           case apkIncorrect:        // EMV2000

                if( CCL_SelectFromTermList() == emvOK )
                  goto  CCL_FINAL;
                else
                  return( emvFailed );

           case apkFuncNotSupported: // 6A81, card blocked
                return( emvFailed );

           default: // failed
                return( emvFailed );
           }

CCL_PSE_FOUND:

     // Empty Candidate List
     apk_EmptyCandidateList();
     *stktop = 0;
     fSelectDDF = 0;

CCL_GET_SFI:

     // Get SFI for Dir from FCI
     iLen = g_ibuf[1]*256 + g_ibuf[0];
     iLen -= 2; // total length, ignore SW1 SW2

     if( ( g_ibuf[2] != 0x6F ) || ( g_ibuf[3] > iLen ) ) // check FCI Template & Length
       return( emvFailed );

     ptrobj = apk_FindTag( 0x88, 0x00, &g_ibuf[2] );  // Tag=88, SFI of the dir EF
     
     apk_GetBERLEN( ptrobj, &cnt );
     sfi = *(ptrobj + cnt);

     rec_num = 1; // set record number to 1 for next read

CCL_READ_DIR_REC:

     g_ibuf[0] = 0; // clear output length
     g_ibuf[1] = 0; //
     result = apk_ReadRecord( sfi, rec_num, g_ibuf );  // read dir record

     if( (result == apkRecNotFound) && (rec_num == 1) )
       {
       if( fSelectDDF == 1 ) // PATCH: 2006-10-02
         {
         fSelectDDF = 0;
         goto CCL_REC_NOT_FOUND;
         }

       result = apkFileInvalidated; // switch to "list of AIDs"
       goto CCL_SELECT_PSE;
       }

     if( result == apkFailed )
       {
       // PATCH: 2005/03/21, EMV2000
       if( (g_ibuf[0]+g_ibuf[1]*256) >= 2 ) // chk output length
         {
         result = apkFileInvalidated; // switch to "list of AIDs"
         goto CCL_SELECT_PSE;
         }
       else
         return( emvFailed );
       }

     if( result == apkOK )  // record found?
       {
       // is there an entry in this record?
       // 2L [70-L-(61-L-DIR1)...(61-L-DIRn)]
       if(g_ibuf[2] != 0x70)
//        return( emvFailed );
         {
         result = apkFileInvalidated; // PATCH: 2006-10-05
         goto CCL_SELECT_PSE;
         }
       else
         {

         if( apk_ParseLenPSEDIR( &g_ibuf[2] ) == FALSE ) // PATCH: 2006-10-05
           {                                             //
           result = apkFileInvalidated;                  //
           goto CCL_SELECT_PSE;                          //
           }                                             //

         rec_len = apk_GetBERLEN( &g_ibuf[3], &cnt ); // length of template 70 record
     //  rec_len = g_ibuf[3]; // length of template 70 record

         // get first entry from record
         ptrdir = apk_FindTag( 0x61, 0x00, &g_ibuf[4] );  // Tag=61?

CCL_CHK_DIR_ENTRY:

         bak_ptrdir = ptrdir;       // pointer to L-field of Tag=61

         if( ptrdir != 0 )
           {
//          dir_len = *ptrdir++; // length of template 61 record
           dir_len = apk_GetBERLEN( ptrdir, &cnt ); // length of template 61 record
           ptrdir += cnt;

           // check data structure
           if( apk_ParseTLV( (UCHAR *)&dir_len, ptrdir, 0 ) == FALSE )
             {
           //return( emvFailed );
             result = apkIncorrect; // EMV2000
             goto CCL_SELECT_PSE;   //
             }
           }
         else // no dir entry
           goto CCL_INC_RECNO;

//        ptrobj = apk_FindTag( 0x4f, 0x00, &g_ibuf[4] );  // Tag=4F?
         ptrobj = apk_FindTag( 0x4f, 0x00, bak_ptrdir-1 );  // Tag=4F?

         if( ptrobj != 0 )                // ADF dir entry found
           {
           // check mandatory data object: Application Label
           if( apk_FindTag( 0x50, 0x00, bak_ptrdir-1 ) == 0 )
             {
           //return( emvFailed );
             result = apkIncorrect; // EMV2000
             goto CCL_SELECT_PSE;   //
             }

           result = apk_MatchingAPP( ptrobj );
           if( (result == apkExactMatch) || (result == apkPartialMatch) )
             apk_AddCandidateList( --bak_ptrdir );

CCL_CHK_NEXT_DIR_ENTRY:

           // (1) AID not matched or
           // (2) is there another dir entry in this record?
           if( rec_len > (dir_len + 2) )
             {
             rec_len -= (dir_len + 2); // decr the previous dir length

             // get next entry Tag=61
             ptrdir += dir_len;
             ptrdir = apk_FindTag( 0x61, 0x00, ptrdir );
       //    dir_len = *ptrdir; // get new dir length
             dir_len = apk_GetBERLEN( ptrdir, &cnt );

             goto CCL_CHK_DIR_ENTRY;
             }
           else // there is no more dir entry in this record
             {
CCL_INC_RECNO:
             rec_num++; // incr record number for next read by 1
             goto CCL_READ_DIR_REC;
             }
           }
         else // non-ADF dir (maybe a DDF, ie, a sub-dir)
           {
//          ptrobj = apk_FindTag( 0x9d, 0x00, &g_ibuf[4] );  // Tag=9D?
           ptrobj = apk_FindTag( 0x9d, 0x00, bak_ptrdir-1 );  // Tag=9D?

           if( ptrobj != 0 )
             {
             // interrupt current dir & place resumption info on stack
             // including: g_ibuf[256], sfi[1], rec_num[1], rec_len[1], dir_len[1], ptrdir[2+(2)]
             g_ibuf[256+0] = sfi;
             g_ibuf[256+1] = rec_num;
             g_ibuf[256+2] = rec_len;
             g_ibuf[256+3] = dir_len;

#ifdef	PLATFORM_16BIT	// 16-bit addressing
             g_ibuf[256+4] = (UINT)bak_ptrdir & 0x00ff;
             g_ibuf[256+5] = ((UINT)bak_ptrdir & 0xff00) >> 8;
#endif

#ifndef	PLATFORM_16BIT	// 32-bit addressing
	      g_ibuf[256+4] = (ULONG) bak_ptrdir & 0x000000FF;
	      g_ibuf[256+5] = ((ULONG)bak_ptrdir & 0x0000FF00) >> 8;
	      g_ibuf[256+6] = ((ULONG)bak_ptrdir & 0x00FF0000) >> 16;
	      g_ibuf[256+7] = ((ULONG)bak_ptrdir & 0xFF000000) >> 24;
#endif

             if( apk_PushRamData( stktop, MAX_STACK_QUE, 256+6+2, g_ibuf ) == TRUE )
               {
               // get DDFNAME from entry
               // SELECT new DDF
               result = apk_SelectDDF( g_ibuf, ptrobj );
               if( result == apkOK )
                 {
                 fSelectDDF = 1; // PATCH: 2006-10-02
                 goto CCL_GET_SFI;
                 }

               if( result == apkFciMissing ) // PATCH: 2006-10-05
                 goto CCL_SELECT_PSE;

               if( result == apkIncorrect ) // EMV2000
                 goto CCL_SELECT_PSE;

               if( result == apkNotReady ) // 2CB.022.00
                 {
                 result = apkFileInvalidated; // switch to "list of AIDs"
                 goto CCL_SELECT_PSE;
                 }

               if( (result == apkFuncNotSupported) || (result == apkFileInvalidated) ) // PATCH: 2006-10-09
                 {
                 result = apkFileInvalidated;
                 goto CCL_SELECT_PSE;
                 }

           //  if( (result == apkFileNotFound) || (result == apkFuncNotSupported) )
               if( result == apkFileNotFound )
                 goto CCL_REC_NOT_FOUND;
               else
                 return( emvFailed );
               }
             else
            // goto CCL_CHK_NEXT_DIR_ENTRY;
               return( emvFailed ); // out of stack memory
             }
           else // neither ADF nor DDF dir entry
             {
          // return( emvFailed ); // out of spec

             result = apkIncorrect; // EMV2000
             goto CCL_SELECT_PSE;   //
             }
           }
         }
       }
     else // dir record not found
       {
CCL_REC_NOT_FOUND:

       // is there an entry on the dir stack?
       if( apk_PopRamData( stktop, 256+6+2, g_ibuf ) == TRUE )
         {
         // get prev directory from stack
         // resume processing previous directory
         sfi = g_ibuf[256+0];
         rec_num = g_ibuf[256+1];
         rec_len = g_ibuf[256+2];
         dir_len = g_ibuf[256+3];

#ifdef	PLATFORM_16BIT	// 16-bit addressing
         ptrdir = (UCHAR *)(g_ibuf[256+5]*256 + g_ibuf[256+4]);
#endif

#ifndef	PLATFORM_16BIT	// 32-bit addressing
	  ptrdir = (UCHAR *)(g_ibuf[256+4] + g_ibuf[256+5]*0x100L + g_ibuf[256+6]*0x10000L + g_ibuf[256+7]*0x1000000L);
#endif

         ptrdir++;

         goto CCL_CHK_NEXT_DIR_ENTRY;
         }
       else // no que data on stack
         {
         // PATCH: 2005/02/17, EMV2000: Using the PSE - Step 6
         // If no directory entries that match applications supported by
         // the terminal, the terminal shall use the list of AIDs method.

         if( apk_CheckCandidateList() == apkFailed )
           {
           if( CCL_SelectFromTermList() != emvOK )
             return( emvFailed );
           }

CCL_FINAL:

//    apk_ReadRamDataTERM( ADDR_CANDIDATE_01, 16, g_ibuf );
//    TL_DumpHexData(0,0,16, g_ibuf);
//    apk_ReadRamDataTERM( ADDR_CANDIDATE_02, 16, g_ibuf );
//    TL_DumpHexData(0,0,16, g_ibuf);
//    apk_ReadRamDataTERM( ADDR_CANDIDATE_03, 16, g_ibuf );
//    TL_DumpHexData(0,0,16, g_ibuf);
//    apk_ReadRamDataTERM( ADDR_CANDIDATE_04, 16, g_ibuf );
//    TL_DumpHexData(0,0,16, g_ibuf);

         if( apk_ArrangeCandidateList() == apkOK )
           return( emvOK ); // complete, EMV 96 SPEC
         else
       //  return( emvFailed );
           return( emvNotReady ); // PATCH: 2003-10-07, VSDC forced to FallBack

//    apk_ReadRamDataTERM( ADDR_CANDIDATE_NAME_01, CANDIDATE_NAME_LEN, g_ibuf );
   TL_DumpHexData(0,0,CANDIDATE_NAME_LEN, g_ibuf);
   apk_ReadRamDataTERM( ADDR_CANDIDATE_NAME_02, CANDIDATE_NAME_LEN, g_ibuf );
   TL_DumpHexData(0,0,CANDIDATE_NAME_LEN, g_ibuf);
   for(;;);



         // --- EMV 2000 SPEC ---
         // are there any entries on the candidate list?
    //   if( apk_CheckCandidateList() == apkOK )
    //     return( emvOK ); // candidate list is complete.
    //   else
    //     return( CCL_SelectFromTermList() );
         }
       }
}

// ---------------------------------------------------------------------------
// FUNCTION: Create a list of ICC applications that are supported by the term.
// INPUT   : none.
// OUTPUT  : none.
// REF     : none.
// RETURN  : emvOK
//           emvFailed
//           emvNotReady - no mutually supported application.
// ---------------------------------------------------------------------------
UCHAR api_emv_CreateCandidateList_PBOC( void )
{
UCHAR result;
UCHAR *ptrobj;
UCHAR *ptrdir;
UCHAR *bak_ptrdir;
UCHAR sfi;          // short file id
UCHAR rec_num;      // record number
UINT  rec_len;
UINT  dir_len;
UINT  iLen;
UCHAR stktop[1];
UINT  temp;
UCHAR cnt;
UCHAR reclen[2];
UCHAR fSelectDDF;
UCHAR ddfname[17];  // PATCH: PB0C2.0, 2006-02-13, L-V(16)
UCHAR fTag61;


      g_emv_ccl_SW1 = 0;	// 2015-06-04, for "Create Candidate List"
      g_emv_ccl_SW2 = 0;	//

      // clear "Candidate List"
      iLen = ADDR_CANDIDATE_END;
      iLen -= ADDR_CANDIDATE_START;
      apk_ClearRamDataICC( ADDR_CANDIDATE_START, iLen, 0 );

      iLen = ADDR_CANDIDATE_NAME_END;
      iLen -= ADDR_CANDIDATE_NAME_START;
      apk_ClearRamDataICC( ADDR_CANDIDATE_NAME_START, iLen, 0 );

      // PATCH: 2009-03-25, clear CTI before selection
      iLen = 0;
      apk_WriteRamDataICC( ADDR_ICC_ISU_CTI, 2, (UCHAR *)&iLen );
      g_pse = TRUE;

      // select PSE
      result = apk_SelectPSE( g_ibuf );

      // PATCH: PBOC2.0, 2006-02-13
      ddfname[0] = 14;
      ddfname[1] =  '1';
      ddfname[2] =  'P';
      ddfname[3] =  'A';
      ddfname[4] =  'Y';
      ddfname[5] =  '.';
      ddfname[6] =  'S';
      ddfname[7] =  'Y';
      ddfname[8] =  'S';
      ddfname[9] =  '.';
      ddfname[10] = 'D';
      ddfname[11] = 'D';
      ddfname[12] = 'F';
      ddfname[13] = '0';
      ddfname[14] = '1';
      fTag61 = FALSE;
      g_fallback = FALSE;

CCL_SELECT_PSE:

      switch( result )
            {
            case apkOK:

                 goto CCL_PSE_FOUND;

            case apkUnknown:
                                                     // PATCH: 2005/01/21
                                                     // MCHIP LITE test card returns "67 00"
                 if( ( g_ibuf[2] & 0xf0 ) == 0x60 )  // SW1 = 6x ?
                   {
                   if( CCL_SelectFromTermList_PBOC() == emvOK )
                     goto  CCL_FINAL;
                   else
                     return( emvFailed );
                   }
                 else
                   return( emvFailed );

            case apkFciMissing:       // 9000, fci missing
            case apkFileNotFound:     // 6A82, file not found
            case apkFileInvalidated:  // 6283, application blocked
            case apkIncorrect:        // EMV2000

                 if( CCL_SelectFromTermList_PBOC() == emvOK )
                   goto  CCL_FINAL;
                 else
                   return( emvFailed );

            case apkFuncNotSupported: // 6A81, card blocked
                 return( emvFailed );

            default: // failed
                 return( emvFailed );
            }

CCL_PSE_FOUND:

      // Empty Candidate List
      apk_EmptyCandidateList();
      *stktop = 0;
      fSelectDDF = 0;

CCL_GET_SFI:

      // Get SFI for Dir from FCI
      iLen = g_ibuf[1]*256 + g_ibuf[0];
      iLen -= 2; // total length, ignore SW1 SW2

      if( ( g_ibuf[2] != 0x6F ) || ( g_ibuf[3] > iLen ) ) // check FCI Template & Length
        return( emvFailed );

      ptrobj = apk_FindTag( 0x88, 0x00, &g_ibuf[2] );  // Tag=88, SFI of the dir EF
      apk_GetBERLEN( ptrobj, &cnt );
      sfi = *(ptrobj + cnt);

      rec_num = 1; // set record number to 1 for next read

CCL_READ_DIR_REC:

      g_ibuf[0] = 0; // clear output length
      g_ibuf[1] = 0; //
      result = apk_ReadRecord( sfi, rec_num, g_ibuf );  // read dir record

      if( (result == apkRecNotFound) && (rec_num == 1) )
        {
        if( fSelectDDF == 1 ) // PATCH: 2006-10-02
          {
          fSelectDDF = 0;
          goto CCL_REC_NOT_FOUND;
          }

        result = apkFileInvalidated; // switch to "list of AIDs"
        goto CCL_SELECT_PSE;
        }

      if( result == apkFailed )
        {
        // PATCH: 2005/03/21, EMV2000
        if( (g_ibuf[0]+g_ibuf[1]*256) >= 2 ) // chk output length
          {
          result = apkFileInvalidated; // switch to "list of AIDs"
          goto CCL_SELECT_PSE;
          }
        else
          return( emvFailed );
        }

      if( result == apkOK )  // record found?
        {
        // is there an entry in this record?
        // 2L [70-L-(61-L-DIR1)...(61-L-DIRn)]
        if(g_ibuf[2] != 0x70)
//        return( emvFailed );
          {
          result = apkFileInvalidated; // PATCH: 2006-10-05
          goto CCL_SELECT_PSE;
          }
        else
          {

          if( apk_ParseLenPSEDIR( &g_ibuf[2] ) == FALSE ) // PATCH: 2006-10-05
            {                                             //
            result = apkFileInvalidated;                  //
            goto CCL_SELECT_PSE;                          //
            }                                             //

          rec_len = apk_GetBERLEN( &g_ibuf[3], &cnt ); // length of template 70 record
      //  rec_len = g_ibuf[3]; // length of template 70 record

          // get first entry from record
          ptrdir = apk_FindTag( 0x61, 0x00, &g_ibuf[3+cnt] );  // Tag=61? //EMV 42a Charles 2009-03-12 2CB.005.00 case 2,3

CCL_CHK_DIR_ENTRY:

          bak_ptrdir = ptrdir;       // pointer to L-field of Tag=61

          if( ptrdir != 0 )
            {
            fTag61 = TRUE; // PATCH: PBOC2.0, 2006-02-16, 2CL0290501a

//          dir_len = *ptrdir++; // length of template 61 record
            dir_len = apk_GetBERLEN( ptrdir, &cnt ); // length of template 61 record
            ptrdir += cnt;

	    if( dir_len >= rec_len )  // PATCH: PBOC2.0, 2006-02-16, 2CL0290502
              {
              result = apkIncorrect;
              goto CCL_SELECT_PSE;
              }

            // check data structure
            if( apk_ParseTLV( (UCHAR *)&dir_len, ptrdir, 0 ) == FALSE )
              {
            //return( emvFailed );
              result = apkIncorrect; // EMV2000
              goto CCL_SELECT_PSE;   //
              }
            }
          else // no dir entry
            {
            if( fTag61 == FALSE ) // PATCH: 2CL0290501a
              {
              result = apkFileInvalidated; // switch to "list of AIDs"
              goto CCL_SELECT_PSE;
              }

            goto CCL_INC_RECNO;
            }

//        ptrobj = apk_FindTag( 0x4f, 0x00, &g_ibuf[4] );  // Tag=4F?
          ptrobj = apk_FindTag( 0x4f, 0x00, bak_ptrdir-1 );  // Tag=4F?

          if( ptrobj != 0 )                // ADF dir entry found
            {
            // check mandatory data object: Application Label
            if( apk_FindTag( 0x50, 0x00, bak_ptrdir-1 ) == 0 )
              {
            //return( emvFailed );
              result = apkIncorrect; // EMV2000
              goto CCL_SELECT_PSE;   //
              }

            result = apk_MatchingAPP( ptrobj );
            if( (result == apkExactMatch) || (result == apkPartialMatch) )
              apk_AddCandidateList( --bak_ptrdir );

CCL_CHK_NEXT_DIR_ENTRY:

            // (1) AID not matched or
            // (2) is there another dir entry in this record?
            if( rec_len > (dir_len + 2) )
              {
              rec_len -= (dir_len + 2); // decr the previous dir length

              // get next entry Tag=61
              ptrdir += dir_len;
              ptrdir = apk_FindTag( 0x61, 0x00, ptrdir );
        //    dir_len = *ptrdir; // get new dir length
              dir_len = apk_GetBERLEN( ptrdir, &cnt );

              goto CCL_CHK_DIR_ENTRY;
              }
            else // there is no more dir entry in this record
              {
CCL_INC_RECNO:
              rec_num++; // incr record number for next read by 1
              goto CCL_READ_DIR_REC;
              }
            }
          else // non-ADF dir (maybe a DDF, ie, a sub-dir)
            {
//          ptrobj = apk_FindTag( 0x9d, 0x00, &g_ibuf[4] );  // Tag=9D?
            ptrobj = apk_FindTag( 0x9d, 0x00, bak_ptrdir-1 );  // Tag=9D?

            if( ptrobj != 0 )
              {
              // interrupt current dir & place resumption info on stack
              // including: g_ibuf[256], sfi[1], rec_num[1], rec_len[1], dir_len[1], ptrdir[2+(2)]
              g_ibuf[256+0] = sfi;
              g_ibuf[256+1] = rec_num;
              g_ibuf[256+2] = rec_len;
              g_ibuf[256+3] = dir_len;

#ifdef	PLATFORM_16BIT	// 16-bit addressing
              g_ibuf[256+4] = (UINT)bak_ptrdir & 0x00ff;
              g_ibuf[256+5] = ((UINT)bak_ptrdir & 0xff00) >> 8;

              memmove( &g_ibuf[256+6], ddfname, sizeof(ddfname) ); // PATCH: PBOC2.0, 2006-02-13, add parents ddfname to stack
#endif

#ifndef	PLATFORM_16BIT	// 32-bit addressing
	      g_ibuf[256+4] = (ULONG) bak_ptrdir & 0x000000FF;
	      g_ibuf[256+5] = ((ULONG)bak_ptrdir & 0x0000FF00) >> 8;
	      g_ibuf[256+6] = ((ULONG)bak_ptrdir & 0x00FF0000) >> 16;
	      g_ibuf[256+7] = ((ULONG)bak_ptrdir & 0xFF000000) >> 24;

	      memmove( &g_ibuf[256+8], ddfname, sizeof(ddfname) ); // PATCH: PBOC2.0, 2006-02-13, add parents ddfname to stack
#endif

#ifdef	PLATFORM_16BIT	// 16-bit addressing
              if( apk_PushRamData( stktop, MAX_STACK_QUE, 256+6+sizeof(ddfname), g_ibuf ) == TRUE )
#endif

#ifndef	PLATFORM_16BIT	// 32-bit addressing
              if( apk_PushRamData( stktop, MAX_STACK_QUE, 256+8+sizeof(ddfname), g_ibuf ) == TRUE )
#endif
                {
                memmove( ddfname, ptrobj, sizeof(ddfname) ); // PATCH: PBOC2.0, 2006-02-13, update ddfname

                // get DDFNAME from entry
                // SELECT new DDF
                result = apk_SelectDDF( g_ibuf, ptrobj );
                if( result == apkOK )
                  {
                  fTag61 = FALSE; // PATCH: PBOC2.0, 2006-02-16, 2CL0290502a
                  fSelectDDF = 1; // PATCH: 2006-10-02
                  goto CCL_GET_SFI;
                  }

                if( result == apkFciMissing ) // PATCH: 2006-10-05
                  goto CCL_SELECT_PSE;

                if( result == apkIncorrect ) // EMV2000
                  goto CCL_SELECT_PSE;

                if( result == apkNotReady ) // 2CB.022.00
                  {
                  result = apkFileInvalidated; // switch to "list of AIDs"
                  goto CCL_SELECT_PSE;
                  }

                if( (result == apkFuncNotSupported) || (result == apkFileInvalidated) ) // PATCH: 2006-10-09
                  {
                  result = apkFileInvalidated;
                  goto CCL_SELECT_PSE;
                  }

                if( result == apkFileNotFound )
//                goto CCL_REC_NOT_FOUND;	// EMV rule
						// PATCH: PBOC2.0, 2006-02-15, 2CB.022
		  goto CCL_SELECT_PSE;          // DF file not found, switch to "list of AIDs"
                else
                  return( emvFailed );
                }
              else
             // goto CCL_CHK_NEXT_DIR_ENTRY;
                return( emvFailed ); // out of stack memory
              }
            else // neither ADF nor DDF dir entry
              {
           // return( emvFailed ); // out of spec

              result = apkIncorrect; // EMV2000
              goto CCL_SELECT_PSE;   //
              }
            }
          }
        }
      else // dir record not found
        {
CCL_REC_NOT_FOUND:

        // is there an entry on the dir stack?
#ifdef	PLATFORM_16BIT	// 16-bit addressing
        if( apk_PopRamData( stktop, 256+6, g_ibuf ) == TRUE )
#endif

#ifndef	PLATFORM_16BIT	// 32-bit addressing
        if( apk_PopRamData( stktop, 256+8, g_ibuf ) == TRUE )
#endif
          {
          // get prev directory from stack
          // resume processing previous directory
          sfi = g_ibuf[256+0];
          rec_num = g_ibuf[256+1];
          rec_len = g_ibuf[256+2];
          dir_len = g_ibuf[256+3];

#ifdef	PLATFORM_16BIT	// 16-bit addressing
          ptrdir = (UCHAR *)(g_ibuf[256+5]*256 + g_ibuf[256+4]);
          memmove( ddfname, &g_ibuf[256+6], sizeof(ddfname) ); // PATCH: PBOC2.0, 2006-02-13, retrieve parents ddfname (L-V)
#endif

#ifndef	PLATFORM_16BIT	// 32-bit addressing
	  ptrdir = (UCHAR *)(g_ibuf[256+4] + g_ibuf[256+5]*0x100L + g_ibuf[256+6]*0x10000L + g_ibuf[256+7]*0x1000000L);
	  memmove( ddfname, &g_ibuf[256+8], sizeof(ddfname) ); // PATCH: PBOC2.0, 2006-02-13, retrieve parents ddfname (L-V)
#endif

          ptrdir++;

//        if( rec_len <= (dir_len + 2) )          // PBOC, 2009-03-10 removed this condition for 2CB.017.01
            apk_SelectDDF( g_temp, ddfname );     // re-select parents ddfname

          goto CCL_CHK_NEXT_DIR_ENTRY;
          }
        else // no que data on stack
          {
          // PATCH: 2005/02/17, EMV2000: Using the PSE - Step 6
          // If no directory entries that match applications supported by
          // the terminal, the terminal shall use the list of AIDs method.

          if( apk_CheckCandidateList() == apkFailed )
            {
            if( CCL_SelectFromTermList_PBOC() != emvOK )
              return( emvFailed );
            }

CCL_FINAL:

//    apk_ReadRamDataTERM( ADDR_CANDIDATE_01, 16, g_ibuf );
//    TL_DumpHexData(0,0,16, g_ibuf);
//    apk_ReadRamDataTERM( ADDR_CANDIDATE_02, 16, g_ibuf );
//    TL_DumpHexData(0,0,16, g_ibuf);
//    apk_ReadRamDataTERM( ADDR_CANDIDATE_03, 16, g_ibuf );
//    TL_DumpHexData(0,0,16, g_ibuf);
//    apk_ReadRamDataTERM( ADDR_CANDIDATE_04, 16, g_ibuf );
//    TL_DumpHexData(0,0,16, g_ibuf);

          if( apk_ArrangeCandidateList() == apkOK )
            return( emvOK ); // complete, EMV 96 SPEC
          else
        //  return( emvFailed );
            return( emvNotReady ); // PATCH: 2003-10-07, VSDC forced to FallBack

//    apk_ReadRamDataTERM( ADDR_CANDIDATE_NAME_01, CANDIDATE_NAME_LEN, g_ibuf );
//    TL_DumpHexData(0,0,CANDIDATE_NAME_LEN, g_ibuf);
//    apk_ReadRamDataTERM( ADDR_CANDIDATE_NAME_02, CANDIDATE_NAME_LEN, g_ibuf );
//    TL_DumpHexData(0,0,CANDIDATE_NAME_LEN, g_ibuf);
//    for(;;);



          // --- EMV 2000 SPEC ---
          // are there any entries on the candidate list?
     //   if( apk_CheckCandidateList() == apkOK )
     //     return( emvOK ); // candidate list is complete.
     //   else
     //     return( CCL_SelectFromTermList_PBOC() );
          }
        }
}

// ---------------------------------------------------------------------------
// FUNCTION: Get the sorted candidate name list for presentation.
// INPUT   : none.
// OUTPUT  : list - a linear fixed length 2-D array (16x18 bytes)
//                  format: LINK[1] LEN1[1] NAME1[16]
//                          LINK[1] LEN2[1] NAME2[16]...
//                          LINK[1] LEN16[1] NAME16[16]
//                          (1) LINK[1]: link index to the file. (used for final selection)
//                          (2) LEN[1] : acutual length of NAME
//                                       0 = the bottom of list.
//                          (3) NAME[] : contains application preferred or label name.
// REF     : SRAM: CANDIDATE_NAME
// RETURN  : number of name lists. (0=no candidate name list)
// NOTE    : call this function each time to refresh the new selection list.
// ---------------------------------------------------------------------------
UCHAR api_emv_GetCandidateList( UCHAR *list )
{
UCHAR i;
UCHAR len;
UCHAR cnt;
UINT  addr;
UCHAR name[CANDIDATE_NAME_LEN];

      cnt = 0;
      for( i=0; i<MAX_CANDIDATE_NAME_CNT; i++ )
         {
         addr = ADDR_CANDIDATE_NAME_01 + CANDIDATE_NAME_LEN*i;
         apk_ReadRamDataTERM( addr, CANDIDATE_NAME_LEN, name );

         len = *(name+1);

         if( len == 0 ) // end of list?
           return( cnt );
         else
           {
           if( len != 255 ) // removed?
             {
             // move it to output list
             memmove( list, name, CANDIDATE_NAME_LEN );

             cnt += 1;
             list += CANDIDATE_NAME_LEN;
             }
           }
         }
      return( cnt );
}

// ---------------------------------------------------------------------------
// FUNCTION: Select candidate by using the list of application in the terminal,
//           because there is no PSE in the card.
// INPUT   : none.
// OUTPUT  : none.
// REF     : none.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR CCL_SelectFromTermList( void )
{
UCHAR aid[TERM_AID_LEN];
UCHAR result;
UCHAR *ptrobj;
UCHAR *ptrtmp;
UCHAR rec_len;
UINT  iLen;
UCHAR aid_index;
UCHAR asi;
UCHAR cnt;

     // Empty Candidate List
     apk_EmptyCandidateList();

     aid_index = 0; // from the 1'st AID

STL_GET_NEXT_AID:

     // get terminal AID
     if( apk_GetTermAID( aid_index, aid ) == apkFailed )
       return( emvOK ); // complete
     aid_index++;

     rec_len = aid[0];
     rec_len--;         // the length of AID
     asi = aid[1];      // get app selection indicator
     aid[1] = rec_len;  // overwirte with length

     // select file using DFname = AID
     result = apk_SelectADF( g_ibuf, &aid[1], 0 ); // g_ibuf=FCI
     if(result == apkIncorrect)	//2008-dec-25 charles
   	  goto STL_GET_NEXT_AID;
     if( result == apkFailed )              // failed
       return( emvFailed );

//    if( (result == apkFuncNotSupported) || (result == apkUnknown) )  // 6A81, card blocked or others
     if( result == apkFuncNotSupported )  // 6A81, card blocked
       {
       if( aid_index == 1 )
         return( emvFailed ); // terminate if it is the 1'st AID
       else
         goto STL_GET_NEXT_AID;
       }

     if( result == apkUnknown )            // PATCH: 2005/01/21
       {                                   // MCHIP LITE test card returns "67 00"
//      if( ( g_ibuf[2] & 0xf0 ) == 0x60 )  // SW1 = 6x ?
//        goto STL_GET_NEXT_AID;
//      else
//        return( emvFailed );              // PATCH: 2005/01/27

       goto STL_GET_NEXT_AID; // PATCH: 2006-10-09, except 9000, 6A81, 6283
       }

STL_FILE_FOUND:

     if( result == apkFileNotFound )        // 6A82, file not found
       // is there another AID in list ?
       goto STL_GET_NEXT_AID;

     // DFname in FCI = AID ?
     ptrobj = apk_FindTag( 0x6f, 0x00, &g_ibuf[2] ); // Tag=6F (FCI template)?
     ptrtmp = ptrobj; // L-V

     if( ptrobj == 0 )
       goto STL_GET_NEXT_AID;
     else
       {
       ptrobj = apk_FindTag( 0x84, 0x00, &g_ibuf[2] ); // Tag=84 (DF Name)?

       if( ptrobj == 0 )
         goto STL_GET_NEXT_AID;

       iLen = apk_GetBERLEN( ptrobj, &cnt );
       ptrobj += cnt; // ptr to "DFName"

       // case 1: length of DFName < AID
       if( iLen < rec_len )
         goto STL_GET_NEXT_AID;

       if( TL_memcmp( &aid[2], ptrobj, rec_len ) != 0 ) // same name?
         goto STL_GET_NEXT_AID;

       // case 2: length of DFNAME = AID (exactly matched)
       if( iLen == rec_len )
         {
         if( result != apkFileInvalidated ) // 6283, application blocked
           // add FCI information to candidate list
           apk_AddCandidateList( --ptrtmp );

         goto STL_GET_NEXT_AID;
         }

       // case 3: length of DFNAME > AID (partially matched)
       if( asi == 1 ) // partial name matching allowed?
         {
         if( result != apkFileInvalidated ) // 6283, application blocked
           // add FCI information to candidate list
           apk_AddCandidateList( --ptrtmp );

         // select NEXT with same DFname = AID
         result = apk_SelectADF( g_ibuf, &aid[1], 1 ); // g_ibuf=FCI
         if( (result == apkOK) || (result == apkFileInvalidated) )
           goto STL_FILE_FOUND;

         // PATCH: 2005/02/18, EMV2000: Using a List of AIDs - Step 7
         if( result == apkUnknown )
           {
           if( (g_ibuf[2] == 0x62) || (g_ibuf[2] == 0x63) )  // SW1 SW2 = 62xx or 63xx?
             goto STL_FILE_FOUND;
           else
             goto STL_GET_NEXT_AID;
           }
         else
           goto STL_GET_NEXT_AID;
         }
       else // only exact match required
         goto STL_GET_NEXT_AID;
       }
}

// ---------------------------------------------------------------------------
// FUNCTION: Select candidate by using the list of application in the terminal,
//           because there is no PSE in the card.
// INPUT   : none.
// OUTPUT  : none.
// REF     : none.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR CCL_SelectFromTermList_PBOC( void )
{
UCHAR aid[TERM_AID_LEN];
UCHAR result;
UCHAR *ptrobj;
UCHAR *ptrtmp;
UCHAR rec_len;
UINT  iLen;
UCHAR aid_index;
UCHAR asi;
UCHAR cnt;

      // PATCH: 2009-03-25, clear CTI before selection
      iLen = 0;
      apk_WriteRamDataICC( ADDR_ICC_ISU_CTI, 2, (UCHAR *)&iLen );
      g_pse = FALSE;

      // Empty Candidate List
      apk_EmptyCandidateList();

      aid_index = 0; // from the 1'st AID

      g_fallback = FALSE;

STL_GET_NEXT_AID:

      // get terminal AID
      if( apk_GetTermAID( aid_index, aid ) == apkFailed )
        return( emvOK ); // complete
      aid_index++;

      rec_len = aid[0];
      rec_len--;         // the length of AID
      asi = aid[1];      // get app selection indicator
      aid[1] = rec_len;  // overwirte with length

      // select file using DFname = AID
      result = apk_SelectADF( g_ibuf, &aid[1], 0 ); // g_ibuf=FCI
      if(result == apkIncorrect)	//2008-dec-25 charles
    	  goto STL_GET_NEXT_AID;
      if( result == apkFailed )              // failed
        return( emvFailed );

//    if( (result == apkFuncNotSupported) || (result == apkUnknown) )  // 6A81, card blocked or others
      if( result == apkFuncNotSupported )  // 6A81, card blocked
        {
        if( aid_index == 1 )
          return( emvFailed ); // terminate if it is the 1'st AID
        else
          goto STL_GET_NEXT_AID;
        }

      if( result == apkUnknown )            // PATCH: 2005/01/21
        {                                   // MCHIP LITE test card returns "67 00"
        // PATCH: 2006-02-21, PBOC2.0, if 6D00 and none of AID is valid, then set flag and fall back
        if( (g_ibuf[2] == 0x6D) && (g_ibuf[3] == 0x00) )
          g_fallback = TRUE;
        else
          g_fallback = FALSE;

        goto STL_GET_NEXT_AID; // PATCH: 2006-10-09, except 9000, 6A81, 6283
        }

STL_FILE_FOUND:

      if( result == apkFileNotFound )        // 6A82, file not found
        // is there another AID in list ?
        goto STL_GET_NEXT_AID;

      // DFname in FCI = AID ?
      ptrobj = apk_FindTag( 0x6f, 0x00, &g_ibuf[2] ); // Tag=6F (FCI template)?
      ptrtmp = ptrobj; // L-V

      if( ptrobj == 0 )
        goto STL_GET_NEXT_AID;
      else
        {
        ptrobj = apk_FindTag( 0x84, 0x00, &g_ibuf[2] ); // Tag=84 (DF Name)?

        if( ptrobj == 0 )
          goto STL_GET_NEXT_AID;

        iLen = apk_GetBERLEN( ptrobj, &cnt );
        ptrobj += cnt; // ptr to "DFName"

        // case 1: length of DFName < AID
        if( iLen < rec_len )
          goto STL_GET_NEXT_AID;

        if( TL_memcmp( &aid[2], ptrobj, rec_len ) != 0 ) // same name?
          goto STL_GET_NEXT_AID;

        // case 2: length of DFNAME = AID (exactly matched)
        if( iLen == rec_len )
          {
          if( result != apkFileInvalidated ) // 6283, application blocked
            // add FCI information to candidate list
            apk_AddCandidateList( --ptrtmp );
          else
            {
            g_emv_ccl_SW1 = 0x62;	// 2015-06-04, to record "6283" for APP reference
            g_emv_ccl_SW2 = 0x83;	//
            }

          goto STL_GET_NEXT_AID;
          }

        // case 3: length of DFNAME > AID (partially matched)
        if( asi == 1 ) // partial name matching allowed?
          {
          if( result != apkFileInvalidated ) // 6283, application blocked
            // add FCI information to candidate list
            apk_AddCandidateList( --ptrtmp );

STL_SELECT_NEXT_ADF:

          // select NEXT with same DFname = AID
          result = apk_SelectADF( g_ibuf, &aid[1], 1 ); // g_ibuf=FCI
          if( (result == apkOK) || (result == apkFileInvalidated) )
            goto STL_FILE_FOUND;

          // PATCH: 2005/02/18, EMV2000: Using a List of AIDs - Step 7
          if( result == apkUnknown )
            {
            if( (g_ibuf[2] == 0x62) || (g_ibuf[2] == 0x63) )  // SW1 SW2 = 62xx or 63xx?
              goto STL_FILE_FOUND;
            else
              goto STL_GET_NEXT_AID;
            }
          else
            goto STL_GET_NEXT_AID;
          }
        else // only exact match required
          {
          // PATCH: PBOC2.0, 2006-02-12, 2CB.031.00 -- to select NEXT until 6A82
//        goto STL_SELECT_NEXT_ADF;

	  // PATCH: BCTC EMV L2, 2009-03-07 -- 2CB.031.00
          goto STL_GET_NEXT_AID;
	  }
        }
}
