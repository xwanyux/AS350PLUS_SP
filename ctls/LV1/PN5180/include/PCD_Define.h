//	PCD Platform
//#define PCD_PLATFORM_CLRC663			1
#define PCD_PLATFORM_PN5180				1

//	PCD Self Define Parameters
#define PCD_PARAMETER_FSD				256			//Frame Size Device (16~4096)
#define PCD_PARAMETER_FSDI				8			//Frame Size Device Index (FSDI:8 = FSD:256)
#define PCD_PARAMETER_FSC				256			//Frame Size Card that PCD is Supported (256~4096)

//	PCD Default Parameters
#define PCD_PARAMETER_PCD2PICC			1			//Bit Rate Divisor PCD to PICC
#define PCD_PARAMETER_PICC2PCD			1			//Bit Rate Divisor PICC to PCD
#define PCD_PARAMETER_FWI_MAX			14			//Max. Frame Waiting Time Integer
#define PCD_PARAMETER_FWT_MAX			67108864	//Max. Frame Waiting Time
#define PCD_PARAMETER_FWT_DELTA			49152		//Frame Waiting Time Delta
#define PCD_PARAMETER_FWT_ACTIVATION	71680		//Frame Waiting Time Activation
#define PCD_PARAMETER_FWT_ATQB			7680		//Frame Waiting Time ATQB
#define PCD_PARAMETER_FWT_DESELECT		65536		//Frame Waiting Time Deselect
#define PCD_PARAMETER_FDT_A_MIN_LB0		1172		//Min. Frame Delay Time (Last Bit = 0)
#define PCD_PARAMETER_FDT_A_MIN_LB1		1236		//Min. Frame Delay Time (Last Bit = 1)
#define PCD_PARAMETER_SFGI_MAX			14			//Max. Start-up Frame Guard Time
#define PCD_PARAMETER_T_DELTA			222384		//Tolerance Delta (~16.4 ms)

//	PCD Data Buffer Size
#define PCD_BUFFER_SIZE_SEND			PCD_PARAMETER_FSC
#define PCD_BUFFER_SIZE_RECEIVE			1024
#define PCD_BUFFER_SIZE_TEMPORARY		PCD_PARAMETER_FSD
#define PCD_BUFFER_SIZE_UID				10
#define PCD_BUFFER_SIZE_PUPI			4

//	UID Size
#define PCD_UID_SINGLE					4		//Single UID
#define PCD_UID_DOUBLE					7		//Double UID
#define PCD_UID_TRIPLE					10		//Triple UID

//	Anticollision Cascade Level
#define PCD_CASCADE_TAG					0x88U	//Cascade Tag
#define PCD_CASCADE_LEVEL1				0x93U	//Cascade Level 1
#define PCD_CASCADE_LEVEL2				0x95U	//Cascade Level 2
#define PCD_CASCADE_LEVEL3				0x97U	//Cascade Level 3

//	Block Type
#define PCD_BLOCK_TYPE_I				'I'
#define PCD_BLOCK_TYPE_R				'R'
#define PCD_BLOCK_TYPE_S				'S'

//	Block Format
#define PCD_BLOCK_I_BLOCK				0x00U	//I-Block
#define PCD_BLOCK_R_BLOCK				0x80U	//R-Block
#define PCD_BLOCK_S_BLOCK				0xC0U	//S-Block
#define PCD_BLOCK_I_NORMAL				0x02U	//I-Block Normal
#define PCD_BLOCK_I_CHAINING			0x12U	//I-Block Chaining
#define PCD_BLOCK_R_ACK					0xA2U	//R-Block ACK
#define PCD_BLOCK_R_NAK					0xB2U	//R-Block NAK
#define PCD_BLOCK_S_WTX					0xF2U	//S-Block WTX
#define PCD_BLOCK_S_DESELECT			0xC2U	//S-Block DESELECT

//	Chaining State
#define PCD_CHAINING_NONE				0xC0U	//None Chaining
#define PCD_CHAINING_PCD				0xC1U	//PCD Chaining
#define PCD_CHAINING_PICC				0xC2U	//PICC Chaining

//	DEP Error Code
#define PCD_ERROR_BLOCKSIZE				0xE0U	//Block Size Error 
#define PCD_ERROR_PCBFORMAT				0xE1U	//PCB Format Error 
#define PCD_ERROR_BLOCKNUMBER			0xE2U	//Block Number Error 
#define PCD_ERROR_RECEIVEPOLICY			0xE3U	//Receive Policy Error
#define PCD_ERROR_PROTOCOL				0xE4U	//Protocol Error
#define PCD_ERROR_OVERFLOW				0xE5U	//Buffer Overflow

