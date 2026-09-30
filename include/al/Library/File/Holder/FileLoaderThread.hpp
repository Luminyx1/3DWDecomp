#pragma once

#include <basis/seadTypes.h>
#include <thread/seadMessageQueue.h>

namespace sead {
class DelegateThread;
class Thread;
}  // namespace sead

namespace al {
class FileEntryBase;

class FileLoaderThread {
public:
    FileLoaderThread(s32 priority);

    void threadFunction(sead::Thread* pThread, sead::MessageQueue::Element message);
    void requestLoadFile(FileEntryBase* pEntry);

    sead::DelegateThread* getThread() const { return mThread; }

    sead::DelegateThread* mThread = nullptr;
};
}  // namespace al
