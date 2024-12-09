

#include "POSAPI.h"
#include "API_FTP.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


#define FTP_IMPLICIT    1
#define SSL_OFF         0

#define FALSE 0
#define TRUE 1


#define FTP_CMMD_BUFFER_SIZE 1024
#define FTP_DATA_BUFFER_SIZE 3096
// #define FTP_DATA_BUFFER_SIZE 1024*1024*8

struct cmd
{
	char code[10];
	char arg[30];
};
static SSL_CTX *ctx;
static SSL *ssl_control;
static SSL *ssl_data;
static UCHAR ssl_flag;
static int sock_control;
static int sock_data;
static UCHAR ip[32];
static unsigned long data_port;

static UCHAR ftp_response_buffer[FTP_CMMD_BUFFER_SIZE];

UCHAR ftp_data_buffer[FTP_DATA_BUFFER_SIZE];


// always the first three characters
UCHAR check_response_code_equal(UCHAR *rspcode){
	int i;
	for(i = 0; i < 3; i++){
		if(rspcode[i] != ftp_response_buffer[i])
			return FALSE;
	}
	return TRUE;

}


UCHAR ftp_recv_sub(int socket, int* count, UCHAR *array, int buffer_size){

	int result;
	memset(array, 0 ,buffer_size);
	result = recv(socket, array, buffer_size, 0);
	*count = 0;
	if(result < 0)
		return apiFailed;

	printf("%s\n",array);
	
	return apiOK;

}

UCHAR ftp_recv_ssl(SSL * ssl, int* count, UCHAR *array, int buffer_size){

	int result;
	memset(array, 0 ,buffer_size);
	result = SSL_read(ssl,array, buffer_size);
	*count = 0;
	if(result < 0)
		return apiFailed;

	printf("%s\n",array);
	
	return apiOK;

}


UCHAR get_ftp_response(int* count) {
	if(ssl_flag == 0)
		return ftp_recv_sub(sock_control, count, ftp_response_buffer, FTP_CMMD_BUFFER_SIZE);
	else if(ssl_flag == 1)
		return ftp_recv_ssl(ssl_control, count, ftp_response_buffer, FTP_CMMD_BUFFER_SIZE);

}

UCHAR get_ftp_data(int* count){
	if(ssl_flag == 0)
		return ftp_recv_sub(sock_data, count, ftp_data_buffer, FTP_DATA_BUFFER_SIZE);
	else if(ssl_flag == 1)
		return ftp_recv_ssl(ssl_data, count, ftp_data_buffer, FTP_DATA_BUFFER_SIZE);
}


UCHAR ftclient_send_cmd(struct cmd *c){
	
	int result;

	memset(ftp_response_buffer,0,  FTP_CMMD_BUFFER_SIZE);

	sprintf(ftp_response_buffer,"%s %s\r\n",c->code,c->arg);

	printf("send:%s\n",ftp_response_buffer);
	fflush(stdout);
	if(ssl_flag == 0)
		result = send(sock_control, ftp_response_buffer, (int)strlen(ftp_response_buffer), 0);
	else if(ssl_flag == 1)
		result = SSL_write(ssl_control,ftp_response_buffer, (int)strlen(ftp_response_buffer));

	if(result < 0)
		return apiFailed;
	
	return apiOK;
}


void update_data_port(){

	// this will get the port number
	//7 Entering Passive Mode (172,16,1,152,234,181)


	int i,idx;
	int count = 0;
	for(i = 0; i <FTP_CMMD_BUFFER_SIZE; i++){
		if(ftp_response_buffer[i] == ',')
			count+= 1;
		if(count == 4)
			break;
	}
	
	idx = i+1;
	int h_port_num = 0;
	int l_port_num = 0;
	for(i = idx; ftp_response_buffer[i] != ','; i++){

		h_port_num *= 10;
		h_port_num += ftp_response_buffer[i] - '0';
	}

	idx = i+1;

	for(i = idx; ftp_response_buffer[i] != ')'; i++){

		l_port_num *= 10;
		l_port_num += ftp_response_buffer[i] - '0';
	}

	data_port = h_port_num * 256 + l_port_num;

	printf("L:%d,H:%d\n",l_port_num,h_port_num);
	fflush(stdout);
}


UCHAR ftp_passive(){

	struct cmd cmd;
	int len;

	strcpy(cmd.code, "PASV");
	strcpy(cmd.arg, "");
	ftclient_send_cmd(&cmd);

	get_ftp_response(&len);

	if(check_response_code_equal("227") == FALSE)
		return apiFailed;

	update_data_port();

	return apiOK;

}


UCHAR create_data_socket(){

	struct sockaddr_in addr;
	int len; 

	bzero(&addr, sizeof(addr));     //將＆addr中的前sizeof（addr）位元組置為0，包括'\0'
	addr.sin_family = AF_INET;      //AF_INET代表TCP／IP協議
	addr.sin_addr.s_addr = inet_addr(ip); //將點間隔地址轉換為網路位元組順序
	addr.sin_port = htons(data_port);    //轉換為網路位元組順序
	len = sizeof(addr);

	if(( sock_data =socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Socket Error!\n");
		return apiOutOfService;
		//exit(1);
	}

	printf("data socket succ\n");

	if(connect(sock_data, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		printf("Connect Error!\n");
		return apiOutOfService;
		//exit(1);
	}

	if(ssl_flag == 1){

		ssl_data = SSL_new(ctx);
    	SSL_set_fd(ssl_data, sock_data);
		SSL_copy_session_id(ssl_data,ssl_control);

		if(SSL_connect(ssl_data) == -1){
			printf("ssl data connected failed\n");

			return apiOutOfService;
		}

		printf("ssl data connected succ\n");

		return apiOutOfService;

	}

	return apiOK;
}



UCHAR  api_ftp_open( UCHAR *serverip , UCHAR *serverport,UCHAR  SSLOnOff ){

    int s, len,result;

    struct sockaddr_in addr;


	// use orignal way to connected
	memset(&addr, 0 , sizeof(addr));
	addr.sin_family = AF_INET;      //AF_INET代表TCP／IP協議
	addr.sin_addr.s_addr = inet_addr(serverip); //將點間隔地址轉換為網路位元組順序
	addr.sin_port = htons(atoi(serverport));    //轉換為網路位元組順序
	len = sizeof(addr);

	if((sock_control=socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Socket Error!\n");
		return apiOutOfService;
	}

	if(connect(sock_control, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		printf("Connect Error!\n");
		return apiOutOfService;
	}


    if(SSLOnOff == SSL_OFF){

        printf("Connected to %s\n", serverip);
        //print_reply(read_reply());

		ssl_flag = 0;
		get_ftp_response(&len);

		if(check_response_code_equal("220") == FALSE)
			return apiOutOfService;
		
		strcpy(ip, serverip);
		
		return apiOK;

    }
    else if(SSLOnOff == FTP_IMPLICIT){

		SSL_library_init();
    	OpenSSL_add_all_algorithms();
    	SSL_load_error_strings();
		ctx = SSL_CTX_new(SSLv23_client_method());

		if(ctx == NULL)
			return apiOutOfService;

    	ssl_control = SSL_new(ctx);
    	SSL_set_fd(ssl_control, sock_control);

		if(SSL_connect(ssl_control) == -1){
			SSL_shutdown(ssl_control);
			SSL_free(ssl_control);
			SSL_CTX_free(ctx);
			close(sock_control);
			ERR_print_errors_fp(stderr);
			return apiOutOfService;
		}

		ssl_flag = 1;
		get_ftp_response(&len);

		if(check_response_code_equal("220") == FALSE){
			SSL_shutdown(ssl_control);
			SSL_free(ssl_control);
			SSL_CTX_free(ctx);
			close(sock_control);
			return apiOutOfService;
		}
		
		strcpy(ip, serverip);

		return apiOK;

	}
        

}




UCHAR  api_ftp_login( UCHAR *user, UCHAR *password ){


    struct cmd cmd;
	int len;

	strcpy(cmd.code, "USER");
	strcpy(cmd.arg, user);
	ftclient_send_cmd(&cmd);

	get_ftp_response(&len);

	if(check_response_code_equal("331") == FALSE)
		return apiFailed;
	

	strcpy(cmd.code, "PASS");
	strcpy(cmd.arg, password);
	ftclient_send_cmd(&cmd);

	get_ftp_response(&len);

	if(check_response_code_equal("230") == FALSE)
		return apiFailed;
	

	return apiOK;


}



UCHAR api_ftp_close( void ){

	struct cmd cmd;
	int len;

	strcpy(cmd.code, "QUIT");
	strcpy(cmd.arg, "");

	ftclient_send_cmd(&cmd);
	get_ftp_response(&len);

	if(check_response_code_equal("221") == FALSE)
		return apiFailed;
	if(ssl_flag == FTP_IMPLICIT){
		SSL_shutdown(ssl_control);
    	SSL_free(ssl_control);
		SSL_CTX_free(ctx);
	}
    close(sock_control);
	
    return apiOK;

}

UCHAR api_ftp_ls(UINT bufsize, ULONG *FileSize, UCHAR *pFile){
	
	struct cmd cmd;
	int len;


	printf("start ls\n");

	if(ftp_passive() != apiOK)
		return apiFailed;
	
	create_data_socket();

	strcpy(cmd.code, "LIST");
	strcpy(cmd.arg, "");

	ftclient_send_cmd(&cmd);
	get_ftp_response(&len);

	if( (check_response_code_equal("150") == FALSE) && 
			(check_response_code_equal("125") == FALSE) )
		return apiFailed;

	get_ftp_data(&len);
	if(ssl_flag == FTP_IMPLICIT){
		SSL_shutdown(ssl_data);
		SSL_free(ssl_data);
	}
	close(sock_data);
	get_ftp_response( &len);

	if(check_response_code_equal("226") == FALSE)
		return apiFailed;
	
	// copy the result
	len = strlen(ftp_data_buffer);
	if(len > bufsize){
		*FileSize = bufsize;
		memmove(pFile, ftp_data_buffer, bufsize);
	}
	else{
		*FileSize = len;
		memmove(pFile, ftp_data_buffer, len);
	}
	
	return apiOK;
	
}


UCHAR api_ftp_pwd( void  ){

	struct cmd cmd;
	int len;

	strcpy(cmd.code, "PWD");
	strcpy(cmd.arg, "");

	ftclient_send_cmd(&cmd);
	get_ftp_response(&len);

	if(check_response_code_equal("257") == FALSE)
		return apiFailed;
	
	return apiOK;

}

UCHAR api_ftp_cd(UCHAR *cd){

	struct cmd cmd;
	int len;

	strcpy(cmd.code, "CWD");
	strcpy(cmd.arg, cd);

	ftclient_send_cmd(&cmd);
	get_ftp_response(&len);

	if(check_response_code_equal("257") == FALSE)
		return apiFailed;
	
	return apiOK;

}

UCHAR *api_ftp_get(UCHAR *FileName, ULONG *FileSize){

	struct cmd cmd;
	int len;

	if(ftp_passive() != apiOK)
		return NULL;

	create_data_socket();

	strcpy(cmd.code, "RETR");
	strcpy(cmd.arg, FileName);

	ftclient_send_cmd(&cmd);
	get_ftp_response(&len);

	if( (check_response_code_equal("150") == FALSE) && 
			(check_response_code_equal("125") == FALSE) )
		return NULL;

	//create_data_socket();
	get_ftp_data(&len);
	if(ssl_flag == FTP_IMPLICIT){
		SSL_shutdown(ssl_data);
		SSL_free(ssl_data);
	}
	close(sock_data);
	get_ftp_response(&len);

	if(check_response_code_equal("226") == FALSE)
		return NULL;
	*FileSize = strlen(ftp_data_buffer);
	return ftp_data_buffer;	
}




UCHAR api_ftp_GetReply(UCHAR *readbuf){

	ULONG len = strlen(ftp_response_buffer);

	if(len == 0)
		return apiFailed;


	if (len > 256) {
		readbuf[0] = len / 256;
		readbuf[1] = len % 256;
	} else {
		readbuf[0] = len;
		readbuf[1] = 0;
	}

	memmove(&readbuf[2], ftp_response_buffer, len);

	return apiOK;
}


UCHAR api_ftp_binary( void ){

	struct cmd cmd;
	int len;

	strcpy(cmd.code, "TYPE");
	strcpy(cmd.arg, "I");

	ftclient_send_cmd(&cmd);
	get_ftp_response(&len);

	if(check_response_code_equal("200") == FALSE)
		return apiFailed;
	
	return apiOK;

}

UCHAR api_ftp_ascii( void ){

	struct cmd cmd;
	int len;

	strcpy(cmd.code, "TYPE");
	strcpy(cmd.arg, "A");

	ftclient_send_cmd(&cmd);
	get_ftp_response(&len);

	if(check_response_code_equal("200") == FALSE)
		return apiFailed;
	
	return apiOK;
	
}

UCHAR api_ftp_dir(UINT bufsize, ULONG *FileSize, UCHAR *pFile){

	return api_ftp_ls(bufsize, FileSize, pFile);
}


UCHAR api_ftp_pbsz(void) {

	struct cmd cmd;
	int len;

	strcpy(cmd.code, "PBSZ");
	strcpy(cmd.arg, "0");

	ftclient_send_cmd(&cmd);
	get_ftp_response(&len);

	if(check_response_code_equal("200") == FALSE)
		return apiFailed;
	
	return apiOK;
}

UCHAR api_ftp_prot(void) {

	struct cmd cmd;
	int len;

	strcpy(cmd.code, "PROT");
	strcpy(cmd.arg, "P");

	ftclient_send_cmd(&cmd);
	get_ftp_response(&len);

	if(check_response_code_equal("200") == FALSE)
		return apiFailed;
	
	return apiOK;
}



