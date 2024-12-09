//	MPP_DBG_Function.c
extern UCHAR	MPP_DBG_Open_AUX(UCHAR iptPrtNo);
extern void		MPP_DBG_Put_Process(UCHAR iptState, UCHAR iptID, UCHAR iptRspCode);
extern void		MPP_DBG_Put_String(UCHAR iptLen, UCHAR * iptString);

//	MPP_APDU.c
extern UCHAR	MPP_APDU_GET_PROCESSING_OPTIONS(UINT sndLen, UCHAR * sndData, UINT * rcvLen, UCHAR * rcvData);
extern UCHAR	MPP_APDU_READ_RECORD(UCHAR sndRecNumber, UCHAR sndSFI, UINT * rcvLen, UCHAR * rcvData);
extern UCHAR	MPP_APDU_COMPUTE_CRYPTOGRAPHIC_CHECKSUM(UCHAR sndLen, UCHAR * sndData, UINT * rcvLen, UCHAR * rcvData);
extern UCHAR	MPP_APDU_GENERATE_AC(UCHAR iptP1, UCHAR sndLen, UCHAR * sndData, UINT * rcvLen, UCHAR * rcvData);
extern UCHAR	MPP_APDU_GET_DATA(UCHAR iptP1, UCHAR iptP2, UINT * rcvLen, UCHAR * rcvData);
extern UCHAR	MPP_APDU_PUT_DATA(UCHAR iptP1, UCHAR iptP2, UCHAR sndLen, UCHAR * sndData, UINT * rcvLen, UCHAR * rcvData);
extern UCHAR	MPP_APDU_RECOVER_AC(UCHAR sndLen, UCHAR * sndData, UINT * rcvLen, UCHAR * rcvData);
extern UCHAR	MPP_APDU_EXCHANGE_RELAY_RESISTANCE_DATA(UCHAR sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData);

//	MPP_DOL_Function.c
extern UCHAR	MPP_DOL_Get_DOLRelatedData(UINT iptLen, UCHAR * iptDOL, UCHAR * optLen, UCHAR * optData);
extern UCHAR	MPP_DSDOL_Get_DOLRelatedData(UINT iptLen, UCHAR * iptDOL, UCHAR * optLen, UCHAR * optData);

//	MPP_Function.c
extern void		MPP_Add_Signal(UCHAR iptSignal);
extern void		MPP_Insert_Signal(UCHAR iptSignal);
extern UCHAR	MPP_Add_TornRecord(UINT iptRecLen, UCHAR * iptRecData);
extern void		MPP_Clear_DEPBuffer(void);
extern void		MPP_Clear_Signal(void);
extern UCHAR	MPP_Check_CVMMethod_IsRecognized(UCHAR iptMedCode);
extern UCHAR	MPP_Check_CVMMethod_IsSupported(UCHAR iptMedCode);
extern UCHAR	MPP_Check_CVMCondition_IsUnderstood(UCHAR iptCndCode);
extern UCHAR	MPP_Check_CVMCondition_IsSatisfied(UCHAR iptCvmCode, UCHAR iptCndCode);
extern UCHAR	MPP_Check_CVMList_IsLastCVR(UCHAR iptLstIndex);
extern UCHAR	MPP_Check_PhoneMessageTable(UCHAR * optMessage, UCHAR * optStatus);
extern UCHAR	MPP_Check_Separator(UCHAR iptLen, UCHAR * iptData, UCHAR iptTrkNo);
extern UCHAR	MPP_Check_TornTransactionExpire(UCHAR * iptTrnDatetime);
extern UCHAR	MPP_Compare_Date(UCHAR * iptDate1, UCHAR * iptDate2);
extern void		MPP_Copy_DD_Track2(UCHAR * disTrack2, UCHAR disLen, UCHAR * disData, UCHAR disPosition);
extern UCHAR   *MPP_Find_DD_Track1(UCHAR * optLenDD);
extern UCHAR   *MPP_Find_DD_Track2(UCHAR * optLen, UCHAR * optStrPosition);
extern UCHAR	MPP_Generate_DD(UCHAR datLen, UCHAR * disData, UCHAR numUN, UCHAR trkNum);
extern void		MPP_Generate_Y(UCHAR * optLen, UCHAR * optData);
extern ULONG	MPP_Get_BCDRandomNumber_4Byte(void);
extern void		MPP_Get_CVMList_CVR(UCHAR iptLstIndex, UCHAR * optCVR);
extern void		MPP_Get_DDLength_Track2(UCHAR iptLen, UCHAR * optLen, UCHAR disPosition);
extern UCHAR	MPP_Get_ErrorIndication_L1(UCHAR iptRspCode);
extern UINT		MPP_Get_NumberOfNonZeroBits(UINT iptLen, UCHAR * iptData);
extern void		MPP_Get_Signal(void);
extern UCHAR	MPP_Get_TornRecordAIDIndex(void);
extern void		MPP_Load_KernelConfiguration(void);
extern void		MPP_Load_TransactionData(void);
extern void		MPP_Move(UINT iptLen, UCHAR * iptData, UCHAR movDirection, UCHAR movBits);
extern void		MPP_Remove_FirstRecordFromActiveAFL(void);
extern void		MPP_Remove_TornRecord(UCHAR iptRecIndex);
extern void		MPP_Reset_Parameter(void);
extern void		MPP_Reset_PresentList(void);
extern UCHAR	MPP_Retrieve_PK_CA(UCHAR * iptRID, UCHAR iptIndex, UCHAR * optModLen, UCHAR * optModulus, UCHAR * optExponent);
extern UCHAR	MPP_Retrieve_PK_ICC(UCHAR iptModLen, UCHAR * iptModulus, UCHAR * iptExponent, UCHAR * optModLen, UCHAR * optModulus, UCHAR * optExponent);
extern UCHAR	MPP_Retrieve_PK_Issuer(UCHAR iptModLen, UCHAR * iptModulus, UCHAR * iptExponent, UCHAR * optModLen, UCHAR * optModulus, UCHAR * optExponent);
extern UCHAR   *MPP_Search_ListItem(UCHAR * iptTag, UCHAR * lstData, UINT * optRmdDatLen, UCHAR lstType);
extern UCHAR	MPP_Search_TornRecord_Oldest(UCHAR * optRecIndex);
extern UCHAR	MPP_Search_TornRecord_PAN(UCHAR * optRecIndex);
extern void		MPP_Set_DefaultConfigurationData(void);
extern void		MPP_Store_Tag(UCHAR lenOfL, UCHAR lenOfV, UCHAR * iptData, UINT iptTagIndex);
extern UCHAR	MPP_Store_TLV(UINT lenOfTLV, UCHAR * iptData, UCHAR * iptList);
extern UCHAR	MPP_Verify_DynamicSignature(UCHAR iptModLen, UCHAR * iptModulus, UCHAR * iptExponent, UCHAR	flgIDSRead, UCHAR flgRRP);

extern void		MPP_OWHF2(UCHAR *iptPD, UCHAR *optR);
extern void		MPP_OWHF2AES(UCHAR* iptC, UCHAR *optR);
extern UCHAR	MPP_AddToList(UCHAR * iptLstItem, UCHAR * iptList, UCHAR iptLstType);
extern void		MPP_RemoveFromList(UCHAR * iptLstItem, UCHAR * iptList, UCHAR iptLstType);
extern UCHAR	MPP_AddListToList(UCHAR * iptList1, UCHAR * iptList2, UCHAR iptLstType);
extern UCHAR	MPP_GetAndRemoveFromList(UCHAR * iptList, UCHAR * optLstItem, UCHAR iptLstType);
extern UCHAR	MPP_GetNextGetDataTagFromList(UCHAR * iptList, UCHAR * optLstItem, UCHAR iptLstType);
extern UCHAR	MPP_IsEmptyList(UCHAR * iptList);
extern UCHAR	MPP_IsNotEmptyList(UCHAR * iptList);
extern UCHAR	MPP_IsKnown(UCHAR *iptTag);
extern UCHAR	MPP_IsPresent(UCHAR *iptTag);
extern UCHAR	MPP_IsNotPresent(UCHAR * iptTag);
extern UCHAR	MPP_IsEmpty(UCHAR *iptTag);
extern UCHAR	MPP_IsNotEmpty(UCHAR *iptTag);
extern void 	MPP_Initialize_Tag(UCHAR *iptTag);
extern UCHAR	MPP_GetTLV(UCHAR *iptTag, UCHAR *optDatObject);
extern UCHAR	MPP_GetLength(UCHAR *iptTag, UINT *optLength);
extern UCHAR	MPP_ParseAndStoreCardResponse(UINT iptDatLen, UCHAR * iptData);
extern void 	MPP_UpdateWithDetData(/*UINT iptLen,*/ UCHAR *iptData);
extern UCHAR	MPP_Update_ListItem2(UCHAR *iptLstItem, UCHAR *iptList, UCHAR lstType);
extern UCHAR	MPP_DEK();
extern void		MPP_Create_List(UINT lstLength, UCHAR * lstData, UCHAR * iptList);
extern void 	MPP_CreateEMVDataRecord(void);
extern void		MPP_CreateMSDataRecord(void);
extern void 	MPP_CreateEMVDiscretionaryData(void);
extern void		MPP_CreateMSDiscretionaryData(void);
extern void		MPP_MSG(UCHAR * iptTag);
extern void		MPP_OUT(UCHAR * iptTag);

//	MPP_Procedure.c
extern UCHAR	MPP_Procedure_CVMSelection(void);
extern UCHAR	MPP_Procedure_PostGenACBalanceReading(void);
extern UCHAR	MPP_Procedure_PreGenACBalanceReading(void);
extern UCHAR	MPP_Procedure_PrepareGenACCommand(void);
extern UCHAR	MPP_Procedure_ProcessingRestrictions(void);
extern UCHAR	MPP_Procedure_TerminalActionAnalysis(void);

//	MPP_Kernel.c
extern UCHAR	MPP_State_16_WaitingForPreGenACBalance(void);
extern UCHAR	MPP_State_17_WaitingForPostGenACBalance(void);
extern void		MPP_DataExchangeLog(UCHAR * buff, UINT len);

