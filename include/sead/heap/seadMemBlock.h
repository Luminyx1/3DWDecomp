#pragma once

#include "basis/seadTypes.h"
#include "container/seadListImpl.h"
#include "container/seadOffsetList.h"

namespace sead
{
class ExpHeap;

class MemBlock
{
public:
    static constexpr u16 cFreeHeapCheckTag = 0xffff;

    MemBlock()
    {
        mSize = 0;
        mHeapCheckTag = 0;
        mOffset = 0;
    }

    static MemBlock* FindManageArea(void* pPtr)
    {
        uintptr_t tag = *(static_cast<uintptr_t*>(pPtr) - 1);
        if (tag & 1)
        {
            return reinterpret_cast<MemBlock*>(tag - 1);
        }
        return static_cast<MemBlock*>(pPtr) - 1;
    }

    static u32 getOffset() { return offsetof(MemBlock, mListNode); }

    u8* getMemory() const
    {
        return reinterpret_cast<u8*>(uintptr_t(this) + mOffset + sizeof(MemBlock));
    }

    u8* getMemoryEnd() const
    {
        return reinterpret_cast<u8*>(uintptr_t(this) + mOffset + mSize + sizeof(MemBlock));
    }

    size_t getTotalSize() const { return sizeof(MemBlock) + mOffset + mSize; }

    bool isFree() const { return s16(mHeapCheckTag) == -1; }

    void markFree() { mHeapCheckTag = cFreeHeapCheckTag; }

    size_t getSize() const { return mSize; }

    void setOffset(u16 offset)
    {
        mOffset = offset;
        if (offset != 0)
        {
            u8* memory = reinterpret_cast<u8*>(this + 1) + offset;
            reinterpret_cast<uintptr_t*>(memory)[-1] = uintptr_t(this) + 1;
        }
    }

private:
    friend class ExpHeap;

    ListNode mListNode;
    u16 mHeapCheckTag;
    u16 mOffset;
    size_t mSize;
};

using MemBlockList = OffsetList<MemBlock>;
}  // namespace sead
