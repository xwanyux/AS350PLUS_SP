
extern int  	UT_CompareDate( UCHAR *date1, UCHAR *date2 );
extern void 	UT_GetDateTime( UCHAR *rtc );
extern UCHAR 	UT_CheckDateTime( UCHAR rtc[] );
extern UCHAR	UT_CheckMonth(UCHAR rtc []);
extern void 	ETP_UI_Request(UCHAR msgID, UCHAR rqtStatus);

extern UCHAR 	VGL_EMV_OFDA(void);
extern void		VGL_CVM(void);
extern void 	VGL_Online_Process(void);
extern void 	VGL_Offline_Completion(void);
extern void 	VGL_End_Application(void);
extern void		VGL_Try_Another_Interface(void);
extern UCHAR 	VGL_Issuer_Update(void);
extern void 	VGL_RR_MSD_Online_Data(void);
extern UCHAR	VGL_qVSDC_Online_Data(void);
extern void		VGL_Try_Another_Interface(void);
extern void		VGL_WaitTime_Data(UINT tenms);
extern UCHAR	VGL_Issuer_Update(void);

