//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : 							    **
//**  PRODUCT  : AS330	                                                    **
//**                                                                        **
//**  FILE     : MPU.C							    **
//**  MODULE   : 			                                    **
//**									    **
//**  FUNCTION : Function entries of secure memory accessing.		    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2019/07/26                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2019 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------

#include "POSAPI.h"
#include "OS_LIB.h"
#include "MPU.h"

static	uint32_t mpu_access_flag = 0;
static	uint32_t mpu_in_pedk = 0;		// 0x01=already in PEDK, no system switching is needed.

#if	0
// ---------------------------------------------------------------------------
// FUNCTION: enable MPU.
// INPUT   : flag
// OUTPUT  : none.
// RETURN  : 0
// ---------------------------------------------------------------------------
int	mpu_enable(uint32_t flag)
{
	volatile MPU_Type	*reg_mpu = (volatile MPU_Type*)MPU_BASE;
	__asm volatile( "dsb" );
	__asm volatile( "isb" );
	reg_mpu->CTRL = flag;

	return 0;
}

// ---------------------------------------------------------------------------
// FUNCTION: disable MPU.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : 0
// ---------------------------------------------------------------------------
int	mpu_disable(void)
{
	volatile MPU_Type	*reg_mpu = (volatile MPU_Type*)MPU_BASE;
	__asm volatile( "dsb" );
	__asm volatile( "isb" );
	reg_mpu->CTRL = 0;

	return 0;
}

// ---------------------------------------------------------------------------
// FUNCTION: 
// INPUT   : 
// OUTPUT  : 
// RETURN  : 
// ---------------------------------------------------------------------------
static uint32_t	mpu_convert_size(uint32_t size)
{
	int i;
	int s = 32;

	for (i = 5; i < 32; i++) {
		if (size <= s)
			break;
		s <<= 1;
	}
	
	return i - 1;
}

// ---------------------------------------------------------------------------
// FUNCTION: 
// INPUT   : 
// OUTPUT  : 
// RETURN  : 
// ---------------------------------------------------------------------------
int	mpu_config_region(uint8_t region, uint32_t addr, uint32_t size, uint32_t attr)
{
	uint32_t s;
//	uint32_t data;
	
	volatile MPU_Type						*reg_mpu = (volatile MPU_Type*)MPU_BASE;

	if (addr & 0x1f || size < 32) {
//		lite_printf("addr must at least 32byte aligned, size >= 32: %08x %d\n", addr, size);
		return -1;
	}

	s = mpu_convert_size(size);

	//just_printf("size %d\n", s);
	__asm volatile( "dsb" );
	__asm volatile( "isb" );
	
//	data = addr | region | MPU_REGION_VALID;
	reg_mpu->RBAR =  addr | region | MPU_REGION_VALID;
//	data = reg_mpu->RBAR;
	
//	data = s << 1 | attr | MPU_REGION_ENABLE;
	reg_mpu->RASR = s << 1 | attr | MPU_REGION_ENABLE;
//	data = reg_mpu->RASR;
	
	return 0;
}

// ---------------------------------------------------------------------------
// FUNCTION: 
// INPUT   : 
// OUTPUT  : 
// RETURN  : 
// ---------------------------------------------------------------------------
int	config_mpu(void)
{
#if	0
//	mpu_config_region(0, 0x10000000, 512*1024, MPU_REGION_READ_WRITE| MPU_REGION_CACHEABLE_BUFFERABLE);

	mpu_config_region(1, 0x20080000, 8*1024, MPU_REGION_PRIVILEGED_READ_WRITE | MPU_REGION_CACHEABLE_BUFFERABLE);

	/* code space */
//	mpu_config_region(2, 0x20000000, 96*1024, MPU_REGION_READ_WRITE | MPU_REGION_CACHEABLE_BUFFERABLE);

	/* register space */
//	mpu_config_region(3, 0x40000000, 0x04000000, MPU_REGION_READ_WRITE);
#endif

#if	0
	mpu_config_region(0, 0x10000000, 512*1024, MPU_REGION_READ_WRITE | MPU_REGION_CACHEABLE_BUFFERABLE);

	mpu_config_region(1, 0x20080000, 8*1024, MPU_REGION_PRIVILEGED_READ_WRITE | MPU_REGION_CACHEABLE_BUFFERABLE);

	/* code space */
//	mpu_config_region(2, 0x20000000, 96*1024, MPU_REGION_READ_WRITE | MPU_REGION_CACHEABLE_BUFFERABLE);
	mpu_config_region(2, 0x20000000, 256*1024, MPU_REGION_READ_WRITE | MPU_REGION_CACHEABLE_BUFFERABLE);

	/* register space */
	mpu_config_region(3, 0x40000000, 0x04000000, MPU_REGION_READ_WRITE);
	
	/* system control */
//	mpu_config_region(4, 0xE0000000, 0x0100000, MPU_REGION_READ_WRITE);
#endif

#if	1
	mpu_config_region(0, 0x10000000, 1024*1024, MPU_REGION_READ_WRITE | MPU_REGION_CACHEABLE_BUFFERABLE);

	/* data space */
	mpu_config_region(1, 0x20000000, 256*1024, MPU_REGION_READ_WRITE | MPU_REGION_CACHEABLE_BUFFERABLE);
	
	mpu_config_region(2, 0x20080000, 8*1024, MPU_REGION_PRIVILEGED_READ_WRITE | MPU_REGION_CACHEABLE_BUFFERABLE);

	/* register space */
	mpu_config_region(3, 0x40000000, 0x04000000, MPU_REGION_READ_WRITE);
	
	/* system control */
//	mpu_config_region(4, 0xE0000000, 0x0100000, MPU_REGION_READ_WRITE);
#endif

//	mpu_enable(MPU_ENABLE | MPU_BGENABLE);


	return 0;
}

// ---------------------------------------------------------------------------
// FUNCTION: 
// INPUT   : 
// OUTPUT  : 
// RETURN  : 
// ---------------------------------------------------------------------------
void	setup_mpu( void )
{
//	return;
	
volatile mml_scbr_regs_t *preg_scbr = (volatile mml_scbr_regs_t *)SCB_BASE;
//volatile mml_icache_regs_t *preg_icache =(volatile mml_icache_regs_t*)MML_ICACHE_IOBASE;  
volatile MPU_Type	*reg_mpu = (volatile MPU_Type*)MPU_BASE;


//	preg_scbr->shcrs |= 0x00070000UL;	// enable all faults
//	preg_scbr->shcrs |= 0x00010000UL;	// enable memory manage fault
//	preg_icache->ctrlstat |=0x01;	
	
	__asm( "CPSID I" );
	
	config_mpu();
	
	preg_scbr->shcrs |= 0x00010000UL;	// enable memory manage fault
	
	__asm volatile( "dsb" );
	__asm volatile( "isb" );
	
	reg_mpu->CTRL =(MPU_ENABLE | MPU_BGENABLE); //Enable the MPU
	
	portSWITCH_TO_USER_MODE();
	
	__asm( "CPSIE I" );
	
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To indicate that system is already in PEDK (PRIVILEGE),
//	     therefore no system switching is needed.
// INPUT   : 
// OUTPUT  : 
// RETURN  : 
// ---------------------------------------------------------------------------
void	MPU_SwitchToPedkMode( void )
{
	mpu_in_pedk = 1;
}

// ---------------------------------------------------------------------------
// FUNCTION: To cancel PEDK mode.
// INPUT   : 
// OUTPUT  : 
// RETURN  : 
// ---------------------------------------------------------------------------
void	MPU_ClearPedkMode( void )
{
	mpu_in_pedk = 0;
}

// ---------------------------------------------------------------------------
// FUNCTION: Switch system to SUPER mode.
// INPUT   : id - ID for system switch.
// OUTPUT  : 
// RETURN  : 
// ---------------------------------------------------------------------------
void	MPU_SwitchToSuperMode( UINT32 id )
{
#ifdef	_MPU_ENABLED_
	
	if( mpu_in_pedk )
	  return;
	  
	if( id != SVCID )
	  return;
	  
//	mpu_access_flag = 1;

	if( mpu_access_flag )
	  {
	  mpu_access_flag++;
	  return;
	  }
	else
	  mpu_access_flag++;

	// __asm( "svc	3" );
	
#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: Switch system to USER mode.
// INPUT   : 
// OUTPUT  : 
// RETURN  : 
// ---------------------------------------------------------------------------
void	MPU_SwitchToUserMode( void )
{
#ifdef	_MPU_ENABLED_

	if( mpu_in_pedk )
	  return;
	  
//	mpu_access_flag = 0;
	
	if( --mpu_access_flag == 0 )
	  portSWITCH_TO_USER_MODE();

#endif
}

// ---------------------------------------------------------------------------
// FUNCTION: 
// INPUT   : 
// OUTPUT  : 
// RETURN  : 
// ---------------------------------------------------------------------------
void	MPU_HardFaultHandler( void )
{
	if( mpu_access_flag )
	  {
	  portSWITCH_TO_SUPER_MODE();
	  }
	else
	  {
//	  LIB_LCD_PutMsg( 0, 0, FONT0+attrCLEARWRITE+attrREVERSE, strlen("HardFault"), "HardFault" );
//	  for(;;);
	  
	  LIB_BUZ_MPU_HardFault();
	  }
	  
	return;
}

// ---------------------------------------------------------------------------
// FUNCTION: 
// INPUT   : 
// OUTPUT  : 
// RETURN  : 
// ---------------------------------------------------------------------------
void	MPU_MemManageHandler( void )
{
	if( mpu_access_flag )
	  {
	  portSWITCH_TO_SUPER_MODE();
	  }
	else
	  {
//	  LIB_LCD_PutMsg( 0, 0, FONT0+attrCLEARWRITE+attrREVERSE, strlen("MemManage"), "MemManage" );
//	  for(;;);
	  
	  LIB_BUZ_MPU_MemManage();
	  }

	return;
}

