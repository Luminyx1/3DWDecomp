/**
 * @file Callback.h
 * @brief VFX callback arguments and callback sets.
 */

#pragma once

#include <nn/gfx/gfx_DescriptorSlot.h>
#include <nn/gfx/gfx_GpuAddress.h>
#include <nn/gfx/gfx_Types.h>
#include <nn/types.h>
#include <nn/util/util_MathTypes.h>
#include <nn/vfx/vfx_Buffer.h>

namespace nn {
namespace vfx {

class Emitter;
class System;

namespace detail {
class EmitterCalculator;
class Shader;
struct ParticleProperty;
struct ResFieldCustom;
struct ResAnim8KeyParamSet;
}  // namespace detail

/** Number of per-frame copies kept of multi-buffered GPU data. */
enum BufferingMode {
    BufferingMode_Single = 1,
    BufferingMode_Double = 2,
    BufferingMode_Triple = 3,
};

class TemporaryBuffer {
public:
    TemporaryBuffer() { Invalidate(); }

    void Initialize(gfx::Device* pDevice, size_t size, BufferingMode bufferingMode);
    void Invalidate();
    void Finalize(gfx::Device* pDevice);
    void Swap();
    void* Map(gfx::GpuAddress* pAddress, size_t size);
    void Unmap();

    u8 _0[0x38];
    detail::Buffer m_Buffer[3];
};

static_assert(sizeof(TemporaryBuffer) == 0x548);

enum CustomShaderConstantBufferIndex {
    CustomShaderConstantBufferIndex_0,
    CustomShaderConstantBufferIndex_1,
    CustomShaderConstantBufferIndex_2,
    CustomShaderConstantBufferIndex_3,
};

enum CustomShaderTextureType {
    CustomShaderTextureType_0,
    CustomShaderTextureType_1,
    CustomShaderTextureType_2,
    CustomShaderTextureType_3,
    CustomShaderTextureType_4,
    CustomShaderTextureType_5,
    CustomShaderTextureType_6,
    CustomShaderTextureType_7,
    CustomShaderTextureType_8,
    CustomShaderTextureType_9,
    CustomShaderTextureType_10,
    CustomShaderTextureType_11,
    CustomShaderTextureType_12,
    CustomShaderTextureType_28 = 28,
    CustomShaderTextureType_29,
};

enum DrawPathCallbackId {
    DrawPathCallbackId_0,
    DrawPathCallbackId_1,
};

enum DrawPathFlag : u64 {
    DrawPathFlag_None = 0,
};

struct ViewParam;

namespace detail {
struct SortData;
}  // namespace detail

struct DrawParameterArg {
    s32 m_ProcessingIndex;
    ViewParam* m_pViewParam;
    u32 m_ViewFlag;
    s32 m_14;
    gfx::DescriptorSlot m_FrameBufferTextureSlot;
    gfx::DescriptorSlot m_DepthBufferTextureSlot;
    gfx::GpuAddress* m_pViewGpuAddress;
    TemporaryBuffer* m_pTemporaryBuffer;
    detail::SortData* m_pParticleSortBuffer;
};

struct EndianFlipArg;

struct EmitterInitializeArg {
    Emitter* pEmitter;
};

struct EmitterPostCalculateArg {
    Emitter* pEmitter;
};

struct EmitterFinalizeArg {
    Emitter* pEmitter;
};

enum ShaderType {
    ShaderType_Normal,
    ShaderType_User1,
    ShaderType_User2,
    ShaderType_Compute,
};

struct EmitterDrawArg {
    gfx::CommandBuffer* pCommandBuffer;
    Emitter* pEmitter;
    ShaderType shaderType;
    void* pUserParam;
    DrawParameterArg* pDrawParameterArg;
};

struct ParticleCalculateArgImpl {
    void* pUserData;
    u8 _8[0x10 - 0x8];
    Emitter* pEmitter;
    f32 time;
    f32 life;
    s32 particleIndex;
};

class RenderStateSetArg {
public:
    detail::Shader* GetShader();
    System* GetSystem();

    gfx::CommandBuffer* pCommandBuffer;
    Emitter* pEmitter;
    s32 shaderType;
    void* pUserParam;
    bool isComputeShader;
    DrawParameterArg* pDrawParameterArg;
};

bool InvokeRenderStateSetCallback(RenderStateSetArg& rArg);

typedef void (*EndianFlipCallback)(EndianFlipArg& rArg);
typedef bool (*EmitterInitializeCallback)(EmitterInitializeArg& rArg);
typedef void (*EmitterPreCalculateCallback)(EmitterPostCalculateArg& rArg);
typedef void (*EmitterPostCalculateCallback)(EmitterPostCalculateArg& rArg);
typedef bool (*EmitterDrawCallback)(EmitterDrawArg& rArg);
typedef void (*EmitterFinalizeCallback)(EmitterFinalizeArg& rArg);
typedef bool (*ParticleEmitCallback)(ParticleCalculateArgImpl& rArg);
typedef bool (*ParticleRemoveCallback)(ParticleCalculateArgImpl& rArg);
typedef void (*ParticleCalculateCallback)(ParticleCalculateArgImpl& rArg);
typedef bool (*RenderStateSetCallback)(RenderStateSetArg& rArg);
typedef void (*DrawPathRenderStateSetCallback)(RenderStateSetArg& rArg);
typedef bool (*CustomFieldCallback)(util::neon::Vector3fType* pPos, util::neon::Vector3fType* pVel,
                                    f32* pTime, f32* pLife, Emitter* pEmitter,
                                    const detail::ParticleProperty* pProperty,
                                    const detail::ResFieldCustom* pField, int particleIndex);

void EndianFlipCallbackImpl(EndianFlipArg& rArg);
void BindReservedCustomShaderConstantBuffer(RenderStateSetArg& rArg);

struct CallbackSet {
    CallbackSet()
        : endianFlip(nullptr), emitterInitialize(nullptr), emitterPreCalculate(nullptr),
          _18(nullptr), emitterPostCalculate(nullptr), emitterDraw(nullptr),
          emitterFinalize(nullptr), particleEmit(nullptr), particleRemove(nullptr),
          particleCalculate(nullptr), renderStateSet(nullptr) {}

    EndianFlipCallback endianFlip;
    EmitterInitializeCallback emitterInitialize;
    EmitterPreCalculateCallback emitterPreCalculate;
    void* _18;
    EmitterPostCalculateCallback emitterPostCalculate;
    EmitterDrawCallback emitterDraw;
    EmitterFinalizeCallback emitterFinalize;
    ParticleEmitCallback particleEmit;
    ParticleRemoveCallback particleRemove;
    ParticleCalculateCallback particleCalculate;
    RenderStateSetCallback renderStateSet;
};

static_assert(sizeof(CallbackSet) == 0x58);

namespace detail {

class Shader {
public:
    void BindCustomShaderUniformBlock(gfx::CommandBuffer* pCommandBuffer,
                                      CustomShaderConstantBufferIndex index,
                                      gfx::GpuAddress* pAddress, size_t size);

    s32 GetCustomTexturePixelLocation(CustomShaderTextureType index) const {
        return m_CustomTextureLocation[index][0];
    }

    s32 GetCustomTextureVertexLocation(CustomShaderTextureType index) const {
        return m_CustomTextureLocation[index][1];
    }

    s32 GetCustomTexturePixelLocationByIndex(s32 index) const {
        return m_CustomTextureLocation[index][0];
    }

    s32 GetCustomTextureVertexLocationByIndex(s32 index) const {
        return m_CustomTextureLocation[index][1];
    }

    const gfx::Shader* GetGfxShader() const { return m_pGfxShader; }

    u8 _0[0x8];
    gfx::Shader* m_pGfxShader;
    u8 _10[0x14 - 0x10];
    s32 m_CustomTextureLocation[32][2];
};

/** Compute shader of a GPU stream-out emitter. */
class ComputeShader {
public:
    s32 GetViewParamLocation() const { return m_ViewParamLocation; }

    u8 _0[0x8];
    s32 m_ViewParamLocation;
};

class EmitterCalculator {
public:
    explicit EmitterCalculator(System* pSystem);
    ~EmitterCalculator();

    void CalculateComputeShader(gfx::CommandBuffer* pCommandBuffer, Emitter* pEmitter,
                                const ComputeShader* pComputeShader, int bufferIndex,
                                bool isDoComputeShaderProcess, void* pUserParam, bool isBatch);
    void Draw(gfx::CommandBuffer* pCommandBuffer, Emitter* pEmitter, void* pUserParam,
              DrawParameterArg* pDrawParameterArg);

    void DrawEmitterUsingBoundShader(gfx::CommandBuffer* pCommandBuffer, Emitter* pEmitter,
                                     Shader* pShader, void* pUserParam,
                                     DrawParameterArg* pDrawParameterArg);

    u8 _0[0x60];
};

}  // namespace detail

}  // namespace vfx
}  // namespace nn
