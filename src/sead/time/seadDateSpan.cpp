#include "time/seadDateSpan.h"

#include "time/seadDateUtil.h"

namespace sead
{
/**
 * Creates a span from a number of seconds.
 * @param span span in seconds
 */
DateSpan::DateSpan(s64 span) : mSpan(span) {}

/**
 * Creates a span from a calendar span.
 * @param rSpan calendar span
 */
DateSpan::DateSpan(const CalendarSpan& rSpan)
{
    set(rSpan);
}

/**
 * Sets the span from days, hours, minutes and seconds.
 * @param d days
 * @param h hours
 * @param m minutes
 * @param s seconds
 * @return span in seconds
 */
s64 DateSpan::setTimeImpl_(s32 d, s32 h, s32 m, s32 s)
{
    mSpan = 86400ll * d + 3600ll * h + 60ll * m + s;
    return mSpan;
}

/**
 * Creates a span from days, hours, minutes and seconds.
 * @param rDay days
 * @param rHour hours
 * @param rMinute minutes
 * @param rSecond seconds
 */
DateSpan::DateSpan(const CalendarSpan::Day& rDay, const CalendarSpan::Hour& rHour,
                   const CalendarSpan::Minute& rMinute, const CalendarSpan::Second& rSecond)
{
    set(rDay, rHour, rMinute, rSecond);
}

/**
 * Converts the span to a calendar span.
 * @param pSpan destination calendar span
 */
void DateSpan::getCalendarSpan(CalendarSpan* pSpan) const
{
    DateUtil::calcSecondToCalendarSpan(pSpan, mSpan);
}

/**
 * Sets the span from a calendar span.
 * @param rSpan calendar span
 * @return span in seconds
 */
s64 DateSpan::set(const CalendarSpan& rSpan)
{
    return setTimeImpl_(rSpan.getDays(), rSpan.getHours(), rSpan.getMinutes(), rSpan.getSeconds());
}

/**
 * Sets the span from days, hours, minutes and seconds.
 * @param rDay days
 * @param rHour hours
 * @param rMinute minutes
 * @param rSecond seconds
 * @return span in seconds
 */
s64 DateSpan::set(const CalendarSpan::Day& rDay, const CalendarSpan::Hour& rHour,
                  const CalendarSpan::Minute& rMinute, const CalendarSpan::Second& rSecond)
{
    return setTimeImpl_(rDay.getValue(), rHour.getValue(), rMinute.getValue(), rSecond.getValue());
}
}  // namespace sead
