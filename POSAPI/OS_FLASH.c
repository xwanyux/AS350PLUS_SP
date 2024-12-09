





#include "POSAPI.h"
#include "OS_FLASH.h"
#include "DEV_MEM.h"


#define FLASG_PAGE_SECTOR_SIZE 128 * 1024

static struct dev_mem_info system_back_up;


/**
 *  this function is used to initialize the os flash
 */ 
void os_flash_init(){

    system_back_up.size = 128 * 1024; // 128 * 1024;
    system_back_up.system_name = "flash_system_back_up";
    system_back_up.base_address = FLASH_BACKUP_BASE_ADDR;

    mem_init(&system_back_up);

}

/**
 *    this function is used to write data to flash by address
 *    @param[in] BaseAddr           the start address to be write
 *    @param[in] pData              the data to be write
 *    @param[in] Len                the length of data
 *    @return error code
 *    @retval apiOK                 write success
 *    @retval apiFailed             write failed
 */ 
UINT32	FLASH_WriteData( void *BaseAddr, void *pData, UINT32 Len ){

    return mem_read_or_write(&system_back_up, (ULONG)BaseAddr, Len, (UCHAR*)pData, WRITE);

}

/**
 *    this function is used to read data to flash by address
 *    @param[in] BaseAddr           the start address to be read
 *    @param[out] pData             the data to be read
 *    @param[in] Len                the length of data want to read
 */ 
void FLASH_ReadData( void *BaseAddr, void *pData, UINT32 Len ){

    mem_read_or_write(&system_back_up, (ULONG)BaseAddr, Len, (UCHAR*)pData, READ);

}

/**
 *    this function is used to write data to flash by address
 *    @param[in] addr               the start address to be write
 *    @param[in] len                the length of data
 *    @param[in] data               the data to be write
 *    @return error code
 *    @retval apiOK                 write success
 *    @retval apiFailed             write failed
 */ 
UINT32 OS_FLS_PutData( UINT32 addr, UINT32 len, UINT8 *data ){

    return mem_read_or_write(&system_back_up, addr, len, data, WRITE);
}

/**
 *    this function is used to write data to flash by address
 *    @param[in] addr               the start address to be read
 *    @param[in] len                the length of data
 *    @param[out] data              the data to be read
 */ 
void OS_FLS_GetData( UINT32 addr, UINT32 len, UINT8 *data ){

    mem_read_or_write(&system_back_up, addr , len, data, READ);

}

/**
 *    this function is used to clear the memory to 0 
 *     from Sectorbase of size FLASG_PAGE_SECTOR_SIZE
 *    @param[in] SectorBase           the start address to be clear
 *    @return error code
 *    @retval apiOK                    clear success
 *    @retval apiFailed                clear failed
 */ 
UINT32 FLASH_EraseSector( UINT32 SectorBase ) {

    return mem_clear(&system_back_up, SectorBase, FLASG_PAGE_SECTOR_SIZE, 0);

}