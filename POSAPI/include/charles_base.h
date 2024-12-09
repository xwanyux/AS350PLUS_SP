/*
名稱：charles_base.c
作者：Charles Tsai
用途：printer driver
功能：
日誌：	
	2008/Sep/16	檔案建立	 
*/

#ifndef CHARLES_BASE_H_
	#define CHARLES_BASE_H_
	#include "massage.h"
//====================================include=======================================================
//c std library

//myself library
	#include "POSAPI.h"
	#include "massage.h"
	#include "option.h"

//program define header
//	#include "printer.h"
	#include "charles_base.h"
	
	
	
//====================================class & structure=============================================


//====================================globe variable================================================
//====================================function prototype============================================
void endian_B2L(unsigned short *in);
//===============================================================================
	
	
	
	
	
//	
	
	extern struct ERROR_INFO ERRNO;
	enum SEMAPHORE_B
	{
		SEM_B_SIGNAL = 1,
		SEM_B_WAIT = 0
	};

//==================================================================================================
	#define EBI_GCFG_REG 0xffff8010
	//static unsigned long int EBI_Globe_config;
	#define EBI_STA_REG 0xffff8064
	//static unsigned long int EBI_WR_ERR_status;//當為1表示發現了寫入錯誤在ready mode
	#define EBI_PORT_BASE 0X10000000
	#define EBI_PORT7_BASE	0x17000000
	#define EBI_CFG_REG1 0xffff8014
	#define EBI_CFG_REG7 0xffff8030
	#define EBI_TIM_REG1 0xffff803c
	#define EBI_TIM_REG7 0xffff8058
	
//==================================================================================================
//==================================================================================================

//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================

#endif /*CHARLES_BASE_H_*/
