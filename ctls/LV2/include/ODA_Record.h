
#define ODA_BUFFER_SIZE_APDU	1024
#define ODA_RECORD_NUMBER		30
#define ODA_RECORD_SIZE			270
#define ODA_BUFFER_SIZE_RECORD	(ODA_RECORD_NUMBER*ODA_RECORD_SIZE)


extern unsigned char oda_bufRecord[ODA_BUFFER_SIZE_RECORD];
extern unsigned char oda_bufRspGAC[ODA_BUFFER_SIZE_APDU];
extern unsigned char oda_bufRspGPO[ODA_BUFFER_SIZE_APDU];

extern void ODA_Clear_Record(void);
extern void ODA_Clear_GACResponse(void);
extern void ODA_Clear_GPOResponse(void);
