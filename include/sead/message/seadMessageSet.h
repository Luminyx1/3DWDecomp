#pragma once

#include "basis/seadTypes.h"
#include "lms/lms.h"
#include "prim/seadSafeString.h"

namespace sead {
class Heap;

class MessageSetBase {
public:
    virtual ~MessageSetBase();

    bool initialize(void* pData, Heap* pHeap);
    void finalize();
    const void* getInitializeData() const;
    s32 calcTextSizeByIndex(s32 index) const;
    bool searchTextLabelByIndex(BufferedSafeString* pLabel, s32 index) const;

    static void* allocForLibms_(size_t size);
    static void freeForLibms_(void* pPtr);

protected:
    LMSMsgFile* mMsgFile = nullptr;
    s32 mTextNum = 0;

    static Heap* sHeap;
};

template <typename T>
class MessageSet : public MessageSetBase {
public:
    struct TagInfo;
    MessageSet() = default;
    ~MessageSet() override = default;

    s32 getTextNum() const { return mTextNum; }

    const T* getText(s32 index) const {
        if (static_cast<u32>(index) < static_cast<u32>(mTextNum))
            return static_cast<const T*>(LMS_GetText(mMsgFile, index));
        return nullptr;
    }

    const T* getTextByLabel(const char* pLabel) const {
        return static_cast<const T*>(LMS_GetTextByLabel(mMsgFile, pLabel));
    }

    s32 getTextIndexByLabel(const char* pLabel) const {
        return LMS_GetTextIndexByLabel(mMsgFile, pLabel);
    }

    s32 getTextStyle(s32 index) const {
        if (static_cast<u32>(index) < static_cast<u32>(mTextNum))
            return LMS_GetTextStyle(mMsgFile, index);
        return -1;
    }

    s32 getTextStyleByLabel(const char* pLabel) const {
        return LMS_GetTextStyleByLabel(mMsgFile, pLabel);
    }
};

template <typename T>
struct MessageSet<T>::TagInfo {
    T mTagMark;
    u16 mGroup;
    u16 mType;
    u16 mParamSize;
};
}  // namespace sead
