#pragma once
#include <nn/atk/atk_DecodeAdpcm.h>
#include <nn/atk/atk_Global.h>
#include <nn/atk/atk_OutputParam.h>
#include <nn/atk/atk_WaveBuffer.h>
#include <nn/types.h>

namespace nn::atk {
class OutputReceiver;
namespace detail {
template <typename T>
class ValueArray;
class BusMixVolumePacket;
class OutputAdditionalParam;
class VolumeThroughModePacket;
struct OutputBusMixVolume;

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

    /** @brief Why a voice callback was invoked. */
    enum VoiceCallbackStatus {
        VoiceCallbackStatus_FinishWave,
        VoiceCallbackStatus_Cancel,
        VoiceCallbackStatus_DropVoice,
        VoiceCallbackStatus_DropDsp,
    };

    /** @brief How the wave channels of the voice are mixed. */
    enum VoiceMode {
        VoiceMode_Mono,
        VoiceMode_StereoLeft,
        VoiceMode_StereoRight,
    };

    typedef void (*VoiceCallback)(MultiVoice* pVoice, VoiceCallbackStatus status, void* pArg);

    void Start();
    void Stop();
    void Free();
    void AppendWaveBuffer(int channel, WaveBuffer* buffer, bool last);
    void Pause(bool isPause);
    void SetSampleFormat(SampleFormat format);
    void SetSampleRate(int sampleRate);
    void SetOutputReceiver(OutputReceiver* pReceiver);
    void SetAdpcmParam(int channel, const audio::AdpcmParameter& rParam);
    size_t GetCurrentPlayingSample() const;
    void SetVolume(float volume);
    void SetPitch(float pitch);
    void SetLpfFreq(float lpfFreq);
    void SetBiquadFilter(int type, float value);
    void SetOutputLine(u32 outputLine);
    void SetPanCurve(PanCurve curve);
    void SetPanMode(PanMode mode);
    void SetVoiceMode(VoiceMode mode);
    void SetTvParam(const OutputParam& rParam);
    void SetTvAdditionalParam(const OutputAdditionalParam& rParam);
    void SetTvAdditionalParam(const ValueArray<float>* pAdditionalSend,
                              const BusMixVolumePacket* pBusMixVolumePacket,
                              const OutputBusMixVolume* pBusMixVolume,
                              const VolumeThroughModePacket* pVolumeThroughModePacket);
    static void CalcOffsetAdpcmParam(AdpcmContext* pContext, const audio::AdpcmParameter& rParam,
                                     long offset, const void* pData);

    /** @brief Gets the number of wave channels in use. @return Channel count. */
    int GetChannelCount() const { return mChannelCount; }
    /**
     * @brief Gets the driver voice of one wave channel.
     * @param channel Channel index, in [0, GetChannelCount()).
     * @return The channel's voice.
     */
    const Voice& GetVoice(int channel) const { return mVoice[channel]; }

    /**
     * @brief Selects the frame rate the voice's parameters are updated at.
     * @param updateType An UpdateType value.
     */
    void SetUpdateType(int updateType) { mUpdateType = updateType; }

private:
    Voice mVoice[WaveChannelMax];
    u8 _1c0[4];
    int mChannelCount;
    // Voice parameters preceding the update type await reconstruction.
    u8 _1c8[0xb0];
    int mUpdateType;
};

/**
 * @brief One channel of a stream sound: its ring of sample buffers and the voice playing them.
 *
 * The decoder states sit on their own cache lines; the padding is spelled out because the
 * players holding channels are allocated with plain (not over-aligned) operator new.
 */
class StreamChannel {
public:
    static const int BufferBlockCountMax = 32;

    /** @brief DSP ADPCM decoder state of one buffer block, kept on its own cache line. */
    struct BlockAdpcmContext {
        AdpcmContext context;
        u8 _6[0x3a];
    };

    void AppendWaveBuffer(WaveBuffer* buffer, bool last);

    void* mBufferAddress;
    MultiVoice* mVoice;
    WaveBuffer mWaveBuffer[BufferBlockCountMax];
    u8 _810[0x30];
    BlockAdpcmContext mAdpcmContext[BufferBlockCountMax];
    int mUpdateType;
    u8 _1044[0x3c];
};
static_assert(sizeof(StreamChannel) == 0x1080, "StreamChannel size");

/** @brief One track of a stream sound: the channels it plays and its mixing parameters. */
class StreamTrack {
public:
    static const int ChannelCountMax = 2;

    /** @brief Mixing parameters read from the stream file. */
    struct TrackInfo {
        u8 channelCount;
        u8 volume;
        u8 pan;
        u8 span;
        u8 mainSend;
        u8 fxSend[AuxBus_Count];
        u8 lpfFreq;
        u8 biquadType;
        u8 biquadValue;
        u8 flags;
    };

    bool mActiveFlag;
    StreamChannel* mChannels[ChannelCountMax];
    TrackInfo mTrackInfo;
    float mVolume;
    int mOutputLine;
    OutputParam mTvParam;
};
static_assert(sizeof(StreamTrack) == 0x80, "StreamTrack size");
}  // namespace detail::driver
}  // namespace nn::atk
