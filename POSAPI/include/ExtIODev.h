#ifndef _EXTIODEV_H_
#define _EXTIODEV_H_
#include "bsp_types.h"
#include "POSAPI.h"
#include <linux/ioctl.h>
#define	SPI_16MHz	16000000
#define	SPI_12MHz	12000000
#define	SPI_10MHz	10000000
#define	SPI_8MHz	8000000
/* 20220810 add function which is change chip select pin status with ioctl*/
#define SPI_IOC_CS_PIN		_IOW(SPI_IOC_MAGIC, 6, __u32)
// #define	SPI_10MHz	25000000
// #define	SPI_8MHz	25000000
void SPI_Transfer(UINT8 *tx,UINT8 *rx,UINT32 SPEED,UINT8 switchMODE,UINT32 length);

#endif