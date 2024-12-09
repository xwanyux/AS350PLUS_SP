#include <linux/input.h>
//#include "bsp_gpio.h"
#include "POSAPI.h"
#include "ExtIODev.h"
#include "bsp_gpio.h"
#include "API_EXTIO.h"
BSP_GPIO	*pPN5180_BUSY;			// GPIO_1[10]
extern int TSC_fd;
UCHAR NFC_BusyPinIRQstatus()
{
UINT16 i=0,rd;
UCHAR tx[4];
UCHAR rx[4];
UCHAR IRQresult=0;
UCHAR data=1;
int rv,fd=TSC_fd;
fd_set NFC_irq_set;
struct input_event NFCevent[5];
struct timeval NFC_irq_timeout;
	FD_ZERO(&NFC_irq_set); /* clear the set */
	FD_SET(fd, &NFC_irq_set); /* add our file descriptor to the set */
	NFC_irq_timeout.tv_sec = 0;
	NFC_irq_timeout.tv_usec = 500;//0.5ms timeout
	rv = select(fd+1, &NFC_irq_set, NULL, NULL, &NFC_irq_timeout);
	
	if (FD_ISSET(fd, &NFC_irq_set))
        {
		  rd=read(fd,&NFCevent, sizeof(NFCevent));//read interrupt Status
		}
	if(rv == -1 || rv == 0 )
	{
		//read error or time out
	}
	else
	{//å¦????select()æ²?timeout????????³NULL
		// printf("IRQ occure!!! ");
		// LTPA245.motor_status=MOT_STOP;
		// while(LTPA245.motor_status!=MOT_TURNOFF);//wait until motor stop
		//check if paper empty
		tx[0]=0x45;
		tx[1]=0x19;//GPIOB
		tx[2]=0x00;
		SPI_Transfer(tx,rx,0,EXTIO,4);
		printf("irq=0x%x\n",rx[2]);
		if((rx[2]&0x40)==0)//if busy pin are low
		{

		}
			
	}	
}
// ---------------------------------------------------------------------------
// FUNCTION: Initialize PN5180 BUSY pin.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE/FALSE
// ---------------------------------------------------------------------------
ULONG	Init_PN5180_BusyPin( void )
{
	//Extention IO chip MCP23S17 default setting is input
	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Get the status of PN5180 BUSY pin.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : the status of the busy pin (0 or 1)
// NOTE	   : In AS350X6, the busy pin is connected to extension IO.
//			 This function will wait interrupt event, and check if the event is caused by busy pin.
// ---------------------------------------------------------------------------
ULONG	Get_PN5180_BusyStaus( void )
{
UCHAR value;
	
	EXTIO_GPIO_Read(_PN5180_BUSY_BANK_ADDR, _PN5180_BUSY_GPIO_PORT, _PN5180_BUSY_GPIO_NUM, &value);
	return( value );
}
