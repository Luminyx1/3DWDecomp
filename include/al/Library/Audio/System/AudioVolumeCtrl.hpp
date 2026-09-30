#pragma once

#include <basis/seadTypes.h>

namespace al {
class IUseAudioKeeper;

class AudioVolumeCtrl {
public:
    AudioVolumeCtrl();

    void init();
    void update();
};

f32 getMicInputPowerOld(const IUseAudioKeeper* pUser);
f32 getMicInputPowerRatio(const IUseAudioKeeper* pUser);
bool isMicInputOn(const IUseAudioKeeper* pUser);
f32 getMicBreathPowerOld(const IUseAudioKeeper* pUser);
f32 getMicBreathPowerRatio(const IUseAudioKeeper* pUser);
bool isMicBreathInputOn(const IUseAudioKeeper* pUser);
void startMicSampling(const IUseAudioKeeper* pUser);
void startMicSamplingForce(const IUseAudioKeeper* pUser);
void stopMicSamplingForce(const IUseAudioKeeper* pUser);
void invalidateMicInput(const IUseAudioKeeper* pUser);
void validateMicInput(const IUseAudioKeeper* pUser);
}  // namespace al
