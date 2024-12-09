extern ULONG	OS_GET_SysTimerFreeCnt( void );
extern void		OS_SET_SysTimerFreeCnt( ULONG value );

extern ULONG	OS_GET_KbdEventFlag( void );
extern void		OS_SET_KbdEventFlag( ULONG value );

#ifdef _PLATFORM_AS350
extern ULONG	OS_GET_ScEventFlag( void );
extern void		OS_SET_ScEventFlag( ULONG value );

extern ULONG	OS_GET_MsrEventFlag( void );
extern void		OS_SET_MsrEventFlag( ULONG value );
#endif

