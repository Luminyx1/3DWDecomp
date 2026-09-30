#include <atomic>

#include "basis/seadRawPrint.h"
#include "mc/seadJobQueue.h"
#include "mc/seadWorker.h"
#include "prim/seadScopedLock.h"

namespace sead
{
/**
 * Constructs a job queue with every core disabled and a granularity of 8.
 */
JobQueue::JobQueue()
{
    for (auto& enabled : mCoreEnabled.mBuffer)
    {
        enabled = 0;
    }

    mNumDoneJobs.storeNonAtomic(0);
    mGranularity.fill(8);
}

/**
 * Runs jobs until the whole queue has been processed.
 * @param pFinishedJobs Receives the number of jobs run.
 */
void JobQueue::runAll(u32* pFinishedJobs)
{
    const u32 size = getNumJobs();
    *pFinishedJobs = 0;
    while (true)
    {
        u32 finished_jobs_batch = 0;
        const bool ok = run(size, &finished_jobs_batch, nullptr);
        *pFinishedJobs += finished_jobs_batch;
        if (ok)
        {
            break;
        }
    }

    SEAD_ASSERT(*pFinishedJobs == size);
}

/**
 * Checks whether every participating core has finished.
 * @return true if no core is still marked as enabled.
 */
bool JobQueue::isAllParticipantThrough() const
{
    for (auto value : mCoreEnabled.mBuffer)
    {
        if (value)
        {
            return false;
        }
    }

    return true;
}

/**
 * Sets how many jobs one core takes at a time.
 * @param core Core to configure.
 * @param x Number of jobs per batch; 0 is treated as 1.
 */
void JobQueue::setGranularity(CoreId core, u32 x)
{
    mGranularity[core] = x ? x : 1;
}

/**
 * Sets how many jobs every core takes at a time.
 * @param x Number of jobs per batch; 0 is treated as 1.
 */
void JobQueue::setGranularity(u32 x)
{
    for (s32 i = 0; i < mGranularity.size(); ++i)
    {
        setGranularity(i, x);
    }
}

/**
 * Selects the participating cores and the synchronisation mode.
 * @param mask Cores that take part in running the queue.
 * @param type How waiting on the queue is synchronised.
 */
void JobQueue::setCoreMaskAndWaitType(CoreIdMask mask, SyncType type)
{
    mStatus = Status::_6;
    mMask = mask;

    for (u32 i = 0; i < CoreInfo::getNumCores(); ++i)
    {
        mCoreEnabled[i] = mask.isOn(i);
        mNumDoneJobs = 0;
    }

    mSyncType = type;
}

/**
 * Marks a core as finished and waits for the queue to complete.
 * @param core Core that finished its work.
 */
void JobQueue::FINISH(CoreId core)
{
    std::atomic_thread_fence(std::memory_order_seq_cst);
    mCoreEnabled[core] = 0;
    wait_AT_WORKER();
}

/**
 * Waits for the queue to complete from a worker thread.
 */
void JobQueue::wait_AT_WORKER()
{
    std::atomic_thread_fence(std::memory_order_seq_cst);

    switch (mSyncType)
    {
        case SyncType::cCore:
            if (!isDone_())
            {
                mFinishEvent.wait();
            }

            break;
        case SyncType::cThread:
            SEAD_ASSERT_MSG(false, "*NOT YET\n");

            if (!isDone_())
            {
                mFinishEvent.wait();
            }

            break;
        default:
            break;
    }
}

/**
 * Waits for the queue to complete.
 */
void JobQueue::wait()
{
    switch (mSyncType)
    {
        case SyncType::cNoSync:
        case SyncType::cCore:
            if (!isDone_())
            {
                mFinishEvent.wait();
            }

            break;
        case SyncType::cThread:
            SEAD_ASSERT_MSG(false, "NOT IMPLEMENTED.\n");

            if (!isDone_())
            {
                mFinishEvent.wait();
            }

            break;
        default:
            break;
    }
}

/**
 * Allocates the per-core bars and counters.
 * @param pName Name given to the process meter bar.
 * @param pHeap Heap used for the per-core buffers.
 */
void PerfJobQueue::initialize(const char* pName, Heap* pHeap)
{
    mBars.allocBufferAssert(CoreInfo::getNumCores(), pHeap);
    mInts.allocBufferAssert(CoreInfo::getNumCores(), pHeap);

    for (s32 i = 0; i < mInts.size(); ++i)
    {
        mInts[CoreId(i)] = 0;
    }

    for (s32 i = 0; i < mBars.size(); ++i)
    {
        mBars[i].setName("?");
    }

    mProcessMeterBar.setColor({1, 1, 0, 1});
    mProcessMeterBar.setName(pName);
}

/**
 * Frees the per-core bars and counters.
 */
void PerfJobQueue::finalize()
{
    mInts.freeBuffer();
    mBars.freeBuffer();
}

/**
 * Resets the per-core colour counters.
 */
void PerfJobQueue::reset()
{
    for (s32 i = 0; i < mInts.size(); ++i)
    {
        mInts[CoreId(i)] = 0;
    }
}

/**
 * Starts measuring job dequeuing on the current core.
 */
void PerfJobQueue::measureBeginDeque()
{
    auto& bar = mBars[getCurrentCoreIdx_()];
    auto& idx = mInts[getCurrentCoreIdx_()];
    static_cast<void>(idx);
    bar.measureBegin(Color4f::cWhite);
}

/**
 * Stops measuring job dequeuing on the current core.
 */
void PerfJobQueue::measureEndDeque()
{
    mBars[getCurrentCoreIdx_()].measureEnd();
}

// NON_MATCHING: inlined getBarColor references lbl_ symbols in the target
void PerfJobQueue::measureBeginRun()
{
    auto& bar = mBars[getCurrentCoreIdx_()];
    auto& idx = mInts[getCurrentCoreIdx_()];
    bar.measureBegin(getBarColor(idx));
    idx = (idx + 1) % 9;
}

/**
 * Stops measuring job execution on the current core.
 */
void PerfJobQueue::measureEndRun()
{
    mBars[getCurrentCoreIdx_()].measureEnd();
}

// NON_MATCHING: sColors and its guard are lbl_ symbols in the target
const Color4f& PerfJobQueue::getBarColor(u32 idx) const
{
    static const SafeArray<Color4f, 9> sColors = {{
        {0.2078431397676468, 0.8313725590705872, 0.6274510025978088, 1.0},
        {0.0, 0.6666666865348816, 0.4470588266849518, 1.0},
        {0.125490203499794, 0.49803921580314636, 0.3764705955982208, 1.0},
        {0.7490196228027344, 0.5254902243614197, 0.1882352977991104, 1.0},
        {1.0, 0.6000000238418579, 0.0, 1.0},
        {1.0, 0.6980392336845398, 0.250980406999588, 1.0},
        {0.6901960968971252, 0.1725490242242813, 0.29411765933036804, 1.0},
        {0.9176470637321472, 0.0, 0.21568627655506134, 1.0},
        {0.9607843160629272, 0.239215686917305, 0.40784314274787903, 1.0},
    }};
    return sColors.mBuffer[idx];
}

/**
 * Attaches the bars to the process meter; does nothing in release builds.
 */
void PerfJobQueue::attachProcessMeter() {}

/**
 * Detaches the bars from the process meter; does nothing in release builds.
 */
void PerfJobQueue::detachProcessMeter() {}

/**
 * Constructs an empty fixed-size job queue.
 */
FixedSizeJQ::FixedSizeJQ()
{
    mStatus.storeNonAtomic(Status::_0);
    mNumJobs = 0;
    mNumProcessedJobs = 0;
}

/**
 * Prepares the queue for running; nothing to do for a fixed-size queue.
 */
void FixedSizeJQ::begin() {}

// NON_MATCHING: register allocation and scheduling of begin/end
bool FixedSizeJQ::run(u32 size, u32* pFinishedJobs, Worker* pWorker)
{
    *pFinishedJobs = 0;

    mPerf.measureBeginDeque();
    u32 num_finished = 0;
    bool ret = true;
    s32 begin = 0;
    s32 end = -1;

    if (size > 0 && mNumJobs > 0)
    {
        if (pWorker)
        {
            pWorker->setState(Worker::State::cRunning_WaitLock);
        }

        mLock.lock();

        if (pWorker)
        {
            pWorker->setState(Worker::State::cRunning_GetLock);
        }

        begin = mNumProcessedJobs;
        const auto num_jobs = mNumJobs;
        num_finished = std::min(num_jobs - begin, size);

        mNumProcessedJobs = num_finished + begin;
        end = num_finished + begin - 1;
        mLock.unlock();
        ret = num_finished + begin >= num_jobs;
    }

    mPerf.measureEndDeque();

    mPerf.measureBeginRun();

    if (pWorker)
    {
        pWorker->setState(Worker::State::cRunning_Run);
    }

    for (s32 i = begin; i <= end; ++i)
    {
        mJobs[i]->invoke();
    }

    if (pWorker)
    {
        pWorker->setState(Worker::State::cRunning_AfterRun);
    }

    mPerf.measureEndRun();

    if (ret)
    {
        if (pWorker)
        {
            pWorker->setState(Worker::State::cRunning_AllJobDoneReturn);
        }
    }
    else
    {
        if (pWorker)
        {
            pWorker->setState(Worker::State::cRunning_BeforeReturn);
        }
    }

    *pFinishedJobs = num_finished;
    return ret;
}

/**
 * Allocates storage for the jobs.
 * @param size Maximum number of jobs.
 * @param pHeap Heap used for the job array.
 */
void FixedSizeJQ::initialize(u32 size, Heap* pHeap)
{
    mPerf.initialize(getName().cstr(), pHeap);

    ScopedLock<JobQueueLock> lock(&mLock);
    mJobs.allocBufferAssert(size, pHeap);
    mNumJobs = 0;
    mNumProcessedJobs = 0;
    mStatus = Status::_1;
}

/**
 * Frees the job storage.
 */
void FixedSizeJQ::finalize()
{
    mPerf.finalize();
    mJobs.freeBuffer();
}

/**
 * Adds a job without locking.
 * @param pJob Job to add.
 * @return false if the queue is full.
 */
bool FixedSizeJQ::enque(Job* pJob)
{
    mStatus = Status::_3;

    if (mNumJobs >= u32(mJobs.size()))
    {
        return false;
    }

    mJobs[mNumJobs++] = pJob;
    return true;
}

/**
 * Adds a job while holding the queue lock.
 * @param pJob Job to add.
 * @return false if the queue is full.
 */
bool FixedSizeJQ::enqueSafe(Job* pJob)
{
    mStatus = Status::_3;

    ScopedLock<JobQueueLock> lock(&mLock);

    if (mNumJobs >= u32(mJobs.size()))
    {
        return false;
    }

    mJobs[mNumJobs++] = pJob;
    return true;
}

/**
 * Takes the next unprocessed job.
 * @return The job, or nullptr if all jobs have been taken.
 */
Job* FixedSizeJQ::deque()
{
    ScopedLock<JobQueueLock> lock(&mLock);

    if (mNumProcessedJobs >= mNumJobs)
    {
        return nullptr;
    }

    return mJobs[mNumProcessedJobs++];
}

/**
 * Takes up to count unprocessed jobs.
 * @param pJobs Receives the jobs.
 * @param count Maximum number of jobs to take.
 * @return Number of jobs taken.
 */
u32 FixedSizeJQ::deque(Job** pJobs, u32 count)
{
    ScopedLock<JobQueueLock> lock(&mLock);

    u32 ret = 0;

    while (mNumProcessedJobs < mNumJobs && ret < count)
    {
        pJobs[ret] = mJobs[mNumProcessedJobs++];
        ++ret;
    }

    return ret;
}

/**
 * Marks every job as unprocessed again.
 * @return Always true.
 */
bool FixedSizeJQ::rewind()
{
    mPerf.reset();
    mNumProcessedJobs = 0;
    return true;
}

/**
 * Removes all jobs.
 */
void FixedSizeJQ::clear()
{
    mStatus = Status::_5;
    mPerf.reset();
    mNumJobs = 0;
    mNumProcessedJobs = 0;
    mSyncType = SyncType::cNoSync;
}

/**
 * Checks whether every job has been taken.
 * @return true if no unprocessed job remains.
 */
bool FixedSizeJQ::debug_IsAllJobDone()
{
    return mNumProcessedJobs >= mNumJobs;
}
}  // namespace sead
