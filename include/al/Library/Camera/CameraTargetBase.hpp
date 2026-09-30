#pragma once

#include <math/seadVector.h>

#include "Library/HostIO/IUseHioNode.hpp"

namespace al {

class CameraTargetBase : public IUseHioNode {
public:
    CameraTargetBase();

    virtual const char* getTargetName() const = 0;
    virtual void calcTrans(sead::Vector3f* pTrans) const = 0;

    virtual void calcSide(sead::Vector3f* pSide) const {}

    virtual void calcUp(sead::Vector3f* pUp) const {}

    virtual void calcFront(sead::Vector3f* pFront) const {}

    virtual void calcGravity(sead::Vector3f* pGravity) const { *pGravity = {0.0f, -1.0f, 0.0f}; }

    virtual void calcVelocity(sead::Vector3f* pVelocity) const {}

    virtual bool isCollideGround() const { return false; }

    virtual bool isInWater() const { return false; }

    virtual bool isInMoonGravity() const { return false; }

    virtual bool isClimbPole() const { return false; }

    virtual bool isGrabCeil() const { return false; }

    virtual bool isWallCatch() const { return false; }

    virtual bool isInvalidMoveByInput() const { return false; }

    virtual bool isEnableEndAfterInterpole() const { return false; }

    virtual bool isClimbing() const { return false; }

    virtual void stopVerticalMove(bool isStop) { mIsStopVerticalMove = isStop; }

    virtual void update() {}

    virtual f32 getRequestDistance() const { return -1.0f; }

    bool isActiveTarget() const { return mIsActiveTarget; }

    void enableTarget() { mIsActiveTarget = true; }

    void disableTarget() { mIsActiveTarget = false; }

    bool isFollowExact() const { return mIsFollowExact; }

    bool isNoCameraReset() const { return mIsNoCameraReset; }

private:
    bool mIsActiveTarget = false;
    bool mIsFollowExact = false;
    bool mIsNoCameraReset = false;
    bool mIsStopVerticalMove = false;
};

static_assert(sizeof(CameraTargetBase) == 0x10);

}  // namespace al
