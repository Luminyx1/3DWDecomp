#include "Library/Movement/FlashingTimer.hpp"

namespace al {
    /**
     * @brief Creates a stopped timer with the given blink rhythm.
     * @param duration The number of frames the timer runs for.
     * @param startHurry The remaining time below which the faster hurry blinking starts.
     * @param blinkVisibleFrames The length of one visible or hidden phase before the hurry starts.
     * @param hurryBlinkVisibleFrames The length of one visible or hidden phase once the hurry has started.
     */
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

    /**
     * @brief Checks whether the remaining time has dropped below the hurry threshold.
     * @return True once the hurry blinking has started.
     */
    bool FlashingTimer::isHurryStart() const {
        return mLastTime < mStartHurry;
    }

    /**
     * @brief Gets the remaining time.
     * @return The remaining frames, clamped to zero.
     */
    s32 FlashingTimer::getLastTime() const {
        return mLastTime < 0 ? 0 : mLastTime;
    }
};
