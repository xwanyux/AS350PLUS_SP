//c std library
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <time.h>
#include <sys/ioctl.h> 
//myself library
#include "POSAPI.h"
#include "massage.h"	
#include "ExtIODev.h"
#include "bsp_gpio.h"
#include "bsp_tmr.h"
#include "bsp_types.h"
//program define header 
#include "charles_base.h"
#include "printer.h"
#include "bsp_SHM.h"
#include "PRN_dev.h"
#define	MAX_MOTOR_ACCEL_STEP		24
#define	PRN_DEBUG_MODE				0
 
enum SEMAPHORE_B PRN_use = SEM_B_SIGNAL;//signal -> printer used, wait->printer useless
static struct PRINT LTPA245;
unsigned char speedtable[65536];
unsigned long point = 0;

unsigned long g_heat_time = 0;
unsigned long g_info0 = 0;
unsigned long g_info2 = 0;

unsigned long prn_reserved_dot_lines = 0;
unsigned long prn_dot_lines_to_be_printed = 0;
unsigned long prn_motor_stop = FALSE;

unsigned long prn_time_tick = 0;
BSP_TIMER* PRN_TMR;
UCHAR PRNRUN_flag=0;
extern UCHAR PRINTER_TEST_BYTE[48];
extern UCHAR prn_dumpbuffer[1024*1024];
extern int prn_count;
struct input_event PRNevent[5];
struct timeval PRN_irq_timeout;
fd_set PRN_irq_set;
UCHAR PRN_ISRflag=0;
UCHAR PRN_ISRIO_INTENA_bak=0;
UCHAR PRN_ISRIO_INTCONA_bak=0; 
UCHAR PRN_releaseTIMER=0;
UINT16 PRN_MOTORstepTIME=0;
extern int TSC_fd;
extern UCHAR	OStimerFlag;//to disable OS pthread before print
extern pthread_t 	OS_Timer_Thread;//OS pthread flag
UCHAR PRN_status;//for other API to know if printer running
int STBdriver_fd=0;//file description for printer kernel driver
UCHAR MOTOR_STEP_CODE[8]={//motor step code for extension IO chip used
	// 0x04,0x26,0x22,0x27,0x05,0x25,0x20,0x24
	0x44,0x66,0x62,0x67,0x45,0x65,0x60,0x64
};
// const unsigned long int step_motor_acceleration[96+1]=
// {
	// 4291,
	// 4291, 2652, 2048, 1719, 1507, 1357, 1243, 1153,
	// 1080, 1019, 967,  923,  883,  849,  818,  790,//17 250~300
	// 765,  742,  721,  702,  684,  668,  652,  638,
	// 624,  612,  600,  588,  578,  568,  558,  549,//33 200~250
	// 540,  532,  524,  516,  509,  502,  495,  489,
	// 483,  477,  471,  466,  460,  455,  450,  445,//49 150~200
	// 440,  436,  432,  427,  423,  419,  415,  411,
	// 408,  404,  400,  397,  394,  390,  387,  384,//65 100~150
	// 381,  378,  375,  372,  370,  367,  364,  362,
	// 359,  357,  354,  352,  350,  347,  345,  343,//81 50~100
	// 341,  339,  336,  334,  332,  330,  329,  327,
	// 325,  323,  321,  319,  318,  316,  314,  313,//97 0~50
	
// };
const unsigned long int step_motor_acceleration[96+1]=
{
	4291,
	4091, 2452, 1848, 1519, 1307, 1157, 1043, 953,
	880,  819, 767,  723,  683,  649,  618,  590,//17 250~300
	565,  542,  521,  502,  484,  468,  452,  438,
	424,  412,  400,  388,  378,  368,  358,  349,//33 200~250
	340,  332,  324,  316,  309,  302,  295,  289,
	283,  277,  271,  266,  260,  255,  250,  245,//49 150~200
	240,  236,  232,  227,  223,  219,  215,  211,
	208,  204,  200,  197,  194,  190,  187,  184,//65 100~150
	181,  178,  175,  172,  170,  167,  164,  162,
	159,  157,  154,  152,  150,  147,  145,  143,//81 50~100
	141,  139,  136,  134,  132,  130,  129,  127,
	125,  123,  121,  119,  118,  116,  114,  113,//97 0~50
	
};

void PRN_setILG(unsigned char row)
{
	LTPA245.ILG = row;
}
//==================================================================================================
//inline 
unsigned char PRN_getILG()
{
	return LTPA245.ILG;
}
//==================================================================================================

//inline 
void PRN_setStatus(enum PRINT_STATUS st)
{
	if(st==PRINT_STATUS_BUFFERFULL)
		printf("!!!!!!BUFFER FULL\n");
	LTPA245.status = st;
}
//==================================================================================================
void PRN_paper_empty_process()
{
	if(LTPA245.havePaper == 1)
	{
	struct ROWMAP *pRow;
		while(LTPA245.pHead_row)
		{
			// printf("LTPA245.pHead_row free \n");
			pRow=LTPA245.pHead_row;
			LTPA245.pHead_row=LTPA245.pHead_row->next;
			free(pRow);
		}
		
		while(LTPA245.pNextFormHead)
		{
			// printf("LTPA245.pNextFormHead free \n");
			pRow = LTPA245.pNextFormHead;
			LTPA245.pNextFormHead = pRow->next;
			free(pRow);
		}
		LTPA245.pTail_row = 0;
		LTPA245.count = 0;
		LTPA245.pNextFormTail = 0;
		LTPA245.count_next = 0;
		point = 0;
		LTPA245.havePaper = 0;
		LTPA245.status = PRINT_STATUS_PAPEREMPTY;
		LTPA245.motor_status=MOT_STOP;
	}
	else
	{		
		LTPA245.havePaper = 1;
		LTPA245.status = PRINT_STATUS_IDEL;
	}
}
void PRN_ISRstatus()
{
UINT16 i=0,rd;
UCHAR tx[4];
UCHAR rx[4];
UCHAR PRN_IRQresult=0;
UCHAR data=1;
int rv,fd=TSC_fd;
	FD_ZERO(&PRN_irq_set); /* clear the set */
	FD_SET(fd, &PRN_irq_set); /* add our file descriptor to the set */
	PRN_irq_timeout.tv_sec = 0;
	PRN_irq_timeout.tv_usec = 500;//0.5ms timeout
	rv = select(fd+1, &PRN_irq_set, NULL, NULL, &PRN_irq_timeout);
	
	if (FD_ISSET(fd, &PRN_irq_set))
        {
		  rd=read(fd,&PRNevent, sizeof(PRNevent));//read interrupt Status
		}
	if(rv == -1 || rv == 0 )
	{
		//read error or time out
	}
	else
	{//å¦????select()æ²?timeout????????³NULL
		printf("IRQ occure!!! ");
		// LTPA245.motor_status=MOT_STOP;
		// while(LTPA245.motor_status!=MOT_TURNOFF);//wait until motor stop
		//check if paper empty
		tx[0]=0x45;
		tx[1]=0x09;//INTCAPA
		tx[2]=0x00;
		SPI_Transfer(tx,rx,0,EXTIO,4);
		printf("irq=0x%x\n",rx[2]);
		if((rx[2]&0x80)>0)
		{
			data=0;
			bsp_shm_ButtPressed(1,&data);
		}			
		else
		{
			data=1;
			bsp_shm_ButtPressed(1,&data);
			LTPA245.motor_status=MOT_STOP;
			PRN_paper_empty_process();
		}	
		if(rx[2]&0x01)
		{
			LTPA245.motor_status=MOT_STOP;
			PRN_paper_empty_process();
		}
			
		//check if overheat
		// tx[1]=0x19;//INTCAPB
		// SPI_Transfer(tx,rx,0,EXTIO,4);
		// PRN_IRQresult|=(rx[2]&0x04);
		// if(PRN_IRQresult&0x04)
	}	
}
//send SPI signal to get paper empty and button pressed status
//return 1 if paper empty or button pressed
//return 0 are nothing happened
UCHAR CheckIfPaperEmpty()
{
UCHAR tx[4];
UCHAR rx[4];
UCHAR data=1;
	//check if paper empty
	tx[0]=0x45;
	tx[1]=0x09;//INTCAPA
	tx[2]=0x00;
	SPI_Transfer(tx,rx,0,EXTIO,4);	
	if((rx[2]&0x80)>0)
	{
		data=0;
		bsp_shm_ButtPressed(1,&data);
	}			
	else
	{
		printf("IRQ occure!!! button pressed\n");
		data=1;
		bsp_shm_ButtPressed(1,&data);
		PRN_paper_empty_process();
		return TRUE;
	}	
	if(rx[2]&0x01)
	{
		printf("IRQ occure!!! paper empty\n");
		PRN_paper_empty_process();
		return TRUE;
	}
	return FALSE;
}
//inline 
enum PRINT_STATUS PRN_getStatus()
{
	PRN_ISRstatus();
	PRN_status=LTPA245.status;
	return LTPA245.status;
}
//==============================================================================
//inline 
void PRN_findFormFeed()
{
	LTPA245.flag |=PRN_FLAG_FORM_FEED;
}
//===============================================================================
//inline 
void PRN_releaseFormFeed()
{
	LTPA245.flag &= ~PRN_FLAG_FORM_FEED;
}
void PRN_getbitmaprow(UCHAR* pdata)
{
	//pdata=LTPA245.pHead_row->data;
	memmove(pdata,LTPA245.pHead_row->data,48);
	LTPA245.pHead_row=LTPA245.pHead_row->next;
}
UINT32 PRN_getBitmapRowLength()
{
	return LTPA245.count;
}

void motor_stop(){//printer motor stop
UCHAR tx[4];
UCHAR rx[4];
UCHAR MOTOR_STATUS;
MOTORSTOP:
	tx[0]=0x40;
	tx[1]=0x19;
	tx[2]=0xC0;//close all printer relate pins,exept 9V_SW_EN and VBATT_8V_SW(heater power source and EDC power source)
	SPI_Transfer(tx,rx,SPI_10MHz,EXTIO,4);
//check if motor pins have power down
	tx[0]=0x41;
	tx[1]=0x19;
	tx[2]=0x00;
	SPI_Transfer(tx,rx,SPI_10MHz,EXTIO,4);
	//if any printer relate pins still on,continue motor stop prcedure
	MOTOR_STATUS=(rx[2]&(~0xC0));
	point = 0;
	printf("motor_stop\n");
	if(MOTOR_STATUS!=0)
		goto MOTORSTOP;
}
struct timeval GetNowTime() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv;
}

UINT16 count_heat_time(UINT16 heatCount)
{
	if(heatCount<30)
		return(330);
	else if(heatCount<40)
		return(330);
	else if(heatCount<50)
	 	return(330);
	else if(heatCount<60)
	 	return(370);
	else if(heatCount<80)
	 	return(370);
	else if(heatCount<100)
		return(370);
	else if(heatCount<128)
		return(400);
	else if(heatCount<140)
		return(550);
	else if(heatCount<150)
		return(600);
	else if(heatCount<170)
		return(660);
	else if(heatCount<200)
		return(700);
	else if(heatCount<250)
		return(700);
	else if(heatCount<300)
		return(900);
	else
		return(PRN_MOTORstepTIME*2-50);
		// return(1000);
	
}
// UINT16 count_heat_time(UINT16 heatCount)
// {
// int difference=128/(350-230);
	
// 	if(heatCount<128)
// 	{
		
// 	}
// 		return(350);
// 	else if(heatCount<150)
// 		return(500);
// 	else if(heatCount<170)
// 		return(550);
// 	else if(heatCount<200)
// 		return(600);
// 	else if(heatCount<250)
// 		return(650);
// 	else if(heatCount<300)
// 		return(700);
// 	else
// 		return(800);
	
// }
//control motor rolling,thermal heat and data transfer
//if heat count>150,heat at step 3,7 
//if heat count<=150,heat at step 012,456 
void *PRN_MOTOR_THREAD()
{
UCHAR tx[4];
UCHAR rx[4];
UCHAR rx2[48];
const UCHAR LATCH_CODE=0x08;//latch data bit for extension IO used
const UCHAR TH_PW_CODE=0x80;//thermal head 9V power bit for extension IO used
UINT16 heatCount;
UINT16 heatCountNext;
int PRN_HeatTimeThreshold;
UCHAR heatTime=0;
UCHAR heatFlag=PRN_CMD_STROBE_STOP;
UCHAR MotorRunAWhile=0;//make sure motor are not running from beginning
UCHAR StepSum=0;
ULONG PRN_TimeCount=0;
ULONG PRN_TimeCountBak=0;
ULONG PRN_HeatTime=0;
struct timeval lasttime;
struct timeval lasttimeBak;
struct timeval heat_lasttime;
struct timeval nowtime;
struct timeval nowtimeBak;
UCHAR halfdata[48];  
UCHAR datasplitcount=0;//count which segmentation data have send.
UCHAR datatransflag=0;//record if data have transfered.
UCHAR lastbit=0;//record every byte last bit
UCHAR pre_lastbit=0;//record every byte last bit
static UCHAR motor_step=0;//count motor step.
	while(1)
	{	
		uint64_t COUNT;
		
		
		if(LTPA245.motor_status==MOT_TURNOFF)
			goto MOT_TURNOFF;
		if(LTPA245.motor_status==MOT_STOP)
			goto MOTORSTOP;
		
		// if(BSP_TMR_GetTick(PRN_TMR,COUNT)>0)
		// {
			// PRN_TimeCount++;
		// }
		// else
			// continue;
		
		
			
				
			
			if(LTPA245.pHead_row!=0)
			{
				nowtime=GetNowTime();
				// if(lasttime.tv_usec>=nowtime.tv_usec)
				// {
				// 	if(nowtime.tv_sec>lasttime.tv_sec)
				// 		PRN_TimeCount=1000000000-lasttime.tv_usec+nowtime.tv_usec;
				// 	else
				// 		PRN_TimeCount=0;
				// }
				// else if(nowtime.tv_usec>lasttime.tv_usec)
				// {
				// 	if(nowtime.tv_sec>lasttime.tv_sec)
				// 		PRN_TimeCount=1000000000-lasttime.tv_usec+nowtime.tv_usec;
				// 	else
				// 		PRN_TimeCount=nowtime.tv_usec-lasttime.tv_usec;
				// }
				if(lasttime.tv_sec<nowtime.tv_sec)
				{
					PRN_TimeCount=(nowtime.tv_sec-lasttime.tv_sec)*1000000-lasttime.tv_usec+nowtime.tv_usec;
				}
				else
				{
					PRN_TimeCount=nowtime.tv_usec-lasttime.tv_usec;
				}
				if(heat_lasttime.tv_sec<nowtime.tv_sec)
				{
					PRN_HeatTime=(nowtime.tv_sec-heat_lasttime.tv_sec)*1000000-heat_lasttime.tv_usec+nowtime.tv_usec;
				}
				else
				{
					PRN_HeatTime=nowtime.tv_usec-heat_lasttime.tv_usec;
				}
				
				//disable thermal head
				// if(heatFlag==PRN_CMD_STROBE_HEATING)
				if(heatFlag==PRN_CMD_STROBE_START)
				{
					// printf("PRN_HeatTime=%d nowtime.sec=%d nowtime.usec heat_lasttime.sec=%d heat_lasttime.usec=%d\n",PRN_HeatTime,nowtime.tv_sec,nowtime.tv_usec,heat_lasttime.tv_sec,heat_lasttime.tv_usec);
					if((PRN_HeatTime>PRN_HeatTimeThreshold)&&(heatCount<300))
					// if((PRN_HeatTime>PRN_HeatTimeThreshold)&&MotorRunAWhile)
					{
						#if PRN_DEBUG_MODE
						tx[2]=0x40;
						// printf("PRN_TimeCount=%d PRN_HeatTime=%d  LTPA245.motor_step=%d\n",PRN_TimeCount,PRN_HeatTime,LTPA245.motor_step);
						#else
						tx[2]=MOTOR_STEP_CODE[LTPA245.motor_step] |LATCH_CODE|TH_PW_CODE;					
						#endif
						heatFlag=PRN_CMD_STROBE_STOP;
					}
				}
				// printf("LTPA245.motor_step=%d\n",LTPA245.motor_step);
				// printf("LTPA245.count=%ld\n",LTPA245.count);
				LTPA245.motor_status=MOT_ROTATING;
				if(LTPA245.motor_step==0||LTPA245.motor_step==4)
				{
					heatCount=LTPA245.pHead_row->info[0]|(LTPA245.pHead_row->info[1]<<8);
					if(LTPA245.pHead_row->next)
						heatCountNext=LTPA245.pHead_row->next->info[0]|(LTPA245.pHead_row->next->info[1]<<8);//get next dot line point
					else
						heatCountNext=heatCount;
					//in case of unknow heatCount value
					heatCount=(heatCount>384)?0:heatCount;
					heatCountNext=(heatCountNext>384)?0:heatCountNext;
					PRN_MOTORstepTIME=step_motor_acceleration[point];
					// if(MotorRunAWhile)
						PRN_HeatTimeThreshold=count_heat_time(heatCount);
					// else
					// 	PRN_HeatTimeThreshold=700;//max heat time to prevent from over heating
					if((!MotorRunAWhile)&&(heatCount<300))
						PRN_HeatTimeThreshold=PRN_MOTORstepTIME/2;
				}	
					
				//if odd motor steps have run, transfer next line data
				if((datatransflag==0)&&!(LTPA245.motor_step%2))
				{
					tx[0]=0x40;
					tx[1]=0x19;
					#if PRN_DEBUG_MODE
					tx[2]=0x40;
					// printf("!!LTPA245.motor_step=%d  datasplitcount=%d\n",LTPA245.motor_step,datasplitcount);
					#endif
					// heatCount=LTPA245.pHead_row->info[0]|(LTPA245.pHead_row->info[1]<<8);
					// printf("datasplitcount=%d\n",datasplitcount);
					// printf("LTPA245.motor_step=%d  datasplitcount=%d\n",LTPA245.motor_step,datasplitcount);
					
					//>128é»?ä¸?è¡??????©æ¬¡???
					if(heatCount<128)
					{
						memset(halfdata,0x00,48);//initial print buffer
						//right shift 1 bit because first bit will left shifted when switch SPI channel after data transmission
						// for(int g=0;g<48;g++)
						// {
						// 	lastbit=LTPA245.pHead_row->data[g]&0x01;
						// 	LTPA245.pHead_row->data[g]>>=1;
						// 	LTPA245.pHead_row->data[g]|=pre_lastbit;
						// 	pre_lastbit=lastbit<<7;
						// }
						lastbit=0;
						pre_lastbit=0;	
						memmove(halfdata,&LTPA245.pHead_row->data,48);//third half data
						SPI_Transfer(halfdata,rx2,SPI_8MHz,PRINTER,48);//transfer print data	
						BSP_IO_Control(GPIO_8VBATT,0);//latch low
						datatransflag=1;
						struct ROWMAP *rm = LTPA245.pHead_row;
						LTPA245.pHead_row = LTPA245.pHead_row->next;
						free(rm);
						LTPA245.count --;
					}
					else
					{
						
						if(datasplitcount==0)
						{
							memset(halfdata,0x00,48);//initial print buffer
							//right shift 1 bit because first bit will left shifted when switch SPI channel after data transmission
							// for(int g=0;g<24;g++)
							// {
							// 	lastbit=LTPA245.pHead_row->data[g]&0x01;
							// 	LTPA245.pHead_row->data[g]>>=1;
							// 	LTPA245.pHead_row->data[g]|=pre_lastbit;
							// 	pre_lastbit=lastbit<<7;
							// }
							lastbit=0;
							pre_lastbit=0;	
							memmove(halfdata,LTPA245.pHead_row->data,24);//first half data
							SPI_Transfer(halfdata,rx2,SPI_8MHz,PRINTER,48);//tansfer print data
							BSP_IO_Control(GPIO_8VBATT,0);//latch low	
							datasplitcount=1;
							datatransflag=1;
						}
						else if(datasplitcount==1)
						{
							memset(halfdata,0x00,48);//initial print buffer
							//right shift 1 bit because first bit will left shifted when switch SPI channel after data transmission
							pre_lastbit=(LTPA245.pHead_row->data[23]&0x01)<<7;
							// for(int g=24;g<48;g++)
							// {
							// 	lastbit=LTPA245.pHead_row->data[g]&0x01;
							// 	LTPA245.pHead_row->data[g]>>=1;
							// 	LTPA245.pHead_row->data[g]|=pre_lastbit;
							// 	pre_lastbit=lastbit<<7;
							// }
							lastbit=0;
							pre_lastbit=0;
							memmove(&halfdata[24],&LTPA245.pHead_row->data[24],24);//second half data
							SPI_Transfer(halfdata,rx2,SPI_8MHz,PRINTER,48);//tansfer print data	
							BSP_IO_Control(GPIO_8VBATT,0);//latch low
							datasplitcount=2;
							datatransflag=1;
							struct ROWMAP *rm = LTPA245.pHead_row;
							LTPA245.pHead_row = LTPA245.pHead_row->next;
							free(rm);				
							LTPA245.count --;

						}
					}
					
					
				}

				
				// printf("heatCount =%d\n",heatCount);
				//if printer just start, choose max heat time to deepen the color because motor speed are slow.
				
				
				// if(LTPA245.motor_step%2)
				// 	if(CheckIfPaperEmpty())
				// 	{
				// 		PRN_paper_empty_process();
				// 		continue;
				// 	}
						
				
				
				if(PRN_TimeCount<PRN_MOTORstepTIME)
					continue;
				// usleep(100);
				// printf ( "step:%d\t count:%ld\tPRN_TimeCount=%d\t sec: %ld\t usec: %ld\n",LTPA245.motor_step,LTPA245.count, PRN_TimeCount,GetNowTime().tv_sec,GetNowTime().tv_usec );
				// printf("PRN_TimeCount=%d \n",PRN_TimeCount);
				#if PRN_DEBUG_MODE 
				// printf("PRN_TimeCount=%d  LTPA245.motor_step=%d  PRN_MOTORstepTIME=%d\n",PRN_TimeCount,LTPA245.motor_step,PRN_MOTORstepTIME);
				// if((LTPA245.motor_step==0)||(LTPA245.motor_step==4))
				if(LTPA245.motor_step%2==0)
				printf("PRN_TimeCount=%d  LTPA245.motor_step=%d  heatCount=%d  heatCountNext=%d\n",PRN_TimeCount,LTPA245.motor_step,heatCount,heatCountNext);
				// printf("PRN_TimeCount=%d  PRN_MOTORstepTIME=%d  heatCount=%d  heatCountNext=%d\n",PRN_TimeCount,PRN_MOTORstepTIME,heatCount,heatCountNext);
				// printf("PRN_TimeCount=%d  sec_difference=%d  nowtime_usec=%d lasttime_usec=%d\n",PRN_TimeCount,nowtime.tv_sec-lasttime.tv_sec,nowtime.tv_usec,lasttime.tv_usec);
				if(PRN_TimeCount>PRN_TimeCountBak)
				{
					PRN_TimeCountBak=PRN_TimeCount;
					nowtimeBak=nowtime;
					lasttimeBak=lasttime;
				}
					
				#endif
				PRN_TimeCount=0;//reset timer count	
				
				if(heatCount>0)
					StepSum++;
				if(StepSum>40)
				{
					StepSum=40;
					MotorRunAWhile=1;
				}
				if(LTPA245.motor_step==3||LTPA245.motor_step==7)
				{		
					if((heatCount<300)||(heatCountNext<300))							
						point++;
					if( point >= (MAX_MOTOR_ACCEL_STEP-10) )
						MotorRunAWhile=1;
					if( point >= MAX_MOTOR_ACCEL_STEP )
						point = MAX_MOTOR_ACCEL_STEP;
				}
				//if motor reach specific speed, start manipulating the motor speed according to the heatCount
				if((heatCount>=300)&&MotorRunAWhile&&(heatCountNext>=300))
					point--;
				if	( (point <= (MAX_MOTOR_ACCEL_STEP-16))&&MotorRunAWhile )	
					point =MAX_MOTOR_ACCEL_STEP-16;

				tx[0]=0x40;
				tx[1]=0x19;				
				if(LTPA245.motor_step%2)
				{
					#if PRN_DEBUG_MODE
					tx[2]=0x40;
					#else
					tx[2]=MOTOR_STEP_CODE[LTPA245.motor_step] |LATCH_CODE|TH_PW_CODE;
					#endif
					heatFlag=PRN_CMD_STROBE_STOP;
				}					
				else
				{	
					#if PRN_DEBUG_MODE
					tx[2]=0x40;
					#else
					tx[2]=MOTOR_STEP_CODE[LTPA245.motor_step] &~LATCH_CODE|TH_PW_CODE;
					#endif
					
					// else
					// {
					// 	if(heatCount<128)
					// 	{
					// 		heatFlag=PRN_CMD_STROBE_STOP;
					// 	}
					// 	heat_lasttime=GetNowTime();
					// }
				}			
				// lasttime=GetNowTime();
				lasttime=nowtime;
				BSP_IO_Control(GPIO_8VBATT,1);//latch high
				SPI_Transfer(tx,rx,SPI_10MHz,EXTIO,4);
				if(!(LTPA245.motor_step%2))
					//enable thermal head
					if(heatFlag==PRN_CMD_STROBE_STOP)
					{
						heatFlag=PRN_CMD_STROBE_START;
						#if PRN_DEBUG_MODE
						#else
						if(datasplitcount==2)
							PRN_HeatTimeThreshold+=50;//heat longer to prevent from white fence on printing content.
						ioctl(STBdriver_fd, IOCTL_PRN_ISR_CHECK ,&PRN_HeatTimeThreshold);//start thermal timer
						#endif
						heat_lasttime=GetNowTime();
					}
				LTPA245.motor_step++;
				
				if(!(LTPA245.motor_step%2))//if heatcount>128 will transfer data two times, so reset datatransflag every 2 steps
					if(heatCount>=128)
						datatransflag=0;
				if(LTPA245.motor_step>7)
					LTPA245.motor_step=0;
				if((LTPA245.motor_step==0)||(LTPA245.motor_step==4))
				{
					// if(datasplitcount!=2)
					// 	printf("datasplitcount=%d\n",datasplitcount);
					datasplitcount=0;//reset flag to input next data line
					datatransflag=0;
				}
					
				// lasttime=GetNowTime();
			}
			else
			{	
				LTPA245.motor_status = MOT_STOP;			
				LTPA245.pTail_row = 0;
				if(LTPA245.count_next!=0)
				{
					printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
					//sometimes PRN_Run() will be called by PRN_insert_rows() and PRN_MOTOR_THREAD(), and it will cause content lost.
					//Adding flag lock to avoid double calling PRN_Run() will fix content lost problem,
					// but it seldom cause conditional lock and motor stop without power off, and it might burn the motor.20211007 by West
					if(PRNRUN_flag==0)//check if PRN_Run() not called by main thread
					{
						PRNRUN_flag=1;
						PRN_Run();
					}		
					PRNRUN_flag=0;//init flag				
				}		
				else
				{	
					printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
					motor_stop();
					LTPA245.status = PRINT_STATUS_COMPLETE; 
			MOTORSTOP: 
					motor_stop();
			MOT_TURNOFF:			
					LTPA245.motor_status = MOT_TURNOFF;
					MotorRunAWhile=0;//initialize flag
					StepSum=0;//initialize flag
					lasttime=GetNowTime();
				}
			}
		
		if(PRN_releaseTIMER)
		{
			#if PRN_DEBUG_MODE
			printf("PRN_releaseTIMER\n");
			printf("PRN_TimeCountBak=%d\n",PRN_TimeCountBak);
			printf("nowtimeBak_sec=%d nowtimeBak_usec=%d  lasttimeBak_sec=%d lasttimeBak_usec=%d\n",nowtimeBak.tv_sec,nowtimeBak.tv_usec,lasttimeBak.tv_sec,lasttimeBak.tv_usec);
			#endif
			// FILE *ptr;
			// ptr = fopen("/home/root/prn_dump","wb");
			// fwrite(prn_dumpbuffer,prn_count,1,ptr);
			// fclose(ptr);
			BSP_TMR_GetTick(PRN_TMR,&COUNT);	
			PRN_releaseTIMER=0;
			goto MOTORSTOP;			
		}
	}
}
RESPONSE PRN_Start()
{
UCHAR tx[4];
UCHAR rx[4];
UCHAR EXTIO_status;
BSP_STATUS status;
UCHAR data=1;
UCHAR *SHM_data=NULL;
MODE_t PRN_ioctlMode = GET_RESOURCE;
	if(TSC_fd<1)
	{
		bsp_shm_acquire(SHM_data);
		bsp_shm_IrqOccupied(1,&data);
		usleep(1000);//wait a while for waiting daemon close /dev/input/event2
		TSC_fd = open("/dev/input/event2", O_RDWR);
	}
		
	if(TSC_fd<0)
		return RETURN_ERROR;
	if(STBdriver_fd==0)
	{
		STBdriver_fd= open("/dev/Printer",O_RDONLY);
		if(STBdriver_fd<0)
		{
			STBdriver_fd=0;
			return RETURN_ERROR;
		}
		ioctl(STBdriver_fd, IOCTL_SWITCH_MODE ,&PRN_ioctlMode);	
	}
	if(OStimerFlag)
	{
		OStimerFlag=0;//20210830 add by west. Terminate others but printer pthread, in case of CPU resource shortage 
		pthread_join(OS_Timer_Thread, NULL);//wait pthread exit
		OS_Timer_Thread=0;
	}
//set paper and thermal ISR
	tx[0]=0x44;
	tx[1]=0x05;//IOCON
	tx[2]=0xE8;//BANK=1,MIRROR=1,SEQOP=1,HAEN=1
	SPI_Transfer(tx,rx,0,EXTIO,4);
//get interrupt setting before overwrite it.
	tx[0]=0x45;
	tx[1]=0x02;//GPINTENA
	tx[2]=0x00;
	SPI_Transfer(tx,rx,0,EXTIO,4);
	PRN_ISRIO_INTENA_bak=rx[2];
	tx[1]=0x04;//INTCONA
	SPI_Transfer(tx,rx,0,EXTIO,4);
	PRN_ISRIO_INTCONA_bak=rx[2];
	//set paper empty sensor and temperature sensor interrupt pin enable
	//and disable other interrupt pin
	//20210628 add power button interrupt
	tx[0]=0x44;
	tx[1]=0x02;//GPINTENA
	tx[2]=0x81;//paper sensor and power button
	SPI_Transfer(tx,rx,0,EXTIO,4);
	tx[1]=0x03;//DEFVALA
	tx[2]=0x80;//the paper sensor pin go high when paper empty, and power button pin go low when button pressed
	SPI_Transfer(tx,rx,0,EXTIO,4);
	//INTCON configs same as GPINTEN
	tx[1]=0x04;//INTCONA
	tx[2]=0x81;//paper sensor and power button
	SPI_Transfer(tx,rx,0,EXTIO,4);
	//read gpio status to reset interrupt pin
	tx[0]=0x45;
	tx[1]=0x09;//INTCAPA
	tx[2]=0x00;
	SPI_Transfer(tx,rx,0,EXTIO,4);
	// tx[1]=0x12;//GPINTENB
	// tx[2]=0x04;//temperature sensor
	// SPI_Transfer(tx,rx,0,EXTIO,4);
	// tx[1]=0x14;//INTCONB
	// SPI_Transfer(tx,rx,0,EXTIO,4);
	/*
	 *set interrupt pins base status value
	 *when paper exist,paper sensor base value is 0.Value rise to 1 when paper empty.
	 *temperature sensor base value is 1,value drop to 0 when overheat occure.
	 */
	// tx[0]=0x44;
	// tx[1]=0x13;//DEFVALB
	// tx[2]=0x04;//make bit 3 to 1
	// SPI_Transfer(tx,rx,0,EXTIO,4);
//set EXTIO config

	//read direction status 
	tx[0]=0x41;
	tx[1]=0x10;
	tx[2]=0x00;
	SPI_Transfer(tx,rx,SPI_10MHz,EXTIO,4);
	EXTIO_status=rx[2]&0x50;//except STB and 8v switch,enable printer relate IO direction
	// printf("EXTIO_status=%x\n",EXTIO_status);
//set printer relate IO direction
	tx[0]=0x40;
	tx[1]=0x10;
	tx[2]=EXTIO_status;
	SPI_Transfer(tx,rx,SPI_10MHz,EXTIO,4);
	LTPA245.havePaper = 1;
	if((LTPA245.status != PRINT_STATUS_PAPEREMPTY)&&(LTPA245.status != PRINT_STATUS_BUFFERFULL))
		LTPA245.status = PRINT_STATUS_IDEL;
	
//acquire timer
	PRN_TMR=BSP_TMR_Acquire3(1);//1us timer
	if(PRN_TMR)
	{
		status=BSP_TMR_Start(PRN_MOTOR_THREAD,PRN_TMR);
		if(status != BSP_SUCCESS)
			return RETURN_ERROR;
	}	
	else
		return RETURN_ERROR;	
		
	PRN_releaseTIMER=0;
	LTPA245.motor_status = MOT_TURNOFF;
	return RETURN_OK;
}
RESPONSE PRN_Stop()
{
	motor_stop();
	LTPA245.motor_status = MOT_TURNOFF;
}
RESPONSE PRN_Release()
{
UCHAR tx[4],rx[4],data;
	motor_stop();
//release timer device
	PRN_releaseTIMER=1;
	OS_EnableTimer1();//restart OS timer pthread
	BSP_TMR_Stop( PRN_TMR );
	data=0;
	bsp_shm_IrqOccupied(1,&data);//release irq event flag
//restore interrupt setting before open printer device.
	tx[0]=0x40;
	tx[1]=0x19;
	tx[2]=0x40;//close all printer relate pins,exept VBATT_8V_SW(EDC power source)
	SPI_Transfer(tx,rx,SPI_10MHz,EXTIO,4);
	tx[0]=0x44;
	tx[1]=0x02;//GPINTENA
	tx[2]=PRN_ISRIO_INTENA_bak&(~0x01);//restore interrupt pin set except paperempty.
	SPI_Transfer(tx,rx,0,EXTIO,4);
	tx[1]=0x04;//GPINTCONA
	tx[2]=PRN_ISRIO_INTCONA_bak;
	SPI_Transfer(tx,rx,0,EXTIO,4);
}
enum RETURN_VALUE PRN_init()
{
UCHAR tx[4],rx[4];
//reset IRQ
	tx[0]=0x45;
	tx[1]=0x09;//INTCAPA
	tx[2]=0x00;
	SPI_Transfer(tx,rx,0,EXTIO,4);
	LTPA245.havePaper = 1;
	if((LTPA245.status != PRINT_STATUS_PAPEREMPTY)&&(LTPA245.status != PRINT_STATUS_BUFFERFULL))
		LTPA245.status = PRINT_STATUS_IDEL;
	
	PRN_releaseTIMER=0;
	LTPA245.motor_status = MOT_STOP;
	
	memset(&LTPA245,0,sizeof(struct PRINT));
	LTPA245.havePaper = 1;
	

	return RETURN_OK;
}
void PRN_Run()
{
	if(LTPA245.motor_status == MOT_ROTATING)
		return;
	// printf("LTPA245.count_next=%ld\n",LTPA245.count_next);
	// printf("LTPA245.flag&PRN_FLAG_FORM_FEED=%ld\n",LTPA245.flag&PRN_FLAG_FORM_FEED);
	printf("PRN_Run\n");
// PATCH: 2009-10-01, to avoid dot lines squeezed
	
	if((LTPA245.count_next < 30) && ((LTPA245.flag&PRN_FLAG_FORM_FEED)==0) )
////	if((LTPA245.count_next < 300) && ((LTPA245.flag&PRN_FLAG_FORM_FEED)==0) )
		return;
	
	PRN_getStatus();
	// if( LTPA245.motor_status == MOT_STOP )
	//   prn_motor_stop = TRUE;
	if(LTPA245.status == PRINT_STATUS_PAPEREMPTY)
		return;
	
	LTPA245.motor_status = MOT_ROTATING;
	LTPA245.status = PRINT_STATUS_BUSY;
	// point = 0;
	//???ç¢ºå??è³?ä¸­å??ç¨?
	// PRNRUN_flag=1;
	if((LTPA245.flag&PRN_FLAG_FORM_FEED) !=0)//have form feed
	{
		// printf("have form feed\n");
		LTPA245.pHead_row = LTPA245.pNextFormHead;
		LTPA245.pTail_row = LTPA245.pNextFormTail;
		LTPA245.count = LTPA245.count_next;
		LTPA245.pNextFormHead = NULL;//20201111 =0 change to =NULL in case of linux double free core dump
		LTPA245.pNextFormTail = NULL;
		LTPA245.count_next = NULL;
		// printf("have form feed finished\n");
	}
	else
	{//remain part rows
		// printf("remain part rows\n");
		struct ROWMAP *ptail = LTPA245.pNextFormTail;
		struct ROWMAP *ptail_bak;
		int i;
		prn_reserved_dot_lines = 0;
		for(i=0;i<84;i++)
		{
			ptail_bak=ptail;
			prn_reserved_dot_lines++;
			ptail = ptail->last;
			if( (ptail->last == 0)||(ptail->next == 0) )
			{
				ptail=ptail_bak;
				break;
			}
		}
		LTPA245.pHead_row = LTPA245.pNextFormHead;
		LTPA245.pTail_row = ptail->last;
		LTPA245.pNextFormHead = ptail;
		LTPA245.pTail_row->next = 0;
		prn_dot_lines_to_be_printed = LTPA245.count_next - prn_reserved_dot_lines;	// dot lines to be printed
		LTPA245.count = prn_dot_lines_to_be_printed;
		if( prn_dot_lines_to_be_printed )
		  LTPA245.count_next -= prn_dot_lines_to_be_printed;	// remaining dot lines in queue
		else
		  LTPA245.count_next = 0;
		
	}
	LTPA245.motor_status = MOT_ROTATING;
	if((LTPA245.status != PRINT_STATUS_PAPEREMPTY)&&(LTPA245.status != PRINT_STATUS_BUFFERFULL))
		LTPA245.status = PRINT_STATUS_BUSY;
	// if( prn_motor_stop )
	//   {
	//   prn_motor_stop = FALSE;
	//   goto PRN_RUN_0100;
	//   }
	if( LTPA245.motor_status != MOT_STOP )
	{
		// printf("MOT_START\n");		
		printf("\033[1;31;40mMOT_START\033[0m\n");
		LTPA245.motor_step = 0;
		LTPA245.motor_status =MOT_START;
	}	
	// for(int j=0;j<LTPA245.count;j++)
	// {
		// printf("j=%d\n",j);
		// for(int i=0;i<48;i++)
			// printf("LTPA245.pHead_row->data[%d]=%x\n",i,LTPA245.pHead_row->data[i]);
		// LTPA245.pHead_row=LTPA245.pHead_row->next;
	// }
// PRN_RUN_0100:
#ifndef	PRN_DEBUG
//	LTPA245.ctrl_reg = PRN_MOT_phase[LTPA245.motor_step]|PRN_CMD_EN_9V|PRN_CMD_EN_LATCH;
	// os_CS7_DATA &= 0xFF00;	// 2016-03-01
	// os_CS7_DATA |= PRN_MOT_phase[LTPA245.motor_step]|PRN_CMD_EN_9V|PRN_CMD_EN_LATCH;
	// LTPA245.ctrl_reg = os_CS7_DATA;
#else
	LTPA245.ctrl_reg = PRN_MOT_phase[LTPA245.motor_step]|PRN_CMD_EN_LATCH;
#endif
	//BSP_WR16(EBI_PORT7_BASE,LTPA245.ctrl_reg);//change control signal
	;
	// point = 0;
	//??«æ??è¨»è§£20201029
	// LTPA245.pTmr_motor->Compare = setupTimer_uS(4291);
	// BSP_TMR_Start(LTPA245.pTmr_motor);	
	// prn_time_tick = 0;
	// prn_sub_strobe_cnt = 0;
	//??«æ??è¨»è§£20201029
	// PRNRUN_flag=0;
}
void PRN_MotorSpeedSetup(unsigned long mode,struct ROWMAP *head,unsigned long size)//speed level 0~25
{	
	
	if( mode == 0xFF )
	  point = 0;
	return;  
	  

	struct ROWMAP *rows = head;
	int countOLD = 0;
	int old_SpLvModify = 0;
	int index;
	int speedLv;
	if(mode == 0xFF)//???ç¨¼å??ç·????è«?æ¨¡ä?????
	{
		point = 0;
		speedLv = 26;
	}
	else//dynamic program mode
	{
		countOLD = point;
		speedLv = speedtable[countOLD-1];
	}
	//copy speed LV value to table
	while(rows!=0)
	{
		speedtable[point++]=rows->motor_time1;
		speedtable[point++]=rows->motor_time2;
		rows = rows->next;
	}

	for(index = countOLD;index<point;index++)
	{
		if(speedLv>speedtable[index])
		{
			speedtable[index] = --speedLv;
		}
		else
		{
			speedLv = speedtable[index];
		}
	}
	for(index = point-2;index>=countOLD;index--)
	{
		if(speedLv>speedtable[index])
		{
			speedtable[index] = --speedLv;
		}
		else
			speedLv = speedtable[index];
	}
	old_SpLvModify = 0;
	if(mode!=0xff)
	{
		if(speedtable[countOLD]>speedtable[countOLD-1])
		{
			speedLv = speedtable[countOLD];
			index = countOLD-1;
			while(speedLv!=speedtable[index])
			{
				old_SpLvModify++;
				if(speedLv>speedtable[index])
				{
					speedtable[index] = --speedLv;
				}
				index--;
//				else
//					speedLv = speedtable[index];
			}
			rows = LTPA245.pNextFormTail;
			index = 0;
			while(old_SpLvModify>index)
			{
				index++;
				rows->motor_time2 = speedtable[countOLD-index];
				index++;
				rows->motor_time1 = speedtable[countOLD-index];
				rows = rows->last;
			}
		}
	}
	rows = head;
	index = 0;
	while(rows!=0)
	{
		rows->motor_time1 = speedtable[countOLD+index];
		index++;
		rows->motor_time2 = speedtable[countOLD+index];
		index++;
		rows = rows->next;
	}
	if(point>=32)
	{
		memmove(speedtable,&speedtable[point-32],32);
		point = 32;
	}
}
//inline 
RESPONSE PRN_insert_rows(unsigned long size,struct ROWMAP *head,struct ROWMAP *tail)
{
UINT32	IntState;


	if(LTPA245.status == PRINT_STATUS_PAPEREMPTY)
		return RETURN_FATAL_FAIL;
	// for(int i=0;i<48;i++)
	// printf("head->data[%d]=%x\n",i,head->data[i]);
	// printf("LTPA245.count_next=%ld\n",LTPA245.count_next);
	// printf("PRN_insert_rows\n");
	if(LTPA245.count_next==0)
	{
		// printf("LTPA245.count_next==0\n");
		PRN_MotorSpeedSetup(0xff,head,size);
		LTPA245.pNextFormHead = head;
		LTPA245.pNextFormTail = tail;
		LTPA245.count_next = size;
	}
	else
	{
		// printf("else\n");
		PRN_MotorSpeedSetup(0,head,size);
		head->last = LTPA245.pNextFormTail;
		LTPA245.pNextFormTail->next = head;

		LTPA245.pNextFormTail = tail;
		LTPA245.count_next += size;
	}
	//sometimes PRN_Run() will be called by PRN_insert_rows() and PRN_MOTOR_THREAD(), and it will cause content lost.
	//Adding flag lock to avoid double calling PRN_Run() will fix content lost problem,
	// but it seldom cause conditional lock and motor stop without power off, and it might burn the motor.20211007 by West
	if(PRNRUN_flag==0)//check if PRN_Run() not called by thread
	{
		PRNRUN_flag=1;
		PRN_Run();
	}
	PRNRUN_flag=0;//init flag	
	return RETURN_OK;
}