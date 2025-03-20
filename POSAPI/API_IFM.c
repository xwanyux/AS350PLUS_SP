

//#ifndef _build_DSS_
#include "POSAPI.h"
#include "API_IFM.h"
#include "DEV_IFM.h"
#include "OS_PROCS.h"

#include <sys/ioctl.h> 
#include <stdio.h>

#include <fcntl.h> // define O_RDWR , open ....
#include "mxc_sim_interface.h"

#include "proto-t1.h"



#define TA 0
#define TB 1
#define TC 2
#define TD 3

#define	_SUPPORT_PTS_REAL_
//#define	_SUPPORT_PTS_TEMP_


static UCHAR SAM_STATUS[5] = {SAM_STATUS_CLOSE};
static Driver_Timing_DATA SAM_Driver_data[5]; // use for driver
static t1_state_t t1_state[5]; // data_sture use for protocol1
static int first_time_flag = 0;

int sim_driver_fd = -1;

UINT8	ATR_sam1[34] = {0};
UINT8	ATR_sam2[34] = {0};
UINT8	ATR_sam3[34] = {0};
UINT8	ATR_sam4[34] = {0};

UINT8	os_ScPTS = 0;		// flag for Protocol Type Selection (PTS)
UINT8	os_ScPTS_DHN = 0xFF;

UINT8	os_SAM_DHN0 = 0;

void	SAM_PutATR( UCHAR dhn, UCHAR *atr );
UINT32	SAM_GetATR( UCHAR dhn, UCHAR *atr );


/**
 *  this function is used to compute the power of value (only support integer value and exponet)
 *  @param[in] value        value to be power
 *  @param[in] exp          the expoenet
 *  @return power of the value
 */ 
int my_pow(int value, int exp){
    int temp = 1,i;
    for(i = 0; i < exp; i++)
        temp *= value;
    
    return temp;
}


/**
 *  this fuction is used to a rough delay only (should be replace by exact timer with ms )
 *  
 *  @param[in] num      delay time
 */
static void my_delay(int num){

    volatile int i,j,k;

    for(int i = 0; i < num; i++){
        for(int j = 0 ; j < 1000; j++){
            for(int k = 0; k < 1000; k++){

            }
        }
    }
}

/**
 *  this function is used to set the atr timing for sim driver receive function.
 * 
 *  @note  the reason is due to sim driver will have some error in future reciving,
 *         we only do one time atr, then the other use sim driver receive function 
 *         to get the other atr value. 
 */         
void set_atr_timing( unsigned int prot ){

    int errval = SIM_OK;
	sim_baud_t baud;
    sim_timing_t timing_data;
    // debug only

    unsigned int protocol = prot;	// SIM_PROTOCOL_T1;

    errval = ioctl(sim_driver_fd, SIM_IOCTL_SET_PROTOCOL, &protocol);
    // this is choosen by experiment 
    timing_data.bgt = 22;
    timing_data.cgt = 0;
    timing_data.cwt = 500;
    timing_data.bwt = 11 + 20 * 960;
    timing_data.wwt = 0;


    errval = ioctl(sim_driver_fd, SIM_IOCTL_SET_TIMING, &timing_data);

    // for now we just use default speed, not allow to change the speed
    baud.fi = 1;
	baud.di = 1;

	errval = ioctl(sim_driver_fd, SIM_IOCTL_SET_BAUD, &baud);


}

/**
 *    this function is used to show the timing data for differnt card
 *    @param[in] dhn       device handle number
 *    @note this function is debug only
 */ 
void show_Driver_Timeing_data(UCHAR dhn){
    printf("dhn:%d\n",dhn);
    printf("protocol:T%d\n",SAM_Driver_data[dhn].protocol - 1);
    printf("bgt:%d\n",SAM_Driver_data[dhn].bgt);
    printf("cwt:%d\n",SAM_Driver_data[dhn].cwt);
    printf("wwt:%d\n",SAM_Driver_data[dhn].wwt);
    printf("cgt:%d\n",SAM_Driver_data[dhn].cgt);
    printf("bwt:%d\n",SAM_Driver_data[dhn].bwt);
}

/**
 *    this function is used to initalize the timing data for protocol T0 or T1
 *    @param[in] dhn        device handle number
 *    @param[in] protocol   macro   SIM_PROTOCOL_T1 or SIM_PROTOCOL_T0
 */        
void init_default_value(UCHAR dhn, UCHAR protocol){

    if(protocol == SIM_PROTOCOL_T1){
        SAM_Driver_data[dhn].bgt = 22;
        SAM_Driver_data[dhn].cgt = 0;
        SAM_Driver_data[dhn].cwt = 13;
        SAM_Driver_data[dhn].bwt = 11 + 16 * 960;
        SAM_Driver_data[dhn].wwt = 0;
        t1_init(&t1_state[dhn],0);
    }
    else if(protocol == SIM_PROTOCOL_T0){

        // SAM_Driver_data[dhn].wwt = 9600;
        // SAM_Driver_data[dhn].cgt = 0;
        // printf("\r\nT=0:init_default_value()\r\n");

        SAM_Driver_data[dhn].bgt = 0;
        SAM_Driver_data[dhn].cgt = 0;
//      SAM_Driver_data[dhn].cgt = 1;	//1+1+5+10;
        SAM_Driver_data[dhn].cwt = 43;	//9600;
        SAM_Driver_data[dhn].bwt = 9600;
//      SAM_Driver_data[dhn].wwt = 13;	//9600; 2023-11-15 (temp solution to shorten APDU response time)
	SAM_Driver_data[dhn].wwt = 960*10*1;	// WWT=960*WI*D (WI=10, D=1 or 16)
    }

}

void Init_T0_WWT(UCHAR dhn, UCHAR APDUcase ){
	
	return;

//#if	0	// 2023-11-24, replaced by Init_T0_WWT_EX()
#if	0	// 2024-01-03, rollback for better performance
        SAM_Driver_data[dhn].bgt = 0;
        SAM_Driver_data[dhn].cgt = 1;	//0;
        SAM_Driver_data[dhn].cwt = 9600;
        SAM_Driver_data[dhn].bwt = 9600;
        if( APDUcase == 0 )	// 2024-07-12, special init for NEW ECC SAM
          {
          SAM_Driver_data[dhn].cgt = 1+1+5+10;	//+15;	// 2024-07-23, set to 1 for PTS mode 
          SAM_Driver_data[dhn].wwt = 960*10*1;	// WWT=960*WI*D (WI=10, D=1 or 16)
          return;
          }
        if( APDUcase == 4 )
          {
//        printf("\r\nCASE 4: Init_T0_WWT\r\n");
          SAM_Driver_data[dhn].wwt = 96;	// 2023-11-20 (temp solution to shorten APDU response time)
          }
        else
          {
	  SAM_Driver_data[dhn].wwt = 13;
          }
#else
        SAM_Driver_data[dhn].bgt = 0;
        SAM_Driver_data[dhn].cgt = 1;
        SAM_Driver_data[dhn].cwt = 9600;
        SAM_Driver_data[dhn].bwt = 9600;
        if( APDUcase == 0 )	// 2024-07-12, special init for NEW ECC SAM
          {
          SAM_Driver_data[dhn].cgt = 1+1+5+10;	//+15;	// 2024-07-23, set to 1 for PTS mode 
          SAM_Driver_data[dhn].wwt = 960*10*1;	// WWT=960*WI*D (WI=10, D=1 or 16)
          return;
          }
        if( APDUcase == 4 )
          SAM_Driver_data[dhn].wwt = 96;	// 2023-11-20 (temp solution to shorten APDU response time)
        else
          SAM_Driver_data[dhn].wwt = 13;
#endif

}

// extra WWT for "get response" in commnd.c
void Init_T0_WWT_EX( UCHAR len ){

#if	0	// 2024-01-03, rollback to Init_T0_WWT()
	if( len == 0 )
	  len = 1;
	
	if( SAM_Driver_data[os_SAM_DHN0].protocol == SIM_PROTOCOL_T0 )
	  {
	  SAM_Driver_data[os_SAM_DHN0].bgt = 0;
	  SAM_Driver_data[os_SAM_DHN0].cgt = 0;
	  SAM_Driver_data[os_SAM_DHN0].cwt = 9600;
	  SAM_Driver_data[os_SAM_DHN0].bwt = 9600;
	  SAM_Driver_data[os_SAM_DHN0].wwt = 83+13*len;
	  }
	  
//	printf("\r\nWWT=%d\r\n", SAM_Driver_data[os_SAM_DHN0].wwt );
#endif
}


/**
 *  This fuction use to initialize all timing data as protocol T1
 *  @note this function is debug only
 */ 
void init_all_SAM_Driver_Data(){
    int i ;
    for(i = 0; i < 5; i++)
        SAM_Driver_data[i].protocol = SIM_PROTOCOL_T1;
        init_default_value(i,SIM_PROTOCOL_T1);
}




/**
 *  This function is used to parse TD_i or T0 to know about the next 4 sequece symbol (TA TB TC TD)
 *  @param[in]  value        TD_i or T0 value
 *  @param[out] next_value   array of next 4 character (can be less maximum 4)
 *  @param[out] len          len of next_value
 */
void update_for_next_4_ATR(UCHAR value,UCHAR* next_value,int* len){

    UCHAR MASK;
    int i;
    *len = 0;
    MASK = 0x10;
    //printf("value:%x\n",value);
    for(i = 0; i < 4; i++){
        //printf("result:%x\n",value & MASK);
        if((value & MASK) != 0)
            next_value[(*len)++] = i;
        MASK *= 2;
    }
}



/**
 *   This function is used to update the timing data for atr_data
 *   @param[in] dhn             device handle number
 *   @param[in] atr_data        atr data
 *   @note this function is not fully implemented. most T0 protocol not implemenet 
 *         and now ignore di fi. for now, atr just provide the information for driver 
 *         to work timing properly
 */  
void update_Driver_Timing_Data(UCHAR dhn, sim_atr_t* atr_data){


    UCHAR protocol = -1;
    UCHAR temp;
    UCHAR CWI,BWI;
    int cursor = 0;
    int i,j,MASK,next_len = 0;
    UCHAR flag = 0; 

    UCHAR next_value[4] = {-1};

    
    // the first char alwaybe be 3B is not important, decoding is done by hardware
    cursor++;
    // the byte is T0, for T0 we can identify protocol be T=0 or T=1 (we only assuem this two)
    // by check the value of TD1 exist
    temp = atr_data->atr_buffer[cursor++];
    printf("T0:%2x ",temp);
    if( (temp & 0x80) == 0)
        SAM_Driver_data[dhn].protocol = SIM_PROTOCOL_T0;
    else
        SAM_Driver_data[dhn].protocol = SIM_PROTOCOL_T1;
    protocol = SAM_Driver_data[dhn].protocol;
    init_default_value(dhn,protocol);
    // MASK = 0x10;
    // for(i = 0; i < 4; i++){
    //     if(temp & MASK != 0)
    //         next_value[next_len++] = i;
    //     MASK *= 2;
    // }
    update_for_next_4_ATR(temp,next_value,&next_len);
    flag = 0;
    if(next_len != 0){
        // doing in next chucnk, dash_value = 1
        for(i = 0; i < next_len; i++){
            if(next_value[i] == TA){
                // ignore it now , TA1 store value Fi Di
                //temp = atr_data->atr_buffer[cursor];
                //printf("TA1:%2x ",temp);
            }
            else if(next_value[i] == TB){
                // ignore it now, TB1 (maybe is about voltage stuff)
                //temp = atr_data->atr_buffer[cursor];
                //printf("TAB1:%2x ",temp);
            }
            else if(next_value[i] == TC){
                // mean extra guard time
                temp = atr_data->atr_buffer[cursor];
                if(temp == 0xFF)
                    SAM_Driver_data[dhn].cgt = 0xFF;
                else 
                    //SAM_Driver_data[dhn].cgt = 12 + temp;
                    SAM_Driver_data[dhn].cgt = temp;
                

                //printf("TC1:%2x ",temp);
            }
            else if(next_value[i] == TD){
                // for now just for simplivity
                // this is work for protocol T=1
                flag = 1;
                temp= atr_data->atr_buffer[cursor];
                if(temp == 0x81){
                    next_len = 1;
                    next_value[0] = TD;
                }
                // this is the genreal case to do it
                // temp = atr_data->atr_buffer[cursor];
                // next_len = 0;
                // MASK = 0x10;
                // for(j = 0; j < 4; j++){
                //     if(temp & MASK != 0)
                //      next_value[next_len++] = j;
                // MASK *= 2;
            // }
            //printf("TD1:%2x ",temp);
            }
            cursor++;
        }
    }

    if(flag == 0)
        next_len = 0;

    //printf("\n");

    if(next_len != 0){
        // in the EMV, seem like this chuck only have value TD2
        temp = atr_data->atr_buffer[cursor++];
        update_for_next_4_ATR(temp,next_value,&next_len);
        // next_len = 0;
        // MASK = 0x10;
        // for(j = 0; j < 4; j++){
        // if(temp & MASK != 0)
        //     next_value[next_len++] = j;
        //     MASK *= 2;
        // }
        //printf("TD2:%x\n",temp);
    }

    // finial block for protocol dash_value = 3
    if(next_len != 0){
        for(i = 0; i < next_len; i++){
            if(next_value[i] == TA){
                //TA3 store value IFSC for T=1
                // value should be '10' to 'FE'
                temp = atr_data->atr_buffer[cursor];
                t1_set_param(&t1_state[dhn],IFD_PROTOCOL_T1_IFSC,temp);
                //printf("TA3:%x\n",temp);

            }
            else if(next_value[i] == TB){
                // get CWI and BWI for T=1 TB3
                temp = atr_data->atr_buffer[cursor];
                CWI = temp & 0x0F;
                BWI = (temp & 0xF0) >> 4;
                SAM_Driver_data[dhn].cwt = 11 + my_pow(2,CWI);
                SAM_Driver_data[dhn].bwt = 11 + my_pow(2,BWI) * 960;
                //printf("TB3:%x\n",temp);
                
            }
            else if(next_value[i] == TC){
                // if the first bit of 1 is one use CRC else use LRC
                temp = atr_data->atr_buffer[cursor];
                if(temp & 0x01 == 1)
                t1_set_param(&t1_state[dhn], IFD_PROTOCOL_T1_CHECKSUM_CRC, 0);
                else
                t1_set_param(&t1_state[dhn], IFD_PROTOCOL_T1_CHECKSUM_LRC, 0);

                //printf("TC3:%x\n",temp);

            }
            else if(next_value[i] == TD){
                // not use
                temp = atr_data->atr_buffer[cursor];
                //printf("TD3:%x\n",temp);
            }
            cursor++;
        }
    }

    //printf("\n");

    // left just historical byte and TCK(if have)

    


    //SAM_Driver_data[dhn].protocol = 1;

}

/**
 *  This fuction is ued to change the timing inforamiton when transmit or receive in 
 *  differnt card.
 *  @param[in] dhn              device handle number
 *  @return 0 or postive number mean no error, negative number mean error happend when set timing data
 */
int SetDriverConfiger(UCHAR dhn)
{
    int errval = SIM_OK;
	sim_baud_t baud;
    sim_timing_t timing_data;
    
    // debug only
    //show_Driver_Timeing_data(dhn);

    //unsigned int protocol = SIM_PROTOCOL_T1;

    errval = ioctl(sim_driver_fd, SIM_IOCTL_SET_PROTOCOL, &SAM_Driver_data[dhn].protocol);
    
    // i don't know why, when I using protocol T0 after some rest from another
    //  prtocol T=1 card, the response of APDU will be wrong (the error message is 
    //  receive too much NACK, but I don't know why.) 
    // you can think this method just diable the hardware driver the NAK ablitiy
    // you just replace the t0 timing to t1, they work the same  
    //errval = ioctl(sim_driver_fd, SIM_IOCTL_SET_PROTOCOL, &protocol);

    // retrieve timing data from Driver_Timing_DATA

    timing_data.bgt = SAM_Driver_data[dhn].bgt;
    timing_data.cwt = SAM_Driver_data[dhn].cwt;
    timing_data.wwt = SAM_Driver_data[dhn].wwt;
    timing_data.cgt = SAM_Driver_data[dhn].cgt;
    timing_data.bwt = SAM_Driver_data[dhn].bwt;


    errval = ioctl(sim_driver_fd, SIM_IOCTL_SET_TIMING, &timing_data);


    // for now we just use default speed, not allow to change the speed
    baud.fi = 1;
    baud.di = 1;
    
#ifdef	_SUPPORT_PTS_REAL_

    	if( (dhn == os_ScPTS_DHN) && (os_ScPTS) )
    	  {
    	  printf("\r\nSetup PTS timing\r\n");
    	  
#ifdef	_SUPPORT_PTS_TEMP_

    	  baud.fi = 1;	// F=372, EMV allowed (TA1=0x13)
    	  baud.di = 3;	// D=4
#else
    	  baud.fi = 9;	// F=512, special ECC high-speed SAM (TA1=0x18)
    	  baud.di = 5;	// D=16
#endif
    	  }
#endif

	errval = ioctl(sim_driver_fd, SIM_IOCTL_SET_BAUD, &baud);


    return errval;

}

/**
 *    this function is to check the device is already open.
 *    @param[in] acceptor           device handle number
 *    @return TRUE if already open, FALSE if not
 */
UCHAR check_device_already_open(UCHAR acceptor){

    if(acceptor > 4)
        return FALSE;
    if(SAM_STATUS[acceptor] == SAM_STATUS_CLOSE)
        return FALSE;
    return TRUE;
}
/**
 *   this function to check the device is finish reset.
 *   @param[in] acceptor            device handle number
 *   @return TRUE if finished reset, FALSE if not
 */
UCHAR check_device_finished_reset(UCHAR acceptor){
    if(SAM_STATUS[acceptor] == SAM_STATUS_RESET_FIN)
        return FALSE;
    return TRUE;
}


/**
 *  this funciton is to open the device
 *  @param[in] acceptor        should be macro, ICC1, SAM1, SAM2, SAM3, SAM4
 *  @return apiOutOfService if open fail, device handle number if oepn success
 */
UCHAR api_ifm_open(UCHAR acceptor){


    // not sure where to put this driver
    // but now just put it here
    if(sim_driver_fd < 0)
        sim_driver_fd = open("/dev/mxc_sim", O_RDWR);

    UCHAR result = TRUE;

    if(check_device_already_open(acceptor))
        return apiOutOfService;

    // close TDA DC to DC
    TDA_DC_TO_DC(0);
    // in I2c, we use number 1,2,3,4,5 not 0,1,2,3,4
    result &= Change_frequency(acceptor+1,EXT_FREQUENCY);
    // IO enable is necessary to change every time for every communication between different IO
    // In our hardware design, all of sam are share the same IO
    //result &= Enable_IO_SAM(acceptor+1);
    result &= Set_Voltage_SAM(acceptor+1,SC_PHY_5V);
    // remind some crucial things here, because the frequency only output on the slot which have 
    // already Start
    //Start_SAM(acceptor + 1);


    if(result != TRUE)
        return apiOutOfService;

    //Enable_IO_SAM(acceptor + 1);
    //Start_SAM(acceptor + 1);

    //FAST_FIND_ENABLE();
    FAST_FIND_DISABLE();
    // using dhn as acceptor number
    SAM_STATUS[acceptor] = SAM_STATUS_INIT_FIN;
    
//  return acceptor;
    return( acceptor + psDEV_SCR + 0x80 );

}

/**
 *     this function is use to close the device
 *     @param[in] dhn          device handle number
 *     @retval apiFailed       the device is not open
 *     @retval apiOK           close success
 */
UCHAR api_ifm_close(UCHAR dhn){

UCHAR dhn0 = dhn - psDEV_SCR - 0x80;

    if(!check_device_already_open(dhn0))
        return apiFailed;

    int i,flag = 0;
    SAM_STATUS[dhn0] = SAM_STATUS_CLOSE;


    Deactivate_SAM(dhn0 + 1);

    // check all device close;
    for(i = 0; i < 5; i++){
//      if(SAM_STATUS[dhn0] != SAM_STATUS_CLOSE)
        if(SAM_STATUS[i] != SAM_STATUS_CLOSE)	// 2025-03-06
            flag = 1;
    }

    if(!flag){
        close(sim_driver_fd);
        sim_driver_fd = -1;
        first_time_flag = 0;
    }
    return apiOK;
    
}

/**
 *      this function is used to check the ICC card present or not
 *      @param[in] dhn          device handle number
 *      @retval apiReady        if the ICC is present
 *      @retval apiNotReady     if the ICC is not present
 *      @note this function only work for ICC, is other sam value, always return apiReady
 */
UCHAR api_ifm_present(UCHAR dhn){

UCHAR dhn0 = dhn - psDEV_SCR - 0x80;

//	printf("\r\ndhn0=%x\r\n", dhn0);

    // only work for main ICC , dhn = 0, due to hardware implememtation
    if(Check_Present_SAM(dhn0 + 1))
    	{
    	usleep(200*1000);	// 2023-12-21, debunce time
        return apiReady;
	}
    else
        return apiNotReady;

}



/**
 *      this function is used to do cold reset to the card
 *      @param[in] dhn      device handle number
 *      @param[in] mode     reserve to future use (not used now)
 *      @param[out] atr     first byte of the atr array is the length of atr, the rest are the content of atr
 */
UCHAR api_ifm_reset(UCHAR dhn, UCHAR mode, UCHAR *atr){

    unsigned char atr_buffer[50];
    int result;
    sim_atr_t atr_data;
    sim_rcv_t receive;

    int retry_time = 2;
    int count = 0;
    
    UCHAR dhn0 = dhn - psDEV_SCR - 0x80;

    atr_data.atr_buffer = atr_buffer;

    if(!check_device_already_open(dhn0))
        return apiFailed;

#if	0
    if(first_time_flag == 1){
        my_delay(10);
        //Delay(20);
        //set_atr_timing();
        //result = ioctl(sim_driver_fd, SIM_IOCTL_COLD_RESET);
       // set_atr_timing();
       // result = ioctl(sim_driver_fd, SIM_IOCTL_COLD_RESET);

    }
#endif


    //if(first_time_flag != 0)
    
    
    //printf("cold reset%d\n",result);

    while(1) 
    {
        Enable_IO_SAM(dhn0 + 1);

        Start_SAM(dhn0 + 1);



        if(first_time_flag == 0){
//            printf("\r\nATR 1st time\r\n");
            
            //result = ioctl(sim_driver_fd, SIM_IOCTL_COLD_RESET);
            result = ioctl(sim_driver_fd, SIM_IOCTL_COLD_RESET);
            result = ioctl(sim_driver_fd, SIM_IOCTL_GET_ATR, &atr_data);
            first_time_flag = 1;
        }
        else{
//            printf("\r\nATR NOT 1st time\r\n");
            
            set_atr_timing( SIM_PROTOCOL_T1 );
            result = ioctl(sim_driver_fd, SIM_IOCTL_COLD_RESET);
            //result = ioctl(sim_driver_fd, SIM_IOCTL_COLD_RESET);
            receive.rcv_buffer = atr_buffer;
            receive.rcv_length = 50;
            receive.timeout = 1000;
            result = ioctl(sim_driver_fd, SIM_IOCTL_RCV, &receive);
#if	1
            if(result < 0)
                atr_data.size = 0;
#else
	    if( (result < 0) || ((result == 0) && (atr_data.size < 3)) )
	      {
	      atr_data.size = 0;
	      
	      my_delay(20);
	      result = ioctl(sim_driver_fd, SIM_IOCTL_RCV, &receive);
	      
              set_atr_timing( SIM_PROTOCOL_T0 );
              result = ioctl(sim_driver_fd, SIM_IOCTL_COLD_RESET);
              //result = ioctl(sim_driver_fd, SIM_IOCTL_COLD_RESET);
              receive.rcv_buffer = atr_buffer;
              receive.rcv_length = 50;
              receive.timeout = 1000;
              result = ioctl(sim_driver_fd, SIM_IOCTL_RCV, &receive);
	      }
#endif
            printf("\r\nrx result:%d\r\n",result);
            printf("\r\nrx size:%d\r\n",receive.rcv_length);
            atr_data.size = receive.rcv_length;
            atr_data.atr_buffer = receive.rcv_buffer;
        }

        printf("atr reset %d, %d\n",result,atr_data.errval);

        if((atr_data.size == 0) || (atr_data.size > 32) || (atr_data.atr_buffer[0] != 0x3b)){
            //Deactivate_SAM(dhn + 1);
            count++;
            
            printf("retry\n");
            if(count == retry_time){
                printf("----------------wrong atr:------------------------\n");
                for(int i = 0; i < atr_data.size; i++)
                    printf("%x ",atr_data.atr_buffer[i]);
                printf("\n");
                printf("----------------wrong atr END------------------------\n");

                printf("Card%d:%x\n",dhn0,Read_Status_Data_SAM(dhn0+1));
                return apiFailed;
            }
            else
                my_delay(20);
        }
        else{
            break;
        }

    }

    atr[0] = atr_data.size;
    memmove(&atr[1],atr_data.atr_buffer,atr_data.size);


    update_Driver_Timing_Data(dhn0,&atr_data);

    SAM_STATUS[dhn0] = SAM_STATUS_RESET_FIN;

    // show the card status
    printf("Card%d:%x\n",dhn0,Read_Status_Data_SAM(dhn0+1));
    
    // 2023-07-27
    if( (dhn0 >= SAM1) && (dhn0 <= SAM4) )
      {
      SAM_PutATR( dhn0, atr );
      
      os_ScPTS = 0;
      os_ScPTS_DHN = 0xFF;
      }
    
    return apiOK;

    
}

/**
 *      this function is to deactivate the IC card.
 *      @param[in] dhn      device handle number
 *      @retval apiFailed   the device is not open
 *      @retval apiOK       the device is deactivated
 */ 
UCHAR api_ifm_deactivate(UCHAR dhn){

    UCHAR result = TRUE;
    UCHAR dhn0 = dhn - psDEV_SCR - 0x80;
    
    if(!check_device_already_open(dhn0))
        return apiFailed;

    // can send by TDA8026 , don't need driver from SAM
    Deactivate_SAM(dhn0 + 1);

    SAM_STATUS[dhn0] = SAM_STATUS_INIT_FIN;

    return apiOK;
 

}




/**
 *     this function is to exchange the apdu to card
 *     @param[in] dhn       device handle number
 *     @param[in] c_apdu    the first two byte must be the length of transmit apdu command (use little Edian),
 *                          the three byte start as the actual command.
 *     @param[out] r_apdu   the first two byte are the length of response apdu command (use little Endian)
 *                          the three byte start as the actual response.
 *     @retval apiFailed    the transmission of apdu failed
 *     @retval apiOK        the transmission of apdu success
 */        
UCHAR api_ifm_exchangeAPDU(UCHAR dhn, UCHAR* c_apdu, UCHAR* r_apdu){

    UINT len_capdu = c_apdu[0] + 256 * c_apdu[1];
    int len_rapdu;

    UCHAR recv_buf[2048];

    UCHAR retry = 2;
    UCHAR count = 0;
    UCHAR result;
    
    UCHAR dhn0 = dhn - psDEV_SCR - 0x80;
    
//    ULONG	start_time = 0;
//    ULONG	end_time = 0;


    os_SAM_DHN0 = dhn0;	// backup


    // for testing just comment now
    if(!check_device_already_open(dhn0))
        return apiFailed;
    
    if(check_device_finished_reset(dhn0))
        return apiFailed;

    

    
    //Deactivate_SAM(dhn + 1);
    //Start_SAM(dhn + 1);
    
//    Init_T0_WWT_EX(1);	// 2023-11-24, reset

    // set protocol and timing for the driver
//    if(SAM_Driver_data[dhn0].protocol == SIM_PROTOCOL_T0)
//      Init_T0_WWT( dhn0, 0 );
    SetDriverConfiger(dhn0);

    Enable_IO_SAM(dhn0 + 1);

    if(SAM_Driver_data[dhn0].protocol == SIM_PROTOCOL_T1){
        // reset the state, then can run the protocol
        //t1_state[dhn].state = 0;
        len_rapdu = t1_transceive(&t1_state[dhn0],0,&c_apdu[2],len_capdu,recv_buf,2048);
    }
    else if(SAM_Driver_data[dhn0].protocol == SIM_PROTOCOL_T0)
    	{
        UCHAR	APDUcase;					// 2023-11-20
        APDUcase = T0CmdParsing( &c_apdu[2], len_capdu );	//
        Init_T0_WWT( dhn0, APDUcase );				//
        
        // omit now
//      start_time = OS_GET_SysTimerFreeCnt();
        
        len_rapdu = t0_transceive(&c_apdu[2], len_capdu, recv_buf,2048);
        
//      end_time = OS_GET_SysTimerFreeCnt();
//      LIB_DispHexLong( 4, 0, end_time - start_time );
	}
    
    //printf("len:%d\n",len_rapdu);

    if(len_rapdu <= 0)
        return apiFailed;

    else{
        r_apdu[0] = len_rapdu & 0x00FF;
        r_apdu[1] = (len_rapdu & 0xFF00) >> 8;
        memmove(&r_apdu[2], recv_buf, len_rapdu);
        return apiOK;
    }



}
//#endif


// ---------------------------------------------------------------------------
// FUNCTION: save ATR values.
// INPUT   : dhn
//	     The specified device handle number.
//	     atr
//           UCHAR len;	  // length of ATR data
//           UCHAR atr[]; // the ATR data
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	SAM_PutATR( UCHAR dhn, UCHAR *atr )
{
//	LIB_DumpHexData( 0, 1, atr[0]+1, atr );
//	switch( dhn - psDEV_SCR - 0x80 )
	switch( dhn )
	      {
	      case SAM1:
	      	   
	      	   memmove( ATR_sam1, atr, atr[0]+1 );
	      	   break;

	      case SAM2:
	      	   
	      	   memmove( ATR_sam2, atr, atr[0]+1 );
	      	   break;
	      	   
	      case SAM3:
	      	   
	      	   memmove( ATR_sam3, atr, atr[0]+1 );
	      	   break;
	      	   
	      case SAM4:
	      	   
	      	   memmove( ATR_sam4, atr, atr[0]+1 );
	      	   break;
	      }
}

// ---------------------------------------------------------------------------
// FUNCTION: retrieve ATR values.
// INPUT   : dhn
//	     The specified device handle number.
// OUTPUT  : atr
//           UCHAR len;	  // length of ATR data
//           UCHAR atr[]; // the ATR data
// OUTPUT  : none.
// RETURN  : TRUE / FALSE
// ---------------------------------------------------------------------------
UINT32	SAM_GetATR( UCHAR dhn, UCHAR *atr )
{
UINT32	result = FALSE ;

	
//	LIB_DispHexByte( 0, 5, dhn );
//	switch( dhn - psDEV_SCR - 0x80 )
	switch( dhn )
	      {
	      case SAM1:
	      	   
	      	   memmove( atr, ATR_sam1, ATR_sam1[0]+1 );
//	      	   LIB_DumpHexData( 0, 1, atr[0]+1, atr );
	      	   break;

	      case SAM2:
	      	   
	      	   memmove( atr, ATR_sam2, ATR_sam2[0]+1 );
	      	   break;
	      	   
	      case SAM3:
	      	   
	      	   memmove( atr, ATR_sam3, ATR_sam3[0]+1 );
	      	   break;
	      	   
	      case SAM4:
	      	   
	      	   memmove( atr, ATR_sam4, ATR_sam4[0]+1 );
	      	   break;
	      }
	      
	if( atr[0] != 0 )
	  result = TRUE;
	
	return( result );
}

// ---------------------------------------------------------------------------
UCHAR	api_ifm_exchangePTS( UCHAR dhn, UCHAR *c_apdu, UCHAR *r_apdu )
{
//BSP_SC_IF	*pSc;
ULONG	result;
UCHAR	atr[34];
UCHAR	cmdPTS1[6]={0x04, 0x00, 0xFF, 0x10, 0x13, 0xFC};	// F=372, D=4   (TA1=0x13), TEMP
UCHAR	cmdPTS2[6]={0x04, 0x00, 0xFF, 0x10, 0x95, 0x7A};	// F=512, D=16  (TA1=0x18), FINAL

UCHAR	dhn0 = dhn - psDEV_SCR - 0x80;

//	pSc = IFM_CheckDHN( dhn );
//	if( pSc == NULLPTR )
//	  return( apiFailed );

#ifdef	_SUPPORT_PTS_TEMP_
	    // 2023-10-13, temp solution
	    r_apdu[0] = 4;
	    r_apdu[1] = 0;
	    r_apdu[2] = 0xff;	// PTSS
	    r_apdu[3] = 0x10;	// PTS0
	    r_apdu[4] = 0x95;	// PTS1
	    r_apdu[5] = 0x7a;	// PCK
	    return( apiOK );
#endif


//#ifdef	_SUPPORT_PTS_	// to be implemented!

	if( SAM_GetATR( dhn0, atr ) )
	  {
	  // 12 (length)
	  // 3B 1F 18 53 4C 45 20
	  // 37 38 43 46 58 34 30
	  // 30 30 50 81
	  if( (atr[2] & 0x10) && (atr[3] == 0x18) )	// TA1=0x18?
	    goto PTS_ST;
	  else
	    {
	    // 2018-12-13, skipped if TA1 <> 0x18
	    r_apdu[0] = 4;
	    r_apdu[1] = 0;
	    r_apdu[2] = 0xff;	// PTSS
	    r_apdu[3] = 0x10;	// PTS0
	    r_apdu[4] = 0x95;	// PTS1
	    r_apdu[5] = 0x7a;	// PCK
	    return( apiOK );
	    }
	  }
//#else
//	    // TEMP solution
//	    // 2018-12-13, skipped if TA1 <> 0x18
//	    r_apdu[0] = 4;
//	    r_apdu[1] = 0;
//	    r_apdu[2] = 0xff;
//	    r_apdu[3] = 0x10;
//	    r_apdu[4] = 0x95;
//	    r_apdu[5] = 0x7a;
//	    return( apiOK );
//#endif

PTS_ST:

#ifdef	_SUPPORT_PTS_TEMP_

	result = api_ifm_exchangeAPDU( dhn, cmdPTS1, r_apdu );
	if( result == apiOK )
	  {
	  if( !memcmp( r_apdu, cmdPTS1, sizeof(cmdPTS1) ) )
	    memmove( r_apdu, cmdPTS2, sizeof(cmdPTS2) ); // pseudo response
	  }
#else
	SAM_Driver_data[dhn0].cgt = 1;	// for PTS
	
	printf("\r\nExchange PTS\r\n");
	result = api_ifm_exchangeAPDU( dhn, c_apdu, r_apdu );
	
#endif
	
	if( result == apiOK )
	  {
	  printf("\r\nPTS[0]=%x", r_apdu[0]);
	  printf("\r\nPTS[1]=%x", r_apdu[1]);
	  printf("\r\nPTS[2]=%x", r_apdu[2]);
	  printf("\r\nPTS[3]=%x", r_apdu[3]);
	  printf("\r\nPTS[4]=%x", r_apdu[4]);
	  printf("\r\nPTS[5]=%x\r\n", r_apdu[5]);
	  
	  if( (r_apdu[0] == 3) && (r_apdu[1] == 0) &&
	      (r_apdu[2] == 0x10) && (r_apdu[3] == 0x95) && (r_apdu[4] == 0x7a) )
	    {
	    // 2024-07-23, temp solution for PTS, 1'st byte 0xff is not captured by driver for unknown reason.
	    r_apdu[0] = 4;
	    r_apdu[1] = 0;
	    r_apdu[2] = 0xff;	// PTSS
	    r_apdu[3] = 0x10;	// PTS0
	    r_apdu[4] = 0x95;	// PTS1
	    r_apdu[5] = 0x7a;	// PCK
	    }
	  
	  os_ScPTS = 1;
	  os_ScPTS_DHN = dhn0;
	  
	  SAM_Driver_data[dhn0].cgt = 0;	// reset to default
	  }

	return( result );

}
