#pragma once

#include <math/seadVector.h>

#include "Project/Camera/CameraPoser.hpp"

namespace al {
class NerveKeeper;
class PlayerWatcher;
class RailKeeper;

struct CameraPoserRailParam {
    CameraPoserRailParam();

    f32 distance = 1800.0f;
    f32 angleV = 30.0f;
    f32 angleH = 0.0f;
    sead::Vector3f lookAtOffset = sead::Vector3f::zero;
};

static_assert(sizeof(CameraPoserRailParam) == 0x18);

class CameraPoserRail : public CameraPoser {
public:
    CameraPoserRail(const CameraPoserRailParam& rParam, const PlayerWatcher* pPlayerWatcher,
                    const PlacementId* pPlacementId);

    void initRail(const PlacementInfo* pInfo) override;
    void start() override;
    void update() override;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    void loadParam(const ByamlIter* pIter) override;

    void exeWait();
    void exeMove();

    f32 getRailCoord() const;
    void setRailCoord(f32 coord);

    bool isReverseCoord() const { return mIsReverseCoord; }

private:
    CameraPoserRailParam mParam;
    RailKeeper* mRailKeeper = nullptr;
    const PlayerWatcher* mPlayerWatcher;
    NerveKeeper* mNerveKeeper;
    f32 mSpeed = 5.0f;
    s32 mWaitTime = 0;
    s32 mSectionIndex = 0;
    bool mIsReverseCoord = true;
};

static_assert(sizeof(CameraPoserRail) == 0xe8);
}  // namespace al
