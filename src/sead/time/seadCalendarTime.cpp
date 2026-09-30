#include <basis/seadRawPrint.h>
#include <container/seadSafeArray.h>
#include <time/seadCalendarTime.h>
#include <time/seadDateUtil.h>

namespace sead
{
const CalendarTime::Month CalendarTime::cMonth_Jan = 1;
const CalendarTime::Month CalendarTime::cMonth_Feb = 2;
const CalendarTime::Month CalendarTime::cMonth_Mar = 3;
const CalendarTime::Month CalendarTime::cMonth_Apr = 4;
const CalendarTime::Month CalendarTime::cMonth_May = 5;
const CalendarTime::Month CalendarTime::cMonth_Jun = 6;
const CalendarTime::Month CalendarTime::cMonth_Jul = 7;
const CalendarTime::Month CalendarTime::cMonth_Aug = 8;
const CalendarTime::Month CalendarTime::cMonth_Sep = 9;
const CalendarTime::Month CalendarTime::cMonth_Oct = 10;
const CalendarTime::Month CalendarTime::cMonth_Nov = 11;
const CalendarTime::Month CalendarTime::cMonth_Dec = 12;

const CalendarTime::Year CalendarTime::cDefaultYear = 1970;
const CalendarTime::Month CalendarTime::cDefaultMonth = CalendarTime::cMonth_Jan;
const CalendarTime::Day CalendarTime::cDefaultDay = 1;
const CalendarTime::Hour CalendarTime::cDefaultHour = 0;
const CalendarTime::Minute CalendarTime::cDefaultMinute = 0;
const CalendarTime::Second CalendarTime::cDefaultSecond = 0;

/**
 * Sets the year.
 * @param year the year
 */
void CalendarTime::Year::setValue(u32 year)
{
    mValue = year;
}

/**
 * Constructs a month from a one-based value.
 * @param month the month, from 1 to 12
 */
CalendarTime::Month::Month(u32 month)
{
    setValueOneOrigin(month);
}

/**
 * Sets the month from a one-based value.
 * @param m the month, from 1 to 12
 */
void CalendarTime::Month::setValueOneOrigin(u32 m)
{
    SEAD_ASSERT_MSG(1 <= m && m <= 12, "wrong month. correct range is [1, 12]. your param %d", m);
    mValue = m;
}

/**
 * Adds months, wrapping around the year.
 * @param rhs the number of months to add
 * @return the number of years carried
 */
s32 CalendarTime::Month::addSelf(u32 rhs)
{
    const s32 sum = s32(rhs) + mValue - 1;
    mValue = sum % 12 + 1;
    return sum / 12;
}

/**
 * Subtracts months, wrapping around the year.
 * @param rhs the number of months to subtract
 * @return the year carry
 */
s32 CalendarTime::Month::subSelf(u32 rhs)
{
    const s32 carry = (mValue - s32(rhs) - 13) / 12;
    mValue = (mValue - rhs % 12 + 11) % 12 + 1;
    return carry;
}

/**
 * Computes the difference between this month and another.
 * @param rhs the month to subtract
 * @return the difference in months
 */
s32 CalendarTime::Month::sub(CalendarTime::Month rhs) const
{
    return s32(mValue) - rhs.getValueOneOrigin();
}

/**
 * Gets the abbreviated English name of a month.
 * @param m the month, from 1 to 12
 * @return the month name
 */
SafeString CalendarTime::Month::makeStringOneOrigin(u32 m)
{
    SEAD_ASSERT_MSG(1 <= m && m <= 12, "wrong month. correct range is [1, 12]. your param %d", m);
    switch (m)
    {
    case 1:
        return "Jan";
    case 2:
        return "Feb";
    case 3:
        return "Mar";
    case 4:
        return "Apr";
    case 5:
        return "May";
    case 6:
        return "Jun";
    case 7:
        return "Jul";
    case 8:
        return "Aug";
    case 9:
        return "Sep";
    case 10:
        return "Oct";
    case 11:
        return "Nov";
    case 12:
    default:
        return "Dec";
    }
}

/**
 * Makes a month from a one-based value.
 * @param m the month, from 1 to 12
 * @return the month
 */
CalendarTime::Month CalendarTime::Month::makeFromValueOneOrigin(u32 m)
{
    SEAD_ASSERT(1 <= m && m <= 12);
    return Month(m);
}

/**
 * Sets the day of the month.
 * @param day the day, from 1 to 31
 */
void CalendarTime::Day::setValue(u32 day)
{
    SEAD_ASSERT_MSG(1 <= day && day <= 31, "wrong day. correct range is [1, 31]. your param %d",
                    day);
    mValue = day;
}

/**
 * Sets the hour.
 * @param hour the hour, from 0 to 23
 */
void CalendarTime::Hour::setValue(u32 hour)
{
    SEAD_ASSERT_MSG(hour <= 23, "wrong hour. correct range is [0, 23]. your param %d", hour);
    mValue = hour;
}

/**
 * Sets the minute.
 * @param minute the minute, from 0 to 59
 */
void CalendarTime::Minute::setValue(u32 minute)
{
    SEAD_ASSERT_MSG(minute <= 59, "wrong minute. correct range is [0, 59]. your param %d", minute);
    mValue = minute;
}

/**
 * Sets the second.
 * @param second the second, from 0 to 59
 */
void CalendarTime::Second::setValue(u32 second)
{
    SEAD_ASSERT_MSG(second <= 59, "wrong day. correct range is [0, 59]. your param %d", second);
    mValue = second;
}

/**
 * Constructs a date and computes its weekday.
 * @param rY the year
 * @param rM the month
 * @param rD the day
 */
CalendarTime::Date::Date(const CalendarTime::Year& rY, const CalendarTime::Month& rM,
                         const CalendarTime::Day& rD)
    : mYear(rY), mMonth(rM), mDay(rD)
{
    mWeek = DateUtil::calcWeekDay(rY, rM, rD);
}

/**
 * Constructs a time of day.
 * @param rH the hour
 * @param rM the minute
 * @param rS the second
 */
CalendarTime::Time::Time(const CalendarTime::Hour& rH, const CalendarTime::Minute& rM,
                         const CalendarTime::Second& rS)
    : mHour(rH), mMinute(rM), mSecond(rS)
{
}

/**
 * Constructs a calendar time from a date and a time, recomputing the weekday.
 * @param rDate the date
 * @param rTime the time
 */
CalendarTime::CalendarTime(const CalendarTime::Date& rDate, const CalendarTime::Time& rTime)
    : mDate(rDate), mTime(rTime)
{
    mDate.calcWeek();
}

/**
 * Constructs a calendar time from individual components.
 * @param rY the year
 * @param rM the month
 * @param rD the day
 * @param rHour the hour
 * @param rMinute the minute
 * @param rSecond the second
 */
CalendarTime::CalendarTime(const CalendarTime::Year& rY, const CalendarTime::Month& rM,
                           const CalendarTime::Day& rD, const CalendarTime::Hour& rHour,
                           const CalendarTime::Minute& rMinute, const CalendarTime::Second& rSecond)
    : mDate(rY, rM, rD), mTime(rHour, rMinute, rSecond)
{
}

/**
 * Sets the date and recomputes its weekday.
 * @param rDate the date
 */
void CalendarTime::setDate(const CalendarTime::Date& rDate)
{
    mDate = rDate;
    mDate.calcWeek();
}

/**
 * Computes the one-based day of the year, accounting for leap years.
 * @return the day of the year
 */
u32 CalendarTime::getYearDays() const
{
    const u32 m = mDate.mMonth.getValueOneOrigin();
    SEAD_ASSERT_MSG(1 <= m && m <= 12, "wrong month. correct range is [1, 12]. your param %d", m);

    static const u32 sCumulativeNumberOfDays[] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334,
    };

    u32 num_days = mDate.mDay.getValue() + sCumulativeNumberOfDays[m - 1];

    if (m >= 3)
    {
        num_days += DateUtil::isLeapYear(mDate.mYear.getValue());
    }

    return num_days;
}

/**
 * Recomputes the weekday from the year, month and day.
 */
void CalendarTime::Date::calcWeek()
{
    mWeek = DateUtil::calcWeekDay(mYear, mMonth, mDay);
}

/**
 * Formats a Japanese weekday label.
 * @param pOutStr receives the label
 * @param week the weekday
 */
void CalendarTime::makeWeekDayNameLabel_(BufferedSafeString* pOutStr, CalendarTime::Week week)
{
    static const SafeArray<const char*, 7> labels = {{"日", "月", "火", "水", "木", "金", "土"}};
    pOutStr->format("曜日:%s", labels[s32(week)]);
}

}  // namespace sead
