

extern UCHAR	NXP_ANTICOLLISION_Receive(UINT * rcvLen, UCHAR * rcvCLn);
extern void		NXP_ANTICOLLISION_Send(UCHAR selCL, ULONG rcvTimeout);
extern UCHAR	NXP_ATTRIB_Receive(UINT * rcvLen, UCHAR * rcvATA);
extern void		NXP_ATTRIB_Send(UCHAR * cmdATTRIB, ULONG rcvTimeout);
extern UCHAR	NXP_AUTHENTICATION(UCHAR iptAutType, UCHAR iptAddress, UCHAR * iptUID);
extern UCHAR	NXP_Check_Register(UCHAR regAddress, UCHAR regBit);
extern UCHAR	NXP_Check_SPI_RC663(void);
extern UCHAR	NXP_DECREMENT(UCHAR iptAddress, UCHAR * iptValue);
extern UCHAR	NXP_DEP_Receive(UCHAR crdType, UINT * rcvLen, UCHAR * rcvData);
extern void		NXP_DEP_Send(UCHAR crdType, UINT datLen, UCHAR * datBuffer, ULONG rcvTimeout);
extern UCHAR	NXP_Get_CLChipSerialNumber(UCHAR * optData);
extern void		NXP_HLTA_Send(ULONG rcvTimeout);
extern UCHAR	NXP_HLTB_Receive(UINT * rcvLen, UCHAR * rcvData);
extern void		NXP_HLTB_Send(UCHAR * iptPUPI, ULONG rcvTimeout);
extern void		NXP_Initialize_Reader(void);
extern UCHAR	NXP_INCREMENT(UCHAR iptAddress, UCHAR * iptValue);
extern void		NXP_Load_Protocol_A(void);
extern void		NXP_Load_RC663_Parameter_AS350(void);
extern void		NXP_Load_RC663_Parameter_TA(void);
extern UCHAR	NXP_LOADKEY(UCHAR * iptKey);
extern UCHAR	NXP_RATS_Receive(UINT * rcvLen, UCHAR * rcvATS);
extern void		NXP_RATS_Send(UCHAR ratPARAM, ULONG rcvTimeout);
extern void		NXP_Read_Register(UCHAR regAddress, UCHAR *regData);
extern UCHAR	NXP_READ(UCHAR iptAddress, UCHAR * optData);
extern void		NXP_REQA_Send(ULONG rcvTimeout);
extern UCHAR	NXP_REQA_Receive(UINT * rcvLen, UCHAR * rcvATQA);
extern UCHAR	NXP_REQB_Receive(UINT * rcvLen, UCHAR * rcvATQB);
extern void		NXP_REQB_Send(ULONG rcvTimeout);
extern UCHAR	NXP_RESTORE(UCHAR iptAddress);
extern UCHAR	NXP_SELECT_Receive(UINT * rcvLen, UCHAR * rcvSAK);
extern void		NXP_SELECT_Send(UCHAR selCL, UCHAR * selUID, UCHAR uidBCC, ULONG rcvTimeout);
extern void		NXP_Switch_FieldOff(void);
extern void		NXP_Switch_FieldOn(void);
extern UCHAR	NXP_TRANSFER(UCHAR iptAddress);
extern void		NXP_Write_Register(UCHAR regAddress, UCHAR regData);
extern UCHAR	NXP_WRITE(UCHAR iptAddress, UCHAR * iptData);
extern UCHAR	NXP_WUPA_Receive(UINT * rcvLen, UCHAR * rcvATQA);
extern void		NXP_WUPA_Send(ULONG rcvTimeout);
extern UCHAR	NXP_WUPB_Receive(UINT * rcvLen, UCHAR * rcvATQB);
extern void		NXP_WUPB_Send(ULONG rcvTimeout);

extern UCHAR	NXP_AV2_AUTHENTICATION_1ST(UCHAR iptAutType, UCHAR iptAddress, UCHAR * optData);
extern UCHAR	NXP_AV2_AUTHENTICATION_2ND(UCHAR * iptData, UCHAR * optData);
extern UCHAR	NXP_AV2_TRANSCEIVE(UINT iptLen, UCHAR * iptData, UINT * optLen, UCHAR * optData);
