#include <cstdlib>
#include <limits>

#ifdef NNSDK
#include <nn/os.h>
#endif

#include <time/seadTickSpan.h>

namespace sead
{
#ifdef NNSDK
const s64 TickSpan::cFrequency = nn::os::GetSystemTickFrequency().GetInt64Value();
#else
#error "Unknown platform"
#endif

/**
 * Converts the span to nanoseconds, keeping as much precision as the range allows.
 * @return the span in nanoseconds
 */
s64 TickSpan::toNanoSeconds() const
{
    const s64 absSpan = std::abs(mSpan);
    const s64 max = std::numeric_limits<s64>::max();

    if (absSpan < max / 1'000'000'000)
    {
        return 1'000'000'000 * mSpan / cFrequency;
    }

    if (absSpan < max / 1'000'000)
    {
        return 1000 * (1'000'000 * mSpan / cFrequency);
    }

    if (absSpan < max / 1000)
    {
        return 1'000'000 * (1000 * mSpan / cFrequency);
    }

    return 1'000'000'000 * (mSpan / cFrequency);
}

/**
 * Sets the span from nanoseconds, keeping as much precision as the range allows.
 * @param nanoSeconds the span in nanoseconds
 */
void TickSpan::setNanoSeconds(s64 nanoSeconds)
{
    const s64 threshold = std::numeric_limits<s64>::max() / cFrequency;
    const s64 absNanoSeconds = std::abs(nanoSeconds);

    if (absNanoSeconds <= threshold)
    {
        mSpan = cFrequency * nanoSeconds / 1'000'000'000;
    }
    else if (absNanoSeconds <= 1000 * threshold)
    {
        mSpan = cFrequency * (nanoSeconds / 1000) / 1'000'000;
    }
    else if (absNanoSeconds <= 1'000'000 * threshold)
    {
        mSpan = cFrequency * (nanoSeconds / 1'000'000) / 1000;
    }
    else
    {
        mSpan = cFrequency * (nanoSeconds / 1'000'000'000);
    }
}

}  // namespace sead
