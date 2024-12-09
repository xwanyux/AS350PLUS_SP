#ifndef OPTION_H_
	#define OPTION_H_
/*
************************************
File:   option.h 
Author: Alex Fok  
Purpose: File option.c header file
Create: 16-Feb-96
************************************
*/

#define UCHAR    unsigned char

#define		ALLOW_MASK		0x80	

		/* issuer option 1 definition */
#define 	AC_TYPE 			0
#define 	PIN_ENTRY 			1
#define 	MANUAL_ALLOW		2
#define 	EXPIRY_DATE_REQ 	3
#define 	OFFLINE_ALLOW		4
#define		VOICE_REF_ALLOW		5
#define		DESCRIPTOR_REQ		6
#define		ADJUST_ALLOW		7

		/* issuer option 2  definition*/
#define		PAN_DIGIT_CHECK		8
#define		ROC_NO_REQ 			9
#define		PRINT_RECEIPT		10
#define		CAPTURE_TRANS 		11
#define		CHK_OFFL_EXPIRY_DATE 12
#define		GUEST_PROCESS		13
#define		REFUND_BLOCK		14
#define		AUTH_BLOCK			15

		/* issuer option 3 defintion */
#define		VOID_BLOCK 			16
#define		ADDITION_DATA_ALLOW 17
#define		EPP_ALLOW 			18
#define		BLOCK_AUTH_CODE		19
#define		CHK_ALL_EXPIRY_DATE 20
#define		SALE_BLOCK			21	// Echo 2003/8/6 NEW
#define		REDEEM_BLOCK		22	// Echo 2003/8/6 NEW
 

		/* terminal configuration option 1 definition */
#define		AMOUNT_DUAL 		0
#define		DISP_MSR_REQ 		1
#define		TIPS_REQ 			2
#define		USER_ACCOUNTING 	3
#define		LODGING_PROCESSING 	4
#define		PRINT_TIME_REQ 		5
#define		DDMM_BUSINESS_DATE 	6
#define		CONFIRM_TOTAL		7 

		/* terminal configuration option 2 definition */
#define		TRACK_REQ 			8
#define		PRINTER_USED		9
#define		ECR_REF_REQ			10
#define		CASH_ADVANCED		11
//#define		SINGLE_SETTLE		12
//#define		TOTAL_ENTRY			13
/* REVERSED */
/* REVERSED */

		/* terminal configuration option 3 NOT DEFINED */
		/* --------------------------------------- */


		/* terminal local option definition */
#define		LOCK_KBD 			0
#define		NO_VOID_PSW 		1
#define		NO_REFUND_PSW 		2
#define		NO_ROC_PSW 			3
#define		MANUAL_PSW_REQ 		4
#define		SETTLE_PSW 			5			/* Echo 2003/3/15 ���b��J�K�X�]�w */
#define		LOGIN_PSW 			6			/* Echo 2003/3/15 �}����J�K�X�]�w */
/* REVERSED */
/* REVERSED */
/* REVERSED */


		/* terminal dial option definition */
/* REVERSED 			0	*/
/* REVERSED 			1	*/
#define		ENHANCED DIAL_MSG	2	
#define		AUTO_ANSWER			3
#define		REFERRAL_DIAL 		4
#define		MEMORY_DIAL			5
#define		TOLL_DIAL			6
#define		KBD_DIAL			7



//==================================================================================================
//FONT config
	#define MAX_FONT	16	//�r�������W��
//==================================================================================================
//printer config	
	#define MAX_FSTRING_SIZE	1024
//==================================================================================================	
//FILE SYSTEM config	
#define	SMIT_SIZE	2	//secondary memory information table size�A�C�W�[�@�ӻ��U�O�и˸m�A�h�[�@
#define SRAM_SIZE	(1024*1024)	//sram��1MB
#define	SRAM_CLUSTER_SIZE	(2*1024)
#define	SFS_CLUSTER_DATA_BYTE	(SRAM_CLUSTER_SIZE-4)	//SFS���O���Ψ��x�s��ƪ��e�q
//��index�ήɽd������0~2043
//���p�ƭȥήɽd������0~2044
#define	MAX_OPENED_FILE	5
#define	FILE_NAME_SIZE	8	//�ɦW�����סA
//==================================================================================================

//==================================================================================================
	
//==================================================================================================
	
//==================================================================================================
	
//==================================================================================================
	
//==================================================================================================
	
//==================================================================================================
	
#endif //OPTION_H_
