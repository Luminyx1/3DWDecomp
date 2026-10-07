#pragma once

#include "Library/LiveActor/LiveActor.hpp"

// Partial declaration for callers; actor fields are not reconstructed yet.
class Lighthouse : public al::LiveActor {
public:
    explicit Lighthouse(const char* pName);

    void activateScenarioAnim(bool isActivate);
    void killInkPillar();
    void startInkPillar();

private:
    u8 mUnreconstructed[0x1b4];
};

static_assert(sizeof(Lighthouse) == 0x2f8);
