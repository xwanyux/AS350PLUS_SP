#include "ODA_Record.h"

#define VAP_Try_Another			0x33U
#define	VAP_Select_Next			0x34U
#define	VAP_Try_Again			0x35U
#define VAP_DDA_FAIL			0x36U

// Return value returned form check sw1 sw2 
#define VAP_SW9000	0x01U
#define VAP_SW6984	0x02U
#define VAP_SW6985	0x03U
#define VAP_SW6986	0x04U

#define VAP_OFDA_Fail  				0
#define VAP_Rid_Len 				5
#define VAP_CAPK_REVOC_LEN			9
#define VAP_MAX_CAPK_REVOC_CNT		10

//----------------------------------------------------------------------------
//      VGL Record Data (for SFI = 1~30)
//      FORMAT: ODA[1] DATA[269]
//                     70-L-V (excluding SW1 SW2)
//              ODA: bit8 = 1 : record for offline data authen.
//                   bit6-7   : RFU
//                   bit1-5   : SFI
//              L  : length 1~3 bytes.
//----------------------------------------------------------------------------
#define VAP_REC_Buf_Size  				ODA_BUFFER_SIZE_RECORD	//30 * 270

#define ADDR_VAP_REC_START              oda_bufRecord

#define MAX_VAP_REC_CNT                 ODA_RECORD_NUMBER	//EMV 42a 20130318
#define VAP_REC_LEN                     ODA_RECORD_SIZE

#define ADDR_VAP_REC_01                 ADDR_VAP_REC_START+VAP_REC_LEN*0
#define ADDR_VAP_REC_02                 ADDR_VAP_REC_START+VAP_REC_LEN*1
#define ADDR_VAP_REC_03                 ADDR_VAP_REC_START+VAP_REC_LEN*2
#define ADDR_VAP_REC_04                 ADDR_VAP_REC_START+VAP_REC_LEN*3
#define ADDR_VAP_REC_05                 ADDR_VAP_REC_START+VAP_REC_LEN*4
#define ADDR_VAP_REC_06                 ADDR_VAP_REC_START+VAP_REC_LEN*5
#define ADDR_VAP_REC_07                 ADDR_VAP_REC_START+VAP_REC_LEN*6
#define ADDR_VAP_REC_08                 ADDR_VAP_REC_START+VAP_REC_LEN*7
#define ADDR_VAP_REC_09                 ADDR_VAP_REC_START+VAP_REC_LEN*8
#define ADDR_VAP_REC_10                 ADDR_VAP_REC_START+VAP_REC_LEN*9

#define ADDR_VAP_REC_11                 ADDR_VAP_REC_START+VAP_REC_LEN*10
#define ADDR_VAP_REC_12                 ADDR_VAP_REC_START+VAP_REC_LEN*11
#define ADDR_VAP_REC_13                 ADDR_VAP_REC_START+VAP_REC_LEN*12
#define ADDR_VAP_REC_14                 ADDR_VAP_REC_START+VAP_REC_LEN*13
#define ADDR_VAP_REC_15                 ADDR_VAP_REC_START+VAP_REC_LEN*14
#define ADDR_VAP_REC_16                 ADDR_VAP_REC_START+VAP_REC_LEN*15
#define ADDR_VAP_REC_17                 ADDR_VAP_REC_START+VAP_REC_LEN*16
#define ADDR_VAP_REC_18                 ADDR_VAP_REC_START+VAP_REC_LEN*17
#define ADDR_VAP_REC_19                 ADDR_VAP_REC_START+VAP_REC_LEN*18
#define ADDR_VAP_REC_20                 ADDR_VAP_REC_START+VAP_REC_LEN*19

#define ADDR_VAP_REC_21                 ADDR_VAP_REC_START+VAP_REC_LEN*20
#define ADDR_VAP_REC_22                 ADDR_VAP_REC_START+VAP_REC_LEN*21
#define ADDR_VAP_REC_23                 ADDR_VAP_REC_START+VAP_REC_LEN*22
#define ADDR_VAP_REC_24                 ADDR_VAP_REC_START+VAP_REC_LEN*23
#define ADDR_VAP_REC_25                 ADDR_VAP_REC_START+VAP_REC_LEN*24
#define ADDR_VAP_REC_26                 ADDR_VAP_REC_START+VAP_REC_LEN*25
#define ADDR_VAP_REC_27                 ADDR_VAP_REC_START+VAP_REC_LEN*26
#define ADDR_VAP_REC_28                 ADDR_VAP_REC_START+VAP_REC_LEN*27
#define ADDR_VAP_REC_29                 ADDR_VAP_REC_START+VAP_REC_LEN*28
#define ADDR_VAP_REC_30                 ADDR_VAP_REC_START+VAP_REC_LEN*29

#define ADDR_VAP_REC_END                ADDR_VAP_REC_START+VAP_REC_LEN*MAX_VAP_REC_CNT

