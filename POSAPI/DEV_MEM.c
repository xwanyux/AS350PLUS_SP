




#include "POSAPI.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "DEV_MEM.h"
#include <sys/stat.h>

#define G_BUFFER_SIZE 70
#define WRTIE_BUFFER_SIZE 1024

/**
 *  @brief this buffer is use for store string of get_mem_path,
 */ 
static UCHAR g_buffer[G_BUFFER_SIZE] = {0};

/**
 *  this function is to construct the memory path from dev_mem_info type
 *  @param[in] info       necessary information of memory system
 *  @note path should be "/home/root/name"
 */ 
static void get_mem_path(struct dev_mem_info *info){
    UCHAR system_path[] = "/home/root/";
    memset(g_buffer,0,G_BUFFER_SIZE);
    sprintf(g_buffer,"%s%s",system_path,info->system_name);
}


/**
 *  this function is use to initilaize the memory system region.
 *  @param[in] info         necessary information of memory system
 *  @return error code
 *  @retval apiOK           create success
 *  @retval apiFailed       create failed
 */ 
UCHAR mem_init(struct dev_mem_info *info){

int i, result;
FILE* fd;
struct stat   FileExist;
UCHAR write_buffer[WRTIE_BUFFER_SIZE] = {0}; 
    // check file exit

    get_mem_path(info);
    
	if(stat(g_buffer, &FileExist)==0)//if file exist. 
        return apiOK;
    
    fd = fopen(g_buffer, "wb+");
    result=ftruncate(fd, info->size);
    fclose(fd);
    if(!result)
    {
        printf("init fail:%d\n",result);
        return apiFailed;
    }
        
    

    printf("%s init complete\n",g_buffer);
    sync();
    return apiOK;
}


/**
 *  this function is used to read or write to memory system by address
 *  @param[in] info         necessary information of memory system
 *  @param[in] mem_add      start read/write address 
 *  @param[in] length       read/write data length
 *  @param[in/out] data     flag = WRITE, as input data
 *                          flag = READ, as output data
 *  @param[in] flag         macro
 *                          @li WRITE
 *                          @li READ
 *  @return error code
 *  @retval apiOK           read/write memory success
 *  @retval apiFailed       read/write memory failed
 */ 
UCHAR mem_read_or_write(struct dev_mem_info *info, ULONG mem_add, ULONG length, UCHAR *data, UCHAR flag){

    FILE* fd;
    int result = -1;
    ULONG shift_address;
// printf("mem_add=%ld\t&mem_add=0x%p\tlength=%ld\tinfo->base_address=%ld\n",mem_add,&mem_add,length,info->base_address);
    // first check address is valid
	// if(flag == READ)
	// printf("mem_add=0x%ld\t&mem_add=0x%p\tlength=%ld\n",mem_add,&mem_add,length);
	
    if(mem_add < info->base_address)
        return apiFailed;

    shift_address = mem_add - info->base_address;
    // printf("(shift_address + length)=%ld\tinfo->size=%d\n",(shift_address + length),info->size);
    // printf("shift_address=0x%06x\n",shift_address);
    if( (shift_address + length) > (info->size - 1))
        return apiFailed;

    get_mem_path(info);
    fd = fopen(g_buffer, "rb+");
    if(fd == NULL)
        return apiFailed;

    fseek(fd, shift_address, SEEK_SET); 

    if(flag == WRITE)
        result = fwrite(data, 1, length, fd);
    else if(flag == READ)
        result = fread(data, 1, length, fd);

    fclose(fd);
    // printf("result=%d\n",result);
    if(result < 0)
       return apiFailed;

    // if(flag == WRITE);
    //     sync();
    return apiOK;
}

/**
 *  this function is used to clear the memory system by address
 *  @param[in] info         necessary information of memory system
 *  @param[in] mem_add      start read/write address 
 *  @param[in] length       read/write data length
 *  @param[in] Pattern      the clear value
 *  @return error code
 *  @retval apiOK           clear success
 *  @retval apiFailed       clear failed
 */ 
UCHAR mem_clear(struct dev_mem_info *info, ULONG mem_add, ULONG length, UCHAR pattern){

    UCHAR write_buffer[WRTIE_BUFFER_SIZE] = {0};
    FILE* fd;
    ULONG left_clear_length = length;
    int return_result, write_result;
    ULONG shift_address; 

    if(mem_add < info->base_address)
        return apiFailed;
    
    shift_address = mem_add - info->base_address;

    if( (shift_address + length) > (info->size - 1))
        return apiFailed;

    get_mem_path(info);
    fd = fopen(g_buffer, "r+");

    if(fd == NULL)
        return apiFailed;

    memset(write_buffer, pattern, WRTIE_BUFFER_SIZE);

    fseek(fd, shift_address, SEEK_SET); 

    return_result = apiOK;
    while(left_clear_length > 0){

        if(left_clear_length <= WRTIE_BUFFER_SIZE){
            write_result = fwrite(write_buffer, 1, left_clear_length, fd);
            left_clear_length = 0;
        }
        else{
            write_result = fwrite(write_buffer, 1, WRTIE_BUFFER_SIZE, fd);
            left_clear_length -= WRTIE_BUFFER_SIZE;
        }

        if(write_result < 0){
            return_result = apiFailed;
            break;
        }
    }

    fclose(fd);
    // if(return_result == apiOK)
    //     sync();
    return return_result;
 
}

