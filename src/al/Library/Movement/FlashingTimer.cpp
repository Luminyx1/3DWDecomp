#include "Library/Movement/FlashingTimer.hpp"

namespace al {
    /** @brief Creates a stopped timer with the given blink rhythm. */
    FlashingTimer::FlashingTimer(s32 duration, s32 startHurry, s32 blinkVisibleFrames, s32 hurryBlinkVisibleFrames)
        : mDuration(duration), mStartHurry(startHurry), mBlinkVisibleFrames(blinkVisibleFrames),
          mHurryBlinkVisibleFrames(hurryBlinkVisibleFrames) {}

    /** @brief Counts the timer down one frame and recalculates whether the blinking object is visible. */
    void FlashingTimer::update() {
        mIsPrevVisible = mIsVisible;

        if (mLastTime < 0) {
            mIsVisible = false;
            return;
        }

        mLastTime--;

        if (isHurryStart()) {
            mIsVisible = (mLastTime / mHurryBlinkVisibleFrames) % 2 != 0;
        }
        else {
            mIsVisible = ((mLastTime - mStartHurry) / mBlinkVisibleFrames) % 2 != 0;
        }
    }

    /** @brief Returns whether the remaining time has dropped below the hurry threshold. */
    bool FlashingTimer::isHurryStart() const {
        return mLastTime < mStartHurry;
    }

    /** @brief Returns the remaining time, clamped to zero. */
    s32 FlashingTimer::getLastTime() const {
        return mLastTime < 0 ? 0 : mLastTime;
    }
};
