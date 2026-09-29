#include "detail/aglGPUMemBlockMgr.h"

#include <gfx/nin/seadGraphicsNvn.h>
#include <heap/seadSeparateHeap.h>
#include <nn/os.h>
#include <nvn/nvn_FuncPtrInline.h>
#include <prim/seadSafeString.h>
#include <prim/seadScopedLock.h>

#include "common/aglGPUMemBlock.h"
#include "detail/aglMemoryPoolHeap.h"
#include "driver/aglNVNMgr.h"

namespace agl::detail
{
namespace
{

struct MappingRequest
{
    const NVNmemoryPool* mPhysicalPool;
    s64 mPhysicalOffset;
    s64 mVirtualOffset;
    u64 mSize;
    s32 mStorageClass;
};

constexpr uintptr_t cSeparateHeapStart = 0x100000;

template <typename T, typename Compare>
void cocktailSort(sead::PtrArray<T>& rArray, Compare compare)
{
    const s32 num = rArray.size();
    if (num < 2)
    {
        return;
    }

    T** pArray = rArray.data();
    s32 lo = 0;
    s32 hi = num - 1;
    do
    {
        s32 last = lo;
        for (s32 i = lo; i < hi; ++i)
        {
            if (compare(pArray[i], pArray[i + 1]) > 0)
            {
                T* tmp = pArray[i + 1];
                pArray[i + 1] = pArray[i];
                pArray[i] = tmp;
                last = i;
            }
        }
        hi = last;
        if (hi <= lo)
        {
            break;
        }

        last = hi;
        for (s32 i = hi; i > lo; --i)
        {
            if (compare(pArray[i], pArray[i - 1]) < 0)
            {
                T* tmp = pArray[i - 1];
                pArray[i - 1] = pArray[i];
                pArray[i] = tmp;
                last = i;
            }
        }
        lo = last;
    } while (lo != hi);
}

}  // namespace

const MemoryPoolType MemoryPoolType::cInvalidPoolType(0);
const MemoryPoolType MemoryPoolType::cValidPoolType(VALID_POOL_TYPE_VALUE);

MemoryPoolType MemoryPoolType::convert(MemoryAttribute attribute)
{
    const u32 attr = static_cast<u32>(attribute);
    u32 flags = (attr & 0x111) ? NVN_MEMORY_POOL_FLAGS_CPU_CACHED |
                                     NVN_MEMORY_POOL_FLAGS_GPU_UNCACHED :
                                 NVN_MEMORY_POOL_FLAGS_CPU_CACHED |
                                     NVN_MEMORY_POOL_FLAGS_GPU_CACHED;
    if (attr & 0x73)
    {
        flags = (flags & 0x30) | NVN_MEMORY_POOL_FLAGS_CPU_UNCACHED;
    }
    MemoryPoolType type(flags);

    if (attr & static_cast<u32>(MemoryAttribute::CompressibleMemory))
    {
        driver::NVNMgr::instance()->setMemoryPoolSettingTexture(&type);
    }
    if (attr & static_cast<u32>(MemoryAttribute::CpuCached))
    {
        type.setDirect((type.getDirect() & ~7u) | NVN_MEMORY_POOL_FLAGS_CPU_CACHED);
    }
    if (attr & static_cast<u32>(MemoryAttribute::MemoryReserved))
    {
        type.setDirect(NVN_MEMORY_POOL_FLAGS_CPU_NO_ACCESS | NVN_MEMORY_POOL_FLAGS_GPU_NO_ACCESS |
                       NVN_MEMORY_POOL_FLAGS_PHYSICAL);
    }
    if (attr & static_cast<u32>(MemoryAttribute::CompressibleMemory))
    {
        type.setDirect(type.getDirect() | NVN_MEMORY_POOL_FLAGS_COMPRESSIBLE);
    }
    return type;
}

void MemoryPool::initialize(void* pStorage, u64 size, const MemoryPoolType& rType)
{
    NVNmemoryPoolBuilder builder;
    const u32 flags = rType.getDirect() & 0xfffffff;
    nvnMemoryPoolBuilderSetDefaults(&builder);
    nvnMemoryPoolBuilderSetDevice(&builder, sead::GraphicsNvn::instance()->getNvnDevice());
    nvnMemoryPoolBuilderSetFlags(&builder, flags);
    nvnMemoryPoolBuilderSetStorage(&builder, pStorage, size);
    driver::NVNMgr::instance()->nvnMemoryPoolInitialize(&mDriverPool, &builder);
    mMemoryType.setDirect(rType.getDirect() | 0x80000000);
}

void MemoryPool::initialize(void* pStorage, u64 size, const MemoryPoolType& rType,
                            const MemoryPool& rPhysicalPool, s32 storageClass)
{
    NVNmemoryPoolBuilder builder;
    const u32 type = rType.getDirect();
    const u32 flags = (type & 0xffffff8) | NVN_MEMORY_POOL_FLAGS_CPU_NO_ACCESS |
                      NVN_MEMORY_POOL_FLAGS_VIRTUAL;
    const u32 access = type & 0x70000000;
    nvnMemoryPoolBuilderSetDefaults(&builder);
    nvnMemoryPoolBuilderSetDevice(&builder, sead::GraphicsNvn::instance()->getNvnDevice());
    nvnMemoryPoolBuilderSetFlags(&builder, flags);
    nvnMemoryPoolBuilderSetStorage(&builder, nullptr, size);
    driver::NVNMgr::instance()->nvnMemoryPoolInitialize(&mDriverPool, &builder);

    MappingRequest request;
    request.mPhysicalPool = rPhysicalPool.getDriverPool();
    request.mPhysicalOffset = 0;
    request.mVirtualOffset = 0;
    request.mSize = size;
    request.mStorageClass = storageClass;
    nn::os::GetSystemTick();
    nn::os::GetSystemTick();
    nvnMemoryPoolMapVirtual(&mDriverPool, 1, reinterpret_cast<const NVNmappingRequest*>(&request));
    mMemoryType.setDirect(access | flags | 0x80000000);
}

void MemoryPool::finalize()
{
    if (mMemoryType.getDirect() & 0x80000000)
    {
        nvnMemoryPoolFinalize(&mDriverPool);
        mMemoryType.setDirect(mMemoryType.getDirect() & 0x7fffffff);
    }
}

MemoryPoolHeap* MemoryPoolHeap::create(u64 size, s32 alignment, u64 userSize, s32 userAlignment,
                                       u64 minBlockSize, u64 maxNodeNum,
                                       const MemoryPoolType& rType, GPUMemBlockMgrHeapEx* pHeapEx)
{
    const u64 poolSize = size > minBlockSize ? size : minBlockSize;
    const u64 freeSize = poolSize - userSize;
    const u64 blockNum = poolSize >> 9;
    const u64 nodeNum =
        freeSize < 0x200 ? 1 : (blockNum < maxNodeNum ? blockNum : maxNodeNum);
    const u64 managementAreaSize = sead::SeparateHeap::getManagementAreaSize(nodeNum);

    void* storage =
        pHeapEx->getHeap()->tryAlloc(sizeof(MemoryPoolHeap) + managementAreaSize, 8);
    if (!storage)
    {
        return nullptr;
    }

    void* buffer = pHeapEx->getHeap()->tryAlloc(poolSize, alignment);
    if (!buffer)
    {
        return nullptr;
    }

    return new (storage) MemoryPoolHeap(buffer, size, poolSize, rType,
                                        static_cast<u8*>(storage) + sizeof(MemoryPoolHeap),
                                        managementAreaSize, pHeapEx);
}

void MemoryPoolHeap::destroy(MemoryPoolHeap* pPoolHeap)
{
    pPoolHeap->~MemoryPoolHeap();
}

MemoryPoolHeap::MemoryPoolHeap(void* pBuffer, u64 bufferSize, u64 poolSize,
                               const MemoryPoolType& rType, void* pManagementArea,
                               u64 managementAreaSize, GPUMemBlockMgrHeapEx* pHeapEx)
    : mHeapEx(pHeapEx), mHeap(nullptr), mBuffer(pBuffer), mBlockList(nullptr), mNext(nullptr)
{
    mHeap = sead::SeparateHeap::create("gpu", pManagementArea, managementAreaSize,
                                       reinterpret_cast<void*>(cSeparateHeapStart), poolSize,
                                       false);
    mMemoryPool.initialize(mBuffer, poolSize, rType);
}

MemoryPoolHeap::~MemoryPoolHeap()
{
    for (GPUMemBlockBase* block = mBlockList; block;)
    {
        GPUMemBlockBase* next = block->getNext();
        block->clear();
        block = next;
    }
    mBlockList = nullptr;

    mMemoryPool.finalize();

    if (mHeap)
    {
        mHeap->destroy();
        mHeap = nullptr;
    }

    if (mBuffer)
    {
        mHeapEx->getHeap()->free(mBuffer);
        mBuffer = nullptr;
    }
}

void MemoryPoolHeap::pushBack(GPUMemBlockBase* pBlock)
{
    if (!mBlockList)
    {
        mBlockList = pBlock;
        return;
    }
    mBlockList->addList(pBlock);
}

void* MemoryPoolHeap::allocFromMemoryPool(u64 size, s32 alignment)
{
    void* ptr = mHeap->tryAlloc(size, alignment);
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(ptr) +
                                   reinterpret_cast<uintptr_t>(mBuffer) - cSeparateHeapStart);
}

void MemoryPoolHeap::freeToHeap(GPUMemBlockBase* pBlock)
{
    sead::CriticalSection* cs = nullptr;
    if (mHeapEx->getHeap()->isLockEnabled())
    {
        cs = mHeapEx->getCriticalSection();
        cs->lock();
    }

    GPUMemBlockBase* prev = nullptr;
    for (GPUMemBlockBase* cur = mBlockList; cur; prev = cur, cur = cur->getNext())
    {
        if (cur == pBlock)
        {
            if (prev)
            {
                prev->setNext(pBlock->getNext());
            }
            else
            {
                mBlockList = mBlockList->getNext();
            }
            break;
        }
    }

    const uintptr_t offset = pBlock->getByteOffset() + cSeparateHeapStart;
    mHeap->free(reinterpret_cast<void*>(offset));
    if (mHeap->isEmpty())
    {
        mHeapEx->freeMemoryPoolHeap(this);
    }

    if (cs)
    {
        cs->unlock();
    }
}

void GPUMemBlockMgrHeapEx::freeMemoryPoolHeap(MemoryPoolHeap* pPoolHeap)
{
    sead::CriticalSection* cs = nullptr;
    if (getHeap()->isLockEnabled())
    {
        cs = &mCS;
        cs->lock();
    }

    MemoryPoolHeap* prev = nullptr;
    MemoryPoolHeap* cur = nullptr;
    for (MemoryPoolHeap* it = mHead; it; prev = it, it = it->mNext)
    {
        if (it == pPoolHeap)
        {
            MemoryPoolHeap** link = prev == nullptr ? &mHead : &prev->mNext;
            *link = (prev == nullptr ? mHead : pPoolHeap)->mNext;

            if (mTail == pPoolHeap)
            {
                mTail = prev ? prev : mHead;
            }
            cur = pPoolHeap;
            break;
        }
    }

    if (cs)
    {
        cs->unlock();
    }

    MemoryPoolHeap::destroy(cur);
    getHeap()->free(cur);
}

bool MemoryPoolHeap::isAllocatable(const MemoryPoolType& rType, u64 size, s32 alignment) const
{
    if (mHeap->getUsedNodeNum() == mHeap->getNodeNumMax())
    {
        return false;
    }

    if ((mMemoryPool.getMemoryType().getDirect() ^ rType.getDirect()) & 0x7fffffff)
    {
        return false;
    }

    return static_cast<s64>(mHeap->getMaxAllocatableSize(alignment) - size) >= 0;
}

GPUMemBlockMgrHeapEx::GPUMemBlockMgrHeapEx(sead::Heap* pHeap)
    : IDisposer(pHeap, HeapNullOption::AlwaysUseSpecifiedHeap), mAllowSharing(1), mHead(nullptr),
      mTail(nullptr)
{
}

GPUMemBlockMgrHeapEx::~GPUMemBlockMgrHeapEx()
{
    {
        sead::ConditionalScopedLock<sead::CriticalSection> lock(&mCS, getHeap()->isLockEnabled());
        for (MemoryPoolHeap* pool = mHead; pool; pool = pool->mNext)
        {
            pool->~MemoryPoolHeap();
        }
    }

    GPUMemBlockMgr::instance()->removeHeap(this);
}

void GPUMemBlockMgr::removeHeap(GPUMemBlockMgrHeapEx* pHeapEx)
{
    mCS.lock();
    s32 index;
    if (findGPUMemBlockMgrHeapEx_(pHeapEx->getHeap(), &index))
    {
        mMngrHeaps.erase(index);
    }
    mCS.unlock();
}

bool GPUMemBlockMgrHeapEx::tryAlloc(GPUMemBlockBase* pBlock, u64 size, s32 alignment,
                                    u64 userSize, s32 userAlignment, u64 minBlockSize,
                                    u64 maxNodeNum, const MemoryPoolType& rType, bool allowSharing,
                                    bool debug)
{
    const s32 absAlignment = userAlignment < 0 ? -userAlignment : userAlignment;

    if (allowSharing && mAllowSharing.isOnBit(0))
    {
        sead::CriticalSection* cs = nullptr;
        if (getHeap()->isLockEnabled())
        {
            cs = &mCS;
            cs->lock();
        }

        MemoryPoolHeap* best = nullptr;
        f32 bestRatio = 0.0f;
        for (MemoryPoolHeap* pool = mHead; pool; pool = pool->mNext)
        {
            if (pool->isAllocatable(rType, userSize, absAlignment))
            {
                f32 ratio = static_cast<f32>(userSize) /
                            static_cast<f32>(pool->getHeap()->getFreeSize());
                if (bestRatio < ratio)
                {
                    bestRatio = ratio;
                    best = pool;
                }
            }
        }

        if (best)
        {
            void* ptr = best->allocFromMemoryPool(userSize, absAlignment);
            pBlock->setMemoryPoolHeap(ptr, userSize, best);
            if (cs)
            {
                cs->unlock();
            }
            return true;
        }

        if (cs)
        {
            cs->unlock();
        }
    }

    nn::os::GetSystemTick();
    nn::os::GetSystemTick();

    MemoryPoolHeap* pool = MemoryPoolHeap::create(size, alignment, userSize, userAlignment,
                                                  minBlockSize, maxNodeNum, rType, this);
    if (!pool)
    {
        return false;
    }

    sead::CriticalSection* cs = nullptr;
    if (getHeap()->isLockEnabled())
    {
        cs = &mCS;
        cs->lock();
    }

    void* ptr = pool->allocFromMemoryPool(userSize, absAlignment);
    pBlock->setMemoryPoolHeap(ptr, userSize, pool);

    if (mTail)
    {
        mTail->mNext = pool;
        mTail = pool;
    }
    else
    {
        mTail = pool;
        mHead = pool;
    }

    if (cs)
    {
        cs->unlock();
    }
    return true;
}

s32 GPUMemBlockMgrHeapEx::countMemoryPoolNum() const
{
    s32 num = 0;
    for (MemoryPoolHeap* pool = mHead; pool; pool = pool->mNext)
    {
        num++;
    }
    return num;
}

u64 GPUMemBlockMgrHeapEx::countMemoryPoolSize() const
{
    u64 size = 0;
    for (MemoryPoolHeap* pool = mHead; pool; pool = pool->mNext)
    {
        size += pool->getHeap()->getSize();
    }
    return size;
}

void GPUMemBlockMgrHeapEx::genMessage(sead::hostio::Context* pContext)
{
    const u64 heapFreeSize = getHeap()->getFreeSize();
    const u64 heapSize = getHeap()->getSize();

    u64 totalSize = 0;
    u64 freeSize = 0;
    u64 managementSize = 0;
    s64 usedNodeNum = 0;
    u64 managementMaxSize = 0;
    s32 nodeNum = 0;
    for (MemoryPoolHeap* pool = mHead; pool; pool = pool->mNext)
    {
        sead::SeparateHeap* heap = pool->getHeap();
        totalSize += heap->getSize();
        freeSize += heap->getFreeSize();
        nodeNum += heap->getNodeNumMax();
        usedNodeNum += heap->getUsedNodeNum();
        managementSize += sead::SeparateHeap::getManagementAreaSize(nodeNum);
        managementMaxSize += sead::SeparateHeap::getManagementAreaSize(usedNodeNum);
    }

    const u64 usedSize = totalSize - freeSize;
    {
        sead::FormatFixedSafeString<1024> msg(
            "sizeof( MemoryPoolHeap ) = %d[byte]\nMemoryPoolHeap x %d / %.2f(%%)up\nAllocate x "
            "%d\nHeap           :%12d/%12d[byte]\nSeparateHeap   :%12d/%12d[byte] "
            "(%.2f[%%])\nMemoryPoolHeap :%12d/%12d[byte] (%.2f[%%])\nTotal          "
            ":%12d/%12d[byte] (%.2f[%%])",
            static_cast<s32>(sizeof(MemoryPoolHeap)), countMemoryPoolNum(),
            static_cast<f32>(managementMaxSize) * 100.0f /
                static_cast<f32>(totalSize + managementMaxSize),
            nodeNum, heapFreeSize, heapSize, usedSize, totalSize,
            (1.0f - static_cast<f32>(freeSize) / static_cast<f32>(totalSize)) * 100.0f,
            managementSize, managementMaxSize,
            static_cast<f32>(managementSize) / static_cast<f32>(managementMaxSize) * 100.0f,
            usedSize + managementSize, totalSize + managementMaxSize,
            static_cast<f32>(usedSize + managementSize) /
                static_cast<f32>(totalSize + managementMaxSize) * 100.0f);
    }

    s32 index = 0;
    for (MemoryPoolHeap* pool = mHead; pool; pool = pool->mNext)
    {
        sead::SeparateHeap* heap = pool->getHeap();
        {
            sead::FormatFixedSafeString<1024> msg("%4d", index);
        }
        {
            sead::FormatFixedSafeString<1024> msg(
                "0x%08x", pool->mMemoryPool.getMemoryType().getDirect());
        }
        {
            sead::FormatFixedSafeString<1024> msg(
                "%12d/%12d", heap->getSize() - heap->getFreeSize(), heap->getSize());
        }
        {
            sead::FormatFixedSafeString<1024> msg("%4d/%4d", heap->getNodeNumMax(),
                                                  heap->getUsedNodeNum());
        }
        index++;
    }
}

void GPUMemBlockMgrHeapEx::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

SEAD_SINGLETON_DISPOSER_IMPL(GPUMemBlockMgr)

GPUMemBlockMgr::GPUMemBlockMgr()
{
    mMinBlockSize = cGPUPhysicalMemorySizeAlignment;
    mFlags = GPUMemBlockMgrFlags::EnablePoolSharing;
}

GPUMemBlockMgr::~GPUMemBlockMgr()
{
    mMngrHeaps.freeBuffer();
}

void GPUMemBlockMgr::initialize(sead::Heap* pHeap, sead::Heap* pDebugHeap)
{
    mMngrHeaps.allocBuffer(0x1000, pHeap);
    mMngrHeaps.clear();
}

bool GPUMemBlockMgr::tryAllocMemory(GPUMemBlockBase* pBlock, sead::Heap* pHeap, u64 size,
                                    s32 alignment, MemoryAttribute attribute)
{
    const u64 gpuSize = calcGPUMemorySize(size);
    const s32 gpuAlignment = calcGPUMemoryAlignment(alignment);
    MemoryPoolType type = MemoryPoolType::convert(attribute);
    MemoryPoolType poolType = type;

    if (mFlags.isOn(GPUMemBlockMgrFlags::MemoryPoolRelated))
    {
        u8* storage =
            static_cast<u8*>(pHeap->tryAlloc(gpuSize + sizeof(MemoryPool), gpuAlignment));
        if (!storage)
        {
            return false;
        }

        auto* pool = new (storage + gpuSize) MemoryPool();
        pool->initialize(storage, gpuSize, poolType);
        pBlock->setMemoryPool(storage, size, pool);
        return true;
    }

    mCS.lock();
    GPUMemBlockMgrHeapEx* heapEx = findGPUMemBlockMgrHeapEx_(pHeap, nullptr);
    if (!heapEx)
    {
        heapEx = new (pHeap, 8) GPUMemBlockMgrHeapEx(pHeap);
        mMngrHeaps.pushBack(heapEx);
        cocktailSort(mMngrHeaps,
                     [](const GPUMemBlockMgrHeapEx* a, const GPUMemBlockMgrHeapEx* b) -> s32 {
                         const intptr_t heapA = reinterpret_cast<intptr_t>(a->getHeap());
                         const intptr_t heapB = reinterpret_cast<intptr_t>(b->getHeap());
                         if (heapA < heapB)
                         {
                             return -1;
                         }
                         if (heapB < heapA)
                         {
                             return 1;
                         }
                         return 0;
                     });
    }
    mCS.unlock();

    return heapEx->tryAlloc(pBlock, gpuSize, gpuAlignment, size, alignment,
                            calcGPUMemorySize(mMinBlockSize), 0x80, poolType,
                            mFlags.isOn(GPUMemBlockMgrFlags::EnablePoolSharing),
                            mFlags.isOn(GPUMemBlockMgrFlags::Debug));
}

u64 GPUMemBlockMgr::calcGPUMemorySize(u64 userSize)
{
    return sead::MathSizeT::roundUp(userSize, cGPUPhysicalMemorySizeAlignment);
}

s32 GPUMemBlockMgr::calcGPUMemoryAlignment(s32 userAlignment)
{
    return sead::Mathi::roundUpPow2(sead::Mathi::abs(userAlignment),
                                    cGPUPhysicalMemorySizeAlignment) *
           sead::Mathi::sign(userAlignment);
}

GPUMemBlockMgrHeapEx* GPUMemBlockMgr::findGPUMemBlockMgrHeapEx_(sead::Heap* pHeap, s32* pOutIndex)
{
    if (mMngrHeaps.isEmpty())
    {
        return nullptr;
    }

    s32 lo = 0;
    s32 hi = mMngrHeaps.size() - 1;
    s32 mid;
    GPUMemBlockMgrHeapEx* result = nullptr;
    while (lo <= hi)
    {
        mid = (lo + hi) / 2;
        s64 diff = reinterpret_cast<intptr_t>(mMngrHeaps.unsafeAt(mid)->getHeap()) -
                   reinterpret_cast<intptr_t>(pHeap);
        if (diff == 0)
        {
            result = mMngrHeaps.at(mid);
            break;
        }
        if (diff < 0)
        {
            lo = mid + 1;
        }
        else
        {
            hi = mid - 1;
        }
    }

    if (pOutIndex)
    {
        *pOutIndex = mid;
    }
    return result;
}


void GPUMemBlockMgr::enableSharedMemoryPool(bool enabled)
{
    mFlags.change(GPUMemBlockMgrFlags::EnablePoolSharing, enabled);
}

bool GPUMemBlockMgr::removeGPUMemBlockMgrHeapExIfNoMemoryPool(sead::Heap* pHeap)
{
    sead::ScopedLock<sead::CriticalSection> lock(&mCS);
    bool removed;
    GPUMemBlockMgrHeapEx* heapEx = findGPUMemBlockMgrHeapEx_(pHeap, nullptr);
    if (heapEx)
    {
        if (heapEx->mHead != nullptr)
        {
            return false;
        }
        removeHeap(heapEx);
        sead::Heap* heap = heapEx->getHeap();
        heapEx->~GPUMemBlockMgrHeapEx();
        heap->free(heapEx);
        removed = true;
    }
    return removed;
}

void GPUMemBlockMgr::enableSharedMemoryPool(sead::Heap* pHeap, bool enabled)
{
    mCS.lock();
    GPUMemBlockMgrHeapEx* heapEx = findGPUMemBlockMgrHeapEx_(pHeap, nullptr);
    if (heapEx)
    {
        heapEx->setAllowSharing(enabled);
    }
    mCS.unlock();
}

void GPUMemBlockMgr::genMessage(sead::hostio::Context* pContext)
{
    {
        sead::FormatFixedSafeString<1024> msg("Effective smallest size: 0x%zx[byte]",
                                              calcGPUMemorySize(mMinBlockSize));
    }

    u64 size = 0;
    s32 num = 0;
    for (GPUMemBlockMgrHeapEx& heapEx : mMngrHeaps)
    {
        num += heapEx.countMemoryPoolNum();
        size += heapEx.countMemoryPoolSize();
    }

    {
        sead::FormatFixedSafeString<1024> msg("Total MemoryPool number: %d (%d[byte])", num, size);
    }
}

void GPUMemBlockMgr::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

}  // namespace agl::detail
