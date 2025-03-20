


#include "POSAPI.h"
#include "OS_PROCS.h"
#include "OS_SECM.h"

#include <stdio.h>
#include <stdlib.h> /*for system function*/
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
// #include <pthread.h>
#if	0
#ifdef _4G_ENABLED_
#include <net/if.h>
#endif
#endif

#define MAX_BUFFER_SIZE 65536
#define TIMEOUT_ERROR_CODE 11

#define LAN_MAX_CONNECTION 			8	// 2023-03-14, reduce max connection number from 10 to 8 for DHN range (0x98..0x9F)
#define LAN_CONNECTION_SECTIME_MAX		10*60	// 2023-10-31, max idle session time in seconds (10 min)

LAN_SOCKET  LAN_Connection_List[LAN_MAX_CONNECTION]={0};
UCHAR	    os_DHN_LAN = 0;
int         g_socket = 0;
API_LAN     g_lan;
UCHAR       rx_first_time_flag = 0;
UCHAR       LAN_open_have_called_flag = 0;
UCHAR       RxreadyFlag = 0;//20210817 added by west to prevent from double minus length.
ULONG	    os_LAN_SEC_TICK = 0;

// UCHAR       g_DHCP_FALG = FALSE;
// UCHAR       flag_stop = FALSE;
// UCHAR       thread_create = FALSE;
// pthread_t   DHCP_thread;

UCHAR receive_buffer[MAX_BUFFER_SIZE];
int   receive_len;
ULONG receive_len2=0;//for count DSS incomming byte.
extern ULONG     DownReceiveSize;

char* command_ipconfig = "ifconfig eth0 ";
char* command_netmask = " netmask ";
char* command_setGateWay = "route add default gw ";

char* command_check_physical_connection1 = "grep \"\" /sys/class/net/eth0/operstate";
char* command_check_physical_connection2 =  "grep \"\" /sys/class/net/eth0/carrier";

// this command to send the DHCP request, if it fail the file will not exit, if succed will get the file
char* command_DHCP = "start-stop-daemon --start --quiet --name udhcpc"
                     " --startas /sbin/udhcpc -- -p /var/run/udhcpc-eth0.pid -n -R";
char* command_check_succ_DHCP = "cat /var/run/udhcpc-eth0.pid";
char* command_killudhcpc = "killall udhcpc";

// default 5 times
char* command_ping = "ping -c 1 ";

char* command_getGateWay = "ip route | grep default | awk '{print $3}'"; // xxx.xxx.xxx.xxx
char* command_gateMAC = "ifconfig | grep eth0 | awk '{print $5}'"; // 3E:7A:8D:92:55:7A
// char* command_getIPAddress = "ifconfig eth0 | grep inet | awk '{print $2}'"; // will return "addr:xxx.xxx.xxx.xxx"
char* command_getIPAddress = "ifconfig eth0 | grep inet | grep -v inet6 | awk '{print $2}'"; // will return "addr:xxx.xxx.xxx.xxx"
char* command_getMASK = "ifconfig eth0 | grep inet | awk '{print $4}'"; //will return "Mask:xxx.xxx.xxx.xxx"
char* command_obtainIPAddress = "dhclient eth0";    //obtain the IP configuration information from the DHCP server
char* command_releaseIPAddress = "dhclient -r eth0";    //release the IP configuration information from the interface
#ifdef _build_DSS_
extern UCHAR	DSS_APPbuffer[DSS_MAX_APP_SIZE];//DSS 10MB buffer 
#endif

// DSS parameters defined in Down.c
extern	UCHAR	DssTelNum[23+1];		
extern	UCHAR	DssRemoteIP[23+1];	
extern	UCHAR	DssRemotePort[23+1];
extern	UCHAR	DssPort[23+1];
extern  UCHAR   DssIP[23+1];
extern  UCHAR   DssGateway[23+1];
extern  UCHAR   DssSubNetMask[23+1];

ULONG	OS_LAN_CheckConnectionTimeout( UCHAR dhn );


// ---------------------------------------------------------------------------
// FUNCTION: To check if DHN matched.
// INPUT   : dhn
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UCHAR	LAN_CheckDHN( UCHAR dhn )
{
	dhn -= 0x80;	// 2023-03-14
	
	if( (dhn >=psDEV_LAN)&&(dhn < (psDEV_LAN+LAN_MAX_CONNECTION)))
    {
        if(LAN_Connection_List[dhn-psDEV_LAN].socketFD>0)
	        return( TRUE );
    }
	else
    {
        // printf("LAN_CheckDHN failed dhn=%d\n",dhn);
        return( FALSE );
    }
	  
}




/**
 *     This function is to set the IP, gateway , netmask.
 *     @param[in] config            necessary information to set IP, gateway, netmask
 *     @retval apiOK                always success
 *     @note this function is just implement the linux command to do this for us
 *           ifconfig eht0 192.168.0.20 netmask 255.255.255.0  set ip and mask
 *           route add default gw 192.168.0.1  set gateway
 */ 
UCHAR api_lan_setIPconfig( API_IPCONFIG config ){

    FILE *fp;
    UCHAR cmd[60] = {0};
    UCHAR cursor = 0;
    UCHAR len,i;
    
    // this part the contruct ifconfig eht0 xxx.xxx.xxx.xxx netmask xxx.xxx.xxx.xxx
    len = strlen(command_ipconfig);
    memmove(cmd, command_ipconfig, len);
    cursor += len;

    memmove(&cmd[cursor],config.IP, config.IP_Len);
    cursor += config.IP_Len;

    len = strlen(command_netmask);
    memmove(&cmd[cursor],command_netmask,len);
    cursor += len;

    memmove(&cmd[cursor],config.SubnetMask,config.SubnetMask_Len);
    // debug only show the combine string
    //printf("%s\n",cmd);

    fp = popen(cmd, "r");
    // if(fp == NULL)
    //     printf("fp is NULL\n");
    // else
    //     printf("fp is not NULL\n");
    
    pclose(fp);


    len = 0;
    cursor = 0;

    // reset
    memset(cmd,0,60);

    // this part is to construt route add default gw xxx.xxx.xxx.xxx

    len = strlen(command_setGateWay);
    memmove(cmd,command_setGateWay, len);
    cursor += len;

    memmove(&cmd[cursor], config.Gateway, config.Gateway_Len);

    //printf("%s\n", cmd);


    fp = popen(cmd, "r");

    // if(fp == NULL)
    //     printf("fp is NULL\n");
    // else
    //     printf("fp is not NULL\n");
    pclose(fp);

    DssIP[0] = config.IP_Len;
    memcpy(&DssIP[1], config.IP, config.IP_Len);

    DssGateway[0] = config.Gateway_Len;
    memcpy(&DssGateway[1], config.Gateway, config.Gateway_Len);

    DssSubNetMask[0] = config.SubnetMask_Len;
    memcpy(&DssSubNetMask[1], config.SubnetMask, config.SubnetMask_Len);

    POST_LoadDssVariables(1);
    PED_SetStateDHCP(0);

    return apiOK;
}


/**
 *  This function is to get back the IP, gateway, netmask, MAC.
 *  @param[out] config          the data structure can contain IP, gatway, netmask MAC
 *  @retval apiOK               always success
 *  @note this function is implement the linux command , with more complex linux command.
 */ 
UCHAR api_lan_getIPconfig( API_IPCONFIG *config ){

    FILE *fp;
    UCHAR response[30] = {0};
    UCHAR len;
    UCHAR i,cursor = 0,temp;

    // get IP
    fp = popen(command_getIPAddress, "r");
    fgets(response, 30, fp);
    pclose(fp);
    // output format addr:xxx.xxx.xxx.xxx
    if(strlen(response) != 0){
        if(strlen(response) < 6)
        {
            POST_LoadDssVariables(0);

            config->IP_Len = DssIP[0];
            memmove(config->IP, &DssIP[1], DssIP[0]);

            config->Gateway_Len = DssGateway[0];
            memmove(config->Gateway, &DssGateway[1], DssGateway[0]);

            config->SubnetMask_Len = DssSubNetMask[0];
            memmove(config->SubnetMask, &DssSubNetMask[1], DssSubNetMask[0]);

            return apiOK;
        }

    //  len = strlen(response) - 5;
        len = strlen(response) - 5 - 1;	// JAMES, ignore last 0x0a
        config->IP_Len = len;
        memmove(config->IP,&response[5],len);
    }
    else
    {
        // config->IP_Len = 0;

        POST_LoadDssVariables(0);

        config->IP_Len = DssIP[0];
        memmove(config->IP, &DssIP[1], DssIP[0]);

        config->Gateway_Len = DssGateway[0];
        memmove(config->Gateway, &DssGateway[1], DssGateway[0]);

        config->SubnetMask_Len = DssSubNetMask[0];
        memmove(config->SubnetMask, &DssSubNetMask[1], DssSubNetMask[0]);

        return apiOK;
    }

    //get mask
    fp = popen(command_getMASK, "r");
    fgets(response, 30, fp);
    pclose(fp);
    // output format Mask:xxx.xxx.xxx.xxx
    if(strlen(response) != 0){
    //  len = strlen(response) - 5;
        len = strlen(response) - 5 - 1;	// ignore last 0x0a
        config->SubnetMask_Len = len;
        memmove(config->SubnetMask,&response[5],len);
    }
    else
        config->SubnetMask_Len = 0;

    // get GateWay
    fp = popen(command_getGateWay, "r");
    fgets(response, 30, fp);
    pclose(fp);
//  len = strlen(response);
    len = strlen(response)-1;	// ignore last 0x0a
    config->Gateway_Len = len;
    memmove(config->Gateway, response, len);

    // get MAC ADDRESS
    fp = popen(command_gateMAC, "r");
    fgets(response, 30, fp);
    pclose(fp);
    len = strlen(response);
    // 3E:7A:8D:92:55:7A
    for(i = 0; i <6; i++){
        config->MAC[i] = 0;
        temp = response[cursor];
        if(temp >= '0' && temp <= '9')
            config->MAC[i] += (temp -'0');
        else
            config->MAC[i] += (temp - 'A' + 10);
        
        config->MAC[i] *= 16;

        temp = response[cursor+1];
        if(temp >= '0' && temp <= '9')
            config->MAC[i] += (temp -'0');
        else
            config->MAC[i] += (temp - 'A' + 10);

        cursor += 3;
    }

    return apiOK;

}

// ---------------------------------------------------------------------------
// FUNCTION: To renew the IP, gateway, netmask.
// INPUT   : none. 
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void api_lan_renewIPconfig(void)
{
    API_IPCONFIG ipconfig;

    
    if(!PED_GetStateDHCP())
    {
        POST_LoadDssVariables(0);

        ipconfig.IP_Len = DssIP[0];
        memmove(ipconfig.IP, &DssIP[1], DssIP[0]);

        ipconfig.Gateway_Len = DssGateway[0];
        memmove(ipconfig.Gateway, &DssGateway[1], DssGateway[0]);

        ipconfig.SubnetMask_Len = DssSubNetMask[0];
        memmove(ipconfig.SubnetMask, &DssSubNetMask[1], DssSubNetMask[0]);

        api_lan_setIPconfig(ipconfig);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: To enable the ethernet device (LAN) and establish the link to remote host.
// INPUT   : API_LAN lan 
//	     
// OUTPUT  : none.
// RETURN  : DeviceHandleNo
//           apiOutOfService
// ---------------------------------------------------------------------------
UCHAR api_lan_open( API_LAN lan ){
UCHAR i;
struct timeval tv;

#if	0
#ifdef _4G_ENABLED_
    struct ifreq nif;//for bind ethernet interface to eth0
    char *inface = "eth0";
    strcpy(nif.ifr_name, inface);
#endif
#endif

//    printf("\napi_lan_open()\n");

    if(!LAN_open_have_called_flag)
    {
        for(i=0;i<LAN_MAX_CONNECTION;i++)
            memset(&LAN_Connection_List[i],0,sizeof(LAN_SOCKET));//initialize
        LAN_open_have_called_flag=1;
    }
        
    for(i=0;i<LAN_MAX_CONNECTION;i++){
		if(LAN_Connection_List[i].socketFD==0)//search if there have free connection to use.
			break;
		
		if(i==LAN_MAX_CONNECTION-1)//if all connection in used
			return apiOutOfService;
	}
int result;
int sock;
UCHAR buffer[30];
struct sockaddr_in serv_addr;
struct timespec now;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    // printf("sock:%d\n",sock);
#if	0
#ifdef _4G_ENABLED_
    setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (char *)&nif, sizeof(nif));
#endif
#endif

    tv.tv_sec = 0;
    tv.tv_usec = 500000; // recv timeout 0.5s
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // for connect to timeout
    tv.tv_sec = 7; // tx time out with 7 sec
    tv.tv_usec = 0; 
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);

    // use for passing string 
    memset(buffer,0,sizeof(buffer));
    memset(&serv_addr, 0 , sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // using IPV4
    memmove(buffer,lan.ServerIP,lan.ServerIP_Len);
    serv_addr.sin_addr.s_addr = inet_addr(buffer);
//    printf("%s\n",buffer);
    serv_addr.sin_port = htons(lan.ServerPort);
//    printf("%d\n",lan.ServerPort);

    //printf("start to connect\n");

    result = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    clock_gettime(CLOCK_MONOTONIC, &now);

    if(result < 0){
        perror("Error:");
        close(sock);
        return apiOutOfService;
    }
//    printf("connected time=%d\n",now.tv_sec);
    LAN_Connection_List[i].socketFD=sock;
    LAN_Connection_List[i].APIconfig=lan;
    LAN_Connection_List[i].ConnectTime=now.tv_sec;
    
    LAN_Connection_List[i].RFU = 0x0001;	// active
    
//  os_DHN_LAN = psDEV_LAN +i;
    os_DHN_LAN = psDEV_LAN +i + 0x80;	// 2023-03-14

    receive_len=0;
    return (os_DHN_LAN);
}

// ---------------------------------------------------------------------------
// FUNCTION: To enable the ethernet device (LAN) and establish the link to remote host.
// INPUT   : API_LAN lan (DEBUG TEST ONLY)
//	     
// OUTPUT  : none.
// RETURN  : DeviceHandleNo
//           apiOutOfService
// ---------------------------------------------------------------------------
UCHAR api_lan_open2( API_LAN lan )
//UCHAR	api_lan_open2()
{
UCHAR i;
struct timeval tv;

int result;
int sock;
UCHAR buffer[30];
struct sockaddr_in serv_addr;
struct timespec now;


    if(!LAN_open_have_called_flag)
    {
        for(i=0;i<LAN_MAX_CONNECTION;i++)
            memset(&LAN_Connection_List[i],0,sizeof(LAN_SOCKET));//initialize
        LAN_open_have_called_flag=1;
    }
        
    for(i=0;i<LAN_MAX_CONNECTION;i++){
		if(LAN_Connection_List[i].socketFD==0)//search if there have free connection to use.
			break;
		
		if(i==LAN_MAX_CONNECTION-1)//if all connection in used
			return apiOutOfService;
	}

    sock = socket(AF_INET, SOCK_STREAM, 0);

    // printf("sock:%d\n",sock);

    tv.tv_sec = 0;
    tv.tv_usec = 500000; // recv timeout 0.5s
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    // for connect to timeout
    tv.tv_sec = 7; // tx time out with 7 sec
    tv.tv_usec = 0; 
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);

    // use for passing string 
    memset(buffer,0,sizeof(buffer));
    memset(&serv_addr, 0 , sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // using IPV4
    memmove(buffer,lan.ServerIP,lan.ServerIP_Len);
    serv_addr.sin_addr.s_addr = inet_addr(buffer);
//    printf("%s\n",buffer);
    serv_addr.sin_port = htons(lan.ServerPort);
//    printf("%d\n",lan.ServerPort);

    //printf("start to connect\n");

    result = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    clock_gettime(CLOCK_MONOTONIC, &now);

    if(result < 0){
        perror("Error:");
        close(sock);
        return apiOutOfService;
    }
//    printf("connected time=%d\n",now.tv_sec);
    LAN_Connection_List[i].socketFD=sock;
    LAN_Connection_List[i].APIconfig=lan;
    LAN_Connection_List[i].ConnectTime=now.tv_sec;
    
    LAN_Connection_List[i].RFU = 0x0001;	// active
    
//  os_DHN_LAN = psDEV_LAN +i;
    os_DHN_LAN = psDEV_LAN +i + 0x80;	// 2023-03-14

    receive_len=0;
    return (os_DHN_LAN);

//	printf("\napi_lan_open2()\n");
//	return( apiOutOfService );
}

// ---------------------------------------------------------------------------
// FUNCTION: To disable the LAN device and disconnect the link to host.
// INPUT   : dhn
//	     The specified device handle number.
//	     0x00 = to close all opened tasks.
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// NOTE	   : by using api_lan_satatus(dhn=0) to check current link status
//           to make sure the disconnection status, it will avoid corss-talk
//           with next link
// ---------------------------------------------------------------------------
UCHAR	api_lan_close( UCHAR dhn )
{
UCHAR	status;
UCHAR   result;
UCHAR LanNumber;
UINT    Socket;
	status = apiFailed;
	
	if( LAN_CheckDHN( dhn ) == TRUE )
	  {
	dhn -= 0x80;	// 2023-03-14
        LanNumber=dhn-psDEV_LAN;
        Socket=LAN_Connection_List[LanNumber].socketFD;  
        result = close(Socket);
	    if(result < 0)
            perror("Error:");
        else{
            memset(&LAN_Connection_List[LanNumber],0,sizeof(LAN_SOCKET));//initialize
            status = apiOK;
        }
	  
	}
	return( status );
}

// ---------------------------------------------------------------------------
// FUNCTION: To determine whether LAN is ready to transmit the next data string.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : none.
// RETURN  : apiReady
//           apiNotReady
//	     apiFailed
// ---------------------------------------------------------------------------

/*
    only check global socket connection, if it fail return apifailed 
*/
UCHAR	api_lan_txready( UCHAR dhn )
{
UCHAR LanNumber;
UINT    Socket;
    if( LAN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	  
    if( OS_LAN_CheckConnectionTimeout( dhn ) )
      return( apiFailed );
	  
    dhn -= 0x80;	// 2023-03-14
    LanNumber=dhn-psDEV_LAN;
    Socket=LAN_Connection_List[LanNumber].socketFD;
	int error = 0;
    socklen_t len = sizeof (error);
    int retval = getsockopt (Socket, SOL_SOCKET, SO_ERROR, &error, &len);   

    if (retval != 0) {
        /* there was a problem getting the error code */
        fprintf(stderr, "error getting socket error code: %s\n", strerror(retval));
        return apiNotReady;
    }

    if (error != 0) {
        /* socket has a non zero error status */
        fprintf(stderr, "socket error: %s\n", strerror(error));
        return apiNotReady;
    } 

    return apiReady;

}



// ---------------------------------------------------------------------------
// FUNCTION: To write the outgoing data string to LAN transmitting stream buffer.
// INPUT   : dhn
//           The specified device handle number.
//           sbuf
//           UINT  length ;     	// length of data string to be transmitted.
//	     UCHAR data[length] ;	// data string.	
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_lan_txstring( UCHAR dhn, UCHAR *sbuf )
{
int result;
ULONG len = sbuf[0] + sbuf[1]*256;
UCHAR len_transmit[2] = {0};
UCHAR LanNumber;
API_LAN LanConfig;
UINT    Socket;
struct timespec now;

    if( LAN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );

    
	if( len == 0 )	// PATCH: 2011-09-27
	  return( apiFailed );

    if( OS_LAN_CheckConnectionTimeout( dhn ) )
      return( apiFailed );
	  
    dhn -= 0x80;	// 2023-03-14
    LanNumber=dhn-psDEV_LAN;
    Socket=LAN_Connection_List[LanNumber].socketFD;
    LanConfig=LAN_Connection_List[LanNumber].APIconfig;
    if(LanConfig.LenType == 0){
        /*HEX*/
        len_transmit[1] = len & 0x00ff;
        len_transmit[0] = (len & 0xff00) >> 8;

    }
    else if(LanConfig.LenType == 1){
        /*BCD*/
        len_transmit[1] = len % 10;
        len_transmit[1] += (16* (len % 100 - len % 10) / 10);
        len_transmit[0] = (len % 1000 - len % 100) / 100;
        len_transmit[0] += (16* (len % 10000 - len % 1000) / 1000);

    }

    // this may have a bug if len is too large, maybe need another turn to send (not fix yet)
    if((LanConfig.LenType != 0)  && (LanConfig.LenType != 1))
        result = send(Socket, sbuf+2, len, 0);
    else{
        sbuf[0] = len_transmit[0];
        sbuf[1] = len_transmit[1];
        result = send(Socket, sbuf, len+2, 0);
    }

    if(result < 0){
        perror("Error:");
        return apiFailed;
    }
    else
    {
        clock_gettime(CLOCK_MONOTONIC, &now);
        LAN_Connection_List[LanNumber].ConnectTime=now.tv_sec;//if have OUTPUT activity, update idle time
        return apiOK;
    }
        

	// if( OS_LAN_TxString( sbuf ) == TRUE )
	//   return( apiOK );
	// else
	//   return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: (For DSS use)To write the outgoing data string to LAN transmitting stream buffer.
// INPUT   : dhn
//           The specified device handle number.
//           sbuf
//           UINT  length ;     	// length of data string to be transmitted.
//	     UCHAR data[length] ;	// data string.	
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_lan_txstring2( UCHAR dhn, UCHAR *sbuf )
{
int result;
ULONG len = sbuf[0] + sbuf[1]*256;
UCHAR len_transmit[2] = {0};
UCHAR LanNumber;
API_LAN LanConfig;
UINT    Socket;
struct timespec now;

    if( LAN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );

	if( len == 0 )	// PATCH: 2011-09-27
	  return( apiFailed );
	  
    if( OS_LAN_CheckConnectionTimeout( dhn ) )
      return( apiFailed );
	  
    dhn -= 0x80;	// 2023-03-14
    LanNumber=dhn-psDEV_LAN;
    Socket=LAN_Connection_List[LanNumber].socketFD;
    LanConfig=LAN_Connection_List[LanNumber].APIconfig;
    if(LanConfig.LenType == 0){
        /*HEX*/
        len_transmit[1] = len & 0x00ff;
        len_transmit[0] = (len & 0xff00) >> 8;

    }
    else if(LanConfig.LenType == 1){
        /*BCD*/
        len_transmit[1] = len % 10;
        len_transmit[1] += (16* (len % 100 - len % 10) / 10);
        len_transmit[0] = (len % 1000 - len % 100) / 100;
        len_transmit[0] += (16* (len % 10000 - len % 1000) / 1000);

    }

    result = send(Socket, sbuf+2, len, 0);

    if(result < 0){
        perror("Error:");
        return apiFailed;
    }
    else
    {
        clock_gettime(CLOCK_MONOTONIC, &now);
        LAN_Connection_List[LanNumber].ConnectTime=now.tv_sec;//if have OUTPUT activity, update idle time
        return apiOK;
    }

	// if( OS_LAN_TxString( sbuf ) == TRUE )
	//   return( apiOK );
	// else
	//   return( apiFailed );
}

// ---------------------------------------------------------------------------
// FUNCTION: To determine whether data is ready for input from the LAN device.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//           UINT  length ;     // length of the received data string.
// RETURN  : apiReady
//           apiNotReady
//	     apiFailed
// ---------------------------------------------------------------------------

/*
    for what I understand, this function to check rx have receive a complete data
    i use recv() return is 0 or not,
    if it's 0 mean finsh, else > 0 not finish

*/
UCHAR	api_lan_rxready( UCHAR dhn, UCHAR *dbuf )
{
UCHAR LanNumber;
API_LAN LanConfig;
UINT    Socket;
struct timespec now;

    if( LAN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	  
    if( OS_LAN_CheckConnectionTimeout( dhn ) )
      return( apiFailed );

    dhn -= 0x80;	// 2023-03-14
    LanNumber=dhn-psDEV_LAN;
    Socket=LAN_Connection_List[LanNumber].socketFD;
    LanConfig=LAN_Connection_List[LanNumber].APIconfig;

    char recv_buf[250];
    int size;

    if((size = recv(Socket, recv_buf, sizeof(recv_buf), 0)) > 0){
        //printf("rx_ready:1\n");
        rx_first_time_flag = 1;
        // 2 just for len of data
        // if already full, we just descard the rest of data
        if(receive_len + size + 2 < MAX_BUFFER_SIZE){

            if(LanConfig.LenType == 0 || LanConfig.LenType == 1){
                /*HEX or BCD type already provide length doesn't need to 2 to store additonal len*/
                memmove(&receive_buffer[receive_len], recv_buf, size);
            }
            else{
                memmove(&receive_buffer[2+receive_len], recv_buf, size);
            }

            receive_len += size;

        
        }
        return apiNotReady;
    }
    // By experiment, receive size equal 0 when sever close the socket, defnitly finish the rx
    else if(size == 0){
        // mean data actually finished transmit
        // printf("rx_ready:2\n");

        if(rx_first_time_flag != 0){

        // if((g_lan.LenType == 0) || (g_lan.LenType == 1)){
        //     // need to minus 2 for len byte
        //     receive_len = receive_len - 2;
        // }
        if(((LanConfig.LenType == 0) || (LanConfig.LenType == 1))&&(RxreadyFlag==0))
        {//20210817 modified by west
            // need to minus 2 for len byte
            receive_len = receive_len - 4;
            RxreadyFlag=1;
        }
        receive_buffer[0] = receive_len & 0x00ff;
        receive_buffer[1] = (receive_len & 0xff00) >> 8;
        *dbuf=receive_buffer[0];
        *(dbuf+1)=receive_buffer[1];
        
        }
        else
          return apiNotReady;
        clock_gettime(CLOCK_MONOTONIC, &now);
        LAN_Connection_List[LanNumber].ConnectTime=now.tv_sec;//if have INPUT activity, update idle time
        return apiReady;
    }
    else{

        // time out error code would be errno = 11
        //printf("rx_ready:3\n");
        //printf ("Error no is : %d\n", errno);
        if(errno == TIMEOUT_ERROR_CODE){
            if(rx_first_time_flag == 1){
                if(((LanConfig.LenType == 0) || (LanConfig.LenType == 1))&&(RxreadyFlag==0))
                {//20210817 added by west
                    // need to minus 2 for len byte
                    receive_len = receive_len - 2;
                    RxreadyFlag=1;
                }
                receive_buffer[0] = receive_len & 0x00ff;
                receive_buffer[1] = (receive_len & 0xff00) >> 8;
                *dbuf=receive_buffer[0];
                *(dbuf+1)=receive_buffer[1];
                // printf("receive_buffer[0]=%d\n",receive_buffer[0]); 
                clock_gettime(CLOCK_MONOTONIC, &now);
                LAN_Connection_List[LanNumber].ConnectTime=now.tv_sec;//if have INPUT activity, update idle time
                return apiReady;
            }
            else 
                return apiNotReady;
        }
        else{
            perror("recv");
            return apiFailed;
        }

       
    }
    
}
#ifdef _build_DSS_
// ---------------------------------------------------------------------------
// FUNCTION: To determine whether data is ready for input from the LAN device.
//           For DSS use because large of buffer size.(for DSS ubuntu version )
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : ULONG  length ;     // length of the received data string.
// RETURN  : apiReady
//           apiNotReady
//	     apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_lan_rxready2( UCHAR dhn, UCHAR *dbuf )
{
UCHAR LanNumber;
API_LAN LanConfig;
UINT    Socket;
struct timespec now;

    if( LAN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );

    if( OS_LAN_CheckConnectionTimeout( dhn ) )
      return( apiFailed );

    dhn -= 0x80;	// 2023-03-14
    LanNumber=dhn-psDEV_LAN;
    Socket=LAN_Connection_List[LanNumber].socketFD;
    LanConfig=LAN_Connection_List[LanNumber].APIconfig;

    char recv_buf[250];
    int size;

    if((size = recv(Socket, recv_buf, sizeof(recv_buf), 0)) > 0){
        //printf("rx_ready:1\n");
        rx_first_time_flag = 1;
        // 2 just for len of data
        // if already full, we just descard the rest of data
        if(receive_len2 + size + 2 < DSS_MAX_APP_SIZE)
        {
            if(LanConfig.LenType == 0 || LanConfig.LenType == 1){
                /*HEX or BCD type already provide length doesn't need to 2 to store additonal len*/
                memmove(&DSS_APPbuffer[receive_len2], recv_buf, size);
            }
            else{
                memmove(&DSS_APPbuffer[2+receive_len2], recv_buf, size);
            }

            receive_len2 += size;


        }
        return apiNotReady;
    }
    // By experiment, receive size equal 0 when sever close the socket, defnitly finish the rx
    else if(size == 0){
        // mean data actually finished transmit
        //if(rx_first_time_flag != 0){
        if((LanConfig.LenType == 0) || (LanConfig.LenType == 1)){
            // need to minus 6 for SOH(1B) length(4b) LRC(1B)
            // receive_len2 = receive_len2 - 6;
        }
        // printf("DSS_APPbuffer[0] =%d\n",DSS_APPbuffer[0]);
        // DSS_APPbuffer[1] = receive_len2 & 0xff;
        // DSS_APPbuffer[2] = ((receive_len2 >> 8 )  & 0xff) ;
        // DSS_APPbuffer[3] = ((receive_len2 >> 16 ) & 0xff) ;
        // DSS_APPbuffer[4] = ((receive_len2 >> 24 ) & 0xff) ;
        // receive_len2+=6;
        *dbuf     = receive_len2 & 0xff;
        *(dbuf+1) = ((receive_len2 >> 8 )  & 0xff) ;
        *(dbuf+2) = ((receive_len2 >> 16 ) & 0xff) ;
        *(dbuf+3) = ((receive_len2 >> 24 ) & 0xff) ;
        clock_gettime(CLOCK_MONOTONIC, &now);
        LAN_Connection_List[LanNumber].ConnectTime=now.tv_sec;//if have INPUT activity, update idle time
        return apiReady;
    }
    else{

        // time out error code would be errno = 11
        //printf("rx_ready:3\n");
        //printf ("Error no is : %d\n", errno);
        if(errno == TIMEOUT_ERROR_CODE){
            if(rx_first_time_flag == 1){

                DSS_APPbuffer[1] = receive_len2 & 0xff;
                DSS_APPbuffer[2] = ((receive_len2 >> 8 )  & 0xff) ;
                DSS_APPbuffer[3] = ((receive_len2 >> 16 ) & 0xff) ;
                DSS_APPbuffer[4] = ((receive_len2 >> 24 ) & 0xff) ;
                *dbuf     = DSS_APPbuffer[1];
                *(dbuf+1) = DSS_APPbuffer[2];
                *(dbuf+2) = DSS_APPbuffer[3];
                *(dbuf+3) = DSS_APPbuffer[4];
                clock_gettime(CLOCK_MONOTONIC, &now);
                LAN_Connection_List[LanNumber].ConnectTime=now.tv_sec;//if have INPUT activity, update idle time
                return apiReady;
            }
            else 
                return apiNotReady;
        }
        else{
            perror("recv");
            return apiFailed;
        }

        
    }
}
#endif
// ---------------------------------------------------------------------------
// FUNCTION: To determine whether data is ready for input from the LAN device.
//           For communicate with PC DSS JAVA version used(SOH+Len+MSG+LRC=1B+2B+xB+1B)
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//           UINT  length ;     // length of the received data string.
// RETURN  : apiReady
//           apiNotReady
//	     apiFailed
// ---------------------------------------------------------------------------
#ifdef _build_DSS_
UCHAR	api_lan_rxready3( UCHAR dhn, UCHAR *dbuf )
{
UCHAR LanNumber;
API_LAN LanConfig;
UINT    Socket;
struct timespec now;

    if( LAN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	  
    if( OS_LAN_CheckConnectionTimeout( dhn ) )
      return( apiFailed );

    dhn -= 0x80;	// 2023-03-14
    LanNumber=dhn-psDEV_LAN;
    Socket=LAN_Connection_List[LanNumber].socketFD;
    LanConfig=LAN_Connection_List[LanNumber].APIconfig;

    char recv_buf[2500];
    int size;

    if((size = recv(Socket, recv_buf, sizeof(recv_buf), 0)) > 0){
        //printf("rx_ready:1\n");
        rx_first_time_flag = 1;
        // 2 just for len of data
        // if already full, we just descard the rest of data
        if(receive_len2 + size < DSS_MAX_APP_SIZE){
            
            if(LanConfig.LenType == 0 || LanConfig.LenType == 1){
                /*HEX or BCD type already provide length doesn't need to 2 to store additonal len*/
                memmove(&DSS_APPbuffer[receive_len2], recv_buf, size);
            }
            else{
                memmove(&DSS_APPbuffer[2+receive_len2], recv_buf, size);
            }

            receive_len2 += size;
            DownReceiveSize=receive_len2;
        
        }
        return apiNotReady;
    }
    // By experiment, receive size equal 0 when sever close the socket, defnitly finish the rx
    else if(size == 0){
        // mean data actually finished transmit
        // printf("rx_ready:2\n");

        if(rx_first_time_flag != 0){

            *dbuf     = receive_len2 & 0xff;
            *(dbuf+1) = ((receive_len2 >> 8 )  & 0xff) ;
            *(dbuf+2) = ((receive_len2 >> 16 ) & 0xff) ;
            *(dbuf+3) = ((receive_len2 >> 24 ) & 0xff) ;    

        }
        else
          return apiNotReady;
        // return apiReady;
        return apiNotReady;
    }
    else{

        // time out error code would be errno = 11
        //printf("rx_ready:3\n");
        //printf ("Error no is : %d\n", errno);
        if(errno == TIMEOUT_ERROR_CODE){
            if(rx_first_time_flag == 1){
                *dbuf     = receive_len2 & 0xff;
                *(dbuf+1) = ((receive_len2 >> 8 )  & 0xff) ;
                *(dbuf+2) = ((receive_len2 >> 16 ) & 0xff) ;
                *(dbuf+3) = ((receive_len2 >> 24 ) & 0xff) ;   
                // printf("receive_buffer[0]=%d\n",receive_buffer[0]); 
                clock_gettime(CLOCK_MONOTONIC, &now);
                LAN_Connection_List[LanNumber].ConnectTime=now.tv_sec;//if have INPUT activity, update idle time
                return apiReady;
            }
            else 
                return apiNotReady;
        }
        else{
            perror("recv");
            return apiFailed;
        }

       
    }
    
}
#endif
// ---------------------------------------------------------------------------
// FUNCTION: To read the incoming data string from LAN receiving block-stream buffer.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//           UINT  length ;     	// length of the received data string.
//	     UCHAR data[length] ;	// data string.	
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_lan_rxstring( UCHAR dhn, UCHAR *dbuf )
{
UCHAR LanNumber;
API_LAN LanConfig;
UINT    Socket;
struct timespec now;

    if( LAN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );

    dhn -= 0x80;	// 2023-03-14
    LanNumber=dhn-psDEV_LAN;
    Socket=LAN_Connection_List[LanNumber].socketFD;
    LanConfig=LAN_Connection_List[LanNumber].APIconfig;
	
    int result;
    

    if( rx_first_time_flag )	// 2023-07-31, ever checking "api_lan_rxready()"?
      goto NEXT;
      
    //while (1)
    //{
    result = api_lan_rxready(dhn, dbuf);
    if(result == apiFailed)
        return apiFailed;
    else if(result == apiNotReady)
        return apiFailed;

    //}

NEXT:
    // this 2 just for the len
    memmove(dbuf, receive_buffer, receive_len + 2);

    // update static variable
    rx_first_time_flag = 0;
    receive_len = 0;
    RxreadyFlag=0;
    clock_gettime(CLOCK_MONOTONIC, &now);
    LAN_Connection_List[LanNumber].ConnectTime=now.tv_sec;//if have INPUT activity, update idle time
    return apiOK;
}

#ifdef _build_DSS_
// ---------------------------------------------------------------------------
// FUNCTION: To read the incoming data string from LAN receiving block-stream buffer.
//           For DSS use because large of buffer size.
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//           ULONG  length ;     	// length of the received data string.
//	         UCHAR  data[length] ;	// data string.	
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_lan_rxstring2( UCHAR dhn,UCHAR *dbuf )
{
    if( LAN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	
    int result;

    //while (1)
    //{
    result = api_lan_rxready2(dhn,dbuf);
    if(result == apiFailed)
        return apiFailed;
    else if(result == apiNotReady)
        return apiFailed;

    //}

    // +4 is for the len
    memmove(dbuf, DSS_APPbuffer, receive_len2);

    // update static variable
    rx_first_time_flag = 0;
    receive_len2 = 0;

    return apiOK;
}
#endif

#ifdef _build_DSS_
// ---------------------------------------------------------------------------
// FUNCTION: To read the incoming data string from LAN receiving block-stream buffer.
//           For communicate with PC DSS JAVA version used(SOH+Len+MSG+LRC=1B+2B+xB+1B)
// INPUT   : dhn
//           The specified device handle number.
// OUTPUT  : dbuf
//           ULONG  length ;     	// length of the received data string.
//	         UCHAR  data[length] ;	// data string.	
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_lan_rxstring3( UCHAR dhn,UCHAR *dbuf )
{
    if( LAN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );
	
    int result;

    //while (1)
    //{
    result = api_lan_rxready3(dhn,dbuf);
    if(result == apiFailed)
        return apiFailed;
    else if(result == apiNotReady)
        return apiFailed;

    //}

    // +4 is for the len
    memmove(dbuf, DSS_APPbuffer, receive_len2);

    // update static variable
    rx_first_time_flag = 0;
    receive_len2 = 0;

    return apiOK;
}
#endif

/**
 *    This function is to check the physical line is connected or not
 *    @retval apiOK            the physical line is connected
 *    @retval apiFailed        the physical line is not connected
 *    @note this function is work when you set at lesat one time ifconfig 
 *           (make sure system run at lesat one time befroe using it)
 */
UCHAR api_lan_lstatus(void){

    UCHAR response[2] = {0};
    FILE *fp;

    fp = popen(command_check_physical_connection2, "r");

    fgets(response, 2,fp);

    // debug
    // printf("%s\n",response);

    pclose(fp);

    // UCHAR response2[6] = {0};
    // FILE *fp2;

    // fp2 = popen(command_check_physical_connection1, "r");

    // fgets(response2, 6,fp2);

    // // debug
    // printf("%s\n",response2);

    // pclose(fp2);

    if(response[0] == '1')
        return apiOK;
    else
        return apiFailed;
}



// ---------------------------------------------------------------------------
// FUNCTION: To get the connection status from LAN device.
// INPUT   : dhn
//	     The specified device handle number.
// OUTPUT  : dbuf
//	     LAN status byte.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
/*
    just check the socket connection again
*/
UCHAR	api_lan_status( UCHAR dhn, UCHAR *dbuf )
{
UCHAR LanNumber;
API_LAN LanConfig;
UINT    Socket;
    if( LAN_CheckDHN( dhn ) == FALSE )
	  return( apiFailed );

    dhn -= 0x80;	// 2023-03-14
    LanNumber=dhn-psDEV_LAN;
    Socket=LAN_Connection_List[LanNumber].socketFD;
    LanConfig=LAN_Connection_List[LanNumber].APIconfig;

	int error = 0;
    socklen_t len = sizeof (error);
    int retval = getsockopt (Socket, SOL_SOCKET, SO_ERROR, &error, &len);   

    if (retval != 0) {
        /* there was a problem getting the error code */
        fprintf(stderr, "error getting socket error code: %s\n", strerror(retval));
        *dbuf = lanDisconnected;

    }

    if (error != 0) {
        /* socket has a non zero error status */
        fprintf(stderr, "socket error: %s\n", strerror(error));
        *dbuf = lanDisconnected;

    } 

    *dbuf = lanConnected;

    return apiOK;
}




/**
 *     this function is run one time DHCP query
 *     @retval TRUE         the query success
 *     @retval FALSE        the query failed
 *     @note this function is implement base on udhcpc command
 */
UCHAR one_Time_DHCP_Process(){


    UCHAR response[10] = {0};
    FILE *fp;
    UCHAR* result;

    fp = popen(command_DHCP, "r");
    fgets(response, 10, fp);
    pclose(fp);

    fp = popen(command_check_succ_DHCP, "r");
    result = fgets(response, 10, fp);
    pclose(fp);


    if(result == 0)
        return FALSE;
    else
    {
        fp = popen(command_killudhcpc, "r");//20210818 added by West.
        pclose(fp);                         //Terminate udhcpc to prevent from udhcpc thread keep alive.
        return TRUE;
    }
        




}



// ---------------------------------------------------------------------------
// FUNCTION: To enable or disable DHCP.
// INPUT   : flag	FALSE = disable, TRUE = enable
// OUTPUT  : none.
// RETURN  : apiOK	-- start or stop DHCP
//           apiFailed	-- line dropped
// ---------------------------------------------------------------------------
UCHAR	api_lan_setup_DHCP( UCHAR flag )
{   
    FILE *fp;
    API_IPCONFIG ipconfig;
    UCHAR dhn_tim;
    UINT  tick1, tick2;


    if(flag)
    {
        if(!PED_GetStateDHCP())
        {
            fp = popen(command_releaseIPAddress, "r");
            if(!fp)
            {
                perror("Failed to release IP address");
                pclose(fp);
                return apiFailed;
            }

            pclose(fp);
        }

        fp = popen(command_obtainIPAddress, "r");
        if(!fp)
        {
            perror("Failed to obtain IP address");
            pclose(fp);
            return apiFailed;
        }

        pclose(fp);

        dhn_tim = api_tim_open( 100 ); // 1 sec
        api_tim_gettick(dhn_tim, (UCHAR *)&tick1);

        while(1)
        {
            if(api_lan_status_DHCP() == apiReady)
            {
                api_lan_getIPconfig(&ipconfig);

                DssIP[0] = ipconfig.IP_Len;
                memcpy(&DssIP[1], ipconfig.IP, ipconfig.IP_Len);

                DssGateway[0] = ipconfig.Gateway_Len;
                memcpy(&DssGateway[1], ipconfig.Gateway, ipconfig.Gateway_Len);

                DssSubNetMask[0] = ipconfig.SubnetMask_Len;
                memcpy(&DssSubNetMask[1], ipconfig.SubnetMask, ipconfig.SubnetMask_Len);

                POST_LoadDssVariables(1);
                PED_SetStateDHCP(1);

                api_tim_close(dhn_tim);
                return apiOK;
            }

            api_tim_gettick(dhn_tim, (UCHAR *)&tick2);
            if((tick2 - tick1) >= 10) // timeout?
            {
                api_tim_close(dhn_tim);
                return apiFailed;
            }
        }
    }
    else
    {
        if(PED_GetStateDHCP())
        {
            fp = popen(command_releaseIPAddress, "r");
            if(!fp)
            {
                perror("Failed to release IP address");
                pclose(fp);
                return apiFailed;
            }

            pclose(fp);

            PED_SetStateDHCP(0);
        }

        return apiOK;
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: To get the final status of DHCP process.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiReady	 -- DHCP has completed
//           apiNotReady -- DHCP in processing
// ---------------------------------------------------------------------------

UCHAR	api_lan_status_DHCP( void )
{
#if 0
    if(one_Time_DHCP_Process() == TRUE)
        return apiReady;
    else
        return apiNotReady;
	// if( g_DHCP_FALG == TRUE )
	//   return( apiReady );
	// else
	//   return( apiNotReady );
#else
    FILE *fp;
    UCHAR response[30] = {0};
    UCHAR len;


    fp = popen(command_getIPAddress, "r");
    fgets(response, 30, fp);
    pclose(fp);
    // output format addr:xxx.xxx.xxx.xxx
    if(strlen(response) != 0)
        return apiReady;
    else
        return apiNotReady;
#endif
}



// ---------------------------------------------------------------------------
// FUNCTION: To PING the target IP address.
// INPUT   : dhn
//           The specified device handle number.
//           ip_len - length of IP string.
//           ip     - the target IP string.	
// OUTPUT  : ms     - connection time in mini-second.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_lan_ping( UCHAR ip_len, UCHAR *ip, ULONG *time )
{
	// if( OS_LAN_Ping( ip_len, ip, time ) )
	//   return( apiOK );
	// else
	//   return( apiFailed );

    UCHAR cmd[30] = {0};

    UCHAR cursor = 0, len = 0;
    UCHAR response[100] = {0};
    FILE *fp;
    UCHAR result;


    len = strlen(command_ping);
    memmove(cmd, command_ping, len);
    cursor += len;
    
    memmove(&cmd[cursor], ip, ip_len);

    printf("command:%s\n",cmd);


    fp = popen(cmd, "r");
    while(fgets(response, 100, fp) != NULL){
        //printf("result:%s\n",response);
    }
    //result = fgets(response, 100, fp);
    pclose(fp);


    printf("finial response:%s\n",response);
    // now is the string processing
    // response succ : round-trip min/avg/max = 2.840/4.279/5.833 ms
    // response fail :

    if(response[0] == '1'){
        return apiFailed;
    }
    else{
        int i,dot_index,equal_index, temp = 0;
        for(i = 0; i < 100; i++){
            if(response[i] == '=')
               equal_index = i;
            if(response[i] == '.'){
                dot_index = i;
                break;
            }
        }
        // get the ms part
        for(i = equal_index+2; i < dot_index; i++){
                temp *= 10;
                temp += response[i] - '0';
        }
 
        // get the actually value
        *time =  temp  * 1000;
        
        
        *time += (response[dot_index+1] - '0')  * 100;
        *time += (response[dot_index+2] - '0')  * 10;
        *time += (response[dot_index+3] - '0')  * 1;

    }

    return apiOK;
    



    // if(result == 0)
    //     printf("get fail\n");
    // else
    //     printf("get succ\n");

    //printf("result:%s\n",response);

    
}


/**
 *     This function is to set default IP
 *     @note only use in debug
 */
void SET_IP_API(){

    API_IPCONFIG ipconfig;

    API_IPCONFIG ipconfig2;


    ipconfig.IP_Len = 12;
    memmove(ipconfig.IP, "172.16.1.173", 12);
    
    ipconfig.Gateway_Len = 10;
    memmove(ipconfig.Gateway, "172.16.1.1", 10);

    ipconfig.SubnetMask_Len = 13;
    memmove(ipconfig.SubnetMask, "255.255.255.0", 13);


    api_lan_setIPconfig(ipconfig);


    api_lan_getIPconfig(&ipconfig2);

    //show_API_IPCONFIG(ipconfig2);


}

void IPstr2int(UCHAR* pIPaddr,UCHAR len,ULONG* IPaddr)
{

}


// ---------------------------------------------------------------------------
// FUNCTION: Get connection time.
// INPUT   : dhn
// OUTPUT  : none.
// RETURN  : current connection time in seconds.
//	     0xffffffff = error
// ---------------------------------------------------------------------------
ULONG	OS_LAN_GetConnectionTime( UCHAR dhn )
{
	if( LAN_CheckDHN( dhn ) )
          return( LAN_Connection_List[dhn-psDEV_LAN-0x80].ConnectTime );
        else
          return( 0xffffffff );
}

// ---------------------------------------------------------------------------
// FUNCTION: LAN task monitor, called by system timer every 10ms periodically.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_LAN_ConnectionTimeTask( void )
{
ULONG	i;
ULONG	contime, contime_now;
ULONG	flag = 0;
struct	timespec now;


	if( ++os_LAN_SEC_TICK < 100 )	// check every 1 second
	  return;
	os_LAN_SEC_TICK = 0;
	
	// any available connection?
	for( i=0; i<LAN_MAX_CONNECTION; i++ )
	   {
	   if( LAN_Connection_List[i].ConnectTime )
	     {
	     clock_gettime(CLOCK_MONOTONIC, &now);
	     contime_now = now.tv_sec;
	     
	     flag = 1;
	     break;
	     }
	   }
	
	if( !flag )
	  return;
	
	// check idle session timeout
	for( i=0; i<LAN_MAX_CONNECTION; i++ )
	   {
	   contime = LAN_Connection_List[i].ConnectTime;
	   if( (contime_now - contime) >= LAN_CONNECTION_SECTIME_MAX )
	     {
	     if( LAN_Connection_List[i].socketFD )
	       LAN_Connection_List[i].RFU = 0xFFFF;	// set timeout flag
	     }
	   }
}

// ---------------------------------------------------------------------------
// FUNCTION: Check connection timeout.
// INPUT   : dhn
// OUTPUT  : none.
// RETURN  : TURE
//	     FALSE
// ---------------------------------------------------------------------------
ULONG	OS_LAN_CheckConnectionTimeout( UCHAR dhn )
{
	if( LAN_Connection_List[dhn-psDEV_LAN-0x80].RFU == 0xFFFF )
	  return( TRUE );
	else
	  return( FALSE );
}
