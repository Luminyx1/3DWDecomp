#include <nn/ui2d/ui2d_ShaderInfo.h>
#include <nn/gfx/gfx_CommandBuffer.h>
namespace nn::ui2d {
// count is the number of texture descriptor slots used by each variation.
void ShaderInfo::SetTextureSlotCount(int count) { m_Flags = (m_Flags & ~0xf0u) | (count << 4); }
// type selects the source, intermediate, or binary shader program.
void ShaderInfo::SetShaderCodeType(nn::gfx::ShaderCodeType type) { m_Flags = (m_Flags & ~7u) | type; }
int ShaderInfo::GetVariationCount() const { return m_pResShaderFile->GetShaderContainer()->GetShaderVariationCount(); }
// index selects the shader variation.
const nn::gfx::Shader* ShaderInfo::GetVertexShader(int index) const {
    return m_pResShaderFile->GetShaderContainer()->GetResShaderVariation(index)->GetResShaderProgram(GetShaderCodeType())->GetShader();
}
// index selects the shader variation.
const nn::gfx::Shader* ShaderInfo::GetGeometryShader(int index) const {
    return m_pResShaderFile->GetShaderContainer()->GetResShaderVariation(index)->GetResShaderProgram(GetShaderCodeType())->GetShader();
}
// index selects the shader variation.
const nn::gfx::Shader* ShaderInfo::GetPixelShader(int index) const {
    return m_pResShaderFile->GetShaderContainer()->GetResShaderVariation(index)->GetResShaderProgram(GetShaderCodeType())->GetShader();
}
nn::gfx::ShaderCodeType ShaderInfo::GetShaderCodeType() const { return static_cast<nn::gfx::ShaderCodeType>(m_Flags & 7); }
// commandBuffer receives shader and vertex-input state for variation index.
void ShaderInfo::SetShader(nn::gfx::CommandBuffer& commandBuffer, int index) const {
    commandBuffer.SetShader(GetVertexShader(index), 0x3f);
    commandBuffer.SetVertexState(&m_pVertexStates[index]);
}
int ShaderInfo::GetTextureSlotCount() const { return (m_Flags >> 4) & 15; }
}
