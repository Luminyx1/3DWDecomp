#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}

/// Runs the player's giga form (giga bell).
class PlayerGigaDirector {
public:
    void start(const al::LiveActor* pBell, bool isClimb, bool isFirst);
    f32 getScaleRate() const;

    bool isGiga() const { return mTimer > 0 || mIsGiga; }

    /// Keep the giga form until told otherwise.
    void stay(bool isResetEndTimer) {
        mTimer = -1;

        if (isResetEndTimer) {
            mEndTimer = 60;
        }
    }

    /// Let the giga form run out again.
    void unstay(bool isResetEndTimer) {
        if (isGiga()) {
            mTimer = 50000;
        } else {
            mTimer = 0;
        }

        if (isResetEndTimer) {
            mEndTimer = 60;
        }
    }

private:
    u8 _0[0x28];
    bool mIsGiga;  // 0x28
    s32 mTimer;  // 0x2c
    s32 mEndTimer;  // 0x30
};
