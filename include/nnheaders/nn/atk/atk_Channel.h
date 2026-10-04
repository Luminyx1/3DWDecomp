#pragma once

#include <nn/atk/atk_CurveAdshr.h>
#include <nn/atk/atk_SoundStartable.h>
#include <nn/atk/atk_WaveInfo.h>
#include <cstddef>

namespace nn::atk::detail::driver {
class alignas(8) Channel {
  public:
    enum ChannelCallbackStatus : int;
    using ChannelCallback = void (*)(Channel*, ChannelCallbackStatus, void*);
    static Channel* AllocChannel(int channelCount, int priority, ChannelCallback callback, void* pArgument);
    void SetUpdateType(UpdateType updateType);
    void SetOutputReceiver(OutputReceiver* pReceiver);
    void Start(const WaveInfo& rWaveInfo, int length, long sampleOffset, bool isLastWave);

    /**
     * @brief Gets the channel's amplitude envelope.
     * @return Mutable envelope used to configure attack, hold, decay, sustain and release.
     */
    CurveAdshr& GetEnvelope() { return mEnvelope; }
    /**
     * @brief Sets the played key and the sample's reference key together.
     * @param key MIDI-style played key, in [0, 127].
     * @param originalKey Original sample key, in [0, 127].
     */
    void SetKey(u8 key, u8 originalKey) {
        mKey = key;
        mOriginalKey = originalKey;
    }
    /** @brief Sets velocity gain. @param volume Linear gain derived from note velocity. */
    void SetVelocityVolume(float volume) { mVelocityVolume = volume; }
    /** @brief Sets instrument gain. @param volume Linear gain from the velocity region. */
    void SetVolume(float volume) { mVolume = volume; }
    /** @brief Sets instrument pitch. @param pitch Playback rate multiplier from the velocity region. */
    void SetPitch(float pitch) { mPitch = pitch; }
    /** @brief Sets the note's pan offset. @param pan Signed pan offset after combining track and region pan.
     */
    void SetPan(float pan) { mPan = pan; }
    /** @brief Sets the exclusive note group. @param group Group identifier used for note exclusion. */
    void SetKeyGroup(u8 group) { mKeyGroup = group; }
    /** @brief Sets whether note-off is ignored. @param ignore Nonzero preserves playback after note-off. */
    void SetIgnoreNoteOff(u8 ignore) { mIgnoreNoteOff = ignore; }
    /** @brief Sets sample interpolation. @param type Interpolation mode stored in the bank region. */
    void SetInterpolationType(u8 type) { mInterpolationType = type; }

  private:
    u8 _0[0x20];
    CurveAdshr mEnvelope;
    u8 _3c[0xc5 - 0x3c];
    u8 mIgnoreNoteOff;
    u8 _c6[0x148 - 0xc6];
    float mPan;
    u8 _14c[4];
    float mPitch;
    u8 _154[0x170 - 0x154];
    u8 mKey;
    u8 mOriginalKey;
    u8 mKeyGroup;
    u8 mInterpolationType;
    float mVolume;
    float mVelocityVolume;
    u8 _17c[0x400 - 0x17c];
};
static_assert(sizeof(Channel) == 0x400, "Channel size");
} // namespace nn::atk::detail::driver
