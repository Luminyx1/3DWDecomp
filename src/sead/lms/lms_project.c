#include "lms/lms.h"

typedef struct LMSAttrInfo {
    signed char mType;
    libms_u8_t pad;
    libms_u16_t mListIndex;
    libms_s32_t mOffset;
} LMSAttrInfo;

typedef struct LMSTagGroupInfo {
    libms_u16_t mIndex;
    libms_u16_t mTagNum;
    libms_u16_t mTagIndex[1];
} LMSTagGroupInfo;

typedef struct LMSTagInfo {
    libms_u16_t mParamNum;
    libms_u16_t mParamIndex[1];
} LMSTagInfo;

typedef struct LMSTagParamInfo {
    libms_u8_t mType;
    libms_u8_t pad;
    libms_u16_t mListItemNum;
    libms_u16_t mListItemIndex[1];
} LMSTagParamInfo;

typedef struct LMSStyleInfo {
    libms_s32_t mRegionWidth;
    libms_s32_t mLineNum;
    libms_s32_t mFontIndex;
    libms_s32_t mBaseColorIndex;
} LMSStyleInfo;

/**
 * Counts the characters of a label including its terminator.
 * @param pLabel NUL-terminated label
 * @return strlen(pLabel) + 1.
 */
static int LMSi_GetLabelLength(const char* pLabel) {
    int length = 0;
    while (pLabel[length++] != '\0') {
    }
    return length;
}

/**
 * Looks up a label in a label hash table block (CLB1, ALB1, SLB1).
 * @param pBlock the label block
 * @param pLabel label to look up
 * @param length label length including its terminator
 * @return the index stored for pLabel, or -1.
 */
static libms_s32_t LMSi_SearchLabel(const LMSBlockInfo* pBlock, const char* pLabel, int length) {
    libms_u32_t hash;
    libms_u32_t count;
    libms_u32_t offset;
    libms_u32_t i;

    hash = LMSi_GetHashTableIndexFromLabel(pLabel, *(const libms_u32_t*)pBlock->pData) * 2 + 1;
    count = ((const libms_u32_t*)pBlock->pData)[(int)hash];
    if (count == 0) {
        return -1;
    }

    offset = ((const libms_u32_t*)pBlock->pData)[(int)(hash + 1)];
    for (i = 0; i < count; i++) {
        libms_u8_t len = pBlock->pData[offset];
        if (len + 1 == length &&
            LMSi_MemCmp(pLabel, pBlock->pData + (offset + sizeof(libms_u8_t)), length - 1)) {
            return *(const libms_s32_t*)(pBlock->pData + (offset + sizeof(libms_u8_t) + len));
        }
        offset += len + 5;
    }
    return -1;
}

/**
 * @param pProj the project file
 * @param blockIndex block index
 * @return the data of block number blockIndex.
 */
static const char* LMSi_GetProjectBlockData(LMSProjFile* pProj, libms_s32_t blockIndex) {
    return pProj->commonInfo.mBlockInfo[blockIndex].pData;
}

/**
 * @param pProj the project file
 * @param index attribute index
 * @return attribute info number index of the ATI2 block, or NULL.
 */
static const LMSAttrInfo* LMSi_GetAttrInfo(LMSProjFile* pProj, libms_s32_t index) {
    const char* pData;
    if (pProj->mAttrInfoBlockIndex == -1) {
        return NULL;
    }
    pData = LMSi_GetProjectBlockData(pProj, pProj->mAttrInfoBlockIndex);
    if (*(const libms_s32_t*)pData <= index) {
        return NULL;
    }
    return &((const LMSAttrInfo*)(pData + 4))[index];
}

/**
 * Searches the TGG2 block (sorted by group index) for a tag group.
 * @param pProj the project file
 * @param groupIndex tag group index
 * @return the tag group, or NULL.
 */
static const LMSTagGroupInfo* LMSi_GetTagGroupInfo(LMSProjFile* pProj, libms_u16_t groupIndex) {
    const char* pData;
    libms_u16_t num;
    libms_u16_t i;
    if (pProj->mTagGroupBlockIndex == -1) {
        return NULL;
    }
    pData = LMSi_GetProjectBlockData(pProj, pProj->mTagGroupBlockIndex);
    num = *(const libms_u16_t*)pData;
    for (i = 0; i < num; i++) {
        const LMSTagGroupInfo* pGroup =
            (const LMSTagGroupInfo*)(pData + ((const libms_u32_t*)pData)[i + 1]);
        if (pGroup->mIndex == groupIndex) {
            return pGroup;
        }
        if (pGroup->mIndex > groupIndex) {
            return NULL;
        }
    }
    return NULL;
}

/**
 * @param pProj the project file
 * @param groupIndex tag group index
 * @param tagIndex tag index inside the group
 * @return the TAG2 entry of the tag, or NULL.
 */
static const LMSTagInfo* LMSi_GetTagInfo(LMSProjFile* pProj, libms_u16_t groupIndex, libms_u16_t tagIndex) {
    const LMSTagGroupInfo* pGroup = LMSi_GetTagGroupInfo(pProj, groupIndex);
    const char* pData;
    libms_u16_t index;
    if (pGroup == NULL) {
        return NULL;
    }
    if (pProj->mTagBlockIndex == -1) {
        return NULL;
    }
    if (pGroup->mTagNum <= tagIndex) {
        return NULL;
    }
    index = pGroup->mTagIndex[tagIndex];
    pData = LMSi_GetProjectBlockData(pProj, pProj->mTagBlockIndex);
    return (const LMSTagInfo*)(pData + ((const libms_u32_t*)pData)[index + 1]);
}

/**
 * @param pProj the project file
 * @param groupIndex tag group index
 * @param tagIndex tag index inside the group
 * @param paramIndex parameter index inside the tag
 * @return the TGP2 entry of the tag parameter, or NULL.
 */
static const LMSTagParamInfo* LMSi_GetTagParamInfo(LMSProjFile* pProj, libms_u16_t groupIndex,
                                                   libms_u16_t tagIndex, libms_u16_t paramIndex) {
    const LMSTagInfo* pTag = LMSi_GetTagInfo(pProj, groupIndex, tagIndex);
    const char* pData;
    libms_u16_t index;
    if (pTag == NULL) {
        return NULL;
    }
    if (pProj->mTagParamBlockIndex == -1) {
        return NULL;
    }
    if (pTag->mParamNum <= paramIndex) {
        return NULL;
    }
    index = pTag->mParamIndex[paramIndex];
    pData = LMSi_GetProjectBlockData(pProj, pProj->mTagParamBlockIndex);
    return (const LMSTagParamInfo*)(pData + ((const libms_u32_t*)pData)[index + 1]);
}

/**
 * Opens an MSBP file in memory and locates its blocks.
 * @param pBlockData the MSBP file
 * @return the project file.
 */
LMSProjFile* LMS_InitProject(const char* pBlockData) {
    LMSProjFile* pProj = (LMSProjFile*)LMSi_Malloc(sizeof(LMSProjFile));
    pProj->commonInfo.pResource = pBlockData;
    LMSi_AnalyzeMessageBinary(&pProj->commonInfo, "MsgPrjBn", 4);
    pProj->mColorBlockIndex = LMSi_SearchBlockByName(&pProj->commonInfo, "CLR1");
    pProj->mColorNameBlockIndex = LMSi_SearchBlockByName(&pProj->commonInfo, "CLB1");
    pProj->mAttrInfoBlockIndex = LMSi_SearchBlockByName(&pProj->commonInfo, "ATI2");
    pProj->mAttrNameBlockIndex = LMSi_SearchBlockByName(&pProj->commonInfo, "ALB1");
    pProj->mAttrListBlockIndex = LMSi_SearchBlockByName(&pProj->commonInfo, "ALI2");
    pProj->mTagGroupBlockIndex = LMSi_SearchBlockByName(&pProj->commonInfo, "TGG2");
    pProj->mTagBlockIndex = LMSi_SearchBlockByName(&pProj->commonInfo, "TAG2");
    pProj->mTagParamBlockIndex = LMSi_SearchBlockByName(&pProj->commonInfo, "TGP2");
    pProj->mTagListItemBlockIndex = LMSi_SearchBlockByName(&pProj->commonInfo, "TGL2");
    pProj->mStyleBlockIndex = LMSi_SearchBlockByName(&pProj->commonInfo, "SYL3");
    pProj->mStyleNameBlockIndex = LMSi_SearchBlockByName(&pProj->commonInfo, "SLB1");
    pProj->mContentBlockIndex = LMSi_SearchBlockByName(&pProj->commonInfo, "CTI1");
    return pProj;
}

/**
 * Frees a project file opened with LMS_InitProject (the data itself is not freed).
 * @param pProj the project file
 */
void LMS_CloseProject(LMSProjFile* pProj) {
    if (pProj->commonInfo.mBlockInfo) {
        LMSi_Free(pProj->commonInfo.mBlockInfo);
    }
    LMSi_Free(pProj);
}

/**
 * @param pProj the project file
 * @param pName 4-character block name, e.g. "CLR1"
 * @return the index of the block named pName, or -1.
 */
libms_s32_t LMS_SearchProjectBlockByName(LMSProjFile* pProj, const char* pName) {
    return LMSi_SearchBlockByName(&pProj->commonInfo, pName);
}

/**
 * @param pProj the project file
 * @return the number of contents in the CTI1 block, or -1 if there is none.
 */
libms_s32_t LMS_GetContentsNum(LMSProjFile* pProj) {
    if (pProj->mContentBlockIndex == -1) {
        return -1;
    }
    return *(const libms_s32_t*)LMSi_GetProjectBlockData(pProj, pProj->mContentBlockIndex);
}

/**
 * @param pProj the project file
 * @param index content index
 * @return the path of content number index, or NULL.
 */
const char* LMS_GetContentPath(LMSProjFile* pProj, libms_s32_t index) {
    const char* pData;
    if (pProj->mContentBlockIndex == -1) {
        return NULL;
    }
    pData = LMSi_GetProjectBlockData(pProj, pProj->mContentBlockIndex);
    if (*(const libms_s32_t*)pData <= index) {
        return NULL;
    }
    return pData + ((const libms_u32_t*)(pData + 4))[index];
}

/**
 * @param pProj the project file
 * @param pName color name
 * @return the index of the color named pName, -1 if it is not found, or -2 if there is no CLB1 block.
 */
libms_s32_t LMS_GetColorIndexByName(LMSProjFile* pProj, const char* pName) {
    LMSBlockInfo* pBlocks;
    int length;
    if (pProj->mColorNameBlockIndex == -1) {
        return -2;
    }
    pBlocks = pProj->commonInfo.mBlockInfo;
    length = LMSi_GetLabelLength(pName);
    return LMSi_SearchLabel(&pBlocks[pProj->mColorNameBlockIndex], pName, length);
}

/**
 * @param pProj the project file
 * @return the number of colors, or 0.
 */
libms_s32_t LMS_GetColorNum(LMSProjFile* pProj) {
    if (pProj->mColorBlockIndex == -1) {
        return 0;
    }
    return *(const libms_s32_t*)LMSi_GetProjectBlockData(pProj, pProj->mColorBlockIndex);
}

/**
 * Copies a color of the CLR1 block.
 * @param pProj the project file
 * @param index color index
 * @param pColor receives the color
 * @return 0, -1 for an invalid index, or -5 if there is no CLR1 block.
 */
libms_s32_t LMS_GetColor(LMSProjFile* pProj, libms_s32_t index, LMSColor* pColor) {
    const char* pData;
    const LMSColor* pSrc;
    if (pProj->mColorBlockIndex == -1) {
        return -5;
    }
    pData = LMSi_GetProjectBlockData(pProj, pProj->mColorBlockIndex);
    if (*(const libms_s32_t*)pData <= index) {
        return -1;
    }
    pSrc = &((const LMSColor*)(pData + 4))[index];
    pColor->r = pSrc->r;
    pColor->g = pSrc->g;
    pColor->b = pSrc->b;
    pColor->a = pSrc->a;
    return 0;
}

/**
 * Copies the color named pName.
 * @param pProj the project file
 * @param pName color name
 * @param pColor receives the color
 * @return 0 or a negative error, see LMS_GetColor.
 */
libms_s32_t LMS_GetColorByName(LMSProjFile* pProj, const char* pName, LMSColor* pColor) {
    return LMS_GetColor(pProj, LMS_GetColorIndexByName(pProj, pName), pColor);
}

/**
 * @param pProj the project file
 * @param pName attribute name
 * @return the index of the attribute named pName, -1 if it is not found, or -2 if there is no ALB1 block.
 */
libms_s32_t LMS_GetAttrInfoIndexByName(LMSProjFile* pProj, const char* pName) {
    int length;
    if (pProj->mAttrNameBlockIndex == -1) {
        return -2;
    }
    length = LMSi_GetLabelLength(pName);
    return LMSi_SearchLabel(&pProj->commonInfo.mBlockInfo[pProj->mAttrNameBlockIndex], pName, length);
}

/**
 * @param pProj the project file
 * @param index attribute index
 * @return the data type of attribute number index, or 0xFF.
 */
libms_s32_t LMS_GetAttrType(LMSProjFile* pProj, libms_s32_t index) {
    const LMSAttrInfo* pInfo = LMSi_GetAttrInfo(pProj, index);
    if (pInfo == NULL) {
        return 0xFF;
    }
    return pInfo->mType;
}

/**
 * @param pProj the project file
 * @param index attribute index
 * @return the offset of attribute number index inside an attribute entry, or -1.
 */
libms_s32_t LMS_GetAttrOffset(LMSProjFile* pProj, libms_s32_t index) {
    const LMSAttrInfo* pInfo = LMSi_GetAttrInfo(pProj, index);
    if (pInfo == NULL) {
        return -1;
    }
    return pInfo->mOffset;
}

/**
 * @param pProj the project file
 * @param pName attribute name
 * @return the data type of the attribute named pName, or 0xFF.
 */
libms_s32_t LMS_GetAttrTypeByName(LMSProjFile* pProj, const char* pName) {
    libms_s32_t index = LMS_GetAttrInfoIndexByName(pProj, pName);
    if (index < 0) {
        return 0xFF;
    }
    return LMS_GetAttrType(pProj, (libms_u16_t)index);
}

/**
 * @param pProj the project file
 * @param pName attribute name
 * @return the offset of the attribute named pName, or a negative error.
 */
libms_s32_t LMS_GetAttrOffsetByName(LMSProjFile* pProj, const char* pName) {
    libms_s32_t index = LMS_GetAttrInfoIndexByName(pProj, pName);
    if (index < 0) {
        return index;
    }
    return LMS_GetAttrOffset(pProj, (libms_u16_t)index);
}

/**
 * @param pProj the project file
 * @param attrIndex index of a list attribute
 * @param itemIndex list item index
 * @return the name of the list item, or NULL.
 */
const char* LMS_GetAttrListItemName(LMSProjFile* pProj, libms_s32_t attrIndex, libms_s32_t itemIndex) {
    const LMSAttrInfo* pInfo = LMSi_GetAttrInfo(pProj, attrIndex);
    const char* pData;
    const char* pList;
    if (pInfo == NULL) {
        return NULL;
    }
    if (pInfo->mType != 9) {
        return NULL;
    }
    if (pProj->mAttrListBlockIndex == -1) {
        return NULL;
    }
    pData = LMSi_GetProjectBlockData(pProj, pProj->mAttrListBlockIndex);
    pList = pData + ((const libms_u32_t*)pData)[pInfo->mListIndex + 1];
    if (*(const libms_s32_t*)pList <= itemIndex) {
        return NULL;
    }
    return pList + ((const libms_u32_t*)(pList + 4))[itemIndex];
}

/**
 * @param pProj the project file
 * @param pName name of a list attribute
 * @param itemIndex list item index
 * @return the name of the list item, or NULL.
 */
const char* LMS_GetAttrListItemNameByName(LMSProjFile* pProj, const char* pName, libms_s32_t itemIndex) {
    libms_s32_t index = LMS_GetAttrInfoIndexByName(pProj, pName);
    if (index < 0) {
        return NULL;
    }
    return LMS_GetAttrListItemName(pProj, index, itemIndex);
}

/**
 * @param pProj the project file
 * @return the number of attributes, or 0.
 */
libms_s32_t LMS_GetAttrNum(LMSProjFile* pProj) {
    if (pProj->mAttrInfoBlockIndex == -1) {
        return 0;
    }
    return *(const libms_s32_t*)LMSi_GetProjectBlockData(pProj, pProj->mAttrInfoBlockIndex);
}

/**
 * @param pProj the project file
 * @param attrIndex index of a list attribute
 * @return the number of list items, or 0.
 */
libms_s32_t LMS_GetAttrListItemNum(LMSProjFile* pProj, libms_s32_t attrIndex) {
    const LMSAttrInfo* pInfo = LMSi_GetAttrInfo(pProj, attrIndex);
    const char* pData;
    if (pInfo == NULL) {
        return 0;
    }
    if (pInfo->mType != 9) {
        return 0;
    }
    if (pProj->mAttrListBlockIndex == -1) {
        return 0;
    }
    pData = LMSi_GetProjectBlockData(pProj, pProj->mAttrListBlockIndex);
    return *(const libms_s32_t*)(pData + ((const libms_u32_t*)pData)[pInfo->mListIndex + 1]);
}

/**
 * @param pProj the project file
 * @param groupIndex tag group index
 * @return the name of the tag group, or NULL.
 */
const char* LMS_GetTagGroupName(LMSProjFile* pProj, libms_u16_t groupIndex) {
    const LMSTagGroupInfo* pGroup = LMSi_GetTagGroupInfo(pProj, groupIndex);
    if (pGroup == NULL) {
        return NULL;
    }
    return (const char*)&pGroup->mTagIndex[pGroup->mTagNum];
}

/**
 * @param pProj the project file
 * @param groupIndex tag group index
 * @param tagIndex tag index inside the group
 * @return the name of the tag, or NULL.
 */
const char* LMS_GetTagName(LMSProjFile* pProj, libms_u16_t groupIndex, libms_u16_t tagIndex) {
    const LMSTagInfo* pTag = LMSi_GetTagInfo(pProj, groupIndex, tagIndex);
    if (pTag == NULL) {
        return NULL;
    }
    return (const char*)&pTag->mParamIndex[pTag->mParamNum];
}

/**
 * @param pProj the project file
 * @param groupIndex tag group index
 * @param tagIndex tag index inside the group
 * @param paramIndex parameter index inside the tag
 * @return the name of the tag parameter, or NULL.
 */
const char* LMS_GetTagParamName(LMSProjFile* pProj, libms_u16_t groupIndex, libms_u16_t tagIndex,
                                libms_u16_t paramIndex) {
    const LMSTagParamInfo* pParam = LMSi_GetTagParamInfo(pProj, groupIndex, tagIndex, paramIndex);
    if (pParam == NULL) {
        return NULL;
    }
    if (pParam->mType == 9) {
        return (const char*)&pParam->mListItemIndex[pParam->mListItemNum];
    }
    return (const char*)&pParam->pad;
}

/**
 * @param pProj the project file
 * @param groupIndex tag group index
 * @param tagIndex tag index inside the group
 * @param paramIndex parameter index inside the tag
 * @return the data type of the tag parameter, or 0xFF.
 */
libms_s32_t LMS_GetTagParamType(LMSProjFile* pProj, libms_u16_t groupIndex, libms_u16_t tagIndex,
                                libms_u16_t paramIndex) {
    const LMSTagParamInfo* pParam = LMSi_GetTagParamInfo(pProj, groupIndex, tagIndex, paramIndex);
    if (pParam == NULL) {
        return 0xFF;
    }
    if (pParam->mType > 9) {
        return 0xFF;
    }
    return pParam->mType;
}

/**
 * @param pProj the project file
 * @param groupIndex tag group index
 * @param tagIndex tag index inside the group
 * @param paramIndex index of a list parameter inside the tag
 * @param itemIndex list item index
 * @return the name of the list item, or NULL.
 */
const char* LMS_GetTagListItemName(LMSProjFile* pProj, libms_u16_t groupIndex, libms_u16_t tagIndex,
                                   libms_u16_t paramIndex, libms_u16_t itemIndex) {
    const LMSTagParamInfo* pParam = LMSi_GetTagParamInfo(pProj, groupIndex, tagIndex, paramIndex);
    const char* pData;
    libms_u16_t index;
    if (pParam == NULL) {
        return NULL;
    }
    if (pProj->mTagListItemBlockIndex == -1) {
        return NULL;
    }
    if (pParam->mType != 9) {
        return NULL;
    }
    if (pParam->mListItemNum <= itemIndex) {
        return NULL;
    }
    index = pParam->mListItemIndex[itemIndex];
    pData = LMSi_GetProjectBlockData(pProj, pProj->mTagListItemBlockIndex);
    return pData + ((const libms_u32_t*)pData)[index + 1];
}

/**
 * @param pProj the project file
 * @return the number of tag groups, or 0.
 */
libms_s32_t LMS_GetTagGroupNum(LMSProjFile* pProj) {
    if (pProj->mTagGroupBlockIndex == -1) {
        return 0;
    }
    return *(const libms_u16_t*)LMSi_GetProjectBlockData(pProj, pProj->mTagGroupBlockIndex);
}

/**
 * @param pProj the project file
 * @param groupIndex tag group index
 * @return the number of tags in the group, or 0.
 */
libms_s32_t LMS_GetTagNum(LMSProjFile* pProj, libms_u16_t groupIndex) {
    const LMSTagGroupInfo* pGroup = LMSi_GetTagGroupInfo(pProj, groupIndex);
    if (pGroup == NULL) {
        return 0;
    }
    return pGroup->mTagNum;
}

/**
 * @param pProj the project file
 * @param groupIndex tag group index
 * @param tagIndex tag index inside the group
 * @return the number of parameters of the tag, or 0.
 */
libms_s32_t LMS_GetTagParamNum(LMSProjFile* pProj, libms_u16_t groupIndex, libms_u16_t tagIndex) {
    const LMSTagInfo* pTag = LMSi_GetTagInfo(pProj, groupIndex, tagIndex);
    if (pTag == NULL) {
        return 0;
    }
    return pTag->mParamNum;
}

/**
 * @param pProj the project file
 * @param groupIndex tag group index
 * @param tagIndex tag index inside the group
 * @param paramIndex index of a list parameter inside the tag
 * @return the number of list items, or 0.
 */
libms_s32_t LMS_GetTagListItemNum(LMSProjFile* pProj, libms_u16_t groupIndex, libms_u16_t tagIndex,
                                  libms_u16_t paramIndex) {
    const LMSTagParamInfo* pParam = LMSi_GetTagParamInfo(pProj, groupIndex, tagIndex, paramIndex);
    if (pParam == NULL) {
        return 0;
    }
    if (pParam->mType != 9) {
        return 0;
    }
    return pParam->mListItemNum;
}

/**
 * @param pProj the project file
 * @param pName style name
 * @return the index of the style named pName, -1 if it is not found, or -9 if there is no SLB1 block.
 */
libms_s32_t LMS_GetStyleIndexByName(LMSProjFile* pProj, const char* pName) {
    LMSBlockInfo* pBlocks;
    int length;
    if (pProj->mStyleNameBlockIndex == -1) {
        return -9;
    }
    pBlocks = pProj->commonInfo.mBlockInfo;
    length = LMSi_GetLabelLength(pName);
    return LMSi_SearchLabel(&pBlocks[pProj->mStyleNameBlockIndex], pName, length);
}

/**
 * @param pProj the project file
 * @return the number of styles, or 0.
 */
libms_s32_t LMS_GetStyleNum(LMSProjFile* pProj) {
    if (pProj->mStyleBlockIndex == -1) {
        return 0;
    }
    return *(const libms_s32_t*)LMSi_GetProjectBlockData(pProj, pProj->mStyleBlockIndex);
}

/**
 * @param pProj the project file
 * @param index style index
 * @return the region width of style number index, or -1.
 */
libms_s32_t LMS_GetRegionWidth(LMSProjFile* pProj, libms_s32_t index) {
    const char* pData;
    if (pProj->mStyleBlockIndex == -1) {
        return -1;
    }
    pData = LMSi_GetProjectBlockData(pProj, pProj->mStyleBlockIndex);
    if (*(const libms_u32_t*)pData <= (libms_u32_t)index) {
        return -1;
    }
    return ((const LMSStyleInfo*)(pData + 4))[index].mRegionWidth;
}

/**
 * @param pProj the project file
 * @param pName style name
 * @return the region width of the style named pName, or a negative error.
 */
libms_s32_t LMS_GetRegionWidthByName(LMSProjFile* pProj, const char* pName) {
    libms_s32_t index = LMS_GetStyleIndexByName(pProj, pName);
    if (index < 0) {
        return index;
    }
    return LMS_GetRegionWidth(pProj, index);
}

/**
 * @param pProj the project file
 * @param index style index
 * @return the line count of style number index, or -1.
 */
libms_s32_t LMS_GetLineNum(LMSProjFile* pProj, libms_s32_t index) {
    const char* pData;
    if (pProj->mStyleBlockIndex == -1) {
        return -1;
    }
    pData = LMSi_GetProjectBlockData(pProj, pProj->mStyleBlockIndex);
    if (*(const libms_u32_t*)pData <= (libms_u32_t)index) {
        return -1;
    }
    return ((const LMSStyleInfo*)(pData + 4))[index].mLineNum;
}

/**
 * @param pProj the project file
 * @param pName style name
 * @return the line count of the style named pName, or a negative error.
 */
libms_s32_t LMS_GetLineNumByName(LMSProjFile* pProj, const char* pName) {
    libms_s32_t index = LMS_GetStyleIndexByName(pProj, pName);
    if (index < 0) {
        return index;
    }
    return LMS_GetLineNum(pProj, index);
}

/**
 * @param pProj the project file
 * @param index style index
 * @return the font index of style number index, -12 if it has none, or -1.
 */
libms_s32_t LMS_GetFontIndex(LMSProjFile* pProj, libms_s32_t index) {
    const char* pData;
    libms_s32_t value;
    if (pProj->mStyleBlockIndex == -1) {
        return -1;
    }
    pData = LMSi_GetProjectBlockData(pProj, pProj->mStyleBlockIndex);
    if (*(const libms_u32_t*)pData <= (libms_u32_t)index) {
        return -1;
    }
    value = ((const LMSStyleInfo*)(pData + 4))[index].mFontIndex;
    if (value == -1) {
        return -12;
    }
    return value;
}

/**
 * @param pProj the project file
 * @param pName style name
 * @return the font index of the style named pName, or a negative error.
 */
libms_s32_t LMS_GetFontIndexByName(LMSProjFile* pProj, const char* pName) {
    libms_s32_t index = LMS_GetStyleIndexByName(pProj, pName);
    if (index < 0) {
        return index;
    }
    return LMS_GetFontIndex(pProj, index);
}

/**
 * @param pProj the project file
 * @param index style index
 * @return the base color index of style number index, -12 if it has none, or -1.
 */
libms_s32_t LMS_GetBaseColorIndex(LMSProjFile* pProj, libms_s32_t index) {
    const char* pData;
    libms_s32_t value;
    if (pProj->mStyleBlockIndex == -1) {
        return -1;
    }
    pData = LMSi_GetProjectBlockData(pProj, pProj->mStyleBlockIndex);
    if (*(const libms_u32_t*)pData <= (libms_u32_t)index) {
        return -1;
    }
    value = ((const LMSStyleInfo*)(pData + 4))[index].mBaseColorIndex;
    if (value == -1) {
        return -12;
    }
    return value;
}

/**
 * @param pProj the project file
 * @param pName style name
 * @return the base color index of the style named pName, or a negative error.
 */
libms_s32_t LMS_GetBaseColorIndexByName(LMSProjFile* pProj, const char* pName) {
    libms_s32_t index = LMS_GetStyleIndexByName(pProj, pName);
    if (index < 0) {
        return index;
    }
    return LMS_GetBaseColorIndex(pProj, index);
}
