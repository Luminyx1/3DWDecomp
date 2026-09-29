#pragma once

#include <basis/seadTypes.h>
#include "detail/aglGPUMemBlockMgr.h"

namespace sead {
class SeparateHeap;
}

namespace agl {

class GPUMemBlockBase;

namespace detail {

class MemoryPoolHeap {
public:
    MemoryPoolHeap(void* pBuffer, u64 bufferSize, u64 poolSize, const MemoryPoolType& rType,
                   void* pManagementArea, u64 managementAreaSize, GPUMemBlockMgrHeapEx* pHeapEx);
    ~MemoryPoolHeap();

    static MemoryPoolHeap* create(u64 size, s32 alignment, u64 userSize, s32 userAlignment,
                                  u64 minBlockSize, u64 maxNodeNum, const MemoryPoolType& rType,
                                  GPUMemBlockMgrHeapEx* pHeapEx);
    static void destroy(MemoryPoolHeap* pPoolHeap);

    void pushBack(GPUMemBlockBase* pBlock);
    void* allocFromMemoryPool(u64 size, s32 alignment);
    void freeToHeap(GPUMemBlockBase* pBlock);
    bool isAllocatable(const MemoryPoolType& rType, u64 size, s32 alignment) const;

    sead::SeparateHeap* getHeap() const { return mHeap; }
    MemoryPoolHeap* getNext() const { return mNext; }
    void* getBuffer() const { return mBuffer; }
    MemoryPool* getMemoryPool() { return &mMemoryPool; }

private:
    friend class GPUMemBlockMgrHeapEx;

    GPUMemBlockMgrHeapEx* mHeapEx;
    sead::SeparateHeap* mHeap;
    void* mBuffer;
    MemoryPool mMemoryPool;
    GPUMemBlockBase* mBlockList;
    MemoryPoolHeap* mNext;
};
static_assert(sizeof(MemoryPoolHeap) == 0x130);

}  // namespace detail
}  // namespace agl
