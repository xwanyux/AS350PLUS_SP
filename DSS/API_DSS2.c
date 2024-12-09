//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : API_DSS2.C                                                 **
//**  MODULE   : api_dss2_init()					    **
//**		 api_dss2_file()			                    **
//**		 api_dss2_burn()					    **
//**		 api_dss2_apid()					    **
//**		 api_dss2_run()						    **
//**									    **
//**  FUNCTION : API::DSS (Download Support System)			    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/12/09                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2009 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
// 8MB FLASH Memory Mapping
// 1'st program at	1002_0000 ~
// 2'nd program at	103B_0000 ~
//
// 16MB SDRAM Memory Mapping	(DSS working buffer)
// 20AF_0000 ~ 20FF_FFFF	(5MB + 64KB)	16MB SDRAM
// 20DF_0000 ~ 20FF_FFFF	(2MB + 64KB)	16MB SDRAM
// 2058_0000 ~ 207F_FFFF	(3MB - 512KB)	8MB  SDRAM with SD capability
//----------------------------------------------------------------------------
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>			//include sync function
#include "POSAPI.h"
#include "DownInterface.h"
#include "OS_LIB.h"
#include "OS_SECM.h"    // ==== [Debug] ====

UCHAR		os_DSS_mode = 0;	// 0 = single APP, 1 = double APP
UCHAR		DSS_APPbuffer[DSS_MAX_APP_SIZE];//10MB AP buffer
// UCHAR		DSS_ELfheader[]={0x7f,0x45,0x4c,0x46,0x01,0x01,0x01,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x28,0x00,0x01,0x00,0x00,0x00};//ELF file header
UCHAR		DSS_ELfheader[]={0x7f,0x45,0x4c,0x46};//ELF file header
ULONG		DSS_APlength;
extern ULONG DownFileSize;
extern ULONG DownburnSize;//Record the burning size


UCHAR	dss_compare_file_header(UCHAR *fdata)
{
	for(int i=0;i<sizeof(DSS_ELfheader);i++)
		if(*(fdata+i)!=DSS_ELfheader[i])
			return apiFailed;
	return apiOK;
}
// ---------------------------------------------------------------------------
// FUNCTION: To initialize DSS.
// INPUT   : mode - 0: single APP platform. (default)
//		    1: double APP platform.
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_dss2_init( UCHAR mode )
{
	if( (mode != 0) && (mode != 1) )
	  {
	  os_DSS_mode = 0;
	  return( apiFailed );
	  }
	else
	  {
	  os_DSS_mode = mode;
	  return( apiOK );
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: To store the code data to the DSS working space.
// INPUT   : offset - the beginning offset address to the program file for the code data.
//	     length - size of the current code data in bytes.
//	     data   - code data of the program file.
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// NOTE    : for ZA9L0 platform, the program file shall be: "FileName_sig.bin".
// ---------------------------------------------------------------------------
UCHAR	api_dss2_file( ULONG offset, ULONG length, UCHAR *data )
{
UCHAR	result = apiFailed;
UCHAR	*pSDRAM = (UCHAR *)DSS_APPbuffer;


	if( length <= DSS_MAX_APP_SIZE )
	  {
	  memmove( pSDRAM, data, length );
	  DSS_APlength=length;
	  result = apiOK;
	  }
	  
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get application ID of the current active program.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : program ID, 0 = the 1'st application. (default)
//			 1 = the 2'nd application. (only effective for the double APP)
// ---------------------------------------------------------------------------
UCHAR	api_dss2_apid( void )
{
FILE	*ptr_apid;
struct stat   FileExist;
UCHAR const apidname[]={"/home/root/fs/apid"};
UCHAR	apid=0;
UCHAR	apid_bak=0;
UCHAR	result=0;
	if(stat(apidname, &FileExist)!=0)//if file "apid" not exist. create file and write default value 0 in it. 
	{
		ptr_apid = fopen("/home/root/fs/apid","wb+");
		fwrite(&apid,1,1,ptr_apid);
	}		
	else
	{//if file "apid" exist. read apid from it.
		ptr_apid = fopen("/home/root/fs/apid","rb+");
		fread(&apid,1,1,ptr_apid);	
		if( (apid != 0) && (apid != 1) )
	  		apid = 0;
	}
	return( apid );
}
#if 0
// ---------------------------------------------------------------------------
// FUNCTION: To get application ID of the current active program.
//	     Called by OS only.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  - APID matched
//	     FALSE - APID not matched
// ---------------------------------------------------------------------------
ULONG	api_dss2_apid_EX( void )
{
UCHAR	apid_old;
UCHAR	apid_new = 0;
UINT	flag;
ULONG	IntState;


	OS_FLS_GetData( F_ADDR_APP_ID, APP_ID_LEN, (UCHAR *)&apid_old );
	if( (apid_old != 0) && (apid_old != 1) )
	  apid_old = 0;
	  
	apid_new = api_dss2_apid();
	
#ifdef	_F97_ENABLED_	// for AS350 only
	OS_FLS_GetData( F_ADDR_REF_F97, REF_F97_LEN, (UCHAR *)&flag );
	if( flag != FLAG_REF_F97_ON )
	  {
	  OS_FLS_GetData( F_ADDR_TAMPER_DETECT, TAMPER_DETECT_LEN, (UCHAR *)&flag );
	  if( flag != FLAG_DETECT_TAMPER_OFF )
	    {
	    IntState = BSP_DisableInterrupts( BSP_INT_MASK );
	    
	    flag = FLAG_DETECT_TAMPER_OFF;
	    OS_FLS_PutData( F_ADDR_TAMPER_DETECT, TAMPER_DETECT_LEN, (UCHAR *)&flag );
	    
	    BSP_RestoreInterrupts( IntState );
	    }
	  }
#endif
	if( apid_old == apid_new )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To burn the specified program code from SDRAM to FLASH memory.
// INPUT   : apid - applicaion id (0 or 1).
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_dss2_burn_EX( UCHAR apid )
{
ULONG	i;
ULONG	SectorAddr;
ULONG	IntState;
ULONG	SectorStartAddr;
ULONG	SectorLastAddr;
ULONG	Sectors;
ULONG	ByteCount;
ULONG	Size;
ULONG	Counter;
UCHAR	*pSDRAM = (UCHAR *)DSS_WORK_BASE;
UCHAR	result = apiFailed;


	if( (os_DSS_mode == 0) && (apid != 0) )
	  return( apiFailed );

	// PATCH: 2013-11-04, check file flag
	if( (*(pSDRAM+0x0C) != 0xA0) || (*(pSDRAM+0x0D) != 0xB1) || (*(pSDRAM+0x0E) != 0xC2) || (*(pSDRAM+0x0F) != 0xD3) )
	  return( apiFailed );

	IntState = BSP_DisableInterrupts( BSP_INT_MASK );
//	DCacheOff();
	
	IdentifyFlash16( 0 );
	
	// erase FLASH sectors
	if( apid == 0 )
	  {	// erase APP1
	  SectorStartAddr = DSS_APP1_BASE;
	  
	  if( os_DSS_mode == 0 )
	    SectorLastAddr  = DSS_APP2_END;
	  else
	    SectorLastAddr  = DSS_APP1_END;

	  // 2013-11-29
	  // change Code Segment in ECB offset 0x22~0x23 to DSS_APP1_BASE
	  *(pSDRAM+0x22) = ((DSS_APP1_BASE & 0xFFFF0000) >> 16) & 0xFF;
	  *(pSDRAM+0x23) = ((DSS_APP1_BASE & 0xFFFF0000) >> 24) & 0xFF;
	  }
	else
	  {	// erase APP2
	  SectorStartAddr = DSS_APP2_BASE;
	  SectorLastAddr  = DSS_APP2_END;
	  
	  // change Code Segment in ECB offset 0x22~0x23 to DSS_APP2_BASE
	  *(pSDRAM+0x22) = ((DSS_APP2_BASE & 0xFFFF0000) >> 16) & 0xFF;
	  *(pSDRAM+0x23) = ((DSS_APP2_BASE & 0xFFFF0000) >> 24) & 0xFF;
	  }
	
	SectorAddr = SectorStartAddr;
	Sectors = (SectorLastAddr - SectorAddr + 1) / FlashBlockSize;	// num of sectors to be erased
	
	for( i=0; i<Sectors; i++ )
	   {	      
	   if( FLASH_BlankCheckSector( (void *)SectorAddr, FlashBlockSize ) == FALSE )
	     {
	     Size = EraseSector( SectorAddr );
	     if( Size == 0 )
	       {
	       break;
	       }
	     }
	   else
	     Size = FlashBlockSize;
	     
	   SectorAddr += Size;
	   ByteCount += Size;
	   }

	ProgramUnlock();		// unlock flash

	// PATCH: 2013-09-23, temp erase the 1'st byte of flag to 0xFF for the 1st APP1
	if( apid == 0 )
	  *(pSDRAM+0x0C) = 0xFF;

	// write program codes from SDRAM to FLASH
	// total file size = 0x144 + software length (in offset 0x139)
	Size = (*(pSDRAM+0x139)*0x1000000) + (*(pSDRAM+0x13A)*0x10000) + (*(pSDRAM+0x13B)*0x100) + (*(pSDRAM+0x13C)) + 0x144;
	if( FlashWrite_Ex( (void *)SectorStartAddr, pSDRAM, Size ) == FLASH_SUCCESS )
	  {
	  result = apiOK;
	  }
	
	SystemRollbackProcess();

	ProgramLock();
	
//	DCacheOn();
	BSP_RestoreInterrupts( IntState );
	
	return( result );
}
#endif
// ---------------------------------------------------------------------------
// FUNCTION: To burn the specified program code from SDRAM to FLASH memory.
// INPUT   : apid - applicaion id (0 or 1).
// OUTPUT  : none.
// RETURN  : apiOK
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_dss2_burn( UCHAR apid )
{
FILE	*ptr_apfile;
UCHAR	result=apiOK;
ULONG	BurnSize;
int		ret;
	if( (os_DSS_mode == 0) && (apid != 0) )
	  return( apiFailed );
	DSS_APlength=( DSS_APPbuffer[1] | ((ULONG)DSS_APPbuffer[2]<<8)| ((ULONG)DSS_APPbuffer[3]<<16)| ((ULONG)DSS_APPbuffer[4]<<24) );
	// PATCH: 2013-11-04, check file flag
	if( dss_compare_file_header(&DSS_APPbuffer[5])!= apiOK )//SOH(1B)+Length(4B)=5B
	{
		// for(int k=0;k<2100;k++)
		// 	printf(" 0x%x ",DSS_APPbuffer[k]);
		// printf("\ndss_compare_file_header fail\n");
		return( apiFailed );
	}
	  
	
	// PATCH: 2013-11-04, check MID
//	if( *(pSDRAM + ECB_OFFSET_MID) != MID_ME )
//	  return( apiFailed );

	// erase FLASH sectors
	if( apid == 0 )
	{	// erase APP1
		remove("/home/root/fs/app1");
		//write received ap into app1 file  
		ptr_apfile=fopen("/home/root/fs/app1","wb+");
			ret=fwrite(&DSS_APPbuffer[5],DSS_APlength,1,ptr_apfile);
			if(!ret)
				perror("fwrite fail");
			BurnSize+=ret;
		fclose(ptr_apfile);
		chmod("/home/root/fs/app1",S_ISUID|S_ISGID|S_IRUSR|S_IXUSR);
	}
	else
	{	// erase APP2
		remove("/home/root/fs/app2");
		//write received ap into app1 file
		ptr_apfile=fopen("/home/root/fs/app2","wb+");
		fwrite(DSS_APPbuffer,1,DSS_APlength,ptr_apfile);
		fclose(ptr_apfile);
		chmod("/home/root/fs/app2",777);
	}
	
	return( result );
}
#ifdef _build_DSS_
UCHAR	api_dss2_burn2( UCHAR apid )    //Modified by Tammy
{
FILE	*ptr_apfile;
UCHAR	result;
ULONG	BurnSize;
int		ret;
DOWNDATAPAC *pseek = NULL;
	result=apiFailed;
	pseek=( DOWNDATAPAC *)DSS_APPbuffer;
	
	// erase FLASH sectors
	if( apid == 0 )
	{	// erase APP
        remove("/home/AP/IPC_client");
		//write received AP into IPC_client file  
        ptr_apfile=fopen("/home/AP/IPC_client","wb+");
		printf("DownFileSize=%d\n",DownFileSize);
		// for(int k=0;k<80;k++)
		// 	printf("%02x ",DSS_APPbuffer[k]);
		while(1)
		{
			ret=fwrite(&(pseek->Msg.Data),pseek->Len-5,1,ptr_apfile);
			if(!ret)
				perror("fwrite fail");
			DownburnSize = DownburnSize + pseek->Len-5;
			if ( (pseek->Msg.Type == DownDataEnd) || (DownFileSize < 1024) )
            {
            	if ( DownburnSize == DownFileSize )
            	{
                  fclose(ptr_apfile);
                  chmod("/home/AP/IPC_client",S_ISUID|S_ISGID|S_IRUSR|S_IWUSR|S_IXUSR);
                  chown("/home/AP/IPC_client", 1200, 879);
				  sync();
                  return apiOK;
                }
              	else  if ( DownburnSize < DownFileSize )
                {

                  continue;
                }
              	else
                {
					printf("DownburnSize=%d DownFileSize=%d\n",DownburnSize,DownFileSize);
                  return apiFailed;
                }
            }
            else
            {
              pseek = ( DOWNDATAPAC *)( (UCHAR *)&( pseek->Msg ) + pseek->Len + 1 );

            }
		}
		
	}
	else
	{	// erase APP2

		//write received ap into app1 file  
		ptr_apfile=fopen("/home/root/fs/app2","wb+");
		printf("DownFileSize=%d\n",DownFileSize);
		// for(int k=0;k<80;k++)
		// 	printf("%02x ",DSS_APPbuffer[k]);
		while(1)
		{
			ret=fwrite(&(pseek->Msg.Data),pseek->Len-5,1,ptr_apfile);
			if(!ret)
				perror("fwrite fail");
			DownburnSize = DownburnSize + pseek->Len-5;
			if ( (pseek->Msg.Type == DownDataEnd) || (DownFileSize < 1024) )
            {
            	if ( DownburnSize == DownFileSize )
            	{
                  fclose(ptr_apfile);
				  chmod("/home/root/fs/app2",S_ISUID|S_ISGID|S_IRUSR|S_IXUSR);
				  sync();
                  return apiOK;
                }
              	else  if ( DownburnSize < DownFileSize )
                {

                  continue;
                }
              	else
                {
					printf("DownburnSize=%d DownFileSize=%d\n",DownburnSize,DownFileSize);
                  return apiFailed;
                }
            }
            else
            {
              pseek = ( DOWNDATAPAC *)( (UCHAR *)&( pseek->Msg ) + pseek->Len + 1 );

            }
		}
	}
	
	return( result );
}

UCHAR	api_dss2_burnSP()   //Modified by Tammy
{
    FILE    *ptr_spfile, *ptr_spfile2;
    int     ret;
    DOWNDATAPAC *pseek = NULL;


    pseek = (DOWNDATAPAC *) DSS_APPbuffer;

    //write received firmware into IPC_server_tmp file
    ptr_spfile = fopen("/home/root/IPC_server_tmp", "wb+");

    printf("DownFileSize=%d\n", DownFileSize);

    while(1)
    {
        ret = fwrite(&(pseek->Msg.Data), pseek->Len - 5, 1, ptr_spfile);
        if(!ret)
            perror("fwrite fail");
        
        DownburnSize = DownburnSize + pseek->Len - 5;

        if((pseek->Msg.Type == DownDataEnd) || (DownFileSize < 1024))
        {
            if(DownburnSize == DownFileSize)
            {
                fclose(ptr_spfile);
                chmod("/home/root/IPC_server_tmp",S_ISUID|S_ISGID|S_IRUSR|S_IXUSR);
                sync();
                
                return apiOK;
            }
            else if(DownburnSize < DownFileSize)
            {
                continue;
            }
            else
            {
                printf("DownburnSize=%d DownFileSize=%d\n", DownburnSize, DownFileSize);
                return apiFailed;
            }
        }
        else
        {
            pseek = (DOWNDATAPAC *)((UCHAR *)&(pseek->Msg) + pseek->Len + 1);
        }
    }

    return apiFailed;
}

UCHAR	api_dss2_burnPrivateKey()   //Added by Tammy
{
    FILE    *ptr;
    int     ret;
    DOWNDATAPAC *pseek = NULL;


    pseek = (DOWNDATAPAC *) DSS_APPbuffer;

    //erase privatekey_EDC.der
    remove("/home/root/privatekey_EDC.der");

    //write received RSA private key into privatekey_EDC.der file
    ptr = fopen("/home/root/privatekey_EDC.der", "wb+");

    printf("DownFileSize=%d\n", DownFileSize);

    while(1)
    {
        ret = fwrite(&(pseek->Msg.Data), pseek->Len - 5, 1, ptr);
        if(!ret)
            perror("fwrite fail");
        
        DownburnSize = DownburnSize + pseek->Len - 5;

        if((pseek->Msg.Type == DownDataEnd) || (DownFileSize < 1024))
        {
            if(DownburnSize == DownFileSize)
            {
                fclose(ptr);
                chmod("/home/root/privatekey_EDC.der",S_ISUID|S_ISGID|S_IRUSR|S_IXUSR);
                sync();
                
                return apiOK;
            }
            else if(DownburnSize < DownFileSize)
            {
                continue;
            }
            else
            {
                printf("DownburnSize=%d DownFileSize=%d\n", DownburnSize, DownFileSize);
                return apiFailed;
            }
        }
        else
        {
            pseek = (DOWNDATAPAC *)((UCHAR *)&(pseek->Msg) + pseek->Len + 1);
        }
    }

    return apiFailed;
}

UCHAR	api_dss2_burnPublicKey()   //Added by Tammy
{
    FILE    *ptr;
    int     ret;
    DOWNDATAPAC *pseek = NULL;


    pseek = (DOWNDATAPAC *) DSS_APPbuffer;

    //erase publickey_EDC.der
    remove("/home/root/publickey_EDC.der");

    //write received RSA public key into publickey_EDC.der file
    ptr = fopen("/home/root/publickey_EDC.der", "wb+");

    printf("DownFileSize=%d\n", DownFileSize);

    while(1)
    {
        ret = fwrite(&(pseek->Msg.Data), pseek->Len - 5, 1, ptr);
        if(!ret)
            perror("fwrite fail");
        
        DownburnSize = DownburnSize + pseek->Len - 5;

        if((pseek->Msg.Type == DownDataEnd) || (DownFileSize < 1024))
        {
            if(DownburnSize == DownFileSize)
            {
                fclose(ptr);
                chmod("/home/root/publickey_EDC.der",S_ISUID|S_ISGID|S_IRUSR|S_IXUSR);
                sync();
                
                return apiOK;
            }
            else if(DownburnSize < DownFileSize)
            {
                continue;
            }
            else
            {
                printf("DownburnSize=%d DownFileSize=%d\n", DownburnSize, DownFileSize);
                return apiFailed;
            }
        }
        else
        {
            pseek = (DOWNDATAPAC *)((UCHAR *)&(pseek->Msg) + pseek->Len + 1);
        }
    }

    return apiFailed;
}

// ---------------------------------------------------------------------------
// FUNCTION: To burn the CA certificate to secure memory for TLS connection.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//	         apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_dss2_burn_CA_Certificate()   //Added by Tammy
{
    FILE    *ptr;
    int     ret;
    DOWNDATAPAC *pseek = NULL;
    UCHAR   buf[2048];
    UINT    i = 0;


    pseek = (DOWNDATAPAC *) DSS_APPbuffer;

    printf("DownFileSize=%d\n", DownFileSize);

    while(1)
    {
        memcpy(&buf[2 + i], &(pseek->Msg.Data), pseek->Len - 5);
        i += pseek->Len - 5;

        DownburnSize = DownburnSize + pseek->Len - 5;

        if((pseek->Msg.Type == DownDataEnd) || (DownFileSize < 1024))
        {
            if(DownburnSize == DownFileSize)
            {
                buf[0] = DownFileSize % 256;
                buf[1] = DownFileSize / 256;
                printf("buf[0] = %02X, buf[1] = %02X\n", buf[0], buf[1]);
                
                OS_SECM_PutData(ADDR_CA_CERTIFICATE, 2 + DownFileSize, buf);
                
                return apiOK;
            }
            else if(DownburnSize < DownFileSize)
            {
                continue;
            }
            else
            {
                printf("DownburnSize=%d DownFileSize=%d\n", DownburnSize, DownFileSize);
                return apiFailed;
            }
        }
        else
        {
            pseek = (DOWNDATAPAC *)((UCHAR *)&(pseek->Msg) + pseek->Len + 1);
        }
    }

    memset(buf, 0x00, sizeof(buf));
    
    return apiFailed;
}

// ---------------------------------------------------------------------------
// FUNCTION: To burn the client certificate to secure memory for TLS connection.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//	         apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_dss2_burn_client_Certificate()   //Added by Tammy
{
    FILE    *ptr;
    int     ret;
    DOWNDATAPAC *pseek = NULL;
    UCHAR   buf[5120];
    UINT    i = 0;


    pseek = (DOWNDATAPAC *) DSS_APPbuffer;

    printf("DownFileSize=%d\n", DownFileSize);

    while(1)
    {
        memcpy(&buf[2 + i], &(pseek->Msg.Data), pseek->Len - 5);
        i += pseek->Len - 5;

        DownburnSize = DownburnSize + pseek->Len - 5;

        if((pseek->Msg.Type == DownDataEnd) || (DownFileSize < 1024))
        {
            if(DownburnSize == DownFileSize)
            {
                buf[0] = DownFileSize % 256;
                buf[1] = DownFileSize / 256;
                printf("buf[0] = %02X, buf[1] = %02X\n", buf[0], buf[1]);
                
                OS_SECM_PutData(ADDR_CLIENT_CERTIFICATE, 2 + DownFileSize, buf);
                
                return apiOK;
            }
            else if(DownburnSize < DownFileSize)
            {
                continue;
            }
            else
            {
                printf("DownburnSize=%d DownFileSize=%d\n", DownburnSize, DownFileSize);
                return apiFailed;
            }
        }
        else
        {
            pseek = (DOWNDATAPAC *)((UCHAR *)&(pseek->Msg) + pseek->Len + 1);
        }
    }

    memset(buf, 0x00, sizeof(buf));

    return apiFailed;
}

// ---------------------------------------------------------------------------
// FUNCTION: To burn the client private key to secure memory for TLS connection.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//	         apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_dss2_burn_client_privateKey()   //Added by Tammy
{
    FILE    *ptr;
    int     ret;
    DOWNDATAPAC *pseek = NULL;
    UCHAR   buf[2048];
    UINT    i = 0;


    pseek = (DOWNDATAPAC *) DSS_APPbuffer;

    printf("DownFileSize=%d\n", DownFileSize);

    while(1)
    {
        memcpy(&buf[2 + i], &(pseek->Msg.Data), pseek->Len - 5);
        i += pseek->Len - 5;

        DownburnSize = DownburnSize + pseek->Len - 5;

        if((pseek->Msg.Type == DownDataEnd) || (DownFileSize < 1024))
        {
            if(DownburnSize == DownFileSize)
            {
                buf[0] = DownFileSize % 256;
                buf[1] = DownFileSize / 256;
                printf("buf[0] = %02X, buf[1] = %02X\n", buf[0], buf[1]);
                
                OS_SECM_PutData(ADDR_CLIENT_PRV_KEY, 2 + DownFileSize, buf);
                
                return apiOK;
            }
            else if(DownburnSize < DownFileSize)
            {
                continue;
            }
            else
            {
                printf("DownburnSize=%d DownFileSize=%d\n", DownburnSize, DownFileSize);
                return apiFailed;
            }
        }
        else
        {
            pseek = (DOWNDATAPAC *)((UCHAR *)&(pseek->Msg) + pseek->Len + 1);
        }
    }

    memset(buf, 0x00, sizeof(buf));

    return apiFailed;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To execute the specified application program, it will reboot after the function called.
// INPUT   : apid - application id (0 or 1).
// OUTPUT  : none.
// RETURN  : apiFailed - if condition is not satisfied.
//	     otherwise the specified AP will be activated automatically.
// ---------------------------------------------------------------------------
UCHAR	api_dss2_run( UCHAR apid )
{
FILE	*ptr_apfile;
UCHAR	result;
UCHAR	apid_bak=0;
struct stat   FileExist;
UCHAR const filename[]={"/home/root/fs/app1"};
UCHAR const filename2[]={"/home/root/fs/app2"};
UCHAR ScriptName[]="/etc/init.d/AS350Pautorun.sh";
UCHAR string[]="/home/root/fs/app";
UCHAR Slen=strlen(string);
UCHAR buffer[Slen];
UCHAR writeString[1];
UINT Flen=0,pos=0;
FILE *fp;
  fp = fopen( ScriptName, "rb+" );
  fseek( fp, 0, SEEK_END );
  Flen=ftell(fp);
  fseek( fp, Flen-Slen-1, SEEK_SET);//search from end of file, skip 1 char app"1".
  while(1)
  {
    fread(buffer, Slen, 1, fp);
    if(memcmp(string, buffer, Slen)==0)
      break;
    pos++;
    fseek( fp, Flen-Slen-1-pos, SEEK_SET);//step back 1 char from end of file
  }
  
	if( (os_DSS_mode == 0) && (apid != 0) )
	  return( apiFailed );
	
	if( apid == 0 )	// *** run APP1 ***
	  {	  
		//if file not exist
		if(stat(filename, &FileExist)!=0)
			return apiFailed;

		//if file exist, check file executable
	    result = apiOK;
		writeString[0]='1';

	  }
	else		// *** run APP2 ***
	  {
	 //if file not exist
		if(stat(filename2, &FileExist)!=0)
			return apiFailed;

		//if file exist, check file executable
	    result = apiOK;
		writeString[0]='2';
	    
	  }
	fwrite(writeString , 1 , 1 , fp );
  	fclose(fp);	
	if( result != apiOK )
	  {
	  goto EXIT;
	  }
	else
	  {	  
	  // store APP ID to FLASH backup area
		ptr_apfile=fopen("/home/root/fs/apid","wb");
		
		if( apid == 0 )
			apid_bak=0;
		else
			apid_bak=1;
			
		fwrite(&apid_bak,1,1,ptr_apfile);
		fclose(ptr_apfile);

	  }

	api_sys_reset( 0 );

EXIT:	
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the address of DSS working buffer.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : address of DSS working buffer.
// ---------------------------------------------------------------------------
UCHAR	*api_dss2_address( void )
{
	return( (UCHAR *)DSS_APPbuffer );
}
