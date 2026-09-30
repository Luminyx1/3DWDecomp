#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>
#include <thread/seadMessageQueue.h>

#include "Library/Thread/Functor.hpp"

namespace sead {
class DelegateThread;
class Heap;
class Thread;
}  // namespace sead

namespace al {
class InitializeThread;

class AsyncFunctorThread {
public:
    AsyncFunctorThread(const sead::SafeString& rName, const FunctorBase& rFunctor, s32 priority,
                       s32 stackSize);
    virtual ~AsyncFunctorThread();

    void threadFunction(sead::Thread* pThread, sead::MessageQueue::Element message);
    void start();
    bool isDone() const;

    sead::DelegateThread* mDelegateThread = nullptr;
    FunctorBase* mFunctor = nullptr;
    bool mIsDone = true;
};

static_assert(sizeof(AsyncFunctorThread) == 0x20);

InitializeThread* createAndStartInitializeThread(sead::Heap* pHeap, s32 priority,
                                                 const FunctorBase& rFunctor);
bool tryWaitDoneAndDestroyInitializeThread(InitializeThread* pThread);
}  // namespace al
