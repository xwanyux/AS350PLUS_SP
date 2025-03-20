#ifndef _BSP_GPIO_H_
#define _BSP_GPIO_H_
#include "bsp_types.h"
#include "POSAPI.h"
//GPIO num
#define GPIO_MAXNUM     10
//GPIO direction
#define IODIR_IN		0
#define IODIR_OUT		1

//GPIO value
#define IOVALUE_HIGH	1
#define IOVALUE_LOW		0

//SPI switch mode
#define EXTIO			0x00
#define NFC				0x01
#define PRINTER			0x02
#define LCM				0x03

//SPI switch 74HC4052BQ select IO pin
// #define EXTIO_SEL0		30//GPIO1_IO30
#define EXTIO_SEL0		0//GPIO1_IO30
#define EXTIO_SEL1		1//GPIO1_IO31
// #define EXTIO_SEL1		31//GPIO1_IO31
#define EXTIO_RDP1		19//GPIO1_IO19
#define EXTIO_RCP1		18//GPIO1_IO18
#define EXTIO_RDP2		23//GPIO1_IO23
#define EXTIO_RCP2		22//GPIO1_IO22
#define EXTIO_RDP3		27//GPIO1_IO27
#define EXTIO_RCP3		26//GPIO1_IO26
// #define GPIO_STB		117//GPIO4_IO21 printer thermal head
#define GPIO_STB		2//GPIO4_IO21 printer thermal head
#define GPIO_8VBATT		3//GPIO4_IO14 printer thermal head
#define GPIO_ISR		120//GPIO4_IO24 interrupt used EXTIO
#define GPIO_SPI_CS		4//GPIO4_IO26 ecspi1 chip select pin
#define GPIO_8VBATT_V2		5//GPIO4_IO20 (V2 PCB)
//GPIO control path
#define EXTIO_SEL0_VALUE      "/sys/class/gpio/gpio30/value"
#define EXTIO_SEL1_VALUE      "/sys/class/gpio/gpio31/value"
#define GPIO_STB_VALUE        "/sys/class/gpio/gpio117/value"
#define GPIO_8VBATT_VALUE     "/sys/class/gpio/gpio110/value"
#define GPIO_8VBATT_VALUE_V2  "/sys/class/gpio/gpio116/value"	// V2 PCB
//GPIO export path
#define GPIO_EXPORT           "/sys/class/gpio/export"
//GPIO IO num
#define EXTIO_SEL0_IONUM      30
#define EXTIO_SEL1_IONUM      31
#define GPIO_STB_IONUM        117
#define GPIO_8VBATT_IONUM     110	// V3 PCB
#define GPIO_8VBATT_IONUM_V2  116	// V2 PCB
#define GPIO_SPI_CS_IONUM	  122//GPIO4_IO26 ecspi1 chip select pin

typedef	struct	BSP_GPIO_S		// TFT LCD parameters
{
	UCHAR   fd;//file descriptor of /sys/class/gpio/gpioxx/value
	UCHAR   IOnum;

} __attribute__((packed)) BSP_GPIO;
//function
BSP_STATUS BSP_IO_Acquire(UINT8 IOnum,UINT8 DIR);
BSP_STATUS BSP_IO_Control(UINT8 IOnum,UINT8 Value);
BSP_STATUS BSP_IO_Status(UINT8 IOnum);
BSP_STATUS BSP_IO_SpiSwitchMode(UINT8 mode);
BSP_STATUS BSP_IO_Init();
BSP_STATUS BSP_IO_Release(UINT8 IOnum);
#endif