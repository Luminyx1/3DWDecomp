#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraDistanceCurve;
class IUseCamera;

class CameraRequestParamHolder {
public:
    CameraRequestParamHolder();

    void resetPlayerType();
    bool isPlayerTypeFlyer() const;
    void onPlayerTypeFlyer(const IUseCamera* pCamera, const char* pName);
    bool isPlayerTypeHighSpeedMove() const;
    void onPlayerTypeHighSpeedMove(const IUseCamera* pCamera, const char* pName);
    bool isPlayerTypeHighJump() const;
    void onPlayerTypeHighJump(const IUseCamera* pCamera, const char* pName);
    bool isPlayerTypeNotTouchGround() const;
    void onPlayerTypeNotTouchGround(const IUseCamera* pCamera, const char* pName);
    void onRideObj(const IUseCamera* pCamera, const char* pName);
    void offRideObj(const IUseCamera* pCamera, const char* pName);

    s32 getStickSensitivityLevel() const { return mStickSensitivityLevel; }

    void setStickSensitivityLevel(s32 level) { mStickSensitivityLevel = level; }

    s32 getGyroSensitivityLevel() const { return mGyroSensitivityLevel; }

    void setGyroSensitivityLevel(s32 level) { mGyroSensitivityLevel = level; }

    CameraDistanceCurve* getBossDistanceCurve() const { return mBossDistanceCurve; }

    CameraDistanceCurve* getEquipmentDistanceCurve() const { return mEquipmentDistanceCurve; }

    bool isOnRideObj() const { return mRideObjCamera && mIsCurrRideObj; }

private:
    s32 mStickSensitivityLevel = 0;
    s32 mGyroSensitivityLevel = 0;
    bool mIsCurrFlyer = false;
    bool mIsPrevFlyer = false;
    const IUseCamera* mFlyerCamera = nullptr;
    bool mIsCurrHighSpeedMove = false;
    bool mIsPrevHighSpeedMove = false;
    const IUseCamera* mHighSpeedMoveCamera = nullptr;
    bool mIsCurrHighJump = false;
    bool mIsPrevHighJump = false;
    const IUseCamera* mHighJumpCamera = nullptr;
    bool mIsCurrNotTouchGround = false;
    bool mIsPrevNotTouchGround = false;
    const IUseCamera* mNotTouchGroundCamera = nullptr;
    bool mIsCurrRideObj = false;
    bool mIsPrevRideObj = false;
    const IUseCamera* mRideObjCamera = nullptr;
    CameraDistanceCurve* mEquipmentDistanceCurve = nullptr;
    CameraDistanceCurve* mBossDistanceCurve = nullptr;
};

}  // namespace al
