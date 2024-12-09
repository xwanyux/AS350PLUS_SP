 
// Wayne 2020/08/12
// #define _PLATFORM_AS350					1
// #define _SCREEN_SIZE_320x240			1

#define _JCB_TA_

#define JCB_Fixed_Length				0x01
#define JCB_Var_Length					0x02

#ifdef _JCB_TA_
//For JCB Transaction Outcome Status
#define JCB_TxnR_Approval				0x01
#define JCB_TxnR_OnlineReq				0x02
#define JCB_TxnR_OnlineReq_Two_Present	0x03
#define JCB_TxnR_OnlineReq_Hold			0x04
#define JCB_TxnR_Decline				0x05
#define JCB_TxnR_TryAnother				0x06
#define JCB_TxnR_EndApp					0x07
#define JCB_TxnR_EndAppWithRst			0x08
#define JCB_TxnR_EndAppWithRstOnDev		0x09
#define JCB_TxnR_SelectNext				0x0A
#endif

#ifdef _SCREEN_SIZE_128x64
#define Display_MAX_Num					16
#endif

#ifdef _SCREEN_SIZE_240x320
#define Display_MAX_Num					20
#endif
// #define Display_MAX_Num					20
// #ifdef _SCREEN_SIZE_320x240
// #define Display_MAX_Num					26
// #endif


#define TRUE							1
#define FALSE							0
#define SUCCESS							1
#define FAIL							0

#define NULLPTR							(void *) 0
#define ERROR							0xFFU


//	Level 3 >>>
#define VAP_PARAMETER_NUMBER			16
#define VAP_MESSAGE_NUMBER				35
#define VAP_SCHEME_NUMBER				(16+1)		//add 1 for JCB MSB transaction
#define VAP_CVM_NUMBER					4
#define VAP_Private_MESSAGE_NUMBER		5
//	Level 3 <<<

//	Level 2 >>>
//	Number of CA Public Key
#define CAPK_NUMBER						50

//	Number of Certification Revocation List
#define CRL_NUMBER						100

//	Number of Exception File
#define EXCEPT_FILE_NUMBER				10

//	Status Word
#define STATUSWORD_6283					0x6283
#define STATUSWORD_6982					0x6982
#define STATUSWORD_6984					0x6984
#define STATUSWORD_6986					0x6986
#define STATUSWORD_6A81					0x6A81
#define STATUSWORD_6A82					0x6A82
#define STATUSWORD_9000					0x9000

//	TLV Data Format
#define FORMAT_AN   					0x00U
#define FORMAT_ANS						0x01U
#define FORMAT_B						0x02U
#define FORMAT_CN						0x03U
#define FORMAT_N						0x04U
#define FORMAT_VAR						0x05U
#define FORMAT_A						0x06U

//	MasterCard PayPass TLV IsKnown
#define MASTER_UNKNOWN					0x00U
#define MASTER_KNOWN					0x01U

//	MasterCard PayPass TLV Update Condition
#define MASTER_UC_UNDEFINED				0x00U
#define MASTER_UC_K						0x01U
#define MASTER_UC_RA					0x02U
#define MASTER_UC_ACT					0x04U
#define MASTER_UC_DET					0x08U

//	MasterCard PayPass TLV Template
#define MASTER_TMP_UNDEFINED			0x00U
#define MASTER_TMP_70					0x01U
#define MASTER_TMP_77					0x02U
#define MASTER_TMP_A5					0x04U
#define MASTER_TMP_BF0C					0x08U
#define MASTER_TMP_6F					0x10U

//	UNDEFINED
#define	UNDEFINED						0x00U

//	VISA Update Capability
#define VISA_UC_UNDEFINED				0x00U
#define VISA_UC_UNCHANGING				0x01U
#define VISA_UC_MODIFIABLE				0x02U	
#define VISA_UC_PERSISTENT				0x04U
#define VISA_UC_DYNAMIC					0x08U
#define VISA_UC_TRANSIENT				0x10U

//	VISA Issuer Update
#define VISA_IU_UNDEFINED				0x00U
#define VISA_IU_NOT_ALLOWED				0x01U		
#define VISA_IU_CSU						0x02U
#define VISA_IU_PIN						0x04U
#define VISA_IU_PUT_DATA				0x08U
#define VISA_IU_UPDATE_RECORD			0x10U
#define VISA_IU_CSU_PIN					0x20U


//	VISA Retrieval
#define VISA_R_UNDEFINED				0x00U
#define VISA_R_NOT_ALLOWED				0x01U	//VISA_UPDATE_Capability
#define VISA_R_GET_DATA					0x02U
#define VISA_R_GET_DATA_SD				0x04U
#define VISA_R_GPO						0x08U
#define VISA_R_READ_RECORD				0x10U
#define VISA_R_SELECT					0x20U

//JCB Presence
#define	JCB_Mandatory					0x01U
#define	JCB_Conditional					0x02U
#define	JCB_Optional					0x03U

#define VISA_Transaction_Type_NUM		3		//0 for Purchase Mode, 1 for the cash Mode, 2 for Cashback

#define VISA_Purchase_Mode				0
#define VISA_Cash_Mode					1
#define VISA_Cashback_Mode				2


//VISA Finish Transaction
#define TransactionFinish				0x35U

//	AE ExpressPay Tag IsKnown
#define AE_UNKNOWN						0x00U
#define AE_KNOWN						0x01U

//	D-PAS TLV IsKnown
#define DPS_UNKNOWN						0x00U
#define DPS_KNOWN						0x01U


//	Entry Point
#define ETP_NUMBER_TRANSACTIONTYPE		4		//Number of Entry Point Support Transaction Type
#define ETP_NUMBER_COMBINATION			(7+2+1)	//Number of Combination Index (+2 for UPI +1 for Discover)
#define ETP_NUMBER_CANDIDATELIST		15		//Number of Candidate List
#define ETP_NUMBER_KERNELCONFIGURATION	15		//Number of Kernel Configuration Data Set
#define ETP_BUFFER_SIZE					1024	//Size of Buffer
#define ETP_TRANSACTION_DATA_SIZE		512		//Size of Transaction Data
#define ETP_TIMEOUT_ACTIVATION			500		//Activation Timeout(Unit:10 ms)

//	Kernel ID
#define ETP_KID_VISAAP					1
#define ETP_KID_MASTER					2
#define ETP_KID_VISA					3
#define ETP_KID_AMEX					4
#define ETP_KID_JCB						5
#define ETP_KID_DISCOVER				6
#define ETP_KID_UPI						7

//	Transaction Type Index in Combination Table
#define ETP_CMBTABLE_PURCHASE			0
#define ETP_CMBTABLE_CASH				1
#define ETP_CMBTABLE_CASHBACK			2
#define ETP_CMBTABLE_REFUND				3

//	Parameter Number
#define ETP_PARA_NUMBER_PID				4	//Application Program ID

//	Parameter Size
#define ETP_PARA_SIZE_TransactionType	1	//TransactionType
#define ETP_PARA_SIZE_CurrencyCode		2	//Currency Code

#define ETP_PARA_SIZE_5F2A				2	//Transaction Currency Code
#define ETP_PARA_SIZE_9C				1	//Transaction Type
#define ETP_PARA_SIZE_9F02				6	//Amount, Authorized (Numeric)
#define ETP_PARA_SIZE_9F03				6	//Amount, Other (Numeric)
#define ETP_PARA_SIZE_9F06				16	//AID
#define ETP_PARA_SIZE_9F09				2	//Application Version Number (Reader)
#define ETP_PARA_SIZE_9F1A				2	//Terminal Country Code
#define ETP_PARA_SIZE_9F1B				4	//Terminal Floor Limit
#define ETP_PARA_SIZE_9F35				1	//Terminal Type
#define ETP_PARA_SIZE_9F5A				16	//Application Program ID
#define ETP_PARA_SIZE_9F7A				1	//VLP Support Indicator
#define ETP_PARA_SIZE_9F66				4	//TTQ
#define ETP_PARA_SIZE_DF00				6	//Reader CL Transaction Limit
#define ETP_PARA_SIZE_DF01				6	//Reader CVM Required Limit
#define ETP_PARA_SIZE_DF02				6	//Reader CL Floor Limit
#define ETP_PARA_SIZE_DF03				1	//Enhanced DDA Version Number
#define ETP_PARA_SIZE_DF04				1	//CVM Required
#define ETP_PARA_SIZE_DF05				1	//Display Offline Available Fund
#define ETP_PARA_SIZE_DF06				2	//Reader Configuration Parameter

#define ETP_PARA_SIZE_9F15				2	//Merchant Code, Reader Configuration Parameter
#define ETP_PARA_SIZE_9F4E				128	//Merchant Name, Reader Configuration Parameter
#define ETP_PARA_SIZE_5F36				1	//Txn Exponent, Reader Configuration Parameter
#define ETP_PARA_SIZE_9F01				6	//AID, Reader Configuration Parameter

#define ETP_PARA_SIZE_9F09				2	//Application Version Number (Reader)
#define ETP_PARA_SIZE_9F33				3	//Terminal Capabilities
#define ETP_PARA_SIZE_9F6D				1	//Contactless Reader Capabilities
#define ETP_PARA_SIZE_9F6E				4	//Enhanced Contactless Reader Capabilities
#define ETP_PARA_SIZE_DF8120			5	//Terminal Action Code �V Default
#define ETP_PARA_SIZE_DF8121			5	//Terminal Action Code �V Denial
#define ETP_PARA_SIZE_DF8122			5	//Terminal Action Code �V Online


//	Outcome Parameter - Start
#define ETP_OCP_Start_NA				0	//Start NA
#define ETP_OCP_Start_A					'A'	//Start A
#define ETP_OCP_Start_B					'B'	//Start B
#define ETP_OCP_Start_C					'C'	//Start C
#define ETP_OCP_Start_D					'D'	//Start D

//	Outcome Parameter - CVM
#define ETP_OCP_CVM_NA							0x00U	//No CVM preference
#define ETP_OCP_CVM_OnlinePIN					0x01U	//Online PIN
#define ETP_OCP_CVM_ConfirmationCodeVerified	0x02U	//Confirmation Code Verified
#define ETP_OCP_CVM_ObtainSignature				0x03U	//Obtain Signature
#define ETP_OCP_CVM_NoCVM						0x04U	//No CVM

//	Outcome Parameter - UI Request on Outcome Present
#define ETP_OCP_UIOnOutcomePresent_No			0x00U	//No
#define ETP_OCP_UIOnOutcomePresent_Yes			0x01U	//Yes

//	Outcome Parameter - UI Request on Restart Present
#define ETP_OCP_UIOnRestartPresent_No			0x00U	//No
#define ETP_OCP_UIOnRestartPresent_Yes			0x01U	//Yes

//	Outcome Parameter - Data Record Present
#define ETP_OCP_DataRecordPresent_No			0x00U	//No
#define ETP_OCP_DataRecordPresent_Yes			0x01U	//Yes

//	Outcome Parameter - Discretionary Data Present
#define ETP_OCP_DiscretionaryDataPresent_No		0x00U	//No
#define ETP_OCP_DiscretionaryDataPresent_Yes	0x01U	//Yes

//	Outcome Parameter - Alternate Interface Preference
#define ETP_OCP_AlternateInterface_NA			0x00	//N/A
#define ETP_OCP_AlternateInterface_ContactChip	0x01	//Contact Chip
#define ETP_OCP_AlternateInterface_MagStripe	0x02	//Mag-stripe

//	Outcome Parameter - Receipt
#define ETP_OCP_Receipt_NA						0x00	//N/A
#define ETP_OCP_Receipt_Yes						0x01	//Yes      

//	Outcome Parameter - Field Off Request
#define ETP_OCP_FieldOffRequest_NA				0x00	//N/A

//	Outcome Parameter - UI Request Message
#define ETP_OCP_UIM_NA							0x00U	//N/A
#define ETP_OCP_UIM_Approved					0x03U	//Approved
#define ETP_OCP_UIM_NotAuthorised				0x07U	//Not Authorised
#define ETP_OCP_UIM_PleaseEnterYourPIN			0x09U	//Please enter your PIN
#define ETP_OCP_UIM_ProcessingError				0x0FU	//Processing error
#define ETP_OCP_UIM_RemoveCard					0x10U	//Remove card
#define ETP_OCP_UIM_Welcome						0x14U	//Welcome
#define ETP_OCP_UIM_PresentCard					0x15U	//Present Card
#define ETP_OCP_UIM_Processing					0x16U	//Processing
#define ETP_OCP_UIM_CardReadOKRemoveCard		0x17U	//Card read OK remove card
#define ETP_OCP_UIM_PleaseInsertOrSwipeCard		0x18U	//Please Insert or Swipe Card
#define ETP_OCP_UIM_PleasePresentOneCardOnly	0x19U	//Please present one card only
#define ETP_OCP_UIM_ApprovedPleaseSign			0x1AU	//Approved Please sign
#define ETP_OCP_UIM_AuthorisingPleaseWait		0x1BU	//Authorising Please Wait
#define ETP_OCP_UIM_InsertSwipeOrTryAnotherCard	0x1CU	//Insert, Swipe or Try Another Card
#define ETP_OCP_UIM_PleaseInsertCard			0x1DU	//Please insert card
#define ETP_OCP_UIM_NoMessageDisplayed			0x1EU	//No message displayed
#define ETP_OCP_UIM_SeePhoneForInstructions		0x20U	//See Phone for Instructions
#define ETP_OCP_UIM_PresentCardAgain			0x21U	//Present card again
#define ETP_OCP_UIM_Terminate					0xF0U	//Transaction Terminate

//	Outcome Parameter - UI Request Status
#define ETP_OCP_UIS_CardReadSuccessfully		0	//Card Read Successfully
#define ETP_OCP_UIS_ProcessingError				1	//Processing Error
#define ETP_OCP_UIS_ReadyToRead					2	//Ready to Read
#define ETP_OCP_UIS_NotReady					3	//Not Ready
#define ETP_OCP_UIS_Processing					4	//Processing
#define ETP_OCP_UIS_NA							5	//N/A

//	Outcome Result
#define ETP_OUTCOME_Timeout						1	//Timeout
#define ETP_OUTCOME_Contact						2	//Contact Interrupt
#define ETP_OUTCOME_TryAnotherInterface			4	//TryAnotherInterface
#define ETP_OUTCOME_EndApplication				10	//EndApplication
#define ETP_OUTCOME_Terminate					255	//Terminate

//	Outcome Status
#define ETP_OUTCOME_Approved					0xF0U	//Approved
#define ETP_OUTCOME_Declined					0xF1U	//Declined
#define ETP_OUTCOME_OnlineRequest				0xF2U	//Online Request
#define ETP_OUTCOME_TryAgain					0xF3U	//Try Again
#define ETP_OUTCOME_SelectNext					0xF4U	//Select Next

//	TTQ
#define ETP_TTQ_MagStripeMode_Supported				0x80U
#define ETP_TTQ_MagStripeMode_NotSupported			0x00U
#define ETP_TTQ_EMVMode_Supported					0x20U
#define ETP_TTQ_EMVMode_NotSupported				0x00U
#define ETP_TTQ_EMVContactChip_Supported			0x10U
#define ETP_TTQ_EMVContactChip_NotSupported			0x00U
#define ETP_TTQ_OfflineOnlyReader					0x08U
#define ETP_TTQ_OnlineCapableReader					0x00U
#define ETP_TTQ_OnlinePIN_Supported					0x04U
#define ETP_TTQ_OnlinePIN_NotSupported				0x00U
#define ETP_TTQ_Signature_Supported					0x02U
#define ETP_TTQ_Signature_NotSupported				0x00U
#define ETP_TTQ_OnlineCryptogram_Required			0x80U
#define ETP_TTQ_OnlineCryptogram_NotRequired		0x00U
#define ETP_TTQ_CVM_Required						0x40U
#define ETP_TTQ_CVM_NotRequired						0x00U
#define ETP_TTQ_OfflinePIN_Supported				0x20U
#define ETP_TTQ_OfflinePIN_NotSupported				0x00U
#define ETP_TTQ_IssuerUpdateProcessing_Supported	0x80U
#define ETP_TTQ_IssuerUpdateProcessing_NotSupported	0x00U
#define ETP_TTQ_ConsumerDeviceCVM_Supported			0x40U
#define ETP_TTQ_ConsumerDeviceCVM_NotSupported		0x00U

#define DOL_BUFFER_SIZE_RelatedData					255		//Buffer Size of DOL Related Data

#define DEP_BUFFER_SIZE_SEND						512
#define DEP_BUFFER_SIZE_RECEIVE						2048
//	Level 2 <<<


