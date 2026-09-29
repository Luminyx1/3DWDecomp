#ifndef SEAD_LMS_LMS_H
#define SEAD_LMS_LMS_H

/* LibMessageStudio (LMS), shipped as part of sead - the MSBT/MSBP message runtime (plain C; the
 * symbols are unmangled in the binary).
 *
 * Types follow the 3dcomp project's libms.h (lib/ms/include/libms.h). */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char libms_u8_t;
typedef unsigned short libms_u16_t;
typedef int libms_s32_t;
typedef unsigned int libms_u32_t;

typedef void* (*LMSMallocPtr)(size_t);
typedef void (*LMSFreePtr)(void*);

/* One section ("pBlock") of an LMS binary, e.g. LBL1 / TXT2 / ATR1. */
typedef struct LMSBlockInfo {
    const char* pData;       /* pBlock contents (after its 16-byte header) */
    char mBlockName[4];
    libms_u32_t mDataSize;
    libms_u16_t mUnk10;
    libms_u16_t pad;
} LMSBlockInfo;

typedef struct LMSCommonBinaryFormat {
    const char* pResource;   /* the whole file */
    size_t mResourceLength;
    libms_u8_t mEncoding;    /* 0: UTF-8, 1: UTF-16, 2: UTF-32 */
    libms_u8_t pad;
    libms_u16_t mBlockNum;
    LMSBlockInfo* mBlockInfo;
} LMSCommonBinaryFormat;

/* A message file (MSBT). */
typedef struct LMSMsgFile {
    LMSCommonBinaryFormat commonInfo;
    libms_s32_t mLabelBlockIndex;     /* LBL1 */
    libms_s32_t mTextBlockIndex;      /* TXT2 */
    libms_s32_t mAttributeBlockIndex; /* ATR1 */
    libms_s32_t mATOBlockIndex;       /* ATO1 */
    libms_s32_t mStyleBlockIndex;     /* TSY1 */
} LMSMsgFile;

/* lms_memory.c */
void LMS_SetMemFuncs(LMSMallocPtr pMallocFunc, LMSFreePtr pFreeFunc);
void* LMSi_Malloc(size_t size);
void LMSi_Free(void* pPtr);
int LMSi_MemCmp(const char* pA, const char* pB, int size);
void LMSi_MemCopy(void* pDst, const void* pSrc, int size);

/* lms_analyze.c */
void LMSi_AnalyzeMessageHeader(LMSCommonBinaryFormat* pBin);
void LMSi_AnalyzeMessageBlocks(LMSCommonBinaryFormat* pBin);
void LMSi_AnalyzeMessageBinary(LMSCommonBinaryFormat* pBin, const char* pMagic, int version);
libms_s32_t LMSi_SearchBlockByName(LMSCommonBinaryFormat* pBin, const char* pName);
LMSBlockInfo* LMSi_GetBlockInfoByName(LMSCommonBinaryFormat* pBin, const char* pName);
libms_u32_t LMSi_GetHashTableIndexFromLabel(const char* pLabel, libms_u32_t numSlots);

/* lms_message.c */
LMSMsgFile* LMS_InitMessage(const char* pBlockData);
void LMS_CloseMessage(LMSMsgFile* pMsg);
libms_s32_t LMS_SearchMessageBlockByName(LMSMsgFile* pMsg, const char* pName);
LMSBlockInfo* LMS_GetMessageBlockInfoByName(LMSMsgFile* pMsg, const char* pName);
libms_s32_t LMS_GetTextNum(LMSMsgFile* pMsg);
libms_s32_t LMS_GetTextIndexByLabel(LMSMsgFile* pMsg, const char* pLabel);
const void* LMS_GetText(LMSMsgFile* pMsg, libms_s32_t index);
libms_s32_t LMS_GetTextSize(LMSMsgFile* pMsg, libms_s32_t index);
const void* LMS_GetTextByLabel(LMSMsgFile* pMsg, const char* pLabel);
int LMS_GetLabelByTextIndex(LMSMsgFile* pMsg, libms_s32_t index, char* pOutLabel);
libms_s32_t LMS_GetAttributeSize(LMSMsgFile* pMsg);
const void* LMS_GetAttribute(LMSMsgFile* pMsg, libms_s32_t index);
const char* LMS_GetAttributeText(LMSMsgFile* pMsg, libms_s32_t offset);
libms_s32_t LMS_GetAttrFilteredOffset(LMSMsgFile* pMsg, libms_s32_t index);
libms_s32_t LMS_GetTextStyle(LMSMsgFile* pMsg, libms_s32_t index);
libms_s32_t LMS_GetTextStyleByLabel(LMSMsgFile* pMsg, const char* pLabel);

#ifdef __cplusplus
}
#endif

#endif /* SEAD_LMS_LMS_H */
