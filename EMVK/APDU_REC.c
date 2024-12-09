//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : APDU_REC.C                                                 **
//**  MODULE   : apdu_READ_RECORD()                                         **
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
//#include <EMVDC.h>
#include "GDATAEX.h"
//#include <EMVAPI.h>

// ---------------------------------------------------------------------------
// FUNCTION: The READ RECORD command reads a file record in alinear file.
// INPUT   : sfi    - short file id. (1..30)
//           recnum - record number.
// OUTPUT  : recdata - 2L-V, the record data read.
// REF     : g_dhn_icc
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR apdu_READ_RECORD( UCHAR sfi, UCHAR recnum, UCHAR *recdata )
{
UCHAR c_apdu[7];

      // --- DEBUG ONLY ---
#ifdef  L2_SW_DEBUG

//UCHAR rapdu[]={ 0x70, 0x1b, 0x61, 0x19, 0x4f, 0x07, 0xa0, 0x00, 0x00,
//                0x00, 0x03, 0x10, 0x10, 0x87, 0x01, 0x01, 0x50, 0x0b,
//                0x56, 0x49, 0x53, 0x41, 0x20, 0x43, 0x52, 0x45, 0x44, 0x49, 0x54, 0x90, 0x00};

UCHAR rapdu[]={ 0x70, 0x3E,

                0x61, 0x19,
                0x4f, 0x07, 0xa0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10,
                0x87, 0x01, 0x01,
                0x50, 0x0b, 0x56, 0x49, 0x53, 0x41, 0x20, 0x43, 0x52, 0x45, 0x44, 0x49, 0x54,

                0x61, 0x21,
                0x4f, 0x07, 0xa0, 0x00, 0x00, 0x00, 0x03, 0x20, 0x10,
                0x87, 0x01, 0x02,
                0x50, 0x08, 0x45, 0x4c, 0x45, 0x43, 0x54, 0x52, 0x4f, 0x4e,
                0x9f, 0x12, 0x08, 'E' , 'L' , 'E' , 'C' , 'T' , 'R' , 'O' , 'N' ,

                0x90, 0x00 };

//UCHAR rapdu[]={ 0x70, 0x54,
//
//                0x61, 0x19,
//                0x4f, 0x07, 0xa0, 0x00, 0x00, 0x00, 0x03, 0x10, 0x10,
//                0x87, 0x01, 0x01,
//                0x50, 0x0b, 0x56, 0x49, 0x53, 0x41, 0x20, 0x43, 0x52, 0x45, 0x44, 0x49, 0x54,
//
//                0x61, 0x14,
//                0x9d, 0x08, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
//                0x73, 0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
//
//                0x61, 0x21,
//                0x4f, 0x07, 0xa0, 0x00, 0x00, 0x00, 0x03, 0x20, 0x10,
//                0x87, 0x01, 0x02,
//                0x50, 0x08, 0x45, 0x4c, 0x45, 0x43, 0x54, 0x52, 0x4f, 0x4e,
//                0x9f, 0x12, 0x08, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
//
//                0x90, 0x00 };

UCHAR rapdu_rec1[]={ 0x70, 0x3C,

                0x57, 0x10, 0x45, 0x63, 0x01, 0x85, 0x02, 0x75, 0x90, 0x15,
                            0xd0, 0x41, 0x22, 0x01, 0x16, 0x38, 0x58, 0x00,
                0x5f, 0x20, 0x0c, 0x4c, 0x45, 0x45, 0x20, 0x59, 0x45, 0x45,
                                  0x20, 0x43, 0x48, 0x49, 0x41,
                0x9f, 0x1f, 0x18, 0x31, 0x36, 0x33, 0x38, 0x35, 0x39, 0x39,
                                  0x39, 0x39, 0x39, 0x39, 0x39, 0x39, 0x30,
                                  0x30, 0x30, 0x32, 0x32, 0x30, 0x30, 0x30,
                                  0x30, 0x30, 0x30,
                0x90, 0x00 };

UCHAR rapdu_rec2[]={ 0x70, 0x05,

                0x9f, 0x07, 0x02, 0xff, 0x00,
                0x90, 0x00 };

UCHAR rapdu_rec3[]={ 0x70, 0x17,

                0x8c, 0x15, 0x9f, 0x02, 0x06,
                            0x9f, 0x03, 0x06,
                            0x9f, 0x1a, 0x02,
                            0x95, 0x05,
                            0x5f, 0x2a, 0x02,
                            0x9a, 0x03,
                            0x9c, 0x01,
                            0x9f, 0x37, 0x04,
                0x90, 0x00 };

UCHAR rapdu_rec4[]={ 0x70, 0x19,

                0x8d, 0x17, 0x8a, 0x02,
                            0x9f, 0x02, 0x06,
                            0x9f, 0x03, 0x06,
                            0x9f, 0x1a, 0x02,
                            0x95, 0x05,
                            0x5f, 0x2a, 0x02,
                            0x9a, 0x03,
                            0x9c, 0x01,
                            0x9f, 0x37, 0x04,
                0x90, 0x00 };

UCHAR rapdu_rec5[]={ 0x70, 0x4c,

                0x5a, 0x08, 0x45, 0x63, 0x01, 0x85, 0x02, 0x75, 0x90, 0x15,
                            0x5f, 0x34, 0x01, 0x00,
                            0x5f, 0x24, 0x03, 0x04, 0x12, 0x31,
                            0x5f, 0x25, 0x03, 0x02, 0x01, 0x01,
                            0x9f, 0x08, 0x02, 0x00, 0x84,
                            0x9f, 0x0d, 0x05, 0x10, 0x40, 0x00, 0x88, 0x00,
                            0x9f, 0x0e, 0x05, 0x00, 0x10, 0x00, 0x00, 0x00,
                            0x9f, 0x0f, 0x05, 0x10, 0x40, 0x00, 0x98, 0x00,
                            0x8e, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x1e, 0x03, 0x02, 0x03,
                                        0x1f, 0x00,
                            0x5f, 0x28, 0x02, 0x01, 0x58,
                0x90, 0x00 };

      if( g_test_read_app_data != 1 ) // read application data?
        goto READ_REC_1000;

      if( recnum == 1 )
        {
        recdata[0] = 64;
        recdata[1] = 0;
        memmove( &recdata[2], rapdu_rec1, 64 );
        return( 0 );
        }
      if( recnum == 2 )
        {
        recdata[0] = 9;
        recdata[1] = 0;
        memmove( &recdata[2], rapdu_rec2, 9 );
        return( 0 );
        }
      if( recnum == 3 )
        {
        recdata[0] = 27;
        recdata[1] = 0;
        memmove( &recdata[2], rapdu_rec3, 27 );
        return( 0 );
        }
      if( recnum == 4 )
        {
        recdata[0] = 29;
        recdata[1] = 0;
        memmove( &recdata[2], rapdu_rec4, 29 );
        return( 0 );
        }
      if( recnum == 5 )
        {
        recdata[0] = 80;
        recdata[1] = 0;
        memmove( &recdata[2], rapdu_rec5, 80 );
        return( 0 );
        }
      return( 1 );

READ_REC_1000:

      if(recnum >= 2)
        {
        recdata[0] = 2;
        recdata[1] = 0;

        recdata[2] = 0x6A; // SW1
        recdata[3] = 0x83; // SW1

        return( 0 );
        }

//    recdata[0] = 31;
      recdata[0] = 55+11;
//    recdata[0] = 55+11+22;
      recdata[1] = 0;

      memmove( &recdata[2], rapdu, 55+11 );
      return( 0 );

#endif

      // ------------------
//    if( g_test_flag == 0 )
//      {
//      recdata[0] = 46;
//      recdata[1] = 0;
//      recdata[2] = 0x70;
//      recdata[3] = 0x2A;
//      recdata[4] = 0x61;
//      recdata[5] = 0x28;
//      recdata[6] = 0x4F;
//      recdata[7] = 0x07;
//      recdata[8] = 0xA0;
//      recdata[9] = 0x00;
//      recdata[10] = 0x00;
//      recdata[11] = 0x00;
//      recdata[12] = 0x03;
//      recdata[13] = 0x10;
//      recdata[14] = 0x10;
//      recdata[15] = 0x50;
//      recdata[16] = 0x0A;
//      recdata[17] = 0x56;
//      recdata[18] = 0x49;
//      recdata[19] = 0x53;
//      recdata[20] = 0x41;
//      recdata[21] = 0x43;
//      recdata[22] = 0x52;
//      recdata[23] = 0x45;
//      recdata[24] = 0x44;
//      recdata[25] = 0x49;
//      recdata[26] = 0x54;
//      recdata[27] = 0x87;
//      recdata[28] = 0x01;
//      recdata[29] = 0x01;
//      recdata[30] = 0x9F;
//      recdata[31] = 0x12;
//      recdata[32] = 0x0D;
//      recdata[33] = 0x43;
//      recdata[34] = 0x52;
//      recdata[35] = 0x45;
//      recdata[36] = 0x44;
//      recdata[37] = 0x49;
//      recdata[38] = 0x54;
//      recdata[39] = 0x4F;
//      recdata[40] = 0x44;
//      recdata[41] = 0x45;
//      recdata[42] = 0x56;
//      recdata[43] = 0x49;
//      recdata[44] = 0x53;
//      recdata[45] = 0x41;
//      recdata[46] = 0x90;
//      recdata[47] = 0x00;
//
//      g_test_flag++;
//      return( apiOK );
//      }
//
//    if( g_test_flag == 1 )
//      {
//      recdata[0] = 2;
//      recdata[1] = 0;
//      recdata[2] = 0x6a;
//      recdata[3] = 0x83;
//
//      g_test_flag++;
//      return( apiOK );
//      }
//
//
//    if( g_test_flag == 2 )
//      {
//      recdata[0] = 2;
//      recdata[1] = 0;
//
//      g_test_flag++;
//      return( apiOK );
//      }

// -------------------------
//    if( g_test_flag == 0 )
//      {
//      recdata[0] = 13;
//      recdata[1] = 0;
//      recdata[2] = 0x70;
//      recdata[3] = 0x09;
//      recdata[4] = 0x61;
//      recdata[5] = 0x07;
//      recdata[6] = 0x9D;
//      recdata[7] = 0x05;
//      recdata[8] = 0xA0;
//      recdata[9] = 0x00;
//      recdata[10] = 0x11;
//      recdata[11] = 0x11;
//      recdata[12] = 0x11;
//      recdata[13] = 0x90;
//      recdata[14] = 0x00;
//
//      g_test_flag++;
//      return( apiOK );
//      }
//
//    if( g_test_flag == 1 )
//      {
//      recdata[0] = 46;
//      recdata[1] = 0;
//      recdata[2] = 0x70;
//      recdata[3] = 0x2A;
//      recdata[4] = 0x61;
//      recdata[5] = 0x28;
//      recdata[6] = 0x4F;
//      recdata[7] = 0x07;
//      recdata[8] = 0xA0;
//      recdata[9] = 0x00;
//      recdata[10] = 0x00;
//      recdata[11] = 0x00;
//      recdata[12] = 0x03;
//      recdata[13] = 0x10;
//      recdata[14] = 0x10;
//      recdata[15] = 0x50;
//      recdata[16] = 0x0A;
//      recdata[17] = 0x56;
//      recdata[18] = 0x49;
//      recdata[19] = 0x53;
//      recdata[20] = 0x41;
//      recdata[21] = 0x43;
//      recdata[22] = 0x52;
//      recdata[23] = 0x45;
//      recdata[24] = 0x44;
//      recdata[25] = 0x49;
//      recdata[26] = 0x54;
//      recdata[27] = 0x87;
//      recdata[28] = 0x01;
//      recdata[29] = 0x01;
//      recdata[30] = 0x9F;
//      recdata[31] = 0x12;
//      recdata[32] = 0x0D;
//      recdata[33] = 0x43;
//      recdata[34] = 0x52;
//      recdata[35] = 0x45;
//      recdata[36] = 0x44;
//      recdata[37] = 0x49;
//      recdata[38] = 0x54;
//      recdata[39] = 0x4F;
//      recdata[40] = 0x44;
//      recdata[41] = 0x45;
//      recdata[42] = 0x56;
//      recdata[43] = 0x49;
//      recdata[44] = 0x53;
//      recdata[45] = 0x41;
//      recdata[46] = 0x90;
//      recdata[47] = 0x00;
//
//      g_test_flag++;
//      return( apiOK );
//      }
//
//
//    if( g_test_flag == 2 )
//      {
//      recdata[0] = 2;
//      recdata[1] = 0;
//      recdata[2] = 0x6a;
//      recdata[3] = 0x83;
//
//      g_test_flag++;
//      return( apiOK );
//      }

// ----------------------------------------
//    if( (sfi == 1) && (recnum == 2) )
//      {
//      recdata[0] = 2;
//      recdata[1] = 0;
//      recdata[2] = 0x6a;
//      recdata[3] = 0x82;
//
//      memset( recdata, 0x00, 86 );
//      recdata[0] = 84;
//      recdata[1] = 0;
//
//      recdata[2] = 0x70;
//      recdata[3] = 0x50;
//      recdata[4] = 0x5a;
//      recdata[5] = 0x08;
//      recdata[6] = 0x47;
//      recdata[7] = 0x61;
//      recdata[8] = 0x73;
//      recdata[9] = 0x90;
//      recdata[10] = 0x01;
//      recdata[11] = 0x01;
//      recdata[12] = 0x00;
//      recdata[13] = 0x10;
//      recdata[14] = 0x5f;
//      recdata[15] = 0x34;
//      recdata[16] = 0x01;
//      recdata[17] = 0x01;
//
//      recdata[84] = 0x90;
//      recdata[85] = 0x00;
//
//      return( apiOK );
//      }


      // ------------------

      c_apdu[0] = 0x05;                 // length of APDU
      c_apdu[1] = 0x00;                 //
      c_apdu[2] = 0x00;                     // CLA
      c_apdu[3] = 0xb2;                     // INS
      c_apdu[4] = recnum;                   // P1
      c_apdu[5] = (sfi << 3) | 0x04;        // P2 = reference control parameter
      c_apdu[6] = 0x00;                     // Le

//    *recdata = 0x2C;		// allocate max storage 300 bytes for SC response
//    *(recdata+1) = 0x01;	//
      return( api_ifm_exchangeAPDU( g_dhn_icc, c_apdu, recdata ) );
}

