#pragma once

#include <nvn/nvn_FuncPtrInline.h>

#include "common/aglGPUMemBlock.h"
#include "detail/aglMemoryPoolHeap.h"

namespace sead {
class Heap;
}

namespace agl {
class GPUMemAddrBase {
public:
    GPUMemAddrBase() {}
    GPUMemAddrBase(const GPUMemAddrBase& rOther)
        : mMemoryPool(rOther.mMemoryPool), mAlignmentAddr(rOther.mAlignmentAddr),
          mMemoryBlock(rOther.mMemoryBlock) {}
    GPUMemAddrBase(const GPUMemAddrBase& other, int alignmentOffset)
        : mMemoryPool(other.mMemoryPool), mAlignmentAddr(other.mAlignmentAddr + alignmentOffset),
          mMemoryBlock(other.mMemoryBlock) {}
    GPUMemAddrBase(const GPUMemBlockBase& memBlock, u64 offset);
    GPUMemAddrBase(detail::MemoryPool* pMemoryPool, s32 offset)
        : mMemoryPool(pMemoryPool), mAlignmentAddr(offset) {}

    u32 verify_() const;
    void deleteGPUMemBlock() const;
    void invalidate();
    u32 getAlignmentAddress() const;
    void setByteOffsetByPtr(void* ptr);
    void roundUp(int addr);
    void flushCPUCache(u64) const;
    void invalidateCPUCache(u64) const;

    bool isValid() const { return mMemoryPool != nullptr; }

    void* getMappedBase() const
    {
        return mMemoryPool ? nvnMemoryPoolMap(mMemoryPool->getDriverPool()) : nullptr;
    }
    void* getPtr() const
    {
        return static_cast<u8*>(getMappedBase()) + static_cast<u32>(mAlignmentAddr);
    }
    detail::MemoryPool* getMemoryPool() const { return mMemoryPool; }
    u32 getByteOffset() const { return mAlignmentAddr; }
    GPUMemBlockBase* getMemoryBlock() const { return mMemoryBlock; }

private:
    detail::MemoryPool* mMemoryPool = nullptr;
    int mAlignmentAddr = 0;
    GPUMemBlockBase* mMemoryBlock = nullptr;
};

template <typename T>
class GPUMemAddr : public GPUMemAddrBase {
public:
    GPUMemAddr() = default;
    GPUMemAddr(const GPUMemAddrBase& rAddr) : GPUMemAddrBase(rAddr) {}
    GPUMemAddr(const GPUMemAddrBase& rAddr, int offset) : GPUMemAddrBase(rAddr, offset) {}
    GPUMemAddr(const GPUMemBlockBase& rBlock, u64 offset) : GPUMemAddrBase(rBlock, offset) {}
};

class GPUMemVoidAddr : public GPUMemAddrBase {
public:
    GPUMemVoidAddr() = default;
    GPUMemVoidAddr(const GPUMemAddrBase& rAddr) : GPUMemAddrBase(rAddr) {}
    GPUMemVoidAddr(const GPUMemAddrBase& rAddr, int offset) : GPUMemAddrBase(rAddr, offset) {}
    GPUMemVoidAddr(const GPUMemBlockBase& rBlock, u64 offset) : GPUMemAddrBase(rBlock, offset) {}
    GPUMemVoidAddr(detail::MemoryPool* pMemoryPool, s32 offset)
        : GPUMemAddrBase(pMemoryPool, offset) {}
};

class ConstGPUMemVoidAddr : public GPUMemAddrBase {
public:
    ConstGPUMemVoidAddr() = default;
    ConstGPUMemVoidAddr(const GPUMemAddrBase& rAddr) : GPUMemAddrBase(rAddr) {}
    ConstGPUMemVoidAddr(const GPUMemAddrBase& rAddr, int offset) : GPUMemAddrBase(rAddr, offset)
    {
    }
    ConstGPUMemVoidAddr(const GPUMemBlockBase& rBlock, u64 offset)
        : GPUMemAddrBase(rBlock, offset)
    {
    }
};
}  // namespace agl
