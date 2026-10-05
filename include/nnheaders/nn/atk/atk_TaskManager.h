#pragma once
#include <attributes.h>

#include <nn/atk/atk_Task.h>
#include <nn/atk/atk_TaskProfileReader.h>

namespace nn::atk::detail {
class TaskManager {
  public:
    enum TaskPriority { TaskPriority_Low, TaskPriority_Normal, TaskPriority_High, TaskPriority_Count };
    TaskManager();
    /** @brief Releases queue, profiling, and synchronization resources. */
    ~TaskManager() = default;
    static TaskManager& GetInstance();
    void Initialize(bool enableProfiling);
    void Finalize();
    void AppendTask(Task* pTask, TaskPriority priority);
    Task* GetNextTask(TaskPriority priority, bool remove);
    Task* PopTask();
    Task* PeekTask();
    void WaitTask();
    void CancelWaitTask();
    void ExecuteTask();
    void CancelTask(Task* pTask);
    bool TryRemoveTask(Task* pTask);
    void CancelTaskById(u32 id);
    void RemoveTaskById(u32 id);
    void CancelAllTask();

  private:
    using TaskList = util::IntrusiveList<Task, util::IntrusiveListMemberNodeTraits<Task, &Task::mNode>>;
    struct WakeQueue {
        /**
         * @brief Initializes the worker wake-up queue over caller-owned storage.
         * @param pBuffer Message slots that must remain valid for the queue lifetime.
         * @param capacity Number of message slots; the manager supplies 32.
         */
        WakeQueue(u64* pBuffer, size_t capacity) { os::InitializeMessageQueue(&queue, pBuffer, capacity); }
        /** @brief Releases the operating-system message queue. */
        ~WakeQueue() { os::FinalizeMessageQueue(&queue); }
        os::MessageQueueType queue;
    };
    /**
     * @brief Selects the first task from the highest nonempty priority queue.
     * @param remove Whether to unlink the selected task.
     * @return Selected task, or nullptr when all queues are empty.
     */
    ALWAYS_INLINE Task* SelectTask(bool remove) {
        Task* pTask = GetNextTask(TaskPriority_High, remove);
        if (pTask == nullptr) {
            pTask = GetNextTask(TaskPriority_Normal, remove);
        }
        if (pTask == nullptr) {
            pTask = GetNextTask(TaskPriority_Low, remove);
        }
        return pTask;
    }
    TaskList mTasks[TaskPriority_Count];
    volatile bool mWaitCancelled;
    os::Mutex mMutex;
    WakeQueue mWakeQueue;
    u64 mWakeMessages[32];
    TaskProfileLogger mProfileLogger;
};
static_assert(sizeof(TaskManager) == 0x1d8, "TaskManager size");
} // namespace nn::atk::detail
