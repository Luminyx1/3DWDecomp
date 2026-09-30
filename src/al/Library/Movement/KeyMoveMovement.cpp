#include "Library/Movement/KeyMoveMovement.hpp"

#include "Library/KeyPose/KeyPoseKeeperUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"

namespace {
using namespace al;

NERVE_DECL(KeyMoveMovement, Stop)
NERVE_DECL(KeyMoveMovement, Wait)
NERVE_DECL(KeyMoveMovement, Move)

NERVES_MAKE_NOSTRUCT(KeyMoveMovement, Stop, Wait, Move)
}  // namespace

namespace al {

/**
 * Constructs a state that moves the host through its placement key poses.
 * @param pHost Actor to move.
 * @param rInfo Placement information.
 */
KeyMoveMovement::KeyMoveMovement(LiveActor* pHost, const ActorInitInfo& rInfo)
    : HostStateBase("キー移動挙動", pHost), mTrans(getTrans(pHost)) {
    mKeyPoseKeeper = createKeyPoseKeeper(rInfo);

    f32 clippingRadius;
    calcKeyMoveClippingInfo(&mClippingTrans, &clippingRadius, mKeyPoseKeeper, 0.0f);
    clippingRadius = getClippingRadius(pHost) + clippingRadius;
    setClippingInfo(pHost, clippingRadius, &mClippingTrans);

    if (getKeyPoseCount(mKeyPoseKeeper) <= 1) {
        initNerve(&NrvKeyMoveMovementStop, 0);
    } else {
        initNerve(&NrvKeyMoveMovementWait, 0);
    }
}

/**
 * Waits at the current key pose before moving on.
 */
void KeyMoveMovement::exeWait() {
    if (isFirstStep(this)) {
        mTime = calcKeyMoveWaitTime(mKeyPoseKeeper);
    }
    if (isGreaterEqualStep(this, mTime)) {
        setNerve(this, &NrvKeyMoveMovementMove);
    }
}

/**
 * Moves toward the next key pose and advances when it is reached.
 */
void KeyMoveMovement::exeMove() {
    if (isFirstStep(this)) {
        mTime = calcKeyMoveMoveTime(mKeyPoseKeeper);
    }
    f32 rate = calcNerveRate(this, mTime);
    calcLerpKeyTrans(&mTrans, mKeyPoseKeeper, rate);
    if (isGreaterEqualStep(this, mTime)) {
        nextKeyPose(mKeyPoseKeeper);
        if (isStop(mKeyPoseKeeper)) {
            setNerve(this, &NrvKeyMoveMovementStop);
        } else {
            setNerve(this, &NrvKeyMoveMovementWait);
        }
    }
}

/**
 * Does nothing after the last key pose.
 */
void KeyMoveMovement::exeStop() {}

}  // namespace al
