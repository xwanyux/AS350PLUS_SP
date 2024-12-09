



#ifndef _FS_H_
#define _FS_H_

#include "POSAPI.h"


#define FILE_NAME_SIZE 16

//----------------------------------------------------------------------------
struct	FILE_INFO
	{
	char		fname[FILE_NAME_SIZE];
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
	char		fname[FILE_NAME_SIZE];
	unsigned short	cluster_count;
	unsigned short	FS_type;
	unsigned long	length;
	struct FILE_INFO	*pDIR;
	struct FILE_INFO	*pFIB;
	unsigned long	*pIndex;
	unsigned long	logic_position;
	struct FILE	*next;
	};

//----------------------------------------------------------------------------
//		Function Prototypes
//----------------------------------------------------------------------------
extern	UCHAR	api_fs_init( void );
extern	void	api_fs_format( void );
extern	struct FILE	*api_fs_open( char *fileName, UCHAR mode );
extern	void	api_fs_close( struct	FILE *pf );
extern	UCHAR	api_fs_create( char fname[], unsigned short fileType );
extern	UCHAR	api_fs_delete( char fname[] );
extern	ULONG	api_fs_read(  struct FILE *pFile, UCHAR *buff, ULONG length );
extern	ULONG	api_fs_write( struct FILE *pFile, UCHAR *buff, ULONG length );
extern	UCHAR	api_fs_seek( struct	FILE *pFile, ULONG position );
extern	ULONG	api_fs_tell( struct	FILE *pFile );
extern	struct	FILE_DIR *api_fs_directory( void );
extern	void 	api_fs_sync( void );

//----------------------------------------------------------------------------
#endif