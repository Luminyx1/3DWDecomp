#pragma once

#include <nn/atk/atk_Global.h>
#include <nn/types.h>

namespace nn::atk { class SoundHandle; struct SoundParamCalculationValues; struct SoundAmbientParam; }
namespace nn::atk {
enum MixMode {
    MixMode_Pan,
    MixMode_MixParameter,
};

struct MixParameter {
    f32 ch[ChannelIndex_Count];
};
}  // namespace nn::atk
namespace nn::atk::detail {
struct RuntimeTypeInfo {
    // parent identifies the immediate base type, or null for the root type.
    explicit RuntimeTypeInfo(const RuntimeTypeInfo* parent) : parent(parent) {}
    const RuntimeTypeInfo* parent;
};
class BasicSound {
public:
    static const RuntimeTypeInfo* GetRuntimeTypeInfoStatic();
    virtual const RuntimeTypeInfo* GetRuntimeTypeInfo() const;
    virtual ~BasicSound();
    virtual void Initialize();
    virtual void Finalize();
    virtual bool IsPrepared() const = 0;
    virtual bool IsAttachedTempSpecialHandle() = 0;
    virtual void DetachTempSpecialHandle() = 0;
    class AmbientInfo;

    class AmbientParamUpdateCallback {
    public:
        /** @brief Destroys the ambient-parameter callback. */
        virtual ~AmbientParamUpdateCallback() = default;
        virtual void detail_UpdateAmbientParam(const void* pArg, u32 soundId,
                                               SoundAmbientParam* pParam) = 0;
        virtual int detail_GetAmbientPriority(const void* pArg, u32 soundId) = 0;
    };
    class AmbientArgAllocator {
    public:
        /** @brief Destroys the ambient-argument allocator. */
        virtual ~AmbientArgAllocator() = default;
        virtual void* detail_AllocAmbientArg(size_t size) = 0;
        virtual void detail_FreeAmbientArg(void* pArg, const BasicSound* pSound) = 0;
    };

    class AmbientArgUpdateCallback {
    public:
        virtual ~AmbientArgUpdateCallback() {}
        virtual void detail_UpdateAmbientArg(void* pArg, const BasicSound* pSound) = 0;
    };

    void Stop(int fadeFrames);
    void Pause(bool flag, int fadeFrames);
    void SetVolume(f32 volume, int frames);
    void SetPitch(f32 pitch);
    void SetPan(f32 pan);
    bool IsAttachedGeneralHandle();
    void DetachGeneralHandle();
    bool IsAttachedTempGeneralHandle();
    void DetachTempGeneralHandle();
    void CalculateSoundParamCalculationValues(SoundParamCalculationValues* values) const;
    bool IsPause() const;
    f32 GetVolume() const;
    void SetSurroundPan(f32 pan);
    void SetMainSend(f32 send);
    void SetFxSend(AuxBus bus, f32 send);
    void SetLpfFreq(f32 freq);
    void StartPrepared();
    void FadeIn(int frames);
    void SetOutputLine(u32 lineFlag);
    void SetOutputFxSend(OutputDevice device, AuxBus bus, f32 send);
    void SetBiquadFilter(int type, f32 value);
    void SetMixMode(MixMode mode);
    void SetOutputChannelMixParameter(OutputDevice device, u32 channel, MixParameter param);

    u32 GetId() const { return m_Id; }

private:
    friend class nn::atk::SoundHandle;
    u8 _8[8];
    SoundHandle* mGeneralHandle;
    SoundHandle* mTempGeneralHandle;
    u8 _20[0x110 - 0x20];
    u32 m_Id;
    u8 _114[0x210 - 0x114];
};
static_assert(sizeof(BasicSound) == 0x210);
}  // namespace nn::atk::detail
