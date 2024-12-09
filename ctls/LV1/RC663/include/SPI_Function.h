extern UCHAR SPI_Open(unsigned long iptBaudrate);
extern UCHAR SPI_Close(void);
extern UCHAR SPI_Transmit(unsigned int datLen, unsigned char *datBuffer);
// just for compatable  Wayne 2020/08/11
extern UCHAR mml_spi_transmit(int not_use, UCHAR* datBuffer, unsigned int dataLen );
