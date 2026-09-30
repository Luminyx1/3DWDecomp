/**
 * @file time.h
 * @brief Time implementation.
 */

#pragma once

#include <nn/types.h>

namespace nn {
class TimeSpan {
public:
    u64 nanoseconds;

    static TimeSpan FromNanoSeconds(u64 nanoSeconds) {
        TimeSpan ret;
        ret.nanoseconds = nanoSeconds;
        return ret;
    }
    static TimeSpan FromMilliSeconds(u64 milliseconds) {
        return FromNanoSeconds(milliseconds * 1000 * 1000);
    }
    static TimeSpan FromSeconds(u64 seconds) {
        return FromNanoSeconds(seconds * 1000 * 1000 * 1000);
    }
    static TimeSpan FromMinutes(u64 minutes) {
        return FromNanoSeconds(minutes * 1000 * 1000 * 1000 * 60);
    }
    static TimeSpan FromHours(u64 hours) {
        return FromNanoSeconds(hours * 1000 * 1000 * 1000 * 60 * 60);
    }
    static TimeSpan FromDays(u64 days) {
        return FromNanoSeconds(days * 1000 * 1000 * 1000 * 60 * 60 * 24);
    }

    /**
     * Converts the duration to whole seconds, truncating toward zero.
     * @return The signed number of complete seconds.
     */
    s64 GetSeconds() const {
        // High half of the signed product with the reciprocal of 1,000,000,000.
        const u64 low = static_cast<u32>(nanoseconds);
        const s64 high = static_cast<s64>(nanoseconds) >> 32;
        const s64 highLowProduct = high * 0x26d694b3;
        const u64 lowProduct = low * 0x26d694b3;
        const s64 highProduct = high * 0x112e0be8;
        const u64 lowHighProduct = low * 0x112e0be8;
        const s64 middle = highLowProduct + (lowProduct >> 32);
        const u64 carry = lowHighProduct + static_cast<u32>(middle);
        const s64 productHigh = highProduct + (middle >> 32) + (carry >> 32);
        return (productHigh >> 26) + (nanoseconds >> 63);
    }

    // Return whole microseconds, truncating signed durations toward zero.
    s64 GetMicroSeconds() const {
        const u64 low = static_cast<u32>(nanoseconds);
        const s64 high = static_cast<s64>(nanoseconds) >> 32;
        const s64 middle = high * 0xe353f7cf + ((low * 0xe353f7cf) >> 32);
        const s64 highProduct = high * 0x20c49ba5;
        const u64 carry = low * 0x20c49ba5 + static_cast<u32>(middle);
        const s64 productHigh = highProduct + (middle >> 32) + (carry >> 32);
        return (productHigh >> 7) + (nanoseconds >> 63);
    }

    // Return whole milliseconds, truncating signed durations toward zero.
    s64 GetMilliSeconds() const {
        const u64 low = static_cast<u32>(nanoseconds);
        const s64 high = static_cast<s64>(nanoseconds) >> 32;
        const s64 middle = high * 0xd7b634db + ((low * 0xd7b634db) >> 32);
        const s64 highProduct = high * 0x431bde82;
        const s64 carry = low * 0x431bde82 + static_cast<u32>(middle);
        const s64 productHigh = highProduct + (middle >> 32) + (carry >> 32);
        return (productHigh >> 18) + (nanoseconds >> 63);
    }
};

namespace time {

Result Initialize();
bool IsInitialized();

struct CalendarTime {
    s16 year;
    s8 month;
    s8 day;
    s8 hour;
    s8 minute;
    s8 second;
};

enum DayOfTheWeek { Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday };

struct TimeZone {
    char standardTimeName[0x8];
    bool _9;        // daylight savings or something?
    s32 utcOffset;  // in seconds
};

struct CalendarAdditionalInfo {
    nn::time::DayOfTheWeek dayOfTheWeek;
    s32 dayofYear;
    nn::time::TimeZone timeZone;
};

struct PosixTime {
    u64 time;
};

class StandardUserSystemClock {
public:
    static Result GetCurrentTime(nn::time::PosixTime*);
};

struct TimeZoneRule;  // shrug

Result ToCalendarTime(nn::time::CalendarTime*, nn::time::CalendarAdditionalInfo*,
                      nn::time::PosixTime const&);
Result ToCalendarTime(nn::time::CalendarTime*, nn::time::CalendarAdditionalInfo*,
                      nn::time::PosixTime const&, nn::time::TimeZoneRule const&);
Result ToPosixTime(int*, PosixTime*, int, const CalendarTime&);
CalendarTime ToCalendarTimeInUtc(const PosixTime&);
PosixTime ToPosixTimeFromUtc(const CalendarTime&);
}  // namespace time
}  // namespace nn
