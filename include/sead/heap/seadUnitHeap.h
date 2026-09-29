#pragma once

#include "container/seadFreeList.h"
#include "heap/seadHeap.h"

namespace sead
{
class UnitHeap : public Heap
{
    SEAD_RTTI_OVERRIDE(UnitHeap, Heap)
    friend class PrintFormatter;

public:
    static UnitHeap* create(size_t size, const SafeString& rName, u32 blockSize, s32 alignment,
                            Heap* pParent, bool enableLock);
    static UnitHeap* tryCreate(size_t size, const SafeString& rName, u32 blockSize,
                               s32 alignment, Heap* pParent, bool enableLock);
    static UnitHeap* tryCreateWithBlockNum(u32 blockSize, u32 blockNum, const SafeString& rName,
                                           s32 alignment, Heap* pParent, bool enableLock);

    static size_t getManagementAreaSize(s32 alignment);

    void destroy() override;
    size_t adjust() override;
    void* tryAlloc(size_t size, s32 alignment) override;
    void free(void* pPtr) override;
    void* resizeFront(void* pPtr, size_t size) override;
    void* resizeBack(void* pPtr, size_t size) override;
    void freeAll() override;
    uintptr_t getStartAddress() const override;
    uintptr_t getEndAddress() const override;
    size_t getSize() const override;
    size_t getFreeSize() const override;
    size_t getMaxAllocatableSize(int alignment) const override;
    bool isInclude(const void* pPtr) const override;
    bool isEmpty() const override;
    bool isFreeable() const override;
    bool isResizable() const override;
    bool isAdjustable() const override;
    void dump() const override;
    void dumpYAML(WriteStream& rStream, int indent) const override;
    void genInformation_(hostio::Context* pContext) override;

    u32 getBlockSize() const { return mBlockSize; }
    s32 getBlockNum() const { return mAreaSize / mBlockSize; }

protected:
    UnitHeap(const SafeString& rName, Heap* pParent, void* pAddress, size_t size, u32 blockSize,
             bool enableLock);
    ~UnitHeap() override;

    void doCreate(s32 alignment, bool isPadded, Heap* pParent);
    void resetFreeList_();

    u32 mBlockSize;
    void* mAreaStart;
    size_t mAreaSize;
    size_t mFreeSize;
    FreeList mFreeList;
};
}  // namespace sead
