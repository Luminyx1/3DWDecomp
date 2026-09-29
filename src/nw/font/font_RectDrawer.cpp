#include <nn/font/font_RectDrawer.h>

#include <cstring>
#include <nn/font/font_Font.h>
#include <nn/font/font_Util.h>
#include <nn/gfx/gfx_BufferInfo.h>
#include <nn/gfx/gfx_MemoryPoolInfo.h>
#include <nn/gfx/gfx_StateInfo.h>
#include <nn/util.h>
#include <nn/util/util_BitUtil.h>

namespace nn {
namespace font {

extern const uint8_t g_RectDrawerShaderBinary[] __attribute__((visibility("hidden")));

namespace {

const size_t ShaderBinarySize = 0x8438;

struct Vertex {
    float position[3];
    uint32_t color;
    float texCoord[3];
};

const int VertexCount = 4;
const int IndexCount = 6;

/**
 * Gets the memory size required by one vertex state.
 * @return required size
 */
size_t GetVertexStateRequiredMemorySize() {
    nn::gfx::VertexStateInfo info;
    info.SetDefault();
    nn::gfx::VertexAttributeStateInfo attribute;
    attribute.SetDefault();
    attribute.SetNamePtr("aVertex");
    attribute.SetBufferIndex(0);
    attribute.SetFormat(nn::gfx::AttributeFormat_32_32_32_Float);
    attribute.SetOffset(0);
    nn::gfx::VertexBufferStateInfo buffer;
    buffer.SetDefault();
    buffer.SetStride(sizeof(Vertex));
    info.SetVertexAttributeStateInfoArray(&attribute, 1);
    info.SetVertexBufferStateInfoArray(&buffer, 1);
    return nn::gfx::VertexState::GetRequiredMemorySize(info);
}

/**
 * Gets the buffer alignment for a GPU access type.
 * @param pDevice gfx device
 * @param gpuAccessFlags GPU access flags
 * @return alignment
 */
size_t GetBufferAlignment(nn::gfx::Device* pDevice, int gpuAccessFlags) {
    nn::gfx::BufferInfo info;
    info.SetDefault();
    info.SetGpuAccessFlags(gpuAccessFlags);
    return nn::gfx::Buffer::GetBufferAlignment(pDevice, info);
}

}  // namespace

size_t RectDrawer::GetWorkBufferAlignment() {
    return nn::gfx::ResShaderFile::ToAccessor(
               reinterpret_cast<const nn::gfx::ResShaderFileData*>(g_RectDrawerShaderBinary))
        ->GetBinaryFileHeader()
        ->GetAlignment();
}

size_t RectDrawer::GetWorkBufferSize(nn::gfx::Device* pDevice, uint32_t charCount) {
    size_t vertexStateSize = nn::util::align_up(GetVertexStateRequiredMemorySize(), 8);

    nn::gfx::MemoryPoolInfo info;
    info.SetDefault();
    size_t size = nn::util::align_up(ShaderBinarySize + vertexStateSize * ShaderVariationCount,
                                     nn::gfx::MemoryPool::GetPoolMemoryAlignment(pDevice, info));
    return size + CalculateMemoryPoolSize(pDevice, charCount);
}

/**
 * Calculates the memory pool size needed by the buffers.
 * @param pDevice gfx device
 * @param charCount maximum number of characters
 * @return memory pool size
 */
size_t RectDrawer::CalculateMemoryPoolSize(nn::gfx::Device* pDevice, uint32_t charCount) {
    size_t size = nn::util::align_up(sizeof(Vertex) * VertexCount,
                                     GetBufferAlignment(pDevice, nn::gfx::GpuAccess_IndexBuffer));
    size = nn::util::align_up(size + sizeof(uint16_t) * IndexCount,
                              GetBufferAlignment(pDevice, nn::gfx::GpuAccess_ConstantBuffer));
    size = nn::util::align_up(size + sizeof(uint32_t),
                              GetBufferAlignment(pDevice, nn::gfx::GpuAccess_ConstantBuffer));
    size += sizeof(uint32_t);

    nn::gfx::MemoryPoolInfo info;
    info.SetDefault();
    return nn::util::align_up(size,
                              nn::gfx::MemoryPool::GetPoolMemorySizeGranularity(pDevice, info));
}

/**
 * Calculates the memory pool alignment needed by the buffers.
 * @param pDevice gfx device
 * @return alignment
 */
size_t RectDrawer::CalculateMemoryPoolAlignment(nn::gfx::Device* pDevice) {
    return GetBufferAlignment(pDevice, nn::gfx::GpuAccess_VertexBuffer);
}

/**
 * Constructs an uninitialized drawer.
 */
RectDrawer::RectDrawer()
    : m_pResShaderFile(nullptr), m_CodeType(nn::gfx::ShaderCodeType_Binary), m_CharCountMax(0),
      m_WorkMemory(nullptr) {}

/**
 * Destroys the drawer.
 */
RectDrawer::~RectDrawer() {}

/**
 * Finalizes the gfx objects and shaders.
 * @param pDevice gfx device
 */
void RectDrawer::Finalize(nn::gfx::Device* pDevice) {
    if (m_CharCountMax < 1) {
        return;
    }

    FinalizeIfNecessary(m_ShaderParamBlackWhiteInterpolationDisabledBuffer, pDevice);
    FinalizeIfNecessary(m_ShaderParamBlackWhiteInterpolationEnabledBuffer, pDevice);
    FinalizeIfNecessary(m_VertexBuffer, pDevice);
    FinalizeIfNecessary(m_IndexBuffer, pDevice);
    FinalizeIfNecessary(m_MemoryPoolForBuffers, pDevice);
    FinalizeIfNecessary(m_Sampler, pDevice);
    for (int i = 0; i < ShaderVariationCount; i++) {
        FinalizeIfNecessary(m_VertexStates[i], pDevice);
    }

    nn::gfx::ResShaderContainer* pContainer = m_pResShaderFile->GetShaderContainer();
    for (int i = 0; i < ShaderVariationCount; i++) {
        pContainer->GetResShaderVariation(i)->GetResShaderProgram(m_CodeType)->Finalize(pDevice);
    }
    pContainer->Finalize(pDevice);

    m_CharCountMax = 0;
}

/**
 * Registers the sampler to a descriptor pool.
 * @param pRegisterSamplerSlot callback registering the sampler
 * @param pUserData user data passed to the callback
 */
void RectDrawer::RegisterSamplerToDescriptorPool(RegisterSamplerSlot pRegisterSamplerSlot,
                                                 void* pUserData) {
    pRegisterSamplerSlot(&m_DescriptorSlotForSampler, m_Sampler, pUserData);
}

/**
 * Unregisters the sampler from its descriptor pool.
 * @param pUnregisterSamplerSlot callback unregistering the sampler
 * @param pUserData user data passed to the callback
 */
void RectDrawer::UnregisterSamplerFromDescriptorPool(UnregisterSamplerSlot pUnregisterSamplerSlot,
                                                     void* pUserData) {
    pUnregisterSamplerSlot(&m_DescriptorSlotForSampler, m_Sampler, pUserData);
    m_DescriptorSlotForSampler.Invalidate();
}

/**
 * Acquires a descriptor slot for the common sampler.
 * @param pAcquireSamplerSlot callback acquiring a sampler slot
 * @param pUserData user data passed to the callback
 */
void RectDrawer::AcquireCommonSamplerSlot(AcquireSamplerSlot pAcquireSamplerSlot,
                                          void* pUserData) {
    nn::gfx::SamplerInfo info;
    info.SetDefault();
    info.SetAddressU(nn::gfx::TextureAddressMode_Repeat);
    info.SetAddressV(nn::gfx::TextureAddressMode_Repeat);
    info.SetFilterMode(nn::gfx::FilterMode_MinLinear_MagLinear_MipPoint);
    pAcquireSamplerSlot(&m_DescriptorSlotForSampler, info, pUserData);
}

/**
 * Releases the descriptor slot of the common sampler.
 * @param pReleaseSamplerSlot callback releasing the sampler slot
 * @param pUserData user data passed to the callback
 */
void RectDrawer::ReleaseCommonSamplerSlot(ReleaseSamplerSlot pReleaseSamplerSlot,
                                          void* pUserData) {
    nn::gfx::SamplerInfo info;
    info.SetDefault();
    info.SetAddressU(nn::gfx::TextureAddressMode_Repeat);
    info.SetAddressV(nn::gfx::TextureAddressMode_Repeat);
    info.SetFilterMode(nn::gfx::FilterMode_MinLinear_MagLinear_MipPoint);
    pReleaseSamplerSlot(&m_DescriptorSlotForSampler, info, pUserData);
    m_DescriptorSlotForSampler.Invalidate();
}

/**
 * Records the draw commands for a display string buffer.
 * @param rCommandBuffer command buffer to record into
 * @param rBuffer display string buffer to draw
 */
void RectDrawer::Draw(nn::gfx::CommandBuffer& rCommandBuffer,
                      const DispStringBuffer& rBuffer) const {
    if (rBuffer.m_CharCountMax < 1) {
        return;
    }
    if (rBuffer.m_pConstantBuffer->IsUnallocated()) {
        return;
    }
    if (m_CharCountMax < rBuffer.m_CharCount) {
        return;
    }

    const DispStringBuffer::VertexBufferData& rVertexBufferData = rBuffer.m_VertexBufferData;
    nn::gfx::GpuAddress constantBufferAddress;
    constantBufferAddress = rBuffer.m_pConstantBuffer->GetGpuAddress();
    constantBufferAddress.Offset(rBuffer.m_ConstantBufferOffset);
    nn::gfx::GpuAddress perCharacterParamsAddress = rBuffer.m_pConstantBuffer->GetGpuAddress();
    perCharacterParamsAddress.Offset(rBuffer.m_PerCharacterParamOffset);

    uint32_t flags = rBuffer.m_ShaderVariationFlags;
    if (rBuffer.m_IsDoubleDrawnBorder) {
        AddDrawCommand(rCommandBuffer, rVertexBufferData,
                       flags & ~DispStringBuffer::ShaderVariationFlag_BorderPass,
                       constantBufferAddress, perCharacterParamsAddress);
        constantBufferAddress.Offset(nn::util::align_up(
            sizeof(DispStringBuffer::ShaderParam),
            rBuffer.m_pConstantBuffer->GetBufferAlignment()));
        AddDrawCommand(rCommandBuffer, rVertexBufferData,
                       flags | DispStringBuffer::ShaderVariationFlag_BorderPass,
                       constantBufferAddress, perCharacterParamsAddress);
    } else {
        AddDrawCommand(rCommandBuffer, rVertexBufferData, flags, constantBufferAddress,
                       perCharacterParamsAddress);
    }
}

/**
 * Records the draw commands for each texture used by the buffer.
 * @param rCommandBuffer command buffer to record into
 * @param rVertexBufferData texture use information of the buffer
 * @param shaderVariationFlags shader variation flags
 * @param rConstantBufferAddress address of the shader parameters
 * @param rPerCharacterParamsAddress address of the per-character parameters
 */
void RectDrawer::AddDrawCommand(nn::gfx::CommandBuffer& rCommandBuffer,
                                const DispStringBuffer::VertexBufferData& rVertexBufferData,
                                uint32_t shaderVariationFlags,
                                const nn::gfx::GpuAddress& rConstantBufferAddress,
                                const nn::gfx::GpuAddress& rPerCharacterParamsAddress) const {
    const uint32_t count = rVertexBufferData.textureUseInfoCount;
    nn::gfx::GpuAddress perCharacterParamsAddress = rPerCharacterParamsAddress;

    int currentVariation = 7;
    for (uint32_t i = 0; i < count; i++) {
        const DispStringBuffer::TextureUseInfo& rInfo = rVertexBufferData.textureUseInfos[i];
        if (rInfo.useCount == 0) {
            continue;
        }

        int variation = 0;
        if (shaderVariationFlags & DispStringBuffer::ShaderVariationFlag_PerCharacterTransform) {
            variation = 3;
            if (rInfo.flags & 2) {
                variation = 4 | (shaderVariationFlags & 1);
            }
        } else if (rInfo.flags & 2) {
            variation =
                (shaderVariationFlags & DispStringBuffer::ShaderVariationFlag_BorderPass) ? 2 : 1;
        }
        if (variation != currentVariation) {
            rCommandBuffer.SetShader(GetVertexShader(variation), nn::gfx::ShaderStageBit_All);
            rCommandBuffer.SetVertexState(&m_VertexStates[variation]);

            nn::gfx::GpuAddress gpuAddress;
            m_VertexBuffer.GetGpuAddress(&gpuAddress);
            rCommandBuffer.SetVertexBuffer(m_VertexShaderSlots[variation], gpuAddress,
                                           sizeof(Vertex), sizeof(Vertex) * VertexCount);
            rCommandBuffer.SetConstantBuffer(m_VertexShaderSlots[variation],
                                             nn::gfx::ShaderStage_Vertex, rConstantBufferAddress,
                                             sizeof(DispStringBuffer::ShaderParam));
            rCommandBuffer.SetConstantBuffer(m_PixelShaderSlots[variation],
                                             nn::gfx::ShaderStage_Pixel, rConstantBufferAddress,
                                             sizeof(DispStringBuffer::ShaderParam));
            currentVariation = variation;
        }

        const size_t perCharacterParamSize =
            (shaderVariationFlags & DispStringBuffer::ShaderVariationFlag_PerCharacterTransform) ?
                sizeof(detail::VertexShaderCharAttributeWithTransform) :
                sizeof(detail::VertexShaderCharAttribute);
        const int shadowShift = (shaderVariationFlags >> 1) & 1;
        size_t size = (perCharacterParamSize * rInfo.useCount) << shadowShift;
        rCommandBuffer.SetUnorderedAccessBuffer(m_VertexShaderPerCharacterParamsSlots[variation],
                                                nn::gfx::ShaderStage_Vertex,
                                                perCharacterParamsAddress, size);
        perCharacterParamsAddress.Offset(size);

        rCommandBuffer.SetTextureAndSampler(m_TextureSlots[variation], nn::gfx::ShaderStage_Pixel,
                                            rInfo.pTexObj->GetDescriptorSlot(),
                                            m_DescriptorSlotForSampler);

        {
            nn::gfx::GpuAddress gpuAddress;
            if (rInfo.flags & 1) {
                m_ShaderParamBlackWhiteInterpolationEnabledBuffer.GetGpuAddress(&gpuAddress);
            } else {
                m_ShaderParamBlackWhiteInterpolationDisabledBuffer.GetGpuAddress(&gpuAddress);
            }
            rCommandBuffer.SetConstantBuffer(m_BlackWhiteInterpolationSlots[variation],
                                             nn::gfx::ShaderStage_Pixel, gpuAddress,
                                             sizeof(uint32_t));
        }

        {
            nn::gfx::GpuAddress gpuAddress;
            m_IndexBuffer.GetGpuAddress(&gpuAddress);
            rCommandBuffer.DrawIndexed(nn::gfx::PrimitiveTopology_TriangleList,
                                       nn::gfx::IndexFormat_Uint16, gpuAddress, IndexCount, 0,
                                       rInfo.useCount << shadowShift, 0);
        }
    }
}

/**
 * Fills the index array for a number of character quads.
 * @param pIndices destination index array
 * @param charCount maximum number of characters
 */
void RectDrawer::CreateIndices(uint16_t* pIndices, uint32_t charCount) {
    for (uint32_t i = 0; i < charCount; i++) {
        uint32_t index = i * IndexCount;
        uint16_t vertex = i * VertexCount;
        pIndices[index + 0] = vertex + 0;
        pIndices[index + 1] = vertex + 2;
        pIndices[index + 2] = vertex + 3;
        pIndices[index + 3] = vertex + 3;
        pIndices[index + 4] = vertex + 1;
        pIndices[index + 5] = vertex + 0;
    }
}

bool RectDrawer::Initialize(nn::gfx::Device* pDevice, void* pWorkMemory, uint32_t charCount,
                            nn::gfx::MemoryPool* pMemoryPool, ptrdiff_t memoryPoolOffset,
                            size_t memoryPoolSize) {
    nn::util::ReferSymbol("SDK MW+Nintendo+NintendoWare_Font-10_4_0-Release");

    if (m_CharCountMax != 0) {
        return false;
    }

    m_WorkMemory = pWorkMemory;

    size_t alignment = GetWorkBufferAlignment();
    nn::util::BytePtr workMemory(pWorkMemory);
    workMemory.AlignUp(alignment);
    void* pShaderBinary = workMemory.Get();
    std::memcpy(pShaderBinary, g_RectDrawerShaderBinary, ShaderBinarySize);
    m_pResShaderFile = nn::gfx::ResShaderFile::ResCast(pShaderBinary);

    nn::gfx::ResShaderContainer* pContainer = m_pResShaderFile->GetShaderContainer();
    pContainer->Initialize(pDevice);

    nn::gfx::ResShaderVariation* pVariation = pContainer->GetResShaderVariation(0);
    bool isInitialized = false;
    m_CodeType = nn::gfx::ShaderCodeType_Binary;
    nn::gfx::ResShaderProgram* pProgram =
        pVariation->GetResShaderProgram(nn::gfx::ShaderCodeType_Binary);
    if (pProgram != nullptr) {
        isInitialized =
            pProgram->Initialize(pDevice) == nn::gfx::ShaderInitializeResult_Success;
    }
    if (!isInitialized) {
        pProgram = pVariation->GetResShaderProgram(nn::gfx::ShaderCodeType_Ir);
        if (pProgram != nullptr) {
            m_CodeType = nn::gfx::ShaderCodeType_Ir;
            isInitialized =
                pProgram->Initialize(pDevice) == nn::gfx::ShaderInitializeResult_Success;
        }
    }
    if (!isInitialized) {
        pProgram = pVariation->GetResShaderProgram(nn::gfx::ShaderCodeType_Source);
        m_CodeType = nn::gfx::ShaderCodeType_Source;
        pProgram->Initialize(pDevice);
    }
    workMemory.Advance(ShaderBinarySize);

    for (int i = 1; i < ShaderVariationCount; i++) {
        pContainer->GetResShaderVariation(i)->GetResShaderProgram(m_CodeType)->Initialize(pDevice);
    }

    for (int i = 0; i < ShaderVariationCount; i++) {
        const nn::gfx::Shader* pVertexShader = GetVertexShader(i);
        const nn::gfx::Shader* pPixelShader = GetPixelShader(i);
        m_VertexShaderSlots[i] = pVertexShader->GetInterfaceSlot(
            nn::gfx::ShaderStage_Vertex, nn::gfx::ShaderInterfaceType_ConstantBuffer,
            "ShaderParam");
        m_VertexShaderPerCharacterParamsSlots[i] = pVertexShader->GetInterfaceSlot(
            nn::gfx::ShaderStage_Vertex, nn::gfx::ShaderInterfaceType_UnorderedAccessBuffer,
            "PerCharacterParamBlock");
        m_PixelShaderSlots[i] = pPixelShader->GetInterfaceSlot(
            nn::gfx::ShaderStage_Pixel, nn::gfx::ShaderInterfaceType_ConstantBuffer,
            "ShaderParam");
        m_BlackWhiteInterpolationSlots[i] = pPixelShader->GetInterfaceSlot(
            nn::gfx::ShaderStage_Pixel, nn::gfx::ShaderInterfaceType_ConstantBuffer,
            "ShaderParamBlackWhiteInterpolation");
        m_TextureSlots[i] = pPixelShader->GetInterfaceSlot(
            nn::gfx::ShaderStage_Pixel, nn::gfx::ShaderInterfaceType_Sampler, "uTextureSrc");
    }

    {
        nn::gfx::VertexStateInfo info;
        info.SetDefault();
        nn::gfx::VertexAttributeStateInfo attribute;
        attribute.SetDefault();
        attribute.SetNamePtr("aVertex");
        attribute.SetBufferIndex(0);
        attribute.SetFormat(nn::gfx::AttributeFormat_32_32_32_Float);
        attribute.SetOffset(0);
        nn::gfx::VertexBufferStateInfo buffer;
        buffer.SetDefault();
        buffer.SetStride(sizeof(Vertex));
        info.SetVertexAttributeStateInfoArray(&attribute, 1);
        info.SetVertexBufferStateInfoArray(&buffer, 1);

        size_t memorySize = nn::gfx::VertexState::GetRequiredMemorySize(info);
        for (int i = 0; i < ShaderVariationCount; i++) {
            workMemory.AlignUp(8);
            m_VertexStates[i].SetMemory(workMemory.Get(), memorySize);
            m_VertexStates[i].Initialize(pDevice, info, GetVertexShader(i));
            workMemory.Advance(memorySize);
        }
    }

    {
        nn::gfx::SamplerInfo info;
        info.SetDefault();
        info.SetAddressU(nn::gfx::TextureAddressMode_Repeat);
        info.SetAddressV(nn::gfx::TextureAddressMode_Repeat);
        info.SetFilterMode(nn::gfx::FilterMode_MinLinear_MagLinear_MipPoint);
        m_Sampler.Initialize(pDevice, info);
        m_DescriptorSlotForSampler.Invalidate();
    }

    nn::gfx::MemoryPoolInfo memoryPoolInfo;
    memoryPoolInfo.SetDefault();
    size_t poolAlignment = nn::gfx::MemoryPool::GetPoolMemoryAlignment(pDevice, memoryPoolInfo);
    size_t poolGranularity =
        nn::gfx::MemoryPool::GetPoolMemorySizeGranularity(pDevice, memoryPoolInfo);

    ptrdiff_t offset = pMemoryPool != nullptr ? memoryPoolOffset : 0;
    ptrdiff_t vertexBufferOffset = offset;
    ptrdiff_t indexBufferOffset =
        nn::util::align_up(vertexBufferOffset + sizeof(Vertex) * VertexCount,
                           GetBufferAlignment(pDevice, nn::gfx::GpuAccess_IndexBuffer));
    ptrdiff_t enabledBufferOffset =
        nn::util::align_up(indexBufferOffset + sizeof(uint16_t) * IndexCount,
                           GetBufferAlignment(pDevice, nn::gfx::GpuAccess_ConstantBuffer));
    ptrdiff_t disabledBufferOffset =
        nn::util::align_up(enabledBufferOffset + sizeof(uint32_t),
                           GetBufferAlignment(pDevice, nn::gfx::GpuAccess_ConstantBuffer));

    if (pMemoryPool == nullptr) {
        const size_t poolSize = nn::util::align_up(
            disabledBufferOffset + sizeof(uint32_t) - memoryPoolOffset, poolGranularity);
        void* pPoolMemory = workMemory.AlignUp(poolAlignment).Get();
        memoryPoolInfo.SetMemoryPoolProperty(nn::gfx::MemoryPoolProperty_CpuUncached |
                                             nn::gfx::MemoryPoolProperty_GpuCached);
        memoryPoolInfo.SetPoolMemory(pPoolMemory, poolSize);
        m_MemoryPoolForBuffers.Initialize(pDevice, memoryPoolInfo);
        pMemoryPool = &m_MemoryPoolForBuffers;
    }

    {
        nn::gfx::BufferInfo info;
        info.SetDefault();
        info.SetSize(sizeof(Vertex) * VertexCount);
        info.SetGpuAccessFlags(nn::gfx::GpuAccess_VertexBuffer);
        m_VertexBuffer.Initialize(pDevice, info, pMemoryPool, vertexBufferOffset,
                                  info.GetSize());
        Vertex* pVertices = m_VertexBuffer.Map<Vertex>();
        for (int i = 0; i < VertexCount; i++) {
            pVertices[i].position[0] = static_cast<float>(i & 1);
            pVertices[i].position[1] = static_cast<float>(i >> 1);
            pVertices[i].position[2] = 0.0f;
            pVertices[i].color = 0xffffffff;
            pVertices[i].texCoord[0] = static_cast<float>(i & 1);
            pVertices[i].texCoord[1] = static_cast<float>(1 - (i >> 1));
            pVertices[i].texCoord[2] = 0.0f;
        }
        m_VertexBuffer.Unmap();
    }

    {
        nn::gfx::BufferInfo info;
        info.SetDefault();
        info.SetSize(sizeof(uint16_t) * IndexCount);
        info.SetGpuAccessFlags(nn::gfx::GpuAccess_IndexBuffer);
        m_IndexBuffer.Initialize(pDevice, info, pMemoryPool, indexBufferOffset, info.GetSize());
        CreateIndices(m_IndexBuffer.Map<uint16_t>(), 1);
        m_IndexBuffer.Unmap();
    }

    {
        nn::gfx::BufferInfo info;
        info.SetDefault();
        info.SetSize(sizeof(uint32_t));
        info.SetGpuAccessFlags(nn::gfx::GpuAccess_ConstantBuffer);
        m_ShaderParamBlackWhiteInterpolationEnabledBuffer.Initialize(
            pDevice, info, pMemoryPool, enabledBufferOffset, info.GetSize());
        uint32_t* pParam = m_ShaderParamBlackWhiteInterpolationEnabledBuffer.Map<uint32_t>();
        m_ShaderParamBlackWhiteInterpolationEnabledBuffer.InvalidateMappedRange(0,
                                                                                info.GetSize());
        *pParam = 1;
        m_ShaderParamBlackWhiteInterpolationEnabledBuffer.Unmap();
    }

    {
        nn::gfx::BufferInfo info;
        info.SetDefault();
        info.SetSize(sizeof(uint32_t));
        info.SetGpuAccessFlags(nn::gfx::GpuAccess_ConstantBuffer);
        m_ShaderParamBlackWhiteInterpolationDisabledBuffer.Initialize(
            pDevice, info, pMemoryPool, disabledBufferOffset, info.GetSize());
        uint32_t* pParam = m_ShaderParamBlackWhiteInterpolationDisabledBuffer.Map<uint32_t>();
        m_ShaderParamBlackWhiteInterpolationDisabledBuffer.InvalidateMappedRange(0,
                                                                                 info.GetSize());
        *pParam = 0;
        m_ShaderParamBlackWhiteInterpolationDisabledBuffer.Unmap();
    }

    m_CharCountMax = charCount;
    return true;
}

/**
 * Gets the vertex shader of a variation.
 * @param variation shader variation index
 * @return vertex shader
 */
const nn::gfx::Shader* RectDrawer::GetVertexShader(int variation) const {
    return GetResShaderProgram(variation)->GetShader();
}

/**
 * Gets the pixel shader of a variation.
 * @param variation shader variation index
 * @return pixel shader
 */
const nn::gfx::Shader* RectDrawer::GetPixelShader(int variation) const {
    return GetResShaderProgram(variation)->GetShader();
}

}  // namespace font
}  // namespace nn
