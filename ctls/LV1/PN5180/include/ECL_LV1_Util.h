

extern void		ECL_LV1_UTI_Beep_1S(void);
extern void		ECL_LV1_UTI_Beep_2S(void);
extern void		ECL_LV1_UTI_ComputeCrc(UCHAR CRCType, char * Data, int Length, BYTE * TransmitFirst, BYTE * TransmitSecond);
extern ULONG	ECL_LV1_UTI_Get_BitRangeValue(ULONG iptData, UCHAR iptRngHigh, UCHAR iptRngLow);
extern UCHAR   *ECL_LV1_UTI_L2P(ULONG iptData);
extern void		ECL_LV1_UTI_Open_Buzzer_1S(void);
extern void		ECL_LV1_UTI_Open_Buzzer_2S(void);
extern UINT		ECL_LV1_UTI_pow(UCHAR numBase, UCHAR numPow);
extern ULONG	ECL_LV1_UTI_Set_BitRangeValue(ULONG iptData, UCHAR iptRngHigh, UCHAR iptRngLow, ULONG iptBitRngValue);
extern void		ECL_LV1_UTI_Wait(ULONG timUnit);

