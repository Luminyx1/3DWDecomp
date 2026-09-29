#include "mc/seadWorkerMgr.h"
#include "framework/seadInfLoopChecker.h"
#include "prim/seadSafeString.h"

namespace sead
{
/**
 * Constructs the manager with its infinite-loop event slot bound to onInfLoop_.
 */
WorkerMgr::WorkerMgr()
    : mInfLoopEventSlot{
          Delegate1<WorkerMgr, const InfLoopChecker::InfLoopParam&>(this, &WorkerMgr::onInfLoop_)}
{
}

/**
 * Handles an infinite-loop report; only samples the current tick in release builds.
 */
void WorkerMgr::onInfLoop_(const InfLoopChecker::InfLoopParam&)
{
    TickTime time;
}

/**
 * Dumps the worker states; only samples the current tick in release builds.
 */
void WorkerMgr::dump()
{
    TickTime time;
}

/**
 * Sets the default job count, name, stack sizes and priorities.
 */
WorkerMgr::InitializeArg::InitializeArg()
{
    worker_num_jobs = 0x20;
    name = "WorkerMgr";
    thread_stack_sizes[0] = 0x1000;
    thread_stack_sizes[1] = 0x8000;
    thread_priorities.fill(Thread::cDefaultPriority);
    thread_stack_sizes[2] = 0x8000;
}

/**
 * Builds the name of the worker thread for a core.
 * @param pHeap Heap the name string is allocated from.
 * @param rArg Initialisation arguments providing the manager name.
 * @param core Core index of the worker.
 * @return Newly allocated name.
 */
static SafeString* makeWorkerName(Heap* pHeap, const WorkerMgr::InitializeArg& rArg, u32 core)
{
    FixedSafeString<128> name;
    const char* core_name = "?";
#ifdef SEAD_DEBUG
    core_name = CoreId(core).text();
#endif
    name.format("%s/Worker%d(%s)", rArg.name, core, core_name);
    return new HeapSafeString(pHeap, name);
}

/**
 * Creates one worker per core and starts every worker except the one on the main core.
 * @param rArg Initialisation arguments.
 */
void WorkerMgr::initialize(const InitializeArg& rArg)
{
    if (InfLoopChecker::instance())
    {
        InfLoopChecker::instance()->getEvent().connect(mInfLoopEventSlot);
    }

    const u32 num_cores = CoreInfo::getNumCores();
    auto* heap = HeapMgr::instance()->getCurrentHeap();
    SEAD_ASSERT(heap);

    mWorkers.allocBufferAssert(num_cores, heap);

    for (u32 i = 0; i < num_cores; ++i)
    {
        auto* name = makeWorkerName(heap, rArg, i);
        auto* worker = new Worker(this, rArg.worker_num_jobs, rArg.thread_stack_sizes[i],
                                  rArg.thread_priorities[i], *name);
        mWorkers[i] = worker;
        worker->mCore = i;
        if (worker->mCore)
        {
            worker->setAffinity(CoreIdMask(i));
            worker->start();
        }
    }

    mJobQueues.allocBufferAssert(64, nullptr);
    mNumJobQueues = 0;
}

/**
 * Stops the worker threads and destroys the workers.
 */
void WorkerMgr::finalize()
{
    if (mWorkers.size() > 1)
    {
        ThreadMgr::quitAndWaitDoneMultipleThread(
            reinterpret_cast<Thread**>(mWorkers.getBufferPtr() + 1), mWorkers.size() - 1, true);
    }

    for (u32 i = 0, n = CoreInfo::getNumCores(); i != n; ++i)
    {
        if (mWorkers[i])
        {
            delete mWorkers[i];
        }
        mWorkers[i] = nullptr;
    }
}

/**
 * Pushes a job queue to the workers of the selected cores without a context name.
 * @param pQueue Queue to run.
 * @param coreIdMask Cores that run the queue.
 * @param syncType How waiting on the queue is synchronised.
 * @param pushType Where the queue is inserted in each worker.
 */
void WorkerMgr::pushJobQueue(JobQueue* pQueue, CoreIdMask coreIdMask, SyncType syncType,
                             JobQueuePushType pushType)
{
    pushJobQueue("nocontext", pQueue, coreIdMask, syncType, pushType);
}

/**
 * Pushes a job queue to the workers of the selected cores.
 * @param pContextName Name of the caller, used for diagnostics.
 * @param pQueue Queue to run.
 * @param coreIdMask Cores that run the queue.
 * @param syncType How waiting on the queue is synchronised.
 * @param pushType Where the queue is inserted in each worker.
 */
void WorkerMgr::pushJobQueue(const char* pContextName, JobQueue* pQueue, CoreIdMask coreIdMask,
                             SyncType syncType, JobQueuePushType pushType)
{
    SEAD_ASSERT_MSG(coreIdMask, "coreIdMask must not be 0. pContextName = %s", pContextName);

    pQueue->setCoreMaskAndWaitType(coreIdMask, syncType);
    pQueue->begin();

    for (int i = 0; i < mWorkers.size(); ++i)
    {
        if (coreIdMask.isOn(i))
        {
            mWorkers[i]->pushJobQueue(pContextName, pQueue, pushType);
        }
    }

    mJobQueues[mNumJobQueues] = pQueue;
    ++mNumJobQueues;
}

/**
 * Runs the pushed queues on the calling thread or wakes up the worker threads.
 */
void WorkerMgr::run()
{
    if (mProcessJobQueues)
    {
        for (u32 i = 0; i < mNumJobQueues; ++i)
        {
            mJobQueues[i]->begin();
            u32 finished_jobs = 0;
            mJobQueues[i]->runAll(&finished_jobs);
            mJobQueues[i]->addNumDoneJobs(finished_jobs);
            mJobQueues[i]->FINISH(CoreInfo::getCurrentCoreId());

            for (int j = 0; j < mWorkers.size(); ++j)
            {
                mWorkers[j]->clearJobQQ();
            }
        }
    }
    else
    {
        ++mNumWakeups;
        mLastWakeup.setNow();
        for (int i = 0; i < mWorkers.size(); ++i)
        {
            if (mWorkers[i]->mCore)
            {
                mWorkers[i]->wakeup_(Worker::cMsg_Process);
            }
        }
    }
}

/**
 * Waits until every worker has finished its queues.
 */
void WorkerMgr::sync()
{
    if (!mProcessJobQueues)
    {
        mWorkers[0]->proc_();
    }

    for (auto it = mWorkers.begin(1), end = mWorkers.end(); it != end; ++it)
    {
        while (!(*it)->mEvent.wait(mWaitDuration))
        {
            continue;
        }
    }

    if (!isAllWorkerSleep())
    {
        std::array<Worker::State, 256> states{};
        u32 idx = 0;
        for (int i = 0; i < mWorkers.size(); ++i)
        {
            states[idx] = mWorkers[i]->mWorkerState.load();
            ++idx;
        }

        for (int i = 0; i < mWorkers.size(); ++i)
        {
            SEAD_DEBUG_PRINT(" [%d] [%s] = %s\n", i, mWorkers[i]->mCore.text(), states[i].text());
        }

        SEAD_ASSERT_MSG(false, "all sleep failed\n");
    }

    mNumJobQueues = 0;
}

/**
 * Checks whether every worker is sleeping.
 * @return true if all workers are in the sleep state.
 */
bool WorkerMgr::isAllWorkerSleep() const
{
    for (int i = 0; i < mWorkers.size(); ++i)
    {
        if (mWorkers[i]->mWorkerState.load() != Worker::State::cSleep)
        {
            return false;
        }
    }
    return true;
}

}  // namespace sead
