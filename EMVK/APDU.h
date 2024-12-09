//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2                                                     **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : APDU.H                                                     **
//**  MODULE   : Declaration of related APDU functions.                     **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/08/07                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018	 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _APDU_H_
#define _APDU_H_

#include "POSAPI.h"


//----------------------------------------------------------------------------

//      APDU_SEL
extern  UCHAR apdu_SELECT( UCHAR *filename, UCHAR occurrence, UCHAR *fci );

//      APDU_REC
extern  UCHAR apdu_READ_RECORD( UCHAR sfi, UCHAR recnum, UCHAR *recdata );

//      APDU_GPO
extern  UCHAR apdu_GET_PROCESSING_OPTIONS( UCHAR *pdol, UCHAR *data );

//      APDU_SAM
extern  UCHAR apdu_SAM_LOAD_EMVL2_CAPK( void );
extern  UCHAR apdu_SAM_DEL_PUBLIC_KEY( UINT fid );
extern  UCHAR apdu_SAM_GET_RESPONSE( UCHAR len, UCHAR *data );
extern  UCHAR apdu_SAM_GET_PUBLIC_KEY( UCHAR pki, UCHAR *rid, UCHAR *pkm, UCHAR *pke );
extern  UCHAR apdu_SAM_LOAD_EXTERNAL_PK( UINT slot, UCHAR *modulus, UCHAR *exponent );
extern  UCHAR apdu_SAM_RSA_RECOVER( UINT slot, UCHAR *pkc );
extern  UCHAR apdu_SAM_HASH( UCHAR algorithm, UINT length, UCHAR *data, UCHAR *digest );
extern  UCHAR apdu_SAM_GET_CHALLENGE( UCHAR *random );
extern  UCHAR apdu_SAM_SELECT( UCHAR *filename, UCHAR occurrence, UCHAR *fci );
extern  UCHAR apdu_SAM_CLEAN( UCHAR *response );

//      APDU_IAU
extern  UCHAR apdu_INTERNAL_AUTHENTICATE( UCHAR *ddol, UCHAR *data );

//      APDU_GDT
extern  UCHAR apdu_GET_DATA( UCHAR tag1, UCHAR tag2, UCHAR *dataobj );

//      APDU_VER
extern  UCHAR apdu_VERIFY( UCHAR qualifier, UCHAR length, UCHAR *pindata, UCHAR *response );

//      APDU_GCH
extern  UCHAR apdu_GET_CHALLENGE( UCHAR *response );

//      APDU_GAC
extern  UCHAR apdu_GENERATE_AC( UCHAR qualifier, UCHAR *cdol, UCHAR *ac );

//      APDU_XAU
extern  UCHAR apdu_EXTERNAL_AUTHENTICATE( UCHAR *iad, UCHAR *response );

//      APDU_ISC
extern  UCHAR apdu_ISSUER_SCRIPT_COMMAND( UINT len, UCHAR *cmd, UCHAR *response );

//----------------------------------------------------------------------------
#endif
