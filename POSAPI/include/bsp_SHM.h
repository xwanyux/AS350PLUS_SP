#ifndef _BSP_SHM_H_
#define _BSP_SHM_H_
#include "bsp_types.h"


//function
BSP_STATUS bsp_shm_acquire(UINT8 * data);
void bsp_shm_IrqOccupied(UINT8 wr,UINT8 *data);
void bsp_shm_ButtPressed(UINT8 wr,UINT8 *data);
void bsp_shm_SPIselect(UINT8 wr,UINT8 *data);
void bsp_shm_Adapter(UINT8 wr,UINT8 *data);
void bsp_shm_BatteryAlive(UINT8 wr,UINT8 *data);
void bsp_shm_BatteryCharging(UINT8 wr,UINT8 *data);
#endif