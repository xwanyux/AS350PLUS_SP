//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L (32-bit Platform)                                     **
//**  PRODUCT  : AS350                                                      **
//**                                                                        **
//**  FILE     : AUX_DLESTX2.C                                              **
//**  FUNCTION : This protocol is used on HitachiOrmon IPASS Card Reader.   **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2015/08/06                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2015 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
// Package Format:
//
//	DLE(1) STX(1) LEN(2) CMD_PAYLOAD(n) BCC(1) DLE(1) ETX(1)
//	DLE(1) STX(1) LEN(2) RSP_PAYLOAD(n) BCC(1) DLE(1) ETX(1)
//
//	Where:
//		LEN = PAYLOAD(n)+BCC(1), big-endian
//
//		CMD_PAYLOAD (command), big-endian
//			CMD(2) SN(1) DATA(n)
//		RSP_PAYLOAD (response)
//			CMD(2) SN(1) SC(2) DATA(n)
//
//			SN = Sequnece Number
//			SC = Status Code, big-endian
//				0x0000: OK, others: ERROR
//
//		BCC = XOR of PAYLOAD
//		DLE = 0x10
//		STX = 0x02
//		ETX = 0x03
//
// Transport Layer Protocol:
//
// COMMAND	->
//		<-	RESPONSE
//
//----------------------------------------------------------------------------

#include "bsp_types.h"
#include "bsp_uart.h"
#include "DEV_AUX.h"

extern	API_AUX		os_AUX_Para[];
extern	API_AUX		os_AUX_ParaBak[];
extern	AUX_DATA	os_AUX_RxDataBuffer[];
extern	AUX_DATA	os_AUX_TxDataBuffer[];
extern	UINT32		os_AUX_State[];
extern	UINT32		os_AUX_Status[];
extern	UINT32		os_AUX_Index[];
extern	UINT32		os_AUX_DataLen[];
extern	UINT32		os_AUX_LRC[];
extern	UINT32		os_AUX_DataCnt[];

//extern	UINT32		os_DLE_Count;

// ---------------------------------------------------------------------------
// FUNCTION: Task manager of AUX data link layer TOB reached.
// INPUT   : port - port number.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_AUX_DataLinkRxTOB_DLESTX2( UINT32 port )
{
	    os_AUX_RxDataBuffer[port].Len = 0;
	    os_AUX_Para[port].RxFlag = FALSE;
	    os_AUX_State[port] = DLE2_STATE_IDLE;
}

// ---------------------------------------------------------------------------
// FUNCTION: Respond ACK to the sender according to DLE protocol.
// INPUT   : pUart -- pointer to the BSP_UART structure.
//           Code  -- ACK or NAK.
// OUTPUT  : none.
// RETURN  : TRUE  - OK.
//           FALSE - device failed or busy.
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkAck_DLE2( BSP_UART *pUart, UINT8 Code )
{
UINT32	i;
UINT32	acks;
UINT8	data[MAX_AUX_ACKS*2];	// max. 8 ACKs


	acks = os_AUX_ParaBak[pUart->UartNum].Acks;	// repetitive count
	
	if( acks == 0 )
	  return( TRUE );
	
	if( Code == NAK )	// 2009-10-30, send only one if NAK
	  acks = 1;
	  
	if( acks > MAX_AUX_ACKS )
	  acks = MAX_AUX_ACKS;
	
	for( i=0; i<acks; i++ )	// DLE + Code
	   {
	   data[i*2+0] = DLE;
	   data[i*2+1] = Code;
	   }
	   
	if( BSP_UART_Write( pUart, data, acks*2 ) == BSP_SUCCESS )
	  return( TRUE );
	else
	  return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: AUX data link layer protocol - STX.
//	     Package Format: DLE STX LEN Data LRC DLE ETX
//			     LRC = XOR(Data)
//	     Acknowledge   : ACK = 0x06, NAK = 0x15 (RFU)
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// ---------------------------------------------------------------------------
/*
UINT32	OS_AUX_DataLinkRx_DLESTX2( BSP_UART *pUart )
{
UINT32	State;
UINT32	count;
UINT32	DataLen = 0;
UINT32	DataCnt = 0;
UINT32	i;
UINT32	result;
UINT8	data;
UINT8	lrc = 0;


START:
	DataLen = 0;
	DataCnt = 0;
	lrc = 0;
	
	result = FALSE;

	if( BSP_UART_GetRxAvail( pUart ) == 0 )	// skip non-RxD events
	  return( result );
	  
	// Startup TOB process
	if( os_AUX_Para[pUart->UartNum].RxFlag == FALSE )
	  {
	  os_AUX_State[pUart->UartNum] = DLE2_STATE_IDLE;
	  os_AUX_Para[pUart->UartNum].Tob = os_AUX_ParaBak[pUart->UartNum].Tob;
	  os_AUX_Para[pUart->UartNum].RxFlag = TRUE;
	  
	  os_AUX_Index[pUart->UartNum] = 0;
	  os_AUX_DataLen[pUart->UartNum] = 0;
	  os_AUX_DataCnt[pUart->UartNum] = 0;
	  os_AUX_LRC[pUart->UartNum] = 0;
	  
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  }

	// retrive state machine
	i = os_AUX_Index[pUart->UartNum];
	DataLen = os_AUX_DataLen[pUart->UartNum];
	DataCnt = os_AUX_DataCnt[pUart->UartNum];
	lrc = os_AUX_LRC[pUart->UartNum];
	State = os_AUX_State[pUart->UartNum];

	// out of range?
	if( DataLen > (pUart->BufferSize) )
	  {
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  
	  os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	  os_AUX_State[pUart->UartNum] = DLE2_STATE_IDLE;
	  
	  while( BSP_UART_Read( pUart, (UINT8 *)&data, 1 ) );	// get rid of all incoming data
	  return( result );
	  }
	

	count = BSP_UART_Read( pUart, &data, 1 );	// read one byte
	     
	     switch( State )
	           {
	           case	DLE2_STATE_IDLE:
	           	
	                if( data == DLE )				// check leading char
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;	// reset final length of package
	                  
	                  DataLen = 0;
	                  DataCnt = 0;
	                  os_AUX_DataLen[pUart->UartNum] = 0;		// reset data length
	                  
	                  lrc = 0;
	                  os_AUX_LRC[pUart->UartNum] = 0;		// reset LRC
	                  
	                  i = 0;					// reset index
	                          
	                  State = DLE2_STATE_STX;
	                  }
	                  
	           	break;
	           	
	           case DLE2_STATE_STX:
	      
	                if( data == STX )				// check leading char
	                  State = DLE2_STATE_LEN_H;
	                else
	                  State = DLE2_STATE_IDLE;
	             
	                break;
	           
	           case DLE2_STATE_LEN_H:
	           	
	           	DataCnt = data*256;
	           	State = DLE2_STATE_LEN_L;

	           	break;
	           	
	           case DLE2_STATE_LEN_L:
	           	
	           	DataCnt += data;

	      		if( DataCnt != 0 )
	      		  {
	      		  DataCnt--;	// excluding BCC
	                  State = DLE2_STATE_DATA;
	                  }
	                else
	                  State = DLE2_STATE_IDLE;			// invalid length
	                  	           	
	           	break;
	           	
	           case	DLE2_STATE_DATA:
	                
	                lrc ^= data;					// check sum
	                
			os_AUX_RxDataBuffer[pUart->UartNum].Data[i] = data;
			
	                DataLen++;					// RX_LEN++
	                i++;						// next byte
			
			if( DataLen == DataCnt )			// end of data field?
			  State = DLE2_STATE_LRC;

	                break;
	           
	           case DLE2_STATE_LRC:

	                if( data == lrc )				// good LRC (ACK will be sent after API reading)
	                  {

//	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = DataLen;	// put LEN & DATA without leading header

//			  OS_AUX_DataLinkAck_DLL( pUart, ACK );		// 2009-10-30
	                  result = TRUE;
	                  }
	                else
	                  {	// bad LRC, NAK to sender (enquire again)
//	                  OS_AUX_DataLinkAck_DLE( pUart, ENQ );
			  result = FALSE;
	                  }
	           	
//	           	os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	           	
	           	if( result )
	           	  State = DLE2_STATE_DLE;
	           	else
	           	  State = DLE2_STATE_IDLE;
	           		           
	                break;

	           case DLE2_STATE_DLE:
	           	
	                if( data == DLE )
	                  State = DLE2_STATE_ETX;
	                else
	                  State = DLE2_STATE_IDLE;
	                  	           	
	           	break;

	           case DLE2_STATE_ETX:
	           	
	                if( data == ETX )
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = DataLen;	// put LEN & DATA without leading header
	                
	                os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	                
	                State = DLE2_STATE_IDLE;
	                  	           	
	           	break;
           	
	           default:
	           
	                break;
	                
	           } // switch()
	     	     
	     os_AUX_Index[pUart->UartNum] = i;		// update data index
	     os_AUX_State[pUart->UartNum] = State;	// update state machine
	     os_AUX_DataLen[pUart->UartNum] = DataLen;	// update data length
	     os_AUX_DataCnt[pUart->UartNum] = DataCnt;	// update data cnt (the expected data LEN)
	     os_AUX_LRC[pUart->UartNum] = lrc;		// update lrc

	goto START;	// PATCH: 2010-09-30, keep processing the queued FIFO data
	     
//	return( result );
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: AUX data link layer protocol - STX.
//	     Package Format: DLE STX LEN Data LRC DLE ETX
//			     LRC = XOR(Data)
//	     Acknowledge   : ACK = 0x06, NAK = 0x15 (RFU)
// INPUT   : pUart
// OUTPUT  : none.
// RETURN  : TRUE  - package received ok
//           FALSE - package not available 
// NOTE	   : Edit by West for while(OS_AUX_RxReady) polling rx used
// ---------------------------------------------------------------------------

UINT32	OS_AUX_DataLinkRx_DLESTX2( BSP_UART *pUart )
{
UINT32	State;
UINT32	count;
UINT32	DataLen = 0;
UINT32	DataCnt = 0;
UINT32	i;
UINT32	result;
UINT8	data;
UINT8	lrc = 0;


START:
	DataLen = 0;
	DataCnt = 0;
	lrc = 0;
	
	result = FALSE;

  
	// Startup TOB process
	if( os_AUX_Para[pUart->UartNum].RxFlag == FALSE )
	  {
	  os_AUX_State[pUart->UartNum] = DLE2_STATE_IDLE;
	  os_AUX_Para[pUart->UartNum].Tob = os_AUX_ParaBak[pUart->UartNum].Tob;
	  os_AUX_Para[pUart->UartNum].RxFlag = TRUE;
	  
	  os_AUX_Index[pUart->UartNum] = 0;
	  os_AUX_DataLen[pUart->UartNum] = 0;
	  os_AUX_DataCnt[pUart->UartNum] = 0;
	  os_AUX_LRC[pUart->UartNum] = 0;
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  }

	// retrive state machine
	i = os_AUX_Index[pUart->UartNum];
	DataLen = os_AUX_DataLen[pUart->UartNum];
	DataCnt = os_AUX_DataCnt[pUart->UartNum];
	lrc = os_AUX_LRC[pUart->UartNum];
	State = os_AUX_State[pUart->UartNum];
	data=os_AUX_RxDataBuffer[pUart->UartNum].Data[i];
	// out of range?
	if( DataLen > (pUart->BufferSize) )
	  {
	  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;
	  
	  os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	  os_AUX_State[pUart->UartNum] = DLE2_STATE_IDLE;
	  
	  while( BSP_UART_Read( pUart, (UINT8 *)&data, 1 ) );	// get rid of all incoming data
	  return( result );
	  }
	     
	     switch( State )
	           {
	           case	DLE2_STATE_IDLE:
	           	
	                if( data == DLE )				// check leading char
	                  {
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = 0;	// reset final length of package
	                  
	                  DataLen = 0;
	                  DataCnt = 0;
	                  os_AUX_DataLen[pUart->UartNum] = 0;		// reset data length
	                  
	                  lrc = 0;
	                  os_AUX_LRC[pUart->UartNum] = 0;		// reset LRC
	                  
	                  i = 0;					// reset index
	                          
	                  State = DLE2_STATE_STX;
	                  }
	                  
	           	break;
	           	
	           case DLE2_STATE_STX:
	      
	                if( data == STX )				// check leading char
	                  State = DLE2_STATE_LEN_H;
	                else
	                  State = DLE2_STATE_IDLE;
	             
	                break;
	           
	           case DLE2_STATE_LEN_H:
	           	
	           	DataCnt = data*256;
	           	State = DLE2_STATE_LEN_L;

	           	break;
	           	
	           case DLE2_STATE_LEN_L:
	           	
	           	DataCnt += data;

	      		if( DataCnt != 0 )
	      		  {
	      		  DataCnt--;	// excluding BCC
	                  State = DLE2_STATE_DATA;
	                  }
	                else
	                  State = DLE2_STATE_IDLE;			// invalid length
	                  	           	
	           	break;
	           	
	           case	DLE2_STATE_DATA:
	                
	                lrc ^= data;					// check sum
	                
			os_AUX_RxDataBuffer[pUart->UartNum].Data[i] = data;
			
	                DataLen++;					// RX_LEN++
	                i++;						// next byte
			
			if( DataLen == DataCnt )			// end of data field?
			  State = DLE2_STATE_LRC;

	                break;
	           
	           case DLE2_STATE_LRC:

	                if( data == lrc )				// good LRC (ACK will be sent after API reading)
	                  {

//	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = DataLen;	// put LEN & DATA without leading header

//			  OS_AUX_DataLinkAck_DLL( pUart, ACK );		// 2009-10-30
	                  result = TRUE;
					  return( result );
	                  }
	                else
	                  {	// bad LRC, NAK to sender (enquire again)
//	                  OS_AUX_DataLinkAck_DLE( pUart, ENQ );
			  result = FALSE;
	                  }
	           	
//	           	os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	           	
	           	if( result )
	           	  State = DLE2_STATE_DLE;
	           	else
	           	  State = DLE2_STATE_IDLE;
	           		           
	                break;

	           case DLE2_STATE_DLE:
	           	
	                if( data == DLE )
	                  State = DLE2_STATE_ETX;
	                else
	                  State = DLE2_STATE_IDLE;
	                  	           	
	           	break;

	           case DLE2_STATE_ETX:
	           	
	                if( data == ETX )
	                  os_AUX_RxDataBuffer[pUart->UartNum].Len = DataLen;	// put LEN & DATA without leading header
	                
	                os_AUX_Para[pUart->UartNum].RxFlag = FALSE;
	                
	                State = DLE2_STATE_IDLE;
	                  	           	
	           	break;
           	
	           default:
	           
	                break;
	                
	           } // switch()
	     	     
	     os_AUX_Index[pUart->UartNum] = i;		// update data index
	     os_AUX_State[pUart->UartNum] = State;	// update state machine
	     os_AUX_DataLen[pUart->UartNum] = DataLen;	// update data length
	     os_AUX_DataCnt[pUart->UartNum] = DataCnt;	// update data cnt (the expected data LEN)
	     os_AUX_LRC[pUart->UartNum] = lrc;		// update lrc

	//goto START;	// PATCH: 2010-09-30, keep processing the queued FIFO data
	     
	return( result );
}
// ---------------------------------------------------------------------------
// FUNCTION: Wait for ACK from the sender according to DLL protocol.
// INPUT   : pUart -- pointer to the BSP_UART structure.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if	0
void	OS_AUX_DataLinkWaitAck_DLESTX( BSP_UART *pUart )
{
UINT8	data1;
UINT8	data2;

	if( os_AUX_State[pUart->UartNum] == DLE_STATE_WAIT_ACK )
	  {
	  if( pUart->Avail >= 2 )
	    {
	    os_AUX_Para[pUart->UartNum].TxFlag = FALSE;	// disable TimeOut task
	    
	    BSP_UART_Read( pUart, (UINT8 *)&data1, 1 );
	    BSP_UART_Read( pUart, (UINT8 *)&data2, 1 );
	    
	    if( data1 == DLE )
	      {
	      switch( data2 )
	      	  {
	      	  case ACK:
	      	  
	      	         OS_AUX_DataLinkAck_DLE( pUart, ENQ );		// enquire response
	      	         
	      	         os_AUX_State[pUart->UartNum] = DLE_STATE_IDLE;
	      	         os_AUX_Status[pUart->UartNum] = AUX_STATUS_FREE;
	      	       
	      	       break;
	      	       
	      	  case NAK:
	      	  	    	  	      	       
	      	       if( OS_AUX_ResendPackage( pUart ) == FALSE )	// resend last package
	      	         os_AUX_Status[pUart->UartNum] = AUX_STATUS_ERROR;
	      	         
	      	       break;
	      	       
	      	  default:
	      	  
	      	       os_AUX_Status[pUart->UartNum] = AUX_STATUS_ERROR;
	      	       
	      	       break;
	      	  }
	      }
	    }
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: AUX data transmission by link layer format - DLESTX2.
//
//	DLE(1) STX(1) LEN(2) CMD_PAYLOAD(n) BCC(1) DLE(1) ETX(1)
//	DLE(1) STX(1) LEN(2) RSP_PAYLOAD(n) BCC(1) DLE(1) ETX(1)
//
//	Where:
//		LEN = PAYLOAD(n)+BCC(1)
//
//		CMD_PAYLOAD (command)
//			CMD(2) SN(1) DATA(n)
//		RSP_PAYLOAD (response)
//			CMD(2) SN(1) SC(2) DATA(n)
//
//			SN = Sequnece Number
//			SC = Status Code
//
//		BCC = XOR of PAYLOAD
//		DLE = 0x10
//		STX = 0x02
//		ETX = 0x03
//
// INPUT   : pUart
//           pData  - (LL-V)
// OUTPUT  : none.
// RETURN  : TRUE  - package transmitted ok.
//           FALSE - package transmitted failed. 
// ---------------------------------------------------------------------------
UINT32	OS_AUX_DataLinkTx_DLESTX2( BSP_UART *pUart, UINT8 *pData )
{
UINT32	i;
UINT32	index;
UINT32	len;
UINT8	len_l, len_h;
UINT8	lrc;

	
  	// --- Build package ---
	index = 0;
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = DLE;	// DLE
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = STX;	// STX

	len_l = *pData++;
	len_h = *pData++;
	len = len_l + (len_h*256);	// total LEN
	
	len++;	// including BCC
	
	len_l = len & 0x00FF;
	len_h = (len & 0xFF00) >> 8;
	
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = len_h;	// LEN_H
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = len_l;	// LEN_L
	
	lrc = 0;	
	for( i=0; i<len-1; i++ )
	   {
	   lrc ^= *pData;
	   os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = *pData++;
	   }
	
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = lrc;	// BCC
	
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = DLE;	// DLE
	os_AUX_TxDataBuffer[pUart->UartNum].Data[index++] = ETX;	// ETX
	
	os_AUX_TxDataBuffer[pUart->UartNum].Len = index;		// total package size
	
	// --- Send package to UARTx ---
	os_AUX_Para[pUart->UartNum].Resend = os_AUX_ParaBak[pUart->UartNum].Resend;	// restore RESEND cnt
	
	os_AUX_State[pUart->UartNum] = DLE2_STATE_IDLE;					// change state to IDLE
	
	return( OS_AUX_SendPackage( pUart ) );
}
