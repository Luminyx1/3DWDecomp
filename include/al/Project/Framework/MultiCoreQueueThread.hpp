#pragma once

#include <basis/seadTypes.h>
#include <thread/seadMessageQueue.h>

namespace sead {
class DelegateThread;
class Thread;
}  // namespace sead

namespace al {
class MultiCoreQueueExecutor {
public:
    virtual const char* executorName() const = 0;
    virtual void executeOnThread() = 0;
};

class MultiCoreQueueThread {
public:
    MultiCoreQueueThread(s32 queueSize);
    ~MultiCoreQueueThread();

    void threadFunc_(sead::Thread* pThread, sead::MessageQueue::Element msg);
    void requestExecute(MultiCoreQueueExecutor* pExecutor);
    bool isQueued(MultiCoreQueueExecutor* pExecutor);
    void waitDone();

    sead::DelegateThread* mThread;
    sead::MessageQueue mDoneQueue;
    MultiCoreQueueExecutor** mExecutors;
    s32 mMaxExecutors;
    s32 mExecutorNum;
};

static_assert(sizeof(MultiCoreQueueThread) == 0x68);
}  // namespace al
