#pragma once
#include <nn/os.h>
#include <nn/util/util_IntrusiveList.h>

namespace nn::atk {
class TaskProfileLogger;
namespace detail {
class Task {
public:
    Task();
    virtual ~Task();
    virtual void Execute(TaskProfileLogger& logger) = 0;
    /**
     * @brief Marks the task terminal and wakes callers waiting for it.
     * @param state Completion state: 3 for executed tasks or 4 for cancelled tasks.
     */
    void Complete(u32 state) {
        mState = state;
        nn::os::SignalEvent(&mCompletionEvent);
    }
    nn::util::IntrusiveListNode mNode;
    nn::os::EventType mCompletionEvent;
    u32 mState;
    u32 mId;
};
static_assert(sizeof(Task) == 0x48, "Task size");
}
}
