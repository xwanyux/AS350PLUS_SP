/*
�W�١Gprinter.h
�@�̡GCharles Tsai
�γ~�Gprinter driver
�\��G
��x�G
	2008/Sep/15	�ɮ׫إ�
*/
#ifndef PRN_COMM_H_
	#define PRN_COMM_H_
//====================================include=======================================================
//c std library

//ZiLog BSP library
	#include "bsp_types.h"

//myself library
	#include "POSAPI.h"
	#include "massage.h"


//program define header
	#include "charles_base.h"
	#include "printer.h"
//====================================symbol=====================================

//printer control register pin define
	#define PRN_CMD_PH1 		0x0001//bit0
	#define PRN_CMD_PH2  		0x0002//bit1
	#define PRN_CMD_EN_MOTOR 	0x0004//bit2
	#define PRN_CMD_EN_LATCH	0x0008//bit3, active low
	#define	PRN_CMD_EN_HEAT		0x0010//bit4, strobe all (only 1 control signal)
	#define PRN_CMD_EN_9V 		0x0080//bit7
	
	#define PRN_CMD_EN_HEAT_B1	0X0100//bit8
	#define PRN_CMD_EN_HEAT_B2	0X0200//bit9
	#define PRN_CMD_EN_HEAT_B3	0X0400//bit10
	#define PRN_CMD_EN_HEAT_B4	0X0800//bit11
	#define PRN_CMD_EN_HEAT_B5	0X1000//bit12
	#define PRN_CMD_EN_HEAT_B6	0X2000//bit13
//	#define PRN_CMD_EN_9V 		0X8000//bit15

	#define PRN_CMD_STROBE_ALL	PRN_CMD_EN_HEAT
	#define PRN_CMD_STROBE_STOP		0
	#define PRN_CMD_STROBE_START	1
	#define PRN_CMD_STROBE_HEATING	2
	
//	#define PRN_CMD_STROBE_ALL (PRN_CMD_EN_HEAT_B1|PRN_CMD_EN_HEAT_B2|PRN_CMD_EN_HEAT_B3|PRN_CMD_EN_HEAT_B4|PRN_CMD_EN_HEAT_B5|PRN_CMD_EN_HEAT_B6)
	#define PRN_CMD_STROBE_123 (PRN_CMD_EN_HEAT_B1|PRN_CMD_EN_HEAT_B2|PRN_CMD_EN_HEAT_B3)
	#define PRN_CMD_STROBE_456 (PRN_CMD_EN_HEAT_B4|PRN_CMD_EN_HEAT_B5|PRN_CMD_EN_HEAT_B6)
	
	#define PRN_CMD_STROBE_135 (PRN_CMD_EN_HEAT_B1|PRN_CMD_EN_HEAT_B3|PRN_CMD_EN_HEAT_B5)
	#define PRN_CMD_STROBE_246 (PRN_CMD_EN_HEAT_B2|PRN_CMD_EN_HEAT_B4|PRN_CMD_EN_HEAT_B6)

	#define PRN_CMD_STROBE_12 (PRN_CMD_EN_HEAT_B1|PRN_CMD_EN_HEAT_B2)
	#define PRN_CMD_STROBE_34 (PRN_CMD_EN_HEAT_B3|PRN_CMD_EN_HEAT_B4)
	#define PRN_CMD_STROBE_56 (PRN_CMD_EN_HEAT_B5|PRN_CMD_EN_HEAT_B6)
	
	//==================================================================================================
	//flags
/*	#define PRN_STATUS_ISIDEL 	0X0001
	#define PRN_STATUS_TxC 		0x0100
	#define PRN_STATUS_Empty_Q 	0x0200
	#define PRN_STATUS_rotating 0x0400//1--> rotating 0--> still
	#define PRN_STATUS_EN_POW 	0x0800//0--> motor is pause, 1-->motor not pause
	#define PRN_STATUS_start
	#define PRN_STATUS_acceleration
	#define PRN_STATUS_run
	#define PRN_STATUS_stop*/
	//==================================================================================================
	enum MOT_STATUS
	{
//		MOT_STATIC =0,
		MOT_START=1,
		MOT_STARTED,
//		MOT_ACCELERATE,
//		MOT_NORMAL,
		MOT_ROTATING,
		MOT_STOP,
//		MOT_PAUSE,
		MOT_TURNOFF=0//idel

	};
//====================================class & structure=============================================
	#define	PRN_MAX_ROW_HEIGHT		64		// font_height + line_space (in dots)
	#define	PRN_DOTS_PER_LINE		384		// LTPA245=384, CAPG=432 dots
	#define	PRN_SPI_LEN			PRN_DOTS_PER_LINE/16
	#define	PRN_DOT_LINE_BYTES		PRN_DOTS_PER_LINE/8
	
	typedef unsigned char SUBROWMAP[PRN_DOTS_PER_LINE/8];
	
	//==================================================================================================
/*	struct FONT
	{
		unsigned char width;
		unsigned char height;
		const unsigned char *bitmap;
	};
/*	struct PRN_BLOCK
	{
		unsigned short start_bit;
		unsigned short end_bit;
	};*/
	//==================================================================================================

	struct ROWMAP
	{
		unsigned char info[4];
		unsigned long heat_time;
		SUBROWMAP data;
		unsigned long motor_time1;
		unsigned long motor_time2;
		struct ROWMAP *next,*last;
	};
/*	struct LINEMAP
	{
		unsigned char height;
		unsigned char width;
//		unsigned char printed_row;//
		struct ROWMAP *rows;//pointer rows[height]
		struct LINEMAP *next,*last;
	};*/
//==================================================================================================

	struct PRINT_PROFILE
	{
		unsigned short Vbat;//battery's voltage�A 0xffffffff��psu
		unsigned short fastestSpeedLV;//
		unsigned short MaxEnableHeat;//The maxunm of count of enable heat at the same time.
//		unsigned long Vmotor;	//the voltage of motor
		unsigned short HeattingTime;
//		unsigned short IncHeattingTime;
	};

//==================================================================================================
	enum PRINT_STATUS
	{
		PRINT_STATUS_IDEL=0,
		PRINT_STATUS_BUSY=1,
		PRINT_STATUS_BUFFERFULL,
		PRINT_STATUS_OFFLINE,
		PRINT_STATUS_PAPEREMPTY,
		PRINT_STATUS_PAPERJAM,
		PRINT_STATUS_COMPLETE,
		PRINT_STATUS_PROCESSING=0x80
	};
	struct PRINT
	{
		unsigned long flag;//bit 0: form feed
		unsigned long count;//count of link-list ROWMAP
		struct ROWMAP *pHead_row;
		struct ROWMAP *pTail_row;

		unsigned long count_next;//count of link-list ROWMAP
		struct ROWMAP *pNextFormHead;
		struct ROWMAP *pNextFormTail;
		unsigned short ctrl_reg;
		enum PRINT_STATUS status;
		unsigned long fastest_speed_LV;
//		unsigned char Vpp_motor;	//�|�v�T�B�i���F����t
		unsigned char motor_step;//�������0
//		unsigned char motor_accelerate;
//		volatile enum SEMAPHORE_B spi_Tx;
		enum MOT_STATUS	motor_status;
		unsigned char havePaper;//1:yes 0:no paper
		unsigned char ILG;//inter-line gap
		struct PRINT_PROFILE ProfilePSU;
#ifdef USE_BATTERY
		struct PRINT_PROFILE ProfileBAT1;
		struct PRINT_PROFILE ProfileBAT2;
		struct PRINT_PROFILE ProfileBAT3;
#endif
	};
#define PRN_FLAG_FORM_FEED	0x0c
//==================================================================================================
	// #define LATCH_CODE	0x08;//latch data bit for extension IO used
	// #define DST_CODE	0x10;//thermal head bit(DST) for extension IO used
	// #define TH_PW_CODE	0x80;//thermal head 9V power bit for extension IO used
	
	
	#define PRN_MOT_IN1			0
	#define PRN_MOT_IN2			1
	
	#define	PRN_MOT_ENA			0x20	// FORMAL
	#define PRN_MOT_ENA1			2			//
	#define	PRN_MOT_ENA2			0x1e		//

	
	#define PRN_MOT_MASK			 (PRN_MOT_ENA | PRN_MOT_IN2 | PRN_MOT_IN1)
	#define PRN_MOT_PHASE_MASK		~(PRN_MOT_ENA | PRN_MOT_IN2 | PRN_MOT_IN1)

	#define PRN_MOT_PHASE_1		(PRN_MOT_ENA1 | PRN_MOT_IN1)
	#define	PRN_MOT_PHASE_2		(PRN_MOT_ENA  | PRN_MOT_IN1)
	#define	PRN_MOT_PHASE_3		(PRN_MOT_ENA2 | PRN_MOT_IN1)
	#define	PRN_MOT_PHASE_4		(PRN_MOT_ENA  )
	#define	PRN_MOT_PHASE_5		(PRN_MOT_ENA1 )
	#define	PRN_MOT_PHASE_6		(PRN_MOT_ENA  | PRN_MOT_IN2)
	#define	PRN_MOT_PHASE_7		(PRN_MOT_ENA2 | PRN_MOT_IN2)
	#define	PRN_MOT_PHASE_8		(PRN_MOT_ENA  | PRN_MOT_IN1 | PRN_MOT_IN2)

//==================================================================================================
	struct ROWMAP_PAIR
	{
		unsigned long count;
		struct ROWMAP *head;
		struct ROWMAP *tail;
	};

//====================================globe variable================================================


//====================================function prototype============================================
void PRN_setILG(unsigned char row);
void PRN_releaseFormFeed();
void PRN_findFormFeed();
void PRN_Run();
enum PRINT_STATUS PRN_getStatus();
RESPONSE PRN_insert_rows(unsigned long size,struct ROWMAP *head,struct ROWMAP *tail);
enum PRINT_STATUS PRN_getStatus();
UCHAR PRN_getILG();
BSP_BOOL StringProcOpen();
void StringProcClose();	
void PRN_getbitmaprow(UCHAR* pdata);	
UINT32 PRN_getBitmapRowLength();
UCHAR API_PRNpicture(void *pPic,ULONG width,ULONG length,ULONG x_mov);	
enum RETURN_VALUE PRN_init();
RESPONSE PRN_Start();
RESPONSE PRN_Stop();
RESPONSE PRN_Release();
RESPONSE API_text_proc(char *str,unsigned char fontID);

UCHAR api_prnt_init();
UCHAR api_prnt_open(UCHAR type,UCHAR mode);
UCHAR api_prnt_close(UCHAR dhn);
UCHAR api_prnt_status(UCHAR dhn,UCHAR *status);
UCHAR api_prnt_putstring( UCHAR dhn, UCHAR fontID, UCHAR *dbuf );
void	API_prnt_setLineGap(unsigned char gap);
UCHAR API_prnt_getLineGap();

//==================================================================================================
#define setupTimer_uS(time) (((time)*90)/8)	//  90/8 = 11 units = 1uS
//===============================================================================

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
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================
//==================================================================================================




#endif /*PRN_COMM_H_*/
