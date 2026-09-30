#include <nn/time.h>

#include "basis/seadRawPrint.h"
#include "time/seadDateTime.h"
#include "time/seadDateUtil.h"

namespace sead
{
namespace
{
constexpr u32 sDaysOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
constexpr u32 sDaysSinceJan1[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

u32 convertCalendarDateToDaysSince1970(const CalendarTime::Date& rDate)
{
    // 0-indexed day.
    const u32 d0 = rDate.mDay.getValue() - 1;

    const u32 m = rDate.mMonth.getValueOneOrigin();
    SEAD_ASSERT_MSG(1 <= m && m <= 12, "wrong month. correct range is [1, 12]. your param %d", m);

    const u32 y = rDate.mYear.getValue();
    SEAD_ASSERT_MSG(y >= 1970, "wrong year. your param %d, must after 1970.", y);

    const u32 days_since_jan1 = sDaysSinceJan1[m - 1];

    u32 num_days = d0 + days_since_jan1;

    if (m >= 3 && DateUtil::isLeapYear(rDate.mYear.getValue()))
    {
        num_days = d0 + days_since_jan1 + 1;
    }

    u32 num_days_since_1970 = num_days + 365 * (rDate.mYear.getValue() - 1970);

    if (rDate.mYear.getValue() > 1970)
    {
        u32 year = 1970;
        do
        {
            num_days_since_1970 += DateUtil::isLeapYear(year);
            ++year;
        } while (year < rDate.mYear.getValue());
    }

    return num_days_since_1970;
}

u64 convertCalendarTimeToSeconds(const CalendarTime::Time& rTime)
{
    return 60 * (60 * rTime.mHour.getValue() + rTime.mMinute.getValue()) + rTime.mSecond.getValue();
}

u64 convertCalendarDateTimeToSeconds(const CalendarTime::Date& rDate,
                                     const CalendarTime::Time& rTime)
{
#ifdef SEAD_DEBUG
    const u32 y = rDate.mYear.getValue();

    const s32 m = rDate.mMonth.getValueOneOrigin();
    const u32 m_idx = m - 1;
    SEAD_ASSERT_MSG(1 <= m && m <= 12, "wrong month. correct range is [1, 12]. your param %d", m);

    u32 num_days;

    if (m == 2 && DateUtil::isLeapYear(y))
    {
        num_days = sDaysOfMonth[m_idx] + 1;
    }
    else
    {
        num_days = sDaysOfMonth[m_idx];
    }

    const u32 d = rDate.mDay.getValue();
    SEAD_ASSERT_MSG(d <= num_days, "wrong day, correct range is [1, %d] (when year %4d month %2d)",
                    num_days, y, m);
#endif

    const u32 days_since_1970 = convertCalendarDateToDaysSince1970(rDate);
    return 3600 * 24 * days_since_1970 + convertCalendarTimeToSeconds(rTime);
}

u32 convertDaysToYears(u32* pDays)
{
    u32 days_to_remove;
    u32 i = 0;
    u32 year = 1969;
    do
    {
        ++year;
        days_to_remove = i;
        i += DateUtil::isLeapYear(year) ? 366 : 365;
    } while (i <= *pDays);
    *pDays -= days_to_remove;
    return year;
}

s32 convertDaysToMonth(u32* pDays, u32 year)
{
    SEAD_ASSERT_MSG(*pDays <= 365, "wrong pDays. correct range is [0, 365]. your param %d", *pDays);
    u32 days_to_remove;
    u32 month_idx = 0;
    u32 i = 0;
    do
    {
        days_to_remove = i;
        i += (month_idx == 1 && DateUtil::isLeapYear(year)) ? 29 : sDaysOfMonth[month_idx];

        if (*pDays < i)
        {
            break;
        }

        ++month_idx;
    } while (month_idx < 12);
    *pDays -= days_to_remove - 1;
    return 1 + month_idx;
}
}  // namespace

bool DateTime::mIsInitialized = false;

/**
 * Creates a date-time from a Unix timestamp.
 * @param unixTime seconds since the Unix epoch
 */
DateTime::DateTime(u64 unixTime)
{
    mUnixTime = unixTime;
}

/**
 * Creates a date-time from calendar components.
 * @param rYear year
 * @param rMonth month
 * @param rDay day
 * @param rHour hour
 * @param rMinute minute
 * @param rSecond second
 */
DateTime::DateTime(const CalendarTime::Year& rYear, const CalendarTime::Month& rMonth,
                   const CalendarTime::Day& rDay, const CalendarTime::Hour& rHour,
                   const CalendarTime::Minute& rMinute, const CalendarTime::Second& rSecond)
{
    setUnixTime(rYear, rMonth, rDay, rHour, rMinute, rSecond);
}

/**
 * Sets the time from calendar components.
 * @param rYear year
 * @param rMonth month
 * @param rDay day
 * @param rHour hour
 * @param rMinute minute
 * @param rSecond second
 * @return seconds since the Unix epoch
 */
u64 DateTime::setUnixTime(const CalendarTime::Year& rYear, const CalendarTime::Month& rMonth,
                          const CalendarTime::Day& rDay, const CalendarTime::Hour& rHour,
                          const CalendarTime::Minute& rMinute, const CalendarTime::Second& rSecond)
{
    CalendarTime::Date date(rYear, rMonth, rDay);
    CalendarTime::Time time(rHour, rMinute, rSecond);
    mUnixTime = convertCalendarDateTimeToSeconds(date, time);
    return mUnixTime;
}

/**
 * Creates a date-time from a calendar time.
 * @param rTime calendar time
 */
DateTime::DateTime(const CalendarTime& rTime)
{
    setUnixTime(rTime);
}

/**
 * Sets the time from a calendar time.
 * @param rTime calendar time
 * @return seconds since the Unix epoch
 */
u64 DateTime::setUnixTime(const CalendarTime& rTime)
{
    mUnixTime = convertCalendarDateTimeToSeconds(rTime.getDate(), rTime.getTime());
    return mUnixTime;
}

/**
 * Creates a local date-time from a UTC date-time (the argument is ignored and the uninitialized
 * member is converted instead).
 * @param rUnused UTC date-time
 */
DateTime::DateTime([[maybe_unused]] const DateTimeUtc& rUnused)
{
    initializeSystemTimeModule();

    nn::time::CalendarTime ctime;
    nn::time::PosixTime time = {mUnixTime};
    nn::time::ToCalendarTime(&ctime, nullptr, time);

    const auto year = CalendarTime::Year(ctime.year);
    const auto month = CalendarTime::Month::makeFromValueOneOrigin(ctime.month);
    const auto day = CalendarTime::Day(ctime.day);
    const auto hour = CalendarTime::Hour(ctime.hour);
    const auto minute = CalendarTime::Minute(ctime.minute);
    const auto second = CalendarTime::Second(ctime.second);
    setUnixTime(year, month, day, hour, minute, second);
}

/**
 * Initializes the nn::time module once.
 */
void DateTime::initializeSystemTimeModule()
{
    if (mIsInitialized)
    {
        return;
    }

    if (!nn::time::IsInitialized())
    {
        nn::time::Initialize();
    }

    mIsInitialized = true;
}

/**
 * Converts the time to a calendar time.
 * @param pCalendar destination calendar time (may be null)
 */
void DateTime::getCalendarTime(CalendarTime* pCalendar) const
{
    u32 d = mUnixTime / (3600 * 24);
    const u32 y = convertDaysToYears(&d);
    const u32 m = convertDaysToMonth(&d, y);

    CalendarTime::Time time;
    const auto reducedTime = mUnixTime % (3600 * 24);
    time.mHour.setValue(reducedTime / 3600);
    time.mMinute.setValue((reducedTime % 3600) / 60);
    time.mSecond.setValue(reducedTime % 60);

    if (pCalendar)
    {
        pCalendar->setDate(
            CalendarTime::Date(y, CalendarTime::Month::makeFromValueOneOrigin(m), d));
        pCalendar->setTime(time);
    }
}

/**
 * Sets the time to the current local time of the user system clock.
 * @return seconds since the Unix epoch
 */
u64 DateTime::setNow()
{
    initializeSystemTimeModule();

    nn::time::PosixTime now;
    nn::time::CalendarTime ctime;
    nn::time::StandardUserSystemClock::GetCurrentTime(&now);
    nn::time::ToCalendarTime(&ctime, nullptr, now);

    const auto year = CalendarTime::Year(ctime.year);
    const auto month = CalendarTime::Month::makeFromValueOneOrigin(ctime.month);
    const auto day = CalendarTime::Day(ctime.day);
    const auto hour = CalendarTime::Hour(ctime.hour);
    const auto minute = CalendarTime::Minute(ctime.minute);
    const auto second = CalendarTime::Second(ctime.second);
    setUnixTime(year, month, day, hour, minute, second);
    return mUnixTime;
}

/**
 * Calculates the span between this time and another time.
 * @param time time to subtract
 * @return difference in seconds
 */
DateSpan DateTime::diff(DateTime time) const
{
    return DateSpan(mUnixTime - time.mUnixTime);
}

/**
 * Calculates the span from this time to now.
 * @return difference in seconds
 */
DateSpan DateTime::diffToNow() const
{
    DateTime now(0);
    now.setNow();
    return now.diff(*this);
}

/**
 * Calculates the span between two times.
 * @param lhs time
 * @param rhs time to subtract
 * @return difference in seconds
 */
DateSpan operator-(DateTime lhs, DateTime rhs)
{
    return DateSpan(lhs.getUnixTime() - rhs.getUnixTime());
}

/**
 * Subtracts a span from a time.
 * @param time time
 * @param span span to subtract
 * @return resulting time
 */
DateTime operator-(DateTime time, DateSpan span)
{
    return DateTime(time.getUnixTime() - span.getSpan());
}

/**
 * Adds a span to a time.
 * @param time time
 * @param span span to add
 * @return resulting time
 */
DateTime operator+(DateTime time, DateSpan span)
{
    return DateTime(time.getUnixTime() + span.getSpan());
}
}  // namespace sead
