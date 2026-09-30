#include "Library/Movement/FlashingTimer.hpp"

#include <math/seadMathCalcCommon.h>

namespace al {

FlashingTimer::FlashingTimer(s32 duration, s32 startHurry, s32 blinkVisibleFrames,
                             s32 hurryBlinkVisibleFrames)
    : mDuration(duration), mStartHurry(startHurry), mBlinkVisibleFrames(blinkVisibleFrames),
      mHurryBlinkVisibleFrames(hurryBlinkVisibleFrames) {}

/**
 * Updates the timer and the flashing state.
 */
void FlashingTimer::update() {
    mIsPrevVisible = mIsVisible;

    if (mLastTime < 0) {
        mIsVisible = false;
        return;
    }

    bool isNotHurry = mLastTime-- > mStartHurry;
    s32 interval = isNotHurry ? mBlinkVisibleFrames : mHurryBlinkVisibleFrames;
    s32 offset = isNotHurry ? mStartHurry : 0;
    mIsVisible = ((mLastTime - offset) / interval) & 1;
}

bool FlashingTimer::isHurryStart() const {
    return mLastTime < mStartHurry;
}

s32 FlashingTimer::getLastTime() const {
    return sead::Mathi::clampMin(mLastTime, 0);
}

}  // namespace al
