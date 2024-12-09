/*
*********************************************************************
REMARK:  For AS320 platform, those global variables defined here 
         will be put into SRAM for backup.
         And the initialed value will not be actuallly loaded 
         into the variables while run time, i.e., the contents of
         global variable are retained after next power on.

NOTE:	 The total size of global variables must be less than 24KB.
*********************************************************************
*/

#include "POSAPI.h"
#include "Define.h"
#include "Datastructure.h"


// --- DSS system variables - DON'T REMOVE! ---
UCHAR DssTelNum[23]={0};
UCHAR DssRemoteIP[23]={0};
UCHAR DssRemotePort[23]={0};
UCHAR DssPort[23]={0};


// ------------------------------------------------------------------
// The following variables are used in EMVDC demo app
UCHAR	EMVDC_term_config_id = 0;			// terminal config id
UCHAR	EMVDC_term_tx_sc[4] = {0};			// transaction Sequence Counter
UINT	EMVDC_term_tx_log_cnt = 0;			// transaction log counter

UCHAR	EMVK_kernel_id = 1;				// kernel id (01..04)
UCHAR	EMVK_online_enc_pin = 1;			// 1=support, 0=not support
UCHAR	EMVK_no_cvm = 1;				//
UCHAR	EMVK_capk_revoc = 1;				//
UCHAR	EMVK_except_file = 1;				//

UCHAR	EMVK_offline_ptx_pin = 1;			// offline plaintext pin
UCHAR	EMVK_offline_enc_pin = 1;			// offline enciphered pin

// ------------------------------------------------------------------
//EMVCL Parameter (It will be set by User)
UCHAR		glv_parFlgDRL=0;		//Flag of Support VISA DRL
UCHAR		glv_parFlgqVSDC=0;		//Flag of Support VISA qVSDC
UCHAR		glv_parFlgCashBack=0;	//Flag of Support VISA Cash Back

UCHAR		glv_parExtSelSupport[ETP_NUMBER_COMBINATION]	={0};							//Extended Selection Support Flag
LIMITSET	glv_parDRLLimitSet[ETP_PARA_NUMBER_PID]			={{0,{0},{0},{0},{0},{0},{0}}};	//VISA VCPS Global DRL Reader Limit Set
LMTSET_LEN	glv_parDRLLimitSet_Len[ETP_PARA_NUMBER_PID]		={{0,0,0,0}};					//VISA VCPS Global DRL Reader Limit Set Length

UCHAR		glv_par9F06[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F06]	={{0}};	//Application ID
UCHAR		glv_par9F1B[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F1B]	={{0}};	//Terminal Floor Limit
UCHAR		glv_par9F5A[ETP_PARA_NUMBER_PID][ETP_PARA_SIZE_9F5A]	={{0}};	//Application Program ID
UCHAR		glv_par9F66[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F66]	={{0}};	//TTQ
UCHAR		glv_parDF00[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF00]	={{0}};	//Reader CL Transaction Limit
UCHAR		glv_parDF01[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF01]	={{0}};	//Reader CVM Required Limit
UCHAR		glv_parDF02[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF02]	={{0}};	//Reader CL Floor Limit
UCHAR		glv_parDF06[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF06]	={{0}};	//Reader Configuration Parameter

UCHAR		glv_parDF00Len[ETP_NUMBER_COMBINATION]={0};	//Length of CTL
UCHAR		glv_parDF01Len[ETP_NUMBER_COMBINATION]={0};	//Length of CRL
UCHAR		glv_parDF02Len[ETP_NUMBER_COMBINATION]={0};	//Length of CFL

//0529 Purchase 0, Cash 1, Cashback 2, VISA Independent TTQ and ¡§Reader Risk Parameters¡¨ settings
UCHAR		glv_parDF00_VISA[VISA_Transaction_Type_NUM][ETP_PARA_SIZE_DF00]	={{0}};	//VISA Reader CL Transaction Limit
UCHAR		glv_parDF01_VISA[VISA_Transaction_Type_NUM][ETP_PARA_SIZE_DF01]	={{0}};	//VISA Reader CVM Required Limit
UCHAR		glv_parDF02_VISA[VISA_Transaction_Type_NUM][ETP_PARA_SIZE_DF02]	={{0}};	//VISA Reader CL Floor Limit
UCHAR		glv_parDF06_VISA[VISA_Transaction_Type_NUM][ETP_PARA_SIZE_DF06]	={{0}};	//VISA Reader Configuration Parameter
UCHAR		glv_par9F1B_VISA[VISA_Transaction_Type_NUM][ETP_PARA_SIZE_9F1B]	={{0}};	//VISA Terminal Floor Limit


UCHAR		glv_par5F2A[ETP_PARA_SIZE_5F2A]={0};	//Transaction Currency Code
UCHAR		glv_par9C[ETP_PARA_SIZE_9C]={0};		//Transaction Type
UCHAR		glv_par9F02[ETP_PARA_SIZE_9F02]={0};	//Amount, Authorized (Numeric)
UCHAR		glv_par9F03[ETP_PARA_SIZE_9F03]={0};	//Amount, Other (Numeric)
UCHAR		glv_par9F1A[ETP_PARA_SIZE_9F1A]={0};	//Terminal Country Code
UCHAR		glv_par9F33[ETP_PARA_SIZE_9F33]={0};	//Terminal Capabilities
//UCHAR		glv_par9F35[ETP_PARA_SIZE_9F35]={0};	//Terminal Type
UCHAR		glv_par9F35[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F35]={{0}};	//Terminal Type
UCHAR		glv_par9F7A[ETP_PARA_SIZE_9F7A]={0};	//VLP Support Indicator
/*
UCHAR		glv_parDF03[ETP_PARA_SIZE_DF03]={0};	//Enhanced DDA Version Number
UCHAR		glv_parDF04[ETP_PARA_SIZE_DF04]={0};	//CVM Required
UCHAR		glv_parDF05[ETP_PARA_SIZE_DF05]={0};	//Display Offline Available Fund
*/
UCHAR		glv_parDF03[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF03] = {{0}};	//Enhanced DDA Version Number
UCHAR		glv_parDF04[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF04] = {{0}};	//CVM Required
UCHAR		glv_parDF05[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF05] = {{0}};	//Display Offline Available Fund

//Test
UCHAR		glv_par9F15[ETP_PARA_SIZE_9F15] = {0};
UCHAR		glv_par9F4E[ETP_PARA_SIZE_9F4E] = {0};
UCHAR		glv_par9F4ELen=0;
UCHAR		glv_par5F36[ETP_PARA_SIZE_5F36] = {0};
UCHAR		glv_par9F01[ETP_PARA_SIZE_9F01] = {0};
UCHAR		glv_par9F06Len[ETP_NUMBER_COMBINATION]={0};

UCHAR		glv_par9F09[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F09] = {{0}};		//Application Version Number (Reader)
UCHAR		glv_par9F6D[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F6D] = {{0}};		//Contactless Reader Capabilities
UCHAR		glv_par9F6E[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F6E] = {{0}};		//Enhanced Contactless Reader Capabilities
UCHAR		glv_parDF8120[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF8120] = {{0}};	//Terminal Action Code ¡V Default
UCHAR		glv_parDF8121[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF8121] = {{0}};	//Terminal Action Code ¡V Denial
UCHAR		glv_parDF8122[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF8122] = {{0}};	//Terminal Action Code ¡V Online

// ------------------------------------------------------------------
CRL			glv_CRL[CRL_NUMBER]={{{0},0,{0}}};

// ------------------------------------------------------------------
//VISA OFDDA Certificate Revocation List
//0xA0,0x00,0x00,0x00,0x03,0x92,0x00,0x82,0x81
UCHAR		Revo_List[10][9] = {{0}};		//Revocation List of the Issuer Public Key Certificate (signed by CAPK)

//Exception File: PAN(10) + PAN Sequence Number(1)
//0x47,0x61,0x73,0x90,0x01,0x01,0x00,0x00,0xFF,0xFF,0x01
UCHAR		Exception_File[EXCEPT_FILE_NUMBER][11] = {{0}};

//WAVE2 Enable
UCHAR		glv_par_WAVE2_Enable = 0;
UCHAR		glv_par_WAVE2_Terminal_Type = 0;

//VISA qVSDC Reader Option
UCHAR Opt_qVSDC_Ter_Except_File=0;
UCHAR Opt_qVSDC_Key_Revocation=0;
UCHAR Opt_qVSDC_ISSUER_Update=0;

UCHAR Opt_Cashback_Transactions = 0;
UCHAR Opt_Cash_Transactions = 0;

//VISA MSD Reader
//Acquirer-Merchant-Optional
UCHAR 	Opt_MSD_CVN17_Support 			= 	0;			//CVN
UCHAR	Opt_MSD_Formatting_Track2		=	0;			//Track 2
UCHAR	Opt_MSD_Constructing_Track1		=	0;			//Track 1

//VISA Global Printer setup
UCHAR	VISA_Opt_Printer = 0;

// ------------------------------------------------------------------
VAPPARA		glv_vap_Parameter[VAP_PARAMETER_NUMBER]={{{0}, {0}, {0}}};

// ------------------------------------------------------------------
VAPMSG		glv_vap_Message[VAP_MESSAGE_NUMBER]={{0,0,{0}}};

// ------------------------------------------------------------------
VAPSCHE		glv_vap_Scheme[VAP_SCHEME_NUMBER]={{0,0}};
// ------------------------------------------------------------------
VAPCVM		glv_vap_CVM[VAP_CVM_NUMBER]={{0,0}};
// ------------------------------------------------------------------
UINT		glv_vap_BaudRate = 0;

//20140103 test
ULONG glv_CAPK_Addr=0;

//20140120 V1 absent problem
UCHAR glv_parDF62_Len = 0;
UCHAR glv_parDF02_VISA_Len = 0;
UCHAR glv_par9F1B_Len = 0;
	

//20140801 used for VAP VGL JCB kernel Upload data
UCHAR Online_Data[1024]={0};
UINT  Online_Data_Length=0;

//PayPass Default Configuration Data
const UCHAR glv_parCfgData[266]={
	//Tag,			Length,	Value
	0x9F,0x06,		0x07,	0xA0,0x00,0x00,0x00,0x04,0x10,0x10,
	0x9C,			0x01,	0x00,
	0x5F,0x36,		0x01,	0x02,
	0x5F,0x57,		0x00,
	0x9F,0x01,		0x00,
	0x9F,0x15,		0x02,	0x00,0x00,
	0x9F,0x16,		0x00,
	0x9F,0x1A,		0x02,	0x01,0x58,
	0x9F,0x1C,		0x00,
	0x9F,0x1D,		0x08,	0x6C,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
	0x9F,0x1E,		0x08,	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x9F,0x09,		0x02,	0x00,0x02,
	0x9F,0x33,		0x00,
	0x9F,0x40,		0x05,	0x00,0x00,0x00,0x00,0x00,
	0x9F,0x4E,		0x01,	0x20,
	0x9F,0x53,		0x01,	0x00,
	0x9F,0x6D,		0x02,	0x00,0x01,
	0x9F,0x7C,		0x14,	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x9F,0x7E,		0x00,
	0xDF,0x60,		0x00,
	0xDF,0x62,		0x00,
	0xDF,0x63,		0x00,
	0xDF,0x81,0x08,	0x00,
	0xDF,0x81,0x09,	0x00,
	0xDF,0x81,0x0A,	0x00,
	0xDF,0x81,0x0C,	0x01,	0x02,
	0xDF,0x81,0x0D,	0x00,
	0xDF,0x81,0x17,	0x01,	0x00,
	0xDF,0x81,0x18,	0x01,	0x60,
	0xDF,0x81,0x19,	0x01,	0x08,
	0xDF,0x81,0x1A,	0x03,	0x9F,0x6A,0x04,
	0xDF,0x81,0x1B,	0x01,	0x30,
	0xDF,0x81,0x1C,	0x02,	0x00,0x00,
	0xDF,0x81,0x1D,	0x01,	0x00,
	0xDF,0x81,0x1E,	0x01,	0x10,
	0xDF,0x81,0x1F,	0x01,	0x08,
	0xDF,0x81,0x20,	0x05,	0x00,0x00,0x00,0x00,0x00,
	0xDF,0x81,0x21,	0x05,	0x00,0x00,0x00,0x00,0x00,
	0xDF,0x81,0x22,	0x05,	0x00,0x00,0x00,0x00,0x00,
	0xDF,0x81,0x23,	0x06,	0x00,0x00,0x00,0x30,0x00,0x00,
	0xDF,0x81,0x24,	0x06,	0x00,0x99,0x99,0x99,0x99,0x99,
	0xDF,0x81,0x25,	0x06,	0x00,0x99,0x99,0x99,0x99,0x99,
	0xDF,0x81,0x26,	0x06,	0x00,0x00,0x00,0x30,0x00,0x00,
	0xDF,0x81,0x2C,	0x01,	0x00};

