


#ifndef _UNITEST_POSAPI_H_
#define _UNITEST_POSAPI_H_


#include "POSAPI.h"
#include "LCDTFTAPI.h"
//#include "LCDTFTAPI.h"

#define testFail 0
#define testSucc 1




// define general testing function

extern void TEST_EQUAL(UCHAR result, UCHAR expect);
extern void TEST_NOT_EQUAL(UCHAR result, UCHAR expect);
extern void Delay(UCHAR num);



/*
** Unit Test KEYBOARD (api level)    PASS
*/

extern void Total_Keyboard_TEST(); // include all of things below
extern void Normal_Usage_KEYBOARD_TEST();
extern void Close_Without_Opening_TEST();
extern void Status_Without_Opening_TEST();
extern void GetChar_Without_Opening_TEST();
extern void Status_GetChar_Close_After_Normal_Closing_TEST();
extern void Double_Opeing_KEYBOARD_TEST();
extern void KEYPAD_LIMIT_ACESS_TEST();
extern void KEYPAD_WHILE();


/*
*  Unit Test Timer (api level)   PASS
*/

extern void Normal_Usage_Timer_TEST(UCHAR mSTime10);
extern void Mutiple_Usage_Timer_TEST();
extern void Two_Usage_Timer_TEST();
extern void Three_Usage_Timer_TEST();
extern void Doulbe_Close_Timer_TEST();

extern void ECL_LV1_UTI_Wait_TEST();
extern void Normal_Usage_Tim3_Test(ULONG micro);
extern void Doulbe_Open_And_Close_Tim2_Tim3_TEST();



/*
*  Unit Test Buzzer (api level)
*/

extern void Normal_Usage_Buz_TEST();
extern void Multiple_Sound_Function_TEST();
extern void Multiple_sound_by_parameter_With_Auto_Close_TEST();
extern void Mutiple_Buzzer_TEST();  // close all not pass, but it's not relevant. Because now we just have a buzzer.
extern void Two_Buzzer_Test();
extern void After_Close_Buz_TEST();

/*
*   Unit Test LCDTFT (api level)
*/

extern void Normal_Usage_LCDTFT_clear_TEST();
extern void Normal_Usage_LCDTFT_putstring_TEST();
extern void Normal_Usage_LCDTFT_putgraphic_TEST();
extern void Normal_Usage_LCDTFT_putwinbmp_TEST();
extern void TEST_showGraphic();
extern void TEST_ShowString();
extern void TEST_showGraphic2();


/*
*   Unit Test AUX (api level)   PASS
*/

extern void Normal_Usage_Aux_BYPASS_10_char_TEST();
extern void Normal_Usage_Aux_BYPASS_Unlimit_char_TEST();
extern void Normal_Usage_Aux_STX_ACK_NoReSend_TEST();
extern void Normal_Usage_Aux_STX_ACK_ReSend_TEST();
extern void Normal_Usage_Aux_STX_NO_ACK_NoReSend_TEST(); // meaningless because STX must need ACK
extern void Normal_Usage_Aux_SOH_ACK_NoReSend_TEST();
extern void Normal_Usage_Aux_SOH_ACK_ReSend_TEST();
extern void Normal_Usage_Aux_SOH_NO_ACK_NoReSend_TEST(); // meaningless because SOH must need ACK
extern void Transmit_Mutiple_Mesaage_SOH_TEST();
extern void Double_Close_And_Double_Open_Aux_TEST();



/*
*   RTC test()  PASS
*/

extern void Normal_Usage_RTC_setdate_TEST();
extern void Normal_Usage_RTC_getdate_TEST();
extern void RTC_Double_Open_And_Double_Close_TEST();

/*
*  Contactless LV1 test  (PASS)
*

*
*/

extern void WAKEUPA_CTLS_LV1_TEST();
extern void EMV_POLLING_CTLS_LV1_TEST();
extern void EMV_MAINLOOP_PRE_VALIDATION_CTLS_LV1_TEST();
extern void EMV_POLLING_DETAIL_CTLS_LV1_TEST();
extern void Check_SPI_CTLS_TEST();
extern void EMV_MAINLOOP_TEST();
extern void EMV_MainLoop_PreValidation_While_TEST();




/*
*
*    CPHR test
*
*
*/

extern void TEST_CRYTO();
extern void TEST_CRYTO2();
extern void TEST_CRYTO3();
extern void TEST_CRYPTO4();
extern void TEST_CRYPTO5();
extern void TEST_CRYPTO6();
extern void TEST_CRYPTO7();
extern void TEST_CRYPTO8();
extern void TEST_CRYPTO9();
extern void TEST_CRYPTO10();
extern void TEST_CRYPTO11();
extern void TEST_CRYPTO12();
extern void TEST_CRYPTO13();
extern void TEST_CRYPTO14();
extern void TEST_CRYPTO15();
extern void TEST_CRYPTO16();
extern void DES_3DES_AES_TEST();
extern void HASH_AND_RANOM_TEST();


/*
*
*    PMU test
*
*
*/
extern void Keyboard_Refresh_PowerSavingTime_TEST();
extern void Keyboard_Refresh_withLCD_PowerSavingTime_TEST();
extern void keyboard_testing_differnt_power_down_mode();
extern void RTC_AND_PMU_test();



/*
*
*    CTLS LV2 test
*
*
*/
extern void ReadyForSale_VISA_TEST();
extern void ReadyForSale_JCB_TEST();
extern void ReadyForSale_MASTER_TEST();
extern void ReadyForSale_AE_TEST();
extern void ReadyForSale_CUP_TEST();


/*
*
*    TSC test
*
*/

extern void  TEST_TouchScreen();
extern void  TEST_TouchScreen2();
extern void  TEST_signed();
extern void  TEST_touch_botton();
extern void  TEST_getSign();



/*
*   LAN test
*
*/

extern void TEST_SETIP_CONFIG();
extern void TEST_DHCP_SET();
extern void TEST_api_lan_lstatus();
extern void TEST_PING();
extern void TEST_Complete_Process_HEX();
extern void TEST_Complete_Process_BCD();
extern void TEST_RX_READY();
extern void TEST_GETIP_CONFIG();
extern void TEST_NotValid_Usage();
extern void SET_IP();


/*
*   MSR test
*
*/
extern void Test_Reading_Binary_data();
extern void Test_Reading_Binary_data2();
extern void testing_reading_binary3();
extern void teseting_track2();
extern void testing_IOCTRL();
extern void testing_track1_2();
extern void testing_track1_2_3();


/*
*   IFM test
*
*/

extern void test_I2C();
extern void testI2C_2();
extern void test_present_SAM2();
extern void test_data_of_IC_card();
extern void test_timer_IC_CARD();
extern void test_GPIO_speed();
extern void test_data_of_IC_card2();
extern void test_data_of_IC_card3();
extern void test_I2c_in_kernel();
extern int  imx_sim_test();
extern void test_data_of_IO_ENABLE();
extern int  ProtocalT1_Test();
extern void test_one_card_ATR_POSAPI();
extern void test_multi_card_ATR_POSAPI();
extern void test_one_card_APDU_POSAPI();
extern void test_multi_card_APDU_POSAPI();
extern void test_switch_protocal_between_T0_and_T1();
extern void test_ICC_MSR_dirver();
extern void test_ICC_MSR_with_4SAM_dirver();
extern void test_ICC_MSR_with_multithread();
extern void test_one_card_APDU_fast_find();
extern void test_two_card_APDU_fast_find_withoud_thread();
extern void test_two_card_APDU_fast_find_with_thread();
extern void test_ICC_MSR_with_4SAM_dirver_APDU();
extern void msr_fast_find_double_test();
extern void test_linux_timer_ms(int ms);
extern void test_linux_timer_micros(int micro);
extern void test_gpio_speed();
extern void test_ICC_MSR_with_4SAM_dirver_APDU_multiple_time();
extern void test_ICC_MSR_with_SAM_dirver_APDU_multiple_time();
extern void test_one_card_APDU_POSAPI_multiple_time_deactivated();
/*
*   PRINTER test
*
*/


extern void POSAPI_PRINTER_test();


/*
*       modem test
*
*/
extern void modem_test_provide_by_west(int MDM_baud);


/**
 *      FS test
 * 
 */
extern void POSAPI_FS_TEST();
extern void test_linux_file_write_read();
extern void FS_double_write();
extern void test_linux_double_write();
extern void test_delete_file();

/**
 *      SRAM test
 * 
 */

extern void test_write_sram();
extern void test_read_sram();
extern void test_clear_sram();
extern void test_read_write_address();

/**
 *      FLASH test
 */

extern void test_write_flash();
extern void test_read_flash();
extern void test_clear_flash();

/**
 *      FTP  test
 */

extern void ftp_connect_test();
extern void ftp_connect_test_2();
extern void ftp_connect_test_3();
extern void ftp_connect_test_4();

/**
 *      SSL test
 * 
 */

extern void ssl_test();
extern void ssl_test2();

/**
 *     sm test
 * 
 */
extern void test_sm_hello_world(); 
extern void test_sm_with_caam_kb();
extern void test_save_key_caam_kb();
extern void test_get_encdoe_key_from_file_caam_kb();
extern void init_secure_region_caam_kb();
extern void test_secure_region_read_write();

/**
 *     os sram
 */

extern void os_flash_test();



#endif