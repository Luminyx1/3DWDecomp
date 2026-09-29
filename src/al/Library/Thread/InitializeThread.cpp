#include "Library/Thread/InitializeThread.hpp"

#include <heap/seadHeapMgr.h>
#include <prim/seadDelegate.h>
#include <thread/seadDelegateThread.h>

#include "Library/Thread/Functor.hpp"

namespace al {
/**
 * @brief Creates a thread that runs a functor once, with a given heap as its current heap.
 * @param rName The name of the thread.
 * @param rFunctor The functor to run; a copy of it is stored.
 * @param pHeap The heap set as current while the functor runs, or nullptr to keep the current one.
 * @param priority The priority of the thread.
 * @param stackSize The base stack size of the thread; three times this amount is reserved.
 */
InitializeThread::InitializeThread(const sead::SafeString& rName, const FunctorBase& rFunctor,
                                   sead::Heap* pHeap, s32 priority, s32 stackSize)
    : mHeap(pHeap) {
    s32 threadStackSize = stackSize * 3;
    mThread = new sead::DelegateThread(
        rName,
        new sead::Delegate2<InitializeThread, sead::Thread*, s64>(
            this, &InitializeThread::threadFunction),
        pHeap, priority, sead::MessageQueue::BlockType::NonBlocking, 0x7fffffff,
        threadStackSize, 0x20);
    mFunctor = rFunctor.clone();
}

/**
 * @brief Thread entry point: runs the functor with the thread's heap set, then quits the thread.
 * @param pThread The thread running this function.
 * @param msg The message that woke up the thread.
 */
void InitializeThread::threadFunction(sead::Thread* pThread, s64 msg) {
    sead::ScopedCurrentHeapSetter setter(mHeap);
    (*mFunctor)();
    mThread->quit(false);
}

/**
 * @brief Starts the thread.
 */
void InitializeThread::start() {
    mThread->start();
    mIsDone = false;
}

/**
 * @brief Destroys the thread if it has finished running.
 * @return True if the thread is finished and has been destroyed.
 */
bool InitializeThread::tryWaitDoneAndDestroy() {
    if (mIsDone) {
        return true;
    }

    if (mThread->isDone()) {
        mThread->destroy();
        mIsDone = true;
        return true;
    }

    return false;
}
}  // namespace al
