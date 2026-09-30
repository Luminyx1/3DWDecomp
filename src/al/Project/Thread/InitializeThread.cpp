#include "Project/Thread/InitializeThread.hpp"

#include <heap/seadHeapMgr.h>
#include <prim/seadDelegate.h>
#include <thread/seadDelegateThread.h>

#include "Library/Thread/Functor.hpp"

namespace al {
/**
 * Creates a delegate thread that runs a copy of the functor with the given heap.
 * @param rName Thread name.
 * @param rFunctor Functor to run.
 * @param pHeap Heap set as current while running.
 * @param priority Thread priority.
 * @param stackSize Base stack size (tripled).
 */
InitializeThread::InitializeThread(const sead::SafeString& rName, const FunctorBase& rFunctor,
                                   sead::Heap* pHeap, s32 priority, s32 stackSize)
    : mHeap(pHeap) {
    s32 size = stackSize * 3;
    mThread = new sead::DelegateThread(
        rName,
        new sead::Delegate2<InitializeThread, sead::Thread*, sead::MessageQueue::Element>(
            this, &InitializeThread::threadFunction),
        pHeap, priority, sead::MessageQueue::BlockType::NonBlocking, 0x7fffffff, size, 0x20);
    mFunctor = rFunctor.clone();
}

/**
 * Runs the functor with the thread heap set as current, then quits.
 * @param pThread Unused.
 * @param message Unused.
 */
void InitializeThread::threadFunction(sead::Thread* pThread, sead::MessageQueue::Element message) {
    sead::ScopedCurrentHeapSetter setter(mHeap);
    (*mFunctor)();
    mThread->quit(false);
}

/**
 * Starts the thread.
 */
void InitializeThread::start() {
    mThread->start();
    mIsDone = false;
}

/**
 * Destroys the thread once it has finished.
 * @return True if the thread is done.
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
