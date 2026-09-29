#pragma once

#include "thread/seadAtomic.h"
#include "thread/seadThread.h"

namespace sead {
class IAudioTaskNin {
public:
    virtual void execute(bool isQuitting) = 0;

private:
    friend class AudioTaskThreadNin;

    Atomic<s32> mPendingCount;
};

class IAudioTaskListenerNin {
public:
    virtual ~IAudioTaskListenerNin() {}
    virtual void onTaskBegin() = 0;
    virtual void onTaskEnd() = 0;
};

class AudioTaskThreadNin : public Thread {
public:
    AudioTaskThreadNin(s32 priority, Heap* pHeap, const SafeString& rName, s32 stackSize,
                       s32 messageQueueSize);
    ~AudioTaskThreadNin() override;

    bool start() override;
    bool addTask(IAudioTaskNin* pTask);

protected:
    void calc_(MessageQueue::Element msg) override;

private:
    IAudioTaskListenerNin* mListener = nullptr;
};
}  // namespace sead
