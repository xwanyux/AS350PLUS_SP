/*
名稱：API_printer.c
作者：Charles Tsai
用途：printer driver
功能：
日誌：
	2008/Sep/15	檔案建立
	2008/NOV/6	在printer為idel時會先插入空白列(0.125mm)到queue中。(修改API_text_proc())

*/
//====================================include=======================================================
//c std library
	#include <stdio.h>
	#include <stdarg.h>
//myself library
	#include "POSAPI.h"
	#include "massage.h"


//program define header
	#include "charles_base.h"
	#include "printer.h"
	#include "font_type.h"
//#define PRINT_DEBUG_ENABLE
//#define PRN_DEBUG
//====================================class & structure=============================================
/*struct PRINT_CONFIG
{
	struct FONT *pCurr_font;//pointer to currect used font
	struct FONT *pBig5;
	unsigned long motor_step_time;
};
*/

//====================================globe variable================================================

extern struct ERROR_INFO ERRNO;
//extern struct FONT font_table[max_font];

static unsigned long PRN_motor_step_time=0;
//static struct PRINT_CONFIG PRNcfg;

//====================================function prototype============================================
//inline RESPONSE API_row_space(unsigned long rows);
//==================================================================================================
extern BSP_BOOL PRN_generateRowmaps(unsigned char s[PRN_MAX_ROW_HEIGHT][PRN_DOT_LINE_BYTES],struct FONT_PAIR *font,struct ROWMAP_PAIR *result);
//==================================================================================================
//==================================================================================================
//non-8bit font

//==================================================================================================

//width <=32
unsigned char api_prnt_init()
{
	unsigned char status;
	font_init();
	if(PRN_init()==RETURN_OK)
	{
//		PRN_motor_step_time = PRN_get_motor_step_time();//*45;
		PRN_setILG(4);	// set default line space = 4 dots
		status = apiOK;
//		PRN_MOTOR_Enable(4*30);
		// PRN_MOTOR_Enable(4*2);
	}
	else
		status = apiFailed;//已被初始化過了
	return status;
}
//==================================================================================================
unsigned char api_prnt_open(unsigned char type,unsigned char mode)//mode: RFU
{

	unsigned char status;
	status = apiOutOfService;
	if(StringProcOpen()==0)
		return status;
	
	if(PRN_Start()==RETURN_OK)
	{
		status = apiOK;
	}
	else
	{
		PRN_Release();
		status = apiDeviceNotOpen;
	}
	api_prnt_init();
	/*
	switch(type)
	{
		case prtThermal:
			
				if(PRN_Start()==RETURN_OK)
				{
					status = apiOK;
					//PRN_MOTOR_Enable(26);
				}
				else
				{
					PRN_Release();
					status = apiDeviceNotOpen;
				}
			
			break;
		default:
			status  = apiNoDevice;
			break;
	}*/
	return status;
}
//==================================================================================================
unsigned char api_prnt_close(unsigned char dhn)
{
	StringProcClose();
	PRN_Stop();
	PRN_Release();
//	DEBUGPRINTF("close printer\r\n");
	return apiOK;
}
//==================================================================================================
inline unsigned char api_prnt_status(unsigned char dhn,unsigned char *status)
{
	*status = (unsigned char)PRN_getStatus();
	return apiOK;
}
//==================================================================================================
inline UCHAR api_prnt_putstring( UCHAR dhn, UCHAR fontID, UCHAR *dbuf )
{
	if(dbuf==0)
		return apiErrorInput;
	if(dbuf[0]==0)
		return apiErrorInput;
	if(API_text_proc(dbuf,fontID)==RETURN_OK)
		return apiOK;
	else
		return apiFailed;
}
//==================================================================================================
inline unsigned char API_prnt_getLineGap()
{
	return PRN_getILG();
}
//==================================================================================================
inline void API_prnt_setLineGap(unsigned char gap)
{
	PRN_setILG(gap);
}
//==================================================================================================

//==================================================================================================
////將format格式字串轉換成c-字串
//static inline unsigned char* API_f2C_string( char * Msg, ... )
//{
//	va_list	args;
//	int 	Len=0;
//	unsigned char PrintBuf[256]={0};
//	va_start( args , Msg );
//	Len = vsprintf( PrintBuf, Msg, args );
//	va_end( args );
//	if(Len>0)
//	{
//		unsigned char *pCstring = BSP_Malloc(sizeof(char)*(Len+1));
//		if(pCstring==0)
//		{
//			ERRNO.error = MEMORY_ALLOCATE_FAIL;
//			//PRN_setStatus(bufferFull);
//			return 0;
//		}
//		//copy string
//		memcpy(pCstring,(const void*)PrintBuf,Len);
//		pCstring[Len]='\0';
//		return pCstring;
//	}
//	return 0;
//
//}
//==================================================================================================
//RESPONSE API_print_fstring(unsigned char fontID,char * Msg, ... )
//{
//	va_list	args;
//	int 	Len=0;
//	unsigned char PrintBuf[MAX_FSTRING_SIZE]={0};
//	va_start( args , Msg );
//	Len = vsprintf( PrintBuf, Msg, args );
//	va_end( args );
//	if(Len<0||Len>MAX_FSTRING_SIZE)
//	{
//		ERRNO.error = INPUT_PARAMETER_INCORRECTLY;//too long
//		ERRNO.info1 = 2;
//		return RETURN_ERROR;
//	}
//	PrintBuf[Len]='\0';
//	return API_text_proc(PrintBuf,fontID);
//}
//==================================================================================================
//說明：列印空白列
//輸入：rows 空白列的數目，每單位為0.125mm
//回傳：RETURN_OK,RETURN_ERROR
//inline RESPONSE API_row_space(unsigned long rows)
//{
//	if(PRN_getStatus()==PRINT_STATUS_COMPLETE)
//		PRN_setStatus(PRINT_STATUS_IDEL);
//	if(rows==0)
//	{
//		ERRNO.error = INPUT_PARAMETER_INCORRECTLY;
//		return RETURN_ERROR;
//	}
//	PRN_MOTOR_Start();
//	unsigned long i;
//	struct ROWMAP *phead,*pcurr;
//	phead=PRN_generate_empty_row();
//	if(phead == 0)
//	{
//		ERRNO.error = MEMORY_ALLOCATE_FAIL;
//		return RETURN_ERROR;
//	}
//	pcurr=phead;
//	pcurr->motor_pluse_width1=PRN_motor_step_time;
//	pcurr->motor_pluse_width2=PRN_motor_step_time;
//	for(i=1;i<rows;i++)
//	{
//		pcurr->next=PRN_generate_empty_row();
//		if(pcurr->next == 0)
//		{
//			while(phead!=0)
//			{
//				pcurr = phead;
//				phead = phead->next;
//				BSP_Free(pcurr->subrow);
//				BSP_Free(pcurr);
//			}
//			ERRNO.error = MEMORY_ALLOCATE_FAIL;
//			return RETURN_ERROR;
//		}
//		pcurr->next->last = pcurr;
////		pTail = pcurr;
//		pcurr=pcurr->next;
//		pcurr->motor_pluse_width1=PRN_motor_step_time;
//		pcurr->motor_pluse_width2=PRN_motor_step_time;
//	}
//	PRN_insert_rows(phead,pcurr);
//	PRN_print();
//	return RETURN_OK;
//}
//==================================================================================================

//inline bool API_PRN_selectProfile()
//{
//#ifdef USE_BATTERY
//	if(PRN_SelectProfile()==TRUE)
//	{
//		PRN_motor_step_time = PRN_get_motor_step_time();
//		return TRUE;
//	}
//	else
//		return FALSE;
//#else
//	return TRUE;
//}
//
//
//#endif
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================



