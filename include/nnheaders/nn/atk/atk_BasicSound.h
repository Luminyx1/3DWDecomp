#pragma once

#include <nn/types.h>

namespace nn::atk { class SoundHandle; struct SoundParamCalculationValues; }
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
