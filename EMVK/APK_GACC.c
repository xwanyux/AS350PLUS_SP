//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APK_GACC.C                                                 **
//**  MODULE   : apk_GenerateAC()                                           **
//**             apk_ExternalAuthen()                                       **
//**             apk_IssuerScriptProcessing()                               **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2003/01/27                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  History  :                                                            **
//**	         2009/03/04 Richard :EMV4.2a bug fixed                      **
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
#include "APDU.H"
#include "TOOLS.h"

// ---------------------------------------------------------------------------
// FUNCTION: Generate AC Command & Retrieve the related data objects.
// INPUT   : type - type of AC. (AC_XXX)
// OUTPUT  : ac   - 2L-V, cryptogram-related data.
//                        V: 80-L-[Value] SW1 SW2      or
//                        V: 77-L-[...Data...] SW1 SW2
//
//              Returned Values/Data Objects Format:
//              ----    --------------------------------  --------------
//              Tag     Value                             Presence
//              ----    --------------------------------  --------------
//          [1] 9F27    Cryptogram Information Data       M  (b1)
//          [2] 9F36    Application Trans. Counter (ATC)  M  (b2)
//          [3] 9F26    Application Cryptogram (AC)       M  (b8)
//                      or
//          [3] 9F4B    Signed Dynamic Application Data   M  (Nic)
//          [4] 9F10    Issuer Application Data           O  (up to b32)
//              Others  (ignored)                         O
//              ----    --------------------------------  --------------
// REF     :
// RETURN  : apkOK
//           apkFailed (device error or no response from icc)
//           apkCondNotSatisfied (SW1 SW2=6985)
//           apkMObjMissing (CDA mandatory object missing Tag 9F4B, 9F36, 9F27 )
// ---------------------------------------------------------------------------
UCHAR apk_GenerateAC( UCHAR type, UCHAR *cdol, UCHAR *ac )
{
UINT  iLen;
UINT  iLen2;
UINT  iPileLen;
UCHAR cnt;
UCHAR *ptrobj;
UCHAR *ptrtmp;
UCHAR tlv[261];
UCHAR result;
UCHAR len[2];
UCHAR cid;

      // check excution result
      if( apdu_GENERATE_AC( type, cdol, ac ) == apiFailed )
        return( apkFailed );

      // check length
      iLen = ac[1]*256 + ac[0];
      if( iLen == 2 ) // PATCH: 2003-11-10, ETEC REQ10 MC, goto FallBack
        {
        if( (ac[2] == 0x69) && (ac[3] == 0x85) )
          return( apkCondNotSatisfied );
        else
          return( apkFailed );
        }
      if( (type & AC_CDA_REQ) == 0 ) // non-CDA process
        {
        if( iLen < 15 )
          return( apkFailed );
        }

      // check return data format
      switch( ac[2] )
            {

            case 0x80: // primitive
            //20090302_Richard: EMV 4.2A,2CC.122.02

            //     if( type & AC_CDA_REQ ) // EMV2000: CDA
            //       return( apkFailed ); // invalid format

            	//check RAPDU format

           //      break;

            case 0x77: // constructed

                 if( ( (ac[iLen] == 0x90) && (ac[iLen+1] == 0x00) ) ||
                     (ac[iLen] == 0x61) )
                   break;
                 else
                   {
                   if( (ac[iLen] == 0x69) && (ac[iLen+1] == 0x85) )
                     return( apkCondNotSatisfied );
                   else
                     return( apkFailed );
                   }

            default:
                 return( apkFailed ); // invalid format
            }

      // PATCH: 2003-06-04, checking for both Tag = 80 & 77
      iLen2 = apk_GetBERLEN( &ac[3], &cnt ); // actual length of data objects
      if( (iLen - 2) != (iLen2 + 1 + cnt) )
        return( apkFailed );

      if( ac[2] == 0x80 ) // primitive?
        {
        // save "Cryptogram Information Data" (CID)
        iLen = 1;
        apk_WriteRamDataICC( ADDR_ICC_CID, 0x02, (UCHAR *)&iLen );
        apk_WriteRamDataICC( ADDR_ICC_CID+2, 0x01, &ac[3+cnt] );

        // save "Application Transaction Counter" (ATC)
        iLen = 2;
        apk_WriteRamDataICC( ADDR_ICC_ATC, 0x02, (UCHAR *)&iLen );
        apk_WriteRamDataICC( ADDR_ICC_ATC+2, 0x02, &ac[4+cnt] );

        // save "Application Cryptogram" (AC)
        iLen = 8;
        apk_WriteRamDataICC( ADDR_ICC_AC, 0x02, (UCHAR *)&iLen );
        apk_WriteRamDataICC( ADDR_ICC_AC+2, 0x08, &ac[6+cnt] );

        // save "Issuer Application Data"
        if( iLen2 > 11 )
        {
        	iLen2 -= 11;
        	//EMV 42a Charles 2009-03-09  2ca.054.01, 2CC003.00 case 13~18 ,RAPDU Tag is 80
        	if(iLen2>32)//check Issuer Application Data Length correction.
        		return apkFailed;
        	//end. //EMV 42a Charles 2009-03-09  2ca.054.01
          apk_WriteRamDataICC( ADDR_ICC_ISU_AP_DATA, 0x02, (UCHAR *)&iLen2 );
          apk_WriteRamDataICC( ADDR_ICC_ISU_AP_DATA+2, iLen2, &ac[14+cnt] );
        }
      }
      else // constructed (Tag=77)
      {
//      iLen2 = apk_GetBERLEN( &ac[3], &cnt ); // template length
//      if( (iLen - 2) != (iLen2 + 1 + cnt) )  // MOVED: 2003-06-04
//        return( apkFailed );

        ptrtmp = &ac[2] + 1 + cnt; // ptr to the 1'st TLV
        iLen = iLen2;

        // find CID (M)
        ptrobj = apk_FindTag( 0x9f, 0x27, &ac[2] );
        if( ptrobj != 0 )
        {
          // save "Cryptogram Information Data" (CID)
      //  iLen = *ptrobj++;
          iLen2 = apk_GetBERLEN( ptrobj, &cnt );
          if( iLen2 != 1 )
            return( apkFailed );

          ptrobj += cnt;
          apk_WriteRamDataICC( ADDR_ICC_CID, 0x02, (UCHAR *)&iLen2 );
          apk_WriteRamDataICC( ADDR_ICC_CID+2, iLen2, ptrobj );

          iPileLen = 2 + iLen2 + cnt;
        }
        else
          return( apkMObjMissing );//20090302_Richard:CC.135.00, mandatory data objects are not present in response to generate AC

        // find ATC (M)
        ptrobj = apk_FindTag( 0x9f, 0x36, &ac[2] );
        if( ptrobj != 0 )
          {
          // save "Application Transaction Counter" (ATC)
      //  iLen = *ptrobj++;
          iLen2 = apk_GetBERLEN( ptrobj, &cnt );
          if( iLen2 != 2 )
            return( apkFailed );

          ptrobj += cnt;
          apk_WriteRamDataICC( ADDR_ICC_ATC, 0x02, (UCHAR *)&iLen2 );
          apk_WriteRamDataICC( ADDR_ICC_ATC+2, iLen2, ptrobj );

          iPileLen += (2 + iLen2 + cnt);
          }
        else
          return( apkMObjMissing );//20090302_Richard:CC.135.00, mandatory data objects are not present in response to generate AC


        // find AC [M]
        if( (type & AC_CDA_REQ) == 0 ) // non-CDA process
        {
          ptrobj = apk_FindTag( 0x9f, 0x26, &ac[2] );
          if( ptrobj != 0 )
            {
            // save "Application Cryptogram" (AC)
        //  iLen = *ptrobj++;
            iLen2 = apk_GetBERLEN( ptrobj, &cnt );
            if( iLen2 != 8 )
              return( apkFailed );

            ptrobj += cnt;
            apk_WriteRamDataICC( ADDR_ICC_AC, 0x02, (UCHAR *)&iLen2 );
            apk_WriteRamDataICC( ADDR_ICC_AC+2, iLen2, ptrobj );

            iPileLen += (2 + iLen2 + cnt);
            }
          else
            return( apkFailed );
        }
        else // CDA process
        {
          len[0] = 0; // clear flags for AC and SDAD
          len[1] = 0; //

          api_emv_GetDataElement( DE_ICC, ADDR_ICC_CID+2, 1, (UCHAR *)&cid );



          ptrobj = apk_FindTag( 0x9f, 0x26, &ac[2] );
          if( ptrobj != 0 )
            {
             // save "Application Cryptogram" (AC)
             iLen2 = apk_GetBERLEN( ptrobj, &cnt );
             if( iLen2 != 8 )
               return( apkFailed );
             len[0] = 1; // set flag

             ptrobj += cnt;
             apk_WriteRamDataICC( ADDR_ICC_AC, 0x02, (UCHAR *)&iLen2 );
             apk_WriteRamDataICC( ADDR_ICC_AC+2, iLen2, ptrobj );

             iPileLen += (2 + iLen2 + cnt);
            }

          ptrobj = apk_FindTag( 0x9f, 0x4b, &ac[2] );
          if( ptrobj != 0 )
            {
            // save "Signed Dynamic Application Data" (SDAD)
            iLen2 = apk_GetBERLEN( ptrobj, &cnt );
            len[1] = 1; // set flag

            ptrobj += cnt;
            apk_WriteRamDataICC( ADDR_ICC_SIGNED_DAD, 0x02, (UCHAR *)&iLen2 );
            apk_WriteRamDataICC( ADDR_ICC_SIGNED_DAD+2, iLen2, ptrobj );

            iPileLen += (2 + iLen2 + cnt);
            }
          else if  ( ( cid & CID_AC_MASK) == AC_ARQC || ( cid & CID_AC_MASK) == AC_TC )
            {
               return( apkMObjMissing );//20090310_Richard: EMV 4.2a, 2CC.135.00, mandatory data objects are not present in response to generate AC
            }

//        if( (len[0] == 0) && (len[1] == 0) ) // both AC & SDAD are absent
//          return( apkFailed );

//        if( (len[0] != 0) && (len[1] != 0) ) // both AC & SDAD are present
//          return( apkFailed );
        }

        // find "Issuer Applicaton Data" (O)
        ptrobj = apk_FindTag( 0x9f, 0x10, &ac[2] );
        if( ptrobj != 0 )
          {
          // save "Issuer Application Data"
      //  iLen = *ptrobj++;
          iLen2 = apk_GetBERLEN( ptrobj, &cnt );
          if( iLen2 > 32 )
            return( apkFailed );

          ptrobj += cnt;
          apk_WriteRamDataICC( ADDR_ICC_ISU_AP_DATA, 0x02, (UCHAR *)&iLen2 );
          apk_WriteRamDataICC( ADDR_ICC_ISU_AP_DATA+2, iLen2, ptrobj );

          iPileLen += (2 + iLen2 + cnt);
          }


//#ifdef L2_PBOC20
//        // PATCH: 2006-02-21, PBOC2.0, 2CA056, don't check the following redundant data elements
//        return( apkOK );
//#endif

        if( iLen  > iPileLen ) // any redundancy?
          {
          result = *(ptrtmp+iPileLen); // get first Tag
          if( apk_CheckWordTag( result ) == FALSE )
            {
            iLen2 = apk_GetBERLEN( ptrtmp+1+iPileLen, (UCHAR *)&cnt );
            iLen2 += ( cnt + 1 );
            }
          else
            {
            iLen2 = apk_GetBERLEN( ptrtmp+2+iPileLen, (UCHAR *)&cnt );
            iLen2 += ( cnt + 2 );
            }

// TL_DispHexWord( 0,0, iLen );
// TL_DispHexWord( 1,0, iPileLen );
// TL_DispHexWord( 2,0, iLen2 );
// UI_WaitKey();

          if( (iLen - iPileLen) == iLen2 )
            return( apkOK );
          else	//mask in EMV 42a 2009-03-03 Charles 2CA.055.01
            return( apkFailed );

          // ----------------------------------------------------------------
          // scan all "T-L-V"
          len[0] = iLen & 0x00ff;
          len[1] = (iLen & 0xff00) >> 8;
          while(1)
               {
               ptrobj = apk_GetBERTLV( len, ptrtmp, tlv );

               if( (len[1]*256 + len[0]) == 0 ) // remaining length
                 return( apkOK );

               if( ptrobj == 0 ) // legal proprietary TLV
                 return( apkFailed );

               if( ptrobj == (UCHAR *)-1 ) // illega TLV
                 return( apkFailed );

               ptrtmp = ptrobj;
               }
          }
        }

      return( apkOK );
}

// ---------------------------------------------------------------------------
// FUNCTION: issuing command to ask the application in the icc
//           to verify a cryptogram.
// INPUT   : iad - 2L-V, the issuer authentication data.
//                 mandatory: 8 bytes containing the cryptogram.
//                 optional : 1~8 bytes are proprietary.
// OUTPUT  : none.
// RETURN  : apkOK               (SW1SW2 = 9000)
//           apkFailed           (SW1SW2 = 6300)
//           apkCondNotSatisfied (SW1SW2 = 6985)
//           apkOutOfService     (not supported or device error) -> terminate tx.
//           apkUnknown          (SW1SW2 = others)
// ---------------------------------------------------------------------------
UCHAR apk_ExternalAuthen( UCHAR *iad )
{
UCHAR rapdu[16];

      if( apdu_EXTERNAL_AUTHENTICATE( iad, rapdu ) == apiOK )
        {
        if( (rapdu[1]*256 + rapdu[0]) == 2 )
          {
          switch( rapdu[2]*256 + rapdu[3] )
                {
                case 0x9000:
                     return( apkOK );

                case 0x6300:
                     return( apkFailed );

                case 0x6985:
                     return( apkCondNotSatisfied );

                default:
                //   return( apkOutOfService );
                     return( apkUnknown ); // EMV2000
                }
          }
        else
          return( apkOutOfService );
        }
      else
        return( apkOutOfService );
}

// ---------------------------------------------------------------------------
// FUNCTION: process the issuer script commands with Template Tag=0x71.
//           (processed before issuing the final GENERATE AC command)
// INPUT   : ist - 2L-[T-L-V], issuer scripts template.
//                 T-L-(9F18-04-ScriptID[4])-[Commands], T=71 or 72.
//                 Commands: [86-L1-CMD1][86-L2-CMD2][86-L3-CMD3]...
//                 ScriptID: optional, also "9F18-04".
//           tag - the script tag to be process. (0x71 or 0x72)
// OUTPUT  : none.
// REF     : g_ibuf
// RETURN  : apkOK           (SW1SW2 = 90xx, 62xx, 63xx)
//           apkFailed       (SW1SW2 = 69xx, 6Axx)
//           apkOutOfService (not supported or device error)
// ---------------------------------------------------------------------------
UCHAR apk_IssuerScriptProcessing( UCHAR tag, UCHAR *ist )
{
UINT  i;
int   iTempLen;
int   iCmdLen;
UINT  iLen;
UCHAR *ptrist;
UCHAR cnt;
UCHAR tag2;

      iTempLen = ist[1]*256 + ist[0]; // total template size
      if( iTempLen < 5 )
        return( apkFailed );

      ptrist = ist + 2; // pointer to the 1'st [T]

      while( iTempLen > 0 ) // scripts
           {

           if( *ptrist == tag )
             {
             iLen = apk_GetBERLEN( ++ptrist, (UCHAR *)&cnt );
             ptrist += cnt; // pointer to next [T] (9F18 or 86)

             // ScriptID present?
             if( (*ptrist == 0x9F) && (*(ptrist+1) == 0x18) )
               {
               if( *(ptrist+2) != 0x04 )
                 return( apkFailed ); // illegal ScriptID size

               ptrist += 7; // yes, skip "ScriptID" and go to next [Tn]
               iCmdLen = iLen - 7;
               }
             else
               iCmdLen = iLen;

             iTempLen -= (iLen + cnt + 1); // remaining template data size

        //   TL_DispHexWord(0,0,iTempLen);
        //   TL_DispHexWord(1,0,iCmdLen);
        //   for(;;);

             // script commands processing: send commands to ICC in sequence
             while( iCmdLen > 0 )
                  {

                  if( *ptrist == 0x86 )
                    {
                    iLen = apk_GetBERLEN( ++ptrist, (UCHAR *)&cnt );
                    ptrist += cnt; // pointer to next cmd [Vn]

                    if( apdu_ISSUER_SCRIPT_COMMAND( iLen, ptrist, g_ibuf ) == apiOK )
                      {
                      i = g_ibuf[1]*256 + g_ibuf[0]; // size of response APDU
                      if( i >= 2 )
                        {
                        // examine only SW1
                        switch( g_ibuf[i] )
                              {
                              case 0x90:
                              case 0x62:
                              case 0x63:
                                   break;

                              default: // 0x69, 0x6a, or others
                                   return( apkFailed );
                              }
                        }
                      else
                        return( apkFailed );

                      iCmdLen -= (iLen + cnt + 1); // 86 Ln Vn
                      ptrist += iLen; // pointer to next [86-Ln-Vn]
                      }
                    else
                      return( apkOutOfService ); // error
                    }
                  else
                    return( apkFailed ); // invalid script command tag

                  } // while( iCmdLen > 0 ) for next command (86)
             }
           else
             {
             if( tag == 0x71 )
               tag2 = 0x72;
             else
               tag2 = 0x71;

             if( *ptrist == tag2 )
               {
               iLen = apk_GetBERLEN( ++ptrist, (UCHAR *)&cnt );
               ptrist += (iLen + cnt); // pointer to next [T=71 or 72]

               iTempLen -= (iLen + cnt + 1);

        //     TL_DispHexWord(0,0,iTempLen);
        //     UI_WaitKey();
        //     TL_DumpHexData(0,0,8,ptrist);
        //     for(;;);
               }
             else
               return( apkFailed ); // invalid script template tag
             }

           } // while( iTempLen > 0 ) for next script (71 or 72)
}

// ---------------------------------------------------------------------------
// FUNCTION: update issuer script results. (5-byte / id )
// INPUT   : id     - the script id (4 bytes)
//           sn     - the sequence number. (0-15)
//           result - the final result of the specified id.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void  apk_UpdateISR( UCHAR *id, UCHAR sn, UCHAR result )
{
UINT  addr;
UCHAR cnt;
UCHAR isr[5];

      apk_ReadRamDataTERM( ADDR_TERM_ISR, 1, (UCHAR *)&cnt ); // get current counter
      addr = ADDR_TERM_ISR + cnt*5 + 1; // next isr address

      if( result == 2 )
        sn = 0; // if successful, do not specify sn.

      isr[0] = ((result << 4) & 0xf0) + sn;
      memmove( &isr[1], id, 4 );

      // update
      cnt++;
      apk_WriteRamDataTERM( ADDR_TERM_ISR, 1, (UCHAR *)&cnt );
      apk_WriteRamDataTERM( addr, 5, isr );
}

// ---------------------------------------------------------------------------
void ISP_SetFailedTVR( UCHAR tag )
{

      if( tag == 0x71 )                                     //
        g_term_TVR[4] |= TVR4_SP_FAILED_BEFORE_FAC;         //
      else                                                  //
        g_term_TVR[4] |= TVR4_SP_FAILED_AFTER_FAC;          //
                                                            //
      api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+4, 1, &g_term_TVR[4] );
}

// ---------------------------------------------------------------------------
// FUNCTION: process the issuer script commands with Template Tag=0x71.
//           (processed before issuing the final GENERATE AC command)
//           (1) read one template head [7x-L-(9F18-L-V)]
//           (2) read one script command [86-Ln-Vn]
//           (3) loop (2)
//           (4) loop (1)
// INPUT   : ist - 2L-[T-L-V], issuer scripts template.
//                 T-L-(9F18-04-ScriptID[4])-[Commands], T=71 or 72.
//                 Commands: [86-L1-CMD1][86-L2-CMD2][86-L3-CMD3]...
//                 ScriptID: optional, also "9F18-04".
//                 The "ist" is located at larger ADDR_ISU_SCRIPT_TEMP[] pool
//                 instead of at limited global memory.
//                 format: 2L-DATA[n]
//           tag - the script tag to be process. (0x71 or 0x72)
// OUTPUT  : none.
// REF     : g_ibuf
// RETURN  : apkOK           (SW1SW2 = 90xx, 62xx, 63xx)
//           apkFailed       (SW1SW2 = 69xx, 6Axx)
//           apkOutOfService (not supported or device error)
//           apkNotReady     (not performed, eg. tag=71 but isc=72...)
//
// NOTE    : terminal shall maintain the "Issuer Script Results" (ADDR_TERM_ISR)
//           BYTE 1: bit5-8
//                        0=script not performed
//                        1=script processing failed
//                        2=script processing successful
//                   bit1-4
//                        0=not specified (OK)
//                        1 ~ E = sequence number       (error)
//                        F     = sequence number >= 15 (error)
//           BYTE 2-5: Script ID. (filled in 0's if not available).
// ---------------------------------------------------------------------------
UCHAR apk_IssuerScriptProcessing2( UCHAR tag, UINT addr_ist )
{
UCHAR ist[300];
UINT  i;
int   iTempLen = 0;
int   iCmdLen = 0;
UINT  iLen;
UINT  iAddr;
UINT  iPileLen;
UCHAR *ptrist;
UCHAR cnt;
UCHAR tag2;
UCHAR script_id[4];
UCHAR script_sn;
UCHAR script_result;
UCHAR flag_exe;
UCHAR flag_err_format;
UCHAR flag_ever;
UCHAR flag_isc_err;

      flag_exe = FALSE;  // reset scripts execution flag
      flag_ever = FALSE; // reset scripts ever executed flag
      flag_err_format = FALSE; // script format

//    iAddr = ADDR_ISU_SCRIPT_TEMP; // set start address
      iAddr = addr_ist; // set start address

      apk_ReadRamDataTERM( iAddr, 2, (UCHAR *)&iTempLen ); // total template size
      if( iTempLen < 8 )
        return( apkFailed );

      iAddr += 2;
//    apk_ReadRamDataTERM( iAddr, 16, ist ); // read template header
//    ptrist = ist; // pointer to the 1'st [T]

      iPileLen = 0;

ISC_ST:

      while( iTempLen > 0 ) // scripts
           {
           flag_isc_err = FALSE;    // ISC.SW1 !=90, 62 or 63

           memset( script_id, 0x00, 4 );
           script_sn = 0;
           script_result = 0;

           apk_ReadRamDataTERM( iAddr, 265, ist ); // read template header
           ptrist = ist; // pointer to the 1'st [T]
//         if( iTempLen > 128 ) // PATCH: PBOC2.0, 2006-02-16, 2CO03402
//           {
//           g_isr_len_overrun = *ptrist;
//           apk_UpdateISR( script_id, script_sn, script_result ); // set default ISR = 00 00 00 00 00
//           return( apkFailed );
//           }

           // scan the template Tag 71 or 72
           if( (*ptrist != 0x71) && (*ptrist != 0x72) )
             {
             while( iTempLen > 0 )
                  {
                  if( (*ptrist != 0x71) && (*ptrist != 0x72) )
                    {
                    ptrist++;
                    iTempLen--;
                    iAddr++;
                    }
                  else
                    {
                    if( iTempLen < 8 )
                      return( apkFailed );
                    else
                      goto ISC_START;
                    }
                  }
             return( apkFailed );
             }

ISC_START:

           if( *ptrist == tag )
             {
             iLen = apk_GetBERLEN( ++ptrist, (UCHAR *)&cnt ); // 71-L
             ptrist += cnt; // pointer to next [T] (9F18 or 86)

             iPileLen += (iLen + 1 + cnt);                           // PATCH: 2006-10-12, 2CO.034.02, 2CO.034.03

             if( iPileLen > 128 )                                    //
               {                                                     //
               apk_UpdateISR( script_id, script_sn, script_result ); // set default ISR = 00 00 00 00 00
               return( apiFailed );
               }

// TL_DispHexWord(1,0,iLen);
// TL_DispHexWord(2,0,iTempLen);
// UI_WaitKey();
             if( iLen >= iTempLen ) // PATCH: PBOC2.0, 2006-02-16, 2CO035b (bad length)
               {
               apk_UpdateISR( script_id, script_sn, script_result ); // set default ISR = 00 00 00 00 00
               return( apkFailed );
               }

             if( iTempLen > (iLen + 2) )
               {
               if( (ist[iLen+2] != 0x71) && (ist[iLen+2] != 0x72) )
                 {
                 // 7x length error, skip this template and find next 7x
                 if( (*ptrist == 0x9F) && (*(ptrist+1) == 0x18) )
                   {
                   if( *(ptrist + 2) == 4 )
                     memmove( script_id, ptrist+3, 4 );
                   }

                 flag_exe = TRUE;
                 flag_ever = TRUE;
                 flag_err_format = TRUE;

                 iAddr += 2;
                 iTempLen -= 2;
                 while( iTempLen > 0 )
                      {
                      if( (*ptrist != 0x71) && (*ptrist != 0x72) )
                        {
                        ptrist++;
                        iAddr++;
                        iTempLen--;
                        }
                      else // found
                        goto ISC_2000;
                      }

                 // PATCH: PBOC2.0, 2006-02-15, 2CO.035.00c-1
                 apk_ReadRamDataTERM( ADDR_TERM_ISR, 1, (UCHAR *)&cnt ); // get current counter
                 if( cnt == 0 )
                   {
                   apk_UpdateISR( script_id, script_sn, script_result ); // set default ISR = 00 00 00 00 00
            //     return( apkNotReady );  2006-02-17 removed
                   }
                 return( apkFailed );
                 }
               }

             // ScriptID present?
             if( (*ptrist == 0x9F) && (*(ptrist+1) == 0x18) )
               {
               if( *(ptrist+2) == 0x00 ) // PATCH: 2003-09-16, seems "no ID"
                 {
                 ptrist += 3; // skip "9F 18 00"
                 iCmdLen = iLen - 3;
                 iAddr += (cnt + 1 + 3); // pointer to the 1'st [T=86]

                 goto ISC_1000;
                 }

               if( *(ptrist+2) != 0x04 )
                 return( apkFailed ); // illegal ScriptID size

               memmove( script_id, ptrist+3, 4 );
               script_sn = 0;

               ptrist += 7; // yes, skip "ScriptID" and go to next [Tn]
               iCmdLen = iLen - 7;
               iAddr += (cnt + 1 + 7); // pointer to the 1'st [T=86]
               }
             else
               {

               // PATCH: PBOC2.0, 2006-02-17, 2CO036, neither 9F18 nor 86 then ignore current script
               if( (*ptrist == 0x9F) && (*(ptrist+2) == 0x04) && (*ptrist != 0x86) )
                 {
                 iTempLen -= (iLen + 1 + cnt);
                 iAddr += (iLen + 1 + cnt);

                 apk_UpdateISR( script_id, script_sn, script_result ); // PATCH: PBOC2.0, 20006-02-17
                                                                       //
            //   if( tag == 0x71 )                                     //
            //     g_term_TVR[4] |= TVR4_SP_FAILED_BEFORE_FAC;         //
            //   else                                                  //
            //     g_term_TVR[4] |= TVR4_SP_FAILED_AFTER_FAC;          //
            //                                                         //
            //   api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+4, 1, &g_term_TVR[4] );

                 flag_exe = TRUE;
                 flag_ever = TRUE;
                 ISP_SetFailedTVR( tag );

                 goto ISC_ST; // parse next one
                 }

               iCmdLen = iLen;
               iAddr += (cnt + 1);     // pointer to the 1'st [T=86]
               }
ISC_1000:
             iTempLen -= (iLen + cnt + 1); // remaining template data size

//  TL_DispHexWord(0,0,iTempLen);
//  TL_DispHexWord(1,0,iCmdLen);
//  TL_DispHexWord(2,0,iLen);
//  UI_WaitKey();

             // script commands processing: send commands to ICC in sequence
             while( iCmdLen > 0 )
                  {
                  apk_ReadRamDataTERM( iAddr, 265, ist ); // read one script command
                  ptrist = ist;

                  if( *ptrist == 0x86 )
                    {
                    iLen = apk_GetBERLEN( ++ptrist, (UCHAR *)&cnt );
                    ptrist += cnt; // pointer to next cmd [Vn]

                    // -----------------------------------------------------
                    // PATCH: 2006-10-11, 2CJ.195.00, case 2
                    // -----------------------------------------------------
//                  if( iLen < 7 )
//                    goto ISC_TAG86_MISSING; // ignore the command set

                    // -----------------------------------------------------

//  TL_DispHexWord(0,0,iCmdLen);
//  TL_DispHexWord(1,0,iLen);
//  TL_DumpHexData(0,2,16,ptrist+iLen);

                    // PATCH: 2006-10-12, 2CJ.195.00-02, 2CO.034.00-01 (Tag 86 with incorrect LEN)
                    if( (iLen+1+cnt) < iCmdLen )
                      {
                      if( *(ptrist+iLen) != 0x86 )
                        {
                        flag_exe = TRUE;
                        flag_ever = TRUE;
                        ISP_SetFailedTVR( tag );

                        iAddr += iCmdLen; // ptr to next [7x-Ln-Vn]
                        goto ISC_2000;
                        }
                      }
                    //EMV 42a 2009-03-05 Charles 2CO.035.02 case1~4
                    else if((iLen+1+cnt)>iCmdLen)
                    {
                    	flag_err_format = TRUE;
                    	apk_UpdateISR( script_id, script_sn, script_result );
                    	goto ISC_EXIT;
                    }

                   if( flag_isc_err == TRUE ) // PATCH: 2006-10-14
                     {                        //
                     ptrist += iLen;          // skip other Tag86 cmd in the same template
                     goto ISC_NEXT_TAG86;     //
                     }

                    script_sn++;
                    if( script_sn >= 15 )
                      script_sn = 15;

                    flag_exe = TRUE;
                    flag_ever = TRUE;
                    if( apdu_ISSUER_SCRIPT_COMMAND( iLen, ptrist, g_ibuf ) == apiOK )
                      {
                      ptrist += iLen;

                      i = g_ibuf[1]*256 + g_ibuf[0]; // size of response APDU
                      if( i >= 2 )
                        {
                        // examine only SW1
                        switch( g_ibuf[i] )
                              {
                              case 0x90:
                              case 0x62:
                              case 0x63:

                                   script_result = 2; // successful
                                   break;

//								08-dec-31 charles Masked
//                              case 0x6d: // PATCH: PBOC2.0, 2006-02-15, 2CO.034B
//                                   script_result = 1; // failed
//                                   break;

                              default: // 0x69, 0x6a, or others, cf: 2CJ.199.00, 2CM,048.00-07, 2006-10-14

                                   script_result = 1; // failed
                            //     apk_UpdateISR( script_id, script_sn, script_result );
                            //     return( apkFailed );

                                   ISP_SetFailedTVR( tag ); // PATCH: 2006-10-14
                                   flag_isc_err = TRUE;     //
                              }
                        }
                      else
                        {
                        script_result = 1; // failed
                        apk_UpdateISR( script_id, script_sn, script_result );

                        return( apkFailed );
                        }
ISC_NEXT_TAG86:
                      iCmdLen -= (iLen + cnt + 1); // 86 Ln Vn
                      iAddr += (iLen + cnt + 1); // pointer to next [86-Ln-Vn]
                      }
                    else
                      return( apkOutOfService ); // error
                    }
                  else // tag 86 is missing (=not performed)
                    {
ISC_TAG86_MISSING:
                    script_result = 0; // not specified, PATCH: 2005/05/06
                    flag_exe = TRUE;
                    flag_ever = TRUE;       // ever try to execute
                    flag_err_format = TRUE; // invalid script command tag

                    // ------------------------------------------------------
                    // PATCH: 2006-10-11, 2CJ.195.00 case 1
                    // ------------------------------------------------------
//                  iAddr += iCmdLen; // ptr to next [7x-Ln-Vn]
//                  iCmdLen = 0;

                    // ------------------------------------------------------
                    iLen = apk_GetBERLEN( ++ptrist, (UCHAR *)&cnt );

                    if( iCmdLen >= (iLen + cnt + 1) )   // PATCH: 2005/05/06
                      {
                      iCmdLen -= (iLen + cnt + 1); // 86 Ln Vn
                      iAddr += (iLen + cnt + 1); // pointer to next [86-Ln-Vn]
                      }
                    else
                      {                      // PATCH: 2005/05/18
                      if( flag_exe == TRUE ) // 2CJ.194.00, 2CJ.195.00
                        {
                        flag_exe = FALSE;
                        apk_UpdateISR( script_id, script_sn, script_result );
                        }

                      goto ISC_EXIT; // terminate
                      }

                    }

                  } // while( iCmdLen > 0 ) for next command (86)
ISC_2000:
             if( flag_exe == TRUE ) // PATCH: 2005/05/06
               {
               flag_exe = FALSE;
               apk_UpdateISR( script_id, script_sn, script_result );
               }
             }
           else
             {
             if( tag == 0x71 )
               tag2 = 0x72;
             else
               tag2 = 0x71;

             if( *ptrist == tag2 )
               {
               iLen = apk_GetBERLEN( ++ptrist, (UCHAR *)&cnt );

               if( iLen == 0 ) // PATCH: PBOC2.0, 2006-02-17, 2CO035c-1
                 return( apkNotReady );

               iAddr += (iLen + cnt + 1);
               apk_ReadRamDataTERM( iAddr, 16, ist ); // read template header
               ptrist = ist; // pointer to next [T=71 or 72]

               iTempLen -= (iLen + cnt + 1);

        //     TL_DispHexWord(0,0,iTempLen);
        //     UI_WaitKey();
        //     TL_DumpHexData(0,0,8,ptrist);
        //     for(;;);
               }
             else
               return( apkFailed ); // invalid script template tag
             }

           } // while( iTempLen > 0 ) for next script (71 or 72)

//    if( flag_exe == TRUE )
//      apk_UpdateISR( script_id, script_sn, script_result );

ISC_EXIT:

      if( flag_err_format == TRUE )
        return( apkFailed );
      else
        {
        if( flag_ever == TRUE )
          return( apkOK ); // done
        else
          return( apkNotReady ); // PATCH: 2003-08-03
                                 // script is not performed
        }
}

