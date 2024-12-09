#ifndef BURNFLASH_H_
#define BURNFLASH_H_

#include "bsp_types.h"
#include "POSAPI.h"


/*
 * Function Prototypes.
 */

extern UCHAR EraseFlashArea (void);
extern UCHAR api_flash_write( ULONG	* BaseAddr,  UCHAR * pSource,  ULONG Len);
//extern UINT32 EraseSector (	UINT32 );
//extern UINT32 IdentifyFlash16 ( UINT32	);

#endif /*BURNFLASH_H_*/
