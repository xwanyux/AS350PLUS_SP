#include <string.h>
#include "Glv_ReaderConfPara.h"
#include "Define.h"
#include "Function.h"
#include "ECL_Tag.h"
#include "VGL_Kernel_Define.h"
#include "VGL_Function.h"
#include "Datastructure.h"
#include "VAP_ReaderInterface_Define.h"

// Add by Wayne 2020/08/21 to avoid compiler warning
#include "UTILS_CTLS.H"

//api_pcd_vap_Command.c
extern UCHAR api_W3_DDAFail[3];
extern UCHAR L3_Response_Code;

//ECL_CAPK.c
extern CAPK glv_CAPK[CAPK_NUMBER];

//VGL_Kernel.c
extern UCHAR VGL_Online_Required_by_Reader;
extern UCHAR VGL_Decline_Required_by_Reader;

////////////////
// ---------------------------------------------------------------------------
// FUNCTION: check CRL (Certificate Revocation List).
// INPUT   : rid   - RID [5]
//           capki - CAPK index[1]
//           csn   - certificate serial number [3] 
// OUTPUT  : none.
// RETURN  : TRUE  - matched
//           FALSE - not matched
// ---------------------------------------------------------------------------
UCHAR	VGL_OFDA_CheckCRL( UCHAR *rid, UCHAR capki, UCHAR *csn )
{
	UINT	i;
	UCHAR	result = FALSE;
	UCHAR	list[VGL_CAPK_REVOC_LEN];

	list[0] = rid[0];
	list[1] = rid[1];
	list[2] = rid[2];
	list[3] = rid[3];
	list[4] = rid[4];
	list[5] = capki;
	list[6] = csn[0];
	list[7] = csn[1];
	list[8] = csn[2];
	   
	/*for( i=0; i<MAX_CAPK_REVOC_CNT; i++ )
	{	
		if( UT_memcmp( glv_CRL[i].RID, &list[0], 5 ) == 0 )
	    {
	    	if( UT_memcmp( &glv_CRL[i].Index, &list[5], 1 ) == 0 )
	    	{
	    		if( UT_memcmp( glv_CRL[i].serNumber, &list[6], 3 ) == 0 )
	    		{
			    	result = TRUE;	// matched
			    	break;
	    		}
	    	}
		}
	}*/
	for( i=0; i<VGL_MAX_CAPK_REVOC_CNT; i++ )
	{	
		if( UT_memcmp(&list[0],&Revo_List[i][0], 9 ) == 0 )
	    {
	    	result = TRUE;	// matched
	    	break;
   		}
	}	
	   
	return result;
}

//Req 5.6.1.1 "Offline capable readers"
UCHAR VGL_OFDA_FDDA(void)
{
	UINT i=0,j=0,iTagListLen=0;
	UINT Re_len=0,iPKcLen=0,iModLen=0,iLeftMostLen=0,iHashLen=0,Tmp_Len1=0,Tag70_Tmp_Len=0;
	UCHAR capki=0;
	UCHAR rid[VGL_Rid_Len]={0};
	UCHAR Index_not_found = 0;
	UCHAR CAPKI[1]={0};		
	UCHAR ISSPK[250] = {0};
	//UCHAR pkm[250]={0};
	UCHAR pkm[1024]={0};
	UCHAR ICCPK[250] = {0};	//Retrieval of the ICC Public Key					tag9f46
	UCHAR pkc[250] = {0};	//Retrieval of the ICC Public Key
	UCHAR VGL_tag70_Data[270]={0};
	UCHAR temp[20]={0};
	UCHAR Tag_Len=0,*ptrlist=0;
	UCHAR rspCode=0;
	ULONG Index={0};
	UCHAR g_ibuf[1500];
	UCHAR g_obuf[1500];

	memset(g_ibuf,0x00,sizeof(g_ibuf));
//	UCHAR command123[1024]={0};
//	UCHAR *Static_Data=0;
//ICTK changed
//In any if the following cases, fdda shall fail

	//AIP byte 1 bits6 is 0b?	DDA support? 1 for yes , o for none
	rspCode = UT_Get_TLVLengthOfV(glv_tag82.Length,&Tmp_Len1);
	if(Tmp_Len1 != 0)
	{
		if(!(glv_tag82.Value[0] & 0x20))
			return VGL_OFDA_Fail;
	}

	//The version of fDDA requested by the card is not "01"
	rspCode = UT_Get_TLVLengthOfV(glv_tag9F69.Length,&Tmp_Len1);
	if(Tmp_Len1 != 0)
	{
		if(glv_tag9F69.Value[0] != 0x01)
			return VGL_OFDA_Fail;
	}
	
//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_A");

//EMV 4.3 Book 2 Section 6.2-Retrieve the Certification Authority Public Key Index		tag8f
	memcpy(CAPKI,glv_tag8F.Value,1);
	
	for(i=0;i<CAPK_NUMBER;i++)
	{
//		UT_DumpHex(1,0,10,(UCHAR *)&glv_CAPK[i]);
		//if((glv_CAPK[i].Index == CAPKI[0]) && (!memcmp(glv_CAPK[i].RID,glv_tag4F.Value,5)))	//20131210 change tag4F to tag9F06
		if((glv_CAPK[i].Index == CAPKI[0]) && (!memcmp(glv_CAPK[i].RID,glv_tag9F06.Value,5)))
		{
			Index_not_found = 0;
			memcpy(&pkm[2],glv_CAPK[i].Modulus,(UINT)glv_CAPK[i].Length);
//			UT_ClearScreen();
//			UT_DumpHex(0,0,(UINT)glv_CAPK[i].Length,pkm);
			break;
		}
		else
			Index_not_found = 1;
	}
	
	if(Index_not_found)
		return VGL_OFDA_Fail ;
	
//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_B");

//EMV 4.3 Book 2 Section 6.3 -Retrieval of the Issuer Public Key						tag90
	//Issuer Public Key Certificate length check 
	
	rspCode = UT_Get_TLVLengthOfV(glv_tag90.Length, &Tmp_Len1);

//	modulus with 248 bytes, 2L = 0xF8, 0x00

	pkm[0]=Tmp_Len1 & 0x00FF;
	pkm[1]=(Tmp_Len1 & 0xFF00) >> 8;
				
	if(glv_CAPK[i].Length == Tmp_Len1)
	{
		ISSPK[0]=Tmp_Len1 & 0x00FF;
		ISSPK[1]=(Tmp_Len1 & 0xFF00) >> 8;
//		UT_DumpHex(0,0,(UINT)Tmp_Len1,glv_tag90.Value);
		memcpy(&ISSPK[2],glv_tag90.Value,Tmp_Len1);
	}
	else
		return VGL_OFDA_Fail ;
	
	//recover function on the Issuer Public Key Certificate using the Certification Authority Public Key
	
	//Load external public key
    if( api_rsa_loadkey( pkm, glv_CAPK[i].Exponent) != apiOK )
		return VGL_OFDA_Fail ;
	
	iPKcLen = Tmp_Len1;
	iLeftMostLen = Tmp_Len1 - 36;

//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_C");
	
	//recover	
	if( api_rsa_recover( ISSPK, ISSPK ) != apiOK )
		return VGL_OFDA_Fail ;
		//UT_ClearScreen();
	//UT_DumpHex(0,0,250,ISSPK);

	iModLen = ISSPK[2+14-1];
		
	//vertify the recover data, trailer 0xBC

	if(ISSPK[Tmp_Len1+2-1] != 0xBC)
		return VGL_OFDA_Fail ;

	//vertify the recover data, Recovered Data Header 0x6A

	if(ISSPK[2] != 0x6A)
		return VGL_OFDA_Fail ;

	// vertify the recover data, check certificate format 0x02
    if(ISSPK[2+1] != 0x02)
     	return VGL_OFDA_Fail ;

//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_D");


	//vertify the recover data, filed2-10, Issuer Public Key Remainder, Issuer Public Key Exponent (Step5,6,7)
	Re_len = (Tmp_Len1 - 22);
	for(i=0;i<Re_len;i++)
		g_ibuf[i] = ISSPK[i+3];	// filed2-10

	rspCode = UT_Get_TLVLengthOfV(glv_tag92.Length,&Tmp_Len1);

	if(Tmp_Len1 != 0)	//Issuer Public Key Remainder
	{
		memcpy(&g_ibuf[i],glv_tag92.Value,Tmp_Len1);	
		i+= Tmp_Len1;
		Re_len+= Tmp_Len1;
	}
	else
	{
		// Ni <= Nca - 36 ?
		if( iModLen > iLeftMostLen )
			return VGL_OFDA_Fail;
	}

//	UT_ClearScreen();
//UT_PutStr(0,0, FONT0, 1, (UCHAR *)"FDDA_E");
//	UT_WaitKey();

	rspCode = UT_Get_TLVLengthOfV(glv_tag9F32.Length,&Tmp_Len1);
	
	if(Tmp_Len1 != 0)	//Issuer Public Key exponent
	{
		memcpy(&g_ibuf[i],glv_tag9F32.Value,Tmp_Len1);	
		i+=Tmp_Len1;
		Re_len+=Tmp_Len1;
	}
	else
		return VGL_OFDA_Fail;

	if( api_sys_SHA1( Re_len, g_ibuf, temp) != apiOK )
		return VGL_OFDA_Fail;

	for( i=0; i<20; i++ )
    {
        if( temp[i] != ISSPK[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
        	return VGL_OFDA_Fail;
    }

	// Verification 8: check Issuer ID Number, Leftmost 3-8 digits from the PAN (padded to the right with Hex 'F's)
//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_F");

	rspCode = UT_Get_TLVLengthOfV(glv_tag5A.Length,&Tmp_Len1);
	
	if(Tmp_Len1 != 0)
		memcpy(temp,glv_tag5A.Value,Tmp_Len1);// load application PAN

	if( UT_CNcmp( &ISSPK[2+2], temp, 4 ) == FALSE )
        return VGL_OFDA_Fail;

	// Verification 9: check the certificate expiration date MMYY
	if( UT_VerifyCertificateExpDate( &ISSPK[2+6] ) == FALSE )	
		return VGL_OFDA_Fail;

	// Verification 10: RID[5] + INDEX[1] + Certificate Serial Number[3] 
	memcpy(&capki,glv_tag8F.Value,1);
	//memcpy(rid,glv_tag4F.Value,5);	//20131210 change tag4F to tag9F06
	memcpy(rid,glv_tag9F06.Value,5);

	if(Opt_qVSDC_Key_Revocation)
	{
	   	if( VGL_OFDA_CheckCRL( rid, capki, &ISSPK[2+8] ) )
	   	{
	//		UT_DumpHex(0,0,5,rid);
			return VGL_OFDA_Fail;
		}
	}
//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_G");

		
	// Verification 11: check the issuer public key algorithm indicator 0x01
      if( ISSPK[2+12] != 0x01 )
        return VGL_OFDA_Fail;

	// Verification 12: concatenate the Leftmost Digits of the Issuer Public Key and the Issuer Public Key Remainder
	// Issuer Public Key Modulus (stored in pkm[iModLen] array) = 2L-V
    //        (1) Leftmost Digits of the Issuer Public Key +
    //        (2) Issuer Public Key Remainder (if present)			tag92
	for(i=0;i<iLeftMostLen;i++)
		pkm[i+2] = ISSPK[i+2+15];

	//UT_DumpHex(0,0,iLeftMostLen,&ISSPK[17]);
	rspCode = UT_Get_TLVLengthOfV(glv_tag92.Length,&Tmp_Len1);
	if(Tmp_Len1 != 0)
		memcpy(&pkm[i+2],glv_tag92.Value,Tmp_Len1);


	rspCode = UT_Get_TLVLengthOfV(glv_tag9F46.Length,&Tmp_Len1);

	//6.4.1 ICC Public Key Certificate has a length different from the length of the Issuer Public Key Modulus	
	if(iModLen == Tmp_Len1)		
	{
		pkm[0]=Tmp_Len1 & 0x00FF;
		pkm[1]=(Tmp_Len1 & 0xFF00)>>8;
		//UT_DumpHex(0,0,Tmp_Len1+2,pkm);
	}	
	else
		return VGL_OFDA_Fail;
	
//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_H");

	//6.4.2 Obtain the recovered data
	iPKcLen = Tmp_Len1;
	iLeftMostLen = Tmp_Len1 - 42;

	/////////////////////////////////////////////////////////////////////////////////////////////
	/*command123[0] = 0xFE;
	command123[1] = 0x00;
	command123[2] = 0xD1;
	command123[3] = 0xDF;
	command123[4] = 0x0F;
	command123[5] = 0xF9;
	command123[6] = 0x00;
	command123[7] = 0x00;
			
	memcpy(&command123[8],pkm,249);
	UT_TransmitAUX(command123);*/
	////////////////////////////////////////////////////////////////////////////////////////////
	//UT_DispHexWord(0,0,Tmp_Len1);
	//UT_DumpHex(0,1,Tmp_Len1+2,pkm);
		
	//Load external public key
	if( api_rsa_loadkey( pkm, glv_tag9F32.Value) != apiOK )
		return VGL_OFDA_Fail ;

	ICCPK[0]=Tmp_Len1 & 0x00FF;
	ICCPK[1]=(Tmp_Len1 & 0xFF00)>>8;
	memcpy(&ICCPK[2],glv_tag9F46.Value,Tmp_Len1);
	
	//recover	
	if( api_rsa_recover( ICCPK, ICCPK ) != apiOK )
		return VGL_OFDA_Fail ;

	//20130625 for ram_gcc_nos_xx6.ld

	for(i=2;i<ICCPK[0];i++)
	{
		if(ICCPK[i] != 0x00)
		{
			memmove(&ICCPK[2],&ICCPK[i],ICCPK[0]);
			break;
		}
	}
	
	//if(ICCPK[0] == 0xF7)
	//{
	//	memmove(&ICCPK[2],&ICCPK[3],0xF7);
	//}

	iModLen = ICCPK[2+20-1];

	//UT_DumpHex(0,1,250,ICCPK);
	

	// Verification 2: check recovered data trailer
    if( ICCPK[ (iPKcLen+2)-1 ] != 0xBC )
    	return VGL_OFDA_Fail ;

	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"A");
	//UT_WaitKey();	

    // Verification 3: check recovered data header
    if( ICCPK[2+0] != 0x6A )
        return VGL_OFDA_Fail ;

    // Verification 4: check certificate format
    if( ICCPK[2+1] != 0x04 )
      	return VGL_OFDA_Fail ;

//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_I");
	
	// Verification 5: Concatenation
	   // (1) Recovered Data[2'nd..10'th] +
	   // (2) ICC Remainder +
	   // (3) ICC Exponent +
	   // (4) Static data to be authenticated
	Re_len = iPKcLen - 22; // header[1], HashResult[20], Trailer[1]
	
	for( i=0; i<Re_len; i++ )
	  	g_ibuf[i] = ICCPK[i+3]; // from "Certificate Format" to "ICC Public Key"

	rspCode = UT_Get_TLVLengthOfV(glv_tag9F48.Length,&Tmp_Len1);

	if(Tmp_Len1 != 0)	//ICC Public Key Remainder
	{
		memcpy(&g_ibuf[i],glv_tag9F48.Value,Tmp_Len1);	
		i+=Tmp_Len1;
		Re_len+=Tmp_Len1;
	}
	else
	{
		// Nic <= Ni - 42 ?
		if( iModLen > iLeftMostLen )
			return VGL_OFDA_Fail;
	}

	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"B");
	//UT_WaitKey();
	
	rspCode = UT_Get_TLVLengthOfV(glv_tag9F47.Length,&Tmp_Len1);
	
	if(Tmp_Len1 != 0) //ICC public key Exponent
	{
		memcpy(&g_ibuf[i],glv_tag9F47.Value,Tmp_Len1);	//ICC Public Key  exponent
		i+=Tmp_Len1;
		Re_len+=Tmp_Len1;
	}
	else
		return VGL_OFDA_Fail;
	
//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_J");

	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"C");
	//UT_WaitKey();
	
/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Get STATIC DATA
	// The fields within [] are included.
	// (1) SFI=1..10,  70 LL [Data] SW1 SW2
      	// (2) SFI=11..30, [70 LL Data] SW1 SW2
      	// (3) If the record is a non Tag-70, ODA has failed.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


	for(j=0;j<MAX_VGL_REC_CNT;j++)
	{
		memcpy(VGL_tag70_Data,ADDR_VGL_REC_01+j*VGL_REC_LEN,VGL_REC_LEN);

		//UT_DumpHex(0,0,270,VGL_tag70_Data);
		
		if(VGL_tag70_Data[0]==0x00)
			break;
		else
        {
	    	if( (VGL_tag70_Data[0] & 0x1f) < 11 )	// SFI=1..10, only store data
	        {
	        	//get length and length's length
	        	if((VGL_tag70_Data[2] & 0x80) == 0x00)	//length = VGL_tag70_Data[2],	length's length = 1
	        	{	        		
	        		Tag70_Tmp_Len = VGL_tag70_Data[2];
					
	    			memcpy(&g_ibuf[i],&VGL_tag70_Data[1+1+1],Tag70_Tmp_Len);	
					i+=Tag70_Tmp_Len;
					Re_len+=Tag70_Tmp_Len;
	        	}
				else									//lenght = VGL_tag70_Data[3][4], length's length = 2 or 3
				{
					switch( VGL_tag70_Data[2] & 0x7f )
                    {
                     	case	0x00: // 1-byte length (128..255)
	                         	Tag70_Tmp_Len = VGL_tag70_Data[2];

	                         	memcpy(&g_ibuf[i],&VGL_tag70_Data[1+1+1],Tag70_Tmp_Len);	
							 	i+=Tag70_Tmp_Len;
							 	Re_len+=Tag70_Tmp_Len;
	                         	break;
								
	                    case	0x01: // 1-byte length (128..255)
	                         	Tag70_Tmp_Len = VGL_tag70_Data[3];

	                         	memcpy(&g_ibuf[i],&VGL_tag70_Data[1+1+2],Tag70_Tmp_Len);	
							 	i+=Tag70_Tmp_Len;
							 	Re_len+=Tag70_Tmp_Len;
	                         	break;

	                    case 	0x02: // 2-byte length (256..65535)
	                         	Tag70_Tmp_Len = VGL_tag70_Data[3]*256 + VGL_tag70_Data[4];

								memcpy(&g_ibuf[i],&VGL_tag70_Data[1+1+3],Tag70_Tmp_Len);	
							 	i+=Tag70_Tmp_Len;
							 	Re_len+=Tag70_Tmp_Len;
	                         	break;

	                    default:   // out of spec
	                        	return VGL_OFDA_Fail;
                    }
				}
	        }
	        else	// SFI=11..30, store tag length data
	        {
	        	if( (VGL_tag70_Data[0] & 0x1f) < 31 )
	            {
	            	if((VGL_tag70_Data[2] & 0x80) == 0x00)	//length = VGL_tag70_Data[2],	length's length = 1
		        	{	        		
		        		Tag70_Tmp_Len = VGL_tag70_Data[2];
						
		    			memcpy(&g_ibuf[i],&VGL_tag70_Data[1],Tag70_Tmp_Len+1+1);	
						i+=Tag70_Tmp_Len+1+1;
						Re_len+=Tag70_Tmp_Len+1+1;
		        	}
					else									//lenght = VGL_tag70_Data[3][4], length's length = 2 or 3
					{
						switch( VGL_tag70_Data[2] & 0x7f )
	                    {
							case	0x00: // 1-byte length (128..255)
	                         		Tag70_Tmp_Len = VGL_tag70_Data[2];

		                         	memcpy(&g_ibuf[i],&VGL_tag70_Data[1],Tag70_Tmp_Len+1+1);	
									i+=Tag70_Tmp_Len+1+1;
									Re_len+=Tag70_Tmp_Len+1+1;
		                         	break;
						
		                    case 	0x01: // 1-byte length (128..255)
		                        	Tag70_Tmp_Len = VGL_tag70_Data[3];

		                         	memcpy(&g_ibuf[i],&VGL_tag70_Data[1],Tag70_Tmp_Len+1+2);		//tag length 1, length length 2
								 	i+=Tag70_Tmp_Len+1+2;
								 	Re_len+=Tag70_Tmp_Len+1+2;
		                         	break;

		                    case 	0x02: // 2-byte length (256..65535)
		                         	Tag70_Tmp_Len = VGL_tag70_Data[3]*256 + VGL_tag70_Data[4];

		                         	memcpy(&g_ibuf[i],&VGL_tag70_Data[1],(UINT)Tag70_Tmp_Len+1+3);		//tag length 1, length length 3
									i+=Tag70_Tmp_Len+1+3;
								 	Re_len+=Tag70_Tmp_Len+1+3;
		                         	break;

		                    default:   // out of spec
		                         	return VGL_OFDA_Fail;
	                    }
					}        
	         	}
				else 
					return VGL_OFDA_Fail;
			}
   		}
	}
	
//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_K");

//20130412

	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"D");
	//UT_WaitKey();

	// tag 9F4A list is present "Static Data Authentication Tag List"
	rspCode = UT_Get_TLVLengthOfV(glv_tag9F4A.Length,&Tmp_Len1);

	if(Tmp_Len1 != 0)
	{
		//get value's pointer
		ptrlist = glv_tag9F4A.Value;

		if(*ptrlist != 0x82)
			return VGL_OFDA_Fail;
/*		//get length
		if((glv_tag9F4A.Length[0] & 0x80) == 0x00)
			iTagListLen = glv_tag9F4A.Length[0];
		else
		{
			switch( glv_tag9F4A.Length[0] & 0x7f )
			{
				case	0x00:
				iTagListLen = glv_tag9F4A.Length[0];
				break;
					
				case 	0x01: // 1-byte length (128..255)
				iTagListLen = glv_tag9F4A.Length[1];
	           		 break;

		            	case 	0x02: // 2-byte length (256..65535)
		            	iTagListLen = glv_tag9F4A.Length[1]*256 + glv_tag9F4A.Length[2];
		            	break;

				default:   // out of spec
			       return VGL_OFDA_Fail;
	        	}
		}
*/
		iTagListLen = Tmp_Len1;

		// concatenated to the current end of the input string
        do
		{
	   	    // check constructed DO
//        	if( apk_EMVCL_CheckConsTag( *ptrlist ) == TRUE )
			if (UT_Check_ConstructedTag(ptrlist) == TRUE)
            	return VGL_OFDA_Fail;

          	// check word tag
//         	if( apk_EMVCL_CheckWordTag( *ptrlist ) == TRUE )
			if (UT_Check_WordTag(ptrlist) == TRUE)
          	{
	            Tag_Len = 2;		//Word tag
				UT_Search(Tag_Len,ptrlist,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);
				ptrlist+=2;
				iTagListLen-=2;
          	}
          	else
          	{
	            Tag_Len = 1;		//single tag
				UT_Search(Tag_Len,ptrlist,(UCHAR *)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE,&Index);
				ptrlist++;
				iTagListLen--;
	        }

			//check length
			if(*glv_addTable[Index] & 0x80)
			{
				if(*glv_addTable[Index] & 0x01)
				{
					Tmp_Len1 = *(glv_addTable[Index]+1);
				}
				else if(*glv_addTable[Index] & 0x02) 
				{
					Tmp_Len1 = ((*(glv_addTable[Index]+1))*256)+(*(glv_addTable[Index]+2));
				}
				else
					Tmp_Len1 = *glv_addTable[Index];
			}
			else
			{
				Tmp_Len1 = *glv_addTable[Index];
			}

			// concatenated to the current end of the input string
          	if( Tmp_Len1 != 0) 
            {
	          	memcpy(&g_ibuf[i],glv_addTable[Index]+3,Tmp_Len1);
				i+=Tmp_Len1;
				Re_len+=Tmp_Len1;
            }
	        else
    	        return VGL_OFDA_Fail;
		} while( iTagListLen > 0 ); // next tag  
		
//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_L");

	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"E");
	//UT_WaitKey();
	
	}
	
	//UT_DumpHex(0,0,Re_len,g_ibuf);

	// Verification 6: calculate & compare SHA1 result
	if( api_sys_SHA1(Re_len, g_ibuf, temp) != apiOK )
		return VGL_OFDA_Fail;

	//UT_DumpHex(0,0,20,&ICCPK[iPKcLen-19]);
	//UT_DumpHex(0,3,20,temp);

	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"F");
	//UT_WaitKey();

	//Verification 7

	for( i=0; i<20; i++ )
	{
		if( temp[i] != ICCPK[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
			return VGL_OFDA_Fail;
	}
	
//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_M");

	
	//UT_PutStr(0,0, FONT0, 1, (UCHAR *)"I");
	//UT_WaitKey();
	

	//Verification 8 : Compare the recovered PAN
	memset(temp, 0xff, 10 ); // padded with 'F'

	rspCode = UT_Get_TLVLengthOfV(glv_tag5A.Length,&Tmp_Len1);
	memcpy(temp,glv_tag5A.Value,Tmp_Len1);

	//UT_DumpHex(0,1,250,ICCPK);
	
	if(	UT_CNcmp2(&ICCPK[2+2],temp,10) == FALSE)
		return VGL_OFDA_Fail;

	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"F");
	//UT_WaitKey();

	//Verification 9 : Verify that the last day
	if( UT_VerifyCertificateExpDate( &ICCPK[2+12] ) == FALSE )
    	return	VGL_OFDA_Fail;

//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_N");

   	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"H");
	//UT_WaitKey();
	
/*    UT_GetDateTime(Ter_Tmp_Date);

	//turn ascii to expiry date (MMYY), ex : (Y Y Y Y M M D D) 32 30 31 33 30 33 31 35  =>  0313
	UT_Compress(&Ter_Date[0],Ter_Tmp_Date+2,1);		//terminal year
	UT_Compress(&Ter_Date[1],Ter_Tmp_Date+4,1);		//terminal mouth
//	UT_Compress(&Ter_Date[2],Ter_Tmp_Date+6,1);		//terminal day

//	UT_DumpHex(0,0,2,&ISSPK[2+6]);					//card  certificate expiration date "MMYY"
//	UT_DumpHex(0,1,2,Ter_Date);
		
	//if(UT_CompareDate(Ter_Date, &ISSPK[2+6]) == 1)	// Terminal date > ExpiryDate 
	//	return VGL_OFDA_Fail;
	if(ICCPK[2+13] < Ter_Date[0])	//compare year
		return VGL_OFDA_Fail;
	
	if(ICCPK[2+13] == Ter_Date[0])
	{
		if(ICCPK[2+12] < Ter_Date[1])	//compare mouth
			return VGL_OFDA_Fail;
	}
*/
	//Verification 10 : Check ICC Public Key Algorithm Indicator
	if( ICCPK[2+18] != 0x01 )
        return VGL_OFDA_Fail;

//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_O");


	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"I");
	//UT_WaitKey();

	//Verification 11 :  Concatenate the Leftmost Digits of the ICC Public Key and the ICC Public Key Remainder (if present) to 
	//				obtain the ICC Public Key Modulus
	for( i=0; i<iLeftMostLen; i++ )
		pkm[i+2] = ICCPK[i+2+21];

	rspCode = UT_Get_TLVLengthOfV(glv_tag9F48.Length,&Tmp_Len1);

	if(Tmp_Len1 != 0 )
	{
		memcpy(&pkm[i+2],glv_tag9F48.Value,Tmp_Len1);
		i+=Tmp_Len1;
	}

//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_P");


	//////////////////////////////////////////

		//6.5.2 Dynamic Signature Verification//

	//////////////////////////////////////////

	//Verification 1 : compare the length between "Signed Dynamic Application Data(9F4B)" & "ICC Public key module" 

	rspCode = UT_Get_TLVLengthOfV(glv_tag9F4B.Length,&Tmp_Len1);

	if(Tmp_Len1 != 0)
	{
		if(iModLen == Tmp_Len1)		
		{
			pkm[0]=Tmp_Len1 & 0x00FF;
			pkm[1]=(Tmp_Len1 & 0xFF00)>>8;
			iPKcLen = Tmp_Len1;
		}	
		else
			return VGL_OFDA_Fail;
	}

	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"G");
	//UT_WaitKey();

	//Verification 2 : check recover data tail
	//Load external public key
	if( api_rsa_loadkey( pkm, glv_tag9F47.Value) != apiOK )
		return VGL_OFDA_Fail ;

//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_Q");


	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"H");
	//UT_WaitKey();

	pkc[0]=Tmp_Len1 & 0x00FF;
	pkc[1]=(Tmp_Len1 & 0xFF00)>>8;
	memcpy(&pkc[2],glv_tag9F4B.Value,Tmp_Len1);
	
	//recover	
	if( api_rsa_recover( pkc, pkc ) != apiOK )
		return VGL_OFDA_Fail ;

	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"I");
	//UT_WaitKey();

	//20130625 for ram_gcc_nos_xx6.ld
	for(i=2;i<pkc[0];i++)
	{
		if(pkc[i] != 0x00)
		{
			memmove(&pkc[2],&pkc[i],pkc[0]);
			break;
		}
	}
	
	//if(pkc[0] == 0xF7)
	//{
	//	memmove(&pkc[2],&pkc[3],0xF7);
	//}

	//UT_DumpHex(0,1,250,pkc);

//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_R");

	if(pkc[iPKcLen+2-1] != 0xBC)
		return VGL_OFDA_Fail ;

	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"J");
	//UT_WaitKey();

	//Verification 3:	check recover data header
	if(pkc[2+0] != 0x6A)
		return VGL_OFDA_Fail;

	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"K");
	//UT_WaitKey();

	//Verification 4:	check Signed Data Format
	if(pkc[2+1] != 0x05)
		return VGL_OFDA_Fail;

	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"J");
	//UT_WaitKey();

	//Verification 5:	Concatenate the second to the sixth data elements, followed by DDOL

	//if(*glv_tag9F49.Length != 0)
	//{
	//	rspCode=DOL_Patch_DOLData((UINT)*glv_tag9F49.Length,glv_tag9F49.Value, &g_obuf[0], &g_obuf[1]);
	//}
	//else
	//{	
	//concatenate default PDOL
	i=0;
	memcpy(&g_obuf[1+i],glv_tag9F37.Value,4);
	i+=4;
	memcpy(&g_obuf[1+i],glv_tag9F02.Value,6);
	i+=6;
	memcpy(&g_obuf[1+i],glv_tag5F2A.Value,2);
	i+=2;
	
	rspCode = UT_Get_TLVLengthOfV(glv_tag9F69.Length,&Tmp_Len1);
	memcpy(&g_obuf[1+i],glv_tag9F69.Value,Tmp_Len1);
	i+=Tmp_Len1;

//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_S");

	g_obuf[0] = i;
	//}

	//UT_DumpHex(0,0,g_obuf[0],&g_obuf[1]);
		
	iHashLen = iPKcLen - 22; // header[1], HashResult[20], Trailer[1]
	for( i=0; i<iHashLen; i++ )
	g_ibuf[i] = pkc[i+3]; // from "Signed Data Format" to "Pad pattern"

	memcpy( &g_ibuf[i], &g_obuf[1], g_obuf[0] ); // cat DDOL
	iHashLen += g_obuf[0];


	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"H");
	//UT_WaitKey();

	if( api_sys_SHA1(iHashLen, g_ibuf, temp) != apiOK )
			return VGL_OFDA_Fail;

//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_T");

	for( i=0; i<20; i++ )
	{
		if( temp[i] != pkc[i+iPKcLen-19] ) // offset address of Hash: (iPKcLen+2)-1-20
			return VGL_OFDA_Fail;
	}


//UT_PutStr(0,0, FONT0, 6, (UCHAR *)"FDDA_U");

	//UT_PutMsg(0,0,FONT0,1,(UCHAR *)"I");
	//UT_WaitKey();

	//UT_DumpHex(0,0,100,pkc);
	
	return SUCCESS;
   
}

//Req 5.6 "Offline Data Authentication"
UCHAR VGL_EMV_OFDA(void)
{
	UCHAR	rspCode;
	UCHAR	CTQ[2];
	UCHAR	TTQ[4];
	UINT	V_Len;

	memcpy(TTQ,glv_tag9F66.Value,4);

	//UT_ClearScreen();	
	
//20130313 test for ofda
//	UT_DispHexByte(2,0,VGL_Online_Required_by_Reader);
//	UT_DispHexByte(3,0,VGL_Decline_Required_by_Reader);
//	UT_WaitKey();
	if((VGL_Decline_Required_by_Reader == 0) && (VGL_Online_Required_by_Reader == 0))
	{
//20130313 test for ofda
		rspCode = VGL_OFDA_FDDA();

		if(rspCode == FAIL)	//examine the Card Transaction Qualifiers(CTQ)
		{
			UT_Get_TLVLengthOfV(glv_tag9F6C.Length,&V_Len);
			if(V_Len != 0)
			{
				memcpy(CTQ,glv_tag9F6C.Value,2);
				if((CTQ[0] & 0x20) && (!(TTQ[0] & 0x08)))
				{
					memset(api_W3_DDAFail,0x01,3);
					VGL_Online_Required_by_Reader = 1;
					return SUCCESS;
				}
				else if((CTQ[0] & 0x10) && (TTQ[0] & 0x10))
				{
					return VGL_OFDA_Try_Another;
				}
				else
				{
					//For Layer 3
					L3_Response_Code = VAP_RIF_RC_DDA_AUTH_FAILURE;
						
					VGL_Decline_Required_by_Reader = 1;
					return SUCCESS;
				}
			}
			else
			{
				//For Layer 3
				L3_Response_Code = VAP_RIF_RC_DDA_AUTH_FAILURE;
		
				VGL_Decline_Required_by_Reader = 1;
				return SUCCESS;
			}				
		}
		else
			return SUCCESS;
	}
		return SUCCESS;
}


