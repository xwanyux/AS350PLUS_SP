#include "bsp_types.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_alg.h>
#include <linux/socket.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
// #include <crypto/if_alg.h>

#include "API_EXTIO.h"
#include "fbutils.h"
#include <string.h>
#include "ExtIODev.h"
#include "bsp_gpio.h"
#define MMAP_DATA_SIZE 1

UINT8 * BSP_SHM_data=NULL;

/* share memory format(1 byte) 
**      bit7                     bit6                              bit5                                   bit4                         bit3                   bit2                           bit1                             bit0
**--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
**      RFU     |    if battery charging flag       |  if battery broken flag           |    adapter flag                    |             SPI gate bit                |     both AP and daemon can set.        | both AP and daemon can set.
**              |    if battery charging bit6=1     |  if battery not applied or broken |    if power source is adpter,      |             00 = EXTIO port             |     This bit means AP detect interrupt | indicate whether AP using TSC or PRN API.
**              |    if not charging bit6=0         |  ,the bat_st_in will alternatively|    set bit4=1,and brightness level |             01 = NFC port               |     ,and daemon will start timer.      | if 1 means TSC or PRN opened,pwrbut_daemon 
**              |                                   |  high and low in 0.5hz.In this    |    is 7.                           |             10 = printer port           |     If time count over 3 sec,daemon    | can't read interrupt event(event2 maybe) and
**              |                                   |  situation,the SHM bit5 will      |    if power source is battery      |             11 = RFU                    |     will read power button status to   | send any SPI signal.
**              |                                   |   be set 0                        |    set bit4=0,and brightness level |             check bsp_gpio.c for        |     check if button still be pressed.  | if 0 means TSC or PRN API are not running or
**              |                                   |  If battery alive,the bat_st_in   |    is 6.                           |             more information.           |     If button still pressed,power off. | system in idle state (AP may crush or AP not running).
**              |                                   |  pin will be constantly 0 or 1.   |                                    |                                         |     If button no longer pressed,set bit| pwrbut_daemon will read interrupt event and send
**              |                                   |  In this case,the SHM bit5 will   |                                    |                                         |     to 0.                              | SPI signal to check if power button pressed
**              |                                   |  be set 1.                        |                                    |                                         |                                        | over 3 second.
**              |                                   |                                   |                                    |                                         |                                        | If daemon detect no AP running,set this bit to 0
*/
//get share memory address
BSP_STATUS bsp_shm_acquire(UINT8 * data)
{
int   fd;
#ifdef AS350PLUS_WIRELESS
    if(BSP_SHM_data==NULL)
    {
        fd=shm_open("/pwr-butt-shm", O_RDWR, 0777);
        if(fd<0)
        {
            // perror("shm open fail");
            return BSP_FAILURE;
        }
            
        BSP_SHM_data = (char*)mmap(NULL, 1, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0); 
        data=BSP_SHM_data;
        
        // printf("@BSP_SHM_data=%d , addr=%p\n",*BSP_SHM_data,BSP_SHM_data);
    }
    else
    {
        // printf("!BSP_SHM_data=%d , addr=%p\n",*BSP_SHM_data,BSP_SHM_data);
        data=BSP_SHM_data;
    }
        
    return BSP_SUCCESS;    
#endif    
#ifdef AS350PLUS_COUNTERTOP
    return BSP_FAILURE;
#endif
}

/*
**INPUT:data 0 or 1
**      wr=1 write data to bit0
**      wr=0 read data from bit0 
**OUTPUT:data 0 or 1
*/
void bsp_shm_IrqOccupied(UINT8 wr,UINT8 *data)
{
UINT8 bit;
    if(bsp_shm_acquire(BSP_SHM_data)==BSP_FAILURE)
        return;
    if(wr)
    {
        bit=*BSP_SHM_data;
        bit&=(~0x01);//erase bit0 original data
        bit|=(*data);//assign input data to bit0
        *BSP_SHM_data=bit;
    }
    else
        *data=(*BSP_SHM_data)&0x01;
}
/*
**INPUT:data 0 or 1
**OUTPUT:data 0 or 1
**wr=1 write data to bit1
**wr=0 read data from bit1
**bit1 =0 button are not pressed
**bit1 =1 button are pressed
*/
void bsp_shm_ButtPressed(UINT8 wr,UINT8 *data)
{
UINT8 bit;
    if(bsp_shm_acquire(BSP_SHM_data)==BSP_FAILURE)
        return;
    if(wr)
    {
        bit=*BSP_SHM_data;
        bit&=(~(0x01<<1));//erase bit1 original data
        bit|=((*data)<<1);//assign input data to bit1
        *BSP_SHM_data=bit;
    }
    else
        *data=((*BSP_SHM_data)&0x02)>>1;
}
/*
**INPUT:data 0 ~ 3
**OUTPUT:data 0 ~ 3
**wr=1 write data to bit2 bit3
**wr=0 read data from bit2 bit3
*/
void bsp_shm_SPIselect(UINT8 wr,UINT8 *data)
{
UINT8 bit;
    if(bsp_shm_acquire(BSP_SHM_data)==BSP_FAILURE)
        return;
    if(wr)
    {
        bit=*BSP_SHM_data;
        bit&=(~(0x03<<2));//erase bit2 3 original data
        bit|=((*data)<<2);//assign input data to bit2 3
        *BSP_SHM_data=bit;
    }
    else
        *data=((*BSP_SHM_data)&(0x03<<2))>>2;
}

/*
**INPUT:data 0 or 1
**OUTPUT:data 0 or 1
**wr=1 write data to bit4
**wr=0 read data from bit4
*/
void bsp_shm_Adapter(UINT8 wr,UINT8 *data)
{
UINT8 bit;
    if(bsp_shm_acquire(BSP_SHM_data)==BSP_FAILURE)
        return;
    if(wr)
    {
        bit=*BSP_SHM_data;
        bit&=(~0x10);//erase bit4 original data
        bit|=((*data)<<4);//assign input data to bit4
        *BSP_SHM_data=bit;
    }
    else
        *data=((*BSP_SHM_data)&0x10)>>4;
}

/*
**INPUT:data 0 or 1
**OUTPUT:data 0 or 1
**wr=1 write data to bit5
**wr=0 read data from bit5
*/
void bsp_shm_BatteryAlive(UINT8 wr,UINT8 *data)
{
UINT8 bit;
    if(bsp_shm_acquire(BSP_SHM_data)==BSP_FAILURE)
        return;
    if(wr)
    {
        bit=*BSP_SHM_data;
        bit&=(~0x20);//erase bit5 original data
        bit|=((*data)<<5);//assign input data to bit5
        *BSP_SHM_data=bit;
    }
    else
        *data=((*BSP_SHM_data)&0x20)>>5;
}

/*
**INPUT:data 0 or 1
**OUTPUT:data 0 or 1
**wr=1 write data to bit6
**wr=0 read data from bit6
*/
void bsp_shm_BatteryCharging(UINT8 wr,UINT8 *data)
{
UINT8 bit;
    if(bsp_shm_acquire(BSP_SHM_data)==BSP_FAILURE)
        return;
    if(wr)
    {
        bit=*BSP_SHM_data;
        bit&=(~0x40);//erase bit6 original data
        bit|=((*data)<<6);//assign input data to bit6
        *BSP_SHM_data=bit;
    }
    else
        *data=((*BSP_SHM_data)&0x40)>>6;
}