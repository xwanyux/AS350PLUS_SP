#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "bsp_types.h"
#include "bsp_gpio.h"
UINT8 BSP_IO_flag=0xff;
static UINT8 BSP_IO_init_flag=0;
BSP_GPIO BSP_GPIO_FD[GPIO_MAXNUM];
// extern UINT8 PRN_ISRfd;
BSP_STATUS BSP_IO_Acquire(UINT8 IOnum,UINT8 DIR)
{
int fd;
UINT8 ASCII[10];
UINT8 DIRstring[40]="/sys/class/gpio/gpio";
UINT8 InputNumLen;
//check if input valid 
	if(IOnum>159)
		return(BSP_INVALID_PARAMETER);
	
	if(DIR>IODIR_OUT)
		return(BSP_INVALID_PARAMETER);
//enable GPIO	
	fd= open("/sys/class/gpio/export", O_WRONLY);
	if (fd == -1)
        return(BSP_FAILURE);
	
	sprintf(ASCII, "%d", IOnum);
	if(IOnum<10)
		InputNumLen=1;
	else if(IOnum<100)
		InputNumLen=2;
	else
		InputNumLen=3;
	if (write(fd, ASCII, InputNumLen) <0)
			return(BSP_FAILURE);
	close(fd);
//set GPIO direction e.g.,/sys/class/gpio/gpio27/direction
	sprintf(&DIRstring[20], "%d", IOnum);
	if(IOnum<10)
	sprintf(&DIRstring[21], "%s", "/direction");
	else if(IOnum<100)
	sprintf(&DIRstring[22], "%s", "/direction");
	else
	sprintf(&DIRstring[23], "%s", "/direction");

	fd = open(DIRstring, O_WRONLY);
    if (fd == -1)
		return(BSP_FAILURE);
	if(DIR==IODIR_IN)
	{
		if (write(fd, "in", 2) != 2)
			return(BSP_FAILURE);
	}
	else
	{
		if (write(fd, "out", 3) != 3)
		return(BSP_FAILURE);
	}
	close(fd);
	return(BSP_SUCCESS);
}
/*
BSP_STATUS BSP_IO_ISR_Acquire()
{
int fd;
UINT8 ASCII[10];
UINT8 DIRstring[40]="/sys/class/gpio/gpio";
UINT8 InputNumLen;

//enable GPIO	
	fd= open("/sys/class/gpio/export", O_WRONLY);
	if (fd == -1)
        return(BSP_FAILURE);
	printf("open done");
	sprintf(ASCII, "%d", 120);
	
	write(fd, ASCII, 3);
	
	printf("write done\n");
	fd= open("/sys/class/gpio/gpio120/value", O_RDONLY);
	PRN_ISRfd=fd;
	return(BSP_SUCCESS);	
}
*/
BSP_STATUS BSP_IO_Release(UINT8 IOnum)
{
int fd;
UINT8 ASCII[10];
UINT8 DIRstring[40]="/sys/class/gpio/gpio";
//check if input valid 
	if(IOnum>159)
		return(BSP_INVALID_PARAMETER);
	
//disable GPIO	
	fd= open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd == -1)
        return(BSP_FAILURE);
	
	sprintf(ASCII, "%d", IOnum);
	if (write(fd, ASCII, 2) != 2)
		return(BSP_FAILURE);
	
	close(fd);
	
	return(BSP_SUCCESS);	
}
// BSP_STATUS BSP_IO_Control(UINT8 IOnum,UINT8 Value)
// {
// int fd;
// UINT8 VALUEstring[30]="/sys/class/gpio/gpio";
// //check if input valid 
// 	if(IOnum>159)
// 		return(BSP_INVALID_PARAMETER);
	
// 	if(Value>IOVALUE_HIGH)
// 		return(BSP_INVALID_PARAMETER);
// //set GPIO value e.g.,/sys/class/gpio/gpio27/value
// 	sprintf(&VALUEstring[20], "%d", IOnum);
// 	if(IOnum<10)
// 	sprintf(&VALUEstring[21], "%s", "/value");
// 	else if(IOnum<100)
// 	sprintf(&VALUEstring[22], "%s", "/value");
// 	else
// 	sprintf(&VALUEstring[23], "%s", "/value");
// 	fd = open(VALUEstring, O_WRONLY);
//     if (fd == -1)
// 		return(BSP_FAILURE);
// 	if(Value==IOVALUE_LOW)
// 	{
// 		if (write(fd, "0", 1) != 1)
// 			return(BSP_FAILURE);
// 	}
// 	else
// 	{
// 		if (write(fd, "1", 1) != 1)
// 		return(BSP_FAILURE);
// 	}
	
// 	close(fd);
// 	return(BSP_SUCCESS);
// }
BSP_STATUS BSP_IO_Control(UINT8 IOnum,UINT8 Value)
{
int fd;
	fd=BSP_GPIO_FD[IOnum].fd;
	
		if (write(fd, Value?"1":"0", 1) != 1)
			return(BSP_FAILURE);
	
	return(BSP_SUCCESS);
}
BSP_STATUS BSP_IO_Status(UINT8 IOnum)
{
int fd;
UINT8 VALUEstring[30]="/sys/class/gpio/gpio";
UINT8 value=0;
//check if input valid 
	if(IOnum>159)
		return(BSP_INVALID_PARAMETER);
//get GPIO value e.g.,/sys/class/gpio/gpio27/value
	sprintf(&VALUEstring[20], "%d", IOnum);
	sprintf(&VALUEstring[22], "%s", "/value");
	fd = open(VALUEstring, O_RDONLY);
    if (fd == -1)
		return(BSP_FAILURE);
	if (read(fd, &value,1) == 0)
		return(BSP_FAILURE);
	
	//value-=30;//ascii to integer
	close(fd);
	return (value-48);
}

BSP_STATUS BSP_IO_SpiSwitchMode(const UINT8 mode)
{
INT8 CTL_status0,CTL_status1;
	if(mode>LCM)
		return(BSP_INVALID_PARAMETER);
	if(BSP_IO_flag==mode)
		return(BSP_SUCCESS);
	
	if(mode==EXTIO)
	{
		CTL_status0=BSP_IO_Control(EXTIO_SEL0,IOVALUE_LOW);
		CTL_status1=BSP_IO_Control(EXTIO_SEL1,IOVALUE_HIGH);
		if((CTL_status0!=BSP_SUCCESS)||(CTL_status1!=BSP_SUCCESS))
			return(BSP_BUSY);
	}
	else if(mode==NFC)
	{
		CTL_status0=BSP_IO_Control(EXTIO_SEL0,IOVALUE_LOW);
		CTL_status1=BSP_IO_Control(EXTIO_SEL1,IOVALUE_LOW);
		if((CTL_status0!=BSP_SUCCESS)||(CTL_status1!=BSP_SUCCESS))
			return(BSP_BUSY);
	}
	else if(mode==PRINTER)
	{
		CTL_status0=BSP_IO_Control(EXTIO_SEL0,IOVALUE_HIGH);
		CTL_status1=BSP_IO_Control(EXTIO_SEL1,IOVALUE_LOW);
		
		if((CTL_status0!=BSP_SUCCESS)||(CTL_status1!=BSP_SUCCESS))
			return(BSP_BUSY);
	}
	else if(mode==LCM)
	{
		CTL_status0=BSP_IO_Control(EXTIO_SEL0,IOVALUE_HIGH);
		CTL_status1=BSP_IO_Control(EXTIO_SEL1,IOVALUE_HIGH);
		if((CTL_status0!=BSP_SUCCESS)||(CTL_status1!=BSP_SUCCESS))
			return(BSP_BUSY);
	}
	else
		return(BSP_FAILURE);
	
	BSP_IO_flag=mode;
	return(BSP_SUCCESS);
}
BSP_STATUS BSP_IO_Init()
{
	int fd;
	if(BSP_IO_init_flag)
		return(BSP_SUCCESS);
	//enable 74HC4052BQ selection pin
	/*
	if
	(	
	(BSP_IO_Acquire(EXTIO_SEL0,IODIR_OUT)!=BSP_SUCCESS)||
	(BSP_IO_Acquire(EXTIO_SEL1,IODIR_OUT)!=BSP_SUCCESS)||
	(BSP_IO_Acquire(GPIO_STB,IODIR_OUT)!=BSP_SUCCESS)
	// (BSP_IO_Acquire(EXTIO_RDP1,IODIR_IN)!=BSP_SUCCESS)||
	// (BSP_IO_Acquire(EXTIO_RCP1,IODIR_IN)!=BSP_SUCCESS)||
	// (BSP_IO_Acquire(EXTIO_RDP2,IODIR_IN)!=BSP_SUCCESS)||
	// (BSP_IO_Acquire(EXTIO_RCP2,IODIR_IN)!=BSP_SUCCESS)||
	// (BSP_IO_Acquire(EXTIO_RDP3,IODIR_IN)!=BSP_SUCCESS)||
	// (BSP_IO_Acquire(EXTIO_RCP3,IODIR_IN)!=BSP_SUCCESS)
	)
	return(BSP_FAILURE);
	*/
	BSP_GPIO_FD[EXTIO_SEL0].IOnum=EXTIO_SEL0_IONUM;
	BSP_GPIO_FD[EXTIO_SEL1].IOnum=EXTIO_SEL1_IONUM;
	BSP_GPIO_FD[GPIO_8VBATT].IOnum=GPIO_8VBATT_IONUM;
	BSP_IO_Acquire(BSP_GPIO_FD[EXTIO_SEL0].IOnum,IODIR_OUT);
	BSP_IO_Acquire(BSP_GPIO_FD[EXTIO_SEL1].IOnum,IODIR_OUT);
	BSP_IO_Acquire(BSP_GPIO_FD[GPIO_8VBATT].IOnum,IODIR_OUT);
	// BSP_IO_Acquire(BSP_GPIO_FD[GPIO_STB].IOnum,IODIR_OUT);
	fd = open(EXTIO_SEL0_VALUE, O_WRONLY);
	if(fd<0)
		return(BSP_FAILURE);
	BSP_GPIO_FD[EXTIO_SEL0].fd=fd;

	fd = open(EXTIO_SEL1_VALUE, O_WRONLY);
	if(fd<0)
		return(BSP_FAILURE);
	BSP_GPIO_FD[EXTIO_SEL1].fd=fd;

	fd = open(GPIO_8VBATT_VALUE, O_WRONLY);
	if(fd<0)
		return(BSP_FAILURE);
	BSP_GPIO_FD[GPIO_8VBATT].fd=fd;
	// fd = open(GPIO_STB_VALUE, O_WRONLY);
	// if(fd<0)
	// 	return(BSP_FAILURE);
	// BSP_GPIO_FD[GPIO_STB].fd=fd;
	// BSP_IO_ISR_Acquire();
	BSP_IO_init_flag=1;
	return(BSP_SUCCESS);
}
// int main(int argc, char *argv[])
// {
// 	int fd;
// 	FILE *f;
// UINT8 VALUEstring[30]="/sys/class/gpio/gpio30/value";
// BSP_IO_Init();
// printf("init done\n");
// 	// f = fopen(VALUEstring, 'w');
// 	// fd = open(VALUEstring, O_WRONLY);
//     // if (fd == -1)
// 	// 	return(BSP_FAILURE);
// 	while(1)
// 	{
// 		BSP_IO_Control(EXTIO_SEL0,IOVALUE_LOW);
// 		// fflush(fd);
// 		usleep(5);
// 		BSP_IO_Control(EXTIO_SEL0,IOVALUE_HIGH);
// 		usleep(5);
// 		// fputs("0", f);
// 		// // fflush(f);
// 		// usleep(500000);
// 		// fputs("1", f);
// 		// // fflush(f);
// 		// usleep(500000);
// 	}
// 	// fclose(f);
// 	return(BSP_SUCCESS);
// }