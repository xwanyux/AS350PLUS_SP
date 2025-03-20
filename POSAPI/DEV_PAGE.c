





/**
 *  This module use to implement the virtual page system using linux file system
 *  create in /home/root/name
 *  each page will be file locate in /home/root/name/number
 *  page 1 will be in /home/root/name/0
 */ 


#include "POSAPI.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "DEV_PAGE.h"
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#define WRTIE_BUFFER_SIZE 1024 // this is use for optimize (1000)
#define G_BUFFER_SIZE 70

/**
 *  @brief this buffer is use for store string of get_system_path,
 *      get_remove_path_cmmd,get_check_directory_exist_cmmd
 */ 
static UCHAR g_buffer[G_BUFFER_SIZE] = {0};


/**
 *  this function is to construct the page path from page_system_info type
 *  @param[in] info       necessary information of page system
 *  @note path should be "/home/root/name"
 */ 
static void get_system_path(struct page_system_info *info){
    UCHAR system_path[] = "/home/root/";
    memset(g_buffer,0,G_BUFFER_SIZE);
    sprintf(g_buffer,"%s%s/",system_path,info->system_name);
}
/**
 *  this function is to construct the remove directory command from 
 *  page_system_info type
 *  @param[in] info    necessray information of page system
 *  @note command should be "rm -r /home/root/name";
 */ 
static void get_remove_path_cmmd(struct page_system_info *info){
    UCHAR remove_path_cmmd[] = "rm -r /home/root/";
    memset(g_buffer,0,G_BUFFER_SIZE);
    sprintf(g_buffer,"%s%s",remove_path_cmmd,info->system_name);
}

/**
 *  this function is to construct the check directory exist command from 
 *  page_system_info type
 *  @param[in] info    necessray information of page system
 *  @note command should be ""ls /home/root/ | grep name";
 */ 
static void get_check_directory_exist_cmmd(struct page_system_info* info){
    UCHAR check_dir_exist_cmmd[] = "/home/root/";
    memset(g_buffer,0,G_BUFFER_SIZE);
    sprintf(g_buffer,"%s%s",check_dir_exist_cmmd,info->system_name);
}

/**
 *  this function is to convert number to string (decimal)
 *  @param[in] number      number be conerted
 *  @param[out] buffer     converted string
 *  @note you should make sure the buffer size have enough size
 *        for 0 to appened in the end
 */ 
static void number_to_string(UINT number, UCHAR* buffer){

    UINT divide = 1;
    UCHAR cursor = 0;

    while(divide * 10 <= number)
        divide *= 10;

    while(divide != 1){
        buffer[cursor++] = number / divide + '0';
        number = number % divide;
        divide /= 10;
    }

    buffer[cursor++] = number + '0';
    buffer[cursor] = 0;
}

/**
 *  this function is to map page number to the corrosponding path
 *  @param[in] info           necessary information of page system
 *  @param[in] page_number    page number
 *  @param[out] page_path     mapping page number path
 */
static void page_number_to_path(struct page_system_info *info,UINT page_number, UCHAR* page_path){

    UCHAR number[5];
    number_to_string(page_number, number);
    get_system_path(info);
    sprintf(page_path, "%s%s",g_buffer,number);

}


/**
 *  this function is used to create the page file system
 *  @param[in] info         necessary information of page system
 *  @return error code
 *  @retval apiOK           create success
 *  @retval apiFailed       create failed
 *  @note first create the directory pages in /home/root/name
 *        then create the file 0..num_page-1, each page 
 *        file initialize with size PAGE_SIZE with value 0
 *        if file already exist, this function will not change
 *        the existing file value (when the size of each page
 *        remain the same)
 */ 
ULONG page_system_init(struct page_system_info *info){

FILE *cmmd_file, *linux_file;
UCHAR cmmd_result[10] = {0}; 
UCHAR path_or_cmmd_buffer[70] = {0}; // use for construct path or command
UCHAR write_buffer[WRTIE_BUFFER_SIZE] = {0}; // use for write (initialization)
UCHAR chmod_str[] = "chmod 777";
UCHAR mkdir_str[] = "mkdir";
UCHAR number[5];
int i,j, write_result;
struct stat s;

    info->link_table = malloc(sizeof(struct range) * info->page_num);

    // #ifdef DEBUG
        // set the page vale to '0' 
        memset(write_buffer, '0', WRTIE_BUFFER_SIZE);
    // #endif

    get_check_directory_exist_cmmd(info);
    write_result = stat(g_buffer, &s);
    if(-1 == write_result) 
    {
        if(ENOENT == errno) 
        {
            printf("dierctory not exist\n");
            get_system_path(info);
            sprintf(path_or_cmmd_buffer, "%s %s",mkdir_str,g_buffer);
            cmmd_file = popen(path_or_cmmd_buffer, "r");
            pclose(cmmd_file);
        } 
        else 
        {
            perror("stat");
        }
    } 
    else 
    {
        if(S_ISDIR(s.st_mode))
        {
            /* it's a dir */
        } 
        else
        {
            printf("same name file exist but dierctory not exist\n");
            get_system_path(info);
            sprintf(path_or_cmmd_buffer, "%s %s",mkdir_str,g_buffer);
            cmmd_file = popen(path_or_cmmd_buffer, "r");
            pclose(cmmd_file);
        }
    }
    printf("info->page_num=%d\n",info->page_num);
    for(i = 0; i < info->page_num; i++){
        
        number_to_string(i,number);
        memset(path_or_cmmd_buffer,0 , 70);
        get_system_path(info);
        sprintf(path_or_cmmd_buffer, "%s%s",g_buffer,number);

        // printf("path%s\n",path_or_cmmd_buffer);
        //fflush(stdout);
        // if aleady initialize, not overwrite
        if(check_page_init(info, i) != apiOK){

            fflush(stdout);

            linux_file = fopen(path_or_cmmd_buffer, "w");
            // perror("=====sssssssssssssss====open fail?");
            for(j = 0; j < info->page_size/WRTIE_BUFFER_SIZE; j++){
                write_result = fwrite(write_buffer,1,WRTIE_BUFFER_SIZE,linux_file);
                if(write_result < 0){

                    fclose(linux_file);
                    free(info->link_table);
                    return apiFailed;
                }
            }
            // add for WRTIE_BUFFER_SIZE can't be divide by page_size
            if((info->page_size % WRTIE_BUFFER_SIZE) != 0)
                write_result = fwrite(write_buffer,1,info->page_size % WRTIE_BUFFER_SIZE,linux_file);

            fclose(linux_file);

            // running chmod command to the file (make file can be read and write)
            memset(path_or_cmmd_buffer,0 , 70);
            get_system_path(info);
            sprintf(path_or_cmmd_buffer, "%s %s%s",chmod_str,g_buffer,number);

            cmmd_file = popen(path_or_cmmd_buffer, "r");

            pclose(cmmd_file);
        }

        // update link
        info->link_table[i].high = i;
        info->link_table[i].low = i;

    }


    sync();

    return apiOK;

}

/**
 *  this function is to check page initalize success
 *  @param[in] info         necessary information of page system
 *  @return error code
 *  @retval apiOK           initial success
 *  @retval apiFailed       initial failed
 */     
ULONG check_page_init(struct page_system_info *info, UINT page_number){

    UCHAR path_buffer[70] = {0};
    FILE* fd;

    page_number_to_path(info,page_number,path_buffer);


    fd = fopen(path_buffer, "r");
    // perror("======!!!!!!!=======open fail?");
    if(fd == NULL){
        return apiFailed;
    }
    //comment out by west for prevent sram data flushed everytime when AP open
    // fseek(fd, 0L, SEEK_END);
    // if(ftell(fd) != info->page_size){

    //     fclose(fd);
    //     return apiFailed;
    // }

    fclose(fd);
    return apiOK;
}



/**
 *  this function is use to link or free the pages
 *  @param[in] info        necessary information of page system
 *  @param[in] action      macro
 *                         @li LINK
 *                         @li FREE
 *  @return error code
 *  @retval apiFailed       link/free failed
 *  @retval apiOK           link/free success
 *  @note when doing linking, range from start page and end page should not be link before
 *        when doing freem the start page and end page should be the same link sturcutre
 */  
ULONG page_link_or_free(struct page_system_info *info, UCHAR action ){


    struct operation_info op_info = info->op_info;
    ULONG page_maximum_number = info->page_num - 1;
    UINT start_page = op_info.start_page;
    UINT end_page = op_info.end_page;
    UINT low_st, high_st; // start page
    UINT low_end, high_end; // end page
    int i;

    if(start_page > end_page || start_page > page_maximum_number ||
        end_page > page_maximum_number )
        return apiFailed;
    // link
    if(action == LINK){

        // check all the page all not link;
        for(i = start_page; i <= end_page; i++)
            if(info->link_table[i].low != info->link_table[i].high)
                return apiFailed;
    
        // make sure all not link
        for(i = start_page; i< end_page; i++){
            info->link_table[i].low = start_page;
            info->link_table[i].high = end_page;
        }        
    }
    else if (action == FREE){
        // check input start page and end page are the same trunk
        low_st = info->link_table[start_page].low;
        high_st = info->link_table[start_page].high;
        low_end = info->link_table[end_page].low;
        high_end = info->link_table[end_page].high;

        // not the same chuck
        if( (low_st != low_end) || (high_st != high_end) )
            return apiFailed;

        for(i = low_st; i < high_st; i++){
            info->link_table[i].low = i;
            info->link_table[i].high = i;
        }

    }
    return apiOK;
}


/**
 *  this function is used to write or read to page
 *  @param[in] info         necessary information of page system
 *  @param[in/out] pData    flag = WRITE, as input data
 *                          flag = READ, as output data
 *  @param[in] flag         macro
 *                          @li WRITE
 *                          @li READ
 *  @return error code
 *  @retval apiOK           read/write page success
 *  @retval apiFailed       read/write page failed
 */ 
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
UCHAR sram_debug=0;
extern int errno;
UCHAR page_read_or_write(struct page_system_info *info, UCHAR * pData, UCHAR flag){
    struct operation_info op_info = info->op_info;
    UCHAR path_buffer[70] = {0};
    FILE* linux_file;
    ULONG data_left_len = op_info.length;
    UINT page_id = op_info.start_page;
    ULONG page_offset = op_info.start_address;
    ULONG page_size_left = 0;
    UCHAR* input_data = pData;
    UCHAR result;
    struct stat   debug_buffer;
    UINT high_page_bound = info->link_table[page_id].high;
    // if(sram_debug&&(page_id==5))
    //     printf("page_id=%d flag=%d page_offset=%d high_page_bound=%d\n",page_id,flag,page_offset,high_page_bound);
    if(sram_debug)
        printf("page_id=%d\n",page_id);
    // if(page_id==5)
    //     sram_debug=1;
     while(data_left_len > 0){
        
        // check valid page range
        if(page_id > high_page_bound)
            return apiFailed;
        memset(path_buffer,0,70);
        page_number_to_path(info,page_id,path_buffer);
        // if(sram_debug)
        // {
        //     printf("path_buffer=");
        //     for(int g=0;g<30;g++)
        //         printf("%c",path_buffer[g]);
        //     printf("\n");
        // }
        linux_file = fopen(path_buffer, "r+");
        // if(sram_debug)
        //     printf("page_id=%d\tpage_offset=%d\n",page_id,page_offset);
        // printf("page_id=%d\n",page_id);
        // perror("=====rrrrrrrrrrrrrr=====open fail?");
        // if(page_id==0)
        // {
        //     printf("==============================================\n");
        //     perror("open fail?");
        // }
        // if(sram_debug)
        // {
        //     if(stat(path_buffer, &debug_buffer)!=0)
        //         printf("file not exist\n");
        //     else
        //         printf("file exist\n");
        //     // printf("strlen(path_buffer)= %d\n",strlen(path_buffer));
        //     perror("open fail");
        // }
        if(linux_file ==NULL)//modify by west.fopen error return NULL
        {
            perror("open fail\n");
            return apiFailed;
        }
            
        
        fseek(linux_file, page_offset, SEEK_SET);
        page_size_left = info->page_size - page_offset;
        // if(sram_debug&&(flag==1))
        //     printf("input_data=0x%x  data_left_len=%d  page_size_left=%d\n",*input_data,data_left_len,page_size_left);
        if(page_size_left>= data_left_len){
            if(flag == WRITE)
                result = fwrite(input_data, 1,data_left_len, linux_file);
            else if(flag == READ)
                result = fread(input_data, 1,data_left_len, linux_file);
            
            //modify by west.Prevent from reading two of 0x30 as data length
            if((data_left_len>=2)&&(!flag))
            {
                // printf("input_data[0]=%d\tinput_data[1]=%d\n",input_data[0],input_data[1]);
                if((input_data[0]==0x30)&&(input_data[1]==0x30))
                {
                    input_data[0]=0;
                    input_data[1]=0;
                }
            }
            data_left_len = 0 ;
        }
        else{
            if(flag == WRITE)
                result = fwrite(input_data, 1,page_size_left, linux_file);
            else if(flag == READ)
                result = fread(input_data, 1,page_size_left, linux_file);
            data_left_len -= page_size_left;
            input_data += page_size_left;
        }
        // if(flag == WRITE)
            // printf("path_buffer:%s\n",path_buffer);
        if(ferror(linux_file))
            printf("read/write error page_id=%d\tpage_offset=%d\n",page_id,page_offset);
        // if(sram_debug)
        // {
        //     printf("page_id=%d\tpage_offset=%d\n",page_id,page_offset);
        //     perror("write fail");
        // }
        fclose(linux_file);
        // if(sram_debug)
        //     perror("close fail");
        if(result < 0)
            return apiFailed;
        
    }

    if(flag == WRITE)
        sync();
    return apiOK;

}

/**
 *  this function is used to clear the page with some value
 *  @param[in] info         necessary information of page system
 *  @param[in] Pattern      the clear value
 *  @return error code
 *  @retval apiOK           clear success
 *  @retval apiFailed       clear failed
 *  @note info->op_info.len is 0, the clear function will clear all the page be link before
 *        info->op_info.len is not 0, will only clean the length.
 *        if clean lengh is bigger than the link page, api will reutrn apiFailed
 */ 
ULONG page_clear( struct page_system_info *info, UCHAR pattern ){

    struct operation_info op_info = info->op_info;
    ULONG len = op_info.length;
    UCHAR write_buffer[WRTIE_BUFFER_SIZE] = {0};
    int i,j;
    UCHAR path_buffer[70] = {0};
    FILE* fd;
    UCHAR result_write;
    ULONG size_left,temp_len;
    UINT high_page_bound = info->link_table[op_info.start_page].high;
    UINT low_page_bound = info->link_table[op_info.start_page].low;

    memset(write_buffer,pattern, WRTIE_BUFFER_SIZE);
    if(len == 0){
        for(i = low_page_bound ; i <= high_page_bound; i++){
            memset(path_buffer,0,70);
            //page_path_procoess(i, path_buffer);
            page_number_to_path(info,i,path_buffer);
            fd = fopen(path_buffer, "r+");
            perror("========gggggggggg====open fail?");
            for(j = 0; j < info->page_size/WRTIE_BUFFER_SIZE; j++){
                result_write = fwrite(write_buffer,1,WRTIE_BUFFER_SIZE,fd);
                if(result_write < 0){

                    fclose(fd);
                    return apiFailed;
                }
            }

            fclose(fd);
        }
    }
    else{
        // in order to make efficient, we create a buffer for len 1000
        while(len > 0){

            if(len < WRTIE_BUFFER_SIZE){
                // need to update the infomation 
                info->op_info.length = len;
                info->op_info.start_page = op_info.start_page;
                info->op_info.start_address = op_info.start_address;
                page_read_or_write(info,write_buffer,WRITE);
                len = 0;
            }
            else{

                if(op_info.start_page > high_page_bound)
                    return apiFailed;
                // avoid to reopen file many time
                memset(path_buffer,0,70);
                page_number_to_path(info,op_info.start_page,path_buffer);
                fd = fopen(path_buffer, "r+");
                perror("======aaaaaaaaaa====open fail?");
                if(fd < 0)
                    return apiFailed;
                fseek(fd, op_info.start_address, SEEK_SET);
                size_left = info->page_size - op_info.start_address;

                // compute the number of data you need to write in this page
                if(size_left >= len)
                    temp_len = len;
                else{
                    temp_len = size_left;
                }

                // this part is for optimiaztion
                // write a buffer with size 1000 
                // I think is faster then write size 1 with 1000 time
                for(j = 0; j < temp_len/WRTIE_BUFFER_SIZE; j++){
                    result_write = fwrite(write_buffer,1,WRTIE_BUFFER_SIZE,fd);
                    if(result_write < 0){

                        fclose(fd);
                        return apiFailed;
                    }
                }
                // the last run
                if((temp_len % WRTIE_BUFFER_SIZE) != 0)
                    result_write = fwrite(write_buffer,1,temp_len % WRTIE_BUFFER_SIZE,fd);

                fclose(fd);


                // update for next page
                len -= temp_len;
                op_info.start_page += 1;
                op_info.start_address = 0;

            }

            if(result_write < 0)
                return apiFailed;

        }

    }

    //sync();
    return apiOK;
}


#ifdef PAGE_ACCESS_BY_ADDRESS
/**
 *  this function is used to covert address to the crossponding page information.
 *  @param[in] info                 necressary information for page system
 *  @param[in] address              start address you want to access
 *  @param[in] length               size of page you want to access
 *  @param[out] back_up_length      the backup link table length
 *  @param[out] convert_start_page  the address converted to start page number
 *  @return backup link table pointer if convert success
 *          NULL convert failed
 */   
static struct range* convert_address_to_page(struct page_system_info *info, UCHAR* address, ULONG length, 
                        UINT *back_up_length,  UINT* convert_start_page){
    
    ULONG page_size = info->page_size;
    ULONG page_maximum_number = info->page_num - 1;
    ULONG address_value = (ULONG) address;
    UINT start_page_idx = (UINT)(address_value / page_size);
    UINT start_page_addr = (UINT)(address_value % page_size);
    UINT end_page_idx, temp_length, link_table_size;
    struct range *backup_link_table;
    int i, result;

    #ifdef DEBUG
        printf("---page convert address debug----\n");
        printf("address:%x\n",address);
        printf("page number:%lu\n",start_page_addr);
        printf("page start idx:%lu\n",start_page_idx );
        printf("---end page debug\n");
    #endif

    // calculate the end of page_idx
    if(length <= (page_size - start_page_addr))
        end_page_idx = start_page_idx;
    else{
        temp_length = length - (page_size - start_page_addr);
        if( (temp_length % page_size) == 0)
            end_page_idx = start_page_idx + temp_length / page_size;
        else
            end_page_idx = start_page_idx + 1 + temp_length / page_size;
    }
    // invalid read or write
    if( (start_page_idx > page_maximum_number) || (end_page_idx > page_maximum_number))
        return NULL;
    
    // backup the result of linktalbe
    link_table_size = end_page_idx - start_page_idx + 1;
    backup_link_table = malloc(link_table_size * sizeof(struct range));
    if(backup_link_table == NULL)
        return NULL;
    
    for(i = 0; i < link_table_size; i++){
        // backup
        backup_link_table[i].high = info->link_table[start_page_idx + i].high;
        backup_link_table[i].low = info->link_table[start_page_idx + i].low;
        // link
        info->link_table[start_page_idx + i].low = start_page_idx;
        info->link_table[start_page_idx + i].high = end_page_idx;
    }
    // update the operation inforamtion
    info->op_info.start_page = start_page_idx;
    info->op_info.end_page = end_page_idx;
    info->op_info.start_address = start_page_addr;
    info->op_info.length = length;

    *back_up_length = link_table_size;
    *convert_start_page = start_page_idx;
    return backup_link_table;

}

/**
 *  this function is used to restore the link table of converted address link table
 *  @param[in] info                necessary information of page system
 *  @param[in] backup_link_table   the pointer return by convert_address_to_page()
 *  @param[in] link_table_size     the backup linktable size by convert_address_to_page()
 *  @param[in] start_page_idx      the start page index by convert_address_to_page()
 */ 
static void restore_link_table(struct page_system_info *info, struct range* backup_link_table, 
                                UINT link_table_size, UINT start_page_idx){
    
    int i;
    for(i = 0; i < link_table_size; i++){
        info->link_table[start_page_idx + i].low = backup_link_table[i].low;
        info->link_table[start_page_idx + i].high = backup_link_table[i].high;
    }
    free(backup_link_table);

}



/**
 *  this function is used to read or write to the page by address
 *  @param[in] info             necessary information of page system
 *  @param[in] address          start address
 *  @param[in] length           length you want to write or read
 *  @param[in/out] data         write/read data
 *  @param[in] flag             READ WRITE macro
 *  @return error code
 *  @retval apiFailed           read/write failed
 *  @retval apiOK               read/write OK
 */ 
UCHAR page_read_or_write_by_address(struct page_system_info *info, UCHAR* address, 
                                ULONG length, UCHAR* data, UCHAR flag){
    
    int result;
    struct range *backup_link_table;
    UINT backup_link_table_len, convert_st_page;

    backup_link_table = convert_address_to_page(info, address, length, &backup_link_table_len, &convert_st_page);

    if(backup_link_table == NULL)
        return apiFailed;

    result = page_read_or_write(info, data, flag);

    restore_link_table(info, backup_link_table, backup_link_table_len, convert_st_page);


    return result;
}



/**
 *  this function is used to clear the page by address
 *  @param[in] info             necessary information of page system
 *  @param[in] address          start address
 *  @param[in] length           length you want to clear
 *  @param[in] pattern          value you want to set
 *  @return error code
 *  @retval apiFailed           clear failed
 *  @retval apiOK               clear OK
 */ 
UCHAR page_clear_by_address(struct page_system_info *info,UCHAR* address, ULONG length, UCHAR pattern ){

    int result;
    struct range *backup_link_table;
    UINT backup_link_table_len, convert_st_page;

    backup_link_table = convert_address_to_page(info, address, length, &backup_link_table_len, &convert_st_page);

    if(backup_link_table == NULL)
        return apiFailed;

    result = page_clear(info, pattern);

    restore_link_table(info, backup_link_table, backup_link_table_len, convert_st_page);

    return result;

}


#endif
