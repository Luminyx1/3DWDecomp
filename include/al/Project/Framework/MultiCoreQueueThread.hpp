#pragma once

#include <basis/seadTypes.h>

namespace sead {
    class Thread;
};

namespace al {
    /// A job that can be executed on a MultiCoreQueueThread.
    class MultiCoreQueueExecutor {
    public:
        virtual const char* executorName() const = 0;
        virtual void executeOnThread() = 0;
    };

    /// A worker thread that executes queued jobs.
    class MultiCoreQueueThread {
    public:
        MultiCoreQueueThread(s32 queueSize);
        ~MultiCoreQueueThread();

        void threadFunc_(sead::Thread* pThread, s64 message);
        void requestExecute(MultiCoreQueueExecutor* pExecutor);
        bool isQueued(MultiCoreQueueExecutor* pExecutor);
        void waitDone();
    };
};
