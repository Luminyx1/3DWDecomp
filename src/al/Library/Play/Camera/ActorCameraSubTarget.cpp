#include "Library/Play/Camera/ActorCameraSubTarget.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"

namespace al {
ActorCameraSubTarget::ActorCameraSubTarget(const LiveActor* pActor) : mActor(pActor) {}

const char* ActorCameraSubTarget::getTargetName() const {
    return mActor->getName();
}

void ActorCameraSubTarget::calcTrans(sead::Vector3f* pTrans) const {
    pTrans->set(getTrans(mActor));

    if (mOffset) {
        sead::Vector3f side;
        sead::Vector3f up;
        sead::Vector3f front;
        calcSide(&side);
        calcUp(&up);
        calcFront(&front);
        *pTrans += side * mOffset->x + up * mOffset->y + front * mOffset->z;
    }
}

void ActorCameraSubTarget::calcSide(sead::Vector3f* pSide) const {
    calcSideDir(pSide, mActor);
}

void ActorCameraSubTarget::calcUp(sead::Vector3f* pUp) const {
    calcUpDir(pUp, mActor);
}

void ActorCameraSubTarget::calcFront(sead::Vector3f* pFront) const {
    calcFrontDir(pFront, mActor);
}

void ActorCameraSubTarget::calcVelocity(sead::Vector3f* pVelocity) const {
    pVelocity->set(getVelocity(mActor));
}

ActorBackAroundCameraSubTarget::ActorBackAroundCameraSubTarget(const LiveActor* pActor)
    : ActorCameraSubTarget(pActor) {
    mTargetName.format("%s[背後回り込み]", pActor->getName());
}

void ActorBackAroundCameraSubTarget::calcTrans(sead::Vector3f* pTrans) const {
    ActorCameraSubTarget::calcTrans(pTrans);
    sead::Vector3f front = {0.0f, 0.0f, 0.0f};
    calcFrontDir(&front, getActor());
    *pTrans += front * 200.0f;
}

}  // namespace al
