/*
名稱：charles_base.c
作者：Charles Tsai
用途：printer driver
功能：
日誌：	
	2008/Sep/16	檔案建立	 
*/
//====================================include=======================================================

//myself library
	#include "POSAPI.h"
	#include "massage.h"
	#include "option.h"

//program define header
	#include "charles_base.h"
	
	
	
	
	
//	#include "printer.h"
	
//====================================class & structure=============================================


//====================================globe variable================================================
extern struct ERROR_INFO ERRNO;


//==================================================================================================
//	inline 
	void endian_B2L(unsigned short *in)
	{
		unsigned short temp = *in;
		temp=temp>>8;
		*in=*in<<8;
		*in|=temp;
		//return in;
	}

