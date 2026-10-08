#pragma once
#include <nn/types.h>

namespace nn::atk {
class WaveBuffer;
namespace detail {
/** @brief Renderer-side voice backing one channel of a driver voice. */
class LowLevelVoice {
public:
    /** @brief Gets the identifier the profiler records this voice under. @return Identifier. */
    u32 GetVoiceId() const { return mVoiceId; }

private:
    // Renderer voice state preceding the identifier awaits reconstruction.
    u8 _0[0xe8];
    u32 mVoiceId;
};
}  // namespace detail
namespace detail::driver {
/** @brief Driver voice for one wave channel. */
class Voice {
public:
    /** @brief Gets the renderer voice in use. @return Renderer voice, or nullptr if none. */
    LowLevelVoice* GetLowLevelVoice() const { return mLowLevelVoice; }

private:
    // Per-channel playback state preceding the renderer voice awaits reconstruction.
    u8 _0[0xd8];
    LowLevelVoice* mLowLevelVoice;
};
static_assert(sizeof(Voice) == 0xe0, "Voice size");

class MultiVoice {
public:
    static const int WaveChannelMax = 2;

    void AppendWaveBuffer(int channel, WaveBuffer* buffer, bool last);
    void Pause(bool isPause);

    /** @brief Gets the number of wave channels in use. @return Channel count. */
    int GetChannelCount() const { return mChannelCount; }
    /**
     * @brief Gets the driver voice of one wave channel.
     * @param channel Channel index, in [0, GetChannelCount()).
     * @return The channel's voice.
     */
    const Voice& GetVoice(int channel) const { return mVoice[channel]; }

private:
    Voice mVoice[WaveChannelMax];
    u8 _1c0[4];
    int mChannelCount;
};
class StreamChannel {
public:
    void AppendWaveBuffer(WaveBuffer* buffer, bool last);
private:
    u8 _0[8];
    MultiVoice* mVoice;
};
}
}
