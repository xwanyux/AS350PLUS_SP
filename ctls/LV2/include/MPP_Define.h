//	MasterCard PayPass

//	Enable DE Log
//#define MPP_DE_LOG													1

//	ListItem Buffer Size
#define MPP_TLVLIST_BUFFERSIZE										1024
#define MPP_SYNCDATA_BUFFERSIZE										2048

//	APDU Buffer Size
#define MPP_APDU_BUFFERSIZE											270

//	Queue Size
#define MPP_QUEUE_SIZE												6

//	Kernel Support Torn Transaction Record
#define MPP_TORN_BUFFERSIZE											1600
#define MPP_TORN_AIDNUMBER											3
#define MPP_TORN_RECORDNUMBER										3

//	Kernel Phone Message Table ID Number
#define MPP_PHONEMESSAGE_IDNUMBER									5

//	Buffer Size of DOL Related Data
#define MPP_DOL_BUFFERSIZE											255

//	82	Application Interchange Profile
#define MPP_AIP_SDASupported										0x40U
#define MPP_AIP_DDASupported										0x20U
#define MPP_AIP_CardholderVerificationIsSupported					0x10U
#define MPP_AIP_TerminalRiskManagementIsToBePerformed				0x08U
#define MPP_AIP_IssuerAuthenticationIsSupported						0x04U
#define MPP_AIP_OnDeviceCardholderVerificationIsSupported			0x02U
#define MPP_AIP_CDASupported										0x01U
#define MPP_AIP_EMVModeIsSupported									0x80U
#define MPP_AIP_RelayResistanceProtocolIsSupported					0x01U

//	95	Terminal Verification Results
#define MPP_TVR_OfflineDataAuthenticationWasNotPerformed			0x80U
#define MPP_TVR_SDAFailed											0x40U
#define MPP_TVR_ICCDataMissing										0x20U
#define MPP_TVR_CardAppearsOnTerminalExceptionFile					0x10U
#define MPP_TVR_DDAFailed											0x08U
#define MPP_TVR_CDAFailed											0x04U
#define MPP_TVR_ICCAndTerminalHaveDifferentApplicationVersions		0x80U
#define MPP_TVR_ExpiredApplication									0x40U
#define MPP_TVR_ApplicationNotYetEffective							0x20U
#define MPP_TVR_RequestedServiceNotAllowedForCardProduct			0x10U
#define MPP_TVR_NewCard												0x08U
#define MPP_TVR_CardholderVerificationWasNotSuccessful				0x80U
#define MPP_TVR_UnrecognisedCVM										0x40U
#define MPP_TVR_PINTryLimitExceeded									0x20U
#define MPP_TVR_PINEntryRequiredAndPINPadNotPresentOrNotWorking		0x10U
#define MPP_TVR_PINEntryRequiredPINPadPresentButPINWasNotEntered	0x08U
#define MPP_TVR_OnlinePINEntered									0x04U
#define MPP_TVR_TransactionExceedsFloorLimit						0x80U
#define MPP_TVR_LowerConsecutiveOfflineLimitExceeded				0x40U
#define MPP_TVR_UpperConsecutiveOfflineLimitExceeded				0x20U
#define MPP_TVR_TransactionSelectedRandomlyForOnlineProcessing		0x10U
#define MPP_TVR_MerchantForcedTransactionOnline						0x08U
#define MPP_TVR_DefaultTDOLUsed										0x80U
#define MPP_TVR_IssuerAuthenticationFailed							0x40U
#define MPP_TVR_ScriptProcessingFailedBeforeFinalGENERATEAC			0x20U
#define MPP_TVR_ScriptProcessingFailedAfterFinalGENERATEAC			0x10U
#define MPP_TVR_RelayResistanceThresholdExceeded					0x08U
#define MPP_TVR_RelayResistanceTimeLimitsExceeded					0x04U
#define MPP_TVR_RelayResistanceProtocolNotSupported					0x00U
#define MPP_TVR_RRPNotPerformed										0x01U
#define MPP_TVR_RRPPerformed										0x02U

//	9C	Transaction Type
#define MPP_TXT_Purchase											0x00U
#define MPP_TXT_Cash												0x01U
#define MPP_TXT_CashBack											0x09U
#define MPP_TXT_ManualCash											0x12U
#define MPP_TXT_CashDisbursement									0x17U
#define MPP_TXT_Refund												0x20U

//	9F07 Application Usage Control
#define MPP_AUC_ValidForDomesticCashTransactions					0x80U
#define MPP_AUC_ValidForInternationalCashTransactions				0x40U
#define MPP_AUC_ValidForDomesticGoods								0x20U
#define MPP_AUC_ValidForInternationalGoods							0x10U
#define MPP_AUC_ValidForDomesticServices							0x08U
#define MPP_AUC_ValidForInternationalServices						0x04U
#define MPP_AUC_ValidAtATMs											0x02U
#define MPP_AUC_ValidAtTerminalsOtherThanATMs						0x01U
#define MPP_AUC_DomesticCashbackAllowed								0x80U
#define MPP_AUC_InternationalCashbackAllowed						0x40U

//	9F33 Terminal Capabilities
#define MPP_TRC_ManualKeyEntry										0x80U
#define MPP_TRC_MagneticStripe										0x40U
#define MPP_TRC_ICWithContacts										0x20U
#define MPP_TRC_PlaintextPINforICCVerification						0x80U
#define MPP_TRC_EncipheredPINForOnlineVerification					0x40U
#define MPP_TRC_Signature_Paper										0x20U
#define MPP_TRC_EncipheredPINForOfflineVerification					0x10U
#define MPP_TRC_NoCVMRequired										0x08U
#define MPP_TRC_SDA													0x80U
#define MPP_TRC_DDA													0x40U
#define MPP_TRC_CardCapture											0x20U
#define MPP_TRC_CDA													0x08U

//	9F5D Application Capabilities Information
#define MPP_ACI_ACIVersionNumber_Version0							0x00U
#define MPP_ACI_DSVersionNumber_DataStorageNotSupported				0x00U
#define MPP_ACI_DSVersionNumber_Version1							0x01U
#define MPP_ACI_DSVersionNumber_Version2							0x02U
#define MPP_ACI_SupportForFieldOffDetection							0x04U
#define MPP_ACI_SupportForBalanceReading							0x02U
#define MPP_ACI_CDAIndicator_CDASupportedAsInEMV					0x00U
#define MPP_ACI_CDAIndicator_CDASupportedOverTC_ARQC_AAC			0x01U
#define MPP_ACI_SDSSchemeIndicator_UndefinedSDSConfiguration		0x00U
#define MPP_ACI_SDSSchemeIndicator_All10Tags32Bytes					0x01U
#define MPP_ACI_SDSSchemeIndicator_All10Tags48Bytes					0x02U
#define MPP_ACI_SDSSchemeIndicator_All10Tags64Bytes					0x03U
#define MPP_ACI_SDSSchemeIndicator_All10Tags96Bytes					0x04U
#define MPP_ACI_SDSSchemeIndicator_All10Tags128Bytes				0x05U
#define MPP_ACI_SDSSchemeIndicator_All10Tags160Bytes				0x06U
#define MPP_ACI_SDSSchemeIndicator_All10Tags192Bytes				0x07U
#define MPP_ACI_SDSSchemeIndicator_AllSDSTags32Bytes_Except9F78		0x08U

//	9F6F DS Slot Management Control
#define MPP_DSM_PermanentSlotType									0x80U
#define MPP_DSM_VolatileSlotType									0x40U
#define MPP_DSM_LowVolatility										0x20U
#define MPP_DSM_LockedSlot											0x10U
#define MPP_DSM_DeactivatedSlot										0x01U

//	9F7E Mobile Support Indicator
#define MPP_MSI_ODCVMRequired										0x02U
#define MPP_MSI_MobileSupported										0x01U

//	DF4B POS Cardholder Interaction Information
#define MPP_PCI_OfflinePINVerificationSuccessful					0x10U
#define MPP_PCI_ContextIsConflicting								0x08U
#define MPP_PCI_OfflineChangePINRequired							0x04U
#define MPP_PCI_ACKRequired											0x02U
#define MPP_PCI_PINRequired											0x01U

//	DF810A DS ODS Info For Reader
#define MPP_DOI_UsableForTC											0x80U
#define MPP_DOI_UsableForARQC										0x40U
#define MPP_DOI_UsableForAAC										0x20U
#define MPP_DOI_StopIfNoDsOdsTerm									0x04U
#define MPP_DOI_StopIfWriteFailed									0x02U

//	DF810B DS Summary Status
#define MPP_DSS_SuccessfulRead										0x80U
#define MPP_DSS_SuccessfulWrite										0x40U

//	DF810E Post-Gen AC Put Data Status
#define MPP_POS_Completed											0x80U

//	DF810F Pre-Gen AC Put Data Status
#define MPP_PRS_Completed											0x80U

//	DF8114 Reference Control Parameter
#define MPP_RCP_ACTYPE_AAC											0x00U
#define MPP_RCP_ACTYPE_TC											0x40U
#define MPP_RCP_ACTYPE_ARQC											0x80U
#define MPP_RCP_CDASignatureRequested 								0x10U

//	DF8115 Error ID
#define MPP_EID_LV1_OK												0x00U
#define MPP_EID_LV1_TimeOutError									0x01U
#define MPP_EID_LV1_TransmissionError								0x02U
#define MPP_EID_LV1_ProtocolError									0x03U
#define MPP_EID_LV2_OK												0x00U
#define MPP_EID_LV2_CardDataMissing									0x01U
#define MPP_EID_LV2_CamFailed										0x02U
#define MPP_EID_LV2_StatusBytes										0x03U
#define MPP_EID_LV2_ParsingError									0x04U
#define MPP_EID_LV2_MaxLimitExceeded								0x05U
#define MPP_EID_LV2_CardDataError									0x06U
#define MPP_EID_LV2_MagStripeNotSupported							0x07U
#define MPP_EID_LV2_NoPpse											0x08U
#define MPP_EID_LV2_PpseFault										0x09U
#define MPP_EID_LV2_EmptyCandidateList								0x0AU
#define MPP_EID_LV2_IdsReadError									0x0BU
#define MPP_EID_LV2_IdsWriteError									0x0CU
#define MPP_EID_LV2_IdsDataError									0x0DU
#define MPP_EID_LV2_IdsNoMatchingAc									0x0EU
#define MPP_EID_LV2_TerminalDataError								0x0FU
#define MPP_EID_LV3_OK												0x00U
#define MPP_EID_LV3_TimeOut											0x01U
#define MPP_EID_LV3_Stop											0x02U
#define MPP_EID_LV3_AmountNotPresent								0x03U
#define MPP_EID_MOE_CardReadOK										0x17U
#define MPP_EID_MOE_TryAgain										0x21U
#define MPP_EID_MOE_Approved										0x03U
#define MPP_EID_MOE_Approved_Sign									0x1AU
#define MPP_EID_MOE_Declined										0x07U
#define MPP_EID_MOE_Error_OtherCard									0x1CU
#define MPP_EID_MOE_InsertCard										0x1DU
#define MPP_EID_MOE_SeePhone										0x20U
#define MPP_EID_MOE_Authorising_PleaseWait							0x1BU
#define MPP_EID_MOE_ClearDisplay									0x1EU
#define MPP_EID_MOE_NA												0xFFU

//	DF8116 UI Request
#define MPP_UIR_MID_CardReadOK										0x17U
#define MPP_UIR_MID_TryAgain										0x21U
#define MPP_UIR_MID_Approved										0x03U
#define MPP_UIR_MID_Approved_Sign									0x1AU
#define MPP_UIR_MID_Declined										0x07U
#define MPP_UIR_MID_Error_OtherCard									0x1CU
#define MPP_UIR_MID_InsertCard										0x1DU
#define MPP_UIR_MID_SeePhone										0x20U
#define MPP_UIR_MID_Authorising_PleaseWait							0x1BU
#define MPP_UIR_MID_ClearDisplay									0x1EU
#define MPP_UIR_MID_NA												0xFFU
#define MPP_UIR_STATUS_NotReady										0x00U
#define MPP_UIR_STATUS_Idle											0x01U
#define MPP_UIR_STATUS_ReadyToRead									0x02U
#define MPP_UIR_STATUS_Processing									0x03U
#define MPP_UIR_STATUS_CardReadSuccessfully							0x04U
#define MPP_UIR_STATUS_ProcessingError								0x05U
#define MPP_UIR_STATUS_NA											0xFFU
#define MPP_UIR_QUALIFIER_None										0x00U
#define MPP_UIR_QUALIFIER_Amount									0x10U
#define MPP_UIR_QUALIFIER_Balance									0x20U

//	DF811B Kernel Configuration
#define MPP_KCF_MagStripeModeContactlessTransactionsNotSupported	0x80U
#define MPP_KCF_EMVModeContactlessTransactionsNotSupported			0x40U
#define MPP_KCF_OnDeviceCardholderVerificationSupported				0x20U
#define MPP_KCF_RelayResistanceProtocolSupported					0x10U

//	DF8128 IDS Status
#define MPP_IDS_Read												0x80U
#define MPP_IDS_Write												0x40U

//	DF8129 Outcome Parameter Set
#define MPP_OPS_STATUS_Approved										0x10U
#define MPP_OPS_STATUS_Declined         							0x20U
#define MPP_OPS_STATUS_OnlineRequest       							0x30U
#define MPP_OPS_STATUS_EndApplication    							0x40U
#define MPP_OPS_STATUS_SelectNext          							0x50U
#define MPP_OPS_STATUS_TryAnotherInterface  						0x60U
#define MPP_OPS_STATUS_TryAgain             						0x70U
#define MPP_OPS_STATUS_NA                   						0x80U
#define MPP_OPS_START_A												0x00U
#define MPP_OPS_START_B												0x10U
#define MPP_OPS_START_C												0x20U
#define MPP_OPS_START_D												0x30U
#define MPP_OPS_START_NA											0xF0U
#define MPP_OPS_OnlineResponseData_NA								0xF0U
#define MPP_OPS_CVM_NoCVM											0x00U
#define MPP_OPS_CVM_ObtainSignature									0x10U
#define MPP_OPS_CVM_OnlinePin										0x20U
#define MPP_OPS_CVM_ConfirmationCodeVerified						0x30U
#define MPP_OPS_CVM_NA												0xF0U
#define MPP_OPS_UIRequestOnOutcomePresent							0x80U
#define MPP_OPS_UIRequestOnRestartPresent							0x40U
#define MPP_OPS_DataRecordPresent									0x20U
#define MPP_OPS_DiscretionaryDataPresent							0x10U
#define MPP_OPS_Receipt_NA											0x00U
#define MPP_OPS_Receipt_Yes											0x08U
#define MPP_OPS_AlternateInterfacePreference_NA						0xF0U
#define MPP_OPS_FieldOffRequest_NA									0xFFU

//	DF812C/DF811E Mag-stripe CVM Capability ¡V No CVM Required
#define	MPP_MCC_CVM_NoCVM											0x00U
#define	MPP_MCC_CVM_ObtainSignature									0x10U
#define	MPP_MCC_CVM_OnlinePin										0x20U
#define	MPP_MCC_CVM_NA												0xF0U

//		PayPass Self Define Tag - AC Type
#define MPP_ACT_ARQC												0x80U
#define MPP_ACT_TC													0x40U
#define MPP_ACT_AAC													0x00U

//		PayPass Self Define Tag - Next Cmd
#define MPP_NXC_ReadRecord											0x00U
#define MPP_NXC_GetData												0x40U
#define MPP_NXC_None												0x80U

//		PayPass Self Define Tag - ODA Status
#define MPP_ODS_CDA													0x80U

//	Cardholder Verification Rule Format
#define MPP_CVM_METHOD_FailCardholderVerificationIfThisCVMIsUnsuccessful				0x00U
#define MPP_CVM_METHOD_ApplySucceedingCVRuleIfThisCVMIsUnsuccessful						0x40U
#define MPP_CVM_METHOD_FailCVMProcessing												0x00U
#define MPP_CVM_METHOD_PlaintextPINVerificationPerformedByICC							0x01U
#define MPP_CVM_METHOD_EncipheredPINVerifiedOnline										0x02U
#define MPP_CVM_METHOD_PlaintextPINVerificationPerformedByICCAndSignature_Paper			0x03U
#define MPP_CVM_METHOD_EncipheredPINVerificationPerformedByICC							0x04U
#define MPP_CVM_METHOD_EncipheredPINVerificationPerformedByICCAndSignature_Paper		0x05U
#define MPP_CVM_METHOD_Signature_Paper													0x1EU
#define MPP_CVM_METHOD_NoCVMRequired													0x1FU
#define MPP_CVM_CONDITION_Always														0x00U
#define MPP_CVM_CONDITION_IfUnattendedCash												0x01U
#define MPP_CVM_CONDITION_IfNotUnattendedCashAndNotManualCashAndNotPurchaseWithCashback	0x02U
#define MPP_CVM_CONDITION_IfTerminalSupportsTheCVM										0x03U
#define MPP_CVM_CONDITION_IfManualCash													0x04U
#define MPP_CVM_CONDITION_IfPurchaseWithCashback										0x05U
#define MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsUnderXValue		0x06U
#define MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsOverXValue		0x07U
#define MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsUnderYValue		0x08U
#define MPP_CVM_CONDITION_IfTransactionIsInTheApplicationCurrencyAndIsOverYValue		0x09U

//		PayPass Self Define Tag Length
#define MPP_LENGTH_ActiveAFL							252
#define MPP_LENGTH_ActiveTag							2
#define MPP_LENGTH_ACType								1
#define MPP_LENGTH_FailedMSCntr							1
#define MPP_LENGTH_NextCmd								1
#define MPP_LENGTH_nUN									1
#define MPP_LENGTH_ODAStatus							1
#define MPP_LENGTH_ReaderContactlessTransactionLimit	6
#define MPP_LENGTH_StaticDataToBeAuthenticated			2048
#define MPP_LENGTH_TornEntry							1
#define MPP_LENGTH_TornTempRecord						2048

//		ListItem Type
#define MPP_LISTTYPE_TAG		0x00U
#define MPP_LISTTYPE_TLV		0x01U

//		Mag. Stripe Track Number
#define MPP_TRACK_1				1
#define MPP_TRACK_2				2

//		Mag. Stripe Track 2 DD Start Position
#define MPP_DD_START_MSB		0xF0U
#define	MPP_DD_START_LSB		0x0FU

//		Mag. Stripe Track 2 DD Move Direction
#define MPP_MOVE_LEFT			0
#define MPP_MOVE_RIGHT			1

//		PayPass Receive Signal
#define MPP_SIGNAL_ACT			1
#define MPP_SIGNAL_STOP			2
#define MPP_SIGNAL_CLEAN		3
#define MPP_SIGNAL_DET			4
#define MPP_SIGNAL_RA			5
#define MPP_SIGNAL_L1RSP		6
#define MPP_SIGNAL_TIMEOUT		7

//		PayPass State Response Code
#define MPP_STATE_RETURN		0x00U
#define MPP_STATE_1				0x01U
#define MPP_STATE_2				0x02U
#define MPP_STATE_3				0x03U
#define MPP_STATE_4				0x04U
#define MPP_STATE_4Apostrophes	0x05U
#define MPP_STATE_5				0x06U
#define MPP_STATE_6				0x07U
#define MPP_STATE_456_A			0x08U
#define MPP_STATE_7				0x09U
#define MPP_STATE_8				0x0AU
#define MPP_STATE_78_A			0x0BU
#define MPP_STATE_9				0x0CU
#define MPP_STATE_10			0x0DU
#define MPP_STATE_910_A			0x0EU
#define MPP_STATE_910_B			0x0FU
#define MPP_STATE_910_C			0x10U
#define MPP_STATE_11			0x11U
#define MPP_STATE_12			0x12U
#define MPP_STATE_13			0x13U
#define MPP_STATE_14			0x14U
#define MPP_STATE_15			0x15U
#define MPP_STATE_16			0x16U
#define MPP_STATE_17			0x17U
#define MPP_STATE_R1			0x18U
#define MPP_STATE_3R1_D			0x19U
#define MPP_STATE_51			0x51U
#define MPP_STATE_52			0x52U
#define MPP_STATE_53			0x53U
#define MPP_STATE_EXITKERNEL	0xFFU

#define MPP_PROCEDURE_PreGACBalanceReading		0xA0U
#define MPP_PROCEDURE_PostGACBalanceReading		0xA1U
#define MPP_PROCEDURE_CVMSelection				0xA2U
#define MPP_PROCEDURE_PrepareGACCommand			0xA3U
#define MPP_PROCEDURE_ProcessingRestrictions	0xA4U
#define MPP_PROCEDURE_TerminalActionAnalysis	0xA5U

// Data Exchange
#define MPP_DET_BUFFER_RECEIVE	1024
#define MPP_DE_LOG_SIZE 		(1024 * 5)


//	Torn Transaction Record
struct tornrec{
	unsigned short int	PANLen;							//Length of Application PAN
	unsigned char		PAN[10];						//Application PAN
	unsigned char		PSN;							//Application PAN Sequence Number
	unsigned char		DateTime[6];					//Date and Time
	unsigned short int	RecLen;							//Length of Record
//	unsigned char		Record[MPP_TORN_BUFFERSIZE];	//Record Data
	unsigned char		*Record;
}__attribute__((packed));;	
typedef struct tornrec TORNREC;
#define TORNREC_LEN	sizeof(TORNREC)

//Tammy 2017/12/07
// ICS Information
struct icsinfo
{
	unsigned char	title[30];
	unsigned char	name[30];
	unsigned char	version[4];
}__attribute__((packed));;
typedef struct icsinfo ICSINFO;
#define ICSINFO_LEN sizeof(ICSINFO)
