#include "Library/Thread/AsyncFunctorThread.hpp"

#include <prim/seadDelegate.h>
#include <thread/seadDelegateThread.h>

#include "Library/Thread/Functor.hpp"
#include "Library/Thread/InitializeThread.hpp"

namespace al {
/**
 * @brief Creates and starts a thread that runs a functor each time start is called.
 * @param rName The name of the thread.
 * @param rFunctor The functor to run; a copy of it is stored.
 * @param priority The priority of the thread.
 * @param stackSize The stack size of the thread, or a negative value for the default of 0x1000.
 */
AsyncFunctorThread::AsyncFunctorThread(const sead::SafeString& rName, const FunctorBase& rFunctor,
                                       s32 priority, s32 stackSize) {
    s32 size = stackSize < 0 ? 0x1000 : stackSize;
    mThread = new sead::DelegateThread(
        rName,
        new sead::Delegate2<AsyncFunctorThread, sead::Thread*, s64>(
            this, &AsyncFunctorThread::threadFunction),
        nullptr, priority, sead::MessageQueue::BlockType::Blocking, 0x7fffffff, size, 4);
    mFunctor = rFunctor.clone();
    mThread->start();
}

/**
 * @brief Runs the functor and marks the work as done.
 * @param pThread The thread running this function.
 * @param msg The message that woke up the thread.
 */
void AsyncFunctorThread::threadFunction(sead::Thread* pThread, s64 msg) {
    (*mFunctor)();
    mIsDone = true;
}

/**
 * @brief Stops the thread and waits for it to finish.
 */
AsyncFunctorThread::~AsyncFunctorThread() {
    mThread->quitAndWaitDoneSingleThread(false);
}

/**
 * @brief Wakes up the thread to run the functor once.
 */
void AsyncFunctorThread::start() {
    mThread->sendMessage(1, sead::MessageQueue::BlockType::NonBlocking);
    mIsDone = false;
}

/**
 * @brief Checks whether the functor has finished running.
 * @return True if the last run is done.
 */
bool AsyncFunctorThread::isDone() const {
    return mIsDone;
}

/**
 * @brief Creates an initialization thread for a functor and starts it.
 * @param pHeap The heap set as current while the functor runs.
 * @param priority The priority of the thread.
 * @param rFunctor The functor to run.
 * @return The started thread.
 */
InitializeThread* createAndStartInitializeThread(sead::Heap* pHeap, s32 priority,
                                                 const FunctorBase& rFunctor) {
    InitializeThread* pThread =
        new InitializeThread("シーン初期化スレッド", rFunctor, pHeap, priority, 0x20000);
    pThread->start();
    return pThread;
}

/**
 * @brief Destroys an initialization thread if it has finished running.
 * @param pThread The thread to check.
 * @return True if the thread is finished and has been destroyed.
 */
bool tryWaitDoneAndDestroyInitializeThread(InitializeThread* pThread) {
    return pThread->tryWaitDoneAndDestroy();
}
}  // namespace al
