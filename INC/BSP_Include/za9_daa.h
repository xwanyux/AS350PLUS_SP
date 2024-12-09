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
#ifndef _ZA9_DAA_H_
#define _ZA9_DAA_H_

#include "za9_defines.h"


/*
 * DAA register Offsets
 */
#define DAA_CTRL_REG        (DAA_BASE|0x0000)
#define DAA_STA_REG         (DAA_BASE|0x0004)
#define DAA_FIFO_REG        (DAA_BASE|0x0008)
#define DAA_SSD_REG_BASE    (DAA_BASE|0x0200)


/*
 * DAA_CTRL_REG Bit definitions
 */
#define DAA_EN          	BIT0
#define DAA_FIFOLVL1    	(0<<1)
#define DAA_FIFOLVL2    	(1<<1)
#define DAA_FIFOLVL3    	(2<<1)
#define DAA_FIFOLVL4    	(3<<1)
#define DAA_FIFO_RST			BIT5
#define DAA_SOFT_RST			BIT6



/*
 * The Conexant Soft Modem (ESF) processes 128 16-bit samples every 8ms per DMA
 * channel (1 for Rx 1 for Tx).  Each DMA channel requires a buffer large enough
 * to hold 16 samples (total buffer size per channel is 4kB).  These buffers
 * must be created in non-cached memory.
 */
#define DAAXFERSAMPS 	128
#define DAAXFERBYTES 	(DAAXFERSAMPS*2)
#define DAAXFERBLKS  	16



#endif 

