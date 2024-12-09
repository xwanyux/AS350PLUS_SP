#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "POSAPI.h"

UINT16 os_DHN_KBD = 0;
UCHAR AP_KeyRetnCodeTable[]	=	{
						 'a', 'b', 'c', 'd',		// F1, F2, F3, F4
						 '1', '2', '3', 'x',		// 1 , 2 , 3 , CANCEL
						 '4', '5', '6', 'n',		// 4 , 5 , 6 , CLEAR
						 '7', '8', '9', 'z',		// 7 , 8 , 9 , ALPHA
						 '*', '0', '#', 'y'			// * , 0 , # , ENTER
						};
int _KeyRetnCodeTable[]	=	{
						 'a', 'b', 'c', 'd',		// F1, F2, F3, F4
						  2 ,  3 ,  4 , 111,		// 1 , 2 , 3 , CANCEL
						  5 ,  6 ,  7 , 14 ,		// 4 , 5 , 6 , CLEAR
						  8 ,  9 ,  10, 'z',		// 7 , 8 , 9 , ALPHA
						  55,  11, 105, 28			// * , 0 , # , ENTER
						};
UCHAR KBD_KAT_SIZE=sizeof(AP_KeyRetnCodeTable);
UCHAR KBD_Kat[5]={0};
UCHAR KBD_ByteNo;
UCHAR KBD_BitNo;
UCHAR KBD_InputKey=0;
struct input_event KBDevent={{0},0};
struct timeval timeout;
fd_set KBD_set;
int KBD_fd=0,rv=0;
UCHAR KBD_CheckDHN( UINT16 dhn )
{
	if( (dhn > 0) && (dhn == os_DHN_KBD) )
	  return( TRUE );//true
	else
	  return( FALSE );//false
}

UCHAR api_kbd_open( UCHAR deviceid, UCHAR *sbuf )
{	
	
	if(KBD_fd==0){
		KBD_fd = open("/dev/input/event0", O_RDWR);
		if(KBD_fd==-1)
			return( apiOutOfService );
	
	}
	
	memmove( KBD_Kat, sbuf, sizeof(KBD_Kat) ); // setup KAT	
		
	os_DHN_KBD=KBD_fd;
	
	// KBD_InputKey = 0;
	
	return( os_DHN_KBD );
}

UCHAR api_kbd_close( UINT16 dhn )
{	
	if( KBD_CheckDHN( dhn ) == TRUE )
	  {
	  for(UCHAR i=0;i<5;i++){//reset KBD_Kat
		  KBD_Kat[i]=0;
	  }
	  os_DHN_KBD = 0;
	  KBD_fd=0;	  
	  return( apiOK );//apiOK
	  }
	else
	  return( apiFailed );//apiFailed
}
UINT32 KBD_Status(UCHAR *ScanCode){
UCHAR sc;
UINT16 i=0;
START:
	FD_ZERO(&KBD_set); /* clear the set */
	FD_SET(KBD_fd, &KBD_set); /* add our file descriptor to the set */
	timeout.tv_sec = 0;
	timeout.tv_usec = 5000;//5ms timeout
	rv = select(KBD_fd+1, &KBD_set, NULL, NULL, &timeout);
	
	if (FD_ISSET(KBD_fd, &KBD_set))
        {
		  read(KBD_fd,&KBDevent, sizeof(KBDevent));//read Keypad Status
		  if(KBDevent.type!= 1)
			  goto START;
		}
	if(rv == -1 || rv == 0 ){//select() timeout?????ï¿½error
		return(FALSE);		
	}
	// if(read(KBD_fd,&KBDevent, sizeof(KBDevent))>0)
	else
	{//ï¿?????select()ï¿??timeout????????ï¿½NULL
		// printf("KBDevent.type=%d\n",KBDevent.type);
		// printf("KBDevent.value=%d\n",KBDevent.value);
		if((KBDevent.type== 1) && (KBDevent.value==1)){//??ï¿½ï¿½??EvKeyï¿?????????????ï¿½ï¿½??
		// printf("KBDevent.code=%d\n",KBDevent.code);
			sc= KBDevent.code;
			for(;i<20;i++){			
			if(sc==_KeyRetnCodeTable[i]){
				sc=AP_KeyRetnCodeTable[i];
				break;
				}
			}
			KBD_ByteNo=i%4;
			KBD_BitNo=i/4;
			if((KBD_Kat[KBD_ByteNo]>>(KBD_BitNo+1))&0x01)//if input key are enabled.(match AP input sbuf[])	
				*ScanCode= sc;
			else
				return(FALSE);
			
			return(TRUE);
		}
		return(FALSE);
	}
		
}

UCHAR api_kbd_status( UINT16 dhn, UCHAR *dbuf )
{
	UCHAR sc ;
	UINT16 i=0;
	if( KBD_CheckDHN( dhn ) == TRUE )
	  {
		
	  if( KBD_Status(&sc)== TRUE )
	    {
	    // convert scan code to return code for AP use		
			//dbuf = &sc;
			*dbuf = sc;
			KBD_InputKey=*dbuf;
			
	    return( apiReady );//key depressed
	    }
	  if(KBD_InputKey!=0)
	  {
		*dbuf=KBD_InputKey;
		
	    return( apiReady );//no key depressed
	  }
	  
	  return( apiNotReady );//no key depressed
	  }
	else
	  return( apiFailed );//device error
}

UCHAR api_kbd_getchar( UINT16 dhn, UCHAR *dbuf )
{
	UCHAR sc;
	if( KBD_CheckDHN( dhn ) == TRUE )
	  {
		while(KBD_Status(&sc)!=TRUE)
		{
		  if(KBD_InputKey!=0)
		  {
			//   printf("222222222KBD_InputKey=%x\n",KBD_InputKey);
			  *dbuf = KBD_InputKey; // return code	  
				KBD_InputKey=0;
				return( apiOK );//apiOK
		  }				  
		}
		// printf("KBD_InputKey=0x%x\n",KBD_InputKey);
		*dbuf = sc; // return code	  
		KBD_InputKey=0;
		return( apiOK );//apiOK
	  }
	else
	  return( apiFailed );//apiFailed
}



/**
 * 	this function is used to get the multiple key char.
 *  @param[in] dhn				device handle number for keyboard
 *  @param[out] dbuf			output data
 * 								dbuf[0] will be the length of keyboad be press
 * 								dbuf[1-4] will be the charater of the key 
 * 								dbuf should be length 5
 * @return error code
 * @retval apiFailed 			device not open or not get any key value
 * @retval apiOK				get at least one key value
 */ 
UCHAR api_kbd_get_multiple_char( UINT16 dhn, UCHAR *dbuf){

UINT dhn_timer;
UINT value;
struct input_event KBDevent={{0},0};
UCHAR sc;
int i, idx,j;
UCHAR key_value[5] = {0};
int num_appear_key = 0;
	if(KBD_CheckDHN( dhn ) == FALSE)
		return apiFailed;
    
	dhn_timer = api_tim_open(10);

	
	
	
	
	// read the keyboad event in 100 ms
	while(1){
		if(api_tim_gettick( dhn_timer,(UCHAR*)&value) == apiOK)
			break;
		FD_ZERO(&KBD_set); /* clear the set */
		FD_SET(KBD_fd, &KBD_set); /* add our file descriptor to the set */
		timeout.tv_sec = 0;
		timeout.tv_usec = 5000;//5ms timeout
		rv = select(KBD_fd+1, &KBD_set, NULL, NULL, &timeout);
		if (FD_ISSET(KBD_fd, &KBD_set))
        {
		  read(KBD_fd, &KBDevent, sizeof(KBDevent));//read Keypad Status
		}
		if(rv == -1 || rv == 0 ){//select() timeout?????ï¿½error
			continue;		
		}
		else{

			/**
			 * 	this is work as keep throwing key pad event 0 and 1
			 */ 
			# if 0
			if(KBDevent.type == 1){
				sc = KBDevent.code;
				idx = -1;
				// find this key exit or not
				for(i = 0; i < 5; i++){
					if(key_value[i] == sc){
						idx = i;
						break;
					}
				}
				if(idx == -1 && num_appear_key < 5)
					key_value[num_appear_key++] = sc;
				else
					count_apper[idx]+= 1;

			}
			#else
			/**
			 * 	we use KBDevent.type = 4 (EV_MSC)
			 * 		   KBDevent.code = 4 (MSC_SCAN)
			 *         KBDevent.value = key_code 
			 *  to store the state of key being pressing
			 */ 

			if(KBDevent.type == 4 && KBDevent.code == 4){

				sc = KBDevent.value;
				idx = -1;
				// 	// find this key exit or not
				for(i = 0; i < 5; i++){
					if(key_value[i] == sc){
						idx = i;
						break;
					}
				}

				if(idx == -1 && num_appear_key < 5)
					key_value[num_appear_key++] = sc;
			}
			#endif

		}

		
	}

	api_tim_close( dhn_timer);

	// retrieve the key result
	if(num_appear_key == 0)
		return apiFailed;

	dbuf[0] = num_appear_key;
	for(i = 0; i < num_appear_key; i++){
		// get the mapping
		for(j = 0; j < KBD_KAT_SIZE; j++){
			if(key_value[i] == _KeyRetnCodeTable[j])
				break;
		}
		// the first byte is length
		dbuf[i + 1] = AP_KeyRetnCodeTable[j];

	}

	return apiOK;


}

