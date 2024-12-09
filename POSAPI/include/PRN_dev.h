 


#ifndef PRNDEV_H
#define PRNDEV_H


#include <linux/ioctl.h>

typedef enum {
    FREE_RESOURCE  = 0,
    GET_RESOURCE = 1
} MODE_t;
 
typedef struct tx{
    int len;
    unsigned char* buffer;
} tx_t;


// this is provide 
#define MAJOR_NUM 10



#define IOCTL_SWITCH_MODE      _IOR(MAJOR_NUM,  0, MODE_t)

#define IOCTL_PRN_ISR_CHECK   _IOR(MAJOR_NUM,  1, char)






#endif
