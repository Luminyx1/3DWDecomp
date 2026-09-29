#pragma once

#include <math/seadVector.h>

namespace al {

class LinearCurve {
public:
    LinearCurve();

    void set(const sead::Vector3f&, const sead::Vector3f&);
    void calcPos(sead::Vector3f*, f32) const;
    void calcVelocity(sead::Vector3f*, f32) const;
    f32 calcLength(f32, f32) const;
    f32 calcCurveParam(f32) const;
    f32 calcNearestParam(const sead::Vector3f&) const;
    f32 calcNearestLength(f32*, const sead::Vector3f&, f32) const;
    void calcNearestPos(sead::Vector3f*, const sead::Vector3f&) const;
    void calcStartPos(sead::Vector3f*) const;
    void calcEndPos(sead::Vector3f*) const;

    f32 getLength() const { return mLength; }

private:
    sead::Vector3f mStart = sead::Vector3f::zero;  // _0
    sead::Vector3f mDiff = sead::Vector3f::zero;   // _c
    f32 mLength = 0.0f;                            // _18
};

}  // namespace al
