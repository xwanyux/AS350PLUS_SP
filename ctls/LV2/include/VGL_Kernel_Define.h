#include "ODA_Record.h"

#define VGL_qVSDC_Process 		0x31U
#define VGL_MSD_Process 		0x32U
#define VGL_Try_Another			0x33U
#define	VGL_Select_Next			0x34U
#define	VGL_Try_Again			0x35U
#define VGL_Switch_Interface	0x36U
#define VGL_GPO_Terminate		0x37U
//Issuer Update
#define IU_SUCCESS				0x40U
#define IU_FAIL					0x41U
#define IU_NotReady				0x42U
#define IU_OutOfService			0x43U

#define VGL_OFDA_Try_Another	0x51U

// Return value returned form check sw1 sw2 
#define VGL_SW9000	0x01U
#define VGL_SW6984	0x02U
#define VGL_SW6985	0x03U
#define VGL_SW6986	0x04U
#define VGL_SW6300	0x05U
#define VGL_SW6A80	0x06U

#define VGL_OFDA_Fail  				0
#define VGL_Rid_Len 				5
#define VGL_CAPK_REVOC_LEN			9
#define VGL_MAX_CAPK_REVOC_CNT		10

//----------------------------------------------------------------------------
//      VGL Record Data (for SFI = 1~30)
//      FORMAT: ODA[1] DATA[269]
//                     70-L-V (excluding SW1 SW2)
//              ODA: bit8 = 1 : record for offline data authen.
//                   bit6-7   : RFU
//                   bit1-5   : SFI
//              L  : length 1~3 bytes.
//----------------------------------------------------------------------------
#define VGL_REC_Buf_Size				ODA_BUFFER_SIZE_RECORD	//30 * 270

#define ADDR_VGL_REC_START              oda_bufRecord

#define MAX_VGL_REC_CNT                 ODA_RECORD_NUMBER	//EMV 42a 20130318
#define VGL_REC_LEN                     ODA_RECORD_SIZE

#define ADDR_VGL_REC_01                 ADDR_VGL_REC_START+VGL_REC_LEN*0
#define ADDR_VGL_REC_02                 ADDR_VGL_REC_START+VGL_REC_LEN*1
#define ADDR_VGL_REC_03                 ADDR_VGL_REC_START+VGL_REC_LEN*2
#define ADDR_VGL_REC_04                 ADDR_VGL_REC_START+VGL_REC_LEN*3
#define ADDR_VGL_REC_05                 ADDR_VGL_REC_START+VGL_REC_LEN*4
#define ADDR_VGL_REC_06                 ADDR_VGL_REC_START+VGL_REC_LEN*5
#define ADDR_VGL_REC_07                 ADDR_VGL_REC_START+VGL_REC_LEN*6
#define ADDR_VGL_REC_08                 ADDR_VGL_REC_START+VGL_REC_LEN*7
#define ADDR_VGL_REC_09                 ADDR_VGL_REC_START+VGL_REC_LEN*8
#define ADDR_VGL_REC_10                 ADDR_VGL_REC_START+VGL_REC_LEN*9

#define ADDR_VGL_REC_11                 ADDR_VGL_REC_START+VGL_REC_LEN*10
#define ADDR_VGL_REC_12                 ADDR_VGL_REC_START+VGL_REC_LEN*11
#define ADDR_VGL_REC_13                 ADDR_VGL_REC_START+VGL_REC_LEN*12
#define ADDR_VGL_REC_14                 ADDR_VGL_REC_START+VGL_REC_LEN*13
#define ADDR_VGL_REC_15                 ADDR_VGL_REC_START+VGL_REC_LEN*14
#define ADDR_VGL_REC_16                 ADDR_VGL_REC_START+VGL_REC_LEN*15
#define ADDR_VGL_REC_17                 ADDR_VGL_REC_START+VGL_REC_LEN*16
#define ADDR_VGL_REC_18                 ADDR_VGL_REC_START+VGL_REC_LEN*17
#define ADDR_VGL_REC_19                 ADDR_VGL_REC_START+VGL_REC_LEN*18
#define ADDR_VGL_REC_20                 ADDR_VGL_REC_START+VGL_REC_LEN*19

#define ADDR_VGL_REC_21                 ADDR_VGL_REC_START+VGL_REC_LEN*20
#define ADDR_VGL_REC_22                 ADDR_VGL_REC_START+VGL_REC_LEN*21
#define ADDR_VGL_REC_23                 ADDR_VGL_REC_START+VGL_REC_LEN*22
#define ADDR_VGL_REC_24                 ADDR_VGL_REC_START+VGL_REC_LEN*23
#define ADDR_VGL_REC_25                 ADDR_VGL_REC_START+VGL_REC_LEN*24
#define ADDR_VGL_REC_26                 ADDR_VGL_REC_START+VGL_REC_LEN*25
#define ADDR_VGL_REC_27                 ADDR_VGL_REC_START+VGL_REC_LEN*26
#define ADDR_VGL_REC_28                 ADDR_VGL_REC_START+VGL_REC_LEN*27
#define ADDR_VGL_REC_29                 ADDR_VGL_REC_START+VGL_REC_LEN*28
#define ADDR_VGL_REC_30                 ADDR_VGL_REC_START+VGL_REC_LEN*29

#define ADDR_VGL_REC_END                ADDR_VGL_REC_START+VGL_REC_LEN*MAX_VGL_REC_CNT


