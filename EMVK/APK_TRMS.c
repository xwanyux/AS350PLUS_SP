//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APK_TRMS.C                                                 **
//**  MODULE   : apk_FloorLimitChecking()                                   **
//**             apk_RandomTransactionSelection()                           **
//**             apk_VelocityChecking()                                     **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2003/01/27                                                 **
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
#include "APDU.H"
#include "TOOLS.h"

extern	ULONG api_sys_random_len( UCHAR *dbuf, UINT len );

// ---------------------------------------------------------------------------
// FUNCTION: generate a random number (1..99) for TRM.
// INPUT   : none.
// OUTPUT  : none.
// REF     :
// RETURN  : the random number (hex).
// ---------------------------------------------------------------------------
UCHAR apk_RandomNumber( void )
{
UCHAR i;
UCHAR ran[8];

/*
      while(1)
           {
//         TL_GetRandomNumber( ran ); // 8-byte hex data
           apk_GetChallengeTERM( ran ); // 8-byte hex data

           for( i=0; i<8; i++ )
              {
              if( (ran[i] >= 1) && (ran[i] <= 99) )
                return( ran[i] );
              }
           }
*/

	// 2012-02-14, RND must be btw 1..99
	while(1)
	     {
	     api_sys_random_len( ran, 1 );
	     if( (ran[0] >= 1) && (ran[0] <= 99) )
	       return( ran[0] );
	     }
}

// ---------------------------------------------------------------------------
// FUNCTION: RFU.
// INPUT   : none.
// OUTPUT  : none.
// REF     :
// RETURN  : apkOK
//           apkFailed
// ---------------------------------------------------------------------------
UCHAR apk_ExceptionFileChecking( void )
{

}

// ---------------------------------------------------------------------------
// FUNCTION: compare the "Amount, Authorised" with the "Terminal Floor Limit".
// INPUT   : amt1 - the current transaction amount. (n10+n2)
//           amt2 - the most recent entry amount with the same PAN. (n10+n2)
//                  this amount value may be zero due to no matched PAN or
//                  no log available.
//                  where: n2 (minor unit, is reserved for decimal part)
// OUTPUT  : none.
// REF     : g_selected_aid_index
// RETURN  : apkOK      ( trans. amt >= TFL )
//           apkFailed  ( trans. amt < TFL  )
// ---------------------------------------------------------------------------
UCHAR apk_FloorLimitChecking( UCHAR *amt1, UCHAR *amt2 )
{
UCHAR i;
UCHAR tfl[TFL_LEN];
ULONG hex1;
ULONG hex2;

      // read the target TFL for the selected AID
//    apk_ReadRamDataTERM( ADDR_TFL_01+TFL_LEN*g_selected_aid_index, TFL_LEN, tfl );
      apk_ReadRamDataTERM( ADDR_TFL, TFL_LEN, tfl );

      // if TFL = 0 then skip checking (TFL not present)
//    for( i=0; i<TFL_LEN; i++ )
//       {
//       if( tfl[i] != 0 )
//         goto TRM_TFL;
//       }
//    return( apkFailed );

      // PATCH: 2003-06-08
//    apk_ReadRamDataTERM( ADDR_TFL_FLAG_01+g_selected_aid_index, 1, (UCHAR *)&i );
      apk_ReadRamDataTERM( ADDR_TFL_FLAG, 1, (UCHAR *)&i );
      if( i == 0 )
        return( apkFailed ); // TFL not present

TRM_TFL:
      // amt = amt1 + amt2 (check only integer part)
      TL_bcd2hex( 5, &amt1[1], (UCHAR *)&hex1 );
      TL_SwapData( 4, (UCHAR *)&hex1 );
      TL_bcd2hex( 5, &amt2[1], (UCHAR *)&hex2 );
      TL_SwapData( 4, (UCHAR *)&hex2 );
      hex1 = hex1 + hex2;
      TL_SwapData( 4, (UCHAR *)&hex1 );     // hex1 = amt

      TL_bcd2hex( 5, &tfl[1], (UCHAR *)&hex2 ); // hex2 = TFL

      //  amt >= TFL ?
      if( TL_memcmp( (UCHAR *)&hex1, (UCHAR *)&hex2, 4 ) >= 0 )
        return( apkOK );
      else
        return( apkFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: the payment system specifies the following values, in addtional
//           to the floor limit (TFL).
//           (1) Target Percentage to be used for Random Selection. (0..99)
//           (2) Threshold Value for Biased Random Selection. (0..TFL-1)
//           (3) Max Target Percentage to be used for Biased Random Selection. (0..99)
// INPUT   : amt1 - the current transaction amount, including adjustments.
//           amt2 - the current transaction amount, excluding adjustments.
// OUTPUT  : none.
// REF     : g_selected_aid_index
// RETURN  : apkOK        (selected for online)
//           apkFailed    (not selected for online)
//           apkIncorrect (invalid parameters)
// ---------------------------------------------------------------------------
UCHAR apk_RandomTransactionSelection( UCHAR *amt1, UCHAR *amt2 )
{
UCHAR temp[6];
UCHAR ran;       // random number
UCHAR tp;        // taget percentage for RS
UCHAR mtp;       // max taget percentage for BRS
ULONG tfl;       // floor limit
ULONG amt_tx;    // transaction amount
ULONG amt_auth;  // amount, authorized
ULONG threshold; // threshold value for BRS
//ULONG ifactor;   // interpolation factor
ULONG ttp;       // transaction target percentage
ULONG data;
UCHAR maxflag;
float	n, d;
float	ifactor;


      // Generate RANDOM number (1..99)
      //
      // If AMT < BRS_THRESHOLD
      //    If RANDOM <= RS_TP, Then it is selected for online.
      //
      // If AMT >= BRS_THRESHOLD and AMT < TFL
      //    If RANDOM <= TTP, Then it is selected for online.

      maxflag = 0;

      // random number
      ran = apk_RandomNumber();

      // Transaction Amount
      TL_bcd2hex( 5, &amt1[1], (UCHAR *)&amt_tx );
      TL_SwapData( 4, (UCHAR *)&amt_tx);

      // Amount, Authorized
      TL_bcd2hex( 5, &amt2[1], (UCHAR *)&amt_auth );
      TL_SwapData( 4, (UCHAR *)&amt_auth);

      // read TFL for the selected AID
//    apk_ReadRamDataTERM( ADDR_TFL_FLAG_01+g_selected_aid_index, 1, temp );
      apk_ReadRamDataTERM( ADDR_TFL_FLAG, 1, temp );
      if( temp[0] == 0 )      // PATCH: 2003-06-08
        return( emvFailed );  // TFL not present, skip checking

//    apk_ReadRamDataTERM( ADDR_TFL_01+TFL_LEN*g_selected_aid_index, TFL_LEN, temp );
//    TL_bcd2hex( TFL_LEN-1, &temp[1], (UCHAR *)&tfl ); // PATCH: 2003-10-08

//    apk_ReadRamDataTERM( ADDR_TERM_FL, 4, (UCHAR *)&tfl );
//    TL_SwapData( 4, (UCHAR *)&tfl );
      apk_ReadRamDataTERM( ADDR_TFL, TFL_LEN, temp );	// PATCH: 2011-04-01
      TL_bcd2hex( TFL_LEN-1, &temp[1], (UCHAR *)&tfl );
      TL_SwapData( 4, (UCHAR *)&tfl );

      // read BRS_THRESHOLD value
      apk_ReadRamDataTERM( ADDR_TERM_BRS_THRESHOLD, BRS_THRESHOLD_LEN, temp );
      TL_bcd2hex( BRS_THRESHOLD_LEN-1, &temp[1], (UCHAR *)&threshold ); // PATCH: 2003-10-08
      TL_SwapData( 4, (UCHAR *)&threshold );

      // read RS_TP value
      apk_ReadRamDataTERM( ADDR_TERM_RS_TP, 1, temp );
      TL_bcd2hex( 1, temp, (UCHAR *)&data );
      TL_SwapData( 4, (UCHAR *)&data );
      tp = (UCHAR) data;

      // read BRS_MTP value
      apk_ReadRamDataTERM( ADDR_TERM_BRS_MTP, 1, temp );
      TL_bcd2hex( 1, temp, (UCHAR *)&data );
      TL_SwapData( 4, (UCHAR *)&data );
      mtp = (UCHAR) data;

      // PATCH: 2003-10-10
      // Case1: amt_auth < threshold
      if( amt_tx < threshold )
        {
        if( ran <= tp )
          return( apkOK );
        else
          return( apkFailed );
        }

      // Case2: amt_auth >= threshold
      if( amt_tx >= tfl )
        return( apkFailed );

      // Transaction Target Percent (TTP)
      ttp = mtp - tp;
      if( ttp != 0 )
        {
//      ttp *= (amt_auth - threshold);

        if( tfl == threshold )
          maxflag = 1;
        else
          {	// 2011-04-28
//        ttp = TL_ldiv( ttp, tfl-threshold );

	  n = amt_auth - threshold;
	  d = tfl - threshold;
	  ifactor = n/d;
	  ttp *= ifactor;

          ttp += tp;
          }
        }
      else
        ttp += tp;

      if( maxflag == 1 )
        return( apkOK );

      if( (ULONG)ran <= ttp )
        return( apkOK );
      else
        return( apkFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: velocity checking permits issuer to request online processing
//           after a specified number of consecutive offline transactions.
//           LCOL - Lower Consecutive Offline Limit
//           UCOL - Upper Consecutive Offline Limit
// INPUT   : none.
// OUTPUT  : none.
// REF     :
// RETURN  : apkOK        (still going offline)
//           apkTRM1      (LCOL exceeded)
//           apkTRM2      (UCOL exceeded)
//           apkTRM3      (Last Online ATC Reg = 0 )
//           apkTRM4
//           apkTRM5      (ATC missing)
//           apkIncorrect (paraemters not available)
//           apkNotReady  (data missing)
//
//           apkOutOfService    (invalid sw from GET_DATA -- shall terminate the trans.)
// ---------------------------------------------------------------------------
UCHAR apk_VelocityChecking( void )
{
UCHAR buf[16];
UINT  iLen1, iLen2;
UCHAR lcol;
UCHAR ucol;
UINT  atc=0;	// ATC
UINT  loatcr;	// last online ATC Register
UCHAR result;

      result = apkOK; // set default return value

      // check LCOL & UCOL
      apk_ReadRamDataICC( ADDR_ICC_LCOL, 2, (UCHAR *)&iLen1 );
      apk_ReadRamDataICC( ADDR_ICC_UCOL, 2, (UCHAR *)&iLen2 );
      if( (iLen1 == 0) || (iLen2 == 0) )
        return( apkIncorrect );

      apk_ReadRamDataICC( ADDR_ICC_LCOL+2, 1, (UCHAR *)&lcol );
      apk_ReadRamDataICC( ADDR_ICC_UCOL+2, 1, (UCHAR *)&ucol );

      // check ATC & Last Online ATC Register
      if( apdu_GET_DATA( 0x9f, 0x36, buf ) == apiOK ) // ATC
        {
        iLen1 = buf[1]*256 + buf[0];
        if( (iLen1 != 7) || (buf[2] != 0x9f) || (buf[3] != 0x36) ||
            (buf[7] != 0x90) || (buf[8] != 0x00) )
          {
        	result = apkTRM5; // ATC data missing	//08-dec-29 charles
//          // PATCH: PBOC2.0, 2006-02-10
////        return( apkNotReady );
//
//          // PATCH: 2006-10-19, 2CJ.002.00 - 04 get data ATC returning 90 00
//          if( (buf[iLen1] == 0x90) && (buf[iLen1+1] == 0x00) )
//            result = apkTRM5; // ATC data missing
//          else
//            {
//            // PATCH: 2006-10-02
//            if( ((buf[iLen1] == 0x6a) && (buf[iLen1+1] == 0x81)) || ((buf[iLen1] == 0x6a) && (buf[iLen1+1] == 0x88)) )
//              result = apkTRM5; // ATC data missing
//            else
//              //return( apkOutOfService ); // invalid status word
//            	result = apkTRM5; // ATC data missing	//08-dec-29 charles
//            }
          }
        else
          atc = buf[5]*256 + buf[6];
        }
      else
        return( apkNotReady );

      if( apdu_GET_DATA( 0x9f, 0x13, buf ) == apiOK ) // Last Online ATC Reg
        {
        iLen1 = buf[1]*256 + buf[0];
        if( (iLen1 != 7) || (buf[2] != 0x9f) || (buf[3] != 0x13) ||
            (buf[7] != 0x90) || (buf[8] != 0x00) )
          {
        	return( apkNotReady );//08-dec-29 charles
//          // PATCH: 2006-10-19
//          if( (buf[iLen1] == 0x90) && (buf[iLen1+1] == 0x00) )
//            return( apkNotReady );
//
//          // PATCH: 2006-10-02
//          if( ((buf[iLen1] == 0x6a) && (buf[iLen1+1] == 0x81)) || ((buf[iLen1] == 0x6a) && (buf[iLen1+1] == 0x88)) )
//            return( apkNotReady );
//          else
//            //return( apkOutOfService ); // invalid status word
//        	  return( apkNotReady );//08-dec-29 charles
          }
        else
          loatcr = buf[5]*256 + buf[6];
        }
      else
        return( apkNotReady );

      // PATCH: PBOC2.0, 2006-02-10
      if( result == apkTRM5 ) // ATC missing?
        goto VOC_NEWCARD;

      // PATCH: EMV2000, atc <= loatcr ?
      if( atc <= loatcr )
        {
        if( loatcr != 0 )
        //return( apkNotReady );
          return( apkTRM4 );
        else
          return( apkTRM1 + apkTRM2 + apkTRM3 );
        }

      result = apkOK;

// UI_ClearScreen();
// TL_DispHexWord(0,0,atc);
// TL_DispHexWord(0,5,loatcr);
// TL_DispHexByte(1,0,lcol);
// TL_DispHexByte(1,3,ucol);
// TL_DispHexWord(2,0,atc-loatcr);

      // ATC - Last_Online_ATC_Reg > LCOL ?
      if( (atc - loatcr) > (UINT)lcol )
        result = apkTRM1;

      // ATC - Last_Online_ATC_Reg > UCOL ?
      if( (atc - loatcr) > (UINT)ucol )
        result += apkTRM2;

VOC_NEWCARD:

      if( loatcr == 0 )
        result += apkTRM3;

      return( result );
}

