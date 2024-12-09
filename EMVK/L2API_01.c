
//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : L2API_01.C                                                 **
//**  MODULE   : api_emv_OpenSession()                                      **
//**             api_emv_CardPresent()                                      **
//**             api_emv_CloseSession()                                     **
//**             api_emv_Deactivate()                                       **
//**             api_emv_ATR()                                              **
//**             api_emv_SelectSAM()                                        **
//**             api_emv_CleanSAM()                                         **
//**             api_emv_RetrievePublicKeyCA()                              **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2002/12/05                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2002-2007 SymLink Corporation. All rights reserved.      **
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
#include "PEDAPI.h"
//#include <APDU.H>

//#include "CAPK.H"	// CAPKV41E.H (BCTC): for EMV L2 & PBOC L2


// ---------------------------------------------------------------------------
// FUNCTION: Open ICC transaction session.
// INPUT   : slot - target device.
// OUTPUT  : none.
// RETURN  : device handle number.
// ---------------------------------------------------------------------------
UCHAR api_emv_OpenSession(UCHAR slot)
{
      return( api_ifm_open(slot) );
}

// ---------------------------------------------------------------------------
// FUNCTION: Check presence of ICC.
// INPUT   : ICC device handle number.
// OUTPUT  : none.
// REF     : g_dhn_kbd
// RETURN  : emvReady
//           emvNotReady
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR api_emv_CardPresent( UCHAR dhn )
{
UCHAR	status;

	status = api_ifm_present(dhn);
	
	if( status == emvReady )
	  BSP_Delay_n_ms( 100 );
	  
	return( status );

//	return( api_ifm_present(dhn) );

}

// ---------------------------------------------------------------------------
// FUNCTION: Close ICC transaction session.
// INPUT   : dhn - ICC device handle number.
// OUTPUT  : none.
// REF     : none.
// RETURN  : emvReady
//           emvFailed
// ---------------------------------------------------------------------------
void api_emv_CloseSession( UCHAR dhn )
{
      // close device
      api_ifm_close( dhn );

      // clear all related data
      apk_EmptyCandidateList();
}

// ---------------------------------------------------------------------------
// FUNCTION: Depower all ICC electric contacts.
// INPUT   : dhn - ICC device handle number.
// OUTPUT  : none.
// REF     : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void api_emv_Deactivate( UCHAR dhn )
{
      api_ifm_deactivate( dhn );
}

// ---------------------------------------------------------------------------
// FUNCTION: Answer To Reset.
// INPUT   : dhn - ICC device handle number.
// OUTPUT  : atr - contents of ATR.
// REF     : none.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
UCHAR api_emv_ATR( UCHAR dhn, UCHAR *atr )
{
      return( api_ifm_reset( dhn, 0, atr ) );
}

// ---------------------------------------------------------------------------
// FUNCTION: Answer To Reset.
// INPUT   : dhn - SAM device handle number.
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
#ifdef	USE_RSA_SAM

UCHAR api_emv_SelectSAM( void )
{
      if( apk_SelectSAM() == apkOK )
        return( emvOK );
      else
        return( emvFailed );
}

#endif

// ---------------------------------------------------------------------------
// FUNCTION: clear up OCS-SAM to avoid from error code 6611.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
#ifdef	USE_RSA_SAM

UCHAR api_emv_CleanSAM( void )
{
      if( apk_CleanSAM() == apkOK )
        return( emvOK );
      else
        return( emvFailed );
}

#endif

// ---------------------------------------------------------------------------
// FUNCTION: retrieve CA Public Key Header to main memory for better performance.
// INPUT   : none.
// OUTPUT  : none.
// REF     : g_capk_cnt
// RETURN  : emvOK
//           emvFailed
// ---------------------------------------------------------------------------
#if	1
UCHAR api_emv_RetrievePublicKeyCA( void )
{
#ifdef	USE_RSA_SAM

UCHAR i, j;
UCHAR c_apdu[16];
UCHAR r_apdu[64];
UINT  iLen;
UINT  iLen2;
UINT  iFID;

      g_capk_cnt = 0;

      iFID = KEY_FID_01; // initial key file id
      for( i=0; i<MAX_KEY_SLOT_NUM; i++ )
         {
         // select public key (Select_Pub)
         c_apdu[0] = 0x07;                 // length of APDU
         c_apdu[1] = 0x00;                 //

         c_apdu[2] = 0x90;                     // CLA
         c_apdu[3] = 0xE3;                     // INS
         c_apdu[4] = 0x00;                     // P1
         c_apdu[5] = 0x00;                     // P2
         c_apdu[6] = 0x02;                     // Lc
         c_apdu[7] = (iFID & 0xff00) >> 8;     // Data=FID
         c_apdu[8] = iFID & 0x00ff;            //
         if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
           return( emvFailed );
         iLen = r_apdu[1]*256 + r_apdu[0];
         if( (iLen != 2) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
           return( emvFailed );

//       UI_WaitKey();

         // retrieve the selected public key header (RID+PKI+EXP_LEN+MOD_LEN+SHA1)
         // (Set_Pub)
         c_apdu[0] = 0x05;                 // length of APDU
         c_apdu[1] = 0x00;                 //

         c_apdu[2] = 0x90;                     // CLA
         c_apdu[3] = 0x34;                     // INS
         c_apdu[4] = 0x01;                     // P1=OP
         c_apdu[5] = 0x00;                     // P2
         c_apdu[6] = 29;                       // Le
         if( api_ifm_exchangeAPDU( g_dhn_sam, c_apdu, r_apdu ) != apiOK )
           return( emvFailed );
         iLen = r_apdu[1]*256 + r_apdu[0];
         if( (iLen != 31) || (r_apdu[iLen] != 0x90) || (r_apdu[iLen+1] != 0x00) )
           return( emvFailed );

         // check RID[0..4] = 0s
         for( j=0; j<5; j++ )
            {
            if( r_apdu[2+j] != 0 )
              break;
            }
         if( j >= 5 )
           return( emvOK ); // end of process

         // save public key header
         apk_WriteRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*i, CA_PK_HEADER_LEN, &r_apdu[2] );

         iFID++;
         g_capk_cnt++;

//       TL_DispHexWord(0,0,iFID);
//       UI_WaitKey();
         }

//    for(i=0; i<9; i++)
//       {
//       apk_ReadRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*i, CA_PK_HEADER_LEN, &r_apdu[2] );
//       TL_DumpHexData(0,0,6, &r_apdu[2]);
//       }

      return( emvOK );
#endif

#ifndef	USE_RSA_SAM
ULONG	i;
ULONG	j;
ULONG 	fEOF;	// end-of-file flag
UCHAR	pkh[63];


	// --- PCI ONLY ---

	g_capk_cnt = 0;
	
	for( i=0; i<MAX_CA_PK_CNT; i++ )
	   {
	   if( api_ped_GetKeyHeader_CAPK( i, pkh ) == apiOK )
	     {
	     // save public key header
	     apk_WriteRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*i, CA_PK_HEADER_LEN, pkh );
	     g_capk_cnt++;
	     }
	   else
	     break;
	   }

	if( g_capk_cnt )
	  return( emvOK );
	else
	  return( emvFailed );	// CAPK not found
	

//	// --- EMV L2 TEST ONLY ---
//	g_capk_cnt = 0;
//	
//	// Copy CAPKs to SRAM for succeeding access
//	for( i=0; i<MAX_CA_PK_CNT; i++ )
//	   {
//	   fEOF = TRUE;
//	   
//	   // RID(5) = 0?
//	   for( j=0; j<5; j++ )
//	      {
//	      if( CA_PublicKey[i][j] != 0 )
//	        {
//	        fEOF = FALSE;
//	        break;
//	        }
//	      }
//	     
//	   apk_WriteRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*i, CA_PK_LEN, (UCHAR *)&CA_PublicKey[i][0] );
//	   g_capk_cnt++;
//	   
//	   if( fEOF == TRUE )
//	     break;	// done
//	   }
//	
//	// --- Init Fixed PED Key --- (EMV TA ONLY)
////	api_ped_EMVL2_InitFixedKey();
//	
//	return( emvOK );
#endif
}
#endif

// ---------------------------------------------------------------------------
// FUNC  : To get the info of CAPK's in SAM. (fixed 60 bytes info) -> 80 bytes
// INPUT : info - CardScheme0[1]   : 00         RID[5] : 00~04
//                IndexNo0[1]      : 01                : 05
//                Index0[10]       : 02~11             : 06~15
//
//                CardScheme1[1]   : 12         RID[5] : 16~20
//                IndexNo1[1]      : 13                : 21
//                Index1[10]       : 14~23             : 22~31
//
//                CardScheme1[1]   : 24         RID[5] : 32~36
//                IndexNo1[1]      : 25                : 37
//                Index1[10]       : 26~35             : 38~47
//                ..............
//                0xFF
//
//                CardScheme: 0x00=VSDC, 0x01=MCHIP, 0x02=JSMART, 0xFF=end of data.
//                IndexNo   : Number of key index
//                Index     : Key index
// OLD:    sizeof(info) = 60 bytes.  (12*5 payment system)
// NEW:    sizeof(info) = 160 bytes. (16*10 payment system)
//
// OUTPUT: none.
// RETURN: none.
// NOTE  : call this function after api_emvk_InitEMVKernel().
// ---------------------------------------------------------------------------
void  api_emv_GetPublicKeyInfo( UCHAR *info )
{
UCHAR i,j;
UCHAR cnt;
UCHAR buf[CA_PK_HEADER_LEN];


      memset( info, 0x00, 160 );
      for( i=0; i<MAX_AID_CNT; i++ )
         info[16*i] = 0xff;


//    memset( buf, 0x00, CA_PK_HEADER_LEN );
//    buf[0] = 0xa0;
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00;
//    buf[4] = 0x03;
//    buf[5] = 0x11;
//    apk_WriteRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*0, CA_PK_HEADER_LEN, buf );
//
//    memset( buf, 0x00, CA_PK_HEADER_LEN );
//    buf[0] = 0xa0;
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00;
//    buf[4] = 0x04;
//    buf[5] = 0x22;
//    apk_WriteRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*1, CA_PK_HEADER_LEN, buf );
//
//    memset( buf, 0x00, CA_PK_HEADER_LEN );
//    buf[0] = 0xa0;
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00;
//    buf[4] = 0x04;
//    buf[5] = 0x33;
//    apk_WriteRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*2, CA_PK_HEADER_LEN, buf );
//
//    memset( buf, 0x00, CA_PK_HEADER_LEN );
//    buf[0] = 0x00;
//    buf[1] = 0x00;
//    buf[2] = 0x00;
//    buf[3] = 0x00;
//    buf[4] = 0x00;
//    buf[5] = 0x00;
//    apk_WriteRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*3, CA_PK_HEADER_LEN, buf );


      for( i=0; i<MAX_KEY_SLOT_NUM; i++ )
         {
         apk_ReadRamDataKEY( ADDR_CA_PK_01+CA_PK_LEN*i, CA_PK_HEADER_LEN, buf );
         if( TL_memcmpc( buf, 0x00, RID_LEN ) != 0 ) // RID=0's ?
           {
           // compare RID
           for( j=0; j<MAX_AID_CNT; j++ )
              {
              if( TL_memcmp( buf, &info[16*j], 5 ) == 0 )
                {
                cnt = info[16*j+5];
                info[16*j+5] += 1; // IndexNo+1
                info[16*j+cnt+6] = buf[5]; // index

                break;
                }
              else
                {
                if( info[16*j] == 0xFF )
                  {
                  memmove( &info[16*j], buf, 5 ); // setup new RID

                  cnt = info[16*j+5];
                  info[16*j+5] += 1; // IndexNo+1
                  info[16*j+cnt+6] = buf[5]; // index

                  break;
                  }
                }
              }
           }
         else
           break;
         }

//SHOWIT:
//   for( i=0; i<10; i++ )
//      {
//      TL_DispHexByte(0,0,i);
//      TL_DumpHexData(0,1,16, &info[i*16]);
//      }
//   goto SHOWIT;

}

// ---------------------------------------------------------------------------
