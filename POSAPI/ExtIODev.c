/*
 *for AS350+ V3(NAND Flash version) 
 */
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "bsp_gpio.h"
#include "bsp_types.h"
#include "ExtIODev.h"

static const char *device = "/dev/spidev0.0";
static uint32_t mode;
static uint8_t bits = 8;
static uint8_t argu=2;
static char *input_file;
static char *output_file;
static uint32_t speed = SPI_10MHz;//max speed for extension IO
static uint16_t delay;
static int transfer_size;
static int SPI_fd;

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static void transfer(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
	int ret;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.delay_usecs = 0,
		.speed_hz = SPI_8MHz,
		.bits_per_word = bits,
		.cs_change=0,
	};
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	
	if (ret < 1)
		pabort("can't send spi message");

}
//for Extension IO,one read/write request transfer must be 3 bytes total(one of 24bit)
static void transfer_ExtIO(int fd, uint8_t *tx, uint8_t const *rx)
{
int ret;
*(tx+3)=*(tx+2);
// if(tx[0]!=0x45)
// printf("tx=0x%x 0x%x 0x%x\n",tx[0],tx[1],tx[2]);
struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)tx,
	.rx_buf = (unsigned long)rx,
	.len = 4,
	.delay_usecs = 0,
	.speed_hz = SPI_10MHz,
	.bits_per_word = 8,
	.cs_change=0,
};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	
	if (ret < 1)
		pabort("can't send spi message");

}
#ifdef PCD_PLATFORM_CLRC663
//for RC663(NFC),one read/write request transfer must be 2 bytes total(two of 8bit)
static void transfer_NFC(int fd, uint8_t const  *tx, uint8_t const  *rx)
{
int ret;
struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)tx,
	.rx_buf = (unsigned long)rx,
	.len = 2,
	.delay_usecs = 0,
	.speed_hz = SPI_10MHz,
	.bits_per_word = 16,
	.cs_change=0,
};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	
	if (ret < 1)
		pabort("can't send spi message");
}
#else
static void transfer_NFC_PN5180(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
int ret;
struct spi_ioc_transfer tr = {
	.tx_buf = (unsigned long)tx,
	.rx_buf = (unsigned long)rx,
	.len = len,
	.delay_usecs = 0,
	.speed_hz = SPI_12MHz,//from datasheet, clock could only be 8 12 16 24 Mhz.
	.bits_per_word = 8,
	.cs_change=0,
};
	ret=0;
	// ioctl(fd, SPI_IOC_CS_PIN, &ret);
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	
	if (ret < 1)
		pabort("can't send spi message");
	ret=1;
	// ioctl(fd, SPI_IOC_CS_PIN, &ret);
}
#endif
void SPI_init(void)
{
int fd,ret;
	if(SPI_fd==0)
	{
		SPI_fd=open(device, O_RDWR);
		if (SPI_fd < 0)
		{
			pabort("can't open device");
			return;
		}
		fd=SPI_fd;
		mode |= SPI_NO_CS;/* 20220810 change chip select pin status with ioctl and not use native CS*/
		/*
		* SET spi mode
		*/
		ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
		if (ret == -1)
			pabort("can't set spi mode");
	
		ret = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
		if (ret == -1)
			pabort("can't get spi mode");
	
		/*
		* SET bits per word
		*/
		ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
		if (ret == -1)
			pabort("can't set bits per word");
	
		ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
		if (ret == -1)
			pabort("can't get bits per word");			
	
		ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
		if (ret == -1)
			pabort("can't set max speed hz");
	
		ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
		if (ret == -1)
			pabort("can't get max speed hz");	
		printf("spi mode: 0x%x\n", mode);
	}	
}
void SPI_close(void)
{
	if(SPI_fd==0)
		return;
	close(SPI_fd);
}
void SPI_Transfer(UINT8 *tx,UINT8 *rx,UINT32 SPEED,UINT8 switchMODE,UINT32 length){
int ret = 0;
int fd;
UINT8 TXtemp[2];
UINT8 RXtemp[2];

	SPI_init();
	fd=SPI_fd;
	if(BSP_IO_SpiSwitchMode(switchMODE)!=BSP_SUCCESS)
	{
		pabort("BSP_IO_SpiSwitchMode fail");
		return;
	}
	

	/*
	 * SET max speed hz
	 */
	
	//printf("tx=%02x%02x%02x\n",*tx,*(tx+1),*(tx+2));
	if(switchMODE==EXTIO)
		transfer_ExtIO(fd, tx, rx);
	else if(switchMODE==NFC)
	{
	#ifdef PCD_PLATFORM_CLRC663
		TXtemp[0]=*(tx+1);
		TXtemp[1]=*tx;
		transfer_NFC(fd, TXtemp, RXtemp);
		*rx=RXtemp[1];
		*(rx+1)=RXtemp[0];
	#else
		transfer_NFC_PN5180(fd, tx, rx, length);
	#endif
	}
	else
		transfer(fd, tx, rx, length);
	
}