//============================================================================
//****************************************************************************
//**                                                                        **
//**  PROJECT  : i.MX6UL                                                    **
//**  PRODUCT  : AS350-X6                                                   **
//**                                                                        **
//**  FILE     : OS_SECM.C                                                  **
//**  MODULE   : 			                                                **
//**									                                    **
//**  FUNCTION : Secure Memory Management.                                  **
//**  VERSION  : V1.01                                                      **
//**  DATE     : 2022/05/24                                                 **
//**  EDITOR   : James Hsieh                                                **
//**                                                                        **
//**  Copyright(C) 2022 SymLink Corporation. All rights reserved.           **
//**                                                                        **
//****************************************************************************
//============================================================================
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/fcntl.h> 
#include <sys/stat.h>
#include <sys/ioctl.h> 
#include <string.h>

#include "bsp_types.h"
#include "OS_SECM.h"


#include "caam_keyblob.h"


static UINT8 skeymod[] = {
	0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08,
	0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
};

FILE_INFO fileInfo[] = {
    {"FILE_SYS_RESET_FLAG", 4}, //to be removed
    {"FILE_APP_CTRL_TBL", 32},  //to be removed
    {"FILE_SYS_SRED_STATUS", 1},
    {"FILE_SYS_LOAD_IKEK_FLAG", 1}, //to be removed
    {"FILE_SYS_SETUP_128_TDES_KPK_FLAG", 1},
    {"FILE_SYS_SETUP_192_TDES_KPK_FLAG", 1},
    {"FILE_SYS_SETUP_128_AES_KPK_FLAG", 1},
    {"FILE_SYS_SETUP_192_AES_KPK_FLAG", 1},
    {"FILE_SYS_SETUP_256_AES_KPK_FLAG", 1},
    {"FILE_SYS_CONF_RFU", 21},
    {"FILE_PED_KEY_MODE", 1},
    {"FILE_PED_TDES_KEY_PROTECT_KEY", PED_TDES_KEY_PROTECT_KEY_SLOT_LEN},   //maybe this should be removed
    {"FILE_PED_128_TDES_KEY_PROTECT_KEY", PED_128_TDES_KEY_PROTECT_KEY_SLOT_LEN},
    {"FILE_PED_192_TDES_KEY_PROTECT_KEY", PED_192_TDES_KEY_PROTECT_KEY_SLOT_LEN},
    {"FILE_PED_AES_KEY_PROTECT_KEY", PED_AES_KEY_PROTECT_KEY_SLOT_LEN}, //maybe this should be removed
    {"FILE_PED_128_AES_KEY_PROTECT_KEY", PED_128_AES_KEY_PROTECT_KEY_SLOT_LEN},
    {"FILE_PED_192_AES_KEY_PROTECT_KEY", PED_192_AES_KEY_PROTECT_KEY_SLOT_LEN},
    {"FILE_PED_256_AES_KEY_PROTECT_KEY", PED_256_AES_KEY_PROTECT_KEY_SLOT_LEN},
    {"FILE_KSN_REG", KSN_REG_LEN},  //maybe this should be removed
    {"FILE_FUTURE_KEY_REG", FUTURE_KEY_SLOT_LEN*MAX_FUTURE_KEY_REG_CNT}, //maybe this should be removed
    {"FILE_MAC_KEY_REG", FUTURE_KEY_SLOT_LEN},  //maybe this should be removed
    {"FILE_AES_DUKPT_KEY_TYPE", 1},
    {"FILE_INT_DERIVATION_KEY_REG", MAX_NUM_REG*MAX_KEYLENGTH},
    {"FILE_INT_DERIVATION_KEY_IN_USE", MAX_NUM_REG},
    {"FILE_CUR_DERIVATION_KEY", 4},
    {"FILE_TRANSACTION_COUNTER", 4},
    {"FILE_PED_MKEY_INDEX", 1}, //to be removed
    {"FILE_PED_SKEY_INDEX", 1}, //to be removed
    {"FILE_PED_MKEY", PED_MSKEY_SLOT_LEN},  //to be removed
    {"FILE_PED_SKEY_01", PED_MSKEY_SLOT_LEN},   //to be removed
    {"FILE_PED_SKEY_02", PED_MSKEY_SLOT_LEN},   //to be removed
    {"FILE_PED_SKEY_03", PED_MSKEY_SLOT_LEN},   //to be removed
    {"FILE_PED_SKEY_04", PED_MSKEY_SLOT_LEN},   //to be removed
    {"FILE_PED_SKEY_05", PED_MSKEY_SLOT_LEN},   //to be removed
    {"FILE_PED_SKEY_06", PED_MSKEY_SLOT_LEN},   //to be removed
    {"FILE_PED_SKEY_07", PED_MSKEY_SLOT_LEN},   //to be removed
    {"FILE_PED_SKEY_08", PED_MSKEY_SLOT_LEN},   //to be removed
    {"FILE_PED_SKEY_09", PED_MSKEY_SLOT_LEN},   //to be removed
    {"FILE_PED_SKEY_10", PED_MSKEY_SLOT_LEN},   //to be removed
    {"FILE_PED_PEK_KEY_TYPE", 1},
    {"FILE_PED_PEK_MKEY_INDEX", 1},
    {"FILE_PED_PEK_SKEY_INDEX", 1},
    {"FILE_PED_PEK_TDES_MKEY", PED_PEK_TDES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_TDES_SKEY_01", PED_PEK_TDES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_TDES_SKEY_02", PED_PEK_TDES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_TDES_SKEY_03", PED_PEK_TDES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_TDES_SKEY_04", PED_PEK_TDES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_TDES_SKEY_05", PED_PEK_TDES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_TDES_SKEY_06", PED_PEK_TDES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_TDES_SKEY_07", PED_PEK_TDES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_TDES_SKEY_08", PED_PEK_TDES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_TDES_SKEY_09", PED_PEK_TDES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_TDES_SKEY_10", PED_PEK_TDES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_AES_MKEY", PED_PEK_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_AES_SKEY_01", PED_PEK_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_AES_SKEY_02", PED_PEK_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_AES_SKEY_03", PED_PEK_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_AES_SKEY_04", PED_PEK_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_AES_SKEY_05", PED_PEK_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_AES_SKEY_06", PED_PEK_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_AES_SKEY_07", PED_PEK_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_AES_SKEY_08", PED_PEK_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_AES_SKEY_09", PED_PEK_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_PEK_AES_SKEY_10", PED_PEK_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_PIN", PED_PIN_SLOT_LEN},
    {"FILE_PED_ADMIN_PSW1", PED_PSW_SLOT_LEN},
    {"FILE_PED_ADMIN_PSW2", PED_PSW_SLOT_LEN},
    {"FILE_PED_USER_PSW", PED_PSW_SLOT_LEN},    //to be removed
    {"FILE_PED_DSS_PSW1", PED_PSW_SLOT_LEN},
    {"FILE_PED_DSS_PSW2", PED_PSW_SLOT_LEN},
    {"FILE_PED_SRED_PSW1", PED_PSW_SLOT_LEN},   //maybe this should be removed
    {"FILE_PED_SRED_PSW2", PED_PSW_SLOT_LEN},   //maybe this should be removed
    {"FILE_PED_TERM_ID", PED_TERM_ID_LEN},
    {"FILE_PED_AccDEK_KEY_TYPE", 1},
    {"FILE_PED_ACC_DEK_MKEY_INDEX", 1}, //to be removed
    {"FILE_PED_ACC_DEK_SKEY_INDEX", 1}, //to be removed
    {"FILE_PED_ACC_DEK_MKEY", PED_ACC_DEK_MSKEY_SLOT_LEN},  //to be removed
    {"FILE_PED_ACC_DEK_SKEY_01", PED_ACC_DEK_MSKEY_SLOT_LEN}, //to be removed
    {"FILE_PED_ACC_DEK_TDES_MKEY", PED_ACC_DEK_TDES_MSKEY_SLOT_LEN},
    {"FILE_PED_ACC_DEK_TDES_SKEY_01", PED_ACC_DEK_TDES_MSKEY_SLOT_LEN},
    {"FILE_PED_ACC_DEK_AES_MKEY", PED_ACC_DEK_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_ACC_DEK_AES_SKEY_01", PED_ACC_DEK_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_ACC_DEK_AES_DUKPT_WORKING_KEY", PED_ACC_DEK_AES_DUKPT_WORKING_KEY_LEN},
    {"FILE_PED_ACC_DEK_CURRENT_KSN", 12},
    {"FILE_PED_SALT", PED_SALT_LEN},
    {"FILE_PED_PAN_DATA", PED_PAN_DATA_SLOT_LEN},
    {"FILE_TAG57_L", 1},
    {"FILE_TAG57_V", LENGTH_TAG57},
    {"FILE_TAG5A_L", 1},
    {"FILE_TAG5A_V", LENGTH_TAG5A},
    {"FILE_TAG9F1F_L", 1},
    {"FILE_TAG9F1F_V", LENGTH_TAG9F1F},
    {"FILE_TAG9F20_L", 1},
    {"FILE_TAG9F20_V", LENGTH_TAG9F20},
    {"FILE_PED_FPE_KEY_TYPE", 1},
    {"FILE_PED_FPE_KEY_MKEY_INDEX", 1},
    {"FILE_PED_FPE_KEY_SKEY_INDEX", 1},
    {"FILE_PED_FPE_KEY_AES_MKEY", PED_FPE_KEY_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_FPE_KEY_AES_SKEY_01", PED_FPE_KEY_AES_MSKEY_SLOT_LEN},
    {"FILE_PED_FPE_KEY_AES_DUKPT_WORKING_KEY", PED_FPE_KEY_AES_DUKPT_WORKING_KEY_LEN},
    {"FILE_PED_FPE_KEY_CURRENT_KSN", 12},
    {"FILE_PED_CAPK", CAPK_KEY_SLOT_LEN*MAX_CAPK_CNT},
    {"FILE_EDC_PRV_KEY", 2048},
    {"FILE_DEVICE_AUTH_STATUS", 1},
    {"FILE_DEVICE_AUTH_DATE_TIME", 12},
    {"FILE_CA_CERTIFICATE", 2048},
    {"FILE_CLIENT_CERTIFICATE", 5120},
    {"FILE_CLIENT_PRV_KEY", 2048}
};

// ---------------------------------------------------------------------------
// FUNCTION: Get secure file name.
// INPUT   : address - start address of data in secure memory.
// OUTPUT  : fileName - name of the corresponding secure file.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    getSecureFileName(UINT32 address, char *fileName)
{
    switch(address)
    {
        //--- System Configuration Area ---
        case ADDR_SYS_RESET_FLAG:   //to be removed
            strcpy(fileName, "FILE_SYS_RESET_FLAG");
            break;

        case ADDR_APP_CTRL_TBL: //to be removed
            strcpy(fileName, "FILE_APP_CTRL_TBL");
            break;

        case ADDR_SYS_SRED_STATUS:
            strcpy(fileName, "FILE_SYS_SRED_STATUS");
            break;

        case ADDR_SYS_LOAD_IKEK_FLAG:   //to be removed
            strcpy(fileName, "FILE_SYS_LOAD_IKEK_FLAG");
            break;

        case ADDR_SYS_SETUP_128_TDES_KPK_FLAG:
            strcpy(fileName, "FILE_SYS_SETUP_128_TDES_KPK_FLAG");
            break;

        case ADDR_SYS_SETUP_192_TDES_KPK_FLAG:
            strcpy(fileName, "FILE_SYS_SETUP_192_TDES_KPK_FLAG");
            break;

        case ADDR_SYS_SETUP_128_AES_KPK_FLAG:
            strcpy(fileName, "FILE_SYS_SETUP_128_AES_KPK_FLAG");
            break;

        case ADDR_SYS_SETUP_192_AES_KPK_FLAG:
            strcpy(fileName, "FILE_SYS_SETUP_192_AES_KPK_FLAG");
            break;
        
        case ADDR_SYS_SETUP_256_AES_KPK_FLAG:
            strcpy(fileName, "FILE_SYS_SETUP_256_AES_KPK_FLAG");
            break;

        case ADDR_SYS_CONF_RFU:
            strcpy(fileName, "FILE_SYS_CONF_RFU");
            break;
        
        case ADDR_PED_KEY_MODE:
            strcpy(fileName, "FILE_PED_KEY_MODE");
            break;

        // ---- ANSI TR-31 TDES Key Block Protection Key (KPK) ---
        case ADDR_PED_TDES_KEY_PROTECT_KEY: //maybe this should be removed
            strcpy(fileName, "FILE_PED_TDES_KEY_PROTECT_KEY");
            break;
        
        // --- ANSI X9.143 TDES 16-byte Key Block Protection Key (KPK) ---
        case ADDR_PED_128_TDES_KEY_PROTECT_KEY:
            strcpy(fileName, "FILE_PED_128_TDES_KEY_PROTECT_KEY");
            break;
        
        // --- ANSI X9.143 TDES 24-byte Key Block Protection Key (KPK) ---
        case ADDR_PED_192_TDES_KEY_PROTECT_KEY:
            strcpy(fileName, "FILE_PED_192_TDES_KEY_PROTECT_KEY");
            break;
        
        // --- ANSI X9.143 TDES 32-byte Key Block Protection Key (KPK) ---
        case ADDR_PED_AES_KEY_PROTECT_KEY: //maybe this should be removed
            strcpy(fileName, "FILE_PED_AES_KEY_PROTECT_KEY");
            break;
        
        // --- ANSI X9.143 AES 16-byte Key Block Protection Key (KPK) ---
        case ADDR_PED_128_AES_KEY_PROTECT_KEY:
            strcpy(fileName, "FILE_PED_128_AES_KEY_PROTECT_KEY");
            break;
        
        // --- ANSI X9.143 AES 24-byte Key Block Protection Key (KPK) ---
        case ADDR_PED_192_AES_KEY_PROTECT_KEY:
            strcpy(fileName, "FILE_PED_192_AES_KEY_PROTECT_KEY");
            break;
        
        // --- ANSI X9.143 AES 32-byte Key Block Protection Key (KPK) ---
        case ADDR_PED_256_AES_KEY_PROTECT_KEY:
            strcpy(fileName, "FILE_PED_256_AES_KEY_PROTECT_KEY");
            break;
        
        // --- DUKPT Key Serial Number Register (KSN) ---
        case ADDR_KSN_REG:  //maybe this should be removed
            strcpy(fileName, "FILE_KSN_REG");
            break;

        // --- DUKPT Future Key Register ---
        case ADDR_FUTURE_KEY_REG:  //maybe this should be removed
            strcpy(fileName, "FILE_FUTURE_KEY_REG");
            break;

        // --- DUKPT MAC Key ---
        case ADDR_MAC_KEY_REG:  //maybe this should be removed
            strcpy(fileName, "FILE_MAC_KEY_REG");
            break;

        // --- AES DUKPT ---
        case ADDR_AES_DUKPT_KEY_TYPE:
            strcpy(fileName, "FILE_AES_DUKPT_KEY_TYPE");
            break;

        case ADDR_INT_DERIVATION_KEY_REG:
            strcpy(fileName, "FILE_INT_DERIVATION_KEY_REG");
            break;

        case ADDR_INT_DERIVATION_KEY_IN_USE:
            strcpy(fileName, "FILE_INT_DERIVATION_KEY_IN_USE");
            break;

        case ADDR_CUR_DERIVATION_KEY:
            strcpy(fileName, "FILE_CUR_DERIVATION_KEY");
            break;
        
        case ADDR_TRANSACTION_COUNTER:
            strcpy(fileName, "FILE_TRANSACTION_COUNTER");
            break;

        // --- Master-Session Key ---
        case ADDR_PED_MKEY_INDEX:   //to be removed
            strcpy(fileName, "FILE_PED_MKEY_INDEX");
            break;

        case ADDR_PED_SKEY_INDEX:   //to be removed
            strcpy(fileName, "FILE_PED_SKEY_INDEX");
            break;
        
        // case ADDR_PED_MKEY:   //to be removed
        case ADDR_PED_MKEY_01:   //to be removed
            strcpy(fileName, "FILE_PED_MKEY");
            break;

        case ADDR_PED_SKEY_01:   //to be removed
            strcpy(fileName, "FILE_PED_SKEY_01");
            break;
        
        case ADDR_PED_SKEY_02:   //to be removed
            strcpy(fileName, "FILE_PED_SKEY_02");
            break;
        
        case ADDR_PED_SKEY_03:   //to be removed
            strcpy(fileName, "FILE_PED_SKEY_03");
            break;
        
        case ADDR_PED_SKEY_04:   //to be removed
            strcpy(fileName, "FILE_PED_SKEY_04");
            break;
        
        case ADDR_PED_SKEY_05:   //to be removed
            strcpy(fileName, "FILE_PED_SKEY_05");
            break;

        case ADDR_PED_SKEY_06:   //to be removed
            strcpy(fileName, "FILE_PED_SKEY_06");
            break;
        
        case ADDR_PED_SKEY_07:   //to be removed
            strcpy(fileName, "FILE_PED_SKEY_07");
            break;
        
        case ADDR_PED_SKEY_08:   //to be removed
            strcpy(fileName, "FILE_PED_SKEY_08");
            break;

        case ADDR_PED_SKEY_09:   //to be removed
            strcpy(fileName, "FILE_PED_SKEY_09");
            break;
        
        case ADDR_PED_SKEY_10:   //to be removed
            strcpy(fileName, "FILE_PED_SKEY_10");
            break;

        // --- PIN Encrption Key (PEK) ---
        case ADDR_PED_PEK_KEY_TYPE:
            strcpy(fileName, "FILE_PED_PEK_KEY_TYPE");
            break;
        
        case ADDR_PED_PEK_MKEY_INDEX:
            strcpy(fileName, "FILE_PED_PEK_MKEY_INDEX");
            break;
        
        case ADDR_PED_PEK_SKEY_INDEX:
            strcpy(fileName, "FILE_PED_PEK_SKEY_INDEX");
            break;
        
        // case ADDR_PED_PEK_TDES_MKEY:
        case ADDR_PED_PEK_TDES_MKEY_01:
            strcpy(fileName, "FILE_PED_PEK_TDES_MKEY");
            break;
        
        case ADDR_PED_PEK_TDES_SKEY_01:
            strcpy(fileName, "FILE_PED_PEK_TDES_SKEY_01");
            break;
        
        case ADDR_PED_PEK_TDES_SKEY_02:
            strcpy(fileName, "FILE_PED_PEK_TDES_SKEY_02");
            break;
        
        case ADDR_PED_PEK_TDES_SKEY_03:
            strcpy(fileName, "FILE_PED_PEK_TDES_SKEY_03");
            break;

        case ADDR_PED_PEK_TDES_SKEY_04:
            strcpy(fileName, "FILE_PED_PEK_TDES_SKEY_04");
            break;
        
        case ADDR_PED_PEK_TDES_SKEY_05:
            strcpy(fileName, "FILE_PED_PEK_TDES_SKEY_05");
            break;

        case ADDR_PED_PEK_TDES_SKEY_06:
            strcpy(fileName, "FILE_PED_PEK_TDES_SKEY_06");
            break;
        
        case ADDR_PED_PEK_TDES_SKEY_07:
            strcpy(fileName, "FILE_PED_PEK_TDES_SKEY_07");
            break;

        case ADDR_PED_PEK_TDES_SKEY_08:
            strcpy(fileName, "FILE_PED_PEK_TDES_SKEY_08");
            break;
        
        case ADDR_PED_PEK_TDES_SKEY_09:
            strcpy(fileName, "FILE_PED_PEK_TDES_SKEY_09");
            break;

        case ADDR_PED_PEK_TDES_SKEY_10:
            strcpy(fileName, "FILE_PED_PEK_TDES_SKEY_10");
            break;

        // case ADDR_PED_PEK_AES_MKEY:
        case ADDR_PED_PEK_AES_MKEY_01:
            strcpy(fileName, "FILE_PED_PEK_AES_MKEY");
            break;

        case ADDR_PED_PEK_AES_SKEY_01:
            strcpy(fileName, "FILE_PED_PEK_AES_SKEY_01");
            break;
        
        case ADDR_PED_PEK_AES_SKEY_02:
            strcpy(fileName, "FILE_PED_PEK_AES_SKEY_02");
            break;
        
        case ADDR_PED_PEK_AES_SKEY_03:
            strcpy(fileName, "FILE_PED_PEK_AES_SKEY_03");
            break;
        
        case ADDR_PED_PEK_AES_SKEY_04:
            strcpy(fileName, "FILE_PED_PEK_AES_SKEY_04");
            break;
        
        case ADDR_PED_PEK_AES_SKEY_05:
            strcpy(fileName, "FILE_PED_PEK_AES_SKEY_05");
            break;

        case ADDR_PED_PEK_AES_SKEY_06:
            strcpy(fileName, "FILE_PED_PEK_AES_SKEY_06");
            break;

        case ADDR_PED_PEK_AES_SKEY_07:
            strcpy(fileName, "FILE_PED_PEK_AES_SKEY_07");
            break;
        
        case ADDR_PED_PEK_AES_SKEY_08:
            strcpy(fileName, "FILE_PED_PEK_AES_SKEY_08");
            break;

        case ADDR_PED_PEK_AES_SKEY_09:
            strcpy(fileName, "FILE_PED_PEK_AES_SKEY_09");
            break;

        case ADDR_PED_PEK_AES_SKEY_10:
            strcpy(fileName, "FILE_PED_PEK_AES_SKEY_10");
            break;
        
        // --- PIN Data ---
        case ADDR_PED_PIN:
            strcpy(fileName, "FILE_PED_PIN");
            break;
        
        // --- Password Management ---
        // case ADDR_PED_ADMIN_PSW:    //to be removed
        //     fileName = "FILE_PED_ADMIN_PSW";
        //     break;
        
        case ADDR_PED_ADMIN_PSW1:
            strcpy(fileName, "FILE_PED_ADMIN_PSW1");
            break;
        
        case ADDR_PED_ADMIN_PSW2:
            strcpy(fileName, "FILE_PED_ADMIN_PSW2");
            break;

        case ADDR_PED_USER_PSW: //to be removed
            strcpy(fileName, "FILE_PED_USER_PSW");
            break;
        
        case ADDR_PED_DSS_PSW1:
            strcpy(fileName, "FILE_PED_DSS_PSW1");
            break;
        
        case ADDR_PED_DSS_PSW2:
            strcpy(fileName, "FILE_PED_DSS_PSW2");
            break;
        
        case ADDR_PED_SRED_PSW1:    //maybe this should be removed
            strcpy(fileName, "FILE_PED_SRED_PSW1");
            break;
        
        case ADDR_PED_SRED_PSW2:    //maybe this should be removed
            strcpy(fileName, "FILE_PED_SRED_PSW2");
            break;
        
        // --- Terminal Parameters ---
        case ADDR_PED_TERM_ID:
            strcpy(fileName, "FILE_PED_TERM_ID");
            break;
        
        // --- Account Data-Encryption Key (ACC_DEK) ---
        case ADDR_PED_AccDEK_KEY_TYPE:
            strcpy(fileName, "FILE_PED_AccDEK_KEY_TYPE");
            break;
        
        case ADDR_PED_ACC_DEK_MKEY_INDEX:   //to be removed
            strcpy(fileName, "FILE_PED_ACC_DEK_MKEY_INDEX");
            break;

        case ADDR_PED_ACC_DEK_SKEY_INDEX:   //to be removed
            strcpy(fileName, "FILE_PED_ACC_DEK_SKEY_INDEX");
            break;
        
        // case ADDR_PED_ACC_DEK_MKEY: //to be removed
        case ADDR_PED_ACC_DEK_MKEY_01:  //to be removed
            strcpy(fileName, "FILE_PED_ACC_DEK_MKEY");
            break;
        
        case ADDR_PED_ACC_DEK_SKEY_01:  //to be removed
            strcpy(fileName, "FILE_PED_ACC_DEK_SKEY_01");
            break;

        // case ADDR_PED_ACC_DEK_TDES_MKEY:
        case ADDR_PED_ACC_DEK_TDES_MKEY_01:
            strcpy(fileName, "FILE_PED_ACC_DEK_TDES_MKEY");
            break;
        
        case ADDR_PED_ACC_DEK_TDES_SKEY_01:
            strcpy(fileName, "FILE_PED_ACC_DEK_TDES_SKEY_01");
            break;
        
        // case ADDR_PED_ACC_DEK_AES_MKEY:
        case ADDR_PED_ACC_DEK_AES_MKEY_01:
            strcpy(fileName, "FILE_PED_ACC_DEK_AES_MKEY");
            break;
        
        case ADDR_PED_ACC_DEK_AES_SKEY_01:
            strcpy(fileName, "FILE_PED_ACC_DEK_AES_SKEY_01");
            break;
        
        case ADDR_PED_ACC_DEK_AES_DUKPT_WORKING_KEY:
            strcpy(fileName, "FILE_PED_ACC_DEK_AES_DUKPT_WORKING_KEY");
            break;
        
        case ADDR_PED_ACC_DEK_CURRENT_KSN:
            strcpy(fileName, "FILE_PED_ACC_DEK_CURRENT_KSN");
            break;
        
        // --- Salt ---
        case ADDR_PED_SALT:
            strcpy(fileName, "FILE_PED_SALT");
            break;
        
        // --- PAN Data ---
        case ADDR_PED_PAN_DATA:
            strcpy(fileName, "FILE_PED_PAN_DATA");
            break;

        // --- Account Data ---
        // case ADDR_PED_ACC_DATA: //to be removed
        case ADDR_TAG57_L:
            strcpy(fileName, "FILE_TAG57_L");
            break;

        case ADDR_TAG57_V:
            strcpy(fileName, "FILE_TAG57_V");
            break;

        case ADDR_TAG5A_L:
            strcpy(fileName, "FILE_TAG5A_L");
            break;
        
        case ADDR_TAG5A_V:
            strcpy(fileName, "FILE_TAG5A_V");
            break;
        
        case ADDR_TAG9F1F_L:
            strcpy(fileName, "FILE_TAG9F1F_L");
            break;
        
        case ADDR_TAG9F1F_V:
            strcpy(fileName, "FILE_TAG9F1F_V");
            break;
        
        case ADDR_TAG9F20_L:
            strcpy(fileName, "FILE_TAG9F20_L");
            break;

        case ADDR_TAG9F20_V:
            strcpy(fileName, "FILE_TAG9F20_V");
            break;

        //--- 128 bits AES KEY for ISO9564-1:2017 format 4 (ISO4_KEY) ---
        // case ADDR_PED_ISO4_KEY: //to be removed
        //     strcpy(fileName, "FILE_PED_ISO4_KEY");
        //     break;
        
        // --- Format-Preserving Encryption Key (FPE Key) ---
        case ADDR_PED_FPE_KEY_TYPE:
            strcpy(fileName, "FILE_PED_FPE_KEY_TYPE");
            break;
        
        case ADDR_PED_FPE_KEY_MKEY_INDEX:
            strcpy(fileName, "FILE_PED_FPE_KEY_MKEY_INDEX");
            break;
        
        case ADDR_PED_FPE_KEY_SKEY_INDEX:
            strcpy(fileName, "FILE_PED_FPE_KEY_SKEY_INDEX");
            break;
        
        // case ADDR_PED_FPE_KEY_AES_MKEY:
        case ADDR_PED_FPE_KEY_AES_MKEY_01:
            strcpy(fileName, "FILE_PED_FPE_KEY_AES_MKEY");
            break;
        
        case ADDR_PED_FPE_KEY_AES_SKEY_01:
            strcpy(fileName, "FILE_PED_FPE_KEY_AES_SKEY_01");
            break;
        
        // case ADDR_PED_FPE_KEY:  //to be removed
        //     strcpy(fileName, "FILE_PED_FPE_KEY");
        //     break;

        case ADDR_PED_FPE_KEY_AES_DUKPT_WORKING_KEY:
            strcpy(fileName, "FILE_PED_FPE_KEY_AES_DUKPT_WORKING_KEY");
            break;
        
        case ADDR_PED_FPE_KEY_CURRENT_KSN:
            strcpy(fileName, "FILE_PED_FPE_KEY_CURRENT_KSN");
            break;
        
        // --- CAPK ---
        case ADDR_PED_CAPK:
            strcpy(fileName, "FILE_PED_CAPK");
            break;
        
        // --- Secret Information ---
        case ADDR_EDC_PRV_KEY:
            strcpy(fileName, "FILE_EDC_PRV_KEY");
            break;

        case ADDR_DEVICE_AUTH_STATUS:
            strcpy(fileName, "FILE_DEVICE_AUTH_STATUS");
            break;
        
        case ADDR_DEVICE_AUTH_DATE_TIME:
            strcpy(fileName, "FILE_DEVICE_AUTH_DATE_TIME");
            break;
        
        // --- Open Protocol ---
        case ADDR_CA_CERTIFICATE:
            strcpy(fileName, "FILE_CA_CERTIFICATE");
            break;

        case ADDR_CLIENT_CERTIFICATE:
            strcpy(fileName, "FILE_CLIENT_CERTIFICATE");
            break;

        case ADDR_CLIENT_PRV_KEY:
            strcpy(fileName, "FILE_CLIENT_PRV_KEY");
            break;

        default:
            fileName = NULL;
            break;
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: To create and initialize secure files.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    init_secure_file()
{
    UINT8               res = TRUE;
    struct stat         stbuf;
    UINT8               i;
    UINT8               cnt;
    UINT8               *data;
    UINT8               *blob;
    struct caam_kb_data kb_data;
    int                 fd;
    int                 result;
    FILE                *fp;
    char                *fileName = NULL;
    char                *secureDirPath = "/home/root/secure_directory/";


    if(stat("/home/root/secure_directory", &stbuf) == 0)
    {
        if(stbuf.st_mode & S_IFDIR)
            return;
    }

    //create secure directory
    if(mkdir("/home/root/secure_directory", S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) != 0)
    {
        printf("Failed to create secure directory\n");
        return;
    }

    fd = open("/dev/caam_kb", O_RDWR);
    if(fd < 0)
    {
        printf("Failed to open driver\n");
        return;
    }

    cnt = (UINT8)(sizeof(fileInfo) / sizeof(fileInfo[0]));
    // printf("cnt = %d\n", cnt);

    //initialize each file in secure directory
    for(i = 0 ; i < cnt ; i++)
    {
        data = calloc(fileInfo[i].fileSize, sizeof(UINT8));
        if(data == NULL)
        {
            res = FALSE;
            printf("Failed to allocate data\n");
            return;
        }

        memset(data, 0x00, fileInfo[i].fileSize);

        blob = calloc(fileInfo[i].fileSize + BLOB_OVERHEAD, sizeof(UINT8));
        if(blob == NULL)
        {
            printf("Failed to allocate blob\n");
            goto free_data;
        }

        kb_data.rawkey = data;
        kb_data.rawkey_len = fileInfo[i].fileSize;
        kb_data.keymod_len = 16;
        kb_data.keymod = skeymod;
        kb_data.keyblob = blob;
        kb_data.keyblob_len = fileInfo[i].fileSize + BLOB_OVERHEAD;

        result = ioctl(fd, CAAM_KB_ENCRYPT, &kb_data);
        if(result < 0)
        {
            printf("Failed to encrypt\n");
            goto close_driver;
        }

        fileName = malloc(strlen(secureDirPath) + strlen(fileInfo[i].fileName) + 1);    //add 1 for null terminator
        if(fileName == NULL)
        {
            res = FALSE;
            printf("Failed to allocate memory for string\n");
            goto close_driver;
        }
        
        strcpy(fileName, secureDirPath);
        strcat(fileName, fileInfo[i].fileName);
        // printf("%s\n", fileName);

        //write ciphertext to the file
        fp = fopen(fileName, "w");
        if(fp == NULL)
        {
            res = FALSE;
            printf("Failed to open secure file\n");
            goto free_fileStr;
        }

        if(fwrite(blob, 1, kb_data.keyblob_len, fp) < 0)
        {
            res = FALSE;
            printf("Failed to write blob\n");
            goto close_file;
        }
        
        fflush(fp);
        fsync(fp);
        fclose(fp);
        free(fileName);
        free(blob);
        free(data);
    }

    if(res)
    {
        close(fd);
        return;
    }

close_file:
    fclose(fp);

free_fileStr:
    free(fileName);

close_driver:
    close(fd);

free_blob:
    memset(blob, 0x00, SEC_MEMORY_SIZE + BLOB_OVERHEAD);
    free(blob);

free_data:
    memset(data, 0x00, SEC_MEMORY_SIZE);
    free(data);
}

// ---------------------------------------------------------------------------
// FUNCTION: To load and decrypt the secure file to memory.
// INPUT   : fileName - name of the corresponding secure file.
// OUTPUT  : none.
// RETURN  : pointer to decrypted secure file.
//           NULL if failed to load.
// ---------------------------------------------------------------------------
UINT8*  load_secure_file(char *fileName)
{
    FILE                *fp;
    UINT8               cnt;
    UINT8               i;
    UINT16              fileSize = 0;
    UINT8               *data;
    UINT8               *blob;
    int                 blob_size;
    int                 fd;
    struct caam_kb_data kb_data;
    int                 result;
    char                *filePath = NULL;
    char                *secureDirPath = "/home/root/secure_directory/";
    char                *command_rmsecure_directory = "rm -r /home/root/secure_directory";
    char                *command_killAP = "killall IPC_client";
    char                *command_rmAP = "rm /home/AP/IPC_client";


    filePath = malloc(strlen(secureDirPath) + strlen(fileName) + 1); //add 1 for null terminator
    if(filePath == NULL)
    {
        printf("Failed to allocate memory for string\n");
        return;
    }

    strcpy(filePath, secureDirPath);
    strcat(filePath, fileName);
    // printf("%s\n", filePath);

    //load encrypted file from file system
    fp = fopen(filePath, "r");
    if(fp == NULL)
    {
        printf("Failed to open secure file\n");
        goto free_mem;
    }

    cnt = (UINT8)(sizeof(fileInfo) / sizeof(fileInfo[0]));
    // printf("cnt = %d\n", cnt);

    for(i = 0 ; i < cnt ; i++)
    {
        if(strcmp(fileName, fileInfo[i].fileName) == 0)
        {
            fileSize = fileInfo[i].fileSize;
            break;
        }
    }
    
    data = malloc(fileSize);
    if(data == NULL)
    {
        printf("Failed to allocate data\n");

        goto free_data;
    }

    blob = malloc(fileSize + BLOB_OVERHEAD);
    if(blob == NULL)
    {
        printf("Failed to allocate blob\n");

        goto free_blob;
    }

    blob_size = fread(blob, 1, fileSize + BLOB_OVERHEAD, fp);
    if(blob_size < 0)
    {
        printf("Failed to read blob\n");
        goto close_file;
    }

    //decrypt the encrypted file
    fd = open("/dev/caam_kb", O_RDWR);
    if(fd < 0)
    {
        printf("Failed to open driver\n");
        goto close_file;
    }
    
    kb_data.rawkey = data;
    kb_data.rawkey_len = blob_size - BLOB_OVERHEAD;
    kb_data.keymod_len = 16;
    kb_data.keymod = skeymod;
    kb_data.keyblob = blob;
    kb_data.keyblob_len = blob_size;

    result = ioctl(fd, CAAM_KB_DECRYPT, &kb_data);
    if(result < 0)
    {
        printf("Failed to decrypt\n");
        goto close_driver;
    }

    close(fd);
    fclose(fp);
    memset(blob, 0x00, fileSize + BLOB_OVERHEAD);
    free(blob);
    free(filePath);
    return data;

close_driver:
    close(fd);

    //remove secure_directory when CAAM fails to decrypt data
    fp = popen(command_rmsecure_directory, "r");
    if(!fp)
        perror("Failed to remove secure_directory");
    else
    {
        //wait 500 ms to complete the deletion of secure_region
        LIB_WaitTime(50);
        init_secure_file();
    }

    pclose(fp);

    fp = popen(command_killAP, "r");
    if(!fp)
        perror("Failed to kill IPC_client");

    pclose(fp);

    fp = popen(command_rmAP, "r");
    if(!fp)
        perror("Failed to remove IPC_client");

    pclose(fp);

close_file:
    fclose(fp);

free_blob:
    memset(blob, 0x00, fileSize + BLOB_OVERHEAD);
    free(blob);

free_data:
    memset(data, 0x00, fileSize);
    free(data);

free_mem:
    free(filePath);

    return NULL;
}

// ---------------------------------------------------------------------------
// FUNCTION: To update the secure file.
// INPUT   : fileName - name of the corresponding secure file.
//           dataPtr  - pointer return by load_secure_file().
// OUTPUT  : none.
// RETURN  : TRUE     - update successful.
//           FALSE    - update failed.
// ---------------------------------------------------------------------------
UINT8   update_secure_file(char *fileName, UINT8 *dataPtr)
{
    int                 fd;
    UINT8               cnt;
    UINT8               i;
    UINT16              fileSize = 0;
    UINT8               *blob;
    struct caam_kb_data kb_data;
    int                 result;
    FILE                *fp;
    char                *filePath = NULL;
    char                *secureDirPath = "/home/root/secure_directory/";


    //encrypt data using caam_kb driver
    fd = open("/dev/caam_kb", O_RDWR);
    if(fd < 0)
    {
        printf("Failed to open driver\n");
        return FALSE;
    }

    cnt = sizeof(fileInfo) / sizeof(fileInfo[0]);
    // printf("cnt = %d\n", cnt);

    for(i = 0 ; i < cnt ; i++)
    {
        if(strcmp(fileName, fileInfo[i].fileName) == 0)
        {
            fileSize = fileInfo[i].fileSize;
            break;
        }
    }

    blob = calloc(fileSize + BLOB_OVERHEAD, sizeof(UINT8));
    if(blob == NULL)
    {
        printf("Failed to allocate blob\n");
        goto close_driver;
    }

    kb_data.rawkey = dataPtr;
    kb_data.rawkey_len = fileSize;
    kb_data.keymod_len = 16;
    kb_data.keymod = skeymod;
    kb_data.keyblob = blob;
    kb_data.keyblob_len = fileSize + BLOB_OVERHEAD;

    result = ioctl(fd, CAAM_KB_ENCRYPT, &kb_data);
    if(result < 0)
    {
        printf("Failed to encrypt\n");
        goto free_mem;
    }

    filePath = malloc(strlen(secureDirPath) + strlen(fileName) + 1); //add 1 for null terminator
    if(filePath == NULL)
    {
        printf("Failed to allocate memory for string\n");
        return;
    }

    strcpy(filePath, secureDirPath);
    strcat(filePath, fileName);
    // printf("%s\n", filePath);

    fp = fopen(filePath, "w");
    if(fp == NULL)
    {
        printf("Failed to open secure file\n");
        goto free_mem;
    }

    //store the ciphertext to the secure file
    if(fwrite(blob, 1, kb_data.keyblob_len, fp) < 0)
    {
        printf("Failed to write blob\n");
        goto close_file;
    }

    printf("update success\n");
    fclose(fp);
    memset(blob, 0x00, fileSize + BLOB_OVERHEAD);
    free(blob);
    memset(dataPtr, 0x00, fileSize);
    free(dataPtr);
    close(fd);
    sync();
    return TRUE;

close_file:
    fclose(fp);

free_mem:
    memset(blob, 0x00, fileSize + BLOB_OVERHEAD);
    free(blob);

close_driver:
    close(fd);
    return FALSE;
}

/**
 * 	this function is used to initialize the secure_region file if not exist
 */ 
void init_secure_memory(){

	FILE *key_save_file;
	int fd, result; 
	UINT8 *data, *blob;
	struct caam_kb_data kb_data;
	/**
	 * 	first check the file exist or not
	 */
    key_save_file= fopen("/home/root/secure_region", "r");
    if(key_save_file != NULL){
		fclose(key_save_file);
        return;
    }
	/**
	 * 	second initial all the value as 0 then encrypt
	 */ 
	data = calloc(SEC_MEMORY_SIZE, sizeof(UINT8));
	if(data == NULL){
		printf("locate data failed");
		return;
	}
	//add for administrator default password
	*(data+ ADDR_PED_ADMIN_PSW)=7;//default administrator password is 0000000, length 7
	memset(data+ ADDR_PED_ADMIN_PSW+1,48,PED_PSW_SLOT_LEN);
	*(data+ ADDR_PED_USER_PSW)=7;//perhaps default user password is 0000000, length 7 ?
	memset(data+ ADDR_PED_USER_PSW +1,48,PED_PSW_SLOT_LEN);
   	blob = calloc(SEC_MEMORY_SIZE + BLOB_OVERHEAD, sizeof(UINT8));

    if(blob == NULL){
        printf("locate blob failed");
		goto free_data;
    }

    kb_data.rawkey = data;
    kb_data.rawkey_len = SEC_MEMORY_SIZE;
    kb_data.keymod_len = 16;
    kb_data.keymod = skeymod;
    kb_data.keyblob = blob;
    kb_data.keyblob_len = SEC_MEMORY_SIZE + BLOB_OVERHEAD;

    fd = open("/dev/caam_kb", O_RDWR);
	if(fd < 0){
		printf("driver open failed\n");
		goto free_blob;
	}

    result = ioctl(fd, CAAM_KB_ENCRYPT, &kb_data);

    if(result < 0){
        printf("encrypt failed\n");
		goto close_driver;

    }
	/**
	 * 
	 * third write the encryt result to file
	 */ 
    key_save_file = fopen("/home/root/secure_region","w");

    if(key_save_file == NULL){
        printf("open failed\n");
		goto close_driver;
    }

    if( fwrite(blob, 1, kb_data.keyblob_len, key_save_file) < 0){
        printf("write failed\n");
		goto close_file;
    }

    printf("init_secure_memory write success\n");
    sync();
close_file:
	fclose(key_save_file);
close_driver:
	close(fd);
free_blob:
    memset(blob, 0x00, SEC_MEMORY_SIZE + BLOB_OVERHEAD);    //Added by Tammy
	free(blob);
free_data:
    memset(data, 0x00, SEC_MEMORY_SIZE); //Added by Tammy
	free(data);

}

/**
 * 	this function is used to load and decrypt the secure_region file to memory
 *  @return pointer to decrypt secure_region file
 *          NULL if failed to load
 *  @note if you call update_security_memory to the return pointer, the pointer will be free automately
 *        or you need to free by yourself
 */ 
UINT8* load_security_memory(){

	FILE *key_save_file;
	int result,fd,blob_size;
	struct caam_kb_data kb_data;
	UINT8 *data, *blob;
    FILE *fp;
    char *command_rmsecure_region = "rm /home/root/secure_region";
    char *command_killAP = "killall IPC_client";
    char *command_rmAP = "rm /home/AP/IPC_client";

	/**
	 * 	first do load encrypt file from file system
	 */
    key_save_file= fopen("/home/root/secure_region", "r");
	if(key_save_file == NULL)
		return NULL;

	data = calloc(SEC_MEMORY_SIZE, sizeof(UINT8));
   	blob = calloc(SEC_MEMORY_SIZE + BLOB_OVERHEAD, sizeof(UINT8));

	if(data == NULL || blob == NULL){
        printf("locate failed");
		goto free_mem;
    }

	blob_size = fread(blob,1, SEC_MEMORY_SIZE + BLOB_OVERHEAD, key_save_file);
	if(blob_size < 0){
        printf("read failed\n");
		goto free_mem;
    }
	/**
	 *  second call caam_kb driver to decode the encrypt file
	 */ 
	fd = open("/dev/caam_kb", O_RDWR);

	if(fd < 0)
		goto free_mem;

	kb_data.rawkey = data;
    kb_data.rawkey_len = blob_size - BLOB_OVERHEAD;
    kb_data.keymod_len = 16;
    kb_data.keymod = skeymod;
    kb_data.keyblob = blob;
    kb_data.keyblob_len = blob_size;

	result = ioctl(fd, CAAM_KB_DECRYPT, &kb_data);

	if(result < 0){
		printf("Decrypt failed\n");
		goto close_driver;
	}

	fclose(key_save_file);
	close(fd);
    memset(blob, 0x00, SEC_MEMORY_SIZE + BLOB_OVERHEAD);    //Added by Tammy
	free(blob);
	return data;

close_driver:
	close(fd);

    //Added by Tammy, remove secure_region when CAAM fails to decrypt data
    fp = popen(command_rmsecure_region, "r");
    if(!fp)
        perror("Failed to remove secure_region");
    else
    {
        //wait 200 ms to complete the deletion of secure_region
        LIB_WaitTime(20);
        init_secure_memory();
    }

    pclose(fp);

    fp = popen(command_killAP, "r");
    if(!fp)
        perror("Failed to kill IPC_client");

    pclose(fp);

    fp = popen(command_rmAP, "r");
    if(!fp)
        perror("Failed to remove IPC_client");

    pclose(fp);
    
free_mem:
    memset(data, 0x00, SEC_MEMORY_SIZE); //Added by Tammy
	free(data);
    memset(blob, 0x00, SEC_MEMORY_SIZE + BLOB_OVERHEAD);    //Added by Tammy
	free(blob);
	fclose(key_save_file);
	return NULL;
}

/**
 * 	 this function is used to update the secure_region file
 *   @param[in] ptr_sm		pointer return by load_security_memory
 *   @return error code
 *   @retval TRUE			update success
 *   @retval FALSE          update failed
 *   @note if this function return TRUE, the ptr_sm is also be free
 *         if this function return FALSE, the ptr_sm is not be free
 */  
UINT8 update_security_memory(UINT8* ptr_sm){

	int fd, result;
	UINT8* blob;
	struct caam_kb_data kb_data;
	FILE *key_save_file;
    FILE *fp;
    char *command_rmsecure_region = "rm /home/root/secure_region";

	/**
	 * 	first encrypt the memory region using caam_kb driver
	 */ 
	fd = open("/dev/caam_kb", O_RDWR);
	if(fd < 0){
		printf("open driver failed\n");
		return FALSE;
	}

	blob = calloc(SEC_MEMORY_SIZE + BLOB_OVERHEAD, sizeof(UINT8));

	if(blob == NULL){
		printf("locate failed\n");
		goto close_driver;
	}


	kb_data.rawkey = ptr_sm;
    kb_data.rawkey_len = SEC_MEMORY_SIZE;
    kb_data.keymod_len = 16;
    kb_data.keymod = skeymod;
    kb_data.keyblob = blob;
    kb_data.keyblob_len = SEC_MEMORY_SIZE + BLOB_OVERHEAD;

	result = ioctl(fd, CAAM_KB_ENCRYPT, &kb_data);

	if(result < 0){
		printf("Encrypt failed\n");
		goto free_mem;
	}
	/**
	 * 	second save the encrypt result to file system
	 */ 
	key_save_file = fopen("/home/root/secure_region", "w");

	if(key_save_file == NULL){
        printf("open file failed\n");
		goto free_mem;

    }

	if( fwrite(blob, 1, kb_data.keyblob_len, key_save_file) < 0){
        printf("write failed\n");
		goto close_file;
    }

	printf("update success\n");
	// remember to free the input
	fclose(key_save_file);
    memset(blob, 0x00, SEC_MEMORY_SIZE + BLOB_OVERHEAD);    //Added by Tammy
	free(blob);
    memset(ptr_sm, 0x00, SEC_MEMORY_SIZE);  //Added by Tammy
	free(ptr_sm);
	close(fd);
	// remember to sync
	sync();
	return TRUE;

close_file:
	fclose(key_save_file);
free_mem:
    memset(blob, 0x00, SEC_MEMORY_SIZE + BLOB_OVERHEAD);    //Added by Tammy
	free(blob);
close_driver:
	close(fd);
	return FALSE;

}


// ---------------------------------------------------------------------------
// FUNCTION: Verify the boot status by checking "SYS_WARMBOOT_FLAG".
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  -- warm boot (flag ok  )
//           FALSE -- cold boot (flag lost)
// ---------------------------------------------------------------------------
UINT32	OS_SECM_VerifyBootStatus( void )
{
	return TRUE;
	// UINT32	*pSecMem = (UINT32 *)SEC_MEMORY_BASE;

	// if( *(pSecMem + ADDR_SYS_RESET_FLAG) == SYS_WARMBOOT_FLAG )
	//   return( TRUE );
	// else
	//   return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Set the boot status to "SYS_WARMBOOT_FLAG".
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  -- OK
//           FALSE -- Failed
// ---------------------------------------------------------------------------
UINT32	OS_SECM_SetWarmBootStatus( void )
{
	return TRUE;
	// UINT32	*pSecMem = (UINT32 *)SEC_MEMORY_BASE;

	// *(pSecMem + ADDR_SYS_RESET_FLAG) = SYS_WARMBOOT_FLAG;
	// return( OS_SECM_VerifyBootStatus() );
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear the boot status to NULL. (initial state)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  -- OK
//           FALSE -- Failed
// ---------------------------------------------------------------------------
UINT32	OS_SECM_ResetBootStatus( void )
{
	return TRUE;
	// UINT32	*pSecMem = (UINT32 *)SEC_MEMORY_BASE;

	// *(pSecMem + ADDR_SYS_RESET_FLAG) = SYS_COLDBOOT_FLAG;
	// if( OS_SECM_VerifyBootStatus() == FALSE )
	//   return( TRUE );
	// else
	//   return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Verify the application status by checking "APP_READY_FLAG".
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  -- ready.
//           FALSE -- not ready.
// ---------------------------------------------------------------------------
UINT32	OS_SECM_VerifyAppStatus( void )
{
	return TRUE;	
	// UINT8	*pSecMem = (UINT8 *)SEC_MEMORY_BASE;

	// if( *(pSecMem + ADDR_APP_CTRL_TBL) == APP_READY_FLAG )
	//   return( TRUE );
	// else
	//   return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Set the application status to "APP_READY_FLAG".
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  -- OK
//           FALSE -- Failed
// ---------------------------------------------------------------------------
UINT32	OS_SECM_SetAppStatus( void )
{
	return TRUE;
	// UINT8	*pSecMem = (UINT8 *)SEC_MEMORY_BASE;

	// *(pSecMem + ADDR_APP_CTRL_TBL) = APP_READY_FLAG;
	// return( OS_SECM_VerifyAppStatus() );
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear the application status to NULL. (initial state)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  -- OK
//           FALSE -- Failed
// ---------------------------------------------------------------------------
UINT32	OS_SECM_ResetAppStatus( void )
{
	return TRUE;
	// UINT8	*pSecMem = (UINT8 *)SEC_MEMORY_BASE;

	// *(pSecMem + ADDR_APP_CTRL_TBL) = APP_NOT_READY_FLAG;
	// if( OS_SECM_VerifyAppStatus() == FALSE )
	//   return( TRUE );
	// else
	//   return( FALSE );
}

// ---------------------------------------------------------------------------
// FUNCTION: Write data to the specified address.
// INPUT   : address - start address to write.
//	     length  - length of data to write.
//	     data    - data to be written.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_PutData( UINT32 address, UINT32 length, UINT8 *data )
{
#if 0
	UINT8   *pSecMem = load_security_memory();


	if(pSecMem != NULL){
		memmove( pSecMem+address, data, length );

		update_security_memory(pSecMem);
	}
#endif

    char    fileName[48];
    UINT8   *pSecMem;


    getSecureFileName(address, fileName);
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memmove(pSecMem, data, length);
        update_secure_file(fileName, pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Read data from the specified address.
// INPUT   : address - start address to read.
//	     length  - length of data to read.
// OUTPUT  : data    - storage of data read.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_GetData( UINT32	address, UINT32 length, UINT8 *data )
{
#if 0
	UINT8   *pSecMem = load_security_memory();


	if(pSecMem != NULL){
		memmove( data, pSecMem+address, length );
        memset(pSecMem, 0x00, SEC_MEMORY_SIZE); //Added by Tammy
		free(pSecMem);
	}
#endif

    char    fileName[48];
    UINT8   *pSecMem;
    UINT8   cnt;
    UINT8   i;
    UINT16  fileSize = 0;


    getSecureFileName(address, fileName);
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memmove(data, pSecMem, length);

        cnt = sizeof(fileInfo) / sizeof(fileInfo[0]);

        for(i = 0 ; i < cnt ; i++)
        {
            if(strcmp(fileName, fileInfo[i].fileName) == 0)
            {
                fileSize = fileInfo[i].fileSize;
                break;
            }
        }

        memset(pSecMem, 0x00, fileSize);
        free(pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear data to the specified pattern.
// INPUT   : address - start address to write.
//	     length  - length of data to write.
//	     pattern - data to be written.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_ClearData( UINT32 address, UINT32 length, UINT8 pattern )
{
#if 0
	UINT8   *pSecMem = load_security_memory();


	if(pSecMem != NULL){
		memset( pSecMem+address, pattern, length );
		update_security_memory(pSecMem);
	}
#endif

    char    fileName[48];
    UINT8   *pSecMem;


    getSecureFileName(address, fileName);
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memset(pSecMem, pattern, length);
        update_secure_file(fileName, pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear the data of each secure file.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    OS_SECM_ClearAllSecureFile(void)
{
    UINT8   i;
    UINT8   cnt;
    char    *fileName;
    UINT8   *pSecMem;


    cnt = sizeof(fileInfo) / sizeof(fileInfo[0]);

    for(i = 0 ; i < cnt ; i++)
    {
        pSecMem = load_secure_file(fileInfo[i].fileName);

        if(pSecMem != NULL)
        {
            memset(pSecMem, 0x00, fileInfo[i].fileSize);
            update_secure_file(fileInfo[i].fileName, pSecMem);
        }
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Write data to the address of (ADDR_INT_DERIVATION_KEY_REG + offset).
// INPUT   : address - start address to write.
//	         length  - length of data to write.
//	         data    - data to be written.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    OS_SECM_PutIntDerivationKeyReg(UINT32 address, UINT32 length, UINT8 *data)
{
    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_INT_DERIVATION_KEY_REG");
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memmove(pSecMem + address, data, length);
        update_secure_file(fileName, pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Read data from the address of (ADDR_INT_DERIVATION_KEY_REG + offset).
// INPUT   : address - start address to read.
//	         length  - length of data to read.
// OUTPUT  : data    - storage of data read.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_GetIntDerivationKeyReg(UINT32 address, UINT32 length, UINT8 *data)
{
    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_INT_DERIVATION_KEY_REG");
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memmove(data, pSecMem + address, length);
        memset(pSecMem, 0x00, MAX_NUM_REG*MAX_KEYLENGTH);
        free(pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear data to the address of (ADDR_INT_DERIVATION_KEY_REG + offset).
// INPUT   : address - start address to write.
//	         length  - length of data to write.
//	         pattern - data to be written.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_ClearIntDerivationKeyReg(UINT32 address, UINT32 length, UINT8 pattern)
{
    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_INT_DERIVATION_KEY_REG");
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memset(pSecMem + address, pattern, length);
        update_secure_file(fileName, pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Write data to the address of (ADDR_INT_DERIVATION_KEY_IN_USE + offset).
// INPUT   : address - start address to write.
//	         length  - length of data to write.
//	         data    - data to be written.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    OS_SECM_PutIntDerivationKeyInUse(UINT32 address, UINT32 length, UINT8 *data)
{
    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_INT_DERIVATION_KEY_IN_USE");
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memmove(pSecMem + address, data, length);
        update_secure_file(fileName, pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Read data from the address of (ADDR_INT_DERIVATION_KEY_IN_USE + offset).
// INPUT   : address - start address to read.
//	         length  - length of data to read.
// OUTPUT  : data    - storage of data read.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_GetIntDerivationKeyInUse(UINT32 address, UINT32 length, UINT8 *data)
{
    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_INT_DERIVATION_KEY_IN_USE");
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memmove(data, pSecMem + address, length);
        memset(pSecMem, 0x00, MAX_NUM_REG);
        free(pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear data to the address of (ADDR_INT_DERIVATION_KEY_IN_USE + offset).
// INPUT   : address - start address to write.
//	         length  - length of data to write.
//	         pattern - data to be written.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_ClearIntDerivationKeyInUse(UINT32 address, UINT32 length, UINT8 pattern)
{
    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_INT_DERIVATION_KEY_IN_USE");
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memset(pSecMem + address, pattern, length);
        update_secure_file(fileName, pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Read data from the specified KEY address.
// INPUT   : address - start address to read.
//	         length  - length of data to read.
// OUTPUT  : data    - storage of data read.
// RETURN  : TRUE/FALSE
// ---------------------------------------------------------------------------
void	OS_SECM_GetKeyData(UINT32 address, UINT32 length, UINT8 *data)
{
#if 0
	OS_SECM_GetData(ADDR_PED_CAPK + address, length, data);
#endif

    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_PED_CAPK");
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memmove(data, pSecMem + address, length);
        memset(pSecMem, 0x00, CAPK_KEY_SLOT_LEN*MAX_CAPK_CNT);
        free(pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Clear KEY data to the specified pattern.
// INPUT   : address - start address to write.
//	         length  - length of data to write.
//	         pattern - data to be written.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_ClearKeyData(UINT32 address, UINT32 length, UINT8 pattern)
{
#if 0
	OS_SECM_ClearData(ADDR_PED_CAPK + address, length, pattern);
#endif

    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_PED_CAPK");
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memset(pSecMem + address, pattern, length);
        update_secure_file(fileName, pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Write data to the specified KEY address.
// INPUT   : address - start address to write.
//	         length  - length of data to write.
//	         data    - data to be written.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_PutKeyData(UINT32 address, UINT32 length, UINT8 *data)
{
#if 0
	OS_SECM_PutData(ADDR_PED_CAPK + address, length, data);
#endif

    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_PED_CAPK");

    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memmove(pSecMem + address, data, length);
        update_secure_file(fileName, pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Write data to the specified EDC private key address.
// INPUT   : address - start address to write.
//	         length  - length of data to write.
//	         data    - data to be written.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_PutEdcPrvKey(UINT32 address, UINT32 length, UINT8 *data)
{
    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_EDC_PRV_KEY");

    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memmove(pSecMem + address, data, length);
        update_secure_file(fileName, pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Read data from the specified EDC private key address.
// INPUT   : address - start address to read.
//	         length  - length of data to read.
// OUTPUT  : data    - storage of data read.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_GetEdcPrvKey(UINT32 address, UINT32 length, UINT8 *data)
{
    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_EDC_PRV_KEY");
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memmove(data, pSecMem + address, length);
        memset(pSecMem, 0x00, EDC_PRV_KEY_SLOT_LEN);
        free(pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Read data from the specified CA certificate address.
// INPUT   : address - start address to read.
//	         length  - length of data to read.
// OUTPUT  : data    - storage of data read.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_GetCaCert(UINT32 address, UINT32 length, UINT8 *data)
{
    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_CA_CERTIFICATE");
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memmove(data, pSecMem + address, length);
        memset(pSecMem, 0x00, CA_CERTIFICATE_SLOT_LEN);
        free(pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Read data from the specified client certificate address.
// INPUT   : address - start address to read.
//	         length  - length of data to read.
// OUTPUT  : data    - storage of data read.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_GetClientCert(UINT32 address, UINT32 length, UINT8 *data)
{
    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_CLIENT_CERTIFICATE");
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memmove(data, pSecMem + address, length);
        memset(pSecMem, 0x00, CLIENT_CERTIFICATE_SLOT_LEN);
        free(pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Read data from the specified client private key address.
// INPUT   : address - start address to read.
//	         length  - length of data to read.
// OUTPUT  : data    - storage of data read.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	OS_SECM_GetClientPrvKey(UINT32 address, UINT32 length, UINT8 *data)
{
    char    fileName[48];
    UINT8   *pSecMem;


    strcpy(fileName, "FILE_CLIENT_PRV_KEY");
    pSecMem = load_secure_file(fileName);

    if(pSecMem != NULL)
    {
        memmove(data, pSecMem + address, length);
        memset(pSecMem, 0x00, CLIENT_PRV_KEY_SLOT_LEN);
        free(pSecMem);
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Verify the SRED status by checking "SYS_ENABLE_FLAG".
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE  -- ready.
//           FALSE -- not ready.
// ---------------------------------------------------------------------------
UINT32	OS_SECM_VerifySredStatus(void)
{
	UINT8	status;


	OS_SECM_GetData(ADDR_SYS_SRED_STATUS, 1, &status);

	if(status == SYS_ENABLE_FLAG)
		return TRUE;
	else
		return FALSE;
}

// ---------------------------------------------------------------------------
// FUNCTION: Set SRED status to enablement or disablement
// INPUT   : flag - 0 = disable SRED
//				    1 = enable SRED
// OUTPUT  : none.
// RETURN  : TRUE  -- OK
//	         FALSE -- Failed
// ---------------------------------------------------------------------------
UINT32	OS_SECM_SetSredStatus(UINT8 flag)
{
	UINT8 status;


	if(flag == 1)
	{
		status = SYS_ENABLE_FLAG;
		OS_SECM_PutData(ADDR_SYS_SRED_STATUS, 1, &status);

		if(OS_SECM_VerifySredStatus() == TRUE)
			return TRUE;
		else
			return FALSE;
	}
	else if(flag == 0)
	{
		status = SYS_DISABLE_FLAG;
		OS_SECM_PutData(ADDR_SYS_SRED_STATUS, 1, &status);
		if(OS_SECM_VerifySredStatus() == FALSE)
			return TRUE;
		else
			return FALSE;
	}

	return FALSE;
}