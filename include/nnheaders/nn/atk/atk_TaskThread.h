#pragma once

#include <nn/atk/atkfnd_Thread.h>
#include <nn/os/os_Mutex.h>

namespace nn::atk {
enum FsPriority : int;
namespace detail {
class TaskThread : public fnd::Thread::Handler {
  public:
    TaskThread();
    ~TaskThread() override;
    static TaskThread& GetInstance();
    bool Create(int priority, void* pStack, size_t stackSize, int core, u32 affinityMask,
                FsPriority fsPriority);
    void Destroy();
    u32 Run(void* pArg) override;

  private:
    fnd::Thread mThread;
    nn::os::Mutex mMutex;
    volatile bool mStopRequested;
    bool mCreated;
    FsPriority mFsPriority;
};
static_assert(sizeof(TaskThread) == 0x220, "TaskThread size");
} // namespace detail
} // namespace nn::atk
