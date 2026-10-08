#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class ByamlIter;
class CameraPoser_RS;
}  // namespace al

class CameraAngleUpdateInfo;

/**
 * @brief Controls the vertical angle of a follow camera.
 * @note Only what reconstructed code needs is declared so far.
 */
class CameraAngleVerticalCtrl {
public:
    CameraAngleVerticalCtrl(al::CameraPoser_RS* pPoser);

    static f32 getInitDefaultAngleDegree();

    void loadParam(const al::ByamlIter& rIter);
    void start(const sead::Vector3f& rTargetTrans);
    void update(const CameraAngleUpdateInfo& rInfo);
    f32 getDefaultAngleDegree() const;
    void setAngleDegree(f32 angle);
    void startTargetInterpole(f32 angle);
    void startTargetInterpole(f32 angle, s32 step);
    void startTargetInterpoleByStep(f32 angle, s32 step);
    void startResetInterpole();
    void startUserCtrl();
    void startWaterCtrl(s32 step);
    void setIsCameraUnderWater(bool isUnderWater);
    void chaseToTargetDegree(f32 angle);
    void chaseToTargetDegreeBySpeed(f32 angle, f32 speed);
    void startSnap(f32 angle);
    void endSnap();
    void setHiDegreeLimit(f32 angle);
    void clearHiDegreeLimit();
    void setLowDegreeLimit(f32 angle);
    void clearLowDegreeLimit();
    void limitNegativeVerticalAngle(bool isLimit);
    void invalidateAutoResetLowAngleV();
    void setRailAngleDegreeRangeAndInterp(f32 min, f32 max, s32 step);
    void resetRailAngleDegreeRange();
    void storeVerticalAngleRange();
    void restoreVerticalAngleRange();
    void setVerticalAngleRange(f32 min, f32 max);
    bool isFixInRange() const;

    f32 getAngleDegree() const { return mAngleDegree; }

    f32 getAngleSpeed() const { return mAngleSpeed; }

    bool isWaterCtrl() const { return mIsWaterCtrl; }

    void setIsWaterCtrl(bool isWaterCtrl) { mIsWaterCtrl = isWaterCtrl; }

    bool isUnderWater() const { return mIsUnderWater; }

    bool isClimbing() const { return mIsClimbing; }

private:
    u8 _0[0xc8];
    f32 mAngleDegree;
    u8 _cc[0xe8 - 0xcc];
    f32 mAngleSpeed;
    bool mIsWaterCtrl;
    bool _ed;
    bool mIsUnderWater;
    u8 _ef[0x100 - 0xef];
    bool mIsClimbing;
    u8 _101[0x108 - 0x101];
};

static_assert(sizeof(CameraAngleVerticalCtrl) == 0x108);
