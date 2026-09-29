#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

namespace sead {
class DelegateThread;
class Heap;
class Thread;
}  // namespace sead

namespace al {
class FunctorBase;
class InitializeThread;

class AsyncFunctorThread {
public:
    AsyncFunctorThread(const sead::SafeString&, const FunctorBase&, s32, s32);
    virtual ~AsyncFunctorThread();

    void threadFunction(sead::Thread*, s64);
    void start();
    bool isDone() const;

    sead::DelegateThread* mThread = nullptr;  // _8
    FunctorBase* mFunctor = nullptr;          // _10
    bool mIsDone = true;                      // _18
};

InitializeThread* createAndStartInitializeThread(sead::Heap*, s32, const FunctorBase&);
bool tryWaitDoneAndDestroyInitializeThread(InitializeThread*);
}  // namespace al
