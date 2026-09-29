#include "thread/seadCriticalSection.h"

namespace sead
{
/**
 * Creates a recursive lock owned by the current heap.
 */
CriticalSection::CriticalSection() : IDisposer()
{
    nn::os::InitializeMutex(&mCriticalSectionInner, true, 0);
}

/**
 * Creates a recursive lock.
 * @param pDisposerHeap heap that destroys the lock with itself, or nullptr for the heap containing it
 */
CriticalSection::CriticalSection(Heap* pDisposerHeap)
    : IDisposer(pDisposerHeap, HeapNullOption::UseSpecifiedOrContainHeap)
{
    nn::os::InitializeMutex(&mCriticalSectionInner, true, 0);
}

/**
 * Creates a recursive lock.
 * @param pDisposerHeap heap that destroys the lock with itself
 * @param heapNullOption what to do when pDisposerHeap is nullptr
 */
CriticalSection::CriticalSection(Heap* pDisposerHeap, HeapNullOption heapNullOption)
    : IDisposer(pDisposerHeap, heapNullOption)
{
    nn::os::InitializeMutex(&mCriticalSectionInner, true, 0);
}

/**
 * Finalizes the lock.
 */
CriticalSection::~CriticalSection()
{
    nn::os::FinalizeMutex(&mCriticalSectionInner);
}

/**
 * Enters the critical section, waiting for other threads to leave it.
 */
void CriticalSection::lock()
{
    nn::os::LockMutex(&mCriticalSectionInner);
}

/**
 * Enters the critical section if no other thread is in it.
 * @return whether it was entered
 */
bool CriticalSection::tryLock()
{
    return nn::os::TryLockMutex(&mCriticalSectionInner);
}

/**
 * Leaves the critical section.
 */
void CriticalSection::unlock()
{
    nn::os::UnlockMutex(&mCriticalSectionInner);
}
}  // namespace sead
