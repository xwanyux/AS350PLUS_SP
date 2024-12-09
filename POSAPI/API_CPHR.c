





//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  :							    **
//**  PRODUCT  : AS330	                                                    **
//**                                                                        **
//**  FILE     : API_CPHR.C 	                                            **
//**  MODULE   : api_des_encipher()			                    **
//**		 api_des_decipher()					    **
//**		 api_3des_encipher()					    **
//**		 api_3des_decipher()					    **
//**		 api_rsa_loadkey()					    **
//**		 api_rsa_recover()					    **
//**		 api_rsa_encrypt()					    **
//**		 api_aes_encipher()					    **
//**		 api_aes_decipher()					    **
//**		 							    **
//**  FUNCTION : API::CIPHER (Crypto Lib Module)			    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2020/09/3                                                 **
//**  EDITOR   : Wayne Shih                                                **
//**                                                                        **
//**  Copyright(C) 2018 SymLink Corporation. All rights reserved.	    **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------


#include "POSAPI.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_alg.h>
#include <linux/socket.h>
#include <string.h>

#ifndef SOL_ALG
#define SOL_ALG 279
#endif

// for rsa encrytion and decrytion
UCHAR rsa_key[550]; // maximum supprot len to 525
UINT  rsa_key_len = 0;
UCHAR flag_load_key = 0;
UINT  g_moduls_len = 0;


/**
 *  This function is use to compute the symmetic block cipher algorthm by netlink in linux kernel.
 *  @param[in] sa                       necessary infomation for using netlink,
 *                                      you should provide  salg_family = AF_ALG (always),
 *                                                          salg_type =  "skcipher",(always),
 *                                                          alg_name =  algorithm name
 * @param[in] KeySize                   length of key size
 * @param[in] InputLen                  length of input 
 * @param[in] pKey                      key data
 * @param[in] pIn                       message to be encrypt or decrypt
 * @param[out] pOut                     result of message be decode or encode
 * @param[in] encryFlag                 1 mean encryption , 0 mean decryption
 * @retval apiFailed                    process failed
 * @retval apiOK                        process success
 * @note this funciton assume input length equal output length
 */     
UCHAR socket_IO_to_if_ALG(struct sockaddr_alg sa,
                        UCHAR KeySize, UCHAR InputLen, UCHAR* pKey, UCHAR* pIn,
                        UCHAR* pOut, UCHAR encryFlag){

  int opfd;
  int tfmfd;

  struct msghdr msg = {};
  struct cmsghdr *cmsg;
  char cbuf[CMSG_SPACE(4)] = {0};

  struct iovec iov;
  int i;
  // create a socket to communicate the linux core alogrithm
  tfmfd = socket(AF_ALG, SOCK_SEQPACKET, 0);

  if(tfmfd < 0)
    return apiFailed;

  // bind the which alogrithm (eg AES,DEG ..)
  bind(tfmfd, (struct sockaddr *)&sa, sizeof(sa));
  // set key 
  setsockopt(tfmfd, SOL_ALG, ALG_SET_KEY,  pKey, KeySize);
  // accept to receive
  opfd = accept(tfmfd, NULL, 0);

  if(opfd < 0){
    close(tfmfd);
    return apiFailed;
  }

  // message format we need to communicate by socket
  msg.msg_control = cbuf;
  msg.msg_controllen = sizeof(cbuf);
  
  // the first message is about doing encryption or decryption
  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_ALG;
  cmsg->cmsg_type = ALG_SET_OP;
  cmsg->cmsg_len = CMSG_LEN(4);
  if(encryFlag == 1)
    *(__u32 *)CMSG_DATA(cmsg) = ALG_OP_ENCRYPT;
  else
    *(__u32 *)CMSG_DATA(cmsg) = ALG_OP_DECRYPT;


  // set the input message 
  iov.iov_base = pIn;
  iov.iov_len = InputLen;
  // set the input message to the protocal header
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  // send the message
  sendmsg(opfd, &msg, 0);
  // get the result
  if(read(opfd, pOut, InputLen) < 0){
    close(opfd);
    close(tfmfd);
    return apiFailed;
  }

  close(opfd);
  close(tfmfd);

  return apiOK;


}

/**
 *  This function is use to compute the rsa algorthm by netlink in linux kernel.
 *  @param[in] sa                       necessary infomation for using netlink,
 *                                      you should provide  salg_family = AF_ALG (always),
 *                                                          salg_type =  "akcipher",(always),
 *                                                          alg_name =  "rsa" (always)
 * @param[in] KeySize                   length of key size
 * @param[in] InputLen                  length of input 
 * @param[in] OutputLen                 length of output 
 * @param[in] pKey                      key data
 * @param[in] pIn                       message to be encrypt or decrypt
 * @param[out] pOut                     result of message be encrypt
 * @retval apiFailed                    process failed
 * @retval apiOK                        process success
 * @note we actually not provide encrypt and decrypt flag, if you want to doing public key, just provide the 
 *       public key. The same as private key.
 */    
UCHAR socket_IO_to_rsa(struct sockaddr_alg sa,
                        UINT KeySize, UINT InputLen, UINT OutputLen,UCHAR* pKey, UCHAR* pIn,
                        UCHAR* pOut){

  int opfd;
  int tfmfd;

  struct msghdr msg = {};
  struct cmsghdr *cmsg;
  char cbuf[CMSG_SPACE(4)] = {0};

  struct iovec iov;
  int i;
  // create a socket to communicate the linux core alogrithm
  tfmfd = socket(AF_ALG, SOCK_SEQPACKET, 0);

  if(tfmfd < 0)
    return apiFailed;

  // bind the which alogrithm (eg AES,DEG ..)
  bind(tfmfd, (struct sockaddr *)&sa, sizeof(sa));
  // set key 
  setsockopt(tfmfd, SOL_ALG, ALG_SET_KEY,  pKey, KeySize);
  // accept to receive
  opfd = accept(tfmfd, NULL, 0);

  if(opfd < 0){
    close(tfmfd);
    return apiFailed;
  }

  // message format we need to communicate by socket
  msg.msg_control = cbuf;
  msg.msg_controllen = sizeof(cbuf);
  
  // the first message is about doing encryption or decryption
  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_ALG;
  cmsg->cmsg_type = ALG_SET_OP;
  cmsg->cmsg_len = CMSG_LEN(4);
    *(__u32 *)CMSG_DATA(cmsg) = ALG_OP_ENCRYPT;



  // set the input message 
  iov.iov_base = pIn;
  iov.iov_len = InputLen;
  // set the input message to the protocal header
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  // send the message
  sendmsg(opfd, &msg, 0);
  // get the result
  if(read(opfd, pOut, OutputLen) < 0){
    close(opfd);
    close(tfmfd);
    return apiFailed;
  }

  close(opfd);
  close(tfmfd);

  return apiOK;


}
/**
 *  This function is use to compute the symmetic block cipher algorthm by netlink in linux kernel.
 *  @param[in] sa                       necessary infomation for using netlink,
 *                                      you should provide  salg_family = AF_ALG (always),
 *                                                          salg_type =  "skcipher",(always),
 *                                                          alg_name =  algorithm name
 * @param[in] KeySize                   length of key size
 * @param[in] InputLen                  length of input 
 * @param[in] pKey                      key data
 * @param[in] pIn                       message to be encrypt or decrypt
 * @param[out] pOut                     result of message be decode or encode
 * @param[in] encryFlag                 1 mean encryption , 0 mean decryption
 * @retval apiFailed                    process failed
 * @retval apiOK                        process success
 * @note this funciton assume input length equal output length
 */     
UCHAR socket_IO_to_if_ALG_CBC(struct sockaddr_alg sa,
                        UCHAR KeySize, UCHAR InputLen, UCHAR* pKey, UCHAR* pIn,
                        UCHAR* pOut, UCHAR *iPiv, UCHAR ivLength, UCHAR encryFlag){

  int opfd;
  int tfmfd;

  struct msghdr msg = {};
  struct cmsghdr *cmsg;
  char cbuf[CMSG_SPACE(4) + CMSG_SPACE(20)] = {0};

  struct iovec iov;
  struct af_alg_iv *iv;
  int i;
  // create a socket to communicate the linux core alogrithm
  tfmfd = socket(AF_ALG, SOCK_SEQPACKET, 0);

  if(tfmfd < 0)
    return apiFailed;

  // bind the which alogrithm (eg AES,DEG ..)
  bind(tfmfd, (struct sockaddr *)&sa, sizeof(sa));
  // set key 
  setsockopt(tfmfd, SOL_ALG, ALG_SET_KEY,  pKey, KeySize);
  // accept to receive
  opfd = accept(tfmfd, NULL, 0);

  if(opfd < 0){
    close(tfmfd);
    return apiFailed;
  }

  // message format we need to communicate by socket
  msg.msg_control = cbuf;
  msg.msg_controllen = sizeof(cbuf);
  
  // the first message is about doing encryption or decryption
  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_ALG;
  cmsg->cmsg_type = ALG_SET_OP;
  cmsg->cmsg_len = CMSG_LEN(4);
  if(encryFlag == 1)
    *(__u32 *)CMSG_DATA(cmsg) = ALG_OP_ENCRYPT;
  else
    *(__u32 *)CMSG_DATA(cmsg) = ALG_OP_DECRYPT;
  cmsg = CMSG_NXTHDR(&msg, cmsg);
  cmsg->cmsg_level = SOL_ALG;
  cmsg->cmsg_type = ALG_SET_IV;
  cmsg->cmsg_len = CMSG_LEN(20);
  iv = (void *)CMSG_DATA(cmsg);
  iv->ivlen = ivLength;
  memcpy(iv->iv, iPiv, ivLength);

  // set the input message 
  iov.iov_base = pIn;
  iov.iov_len = InputLen;
  // set the input message to the protocal header
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  // send the message
  sendmsg(opfd, &msg, 0);
  // get the result
  if(read(opfd, pOut, InputLen) < 0){
    close(opfd);
    close(tfmfd);
    return apiFailed;
  }

  close(opfd);
  close(tfmfd);

  return apiOK;


}
// ---------------------------------------------------------------------------
// FUNCTION: To encipher plaintext data (pIn) to enciphered data (pOut) by DES.
// INPUT   : pIn  -- the plaintext data. (8-byte)
//           pKey  -- the DES key.       (8-byte)
// OUTPUT  : pOut -- the ciphered data.  (8-byte)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_des_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey )
{   
    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name ="ecb(des)"
     };
    return socket_IO_to_if_ALG(sa,8,8,pKey,pIn,pOut,1);
}

// ---------------------------------------------------------------------------
// FUNCTION: To decipher enciphered data (pIn) to plaintext data (pOut) by DES.
// INPUT   : pIn  -- the ciphered data.  (8-byte)
//           pKey -- the DES key.        (8-byte)
// OUTPUT  : pOut -- the plaintext data. (8-byte)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_des_decipher( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey )
{

    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name ="ecb(des)"
     };
    return socket_IO_to_if_ALG(sa,8,8,pKey,pIn,pOut,0);
}

// ---------------------------------------------------------------------------
// FUNCTION: To encipher plaintext data (pIn) to enciphered data (pOut) by 3DES.
// INPUT   : pIn  -- the plaintext data. (8-byte)
//           pKey -- the 3DES key.       (24-byte)
// OUTPUT  : pOut -- the ciphered data.  (8-byte)
// RETURN  : apiOK
//           apiFailed
// NOTE    : ECB mode. (This function cannot work! To be checked.)
// ---------------------------------------------------------------------------
ULONG	api_3des_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey )
{
    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name = "ecb(des3_ede)"
     };
    return socket_IO_to_if_ALG(sa,24,8,pKey,pIn,pOut,1);

}
// ---------------------------------------------------------------------------
// FUNCTION: To encipher plaintext data (pIn) to enciphered data (pOut) by 3DES.
// INPUT   : pIn  -- the plaintext data. (8 or 8*N -bytes)
//			     inLen - length of the input data
//           pKey -- the 3DES key.       (24-byte)
// OUTPUT  : pOut -- the ciphered data.  (8 or 8*N-byte)
// RETURN  : apiOK
//           apiFailed
// NOTE    : ECB mode. (This function cannot work! To be checked.)
// ---------------------------------------------------------------------------
ULONG	api_3des_encipher2( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR inLen )
{
    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name = "ecb(des3_ede)"
     };
    return socket_IO_to_if_ALG(sa,24,inLen,pKey,pIn,pOut,1);

}
// ---------------------------------------------------------------------------
// FUNCTION: To decipher enciphered data (pIn) to plaintext data (pOut) by 3DES.
// INPUT   : pIn  -- the ciphered data.   (8-byte)
//           pKey -- the 3DES key.        (24-byte)
// OUTPUT  : pOut -- the plaintext data.  (8-byte)
// RETURN  : apiOK
//           apiFailed
// NOTE    : ECB mode. (This function cannot work! To be checked.)
// ---------------------------------------------------------------------------
ULONG	api_3des_decipher( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey )
{
    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name = "ecb(des3_ede)"
     };
    return socket_IO_to_if_ALG(sa,24,8,pKey,pIn,pOut,0);
}

// ---------------------------------------------------------------------------
// FUNCTION: To decipher enciphered data (pIn) to plaintext data (pOut) by 3DES.
// INPUT   : pIn  -- the plaintext data. (8 or 8*N -bytes)
//			     inLen - length of the input data
//           pKey -- the 3DES key.       (24-byte)
// OUTPUT  : pOut -- the ciphered data.  (8 or 8*N-byte)
// RETURN  : apiOK
//           apiFailed
// NOTE    : ECB mode. (This function cannot work! To be checked.)
// ---------------------------------------------------------------------------
ULONG	api_3des_decipher2( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey, UCHAR inLen )
{
    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name = "ecb(des3_ede)"
     };
    return socket_IO_to_if_ALG(sa,24,inLen,pKey,pIn,pOut,0);

}

// ---------------------------------------------------------------------------
// FUNCTION: To encipher plaintext data (pIn) to enciphered data (pOut) by 3DES(CBC).
// INPUT   : pIn     -- the plaintext data. (8 or 8*N -bytes)
//           pKey    -- the 3DES key.	    (key values)
//	         KeySize -- key size.	        (16/24 bytes)
//           inLen   -- length of the input data.
//           icv     -- initial chain vector.
//           icvLen  -- length of initial chain vector.
// OUTPUT  : pOut    -- the ciphered data.  (8 or 8*N-byte)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_3des_cbc_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize, UCHAR inLen, UCHAR *iv, UCHAR ivLen )
{
    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name = "cbc(des3_ede)"
     };
    return socket_IO_to_if_ALG_CBC(sa, KeySize, inLen, pKey, pIn, pOut, iv, ivLen, 1);
}

// ---------------------------------------------------------------------------
// FUNCTION: To decipher enciphered data (pIn) to plaintext data (pOut) by 3DES(CBC).
// INPUT   : pIn     -- the ciphered data. (8 or 8*N -bytes)
//           pKey    -- the 3DES key.	    (key values)
//	         KeySize -- key size.	        (16/24 bytes)
//           inLen   -- length of the input data.
//           icv     -- initial chain vector.
//           icvLen  -- length of initial chain vector.
// OUTPUT  : pOut    -- the plaintext data.  (8 or 8*N-byte)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_3des_cbc_decipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize, UCHAR inLen, UCHAR *iv, UCHAR ivLen )
{
    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name = "cbc(des3_ede)"
     };
    return socket_IO_to_if_ALG_CBC(sa, KeySize, inLen, pKey, pIn, pOut, iv, ivLen, 0);
}

// ---------------------------------------------------------------------------
// FUNCTION: To encipher plaintext data (pIn) to enciphered data (pOut) by AES.
// INPUT   : pIn     -- the plaintext data. (same length of KeySize)
//           pKey    -- the AES key.	    (key values)
//	     KeySize -- key size.	    (16/24/32 bytes)
// OUTPUT  : pOut    -- the ciphered data.  (same length of KeySize)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_aes_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize )
{
    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name = "ecb(aes)"
     };
    return socket_IO_to_if_ALG(sa,KeySize,16,pKey,pIn,pOut,1);
}

// ---------------------------------------------------------------------------
// FUNCTION: To encipher plaintext data (pIn) to enciphered data (pOut) by AES.
// INPUT   : pIn     -- the plaintext data.
//           pKey    -- the AES key.	    (key values)
//	         KeySize -- key size.	        (16/24/32 bytes)
//           inLen   -- length of the plaintext data
// OUTPUT  : pOut    -- the ciphered data.  (same length of KeySize)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_aes_encipher2( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize, UCHAR inLen )
{
    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name = "ecb(aes)"
     };
    return socket_IO_to_if_ALG(sa,KeySize,inLen,pKey,pIn,pOut,1);
}

// ---------------------------------------------------------------------------
// FUNCTION: To decipher enciphered data (pIn) to plaintext data (pOut) by AES.
// INPUT   : pIn  -- the ciphered data.     (same length of KeySize)
//           pKey -- the AES key.	    (key values)
//	     KeySize -- key size.	    (16/24/32 bytes)
// OUTPUT  : pOut -- the plaintext data.    (same length of KeySize)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_aes_decipher( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey, UCHAR KeySize )
{

    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name = "ecb(aes)"
     };
    return socket_IO_to_if_ALG(sa,KeySize,16,pKey,pIn,pOut,0);
}

// ---------------------------------------------------------------------------
// FUNCTION: To decipher enciphered data (pIn) to plaintext data (pOut) by AES.
// INPUT   : pIn  -- the ciphered data.
//           pKey -- the AES key.	        (key values)
//	         KeySize -- key size.	        (16/24/32 bytes)
//           inLen   -- length of the plaintext data
// OUTPUT  : pOut -- the plaintext data.    (same length of KeySize)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_aes_decipher2( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey, UCHAR KeySize, UCHAR inLen )
{
    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name = "ecb(aes)"
     };
    return socket_IO_to_if_ALG(sa,KeySize,inLen,pKey,pIn,pOut,0);
}

// ---------------------------------------------------------------------------
// FUNCTION: To encipher plaintext data (pIn) to enciphered data (pOut) by AES(CBC).
// INPUT   : pIn     -- the plaintext data. (same length of KeySize)
//           pKey    -- the AES key.	    (key values)
//	     KeySize -- key size.	    (16/24/32 bytes)
// OUTPUT  : pOut    -- the ciphered data.  (same length of KeySize)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_aes_cbc_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize, UCHAR *iv, UCHAR ivLength)
{
    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name = "cbc(aes)"
     };
    return socket_IO_to_if_ALG_CBC( sa, KeySize, KeySize, pKey, pIn, pOut, iv, ivLength,1);
}

// ---------------------------------------------------------------------------
// FUNCTION: To decipher enciphered data (pIn) to plaintext data (pOut) by AES(CBC).
// INPUT   : pIn  -- the ciphered data.     (same length of KeySize)
//           pKey -- the AES key.	    (key values)
//	         KeySize -- key size.	    (16/24/32 bytes)
// OUTPUT  : pOut -- the plaintext data.    (same length of KeySize)
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
ULONG	api_aes_cbc_decipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize, UCHAR *iv, UCHAR ivLength)
{

    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "skcipher",
    .salg_name = "cbc(aes)"
     };
    return socket_IO_to_if_ALG_CBC( sa, KeySize, KeySize, pKey, pIn, pOut, iv, ivLength, 0);
}

// ---------------------------------------------------------------------------
// FUNCTION: To load RSA public key (n,e) for recover function.
// INPUT   : mod -- modulus  (2L-V, max 256 bytes).
//           exp -- exponent (max 3 bytes, value =0x02, 0x03 or 0x010001)
// OUTPUT  : none.
// RETURN  : apiOK
//           apiFailed
// NOTE	   : This function is used by EMV L2 kernel only.
// ---------------------------------------------------------------------------
ULONG	api_rsa_loadkey( UCHAR *modulus, UCHAR *exponent )
{
UINT	modulus_len;
UINT exponet_len;
UINT exponet_value;
UINT modulus_tag_len;
//int	err;
UINT total_tag_len;
UINT total_len;

UINT cursor;

 
	modulus_len = *modulus + *(modulus+1)*256;
  // update global variable
  g_moduls_len  = modulus_len;
  // printf("modulus %d\n", g_moduls_len);
  modulus_len += 1; // this is add for start of modulus \x00
  if(modulus_len > 255)
      modulus_tag_len = 2;
  else
      modulus_tag_len = 1;

	
	switch( *exponent )	// check 1'st byte of the exponent & pad leading zero to fit modulus length
	      {
	      case 0x00:  // 00 00 02 or 00 00 03
	      	
	      	   exponet_len = 1;
             exponet_value = *(exponent + 2);
	      	   break;
	      	   
	      case 0x01:  // 01 00 01 (65537)

		        exponet_len = 3;
	           break;

	      case 0x02:  // 02
	      case 0x03:  // 03
             exponet_len = 1;
	      	   exponet_value = *exponent;
	      	   break;
	      	   
	      default:    // invalid exponent value
	           return( apiFailed );

	      }

      total_tag_len =  2  + modulus_tag_len + modulus_len + 2 + exponet_len;

      //printf("start flag:%d\n",total_tag_len);

      if(total_tag_len > 255)
          total_len = 2 +  2 + total_tag_len;
      else
          total_len = 2 +  1 + total_tag_len;


    // update global variable
    rsa_key_len = total_len;

    //printf("total len:%d\n",total_len);


    /* public key structure for rsa kernel

    header(1) 0x30 0x82 0xXX 0xXX 
    header(2) 0x30 0x81 0xXX
    
    modulus(1) 0x02 0x81 0xXX 0x00 + (actaul public key)
    modulus(2) 0x02 0x82 0xXX 0xXX 0x00 + (actual public key)

    exponet(1) 0x02 0x01 (0x02, 0x03)
    exponet(2) 0x02 0x03  0x01 0x00 0x01

    head + modulus + exponent
    */

   // header
   rsa_key[0] = 0x30;
   if(total_tag_len > 255){
     rsa_key[1] = 0x82;
     rsa_key[2] = total_tag_len / 256;
     rsa_key[3] = total_tag_len % 256;
     cursor = 4;
   }
   else{
     rsa_key[1] = 0x81;
     rsa_key[2] = total_tag_len;
     cursor = 3;
   }
   // modulus
   rsa_key[cursor++] = 0x02;
   
   if(modulus_len > 255){
     rsa_key[cursor++] = 0x82;
     rsa_key[cursor++] = modulus_len / 256;
     rsa_key[cursor++] = modulus_len % 256;
     rsa_key[cursor++] = 0x00;
   }
   else{
    rsa_key[cursor++] = 0x81;
     rsa_key[cursor++] = modulus_len;
     rsa_key[cursor++] = 0x00;
   }

   memmove(&rsa_key[cursor], modulus+2, g_moduls_len);
   cursor += (modulus_len -1);
  // exponent (just for public key)
  rsa_key[cursor++] = 0x02;
  if(exponet_len == 3){
    rsa_key[cursor++] = 0x03;
    rsa_key[cursor++] = 0x01;
    rsa_key[cursor++] = 0x00;
    rsa_key[cursor++] = 0x01;
  }
  else{
    rsa_key[cursor++] = 0x01;
    rsa_key[cursor++] = exponet_value; 
  }
  // update global variable
   flag_load_key = 1;

   // debug only dump key
   int i =0;
   printf("result key:\n");
    printf("\n\t");
  //   for(i = 0; i < rsa_key_len; i++){
  //       printf("%2x",rsa_key[i]);
  //       if((i+1) % 16 == 0){
  //           printf("\n");
  //           printf("\t");
  //       }
  //   }
  //   printf("\n");
  for(i = 0 ; i < rsa_key_len ; i++)
  {
    printf("%02X ", rsa_key[i]);
    if((i + 1) % 16 == 0)
    {
        printf("\n\t");
    }
  }
  printf("\n");
   

	
	return( apiOK );
}


// ---------------------------------------------------------------------------
// FUNCTION: To recover the certificate data using the specified RSA public key.
//	     (RSA Public Key Encryption - certificate verification)
// INPUT   : pIn  -- the certificate data. (2L-V)
// OUTPUT  : pOut -- the recovered data.   (2L-V)
// RETURN  : apiOK
//           apiFailed
// NOTE    : pIn & pOut can pointer to the same storage buffer.
// 	     This function is used by EMV L2 kernel only.
// ---------------------------------------------------------------------------
ULONG	api_rsa_recover( UCHAR *pIn, UCHAR *pOut )
{
ULONG	result = apiOK;
ULONG	input_len;
ULONG output_len;
int i;

//int	err;

  if(flag_load_key == 0)
      return apiFailed;


	input_len = *pIn + *(pIn+1)*256;
  if(	input_len % g_moduls_len != 0){
    output_len = g_moduls_len * ( (input_len / g_moduls_len) + 1);
    *pOut++ = output_len % 256;
    *pOut++ = output_len / 256;
     pIn += 2;
  }
  else{
	*pOut++ = *pIn++;	// input length = output length
	*pOut++ = *pIn++;	//
    //256 bytes
    // *pOut = *pIn;
    // *(pOut + 1) = *(pIn + 1);
    //32 bytes
    // *pOut = 0x20;
    // *(pOut + 1) = 0x00;
    // output_len = *pOut + *(pOut + 1) * 256;
    // pIn += 2;
    // pOut += 2;
  }

  printf("input_len:%d\n",input_len);
  printf("output_len:%d\n",output_len);

  printf("input:\n");
  for(i = 0; i < input_len; i++)
    printf("%02x",pIn[i]);
  printf("\n");

  struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "akcipher",
    .salg_name = "rsa"
  };


  while(input_len > 0){

    if(input_len > g_moduls_len){
      input_len -= g_moduls_len;
      result = socket_IO_to_rsa(sa, rsa_key_len, g_moduls_len, g_moduls_len, rsa_key,
                      pIn,pOut);

        pIn += g_moduls_len;
        pOut += g_moduls_len;

    }
    else{

      result = socket_IO_to_rsa(sa, rsa_key_len, input_len, g_moduls_len, rsa_key,
                      pIn,pOut);
      input_len -= input_len;
    }

    if(result != apiOK)
      return result;

  }
	
	return( result );
}


// ---------------------------------------------------------------------------
// FUNCTION: RSA encryption function.
//	     (1) RSA Public Key  (N,e) Encryption - certificate verification.
//	     (2) RSA Private Key (N,d) Decryption - digital signature.
//		 N: modulus
//		 e: public key exponent (generally used: 2, 3, 65537)
//		 d: private key exponent
// INPUT   : cipher	- in API_RSA structure.
// 	     	Mode	- encryption (0) or decryption (1).
//	     	ModLen	- size of modulus   (in bytes).
//	     	Modulus	- RSA key modulus.  (in big-endian)
//	     	ExpLen	- size of exponent  (in bytes )
//	     	Exponent- RSA key exponent. (in big-endian)
//	     	Length	- size of data to be transformed. (in bytes)
//	     	pIn	- data to be transformed.
// OUTPUT  : pOut	- data transformed.
// RETURN  : apiOK
//           apiFailed
// NOTE	   : (1) This is the RSA general purpose function.
//	     (2) Max length of modulus is 2048 bits (256 bytes).
//	     (3) Public key exponent shall be 2, 3 or 65536.
// ---------------------------------------------------------------------------
ULONG	api_rsa_encrypt( API_RSA cipher )
{
  
  // construct the key
  UINT modulus_len;
  UINT modulus_tag_len;
  UINT exponet_tag_len;
  //int	err;
  UINT total_tag_len;
  UINT total_len;
  int result;
  UINT cursor;
  // add for 0x00 for start
  modulus_len = cipher.ModLen + 1;
  if(modulus_len > 255)
     modulus_tag_len = 2;
  else
     modulus_tag_len = 1;
    
  if(cipher.ExpLen > 255)
     exponet_tag_len = 2;
  else
     exponet_tag_len = 1;
  
  total_tag_len =  2  + modulus_tag_len + modulus_len + 2 + exponet_tag_len + cipher.ExpLen;

  if(total_tag_len > 255)
      total_len = 2 +  2 + total_tag_len;
  else
      total_len = 2 +  1 + total_tag_len;

    // update global variable
    rsa_key_len = total_len;
    
       // header
   rsa_key[0] = 0x30;
   if(total_tag_len > 255){
     rsa_key[1] = 0x82;
     rsa_key[2] = total_tag_len / 256;
     rsa_key[3] = total_tag_len % 256;
     cursor = 4;
   }
   else{
     rsa_key[1] = 0x81;
     rsa_key[2] = total_tag_len;
     cursor = 3;
   }
   // modulus
   rsa_key[cursor++] = 0x02;
   
   if(modulus_len > 255){
     rsa_key[cursor++] = 0x82;
     rsa_key[cursor++] = modulus_len / 256;
     rsa_key[cursor++] = modulus_len % 256;
     rsa_key[cursor++] = 0x00;
   }
   else{
    rsa_key[cursor++] = 0x81;
     rsa_key[cursor++] = modulus_len;
     rsa_key[cursor++] = 0x00;
   }

   memmove(&rsa_key[cursor], cipher.Modulus, cipher.ModLen);
   cursor += (cipher.ModLen);
  // exponent 

  rsa_key[cursor++] = 0x02;

   if(cipher.ExpLen > 255){
     rsa_key[cursor++] = 0x82;
     rsa_key[cursor++] = cipher.ExpLen / 256;
     rsa_key[cursor++] = cipher.ExpLen % 256;
   }
   else{
    rsa_key[cursor++] = 0x81;
    rsa_key[cursor++] = cipher.ExpLen % 256;
   }

  memmove(&rsa_key[cursor], cipher.Exponent, cipher.ExpLen);

  //finish set key

  
   // debug only dump key
  //  int i =0;
  //  printf("total len:%d\n",rsa_key_len);
  //  printf("result key:\n");
  //   printf("\n\t");
  //   for(i = 0; i < rsa_key_len; i++){
  //       printf("%2x",rsa_key[i]);
  //       if((i+1) % 16 == 0){
  //           printf("\n");
  //           printf("\t");
  //       }
  //   }
  //   printf("\n");


  struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "akcipher",
    .salg_name = "rsa"
  };

  int input_len = cipher.Length;

  while(input_len > 0){

    if(input_len > cipher.ModLen){
      input_len -= cipher.ModLen;
      result = socket_IO_to_rsa(sa, rsa_key_len, cipher.ModLen, cipher.ModLen, rsa_key,
                      cipher.pIn, cipher.pOut);

        cipher.pIn += g_moduls_len;
        cipher.pOut += g_moduls_len;

    }
    else{

      result = socket_IO_to_rsa(sa, rsa_key_len, input_len, cipher.ModLen, rsa_key,
                      cipher.pIn, cipher.pOut);
      input_len -= input_len;
    }

    if(result != apiOK)
      return result;

  }
	
	return( result );

}
// ---------------------------------------------------------------------------
// FUNCTION: To encipher plaintext data (pIn) to enciphered data (pOut) by HMAC-SHA256.
// INPUT   : pIn        -- the plaintext data. 
//           InLength   -- pIn data length in byte
//           pKey       -- the key.    
//           KeyLength  -- The key length in byte
//           OutLength  -- Output data length in byte
// OUTPUT  : pOut       -- the ciphered data.
// RETURN  : apiOK
//           apiFailed
// ---------------------------------------------------------------------------
UCHAR	api_HMAC_SHA256_encipher( UCHAR *pIn, UCHAR InLength, UCHAR *pOut, UCHAR OutLength, UCHAR *pKey, UCHAR KeyLength)
{   
    struct sockaddr_alg sa = {
    .salg_family = AF_ALG,
    .salg_type = "hash",
    .salg_name ="hmac(sha256)"
     };

    return socket_IO_to_rsa(sa,KeyLength, InLength, OutLength,pKey,pIn,pOut);
}