#pragma once

#include <basis/seadTypes.h>

namespace sead {
class AudioSettingParameter;
class MicMgrCafe;
}  // namespace sead

namespace al {
class AudioMicBreathChecker;

class AudioMicPlatform {
public:
    AudioMicPlatform(sead::AudioSettingParameter& rParam, s32 sampleNum);

    void finalize();
    virtual void update();
    void validateInput();
    void invalidateInput();
    void startSampling();
    void startSamplingForce();
    void stopSampling();
    void stopSamplingForce();
    bool isBreathInput() const;
    f32 getBreathPower() const;
    f32 getBreathPowerRatio() const;
    bool isMicInput() const;
    f32 getMicInputPower() const;
    f32 getMicInputPowerRatio() const;

private:
    s16* mSampleBuffer = nullptr;
    s32 mSampleNum;
    f32 mInputPower = 0.0f;
    f32 mInputPowerRatio = 0.0f;
    bool mIsValidInput = true;
    AudioMicBreathChecker* mBreathChecker = nullptr;
    s16* mWorkBuffer = nullptr;
    s32 mHalfSampleNum;
    sead::MicMgrCafe* mMicMgr;
    void* _40 = nullptr;
};

class AudioMic : public AudioMicPlatform {};
}  // namespace al
