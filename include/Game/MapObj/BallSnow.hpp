#pragma once

#include <basis/seadTypes.h>

#include "Library/LiveActor/LiveActor.hpp"

/** @brief Throwable snow ball, e.g. left behind by a defeated snow Pokey. */
class BallSnow : public al::LiveActor {
public:
    explicit BallSnow(const char* pName);

    bool isPlayerHold() const;
    void requestPlayerRelease();

private:
    u8 mUnreconstructed[0x9c];
};
static_assert(sizeof(BallSnow) == 0x1e0);
