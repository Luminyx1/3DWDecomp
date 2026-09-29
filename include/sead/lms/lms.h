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

/* A project file (MSBP). */
typedef struct LMSProjFile {
    LMSCommonBinaryFormat commonInfo;
    libms_s32_t mColorNameBlockIndex;       /* CLB1 */
    libms_s32_t mColorBlockIndex;           /* CLR1 */
    libms_s32_t mAttrNameBlockIndex;        /* ALB1 */
    libms_s32_t mAttrInfoBlockIndex;        /* ATI2 */
    libms_s32_t mAttrListBlockIndex;        /* ALI2 */
    libms_s32_t mTagGroupBlockIndex;        /* TGG2 */
    libms_s32_t mTagBlockIndex;             /* TAG2 */
    libms_s32_t mTagParamBlockIndex;        /* TGP2 */
    libms_s32_t mTagListItemBlockIndex;     /* TGL2 */
    libms_s32_t mStyleBlockIndex;           /* SYL3 */
    libms_s32_t mStyleNameBlockIndex;       /* SLB1 */
    libms_s32_t mContentBlockIndex;         /* CTI1 */
} LMSProjFile;

typedef struct LMSColor {
    libms_u8_t r;
    libms_u8_t g;
    libms_u8_t b;
    libms_u8_t a;
} LMSColor;

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

/* lms_project.c */
LMSProjFile* LMS_InitProject(const char* pBlockData);
void LMS_CloseProject(LMSProjFile* pProj);
libms_s32_t LMS_SearchProjectBlockByName(LMSProjFile* pProj, const char* pName);
libms_s32_t LMS_GetContentsNum(LMSProjFile* pProj);
const char* LMS_GetContentPath(LMSProjFile* pProj, libms_s32_t index);
libms_s32_t LMS_GetColorIndexByName(LMSProjFile* pProj, const char* pName);
libms_s32_t LMS_GetColorNum(LMSProjFile* pProj);
libms_s32_t LMS_GetColor(LMSProjFile* pProj, libms_s32_t index, LMSColor* pColor);
libms_s32_t LMS_GetColorByName(LMSProjFile* pProj, const char* pName, LMSColor* pColor);
libms_s32_t LMS_GetAttrInfoIndexByName(LMSProjFile* pProj, const char* pName);
libms_s32_t LMS_GetAttrType(LMSProjFile* pProj, libms_s32_t index);
libms_s32_t LMS_GetAttrOffset(LMSProjFile* pProj, libms_s32_t index);
libms_s32_t LMS_GetAttrTypeByName(LMSProjFile* pProj, const char* pName);
libms_s32_t LMS_GetAttrOffsetByName(LMSProjFile* pProj, const char* pName);
const char* LMS_GetAttrListItemName(LMSProjFile* pProj, libms_s32_t attrIndex, libms_s32_t itemIndex);
const char* LMS_GetAttrListItemNameByName(LMSProjFile* pProj, const char* pName, libms_s32_t itemIndex);
libms_s32_t LMS_GetAttrNum(LMSProjFile* pProj);
libms_s32_t LMS_GetAttrListItemNum(LMSProjFile* pProj, libms_s32_t attrIndex);
const char* LMS_GetTagGroupName(LMSProjFile* pProj, libms_u16_t groupIndex);
const char* LMS_GetTagName(LMSProjFile* pProj, libms_u16_t groupIndex, libms_u16_t tagIndex);
const char* LMS_GetTagParamName(LMSProjFile* pProj, libms_u16_t groupIndex, libms_u16_t tagIndex,
                                libms_u16_t paramIndex);
libms_s32_t LMS_GetTagParamType(LMSProjFile* pProj, libms_u16_t groupIndex, libms_u16_t tagIndex,
                                libms_u16_t paramIndex);
const char* LMS_GetTagListItemName(LMSProjFile* pProj, libms_u16_t groupIndex, libms_u16_t tagIndex,
                                   libms_u16_t paramIndex, libms_u16_t itemIndex);
libms_s32_t LMS_GetTagGroupNum(LMSProjFile* pProj);
libms_s32_t LMS_GetTagNum(LMSProjFile* pProj, libms_u16_t groupIndex);
libms_s32_t LMS_GetTagParamNum(LMSProjFile* pProj, libms_u16_t groupIndex, libms_u16_t tagIndex);
libms_s32_t LMS_GetTagListItemNum(LMSProjFile* pProj, libms_u16_t groupIndex, libms_u16_t tagIndex,
                                  libms_u16_t paramIndex);
libms_s32_t LMS_GetStyleIndexByName(LMSProjFile* pProj, const char* pName);
libms_s32_t LMS_GetStyleNum(LMSProjFile* pProj);
libms_s32_t LMS_GetRegionWidth(LMSProjFile* pProj, libms_s32_t index);
libms_s32_t LMS_GetRegionWidthByName(LMSProjFile* pProj, const char* pName);
libms_s32_t LMS_GetLineNum(LMSProjFile* pProj, libms_s32_t index);
libms_s32_t LMS_GetLineNumByName(LMSProjFile* pProj, const char* pName);
libms_s32_t LMS_GetFontIndex(LMSProjFile* pProj, libms_s32_t index);
libms_s32_t LMS_GetFontIndexByName(LMSProjFile* pProj, const char* pName);
libms_s32_t LMS_GetBaseColorIndex(LMSProjFile* pProj, libms_s32_t index);
libms_s32_t LMS_GetBaseColorIndexByName(LMSProjFile* pProj, const char* pName);

#ifdef __cplusplus
}
#endif

#endif /* SEAD_LMS_LMS_H */
