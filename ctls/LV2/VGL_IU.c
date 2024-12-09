#include <string.h>
#include "ECL_Tag.h"
#include "Datastructure.h"
#include "Define.h"
#include "Function.h"
#include "VGL_Function.h"
#include "VGL_Kernel_Define.h"
#include "ECL_LV1_Define.h"
#include "ECL_LV1_Function.h"

// Add by Wayne 2020/08/21 to avoid compiler warning
#include "UTILS_CTLS.H"

//api_pcd_vap_function.c
extern UCHAR	Issuer_Script_Data[1024];
extern UINT		*Script_Data_Length;
extern UCHAR	Issuer_Script_Result[81];
extern UCHAR	*Issuer_Script_Result_Length;
extern UCHAR	IAD[16];
extern UCHAR	IAD_Len;

//Glv_ReaderConfPara.c
extern UCHAR	Opt_qVSDC_ISSUER_Update	;

//ETP_Entrypoint.c
extern OUTCOME etp_Outcome;
extern UCHAR   etp_flgCmdTimeout;

//VGL_Kernel.c
extern UCHAR	VGL_rcvBuff[DEP_BUFFER_SIZE_RECEIVE];	//Receive Data Buffer
extern UINT		VGL_rcvLen;								//Receive Length
extern UCHAR	VGL_sendBuff[DEP_BUFFER_SIZE_SEND];		//Send Data Buffer
extern UINT		VGL_sendLen;							//Send Length

void VGL_IUS_SEC_TAP_COMPLETE(void)
{
	etp_Outcome.Start			= ETP_OCP_Start_NA;
	memset(etp_Outcome.rspData, 0, sizeof(etp_Outcome.rspData));	
	etp_Outcome.CVM				= ETP_OCP_CVM_NA;
	etp_Outcome.rqtOutcome		= ETP_OCP_UIOnOutcomePresent_No;
	etp_Outcome.rqtRestart		= ETP_OCP_UIOnRestartPresent_No;
	etp_Outcome.datRecord		= ETP_OCP_DataRecordPresent_No;
	etp_Outcome.dscData			= ETP_OCP_DiscretionaryDataPresent_No;	
	etp_Outcome.altInterface	= ETP_OCP_AlternateInterface_NA;
	etp_Outcome.Receipt			= ETP_OCP_Receipt_NA;
	etp_Outcome.filOffRequest	= ETP_OCP_FieldOffRequest_NA;
	etp_Outcome.rmvTimeout		= 0;
}


/*void VGL_IUS_Check_SW12(UCHAR *datSW,UCHAR *Result,UCHAR Seq)
{
	if (((datSW[0] == 0x90) && (datSW[1] == 0x00)) || (datSW[0] == 0x62) || (datSW[0] == 0x63))
	{
		if(Seq > 0x0F)
		{
			Seq = 0x0F;
			Result[0] = 0x20 | Seq;
		}
		else
			Result[0] = 0x20 | Seq;
	}
	else		//error code 
	{
		if(Seq > 0x0F)
		{
			Seq = 0x0F;
			Result[0] = 0x10 | Seq;
		}
		else
			Result[0] = 0x10 | Seq;
	}
}*/

UCHAR VGL_Ex_Auth(UINT sndLen, UCHAR *sndData, UINT *rcvLen, UCHAR *rcvData)
{
	UCHAR rspCode;
	
	// VGL_sendBuff = Ex_Auth_Header (CLA+INS+P1+P2 )+ Len (Lc) + Ex_Auth_Data (Data)
	UCHAR Ex_Auth_Header[4]={0x00,0x82,0x00,0x00};
		
	memcpy(VGL_sendBuff,Ex_Auth_Header,4);				//Copy header
	VGL_sendBuff[4] = (UCHAR)sndLen ;					//Total length
	memcpy(&VGL_sendBuff[5],sndData,sndLen);			//Copy data

	VGL_sendLen = sndLen + 5;

	//	UT_DumpHex(0x00,0,*rcvLen,rcvData);

	rspCode = ECL_LV1_DEP(VGL_sendLen , VGL_sendBuff , rcvLen, rcvData, 1000);

	if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
		etp_flgCmdTimeout=TRUE;

	if(rspCode == ECL_LV1_SUCCESS)
		return SUCCESS;
	else
		return FAIL;
}

// ---------------------------------------------------------------------------
// FUNCTION: update issuer script results. (5-byte / id )
// INPUT   : id     - the script id (4 bytes)
//           sn     - the sequence number. (0-15)
//           result - the final result of the specified id.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void  VGL_UpdateISR( UCHAR *id, UCHAR sn, UCHAR result )
{
	UCHAR *addr;
	UCHAR cnt;
	UCHAR isr[5];

	//apk_ReadRamDataTERM( ADDR_TERM_ISR, 1, (UCHAR *)&cnt ); // get current counter
	cnt = Issuer_Script_Result[0];
    addr = &Issuer_Script_Result[0] + cnt*5 + 1; // next isr address

      if( result == 0x02 )
        sn = 0; // if successful, do not specify sn.

      isr[0] = ((result << 4) & 0xf0) + sn;
      memmove( &isr[1], id, 4 );

      // update
      cnt++;
      //apk_WriteRamDataTERM( ADDR_TERM_ISR, 1, (UCHAR *)&cnt );
      Issuer_Script_Result[0] = cnt;
      memcpy( addr, isr, 5);
}


// ---------------------------------------------------------------------------
// FUNCTION: process the issuer script commands with Template Tag=0x71.
//           (processed before issuing the final GENERATE AC command)
//           (1) read one template head [7x-L-(9F18-L-V)]
//           (2) read one script command [86-Ln-Vn]
//           (3) loop (2)
//           (4) loop (1)
// INPUT   : ist - 2L-[T-L-V], issuer scripts template.
//                 T-L-(9F18-04-ScriptID[4])-[Commands], T=71 or 72.
//                 Commands: [86-L1-CMD1][86-L2-CMD2][86-L3-CMD3]...
//                 ScriptID: optional, also "9F18-04".
//                 The "ist" is located at larger ADDR_ISU_SCRIPT_TEMP[] pool
//                 instead of at limited global memory.
//                 format: 2L-DATA[n]
//           tag - the script tag to be process. (0x71 or 0x72)
// OUTPUT  : none.
// REF     : g_ibuf
// RETURN  : apkOK           (SW1SW2 = 90xx, 62xx, 63xx)
//           apkFailed       (SW1SW2 = 69xx, 6Axx)
//           apkOutOfService (not supported or device error)
//           NotReady     (not performed, eg. tag=71 but isc=72...)
//
// NOTE    : terminal shall maintain the "Issuer Script Results" (ADDR_TERM_ISR)
//           BYTE 1: bit5-8
//                        0=script not performed
//                        1=script processing failed
//                        2=script processing successful
//                   bit1-4
//                        0=not specified (OK)
//                        1 ~ E = sequence number       (error)
//                        F     = sequence number >= 15 (error)
//           BYTE 2-5: Script ID. (filled in 0's if not available).
// ---------------------------------------------------------------------------
UCHAR VGL_IssuerScriptProcessing2( UCHAR *addr_ist )
{
UCHAR ist[300];
UINT  i;
int   iTempLen = 0;
int   iCmdLen = 0;
UINT  iLen;
UCHAR *iAddr;
UINT  iPileLen;
UCHAR *ptrist;
UCHAR cnt;
//UCHAR tag2;
UCHAR script_id[4];
UCHAR script_sn;
UCHAR script_result;
UCHAR flag_exe;
UCHAR flag_err_format;
UCHAR flag_ever;
UCHAR flag_isc_err;
//sam
UCHAR rspCode;

	flag_exe = FALSE;  // reset scripts execution flag
    flag_ever = FALSE; // reset scripts ever executed flag
    flag_err_format = FALSE; // script format

    iAddr = addr_ist; // set start address

	iTempLen = addr_ist[0];
	

//	UT_ClearScreen();
//	UT_DispHexWord(0,0,iTempLen);
//	UT_WaitKey();
	
    if( iTempLen < 8 )
    	return  IU_FAIL;

    iAddr += 2;

    iPileLen = 0;

ISC_ST:

    while( iTempLen > 0 ) // scripts
    {
    	flag_isc_err = FALSE;    // ISC.SW1 !=90, 62 or 63
        memset( script_id, 0x00, 4 );
        script_sn = 0;
        script_result = 0;

		memcpy(ist,iAddr,256);
        ptrist = ist; // pointer to the 1'st [T]

        // scan the template Tag 71 or 72
        if( (*ptrist != 0x71) && (*ptrist != 0x72) )
        {
        	while( iTempLen > 0 )
            {
            	if( (*ptrist != 0x71) && (*ptrist != 0x72) )
                {
                	ptrist++;
                    iTempLen--;
                    iAddr++;
                }
                else
                {
                	if( iTempLen < 8 )
                    	return  IU_FAIL;
                    else
                      	goto ISC_START;
                }
            }

			return  FAIL;
        }

ISC_START:

        if(( *ptrist == 0x71 ) || ( *ptrist == 0x72 ))
        {
        	UT_EMVCL_GetBERLEN( ++ptrist, (UCHAR *)&cnt ,&iLen); // 71-L
            ptrist += cnt; // pointer to next [T] (9F18 or 86)

            iPileLen += (iLen + 1 + cnt);                           // PATCH: 2006-10-12, 2CO.034.02, 2CO.034.03

	        if( iPileLen > 128 )                                    //
            {                                                     //
    	        VGL_UpdateISR( script_id, script_sn, script_result ); // set default ISR = 00 00 00 00 00
            	return IU_FAIL;
            }

// TL_DispHexWord(1,0,iLen);
// TL_DispHexWord(2,0,iTempLen);
// UI_WaitKey();
            if( iLen >= iTempLen ) // script 71 length > total script length
            {
            	VGL_UpdateISR( script_id, script_sn, script_result ); // set default ISR = 00 00 00 00 00
               	return  IU_FAIL;
            }

            if( iTempLen > (iLen + 2) )	//  total script length > script 71 length
            {
            	if( (ist[iLen+2] != 0x71) && (ist[iLen+2] != 0x72) )
                {
                // 7x length error, skip this template and find next 7x
                	if( (*ptrist == 0x9F) && (*(ptrist+1) == 0x18) )
                   	{
                   		if( *(ptrist + 2) == 4 )
                     		memmove( script_id, ptrist+3, 4 );
                   	}

                 	flag_exe = TRUE;
                 	flag_ever = TRUE;
                 	flag_err_format = TRUE;

                 	iAddr += 2;
                 	iTempLen -= 2;

					while( iTempLen > 0 )
                    {
                    	if( (*ptrist != 0x71) && (*ptrist != 0x72) )
                        {
	                        ptrist++;
	                        iAddr++;
	                        iTempLen--;
                        }
                      	else // found
                        	goto ISC_2000;
                    }

                 // PATCH: PBOC2.0, 2006-02-15, 2CO.035.00c-1
                 //	apk_ReadRamDataTERM( ADDR_TERM_ISR, 1, (UCHAR *)&cnt ); // get current counter
                 	cnt = Issuer_Script_Result[0];
                 	if( cnt == 0 )
                   	{
                   		VGL_UpdateISR( script_id, script_sn, script_result ); // set default ISR = 00 00 00 00 00
            //     return  NotReady;  2006-02-17 removed
                   	}
                 	return  IU_FAIL;
               	}
          	}

             // ScriptID present?
            if( (*ptrist == 0x9F) && (*(ptrist+1) == 0x18) )
            {
            	if( *(ptrist+2) == 0x00 ) // PATCH: 2003-09-16, seems "no ID"
                {
	                ptrist += 3; // skip "9F 18 00"
	                iCmdLen = iLen - 3;
	                iAddr += (cnt + 1 + 3); // pointer to the 1'st [T=86]

                 	goto ISC_1000;
                }

               	if( *(ptrist+2) != 0x04 )
                	return  IU_FAIL; // illegal ScriptID size

               	memmove( script_id, ptrist+3, 4 );
               	script_sn = 0;

               	ptrist += 7; // yes, skip "ScriptID" and go to next [Tn]
               	iCmdLen = iLen - 7;
               	iAddr += (cnt + 1 + 7); // pointer to the 1'st [T=86]
            }
            else
            {

               // PATCH: PBOC2.0, 2006-02-17, 2CO036, neither 9F18 nor 86 then ignore current script
            	if( (*ptrist == 0x9F) && (*(ptrist+2) == 0x04) && (*ptrist != 0x86) )
                {
                	iTempLen -= (iLen + 1 + cnt);
                 	iAddr += (iLen + 1 + cnt);

                 	VGL_UpdateISR( script_id, script_sn, script_result ); // PATCH: PBOC2.0, 20006-02-17

                 	flag_exe = TRUE;
                 	flag_ever = TRUE;
                 	//ISP_SetFailedTVR( tag );

                 	goto ISC_ST; // parse next one
                }

               	iCmdLen = iLen;
               	iAddr += (cnt + 1);     // pointer to the 1'st [T=86]
          	}
ISC_1000:
             iTempLen -= (iLen + cnt + 1); // remaining template data size

 // 	UT_DispHexWord(0,0,iTempLen);
 // 	UT_DispHexWord(1,0,iCmdLen);
 // 	UT_DispHexWord(2,0,iLen);
 //	UT_WaitKey();

            // script commands processing: send commands to ICC in sequence
            while( iCmdLen > 0 )
			{
				memcpy(ist,iAddr,265);
//            	apk_ReadRamDataTERM( iAddr, 265, ist ); // read one script command
                ptrist = ist;

				//UT_DumpHex(0,4,3,ptrist);

				if( *ptrist == 0x86 )
				{
                	UT_EMVCL_GetBERLEN( ++ptrist, (UCHAR *)&cnt ,&iLen );
                    ptrist += cnt; // pointer to next cmd [Vn]

                    // PATCH: 2006-10-12, 2CJ.195.00-02, 2CO.034.00-01 (Tag 86 with incorrect LEN)
                    if( (iLen+1+cnt) < iCmdLen )
                    {
                    	if( *(ptrist+iLen) != 0x86 )
                        {
	                        flag_exe = TRUE;
	                        flag_ever = TRUE;
	                        //ISP_SetFailedTVR( tag );

	                        iAddr += iCmdLen; // ptr to next [7x-Ln-Vn]
	                        goto ISC_2000;
	                    }
                    }
                    //EMV 42a 2009-03-05 Charles 2CO.035.02 case1~4
                    else if((iLen+1+cnt)>iCmdLen)
                    {
                    	//flag_err_format = TRUE;
                    	VGL_UpdateISR( script_id, script_sn, script_result );

						//20130623 ICTK
						iAddr += iCmdLen; // ptr to next [7x-Ln-Vn]
                    	goto ISC_ST;
                    }

                  	if( flag_isc_err == TRUE ) // PATCH: 2006-10-14
    	           	{                        //
                   		ptrist += iLen;          // skip other Tag86 cmd in the same template
	                    goto ISC_NEXT_TAG86;     //
                   	}

                    script_sn++;

					if( script_sn >= 15 )
                    	script_sn = 15;

                    flag_exe = TRUE;
                    flag_ever = TRUE;

					//UT_DumpHex(0,0,iLen,ptrist);

					rspCode = ECL_LV1_DEP(iLen,ptrist,&VGL_rcvLen,VGL_rcvBuff, 1000);							
					//		VGL_IUS_Check_SW12(&VGL_rcvBuff[VGL_rcvLen-2],ScrResult,ScrSeq);

					if ((rspCode == ECL_LV1_TIMEOUT_ISO) || (rspCode == ECL_LV1_TIMEOUT_USER))
						etp_flgCmdTimeout=TRUE;

					if( rspCode == ECL_LV1_SUCCESS )
                    {
                    	ptrist += iLen;

                      	i = VGL_rcvLen; // size of response APDU
                      	if( i >= 2 )
                        {
                        // examine only SW1
                        	switch( VGL_rcvBuff[VGL_rcvLen-2] )
                            {
                            	case 0x90:
                              	case 0x62:
                              	case 0x63:

                                script_result = 2; // successful
                                break;

                              	default: // 0x69, 0x6a, or others, cf: 2CJ.199.00, 2CM,048.00-07, 2006-10-14

                                script_result = 1; // failed
                            //     apk_UpdateISR( script_id, script_sn, script_result );
                            //     return  FAIL;

                                //ISP_SetFailedTVR( tag ); // PATCH: 2006-10-14
                                flag_isc_err = TRUE;     //
                             }
                        }
                      	else
                        {
	                        script_result = 1; // failed
	                        VGL_UpdateISR( script_id, script_sn, script_result );

	                        return  IU_FAIL;
                        }
ISC_NEXT_TAG86:
                      	iCmdLen -= (iLen + cnt + 1); // 86 Ln Vn
                      	iAddr += (iLen + cnt + 1); // pointer to next [86-Ln-Vn]

						//UT_DumpHex(0,2,20,Issuer_Script_Result);
                    }
                    else
                    	return	IU_OutOfService; // error
				}
                else // tag 86 is missing (=not performed)
                {
                	script_result = 0; // not specified, PATCH: 2005/05/06
                    flag_exe = TRUE;
                    flag_ever = TRUE;       // ever try to execute
                    flag_err_format = TRUE; // invalid script command tag

                    UT_EMVCL_GetBERLEN( ++ptrist, (UCHAR *)&cnt ,&iLen );

                    if( iCmdLen >= (iLen + cnt + 1) )   // PATCH: 2005/05/06
                    {
                    	iCmdLen -= (iLen + cnt + 1); // 86 Ln Vn
                      	iAddr += (iLen + cnt + 1); // pointer to next [86-Ln-Vn]
                    }
                    else
                    {                      // PATCH: 2005/05/18
                    	if( flag_exe == TRUE ) // 2CJ.194.00, 2CJ.195.00
                        {
                        	flag_exe = FALSE;
                        	VGL_UpdateISR( script_id, script_sn, script_result );
                        }
						
						//20130623 ICTK
						iAddr += iCmdLen;
						iCmdLen -= iCmdLen;

                      	//goto ISC_EXIT; // terminate
                    }

				}

			} // while( iCmdLen > 0 ) for next command (86)
ISC_2000:
            if( flag_exe == TRUE ) // PATCH: 2005/05/06
            {
            	flag_exe = FALSE;
               	VGL_UpdateISR( script_id, script_sn, script_result );
            }
		}
	} // while( iTempLen > 0 ) for next script (71 or 72)

//ISC_EXIT:

	if( flag_err_format == TRUE )
    	return  IU_FAIL;
    else
    {
    	if( flag_ever == TRUE )
        	return  IU_SUCCESS; // done
        else
          	return  IU_NotReady; // PATCH: 2003-08-03
                                 // script is not performed
	}
}



UCHAR VGL_Issuer_Update(void)
{

//	VGL_Store_IUST(IUSTest);
	
	UCHAR rspCode = 0;
	UINT V_Len;

	if((glv_tag9F66.Value[2] & 0x80) || Opt_qVSDC_ISSUER_Update)
	{
		//External Authenticate : Issuer Authentication Data copy from "DatatoReader" in Terminal	
		//Test, assume we have received Issuer Authentication Data

		if(IAD_Len != 0)
		{
			glv_tag91.Length[0] = IAD_Len;
			memcpy(glv_tag91.Value,IAD,IAD_Len);
		}

//		UT_DumpHex(0,0,glv_tag91.Length[0],glv_tag91.Value);

//		UT_DumpHex(0,3,Issuer_Script_Data[0],Issuer_Script_Data);
		
		UT_Get_TLVLengthOfV(glv_tag91.Length,&V_Len);

		if(V_Len != 0)	//Issuer Authentication Data received?
		{
			//EXTERNAL AUTHENTICATE Command
			rspCode = VGL_Ex_Auth(V_Len , glv_tag91.Value , &VGL_rcvLen , VGL_rcvBuff);

			/* CL K3	Requirements 6.2.1.1
			Note: The kernel does not perform processing based on the card
			response to the EXTERNAL AUTHENTICATE command. The kernel
			continues Issuer Update Processing regardless of the SW1 SW2
			value returned by the card.*/
					
			if(rspCode == SUCCESS)
			{	
				rspCode = VGL_IssuerScriptProcessing2(Issuer_Script_Data);
			}
			else
				return FAIL;
		}
		else
		{
			rspCode = VGL_IssuerScriptProcessing2(Issuer_Script_Data);
			//return SUCCESS;
		}
	
		//UT_DumpHex(0,0,81,Issuer_Script_Result);

		VGL_IUS_SEC_TAP_COMPLETE();

		return rspCode;
	}
	else 
		return FAIL;

	//return SUCCESS;
}


