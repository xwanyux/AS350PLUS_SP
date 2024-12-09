//#include "DataStructure.h"
#include "Datastructure.h"

extern UCHAR	Load_CAPK(void);


//	DOL_Function.c
extern UCHAR	DOL_Get_DOLData(UINT iptLen,UCHAR * iptDOL,UCHAR * optLen,UCHAR * optData);
extern UCHAR	DOL_Patch_DOLData(UINT iptLen,UCHAR * iptDOL,UCHAR * optLen,UCHAR * optData);


//	ECL_Tag.c
extern void		ECL_Reset_Tag(void);


//	ETP_EntryPoint.c
extern void		ETP_Load_Parameter(void);
extern void		ETP_Initialize(void);
extern void		ETP_EntryPoint(void);
extern void		ETP_Clear_CandidateList(void);
extern void 	ETP_Clear_TransactionData(void);
extern void 	ETP_Add_TransactionDataFromParameter(void);
extern UCHAR	ETP_Add_TransactionData(UCHAR * iptTLV);
extern void		ETP_Remove_CandidateList(void);
extern void		ETP_Initialize_CombinationTable(void);
extern void		ETP_Initialize_EntryPointConfiguration(void);
extern void		ETP_UI_Request(UCHAR msgID,UCHAR rqtStatus);
extern void		ETP_Set_DefaultOutcome(UCHAR ocmResult);
extern void		ETP_Load_MPP_ConfigurationData(void);
extern void		ETP_Load_TransactionData(void);

//	Flash_Function.c
extern	UCHAR	FLS_Read_EncryptionKey(UCHAR iptKeyType, UCHAR iptKeyIndex, UCHAR * optKey);
extern	UCHAR	FLS_Write_EncryptionKey(UCHAR iptKeyType, UCHAR iptKeyIndex, UCHAR * iptKey);
extern  UCHAR 	FLS_Read_PayPassConfigurationData(UINT *optLength, UCHAR *optData);
extern	UCHAR	FLS_Write_PayPassConfigurationData(UINT iptLength, UCHAR * iptData);
extern	UCHAR	FLS_Read_QuickPassMultipleAIDData(UINT * optLength, UCHAR * optData);
extern	UCHAR	FLS_Write_QuickPassMultipleAIDData(UINT iptLength, UCHAR * iptData);
extern	UCHAR	FLS_Read_DPASMultipleAIDData(UINT * optLength, UCHAR * optData);
extern	UCHAR	FLS_Write_DPASMultipleAIDData(UINT iptLength, UCHAR * iptData);


//	TEI_Function.c
extern UCHAR	TEI_Get_AUXCommand_STOP(void);
extern void		TEI_Upload_Data(UINT sndLen, UCHAR * sndData);

//	Utils.c
extern void 	UT_1hexto3asc(UCHAR data,UCHAR * ascout);
extern UINT		UT_C2S(UCHAR * iptData);
extern void 	UT_Check_Padding(UINT iptLen, UINT iptPstLen, UCHAR *iptData, UINT *optPadLen);
extern UCHAR	UT_Check_EMVTagEncodingError(UINT iptLen, UCHAR * iptData);
extern UCHAR 	UT_CheckYearMonthDate(UCHAR rtc[]);
extern int		UT_CompareDate(UCHAR * date1,UCHAR * date2);
extern void 	UT_Disp_Show_Status(UCHAR disp_len, UCHAR *disp_str, UCHAR *line);
extern void 	UT_LED_F_Off_S_On(UCHAR id1,UCHAR id2,UCHAR state);
extern void		UT_LED_SingleAct(UCHAR id1, UCHAR state);
extern void		UT_LED_Switch(UCHAR id,UCHAR state);
extern void		UT_BUZ_Beep1L(void);
extern void		UT_Buz_Option(UCHAR Flag);
extern void		UT_asc2bcd(UCHAR bcdlen,UCHAR * bcd,UCHAR * str);
extern UCHAR 	UT_bcd_add_bcd(UCHAR bcdlen,UCHAR * bcd1,UCHAR * bcd2);
extern void		UT_bcd2hex(UCHAR bcdlen, UCHAR * bcd, UCHAR * hex);
extern UCHAR	UT_bcdcmp(UCHAR* bcdOne, UCHAR* bcdTwo, UCHAR cmpLen);
extern void		UT_BUZ_Beep1(void);
extern void		UT_BUZ_Beep_Delay(UINT iptDelay);
extern void		UT_BUZ_Success(void);
extern ULONG	UT_C2L(UCHAR * iptData);
extern UCHAR	UT_Check_PrimitiveTag(UCHAR * iptTag);
extern UCHAR	UT_Check_PrivateClassTag(UCHAR * iptTag);
extern UCHAR 	UT_Check_SW12(UCHAR *datSW, UINT chkSW);
extern UCHAR 	UT_CheckAUX(void);
extern UCHAR	UT_CheckDateTime(UCHAR rtc [ ]);
extern void		UT_ClearRow(UCHAR row, UCHAR cnt, UCHAR font);
extern void		UT_ClearScreen(void);
extern UCHAR	UT_CNcmp(UCHAR * s1,UCHAR * s2,UCHAR len);
extern UCHAR	UT_CNcmp2(UCHAR * s1,UCHAR * s2,UCHAR len);
extern void 	UT_Compress(UCHAR *des, UCHAR *src, UCHAR pair_len);
extern void		UT_DispHexByte(UCHAR row, UCHAR col, UCHAR data);
extern void		UT_DispHexWord(UCHAR row,UCHAR col,UINT data);
extern void		UT_DumpHex(UCHAR mode, UCHAR row, UINT length, UCHAR * data);
extern UCHAR   *UT_Find_Tag(UCHAR * iptTag, UINT iptLen, UCHAR * iptData);
extern UCHAR   *UT_Find_TagInDOL(UCHAR * iptTag, UINT iptLen, UCHAR * iptDOL);
extern UCHAR	UT_Get_TLVLength(UCHAR * iptDataOfT, UCHAR * optLenOfT, UCHAR * optLenOfL, UINT * optLenOfV);
extern UCHAR	UT_Get_TLVLengthOfT(UCHAR * iptDataOfT, UCHAR * optLength);
extern UCHAR	UT_Get_TLVLengthOfL(UCHAR * iptDataOfL, UCHAR * optLength);
extern UCHAR	UT_Get_TLVLengthOfV(UCHAR * iptDataOfL, UINT * optLength);
extern ULONG	UT_Get_UnixTime(UCHAR * iptDatetime);
extern void		UT_GetDateTime(UCHAR * rtc);
extern UCHAR	UT_GetKey(void);
extern UCHAR	UT_GetKeyStatus(void);
extern void		UT_GHL_Trans_BIG5(UCHAR *iptWord, UCHAR *optWord);
extern void 	UT_Handle_2Line_Message(UCHAR *iptmsg,UINT iptLen, UCHAR *optFirstMsg, UCHAR *optFirstLen,UCHAR *optSecMsg, UCHAR *optSecLen);
extern void 	UT_Handle_2Type_Message(UCHAR *iptmsg,UINT iptLen, UCHAR *optFirstMsg, UCHAR *optFirstLen,UCHAR *optSecMsg, UCHAR *optSecLen);
extern void		UT_hexb2ascw( UCHAR data, UCHAR *byte_h, UCHAR *byte_l );
extern void		UT_INT2ASC(ULONG iptInt, UCHAR * optAscii);
extern UCHAR	UT_itoa(UINT value, UCHAR * abuf);
extern void		UT_L2C(ULONG iptData, UCHAR * optData);
extern int		UT_memcmp(UCHAR * s1,UCHAR * s2,UCHAR len);
extern UCHAR	UT_min(UCHAR numA, UCHAR numB);
extern void 	UT_OpenBuzzer_1L(void);
extern void		UT_OpenBuzzer_1S(void);
extern void		UT_OpenKey_ALL(void);
extern UINT		UT_pow(UCHAR numBase, UCHAR numPow);
extern void		UT_PutMsg(UCHAR row,UCHAR pos,UCHAR font,UCHAR len,UCHAR * msg);
extern void		UT_PutStr(UCHAR row, UCHAR col, UCHAR font, UCHAR len, UCHAR * msg);
extern void		UT_PutStr_SetPalette(UCHAR row, UCHAR col, UCHAR font, UCHAR len, UCHAR * msg, UCHAR * iptPalette);
extern UCHAR 	UT_ReceiveAUX(UCHAR * data);
extern UCHAR	UT_Remove_PaddingData(UINT iptLen, UCHAR * iptData, UINT * optLen, UCHAR * optData);
extern void		UT_S2C(UINT iptData, UCHAR * optData);
extern UCHAR	UT_Search(UCHAR keyStrSize, UCHAR * keyString, UCHAR * srhString, ULONG recNumber, UCHAR recSize, ULONG * optIndex);
extern UCHAR	UT_Search_Record(UCHAR keyStrSize, UCHAR * keyString, UCHAR * srhString, UINT recNumber, UCHAR recSize, UINT * optIndex);
extern void		UT_Set_LEDSignal(UINT iptID, UINT iptBlkOn, UINT iptBlkOff);
extern void		UT_Set_LED(UINT iptID);
extern UCHAR	UT_Set_TagLength(UINT iptLength,UCHAR * optLength);
extern UCHAR 	UT_SetCentury(signed char year);
extern void 	UT_Split(UCHAR *des, UCHAR *src, char pair_len);
extern void 	UT_SwapData(UCHAR length,UCHAR * data);
extern UCHAR 	UT_TransmitAUX(UCHAR * data);
extern UCHAR 	UT_Tx_AUX(UCHAR dhnAux, UCHAR * sndData);
extern ULONG	UT_UCHAR2ULONG(UCHAR lenUCHAR, UCHAR *iptUCHAR);
extern UCHAR	UT_VerifyCertificateExpDate(UCHAR * cdate);
extern void		UT_Wait(ULONG timUnit);
extern UCHAR	UT_WaitKey(void);
extern void		UT_WaitTime(UINT tenms);


//	VAP_ReaderInterface.c
extern UCHAR	VAP_RIF_Close_AUX(void);
extern UCHAR	VAP_RIF_Open_AUX(void);
extern void		VAP_RIF_Get_ReaderInterfaceInstruction(void);
extern UCHAR	VAP_RIF_Check_ResetCommand(void);
extern void		VAP_RIF_Check_ShowStatusCommand(void);

extern UCHAR	VAP_RIF_Check_StopPolling(void);
extern UCHAR	VAP_RIF_Check_StopRemoval(void);


//	VGL_Kernel.c
extern void		VGL_Start_Kernel(UINT lenFCI, UCHAR *datFCI);
extern UCHAR 	VGL_Dynamic_Reader_Limit(PREDRLLIMITSET *ID);
extern UCHAR 	VGL_Issuer_Update(void);
extern void 	VGL_DRL_Preprocess(void);
extern void		VGL_M_Tag(UCHAR * Buf);
extern UCHAR 	VGL_Chip_Data(UCHAR Tag_Total_Len,UCHAR * taglist);
extern UCHAR 	VGL_AS210_D1_Track(void);
extern UCHAR	VGL_Check_PAN_Consistency(void);

// VAP_Kernel.c
extern void		VAP_Start_Kernel(UINT lenFCI,UCHAR * datFCI);

//JCB_Kernel.c
extern void 	JCB_Start_Kernel(UINT lenFCI,UCHAR * datFCI);
extern UCHAR 	JCB_Check_ResetCommand(void);
extern UCHAR	JCB_Check_Cancel(void);
extern UCHAR	JCB_GET_L3_RCCode(void);
extern UCHAR	JCB_GET_SchemeID(void);
extern void		JCB_Update_OnlineData(void);

//	UNP_Kernel.c
extern void		UNP_Start_Kernel(UINT lenFCI, UCHAR * datFCI);

//	AEX_Kernel.c
extern void		AEX_Start_Kernel(UINT lenFCI, UCHAR * datFCI);

//	MPP_Kernel.c
extern void		MPP_Start_Kernel(UINT lenFCI, UCHAR *datFCI);

//	DPS_Kernel.c
extern void		DPS_Start_Kernel(UINT lenFCI, UCHAR * datFCI);
extern UCHAR	DPS_7bit_LRC(UCHAR *buf, UINT len, UCHAR track);
extern UCHAR	DPS_Check_ZIP_AID(void);
extern void		DPS_Approved_OutCome(void);
extern void		DPS_Online_Request_OutCome(void);
extern void		DPS_Online_Request_For_Two_Presentments(void);
extern void		DPS_Declined_OutCome(void);
extern void		DPS_Try_Another_Interface_OutCome(void);
extern void		DPS_End_Application_OutCome(void);
extern void		DPS_Select_Next_OutCome(void);
extern UCHAR	DPS_DOL_Get_DOLData(UINT iptLen, UCHAR *iptDOL, UCHAR *optLen, UCHAR *optData);
extern void		DPS_OutcomeProcess(void);
extern void		DPS_DBG_Put_Process(UCHAR iptFigure, UCHAR iptID, UCHAR iptRspCode);


extern void		TEI_Upload_Receipt(UINT sndLen, UCHAR * sndData);
extern void		TEI_Upload_RRP_Data(UINT sndLen, UCHAR * sndData);
extern void		TEI_Upload_DE_Data(UINT sndLen, UCHAR * sndData);


//api_pcd_vap_Function
extern UCHAR 	api_pcd_qVSDC_Purchase(UCHAR * salAmount,UINT * rspLength,UCHAR * rspData);
extern UCHAR 	api_pcd_qVSDC_Purchase_Cashback(UCHAR * salAmount,UCHAR * CashbackAmount,UINT * rspLength,UCHAR * rspData);
extern UCHAR 	api_pcd_qVSDC_Cash(UCHAR * salAmount,UINT * rspLength,UCHAR * rspData);
extern UCHAR 	api_pcd_qVSDC_Refund(UCHAR * salAmount,UINT * rspLength,UCHAR * rspData);
extern UCHAR 	api_pcd_JCB_Purchase_Cashback(UCHAR *salAmount, UCHAR *CashbackAmount ,UINT *rspLength, UCHAR *rspData);
extern UCHAR	api_pcd_vap_IAD(UINT *HostScriptLen,UCHAR *ScriptData,UCHAR *HostScriptResult);
extern void 	api_pcd_JCB_Approval(void);
extern void 	api_pcd_JCB_Decline(void);


extern	void	APP_Open_ContactlessInterface(void);
extern	void	APP_Close_ContactlessInterface(void);
extern	void	APP_L3_VISA_ReaderInterface(void);


extern UCHAR ECL_APDU_Internal_Authenticate(UINT sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData);
extern UCHAR ECL_APDU_Generate_AC(UCHAR Ref_Con_Para, UINT sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData);
extern UCHAR ECL_APDU_Get_Magstripe_Data(UCHAR Ref_Con_Para, UINT sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData);
extern UCHAR ECL_APDU_Get_ProcessingOptions(UINT sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData);
extern UCHAR ECL_APDU_Read_Record(UCHAR sndRecNumber, UCHAR sndSFI, UINT *rcvLen, UCHAR *rcvData);
extern UCHAR ECL_APDU_Select_PPSE(UINT *rcvLen, UCHAR *rcvData);


