/*
//�ɦW�Gsfs.h
//�γ~�G
//
//�إߤ���G2008/06/11
//�@�̡Gcharles tsai
//�ק��x�G
//			2008/06/18	�[�J�`��SFS_EOF
//						struct FILE_INFO's member used cluster byte���ثפӤp�A�ɭP���~�A�קאּ2byte
//			2008/07/08	�ק�struct MEM_INFO�G���F�O�@�g�J�ɹq�����_���M�p�o�͡C�[�Jlast_command,last_write_fname,last_write_length�A
 			2008/july/18	����july/08�Ұ����ܧ�A�H���P��hash�ȨӰ��O�@�N�n�F
*/
#ifndef SFS_H_
	#define SFS_H_
//====================================include=======================================================
#include "sfs_option.h"
#include "sfs_message.h"
//====================================globe variable================================================

//====================================function prototype============================================
void OS_SRAM_Setup( void );
MESSAGE SFS_init(unsigned char Did,unsigned long base);//��l��SFS
struct FILE* SFS_fopen(char fname[FILE_NAME_SIZE]);
struct FILE* SFS_fopen_force(char fname[FILE_NAME_SIZE]);
//==================================================================================================
/*	enum COMM
	{
		FORMAT 	= 0XFF,

		CREATE 	= 0X01,
		DELETE 	= 0X02,
		OPEN	= 0X03,
		CLOSE	= 0X04,
		READ	= 0X05,
		WRITE = 0X86,
		CHECK_HASH_VALUE= 0X87,
		MODIFY_INDEX	= 0X88,
		WRITING			= 0X89,
		GEN_HASH		= 0X8A,
		WRITE_HASH		= 0X8B,
		WRITED			= 0X8C,
		SEEK = 0X07,
		TALL = 0X08
	};*/

	struct	FILE_DIR
	{
		UCHAR	FileName[50][FILE_NAME_SIZE];	// file name list
		ULONG	FileSize[50];		// bytes
		ULONG	TotalFiles;		// total files in the directory
		ULONG	UsedMemorySize;		// K bytes
		ULONG	UnusedMemorySize;	// K bytes
	};

	struct	MEM_INFO
	{

		unsigned char	FS_type;
		unsigned char	cluster_size;
//		unsigned short	reserved;
		unsigned short 	mem_size;
		unsigned short	free_FIB_count;
		unsigned short	free_cluster_count;
		unsigned short	iFree_cluster;
//		enum COMM		last_command;
//		unsigned char	last_write_fname[8];
//		unsigned long	last_write_length;
		struct FILE_INFO	*FIT;
	}__attribute__((packed));

//==================================================================================================
	struct FILE_INFO
	{
		char	fname[FILE_NAME_SIZE];
		unsigned short	mode;
		unsigned short	iHead;
		unsigned short	iTail;
		unsigned short	total_cluster;
//		unsigned short	tail_cluster_used_byte;
		unsigned short	tail_cluster_unuse_byte;
		unsigned short	reserved;
		unsigned long	hash[5];
	}__attribute__((packed));
	#define	MODE_FREAD	0X01
	#define	MODE_FWRITE	0X02
	#define	MODE_FEXEC	0X04
	#define	MODE_FINDEX	0X80

//==================================================================================================
/*	union data{
		unsigned long	ldata[(SRAM_CLUSTER_SIZE-4)/4];
		unsigned char	cdata[(SRAM_CLUSTER_SIZE-4)];
	};
*/
union GenType
{
	long word;
	unsigned long 	uword;
	short dbyte[2];
	unsigned short 	udbyte[2];
	char byte[4];
	unsigned char	ubyte[4];
};
//==================================================================================================
	struct	SFS_CLUSTER
	{
		unsigned short	iPre;
		unsigned short	iNext;
		unsigned long	data[(SRAM_CLUSTER_SIZE-4)/4];
	}__attribute__((packed));
//==================================================================================================
#define SFS_EOF	0xffff	//end of file
//==================================================================================================
#endif /*SFS_H_*/
