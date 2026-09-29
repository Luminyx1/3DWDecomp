#pragma once

#include "nn/gfx/gfx_Buffer.h"
#include "nn/gfx/gfx_DescriptorSlot.h"
#include "nn/gfx/gfx_GpuAddress.h"
#include "nn/gfx/gfx_MemoryPool.h"
#include "nn/gfx/gfx_ResShader.h"
#include "nn/gfx/gfx_Sampler.h"
#include "nn/gfx/gfx_State.h"
#include "nn/gfx/gfx_Types.h"

namespace nn {
namespace font {
class RectDrawer {
public:
    virtual ~RectDrawer();

    virtual bool Initialize(nn::gfx::Device*, void*, uint32_t, nn::gfx::MemoryPool*, ptrdiff_t,
                            size_t);
    virtual void Finalize(nn::gfx::Device*);

    nn::gfx::ResShaderFile* m_pResShaderFile;
    nn::gfx::ShaderCodeType m_CodeType;
    int m_VertexShaderSlots[6];
    int m_VertexShaderPerCharacterParamsSlots[6];
    int m_PixelShaderSlots[6];
    int m_BlackWhiteInterpolationSlots[6];
    int m_TextureSlots[6];
    int32_t m_CharCountMax;
    nn::gfx::VertexState m_VertexStates[6];
    nn::gfx::MemoryPool m_MemoryPoolForBuffers;
    nn::gfx::Buffer m_VertexBuffer;
    nn::gfx::Buffer m_IndexBuffer;
    nn::gfx::Buffer m_ShaderParamBlackWhiteInterpolationEnabledBuffer;
    nn::gfx::Buffer m_ShaderParamBlackWhiteInterpolationDisabledBuffer;
    nn::gfx::Sampler m_Sampler;
    nn::gfx::DescriptorSlot m_DescriptorSlotForSampler;
    void* m_WorkMemory;
};
};  // namespace font
};  // namespace nn