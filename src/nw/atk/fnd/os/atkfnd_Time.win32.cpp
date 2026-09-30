#include <nn/atk/atkfnd_Time.h>

namespace nn::atk::detail::fnd {
Time Time::Current() { return {nn::os::GetSystemTick().GetInt64Value()}; }
// duration is a signed nanosecond interval to convert to system ticks.
TimeSpan TimeSpan::FromNanoSeconds(long duration) {
    return {nn::os::ConvertToTick(nn::TimeSpan::FromNanoSeconds(duration)).GetInt64Value()};
}

// duration is a signed microsecond interval to convert to system ticks.
TimeSpan TimeSpan::FromMicroSeconds(long duration) { return FromNanoSeconds(duration * 1000); }
// duration is a signed millisecond interval to convert to system ticks.
TimeSpan TimeSpan::FromMilliSeconds(long duration) { return FromNanoSeconds(duration * 1000000); }
s64 TimeSpan::ToNanoSeconds() const { return nn::os::ConvertToTimeSpan(nn::os::Tick(ticks)).nanoseconds; }
s64 TimeSpan::ToMicroSeconds() const { return nn::os::ConvertToTimeSpan(nn::os::Tick(ticks)).GetMicroSeconds(); }
s64 TimeSpan::ToMilliSeconds() const { return nn::os::ConvertToTimeSpan(nn::os::Tick(ticks)).GetMilliSeconds(); }
}
