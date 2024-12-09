#include <string.h>
#include "POSAPI.h"
#include "Define.h"
#include "ECL_Tag.h"
#include "Function.h"
#include "UTILS_CTLS.H"

void DOL_Patch_SpecialDOLData( UCHAR lenOfT, UCHAR *datOfTag )
{
	UINT  iTag;
	UCHAR i;
	UCHAR buf1[22];
	UCHAR buf2[22];
//	UCHAR length[1];

	if (lenOfT == 1)
		iTag=datOfTag[0];
	else
		iTag=datOfTag[0]*256+datOfTag[1];
	
	switch( iTag )
	{
		case 0x009A: // transaction date

			for( i=0; i<3; i++ )
			{
				if( glv_tag9A.Value[i] != 0 ) // ever been set
					break;
			}

			// setup current value
			UT_GetDateTime( &buf1[1] );
			buf1[0]=6;
			UT_asc2bcd( 3, buf2, buf1 ); // convert to BCD, buf2: YYMMDD

			glv_tag9A.Length[0]=3;
			memcpy(glv_tag9A.Value, buf2, 3);
			
			break;

		case 0x9F21: // transaction time

			if(glv_tag9F21.Length[0] == 0)
			{
				for( i=0; i<3; i++ )
				{
					if( glv_tag9F21.Value[i] != 0 ) // ever been set
						break;
				}
				// setup current value
				UT_GetDateTime( &buf1[1] );
				buf1[5]=6;
				UT_asc2bcd( 3, buf2, &buf1[5] ); // convert to BCD, buf2: HHMMSS	
				glv_tag9F21.Length[0]=3;
				memcpy(glv_tag9F21.Value, buf2, 3);
			}

			break;

//There's no Tag 98 in EMVCL
/*            case 0x0098: // TC hash value = HASH(TDOL)

                 apk_ReadRamDataICC( ADDR_ICC_TDOL, 254, g_ibuf );

//  g_ibuf[0] = 10;
//  g_ibuf[1] = 0;
//  g_ibuf[2] = 0x5a;
//  g_ibuf[3] = 0x03;
//  g_ibuf[4] = 0x95;
//  g_ibuf[5] = 0x01;
//  g_ibuf[6] = 0x5f;
//  g_ibuf[7] = 0x2d;
//  g_ibuf[8] = 0x06;
//  g_ibuf[9] = 0x5f;
//  g_ibuf[10] = 0x20;
//  g_ibuf[11] = 0x01;

                 if( (g_ibuf[1]*256+g_ibuf[0]) != 0 )
                   {
                   // (1) using ICC TDOL (from card issuer )
                   g_ibuf[1] = g_ibuf[0]; // convert to berLEN format
                   g_ibuf[0] = 0x81;      //
                   Build_DOL2( 0, g_ibuf, length, g_obuf );
                   apk_HASH( SHA1, (UINT)length[0], g_obuf, buf1 ); // generate hash value
                   }
                 else
                   {
                   // (2) using TERM default TDOL (from payment system)
                   //     - if a default TDOL is required but is not present
                   //       in the terminal (because the payment system does not
                   //       support), a default TDOL with no data objects in the
                   //       list shall be assumed.
                   //     - set TVR.default_TDOL_used = 1

                   apk_ReadRamDataTERM( ADDR_TERM_TDOL, 254, g_ibuf );
                   if( (g_ibuf[1]*256+g_ibuf[0]) != 0 )
                     {
                     g_ibuf[1] = g_ibuf[0]; // convert to berLEN format
                     g_ibuf[0] = 0x81;      //
                     Build_DOL2( 0, g_ibuf, length, g_obuf );
                     apk_HASH( SHA1, (UINT)length[0], g_obuf, buf1 ); // generate hash value

                     g_term_TVR[4] |= TVR4_DEFAULT_TDOL_USED; // set TVR bit
                     api_emv_PutDataElement( DE_TERM, ADDR_TERM_TVR+4, 1, &g_term_TVR[4] );
                     }
                   else // neither ICC TDOL nor default TDOL is present
                     {
                //   apk_HASH( SHA1, 0, g_obuf, buf1 ); // generate hash value
                //   NOTE: according to HASH algorithm, put empty message to
                //         calculate the HASH value, ie, length = 0,
                //         the default result is as follows: (IFM cannot support this case)

                     buf1[0]  = 0xDA;
                     buf1[1]  = 0x39;
                     buf1[2]  = 0xA3;
                     buf1[3]  = 0xEE;
                     buf1[4]  = 0x5E;
                     buf1[5]  = 0x6B;
                     buf1[6]  = 0x4B;
                     buf1[7]  = 0x0D;
                     buf1[8]  = 0x32;
                     buf1[9]  = 0x55;
                     buf1[10] = 0xBF;
                     buf1[11] = 0xEF;
                     buf1[12] = 0x95;
                     buf1[13] = 0x60;
                     buf1[14] = 0x18;
                     buf1[15] = 0x90;
                     buf1[16] = 0xAF;
                     buf1[17] = 0xD8;
                     buf1[18] = 0x07;
                     buf1[19] = 0x09;
                     }
                   }

                 // setup current value
                 apk_WriteRamDataTERM( ADDR_TERM_TC_SHA1, 20, buf1 );

                 break;
*/
		case 0x9F37: // unpredictable number
			if( glv_tag9F37.Length[0] != 0 )
				break;

			// setup current value
			api_sys_random(buf1);
			glv_tag9F37.Length[0]=4;
			memcpy(glv_tag9F37.Value, buf1, 4);

			break;
	}
}


UCHAR DOL_Patch_DOLData(UINT iptLen, UCHAR *iptDOL, UCHAR *optLen, UCHAR *optData)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenActual=0;
	UINT	lenOfDOL=0;
//	UCHAR	lenInTable=0;
	UCHAR	rspCode=0;
	UINT	recIndex=0;
	UCHAR	difNumber=0;
	UCHAR	idxNumber=0;
	UCHAR	flgDntSearch=0;
	UCHAR	flgFilZero=0;

	//Reset output optLen
	*optLen=0;

	if (iptLen == 0)
		return FAIL;

	//Save DOL Length
	lenOfDOL=iptLen;

	//Parse Every TL Element is DOL
	do {
		//Reset Flag
		flgDntSearch=FALSE;
		flgFilZero=FALSE;

		/*
		//Get TL Element Length of TLV
		rspCode=UT_Get_TLVLength(iptDOL, &lenOfT, &lenOfL, &lenOfV);
		if (rspCode == FAIL)
			return FAIL;
		*/
		//20130822, Get TL Element Length of TLV following EMV 4.3 Book3
		rspCode=UT_Get_TLVLengthOfT(iptDOL, &lenOfT);
		lenOfL=1;
		lenOfV=iptDOL[lenOfT];

		if (rspCode == FAIL)
			return FAIL;
		
		//Check constructed data element
//		if (apk_EMVCL_CheckConsTag(iptDOL[0]) == TRUE)
		if (UT_Check_ConstructedTag(iptDOL) == TRUE)
		{
			//Don't Search TLV & Fill with 0
			flgDntSearch=TRUE;
			flgFilZero=TRUE;
		}

		if (flgDntSearch == FALSE)
		{
			//Check Tag Table
			rspCode=UT_Search_Record(lenOfT, iptDOL, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
			if (rspCode == SUCCESS)
			{
				DOL_Patch_SpecialDOLData(lenOfT, iptDOL);

				//20130822, we modify this line to follow the SPEC EMV4.3 book 3				
				//lenInTable=glv_tagTable[recIndex].VISA_Length;	//Temporary Use Master Length

				//20130822, Follow the EMV 4.3 book 3
				//Get Tag Actual Length
				UT_Get_TLVLengthOfV((UCHAR*)glv_addTable[recIndex], &lenActual);

				if (lenActual == 0) //EMV 4.3 book 3 section 5.4 B situation, Absent
				{
					//Fill with 0
					flgFilZero=TRUE;
				}
				else
				{
					if (lenOfV == lenActual)
					{
						memcpy(optData, glv_addTable[recIndex]+3, lenOfV);
						//UT_DispHexWord(0,0,recIndex);
						//UT_DumpHex(0,1,lenOfV,optData);
						optData+=lenOfV;
					}
					else if (lenOfV < lenActual)
					{
						if (glv_tagTable[recIndex].Format == FORMAT_N)
						{
							//Format='n': truncate leftmost bytes
							difNumber=lenActual-lenOfV;
							memcpy(optData, glv_addTable[recIndex]+3+difNumber, lenOfV);
							optData += lenOfV;
						}
						else
						{
							//Others: truncate rightmost bytes
							memcpy(optData, glv_addTable[recIndex]+3, lenOfV);
							optData += lenOfV;
						}
					}
					else if (lenOfV > lenActual)
					{
						difNumber=lenOfV-lenActual;

						switch (glv_tagTable[recIndex].Format)
						{
							case FORMAT_N:	//Format='n' : paded with leading hex  0x00
								for(idxNumber=0; idxNumber < difNumber; idxNumber++ )
									*optData++ = 0x00;

								memcpy(optData, glv_addTable[recIndex]+3, lenActual);
								optData+=lenActual;

								break;

							case FORMAT_CN: //Format='cn': paded with leading hex  0xff (changed to trailing hex 0xff)
								memcpy(optData, glv_addTable[recIndex]+3, lenActual);
								optData+=lenActual; //Update Pointer

								for(idxNumber=0; idxNumber < difNumber; idxNumber++)
									*optData++ = 0xFF;

								break;

							default:	//others: paded with trailing hex 0x00
								memcpy(optData, glv_addTable[recIndex]+3, lenActual);
								optData+=lenActual; //Update Pointer

								for(idxNumber=0; idxNumber < difNumber; idxNumber++ )
									*optData++ = 0x00;

								break;
						}
					}
				}
			}
			else	//Tag is Unknown
			{
				//Fill with 0
				flgFilZero=TRUE;
			}
		}

		if (flgFilZero == TRUE)
		{
			for (idxNumber=0; idxNumber < lenOfV; idxNumber++)
				*optData++ = 0x00;
		}

		*optLen+=lenOfV;			//Accumulate data element optLen
		iptDOL+=(lenOfT+lenOfL);	//Point to Next Tag
		lenOfDOL-=(lenOfT+lenOfL);

	} while (lenOfDOL > 0);

	return SUCCESS;
}
	


UCHAR DOL_Get_DOLData(UINT iptLen, UCHAR *iptDOL, UCHAR *optLen, UCHAR *optData)
{
	UCHAR	tmpLen=0;
	UCHAR	tmpBuffer[512]={0};
	UCHAR	rspCode=0;
	UCHAR	rspLen=0;

	rspCode=DOL_Patch_DOLData(iptLen, iptDOL, &tmpLen, tmpBuffer);
	if (rspCode == SUCCESS)
	{
		rspLen=UT_Set_TagLength(tmpLen, &optData[1]);

		if ((1+rspLen+tmpLen) <= DOL_BUFFER_SIZE_RelatedData)	//83(1)+Len(rspLen)+Value(tmpLen)
		{
			optData[0]=0x83;

			if (rspLen == 1)
				memcpy(&optData[2], tmpBuffer, tmpLen);
			else
				memcpy(&optData[3], tmpBuffer, tmpLen);

			optLen[0]=1+rspLen+tmpLen;	//83(1)+Len(rspLen)+Value(tmpLen)

			return SUCCESS;
		}
	}
	return FAIL;
}


