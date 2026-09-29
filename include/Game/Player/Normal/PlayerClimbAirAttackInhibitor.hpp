#pragma once

#include <basis/seadTypes.h>

class PlayerConstParam;

/// Blocks the cat's climb air attack for a while.
class PlayerClimbAirAttackInhibitor {
public:
    PlayerClimbAirAttackInhibitor(const PlayerConstParam*);

    void requestInhibit();
    void update();

    bool isInhibit() const { return mInhibitFrame != 0; }

private:
    const PlayerConstParam* mConstParam;  // 0x0
    s32 mInhibitFrame;                    // 0x8
};
