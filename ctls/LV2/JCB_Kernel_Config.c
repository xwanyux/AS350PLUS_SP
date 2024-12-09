#include "Define.h"
#include "Datastructure.h"
#include "JCB_Kernel_Define.h"

//20140620 add
UCHAR JCB_Issue_Update_Flag = FALSE;

//entrypoint flag
UCHAR JCB_JCB_FCI_Error;

UCHAR JCB_CVM_Result;
UCHAR JCB_TAA_Result;

//Implementation Option
UCHAR JCB_TxnMode;			// Undefine Mode 00 , EMV Mode 01, Magstripe Mode 02, Legacy Mode 04

//UCHAR JCB_Snd_Buf[DEP_BUFFER_SIZE_SEND] = {0};
UCHAR *JCB_Snd_Buf;
UINT JCB_Snd_Buf_Len = 0;

//UCHAR JCB_Rcv_Buf[DEP_BUFFER_SIZE_RECEIVE] = {0};
UCHAR *JCB_Rcv_Buf;
UINT JCB_Rcv_Buf_Len = 0;

//20140527 add for CDA process
//UCHAR JCB_GenAC_Response[512] = {0};
UCHAR *JCB_GenAC_Response;
UINT JCB_GenAC_ResponseLen = 0;

UCHAR JCB_Kernel_Support;	//EMV Mode 0x01, Magstripe Mode 0x02, Legacy Mode 0x04, Issuer Update 0x08

JCB_SKCP JCB_Static_Parameter;
JCB_DTP	JCB_Dynamic_Parameter;	
JCB_Recover_Context JCB_ReContext_Par;
JCB_Online_Txn_Context JCB_Online_Context;
//20140527 add
JCB_ExpFile JCB_ExceptionFile[10];

//20140603
JCB_RevoList JCB_RevocationList[10];

//20140604 add
UCHAR JCB_TAC_Decline[5] = {0x00};
UCHAR JCB_TAC_Online[5] = {0x00};
UCHAR JCB_TAC_Default[5] = {0x00};

