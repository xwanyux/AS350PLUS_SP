#ifndef _JCB_KERNEL_DEFINE_H_
#define _JCB_KERNEL_DEFINE_H_

#include "POSAPI.h"
#include "Define.h"
#include "Datastructure.h"
#include "ODA_Record.h"

//For Torn Recover Txn, Add or Remove Tags
#define JCB_GPO_Tag			0x01
#define JCB_RR_Tag			0x02
#define JCB_GenAC_Tag		0x04
#define JCB_GenAC2_Tag		0x08

//Pack Online Data
#define JCB_TxnApprovalOrOnline 0
#define JCB_TxnDecline			1

//Terminal Action Analysis
#define JCB_TAA_Denial		0				
#define JCB_TAA_Online		1
#define JCB_TAA_Default		2

//Terminal Action Analysis Result
#define JCB_TAAR_AAC		0
#define JCB_TAAR_TC			1
#define JCB_TAAR_ARQC		2

//JCB_Process respconse code
#define JCB_FAIL						0
#define JCB_SUCCESS						1
#define JCB_TornTransactionRecovery		2
#define JCB_IssuerUpdate				3
#define JCB_SelectNext					16	// conflict with SW6986
#define JCB_TryAnother					5
#define JCB_EndApplication				6
#define JCB_EndAppWithRestart			7
#define JCB_EndAppWithRestartComm		8
#define JCB_Decline						9
#define JCB_TAAR_Denial					10
#define JCB_TAAR_Online					11
#define JCB_TAAR_Default				12
#define JCB_Recover_Break_1				13
#define JCB_Recover_Break_2				14
#define JCB_Recover_Break_3				15
#define JCB_EndApplicationWithARC		17

//JCB Transaction Mode
#define JCB_Txn_Undefined				0x00U
#define JCB_Txn_EMV						0x01U
#define JCB_Txn_Mag						0x02U
#define JCB_Txn_Legacy					0x04U
#define JCB_Txn_ISUSupport				0x08U


//20140620 add
extern UCHAR JCB_Issue_Update_Flag;

//entrypoint flag
extern UCHAR JCB_JCB_FCI_Error;

extern UCHAR JCB_CVM_Result;
extern UCHAR JCB_TAA_Result;

//Implementation Option
extern UCHAR JCB_TxnMode;			// Undefine Mode 00 , EMV Mode 01, Magstripe Mode 02, Legacy Mode 04

//extern UCHAR JCB_Snd_Buf[DEP_BUFFER_SIZE_SEND];
extern UCHAR *JCB_Snd_Buf;
extern UINT JCB_Snd_Buf_Len;

//extern UCHAR JCB_Rcv_Buf[DEP_BUFFER_SIZE_RECEIVE];
extern UCHAR *JCB_Rcv_Buf;
extern UINT JCB_Rcv_Buf_Len;

//20140527 add for CDA process
//extern UCHAR JCB_GenAC_Response[512];
extern UCHAR *JCB_GenAC_Response;
extern UINT JCB_GenAC_ResponseLen;

extern UCHAR JCB_Kernel_Support;	//EMV Mode 0x01, Magstripe Mode 0x02, Legacy Mode 0x04, Issuer Update 0x08

extern JCB_SKCP JCB_Static_Parameter;
extern JCB_DTP	JCB_Dynamic_Parameter;	
extern JCB_Recover_Context JCB_ReContext_Par;
extern JCB_Online_Txn_Context JCB_Online_Context;
//20140527 add
extern JCB_ExpFile JCB_ExceptionFile[10];

//20140603
extern JCB_RevoList JCB_RevocationList[10];

//20140604 add
extern UCHAR JCB_TAC_Decline[5];
extern UCHAR JCB_TAC_Online[5];
extern UCHAR JCB_TAC_Default[5];

//Check SW1 SW2
#define JCB_SW9000	0x01U
#define JCB_SW6984	0x02U
#define JCB_SW6985	0x03U
#define JCB_SW6986	0x04U
#define JCB_SW6300	0x05U
#define JCB_SW6A80	0x06U

//CDA Process
#define JCB_OFDA_Fail  				0
#define JCB_Rid_Len 				5
#define JCB_CAPK_REVOC_LEN			9
#define JCB_MAX_CAPK_REVOC_CNT		10


//----------------------------------------------------------------------------
//      JCB Record Data (for SFI = 1~30)
//      FORMAT: ODA[1] DATA[269]
//                     70-L-V (excluding SW1 SW2)
//              ODA: bit8 = 1 : record for offline data authen.
//                   bit6-7   : RFU
//                   bit1-5   : SFI
//              L  : length 1~3 bytes.
//----------------------------------------------------------------------------
#define JCB_REC_Buf_Size  				ODA_BUFFER_SIZE_RECORD	//30 * 270

//Read Record Buffer
#define ADDR_JCB_REC_START              oda_bufRecord

#define MAX_JCB_REC_CNT                 ODA_RECORD_NUMBER	//EMV 42a 20130318
#define JCB_REC_LEN                     ODA_RECORD_SIZE

#define ADDR_JCB_REC_01                 ADDR_JCB_REC_START+JCB_REC_LEN*0
#define ADDR_JCB_REC_02                 ADDR_JCB_REC_START+JCB_REC_LEN*1
#define ADDR_JCB_REC_03                 ADDR_JCB_REC_START+JCB_REC_LEN*2
#define ADDR_JCB_REC_04                 ADDR_JCB_REC_START+JCB_REC_LEN*3
#define ADDR_JCB_REC_05                 ADDR_JCB_REC_START+JCB_REC_LEN*4
#define ADDR_JCB_REC_06                 ADDR_JCB_REC_START+JCB_REC_LEN*5
#define ADDR_JCB_REC_07                 ADDR_JCB_REC_START+JCB_REC_LEN*6
#define ADDR_JCB_REC_08                 ADDR_JCB_REC_START+JCB_REC_LEN*7
#define ADDR_JCB_REC_09                 ADDR_JCB_REC_START+JCB_REC_LEN*8
#define ADDR_JCB_REC_10                 ADDR_JCB_REC_START+JCB_REC_LEN*9
#define ADDR_JCB_REC_11                 ADDR_JCB_REC_START+JCB_REC_LEN*10
#define ADDR_JCB_REC_12                 ADDR_JCB_REC_START+JCB_REC_LEN*11
#define ADDR_JCB_REC_13                 ADDR_JCB_REC_START+JCB_REC_LEN*12
#define ADDR_JCB_REC_14                 ADDR_JCB_REC_START+JCB_REC_LEN*13
#define ADDR_JCB_REC_15                 ADDR_JCB_REC_START+JCB_REC_LEN*14
#define ADDR_JCB_REC_16                 ADDR_JCB_REC_START+JCB_REC_LEN*15
#define ADDR_JCB_REC_17                 ADDR_JCB_REC_START+JCB_REC_LEN*16
#define ADDR_JCB_REC_18                 ADDR_JCB_REC_START+JCB_REC_LEN*17
#define ADDR_JCB_REC_19                 ADDR_JCB_REC_START+JCB_REC_LEN*18
#define ADDR_JCB_REC_20                 ADDR_JCB_REC_START+JCB_REC_LEN*19
#define ADDR_JCB_REC_21                 ADDR_JCB_REC_START+JCB_REC_LEN*20
#define ADDR_JCB_REC_22                 ADDR_JCB_REC_START+JCB_REC_LEN*21
#define ADDR_JCB_REC_23                 ADDR_JCB_REC_START+JCB_REC_LEN*22
#define ADDR_JCB_REC_24                 ADDR_JCB_REC_START+JCB_REC_LEN*23
#define ADDR_JCB_REC_25                 ADDR_JCB_REC_START+JCB_REC_LEN*24
#define ADDR_JCB_REC_26                 ADDR_JCB_REC_START+JCB_REC_LEN*25
#define ADDR_JCB_REC_27                 ADDR_JCB_REC_START+JCB_REC_LEN*26
#define ADDR_JCB_REC_28                 ADDR_JCB_REC_START+JCB_REC_LEN*27
#define ADDR_JCB_REC_29                 ADDR_JCB_REC_START+JCB_REC_LEN*28
#define ADDR_JCB_REC_30                 ADDR_JCB_REC_START+JCB_REC_LEN*29

#define ADDR_JCB_REC_END                ADDR_JCB_REC_START+JCB_REC_LEN*MAX_JCB_REC_CNT

#endif
