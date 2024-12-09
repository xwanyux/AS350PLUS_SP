#ifndef _DATASTRUCTURE_H_
#define _DATASTRUCTURE_H_


#include "POSAPI.h"

//	Reader Combination Table
struct cmbtable{
	UCHAR	aidLen;			//AID Length
	UCHAR	cmbAID[16];		//Combination AID
	UCHAR	lstKernel[8];	//Kernel ID List
	UCHAR	flgASI;			//Flag of Application Selection Indicator
}__attribute__((packed));;	
typedef struct cmbtable CMBTABLE;
#define CMBTABLE_LEN	sizeof(CMBTABLE)

//	Entry Point Configuration Data
struct etpconfig{
	UCHAR	pstStaCheck;	//Present of Status Check Support flag
	UCHAR	flgStaCheck;	//Status Check Support flag
	UCHAR	pstZroAllowed;	//Present of Zero Amount Allowed flag
	UCHAR	flgZroAllowed;	//Zero Amount Allowed flag
	UCHAR	pstRdrCTL;		//Present of Reader Contactless Transaction Limit
	UCHAR	rdrCTL[6];		//Reader Contactless Transaction Limit
	UCHAR	pstRdrCFL;		//Present of Reader Contactless Floor Limit
	UCHAR	rdrCFL[6];		//Reader Contactless Floor Limit
	UCHAR	pstTrmFL;		//Present of Terminal Floor Limit
	UCHAR	trmFL[6];		//Terminal Floor Limit
	UCHAR	pstRdrCRL;		//Present of Reader CVM Required Limit
	UCHAR	rdrCRL[6];		//Reader CVM Required Limit
	UCHAR	pstTTQ;			//Present of Terminal Transaction Qualifiers
	UCHAR	TTQ[4];			//Terminal Transaction Qualifiers
	UCHAR	pstExtSelection;//Present of Extended Selection Support flag
	UCHAR	flgExtSelection;//Extended Selection Support flag
}__attribute__((packed));;
typedef struct etpconfig ETPCONFIG;
#define ETPCONFIG_LEN	sizeof(ETPCONFIG)

//	Kernel Configuration Data
struct krnconfig{
	UCHAR	AIDLen;			//Length of Application Identifier
	UCHAR	AID[16];		//Application Identifier
	UCHAR	txnType;		//Transaction Type
	UINT	cfgLen;			//Length of Configuration Data
	UCHAR	cfgData[512];	//Configuration Data
}__attribute__((packed));;
typedef struct krnconfig KRNCONFIG;
#define KRNCONFIG_LEN	sizeof(KRNCONFIG)

//	Entry Point Pre-Processing Indicator
struct indicator{
	UCHAR	staCheck;		//Status Check Requested
	UCHAR	notAllowed;		//Contactless Application Not Allowed
	UCHAR	zroAmount;		//Zero Amount
	UCHAR	rdrCVMExceeded;	//Reader CVM Required Limit Exceeded
	UCHAR	rdrCFLExceeded;	//Reader Contactless Floor Limit Exceeded
	UCHAR	cpyTTQ[4];		//Copy of TTQ
}__attribute__((packed));;
typedef struct indicator INDICATOR;
#define INDICATOR_LEN	sizeof(INDICATOR)

//	VISA VCPS Global DRL Reader Limit Set
struct limitset{
	UCHAR	AppProgIDLen;
	UCHAR	AppProgID[16];	//Application Program ID
	UCHAR	Combined[2];	//B1b8:Status Check Support Flag,
							//B1b7:Zero Amount Allowed Flag,
							//B1b6:Zero Amount Opt Flag,
							//B1b5:Reader CL Transaction Check Flag,
							//B1b4:Reader CVM Limit Check Flag,
							//B1b3:Reader Floor limit Check Flag,
							//B2b8:Terminal txnLimit Check Flag
	UCHAR	rdrtxnLimit[6];	//Reader Contactless Transaction Limit	
	UCHAR	rdrFlrLimit[6];	//Reader Contactless Floor Limit	
	UCHAR	rdrcvmLimit[6];	//Reader CVM Required Limit		
	UCHAR	tertxnLimit[4];	//Terminal txnLimit 
}__attribute__((packed));;
typedef struct limitset LIMITSET;
#define LIMITSET_LEN	sizeof(LIMITSET)

struct limitset_len{
	UCHAR	rdrtxnLimit_Len;	//Length of Reader Contactless Transaction Limit
	UCHAR	rdrFlrLimit_Len;	//Length of Reader Contactless Floor Limit	
	UCHAR	rdrcvmLimit_Len;	//Length of Reader CVM Required Limit		
	UCHAR	tertxnLimit_Len;	//Length of Terminal txnLimit 
}__attribute__((packed));;
typedef struct limitset_len LMTSET_LEN;
#define LMTSET_LEN_LENGTH	sizeof(LMTSET_LEN)

//	VISA VCPS Global DRL Pre-processing Setting
struct predrllimitset{
	UCHAR	DRL_TTQ[4];			//DRL, TTQ
	UCHAR	DRL_CLAP_Not_Allowed;	//DRL, Contactless Application Not Allowed
}__attribute__((packed));;
typedef struct predrllimitset PREDRLLIMITSET;
#define PREDRLLIMITSET_LEN	sizeof(PREDRLLIMITSET)


//	Reader Transaction Candidate List
struct cddlist{
	UCHAR 	adfLen;				//Length of ADF Name
	UCHAR	adfName[16];		//ADF Name
	UCHAR	kidLen;				//Length of KID
	UCHAR	krnID[3];			//Kernel ID
	UCHAR	appPriority;		//Application Priority Indicator
	UCHAR	extLen;				//Length of Extended Selection
	UCHAR	extSelection[11];	//Extended Selection
	UCHAR	cmbIndex;			//Combination Index
}__attribute__((packed));;
typedef struct cddlist CDDLIST;
#define CDDLIST_LEN	sizeof(CDDLIST)

//	Entry Point Outcome
struct outcome{
	UCHAR	Start;			//Start
	UCHAR	rspData[1024];	//Online Response Data
	UCHAR	CVM;			//CVM
	UCHAR	rqtOutcome;		//UI Request on Outcome Present
	UCHAR	ocmMsgID;		//Message Identifier
	UCHAR	ocmStatus;		//Status	[0]Success [1]Fail [2]Ready [3]Not Ready [4]Processing
	UINT	hldTime;		//Hold Time
	UCHAR	ocmLanguage;	//Language Preference
	UCHAR	rqtRestart;		//UI Request on Restart Present
	UCHAR	rstMsgID;		//Restart Message Identifier
	UCHAR	rstStatus;		//Restart Status
	UCHAR	datRecord;		//Data Record Present
	UCHAR	dscData;		//Discretionary Data Present
	UCHAR	altInterface;	//Alternate Interface Preference
	UCHAR	Receipt;		//Receipt
	UINT	filOffRequest;	//Field Off Request
	UINT	rmvTimeout;		//Removal Timeout
}__attribute__((packed));;
typedef struct outcome OUTCOME;
#define OUTCOME_LEN	sizeof(OUTCOME)

//	CA Public Key
struct capk{
	UCHAR	RID[5];			//RID
	UCHAR	Index;			//Key Index
	UCHAR	hshIndicator;	//Hash Algorithm Indicator
	UCHAR	keyIndicator;	//CAPK Algorithm Indicator
	UCHAR	Length;			//Modulus Length
	UCHAR	Modulus[256];	//Modulus
	UCHAR	Exponent[3];	//Exponent
	UCHAR	hshValue[20];	//Hash Value
}__attribute__((packed));;
typedef struct capk CAPK;
#define CAPK_LEN	sizeof(CAPK)

//	Certification Revocation List
struct crl{
	UCHAR	RID[5];			//RID
	UCHAR 	Index;			//Key Index
	UCHAR	serNumber[3];	//Certificate Serial Number
}__attribute__((packed));;
typedef struct crl CRL;
#define CRL_LEN	sizeof(CRL)

//	VISA AP Reader Interface Parameter
struct vappara{
	UCHAR	Index[2];		//Parameter Index
	UCHAR	Length[2];		//Data Length
	UCHAR	Data[16];		//Data Value
}__attribute__((packed));;	
typedef struct vappara VAPPARA;

//	VISA AP Reader Interface Message
struct vapmsg{
	UCHAR	ID;				//Message ID
	UCHAR	Length;			//Message Length
	UCHAR	Message[64];	//Message
}__attribute__((packed));;	
typedef struct vapmsg VAPMSG;

//	VISA AP Reader Interface Scheme Identifier
struct vapsche{
	UCHAR	ID;				//Scheme ID
	UCHAR	Support;		//Support Flag
}__attribute__((packed));;	
typedef struct vapsche VAPSCHE;

//	VISA AP Reader Interface Scheme Identifier
struct vapcvm{
	UCHAR	ID;				//Scheme ID
	UCHAR	State;		//Support Flag
}__attribute__((packed));;	
typedef struct vapcvm VAPCVM;

//JCB Online Transaction Context
struct jcb_online_txn_context
{
	UINT CDOL2_Length;
	UCHAR CDOL2_Value[252];
	UCHAR CVM_Result;
	UINT Txn_Record_Length;
	UCHAR Txn_Record[1024];	
}__attribute__((packed));;
typedef struct jcb_online_txn_context JCB_Online_Txn_Context;

//JCB Recovery Context Data
struct jcb_recovery_context
{
	UCHAR LastTxnType;	//Last Generate AC, TxnType
	UCHAR LastGenACType;//Last Generate AC, GenACType 
	UCHAR RecoverEMVFlag;
	UCHAR TornTrack2Data[19];
	UCHAR TornCDAHashDataBufSize;
	UCHAR TornCDAHashDataBuf[509];	//PDOL L-V, CDOL L-V
	UCHAR UnPredNum[4];
}__attribute__((packed));;
typedef struct jcb_recovery_context JCB_Recover_Context;

//JCB Static Kernel Configuration Parameter
struct jcb_skcp{
	UCHAR	CombinationOption[2];			//Combination Options
	UCHAR	ClessFloorLimitLen;
	UCHAR	ClessFloorLimit[6];				//Contactless Floor Limit
	UCHAR	ClessTxnLimitLen;
	UCHAR	ClessTxnLimit[6];				//Contactless Transaction Limit
	UCHAR	ClessCVMLimitLen;
	UCHAR	ClessCVMLimit[6];				//CVM Required Limit
	UCHAR	MAXTargetPercent_RandomSelection;	//Maximum Target Percentage to be Used for Biased Random Selection
	UCHAR	Remove_Timeout[2];				//Removal Timeout
	UCHAR	TargetPercent_RandomSelection;	//Target Percentage to be Used for Biased Random Selection
	UCHAR	TAC_Default_Present;
	UCHAR	TAC_Default[5];					//Terminal Action Code - Default
	UCHAR	TAC_Denial_Present;
	UCHAR	TAC_Denial[5];					//Terminal Action Code - Denial
	UCHAR	TAC_Online_Present;
	UCHAR	TAC_Online[5];					//Terminal Action Code - Online
	UCHAR	TerminalInterchangeProfile[3];	//Terminal Interchange Profile (static)
	UCHAR	ThresholdValue_RandomSelection[6];	//Threshold Value for Biased Random Selection
	UCHAR	AcquirerID[6];					//Acquirer Identifier
	UCHAR	MerchantCategoryCode[2];		//Merchant Category Code
	UCHAR	MerchantNameLocationLen;		//Merchant Name and Location Len
	UCHAR	MerchantNameLocation[128];		//Merchant Name and Location
	UCHAR	TerminalCountryCode[2];			//Terminal Country Code
	UCHAR	TerminalType;					//Terminal Type
	UCHAR	TxnCurrencyCode[2];				//Transaction Currency Code
	UCHAR	TxnCurrencyExponent;			//Transaction Currency Exponent
}__attribute__((packed));;
typedef struct jcb_skcp JCB_SKCP;

//JCB Dynamic Transaction Parameter
struct jcb_dtp{
	UCHAR	JCB_AmountAuth[6];			//Amount, Authorised
	UCHAR	JCB_AmountAuthOther[6];		//Amount, Other
	UCHAR	AuthRspCodeLen;
	UCHAR	AuthRspCode[2];				//Authorisation Response Code
	UCHAR	JCB_IADLen;
	UCHAR	JCB_IAD[16];				//Issuer Authentication Data
	UCHAR	JCB_IST_1Len;
	UCHAR	JCB_IST_1[128];				//Issuer Script Template 1
	UCHAR	JCB_IST_2Len;
	UCHAR	JCB_IST_2[128];				//Issuer Script Template 2
	UCHAR	TxnDate[3];					//Transaction Date
	UCHAR	Txntime[3];					//Transaction Time
	UCHAR	TxnType;					//Transaction Type
	UCHAR	JCB_UnpredictNum[4];		//Unpredictable Number
}__attribute__((packed));;
typedef struct jcb_dtp JCB_DTP;

//JCB Exception File
struct jcb_expfile{
	UCHAR	PAN[10];		//Application Primary Account Number
	UCHAR	SN;				//Application Primary Account Number (PAN) Sequence Number
}__attribute__((packed));;
typedef struct jcb_expfile JCB_ExpFile;

//JCB Revo List
struct jcb_revolist{
	UCHAR	RID[5];		//RID
	UCHAR	CAPKIndex;	//Certification Authority Public Key Index
	UCHAR	SN[3];		//Certificate Sequence Number
}__attribute__((packed));;
typedef struct jcb_revolist JCB_RevoList;

#endif

