#include "ECL_Tag_Structure.h"
#include "MPP_Define.h"

//		Buffer of PayPass Self Define Tags
//unsigned char mpp_bufActiveAFL[3+MPP_LENGTH_ActiveAFL]={0};
//unsigned char mpp_bufActiveTag[3+MPP_LENGTH_ActiveTag]={0};
//unsigned char mpp_bufACType[3+MPP_LENGTH_ACType]={0};
//unsigned char mpp_bufFailedMSCntr[3+MPP_LENGTH_FailedMSCntr]={0};
//unsigned char mpp_bufNextCmd[3+MPP_LENGTH_NextCmd]={0};
//unsigned char mpp_bufnUN[3+MPP_LENGTH_nUN]={0};
//unsigned char mpp_bufODAStatus[3+MPP_LENGTH_ODAStatus]={0};
//unsigned char mpp_bufReaderContactlessTransactionLimit[3+MPP_LENGTH_ReaderContactlessTransactionLimit]={0};
//unsigned char mpp_bufStaticDataToBeAuthenticated[3+MPP_LENGTH_StaticDataToBeAuthenticated]={0};
//unsigned char mpp_bufTagsToReadYet[3+MPP_TLVLIST_BUFFERSIZE]={0};
//unsigned char mpp_bufTagsToWriteYetAfterGenAC[3+MPP_TLVLIST_BUFFERSIZE]={0};
//unsigned char mpp_bufTagsToWriteYetBeforeGenAC[3+MPP_TLVLIST_BUFFERSIZE]={0};
//unsigned char mpp_bufTornEntry[3+MPP_LENGTH_TornEntry]={0};
//unsigned char mpp_bufTornTempRecord[3+MPP_LENGTH_TornTempRecord]={0};
unsigned char *mpp_bufActiveAFL;
unsigned char *mpp_bufActiveTag;
unsigned char *mpp_bufACType;
unsigned char *mpp_bufFailedMSCntr;
unsigned char *mpp_bufNextCmd;
unsigned char *mpp_bufnUN;
unsigned char *mpp_bufODAStatus;
unsigned char *mpp_bufReaderContactlessTransactionLimit;
unsigned char *mpp_bufStaticDataToBeAuthenticated;
unsigned char *mpp_bufTagsToReadYet;
unsigned char *mpp_bufTagsToWriteYetAfterGenAC;
unsigned char *mpp_bufTagsToWriteYetBeforeGenAC;
unsigned char *mpp_bufTornEntry;
unsigned char *mpp_bufTornTempRecord;


//		PayPass Self Define Tags
//
//		[Check MPP_Reset_SelfDefineTag After Adding Tag]
//
//		Tag Number:14
//ECL_TAG mpp_tagActiveAFL						={mpp_bufActiveAFL,							3+mpp_bufActiveAFL};
//ECL_TAG mpp_tagActiveTag						={mpp_bufActiveTag,							3+mpp_bufActiveTag};
//ECL_TAG mpp_tagACType							={mpp_bufACType,							3+mpp_bufACType};
//ECL_TAG mpp_tagFailedMSCntr					={mpp_bufFailedMSCntr,						3+mpp_bufFailedMSCntr};
//ECL_TAG mpp_tagNextCmd						={mpp_bufNextCmd,							3+mpp_bufNextCmd};

//ECL_TAG mpp_tagnUN							={mpp_bufnUN,								3+mpp_bufnUN};
//ECL_TAG mpp_tagODAStatus						={mpp_bufODAStatus,							3+mpp_bufODAStatus};
//ECL_TAG mpp_tagReaderContactlessTransactionLimit={mpp_bufReaderContactlessTransactionLimit,	3+mpp_bufReaderContactlessTransactionLimit};
//ECL_TAG mpp_tagStaticDataToBeAuthenticated	={mpp_bufStaticDataToBeAuthenticated,		3+mpp_bufStaticDataToBeAuthenticated};
//ECL_TAG mpp_tagTagsToReadYet					={mpp_bufTagsToReadYet,						3+mpp_bufTagsToReadYet};

//ECL_TAG mpp_tagTagsToWriteYetAfterGenAC		={mpp_bufTagsToWriteYetAfterGenAC,			3+mpp_bufTagsToWriteYetAfterGenAC};
//ECL_TAG mpp_tagTagsToWriteYetBeforeGenAC		={mpp_bufTagsToWriteYetBeforeGenAC,			3+mpp_bufTagsToWriteYetBeforeGenAC};
//ECL_TAG mpp_tagTornEntry						={mpp_bufTornEntry,							3+mpp_bufTornEntry};
//ECL_TAG mpp_tagTornTempRecord					={mpp_bufTornTempRecord,					3+mpp_bufTornTempRecord};

ECL_TAG	mpp_tagActiveAFL						={0,	0};
ECL_TAG mpp_tagActiveTag						={0,	0};
ECL_TAG	mpp_tagACType							={0,	0};
ECL_TAG	mpp_tagFailedMSCntr						={0,	0};
ECL_TAG mpp_tagNextCmd							={0,	0};

ECL_TAG mpp_tagnUN								={0,	0};
ECL_TAG	mpp_tagODAStatus						={0,	0};
ECL_TAG mpp_tagReaderContactlessTransactionLimit={0,	0};
ECL_TAG	mpp_tagStaticDataToBeAuthenticated		={0,	0};
ECL_TAG	mpp_tagTagsToReadYet					={0,	0};

ECL_TAG	mpp_tagTagsToWriteYetAfterGenAC			={0,	0};
ECL_TAG	mpp_tagTagsToWriteYetBeforeGenAC		={0,	0};
ECL_TAG mpp_tagTornEntry						={0,	0};
ECL_TAG mpp_tagTornTempRecord					={0,	0};
