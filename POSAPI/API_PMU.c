




#include "POSAPI.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ExtIODev.h"
#include <string.h>
#include "bsp_gpio.h"

#include "API_EXTIO.h"
#include "bsp_SHM.h"



void api_pmu_poweroff( void );
float PMU_CalculateVoltage( UINT32 AdcData, UCHAR *Percent );

extern ULONG   baud_rate;
extern void OS_EnableTimer1();
char* command_wakeup_last = " > /sys/class/rtc/rtc0/wakealarm";

char* command_wakeup_first = "echo +";

char* command_cleanup_wakeup = "echo 0 > /sys/class/rtc/rtc0/wakealarm";

ULONG backup_PowerSavingTime;
ULONG backup_BackLitTime;
ULONG global_counter_pmu_poweroff;
ULONG global_counter_pmu_backlite_off;
ULONG global_PowerOffTime;
UCHAR global_powerSaving_on = FALSE;
UCHAR global_Backlight_off_LCD = FALSE;

float	voltage2; // use for testing only 

/**
 *    This function is to check which power source in used(adapter or battery) 
 * 	  RETURN:
 * 	  apiOK                     power source is from adapter
 *    apiFailed                 power source is from battery  
 */
UCHAR PMU_PowerSourceState()
{
UCHAR *dSHM;
UCHAR data;
UCHAR result;
	bsp_shm_acquire(dSHM);
	bsp_shm_Adapter(0,&data);
	if(data)
		result=apiOK;
	else
		result=apiFailed;
	return result;
}
/**
 *    This function is to check if battery alive(battery removed or broken) 
 * 	  RETURN:
 * 	  apiOK                     battery alive
 *    apiFailed                 can't detect battery
 */
UCHAR PMU_BatteryAlive()
{
UCHAR *dSHM;
UCHAR data;
UCHAR result;
	bsp_shm_BatteryAlive(0,&data);
	if(data)
		result=apiOK;
	else
		result=apiFailed;
	return result;
}
/**
 *    This function is to check if battery alive(battery removed or broken) 
 * 	  RETURN:
 * 	  apiOK                     battery charging
 *    apiFailed                 battery charging completed
 */
UCHAR PMU_BatteryCharging()
{
UCHAR *dSHM;
UCHAR data;
UCHAR result;
	bsp_shm_BatteryCharging(0,&data);
	if(data)
		result=apiOK;
	else
		result=apiFailed;
	return result;
}
/**
 *    This function is used to set up three different power saving time
 * 
 *    @param[in] BackLitTime        after BackLitTime, the screen will turn off,
 *                                  0xFFFFFFFF mean never turn off the screen
 *    @param[in] PowerSavingTime    after PowerSavingTime, the system will enter sleep mode,
 *                                  0xFFFFFFFF mean never enter sleep mode
 *    @param[in] PowerOffTime       after PowerSavingTime + PowerOffTime, the system will be powered down
 *                                  0xFFFFFFFF mean never power down
 *    @note the actual implementation is by anther thread (10ms system timer), in this function we just update
 *          the flag and value. In addtional to the value 0xFFFFFFFF, BackLitTime < PowerSavingTime < PowerOffTime
 */
void	api_pmu_setup( ULONG BackLitTime, ULONG PowerSavingTime, ULONG PowerOffTime )
{
	if(PMU_PowerSourceState()==apiOK)
		return;
	OS_EnableTimer1();
    // update the global counter to enter PowerSavingTime
    // (conver to 10ms)
    global_counter_pmu_poweroff = PowerSavingTime * 100;
	global_counter_pmu_backlite_off = BackLitTime * 100;
    // update the PowerOFFTime
    global_PowerOffTime = PowerOffTime;

    backup_PowerSavingTime = PowerSavingTime;
	backup_BackLitTime  = BackLitTime;

	global_powerSaving_on = TRUE;
	global_Backlight_off_LCD = TRUE;

	if(BackLitTime == 0xFFFFFFFF)
		global_Backlight_off_LCD = FALSE;
	
	if(PowerSavingTime == 0xFFFFFFFF)
		global_powerSaving_on = FALSE;


}

/**
 *   This function is used to enter sleep mode and after PowerOffTime , the system will power down.
 * 
 *   @param[in] PowerOffTime        after PowerOffTime, the system will be powered down,
 *                                  0xFFFFFFFF mean never power down
 */   
void PowerSavingAndPowerOff(ULONG PowerOffTime){

    FILE *fp, *fp2; 
    UCHAR str_PowerOffTime[10]; // maximum 4294967295-1
    UCHAR len = 0;

    UCHAR wakeup_command[50] = {0};
    UCHAR len_wakeup_command = 0;
    UCHAR buffer[6] = {0}; // not sure why zero size buffer always successful fgets, so this use a small buffer
    UCHAR wakeup_flag = 0;

    int i;
    int result;
	

    if( PowerOffTime != 0xFFFFFFFF && PowerOffTime > 0){

        // make sure the wakeup already been clean up
        // or you can set the wake up time
        fp2 = popen(command_cleanup_wakeup, "r");
        pclose(fp2);

        // convert value to string
        while(PowerOffTime > 0){
            str_PowerOffTime[9-len] = '0' + PowerOffTime % 10;
            len += 1;
            PowerOffTime /= 10;
        }

        /*overall structure to setup RTC alarm*/
        /*"echo +4294967295 > /sys/class/rtc/rtc0/wakealarm" */
        memmove(wakeup_command, command_wakeup_first, strlen(command_wakeup_first));
        len_wakeup_command += strlen(command_wakeup_first);
        memmove(&wakeup_command[len_wakeup_command], &str_PowerOffTime[10 - len], len);
        len_wakeup_command += len;
        memmove(&wakeup_command[len_wakeup_command],command_wakeup_last,strlen(command_wakeup_last));

        printf("%s\n",wakeup_command);
        fp2 = popen(wakeup_command, "r");
        pclose(fp2);

    }

    // go into deep sleep mode
    fp=popen("echo mem > /sys/power/state","r");
    pclose(fp);

    //once alarm happend, you should read fail
    fp = popen("cat /sys/class/rtc/rtc0/wakealarm", "r");
    if(fp == NULL)
        printf("open fail\n");
    else
        printf("open secc\n");

    //result = fgets(buffer, 6,fp);
    if(fgets(buffer, 6,fp) != NULL){
        printf("read succ\n");
        wakeup_flag = 0;
        global_counter_pmu_poweroff = backup_PowerSavingTime * 100;
		global_counter_pmu_backlite_off = backup_BackLitTime * 100;
		// open the lcd
		api_sys_backlight(0, 0xFFFFFFFF);
    }
    else{
        printf("read fail\n");
        wakeup_flag = 1;
    }
    pclose(fp);

    if(wakeup_flag == 1 && PowerOffTime != 0xFFFFFFFF ){
        printf("enter power of\n");
        api_pmu_poweroff();
    }
    
    printf("exiting main process ----\n");
    

}

// ---------------------------------------------------------------------------
// FUNCTION: get the battery power status.
// INPUT   : none.
// OUTPUT  : VoltValue  - converted value read from the ADC fifo.
//           StatValue  - converted value read from the ADC fifo.
//	                  ADC : 0..1023
//			  GPIO: 0 or 1
//	     Vol        - real analog voltage value.
//	     Percent	- percentage of the energy volume.
// RETURN  : 
//		 0x00 - battery power is GOOD. 80%~99%
//	     0x01 - battery power is POOR. 0%~39%
//	     0x02 - battery power is FAIR. 40%~79%
//	     0x80 - DC power here and battery power is GOOD without charging.	
//	     0x82 - DC power here and battery power is UNKNOW without charging. - BATT may be not connected or bad
//	     0x81 - DC power here and battery power is POOR and in charging.	0%~39%
//	     0x83 - DC power here and battery power is FAIR and in charging.	40%~79%
//	     0x84 - DC power here and battery power is GOOD and in chargeing.	80%~99%
// ---------------------------------------------------------------------------
/**
 *  This function is use to get the battery energy left for the system.
 * 
 *  @param[out] BattEnergyPercent     number of battery engery left (in percentage) 
 *  @retval apiOK                     get the battery energy left success
 *  @retval apiFailed                 get the battery energy left fail
 */ 
UCHAR api_pmu_status( UCHAR *BattEnergyPercent ){

    FILE * fp;
    char buffer[6];
    UINT value = 0;
    int i;
	UCHAR result;
	//if power source is adapter
	
	
    fp=popen("cat /sys/bus/iio/devices/iio:device0/in_voltage5_raw","r");
    if(fp == NULL)
        return apiFailed;
    fgets(buffer, 5,fp);
    printf("%s\n",buffer);
    pclose(fp);

    for(i = 0; i < 6 ; i++){
        if(buffer[i] == 0)
            break;
        value *= 10;
        value += buffer[i] - '0';
    }
    //printf("%s\n",buffer);
    PMU_CalculateVoltage(value,BattEnergyPercent);
	if(PMU_PowerSourceState()==apiOK)
	{
		if(PMU_BatteryAlive()==apiFailed)
			result=0x82;
		if(PMU_BatteryCharging)
		{
			if(BattEnergyPercent>79)
				result=0x84;
			else if(BattEnergyPercent>39)
				result=0x83;
			else
				result=0x81;
		}	
		else
			result=0x80;
	}
	else
	{
		if(BattEnergyPercent>79)
			result= 0x00;
		else if(BattEnergyPercent>39)
			result=0x02;
		else
			result=0x01;
	}
    return result;
}



/**
 *   This fuction is used to write the extention battery pin to 0. (mean power off)
 */ 
void api_pmu_poweroff( void ){
	if(PMU_PowerSourceState()==apiOK)
		return; 
	EXTIO_GPIO(_VBATT_8V_SW_BANK_ADDR,_VBATT_8V_SW_GPIO_PORT,_VBATT_8V_SW_GPIO_NUM,0);
}


/**
 *   This fuction is used to write the extention battery pin to 1. (mean power on)
 */ 
void api_pmu_poweron( void ){
	if(PMU_PowerSourceState()==apiOK)
		return; 
	EXTIO_GPIO(_VBATT_8V_SW_BANK_ADDR,_VBATT_8V_SW_GPIO_PORT,_VBATT_8V_SW_GPIO_NUM,1);
}

/**
 *    This function is to calculate the voltage from the adc value.
 *    @param[in] AdcData        adc data
 *    @param[out] Percent       the battery power left in percentage
 *    @return   the adc value tranfom to voltage
 */  
float PMU_CalculateVoltage( UINT32 AdcData, UCHAR *Percent )
{
float	voltage;
float	f_AdcData;
float	f_Bias;
float	f_10 = 3.3/0.9;	// 3.3;
float	f_1024 = 4095;	// 1024;


	f_AdcData = AdcData;
	f_Bias = 0;
	// Wayne this 5.0 need some read estmiation to get the right value
	voltage = (f_AdcData - f_Bias) * (f_10 / f_1024) + 5.0 + 0.02;
  voltage2 = (f_AdcData - f_Bias) * (f_10 / f_1024) + 5.0 + 0.02;
  //printf("------");
  //printf("vol:%f",voltage);

//	if( !BSP_GPIO_READ( pPMU_DC_POWER_IN ) )	// DC power in?
//	  voltage -= 0.23;
//	else
//	  voltage += 0.02;
	
	*Percent = 0;
	if( voltage >= 8.2 )
	  {
//	  *Percent = 100;
	  *Percent = 99;
	  goto EXIT;
	  }
	  
	if( voltage >= 8.1 )
	  {
	  *Percent = 95;
	  goto EXIT;
	  }

	if( voltage >= 8.0 )
	  {
	  *Percent = 90;
	  goto EXIT;
	  }

	if( voltage >= 7.9 )
	  {
	  *Percent = 85;
	  goto EXIT;
	  }

	if( voltage >= 7.8 )
	  {
	  *Percent = 80;
	  goto EXIT;
	  }

	if( voltage >= 7.7 )
	  {
	  *Percent = 75;
	  goto EXIT;
	  }

	if( voltage >= 7.6 )
	  {
	  *Percent = 70;
	  goto EXIT;
	  }

	if( voltage >= 7.575 )
	  {
	  *Percent = 65;
	  goto EXIT;
	  }

	if( voltage >= 7.55 )
	  {
	  *Percent = 60;
	  goto EXIT;
	  }

	if( voltage >= 7.525 )
	  {
	  *Percent = 55;
	  goto EXIT;
	  }

	if( voltage >= 7.5 )
	  {
	  *Percent = 50;
	  goto EXIT;
	  }

	if( voltage >= 7.475 )
	  {
	  *Percent = 45;
	  goto EXIT;
	  }

	if( voltage >= 7.45 )
	  {
	  *Percent = 40;
	  goto EXIT;
	  }

	if( voltage >= 7.425 )
	  {
	  *Percent = 35;
	  goto EXIT;
	  }

	if( voltage >= 7.4 )
	  {
	  *Percent = 30;
	  goto EXIT;
	  }

	if( voltage >= 7.35 )
	  {
	  *Percent = 25;
	  goto EXIT;
	  }

	if( voltage >= 7.3 )
	  {
	  *Percent = 20;
	  goto EXIT;
	  }

	if( voltage >= 7.2 )
	  {
	  *Percent = 15;
	  goto EXIT;
	  }

	if( voltage >= 7.1 )
	  {
	  *Percent = 10;
	  goto EXIT;
	  }

	if( voltage >= 7.0 )
	  {
	  *Percent = 5;
	  goto EXIT;
	  }
EXIT:
	return( voltage );
}
