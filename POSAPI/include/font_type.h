#ifndef FONT_TYPE_H_
	#define FONT_TYPE_H_
	


//====================================class & structure=============================================
	enum FONT_TYPE
	{
		USELESS_FONT = 0,	//���ܦ�font�|���Q�ϥ�
		ASCII=1,	//8bit
		BIG5,		//16bit
		GB,			//16bit
		UNICODE,	//16bit
		ISO88591	//8bit
	};
//==================================================================================================
	struct FONT
	{
		unsigned char width;	//bitmap���e�סA�e�פ��p��8BIT
//		unsigned char height;	//bitmap�����סA���פ��j��32bit
		unsigned char code_size;//range = 1~4 ,�s�X�榡���A�C�Ӧr�����j�p�A��쬰byte�Fbig5��2�Aascii��1
		unsigned short member_size;//�C��bitmap�Ҧ��Ϊ��Ŷ�=width/8*height,when width%8==0; (width/8+1)*height ,when width%8!=0
		unsigned long member;//�r���ɩҥ]�t���r����
		enum FONT_TYPE type;
		const unsigned char *bitmap;
		const unsigned char *code_list;

	};

	struct FONT_PAIR
	{
		// enum SEMAPHORE_B idel;
		unsigned char height;	//bitmap�����סA���פ��j��255
		struct FONT SBC;//single Byte code
		struct FONT MBC;//multiple byte code
	};
const unsigned char* Font_getBitMap(unsigned short code,unsigned char fontID);
void font_init();
#endif /*FONT_TYPE_H_*/
