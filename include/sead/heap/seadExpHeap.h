#pragma once

#include "heap/seadHeap.h"
#include "heap/seadMemBlock.h"
#include "prim/seadSizedEnum.h"

namespace sead
{
class ExpHeap : public Heap
{
    SEAD_RTTI_OVERRIDE(ExpHeap, Heap)
    friend class PrintFormatter;

public:
    enum class AllocMode
    {
        FirstFit = 0,
        BestFit = 1,
    };

    enum class FindFreeBlockMode
    {
        Auto = 0,
        FromFreeList = 1,
        ByIteratingMemBlock = 2,
    };

    enum class FindMode
    {
        FirstFit = 0,
        BestFit = 1,
        LargestFit = 2,
    };

    static ExpHeap* create(size_t size, const SafeString& name, Heap* pParent,
                           s32 alignment = sizeof(void*),
                           HeapDirection direction = cHeapDirection_Forward,
                           bool enableLock = false);
    static ExpHeap* create(void* pAddress, size_t size, const SafeString& name,
                           bool enableLock = false);
    static ExpHeap* create(void* pAddress, size_t size, const SafeString& name, Heap* pParent,
                           bool enableLock);

    static ExpHeap* tryCreate(size_t size, const SafeString& name, Heap* pParent,
                              s32 alignment = sizeof(void*),
                              HeapDirection direction = cHeapDirection_Forward,
                              bool enableLock = false);
    static ExpHeap* tryCreate(void* pAddress, size_t size, const SafeString& name,
                              bool enableLock = false);
    static ExpHeap* tryCreate(void* pAddress, size_t size, const SafeString& name, Heap* pParent,
                              bool enableLock);

    static size_t getManagementAreaSize(s32 alignment);
    static size_t getPerAllocationOverhead(s32 alignment);

    void destroy() override;
    size_t adjust() override;
    void* tryAlloc(size_t size, s32 alignment) override;
    void free(void* pPtr) override;
    size_t freeAndGetAllocatableSize(void* pPtr, s32 alignment);
    void* resizeFront(void* pPtr, size_t size) override;
    void* resizeBack(void* pPtr, size_t size) override;
    void* tryRealloc(void* pPtr, size_t size, s32 alignment) override;
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

    virtual size_t destroyAndGetAllocatableSize(s32 alignment);
    virtual void setFindFreeBlockMode(FindFreeBlockMode mode);

    AllocMode getAllocMode() const { return mAllocMode; }
    void setAllocMode(AllocMode mode) { mAllocMode = mode; }

    // XXX: this isn't const-correct...
    size_t getAllocatedSize(void* pPtr);

    void dumpFreeList() const;
    void dumpUseList() const;

    void checkFreeList() const;
    bool tryCheckFreeList() const;
    void checkUseList() const;
    bool tryCheckUseList() const;

protected:
    ExpHeap(const SafeString& name, Heap* pParent, void* pAddress, size_t size,
            HeapDirection direction, bool enableLock);
    ~ExpHeap() override;

    static void doCreate(ExpHeap* pHeap, Heap* pParent);

    static void createMaxSizeFreeMemBlock_(ExpHeap* pHeap);
    MemBlock* findFreeMemBlockFromHead_(size_t size, FindMode mode) const;
    MemBlock* findFreeMemBlockFromHead_(size_t size, s32 alignment, FindMode mode) const;
    MemBlock* findFreeMemBlockFromTail_(size_t size, FindMode mode) const;
    MemBlock* findFreeMemBlockFromTail_(size_t size, s32 alignment, FindMode mode) const;
    MemBlock* findLastMemBlockIfFree_();
    MemBlock* findFirstMemBlockIfFree_();

    void pushToUseList_(MemBlock* pBlock);
    MemBlock* pushToFreeList_(MemBlock* pBlock);

    void* realloc_(void* pPtr, u8* pMemory, size_t copySize, size_t size, s32 alignment);

    static const char* getFindFreeBlockModeString_(FindFreeBlockMode mode);

    size_t adjustBack_();
    size_t adjustFront_();

    MemBlock* allocFromHead_(size_t size);
    MemBlock* allocFromHead_(size_t size, s32 alignment);
    MemBlock* allocFromTail_(size_t size);
    MemBlock* allocFromTail_(size_t size, s32 alignment);

    static s32 compareMemBlockAddr_(const MemBlock* pA, const MemBlock* pB);

    SizedEnum<AllocMode, u8> mAllocMode;
    SizedEnum<FindFreeBlockMode, u8> mFindFreeBlockMode;
    MemBlockList mFreeList;
    MemBlockList mUseList;
};
}  // namespace sead
