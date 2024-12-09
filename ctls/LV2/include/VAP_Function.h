
extern int  	UT_CompareDate( UCHAR *date1, UCHAR *date2 );
extern void 	UT_GetDateTime( UCHAR *rtc );
extern UCHAR 	UT_CheckDateTime( UCHAR rtc[] );
extern UCHAR	UT_CheckMonth(UCHAR rtc []);

extern void 	ETP_UI_Request(UCHAR msgID, UCHAR rqtStatus);

extern UCHAR 	VAP_EMV_OFDA(void);
extern void 	VAP_Switch_Interface (void);
extern void 	VAP_End_Application(void);
extern UCHAR 	VAP_Check_SW12(UCHAR * datSW);
extern void		VAP_M_Tag(UCHAR * Buf);

