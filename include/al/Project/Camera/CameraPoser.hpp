#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace sead {
class LookAtCamera;
}

namespace al {
class ByamlIter;
class ControlAngleParam;
class PlacementId;
class SettingParam;
struct PlacementInfo;
class CameraPoser {
public:
    CameraPoser();

    virtual void init(const SettingParam* pParam) {}
    virtual void start() {}
    virtual void update() {}
    virtual s32 getCameraOwnerIdx() const { return -1; }
    virtual void initRail(const PlacementInfo* pInfo) {}
    virtual void loadParam(const ByamlIter* pIter);
    virtual void makeLookAtCamera(sead::LookAtCamera* pCamera) const {}
    virtual void setCameraPos(const sead::Vector3f& rPos) { mCameraPos = rPos; }
    virtual void setLookAtPos(const sead::Vector3f& rPos) { mLookAtPos = rPos; }
    virtual void setUp(const sead::Vector3f& rUp) { mCameraUp = rUp; }
    virtual void setAngleV(f32 angle) {}
    virtual void setAngleH(f32 angle) {}
    virtual void setDistance(f32 distance) {}
    virtual void setZoneMatrix(sead::Matrix34f mtx) { mZoneMtx = mtx; }
    virtual void setFovyDegree(f32 fovy) { mFovyDegree = fovy; }
    virtual f32 getAngleV() const { return 0.0f; }
    virtual f32 getAngleH() const { return 0.0f; }
    virtual f32 getDistance() { return 1600.0f; }
    virtual f32 getDistanceMin() { return 1600.0f; }
    virtual f32 getDistanceMax() { return 1600.0f; }
    virtual const sead::Vector3f& getCameraRailOffset() { return sead::Vector3f::zero; }
    virtual void setCameraRailOffset(const sead::Vector3f& rOffset) {}
    virtual f32 getSpeedCompensationV() { return 0.0f; }
    virtual void setSpeedCompensationV(f32 speed) {}
    virtual void setFirstCalcFlag(bool isFirstCalc) { mIsFirstCalc = isFirstCalc; }
    virtual void setActivateGyroModeFlag(bool isActivate) { mIsActivateGyroMode = isActivate; }
    virtual bool getActivateGyroModeFlag() const { return mIsActivateGyroMode; }
    virtual f32 updateSnapshotFovy() { return 0.0f; }
    virtual s32 getInterpoleApproachFrame() const { return mInterpoleApproachFrame; }
    virtual s32 getInterpoleGoAwayFrame() const { return mInterpoleGoAwayFrame; }
    virtual void startSnapshotMode(f32 fovy) { mIsSnapshotMode = true; }
    virtual void endSnapshotMode();
    virtual f32 getSnapshotOffset() const { return 0.0f; }

    const char* mName = nullptr;
    PlacementId* mPlacementId = nullptr;
    sead::Vector3f mCameraPos = sead::Vector3f::zero;
    sead::Vector3f mLookAtPos = sead::Vector3f::zero;
    sead::Vector3f mCameraUp = sead::Vector3f::ey;
    sead::Vector3f mCameraFront = sead::Vector3f::ez;
    f32 mFovyDegree = 35.0f;
    s32 mInterpolationFrame = 60;
    s32 mInterpoleApproachFrame = 240;
    s32 mInterpoleGoAwayFrame = 60;
    sead::Matrix34f mZoneMtx = sead::Matrix34f::ident;
    bool _88 = false;
    bool _89 = true;
    bool _8A = false;
    ControlAngleParam* mControlAngleParam = nullptr;
    bool mIsFirstCalc = true;
    bool mIsNoNormalInterpole = false;
    s32 _9C = -1;
    bool mIsActivateGyroMode = false;
    bool mIsSnapshotMode = false;
    bool _A2 = false;
};
static_assert(sizeof(CameraPoser) == 0xa8);
}  // namespace al
