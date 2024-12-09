

extern void		SPI_Close( void );
extern UCHAR	SPI_Open(ULONG bauRate);
extern void		SPI_Read(UINT datLen, UINT * datBuff);
extern void		SPI_Write(UINT datLen, UINT * datBuff);
extern UCHAR	SPI_Transmit(UINT datLen, UCHAR * datBuffer);
extern UCHAR	SPI_Transmit_DebugMode(UINT datLen, UCHAR *datBuffer);


