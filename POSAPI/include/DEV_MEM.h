




#include "POSAPI.h"

#ifndef _DEV_MEM_H_
#define _DEV_PAGE_H_


#define TRUE 1
#define FALSE 0 
#define READ 0
#define WRITE 1


/**
 *  @brief main data structure of memory allocation
 */ 
struct dev_mem_info{

    ULONG size;                     /*!<   size of memory region */
    UCHAR* system_name;             /*!<   name of memory system in file system */
    ULONG base_address;             /*!<   simulate base address of memory */
};


extern UCHAR mem_init(struct dev_mem_info *info);
extern UCHAR mem_read_or_write(struct dev_mem_info *info, ULONG mem_add, ULONG length, UCHAR *data, UCHAR flag);
extern UCHAR mem_clear(struct dev_mem_info *info, ULONG mem_add, ULONG length, UCHAR pattern);



#endif