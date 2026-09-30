#include "Project/Framework/MultiCoreQueueThread.hpp"

#include <mc/seadCoreInfo.h>
#include <prim/seadDelegate.h>
#include <thread/seadDelegateThread.h>

#include <cstring>

namespace al {
/**
 * Creates the worker thread on the first sub core.
 * @param queueSize maximum number of queued executors
 */
MultiCoreQueueThread::MultiCoreQueueThread(s32 queueSize) {
    mThread = new sead::DelegateThread(
        "ClippingAreaMultiCoreUpdate",
        new sead::Delegate2<MultiCoreQueueThread, sead::Thread*, sead::MessageQueue::Element>(
            this, &MultiCoreQueueThread::threadFunc_),
        nullptr, sead::Thread::cDefaultPriority, sead::MessageQueue::BlockType::Blocking,
        0x7fffffff, 0x20000, queueSize);
    mThread->setAffinity(sead::CoreIdMask(sead::CoreId::cSub1));
    mDoneQueue.allocate(queueSize, nullptr);
    mThread->start();
    mExecutors = new MultiCoreQueueExecutor*[queueSize];
    memset(mExecutors, 0, sizeof(MultiCoreQueueExecutor*) * queueSize);
    mMaxExecutors = queueSize;
    mExecutorNum = 0;
}

/**
 * Executes a queued executor on the worker thread and reports it as done.
 * @param pThread worker thread
 * @param msg queued executor
 */
void MultiCoreQueueThread::threadFunc_(sead::Thread* pThread, sead::MessageQueue::Element msg) {
    reinterpret_cast<MultiCoreQueueExecutor*>(msg)->executeOnThread();
    mDoneQueue.push(msg, sead::MessageQueue::BlockType::Blocking);
}

/**
 * Queues an executor unless it is already queued.
 * @param pExecutor executor
 */
void MultiCoreQueueThread::requestExecute(MultiCoreQueueExecutor* pExecutor) {
    if (isQueued(pExecutor)) {
        return;
    }

    if (mExecutorNum < mMaxExecutors) {
        mExecutors[mExecutorNum] = pExecutor;
        mExecutorNum++;
        mThread->sendMessage(reinterpret_cast<sead::MessageQueue::Element>(pExecutor),
                             sead::MessageQueue::BlockType::Blocking);
    }
}

/**
 * Checks if an executor is queued.
 * @param pExecutor executor
 * @return whether the executor is queued
 */
bool MultiCoreQueueThread::isQueued(MultiCoreQueueExecutor* pExecutor) {
    for (s32 i = 0; i < mExecutorNum; i++) {
        if (mExecutors[i] == pExecutor) {
            return true;
        }
    }

    return false;
}

/**
 * Waits until all queued executors are done.
 */
void MultiCoreQueueThread::waitDone() {
    if (mExecutorNum <= 0) {
        return;
    }

    for (s32 i = 0; i < mExecutorNum; i++) {
        mDoneQueue.pop(sead::MessageQueue::BlockType::Blocking);
    }

    mExecutorNum = 0;
}

/**
 * Stops the worker thread.
 */
MultiCoreQueueThread::~MultiCoreQueueThread() {
    mThread->quitAndWaitDoneSingleThread(false);
}
}  // namespace al
