#pragma once

#include "container/seadObjList.h"
#include "heap/seadHeap.h"

namespace sead
{
class SeparateHeap : public Heap
{
    SEAD_RTTI_OVERRIDE(SeparateHeap, Heap)
    friend class PrintFormatter;

public:
    struct Block
    {
        void* mAddress;
        size_t mSize;
    };

    static SeparateHeap* create(const SafeString& rName, void* pManagementArea,
                                size_t managementAreaSize, void* pHeapStart, size_t heapSize,
                                bool enableLock);
    static SeparateHeap* create(const SafeString& rName, size_t managementAreaSize,
                                size_t heapSize, Heap* pParent, bool enableLock);
    static SeparateHeap* tryCreate(const SafeString& rName, size_t managementAreaSize,
                                   size_t heapSize, Heap* pParent, bool enableLock);
    static size_t getManagementAreaSize(size_t nodeNum);
    static size_t getAllocateAreaSize(void* pAddress, size_t size, s32 alignment);

    void destroy() override;
    size_t adjust() override { return getSize(); }
    void* tryAlloc(size_t size, s32 alignment) override;
    void free(void* pPtr) override;
    void* resizeFront(void* pPtr, size_t size) override;
    void* resizeBack(void* pPtr, size_t size) override;
    void freeAll() override;
    uintptr_t getStartAddress() const override { return uintptr_t(mStart); }
    uintptr_t getEndAddress() const override { return uintptr_t(mStart) + getSize(); }
    size_t getSize() const override { return mSize; }
    size_t getFreeSize() const override;
    size_t getMaxAllocatableSize(int alignment) const override;
    bool isInclude(const void* pPtr) const override;
    bool isEmpty() const override { return mBlockList.size() == 0; }
    bool isFreeable() const override { return true; }
    bool isResizable() const override { return true; }
    bool isAdjustable() const override { return false; }
    void dump() const override;
    void dumpYAML(WriteStream& rStream, int indent) const override;
    void genInformation_(hostio::Context* pContext) override;

    void dumpBlockList() const;

    s32 getNodeNumMax() const { return mBlockList.size(); }
    s32 getUsedNodeNum() const { return mBlockList.getMaxNum(); }

protected:
    SeparateHeap(const SafeString& rName, Heap* pParent, void* pManagementArea,
                 size_t managementAreaSize, void* pHeapStart, size_t heapSize, bool enableLock);
    ~SeparateHeap() override;

    Block* findBlock_(void* pPtr);

    ObjList<Block> mBlockList;
};

}  // namespace sead
