//
//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : PCI PED (V6.2)						    **
//**  PRODUCT  : AS350 PLUS						    **
//**                                                                        **
//**  FILE     : DUKPT-AES.C (cf. ANS X9.24-3-2017 & ISO 9564-1:2017)	    **
//**  MODULE   :                                                            **
//**                                                                        **
//**                                                                        **
//**  VERSION  : V1.00                                                      **
//**  DATE     : 2023/03/30                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2005-2023 SymLink Corporation. All rights reserved.      **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include "bsp_types.h"
#include "bsp_cipher.h"
#include "POSAPI.H"
#include "OS_SECM.h"

//#include "EMVDC.H"
//#include "GDATAEX.H"
//#include "EMVAPI.H"
//#include "TOOLS.H"

#define	UCHAR	UINT8
#define	UINT	UINT16
#define	ULONG	UINT32

extern  UINT8   *ped_pin_ptr;
extern	void	PED_GenEncPinBlock_ISO4( UCHAR *pindata, UCHAR *pandata, UCHAR *key, UCHAR *epb );	// defined in OS_PED3.c


// ----------------------------------------------------------------------------
//	AES DUKPT
// ----------------------------------------------------------------------------
#define	_USE_SECM_		// comment this tag if using external memory for DEBUG purpose

#define	NUM_REG			32
#define	MAX_WORK		16
#define	KEYLENGTH		32


// DerivationPurpose identifies if this derivation is to create an initial key
// or any other key type to help select which derivation data table to use
enum	DerivationPurpose
{
	_Initial_Key_,
	_Derivation_or_Working_Key_
};

// KeyUsage defines the possible key usages that can be derived
enum	KeyUsage
{
	_Key_Encryption_Key_,
	_PIN_Encryption_,
	_Message_Authentication_generation_,
	_Message_Authentication_verification_,
	_Message_Authentication_both_ways_,
	_Data_Encryption_encrypt_,
	_Data_Encryption_decrypt_,
	_Data_Encryption_both_ways_,
	_Key_Derivation_,
	_Key_Derivation_Initial_Key_
};

// KeyType defines the cryptographic key type being derived
enum	KeyType
{
	_2TDEA_,
	_3TDEA_,
	_AES128_,
	_AES192_,
	_AES256_
};


UCHAR   trueValue = 1;
UCHAR   falseValue = 0;

#ifndef	_USE_SECM_
// IntermediateDerivationKeyRegister
//	a set of registers used to store intermediate derivation keys
UCHAR	IntermediateDerivationKeyRegister[NUM_REG][KEYLENGTH] = {0};
#endif

// IntermediateDerivationKeyInUse
//	a set of Booleans used to store whether the corresponding 
//	IntermediateDerivationKeyRegister has a valid value
UCHAR	IntermediateDerivationKeyInUse[NUM_REG] = {0};

// CurrentDerivationKey
// 	contains the index of the Intermediate Derivation Key Register whose contents
// 	are being used in the current cryptographic operation
ULONG	g_CurrentDerivationKey = 0;	// gCurrentKey

// TransactionCounter
//	defines a counter of the number of transactions that have occurred since the initial key was first loaded
ULONG	g_TransactionCounter = 0;	// gCounter;

// ShiftRegister
//	a 32-bit register. 
//	This register normally contains 31 "zero" bits and a single "one" bit. 
//	This register is used to select one of the Intermediate Derivation Key Registers. 
//	Its value is 1 << CurrentDerivationKey.
ULONG	g_ShiftRegister;		// gShiftRegister

// InitialKeyID
//	In systems without KSN compatibility requirements, 
//		this value is injected as a 64-bit Initial Key ID.
//	In devices with KSN compatibility mode, 
//		this register will hold the left-most 64 bits of the initial key serial number 
//		injected into the Transaction-originating device along with the initial key during the "Load Initial Key" command
//
//	IKID:	Initial Key ID[64b] = BDK ID[32b] + derivation ID[32b]
//		KSN[96b] = IKID[64b] + Transaction Counter[32b]
UCHAR	g_InitialKeyID[8];		// gDeviceID;

// initialKeyType
//	The initialKeytype must be one of _AES128_, _AES192_, or _AES256_ 
//	and defines the type of the derivation keys to use
//	(aka ��intermediate derivation keys��)
ULONG	g_InitialKeyType;		// gDeriveKeyType

UCHAR	g_derivedKey[32];
UCHAR	g_DerivationData[16];		// refer to Table-2, Table-3



// ---------------------------------------------------------------------------
// FUNCTION: given a key K and a 16-byte value Y, 
//	     return the AES ECB encryption of X as a 16-byte array.
// INPUT   : 
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
void	AES_Encrypt_ECB( UCHAR *K, UCHAR *Y, UCHAR *X )
{
	api_aes_encipher( Y, X, K, 16 );
}

// ---------------------------------------------------------------------------
// FUNCTION: given a key K and a 16-byte value Y, 
//	     return the AES ECB decryption of X as a 16-byte array.
// INPUT   : 
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
void	AES_Decrypt_ECB(UCHAR *K, UCHAR *Y, UCHAR *X )
{
	api_aes_decipher( X, Y, K, 16 );
}

// ---------------------------------------------------------------------------
// FUNCTION: used to convert the key type into a key length (in bits).
// INPUT   : 
// OUTPUT  : none.
// RETURN  : key length
// ---------------------------------------------------------------------------
/*
Key_Length(keyType)
{
if (keyType == _2TDEA_) { return 128; }
else if (keyType == _3TDEA_) { return 192; }
else if (keyType == _AES128_) { return 128; }
else if (keyType == _AES192_) { return 192; }
else if (keyType == _AES256_) { return 256; }
}
*/
// ---------------------------------------------------------------------------
ULONG	Key_Length( ULONG keyType )
{
	if (keyType == _2TDEA_) { return 128; }
	else if (keyType == _3TDEA_) { return 192; }
	else if (keyType == _AES128_) { return 128; }
	else if (keyType == _AES192_) { return 192; }
	else if (keyType == _AES256_) { return 256; }
}

// ---------------------------------------------------------------------------
// FUNCTION: takes the derivation key, 
//	     the desired output length of the key to be derived, 
//	     and the derivation data (defined in the next section) 
//	     and outputs a derived key equal in length to the derivation key.
// INPUT   : derivationKey
//	     keyType
//	     derivationData
// OUTPUT  : derivedKey
// RETURN  : none
// ---------------------------------------------------------------------------
/*
Derive_Key(derivationKey, keyType, derivationData)
{
L = Key_Length(keyType);
n = ceil(L/128); // number of blocks required to construct the derived key
	for (i=1;i<=n;i++) {
	// Set the value of the derivation data key block counter field equal to
	// the block count being derived.
	// First block is 0x01, second block is 0x02.
	derivationData[1] = i;
	result[(i-1)*16..i*16-1] = AES_Encrypt_ECB(derivationKey, derivationData);
	}
derivedKey = result[0..(L/8)-1];
return derivedKey;
}
*/
// ---------------------------------------------------------------------------
//UCHAR	*Derive_Key( UCHAR *derivationKey, ULONG keyType, UCHAR *derivationData )
void	Derive_Key( UCHAR *derivationKey, ULONG keyType, UCHAR *derivationData, UCHAR *derivedKey )
{
ULONG	i;
ULONG	L;
ULONG	n;
ULONG	cnt;
ULONG	left_bytes;
UCHAR	temp[16];


	L = Key_Length( keyType );
//	n = ceil(L/128);	// min integer not less than input data
        n = L/128;
        if( L%128 )
          n++;
        
	for( i=1; i<=n; i++ )
	   {
	   derivationData[1] = i;
	   AES_Encrypt_ECB(derivationKey, derivationData, temp );
	   memmove( &derivedKey[(i-1)*16], temp, sizeof(temp) );
	   
#ifndef	_USE_SECM_
	   memmove( &g_derivedKey[(i-1)*16], temp, sizeof(temp) );
#endif
	   }
	   
//	return( g_derivedKey );
}

// ---------------------------------------------------------------------------
// FUNCTION: creates the derivation data according to Table 2 and Table 3 
//	     depending on the derivationPurpose.
//	     The inputs of "keyUsage" and "derivedKeyType" help define derivation data for the key being derived, 
//	     and "initialKeyID" and transaction "counter" are also used in the derivation data block.
//		Initial Key ID[64b] = BDK ID[32b] + derivation ID[32b]
//		Transaction Counter[32b]
// INPUT   : 
// OUTPUT  : DerivationData
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
/*
Create_Derivation_Data(derivationPurpose, keyUsage, derivedKeyType, initialKeyID, counter)
{
// Set Version ID of the table structure.
DerivationData[0] = 0x01; // version 1
// set Key Block Counter
DerivationData[1] = 0x01; // 1 for first block, 2 for second, etc.

	// set Key Usage Indicator
	if (keyUsage == _Key-Encryption-Key_) {
		DerivationData[2..3] = 0x0002; }
	else if (keyUsage == _PIN-Encryption_) {
		DerivationData[2..3] = 0x1000; }
	else if (keyUsage == _Message-Authentication-generation_) {
		DerivationData[2..3] = 0x2000; }
	else if (keyUsage == _Message-Authentication-verification_) {
		DerivationData[2..3] = 0x2001; }
	else if (keyUsage == _Message-Authentication-both-ways_) {
		DerivationData[2..3] = 0x2002; }
	else if (keyUsage == _Data-Encryption-encrypt_) {
		DerivationData[2..3] = 0x3000; }
	else if (keyUsage == _Data-Encryption-decrypt_) {
		DerivationData[2..3] = 0x3001; }
	else if (keyUsage == _Data-Encryption-both-ways_) {
		DerivationData[2..3] = 0x3002; }
	else if (keyUsage == _Key-Derivation_) {
		DerivationData[2..3] = 0x8000; }
	else if (keyUsage == _Key-Derivation-Initial-Key_) {
		DerivationData[2..3] = 0x8001; }
		
	// set Algorithm Indicator and key size
	if (derivedKeyType == _2TDEA_) { // Note: 2TDEA and 3TDEA are included
		DerivationData[4..5] = 0x0000; } // as an option for working keys
	else if (derivedKeyType == _3TDEA_) { // and must not be used for
		DerivationData[4..5] = 0x0001; } // initial keys or derivation keys
	else if (derivedKeyType == _AES128_) {
		DerivationData[4..5] = 0x0002; }
	else if (derivedKeyType == _AES192_) {
		DerivationData[4..5] = 0x0003; }
	else if (derivedKeyType == _AES256_) {
		DerivationData[4..5] = 0x0004; }
		
	// set length of key material being generated
	if (derivedKeyType == _2TDEA_) { // Note: 2TDEA and 3TDEA are included
		DerivationData[6..7] = 0x0080; } // as an option for working keys
	else if (derivedKeyType == _3TDEA_) { // and must not be used for
		DerivationData[6..7] = 0x00C0; } // initial keys or derivation keys
	else if (derivedKeyType == _AES128_) {
		DerivationData[6..7] = 0x0080; }
	else if (derivedKeyType == _AES192_) {
		DerivationData[6..7] = 0x00C0; }
	else if (derived_key_type == _AES256_) {
		DerivationData[6..7] = 0x0100; }
		
	// next 8 bytes depend on the derivation purpose
	if (derivationPurpose == _Initial-Key_) {
		DerivationData[8..15] = initialKeyID[0..7];
	}
	else if (derivationPurpose == _Derivation-or-Working-Key_) {
		DerivationData[8..11] = initialKeyID[4..7];
		DerivationData[12..15] = counter[0..3];
	}
Return DerivationData;
}
*/
// ---------------------------------------------------------------------------
//UCHAR	*Create_Derivation_Data( ULONG derivationPurpose, ULONG keyUsage, ULONG derivedKeyType, UCHAR *initialKeyID, ULONG counter )
UCHAR	Create_Derivation_Data( ULONG derivationPurpose, ULONG keyUsage, ULONG derivedKeyType, UCHAR *initialKeyID, ULONG counter, UCHAR *DerivationData )
{
//UCHAR	DerivationData[16];	// refer to Table-2, Table-3


	// Set Version ID of the table structure.
	DerivationData[0] = 0x01; // version 1
	
	// set Key Block Counter
	DerivationData[1] = 0x01; // 1 for first block, 2 for second, etc.
	
	// set Key Usage Indicator
	if (keyUsage == _Key_Encryption_Key_)  {
		DerivationData[2] = 0x00;
		DerivationData[3] = 0x02; }
	else if (keyUsage == _PIN_Encryption_) {
		DerivationData[2] = 0x10; 
		DerivationData[3] = 0x00;}
	else if (keyUsage == _Message_Authentication_generation_) {
		DerivationData[2] = 0x20; 
		DerivationData[3] = 0x00; }
	else if (keyUsage == _Message_Authentication_verification_) {
		DerivationData[2] = 0x20;
		DerivationData[3] = 0x01; }
	else if (keyUsage == _Message_Authentication_both_ways_) {
		DerivationData[2] = 0x20;
		DerivationData[3] = 0x02; }
	else if (keyUsage == _Data_Encryption_encrypt_) {
		DerivationData[2] = 0x30;
		DerivationData[3] = 0x00; }
	else if (keyUsage == _Data_Encryption_decrypt_) {
		DerivationData[2] = 0x30;
		DerivationData[3] = 0x01; }
	else if (keyUsage == _Data_Encryption_both_ways_) {
		DerivationData[2] = 0x30;
		DerivationData[3] = 0x02; }
	else if (keyUsage == _Key_Derivation_) {
		DerivationData[2] = 0x80;
		DerivationData[3] = 0x00; }
	else if (keyUsage == _Key_Derivation_Initial_Key_) {
		DerivationData[2] = 0x80;
		DerivationData[3] = 0x01; }
	     else
	       return( 0 );	// assert False

	// set Algorithm Indicator and key size
	if (derivedKeyType == _2TDEA_) { // Note: 2TDEA and 3TDEA are included
		DerivationData[4] = 0x00;
		DerivationData[5] = 0x00; } // as an option for working keys
	else if (derivedKeyType == _3TDEA_) { // and must not be used for
		DerivationData[4] = 0x00;
		DerivationData[5] = 0x01; } // initial keys or derivation keys
	else if (derivedKeyType == _AES128_) {
		DerivationData[4] = 0x00;
		DerivationData[5] = 0x02; }
	else if (derivedKeyType == _AES192_) {
		DerivationData[4] = 0x00;
		DerivationData[5] = 0x03; }
	else if (derivedKeyType == _AES256_) {
		DerivationData[4] = 0x00;
		DerivationData[5] = 0x04; }
	     else
	       return( 0 );	// assert False

	// set length of key material being generated
	if (derivedKeyType == _2TDEA_) { // Note: 2TDEA and 3TDEA are included
		DerivationData[6] = 0x00;
		DerivationData[7] = 0x80; } // as an option for working keys
	else if (derivedKeyType == _3TDEA_) { // and must not be used for
		DerivationData[6] = 0x00;
		DerivationData[7] = 0xC0; } // initial keys or derivation keys
	else if (derivedKeyType == _AES128_) {
		DerivationData[6] = 0x00;
		DerivationData[7] = 0x80; }
	else if (derivedKeyType == _AES192_) {
		DerivationData[6] = 0x00;
		DerivationData[7] = 0xC0; }
	else if (derivedKeyType == _AES256_) {
		DerivationData[6] = 0x01;
		DerivationData[7] = 0x00; }
	     else
	       return( 0 );	// assert False

	// next 8 bytes depend on the derivation purpose
	if (derivationPurpose == _Initial_Key_) {
//		DerivationData[8..15] = initialKeyID[0..7];
		memmove( &DerivationData[8], &initialKeyID[0], 8 ); }
	else if (derivationPurpose == _Derivation_or_Working_Key_) {
//		DerivationData[8..11] = initialKeyID[4..7];
		memmove( &DerivationData[8], &initialKeyID[4], 4 );
//		DerivationData[12..15] = counter[0..3];
		DerivationData[12] = (counter & 0xff000000) >> 24;
		DerivationData[13] = (counter & 0x00ff0000) >> 16;
		DerivationData[14] = (counter & 0x0000ff00) >> 8;
		DerivationData[15] = (counter & 0x000000ff); }
	     else
	       return( 0 );	// assert False

#ifndef	_USE_SECM_
	memmove( g_DerivationData, DerivationData, sizeof(g_DerivationData) );
#endif

	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Updates the intermediate derivation key registers based on
//	     the current values of the ShiftRegister, TranactionCounter, 
//	     CurrentDerivationKey, InitialKeyType, and InitialKeyID.
//
//	     The first time this function is called, start = NUM_REG-1 and all the registers are initialized
//	     When it is called after a transaction key is generated, start = CurrentDerivationKey
//
// INPUT   : start	- to identify load initial key or new one key.
// OUTPUT  : none.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
/*
Update_Derivation_Keys(start)
{
index = start;
registerBit = 1 << start;
baseKey = IntermediateDerivationKeyRegister[CurrentDerivationKey];
while (registerBit != 0) {
	derivationData = Create_Derivation_Data(_Derivation-or-Working-Key_,
	_Key-Derivation_,InitialKeyType, InitialKeyID,(registerBit OR TransactionCounter));
	IntermediateDerivationKeyRegister[index] = Derive_Key(baseKey,deriveKeyType, derivationData);
	IntermediateDerivationKeyInUse[index] = true;
	registerBit = registerBit >> 1;
	index = index �V 1;
	}
return SUCCESS;
}
*/
// ---------------------------------------------------------------------------
UCHAR	Update_Derivation_Keys( ULONG start, ULONG deriveKeyType )
{
ULONG	index;
ULONG	registerBit;
UCHAR	result;
UCHAR	baseKey[KEYLENGTH];
UCHAR	derivationData[16];
UCHAR	derivekey[KEYLENGTH];
UCHAR	*p_derivationData;
//UCHAR	*p_derivekey;


	index = start;
	registerBit = 1 << start;
	
    OS_SECM_GetData(ADDR_CUR_DERIVATION_KEY, 4, &g_CurrentDerivationKey);

#ifdef	_USE_SECM_
	// OS_SECM_GetData( ADDR_INT_DERIVATION_KEY_REG+(g_CurrentDerivationKey*KEYLENGTH), Key_Length(deriveKeyType)/8, baseKey );
    OS_SECM_GetIntDerivationKeyReg(g_CurrentDerivationKey * KEYLENGTH, Key_Length(deriveKeyType)/8, baseKey);
#else
	memmove( baseKey, &IntermediateDerivationKeyRegister[g_CurrentDerivationKey][0], Key_Length(deriveKeyType)/8 );
#endif
	
	p_derivationData = derivationData;
	while( registerBit )
	     {
         OS_SECM_GetData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);

	     result = Create_Derivation_Data( _Derivation_or_Working_Key_, _Key_Derivation_, 
	     					g_InitialKeyType, g_InitialKeyID, (registerBit | g_TransactionCounter), p_derivationData );
	     if( !result )
	       return( FALSE );
	       
	     Derive_Key( baseKey, deriveKeyType, p_derivationData, derivekey );

#ifdef	_USE_SECM_
	    //  OS_SECM_PutData( ADDR_INT_DERIVATION_KEY_REG+(index*KEYLENGTH), Key_Length(deriveKeyType)/8, derivekey );
         OS_SECM_PutIntDerivationKeyReg(index * KEYLENGTH, Key_Length(deriveKeyType)/8, derivekey);
#else
	     memmove( &IntermediateDerivationKeyRegister[index][0], derivekey, Key_Length(deriveKeyType)/8 );
#endif

	     IntermediateDerivationKeyInUse[index] = TRUE;
        //  OS_SECM_PutData(ADDR_INT_DERIVATION_KEY_IN_USE + index, 1, &IntermediateDerivationKeyInUse[index]);
         OS_SECM_PutIntDerivationKeyInUse(index, 1, &IntermediateDerivationKeyInUse[index]);

	     registerBit = registerBit >> 1;
	     
	     index--;
	     }
	
	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: The Load_Initial_Key function initializes the system from an initial key, 
//	     the initialKeyID, and a initialKeyType. The initialKeytype must be one of 
//	     _AES128_, _AES192_, or _AES256_ and defines the type of the derivation keys to use 
//	     (aka ��intermediate derivation keys��).
// INPUT   : initialKey
//	     initialKeyType
//	     initialKeyID
// OUTPUT  : none.
// RETURN  : TRUE  - OK.
//           FALSE - the PIN pad is now inoperative, having encrypted more than 1 million PINs.
// ---------------------------------------------------------------------------
/*	(Key Management Command)
Load_Initial_Key(initialKey, initialKeyType, initialKeyID)
{
IntermediateDerivationKeyRegister[0] = initialKey;
IntermediateDerivationKeyInUse[0] = true;
CurrentDerivationKey = 0;
InitialKeyID = initialKeyID;
TransactionCounter = 0;
ShiftRegister = 1;
InitialKeyType = initialKeyType;
Update_Derivation_Keys(NUM_REG-1);
TransactionCounter++;
}
*/
// ---------------------------------------------------------------------------
void	AES_DUKPT_LoadInitialKey( UCHAR *initialKey, ULONG initialKeyType, UCHAR *initialKeyID )
{
#ifdef	_USE_SECM_
	// OS_SECM_PutData( ADDR_INT_DERIVATION_KEY_REG, Key_Length(initialKeyType)/8, initialKey );
    OS_SECM_PutIntDerivationKeyReg(0, Key_Length(initialKeyType)/8, initialKey);
#else
	memmove( &IntermediateDerivationKeyRegister[0][0], initialKey, Key_Length(initialKeyType)/8 );
#endif

	IntermediateDerivationKeyInUse[0] = TRUE;
    // OS_SECM_PutData(ADDR_INT_DERIVATION_KEY_IN_USE, 1, &IntermediateDerivationKeyInUse[0]);
    OS_SECM_PutIntDerivationKeyInUse(0, 1, &IntermediateDerivationKeyInUse[0]);
	
	g_CurrentDerivationKey = 0;
    OS_SECM_PutData(ADDR_CUR_DERIVATION_KEY, 4, &g_CurrentDerivationKey);
//	g_InitialKeyID = initialKeyID;
	memmove( g_InitialKeyID, initialKeyID, 8 );	// 64 bits (8 bytes)
	g_TransactionCounter = 0;
    OS_SECM_PutData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);
	g_ShiftRegister = 1;
	g_InitialKeyType = initialKeyType;
	
	Update_Derivation_Keys( NUM_REG-1, initialKeyType );
	
	g_TransactionCounter++;
    OS_SECM_PutData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);
}

// ---------------------------------------------------------------------------
// FUNCTION: sets to "one" that bit in the ShiftRegister that 
//	     corresponds to the right-most "one" bit in the TransactionCounter, 
//	     making all other bits in the ShiftRegister equal zero.
// INPUT   : 
// OUTPUT  : 
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
/*
Set_Shift_Register()
{
ShiftRegister = 1;
CurrentDerivationKey = 0;
if (TransactionCounter == 0) {
Return SUCCESS;
}
While ((ShiftRegister AND TransactionCounter) == 0) {
ShiftRegister = ShiftRegister << 1;
CurrentDerivationKey++;
}
Return SUCCESS;
}
*/
// ---------------------------------------------------------------------------
UCHAR	Set_Shift_Register( void )
{
	g_ShiftRegister = 1;
	g_CurrentDerivationKey = 0;
    OS_SECM_PutData(ADDR_CUR_DERIVATION_KEY, 4, &g_CurrentDerivationKey);
	
    OS_SECM_GetData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);

	if( g_TransactionCounter == 0 )
	  return( TRUE );
	
	while( (g_ShiftRegister & g_TransactionCounter) == 0 )
	     {
	     g_ShiftRegister = g_ShiftRegister << 1;
	     g_CurrentDerivationKey++;
         OS_SECM_PutData(ADDR_CUR_DERIVATION_KEY, 4, &g_CurrentDerivationKey);
	     }
	     
	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Count_One_Bits()
// INPUT   : 
// OUTPUT  : 
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
/*
Count_One_Bits(value)
{
// For clarity, a simple, straightforward implementation has been chosen.
// There are faster ways of computing the value on most platforms.
bits = 0;
mask = 1 << (NUMREG �V 1);
while (mask > 0) {
if ((value & mask) > 0)
bits = bits + 1;
mask = mask >> 1;
}
return bits;
}
*/
// ---------------------------------------------------------------------------
ULONG	Count_One_Bits( ULONG value )
{
ULONG	bits = 0;
ULONG	mask = 1 << (NUM_REG - 1);


	while( mask )
	     {
	     if( (value & mask) )
	       bits++;
	     mask >>= 1;
	     }
	     
	return( bits );
}

// ---------------------------------------------------------------------------
// FUNCTION: invalidates the current key in the register named IntermediateDerivationKeyRegister
//	     and its in use flag, and increments the TransactionCounter. 
//	     InitialKeyType is the key type for the injected initial key.
// INPUT   : 
// OUTPUT  : 
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
/*
Update_State_for_next_Transaction()
{
oneBits = Count_One_Bits(TransactionCounter);
if (oneBits <= MAX_WORK) {
Update_Derivation_Keys(CurrentDerivationKey);
// erase the current intermediate derivation key
IntermediateDerivationKeyRegister[CurrentDerivationKey] = 0;
// invalidate the current register
IntermediateDerivationKeyInUse[CurrentDerivationKey] = false;
// increment the transaction counter
TransactionCounter++;
}
else { // number of ��one�� bits in the transaction counter is greater than MAX_WORK
// erase the current intermediate derivation key
IntermediateDerivationKeyRegister[CurrentDerivationKey] = 0;
// invalidate the current register
IntermediateDerivationKeyInUse[CurrentDerivationKey] = false;
// skip transaction counter values with more than MAX_WORK bits
TransactionCounter = TransactionCounter + ShiftRegister;
}
// check if transaction counter has exceeded max value
if (TransactionCounter >= ((1 << NUM_REG) �V 1)) {
Cease_Operation(); // this function will cease using this this key set
return ERROR;
}
return SUCCESS;
}
*/
// ---------------------------------------------------------------------------
UCHAR	Update_State_for_next_Transaction( void )
{
ULONG	oneBits;

	
    OS_SECM_GetData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);
	oneBits = Count_One_Bits( g_TransactionCounter );
	
	if( oneBits <= MAX_WORK )
	  {
        OS_SECM_GetData(ADDR_CUR_DERIVATION_KEY, 4, &g_CurrentDerivationKey);
	  if( Update_Derivation_Keys( g_CurrentDerivationKey, g_InitialKeyType ) == FALSE )
	    return( FALSE );

	  // erase the current intermediate derivation key
      OS_SECM_GetData(ADDR_CUR_DERIVATION_KEY, 4, &g_CurrentDerivationKey);
#ifdef	_USE_SECM_
	//   OS_SECM_ClearData( ADDR_INT_DERIVATION_KEY_REG+(g_CurrentDerivationKey*KEYLENGTH), KEYLENGTH, 0x00 );
      OS_SECM_ClearIntDerivationKeyReg(g_CurrentDerivationKey * KEYLENGTH, KEYLENGTH, 0x00);
#else
	  memset( &IntermediateDerivationKeyRegister[g_CurrentDerivationKey][0], 0x00, KEYLENGTH );
#endif
	  // invalidate the current register
      OS_SECM_GetData(ADDR_CUR_DERIVATION_KEY, 4, &g_CurrentDerivationKey);
	  IntermediateDerivationKeyInUse[g_CurrentDerivationKey] = FALSE;
    //   OS_SECM_PutData(ADDR_INT_DERIVATION_KEY_IN_USE + g_CurrentDerivationKey, 1, &IntermediateDerivationKeyInUse[g_CurrentDerivationKey]);
      OS_SECM_PutIntDerivationKeyInUse(g_CurrentDerivationKey, 1, &IntermediateDerivationKeyInUse[g_CurrentDerivationKey]);

	  // increment the transaction counter
	  g_TransactionCounter++;
      OS_SECM_PutData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);
	  }
	else	// number of ��one�� bits in the transaction counter is greater than MAX_WORK
	  {
	  // erase the current intermediate derivation key
      OS_SECM_GetData(ADDR_CUR_DERIVATION_KEY, 4, g_CurrentDerivationKey);
#ifdef	_USE_SECM_
	//   OS_SECM_ClearData( ADDR_INT_DERIVATION_KEY_REG+(g_CurrentDerivationKey*KEYLENGTH), KEYLENGTH, 0x00 );
      OS_SECM_ClearIntDerivationKeyReg(g_CurrentDerivationKey * KEYLENGTH, KEYLENGTH, 0x00);
#else
	  memset( &IntermediateDerivationKeyRegister[g_CurrentDerivationKey][0], 0x00, KEYLENGTH );
#endif
	  
	  // invalidate the current register
      OS_SECM_GetData(ADDR_CUR_DERIVATION_KEY, 4, &g_CurrentDerivationKey);
	  IntermediateDerivationKeyInUse[g_CurrentDerivationKey] = FALSE;
    //   OS_SECM_PutData(ADDR_INT_DERIVATION_KEY_IN_USE + g_CurrentDerivationKey, 1, &IntermediateDerivationKeyInUse[g_CurrentDerivationKey]);
      OS_SECM_PutIntDerivationKeyInUse(g_CurrentDerivationKey, 1, &IntermediateDerivationKeyInUse[g_CurrentDerivationKey]);
	  
	  // skip transaction counter values with more than MAX_WORK bits
	  g_TransactionCounter += g_ShiftRegister;
      OS_SECM_PutData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);
	  }

	// check if transaction counter has exceeded max value
    OS_SECM_GetData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);
	if( g_TransactionCounter >= (ULONG)((1 << NUM_REG) - 1) )	// may be incorrect (to be verified) 
	// if( g_TransactionCounter >= (ULONG)(1 << (NUM_REG-1)) )
	  {
	  // Cease_Operation();
	  return( FALSE );	// this function will cease using this this key set
	  }
	  
	return( TRUE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Generate_Working_Keys function generates a working key for the current transaction
//	     Working keys SHALL only be used with a single algorithm and mode, 
//	     and for a single purpose
// INPUT   : workingKeyUsage
//	     workingKeyType
// OUTPUT  : workingKey
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
/*	(Key Management Command)
Generate_Working_Keys(workingKeyUsage, workingKeyType)
{
// initialize the ShiftRegister and CurrentDerivationKey
Set_Shift_Register();
	// advance transaction counter until current key is valid
while ( !IntermediateDerivationKeyInUse[CurrentDerivationKey] ) {
	TransactionCounter = TransactionCounter + ShiftRegister; // skip over invalid key
	if (TransactionCounter >= ((1 << NUM_REG) �V 1)) {
	Cease_Operation(); // this function will cease using this this key set
	return ERROR;
	}
	Set_Shift_Register();
	}

// derive a working key from the current key pointer
derivationData = Create_Derivation_Data(_Derivation-or-Working-Key_, workingKeyUsage,
workingKeyType, InitialKeyID, TransactionCounter);
workingKey = Derive_Key(IntermediateDerivationKeyRegister[CurrentDerivationKey],
workingKeyType, derivationData);
Update_State_for_next_Transaction();
return workingKey;
}
*/
// ---------------------------------------------------------------------------
UCHAR	AES_DUKPT_GenerateWorkingKeys( ULONG workingKeyUsage, ULONG workingKeyType, UCHAR *workingKey )
{
UCHAR	result;
UCHAR	temp[KEYLENGTH];
UCHAR	derivationData[16];
UCHAR	derivedKey[KEYLENGTH];
UCHAR	*p_derivationData = derivationData;
//UCHAR	*p_derivekey;


	// initialize the ShiftRegister and CurrentDerivationKey
	Set_Shift_Register();

    OS_SECM_GetData(ADDR_CUR_DERIVATION_KEY, 4, &g_CurrentDerivationKey);
    // OS_SECM_GetData(ADDR_INT_DERIVATION_KEY_IN_USE, 32, IntermediateDerivationKeyInUse);
    OS_SECM_GetIntDerivationKeyInUse(0, 32, IntermediateDerivationKeyInUse);

	// advance transaction counter until current key is valid
	while( !IntermediateDerivationKeyInUse[g_CurrentDerivationKey] )
	     {
         OS_SECM_GetData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);
	     g_TransactionCounter = g_TransactionCounter + g_ShiftRegister; // skip over invalid key
         OS_SECM_PutData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);

	     if( g_TransactionCounter >= (ULONG)((1 << NUM_REG) - 1) )	// may be incorrect (to be verified)   
	    //  if( g_TransactionCounter >= (ULONG)(1 << (NUM_REG-1)) )
	       {
	       //Cease_Operation();
	       return( FALSE );	// this function will cease using this this key set
	       }
	     
	     Set_Shift_Register();
	     }
	     
	// derive a working key from the current key pointer
    OS_SECM_GetData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);
	result = Create_Derivation_Data(_Derivation_or_Working_Key_, workingKeyUsage,
					workingKeyType, g_InitialKeyID, g_TransactionCounter, p_derivationData );
	if( !result )
	  goto EXIT;

    OS_SECM_GetData(ADDR_CUR_DERIVATION_KEY, 4, &g_CurrentDerivationKey);

#ifdef	_USE_SECM_
	// OS_SECM_GetData( ADDR_INT_DERIVATION_KEY_REG+(g_CurrentDerivationKey*KEYLENGTH), KEYLENGTH, temp );
    OS_SECM_GetIntDerivationKeyReg(g_CurrentDerivationKey * KEYLENGTH, KEYLENGTH, temp);
	Derive_Key( temp, workingKeyType, p_derivationData, derivedKey );
#else
	Derive_Key( &IntermediateDerivationKeyRegister[g_CurrentDerivationKey][0], workingKeyType, p_derivationData, derivedKey );
#endif	 

	memmove( workingKey, derivedKey, Key_Length(workingKeyType)/8 );
				 
	result = Update_State_for_next_Transaction();
	
EXIT:
	memset( temp, 0x00, sizeof(temp) );
	memset( derivationData, 0x00, sizeof(derivationData) );
	
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: The wrapping key for the Update_Initial_Key function 
//	     is the key that corresponds to the counter value 0xFFFFFFFF. 
//	     That counter value SHALL NOT be used to calculate transaction keys.
// INPUT   : 
// OUTPUT  : 
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
/*
Calculate_DUKPT_Update_Key()
{
bit = 1 << (NUM_REG - 1);
register = NUM_REG �V 1;
while ((TransactionCounter & bit) == 1)
{
	bit = bit >> 1;
	register--;
}

ctr = TransactionCounter & ~(bit �V 1);
key = IntermediateDerivationKeyRegister[register];
while (bit > 0)
{
	ctr = ctr | bit;
	derivationData = Create_Derivation_data(_Derivation-or-Working-Key_, _Key-Derivation_, InitialKeyType, InitialKeyID, ctr);
	key = Derive_Key(key, deriveKeyType, derivationData);
	register--;
	bit = bit >> 1;
}

derivationData = Create_Derivation_Data(_Derivation-or-Working-Key_, _Key-Encryption-Key_, InitialKeyType, InitialKeyID, ctr);
return Derive_Key(key, InitialKeyType, derivationData);
}
*/
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// FUNCTION: Update_Initial_Key function takes a new encrypted initial key wrapped with the last 
//	     possible key encryption key derived from the current initial key on the device, 
//	     decrypts the new initial key with the key encryption key, 
//	     and installs the new encryption key. After a new initial key is loaded, 
//	     the transaction counter SHALL be reset to zero.
//
//	     The new initial key SHALL be protected in a way that complies with the requirements in this standard.
//	     UnwrapKey SHALL be implemented in a way that is compliant with the requirements in section 7.4 (for example by using a TR-31 key block).
//	     The wrapping key SHALL only be used with a single key wrap algorithm and mode.
//	     For interoperability, TR-31 SHOULD be used for interchange, as soon as support for wrapping AES keys has been added.
//	     If TR-31 key blocks are used, the TR-31 key block carrying the initial key SHOULD have the following format:
//		D nnnn B1 A X 00 N 01 00 IK 14 hhhhhhhhhhhhhhhh
//			where nnnn is the decimal key block length (including count and header) 
//			and hhhhhhhhhhhhhhhh is the key ID.
//
// INPUT   : workingKeyUsage
//	     workingKeyType
// OUTPUT  : workingKey
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
/*	(Optional Key Management Command)
Update_Initial_Key(encryptedInitialKey, newInitialKeyType, newInitialKeyID)
{
// derive a key encryption key from the last counter value
// for the device
dukptUpdateKey = Calculate_DUKPT_Update_Key();
if (!UnwrapKey(encryptedInitialKey, dukptUpdateKey, newInitialKey))
   {
   return ERROR;
   }
Load_Initial_Key(newInitialKey, newInitialKeyType, newInitialKeyID);
return SUCCESS;
}
*/
// ---------------------------------------------------------------------------
/*
UCHAR	AES_DUKPT_UpdateInitialKey( UCHAR *encryptedInitialKey, ULONG newInitialKeyType, UCHAR *newInitialKeyID )
{

	// derive a key encryption key from the last counter value
	// for the device

	if( g_TransactionCounter >= (ULONG)(1 << (NUM_REG-1)) )
	  {
	  //Cease_Operation();
	  return( FALSE );	// this function will cease using this this key set
	  }
}
*/

// ---------------------------------------------------------------------------
// FUNCTION: Generate EPB by using TDEA.
// INPUT   : Key_Reg     - key register for working key.
// OUTPUT  : Crypto_Reg1 - the crypto register 1. (8 bytes, the EPB)
// RETURN  : none.
// ---------------------------------------------------------------------------
void 	AES_DUKPT_GenEpbByTDEA( UCHAR keyType, UCHAR *Key_Reg, UCHAR *pinblock, UCHAR *epb )
{
UCHAR	deskey[24];
UCHAR	data[8];

	
	if( keyType == _2TDEA_ )
	  memmove( deskey, Key_Reg, 16 );
	else
	  memmove( deskey, Key_Reg, 24 );
	  
	memmove( data, pinblock, 8 );
	
	// Crypto Register-1 DEA-encrypted using the left half of the Key Register
	// as the key goes to Crypto Register-1
	
	api_des_encipher( data, data, &deskey[0] );
	
	// Crypto Register-1 DEA-decrypted using the right half of the Key Register
	// as the key goes to Crypto Register-1
	
	api_des_decipher( data, data, &deskey[8] );
	
	// Crypto Register-1 DEA-encrypted using the left half of the Key Register
	// as the key goes to Crypto Register-1
	if( keyType == _2TDEA_ )
	  api_des_encipher( data, data, &deskey[0] );
	else
	  api_des_encipher( data, data, &deskey[16] );
	  
	memmove( epb, data, 8 );
}

// ---------------------------------------------------------------------------
// FUNCTION: Generate EPB by using AES. (ONLY for ISO 9564 Format 4)
// INPUT   : Key_Reg     - key register for working key.
//	     pan         - PAN. (format: L-V)
// OUTPUT  : epb	 - encrypted PIN block (16/24/32 bytes)
// RETURN  : TRUE
//	     FALSE
// ---------------------------------------------------------------------------
UCHAR 	AES_DUKPT_GenEpbByAES( UCHAR keyType, UCHAR *pan, UCHAR *Key_Reg, UCHAR *pinblock, UCHAR *epb )
{
UCHAR	result = FALSE;
UCHAR	aeskey[32];
UCHAR	keyLen;
UINT8	pin[PED_PIN_SLOT_LEN];

	
	keyLen = Key_Length( keyType )/8;
	memmove( aeskey, Key_Reg, keyLen );
	
	// restore PIN data
	// OS_SECM_GetData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, pin ); // L-V
    memcpy(pin, ped_pin_ptr, PED_PIN_SLOT_LEN); // L-V
	if(pin[0] != 0)
	  {
	  // generate EPB (Encrypted PIN Block) by using ISO4_KEY
      PED_GenEncPinBlock_ISO4( pin, pan, aeskey, epb );

      if(ped_pin_ptr != NULL)
        PED_ClearPin();  // deallocate PIN data memory
	  
	  result = TRUE;
	  }
	
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Request PIN entry for DUKPT. (PED function)
// INPUT   : pinblock - the plaintext pinblock. (fixed 8 bytes)
//	     keyType  - key type for the derived working key.
//			_2TDEA_	(0)
//			_3TDEA_	(1)
//			_AES128_(2)
//			_AES192_(3)
//			_AES256_(4)
//	     pan    - PAN. (format: L-V)
// OUTPUT  : ksn      - fixed 12 bytes (96 bits)
//			KSN[96b] = IKID[64b] + Transaction Counter[32b]
//			IKID[64b] = BDK ID[32b] + derivation ID[32b]
//           epb      - encryption PIN block. (8, 16, 24, 32 bytes)
//
// RETURN  : TRUE  - OK.
//           FALSE - the PIN pad is now inoperative, having encrypted more than
//		     transaction limit. (1 billion times)
// ---------------------------------------------------------------------------
UCHAR	AES_DUKPT_RequestPinEntry( UCHAR keyType, UCHAR *pan, UCHAR *pinblock, UCHAR *ksn, UCHAR *epb )
{
UCHAR	result = TRUE;
UCHAR	workingKey[32];


	result = AES_DUKPT_GenerateWorkingKeys( _PIN_Encryption_, keyType, workingKey );
	if( result )
	  {
	  switch( keyType )
	  	{
	  	case _2TDEA_:
	  	case _3TDEA_:

	  	     AES_DUKPT_GenEpbByTDEA( keyType, workingKey, pinblock, epb );
	  	     break;
	  	     
	  	case _AES128_:
	  	case _AES192_:
	  	case _AES256_:
	  	     
	  	     AES_DUKPT_GenEpbByAES( keyType, pan, workingKey, pinblock, epb );
	  	     break;
	  		
	  	default:
	  	     result = FALSE;
	  	     break;
	  	}
	  }
	else
	  result = FALSE;
	  
	if( result )
	  {
	  // KSN[96b] = IKID[64b] + Transaction Counter[32b]
	  memmove( ksn, g_InitialKeyID, 8 );
	  
      OS_SECM_GetData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);

	  ksn[8]  = (g_TransactionCounter & 0xff000000) >> 24;
	  ksn[9]  = (g_TransactionCounter & 0x00ff0000) >> 16;
	  ksn[10] = (g_TransactionCounter & 0x0000ff00) >> 8;
	  ksn[11] = (g_TransactionCounter & 0x000000ff);
	  }
	
	memset( workingKey, 0x00, sizeof(workingKey) );
	
	return( result );
}

// ---------------------------------------------------------------------------
UCHAR	AES_DUKPT_RequestMacKey( UCHAR keyType, UCHAR *workingKey )
{
	return( AES_DUKPT_GenerateWorkingKeys( _Message_Authentication_generation_, keyType, workingKey ) );
}

// ---------------------------------------------------------------------------
//ULONG	AES_DUKPT_GetCounter( void )
//{
//	return( g_TransactionCounter );
//}

// ---------------------------------------------------------------------------
//UCHAR	AES_DUKPT_GetDerivationKey( UCHAR *key )
//{
//UCHAR	keylen = 16;	// key length may be 16, 24, 32
//
//	memmove( key, &IntermediateDerivationKeyRegister[g_CurrentDerivationKey][0], keylen );
//	
//	return( keylen );
//}

// ---------------------------------------------------------------------------

/*
"Derive Initial Key"
The Derive_Initial_Key function sets the initial key derivation data up 
according to Table 2 using the Initial Key ID and the appropriate constants 
for the AES size being used.

Derive_Initial_Key(BDK, keyType, initialKeyID)
{
derivationData = Create_Derivation_Data(_Initial-Key_,
_Key-Derivation-Initial-Key_,
keyType, initialKeyID, 0);
initialKey = Derive_Key(BDK, keyType, derivationData);
return initialKey;
}
*/
// ---------------------------------------------------------------------------
// FUNCTION: Derive IPEK from BDK, KEY_TYPE & IKID(64) by the method as described in the spec "6.4".
//
// INPUT   : bdk      - base derivation key (BDK, fixed 16 bytes of TDES key)
//	     keyType  - _3TDEA_,...
//	     ikid     - initial key ID
// OUTPUT  : ipek     - derived initial PIN entry key (IPEK, fixed 16 bytes of TDES key)
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
UCHAR	AES_DUKPT_DeriveIPEK( UCHAR *bdk, ULONG keyType, UCHAR *ikid, UCHAR *ipek )
{
UCHAR	result;
UCHAR	derivationData[16];


	result = Create_Derivation_Data( _Initial_Key_, _Key_Derivation_Initial_Key_, keyType, ikid, 0, derivationData );
	if( result )
	  Derive_Key( bdk, keyType, derivationData, ipek );
	   
	return( result );
}

// ---------------------------------------------------------------------------
// FUNCTION: Get current KSN
// INPUT   : none.
// OUTPUT  : ksn - fixed 12 bytes (96 bits)
//			 KSN[96b] = IKID[64b] + Transaction Counter[32b]
//			 IKID[64b] = BDK ID[32b] + derivation ID[32b]
// RETURN  : none.
// ---------------------------------------------------------------------------
void    AES_DUKPT_GetKSN(UCHAR *ksn)
{
    memmove( ksn, g_InitialKeyID, 8 );
	
    OS_SECM_GetData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);

	ksn[8]  = (g_TransactionCounter & 0xff000000) >> 24;
	ksn[9]  = (g_TransactionCounter & 0x00ff0000) >> 16;
	ksn[10] = (g_TransactionCounter & 0x0000ff00) >> 8;
	ksn[11] = (g_TransactionCounter & 0x000000ff);
}

// ---------------------------------------------------------------------------
// FUNCTION: Get data encryption key
// INPUT   : keyType    - key type.
//			              0: _2TDEA_
//			              1: _3TDEA_
//			              2: _AES128_
//			              3: _AES192_
//			              4: _AES256_
// OUTPUT  : workingKey - derived working key
// RETURN  : TRUE.
//           FALSE.
// ---------------------------------------------------------------------------
UCHAR	AES_DUKPT_RequestDataEncryptionKey(UCHAR keyType, UCHAR *workingKey)
{
	return(AES_DUKPT_GenerateWorkingKeys(_Data_Encryption_both_ways_, keyType, workingKey));
}

