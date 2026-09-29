#include "thread/seadMutex.h"

namespace sead
{
/**
 * Creates a recursive mutex owned by the current heap.
 */
Mutex::Mutex() : IDisposer()
{
    nn::os::InitializeMutex(&mMutexInner, true, 0);
}

/**
 * Creates a recursive mutex.
 * @param pDisposerHeap heap that destroys the mutex with itself, or nullptr for the heap containing it
 */
Mutex::Mutex(Heap* pDisposerHeap) : Mutex(pDisposerHeap, HeapNullOption::UseSpecifiedOrContainHeap)
{
}

/**
 * Creates a recursive mutex.
 * @param pDisposerHeap heap that destroys the mutex with itself
 * @param heapNullOption what to do when pDisposerHeap is nullptr
 */
Mutex::Mutex(Heap* pDisposerHeap, HeapNullOption heapNullOption) : IDisposer(pDisposerHeap, heapNullOption)
{
    nn::os::InitializeMutex(&mMutexInner, true, 0);
}

/**
 * Finalizes the mutex.
 */
Mutex::~Mutex()
{
    nn::os::FinalizeMutex(&mMutexInner);
}

/**
 * Locks the mutex, waiting for other threads to release it.
 */
void Mutex::lock()
{
    nn::os::LockMutex(&mMutexInner);
}

/**
 * Locks the mutex if no other thread holds it.
 * @return whether the mutex was locked
 */
bool Mutex::tryLock()
{
    return nn::os::TryLockMutex(&mMutexInner);
}

/**
 * Releases the mutex.
 */
void Mutex::unlock()
{
    nn::os::UnlockMutex(&mMutexInner);
}
}  // namespace sead
