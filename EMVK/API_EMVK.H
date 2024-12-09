//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : EMV L2 KERNEL APIs.                                        **
//**  PRODUCT  : AS320	                                                    **
//**                                                                        **
//**  FILE     : API_EMVK.H                                                 **
//**  MODULE   :                                                            **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2009/06/10                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2009 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#ifndef _API_EMVK_H_
#define _API_EMVK_H_

//----------------------------------------------------------------------------
#define	emvkOK				0x00000000		// function has completed
#define	emvkFailed			0x00000001		// function has failed
#define emvkAborted              	0x00000003		// function has aborted for some reason

#define	emvkCardPresent			0x01000000		// ICC in place
#define	emvkCardNotPresent		0x01000001		// ICC not in place

#define	emvkFailedCCL			0x02000001		// fail to create candidate list
#define	emvkNoMutualSupport		0x02000002		// no mutually supported applications (should go to FallBack)
#define	emvkNoCandidate			0x02000003		// no any application supported

#define	emvkMandatoryDataMissing_FS	0x03000001		// mandatory data objects missing - Final Selection
#define	emvkFinalSelectionFailed	0x03000002		// failure of final selection
#define	emvkSelectAppQuit		0x03000003		// timeout or aborted in application selection

#define	emvkGetAmountQuit		0x04000001		// quit getting amount

#define	emvkInitAppFailed		0x05000001		// fail to init application

#define	emvkReadAppDataFailed		0x06000001		// fail to read application data

#define	emvkOfflineDataAuthFailed	0x07000001		// failed in offline data authentication

#define	emvkCVMFailed			0x08000001		// failed in cardholder verification

#define	emvkTAAFailed			0x0B000001		// fail in terminal action analysis
#define	emvkVoiceReferralRequired	0x0B000002		// call bank for the declined reason
#define	emvkConditionNotSatisfied	0x0B000003		// fail to perform TAA

//----------------------------------------------------------------------------
//	Function Prototypes
//----------------------------------------------------------------------------
extern	ULONG	api_emvk_PutTmsPARA_EX( UCHAR *data );
extern	ULONG	api_emvk_PutTmsCAPK_EX( UCHAR *data );
extern	ULONG	api_emvk_GetDataElement_EX( UCHAR tag1, UCHAR tag2, UCHAR *data );
extern	ULONG	api_emvk_GetTmsDataElement_EX( UCHAR index, UCHAR tag1, UCHAR tag2, UCHAR *data );
extern	void	api_emvk_GetPublicKeyInfo_EX( UCHAR *info );

extern	ULONG	api_emvk_InitKernel( UCHAR *tid, UCHAR UserDefinedMsg );
extern	ULONG	api_emvk_OpenCardSession( void );
extern	ULONG	api_emvk_CloseCardSession( void );
extern	ULONG	api_emvk_DetectCardPosition( void );
extern	ULONG	api_emvk_ActivateCard( void );
extern	ULONG	api_emvk_DeactivateCard( void );
extern	ULONG	api_emvk_DoTransaction( UCHAR TransType, UCHAR ForcedOnline, UCHAR *arc );

//----------------------------------------------------------------------------
#endif


