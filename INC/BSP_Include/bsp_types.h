/*
 * Copyright 2006, ZiLOG Inc.
 * All Rights Reserved
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ZiLOG Inc., and might
 * contain proprietary, confidential and trade secret information of
 * ZiLOG, our partners and parties from which this code has been licensed.
 * 
 * The contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of ZiLOG Inc.
 */

#ifndef _BSP_TYPES_H_
#define _BSP_TYPES_H_

/*
 * The BSP assumes the following base types are available from the underlying 
 * system (Nucleus):
 */
 
//20130828 test
/*
typedef unsigned char					UINT8;
typedef unsigned short int				UINT16;
typedef unsigned long int				UINT32;
typedef unsigned long long int 			UINT64;
typedef signed char  					INT8;
typedef signed short int				INT16;
typedef signed long int	   				INT32;
typedef signed long long int 			INT64;
*/
#ifdef BSP_OS_IS_NO_OS
	typedef unsigned char					UINT8;
	typedef unsigned short int				UINT16;
	typedef unsigned long int				UINT32;
	typedef unsigned long long int			UINT64;

	typedef signed char  					INT8;
	typedef signed short int				INT16;
	typedef signed long int	   				INT32;
	typedef signed long long int 			INT64;
#endif

#ifdef BSP_OS_IS_NOS
	typedef unsigned char					UINT8;
	typedef unsigned short int				UINT16;
	typedef unsigned long int				UINT32;
	typedef unsigned long long int 			UINT64;

	typedef signed char  					INT8;
	typedef signed short int				INT16;
	typedef signed long int	   				INT32;
	typedef signed long long int 			INT64;
#endif

#ifdef BSP_OS_IS_NUCLEUS
	#include "plus/nucleus.h"
	typedef unsigned long long int 			UINT64;
	typedef signed long long int 			INT64;
#endif

#ifdef BSP_OS_IS_LINUX
	typedef unsigned char					UINT8;
	typedef unsigned short					UINT16;
	typedef unsigned int					UINT32;
	typedef unsigned long long int 			UINT64;

	typedef signed char						INT8;
	typedef signed short					INT16;
	typedef signed int						INT32;
	typedef signed long long int 			INT64;
#endif

#include "za9_defines.h"
#ifndef BSP_OS_IS_LINUX
	#include <string.h>
#endif


typedef UINT32								BSP_BOOL;
typedef void *								BSP_HANDLE;
typedef INT32								BSP_STATUS;

#define TRUE								1
#define FALSE								0

#define NULLPTR								(void *) 0

/*
 * BSP Status Codes
 */
#define BSP_SUCCESS							0
#define BSP_FAILURE							-1
#define BSP_BUSY							-2
#define BSP_INVALID_PARAMETER				-3
/*
 * Macros used to access ZA9 registers.
 */
#ifdef BSP_OS_IS_LINUX
	#define BSP_WR32( Reg, Data )	( (*((volatile UINT32 *)((Reg) - 0x01000000)) ) = (UINT32)(Data) )
	#define BSP_RD32( Reg )        ( *((volatile UINT32 *)((Reg) - 0x01000000)) )

	#define BSP_WR16( Reg, Data )	( (*((volatile UINT16 *)((Reg) - 0x01000000)) ) = (UINT16)(Data) )
	#define BSP_RD16( Reg )        ( *((volatile UINT16 *)((Reg) - 0x01000000)) )
#else
	#define BSP_WR32( Reg, Data )	( (*((volatile UINT32 *)(Reg)) ) = (UINT32)(Data) )
	#define BSP_RD32( Reg )        ( *((volatile UINT32 *)(Reg)) )

	#define BSP_WR16( Reg, Data )	( (*((volatile UINT16 *)(Reg)) ) = (UINT16)(Data) )
	#define BSP_RD16( Reg )        ( *((volatile UINT16 *)(Reg)) )
#endif


#endif
