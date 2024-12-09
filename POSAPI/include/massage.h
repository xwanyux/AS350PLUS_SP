//�ɦW�Gmassage.h
//�γ~�G
//
//�إߤ���G2008/06/11
//�@�̡Gcharles tsai
//�ק��x�G
#ifndef MASSAGE_H_
	#define MASSAGE_H_
//====================================include=======================================================

//====================================globe variable================================================

//====================================function prototype============================================

//==================================================================================================
	typedef unsigned char bool;
	#define TRUE 1
	#define FALSE 0
//==================================================================================================
	enum RETURN_VALUE
	{
		RETURN_OK = 0,			//���\ (�L���D)
		RETURN_BUSY = 1,
		RETURN_WARNING,		//ĵ�i 
		RETURN_ERROR,		//���~
		RETURN_EXPECTION,	//�ҥ~
		RETURN_FATAL_FAIL = 0XF	//�P�R�����ѡA�N�L�k�~�����{��
	};
	typedef enum RETURN_VALUE RESPONSE;
	enum ERROR_CODE
	{
		INPUT_PARAMETER_INCORRECTLY,	//��J�ѼƤ����T
		MEMORY_ALLOCATE_FAIL,//�O������t���ѡA�q�`���O���餣�����G
		TIMEOUT_FAIL,	//���ݶW��
		DRIVER_TRANSMIT_FAIL,	//�X�ʵ{���ǰe����
		BSP_ACQUIRE_FAIL,	// BSP ACQUIRE ����
		BSP_START_FAIL,		// BSP START ����
		DRIVER_INIT_FAIL,	//�X�ʵ{����l�ƥ���
		//TCPIP
		NET_UNDEFINED_PROTOCOL,	//���w�q���q�T��w
		NET_IP_COLLISION_EVENT,	//IP�Ĭ�ƥ�
		NET_PACKET_LENGTH_TOO_LONG,	//�ʥ]���׶W�L�̤j��
		NET_PACKET_LENGTH_TOO_SHORT,	//�ʥ]���פp��̤p��
		DEV_NIC_TX_IS_BUSY,		//���d���ǰe��b�ϥΤ�
		//SYSTEM
			//FILE SYSTEM
			ERR_END_OF_FILE,
			ERR_FIO_WR_FAIL,
			ERR_FIO_RD_FAIL,
			ERR_FILE_NOT_FOUND,
			ERR_FILE_OPENED,
			ERR_FILE_MODE,
			ERR_FILE_BREAKAGE,
			ERR_FS_NO_FREE_CLUSTER,
			ERR_FS_NO_FREE_FIB,
			ERR_FS_MUST_OPEN_FIRST,
			ERR_FILE_EXIST,
		//DRIVER
			DEV_PRN_PAPER_EMPTY,
		//INTERFACE
			DEV_NIC_IN_QUEUE_USED,	//���d����J��C�w�Q�ϥ�
			DEV_NIC_OUT_QUEUE_USED,	//���d����X��C�w�Q�ϥ�
			DEV_NIC_IN_QUEUE_EMPTY,
			DEV_NIC_OUT_QUEUE_EMPTY,
		//DEVICE
		DEV_NIC_HW_ERROR,	//���d�w��o�Ϳ��~
		DEV_EEPROM_RD_FAIL,	//Ū��-�˸mEEPROM�ɡA�o�Ϳ��~
		DEV_EEPROM_WR_FAIL	//�g�J-�˸mEEPROM�ɡA�o�Ϳ��~
		//PS. EEPROM�O�s���bDM9000B���������W
		
	};
	struct ERROR_INFO
	{
		enum ERROR_CODE error;
		unsigned long info1;
	};
//==================================================================================================
	
	typedef	unsigned long MASSAGE;
	#define MSG_SUCCESS 		0x00000000
	#define MSG_ARGUMENT      	0X01000000
		#define ERR_IN_ARG	    (MSG_ARGUMENT|0X1000)
			#define ERR_IN_ARG1	(ERR_IN_ARG|0x1)
			#define ERR_IN_ARG2	(ERR_IN_ARG|0x2)
			#define ERR_IN_ARG3	(ERR_IN_ARG|0x4)
			#define ERR_IN_ARG4	(ERR_IN_ARG|0x8)
			
	#define	MSG_MEMORY			0x02000000
		#define	ERR_NO_FREE_MEM		(MSG_MEMORY|0x1000)
		#define ARRAY_TOO_SMALL		(MSG_MEMORY|0x2000)
		
		
	
//DRIVER��ERRNO�̦U�˸m��base address�ӳ]�w�A�Ҧpexternal SRAM�ϥ�EBI port3�Abase address�b0x13000000�A�N�N�������errno�]��0x13�}�Y�C
	#define	MSG_EXT_SRAM		0X13000000
//APB�g��˸m	
	#define MSG_APB		    	0xff000000
		#define	MSG_BSP_DRIVER		(MSG_APB | 0x00100000)
		#define	MSG_BSP_API			(MSG_APB | 0x00200000)
			#define MSG_BSP_API_INI_FAIL	(MSG_BSP_API | 0x100)	//initial
			#define MSG_BSP_API_ACQ_FAIL	(MSG_BSP_API | 0x200)	//Acquire
			#define MSG_BSP_API_STR_FAIL	(MSG_BSP_API | 0x400)	//start
			#define MSG_BSP_API_REL_FAIL	(MSG_BSP_API | 0x800)	//release		
	
//�䥦��ERRNO�N�]�w�bRESTRICTED���Ϭq(�Ѧ�MEMORY MAP)	
	#define MSG_OS				0x60000000
		#define MSG_FILE_SYSTEM		( MSG_OS | 0x01000000)
		#define MSG_NETWORK			( MSG_OS | 0X02000000)
			#define	MSG_TCPIP			(MSG_NETWORK|0X00100000)
//	#define ERR_DEVICE_ID	(0X00000001)

//	#define MSG_OS				0x60000000
//		#define MSG_FILE_SYSTEM		(MSG_OS|0x01000000)	
			#define	MSG_FILE_IO			( MSG_FILE_SYSTEM | 0X00100000)
//				#define	ERR_END_OF_FILE		(MSG_FILE_IO | 0X00000001)	//
//				#define	ERR_FIO_WR_FAIL		(MSG_FILE_IO | 0x00000002)
//				#define	ERR_FIO_RD_FAIL		(MSG_FILE_IO | 0x00000003)
				
//			#define	ERR_FILE_NOT_FOUND	(MSG_FILE_SYSTEM|0X0100)
//			#define	ERR_FILE_OPENED  	(MSG_FILE_SYSTEM|0X0200)
//			#define	ERR_FILE_MODE        	(MSG_FILE_SYSTEM|0X0400)
//			#define	ERR_FILE_BREAKAGE   	(MSG_FILE_SYSTEM|0X0800)
	
//			#define	ERR_FS_NO_FREE_CLUSTER	(MSG_FILE_SYSTEM|0X1000)
//			#define	ERR_FS_NO_FREE_FIB    	(MSG_FILE_SYSTEM|0X2000)
//			#define	ERR_FS_MUST_OPEN_FIRST	(MSG_FILE_SYSTEM|0X4000)
//			#define	ERR_FILE_EXIST   	(MSG_FILE_SYSTEM|0X8000)
//==============================================================================================

#endif /*MASSAGE_H_*/
