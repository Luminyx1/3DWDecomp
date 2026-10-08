#pragma once

#include <basis/seadTypes.h>

/// Runs the player's giant (mega mushroom) form.
class PlayerGiantDirector {
public:
    void start();
    void end();
    void forceEnd();

    bool isGiant() const { return mTimer > 0 || mIsGiant; }

    /** @brief Tests whether only the timer still keeps the giant form. @return True if running out. */
    bool isRunningOut() const { return mTimer >= 1 && !mIsGiant; }

private:
    u8 _0[0x18];
    bool mIsGiant;  // 0x18
    s32 mTimer;  // 0x1c
};
