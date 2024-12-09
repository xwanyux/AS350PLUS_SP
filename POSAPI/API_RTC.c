//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : MAX32550 (32-bit Platform)				    **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : API_RTC.C                                                  **
//**  MODULE   : api_rtc_open()				                    **
//**		 api_rtc_close()					    **
//**		 api_rtc_getdatetime()					    **
//**		 api_rtc_setdatetime()					    **
//**		 api_rtc_getUnixTime()					    **
//**									    **
//**  FUNCTION : API::RTC (Real Time Clock Module)			    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/07/14                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "POSAPI.h"
#include "OS_PROCS.h"
#include <stdio.h>   
#include <time.h>
#include <linux/rtc.h>   
#include <sys/ioctl.h>   
#include <sys/time.h>   
#include <sys/types.h>   
#include <fcntl.h>   
#include <unistd.h>   
#include <errno.h> 



// this is a example of using linux RTC
#if 0
// side note: once you want to set the date time of RTC 
//	you need to make sure your struct rtc_time attribute to be set to the right range
//  the easy way to avid the poor attribute value , just call get ioctl(fd, RTC_RD_TIME, &rtc_tm);
//  use this function to initalize your variable, can avoid some trouble 
void test(){
int fd, retval,i;   
struct rtc_time rtc_tm;   
   
fd = open ("/dev/rtc", O_RDONLY);   
if (fd ==  -1) {   
    perror("/dev/rtc");   
    exit(errno);   
}   
fprintf(stderr, "\tRTC Driver Test Example.\n\n");   
   
/* Read the RTC time/date */   
retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);   
if (retval == -1) {   
    perror("ioctl");   
    exit(errno);   
}   
fprintf(stderr, "Current RTC date/time is %02d-%02d-%d, %02d:%02d:%02d.\n\n",   
    rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,   
    rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);   
   
//SET RTC date/time    
       
fprintf(stderr,"\t**** NOW SET TIME YOU WANT!!****\n\t****format: day month year hour min sec****\n");   
scanf("%02d%02d%04d%02d%02d%02d",&rtc_tm.tm_mday,&rtc_tm.tm_mon,&rtc_tm.tm_year,&rtc_tm.tm_hour,&rtc_tm.tm_min,&rtc_tm.tm_sec);   
rtc_tm.tm_mon-=1;   
rtc_tm.tm_year-=1900;   
   
retval = ioctl(fd, RTC_SET_TIME, &rtc_tm);   
if (retval == -1) {   
    perror("ioctl");   
    exit(errno);   
}   
//Read back just wrote TIME   
   
retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);   
if (retval == -1) {   
    perror("ioctl");   
    exit(errno);   
}   
fprintf(stderr, "RTC date/time SET is %02d-%02d-%d, %02d:%02d:%02d.\n\n",   
    rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,   
    rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);   
   
//delay one sec   
   
for(i=0;i<20000000;i++)   
{}   
retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);   
if (retval == -1) {   
    perror("ioctl");   
    exit(errno);   
}   
fprintf(stderr, "Current RTC date/time is %02d-%02d-%d, %02d:%02d:%02d.\n\n",   
    rtc_tm.tm_mday, rtc_tm.tm_mon + 1, rtc_tm.tm_year + 1900,   
    rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec);     
fprintf(stderr, "\t *** Test complete ***\n");   
close(fd);   
}

#endif

/**
 * 	This function is used to covert value modulus 100 to char array
 * 	@param[in] value	the value be converted
 *  @param[out]output	converted array (size should be at least 2)
 */
void ConvertInt2Char(int value, char* output){
	if(value > 100)
		value = value % 100;

	output[0] = value / 10 + '0';
	output[1] = value % 10 + '0';
}

/**
 * 	This function is used to convert string day to integer (e.g "31" -> 31)
 *  @param[in] input	the day string
 *  @return  the convertd integer day value
 */
int ConvertDay2Int(char* input){

	int temp = 0;

	temp += (int)(input[0] - '0') * 10;
	temp += (int)(input[1] - '0');
	return temp;

}



UCHAR	os_DHN_RTC = 0;
// value for rtc driver
static int fd; 

// ---------------------------------------------------------------------------
// FUNCTION: To check if DHN matched.
// INPUT   : dhn
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UCHAR RTC_CheckDHN( UCHAR dhn )
{
	if( (dhn == 0) || (dhn == os_DHN_RTC) )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: To enable the service of real time clock device.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : DeviceHandleNumber
//           apiOutOfService
// ---------------------------------------------------------------------------
UCHAR api_rtc_open( void )
{
	if( os_DHN_RTC != 0 )	// already opened?
	  return( apiOutOfService );

	// open linux driver
	fd = open("/dev/rtc",O_RDONLY);
	if (fd == -1) {
		return( apiOutOfService );
	}
	else
	{  
		close(fd);
		os_DHN_RTC = psDEV_RTC + 0x80;
		return( os_DHN_RTC );
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: To disable the service of real time clock device.
// INPUT   : dhn
//	     The specified device handle number.
//	     0x00 = to close all opened tasks.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR api_rtc_close( UCHAR dhn )
{
	if( RTC_CheckDHN( dhn ) == TRUE )
	  {
	  os_DHN_RTC = 0;
	  fd=0;
	  return( apiOK );
	  }
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To read the current date and time from RTC device.
// INPUT   : dhn
//	     The specified device handle number. 
// OUTPUT  : dbuf
//	     UCHAR  year[2] ;		// "00", "01", "02",..."99"
//	     UCHAR  month[2] ;        	// "01", "02", "03",..."12"
//	     UCHAR  day[2] ;            // "01", "02", "03",..."31"
//	     UCHAR  hour[2] ;           // "00", "01", "02",..."23"
//	     UCHAR  minute[2] ;        	// "00", "01", "02",..."59"
//	     UCHAR  second[2] ;       	// "00", "01", "02",..."59"
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
/*
UCHAR api_rtc_getdatetime( UCHAR dhn, UCHAR *dbuf )
{

	if( RTC_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );

	
	time_t nSeconds;
    struct tm * rtc_tm;
	time(&nSeconds);
	rtc_tm = localtime(&nSeconds);
	// printf("%02d/%02d/%02d\n", rtc_tm->tm_year + 1900,  rtc_tm->tm_mon + 1,  rtc_tm->tm_mday);
	// printf("%02d:%02d:%02d\n", rtc_tm->tm_hour,  rtc_tm->tm_min,  rtc_tm->tm_sec);
		ConvertInt2Char(rtc_tm->tm_year+1900,dbuf);
		dbuf += 2;
		ConvertInt2Char(rtc_tm->tm_mon + 1,dbuf);
		dbuf += 2;
		ConvertInt2Char(rtc_tm->tm_mday,dbuf);
		dbuf += 2;
		ConvertInt2Char( rtc_tm->tm_hour,dbuf);
		dbuf += 2;
		ConvertInt2Char( rtc_tm->tm_min,dbuf);
		dbuf += 2;
		ConvertInt2Char( rtc_tm->tm_sec,dbuf);
		
		return apiOK;

	
	
	// if( OS_RTC_GetDateTime( dbuf ) )
	//   return( apiOK );
	// else
	//   return( apiFailed );
}
*/

//Wayne version. Abandoned for not concerning localtime(GMT+xx)
UCHAR api_rtc_getdatetime( UCHAR dhn, UCHAR *dbuf )
{

	if( RTC_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );

	
	struct rtc_time rtc_tm; 
	int response;

	// open linux driver
	fd = open("/dev/rtc",O_RDONLY);
	if (fd == -1) {
		return( apiFailed );
	}
	
	// fd is the RTC device number, action is RTC read time, value save to rtc_tm
	response = ioctl(fd,RTC_RD_TIME,&rtc_tm);
	close(fd);
	if(response == -1)
		return apiFailed;		
	else{

		ConvertInt2Char(rtc_tm.tm_year,dbuf);
		dbuf += 2;
		ConvertInt2Char(rtc_tm.tm_mon + 1,dbuf);
		dbuf += 2;
		ConvertInt2Char(rtc_tm.tm_mday,dbuf);
		dbuf += 2;
		ConvertInt2Char( rtc_tm.tm_hour,dbuf);
		dbuf += 2;
		ConvertInt2Char( rtc_tm.tm_min,dbuf);
		dbuf += 2;
		ConvertInt2Char( rtc_tm.tm_sec,dbuf);
		return apiOK;
	}

	
	
	if( OS_RTC_GetDateTime( dbuf ) )
	  return( apiOK );
	else
	  return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To assign the new date and time to RTC device.
// INPUT   : dhn
//	     The specified device handle number.
// 	     sbuf
//	     UCHAR  year[2] ;		// "00", "01", "02",..."99"
//	     UCHAR  month[2] ;        	// "01", "02", "03",..."12"
//	     UCHAR  day[2] ;            // "01", "02", "03",..."31"
//	     UCHAR  hour[2] ;           // "00", "01", "02",..."23"
//	     UCHAR  minute[2] ;        	// "00", "01", "02",..."59"
//	     UCHAR  second[2] ;       	// "00", "01", "02",..."59"
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR api_rtc_setdatetime( UCHAR dhn, UCHAR *sbuf )
{
UCHAR syncRTC[]="hwclock -s";//overwrite local time with RTC time. If have internet it will do nothing. 
FILE *fp;
	if( RTC_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );

	struct rtc_time rtc_tm; 
	int response;
	fd = open("/dev/rtc",O_RDONLY);
	if (fd == -1) {
		perror("open rtc fail");
		return( apiFailed );
	}
	// this is just to avoid some wrong initalize value for rtc_tm 
	ioctl(fd,RTC_RD_TIME,&rtc_tm);
	
	rtc_tm.tm_year =  ConvertDay2Int(sbuf) + 100; // assum input is after 2000
	sbuf += 2;
	rtc_tm.tm_mon = ConvertDay2Int(sbuf) - 1;
	sbuf += 2;
	rtc_tm.tm_mday = ConvertDay2Int(sbuf);
	sbuf += 2;
	rtc_tm.tm_hour = ConvertDay2Int(sbuf);
	sbuf += 2;
	rtc_tm.tm_min  = ConvertDay2Int(sbuf);
	sbuf += 2;
	rtc_tm.tm_sec = ConvertDay2Int(sbuf);


	response = ioctl(fd, RTC_SET_TIME, &rtc_tm);
	close(fd);
	if(response == -1){
		return ( apiFailed );
	}		
	fp = popen(syncRTC, "r");
	pclose(fp);
	return( apiOK );
}
// ---------------------------------------------------------------------------
// FUNCTION: To set system local time zone.
// INPUT   : dhn
//	     The specified device handle number.
//			 data
//		 Time zone value.More information refer to DEV_RTC.h
// OUTPUT  : none
// RETURN  : apiOK		Time zone set successful
//           apiFailed  Fail to set time zone
// ---------------------------------------------------------------------------
UCHAR api_rtc_settimezone(UCHAR dhn, UCHAR data)
{
UCHAR timecmdP[]="timedatectl set-timezone Etc/GMT+";
UCHAR timecmdM[]="timedatectl set-timezone Etc/GMT-";
UCHAR syncRTC[]="hwclock -w";
UCHAR cmd[40];
UCHAR dec[]="1";
UCHAR len;
UCHAR tmp;
FILE *fp;
	if(data>0x0c)
		if((data&0x80==0)||(data>0x8c))
			return apiFailed;
	memset(cmd,0,40);
	if(data&0x80>0)//GMT+
	{
		len=strlen(timecmdP);
		memmove(cmd,timecmdP,len);//timedatectl set-timezone Etc/GMT+
		tmp=data&(~0x80);
		tmp+=48;
		if(tmp>9+48)
		{
			tmp-=10;
			memmove(&cmd[len++],dec,1);//timedatectl set-timezone Etc/GMT+1
			memmove(&cmd[len++],&tmp,1);//timedatectl set-timezone Etc/GMT+1x
		}
		else
			memmove(&cmd[len++],&tmp,1);//timedatectl set-timezone Etc/GMT+x
	}
	else if(data==0)
	{
		len=strlen(timecmdM);
		memmove(cmd,timecmdM,len);//timedatectl set-timezone Etc/GMT-
		tmp=0+48;//ASCII 0
		memmove(&cmd[len++],&tmp,1);//timedatectl set-timezone Etc/GMT-0
	}
	else//GMT-
	{
		len=strlen(timecmdM);
		memmove(cmd,timecmdM,len);//timedatectl set-timezone Etc/GMT-
		tmp=data+48;
		if(tmp>9+48)
		{
			tmp-=10;
			memmove(&cmd[len++],dec,1);//timedatectl set-timezone Etc/GMT-1
			memmove(&cmd[len++],&tmp,1);//timedatectl set-timezone Etc/GMT-1x
		}
		else
			memmove(&cmd[len++],&tmp,1);//timedatectl set-timezone Etc/GMT-x

	}
	fp = popen(cmd, "r");
	pclose(fp);
	fp = popen(syncRTC, "r");
	pclose(fp);
	return apiOK;
}

// ---------------------------------------------------------------------------
// FUNCTION: To get system local time zone.
// INPUT   : dhn
//	     The specified device handle number.
// OUTPUT  : data
//		 Time zone value.More information refer to DEV_RTC.h
// RETURN  : apiOK		Get time zone value successfully
//           apiFailed  Fail to get time zone value
// ---------------------------------------------------------------------------
UCHAR api_rtc_gettimezone(UCHAR dhn, UCHAR *data)
{
UCHAR cmd_rdln[]="readlink /etc/localtime";
UCHAR syncRTC[]="hwclock -w";
UCHAR response[40];
UCHAR len;
UCHAR cursor;
UCHAR result=0;
FILE *fp;
	fp = popen(syncRTC, "r");
	pclose(fp);
	memset(response,0,40);
	fp = popen(cmd_rdln, "r");
    fgets(response, 40, fp);
    pclose(fp);

	len=strlen(response);
	for(cursor=len;cursor>=0;cursor--)
		if(response[cursor]==0x54)//"T"
			break;
	if(response[cursor+1]=='+')//GMT+
	{
		result|=0x80;
		if((len-cursor)==5)//GMT+1x/r/n
			result|=response[cursor+3]-48+10;//-ASCII '0' + 0x0A
		else//GMT+x
			result|=response[cursor+2]-48;
	}
	else//GMT-
	{
		if((len-cursor)==5)//GMT-1x/r/n
			result|=response[cursor+3]-48+10;//-ASCII '0' + 0x0A
		else//GMT-x
			result|=response[cursor+2]-48;
	}
	*data=result;
	return apiOK;
}
// ---------------------------------------------------------------------------
// FUNCTION: Set if system time syncronize with NTP server automatically.
// INPUT   : dhn
//	     		The specified device handle number.
//			 action
//				1 for system enable syncronize with NTP server.This will change RTC time and UTC time automatically
//				0 for system disable syncronize with NTP server.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR api_rtc_sync_NTP_switch( UCHAR dhn, UCHAR action )
{
UCHAR cmd_setntp[]="timedatectl set-ntp ";
UCHAR switchON[]="yes";
UCHAR switchOFF[]="no";
UCHAR buffer[24];
UCHAR result=0;
FILE *fp;
	memset(buffer,0,sizeof(buffer));
	memmove(buffer,cmd_setntp,strlen(cmd_setntp));
	if(action)
		memmove(&buffer[strlen(cmd_setntp)],switchON,strlen(switchON));
	else
		memmove(&buffer[strlen(cmd_setntp)],switchOFF,strlen(switchOFF));
	fp = popen(buffer, "r");
	pclose(fp);
	return apiOK;
}

// ---------------------------------------------------------------------------
// FUNCTION: To read the current date and time in UNIX time format.
// INPUT   : dhn
//	     The specified device handle number.
// OUTPUT  : dbuf
//		unix time in seconds, 4 bytes, little-endia (since 1970)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
// UCHAR	api_rtc_getUnixTime( UCHAR dhn, UCHAR *dbuf )
// {
// UCHAR	result;
// UCHAR	rtc[16];
// ULONG	utc_sec;


// 	result = api_rtc_getdatetime( dhn, rtc );
// 	if( result == apiOK )
// 	  {
// 	  utc_sec = OS_RTC_ToUnixTime( rtc );
	  
// 	  dbuf[0] = utc_sec & 0x000000ff;
// 	  dbuf[1] = (utc_sec & 0x0000ff00) >> 8;
// 	  dbuf[2] = (utc_sec & 0x00ff0000) >> 16;
// 	  dbuf[3] = (utc_sec & 0xff000000) >> 24;
// 	  }
	  
// 	return( result );
// }






