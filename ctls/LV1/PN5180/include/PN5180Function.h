#include "POSAPI.h"
//	APK_FTAG.c
extern UCHAR	apk_EMVCL_CheckConsTag(UCHAR tag);
extern UCHAR	apk_EMVCL_CheckWordTag(UCHAR tag);
extern UCHAR	apk_EMVCL_GetBERLEN(UCHAR * de, UCHAR * cnt, UINT * optValue);
extern UCHAR	*apk_EMVCL_FindTag(UCHAR tag1, UCHAR tag2, UCHAR * data);
extern UCHAR	apk_EMVCL_FindTagDOL(UCHAR tag1, UCHAR tag2, UCHAR * dol);
extern UCHAR	apk_EMVCL_ParseTLV(UCHAR reclen [ ], UCHAR * ptrobj, UCHAR padlen);
extern UCHAR	apk_EMVCL_ParseLenFCI(UCHAR * ptrfci, UCHAR * padlen);
extern UCHAR	apk_EMVCL_SetBERLEN(UCHAR orgLen,UCHAR berLen [ ]);
extern void		apk_EMVCL_CheckIsoPadding_Left(UCHAR * rec);
extern UCHAR	apk_EMVCL_CheckIsoPadding_Right(UCHAR * rec,UCHAR * padlen);
extern UINT		apk_a_GetBERLEN( UCHAR *de, UCHAR *cnt );
extern UCHAR 	apk_EMVCL_FindTagDOL( UCHAR tag1, UCHAR tag2, UCHAR *dol );


//	Utils.c
extern void		UT_asc2bcd(UCHAR bcdlen,UCHAR * bcd,UCHAR * str);
extern UCHAR 	UT_bcd_add_bcd(UCHAR bcdlen,UCHAR * bcd1,UCHAR * bcd2);
extern void		UT_bcd2hex(UCHAR bcdlen, UCHAR * bcd, UCHAR * hex);
extern UCHAR	UT_bcdcmp(UCHAR* bcdOne, UCHAR* bcdTwo, UCHAR cmpLen);
extern void		UT_BUZ_Beep1(void);
extern void		UT_BUZ_Beep1L(void);
extern void		UT_BUZ_Alert(void);
extern void		UT_BUZ_Success(void);
extern UCHAR	UT_Check_PrimitiveTag(UCHAR * iptTag);
extern UCHAR	UT_Check_PrivateClassTag(UCHAR * iptTag);
extern UCHAR 	UT_Check_SW12(UCHAR *datSW, UINT chkSW);
extern UCHAR	UT_Check_TLV(UINT iptLen, UCHAR * iptData);
extern UCHAR 	UT_CheckAUX(void);
extern void		UT_ClearRow(UCHAR row, UCHAR cnt, UCHAR font);
extern void		UT_ClearScreen(void);
extern UCHAR	UT_CNcmp(UCHAR * s1,UCHAR * s2,UCHAR len);
extern UCHAR	UT_CNcmp2(UCHAR * s1,UCHAR * s2,UCHAR len);
extern void 	UT_Compress(UCHAR *des, UCHAR *src, UCHAR pair_len);
extern UCHAR 	UT_DataReceiveAUX( UCHAR *data );
extern void		UT_DispHexByte(UCHAR row, UCHAR col, UCHAR data);
extern void		UT_DispHexWord(UCHAR row,UCHAR col,UINT data);
extern void		UT_DumpHex(UCHAR mode, UCHAR row, UINT length, UCHAR * data);
extern void 	UT_DumpHexData(UCHAR mode,UCHAR row,UINT length,UCHAR * data);
extern void 	UT_DumpHexData2(UCHAR mode,UCHAR row,UINT length,UCHAR * data);
extern UCHAR	*UT_Find_Tag(UCHAR * iptTag, UINT iptLen, UCHAR * iptData);
extern UCHAR	UT_Get_TLVLength(UCHAR * iptDataOfT, UCHAR * optLenOfT, UCHAR * optLenOfL, UINT * optLenOfV);
extern UCHAR	UT_Get_TLVLengthOfT(UCHAR * iptDataOfT, UCHAR * optLength);
extern UCHAR	UT_Get_TLVLengthOfL(UCHAR * iptDataOfL, UCHAR * optLength);
extern UCHAR	UT_Get_TLVLengthOfV(UCHAR * iptDataOfL, UINT * optLength);
extern ULONG	UT_Get_UnixTime(UCHAR * iptDatetime);
extern void		UT_GetDateTime(UCHAR * rtc);
extern UCHAR	UT_GetKey(void);
extern UCHAR	UT_GetKeyStatus(void);
extern UCHAR 	UT_GetNumKey(UINT tout,UCHAR type,UCHAR idle,UCHAR font,UCHAR row,UCHAR len,UCHAR * buf);
extern void		UT_hexb2ascw( UCHAR data, UCHAR *byte_h, UCHAR *byte_l );
extern void		UT_INT2ASC(ULONG iptInt, UCHAR * optAscii);
extern int		UT_memcmp(UCHAR * s1,UCHAR * s2,UCHAR len);
extern UCHAR	UT_min(UCHAR numA, UCHAR numB);
extern UCHAR	UT_Open_AUX(UCHAR auxPort, UINT bauRate, UINT parTOB, UINT parTOR);
extern UCHAR	UT_OpenAUX(UCHAR port);
extern void		UT_OpenBuzzer_1L(void);
extern void		UT_OpenBuzzer_1S(void);
extern void		UT_OpenBuzzer_Alert(void);
extern void		UT_OpenBuzzer_Success(void);
extern void		UT_OpenDisplay(void);
extern void		UT_OpenKey_ALL(void);
extern UINT		UT_pow(UCHAR numBase, UCHAR numPow);
extern void		UT_PutChar(UCHAR row, UCHAR col, UCHAR font, UCHAR data);
extern void		UT_PutMsg(UCHAR row,UCHAR pos,UCHAR font,UCHAR len,UCHAR * msg);
extern void		UT_PutStr(UCHAR row, UCHAR col, UCHAR font, UCHAR len, UCHAR * msg);
extern UCHAR 	UT_ReceiveAUX(UCHAR * data);
extern UCHAR	UT_Search(UCHAR keyStrSize, UCHAR * keyString, UCHAR * srhString, ULONG recNumber, UCHAR recSize, ULONG * optIndex);
extern UCHAR	UT_Search_Record(UCHAR keyStrSize, UCHAR * keyString, UCHAR * srhString, UINT recNumber, UCHAR recSize, UINT * optIndex);
extern UCHAR	*UT_Search_TLV(UCHAR * iptTag, UINT tlvNumber, UCHAR * tlvData, UINT * optIndex);
extern void		UT_Set_LEDSignal(UINT iptID, UINT iptBlkOn, UINT iptBlkOff);
extern UCHAR	UT_Set_TagLength(UINT iptLength,UCHAR * optLength);
extern UCHAR	UT_SetCentury(signed char year);
extern void 	UT_Split(UCHAR *des, UCHAR *src, char pair_len);
extern UCHAR 	UT_TransmitAUX(UCHAR * data);
extern UCHAR 	UT_Tx_AUX(UCHAR dhnAux, UCHAR * sndData);
extern UCHAR	UT_VerifyCertificateExpDate(UCHAR * cdate);
extern void		UT_Wait(ULONG timUnit);
extern UCHAR	UT_WaitKey(void);
extern void		UT_WaitTime(UINT tenms);
extern void		UT_1hexto3asc(UCHAR data,UCHAR * ascout);
extern UCHAR	UT_ClearRowEx(UCHAR row, UCHAR cnt, UCHAR font, UCHAR *palette);
extern void		UT_ClearLine(UCHAR row, UCHAR cnt, UCHAR font);



