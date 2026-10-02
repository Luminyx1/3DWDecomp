#pragma once

#include <math/seadVector.h>

#include "Project/Camera/CameraPoser.hpp"

namespace al {
class IUseAudioKeeper;
class PlacementId;
class PlayerWatcher;

class CameraPoserKinopioBrigade : public CameraPoser {
public:
    CameraPoserKinopioBrigade(const PlacementId* pPlacementId, const bool* pIsReverseH,
                              const bool* pIsReverseV, IUseAudioKeeper* pAudioKeeper,
                              PlayerWatcher* pPlayerWatcher);

    void start() override;
    s32 getCameraOwnerIdx() const override;
    void update() override;
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    void loadParam(const ByamlIter* pIter) override;

public:
    f32 mAngleV = 30.0f;
    f32 mAngleH = 45.0f;
    f32 mBaseAngleV = 30.0f;
    f32 mBaseAngleH = 45.0f;
    f32 mCurrentBaseAngleV = 30.0f;
    f32 mCurrentBaseAngleH = 45.0f;
    sead::Vector3f _bc = {0.0f, 0.0f, 1.0f};
    f32 _c8 = 0.0f;
    sead::Vector3f mPadAxisX = sead::Vector3f::ex;
    sead::Vector3f mPadAxisY = sead::Vector3f::ey;
    sead::Vector3f mPadAxisZ = sead::Vector3f::ez;
    f32 mDrcAngleV = 0.0f;
    f32 mDrcAngleH = 0.0f;
    const bool* mIsReverseH;
    const bool* mIsReverseV;
    s32 mWaitFrame = 0;
    f32 mPadAngleV = 0.0f;
    IUseAudioKeeper* mAudioKeeper;
    PlayerWatcher* mPlayerWatcher;
    f32 mPrevAngleV = 0.0f;
    f32 mPrevAngleH = 0.0f;
    s32 mStickPort = -1;
    s32 mOwnerIdx = -1;
};

static_assert(sizeof(CameraPoserKinopioBrigade) == 0x130);

}  // namespace al
