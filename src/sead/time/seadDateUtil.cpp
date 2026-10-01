#include <prim/seadStringUtil.h>
#include <time/seadCalendarSpan.h>
#include <time/seadDateUtil.h>

namespace sead
{
namespace DateUtil
{
/**
 * Checks whether a year is a Gregorian leap year.
 * @param year year
 * @return true if the year has 366 days
 */
bool isLeapYear(u32 year)
{
    const bool div4 = year % 4 == 0;
    const bool notDiv100 = year % 100 != 0;
    return (notDiv100 & div4) | (year % 400 == 0);
}

/**
 * Calculates the day of the week of a date (Zeller's congruence).
 * @param rYear year
 * @param rMonth month
 * @param rDay day
 * @return day of the week
 */
CalendarTime::Week calcWeekDay(const CalendarTime::Year& rYear, const CalendarTime::Month& rMonth,
                               const CalendarTime::Day& rDay)
{
    int y = rYear.getValue();
    int m = rMonth.getValueOneOrigin();
    int d = rDay.getValue();

    if (m < 3)
    {
        y -= 1;
        m += 12;
    }

    d += y + (y / 4) - (y / 100) + (y / 400);
    d += (26 * m + 16) / 10;
    return CalendarTime::Week(d % 7);
}

/**
 * Splits a number of seconds into days, hours, minutes and seconds.
 * @param pOutSpan destination calendar span (may be null)
 * @param sec number of seconds
 */
void calcSecondToCalendarSpan(CalendarSpan* pOutSpan, u64 sec)
{
    if (pOutSpan == nullptr)
    {
        return;
    }

    pOutSpan->setDays(sec / (3600 * 24));
    pOutSpan->setHours((sec % (3600 * 24)) / 3600);
    pOutSpan->setMinutes((sec % 3600) / 60);
    pOutSpan->setSeconds(sec % 60);
}

/**
 * Parses one fixed-width number field of a W3C-DTF string and advances past its separator.
 * @param pOk set to whether the field was valid
 * @param pValue parsed value
 * @param pStr remaining string, advanced past the field and separator
 * @param pStrLength remaining string length, reduced accordingly
 * @param pOutSeparator separator that followed the field (may be null)
 * @param parseLength field width in characters
 * @param rSeparators allowed separator characters
 * @param allowNullSeparator whether the field may end the string
 * @param valueMin minimum valid value
 * @param valueMax maximum valid value
 * @return true if parsing stops here (error or end of string)
 */
static bool parseW3CDTFSubString(bool* pOk, u32* pValue, SafeString* pStr, s32* pStrLength,
                                 char* pOutSeparator, s32 parseLength,
                                 const SafeString& rSeparators, bool allowNullSeparator,
                                 u32 valueMin, u32 valueMax)
{
    if (*pStrLength < parseLength)
    {
        *pOk = false;
        return true;
    }

    const char c = pStr->at(parseLength);

    if (!rSeparators.include(c) && (!allowNullSeparator || c != SafeString::cNullChar))
    {
        *pOk = false;
        return true;
    }

    FixedSafeString<8> buffer;
    buffer.copy(*pStr, parseLength);

    if (!StringUtil::tryParseNumber(pValue, buffer, StringUtil::CardinalNumber::Base10) ||
        *pValue < valueMin || *pValue > valueMax)
    {
        *pOk = false;
        return true;
    }

    if (c == SafeString::cNullChar)
    {
        *pOk = true;
        return true;
    }

    *pStr = pStr->getPart(parseLength + 1);
    *pStrLength -= parseLength + 1;
    if (pOutSeparator != nullptr)
    {
        *pOutSeparator = c;
    }

    return false;
}

/**
 * Parses a W3C-DTF date-time string into its components.
 * @param pYear year
 * @param pMonth month (one origin)
 * @param pDay day
 * @param pHour hour
 * @param pMinute minute
 * @param pSecond second
 * @param pTzHour time zone hour offset
 * @param pTzMinute time zone minute offset
 * @param rString string to parse
 * @return true if the string is valid
 */
static bool parseW3CDTFStringImpl(u32* pYear, u32* pMonth, u32* pDay, u32* pHour, u32* pMinute,
                                  u32* pSecond, s32* pTzHour, s32* pTzMinute,
                                  const SafeString& rString)
{
    s32 len = rString.calcLength();
    SafeString substr = rString;
    bool ok = true;
    char separator;

    if (parseW3CDTFSubString(&ok, pYear, &substr, &len, &separator, 4, "-", true, 0, 0xFFFFFFFF))
    {
        return ok;
    }

    if (parseW3CDTFSubString(&ok, pMonth, &substr, &len, &separator, 2, "-", true, 1, 12))
    {
        return ok;
    }

    if (parseW3CDTFSubString(&ok, pDay, &substr, &len, &separator, 2, "T", true, 1, 31))
    {
        return ok;
    }

    if (parseW3CDTFSubString(&ok, pHour, &substr, &len, &separator, 2, ":", false, 0, 23))
    {
        return ok;
    }

    if (parseW3CDTFSubString(&ok, pMinute, &substr, &len, &separator, 2, ":+-Z", true, 0, 59))
    {
        return ok;
    }

    if (separator == ':')
    {
        if (parseW3CDTFSubString(&ok, pSecond, &substr, &len, &separator, 2, ".+-Z", true, 0, 59))
        {
            return ok;
        }

        if (separator == '.')
        {
            if (len == 0)
            {
                return false;
            }

            auto it = substr.tokenBegin("+-Z");
            ++it;
            auto end = substr.tokenEnd("+-Z");

            if (it == end)
            {
                *pTzHour = 0;
                *pTzMinute = 0;
                return true;
            }

            separator = substr.at(it.getIndex() - 1);
            substr = substr.getPart(it.getIndex());
            len -= it.getIndex();
        }
    }

    if (separator == '+' || separator == '-')
    {
        u32 tzHourAbs = 0;
        bool done =
            parseW3CDTFSubString(&ok, &tzHourAbs, &substr, &len, nullptr, 2, ":", false, 0, 11);
        if (ok)
        {
            *pTzHour = tzHourAbs;
            if (separator == '-')
            {
                *pTzHour = -tzHourAbs;
            }
        }

        if (done)
        {
            return ok;
        }

        u32 tzMinuteAbs = 0;
        done = parseW3CDTFSubString(&ok, &tzMinuteAbs, &substr, &len, nullptr, 2, "", true, 0, 59);

        if (!ok)
        {
            return false;
        }

        *pTzMinute = tzMinuteAbs;
        if (separator == '-')
        {
            *pTzMinute = -tzMinuteAbs;
        }

        if (!done)
        {
            return false;
        }

        len -= 2;
    }

    return len == 0;
}

/**
 * Parses a W3C-DTF date-time string (e.g. 1997-07-16T19:20:30+01:00).
 * @param pOutTime parsed date and time
 * @param pTimeZone parsed time zone offset
 * @param rString string to parse
 * @return true if the string is valid
 */
bool parseW3CDTFString(CalendarTime* pOutTime, CalendarSpan* pTimeZone, const SafeString& rString)
{
    u32 year = 1970;
    u32 month = 1;
    u32 day = 1;
    u32 hour = 0;
    u32 minute = 0;
    u32 second = 0;

    s32 tzHour = 0;
    s32 tzMinute = 0;

    const bool ret = parseW3CDTFStringImpl(&year, &month, &day, &hour, &minute, &second, &tzHour,
                                           &tzMinute, rString);

    if (ret)
    {
        pOutTime->setDate({year, CalendarTime::Month::makeFromValueOneOrigin(month), day});
        pOutTime->setTime({hour, minute, second});

        pTimeZone->setDays(0);
        pTimeZone->setHours(tzHour);
        pTimeZone->setMinutes(tzMinute);
        pTimeZone->setSeconds(0);
    }

    return ret;
}
}  // namespace DateUtil
}  // namespace sead
