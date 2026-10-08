#pragma once

#include "Library/Nerve/NerveStateBase.hpp"

class RaidonBase;

/// Animation state of Plessie while she is ridden (run, swim, jump, dash...).
class RaidonRideAnimState : public al::NerveStateBase {
public:
    RaidonRideAnimState(const char* pName, RaidonBase* pHost);

    bool requestJump();
    void requestDash();
    void requestHit();
    void requestBound();

    /** @return Whether Plessie is currently swimming. */
    bool isSwim() const { return mIsSwim; }

private:
    u8 _11[0x2c - 0x11];
    bool mIsSwim;  // 0x2c
    u8 _2d[0x30 - 0x2d];
};

static_assert(sizeof(RaidonRideAnimState) == 0x30);
