#include "mc/seadWorkerMgr.h"
#include "framework/seadInfLoopChecker.h"
#include "prim/seadSafeString.h"

namespace sead
{
WorkerMgr::WorkerMgr()
    : mInfLoopEventSlot{
          Delegate1<WorkerMgr, const InfLoopChecker::InfLoopParam&>(this, &WorkerMgr::onInfLoop_)}
{
}

void WorkerMgr::onInfLoop_(const InfLoopChecker::InfLoopParam&)
{
    TickTime time;
}

WorkerMgr::InitializeArg::InitializeArg()
{
    worker_num_jobs = 0x20;
    name = "WorkerMgr";
    thread_stack_sizes[0] = 0x1000;
    thread_priorities.fill(Thread::cDefaultPriority);
    thread_stack_sizes[1] = thread_stack_sizes[2] = 0x8000;
}

static SafeString* makeWorkerName(Heap* pHeap, const WorkerMgr::InitializeArg& rArg, u32 core)
{
    FixedSafeString<128> name;
    const char* core_name = "?";
#ifdef SEAD_DEBUG
    core_name = CoreId(i).text();
#endif
    name.format("%s/Worker%d(%s)", rArg.name, core, core_name);
    return new HeapSafeString(pHeap, name);
}

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

void WorkerMgr::pushJobQueue(JobQueue* pQueue, CoreIdMask coreIdMask, SyncType syncType,
                             JobQueuePushType pushType)
{
    pushJobQueue("nocontext", pQueue, coreIdMask, syncType, pushType);
}

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
