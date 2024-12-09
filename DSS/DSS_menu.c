


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bsp_tmr.h"
#include "bsp_types.h"
#include "POSAPI.h"
#include "DSS_function.h"
#include "OS_LIB.h"
#include "OS_FLASH.h"

UCHAR DSS_menu_ethernet()
{
  UCHAR    const dss_menu[]={"====  ETHERNET  ===="};
  UCHAR    const dss_func1[]={"[1] TCP/IP SETUP "};
  UCHAR    const dss_func2[]={"[2] PING "};
  UCHAR    const dss_func3[]={"[3] ETHERNET "};
  UCHAR    const dss_message_successful[]={"Successfully!"};
  UCHAR    const dss_message_reboot[]={"Reboot..."};
  UCHAR    const dss_message_format1[]={"Please format the"};
  UCHAR    const dss_message_format2[]={"file system before!"};
  UCHAR	   const dss_type1[] = {"[1] APP"};
  UCHAR	   const dss_type2[] = {"[2] FILE"};
  UCHAR	   const dss_format[] = {"FORMAT(Y/N)?"};
  UCHAR	   const dss_com0[] = {"[1] COM0"};
  UCHAR	   const dss_com2[] = {"[2] COM1"};
  UCHAR	   const msg_any_key[] = {"(Press any key)"};
  UCHAR	   sbuf[256];
  UCHAR	   dbuf[256];
  UCHAR    apiStatus;
  UCHAR	   first = 1;
  UCHAR	result;
  UCHAR LineStatus;
	
MENU:
  LIB_LCD_Cls();
  LIB_LCD_Puts( 0, 0, FONT4+attrREVERSE, sizeof( dss_menu ), (UINT8 *)dss_menu );

SEL_CHANNEL:

  LIB_LCD_ClearRow( 1, 2, FONT4 );

  LIB_LCD_Puts( 1, 0, FONT4, sizeof(dss_func1), (UINT8 *)dss_func1 );

ENTER_MENU:
  switch ( LIB_WaitKey() )
    {

    case ( '1' ):
    {
      LIB_LCD_ClearRow( 1, 3, FONT4 );
      return DSS_function_LANset();
    }

    case ('x')://cancel
    {
      //  SYS_FUNC_SetAppBootStatus();	// 20210219 comment out by west because don't know doing what
      return apiFailed;
    }

    default:
    {
      goto ENTER_MENU;

    }
    }



}