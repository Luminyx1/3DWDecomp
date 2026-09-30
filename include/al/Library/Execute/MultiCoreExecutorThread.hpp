#pragma once

#include <mc/seadCoreInfo.h>
#include <thread/seadMessageQueue.h>

namespace sead {
class DelegateThread;
class Thread;
}  // namespace sead

namespace al {
class MultiCoreExecutorThreadBase {
public:
    MultiCoreExecutorThreadBase(s32 queueSize, s32 priority, sead::CoreId core);
    ~MultiCoreExecutorThreadBase();

    virtual void executeOnThread(s64 message) = 0;

    void threadFunction(sead::Thread* pThread, s64 message);
    void waitDoneAll();

    sead::DelegateThread* mThread = nullptr;
    sead::MessageQueue mMessageQueue;
};

class ExecutorListBase;

class MultiCoreExecutorThreadExecutorList : public MultiCoreExecutorThreadBase {
public:
    void executeOnThread(s64 message) override;
};

class MultiCoreExecutorThreadActorCalcAnim : public MultiCoreExecutorThreadBase {
public:
    MultiCoreExecutorThreadActorCalcAnim(s32 queueSize, s32 priority, sead::CoreId core)
        : MultiCoreExecutorThreadBase(queueSize, priority, core) {}

    void executeOnThread(s64 message) override;
};
}  // namespace al
