#pragma once

#include <basis/seadTypes.h>

namespace al {
    /// Fires a trigger every time an accumulated time crosses a fixed interval.
    class IntervalTrigger {
    public:
        IntervalTrigger(f32 interval);

        void update(f32 delta);

        bool isTriggered() const { return mIsTriggered; }

        f32 mInterval;          // _0
        f32 mTime;              // _4
        bool mIsTriggered;      // _8
    };
};
