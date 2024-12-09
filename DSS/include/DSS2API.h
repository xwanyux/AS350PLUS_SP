#ifndef _DSS2_API_H_
#define _DSS2_API_H_
//----------------------------------------------------------------------------

#define MODE_SINGLE_APP			0x00
#define	MODE_DOUBLE_APP			0x01
#define	APID_APP1			0x00
#define	APID_APP2			0x01

/*
** ------------------------------------------------------------------ **
**                      API Prototype Declaration                     **
** ------------------------------------------------------------------ **
*/
extern  UCHAR	api_dss2_init( UCHAR mode );
extern  UCHAR	api_dss2_file( ULONG offset, ULONG length, UCHAR *data );
extern  UCHAR	api_dss2_apid( void );
extern  UCHAR	api_dss2_burn( UCHAR apid );
extern	UCHAR	api_dss2_burn2( UCHAR apid );
extern	UCHAR	api_dss2_burnSP();
extern	UCHAR	api_dss2_burnPrivateKey();
extern  UCHAR	api_dss2_burnPublicKey();
extern	UCHAR	api_dss2_run( UCHAR apid );
extern	UCHAR	*api_dss2_address( void );

//----------------------------------------------------------------------------
#endif
