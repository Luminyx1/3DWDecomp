#include "Project/Math/IntervalTrigger.hpp"
#include "Library/Math/MathUtil.hpp"

namespace al {
    /**
     * @brief Constructs a trigger with the given interval.
     * @param interval The time between two triggers.
     */
    IntervalTrigger::IntervalTrigger(f32 interval) : mInterval(interval), mTime(0.0f), mIsTriggered(false) {}

    /**
     * @brief Advances the time and sets the trigger flag if an interval boundary was crossed.
     * @param delta The time to advance by.
     */
    void IntervalTrigger::update(f32 delta) {
        mIsTriggered = false;
        mTime += delta;

        if (mTime >= mInterval) {
            mIsTriggered = true;
            mTime = modf(mTime, mInterval);
        }

        if (mTime < 0.0f) {
            mIsTriggered = true;
            mTime += (static_cast<s32>(-mTime / mInterval) + 1) * mInterval;
        }
    }
};
