#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Project/Camera/CameraPoser.hpp"

namespace al {
class PlayerWatcher;

class CameraPoserFixedAllParam {
public:
    CameraPoserFixedAllParam();

    f32 mDistance;
    f32 mAngleV;
    f32 mAngleH;
};

static_assert(sizeof(CameraPoserFixedAllParam) == 0xc);

class CameraPoserFixedAll : public CameraPoser {
public:
    CameraPoserFixedAll(const CameraPoserFixedAllParam& rParam,
                        const PlayerWatcher* pPlayerWatcher, const PlacementId* pPlacementId);

    void update() override;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    void loadParam(const ByamlIter* pIter) override;
    f32 getDistance() override { return mParam.mDistance; }
    f32 getDistanceMin() override { return mParam.mDistance; }
    f32 getDistanceMax() override { return mParam.mDistance; }

    CameraPoserFixedAllParam mParam;
    const PlayerWatcher* mPlayerWatcher;
    sead::Vector3f mLocalLookAtPos = sead::Vector3f::zero;
};

static_assert(sizeof(CameraPoserFixedAll) == 0xc8);
}  // namespace al
