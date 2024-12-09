#include<stdint.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdio.h>
#include<linux/input.h>
#include<stdlib.h>
#include <string.h>
#include "POSAPI.h"
#include "OS_PROCS.h"

#if	0	// ==>
UCHAR	Buzz_apiBuf[10][6];
UCHAR	Buzz_dhn=0;
UCHAR	Input_EventName[50];//get input event description,to check if open event is right event
const UCHAR	Buzz_EventName[]="pwm-beeper";



int Buzz_fd=0;
static UCHAR BUZ_Check_dhn(UCHAR dhn)
{
	return ((dhn&psDEV_BUZ)>0);
} 
UINT api_buz_open( UCHAR *sbuf )
{
	if(Buzz_fd==0){
		Buzz_fd = open("/dev/input/event3", O_RDWR);
	}
	ioctl(Buzz_fd, EVIOCGNAME(sizeof(Input_EventName)), Input_EventName);
	//check whether opened event are beeper event.If not,close event3 and open event4
	if(strcmp(Buzz_EventName,Input_EventName)!=0)
	{
		// printf("!!!!!!!!!!%s\n",Input_EventName);
		close(Buzz_fd);
		Buzz_fd = open("/dev/input/event4", O_RDWR);
	}
	//find which buffer are empty
	for(UCHAR i=0;i<10;i++)
	{
		if(Buzz_apiBuf[i][0]<1)
		{
			Buzz_dhn=i;
			break;
		}
		if(i==9)
			Buzz_dhn=10;
	}
	if (Buzz_fd < 0||Buzz_dhn==10) 
        return apiOutOfService;
	
	 memmove(Buzz_apiBuf[Buzz_dhn],sbuf,sizeof(6));
	 
	return(Buzz_dhn+psDEV_BUZ);
}
//
UCHAR api_buz_sound( UINT16 dhn ){
int ret=0;
UINT16 	FreqInHz;
UCHAR	Buzz_autoclose=0; 
	struct input_event event;
	if (!BUZ_Check_dhn(dhn)) { 
        return apiFailed;
    }	
	dhn-=psDEV_BUZ;
	event.type = EV_SND;
    event.code = SND_TONE;	
	FreqInHz=Buzz_apiBuf[dhn][3]+Buzz_apiBuf[dhn][4]*256;
	 if(FreqInHz==0){
		 FreqInHz=2500;
	 }	
	Buzz_autoclose=Buzz_apiBuf[dhn][5];
	for(int i=0;i<Buzz_apiBuf[dhn][0];i++)
	{
		event.value = FreqInHz;
		ret = write(Buzz_fd, &event, sizeof(struct input_event));
		usleep(Buzz_apiBuf[dhn][1]*10000);
		event.value = 0;
		ret = write(Buzz_fd, &event, sizeof(struct input_event));
		usleep(Buzz_apiBuf[dhn][2]*10000);	
	}
	if(Buzz_autoclose==0x5A)
	{
		if (close(Buzz_fd) < 0) {
			return apiFailed;
		}
		memset(Buzz_apiBuf[dhn],0x00,sizeof(6));
	}
	return apiOK;
}

UCHAR api_buz_close(UINT16 dhn){
	if (!BUZ_Check_dhn(dhn)) { 
        return apiFailed;
    }	
	dhn-=psDEV_BUZ;
	memset(Buzz_apiBuf[dhn],0x00,sizeof(6));
	return( apiOK );
}
#endif	// <==
