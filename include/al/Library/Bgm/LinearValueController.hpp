#pragma once

#include <basis/seadTypes.h>

namespace sead {
class SoundHandle;
}

namespace al {
class LinearValueController {
public:
    LinearValueController(f32 value);

    void update();
    void changeTarget(f32 target, f32 speed);

    f32 getValue() const { return mValue; }
    f32 getTarget() const { return mTarget; }
    bool isReachedTarget() const { return mTarget == mValue; }

private:
    f32 mValue;
    f32 mTarget = 0.0f;
    f32 mStep = 0.0f;
};

static_assert(sizeof(LinearValueController) == 0xc);

class BgmLpfController {
public:
    BgmLpfController(sead::SoundHandle* pHandle);

    void update();
    void changeCutOffFreq(f32 freq, f32 speed);

private:
    sead::SoundHandle* mHandle;
    LinearValueController* mFreqController;
    bool mIsReached = true;
};

static_assert(sizeof(BgmLpfController) == 0x18);

class BgmPitchController {
public:
    BgmPitchController(sead::SoundHandle* pHandle);

    void update();
    void changePitch(f32 pitch, f32 speed);
    void changeModulation(f32 depth, f32 depthSpeed, f32 speed);

private:
    void applyPitch();

    sead::SoundHandle* mHandle;
    LinearValueController* mPitchController;
    LinearValueController* mModulationDepthController;
    f32 mModulationPhase = 0.0f;
    f32 mModulationSpeed = 0.0f;
};

static_assert(sizeof(BgmPitchController) == 0x20);

class BgmVolumeController {
public:
    BgmVolumeController(sead::SoundHandle* pHandle);

    void update();
    void changeVolume(f32 volume, f32 speed);
    bool isFadeOutNow() const;
    bool isFadeInNow() const;
    bool isFinishedFadeOut() const;
    f32 getCurFadeVolume() const;

private:
    sead::SoundHandle* mHandle;
    LinearValueController* mVolumeController;
    bool mIsReached = true;
};

static_assert(sizeof(BgmVolumeController) == 0x18);
}  // namespace al
