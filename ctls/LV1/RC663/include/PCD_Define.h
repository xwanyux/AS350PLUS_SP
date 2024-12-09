//	PCD Data Buffer Size
#define PCD_BUFFER_SIZE_SEND		64
#define PCD_BUFFER_SIZE_READER		512
#define PCD_BUFFER_SIZE_RECEIVE		2048
#define PCD_BUFFER_SIZE_UID			10
#define PCD_BUFFER_SIZE_PUPI		4

//	UID Size
#define PCD_UID_SINGLE				4		//Single UID
#define PCD_UID_DOUBLE				7		//Double UID
#define PCD_UID_TRIPLE				10		//Triple UID

//	Anticollision Cascade Level
#define PCD_CASCADE_TAG				0x88U	//Cascade Tag
#define PCD_CASCADE_LEVEL1			0x93U	//Cascade Level 1
#define PCD_CASCADE_LEVEL2			0x95U	//Cascade Level 2
#define PCD_CASCADE_LEVEL3			0x97U	//Cascade Level 3

//	Block Type
#define PCD_BLOCK_TYPE_I			'I'
#define PCD_BLOCK_TYPE_R			'R'
#define PCD_BLOCK_TYPE_S			'S'

//	Block Format
#define PCD_BLOCK_I_BLOCK			0x00U	//I-Block
#define PCD_BLOCK_R_BLOCK			0x80U	//R-Block
#define PCD_BLOCK_S_BLOCK			0xC0U	//S-Block
#define PCD_BLOCK_I_NORMAL			0x02U	//I-Block Normal
#define PCD_BLOCK_I_CHAINING		0x12U	//I-Block Chaining
#define PCD_BLOCK_R_ACK				0xA2U	//R-Block ACK
#define PCD_BLOCK_R_NAK				0xB2U	//R-Block NAK
#define PCD_BLOCK_S_WTX				0xF2U	//S-Block WTX
#define PCD_BLOCK_S_DESELECT		0xC2U	//S-Block DESELECT

//	Chaining State
#define PCD_CHAINING_None			0xC0U	//None Chaining
#define PCD_CHAINING_PCD			0xC1U	//PCD Chaining
#define PCD_CHAINING_PICC			0xC2U	//PICC Chaining

//	DEP Error Code
#define PCD_ERROR_BLOCKSIZE			0xE0U	//Block Size Error 
#define PCD_ERROR_PCBFORMAT			0xE1U	//PCB Format Error 
#define PCD_ERROR_BLOCKNUMBER		0xE2U	//Block Number Error 
#define PCD_ERROR_RECEIVEPOLICY		0xE3U	//Receive Policy Error
#define PCD_ERROR_PROTOCOL			0xE4U	//Protocol Error
#define PCD_ERROR_OVERFLOW			0xE5U	//Buffer Overflow

