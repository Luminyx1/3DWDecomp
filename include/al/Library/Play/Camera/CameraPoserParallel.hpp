#pragma once

#include <math/seadBoundBox.h>
#include <math/seadVector.h>

#include "Project/Camera/CameraPoser.hpp"
#include "Project/Camera/SettingParam.hpp"
#include "Project/Collision/IUseCollision.hpp"

namespace sead {
class LookAtCamera;
class PerspectiveProjection;
}  // namespace sead

namespace al {
class CollisionDirector;
class PlayerWatcher;

class CameraPoserParallelParam {
public:
    CameraPoserParallelParam();

    f32 mDistance = 1600.0f;
    f32 mAngleV = 30.0f;
    f32 mAngleH = 0.0f;
    f32 mDistanceMin = 1600.0f;
    f32 mDistanceMax = 2600.0f;
    f32 mLookAtOffsetX = 0.0f;
    f32 mLookAtOffsetY = 0.0f;
    sead::Vector3f mMoveLimitMin = {-1000000.0f, -1000000.0f, -1000000.0f};
    sead::Vector3f mMoveLimitMax = {1000000.0f, 1000000.0f, 1000000.0f};
    sead::Vector3f mMoveLimitMinForMulti = {-1000000.0f, -1000000.0f, -1000000.0f};
    sead::Vector3f mMoveLimitMaxForMulti = {1000000.0f, 1000000.0f, 1000000.0f};
    f32 mMoveLimitRotateDegreeY = 0.0f;
    SettingParam mSettingParam;
};

static_assert(sizeof(CameraPoserParallelParam) == 0x88);

class CameraPoserParallel : public CameraPoser, public IUseCollision {
public:
    CameraPoserParallel(const CameraPoserParallelParam& rParam,
                        const sead::PerspectiveProjection* pProjection,
                        PlayerWatcher* pPlayerWatcher, const sead::LookAtCamera* pLookAtCamera,
                        CollisionDirector* pCollisionDirector, const PlacementId* pPlacementId);

    void init(const SettingParam* pParam) override;
    void start() override;
    void update() override;
    void updateCourseSelectCamera();
    void updateSingleCamera();
    void updateMultiCamera();
    void startSnapshotMode(f32 fovy) override;
    void endSnapshotMode() override;
    void setParam(const CameraPoserParallelParam& rParam);
    void changeSingleCameraMode();
    void updateLookAtCamera();
    void applyCameraOffset();
    void applyCameraRailOffset();
    void applyMoveLimit(const sead::Vector3f& rMin, const sead::Vector3f& rMax);
    bool checkStrikeCollision(const sead::Vector3f& rPos);
    void calcLookAtPosToScreenCenterX();
    void calcLookAtPosToScreenCenterY();
    void calcMultiCameraDistance();
    void calcPlayerVecOutsideBaseScreen(sead::Vector2f* pOut, s32 index) const;
    f32 calcPlayerScaleInScreen(s32 index) const;
    void calcPlayerVecOnScreen(sead::Vector2f* pOut, s32 index) const;
    void calcPlayerLayoutPos(sead::Vector2f* pOut, s32 index) const;
    void calcVecOutsideBaseScreen(sead::Vector2f* pOut, const sead::Vector2f& rLayoutPos) const;
    void calcLookAtPosToPutPlayerInBaseScreenX(s32 index);
    void calcLookAtPosToPutPlayerInBaseScreenY(s32 index);
    void calcSpeedCompensationMulti();
    void calcWorldPosToLayoutPos(sead::Vector2f* pOut, const sead::Vector3f& rWorldPos) const;
    void calcMoveLimitValueForMulti(sead::Vector3f* pMin, sead::Vector3f* pMax) const;
    void includeBaseAfterMoveLimit();
    bool checkEnableCameraApproach();
    bool isAllPlayerInLayoutBox() const;
    bool isExistCameraPosWithinMoveLimitForMulti() const;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    void loadParam(const ByamlIter* pIter) override;
    void calcPlayerWorldCenterPos(sead::Vector3f* pOut) const;
    bool isPlayerInLayoutBox(s32 index) const;
    void calcLayoutBoxIncludeAllPlayer(sead::BoundBox2f* pOut, bool* pIsBehindCamera) const;

    void setLookAtPos(const sead::Vector3f& rPos) override { mLookAtPos = rPos; }

    void setDistance(f32 distance) override { mParam.mDistance = distance; }

    f32 getAngleV() const override { return mParam.mAngleV; }

    f32 getAngleH() const override { return mParam.mAngleH; }

    f32 getDistance() override { return mParam.mDistance; }

    f32 getDistanceMin() override { return mParam.mDistanceMin; }

    f32 getDistanceMax() override { return mParam.mDistanceMax; }

    const sead::Vector3f& getCameraRailOffset() override { return mCameraRailOffset; }

    void setCameraRailOffset(const sead::Vector3f& rOffset) override {
        mCameraRailOffset = rOffset;
    }

    f32 getSpeedCompensationV() override { return mSpeedCompensationV; }

    void setSpeedCompensationV(f32 speed) override { mSpeedCompensationV = speed; }

    CollisionDirector* getCollisionDirector() const override { return mCollisionDirector; }

    CollisionDirector* mCollisionDirector;
    sead::LookAtCamera* mLookAtCamera = nullptr;
    const sead::LookAtCamera* mSceneLookAtCamera;
    const sead::PerspectiveProjection* mProjection;
    CameraPoserParallelParam mParam;
    PlayerWatcher* mPlayerWatcher;
    const SettingParam* mSettingParam = nullptr;
    s32 mDistanceKeepFrame = 0;
    s32 mTopPlayerInterpFrame = 0;
    s32 mBasePlayerInterpFrame = 0;
    s32 mBasePlayerIndex = -1;
    sead::Vector3f* mPrevPlayerPos;
    bool mIsTopPlayerBase = true;
    bool mIsDistanceMax = false;
    f32 mAngleHFromZoneToRoot;
    s32 mHeightAdjustFrame = 0;
    bool mIsUseAdvancedSetting = false;
    sead::Vector3f mCameraRailOffset = sead::Vector3f::zero;
    bool mIsUseRotateDegreeYMoveLimit = false;
    f32 mSpeedCompensationH = 0.0f;
    f32 mSpeedCompensationV = 0.0f;
    sead::Vector3f mInterpStartLookAtPos = sead::Vector3f::zero;
    sead::Vector3f mBaseLookAtPos = sead::Vector3f::zero;
    sead::Vector2f mTopPlayerMoveDir = sead::Vector2f::zero;
    s32 mBaseChangeFrame = 65;
    s32 mBaseCandidateIndex = -1;
    bool mIsWaitApproach = false;
    bool* mIsPlayerApproachChecked = nullptr;
    s32 mApproachWaitFrame = 0;
    bool mIsKeepCameraHeight = false;
    sead::Vector3f mKeepCameraPos = sead::Vector3f::zero;
    s32 mMoveLimitInterpFrame = 0;
    s32 mMoveLimitReleaseFrame = 0;
    sead::Vector3f mMoveLimitInterpStartPos = sead::Vector3f::zero;
    sead::Vector3f mPrevLookAtPos = sead::Vector3f::zero;
    bool mIsForceGyroMode = false;
    bool mIsUseOffsetAtMultiMode = false;
    bool mIsNoNormalInterpoleBeforeSnapshot;
};

static_assert(sizeof(CameraPoserParallel) == 0x218);

}  // namespace al
