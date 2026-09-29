#include "lms/lms.h"

/**
 * Opens an MSBT file in memory and locates its LBL1, TXT2, ATR1, ATO1 and TSY1 blocks.
 * @param pBlockData
 */
LMSMsgFile* LMS_InitMessage(const char* pBlockData) {
    LMSMsgFile* pMsg = (LMSMsgFile*)LMSi_Malloc(sizeof(LMSMsgFile));
    pMsg->commonInfo.pResource = pBlockData;
    LMSi_AnalyzeMessageBinary(&pMsg->commonInfo, "MsgStdBn", 3);
    pMsg->mLabelBlockIndex = LMSi_SearchBlockByName(&pMsg->commonInfo, "LBL1");
    pMsg->mTextBlockIndex = LMSi_SearchBlockByName(&pMsg->commonInfo, "TXT2");
    pMsg->mAttributeBlockIndex = LMSi_SearchBlockByName(&pMsg->commonInfo, "ATR1");
    pMsg->mATOBlockIndex = LMSi_SearchBlockByName(&pMsg->commonInfo, "ATO1");
    pMsg->mStyleBlockIndex = LMSi_SearchBlockByName(&pMsg->commonInfo, "TSY1");
    return pMsg;
}

/**
 * Frees a message file opened with LMS_InitMessage (the data itself is not freed).
 * @param pMsg the message file
 */
void LMS_CloseMessage(LMSMsgFile* pMsg) {
    if (pMsg->commonInfo.mBlockInfo) {
        LMSi_Free(pMsg->commonInfo.mBlockInfo);
    }
    LMSi_Free(pMsg);
}

/**
 * @param pMsg the message file
 * @param pName 4-character block name, e.g. "TXT2"
 * @return the index of the block named pName, or -1.
 */
libms_s32_t LMS_SearchMessageBlockByName(LMSMsgFile* pMsg, const char* pName) {
    return LMSi_SearchBlockByName(&pMsg->commonInfo, pName);
}

/**
 * @param pMsg the message file
 * @param pName 4-character block name, e.g. "TXT2"
 * @return the block named pName, or NULL.
 */
LMSBlockInfo* LMS_GetMessageBlockInfoByName(LMSMsgFile* pMsg, const char* pName) {
    return LMSi_GetBlockInfoByName(&pMsg->commonInfo, pName);
}

/**
 * @param pMsg the message file
 * @return the number of texts in the TXT2 block, or -1 if there is none.
 */
libms_s32_t LMS_GetTextNum(LMSMsgFile* pMsg) {
    if (pMsg->mTextBlockIndex == -1) {
        return -1;
    }
    return *(const libms_s32_t*)pMsg->commonInfo.mBlockInfo[pMsg->mTextBlockIndex].pData;
}

/**
 * Looks up a label in the LBL1 hash table (u32 slot count, then per slot {u32 labelCount, u32 offset};
 * each entry is {u8 length, char name[length], u32 textIndex}).
 * @param pMsg the message file
 * @param pLabel label to look up
 * @return the text index of pLabel, -1 if it is not found, or -2 if there is no LBL1 block.
 */
libms_s32_t LMS_GetTextIndexByLabel(LMSMsgFile* pMsg, const char* pLabel) {
    LMSBlockInfo* pBlock;
    libms_u32_t hash;
    libms_u32_t count;
    libms_u32_t offset;
    libms_u32_t i;
    int length;

    if (pMsg->mLabelBlockIndex == -1) {
        return -2;
    }

    length = 0;
    while (pLabel[length++] != '\0') {
    }

    pBlock = &pMsg->commonInfo.mBlockInfo[pMsg->mLabelBlockIndex];
    hash = LMSi_GetHashTableIndexFromLabel(pLabel, *(const libms_u32_t*)pBlock->pData) * 2 + 1;
    count = ((const libms_u32_t*)pBlock->pData)[(int)hash];
    if (count == 0) {
        return -1;
    }

    offset = ((const libms_u32_t*)pBlock->pData)[(int)(hash + 1)];
    for (i = 0; i < count; i++) {
        libms_u8_t len = pBlock->pData[offset];
        if (len + 1 == length && LMSi_MemCmp(pLabel, pBlock->pData + (offset + sizeof(libms_u8_t)), length - 1)) {
            return *(const libms_s32_t*)(pBlock->pData + (offset + sizeof(libms_u8_t) + len));
        }
        offset += len + 5;
    }
    return -1;
}

/**
 * @param pMsg the message file
 * @param index text index
 * @return text number index from the TXT2 block, or NULL.
 */
const void* LMS_GetText(LMSMsgFile* pMsg, libms_s32_t index) {
    const char* pBlockData;
    if (pMsg->mTextBlockIndex == -1) {
        return NULL;
    }
    pBlockData = pMsg->commonInfo.mBlockInfo[pMsg->mTextBlockIndex].pData;
    if (*(const libms_s32_t*)pBlockData <= index) {
        return NULL;
    }
    return pBlockData + ((const libms_u32_t*)pBlockData)[index + 1];
}

/**
 * Measures a text. Tags are 0x0E [group] [type] [paramSize] [params...]; 0x0F closes a tag.
 * @param pMsg the message file
 * @param index text index
 * @return the size in bytes of text number index (tags included, terminator excluded), or -1.
 */
libms_s32_t LMS_GetTextSize(LMSMsgFile* pMsg, libms_s32_t index) {
    const char* pBlockData;
    const char* pText;
    const char* pCur;

    if (pMsg->mTextBlockIndex == -1) {
        return -1;
    }
    pBlockData = pMsg->commonInfo.mBlockInfo[pMsg->mTextBlockIndex].pData;
    if (*(const libms_s32_t*)pBlockData <= index) {
        return -1;
    }
    pText = pBlockData + ((const libms_u32_t*)pBlockData)[index + 1];
    if (pText == NULL) {
        return -1;
    }

    pCur = pText;
    switch (pMsg->commonInfo.mEncoding) {
    case 0:
        for (;;) {
            libms_u8_t c = *(const libms_u8_t*)pCur;
            if (c == 0x0E) {
                pCur += *(const libms_u16_t*)(pCur + 5) + 7;
            } else if (c == 0x0F) {
                pCur += 6;
            } else if (c == 0) {
                break;
            } else {
                pCur += 1;
            }
        }
        break;
    case 1:
        for (;;) {
            libms_u16_t c = *(const libms_u16_t*)pCur;
            if (c == 0x0E) {
                pCur += *(const libms_u16_t*)(pCur + 6) + 8;
            } else if (c == 0x0F) {
                pCur += 6;
            } else if (c == 0) {
                break;
            } else {
                pCur += 2;
            }
        }
        break;
    case 2:
        for (;;) {
            libms_u32_t c = *(const libms_u32_t*)pCur;
            if (c == 0x0E) {
                pCur += *(const libms_u16_t*)(pCur + 8) + 10;
            } else if (c == 0x0F) {
                pCur += 6;
            } else if (c == 0) {
                break;
            } else {
                pCur += 4;
            }
        }
        break;
    default:
        return -1;
    }
    return pCur - pText;
}

/**
 * @param pMsg the message file
 * @param pLabel label to look up
 * @return the text labelled pLabel, or NULL.
 */
const void* LMS_GetTextByLabel(LMSMsgFile* pMsg, const char* pLabel) {
    libms_s32_t index = LMS_GetTextIndexByLabel(pMsg, pLabel);
    if (index < 0) {
        return NULL;
    }
    return LMS_GetText(pMsg, index);
}

/**
 * Copies the label of a text into pOutLabel by scanning every LBL1 entry.
 * @param pMsg the message file
 * @param index text index
 * @param pOutLabel receives the NUL-terminated label
 * @return 1 if a label was found, otherwise 0.
 */
int LMS_GetLabelByTextIndex(LMSMsgFile* pMsg, libms_s32_t index, char* pOutLabel) {
    LMSBlockInfo* pBlock = &pMsg->commonInfo.mBlockInfo[pMsg->mLabelBlockIndex];
    libms_u32_t offset = *(const libms_u32_t*)pBlock->pData * 8 + 4;

    while (offset < pBlock->mDataSize) {
        libms_u8_t len = pBlock->pData[offset];
        if (*(const libms_s32_t*)(&pBlock->pData[offset + sizeof(libms_u8_t) + len]) == index) {
            LMSi_MemCopy(pOutLabel, &pBlock->pData[offset + 1], len);
            pOutLabel[len] = '\0';
            return 1;
        }
        offset += len + 5;
    }
    return 0;
}

/**
 * @param pMsg the message file
 * @return the size of one attribute entry in the ATR1 block.
 */
libms_s32_t LMS_GetAttributeSize(LMSMsgFile* pMsg) {
    return ((const libms_s32_t*)pMsg->commonInfo.mBlockInfo[pMsg->mAttributeBlockIndex].pData)[1];
}

/**
 * @param pMsg the message file
 * @param index text index
 * @return the attribute entry of text number index.
 */
const void* LMS_GetAttribute(LMSMsgFile* pMsg, libms_s32_t index) {
    const char* pBlockData = pMsg->commonInfo.mBlockInfo[pMsg->mAttributeBlockIndex].pData;
    return pBlockData + ((const libms_u32_t*)pBlockData)[1] * index + 8;
}

/**
 * @param pMsg the message file
 * @param offset byte offset into the ATR1 block
 * @return the string at offset in the ATR1 block.
 */
const char* LMS_GetAttributeText(LMSMsgFile* pMsg, libms_s32_t offset) {
    return pMsg->commonInfo.mBlockInfo[pMsg->mAttributeBlockIndex].pData + offset;
}

/**
 * @param pMsg the message file
 * @param index text index
 * @return entry index of the ATO1 block, or -11 if there is none.
 */
libms_s32_t LMS_GetAttrFilteredOffset(LMSMsgFile* pMsg, libms_s32_t index) {
    if (pMsg->mATOBlockIndex == -1) {
        return -11;
    }
    return ((const libms_s32_t*)pMsg->commonInfo.mBlockInfo[pMsg->mATOBlockIndex].pData)[index];
}

/**
 * @param pMsg the message file
 * @param index text index
 * @return the style of text number index from the TSY1 block, or -3 if there is none.
 */
libms_s32_t LMS_GetTextStyle(LMSMsgFile* pMsg, libms_s32_t index) {
    if (pMsg->mStyleBlockIndex == -1) {
        return -3;
    }
    return ((const libms_s32_t*)pMsg->commonInfo.mBlockInfo[pMsg->mStyleBlockIndex].pData)[index];
}

/**
 * @param pMsg the message file
 * @param pLabel label to look up
 * @return the style of the text labelled pLabel, or a negative error from LMS_GetTextIndexByLabel.
 */
libms_s32_t LMS_GetTextStyleByLabel(LMSMsgFile* pMsg, const char* pLabel) {
    libms_s32_t index = LMS_GetTextIndexByLabel(pMsg, pLabel);
    if (index < 0) {
        return index;
    }
    return LMS_GetTextStyle(pMsg, index);
}
