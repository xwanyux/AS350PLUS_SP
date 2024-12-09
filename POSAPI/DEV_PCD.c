//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : LPC1788 (32-bit Platform)                                  **
//**  PRODUCT  : AS350                                                      **
//**                                                                        **
//**  FILE     : DEV_PCD.C                                                  **
//**  MODULE   : 						    	    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2013/04/26                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2013 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <string.h>

#include "POSAPI.h"
#include "DEV_PCD.h"
#include "LCDTFTAPI.h"

extern const GUI_BITMAP bmtaplogo;

UINT32	os_flag_pcd_led = 0;

UINT32	os_flag_led_blue_on = 0;
UINT32	os_flag_led_yellow_on = 0;
UINT32	os_flag_led_green_on = 0;
UINT32	os_flag_led_red_on = 0;

UINT32	os_flag_led_blue_off = 0;
UINT32	os_flag_led_yellow_off = 0;
UINT32	os_flag_led_green_off = 0;
UINT32	os_flag_led_red_off = 0;

UINT16	os_LED_BLUE_ON_TOUT = 0;
UINT16	os_LED_BLUE_ON_TOUT_BAK = 0;
UINT16	os_LED_YELLOW_ON_TOUT = 0;
UINT16	os_LED_YELLOW_ON_TOUT_BAK = 0;
UINT16	os_LED_GREEN_ON_TOUT = 0;
UINT16	os_LED_GREEN_ON_TOUT_BAK = 0;
UINT16	os_LED_RED_ON_TOUT = 0;
UINT16	os_LED_RED_ON_TOUT_BAK = 0;

UINT16	os_LED_BLUE_OFF_TOUT = 0;
UINT16	os_LED_BLUE_OFF_TOUT_BAK = 0;
UINT16	os_LED_YELLOW_OFF_TOUT = 0;
UINT16	os_LED_YELLOW_OFF_TOUT_BAK = 0;
UINT16	os_LED_GREEN_OFF_TOUT = 0;
UINT16	os_LED_GREEN_OFF_TOUT_BAK = 0;
UINT16	os_LED_RED_OFF_TOUT = 0;
UINT16	os_LED_RED_OFF_TOUT_BAK = 0;

void LCD_FillRect( UCHAR dhn, UINT X_START, UINT X_END, UINT Y_START, UINT Y_END, ULONG COLOR )
{
API_LCDTFT_RECT rect;
	rect.Xstart	= Y_START;
	rect.Xend	= Y_END;
	rect.Ystart	= X_START;
	rect.Yend	= X_END;
	rect.Palette[0]	= COLOR>>16;
	rect.Palette[1]	= COLOR>>8;
	rect.Palette[2]	= COLOR;
	api_lcdtft_fillRECT( 0, rect );
}
// ---------------------------------------------------------------------------
// FUNCTION: PCD LED Control Task. (called by timer ISR)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_PCD_ResetPara( void )
{
	os_flag_pcd_led = 0;

	os_flag_led_blue_on = 0;
	os_flag_led_yellow_on = 0;
	os_flag_led_green_on = 0;
	os_flag_led_red_on = 0;

	os_flag_led_blue_off = 0;
	os_flag_led_yellow_off = 0;
	os_flag_led_green_off = 0;
	os_flag_led_red_off = 0;

	os_LED_BLUE_ON_TOUT = 0;
	os_LED_BLUE_ON_TOUT_BAK = 0;
	os_LED_YELLOW_ON_TOUT = 0;
	os_LED_YELLOW_ON_TOUT_BAK = 0;
	os_LED_GREEN_ON_TOUT = 0;
	os_LED_GREEN_ON_TOUT_BAK = 0;
	os_LED_RED_ON_TOUT = 0;
	os_LED_RED_ON_TOUT_BAK = 0;

	os_LED_BLUE_OFF_TOUT = 0;
	os_LED_BLUE_OFF_TOUT_BAK = 0;
	os_LED_YELLOW_OFF_TOUT = 0;
	os_LED_YELLOW_OFF_TOUT_BAK = 0;
	os_LED_GREEN_OFF_TOUT = 0;
	os_LED_GREEN_OFF_TOUT_BAK = 0;
	os_LED_RED_OFF_TOUT = 0;
	os_LED_RED_OFF_TOUT_BAK = 0;
}

// ---------------------------------------------------------------------------
// FUNCTION: PCD LED Control Task. (called by timer ISR)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_PCD_LED_Task( void )
{
  	if( !os_flag_pcd_led )
	  return;
	
  	// LED BLUE
  	if( os_LED_BLUE_ON_TOUT )
	  {
	  if( os_LED_BLUE_ON_TOUT == 0xFFFF )	// always turn on?
	    {
	    if( !os_flag_led_blue_on )
	      {
//	      LCD_FillRect( 0, 0, 16, 172, 188, 0x7C00 );	// BLUE ON
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_BLUE, LED_Y_END_BLUE, LED_COLOR_BLUE );
	      
	      os_flag_led_blue_on = 1;
	      os_flag_led_blue_off = 0;
	      }
	    }
	  else
	    {
	    if( !os_flag_led_blue_on )
	      {
//	      LCD_FillRect( 0, 0, 16, 172, 188, 0x7C00 );	// BLUE ON
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_BLUE, LED_Y_END_BLUE, LED_COLOR_BLUE );
	    
	      os_flag_led_blue_on = 1;
	      os_flag_led_blue_off = 0;
	      }
	    
	    os_LED_BLUE_ON_TOUT--;
	    }
	  }
	else
	  {
	  if( os_LED_BLUE_OFF_TOUT )
	    {
	    if( !os_flag_led_blue_off )
	      {
//	      LCD_FillRect( 0, 0, 16, 172, 188, 0x6318 );	// BLUE OFF
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_BLUE, LED_Y_END_BLUE, LED_COLOR_OFF );
	      
	      os_flag_led_blue_off = 1;
	      os_flag_led_blue_on = 0;
	      }
	    
	    os_LED_BLUE_OFF_TOUT--;
	    }
	  else
	    {
	    if( !os_flag_led_blue_off )
	      {
//	      LCD_FillRect( 0, 0, 16, 172, 188, 0x6318 );	// BLUE OFF
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_BLUE, LED_Y_END_BLUE, LED_COLOR_OFF );
	      
	      os_flag_led_blue_off = 1;
	      }
	    
	    os_LED_BLUE_ON_TOUT = os_LED_BLUE_ON_TOUT_BAK;
	    os_LED_BLUE_OFF_TOUT = os_LED_BLUE_OFF_TOUT_BAK;
	    
	    os_flag_led_blue_on = 0;
	    }
	  }
	
  	// LED YELLOW
  	if( os_LED_YELLOW_ON_TOUT )
	  {
	  if( os_LED_YELLOW_ON_TOUT == 0xFFFF )	// always turn on?
	    {
	    if( !os_flag_led_yellow_on )
	      {
//	      LCD_FillRect( 0, 0, 16, 132, 148, 0x03FF );	// YELLOW ON
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_YELLOW, LED_Y_END_YELLOW, LED_COLOR_YELLOW );
	      
	      os_flag_led_yellow_on = 1;
	      os_flag_led_yellow_off = 0;
	      }
	    }
	  else
	    {
	    if( !os_flag_led_yellow_on )
	      {
//	      LCD_FillRect( 0, 0, 16, 132, 148, 0x03FF );	// YELLOW ON
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_YELLOW, LED_Y_END_YELLOW, LED_COLOR_YELLOW );
	      
	      os_flag_led_yellow_on = 1;
	      os_flag_led_yellow_off = 0;
	      }
	    
	    os_LED_YELLOW_ON_TOUT--;
	    }
	  }
	else
	  {
	  if( os_LED_YELLOW_OFF_TOUT )
	    {
	    if( !os_flag_led_yellow_off )
	      {
//	      LCD_FillRect( 0, 0, 16, 132, 148, 0x6318 );	// YELLOW OFF
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_YELLOW, LED_Y_END_YELLOW, LED_COLOR_OFF );
	      
	      os_flag_led_yellow_off = 1;
	      os_flag_led_yellow_on = 0;
	      }
	    
	    os_LED_YELLOW_OFF_TOUT--;
	    }
	  else
	    {
	    if( !os_flag_led_yellow_off )
	      {
//	      LCD_FillRect( 0, 0, 16, 132, 148, 0x6318 );	// YELLOW OFF
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_YELLOW, LED_Y_END_YELLOW, LED_COLOR_OFF );
	      
	      os_flag_led_yellow_off = 1;
	      }
	    
	    os_LED_YELLOW_ON_TOUT = os_LED_YELLOW_ON_TOUT_BAK;
	    os_LED_YELLOW_OFF_TOUT = os_LED_YELLOW_OFF_TOUT_BAK;
	    
	    os_flag_led_yellow_on = 0;
	    }
	  }

  	// LED GREEN
  	if( os_LED_GREEN_ON_TOUT )
	  {
	  if( os_LED_GREEN_ON_TOUT == 0xFFFF )	// always turn on?
	    {
	    if( !os_flag_led_green_on )
	      {
//	      LCD_FillRect( 0, 0, 16, 92, 108, 0x03E0);		// GREEN ON
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_GREEN, LED_Y_END_GREEN, LED_COLOR_GREEN );
	      
	      os_flag_led_green_on = 1;
	      os_flag_led_green_off = 0;
	      }
	    }
	  else
	    {
	    if( !os_flag_led_green_on )
	      {
//	      LCD_FillRect( 0, 0, 16, 92, 108, 0x03E0 );	// GREEN ON
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_GREEN, LED_Y_END_GREEN, LED_COLOR_GREEN );
	    
	      os_flag_led_green_on = 1;
	      os_flag_led_green_off = 0;
	      }
	    
	    os_LED_GREEN_ON_TOUT--;
	    }
	  }
	else
	  {
	  if( os_LED_GREEN_OFF_TOUT )
	    {
	    if( !os_flag_led_green_off )
	      {
//	      LCD_FillRect( 0, 0, 16, 92, 108, 0x6318 );	// GREEN OFF
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_GREEN, LED_Y_END_GREEN, LED_COLOR_OFF );
	      
	      os_flag_led_green_off = 1;
	      os_flag_led_green_on = 0;
	      }
	    
	    os_LED_GREEN_OFF_TOUT--;
	    }
	  else
	    {
	    if( !os_flag_led_green_off )
	      {
//	      LCD_FillRect( 0, 0, 16, 92, 108, 0x6318 );	// GREEN OFF
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_GREEN, LED_Y_END_GREEN, LED_COLOR_OFF );
	      
	      os_flag_led_green_off = 1;
	      }
	    
	    os_LED_GREEN_ON_TOUT = os_LED_GREEN_ON_TOUT_BAK;
	    os_LED_GREEN_OFF_TOUT = os_LED_GREEN_OFF_TOUT_BAK;
	    
	    os_flag_led_green_on = 0;
	    }
	  }

  	// LED RED
  	if( os_LED_RED_ON_TOUT )
	  {
	  if( os_LED_RED_ON_TOUT == 0xFFFF )	// always turn on?
	    {
	    if( !os_flag_led_red_on )
	      {
//	      LCD_FillRect( 0, 0, 16, 52, 68, 0x001F );		// RED ON
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_RED, LED_Y_END_RED, LED_COLOR_RED );
	      
	      os_flag_led_red_on = 1;
	      os_flag_led_red_off = 0;
	      }
	    }
	  else
	    {
	    if( !os_flag_led_red_on )
	      {
//	      LCD_FillRect( 0, 0, 16, 52, 68, 0x001F );		// RED ON
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_RED, LED_Y_END_RED, LED_COLOR_RED );
	    
	      os_flag_led_red_on = 1;
	      os_flag_led_red_off = 0;
	      }
	    
	    os_LED_RED_ON_TOUT--;
	    }
	  }
	else
	  {
	  if( os_LED_RED_OFF_TOUT )
	    {
	    if( !os_flag_led_red_off )
	      {
//	      LCD_FillRect( 0, 0, 16, 52, 68, 0x6318 );		// RED OFF
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_RED, LED_Y_END_RED, LED_COLOR_OFF );
	      
	      os_flag_led_red_off = 1;
	      os_flag_led_red_on = 0;
	      }
	    
	    os_LED_RED_OFF_TOUT--;
	    }
	  else
	    {
	    if( !os_flag_led_red_off )
	      {
//	      LCD_FillRect( 0, 0, 16, 52, 68, 0x6318 );		// RED OFF
	      LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_RED, LED_Y_END_RED, LED_COLOR_OFF );
	      
	      os_flag_led_red_off = 1;
	      }
	    
	    os_LED_RED_ON_TOUT = os_LED_RED_ON_TOUT_BAK;
	    os_LED_RED_OFF_TOUT = os_LED_RED_OFF_TOUT_BAK;
	    
	    os_flag_led_red_on = 0;
	    }
	  }
}

// ---------------------------------------------------------------------------
// FUNCTION: LCD command -- Clear Screen.
// INPUT   : ParaLen  - length of parameters.
//	     Para     - parameters. (16 bytes)
//
//			PixRow(2)		- RFU
//			PixColumn(2)		- RFU
//			FontID & Attribute(1)
//                      RGB mode(1)
//			FG_Palette(3)		- RFU
//			FontSize_Height(1)	- RFU
//			FontSize_Width(1)	- RFU
//			BG_Palette(3)		- effective
//                      RFU(2)
//
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT32  PCD_ControlLED( UINT16 ParaLen, UINT8 *Para )
{
  
	return( apiOK );
}
UINT32  LCD_PutTapLogo( UINT16 ParaLen, UINT8 *Para )
{
API_LCDTFT_GRAPH	graph;		// 顯示圖形外觀
API_LCDTFT_ICON		icon;		// 顯示圖形設定
UCHAR	*bitmapaddr;
UCHAR	result=apiOK;

	graph.ID = 0;
	graph.RGB = 0;
	bitmapaddr= &bmtaplogo;
	memmove( &graph.Bitmap, &bmtaplogo, sizeof(GUI_BITMAP) );	
	result=api_lcdtft_putgraphics( 0, graph );
	
	memset( (UCHAR *)&icon, 0x00, sizeof(icon) );
	icon.ID = 0;
	icon.Method = 0;
	icon.Width	= 146;
	icon.Height	= 86;
	icon.Xleft	= 50;
	icon.Ytop	= 120;
	result=api_lcdtft_showICON( 0, icon );
	return( result );
}
// ---------------------------------------------------------------------------
// FUNCTION: PCD command -- Show PCD icons.
// INPUT   : ParaLen  - length of parameters.
//	     Para     - parameters. (16 bytes)
//
//			ID(2)
//			BlinkOn(2)
//			BlinkOff(2)
//                      RFU(10)
//
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UINT32  PCD_ShowPCD( UINT16 ParaLen, UINT8 *Para )
{
UINT32	result = apiOK;
UINT16	id;
UINT16	blink_on;
UINT16	blink_off;
UINT16	id_org;

  
  	if( ParaLen != 16 )
	  return( apiFailed );
	
	os_flag_pcd_led = 0;
	
	id = Para[OFS_PCD_ID_L] + Para[OFS_PCD_ID_H]*256;
	blink_on  = Para[OFS_PCD_BLINK_ON_L] + Para[OFS_PCD_BLINK_ON_H]*256;
	blink_off = Para[OFS_PCD_BLINK_OFF_L] + Para[OFS_PCD_BLINK_OFF_H]*256;
	
	if( id == IID_TAP_LOGO )
	  {
	  result = LCD_PutTapLogo( ParaLen, Para );
//	  LCD_FillRect ( 0, 0, 16-1, 0, 240-1, 0xFFFF);		// WHITE area for LED
	  
//	  LCD_FillRect ( 0, 24, 24+64-1, 0, 240-1, 0xFFFF);	// WHITE area for MSG
	  
//	  LCD_FillRect ( 0, 0, 16, 172, 188, 0x6318);
//	  LCD_FillRect ( 0, 0, 16, 132, 148, 0x6318);
//	  LCD_FillRect ( 0, 0, 16, 92, 108, 0x6318);
//	  LCD_FillRect ( 0, 0, 16, 52, 68, 0x6318);
	  
	  OS_PCD_ResetPara();
	  
	  os_flag_pcd_led = 1;
	  }
	
	if( (id & IID_LED_ONESHOT) && (id != (IID_TAP_LOGO+IID_LED_ONESHOT)) )	// 2015-05-04
	  {
	  id_org = id;
	  id &= ~IID_LED_ONESHOT;
	  
	  OS_PCD_ResetPara();
//	  LCD_FillRect( 0, 0, 16, 172, 188, 0x6318 );	// BLUE OFF
	  LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_BLUE, LED_Y_END_BLUE, LED_COLOR_OFF );
//	  LCD_FillRect( 0, 0, 16, 132, 148, 0x6318 );	// YELLOW OFF
	  LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_YELLOW, LED_Y_END_YELLOW, LED_COLOR_OFF );
//	  LCD_FillRect( 0, 0, 16, 92, 108, 0x6318);	// GREEN OFF
	  LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_GREEN, LED_Y_END_GREEN, LED_COLOR_OFF );
//	  LCD_FillRect( 0, 0, 16, 52, 68, 0x6318 );	// RED OFF
	  LCD_FillRect( 0, LED_X_START, LED_X_END, LED_Y_START_RED, LED_Y_END_RED, LED_COLOR_OFF );
	  
	  if( id_org == IID_LED_ONESHOT )
	    {
	    
	    return( result );		// 2015-05-05, turn off all LEDs
	    }
	  }
	
	if( id == IID_LED_BLUE )
	  {
	  os_LED_BLUE_ON_TOUT = blink_on;
	  os_LED_BLUE_ON_TOUT_BAK = blink_on;
	  os_LED_BLUE_OFF_TOUT = blink_off;
	  os_LED_BLUE_OFF_TOUT_BAK = blink_off;
	  //LCD_FillRect ( 0, 0, 16, 172, 188, 0x7C00);	// BLUE ON
	  //LCD_FillRect ( 0, 0, 16, 172, 188, 0x6318);	// BLUE OFF
	  
	  os_flag_led_blue_on = 0;
	  os_flag_led_blue_off = 0;
	  
	  os_flag_pcd_led = 1;
	  }
	
	if( id == IID_LED_YELLOW )
	  {
	  os_LED_YELLOW_ON_TOUT = blink_on;
	  os_LED_YELLOW_ON_TOUT_BAK = blink_on;
	  os_LED_YELLOW_OFF_TOUT = blink_off;
	  os_LED_YELLOW_OFF_TOUT_BAK = blink_off;
	  //LCD_FillRect ( 0, 0, 16, 132, 148, 0x03FF);	// YELLOW ON
	  //LCD_FillRect ( 0, 0, 16, 132, 148, 0x6318);	// YELLOW OFF
	  
	  os_flag_led_yellow_on = 0;
	  os_flag_led_yellow_off = 0;
	  
	  os_flag_pcd_led = 1;
	  }
	
	if( id == IID_LED_GREEN )
	  {
	  os_LED_GREEN_ON_TOUT = blink_on;
	  os_LED_GREEN_ON_TOUT_BAK = blink_on;
	  os_LED_GREEN_OFF_TOUT = blink_off;
	  os_LED_GREEN_OFF_TOUT_BAK = blink_off;
	  //LCD_FillRect ( 0, 0, 16, 92, 108, 0x03E0);	// GREEN ON
	  //LCD_FillRect ( 0, 0, 16, 92, 108, 0x6318);	// GREEN OFF
	  
	  os_flag_led_green_on = 0;
	  os_flag_led_green_off = 0;
	  
	  os_flag_pcd_led = 1;
	  }
	
	if( id == IID_LED_RED )
	  {
	  os_LED_RED_ON_TOUT = blink_on;
	  os_LED_RED_ON_TOUT_BAK = blink_on;
	  os_LED_RED_OFF_TOUT = blink_off;
	  os_LED_RED_OFF_TOUT_BAK = blink_off;
	  //LCD_FillRect ( 0, 0, 16, 52, 68, 0x001F);	// RED ON
	  //LCD_FillRect ( 0, 0, 16, 52, 68, 0x6318);	// RED OFF
	  
	  os_flag_led_red_on = 0;
	  os_flag_led_red_off = 0;
	  
	  os_flag_pcd_led = 1;
	  }
	
	if( id == IID_PCD_EXIT )
	  {
	  OS_PCD_ResetPara();
	  LCD_FillRect( 0, 0, 320, 0, 240, 0XFFFFFF );//clear screen
	  }
	
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: PCD command processor.
// INPUT   : cid      - command id.
//           ParaLen  - length of parameters.
//	     Para     - parameters.
// 	     DataLen  - length of data.
//           Data     - data.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
#if 0
UINT32  PCD_CommandProcess( UINT8 cid, UINT16 ParaLen, UINT8 *Para, UINT16 DataLen, UINT8 *Data )
{
UINT32  result;

  
	result = apiFailed;
        switch( cid )
              {
	      case CID_PCD_SHOW_PCD:
                   
		   result = PCD_ShowPCD( ParaLen, Para );
		   break;
		     
//	      case CID_PCD_CONTROL_LED:
//		
//		   result = PCD_ControlLED( ParaLen, Para );
//		   break;
		   
              }
        
        return( result );
}
#endif
