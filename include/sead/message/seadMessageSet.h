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
    MessageSet() = default;
    ~MessageSet() override = default;
};
}  // namespace sead
