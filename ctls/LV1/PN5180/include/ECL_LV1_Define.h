#ifndef FALSE
#define FALSE	0
#endif

#ifndef TRUE
#define TRUE	1
#endif

#ifndef NULLPTR
#define NULLPTR	(void *) 0
#endif

#define ECL_LV1_FAIL				0x00U
#define ECL_LV1_SUCCESS				0x01U
#define ECL_LV1_POWEROFF			0xC0U
#define	ECL_LV1_STOP_NOW			0xE0U
#define	ECL_LV1_STOP_LATER			0xE1U
#define	ECL_LV1_STOP_CANCEL			0xE2U
#define	ECL_LV1_STOP_ICCARD			0xE3U
#define	ECL_LV1_STOP_MAGSTRIPE		0xE4U
#define	ECL_LV1_TIMEOUT_ISO			0xF0U
#define	ECL_LV1_TIMEOUT_USER		0xF1U
#define	ECL_LV1_ERROR				0xF2U
#define	ECL_LV1_RESTARTRX			0xF3U
#define	ECL_LV1_COLLISION			0xF4U
#define	ECL_LV1_ERROR_PROTOCOL		0xF5U
#define	ECL_LV1_ERROR_INTEGRITY		0xF6U
#define	ECL_LV1_ERROR_TRANSMISSION	0xF7U

