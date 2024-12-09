
extern	void	api_pcd_vap_pol_POLL(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_pol_Echo(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);

extern	void	api_pcd_vap_deb_SetDebugAndOptimizationMode(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_deb_SetParameters(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);

extern	void	api_pcd_vap_aut_InitializeCommunication(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_aut_MutualAauthenticate(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_aut_GenerateKey(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_aut_InvalidateReader(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);

extern	void	api_pcd_vap_tra_ReadyForSale(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_tra_Reset(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_tra_ShowStatus(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_tra_Issuer_Update(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);


extern	void	api_pcd_vap_adm_SwitchToAdminMode(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_adm_GetCapability(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_adm_SetCapability(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_adm_GetDateAndTime(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_adm_SetDateAndTime(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_adm_GetParameters(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_adm_GetEMVTagsValues(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_adm_SetEMVTagsValues(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_adm_GetDisplayMessages(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern	void	api_pcd_vap_adm_SetDisplayMessages(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);

extern 	void 	api_pcd_vap_adm_SetVisaPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern 	void 	api_pcd_vap_adm_GetVisaPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);

extern 	void 	api_pcd_vap_adm_SetMasterPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern 	void 	api_pcd_vap_adm_GetMasterPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);

extern	void 	api_pcd_vap_adm_SetJCBPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern	void 	api_pcd_vap_adm_GetJCBPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);

extern 	void 	api_pcd_vap_adm_SetUPIPublicKey(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern 	void 	api_pcd_vap_adm_GetUPIPublicKey(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);

extern  void 	api_pcd_vap_adm_SetVisaTestAIDPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern  void 	api_pcd_vap_adm_GetVisaTestAIDPublicKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);

extern	void	api_pcd_vap_adm_SetCapability(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData);
extern	void	api_pcd_vap_adm_GetCapability(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData);

extern	void	api_pcd_vap_adm_SetCVMCapability(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData);
extern	void	api_pcd_vap_adm_GetCVMCapability(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData);

extern 	void 	api_pcd_vap_adm_GetAPID(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern 	void 	api_pcd_vap_adm_SetAPID(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);

extern 	void 	api_pcd_vap_adm_GetExceptionFile(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData);
extern 	void 	api_pcd_vap_adm_SetExceptionFile(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData);

extern  void 	api_pcd_vap_adm_GetDRLEnable(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern  void 	api_pcd_vap_adm_SetDRLEnable(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern 	void 	api_pcd_vap_adm_GetCashRRP(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern 	void 	api_pcd_vap_adm_SetCashRRP(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern 	void 	api_pcd_vap_adm_GetCashBackRRP(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern 	void 	api_pcd_vap_adm_SetCashBackRRP(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);

extern void 	api_pcd_vap_adm_GetKeyRevoList(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void 	api_pcd_vap_adm_SetKeyRevoList(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);

extern void		api_pcd_vap_adm_SetBaudRate(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData);
extern void		api_pcd_vap_adm_GetBaudRate(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData);

extern void		api_pcd_vap_adm_ResetAcquirerKey(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData);
extern void		api_pcd_vap_adm_ReaderRecover(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData);

extern void		api_pcd_vap_adm_GetPayPassConfigurationData(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_vap_adm_SetPayPassConfigurationData(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_vap_adm_GetPayPassDataExchangeData(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_vap_adm_SetPayPassDataExchangeData(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);

extern void		api_pcd_vap_adm_GetQuickPassMultipleAIDData(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_vap_adm_SetQuickPassMultipleAIDData(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_vap_adm_GetQuickPassMultipleAIDEnable(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_vap_adm_SetQuickPassMultipleAIDEnable(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);

extern void 	api_pcd_vap_adm_GetMSDOption(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void 	api_pcd_vap_adm_SetMSDOption(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);

extern void 	api_pcd_vap_adm_GetExpressPayDefaultDRL(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void 	api_pcd_vap_adm_SetExpressPayDefaultDRL(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void 	api_pcd_vap_adm_GetExpressPayDRL(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void 	api_pcd_vap_adm_SetExpressPayDRL(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void 	api_pcd_vap_adm_GetExpressPayKeyRevoList(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void 	api_pcd_vap_adm_SetExpressPayKeyRevoList(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void 	api_pcd_vap_adm_GetExpressPayExceptionFile(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void 	api_pcd_vap_adm_SetExpressPayExceptionFile(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);

extern void 	api_pcd_vap_adm_GetAEPublicKey(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void 	api_pcd_vap_adm_SetAEPublicKey(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);

extern void 	api_pcd_vap_adm_GetDiscoverPublicKey(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void 	api_pcd_vap_adm_SetDiscoverPublicKey(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);

extern void		api_pcd_vap_pro_SetVLPSupportIndicator(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_vap_pro_GetVLPSupportIndicator(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_vap_pro_KeyInjection(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData);
extern void		api_pcd_vap_pro_GetEMVTags(UCHAR * iptMsgLength,UCHAR * iptData,UCHAR * optMsgLength,UCHAR * optData);

extern void		api_pcd_vap_sys_SetReadyForSaleBeep(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_vap_sys_SetRFRangeUnder4cm(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_vap_sys_SetRFRangeEMV(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_vap_sys_SetJCBRefundResponse(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
