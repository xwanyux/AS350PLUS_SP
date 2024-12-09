//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS320-A                                                    **
//**                                                                        **
//**  FILE     : DEV_MSR.C                                                  **
//**  MODULE   : 							    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2007/08/21                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2007 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------

#include "DEV_MSR.h"
#include "POSAPI.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bsp_gpio.h"
#include <sys/ioctl.h>   
#include <sys/time.h>   
#include <sys/types.h>   
#include <fcntl.h>   
#include <unistd.h>   
#include <errno.h> 

#include "msr_dev.h"
#include "API_EXTIO.h"



#define CLEAR_DATA 0
#define KEEP_DATA  1 



//BSP_MCR *os_pMcr;
UINT32	os_fMsrRead = 0;				// 1=ever read
UINT8	os_Trk1CharBuffer[MAX_TRK1_CHAR_CNT+1];		// track 1 LEN(1)+DATA(79)
UINT8	os_Trk2CharBuffer[MAX_TRK2_CHAR_CNT+1];		// track 2 LEN(1)+DATA(40)
UINT8	os_Trk3CharBuffer[MAX_TRK3_CHAR_CNT+1];		// track 3 LEN(1)+DATA(107)
UINT8	os_Trk1Status = TRK_STATUS_NO_DATA;
UINT8	os_Trk2Status = TRK_STATUS_NO_DATA;
UINT8	os_Trk3Status = TRK_STATUS_NO_DATA;

UCHAR   track1len = 0;
UCHAR 	track2len = 0;
UCHAR 	track3len = 0;
static UCHAR 	swiped = 0;

UCHAR   clear_flag = 0;

UCHAR  track1Table[] = { 0x20, 0x01, 0x02, 0x03, '$', '%', 0x06, 0x07, '(', ')', '*', '+',
						  ',',  '-',  '.',  '/', '0', '1',  '2',  '3', '4', '5', '6', '7',
						  '8',  '9', 0x1A, 0x1B,0x1C,0x1D, 0x1E,  '?', '@', 'A', 'B', 'C',
						  'D',  'E',  'F',  'G', 'H', 'I',  'J',  'K', 'L', 'M', 'N', 'O',
						  'P',  'Q',  'R',  'S', 'T', 'U',  'V',  'W', 'X', 'Y', 'Z',0x3B,
						 0x3C, 0x3D,  '^',  '_'};
UCHAR  track2Table[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0x3A,  0x3B, 0x3C, 0x3D, 0x3E, 0x3F};

UINT8	os_Tracks = 0;

UCHAR	os_TrackSwiped = TRK_STATUS_NO_SWIPED;

// driver 
static int driver_fd_msr = 0;



// ---------------------------------------------------------------------------
// FUNCTION: To gain access of magnetic stripe interface driver.
// INPUT   : tracks -- MSR track number.
// OUTPUT  : none.
// RETURN  : False(Failed)
//           TRUE    (OK)
// ---------------------------------------------------------------------------
UCHAR OS_MSR_Open( UINT8 tracks )
{
	int fd;
	int result;
	MODE_t msr_type = GET_RESOURCE;

	init_msr();
	
	fd = open("/dev/mydev",O_RDONLY);
	if(fd < 0){
		perror("open:");
		return FALSE;
	}
	// need to free the icc the device first
	
	ioctl(fd, IOCTL_SWITCH_MODE ,&msr_type);

	result = ioctl(fd, IOCTL_OPEN_TRACK,&tracks);
	if(result < 0){
		// open track fail
		close(fd);
		return FALSE;
	}
	driver_fd_msr = fd;
	return TRUE;

}

// ---------------------------------------------------------------------------
// FUNCTION: To deactivate the specified magnetic stripe interface driver's state machine.
// INPUT   : pMcr -- pointer to the BSP_MCR structure.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UINT8 OS_MSR_Close()
{
	int result;
    result = close(driver_fd_msr);
	if(result < 0){
		perror("close:");
		return FALSE;
	}
	os_TrackSwiped = FALSE;
	clear_buffer();
	clear_status();
	driver_fd_msr = -1;
	return TRUE;

}

// ---------------------------------------------------------------------------
// FUNCTION: To get the current status of MSR track(s).
// INPUT   : action (RFU, no function in this version)
//           0 = clear the status after reading.
//           1 = keep the status after reading.
// OUTPUT  : dbuf
//	     UCHAR  swipe ;            // MSR swiped status.
//	     UCHAR  isoTrk1 ;          // iso track1 status.
//	     UCHAR  isoTrk2 ;          // iso track2 status.
//	     UCHAR  isoTrk3 ;          // iso track3 status.
// RETURN  : none.
// ---------------------------------------------------------------------------
void OS_MSR_Status( UINT8 action, UINT8 *dbuf )
{
	// omit action
	int count;
	int result;
	char buf[800]; // use by all source code
	dbuf[0] = TRK_STATUS_NO_SWIPED;
	dbuf[1] = TRK_STATUS_NO_DATA;
	dbuf[2] = TRK_STATUS_NO_DATA;
	dbuf[3] = TRK_STATUS_NO_DATA;

	// in order to make type consistent to IOCTL
	char* buf_pt = buf;


	ioctl(driver_fd_msr, IOCTL_SWIPE, &swiped);
	// printf("os_TrackSwiped=%d\n",os_TrackSwiped);
	
	if(swiped==TRUE)
	{
		dbuf[0]	=  TRK_STATUS_SWIPED;
		os_TrackSwiped = TRK_STATUS_SWIPED;
		printf("inside SWIPED\n");
		// start to get data track by track
		// track 1
		count = ioctl(driver_fd_msr, IOCTL_GET_TRACK1_DATA,&buf_pt);
		printf("count=%d\n",count);
		// mean have data
		if(count > 0){
			result = decode_track1(buf, count);
			if(result == FALSE)
				dbuf[1] = TRK_STATUS_DATA_ERROR;
			else
				dbuf[1] = TRK_STATUS_DATA_OK;
		}

		count = ioctl(driver_fd_msr, IOCTL_GET_TRACK2_DATA,&buf_pt);
		// mean have data
		if(count > 0){
			result = decode_track2(buf, count);
			if(result == FALSE)
				dbuf[2] = TRK_STATUS_DATA_ERROR;
			else
				dbuf[2] = TRK_STATUS_DATA_OK;
		}

		count = ioctl(driver_fd_msr, IOCTL_GET_TRACK3_DATA,&buf_pt);
		// mean have data
		if(count > 0){
			result = decode_track3(buf, count);
			if(result == FALSE)
				dbuf[3] = TRK_STATUS_DATA_ERROR;
			else
				dbuf[3] = TRK_STATUS_DATA_OK;
		}

		// update the global parameters
		os_Trk1Status = dbuf[1];
		os_Trk2Status = dbuf[2];
		os_Trk3Status = dbuf[3];
	}
	
	if(action)
	{
		dbuf[0] = os_TrackSwiped;
		dbuf[1] = os_Trk1Status;
		dbuf[2] = os_Trk2Status;
		dbuf[3] = os_Trk3Status;
	}
	else
	{
		os_TrackSwiped = TRK_STATUS_NO_SWIPED;
		os_Trk1Status = TRK_STATUS_NO_DATA;
		os_Trk2Status = TRK_STATUS_NO_DATA;
		os_Trk3Status = TRK_STATUS_NO_DATA;
	}
	/*
	if(os_TrackSwiped == FALSE){
		dbuf[0] = TRK_STATUS_NO_SWIPED;
		dbuf[1] = TRK_STATUS_NO_DATA;
		dbuf[2] = TRK_STATUS_NO_DATA;
		dbuf[3] = TRK_STATUS_NO_DATA;
	}
	// actaully need to get data from kernel
	else if(os_TrackSwiped == TRUE){
		dbuf[0]	=  TRK_STATUS_SWIPED;
		// printf("@@os_Trk1Status=%d\n",os_Trk1Status);
		if(os_Trk1Status == TRK_STATUS_NO_DATA && os_Trk2Status == TRK_STATUS_NO_DATA
			&& os_Trk3Status == TRK_STATUS_NO_DATA){
			printf("inside SWIPED\n");
			// start to get data track by track
			// track 1
			count = ioctl(driver_fd_msr, IOCTL_GET_TRACK1_DATA,&buf_pt);
			printf("count=%d\n",count);
			// mean have data
			if(count > 0){
				result = decode_track1(buf, count);
				if(result == FALSE)
					dbuf[1] = TRK_STATUS_DATA_ERROR;
				else
					dbuf[1] = TRK_STATUS_DATA_OK;
			}

			count = ioctl(driver_fd_msr, IOCTL_GET_TRACK2_DATA,&buf_pt);
			// mean have data
			if(count > 0){
				result = decode_track2(buf, count);
				if(result == FALSE)
					dbuf[2] = TRK_STATUS_DATA_ERROR;
				else
					dbuf[2] = TRK_STATUS_DATA_OK;
			}

			count = ioctl(driver_fd_msr, IOCTL_GET_TRACK3_DATA,&buf_pt);
			// mean have data
			if(count > 0){
				result = decode_track3(buf, count);
				if(result == FALSE)
					dbuf[3] = TRK_STATUS_DATA_ERROR;
				else
					dbuf[3] = TRK_STATUS_DATA_OK;
			}

			// update the global parameters
			os_Trk1Status = dbuf[1];
			os_Trk2Status = dbuf[2];
			os_Trk3Status = dbuf[3];
		}
		else{

			dbuf[1] = os_Trk1Status;
			dbuf[2] = os_Trk2Status;
			dbuf[3] = os_Trk3Status;

		}

	}


	// psedo clear
	// have some bug on it, now just not clean
	if(action == CLEAR_DATA){
		os_TrackSwiped = FALSE;
		//clear_status();
	}
*/

}

// ---------------------------------------------------------------------------
// FUNCTION: To read the captured data from the MSR device.
// INPUT   : UCHAR  action ; 	// 0x00 = system will clear all captured data after reading.
//	                   	// 0x01 = system will keep all captured data after reading.
//	     UCHAR  TrackNo ;	// MSR Track Number 
// OUTPUT  : dbuf -- the concatenation track data of the specified TrackNo.
//	     UCHAR  Length ;         // length of the track data.
//	     UCHAR  StartSentinel ;  // start sentinel of the track.
//	     UCHAR  Data[Length-2] ; // data in the track.
//           UCHAR  EndSentinel ;    // end sentinel of the track.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
//modified by West 20211004
UINT8 OS_MSR_Read( UINT8 action, UINT8 TrackNo, UINT8 *dbuf )
{
	UCHAR dbuf_status[4];
	printf("OS_MSR_Read\n");
	switch (TrackNo)
	{
	case ISO_TRACK1:
		memmove(dbuf, os_Trk1CharBuffer, track1len);
		break;
	case ISO_TRACK2:
		memmove(dbuf, os_Trk2CharBuffer, track2len);
		break;
	case ISO_TRACK3:
		memmove(dbuf, os_Trk3CharBuffer, track3len);
		break;
	case ISO_TRACK1 + ISO_TRACK2:
		memmove(dbuf, os_Trk1CharBuffer, track1len);
		memmove(dbuf+track1len, os_Trk2CharBuffer, track2len);
		break;
	case ISO_TRACK1 + ISO_TRACK3:
		memmove(dbuf, os_Trk1CharBuffer, track1len);
		memmove(dbuf+track1len, os_Trk3CharBuffer, track3len);
		break;
	case ISO_TRACK2 + ISO_TRACK3:
		memmove(dbuf, os_Trk2CharBuffer, track2len);
		memmove(dbuf+track2len, os_Trk3CharBuffer, track3len);
		break;
	case ISO_TRACK1 + ISO_TRACK2 + ISO_TRACK3:
		memmove(dbuf, os_Trk1CharBuffer, track1len);
		memmove(dbuf+track1len, os_Trk2CharBuffer, track2len);
		memmove(dbuf+track2len+track1len, os_Trk3CharBuffer, track3len);
		break;
	default:
		return FALSE;
	}	

	if(action == CLEAR_DATA)
	{
		OS_MSR_Status(0, dbuf_status);
		os_TrackSwiped = TRK_STATUS_NO_SWIPED;
		clear_buffer();
		clear_status();
	}
	//memmove(dbuf, os_Trk2CharBuffer, track2len);

}
/*
UINT8 OS_MSR_Read( UINT8 action, UINT8 TrackNo, UINT8 *dbuf )
{
	UCHAR dbuf_status[4];
	if(os_TrackSwiped == FALSE){
		OS_MSR_Status(1, dbuf_status);
	}
	printf("os_TrackSwiped=%d\n",os_TrackSwiped);
	if(os_TrackSwiped == FALSE){
		return FALSE;
	}
	else{
		printf("OS_MSR_Read\n");
		switch (TrackNo)
		{
		case ISO_TRACK1:
			memmove(dbuf, os_Trk1CharBuffer, track1len);
			break;
		case ISO_TRACK2:
			memmove(dbuf, os_Trk2CharBuffer, track2len);
			break;
		case ISO_TRACK3:
			memmove(dbuf, os_Trk3CharBuffer, track3len);
			break;
		case ISO_TRACK1 + ISO_TRACK2:
			memmove(dbuf, os_Trk1CharBuffer, track1len);
			memmove(dbuf+track1len, os_Trk2CharBuffer, track2len);
			break;
		case ISO_TRACK1 + ISO_TRACK3:
			memmove(dbuf, os_Trk1CharBuffer, track1len);
			memmove(dbuf+track1len, os_Trk3CharBuffer, track3len);
			break;
		case ISO_TRACK2 + ISO_TRACK3:
			memmove(dbuf, os_Trk2CharBuffer, track2len);
			memmove(dbuf+track2len, os_Trk3CharBuffer, track3len);
			break;
		case ISO_TRACK1 + ISO_TRACK2 + ISO_TRACK3:
			memmove(dbuf, os_Trk1CharBuffer, track1len);
			memmove(dbuf+track1len, os_Trk2CharBuffer, track2len);
			memmove(dbuf+track2len+track1len, os_Trk3CharBuffer, track3len);
			break;
		default:
			return FALSE;
		}
	}

	if(action == CLEAR_DATA){
		os_TrackSwiped = FALSE;
		clear_buffer();
		clear_status();
	}
	//memmove(dbuf, os_Trk2CharBuffer, track2len);

}
*/


/**
*    This function is convert binary array to hex value, and
*	 support both big endian and small endian
* 
*    @param[in]   array             binary array
*    @param[in]   len               array length
*    @param[in]   encodingRule      marco of SmallEndian or BigEndian
*    @return                        converted value    
*/
unsigned int BinaryToHex(char* array, int len, int encodingRule){
    unsigned int temp = 0;
    int i;

    if(encodingRule == SamllEndian){
        for(i = len -1; i >= 0; i--){
            temp *= 2;
            temp += array[i];
        }
    }
    else if(encodingRule == BigEndian){
        for(i = 0; i < len; i++){
            temp *= 2;
            temp += array[i];
        }
    }

    return temp;
}


/**
*   This function is check the binary array parity , supprot both Even and Odd 
* 
*	@param[in] array				binary array
*	@param[in] len					array length
*	@param[in] mode   				macro of Odd or Even
*	@retval TRUE					pass the parity check
*	@retval FALSE					fail the pairty check
*/
UCHAR CheckParity(char* array, int len, int mode){
    int temp = 0;
    int i;
    for(i = 0; i < len; i++)
        temp += array[i];

    
    if(mode == Odd){
        if(temp % 2 == 1)
            return TRUE;
        else
            return FALSE; 
    }
    else if(mode == Even){
        if(temp % 2 == 0)
            return TRUE;
        else
            return FALSE;
    }

}

/**
 *    This function is to reverse the array data
 * 		
 *    @param[in] track          data
 * 	  @param[in] len			data length
 * 
 */
void reverse(UCHAR* track, UINT len){

    int i;
    int temp;
    for(i = 0; i < len/2; i++){
        temp = track[i];
        track[i] = track[len - 1 -i];
        track[len - 1 -i] = temp;
    }

}
/**
 * 	  This function is toggle binary data, convert 1 to 0, 0 to 1
 *    @param[in] track			binary data
 * 	  @param[in] len			data length
 */
void toggleData(UCHAR* track, UINT len){
	int i;
	for(i = 0; i < len; i++){
		if(track[i] == 0)
			track[i] = 1;
		else
			track[i] = 0;
	}
}

/**
 * 	This function is to clear buffer of track data
 */
void clear_buffer(){

	memset(os_Trk1CharBuffer,0,MAX_TRK1_CHAR_CNT+1);
	memset(os_Trk2CharBuffer,0,MAX_TRK2_CHAR_CNT+1);
	memset(os_Trk3CharBuffer,0,MAX_TRK3_CHAR_CNT+1);
	track1len = 0;
	track2len = 0;
	track3len = 0;

	
}
/**
 *  This function is to clear the status data only
 */ 
void clear_status(){
	os_Trk1Status = TRK_STATUS_NO_DATA;
	os_Trk2Status = TRK_STATUS_NO_DATA;
	os_Trk3Status = TRK_STATUS_NO_DATA;
}
/**
 * 	This function is to decode for track1 data
 * 	@param[in] binary_source 		binary source of track1 data
 *  @param[in] source_len			binary source len
 *  @retval TURE					decode track 1 success
 *  @retval FALSE					decode track 1 fail
 */
UCHAR decode_track1(UCHAR* binary_soruce, UINT source_len){

	track_decode_information track_info = {
		.start_symbol_hex_value = ISO_TRK1_ST_SENTINEL,
		.end_symbol_hex_value = ISO_TRK1_END_SENTINEL,
		.decode_block_len = ISO_TRK1_DECODE_BLOCK_LEN,
		.HexToCharTable = track1Table,
		.TrackDecodeBuffer = os_Trk1CharBuffer
	};
	UCHAR result;
	result =  reverse_and_non_reverse_decode(&track_info,binary_soruce, source_len);
	// update the global len
	if(result == TRUE)
		track1len = os_Trk1CharBuffer[0] + 1;
	else
		track1len = 0;
	
	return result;

}


/**
 * 	This function is to decode for track2 data
 * 	@param[in] binary_source 		binary source of track2 data
 *  @param[in] source_len			binary source len
 *  @retval TURE					decode track 2 success
 *  @retval FALSE					decode track 2 fail
 */
UCHAR decode_track2(UCHAR* binary_soruce, UINT source_len){

	track_decode_information track_info = {
		.start_symbol_hex_value = ISO_TRK2_ST_SENTINEL,
		.end_symbol_hex_value = ISO_TRK2_END_SENTINEL,
		.decode_block_len = ISO_TRK2_DECODE_BLOCK_LEN,
		.HexToCharTable = track2Table,
		.TrackDecodeBuffer = os_Trk2CharBuffer
	};
	UCHAR result;
	result =  reverse_and_non_reverse_decode(&track_info,binary_soruce, source_len);
	// update the global len
	if(result == TRUE)
		track2len = os_Trk2CharBuffer[0] + 1; // including the first data
	else
		track2len = 0;
	
	return result;

}

/**
 * 	This function is to decode for track3 data
 * 	@param[in] binary_source 		binary source of track3 data
 *  @param[in] source_len			binary source len
 *  @retval TURE					decode track 3 success
 *  @retval FALSE					decode track 3 fail
 */
UCHAR decode_track3(UCHAR* binary_soruce, UINT source_len){

	track_decode_information track_info = {
		.start_symbol_hex_value = ISO_TRK3_ST_SENTINEL,
		.end_symbol_hex_value = ISO_TRK3_END_SENTINEL,
		.decode_block_len = ISO_TRK3_DECODE_BLOCK_LEN,
		.HexToCharTable = track2Table,
		.TrackDecodeBuffer = os_Trk3CharBuffer
	};
	UCHAR result;
	result =  reverse_and_non_reverse_decode(&track_info,binary_soruce, source_len);
	// update the global len
	if(result == TRUE)
		track3len = os_Trk3CharBuffer[0] + 1;
	else
		track3len = 0;
	
	return result;

}

/**
 * 	This function is to call decode_track in both reverse and no reverse direction 
 * 	@param[in] track_info 			necessary infomation for decoding track
 * 	@param[in] binary_source		binary source
 *  @param[in] source_len			binary source len
 *  @retval TURE					decode track success
 *  @retval FALSE					decode track fail
 */
UCHAR reverse_and_non_reverse_decode(track_decode_information* track_info,
					UCHAR* binary_soruce, UINT source_len){

	UCHAR result;

	result = decode_track(track_info, binary_soruce, source_len);
		printf("result=%d\n",result);
	if(result == FALSE){
		reverse(binary_soruce, source_len);
		result = decode_track(track_info,binary_soruce,source_len);
		printf("result=%d\n",result);
	}

	return result;

}

/**
 * 	 This function is the alogirthm to decode binary track data.
 * 
 * 	@param[in] track_info			necessary informaiton for decoding track
 *  @param[in] binary_source		binary source
 *  @param[in] source_len			binary source length
 * 	@retval TRUE					decode track success
 * 	@retval FALSE					decode track fail
 */
UCHAR decode_track(track_decode_information* track_info,
					UCHAR* binary_soruce, UINT source_len){

    int i,j;
    int start_idx = 0;
    int end_idx = -1;
    int hexValue = 0;
	int decode_block_len = track_info->decode_block_len;
	int block_len_no_parity = decode_block_len - 1;
	int start_symbol_hex_value = track_info->start_symbol_hex_value;
	int end_symbol_hex_value = track_info->end_symbol_hex_value;

	UCHAR* start_pointer;

    for(i = 0; i <source_len; i++){
		start_pointer = binary_soruce + i;
        hexValue = BinaryToHex(start_pointer,block_len_no_parity,SamllEndian);
        // we only care about the first time we see start and first time we see start symbol
        if(hexValue == start_symbol_hex_value && CheckParity(start_pointer, decode_block_len, Odd)){
            start_idx = i;
            break;
        }
    }
    // this is make sure the end sequnce we get is correct 
    // this is the same as synchnize data
    for(i = start_idx; i < source_len; i = i + decode_block_len ){
		start_pointer = binary_soruce + i;
        hexValue = BinaryToHex(start_pointer, block_len_no_parity, SamllEndian);
        if(hexValue == end_symbol_hex_value && CheckParity(start_pointer, decode_block_len, Odd)){
            end_idx = i;
            break;
        }
    }
    //printf("start_idx:%d,",start_idx);
    //printf("end_idx:%d\n",end_idx);
    if(end_idx < start_idx)
        return FALSE;
    // LRC 
    int LRC_check = 0;
    int count = 1; // start from one because we add a len in the first postion
	// we need to check finial LRC 
    for(i = start_idx; i <= end_idx + decode_block_len; i = i+ decode_block_len){
		start_pointer = binary_soruce + i;
        if(CheckParity(start_pointer, decode_block_len, Odd) == FALSE)
            return FALSE;
        else{
            //for(j = i; j< i + 5; j++)
            //    printf("%d",binary_soruce[j]);
            //printf("\n");
            hexValue = BinaryToHex(start_pointer,block_len_no_parity,SamllEndian);
            // printf("i:%d,hexValue:%d,decode:%c,LRC_cjeck:%d\n",i,hexValue,track_info->HexToCharTable[hexValue],LRC_check);
            LRC_check ^= hexValue;
            track_info->TrackDecodeBuffer[count++] = track_info->HexToCharTable[hexValue];
        }
    }

	

   printf("%d\n",LRC_check);

    if((LRC_check & 0xFFFF) != 0)
        return FALSE;
    
    
    //track2len = count - 1;
	//os_Trk2CharBuffer[0] = track2len;
	// minus one LRC and first charater
	track_info->TrackDecodeBuffer[0] = count - 2;

    return TRUE;					
					
					
				

}


/**
 * 	This function is use to set the extension IO SAM_FAST_FIND(P0.3) to 0
 *  @note when SAM_FAST_FIND set to 1, the MSR can get any data. make sure
 * 		  this pin is set to 0 when using msr device.
 * 		
 */  
void init_msr(){

	EXTIO_GPIO(_SAM_FAST_FIND_BANK_ADDR,_SAM_FAST_FIND_GPIO_PORT,_SAM_FAST_FIND_GPIO_NUM,0);

}