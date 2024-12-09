//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : DEV_MSR.H                                                  **
//**  MODULE   : Declaration of MSR Module.		                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2007/08/21                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2007 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _DEV_MSR_H_
#define _DEV_MSR_H_

//----------------------------------------------------------------------------
#include "bsp_types.h"
#include "POSAPI.h"

#define	MAX_TRK1_CHAR_CNT			79
#define	MAX_TRK2_CHAR_CNT			40
#define	MAX_TRK3_CHAR_CNT			107

#define	ISO_TRACK1				0x01
#define	ISO_TRACK2				0x02
#define	ISO_TRACK3				0x04

#define	TRK_STATUS_NO_SWIPED	    0x00
#define	TRK_STATUS_SWIPED			0x01

#define	TRK_STATUS_NO_DATA			0x00
#define	TRK_STATUS_DATA_OK			0x01
#define	TRK_STATUS_DATA_ERROR		0x02

#define	ISO_TRK1_ST_SENTINEL			0x05
#define	ISO_TRK1_END_SENTINEL			0x1F
#define	ISO_TRK2_ST_SENTINEL			0x0B
#define	ISO_TRK2_END_SENTINEL			0x0F
#define	ISO_TRK3_ST_SENTINEL			0x0B
#define	ISO_TRK3_END_SENTINEL			0x0F


#define ISO_TRK1_DECODE_BLOCK_LEN      0x07
#define ISO_TRK2_DECODE_BLOCK_LEN      0x05
#define ISO_TRK3_DECODE_BLOCK_LEN      0x05



#define BigEndian 1
#define SamllEndian 0 // Small low byte first address

#define Odd 1
#define Even 0

// define a data sturture for using decode track

typedef struct track_decode_information_S{

    UCHAR start_symbol_hex_value; // without pairty
    UCHAR end_symbol_hex_value; // without parity
    UCHAR decode_block_len; // include parity bits
    UCHAR* HexToCharTable;
    UCHAR* TrackDecodeBuffer; 

}track_decode_information;

// private function
unsigned int BinaryToHex(char* array, int len, int encodingRule);
UCHAR CheckParity(char* array, int len, int mode);
void reverse(UCHAR* track, UINT len);
UCHAR decode_track1(UCHAR* binary_soruce, UINT source_len);
UCHAR decode_track2(UCHAR* binary_soruce, UINT source_len);
UCHAR decode_track3(UCHAR* binary_soruce, UINT source_len);
UCHAR reverse_and_non_reverse_decode(track_decode_information* track_info,
					UCHAR* binary_soruce, UINT source_len);
UCHAR decode_track(track_decode_information* track_info,
					UCHAR* binary_soruce, UINT source_len);

void toggleData(UCHAR* track, UINT len);
void clear_buffer();
void init_msr();





//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern UCHAR OS_MSR_Open( UINT8 tracks );
extern UINT8 OS_MSR_Close();
extern void OS_MSR_Status( UINT8 action, UINT8 *dbuf );
extern UINT8 OS_MSR_Read( UINT8 action, UINT8 TrackNo, UINT8 *dbuf );

// ---------------------------------------------------------------------------
#endif
