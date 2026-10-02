#pragma once

#include <math/seadVector.h>
#include <prim/seadBitFlag.h>

#include "Library/Nerve/IUseNerve.hpp"
#include "Project/Camera/CameraPoser.hpp"

namespace al {
class IUseAudioKeeper;
class NerveKeeper;
class PlayerWatcher;

class CameraPoserZoomParam {
public:
    CameraPoserZoomParam();
    CameraPoserZoomParam(f32 distance, f32 angleV, sead::Vector3f offsetLookAt);

    f32 mDistance;
    f32 mAngleV;
    sead::Vector3f mOffsetLookAt;
};

static_assert(sizeof(CameraPoserZoomParam) == 0x14);

class CameraPoserZoom : public CameraPoser, public IUseNerve {
public:
    CameraPoserZoom(const PlayerWatcher* pPlayerWatcher, const PlacementId* pPlacementId,
                    IUseAudioKeeper* pAudioKeeper, const bool* pIsReverseZoomInput);

    void update() override;
    void updateLookAtPos();
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const override;
    const CameraPoserZoomParam& getZoomParam(s32 level) const;
    void loadParam(const ByamlIter* pIter) override;
    void startSnapshotMode(f32 rate) override;
    f32 updateSnapshotFovy() override;
    f32 getSnapshotOffset() const override;
    void endSnapshotMode() override;
    void invalidControl();
    void validControl();
    void resetZoomLevel();

    void exeWait();
    void exeInterpolate();
    void exeInvalid();

    s32 getInterpoleApproachFrame() const override;
    s32 getInterpoleGoAwayFrame() const override;
    const CameraPoserZoomParam& getParamNear() const;
    const CameraPoserZoomParam& getParamNormal() const;
    const CameraPoserZoomParam& getParamFar() const;
    static const CameraPoserZoomParam& getDefaultParamNear();
    static const CameraPoserZoomParam& getDefaultParamNormal();
    static const CameraPoserZoomParam& getDefaultParamFar();
    NerveKeeper* getNerveKeeper() const override;

private:
    friend void tryLoadZoomParam(CameraPoserZoom* pPoser, s32 level, const ByamlIter* pIter,
                                 const char* pKey);

    NerveKeeper* mNerveKeeper = nullptr;
    const PlayerWatcher* mPlayerWatcher;
    CameraPoserZoomParam** mZoomParams = new CameraPoserZoomParam*[3];
    s32 mZoomLevel = 1;
    s32 mPrevZoomLevel;
    s32 mSnapshotZoomLevel = 1;
    s32 mInterpolateFrame = 0;
    bool mIsStickReleased = true;
    bool mIsValidControl = true;
    sead::BitFlag16 mStickPlayerFlag;
    IUseAudioKeeper* mAudioKeeper;
    sead::Vector3f mPrevPlayerPos = sead::Vector3f::zero;
    f32 mLookAtOffsetLength = 0.0f;
    f32 mSnapshotDistance = 0.0f;
    f32 mSnapshotTargetDistance = 0.0f;
    f32 mSnapshotStartDistance;
    const bool* mIsReverseZoomInput;
};

static_assert(sizeof(CameraPoserZoom) == 0x110);
}  // namespace al
