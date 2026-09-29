#pragma once

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

    u32 verify_() const;
    void deleteGPUMemBlock() const;
    void invalidate();
    u32 getAlignmentAddress() const;
    void setByteOffsetByPtr(void* ptr);
    void roundUp(int addr);
    void flushCPUCache(u64);
    void invalidateCPUCache(u64) const;

    bool isValid() const { return mMemoryPool != nullptr; }

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
    GPUMemAddr(const GPUMemBlockBase& rBlock, u64 offset) : GPUMemAddrBase(rBlock, offset) {}
};
}  // namespace agl
