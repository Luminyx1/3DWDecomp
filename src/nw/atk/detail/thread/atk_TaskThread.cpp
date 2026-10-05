#include <nn/atk/atk_TaskThread.h>
#include <nn/atk/atk_TaskManager.h>
#include <nn/atk/atk_DriverCommand.h>

namespace nn::atk::detail {
/** @brief Creates an inactive task worker with a recursive execution mutex and normal file priority. */
TaskThread::TaskThread()
    : mMutex(true), mStopRequested(false), mCreated(false), mFsPriority(static_cast<FsPriority>(1)) {}

/** @brief Stops the worker before destroying its mutex and thread resources. */
TaskThread::~TaskThread() { Destroy(); }

/** @brief Gets the process-wide task worker. @return Lazily initialized worker instance. */
TaskThread& TaskThread::GetInstance() {
    static TaskThread instance;
    return instance;
}

/** @brief Requests termination, wakes a waiting worker, and waits for it to release its thread. */
void TaskThread::Destroy() {
    if (mCreated) {
        mStopRequested = true;
        TaskManager::GetInstance().CancelWaitTask();
        mThread.WaitForExit();
        mThread.Release();
        mCreated = false;
    }
}

/**
 * @brief Restarts the task worker using a caller-owned stack.
 * @param priority Thread scheduling priority accepted by the underlying thread implementation.
 * @param pStack Stack storage that must remain valid until Destroy completes.
 * @param stackSize Size of the stack storage in bytes; must be nonzero.
 * @param core Preferred processor core passed to the thread affinity configuration.
 * @param affinityMask Allowed processor mask, or zero for the thread's default affinity.
 * @param fsPriority File-system scheduling priority used by the worker.
 * @return True if the operating-system thread starts successfully.
 */
bool TaskThread::Create(int priority, void* pStack, size_t stackSize, int core, u32 affinityMask,
                        FsPriority fsPriority) {
    Destroy();
    mStopRequested = false;
    fnd::Thread::RunArgs args;
    args.name = "nn::atk::detail::TaskThread";
    args.stack = pStack;
    args.stackSize = stackSize;
    args.core = core;
    args.affinity = static_cast<fnd::Thread::AffinityMask>(affinityMask);
    args.priority = priority;
    args.fsPriority = static_cast<fnd::Thread::FsPriority>(fsPriority);
    args.argument = nullptr;
    args.handler = this;
    if (!mThread.Run(args)) {
        return false;
    }
    mFsPriority = fsPriority;
    mCreated = true;
    return true;
}

/**
 * @brief Executes queued tasks and collects their driver command replies until termination is requested.
 * @param pArg Unused thread callback context; Create supplies nullptr.
 * @return Zero after the worker stops.
 */
u32 TaskThread::Run(void* pArg) {
    while (!mStopRequested) {
        TaskManager::GetInstance().WaitTask();
        if (mStopRequested) {
            break;
        }
        mMutex.Lock();
        TaskManager::GetInstance().ExecuteTask();
        mMutex.Unlock();
        DriverCommand::GetInstanceForTaskThread().RecvCommandReply();
    }
    return 0;
}
} // namespace nn::atk::detail
