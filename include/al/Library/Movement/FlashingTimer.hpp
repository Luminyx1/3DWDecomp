#pragma once

#include <basis/seadTypes.h>

namespace al {
    /// Counts down a timer and toggles a visibility flag in two blink rhythms (normal and hurry).
    class FlashingTimer {
    public:
        FlashingTimer(s32 duration, s32 startHurry, s32 blinkVisibleFrames, s32 hurryBlinkVisibleFrames);

        void update();
        bool isHurryStart() const;
        s32 getLastTime() const;

        s32 mLastTime = -1;                 // _0
        s32 mDuration;                      // _4
        s32 mStartHurry;                    // _8
        s32 mBlinkVisibleFrames;            // _C
        s32 mHurryBlinkVisibleFrames;       // _10
        bool mIsVisible = false;            // _14
        bool mIsPrevVisible = false;        // _15
    };

    static_assert(sizeof(FlashingTimer) == 0x18, "FlashingTimer size");
};
