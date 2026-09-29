#include <prim/seadStringUtil.h>
#include <time/seadCalendarSpan.h>
#include <time/seadDateUtil.h>

namespace sead
{
namespace DateUtil
{
bool isLeapYear(u32 year)
{
#ifdef MATCHING_HACK_NX_CLANG
    bool div100, div4;
    return (div100 = year % 100 == 0, div4 = year % 4 == 0, !div100 & div4) | (year % 400 == 0);
#else
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
#endif
}

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

void calcSecondToCalendarSpan(CalendarSpan* pOutSpan, u64 sec)
{
    if (!pOutSpan)
    {
        return;
    }
    pOutSpan->setDays(sec / (3600 * 24));
    pOutSpan->setHours((sec % (3600 * 24)) / 3600);
    pOutSpan->setMinutes((sec % 3600) / 60);
    pOutSpan->setSeconds(sec % 60);
}

bool parseW3CDTFSubString(bool* pOk, u32* pValue, SafeString* pStr, s32* pStrLength,
                          char* pOutSeparator, s32 parseLength, const SafeString& rSeparators,
                          bool allowNullSeparator, u32 valueMin, u32 valueMax)
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

    if (!StringUtil::tryParseU32(pValue, buffer, StringUtil::CardinalNumber::Base10) ||
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
    if (pOutSeparator)
    {
        *pOutSeparator = c;
    }
    return false;
}

static bool parseW3CDTFStringImpl(u32* pYear, u32* pMonth, u32* pDay, u32* pHour, u32* pMinute,
                                  u32* pSecond, s32* pTzHour, s32* pTzMinute,
                                  const SafeString& rString)
{
    s32 len = rString.calcLength();
    bool ok = true;
    SafeString substr = rString;
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

            // No timezone information
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

    if (separator != '+' && separator != '-')
    {
    checkLength:
        return len == 0;
    }

    bool done;
    u32 tz_hour_abs = 0;
    done = parseW3CDTFSubString(&ok, &tz_hour_abs, &substr, &len, nullptr, 2, ":", false, 0, 11);
    if (ok)
    {
        *pTzHour = tz_hour_abs;
        if (separator == '-')
        {
            *pTzHour = -tz_hour_abs;
        }
    }
    if (done)
    {
        return ok;
    }

    u32 tz_minute_abs = 0;
    done = parseW3CDTFSubString(&ok, &tz_minute_abs, &substr, &len, nullptr, 2, "", true, 0, 59);
    if (ok)
    {
        *pTzMinute = tz_minute_abs;
        if (separator == '-')
        {
            *pTzMinute = -tz_minute_abs;
        }

        if (done)
        {
            len -= 2;
            goto checkLength;
        }
    }

    return false;
}

bool parseW3CDTFString(CalendarTime* pOutTime, CalendarSpan* pTimeZone, const SafeString& rString)
{
    u32 year = 1970;
    u32 month = 1;
    u32 day = 1;
    u32 hour = 0;
    u32 minute = 0;
    u32 second = 0;

    s32 tz_hour = 0;
    s32 tz_minute = 0;

    const bool ret = parseW3CDTFStringImpl(&year, &month, &day, &hour, &minute, &second, &tz_hour,
                                           &tz_minute, rString);

    if (ret)
    {
        pOutTime->setDate({year, CalendarTime::Month::makeFromValueOneOrigin(month), day});
        pOutTime->setTime({hour, minute, second});

        pTimeZone->setDays(0);
        pTimeZone->setHours(tz_hour);
        pTimeZone->setMinutes(tz_minute);
        pTimeZone->setSeconds(0);
    }

    return ret;
}
}  // namespace DateUtil
}  // namespace sead
