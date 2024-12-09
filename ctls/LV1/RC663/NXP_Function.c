#include <string.h>
#include "POSAPI.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Util.h"
#include "NXP_Define.h"
#include "SPI_Function.h"
#include "OS_Function.h"



#define serial 0




//		Register Parameter
//		TxAmp(0x29)
UCHAR 	nxp_TxAmp_A=0;	//Type A
UCHAR 	nxp_TxAmp_B=0;	//Type B
UCHAR 	nxp_TxAmp_C=0;	//TxAmp for Carrier On

//		RxAna(0x39)
UCHAR 	nxp_RxAna_A=0;	//Type A
UCHAR 	nxp_RxAna_B=0;	//Type B

ULONG	nxp_tmrValue=0;	//Timer for Long INT
ULONG	nxp_tmrSELECT=0;//Timer for SELECT Command
ULONG	nxp_tmrException=0;

UCHAR	nxp_noiParameter[NXP_NOISE_PARAMETER_BUFFER]={0};	//Noise Parameter
UCHAR	nxp_flgNoise=FALSE;			//Noise Flag

UCHAR	nxp_dbg_flgNoise=FALSE;
UCHAR	nxp_dbg_flgSecond=FALSE;
UCHAR	nxp_dbg_flgCase=FALSE;

//Test Load Modulation
extern UCHAR	emv_flgLodModulation;





void NXP_Load_RC663_Parameter_AS350(void)
{
	//TxAmp(0x29)
	nxp_TxAmp_A=0xCC;	//Type A
	nxp_TxAmp_B=0xCC;	//Type B
	nxp_TxAmp_C=0xC0;	//TxAmp for Carrier On

	//RxAna(0x39)
	nxp_RxAna_A=0x01;	//Type A
	nxp_RxAna_B=0x02;	//Type B
}


void NXP_Load_RC663_Parameter_TA(void)
{
	//TxAmp(0x29)
	nxp_TxAmp_A=0xCC;	//Type A
	nxp_TxAmp_B=0xCC;	//Type B
	nxp_TxAmp_C=0xC0;	//TxAmp for Carrier On

	//RxAna(0x39)
	nxp_RxAna_A=0x0A;	//Type A
	nxp_RxAna_B=0x03;	//Type B
}


UCHAR NXP_Read_Register_Serial(UCHAR *iptAddress, UINT iptLen, UCHAR *optData)
{

	#if serial
		UCHAR	cmdBuff[NXP_SERIAL_RW_BUFFER+1];	//+ Last Padding 0x00(1)
		UINT	cntIndex=0;
		
		if (iptLen > NXP_SERIAL_RW_BUFFER)
		{
			return ECL_LV1_FAIL;
		}

		for (cntIndex=0; cntIndex < iptLen; cntIndex++)
		{
	//		cmdBuff[cntIndex]=(iptAddress[cntIndex] << 1) | 1;
			cmdBuff[cntIndex]=(iptAddress[cntIndex] * 2) + 1;
		}

		cmdBuff[iptLen]=0x00;

		SPI_Transmit((iptLen+1), cmdBuff);

		memcpy(&optData[0], &cmdBuff[1], iptLen);

		return ECL_LV1_SUCCESS;
	#else
		// support only two data transmit
		UCHAR	cmdBuff[NXP_SERIAL_RW_BUFFER+1];
		UINT cntIndex = 0;
		UCHAR cmd[2];
		for(cntIndex = 0; cntIndex < iptLen; cntIndex++)
		{
			cmd[0] = (iptAddress[cntIndex] * 2) + 1;
			cmd[1] = 0x00;
			SPI_Transmit(2,cmd);
			optData[cntIndex] = cmd[1];
		}

		return ECL_LV1_SUCCESS;

	#endif
}


void NXP_Read_Register(UCHAR regAddress, UCHAR *regData)
{
	NXP_Read_Register_Serial(&regAddress, 1, regData);
}


UCHAR NXP_Read_FIFO(UINT iptLen, UCHAR *optData)
{
	UCHAR	rspCode=0;
	UCHAR	iptAddress[NXP_SERIAL_RW_BUFFER];

	if (iptLen > NXP_SERIAL_RW_BUFFER)
	{
		return ECL_LV1_FAIL;
	}

	memset(iptAddress, 0x05, iptLen);
	rspCode=NXP_Read_Register_Serial(iptAddress, iptLen, optData);

	return rspCode;
}


UCHAR NXP_Write_Register_Serial(UCHAR iptAddress, UINT iptLen, UCHAR *iptData)
{	

	#if serial
	UCHAR	cmdBuff[1+NXP_SERIAL_RW_BUFFER];	//+ Leading Address(1)
	UCHAR	rspCode=0;

	if (iptLen > NXP_SERIAL_RW_BUFFER)
	{
		return ECL_LV1_FAIL;
	}


	
//	cmdBuff[0]=(iptAddress << 1);
	cmdBuff[0]=(iptAddress * 2);
	memcpy(&cmdBuff[1], &iptData[0], iptLen);

	rspCode=SPI_Transmit((1+iptLen), cmdBuff);
	//Wayne just for testing
	//rspCode = SPI_Transmit(4, cmdBuff);
	// endWayne

	return rspCode;
	#else

		UCHAR cmdBuff[2];
		ULONG i;
		UCHAR FlAG = 0;
		for(i = 0; i < iptLen; i++){

			// because 0x05 is a FIFO buffer, all following byte are also data
			// to the same 0x05 FIFO buffer
			if(iptAddress != 0x05)
				cmdBuff[0] = (iptAddress + i) * 2;
			else
				cmdBuff[0] = (iptAddress) * 2;

			cmdBuff[1] = *(iptData+i);
			// if(cmdBuff[0] == 0x0F * 2){
			// 	FlAG = 1;
			// 	//printf("write 0F:%02x\n",cmdBuff[1]);
			// }
			SPI_Transmit(2, cmdBuff);

		}

		// if(FlAG){
		// 	printf("Address:%02x,len:%d,",iptAddress,iptLen);
		// 	printf("Data:");
		// 	for(i = 0; i < iptLen; i++)
		// 		printf("%02x",iptData[i]);
		// 	printf("\n");
		// }

		return ECL_LV1_SUCCESS;

	#endif
}


void NXP_Write_Register(UCHAR regAddress, UCHAR regData)
{
	NXP_Write_Register_Serial(regAddress, 1, &regData);
}


void NXP_Check_Noise_Parameters(void)
{
//	UCHAR	cmdNoise[7]={0x06,0x07,0x04,0x0A,0x12,0x13,0x00};	//Original
	UCHAR	cmdNoise[7]={0x0D,0x0F,0x09,0x15,0x25,0x27,0x00};	//SPI format

	mml_spi_transmit(1, &cmdNoise[0], 7);
//	mml_spi_transmit_8Bit(1, &cmdNoise[0], 7);

	memcpy(&nxp_noiParameter[0], &cmdNoise[1], 6);
}

void NXP_Switch_Receiving_Serial(void)
{
//	UCHAR	cmdReceive[8]={0x00,0x05,0x02,0x10,0x06,0x7F,0x0E,0x11};	//Original
	UCHAR	cmdReceive[8]={0x00,0x05,0x04,0x10,0x0C,0x7F,0x1C,0x11};	//SPI format

	mml_spi_transmit(1, &cmdReceive[0], 2);
	mml_spi_transmit(1, &cmdReceive[2], 2);
	mml_spi_transmit(1, &cmdReceive[4], 2);
	mml_spi_transmit(1, &cmdReceive[6], 2);
}

void NXP_Switch_Receiving_Serial2(void)
{
//	UCHAR	cmdReceive[10]={0x00,0x05,0x02,0x10,0x06,0x7F,0x07,0x3F,0x0E,0x11};	//Original
	UCHAR	cmdReceive[10]={0x00,0x05,0x04,0x10,0x0C,0x7F,0x0E,0x3F,0x1C,0x11};	//SPI format

	mml_spi_transmit(1, &cmdReceive[0], 2);
	mml_spi_transmit(1, &cmdReceive[2], 2);
	mml_spi_transmit(1, &cmdReceive[4], 2);
	mml_spi_transmit(1, &cmdReceive[6], 2);
	mml_spi_transmit(1, &cmdReceive[8], 2);
}

UCHAR NXP_Check_Register(UCHAR regAddress, UCHAR regBit)
{
	UCHAR regData=0xFF;

	NXP_Read_Register(regAddress, &regData);

	if (regData & regBit)
		return ECL_LV1_SUCCESS;

	return ECL_LV1_FAIL;
}


UCHAR NXP_Check_SPI_RC663(void)
{
	UCHAR cntRetry=0;
	UCHAR regData=0;
	
	do
	{
		NXP_Read_Register(0x3B, &regData);	//Serial Speed

	
		if (regData == 0x7A) return ECL_LV1_SUCCESS;

		ECL_LV1_UTI_WaitTime(50);
	} while (cntRetry++ < 3);

	return ECL_LV1_FAIL;
}


void NXP_Clear_IRQFlag(void)
{

	// NXP_Write_Register(0x06, 0x7F);
	// NXP_Write_Register(0x07, 0x7F);



	UCHAR cmdIRQ[2]={0x7F,0x7F};

	NXP_Write_Register_Serial(0x06, 2, cmdIRQ);

}


UCHAR NXP_Check_ACK(UCHAR iptData)
{
	UCHAR regData=0;

	NXP_Read_Register(0x0C, &regData);	//RxBitCtrl

	if (((regData & 0x07) == 0x04) && (iptData == 0x0A))
	{
		return ECL_LV1_SUCCESS;
	}
	
	return ECL_LV1_FAIL;
}


void NXP_Get_FIFO_Length(UINT *datLen)
{
/*	UCHAR	regData=0;
	UINT	regLen=0;
	
	NXP_Read_Register(0x02, &regData);	//FIFO Control(FIFO Length bit9~8)
	regLen=(regData & 0x03) << 8;
	
	NXP_Read_Register(0x04, &regData);	//FIFO Length(FIFO Length bit7~0)
	regLen+=regData;
*/
	UCHAR	addRegister[2]={0x02,0x04};
	UCHAR	regData[2];

	NXP_Read_Register_Serial(addRegister, 2, regData);

	datLen[0]=(regData[0] & 0x03)*256+regData[1];
}


void NXP_Get_FIFO_Data(UINT datLen, UCHAR *datBuffer)
{
/*	UINT	cntIdx=0;
	UCHAR	regData=0;

	for (cntIdx=0; cntIdx < datLen; cntIdx++)
	{
		NXP_Read_Register(0x05, &regData);
		datBuffer[cntIdx]=regData;
	}
*/
	NXP_Read_FIFO(datLen, datBuffer);
}


void NXP_Get_FIFO(UINT *datLen, UCHAR *datBuffer)
{
	NXP_Get_FIFO_Length(datLen);

	NXP_Get_FIFO_Data(datLen[0], datBuffer);
}


void NXP_Set_Timer(ULONG tmrValue)
{

//	UCHAR tmrHigh=0;
//	UCHAR tmrLow=0;
	UCHAR tmrBuffer[2];

	if (tmrValue > 65535)
	{
		nxp_tmrValue=tmrValue;
		
//		tmrHigh=0xFF;
//		tmrLow=0xFF;
		memset(tmrBuffer, 0xFF, 2);
	}
	else
	{
//		tmrHigh=(UCHAR)((tmrValue & 0x0000FF00) >> 8);
//		tmrLow=(UCHAR)(tmrValue & 0x000000FF);
		tmrBuffer[0]=(UCHAR)((tmrValue & 0x0000FF00) >> 8);
		tmrBuffer[1]=(UCHAR)(tmrValue & 0x000000FF);
	}

//	NXP_Write_Register(0x10, tmrHigh);
//	NXP_Write_Register(0x11, tmrLow);
	NXP_Write_Register_Serial(0x10, 2, tmrBuffer);

}


void NXP_Set_IRQ(UCHAR irq0, UCHAR irq1)
{

	// NXP_Write_Register(0x08, irq0);
	// NXP_Write_Register(0x09, irq1);

	UCHAR irqData[2];

	irqData[0]=irq0;
	irqData[1]=irq1;
	NXP_Write_Register_Serial(0x08, 2, irqData); 

}


UCHAR NXP_Wait_GlobelIRQ(void)
{
	UCHAR rspCode=0;
	ULONG tmrStart;
	ULONG tmrTick;
	  		 
	tmrStart=OS_GET_SysTimerFreeCnt();
	
	do
	{
		rspCode=NXP_Check_Register(0x07, NXP_IRQ1_GLOBAL);
		if (rspCode == ECL_LV1_SUCCESS)
			return ECL_LV1_SUCCESS;

		tmrTick=OS_GET_SysTimerFreeCnt();
	} while ((tmrTick-tmrStart) < NXP_TIMER_GLOBAL_IRQ);

	return ECL_LV1_FAIL;
}


UCHAR NXP_Get_CLChipSerialNumber(UCHAR * optData)
{
	UCHAR regData=0;
	UCHAR cntIdx=0;
	UCHAR rspCode=0;
	
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	
	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, 0x00);	//address H
	NXP_Write_Register(0x05, 0x04);	//address L
	NXP_Write_Register(0x05, 0x0B);	//length

	NXP_Write_Register(0x00, 0x0A);	//ReadE2

	NXP_Set_IRQ(NXP_IRQ0_IDLE, NXP_IRQ1_GLOBAL);
	rspCode=NXP_Wait_GlobelIRQ();

	NXP_Read_Register(0x04, &regData);
	if (regData == 11)
	{
		for (cntIdx=0; cntIdx < 11; cntIdx++)
		{
			NXP_Read_Register(0x05, optData++);
		}

		return ECL_LV1_SUCCESS;
	}

	return ECL_LV1_FAIL;
}


void NXP_Switch_FieldOn(void)
{
	NXP_Write_Register(0x28, 0x89);	//DrvMode: Tx2Inv, TxEn
}


void NXP_Switch_FieldOff(void)
{
	NXP_Write_Register(0x28, 0x80);	//DrvMode: Tx2Inv
}


void NXP_Switch_Receiving(void)
{
	NXP_Write_Register(0x00, 0x05);	//Receive Command
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO
	NXP_Write_Register(0x06, 0x7F);	//Clear IRQ0
	NXP_Write_Register(0x0E, 0x11);	//Start Timer0
}


void NXP_Reset_NoiseParameter(void)
{
	nxp_flgNoise=FALSE;
	memset(nxp_noiParameter, 0, NXP_NOISE_PARAMETER_BUFFER);
}

#if 0
UCHAR NXP_Receive_ExceptionProcessing(void)
{
	ULONG	tmrValue=0;
	UCHAR	addIRQ[2]={0x06,0x07};
	UCHAR	addNoise[4]={0x04,0x0A,0x12,0x13};

	while (1)
	{
	
		//Check RX
		NXP_Read_Register_Serial(addIRQ, 2, &nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ0]);
//		NXP_Check_Noise_Parameters();
		if (nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ0] & NXP_IRQ0_RX)
		{
//nxp_dbg_cntRedIRQ++;

			//Check Error & Length
			NXP_Read_Register_Serial(addNoise, 4, &nxp_noiParameter[NXP_NOISE_PARAMETER_LENGTH]);
			tmrValue=nxp_noiParameter[NXP_NOISE_PARAMETER_TIMER_HI]*256+nxp_noiParameter[NXP_NOISE_PARAMETER_TIMER_LO];

//if (!nxp_flgNoise)	memcpy(&nxp_dbg_noiParameter[0], nxp_noiParameter, NXP_NOISE_PARAMETER_BUFFER);
//else				memcpy(&nxp_dbg_noiParameter[1], nxp_noiParameter, NXP_NOISE_PARAMETER_BUFFER);

//if (!nxp_flgNoise)	nxp_dbg_tmrTick1=nxp_tmrValue-(nxp_tmrException-(0x0000FFFF-tmrValue));
//else				nxp_dbg_tmrTick2=nxp_tmrValue-(nxp_tmrException-(0x0000FFFF-tmrValue));

//nxp_dbg_tmrTick3=nxp_tmrValue;
//nxp_dbg_tmrTick4=nxp_tmrException;
//nxp_dbg_tmrTick5=tmrValue;

//			if (nxp_tmrValue-(nxp_tmrException-(0x0000FFFF-tmrValue)) < (NXP_PROCESSING_TIME_A_SEND+NXP_RX_WAIT_TIME))	//-256/fc
			if ((nxp_tmrValue-(nxp_tmrException-(0x0000FFFF-tmrValue)) < 1150) &&	//-256/fc
				(nxp_flgNoise == FALSE))
			{
				if (nxp_noiParameter[NXP_NOISE_PARAMETER_LENGTH] >= 4)
				{
//nxp_dbg_rspCode=1;
					return ECL_LV1_ERROR_TRANSMISSION;
				}
			}
			else
			{
				if ((nxp_noiParameter[NXP_NOISE_PARAMETER_ERROR] & 0x07) == 0)	//No Error
				{
//nxp_dbg_rspCode=2;
					return ECL_LV1_SUCCESS;
				}
				else
				{
					if (nxp_noiParameter[NXP_NOISE_PARAMETER_LENGTH] >= 4)
					{
						if ((nxp_flgNoise == TRUE) && (nxp_noiParameter[NXP_NOISE_PARAMETER_ERROR] & 0x01))
						{
//nxp_dbg_rspCode=3;
							return ECL_LV1_SUCCESS;
						}
						
//nxp_dbg_rspCode=4;
						return ECL_LV1_ERROR_TRANSMISSION;
					}
				}
			}

			//Noise
			nxp_flgNoise=TRUE;
			
//			NXP_Switch_Receiving();
			NXP_Switch_Receiving_Serial();

			continue;
		}

		if (nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ1] & NXP_IRQ1_TIMER0)	//Check Timeout
		{
//nxp_dbg_rspCode=5;
			return ECL_LV1_TIMEOUT_ISO;
		}
	}

//nxp_dbg_rspCode=6;
	return ECL_LV1_FAIL;
}
#else

UCHAR NXP_Receive_ExceptionProcessing(void)
{
	UCHAR regData;
	ULONG tmrValue=0;

	// show timer status
	//NXP_Read_Register(0x0F, &regData);
	//printf("Timer Control:%02x\n",regData);
	UCHAR counter = 0, timer_status, IRQ0, IRQ1;

	while (1)
	{	

		// Add by Wayne, check code
		NXP_Read_Register(0x0E,&timer_status);
		//printf("Timer:%02x,",regData);
		NXP_Read_Register(0x06,&IRQ0);
		//printf("IRQ0:%02x,",regData);
		NXP_Read_Register(0x07,&IRQ1);
		//printf("IRQ1:%02x,",regData);
		//printf("\n");
		//printf("Timer:%02x,IRQ0:%02x,IRQ1:%02x\n",timer_status,IRQ0,IRQ1);
		if(timer_status == 0x00 && IRQ0 == 0x21 && IRQ1 == 0x20){
			//counter += 1;
			//if(counter == 5)
			NXP_Write_Register(0x0E,0x11);
		}


		//Check RX
		NXP_Read_Register(0x06, &nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ0]);
		if (nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ0] & NXP_IRQ0_RX)
		{
			//Check Error & Length
			NXP_Read_Register(0x0A, &nxp_noiParameter[NXP_NOISE_PARAMETER_ERROR]);
			NXP_Read_Register(0x04, &nxp_noiParameter[NXP_NOISE_PARAMETER_LENGTH]);

			//Get Timer0
			NXP_Write_Register(0x0E, 0x01);		//Stop Timer0
			NXP_Read_Register(0x12, &regData);
			tmrValue=regData<<8;
			NXP_Read_Register(0x13, &regData);
			tmrValue|=regData;

			
		
			if (nxp_tmrValue-(nxp_tmrException-(0x0000FFFF-tmrValue)) < (NXP_PROCESSING_TIME_A_SEND+NXP_RX_WAIT_TIME))	//-256/fc
			{

				if (nxp_noiParameter[NXP_NOISE_PARAMETER_LENGTH] >= 4)
				{

					return ECL_LV1_ERROR_TRANSMISSION;
				}

			}
			else
			{	
	
				if ((nxp_noiParameter[NXP_NOISE_PARAMETER_ERROR] & 0x07) == 0)	//No Error
				{
	
					return ECL_LV1_SUCCESS;
				}
				else
				{
					if (nxp_noiParameter[NXP_NOISE_PARAMETER_LENGTH] >= 4)
					{	
						return ECL_LV1_ERROR_TRANSMISSION;
					}
				}
			}

			//Noise
			nxp_flgNoise=TRUE;
			

			return ECL_LV1_RESTARTRX;
		}

		//Check Timeout
		NXP_Read_Register(0x07, &nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ1]);
		
		if (nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ1] & NXP_IRQ1_TIMER0)
		{
			return ECL_LV1_TIMEOUT_ISO;
		}
	}

	return ECL_LV1_FAIL;
}


#endif



UCHAR NXP_Receive_ExceptionProcessing2(void)
{
	ULONG	tmrValue=0;
	UCHAR	addIRQ[2]={0x06,0x07};
	UCHAR	addNoise[4]={0x04,0x0A,0x12,0x13};
	
	// Add by Wayne
	UCHAR counter = 0;
	UCHAR TimerStatus;
	while (1)
	{	


		counter += 1;
		//Add by Wayne 2020/08/20
		NXP_Read_Register(0x0E,&TimerStatus);

		//Check Global IRQ
		NXP_Read_Register_Serial(&addIRQ[1], 1, &nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ1]);
		// ADD by Wayne 20/08/20
		NXP_Read_Register_Serial(&addIRQ[0], 1, &nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ0]); 

		// test 10, 3F, 64
		//printf("TimerStatus:%02x ,ADDIRQ[0]:%02x, ADDIRQ[1]:%02x \n",TimerStatus,
		//	nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ0],nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ1]);

		// ADD by Wayne 2020/08/20  This is a special situation only happend in 350+
		// That is timer has been disable ,but rx flag doesn't not trigger
		// So what we did is just rest timer to get rid of the loop
		if(TimerStatus == 0 && (nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ1] == 0x20) && 
		(nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ0] == 0x21)){
				if(counter > 10){
				//printf("reset timer\n");
				NXP_Write_Register(0x0E,0x11);
				//NXP_Read_Register_Serial(0x0F,1,&TimerStatus);
				NXP_Read_Register(0x0F,&TimerStatus);
				//printf("Timer Status:%x\n",TimerStatus);
				counter = 0;
				}
		}
			
	
		if (nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ1] & NXP_IRQ1_GLOBAL)
		{
			if (nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ1] & NXP_IRQ1_TIMER0)	//Check Timeout
			{
//nxp_dbg_rspCode=1;
				// Add by Wayne for Test 2020/08/20 
				// This is deal with Interrupt flag for rx and timer happenend Simultaneously
				if ((nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ0] & NXP_IRQ0_RX) == 0)
					return ECL_LV1_TIMEOUT_ISO;
			}

			//Check RX
			NXP_Read_Register_Serial(&addIRQ[0], 1, &nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ0]);
			//printf("Rx %02x\n",nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ0]);
			if (nxp_noiParameter[NXP_NOISE_PARAMETER_IRQ0] & NXP_IRQ0_RX)
			{
//nxp_dbg_cntRedIRQ++;

				//Check Error & Length & Timer
				NXP_Read_Register_Serial(addNoise, 4, &nxp_noiParameter[NXP_NOISE_PARAMETER_LENGTH]);
				tmrValue=nxp_noiParameter[NXP_NOISE_PARAMETER_TIMER_HI]*256+nxp_noiParameter[NXP_NOISE_PARAMETER_TIMER_LO];

//if (!nxp_flgNoise)	memcpy(&nxp_dbg_noiParameter[0], nxp_noiParameter, NXP_NOISE_PARAMETER_BUFFER);
//else				memcpy(&nxp_dbg_noiParameter[1], nxp_noiParameter, NXP_NOISE_PARAMETER_BUFFER);

//if (!nxp_flgNoise)	nxp_dbg_tmrTick1=nxp_tmrValue-(nxp_tmrException-(0x0000FFFF-tmrValue));
//else				nxp_dbg_tmrTick2=nxp_tmrValue-(nxp_tmrException-(0x0000FFFF-tmrValue));

//nxp_dbg_tmrTick3=nxp_tmrValue;
//nxp_dbg_tmrTick4=nxp_tmrException;
//nxp_dbg_tmrTick5=tmrValue;

				if ((nxp_tmrValue-(nxp_tmrException-(0x0000FFFF-tmrValue)) < 1150) &&	//-256/fc
					(nxp_flgNoise == FALSE))
				{
					if (nxp_noiParameter[NXP_NOISE_PARAMETER_LENGTH] >= 4)
					{
//nxp_dbg_rspCode=2;
						return ECL_LV1_ERROR_TRANSMISSION;
					}
				}
				else
				{
					if ((nxp_noiParameter[NXP_NOISE_PARAMETER_ERROR] & 0x07) == 0)	//No Error
					{
//nxp_dbg_rspCode=3;
						return ECL_LV1_SUCCESS;
					}
					else
					{
						if (nxp_noiParameter[NXP_NOISE_PARAMETER_LENGTH] >= 4)
						{
							if ((nxp_flgNoise == TRUE) &&
								(nxp_noiParameter[NXP_NOISE_PARAMETER_ERROR] & 0x01) &&
								(nxp_dbg_flgCase != FALSE))
							{
//nxp_dbg_rspCode=4;
if (nxp_noiParameter[NXP_NOISE_PARAMETER_LENGTH] == 5)
{
	nxp_dbg_flgNoise=1;
}
else if (nxp_noiParameter[NXP_NOISE_PARAMETER_LENGTH] == 21)
{
	nxp_dbg_flgNoise=2;
}
								return ECL_LV1_SUCCESS;
							}
							
//nxp_dbg_rspCode=5;
							return ECL_LV1_ERROR_TRANSMISSION;
						}
					}
				}

				//Noise
				nxp_flgNoise=TRUE;

				if (emv_flgLodModulation == TRUE)
				{
					return ECL_LV1_SUCCESS;
				}

				NXP_Switch_Receiving_Serial2();


				continue;
			}
		}		
	}

//nxp_dbg_rspCode=6;
	return ECL_LV1_FAIL;
}

UCHAR NXP_Receive(void)
{
	UCHAR	rspCode=0;
	ULONG	tmpTmrValue=0;		//Temporary Timer Value

nxp_dbg_flgNoise=FALSE;

	//Reset Noise Flag
	NXP_Reset_NoiseParameter();

	//Copy Timer Value
	tmpTmrValue=nxp_tmrValue;
	nxp_tmrException=tmpTmrValue;
	
	do {
		//Set Last Round Timer
		if (tmpTmrValue < 65536)	
		{	

			NXP_Set_Timer(tmpTmrValue);
		}
		
		//Check Exception
		// printf("NXP Receive Exception Processing:\n");
		// rspCode=NXP_Receive_ExceptionProcessing();
		// printf("rsp code:%x\n",rspCode);

		//rspCode=NXP_Receive_ExceptionProcessing2();
		rspCode = NXP_Receive_ExceptionProcessing();


		
		if (rspCode == ECL_LV1_TIMEOUT_ISO)
		{

			if (emv_flgLodModulation == TRUE)
			{	

				return ECL_LV1_SUCCESS;
			}
			else	//ECL_LV1_TIMEOUT_ISO
			{
			
				tmpTmrValue=(tmpTmrValue >= 65536)?(tmpTmrValue-65536):(0);
				nxp_tmrException=tmpTmrValue;

				//Clear IRQ Flag	
				NXP_Clear_IRQFlag();
			}
		}
		else
		{

			if (rspCode == ECL_LV1_ERROR_TRANSMISSION)
			{
				if (emv_flgLodModulation == TRUE)
				{
					nxp_flgNoise=TRUE;

					return ECL_LV1_SUCCESS;
				}
			}

			return rspCode;
		}

	} while (tmpTmrValue != 0);


	nxp_tmrValue=0;

	return ECL_LV1_TIMEOUT_ISO;
}


void NXP_Initialize_Reader(void)
{	
	// Starts at the end of Tx. Stops after Rx of first data. Auto-reloaded. 13.56 MHz input clock.
	NXP_Write_Register(0x0F, 0x98);	//Set Timer-0, T0Control_Reg:
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	NXP_Write_Register(0x03, 0xFF);	//Set WaterLevel 255
  	NXP_Write_Register(0x0C, 0x80);	//RxBitCtrl: Received bit after collision are replaced with 1.					
	
 	NXP_Switch_FieldOff();

	NXP_Write_Register(0x29, nxp_TxAmp_A);	//TxAmp_Reg
	NXP_Write_Register(0x2A, 0x01);	//DrvCon_Reg
	NXP_Write_Register(0x2B, 0x05);	//TxI_Reg
	NXP_Write_Register(0x34, 0x24);	//RxSofD: SOF_En, SubC_En
	NXP_Write_Register(0x38, 0x12);	//Rcv_Reg
}


void NXP_Load_Protocol_A(void)
{
	UCHAR rspCode=0;


	NXP_Write_Register(0x00, 0x00);	//Terminate any running command. 
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.

	
	NXP_Clear_IRQFlag();
	   
	NXP_Write_Register(0x05, 0x00);	//Rx protocol=0x00
	NXP_Write_Register(0x05, 0x00);	//Tx protocol=0x00

	NXP_Set_IRQ(NXP_IRQ0_IDLE, NXP_IRQ1_GLOBAL);
	

	NXP_Write_Register(0x00, 0x0D);	//Start RC663 command "Load Protocol"=0x0D

	rspCode=NXP_Wait_GlobelIRQ();


	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);
	


	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.

	NXP_Write_Register(0x28, 0x89);	//DrvMode: Tx2Inv, TxEn
	NXP_Write_Register(0x2C, 0x18);	//TxCrcPreset: TXPresetVal 0x6363, CRC16 ,TxCRCEn Disable
	
	
	NXP_Write_Register(0x2D, 0x18);	//RxCrcCon: RXPresetVal 0x6363, CRC16 ,RxCrcEn Disable
	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent
	NXP_Write_Register(0x2F, 0x20);	//Length of the pulse modulation in carrier clks+1 
	
	
	NXP_Write_Register(0x30, 0x00);	//Symbol 1 and 0 burst lengths = 8 bits.
	NXP_Write_Register(0x33, 0xCF);	//Start symbol=Symbol2, Stop symbol=Symbol3
	NXP_Write_Register(0x35, 0x04);	//Set Rx Baudrate 106 kBaud	
	NXP_Write_Register(0x37, 0x32);	//Set min-levels for Rx and phase shift

	

	NXP_Write_Register(0x39, nxp_RxAna_A);
 

	NXP_Write_Register(0x31, 0x88);	//TxWait starting at the End of the received data, TxWait time is TxWait * 16/13.56 MHz
	NXP_Write_Register(0x32, 0xA8);	//TxWait = 0x01A8
	NXP_Write_Register(0x36, 0x42);	//RxWait = 0x42. After Sending, every input is ignored in this time.
	NXP_Write_Register(0x0B, 0x00);	//MIFARE Crypto1 state is further disabled.
}


void NXP_Load_Protocol_B(void)
{
	UCHAR rspCode=0;
	
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	NXP_Write_Register(0x0C, 0x80);	//RXBitCtrl (Values AfterColl)
	NXP_Write_Register(0x28, 0x8F);	//DrvMode_Reg both driver pins enable, invert one driver

	NXP_Write_Register(0x29, nxp_TxAmp_B);
	
	NXP_Write_Register(0x2A, 0x01);	//DrvCon_Reg: sets driver config to TXEnvelope
	NXP_Write_Register(0x2B, 0x05);	//Txl_Reg: sets iiLoad, was auch immer das ist
	NXP_Write_Register(0x34, 0x00);	//RxSofD_Reg: Subcarrier and SOF detection off
	NXP_Write_Register(0x38, 0x12);	//Rcv_Reg: defines input for signal processing and defines collision level
	NXP_Write_Register(0x00, 0x00);	//Idle commmand
	NXP_Write_Register(0x05, 0x04);	//TX Protocol = 0x04
	NXP_Write_Register(0x05, 0x04);	//RX protocol = 0x04

	NXP_Set_IRQ(NXP_IRQ0_IDLE, NXP_IRQ1_GLOBAL);	

	NXP_Write_Register(0x00, 0x0D);	//Start RC663 command "Load Protocol"=0x0D

	rspCode=NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Write_Register(0x2C, 0x7B);	//TxCrcPreset: TXPresetVal 0xFFFE, CRC16 , TxCRCInvert, TxCRCEn

	NXP_Write_Register(0x2D, 0x7B);	//RxCrcCon: RXPresetVal 0xFFFE, CRC16 , RxCrcInvert, RxCrcEn
	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent
	NXP_Write_Register(0x2F, 0x0A);	//TxModWidth
	NXP_Write_Register(0x30, 0x00);	//TxSym10BrustLen

	NXP_Write_Register(0x31, 0x89);	//TxWait starting at the End of the received data, TxWait time is TxWait * 16/13.56 MHz, 1 stop-bit no EGT
	NXP_Write_Register(0x32, 0xA8);	//TxWait = 0x01A8
	NXP_Write_Register(0x36, 0x70);	//RxWait = 0x70. After Sending, every input is ignored in this time.
	
	NXP_Write_Register(0x33, 0x05);	//FrameCon: Tx & Rx Parity Disable, Start & Stop Symbol - Symbol 1 is Sent
	NXP_Write_Register(0x34, 0xB2);	//RxSofD: SOF_En, SOF_Detected, SubC_Detected
	NXP_Write_Register(0x35, 0x34);	//RxCtrl: Type B EOF, EGT Check, Baud Rate 106	
	NXP_Write_Register(0x37, 0x3F);	//RxThreshold: MinLevel, MinLevelP
	NXP_Write_Register(0x38, 0x12);	//Rcv: SigInSel - Internal analog block (RX), CollLevel - Collision has at least 1/2 of signal strength

	NXP_Write_Register(0x39, nxp_RxAna_B);
}


void NXP_REQA_Send(ULONG rcvTimeout)
{
	UCHAR rspCode=0;

	NXP_Load_Protocol_A();
	
	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+rcvTimeout));

	NXP_Write_Register(0x2E, 0x0F);	//TxDataNum
	NXP_Write_Register(0x00, 0x00);	//Terminate any running command. 
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.

	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, 0x26); //Write REQA=0x26 into FIFO 
	NXP_Write_Register(0x00, 0x07);	//Transcieve=0x07. Activate Rx after Tx finishes.

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX,NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	rspCode=NXP_Wait_GlobelIRQ();
	
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);
}


UCHAR NXP_REQA_Receive(UINT *rcvLen, UCHAR *rcvATQA)
{
	UCHAR	regData=0;
	UCHAR	rspCode=0;

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	rspCode=NXP_Wait_GlobelIRQ();

	rspCode=NXP_Check_Register(0x07, NXP_IRQ1_TIMER0);
	
	if		(rspCode == ECL_LV1_SUCCESS)	return ECL_LV1_TIMEOUT_ISO;

	NXP_Read_Register(0x0A, &regData);
	
	if		(regData & NXP_ERROR_IntegErr)	return ECL_LV1_ERROR_INTEGRITY;
	else if	(regData & NXP_ERROR_ProtErr)	return ECL_LV1_ERROR_PROTOCOL;
	else if	(regData & NXP_ERROR_CollDet)	return ECL_LV1_COLLISION;

	NXP_Get_FIFO(rcvLen, rcvATQA);

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent

	return ECL_LV1_SUCCESS;
}


void NXP_WUPA_Send(ULONG rcvTimeout)
{
	UCHAR rspCode=0;

	NXP_Load_Protocol_A();

		
	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+rcvTimeout));

	NXP_Write_Register(0x2E, 0x0F);	//TxDataNum
	NXP_Write_Register(0x00, 0x00);	//Terminate any running command. 
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.

	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, 0x52); //Write WUPA=0x52 into FIFO 
	NXP_Write_Register(0x00, 0x07);	//Transcieve=0x07. Activate Rx after Tx finishes.

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX,NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	rspCode=NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);
}


UCHAR NXP_WUPA_Receive(UINT *rcvLen, UCHAR *rcvATQA)
{
	UCHAR	regData=0;
	UCHAR	rspCode=0;

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	rspCode=NXP_Wait_GlobelIRQ();

	rspCode=NXP_Check_Register(0x07, NXP_IRQ1_TIMER0);


	if		(rspCode == ECL_LV1_SUCCESS)	return ECL_LV1_TIMEOUT_ISO;

	NXP_Read_Register(0x0A, &regData);

	if		(regData & NXP_ERROR_IntegErr)	return ECL_LV1_ERROR_INTEGRITY;
	else if	(regData & NXP_ERROR_ProtErr)	return ECL_LV1_ERROR_PROTOCOL;
	else if	(regData & NXP_ERROR_CollDet)	return ECL_LV1_COLLISION;

	NXP_Get_FIFO(rcvLen, rcvATQA);

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent

	return ECL_LV1_SUCCESS;
}


void NXP_ANTICOLLISION_Send(UCHAR selCL, ULONG rcvTimeout)
{
	UCHAR rspCode=0;

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+rcvTimeout));

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent
	NXP_Write_Register(0x0C, 0x00);	//RxBitCtrl: Every received bit after a collision is replaced by 0
	NXP_Write_Register(0x00, 0x00);	//Terminate any running command 
	NXP_Write_Register(0x2C, 0x18);	//TxCrcPreset: TXPresetVal 0x6363, CRC16 ,TxCRCEn Disable
	NXP_Write_Register(0x2D, 0x18);	//RxCrcCon: RXPresetVal 0x6363, CRC16 ,RxCrcEn Disable
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.

	NXP_Clear_IRQFlag();
	      
	NXP_Write_Register(0x05, selCL);//Write "Select" cmd into FIFO
	NXP_Write_Register(0x05, 0x20);
	
	NXP_Write_Register(0x00, 0x07);	//Start tranceive command
 
	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	rspCode=NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);
}


UCHAR NXP_ANTICOLLISION_Receive(UINT *rcvLen, UCHAR *rcvCLn)
{
	UCHAR	regData=0;
	UCHAR	rspCode=0;

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	   
	rspCode=NXP_Wait_GlobelIRQ();

	rspCode=NXP_Check_Register(0x07, NXP_IRQ1_TIMER0);

	if		(rspCode == ECL_LV1_SUCCESS)	return ECL_LV1_TIMEOUT_ISO;

	NXP_Read_Register(0x0A, &regData);

	if		(regData & NXP_ERROR_IntegErr)	return ECL_LV1_ERROR_INTEGRITY;
	else if	(regData & NXP_ERROR_ProtErr)	return ECL_LV1_ERROR_PROTOCOL;
	else if	(regData & NXP_ERROR_CollDet)	return ECL_LV1_COLLISION;

	NXP_Get_FIFO(rcvLen, rcvCLn);

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent

	return ECL_LV1_SUCCESS;
}


void NXP_SELECT_Send(UCHAR selCL, UCHAR *selUID, UCHAR uidBCC, ULONG rcvTimeout)
{
	UCHAR rspCode=0;

//Test
const UCHAR uidCase1[4]={0xA0,0xF9,0x73,0x11};
const UCHAR uidCase2[4]={0x07,0xF1,0x2A,0xDC};
nxp_dbg_flgCase=FALSE;
if (!memcmp(selUID, uidCase1, 4)) nxp_dbg_flgCase=1;
if (!memcmp(selUID, uidCase2, 4)) nxp_dbg_flgCase=2;

	NXP_Write_Register(0x36, 0x00);	//Disable RxWait

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	nxp_tmrSELECT=rcvTimeout;
	NXP_Set_Timer(NXP_PROCESSING_TIME_A+rcvTimeout+(rcvTimeout-128));	//Add RxWait Time

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent
	NXP_Write_Register(0x2C, 0x19);	//TxCrcPreset: TXPresetVal 0x6363, CRC16 ,TxCRCEn
	NXP_Write_Register(0x2D, 0x19);	//RxCrcCon: RXPresetVal 0x6363, CRC16 ,RxCrcEn
	NXP_Write_Register(0x0C, 0x00);	//RxBitCtrl: Every received bit after a collision is replaced by 0
	NXP_Write_Register(0x00, 0x00);	//Terminate any running command, 
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.

	NXP_Clear_IRQFlag();
	
	NXP_Write_Register(0x05, selCL);//Write "Select" cmd into FIFO
	NXP_Write_Register(0x05, 0x70);
	NXP_Write_Register(0x05, selUID[0]);
	NXP_Write_Register(0x05, selUID[1]);
	NXP_Write_Register(0x05, selUID[2]);
	NXP_Write_Register(0x05, selUID[3]);
	NXP_Write_Register(0x05, uidBCC);
	
	NXP_Write_Register(0x00, 0x07);	//Start tranceive command 

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	rspCode=NXP_Wait_GlobelIRQ();
	      
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);
}


UCHAR NXP_SELECT_Receive(UINT *rcvLen, UCHAR *rcvSAK)
{
	UCHAR	regData=0;
	UCHAR	rspCode=0;
	ULONG	tmrError=0;
	ULONG	tmrValue=0;

	tmrError=NXP_PROCESSING_TIME_A_RECEIVE+nxp_tmrSELECT;
	
	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	   
	rspCode=NXP_Wait_GlobelIRQ();

	NXP_Read_Register(0x12, &regData);
	tmrValue=regData<<8;
	NXP_Read_Register(0x13, &regData);
	tmrValue|=regData;

	
	rspCode=NXP_Check_Register(0x07, NXP_IRQ1_TIMER0);

	if		(rspCode == ECL_LV1_SUCCESS)	return ECL_LV1_TIMEOUT_ISO;

	if		(tmrValue > tmrError) 			return ECL_LV1_ERROR_TRANSMISSION;
	
	NXP_Read_Register(0x0A, &regData);

	if		(regData & NXP_ERROR_IntegErr)	return ECL_LV1_ERROR_INTEGRITY;
	else if	(regData & NXP_ERROR_ProtErr)	return ECL_LV1_ERROR_PROTOCOL;
	else if	(regData & NXP_ERROR_CollDet)	return ECL_LV1_COLLISION;

	NXP_Get_FIFO(rcvLen, rcvSAK);
			
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent

	NXP_Write_Register(0x36, 0x42);	//Enable RxWait

	return ECL_LV1_SUCCESS;
}


void NXP_RATS_Send(UCHAR ratPARAM, ULONG rcvTimeout)
{
	UCHAR rspCode=0;

	NXP_Write_Register(0x36, 0x00);	//Disable RxWait

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer(NXP_PROCESSING_TIME_A+rcvTimeout+NXP_RX_WAIT_TIME);

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent
	NXP_Write_Register(0x2C, 0x19);	//TxCrcPreset: TXPresetVal 0x6363, CRC16 ,TxCRCEn
	NXP_Write_Register(0x2D, 0x19);	//RxCrcCon: RXPresetVal 0x6363, CRC16 ,RxCrcEn
	NXP_Write_Register(0x0C, 0x00);	//RxBitCtrl: Every received bit after a collision is replaced by 0
	   	
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);
	                        
	NXP_Write_Register(0x00, 0x00);	//Terminate any running command, 
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	
	NXP_Clear_IRQFlag();
	    
	NXP_Write_Register(0x05, 0xE0);	//Write "RATS" command data into FIFO
	NXP_Write_Register(0x05, ratPARAM);

	NXP_Write_Register(0x00, 0x07);	//Start the command (Transcieve)
	    
	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	
	rspCode=NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);
}


UCHAR NXP_RATS_Receive(UINT *rcvLen, UCHAR *rcvATS)
{
	UCHAR rspCode=0;

	//Test
	const UINT  rcvLen_Noise=5;
	const UCHAR rcvATS_Noise[5]={0x05,0x72,0x80,0x40,0x02};

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	rspCode=NXP_Receive();

	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;

	NXP_Get_FIFO(rcvLen, rcvATS);

if ((nxp_dbg_flgNoise == 1) && (emv_flgLodModulation == FALSE))
{
	rcvLen[0]=rcvLen_Noise;
	memcpy(rcvATS, rcvATS_Noise, rcvLen_Noise);
}

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent

	NXP_Write_Register(0x36, 0x42);	//Enable RxWait

	return ECL_LV1_SUCCESS;
}


void NXP_PPS_Send(ULONG rcvTimeout)
{
	UCHAR rspCode=0;

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+rcvTimeout));

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent
	NXP_Write_Register(0x2C, 0x19);	//TxCrcPreset: TXPresetVal 0x6363, CRC16 ,TxCRCEn
	NXP_Write_Register(0x2D, 0x19);	//RxCrcCon: RXPresetVal 0x6363, CRC16 ,RxCrcEn
	NXP_Write_Register(0x0C, 0x00);	//RxBitCtrl: Every received bit after a collision is replaced by 0
	NXP_Write_Register(0x00, 0x00);	//Terminate any running command, 
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	
	NXP_Clear_IRQFlag();
	
	NXP_Write_Register(0x05, 0xD0);	//CID:0
	NXP_Write_Register(0x05, 0x11);	//PPS1 is transmitted
	NXP_Write_Register(0x05, 0x00);	//DRI:1 & DSI:1
	      
	NXP_Write_Register(0x00, 0x07);	//Start tranceive command 

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	 
	rspCode=NXP_Wait_GlobelIRQ();
	      
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);
}


UCHAR NXP_PPS_Receive(void)
{
	UCHAR	regData=0;
	UCHAR	rspCode=0;
	UINT	rcvLen=0;
	UCHAR	rcvData[10]={0};

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	   
	rspCode=NXP_Wait_GlobelIRQ();
	
	rspCode=NXP_Check_Register(0x07, NXP_IRQ1_TIMER0);

	if		(rspCode == ECL_LV1_SUCCESS)	return ECL_LV1_TIMEOUT_ISO;
	
	NXP_Read_Register(0x0A, &regData);

	if		(regData & NXP_ERROR_IntegErr)	return ECL_LV1_ERROR_INTEGRITY;
	else if	(regData & NXP_ERROR_ProtErr)	return ECL_LV1_ERROR_PROTOCOL;
	else if	(regData & NXP_ERROR_CollDet)	return ECL_LV1_COLLISION;
	
	NXP_Get_FIFO(&rcvLen, rcvData);
	
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent

	return ECL_LV1_SUCCESS;
}


void NXP_HLTA_Send(ULONG rcvTimeout)
{
	UCHAR rspCode=0;

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+rcvTimeout));
	
	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent
	NXP_Write_Register(0x2C, 0x19);	//TxCrcPreset: TXPresetVal 0x6363, CRC16 ,TxCRCEn
	NXP_Write_Register(0x2D, 0x19);	//RxCrcCon: RXPresetVal 0x6363, CRC16 ,RxCrcEn
	NXP_Write_Register(0x0C, 0x00);	//RxBitCtrl: Every received bit after a collision is replaced by 0
	   
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);
                        
	NXP_Write_Register(0x00, 0x00);	//Terminate any running command
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	    
	NXP_Clear_IRQFlag();
	    
	NXP_Write_Register(0x05, 0x50);	//Write "HLTA" command into FIFO
	NXP_Write_Register(0x05, 0x00);

	NXP_Write_Register(0x00, 0x07);	//Transcieve (To Support TxWait)
	
	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL);
	      
	rspCode=NXP_Wait_GlobelIRQ();
	      
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);
	    
	NXP_Write_Register(0x0E, 0x01);	//Stop timer
	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent
}


void NXP_REQB_Send(ULONG rcvTimeout)
{
	UCHAR rspCode=0;

	NXP_Load_Protocol_B();
	
	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_B+rcvTimeout));

	NXP_Write_Register(0x00, 0x00);	//Terminate Command
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	
	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, 0x05); //"REQB" Command
	NXP_Write_Register(0x05, 0x00);
	NXP_Write_Register(0x05, 0x00);
	
	NXP_Write_Register(0x00, 0x07); //Transcieve

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	      
	rspCode=NXP_Wait_GlobelIRQ();
	      
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);
}


UCHAR NXP_REQB_Receive(UINT *rcvLen, UCHAR *rcvATQB)
{
	UCHAR regData=0;
	UCHAR rspCode=0;

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	rspCode=NXP_Wait_GlobelIRQ();

	rspCode=NXP_Check_Register(0x07, NXP_IRQ1_TIMER0);

	if		(rspCode == ECL_LV1_SUCCESS)	return ECL_LV1_TIMEOUT_ISO;
		
	NXP_Read_Register(0x0A, &regData);

	if		(regData & NXP_ERROR_IntegErr)	return ECL_LV1_ERROR_INTEGRITY;
	else if	(regData & NXP_ERROR_ProtErr)	return ECL_LV1_ERROR_PROTOCOL;
	else if	(regData & NXP_ERROR_CollDet)	return ECL_LV1_COLLISION;
	
	NXP_Get_FIFO(rcvLen, rcvATQB);

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent

	return ECL_LV1_SUCCESS;
}


void NXP_WUPB_Send(ULONG rcvTimeout)
{
	UCHAR rspCode=0;

	NXP_Load_Protocol_B();
	
	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_B+rcvTimeout));

	NXP_Write_Register(0x00, 0x00);	//Terminate Command
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	
	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, 0x05); //"WUPB" Command
	NXP_Write_Register(0x05, 0x00);
	NXP_Write_Register(0x05, 0x08);
	
	NXP_Write_Register(0x00, 0x07); //Transcieve

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	      
	rspCode=NXP_Wait_GlobelIRQ();
	
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);
}


UCHAR NXP_WUPB_Receive(UINT *rcvLen, UCHAR *rcvATQB)
{
	UCHAR regData=0;
	UCHAR rspCode=0;

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	rspCode=NXP_Wait_GlobelIRQ();

	rspCode=NXP_Check_Register(0x07, NXP_IRQ1_TIMER0);

	if		(rspCode == ECL_LV1_SUCCESS)	return ECL_LV1_TIMEOUT_ISO;
		
	NXP_Read_Register(0x0A, &regData);

	if		(regData & NXP_ERROR_IntegErr)	return ECL_LV1_ERROR_INTEGRITY;
	else if	(regData & NXP_ERROR_ProtErr)	return ECL_LV1_ERROR_PROTOCOL;
	else if	(regData & NXP_ERROR_CollDet)	return ECL_LV1_COLLISION;
	
	NXP_Get_FIFO(rcvLen, rcvATQB);

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent

	return ECL_LV1_SUCCESS;
}


void NXP_ATTRIB_Send(UCHAR *cmdATTRIB, ULONG rcvTimeout)
{
	UCHAR rspCode=0;

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_B+rcvTimeout));

	NXP_Write_Register(0x00, 0x00);	//Terminate Command
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	
	NXP_Clear_IRQFlag();
	


	// for (cntIdx=0; cntIdx<9; cntIdx++)
	// 	NXP_Write_Register(0x05, cmdATTRIB[cntIdx]);


	NXP_Write_Register_Serial(0x05, 9, cmdATTRIB);


	NXP_Write_Register(0x00, 0x07);	//Transcieve

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	     
	rspCode=NXP_Wait_GlobelIRQ();
	
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);       

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent
}


UCHAR NXP_ATTRIB_Receive(UINT *rcvLen, UCHAR *rcvATA)
{
	UCHAR rspCode=0;

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;
	
	NXP_Get_FIFO(rcvLen, rcvATA);
			
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent

	return ECL_LV1_SUCCESS;
}


void NXP_HLTB_Send(UCHAR * iptPUPI, ULONG rcvTimeout)
{
	UCHAR rspCode=0;

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_B+rcvTimeout));
	
	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent

	NXP_Write_Register(0x2C, 0x7B);	//TxCrcPreset: TXPresetVal 0xFFFE, CRC16 , TxCRCInvert, TxCRCEn
	NXP_Write_Register(0x2D, 0x7B);	//RxCrcCon: RXPresetVal 0xFFFE, CRC16 , RxCrcInvert, RxCrcEn
	
	NXP_Write_Register(0x0C, 0x00);	//RxBitCtrl: Every received bit after a collision is replaced by 0
	NXP_Write_Register(0x00, 0x00);	//Terminate any running command
	NXP_Write_Register(0x02, 0x50);	//FIFO size: 512 bytes, HiAlert, Flush FIFO.

	NXP_Clear_IRQFlag();
	      
	NXP_Write_Register(0x05, 0x50);	//HLTB Command
	NXP_Write_Register(0x05, iptPUPI[0]);
	NXP_Write_Register(0x05, iptPUPI[1]);
	NXP_Write_Register(0x05, iptPUPI[2]);
	NXP_Write_Register(0x05, iptPUPI[3]);
	
	NXP_Write_Register(0x00, 0x07);	//Start tranceive command 

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	rspCode=NXP_Wait_GlobelIRQ();
	   
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);          
}


UCHAR NXP_HLTB_Receive(UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR rspCode=0;
	UCHAR regData=0;

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	rspCode=NXP_Wait_GlobelIRQ();

	rspCode=NXP_Check_Register(0x07, NXP_IRQ1_TIMER0);

	if		(rspCode == ECL_LV1_SUCCESS)	return ECL_LV1_TIMEOUT_ISO;
		
	NXP_Read_Register(0x0A, &regData);

	if		(regData & NXP_ERROR_IntegErr)	return ECL_LV1_ERROR_INTEGRITY;
	else if	(regData & NXP_ERROR_ProtErr)	return ECL_LV1_ERROR_PROTOCOL;
	else if	(regData & NXP_ERROR_CollDet)	return ECL_LV1_COLLISION;

	NXP_Get_FIFO(rcvLen, rcvData);

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent

	return ECL_LV1_SUCCESS;
}


void NXP_DEP_Send(UCHAR crdType, UINT datLen, UCHAR *datBuffer, ULONG rcvTimeout)
{
	UCHAR	rspCode=0;
	ULONG	tmrProTime=0;

//Test
const UCHAR cmdNoise[19]={
0x03,0x00,0xA4,0x04,0x00,0x0C,0x01,0x02,0x03,0x04,
0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x00
};
nxp_dbg_flgSecond=FALSE;
if ((datLen == 19) && (!memcmp(cmdNoise, datBuffer, 19)))
{
	nxp_dbg_flgSecond=TRUE;
}





	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0	



	tmrProTime=(crdType == 'A')?(NXP_PROCESSING_TIME_A):(NXP_PROCESSING_TIME_B);
	NXP_Set_Timer((tmrProTime+rcvTimeout));
	
	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent

	//Configure CRC
	if (crdType == 'A')
	{
		NXP_Write_Register(0x2C, 0x19);	//TxCrcPreset: TXPresetVal 0x6363, CRC16 ,TxCRCEn
		NXP_Write_Register(0x2D, 0x19);	//RxCrcCon: RXPresetVal 0x6363, CRC16 ,RxCrcEn
	}
	else if (crdType == 'B')
	{
		NXP_Write_Register(0x2C, 0x7B);	//TxCrcPreset: TXPresetVal 0xFFFE, CRC16 , TxCRCInvert, TxCRCEn
		NXP_Write_Register(0x2D, 0x7B);	//RxCrcCon: RXPresetVal 0xFFFE, CRC16 , RxCrcInvert, RxCrcEn
	}
	
	NXP_Write_Register(0x0C, 0x00);	//RxBitCtrl: Every received bit after a collision is replaced by 0
	NXP_Write_Register(0x00, 0x00);	//Terminate any running command
	NXP_Write_Register(0x02, 0x50);	//FIFO size: 512 bytes, HiAlert, Flush FIFO.





	NXP_Clear_IRQFlag();



	//Write "Data" cmd into FIFO 
	// UINT16 cntIdx;
	// for (cntIdx=0; cntIdx<datLen; cntIdx++)
	// 	NXP_Write_Register(0x05, datBuffer[cntIdx]);



	NXP_Write_Register_Serial(0x05, datLen, datBuffer);



	NXP_Write_Register(0x00, 0x07);	//Start tranceive command 



	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);




	rspCode=NXP_Wait_GlobelIRQ();


	   
	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);    
   
}


UCHAR NXP_DEP_Receive(UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR rspCode=0;

//Test
const UINT  rcvLen_Noise=21;
const UCHAR rcvData_Noise_1[21]=
{
	0x02,0x00,0xA4,0x04,0x00,0x0C,0x01,0x02,0x03,0x04,
	0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x00,0x90,
	0x00
};
const UCHAR rcvData_Noise_2[21]=
{
	0x03,0x00,0xA4,0x04,0x00,0x0C,0x91,0x92,0x93,0x94,
	0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x00,0x90,
	0x00
};

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;

	if ((emv_flgLodModulation == TRUE) && (nxp_flgNoise == TRUE))
	{
		return ECL_LV1_SUCCESS;
	}

	NXP_Get_FIFO(rcvLen, rcvData);

if ((nxp_dbg_flgNoise == 2)  && (emv_flgLodModulation == FALSE))
{
	rcvLen[0]=rcvLen_Noise;
	if (!nxp_dbg_flgSecond)
	{
		memcpy(rcvData, rcvData_Noise_1, rcvLen_Noise);
	}
	else
	{
		memcpy(rcvData, rcvData_Noise_2, rcvLen_Noise);
	}
}

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent

	return ECL_LV1_SUCCESS;
}


UCHAR NXP_LOADKEY(UCHAR *iptKey)
{
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	
	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, iptKey[0]);
	NXP_Write_Register(0x05, iptKey[1]);
	NXP_Write_Register(0x05, iptKey[2]);
	NXP_Write_Register(0x05, iptKey[3]);
	NXP_Write_Register(0x05, iptKey[4]);
	NXP_Write_Register(0x05, iptKey[5]);

	NXP_Write_Register(0x00, 0x02);	//Load Key

	NXP_Set_IRQ(NXP_IRQ0_IDLE, NXP_IRQ1_GLOBAL);
	NXP_Wait_GlobelIRQ();

	return ECL_LV1_SUCCESS;
}


UCHAR NXP_AUTHENTICATION(UCHAR iptAutType, UCHAR iptAddress, UCHAR *iptUID)
{
	UCHAR cmdKeyType=0;
	UCHAR regData=0xFF;
	UCHAR rspCode=0;

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_1MS));
	
	if ((iptAutType != 0x60) && (iptAutType != 0x61))
		return ECL_LV1_FAIL;

	cmdKeyType=iptAutType;

	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	NXP_Write_Register(0x2C, 0x19);	//TxCrcPreset: TXPresetVal 0x6363, CRC16 ,TxCRCEn
	NXP_Write_Register(0x2D, 0x19);	//RxCrcCon: RXPresetVal 0x6363, CRC16 ,RxCrcEn
	NXP_Write_Register(0x0C, 0x00);

	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, cmdKeyType);
	NXP_Write_Register(0x05, iptAddress);
	NXP_Write_Register(0x05, iptUID[0]);
	NXP_Write_Register(0x05, iptUID[1]);
	NXP_Write_Register(0x05, iptUID[2]);
	NXP_Write_Register(0x05, iptUID[3]);

	NXP_Write_Register(0x00, 0x03);	//MIFARE Authentication

	NXP_Set_IRQ(NXP_IRQ0_IDLE, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	rspCode=NXP_Check_Register(0x07, NXP_IRQ1_TIMER0);
	if (rspCode == ECL_LV1_FAIL)
	{
		NXP_Read_Register(0x0A, &regData);	//Error
		if (regData == 0x00)
		{
			rspCode=NXP_Check_Register(0x0B, 0x20);	//Status: Crypto1On
			if (rspCode == ECL_LV1_SUCCESS)
			{
				return ECL_LV1_SUCCESS;
			}
		}
	}

	NXP_Write_Register(0x00, 0x00);	//Terminate Command

	return ECL_LV1_FAIL;
}


UCHAR NXP_READ(UCHAR iptAddress, UCHAR *optData)
{
	UCHAR rspCode=0;
	UINT  datLen=0;

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_5MS));

	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.

	NXP_Write_Register(0x2C, 0x19);	//TxCrcPreset: TXPresetVal 0x6363, CRC16 ,TxCRCEn
	NXP_Write_Register(0x2D, 0x19);	//RxCrcCon: RXPresetVal 0x6363, CRC16 ,RxCrcEn

	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, 0x30);	//Read Command
	NXP_Write_Register(0x05, iptAddress);

	NXP_Write_Register(0x00, 0x07);	//Transceive

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	
	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;

	NXP_Get_FIFO_Length(&datLen);
	if (datLen != 16)
		return ECL_LV1_FAIL;

	NXP_Get_FIFO_Data(datLen, optData);

	return ECL_LV1_SUCCESS;
}


UCHAR NXP_WRITE(UCHAR iptAddress, UCHAR *iptData)
{
	UCHAR cntIdx=0;
	UCHAR rspCode=0;
	UINT  datLen=0;
	UCHAR datBuffer[1];
	
	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_5MS));

	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.

	NXP_Write_Register(0x2C, 0x19);	//Disable RX CRC and enable TX CRC
	NXP_Write_Register(0x2D, 0x18);

	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, 0xA0);	//Write Command
	NXP_Write_Register(0x05, iptAddress);

	NXP_Write_Register(0x00, 0x07);	//Transceive

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	
	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;

	NXP_Get_FIFO_Length(&datLen);
	
	if (datLen != 1)
		return ECL_LV1_FAIL;

	NXP_Get_FIFO_Data(datLen, datBuffer);

	rspCode=NXP_Check_ACK(datBuffer[0]);
	if (rspCode == ECL_LV1_FAIL)
		return ECL_LV1_FAIL;

	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.

	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_10MS));

	for (cntIdx=0; cntIdx < 16; cntIdx++)
		NXP_Write_Register(0x05, *iptData++);

	NXP_Write_Register(0x00, 0x07);	//Transceive

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	
	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;
	
	NXP_Get_FIFO_Length(&datLen);
	
	if (datLen != 1)
		return ECL_LV1_FAIL;

	NXP_Get_FIFO_Data(datLen, datBuffer);

	rspCode=NXP_Check_ACK(datBuffer[0]);
	if (rspCode == ECL_LV1_FAIL)
		return ECL_LV1_FAIL;
	
	return ECL_LV1_SUCCESS;
}


UCHAR NXP_DECREMENT(UCHAR iptAddress, UCHAR *iptValue)
{
	UCHAR rspCode=0;
	UINT  datLen=0;
	UCHAR datBuffer[1];
	
	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_5MS));

	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.

	NXP_Write_Register(0x2C, 0x19);	//Disable RX CRC and enable TX CRC
	NXP_Write_Register(0x2D, 0x18);
	
	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, 0xC0);	//Decrement Command
	NXP_Write_Register(0x05, iptAddress);

	NXP_Write_Register(0x00, 0x07);	//Transceive

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	
	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;

	NXP_Get_FIFO_Length(&datLen);
	
	if (datLen != 1)
		return ECL_LV1_FAIL;

	NXP_Get_FIFO_Data(datLen, datBuffer);

	rspCode=NXP_Check_ACK(datBuffer[0]);
	if (rspCode == ECL_LV1_FAIL)
		return ECL_LV1_FAIL;
	
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_5MS));
	
	NXP_Write_Register(0x05, iptValue[0]);
	NXP_Write_Register(0x05, iptValue[1]);
	NXP_Write_Register(0x05, iptValue[2]);
	NXP_Write_Register(0x05, iptValue[3]);

	NXP_Write_Register(0x00, 0x07);	//Transceive

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	//Timeout Means SUCCESS for this Command
	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_TIMEOUT_ISO)
		return ECL_LV1_FAIL;

	return ECL_LV1_SUCCESS;
}


UCHAR NXP_INCREMENT(UCHAR iptAddress, UCHAR *iptValue)
{
	UCHAR rspCode=0;
	UINT  datLen=0;
	UCHAR datBuffer[1];
	
	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_5MS));

	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.

	NXP_Write_Register(0x2C, 0x19);	//Disable RX CRC and enable TX CRC
	NXP_Write_Register(0x2D, 0x18);
	
	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, 0xC1);	//Increment Command
	NXP_Write_Register(0x05, iptAddress);

	NXP_Write_Register(0x00, 0x07);	//Transceive

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	
	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;

	NXP_Get_FIFO_Length(&datLen);
	
	if (datLen != 1)
		return ECL_LV1_FAIL;

	NXP_Get_FIFO_Data(datLen, datBuffer);

	rspCode=NXP_Check_ACK(datBuffer[0]);
	if (rspCode == ECL_LV1_FAIL)
		return ECL_LV1_FAIL;
	
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_5MS));
	
	NXP_Write_Register(0x05, iptValue[0]);
	NXP_Write_Register(0x05, iptValue[1]);
	NXP_Write_Register(0x05, iptValue[2]);
	NXP_Write_Register(0x05, iptValue[3]);

	NXP_Write_Register(0x00, 0x07);	//Transceive

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	//Timeout Means SUCCESS for this Command
	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_TIMEOUT_ISO)
		return ECL_LV1_FAIL;

	return ECL_LV1_SUCCESS;
}


UCHAR NXP_RESTORE(UCHAR iptAddress)
{
	UCHAR rspCode=0;
	UINT  datLen=0;
	UCHAR datBuffer[1];	
	
	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_5MS));

	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.

	NXP_Write_Register(0x2C, 0x19);	//Disable RX CRC and enable TX CRC
	NXP_Write_Register(0x2D, 0x18);
	
	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, 0xC2);	//Restore Command
	NXP_Write_Register(0x05, iptAddress);

	NXP_Write_Register(0x00, 0x07);	//Transceive
	
	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	
	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;
	
	NXP_Get_FIFO_Length(&datLen);
	
	if (datLen != 1)
		return ECL_LV1_FAIL;

	NXP_Get_FIFO_Data(datLen, datBuffer);

	rspCode=NXP_Check_ACK(datBuffer[0]);
	if (rspCode == ECL_LV1_FAIL)
		return ECL_LV1_FAIL;
	
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_5MS));
	
	NXP_Write_Register(0x05, 0x00);
	NXP_Write_Register(0x05, 0x00);
	NXP_Write_Register(0x05, 0x00);
	NXP_Write_Register(0x05, 0x00);

	NXP_Write_Register(0x00, 0x07);	//Transceive

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);

	//Timeout Means SUCCESS for this Command
	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_TIMEOUT_ISO)
		return ECL_LV1_FAIL;

	return ECL_LV1_SUCCESS;
}


UCHAR NXP_TRANSFER(UCHAR iptAddress)
{
	UCHAR rspCode=0;
	UINT  datLen=0;
	UCHAR datBuffer[1];

	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_10MS));
	
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.

	NXP_Write_Register(0x2C, 0x19);	//Disable RX CRC and enable TX CRC
	NXP_Write_Register(0x2D, 0x18);
	
	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, 0xB0);	//Transfer Command
	NXP_Write_Register(0x05, iptAddress);

	NXP_Write_Register(0x00, 0x07);	//Transceive

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	
	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;

	NXP_Get_FIFO_Length(&datLen);
	
	if (datLen != 1)
		return ECL_LV1_FAIL;

	NXP_Get_FIFO_Data(datLen, datBuffer);

	rspCode=NXP_Check_ACK(datBuffer[0]);
	if (rspCode == ECL_LV1_FAIL)
		return ECL_LV1_FAIL;

	return ECL_LV1_SUCCESS;
}


UCHAR NXP_AV2_AUTHENTICATION_1ST(UCHAR iptAutType, UCHAR iptAddress, UCHAR *optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UINT	datLen=0;
	
	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_5MS));
	
	if ((iptAutType != 0x60) && (iptAutType != 0x61))
		return ECL_LV1_FAIL;

	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	NXP_Write_Register(0x2C, 0x19);	//TxCrcPreset: TXPresetVal 0x6363, CRC16 ,CRC Enable
	NXP_Write_Register(0x2D, 0x18);	//RxCrcCon: RXPresetVal 0x6363, CRC16 ,CRC Disable
	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent, Tx 8 bits
	NXP_Write_Register(0x33, 0xCF);	//Enable Tx & Rx Parity
	NXP_Write_Register(0x0C, 0x00);	//Receive whole Byte

	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, iptAutType);
	NXP_Write_Register(0x05, iptAddress);

	NXP_Write_Register(0x00, 0x07);	//Transceive

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	
	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;
	
	NXP_Get_FIFO_Length(&datLen);
	if (datLen != 4)
		return ECL_LV1_FAIL;
	
	NXP_Get_FIFO_Data(datLen, optData);

	return ECL_LV1_SUCCESS;
}


UCHAR NXP_AV2_AUTHENTICATION_2ND(UCHAR *iptData, UCHAR *optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UINT	datLen=0;
	
	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_5MS));
	
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	NXP_Write_Register(0x2C, 0x18);	//TxCrcPreset: TXPresetVal 0x6363, CRC16 ,CRC Disable
	NXP_Write_Register(0x2D, 0x18);	//RxCrcCon: RXPresetVal 0x6363, CRC16 ,CRC Disable
	NXP_Write_Register(0x2E, 0x08);	//TxDataNum: Data is sent, Tx 8 bits
	NXP_Write_Register(0x33, 0x0F);	//Disable Tx & Rx Parity

	NXP_Clear_IRQFlag();

	NXP_Write_Register(0x05, iptData[0]);
	NXP_Write_Register(0x05, iptData[1]);
	NXP_Write_Register(0x05, iptData[2]);
	NXP_Write_Register(0x05, iptData[3]);
	NXP_Write_Register(0x05, iptData[4]);
	NXP_Write_Register(0x05, iptData[5]);
	NXP_Write_Register(0x05, iptData[6]);
	NXP_Write_Register(0x05, iptData[7]);
	NXP_Write_Register(0x05, iptData[8]);

	NXP_Write_Register(0x00, 0x07);	//Transceive

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	
	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;
	
	NXP_Get_FIFO_Length(&datLen);
	if (datLen != 5)
		return ECL_LV1_FAIL;
	
	NXP_Get_FIFO_Data(datLen, optData);

	return ECL_LV1_SUCCESS;
}


UCHAR NXP_AV2_TRANSCEIVE(UINT iptLen, UCHAR *iptData, UINT *optLen, UCHAR *optData)
{
	UCHAR	rspCode=ECL_LV1_FAIL;
	UCHAR	cntIndex=0;
	UCHAR	txnLasBit=0;

	if (iptLen > 512)	//Max. FIFO size
		return ECL_LV1_FAIL;

	txnLasBit=8-(iptLen*8%9);
	
	NXP_Write_Register(0x0E, 0x01);	//Stop Timer0

	NXP_Set_Timer((NXP_PROCESSING_TIME_A+NXP_MIFARE_TIMEOUT_10MS));
	
	NXP_Write_Register(0x02, 0x10);	//FIFO size: 512 bytes, Flush FIFO.
	NXP_Write_Register(0x2C, 0x18);	//TxCrcPreset: TXPresetVal 0x6363, CRC16 ,CRC Disable
	NXP_Write_Register(0x2D, 0x18);	//RxCrcCon: RXPresetVal 0x6363, CRC16 ,CRC Disable
	NXP_Write_Register(0x2E, (0x08 | txnLasBit));	//TxDataNum: Data is sent, Tx Last ? bits
	NXP_Write_Register(0x33, 0x0F);	//Disable Tx & Rx Parity

	NXP_Clear_IRQFlag();

	for (cntIndex=0; cntIndex < iptLen; cntIndex++)
		NXP_Write_Register(0x05, iptData[cntIndex]);

	NXP_Write_Register(0x00, 0x07);	//Transceive

	NXP_Set_IRQ(NXP_IRQ0_IDLE|NXP_IRQ0_TX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	NXP_Wait_GlobelIRQ();

	NXP_Set_IRQ(NXP_IRQ0_DISABLE, NXP_IRQ1_DISABLE);

	NXP_Set_IRQ(NXP_IRQ0_RX, NXP_IRQ1_GLOBAL|NXP_IRQ1_TIMER0);
	
	rspCode=NXP_Receive();
	if (rspCode != ECL_LV1_SUCCESS)
		return rspCode;
	
	NXP_Get_FIFO(optLen, optData);

	return ECL_LV1_SUCCESS;
}
