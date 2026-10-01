/**
 * @file EmitterSet.h
 * @brief VFX emitter set.
 */

#pragma once

#include <nn/types.h>
#include <nn/util/util_MathTypes.h>

namespace nn {
namespace vfx {
class EmitterSet;
class System;
struct EmitterResource;

namespace detail {
struct ResEmitterSet {
    u8 _0[0x10];
    char m_Name[0x48];
    u32 m_ClipRadius;
};
}  // namespace detail

struct EmitterSetResource {
    const char* GetName() const { return m_ResEmitterSet->m_Name; }

    u8 _0[0x10];
    detail::ResEmitterSet* m_ResEmitterSet;
    u8 _18[0x29 - 0x18];
    bool m_IsLoop;
    bool m_IsInfinity;
    u8 _2b[0x38 - 0x2b];
};

namespace detail {
class EmitterCalculator;
struct ResEmitter;
struct ParticleAttribute;
}  // namespace detail

class Emitter {
public:
    f32 GetFrame() const { return m_Frame; }
    Emitter* GetNextEmitter() const { return m_Next; }

    u8 _0[0x2];
    bool m_IsCalculated;
    u8 _3[0x28 - 0x3];
    s32 m_ParticleNum;
    u8 _2c[0x44 - 0x2c];
    f32 m_Frame;
    u8 _48[0x70 - 0x48];
    f32 m_EmitterAnimScale;
    f32 m_EmitterSetScale;
    EmitterSet* m_EmitterSet;
    Emitter* m_Next;
    u8 _88[0x98 - 0x88];
    detail::EmitterCalculator* m_EmitterCalculator;
    detail::ResEmitter* m_pEmitterData;
    u8 _a8[0xb0 - 0xa8];
    detail::ParticleAttribute* m_ParticleAttr;
    u8 _b8[0x1d0 - 0xb8];
    util::Float4* m_ParticlePos;
    u8 _1d8[0x1e8 - 0x1d8];
    util::Float4* m_ParticleRandom;
    u8 _1f0[0x210 - 0x1f0];
    util::Float4* m_ParticleEmitterMatrixRow[3];
    u8 _228[0x238 - 0x228];
    EmitterResource* m_pEmitterRes;
    u8 _240[0x340 - 0x240];
    util::Matrix4x3fType m_MatrixSrt;
    u8 _380[0x3b0 - 0x380];
    util::Vector3fType m_EmitterLocalPos;
    u8 _3c0[0x408 - 0x3c0];
    void* m_UserData;
    void* m_UserData2;
    u8 _418[0x440 - 0x418];
    Emitter* m_ChildEmitter[16];
};

class EmitterSet {
public:
    void Kill(bool isImmediate);
    void Fade();
    Emitter* GetAliveEmitter(s32 index) const;
    void SetMatrix(const util::Matrix4x3fType& rMatrix);
    void ForceCalculate(s32 frame);

    s32 GetCreateId() const { return m_CreateId; }
    void SetDirectionalVel(f32 vel) { m_DirectionalVel = vel; }
    bool IsFadeRequest() const { return m_IsFadeRequest; }
    bool IsAlive() const { return m_EmitterNum > 0 && m_IsAlive; }
    bool IsCalcEnable() const { return m_IsCalcEnable; }
    bool IsDrawEnable() const { return m_IsDrawEnable; }
    void SetCalcEnable(bool isEnable) { m_IsCalcEnable = isEnable; }
    void SetDrawEnable(bool isEnable) { m_IsDrawEnable = isEnable; }
    void SetUserData(u64 userData) { m_UserData = userData; }
    u64 GetUserData() const { return m_UserData; }
    EmitterSet* GetNext() const { return m_Next; }
    u32 GetDrawPathFlag() const { return m_DrawPathFlag; }
    const EmitterSetResource* GetEmitterSetResource() const { return m_EmitterSetResource; }
    const util::Vector3fType& GetClipPos() const {
        return reinterpret_cast<const util::Vector3fType&>(m_MatrixSrt._m.val[3]);
    }

    void SetEmissionRatioScale(f32 ratio);
    void SetParticleLifeScale(f32 scale);
    void SetEmitterColor0(const util::Vector4fType& rColor);
    void SetEmitterColor1(const util::Vector4fType& rColor);

    s32 GetEmitterNum() const { return m_EmitterNum; }
    const util::Matrix4x3fType& GetMatrixRt() const { return m_MatrixRt; }
    const util::Vector3fType& GetAutoCalcScale() const { return m_AutoCalcScale; }
    const util::Vector3fType& GetParticleScale() const { return m_ParticleScale; }
    const util::Vector3fType& GetParticleScaleForCalc() const { return m_ParticleScaleForCalc; }

    void SetMatrixAndScale(const util::Matrix4x3fType& rMatrix, const util::Vector3fType& rScale) {
        m_MatrixSrt._m.val[0] = vmulq_n_f32(rMatrix._m.val[0], vgetq_lane_f32(rScale._v, 0));
        m_MatrixSrt._m.val[1] = vmulq_n_f32(rMatrix._m.val[1], vgetq_lane_f32(rScale._v, 1));
        m_MatrixSrt._m.val[2] = vmulq_n_f32(rMatrix._m.val[2], vgetq_lane_f32(rScale._v, 2));
        m_MatrixSrt._m.val[3] = rMatrix._m.val[3];
        m_MatrixRt = rMatrix;
        m_IsMatrixUpdated = 1;
        m_AutoCalcScale = rScale;
        m_ParticleScaleForCalc._v = vmulq_f32(m_AutoCalcScale._v, m_ParticleScale._v);
    }

    void SetEmitterScale(const util::Vector3fType& rScale) { m_EmitterVolumeScale = rScale; }

    void SetEmitterVolumeScale(const util::Vector3fType& rScale) { m_EmitterVolumeScale = rScale; }

    void SetParticleScale(const util::Vector3fType& rScale) {
        m_ParticleScale = rScale;
        m_ParticleScaleForCalc._v = vmulq_f32(m_AutoCalcScale._v, rScale._v);
    }

    void SetColor(const util::Vector4fType& rColor) { m_Color = rColor; }

    void SetColor(f32 r, f32 g, f32 b) {
        m_Color._v = vsetq_lane_f32(r, m_Color._v, 0);
        m_Color._v = vsetq_lane_f32(g, m_Color._v, 1);
        m_Color._v = vsetq_lane_f32(b, m_Color._v, 2);
    }

    void SetAlpha(f32 alpha) { m_Color._v = vsetq_lane_f32(alpha, m_Color._v, 3); }

    u8 _0[0x9];
    bool m_IsFadeRequest;
    bool m_IsCalcEnable;
    bool m_IsDrawEnable;
    bool m_IsAlive;
    u8 _d[0x18 - 0xd];
    s32 m_IsMatrixUpdated;
    u8 _1c[0x20 - 0x1c];
    System* m_System;
    u8 _28[0x2c - 0x28];
    s32 m_CreateId;
    u8 _30[0x60 - 0x30];
    util::Matrix4x3fType m_MatrixSrt;
    util::Matrix4x3fType m_MatrixRt;
    util::Vector3fType m_EmitterVolumeScale;
    util::Vector3fType m_AutoCalcScale;
    util::Vector4fType m_Color;
    u8 _110[0x120 - 0x110];
    util::Vector3fType m_ParticleScale;
    u8 _130[0x140 - 0x130];
    util::Vector3fType m_ParticleScaleForCalc;
    u8 _150[0x17c - 0x150];
    s32 m_EmitterNum;
    u8 _180[0x1c0 - 0x180];
    u64 m_UserData;
    u8 _1c8[0x1d0 - 0x1c8];
    EmitterSet* m_Next;
    u8 _1d8[0x210 - 0x1d8];
    f32 m_DirectionalVel;
    u8 _214[0x230 - 0x214];
    u32 m_DrawPathFlag;
    EmitterSetResource* m_EmitterSetResource;
};
}  // namespace vfx
}  // namespace nn
