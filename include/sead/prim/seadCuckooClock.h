#pragma once

#include <framework/seadCalculateTask.h>
#include <framework/seadTaskMgr.h>
#include <prim/seadSafeString.h>
#include <time/seadCalendarTime.h>
#include <time/seadTickSpan.h>
#include <time/seadTickTime.h>

namespace sead
{
class CuckooClock : public CalculateTask
{
    SEAD_TASK_SINGLETON(CuckooClock)
    SEAD_RTTI_OVERRIDE(CuckooClock, CalculateTask)

public:
    explicit CuckooClock(const TaskConstructArg& rArg);
    ~CuckooClock() override;

    void initialize();
    void calc() override;
    s32 getTimeString(BufferedSafeString* pStr) const;

private:
    void updateLatest_();
    void cuckoo_();
    void calcTime_(s32* pHour, s32* pMinute, s32* pSecond, s32* pMilliSecond) const;
    bool getUtcString_(BufferedSafeString* pStr) const;

    CalendarTime mCalendarTime;
    TickTime mUpdateTime;
    TickTime mCuckooTime;
    TickSpan mUpdateInterval;
};
}  // namespace sead
