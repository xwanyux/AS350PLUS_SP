//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : MAX32550 (32-bit Platform)				    **
//**  PRODUCT  : AS330_mPOS						    **
//**                                                                        **
//**  FILE     : DEV_KBD.C                                                  **
//**  MODULE   : 							    **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2018/04/20						    **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
/** Global includes */
#include <config.h>
//#include <errors.h>
/** Other includes */
//#include <mml_skbd.h>
//#include <mml_uart.h>
//#include <mml_gpio.h>
//#include <cobra_defines.h>
//#include <cobra_macros.h>
//#include <cobra_functions.h>
/** Local includes */
//#include <private.h>
#include <uart_config.h>
#include <printf_lite.h>

#include "POSAPI.h"
#include "DEV_KBD.h"
#include <string.h>


//mml_skbd_keys_t os_KBD_keys = {0, 0, 0, 0};
volatile int	os_KBD_PressStatus = FALSE;		// key_pressed
// volatile UINT32	os_KbdEventFlag = 0;			// CANCEL key stroke will set this flag, reset by APP

UINT8	KBD_Kat[KBD_KAT_SIZE];				// keyboard allocation table, default all keys enabled

UINT32	os_KBDBL_TOUT	= 0x0;		// OFF duration of KBD back-light
UINT32	os_KBDBL_Status	= FALSE;	// OFF after BSP init


/** keys mapping on the keyboard */
//const	unsigned char AP_KeyRetnCodeTable[16] =
//{
//	'x', 'n', 'z', 'y',
//	'3', '6', '9', '#',
//	'2', '5', '8', '0',
//	'1', '4', '7', '*'
//};

const	unsigned char KBD_KatBitmap[16] =			// ByteNum[b7~b4] | BitNum[b3~b0]
{
	0x32, 0x33, 0x34, 0x35,
	0x22, 0x23, 0x24, 0x25,
	0x12, 0x13, 0x14, 0x15,
	0x02, 0x03, 0x04, 0x05
};




// ---------------------------------------------------------------------------
// FUNCTION: Initialize keyboard buffer.
// INPUT   : KatValue = default settings of KAT.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if 0
void OS_KBD_Reset( UINT8 KatValue )
{
	memset( KBD_Kat, KatValue, KBD_KAT_SIZE );
	
	os_KBD_PressStatus = FALSE;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Get current KAT settings.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void OS_KBD_GetKAT( UINT8 *kat )
{
	memmove( kat, KBD_Kat, KBD_KAT_SIZE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Get current KAT settings.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void OS_KBD_SetKAT( UINT8 *kat )
{
	memmove( KBD_Kat, kat, KBD_KAT_SIZE );
}

// ---------------------------------------------------------------------------
// FUNCTION: KBD event handle.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if 0
void	OS_KBD_EventHandler( void )
{
unsigned int	status;


	/** Do what has to be done */
	mml_keypad_interrupt_status(&status);
	if ( MML_SKBD_INTERRUPT_STATUS_OVERIS & status )
	{
		mml_keypad_clear_interrupt_status(MML_SKBD_INTERRUPT_STATUS_OVERIS);
	}
	/**  */
	if ( MML_SKBD_INTERRUPT_STATUS_PUSHIS & status )
	{
		os_KbdEventFlag = 1;	// 2019-01-15
		
		mml_keypad_read_keys(&os_KBD_keys);
		os_KBD_PressStatus = TRUE;
		
		/** Clear interruption */
		mml_keypad_clear_interrupt_status(MML_SKBD_INTERRUPT_STATUS_PUSHIS);
	}
	/**  */
	if ( MML_SKBD_INTERRUPT_STATUS_RELEASEIS & status )
	{
		mml_keypad_clear_interrupt_status(MML_SKBD_INTERRUPT_STATUS_RELEASEIS);
	}
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Initialize SECURE KEYPAD device.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE/FALSE
// ---------------------------------------------------------------------------
#if 0
UINT32	OS_KBD_Init( void )
{
int	result = MML_SKBD_ERR_UNKNOWN;
mml_skbd_config_t	config;
mml_gpio_config_t	config_bank1;
register unsigned int 	loop = 0;


	config.debounce = MML_SKBD_DBTM_10MS;
	config.inputs = ( MML_SKBD_KBDIO4 | MML_SKBD_KBDIO5 | MML_SKBD_KBDIO6 | MML_SKBD_KBDIO7 );
	config.outputs = ( MML_SKBD_KBDIO0 | MML_SKBD_KBDIO1 | MML_SKBD_KBDIO2 | MML_SKBD_KBDIO3 );
	config.reg_erase = TRUE;
	config.irq_handler = OS_KBD_EventHandler;
	result = mml_keypad_init(config);
	if ( result )
	{
		/** Failed to initialize keypad */
		result = FALSE;
		goto EXIT;
	}
	/**  */
	result =  mml_keypad_enable_interrupt_events( MML_SKBD_EVENT_PUSH | MML_SKBD_EVENT_OVERRUN );
	if ( result )
	{
		/** Failed to enable the keypad interrupts */
		result = FALSE;
		goto EXIT;
	}

	OS_KBD_Reset( 0xFF );	// all keys enabled by default
	
	// init backlight LEDs (HI active)
	// EMV_CLK_CUT: P1.18
	config_bank1.gpio_direction	= MML_GPIO_DIR_OUT;
	config_bank1.gpio_function	= MML_GPIO_NORMAL_FUNCTION;
	config_bank1.gpio_pad_config	= MML_GPIO_PAD_NORMAL;
	config_bank1.gpio_intr_mode	= 0;
	config_bank1.gpio_intr_polarity	= 0;
	result = mml_gpio_init( MML_GPIO_DEV1, MML_GPIO1_KBD_BL_PIN, 1, config_bank1 );
	if( result != NO_ERROR )
	  return( FALSE );

	// turn on LEDs
	result =  mml_gpio_write_bit_pattern( MML_GPIO_DEV1, MML_GPIO1_KBD_BL_PIN, 1, 0x01 );
	if( result != NO_ERROR )
	  return( FALSE );
	else
	  {
	  /** Delay */
	  for( loop = 0; loop < (10 * 0xFFFF); loop++ );
	  
	  // turn off LEDs
	  result =  mml_gpio_write_bit_pattern( MML_GPIO_DEV1, MML_GPIO1_KBD_BL_PIN, 1, 0x00 );
	  }	
	
	result = TRUE;
EXIT:
	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To check the key buffer status.
// INPUT   : none.
// OUTPUT  : ScanCode -- return code of the key derpessed.
//           pHead    -- pointer to the current key buffer position. (RFU)
// RETURN  : TRUE  -- key available
//           FALSE -- not available
// ---------------------------------------------------------------------------
#if 0
UINT32 OS_KBD_Status( UINT32 *ScanCode )
{
UINT32	result = FALSE;
UINT8	katmap;
UINT8	ByteNo;
UINT8	BitNo;

volatile unsigned int	in;
volatile unsigned int	out;
volatile unsigned int	i;
unsigned short		*key;


	if( os_KBD_PressStatus )
	  {
	  key = &os_KBD_keys.key0;
	  for( i = 0;i < 4;i++ )
	     {
	     in = 0x0f & *key;
	     out = ( 0xf0 & *key ) >> 4;
	     if( MML_SKBD_FLAG_KEY_PRESS & *key )
	       {
	       katmap = KBD_KatBitmap[( in - 4 ) * 4 + out];
	       ByteNo = (katmap & 0xF0) >> 4;
	       BitNo  = (katmap & 0x0F);
	       BitNo = 1 << BitNo;
	       if( !(KBD_Kat[ByteNo] & BitNo) )
	         continue;
	         
	       *ScanCode = AP_KeyRetnCodeTable[( in - 4 ) * 4 + out];
	       
	       result = TRUE;
	       break;
	       }

	     *key = 0;
	     key++;
	     }
	  }

	return( result );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To wait for keystroke and read the key.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : the scan code read.
// ---------------------------------------------------------------------------
#if 0
UINT32	OS_KBD_GetChar( void )
{
UINT32	sc;


	while( OS_KBD_Status( (UINT32 *)&sc ) == FALSE );	// wait key stroke
	
	os_KBD_PressStatus = FALSE;
	
	return( sc );
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: turn on/off kbd backlight.
// INPUT   : flag - TRUE = ON
//		    FALSE= OFF
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if 0
void	KBD_BackLight( UINT32 flag )
{
	if( flag )
	  {
	  // --- Turn on (P1.18) ---
	  mml_gpio_write_bit_pattern( MML_GPIO_DEV1, MML_GPIO1_KBD_BL_PIN, 1, 0x01 );	// HI
	  }
	else
	  {
	  // --- Turn off (P1.18) ---
	  mml_gpio_write_bit_pattern( MML_GPIO_DEV1, MML_GPIO1_KBD_BL_PIN, 1, 0x00 );	// LO
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: KBD back light timer ISR.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if 0
void	OS_KBD_BL_Task( void )
{
UINT32	signal = TRUE;


	switch( os_KBDBL_TOUT )
	      {
	      case 0x00000000:	// turn off right away
	      	   
	      	   signal = FALSE;
	      	   break;
	      	   
	      case 0xFFFFFFFF:	// turn on forever
	      	   
	      	   break;
	      	   
	      default:		// turn on in timeout
	      	   
	      	   os_KBDBL_TOUT--;
	      	   break;
	      }

	if( signal == TRUE )
	  {
	  if( os_KBDBL_Status == FALSE )	// current status = OFF?
	    {
	    KBD_BackLight( TRUE );
	    os_KBDBL_Status = TRUE;
	    }
	  }
	else
	  {
	  if( os_KBDBL_Status == TRUE )		// current status = ON?
	    {
	    KBD_BackLight( FALSE );
	    os_KBDBL_Status = FALSE;
	    }
	  }
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To control backlight function for KBD.
// INPUT   : duration of turning on back-light of KBD in 10ms.
//           0x00000000 = turn off right away.
//	     0xFFFFFFFF = turn on forever.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#if 0
void	OS_KBD_BackLight( UINT32 duration )
{
	os_KBDBL_TOUT = duration;
}
#endif
