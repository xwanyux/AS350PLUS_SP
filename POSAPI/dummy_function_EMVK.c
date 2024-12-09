

#include "bsp_types.h"
#include "POSAPI.h"
#include "LCDTFTAPI.h"
#include <stdlib.h>


// also modify EMVCB.c (in EMVK) and EmvCb (in APP)

/**
 *   for EMVK compile
 */
// UCHAR api_sram_write(UCHAR* address, ULONG length, UCHAR* data ){

//     return apiOK;
// }

// UCHAR api_sram_read(UCHAR* address, ULONG length, UCHAR* data ){

//     return apiOK;
// }


// UCHAR api_sram_clear(UCHAR* address, ULONG length, UCHAR pattern ){
//     return apiOK;
// }


// void BSP_Delay_n_ms(UCHAR ms){

//     return;

// }

// UCHAR api_ped_GetKeyHeader_CAPK(UCHAR idx, UCHAR* data){
//     return apiOK;
// }


#ifdef _build_DSS_
void EMV_CB_ShowMsg_ENTER_PIN( UCHAR *amt ){return;}
void EMV_CB_ShowMsg_ENTER_PIN_BY_EXT_PINPAD( void ){return;}
void EMV_CB_ShowMsg_PIN_OK( void ){return;}
void EMV_CB_ShowMsg_INCORRECT_PIN(){return;}
void EMV_CB_ShowMsg_LAST_PIN_TRY(){return;}
void EMV_CB_ShowMsg_PLEASE_WAIT(){}
void EMV_CB_SetupPinPad( API_LCDTFT_PARA *para ){return;}
UCHAR EMV_CB_OnlineProcessing( UCHAR *arc, UCHAR *air, UCHAR *rmsg ){return apiOK;}
UCHAR EMV_CB_ReferralProcessing(void){return apiOK;}
UCHAR EMV_CB_FindMostRecentPAN( UCHAR *pan, UCHAR *amt ){return apiOK;}
#endif
//void api_xped_show_MsgID(UCHAR unknown) {return;}

//void api_ped_SetupPinPad( API_LCDTFT_PARA para ){return;}

//UCHAR api_xped_GetPin( UINT tout, UCHAR* amt ){return apiOK;}

//UCHAR api_xped_GenEncryptedPinBlock(UCHAR a, UCHAR b, UCHAR c, UCHAR* d, UCHAR* e, UCHAR* f) 
//{return apiOK;}

//UCHAR api_xped_MSKEY_GenPinBlock(UCHAR a, UCHAR b, UCHAR* c, UCHAR* d){return apiOK;}
//UCHAR api_ped_GenPinBlock_FXKEY(UCHAR a, UCHAR b, UCHAR* c, UCHAR* d){return apiOK;}
//UCHAR api_ped_GenPinBlock_MSKEY(UCHAR a, UCHAR b, UCHAR* c, UCHAR* d){return apiOK;}
//UCHAR api_ped_GenPinBlock_DUKPT(UCHAR a, UCHAR b, UCHAR* c, UCHAR* d){return apiOK;}


//UCHAR PED_PutWaveIEK( UCHAR *imek, UCHAR *iaek ){return apiOK;}
//UCHAR PED_GetWaveIMEK( UCHAR *imek ){return apiOK;}
//UCHAR PED_GetWaveIAEK( UCHAR *iaek ){return apiOK;}

//UCHAR api_ped_SetPinPadPort( UCHAR port ){return apiOK;}


//void PED_GenPinBlock( UCHAR* pinblock ){return;}

/**
 *  end for EMVK compile
 */ 


// compile for OS_FLASH
//UINT32	FLASH_EraseSector( UINT32 SectorBase ) {return apiOK;}
// UINT32	FLASH_WriteData( void *BaseAddr, void *pData, UINT32 Len ){return apiOK;}
// void	FLASH_ReadData( void *BaseAddr, void *pData, UINT32 Len ){return;}

// UINT32	OS_FLS_PutData( UINT32 addr, UINT32 len, UINT8 *data ){return apiOK;}
// void	OS_FLS_GetData( UINT32 addr, UINT32 len, UINT8 *data ){return;}



/**
 *  compile for API_PEDS.c
 */


//void PED_TripleDES(UCHAR* a, UINT16 b, UCHAR* c, UCHAR* d){return;}

//void PED_TripleDES2(UCHAR* a, UINT16 b, UCHAR* c, UCHAR* d){return;}

BSP_HANDLE BSP_Malloc_EX( UINT32 Size ){return malloc(Size);}

void BSP_Free_EX( BSP_HANDLE pData ){free(pData);}

//void PED_GenPinBlock_ISO0(UCHAR* a, UCHAR* b, UCHAR*c) {return;}

//void PED_GenMAC_BASE24(UCHAR* a, UCHAR *b, UINT c, UCHAR *d, UCHAR *e){return;}


//void PED_CBC_TripleDES2(UCHAR *a, UINT8 *icv, UINT16 length, UINT8 *idata, UINT8 *odata){return;}

//void PED_CBC_TripleDES(UCHAR *a, UINT8 *icv, UINT16 length, UINT8 *idata, UINT8 *odata){return;}


/**
 *  compiler for ped api
 */

// function api_ped_GetPin
//UINT8	PED_GetPin( UINT16 tout, UINT8 *amt ){return apiOK;}
// function api_ped_GetKeyHeader_CAPK
//UINT8	PED_CAPK_GetKeyHeader( UINT8 index, UINT8 *pkh ){return apiOK;}
// function api_ped_SelectKey_CAPK
//UINT8	PED_CAPK_SelectKey( UINT8 pki, UINT8 *rid, UINT8 *pkh, UINT8 *index ){return apiOK;}
// function api_ped_GenPinBlock_MSKEY

// function api_ped_SetupPinPad
//API_LCDTFT_PARA	os_ped_EMVSignPadPara;

// should ommit
//UINT8 PED_MSKEY_GenPinBlock( UINT8 mode, UINT8 index, UINT8 *pan, UINT8 *epb ){return apiOK;}
//UINT8 PED_FXKEY_GenPinBlock( UINT8 mode, UINT8 index, UINT8 *pan, UINT8 *epb ){return apiOK;}
//void PED_InUse(UCHAR bool){return;}

//UCHAR DUKPT_RequestPinEntry(  UCHAR *pindata, UCHAR *pan, UCHAR *ksn, UCHAR *epb ){return apiOK;}

//functionPED_GetWaveIMEK, and PED_GetWaveIAEK
//int LIB_memcmpc(const void *str1, const void *str2, size_t n){return memcmp(str1,str2, n);}

// not sure how to fix its
UINT8	post_dhn_kbd;
UINT8	post_dhn_aux;
//UINT8	os_LCD_DID;


//UINT8	PED_GenPinBlock( UINT8 *pb ){return apiOK;}
