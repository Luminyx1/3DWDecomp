#pragma once

#include <math/seadVector.h>

namespace al { class LiveActor; }

class NpcTargetFinder {
public:
    void update();
    const sead::Vector3f& getTargetPos() const;
    const sead::Vector3f& getLastTargetPos() const;

    // Layout prefix used by NPC states. The remaining finder storage is not yet reconstructed.
    unsigned char mUnknown0[0x10];
    const al::LiveActor* mTarget;
    const al::LiveActor* mLastTarget;
    unsigned char mUnknown20[0xD];
    bool mIsTargetVisible;
    bool mIsTargetReached;
};
