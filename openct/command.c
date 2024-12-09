



#include"ccid.h"
#include "mxc_sim_interface.h"
#include "proto-t1.h"
#include <stdio.h>

#include <sys/ioctl.h> 

//#include "POSAPI.h"
#include <string.h>



#define DEBUG 0  // this is not a good way to write (you can't control the debug by make file)

#define ERROR -1

extern int global_sim_fd; // this is use in test 
extern int sim_driver_fd; // this is use in POSAPI

unsigned int	g_sam_flag = 0;		// 1=to output processing time for tx & rx
extern	unsigned long	os_SysTimerFreeCnt;

extern	void Init_T0_WWT_EX( UCHAR len );	// defined in API_IFM.c


_ccid_descriptor serialDevice = {
    .readTimeout = 3

};


int isCharLevel(int reader_index)
{
	return 0;
} /* isCharLevel */

/*****************************************************************************
 *
 *					CCID_Transmit
 *
 ****************************************************************************/
int CCID_Transmit(unsigned int reader_index, unsigned int tx_length,
	 unsigned char tx_buffer[], unsigned short rx_length, unsigned char bBWI)
{

    //printf("inside CCID Transmit\n");
    sim_xmt_t tx;

    int errval = 0;

    tx.xmt_buffer = tx_buffer;
    tx.xmt_length = tx_length;
    tx.timeout = 300;
    tx.errval = 0;
	//errval = ioctl(global_sim_fd, SIM_IOCTL_XMT, &tx);
    errval = ioctl(sim_driver_fd, SIM_IOCTL_XMT, &tx);
    #if DEBUG
        printf("trans len:%d\n",tx_length);
        printf("data(tx):");
        int i;
        for(i = 0; i < tx_length;i++)
            printf("0x%x ",tx_buffer[i]);
        printf("\n");

        printf("error(tx):%d\n",tx.errval);
    #endif

    if(errval != 0)
        return errval;

    return IFD_SUCCESS;
} /* CCID_Transmit */


int CCID_Receive(unsigned int reader_index, unsigned int *rx_length,
	unsigned char rx_buffer[], unsigned char *chain_parameter)
{
    // if(alternate_wait_time_flag == 0){
    //     usleep(200*150);
    //     alternate_wait_time_flag = 1;
    // }
    // else{
    //     usleep(200*20);
    //     alternate_wait_time_flag = 0;
    // }
    //printf("inside CCID Receive\n");
    //printf("rcv_len(input)%d\n",*rx_length);
    int errval = 0;
    sim_rcv_t receive;
    receive.rcv_buffer = rx_buffer;
	receive.rcv_length=*rx_length;
	receive.timeout = 500;
	//errval = ioctl(global_sim_fd, SIM_IOCTL_RCV, &receive);
    errval = ioctl(sim_driver_fd, SIM_IOCTL_RCV, &receive);

    if(errval != 0)
       return errval;

    *rx_length = receive.rcv_length;
    int i;
    #if DEBUG
        printf("rcv len(output):%d\n",receive.rcv_length);
        printf("data(rx):");
        for(i = 0; i < receive.rcv_length;i++)
            printf("0x%x ",receive.rcv_buffer[i]);
        printf("\n");

        printf("error(rx):%d\n",receive.errval);
    #endif

    //if(receive.errval == SIM_ERROR_NACK_THRESHOLD)
    //    return -SIM_ERROR_NACK_THRESHOLD;

    // reduce some error message (we assum the first byte we receive are always 1)
    // int idx,i;
    // if(receive.rcv_buffer[0] != 0x00){
    //     for(i = 1; i < receive.rcv_length; i++)
    //         if(receive.rcv_buffer[i] == 0x00)
    //             break;
    //     // rewrite the len
    //     idx = i;
    //     receive.rcv_length -= idx;
    //     for(i = 0; i < receive.rcv_length; i++)
    //         receive.rcv_buffer[i] = receive.rcv_buffer[i+idx];
    
    // }

    // if(receive.rcv_length < 4)
    //     return IFD_PARITY_ERROR;
    // if(receive.rcv_buffer[2] + 4 != receive.rcv_length)
    //     return IFD_PARITY_ERROR; 
    return IFD_SUCCESS;
}



int DEBUG_INFO2(char* str, int data){
    #if DEBUG
        printf("DEBUG_INFO2:%s\n",str);
    #endif

    return 0;
}

int DEBUG_CRITICAL(char* str){
    #if DEBUG
        printf("DEBUG_CRITICAL:%s\n",str);
    #endif
    return 0;
}

int DEBUG_COMM(char* str){
    #if DEBUG
        printf("DEBUG_COMM:%s\n",str);
    #endif
    return 0;
}


int DEBUG_COMM4(char* str, int a, int b, int c){
    #if DEBUG
        printf("DEBUG_COMM4:%s\n",str);
    #endif
    return 0;
}

int DEBUG_CRITICAL2(char* str, int a){
    #if DEBUG
        printf("DEBUG_CRITICAL2:%s\n",str);
    #endif
    return 0;
}

int DEBUG_XXD(char* str, unsigned char *block, int len){
    #if DEBUG
        int i;
        printf("DEBUG_XXD:%s,len:%d\n",str,len);
        for(i = 0; i < len; i++)
            printf("0x%x ",block[i]);
        printf("\n");
    #endif
    return 0;
}

int DEBUG_COMM2(char* str, int a){
    #if DEBUG
        char buf[100] = {0};
        sprintf(buf,str,a);

        printf("DEBUG_COMM2:%s\n",buf);
    #endif
    return 0;
}

_ccid_descriptor *get_ccid_descriptor(unsigned int reader_index)
{
	return &serialDevice;
} /* get_ccid_descriptor */


#define CASE_1 1
#define CASE_2 2
#define CASE_3 3
#define CASE_4 4


UCHAR  IsProcedureByte(UCHAR value, UCHAR INS){

    if( (value == INS) || (value == (INS ^ 0xFF)) || (value == 0x60)
        || (value == 0x61) || (value == 0x6c) )
            return TRUE;
    
    return FALSE;

}

UCHAR IsStatusFirstByte(UCHAR value){
    if((value == 0x60) || (value == 0x61) || (value == 0x6c))
        return FALSE;
    else if( ((value & 0xF0) == 0x60) || ((value & 0xF0) == 0x90) )
        return TRUE;
    else
        return FALSE;
}


UCHAR   T0CmdParsing(unsigned char * cmd, unsigned int cmd_len){
    if(cmd_len == 4)
        return CASE_1;
    
    if(cmd_len == 5)
        return CASE_2;

    if(cmd_len == cmd[4] + 5)
        return CASE_3;
    
    if(cmd_len == cmd[4] + 6)
        return CASE_4;

}

// assum cmd size always 5, 
void  command_GetResponse(UCHAR* cmd, UCHAR max_len_data){

    cmd[0] = 0x00;
    cmd[1] = 0xc0;
    cmd[2] = 0x00;
    cmd[3] = 0x00;
    cmd[4] = max_len_data;
    
    Init_T0_WWT_EX( max_len_data );	// 2023-11-24
}

// all of the process is the tx Rx pair
// return value = -1 , mean error
// return value > 0 mean number of receive data
int   Tx_Rx_Pair(UCHAR* send_buf, unsigned int snd_len, UCHAR *rcv_buf, unsigned int rcv_len){

    int result;

    int retry = 3;

    unsigned int rec_len_temp = rcv_len;
    do{
    	if( g_sam_flag )
    	  printf("\r\nCCID_Transmit() Start: %x\r", os_SysTimerFreeCnt );
        result = CCID_Transmit(0,snd_len,send_buf,0,0);
    	if( g_sam_flag )
    	  {
    	  printf("\r\nCCID_Transmit() End: %x\r\n", os_SysTimerFreeCnt );
    	  printf("\rCCID_Transmit() result=%x \r\n", result );
    	  }

    	if( g_sam_flag )
    	  printf("\r\nCCID_Receive() Start: %x\r", os_SysTimerFreeCnt );
        result = CCID_Receive(0,&rec_len_temp, rcv_buf, NULL);
    	if( g_sam_flag )
    	  {
    	  printf("\r\nCCID_Receive() End: %x\r\n", os_SysTimerFreeCnt );
    	  printf("\rCCID_Receive() result=%x \r\n", result );
    	  }
        retry -= 1;
    }
    while(result == -SIM_ERROR_NACK_THRESHOLD && retry > 0);

    if(result < 0)
        return result;
    
    return (int)rec_len_temp;

}

// no data , no response
int case1_TPDU(UCHAR* snd_buf, unsigned int snd_len, UCHAR *rcv_buf, unsigned int rcv_len){

    UCHAR cmd[5] = {0};
    int middle_len;

    memmove(cmd, snd_buf, snd_len);

    // result = CCID_Transmit(0,5,cmd,0,0);

    // // no peocedure byte only status byte
    // result = CCID_Receive(0,&rcv_len,rcv_buf, NULL);

    middle_len = Tx_Rx_Pair(cmd,5,rcv_buf,rcv_len);
    
//    printf("\r\nCASE1: middle_len=%x \r\n", middle_len );
//    printf("\r\nrcv_buf[0]=%x \r\n", rcv_buf[0] );

    if(middle_len < 0)
        return ERROR;
    
    return middle_len;

}



// no data, have response
// implement the prcedure
int case2_TPDU(UCHAR* snd_buf, unsigned int snd_len, UCHAR *rcv_buf, unsigned int rcv_len){
    
    int middle_len; // 
    unsigned int temp_buffer_len = 300;
    unsigned int command_len = 5;
    UCHAR Licc,INS;
    UCHAR cmd[5] = {0};
    UINT  output_len = 0, temp_len;
    UINT  cursor = 0;
    UCHAR temp[300],firstReturnByte;
    UCHAR Le = 0;
    

    memmove(cmd, snd_buf, snd_len);
    
//    Le = cmd[4];	// 2023-10-19
//    cmd[4] = 0;		//

    INS = cmd[1];

    // result = CCID_Transmit(0,5,cmd,0,0);
    
////    g_sam_flag = 1;

    // result = CCID_Receive(0,&back_up, temp, NULL);
    middle_len = Tx_Rx_Pair(cmd,command_len,temp,temp_buffer_len);
    
    if(middle_len == -1)
        return ERROR;

    firstReturnByte = temp[0];

    if(middle_len == 2 && IsStatusFirstByte(firstReturnByte)){
        memmove(rcv_buf,temp,2);
        output_len = 2;
        return output_len;
    }

//    printf("\r\nfirstReturnByte=%x\r\n", firstReturnByte );
//    printf("\r\ntemp[1]=%x\r\n", temp[1]);
    
    if( firstReturnByte == INS )
      {
//    printf("\r\nmiddle_len=%x \r\n", middle_len );
      Licc = middle_len - 1 - 2;	// TTL = [INS Data(Le) SW1 SW2]
      goto GET_RESPONSE;
      }

//  if(firstReturnByte != 0x6c)
    if( (firstReturnByte != 0x6c) && (firstReturnByte != 0x61) )
        return ERROR;
    
    // update command
    Licc = temp[1];
    cmd[4] = Licc;
    
    if( firstReturnByte == 0x61 )
      {
      goto GET_RESPONSE;
      }

    // result = CCID_Transmit(0,5,cmd,0,0);

    // result = CCID_Receive(0,&back_up, temp, NULL);
    middle_len = Tx_Rx_Pair(cmd,command_len,temp,temp_buffer_len);

    firstReturnByte = temp[0];

    if(middle_len == -1)
        return ERROR;
    // response be INS [data(licc)] 90 00

GET_RESPONSE:
	
    if(firstReturnByte == INS){
        output_len = Licc + 2;
        // copy to actua rcv_buf
        memmove(&rcv_buf[cursor],&temp[1],output_len);
        return output_len;
   
    }
    // should be 61 xx
    else if(firstReturnByte == 0x61){

#if	0
        command_GetResponse(cmd,temp[1]/2);	// [00 c0 00 00 xx]

        temp_len = temp[1]/2;
        output_len += temp[1]/2;
#else
	command_GetResponse(cmd,temp[1]);	// [00 c0 00 00 xx]
	temp_len = temp[1];
//	output_len += temp[1];

//	temp_buffer_len = temp_len + 2 + 2;
//	printf("\r\ntemp_buffer_len=%d\r\n", temp_buffer_len );
	
//	printf("\r\ncmd[0]=%x\r\n", cmd[0]);
//	printf("\rcmd[1]=%x\r\n", cmd[1]);
//	printf("\rcmd[2]=%x\r\n", cmd[2]);
//	printf("\rcmd[3]=%x\r\n", cmd[3]);
//	printf("\rcmd[4]=%x\r\n", cmd[4]);
#endif

    }
    else
        return ERROR;

    middle_len = Tx_Rx_Pair(cmd,command_len,temp,temp_buffer_len);
//  printf("\r\nmiddle_len=%x\r\n", middle_len);

    if(middle_len == -1)
        return ERROR;

    firstReturnByte = temp[0];
    
//  printf("\r\nfirstReturnByte=%x\r\n", firstReturnByte );
//  printf("\rmiddle_len=%x\r\n", middle_len );

//  if( (firstReturnByte != 0xc0) || (temp[middle_len - 2] != 0x61))
    if( (firstReturnByte != 0xc0) || ((temp[middle_len - 2] != 0x61) && (temp[middle_len - 2] != 0x90)) )
        return ERROR;
    else{
    	if( (temp[middle_len - 2] == 0x90) && (temp[middle_len - 1] == 0x00) )
    	  {
    	  temp_len = middle_len - 1;	// ignore 0xC0
    	  output_len = temp_len;
    	  goto EXIT;
    	  }
    	
        memmove(&rcv_buf[cursor],&temp[1],temp_len);
        cursor += temp_len;

        command_GetResponse(cmd,temp[middle_len - 1]);// [00 c0 00 00 xx]

        temp_len = temp[middle_len - 1] + 2;
        output_len += temp_len;
        
        temp_buffer_len = temp_len;
    
    }

    middle_len = Tx_Rx_Pair(cmd,command_len,temp,temp_buffer_len);

    if(middle_len == -1)
        return ERROR;

    if(temp[0] != 0xc0)
        return ERROR;

EXIT:
    memmove(&rcv_buf[cursor],&temp[1],temp_len);

    return output_len;

}

int case3_TPDU(UCHAR* snd_buf, unsigned int snd_len, UCHAR *rcv_buf, unsigned int rcv_len){


    int middle_len; 
    unsigned int temp_buffer_len = 20;
    unsigned int command_len = 5;
    UCHAR Licc,INS;
    UCHAR cmd[5] = {0};
    UINT  output_len = 0, temp_len;
    UINT  cursor = 0;
    UCHAR temp[20],firstReturnByte;

    temp_len = snd_buf[4]; // number input data to be send

    memmove(cmd, snd_buf, 5);

    INS = cmd[1];

    middle_len = Tx_Rx_Pair(cmd,command_len,temp,temp_buffer_len);

    firstReturnByte = temp[0];

    // page 123 EV level 1, have chance to receive status
    if(middle_len == 2 && IsStatusFirstByte(firstReturnByte)){
        memmove(rcv_buf,temp,2);
        output_len = 2;
        return output_len;
    }

    // should be INS only, 
    if(temp[0] != INS)
        return ERROR;

    middle_len = Tx_Rx_Pair( snd_buf + 5,temp_len,temp,temp_buffer_len);

    if(middle_len != 2)
        return ERROR;
    
    memmove(rcv_buf,temp,2);
    return 2;

    


}

int case4_TPDU(UCHAR* snd_buf, unsigned int snd_len, UCHAR *rcv_buf, unsigned int rcv_len){

    int middle_len; // 
    unsigned int temp_buffer_len = 300;
    unsigned int command_len = 5;
    UCHAR Licc,INS;
    UCHAR cmd[5] = {0};
    UINT  output_len = 0, temp_len;
    UINT  cursor = 0;
    UCHAR temp[300],firstReturnByte;
    UCHAR status[2], max_len_response;

    temp_len = snd_buf[4];

    memmove(cmd, snd_buf, 5);

    INS = cmd[1];
    
////    g_sam_flag = 1;

//    printf("\r\ncase4_TPDU\r\n");

    middle_len = Tx_Rx_Pair(cmd,command_len,temp,temp_buffer_len);
    firstReturnByte = temp[0];

	printf("\r\nmiddle_len=%x\r\n", middle_len );
	printf("\r\nfirstReturnByte=%x\r\n", firstReturnByte );

    if(middle_len == 2 && IsStatusFirstByte(firstReturnByte)){
        memmove(rcv_buf,temp,2);
        return 2;
    }

    if(firstReturnByte != INS)
        return ERROR;


    middle_len = Tx_Rx_Pair( snd_buf + 5,temp_len,temp,temp_buffer_len);
    firstReturnByte = temp[0];
    max_len_response = temp[1];
    
    printf("\r\nfirstReturnByte=%x\r\n", firstReturnByte );
    printf("\r\nmax_len_response=%x\r\n", max_len_response );
    printf("\r\nmiddle_len=%x\r\n", middle_len );

    if(middle_len  != 2)
        return ERROR;
    
    if(middle_len == 2 && IsStatusFirstByte(firstReturnByte)){
        memmove(rcv_buf,temp,2);
        return 2;
    }

    

    if(firstReturnByte == 0x61){

        while(1){

            command_GetResponse(cmd, max_len_response);

            temp_len = temp[1];
            output_len += temp_len;

            middle_len = Tx_Rx_Pair(cmd,command_len,temp,temp_buffer_len);
            firstReturnByte = temp[0];

            if(firstReturnByte != 0xc0)
                return ERROR;
            else if( IsStatusFirstByte(temp[middle_len-2])){
                //(temp[back_up-1] == 0x00) && (temp[back_up-2] == 0x90)){

                temp_len += 2;
                memmove(&rcv_buf[cursor],&temp[1],temp_len);

                return output_len + 2;

            }
            else if( (temp[middle_len-2] == 0x61)){

                memmove(&rcv_buf[cursor],&temp[1],temp_len);
                cursor += temp_len;
                max_len_response = temp[middle_len - 1];

            }


        }

    }
    else if((firstReturnByte == 0x62) || (firstReturnByte == 0x63) || 
        ( (firstReturnByte != 0x90) && ( (firstReturnByte & 0xF0) == 0x90) )){
        // warning status
        status[0] = firstReturnByte;
        status[1] = temp[1];

        command_GetResponse(cmd, 0x00);

        middle_len = Tx_Rx_Pair(cmd,command_len,temp,temp_buffer_len);
        firstReturnByte = temp[0];

        if(firstReturnByte != 0x6c)
            return ERROR;
        Licc = temp[1];
        cmd[4] = Licc;

        middle_len = Tx_Rx_Pair(cmd,command_len,temp,temp_buffer_len);
        firstReturnByte = temp[0];

        if(firstReturnByte != 0xc0)
            return ERROR;
        
        memmove(rcv_buf,&temp[1],Licc);

        rcv_buf[Licc] = status[0];
        rcv_buf[Licc+1] = status[1];

        return Licc + 2;

    }
    else if ( (middle_len == 2) && IsStatusFirstByte(firstReturnByte)) {
        memmove(rcv_buf,temp,2);
        return 2;

    }
    
    return ERROR;

}




int t0_transceive(UCHAR* snd_buf, unsigned int snd_len, UCHAR *rcv_buf, unsigned int rcv_len)
{

    UCHAR CASE;
    int result;

    CASE = T0CmdParsing(snd_buf, snd_len);
    printf("\r\nCASE=%x\r\n", CASE );

    switch (CASE)
    {
    case CASE_1:
        result =  case1_TPDU(snd_buf,snd_len,rcv_buf,rcv_len);
        break;
    
    case CASE_2:
        result = case2_TPDU(snd_buf,snd_len,rcv_buf,rcv_len);
        break;
    case CASE_3:
        result = case3_TPDU(snd_buf,snd_len,rcv_buf,rcv_len);
        break;
    case CASE_4:
        result = case4_TPDU(snd_buf,snd_len,rcv_buf,rcv_len);
        break;
    default:
        return -1;
    }

    return result;
    

}







