#pragma once
#include "Library/LiveActor/LiveActor.hpp"
// Partial interface; the remaining key state is not yet reconstructed.
class DoorKey : public al::LiveActor {
public:
    explicit DoorKey(const char* name);
    void triggerKillForce(bool isForce);
    void updateOpenThrowPose(int step);
    void appearPopUpFront();
    void startUnlock() { mIsUsed = true; mIsHeld = false; }
private:
    bool mIsUsed;
    bool mIsHeld;
    u8 mUnreconstructed[0x1f8 - 0x146];
};
static_assert(sizeof(DoorKey) == 0x1f8);
