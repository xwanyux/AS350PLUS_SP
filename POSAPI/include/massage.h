//檔名：massage.h
//用途：
//
//建立日期：2008/06/11
//作者：charles tsai
//修改日誌：
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
		RETURN_OK = 0,			//成功 (無問題)
		RETURN_BUSY = 1,
		RETURN_WARNING,		//警告 
		RETURN_ERROR,		//錯誤
		RETURN_EXPECTION,	//例外
		RETURN_FATAL_FAIL = 0XF	//致命的失敗，將無法繼續執行程式
	};
	typedef enum RETURN_VALUE RESPONSE;
	enum ERROR_CODE
	{
		INPUT_PARAMETER_INCORRECTLY,	//輸入參數不正確
		MEMORY_ALLOCATE_FAIL,//記憶體分配失敗，通常為記憶體不足之故
		TIMEOUT_FAIL,	//等待超時
		DRIVER_TRANSMIT_FAIL,	//驅動程式傳送失敗
		BSP_ACQUIRE_FAIL,	// BSP ACQUIRE 失敗
		BSP_START_FAIL,		// BSP START 失敗
		DRIVER_INIT_FAIL,	//驅動程式初始化失敗
		//TCPIP
		NET_UNDEFINED_PROTOCOL,	//未定義的通訊協定
		NET_IP_COLLISION_EVENT,	//IP衝突事件
		NET_PACKET_LENGTH_TOO_LONG,	//封包長度超過最大值
		NET_PACKET_LENGTH_TOO_SHORT,	//封包長度小於最小值
		DEV_NIC_TX_IS_BUSY,		//網卡的傳送埠在使用中
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
			DEV_NIC_IN_QUEUE_USED,	//網卡的輸入佇列已被使用
			DEV_NIC_OUT_QUEUE_USED,	//網卡的輸出佇列已被使用
			DEV_NIC_IN_QUEUE_EMPTY,
			DEV_NIC_OUT_QUEUE_EMPTY,
		//DEVICE
		DEV_NIC_HW_ERROR,	//網卡硬體發生錯誤
		DEV_EEPROM_RD_FAIL,	//讀取-裝置EEPROM時，發生錯誤
		DEV_EEPROM_WR_FAIL	//寫入-裝置EEPROM時，發生錯誤
		//PS. EEPROM是連接在DM9000B網路晶片上
		
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
		
		
	
//DRIVER的ERRNO依各裝置的base address來設定，例如external SRAM使用EBI port3，base address在0x13000000，就將其相關的errno設為0x13開頭。
	#define	MSG_EXT_SRAM		0X13000000
//APB週邊裝置	
	#define MSG_APB		    	0xff000000
		#define	MSG_BSP_DRIVER		(MSG_APB | 0x00100000)
		#define	MSG_BSP_API			(MSG_APB | 0x00200000)
			#define MSG_BSP_API_INI_FAIL	(MSG_BSP_API | 0x100)	//initial
			#define MSG_BSP_API_ACQ_FAIL	(MSG_BSP_API | 0x200)	//Acquire
			#define MSG_BSP_API_STR_FAIL	(MSG_BSP_API | 0x400)	//start
			#define MSG_BSP_API_REL_FAIL	(MSG_BSP_API | 0x800)	//release		
	
//其它的ERRNO就設定在RESTRICTED的區段(參考MEMORY MAP)	
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
