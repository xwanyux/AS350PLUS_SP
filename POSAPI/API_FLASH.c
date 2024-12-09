



#include "POSAPI.h"
#include "FLASHAPI.h"
#include "DEV_PAGE.h"
#include "bsp_types.h"
#include "OS_FLASH.h"

#define NOR_FLASH_SECTOR_SIZE 128 * 1024 // this thing is depenand on hardware you simluate


// use as inhernt from "page class" (DEV_PAGE)
static struct page_system_info page_nand_flash;
static struct page_system_info page_nor_flash;
static int current_type_flash;

/**
 *     this function is used to update the  page_nand_flash variable 
 *     with the API_FLASH type
 *     @param[in] pFls      necessory info for flash
 */
static void update_page_nand_flash_op_info(API_FLASH* pFls){

    page_nand_flash.op_info.start_address = pFls->StAddr;
    page_nand_flash.op_info.end_address = pFls->EndAddr;
    page_nand_flash.op_info.start_page = pFls->StPage;
    page_nand_flash.op_info.end_page = pFls->EndPage;
    page_nand_flash.op_info.length = pFls->Len;
}

static void update_page_nor_flash_op_info(API_FLASH* pFls){

    page_nor_flash.op_info.start_address = pFls->StAddr;
    page_nor_flash.op_info.end_address = pFls->EndAddr;
    page_nor_flash.op_info.start_page = pFls->StPage;
    page_nor_flash.op_info.end_page = pFls->EndPage;
    page_nor_flash.op_info.length = pFls->Len;

} 


/**
 *  this function is used to create the flash file system
 *  @note first create the directory pages in /home/root/
 *        then create the file 0..num_page-1, each page 
 *        file initialize with size PAGE_SIZE with value 0
 *        if file already exist, this function will not change
 *        the existing file value (when the size of each page
 *        remain the same)
 */ 
ULONG api_flash_init(void){


    ULONG result;
    
    page_nand_flash.page_num = 1024; // 1024
    page_nand_flash.page_size = 128 * 1024; // 128K
    page_nand_flash.system_name = "nand_flash";

    result = page_system_init(&page_nand_flash);
    if(result != apiOK)
        return result;

    page_nor_flash.page_num = 1; // 1024
    page_nor_flash.page_size = 16 * 1024 * 1024; // 128K
    page_nor_flash.system_name = "nor_flash";

    result = page_system_init(&page_nor_flash);

    // set the default flash as nor
    current_type_flash = TYPE_NOR;
    return result;

}

/**
 *  this function is is not implemented.
 *  @param[in] Type         not use
 *  @return apiOK 
 */ 
ULONG	api_flash_TypeSelect( ULONG Type ){

    if( (Type != TYPE_NOR) && (Type != TYPE_NAND))
        return apiFailed;
    
    current_type_flash = Type;
    return apiOK;
}

/**
 *  this function to check the page initialzation success or not
 *  @param[in] Page     page number 
 *  @param[out] pAddr  the valid start address and end address of query page
 *  @return error code
 *  @retval apiOK       page initalize success
 *  @retval apiFailed   page initialize failed
 */
ULONG api_flash_PageSelect( UINT Page, API_FLASH_ADDR *pAddr ){

    int result = apiFailed;

    if(current_type_flash == TYPE_NAND)
        result = check_page_init(&page_nand_flash, Page);
    else if(current_type_flash == TYPE_NOR)
        result = check_page_init(&page_nor_flash, Page);

    if(result == apiFailed)
        return apiFailed;
    
    pAddr->Type = current_type_flash;
    pAddr->StAddr = 0;

    if(current_type_flash == TYPE_NAND)
        pAddr->EndAddr = page_nand_flash.page_size - 1;
    else if(current_type_flash == TYPE_NOR)
        pAddr->EndAddr = page_nor_flash.page_size - 1;

    return apiOK;
}

/**
 *  this function is used to link page or free page
 *  @param[in] pFls        provide information of start page and end page
 *  @param[in] Action       0/1   link/free
 *  @return error code
 *  @retval apiFailed       link/free failed
 *  @retval apiOK           link/free success
 *  @note when doing linking, range from start page and end page should not be link before
 *        when doing freem the start page and end page should be the same link sturcutre
 */
ULONG api_flash_PageLink( API_FLASH pFls, UCHAR Action ){


    if(current_type_flash == TYPE_NAND){

        update_page_nand_flash_op_info(&pFls);

        if(Action == 0)
            return page_link_or_free(&page_nand_flash, LINK);
        else if(Action == 1)
            return page_link_or_free(&page_nand_flash, FREE);
    }
    else if(current_type_flash == TYPE_NOR){

        update_page_nor_flash_op_info(&pFls);

        if(Action == 0)
            return page_link_or_free(&page_nor_flash, LINK);
        else if(Action == 1)
            return page_link_or_free(&page_nor_flash, FREE);

    }

    return apiFailed;

}

/**
 *  this function is used to read data from page
 *  @param[in] pFls        necessary information for flash
 *  @param[out] pData       read data from page
 *  @return error code
 *  @retval apiOK           read success
 *  @retval apiFailed       read failed
 *  @note the pFls lengh should be in the range of link page
 *        if length is bigger than the link page , will return apiFailed
 */ 
ULONG api_flash_PageRead( API_FLASH pFls, UCHAR *pData ){
    if(current_type_flash == TYPE_NAND){

        update_page_nand_flash_op_info(&pFls);

        return page_read_or_write(&page_nand_flash, pData, READ);
    }
    else if(current_type_flash == TYPE_NOR){

        update_page_nor_flash_op_info(&pFls);

        return page_read_or_write(&page_nor_flash, pData, READ);

    }
}

/**
 *  this function is used to write data to page
 *  @param[in] pFls         necessary information for flash
 *  @param[in] pData        data be written to page
 *  @return error code
 *  @retval apiOK           write success
 *  @retval apiFailed       write failed
 *  @note the pFls lengh should be in the range of link page
 *        if length is bigger than the link page , will return apiFailed
 */ 
ULONG api_flash_PageWrite( API_FLASH pFls, UCHAR *pData ){

    if(current_type_flash == TYPE_NAND){

        update_page_nand_flash_op_info(&pFls);

        return page_read_or_write(&page_nand_flash, pData, WRITE);
    }
    else if(current_type_flash == TYPE_NOR){

        update_page_nor_flash_op_info(&pFls);

        return page_read_or_write(&page_nor_flash, pData, WRITE);

    }
}

/**
 *  this function is used to clear the page with some value
 *  @param[in] pFls        necessary information for flash
 *  @param[in] Pattern      the clear value
 *  @return error code
 *  @retval apiOK           clear success
 *  @retval apiFailed       clear failed
 *  @note pFls.Len is 0, the clear function will clear all the page be link before
 *        pFls.Len is not 0, will only clean the length.
 *        if clean lengh is bigger than the link page, api will reutrn apiFailed
 */ 
ULONG api_flash_PageClear( API_FLASH pFls, UCHAR Pattern ){

    if(current_type_flash == TYPE_NAND){

        update_page_nand_flash_op_info(&pFls);

        return page_clear(&page_nand_flash, Pattern);
    }
    else if(current_type_flash == TYPE_NOR){

        update_page_nor_flash_op_info(&pFls);

        return page_clear(&page_nor_flash, Pattern);

    }
}
