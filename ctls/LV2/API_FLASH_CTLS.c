





#include "DEV_MEM.h"
#include "FLSAPI.h"



static struct dev_mem_info ctls_capk;
static struct dev_mem_info ctls_para;
static struct dev_mem_info ctls_imek;
static struct dev_mem_info ctls_iaek;


UCHAR FLASH_INIT_FLAG=0;
/**
 *  this function is used to initialize the ctls region of memory
 */ 

void flash_ctls_init(){


    ctls_capk.size = 25 * 1024; // 25 k
    ctls_capk.system_name = "ctls_capk";
    ctls_capk.base_address = 0;

    mem_init(&ctls_capk);

    ctls_para.size = 10 * 1024; // 10k
    ctls_para.system_name = "ctls_para";
    ctls_para.base_address = 0;

    mem_init(&ctls_para);

    ctls_imek.size = 16; // 16 bytes
    ctls_imek.system_name = "ctls_imek";
    ctls_imek.base_address = 0;

    mem_init(&ctls_imek);

    ctls_iaek.size = 16; // 16 bytes
    ctls_iaek.system_name = "ctls_iaek";
    ctls_iaek.base_address = 0;

    mem_init(&ctls_iaek);
    FLASH_INIT_FLAG=1;
}

/**
 *  this function is used to read value from flash by different id region
 *  @param[in] id                   the region 
 *                                  @li FLSID_CAPK 25k bytes
 *                                  @li FLSID_PARA 10k bytes
 *                                  @li FLSID_IMEK 16  bytes
 *                                  @li FLSID_IAEK 16  bytes
 * @param[in] add                   the address you want to access 
 *                                  (each region start from 0)
 * @param[in] len                   number of data
 * @param[out] buf                  the data read from flash
 * @return error code
 * @retval apiOK                    read success
 * @retval apiFailed                read failed
 */ 
ULONG api_fls_read( UCHAR id, ULONG addr, ULONG len, UCHAR *buf ){

    if(!FLASH_INIT_FLAG)
        flash_ctls_init();
    switch (id)
    {
    case FLSID_CAPK:
        return mem_read_or_write(&ctls_capk, addr, len, buf, READ);
        break;
    case FLSID_PARA:
        return mem_read_or_write(&ctls_para, addr, len, buf, READ);
        break;
    case FLSID_IMEK:
        return mem_read_or_write(&ctls_imek, addr, len, buf, READ);
        break;
    case FLSID_IAEK:
        return mem_read_or_write(&ctls_iaek, addr, len, buf, READ);
        break;
    default:
        return apiFailed;
    }

}


/**
 *  this function is used to write value to flash by different id region
 *  @param[in] id                   the region 
 *                                  @li FLSID_CAPK 25k bytes
 *                                  @li FLSID_PARA 10k bytes
 *                                  @li FLSID_IMEK 16  bytes
 *                                  @li FLSID_IAEK 16  bytes
 * @param[in] add                   the address you want to access 
 *                                  (each region start from 0)
 * @param[in] len                   number of data
 * @param[in] buf                   the data be written to flash
 * @return error code
 * @retval apiOK                    write success
 * @retval apiFailed                write failed
 */ 
ULONG api_fls_write( UCHAR id, ULONG addr, ULONG len, UCHAR *buf ){

    if(!FLASH_INIT_FLAG)
        flash_ctls_init();
    switch (id)
    {
    case FLSID_CAPK:
        return mem_read_or_write(&ctls_capk, addr, len, buf, WRITE);
        break;
    case FLSID_PARA:
        return mem_read_or_write(&ctls_para, addr, len, buf, WRITE);
        break;
    case FLSID_IMEK:
        return mem_read_or_write(&ctls_imek, addr, len, buf, WRITE);
        break;
    case FLSID_IAEK:
        return mem_read_or_write(&ctls_iaek, addr, len, buf, WRITE);
        break;
    default:
        return apiFailed;
    }

}




