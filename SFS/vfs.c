//檔名：vfs.c
//用途：
//
//建立日期：2008/06/11
//作者：charles tsai
//修改日誌：

//====================================include=======================================================
//>>>>>>>>>>>BSP
#include "bsp_mem.h"

#include "sfs_message.h"
#include "sfs_option.h"
#include "vfs.h"
#include "sfs.h"
#include "POSAPI.h"

extern	struct FILE_DIR *SFS_fdir();

//extern	UINT32	g_malloc;
//extern	UINT32	g_free;

//====================================globe variable================================================
struct	SEC_MEM_INFO	SMIT[SMIT_SIZE];
struct	FILE		*plFILE;

//====================================extern variable===============================================
extern unsigned long 	ERRNO;	//當為0表示沒有錯誤，當不為0時，請參考massage.h
//====================================function prototype============================================
static void ins_pfile(struct FILE* pf);
//==================================================================================================
UCHAR vfs_init()
{
	switch(SFS_init(0x13,SFS_START_ADDR))
	{
		case MSG_SUCCESS:
			return apiOK;
		case ERR_IN_ARG2:
			return apiErrorInput;
		case ERR_NO_FREE_MEM:
			return apiNoDevice;
	}
	return apiFailed;
}

//==================================================================================================
#if	1
struct FILE_DIR *api_sfs_directory( void )
{
	return( SFS_fdir() );
}
#endif

//==================================================================================================
#if	1
struct FILE* api_sfs_open(char *path,unsigned char mode)
{
	//在SMIT中找PATH所描述的裝置
	struct FILE* pf = SMIT[0].fopen(path);
	if(pf!=0)
	{
		pf->next = 0;
		ins_pfile(pf);
	}
	return pf;
}
#endif

//==================================================================================================
#if	1
void	api_sfs_close(struct FILE *pf)
{
	if(plFILE == pf)
	{
		plFILE = plFILE->next;
		BSP_Free(pf->pIndex);	// PATCH: 2013-03-31
//			g_free++;
		BSP_Free(pf);
//			g_free++;

		SFS_LoadFsVariables( 1 );
		
		return;
	}
	struct FILE *pre = plFILE , *next;

	while(pre->next != 0)
	{
		if(pre->next == pf)
		{
			break;
		}
		pre = pre->next;
	}
	if(pre->next != 0)
	{
		pre->next = pf->next;
		BSP_Free(pf->pIndex);	// PATCH: 2013-03-31
//			g_free++;
		BSP_Free(pf);
//			g_free++;
	}
	
	SFS_LoadFsVariables( 1 );
}
#endif

//==================================================================================================
#if	0	// NA
unsigned long mount_SMD(struct SEC_MEM_INFO *pSMD)
{
	//get a unused SMIT row.
	unsigned long i = 0;
	for(i=0;i<SMIT_SIZE;i++)
	{
		if(SMIT[i].device_name[0]==0)
			break;
	}
	if(i<SMIT_SIZE)//have unused SMIT row
	{
		SMIT[i].device_id = pSMD->device_id;
		unsigned long j;
		for(j=0;j<FILE_NAME_SIZE;j++)
			SMIT[i].device_name[j] = pSMD->device_name[j];
		SMIT[i].FS_type = pSMD->FS_type;
		SMIT[i].base_addr = pSMD->base_addr;
		SMIT[i].fcreate = pSMD->fcreate;
		SMIT[i].fopen = pSMD->fopen;
		SMIT[i].fread = pSMD->fread;
		return MSG_SUCCESS;
	}
	return ARRAY_TOO_SMALL;
}
#endif

//==================================================================================================
#if	1
static void ins_pfile(struct FILE* pf)
{
	if(pf != 0)
	{
		if(plFILE==0)
			plFILE=pf;
		else
		{
			struct FILE *plfile = plFILE;
			while(plfile->next!=0)
			{
				plfile = plfile->next;
			}
			plfile->next = pf;
		}
	}
}
#endif

//==================================================================================================
//檢查檔案是否已開啟，利用檢查是否有相同的指向檔案資訊區塊的指標(pFIB)來判斷
#if	1
bool file_opened(unsigned long point)
{
	struct FILE *pf = plFILE;
	while(pf!=0)
	{
		if((unsigned long)pf->pFIB == point)
			break;
		pf = pf->next;
	}
	if(pf!=0)
		return TRUE;//opened
	else
		return FALSE;//not open
}
#endif

//==================================================================================================
/*	input char *str is a c-style string.in the other word,this string is end in '\0'.
  	every token can't over than 8-character.
*/
#if	0
const short	MAX_TOKEN_SIZE = 8;
unsigned long  get_token(char *str,char *token)
{
	char temp[8];
	//讀取到'\','/' or '\0'時停止。
	//token可使用的字元只可包括大、小寫字母，數字
	int i=0;
	for(i=0;i<MAX_TOKEN_SIZE+1;i++)
	{
		switch(str[i])
		{
			case '/':
			//case '\':
			case 0:return i;
			default:
				if(((str[i]>=0x30)&&(str[i]<=0x39))	//charater = 0~9
				||((str[i]>=0x41)&&(str[i]<=0x5a))	//charater = A~Z
				||((str[i]>=0x61)&&(str[i]<=0x7a))	//charater = a~z
				//||(str[i]==':'))					//
				)
					temp[i]=str[i];
				else
					return 0;//不合法的字元
				break;


		}
	}
	return 0;//超過一個token的大小。
}
#endif

//==================================================================================================
//a path style
// token1\token2\token3\...\tokenN
// token1 shall be second memory device name.
// token2~token(N-1) shall be directory name.
// tokenN shall be file name.

//==================================================================================================
#if	0
inline void vfs_format()
{
	SFS_format();
}
#endif

//==================================================================================================
#if	1
UCHAR	api_sfs_create(char fname[FILE_NAME_SIZE],unsigned short fileType)
{
	switch(SFS_fcreate(fname,fileType))
	{
		case MSG_SUCCESS:
			return apiOK;
		case ERR_NO_FREE_MEM:
			return apiDeviceError;
		case ERR_FILE_EXIST:
			return apiErrorInput;
		case ERR_FILE_BREAKAGE:
			return apiFailed;

	}

}
#endif

//==================================================================================================
#if	1
UCHAR	api_sfs_delete(char fname[FILE_NAME_SIZE])
{
	switch(SFS_fdelete(fname))
	{
		case MSG_SUCCESS:
		case ERR_FILE_NOT_FOUND:
			return apiOK;
		case ERR_FILE_OPENED:
			return apiFailed;
	}
}
#endif

//==================================================================================================
#if	1
ULONG	api_sfs_read(struct FILE *pFile,unsigned char *buff,unsigned long length)
{
	return SFS_fread(buff,length,pFile);
}
#endif

//==================================================================================================
#if	1
ULONG	api_sfs_write(struct FILE *pFile,unsigned char *buff,unsigned long length)
{
	return SFS_fwrite(buff,length,pFile);
}
#endif

//==================================================================================================
#if	1
UCHAR	api_sfs_seek(struct FILE *pFile,unsigned long position)
{
	if(SFS_fseek(pFile,position)==TRUE)
		return apiOK;
	return apiFailed;
}
#endif

//==================================================================================================
#if	1
ULONG	api_sfs_tell(struct FILE *pFile)
{
	return SFS_ftall(pFile);
}
#endif

//==================================================================================================
#if	1
UCHAR	api_sfs_init()
{
UCHAR	result;


	result = vfs_init();
	
	SFS_LoadFsVariables( 1 );
	
	return( result );
}
#endif

//==================================================================================================
#if	1
void	api_sfs_format()
{
	//vfs_format();
	SFS_format();
	
	SFS_LoadFsVariables( 1 );
}
#endif

//==================================================================================================

//==================================================================================================

