








#ifndef MSRDEV_H
#define MSRDEV_H


#include <linux/ioctl.h>

typedef enum {
    FREE_RESOURCE  = 0,
    GET_RESOURCE = 1
} MODE_t;


// this is provide (this number may cause a bug, for now seem doesn't have the bug)
// need fix it
#define MAJOR_NUM 10


#define IOCTL_OPEN_TRACK       _IOWR(MAJOR_NUM, 0, unsigned char)


#define IOCTL_SWIPE            _IOR(MAJOR_NUM,  1, char)


#define IOCTL_GET_TRACK1_DATA  _IOWR(MAJOR_NUM, 2, char*)


#define IOCTL_GET_TRACK2_DATA  _IOWR(MAJOR_NUM, 3, char*)


#define IOCTL_GET_TRACK3_DATA  _IOWR(MAJOR_NUM, 4, char*)


#define IOCTL_SWITCH_MODE      _IOR(MAJOR_NUM,  5, MODE_t)

// #define IOCTL_SAM1_FIN_CHECK   _IOR(MAJOR_NUM,  6, char*)

// #define IOCTL_GET_SAM1_DATA    _IOWR(MAJOR_NUM, 7, char*)

// #define IOCTL_SAM2_FIN_CHECK   _IOR(MAJOR_NUM,  8, char*)

// #define IOCTL_GET_SAM2_DATA    _IOWR(MAJOR_NUM, 9, char*)

// #define IOCTL_SAM3_FIN_CHECK   _IOR(MAJOR_NUM,  10, char*)

// #define IOCTL_GET_SAM3_DATA    _IOWR(MAJOR_NUM, 11, char*)

// #define IOCTL_SAM4_FIN_CHECK   _IOR(MAJOR_NUM,  12, char*)

// #define IOCTL_GET_SAM4_DATA    _IOWR(MAJOR_NUM, 13, char*)






#endif