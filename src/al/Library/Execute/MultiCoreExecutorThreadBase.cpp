#include "Library/Execute/MultiCoreExecutorThread.hpp"

#include <prim/seadDelegate.h>
#include <thread/seadDelegateThread.h>

#include "Library/Execute/ExecutorListBase.hpp"
#include "Library/LiveActor/LiveActor.hpp"

namespace al {
/**
 * Creates the worker thread on the given core.
 * @param queueSize Message queue size of the thread.
 * @param priority Thread priority.
 * @param core Core to run on.
 */
MultiCoreExecutorThreadBase::MultiCoreExecutorThreadBase(s32 queueSize, s32 priority,
                                                         sead::CoreId core) {
    mThread = new sead::DelegateThread(
        "MultiCoreUpdate",
        new sead::Delegate2<MultiCoreExecutorThreadBase, sead::Thread*, s64>(
            this, &MultiCoreExecutorThreadBase::threadFunction),
        nullptr, priority, sead::MessageQueue::BlockType::Blocking, 0x7fffffff, 0x20000,
        queueSize);
    mThread->setAffinity(sead::CoreIdMask(core));
    mMessageQueue.allocate(1, nullptr);
    mThread->start();
}

/**
 * Runs a message on the thread, or signals completion for the done marker.
 * @param pThread Unused.
 * @param message Message to execute, or -1 to signal completion.
 */
void MultiCoreExecutorThreadBase::threadFunction(sead::Thread* pThread, s64 message) {
    if (message == -1) {
        mMessageQueue.push(-1, sead::MessageQueue::BlockType::Blocking);
        return;
    }
    executeOnThread(message);
}

MultiCoreExecutorThreadBase::~MultiCoreExecutorThreadBase() {
    mThread->quitAndWaitDoneSingleThread(false);
}

/**
 * Waits until every queued message was executed.
 */
void MultiCoreExecutorThreadBase::waitDoneAll() {
    mThread->sendMessage(-1, sead::MessageQueue::BlockType::Blocking);
    mMessageQueue.pop(sead::MessageQueue::BlockType::Blocking);
}

/**
 * Executes the executor list passed as message.
 * @param message Pointer to the executor list.
 */
void MultiCoreExecutorThreadExecutorList::executeOnThread(s64 message) {
    reinterpret_cast<ExecutorListBase*>(message)->executeList();
}

/**
 * Calculates the anims of the null terminated actor array passed as message.
 * @param message Pointer to the actor array.
 */
void MultiCoreExecutorThreadActorCalcAnim::executeOnThread(s64 message) {
    LiveActor** actors = reinterpret_cast<LiveActor**>(message);
    LiveActor* actor = *actors;
    while (actor) {
        actor->calcAnim();
        actor = *actors++;
    }
}
}  // namespace al
