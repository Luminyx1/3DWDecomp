#include <heap/seadExpHeap.h>
#include <heap/seadHeap.h>
#include <heap/seadHeapMgr.h>
#include <prim/seadScopedLock.h>
#include <thread/seadThread.h>
#include <time/seadTickSpan.h>
#include <utility>

namespace sead
{
HeapMgr* HeapMgr::sInstancePtr = nullptr;

HeapMgr HeapMgr::sInstance;
Arena HeapMgr::sDefaultArena;
HeapMgr::RootHeaps HeapMgr::sRootHeaps;
HeapMgr::IndependentHeaps HeapMgr::sIndependentHeaps;
CriticalSection HeapMgr::sHeapTreeLockCS;
Atomic<u32> HeapMgr::sHeapCheckTag;
TickSpan HeapMgr::sSleepSpanAtRemoveCacheFailure;

HeapMgr::HeapMgr() = default;
HeapMgr::~HeapMgr() = default;

void HeapMgr::initialize(size_t size)
{
    sHeapTreeLockCS.lock();
    sArena = &sDefaultArena;
    sDefaultArena.initialize(size);
    initializeImpl_();
    sHeapTreeLockCS.unlock();
}

void HeapMgr::initializeImpl_()
{
    sInstance.mAllocFailedCallback = nullptr;
    sSleepSpanAtRemoveCacheFailure = TickSpan::makeFromMicroSeconds(10);
    createRootHeap_();
    sInstancePtr = &sInstance;
}

void HeapMgr::initialize(Arena* pArena)
{
    sArena = pArena;
    initializeImpl_();
}

void HeapMgr::createRootHeap_()
{
    auto* expHeap = ExpHeap::tryCreate(sArena->mStart, sArena->mSize, "RootHeap", false);
    sRootHeaps.pushBack(expHeap);
}

void HeapMgr::destroy()
{
    sHeapTreeLockCS.lock();
    sInstance.mAllocFailedCallback = nullptr;

    while (!sIndependentHeaps.isEmpty())
    {
        sIndependentHeaps.back()->destroy();
        sIndependentHeaps.popBack();
    }

    while (!sRootHeaps.isEmpty())
    {
        sRootHeaps.back()->destroy();
        sRootHeaps.popBack();
    }

    sInstancePtr = nullptr;
    sArena->destroy();
    sArena = nullptr;
    sHeapTreeLockCS.unlock();
}

void HeapMgr::initHostIO() {}

bool HeapMgr::isContainedInAnyHeap(const void* ptr)
{
    for (auto& heap : sRootHeaps)
    {
        if (heap.isInclude(ptr))
        {
            return true;
        }
    }
    for (auto& heap : sIndependentHeaps)
    {
        if (heap.isInclude(ptr))
        {
            return true;
        }
    }
    return false;
}
void HeapMgr::dumpTreeYAML(WriteStream& rStream)
{
    sHeapTreeLockCS.lock();
    for (auto& heap : sRootHeaps)
    {
        heap.dumpTreeYAML(rStream, 0);
    }
    for (auto& heap : sIndependentHeaps)
    {
        heap.dumpTreeYAML(rStream, 0);
    }
    sHeapTreeLockCS.unlock();
}

void HeapMgr::setAllocFromNotSeadThreadHeap(Heap* pHeap)
{
    mAllocFromNotSeadThreadHeap = pHeap;
}

void HeapMgr::removeFromFindContainHeapCache_(Heap* pHeap)
{
    auto* threadMgr = ThreadMgr::instance();
    if (!threadMgr)
    {
        return;
    }

    Thread* mainThread = threadMgr->getMainThread();
    if (mainThread)
    {
        while (!mainThread->getFindContainHeapCache()->tryRemoveHeap(pHeap))
        {
            Thread::sleep(sSleepSpanAtRemoveCacheFailure);
        }
    }

    while (threadMgr->tryRemoveFromFindContainHeapCache(pHeap))
    {
        Thread::sleep(sSleepSpanAtRemoveCacheFailure);
    }
}

Heap* HeapMgr::findHeapByName(const sead::SafeString& rName, int index) const
{
    auto lock = makeScopedLock(sHeapTreeLockCS);
    for (auto& heap : sRootHeaps)
    {
        Heap* found = findHeapByName_(&heap, rName, &index);
        if (found)
        {
            return found;
        }
    }
    for (auto& heap : sIndependentHeaps)
    {
        Heap* found = findHeapByName_(&heap, rName, &index);
        if (found)
        {
            return found;
        }
    }
    return nullptr;
}

Heap* HeapMgr::findHeapByName_(Heap* pHeap, const SafeString& rName, int* pIndex)
{
    if (pHeap->getName() == rName)
    {
        if (*pIndex == 0)
        {
            return pHeap;
        }
        --*pIndex;
    }
    for (auto& child : pHeap->mChildren)
    {
        Heap* found = findHeapByName_(&child, rName, pIndex);
        if (found)
        {
            return found;
        }
    }
    return nullptr;
}

Heap* HeapMgr::getCurrentHeap() const
{
    Thread* currentThread = ThreadMgr::instance()->getCurrentThread();
    if (currentThread)
    {
        return currentThread->getCurrentHeap();
    }
    return mAllocFromNotSeadThreadHeap;
}

Heap* HeapMgr::setCurrentHeap_(Heap* pHeap)
{
    return ThreadMgr::instance()->getCurrentThread()->setCurrentHeap(pHeap);
}

void HeapMgr::removeRootHeap(Heap* pHeap)
{
    if (sRootHeaps.size() < 1)
    {
        return;
    }
    s32 index = sRootHeaps.indexOf(pHeap);
    if (index != -1)
    {
        sRootHeaps.erase(index);
    }
}

HeapMgr::IAllocFailedCallback*
HeapMgr::setAllocFailedCallback(HeapMgr::IAllocFailedCallback* pCallback)
{
    return std::exchange(mAllocFailedCallback, pCallback);
}

FindContainHeapCache::FindContainHeapCache() = default;

bool FindContainHeapCache::tryRemoveHeap(Heap* pHeap)
{
    uintptr_t original;
    if (mHeap.compareExchange(uintptr_t(pHeap), 0, &original))
    {
        return true;
    }
#if SEAD_FINDCONTAINHEAPCACHE_FIXED
    return (original & ~1ul) != uintptr_t(pHeap);
#else
    return (original & ~1u) != uintptr_t(pHeap);
#endif
}
}  // namespace sead
