#pragma once

#include <basis/seadTypes.h>

namespace al {
class AcLSoundHandle;
class AudioMixVolume;
class SePlayParamList;
class SeResourceSpecificInfo;
class SeSource;

class SeRequest {
  public:
    void setMulParamVolume(f32 volume);
    void applyVolume(bool isAfterGoal, f32 volume);

    /**
     * @brief Gets the sound-archive identifier of this request.
     * @return Current sound identifier.
     */
    u32 getSoundId() const { return mSoundId; }

    /**
     * @brief Gets the resource settings used by this sound request.
     * @return Resource-specific playback settings; required for a delayed request.
     */
    const SeResourceSpecificInfo* getSpecificInfo() const { return mSpecificInfo; }

    /**
     * @brief Gets the multiplier applied to the resource's delay in frames.
     * @return Delay multiplier assigned to this request.
     */
    s32 getDelayMultiplier() const { return mDelayMultiplier; }

  private:
    u32 mSoundId;
    const SeResourceSpecificInfo* mSpecificInfo;
    u8 _10[0x18];
    AcLSoundHandle* mHandle;
    SeSource* mSource;
    SePlayParamList* mParamList;
    AudioMixVolume* mMixVolume;
    s32 mDelayMultiplier;
    u8 _4c[0x14];
};

static_assert(sizeof(SeRequest) == 0x60);
} // namespace al
