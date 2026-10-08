#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

/**
 * @brief The state of the target and the input that a vertical camera angle update reads.
 * @note Only what reconstructed code needs is declared so far.
 */
class CameraAngleUpdateInfo {
public:
    CameraAngleUpdateInfo(bool isParallel2D, bool isSnapShotMode, f32 defaultAngle,
                          const sead::Vector3f& rCameraPos, const sead::Vector3f& rLookAtPos,
                          const sead::Vector3f& rTargetTrans, const sead::Vector3f& rPrevTargetTrans,
                          const sead::Vector2f& rStick, s32 stickSensitivityLevel,
                          f32 stickSensitivityScale, bool isPlayerTypeFlyer, bool isOnRideObj,
                          bool isOnGround, const sead::Vector3f& rGroundNormal);

    void chaseToSubTarget(f32 angle, f32 rate);

    void setRotationScaler(f32 scaler) { mRotationScaler = scaler; }

    void setIsFreezeAngle(bool isFreeze) { mIsFreezeAngle = isFreeze; }

    void setIsInInk(bool isInInk) { mIsInInk = isInInk; }

private:
    u8 _0[0x64];
    f32 mRotationScaler;
    bool mIsFreezeAngle;
    bool mIsInInk;
};

static_assert(sizeof(CameraAngleUpdateInfo) == 0x6c);
