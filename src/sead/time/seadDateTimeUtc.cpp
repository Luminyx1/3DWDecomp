#include <nn/time.h>

#include "time/seadDateTime.h"
#include "time/seadDateUtil.h"

namespace sead
{
/**
 * Creates a UTC date-time from a Unix timestamp.
 * @param unixTime seconds since the Unix epoch
 */
DateTimeUtc::DateTimeUtc(u64 unixTime)
{
    mUnixTime = unixTime;
}

/**
 * Creates a UTC date-time from calendar components.
 * @param rYear year
 * @param rMonth month
 * @param rDay day
 * @param rHour hour
 * @param rMinute minute
 * @param rSecond second
 */
DateTimeUtc::DateTimeUtc(const CalendarTime::Year& rYear, const CalendarTime::Month& rMonth,
                         const CalendarTime::Day& rDay, const CalendarTime::Hour& rHour,
                         const CalendarTime::Minute& rMinute, const CalendarTime::Second& rSecond)
{
    setUnixTime(rYear, rMonth, rDay, rHour, rMinute, rSecond);
}

/**
 * Sets the time from calendar components interpreted as UTC.
 * @param rYear year
 * @param rMonth month
 * @param rDay day
 * @param rHour hour
 * @param rMinute minute
 * @param rSecond second
 * @return seconds since the Unix epoch
 */
u64 DateTimeUtc::setUnixTime(const CalendarTime::Year& rYear, const CalendarTime::Month& rMonth,
                             const CalendarTime::Day& rDay, const CalendarTime::Hour& rHour,
                             const CalendarTime::Minute& rMinute,
                             const CalendarTime::Second& rSecond)
{
    DateTime::initializeSystemTimeModule();
    CalendarTime time(rYear, rMonth, rDay, rHour, rMinute, rSecond);

    nn::time::CalendarTime nnTime;
    nnTime.year = time.getYear();
    nnTime.month = time.getMonth().getValueOneOrigin();
    nnTime.day = time.getDay();
    nnTime.hour = time.getHour();
    nnTime.minute = time.getMinute();
    nnTime.second = time.getSecond();
    mUnixTime = nn::time::ToPosixTimeFromUtc(nnTime).time;
    return mUnixTime;
}

/**
 * Creates a UTC date-time from a calendar time.
 * @param rTime calendar time
 */
DateTimeUtc::DateTimeUtc(const CalendarTime& rTime)
{
    setUnixTime(rTime);
}

/**
 * Sets the time from a calendar time interpreted as UTC.
 * @param rTime calendar time
 * @return seconds since the Unix epoch
 */
u64 DateTimeUtc::setUnixTime(const CalendarTime& rTime)
{
    DateTime::initializeSystemTimeModule();

    nn::time::CalendarTime nnTime;
    nnTime.year = rTime.getYear();
    nnTime.month = rTime.getMonth().getValueOneOrigin();
    nnTime.day = rTime.getDay();
    nnTime.hour = rTime.getHour();
    nnTime.minute = rTime.getMinute();
    nnTime.second = rTime.getSecond();
    mUnixTime = nn::time::ToPosixTimeFromUtc(nnTime).time;
    return mUnixTime;
}

/**
 * Creates a UTC date-time from a local date-time.
 * @param rDateTime local date-time
 */
DateTimeUtc::DateTimeUtc(const DateTime& rDateTime)
{
    CalendarTime time;
    rDateTime.getCalendarTime(&time);

    nn::time::CalendarTime nnTime;
    nnTime.year = time.getYear();
    nnTime.month = time.getMonth().getValueOneOrigin();
    nnTime.day = time.getDay();
    nnTime.hour = time.getHour();
    nnTime.minute = time.getMinute();
    nnTime.second = time.getSecond();
    nn::time::PosixTime posixTime;
    int count = 0;
    nn::time::ToPosixTime(&count, &posixTime, 1, nnTime);
    mUnixTime = posixTime.time;
}

/**
 * Converts the time to a UTC calendar time.
 * @param pTime destination calendar time
 */
void DateTimeUtc::getCalendarTime(CalendarTime* pTime) const
{
    DateTime::initializeSystemTimeModule();
    const nn::time::CalendarTime nnTime = nn::time::ToCalendarTimeInUtc({mUnixTime});

    CalendarTime::Date date(nnTime.year, CalendarTime::Month::makeFromValueOneOrigin(nnTime.month),
                            nnTime.day);
    CalendarTime::Time time(nnTime.hour, nnTime.minute, nnTime.second);

    pTime->setDate(date);
    pTime->setTime(time);
}

/**
 * Sets the time to the current time of the user system clock.
 * @return seconds since the Unix epoch
 */
u64 DateTimeUtc::setNow()
{
    DateTime::initializeSystemTimeModule();
    nn::time::PosixTime now;
    nn::time::StandardUserSystemClock::GetCurrentTime(&now);
    mUnixTime = now.time;
    return mUnixTime;
}

/**
 * Calculates the span between this time and another time.
 * @param time time to subtract
 * @return difference in seconds
 */
DateSpan DateTimeUtc::diff(DateTimeUtc time) const
{
    return DateSpan(mUnixTime - time.mUnixTime);
}

/**
 * Calculates the span from this time to now.
 * @return difference in seconds
 */
DateSpan DateTimeUtc::diffToNow() const
{
    DateTimeUtc now(0);
    now.setNow();
    return now.diff(*this);
}

/**
 * Calculates the span between two times.
 * @param lhs time
 * @param rhs time to subtract
 * @return difference in seconds
 */
DateSpan operator-(DateTimeUtc lhs, DateTimeUtc rhs)
{
    return DateSpan(lhs.getUnixTime() - rhs.getUnixTime());
}

/**
 * Subtracts a span from a time.
 * @param time time
 * @param span span to subtract
 * @return resulting time
 */
DateTimeUtc operator-(DateTimeUtc time, DateSpan span)
{
    return DateTimeUtc(time.getUnixTime() - span.getSpan());
}

/**
 * Adds a span to a time.
 * @param time time
 * @param span span to add
 * @return resulting time
 */
DateTimeUtc operator+(DateTimeUtc time, DateSpan span)
{
    return DateTimeUtc(time.getUnixTime() + span.getSpan());
}
}  // namespace sead
