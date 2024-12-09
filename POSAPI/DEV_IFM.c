

#include "POSAPI.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h> // define O_RDWR , open ....
#include <linux/i2c.h> 
#include <linux/i2c-dev.h> 
#include <sys/ioctl.h> 

#include <unistd.h> // define close

#include "DEV_IFM.h"
#include "ExtIODev.h"

#include "API_EXTIO.h"


// because we can't read the value of the write register of bank1 
UCHAR g_bank1_write_register0_value[6] = {0};


/**
 *   This function is I2C to write data to device
 *   @param[in] device_addr         device address (make sure to follow the I2C spec)
 *   @param[in] data                data
 *   @retval TRUE                   write success
 *   @retval FALSE                  write fail
 */
UCHAR I2CWrite(UCHAR device_addr, UCHAR data){

    int fd, ret;
    struct i2c_rdwr_ioctl_data tda8086_data;

    if((fd = open("/dev/i2c-0", O_RDWR)) < 0){
        perror("open");
        return FALSE;
    }
    // 
    tda8086_data.msgs = (struct i2c_msg *)malloc(1 * sizeof(struct i2c_msg));
    if(!tda8086_data.msgs){
        perror("malloc1");
        close(fd);
        return FALSE;
    }
    (tda8086_data.msgs[0]).buf = (UCHAR*)malloc(sizeof(UCHAR));
    if(!(tda8086_data.msgs[0]).buf){
        perror("malloc2");
        close(fd);
        return FALSE;
    }
    /*
        usually in I2c will specify the I2C address(chip address), then spcify chip register address.
        But in tda8086, there is no chip address. The address in I2C bus is exatly register address.
        We just need one address.

    */
    tda8086_data.nmsgs = 1;
    (tda8086_data.msgs[0]).len = 1; // try one only data 
    (tda8086_data.msgs[0]).addr = device_addr;
    (tda8086_data.msgs[0]).flags = 0; // write
    (tda8086_data.msgs[0]).buf[0] = data;

    ret=ioctl(fd,I2C_RDWR,(unsigned long)&tda8086_data);
    if(ret<0)
    {
        perror("ioctl");
        close(fd);
            return FALSE;
    }

    close(fd);
    free((tda8086_data.msgs[0]).buf);
    free(tda8086_data.msgs);
    return TRUE;

}

/**
 *    This fuction is I2C to read data from device
 *    @param[in] device_addr        device address (make sure to follow the I2C spec)
 *    @param[out] data              data you read from device
 *    @retval TRUE                  read sucess
 *    @retval FALSE                 read fail
 */
UCHAR I2CRead(UCHAR device_addr, UCHAR* data){

    int fd, ret;
    struct i2c_rdwr_ioctl_data tda8086_data;

    if((fd = open("/dev/i2c-0", O_RDWR)) < 0){
        perror("open");
        return FALSE;
    }
    // 
    tda8086_data.msgs = (struct i2c_msg *)malloc(1 * sizeof(struct i2c_msg));
    if(!tda8086_data.msgs){
        perror("malloc1");
        close(fd);
        return FALSE;
    }
    (tda8086_data.msgs[0]).buf = (UCHAR*)malloc(sizeof(UCHAR));
    if(!(tda8086_data.msgs[0]).buf){
        perror("malloc2");
        close(fd);
        return FALSE;
    }
    /*
        usually in I2c will specify the I2C address(chip address), then spcify chip register address.
        But in tda8086, there is no chip address. The address in I2C bus is exatly register address.
        We just need one address.

    */

    tda8086_data.nmsgs = 1;
    (tda8086_data.msgs[0]).len = 1; // try one only data 
    (tda8086_data.msgs[0]).addr = device_addr;
    (tda8086_data.msgs[0]).flags = I2C_M_RD; // read
    (tda8086_data.msgs[0]).buf[0] = 0;

    ret=ioctl(fd,I2C_RDWR,(unsigned long)&tda8086_data);
    if(ret<0)
    {
        perror("ioctl");
        close(fd);
        return FALSE;
    }

    *(data) = (tda8086_data.msgs[0]).buf[0]; // copy to output
    close(fd);
    free((tda8086_data.msgs[0]).buf);
    free(tda8086_data.msgs);
    return TRUE;

}


/**
 *     This function is used in select differnt card slow in TDA8026
 *     @param[in] sam_id         card id (we only support 1-5)
 *                               ICC1   1
 *                               SAM1   2
 *                               SAM2   3
 *                               SAM3   4
 *                               SAM4   5
 *     @retval TRUE              select success
 *     @retval FALSE             select fail
 */
UCHAR card_slots_selection(UCHAR sam_id){

    if(sam_id <= 5 && sam_id >= 1)
        I2CWrite(TDA8026_BANK0_REGISTER_ADDRESS, sam_id);
    else
        return FALSE;
    
    return TRUE;

}
/**
 *    This function is used to check the card active or not by TDA8026
 *    @param[in] sam_id         card id (we only support 1-5)
 *                              ICC1   1
 *                              SAM1   2
 *                              SAM2   3
 *                              SAM3   4
 *                              SAM4   5
 *    @return  TRUE if the card is activated, FALSE if not
 */
UCHAR Check_Active_SAM(UCHAR sam_id){

    card_slots_selection(sam_id);
    UCHAR data;
    I2CRead(TDA8026_BANK1_REGISTER0_ADDRESS, &data);

    if(data & ACTIVE_BIT_POSITION_SAM12)
        return TRUE;
    else
        return FALSE;
}
/**
 *      This function is used to check the card present by TDA8026
 *      @param[in] sam_id       card id (we only support 1-5)
 *                              ICC1   1
 *                              SAM1   2
 *                              SAM2   3
 *                              SAM3   4
 *                              SAM4   5
 *      @return TRUE if the card is presented, FALSE if not
 *      @note The result only legal for ICC1 (sam id = 1). other sam id always return TRUE
 */
UCHAR Check_Present_SAM(UCHAR sam_id)
{
    card_slots_selection(sam_id);
    UCHAR data;
    I2CRead(TDA8026_BANK1_REGISTER0_ADDRESS, &data);
    //printf("data:%2x\n",data);

    if(data & PRES_BIT_POSITION_SAM12)
        return TRUE;
    else
        return FALSE;
}

/**
 *      This function is used to read the status of the card by TDA8026
 *      @param[in] sam_id       card id (we only support 1-5)
 *                              ICC1   1
 *                              SAM1   2
 *                              SAM2   3
 *                              SAM3   4
 *                              SAM4   5
 *      @return  Status value return by TDA8026
 */
UCHAR Read_Status_Data_SAM(UCHAR sam_id){

    card_slots_selection(sam_id);

    UCHAR data;
    I2CRead(TDA8026_BANK1_REGISTER0_ADDRESS, &data);

    return data;
}

/**
 *      This fuction is used to disable the IO of the card by TDA8026
 *      @param[in] sam_id       card id (we only support 1-5)
 *                              ICC1   1
 *                              SAM1   2
 *                              SAM2   3
 *                              SAM3   4
 *                              SAM4   5
 *      @retval TRUE                disable IO success
 *      @retval FALSE               disable IO fail
 */
UCHAR Disable_IO_SAM(UCHAR sam_id){

    card_slots_selection(sam_id);
    
    g_bank1_write_register0_value[sam_id]  &= ~IOEN_BIT_POSITION_SAM12;

    return(I2CWrite(TDA8026_BANK1_REGISTER0_ADDRESS, g_bank1_write_register0_value[sam_id]));
}

/**
 *      This fuction is used to enable the IO of the card by TDA8026
 *      @param[in] sam_id       card id (we only support 1-5)
 *                              ICC1   1
 *                              SAM1   2
 *                              SAM2   3
 *                              SAM3   4
 *                              SAM4   5
 *      @retval TRUE                enable IO success
 *      @retval FALSE               enable IO fail
 */
UCHAR Enable_IO_SAM(UCHAR sam_id){


    // make sure diable other before Enable IO;

    int i;
    for(i = 1; i <= 5; i++)
        Disable_IO_SAM(i);

    card_slots_selection(sam_id);
    
    g_bank1_write_register0_value[sam_id] |= IOEN_BIT_POSITION_SAM12;

    return(I2CWrite(TDA8026_BANK1_REGISTER0_ADDRESS, g_bank1_write_register0_value[sam_id]));
}


/**
 *      This fuction is used to pefrom cold reset to the card by TDA8026
 *      @param[in] sam_id       card id (we only support 1-5)
 *                              ICC1   1
 *                              SAM1   2
 *                              SAM2   3
 *                              SAM3   4
 *                              SAM4   5
 *      @retval TRUE                cold reset success
 *      @retval FALSE               cold reset fail
 */
UCHAR Start_SAM(UCHAR sam_id){

    card_slots_selection(sam_id);

    g_bank1_write_register0_value[sam_id]  |=  START_BIT_POSITION_SAM12;

    return(I2CWrite(TDA8026_BANK1_REGISTER0_ADDRESS, g_bank1_write_register0_value[sam_id]));
}

/**
 *      This function is used to set voltage to the card by TDA8026
 *      @param[in] sam_id       card id (we only support 1-5)
 *                              ICC1   1
 *                              SAM1   2
 *                              SAM2   3
 *                              SAM3   4
 *                              SAM4   5
 *      @param[in] voltage      should be macro SC_PHY_1_8V (1.8V)
 *                                              SC_PHY_5V (5V)
 *                                              SC_PHY_3V (3V)
 *      @retval TRUE            set voltage success
 *      @retval FALSE           set voltage fail
 */
UCHAR Set_Voltage_SAM(UCHAR sam_id, UCHAR voltage){

    card_slots_selection(sam_id);

    g_bank1_write_register0_value[sam_id] &= ~(VN3OR5_BIT_POSITION_SAM12 | VCC1V8_BIT_POSITION_SAM12);

    switch (voltage) {

	case SC_PHY_1_8V:

		g_bank1_write_register0_value[sam_id] |= VCC1V8_BIT_POSITION_SAM12;

	break;

	case SC_PHY_5V:

		g_bank1_write_register0_value[sam_id] |= VN3OR5_BIT_POSITION_SAM12;

	break;

	case SC_PHY_3V:

	default:

	break;

	}

    return(I2CWrite(TDA8026_BANK1_REGISTER0_ADDRESS, g_bank1_write_register0_value[sam_id]));

}


/**
 *     This function is used to deactivate the card by TDA8026 
 *      @param[in] sam_id       card id (we only support 1-5)
 *                              ICC1   1
 *                              SAM1   2
 *                              SAM2   3
 *                              SAM3   4
 *                              SAM4   5
 *     @retval TRUE             deactivate succuss
 *     @retval FALSE            deactivate fail
 */
UCHAR Deactivate_SAM(UCHAR sam_id){

    card_slots_selection(sam_id);

    g_bank1_write_register0_value[sam_id]  &=  ~START_BIT_POSITION_SAM12;

    return(I2CWrite(TDA8026_BANK1_REGISTER0_ADDRESS, g_bank1_write_register0_value[sam_id]));

}

/**
 *    This function is used to select the band1 of different register for TDA8026
 *      @param[in] sam_id       card id (we only support 1-5)
 *                              ICC1   1
 *                              SAM1   2
 *                              SAM2   3
 *                              SAM3   4
 *                              SAM4   5
 *    @param[in] value          should be marcro BANK1_REGISTER1_00,BANK1_REGISTER1_01
 *                              BANK1_REGISTER1_10,BANK1_REGISTER1_11
 *    @retval TRUE              select bank success
 *    @retval FALSE             select bank fail
 */
UCHAR bank1_regiser1_selection_SAM(UCHAR sam_id,UCHAR value){

    card_slots_selection(sam_id);

    switch (value)
    {
    case BANK1_REGISTER1_00:
        g_bank1_write_register0_value[sam_id]  &= ~REGLOW_BIT_POSTION_SAM12;
        g_bank1_write_register0_value[sam_id]  &= ~REGHIGH_BIT_POSITION_SAM12;
        break;
    case BANK1_REGISTER1_01:
        g_bank1_write_register0_value[sam_id]  |= REGLOW_BIT_POSTION_SAM12;
        g_bank1_write_register0_value[sam_id]  &= ~REGHIGH_BIT_POSITION_SAM12;
        break;
    case BANK1_REGISTER1_10:
        g_bank1_write_register0_value[sam_id]  &= ~REGLOW_BIT_POSTION_SAM12;
        g_bank1_write_register0_value[sam_id]  |= REGHIGH_BIT_POSITION_SAM12;
        break;
    case BANK1_REGISTER1_11:
        g_bank1_write_register0_value[sam_id]  |= REGLOW_BIT_POSTION_SAM12;
        g_bank1_write_register0_value[sam_id]  |= REGHIGH_BIT_POSITION_SAM12;
        break;
    default:
        return FALSE;
    }

    return(I2CWrite(TDA8026_BANK1_REGISTER0_ADDRESS, g_bank1_write_register0_value[sam_id]));

}


/**
 *      This function is to perform warm reset to the card by TDA8026
 *      @param[in] sam_id       card id (we only support 1-5)
 *                              ICC1   1
 *                              SAM1   2
 *                              SAM2   3
 *                              SAM3   4
 *                              SAM4   5
 *      @retval TRUE            warm reset success
 *      @retval FALSE           warm reset fail
 */
UCHAR WARM_RESET(UCHAR sam_id){

    card_slots_selection(sam_id);
    
    g_bank1_write_register0_value[sam_id] |= WARM_BIT_POSITION_SAM12;

    return(I2CWrite(TDA8026_BANK1_REGISTER0_ADDRESS, g_bank1_write_register0_value[sam_id]));

}
/**
 *  This function is to select synchronous or asynchronous mode of TDA8026
 *  @param[in] sam_id       card id (we only support 1-5)
 *                              ICC1   1
 *                              SAM1   2
 *                              SAM2   3
 *                              SAM3   4
 *                              SAM4   5
 *  @param[in] mode             macro MODE_ASYNCHRONOUS or MODE_SYNCHRONOUS
 *  @retval TRUE                select mode success
 *  @retval FALSE               select mode fail
 *  @note  we can only use asynchronous mode
 */
UCHAR Syn_Asyn_Selcetion_SAM12(UCHAR sam_id, UCHAR mode){

    UCHAR data,result;

    bank1_regiser1_selection_SAM(sam_id, BANK1_REGISTER1_00);

    result = I2CRead(TDA8026_BANK1_REGISTER1_ADDRESS,&data);
    if(result == FALSE)
        return FALSE;

    if(mode == MODE_ASYNCHRONOUS)
        data |= RSTIN_BIT_POSITION;
    else if(mode == MODE_SYNCHRONOUS)
        data &= ~RSTIN_BIT_POSITION;
    else
        return FALSE;

    return (I2CWrite(TDA8026_BANK1_REGISTER1_ADDRESS,data));

}

/**
 *  This function is used to change the card frequency by TDA8026
 *  @param[in] sam_id       card id (we only support 1-5)
 *                              ICC1   1
 *                              SAM1   2
 *                              SAM2   3
 *                              SAM3   4
 *                              SAM4   5
 *  @param[in] frequency_mode   macro EXT_FREQUENCY , externel frequency
 *                                    EXT_FREQUENCY_DEVIDE_2, externel frequency/2
 *                                    EXT_FREQUENCY_DEVIDE_4, externel frequency/4
 *                                    EXT_FREQUENCY_DEVIDE_5, externel frequency/5
 * @retval TRUE                 change frequency successful
 * @retval FALSE                change frequency fail
 */
UCHAR Change_frequency(UCHAR sam_id, UCHAR frequency_mode){

    UCHAR data,result;
    bank1_regiser1_selection_SAM(sam_id, BANK1_REGISTER1_00);

    result = I2CRead(TDA8026_BANK1_REGISTER1_ADDRESS,&data);
    if(result == FALSE)
        return FALSE;
    
    switch (frequency_mode)
    {
    case EXT_FREQUENCY:
        data &= ~CLKDIVLOW_BIT_POSITION;
        data &= ~CLKDIVHIGH_BIT_POSITION;
        break;
    case EXT_FREQUENCY_DEVIDE_2:
        data |= CLKDIVLOW_BIT_POSITION;
        data &= ~CLKDIVHIGH_BIT_POSITION;
        break;
    case EXT_FREQUENCY_DEVIDE_4:
        data &= ~CLKDIVLOW_BIT_POSITION;
        data |= CLKDIVHIGH_BIT_POSITION;
        break;
    
    case EXT_FREQUENCY_DEVIDE_5:
        data |= CLKDIVLOW_BIT_POSITION;
        data |= CLKDIVHIGH_BIT_POSITION;
        break;
    default:
        return FALSE;
    }

    return (I2CWrite(TDA8026_BANK1_REGISTER1_ADDRESS,data));

}



/** 
 * This function is to set the extention IO SAM_FAST_FIND to 1
 */
void FAST_FIND_ENABLE(){
    EXTIO_GPIO(_SAM_FAST_FIND_BANK_ADDR,_SAM_FAST_FIND_GPIO_PORT,_SAM_FAST_FIND_GPIO_NUM,1);
}
/** 
 * This function is to set the extention IO SAM_FAST_FIND to 0
 */
void FAST_FIND_DISABLE(){
    EXTIO_GPIO(_SAM_FAST_FIND_BANK_ADDR,_SAM_FAST_FIND_GPIO_PORT,_SAM_FAST_FIND_GPIO_NUM,0);
}
/** 
 * This function is to set the extention IO TDA_DCDC_OFF(P1.5) 
 * 
 * @param[in] value             0/1 (0 mean set to 0)
 *                                  (1 mean set to 1)
 */
void TDA_DC_TO_DC(UCHAR value){
    EXTIO_GPIO(_TDA_DCDC_OFF_BANK_ADDR,_TDA_DCDC_OFF_GPIO_PORT,_TDA_DCDC_OFF_GPIO_NUM,value);
}