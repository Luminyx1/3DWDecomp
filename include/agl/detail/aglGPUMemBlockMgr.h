#pragma once

#include <nvn/nvn.h>
#include <container/seadPtrArray.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadBitFlag.h>
#include <prim/seadTypedBitFlag.h>
#include <thread/seadCriticalSection.h>
#include "common/aglGPUCommon.hpp"

namespace sead::hostio {
class Context;
class PropertyEvent;
}  // namespace sead::hostio

namespace agl {
class GPUMemBlockBase;
}

namespace agl::detail {

class MemoryPoolHeap;

using MemoryPoolDriverBitFlag = sead::BitFlag32;

constexpr s32 VALID_POOL_TYPE_VALUE = -1;
constexpr s32 cGPUAccessMask = 0xF0000000;
constexpr u64 cGPUPhysicalMemorySizeAlignment = 0x1000;

class MemoryPoolType : public MemoryPoolDriverBitFlag {
public:
    MemoryPoolType() : MemoryPoolDriverBitFlag() {}
    MemoryPoolType(u32 value) : MemoryPoolDriverBitFlag(value) {}

    static MemoryPoolType convert(MemoryAttribute attribute);

    bool IsValid() const { return (*this & cValidPoolType) == cValidPoolType; }

    void MarkValid() { *this = *this | cValidPoolType; }

private:
    static const MemoryPoolType cInvalidPoolType;
    static const MemoryPoolType cValidPoolType;
};

class MemoryPool {
public:
    MemoryPool() { mMemoryType.setDirect(0); }

    void initialize(void* pStorage, u64 size, const MemoryPoolType& rType);
    void initialize(void* pStorage, u64 size, const MemoryPoolType& rType,
                    const MemoryPool& rPhysicalPool, s32 storageClass);

    void finalize();

    NVNmemoryPool* getDriverPool() { return &mDriverPool; }
    const NVNmemoryPool* getDriverPool() const { return &mDriverPool; }
    const MemoryPoolType& getMemoryType() const { return mMemoryType; }

private:
    NVNmemoryPool mDriverPool;
    MemoryPoolType mMemoryType;
    u32 _104;
};
static_assert(sizeof(MemoryPool) == 0x108);

class GPUMemBlockMgrHeapEx : public sead::hostio::Node, public sead::IDisposer {
public:
    GPUMemBlockMgrHeapEx(sead::Heap* pHeap);
    ~GPUMemBlockMgrHeapEx() override;

    bool tryAlloc(GPUMemBlockBase* pBlock, u64 size, s32 alignment, u64 userSize,
                  s32 userAlignment, u64 minBlockSize, u64 maxNodeNum, const MemoryPoolType& rType,
                  bool allowSharing, bool debug);
    void freeMemoryPoolHeap(MemoryPoolHeap* pPoolHeap);
    s32 countMemoryPoolNum() const;
    u64 countMemoryPoolSize() const;

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    sead::Heap* getHeap() const { return getDisposerHeap_(); }
    sead::CriticalSection* getCriticalSection() { return &mCS; }
    void setAllowSharing(bool allow) { mAllowSharing.change(1, allow); }

private:
    friend class GPUMemBlockMgr;

    sead::BitFlag32 mAllowSharing;
    MemoryPoolHeap* mHead;
    MemoryPoolHeap* mTail;
    sead::CriticalSection mCS;
};
static_assert(sizeof(GPUMemBlockMgrHeapEx) == 0x80);

enum class GPUMemBlockMgrFlags : u8 {
    MemoryPoolRelated = 1 << 0,
    EnablePoolSharing = 1 << 1,
    Debug = 1 << 2
};

class GPUMemBlockMgr : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(GPUMemBlockMgr)
public:
    GPUMemBlockMgr();
    virtual ~GPUMemBlockMgr();

    void initialize(sead::Heap* pHeap, sead::Heap* pDebugHeap);
    bool tryAllocMemory(GPUMemBlockBase* pBlock, sead::Heap* pHeap, u64 size, s32 alignment,
                        MemoryAttribute attribute);
    void enableSharedMemoryPool(bool enabled);
    void enableSharedMemoryPool(sead::Heap* pHeap, bool enabled);
    bool removeGPUMemBlockMgrHeapExIfNoMemoryPool(sead::Heap* pHeap);
    void removeHeap(GPUMemBlockMgrHeapEx* pHeapEx);
    static u64 calcGPUMemorySize(u64 userSize);
    static s32 calcGPUMemoryAlignment(s32 userAlignment);

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    GPUMemBlockMgrHeapEx* findGPUMemBlockMgrHeapEx_(sead::Heap* pHeap, s32* pOutIndex);

    sead::CriticalSection mCS;
    sead::PtrArray<GPUMemBlockMgrHeapEx> mMngrHeaps;
    size_t mMinBlockSize;
    sead::TypedBitFlag<GPUMemBlockMgrFlags> mFlags;
};
static_assert(sizeof(GPUMemBlockMgr) == 0x88);

}  // namespace agl::detail
