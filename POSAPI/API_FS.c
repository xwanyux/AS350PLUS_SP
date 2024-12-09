




#include <stdio.h>
#include "FS.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h> // malloc free
#include <string.h>


/**
 *  this api use linux file system to create a comptiable file system to POSAPI
 *  I want to review some linux file open flag
 *  r+ (we use) (modify file and reset the lengh (with read and write) file need exit)
 *  w+ (not use) (if file not exit, create one, this flag always clean the file )
 *  a+ (not use) (you can't change the orignal result, only add new thing)
 */ 

static UCHAR file_system_directory[] = "/home/root/fs/";

static UCHAR delete_file_system_command[] = "rm -r /home/root/fs/";

static UCHAR list_fs_exist[] = "ls /home/root/ | grep fs";



/**
 *  @brief the data sturcture use in file api in order to compatible to old data structure
 */ 
struct compatitble_old_file{
    struct FILE old_file;       /*!<   old file structure  */
    FILE* linux_file;           /*!<   linux file structrue */
};


/**
 *  this function is used to initialize file system
 *  @return error code
 *  @retval apiOK      always apiOK
 *  @note  just create a directory fs in /home/root/
 */ 
UCHAR api_fs_init( void ){

    FILE *file_cmmd;

    UCHAR result[10] = {0};
    UCHAR commd_buffer[70] = {0};

    int i;

    file_cmmd = popen(list_fs_exist, "r");
    fgets(result, 10,file_cmmd);
    pclose(file_cmmd);

    // mean file exit
    if(result[0] != 0)
        return apiOK;

    UCHAR cmmd[] = "mkdir";

    sprintf(commd_buffer, "%s %s",cmmd,file_system_directory);

    file_cmmd = popen(commd_buffer, "r");
    pclose(file_cmmd);


    return apiOK;


}


/**
 *  this function is used to delete all the file in file system
 *  @note just delete the /home/root/fs/ directory,
 *        if you runing this function , but you are not in root directory
 *        will cause some strange side effect (for now seem does matter)
 */
void api_fs_format( void ){

    FILE *fp;

    fp = popen(delete_file_system_command, "r");
    pclose(fp);

    // create the directory
    api_fs_init();

}

/**
 *  this function is used to open file with file name
 *  @param[in] fileName         file name
 *  @param[in] mode             RFU
 *  @note in POSAPI, we don't have another directory
 *        all file just /home/root/fileName
 */ 
struct FILE	*api_fs_open( char *fileName, UCHAR mode ){

    UCHAR path_buffer[50] = {0};
    FILE* linux_file;

    sprintf(path_buffer, "%s%s",file_system_directory,fileName);

    printf("%s\n",path_buffer);

    linux_file = fopen(path_buffer,"r+");
    if(linux_file == NULL)
        return NULL;
    
    // this is the crutial part to embedd old file to new struture
    struct compatitble_old_file *ptr = malloc(sizeof(struct compatitble_old_file));


    // remember to update relative information of struct file
    // for now just update length (omit other)
    fseek(linux_file, 0L, SEEK_END);
    ptr->old_file.length = ftell(linux_file);
    rewind(linux_file);

    ptr->linux_file = linux_file;

    return &(ptr->old_file);
}

/**
 *  this function is used to close the file
 *  @param[in] pf          old fild type pointer
 */
void api_fs_close( struct  FILE *pf ){

    struct compatitble_old_file *old_pt = (struct compatitble_old_file *)pf;

    fclose(old_pt->linux_file);
    free(old_pt);
}
/**
 *  this function is to sync the file system
 *  
 */
void api_fs_sync( void ){
    sync();
}

/**
 *  this function is used to create file with file name
 *  @param[in] fname            file name
 *  @param[in] fileType         not used (not implemented)
 *  @return error code
 *  @retval apiFailed           create file fail
 *  @retval apiOK               create file success
 */ 
UCHAR	api_fs_create( char fname[], unsigned short fileType ){

    UCHAR path_or_cmd_buffer[70] = {0};
    UCHAR result = apiOK;
    UCHAR chmod_str[] = "chmod 777";
    FILE *cmd_file,*linux_file;

    sprintf(path_or_cmd_buffer, "%s%s",file_system_directory,fname);

    //printf("%s\n",path_buffer);
    // use this to force create file
    linux_file = fopen(path_or_cmd_buffer, "w");

    if(linux_file == NULL)
        result= apiFailed;

    fclose(linux_file);

    sprintf(path_or_cmd_buffer, "%s %s%s",chmod_str,file_system_directory,fname);

    // change mode can be write and read
    cmd_file = popen(path_or_cmd_buffer, "r");
    pclose(cmd_file);

    fflush(stdout);

    return result;
}


/**
 *  this function is used to delete the file with name
 *  @param[in] fname    file name
 *  @return error code
 *  @retval apiOK       delete success
 *  @retval apiFailed   delete failed
 *  @note not testing yet, should add some check this file exist or not
 *        if not exist , return apifailed
 */ 
UCHAR api_fs_delete( char fname[] ){

    UCHAR cmmd_or_path_buffer[70] = {0};
    FILE *cmd_file, *linux_file;


    sprintf(cmmd_or_path_buffer, "%s%s",file_system_directory,fname);

    //printf("%s\n",cmmd_or_path_buffer);
    // use this to check file exist
    linux_file = fopen(cmmd_or_path_buffer, "r");

    if(linux_file == NULL)
        return apiFailed;
    
    fclose(linux_file);

    memset(cmmd_or_path_buffer, 0 , 70);
    sprintf(cmmd_or_path_buffer, "%s%s",delete_file_system_command,fname);

    //printf("%s\n",cmmd_or_path_buffer);

    cmd_file = popen(cmmd_or_path_buffer, "r");
    pclose(cmd_file);

    return apiOK;

}


/**
 *  this function is used to read file from file system
 *  @param[in] pFile        old file data structure pointer
 *  @param[out] buff        data to be read in this buffer
 *  @param[in] length       maximum length to read
 *  @return number data you read or -1 (read failed)
 */ 
ULONG api_fs_read( struct FILE *pFile, UCHAR *buff, ULONG length ){

    FILE* linux_file = ((struct compatitble_old_file*) pFile)->linux_file;
    return fread(buff,1, length, linux_file);

}


/**
 *  this function is used to write file to file system
 *  @param[in] pFile        old file data sturcture poniter
 *  @param[in] buff         data to be wirte to file system
 *  @param[in] length       number of data to be write
 *  @return  number data be successful write or -1 (write failed)
 */ 
ULONG api_fs_write( struct FILE *pFile, UCHAR *buff, ULONG length ){
    int result;
    FILE* linux_file = ((struct compatitble_old_file*) pFile)->linux_file;
    result =  fwrite(buff,1,length,linux_file);
    
    return result;
    
}

/**
 *  this function is used to change the file pointer postion.
 *  @param[in] pFile      old file data sturcture poniter
 *  @param[in] position   position you want to change
 *  @return error code
 *  @retval apiOK         change position success
 *  @retval apiFailed     change position failed
 */ 
UCHAR api_fs_seek( struct FILE *pFile, ULONG position ){

    FILE* linux_file = ((struct compatitble_old_file*) pFile)->linux_file;

    if(fseek(linux_file, position, SEEK_SET) == 0)
        return apiOK;
    return apiFailed;
}

/**
 *  this function is used to tell the current pointer postion.
 *  @param[in] pFile    old file data sturcture pointer
 *  @return position of pointer or -1(tell failed)
 */ 
ULONG api_fs_tell( struct FILE *pFile ){
    FILE* linux_file = ((struct compatitble_old_file*) pFile)->linux_file;

    return ftell(linux_file);
}






