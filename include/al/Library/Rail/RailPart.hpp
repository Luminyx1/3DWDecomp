#pragma once

#include <math/seadVector.h>

namespace al {
class BezierCurve;
class LinearCurve;

class RailPart {
public:
    RailPart();

    void init(const sead::Vector3f&, const sead::Vector3f&, const sead::Vector3f&,
              const sead::Vector3f&);
    void setUp(sead::Vector3f&, sead::Vector3f&);
    void setAccel(f32, f32);
    void getAccels(f32*, f32*);
    void setAngleS(f32);
    void setAngleE(f32);
    bool getAngleS(f32*);
    bool getAngleE(f32*);
    void calcPos(sead::Vector3f*, f32) const;
    void calcVelocity(sead::Vector3f*, f32) const;
    void calcUpDir(sead::Vector3f*, f32) const;
    f32 getPartLength() const;
    void calcDir(sead::Vector3f*, f32) const;
    void calcStartPos(sead::Vector3f*) const;
    void calcEndPos(sead::Vector3f*) const;
    f32 calcLength(f32, f32, s32) const;
    f32 calcCurveParam(f32) const;
    f32 calcNearestParam(const sead::Vector3f&, f32) const;
    void calcNearestPos(sead::Vector3f*, const sead::Vector3f&, f32) const;
    f32 calcNearestLength(f32*, const sead::Vector3f&, f32, f32) const;

    void setTotalDistance(f32 distance) { mTotalDistance = distance; }

    f32 getTotalDistance() const { return mTotalDistance; }

    bool isBezierCurve() const { return mBezierCurve != nullptr; }

private:
    BezierCurve* mBezierCurve = nullptr;              // _0
    LinearCurve* mLinearCurve = nullptr;              // _8
    f32 mTotalDistance = 0.0f;                        // _10
    sead::Vector3f mUpStart = sead::Vector3f::ey;     // _14
    sead::Vector3f mUpEnd = sead::Vector3f::ey;       // _20
    f32 mAccelStart = 0.0f;                           // _2c
    f32 mAccelEnd = 0.0f;                             // _30
    f32 mAngleStart = 0.0f;                           // _34
    f32 mAngleEnd = 0.0f;                             // _38
    bool mIsSetAngleStart = false;                    // _3c
    bool mIsSetAngleEnd = false;                      // _3d
};

static_assert(sizeof(RailPart) == 0x40);

}  // namespace al
