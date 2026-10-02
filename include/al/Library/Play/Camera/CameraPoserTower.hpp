#pragma once

#include <math/seadBoundBox.h>
#include <math/seadVector.h>

#include "Project/Camera/CameraPoser.hpp"

namespace sead {
class LookAtCamera;
class PerspectiveProjection;
}  // namespace sead

namespace al {
class PlayerWatcher;

struct CameraPoserTowerParam {
    CameraPoserTowerParam();

    sead::Vector3f axisPos = sead::Vector3f::zero;
    f32 upOffset = 0.0f;
    f32 distance = 1800.0f;
    f32 angleV = 30.0f;
    f32 distanceMin = 1800.0f;
    f32 distanceMax = 3000.0f;
    f32 layoutPosMaxX = 350.0f;
    f32 layoutPosMaxTopY = 200.0f;
    f32 layoutPosMaxBottomY = 200.0f;
    f32 topPlayerPriorRate = 1.5f;
    f32 topPlayerIncludingBaseScreenTime = 60.0f;
    f32 behindPlayerIncludingBaseScreenTime = 60.0f;
    bool isAlwaysTopPlayerPrior = false;
    f32 moveLimitMinY = -1000000.0f;
    f32 moveLimitMaxY = 1000000.0f;
    f32 moveLimitMultiMinY = -1000000.0f;
    f32 moveLimitMultiMaxY = 1000000.0f;
};

static_assert(sizeof(CameraPoserTowerParam) == 0x4c);

class CameraPoserTower : public CameraPoser {
public:
    CameraPoserTower(const CameraPoserTowerParam& rParam,
                     const sead::PerspectiveProjection* pProjection,
                     const PlayerWatcher* pPlayerWatcher, const PlacementId* pPlacementId);

    void update() override;
    void start() override;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    void loadParam(const ByamlIter* pIter) override;

    /**
     * @return The closest distance of the camera to the look at position.
     */
    f32 getDistanceMin() override { return mParam.distanceMin; }

    /**
     * @return The farthest distance of the camera to the look at position.
     */
    f32 getDistanceMax() override { return mParam.distanceMax; }

    /**
     * @return The current distance of the camera to the look at position.
     */
    virtual f32 getDstance() { return mParam.distance; }

    void updateSingleCamera();
    void updateMultiCamera();
    void calcPlayerWorldCenterPos(sead::Vector3f* pOut);
    void updateLookAtCamera();
    void limitMoveDegree();
    void applyMoveLimit(f32 minY, f32 maxY);
    void calcLookAtPosToScreenCenterX();
    void calcLookAtPosToScreenCenterY();
    void calcCameraDistance();
    void calcPlayerVecOutsideBaseScreen(sead::Vector2f* pOut, s32 index);
    void calcLookAtPosToPutPlayerInBaseScreenX(s32 index);
    void calcLookAtPosToPutPlayerInBaseScreenY(s32 index);
    void calcMoveLimitValueForMulti(f32* pMinY, f32* pMaxY) const;
    void calcLayoutBoxIncludeAllPlayer(sead::BoundBox2f* pBox);
    void calcPlayerLayoutPos(sead::Vector2f* pOut, s32 index) const;
    bool isAllPlayerInLayoutBox();
    bool isExistCameraPosWithinMoveLimitForMulti() const;
    void calcWorldPosToLayoutPos(sead::Vector2f* pOut, sead::Vector3f pos);
    void calcVecOutsideBaseScreen(const sead::Vector2f& rBasePos, sead::Vector2f* pVec);

private:
    CameraPoserTowerParam mParam;
    sead::LookAtCamera* mLookAtCamera;
    const sead::PerspectiveProjection* mProjection;
    const PlayerWatcher* mPlayerWatcher;
    bool mIsTopPlayerPrior = true;
    bool mIsDistanceMax = false;
    sead::Vector3f mAxisPosRoot = sead::Vector3f::zero;
    s32 mDistanceKeepFrame = 0;
    s32 mPriorPlayerIndex = -1;
    s32 mTopPlayerFrame = 0;
    s32 mPriorPlayerFrame = 0;
    sead::Vector3f mPrevLookAtPos = sead::Vector3f::zero;
};

static_assert(sizeof(CameraPoserTower) == 0x138);
}  // namespace al
