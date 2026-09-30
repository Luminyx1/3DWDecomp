#pragma once
#include <nn/os.h>

namespace nn::os {
Tick ConvertToTick(nn::TimeSpan duration);
}
namespace nn::atk::detail::fnd {
struct Time {
    static Time Current();
    s64 ticks;
};
struct TimeSpan {
    static TimeSpan FromNanoSeconds(long duration);
    static TimeSpan FromMicroSeconds(long duration);
    static TimeSpan FromMilliSeconds(long duration);
    s64 ToNanoSeconds() const;
    s64 ToMicroSeconds() const;
    s64 ToMilliSeconds() const;
    s64 ticks;
};
}
