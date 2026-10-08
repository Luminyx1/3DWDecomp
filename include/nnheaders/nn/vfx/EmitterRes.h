/**
 * @file EmitterRes.h
 * @brief VFX emitter resources and per-particle data.
 */

#pragma once

#include <nn/gfx/gfx_Types.h>
#include <nn/types.h>
#include <nn/util/util_MathTypes.h>

namespace nn {
namespace vfx {

namespace detail {
class Shader;
class ComputeShader;
struct EmitterStaticUniformBlock;
struct ResFieldRandom;
struct ResFieldRandomSimple;
struct ResFieldMagnet;
struct ResFieldSpin;
struct ResFieldCollision;
struct ResFieldConvergence;
struct ResFieldPosAdd;
struct ResFieldCurlNoise;
struct ResFieldCustom;

struct ResEmitter {
    u8 _0[0x10];
    char name[0x40];
    u8 _50[0x752 - 0x50];
    u8 calcType;
    u8 followType;
    u8 _754[0x7f0 - 0x754];
    bool isLoop;
    bool isWorldGravity;
    u8 _7f2[0x7f4 - 0x7f2];
    u32 emitEndFrame;
    u8 _7f8[0x818 - 0x7f8];
    util::Float3 gravity;
    u8 _824[0x89b - 0x824];
    bool isAlphaMaskEnable;
    u8 _89c[0x89f - 0x89c];
    u8 maskType;
    u8 _8a0[0x8ad - 0x8a0];
    bool isRotateDirRandom[3];
    u8 _8b0[0x8b8 - 0x8b0];
    s32 particleLife;
    u8 _8bc[0x8d8 - 0x8bc];
    bool isAnimLoop[5];
    u8 isAnimStartRandom[5];
    u8 _8e2[0x8e4 - 0x8e2];
    s32 animLoopRate[5];
    u8 _8f8[0x924 - 0x8f8];
    u32 drawPath;
    u64 customShaderFlag;
    u64 customShaderSwitch;
    u8 _938[0x99c - 0x938];
    u8 colorCalcType[2];
    u8 alphaCalcType[2];
    util::Float4 color[2];
    f32 particleScaleX;
    f32 particleScaleY;
    u8 _9c8[0x9e4 - 0x9c8];
    bool isAlphaFluctuation;
    bool isScaleFluctuation;
    bool isScaleFluctuationAxisSeparate;
    u8 fluctuationFlag;
};

struct ResAnimKey {
    f32 x;
    f32 y;
    f32 z;
    f32 time;

    /** @return the key value */
    const util::Float3& GetValue() const { return *reinterpret_cast<const util::Float3*>(&x); }
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

    u8 _0[0x3];
    bool m_IsUseField;
    u8 _4[0x10 - 0x4];
    detail::ResEmitter* m_pResEmitter;
    detail::EmitterStaticUniformBlock* m_pEmitterStaticUbo;
    u8 _20[0x88 - 0x20];
    s32 m_ChildEmitterResNum;
    EmitterResource* m_ChildEmitterResSet[16];
    u8 _110[0x248 - 0x110];
    detail::ResFieldRandom* m_pFieldRandomData;
    detail::ResFieldRandomSimple* m_pFieldRandomSimpleData;
    detail::ResFieldMagnet* m_pFieldMagnetData;
    detail::ResFieldSpin* m_pFieldSpinData;
    detail::ResFieldCollision* m_pFieldCollisionData;
    detail::ResFieldConvergence* m_pFieldConvergenceData;
    detail::ResFieldPosAdd* m_pFieldPosAddData;
    detail::ResFieldCurlNoise* m_pFieldCurlNoiseData;
    detail::ResFieldCustom* m_pFieldCustomData;
    u8 _290[0x300 - 0x290];
    void* m_CustomShaderParam;
    size_t m_CustomShaderParamSize;
    void* m_CustomActionParam;
    s32* m_CustomDataParam;
    u8 _320[0x340 - 0x320];
    detail::Shader* m_Shader[8];
};

}  // namespace vfx
}  // namespace nn
