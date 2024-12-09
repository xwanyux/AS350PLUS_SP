
#define Linux_env




#include "POSAPI.h"
#include "ECL_LV1_Define.h"

#include <stdio.h>
#include <stdlib.h>
#include "ExtIODev.h"
#include <string.h>
#include "bsp_gpio.h"


#define NXP_SERIAL_RW_BUFFER			512


UCHAR	spi_flgOpen=FALSE;
ULONG   baud_rate = 8000000; //default

UCHAR SPI_Transmit(unsigned int datLen, unsigned char *datBuffer);
void SPI_Read_BaudRate(unsigned char *regData);
extern UCHAR NXP_Read_Register_Serial(UCHAR *iptAddress, UINT iptLen, UCHAR *optData);
extern void NXP_Read_Register(UCHAR regAddress, UCHAR *regData);
extern UCHAR NXP_Check_SPI_RC663(void);

void show_Tx_Rx(UCHAR* tx, UCHAR* rx, int len){

	printf("Tx:");
	for(int i = 0; i < len; i++)
		printf("%2x",*(tx+i));
	printf("\n");

	printf("Rx:");
	for(int i = 0; i < len; i++)
		printf("%2x",*(rx+i));
	printf("\n");

}

void SPI_test1(){

	// test of basic function of SPI (read baurd rate)

	UCHAR baudRate[2] = {0};
	SPI_Read_BaudRate(baudRate);

	printf("BaudRate:%x\n",baudRate[0]);

	NXP_Check_SPI_RC663();

}


/*
in 350+, we have three channel using 
*/
void select_CLTS_channel_for_SPI(){

}

UCHAR SPI_Open(unsigned long iptBaudrate)
{
	baud_rate = iptBaudrate;
}

UCHAR SPI_Close(void)
{
}


UCHAR SPI_Transmit(unsigned int datLen, unsigned char *datBuffer)
{
	UCHAR *rx=malloc(sizeof(UCHAR)*datLen);
	SPI_Transfer(datBuffer,rx,SPI_8MHz,NFC,datLen);

	memmove(datBuffer, rx, datLen);

	free(rx);


	return ECL_LV1_SUCCESS;

}
UCHAR mml_spi_transmit(int not_use, UCHAR* datBuffer, unsigned int dataLen ){

	UCHAR *rx=malloc(sizeof(UCHAR)*dataLen);
	SPI_Transfer(datBuffer,rx,SPI_8MHz,NFC,dataLen);
	memmove(datBuffer, rx, dataLen);

	free(rx);


	return ECL_LV1_SUCCESS;

}
void SPI_Read_BaudRate(unsigned char *regData)
{
	UCHAR cmdBuff[8] = {0};
	cmdBuff[0]=((0x3B << 1) + 1);
	cmdBuff[1]=0x00;
	SPI_Transmit(2, cmdBuff);
	regData[0]=cmdBuff[1] & 0x00FF;
}
