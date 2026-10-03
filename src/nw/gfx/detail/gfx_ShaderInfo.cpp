#include <nn/gfx/gfx_ShaderInfo.h>

namespace nn::gfx {

namespace {

/// Member pointers to the shader code of each ShaderStage.
static detail::Ptr<const void> ShaderInfoData::*const g_pShaderCodes[] = {
    &ShaderInfoData::pVertexShaderCode, &ShaderInfoData::pHullShaderCode,
    &ShaderInfoData::pDomainShaderCode, &ShaderInfoData::pGeometryShaderCode,
    &ShaderInfoData::pPixelShaderCode,  &ShaderInfoData::pComputeShaderCode,
};

}  // namespace

/**
 * Resets the info to a non-separable GLSL source shader with no code.
 */
void ShaderInfo::SetDefault() {
    SetSeparationEnabled(false);
    SetCodeType(ShaderCodeType_Source);
    SetSourceFormat(ShaderSourceFormat_Glsl);
    SetBinaryFormat(0);
    SetShaderCodePtr(ShaderStage_Vertex, nullptr);
    SetShaderCodePtr(ShaderStage_Hull, nullptr);
    SetShaderCodePtr(ShaderStage_Domain, nullptr);
    SetShaderCodePtr(ShaderStage_Geometry, nullptr);
    SetShaderCodePtr(ShaderStage_Pixel, nullptr);
    SetShaderCodePtr(ShaderStage_Compute, nullptr);
}

/**
 * Sets the code of one shader stage.
 *
 * @param shaderStage Shader stage.
 * @param pCode Shader code.
 */
void ShaderInfo::SetShaderCodePtr(ShaderStage shaderStage, const void* pCode) {
    this->*g_pShaderCodes[shaderStage] = pCode;
}

/**
 * Returns the code of one shader stage.
 *
 * @param shaderStage Shader stage.
 * @return Shader code.
 */
const void* ShaderInfo::GetShaderCodePtr(ShaderStage shaderStage) const {
    return this->*g_pShaderCodes[shaderStage];
}

}  // namespace nn::gfx
