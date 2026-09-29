#include "lms/lms.h"

/**
 * Reads the encoding, block count and file size from an LMS binary header and allocates its block table.
 */
void LMSi_AnalyzeMessageHeader(LMSCommonBinaryFormat* pBin) {
    pBin->mEncoding = pBin->pResource[12];
    pBin->mBlockNum = *(const libms_u16_t*)(pBin->pResource + 14);
    if (pBin->mBlockNum != 0) {
        pBin->mBlockInfo = (LMSBlockInfo*)LMSi_Malloc(pBin->mBlockNum * sizeof(LMSBlockInfo));
    } else {
        pBin->mBlockInfo = NULL;
    }
    pBin->mResourceLength = *(const libms_u32_t*)(pBin->pResource + 18);
}

/**
 * Fills the block table: each block is a 16-byte header (name, size, flags) followed by 16-byte aligned data.
 */
void LMSi_AnalyzeMessageBlocks(LMSCommonBinaryFormat* pBin) {
    size_t offset = 0x20;
    int i;
    size_t pos;
    for (i = 0; i < pBin->mBlockNum; i++) {
        LMSBlockInfo* pInfo = &pBin->mBlockInfo[i];
        pInfo->pData = pBin->pResource + offset + 0x10;
        pos = offset;
        pInfo->mBlockName[0] = pBin->pResource[pos++];
        pInfo->mBlockName[1] = pBin->pResource[pos++];
        pInfo->mBlockName[2] = pBin->pResource[pos++];
        pInfo->mBlockName[3] = pBin->pResource[pos++];
        pInfo->mDataSize = *(const libms_u32_t*)(pBin->pResource + pos);
        pos += 4;
        pInfo->mUnk10 = *(const libms_u16_t*)(pBin->pResource + pos);
        offset = (offset + 0x10 + pInfo->mDataSize + 0xF) & ~(size_t)0xF;
    }
}

/**
 * Parses the header and block table of an LMS binary. pMagic and version are not checked.
 */
void LMSi_AnalyzeMessageBinary(LMSCommonBinaryFormat* pBin, const char* pMagic, int version) {
    LMSi_AnalyzeMessageHeader(pBin);
    LMSi_AnalyzeMessageBlocks(pBin);
}

/**
 * @return the index of the block named pName (4 characters), or -1.
 */
libms_s32_t LMSi_SearchBlockByName(LMSCommonBinaryFormat* pBin, const char* pName) {
    int i;
    for (i = 0; i < pBin->mBlockNum; i++) {
        if (LMSi_MemCmp(pBin->mBlockInfo[i].mBlockName, pName, 4)) {
            return i;
        }
    }
    return -1;
}

/**
 * @return the block named pName (4 characters), or NULL.
 */
LMSBlockInfo* LMSi_GetBlockInfoByName(LMSCommonBinaryFormat* pBin, const char* pName) {
    int i;
    for (i = 0; i < pBin->mBlockNum; i++) {
        if (LMSi_MemCmp(pBin->mBlockInfo[i].mBlockName, pName, 4)) {
            return &pBin->mBlockInfo[i];
        }
    }
    return NULL;
}

/**
 * Hashes a label (hash = hash * 0x492 + c) into one of numSlots label hash table slots.
 */
libms_u32_t LMSi_GetHashTableIndexFromLabel(const char* pLabel, libms_u32_t numSlots) {
    libms_u32_t hash = 0;
    for (;;) {
        char c = *pLabel++;
        if (c == '\0') {
            break;
        }
        hash = hash * 0x492 + c;
    }
    return hash % numSlots;
}
