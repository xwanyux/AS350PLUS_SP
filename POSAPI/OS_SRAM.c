




#include "DEV_MEM.h"


static struct dev_mem_info emvk_info;
static UCHAR mem_init_flag=0;
static void Have_mem_inited()
{
    if(!mem_init_flag)
        os_sram_init();
    mem_init_flag=1;
}
/**
 *  this function is used to initialize the os sram
 */ 
void os_sram_init(){


    emvk_info.size = 10 * 1024 * 32; // 32k * 4
    emvk_info.system_name = "sram_emvk";
    emvk_info.base_address = 65536;

    mem_init(&emvk_info);
}

/**
 *  this function is used to write sram by address
 *  @param[in] address          start address
 *  @param[in] length           length of data
 *  @param[in] data             data you want to write
 *  @return error code
 *  @retval apiFailed           write failed
 *  @retval apiOK               write OK
 */ 
UCHAR api_sram_write(UCHAR* address, ULONG length, UCHAR* data ){
	// printf("address=%p\n",address);
    Have_mem_inited();
    return mem_read_or_write(&emvk_info, (ULONG)address, length, data, WRITE);
   
}

/**
 *  this function is used to write sram by address
 *  @param[in] address          start address
 *  @param[in] length           length of data
 *  @param[out] data            data read form sram
 *  @return error code
 *  @retval apiFailed           read failed
 *  @retval apiOK               read OK
 */ 
UCHAR api_sram_read(UCHAR* address, ULONG length, UCHAR* data){
    // printf("address=%p\n",address);
    Have_mem_inited();
    return mem_read_or_write(&emvk_info, (ULONG)address, length, data, READ);
}

/**
 *  this function is used to clear sram by address
 *  @param[in] address          start address
 *  @param[in] length           length you want to clear
 *  @param[in] pattern          value you want to set
 *  @return error code
 *  @retval apiFailed           clear failed
 *  @retval apiOK               clear OK
 */
UCHAR api_sram_clear(UCHAR* address, ULONG length, UCHAR pattern ){
    Have_mem_inited();
    return mem_clear(&emvk_info, (ULONG)address, length, pattern);
}



