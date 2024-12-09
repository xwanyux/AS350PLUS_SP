#include "POSAPI.h"
#include "USBAPI.h"

UCHAR	api_usb_open( UCHAR port, API_USB_PARA para )
{
	port=port;
	para=para;
	return apiOutOfService;
}

UCHAR	api_usb_close( UCHAR dhn )
{
	dhn=dhn;
	return apiFailed;
}

UCHAR	api_usb_rxready( UCHAR dhn, UCHAR *dbuf )
{
	dhn=dhn;
	dbuf=dbuf;
	return apiFailed;
}

UCHAR	api_usb_rxstring( UCHAR dhn, UCHAR *dbuf )
{
	dhn=dhn;
	dbuf=dbuf;
	return apiFailed;
}

UCHAR	api_usb_txready( UCHAR dhn )
{
	dhn=dhn;
	return apiFailed;
}

UCHAR	api_usb_txstring( UCHAR dhn, UCHAR *sbuf )
{
	dhn=dhn;
	sbuf=sbuf;
	return apiFailed;
}

