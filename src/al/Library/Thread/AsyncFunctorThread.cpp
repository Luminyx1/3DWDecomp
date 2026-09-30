#include "Library/Thread/AsyncFunctorThread.hpp"

#include <prim/seadDelegate.h>
#include <thread/seadDelegateThread.h>

#include "Project/Thread/InitializeThread.hpp"

namespace al {
/**
 * Creates a delegate thread that runs a copy of the functor on request.
 * @param rName Thread name.
 * @param rFunctor Functor to run.
 * @param priority Thread priority.
 * @param stackSize Stack size, or a negative value for the default.
 */
AsyncFunctorThread::AsyncFunctorThread(const sead::SafeString& rName, const FunctorBase& rFunctor,
                                       s32 priority, s32 stackSize) {
    s32 size = stackSize < 0 ? 0x1000 : stackSize;
    mDelegateThread = new sead::DelegateThread(
        rName,
        new sead::Delegate2<AsyncFunctorThread, sead::Thread*, sead::MessageQueue::Element>(
            this, &AsyncFunctorThread::threadFunction),
        nullptr, priority, sead::MessageQueue::BlockType::Blocking, 0x7fffffff, size, 4);
    mFunctor = rFunctor.clone();
    mDelegateThread->start();
}

/**
 * Runs the functor and marks the thread as done.
 * @param pThread Unused.
 * @param message Unused.
 */
void AsyncFunctorThread::threadFunction(sead::Thread* pThread,
                                        sead::MessageQueue::Element message) {
    (*mFunctor)();
    mIsDone = true;
}

AsyncFunctorThread::~AsyncFunctorThread() {
    mDelegateThread->quitAndWaitDoneSingleThread(false);
}

/**
 * Wakes the thread up to run the functor once.
 */
void AsyncFunctorThread::start() {
    mDelegateThread->sendMessage(1, sead::MessageQueue::BlockType::NonBlocking);
    mIsDone = false;
}

/**
 * Checks whether the functor has finished running.
 * @return True if done.
 */
bool AsyncFunctorThread::isDone() const {
    return mIsDone;
}

/**
 * Creates and starts a scene initialization thread.
 * @param pHeap Heap used by the thread.
 * @param priority Thread priority.
 * @param rFunctor Functor to run.
 * @return The started thread.
 */
InitializeThread* createAndStartInitializeThread(sead::Heap* pHeap, s32 priority,
                                                 const FunctorBase& rFunctor) {
    auto* thread = new InitializeThread("シーン初期化スレッド", rFunctor, pHeap, priority, 0x20000);
    thread->start();
    return thread;
}

/**
 * Destroys an initialization thread if it is done.
 * @param pThread The thread.
 * @return True if the thread is done.
 */
bool tryWaitDoneAndDestroyInitializeThread(InitializeThread* pThread) {
    return pThread->tryWaitDoneAndDestroy();
}
}  // namespace al
