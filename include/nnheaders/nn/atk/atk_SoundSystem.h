#pragma once

#include <nn/atk/atk_EffectAux.h>
#include <nn/atk/atk_Global.h>

namespace nn::atk {
enum WarningId : int;
class IWarningCallbackInfo {};
class SoundSystem {
public:
    enum FsPriority {
        FsPriority_RealTime,
        FsPriority_Normal,
        FsPriority_Low
    };

    struct SoundSystemParam {
        SoundSystemParam();

        s32 soundThreadPriority;
        size_t soundThreadStackSize;
        size_t soundThreadCommandBufferSize;
        int soundThreadCommandQueueCount;
        size_t voiceCommandBufferSize;
        s32 taskThreadPriority;
        size_t taskThreadStackSize;
        size_t taskThreadCommandBufferSize;
        int taskThreadCommandQueueCount;
        FsPriority taskThreadFsPriority;
        bool enableNwRenderer;
        u32 nwVoiceSynthesizeBufferCount;
        int rendererSampleRate;
        int effectCount;
        int voiceCountMax;
        int voiceCommandWaveBufferPacketCount;
        bool enableProfiler;
        bool enableDetailSoundThreadProfile;
        bool enableRecordingFinalOutputs;
        bool enableCircularBufferSink;
        int recordingAudioFrameCount;
        int soundThreadCoreNumber;
        int taskThreadCoreNumber;
        bool enableAdditionalEffectBus;
        bool enableAdditionalSubMix;
        bool enableTaskThread;
        bool enableSoundThread;
        bool enableMemoryPoolManagement;
        bool enableCircularBufferSinkBufferManagement;
        bool enableEffect;
        bool enableSubMix;
        bool enableStereoMode;
        bool enableVoiceDrop;
        bool enableCompatibleDownMixSetting;
        bool enableCompatibleLowPassFilter;
        bool enableUnusedEffectChannelMuting;
        bool enableCompatibleBusVolume;
        bool enableCompatiblePanCurve;
        bool enableUserThreadRendering;
        bool enableCustomSubMix;
        int subMixCount;
        int subMixTotalChannelCount;
        int mixBufferCount;
        int busCountMax;
        bool enableMemoryPoolAttachCheck;
        bool enableBusMixVolume;
        bool enableVolumeThroughMode;
        bool enableHighQualityVoiceResampler;
    };
    static_assert(sizeof(SoundSystemParam) == 0x98);

    static size_t GetRequiredMemSize(const SoundSystemParam& rParam);
    static bool Initialize(const SoundSystemParam& rParam, uintptr_t workMem, size_t workMemSize);
    static void Finalize();
    static bool IsInitialized();
    static void CallWarningCallback(WarningId id, IWarningCallbackInfo* pInfo);
    static void AttachMemoryPool(audio::MemoryPoolType* pPool, void* pMemory, size_t size);
    static void DetachMemoryPool(audio::MemoryPoolType* pPool);
    static size_t GetPerformanceFrameBufferSize();
    static size_t GetRequiredEffectAuxBufferSize(const EffectAux* pEffect);
    static bool AppendEffect(AuxBus bus, EffectAux* pEffect, void* buffer, size_t bufferSize,
                             OutputDevice device);
    static void ClearEffect(AuxBus bus, OutputDevice device);
    static bool IsClearEffectFinished(AuxBus bus, OutputDevice device);
};
}  // namespace nn::atk
