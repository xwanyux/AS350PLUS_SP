
extern	void	DBG_Close_AUX(void);
extern	UCHAR	DBG_Open_AUX(UCHAR iptPrtNo);
extern	void	DBG_Put_DateTime(void);
extern	void	DBG_Put_Dialog(UCHAR iptDirection, UINT iptLen, UCHAR * iptData);
extern	void	DBG_Put_Hex(UINT iptLen, UCHAR *iptHex);
extern	void	DBG_Put_UCHAR(UCHAR iptUCHAR);
extern	void	DBG_Put_UINT(UINT iptUINT);
extern	void	DBG_Put_ULONG(ULONG iptULONG);
extern	void	DBG_Put_String(UCHAR iptLen, UCHAR *iptString);
extern	void	DBG_Put_Text(char *iptText);
