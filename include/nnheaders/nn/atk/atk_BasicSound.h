#pragma once

#include <nn/types.h>

namespace nn::atk::detail {
class BasicSound {
public:
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

    u32 GetId() const { return m_Id; }

private:
    u8 _0[0x110];
    u32 m_Id;
    u8 _114[0x210 - 0x114];
};
static_assert(sizeof(BasicSound) == 0x210);
}  // namespace nn::atk::detail
