
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "POSAPI.h"
#include "ECL_LV1_Define.h"
#include "ExtIODev.h"
#include "bsp_gpio.h"
#define SPI_TIMER_BUSY			300

UCHAR	spi_flgOpen=FALSE;

#define NXP_SERIAL_RW_BUFFER			512
ULONG   baud_rate = 8000000; //default

extern	ULONG	OS_GET_SysTimerFreeCnt( void );
extern	ULONG	Init_PN5180_BusyPin(void);
extern	ULONG	Get_PN5180_BusyStaus( void );
extern  int TSC_fd;//from DEV_TSC.c
//disable busy pin interrupt
void SPI_Close( void )
{
UCHAR tx[4];
UCHAR rx[4];
//make sure extIO configures are set properly
	tx[0]=0x44;
	tx[1]=0x05;//IOCON
	tx[2]=0xE8;//BANK=1,MIRROR=1,SEQOP=1,HAEN=1
	SPI_Transfer(tx,rx,0,EXTIO,4);
	
	//get which interrupt pin enabled
	tx[0]=0x45;
	tx[1]=0x12;//GPINTENB
	tx[2]=0x00;
	SPI_Transfer(tx,rx,0,EXTIO,4);
	tx[2]=rx[2];
	tx[2]&=~0x40;//make bit7 to 0
	tx[0]=0x44;
	SPI_Transfer(tx,rx,0,EXTIO,4);
	
	//INTCON configs same as GPINTEN
	tx[1]=0x14;//INTCONB
	tx[2]&=~0x40;//make bit 7 to 0
	SPI_Transfer(tx,rx,0,EXTIO,4);
	
	
}


UCHAR SPI_Open(ULONG bauRate)
{
	baud_rate = bauRate;
	
UCHAR tx[4];
UCHAR rx[4];
//make sure extIO configures are set properly
	tx[0]=0x44;
	tx[1]=0x05;//IOCON
	tx[2]=0xE8;//BANK=1,MIRROR=1,SEQOP=1,HAEN=1
	SPI_Transfer(tx,rx,0,EXTIO,4);
	
	//get which interrupt pin enabled
	tx[0]=0x45;
	tx[1]=0x12;//GPINTENB
	tx[2]=0x00;
	SPI_Transfer(tx,rx,0,EXTIO,4);
	tx[2]=rx[2];
	tx[2]|=0x40;//make bit7 to 1
	tx[0]=0x44;
	SPI_Transfer(tx,rx,0,EXTIO,4);
	
	//INTCON configs same as GPINTEN
	tx[1]=0x14;//INTCONB
	tx[2]|=0x40;//make bit 7 to 1
	SPI_Transfer(tx,rx,0,EXTIO,4);
	
	
	//get interrupt pins base status
	tx[0]=0x45;
	tx[1]=0x13;//DEFVALB
	tx[2]=0x00;
	SPI_Transfer(tx,rx,0,EXTIO,4);
	tx[2]=rx[2];
	tx[2]&=~0x40;//make bit 7 to 0
	tx[0]=0x44;
	SPI_Transfer(tx,rx,0,EXTIO,4);

	//open interrupt pin handle driver
	// if(TSC_fd<1){
	// 	bsp_shm_acquire(SHM_data);//power button stuff
	// 	bsp_shm_IrqOccupied(1,&data);
	// 	TSC_fd = open("/dev/input/event2", O_RDWR);
	// 	if(TSC_fd==-1)
	// 		return( FALSE );
	// }
	return ECL_LV1_SUCCESS;
}


void SPI_Read(UINT datLen, UINT *datBuff)
{
	UCHAR *rx=malloc(sizeof(UCHAR)*datLen*2);
	SPI_Transfer(datBuff,rx,SPI_8MHz,NFC,datLen);
	free(rx);
}


void SPI_Write(UINT datLen, UINT *datBuff)
{
	UCHAR *rx=malloc(sizeof(UCHAR)*datLen*2);
	SPI_Transfer(datBuff,rx,SPI_8MHz,NFC,datLen);
	free(rx);
}


UCHAR SPI_Transmit(UINT datLen, UCHAR *datBuffer)
{
	UCHAR *rx=malloc(sizeof(UCHAR)*datLen*2);//rx len twice longer than tx because of recieve data are follow transfer data 
	SPI_Transfer(datBuffer,rx,SPI_8MHz,NFC,datLen);

	memmove(datBuffer, rx, datLen);

	free(rx);


	return ECL_LV1_SUCCESS;
}


UCHAR SPI_Transmit_DebugMode(UINT datLen, UCHAR *datBuffer)
{
	BSP_STATUS	Status=0;
	UINT32		DataRead=0;
	UINT32		tmrTick1=0, tmrTick2=0;


	tmrTick1=OS_GET_SysTimerFreeCnt();

	do
	{
		if (Get_PN5180_BusyStaus() == 0) break;	//0 (Low) means Idle
		
		tmrTick2=OS_GET_SysTimerFreeCnt()-tmrTick1;
	} while (tmrTick2 < SPI_TIMER_BUSY);

	if (tmrTick2 >= SPI_TIMER_BUSY) return ECL_LV1_FAIL;

	SPI_Transmit(datLen, datBuffer);

	tmrTick1=OS_GET_SysTimerFreeCnt();

	do
	{
		if (Get_PN5180_BusyStaus()) break;
		
		tmrTick2=OS_GET_SysTimerFreeCnt()-tmrTick1;
	} while (tmrTick2 < SPI_TIMER_BUSY);

	if (tmrTick2 >= SPI_TIMER_BUSY) return ECL_LV1_FAIL;

	return ECL_LV1_SUCCESS;
}

