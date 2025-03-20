




#include <stdio.h>
#include "FS.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h> // malloc free
#include <string.h>

#include "SFSAPI.h"

/**
 *  this api use linux file system to create a comptiable file system to POSAPI
 *  I want to review some linux file open flag
 *  r+ (we use) (modify file and reset the lengh (with read and write) file need exit)
 *  w+ (not use) (if file not exit, create one, this flag always clean the file )
 *  a+ (not use) (you can't change the orignal result, only add new thing)
 */ 

static UCHAR file_system_directory[]	= "/home/root/fs/";		// FLASH
static UCHAR file_system_directory_SD[]	= "/media/mmcblk0p1/";		// SD

static UCHAR delete_file_system_command[]    = "rm -r /home/root/fs/";
static UCHAR delete_file_system_command_SD[] = "rm -r /media/mmcblk0p1/";

static UCHAR list_fs_exist[]    = "ls /home/root/ | grep fs";
static UCHAR list_fs_exist_SD[] = "ls /media/ | grep mmcblk0p1";

UCHAR	os_FS_MEDIA = MEDIA_FLASH;	// 0=FLASH, 1=SD Card, others: RFU

#define	MAX_FP_ID_NUM				16
#define	FILE_NAME_SIZE2				FILE_NAME_SIZE + 1	// ASCII-Z
FILE	*os_FP[MAX_FP_ID_NUM]= {0};		// max 16 file opened at the same time (SD only)
ULONG	os_FP_ID[MAX_FP_ID_NUM] = {0};		// id=0x80000000~0x8000000F (SD only)
char	os_FP_NAME[MAX_FP_ID_NUM][FILE_NAME_SIZE2] = {0};	// file name storage 16x64 (ASCII-Z)
ULONG	g_os_FPID = 0;				// current file ID

/**
 *  @brief the data sturcture use in file api in order to compatible to old data structure
 */ 
struct compatitble_old_file{
    struct FILE old_file;       /*!<   old file structure  */
    FILE* linux_file;           /*!<   linux file structrue */
};


// ---------------------------------------------------------------------------
// FUNCTION: To find an available FD ID.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : ID (0x80000000~0x8000000F)
//	     -1, not available.
// ---------------------------------------------------------------------------
int	FS_FindEmptyFID( void )
{
int	i;


	for( i=0; i<MAX_FP_ID_NUM; i++ )
	   {
	   if( os_FP_ID[i] == 0 )
	     return( i );
	   }
	   
	return( -1 );
}

// ---------------------------------------------------------------------------
// FUNCTION: To seek an existent FD ID.
// INPUT   : fid (FD ID)
// OUTPUT  : none.
// RETURN  : Linux file handle.
//	     NULL, not available.
// ---------------------------------------------------------------------------
FILE	*FS_SeekFID( ULONG fid )
{
int	i;


	for( i=0; i<MAX_FP_ID_NUM; i++ )
	   {
	   if( os_FP_ID[i] == fid )
	     {
	     return( os_FP[i] );
	     }
	   }
	   
	return( NULL );
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup FD ID.
// INPUT   : id
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FS_SetupFID( int id  )
{
	os_FP_ID[id] = 0x80000000 + id;
	g_os_FPID = os_FP_ID[id];
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup FD ID.
// INPUT   : id
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FS_SetupFileName( int id, char *fileName  )
{
	memset( &os_FP_NAME[id][0], 0x00, FILE_NAME_SIZE2 );
	memmove( &os_FP_NAME[id][0], fileName, strlen(fileName) );
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup FD ID.
// INPUT   : id (0..n)
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FS_ClearFID( int id )
{
	os_FP_ID[id] = 0;
	os_FP[id] = 0;
	
	g_os_FPID = 0;
	
	memset( &os_FP_NAME[id][0], 0x00, FILE_NAME_SIZE2 );
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup FD ID.
// INPUT   : id
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FS_ClearAllFID( void )
{
int	i;

	
	for( i=0; i<MAX_FP_ID_NUM; i++ )
	   {
	   os_FP_ID[i] = 0;
	   os_FP[i] = 0;
	   memset( &os_FP_NAME[i][0], 0x00, FILE_NAME_SIZE2 );
	   }
	   
	g_os_FPID = 0;
}

// ---------------------------------------------------------------------------
// FUNCTION: To setup FD ID.
// INPUT   : id
// OUTPUT  : none.
// RETURN  : TRUE / FALSE
// ---------------------------------------------------------------------------
ULONG	FS_ClearFileName( char *fileName )
{
int	i;
UCHAR	len1, len2;


	len1 = strlen( fileName );
	for( i=0; i<MAX_FP_ID_NUM; i++ )
	   {
	   len2 = strlen( &os_FP_NAME[i][0] );
	   if( len1 == len2 )
	     {
	     if( memcmp( fileName, &os_FP_NAME[i][0], len1 ) == 0 )
	       {
	       FS_ClearFID(i);	// clear id and name
	       return( TRUE );	// found
	       }
	     }
	   }
	   
	return( FALSE );	// not found
}

// ---------------------------------------------------------------------------
/**
 *  2023-06-08, new api to select media to be accessed.
 * @input  media: 0=flash (default), 1=sdcard
 */ 
UCHAR	api_fs_select( UCHAR media )
{
UCHAR	result = apiOK;


	if( (media == MEDIA_FLASH) || (media == MEDIA_SD) || (media == MEDIA_RAM) )
	  os_FS_MEDIA = media;
	else
	  result = apiFailed;	// others RFU
	  
	return( result );
}

// ---------------------------------------------------------------------------
/**
 *  this function is used to initialize file system
 *  @return error code
 *  @retval apiOK      always apiOK
 *  @note  just create a directory fs in /home/root/
 */ 
#if	0
UCHAR api_fs_init( void ){

    FILE *file_cmmd;

    UCHAR result[10] = {0};
    UCHAR commd_buffer[70] = {0};

    int i;

//  printf("\nos_FS_MEDIA=%x ", os_FS_MEDIA );
//  LIB_DispHexByte( 5, 0, os_FS_MEDIA );
    switch( os_FS_MEDIA )
    	{
    	case MEDIA_FLASH:
    	     file_cmmd = popen(list_fs_exist, "r");
    	     break;
    	
    	case MEDIA_SD:
    	     file_cmmd = popen(list_fs_exist_SD, "r");
    	     break;
    	     
    	case MEDIA_RAM:
    	     return( api_sfs_init() );
    	     
    	default:
    	     return( apiFailed );
    	}
      
    fgets(result, 10,file_cmmd);
    pclose(file_cmmd);

    // mean file exit
    if(result[0] != 0)
      {
      printf("DIR found\n");

      if( os_FS_MEDIA == MEDIA_SD )
      	{
        UCHAR cmd[] = "mount /dev/mmcblk0p1 /media/mmcblk0p1";
        memmove( commd_buffer, cmd, sizeof(cmd) );
        goto EXIT;
	}
	
      return apiOK;
      }
    else
      printf("DIR not found\n");
      
    if( os_FS_MEDIA == MEDIA_FLASH )
      {
      UCHAR cmmd[] = "mkdir";
      sprintf(commd_buffer, "%s %s",cmmd,file_system_directory);
      }
    else
      {
      UCHAR cmd[] = "mkdir -p /media/mmcblk0p1 && mount /dev/mmcblk0p1 /media/mmcblk0p1";
      memmove( commd_buffer, cmd, sizeof(cmd) );
      }

EXIT:
    file_cmmd = popen(commd_buffer, "r");
    pclose(file_cmmd);
    
    FS_ClearAllFID();

    return apiOK;
}
#endif

// ---------------------------------------------------------------------------
UCHAR api_fs_init( void ){

    FILE *file_cmmd;

    UCHAR result[10] = {0};
    UCHAR commd_buffer[70] = {0};

    int i;

    switch( os_FS_MEDIA )
    	{
    	case MEDIA_FLASH:
    	     file_cmmd = popen(list_fs_exist, "r");
    	     break;
    	
    	case MEDIA_SD:
    	     file_cmmd = popen(list_fs_exist_SD, "r");
    	     break;
    	     
    	case MEDIA_RAM:
    	     return( api_sfs_init() );
    	     
    	default:
    	     return( apiFailed );
    	}
    
    
    // The following processes are only for FLASH & SD
    memset( result, 0x00, sizeof(result) );
    fgets(result, 10,file_cmmd);
    pclose(file_cmmd);

    // mean file exit
    if(result[0] != 0)
      {
//    printf("DIR found\n");

      if( os_FS_MEDIA == MEDIA_SD )
      	{
      	printf("DIR found - SD\n");
      	
//        printf("FSCK\n");
//        UCHAR cmd00[] = "fsck /dev/mmcblk0p1";
//        memmove( commd_buffer, cmd00, sizeof(cmd00) );
//        file_cmmd = popen(commd_buffer, "r");
//        pclose(file_cmmd);
      	
      	printf("MOUNT\n");
        UCHAR cmd01[] = "mount /dev/mmcblk0p1 /media/mmcblk0p1";
        memmove( commd_buffer, cmd01, sizeof(cmd01) );
	file_cmmd = popen(commd_buffer, "r");
	pclose(file_cmmd);

	printf("LIST DEV\n");
        UCHAR cmd02[] = "ls /dev/ | grep mmcblk0p1";
        memmove( commd_buffer, cmd02, sizeof(cmd02) );
	file_cmmd = popen(commd_buffer, "r");
	memset( result, 0x00, sizeof(result) );
	fgets(result, 10,file_cmmd);
	pclose(file_cmmd);
	
//	LIB_DumpHexData( 0, 3, 10, result );
	if( result[0] == 0 )
	  return( apiFailed );	// SD card not found
	}
      else
      	printf("DIR found - FLASH\n");
      	
      return( apiOK );	// SD mounted OK
      }
//    else
//      printf("DIR not found\n");
    
    
    // The following process is used to setup a new device for FLASH or SD
    if( os_FS_MEDIA == MEDIA_FLASH )
      {
      printf("DIR not found - FLASH\n");
      
      UCHAR cmmd[] = "mkdir";
      sprintf(commd_buffer, "%s %s",cmmd,file_system_directory);
      file_cmmd = popen(commd_buffer, "r");
      pclose(file_cmmd);

      FS_ClearAllFID();
      
      return( apiOK );
      }
    else
      {
      printf("DIR not found - SD\n");
      
      printf("MKDIR\n");
//    UCHAR cmd1[] = "mkdir -p /media/mmcblk0p1 && mount /dev/mmcblk0p1 /media/mmcblk0p1";
      UCHAR cmd1[] = "mkdir -p /media/mmcblk0p1";
      memmove( commd_buffer, cmd1, sizeof(cmd1) );
      file_cmmd = popen(commd_buffer, "r");
      pclose(file_cmmd);

      printf("FSCK\n");
      UCHAR cmd2[] = "fsck /dev/mmcblk0p1";
      memmove( commd_buffer, cmd2, sizeof(cmd2) );
      file_cmmd = popen(commd_buffer, "r");
      pclose(file_cmmd);
      
      printf("MOUNT\n");
      UCHAR cmd3[] = "mount /dev/mmcblk0p1 /media/mmcblk0p1";
      memmove( commd_buffer, cmd3, sizeof(cmd3) );
      file_cmmd = popen(commd_buffer, "r");
      pclose(file_cmmd);
      
      UCHAR cmd02[] = "ls /dev/ | grep mmcblk0p1";
      memmove( commd_buffer, cmd02, sizeof(cmd02) );
      file_cmmd = popen(commd_buffer, "r");
      memset( result, 0x00, sizeof(result) );
      fgets(result, 10,file_cmmd);
      pclose(file_cmmd);
      if( result[0] == 0 )
	return( apiFailed );	// SD card not found
      
      return( apiOK );	// SD mounted OK
      }
}

// ---------------------------------------------------------------------------
/**
 *  this function is used to delete all the file in file system
 *  @note just delete the /home/root/fs/ directory,
 *        if you runing this function , but you are not in root directory
 *        will cause some strange side effect (for now seem does matter)
 */
void api_fs_format( void ){

FILE *fp;


     switch( os_FS_MEDIA )
    	{
    	case MEDIA_FLASH:
    	     break;
    	     
    	case MEDIA_SD:
    	     return;	// not supported if SD Card (temp NA)
    	     
    	case MEDIA_RAM:
    	     return( api_sfs_format() );
    	
    	default:
    	     return;
    	}
    
    if( os_FS_MEDIA == MEDIA_FLASH )
      {
      fp = popen(delete_file_system_command, "r");
      pclose(fp);

      // create the directory
      api_fs_init();
      }
}

// ---------------------------------------------------------------------------
/**
 *  this function is used to open file with file name
 *  @param[in] fileName         file name
 *  @param[in] mode             RFU
 *  @note in POSAPI, we don't have another directory
 *        all file just /home/root/fileName
 */ 
struct FILE *api_fs_open( char *fileName, UCHAR mode ){

    UCHAR path_buffer[50] = {0};
    FILE* linux_file;
    int	  id;


    switch( os_FS_MEDIA )
    	{
    	case MEDIA_FLASH:
    	     sprintf(path_buffer, "%s%s",file_system_directory,fileName);
    	     break;
    	
    	case MEDIA_SD:
    	     sprintf(path_buffer, "%s%s",file_system_directory_SD,fileName);
    	     break;
    	     
    	case MEDIA_RAM:
    	     return( api_sfs_open( fileName, mode) );
    	     
    	default:
    	     return( NULL );
    	}

//    printf("%s\n",path_buffer);

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
    
//    if( os_FS_MEDIA == MEDIA_SD )
//      {
      id = FS_FindEmptyFID();
      if( id != -1 )
        {
        os_FP[id] = &(ptr->old_file);
        FS_SetupFID( id );
        FS_SetupFileName( id, fileName );
        }
      else
      	return( NULL );
//      }

    return &(ptr->old_file);
}

// ---------------------------------------------------------------------------
//	AP layer function call
// ---------------------------------------------------------------------------
UCHAR	api_fs2_open( UCHAR len, char *fileName, ULONG *fdid )
{
UCHAR	result = apiOK;
FILE	*fh;

	
	fh = api_fs_open( fileName, 0 );	// mode always = 0
	
//	_PRINTF("fh=%p\n", fh);
	
	if( fh != NULL )
	  {
	  *fdid = g_os_FPID;
//	  _PRINTF("os_fid=%x\n",g_os_FPID );
	  }
	else
	  result = apiFailed;

	return( result );
}

// ---------------------------------------------------------------------------
/**
 *  this function is used to close the file
 *  @param[in] pf          old fild type pointer
 */
void api_fs_close( struct  FILE *pf ){

    if( os_FS_MEDIA == MEDIA_RAM )
      return( api_sfs_close( pf ) );

    struct compatitble_old_file *old_pt = (struct compatitble_old_file *)pf;

    fclose(old_pt->linux_file);
    free(old_pt);
}

// ---------------------------------------------------------------------------
//	AP layer function call
// ---------------------------------------------------------------------------
UCHAR	api_fs2_close( ULONG fid )
{
UCHAR	result = apiFailed;
FILE	*fh;


	fh = FS_SeekFID( fid );
	if( fh )
	  {
	  api_fs_close( fh );
	  FS_ClearFID( fid - 0x80000000 );
	  
	  result = apiOK;
	  }
	  
	return( result );
}

// ---------------------------------------------------------------------------
/**
 *  this function is to sync the file system
 *  
 */
void api_fs_sync( void ){
//    sync();	// TEMP: (na)
}

// ---------------------------------------------------------------------------
/**
 *  this function is used to create file with file name
 *  @param[in] fname            file name
 *  @param[in] fileType         not used (RFU)
 *  @return error code
 *  @retval apiFailed           create file fail
 *  @retval apiOK               create file success
 */ 
UCHAR	api_fs_create( char fname[], unsigned short fileType ){

    UCHAR path_or_cmd_buffer[70] = {0};
    UCHAR result = apiOK;
    UCHAR chmod_str[] = "chmod 777";
    FILE *cmd_file,*linux_file;
    
      
    switch( os_FS_MEDIA )
    	{
    	case MEDIA_FLASH:
    	     sprintf(path_or_cmd_buffer, "%s%s",file_system_directory,fname);
    	     break;
    	
    	case MEDIA_SD:
    	     sprintf(path_or_cmd_buffer, "%s%s",file_system_directory_SD,fname);
    	     break;
    	     
    	case MEDIA_RAM:
    	     return( api_sfs_create( fname, fileType ) );
    	     
    	default:
    	     return( apiFailed );
    	}

    //printf("%s\n",path_buffer);
    // use this to force create file
    linux_file = fopen(path_or_cmd_buffer, "w");

    if(linux_file == NULL)
        result= apiFailed;

    fclose(linux_file);

    if( os_FS_MEDIA == MEDIA_FLASH )
      sprintf(path_or_cmd_buffer, "%s %s%s",chmod_str,file_system_directory,fname);
    else
      sprintf(path_or_cmd_buffer, "%s %s%s",chmod_str,file_system_directory_SD,fname);

    // change mode can be write and read
    cmd_file = popen(path_or_cmd_buffer, "r");
    pclose(cmd_file);

    fflush(stdout);

    return result;
}

// ---------------------------------------------------------------------------
//	AP layer function call
// ---------------------------------------------------------------------------
UCHAR	api_fs2_create( UCHAR len, char fname[], unsigned short fileType )
{
UCHAR	result;


	result = api_fs_create( fname, fileType );
	return( result );
}

// ---------------------------------------------------------------------------
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

      
    switch( os_FS_MEDIA )
    	{
    	case MEDIA_FLASH:
    	     sprintf(cmmd_or_path_buffer, "%s%s",file_system_directory,fname);
    	     break;
    	
    	case MEDIA_SD:
    	     sprintf(cmmd_or_path_buffer, "%s%s",file_system_directory_SD,fname);
    	     break;
    	     
    	case MEDIA_RAM:
    	     return( api_sfs_delete( fname ) );
    	     
    	default:
    	     return( apiFailed );
    	}

    //printf("%s\n",cmmd_or_path_buffer);
    // use this to check file exist
    linux_file = fopen(cmmd_or_path_buffer, "r");

    if(linux_file == NULL)
        return apiFailed;
    
    fclose(linux_file);

    memset(cmmd_or_path_buffer, 0 , 70);
    
    if( os_FS_MEDIA == MEDIA_FLASH )
      sprintf(cmmd_or_path_buffer, "%s%s",delete_file_system_command,fname);
    else
      sprintf(cmmd_or_path_buffer, "%s%s",delete_file_system_command_SD,fname);

    //printf("%s\n",cmmd_or_path_buffer);

    cmd_file = popen(cmmd_or_path_buffer, "r");
    pclose(cmd_file);

    return apiOK;

}

// ---------------------------------------------------------------------------
//	AP layer function call
// ---------------------------------------------------------------------------
UCHAR	api_fs2_delete( UCHAR len, char fname[] )
{
UCHAR	result;


	result = api_fs_delete( fname );
	
	if( result == apiOK )
	  FS_ClearFileName( fname );
	  
	return( result );
}

// ---------------------------------------------------------------------------
/**
 *  this function is used to read file from file system
 *  @param[in] pFile        old file data structure pointer
 *  @param[out] buff        data to be read in this buffer
 *  @param[in] length       maximum length to read
 *  @return number data you read or -1 (read failed)
 */ 
ULONG api_fs_read( struct FILE *pFile, UCHAR *buff, ULONG length ){

    if( os_FS_MEDIA == MEDIA_RAM )
      return( api_sfs_read( pFile, buff, length ) );

    FILE* linux_file = ((struct compatitble_old_file*) pFile)->linux_file;

#if	0
    ULONG len = fread(buff+4,1, length, linux_file);
    buff[0] = len & 0x000000ff;
    buff[1] = (len & 0x0000ff00) >> 8;
    buff[2] = (len & 0x00ff0000) >> 16;
    buff[3] = (len & 0xff000000) >> 24;
#else
	ULONG len = fread(buff,1, length, linux_file);
#endif

    return( len );
    
//    return fread(buff,1, length, linux_file);

}

// ---------------------------------------------------------------------------
//	AP layer function call
// ---------------------------------------------------------------------------
ULONG	api_fs2_read( ULONG fid, ULONG length, UCHAR *buff )
{
FILE	*fh;
ULONG	len = 0xffffffff;


	fh = FS_SeekFID( fid );
	if( fh )
	  len = api_fs_read( fh, buff, length );

	return( len );
}

// ---------------------------------------------------------------------------
/**
 *  this function is used to write file to file system
 *  @param[in] pFile        old file data sturcture poniter
 *  @param[in] buff         data to be wirte to file system
 *  @param[in] length       number of data to be write
 *  @return  number data be successful write or -1 (write failed)
 */ 
ULONG api_fs_write( struct FILE *pFile, UCHAR *buff, ULONG length ){
	
    if( os_FS_MEDIA == MEDIA_RAM )
      return( api_sfs_write( pFile, buff, length ) );
      
//  int result;
    FILE* linux_file = ((struct compatitble_old_file*) pFile)->linux_file;
    ULONG len =  fwrite(buff,1,length,linux_file);
    
//    buff[0] = len & 0x000000ff;
//    buff[1] = (len & 0x0000ff00) >> 8;
//    buff[2] = (len & 0x00ff0000) >> 16;
//    buff[3] = (len & 0xff000000) >> 24;

    return( len );
    
}

// ---------------------------------------------------------------------------
//	AP layer function call
// ---------------------------------------------------------------------------
ULONG	api_fs2_write( ULONG fid, ULONG length, UCHAR *buff )
{
FILE	*fh;
ULONG	len = 0xffffffff;


	fh = FS_SeekFID( fid );
	if( fh )
	  len = api_fs_write( fh, buff, length );

	return( len );
}

// ---------------------------------------------------------------------------
/**
 *  this function is used to change the file pointer postion.
 *  @param[in] pFile      old file data sturcture poniter
 *  @param[in] position   position you want to change
 *  @return error code
 *  @retval apiOK         change position success
 *  @retval apiFailed     change position failed
 */ 
UCHAR api_fs_seek( struct FILE *pFile, ULONG position ){

    if( os_FS_MEDIA == MEDIA_RAM )
      return( api_sfs_seek( pFile, position ) );

    FILE* linux_file = ((struct compatitble_old_file*) pFile)->linux_file;

    if(fseek(linux_file, position, SEEK_SET) == 0)
        return apiOK;
    return apiFailed;
}

// ---------------------------------------------------------------------------
UCHAR api_fs_fseek( struct FILE *pFile, ULONG position, int origin ){

    FILE* linux_file = ((struct compatitble_old_file*) pFile)->linux_file;

    if(fseek(linux_file, position, origin) == 0)
        return apiOK;
    return apiFailed;
}

// ---------------------------------------------------------------------------
//	AP layer function call
//
// origin =	SEEK_SET (beginning of file)
//		SEEK_END (end of file)
//		SEEK_CUR (current position of file pointer)
UCHAR	api_fs2_seek( ULONG fid, ULONG offset, int origin )
{
UCHAR	result = apiFailed;
FILE	*fh;


	fh = FS_SeekFID( fid );
	if( fh )
	  result = api_fs_fseek( fh, offset, origin );
	
	return( result );
}

// ---------------------------------------------------------------------------
/**
 *  this function is used to tell the current pointer postion.
 *  @param[in] pFile    old file data sturcture pointer
 *  @return position of pointer or -1(tell failed)
 */ 
ULONG api_fs_tell( struct FILE *pFile ){
	
    if( os_FS_MEDIA == MEDIA_RAM )
      return( api_sfs_tell( pFile ) );
      
    FILE* linux_file = ((struct compatitble_old_file*) pFile)->linux_file;

    return ftell(linux_file);
}

// ---------------------------------------------------------------------------
//	AP layer function call
// ---------------------------------------------------------------------------
UCHAR	api_fs2_tell( ULONG fid, ULONG *offset )
{
UCHAR	result = apiFailed;
FILE	*fh;


	fh = FS_SeekFID( fid );
	if( fh )
	  {
          *offset = api_fs_tell( fh );
          if( *offset != 0xffffffff )
      	    return( apiOK );
          else
      	    return( apiFailed );
      	  }
}

// ---------------------------------------------------------------------------
/**
 *  this function is used to list the target directory.
 *  @param[in]
 *  @param[out]
 *  @return
 */ 
struct	FILE_DIR *api_fs_directory( void )
{
	return( SFS_fdir() );
}

// ---------------------------------------------------------------------------
//	FS default initialization
// ---------------------------------------------------------------------------
void	FS_RamInit( void )
{
	api_fs_select( MEDIA_RAM );
	vfs_init();
	
//	api_fs_init();		// initialize file system
//	api_fs_format();	// optional: format the file system before first usage.
}
