
//Channel
#define Mifare_Channel_Human_Interface						0x01U
#define Mifare_Channel_Contact_Card_Interface				0x02U
#define Mifare_Channel_Cless_Card_Interface					0x03U
#define Mifare_Channel_Crypto_Component						0x04U
#define Mifare_Channel_RS232				 				0x05U
#define Mifare_Channel_System								0x06U
#define Mifare_Channel_Advance_Command						0x07U
#define Mifare_Channel_Custom_Command						0x08U

#define	Mifare_Channel_TSC									0x0AU
#define	Mifare_Channel_PED									0x0BU
#define	Mifare_Channel_LCDTFT								0x0CU

#define	Mifare_Channel_MSR									0x0DU
#define	Mifare_Channel_KBD									0x0EU
#define	Mifare_Channel_BCR									0x0FU

#define	Mifare_Channel_NCCC									0x10U

#define	Mifare_Channel_ICC									0x20U
#define	Mifare_Channel_CL									0x21U

#define	Mifare_Channel_ESC									0x40U

//Human Interface					0x01
#define	Mifare_HI_Clear_Screen								0x01U
#define	Mifare_HI_Clear_Line								0x02U
#define	Mifare_HI_Write_Line								0x03U
#define	Mifare_HI_Turn_On_Off_LCD_Backlight					0x04U
#define	Mifare_HI_Clear_Line_Alphanumeric					0x05U
#define	Mifare_HI_Write_Line_Alphanumeric					0x06U
#define	Mifare_HI_Beep										0x10U
#define	Mifare_HI_Set_LED									0x20U
#define	Mifare_HI_Blink_LED									0x21U
#define	Mifare_HI_Set_BG_Palette							0x30U
#define	Mifare_HI_Show_ContactlessSymbol					0xFFU

//Contact Card Interface			0x02
#define	Mifare_CI_Get_ATR									0x01U
#define	Mifare_CI_PTS										0x02U
#define	Mifare_CI_Transmit_APDU								0x03U

//Cless Card Interface				0x03
#define Mifare_Cless_RF_Off									0x10U
#define Mifare_Cless_RF_On									0x11U
#define Mifare_Cless_REQ_A									0x20U
#define Mifare_Cless_WUP_A									0x21U
#define Mifare_Cless_ANTICOL				 				0x22U
#define Mifare_Cless_Select									0x23U
#define Mifare_Cless_Polling_Card							0x2AU
#define Mifare_Cless_Get_Card_Data							0x2FU
#define Mifare_Cless_Read									0x33U
#define Mifare_Cless_Write									0x34U
#define Mifare_Cless_Value									0x35U
#define Mifare_Cless_Restore								0x36U
#define Mifare_Cless_Transfer								0x37U
#define Mifare_Cless_3PASS_Auth								0x3AU
#define Mifare_Cless_Type_A_Transparent						0x41U
#define Mifare_Cless_Type_B_Transparent						0x51U
#define Mifare_Cless_Activate_Card							0x72U
#define Mifare_Cless_Transmit_APDU							0x73U
#define Mifare_Cless_Get_Status								0x74U
#define Mifare_Cless_Card_Removal							0x76U
#define Mifare_Cless_WUPB									0x80U

//Crypto Component				0x04

//RS232							0x05
#define Mifare_RS232_ECHO_TEST								0x01U
#define Mifare_RS232_Change_Baudrate						0x02U

//System						0x06
#define Mifare_SYS_Get_ContactlessChipSerialNumber			0x99U
#define Mifare_SYS_Get_Sys_Command							0xE8U
#define Mifare_SYS_Inject_Encry_Key							0xE9U
#define Mifare_SYS_Enable_Manual_Resource_Control			0xC0U
#define Mifare_SYS_Replace_Default_LOGO						0xC1U
#define Mifare_SYS_Disable_Screean_Saver					0xC2U

#define Mifare_SYS_Get_Firm_Info							0xE7U
#define Mifare_SYS_Get_Datetime								0xC6U
#define Mifare_SYS_Set_Datetime								0xC7U

//Advance Command				0x07
#define Mifare_ADV_Multiple_Command							0xD0U

//Custom Command				0x08
#define Mifare_Custom_File_Info								0x01U
#define Mifare_Custom_Rcv_File								0x02U
#define	Mifare_Custom_Get_CAPK_Index						0x03U
#define	Mifare_Custom_Del_ALL_CAPK_Key						0x04U

//TSC							0x0A
#define	Mifare_TSC_OPEN										0x01U
#define	Mifare_TSC_CLOSE									0x02U
#define	Mifare_TSC_STATUS									0x03U
#define	Mifare_TSC_SIGNPAD									0x04U
#define	Mifare_TSC_GETSIGN									0x05U

//PED							0x0B
#define	Mifare_PED_ENABLE									0x01U
#define	Mifare_PED_DISABLE									0x02U

//LCDTFT						0x0C
#define	Mifare_LCDTFT_CLEAR									0x01U
#define	Mifare_LCDTFT_PUTS									0x02U
#define	Mifare_LCDTFT_PUTG_PARA								0x03U
#define	Mifare_LCDTFT_PUTG_DATA								0x13U
#define	Mifare_LCDTFT_PUTWB_PARA							0x04U
#define	Mifare_LCDTFT_PUTWB_DATA							0x14U
#define	Mifare_LCDTFT_SHOWICON								0x05U
#define	Mifare_LCDTFT_FILLRECT								0x06U
#define	Mifare_LCDTFT_SIGNPAD_PUTS							0x07U
#define	Mifare_LCDTFT_STATUSICON							0x08U

//MSR							0x0D
#define	Mifare_MSR_OPEN										0x01U
#define	Mifare_MSR_CLOSE									0x02U
#define	Mifare_MSR_STATUS									0x03U
#define	Mifare_MSR_READ										0x04U

//KBD							0x0E
#define	Mifare_KBD_OPEN										0x01U
#define	Mifare_KBD_CLOSE									0x02U
#define	Mifare_KBD_STATUS									0x03U
#define	Mifare_KBD_GET										0x04U
#define	Mifare_KBD_GETC										0x05U
#define	Mifare_KBD_GETS										0x06U

//BCR							0x0F
#define	Mifare_BCR_OPEN										0x01U
#define	Mifare_BCR_CLOSE									0x02U
#define	Mifare_BCR_READ										0x03U

//NCCC Parameter				0x10
#define	Mifare_NCCC_PARAMETER_INFORMATION					0x00U
#define	Mifare_NCCC_SHOW_ERROR								0x01U
#define	Mifare_NCCC_RESET									0x02U

// EMV CT L2
#define	Mifare_EMVK_INIT_KERNEL								0x01
#define	Mifare_EMVK_OPEN_SESSION							0x02
#define	Mifare_EMVK_CLOSE_SESSION							0x03
#define	Mifare_EMVK_DETECT_ICC								0x04
#define	Mifare_EMVK_ENABLE_ICC								0x05
#define	Mifare_EMVK_DISABLE_ICC								0x06
#define	Mifare_EMVK_CREATE_CANDIDATE_LIST						0x07
#define	Mifare_EMVK_SELECT_APP								0x08
#define	Mifare_EMVK_EXEC_1								0x09
#define	Mifare_EMVK_CVM									0x0A
#define	Mifare_EMVK_EXEC_2								0x0B
#define	Mifare_EMVK_COMPLETION								0x0C
#define	Mifare_EMVK_SET_PARA								0x0D
#define	Mifare_EMVK_GET_DATA_ELEMENT							0x0E

//Mifare Response
#define	Mifare_RC_Success									0x00U
#define	Mifare_RC_Fail										0xFFU
#define	Mifare_RC_Invalid_Input								0x01U
#define	Mifare_RC_Invalid_Channel							0x02U
#define	Mifare_RC_Invalid_Command							0x03U
#define	Mifare_RC_Invalid_Data_Field						0x04U
#define	Mifare_RC_Invalid_SAM_Slot							0x05U
#define	Mifare_RC_No_Response								0x12U

#define	Mifare_RC_Ready									0x20
#define	Mifare_RC_Not_Ready								0x21
#define	Mifare_RC_CVM_Required								0x24
#define	Mifare_RC_Auto_Selected								0x25

#ifdef _SCREEN_SIZE_128x64
#define Mifare_LCD_MAX_LINES_FONT0							8
#define Mifare_LCD_MAX_LINES_FONT1							4
#define Mifare_LCD_MAX_WORDS_FONT0							21
#define Mifare_LCD_MAX_WORDS_FONT1							16
#elif defined _SCREEN_SIZE_240x320
#define Mifare_LCD_MAX_LINES_FONT0							20
#define Mifare_LCD_MAX_LINES_FONT1							13
#define Mifare_LCD_MAX_WORDS_FONT0							30
#define Mifare_LCD_MAX_WORDS_FONT1							20
#elif defined _SCREEN_SIZE_320x240
#define Mifare_LCD_MAX_LINES_FONT0							15
#define Mifare_LCD_MAX_LINES_FONT1							10
#define Mifare_LCD_MAX_WORDS_FONT0							40
#define Mifare_LCD_MAX_WORDS_FONT1							26
#endif


#define MIFARE_2DR_BUFFER									(2+2048)


//MIfare Reader Info, Firmware Info
extern       UCHAR	Mifare_Rdr_Serial_Num[8];
extern const UCHAR	Mifare_Rdr_HW_ID[8];
extern const UCHAR	Mifare_Rdr_HW_Ver[4];
extern const UCHAR	Mifare_Rdr_Firm_ID[8];
extern const UCHAR	Mifare_Rdr_Firm_Ver[4];

extern const UCHAR	Mifare_Firm_Name[20];
extern const UCHAR	Mifare_Firm_Ver[20];
extern const UCHAR	Mifare_Firm_Date[20];
extern const UCHAR	Mifare_Firm_PP_Ver[20];
extern const UCHAR	Mifare_IPASS_Ver[20];
extern const UCHAR	Mifare_Firm_VISA_Ver[20];
extern const UCHAR	Mifare_Firm_MAS_Ver[20]; 	
extern const UCHAR	Mifare_Firm_Mifare_Ver[20];
//MIfare Reader Info, Firmware Info end

extern ULONG	Mifare_Seq_Num;
extern ULONG	Mifare_Mem_Offset;
extern ULONG 	Mifare_Total_File_Size;

extern UCHAR	Mifare_File_Download_Flag;
extern ULONG	Mifare_File_Timer1;
extern ULONG	Mifare_File_Timer2;




