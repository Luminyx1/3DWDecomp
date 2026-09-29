#pragma once

#include "basis/seadTypes.h"
#include "framework/seadCalculateTask.h"
#include "framework/seadTaskMgr.h"
#include "thread/seadAtomic.h"

namespace sead
{
class InfLoopChecker : public CalculateTask
{
    SEAD_TASK_SINGLETON(InfLoopChecker)
    SEAD_RTTI_OVERRIDE(InfLoopChecker, CalculateTask)

public:
    struct InfLoopParam
    {
    };

    using InfLoopEvent = DelegateEvent<const InfLoopParam&>;

    explicit InfLoopChecker(const TaskConstructArg& arg);
    ~InfLoopChecker() override;

    void countUp();
    void prepare() override;
    void calc() override;

    InfLoopEvent& getEvent() { return mEvent; }
    bool isEnabled() const { return mEnabled; }
    void setEnabled(bool enabled) { mEnabled = enabled; }

private:
    void onInfLoop_();

    u32 mLoopCount = 0;
    u32 mLoopThreshold = 600;
    bool mEnabled = true;
    InfLoopEvent mEvent;
    Atomic<u32> mSkipCounter = 0;
};
}  // namespace sead
