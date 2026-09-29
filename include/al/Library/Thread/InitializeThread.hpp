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

class InitializeThread {
public:
    InitializeThread(const sead::SafeString&, const FunctorBase&, sead::Heap*, s32, s32);

    void threadFunction(sead::Thread*, s64);
    void start();
    bool tryWaitDoneAndDestroy();

    sead::DelegateThread* mThread = nullptr;  // _0
    sead::Heap* mHeap;                        // _8
    FunctorBase* mFunctor = nullptr;          // _10
    bool mIsDone = false;                     // _18
};
}  // namespace al
