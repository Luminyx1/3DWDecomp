#pragma once
#include "Library/LiveActor/LiveActor.hpp"
namespace al { class KeyPoseKeeper; }
class KeyMoveLoopLift : public al::LiveActor {
public:
    explicit KeyMoveLoopLift(const char*);
    void startStandBy();
    void startAppearAndStandBy();
    void startAppear();
    bool isStandByEnd() const;
    const al::KeyPoseKeeper* getKeyPoseKeeper() const { return mKeyPoseKeeper; }
private:
    al::KeyPoseKeeper* mKeyPoseKeeper;
    u8 mUnreconstructed[0x18];
};
static_assert(sizeof(KeyMoveLoopLift) == 0x168);
