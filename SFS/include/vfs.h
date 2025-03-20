/*
//檔名：vfs.h
//用途：
//
//
//作者：charles tsai
//修改日誌：
	V 0.01	2008/06/11	檔案建立

*/
#ifndef VFS_H_
	#define VFS_H_
//====================================include=======================================================
	#include "sfs_message.h"
	#include "sfs_option.h"
#include "POSAPI.h"
//====================================globe variable================================================

//====================================function prototype============================================

//==================================================================================================

//==================================================================================================
	struct SEC_MEM_INFO
	{
		unsigned char device_id;
		char	device_name[8];
		unsigned char FS_type;
		unsigned long base_addr;
		MESSAGE	(*fcreate)	(unsigned long path,char *fname);
		MESSAGE	(*fread)(unsigned long path,unsigned long seek,unsigned long length);
		struct FILE* (*fopen)(char fname[FILE_NAME_SIZE]);
	};

	#define NO_FS	0X00
	#define SFS 	0x01
	#define FAT 	0X02
/*	enum FILE_SYSTEM_TYPE
	{
		FS_IS_NO_FS = 0,
		FS_IS_SFS	= 1,
		FS_IS_FAT	= 2
	};

	struct LOGIC_DISK_DEVICE
	{
		unsigned char device_id;
		unsigned long base_addr;
		enum  FILE_SYSTEM_TYPE FS_type;
		MESSAGE	(*fcreate)	(unsigned long path,char *fname);
		struct FILE* (*fopen)(char fname[FILE_NAME_SIZE]);
		MESSAGE	(*fread)(unsigned long path,unsigned long seek,unsigned long length);

	};
	extern enum RETURN_VALUE create_SFS_disk(struct LOGIC_DISK_DEVICE* pDisk);
*/
//==================================================================================================
	//struct INDEX
//==================================================================================================
/*	struct LL_OF
	{
		struct OPENED_FILE	*file;
		struct LL_OF	*next;
	};*/
//==================================================================================================
	struct	FILE
	{
		unsigned long	status;//不需要status了
		char	fname[FILE_NAME_SIZE];
//		unsigned char	iSMIT;
		unsigned short	cluster_count;
		unsigned short	FS_type;//open & close 尚未處理type
		unsigned long	length;
		struct FILE_INFO	*pDIR;
		struct FILE_INFO	*pFIB;
		unsigned long	*pIndex;
		//run time information
		unsigned long	logic_position;
//		struct SFS_CLUSTER *pCurr_block;
		struct FILE *next;	//link到下一個物件
	};
	#define	OF_UNUSED	0
	#define	OF_USED 	0X10000000
//==================================================================================================
#if	0
	inline UCHAR api_fs_init();
	inline void api_fs_format();
	struct FILE* api_fs_open(char *fileName,unsigned char mode);
	void api_fs_close(struct FILE *pf);
	UCHAR api_fs_create(char fname[FILE_NAME_SIZE],unsigned short fileType);
	inline UCHAR api_fs_delete(char fname[FILE_NAME_SIZE]);
	inline unsigned long api_fs_read(struct FILE *pFile,unsigned char *buff,unsigned long length);
	inline unsigned long api_fs_write(struct FILE *pFile,unsigned char *buff,unsigned long length);
	inline UCHAR api_fs_seek(struct FILE *pFile,unsigned long position);
	inline unsigned long api_fs_tell(struct FILE *pFile);
#else
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
#endif

#endif /*VFS_H_*/
