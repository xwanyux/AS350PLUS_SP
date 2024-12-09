#include <string.h>
#include "POSAPI.h"
#include "Define.h"
#include "ECL_Tag.h"
#include "Function.h"
#include "MPP_Define.h"
#include "MPP_Function.h"
// Add by Wayne 2020/08/21 to avoid compiler warning
#include "UTILS_CTLS.H"


extern ECL_TAG mpp_lstTlvDB;


void MPP_DOL_Patch_SpecialDOLRelatedData( UCHAR lenOfT, UCHAR *datOfTag )
{
	UINT  iTag;
	UCHAR i;
	UCHAR buf1[22];
	UCHAR buf2[22];

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
					return;
			}

			// setup current value
			UT_GetDateTime( &buf1[1] );
			buf1[0]=6;
			UT_asc2bcd( 3, buf2, buf1 ); // convert to BCD, buf2: YYMMDD

			glv_tag9A.Length[0]=3;
			memcpy(glv_tag9A.Value, buf2, 3);
			
			break;

		case 0x9F21: // transaction time

			for( i=0; i<3; i++ )
			{
				if( glv_tag9F21.Value[i] != 0 ) // ever been set
					return;
			}

			// setup current value
			UT_GetDateTime( &buf1[1] );

			buf1[5]=6;
			UT_asc2bcd( 3, buf2, &buf1[5] ); // convert to BCD, buf2: HHMMSS

			glv_tag9F21.Length[0]=3;
			memcpy(glv_tag9F21.Value, buf2, 3);

			break;

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


UCHAR MPP_DOL_Get_DOLRelatedData(UINT iptLen, UCHAR *iptDOL, UCHAR *optLen, UCHAR *optData)
{
	UCHAR	lenOfT=0;
	UCHAR	lenOfL=0;
	UINT	lenOfV=0;
	UINT	lenOfDOL=0;
	UINT	lenActual=0;
	UCHAR	rspCode=0;
	UINT	recIndex=0;
	UCHAR	difNumber=0;
	UCHAR	idxNumber=0;
	UCHAR	flgDntSearch=0;
	UCHAR	flgFilZero=0;
	UCHAR	*ptrData=NULLPTR;
	UINT	optRmdDatLen=0;
	UCHAR	lenOfT_DB=0;
	UCHAR	lenOfL_DB=0;
	UINT	lenOfV_DB=0;

	//Reset output optLen
	*optLen=0;

	if (iptLen == 0)
		return SUCCESS;

	//Save DOL Length
	lenOfDOL=iptLen;

	//Parse Every TL Element in DOL
	do {
		//Reset Flag
		flgDntSearch=FALSE;
		flgFilZero=FALSE;

		//Get TL Element Length of TLV
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
				MPP_DOL_Patch_SpecialDOLRelatedData(lenOfT, iptDOL);

				//Get Tag Actual Length
				UT_Get_TLVLengthOfV((UCHAR*)glv_addTable[recIndex], &lenActual);

				if (lenActual == 0)
				{
					flgFilZero=TRUE;
				}
				else
				{
					if (lenOfV == lenActual)
					{
						memcpy(optData, glv_addTable[recIndex]+3, lenOfV);
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

							case FORMAT_CN:	//Format='cn': paded with leading hex  0xff (changed to trailing hex 0xff)
								memcpy(optData, glv_addTable[recIndex]+3, lenActual);
								optData+=lenActual;	//Update Pointer

								for(idxNumber=0; idxNumber < difNumber; idxNumber++)
									*optData++ = 0xFF;

								break;

							default:	//others: paded with trailing hex 0x00
								memcpy(optData, glv_addTable[recIndex]+3, lenActual);
								optData+=lenActual;	//Update Pointer

								for(idxNumber=0; idxNumber < difNumber; idxNumber++ )
									*optData++ = 0x00;

								break;
						}
					}
				}
			}
			else	//Tag is Unknown
			{
				//Check If Present in TLV DB
				ptrData=MPP_Search_ListItem(iptDOL, mpp_lstTlvDB.Value, &optRmdDatLen, MPP_LISTTYPE_TLV);
				if (ptrData != NULLPTR)
				{
					UT_Get_TLVLength(ptrData, &lenOfT_DB, &lenOfL_DB, &lenOfV_DB);
					if (lenOfV == lenOfV_DB)
					{
						ptrData+=(lenOfT_DB+lenOfL_DB);
						memcpy(optData, ptrData, lenOfV_DB);
						optData+=lenOfV_DB;
					}
					else
					{
						//Fill with 0
						flgFilZero=TRUE;
					}
				}
				else
				{
					//Fill with 0
					flgFilZero=TRUE;
				}
			}
		}

		if (flgFilZero == TRUE)
		{
			for (idxNumber=0; idxNumber < lenOfV; idxNumber++)
				*optData++ = 0x00;
		}

		*optLen+=lenOfV; 			//Accumulate data element optLen
		iptDOL+=(lenOfT+lenOfL);	//Point to Next Tag
		lenOfDOL-=(lenOfT+lenOfL);
	} while (lenOfDOL > 0);

	return SUCCESS;
}

UCHAR MPP_DSDOL_Get_DOLRelatedData(UINT iptLen, UCHAR *iptDOL, UCHAR *optLen, UCHAR *optData)
{
	UCHAR	lenOfT = 0;
	UCHAR	lenOfL = 0;
	UINT	lenOfV = 0;
	UINT	lenOfDOL = 0;
	UINT	lenActual = 0;
	UCHAR	rspCode = 0;
	UINT	recIndex = 0;
	UCHAR	difNumber = 0;
	UCHAR	idxNumber = 0;
	UCHAR	flgDntSearch = 0;
	UCHAR	flgFilZero = 0;
	UCHAR	*ptrData = NULLPTR;
	UINT	optRmdDatLen = 0;
	UCHAR	lenOfT_DB = 0;
	UCHAR	lenOfL_DB = 0;
	UINT	lenOfV_DB = 0;

	//Reset output optLen
	*optLen = 0;

	if (iptLen == 0)
		return SUCCESS;

	//Save DOL Length
	lenOfDOL = iptLen;

	//Parse Every TL Element in DOL
	do {
		//Reset Flag
		flgDntSearch = FALSE;
		flgFilZero = FALSE;

		//Get TL Element Length of TLV
		rspCode = UT_Get_TLVLengthOfT(iptDOL, &lenOfT);
		lenOfL = 1;
		lenOfV = iptDOL[lenOfT];

		if (rspCode == FAIL)
			return FAIL;

		//Check constructed data element
//		if (apk_EMVCL_CheckConsTag(iptDOL[0]) == TRUE)
		if (UT_Check_ConstructedTag(iptDOL) == TRUE)
		{
			//Don't Search TLV & Fill with 0
			flgDntSearch = TRUE;
			flgFilZero = TRUE;
		}

		if (flgDntSearch == FALSE)
		{
			//Check Tag Table
			rspCode = UT_Search_Record(lenOfT, iptDOL, (UCHAR*)glv_tagTable, ECL_NUMBER_TAG, ECL_SIZE_TAGTABLE, &recIndex);
			if (rspCode == SUCCESS)
			{
				MPP_DOL_Patch_SpecialDOLRelatedData(lenOfT, iptDOL);

				//Get Tag Actual Length
				UT_Get_TLVLengthOfV((UCHAR*)glv_addTable[recIndex], &lenActual);

				if (lenActual == 0)
				{
					flgFilZero = TRUE;
				}
				else
				{
					if (lenOfV == lenActual)
					{
						memcpy(optData, glv_addTable[recIndex] + 3, lenOfV);
						optData += lenOfV;
					}
					else if (lenOfV < lenActual)
					{
						if (glv_tagTable[recIndex].Format == FORMAT_N)
						{
							//Format='n': truncate leftmost bytes
							difNumber = lenActual - lenOfV;
							memcpy(optData, glv_addTable[recIndex] + 3 + difNumber, lenOfV);
							optData += lenOfV;
						}
						else
						{
							//Others: truncate rightmost bytes
							memcpy(optData, glv_addTable[recIndex] + 3, lenOfV);
							optData += lenOfV;
						}
					}
					else if (lenOfV > lenActual)
					{
						memcpy(optData, glv_addTable[recIndex] + 3, lenActual);
						optData += lenActual;
						lenOfV = lenActual;
					}
				}
			}
			else	//Tag is Unknown
			{
				//Check If Present in TLV DB
				ptrData = MPP_Search_ListItem(iptDOL, mpp_lstTlvDB.Value, &optRmdDatLen, MPP_LISTTYPE_TLV);
				if (ptrData != NULLPTR)
				{
					UT_Get_TLVLength(ptrData, &lenOfT_DB, &lenOfL_DB, &lenOfV_DB);
					if (lenOfV == lenOfV_DB)
					{
						ptrData += (lenOfT_DB + lenOfL_DB);
						memcpy(optData, ptrData, lenOfV_DB);
						optData += lenOfV_DB;
					}
					else
					{
						//Fill with 0
						flgFilZero = TRUE;
					}
				}
				else
				{
					//Fill with 0
					flgFilZero = TRUE;
				}
			}
		}

		if (flgFilZero == TRUE)
		{
			for (idxNumber = 0; idxNumber < lenOfV; idxNumber++)
				*optData++ = 0x00;
		}

		*optLen += lenOfV; 			//Accumulate data element optLen
		iptDOL += (lenOfT + lenOfL);	//Point to Next Tag
		lenOfDOL -= (lenOfT + lenOfL);
	} while (lenOfDOL > 0);

	return SUCCESS;
}

