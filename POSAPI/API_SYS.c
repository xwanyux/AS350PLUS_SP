




#include "POSAPI.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_alg.h>
#include <linux/socket.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <fcntl.h>
// #include <crypto/if_alg.h>

#include "API_EXTIO.h"
#include "fbutils.h"
#include <string.h>
#include "ExtIODev.h"
#include "bsp_gpio.h"
#include "bsp_SHM.h"

// testing

#ifndef SOL_ALG
#define SOL_ALG 279
#endif
UCHAR SYS_brightness=7;
UCHAR API_RNGfd=0;
/*
**  set necessary GPIO and extension IO output 

*/
void sys_IOinit()
{
int opResult;
char devID[]="1782 4d10 ff";
    //GPIO
    BSP_IO_Init();
    //extension IO    
    EXTIO_IOINIT();
    //make USB1_2 power on and off to prevent from showing massive kernel message
    EXTIO_GPIO(_USB1_PWR_BANK_ADDR,_USB1_PWR_GPIO_PORT,_USB1_PWR_GPIO_NUM,1);
    // usleep(50000);
    // EXTIO_GPIO(_USB1_PWR_BANK_ADDR,_USB1_PWR_GPIO_PORT,_USB1_PWR_GPIO_NUM,0);
    #ifdef _4G_ENABLED_
    // EXTIO_GPIO(_4G_IGT_BANK_ADDR,_4G_IGT_GPIO_PORT,_4G_IGT_GPIO_NUM,1);//power on module L610
    // opResult=open("/sys/bus/usb-serial/drivers/option1/new_id",O_WRONLY);
    // if(opResult<0)
    //     return;
    // write(opResult,devID,sizeof(devID));
    // close(opResult);
    #endif
}
/**
 *    this function is used to calculate hash function for linux kernel using netlink.
 *    @param[in] sa                            necessary infomation for using netlink,
 *                                             you should provide salg_family = AF_ALG (always),
 *                                                                salg_type =  "hash",
 *                                                                salg_name =  hash algorithm name
 *    @param[in] data                           message to be hashed
 *    @param[in] data_length                    length of message
 *    @param[out] digest                        hashed result
 *    @param[out] digest_length                 hash result length
 *    @retval apiFailed                         hash failed
 *    @retval apiOK                             hash success
 */                                                          
UCHAR socket_IO_to_if_ALG_HASH(struct sockaddr_alg sa, UCHAR* data, ULONG data_length,
                        UCHAR* digest, ULONG digest_length){

    int opfd;
    int tfmfd;
    tfmfd = socket(AF_ALG, SOCK_SEQPACKET, 0);

    if(tfmfd < 0)
       return apiFailed;
 
    bind(tfmfd, (struct sockaddr *)&sa, sizeof(sa));
 
    opfd = accept(tfmfd, NULL, 0);

    if(opfd < 0){
        close(tfmfd);
        return apiFailed;
    }
 
    if(write(opfd, data, data_length) < 0){
        close(opfd);
        close(tfmfd);
        return apiFailed;
    };
    
    if(read(opfd, digest, digest_length) < 0){
        close(opfd);
        close(tfmfd);
        return apiFailed;
    };
 
 
    close(opfd);
    close(tfmfd);
    
    return apiOK;

}



// ---------------------------------------------------------------------------
// FUNCTION: To generate an n-byte random number.
// INPUT   : len  -- length in bytes. (1..n)
// OUTPUT  : dbuf -- n-byte random number.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------

ULONG api_sys_random_len( UCHAR *dbuf, UINT len )
{
int fd;
    if(API_RNGfd<3)
    {
        fd=open("/dev/hwrng",O_RDONLY|O_LARGEFILE);
        if(fd<1)
            return apiFailed;
        API_RNGfd=fd;
    }
     
    read(API_RNGfd,dbuf,len);
    return apiOK;
}

//dispose for too slow, hwrng is about 100 to 300 times faster than socket way.
/*
ULONG api_sys_random_len( UCHAR *dbuf, UINT len )
{
    int opfd;
    int tfmfd;

    tfmfd = socket(AF_ALG, SOCK_SEQPACKET, 0);

    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "rng", 
    .salg_name = "jitterentropy_rng" 
  };

    if(tfmfd < 0)
       return apiFailed;
    
    bind(tfmfd, (struct sockaddr *)&sa, sizeof(sa));
 
    opfd = accept(tfmfd, NULL, 0);

    if(opfd < 0){
        close(tfmfd);
        return apiFailed;
    }

    UINT temp = len;

    while (temp > 128)
    {
        if(read(opfd, dbuf, 128) < 0){
            close(opfd);
            close(tfmfd);
        return apiFailed;
        };
        dbuf += 128;
        temp -= 128; 
    }
    
    if(read(opfd, dbuf, temp) < 0){
        close(opfd);
        close(tfmfd);
        return apiFailed;
    };

    close(opfd);
    close(tfmfd);
    return apiOK;

}
*/

// ---------------------------------------------------------------------------
// FUNCTION: To generate an 8-byte random number.
// INPUT   : none.
// OUTPUT  : dbuf -- 8-byte random number.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG api_sys_random( UCHAR *dbuf )
{
    return api_sys_random_len(dbuf, 8);
}

void	api_sys_reset( UCHAR target )
{
	sync();
    backlight_control_LCDTFT(0);
	close_framebuffer();
	reboot(RB_AUTOBOOT);
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate the digest (fixed 20 bytes) by using SHA-1 in one step.
// INPUT   : length -- length of data to be hashed.
//           data   -- the data to be hashed.
// OUTPUT  : digest -- the digest. (20-byte)
// RETURN  : apiOK
//           apiFailed
// NOTE    : This function is called by EMV L2 kernel only.
// ---------------------------------------------------------------------------
ULONG	api_sys_SHA1( ULONG length, UCHAR *data, UCHAR *digest )
{

    struct sockaddr_alg sa = {
        .salg_family = AF_ALG,
        .salg_type = "hash",
        .salg_name = "sha1"
    };

    return socket_IO_to_if_ALG_HASH(sa,data,length,digest,20);

}

// ---------------------------------------------------------------------------
// FUNCTION: To generate the digest (fixed 16 bytes) by using MD5 in one step.
// INPUT   : length -- length of data to be hashed.
//           data   -- the data to be hashed.
// OUTPUT  : digest -- the digest. (16-byte)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_sys_MD5( ULONG length, UCHAR *data, UCHAR *digest )
{

    struct sockaddr_alg sa = {
        .salg_family = AF_ALG,
        .salg_type = "hash",
        .salg_name = "md5"
    };

    return socket_IO_to_if_ALG_HASH(sa,data,length,digest,16);

}

// ---------------------------------------------------------------------------
// FUNCTION: To generate the digest by using SHA-2 in one step. (SHA2_224)
// INPUT   : length -- length of data to be hashed.
//           data   -- the data to be hashed.
// OUTPUT  : digest -- the digest. (28-byte)
// RETURN  : none.
// ---------------------------------------------------------------------------
UCHAR	SYS_SHA2_224( ULONG length, UCHAR *data, UCHAR *digest )
{
// sha224_ctx	ctx;

    struct sockaddr_alg sa = {
        .salg_family = AF_ALG,
        .salg_type = "hash",
        .salg_name = "sha224"
    };

    return socket_IO_to_if_ALG_HASH(sa,data,length,digest,28);

// 	sha224_init(&ctx);
// 	sha224_update(&ctx, data, length );
// 	sha224_final(&ctx, digest);
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate the digest by using SHA-2 in one step. (SHA2_256)
// INPUT   : length -- length of data to be hashed.
//           data   -- the data to be hashed.
// OUTPUT  : digest -- the digest. (32-byte)
// RETURN  : none.
// ---------------------------------------------------------------------------
UCHAR	SYS_SHA2_256( ULONG length, UCHAR *data, UCHAR *digest )
{

    struct sockaddr_alg sa = {
        .salg_family = AF_ALG,
        .salg_type = "hash",
        .salg_name = "sha256"
    };

    return socket_IO_to_if_ALG_HASH(sa,data,length,digest,32);

}

// ---------------------------------------------------------------------------
// FUNCTION: To generate the digest by using SHA-2 in one step. (SHA2_384)
// INPUT   : length -- length of data to be hashed.
//           data   -- the data to be hashed.
// OUTPUT  : digest -- the digest. (48-byte)
// RETURN  : none.
// ---------------------------------------------------------------------------
UCHAR	SYS_SHA2_384( ULONG length, UCHAR *data, UCHAR *digest )
{
    struct sockaddr_alg sa = {
        .salg_family = AF_ALG,
        .salg_type = "hash",
        .salg_name = "sha384"
    };

    return socket_IO_to_if_ALG_HASH(sa,data,length,digest,48);
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate the digest by using SHA-2 in one step. (SHA2_512)
// INPUT   : length -- length of data to be hashed.
//           data   -- the data to be hashed.
// OUTPUT  : digest -- the digest. (64-byte)
// RETURN  : none.
// ---------------------------------------------------------------------------
UCHAR	SYS_SHA2_512( ULONG length, UCHAR *data, UCHAR *digest )
{
    struct sockaddr_alg sa = {
        .salg_family = AF_ALG,
        .salg_type = "hash",
        .salg_name = "sha512"
    };

    return socket_IO_to_if_ALG_HASH(sa,data,length,digest,64);
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate the digest by using SHA-2 in one step.
// INPUT   : length -- length of data to be hashed.
//           data   -- the data to be hashed.
//	     mode   -- SHA2_224, SHA2_256, SHA2_384, SHA2_512.
// OUTPUT  : digest -- the digest.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_sys_SHA2( UCHAR mode, ULONG length, UCHAR *data, UCHAR *digest )
{
ULONG	result = apiOK;


	switch( mode )
	      {
	      case HASH_SHA2_224:
	      	   
	      	   result = SYS_SHA2_224( length, data, digest );
	           break;
	           
	      case HASH_SHA2_256:
	      	   
	      	   result = SYS_SHA2_256( length, data, digest );
	      	   break;
	      	   
	      case HASH_SHA2_384:
	      	
	      	   result = SYS_SHA2_384( length, data, digest );
	      	   break;
	      	   
	      case HASH_SHA2_512:
	      	
	      	   result = SYS_SHA2_512( length, data, digest );
	      	   break;
	      	   
	      default:
	      	   
	      	   result = apiFailed;
	      	   break;
	      }
	      
	return( result );
}


/**
 *     This function is to write value 0/1 to extension IO which control backlight of LCDTFT
 *     @param[in] operation                value 0/1 (0 mean backlight off) (1 mean backlight on)
 */
void backlight_control_LCDTFT(UCHAR operation,UCHAR bright){
int result;  
UCHAR * dSHM;
UCHAR data=bright+48;
    // EXTIO_GPIO(_LCD_BL_EN_ADDR,_LCD_BL_EN_GPIO_PORT,_LCD_BL_EN_GPIO_NUM,operation);
    result=open("/sys/class/backlight/backlight-display/brightness", O_WRONLY);
    
    if(operation)
    {
        write(result,&data, 1);//brightness value from 0-7,correspond to PWM value 0 4 8 16 32 64 128 255.The values are defined in device tree
    }
    else
        write(result,"0", 1);
    close(result);
}
/**
 *     This function is to write value 0/1 to extension IO which control backlight of KEYBOARD
 *     @param[in] operation                value 0/1 (0 mean backlight off) (1 mean backlight on)
 */
void backlight_control_KEYBOARD(UCHAR operation){
  
    EXTIO_GPIO(_KEY_LED_EN_BANK_ADDR,_KEY_LED_EN_GPIO_PORT,_KEY_LED_EN_GPIO_NUM,operation);
    
	
	
    
}


// ---------------------------------------------------------------------------
// FUNCTION: To control DEVICE backlight.
// INPUT   : device   - RFU. (LCD or keypad...)
//                     0 = LCD
//                     1 = KBD
//                     0x80 = LCD backlight level
//           duration - duration of turning on back-light of LCD in 10ms.
//                     if device = 0 or 1
//                             0x00000000 = turn off right away.
//                             0xFFFFFFFF = turn on forever.
//                     if device = 0x80
//                             duration is used for the backlight level.
//                             level: 0..7 (0=turn off, 7=highest)
// OUTPUT  : none.
// RETURN  : apiOK
//          apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_sys_backlight( UCHAR device, ULONG duration )
{
UCHAR	result = apiFailed;

	if( device == 0 )	// 2016-03-03
	{

        if(duration == 0x00000000)
              backlight_control_LCDTFT(0,0);
        else if(duration == 0xFFFFFFFF)
              backlight_control_LCDTFT(1,7);
        else{
        
        }
    
	    //OS_LCD_BackLight( duration );
        
	    result = apiOK;
	}
    if( device == 1 )
    {
        if(duration == 0x00000000)
                backlight_control_KEYBOARD(0);
        else if(duration == 0xFFFFFFFF)
                backlight_control_KEYBOARD(1);
        else{}

        result = apiOK;
	}
    // printf("device=0x%x  duration=%d\n",device,dWuration);
    if( device == 0x80 )
    {
        if(duration>7)
            return apiFailed;
        backlight_control_LCDTFT(1,duration);
        SYS_brightness=duration;
        result = apiOK;
    }
	return( result );
}


// ---------------------------------------------------------------------------
// FUNCTION: To read system related information.
// INPUT   : id   - system information id.
//		    refer to SID_xxx defined in POSAPI.h.
// OUTPUT  : info - information read. [L(1)-V(n)], min. 17 bytes storage.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_sys_info( UCHAR id, UCHAR *info )
{
UINT	i;
UCHAR	result;
UCHAR	ret;
UCHAR	*pInfo;
ULONG	data;
ULONG	cid;
ULONG	sn;
UCHAR	len1, len2;
UCHAR	cid_buf[3];
UCHAR	sn_buf[16];
UCHAR	buffer[32];
UCHAR	buffer2[16];
FILE    *UIDfp=NULL;
UCHAR	*UID1_path="/sys/fsl_otp/HW_OCOTP_CFG0";
UCHAR	*UID2_path="/sys/fsl_otp/HW_OCOTP_CFG1";


UCHAR  fake_SerinalNUmber[] = "0123456789";


	pInfo = info;
	result = apiFailed;
	
	memset( buffer, 0x30, 32 );
	
	switch( id )
	      {
	      case SID_TerminalSerialNumber:	// text format
	      	
		// the contents of CF0 and CF1 are text strings,
		// such as CF0: "0xee6b78b7" (10 bytes)
		//	   CF1: "0x260719d4" (10 bytes)
#if	0   
           UIDfp=fopen(UID1_path,"rb+");
           if(UIDfp==NULL)
           {
               printf("UIDfp open failed\n");
               return result;
           }        
	       ret=fread(buffer,1,10,UIDfp);
           if(ret<10)
           {
               printf("UID read fail ret=%d\n",ret);
               return result;
           }             
	       fclose(UIDfp);
           memmove(sn_buf,&buffer[2],8);//read value will be "0x........" in total 10 byte, so skip "0x" will be 8 byte
           UIDfp=fopen(UID2_path,"rb+");
	       ret=fread(buffer,1,10,UIDfp);
           if(ret<10)
             return result;
	       fclose(UIDfp);
           memmove(&sn_buf[sizeof(sn_buf)/2],&buffer[2],8);//read value will be "0x........" in total 10 byte, so skip "0x" will be 8 byte
		   // format1: LEN=12, CID(3) + CSN(8)
		   buffer[0] = sizeof(sn_buf);	// length of output string

           memmove(&buffer[1],sn_buf, sizeof(sn_buf));


		   memmove( info, buffer, sizeof(sn_buf)+1 );
		   result = apiOK;
		   
#else
	      	   UIDfp=fopen(UID1_path,"rb+");
	      	   if( UIDfp )
	      	     {
	      	     memset( buffer, 0x30, sizeof(buffer) );
	      	     result = apiOK;
	      	     
	      	     ret=fread(buffer,1,10,UIDfp);
	      	     if( ret >= 10 )
	      	       memmove( sn_buf, &buffer[2], 8 );	// ignore "0x"
	      	     else if( ret > 2 )
	      	            memmove( &sn_buf[10-ret], &buffer[2], ret-2 );
	      	          else
	      	            result = apiFailed;
	      	            
	      	     fclose(UIDfp);
	      	     }

		  if( result != apiOK )
		    break;
		    
	      	   UIDfp=fopen(UID2_path,"rb+");
	      	   if( UIDfp )
	      	     {
	      	     memset( buffer, 0x30, sizeof(buffer) );
	      	     result = apiOK;
	      	     
	      	     ret=fread(buffer,1,10,UIDfp);
	      	     if( ret >= 10 )
	      	       memmove( &sn_buf[8], &buffer[2], 8 );	// ignore "0x"
	      	     else if( ret > 2 )
	      	            memmove( &sn_buf[8+(10-ret)], &buffer[2], ret-2 );
	      	          else
	      	            result = apiFailed;
	      	            
	      	     fclose(UIDfp);
	      	     }

		  if( result != apiOK )
		    break;
		    
		  info[0] = 16;
		  memmove( &info[1], sn_buf, 16 );
#endif
	      	   break;

	      case SID_TerminalSerialNumber2:
	      	   

//		   // format2: LEN=8, TSN(8) or CSN(8)
//		   // NOTE   : TSN solution is to be implemented.
//		   buffer[0] = 8;	// length of output string
//		   memmove( &buffer[1+8-len2], sn_buf, len2 );	// CSN(8)
//		   memmove( info, buffer, 9 );

		   // Method 2: TSN of sticker
//		   OS_FLS_GetData( F_ADDR_TSN, TSN_LEN, buffer );
//		   memmove( info, buffer, 9 );
//		   OS_FLS_GetData( F_ADDR_TSN, 9, info );

            buffer[0] = 8;	// length of output string

           memmove(&buffer[1],fake_SerinalNUmber, 8);
		   memmove( info, buffer, 9 );
	       result = apiOK;	   
	      	   break;

	      case SID_ChineseFontVersion:
	      case SID_BIOSversion:
	      case SID_BSPversion:
	      case SID_MODEMversion:
	      case SID_PRINTERversion:
	      case SID_TCPIPversion:
	      case SID_FTPversion:
	      case SID_FTPSversion:
	      case SID_SSLversion:
	      case SID_CRYPTOversion:
	      case SID_POSAPIversion:
	      case SID_USBversion:				
	      case SID_LCDTFTversion:			
	      case SID_TSCversion:				
	      case SID_MEversion:				
	      case SID_ReleaseDateTime:
	      	   break;
	      	   
	      case SID_McuSN:	// binary format

	      	   UIDfp=fopen(UID1_path,"rb+");
	      	   if( UIDfp )
	      	     {
	      	     memset( buffer, 0x00, sizeof(buffer) );
	      	     result = apiOK;
	      	     
	      	     ret=fread(buffer,1,10,UIDfp);
	      	     if( ret >= 10 )
	      	       memmove( sn_buf, &buffer[2], 8 );	// ignore "0x"
	      	     else if( ret > 2 )
	      	            memmove( &sn_buf[10-ret], &buffer[2], ret-2 );
	      	          else
	      	            result = apiFailed;
	      	            
	      	     fclose(UIDfp);
	      	     }

		  if( result != apiOK )
		    break;
		    
	      	   UIDfp=fopen(UID2_path,"rb+");
	      	   if( UIDfp )
	      	     {
	      	     memset( buffer, 0x00, sizeof(buffer) );
	      	     result = apiOK;
	      	     
	      	     ret=fread(buffer,1,10,UIDfp);
	      	     if( ret >= 10 )
	      	       memmove( &sn_buf[8], &buffer[2], 8 );	// ignore "0x"
	      	     else if( ret > 2 )
	      	            memmove( &sn_buf[8+(10-ret)], &buffer[2], ret-2 );
	      	          else
	      	            result = apiFailed;
	      	            
	      	     fclose(UIDfp);
	      	     }

		  if( result != apiOK )
		    break;
		    
	      	   // convert ascii to binary
	      	   info[0] = 8;
	      	   
	      	   for( i=0; i<8; i++ )
	      	      info[1+i] = api_tl_ascw2hexb( &sn_buf[i*2] );
		   
	      	   break;
	      	   
	      default:
	           result = apiFailed;
	      } // switch( id )
EXIT:	      
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC based on OUI & UID. (TEMP SOLUTION)
//		    EE 6B 78 B7
//		    22 49 19 D4 (or 26 07 19 D4), only byte 4 & 5 different.
// INPUT   : none.
// OUTPUT  : mac_b - the MAC in binary (fixed 6 bytes, 12 34 56 78 9A BC).
//	         mac_s - the MAC in string (fixed 17 bytes, "12:23:56:78:9A:BC")
// RETURN  : none.
// ---------------------------------------------------------------------------
void	api_sys_genMAC( UCHAR *mac_b, UCHAR *mac_s )
{
UINT	i;
UCHAR	buf[32];
UCHAR	tmp[6];
UCHAR	byte_h;
UCHAR	byte_l;


	api_sys_info( SID_McuSN, buf );	// L[1] + DATA[8]
	
	mac_b[0] = 0x6C;	// SYMLINK OUI = 6C 15 24 Dx (formal 28 bits)
	mac_b[1] = 0x15;
	mac_b[2] = 0x24;
	mac_b[3] = 0xD0;
	mac_b[3] |= buf[4] & 0x0F;	// using only 3 bytes of UID
	mac_b[4] = buf[5];
	mac_b[5] = buf[6];
	
	for( i=0; i<6; i++ )
	   {
	   api_tl_hexb2ascw( mac_b[i], &byte_h, &byte_l );
	   mac_s[i*3+0] = byte_h;
	   mac_s[i*3+1] = byte_l;
	   mac_s[i*3+2] = ':';
	   }
	mac_s[17] = 0;
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC based on OUI & UID. (TEMP SOLUTION)
//		     EE 6B 78 B7
//		     22 49 19 D4 (or 26 07 19 D4), only byte 4 & 5 different.
// INPUT   : none.
// OUTPUT  : mac_b - the MAC in binary (fixed 6 bytes, 12 34 56 78 9A BC).
//	         mac_s - the MAC in string (fixed 17 bytes, "12:23:56:78:9A:BC")
// RETURN  : none.
// ---------------------------------------------------------------------------
void	api_sys_genMAC_PSEUDO( UCHAR *mac_b, UCHAR *mac_s )
{
UINT	i;
UCHAR	buf[32];
UCHAR	tmp[6];
UCHAR	byte_h;
UCHAR	byte_l;


	api_sys_info( SID_McuSN, buf );	// L[1] + DATA[8]
	
	mac_b[0] = 0x00;	// Pseudo OUI = 00 F0 FF xx xx xx
	mac_b[1] = 0xF0;
	mac_b[2] = 0xFF;
	mac_b[3] = buf[4];
	mac_b[4] = buf[5];
	mac_b[5] = buf[6];
	
	for( i=0; i<6; i++ )
	   {
	   api_tl_hexb2ascw( mac_b[i], &byte_h, &byte_l );
	   mac_s[i*3+0] = byte_h;
	   mac_s[i*3+1] = byte_l;
	   mac_s[i*3+2] = ':';
	   }
	mac_s[17] = 0;
}
