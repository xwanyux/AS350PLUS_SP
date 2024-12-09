




#define ICC1 0x00
#define SAM1 0x01
#define SAM2 0x02
#define SAM3 0x03
#define SAM4 0x04




#define SAM_STATUS_CLOSE                0x00
#define SAM_STATUS_INIT_FIN             0x01
#define SAM_STATUS_RESET_FIN            0x02
#define SAM_STATUS_PROTOCOL_INIT_FIN    0x03




typedef struct Driver_Timing_DATA_T
{
    unsigned int protocol;
    unsigned int cwt;
    unsigned int bwt;
    unsigned int cgt;
    unsigned int bgt;
    unsigned int wwt;

}Driver_Timing_DATA;

