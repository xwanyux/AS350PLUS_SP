

extern void		api_pcd_mfc_con_RFOn(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void		api_pcd_mfc_con_RFOff(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void		api_pcd_mfc_con_REQA(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void		api_pcd_mfc_con_WUPA(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_con_AntiCol(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void		api_pcd_mfc_con_Select(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_con_WUPB(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_con_LoadKey(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_con_Authenticate(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_con_Read(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_con_Write(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_con_Value(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_con_Restore(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_con_Transfer(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_con_TypeATransparent(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_con_TypeBTransparent(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_con_Polling(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_con_GetCardData(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void		api_pcd_mfc_sys_GetReaderInformation(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void		api_pcd_mfc_sys_GetFirmwareInformation(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void		api_pcd_mfc_fil_UpdateFileInformation(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void		api_pcd_mfc_fil_ReceiveFile(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void		api_pcd_mfc_pro_GetCAPKIndex(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void		api_pcd_mfc_pro_DeleteAllCAPKKey(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void		api_pcd_mfc_sys_GetContactlessChipSerialNumber(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_sys_GetDateTime(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_sys_SetDateTime(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_lcd_ClearScreen(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_lcd_ClearLine(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_lcd_ClearLineAlphanumeric(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_lcd_WriteLine(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_lcd_WriteLineAlphanumeric(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_lcd_SwitchBackLight(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_lcd_ShowContactlessSymbol(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_lcd_SetBGPalette(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_led_SetLED(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_led_BlinkLED(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_buz_Beep(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_aux_ECHOTest(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_aux_ChangeBaudrate(UCHAR *iptMsgLength, UCHAR *iptData, UCHAR *optMsgLength, UCHAR *optData);
extern void		api_pcd_mfc_iso_ActivateCard(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_iso_GetStatus(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_iso_TransmitAPDU(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_iso_CardRemoval(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);

#ifdef _PLATFORM_AS350_LITE
#ifdef _ACQUIRER_NCCC
extern void		api_pcd_mfc_n3c_GetParameterInformation(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_n3c_ShowError(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
extern void		api_pcd_mfc_n3c_Reset(UCHAR * iptMsgLength, UCHAR * iptData, UCHAR * optMsgLength, UCHAR * optData);
#endif
#endif

