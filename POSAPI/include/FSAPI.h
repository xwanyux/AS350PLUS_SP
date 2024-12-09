//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :                                                            **
//**  PRODUCT  : AS350                                                      **
//**                                                                        **
//**  FILE     : FSAPI.H                                                    **
//**  MODULE   : Declaration of the File System APIs.			    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2008/06/11                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2008-2015 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _FS_API_H_
#define _FS_API_H_

#include "POSAPI.h"

//----------------------------------------------------------------------------
struct	FILE_INFO
	{
	char		fname[16];
	unsigned short	mode;
	unsigned short	iHead;
	unsigned short	iTail;
	unsigned short	total_cluster;
	unsigned short	tail_cluster_unuse_byte;
	unsigned short	reserved;
	unsigned long	hash[5];
	}__attribute__((packed));

struct	FILE
	{
	unsigned long	status;
	char		fname[16];
	unsigned short	cluster_count;
	unsigned short	FS_type;
	unsigned long	length;			// file size in bytes
	struct 	 FILE_INFO	*pDIR;
	struct 	 FILE_INFO	*pFIB;
	unsigned long	*pIndex;
	unsigned long	logic_position;
	struct FILE	*next;
	};

struct	FILE_DIR
	{
	char		FileName[50][16];	// file name list, max 50 files, 16 characters per file name
	unsigned long	FileSize[50];		// file size in bytes
	unsigned long	TotalFiles;		// total files in the directory
	unsigned long	UsedMemorySize;		// used memory size of the file system in K bytes
	unsigned long	UnusedMemorySize;	// unused memory size of the file system in K bytes
	};

//----------------------------------------------------------------------------
//		Function Prototypes
//----------------------------------------------------------------------------
extern	UCHAR	api_fs_init( void );
extern	void	api_fs_format( void );
extern	struct	FILE *api_fs_open( char *fileName, UCHAR mode );
extern	void	api_fs_close( struct FILE *pf );
extern	UCHAR	api_fs_create( char fname[], unsigned short fileType );
extern	UCHAR	api_fs_delete( char fname[] );
extern	ULONG	api_fs_read( struct FILE *pFile, UCHAR *buff, ULONG length );
extern	ULONG	api_fs_write( struct FILE *pFile, UCHAR *buff, ULONG length );
extern	UCHAR	api_fs_seek( struct FILE *pFile, ULONG position );
extern	ULONG	api_fs_tell( struct FILE *pFile );
extern	struct	FILE_DIR *api_fs_directory( void );
extern	void 	api_fs_sync( void );
//----------------------------------------------------------------------------
#endif
