#ifndef _AS350_READERCONFPARA_H_
#define _AS350_READERCONFPARA_H_

#include "POSAPI.h"
#include "Datastructure.h"
#include "Define.h"


// ------------------------------------------------------------------
// The following variables are used in EMVDC demo app
extern	UCHAR	EMVDC_term_config_id;
extern	UCHAR	EMVDC_term_tx_sc[4];
extern	UINT	EMVDC_term_tx_log_cnt;

extern	UCHAR	EMVK_kernel_id;
extern	UCHAR	EMVK_online_enc_pin;			// 1=support, 0=not support
extern	UCHAR	EMVK_no_cvm;				//
extern	UCHAR	EMVK_capk_revoc;			//
extern	UCHAR	EMVK_except_file;			//

extern	UCHAR	EMVK_offline_ptx_pin;
extern	UCHAR	EMVK_offline_enc_pin;

// ------------------------------------------------------------------
//EMVCL Parameter (It will be set by User)
extern UCHAR		glv_parFlgDRL;		//Flag of Support VISA DRL
extern UCHAR		glv_parFlgqVSDC;	//Flag of Support VISA qVSDC
extern UCHAR		glv_parFlgCashBack;	//Flag of Support VISA Cash Back

extern UCHAR		glv_parExtSelSupport[ETP_NUMBER_COMBINATION];	//Extended Selection Support Flag
extern LIMITSET		glv_parDRLLimitSet[ETP_PARA_NUMBER_PID];		//VISA VCPS Global DRL Reader Limit Set
extern LMTSET_LEN	glv_parDRLLimitSet_Len[ETP_PARA_NUMBER_PID];	//VISA VCPS Global DRL Reader Limit Set Length

extern UCHAR		glv_par9F06[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F06];	//Application ID
extern UCHAR		glv_par9F1B[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F1B];	//Terminal Floor Limit
extern UCHAR		glv_par9F5A[ETP_PARA_NUMBER_PID][ETP_PARA_SIZE_9F5A];		//Application Program ID
extern UCHAR		glv_par9F66[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F66];	//TTQ
extern UCHAR		glv_parDF00[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF00];	//Reader CL Transaction Limit
extern UCHAR		glv_parDF01[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF01];	//Reader CVM Required Limit
extern UCHAR		glv_parDF02[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF02];	//Reader CL Floor Limit
extern UCHAR		glv_parDF06[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF06];	//Reader Configuration Parameter

extern UCHAR		glv_parDF00Len[ETP_NUMBER_COMBINATION];	//Length of CTL
extern UCHAR		glv_parDF01Len[ETP_NUMBER_COMBINATION];	//Length of CRL
extern UCHAR		glv_parDF02Len[ETP_NUMBER_COMBINATION];	//Length of CFL

//0529 Purchase 0, Cash 1, Cashback 2, VISA Independent TTQ and ¡§Reader Risk Parameters¡¨ settings
extern UCHAR		glv_parDF00_VISA[VISA_Transaction_Type_NUM][ETP_PARA_SIZE_DF00];	//VISA Reader CL Transaction Limit
extern UCHAR		glv_parDF01_VISA[VISA_Transaction_Type_NUM][ETP_PARA_SIZE_DF01];	//VISA Reader CVM Required Limit
extern UCHAR		glv_parDF02_VISA[VISA_Transaction_Type_NUM][ETP_PARA_SIZE_DF02];	//VISA Reader CL Floor Limit
extern UCHAR		glv_parDF06_VISA[VISA_Transaction_Type_NUM][ETP_PARA_SIZE_DF06];	//VISA Reader Configuration Parameter
extern UCHAR		glv_par9F1B_VISA[VISA_Transaction_Type_NUM][ETP_PARA_SIZE_9F1B];	//VISA Terminal Floor Limit

extern UCHAR		glv_par5F2A[ETP_PARA_SIZE_5F2A];	//Transaction Currency Code
extern UCHAR		glv_par9C[ETP_PARA_SIZE_9C];		//Transaction Type
extern UCHAR		glv_par9F02[ETP_PARA_SIZE_9F02];	//Amount, Authorized (Numeric)
extern UCHAR		glv_par9F03[ETP_PARA_SIZE_9F03];	//Amount, Other (Numeric)
extern UCHAR		glv_par9F1A[ETP_PARA_SIZE_9F1A];	//Terminal Country Code
extern UCHAR		glv_par9F33[ETP_PARA_SIZE_9F33];	//Terminal Capabilities
//extern UCHAR		glv_par9F35[ETP_PARA_SIZE_9F35];	//Terminal Type
extern UCHAR		glv_par9F35[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F35];//Terminal Type
extern UCHAR		glv_par9F7A[ETP_PARA_SIZE_9F7A];	//VLP Support Indicator
/*
extern UCHAR		glv_parDF03[ETP_PARA_SIZE_DF03];	//Enhanced DDA Version Number
extern UCHAR		glv_parDF04[ETP_PARA_SIZE_DF04];	//CVM Required
extern UCHAR		glv_parDF05[ETP_PARA_SIZE_DF05];	//Display Offline Available Fund
*/
extern UCHAR		glv_parDF03[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF03];	//Enhanced DDA Version Number
extern UCHAR		glv_parDF04[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF04];	//CVM Required
extern UCHAR		glv_parDF05[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF05];	//Display Offline Available Fund

extern UCHAR		glv_par9F15[ETP_PARA_SIZE_9F15];
extern UCHAR		glv_par9F4E[ETP_PARA_SIZE_9F4E];
extern UCHAR		glv_par9F4ELen;
extern UCHAR		glv_par5F36[ETP_PARA_SIZE_5F36];
extern UCHAR		glv_par9F01[ETP_PARA_SIZE_9F01];
extern UCHAR		glv_par9F06Len[ETP_NUMBER_COMBINATION];

extern UCHAR		glv_par9F09[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F09];		//Application Version Number (Reader)
extern UCHAR		glv_par9F6D[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F6D];		//Contactless Reader Capabilities
extern UCHAR		glv_par9F6E[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_9F6E];		//Enhanced Contactless Reader Capabilities
extern UCHAR		glv_parDF8120[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF8120];	//Terminal Action Code ¡V Default
extern UCHAR		glv_parDF8121[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF8121];	//Terminal Action Code ¡V Denial
extern UCHAR		glv_parDF8122[ETP_NUMBER_COMBINATION][ETP_PARA_SIZE_DF8122];	//Terminal Action Code ¡V Online

// ------------------------------------------------------------------
//VISA OFDDA Certificate Revocation List
//0xA0,0x00,0x00,0x00,0x03,0x92,0x00,0x82,0x81
extern UCHAR		Revo_List[10][9];		//Revocation List of the Issuer Public Key Certificate (signed by CAPK)

extern UCHAR		Exception_File[EXCEPT_FILE_NUMBER][11];

extern UCHAR 		glv_par_WAVE2_Enable;
extern UCHAR 		glv_par_WAVE2_Terminal_Type;

//VISA qVSDC Reader Option
extern UCHAR Opt_qVSDC_Ter_Except_File;
extern UCHAR Opt_qVSDC_Key_Revocation;
extern UCHAR Opt_qVSDC_ISSUER_Update;

extern UCHAR Opt_Cashback_Transactions;
extern UCHAR Opt_Cash_Transactions;

//VISA MSD
extern UCHAR 	Opt_MSD_CVN17_Support;			//CVN
extern UCHAR	Opt_MSD_Formatting_Track2;		//Track 2
extern UCHAR	Opt_MSD_Constructing_Track1;			//Track 1

extern UCHAR	VISA_Opt_Printer;
// ------------------------------------------------------------------
extern	CRL			glv_CRL[CRL_NUMBER];

// ------------------------------------------------------------------
extern	VAPPARA		glv_vap_Parameter[VAP_PARAMETER_NUMBER];

// ------------------------------------------------------------------
extern	VAPMSG		glv_vap_Message[VAP_MESSAGE_NUMBER];

// ------------------------------------------------------------------
extern	VAPSCHE		glv_vap_Scheme[VAP_SCHEME_NUMBER];

//-------------------------------------------------------------------
extern 	VAPCVM		glv_vap_CVM[VAP_CVM_NUMBER];
//-------------------------------------------------------------------
extern 	UINT		glv_vap_BaudRate;

//-------------------------------------------------------------------

extern	ULONG 		glv_CAPK_Addr;

//20140120 V1, absent problem
extern UCHAR glv_parDF62_Len;
extern UCHAR glv_parDF02_VISA_Len;
extern UCHAR glv_par9F1B_Len;


//20140801 used for VAP VGL JCB kernel Upload data
extern UCHAR Online_Data[1024];
extern UINT  Online_Data_Length;

//PayPass Default Configuration Data
extern const UCHAR glv_parCfgData[266];

#endif
