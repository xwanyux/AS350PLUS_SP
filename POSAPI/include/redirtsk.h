#ifndef __REDIRTSK_H__
#define __REDIRTSK_H__

#include "esf.h"
#ifdef BSP_OS_IS_NOS
	#include "nos.h"


	/*
	 * Redirtsk Task priorities when running with NOS:
	 */
	#define PERIODIC_PRIO					5
	#define SAMPLE_PRIO						1
	#define BLAM_PRIO							0xFF	// Not Used
	#define PORT_MON_PRIO					2
	#define MODEM_WRITE_PRIO				4
	#define MODEM_READ_PRIO					3
	#define mdmOpen_PRIO                    6           //New Add

	#define OS_SLEEP					    NosSleep
	#define OS_DELETE_TASK					NosDelete
	#define OS_SUSPEND_TASK					NosSuspend
	#define OS_RESUME_TASK					NosResume
	#define OS_CUR_TASK						pCurTask
	#define OS_TICKS							NosTickCounter
	#define TASK_PARAMS						void
	typedef NOS_TASK							OS_TASK;
#endif


#ifdef BSP_OS_IS_NUCLEUS
	#include "nucleus.h"

	/*
	 * Redirtsk Task priorities when running with Nucleus:
	 */
	#define PERIODIC_PRIO					64
	#define SAMPLE_PRIO						32
	#define BLAM_PRIO							0xFF	// Not Used
	#define PORT_MON_PRIO					60
	#define MODEM_WRITE_PRIO				62
	#define MODEM_READ_PRIO					61

	#define OS_SLEEP							NU_Sleep
	#define OS_DELETE_TASK					NU_Delete_Task
	#define OS_SUSPEND_TASK					NU_Suspend_Task
	#define OS_RESUME_TASK					NU_Resume_Task
	#define OS_CUR_TASK						NU_Current_Task_Pointer()
	#define OS_TICKS							NU_Retrieve_Clock()
	#define TASK_PARAMS						UNSIGNED argc, void* pData 
	typedef NU_TASK							OS_TASK;
#endif

#ifdef BSP_OS_IS_LINUX
	#include "linuxtask.h"

	/*
	 * Redirtsk Task priorities when running with Linux:
	 */
	#define PERIODIC_PRIO					1
	#define SAMPLE_PRIO						0
	#define BLAM_PRIO							0xFF	// Not Used
	#define PORT_MON_PRIO					2
	#define MODEM_WRITE_PRIO				4
	#define MODEM_READ_PRIO					3

	#define OS_SLEEP							LinuxSleep
	#define OS_DELETE_TASK					LinuxDelete
	#define OS_SUSPEND_TASK					LinuxSuspend
	#define OS_RESUME_TASK					LinuxResume
	#define OS_CUR_TASK						LinuxCurrent()
	#define OS_TICKS							LinuxTickCounter
	#define TASK_PARAMS						void
	typedef LINUX_TASK						OS_TASK;
#endif

#define TASK_STACK_SIZE						4096				/* Size of task stacks */


///////////////////////////////////////////////////// MOVE to OS Services.h
typedef enum WAIT_RESULT_ENUM
{
	WAIT_OK,
	WAIT_TIMEOUT,
	WAIT_ERROR
} WAIT_RESULT;

typedef enum _TASK_INDEX_T
{
	TASK_OSPERIODICTIMER,
	TASK_IASAMPLEPROCESS,
	TASK_BLAM,
	TASK_PORT_MONITOR,
	TASK_WRITE_TO_MODEM,
	TASK_READ_FROM_MODEM,
	TASK_mdmOpen          //New Add
} TASK_INDEX_T;


// typedef PVOID (*PALLOC_FUNC)
// (	
	// unsigned 	/* Alloc Size */, 
	// PVOID 		/* pRefData */
// );
// typedef void (*PFREE_FUNC)
// (		
	// PVOID 		/* Block Pointer */,
	// PVOID 		/* pRefData */
// );

//extern void modem_task_create();

// extern HTHREAD OsCreateTask( void *pModemFunc, UINT32 argc, void *argv, UINT32 stack_size, UINT8 priority, TASK_INDEX_T task_id );
// extern void OsDestroyTask( HTHREAD hThread );
//////////////////////////////////////////////////////////////////////////////
#ifdef BSP_OS_IS_LINUX
	#define DEBUGPRINTF( args, ... ) 	printk( (args), ... )
#else
	extern void DEBUGPRINTF( char * Msg, ... );
#endif
//#define MDM_DEBUG 1


#endif /*__REDIRTSK_H__*/

