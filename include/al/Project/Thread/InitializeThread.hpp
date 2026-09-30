#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>
#include <thread/seadMessageQueue.h>

namespace sead {
class Heap;
class DelegateThread;
class Thread;
}  // namespace sead

namespace al {
class FunctorBase;

class InitializeThread {
public:
    InitializeThread(const sead::SafeString& rName, const FunctorBase& rFunctor, sead::Heap* pHeap,
                     s32 priority, s32 stackSize);
    void threadFunction(sead::Thread* pThread, sead::MessageQueue::Element message);
    void start();
    bool tryWaitDoneAndDestroy();

    sead::DelegateThread* mThread = nullptr;
    sead::Heap* mHeap = nullptr;
    FunctorBase* mFunctor = nullptr;
    bool mIsDone = false;
};
}  // namespace al
