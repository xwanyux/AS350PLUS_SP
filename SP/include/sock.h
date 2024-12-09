#ifndef _SOCK_H_
#define _SOCK_H_
#include "bsp_types.h"
#define Host "127.0.0.1"
#define PortNumber 8000
#define MaxConnects 3
#define BuffSize 2400
#define ConversationLen 1
typedef struct OS_IPCSOCKET_HEADER_S
{
	UINT8			PID;					// psDEV id
	UINT8			Func_num;				// API function numbers
	UINT8			Func_input_num;			// Input arguments number
	UINT8			Message_status;			// 1 for first message
                                            // 2 for continued message
                                            // 0 for finished message
	UINT32			ArgsTotalSize;			// Sum of all arguments size
} __attribute__((packed)) OS_IPCSOCKET_HEADER;

typedef struct OS_IPCSOCKET_BODY_S
{
	UINT8			PID;					// psDEV id
	UINT8			Func_num;				// API function numbers
	UINT8			Func_input_num;			// Input arguments number
	UINT8			Message_status;			// 1 for first message
                                            // 2 for continued message
                                            // 0 for finished message
	//UINT8			*Msg;			        // Message buffer pointer.[4B Len]+[n Msg]+[4B Len]+[n Msg]+...
} __attribute__((packed)) OS_IPCSOCKET_BODY;

typedef struct OS_IPCSOCKET_RETURN_S
{
	UINT8			PID;					// psDEV id
	UINT8			Func_num;				// API function numbers
	UINT8			Func_input_num;			// Input arguments number
	UINT8			Message_status;			// 1 for first message
                                            // 2 for continued message
                                            // 0 for finished message
	UINT8			Retval;			        // 
} __attribute__((packed)) OS_IPCSOCKET_RETURN;
#endif