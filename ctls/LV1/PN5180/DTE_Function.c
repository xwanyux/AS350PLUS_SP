/*
*	Device Test Environment for EMV CL Level 1
*/

#include <string.h>
#include "POSAPI.h"
#include "ECL_LV1_Define.h"
#include "EMV_Define.h"
#include "PCD_Define.h"
#include "DTE_Define.h"
#include "ECL_LV1_Util.h"
#include "USBAPI.h"


#ifdef PCD_PLATFORM_CLRC663
#include "NXP_Function.h"

extern UCHAR	nxp_TxAmp_A;
extern UCHAR	nxp_TxAmp_B;
extern UCHAR	nxp_RxAna_A;
extern UCHAR	nxp_RxAna_B;
#else
#include "HIC_Function.h"

extern UCHAR	pnq_parRX_GAIN_A;
extern UCHAR	pnq_parRX_HPCF_A;
extern UCHAR	pnq_parRX_GAIN_B;
extern UCHAR	pnq_parRX_HPCF_B;
extern UCHAR	pnq_parTX_RESIDUAL_CARRIER_A;
extern UCHAR	pnq_parTX_RESIDUAL_CARRIER_B;
#endif

extern UCHAR	emv_appFunction;	//Application Function
extern UCHAR	emv_appMode;		//Application Mode
extern UCHAR	emv_idxPCDFun;		//PCD Function Index
extern UCHAR	emv_typSndApp;		//Type of Transaction Send Application

//Test Load Modulation
extern UCHAR	emv_flgLodModulation;	//Load Modulation Test


extern void	UT_ClearRow(UCHAR row, UCHAR cnt, UCHAR font);
extern void	UT_PutStr(UCHAR row, UCHAR col, UCHAR font, UCHAR len, UCHAR *msg);


UCHAR	dte_flgTA=FALSE;
UCHAR	dte_modPeripheral=DTE_MODE_PERIPHERAL_AUX;

UCHAR	dte_dhnAUX=0;
UCHAR	dte_dhnUSB=0;

UINT	dte_sndLen=0;
UCHAR	dte_sndBuffer[DTE_SEND_Buffer_SIZE];

UINT	dte_rcvLen=0;
UCHAR	dte_rcvBuffer[DTE_RECEIVE_Buffer_SIZE];






UCHAR DTE_RxReady_Peripheral(void)
{
	UCHAR	rspCode=apiNotReady;

	dte_rcvLen=0;

	if (dte_modPeripheral == DTE_MODE_PERIPHERAL_AUX)
	{
		rspCode=api_aux_rxready(dte_dhnAUX, (UCHAR*)&dte_rcvLen);
	}
	else
	{
		rspCode=api_usb_rxready(dte_dhnUSB, (UCHAR*)&dte_rcvLen);
	}

	return rspCode;
}

UCHAR DTE_RxString_Peripheral(void)
{
	UCHAR	rspCode=apiNotReady;

	if (dte_modPeripheral == DTE_MODE_PERIPHERAL_AUX)
	{
		rspCode=api_aux_rxstring(dte_dhnAUX, dte_rcvBuffer);
	}
	else
	{
		rspCode=api_usb_rxstring(dte_dhnUSB, dte_rcvBuffer);
	}

	return rspCode;
}

UCHAR DTE_Tx_USB(void)
{
	if (api_usb_txready(dte_dhnUSB) == apiReady)
	{
		api_usb_txstring(dte_dhnUSB, dte_sndBuffer);

		while (api_usb_txready(dte_dhnUSB) != apiReady);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

UCHAR DTE_Tx_AUX(void)
{
	if (api_aux_txready(dte_dhnAUX) == apiReady)
	{
		api_aux_txstring(dte_dhnAUX, dte_sndBuffer);

		while (api_aux_txready(dte_dhnAUX) != apiReady);
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

UCHAR DTE_Tx_Peripheral(void)
{
	UCHAR	rspCode=FALSE;

	if (dte_modPeripheral == DTE_MODE_PERIPHERAL_AUX)
	{
		rspCode=DTE_Tx_AUX();
	}
	else
	{
		rspCode=DTE_Tx_USB();
	}

	return rspCode;
}

UCHAR DTE_Open_USB(void)
{
	API_USB_PARA	sbuf;

	if (dte_flgTA == TRUE)
	{
		memset( &sbuf, 0x00, sizeof(API_USB_PARA) );
		sbuf.Mode = auxSOH;
		sbuf.Baud = COM_115200 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
		sbuf.Acks = 0;
		sbuf.Tob = 10;
		sbuf.Tor = 50;

		dte_dhnUSB = api_usb_open( 0, sbuf );
		if (dte_dhnUSB != apiOutOfService)
		{
			dte_modPeripheral=1;
		}
	}

	return dte_dhnUSB;
}


UCHAR DTE_Open_AUX(void)
{
	API_AUX	sbuf;

	if (dte_flgTA == TRUE)
	{
		sbuf.Mode	=auxSOH;
		sbuf.Baud	=COM_115200+COM_CHR8+COM_NOPARITY+COM_STOP1;
		sbuf.Tob	=25;
		sbuf.Tor	=50;
		sbuf.Acks	=0;
		sbuf.Resend	=0;
		
		dte_dhnAUX=api_aux_open(COM0, sbuf);
		if (dte_dhnAUX != apiOutOfService)
		{
			dte_modPeripheral=0;
		}
	}

	return dte_dhnAUX;
}

UCHAR DTE_Open_Peripheral(UCHAR iptMode)
{
	UCHAR	rspCode=apiOutOfService;

	if (iptMode > DTE_MODE_PERIPHERAL_USB)
	{
		return apiFailed;
	}

	if (iptMode == DTE_MODE_PERIPHERAL_AUX)
	{
		rspCode=DTE_Open_AUX();
	}
	else
	{
		rspCode=DTE_Open_USB();
	}

	return rspCode;
}


UCHAR DTE_Read_Register(UCHAR iptAddress, UCHAR * optData)
{
#ifdef PCD_PLATFORM_CLRC663
	NXP_Read_Register(iptAddress, optData);
#else
	HIC_READ_REGISTER(iptAddress, optData);
#endif

	return TRUE;
}

UCHAR DTE_Read_Register_TX_A(UCHAR * optData)
{
#ifdef PCD_PLATFORM_CLRC663
	optData[0]=nxp_TxAmp_A;
#else
	optData[0]=pnq_parTX_RESIDUAL_CARRIER_A;
#endif

	return TRUE;
}

UCHAR DTE_Read_Register_TX_B(UCHAR * optData)
{
#ifdef PCD_PLATFORM_CLRC663
	optData[0]=nxp_TxAmp_B;
#else
	optData[0]=pnq_parTX_RESIDUAL_CARRIER_B;
#endif

	return TRUE;
}

UCHAR DTE_Read_Register_RX_A(UCHAR * optData)
{
#ifdef PCD_PLATFORM_CLRC663
	optData[0]=nxp_RxAna_A;
#else
	optData[0]=(pnq_parRX_GAIN_A | (pnq_parRX_HPCF_A << 2));
#endif

	return TRUE;
}

UCHAR DTE_Read_Register_RX_B(UCHAR * optData)
{
#ifdef PCD_PLATFORM_CLRC663
	optData[0]=nxp_RxAna_B;
#else
	optData[0]=(pnq_parRX_GAIN_B | (pnq_parRX_HPCF_B << 2));
#endif

	return TRUE;
}

UCHAR DTE_Write_Register(UCHAR iptAddress, UCHAR * iptData)
{	
#ifdef PCD_PLATFORM_CLRC663
	NXP_Write_Register(iptAddress, iptData[0]);
#else
	HIC_WRITE_REGISTER(iptAddress, iptData);
#endif

	return TRUE;
}

UCHAR DTE_Write_Register_TX_A(UCHAR * iptData)
{
#ifdef PCD_PLATFORM_CLRC663
	nxp_TxAmp_A=iptData[0];
#else
	pnq_parTX_RESIDUAL_CARRIER_A=iptData[0] & 0x1F;
#endif

	return TRUE;
}

UCHAR DTE_Write_Register_TX_B(UCHAR * iptData)
{
#ifdef PCD_PLATFORM_CLRC663
	nxp_TxAmp_B=iptData[0];
#else
	pnq_parTX_RESIDUAL_CARRIER_B=iptData[0] & 0x1F;
#endif

	return TRUE;
}

UCHAR DTE_Write_Register_RX_A(UCHAR * iptData)
{
#ifdef PCD_PLATFORM_CLRC663
	nxp_RxAna_A=iptData[0];
#else
	pnq_parRX_GAIN_A=(iptData[0] & 0x03);
	pnq_parRX_HPCF_A=(iptData[0] & 0x0C)>>2;
#endif

	return TRUE;
}

UCHAR DTE_Write_Register_RX_B(UCHAR * iptData)
{
#ifdef PCD_PLATFORM_CLRC663
	nxp_RxAna_B=iptData[0];
#else
	pnq_parRX_GAIN_B=(iptData[0] & 0x03);
	pnq_parRX_HPCF_B=(iptData[0] & 0x0C)>>2;
#endif

	return TRUE;
}

void DTE_Patch_ReadRegisterResponse(void)
{
#ifdef PCD_PLATFORM_CLRC663
	dte_sndBuffer[0]=2;
#else
	dte_sndBuffer[0]=5;
#endif

	dte_sndBuffer[1]=0;
	dte_sndBuffer[2]=DTE_READ_REGISTER;
}


UCHAR DTE_Get_AUXCommand(void)
{
	UCHAR	rspCode=0;


	if (dte_flgTA == TRUE)
	{
		rspCode=DTE_RxReady_Peripheral();
		if (rspCode == apiOK)
		{
			if (dte_rcvLen != 0)
			{
				rspCode=DTE_RxString_Peripheral();

				switch (dte_rcvBuffer[2])
				{
					case DTE_ANALOGUE_RESET:
						emv_appFunction=EMV_FUNCTION_PCD;
						emv_idxPCDFun=EMV_PCD_FUNCTION_RESET;
						
						UT_ClearRow(1, 1, FONT0);
						UT_PutStr(1, 0, FONT0, 15, (UCHAR*)"Analogue: Reset");
						
						return TRUE;

					case DTE_ANALOGUE_POLLING:
						emv_appFunction=EMV_FUNCTION_PCD;
						emv_idxPCDFun=EMV_PCD_FUNCTION_POLLING;
						
						UT_ClearRow(1, 1, FONT0);
						UT_PutStr(1, 0, FONT0, 17, (UCHAR*)"Analogue: Polling");
						
						return TRUE;

					case DTE_ANALOGUE_WUPA:
						emv_appFunction=EMV_FUNCTION_PCD;
						emv_idxPCDFun=EMV_PCD_FUNCTION_WUPA;
						
						UT_ClearRow(1, 1, FONT0);
						UT_PutStr(1, 0, FONT0, 14, (UCHAR*)"Analogue: WUPA");
						
						return TRUE;

					case DTE_ANALOGUE_RATS:
						emv_appFunction=EMV_FUNCTION_PCD;
						emv_idxPCDFun=EMV_PCD_FUNCTION_RATS;
						
						UT_ClearRow(1, 1, FONT0);
						UT_PutStr(1, 0, FONT0, 14, (UCHAR*)"Analogue: RATS");
						
						return TRUE;

					case DTE_ANALOGUE_WUPB:
						emv_appFunction=EMV_FUNCTION_PCD;
						emv_idxPCDFun=EMV_PCD_FUNCTION_WUPB;
						
						UT_ClearRow(1, 1, FONT0);
						UT_PutStr(1, 0, FONT0, 14, (UCHAR*)"Analogue: WUPB");
					
						return TRUE;

					case DTE_ANALOGUE_ATTRIB:
						emv_appFunction=EMV_FUNCTION_PCD;
						emv_idxPCDFun=EMV_PCD_FUNCTION_ATTRIB;
						
						UT_ClearRow(1, 1, FONT0);
						UT_PutStr(1, 0, FONT0, 16, (UCHAR*)"Analogue: ATTRIB");
						
						return TRUE;

					case DTE_ANALOGUE_CARRIER_ON:
						emv_appFunction=EMV_FUNCTION_PCD;
						emv_idxPCDFun=EMV_PCD_FUNCTION_CARRIERON;
						
						UT_ClearRow(1, 1, FONT0);
						UT_PutStr(1, 0, FONT0, 18, (UCHAR*)"Analogue: Field On");

						return TRUE;

					case DTE_ANALOGUE_CARRIER_OFF:
						emv_appFunction=EMV_FUNCTION_PCD;
						emv_idxPCDFun=EMV_PCD_FUNCTION_CARRIEROFF;
						
						UT_ClearRow(1, 1, FONT0);
						UT_PutStr(1, 0, FONT0, 19, (UCHAR*)"Analogue: Field Off");
						
						return TRUE;

					case DTE_POWER_OFF:
						emv_appFunction=EMV_FUNCTION_PCD;
						emv_idxPCDFun=EMV_PCD_FUNCTION_POWEROFF;
						
						UT_ClearRow(1, 1, FONT0);
						UT_PutStr(1, 0, FONT0, 9, (UCHAR*)"Power Off");
						
						return TRUE;
					
						
					case DTE_PREVALIDATION_START:
						emv_appFunction=EMV_FUNCTION_APPLICATION;
						emv_appMode=EMV_MODE_PREVALIDATION;
						emv_flgLodModulation=FALSE;

						UT_ClearRow(1, 3, FONT0);
						UT_PutStr(1, 0, FONT0, 19, (UCHAR*)"PreValidation Start");
												
						return TRUE;

					case DTE_PREVALIDATION_STOP:
						emv_appFunction=EMV_FUNCTION_NONE;
						emv_appMode=EMV_MODE_NONE;

						UT_ClearRow(1, 3, FONT0);
						UT_PutStr(1, 0, FONT0, 18, (UCHAR*)"PreValidation Stop");
						
						return TRUE;

					case DTE_LOOPBACK_START:
						emv_appFunction=EMV_FUNCTION_APPLICATION;
						emv_appMode=EMV_MODE_LOOPBACK;
						emv_flgLodModulation=FALSE;

						UT_ClearRow(1, 3, FONT0);
						UT_PutStr(1, 0, FONT0, 14, (UCHAR*)"LoopBack Start");
												
						return TRUE;

					case DTE_LOOPBACK_STOP:
						emv_appFunction=EMV_FUNCTION_NONE;
						emv_appMode=EMV_MODE_NONE;

						UT_ClearRow(1, 3, FONT0);
						UT_PutStr(1, 0, FONT0, 13, (UCHAR*)"LoopBack Stop");
						
						return TRUE;

					case DTE_SENDAPPLICATION_START:
						if (dte_rcvBuffer[3] < 2)
						{
							emv_appFunction=EMV_FUNCTION_TXNSENDAPP;
							emv_typSndApp=dte_rcvBuffer[3];

							UT_ClearRow(1, 3, FONT0);

							if (dte_rcvBuffer[3] == 0)
								UT_PutStr(1, 0, FONT0, 20, (UCHAR*)"Txn Send App A Start");
							else
								UT_PutStr(1, 0, FONT0, 20, (UCHAR*)"Txn Send App B Start");

							return TRUE;
						}
						else
						{
							break;
						}

					case DTE_SENDAPPLICATION_STOP:
						emv_appFunction=EMV_FUNCTION_NONE;

						UT_ClearRow(1, 3, FONT0);
						UT_PutStr(1, 0, FONT0, 17, (UCHAR*)"Txn Send App Stop");

						return TRUE;


					case DTE_WRITE_REGISTER:
						DTE_Write_Register(dte_rcvBuffer[3], &dte_rcvBuffer[4]);

						return TRUE;

					case DTE_WRITE_REGISTER_RxAna_A:
						DTE_Write_Register_RX_A(&dte_rcvBuffer[3]);

						return TRUE;

					case DTE_WRITE_REGISTER_RxAna_B:
						DTE_Write_Register_RX_B(&dte_rcvBuffer[3]);
						
						return TRUE;

					case DTE_WRITE_REGISTER_TxAmp_A:
						DTE_Write_Register_TX_A(&dte_rcvBuffer[3]);

						return TRUE;

					case DTE_WRITE_REGISTER_TxAmp_B:
						DTE_Write_Register_TX_B(&dte_rcvBuffer[3]);

						return TRUE;

					case DTE_WRITE_REGISTER_TxAmp_C:
//						nxp_TxAmp_C=dte_rcvBuffer[3];
//						NXP_Write_Register(0x29, nxp_TxAmp_C);

						return TRUE;

					case DTE_READ_REGISTER:
						DTE_Patch_ReadRegisterResponse();
						DTE_Read_Register(dte_rcvBuffer[3], &dte_sndBuffer[3]);

						rspCode=DTE_Tx_Peripheral();

						return TRUE;

					case DTE_READ_REGISTER_RxAna_A:
						dte_sndBuffer[0]=2;
						dte_sndBuffer[1]=0;
						dte_sndBuffer[2]=DTE_READ_REGISTER_RxAna_A;
						DTE_Read_Register_RX_A(&dte_sndBuffer[3]);

						rspCode=DTE_Tx_Peripheral();

						return TRUE;

					case DTE_READ_REGISTER_RxAna_B:
						dte_sndBuffer[0]=2;
						dte_sndBuffer[1]=0;
						dte_sndBuffer[2]=DTE_READ_REGISTER_RxAna_B;
						DTE_Read_Register_RX_B(&dte_sndBuffer[3]);

						rspCode=DTE_Tx_Peripheral();

						return TRUE;

					case DTE_READ_REGISTER_TxAmp_A:
						dte_sndBuffer[0]=2;
						dte_sndBuffer[1]=0;
						dte_sndBuffer[2]=DTE_READ_REGISTER_TxAmp_A;
						DTE_Read_Register_TX_A(&dte_sndBuffer[3]);

						rspCode=DTE_Tx_Peripheral();

						return TRUE;

					case DTE_READ_REGISTER_TxAmp_B:
						dte_sndBuffer[0]=2;
						dte_sndBuffer[1]=0;
						dte_sndBuffer[2]=DTE_READ_REGISTER_TxAmp_B;
						DTE_Read_Register_TX_B(&dte_sndBuffer[3]);

						rspCode=DTE_Tx_Peripheral();

						return TRUE;

					//Test Load Modulation
					case DTE_LOADMODULATION_START:
						emv_appFunction=EMV_FUNCTION_APPLICATION;
						emv_appMode=EMV_MODE_LOOPBACK;
//Test
//						emv_flgLodModulation=TRUE;

						UT_ClearRow(1, 3, FONT0);
						UT_PutStr(1, 0, FONT0, 14, (UCHAR*)"LoopBack Start");
												
						return TRUE;

					case DTE_LOADMODULATION_STOP:
						emv_appFunction=EMV_FUNCTION_NONE;
						emv_appMode=EMV_MODE_NONE;
						emv_flgLodModulation=FALSE;

						UT_ClearRow(1, 3, FONT0);
						UT_PutStr(1, 0, FONT0, 13, (UCHAR*)"LoopBack Stop");
												
						return TRUE;

					default:
						break;
				}
			}
		}
	}

	return FALSE;
}


void DTE_Upload_Data(UINT sndLen, UCHAR *sndData, UCHAR cmdType)
{
	UINT	uplLen=0;	//Upload Length

	if (dte_flgTA == TRUE)
	{
		if ((emv_appFunction == EMV_FUNCTION_APPLICATION) && (emv_appMode == EMV_MODE_PREVALIDATION))
		{
			memset(dte_sndBuffer, 0, DTE_SEND_Buffer_SIZE);

			//Length
			uplLen=sndLen+1+1+2;	//Send Length + Command(1) + Data Header(1) + Data Length(2)
			dte_sndBuffer[0]=uplLen & 0x00FF;
			dte_sndBuffer[1]=(uplLen & 0xFF00) >> 8;

			//Command
			dte_sndBuffer[2]=DTE_UPLOAD_DATA;

			//Data
			if (cmdType == 'C')
			{
				dte_sndBuffer[3]='C';
			}
			else if (cmdType == 'R')
			{
				dte_sndBuffer[3]='R';
			}
			
			dte_sndBuffer[4]=sndLen & 0x00FF;
			dte_sndBuffer[5]=(sndLen & 0xFF00) >> 8;
			memcpy(&dte_sndBuffer[6], sndData, sndLen);

			DTE_Tx_Peripheral();
		}
	}
}

