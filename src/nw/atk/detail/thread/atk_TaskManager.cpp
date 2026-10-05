#include <nn/atk/atk_TaskManager.h>

namespace nn::atk::detail {
namespace {
class ScopedMutexLock {
  public:
    /** @brief Acquires a mutex for the enclosing scope. @param rMutex Mutex to lock. */
    explicit ScopedMutexLock(os::Mutex& rMutex) : mMutex(rMutex) { mMutex.Lock(); }
    /** @brief Releases the mutex acquired at construction. */
    ~ScopedMutexLock() { mMutex.Unlock(); }

  private:
    os::Mutex& mMutex;
};
} // namespace

/** @brief Initializes empty priority queues, a recursive queue mutex, and task profiling. */
TaskManager::TaskManager() : mWaitCancelled(false), mMutex(true), mWakeQueue(mWakeMessages, 32) {}

/** @brief Gets the process-wide task queue manager. @return Lazily initialized manager. */
TaskManager& TaskManager::GetInstance() {
    static TaskManager instance;
    return instance;
}

/**
 * @brief Configures task profiling for this manager.
 * @param enableProfiling Whether executed tasks should collect profile records.
 */
void TaskManager::Initialize(bool enableProfiling) { mProfileLogger.SetProfilingEnabled(enableProfiling); }

/** @brief Disables task profiling and removes registered profile readers. */
void TaskManager::Finalize() {
    mProfileLogger.SetProfilingEnabled(false);
    mProfileLogger.Finalize();
}

/**
 * @brief Queues a task and sends a nonblocking worker wake-up notification.
 * @param pTask Non-null, unlinked task that must remain alive through completion.
 * @param priority Queue priority, in the range [TaskPriority_Low, TaskPriority_High].
 */
void TaskManager::AppendTask(Task* pTask, TaskPriority priority) {
    os::ClearEvent(&pTask->mCompletionEvent);
    pTask->mState = 1;
    mMutex.Lock();
    mTasks[priority].push_back(*pTask);
    mMutex.Unlock();
    os::TrySendMessageQueue(&mWakeQueue.queue, 0);
}

/**
 * @brief Reads the oldest task in one priority queue under the manager mutex.
 * @param priority Valid priority queue index.
 * @param remove Whether to unlink the returned task from its queue.
 * @return First task, or nullptr when that priority queue is empty.
 */
Task* TaskManager::GetNextTask(TaskPriority priority, bool remove) {
    ScopedMutexLock lock(mMutex);
    auto& rList = mTasks[priority];
    if (rList.empty()) {
        return nullptr;
    }
    Task* pTask = &*rList.begin();
    if (remove) {
        rList.pop_front();
    }
    return pTask;
}

/** @brief Removes the oldest task at the highest pending priority. @return Task, or nullptr if all queues are
 * empty. */
Task* TaskManager::PopTask() {
    ScopedMutexLock lock(mMutex);
    return SelectTask(true);
}

/** @brief Peeks at the oldest task at the highest pending priority. @return Task, or nullptr if all queues
 * are empty. */
Task* TaskManager::PeekTask() { return SelectTask(false); }

/** @brief Executes queued tasks in priority order and signals each task's completion. */
void TaskManager::ExecuteTask() {
    Task* pTask = PopTask();
    while (pTask != nullptr) {
        pTask->Execute(mProfileLogger);
        pTask->Complete(3);
        pTask = PopTask();
    }
}

/**
 * @brief Cancels a queued task without waiting for an already running task.
 * @param pTask Non-null task to locate by object identity in all priority queues.
 * @return True if the task was removed and signalled as cancelled.
 */
bool TaskManager::TryRemoveTask(Task* pTask) {
    ScopedMutexLock lock(mMutex);
    for (auto& rList : mTasks) {
        auto next = rList.begin();
        while (next != rList.end()) {
            auto it = next++;
            Task* pFound = &*it;
            if (pFound == pTask) {
                rList.erase(it);
                pFound->Complete(4);
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief Cancels a queued task or waits for its ongoing execution to finish.
 * @param pTask Non-null task whose completion event remains valid throughout this call.
 */
void TaskManager::CancelTask(Task* pTask) {
    if (!TryRemoveTask(pTask)) {
        os::WaitEvent(&pTask->mCompletionEvent);
    }
}

/**
 * @brief Cancels pending tasks sharing an identifier.
 * @param id Identifier to compare against each task's group identifier.
 */
void TaskManager::CancelTaskById(u32 id) { RemoveTaskById(id); }

/**
 * @brief Removes and signals all queued tasks with the requested identifier.
 * @param id Identifier to compare against each task's group identifier.
 */
void TaskManager::RemoveTaskById(u32 id) {
    mMutex.Lock();
    for (auto& rList : mTasks) {
        auto it = rList.begin();
        while (it != rList.end()) {
            auto current = it++;
            if (current->mId == id) {
                Task* pTask = &*current;
                rList.erase(current);
                pTask->Complete(4);
            }
        }
    }
    mMutex.Unlock();
}

/** @brief Preserves the original no-op implementation of bulk cancellation. */
void TaskManager::CancelAllTask() {}

/** @brief Waits until a task is available, a zero wake-up message arrives, or waiting is cancelled. */
void TaskManager::WaitTask() {
    mWaitCancelled = false;
    while (PeekTask() == nullptr && !mWaitCancelled) {
        u64 message;
        os::ReceiveMessageQueue(&message, &mWakeQueue.queue);
        if (message == 0) {
            break;
        }
    }
}

/** @brief Cancels the current task wait and sends a blocking wake-up notification. */
void TaskManager::CancelWaitTask() {
    mWaitCancelled = true;
    os::SendMessageQueue(&mWakeQueue.queue, 0);
}
} // namespace nn::atk::detail
