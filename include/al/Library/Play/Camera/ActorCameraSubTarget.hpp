#pragma once

#include <prim/seadSafeString.h>

#include "Library/Camera/CameraSubTargetBase.hpp"

namespace al {
class LiveActor;

class ActorCameraSubTarget : public CameraSubTargetBase {
public:
    ActorCameraSubTarget(const LiveActor* pActor);

    const char* getTargetName() const override;
    void calcTrans(sead::Vector3f* pTrans) const override;
    void calcSide(sead::Vector3f* pSide) const override;
    void calcUp(sead::Vector3f* pUp) const override;
    void calcFront(sead::Vector3f* pFront) const override;
    void calcVelocity(sead::Vector3f* pVelocity) const override;

    const LiveActor* getActor() const { return mActor; }

    void setOffset(const sead::Vector3f* pOffset) { mOffset = pOffset; }

private:
    const LiveActor* mActor;
    const sead::Vector3f* mOffset = nullptr;
};

class ActorBackAroundCameraSubTarget : public ActorCameraSubTarget {
public:
    ActorBackAroundCameraSubTarget(const LiveActor* pActor);

    void calcTrans(sead::Vector3f* pTrans) const override;

    const char* getTargetName() const override { return mTargetName.cstr(); }

private:
    sead::FixedSafeString<128> mTargetName{""};
};

class TransCameraSubTarget : public CameraSubTargetBase {
public:
    TransCameraSubTarget(const char* pName, const sead::Vector3f* pTrans)
        : mName(pName), mTrans(pTrans) {}

    const char* getTargetName() const override { return mName; }

    void calcTrans(sead::Vector3f* pTrans) const override { pTrans->set(*mTrans); }

private:
    const char* mName;
    const sead::Vector3f* mTrans;
};

}  // namespace al
