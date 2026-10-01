#include "message/seadMessageSet.h"

#include "heap/seadHeap.h"

namespace sead {

Heap* MessageSetBase::sHeap = nullptr;

/**
 * Destroys the message set.
 */
MessageSetBase::~MessageSetBase() = default;

/**
 * Opens an MSBT file, allocating LibMessageStudio's bookkeeping from pHeap.
 * @param pData the MSBT file
 * @param pHeap heap for the LibMessageStudio allocations
 * @return true if the file has a TXT2 block.
 */
bool MessageSetBase::initialize(void* pData, Heap* pHeap) {
    sHeap = pHeap;
    LMS_SetMemFuncs(allocForLibms_, freeForLibms_);
    mMsgFile = LMS_InitMessage(static_cast<const char*>(pData));
    s32 textNum = LMS_GetTextNum(mMsgFile);
    s32 num = textNum;

    if (textNum < 0) {
        LMS_CloseMessage(mMsgFile);
        num = 0;
        mMsgFile = nullptr;
    }

    mTextNum = num;
    LMS_SetMemFuncs(nullptr, nullptr);
    sHeap = nullptr;
    return textNum >= 0;
}

/**
 * Allocation callback handed to LibMessageStudio.
 * @param size number of bytes
 * @return the allocated memory.
 */
void* MessageSetBase::allocForLibms_(size_t size) {
    return new (sHeap, 8) u8[size];
}

/**
 * Free callback handed to LibMessageStudio.
 * @param pPtr memory from allocForLibms_
 */
void MessageSetBase::freeForLibms_(void* pPtr) {
    delete[] static_cast<u8*>(pPtr);
}

/**
 * Closes the MSBT file.
 */
void MessageSetBase::finalize() {
    LMS_SetMemFuncs(nullptr, freeForLibms_);
    LMS_CloseMessage(mMsgFile);
    mMsgFile = nullptr;
    mTextNum = 0;
    LMS_SetMemFuncs(nullptr, nullptr);
}

/**
 * @return the MSBT data passed to initialize, or nullptr.
 */
const void* MessageSetBase::getInitializeData() const {
    if (mMsgFile != nullptr) {
        return mMsgFile->commonInfo.pResource;
    }

    return nullptr;
}

/**
 * @param index text index
 * @return the size in bytes of text number index, or 0.
 */
s32 MessageSetBase::calcTextSizeByIndex(s32 index) const {
    if (static_cast<u32>(index) < static_cast<u32>(mTextNum)) {
        return LMS_GetTextSize(mMsgFile, index);
    }

    return 0;
}

/**
 * Copies the label of a text into pLabel, which needs room for 256 characters.
 * @param pLabel receives the label (cleared on failure)
 * @param index text index
 * @return true if index is valid.
 */
bool MessageSetBase::searchTextLabelByIndex(BufferedSafeString* pLabel, s32 index) const {
    if (pLabel->getBufferSize() >= 0x100) {
        if (static_cast<u32>(index) >= static_cast<u32>(mTextNum)) {
            pLabel->clear();
            return false;
        }

        LMS_GetLabelByTextIndex(mMsgFile, index, pLabel->getBuffer());
        return true;
    }

    pLabel->clear();
    return false;
}

}  // namespace sead
