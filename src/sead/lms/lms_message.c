#include "lms/lms.h"

LMSMsgFile* LMS_InitMessage(const char* data) {
    LMSMsgFile* msg = (LMSMsgFile*)LMSi_Malloc(sizeof(LMSMsgFile));
    msg->commonInfo.pResource = data;
    LMSi_AnalyzeMessageBinary(&msg->commonInfo, "MsgStdBn", 3);
    msg->mLabelBlockIndex = LMSi_SearchBlockByName(&msg->commonInfo, "LBL1");
    msg->mTextBlockIndex = LMSi_SearchBlockByName(&msg->commonInfo, "TXT2");
    msg->mAttributeBlockIndex = LMSi_SearchBlockByName(&msg->commonInfo, "ATR1");
    msg->mATOBlockIndex = LMSi_SearchBlockByName(&msg->commonInfo, "ATO1");
    msg->mStyleBlockIndex = LMSi_SearchBlockByName(&msg->commonInfo, "TSY1");
    return msg;
}

void LMS_CloseMessage(LMSMsgFile* msg) {
    if (msg->commonInfo.mBlockInfo)
        LMSi_Free(msg->commonInfo.mBlockInfo);
    LMSi_Free(msg);
}

libms_s32_t LMS_SearchMessageBlockByName(LMSMsgFile* msg, const char* name) {
    return LMSi_SearchBlockByName(&msg->commonInfo, name);
}

LMSBlockInfo* LMS_GetMessageBlockInfoByName(LMSMsgFile* msg, const char* name) {
    return LMSi_GetBlockInfoByName(&msg->commonInfo, name);
}

libms_s32_t LMS_GetTextNum(LMSMsgFile* msg) {
    if (msg->mTextBlockIndex == -1)
        return -1;
    return *(const libms_s32_t*)msg->commonInfo.mBlockInfo[msg->mTextBlockIndex].pData;
}

/* LBL1 is a hash table: u32 slot count, then per slot {u32 labelCount, u32 offset};
 * each label entry is {u8 length, char name[length], u32 textIndex}. */
libms_s32_t LMS_GetTextIndexByLabel(LMSMsgFile* msg, const char* label) {
    LMSBlockInfo* block;
    int slot;
    libms_u32_t count;
    libms_u32_t offset;
    libms_u32_t i;
    int length;

    if (msg->mLabelBlockIndex == -1)
        return -2;

    length = 0;
    while (label[length++] != '\0') {
    }

    block = &msg->commonInfo.mBlockInfo[msg->mLabelBlockIndex];
    slot = LMSi_GetHashTableIndexFromLabel(label, *(const libms_u32_t*)block->pData);
    count = ((const libms_u32_t*)block->pData)[slot * 2 + 1];
    if (count == 0)
        return -1;

    offset = ((const libms_u32_t*)block->pData)[slot * 2 + 2];
    for (i = 0; i < count; i++) {
        const char* entry = block->pData;
        libms_u8_t len = entry[offset];
        if (len + 1 == length && LMSi_MemCmp(label, entry + offset + 1, length - 1))
            return *(const libms_s32_t*)(block->pData + offset + 1 + len);
        offset += len + 5;
    }
    return -1;
}

const void* LMS_GetText(LMSMsgFile* msg, libms_s32_t index) {
    const char* data;
    if (msg->mTextBlockIndex == -1)
        return NULL;
    data = msg->commonInfo.mBlockInfo[msg->mTextBlockIndex].pData;
    if (*(const libms_s32_t*)data <= index)
        return NULL;
    return data + ((const libms_u32_t*)data)[index + 1];
}

/* Size in bytes of a text, up to (not including) its terminator.  Tags are
 * 0x0E <group> <type> <paramSize> <params...>; 0x0F closes a tag. */
libms_s32_t LMS_GetTextSize(LMSMsgFile* msg, libms_s32_t index) {
    const char* data;
    const char* text;
    const char* p;

    if (msg->mTextBlockIndex == -1)
        return -1;
    data = msg->commonInfo.mBlockInfo[msg->mTextBlockIndex].pData;
    if (*(const libms_s32_t*)data <= index)
        return -1;
    text = data + ((const libms_u32_t*)data)[index + 1];
    if (text == NULL)
        return -1;

    p = text;
    switch (msg->commonInfo.mEncoding) {
    case 0:
        for (;;) {
            libms_u8_t c = *(const libms_u8_t*)p;
            if (c == 0x0E)
                p += *(const libms_u16_t*)(p + 5) + 7;
            else if (c == 0x0F)
                p += 6;
            else if (c == 0)
                break;
            else
                p += 1;
        }
        break;
    case 1:
        for (;;) {
            libms_u16_t c = *(const libms_u16_t*)p;
            if (c == 0x0E)
                p += *(const libms_u16_t*)(p + 6) + 8;
            else if (c == 0x0F)
                p += 6;
            else if (c == 0)
                break;
            else
                p += 2;
        }
        break;
    case 2:
        for (;;) {
            libms_u32_t c = *(const libms_u32_t*)p;
            if (c == 0x0E)
                p += *(const libms_u16_t*)(p + 8) + 10;
            else if (c == 0x0F)
                p += 6;
            else if (c == 0)
                break;
            else
                p += 4;
        }
        break;
    default:
        return -1;
    }
    return p - text;
}

const void* LMS_GetTextByLabel(LMSMsgFile* msg, const char* label) {
    libms_s32_t index = LMS_GetTextIndexByLabel(msg, label);
    if (index < 0)
        return NULL;
    return LMS_GetText(msg, index);
}

int LMS_GetLabelByTextIndex(LMSMsgFile* msg, libms_s32_t index, char* outLabel) {
    LMSBlockInfo* block = &msg->commonInfo.mBlockInfo[msg->mLabelBlockIndex];
    const char* data = block->pData;
    libms_u32_t offset = *(const libms_u32_t*)data * 8 + 4;

    while (offset < block->mDataSize) {
        libms_u8_t len = data[offset];
        if (*(const libms_s32_t*)(data + offset + 1 + len) == index) {
            LMSi_MemCopy(outLabel, data + offset + 1, len);
            outLabel[len] = '\0';
            return 1;
        }
        offset += len + 5;
    }
    return 0;
}

libms_s32_t LMS_GetAttributeSize(LMSMsgFile* msg) {
    return ((const libms_s32_t*)msg->commonInfo.mBlockInfo[msg->mAttributeBlockIndex].pData)[1];
}

const void* LMS_GetAttribute(LMSMsgFile* msg, libms_s32_t index) {
    const char* data = msg->commonInfo.mBlockInfo[msg->mAttributeBlockIndex].pData;
    return data + ((const libms_u32_t*)data)[1] * index + 8;
}

const char* LMS_GetAttributeText(LMSMsgFile* msg, libms_s32_t offset) {
    return msg->commonInfo.mBlockInfo[msg->mAttributeBlockIndex].pData + offset;
}

libms_s32_t LMS_GetAttrFilteredOffset(LMSMsgFile* msg, libms_s32_t index) {
    if (msg->mATOBlockIndex == -1)
        return -11;
    return ((const libms_s32_t*)msg->commonInfo.mBlockInfo[msg->mATOBlockIndex].pData)[index];
}

libms_s32_t LMS_GetTextStyle(LMSMsgFile* msg, libms_s32_t index) {
    if (msg->mStyleBlockIndex == -1)
        return -3;
    return ((const libms_s32_t*)msg->commonInfo.mBlockInfo[msg->mStyleBlockIndex].pData)[index];
}

libms_s32_t LMS_GetTextStyleByLabel(LMSMsgFile* msg, const char* label) {
    libms_s32_t index = LMS_GetTextIndexByLabel(msg, label);
    if (index < 0)
        return index;
    return LMS_GetTextStyle(msg, index);
}
