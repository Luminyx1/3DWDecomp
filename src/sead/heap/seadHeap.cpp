#include <heap/seadHeap.h>
#include <heap/seadHeapMgr.h>
#include <prim/seadScopedLock.h>

namespace sead
{
Heap::~Heap() = default;

void Heap::appendDisposer_(IDisposer* pDisposer)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    mDisposerList.pushBack(pDisposer);
}

void Heap::removeDisposer_(IDisposer* pDisposer)
{
    ConditionalScopedLock<CriticalSection> lock(&mCS, isLockEnabled());
    mDisposerList.erase(pDisposer);
}

Heap* Heap::findContainHeap_(const void* ptr)
{
    if (!isInclude(ptr))
    {
        return nullptr;
    }

    for (auto it = mChildren.begin(); it != mChildren.end(); ++it)
    {
        if (it->isInclude(ptr))
        {
            return it->findContainHeap_(ptr);
        }
    }

    return this;
}

}  // namespace sead
