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
#ifndef _BSP_MEM_H_
#define _BSP_MEM_H_

#include "bsp_types.h"



/*
 * The BSP includes a simple memory manager that controls 2 memory pools.
 * The first pool is used to allocate memory from cacheable memory.
 * the second pool is used to allocate memory from memory that is not cached.
 * 
 * If the BSP memory manager is used, make sure the system initialization code
 * configures the MMU to to mark the DMA_MEMORY as non-cacheable.
 *
 * Also ensure the scatter file used for the project does not direct the linker
 * to locate code or data within either of these memory regions.
 *
 * If the BSP is ported to an actual operating system, the BSP memory 
 * allocation routines should be directed to go through the underlying kernel.
 */


 
 /*
 * System Memory
 * Default configuration is to allocate 2MB of cacheable memory.
 * The location of this memory is determined by the location of the
 * 	UINT8 SysMemory[ SYS_MEMORY_SIZE ];
 * array defined in os_mem.c.  This memory block will be within the BSS
 * section of the memory map.
 */
#define SYS_MEMORY_SIZE		( 1024 * 2048 )



/*
 * DMA Memory
 * The default configuration is to allocate 32 kB of non-cacheable
 * memory from internal SRAM at the memory address determined by
 * DMA_MEMORY_BASE.  This region can be moved/ resized as appropraie,
 * but the memory must always reside in the same physical memory device.
 */
//#define DMA_MEMORY_BASE		0x8000
//#define DMA_MEMORY_BASE		0x20700000		// primitive BSP
#define DMA_MEMORY_BASE			0x20D00000		// PATCH: 2009-11-17
//#define DMA_MEMORY_BASE		0x20A00000		// PATCH: 2009-12-09, for global DSS
#define DMA_MEMORY_SIZE		( 1024 * 32 )

#define INT_MEMORY_BASE		0xDFF0C000
#define INT_MEMORY_SIZE		( 1024 * 16 )

#define SEC_MEMORY_BASE		SEC_SRAM_BASE
#define SEC_MEMORY_SIZE		( 1024 * 4 )


/*
 * Function Prototypes
 */
extern BSP_STATUS BSP_MEM_Init( void );
extern BSP_HANDLE BSP_Malloc( UINT32 Size );
extern void BSP_Free( BSP_HANDLE pData );

#ifdef BSP_OS_IS_LINUX
	extern UINT32 BSP_DmaMemFromVirt( UINT32 VirtAddress );
#endif
extern BSP_HANDLE BSP_DmaMalloc( UINT32 Size );
extern void BSP_DmaFree( BSP_HANDLE pData );

extern BSP_HANDLE BSP_IntMalloc( UINT32 Size );
extern void BSP_IntFree( BSP_HANDLE pData );

extern BSP_HANDLE BSP_SecMalloc( UINT32 Size );
extern void BSP_SecFree( BSP_HANDLE pData );

extern void MemMgrInit(	UINT32 Base, UINT32 Len );
extern void * getmem( void * pPool,	UINT32 NumBytes );
extern void freemem( void * pMem );



/*
 * Memory pools
 */
extern UINT8 					SysMemory[];
extern UINT8				 * pDmaMemory;
extern UINT8				 * pIntMemory;
extern UINT8				 * pSecMemory;


/*
 * Memory managemenr macros definition for memory allocation
 */
#if defined BSP_OS_IS_NO_OS || defined BSP_OS_IS_NOS
	#define BSP_Malloc( Size )		getmem( SysMemory, (Size) )
	#define BSP_Free( pMem )		freemem( (pMem) )
	#define BSP_DmaMalloc( Size )	getmem( pDmaMemory, (Size) )
	#define BSP_DmaFree( pMem )	freemem( (pMem) )
	#define BSP_IntMalloc( Size )	getmem( pIntMemory, (Size) )
	#define BSP_IntFree( pMem )	freemem( (pMem) )
	#define BSP_SecMalloc( Size )	getmem( pSecMemory, (Size) )
	#define BSP_SecFree( pMem )	freemem( (pMem) )
#endif



#endif

