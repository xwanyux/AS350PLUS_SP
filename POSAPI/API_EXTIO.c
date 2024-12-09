



#include "POSAPI.h"

#include <stdio.h>
#include <stdlib.h>
#include "ExtIODev.h"
#include <string.h>
#include "bsp_gpio.h"



//enable extension IO hardware chip select mode and enter byte mode
UCHAR EXTIO_IOINIT()
{
UCHAR *tx=malloc(sizeof(UCHAR)*4);
UCHAR *rx=malloc(sizeof(tx));
    
	*tx=0x40;
	*(tx+1)=0x0B;//IOCON
	*(tx+2)=0xA8;//BANK=1,SEQOP=1,HAEN=1
	SPI_Transfer(tx,rx,0,EXTIO,4);    
    free(tx);
	free(rx);
}
/**
 *      This function is to output value to extention gpio
 *      @param[in] BANK_ADDR               extention bank address
 *      @param[in] GPIO_PORT               extention gpio port 0//1
 *      @param[in] GPIO_NUM                extention gpio number
 *      @param[in] output                  value you want to set (0.1)
 *      @retval apiFailed                  set extention IO value fail
 *      @retval apiOK                      set extention IO value success
 */
UCHAR EXTIO_ENABLE_INTERRUPT(UCHAR BANK_ADDR, UCHAR GPIO_PORT, UCHAR GPIO_NUM, UCHAR output){


    UCHAR *tx=malloc(sizeof(UCHAR)*3);
	UCHAR *rx=malloc(sizeof(tx));

    // [0010 [BANKADDR] [W/R]]
    UCHAR ReadCommad = 0x40 + 0x1 + BANK_ADDR * 2;  // Read 1
    UCHAR WriteCommand = 0x40 + 0x0 + BANK_ADDR * 2; // Wrrtie 0


    UCHAR IODIR_Register = 0;
    UCHAR GPIO_Register = 0;

    UCHAR MASK = 1 << GPIO_NUM;

    if(GPIO_PORT == 0){
        IODIR_Register = 0x00;
        GPIO_Register = 0x09;
    }
    else if(GPIO_PORT == 1){
        IODIR_Register = 0x10;
        GPIO_Register = 0x19;
    }
    else
        return apiFailed;


	// This part is fix, just make sure we do the initial setting.
	//enable extension IO hardware chip select mode 
	*tx=0x40;
	*(tx+1)=0x0B;
	*(tx+2)=0xA8;
    SPI_Transfer(tx,rx,0,EXTIO,4);

    *tx = ReadCommad;//read register
	*(tx+1) = IODIR_Register;
	*(tx+2) = 0x00; // just for retrieve data
    SPI_Transfer(tx,rx,0,EXTIO,4);

    *(tx+2) = *(rx+2);	
	*(tx+2) &= ~MASK;// pin P1.6 
	*tx=WriteCommand;//set register
    SPI_Transfer(tx,rx,0,EXTIO,4);


	*tx = ReadCommad;
	*(tx+1) = GPIO_Register;
	*(tx+2) = 0x00;
    SPI_Transfer(tx,rx,0,EXTIO,4);

	*(tx+2)=*(rx+2);
    if(output == 1)	
	    *(tx+2) |= MASK;//   
    else if(output == 0)
        *(tx+2) &= ~MASK;
    else
        return apiFailed;
	*tx=WriteCommand;
    SPI_Transfer(tx,rx,0,EXTIO,4);

	free(tx);
	free(rx);

    return apiOK;


}
/**
 *      This function is to output value to extention gpio
 *      @param[in] BANK_ADDR               extention bank address
 *      @param[in] GPIO_PORT               extention gpio port 0//1
 *      @param[in] GPIO_NUM                extention gpio number
 *      @param[in] output                  value you want to set (0.1)
 *      @retval apiFailed                  set extention IO value fail
 *      @retval apiOK                      set extention IO value success
 */
UCHAR EXTIO_GPIO(UCHAR BANK_ADDR, UCHAR GPIO_PORT, UCHAR GPIO_NUM, UCHAR output){


    UCHAR *tx=malloc(sizeof(UCHAR)*3);
	UCHAR *rx=malloc(sizeof(tx));

    // [0010 [BANKADDR] [W/R]]
    UCHAR ReadCommad = 0x40 + 0x1 + BANK_ADDR * 2;  // Read 1
    UCHAR WriteCommand = 0x40 + 0x0 + BANK_ADDR * 2; // Wrrtie 0


    UCHAR IODIR_Register = 0;
    UCHAR GPIO_Register = 0;

    UCHAR MASK = 1 << GPIO_NUM;

    if(GPIO_PORT == 0){
        IODIR_Register = 0x00;
        GPIO_Register = 0x09;
    }
    else if(GPIO_PORT == 1){
        IODIR_Register = 0x10;
        GPIO_Register = 0x19;
    }
    else
        return apiFailed;


	// This part is fix, just make sure we do the initial setting.
	//enable extension IO hardware chip select mode 
	*tx=0x40;
	*(tx+1)=0x0B;
	*(tx+2)=0xA8;
    SPI_Transfer(tx,rx,0,EXTIO,4);

    *tx = ReadCommad;//read register
	*(tx+1) = IODIR_Register;
	*(tx+2) = 0x00; // just for retrieve data
    SPI_Transfer(tx,rx,0,EXTIO,4);

    *(tx+2) = *(rx+2);	
	*(tx+2) &= ~MASK;// pin P1.6 
	*tx=WriteCommand;//set register
    SPI_Transfer(tx,rx,0,EXTIO,4);


	*tx = ReadCommad;
	*(tx+1) = GPIO_Register;
	*(tx+2) = 0x00;
    SPI_Transfer(tx,rx,0,EXTIO,4);

	*(tx+2)=*(rx+2);
    if(output == 1)	
	    *(tx+2) |= MASK;//   
    else if(output == 0)
        *(tx+2) &= ~MASK;
    else
        return apiFailed;
	*tx=WriteCommand;
    SPI_Transfer(tx,rx,0,EXTIO,4);

	free(tx);
	free(rx);

    return apiOK;


}

/**
 *      20220729 added by west
 *      Function Name:EXTIO_GPIO_Read
 *      This function is to get pin status of specific extention gpio
 *      @param[in] BANK_ADDR               extention bank address
 *      @param[in] GPIO_PORT               extention gpio port 0//1
 *      @param[in] GPIO_NUM                extention gpio number
 *      @param[out] value                  value you want to set (0.1)
 *      @retval apiFailed                  get extention IO value fail
 *      @retval apiOK                      get extention IO value success
 */
UCHAR EXTIO_GPIO_Read(UCHAR BANK_ADDR, UCHAR GPIO_PORT, UCHAR GPIO_NUM, UCHAR *value){


    UCHAR *tx=malloc(sizeof(UCHAR)*3);
	UCHAR *rx=malloc(sizeof(tx));

    // [0010 [BANKADDR] [W/R]]
    UCHAR ReadCommad = 0x40 + 0x1 + BANK_ADDR * 2;  // Read 1
    UCHAR WriteCommand = 0x40 + 0x0 + BANK_ADDR * 2; // Wrrtie 0


    UCHAR IODIR_Register = 0;
    UCHAR GPIO_Register = 0;

    UCHAR MASK = 1 << GPIO_NUM;

    if(GPIO_PORT == 0){
        IODIR_Register = 0x00;
        GPIO_Register = 0x09;
    }
    else if(GPIO_PORT == 1){
        IODIR_Register = 0x10;
        GPIO_Register = 0x19;
    }
    else
        return apiFailed;

	*tx = ReadCommad;
	*(tx+1) = GPIO_Register;
	*(tx+2) = 0x00;
    SPI_Transfer(tx,rx,0,EXTIO,4);

    *value=*(rx+2)&(1 << GPIO_NUM);

	free(tx);
	free(rx);

    return apiOK;


}