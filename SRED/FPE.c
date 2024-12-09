//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL                                                    **
//**  PRODUCT  : AS350-X6                                                   **
//**                                                                        **
//**  FILE     : FPE.C                                                      **
//**  MODULE   : 			   				                                **
//**                                                                        **
//**  FUNCTION : Methods for Format-Preserving Encryption 				    **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : Tammy                                                      **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "bsp_types.h"
#include "POSAPI.h"
#include "OS_LIB.h"
#include "UTILS.h"

#include <ucl/ucl_defs.h>
#include <ucl/ucl_types.h>
#include <ucl/ucl_aes.h>
#include <ucl/ucl_aes_cbc.h>
#include <ucl/ucl_aes_ecb.h>

#include "SRED_DBG_Function.h"
#include "PEDKconfig.h"


// ---------------------------------------------------------------------------
// FUNCTION: Returns the smallest integer value that is greater than or equal to the real number
// INPUT   : value - the real number
// OUTPUT  : ceil  - the least integer that is not less than the real number
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FPE_Func_Ceiling(double value, int *ceil)
{
	int		iValue;

	
	iValue = (int)value;
	if(value == (double)iValue)
	{
		*ceil = iValue;
	}
	else
	{
		*ceil = iValue + 1;
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: Returns the largest integer value that is less than or equal to x
// INPUT   : value - the real number
// OUTPUT  : ceil  - The greatest integer that does not exceed the real number
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FPE_Func_Floor(double value, int *floor)
{
	*floor = (int)value;
}

// ---------------------------------------------------------------------------
// FUNCTION: Returns the base 2 logarithm of the real number x
// INPUT   : value - The real number > 0
// OUTPUT  : result - The base 2 logarithm of the real number x > 0
// RETURN  : none.
// ---------------------------------------------------------------------------
//void	FPE_Func_Log(int x, double *result)
//{
//	//*result = log2((double)x);
//}

// ---------------------------------------------------------------------------
// FUNCTION: NIST SP 800-38G notation: [x]<sup>s</sup>
//			 Expands x into an array of bytes of length s
// INPUT   : x         - the integer to convert to an array of bytes
//			 s         - the length of the output in bytes
// OUTPUT  : byteArray - given a nonnegative integer x less than 256<sup>s</sup> , 
//						 the representation of x as a string of s bytes
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FPE_Func_ByteString(int x, UINT16 s, UINT8 *byteArray)
{
	UINT16	i;


	for(i = 0 ; i < s ; i++)
	{
		*(byteArray + i) = (x >> (8 * (s - 1 - i))) & 0xFF;
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: NIST SP 800-38G Algorithm 1 : NUM<sub>radix</sub>(X)
//           Converts a string of numerals to an integer, valuing the numerals
//			 in decreasing order of significance
// INPUT   : radix  - the base of the numerals such that 0 &lt;= X[i] &lt; radix for all i
//			 dataIn - the string of numerals to convert to a number
//			 length	- length of the string of numerals
// OUTPUT  : number - The number that the numeral string X represents in base <i>radix</i> when the
//                    numerals are valued in decreasing order of significance
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FPE_Func_NUM(UINT8 radix, UINT8 *dataIn, UINT16 length, int *number)
{
	int		value;
	UINT16	i;


	//Let x = 0
	value = 0;

	for(i = 0 ; i < length ; i++)
	{
		//x = x * radix + X[i]
		value = value * radix + dataIn[i];
	}

	//Return x
	*number = value;
}

// ---------------------------------------------------------------------------
// FUNCTION: NIST SP 800-38G Algorithm 2: NUM(X)
//			 Converts a string of bytes to an integer, valuing the bytes as 
//           unsigned integers in decreasing order of significance
// INPUT   : dataIn - the string of bytes to convert to a number
//			 length	- length of the string of numerals
// OUTPUT  : number - The integer that a byte string X represents when the 
//					  bytes are valued in decreasing order of significance
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FPE_Func_NUM2(UINT8 *dataIn, UINT16 length, UINT64 *number)
{
	UINT16	i;
	UINT16	j;
	UINT64	powValue;
	

	powValue = 1;
	for(i = 1 ; i <= (2 * length - 1) ; i++)
	{
		powValue *= 16;
	}

	*number = 0;
	for(j = 0 ; j < length ; j++)
	{
		*number += ((dataIn[j] & 0xF0) >> 4) * powValue;

		powValue /= 16;

		*number += (dataIn[j] & 0x0F) * powValue;

		powValue /= 16;
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: NIST SP 800-38G Algorithm 3: STR<sup>m</sup><sub>radix</sub>(x)
//			 Converts an integer to an array of numerals of a given radix
// INPUT   : radix   - The base of the numerals such that 0 <= X[i] < radix for all i
//			 strLen	 - the length of the string of numerals
//			 dataIn  - integer, such that 0 <= x < radix<sup>m</sup>
// OUTPUT  : dataOut - numeral string
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FPE_Func_STR(UINT8 radix, UINT16 strLen, int dataIn, UINT8 *dataOut)
{
	int		temp;
	UINT16	i;


	temp = dataIn;

	for(i = 1 ; i <= strLen ; i++)
	{
		// X[m+1-i] = x mod radix
		dataOut[strLen - i] = temp % radix;

		//x = floor(x/radix)
		temp = (int)(temp / radix);
		
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: NIST SP 800-38G Algorithm 4: REV(X)
//			 Reverse a string of numerals
// INPUT   : dataIn - the numeral string to reverse
//			 length - length of numeral string
// OUTPUT  : dataOut - numeral string in reverse order
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FPE_Func_REV(UINT8 *dataIn, UINT16 length, UINT8 *dataOut)
{
	UINT16	i;

	//For i from 1 to LEN(X)
	for(i = 1 ; i <= length ; i++)
	{
		//Let Y[i] = X[LEN(X)+1-i]
		dataOut[i - 1] = dataIn[length - i];
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: NIST SP 800-38G Algorithm 6: PRF(X)
//			 Applies the pseudorandom function to the input using the supplied key
// INPUT   : K      - the AES key for the block cipher
//			 X      - block string
//			 strLen - length of block string
// OUTPUT  : Y      - the output of the function PRF applied to the block X
//                    PRF is defined in terms of a given designated cipher function
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FPE_Func_PRF(UINT8 *K, UINT8 *X, UINT16 strLen, UINT8 *Y)
{
	UINT8	m;
	UINT8	Xi[UCL_AES_BLOCKSIZE];
	UINT16	i,j;


	//Let m = LEN(X)/128, i.e. BYTELEN(X)/16
	m = strLen / UCL_AES_BLOCKSIZE;

	//Let the bit string Y consist of 128 consecutive ¡¥0¡¦ bits
	memset(Y, 0x00, 16);

	//Let X[1], ..., X[m] be the blocks for which X = X[1] || ... || X[m]
	//for i from 1 to m let Y(i) = CIPH(K,Y(i-1) xor X[i])
	for(i = 0 ; i < m ; i++)
	{
		//XOR
		for(j = 0 ; j < UCL_AES_BLOCKSIZE ; j++)
		{
			Xi[j] = Y[j] ^ X[i*UCL_AES_BLOCKSIZE + j];
		}

		api_aes_encipher(Xi, Y, K, UCL_AES_KEYLEN_128);
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: Equivalent implementation of the PRF(X) algorithm using the AES CBC
//			 cipher with a zero initialization vector
// INPUT   : K      - the AES key for the block cipher
//			 X      - block string
//			 strLen - length of block string
// OUTPUT  : Y      - the output of the function PRF applied to the block X
//                    PRF is defined in terms of a given designated cipher function
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FPE_Func_PRF2(UINT8 *K, UINT8 *X, UINT16 strLen, UINT8 *Y)
{
	UINT8	iv[UCL_AES_KEYLEN_128] = {0x00};

	api_aes_cbc_encipher(X, Y, K, UCL_AES_KEYLEN_128, iv, UCL_AES_KEYLEN_128);
}

// ---------------------------------------------------------------------------
// FUNCTION: Encrypts the input using the AES block cipher in ECB mode using 
//           the specified key
// INPUT   : K - the AES key for the cipher function
//			 X - the block string
//			 strLen - length of block string
// OUTPUT  : the output of the cipher function applied to the block X
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FPE_Func_CIPH(UINT8 *K, UINT8 *X, UINT16 strLen, UINT8 *Y)
{
	api_aes_encipher(X, Y, K, UCL_AES_KEYLEN_128);
}

// ---------------------------------------------------------------------------
// FUNCTION: NIST SP 800-38G Algorithm 7: FF1.Encrypt(K, T, X)
//           Encrypt a plaintext string of numerals and produce a ciphertext 
//			 string of numerals of the same length and radix
// INPUT   : radix - base
//			 K     - the 128-, 192- or 256-bit AES key
//			 T     - tweak T, a byte string of byte length t, such that t is
//			         in the range [0..maxTlen]
//			 tLen  - byte length t of tweak
//			 X     - the plaintext numeral string
//			 xLen  - length n of the plaintext numeral string
// OUTPUT  : Y     - the ciphertext numeral string of the same length and radix
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FPE_FF1_Encrypt(UINT8 radix, UINT8 *K, UINT8 *T, UINT16 tLen, UINT8 *X, UINT16 xLen, UINT8 *Y)
{
	UINT16	u;
	UINT16	v;
	UINT8	A[16];
	UINT16	aLen = 0;
	UINT8	B[16];
	UINT16	bLen = 0;
	int		temp;
	int		ceil;
	int		b;
	int		d;
	UINT8	P[16];
	UINT8	i;
	int		operand;
	UINT8	Q[32];
	UINT16	qLen = 0;
	int		num;
	UINT8	ptext[48];
	UINT8	R[16];
	UINT8	S[16];
	UINT8	blockStr[100];
	UINT16	blockStrLen = 0;
	UINT8	j;
	int		blockNum;
	UINT8	k;
	UINT8	ctext[16];
	UINT64	y;
	UINT8	m;
	int		c;
	UINT8	C[16];


	//=====================================================
	//Key = 2B 7E 15 16 28 AE D2 A6 AB F7 15 88 09 CF 4F 3C
	//Radix = 10
	//X = 0 1 2 3 4 5 6 7 8 9
    //Tweak = 39 38 37 36 35 34 33 32 31 30
	//=====================================================

	SRED_DBG_Put_String(13, (UINT8*)"==== Key ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(16, K);	// ==== [Debug] ====

	SRED_DBG_Put_String(15, (UINT8*)"==== Radix ====");	// ==== [Debug] ====
	SRED_DBG_Put_UCHAR(radix);	// ==== [Debug] ====

	SRED_DBG_Put_String(19, (UINT8*)"==== Plaintext ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(xLen, X);	// ==== [Debug] ====

	SRED_DBG_Put_String(15, (UINT8*)"==== Tweak ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(tLen, T);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	//Step 1. Let u = floor(n/2); v = n - u
	FPE_Func_Floor(xLen / 2, (int*)&u);
	v = xLen - u;

	SRED_DBG_Put_String(13, (UINT8*)"Step 1 ---> u");	// ==== [Debug] ====
	SRED_DBG_Put_UINT(u);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	SRED_DBG_Put_String(13, (UINT8*)"Step 1 ---> v");	// ==== [Debug] ====
	SRED_DBG_Put_UINT(v);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	//Step 2. Let A = X[1..u]; B = X[u + 1..n]
	memcpy(A, X, u);
	aLen = u;
	memcpy(B, &X[u], v);
	bLen = v;

	SRED_DBG_Put_String(13, (UINT8*)"Step 2 ---> A");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(aLen, A);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	SRED_DBG_Put_String(13, (UINT8*)"Step 2 ---> B");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(bLen, B);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	//Step 3. Let b = ceiling(ceiling(v * LOG(radix))/8)
	FPE_Func_Ceiling((v*log2(10)), &ceil);
	//FPE_Func_Ceiling(v*log2((double)radix), &ceil);
	FPE_Func_Ceiling(((double)ceil / 8.0), &b);

	SRED_DBG_Put_String(13, (UINT8*)"Step 3 ---> b");	// ==== [Debug] ====
	SRED_DBG_Put_UINT(b);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	//Step 4. Let d = 4 * ceiling(b/4) + 4
	FPE_Func_Ceiling(((double)b / 4.0), &temp);
	d = 4 * temp + 4;

	SRED_DBG_Put_String(13, (UINT8*)"Step 4 ---> d");	// ==== [Debug] ====
	SRED_DBG_Put_UINT(d);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	//Step 5. Let P = [1]^1 || [2]^1 || [1]^1 || [radix]^3 || [10]^1 || [u mod 256]^1 || [n]^4 || [t]^4
	FPE_Func_ByteString(1, 1, P);				//[1]^1
	FPE_Func_ByteString(2, 1, &P[1]);			//[2]^1
	FPE_Func_ByteString(1, 1, &P[2]);			//[1]^1
	FPE_Func_ByteString(radix, 3, &P[3]);		//[radix]^3
	FPE_Func_ByteString(10, 1, &P[6]);			//[10]^1
	FPE_Func_ByteString((u % 256), 1, &P[7]);	//[u mod 256]^1
	FPE_Func_ByteString(xLen, 4, &P[8]);		//[n]^4
	FPE_Func_ByteString(tLen, 4, &P[12]);		//[t]^4

	SRED_DBG_Put_String(13, (UINT8*)"Step 5 ---> P");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(16, P);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	//Step 6. For i from 0 to 9
	for(i = 0 ; i < 10 ; i++)
	{
		SRED_DBG_Put_String(7, (UINT8*)"Round #");	// ==== [Debug] ====
		SRED_DBG_Put_UCHAR(i);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step i. Let Q = T || [0]^(-t-b-1) mod 16 || [i]^1 || [NUMradix (B)]^b
		//Clear data
		qLen = 0;

		memcpy(Q, T, tLen);	//T
		qLen += tLen;

		//C has a remainder operation (e.g. -14 % 16 = -14).
		//It has a direct relation with how the negative integer division is handled, the C's behavior is consistent
		//with rounding towards 0.
		//"a = b * q + r"
		//Do "((a % b) + b) % b" to get the same result as true modulo operation (e.g. -14 % 16 = 2).
		operand = -tLen - b - 1;
		FPE_Func_ByteString(0, ((operand % 16) + 16) % 16, &Q[qLen]);	//[0]^(-t-b-1) mod 16
		qLen += ((operand % 16) + 16) % 16;

		FPE_Func_ByteString((int)i, 1, &Q[qLen]);	//[i]^1
		qLen++;

		FPE_Func_NUM(radix, B, bLen, &num);
		FPE_Func_ByteString(num, b, &Q[qLen]);	//[NUMradix (B)]^b
		qLen += b;

		SRED_DBG_Put_String(15, (UINT8*)"Step 6.i ---> Q");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(qLen, Q);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step ii. Let R = PRF(P || Q)
		memset(ptext, 0x00, sizeof(ptext));
		memcpy(ptext, P, 16);
		memcpy(&ptext[16], Q, qLen);
		FPE_Func_PRF(K, ptext, (16 + qLen), R);

		SRED_DBG_Put_String(16, (UINT8*)"Step 6.ii ---> R");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(16, R);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step iii. Let S be the first d bytes of the following string of ceiling(d/16) blocks:
		//R || CIPH K (R xor [1]^16 ) || CIPH K (R xor [2]^16 ) ... CIPH K (R xor [ceiling(d/16)-1]^16 )
		memcpy(blockStr, R, 16);
		blockStrLen += 16;

		FPE_Func_Ceiling((d / 16), &blockNum);
		for(j = 1 ; j <= (blockNum - 1) ; j++)
		{
			FPE_Func_ByteString(j, 16, ptext);	//[j]^16
			for(k = 0 ; k < 16 ; k++)
			{
				ptext[k] ^= R[k];	//R xor [j]^16
			}

			FPE_Func_CIPH(K, ptext, 16, ctext);	//CIPH K (R xor [j]^16 )
			memcpy(&blockStr[blockStrLen], ctext, 16);
			blockStrLen += 16;
		}

		memcpy(S, blockStr, d);

		SRED_DBG_Put_String(17, (UINT8*)"Step 6.iii ---> S");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(d, S);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step iv. Let y = NUM(S)
		FPE_Func_NUM2(S, d, &y);

		//SRED_DBG_Put_String(16, (UINT8*)"Step 6.iv ---> y");	// ==== [Debug] ====
		//dbgBufLen = UTIL_itoa(y, dbgBuf);	// ==== [Debug] ====
		//SRED_DBG_Put_Hex(dbgBufLen, dbgBuf);	// ==== [Debug] ====
		//SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step v. If i is even, let m = u; else, let m = v
		if(i % 2 == 0)
		{
			m = u;
		}
		else
		{
			m = v;
		}

		SRED_DBG_Put_String(15, (UINT8*)"Step 6.v ---> m");	// ==== [Debug] ====
		SRED_DBG_Put_UCHAR(m);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step vi. Let c = (NUMradix (A)+y) mod radix^m
		FPE_Func_NUM(radix, A, aLen, &num);

		temp = 1;
		for(j = 1 ; j <= m ; j++)
		{
			temp = temp * radix;
		}

		c = (num + y) % temp;

		SRED_DBG_Put_String(16, (UINT8*)"Step 6.vi ---> c");	// ==== [Debug] ====
		SRED_DBG_Put_UINT((UINT16)c);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step vii. Let C = STR m radix (c)
		FPE_Func_STR(radix, m, c, C);

		SRED_DBG_Put_String(17, (UINT8*)"Step 6.vii ---> C");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(m, C);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step viii. Let A = B
		memcpy(A, B, bLen);
		aLen = bLen;

		SRED_DBG_Put_String(18, (UINT8*)"Step 6.viii ---> A");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(aLen, A);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step ix. Let B = C
		memcpy(B, C, m);
		bLen = m;

		SRED_DBG_Put_String(16, (UINT8*)"Step 6.ix ---> B");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(bLen, B);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====
	}

	//Step 7. Return A || B
	memcpy(Y, A, aLen);
	memcpy(&Y[aLen], B, bLen);

	SRED_DBG_Put_String(13, (UINT8*)"Step 7 ---> Y");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(xLen, Y);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====
}

// ---------------------------------------------------------------------------
// FUNCTION: NIST SP 800-38G Algorithm 8: FF1.Decrypt(K, T, X)
//			 Decrypt a ciphertext string of numerals and produce a plaintext 
//			 string of numerals of the same length and radix
// INPUT   : radix - base
//			 K     - the 128-, 192- or 256-bit AES key
//			 T     - tweak T, a byte string of byte length t, such that t is
//			         in the range [0..maxTlen]
//			 tLen  - byte length t of tweak
//			 X     - the ciphertext numeral string
//			 xLen  - length n of the ciphertext numeral string
// OUTPUT  : Y     - the plaintext numeral string of the same length and radix
// RETURN  : none.
// ---------------------------------------------------------------------------
void	FPE_FF1_Decrypt(UINT8 radix, UINT8 *K, UINT8 *T, UINT16 tLen, UINT8 *X, UINT16 xLen, UINT8 *Y)
{
	UINT16	u;
	UINT16	v;
	UINT8	A[16];
	UINT16	aLen = 0;
	UINT8	B[16];
	UINT16	bLen = 0;
	int		temp;
	int		ceil;
	int		b;
	int		d;
	UINT8	P[16];
	int		i;
	int		operand;
	UINT8	Q[32];
	UINT16	qLen = 0;
	int		num;
	UINT8	ptext[48];
	UINT8	R[16];
	UINT8	S[16];
	UINT8	blockStr[100];
	UINT16	blockStrLen = 0;
	UINT8	j;
	int		blockNum;
	UINT8	k;
	UINT8	ctext[16];
	UINT64	y;
	UINT8	m;
	int		c;
	UINT8	C[16];


	//=====================================================
	//Key = 2B 7E 15 16 28 AE D2 A6 AB F7 15 88 09 CF 4F 3C
	//Radix = 10
	//X = 6 1 2 4 2 0 0 7 7 3
    //Tweak = 39 38 37 36 35 34 33 32 31 30
	//=====================================================

	SRED_DBG_Put_String(13, (UINT8*)"==== Key ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(16, K);	// ==== [Debug] ====

	SRED_DBG_Put_String(15, (UINT8*)"==== Radix ====");	// ==== [Debug] ====
	SRED_DBG_Put_UCHAR(radix);	// ==== [Debug] ====

	SRED_DBG_Put_String(20, (UINT8*)"==== Ciphertext ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(xLen, X);	// ==== [Debug] ====

	SRED_DBG_Put_String(15, (UINT8*)"==== Tweak ====");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(tLen, T);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	//Step 1. Let u = floor(n/2); v = n - u
	FPE_Func_Floor(xLen / 2, (int*)&u);
	v = xLen - u;

	SRED_DBG_Put_String(13, (UINT8*)"Step 1 ---> u");	// ==== [Debug] ====
	SRED_DBG_Put_UINT(u);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	SRED_DBG_Put_String(13, (UINT8*)"Step 1 ---> v");	// ==== [Debug] ====
	SRED_DBG_Put_UINT(v);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	//Step 2. Let A = X[1..u]; B = X[u + 1..n]
	memcpy(A, X, u);
	aLen = u;
	memcpy(B, &X[u], v);
	bLen = v;

	SRED_DBG_Put_String(13, (UINT8*)"Step 2 ---> A");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(aLen, A);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	SRED_DBG_Put_String(13, (UINT8*)"Step 2 ---> B");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(bLen, B);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	//Step 3. Let b = ceiling(ceiling(v * LOG(radix))/8)
	FPE_Func_Ceiling((v*log2(10)), &ceil);
	//FPE_Func_Ceiling(v*log2((double)radix), &ceil);
	FPE_Func_Ceiling(((double)ceil / 8.0), &b);

	SRED_DBG_Put_String(13, (UINT8*)"Step 3 ---> b");	// ==== [Debug] ====
	SRED_DBG_Put_UINT(b);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	//Step 4. Let d = 4 * ceiling(b/4) + 4
	FPE_Func_Ceiling(((double)b / 4.0), &temp);
	d = 4 * temp + 4;

	SRED_DBG_Put_String(13, (UINT8*)"Step 4 ---> d");	// ==== [Debug] ====
	SRED_DBG_Put_UINT(d);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	//Step 5. Let P = [1]^1 || [2]^1 || [1]^1 || [radix]^3 || [10]^1 || [u mod 256]^1 || [n]^4 || [t]^4
	FPE_Func_ByteString(1, 1, P);				//[1]^1
	FPE_Func_ByteString(2, 1, &P[1]);			//[2]^1
	FPE_Func_ByteString(1, 1, &P[2]);			//[1]^1
	FPE_Func_ByteString(radix, 3, &P[3]);		//[radix]^3
	FPE_Func_ByteString(10, 1, &P[6]);			//[10]^1
	FPE_Func_ByteString((u % 256), 1, &P[7]);	//[u mod 256]^1
	FPE_Func_ByteString(xLen, 4, &P[8]);		//[n]^4
	FPE_Func_ByteString(tLen, 4, &P[12]);		//[t]^4

	SRED_DBG_Put_String(13, (UINT8*)"Step 5 ---> P");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(16, P);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

	//Step 6. For i from 9 to 0
	for(i = 9 ; i >= 0 ; i--)
	{
		SRED_DBG_Put_String(7, (UINT8*)"Round #");	// ==== [Debug] ====
		SRED_DBG_Put_UCHAR((UINT8)i);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step i. Let Q = T || [0]^(-t-b-1) mod 16 || [i]^1 || [NUMradix (A)]^b
		//Clear data
		qLen = 0;

		memcpy(Q, T, tLen);	//T
		qLen += tLen;

		//C has a remainder operation (e.g. -14 % 16 = -14).
		//It has a direct relation with how the negative integer division is handled, the C's behavior is consistent
		//with rounding towards 0.
		//"a = b * q + r"
		//Do "((a % b) + b) % b" to get the same result as true modulo operation (e.g. -14 % 16 = 2).
		operand = -tLen - b - 1;
		FPE_Func_ByteString(0, ((operand % 16) + 16) % 16, &Q[qLen]);	//[0]^(-t-b-1) mod 16
		qLen += ((operand % 16) + 16) % 16;

		FPE_Func_ByteString(i, 1, &Q[qLen]);	//[i]^1
		qLen++;

		FPE_Func_NUM(radix, A, aLen, &num);
		FPE_Func_ByteString(num, b, &Q[qLen]);	//[NUMradix (A)]^b
		qLen += b;

		SRED_DBG_Put_String(15, (UINT8*)"Step 6.i ---> Q");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(qLen, Q);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step ii. Let R = PRF(P || Q)
		memset(ptext, 0x00, sizeof(ptext));
		memcpy(ptext, P, 16);
		memcpy(&ptext[16], Q, qLen);
		FPE_Func_PRF(K, ptext, (16 + qLen), R);

		SRED_DBG_Put_String(16, (UINT8*)"Step 6.ii ---> R");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(16, R);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step iii. Let S be the first d bytes of the following string of ceiling(d/16) blocks:
		//R || CIPH K (R xor [1]^16 ) || CIPH K (R xor [2]^16 ) ... CIPH K (R xor [ceiling(d/16)-1]^16 )
		memcpy(blockStr, R, 16);
		blockStrLen += 16;

		FPE_Func_Ceiling((d / 16), &blockNum);
		for(j = 1 ; j <= (blockNum - 1) ; j++)
		{
			FPE_Func_ByteString(j, 16, ptext);	//[j]^16
			for(k = 0 ; k < 16 ; k++)
			{
				ptext[k] ^= R[k];	//R xor [j]^16
			}

			FPE_Func_CIPH(K, ptext, 16, ctext);	//CIPH K (R xor [j]^16 )
			memcpy(&blockStr[blockStrLen], ctext, 16);
			blockStrLen += 16;
		}

		memcpy(S, blockStr, d);

		SRED_DBG_Put_String(17, (UINT8*)"Step 6.iii ---> S");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(d, S);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step iv. Let y = NUM(S)
		FPE_Func_NUM2(S, d, &y);

		//SRED_DBG_Put_String(16, (UINT8*)"Step 6.iv ---> y");	// ==== [Debug] ====
		//dbgBufLen = UTIL_itoa(y, dbgBuf);	// ==== [Debug] ====
		//SRED_DBG_Put_Hex(dbgBufLen, dbgBuf);	// ==== [Debug] ====
		//SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step v. If i is even, let m = u; else, let m = v
		if(i % 2 == 0)
		{
			m = u;
		}
		else
		{
			m = v;
		}

		SRED_DBG_Put_String(15, (UINT8*)"Step 6.v ---> m");	// ==== [Debug] ====
		SRED_DBG_Put_UCHAR(m);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step vi. Let c = (NUMradix (B)-y) mod radix^m
		FPE_Func_NUM(radix, B, bLen, &num);

		temp = 1;
		for(j = 1 ; j <= m ; j++)
		{
			temp = temp * radix;
		}

		//Negative Big Integer modulo operation : a = bq + r
		//The dividend "NUMradix (B)-y" is denoted by a (negative Big Integer)
		//The divisor "radix^m" is denoted by b
		//The quotient is denoted by q
		//The remainder is denoted by r
		//The quotient of the negative Big Integer modulo operation is derived from that converting negative dividend to
		//a positive one and doing a modulo operation to get the quotient of positive Big Integer modulo operation
		//(a * (-1) / b + 1) * (-1) = q
		c = ((UINT64)num - y) - (temp * (((-1)*((UINT64)num - y) / temp)+1) * (-1));

		SRED_DBG_Put_String(16, (UINT8*)"Step 6.vi ---> c");	// ==== [Debug] ====
		SRED_DBG_Put_UINT((UINT16)c);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step vii. Let C = STR m radix (c)
		FPE_Func_STR(radix, m, c, C);

		SRED_DBG_Put_String(17, (UINT8*)"Step 6.vii ---> C");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(m, C);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step viii. Let B = A
		memcpy(B, A, aLen);
		bLen = aLen;

		SRED_DBG_Put_String(18, (UINT8*)"Step 6.viii ---> B");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(bLen, B);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====

		//Step ix. Let A = C
		memcpy(A, C, m);
		aLen = m;

		SRED_DBG_Put_String(16, (UINT8*)"Step 6.ix ---> A");	// ==== [Debug] ====
		SRED_DBG_Put_Hex(aLen, A);	// ==== [Debug] ====
		SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====
	}

	//Step 7. Return A || B
	memcpy(Y, A, aLen);
	memcpy(&Y[aLen], B, bLen);

	SRED_DBG_Put_String(13, (UINT8*)"Step 7 ---> Y");	// ==== [Debug] ====
	SRED_DBG_Put_Hex(xLen, Y);	// ==== [Debug] ====
	SRED_DBG_Put_String(0, (UINT8*)"");	// ==== [Debug] ====
}
