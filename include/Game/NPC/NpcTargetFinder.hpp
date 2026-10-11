#pragma once

#include <math/seadVector.h>

namespace al {
class LiveActor;
}
class NpcTargetFinderParam;

class NpcTargetFinder {
public:
    void update();
    const sead::Vector3f& getTargetPos() const;
    const sead::Vector3f& getLastTargetPos() const;

    // Layout prefix used by NPC states. The remaining finder storage is not yet reconstructed.
    const al::LiveActor* mHostActor;
    unsigned char mUnknown8[8];
    const al::LiveActor* mTarget;
    const al::LiveActor* mLastTarget;
    unsigned char mUnknown20[0xD];
    bool mIsTargetVisible;
    bool mIsTargetReached;
    const sead::Vector3f* mFront;
    const sead::Vector3f* mGravity;
    unsigned char mUnknown40[8];
    const NpcTargetFinderParam* mParam;
};
