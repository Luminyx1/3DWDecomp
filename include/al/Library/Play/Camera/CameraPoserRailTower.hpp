#pragma once

#include <math/seadVector.h>

#include "Project/Camera/CameraPoser.hpp"

namespace al {
class NerveKeeper;
class RailKeeper;

struct CameraPoserRailTowerParam {
    CameraPoserRailTowerParam();

    sead::Vector3f axisPos = sead::Vector3f::zero;
};

static_assert(sizeof(CameraPoserRailTowerParam) == 0xc);

class CameraPoserRailTower : public CameraPoser {
public:
    CameraPoserRailTower(const CameraPoserRailTowerParam& rParam, const PlacementId* pPlacementId);

    void initRail(const PlacementInfo* pInfo) override;
    void start() override;
    void update() override;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    void loadParam(const ByamlIter* pIter) override;

    void exeWait();
    void exeMove();

private:
    CameraPoserRailTowerParam mParam;
    RailKeeper* mRailKeeper = nullptr;
    NerveKeeper* mNerveKeeper = nullptr;
    f32 mSpeed = 5.0f;
    s32 mWaitTime = 0;
    f32 mOffsetY = 0.0f;
    f32 mAngleV = 30.0f;
    f32 mDistance = 1600.0f;
    s32 mSectionIndex = 0;
    sead::Vector3f mAxisPosRoot = sead::Vector3f::zero;
};

static_assert(sizeof(CameraPoserRailTower) == 0xe8);
}  // namespace al
