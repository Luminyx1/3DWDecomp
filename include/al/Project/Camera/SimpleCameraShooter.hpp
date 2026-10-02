#pragma once

#include <math/seadVector.h>

namespace sead {
class LookAtCamera;
class PerspectiveProjection;
}  // namespace sead

namespace al {
class IUseCollision;
class Triangle;

class SimpleCameraShooter {
public:
    SimpleCameraShooter(const char* pName);

    void calcAspectRatioByScreenSize(s32 width, s32 height);
    void calcCameraPos(sead::Vector3f* pPos, const IUseCollision* pCollision) const;
    void calcFrontDir(sead::Vector3f* pDir) const;
    bool checkCollision(f32 distance, const IUseCollision* pCollision) const;
    void applyCameraWithCollision(sead::LookAtCamera* pCamera,
                                  const IUseCollision* pCollision) const;
    void applyProjection(sead::PerspectiveProjection* pProjection) const;
    void lookAt(const sead::Vector3f& rPos);
    void applyLimitation();
    void rotateYaw(f32 degree);
    void rotatePitch(f32 degree);

    void setTargetPos(const sead::Vector3f& rPos) { mTargetPos.set(rPos); }

    void setDistance(f32 distance) { mDistance = distance; }

private:
    sead::Vector3f mTargetPos;
    sead::Vector3f _c;
    sead::Vector3f _18;
    sead::Vector3f _24;
    f32 mDistance;
    f32 mFovyDegree;
    f32 mNear;
    f32 mFar;
    f32 mAspect;
    Triangle* mTriangles;
};

static_assert(sizeof(SimpleCameraShooter) == 0x50);

}  // namespace al
