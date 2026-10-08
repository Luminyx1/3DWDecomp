/**
 * @file EmitterRes.h
 * @brief VFX emitter resources and per-particle data.
 */

#pragma once

#include <nn/gfx/gfx_Types.h>
#include <nn/types.h>

namespace nn {
namespace vfx {

namespace detail {
class Shader;
class ComputeShader;

struct ResEmitter {
    u8 _0[0x10];
    char name[0x40];
    u8 _50[0x752 - 0x50];
    u8 calcType;
    u8 followType;
    u8 _754[0x7f0 - 0x754];
    bool isLoop;
    u8 _7f1[0x7f4 - 0x7f1];
    u32 emitEndFrame;
    u8 _7f8[0x89b - 0x7f8];
    bool isAlphaMaskEnable;
    u8 _89c[0x89f - 0x89c];
    u8 maskType;
    u8 _8a0[0x8b8 - 0x8a0];
    u32 particleLife;
    u8 _8bc[0x924 - 0x8bc];
    u32 drawPath;
    u64 customShaderFlag;
    u64 customShaderSwitch;
    u8 _938[0x9c0 - 0x938];
    f32 particleScaleX;
    f32 particleScaleY;
};

struct ResAnimKey {
    f32 x;
    f32 y;
    f32 z;
    f32 time;
};

struct ResAnim8KeyParamSet {
    u32 enable;
    u32 loop;
    u32 startRandom;
    s32 keyNum;
    s32 loopRate;
    ResAnimKey keys[8];
};

struct ParticleAttribute {
    u8 _0[0x8];
    f32 createTime;
    f32 life;
    u8 _10[0x20 - 0x10];
};

}  // namespace detail

struct EmitterResource {
    void InitializeRenderState(gfx::Device* pDevice);
    void FinalizeRenderState(gfx::Device* pDevice);

    /** @return the compute shader, stored in the slot of ShaderType_Compute */
    detail::ComputeShader* GetComputeShader() const {
        return reinterpret_cast<detail::ComputeShader*>(m_Shader[3]);
    }

    u8 _0[0x10];
    detail::ResEmitter* m_pResEmitter;
    u8 _18[0x88 - 0x18];
    s32 m_ChildEmitterResNum;
    EmitterResource* m_ChildEmitterResSet[16];
    u8 _110[0x300 - 0x110];
    void* m_CustomShaderParam;
    size_t m_CustomShaderParamSize;
    void* m_CustomActionParam;
    s32* m_CustomDataParam;
    u8 _320[0x340 - 0x320];
    detail::Shader* m_Shader[8];
};

}  // namespace vfx
}  // namespace nn
