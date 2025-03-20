//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :                                                            **
//**  PRODUCT  : AS350	                                                    **
//**                                                                        **
//**  FILE     : LTEAPI.H						    **
//**  MODULE   : Declaration of 4G LTE APIs.		                    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2019/11/21                                                 ** 
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2019 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _LTEAPI_H_
#define _LTEAPI_H_
//----------------------------------------------------------------------------
#define	lteIdle					0x00
#define	lteDisconnected				0x00
#define	lteConnecting				0x01
#define	lteConnected				0x02

//----------------------------------------------------------------------------
//	Radio Access Technology
//----------------------------------------------------------------------------
#define	RAT_GSM					0x30	// "2G"	(2G)
#define	RAT_UTRAN				0x32	// "3G"	(3G)
#define	RAT_GSM_EGRPS				0x33	// "E"	(2.5G)
#define	RAT_UTRAN_HSDPA				0x34	// "H"	(3.5G)
#define	RAT_UTRAN_HSDPA_HSUPA			0x36	// "H+"	(3.75G)
#define	RAT_E_UTRAN				0x37	// "4G"	(3.9/4G)

//----------------------------------------------------------------------------
//	Profile Defintion
//----------------------------------------------------------------------------
#define	LTE_MAX_CON_PROFILE_CNT			6
#define	LTE_MAX_SRV_PROFILE_CNT			10
#define	LTE_MAX_PIN_LEN				12
#define	LTE_MAX_ADDR_LEN			32
#define	LTE_MAX_APN_LEN				100
#define	LTE_MAX_USER_LEN			32
#define	LTE_MAX_DIR_LEN				64

//----------------------------------------------------------------------------
//	Service Type
//----------------------------------------------------------------------------
#define	SVT_SOCKET				0x00
#define	SVT_FTP					0x01
#define	SVT_FTPS				0x81

//----------------------------------------------------------------------------
//	FTP Status
//----------------------------------------------------------------------------
#define	FTP_IN_PROCESSING			0x80
#define	FTP_FINISHED				0x00
#define	FTP_ERROR				0x01

//----------------------------------------------------------------------------
#define LTE_MAX_BUFFER_SIZE	65536
typedef	struct	LTE_CONNECTION_PROFILE_S
{
	UCHAR	ConProfileID;			// internet connection profile id ( 0x30..0x35=ID, 0xFF=end of profile)
	UCHAR	APN_len;			// length of APN (Access Point Name)
	UCHAR	APN[LTE_MAX_APN_LEN];		// APN string
	
	UCHAR	UserName_len;			// length of UserName, set to 0 if not used
	UCHAR	UserName[LTE_MAX_USER_LEN];	// user name string
	
	UCHAR	PassWord_len;			// length of PassWord, set to 0 if not used
	UCHAR	PassWord[LTE_MAX_USER_LEN];	// password string	
} LTE_CONNECTION_PROFILE;


// typedef	struct	LTE_SERVICE_PROFILE_S
// {	
// 	UCHAR	SrvProfileID;			// service profile id (0x30..0x39=ID, 0xFF=end of profile)
// 	UCHAR	ConProfileID;			// target "ConProfileID" to be linked
// 	UCHAR	Address_len;			// length of URL address string
// 	UCHAR	Address[LTE_MAX_ADDR_LEN];	// URL address string, format: "NNN.NNN.NNN.NNN:PortNo"
	
// 						// NEW
// 	UCHAR	Platform;			// 0=default (ME), 1=user defined (JAVA), 0x80=default SSL
// } LTE_SERVICE_PROFILE;
typedef	struct	LTE_SERVICE_PROFILE_S
{	
	UCHAR	SrvProfileID;			// service profile id (0x30..0x39=ID, 0xFF=end of profile)
	UCHAR	ConProfileID;			// target "ConProfileID" to be linked
	UCHAR	LenType;	// 0 = HEX, 1 = BCD
	UINT  	ClientPort;	// client port number
						// if C_port = 65100, the C_port will be auto increased by 1 for next connection
	UINT	ServerPort;	// server port number, such as 8000
	UINT	ServerIP_Len;	// length of server ip string
	UCHAR	ServerIP[16];	// server ip string, such as "172.16.1.100"
} LTE_SERVICE_PROFILE;

typedef	struct	LTE_SERVICE_PROFILE_FTP_S	// for FTP(S) connection
{	
	UCHAR	SrvProfileID;			// service profile id (0x30..0x39=ID, 0xFF=end of profile)
	UCHAR	ConProfileID;			// target "ConProfileID" to be linked
	UCHAR	Address_len;			// length of URL address string
	UCHAR	Address[LTE_MAX_ADDR_LEN];	// URL address string, format: "NNN.NNN.NNN.NNN:PortNo"

						// === The following elements are effective in using FTP(S) download! ===
	UCHAR	SrvType;			// service type; 0x00=socket(default), 0x01=ftp, 0x81=ftps
	
	UCHAR	UserName_len;			// length of UserName, set to 0 if not used
	UCHAR	UserName[LTE_MAX_USER_LEN];	// user name string
	
	UCHAR	PassWord_len;			// length of PassWord, set to 0 if not used
	UCHAR	PassWord[LTE_MAX_USER_LEN];	// password string
	
	UCHAR	FileName_len;			// length of FileName, empty length means to list the directory on the FTP server
	UCHAR	FileName[LTE_MAX_DIR_LEN];	// the specified file name to be dowloaded or the directory contents on the FTP server

						// NEW
	UCHAR	FilePath_len;			// length of FilePath
	UCHAR	FilePath[LTE_MAX_DIR_LEN];	// the specifiled file path, eg. "/ftp_edc/out/blc"
} LTE_SERVICE_PROFILE_FTP;


typedef	struct API_LTE_PROFILE_S
{	
	UCHAR	PIN_len;			// length of PIN (Personal Identification Number), 0 if no PIN needed
	UCHAR	PIN[LTE_MAX_PIN_LEN];		// PIN string
	
	LTE_CONNECTION_PROFILE	ConProfile[LTE_MAX_CON_PROFILE_CNT];
	
	LTE_SERVICE_PROFILE	SrvProfile[LTE_MAX_SRV_PROFILE_CNT];
} API_LTE_PROFILE;


typedef	struct API_LTE_PROFILE_FTP_S		// for FTP(S) connection
{	
	UCHAR	PIN_len;			// length of PIN (Personal Identification Number), 0 if no PIN needed
	UCHAR	PIN[LTE_MAX_PIN_LEN];		// PIN string
	
	LTE_CONNECTION_PROFILE		ConProfile[LTE_MAX_CON_PROFILE_CNT];
	
	LTE_SERVICE_PROFILE_FTP		SrvProfile[LTE_MAX_SRV_PROFILE_CNT];
	
						// NEW
	UCHAR	PROT_P;				// 0=no data encryption, 1=enforce data encryption
} API_LTE_PROFILE_FTP;


typedef	struct	API_LTE_S
{
	UCHAR	LenType;			// length format used in data transmission, 0 = HEX, 1 = BCD, 0xFF = Bypass
	
	UCHAR	SrvProfileID;			// service profile id (0x30..0x39) used to open internet service
	UINT	ConnectTimeout;			// max. waiting time in seconds used to link with host
} API_LTE;


//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	UCHAR	api_lte_powerON( void );
extern	UCHAR	api_lte_powerOFF( void );
extern	UCHAR	api_lte_checkPIN( void );
extern	UCHAR	api_lte_checkPINC( UCHAR *count );
extern	UCHAR	api_lte_checkOP( UCHAR *opname, UCHAR *rat );
extern	UCHAR	api_lte_checkSIGNAL( UCHAR *quality );
// extern	UCHAR	api_lte_selectSIM( UCHAR slot );
// extern	UCHAR	api_lte_whichSIM( void );
extern	UCHAR	api_lte_setup( API_LTE_PROFILE profile );

extern	UCHAR	api_lte_open( API_LTE lte );
extern	UCHAR	api_lte_close( UCHAR dhn );
extern	UCHAR	api_lte_status( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR	api_lte_txready( UCHAR dhn );
extern	UCHAR	api_lte_txstring( UCHAR dhn, UCHAR *sbuf );
extern	UCHAR	api_lte_rxready( UCHAR dhn, UCHAR *dbuf );
extern	UCHAR	api_lte_rxstring( UCHAR dhn, UCHAR *dbuf );

// extern	UCHAR	api_lte_ftp_setup( API_LTE_PROFILE_FTP profile );
// extern	UCHAR	api_lte_ftp_setup_SP( LTE_SERVICE_PROFILE_FTP profile );
// extern	UCHAR	api_lte_ftp_open( API_LTE lte );
// extern	UCHAR	api_lte_ftp_close( UCHAR dhn );
// extern	UCHAR	*api_lte_ftp_get( UCHAR dhn );
// extern	UCHAR	api_lte_ftp_get_status( UCHAR dhn, UCHAR *Status, ULONG *FileSize );
// extern	UCHAR	api_lte_ftp_cd( UCHAR dhn, UCHAR *cd );
// extern	UCHAR	api_lte_ftp_dir( UCHAR dhn, ULONG BufSize, UCHAR *Buf, ULONG *DirSize );
// extern	UCHAR	api_lte_ftp_size( UCHAR dhn, UCHAR *Path, UCHAR *FileName, ULONG *FileSize );

//----------------------------------------------------------------------------
#endif
