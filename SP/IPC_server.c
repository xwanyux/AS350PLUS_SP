#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <sys/shm.h> //shmget,shmat,shmdt,shmctl
#include <semaphore.h>
#include <fcntl.h>           /* For O_* constants */
#include <time.h>
#include <pthread.h>
#include "sock.h"
#include "bsp_types.h"
#include "OS_PROCS.h"
#include "POSAPI.h"
#include "PEDKconfig.h"
#include "OS_LIB.h"
#include "OS_MSG.h"
#include "DEV_PED.h"
#include "TSCAPI.h"
#if 0   //used for PEDK test
#include "GDATAEX.h"
#include "PEDAPI.h" 
#include "SRED.h"
#include "SRED_Func.h"
#include "SRED_DBG_Function.h"
#endif
//LCD
#include "DEV_LCD.h"
#include "GUI.h"
#include "UTILS.h"
//PEDS
#include "OS_SECM.h"
//DEV_MEM
#include "DEV_MEM.h"
//Segmentation fault
#include <signal.h>

//Added by Tammy, IOCTL
#define __USE_LINUX_IOCTL_DEFS
#include <sys/ioctl.h>

// secret information
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>

/**
 * SHM structure
 * UINT8    API return value(If client call API, this value will be 0x99)
 * UINT8    PID             (API code ex:psDEV_LAN  see OS_PROCS.h for more information)
 * UINT8    APInumber       (Order of API function. XXX_open() will be 1, and XXX_close() will be 2)
 * UINT8    ArgsNumber      (Arguments number)
 * UINT32   ArgsSize        (Input arguments total size)
 * UINT8    *Args           (Input arguments)
 * 
*/
//graphic size(320*240*3)+reserve size( 100 )
#define MMAP_DATA_SIZE 2342500
#define CLEIENT_ARGS_SET 0x99
#define _23HR_IN_SEC 60*60*23
#define _2HR_IN_SEC 60*60*2
//#define	_DPA_TEST_	1
static char* command_cdRoot = "cd ~";
static char* command_cdAP = "cd /home/AP";
static char* command_killAP = "killall IPC_client";
static char* command_killSP = "killall IPC_server";
static char* command_rmAP = "rm /home/AP/IPC_client";
// static char* command_rmsecure_region = "rm /home/root/secure_region";
static char    *command_rmsecure_directory = "rm -r /home/root/secure_directory";
int fd;
struct sockaddr_in saddr;
int client_fd;
int shmfd;
int shmTMRfd;
static sem_t  *sem_A,*sem_B;
int tamperRegisterfd;
int tamperDevfd;
static key_t shm_key;
UCHAR *shm_data;
UCHAR *shm_timer;
UCHAR *HPSVSR,*LPTDSR,*LPSR;
static struct timespec NowTime;
UCHAR static const SP_initFilePath[] = {"/home/root/IPC_server_init"};
UCHAR static const AP_filePath[] = {"/home/AP/IPC_client"};
pthread_t tamperThread = 0;
pthread_t lanThread = 0;
UCHAR isTamperDetected = FALSE;
UINT8 dhn_rtc;

//Added by Tammy
//AES DUKPT variable
extern UCHAR IntermediateDerivationKeyInUse[32];
extern ULONG g_CurrentDerivationKey;
extern ULONG g_TransactionCounter;

//Added by Tammy, IOCTL
typedef struct cipherData_t
{
    unsigned short dataLen;
    unsigned char data[2048];
    unsigned short keyLen;
    unsigned char key[2048];
} cipherData;

typedef struct tcpConnectionRecord_t
{
    unsigned char needToClose;
    unsigned long connectionTime;
} connectionRecord;

//magic number
#define RSA_MAGIC_NUM   'b'
#define TCP_MAGIC_NUM   'c'
//create IOCTL commands in a User space application
#define IOCTL_RSA_VERIFY    _IOR(RSA_MAGIC_NUM, 0, int)
#define IOCTL_RSA_ENCRYPT   _IOWR(RSA_MAGIC_NUM, 1, cipherData)
#define IOCTL_RSA_DECRYPT   _IOWR(RSA_MAGIC_NUM, 2, cipherData)
#define IOCTL_TCP_GET_CONNECTION_TIME _IOR(TCP_MAGIC_NUM, 0, connectionRecord)
#define IOCTL_TCP_DISCONNECT    _IOW(TCP_MAGIC_NUM, 1, connectionRecord)

#define TCP_MAX_CONNECTION_CNT 8
#define TCP_MAX_DURATION 10*60


// ---------------------------------------------------------------------------
//	HW  :AS350-PED-03-HW-V1.0
//  FW  :AS350-PED-03-FW-V1.0
//	AP  :AS350-PED-03-AP-V1.0
//	SRED:AS350-PED-03-SRED-V1.0
// ---------------------------------------------------------------------------
void	ShowPEDVersions(void)
{
    UINT8   msg_HWV[] = {" HW  :AS350-PED-03-HW-V1.0"};
    UINT8   msg_FWV[] = {" FW  :AS350-PED-03-FW-V1.0"};
    UINT8	msg_APV[] = {" AP  :AS350-PED-03-AP-V1.0"};
    UINT8	msg_SRV[] = {" SRED:AS350-PED-03-SRED-V1.0"};


    LIB_LCD_Cls();

    LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_HWV) - 1, (UINT8 *)msg_HWV);
    LIB_LCD_Puts(3, 0, FONT1, sizeof(msg_FWV) - 1, (UINT8 *)msg_FWV);
    LIB_LCD_Puts(4, 0, FONT1, sizeof(msg_APV) - 1, (UINT8 *)msg_APV);

    if(OS_SECM_VerifySredStatus() == TRUE)
        LIB_LCD_Puts(5, 0, FONT1, sizeof(msg_SRV) - 1, (UINT8 *)msg_SRV);
    
    LIB_WaitTime(100);
}

// ---------------------------------------------------------------------------
// FUNCTION: 95 - Show identifier information.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	ShowIdentifierInfo(void)
{
    UINT8   msg_modelName[] = {" AS350 PINpad Terminal"};
    UINT8   msg_HWV[] = {" HW  :AS350-PED-03-HW-V1.0"};
    UINT8   msg_FWV[] = {" FW  :AS350-PED-03-FW-V1.0"};
    UINT8	msg_APV[] = {" AP  :AS350-PED-03-AP-V1.0"};
    UINT8	msg_SRV[] = {" SRED:AS350-PED-03-SRED-V1.0"};


    LIB_LCD_Cls();

    LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_modelName) - 1, (UINT8 *)msg_modelName);
    LIB_LCD_Puts(3, 0, FONT1, sizeof(msg_HWV) - 1, (UINT8 *)msg_HWV);
    LIB_LCD_Puts(4, 0, FONT1, sizeof(msg_FWV) - 1, (UINT8 *)msg_FWV);
    LIB_LCD_Puts(5, 0, FONT1, sizeof(msg_APV) - 1, (UINT8 *)msg_APV);

    if(OS_SECM_VerifySredStatus() == TRUE)
        LIB_LCD_Puts(6, 0, FONT1, sizeof(msg_SRV) - 1, (UINT8 *)msg_SRV);
    
    LIB_WaitKey();
}

void    Self_Test()
{
    time_t BeginDate = time(NULL);
    struct tm tm = *localtime(&BeginDate);
    clock_gettime(CLOCK_MONOTONIC, &NowTime);
    // printf("tm.tm_hour=%d\n",tm.tm_hour);
    if(
    //   ((tm.tm_hour == 5) && (NowTime.tv_sec > _2HR_IN_SEC)) ||//at 5 a.m. and have been powered on over 2 hr
    //   ((tm.tm_hour == 1) && (NowTime.tv_sec > _2HR_IN_SEC)) ||//at 1 a.m. and have been powered on over 2 hr
      (NowTime.tv_sec > _23HR_IN_SEC)//power on time over 23 hr
      )
        api_sys_reset(0);
}

//Function name: Check_firmware_status()
//Note:
//if uboot,kernel,SP update byte are all 0, set these three bytes 1 and clear tamper event.
//if these three bytes are not all 0, do nothing.
//execute this function one time when power on.
static void Check_firmware_status()
{

}

void    SegV_handler(int sig)
{
    UINT8* text_STACK_CORRUPT = "[SP] STACK CORRUPTED";
    api_lcdtft_open(0);
    LIB_LCD_Cls();
    write(2, "stack overflow\n", 15);
    LIB_LCD_Puts(0, 0, FONT1+attrREVERSE, strlen(text_STACK_CORRUPT), (UINT8 *)text_STACK_CORRUPT);
    _exit(1);
}

void    SegV_sethandler()
{
    static char stack[SIGSTKSZ];
    stack_t ss = {
        .ss_size = SIGSTKSZ,
        .ss_sp = stack,
    };
    struct sigaction sa = {
        .sa_handler = SegV_handler,
        .sa_flags = SA_ONSTACK
    };
    sigaltstack(&ss, 0);
    sigfillset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, 0);
}

void    openSHM()
{
    shm_key = ftok("/", 888);
    if(shm_key < 0)
    {
        perror("get_key failed ");
        exit(1);
    }
    // printf("server shm_key=%d\n",shm_key);

    shmfd = shmget(shm_key, MMAP_DATA_SIZE, 0777|IPC_CREAT);
    if(shmfd < 0)
    {
        perror("creat failed ");
        exit(1);
    }

    shm_data = shmat(shmfd, NULL, 0);
    if(shm_data < 0)
    {
        perror("map failed ");
        exit(1);
    }

    sem_A = sem_open("/semA", O_CREAT, 0777, 0);
    sem_B = sem_open("/semB", O_CREAT, 0777, 0);
    chmod("/dev/shm/sem.semA", 0777);
    chmod("/dev/shm/sem.semB", 0777);
    *shm_data = 0;//initial return value
}

/**
 * In the case of a security violation the SRTC stops counting and the SRTC is invalidated
 * (SRTC_ENV bit is cleared).
*/
UINT8   IsSRTCInvalidated()   //Added by Tammy
{
    UINT8   result = FALSE;
    void    *map;
    UINT8   *LPCR;


    tamperRegisterfd=open("/dev/mem", O_RDWR|O_RSYNC);
    
    if(tamperRegisterfd == -1)
    {
        printf("open /dev/mem fail\n");
        return FALSE;
    }
    map = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, tamperRegisterfd, 0x020CC000); 
    if (map == MAP_FAILED)
    {
        printf("mmap fail\n");
        return FALSE;
    }

    LPCR = map + 0x38;
    // printf(">>>LPCR = %02X %02X %02X %02X\n", *(LPCR + 3), *(LPCR + 2), *(LPCR + 1), *LPCR);
    if(*LPCR & 0x01)
        result = FALSE;
    else
        result = TRUE;
    
    return result;
}

UINT8   openTamperRegister()
{
    void  *map;
    tamperRegisterfd = open("/dev/mem", O_RDWR|O_RSYNC);
    
    if(tamperRegisterfd == -1)
    {
        printf("open /dev/mem fail\n");
        return FALSE;
    }

    map = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, tamperRegisterfd, 0x020CC000); 
    if(map == MAP_FAILED)
    {
        printf("mmap fail\n");
        return FALSE;
    }

    HPSVSR = map + 0x18;
    LPTDSR = map + 0xA4;
    LPSR = map + 0x4C;
}

void    Tamper_ShowSNVSRegister()  // ==== [Debug] ====
{
    void    *map;
    UINT8   *LPMKCR, *HPLR, *LPLR, *HPCOMR, *HPSR;
    UINT8   MASTER_KEY_SEL = 0;
    UINT8   ZMK_HWP = 0;
    UINT8   ZMK_VAL = 0;
    UINT8   ZMK_WSL = 0;
    UINT8   ZMK_RSL = 0;
    UINT8   ZMK_WHL = 0;
    UINT8   ZMK_RHL = 0;
    UINT8   PROG_ZMK = 0;
    UINT8   MKS_EN = 0;
    UINT8   SSM_STATE = 0;
    UINT8   ZMK_ZERO = 0;
    UINT8   *LPCR;
    UINT8   *LPSR;
    UINT8   *LPPGDR;
    UINT8   *HPCR;


    tamperRegisterfd = open("/dev/mem", O_RDWR|O_RSYNC);
    
    if(tamperRegisterfd == -1)
    {
        printf("open /dev/mem fail\n");
        return FALSE;
    }
    map = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, tamperRegisterfd, 0x020CC000); 
    if (map == MAP_FAILED)
    {
        printf("mmap fail\n");
        return FALSE;
    }

    LPMKCR = map + 0x3C;
    printf(">>>LPMKCR = %02X %02X %02X %02X\n", *(LPMKCR + 3), *(LPMKCR + 2), *(LPMKCR + 1), *LPMKCR);

    MASTER_KEY_SEL = *(LPMKCR) & 0x03;
    printf("Master Key Select = %02X\n", MASTER_KEY_SEL);

    ZMK_HWP = (*(LPMKCR) & (1 << 2)) >> 2;
    printf("ZMK hardware Programming mode = %02X\n", ZMK_HWP);

    ZMK_VAL = (*(LPMKCR) & (1 << 3)) >> 3;
    printf("ZMK Valid = %02X\n", ZMK_VAL);

    HPLR = map;
    printf(">>>HPLR = %02X %02X %02X %02X\n", *(HPLR + 3), *(HPLR + 2), *(HPLR + 1), *HPLR);

    ZMK_WSL = *(HPLR) & 1;
    printf("ZMK Write Soft Lock = %02X\n", ZMK_WSL);

    ZMK_RSL = (*(HPLR) & (1 << 1)) >> 1;
    printf("ZMK Read Soft Lock = %02X\n", ZMK_RSL);

    LPLR = map + 0x34;
    printf(">>>LPLR = %02X %02X %02X %02X\n", *(LPLR + 3), *(LPLR + 2), *(LPLR + 1), *LPLR);

    ZMK_WHL = *(LPLR) & 1;
    printf("ZMK Write Hard Lock = %02X\n", ZMK_WHL);

    ZMK_RHL = (*(LPLR) & (1 << 1)) >> 1;
    printf("ZMK Read Hard Lock = %02X\n", ZMK_RHL);

    HPCOMR = map + 0x04;
    printf(">>>HPCOMR = %02X %02X %02X %02X\n", *(HPCOMR + 3), *(HPCOMR + 2), *(HPCOMR + 1), *HPCOMR);

    PROG_ZMK = (*(HPCOMR + 1) & (1 << 4)) >> 4;
    printf("Program ZMK = %02X\n", PROG_ZMK);

    MKS_EN = (*(HPCOMR + 1) & (1 << 5)) >> 5;
    printf("Master Key Select Enable = %02X\n", MKS_EN);

    HPSR = map + 0x14;
    printf(">>>HPSR = %02X %02X %02X %02X\n", *(HPSR + 3), *(HPSR + 2), *(HPSR + 1), *HPSR);

    SSM_STATE = *(HPSR + 1) & 0x0F;
    printf("System Security Monitor State = %02X\n", SSM_STATE);
    if(SSM_STATE >= 0x0B)
    {
        switch(SSM_STATE)
        {
            case 0x0B:
                printf ("System Security Monitor is in Non-Secure mode\n");
				break;
			case 0x0D:
				printf ("System Security Monitor is in Trusted mode\n");
				break;
			case 0x0F:
				printf ("System Security Monitor is in Secure mode\n");
				break;
			default:
				printf ("System Security Monitor is in an undefined mode. Possible to have a hw problem or a Secure-boot issue (check HAB events).\n");
				return -1;
        }
    }

    ZMK_ZERO = (*(HPSR + 3) & (1 << 7)) >> 7;
    printf("Zeroizable Master Key is Equal to Zero = %02X\n", ZMK_ZERO);

    LPCR = map + 0x38;
    printf(">>>LPCR = %02X %02X %02X %02X\n", *(LPCR + 3), *(LPCR + 2), *(LPCR + 1), *LPCR);

    LPSR = map + 0x4C;
    printf(">>>LPSR = %02X %02X %02X %02X\n", *(LPSR + 3), *(LPSR + 2), *(LPSR + 1), *LPSR);

    LPPGDR = map + 0x64;
    printf(">>>LPPGDR = %02X %02X %02X %02X\n", *(LPPGDR + 3), *(LPPGDR + 2), *(LPPGDR + 1), *LPPGDR);

    HPCR = map + 0x08;
    printf(">>>HPCR = %02X %02X %02X %02X\n", *(HPCR + 3), *(HPCR + 2), *(HPCR + 1), *HPCR);

    LPTDSR = map + 0xA4;
    printf(">>>LPTDSR = %02X %02X %02X %02X\n", *(LPTDSR + 3), *(LPTDSR + 2), *(LPTDSR + 1), *LPTDSR);
}

void    Tamper_RemoveSensitiveData()   //Modified by Tammy
{
    FILE *fp;


    fp = popen(command_cdAP, "r");
    if(!fp)
        perror("Failed to cd /home/AP");
    pclose(fp);

    fp = popen(command_killAP, "r");
    if(!fp)
        perror("Failed to kill IPC_client");
    pclose(fp);

    fp = popen(command_rmAP, "r");
    if(!fp)
        perror("Failed to remove IPC_client");
    pclose(fp);

    // fp = popen(command_rmsecure_region, "r");
    fp = popen(command_rmsecure_directory, "r");
    if(!fp)
        perror("Failed to remove secure_directory");
    pclose(fp);

    fp = popen(command_cdRoot, "r");
    if(!fp)
        perror("Failed to cd /home/root");
    pclose(fp);

    fp = popen(command_killSP, "r");
    if(!fp)
        perror("Failed to kill IPC_server");
    pclose(fp);
}

void    Tamper_DisplayTamperMessage(UCHAR tamperResult)    //Modified by Tammy
{
    char    *text_TamperOccur = " TAMPER OCCURRED ";
    char    *tamperMsg1 = "External Tampering 1 Detected!";
    char    *tamperMsg2 = "External Tampering 2 Detected!";
    char    *tamperMsg3 = "External Tampering 3 Detected!";
    char    *tamperMsg4 = "External Tampering 4 Detected!";
    char    *tamperMsg5 = "External Tampering 5 Detected!";
    // char    *tamperMsg6 = "Temperature Tampering Detected!";
    // char    *tamperMsg7 = "Voltage Tampering Detected!";
    char    *msg = "Please contact the vendor.";
    UINT8   line = 3;


    api_lcdtft_open(0);
    LIB_LCD_Cls();
    printf("after LIB_LCD_Cls()\n");
    
    LIB_LCD_Puts(0, 2, FONT2 + attrREVERSE, strlen(text_TamperOccur), (UINT8 *)text_TamperOccur);

    if(tamperResult & (1 << 0))
    {
        printf("\033[1;31;40mExternal Tampering 1 Detected\033[0m\n");
        LIB_LCD_Puts(line, 5, FONT1, strlen(tamperMsg1), (UINT8 *)tamperMsg1);
        line++;
    }
        
    if(tamperResult & (1 << 1))
    {
        printf("\033[1;31;40mExternal Tampering 2 Detected\033[0m\n");
        LIB_LCD_Puts(line, 5, FONT1, strlen(tamperMsg2), (UINT8 *)tamperMsg2);
        line++;
    }
    
    if(tamperResult & (1 << 2))
    {
        printf("\033[1;31;40mExternal Tampering 3 Detected\033[0m\n");
        LIB_LCD_Puts(line, 5, FONT1, strlen(tamperMsg3), (UINT8 *)tamperMsg3);
        line++;
    }

    if(tamperResult & (1 << 3))
    {
        printf("\033[1;31;40mExternal Tampering 4 Detected\033[0m\n");
        LIB_LCD_Puts(line, 5, FONT1, strlen(tamperMsg4), (UINT8 *)tamperMsg4);
        line++;
    }

    if(tamperResult & (1 << 4))
    {
        printf("\033[1;31;40mExternal Tampering 5 Detected\033[0m\n");
        LIB_LCD_Puts(line, 5, FONT1, strlen(tamperMsg5), (UINT8 *)tamperMsg5);
        line++;
    }

    // if(tamperResult & (1 << 5))
    // {
    //     printf("\033[1;31;40mTemperature Tampering Detected\033[0m\n");
    //     LIB_LCD_Puts(line, 5, FONT1, strlen(tamperMsg6), (UINT8 *)tamperMsg6);
    //     line++;
    // }

    // if(tamperResult & (1 << 6))
    // {
    //     printf("\033[1;31;40mVoltage Tampering Detected\033[0m\n");
    //     LIB_LCD_Puts(line, 5, FONT1, strlen(tamperMsg7), (UINT8 *)tamperMsg7);
    //     line++;
    // }

    LIB_LCD_Puts(++line, 7, FONT1, strlen(msg), (UINT8 *)msg);
}

UINT8   Tamper_IsSecureDevice()    //Added by Tammy
{
    int     fd;
    void    *map;
    UINT8   *LPGPR0_alias;
    UINT8   value[4] = {0x83, 0x00, 0x45, 0x70};
    UINT8   result = FALSE;


    if((fd = open("/dev/mem", O_RDWR, 0)) < 0)
        printf("Open /dev/mem error, maybe check permission.\n");
    
    map = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x020CC000); 
    if(map == NULL)
    {
        printf("Error mapping address\n");
		close(fd);
    }

    LPGPR0_alias = map + 0x68;
    if(!memcmp(LPGPR0_alias, value, 4))
        result = TRUE;
    else
        result = FALSE;

    munmap(map, 4096);
    close(fd);
    return result;
}

void    Tamper_SetupSecureDevice()    //Added by Tammy
{
    int     fd;
    void    *map;
    UINT8   *LPGPR0_alias;
    UINT8   value[4] = {0x70, 0x45, 0x00, 0x83};


    if((fd = open("/dev/mem", O_RDWR, 0)) < 0)
        printf("Open /dev/mem error, maybe check permission.\n");
    
    map = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x020CC000); 
    if(map == NULL)
    {
        printf("Error mapping address\n");
		close(fd);
    }

    LPGPR0_alias = map + 0x68;
    memcpy(LPGPR0_alias, value, 4);

    munmap(map, 4096);
    close(fd);
}

UINT8   WhichTamperHappened() //Modified by Tammy
{
    UINT8   result = 0;
    

    if(*(LPSR + 1) & 1 << 1)
        result |= 1;        // tamper 1 event
    if(*(LPSR + 1) & 1 << 2)
        result |= 1 << 1;   // tamper 2 event
    if(*(LPTDSR) & 1)
        result |= 1 << 2;   // tamper 3 event
    if(*(LPTDSR) & 1 << 1)
        result |= 1 << 3;   // tamper 4 event
    if(*(LPTDSR) & 1 << 2)
        result |= 1 << 4;   // tamper 5 event
    // if(*(LPSR) & 1 << 5)
    //     result |= 1 << 5;   // temperature tamper event
    // if(*(LPSR) & 1 << 6)
    //     result |= 1 << 6;   // voltage tamper event

    return result;
}

void    TAMPER_Handler()    //Modified by Tammy
{
    UINT8   tamperResult = 0;
    

    while(1)
    {
        tamperResult = WhichTamperHappened();
        if(tamperResult)
        {
            isTamperDetected = TRUE;

            LIB_BUZ_Open();

            printf("\033[1;31;40mTamper occurred\033[0m\n");
            Tamper_DisplayTamperMessage(tamperResult);
            LIB_BUZ_TamperedBeeps(1);
            Tamper_RemoveSensitiveData();
        }
        else
            usleep(10000);
    }
}

void    Tamper_polling()    //Added by Tammy
{
    pthread_create(&tamperThread, NULL, TAMPER_Handler, NULL);
}

void    Tamper_LoopTest()  //Added by Tammy
{
    UINT8   tamperResult = 0;
    char    *noTamperMsg = "No External Tampering Detected";
    UINT8   key;

    while(1)
    {
        if(LIB_GetKeyStatus() == apiReady)
        {
            if(LIB_WaitKey() == 'x')
                break;
        }

        tamperResult = WhichTamperHappened();
        if(tamperResult)
        {
            Tamper_DisplayTamperMessage(tamperResult);
        }
        else
        {
            LIB_LCD_Cls();
            printf("\033[1;31;40mNo External Tampering Detected\033[0m\n");
            LIB_LCD_Puts(2, 5, FONT1, strlen(noTamperMsg), (UINT8 *)noTamperMsg);
        }

        LIB_WaitTime(100);
    }
}

void    report(const char* msg, int terminate)
{
    perror(msg);
    if(terminate) exit(-1); /* failure */
}

UINT8   SHM_optArgLength(UINT32 optlen)
{
    memmove(&shm_data[4],&optlen,4);
    // printf("optlen=%d shm_data[4]=%d\n", optlen, shm_data[4]);
}

UINT8   IPC_FunctionCaller(OS_IPCSOCKET_HEADER socketHeader, UCHAR *args)
{
    UINT8             PID = socketHeader.PID;             // psDEV id
    UINT8             FuncNum = socketHeader.Func_num;    // API function numbers
    UINT8             Argc = socketHeader.Func_input_num; // Input arguments number
    UINT32            ArgLen = socketHeader.ArgsTotalSize;
    UINT8             ret = apiFailed;
    UINT32            optlen = 0;
    UINT8             status;
    struct timeval    nowtime;


    if(isTamperDetected)
        return ret;

    // if(PID==psDEV_NXP)
    // printf("Funcall PID=0x%x Funcnum=%d\n",PID,FuncNum);
    switch(PID)
    {
#pragma region //==FUNC_OS==
        case psDEV_OS:
            switch (FuncNum)
            {
                UINT32 cnt;

                case 1:	// ULONG OS_GET_SysTimerFreeCnt( void );
                    cnt = OS_GET_SysTimerFreeCnt();

                    *(args + 0) =  cnt & 0x000000ff;
                    *(args + 1) = (cnt & 0x0000ff00) >> 8;
                    *(args + 2) = (cnt & 0x00ff0000) >> 16;
                    *(args + 3) = (cnt & 0xff000000) >> 24;
      
                    optlen = 4;
                    break;
                case 2:	// void OS_SET_SysTimerFreeCnt( ULONG value );
                    cnt = *(args + 0) + *(args + 1) * 0x100 + *(args + 2) * 0x10000 + *(args + 3) * 0x1000000;
                    OS_SET_SysTimerFreeCnt(cnt);
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region //==TIM==
        case psDEV_TIM:
            switch(FuncNum)
            {
                case 1:
                    ret = api_tim_open(*args);
                    break;
                case 2:
                    ret = api_tim_close(*args);
                    break;
                case 3:
                    ret = api_tim_gettick(*args, args+1);
                    // printf("timer get tick=%x\n",(UINT16)*(args+1));
                    optlen = 2;
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region //==BUZ==
        case psDEV_BUZ:
            switch(FuncNum)
            {
                case 1:
                    ret = api_buz_open(args);
                    break;
                case 2:
                    ret = api_buz_close(*args);
                    break;
                case 3:
                    ret = api_buz_sound(*args);
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region //==KBD==
        case psDEV_KBD:
            switch(FuncNum)
            {
                case 1:
                    ret = api_kbd_open(*args, (args+1));
                    break;
                case 2:
                    ret = api_kbd_close(*args);
                    break;
                case 3:
                    ret = api_kbd_status(*args, (args + 1));
                    optlen = 1;
                    break;
                case 4:
                    ret = api_kbd_getchar(*args, (args + 1));
                    // printf("server get char:0x%x\n",*(args+1));
                    optlen = 1;
                    break;
                case 5:
                    ret = api_kbd_get_multiple_char(*args, (args + 1));
                    optlen = 5;
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region //==LCD==
        case psDEV_LCD:    
            switch(FuncNum)
            {
                API_LCDTFT_PARA     para;
                API_LCDTFT_GRAPH    graph;
                API_LCDTFT_ICON     graphicon;
                API_PCD_ICON        pcdicon;
                API_LCDTFT_WINBMP   *bmppara;
                API_LCD_FONT        ft;
                API_TSC_PARA        para_tsc;
                UCHAR               *pdata;
                UCHAR               *pcode;
                UCHAR               *pbmp;
                ULONG               datalen;
                ULONG	            FontCodeLen;
                ULONG	            FontBmpLen;
                
                case 1:
                    ret = api_lcdtft_open(*args);
                    // printf("lcdtft open input=%d\n", *args);
                    break;
                case 2:
                    ret = api_lcdtft_close(*args);
                    break;
                case 3:
                    memmove(&para, args + 1, sizeof(para));
                    ret = api_lcdtft_clear(*args, para);
                    break;
                case 4:
                    // nowtime = GetNowTime();
                    // printf("!!server:%d us\n", nowtime.tv_usec);
                    memmove(&para, args + 1, sizeof(para));
                    ret = api_lcdtft_putstring(*args, para, args + 1 + sizeof(para));
                    // nowtime = GetNowTime();
                    // printf("!!server:%d us\n", nowtime.tv_usec);
                    break;
                case 5:
                    memmove(&graph, args + 1, sizeof(graph));
                    datalen = graph.Bitmap.BytesPerLine * graph.Bitmap.YSize;
                    pdata = malloc(datalen);
                    if(pdata == NULL)
                        printf("!!!!!!!!!!pdata malloc failed\n");
                    memset(pdata, 0, datalen);
                    for(ULONG i = 0 ; i < datalen ; i++)
                        *(pdata + i) = *(args + 1 + sizeof(graph) + i);
                    graph.Bitmap.pData = pdata;
                    // for(ULONG i = 0, value = 0 ;; i++)
                    // {
                    //     if(value = *(graph.Bitmap.pData + i) > 0)
                    //     {
                    //         printf("server i=%d value= ", i);
                    //         for(int g = 0 ; g < 10 ; g++)
                    //             printf("%d ", *(graph.Bitmap.pData + i + g));
                    //         break;
                    //     }
                    // }
                    // printf("\n");
                    ret = api_lcdtft_putgraphics(*args, graph);
                    free(pdata);
                    break;
                case 6:
                    memmove(&graphicon, args + 1, sizeof(graphicon));
                    ret = api_lcdtft_showICON(*args, graphicon);
                    break;
                case 7:
                    memmove(&pcdicon, args + 1, sizeof(pcdicon));
                    ret = api_lcdtft_showPCD(*args, pcdicon);
                    break;
                case 8:
                    bmppara = (API_LCDTFT_WINBMP *)(args + 1);
                    pdata = args + 1 + sizeof(API_LCDTFT_WINBMP);
                    ret = api_lcdtft_putwinbmp(*args, bmppara, pdata);
                    break;
                case 9:
                    memmove(&para, args + 1, sizeof(API_LCDTFT_PARA));
                    pdata = args + 1 + sizeof(API_LCDTFT_PARA);
                    ret = SIGNPAD_lcdtft_putstring(*args, para, pdata, *(args + ArgLen - 2));
                    break;
                case 10:
                    memmove(&ft, args, sizeof(API_LCD_FONT));
                    memmove(&FontCodeLen, args + sizeof(API_LCD_FONT), 4);
                    pcode = malloc(FontCodeLen);
                    for(ULONG i = 0 ; i < FontCodeLen ; i++)
                        *(pcode + i) = *(args + sizeof(API_LCD_FONT) + 4 + i);
                    ft.codStAddr = pcode;
                    ft.codEndAddr = pcode + FontCodeLen - 1;
                    memmove(&FontBmpLen, args + sizeof(API_LCD_FONT) + 4 + FontCodeLen, 4);
                    pbmp = malloc(FontBmpLen);
                    for(ULONG i = 0 ; i < FontBmpLen ; i++)
                        *(pbmp + i) = *(args + sizeof(API_LCD_FONT) + 4 + FontCodeLen + 4 + i);
                    ft.bmpStAddr = pbmp;
                    ft.bmpEndAddr = pbmp + FontBmpLen - 1;
                    // printf("server FontCodeLen=%ld  FontBmpLen=%ld\n", FontCodeLen, FontBmpLen);
                    ret = api_lcdtft_initfont(ft);
                    free(pcode);
                    free(pbmp);
                    break;
                case 11:	// UCHAR api_tsc_open( UCHAR deviceid );
                    ret = api_tsc_open(*args);
                    break;
                case 12:	// UCHAR api_tsc_close( UCHAR dhn );
                    ret = api_tsc_close(*args);
                    break;
                case 13:	// UCHAR api_tsc_status( UCHAR dhn, API_TSC_PARA para, UCHAR *status );
                    memmove(&para_tsc, args + 1, sizeof(API_TSC_PARA));
                    status = 0;
                    ret = api_tsc_status(*args, para_tsc, &status);
                    *(args + 1 + sizeof(API_TSC_PARA)) = status;
                    // LIB_DispHexByte( 1, 0, status );
                    optlen = 1;
                    break;
                case 14:	// UCHAR api_tsc_signpad( UCHAR dhn, API_TSC_PARA para, UCHAR *status, UCHAR *palette );
                    memmove(&para_tsc, args + 1, sizeof(API_TSC_PARA));
                    ret = api_tsc_signpad(*args, para_tsc, args + 1, args + 1 + sizeof(para_tsc));
                    optlen = 1;
                    break;
                case 15:	// UCHAR api_tsc_getsign( UCHAR dhn, API_TSC_PARA para, UCHAR *status, UCHAR *sign, ULONG *length );
                    memmove(&para_tsc, args + 1, sizeof(API_TSC_PARA));
                    ret = api_tsc_getsign(*args, para_tsc, args + 1, args + 6, args + 2);
                    optlen = 1;
                    if(ret == apiOK)
                        optlen += *(args + 2) + *(args + 3) * 0x100 + *(args + 4) * 0x10000 + *(args + 5) * 0x1000000;
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region//==MSR==
        case psDEV_MSR:
            switch(FuncNum)
            {
                case 1:
                    ret = api_msr_open(*args);
                    break;
                case 2:
                    ret = api_msr_close(*args);
                    break;
                case 3:
                    ret = api_msr_status(*args, *(args + 1), args + 2);
                    optlen = 4;
                    break;
                case 4:
                    ret = api_msr_getstring(*args, args + 1, args + 3);
                    optlen = *(args + 3) + 1;
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region//==IFM==
    case psDEV_SCR:
        switch(FuncNum)
        {
            UINT    len;

            case 1:
                ret = api_ifm_open(*args);
                break;
            case 2:
                ret = api_ifm_close(*args);
                break;
            case 3:
                ret = api_ifm_present(*args);
                break;
            case 4:
                ret = api_ifm_reset(*args, *(args + 1), args + 2);
                optlen = *(args + 2) + 1;
                break;
            case 5:
                ret = api_ifm_deactivate(*args);
                break;
            case 6:
                len = *(args + 1) + (*(args + 2)) * 256;
                ret = api_ifm_exchangeAPDU(*args, args + 1, args + 1 + len + 2);
                optlen = *(args + 1 + len + 2) + (*(args + 1 + len + 2 + 1)) * 256 + 2;
                break;
            default:
                break;
        }
        break;
#pragma endregion
#pragma region//==AUX==
    case psDEV_AUX:
        switch(FuncNum)
        {
            API_AUX pAux;

            case 1:
                memmove(&pAux, args + 1, sizeof(API_AUX));
                ret = api_aux_open(*args, pAux);
                break;
            case 2:
                // printf("*args=%d\n",*args);
                ret = api_aux_close(*args);
                optlen = 0;
                // printf("@@@@@@@@@@@@%s %d\n",__func__,__LINE__);
                break;
            case 3:
                ret = api_aux_rxready(*args, args + 1);
                optlen = 2;
                break;
            case 4:
                ret = api_aux_rxstring(*args, args + 1);
                optlen = *(args + 1) + (*(args + 2) * 256) + 2;
                break;
            case 5:
                ret = api_aux_txready(*args);
                break;
            case 6:
                ret = api_aux_txstring(*args, args + 1);
                break;
            case 7:
                ret = api_aux_SetLongLen(*args, *(args + 1));
                break;
            case 8:
                ret = api_aux_SetAckMode(*args, *(args + 1));
                break;
            case 9:
                ret = BSP_UART_SendAck(*args, *(args + 1));
                break;
            default:
                break;
        }
        break;
#pragma endregion
#pragma region//==LAN==
        case psDEV_LAN:
            switch(FuncNum)
            {
                API_IPCONFIG    config;
                API_LAN         lan;
                UCHAR           SizeOfconfig = sizeof(API_IPCONFIG);
                UCHAR           SizeOflan = sizeof(API_LAN);
                ULONG           ConnectionTime;
                
                case 1:
                    // memmove(&config, args, SizeOfconfig);
                    // ret = api_lan_setIPconfig(config);
                    memmove(&config, args, sizeof(API_IPCONFIG));
                    ret = api_lan_setIPconfig(config);
                    break;
                case 2:
                    ret = api_lan_getIPconfig(&config);
                    memmove(args, &config, SizeOfconfig);
                    optlen = SizeOfconfig;
                    break;
                case 3:
                    // memmove(&lan, args, SizeOflan);
                    // ret = api_lan_open(lan);
                    memmove(&lan, args, sizeof(API_LAN));
                    ret = api_lan_open(lan);
                    break;
                case 4:
                    ret = api_lan_close(*args);
                    break;
                case 5:
                    ret = api_lan_txready(*args);
                    break;
                case 6:
                    ret = api_lan_txstring(*args, args + 1);
                    break;
                case 7:
                    ret = api_lan_rxready(*args, args + 1);
                    optlen = 2;
                    break;
                case 8:
                    ret = api_lan_rxstring(*args, args + 1);
                    optlen = *(args + 1) | (*(args + 2) << 8) + 2;
                    break;
                case 9:
                    ret = api_lan_lstatus();
                    break;
                case 10:
                    ret = api_lan_status(*args, args + 1);
                    optlen = 1;
                    break;
                case 11:
                    ret = api_lan_setup_DHCP(*args);
                    break;
                case 12:
                    ret = api_lan_status_DHCP();
                    break;
                case 13:
                    ret = api_lan_ping(*args, args + 1, &ConnectionTime);
                    memmove(args + 2, &ConnectionTime, 4);
                    optlen = 4;
                    break;

                // case 20: // DEBUG TEST ONLY
                //     memmove(&lan, args, sizeof(API_LAN));
                //     ret = api_lan_open2(lan);
                //     //	ret=api_lan_open2();
                //     break;

                default:
                    break;
            }
            break;
#pragma endregion
#pragma region//==SYS==
        case psDEV_SYS:
            switch(FuncNum)
            {
                UINT    len;
                ULONG   length;

                case 1:
                    len = *args | *(args + 1) << 8;
                    ret = api_sys_random_len(args + 2, len);
                    optlen = len;
                    break;
                case 2:
                    ret = api_sys_random_len(args, 8);
                    optlen = 8;
                    break;
                case 3:
                    api_sys_reset(*args);
                    ret = 1;
                    break;
                case 4:
                    memmove(&length, args, 4);
                    ret = api_sys_SHA1(length, args + 4, args + 4 + length);
                    optlen = 20;
                    break;
                case 5:
                    memmove(&length, args, 4);
                    ret = api_sys_MD5(length, args + 4, args + 4 + length);
                    optlen = 16;
                    break;
                case 6:
                    memmove(&length, args + 1, 4);
                    ret = api_sys_SHA2(*args, length, args + 5, args + 5 + length);
                    optlen = 28;
                    break;
                case 7:
                    memmove(&length, args + 1, 4);
                    ret = api_sys_backlight(*args, length);
                    break;
                case 8:
                    ret = api_sys_info(*args, args + 1);
                    optlen = *(args + 1) + 1;
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region//==TSC==
        case psDEV_TSC:
            switch(FuncNum)
            {
                UINT    len;
                ULONG   length;

                case 1:
                    ret = api_tsc_close(*args);
                    break;
                default:
                    break;
            }
        break;
#pragma endregion
#pragma region//==SECM==
        case psDEV_SECM:
            switch(FuncNum)
            {
                ULONG   length5;
                ULONG   address5;

                case 1:
                    ret = OS_SECM_VerifyBootStatus();
                    break;
                case 2:
                    ret = OS_SECM_SetWarmBootStatus();
                    break;
                case 3:
                    ret = OS_SECM_ResetBootStatus();
                    break;
                case 4:
                    ret = OS_SECM_VerifyAppStatus();
                    break;
                case 5:
                    ret = OS_SECM_SetAppStatus();
                    break;
                case 6:
                    ret = OS_SECM_ResetAppStatus();
                    break;
                case 7:
                    memmove(&address5, args, 4);
                    memmove(&length5, args + 4, 4);
                    OS_SECM_PutData(address5, length5, args + 8);
                    ret = 0;
                    break;
                case 8:
                    memmove(&address5, args, 4);
                    memmove(&length5, args + 4, 4);
                    OS_SECM_GetData(address5, length5, args + 8);
                    ret = 0;
                    optlen = length5;
                    break;
                case 9:
                    memmove(&address5, args, 4);
                    memmove(&length5, args + 4, 4);
                    OS_SECM_ClearData(address5, length5, *(args + 8));
                    ret = 0;
                    break;
                case 10:
                    ret = OS_SECM_VerifySredStatus();
                    break;
                case 11:
                    ret = OS_SECM_SetSredStatus(*args);
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region//==SRAM==
        case psDEV_SRAM:
            switch(FuncNum)
            {
                ULONG           length;
                ULONG           address;
                API_SRAM_ADDR   pSramAddr;
                API_SRAM        pSram;

                case 1:
                    ret = api_sram_PageInit();
                    break;
                case 2:
                    memmove(&pSramAddr, args + 1, sizeof(API_SRAM_ADDR));
                    ret = api_sram_PageSelect(*args, &pSramAddr);
                    break;
                case 3:
                    memmove(&pSram, args, sizeof(API_SRAM));
                    ret = api_sram_PageLink(pSram, *(args + sizeof(API_SRAM)));
                    break;
                case 4:
                    memmove(&pSram, args, sizeof(API_SRAM));
                    ret = api_sram_PageRead(pSram, args + sizeof(API_SRAM));
                    optlen = pSram.Len;
                    break;
                case 5:
                    memmove(&pSram, args, sizeof(API_SRAM));
                    ret = api_sram_PageWrite(pSram, args + sizeof(API_SRAM));
                    break;
                case 6:
                    memmove(&pSram, args, sizeof(API_SRAM));
                    ret = api_sram_PageClear(pSram, *(args + sizeof(API_SRAM)));
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region//==SRED==
        case psDEV_SRED:
            switch(FuncNum)
            {
                ULONG   OutLength;
                ULONG   InLength;
                ULONG   address;
                UCHAR   panLen;
                UCHAR   pan[19];
                UCHAR   inLen;

                case 1:
                    //prototype:UINT8   SRED_Func_emv_PutDataElement(UINT8 index, UINT32 address, UINT32 length, UINT8 *data)
                    //argument order:index(1B) address(4B) length(4B) *data(length B)
                    memmove(&address, &args[1], 4);
                    memmove(&InLength, &args[1 + 4], 4);
                    ret = SRED_Func_emv_PutDataElement(args[0], address, InLength, &args[1 + 4 + 4]);
                    break;
                case 2:
                    //prototype:UINT8	SRED_Func_emv_GetDataElement(UINT32 address, UINT32 length, UINT8 *data)
                    //argument order:address(4B) length(4B) *data(length B)
                    memmove(&address, args, 4);
                    memmove(&InLength, &args[4], 4);
                    ret = SRED_Func_emv_GetDataElement(address, InLength, &args[4 + 4]);
                    optlen = InLength;
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region//==CPHR==
        case psDEV_CPHR:
            switch(FuncNum)
            {
                UCHAR   OutLength;
                UCHAR   InLength;
                UCHAR   ivLength;
                UCHAR   KeyLength;
                ULONG   modlen;
                ULONG   explen;
                UCHAR*  pOut;
                API_RSA cipher;

                case 1:
                    //prototype:ULONG	api_des_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey )
                    //argument order:*pIn(8B) *pKey(8B) *pOut(8 B)
                    ret = api_des_encipher(args, args + 16, args + 8);
                    optlen = 8;
                    break;
                case 2:
                    //prototype:ULONG	api_des_decipher( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey )
                    //argument order:*pIn(8B) *pKey(8B) *pOut(8 B)
                    ret = api_des_decipher(args + 16, args, args + 8);
                    optlen = 8;
                    break;
                case 3:
                    //prototype:ULONG	api_3des_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey )
                    //argument order:*pIn(8B) *pKey(24B) *pOut(8 B)
                    ret = api_3des_encipher(args, args + 32, args + 8);
                    optlen = 8;
                    break;
                case 4:
                    //prototype:ULONG	api_3des_encipher2( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR inLen )
                    //argument order:inLen(1B) *pIn(inLen B) *pKey(24B) *pOut(inLen/8 + inLen%8>0?1:0 B)
                    InLength = *args;
                    ret = api_3des_encipher2(args + 1, args + 1 + InLength + 24, args + 1 + InLength, InLength);
                    optlen = InLength / 8 + (InLength % 8 > 0) ? 1 : 0;
                    break;
                case 5:
                    //prototype:ULONG	api_3des_decipher( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey )
                    //argument order:*pIn(8B) *pKey(24B) *pOut(8 B)
                    ret = api_3des_decipher(args, args + 32, args + 8);
                    optlen = 8;
                    break;
                case 6:
                    //prototype:ULONG	api_3des_decipher2( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey, UCHAR inLen )
                    //argument order:inLen(1B) *pIn(inLen B) *pKey(24B) *pOut(inLen/8 + inLen%8>0?1:0 B)
                    InLength = *args;
                    ret = api_3des_decipher2(args + 1, args + 1 + InLength + 24, args + 1 + InLength, InLength);
                    optlen = InLength / 8 + (InLength % 8 > 0) ? 1 : 0;
                    break;
                case 7:
                    //prototype:ULONG	api_aes_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize )
                    //argument order:KeySize(1B) *pIn(KeySize B) *pKey(KeySize B) *pOut(KeySize B)
                    InLength = *args;
                    ret = api_aes_encipher(args + 1, args + 1 + InLength + InLength, args + 1 + InLength, InLength);
                    optlen = InLength;
                    break;
                case 8:
                    //prototype:ULONG	api_aes_decipher( UCHAR *pOut, UCHAR *pIn, UCHAR *pKey, UCHAR KeySize )
                    //argument order:KeySize(1B) *pIn(KeySize B) *pKey(KeySize B) *pOut(KeySize B)
                    InLength = *args;
                    ret = api_aes_decipher(args + 1, args + 1 + InLength + InLength, args + 1 + InLength, InLength);
                    optlen = InLength;
                    break;
                case 9:
                    //prototype:ULONG	api_aes_cbc_encipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize, UCHAR *iv, UCHAR ivLength)
                    //argument order:KeySize(1B) *pIn(KeySize B) *pKey(KeySize B) ivLength(1B) *iv(ivLength B) *pOut(KeySize B)
                    InLength = *args;
                    ivLength = *(args + 1 + InLength + InLength);
                    ret = api_aes_cbc_encipher(args + 1, args + 1 + InLength + InLength + 1 + ivLength, args + 1 + InLength, InLength, args + 1 + InLength + InLength + 1, ivLength);
                    optlen = InLength;
                    break;
                case 10:
                    //prototype:ULONG	api_aes_cbc_decipher( UCHAR *pIn, UCHAR *pOut, UCHAR *pKey, UCHAR KeySize, UCHAR *iv, UCHAR ivLength)
                    //argument order:KeySize(1B) *pIn(KeySize B) *pKey(KeySize B) ivLength(1B) *iv(ivLength B) *pOut(KeySize B)
                    InLength = *args;
                    ivLength = *(args + 1 + InLength + InLength);
                    ret = api_aes_cbc_decipher(args + 1, args + 1 + InLength + InLength + 1 + ivLength, args + 1 + InLength, InLength, args + 1 + InLength + InLength + 1, ivLength);
                    optlen = InLength;
                    break;
                case 11:
                    //prototype:ULONG	api_rsa_loadkey( UCHAR *modulus, UCHAR *exponent )
                    //argument order:*modulus(modulus[0]+modulus[1]*256+2 B) *exponent(1~3 B)
                    ret = api_rsa_loadkey(args, args + *args + (*(args + 1) * 256) + 2);
                    optlen = 0;
                    break;
                case 12:
                    //prototype:ULONG	api_rsa_recover( UCHAR *pIn, UCHAR *pOut )
                    //argument order:*pIn(pIn[0]+pIn[1]*256 B) *pOut(pOut[0]+pOut[1]*256 B)
                    pOut = args + 2 + *args + (*(args + 1) * 256);
                    ret = api_rsa_recover(args, pOut);
                    optlen = 2 + *pOut + *(pOut + 1) * 256;
                    break;
                case 13:
                    //prototype:ULONG	api_rsa_encrypt( API_RSA cipher )
                    memmove(&cipher, args, sizeof(ULONG) + 1);
                    cipher.Modulus = &args[1 + sizeof(ULONG)];
                    memmove(&cipher.ExpLen, &args[1 + sizeof(ULONG) + cipher.ModLen], sizeof(ULONG));
                    cipher.Exponent = &args[1 + sizeof(ULONG) + cipher.ModLen + sizeof(ULONG)];
                    memmove(&cipher.Length, &args[1 + sizeof(ULONG) + cipher.ModLen + sizeof(ULONG) + cipher.ExpLen], sizeof(ULONG));
                    cipher.pIn = &args[1 + sizeof(ULONG) + cipher.ModLen + sizeof(ULONG) + cipher.ExpLen + sizeof(ULONG)];
                    cipher.pOut = &args[1 + sizeof(ULONG) + cipher.ModLen + sizeof(ULONG) + cipher.ExpLen + sizeof(ULONG) + cipher.Length];
                    memmove(&optlen, cipher.pOut, sizeof(ULONG)); // output length have count already in ap
                    ret = api_rsa_encrypt(cipher);
                    break;
                case 14:
                //prototype:UCHAR	api_HMAC_SHA256_encipher( UCHAR *pIn, UCHAR InLength, UCHAR *pOut, UCHAR OutLength, UCHAR *pKey, UCHAR KeyLength)
                //argument order:InLength(1B) *pIn(InLength B) KeyLength(1B) *pKey(KeyLength B) OutLength(1B) *pOut(OutLength B)
                    InLength = *args;
                    KeyLength = *(args + 1 + InLength);
                    OutLength = *(args + 1 + InLength + 1 + KeyLength);
                    ret = api_HMAC_SHA256_encipher(args + 1, InLength, args + 1 + InLength + 1 + KeyLength + 1, OutLength, args + 1 + InLength + 1, KeyLength);
                    // printf("InLength=%d KeyLength=%d OutLength=%d\n",InLength,KeyLength,OutLength);
                    optlen = OutLength;
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region//==OS_SRAM==
        case psDEV_OSSRAM:
            switch(FuncNum)
            {
                ULONG   address;
                ULONG   length;
                UCHAR*  data;
                UCHAR   pattern;

                case 1:
                    memmove(&address, args, sizeof(ULONG));
                    memmove(&length, &args[sizeof(ULONG)], sizeof(ULONG));
                    data = &args[sizeof(ULONG) + sizeof(ULONG)];
                    ret = api_sram_write(address, length, data);
                    optlen = 0;
                    break;
                case 2:
                    memmove(&address, args, sizeof(ULONG));
                    memmove(&length, &args[sizeof(ULONG)], sizeof(ULONG));
                    data = &args[sizeof(ULONG) + sizeof(ULONG)];
                    ret = api_sram_read(address, length, data);
                    // printf("server addr=0x%lx\n", address);
                    optlen = length;
                    break;
                case 3:
                    memmove(&address, args, sizeof(ULONG));
                    memmove(&length, &args[sizeof(ULONG)], sizeof(ULONG));
                    pattern = args[sizeof(ULONG) + sizeof(ULONG)];
                    ret = api_sram_clear(address, length, pattern);
                    optlen = 0;
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region//==PEDK==
        case psDEV_PED:
            switch(FuncNum)
            {
                ULONG   length;
                ULONG   address;
                UINT    mode;

                case 1:
                    ret = PED_ReadKeyMode();
                    break;
                case 2:
                    //prototype:UINT8	PED_GetPin( UINT16 tout, UINT8 *amt )
                    //argument order:tout(1B) *amt(amt[0] + 1B)
                    ret = PED_GetPin(*args, args + 1);
                    break;
                case 3:
                    //prototype:UINT8	PED_MSKEY_GenPinBlock( UINT8 mode, UINT8 index, UINT8 *pan, UINT8 *epb )
                    //argument order:mode(1B) index(1B) *pan(32B) *epb(8B)
                    ret = PED_MSKEY_GenPinBlock(*args, *(args + 1), args + 2, args + 34);
                    optlen = 8;
                    break;
                case 4:
                    //prototype:UINT8	PED_DUKPT_GenPinBlock( UINT8 mode, UINT8 *pan, UINT8 *epb, UINT8 *ksn )
                    //argument order:mode(1B) *pan(32B) *epb(8B) *ksn(10B)
                    ret = PED_DUKPT_GenPinBlock(*args, args + 1, args + 33, args + 41);
                    optlen = 18;
                    break;
                case 5:
                    //prototype:UINT8	PED_AESKEY_GenPinBlock( UINT8 mode, UINT8 index, UINT8 *pan, UINT8 *epb )
                    //argument order:mode(1B) index(1B) *pan(32B) *epb(16B)
                    ret = PED_AESKEY_GenPinBlock(*args, *(args + 1), args + 2, args + 34);
                    optlen = 16;
                    break;
                case 6:
                    //prototype:UINT8	PED_MSKEY_GenMAC( UINT16 mode, UINT8 index, UINT8 *icv, UINT16 length, UINT8 *data, UINT8 *mac )
                    //argument order:mode(2B) index(1B) *icv(8B) length(2B) *data(length B) *mac(8B)
                    // length = *(args + 10);
                    // ret = PED_MSKEY_GenMAC(*args, *(args + 1), args + 2, *(args + 10), args + 12, args + 12 + length);
                    // optlen = 8;
                    memmove(&mode, args, 2);
                    length = *(args + 11);
                    ret = PED_MSKEY_GenMAC(*args, *(args + 2), args + 3, *(args + 11), args + 13, args + 13 + length);
                    optlen = 16;
                    break;
                case 7:
                    //prototype:UINT8	PED_DUKPT_GenMAC( UINT8 mode, UINT8 *icv, UINT16 length, UINT8 *data, UINT8 *mac, UINT8 *ksn )
                    //argument order:mode(1B) *icv(8B) length(2B) *data(length B) *mac(8B) *ksn(10B)
                    length = *(args + 9);
                    ret = PED_DUKPT_GenMAC(*args, args + 1, *(args + 9), args + 11, args + 11 + length, args + 11 + length + 8);
                    optlen = 18;
                    break;
                case 8:
                    //prototype:UINT8	PED_CAPK_GetKeyHeader( UINT8 index, UINT8 *pkh )
                    //argument order:index(1B) *pkh(63B)
                    ret = PED_CAPK_GetKeyHeader(*args, args + 1);
                    optlen = 63;
                    break;
                case 9:
                    //prototype:UINT8	PED_CAPK_SelectKey( UINT8 pki, UINT8 *rid, UINT8 *pkh, UINT8 *index )
                    //argument order:pki(1B) *rid(5B) *pkh(29B) *index(4B)
                    ret = PED_CAPK_SelectKey(*args, args + 1, args + 1 + 5, args + 1 + 5 + 29);
                    optlen = 29 + 4;
                    break;
                case 10:
                    //prototype:UINT8	PED_AES_DUKPT_GenPinBlock( UINT16 mode, UINT8 *pan, UINT8 *epb, UINT8 *ksn )
                    //argument order:mode(2B) *pan(32B) *epb(16B) *ksn(12B)
                    memmove(&mode, args, 2);
                    ret = PED_AES_DUKPT_GenPinBlock(mode, args + 2, args + 34, args + 50);
                    optlen = 28;
                    break;
                case 11:
                    //prototype:UINT8	PED_AES_DUKPT_GenMAC( UINT16 mode, UINT8 *icv, UINT16 length, UINT8 *data, UINT8 *mac, UINT8 *ksn )
                    //argument order:mode(2B) *icv(8B) length(2B) *data(length B) *mac(16B) *ksn(12B)
                    memmove(&mode, args, 2);
                    length = *(args + 10);
                    ret = PED_AES_DUKPT_GenMAC(mode, args + 2, *(args + 10), args + 12, args + 12 + length, args + 12 + length + 16);
                    optlen = 28;
                    break;
                case 12:
#if 0
                    //prototype:UINT8	PED_GenPinBlock( UINT8 *pb )
                    //argument order:*pb(8B)
                    ret = PED_GenPinBlock(args);
                    optlen = 8;
#else
                    //prototype:UCHAR CVM_VerifyPIN( UCHAR mode, UCHAR *pindata, UCHAR *pkm, UCHAR *pke )
                    //argument order:mode(1B) *pkm(2 + pkm[0] + pkm[1] * 256 B) *pke(3B)
                    length = 2 + args[1] + args[2] * 256;
                    ret = CVM_VerifyPIN(*args, NULLPTR, args + 1, args + 1 + length);
#endif
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region//==RTC==
        case psDEV_RTC:
            switch(FuncNum)
            {
                case 1:
                    ret = api_rtc_open();
                    break;
                case 2:
                    ret = api_rtc_close(*args);
                    break;
                case 3:
                    ret = api_rtc_getdatetime(*args, args + 1);
                    optlen = 12;
                    break;
                case 4:
                    ret = api_rtc_setdatetime(*args, args + 1);
                    break;
                case 5:
                    ret = api_rtc_sync_NTP_switch(*args, *(args + 1));
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region//==FLS==
        case psDEV_FLS:
            switch(FuncNum)
            {
                ULONG   address;
                ULONG   length;
                UCHAR*  data;
                UCHAR   pattern;

                case 1:
                    memmove(&address, args, sizeof(ULONG));
                    memmove(&length, &args[sizeof(ULONG)], sizeof(ULONG));
                    data = &args[sizeof(ULONG) + sizeof(ULONG)];
                    ret = FLASH_WriteData((ULONG)address, data, length);
                    optlen = 0;
                    break;
                case 2:
                    memmove(&address, args, sizeof(ULONG));
                    memmove(&length, &args[sizeof(ULONG)], sizeof(ULONG));
                    data = &args[sizeof(ULONG) + sizeof(ULONG)];
                    ret = FLASH_ReadData((ULONG)address, data, length);
                    optlen = length;
                    break;
                case 3:
                    memmove(&address, args, sizeof(ULONG));
                    ret = FLASH_EraseSector((ULONG)address);
                    optlen = 0;
                    break;
                default:
                    break;
            }
            break;  
#pragma endregion
#pragma region//==SPI==
        case psDEV_SPI:
            switch(FuncNum)
            {
                UINT    length;
                UCHAR*  data;
                UCHAR   pattern;

                case 1:
                    memmove(&length, args, sizeof(UINT));
                    data = &args[sizeof(UINT)];
                    ret = SPI_Transmit(length, data);
                    memmove(&args[sizeof(UINT) + length], data, length);
                    optlen = length;
                    break;
                case 2:
                    memmove(&length, args, sizeof(UINT));
                    data = &args[sizeof(UINT)];
                    ret = SPI_Transmit(length, data);
                    memmove(&args[sizeof(UINT) + length], data, length);
                    optlen = length;
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region//==MEM==
        case psDEV_MEM:
            switch(FuncNum)
            {
                ULONG               length;
                ULONG               address;
                UCHAR               NameLen;
                UCHAR               flag;
                UCHAR               SysName[50];
                UCHAR*              data;
                UCHAR               pattern;
                struct dev_mem_info info;

                //args order: info->size(4B) system_name length(1B) info->system_name(system_name_length B) info->base_address(4B)
                case 1:
                    memset(SysName, 0, sizeof(SysName));
                    memmove(&info.size, args, 4);
                    NameLen = args[4];
                    memmove(SysName, &args[5], NameLen);
                    info.system_name = SysName;
                    memmove(&info.base_address, &args[5 + NameLen], 4);
                    ret = mem_init((struct dev_mem_info *)&info);
                    optlen = 0;
                    break;
                //args order: info->size(4B) system_name length(1B) info->system_name(system_name_length B) info->base_address(4B) mem_add(4B) flag(1B) length(4B) *data(length B)
                case 2:
                    memset(SysName, 0, sizeof(SysName));
                    memmove(&info.size, args, 4);
                    NameLen = args[4];
                    memmove(SysName, &args[5], NameLen);
                    info.system_name = SysName;
                    memmove(&info.base_address, &args[5 + NameLen], 4);
                    memmove(&address, &args[5 + NameLen + 4], 4);
                    flag = args[5 + NameLen + 4 + 4];
                    memmove(&length, &args[5 + NameLen + 4 + 4 + 1], 4);
                    data = &args[5 + NameLen + 4 + 4 + 1 + 4];
                    // printf("address=0x%08lx length=0x%08lx flag=%d\n", address, length, flag);
                    ret = mem_read_or_write((struct dev_mem_info *)&info, address, length, data, flag);
                    // printf("info.base_address=0x%08lx \n", info.base_address);
                    optlen = flag ? 0 : length;
                    break;
                //args order: info->size(4B) system_name length(1B) info->system_name(system_name_length B) info->base_address(4B) mem_add(4B) length(4B) pattern(1B)
                case 3:
                    memset(SysName, 0, sizeof(SysName));
                    memmove(&info.size, args, 4);
                    NameLen = args[4];
                    memmove(SysName, &args[5], NameLen);
                    info.system_name = SysName;
                    memmove(&info.base_address, &args[5 + NameLen], 4);
                    memmove(&address, &args[5 + NameLen + 4], 4);
                    flag = args[5 + NameLen + 4 + 4];
                    memmove(&length, &args[5 + NameLen + 4 + 4 + 1], 4);
                    pattern = args[5 + NameLen + 4 + 4 + 1 + 4];
                    ret = mem_clear((struct dev_mem_info *)&info, address, length, pattern);
                    optlen = 0;
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
#pragma region//==NXP==
#ifdef PCD_PLATFORM_CLRC663
        //NXP_Function.c
        case psDEV_NXP:
            switch(FuncNum)
            {
                UINT    iptLen;
                UINT    optLen;
                UCHAR   *address;
                ULONG   LongValue;

                case 1:
                //prototype:void NXP_Load_RC663_Parameter_AS350(void)
                    ret = 0;
                    NXP_Load_RC663_Parameter_AS350();
                    break;
                case 2:
                //prototype:void NXP_Load_RC663_Parameter_TA(void)
                    ret = 0;
                    NXP_Load_RC663_Parameter_TA();
                    break;
                case 3:
                //prototype:UCHAR NXP_Read_Register_Serial(UCHAR *iptAddress, UINT iptLen, UCHAR *optData)
                //argument order:iptLen(2B) *iptAddress(iptLen B) *optData(iptLen B)
                    iptLen = *args + *(args + 1) * 256;
                    ret = NXP_Read_Register_Serial(address, iptLen, args + 2 + iptLen);
                    optlen = iptLen;
                    break;
                case 4:
                //prototype:UCHAR NXP_Read_FIFO(UINT iptLen, UCHAR *optData)
                //argument order:iptLen(2B) *optData(iptLen B)
                    iptLen = *args + *(args + 1) * 256;
                    ret = NXP_Read_FIFO(iptLen, args + 2);
                    optlen = iptLen;
                    break;
                case 5:
                //prototype:UCHAR NXP_Write_Register_Serial(UCHAR iptAddress, UINT iptLen, UCHAR *iptData)
                //argument order:iptLen(2B) iptAddress(1B) *iptData(iptLen B)
                    iptLen = *args + *(args + 1) * 256;
                    ret = NXP_Write_Register_Serial(*(args + 2), iptLen, args + 3);
                    break;
                case 6:
                //prototype:void void NXP_Check_Noise_Parameters(void)
                    ret = 0;
                    NXP_Check_Noise_Parameters();
                    break;
                case 7:
                //prototype:void NXP_Switch_Receiving_Serial(void)
                    ret = 0;
                    NXP_Switch_Receiving_Serial();
                    break;
                case 8:
                //prototype:void NXP_Switch_Receiving_Serial2(void)
                    ret = 0;
                    NXP_Switch_Receiving_Serial2();
                    break;
                case 9:
                //prototype:UCHAR NXP_Get_CLChipSerialNumber(UCHAR * optData)
                //argument order:optData(11B)
                    ret = NXP_Get_CLChipSerialNumber(args);
                    optlen = 11;
                    break;
                case 10:
                //prototype:void NXP_Set_Timer(ULONG tmrValue)
                //argument order:tmrValue(4B)
                    memmove(&LongValue, args, 4);
                    ret = NXP_Set_Timer(LongValue);
                    break;
                case 11:
                //prototype:void NXP_Switch_Receiving(void)
                    ret = 0;
                    NXP_Switch_Receiving();
                    break;
                case 12:
                //prototype:void NXP_Reset_NoiseParameter(void)
                    ret = 0;
                    NXP_Reset_NoiseParameter();
                    break;
                case 13:
                //prototype:UCHAR NXP_Receive_ExceptionProcessing(void)
                    ret = NXP_Receive_ExceptionProcessing();
                    break;
                case 14:
                //prototype:UCHAR NXP_Receive_ExceptionProcessing2(void)
                    ret = NXP_Receive_ExceptionProcessing2();
                    break;
                case 15:
                //prototype:UCHAR NXP_Receive(void)
                    ret = NXP_Receive();
                    break;
                case 16:
                //prototype:void NXP_Initialize_Reader(void)
                    ret = 0;
                    NXP_Initialize_Reader();
                    break;
                case 17:
                //prototype:void NXP_Load_Protocol_A(void)
                    ret = 0;
                    NXP_Load_Protocol_A();
                    break;
                case 18:
                //prototype:void NXP_Load_Protocol_B(void)
                    ret = 0;
                    NXP_Load_Protocol_B();
                    break;
                case 19:
                //prototype:void NXP_REQA_Send(ULONG rcvTimeout)
                    memmove(&LongValue, args, 4);
                    NXP_REQA_Send(LongValue);
                    ret = 0;
                    break;
                case 20:
                //prototype:UCHAR NXP_REQA_Receive(UINT *rcvLen, UCHAR *rcvATQA)
                //argument order:*rcvLen(2B) *rcvATQA(rcvLen B)
                    ret = NXP_REQA_Receive(args, args + 2);
                    memmove(&iptLen, args, 2);
                    optlen = 2 + iptLen; //*rcvLen(2B) *rcvATQA(rcvLen B)
                    break;
                case 21:
                //prototype:void NXP_WUPA_Send(ULONG rcvTimeout)
                    memmove(&LongValue, args, 4);
                    NXP_WUPA_Send(LongValue);
                    ret = 0;
                    break;
                case 22:
                //prototype:UCHAR NXP_WUPA_Receive(UINT *rcvLen, UCHAR *rcvATQA)
                //argument order:*rcvLen(2B) *rcvATQA(rcvLen B)
                    ret = NXP_WUPA_Receive(args, args + 2);
                    memmove(&iptLen, args, 2);
                    optlen = 2 + iptLen; //*rcvLen(2B) *rcvATQA(rcvLen B)
                    break;
                case 23:
                //prototype:void NXP_ANTICOLLISION_Send(UCHAR selCL, ULONG rcvTimeout)
                    memmove(&LongValue, args + 1, 4);
                    NXP_ANTICOLLISION_Send(*args, LongValue);
                    ret = 0;
                    break;
                case 24:
                //prototype:UCHAR NXP_ANTICOLLISION_Receive(UINT *rcvLen, UCHAR *rcvCLn)
                //argument order:*rcvLen(2B) *rcvCLn(rcvLen B)
                    ret = NXP_ANTICOLLISION_Receive(args, args + 2);
                    memmove(&iptLen, args, 2);
                    optlen = 2 + iptLen; //*rcvLen(2B) *rcvCLn(rcvLen B)
                    break;
                case 25:
                //prototype:void NXP_SELECT_Send(UCHAR selCL, UCHAR *selUID, UCHAR uidBCC, ULONG rcvTimeout)
                //argument order:selCL(1B) *selUID(4B) uidBCC(1B) rcvTimeout(4B)
                    memmove(&LongValue, args + 6, 4);
                    ret = 0;
                    NXP_SELECT_Send(*args, args + 1, *(args + 5), LongValue);
                    break;
                case 26:
                //prototype:UCHAR NXP_SELECT_Receive(UINT *rcvLen, UCHAR *rcvSAK)
                //argument order:*rcvLen(2B) *rcvSAK(rcvLen B)
                    ret = NXP_SELECT_Receive(args, args + 2);
                    memmove(&iptLen, args, 2);
                    optlen = 2 + iptLen; //*rcvLen(2B) *rcvSAK(rcvLen B)
                    break;
                case 27:
                //prototype:void NXP_RATS_Send(UCHAR ratPARAM, ULONG rcvTimeout)
                //argument order:ratPARAM(1B) rcvTimeout(4B)
                    memmove(&LongValue, args + 1, 4);
                    ret = 0;
                    NXP_RATS_Send(*args, LongValue);
                    break;
                case 28:
                //prototype:UCHAR NXP_RATS_Receive(UINT *rcvLen, UCHAR *rcvATS)
                //argument order:*rcvLen(2B) *rcvATS(rcvLen B)
                    ret = NXP_RATS_Receive(args, args + 2);
                    memmove(&iptLen, args, 2);
                    optlen = 2 + iptLen; //*rcvLen(2B) *rcvATS(rcvLen B)
                    break;
                case 29:
                //prototype:void NXP_PPS_Send(ULONG rcvTimeout)
                //argument order:  rcvTimeout(4B)
                    memmove(&LongValue, args, 4);
                    ret = 0;
                    NXP_PPS_Send(LongValue);
                    break;
                case 30:
                //prototype:UCHAR NXP_PPS_Receive(void)
                    ret = NXP_PPS_Receive();
                    break;
                case 31:
                //prototype:void NXP_HLTA_Send(ULONG rcvTimeout)
                //argument order:  rcvTimeout(4B)
                    memmove(&LongValue, args, 4);
                    ret = 0;
                    NXP_HLTA_Send(LongValue);
                    break;
                case 32:
                //prototype:void NXP_REQB_Send(ULONG rcvTimeout)
                //argument order:  rcvTimeout(4B)
                    memmove(&LongValue, args, 4);
                    ret = 0;
                    NXP_REQB_Send(LongValue);
                    break;
                case 33:
                //prototype:UCHAR NXP_REQB_Receive(UINT *rcvLen, UCHAR *rcvATQB)
                //argument order:*rcvLen(2B) *rcvATQB(rcvLen B)
                    ret = NXP_REQB_Receive(args, args + 2);
                    memmove(&iptLen, args, 2);
                    optlen = 2 + iptLen; //*rcvLen(2B) *rcvATQB(rcvLen B)
                    break;
                case 34:
                //prototype:void NXP_WUPB_Send(ULONG rcvTimeout)
                //argument order:  rcvTimeout(4B)
                    memmove(&LongValue, args, 4);
                    ret = 0;
                    NXP_WUPB_Send(LongValue);
                    break;
                case 35:
                //prototype:UCHAR NXP_WUPB_Receive(UINT *rcvLen, UCHAR *rcvATQB)
                //argument order:*rcvLen(2B) *rcvATQB(rcvLen B)
                    ret = NXP_WUPB_Receive(args, args + 2);
                    memmove(&iptLen, args, 2);
                    optlen = 2 + iptLen; //*rcvLen(2B) *rcvATQB(rcvLen B)
                    break;
                case 36:
                //prototype:void NXP_ATTRIB_Send(UCHAR *cmdATTRIB, ULONG rcvTimeout)
                //argument order:rcvTimeout(4B) *cmdATTRIB(11B)
                    memmove(&LongValue, args, 4);
                    NXP_ATTRIB_Send(args + 4, LongValue);
                    optlen = 11;
                    ret = 0;
                    break;
                case 37:
                //prototype:UCHAR NXP_ATTRIB_Receive(UINT *rcvLen, UCHAR *rcvATA)
                //argument order:*rcvLen(2B) *rcvATA(rcvLen B)
                    ret = NXP_ATTRIB_Receive(args, args + 2);
                    memmove(&iptLen, args, 2);
                    optlen = 2 + iptLen; //*rcvLen(2B) *rcvATA(rcvLen B)
                    break;
                case 38:
                //prototype:void NXP_HLTB_Send(UCHAR * iptPUPI, ULONG rcvTimeout)
                //argument order:rcvTimeout(4B) *iptPUPI(4B)
                    memmove(&LongValue, args, 4);
                    NXP_HLTB_Send(args + 4, LongValue);
                    ret = 0;
                    break;
                case 39:
                //prototype:UCHAR NXP_HLTB_Receive(UINT *rcvLen, UCHAR *rcvData)
                //argument order:*rcvLen(2B) *rcvData(rcvLen B)
                    ret = NXP_HLTB_Receive(args, args + 2);
                    memmove(&iptLen, args, 2);
                    optlen = 2 + iptLen; //*rcvLen(2B) *rcvData(rcvLen B)
                    break;
                case 40:
                //prototype:void NXP_DEP_Send(UCHAR crdType, UINT datLen, UCHAR *datBuffer, ULONG rcvTimeout)
                //argument order:crdType(1B) datLen(2B) rcvTimeout(4B) *datBuffer(datLen B)
                    memmove(&iptLen, args + 1, 2);
                    memmove(&LongValue, args + 3, 4);
                    ret = 0;
                    NXP_DEP_Send(*args, iptLen, args + 7, LongValue);
                    break;
                case 41:
                //prototype:UCHAR NXP_DEP_Receive(UINT *rcvLen, UCHAR *rcvData)
                //argument order:*rcvLen(2B) *rcvData(rcvLen B)
                    ret = NXP_DEP_Receive(args, args + 2);
                    memmove(&iptLen, args, 2);
                    optlen = 2 + iptLen; //*rcvLen(2B) *rcvData(rcvLen B)
                    break;
                case 42:
                //prototype:UCHAR NXP_LOADKEY(UCHAR *iptKey)
                    ret = NXP_LOADKEY(args);
                    break;
                case 43:
                //prototype:UCHAR NXP_AUTHENTICATION(UCHAR iptAutType, UCHAR iptAddress, UCHAR *iptUID)
                //argument order:iptAutType(1B) iptAddress(1B) *iptUID(4B)
                    ret = NXP_AUTHENTICATION(*args, *(args + 1), *(args + 2));
                    break;
                case 44:
                //prototype:UCHAR NXP_READ(UCHAR iptAddress, UCHAR *optData)
                //argument order:iptAddress(1B) *optData(16B)
                    ret = NXP_READ(*args, args + 1);
                    optlen = 16;
                    break;
                case 45:
                //prototype:UCHAR NXP_WRITE(UCHAR iptAddress, UCHAR *iptData)
                //argument order:iptAddress(1B) *iptData(16B)
                    ret = NXP_WRITE(*args, args + 1);
                    break;
                case 46:
                //prototype:UCHAR NXP_DECREMENT(UCHAR iptAddress, UCHAR *iptValue)
                //argument order:iptAddress(1B) *iptValue(4B)
                    ret = NXP_DECREMENT(*args, args + 1);
                    break;
                case 47:
                //prototype:UCHAR NXP_INCREMENT(UCHAR iptAddress, UCHAR *iptValue)
                //argument order:iptAddress(1B) *iptValue(4B)
                    ret = NXP_INCREMENT(*args, args + 1);
                    break;
                case 48:
                //prototype:UCHAR NXP_RESTORE(UCHAR iptAddress)
                //argument order:iptAddress(1B)
                    ret = NXP_RESTORE(*args);
                    break;
                case 49:
                //prototype:UCHAR NXP_TRANSFER(UCHAR iptAddress)
                //argument order:iptAddress(1B)
                    ret = NXP_TRANSFER(*args);
                    break;
                case 50:
                //prototype:UCHAR NXP_AV2_AUTHENTICATION_1ST(UCHAR iptAutType, UCHAR iptAddress, UCHAR *optData)
                //argument order:iptAutType(1B) iptAddress(1B) *optData(4B)
                    ret = NXP_AV2_AUTHENTICATION_1ST(*args, *(args + 1), args + 2);
                    optlen = 4;
                    break;
                case 51:
                //prototype:UCHAR NXP_AV2_AUTHENTICATION_2ND(UCHAR *iptData, UCHAR *optData)
                //argument order:iptData(9B) *optData(5B)
                    ret = NXP_AV2_AUTHENTICATION_2ND(args, args + 9);
                    optlen = 5;
                    break;
                case 52:
                //prototype:UCHAR NXP_AV2_TRANSCEIVE(UINT iptLen, UCHAR *iptData, UINT *optLen, UCHAR *optData)
                //argument order:iptLen(2B) *iptData(iptLen B) *optLen() *optData(optLen B)
                    memmove(&iptLen, args, 2);
                    ret = NXP_AV2_TRANSCEIVE(iptLen, args + 2, args + 2 + iptLen, args + 2 + iptLen + 2);
                    memmove(&optLen, args + 2 + iptLen, 2);
                    optlen = 2 + optLen;
                    break;
                default:
                    break;
            }
            break;
#else
    //PN5180 ctls lv1
#endif
#pragma endregion
#pragma region //==TLS==
        case psDEV_TLS:
            switch (FuncNum)
            {
                case 1:
                    ret = api_tls_cpkey();
                    break;
                case 2:
                    api_tls_rmkey();
                    break;
                default:
                    break;
            }
            break;
#pragma endregion
        default:
            break;
    }

  SHM_optArgLength(optlen);
  // if(PID==psDEV_NXP)
  // printf("ret=%d\n",ret);
  // if(ret!=0)
    // printf("Funcall PID=0x%x Funcnum=%d ret=%d\n",PID,FuncNum,ret);
  return ret;
}

UINT8   SHM_Handler()
{
#ifdef _DEBUG_SEMAPHORE_
    printf(">>>%s %d\n", __func__, __LINE__);
#endif

    OS_IPCSOCKET_HEADER socketHeader;
    OS_IPCSOCKET_BODY   socketBody;
    UINT32 buffersize;
    UINT32 retbuffersize;
    // if(CLEIENT_ARGS_SET != shm_data[0])
    //     return 0;//wait until client set flag
    socketHeader.PID = shm_data[1];
    socketHeader.Func_num = shm_data[2];
    socketHeader.Func_input_num = shm_data[3];
    socketHeader.Message_status = 1;
    memmove(&socketHeader.ArgsTotalSize, &shm_data[4], 4);
    shm_data[0] = IPC_FunctionCaller(socketHeader, &shm_data[8]);
    // if(socketHeader.PID == psDEV_OSSRAM)
    //     printf("@@@@@@@@@@@@%s %d\n", __func__, __LINE__);
#ifdef _DEBUG_SEMAPHORE_
    printf(">>>%s %d\n", __func__, __LINE__);
#endif
}

void    connectionTimeHandler()    //Added by Tammy
{
    int     fd = 0;
    connectionRecord record[TCP_MAX_CONNECTION_CNT];
    UINT32  connectionTime = 0;
    UINT32  currentTime = 0;
    struct	timespec now;
    UINT8   i;
    UINT8   needToCloseByKernel = FALSE;
    

    //open tcp driver
    fd = open("/dev/tcp", O_RDWR);
    if(fd < 0)
    {
        printf("open tcp failed\n");
        return;
    }
    
    while(1)
    {
        needToCloseByKernel = FALSE;

        ioctl(fd, IOCTL_TCP_GET_CONNECTION_TIME, record);
        
        clock_gettime(CLOCK_MONOTONIC, &now);
        currentTime = now.tv_sec + now.tv_nsec / 1000000000;
        // printf("current time = %lu\n", currentTime);
        
        for(i = 0 ; i < TCP_MAX_CONNECTION_CNT ; i++)
        {
            // printf("connection[%d] = %ld\n", i, record[i].connectionTime);

            if(record[i].connectionTime != 0)
            {
                if((currentTime - record[i].connectionTime) >= TCP_MAX_DURATION)
                {
                    // printf("It's time to disconnect.\n");
                    record[i].needToClose = 1;
                    needToCloseByKernel = TRUE;
                }
            }
        }
        
        if(needToCloseByKernel == TRUE)
            ioctl(fd, IOCTL_TCP_DISCONNECT, record);

        sleep(5);
    }
}

void    Lan_polling()    //Added by Tammy
{
    pthread_create(&lanThread, NULL, connectionTimeHandler, NULL);
}

UINT8   Verify_AP_Signature() //Modified by Tammy
{
    int fd = 0;
    int verifiedResult;

    //open RSA driver
    fd = open("/dev/RSA", O_RDWR);
    if(fd < 0)
    {
        printf("open RSA failed\n");
        return apiFailed;
    }

    //use IOCTL system call to request RSA driver to verify AP signature
    ioctl(fd, IOCTL_RSA_VERIFY, (int*)&verifiedResult);
    printf("verified result = %d\n", verifiedResult);

    //close RSA driver
    close(fd);

    if(verifiedResult == 0)
        return apiOK;
    
    return apiFailed;
}

#if 0
// ---------------------------------------------------------------------------
// FUNCTION: Test TSC (Touch Screen Controller) -- Touch Status.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#include "TouchHere_64x64-CCW.h"

void	TEST_TOUCH( void )
{
API_TSC_PARA		tscpara;
API_LCDTFT_GRAPH	graph;
API_LCDTFT_ICON		gicon;
UCHAR	dhn_tsc;
UCHAR	status;
UCHAR	result;

START:
	// show "TouchHere" icon at the lower center of display
	memset( (UCHAR *)&graph, 0x00, sizeof(graph) );
	graph.ID	= 0;
	graph.RGB	= 0;
	memmove( &graph.Bitmap, &bmTouchHere_64x64CCW, sizeof(GUI_BITMAP) );
	api_lcdtft_putgraphics( util_dhn_lcd, graph );

	memset( (UCHAR *)&gicon, 0x00, sizeof(gicon) );
	gicon.ID	= 0;
	gicon.Xleft	= 11;		//88;
	gicon.Ytop	= 64+64;	//238;
	gicon.Method	= 0;
	gicon.Width	= 64;
	gicon.Height	= 64;
	api_lcdtft_showICON( util_dhn_lcd, gicon );
	
	gicon.ID	= 0;
	gicon.Xleft	= 125;		//88;
	gicon.Ytop	= 64+64;	//238;
	gicon.Method	= 0;
	gicon.Width	= 64;
	gicon.Height	= 64;
	api_lcdtft_showICON( util_dhn_lcd, gicon );
	
	dhn_tsc = api_tsc_open( 0 );
	LIB_DispHexByte( 0, 0, dhn_tsc );
	
//	LIB_LCD_Puts( 1, 0, FONT0, strlen("HIT ANY KEY"), "HIT ANY KEY" );
//	LIB_WaitKey();
//	LIB_LCD_ClearRow( 1, 1, FONT0 );
	
	// waiting for the touch...
	memset( (UCHAR *)&tscpara, 0x00, sizeof( API_TSC_PARA ) );
//	do{
	while(1)
	  {
//	  if( LIB_GetKeyStatus() == apiReady )
//	    {
//	    LIB_WaitKey();
//	    break;
//	    }

//	LIB_WaitTime(3);
//	usleep(3000);
	
#if	1
	  tscpara.ID	= FID_TSC_STATUS_BUTTON;
	  tscpara.X	= 11;		//88;
	  tscpara.Y	= 64+64;	//238;
	  tscpara.Width	= 64;
	  tscpara.Height= 64;
	  tscpara.RxLen = 1;
	  status = 0;
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  if( (result == apiOK) && (status == 1) )
	    break;
#endif
//	LIB_WaitTime(3);
//	usleep(3000);

	  tscpara.ID	= FID_TSC_STATUS_BUTTON;
	  tscpara.X	= 125;		//88;
	  tscpara.Y	= 64+64;	//238;
	  tscpara.Width	= 64;
	  tscpara.Height= 64;
	  tscpara.RxLen = 1;
	  status = 0;
	  result = api_tsc_status( dhn_tsc, tscpara, &status );
	  if( (result == apiOK) && (status == 1) )
	    break;
	  
//	LIB_WaitTime(2);
//	usleep(1000);
	
//	  } while( !status );
	  }
	  
	LIB_DispHexByte( 0, 3, api_tsc_close( dhn_tsc ) );
	
		LIB_BUZ_Beep1();
		LIB_WaitKey();
		
		goto	START;
	
//	LIB_BUZ_Beep1();
}

// ---------------------------------------------------------------------------
void	TEST_TSC( void )
{
	while(1)
	{
	LIB_LCD_Cls();
	
	TEST_TOUCH();
	break;
	
//	LIB_WaitKey();
	}
}

// ---------------------------------------------------------------------------
// FUNCTION: Test TSC (Touch Screen Controller) -- Sign Pad.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
UCHAR	signbmp[(2*320*120/8) + 0x3E];

void	TEST_SIGNPAD( void )
{
API_TSC_PARA		tscpara;
API_LCDTFT_PARA		para;
API_LCDTFT_ICON		gicon;
API_LCDTFT_RECT		rect;
API_LCDTFT_WINBMP	winBMP;
API_GRAPH		dim;
UCHAR	sbuf[16];
UCHAR	dbuf[16];
UCHAR	status;
UCHAR	result;
ULONG	length = 0;
UCHAR	direction;
UCHAR	dhn_tsc;
UCHAR	dhn_prt;

	
	LIB_LCD_Cls();
	
//	dhn_prt = api_prt_open( prtThermal, 0 );
	dhn_tsc = api_tsc_open( 0 );
	
	direction = 0;
	while(1)
	{
	// SIGN PAD FUNCTION
	// set background RGB palette
	sbuf[0] = 255;
	sbuf[1] = 208;
	sbuf[2] = 139;

	memset( (UCHAR *)&rect, 0x00, sizeof(API_LCDTFT_RECT) );
	switch( direction )
	      {
	      case 0:
		   rect.Xstart	= 40+120;
		   rect.Xend	= 240;
		   rect.Ystart	= 0;
		   rect.Yend	= 320;
		   
	           break;
	      
	      case 1:
		   rect.Xstart	= 0;
		   rect.Xend	= 80;
		   rect.Ystart	= 0;
		   rect.Yend	= 320;
	  
	      	   break;
	      }
	rect.Palette[0]	= sbuf[0];
	rect.Palette[1]	= sbuf[1];
	rect.Palette[2]	= sbuf[2];
	api_lcdtft_fillRECT( util_dhn_lcd, rect );

	// display transaction AMOUNT
	para.Row = 0;
	para.Col = 0;
	para.Font = LCDTFT_FONT2;
	para.FontHeight = TFTLCD_FONT2_H;
	para.FontWidth = TFTLCD_FONT2_W;
	para.RGB = RGB_BPP16;
	
	para.FG_Palette[0] = 0x00;
	para.FG_Palette[1] = 0x00;
	para.FG_Palette[2] = 0x00;
	
	para.BG_Palette[0] = 255;
	para.BG_Palette[1] = 208;
	para.BG_Palette[2] = 139;

	para.Row = 7;
	para.Col = 0;
	dbuf[0] = 7;
	dbuf[1] = '$';
	dbuf[2] = '1';
	dbuf[3] = '0';
	dbuf[4] = '0';
	dbuf[5] = '.';
	dbuf[6] = '0';
	dbuf[7] = '0';
	if( direction == 0 )
	  SIGNPAD_lcdtft_putstring( util_dhn_lcd, para, dbuf, CCW_90 );
	else
	  SIGNPAD_lcdtft_putstring( util_dhn_lcd, para, dbuf, CCW_270 );  

	status = 0;
	do{
	  // start signpad function
	  memset( (UCHAR *)&tscpara, 0x00, sizeof( API_TSC_PARA ) );
	  tscpara.RFU[1] = direction;
	  
	  tscpara.ID = FID_TSC_SIGNPAD;
          
	  tscpara.X	= 0;
	  tscpara.Y	= 40;
	  tscpara.Width	= 320;
	  tscpara.Height= 120;
          
	  tscpara.RxLen = 1;
	  tscpara.RFU[0] = status;
	  tscpara.RFU[1] = direction;
	  
	  tscpara.Timeout = 100*30;	// 30 sec
	  
	  status = 0;
	  result = api_tsc_signpad( dhn_tsc, tscpara, &status, sbuf );
	  } while( (status == SIGN_STATUS_PROCESSING) || (status == SIGN_STATUS_CLEAR) );
	
	if( status == SIGN_STATUS_ROTATE )
	  {
	  switch( direction )
	        {
	        case 0:
	             direction = 1;
	             break;
	             
	        case 1:
	             direction = 0;
	             break;
	        }
	  }
	else
	  break;
	} // while(1)
	
	LIB_BUZ_Beep1();

	// get signature image
	memset( (UCHAR *)&tscpara, 0x00, sizeof( API_TSC_PARA ) );

	tscpara.ID	= FID_TSC_GET_SIGNATURE;
	tscpara.X	= 0;
	tscpara.Y	= 40;

	tscpara.Width	= 320;
	tscpara.Height	= 120;

	tscpara.RxLen = (tscpara.Width * tscpara.Height)/8;
	tscpara.RFU[0] = 2;		// 0=RAW, 1=WinBMP, 2=(WinBMP + RAW)
	tscpara.RFU[1] = direction;	// 0=CCW_0, 1=CCW_180
	
	status = 0;
	api_tsc_getsign( dhn_tsc, tscpara, &status, signbmp, &length );

	LIB_LCD_Cls();

	// write formated image to the display memory
	memset( (UCHAR *)&winBMP, 0x00, sizeof(winBMP) );
	winBMP.ID = 0;
	
	switch( direction )
	      {
	      case 0:
	      	   winBMP.CCWdegrees = CCW_180;
	      	   break;
	      
	      case 1:
	      	   winBMP.CCWdegrees = CCW_0;
	      	   break;
	      }
	
	winBMP.Method = WINBMP_SIGN_FRAME;
	winBMP.FG_Palette[0] = 0x00;
	winBMP.FG_Palette[1] = 0x00;
	winBMP.FG_Palette[2] = 0x00;
	winBMP.BG_Palette[0] = 0xFF;
	winBMP.BG_Palette[1] = 0xFF;
	winBMP.BG_Palette[2] = 0xFF;
	api_lcdtft_putwinbmp( util_dhn_lcd, &winBMP, signbmp );

	// show it
	memset( (UCHAR *)&gicon, 0x00, sizeof(gicon) );
	gicon.ID	= 0;

	gicon.Xleft	= 40;
	gicon.Ytop	= 0;

	gicon.Method	= 0;
	gicon.Width	= winBMP.Width[0];
	gicon.Height	= winBMP.Height[0];

	api_lcdtft_showICON( util_dhn_lcd, gicon );

#if	0
	// print out signature?
	if( tscpara.RFU[0] == 2 )
	  {
	  dim.Xleft  = 0;
	  dim.Ytop   = 0;
	  dim.Width  = tscpara.Width;
	  dim.Height = tscpara.Height;
	  dim.Method = 0;
	  api_prt_putgraphics( dhn_prt, dim, &signbmp[length] );
	  
	  sbuf[0] = 4;
	  sbuf[1] = 0x0A;
	  sbuf[2] = 0x0A;
	  sbuf[3] = 0x0A;
	  sbuf[4] = 0x0C;
	  api_prt_putstring( dhn_prt, FONT0, sbuf );
	  }
#endif

	LIB_WaitKey();
	
//	api_prt_close( dhn_prt );
	api_tsc_close( dhn_tsc );
}

// ---------------------------------------------------------------------------
void	TEST_SPEAKER( void )
{
	LIB_LCD_Cls();
	
	LIB_LCD_Puts( 0, 0, FONT0, strlen("TEST SPEAKER"), "TEST SPEAKER" );
//	LIB_LCD_Puts( 2, 0, FONT0, strlen("HIT ANY KEY TO START..."), "HIT ANY KEY TO START..." );
	
	system("amixer set 'Playback' 255");
	system("amixer cset numid=14 1");
	system("amixer cset numid=13 127");
	
//	LIB_LCD_Puts( 2, 0, FONT0, strlen("HIT ANY KEY TO EXIT..."), "HIT ANY KEY TO EXIT..." );
	
	while(1)
	{
//	if( LIB_GetKeyStatus() == apiReady )
//	  {
//	  if( LIB_WaitKey() == 'x' )
//	  return;
//	  }
	  
//	LIB_WaitTime( 200 );
	
	LIB_LCD_Puts( 2, 0, FONT0, strlen("PLAYBACK..."), "PLAYBACK..." );
	
//	LIB_WaitTime( 100 );
	
//	system("aplay -D plughw:0,0 /usr/share/sounds/alsa/sample12s.wav -v");

	system("aplay -D plughw:0,0 /usr/share/sounds/alsa/InsertCard.wav -v");	
	LIB_WaitKey();	
	system("aplay -D plughw:0,0 /usr/share/sounds/alsa/InProcessing.wav -v");
	
//	LIB_BUZ_Beep1();
	LIB_LCD_Puts( 2, 0, FONT0, strlen("HIT ANY KEY TO CONTINUE..."), "HIT ANY KEY TO CONTINUE..." );
	
	if( LIB_WaitKey() == 'x' )
	  break;
	  
	LIB_LCD_Puts( 2, 0, FONT0, strlen("                           "), "                           " );
	}
}

// --------------------------------------------------------------------------
void	TEST_TIMER( void )
{
UCHAR	msg_TIMER[] =	{"TIMER"};
UCHAR	dhn_tim1 = 0;
UCHAR	dhn_tim2 = 0;
UCHAR	dhn_tim3 = 0;
UCHAR	dhn_tim4 = 0;
UCHAR	dhn_tim5 = 0;
UCHAR	dhn_tim6 = 0;
UCHAR	dhn_tim7 = 0;
UCHAR	dhn_tim8 = 0;

UINT	old_time1 = 0;
UINT	new_time1 = 0;
UINT	old_time2 = 0;
UINT	new_time2 = 0;
UINT	old_time3 = 0;
UINT	new_time3 = 0;
UINT	old_time4 = 0;
UINT	new_time4 = 0;
UINT	old_time5 = 0;
UINT	new_time5 = 0;
UINT	old_time6 = 0;
UINT	new_time6 = 0;
UINT	old_time7 = 0;
UINT	new_time7 = 0;
UINT	old_time8 = 0;
UINT	new_time8 = 0;


	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_TIMER), (UCHAR *)msg_TIMER );
	
	dhn_tim1 = api_tim_open( 50 );
	dhn_tim2 = api_tim_open( 100 );
	dhn_tim3 = api_tim_open( 150);
	dhn_tim4 = api_tim_open( 200);
//	dhn_tim5 = api_tim_open( 140 );
//	dhn_tim6 = api_tim_open( 160 );
//	dhn_tim7 = api_tim_open( 180 );
//	dhn_tim8 = api_tim_open( 200 );
	
//	dhn_tim9 = api_tim_open3( 100, 1, 0xFFFFFFFF );
//	dhn_tim10= api_tim_open3( 200, 1, 0xFFFFFFFF );
	
	UT_DispHexByte( 1, 0,  dhn_tim1 );
	UT_DispHexByte( 1, 3,  dhn_tim2 );
	UT_DispHexByte( 1, 6,  dhn_tim3 );
	UT_DispHexByte( 1, 9,  dhn_tim4 );
//	UT_DispHexByte( 1, 12, dhn_tim5 );
//	UT_DispHexByte( 1, 15, dhn_tim6 );
//	UT_DispHexByte( 1, 18, dhn_tim7 );
//	UT_DispHexByte( 1, 21, dhn_tim8 );
	
	
	while(1)
	{
	if( LIB_GetKeyStatus() == apiReady )
	  {
	  LIB_WaitKey();
	  break;
	  }
	    
	LIB_DispHexByte( 3, 0, api_tim_gettick( dhn_tim1, &new_time1 ) );
	if( new_time1 != old_time1 )
	  {
	  LIB_DispHexByte( 3, 12, (new_time1 & 0xff00) >> 8 );
	  LIB_DispHexByte( 3, 15, (new_time1 & 0x00ff) );
	  
	  old_time1 = new_time1;
	  }

	LIB_DispHexByte( 4, 0, api_tim_gettick( dhn_tim2, &new_time2 ) );
	if( new_time2 != old_time2 )
	  {
	  LIB_DispHexByte( 4, 12, (new_time2 & 0xff00) >> 8 );
	  LIB_DispHexByte( 4, 15, (new_time2 & 0x00ff) );
	  
	  old_time2 = new_time2;
	  }

	LIB_DispHexByte( 5, 0, api_tim_gettick( dhn_tim3, &new_time3 ) );
	if( new_time3 != old_time3 )
	  {
	  LIB_DispHexByte( 5, 12, (new_time3 & 0xff00) >> 8 );
	  LIB_DispHexByte( 5, 15, (new_time3 & 0x00ff) );
	  
	  old_time3 = new_time3;
	  }
	  
	LIB_DispHexByte( 6, 0, api_tim_gettick( dhn_tim4, &new_time4 ) );
	if( new_time4 != old_time4 )
	  {
	  LIB_DispHexByte( 6, 12, (new_time4 & 0xff00) >> 8 );
	  LIB_DispHexByte( 6, 15, (new_time4 & 0x00ff) );
	  
	  old_time4 = new_time4;
	  }

	}
	
	api_tim_close( dhn_tim1 );
	api_tim_close( dhn_tim2 );
	api_tim_close( dhn_tim3 );
	api_tim_close( dhn_tim4 );
//	api_tim_close( dhn_tim5 );
//	api_tim_close( dhn_tim6 );
//	api_tim_close( dhn_tim7 );
//	api_tim_close( dhn_tim8 );
}

// --------------------------------------------------------------------------
void	TEST_TIMER2( void )
{
UCHAR	msg_TIMER[] =	{"TIMER"};

UINT32	status = 0xFFFFFFFF;
API_AUX	pAux;
UINT8	dhn_aux = 0;
UINT8	dhn_tim = 0;
UINT8	sbuf[128];
UINT8	dbuf[300];
UINT8	result = 0;
UINT16	tick1, tick2;
UINT16	timeout = 60;

UINT16	i;
UINT16	crc;
UINT16	len;
UINT8	data;


	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, strlen(msg_TIMER), (UCHAR *)msg_TIMER );
	
	// === Open and Turn on 2DR ===
	pAux.Mode = auxBYPASS;
	pAux.Baud = COM_9600 + COM_CHR8 + COM_NOPARITY + COM_STOP1;
	pAux.Tob = 10;	// 100ms
	pAux.Tor = 10;	// 100ms
	pAux.Acks = 0;
	pAux.Resend = 0;

//	dhn_aux = api_aux_open( COM0, pAux );
	dhn_aux = api_aux_open( COM1, pAux );
	if( dhn_aux != apiOutOfService )
	  {
	  //  Verify QRC connection (MODEL1: EM3085)
	  // -> '?'
	  // <- '!'
	  
	  while( api_aux_txready( dhn_aux ) != apiReady );
	  
	  sbuf[0] = 1;
	  sbuf[1] = 0;
	  sbuf[2] = '?';
	  api_aux_txstring( dhn_aux, sbuf );
	
	  while( api_aux_txready( dhn_aux ) != apiReady );

	  // 2016-01-06, waiting for response
	  dhn_tim = api_tim_open( 10 );		// in 100ms unit
	  api_tim_gettick( dhn_tim, (UCHAR *)&tick1 );
	  result = 0;
	  
	  do{
	    api_tim_gettick( dhn_tim, (UCHAR *)&tick2 );

	    sbuf[0] = 0;
	    sbuf[1] = 0;
	    if( api_aux_rxready( dhn_aux, sbuf ) == apiReady )
	      {
	      result = TRUE;
	      break;
	      }
	    } while( (tick2 - tick1) < 10 );	// 1 sec timeout


	  if( result )
	    {
	    // verify response
	    memset( sbuf, 0x00, sizeof(sbuf) );
	    api_aux_rxstring( dhn_aux, sbuf );
	    if( (sbuf[0] != 1) || (sbuf[1] != 0) || (sbuf[2] != '!') )
	      {
	      goto VERIFY_MODEL2;
	      }
	    else
	      {
	      status = 0x00;	// MODEL-1 verified
	      }
	    }
	  }

VERIFY_MODEL2:
	
	LIB_BUZ_Beep1();
	LIB_DispHexByte( 1, 0, result );
	LIB_DispHexByte( 2, 0, status );
	
	api_tim_close( dhn_tim );
	api_aux_close( dhn_aux );
	
	LIB_WaitKey();
}

// --------------------------------------------------------------------------
void	PrintString( UCHAR fontid )
{
UCHAR	i;
UCHAR 	result;
UCHAR	status;
UCHAR	str[] =	{ "01234567890123456789012345678901" };
UCHAR	sbuf[34];

	
	// --- print strings and check status ---	
	for( i=1; i<=32; i++ )
	   {
	   sbuf[0] = i + 1;	// length of (string + LF)
	   memmove( &sbuf[1], str, i ); // string
	   sbuf[i+1] = 0x0A;	// line feed
	   
	   result = UTIL_PrintString( FALSE, fontid, sbuf, (UCHAR *)&status );
	   if( !result )
	     break;	// exception occurs
	   }
	   
	// the last procedure to complete printer task
	if( result )
	  {
	  sbuf[0] = 6;
	  memset( &sbuf[1], 0x0A, 5 );// 5 addtional LFs
	  sbuf[6] = 0x0C;		// last character shall be Form Feed
	  
	  UTIL_PrintString( TRUE, fontid, sbuf, (UCHAR *)&status );
	  // check "status" here if needed
	  }

}

// --------------------------------------------------------------------------
void	TEST_PRINTER( void )
{
UCHAR	key;
char	msg_TEST_PRINTER[]	=	{"TEST PRINTER"};
char	msg_CANCEL_TO_EXIT[]	=	{"CANCEL TO EXIT"};
char	msg_PLEASE_SELECT[]	=	{"PLEASE SELECT:"};
char	msg_1_PRINT_STRING[]	=	{"1-PRINT STRING"};
char	msg_2_PRINT_BITMAP[]	=	{"2-PRINT BITMAP"};
char	msg_3_PRINT_BOTH[]	=	{"3-PRINT STR & BMP"};


	UTIL_ClearScreen();
	
	UTIL_PutStr( 0, 0, FONT1+attrREVERSE, strlen(msg_TEST_PRINTER), (UCHAR *)msg_TEST_PRINTER );
	
	UTIL_PutStr( 2, 0, FONT0, strlen(msg_1_PRINT_STRING), (UCHAR *)msg_1_PRINT_STRING );
	UTIL_PutStr( 3, 0, FONT0, strlen(msg_2_PRINT_BITMAP), (UCHAR *)msg_2_PRINT_BITMAP );
	UTIL_PutStr( 4, 0, FONT0, strlen(msg_3_PRINT_BOTH)  , (UCHAR *)msg_3_PRINT_BOTH );
	UTIL_PutStr( 7, 0, FONT0, strlen(msg_PLEASE_SELECT), (UCHAR *)msg_PLEASE_SELECT );
	
	UTIL_OpenPrinter();
	
	do{
	  key = LIB_WaitKey();
	  
	  UTIL_PutStr( 7, 0, FONT0, strlen(msg_CANCEL_TO_EXIT), (UCHAR *)msg_CANCEL_TO_EXIT );
	  
	  if( key == '1' )
	    {
	    PrintString( FONT0 );
//	    PrintString( FONT1 );
	    }
	    
//	  if( key == '2' )
//	    PrintBitmap();
	    
//	  if( key == '3' )
//	    PrintStringBitmap();
	    
	  } while( key != 'x' );	// exit if "CANCEL" key pressed	
	
	UTIL_ClosePrinter();
	
}

// --------------------------------------------------------------------------
void	TEST_SYSAPI( void )
{
UCHAR	buffer[32];

FILE    *UIDfp=NULL;
UCHAR	*UID1_path="/sys/fsl_otp/HW_OCOTP_CFG0";
UCHAR	*UID2_path="/sys/fsl_otp/HW_OCOTP_CFG1";
UCHAR	ret;


ST:
	LIB_LCD_Cls();
	LIB_LCD_Puts( 0, 0, FONT0, strlen("API_SYS"), (UCHAR *)"API_SYS" );
	if( LIB_WaitKey() == 'x' )
	  return;
	
	memset( buffer, 0x00, sizeof(buffer) );
	LIB_DispHexByte( 1, 0, api_sys_info( SID_TerminalSerialNumber, buffer ) );
	LIB_DumpHexData( 0, 2, 17, buffer );
	
	memset( buffer, 0x00, sizeof(buffer) );
	LIB_DispHexByte( 5, 0, api_sys_info( SID_McuSN, buffer ) );
	LIB_DumpHexData( 0, 6, 9, buffer );
	goto ST;
	

	UIDfp=fopen(UID1_path,"rb+");
	if( UIDfp )
	  {
	  ret = fread( buffer, 1, 10, UIDfp );
	  if( ret >= 10 )
	    {
	    LIB_DumpHexData( 0, 2, 10, buffer );
	    LIB_DumpHexData( 1, 2, 10, buffer );
	    }
	    
	  fclose(UIDfp);
	  }
	
	memset( buffer, 0x00, sizeof(buffer) );
	UIDfp=fopen(UID2_path,"rb+");
	if( UIDfp )
	  {
	  ret = fread( buffer, 1, 10, UIDfp );
	  if( ret >= 10 )
	    {
	    LIB_DumpHexData( 0, 5, 10, buffer );
	    LIB_DumpHexData( 1, 5, 10, buffer );
	    }
	    
	  fclose(UIDfp);
	  }
	
	goto ST;
}

// --------------------------------------------------------------------------
extern	void	DPA_main( void );

void	TEST_DPA( void )
{
	DPA_main();
}

// ---------------------------------------------------------------------------
// FUNCTION: To show string "Hello World" in color.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void	DisplayStr_HELLO_WORLD( void )
{
char	str[] = {"Hello World"};
UCHAR	dbuf[64];
API_LCDTFT_PARA para;


	memset( (UCHAR *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
	
	para.RGB = RGB_BPP16;
	para.Row = 2;
	para.Col = 4;
	para.Font = LCDTFT_FONT1;
	para.FontHeight = TFTLCD_FONT1_H;
	para.FontWidth = TFTLCD_FONT1_W;
	
	para.FG_Palette[0] = 0xFF; // RED
	para.FG_Palette[1] = 0x00;
	para.FG_Palette[2] = 0x00;
	para.BG_Palette[0] = 0x00; // GREEN
	para.BG_Palette[1] = 0xFF;
	para.BG_Palette[2] = 0x00;
	dbuf[0] = strlen(str);
	memmove( &dbuf[1], str, strlen(str) );
	api_lcdtft_putstring( 0, para, dbuf );
}

// ---------------------------------------------------------------------------
// FUNCTION: To show BMP Logo. (240x120, with black background palette)
//
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
#include "helloworld_240x120-CCW.h"

void	DisplayBmp_HELLO_WORLD( void )
{
API_LCDTFT_PARA		para;
API_LCDTFT_GRAPH	graph;
API_LCDTFT_ICON		gicon;


	// clear screen
	memset( (UCHAR *)&para, 0x00, sizeof(API_LCDTFT_PARA) );
	para.RGB = RGB_BPP16;
	para.Row = 0xFFFF;
	para.BG_Palette[0] = 0x00; // BLACK
	para.BG_Palette[1] = 0x00;
	para.BG_Palette[2] = 0x00;

	api_lcdtft_clear( 0, para );
	
	// show LOGO at center of display
	memset( (UCHAR *)&graph, 0x00, sizeof(graph) );
	graph.ID	= 0;
	graph.RGB	= 0;
	memmove( &graph.Bitmap, &bmhelloworld_240x120CCW, sizeof(GUI_BITMAP) );
	api_lcdtft_putgraphics( util_dhn_lcd, graph );

	memset( (UCHAR *)&gicon, 0x00, sizeof(gicon) );
	gicon.ID	= 0;
	gicon.Xleft	= 0;
	gicon.Ytop	= 100;
	gicon.Method	= 0;
	gicon.Width	= 240;
	gicon.Height	= 120;
	api_lcdtft_showICON( 0, gicon );
}

// ---------------------------------------------------------------------------
ULONG	g_DEBUG = 0;

void	TEST_LCD( void )
{
ULONG	i;
UCHAR	row;	// row #
UCHAR	col;	// column #
UCHAR	pattern[] =  {"012345678901234567890123456789"};
UCHAR	pattern2[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
UCHAR	sbuf[64];
UCHAR	dbuf[64];
UCHAR	font;
UCHAR	max_row;
UCHAR	max_col;
API_LCDTFT_PARA para;


	g_DEBUG = 1;
//	goto LCD_CLR_WT;

LCD_X6:
	LIB_LCD_Cls();
	if( LIB_WaitKey() == 'x' )
	  return;
#if	0
	// test X6 extended font (FONT0 8x16 --> 16x32)
//	para.RGB = RGB_BPP16;
//	para.Row = 0;
//	para.Col = 0;
//	para.Font = LCDTFT_FONT1;
//	para.FontHeight = TFTLCD_FONT1_H;
//	para.FontWidth = TFTLCD_FONT1_W;
//	
//	para.FG_Palette[0] = 0x00; // B
//	para.FG_Palette[1] = 0x00;
//	para.FG_Palette[2] = 0x00;
//	para.BG_Palette[0] = 0xFF; // W
//	para.BG_Palette[1] = 0xFF;
//	para.BG_Palette[2] = 0xFF;
//	
//	dbuf[0] = 1;
//	dbuf[1] = 'H';
//	//memmove( &dbuf[1], pattern, 20 );
//	api_lcdtft_putstring( 0, para, dbuf );
//	
//	LIB_WaitKey();

	para.RGB = RGB_BPP16;
	para.Row = 0;
	para.Col = 0;
	para.Font = LCDTFT_FONT1+attrISO;
	para.FontHeight = TFTLCD_FONT1_H;
	para.FontWidth = TFTLCD_FONT1_W;
	
	para.FG_Palette[0] = 0x00; // B
	para.FG_Palette[1] = 0x00;
	para.FG_Palette[2] = 0x00;
	para.BG_Palette[0] = 0xFF; // W
	para.BG_Palette[1] = 0xFF;
	para.BG_Palette[2] = 0xFF;
	
	dbuf[0] = 2;
	dbuf[1] = 'H';
	dbuf[2] = 'H';
	//memmove( &dbuf[1], pattern, 20 );
	api_lcdtft_putstring( 0, para, dbuf );
#else
	for( row=0; row<13; row++ )
	{
	para.RGB = RGB_BPP16;
	para.Row = row;
	para.Col = 0;
	para.Font = LCDTFT_FONT1;
	para.FontHeight = TFTLCD_FONT1_H;
	para.FontWidth = TFTLCD_FONT1_W;
	
	para.FG_Palette[0] = 0x00; // B
	para.FG_Palette[1] = 0x00;
	para.FG_Palette[2] = 0x00;
	para.BG_Palette[0] = 0xFF; // W
	para.BG_Palette[1] = 0xFF;
	para.BG_Palette[2] = 0xFF;
	
	dbuf[0] = 20;
	memmove( &dbuf[1], pattern, 20 );
	api_lcdtft_putstring( 0, para, dbuf );
	}
	
	LIB_WaitKey();
	LIB_LCD_Cls();
	
	for( row=0; row<10; row++ )
	{
	para.RGB = RGB_BPP16;
	para.Row = row;
	para.Col = 0;
	para.Font = LCDTFT_FONT1+attrISO;
	para.FontHeight = TFTLCD_FONT1_H;
	para.FontWidth = TFTLCD_FONT1_W;
	
	para.FG_Palette[0] = 0x00; // B
	para.FG_Palette[1] = 0x00;
	para.FG_Palette[2] = 0x00;
	para.BG_Palette[0] = 0xFF; // W
	para.BG_Palette[1] = 0xFF;
	para.BG_Palette[2] = 0xFF;
	
	dbuf[0] = 15;
	memmove( &dbuf[1], pattern, 15 );
	api_lcdtft_putstring( 0, para, dbuf );
	}
#endif
	LIB_WaitKey();
	goto LCD_X6;
	
	
	
	
LCD_0:
	LIB_LCD_Cls();
	
	DisplayStr_HELLO_WORLD();
	
	LIB_WaitKey();
	LIB_LCD_Cls();
	
	DisplayBmp_HELLO_WORLD();
	
	LIB_WaitKey();
	goto LCD_0;
	
	font = FONT0;
	max_row = 20;
	max_col = 30;
	
//	font = FONT1;
//	max_row = 13;
//	max_col = 20;

	// test clear row with color
	row = 0;
	col = 1;
	for( i=0; i<max_row; i++ )
	{
	memset( &para, 0x00, sizeof(API_LCDTFT_PARA) );
	para.Row = row;
	para.Col = col;	// cnt
	para.Font = font;
	para.FontHeight = TFTLCD_FONT0_H;
	para.FontWidth  = TFTLCD_FONT0_W;
//	para.BG_Palette[0] = 0xFF;	// white
//	para.BG_Palette[1] = 0xFF;
//	para.BG_Palette[2] = 0xFF;
	
	para.BG_Palette[0] = 0x00;	// black
	para.BG_Palette[1] = 0x00;
	para.BG_Palette[2] = 0x00;
	 
	api_lcdtft_clear( 0, para );
	
	row += 1;
	LIB_WaitKey();
	}

	LIB_WaitKey();
	LIB_LCD_Cls();
	
	row = 0;
	col = 1;
	for( i=0; i<max_row; i++ )
	{
	memset( &para, 0x00, sizeof(API_LCDTFT_PARA) );
	para.Row = row;
	para.Col = col;	// cnt
	para.Font = font;
	para.FontHeight = TFTLCD_FONT0_H;
	para.FontWidth  = TFTLCD_FONT0_W;
	
	para.BG_Palette[0] = 0xFF;	// RED
	para.BG_Palette[1] = 0x00;
	para.BG_Palette[2] = 0x00;
	 
	api_lcdtft_clear( 0, para );
	
	row += 1;
	LIB_WaitKey();
	}
	
	LIB_WaitKey();
	LIB_LCD_Cls();
	
	row = 0;
	col = 1;
	for( i=0; i<max_row; i++ )
	{
	memset( &para, 0x00, sizeof(API_LCDTFT_PARA) );
	para.Row = row;
	para.Col = col;	// cnt
	para.Font = font;
	para.FontHeight = TFTLCD_FONT0_H;
	para.FontWidth  = TFTLCD_FONT0_W;
	
	para.BG_Palette[0] = 0x00;	// GREEN
	para.BG_Palette[1] = 0xFF;
	para.BG_Palette[2] = 0x00;
	 
	api_lcdtft_clear( 0, para );
	
	row += 1;
	LIB_WaitKey();
	}
	
	LIB_WaitKey();
	LIB_LCD_Cls();
	
	row = 0;
	col = 1;
	for( i=0; i<max_row; i++ )
	{
	memset( &para, 0x00, sizeof(API_LCDTFT_PARA) );
	para.Row = row;
	para.Col = col;	// cnt
	para.Font = font;
	para.FontHeight = TFTLCD_FONT0_H;
	para.FontWidth  = TFTLCD_FONT0_W;
	
	para.BG_Palette[0] = 0x00;	// BLUE
	para.BG_Palette[1] = 0x00;
	para.BG_Palette[2] = 0xFF;
	 
	api_lcdtft_clear( 0, para );
	
	row += 1;
	LIB_WaitKey();
	}
	
	goto LCD_0;
	
	// test CLEAR ROW
	col = 0;
	for( row=0; row<max_row; row++ )
	   {
	   LIB_LCD_Puts( row, col, font, max_col, pattern );
	   }
	   
//	LIB_LCD_Puts( 0, 0, FONT0, 30, pattern );
//	LIB_LCD_Puts( 1, 0, FONT0, 30, pattern );
	
	LIB_WaitKey();
	
	row = 0;
	col = 2;
	for( i=0; i<max_row/2; i++ )
	{
	// test api_lcdtft_clear()
	memset( &para, 0x00, sizeof(API_LCDTFT_PARA) );
	para.Row = row;
	para.Col = col;	// cnt
	para.Font = font;
	para.FontHeight = TFTLCD_FONT0_H;
	para.FontWidth  = TFTLCD_FONT0_W;
	para.BG_Palette[0] = 0xFF;	// white
	para.BG_Palette[1] = 0xFF;
	para.BG_Palette[2] = 0xFF;
	 
	api_lcdtft_clear( 0, para );
	
	row += 2;
	LIB_WaitKey();
	}
	
	LIB_WaitKey();
	goto LCD_0;
	
	para.Row = 1;
	para.Col = 1;	// cnt
	para.Font = FONT0;
	para.FontHeight = TFTLCD_FONT0_H;
	para.FontWidth  = TFTLCD_FONT0_W;
	para.BG_Palette[0] = 0xFF;	// white
	para.BG_Palette[1] = 0xFF;
	para.BG_Palette[2] = 0xFF;
	 
	api_lcdtft_clear( 0, para );
	LIB_WaitKey();
	goto LCD_0;

LCD_CLR_WT:
	// test clear write (FONT0, FONT1)
	LIB_LCD_Cls();
	
	col = 0;
	
#if	0
	for( row=0; row<20; row++ )
	   {
	   LIB_LCD_Puts( row, col, FONT0, 30, pattern );
	   }
#endif
  
	for( row=0; row<13; row++ )
	   {
	   LIB_LCD_Puts( row, col, FONT1, 20, pattern );
	   }
	
	LIB_WaitKey();
	
	memset( (UCHAR *)&para, 0x00, sizeof(API_LCDTFT_PARA) );

#if	0
	for( row=0; row<20; row++ )
	{
	para.RGB = RGB_BPP16;
	para.Row = row;
	para.Col = 2;
	para.Font = LCDTFT_FONT0 + attrCLEARWRITE;
	para.FontHeight = TFTLCD_FONT0_H;
	para.FontWidth = TFTLCD_FONT0_W;
	
	para.FG_Palette[0] = 0xFF; // R
	para.FG_Palette[1] = 0x00;
	para.FG_Palette[2] = 0x00;
	para.BG_Palette[0] = 0xFF; // W
	para.BG_Palette[1] = 0xFF;
	para.BG_Palette[2] = 0xFF;
	
	dbuf[0] = strlen(pattern2);
	memmove( &dbuf[1], pattern2, strlen(pattern2) );
	api_lcdtft_putstring( 0, para, dbuf );
	
	LIB_WaitKey();
	}
#endif

	for( row=0; row<13; row++ )
	{
	para.RGB = RGB_BPP16;
	para.Row = row;
	para.Col = 2;
	para.Font = LCDTFT_FONT1 + attrCLEARWRITE + attrREVERSE;
	para.FontHeight = TFTLCD_FONT1_H;
	para.FontWidth = TFTLCD_FONT1_W;
	
	para.FG_Palette[0] = 0xFF; // R
	para.FG_Palette[1] = 0x00;
	para.FG_Palette[2] = 0x00;
	para.BG_Palette[0] = 0xFF; // W
	para.BG_Palette[1] = 0xFF;
	para.BG_Palette[2] = 0xFF;
	
	dbuf[0] = 16;
	memmove( &dbuf[1], pattern2, 16 );
	api_lcdtft_putstring( 0, para, dbuf );
	
	LIB_WaitKey();
	}
	
	LIB_WaitKey();
	goto LCD_CLR_WT;
	
	// test pixel col
LCD_1:
	col = 0;
	row = 0;
	
	for( i=0; i<4; i++ )
	   {
	   LIB_LCD_Cls();

	   sbuf[0] = 'H';
	   LIB_LCD_Puts( col++, row++, FONT0+attrPIXCURSOR, 1, sbuf );	// FONT0
	
	   LIB_WaitKey();
	   }
	
	LIB_WaitKey();

	col = 0;
	row = 0;
	
	for( i=0; i<4; i++ )
	   {
	   LIB_LCD_Cls();

	   sbuf[0] = 'H';
	   LIB_LCD_Puts( col++, row++, FONT1+attrPIXCURSOR, 1, sbuf );	// FONT1
	
	   LIB_WaitKey();
	   }

	LIB_WaitKey();

	col = 0;
	row = 0;
	
	for( i=0; i<4; i++ )
	   {
	   LIB_LCD_Cls();

	   sbuf[0] = 0xA4;
	   sbuf[1] = 0xA4;
	   LIB_LCD_Puts( col++, row++, FONT2+attrPIXCURSOR, 2, sbuf );	// FONT2
	
	   LIB_WaitKey();
	   }
	
	LIB_WaitKey();
	goto LCD_1;
//ST0:
//	LIB_WaitKey();
//	printf("\r\TEST_LCD\r\n");
//	goto ST0;
	
ST:
	LIB_LCD_Cls();
	
	LIB_LCD_Puts( 0, 0, FONT0, 30, pattern );
	LIB_WaitKey();
	goto TEST_FONT1;
	
	// test FONT0
	col = 0;
	for( row=0; row<20; row++ )
	   {
	   LIB_LCD_Puts( row, col, FONT0, 30, pattern );
	   LIB_WaitKey();
	   }
	   
//	LIB_WaitKey();
//	goto ST;
	   
TEST_FONT1:
	LIB_LCD_Cls();
	
	LIB_LCD_Puts( 0, 0, FONT1, 20, pattern );
	LIB_WaitKey();
	goto TEST_FONT2;
	
	// test FONT1
	col = 0;
	for( row=0; row<13; row++ )
	   {
	   LIB_LCD_Puts( row, col, FONT1, 20, pattern );
	   LIB_WaitKey();
	   }
	   
	LIB_WaitKey();
	goto ST;
	
TEST_FONT2:	// Chinese
	LIB_LCD_Cls();
/*
TEST_FONT2_1:
	sbuf[0] = 0xA4;
	sbuf[1] = 0xA4;
	LIB_LCD_Puts( row++, col++, FONT2, 2, sbuf );
	if( col >= 20 )
	  col = 0;
	if( row >= 13 )
	  {
	  row = 0;
	  LIB_WaitKey();
	  goto TEST_FONT2;
	  }
	
	LIB_WaitKey();
	goto TEST_FONT2_1;

	sbuf[0] = 0xA4;
	sbuf[1] = 0xA4;
	sbuf[2] = 0xA4;
	sbuf[3] = 0xE5;
	LIB_LCD_Puts( 0, 0, FONT2, 4, sbuf );
	
	LIB_WaitKey();
	
	sbuf[0] = 0xA4;
	sbuf[1] = 0xA4;
	sbuf[2] = 0xA4;
	sbuf[3] = 0xE5;
	LIB_LCD_Puts( 1, 1, FONT2, 4, sbuf );
	
	LIB_WaitKey();
	
	sbuf[0] = 'A';
	sbuf[1] = 'B';
	sbuf[2] = 'C';
	sbuf[3] = 'D';
	LIB_LCD_Puts( 0, 1, FONT1, 4, sbuf );
	
	LIB_WaitKey();
	goto TEST_FONT2;
*/

	sbuf[0] = 'A';
	sbuf[1] = 'B';
	sbuf[2] = 'C';
	sbuf[3] = 'D';
	LIB_LCD_Puts( 0, 0, FONT1, 4, sbuf );
	
	LIB_WaitKey();
	
	sbuf[0] = 0xA4;
	sbuf[1] = 0xA4;
	sbuf[2] = 'A';
	sbuf[3] = 0xA4;
	sbuf[4] = 0xE5;
	sbuf[5] = 'b';
	LIB_LCD_Puts( 1, 1, FONT2, 6, sbuf );
	
	LIB_WaitKey();
	
	sbuf[0] = 'B';
	sbuf[1] = 0xA4;
	sbuf[2] = 0xA4;
	sbuf[3] = 0xA4;
	sbuf[4] = 0xE5;
	sbuf[5] = 'b';
	LIB_LCD_Puts( 2, 2, FONT2, 6, sbuf );
	
	LIB_WaitKey();
	
	goto ST;
	
}

// ---------------------------------------------------------------------------
static	void system_set_mac( void )
{
UCHAR	mac_b[6];
UCHAR	mac_s[18];

	
	api_sys_genMAC( mac_b, mac_s );
	
//	LIB_LCD_Cls();
//	LIB_DumpHexData( 0, 2, 6, mac_b );
//	LIB_DumpHexData( 1, 3, 17, mac_s );
	
//	system("ifconfg eth0 down");
//	system("ifconfg eth0 hw ether ...");
//	system("ifconfg eth0 up");
}
#endif

#if 0
static UINT8    RollBackUboot()
{
    char* command_kobs = "/usr/bin/kobs-ng init -x -v --chip_0_device_path=/dev/mtd0 /usr/bin/uboot";
    LIB_LCD_Cls();
    system(command_kobs);
    api_sys_reset(0);
    return 0;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: setup ISO format for PIN block. (used for test)
// INPUT   : none.
// OUTPUT  : ISO format.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
#if 0
UINT8	TEST_set_iso_format()
{
    UINT8   msg_setISOFormat[] = {"SET ISO FORMAT"};
    UINT8	msg_ISO0[] = {"0-ISO0"};
    UINT8	msg_ISO1[] = {"1-ISO1"};
    UINT8	msg_ISO3[] = {"3-ISO3"};
    UINT8	msg_ISO4[] = {"4-ISO4"};
    UINT8   result = TRUE;
    

    LIB_LCD_Cls();
    LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, strlen(msg_setISOFormat), (UINT8 *)msg_setISOFormat);

    LIB_DispHexByte(1, 0, g_iso_format);

    LIB_LCD_PutMsg(2, COL_LEFTMOST, FONT1, strlen(msg_ISO0), (UINT8 *)msg_ISO0);
    LIB_LCD_PutMsg(3, COL_LEFTMOST, FONT1, strlen(msg_ISO1), (UINT8 *)msg_ISO1);
    LIB_LCD_PutMsg(4, COL_LEFTMOST, FONT1, strlen(msg_ISO3), (UINT8 *)msg_ISO3);
    LIB_LCD_PutMsg(5, COL_LEFTMOST, FONT1, strlen(msg_ISO4), (UINT8 *)msg_ISO4);

    switch(LIB_WaitKey())
    {
        case 'x':
            result = FALSE;
            break;

        case '0':
            g_iso_format = 0;
            break;

        case '1':
            g_iso_format = 1;
            break;

        case '3':
            g_iso_format = 3;
            break;

        case '4':
            g_iso_format = 4;
            break;

        default:
            result = FALSE;
            break;
    }

    LIB_DispHexByte(1, 0, g_iso_format);

    LIB_WaitKey();

    return result;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: setup AES DUKPT derived working key for PIN block and MAC generation. (used for test)
// INPUT   : none.
// OUTPUT  : AES DUKPT working key type.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
#if 0
UINT8	TEST_AES_DUKPT_setWorkingKeyType(UINT16 *keyType)
{
    UINT8   msg_setWorkingKeyType[] = {"SET WORKING KEY TYPE"};
    UINT8	msg_2TDES[] = {"0-2TDEA"};
    UINT8	msg_3TDEA[] = {"1-3TDEA"};
    UINT8	msg_AES128[] = {"2-AES128"};
    UINT8	msg_AES192[] = {"3-AES192"};
    UINT8	msg_AES256[] = {"4-AES256"};
    UINT8   result = TRUE;
    

    LIB_LCD_Cls();
    LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, strlen(msg_setWorkingKeyType), (UINT8 *)msg_setWorkingKeyType);

    LIB_DispHexByte(1, 0, *keyType);

    LIB_LCD_PutMsg(2, COL_LEFTMOST, FONT1, strlen(msg_2TDES), (UINT8 *)msg_2TDES);
    LIB_LCD_PutMsg(3, COL_LEFTMOST, FONT1, strlen(msg_3TDEA), (UINT8 *)msg_3TDEA);
    LIB_LCD_PutMsg(4, COL_LEFTMOST, FONT1, strlen(msg_AES128), (UINT8 *)msg_AES128);
    LIB_LCD_PutMsg(5, COL_LEFTMOST, FONT1, strlen(msg_AES192), (UINT8 *)msg_AES192);
    LIB_LCD_PutMsg(6, COL_LEFTMOST, FONT1, strlen(msg_AES256), (UINT8 *)msg_AES256);

    switch(LIB_WaitKey())
    {
        case 'x':
            result = FALSE;
            break;

        case '0':
            *keyType = 0;
            break;

        case '1':
            *keyType = 1;
            break;

        case '2':
            *keyType = 2;
            break;

        case '3':
            *keyType = 3;
            break;

        case '4':
            *keyType = 4;
            break;

        default:
            result = FALSE;
            break;
    }

    LIB_DispHexByte(1, 0, *keyType);

    LIB_WaitKey();

    return result;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: setup key index (key slot#) for PIN block. (used for test)
// INPUT   : none.
// OUTPUT  : key index.
// RETURN  : TRUE
//           FALSE
// ---------------------------------------------------------------------------
#if 0
UINT8	TEST_set_key_index()
{
    UINT8   msg_setKeyIndex[] = {"SET KEY INDEX"};
    UINT8   buf[16];


    LIB_LCD_Cls();
    LIB_LCD_PutMsg(0, COL_LEFTMOST, FONT1, strlen(msg_setKeyIndex), (UINT8 *)msg_setKeyIndex);

    LIB_DispHexByte(1, 0, g_key_index);

    memset(buf, 0x00, sizeof(buf));

    if(LIB_GetNumKey(0, 0, '_', FONT1, 3, 2, buf) == FALSE)
        return FALSE;

    g_key_index = atoi(&buf[1]);
    if(g_key_index > 10)
        g_key_index = 0;

    LIB_DispHexByte(1, 0, g_key_index);

    LIB_WaitKey();

    return TRUE;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: construct an encrypted PIN block.
// INPUT   : pan    - full PAN digits or transaction field for ISO3.
//		              format: L-V(n)
//	         mod    - bit0~7: ISO format
//                    bit8~15: key type
//	         idx    - key slot index
// OUTPUT  : epb    - the encrypted pin block.
//		              format: LL-V(8/16)
//	         ksn    - key serial number for DUKPT if available.
//		              format: LL-V(12)
// RETURN  : apiOK
//	         apiFailed
// ---------------------------------------------------------------------------
#if 0
UCHAR	TEST_GenEncrypedPinBlock(UCHAR *pan, UCHAR *epb, UCHAR *ksn, UINT mod, UCHAR idx)
{
    UINT8   result;
    UINT8   epb2[16];
    // UINT8   ksn2[10];
    UINT8   ksn2[12];
    UINT8   mode;


    // memset(ksn, 0x00, 12);
    // memset(epb, 0x00, 10);
    memset(ksn, 0x00, 14);
    memset(epb, 0x00, 18);

    result = apiFailed;

    switch(api_ped_GetKeyMode())
    {
        case PED_KEY_MODE_MS:
            printf("key mode = MS\n");  // ==== [Debug] ====
            mode = mod & 0x00FF;
            result = api_ped_GenPinBlock_MSKEY(mode, idx, pan, epb2);
            if(result == apiOK)
            {
                epb[0] = 8;
                epb[1] = 0;
                memmove(&epb[2], epb2, 8);
            }
            break;
            
        case PED_KEY_MODE_DUKPT:
            printf("key mode = DUKPT\n");  // ==== [Debug] ====
            // result = api_ped_GenPinBlock_DUKPT(mod, pan, epb2, ksn2);
            // if(result == apiOK)
            // {
            //     epb[0] = 8;
            //     epb[1] = 0;
            //     memmove(&epb[2], epb2, 8);

            //     ksn[0] = 10;
            //     ksn[1] = 0;
            //     memmove(&ksn[2], ksn2, 10);
            // }

            result = api_ped_GenPinBlock_AES_DUKPT(mod, pan, epb2, ksn2);
            if(result == apiOK)
            {
                if(((mod & 0xFF00) >> 8) > 1)   // AES-128, AES-192, AES-256
                {
                    epb[0] = 16;
                    epb[1] = 0;
                    memmove(&epb[2], epb2, 16);
                }
                else    // TDES-128, TDES-192
                {
                    epb[0] = 8;
                    epb[1] = 0;
                    memmove(&epb[2], epb2, 8);
                }
                
                ksn[0] = 12;
                ksn[1] = 0;
                memmove(&ksn[2], ksn2, 12);
            }
            break;

        case PED_KEY_MODE_ISO4:
            printf("key mode = ISO4\n");  // ==== [Debug] ====
            mode = mod & 0x00FF;
            result = api_ped_GenPinBlock_AESKEY(mode, idx, pan, epb2);
            if(result == apiOK)
            {
                epb[0] = 16;
                epb[1] = 0;
                memmove(&epb[2], epb2, 16);
            }
            break;
    }
    
    return result;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: To generate MAC by using Master/Session key or DUKPT algorithm. (used for test)
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//	         apiFailed
// ---------------------------------------------------------------------------
#if 0
UINT8    TEST_GenMAC(void)
{
    UINT8   result = apiFailed;
    UINT8   buf[3];
    UINT8   num;
    UINT8   select = 0;
    UINT8   index = 0;
    UINT8	macAlgo = 0;
    UINT8	padding = 0;
    UINT8	ap;	//MAC_ALGx + MAC_PADx
    UINT8	iv[8] = {0};
    UINT8   testData[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    UINT8	mac[16];
    UINT8	ksn[10];
    UINT8   workingKeyType = 0;
    UINT16  mode = 0;
    UINT8   keyType;
    UINT8	idx;    // ==== [Debug] ====


    LIB_LCD_Cls();
    LIB_LCD_Puts(0, 0, FONT1, 25, (UINT8 *)"1. Generate MAC by MS key");
    LIB_LCD_Puts(1, 0, FONT1, 24, (UINT8 *)"2. Generate MAC by DUKPT");
    LIB_LCD_Puts(2, 0, FONT1, 7, (UINT8 *)"Select:");

    if(LIB_GetNumKey(0, 0, '_', FONT1, 3, 1, buf) == FALSE)
        return apiFailed;

    select = atoi(&buf[1]);
    
    if((select != 1) && (select != 2))
        return apiFailed;

    if(select == 1)
    {
        LIB_LCD_Cls();
        LIB_LCD_Puts(0, 0, FONT1, 27, (UINT8 *)"Please enter the key index.");
        LIB_LCD_Puts(1, 0, FONT1, 7, (UINT8 *)"Select:");

        if(LIB_GetNumKey(0, 0, '_', FONT1, 2, 2, buf) == FALSE)
            return apiFailed;

        index = atoi(&buf[1]);
        if(index > 10)
            return apiFailed;
    }

    LIB_LCD_Cls();
    LIB_LCD_Puts(0, 0, FONT1, 13, (UINT8 *)"MAC algorithm");
    LIB_LCD_Puts(1, 0, FONT1, 28, (UINT8 *)"1. ISO 16609 MAC Algorithm 1");
    LIB_LCD_Puts(2, 0, FONT1, 29, (UINT8 *)"2. ISO 9797-1 MAC Algorithm 1");
    LIB_LCD_Puts(3, 0, FONT1, 29, (UINT8 *)"3. ISO 9797-1 MAC Algorithm 3");
    LIB_LCD_Puts(4, 0, FONT1, 29, (UINT8 *)"4. ISO 9797-1 MAC Algorithm 5");
    LIB_LCD_Puts(5, 0, FONT1, 7, (UINT8 *)"Select:");

    if(LIB_GetNumKey(0, 0, '_', FONT1, 6, 1, buf) == FALSE)
        return apiFailed;

    num = atoi(&buf[1]);
    switch(num)
    {
        case 1:
            macAlgo = MAC_ISO16609;
            break;

        case 2:
            macAlgo = MAC_ALG1;
            break;

        case 3:
            macAlgo = MAC_ALG3;
            break;

        case 4: // only for AES key
            macAlgo = MAC_ALG5;
            break;
    }

    LIB_LCD_Cls();
    LIB_LCD_Puts(0, 0, FONT1, 14, (UINT8 *)"Padding method");
    LIB_LCD_Puts(1, 0, FONT1, 19, (UINT8 *)"1. Padding Method 1");
    LIB_LCD_Puts(2, 0, FONT1, 19, (UINT8 *)"2. Padding Method 2");
    LIB_LCD_Puts(3, 0, FONT1, 19, (UINT8 *)"3. Padding Method 3");
    LIB_LCD_Puts(4, 0, FONT1, 19, (UINT8 *)"4. Padding Method 4");
    LIB_LCD_Puts(5, 0, FONT1, 7, (UINT8 *)"Select:");

    if(LIB_GetNumKey(0, 0, '_', FONT1, 6, 1, buf) == FALSE)
        return apiFailed;

    num = atoi(&buf[1]);
    switch(num)
    {
        case 1:
            padding = MAC_PAD1;
            break;

        case 2:
            padding = MAC_PAD2;
            break;

        case 3:
            padding = MAC_PAD3;
            break;

        case 4:
            padding = MAC_PAD4;
            break;
    }

    ap = macAlgo | padding;
    printf("MAC_ALGx + MAC_PADx = 0x%02X\n", ap);

    if(select == 1)
    {
        result = api_ped_GenMAC_MSKEY(ap, index, iv, sizeof(testData), testData, mac);

        if(result == apiOK)
        {
            printf("Generate MAC by using Master/Session key algorithm : ");

            PED_PEK_GetKeyType(&keyType);
            if(keyType <= 2)
            {
                for(idx = 0 ; idx < 8 ; idx++)
                    printf("0x%02X ", mac[idx]);
            }
            else if(keyType > 2)
            {
                for(idx = 0 ; idx < 16 ; idx++)
                    printf("0x%02X ", mac[idx]);
            }

            printf("\n");
        }
    }
    else
    {
        TEST_AES_DUKPT_setWorkingKeyType(&workingKeyType);

        if(workingKeyType > 1)  // AES key
        {
            if(!((macAlgo == MAC_ALG5) && (padding = MAC_PAD4)))
                return apiFailed;
        }

        mode = (workingKeyType << 8) | ap;
        printf("mode = %04x\n", mode);

        // result = api_ped_GenMAC_DUKPT(ap, iv, sizeof(testData), testData, mac, ksn);
        result = api_ped_GenMAC_AES_DUKPT(mode, iv, sizeof(testData), testData, mac, ksn);

        if(result == apiOK)
        {
            printf("Generate MAC by using DUKPT algorithm : ");
            if(workingKeyType > 1)  // AES key
            {
                for(idx = 0 ; idx < 16 ; idx++)
                    printf("0x%02X ", mac[idx]);
            }
            else
            {
                for(idx = 0 ; idx < 8 ; idx++)
                    printf("0x%02X ", mac[idx]);
            }

            printf("\n");
        }
    }

    return result;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Generate surrogate PAN values and output outside of the device
//			 according to PCI DSS Requirements 3.4
// INPUT   : index - key slot index.
// OUTPUT  : none.
// RETURN  : TRUE
//			 FALSE
// ---------------------------------------------------------------------------
#if 0
UINT8	SRED_TEST_OutputPAN(UINT8 index)
{
    UINT8	pan[19];
    UINT8	panLen = 0;
    int		select;
    UINT8	dataOut[SRED_BUFFER_SIZE];
    UINT8	length = 0;
    UINT8	tag5A[12];
    UINT8	keyScheme;
    //UINT8	index;
    UINT8   buf[2];


    //Enter PAN
    LIB_LCD_Cls();
    LIB_LCD_Puts(0, 0, FONT1, 17, (UINT8 *)"Please enter PAN.");
    if(!SRED_ManualPanKeyEntry(pan, &panLen))
        return FALSE;

    LIB_LCD_Cls();

    //Construct Tag 5A
    tag5A[0] = 0x5A;	//T
    if((panLen % 2) != 0)
    {
        tag5A[1] = (panLen + 1) / 2;	//L
        SRED_Compress(&tag5A[2], pan, ((panLen + 1) / 2));	//V

        SRED_DBG_Put_String(16, (UINT8 *)"==== Tag 5A ====");	// ==== [Debug] ====
        SRED_DBG_Put_Hex((((panLen + 1) / 2) + 2), tag5A);	// ==== [Debug] ====
    }
    else
    {
        tag5A[1] = panLen / 2;	//L
        SRED_Compress(&tag5A[2], pan, (panLen / 2));	//V

        SRED_DBG_Put_String(16, (UINT8 *)"==== Tag 5A ====");	// ==== [Debug] ====
        SRED_DBG_Put_Hex(((panLen / 2) + 2), tag5A);	// ==== [Debug] ====
    }

    //Read key opearation mode
    keyScheme = PED_ReadKeyMode();

    //The key index for demo is set to 0
    //index = 0;

    //Encrypt PAN data and write it to NVSRAM
    if(SRED_Func_StoreDataElement(tag5A, tag5A[1], &tag5A[2], keyScheme, index) == FALSE)
    {
        LIB_LCD_Cls();
        LIB_LCD_PutMsg(1, COL_RIGHTMOST, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING_ERR), (UINT8 *)os_msg_PROCESSING_ERR);
        LIB_WaitTimeAndKey(200);
        return FALSE;
    }

    LIB_LCD_Cls();
    LIB_LCD_PutMsg(1, COL_RIGHTMOST, FONT1 + attrCLEARWRITE, sizeof(os_msg_WRITE_OK), (UINT8 *)os_msg_WRITE_OK);
    LIB_WaitTimeAndKey(200);

    //Get PAN data
    LIB_LCD_Cls();
    LIB_LCD_Puts(0, 0, FONT1, 28, (UINT8 *)"Select the output format of ");
    LIB_LCD_Puts(1, 0, FONT1, 9, (UINT8 *)"PAN data.");
    LIB_LCD_Puts(2, 0, FONT1, 21, (UINT8 *)"1. Plaintext PAN data");
    LIB_LCD_Puts(3, 0, FONT1, 30, (UINT8 *)"2. Encrypted sensitive middle digits");
    LIB_LCD_Puts(4, 0, FONT1, 9, (UINT8 *)"   digits");
    LIB_LCD_Puts(5, 0, FONT1, 22, (UINT8 *)"3. Ciphertext PAN data");

    if(LIB_GetNumKey(0, 0, '_', FONT1, 6, 1, buf) == FALSE)
        return FALSE;

    select = atoi(&buf[1]);

    if(!(select >= 1) && !(select <= 3))
        return FALSE;

    if(SRED_Func_GetDataElement((select - 1), tag5A, dataOut, &length) == FALSE)
    {
        LIB_LCD_Cls();
        LIB_LCD_PutMsg(1, COL_RIGHTMOST, FONT1 + attrCLEARWRITE, sizeof(os_msg_PROCESSING_ERR), (UINT8 *)os_msg_PROCESSING_ERR);
        LIB_WaitTimeAndKey(200);
        return FALSE;
    }

    LIB_LCD_Cls();
    LIB_LCD_PutMsg(1, COL_RIGHTMOST, FONT1 + attrCLEARWRITE, sizeof(os_msg_READ_OK), (UINT8 *)os_msg_READ_OK);
    LIB_WaitTimeAndKey(200);

    LIB_LCD_Cls();
    LIB_LCD_Puts(0, 0, FONT1, 8, (UINT8 *)"PAN data");
    LIB_DumpHexData(0, 1, length, dataOut);

    //Clear sensitive data
    memset(pan, 0x00, sizeof(pan));
    memset(dataOut, 0x00, sizeof(dataOut));

    return TRUE;
}
#endif

// ---------------------------------------------------------------------------
// FUNCTION: Write EDC private key (PEM format) to the secure file.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//			 apiFailed
// ---------------------------------------------------------------------------
UINT8    SecretInfo_PutEdcPrvKey()
{
    FILE    *fptr;
    UINT16  edcPrvKeySize = 0;
    UINT8   *edcPrvKey;


    fptr = fopen("/home/root/EDC_PRV_KEY.pem", "rb");
    if(fptr == NULL)
    {
        printf("open EDC_PRV_KEY.pem failed\n");
        return apiFailed;
    }

    fseek(fptr, 0, SEEK_END);
    edcPrvKeySize = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);

    edcPrvKey = malloc(edcPrvKeySize);
    if(edcPrvKey == NULL)
    {
        perror("unable to allocate required memory");
        fclose(fptr);
        return apiFailed;
    }
    
    printf("edcPrvKeySize = %d\n", edcPrvKeySize);
    OS_SECM_PutEdcPrvKey(0, 2, (UINT8 *)&edcPrvKeySize);

    fread(edcPrvKey, edcPrvKeySize, 1, fptr);
    OS_SECM_PutEdcPrvKey(2, edcPrvKeySize, edcPrvKey);

    fclose(fptr);
    memset(edcPrvKey, 0x00, edcPrvKeySize);
    free(edcPrvKey);

    return apiOK;
}

// ---------------------------------------------------------------------------
// FUNCTION: Read EDC private key from the secure file and store it in a PEM file.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//			 apiFailed
// ---------------------------------------------------------------------------
UINT8    SecretInfo_GetEdcPrvKey()
{
    FILE    *fptr;
    UINT16  edcPrvKeySize;
    UINT8   *edcPrvKey;
    int     ret;


    OS_SECM_GetEdcPrvKey(0, 2, (UINT8 *)&edcPrvKeySize);
    printf("edcPrvKeySize = %d\n", edcPrvKeySize);

    edcPrvKey = malloc(edcPrvKeySize);
    if(edcPrvKey == NULL)
    {
        perror("unable to allocate required memory");
        return apiFailed;
    }

    OS_SECM_GetEdcPrvKey(2, edcPrvKeySize, edcPrvKey);

    fptr = fopen("/home/root/EDC_PRV_KEY.pem", "wb+");
    if(fptr == NULL)
    {
        perror("open EDC_PRV_KEY.pem failed");
        memset(edcPrvKey, 0x00, edcPrvKeySize);
        free(edcPrvKey);
        return apiFailed;
    }

    ret = fwrite(edcPrvKey, edcPrvKeySize, 1, fptr);
    if(!ret)
    {
        perror("fwrite fail");
        fclose(fptr);
        memset(edcPrvKey, 0x00, edcPrvKeySize);
        free(edcPrvKey);
        return apiFailed;
    }
    else
    {
        sync();
        fclose(fptr);
        memset(edcPrvKey, 0x00, edcPrvKeySize);
        free(edcPrvKey);
        return apiOK;
    }
}

// ---------------------------------------------------------------------------
// FUNCTION: Delete EDC_PRV_KEY.pem.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    SecretInfo_DeleteEdcPrvKey()
{
    FILE *fp;


    fp = popen("rm /home/root/EDC_PRV_KEY.pem", "r");
    if(!fp)
        perror("Failed to remove EDC_PRV_KEY.pem");
    pclose(fp);
}

// ---------------------------------------------------------------------------
// FUNCTION: To check if EDC_PRV_KEY.pem exists.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : TRUE.
//           FALSE.
// ---------------------------------------------------------------------------
UINT8    SecretInfo_EdcPrvKeyExists()
{
    UINT16  edcPrvKeySize;


    OS_SECM_GetEdcPrvKey(0, 2, (UINT8 *)&edcPrvKeySize);
    printf("edcPrvKeySize = %d\n", edcPrvKeySize);

    if(edcPrvKeySize != 0)
        return TRUE;
    else
        return FALSE;
}

// ---------------------------------------------------------------------------
// FUNCTION: To generate RSA key pair via OpenSSL
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : apiOK
//			 apiFailed
// ---------------------------------------------------------------------------
UINT8   SecretInfo_GenRSAKeyPair()
{
    struct stat stbuf;
    int         ret = 0;
    RSA         *r = NULL;
    BIGNUM      *bne = NULL;
    BIO         *bp_public = NULL, *bp_private = NULL;
    int         bits = 2048;
    UINT32      e = RSA_F4;
    UINT8       msg_RSAExist[] = {"RSA key pair already exist."};
    UINT8	    msg_genRSAKey[] = {"RSA key pair are generating..."};


    LIB_LCD_Cls();
    LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_GEN_RSA_KEY_PAIR), (UINT8 *)os_msg_GEN_RSA_KEY_PAIR);
    
    //if EDC RSA key pair exist
    if((stat("/home/root/EDC_PUB_KEY.pem", &stbuf) == 0) &&
       (SecretInfo_EdcPrvKeyExists() == TRUE))
    {
        LIB_LCD_Puts(2, 4, FONT1, sizeof(msg_RSAExist) - 1, (UINT8 *)msg_RSAExist);

        if(LIB_WaitKeyMsgYesNoTO(5, COL_LEFTMOST, sizeof(os_msg_Q_RESET), (UINT8 *)os_msg_Q_RESET) == FALSE)
            return apiFailed;
    }    

    LIB_LCD_ClearRow(2, 4, FONT1);
    LIB_LCD_Puts(2, 4, FONT1, sizeof(msg_genRSAKey) - 1, (UINT8 *)msg_genRSAKey);

    //Generate RSA key
    bne = BN_new();
    ret = BN_set_word(bne, e);
    if(ret != 1)
        goto FREE_ALL;

    r = RSA_new();
    ret = RSA_generate_key_ex(r, bits, bne, NULL);
    if(ret != 1)
        goto FREE_ALL;
    
    //Save public key
    bp_public = BIO_new_file("/home/root/EDC_PUB_KEY.pem", "w+");
    ret = PEM_write_bio_RSAPublicKey(bp_public, r);
    if(ret != 1)
        goto FREE_ALL;
    
    //Save private key
    bp_private = BIO_new_file("/home/root/EDC_PRV_KEY.pem", "w+");
    ret = PEM_write_bio_RSAPrivateKey(bp_private, r, NULL, NULL, 0, NULL, NULL);

    //free
FREE_ALL:
    BIO_free_all(bp_public);
    BIO_free_all(bp_private);
    RSA_free(r);
    BN_free(bne);

    if(ret == 1)
    {
        ret = SecretInfo_PutEdcPrvKey();
        SecretInfo_DeleteEdcPrvKey();
        return ret;
    }
    else
        return apiFailed;
}

// ---------------------------------------------------------------------------
// FUNCTION: To sign message digest with private key.
// INPUT   : key  - a pointer to a private key using an EVP_PKEY structure.
//           msg  - data to be signed with private key.
//           mlen - length of data to be signed.
// OUTPUT  : sig  - signature.
//           slen - length of signature.
// RETURN  : 1    - success.
//			 0    - failure.
// ---------------------------------------------------------------------------
UINT8   doSign(EVP_PKEY *key, const UINT8 *msg, const size_t mlen, UINT8 **sig, size_t *slen)
{
    EVP_MD_CTX  *mdctx = NULL;
    UINT8       result = 0;


    //Create the Message Digest Context
    if(!(mdctx = EVP_MD_CTX_create()))
        goto ERROR;

    //Initialise the DigestSign operation - SHA-256 has been selected
    //as the message digest function in this example
    if(1 != EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, key))
        goto ERROR;

    //Call update with the message
    if(1 != EVP_DigestSignUpdate(mdctx, msg, mlen))
        goto ERROR;

    //Finalise the DigestSign operation
    //First call EVP_DigestSignFinal with a NULL sig parameter to
    //obtain the length of the signature. Length is returned in slen
    if(1 != EVP_DigestSignFinal(mdctx, NULL, slen))
        goto ERROR;
    //Allocate memory for the signature based on size in slen
    if(!(*sig = OPENSSL_malloc(*slen)))
        goto ERROR;
    //Obtain the signature
    if(1 != EVP_DigestSignFinal(mdctx, *sig, slen))
        goto ERROR;

    //Success
    result = 1;

ERROR:
    //Clean up
    if(*sig && !result)
        OPENSSL_free(*sig);
    if(mdctx)
        EVP_MD_CTX_destroy(mdctx);

    return result;
}

// ---------------------------------------------------------------------------
// FUNCTION: To sign message digest with private key.
// INPUT   : prvKeyFilePtr - a pointer to a FILE object.
//           data          - data to be signed with private key.
//           dataLen       - length of data to be signed.
// OUTPUT  : signature     - signature.
//           signatureLen  - length of signature.
// RETURN  : none.
// ---------------------------------------------------------------------------
void   SecretInfo_Sign(FILE *prvKeyFilePtr, UINT8 *data, UINT16 dataLen, UINT8 *signature, UINT16 *signatureLen)
{
    EVP_PKEY    *privateKey = NULL;
    UINT8       *sig = NULL;
    size_t      slen = 0;


    privateKey = PEM_read_PrivateKey(prvKeyFilePtr, NULL, NULL, NULL);

    if(!doSign(privateKey, data, dataLen, &sig, &slen))
        goto ERROR;

    memcpy(signature, sig, slen);
    *signatureLen = slen;

    OPENSSL_free(sig);
    sig = NULL;

ERROR:
    if(privateKey)
        EVP_PKEY_free(privateKey);
}

// ---------------------------------------------------------------------------
// FUNCTION: To verify the authenticity of the signature.
// INPUT   : pubKeyFilePtr - a pointer to a FILE object.
//           data          - data used to hash for authentication.
//           dataLen       - length of data.
//           signature     - signature to be verified with public key.
//           signatureLen  - length of signature to be verified with public key.
// OUTPUT  : authentic     - TRUE for valid signature.
//                         - FALSE for invalid signature.
// RETURN  : TRUE          - complete verification.
//           FALSE         - some error occur during verification.
// ---------------------------------------------------------------------------
UINT8   doVerify(EVP_PKEY *publicKey, UINT8 *data, UINT16 dataLen, UINT8 *signature, UINT16 signatureLen, UINT8 *authentic)
{
    EVP_MD_CTX  *mdctx = NULL;
    int         authStatus;


    *authentic = FALSE;

    mdctx = EVP_MD_CTX_create();
    
    if(EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, publicKey) <= 0)
    {
        return FALSE;
    }

    if(EVP_DigestVerifyUpdate(mdctx, data, dataLen) <= 0)
    {
        return FALSE;
    }

    authStatus = EVP_DigestVerifyFinal(mdctx, signature, signatureLen);
    if(authStatus == 1)
    {
        *authentic = TRUE;
        EVP_MD_CTX_destroy(mdctx);
        return TRUE;
    }
    else if(authStatus == 0)
    {
        *authentic = FALSE;
        EVP_MD_CTX_destroy(mdctx);
        return TRUE;
    }
    else
    {
        *authentic = FALSE;
        EVP_MD_CTX_destroy(mdctx);
        return FALSE;
    } 
}

// ---------------------------------------------------------------------------
// FUNCTION: To verify the authenticity of the signature.
// INPUT   : pubKeyFilePtr - a pointer to a FILE object.
//           data      - data used to hash for authentication.
//           dataLen   - length of data.
//           signature - signature to be verified with public key.
//           signatureLen - length of signature to be verified with public key.
// OUTPUT  : isAuthenticated - check if the signature matches a given message digest.
// RETURN  : none.
// ---------------------------------------------------------------------------
void   SecretInfo_Verify(FILE *pubKeyFilePtr, UINT8 *data, UINT16 dataLen, UINT8 *signature, UINT16 signatureLen, UINT8 *isAuthenticated)
{
    EVP_PKEY    *publicKey = NULL;
    UINT8       authentic = FALSE;
    UINT8       result = FALSE;


    publicKey = PEM_read_PUBKEY(pubKeyFilePtr, NULL, NULL, NULL);

    result = doVerify(publicKey, data, dataLen, signature, signatureLen, &authentic);
    *isAuthenticated = result & authentic;

    if(publicKey)
        EVP_PKEY_free(publicKey);
}

// ---------------------------------------------------------------------------
// FUNCTION: To write device audit trail.
// INPUT   : authStatus - authentication result.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    SecretInfo_writeAuditTrail(UINT8 authStatus)
{
    UINT8   dbuf[12];
    UINT8   rtc[17];
    UINT8   key;
    UINT8   buffer[13];
    UINT8   msg_title[] = {"Secret Info Audit Trail"};
    UINT8   msg_date_format[] = {"YY/MM/DD hh:mm:ss"};


    LIB_LCD_Cls();
    LIB_LCD_Puts(0, 0, FONT1, sizeof(msg_title) - 1, (UINT8 *)msg_title);

    //read current date & time
    api_rtc_getdatetime(dhn_rtc, dbuf);

    //show current date & time
    rtc[0] = dbuf[0];
    rtc[1] = dbuf[1];
    rtc[2] = '/';
    rtc[3] = dbuf[2];
    rtc[4] = dbuf[3];
    rtc[5] = '/';
    rtc[6] = dbuf[4];
    rtc[7] = dbuf[5];
    rtc[8] = ' ';
    rtc[9] = dbuf[6];
    rtc[10]= dbuf[7];
    rtc[11]= ':';
    rtc[12]= dbuf[8];
    rtc[13]= dbuf[9];
    rtc[14]= ':';
    rtc[15]= dbuf[10];
    rtc[16]= dbuf[11];
    LIB_LCD_Puts(2, 0, FONT1, 17, rtc);

    //update?
    LIB_LCD_Puts(4, 0, FONT1, sizeof(os_msg_Q_UPDATE) - 1, (UINT8 *)os_msg_Q_UPDATE);

    while(1)
    {
        key = LIB_WaitKey();
        if(key == KEY_OK)
        {
            LIB_LCD_Puts(2, 0, FONT1, sizeof(msg_date_format) - 1, (UINT8 *)msg_date_format);
            LIB_LCD_ClearRow(3, 2, FONT1);

            //keyin
            do {
                LIB_GetNumKey(0, NUM_TYPE_LEADING_ZERO, '_', FONT1, 4, 12, buffer);
            } while(TL_CheckDateTime(buffer) != TRUE);
            
            //update
            api_rtc_setdatetime(dhn_rtc, &buffer[1]);

            do {
                api_rtc_getdatetime(dhn_rtc, dbuf);

                //show new date & time
                rtc[0] = dbuf[0];
                rtc[1] = dbuf[1];
                rtc[2] = '/';
                rtc[3] = dbuf[2];
                rtc[4] = dbuf[3];
                rtc[5] = '/';
                rtc[6] = dbuf[4];
                rtc[7] = dbuf[5];
                rtc[8] = ' ';
                rtc[9] = dbuf[6];
                rtc[10]= dbuf[7];
                rtc[11]= ':';
                rtc[12]= dbuf[8];
                rtc[13]= dbuf[9];
                rtc[14]= ':';
                rtc[15]= dbuf[10];
                rtc[16]= dbuf[11];

                LIB_LCD_ClearRow(4, 1, FONT1);
                LIB_LCD_Puts(2, 0, FONT1, 17, rtc);
            } while(LIB_GetKeyStatus() == apiNotReady);

            LIB_WaitKey();
            break;
        }
        else
        {
            if(key == KEY_CANCEL)
                break;
        }
    }

    api_rtc_getdatetime(dhn_rtc, dbuf);

    OS_SECM_PutData(ADDR_DEVICE_AUTH_STATUS, 1, &authStatus);
    OS_SECM_PutData(ADDR_DEVICE_AUTH_DATE_TIME, 12, dbuf);
    
    LIB_LCD_ClearRow(4, 1, FONT1);
    LIB_LCD_Puts(4, 0, FONT1, sizeof(os_msg_WRITE_OK), (UINT8 *)os_msg_WRITE_OK);
}

// ---------------------------------------------------------------------------
// FUNCTION: To read device audit trail.
// INPUT   : none.
// OUTPUT  : none.
// RETURN  : none.
// ---------------------------------------------------------------------------
void    SecretInfo_readAuditTrail()
{
    UINT8   dbuf[12];
    UINT8   rtc[17];
    UINT8   i;
    UINT8   authStatus;
    UINT8   msg_title[] = {"Secret Info Audit Trail"};
    UINT8   msg_success[] = {"Validation Result: Success"};
    UINT8   msg_failure[] = {"Validation Result: Failure"};


    LIB_LCD_Puts(0, 0, FONT1, sizeof(msg_title) - 1, (UINT8 *)msg_title);

    OS_SECM_GetData(ADDR_DEVICE_AUTH_DATE_TIME, 12, dbuf);

    if(LIB_memcmpc(dbuf, 0x00, sizeof(dbuf)) == 0)
    {
        for(i = 0 ; i < sizeof(dbuf) ; i++)
            dbuf[i] += '0';
    }

    rtc[0] = dbuf[0];
    rtc[1] = dbuf[1];
    rtc[2] = '/';
    rtc[3] = dbuf[2];
    rtc[4] = dbuf[3];
    rtc[5] = '/';
    rtc[6] = dbuf[4];
    rtc[7] = dbuf[5];
    rtc[8] = ' ';
    rtc[9] = dbuf[6];
    rtc[10]= dbuf[7];
    rtc[11]= ':';
    rtc[12]= dbuf[8];
    rtc[13]= dbuf[9];
    rtc[14]= ':';
    rtc[15]= dbuf[10];
    rtc[16]= dbuf[11];
    LIB_LCD_Puts(2, 0, FONT1, 17, rtc);

    OS_SECM_GetData(ADDR_DEVICE_AUTH_STATUS, 1, &authStatus);
    if(authStatus == 1)
        LIB_LCD_Puts(3, 0, FONT1, sizeof(msg_success) - 1, (UINT8 *)msg_success);
    else
        LIB_LCD_Puts(3, 0, FONT1, sizeof(msg_failure) - 1, (UINT8 *)msg_failure);

    LIB_WaitKey();
}

static void DSS_main()
{
    UINT8         *text_AS350version = "AS350X6        V1.00";
    UINT8         *text_BSPversion = "READY      BSP 1.0.0";
    UINT8         *text_FunctionNumber = "Function Number:";
    UINT8         buf[4];
    UINT8         len = 0;
    UINT8         function_num;
    UINT8         result;
    struct stat   stbuf;    //Added by Tammy
    // ==== [Debug] secret information ====
    UINT16        i = 0;
    UINT8         uid[17];  //L-V
    UINT8         digest[SHA256_DIGEST_LENGTH];
    UINT16        length = 0;
    UINT8         signature[256] = {0};
    UINT8         isAuthenticated = FALSE;

    FILE          *prvKeyFilePtr;    // ==== [Debug] test ====
    FILE          *pubKeyFilePtr;    // ==== [Debug] test ====
    // ==== [Debug] secret information ====


#if 0   //used for PEDK test
    UINT8         accDek[PED_ACC_DEK_MSKEY_LEN];
    UINT8         fpe[PED_FPE_KEY_MSKEY_LEN];
    UINT8	      buffer[KEY_BUNDLE_LEN2];
    UINT8	      idx;
    UINT16	      tout = 60;  // 60 seconds
    UINT8         amt[] = {0x07, 0x24, 0x31, 0x30, 0x30, 0x2E, 0x30, 0x30};   // L-V, $100.00
    UINT8         pinblock[16];
    UINT8         pan[17] = {0x10, 0x34, 0x39, 0x33, 0x38, 0x31, 0x37, 0x30, 0x31, 0x30, 0x30, 0x30, 0x30, 0x31, 0x32, 0x31, 0x33};
    UINT8         epb[20];
    UINT8         ksn[12];
    UINT8         workingKeyType = 0;
    UINT16        mode = 0;
    UINT8	      workingKey[32]; // AES DUKPT test
    UINT8         pb[16] = {0x48, 0x12, 0x34, 0x56, 0x78, 0xaa, 0xaa, 0xaa, 0x54, 0x1b, 0x33, 0xfe, 0xb2, 0x33, 0x89, 0x9c};   // AES DUKPT test
    UINT8         pin[17] = {0x11, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};   // AES DUKPT test
#endif

    while(1)
    {
        LIB_LCD_Cls();


#ifdef	_DPA_TEST_
        for(UINT i = 0 ; i < 3 ; i++)
        {
            LIB_BUZ_Beep1();
            LIB_WaitTime(50);
        }
        if(LIB_WaitTimeAndKey(200) == 255)
            DPA_main();
#endif
    
        LIB_LCD_Puts(0, 0, FONT1, strlen(text_AS350version), (UINT8 *)text_AS350version);
        LIB_LCD_Puts(8-1, 0, FONT1, strlen(text_BSPversion), (UINT8 *)text_BSPversion);
        LIB_BUZ_Beep1();

        while(LIB_WaitKey() != 'y');    //only FUNC key accepted

        LIB_LCD_Cls();

        LIB_LCD_Puts(0, 0, FONT1, sizeof(os_msg_FUNCTION), (UINT8 *)os_msg_FUNCTION);

        if(LIB_GetNumKey(0, NUM_TYPE_DIGIT, '_', FONT1, 1, 2, buf) == FALSE)
            continue;

        len = buf[0];
        buf[len + 1] = 0x00;
        function_num = atoi((char *)&buf[1]);

        LIB_LCD_Cls();
        
        result = FALSE;
        switch(function_num)
        {
            case 1:
                //Modified by Tammy, the DSS process should be performed under dual control
                PED_SetSensitiveServiceTime(TRUE);
                if(!PED_DualPasswordControl(0))
                {
                    PED_SetSensitiveServiceTime(FALSE);
                    result  = FALSE;
                }
                else
                {
                    result = api_dss_interface();
                    PED_SetSensitiveServiceTime(FALSE);
                }  
                break;

            case 2:
                if(SecretInfo_GenRSAKeyPair() == apiOK)
                    result = TRUE;
                else
                    result = FALSE;
                break;

            case 3:
                PED_ADMIN_DeviceAuthenticationPhase1();
                result = TRUE;
                break;
            
            case 4:
                PED_ADMIN_DeviceAuthenticationPhase2();
                result = TRUE;
                break;
            
            case 5:
                SecretInfo_readAuditTrail();
                result = TRUE;
                break;
            
            case 6:
                ShowPEDVersions();
                result = TRUE;
                break;

            case 21:
                result = DSS_function_LANset();
                break;

            case 90:
                //Added by Tammy
                //if SP's initial version exists or SP has not been reloaded after tamper recovering
#if 1
                if((stat(SP_initFilePath, &stbuf) == 0) || (!Tamper_IsSecureDevice()))
#else
                if(stat(SP_initFilePath, &stbuf) == 0)
#endif
                    printf("Prohibit the user from entering KMS function\n");
                else
                    result = SYS_FUNC_KeyManagementSystem();
                break;

            case 95:
                ShowIdentifierInfo();
                result = TRUE;
                break;

            case 98:    //Added by Tammy
                Tamper_LoopTest();
                result = TRUE;
                break;

#if 0
            case 2:	// API TEST
            	
//              TEST_LCD();
		        TEST_TSC();
//		        TEST_SIGNPAD();
//		        TEST_TIMER();
//		        TEST_TIMER2();
//		        TEST_PRINTER();
//		        TEST_SYSAPI();

//		        TEST_DPA();	// function test OK, only trigger IO is to be verified!

		        result = TRUE;
		
                break;

            case 3:
                
                TEST_SPEAKER();
                break;
            
            case 14:	// MSR
                
                
                break;
            
            case 39:	// QR Reader
                
                result = DIAG_FUNC_Test2DReader();
                break;
#endif
            
#if 0   //used for PEDK test
            case 99:    //roll back uboot to enable printf log to COM0(mini USB)
                result = RollBackUboot();
                break;

            case 2: // Offline PIN test
                memset(pinblock, 0x00, sizeof(pinblock));

                if(api_ped_GetPin(tout, amt) == apiOK)
                {
                    if(PED_GenPinBlock(pinblock) == apiOK)
                    {
                        printf("[Offline PIN] ISO2 PIN block : ");
                        for(idx = 0 ; idx < sizeof(pinblock) ; idx++)
                            printf("0x%02X ", pinblock[idx]);

                        printf("\n");

                        result = TRUE;
                    }
                    else
                        result = FALSE;
                }
                else
                    result = FALSE;

                // To avoid not closing the buzzer properly in api_ped_GetPin()
                LIB_BUZ_Close();
                LIB_BUZ_Open();
                
                break;

            case 3: // Setup parameter test
                if(TEST_set_iso_format())
                {
                    if(api_ped_GetKeyMode() == PED_KEY_MODE_DUKPT)
                    {
                        TEST_AES_DUKPT_setWorkingKeyType(&workingKeyType);
                    }

                    mode = (workingKeyType << 8) | g_iso_format;
                    printf("mode = %04x\n", mode);

                    if(TEST_set_key_index())
                    {
                        result = TRUE;
                    }
                }
                else
                    result = FALSE;
                break;

            case 4: // Online PIN test
                if(api_ped_GetPin(tout, amt) == apiOK)
                {
                    printf("ISO format %d, key index %d\n", g_iso_format, g_key_index);

                    //if(TEST_GenEncrypedPinBlock(pan, epb, ksn, g_iso_format, g_key_index) == apiOK)
                    if(TEST_GenEncrypedPinBlock(pan, epb, ksn, mode, g_key_index) == apiOK)
                    {
                        printf("[Online PIN] ISO%d PIN block : ", g_iso_format);
                        for(idx = 0 ; idx < epb[0] ; idx++)
                            printf("0x%02X ", epb[idx + 2]);

                        printf("\n");

                        result = TRUE;
                    }
                    else
                        result = FALSE;
                }
                else
                    result = FALSE;

                // To avoid not closing the buzzer properly in api_ped_GetPin()
                LIB_BUZ_Close();
                LIB_BUZ_Open();
                break;

            case 5: // Generate MAC test
                if(TEST_GenMAC() == apiOK)
                    result = TRUE;
                else
                    result = FALSE;
                break;

            case 8: // SRED test
                if(SRED_DBG_Open_AUX(COM1) != apiOutOfService)
                {
                    if(api_ped_GetKeyMode() == PED_KEY_MODE_MS)
                    {
                        if(TEST_set_key_index())
                            result = SRED_TEST_OutputPAN(g_key_index);
                    }
                    else
                        result = SRED_TEST_OutputPAN(g_key_index);
                    
                    SRED_DBG_Close_AUX();
                }
                else
                    result = FALSE;

                break;

            case 9: // AES DUKPT test
                g_iso_format = 4;

                OS_SECM_PutData( ADDR_PED_PIN, PED_PIN_SLOT_LEN, pin );

                if(AES_DUKPT_RequestPinEntry(3, pan, pb, ksn, epb))
                {
                    printf("[Online PIN] ISO%d PIN block : ", g_iso_format);
                    for(idx = 0 ; idx < 16 ; idx++)
                        printf("0x%02X ", epb[idx]);

                    printf("\n");

                    result = TRUE;
                }
                else
                    result = FALSE;
                
                break;
#endif
        }

        if(result == TRUE)
        {
            LIB_BUZ_Beep1();
            LIB_LCD_Puts(18, 0, FONT1 + attrCLEARWRITE, sizeof(os_msg_OK), (UINT8 *)os_msg_OK);
        }
        else
        {
            LIB_BUZ_Beep2();
            LIB_LCD_Puts(18, 0, FONT1 + attrCLEARWRITE, sizeof(os_msg_ERROR), (UINT8 *)os_msg_ERROR);
        }
        
        LIB_WaitTimeAndKey(100); //delay 1.0 sec
    }
}

int main()
{
    struct stat stbuf;
    
    UINT8       sbuf[4];
    UINT8       dbuf[5];
    UINT8       multipleKeyCount=0;
    UINT16      dhn;
    UINT8       i;
    UINT8       isAPRunning = FALSE;

    //tamper loop test
    UINT8       tamperResult = 0;
    char        *noTamperMsg = "No External Tampering Detected";
    UINT8       key;


    // Tamper_ShowSNVSRegister();    // ==== [Debug] ====
    // printf("\n");   // ==== [Debug] ====
    openTamperRegister();
    openSHM();
    OS_EnableTimer1();
    OS_BUZ_Init();	// 2022-11-30, JAMES
    sys_IOinit();

    api_flash_init();
    api_sram_PageInit();
    // init_secure_memory();
    init_secure_file();
    os_flash_init();
    os_sram_init();
    flash_ctls_init();
    
    api_lcdtft_open(0);
    LIB_BUZ_Open();
    LIB_OpenKeyAll();
  
    backlight_control_LCDTFT(1,7);

    POST_AutoSetMAC();	// 2023-07-17, JAMES

    ShowPEDVersions();

    sbuf[0] = 0xff; // 7, 4, 1
    sbuf[1] = 0xff; // 0, 8, 5, 2
    sbuf[2] = 0xff; // 9, 6, 3
    sbuf[3] = 0xff; // ENTER, CLEAR, CANCEL
    dhn = api_kbd_open(0, sbuf);

    // UINT8   *pSecMem = load_security_memory();

#if 0
    //keep pressing 'y' to enter system mode
    if(LIB_GetKeyStatus() == apiReady)
    {
        if(LIB_WaitMuteKey() == 'y')
        {
            DSS_main();
        }
    }

    //tamper loop test
    while(1)
    {
        // if(LIB_GetKeyStatus() == apiReady)
        // {
        //     LIB_WaitKey();
        // }

        tamperResult = WhichTamperHappened();
        if(tamperResult)
        {
            Tamper_DisplayTamperMessage(tamperResult);
        }
        else
        {
            LIB_LCD_Cls();
            printf("\033[1;31;40mNo External Tampering Detected\033[0m\n");
            LIB_LCD_Puts(2, 5, FONT1, strlen(noTamperMsg), (UINT8 *)noTamperMsg);
        }

        LIB_WaitTime(100);
    }
#else
    //Added by Tammy, initicate tamper detection mechanism when the SRTC is invalidated
    if(IsSRTCInvalidated())
    {
        UINT8   tamperResult = 0;

        printf( "\033[1;31;40mthe SRTC is invalidated\033[0m\n" );
        
        tamperResult = WhichTamperHappened();
        if(tamperResult)
        {
            LIB_BUZ_Open();

            printf("\033[1;31;40mTamper occurred\033[0m\n");
            Tamper_DisplayTamperMessage(tamperResult);
            LIB_BUZ_TamperedBeeps(1);
            Tamper_RemoveSensitiveData();
        }
    }

    Tamper_polling();
#endif

    dhn_rtc = api_rtc_open();
    api_rtc_settimezone(dhn_rtc, 0x88); //GMT+8

#if 1
    //enforce to reload SP
    if(!Tamper_IsSecureDevice())
    {
        LIB_LCD_Cls();
        LIB_LCD_Puts(0, 4, FONT2 + attrREVERSE, strlen(" WARNING "), (UINT8 *)" WARNING ");
        LIB_WaitKeyMsgYesNoTO(7, 8, strlen("Please reload legal SP"), (UCHAR *)"Please reload legal SP");
        DSS_main();
    }
#endif

    if(stat("/home/AP/ca.crt", &stbuf) == 0)    //if ca.crt exists
        remove("/home/AP/ca.crt");

    if(stat("/home/AP/client.crt", &stbuf) == 0)    //if client.crt exists
        remove("/home/AP/client.crt");

    if(stat("/home/AP/client.key", &stbuf) == 0)    //if client.key exists
        remove("/home/AP/client.key");

    // get AES DUKPT information
    // OS_SECM_GetData(ADDR_INT_DERIVATION_KEY_IN_USE, 32, IntermediateDerivationKeyInUse);
    OS_SECM_GetIntDerivationKeyInUse(0, 32, IntermediateDerivationKeyInUse);
    OS_SECM_GetData(ADDR_CUR_DERIVATION_KEY, 4, &g_CurrentDerivationKey);
    OS_SECM_GetData(ADDR_TRANSACTION_COUNTER, 4, &g_TransactionCounter);

    //enforce to download signed SP, if SP's initial version exists
    if(stat(SP_initFilePath, &stbuf) == 0)
    {
        printf("SP's initial version exists!\n");
        DSS_main();
    }

#if 1
    //keep pressing 'y' to enter system mode
    if(LIB_GetKeyStatus() == apiReady)
    {
        if(LIB_WaitMuteKey() == 'y')
        {
            DSS_main();
        }
    }
#else
    //keep pressing '*', '0', and 'y' at the same time to enter system mode
    if(api_kbd_get_multiple_char(dhn, dbuf) == apiOK)
    {
        if(dbuf[0] == 3)
        {
            for(i = 0 ; i < 3 ; i++)
            {
                printf("dbuf[%d]=%c ", 1 + i, dbuf[1 + i]);
                if((dbuf[1 + i] == '*') || (dbuf[1 + i] == '0') || (dbuf[1 + i] == 'y'))
                    multipleKeyCount++;
            }

            if(multipleKeyCount == 3)
            {
                printf("multipleKeyCount execute DSS\n");
                DSS_main();
            }
        }
    }
#endif

    //AP signature verification
    if(stat(AP_filePath, &stbuf) == 0) //if AP exists
    {
        printf("AP exists!\n");

        if(Verify_AP_Signature() != apiOK)
        {
            remove(AP_filePath);
            DSS_main();
        }
        else
            Lan_polling();
    }
    else
    {
        perror("IPC_client");
        LIB_WaitTime(300);
        printf("execute DSS\n");
        DSS_main();
    }
    
#if 0   //used for SRED debug test
    // if(SRED_DBG_Open_AUX(COM1) != apiOutOfService)
    //     printf("Enable SRED debug\n");
#endif

    //Added by Tammy
    sem_init(sem_A, 1, 0);
    sem_init(sem_B, 1, 0);
    
#ifdef _DEBUG_SEMAPHORE_
    int valueOfA, valueOfB;
    sem_getvalue(sem_A, &valueOfA);
    sem_getvalue(sem_B, &valueOfB);
    printf("[SP] initial sem_A = %d, initial sem_B = %d\n", valueOfA, valueOfB);
#endif

    while(1)
    {
#if 1
        //IPC_client has normal user privilege
        if(0 > sem_trywait(sem_A) )//wait for client signal
        {
            if(!isAPRunning)
            {
                //The fork() function creates a duplicate of the calling process.
                if(fork() == 0)
                {
                    //The parent process is the one from which we call the execl() function to replace it with the child process.
                    //The execl() function replaces a running process with a new process.
                    //Both parent and child processes are executed simultaneously in case of fork()
                    //while Control never returns to the original program unless there is an execl() error.
                    if(execl("/bin/su", "su", "ap", "-c", "/home/AP/IPC_client", NULL) < 0)
                    {
                        perror("Failed to su ap -c /home/AP/IPC_client");
                        exit(0);    //terminates the child process
                    }
                }

                isAPRunning = TRUE;
            }

            usleep(100);
            continue;
        }
#else
        //IPC_client has root privilege
        if(0 > sem_trywait(sem_A) )//wait for client signal 
        continue;
#endif

#ifdef _DEBUG_SEMAPHORE_
        sem_getvalue(sem_A, &valueOfA);
        sem_getvalue(sem_B, &valueOfB);
        printf("[SP] sem_A = %d, sem_B = %d after sem_trywait(sem_A)\n", valueOfA, valueOfB);
#endif

        SHM_Handler();
        
#ifdef _DEBUG_SEMAPHORE_
        sem_getvalue(sem_A, &valueOfA);
        sem_getvalue(sem_B, &valueOfB);
        printf("[SP] sem_A = %d, sem_B = %d before sem_post(sem_B)\n", valueOfA, valueOfB);
#endif

        sem_post(sem_B); //release client signal (sem_B++ to signal AP for finishing process)

#ifdef _DEBUG_SEMAPHORE_
        sem_getvalue(sem_A, &valueOfA);
        sem_getvalue(sem_B, &valueOfB);
        printf("[SP] sem_A = %d, sem_B = %d after sem_post(sem_B)\n", valueOfA, valueOfB);
#endif
    }
    
}