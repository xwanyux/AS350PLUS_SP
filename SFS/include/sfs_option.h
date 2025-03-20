//�ɦW�Goption.h
//�γ~�G
//
//�إߤ���G2008/06/11
//�@�̡Gcharles tsai
//�ק��x�G	2008/06/23	�[�JFILE_NAME_SIZE
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

#define	SMIT_SIZE	2	//secondary memory information table size�A�C�W�[�@�ӻ��U�O�и˸m�A�h�[�@

#ifdef	_SRAM_2MB_
#define SFS_USE_SRAM_SIZE_KB	(2048-128-64)	//1unit = 1KB,range = 1 ~ 65535 SRAM=2MB
#endif

#ifdef	_SRAM_4MB_
#define SFS_USE_SRAM_SIZE_KB	(4096-128-64)	//1unit = 1KB,range = 1 ~ 65535 SRAM=4MB
#endif

#define	SRAM_CLUSTER_SIZE	(2*1024)
#define	SFS_CLUSTER_DATA_BYTE	(SRAM_CLUSTER_SIZE-4)	//SFS���O���Ψ��x�s��ƪ��e�q
//��index�ήɽd������0~2043
//��p�ƭȥήɽd������0~2044
//==================================================================================================
//file system options

#define	MAX_OPENED_FILE	5
#define	FILE_NAME_SIZE	64	//�ɦW�����סA


#endif /*OPTION_H_*/
