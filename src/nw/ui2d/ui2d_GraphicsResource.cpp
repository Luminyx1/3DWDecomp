#include <nn/ui2d/ui2d_GraphicsResource.h>
#include <nn/gfx/gfx_CommandBuffer.h>
namespace nn::ui2d {
// id selects one of the built-in blend states.
nn::gfx::BlendState* GraphicsResource::GetPresetBlendState(PresetBlendStateId id) { return &m_PresetBlendState[static_cast<u32>(id)]; }
// commandBuffer binds the shared four-corner vertex buffer at stream zero.
void GraphicsResource::ActivateVertexBuffer(nn::gfx::CommandBuffer* commandBuffer) const {
    commandBuffer->SetVertexBuffer(0, m_VertexBufferGpuAddress, 8, 32);
}
// wrapS and wrapT select wrapping; minFilter and magFilter select texture filtering.
nn::gfx::DescriptorSlot& GraphicsResource::GetSamplerDescriptorSlot(TexWrap wrapS, TexWrap wrapT, TexFilter minFilter, TexFilter magFilter) const {
    return m_pSamplerDescriptorSlotTable[12 * static_cast<int>(wrapS) + 4 * static_cast<int>(wrapT) + static_cast<int>(minFilter) + 2 * static_cast<int>(magFilter)];
}
// id selects a preset sampler descriptor.
nn::gfx::DescriptorSlot& GraphicsResource::GetSamplerDescriptorSlot(PresetSamplerId id) const { return m_pSamplerDescriptorSlotTable[static_cast<int>(id)]; }
}
