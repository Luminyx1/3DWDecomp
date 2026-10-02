#pragma once

#include <math/seadVector.h>

#include "Project/Camera/CameraPoser.hpp"

namespace al {
class CameraSwitcher;
class PlayerWatcher;

class CameraPoserEntranceParam {
public:
    CameraPoserEntranceParam();

    f32 mAngleV = 30.0f;
    f32 mAngleH = 0.0f;
    f32 mDistance = 1800.0f;
    sead::Vector3f mLookAtOffset = {0.0f, 0.0f, 0.0f};
};

static_assert(sizeof(CameraPoserEntranceParam) == 0x18);

class CameraPoserEntrance : public CameraPoser {
public:
    CameraPoserEntrance(const CameraPoserEntranceParam& rParam,
                        const PlayerWatcher* pPlayerWatcher, CameraSwitcher* pCameraSwitcher,
                        const PlacementId* pPlacementId);

    void start() override;
    void update() override;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    void loadParam(const ByamlIter* pIter) override;

    f32 getAngleV() const override { return mParam.mAngleV; }

    f32 getAngleH() const override { return mParam.mAngleH; }

    f32 getDistance() override { return mParam.mDistance; }

    f32 getDistanceMin() override { return mParam.mDistance; }

    f32 getDistanceMax() override { return mParam.mDistance; }

private:
    CameraPoserEntranceParam mParam;
    const PlayerWatcher* mPlayerWatcher;
    sead::Vector3f* mStartPlayerPos = nullptr;
    CameraSwitcher* mCameraSwitcher;
    bool _d8 = false;
};

static_assert(sizeof(CameraPoserEntrance) == 0xe0);

}  // namespace al
