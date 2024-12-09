//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL				                                    **
//**  PRODUCT  : AS350 PLUS					                                **
//**                                                                        **
//**  FILE     : API_TLS.C                                                  **
//**  MODULE   :                                                            **
//**									                                    **
//**  FUNCTION : API::TLS (SSL/TLS Module)			                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2023/11/14                                                 **
//**  EDITOR   : Tammy Tsai                                                 **
//**                                                                        **
//**  Copyright(C) 2023 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "POSAPI.h"
#include "OS_SECM.h"


// ---------------------------------------------------------------------------
// FUNCTION: To copy certificate and key from secure memory to FLASH memory.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//	         apiFailed
// ---------------------------------------------------------------------------
UCHAR   api_tls_cpkey(void)
{
    UCHAR   buf[5120];
    UINT    length;
    FILE    *ptr;
    int     ret;
    UCHAR   result = apiFailed;


    //get CA certificate from secure memory
    memset(buf, 0x00, sizeof(buf));
    // OS_SECM_GetData(ADDR_CA_CERTIFICATE, 2, buf);
    OS_SECM_GetCaCert(0, 2, buf);
    length = buf[0] + buf[1] * 256;
    // OS_SECM_GetData(ADDR_CA_CERTIFICATE + 2, length, buf + 2);
    OS_SECM_GetCaCert(2, length, buf + 2);
    
    //write CA certificate to the file
    ptr = fopen("/home/AP/ca.crt", "wb+");
    ret = fwrite(buf + 2, length, 1, ptr);
    if(!ret)
    {
        perror("fwrite fail");
        fclose(ptr);
    }
    else
    {
        fclose(ptr);
        chmod("/home/AP/client.crt", S_IRUSR|S_IRGRP|S_IROTH);
        sync();
    }

    //get client certificate from secure memory
    memset(buf, 0x00, sizeof(buf));
    // OS_SECM_GetData(ADDR_CLIENT_CERTIFICATE, 2, buf);
    OS_SECM_GetClientCert(0, 2, buf);
    length = buf[0] + buf[1] * 256;
    // OS_SECM_GetData(ADDR_CLIENT_CERTIFICATE + 2, length, buf + 2);
    OS_SECM_GetClientCert(2, length, buf + 2);
    
    //write client certificate to the file
    ptr = fopen("/home/AP/client.crt", "wb+");
    ret = fwrite(buf + 2, length, 1, ptr);
    if(!ret)
    {
        perror("fwrite fail");
        fclose(ptr);
    }
    else
    {
        fclose(ptr);
        chmod("/home/AP/client.crt", S_IRUSR|S_IRGRP|S_IROTH);
        sync();
    }

    //get client private key from secure memory
    memset(buf, 0x00, sizeof(buf));
    // OS_SECM_GetData(ADDR_CLIENT_PRV_KEY, 2, buf);
    OS_SECM_GetClientPrvKey(0, 2, buf);
    length = buf[0] + buf[1] * 256;
    // OS_SECM_GetData(ADDR_CLIENT_PRV_KEY + 2, length, buf + 2);
    OS_SECM_GetClientPrvKey(2, length, buf + 2);
    
    //write client private key to the file
    ptr = fopen("/home/AP/client.key", "wb+");
    ret = fwrite(buf + 2, length, 1, ptr);
    if(!ret)
    {
        perror("fwrite fail");
        fclose(ptr);
    }
    else
    {
        fclose(ptr);
        chmod("/home/AP/client.key", S_IRUSR|S_IRGRP|S_IROTH);
        sync();
    }

    result = apiOK;

    return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To remove certificate and key.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//	         apiFailed
// ---------------------------------------------------------------------------
UCHAR   api_tls_rmkey(void)
{
    UCHAR   result = apiOK;


    if(remove("/home/AP/ca.crt") != 0)
        result = apiFailed;
    else
    {
        if(remove("/home/AP/client.crt") != 0)
            result = apiFailed;
        else
        {
            if(remove("/home/AP/client.key") != 0)
                result = apiFailed;
        }
    }
    
    return result;
}
