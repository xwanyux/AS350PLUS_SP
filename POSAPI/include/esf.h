#ifndef __CNXT_ESF_H__
#define __CNXT_ESF_H__


/*
 * ESF data types
 */
typedef const char * 	LPCSTR;
typedef int	 				ESF_BOOL;
typedef void * 			HTHREAD;
typedef void * 			HLOCK;
typedef void * 			PVOID;
typedef void * 			HOSTIMER;
typedef void * 			HOSEVENT;
typedef char * 			HANDLE;
typedef unsigned long *	PUINT32;
typedef void  			(* PCBFUNC)( PVOID );

typedef enum
{
   MODEM_OK,
   MODEM_ERROR,
   MODEM_OTHERS
} MODEM_STATUS;

typedef enum
{
   MODEM_IOCTL_NONE,
   MODEM_IOCTL_DTR,
   MODEM_IOCTL_RTS,
   MODEM_IOCTL_RX_AVAIL,
   MODEM_IOCTL_TX_FREE
} MODEM_IOCTL_CODE;



/* 
 * Modem Callback Events/Status:
 *
 * These defines are used by the modem driver to notifiy the host functions
 * of event(s) that have been generated in the modem.
 *
 * Note:  These events are bit-mapped, and one or more events may be indicated
 * in a single event callback.
 *
 * Note:  Not every Conexant Modem will implement every event
 */
#define  CNXT_MODEM_EVT_RXCHAR     0x00000001  /* Any Character received */
#define  CNXT_MODEM_EVT_RXFLAG     0x00000002  /* Received certain character */
#define  CNXT_MODEM_EVT_TXEMPTY    0x00000004  /* Transmit Queue Empty */
#define  CNXT_MODEM_EVT_CTS        0x00000008  /* CTS changed state */
#define  CNXT_MODEM_EVT_DSR        0x00000010  /* DSR changed state */
#define  CNXT_MODEM_EVT_RLSD       0x00000020  /* RLSD changed state */
#define  CNXT_MODEM_EVT_BREAK      0x00000040  /* BREAK received */
#define  CNXT_MODEM_EVT_ERR        0x00000080  /* Line status error occurred */
#define  CNXT_MODEM_EVT_RING       0x00000100  /* Ring signal detected */
#define  CNXT_MODEM_EVT_CTSS       0x00000400  /* CTS state */
#define  CNXT_MODEM_EVT_DSRS       0x00000800  /* DSR state */
#define  CNXT_MODEM_EVT_RLSDS      0x00001000  /* RLSD state */
#define  CNXT_MODEM_EVT_PARITY_ERR 0x00002000  /* Parity Error occured */
#define  CNXT_MODEM_EVT_TXCHAR     0x00004000  /* Any character transmitted */
#define  CNXT_MODEM_EVT_RXOVRN     0x00030000  /* Rx buffer overrun detected */
#define  CNXT_MODEM_EVT_CID        0x00100000  /* CALLER ID Detected */



/*
 * Soft Modem API
 */
//extern MODEM_STATUS cnxt_modem_create( void );
//extern MODEM_STATUS cnxt_modem_destroy( void );
//extern MODEM_STATUS cnxt_modem_open( void (*pfnCallback)(unsigned int) );
//extern MODEM_STATUS cnxt_modem_close( void );
extern MODEM_STATUS cnxt_modem_read( char* buf, unsigned int* pcount );
extern MODEM_STATUS cnxt_modem_write( const char* buf, unsigned int* pcount );
extern MODEM_STATUS cnxt_modem_ioctl( const MODEM_IOCTL_CODE code, int* pctl );
extern MODEM_STATUS	cnxt_modem_write_atc( const char *buf, unsigned int *pcount, ULONG delay );
extern void NosSleep(unsigned int DelayTime);


#endif

