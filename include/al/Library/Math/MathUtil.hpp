#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {

    bool isNearZero(const sead::Vector2f&, f32);
    bool tryCalcAngleDegree(f32*, const sead::Vector3f&, const sead::Vector3f&);
    bool isNearZero(const sead::Vector3f&, f32);
    f32 calcAngleOnPlaneDegree(const sead::Vector3f&, const sead::Vector3f&, const sead::Vector3f&);

    void normalize(sead::Vector2f*, const sead::Vector2f&);
    void normalizeorZero(sead::Vector2f*);

    void normalize(sead::Vector3f*, const sead::Vector3f&);

    void normalizeOrZero(sead::Vector3f*, const sead::Vector3f&);
    bool normalizeOrZero(sead::Vector3f*);
    void verticalizeVec(sead::Vector3f*, const sead::Vector3f&, const sead::Vector3f&);

    bool isNear(f32, f32, f32);
    bool isNear(const sead::Vector2f&, const sead::Vector2f&, f32);
    bool isNear(const sead::Vector3f&, const sead::Vector3f&, f32);

    bool isNearZero(f32, f32);
    bool isNearZero(const sead::Matrix34f&, f32);
    bool isNearZeroOrLess(f32, f32);
    bool isExistNearZeroVal(const sead::Vector3f&, f32);
    bool isNormalize(const sead::Vector3f&, f32);
    bool isNormalize(const sead::Matrix34f&);
    bool isParallelDirection(const sead::Vector3f&, const sead::Vector3f&, f32);
    bool isReverseDirection(const sead::Vector3f&, const sead::Vector3f&, f32);
    bool isNearDirection(const sead::Vector3f&, const sead::Vector3f&, f32);
    bool isInRange(f32, f32, f32);
    void normalize(sead::Vector2f*);
    void normalize(sead::Vector3f*);
    void normalize(sead::Matrix33f*);
    void normalize(sead::Matrix34f*);


    bool isHalfProbability();

    f32 getRandom(f32, f32);

    f32 lerpValue(f32, f32, f32);
    f32 normalize(f32, f32, f32);
    f32 easeIn(f32);
    f32 easeOut(f32);
    f32 easeInOut(f32);
    f32 squareIn(f32);
    f32 squareOut(f32);
    f32 modf(f32, f32);

    bool checkHitSegmentSphereNearDepth(const sead::Vector3f&, const sead::Vector3f&,
                                        const sead::Vector3f&, f32, sead::Vector3f*,
                                        sead::Vector3f*);

    inline f32 wrapValue(f32 value, f32 max) {
        return modf(value + max, max) + 0.0f;
    }
    f32 easeByType(f32, s32);
    void lerpVec(sead::Vector3f*, const sead::Vector3f&, const sead::Vector3f&, f32);
    void slerpQuat(sead::Quatf*, const sead::Quatf&, const sead::Quatf&, f32);
    void calcSphereMargeSpheres(sead::Vector3f*, f32*, const sead::Vector3f&, f32, const sead::Vector3f&, f32);
    void normalizeOrDirZ(sead::Vector3f*);
    void normalizeOrDirZ(sead::Vector3f*, const sead::Vector3f&);
    void makeQuatFrontUp(sead::Quatf*, const sead::Vector3f&, const sead::Vector3f&);
    void calcQuatUp(sead::Vector3f*, const sead::Quatf&);
    void turnQuatYDirRate(sead::Quatf*, const sead::Quatf&, const sead::Vector3f&, f32);
    f32 calcRate01(f32, f32, f32);

}  // namespace al
