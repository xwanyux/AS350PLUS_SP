



#ifndef _DEV_IFM_H_
#define _DEV_IFM_H_

#include "bsp_types.h"
#include "POSAPI.h"






#define MODE_SYNCHRONOUS         0x01
#define MODE_ASYNCHRONOUS        0x00


#define SC_PHY_1_8V         0x01
#define SC_PHY_5V           0x02
#define SC_PHY_3V           0x03


#define TDA8026_BANK0_REGISTER_ADDRESS  0x24
#define TDA8026_BANK1_REGISTER0_ADDRESS 0x20
#define TDA8026_BANK1_REGISTER1_ADDRESS 0x21


// BANK1 Register0 SAM12 Read
#define PRES_BIT_POSITION_SAM12   0x01
#define PRESL_BIT_POSITION_SAM12  0x02
#define CLKSW_BIT_POSITION_SAM12  0x04
#define SUPL_BIT_POSITION_SAM12   0x08
#define PROT_BIT_POSITION_SAM12   0x10
#define MUTE_BIT_POSITION_SAM12   0x20
#define EARLY_BIT_POSITION_SAM12  0x40
#define ACTIVE_BIT_POSITION_SAM12 0x80


// BANK1 Reigister0 SAM12 Write
#define START_BIT_POSITION_SAM12   0x01
#define WARM_BIT_POSITION_SAM12    0x02
#define VN3OR5_BIT_POSITION_SAM12  0x04
#define PWDN_BIT_POSITION_SAM12    0x08
#define REGLOW_BIT_POSTION_SAM12   0x10
#define REGHIGH_BIT_POSITION_SAM12 0x20
#define IOEN_BIT_POSITION_SAM12    0x40
#define VCC1V8_BIT_POSITION_SAM12  0x80


// BANK1 Register1 MODE
#define BANK1_REGISTER1_00         0x00
#define BANK1_REGISTER1_01         0x01
#define BANK1_REGISTER1_10         0x02
#define BANK1_REGISTER1_11         0x03

// BANK1 Reigser1 MODE 00 (Read and Write are the same)
#define CLKDIVLOW_BIT_POSITION     0x01
#define CLKDIVHIGH_BIT_POSITION    0x02
#define CLKPDLOW_BIT_POSITION      0x04
#define CLKPDHIGH_BIT_POSITION     0x08
#define C4_1_BIT_POSITION          0x10
#define C8_1_BIT_POSITION          0x20
#define RSTIN_BIT_POSITION         0x40
#define CFGP2_BIT_POSITION         0x80 

// frequency mode
#define EXT_FREQUENCY              0x00
#define EXT_FREQUENCY_DEVIDE_2     0x01
#define EXT_FREQUENCY_DEVIDE_4     0x02
#define EXT_FREQUENCY_DEVIDE_5     0x03


// private funciton
UCHAR I2CWrite(UCHAR device_addr, UCHAR data);
UCHAR I2CRead(UCHAR device_addr, UCHAR* data);

// TDA8026 gneral operation to ICC1 , SAM1 , SAM2, SAM3, SAM4
// all of sam_id from range 1,2,3,4,5 map to ICC1, SAM1, SAM2, SAM3, SAM4

extern UCHAR Check_Active_SAM(UCHAR sam_id);
extern UCHAR Check_Present_SAM(UCHAR sam_id);
extern UCHAR Read_Status_Data_SAM(UCHAR sam_id);
extern UCHAR Disable_IO_SAM(UCHAR sam_id);
extern UCHAR Enable_IO_SAM(UCHAR sam_id);
extern UCHAR Start_SAM(UCHAR sam_id);
extern UCHAR Set_Voltage_SAM(UCHAR sam_id, UCHAR voltage);
extern UCHAR Deactivate_SAM(UCHAR sam_id);
extern UCHAR Change_frequency(UCHAR sam_id, UCHAR frequency_mode);
extern UCHAR Syn_Asyn_Selcetion_SAM12(UCHAR sam_id, UCHAR mode);
extern UCHAR WARM_RESET(UCHAR sam_id);


// other function related but not operate by TDA8026
extern void FAST_FIND_ENABLE();
extern void FAST_FIND_DISABLE();
extern void TDA_DC_TO_DC(UCHAR value);




#endif