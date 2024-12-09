#include <stdio.h>
#include <unistd.h>
#include "bsp_types.h"
void NosSleep(UINT16 DelayTime)//sleep DelayTime*10 ms
{
	usleep(DelayTime*10000);
}
