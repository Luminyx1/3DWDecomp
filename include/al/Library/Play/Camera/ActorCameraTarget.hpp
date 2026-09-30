#pragma once

#include <math/seadMatrix.h>

#include "Library/Camera/CameraTargetBase.hpp"

namespace al {
class LiveActor;

class ActorCameraTarget : public CameraTargetBase {
public:
    ActorCameraTarget(const LiveActor* pActor, f32 offsetY, const sead::Vector3f* pLocalOffset);

    const char* getTargetName() const override;
    void calcTrans(sead::Vector3f* pTrans) const override;
    void setTrans(sead::Vector3f& rTrans);
    void calcSide(sead::Vector3f* pSide) const override;
    void calcUp(sead::Vector3f* pUp) const override;
    void calcFront(sead::Vector3f* pFront) const override;
    void calcGravity(sead::Vector3f* pGravity) const override;
    void calcVelocity(sead::Vector3f* pVelocity) const override;
    bool isCollideGround() const override;
    bool isInWater() const override;
    bool isInWater(f32 margin) const;

    f32 getRequestDistance() const override { return mRequestDistance; }

    const LiveActor* getActor() const { return mActor; }

    void setRequestDistance(f32 distance) { mRequestDistance = distance; }

private:
    const LiveActor* mActor;
    void* _18 = nullptr;
    const sead::Vector3f* mLocalOffset;
    f32 mOffsetY;
    f32 mRequestDistance = -1.0f;
};

class ActorMatrixCameraTarget : public ActorCameraTarget {
public:
    ActorMatrixCameraTarget(const LiveActor* pActor, const sead::Matrix34f* pMtx);

    void calcTrans(sead::Vector3f* pTrans) const override {
        mMtx->getTranslation(*pTrans);
    }

    void calcSide(sead::Vector3f* pSide) const override {
        mMtx->getBase(*pSide, 0);
        pSide->normalize();
    }

    void calcUp(sead::Vector3f* pUp) const override {
        mMtx->getBase(*pUp, 1);
        pUp->normalize();
    }

    void calcFront(sead::Vector3f* pFront) const override {
        mMtx->getBase(*pFront, 2);
        pFront->normalize();
    }

    void calcVelocity(sead::Vector3f* pVelocity) const override { pVelocity->set(0.0f, 0.0f, 0.0f); }

private:
    const sead::Matrix34f* mMtx;
};

}  // namespace al
