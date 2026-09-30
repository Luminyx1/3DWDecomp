#include "Library/Message/MessageTagDataHolder.hpp"

#include "Library/Message/MessageTagData.hpp"
#include "Library/Message/MessageHolder.hpp"

namespace al {
/**
 * Creates a holder with room for a fixed number of tag data entries.
 * @param maxNum maximum number of entries
 */
MessageTagDataHolder::MessageTagDataHolder(s32 maxNum) : mNum(0), mMaxNum(maxNum) {
    mData = new MessageTagDataBase*[maxNum];
    for (s32 i = 0; i < mMaxNum; i++) {
        mData[i] = nullptr;
    }
}

/**
 * Adds a tag data entry.
 * @param pData entry to add
 */
void MessageTagDataHolder::registerMessageTagData(MessageTagDataBase* pData) {
    mData[mNum] = pData;
    mNum++;
}

/**
 * Applies every registered tag data entry to a message.
 * @param pString output string
 * @param pMsgSystem message system
 * @param pMessage source message
 */
void MessageTagDataHolder::replaceMessage(sead::BufferedSafeStringBase<char16_t>* pString,
                                          const IUseMessageSystem* pMsgSystem,
                                          const char16_t* pMessage) const {
    char16_t buffer[0x400];
    copyMessageWithTag(buffer, 0x400, pMessage);
    for (s32 i = 0; i < mNum; i++) {
        mData[i]->replaceMessage(pString, pMsgSystem, buffer);
        copyMessageWithTag(buffer, 0x400, pString->cstr());
    }
}
}  // namespace al
