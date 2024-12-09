



#include "POSAPI.h"

#ifndef _DEV_PAGE_H_
#define _DEV_PAGE_H_

#define TRUE 1
#define FALSE 0 
#define LINK 0
#define FREE 1
#define READ 0
#define WRITE 1


/**
 *  @brief use in link pages to describe each page be link in which range
 *  @note initial as itself. 
 */ 
struct range{
    UINT low;      /*!<    linked start pages number  */
    UINT high;     /*!<    linked end pages number  */  
};

/**
 *  @brief use in perform page operation like link , read , write ,clear
 */ 
struct operation_info{
    UINT start_page;      /*!<   start page number */
    UINT end_page;        /*!<   end page number */
    ULONG start_address;  /*!<   start address number */
    ULONG end_address;    /*!<   end address number */
    ULONG length;         /*!<   read,write,clear length */
};


/**
 *  @brief main data structure of page system
 */ 
struct page_system_info{
    UINT page_num;                  /*<   total number of page */
    ULONG page_size;                /*<   size of per page */
    UCHAR* system_name;             /*<   page system name */
    struct range *link_table;       /*<   link table of each page  */
    struct operation_info op_info;  /*<   operation parameter to be perform */
};


extern ULONG page_system_init(struct page_system_info *info);
extern ULONG check_page_init(struct page_system_info *info, UINT page_number);
extern ULONG page_link_or_free(struct page_system_info *info, UCHAR action );
extern UCHAR page_read_or_write(struct page_system_info *info, UCHAR * pData, UCHAR flag);
extern ULONG page_clear( struct page_system_info *info, UCHAR pattern );
#ifdef PAGE_ACCESS_BY_ADDRESS
extern UCHAR page_read_or_write_by_address(struct page_system_info *info, UCHAR* address, 
                                ULONG length, UCHAR* data, UCHAR flag);
extern UCHAR page_clear_by_address(struct page_system_info *info,UCHAR* address, 
                                ULONG length, UCHAR pattern );
#endif


#endif


