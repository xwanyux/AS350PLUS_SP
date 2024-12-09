//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APK_MAPP.C                                                 **
//**  MODULE   : apk_MatchingAPP()                                          **
//**             apk_GetTermAID()                                           **
//**             apk_EmptyCandidateList()                                   **
//**             apk_AddCandidateList()                                     **
//**             apk_GetCandidateList()                                     **
//**             apk_CheckCandidateList()                                   **
//**             apk_ArrangeCandidateList()                                 **
//**             apk_RemoveCandidateList()                                  **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2002/12/08                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2002-2007 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"
//#include "EMVDC.h"
#include "GDATAEX.h"
#include "EMVAPI.h"
#include "EMVAPK.h"
//#include "APDU.H>
#include "TOOLS.h"

// ---------------------------------------------------------------------------
// FUNCTION: To match the applications supported by both terminal & ICC.
// INPUT   : aid - L-V (V=RID+PIX), current AID got.
// OUTPUT  : none.
// RETURN  : apkExactMatch   - exactly matched.
//           apkPartialMatch - partially matched.
//           apkNoMatched    - not matched.
// ---------------------------------------------------------------------------
UCHAR apk_MatchingAPP( UCHAR *aid )
{
UINT  addr;
UCHAR i;
UCHAR len;
UCHAR buf[TERM_AID_LEN];

      // compare terminal AIDs with the given AID
      for( i=0; i<MAX_AID_CNT; i++ )
         {
         // read one terminal AID entry
         addr = ADDR_TERM_AID_01 + i*TERM_AID_LEN;
         apk_ReadRamDataTERM( addr, TERM_AID_LEN, buf );
         len = buf[0] - 1;

         // exact matching criterion
         if( aid[0] == len )
           {
           if( TL_memcmp( &aid[1], &buf[2], len ) == 0 )
             return( apkExactMatch );
           }
         else // partial matching criterion
           {
//         if( ( aid[0] > len ) && ( buf[1] == 1 ) )
           if( ( aid[0] > len ) && (( buf[1] == 1 ) || ( buf[1] == 0x80 )) )	// 2020-04-09, ASI=0x80: support PSE partial, non-PSE exact
             {
             if( TL_memcmp( &aid[1], &buf[2], len ) == 0 )
               return( apkPartialMatch );
             }
           }
         }
      return( apkNoMatch );
}

// ---------------------------------------------------------------------------
// FUNCTION: To match the applications supported by both terminal & ICC.
// INPUT   : aid  - L-V (V=RID+PIX), current AID got.
//           flag - partial match flag (1: partial match, 0: exact match)
// OUTPUT  : none.
// RETURN  : apkExactMatch   - exactly matched.
//           apkNoMatched    - not matched.
// ---------------------------------------------------------------------------
UCHAR apk_SetASI( UCHAR *aid, UCHAR flag )
{
UINT  addr;
UCHAR i;
UCHAR len;
UCHAR buf[TERM_AID_LEN];

      // compare terminal AIDs with the given AID
      for( i=0; i<MAX_AID_CNT; i++ )
         {
         // read one terminal AID entry
         addr = ADDR_TERM_AID_01 + i*TERM_AID_LEN;
         apk_ReadRamDataTERM( addr, TERM_AID_LEN, buf );
         len = buf[0] - 1;

         // exact matching criterion
         if( aid[0] == len )
           {
           if( TL_memcmp( &aid[1], &buf[2], len ) == 0 )
             {
             
             buf[1] = flag;	// set new ASI
             apk_WriteRamDataTERM( addr, TERM_AID_LEN, buf );
             
             return( apkExactMatch );
             }
           }
         }
         
      return( apkNoMatch );
}

// ---------------------------------------------------------------------------
// FUNCTION: To read the specified terminal AID from terminal current list.
// INPUT   : index - the target AID number. (0..MAX_AID_CNT-1)
// OUTPUT  : aid   - pointer to target AID.
//                   format: LEN[1] ASI[1] AID[16]
// RETURN  : apkOK     - found.
//           apkFailed - not found.
// ---------------------------------------------------------------------------
UCHAR apk_GetTermAID( UCHAR index, UCHAR *aid )
{
UINT  addr;
UCHAR i;
UCHAR len;
UCHAR buf[TERM_AID_LEN];

      if( index >= MAX_AID_CNT )
        return( apkFailed ); // out of range

      addr = ADDR_TERM_AID_01 + index*TERM_AID_LEN;
      apk_ReadRamDataTERM( addr, TERM_AID_LEN, aid );

      if( *aid == 0x00 )
        return( apkFailed );
      else
        return( apkOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: Erase application candidate list.
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_candidate_list_head
//           g_candidate_list_tail
// RETURN  : none.
// ---------------------------------------------------------------------------
void apk_EmptyCandidateList( void )
{
UCHAR buf[CANDIDATE_LEN];
UINT  i;
UINT  addr;

      for( i=0; i<CANDIDATE_LEN; i++ )
         buf[i] = 0;

      for( i=0; i<MAX_CANDIDATE_CNT; i++ )
         {
         addr = ADDR_CANDIDATE_01 + CANDIDATE_LEN*i;
         apk_WriteRamDataTERM( addr, CANDIDATE_LEN, buf );
         }

      g_candidate_list_head = 0; // reset index
      g_candidate_list_tail = 0;
}

// ---------------------------------------------------------------------------
// FUNCTION: Add the matched dir entry to candidate list.
// INPUT   : ptrdata - T-L-V, pointer to the FCI or DIR entry of ADF.
// OUTPUT  : none.
// REF     : g_candidate_list_tail
// RETURN  : apkOK
//           apkFailed (out of memory)
// ---------------------------------------------------------------------------
UCHAR apk_AddCandidateList( UCHAR *ptrdata )
{
UINT addr;

      if( g_candidate_list_tail >= MAX_CANDIDATE_CNT )
        return( apkFailed );

      addr = ADDR_CANDIDATE_01 + CANDIDATE_LEN*g_candidate_list_tail;
      apk_WriteRamDataTERM( addr, *(ptrdata+1)+2, ptrdata );

      g_candidate_list_tail++;

      return( apkOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: Read the candidate information by using the specified index.
// INPUT   : g_candidate_list_head
// OUTPUT  : ptrdata - T-L-V, pointer to the FCI or DIR entry of ADF
//                          61 LEN ADF_NAME (AID) ... or
//                          6F LEN DDF_NAME ...
// REF     : g_candidate_list_head
// RETURN  : apkOK     (found)
//           apkFailed (not found)
// ---------------------------------------------------------------------------
UCHAR apk_GetCandidateList( UCHAR *ptrdata )
{
UINT addr;

      if( g_candidate_list_head >= g_candidate_list_tail )
        return( apkFailed );

      addr = ADDR_CANDIDATE_01 + CANDIDATE_LEN*g_candidate_list_head;
      apk_ReadRamDataTERM( addr, CANDIDATE_LEN, ptrdata );

      g_candidate_list_head++;

      return( apkOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: Check if at least one candidate is available.
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_candidate_list_head
//           g_candidate_list_tail
// RETURN  : apkOK     (found)
//           apkFailed (not found)
// ---------------------------------------------------------------------------
UCHAR apk_CheckCandidateList( void )
{
UINT addr;

      if( g_candidate_list_head == g_candidate_list_tail )
        return( apkFailed );
      else
        return( apkOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: Arrange & generate the final candidate name list for selection by
//           (1) Application Priority Indicator (API, if present) or
//               in the order in which the APs were encountered in the card
//           (2) Application Preferred Name or
//               Application Label
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_candidate_list_head
//           g_candidate_list_tail
//           SRAM data for input : ADDR_CANDIDATE[16][256]
//           SRAM data for output: ADDR_CANDIDATE_NAME[16][18]
//           g_api[]
// RETURN  : apkOK
//           apkFailed (no mutually supported application)
// ---------------------------------------------------------------------------
UCHAR apk_ArrangeCandidateList( void )
{
UCHAR i;
UINT  addr;
UCHAR *ptrobj;
UCHAR *bak_ptrobj;
UCHAR rec_num;
UCHAR rec_link;
UCHAR data[CANDIDATE_NAME_LEN];
UCHAR api[MAX_CANDIDATE_NAME_CNT];   // application priority indicator
UCHAR atcap[5];                      // additional terminal capabilities
UCHAR icti;                          // issuer code table index
UCHAR cnt;
UINT  iLen;

      if( g_candidate_list_tail == 0 )
        return( apkFailed ); // no candidate here

      // clear output memory to: [00 00 20 20 ... 20]
      data[0] = 0;       // byte 0 = 0x00 (LINK)
      data[1] = 0;       // byte 1 = 0x00 (LEN)
      for( i=2; i<CANDIDATE_NAME_LEN; i++ )
         data[i] = 0x20; // byte 2-17 = 0x20 (NAME)

      for( i=0; i<MAX_CANDIDATE_NAME_CNT; i++ )
         {
         addr = ADDR_CANDIDATE_NAME_01 + CANDIDATE_NAME_LEN*i;
         apk_WriteRamDataTERM( addr, CANDIDATE_NAME_LEN, data ); // byte 0-17
         }

      // clear api[0..x]
      for( i=0; i<MAX_CANDIDATE_NAME_CNT; i++ )
         {
         api[i] = 0;
         g_api[i] = 0;
         }

      // read all candidate and save its API if available
      g_candidate_list_head = 0; // reset access index to the 1'st entry

      i=0;
      while(1)
           {
           if( apk_GetCandidateList( g_ibuf ) == apkOK )
             {
             ptrobj = apk_FindTag( 0x87, 0, g_ibuf ); // find API tag
             if( ptrobj != 0 ) // found
               {
               // api[n]
               // bit5-8: index to the original list
               // bit1-4: API value
               api[i] = *(ptrobj+1) & 0x0f; // set priority nibble (bit1-4)
               g_api[i] = *(ptrobj+1);
               }
             else
               {
               api[i] = 0x0f; // no api, set it to lowest priority
               g_api[i] = 0x8f;
               }

             api[i] |= (i << 4) & 0xf0; // set index nibble (bit5-8)
             i++;
             }
           else
             break;
           }

      // sort by lower nibble of api[] (1..15, 0)
      rec_num = g_candidate_list_tail;
      TL_isort( api, rec_num, 0x0f );

      // get "Additional Terminal Capabilities"
      apk_ReadRamDataTERM( ADDR_TERM_ADD_CAP, 5, atcap );

      // move candidate name or label to the output buffer
      // format: LINK[1] LEN[1] NAME[16]
      for( i=0; i< rec_num; i++ )
         {
         g_candidate_list_head = ( api[i] & 0xf0 ) >> 4; // src access index
         addr = ADDR_CANDIDATE_NAME_01 + CANDIDATE_NAME_LEN*i; // dst access address

         // store LINK[1]
         apk_WriteRamDataTERM( addr, 1, &g_candidate_list_head );

         // read the Candidate List record
         apk_GetCandidateList( g_ibuf ); // 61-L-[4F-L-(ADF Name)...] or
                                         // 6F-L-[84-L-(DF Name)... ]

         ptrobj = apk_FindTag( 0x9f, 0x12, g_ibuf );   // find "App Preferred Name"
//       ptrobj = 0; // not support prefered name because not support Issuer Code Table
         bak_ptrobj = ptrobj;

ACL_USE_APP_LABEL:

         if( ptrobj == 0 )
           {
           ptrobj = apk_FindTag( 0x50, 0x00, g_ibuf ); // or find "App Label"
           if( ptrobj == 0 )
             {
             // "App Label" not found, get "DF Name"
             ptrobj = apk_FindTag( 0x84, 0x00, g_ibuf );
             if( ptrobj == 0 )
               return( emvFailed );

             if( *ptrobj > 8 ) // only show the first 8 bytes (16 char)
               *ptrobj = 8;

             TL_hex2asc( *ptrobj, ptrobj+1, data ); // convert to ASCII
             ptrobj = data;
             }
           }
         else // PATCH: 2005/02/25, EMV2000 - supporting preferred name
           {
           ptrobj = apk_FindTag( 0x9f, 0x11, g_ibuf );   // find "Issuer Code Table Index"
           if( ptrobj != 0 )
             {
             apk_GetBERLEN( ptrobj, &cnt );
             icti = *(ptrobj + cnt);

             if( (icti == 1 ) && (atcap[4] == 1) ) // code table 1?
               ptrobj = bak_ptrobj; // using preferred name
             else // ICTI not supported
               {
               ptrobj = 0;
               goto ACL_USE_APP_LABEL;
               }
             }
           else // ICTI not found
             {
             ptrobj = 0;

             apk_ReadRamDataICC( ADDR_ICC_ISU_CTI, 2, (UCHAR *)&iLen );
//           if( iLen == 1 ) // CTI already there?
	     if( (iLen == 1) && (g_pse == TRUE) ) // PATCH: 2009-03-25, CTI already there? (only for PSE)
               {
               apk_ReadRamDataICC( ADDR_ICC_ISU_CTI+2, 1, (UCHAR *)&icti );

               if( (icti == 1 ) && (atcap[4] == 1) ) // code table 1?
                 ptrobj = bak_ptrobj; // using preferred name
               else
                 goto ACL_USE_APP_LABEL;
               }
             else
               goto ACL_USE_APP_LABEL;
             }
           }

         // store LEN[1] NAME[LEN]
         apk_WriteRamDataTERM( addr+1, *ptrobj+1, ptrobj );
         }

      return( emvOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: Remove the specified candidate from list, because failed to select.
// INPUT   : link - index to the candidate name list.
// OUTPUT  : none.
// REF     : SRAM data for output: ADDR_CANDIDATE_NAME[16][18]
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
UCHAR apk_RemoveCandidateList( UCHAR link )
{
UCHAR i;
UINT  addr;
UCHAR data;
UCHAR appname[CANDIDATE_NAME_LEN];

      for( i=0; i<MAX_CANDIDATE_NAME_CNT; i++ )
         {
         addr = ADDR_CANDIDATE_NAME_01 + CANDIDATE_NAME_LEN*i;
         apk_ReadRamDataTERM( addr, CANDIDATE_NAME_LEN, appname );

         if( *appname == link )
           {
           // set "removed" mark on LEN field
           data = -1;
           apk_WriteRamDataTERM( addr+1, 1, &data );

           return( apkOK );
           }
         }

      return( apkFailed );
}

