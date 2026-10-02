#include "Library/Play/Camera/ActorCameraSubTarget.hpp"

#include <attributes.h>

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Project/Camera/CameraSubTargetTurnParam.hpp"

namespace al {
namespace {
const CameraSubTargetTurnParam sDefaultTurnParam;
}  // namespace

/**
 * @brief Constructs a sub target using the default turn parameters.
 */
NOINLINE CameraSubTargetBase::CameraSubTargetBase()
    : mTurnParam(&sDefaultTurnParam) {}

/**
 * Constructs a sub target that follows an actor.
 * @param pActor Actor to follow.
 */
ActorCameraSubTarget::ActorCameraSubTarget(const LiveActor* pActor) : mActor(pActor) {}

/**
 * @return Name of the followed actor.
 */
const char* ActorCameraSubTarget::getTargetName() const {
    return mActor->getName();
}

/**
 * Calculates the actor position, moved by the offset in the local space of the actor if set.
 * @param pTrans Output position.
 */
void ActorCameraSubTarget::calcTrans(sead::Vector3f* pTrans) const {
    pTrans->set(getTrans(mActor));

    if (mOffset != nullptr) {
        sead::Vector3f side;
        sead::Vector3f up;
        sead::Vector3f front;
        calcSide(&side);
        calcUp(&up);
        calcFront(&front);
        *pTrans += side * mOffset->x + up * mOffset->y + front * mOffset->z;
    }
}

/**
 * @param pSide Output side direction of the actor.
 */
void ActorCameraSubTarget::calcSide(sead::Vector3f* pSide) const {
    calcSideDir(pSide, mActor);
}

/**
 * @param pUp Output up direction of the actor.
 */
void ActorCameraSubTarget::calcUp(sead::Vector3f* pUp) const {
    calcUpDir(pUp, mActor);
}

/**
 * @param pFront Output front direction of the actor.
 */
void ActorCameraSubTarget::calcFront(sead::Vector3f* pFront) const {
    calcFrontDir(pFront, mActor);
}

/**
 * @param pVelocity Output velocity of the actor.
 */
void ActorCameraSubTarget::calcVelocity(sead::Vector3f* pVelocity) const {
    pVelocity->set(getVelocity(mActor));
}

/**
 * Constructs a sub target that moves the camera around to the back of an actor.
 * @param pActor Actor to follow.
 */
ActorBackAroundCameraSubTarget::ActorBackAroundCameraSubTarget(const LiveActor* pActor)
    : ActorCameraSubTarget(pActor) {
    mTargetName.format("%s[背後回り込み]", pActor->getName());
}

/**
 * Calculates a position in front of the actor, so the camera turns to its back.
 * @param pTrans Output position.
 */
void ActorBackAroundCameraSubTarget::calcTrans(sead::Vector3f* pTrans) const {
    ActorCameraSubTarget::calcTrans(pTrans);
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    calcFrontDir(&front, getActor());
    *pTrans += front * 200.0f;
}

}  // namespace al
