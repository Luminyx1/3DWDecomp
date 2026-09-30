#include <prim/seadCuckooClock.h>

#include <math/seadMathCalcCommon.h>
#include <time/seadDateTime.h>

namespace sead
{
SEAD_TASK_SINGLETON_IMPL(CuckooClock)

/**
 * Creates the clock task.
 * @param rArg task construction arguments
 */
CuckooClock::CuckooClock(const TaskConstructArg& rArg)
    : CalculateTask(rArg, "sead::CuckooClock"), mUpdateInterval(0)
{
}

/**
 * Destroys the clock task.
 */
CuckooClock::~CuckooClock() = default;

/**
 * Reads the current date and prints the first time stamp.
 */
void CuckooClock::initialize()
{
    updateLatest_();
    cuckoo_();
}

/**
 * Stores the current tick and calendar time.
 */
void CuckooClock::updateLatest_()
{
    mUpdateTime.setNow();
    DateTime dateTime(0);
    dateTime.setNow();
    dateTime.getCalendarTime(&mCalendarTime);
}

// NON_MATCHING: ~80%; see getUtcString_
void CuckooClock::cuckoo_()
{
    FixedSafeString<16> timeString;
    getTimeString(&timeString);
    FixedSafeString<16> utcString;
    getUtcString_(&utcString);
    mCuckooTime.setNow();
}

/**
 * Refreshes the calendar time and cuckoos every full hour (or after an hour at most).
 */
void CuckooClock::calc()
{
    if (mUpdateInterval.toS64() > 0 &&
        TickTime().diff(mUpdateTime).toS64() >= mUpdateInterval.toS64())
    {
        updateLatest_();
    }

    s32 hour;
    s32 minute;
    s32 second;
    s32 milliSecond;
    calcTime_(&hour, &minute, &second, &milliSecond);

    const TickSpan sinceCuckoo = TickTime().diff(mCuckooTime);

    if (sinceCuckoo.toS64() >= TickSpan::makeFromSeconds(1).toS64() && second == 0 && minute == 0)
    {
        updateLatest_();
        cuckoo_();
    }
    else if (sinceCuckoo.toS64() >= TickSpan::makeFromSeconds(3600).toS64())
    {
        updateLatest_();
        cuckoo_();
    }
}

// NON_MATCHING: ~89%; the tick span and the elapsed seconds use swapped registers
void CuckooClock::calcTime_(s32* pHour, s32* pMinute, s32* pSecond, s32* pMilliSecond) const
{
    const TickSpan span = TickTime().diff(mUpdateTime);
    *pSecond = mCalendarTime.getSecond() + span.toS64() / TickSpan::cFrequency;
    *pMinute = *pSecond / 60 + mCalendarTime.getMinute();
    *pHour = *pMinute / 60 + mCalendarTime.getHour();
    *pMilliSecond = span.toMilliSeconds() % 1000;
    *pSecond %= 60;
    *pMinute %= 60;
}

// NON_MATCHING: ~99.5%; register allocation (calcTime_ inlined)
s32 CuckooClock::getTimeString(BufferedSafeString* pStr) const
{
    s32 hour;
    s32 minute;
    s32 second;
    s32 milliSecond;
    calcTime_(&hour, &minute, &second, &milliSecond);
    return pStr->format("%02d:%02d:%02d.%03d", hour, minute, second, milliSecond);
}

// NON_MATCHING: ~58%; the two float divisions are scheduled after the first rounding
bool CuckooClock::getUtcString_(BufferedSafeString* pStr) const
{
    const DateTime local(mCalendarTime);
    const DateTimeUtc utc(local);
    const s64 diff = local.getUnixTime() - utc.getUnixTime();
    const f32 minutes = diff / 60.0f;
    const f32 hours = diff / 3600.0f;
    const s32 minute = minutes + (minutes >= 0.0f ? 0.5f : -0.5f);
    const s32 hour = hours + (hours >= 0.0f ? 0.5f : -0.5f);
    pStr->format("UTC %c%02d:%02d", diff >= 0 ? '+' : '-', hour, minute % 60);
    return true;
}
}  // namespace sead
