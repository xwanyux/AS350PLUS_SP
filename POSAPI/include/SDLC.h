//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : SDLC.h                                                     **
//**  MODULE   : Declaration of related modem APIs used in SDLC protocol    ** 
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2008/08/21                                                 **
//**  EDITOR   : Richard Hu                                                 **
//**                                                                        **
//**  Copyright(C) 2008 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================


#ifndef SDLC_H_
#define SDLC_H_

//#include "modem_api.h"
#include "POSAPI.h"

/*
 * Specify the frame type
 */ 

#define SDLC_I             0x00
#define SDLC_S             0x01
#define SDLC_U             0x03

/*
 * S-format frame types
 */   
//#define SDLC_S_FTYPE_MASK  0x0C
#define SDLC_RR            0x00
#define SDLC_RNR           0x04
#define SDLC_REJ           0x08

/*
 * U-format modifier
 */

//#define SDLC_U_MODIFIER_MASK   0xEC
#define SDLC_UI                0X00
#define SDLC_UP                0x20
#define SDLC_DISC              0x40
#define SDLC_RD                0x40
#define SDLC_UA                0x60
#define SDLC_SNRM              0x80
#define SDLC_TEST              0xE0
#define SDLC_SIM               0x04
#define SDLC_RIM               0x04
#define SDLC_FRMR              0x84
#define SDLC_CFGR              0xC4
#define SDLC_XID               0xAC
#define SDLC_BCN               0xEC

/*
 * Define the structure for the packet
 */

struct UFrameHeader
{
  UCHAR address;
  UCHAR type:2;
  UCHAR code2:2;
  UCHAR PF:1;
  UCHAR code1:3; 
}__attribute__((packed));;

struct SFrameHeader
{
  UCHAR address;
  UCHAR type:2;
  UCHAR code:2;
  UCHAR PF:1;
  UCHAR NR:3; 
}__attribute__((packed));;

struct IFrameHeader
{
  UCHAR address;
  UCHAR type:1;
  UCHAR NS:3;
  UCHAR PF:1;
  UCHAR NR:3; 
}__attribute__((packed));;

union Header
{
  struct UFrameHeader* UHeader;
  struct SFrameHeader* SHeader;
  struct IFrameHeader* IHeader;
};

struct Tailer
{
  UCHAR EM;
  UCHAR Flag;
};


struct Packet
{ 
  unsigned int length;
  union Header packetHead;
  UCHAR *Data;
  struct Tailer *packetTail;  
};


extern void SdlcRxDataRecombine( UCHAR *, unsigned int );
extern void SdlcRxSyncConversion( UCHAR * , unsigned int );
extern void SdlcRXPacket (UCHAR *, unsigned int );
extern void SdlcRxRRResponse( struct Packet * );
extern void SdlcRxIframeResponse( struct Packet * );
extern void SdlcTxRRSend();
extern void SdlcTxUASend();
extern void SdlcTxIframeSend();
extern void SdlcTxSyncConversion( UCHAR *, unsigned int );
/*
void SdlcRxSyncConversion (UCHAR * , unsigned int );
void SdlcRXPacket (UCHAR * , unsigned int );
*/


#endif /*SDLC_H_*/
