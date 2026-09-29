#pragma once

#include <container/seadBuffer.h>
#include <container/seadOffsetList.h>
#include <prim/seadSafeString.h>
#include "common/aglGPUMemAddr.h"
#include "common/aglGPUMemBlock.h"

namespace sead {
class Heap;
namespace hostio {
class Context;
}
}  // namespace sead

namespace agl::utl {

class TextureMemoryAllocator {
public:
    struct AllocateArg {
        s32 mStorageClass;
        u32 mSize;
        u32 mImageOffset;
        u32 mMipSize;
        s32 mAlignment;
        u32 mExtraSize;
        s32 mExtraAlignment;
    };

    struct MemoryBlock {
        ~MemoryBlock() { ; }

        u32 getEndOffset() const { return static_cast<u32>(mSize) + mAddr.getByteOffset(); }

        GPUMemVoidAddr mAddr;
        u64 mSize;
        GPUMemVoidAddr mImageAddr;
        GPUMemVoidAddr mMipAddr;
        GPUMemAddr<u8> mMemBlockAddr;
        sead::ListNode mListNode;
        s32 mStorageClass;
    };
    static_assert(sizeof(MemoryBlock) == 0x80);

    struct Storage {
        GPUMemBlock<u8> mBlock;
        s32 mStorageClass;
    };
    static_assert(sizeof(Storage) == 0x40);

    TextureMemoryAllocator();
    virtual ~TextureMemoryAllocator();

    void initialize(GPUMemVoidAddr addr, sead::Buffer<Storage>* pStorage, u64 size, s32 blockNum,
                    sead::Heap* pHeap);
    MemoryBlock* alloc(const AllocateArg& rArg, GPUMemVoidAddr* pAddr, bool fromFront);
    u64 getMaxAllocatableSize() const;
    void free(MemoryBlock* pBlock, bool deleteMemBlock);
    void dump(const sead::SafeString& rTitle, const sead::SafeString& rIndent,
              const MemoryBlock& rBlock) const;
    void dumpDetail() const;
    void genMessageInfo(const sead::SafeString& rName, sead::hostio::Context* pContext) const;
    bool isOverwrapperd(const TextureMemoryAllocator& rOther) const;

    static void setupStorage(sead::Buffer<Storage>* pStorage, GPUMemVoidAddr addr, u64 size,
                             sead::Heap* pHeap);

    void setHeap(sead::Heap* pHeap) { mHeap = pHeap; }
    void setAlign64K(bool enable) { mFlags = enable ? (mFlags | 1) : (mFlags & ~1); }
    u64 getUsedSize() const { return mUsedSize; }

private:
    bool alloc_(MemoryBlock* pBlock, const AllocateArg& rArg, GPUMemVoidAddr* pAddr,
                bool fromFront);

    GPUMemVoidAddr mAddr;
    u64 mEndOffset;
    u64 mUsedSize = 0;
    sead::Buffer<MemoryBlock> mBlocks;
    sead::OffsetList<MemoryBlock> mUsedList;
    sead::OffsetList<MemoryBlock> mFreeList;
    sead::OffsetList<MemoryBlock> mUnusedList;
    sead::Heap* mHeap = nullptr;
    sead::Buffer<Storage>* mStorage = nullptr;
    u8 mFlags = 0;
};
static_assert(sizeof(TextureMemoryAllocator) == 0xa0);

}  // namespace agl::utl
