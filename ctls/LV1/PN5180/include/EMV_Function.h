

extern UCHAR	EMV_ACTIVATE(void);
extern UCHAR	EMV_COLLISION_DETECTION(void);
extern void		EMV_MainLoop(void);
extern void		EMV_PcdFunction(void);
extern void		EMV_PcdFunction_FieldOn(void);
extern UCHAR	EMV_POLLING_Check_Collision(void);
extern UCHAR	EMV_POLLING_TypeA(void);
extern UCHAR	EMV_POLLING_TypeB(void);
extern void		EMV_Reset_PcdParameter(void);
extern void		EMV_RESET(void);
extern void		EMV_TransactionSendApplication(void);
extern void		EMV_WAIT(void);

