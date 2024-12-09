//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :                                                            **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : FTP.H     	          	                            **
//**  MODULE   : Declaration of FTP APIs.		                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/09/17                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2009-2011 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _FTPAPI_H_
#define _FTPAPI_H_




//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
//client
extern	UCHAR   api_ftp_ni( UCHAR ni ) ;
extern 	UCHAR	api_ftp_open( UCHAR *serverip , UCHAR *serverport,UCHAR  SSLOnOff );
extern 	UCHAR	api_ftp_login( UCHAR *user, UCHAR *password );
extern 	UCHAR	api_ftp_binary( void );
extern 	UCHAR	api_ftp_ascii( void);
extern	UCHAR	api_ftp_close( void );
extern	UCHAR	api_ftp_pwd( void  );
extern	UCHAR	api_ftp_ls( UINT bufsize, ULONG *FileSize, UCHAR *pFile);
extern	UCHAR	api_ftp_dir( UINT bufsize, ULONG *FileSize, UCHAR *pFile);
extern	UCHAR	api_ftp_cd( UCHAR *cd );
extern	UCHAR	*api_ftp_get( UCHAR *FileName, ULONG *FileSize );
extern	UCHAR	api_ftp_pbsz( void );
extern	UCHAR	api_ftp_prot( void );
extern	UCHAR	api_ftp_GetReply( UCHAR *readbuf);




//----------------------------------------------------------------------------
#endif
