#include <mc/seadWorker.h>

#include <prim/seadScopedLock.h>

namespace sead {

/**
 * Creates a worker and its pending-queue buffer, initially signalled as idle.
 * @param pMgr Manager that owns the worker.
 * @param numJobs Maximum number of pending queues.
 * @param stackSize Thread stack size in bytes.
 * @param priority Thread scheduling priority.
 * @param rName Thread name.
 */
Worker::Worker(WorkerMgr* pMgr, u32 numJobs, s32 stackSize, s32 priority, const SafeString& rName)
    : Thread(rName, nullptr, priority, MessageQueue::BlockType::Blocking, 0x7fffffff, stackSize, 1),
      mMgr(pMgr)
{
    mJobQueues.allocBuffer(numJobs, nullptr);
    mEvent.setSignal();
}

/** Clears the pending queues while holding the queue lock. */
void Worker::clearJobQQ()
{
    ScopedLock<JobQueueLock> lock(&mLock);
    mJobQueues.clear();
}

/**
 * Dispatches a process message to the worker loop.
 * @param msg Received thread message; other messages are ignored.
 */
void Worker::calc_(MessageQueue::Element msg)
{
    if (msg == cMsg_Process)
        proc_();
}

/**
 * Inserts a queue and records its diagnostic description.
 * @param pName Description of the submitted work.
 * @param pQueue Queue to insert.
 * @param type Forward appends to the queue; backward inserts at its front.
 * @return Whether the pending-queue buffer had room.
 */
bool Worker::pushJobQueue(const char* pName, JobQueue* pQueue, JobQueuePushType type)
{
    ScopedLock<JobQueueLock> lock(&mLock);
    bool result;

    if (type == JobQueuePushType::cForward)
        result = mJobQueues.pushBack(pQueue);
    else
        result = mJobQueues.pushBackwards(pQueue);
    pQueue->setDescription(pName);
    return result;
}

/**
 * Removes the oldest pending queue while holding the queue lock.
 * @return The next queue, or nullptr when none remains.
 */
JobQueue* Worker::getNextJQ_()
{
    ScopedLock<JobQueueLock> lock(&mLock);
    return mJobQueues.popFront();
}

/**
 * Wakes the thread when queues are pending and clears its completion event.
 * @param msg Message to send without blocking.
 */
void Worker::wakeup_(MessageQueue::Element msg)
{
    if (mJobQueues.size() == 0)
        return;
    mEvent.resetSignal();
    mWorkerState = State::cWakeup;
    sendMessage(msg, MessageQueue::BlockType::NonBlocking);
}

/** Runs pending queues, records completion and signals when the worker is idle. */
void Worker::proc_()
{
    ++mNumRuns;
    mLastRun.setNow();
    mWorkerState = State::cRunning;
    JobQueue* queue = getNextJQ_();
    const u32 core = mCore;

    while (queue) {
        mCurrentQueue = queue;
        mCurrentQueueDescription = queue->getDescription();
        const u32 granularity = queue->getGranularity(core);
        queue->resetFinishEvent();
        volatile bool done = false;
        u32 total = 0;

        while (!done) {
            u32 finished = 0;
            done = queue->run(granularity, &finished, this);
            total += finished;
        }

        if (queue->addNumDoneJobs(total) + total >= queue->getNumJobs())
            queue->signalFinishEvent();
        mWorkerState = State::cFinished;
        queue->FINISH(mCore);
        mWorkerState = State::cWaitingAtWorker;
        mWorkerState = State::cRunning;
        queue = getNextJQ_();
    }

    mCurrentQueue = nullptr;
    mCurrentQueueDescription = nullptr;
    mWorkerState = State::cSleep;
    mEvent.setSignal();
}

}  // namespace sead
