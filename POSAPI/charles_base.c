/*
�W�١Gcharles_base.c
�@�̡GCharles Tsai
�γ~�Gprinter driver
�\��G
��x�G	
	2008/Sep/16	�ɮ׫إ�	 
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

