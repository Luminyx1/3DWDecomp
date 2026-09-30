#include <heap/seadDisposer.h>
#include <heap/seadHeap.h>
#include <heap/seadHeapMgr.h>

namespace
{
const u32 cDestructedFlag = 1;

}  // namespace

namespace sead
{
/**
 * Registers the object with the heap that contains it, so it is destroyed with that heap.
 */
IDisposer::IDisposer() : IDisposer(nullptr, HeapNullOption::UseSpecifiedOrContainHeap) {}

/**
 * Registers the object with a heap, so it is destroyed with that heap.
 * @param pDisposerHeap heap to register with, or nullptr to pick one by heapNullOption
 * @param heapNullOption which heap to use when pDisposerHeap is nullptr
 */
IDisposer::IDisposer(Heap* const pDisposerHeap, HeapNullOption heapNullOption)
{
    mDisposerHeap = pDisposerHeap;
    if (mDisposerHeap)
    {
        mDisposerHeap->appendDisposer_(this);
        return;
    }

    switch (heapNullOption)
    {
    case HeapNullOption::AlwaysUseSpecifiedHeap:
        SEAD_ASSERT_MSG(false, "disposerHeap must not be nullptr");
    case HeapNullOption::UseSpecifiedOrContainHeap:
        if (!sead::HeapMgr::sInstancePtr)
        {
            return;
        }

        mDisposerHeap = sead::HeapMgr::sInstancePtr->findContainHeap(this);
        if (mDisposerHeap)
        {
            mDisposerHeap->appendDisposer_(this);
        }

        return;
    case HeapNullOption::DoNotAppendDisposerIfNoHeapSpecified:
        return;
    case HeapNullOption::UseSpecifiedOrCurrentHeap:
        if (!sead::HeapMgr::sInstancePtr)
        {
            return;
        }

        mDisposerHeap = sead::HeapMgr::sInstancePtr->getCurrentHeap();
        if (mDisposerHeap)
        {
            mDisposerHeap->appendDisposer_(this);
        }

        return;
    default:
        SEAD_ASSERT_MSG(false, "illegal option[%d]", int(heapNullOption));
        return;
    }
}

/**
 * Unregisters the object from its heap and marks it as destroyed.
 */
IDisposer::~IDisposer()
{
    if (reinterpret_cast<uintptr_t>(mDisposerHeap) != cDestructedFlag)
    {
        if (mDisposerHeap != nullptr)
        {
            mDisposerHeap->removeDisposer_(this);
        }

        *reinterpret_cast<uintptr_t*>(&mDisposerHeap) = cDestructedFlag;
    }
}

}  // namespace sead
