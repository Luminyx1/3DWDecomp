#pragma once

#include <math/seadVector.h>

namespace al {
class BezierCurve;
class LinearCurve;

class RailPart {
public:
    RailPart();

    void init(const sead::Vector3f& rStart, const sead::Vector3f& rStartHandle,
              const sead::Vector3f& rEndHandle, const sead::Vector3f& rEnd);
    void setUp(sead::Vector3f& rUpStart, sead::Vector3f& rUpEnd);
    void setAccel(f32 accelStart, f32 accelEnd);
    void getAccels(f32* pAccelStart, f32* pAccelEnd);
    void setAngleS(f32 angle);
    void setAngleE(f32 angle);
    bool getAngleS(f32* pAngle);
    bool getAngleE(f32* pAngle);
    void calcPos(sead::Vector3f* pPos, f32 param) const;
    void calcVelocity(sead::Vector3f* pVel, f32 param) const;
    void calcUpDir(sead::Vector3f* pUp, f32 distance) const;
    f32 getPartLength() const;
    void calcDir(sead::Vector3f* pDir, f32 param) const;
    void calcStartPos(sead::Vector3f* pPos) const;
    void calcEndPos(sead::Vector3f* pPos) const;
    f32 calcLength(f32 startParam, f32 endParam, s32 stepCount) const;
    f32 calcCurveParam(f32 param) const;
    f32 calcNearestParam(const sead::Vector3f& rPos, f32 interval) const;
    void calcNearestPos(sead::Vector3f* pNearest, const sead::Vector3f& rPos, f32 interval) const;
    f32 calcNearestLength(f32* pParam, const sead::Vector3f& rPos, f32 max, f32 interval) const;

    void setTotalDistance(f32 len) { mTotalDistance = len; }
    f32 getTotalDistance() const { return mTotalDistance; }
    bool isBezierCurve() const { return mBezierCurve != nullptr; }

private:
    BezierCurve* mBezierCurve = nullptr;
    LinearCurve* mLinearCurve = nullptr;
    f32 mTotalDistance = 0.0f;
    sead::Vector3f mUpStart = sead::Vector3f::ey;
    sead::Vector3f mUpEnd = sead::Vector3f::ey;
    f32 mAccelStart = 0.0f;
    f32 mAccelEnd = 0.0f;
    f32 mAngleS = 0.0f;
    f32 mAngleE = 0.0f;
    bool mIsSetAngleS = false;
    bool mIsSetAngleE = false;
};
}  // namespace al
