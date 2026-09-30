#include "Library/Movement/ParabolicPathMovement.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/ParabolicPath.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"

namespace {
using namespace al;

NERVE_DECL(ParabolicPathMovement, Move)

NERVES_MAKE_NOSTRUCT(ParabolicPathMovement, Move)
}  // namespace

namespace al {

/**
 * Constructs a state that moves the host along a parabola.
 * @param pHost Actor to move.
 * @param isUseVelocity Whether to move through velocity instead of setting the position.
 */
ParabolicPathMovement::ParabolicPathMovement(LiveActor* pHost, bool isUseVelocity)
    : HostStateBase("放物線挙動", pHost), mPath(new ParabolicPath()),
      mIsUseVelocity(isUseVelocity) {
    initNerve(&NrvParabolicPathMovementMove, 0);
}

/**
 * Starts a jump from the host position to a target.
 * @param rTarget End position.
 * @param maxHeight Peak height of the path.
 * @param speed Speed along the path.
 */
void ParabolicPathMovement::start(const sead::Vector3f& rTarget, f32 maxHeight, f32 speed) {
    const sead::Vector3f& trans = getTrans(getHost());
    mPath->initFromUpVector(trans, rTarget, -getGravity(getHost()), maxHeight);
    s32 time = mPath->getLength(0.0f, 1.0f, 32) / speed;
    time = time < 100 ? time : 100;
    mMoveTime = time > 10 ? time : 10;
    setNerve(this, &NrvParabolicPathMovementMove);
}

/**
 * Sets up the path and its duration without starting.
 * @param rStart Start position.
 * @param rEnd End position.
 * @param maxHeight Peak height of the path.
 * @param speed Speed along the path.
 */
void ParabolicPathMovement::setOrbitParams(const sead::Vector3f& rStart,
                                           const sead::Vector3f& rEnd, f32 maxHeight, f32 speed) {
    mPath->initFromUpVector(rStart, rEnd, -getGravity(getHost()), maxHeight);
    s32 time = mPath->getLength(0.0f, 1.0f, 32) / speed;
    time = time < 100 ? time : 100;
    mMoveTime = time > 10 ? time : 10;
}

/**
 * Checks whether the move has finished.
 * @return Whether the end was reached.
 */
bool ParabolicPathMovement::isReachedEnd() const {
    return isNerve(this, &NrvParabolicPathMovementMove) && isGreaterEqualStep(this, mMoveTime);
}

/**
 * Checks whether the move has passed its halfway point.
 * @return Whether the top was passed.
 */
bool ParabolicPathMovement::isOverTheTop() const {
    return isNerve(this, &NrvParabolicPathMovementMove) &&
           isGreaterEqualStep(this, mMoveTime / 2);
}

/**
 * Moves the host to the path position for the current step.
 */
void ParabolicPathMovement::exeMove() {
    f32 rate = static_cast<f32>(getNerveStep(this)) / mMoveTime;

    if (rate < 0.0f) {
        rate = 0.0f;
    } else if (rate > 1.0f) {
        rate = 1.0f;
    }

    sead::Vector3f pos;
    mPath->calcPosition(&pos, rate);

    if (mIsUseVelocity) {
        setVelocity(getHost(), pos - getTrans(getHost()));
    } else {
        setTrans(getHost(), pos);
    }
}

}  // namespace al
