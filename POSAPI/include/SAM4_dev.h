


#ifndef MSRDEV_H
#define MSRDEV_H


#include <linux/ioctl.h>

typedef enum {
    FREE_RESOURCE  = 0,
    GET_RESOURCE = 1
} MODE_t;

typedef struct tx{
    int len;
    unsigned char* buffer;
} tx_t;


// this is provide (this number may cause a bug, for now seem doesn't have the bug)
// need fix it
#define MAJOR_NUM 10



#define IOCTL_SWITCH_MODE      _IOR(MAJOR_NUM,  0, MODE_t)

#define IOCTL_SAM1_FIN_CHECK   _IOR(MAJOR_NUM,  1, char)

#define IOCTL_GET_SAM1_DATA    _IOWR(MAJOR_NUM, 2, char*)

#define IOCTL_SAM2_FIN_CHECK   _IOR(MAJOR_NUM,  3, char)

#define IOCTL_GET_SAM2_DATA    _IOWR(MAJOR_NUM, 4, char*)

#define IOCTL_SAM3_FIN_CHECK   _IOR(MAJOR_NUM,  5, char)

#define IOCTL_GET_SAM3_DATA    _IOWR(MAJOR_NUM, 6, char*)

#define IOCTL_SAM4_FIN_CHECK   _IOR(MAJOR_NUM,  7, char)

#define IOCTL_GET_SAM4_DATA    _IOWR(MAJOR_NUM, 8, char*)

 #define IOCTL_SAM1_TX_DATA     _IOR(MAJOR_NUM,  9, tx_t)

 #define IOCTL_SAM2_TX_DATA     _IOR(MAJOR_NUM,  10, tx_t)

 #define IOCTL_SAM3_TX_DATA     _IOR(MAJOR_NUM,  11, tx_t)

#define IOCTL_SAM4_TX_DATA     _IOR(MAJOR_NUM,  12, tx_t)




#endif