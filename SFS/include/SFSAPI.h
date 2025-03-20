//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :                                                            **
//**  PRODUCT  : AS350                                                      **
//**                                                                        **
//**  FILE     : SFSAPI.H                                                    **
//**  MODULE   : Declaration of the File System APIs.			    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2023/07/31                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2023 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------

#ifndef SFSAPI_H_
	#define SFSAPI_H_
//====================================include=======================================================
#include "sfs_message.h"
#include "sfs_option.h"
#include "POSAPI.h"

//----------------------------------------------------------------------------
//		Function Prototypes
//----------------------------------------------------------------------------
UCHAR	api_sfs_init();
void	api_sfs_format();
struct	FILE_DIR *api_sfs_directory( void );
struct	FILE* api_sfs_open(char *fileName,unsigned char mode);
void	api_sfs_close(struct FILE *pf);
UCHAR	api_sfs_create(char fname[FILE_NAME_SIZE],unsigned short fileType);
UCHAR	api_sfs_delete(char fname[FILE_NAME_SIZE]);
ULONG	api_sfs_read(struct FILE *pFile,unsigned char *buff,unsigned long length);
ULONG	api_sfs_write(struct FILE *pFile,unsigned char *buff,unsigned long length);
UCHAR	api_sfs_seek(struct FILE *pFile,unsigned long position);
ULONG	api_sfs_tell(struct FILE *pFile);

#endif /*SFSAPI_H_*/
