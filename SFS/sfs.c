/*
//檔名：sfs.c
//用途：SFS的基本SYSTEM CALL FUNCTION
//
//建立日期：2008/06/11
//作者：charles tsai
//修改日誌：2008-06-12	加入全域變數OFT	在INIT()中清除OFT的status		加入函式SFS_fsearch()		加入file open()
//						加入 file close()		file read	file write()
//		   2008-06-30 	修改叢集處理函式：將read/write改為
		   2008-07-29	add function "sfs_fopen_force()",此函式在hash值錯誤依然會開啟檔案。
		   2008-07-31	modify cluster_read()，late byte read error，(ps. write已經改了，read可能是漏掉了)
*/
//====================================include=======================================================
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> BSP
#include "bsp_mem.h"
//#include "bsp_sha1.h"
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> FILE SYSTEM
#include "sfs_message.h"
#include "sfs_option.h"
#include "vfs.h"
#include "sfs.h"

//====================================globe variable================================================
static struct MEM_INFO	*pMIB;
static struct SFS_CLUSTER *pBLOCK;//cluster區塊的起始位址，第0塊用來存目錄，1~n塊用來存資料
static unsigned long iSMIT;
static unsigned long total_used_FIB;
//static struct FILE	OFT[MAX_OPENED_FILE];
//unsigned long sram_img[1024*1024/4];

static struct	FILE_DIR pFSDIR;	// 2011-11-02
static unsigned short	FS_Free_FIB_Count = 0;


//UINT32	g_malloc = 0;
//UINT32	g_free = 0;

//====================================extern variable===============================================
extern unsigned long 	ERRNO;	//當為0表示沒有錯誤，當不為0時，請參考massage.h
extern struct SEC_MEM_INFO	SMIT[SMIT_SIZE];

//extern void OS_SRAM_Setup( void );
//====================================function prototype============================================
static void copy_array(char *s,char *d,unsigned long len);
static bool compare_array(char *s,char *d,unsigned long len);
static void SFS_eraser_cluster_data(unsigned long index);
struct FILE_INFO*	SFS_fsearch(char *fname);
void   build_index(unsigned long iBlock,unsigned long *pIndex,unsigned long len);
////static bool sha_update(BSP_SHA *pSha,struct FILE *pf);
static char* fname_copy(char *name);
static unsigned long cluster_write(struct SFS_CLUSTER *pC,unsigned char *string,unsigned long start_point,unsigned long len);
static unsigned long cluster_read(struct SFS_CLUSTER *pC,unsigned char *string,unsigned long start_point,unsigned long len);
//test function
//void copy_sram_2_img();

//==================================================================================================
MESSAGE SFS_init(unsigned char Did,unsigned long base)//初始化SFS
{
//	OS_SRAM_Setup();
	//檢查smit中尚未註冊此裝置
	int i;
	for(i=0;i<SMIT_SIZE;i++)
		if(SMIT[i].base_addr==base)
		{
			//已註冊過的位址
			return ERR_IN_ARG2;
		}

	for(i=0;i<SMIT_SIZE;i++)
		if(SMIT[i].base_addr==0)
		{
			//尚未使用的物件
			break;
		}
	if(i==SMIT_SIZE)
		return ERR_NO_FREE_MEM;
	//註冊裝置資訊
	struct MEM_INFO *pmib;
	pmib = (struct MEM_INFO*)base;
//	DEBUGPRINTF("pmib %X %X\r\n",pmib,pmib->name);
	iSMIT = i;
	SMIT[iSMIT].device_id = Did;
//	SMIT[iSMIT].device_name = pmib->name;
	SMIT[iSMIT].base_addr = base;
	SMIT[iSMIT].FS_type = SFS;
	//SMIT[iSMIT].fopen=SFS_fopen;
	pMIB = (struct MEM_INFO*)base;
	pBLOCK = (struct SFS_CLUSTER*)base;
	total_used_FIB = (SRAM_CLUSTER_SIZE-sizeof(struct MEM_INFO))/sizeof(struct FILE_INFO)-pMIB->free_FIB_count;
	SMIT[iSMIT].fopen = SFS_fopen;
	//SMIT[iSMIT].fc
	return MSG_SUCCESS;
}


//==================================================================================================
/*#define SRAM_CS3_TIMING_REG 0xffff8048
void OS_SRAM_Setup( void )
{
UINT32 Status;

 // setup timing for READ cycles for nCS[3]
 Status = BSP_RD32( SRAM_CS3_TIMING_REG );
 Status &= ~0x0F;
 Status |= 0x04;  // 4 HCLK cycles (system default:0)
 BSP_WR32( SRAM_CS3_TIMING_REG, Status );
}*/

//==================================================================================================
#if	1
UINT	SFS_max_file_cnt( void )
{
	return( FS_Free_FIB_Count );	// 2017-04-18, max number of created files
}
#endif

//==================================================================================================
//需在call sfs_init()前call，或在call format之後重開機，否則SMIT中的資訊將可能不一致
#if	1
MESSAGE	SFS_format()
{
	memset( SFS_DISK, 0x00, SFS_DISK_SIZE );	// 2023-09-18
	
//	pMIB->reserved = 0;
	pMIB->FS_type = SFS;
	pMIB->cluster_size	= SRAM_CLUSTER_SIZE/1024;
	pMIB->mem_size = SFS_USE_SRAM_SIZE_KB;
	pMIB->free_FIB_count = (SRAM_CLUSTER_SIZE-sizeof(struct MEM_INFO))/sizeof(struct FILE_INFO);
	
	FS_Free_FIB_Count = pMIB->free_FIB_count;	// 2017-04-18
	
	total_used_FIB = (SRAM_CLUSTER_SIZE-sizeof(struct MEM_INFO))/sizeof(struct FILE_INFO)-pMIB->free_FIB_count;	// ???

/*	pMIB->last_command = FORMAT;
	pMIB->last_write_fname[0]=0;
	pMIB->last_write_fname[1]=0;
	pMIB->last_write_fname[2]=0;
	pMIB->last_write_fname[3]=0;
	pMIB->last_write_fname[4]=0;
	pMIB->last_write_fname[5]=0;
	pMIB->last_write_fname[6]=0;
	pMIB->last_write_fname[7]=0;
	pMIB->last_write_length = 0;
*///2008-july-18
	pMIB->FIT= (struct FILE_INFO*)(((unsigned long)pMIB)+sizeof(struct MEM_INFO));
	int i;
	int k;
	for(i=0;i<pMIB->free_FIB_count;i++)
	{
		//DEBUGPRINTF("point = %X\r\n",(unsigned long)&pMIB->FIT[i]);
		for( k=0; k<FILE_NAME_SIZE; k++ )
		   pMIB->FIT[i].fname[k] = 0;
#if	0		   
		pMIB->FIT[i].fname[0] = 0;
		pMIB->FIT[i].fname[1] = 0;
		pMIB->FIT[i].fname[2] = 0;
		pMIB->FIT[i].fname[3] = 0;
		pMIB->FIT[i].fname[4] = 0;
		pMIB->FIT[i].fname[5] = 0;
		pMIB->FIT[i].fname[6] = 0;
		pMIB->FIT[i].fname[7] = 0;
#endif
		pMIB->FIT[i].mode = 0;
		pMIB->FIT[i].iHead = 0;
		pMIB->FIT[i].iTail = 0;
		pMIB->FIT[i].total_cluster = 0;
//		pMIB->FIT[i].tail_cluster_used_byte = 0;
//		pMIB->FIT[i].tail_cluster_unuse_byte = SFS_CLUSTER_DATA_BYTE;
		pMIB->FIT[i].tail_cluster_unuse_byte = SFS_CLUSTER_DATA_BYTE;
		pMIB->FIT[i].hash[0] = 0;
		pMIB->FIT[i].hash[1] = 0;
		pMIB->FIT[i].hash[2] = 0;
		pMIB->FIT[i].hash[3] = 0;
		pMIB->FIT[i].hash[4] = 0;
		pMIB->FIT[i].reserved = 0;
	}
	pMIB->free_cluster_count = (SFS_USE_SRAM_SIZE_KB*1024/SRAM_CLUSTER_SIZE)-1;//第一塊已用來存放目錄
	pMIB->iFree_cluster = 1;
	int j;
/*	//test part
		unsigned long x,*y;
		for(x=0x13000800;x<0x13100000;x+=4)
		{
			y=x;
			*y=x;
		}
*/	//end test part
	for(i=1;i<=pMIB->free_cluster_count;i++)
	{
		pBLOCK[i].iPre = i-1;
		pBLOCK[i].iNext = i+1;
		for(j=0;j<(SRAM_CLUSTER_SIZE-4)/4;j++)
			pBLOCK[i].data[j]=0;
	}
	pBLOCK[i-1].iNext = 0;
	//pMIB->pFree_cluster
	DEBUGPRINTF("The SRAM format complete.\r\n");
	DEBUGPRINTF("The SRAM size is %d KB.\r\n",SFS_USE_SRAM_SIZE_KB);
	DEBUGPRINTF("The SRAM used file system is SFS.\r\n");
	DEBUGPRINTF("The FILE SYSTEM's cluster is %d\r\n",SRAM_CLUSTER_SIZE);
	DEBUGPRINTF("The FS's maximum useable file number is %d\r\n",pMIB->free_FIB_count);
	DEBUGPRINTF("The FS's maximum useable cluster number is %d\r\n",pMIB->free_cluster_count);


	return MSG_SUCCESS;
}
#endif

//==================================================================================================
// New function: 2011-11-02
//==================================================================================================
#if	1
struct	FILE_DIR *SFS_fdir( void )
{	
ULONG	i, j;
ULONG	used_cluster;
UCHAR	empty_name[FILE_NAME_SIZE] = {0};


	for( i=0; i<FS_Free_FIB_Count; i++ )
	   {
	   memset( &pFSDIR.FileName[i][0], 0x00, FILE_NAME_SIZE );
	   pFSDIR.FileSize[i] = 0;
	   }
	
	j = 0;
	used_cluster = 0;
	for( i=0; i<FS_Free_FIB_Count; i++ )
	   {
	   if( memcmp( pMIB->FIT[i].fname, empty_name, sizeof(empty_name) ) != 0 )
	     {
	     // file name & file size
	     memmove( &pFSDIR.FileName[j][0], pMIB->FIT[i].fname, FILE_NAME_SIZE );
	     pFSDIR.FileSize[j] = (pMIB->FIT[i].total_cluster-1)*(SFS_CLUSTER_DATA_BYTE)+(SFS_CLUSTER_DATA_BYTE-pMIB->FIT[i].tail_cluster_unuse_byte);
	     used_cluster += pMIB->FIT[i].total_cluster;
	     
	     j++;	// 2016-12-26
	     }
	   }
	
	pFSDIR.TotalFiles = j;
	pFSDIR.UsedMemorySize   = used_cluster * pMIB->cluster_size;
	pFSDIR.UnusedMemorySize = pMIB->free_cluster_count * pMIB->cluster_size;
	
	return( &pFSDIR );
}
#endif

//==================================================================================================
#if	1
MESSAGE SFS_fcreate(char fname[FILE_NAME_SIZE],unsigned short mode)
{

	if((pMIB->free_FIB_count==0)||(pMIB->free_cluster_count==0))
		return ERR_NO_FREE_MEM;//所有的fib或cluster均用掉了


	//複製fname，這是為了避免fname的長度不等長的問題
	char *new_fname;
	new_fname = fname_copy(fname);

	//檢查沒有相同檔名的檔案存在
	unsigned int i=0,j=0;
	while(i<total_used_FIB+pMIB->free_FIB_count)
	{
		if(pMIB->FIT[i].total_cluster!=0)
		{
			if(compare_array(new_fname,pMIB->FIT[i].fname,FILE_NAME_SIZE)==TRUE)
				return ERR_FILE_EXIST;
			j++;
		}
		if(j>=total_used_FIB)
			break;
		i++;
	}

	//在FIT中找到一個未使用的ROW (total cluster為0的FIB為尚未使用的)
	i=0;
	while(pMIB->FIT[i].total_cluster!=0)
		i++;
	pMIB->free_FIB_count--;
	total_used_FIB = (SRAM_CLUSTER_SIZE-sizeof(struct MEM_INFO))/sizeof(struct FILE_INFO)-pMIB->free_FIB_count;	// ???
	
	//copy file name
	copy_array(new_fname,pMIB->FIT[i].fname,FILE_NAME_SIZE);
	BSP_Free(new_fname);
//		g_free++;
	pMIB->FIT[i].mode = mode;
	pMIB->FIT[i].total_cluster = 1;
	//取得一塊未使用叢集
	pMIB->FIT[i].iHead = pMIB->iFree_cluster;//檔案的起始叢集為未使用叢集的第一塊
	pMIB->iFree_cluster = pBLOCK[pMIB->FIT[i].iHead].iNext;//將未使用叢集串列的第一塊變成原來串列的下一塊
	pBLOCK[pMIB->FIT[i].iHead].iNext = 0;//將檔案串列的下一塊設為無(0)
	pBLOCK[pMIB->iFree_cluster].iPre = 0;//將未使用叢集串列的第一塊的前一塊設為無(0)
	pMIB->FIT[i].iTail = pMIB->FIT[i].iHead;
	pMIB->free_cluster_count--;
//	pMIB->FIT[i].tail_cluster_used_byte = 0;
	pMIB->FIT[i].tail_cluster_unuse_byte = SFS_CLUSTER_DATA_BYTE;
	//產生hash值
#if	0
	BSP_SHA *pSha;
	pSha = BSP_SHA_Acquire();//取得sha1
	if(!pSha)
		return ERR_FILE_BREAKAGE;
	//檢查可用空間
	//sha_update(pSha,&pMIB->FIT[i]);
	BSP_SHA_Update(pSha,(unsigned char*)&pMIB->FIT[i],sizeof(struct FILE_INFO)-16);//本來應為-20，然後再update0x00000000
	BSP_SHA_Final(pSha,(unsigned char *)pMIB->FIT[i].hash,20);
	BSP_SHA_Release(pSha);
	//pMIB->FIT[i].hash
#endif
	return MSG_SUCCESS;
}
#endif

//==================================================================================================
#if	1
static void copy_array(char *s,char *d,unsigned long len)
{
	unsigned int i;
	for(i=0;i<len;i++)
		d[i]=s[i];
}
#endif

//==================================================================================================
#if	1
static bool compare_array(char *s,char *d,unsigned long len)
{
	unsigned int i;
	for(i=0;i<len;i++)
	{
		if(s[i]!=d[i])
			return FALSE;
	}
	return TRUE;
}
#endif

//==================================================================================================
#if	0
static bool compare_long_array(unsigned long *s,unsigned long *d,unsigned long len)
{
	unsigned int i;
	for(i=0;i<len;i++)
	{
		if(s[i]!=d[i])
		{
			DEBUGPRINTF("error in %x\r\n",i);
			return FALSE;
		}
	}
	return TRUE;

}
#endif

//==================================================================================================
#if	1
MESSAGE SFS_fdelete(char fname[FILE_NAME_SIZE])
{
	//複製fname，這是為了避免fname的長度不等長的問題
	char *new_fname;
	new_fname = fname_copy(fname);

	//檢查檔案是否存在
	unsigned int index=0;
	while(index<total_used_FIB+pMIB->free_FIB_count)
	{
		if(pMIB->FIT[index].total_cluster!=0)
		{
			if(compare_array(new_fname,pMIB->FIT[index].fname,FILE_NAME_SIZE)==TRUE)
				break;
		}
		index++;
	}
	
	BSP_Free( new_fname );	// PATCH: 2013-03-31
	
	if(index == total_used_FIB+pMIB->free_FIB_count)
		return ERR_FILE_NOT_FOUND;
	//檢查檔案是否已開啟
	if(file_opened(&pMIB->FIT[index])==TRUE)
		return ERR_FILE_OPENED;

	//抹除資料
	unsigned long iBlock;
	iBlock=pMIB->FIT[index].iHead;
	while(iBlock!=0)
	{
		SFS_eraser_cluster_data(iBlock);
		iBlock=pBLOCK[iBlock].iNext;
	}
	//將cluster插回到free list
	iBlock = pMIB->FIT[index].iTail;
	pBLOCK[iBlock].iNext = pMIB->iFree_cluster;
	pBLOCK[pMIB->iFree_cluster].iPre = iBlock;
	pMIB->iFree_cluster = pMIB->FIT[index].iHead;
	pMIB->free_cluster_count+=pMIB->FIT[index].total_cluster;

	//清除FIB
	pMIB->FIT[index].total_cluster = 0;
	
	pMIB->free_FIB_count++;
	total_used_FIB = (SRAM_CLUSTER_SIZE-sizeof(struct MEM_INFO))/sizeof(struct FILE_INFO)-pMIB->free_FIB_count;	// ???
	
//	copy_array("00000000",pMIB->FIT[index].fname,FILE_NAME_SIZE);
	memset( pMIB->FIT[index].fname, 0x00, FILE_NAME_SIZE);	// PATCH: 2011-11-02
	pMIB->FIT[index].mode = 0;
	pMIB->FIT[index].iHead=0;
	pMIB->FIT[index].iTail=0;
//	pMIB->FIT[index].tail_cluster_used_byte=0;
	pMIB->FIT[index].tail_cluster_unuse_byte = SFS_CLUSTER_DATA_BYTE;
	pMIB->FIT[index].hash[0] = 0;
	pMIB->FIT[index].hash[1] = 0;
	pMIB->FIT[index].hash[2] = 0;
	pMIB->FIT[index].hash[3] = 0;
	pMIB->FIT[index].hash[4] = 0;
	return MSG_SUCCESS;
}
#endif

//==================================================================================================
#if	1
void SFS_eraser_cluster_data(unsigned long index)
{
	int i;
	for(i=0;i<(SFS_CLUSTER_DATA_BYTE/4);i++)
	{
		pBLOCK[index].data[i]=0;
	}
}
#endif

//==================================================================================================
struct FILE* SFS_fopen(char fname[FILE_NAME_SIZE])
{
	//取得可用記憶體
	struct FILE *pf = BSP_Malloc(sizeof(struct FILE));
//		g_malloc++;

	//複製fname，這是為了避免fname的長度不等長的問題
	char *new_fname;
	new_fname = fname_copy(fname);

	//搜尋file
	struct FILE_INFO *pFile = SFS_fsearch(new_fname);
	if(pFile==0)
	{
		//DEBUGPRINTF("file not found.\r\n");
		BSP_Free( new_fname );	// PATCH: 2013-03-31
		BSP_Free( pf );		//
		
		ERRNO = ERR_FILE_NOT_FOUND;
		return 0;
	}
	//檢查file是否已被開啟
	if(file_opened((unsigned long)pFile)==TRUE)
	{
		//DEBUGPRINTF("file is opened.\r\n");
		BSP_Free( new_fname );	// PATCH: 2013-03-31
		BSP_Free( pf );		//
		
		ERRNO = ERR_FILE_OPENED;
		return 0;
	}
	pf->pFIB = pFile;
	pf->cluster_count = pFile->total_cluster;
	pf->status = OF_USED;
	copy_array(new_fname,pf->fname,FILE_NAME_SIZE);
	BSP_Free(new_fname);	// PATCH: 2013-03-31
//		g_free++;
	pf->logic_position = 0;
	pf->pDIR = pMIB->FIT;
	pf->pIndex = BSP_Malloc(sizeof(unsigned long)*pf->cluster_count);
//		g_malloc++;
	build_index(pFile->iHead,pf->pIndex,pf->cluster_count);

	//計算檔案總長度
	pf->length = (pFile->total_cluster-1)*(SFS_CLUSTER_DATA_BYTE)+(SFS_CLUSTER_DATA_BYTE-pFile->tail_cluster_unuse_byte);//pFile->tail_cluster_used_byte;
	//取得bsp sha
#if	0	// TEMP: skip HASH checking
	BSP_SHA	*pSha = BSP_SHA_Acquire();
	if(!pSha)
	{
		DEBUGPRINTF("sha not found.\r\n");
		
		BSP_Free( pf );		// PATCH: 2013-03-31
		return 0;
	}
	//產生hash值
//	DEBUGPRINTF("hash value is");
	sha_update(pSha,pf);
	unsigned char temp[20];
	BSP_SHA_Final(pSha,temp,20);
	BSP_SHA_Release(pSha);

	//比對hash值
	if(compare_array(temp,(unsigned char*)pFile->hash,20)==FALSE)
	{
		DEBUGPRINTF("hash value is %X %X %X %X %X\r\n shell be %X %X %X %X %X in FIB\r\n"
		,pFile->hash[0],pFile->hash[1],pFile->hash[2],pFile->hash[3],pFile->hash[4]
		,*((unsigned long*)&temp[0]),*((unsigned long*)&temp[4]),*((unsigned long*)&temp[8]),*((unsigned long*)&temp[12]),*((unsigned long*)&temp[16])
		);
		ERRNO = ERR_FILE_BREAKAGE;
		BSP_Free(pf);
//			g_free++;
		return 0;
	}
#endif
	return pf;
}

//==================================================================================================
//強迫開啟檔案，此函式無論hash值是否錯誤均會開啟檔案，但ERRNO中的值為0時，為沒有錯誤
#if	0	// NA
struct FILE* SFS_fopen_force(char fname[FILE_NAME_SIZE])
{
	//取得可用記憶體
	struct FILE *pf = BSP_Malloc(sizeof(struct FILE));
//		g_malloc++;

	//複製fname，這是為了避免fname的長度不等長的問題
	char *new_fname;
	new_fname = fname_copy(fname);

	//搜尋file
	struct FILE_INFO *pFile = SFS_fsearch(new_fname);
	if(pFile==0)
	{
		//DEBUGPRINTF("file not found.\r\n");
		ERRNO = ERR_FILE_NOT_FOUND;
		return 0;
	}
	//檢查file是否已被開啟
	if(file_opened((unsigned long)pFile)==TRUE)
	{
		//DEBUGPRINTF("file is opened.\r\n");
		ERRNO = ERR_FILE_OPENED;
		return 0;
	}
	pf->pFIB = pFile;
	pf->cluster_count = pFile->total_cluster;
	pf->status = OF_USED;
	copy_array(new_fname,pf->fname,FILE_NAME_SIZE);
	pf->logic_position = 0;
	pf->pDIR = pMIB->FIT;
	pf->pIndex = BSP_Malloc(sizeof(unsigned long)*pf->cluster_count);
//		g_malloc++;
	build_index(pFile->iHead,pf->pIndex,pf->cluster_count);

	//計算檔案總長度
	pf->length = (pFile->total_cluster-1)*(SFS_CLUSTER_DATA_BYTE)+(SFS_CLUSTER_DATA_BYTE-pFile->tail_cluster_unuse_byte);//pFile->tail_cluster_used_byte;
	//取得bsp sha
	BSP_SHA	*pSha = BSP_SHA_Acquire();
	if(!pSha)
	{
		DEBUGPRINTF("sha not found.\r\n");
		return 0;
	}
	//產生hash值
//	DEBUGPRINTF("hash value is");
	sha_update(pSha,pf);
	unsigned char temp[20];
	BSP_SHA_Final(pSha,temp,20);
	BSP_SHA_Release(pSha);
	//比對hash值
	if(compare_array(temp,(unsigned char*)pFile->hash,20)==FALSE)
	{
		DEBUGPRINTF("hash value is %X %X %X %X %X\r\n shell be %X %X %X %X %X in FIB\r\n"
		,pFile->hash[0],pFile->hash[1],pFile->hash[2],pFile->hash[3],pFile->hash[4]
		,*((unsigned long*)&temp[0]),*((unsigned long*)&temp[4]),*((unsigned long*)&temp[8]),*((unsigned long*)&temp[12]),*((unsigned long*)&temp[16])
		);
		ERRNO = ERR_FILE_BREAKAGE;
//		BSP_Free(pf);
//		return 0;
	}
	else
	{
		ERRNO = 0;
	}
	return pf;
}
#endif

//==================================================================================================
#if	1
struct FILE_INFO*	SFS_fsearch(char *fname)
{
	unsigned long index;
	for(index=0;index<pMIB->free_FIB_count+total_used_FIB;index++)
	{
		if(compare_array(fname,pMIB->FIT[index].fname,FILE_NAME_SIZE)==TRUE)
		{
			return &pMIB->FIT[index];
		}
	}
	return 0;
}
#endif

//==================================================================================================
#if	1
void build_index(unsigned long iBlock,unsigned long *pIndex,unsigned long len)
{
	unsigned int i;
	for(i=0;i<len;i++)
	{
		pIndex[i] = (unsigned long)&pBLOCK[iBlock];
		iBlock = pBLOCK[iBlock].iNext;
	}
}
#endif

//==================================================================================================
#if	0
//呼叫get unsigned char前需先檢查logic_position
unsigned long SFS_fgetuc(struct FILE *pFile)
{
	if(pFile == 0)
	{
		ERRNO = ERR_IN_ARG1;
		return 0XFFFF;
	}
	if(pFile->logic_position>=pFile->length)
		return SFS_EOF;
	unsigned long cluster,point;
	cluster = pFile->logic_position/(SRAM_CLUSTER_SIZE-4);//
	point	= pFile->logic_position%(SRAM_CLUSTER_SIZE-4);
	pFile->logic_position++;	//logic_position自動遞增
	struct SFS_CLUSTER *block = (struct SFS_CLUSTER*)pFile->pIndex[cluster];
	return block->data[point];
}
#endif

//==================================================================================================
//SFS file get unsigned char
#if	0
unsigned long SFS_fputuc(struct FILE *pFile,unsigned char data)
{
	if(pFile == 0)
	{
		ERRNO = ERR_IN_ARG1;
		return 0XFFFF;
	}
	if(pFile->logic_position>=pFile->length)
		return SFS_EOF;
	unsigned long cluster,point;
	cluster = pFile->logic_position/(SRAM_CLUSTER_SIZE-4);//4byte為保存索引值所用的。
	point	= pFile->logic_position%(SRAM_CLUSTER_SIZE-4);
	struct SFS_CLUSTER *block = (struct SFS_CLUSTER*)pFile->pIndex[cluster];
//	if(((((unsigned long)block)&0xffffff)%0x800)!=0)
//		DEBUGPRINTF("calculate point error in %X\r\n",block);
//	if((((unsigned long)(&block->data[point]))==0x13015804))
//		DEBUGPRINTF("error point\r\n");
/*	if(*((unsigned char*)0x13006000)!=0xb)
		DEBUGPRINTF("before error in C %X,P %X\r\n",cluster,point);
	if(*((unsigned char*)0x13016000)!=0x2b)
		DEBUGPRINTF("before error in C %X,P %X\r\n",cluster,point);
	if(*((unsigned char*)0x1302b800)!=0x56)
		DEBUGPRINTF("before error in C %X,P %X\r\n",cluster,point);
	if(*((unsigned char*)0x13036000)!=0x6b)
		DEBUGPRINTF("before error in C %X,P %X\r\n",cluster,point);
	if(*((unsigned char*)0x13056000)!=0xab)
		DEBUGPRINTF("before error in C %X,P %X\r\n",cluster,point);
	if(*((unsigned char*)0x13076000)!=0xeb)
		DEBUGPRINTF("before error in C %X,P %X\r\n",cluster,point);
//	else*/
		block->data[point] = data;
//	if(*((unsigned char*)0x13016000)!=0x2b)
//		DEBUGPRINTF("after error in C %X,P %X\r\n",cluster,point);
//	unsigned long index = ((((unsigned long)&block->data[point])&0xffffff)/4);
//	unsigned char *img = (unsigned char*)&sram_img[index];
//	img+=((((unsigned long)&block->data[point])&0xffffff)%4);
//	*img = data;
//	if(compare_long_array(sram_img,(unsigned long*)0x13000000,(1024*1024/4))==FALSE)
//		DEBUGPRINTF("write error in %X\r\n",index);
	pFile->logic_position++;	//logic_position自動遞增
	return block->data[point];
}
#endif

//==================================================================================================
//output --> *string
//return --> count of readed
#if	1
unsigned long SFS_fread(unsigned char *string,unsigned long length,struct FILE *pFile)
{
	if(pFile == 0)
	{
		ERRNO = ERR_IN_ARG2;
		return 0;
	}
	//unsigned long iString,data;
	if(length>(pFile->length-pFile->logic_position))//2008-07-18 error (pFile->length+pFile->logic_position)
	{
		//
		length = pFile->length-pFile->logic_position;
	}
//	if(length>0)//讀取的長度為0
//	{
//		string = BSP_Malloc(sizeof(unsigned char)*length);
//	}
	if(length==0)
	{
		string = 0;
		ERRNO = ERR_IN_ARG3;
		return 0;
	}
	unsigned long iStr=0;
	unsigned long count;
//	unsigned char *str_temp = BSP_Malloc(sizeof(unsigned char)*SFS_CLUSTER_DATA_BYTE);

	if((pFile->logic_position%SFS_CLUSTER_DATA_BYTE)!=0)
	{
		count = cluster_read(
		(struct SFS_CLUSTER*)pFile->pIndex[pFile->logic_position/SFS_CLUSTER_DATA_BYTE]
  		,&string[iStr],(pFile->logic_position%SFS_CLUSTER_DATA_BYTE)
//  		,SFS_CLUSTER_DATA_BYTE-(pFile->logic_position%SFS_CLUSTER_DATA_BYTE));
  		,((SFS_CLUSTER_DATA_BYTE-pFile->logic_position%SFS_CLUSTER_DATA_BYTE)<length)?SFS_CLUSTER_DATA_BYTE-(pFile->logic_position%SFS_CLUSTER_DATA_BYTE):length);
  		//bug : 在所寫入的資料長度小於最後叢集所剩長度時，將cluster_write()的len設為
  		//SFS_CLUSTER_DATA_BYTE-(pFile->logic_position%SFS_CLUSTER_DATA_BYTE)會發生錯誤，因為此值會超過真正的長度

		if(count==0)
		{
//			BSP_Free(str_temp);
			return 0;
		}
/*		if(str_temp==0)
		{
			return 0;
		}*/
//		copy_array(str_temp,string,count);
		iStr = count;
		pFile->logic_position += count;
//		BSP_Free(str_temp);
	}
	while(((length-iStr)/SFS_CLUSTER_DATA_BYTE)>0)
	{
		count = cluster_read(
		(struct SFS_CLUSTER*)pFile->pIndex[pFile->logic_position/SFS_CLUSTER_DATA_BYTE]
		,&string[iStr],0,SFS_CLUSTER_DATA_BYTE);
		if(count==0)
		{
//			BSP_Free(str_temp);
			return 0;
		}
/*		if(str_temp==0)
		{
			return 0;
		}
		copy_array(str_temp,&string[iStr],count);
*/		iStr += SFS_CLUSTER_DATA_BYTE;
		pFile->logic_position += count;
//		BSP_Free(str_temp);
	}
	if(((length-iStr)%SFS_CLUSTER_DATA_BYTE)>0)
	{
		count = cluster_read(
		(struct SFS_CLUSTER*)pFile->pIndex[pFile->logic_position/SFS_CLUSTER_DATA_BYTE]
  		,&string[iStr],0,((length-iStr)%SFS_CLUSTER_DATA_BYTE));
//		copy_array(str_temp,&string[iStr],count);
		pFile->logic_position += count;
		iStr += count;


	}
//	BSP_Free(str_temp);
	return iStr;
	//
//	if(remain!=0)
//	{
//		temp =
//		for(i=remain;i<4;i++)

//	}
/*	for(iString=0;iString<length;iString++)
	{
		data = SFS_fgetuc(pFile);
		if(data==SFS_EOF)
			return iString;
		string[iString] = (unsigned char)data;
	}
	return iString;
*/
}
#endif

//==================================================================================================
//return 已寫入的資料長度(byte)
#if	1
unsigned long SFS_fwrite(unsigned char *string,unsigned long length,struct FILE *pFile)
{
	unsigned long iWrited=0,msg;//iWrited 記錄讀取到string的那裡
	if(pFile == 0)
	{
		ERRNO = ERR_IN_ARG1;
		return 0;
	}
	unsigned long i=0,j;
	
#if	0
	BSP_SHA	*pSha;
	pSha = BSP_SHA_Acquire();
	if(!pSha)
	{
		//BSP_SHA_Release(pSha);
		iWrited = 0;
		ERRNO = MSG_BSP_API_ACQ_FAIL;
		goto end;
	}
	//產生hash值
	sha_update(pSha,pFile);
	unsigned char temp[20];
	//BSP_Delay_n_ms(1);
	BSP_SHA_Final(pSha,temp,20);
	//比對hash值
	if(compare_array(temp,(unsigned char*)pFile->pFIB->hash,20)==FALSE)
	{
		DEBUGPRINTF("hash value is %X %X %X %X %X\r\n shell be %X %X %X %X %X in FIB\r\n"
		,pFile->pFIB->hash[0],pFile->pFIB->hash[1],pFile->pFIB->hash[2],pFile->pFIB->hash[3],pFile->pFIB->hash[4]
		,*((unsigned long*)&temp[0]),*((unsigned long*)&temp[4]),*((unsigned long*)&temp[8]),*((unsigned long*)&temp[12]),*((unsigned long*)&temp[16])
		);
		ERRNO = ERR_FILE_BREAKAGE;
		goto end;
	}
#endif

	if(pFile->length<(pFile->logic_position+length))
	{//寫入操作將使檔案使用的空間變大
		if((((length+pFile->logic_position)/SFS_CLUSTER_DATA_BYTE)>pFile->cluster_count)||
		((((length+pFile->logic_position)/SFS_CLUSTER_DATA_BYTE)==pFile->cluster_count))&&
		((length+pFile->logic_position)%SFS_CLUSTER_DATA_BYTE)!=0)
		{//需要加入新叢集
			i = length-(pFile->length-pFile->logic_position);//檔案增加的長度
			i -= pFile->pFIB->tail_cluster_unuse_byte;//(SFS_CLUSTER_DATA_BYTE-pFile->pFIB->tail_cluster_used_byte);//減去最後一個叢集剩下的空位元組
			j = (i/SFS_CLUSTER_DATA_BYTE);//需新增的叢集數
			if((i%SFS_CLUSTER_DATA_BYTE)!=0)
				j++;//無條件進位
			//檢查空叢集是否足夠
			if(pMIB->free_cluster_count<j)
			{
				iWrited = 0;//0表示沒有複製資料
				goto end;
			}

			//新增叢集
//			struct SFS_CLUSTER *pblock;
			unsigned long int iPre,iNext;
			iPre = pFile->pFIB->iTail;
			iNext = pMIB->iFree_cluster;
			for(i=0;i<j-1;i++)
			{
				iNext = pBLOCK[iNext].iNext;
			}
			unsigned long iLast = 0;
			if(pBLOCK[iNext].iNext==0)//為最後一個空叢集
				iLast = iNext;
			iNext = pBLOCK[iNext].iNext;
			pBLOCK[iPre].iNext = pMIB->iFree_cluster;
			pBLOCK[pMIB->iFree_cluster].iPre = iPre;
			pMIB->iFree_cluster = iNext;
			if(iNext!=0)
			{
				pFile->pFIB->iTail = pBLOCK[iNext].iPre;
				//pBLOCK[pFile->pFIB->iTail].iNext = 0;
				pBLOCK[pMIB->iFree_cluster].iPre = 0;
			}
			else
			{//已無空叢集
				pFile->pFIB->iTail = iLast;
				//pBLOCK[pFile->pFIB->iTail].iNext = 0;
				//pBLOCK[pMIB->iFree_cluster].iPre = 0;
			}
			pBLOCK[pFile->pFIB->iTail].iNext = 0;


			pMIB->free_cluster_count-=j;
			pFile->pFIB->total_cluster+=j;
			pFile->cluster_count+=j;
			//重新產生索引陣列
			BSP_Free(pFile->pIndex);
//				g_free++;
			pFile->pIndex = BSP_Malloc(sizeof(unsigned long)*pFile->cluster_count);
//				g_malloc++;
			build_index(pFile->pFIB->iHead,pFile->pIndex,pFile->cluster_count);
		}
		pFile->length = pFile->logic_position+length;
		pFile->pFIB->tail_cluster_unuse_byte = SFS_CLUSTER_DATA_BYTE-(pFile->length%SFS_CLUSTER_DATA_BYTE);
		if(pFile->pFIB->tail_cluster_unuse_byte==SFS_CLUSTER_DATA_BYTE)
			pFile->pFIB->tail_cluster_unuse_byte=0;
	}
	if((pFile->logic_position%SFS_CLUSTER_DATA_BYTE)!=0)
	{
		msg = cluster_write(
		(struct SFS_CLUSTER*)pFile->pIndex[pFile->logic_position/SFS_CLUSTER_DATA_BYTE]
  		,&string[iWrited],(pFile->logic_position%SFS_CLUSTER_DATA_BYTE)
  		,((SFS_CLUSTER_DATA_BYTE-pFile->logic_position%SFS_CLUSTER_DATA_BYTE)<length)?SFS_CLUSTER_DATA_BYTE-(pFile->logic_position%SFS_CLUSTER_DATA_BYTE):length);
  		//bug : 在所寫入的資料長度小於最後叢集所剩長度時，將cluster_write()的len設為
  		//SFS_CLUSTER_DATA_BYTE-(pFile->logic_position%SFS_CLUSTER_DATA_BYTE)會發生錯誤，因為此值會超過真正的長度

		if(msg==0)
			return 0;
		iWrited = msg;
		pFile->logic_position += msg;

	}
	while(((length-iWrited)/SFS_CLUSTER_DATA_BYTE)>0)
	{
		msg = cluster_write(
		(struct SFS_CLUSTER*)pFile->pIndex[pFile->logic_position/SFS_CLUSTER_DATA_BYTE]
		,&string[iWrited],0,SFS_CLUSTER_DATA_BYTE);
		if(msg == 0)
			return 0;
		iWrited += SFS_CLUSTER_DATA_BYTE;
		pFile->logic_position += msg;
	}
	if(((length-iWrited)%SFS_CLUSTER_DATA_BYTE)>0)
	{
		msg = cluster_write(
		(struct SFS_CLUSTER*)pFile->pIndex[pFile->logic_position/SFS_CLUSTER_DATA_BYTE]
  		,&string[iWrited],0,((length-iWrited)%SFS_CLUSTER_DATA_BYTE));
		if(msg==0)
			return 0;
		iWrited += (length-iWrited);
		pFile->logic_position += msg;
	}
		//DEBUGPRINTF("HASH VALUE IS ");
#if	0
	sha_update(pSha,pFile);
	BSP_SHA_Final(pSha,(unsigned char *)pFile->pFIB->hash,20);
//	DEBUGPRINTF("hash1 in FIB = %X %X %X %X %X\r\n"
//		,pFile->pFIB->hash[0],pFile->pFIB->hash[1],pFile->pFIB->hash[2],pFile->pFIB->hash[3],pFile->pFIB->hash[4]);
end:
	BSP_SHA_Release(pSha);
#else
end:
	
#endif

	return iWrited;
}
#endif

//==================================================================================================
#if	1
BSP_BOOL SFS_fseek(struct FILE *pFile,unsigned long position)
{
	if(pFile == 0)
	{
		ERRNO = ERR_IN_ARG1;
		return FALSE;
	}
	if(pFile->length<position)
		return FALSE;
	pFile->logic_position = position;
	return TRUE;
}
#endif

//==================================================================================================
#if	1
unsigned long SFS_ftall(struct FILE *pFile)
{
	if(pFile == 0)
	{
		ERRNO = ERR_IN_ARG1;
		return 0;
	}
	return pFile->logic_position;
}
#endif

//==================================================================================================
//#ifdef HASH_FUN_IS_HW_HASH
//2008/7/16 暫時去掉 static
#if	0
static bool sha_update(BSP_SHA *pSha,struct FILE *pf)
{
	BSP_SHA_Update(pSha,(unsigned char*)pf->pFIB,sizeof(struct FILE_INFO)-20);
	struct SFS_CLUSTER *pBlock;
	pBlock = &pBLOCK[pf->pFIB->iHead];
	int i=0;
	while(i<pf->pFIB->total_cluster-1)
	{
		BSP_SHA_Update(pSha,(unsigned char*)pBlock,SRAM_CLUSTER_SIZE);
		pBlock = &pBLOCK[pBlock->iNext];
		i++;
	}
	BSP_SHA_Update(pSha,(unsigned char*)pBlock,SRAM_CLUSTER_SIZE-pf->pFIB->tail_cluster_unuse_byte);
}
#endif

//==================================================================================================
//
#if	1
static char* fname_copy(char *name)
{
	int i;
	char *new_name = BSP_Malloc(sizeof(char)*FILE_NAME_SIZE);
//		g_malloc++;
	for(i=0;i<FILE_NAME_SIZE;i++)
		new_name[i] = 0;
	for(i=0;i<FILE_NAME_SIZE;i++)
	{
		if(name[i]==0)
		{
			i++;
			break;
		}
	}
	copy_array(name, new_name,i);
	return new_name;
}
#endif

//==================================================================================================
// PATCH: 2011-01-18
#if	1
static unsigned long cluster_write(struct SFS_CLUSTER *pC,unsigned char *string,unsigned long start_point,unsigned long len)
{
UCHAR	buf[4];
ULONG	length;
ULONG	st_point;
unsigned long remain;
unsigned long temp,i,j,index = 0;
unsigned long iData = start_point/4;


	length = len;
	st_point = start_point;

	if((start_point+len)>SFS_CLUSTER_DATA_BYTE)
	  {
	  ERRNO = ERR_IN_ARG3;
	  return 0;
	  }
	  
	while( length )
	     {
	     temp = pC->data[iData];
	     buf[0] = temp & 0x000000FF;
	     buf[1] = (temp & 0x0000FF00) >> 8;
	     buf[2] = (temp & 0x00FF0000) >> 16;
	     buf[3] = (temp & 0xFF000000) >> 24;
	
	     remain = st_point%4;
	     
	     j = 0;
	     for( i=remain; i<4; i++ )
		{
		if( length != 0 )
		  {
		  buf[i] = string[index++];
		  length--;
		  j++;
		  }
		else
		  break;
		}
	     pC->data[iData++] = buf[0] + (buf[1]*0x100) + (buf[2]*0x10000) + (buf[3]*0x1000000);		
	     st_point += j;
	     }

	if(index==len)
	  return len;
	else
	  {
	  ERRNO = ERR_FIO_WR_FAIL;
	  return 0;
	  }
}
#endif

//==================================================================================================
// PATCH: 2011-01-18
#if	1
static unsigned long cluster_read(struct SFS_CLUSTER *pC,unsigned char *string,unsigned long start_point,unsigned long len)
{
UCHAR	buf[4];
ULONG	length;
ULONG	st_point;
unsigned long remain;
unsigned long temp,i,j,index = 0;
unsigned long iData = start_point/4;


	length = len;
	st_point = start_point;

	if((start_point+len)>SFS_CLUSTER_DATA_BYTE)
	  {
	  ERRNO = ERR_IN_ARG3;
	  return 0;
	  }
	  
	while( length )
	     {
	     temp = pC->data[iData++];
	     buf[0] = temp & 0x000000FF;
	     buf[1] = (temp & 0x0000FF00) >> 8;
	     buf[2] = (temp & 0x00FF0000) >> 16;
	     buf[3] = (temp & 0xFF000000) >> 24;
	
	     remain = st_point%4;
	     
	     j = 0;
	     for( i=remain; i<4; i++ )
		{
		if( length != 0 )
		  {
		  string[index++] = buf[i];
		  length--;
		  j++;
		  }
		else
		  break;
		}
	     
	     st_point += j;
	     }

	if(index==len)
	  return len;
	else
	  {
	  ERRNO = ERR_FIO_RD_FAIL;
	  return 0;
	  }
}
#endif

//==================================================================================================
#if	0
void list2uart()
{
	unsigned int size = (SRAM_CLUSTER_SIZE-sizeof(struct MEM_INFO))/sizeof(struct FILE_INFO);
	unsigned int i;
	for(i=0;i<size;i++)
	{
		if(pMIB->FIT[i].total_cluster!=0)
		{
			DEBUGPRINTF("%.8s\r\n",pMIB->FIT[i].fname);
		}

	}
}
#endif

//==================================================================================================
#if	0
void show_list()
{
//	struct FILE_INFO *pf;
//	pf = pMIB->FIT;
	unsigned int i;
	DEBUGPRINTF("File in smb0\r\n");
	for(i=0;i<(SRAM_CLUSTER_SIZE-sizeof(struct MEM_INFO))/sizeof(struct FILE_INFO);i++)
	{
		if(pMIB->FIT[i].total_cluster!=0)
		{
			DEBUGPRINTF("%.8s ",pMIB->FIT[i].fname);
		}
	}
	DEBUGPRINTF("/r/n");
}
#endif

//==================================================================================================

//==================================================================================================

// ---------------------------------------------------------------------------
// FUNCTION: Load File System variables from backup flash area.
// INPUT   : flag	0=read, 1=write
// OUTPUT  : none.
// RETURN  : TRUE / FALSE
// ---------------------------------------------------------------------------
ULONG	SFS_LoadFsVariables( ULONG flag )
{
#if	0	// TO BE TESTED !!!

UINT32	result = FALSE;
UINT32	bytes = 0;
struct	FILE	*fh;
UINT8	*pMem;


	api_fs_select( 0 );	// MEDIA_FLASH = 0
	api_fs_init();
	
	// UCHAR  SFS_DISK[SFS_DISK_SIZE];
	
	fh = api_fs_open( "SFS_DISK", 0 );
	if( !fh )
	  {
	  printf("\n1st time to create SFS_DISK\n");
	  // 1'st time process to create an empty file
	  
	  vfs_init();
	  SFS_format();	// format RAM DISK
	  
	  api_fs_create( "SFS_DISK", 0 );
	  fh = api_fs_open( "SFS_DISK", 0 );
	  if( fh )
	    {
	    pMem = malloc( SFS_DISK_SIZE );
	    if( pMem )
	      {
//	      memset( pMem, 0x00, SFS_DISK_SIZE );
	      memmove( pMem, &SFS_DISK[0], SFS_DISK_SIZE );	      
	      bytes = api_fs_write( fh, pMem, SFS_DISK_SIZE );

//	      bytes = api_fs_write( fh, &SFS_DISK[0], SFS_DISK_SIZE );

	      free( pMem );
	      result = TRUE;
	      }
	    goto EXIT0;
	    }
	  else
	    {
	    result = FALSE;
	    goto EXIT;
	    }
	  }

	if( flag )
	  {
	  printf("\nWrite SFS_DISK\n");
	  
	  // write DISK to backup file
	  bytes = api_fs_write( fh, &SFS_DISK[0], SFS_DISK_SIZE );
	  if( bytes == SFS_DISK_SIZE )
	    result = TRUE;
	  else
	    result = FALSE;

	  goto EXIT0;
	  }
	  
	// read DISK from backup file
	printf("\nRead SFS_DISK\n");
	bytes = api_fs_read( fh, &SFS_DISK[0], SFS_DISK_SIZE );
	if( bytes == SFS_DISK_SIZE )
	  result = TRUE;
	else
	  result = FALSE;

EXIT0:
	api_fs_close( fh );
EXIT:
	api_fs_select( 2 );	// MEDIA_RAM = 2
	
	return( result );
#else

	return( TRUE );
#endif
}
