#include "POSAPI.h"
/**
 * 20220729 modified by west
 * add PN5180 busy pin definition
 * add USB2_OC pin definition
 **/

/*
    e.g
    
    EXTIO_GPIO(_PAPEREND_BANK_ADDR,_PAPEREND_BANK_PORT,_PAPEREND_BANK_NUM,1);

    EXTIO_GPIO(_PAPEREND_BANK_ADDR,_PAPEREND_BANK_PORT,_PAPEREND_BANK_NUM,0);
*/
UCHAR EXTIO_GPIO(UCHAR BANK_ADDR, UCHAR GPIO_PORT, UCHAR GPIO_NUM, UCHAR output);
UCHAR EXTIO_GPIO_Read(UCHAR BANK_ADDR, UCHAR GPIO_PORT, UCHAR GPIO_NUM, UCHAR *value);
extern UCHAR EXTIO_IOINIT();

// this define is follow the Extention I/O circuit pin name

// (ADDR, Port) = (010, 0)
#define _PAPEREND_BANK_ADDR     0x010
#define _PAPEREND_GPIO_PORT     0x0
#define _PAPEREND_GPIO_NUM      0x0

#define _BAT_ST_IN_BANK_ADDR    0x010
#define _BAT_ST_IN_GPIO_PORT    0x0
#define _BAT_ST_IN_GPIO_NUM     0x1

#define _MSR_CLS_BANK_ADDR      0x010
#define _MSR_CLS_GPIO_PORT      0x0
#define _MSR_CLS_GPIO_NUM       0x2

#define _SD1_CD_BANK_ADDR       0x010
#define _SD1_CD_GPIO_PORT       0x0
#define _SD1_CD_GPIO_NUM        0x3

#define _Power_in_BANK_ADDR     0x010
#define _Power_in_GPIO_PORT     0x0
#define _Power_in_GPIO_NUM      0x4

#define _4G_CTS_BANK_ADDR       0x010
#define _4G_CTS_GPIO_PORT       0x0
#define _4G_CTS_GPIO_NUM        0x5

#define _TP_IRQ_BANK_ADDR       0x010
#define _TP_IRQ_GPIO_PORT       0x0
#define _TP_IRQ_GPIO_NUM        0x6

#define _SW_DETECT_BANK_ADDR    0x010
#define _SW_DETECT_GPIO_PORT    0x0
#define _SW_DETECT_GPIO_NUM     0x7


// (ADDR, Port) = (010, 1)
#define _USB1_OC_BANK_ADDR      0x010
#define _USB1_OC_GPIO_PORT      0x1
#define _USB1_OC_GPIO_NUM       0x0

#define _USB2_OC_BANK_ADDR      0x010
#define _USB2_OC_GPIO_PORT      0x1
#define _USB2_OC_GPIO_NUM       0x0

#define _TM_DETECT_BANK_ADDR    0x010
#define _TM_DETECT_GPIO_PORT    0x1
#define _TM_DETECT_GPIO_NUM     0x2

#define _AUD_INT_BANK_ADDR      0x010
#define _AUD_INT_GPIO_PORT      0x1
#define _AUD_INT_GPIO_NUM       0x4

#define _ENET2_nINT_BANK_ADDR   0x010
#define _ENET2_nINT_GPIO_PORT   0x1
#define _ENET2_nINT_GPIO_NUM    0x5

#define _PN5180_BUSY_BANK_ADDR   0x010
#define _PN5180_BUSY_GPIO_PORT   0x1
#define _PN5180_BUSY_GPIO_NUM    0x6

#define _Modem_Ring_BANK_ADDR   0x010
#define _Modem_Ring_GPIO_PORT   0x1
#define _Modem_Ring_GPIO_NUM    0x7


// (ADDR, Port) = (000, 0)
#define _NFC_RESET_BANK_ADDR    0x000
#define _NFC_RESET_GPIO_PORT    0x0
#define _NFC_RESET_GPIO_NUM     0x0

#define _ENET2_nRST_BANK_ADDR   0x000
#define _ENET2_nRST_GPIO_PORT   0x0
#define _ENET2_nRST_GPIO_NUM    0x1

#define _RESET_TDA_BANK_ADDR    0x000
#define _RESET_TDA_GPIO_PORT    0x0
#define _RESET_TDA_GPIO_NUM     0x2

#define _EMV_CLK_CUT_BANK_ADDR  0x000
#define _EMV_CLK_CUT_GPIO_PORT  0x0
#define _EMV_CLK_CUT_GPIO_NUM   0x3

#define _LED_B_BANK_ADDR        0x000
#define _LED_B_GPIO_PORT        0x0
#define _LED_B_GPIO_NUM         0x4

#define _LED_Y_BANK_ADDR        0x000
#define _LED_Y_GPIO_PORT        0x0
#define _LED_Y_GPIO_NUM         0x5

#define _LED_G_BANK_ADDR        0x000
#define _LED_G_GPIO_PORT        0x0
#define _LED_G_GPIO_NUM         0x6

#define _LED_R_BANK_ADDR        0x000
#define _LED_R_GPIO_PORT        0x0
#define _LED_R_GPIO_NUM         0x7


// (ADDR, Port) = (000, 1)
#define _PH1_BANK_ADDR          0x000
#define _PH1_GPIO_PORT          0x1
#define _PH1_GPIO_NUM           0x0

#define _PH2_BANK_ADDR          0x000
#define _PH2_GPIO_PORT          0x1
#define _PH2_GPIO_NUM           0x1

#define _EN_1838_A_BANK_ADDR    0x000
#define _EN_1838_A_GPIO_PORT    0x1
#define _EN_1838_A_GPIO_NUM     0x2

#define _PRT_LATCH_BANK_ADDR    0x000
#define _PRT_LATCH_GPIO_PORT    0x1
#define _PRT_LATCH_GPIO_NUM     0x3

#define _STB_BANK_ADDR          0x000
#define _STB_GPIO_PORT          0x1
#define _STB_GPIO_NUM           0x4

#define _EN_1838_B_BANK_ADDR    0x000
#define _EN_1838_B_GPIO_PORT    0x1
#define _EN_1838_B_GPIO_NUM     0x5

#define _VBATT_8V_SW_BANK_ADDR  0x000
#define _VBATT_8V_SW_GPIO_PORT  0x1
#define _VBATT_8V_SW_GPIO_NUM   0x6

#define _9V_SW_EN_BANK_ADDR     0x000
#define _9V_SW_EN_GPIO_PORT     0x1
#define _9V_SW_EN_GPIO_NUM      0x7


// (ADDR, Port) = (001, 0)
#define _4G_IGT_BANK_ADDR       0x001
#define _4G_IGT_GPIO_PORT       0x0
#define _4G_IGT_GPIO_NUM        0x0

#define _4G_OFF_BANK_ADDR       0x001
#define _4G_OFF_GPIO_PORT       0x0
#define _4G_OFF_GPIO_NUM        0x1

#define _Modem_RST_BANK_ADDR    0x001
#define _Modem_RST_GPIO_PORT    0x0
#define _Modem_RST_GPIO_NUM     0x1

#define _SD1_PWR_ON_BANK_ADDR   0x001
#define _SD1_PWR_ON_GPIO_PORT   0x0
#define _SD1_PWR_ON_GPIO_NUM    0x2

#define _SAM_FAST_FIND_BANK_ADDR  0x001
#define _SAM_FAST_FIND_GPIO_PORT  0x0
#define _SAM_FAST_FIND_GPIO_NUM   0x3

#define _MOD_USB_SEL_BANK_ADDR  0x001
#define _MOD_USB_SEL_GPIO_PORT  0x0
#define _MOD_USB_SEL_GPIO_NUM   0x4

#define _WIFI_DIS_BANK_ADDR     0x001
#define _WIFI_DIS_GPIO_PORT     0x0
#define _WIFI_DIS_GPIO_NUM      0x5

#define _WIFI_BT_EN_BANK_ADDR   0x001
#define _WIFI_BT_EN_GPIO_PORT   0x0
#define _WIFI_BT_EN_GPIO_NUM    0x6

#define _WIFI_BT_ON_BANK_ADDR   0x001
#define _WIFI_BT_ON_GPIO_PORT   0x0
#define _WIFI_BT_ON_GPIO_NUM    0x7


// (ADDR, Port) = (001, 1)
#define _USB1_SEL0_BANK_ADDR     0x001
#define _USB1_SEL0_GPIO_PORT     0x1
#define _USB1_SEL0_GPIO_NUM      0x0

#define _USB1_SEL1_BANK_ADDR     0x001
#define _USB1_SEL1_GPIO_PORT     0x1
#define _USB1_SEL1_GPIO_NUM      0x1

#define _MSR_PWDN_BANK_ADDR      0x001
#define _MSR_PWDN_GPIO_PORT      0x1
#define _MSR_PWDN_GPIO_NUM       0x2

#define _USB1_PWR_BANK_ADDR      0x001
#define _USB1_PWR_GPIO_PORT      0x1
#define _USB1_PWR_GPIO_NUM       0x3

#define _4G_RTS_BANK_ADDR        0x001
#define _4G_RTS_GPIO_PORT        0x1
#define _4G_RTS_GPIO_NUM         0x4

#define _TDA_DCDC_OFF_BANK_ADDR  0x001
#define _TDA_DCDC_OFF_GPIO_PORT  0x1
#define _TDA_DCDC_OFF_GPIO_NUM   0x5

#define _KEY_LED_EN_BANK_ADDR   0x001
#define _KEY_LED_EN_GPIO_PORT   0x1
#define _KEY_LED_EN_GPIO_NUM    0x6

#define _LCD_BL_EN_ADDR         0x001
#define _LCD_BL_EN_GPIO_PORT    0x1
#define _LCD_BL_EN_GPIO_NUM     0x7






