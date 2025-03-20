//檔名：option.h
//用途：
//
//建立日期：2008/06/11
//作者：charles tsai
//修改日誌：	2008/06/23	加入FILE_NAME_SIZE
#include "POSAPI.h"

#ifndef _SFS_OPTION_H_
	#define _SFS_OPTION_H_
//====================================include=======================================================

//====================================globe variable================================================

//====================================function prototype============================================

//==================================================================================================
//memory options

#define	SFS_DISK_SIZE		2*1024*1024
UCHAR	SFS_DISK[SFS_DISK_SIZE];		// 2MB virtual DISK in RAM for backward compatible

#define	_SRAM_2MB_


#define	SFS_START_ADDR		&SFS_DISK	// 0x21600000	// 2017-04-17, new memory config for FS in SDRAM for ECC project. (without battery backup)

#define	SMIT_SIZE	2	//secondary memory information table size，每增加一個輔助記憶裝置，則加一

#ifdef	_SRAM_2MB_
#define SFS_USE_SRAM_SIZE_KB	(2048-128-64)	//1unit = 1KB,range = 1 ~ 65535 SRAM=2MB
#endif

#ifdef	_SRAM_4MB_
#define SFS_USE_SRAM_SIZE_KB	(4096-128-64)	//1unit = 1KB,range = 1 ~ 65535 SRAM=4MB
#endif

#define	SRAM_CLUSTER_SIZE	(2*1024)
#define	SFS_CLUSTER_DATA_BYTE	(SRAM_CLUSTER_SIZE-4)	//SFS的叢集用來儲存資料的容量
//當index用時範圍應為0~2043
//當計數值用時範圍應為0~2044
//==================================================================================================
//file system options

#define	MAX_OPENED_FILE	5
#define	FILE_NAME_SIZE	64	//檔名的長度，


#endif /*OPTION_H_*/
