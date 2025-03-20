/*
 * Copyright 2006, ZiLOG Inc.
 * All Rights Reserved
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ZiLOG Inc., and might
 * contain proprietary, confidential and trade secret information of
 * ZiLOG, our partners and parties from which this code has been licensed.
 * 
 * The contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of ZiLOG Inc.
 */

#include "POSAPI.h"
#include "bsp_uart.h"
#include "bsp_types.h"
#include <stdio.h>   /* I/O Definitions                    */
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>  /* Standard Symbolic Constants        */
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <string.h>  /* String Manipulation Definitions    */
#include <errno.h>   /* Error Code Definitions             */
#include <time.h>		//for count tob

#define RX_FLOW_OFF_THRESHOLD				32
#define XON_CHAR								0x11
#define XOFF_CHAR								0x13

#define	ACK					0x06

void RxFlowOff( BSP_UART * pUart );
void RxFlowOn( BSP_UART * pUart );
BSP_UART							UartTable[ BSP_MAX_UARTS ];
BSP_UART	*glpUart=NULLPTR;

//DSS
extern UCHAR DSS_APPbuffer[DSS_MAX_APP_SIZE];
extern ULONG     DownFileSize;   //Record the information of file size from "request response packet"
extern ULONG     DownReceiveSize;//Record the received size
ULONG     DSS_recieveData;//Record the received size
ULONG     DSS_recieveData_bak;//Record the received size
UCHAR	  BSP_UART_Rxreadyflag=0;
extern struct timeval GetNowTime();
//=====================================================================================
UINT32							UartGpio[ BSP_MAX_UARTS ] =
{
	(UART_0_CTS | UART_0_RTS | UART_0_DCD | UART_0_DTR | UART_0_DSR | UART_0_RI),
	(UART_1_CTS | UART_1_RTS),
	(UART_2_RX | UART_2_TX | UART_2_CTS | UART_2_RTS)
};


UINT32 BSP_UART_GetRxAvail( BSP_UART * pUart )
{
	UINT32						Count = (UINT32)BSP_FAILURE;
	UINT32						Rld;
	UINT32						Cfg;
	UINT32						IntLevel;
	//BSP_DMA					 * pRxDma = pUart->pRxDma;

/*
	if( pUart && (pUart == &UartTable[pUart->UartNum]) && (pUart->StartSem == FALSE) )
	{
		if( pUart->Mode == UART_MODE_IRQ )
		{
			IntLevel = BSP_DisableInterrupts( BSP_INT_MASK );
			Count = pUart->Avail;
			BSP_RestoreInterrupts( IntLevel );
			return( Count );
		}

		if( pUart->Mode == UART_MODE_POLL )
		{
			Count  = BSP_RD32( pUart->Base | UART_REG_LSR ) & LSR_DR;
			return( Count );
		}

		/*
		 * Dma Mode
		 */
		 /*
		IntLevel = BSP_DisableInterrupts( BSP_INT_MASK );

		/*
		 * Disable DMA while determining remaining DMA xfer count.
		 */
		 /*
		Cfg = BSP_RD32( pRxDma->Base| DMA_CFG_REG );
		BSP_WR32( (pRxDma->Base| DMA_CFG_REG), (Cfg & 0xFFFFFFFE) );
		Count  = BSP_RD32( pRxDma->Base| DMA_CNT_REG );
		Rld = BSP_RD32( pRxDma->Base| DMA_CRLD_REG );
		BSP_WR32( (pRxDma->Base| DMA_CFG_REG), Cfg );
		if( Rld & 0x10000 )
		{
			Count += (Rld & 0xFFFF);
		}

		/*
		 * The amount of data available to be read is:
	    * = Amount of unread data in the receive buffer
		 *   + (Programmed DMA Count - Remaining DMA Count)
		 *   - Amount of data already read
		 */
		 /*
		Count = pUart->Avail + pUart->Count - Count - pUart->Read;

		if( Count > (pUart->BufferSize - RX_FLOW_OFF_THRESHOLD) )
		{
			RxFlowOff( pUart );
		}
		
		BSP_RestoreInterrupts( IntLevel );
	}
	return( Count );
	*/
	return( 1 );
}



UINT32 TotalRead = 0;
UINT32 BSP_UART_Read( BSP_UART * pUart, UINT8 * pBuf, UINT32 Max )
{
	UINT32						BufSize = pUart->BufferSize;
	int							ifsigned=0;//to determine whether the read() return value is signed number
	UINT32						len=0;
	UINT8						uart_buf[10];
	if(Max>10)
	{
		// printf("BSP_UART_Read invalid input 'Max'\n");
	}
	if( pUart && (pUart == &UartTable[pUart->UartNum]) )
	{
		ifsigned=read(pUart->Fd, uart_buf, sizeof(UINT8));
		// printf("pUart->Fd=%d\n",pUart->Fd);
		if(ifsigned==-1){//read nothing return -1;or return read byte length
			return( 0 );
		}
		// printf("error[%d]:%s\n",errno,strerror(errno));
		//ignore null byte
		// if((*uart_buf)==0)
			// return( 0 );
			
		// if((*uart_buf>0x20)&&(*uart_buf<0x7f))
		// 	printf("*pBuf= %c\n",*uart_buf);
		// else
		// 	printf("*pBuf= 0x%x\n",*uart_buf);
		if(Max>1)
			len=read(pUart->Fd, uart_buf+1, Max-1);
		
		len+=ifsigned;//add data length read() before
		if(len==0&&ifsigned==1)//if input data only 1 length long,variable will get -1 after second read().
			len=1;
		memmove(pBuf,uart_buf,len);
		Max=len;
	}
	else
	{
		Max = 0;
	}
		return( Max );
}
#ifdef _build_DSS_
//for DSS
UINT32 BSP_UART_Rxready( BSP_UART * pUart )
{	
int							ifsigned=0;//to determine whether the read() return value is signed number
UINT32						PastTime=0;
UINT8						uart_buf[10];
UINT8						*pBuf;
struct timeval 				lasttime,nowtime;//for count tob

	pBuf=DSS_APPbuffer;
	nowtime=GetNowTime();
	lasttime=GetNowTime();
	// printf("DSS UART fd=%d\n",pUart->Fd);
	// write(pUart->Fd, test_buf, sizeof(test_buf));
	// printf("error[%d]:%s\n",errno,strerror(errno));
	if( pUart && (pUart == &UartTable[pUart->UartNum]) )
	{
		BSP_UART_Rxreadyflag=0;
		while(1)
		{
			if(ifsigned>0)
			{
				// printf("ifsigned=%d\n",ifsigned);
				lasttime=nowtime;
				DownReceiveSize+=ifsigned;//add data length read() before
				printf("DownReceiveSize=%d\n",DownReceiveSize);
				// for(int j=0;j<DownReceiveSize;j++)
				// 	printf("0x%02x ",pBuf[j]);
			}
				

			nowtime=GetNowTime();
			
			ifsigned=read(pUart->Fd, pBuf+DownReceiveSize, 32767);//32767 is maximum value of signed interger 
			//read nothing return -1;or return read byte length
			//if read something update timestamp.
			// printf("error[%d]:%s\n",errno,strerror(errno));
			//if timeout return data length
			if(lasttime.tv_sec<nowtime.tv_sec)
			{
				PastTime=(nowtime.tv_sec-lasttime.tv_sec)*1000000-lasttime.tv_usec+nowtime.tv_usec;
			}
			else
			{
				PastTime=nowtime.tv_usec-lasttime.tv_usec;
			}
			if(PastTime>2000000)//2s timeout
			{
				BSP_UART_Rxreadyflag=1;//read done
				printf("\nTOB>2sec DownReceiveSize=%d\n",DownReceiveSize);
				return DownReceiveSize;
			}

			//ignore null byte
			// if((*uart_buf)==0)
				// return( 0 );
			// if(*pBuf!=0xd8)
			// 	if((*pBuf>0x20)&&(*pBuf<0x7f))
			// 		printf("*pBuf= %c\n",*pBuf);
			// 	else
			// 		printf("*pBuf= 0x%x\n",*pBuf);

			
			// if(DownReceiveSize>5)
			// 	DownFileSize= pBuf[1] | ((ULONG)pBuf[2]<<8)| ((ULONG)pBuf[3]<<16)| ((ULONG)pBuf[4]<<24);
			// return 0;//still have data incomming,return 0 means not ready
		}
	}
	else
	{
		return( 0 );
	}
		
}
//for DSS
UINT32 BSP_UART_Rxready2( BSP_UART * pUart )
{	
int							ifsigned=0;//to determine whether the read() return value is signed number
UINT32						PastTime=0;
UINT8						uart_buf[10];
UINT8						*pBuf;
struct timeval 				lasttime,nowtime;//for count tob
	pBuf=DSS_APPbuffer;
	nowtime=GetNowTime();
	lasttime=GetNowTime();
	// printf("DSS UART fd=%d\n",pUart->Fd);
	// write(pUart->Fd, test_buf, sizeof(test_buf));
	// printf("error[%d]:%s\n",errno,strerror(errno));
	if( pUart && (pUart == &UartTable[pUart->UartNum]) )
	{
		BSP_UART_Rxreadyflag=0;
		// while(1)
		// {
			

			nowtime=GetNowTime();
			
			ifsigned=read(pUart->Fd, pBuf+DSS_recieveData_bak, 32767);//32767 is maximum value of signed interger 
			if(ifsigned>0)
			{
				// printf("ifsigned=%d\n",ifsigned);
				lasttime=nowtime;
				DSS_recieveData+=ifsigned;//add data length read() before
				DSS_recieveData_bak+=ifsigned;
				BSP_UART_Rxreadyflag=1;//read done
				printf("DSS_recieveData_bak=%d\n",DSS_recieveData_bak);
				return DSS_recieveData;
			}
			else
				return( DSS_recieveData );	
			//read nothing return -1;or return read byte length
			//if read something update timestamp.
			// printf("error[%d]:%s\n",errno,strerror(errno));
			//if timeout return data length
			if(lasttime.tv_sec<nowtime.tv_sec)
			{
				PastTime=(nowtime.tv_sec-lasttime.tv_sec)*1000000-lasttime.tv_usec+nowtime.tv_usec;
			}
			else
			{
				PastTime=nowtime.tv_usec-lasttime.tv_usec;
			}
			if(PastTime>900000)//900ms timeout
			{
				BSP_UART_Rxreadyflag=1;//read done
				printf("DownReceiveSize=%d\n",DownReceiveSize);
				return DSS_recieveData;
			}

			//ignore null byte
			// if((*uart_buf)==0)
				// return( 0 );
			// if(*pBuf!=0xd8)
			// 	if((*pBuf>0x20)&&(*pBuf<0x7f))
			// 		printf("*pBuf= %c\n",*pBuf);
			// 	else
			// 		printf("*pBuf= 0x%x\n",*pBuf);

			
			if(DownReceiveSize>5)
				DownFileSize= pBuf[1] | ((ULONG)pBuf[2]<<8)| ((ULONG)pBuf[3]<<16)| ((ULONG)pBuf[4]<<24);
			// return 0;//still have data incomming,return 0 means not ready
		// }
	}
	else
	{
		return( 0 );
	}
		
}
UINT32 BSP_UART_Rxstring( BSP_UART * pUart )
{
	DownReceiveSize=0;
}
#endif
BSP_STATUS BSP_UART_Write( BSP_UART * pUart, UINT8 * pData, UINT32 Len )
{
	BSP_STATUS	Status;
	// printf("send data=\n");
	// for(int g=0;g<Len;g++)
	// 	if((pData[g]>0x20)&&(pData[g]<0x7f))
	// 		printf("pData[%d]= %c\n",g,pData[g]);
	// 	else
			// printf("pData[%d]= 0x%x\n",g,pData[g]);
	if(write(pUart->Fd, pData, Len)==-1)
		Status = BSP_FAILURE;
	else
		Status = BSP_SUCCESS;
	
	return( Status );
}

BSP_UART *
BSP_UART_Acquire
(
	UINT8						UartNum,
	UART_ISR_FUNC				pIsr
)
{
BSP_UART	*pUart = NULLPTR;
int 		fd;
UINT16		flag;
	flag=O_RDWR |  O_NDELAY;/* O_RDWR Read/write access to the serial port */
							/* O_NOCTTY No terminal will control the process */
							/* O_NDELAY Use non-blocking I/O */

	if( UartNum < BSP_MAX_UARTS )
	{
			/*
			 * Open uart port
			 */			
			if(UartNum==COM0)
				fd = open("/dev/ttymxc0", flag);
			else if(UartNum==COM1)
				fd = open("/dev/ttymxc1", flag);
			else if(UartNum==COM2)
			//	fd = open("/dev/ttymxc3", flag);
				fd = open("/dev/ttymxc2", flag);	// 2023-04-25, M6
			else if(UartNum==COM3)
				fd = open("/dev/ttymxc3", flag);	// 2023-07-11, M6 for LTE (L610)
			else
				return( pUart );
			
			if(fd==-1){//open() fail return -1
				return( pUart );
			}
			pUart = &UartTable[ UartNum ];
			/*
			 * Setup default Uart configuration
			 */
			switch( UartNum )
			{
				case COM0:
					
					pUart->UartNum=0;
					break;
	           
				case COM1:

					pUart->UartNum=1;
					break;
				case COM2:

					pUart->UartNum=2;
					break;
				case COM3:

					pUart->UartNum=3;
					break;
				default:
					return(pUart);
				}
			pUart->pIsrFunc		 = pIsr;
			pUart->Fd		   = fd;
			pUart->Mode        = UART_MODE_IRQ;
			pUart->Baud        = 115200;
			pUart->DataBits    = 8;
			pUart->StopBits    = 1;
			pUart->Parity      = UART_PARITY_NONE;
			pUart->BufferSize  = UART_BUFF_SIZE;
			pUart->FlowControl = UART_FLOW_NONE;
			//pUart->GpioConfig  = UartGpio[ pUart->UartNum ];

			pUart->RxCallLevel = 0;
	}
	glpUart=pUart;
	return( pUart );
}



BSP_STATUS
BSP_UART_Start
(
	BSP_UART					 * pUart
)
{
BSP_STATUS					Status = BSP_SUCCESS;
UINT32						Temp;
UINT32						BufSize = pUart->BufferSize;
UINT16	Bbaud;	
UINT8	Bdatabits = 0;
UINT8	fd;
struct	termios   tty;
	

	if( pUart && (pUart == &UartTable[pUart->UartNum]) )
	{
		
		/*
		 * Validate Uart Parameters
		 */
		if( ((pUart->DataBits < 5) || (pUart->DataBits > 8))
		 || ((pUart->StopBits < 1) || (pUart->StopBits > 2))
		 || (pUart->Parity > UART_PARITY_EVEN)
		 || (pUart->FlowControl > UART_FLOW_XON_XOFF)
	  	 || (pUart->Mode > UART_MODE_DMA) )
		{
			Status = BSP_INVALID_PARAMETER;
		};
		fd=pUart->Fd;
		tcgetattr(fd, &tty);//get device port default attribute setting
		/*
		 *determine attribute setting
		 */
		switch( pUart->Baud )
	        {
	        case 300:
				 Bbaud=	B300;
	             break;
	        case 600:
	             Bbaud = B600;
	             break;
	        case 1200:
	             Bbaud = B1200;
	             break;
	        case 2400:
	             Bbaud = B2400;
	             break;
	        case 4800:
	             Bbaud = B4800;
	             break;
	        case 9600:
	             Bbaud = B9600;
	             break;
	        case 19200:
	             Bbaud = B19200;
	             break;
	        case 38400:
	             Bbaud = B38400;
	             break;
	        case 57600:
	             Bbaud = B57600;
	             break;
	        case 115200:
	             Bbaud = B115200;
	             break;
	        case 230400:
	             Bbaud = B230400;
	             break;
	        
	        default:
	             Bbaud = B9600;
	        }
	  
	  switch( pUart->DataBits )
	        {
	        case 7:
	             Bdatabits= CS7;
	             break;
	        case 8:
	             Bdatabits = CS8;
	             break;
	        default:
	             Bdatabits = CS8;
	        }
	  
	  switch( pUart->StopBits )
	        {
	        case 1:
				 tty.c_cflag &= ~CSTOPB;
	             break;
	        case 2:
				 tty.c_cflag |=  CSTOPB;
	             break;
	        default:
				 tty.c_cflag &= ~CSTOPB;
	        }
	        
	  switch( pUart->Parity )
	        {
	        case UART_PARITY_NONE:
				 tty.c_cflag &= ~PARENB;   /* Clear parity enable */ 
                 tty.c_iflag &= ~INPCK;    /* Enable parity checking */
	             break;
	        case UART_PARITY_ODD:
				 tty.c_cflag |= (PARODD | PARENB);  /* è¨­ç½®??ï¿½ï¿½?????ï¿??*/  
                 tty.c_iflag |= INPCK;             /* Disable parity checking */ 				 
	             break;
	        case UART_PARITY_EVEN:
				 tty.c_cflag |= PARENB;     /* Enable parity */ 
                 tty.c_cflag &= ~PARODD;   	/* ï¿???????ï¿½ï¿½?ï¿½ï¿½??ï¿??*/   
                 tty.c_iflag |= INPCK;       /* Disable parity checking */ 
	             break;
	        default:
				 tty.c_cflag &= ~PARENB;   /* Clear parity enable */ 
                 tty.c_iflag &= ~INPCK;    /* Enable parity checking */
	        }
		cfsetispeed(&tty, Bbaud); /* Set read speed as Bbaud */
		cfsetospeed(&tty, Bbaud); /* Set write speed as Bbaud*/
		tty.c_cflag |= (CLOCAL | CREAD);   /* enable the receiver and set local mode */
		tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);    /* Raw input mode, sends the raw and unprocessed data  ( send as it is) */
		tty.c_cflag &= ~CSIZE;    /* Using mask to clear data size setting   */
		tty.c_cflag |=  Bdatabits;/* Set data bits                           */
		tty.c_cflag &= ~CRTSCTS;  /* Disable Hardware Flow Control           */
		
		tty.c_iflag &= ~(INLCR | ICRNL); // 
		tty.c_iflag &= ~(IXON | IXOFF | IXANY);
		tty.c_oflag &= ~OPOST;
		
		if((tcsetattr(fd, TCSANOW, &tty)) != 0){ /* Write the configuration to the termios structure*/
			Status=BSP_FAILURE;
		}else{
			Status=BSP_SUCCESS;
		}
			/*
			 * Initialize Error Counters and flow control
			 */
			pUart->BreakErrors   = 0;
			pUart->FramingErrors = 0;
			pUart->ParityErrors  = 0;
			pUart->OverrunErrors = 0;
			pUart->RxSpillErrors = 0;

		}
	if( Status != BSP_SUCCESS )
	{
		/*
		 * Release resources
		 */
		//BSP_UART_Stop( pUart );
	}
	return( Status );
}



BSP_STATUS 
BSP_UART_Stop
( 
	BSP_UART					 * pUart 
)
{
	BSP_STATUS					Status = BSP_SUCCESS;
		
			
				if( pUart->pRxBuffer )
				{
					//free( pUart->pRxBuffer );
				}
				if( pUart->pTxBuffer )
				{
					//free( pUart->pTxBuffer );
				}
	return( Status );
}



BSP_STATUS 
BSP_UART_Release
( 
	BSP_UART					 * pUart 
)
{
	BSP_STATUS					Status = BSP_FAILURE;
		//BSP_UART_Stop( pUart );



		if( close( pUart->Fd ) == 0 )
			Status = BSP_SUCCESS;
		
		//pUart->pIsrFunc = NULLPTR;
	glpUart=NULLPTR;
	return( Status );
}

#if	0	// 2023-02-11, re-defined in DBTOOL.c
void	_DEBUGPRINTF( char *Msg, ... )
{
char PrintfBuffer[300];
	va_list list_ptr;
	va_start(list_ptr,Msg);
	vprintf(Msg,list_ptr);
	va_end(list_ptr);
	// printf("%s",PrintfBuffer);
}
#endif

UINT8 BSP_UART_SendAck( UINT8 dhn, UINT8 acknum)
{
UINT8 ackbuf=0x06;
	if(write(dhn,&ackbuf, acknum)==-1)
		return( FALSE );
	else
		return( TRUE );
	
		
}
/*
BSP_STATUS
BSP_UART_SetModemControl
(

	BSP_UART					 * pUart,
   UINT32						Flags
)
{
	BSP_STATUS					Status;


	if( pUart && (pUart == &UartTable[pUart->UartNum]) && (pUart->StartSem == FALSE) )
	{
		Flags &= ( MCTL_RTS | MCTL_DTR );
		BSP_WR32( pUart->Base | UART_REG_MCTL, Flags );
		Status = BSP_SUCCESS;
	}
	else
	{
		Status = BSP_FAILURE;
	}
	return( Status );
}



BSP_STATUS
BSP_UART_GetModemStatus
(
	BSP_UART					 * pUart
)
{
	BSP_STATUS					Status;


	if( pUart && (pUart == &UartTable[pUart->UartNum]) && (pUart->StartSem == FALSE) )
	{
		pUart->ModemStatus = BSP_RD32( pUart->Base | UART_REG_MSR );
		Status = BSP_SUCCESS;
	}
	else
	{
		Status = BSP_FAILURE;
	}
	return( Status );
}
*/
