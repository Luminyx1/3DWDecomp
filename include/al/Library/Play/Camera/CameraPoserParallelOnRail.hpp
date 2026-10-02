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
class RailKeeper;

class CameraPoserParallelOnRailParam {
public:
    CameraPoserParallelOnRailParam();

    f32 mDistance = 1800.0f;
    f32 mAngleV = 30.0f;
    f32 mAngleH = 0.0f;
    f32 mDistanceMin = 1800.0f;
    f32 mDistanceMax = 1800.0f;
    sead::Vector3f mLookAtOffset = sead::Vector3f::zero;
    f32 mLayoutPosMaxX = 350.0f;
    f32 mLayoutPosMaxTopY = 200.0f;
    f32 mLayoutPosMaxBottomY = 200.0f;
};

static_assert(sizeof(CameraPoserParallelOnRailParam) == 0x2c);

class CameraPoserParallelOnRail : public CameraPoser {
public:
    enum RailType : s32 {
        RailType_CameraPos = 0,
        RailType_LookAt = 1,
    };

    enum AngleType : s32 {
        AngleType_Fixed = 0,
        AngleType_Rail = 1,
    };

    CameraPoserParallelOnRail(const CameraPoserParallelOnRailParam& rParam,
                              const PlayerWatcher* pPlayerWatcher,
                              const sead::PerspectiveProjection* pProjection,
                              const PlacementId* pPlacementId);

    void initRail(const PlacementInfo* pInfo) override;
    void update() override;
    void calcDirAngleTypeFixed(sead::Vector3f* pDir) const;
    void calcDirAngleTypeRail(sead::Vector3f* pDir) const;
    void calcCameraDistance();
    void updateLookAtCamera();
    bool isAllPlayerInLayoutBox();
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    void makeLookAtCameraRailTypeCameraPos(sead::LookAtCamera* pCamera) const;
    void makeLookAtCameraRailTypeLookAt(sead::LookAtCamera* pCamera) const;
    void calcLayoutBoxIncludeAllPlayer(sead::BoundBox2f* pBox);
    void loadParam(const ByamlIter* pIter) override;

    f32 getAngleV() const override { return mParam.mAngleV; }

    f32 getAngleH() const override { return mParam.mAngleH; }

    f32 getDistanceMax() override { return mParam.mDistanceMax; }

private:
    CameraPoserParallelOnRailParam mParam;
    RailKeeper* mRailKeeper = nullptr;
    const PlayerWatcher* mPlayerWatcher;
    sead::LookAtCamera* mLookAtCamera = nullptr;
    const sead::PerspectiveProjection* mProjection;
    s32 mRailType = RailType_CameraPos;
    s32 mAngleType = AngleType_Fixed;
    s32 mDistanceKeepFrame = 0;
};

static_assert(sizeof(CameraPoserParallelOnRail) == 0x100);
}  // namespace al
