#pragma once

#include <basis/seadTypes.h>

class PlayerConstParam;

/// Blocks squatting while swimming for a while.
class PlayerSwimSquatInhibitor {
public:
    PlayerSwimSquatInhibitor(const PlayerConstParam*);

    void requestInhibit();
    void update();

    bool isInhibit() const { return mInhibitFrame != 0; }

private:
    const PlayerConstParam* mConstParam;  // 0x0
    s32 mInhibitFrame;                    // 0x8
};
