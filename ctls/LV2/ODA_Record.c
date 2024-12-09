#include <string.h>
#include "ODA_Record.h"

unsigned char oda_bufRecord[ODA_BUFFER_SIZE_RECORD];
unsigned char oda_bufRspGAC[ODA_BUFFER_SIZE_APDU];
unsigned char oda_bufRspGPO[ODA_BUFFER_SIZE_APDU];


void ODA_Clear_Record(void)
{
	memset(oda_bufRecord, 0, ODA_BUFFER_SIZE_RECORD);
}


void ODA_Clear_GACResponse(void)
{
	memset(oda_bufRspGAC, 0, ODA_BUFFER_SIZE_APDU);
}


void ODA_Clear_GPOResponse(void)
{
	memset(oda_bufRspGPO, 0, ODA_BUFFER_SIZE_APDU);
}


