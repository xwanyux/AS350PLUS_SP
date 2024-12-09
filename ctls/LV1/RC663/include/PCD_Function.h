

extern UCHAR	PCD_ANTICOLLISION(UCHAR selCL, UINT * rcvLen, UCHAR * rcvUID);
extern void		PCD_ANTICOLLISION_Send(UCHAR selCL);
extern UCHAR	PCD_ATTRIB(UCHAR * iptPUPI, UINT * rcvLen, UCHAR * rcvATA);
extern void		PCD_ATTRIB_Send(UCHAR * iptPUPI);
extern UCHAR	PCD_DEP(UINT sndLen, UCHAR * sndData, UINT * rcvLen, UCHAR * rcvData);
extern UCHAR	PCD_DEP_WithTimeout(UINT sndLen, UCHAR * sndData, UINT * rcvLen, UCHAR * rcvData, ULONG iptTimeout);
extern UCHAR	PCD_DEP_Send(UCHAR blkType, UINT sndLen, UCHAR * sndData);
extern void		PCD_Get_Card_Type(UCHAR * crdType);
extern UCHAR	PCD_HLTA(void);
extern UCHAR	PCD_RATS(UINT * rcvLen, UCHAR * rcvATS);
extern void		PCD_RATS_Send(void);
extern void		PCD_Reset_EmvParameter(void);
extern UCHAR	PCD_REQA(UINT * rcvLen, UCHAR * rcvATQA);
extern UCHAR	PCD_SELECT(UCHAR selCL, UCHAR * selUID, UINT * rcvLen, UCHAR * rcvSAK);
extern void		PCD_SELECT_Send(UCHAR selCL, UCHAR * selUID);
extern UCHAR	PCD_WUPA(UINT * rcvLen, UCHAR * rcvATQA);
extern void		PCD_WUPA_Send(void);
extern UCHAR	PCD_WUPB(UINT * rcvLen, UCHAR * rcvATQB);
extern void		PCD_WUPB_Send(void);

