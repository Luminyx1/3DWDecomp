#include "Library/Play/Camera/ActorCameraTarget.hpp"

#include "Library/Actor/ActorPoseKeeper.hpp"
#include "Library/LiveActor/ActorCollisionFunction.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {

ActorCameraTarget::ActorCameraTarget(const LiveActor* pActor, f32 offsetY,
                                     const sead::Vector3f* pLocalOffset)
    : mActor(pActor), mLocalOffset(pLocalOffset), mOffsetY(offsetY) {}

const char* ActorCameraTarget::getTargetName() const {
    return mActor->getName();
}

void ActorCameraTarget::calcTrans(sead::Vector3f* pTrans) const {
    calcTransLocalOffset(pTrans, mActor, mLocalOffset ? *mLocalOffset : sead::Vector3f::zero);
    pTrans->y += mOffsetY;
}

void ActorCameraTarget::setTrans(sead::Vector3f& rTrans) {
    if (mActor != nullptr && mActor->getPoseKeeper() != nullptr) {
        mActor->getPoseKeeper()->getTransPtr()->set(rTrans);
    }
}

void ActorCameraTarget::calcSide(sead::Vector3f* pSide) const {
    calcSideDir(pSide, mActor);
}

void ActorCameraTarget::calcUp(sead::Vector3f* pUp) const {
    calcUpDir(pUp, mActor);
}

void ActorCameraTarget::calcFront(sead::Vector3f* pFront) const {
    calcFrontDir(pFront, mActor);
}

void ActorCameraTarget::calcGravity(sead::Vector3f* pGravity) const {
    pGravity->set(getGravity(mActor));
}

void ActorCameraTarget::calcVelocity(sead::Vector3f* pVelocity) const {
    pVelocity->set(getVelocity(mActor));
}

bool ActorCameraTarget::isCollideGround() const {
    return isExistActorCollider(mActor) && !isNoCollide(mActor) && isOnGround(mActor, 0, 0.0f);
}

bool ActorCameraTarget::isInWater() const {
    return isInWaterArea(mActor);
}

bool ActorCameraTarget::isInWater(f32 margin) const {
    return isInWaterArea(mActor, margin);
}

ActorMatrixCameraTarget::ActorMatrixCameraTarget(const LiveActor* pActor,
                                                 const sead::Matrix34f* pMtx)
    : ActorCameraTarget(pActor, 0.0f, nullptr), mMtx(pMtx) {}

}  // namespace al
