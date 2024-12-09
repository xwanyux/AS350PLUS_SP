//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : ZA9L                                                       **
//**  PRODUCT  : AS350							    **
//**                                                                        **
//**  FILE     : POSAPI.h                                                   **
//**  MODULE   : Declaration of related APIs.				    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2007/08/03                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2007-2015 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _POSAPI_H_
#define _POSAPI_H_
//----------------------------------------------------------------------------
// for support LCDTFRAPI

/*
** Common Variable Type Declaration
*/
typedef	unsigned char			UCHAR;		// UINT8
typedef	unsigned char			UINT8;		// UINT8
typedef	unsigned short int		UINT;		// UINT16
typedef	unsigned short int		UINT16;		// UINT16
typedef unsigned long int		ULONG;		// UINT32
typedef unsigned long int		UINT32;		// UINT32
typedef	unsigned char			BYTE;		// UINT8
typedef	unsigned short int		WORD;		// UINT16
/*
** API Return Value
*/
#define	apiOK				0x00		//
#define apiFailed			0x01		//
#define apiReady			0x00		//
#define apiNotReady			0x02		//
#define apiErrorInput			0x03		//
#define apiDeviceNotOpen		0x04		//
#define apiNoDevice			0x05		//
#define apiDeviceError			0x06		//
#define apiOutOfLink			0x10		//
#define apiOutOfService			0xFF		//
#define TRUE					1		//
#define FALSE					0		//

/*
** Font ID and Attribute
*/
#define	FONT0				0x00		// default, 5x7 alphanumeric
#define FONT1				0x01		// default, 7x9 alphanumeric
#define	FONT2				0x02		// user-defined, recommended 16x16 in SRAM
#define	FONT3				0x03		// user-defined, recommended 16x16 in FLASH
#define	FONT4				0x04		// user-defined, recommended 24x24 in SRAM
#define	FONT5				0x05		// user-defined, recommended 24x24 in FLASH
#define	FONT10				0x0A		// printer font, traditional Chinease 24x24
#define	FONT11				0x0B		// printer font, simplified  Chinease 24x24
#define	FONT12				0x0C		// printer font, user-defined
#define	FONT13				0x0D		// printer font, user-defined
#define	FONT14				0x0E		// printer font, user-defined
#define	FONT15				0x0F

#define	attrNORMAL			0x00		// normal pattern writing
#define	attrOVERWRITE			0x00		// overwrite current row
#define	attrREVERSE			0x10		// reverse pattern writing
#define	attrCLEARWRITE			0x20		// clear row before writing
#define	attrPIXCOLUMN			0x40		// set column cursor to pixel resolution
#define	attrPIXCURSOR			0x40		// set column & row cursor to pixel resolution
#define attrX2HEIGHT 		0x80
#define	attrISO				0x80		// using ISO 8859 code definition (RFU)

#define	BMP_OW				0x00		// bitmap method: OverWrite
#define	BMP_OR				0x01		// bitmap method: OR
#define	BMP_AND				0x02		// bitmap method: AND
#define	BMP_XOR				0x03		// bitmap method: XOR

#define	FT_UNUSED			0x00		// font type
#define	FT_ASCII			0x01		//
#define	FT_BIG5				0x02		//
#define	FT_GB				0x03		//
#define	FT_UNICODE			0x04		//


typedef	struct	API_GRAPH_S
{
	ULONG			Xleft;			// start pixel column coordinate at the upper left
	ULONG			Ytop;			// start pixel row coordinate at the upper left
	ULONG			Width;			// image width in dots in horizontal direction
	ULONG			Height;			// image height in dots in vertical direction
	ULONG			Method;			// method of writing graphics (RFU)
} API_GRAPH;


typedef struct	API_LCD_FONT_S				// Font Descriptor Table, size=16 bytes (for LCD)
{
	UCHAR			FontID;			// font id
	UCHAR			ByteNo;			// number of bytes of one char image
	UCHAR			Width;			// width of the font in dots
	UCHAR			Height;			// height of the font in dots
	UCHAR			codPageNo;		// page number of the code table (effective only for page MMU)
	UCHAR			*codStAddr;		// start address of char code table
	UCHAR			*codEndAddr;		// end   address of char code table
	UCHAR			bmpPageNo;		// page number of the bitmap table (effective only for page MMU)
	UCHAR			*bmpStAddr;		// start address of char image table
	UCHAR			*bmpEndAddr;		// end   address of char image table
} API_LCD_FONT;


typedef struct	API_PRT_FONT_S				// Font Descriptor Table (for PRINTER)
{
	UCHAR			FontID;			// font id (FONT10..FONT1x)
	UCHAR			Height;			// height of the font in dots (for both AN & GC)
	
	UCHAR			AN_Type;		// Alphanumerics, font type (FT_xxx)
	UCHAR			AN_CodeNo;		// Alphanumerics, number of bytes of one char coding
	ULONG			AN_ByteNo;		// Alphanumerics, number of bytes of one char image
	ULONG			AN_CharNo;		// Alphanumerics, number of character set
	UCHAR			AN_Width;		// Alphanumerics, width of the font in dots
	UCHAR			*AN_BitMap;		// Alphanumerics, start address of bitmap
	UCHAR			*AN_CodeMap;		// Alphanumerics, start address of code table

	UCHAR			GC_Type;		// Graphic Character, font type (FT_xxx)
	UCHAR			GC_CodeNo;		// Alphanumerics, number of bytes of one char coding
	ULONG			GC_ByteNo;		// Graphic Character, number of bytes of one char image
	ULONG			GC_CharNo;		// Graphic Character, number of character set
	UCHAR			GC_Width;		// Graphic Character, width of the font in dots
	UCHAR			*GC_BitMap;		// Graphic Character, start address of bitmap
	UCHAR			*GC_CodeMap;		// Graphic Character, start address of code table
} API_PRT_FONT;


/*
** ICC
*/
#define	ICC1				0x00
#define	SAM1				0x01
#define SAM2				0x02
#define SAM3				0x03
#define SAM4				0x04
/*
** DSS
*/
#define	DSS_MAX_APP_SIZE	1024*1024*10	// 10MB for single APP structure
// #define	_MODEM_ENABLED_		1					
#define	_LAN_ENABLED_		1					
#define	_FS_ENABLED_		1					
#define	DEBUG				1					
/*
** MSR Track & Status
*/
#define	isoTrack1			0x01
#define	isoTrack2			0x02
#define	isoTrack3			0x04

#define	msrNoSwiped			0x00
#define	msrSwiped			0x01
#define	msrNoData			0x00
#define	msrDataOK			0x01
#define	msrDataError			0x02

/*
** SRAM Access Parameters
*/
typedef	struct API_SRAM_S
{
	UCHAR			StPage;				// start page no
	UCHAR			EndPage;			// end   page no
	ULONG			StAddr;				// start address
	ULONG			EndAddr;			// end   address
	ULONG			Len;				// data length
} API_SRAM;

typedef	struct API_SRAM_ADDR_S
{
	ULONG			StAddr;				// start address
	ULONG			EndAddr;			// end   address
} __attribute__((packed)) API_SRAM_ADDR;

/*
** AUX Port Parameters
*/
/* Ports */
#define	COM0				0x00//mini USB port
#define	COM1				0x01//RJ12 RS232 port
#define	COM2				0x02

/* Protocol */
#define	auxBYPASS			0x00			// protocol: transparent
#define	auxSOH				0x01			// protocol: SOH hexLEN(2) hexMSG(N) LRC
#define	auxDLL				0x02			// protocol: SOH hexLEN(1) hexMSG(N) LRC
#define	auxSTX				0x03			// protocol: STX txtMSG(N) ETX LRC
#define	auxSTXB				0x04			// protocol: STX bcdLEN(2) hexMSG(N) ETX LRC
#define	auxVISAWAVE			0x05			// protocol: STX hexMSG(N) CRC(2) ETX
#define	auxBARCODE			0x06			// protocol: txtMSG(N) CR
#define	auxSVC				0x0A			// protocol: STX hexMSG(N) CRC(2) ETX
#define	auxDLESTX			0x0B			// protocol: DLE STX hexMSG(N) DLE ETX LRC
#define	auxSTXLEN			0x0C			// protocol: STX LEN(2) TEXT CRCC(2)
#define	auxDLESTX2			0x0D			// protocol: DLE STX LEN(2) hexMSG(N) LRC DLE ETX (IPASS)

/* Data Bits */
#define	COM_CHR7			0x02
#define	COM_CHR8			0x03

/* Stop Bits */
#define	COM_STOP1			0x00
#define	COM_STOP2			0x04

/* Parity */
#define	COM_NOPARITY			0x00
#define	COM_ODDPARITY			0x08
#define	COM_EVENPARITY			0x18

/* Baud Rate */
#define	COM_300				0x00
#define	COM_600				0x20
#define	COM_1200			0x40
#define	COM_2400			0x60
#define	COM_4800			0x80
#define	COM_9600			0xA0
#define	COM_19200			0xC0
#define	COM_28800			0xE00
#define	COM_38400			0xE0
#define	COM_57600			0x100
#define	COM_115200			0x200
#define	COM_230400			0x400
#define	COM_460800			0x800
#define	COM_921600			0xA00
#define	COM_1228800			0xC00

typedef struct API_AUX_S				// AUX port structure
{
	UCHAR			Mode;			// data link protocol
	UINT			Baud;			// baud rate
	UINT			Tob;			// inter-byte timeout for receiving character string
	UINT			Tor;			// timeout of waiting for response
	UCHAR			Resend;			// re-transmit limit
	UCHAR			Acks;			// no. of repetitive acknowledgement to the received message
							// The following members are optional: (do not set by default)
	ULONG			TxFlag;			//   tx operation flag
	ULONG			RxFlag;			//   rx operation flag						
	ULONG			BufferSize;		//   size of the transmit and receive driver buffer (in bytes)
	ULONG			FlowCtrl;		//   flow control
	ULONG			IoConfig;		//   io control for RTC,CTS,DTR,DSR...
} API_AUX;

/*
** Printer
*/
//print type
#define prtSprocket			0x01
#define prtRoll				0x02
#define prtSlip				0x04
#define prtJournal			0x08
#define prtText				0x10
#define prtGraphic			0x20
#define prtDotMatrix			0x40
#define prtThermal			0x80

//print status
#define prtIdle				0x00
#define prtBusy				0x01
#define prtBufferFull 			0x02
#define prtOffLine			0x03
#define prtPaperEmpty			0x04
#define prtPaperJam			0x05
#define prtComplete			0x06
#define prtProcessing 			0x80

/*
** System Information ID
** NOTE: Please refer to api_sys_info().
*/
#define	SID_BIOSversion			0x00		// OS kernel version
#define	SID_BSPversion			0x01		// BSP version number
#define	SID_MODEMversion		0x02		// modem driver version
#define	SID_PRINTERversion		0x03		// printer driver version

#define	SID_TerminalSerialNumber2	0x0A		// TSN(8) from barcode sticker of product
#define	SID_TerminalSerialNumber	0x0B		// CID(3) + CSN(8) from MCU chip
#define	SID_ChineseFontVersion		0x0C		//
#define	SID_ReleaseDateTime		0x0D		// kernel release date & time
#define	SID_McuSN			0x0E		// MCU serial number (fixed 4 hex bytes)

#define	SID_TCPIPversion		0x10		// OS-embedded library versions
#define	SID_FTPversion			0x11		//
#define	SID_FTPSversion			0x12		//
#define	SID_SSLversion			0x13		//
#define	SID_CRYPTOversion		0x14		//
#define	SID_POSAPIversion		0x15		//
#define	SID_USBversion			0x16		//
#define	SID_LCDTFTversion		0x17		//
#define	SID_TSCversion			0x18		//
#define	SID_MEversion			0x19		// mobile equipment (GPRS, WCDMA, LTE...)

/*
** HASH Mode
*/
#define	HASH_MD5			0x00
#define	HASH_SHA1			0x01
#define	HASH_SHA2_224			0x02
#define	HASH_SHA2_256			0x03
#define	HASH_SHA2_384			0x04
#define	HASH_SHA2_512			0x05

/*
** RSA
*/
#define	RSA_ENCRYPT			0x00
#define	RSA_DECRYPT			0x01

typedef	struct	API_RSA_S
{
	UCHAR			Mode;			// 0=public key encryption, 1=private key decription
	ULONG			ModLen;			// size of modulus (N) in bytes
	UCHAR			*Modulus;		// modulus (N)
	ULONG			ExpLen;			// size of exponent (e or d) in bytes
	UCHAR			*Exponent;		// exponent (e or d)
	ULONG			Length;			// size of data to be transformed in bytes
	UCHAR			*pIn;			// data to be transformed
	UCHAR			*pOut;			// data transformed
} API_RSA;

/*
** RTC
*/
#define GMTp12 0x8C     //GMT+12
#define GMTp11 0x8B     //GMT+11
#define GMTp10 0x8A     //GMT+10
#define GMTp9  0x89     //GMT+9
#define GMTp8  0x88     //GMT+8
#define GMTp7  0x87     //GMT+7
#define GMTp6  0x86     //GMT+6
#define GMTp5  0x85     //GMT+5
#define GMTp4  0x84     //GMT+4
#define GMTp3  0x83     //GMT+3
#define GMTp2  0x82     //GMT+2
#define GMTp1  0x81     //GMT+1
#define GMT0   0x00     //GMT+0
#define GMTm1  0x01     //GMT-1
#define GMTm2  0x02     //GMT-2
#define GMTm3  0x03     //GMT-3
#define GMTm4  0x04     //GMT-4
#define GMTm5  0x05     //GMT-5
#define GMTm6  0x06     //GMT-6
#define GMTm7  0x07     //GMT-7
#define GMTm8  0x08     //GMT-8
#define GMTm9  0x09     //GMT-9
#define GMTm10 0x0A     //GMT-10
#define GMTm11 0x0B     //GMT-11
#define GMTm12 0x0C     //GMT-12
#define GMTm13 0x0D     //GMT-13
#define GMTm14 0x0E     //GMT-14

/*
** MODEM
*/
// modem status
#define mdmIdle              		0x00
#define mdmConnecting        		0x01
#define mdmConnected         		0x02
#define mdmLocalLineBusy     		0x03
#define mdmRemoteLineBusy    		0x04
#define mdmRequestDropLine   		0x05
#define mdmLineDropped       		0x06
#define mdmRinging           		0x07
#define mdmFailed            		0x08
#define mdmRedialing         		0x09
#define mdmNoLine            		0x10
#define mdmProcessing        		0x80

// modem data link protocol
#define mdmBYPASS            		0x00		// No protocol, transparent
#define mdmSDLC              		0x01		// Protocol-SDLC
#define mdmVISA12            		0x02		// Protocol-VISA12
#define mdmXMODEM            		0x03		// Protocol-XMODEM
#define mdmSTXB              		0x08		// Protocol-STXBIN
#define mdmREFERRAL          		0x80		// Referral dial-up

#define MDM_300    			0x20		//V.21
#define MDM_1200    			0x40		//V.22
#define MDM_2400   			0x60		//V.22 bias
#define MDM_9600    			0x80		//V.32
#define MDM_14400  			0xA0		//V.32 bias
#define MDM_33600  			0xC0		//V.34
#define MDM_56000  			0x00		//V.90

#define	MDM_BELL			0x8000		// BELL 103 or 212

typedef	struct	API_HOST_S
{
	UCHAR  			country ;       // modem Country Code 
	UCHAR  			address ;       // node address of the station. (only effective in SDLC mode)
	UCHAR  			mode ;          // modem data link protocol.
	UINT   			baud ;          // modem communication data format.
	UCHAR  			rdpi ;          // pause interval (seconds) for redialing.
	UCHAR  			attempt1 ;      // no. of attempts to connect host by the primary phone number.
	UCHAR  			cdon1 ;         // connection time (seconds) for the primary phone number.
	UCHAR  			len1 ;          // length of the primary phone number.
	UCHAR  			phone1[30] ;    // primary phone number. ( including  'T', 'P', ',' , '0' to '9' )
	UCHAR  			attempt2 ;      // no. of attempts to connect host by the secondary phone number.
	UCHAR  			cdon2 ;         // connection time (seconds) for the secondary phone number.
	UCHAR  			len2 ;          // length of the secondary phone number.
	UCHAR  			phone2[30] ;    // secondary phone number. ( including  'T', 'P', ',' , '0' to '9' )
} API_HOST;

/*
** LAN (Ethernet)
*/
// LAN status
#define lanIdle              		0x00
#define	lanDisconnected				0x00
#define lanConnecting        		0x01
#define lanConnected         		0x02
#define lanLineDropped       		0x06
#define lanProcessing				0x80

//#define LAN_MAX_CONNECTION 			8	// 2023-03-14, reduce max connection number from 10 to 8 for DHN range (0x98..0x9F)
//#define LAN_CONNECTION_SECTIME_MAX		10*60	// 2023-10-31, moved to API_LAN.c

typedef	struct	API_LAN_S
{
	UCHAR			LenType;	// 0 = HEX, 1 = BCD
	UINT  			ClientPort;	// client port number
						// if C_port = 65100, the C_port will be auto increased by 1 for next connection
	UINT			ServerPort;	// server port number, such as 8000
	UINT			ServerIP_Len;	// length of server ip string
	UCHAR			ServerIP[16];	// server ip string, such as "172.16.1.100"
} API_LAN;

typedef	struct	API_IPCONFIG_S
{
	UINT			IP_Len;		// length of IP
	UCHAR 			IP[16];		// client IP string
	UINT			Gateway_Len;	// length of GATEWAY
	UCHAR 			Gateway[16];	// gateway IP string
	UINT			SubnetMask_Len;	// length of SUBNET MASK
	UCHAR 			SubnetMask[16];	// subnet mask IP string
	UCHAR			MAC[6];		// MAC address
} API_IPCONFIG;

typedef struct LAN_SOCKET_S
{
	UINT		socketFD;//socket file description
    ULONG       ConnectTime;//Time stamp when connection establish
    API_LAN		APIconfig;
    UINT        RFU;
}LAN_SOCKET;
/*
 *TIMER
 */
 #define TimerNumbers 20
//----------------------------------------------------------------------------
//		Function Prototypes
//----------------------------------------------------------------------------
extern	int	SSL_dhn_lan;
/*
** API: TIMER
*/
extern	UCHAR	api_tim_open( UCHAR unit );
extern	UCHAR	api_tim_close( UCHAR dhn );
extern	UCHAR	api_tim_gettick( UCHAR dhn, UCHAR *dbuf );

extern	UCHAR	api_tim_open2( UCHAR unit, UCHAR mode, ULONG ivalue );
extern	UCHAR	api_tim_close2( UCHAR dhn );
extern	UCHAR	api_tim_gettick2( UCHAR dhn, UCHAR *dbuf );

extern	UCHAR	api_tim_open3( UCHAR unit, UCHAR mode, ULONG ivalue );
extern	UCHAR	api_tim_close3( UCHAR dhn );
extern	UCHAR	api_tim_gettick3( UCHAR dhn, UCHAR *dbuf );
// micro sec timer for CTLS
extern	UCHAR	api_tim2_open( ULONG microSec );
extern	UCHAR	api_tim2_close( UCHAR dhn );
extern	UCHAR	api_tim2_status( UCHAR dhn );

extern	UCHAR	api_tim3_open( ULONG microSec );
extern	UCHAR	api_tim3_close( UCHAR dhn );
extern	UCHAR	api_tim3_gettick( UCHAR dhn, ULONG *dbuf );/*
** API: BUZZER
*/
extern	UCHAR	api_buz_open( UCHAR *sbuf );
extern	UCHAR	api_buz_close( UCHAR dhn );
extern	UCHAR 	api_buz_sound( UCHAR dhn );

/*
** API: RTC
*/
extern	UCHAR	api_rtc_open( void );
extern	UCHAR	api_rtc_close( UCHAR dhn );
extern	UCHAR	api_rtc_getdatetime( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR	api_rtc_setdatetime( UCHAR dhn, UCHAR *sbuf );
extern	UCHAR	api_rtc_gettimezone(UCHAR dhn, UCHAR *data);
extern	UCHAR 	api_rtc_settimezone(UCHAR dhn, UCHAR data);
extern	UCHAR   api_rtc_sync_NTP_switch( UCHAR dhn, UCHAR action );

/*
** API: KEYBOARD
*/
extern	UCHAR	api_kbd_open( UCHAR deviceid, UCHAR *sbuf );
extern	UCHAR	api_kbd_close( UINT16 dhn );
extern	UCHAR	api_kbd_status( UINT16 dhn, UCHAR *dbuf );
extern	UCHAR	api_kbd_getchar( UINT16 dhn, UCHAR *dbuf );
extern	UCHAR	api_kbd_get_multiple_char( UINT16 dhn, UCHAR *dbuf);

/*
** API: IFM
*/
extern	UCHAR	api_ifm_open( UCHAR acceptor );
extern	UCHAR	api_ifm_close( UCHAR dhn );
extern	UCHAR	api_ifm_deactivate( UCHAR dhn );
extern	UCHAR	api_ifm_present( UCHAR dhn );
extern	UCHAR	api_ifm_reset( UCHAR dhn, UCHAR mode, UCHAR *dbuf );
extern	UCHAR	api_ifm_exchangeAPDU( UCHAR dhn, UCHAR *c_apdu, UCHAR *r_apdu );

/*
** API: CIPHER
*/
extern	ULONG	api_des_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey );
extern	ULONG	api_des_decipher( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey );
extern	ULONG	api_3des_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey );
extern	ULONG	api_3des_encipher2( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR inLen );
extern	ULONG	api_3des_decipher( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey );
extern	ULONG	api_3des_decipher2( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey, UCHAR inLen );
extern  ULONG	api_3des_cbc_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize, UCHAR inLen, UCHAR *iv, UCHAR ivLen );
extern  ULONG	api_3des_cbc_decipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize, UCHAR inLen, UCHAR *iv, UCHAR ivLen );
extern	ULONG	api_rsa_loadkey( UCHAR *modulus, UCHAR *exponent );
extern	ULONG	api_rsa_recover( UCHAR *pIn, UCHAR *pOut );
//extern	ULONG	api_rsa_encrypt( UCHAR *sbuf );
extern  ULONG	api_rsa_encrypt( API_RSA cipher );
extern	void	api_rsa_release( void );
extern	ULONG	api_aes_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize );
extern  ULONG	api_aes_encipher2( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize, UCHAR inLen );
extern	ULONG	api_aes_decipher( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey, UCHAR KeySize );
extern  ULONG	api_aes_decipher2( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey, UCHAR KeySize, UCHAR inLen );

/*
** API: SYSTEM
*/
extern	ULONG	api_sys_random( UCHAR *dbuf );
extern	ULONG	api_sys_SHA1Init( void );
extern	ULONG	api_sys_SHA1Update( ULONG length, UCHAR *data );
extern	ULONG	api_sys_SHA1Final( ULONG length, UCHAR *digest );
extern	ULONG	api_sys_SHA1( ULONG length, UCHAR *data, UCHAR *digest );
extern	ULONG	api_sys_info( UCHAR id, UCHAR *info );
extern	void	api_sys_reset( UCHAR target );

extern	ULONG	api_sys_MD5( ULONG length, UCHAR *data, UCHAR *digest );
extern	ULONG	api_sys_SHA2( UCHAR mode, ULONG length, UCHAR *data, UCHAR *digest );

extern	ULONG	api_sys_gettick( void );
extern	void	api_sys_delayNms( ULONG ms );
extern	ULONG	api_sys_writeSECM( ULONG addr, ULONG len, UCHAR *data );
extern	ULONG	api_sys_readSECM( ULONG addr, ULONG len, UCHAR *data );
extern  UCHAR	api_sys_backlight( UCHAR device, ULONG duration );

extern  void	api_sys_genMAC( UCHAR *mac_b, UCHAR *mac_s );
extern  void	api_sys_genMAC_PSEUDO( UCHAR *mac_b, UCHAR *mac_s );

/*
** API: MSR
*/
extern	UCHAR	api_msr_open( UCHAR tracks );
extern	UCHAR	api_msr_close( UCHAR dhn );
extern	UCHAR	api_msr_status( UCHAR dhn, UCHAR action, UCHAR *dbuf );
extern	UCHAR	api_msr_getstring( UCHAR dhn, UCHAR *sbuf, UCHAR *dbuf );

/*
** API: LCD
*/
// extern	UCHAR	api_lcdtft_open( UCHAR deviceid );
// extern	UCHAR	api_lcdtft_close( UCHAR dhn );
// extern	UCHAR	api_lcdtft_clear( UCHAR dhn, API_LCDTFT_PARA para );
// extern  UCHAR   api_lcdtft_putstring( UCHAR dhn, API_LCDTFT_PARA para, UCHAR *dbuf );
// extern 	UCHAR   api_lcdtft_showPCD(UCHAR dhn, API_PCD_ICON icon);
// extern	UCHAR	api_lcdtft_putgraphics( UCHAR dhn, API_LCDTFT_GRAPH graph);
// extern	UCHAR	api_lcdtft_showICON( UCHAR dhn, API_LCDTFT_ICON icon );
// extern	UCHAR	api_lcdtft_putwinbmp( UCHAR dhn, API_LCDTFT_WINBMP *bmppara, UCHAR *winbmp );
// extern	UCHAR	api_lcdtft_fillRECT( UCHAR dhn, API_LCDTFT_RECT rect);
// extern   UCHAR  SIGNPAD_lcdtft_putstring( UCHAR dhn, API_LCDTFT_PARA para, UCHAR *dbuf, UINT CCWdegrees );
// extern	UCHAR	api_lcd_open( UCHAR deviceid );
// extern	UCHAR	api_lcd_close( UCHAR dhn );
// extern	UCHAR	api_lcd_clear( UCHAR dhn, UCHAR *sbuf );
// extern	UCHAR	api_lcd_putstring( UCHAR dhn, UCHAR *sbuf, UCHAR *dbuf );
// extern	UCHAR	api_lcd_initfont( API_LCD_FONT ft );
// extern	void	api_lcd_convertfont( UCHAR flag );

/*
** API: SRAM
** NOTE: Please DO NOT call the following functions if File System is in use.
*/
// extern	ULONG	api_sram_PageInit( void );
// extern	ULONG	api_sram_PageLink( UCHAR *sbuf, UCHAR Action );
// extern	ULONG	api_sram_PageSelect( UCHAR Page, UCHAR *dbuf );
// extern	ULONG	api_sram_PageRead( UCHAR *sbuf, UCHAR *pData );
// extern	ULONG	api_sram_PageWrite( UCHAR *sbuf, UCHAR *pData );
// extern	ULONG	api_sram_PageClear( UCHAR *sbuf, UCHAR Pattern );


// Change in type
extern	ULONG	api_sram_PageInit( void );
extern	ULONG	api_sram_PageLink( API_SRAM pSram, UCHAR Action );
extern	ULONG	api_sram_PageSelect( UCHAR Page, API_SRAM_ADDR *pSram );
extern	ULONG	api_sram_PageRead( API_SRAM pSram, UCHAR *pData );
extern	ULONG	api_sram_PageWrite( API_SRAM pSram, UCHAR *pData );
extern	ULONG	api_sram_PageClear( API_SRAM pSram, UCHAR Pattern );
extern	void	api_sram_GlobalClear( void );

extern	void 	os_sram_init();
extern	UCHAR 	api_sram_write(UCHAR* address, ULONG length, UCHAR* data );
extern	UCHAR 	api_sram_read(UCHAR* address, ULONG length, UCHAR* data);
extern	UCHAR 	api_sram_clear(UCHAR* address, ULONG length, UCHAR pattern );

/*
** API: AUX
*/




extern	UCHAR	api_aux_open( UCHAR port, API_AUX pAux );
extern	UCHAR 	api_aux_close( UCHAR dhn );
extern	UCHAR	api_aux_rxready( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR	api_aux_rxstring( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR	api_aux_txready( UCHAR dhn );
extern	UCHAR	api_aux_txstring( UCHAR dhn, UCHAR *sbuf );

/*
** API: PRINTER
*/
extern	UCHAR	api_prt_open( UCHAR type, UCHAR mode );
extern	UCHAR	api_prt_close( UCHAR dhn );
extern	UCHAR	api_prt_status( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR	api_prt_putstring( UCHAR dhn, UCHAR fontid, UCHAR *sbuf );
//extern	UCHAR	api_prt_putgraphics( UCHAR dhn, UCHAR *sbuf, UCHAR *image );
extern	UCHAR	api_prt_setlinespace( UCHAR dhn, UCHAR space );
extern	UCHAR	api_prt_getlinespace( UCHAR dhn, UCHAR *space );
//extern	UCHAR	api_prt_initfont( UCHAR *sbuf  );
//extern	UCHAR	api_prt_getfont( UCHAR fontid, UCHAR *dbuf  );


extern	UCHAR	api_prt_putgraphics( UCHAR dhn, API_GRAPH dim, UCHAR *image );
extern	UCHAR	api_prt_initfont( API_PRT_FONT ft  );
extern	UCHAR	api_prt_getfont( UCHAR fontid, API_PRT_FONT *ft  );

/*
** API: MODEM
*/
//extern	UCHAR api_mdm_open( UCHAR *atcmd, UCHAR *sbuf );
extern 	UCHAR api_mdm_open( UCHAR *atcmd, API_HOST *sbuf );
extern	UCHAR api_mdm_close( UCHAR dhn , UINT delay );
extern	UCHAR api_mdm_status( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR api_mdm_txready( UCHAR dhn );
extern	UCHAR api_mdm_rxready( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR api_mdm_txstring( UCHAR dhn, UCHAR *sbuf );
extern	UCHAR api_mdm_rxstring( UCHAR dhn, UCHAR *dbuf );


/*
** API: LAN (Ethernet)
*/
extern	UCHAR api_lan_open( API_LAN lan );
extern	UCHAR api_lan_close( UCHAR dhn );
extern	UCHAR api_lan_status( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR api_lan_rxready( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR api_lan_rxstring( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR api_lan_txready( UCHAR dhn );
extern	UCHAR api_lan_txstring( UCHAR dhn, UCHAR *sbuf );
extern	UCHAR api_lan_ping( UCHAR ip_len, UCHAR *ip, ULONG *ms );
extern	UCHAR api_lan_setIPconfig( API_IPCONFIG config );
extern	UCHAR api_lan_getIPconfig( API_IPCONFIG *config );
extern	UCHAR api_lan_setup_DHCP( UCHAR flag );
extern	UCHAR api_lan_status_DHCP( void );
extern  UCHAR api_lan_lstatus ( void );


/*
** API: WDT (Watch Dog Timer)
*/
extern	UCHAR	api_wdt_init( void );
extern	UCHAR	api_wdt_clear( void );

/*
** API: TOOL
*/
extern  UCHAR   api_tl_itoa( UINT value, UCHAR *abuf );
extern  UCHAR	api_tl_ltoa( ULONG value, UCHAR *abuf );
extern  ULONG   api_tl_numw2decb( UCHAR *buf );
extern  UCHAR   api_tl_ascw2hexb( UCHAR *buf );
extern  void    api_tl_hexb2ascw( UCHAR data, UCHAR *byte_h, UCHAR *byte_l );
extern  void    api_tl_disphexbyte( UCHAR row, UCHAR col, UCHAR data );
extern  void    api_tl_disphexword( UCHAR row, UCHAR col, UINT data );


// Add by Wayne 2020/08/21
/*
USB function prototype
*/
extern	UCHAR	api_usb_close( UCHAR dhn );
extern	UCHAR	api_usb_rxstring( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR	api_usb_txready( UCHAR dhn );
extern	UCHAR	api_usb_txstring( UCHAR dhn, UCHAR *sbuf );

/*
** API: TLS
*/
extern  UCHAR   api_tls_cpkey(void);
extern  UCHAR   api_tls_rmkey(void);




//----------------------------------------------------------------------------
#endif
