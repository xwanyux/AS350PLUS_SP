
#include "POSAPI.h"
#include <stdio.h>
#include <string.h>
#include "DEV_PAGE.h"


// use as inhernt from "page class" (DEV_PAGE)
static struct page_system_info page_sram; 


/**
 *     this function is used to update the page_flash variable 
 *     with the API_FLASH type
 *     @param[in] pSram      necessory info for sram
 */
void update_page_sram_op_info(API_SRAM *pSram){
    page_sram.op_info.start_address = pSram->StAddr;
    page_sram.op_info.end_address = pSram->EndAddr;
    page_sram.op_info.start_page = pSram->StPage;
    page_sram.op_info.end_page = pSram->EndPage;
    page_sram.op_info.length = pSram->Len;
}


/**
 *  this function is used to create the sram file system
 *  @note first create the directory pages in /home/root/
 *        then create the file 0..num_page-1, each page 
 *        file initialize with size PAGE_SIZE with value 0
 *        if file already exist, this function will not change
 *        the existing file value (when the size of each page
 *        remain the same)
 */ 
ULONG api_sram_PageInit( void ){

    page_sram.page_num = 59; // 58
    page_sram.page_size = 32 * 1024; // 32k
    page_sram.system_name = "sram";

    return page_system_init(&page_sram);

}


/**
 *  this function to check the page initialzation success or not
 *  @param[in] Page     page number 
 *  @param[out] pSram   the valid start address and end address of query page
 *  @return error code
 *  @retval apiOK       page initalize success
 *  @retval apiFailed   page initialize failed
 */
ULONG api_sram_PageSelect( UCHAR Page, API_SRAM_ADDR *pSram ){

    int result;

    result = check_page_init(&page_sram, Page);

    if(result == apiFailed)
        return apiFailed;

    pSram->StAddr = 0;
    // pSram->StAddr = page_sram.page_size - 1;
    pSram->EndAddr = page_sram.page_size - 1;//20210827 fixed by West

    return apiOK;

}


/**
 *  this function is used to link page or free page
 *  @param[in] pSram        provide information of start page and end page
 *  @param[in] Action       0/1   link/free
 *  @return error code
 *  @retval apiFailed       link/free failed
 *  @retval apiOK           link/free success
 *  @note when doing linking, range from start page and end page should not be link before
 *        when doing freem the start page and end page should be the same link sturcutre
 */
ULONG api_sram_PageLink( API_SRAM pSram, UCHAR Action ){

    update_page_sram_op_info(&pSram);

    if(Action == 0)
        return page_link_or_free(&page_sram, LINK);
    else if(Action == 1)
        return page_link_or_free(&page_sram, FREE);

    return apiFailed;
}


                
/**
 *  this function is used to read data from page
 *  @param[in] pSram        necessary information for Sram
 *  @param[out] pData       read data from page
 *  @return error code
 *  @retval apiOK           read success
 *  @retval apiFailed       read failed
 *  @note the pSram lengh should be in the range of link page
 *        if length is bigger than the link page , will return apiFailed
 */ 
extern UCHAR gl_mode;
ULONG api_sram_PageRead( API_SRAM pSram, UCHAR *pData ){
ULONG result;
    update_page_sram_op_info(&pSram);

    result=page_read_or_write(&page_sram, pData, READ);
    return result;
}

/**
 *  this function is used to write data to page
 *  @param[in] pSram        necessary information for Sram
 *  @param[in] pData        data be written to page
 *  @return error code
 *  @retval apiOK           write success
 *  @retval apiFailed       write failed
 *  @note the pSram lengh should be in the range of link page
 *        if length is bigger than the link page , will return apiFailed
 */

ULONG api_sram_PageWrite( API_SRAM pSram, UCHAR *pData ){
    update_page_sram_op_info(&pSram);
    return page_read_or_write(&page_sram, pData, WRITE);
}

/**
 *  this function is used to clear the page with some value
 *  @param[in] pSram        necessary information for sram
 *  @param[in] Pattern      the clear value
 *  @return error code
 *  @retval apiOK           clear success
 *  @retval apiFailed       clear failed
 *  @note pSram.Len is 0, the clear function will clear all the page be link before
 *        pSram.Len is not 0, will only clean the length.
 *        if clean lengh is bigger than the link page, api will reutrn apiFailed
 */ 
ULONG api_sram_PageClear( API_SRAM pSram, UCHAR Pattern ){

    update_page_sram_op_info(&pSram);

    return page_clear(&page_sram, Pattern);
}


