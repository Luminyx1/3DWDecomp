#include <nn/gfx/gfx_StateInfo.h>

namespace nn::gfx {

/**
 * Resets the multisample state to its defaults: alpha-to-coverage disabled, a single sample and
 * every sample enabled in the mask.
 */
void MultisampleStateInfo::SetDefault() {
    SetAlphaToCoverageEnabled(false);
    SetSampleCount(1);
    SetSampleMask(0xFFFFFFFF);
}

/**
 * Resets the rasterizer state to its defaults: solid fill, counter-clockwise front faces, back-face
 * culling, triangle topology, rasterization and depth clipping enabled, no depth bias and a
 * default multisample state.
 */
void RasterizerStateInfo::SetDefault() {
    SetFillMode(FillMode_Solid);
    SetFrontFace(FrontFace_Ccw);
    SetCullMode(CullMode_Back);
    SetPrimitiveTopologyType(PrimitiveTopologyType_Triangle);

    SetRasterEnabled(true);
    SetMultisampleEnabled(false);
    SetDepthClipEnabled(true);
    SetScissorEnabled(false);

    SetDepthBias(0);
    SetDepthBiasClamp(0.0f);
    SetSlopeScaledDepthBias(0.0f);

    SetConservativeRasterizationMode(ConservativeRasterizationMode_Disable);

    EditMultisampleStateInfo().SetDefault();
}

/**
 * Resets the blend target state to its defaults: blending disabled, an additive one/zero blend
 * equation for both color and alpha, and all channels writable.
 */
void BlendTargetStateInfo::SetDefault() {
    SetBlendEnabled(false);
    SetSourceColorBlendFactor(BlendFactor_One);
    SetDestinationColorBlendFactor(BlendFactor_Zero);
    SetColorBlendFunction(BlendFunction_Add);
    SetSourceAlphaBlendFactor(BlendFactor_One);
    SetDestinationAlphaBlendFactor(BlendFactor_Zero);
    SetAlphaBlendFunction(BlendFunction_Add);
    SetChannelMask(ChannelMask_Red | ChannelMask_Green | ChannelMask_Blue | ChannelMask_Alpha);
}

/**
 * Resets the blend state to its defaults: every blend feature disabled, a no-op logic operation,
 * an opaque black blend constant and no blend targets.
 */
void BlendStateInfo::SetDefault() {
    SetAlphaToCoverageEnabled(false);
    SetDualSourceBlendEnabled(false);
    SetIndependentBlendEnabled(false);
    SetLogicOperationEnabled(false);

    SetLogicOperation(LogicOperation_NoOp);
    SetBlendConstant(0.0f, 0.0f, 0.0f, 1.0f);

    SetBlendTargetStateInfoArray(nullptr, 0);
}

/**
 * Resets the stencil state to its defaults: keep on every outcome, always pass and a reference of 0.
 */
void StencilStateInfo::SetDefault() {
    SetStencilFailOperation(StencilOperation_Keep);
    SetDepthFailOperation(StencilOperation_Keep);
    SetDepthPassOperation(StencilOperation_Keep);
    SetComparisonFunction(ComparisonFunction_Always);
    SetStencilRef(0);
}

/**
 * Resets the depth/stencil state to its defaults: a less-than depth test, all tests and writes
 * disabled, full stencil masks and default front/back stencil states.
 */
void DepthStencilStateInfo::SetDefault() {
    SetDepthComparisonFunction(ComparisonFunction_Less);

    SetDepthTestEnabled(false);
    SetDepthWriteEnabled(false);
    SetStencilTestEnabled(false);
    SetDepthBoundsTestEnabled(false);

    SetStencilReadMask(0xFF);
    SetStencilWriteMask(0xFF);

    EditFrontStencilStateInfo().SetDefault();
    EditBackStencilStateInfo().SetDefault();
}

/**
 * Resets the color target state to its defaults (an undefined format).
 */
void ColorTargetStateInfo::SetDefault() {
    SetFormat(ImageFormat_Undefined);
}

/**
 * Resets the render target state to its defaults: an undefined depth/stencil format and no color
 * targets.
 */
void RenderTargetStateInfo::SetDefault() {
    SetDepthStencilFormat(ImageFormat_Undefined);
    SetColorTargetStateInfoArray(nullptr, 0);
}

/**
 * Resets the vertex attribute state to its defaults: semantic index 0, no shader slot, buffer 0 at
 * offset 0, an undefined format and no name.
 */
void VertexAttributeStateInfo::SetDefault() {
    SetSemanticIndex(0);
    SetShaderSlot(-1);
    SetBufferIndex(0);
    SetOffset(0);

    SetFormat(AttributeFormat_Undefined);
    SetNamePtr(nullptr);
}

/**
 * Resets the vertex buffer state to its defaults (zero stride, no instancing divisor).
 */
void VertexBufferStateInfo::SetDefault() {
    SetStride(0);
    SetDivisor(0);
}

/**
 * Resets the vertex state to its defaults (no vertex attributes and no vertex buffers).
 */
void VertexStateInfo::SetDefault() {
    SetVertexAttributeStateInfoArray(nullptr, 0);
    SetVertexBufferStateInfoArray(nullptr, 0);
}

/**
 * Resets the tessellation state to its defaults (one control point per patch).
 */
void TessellationStateInfo::SetDefault() {
    SetPatchControlPointCount(1);
}

/**
 * Resets the viewport state to its defaults: an empty viewport at the origin with a [0, 1] depth
 * range.
 */
void ViewportStateInfo::SetDefault() {
    SetOriginX(0.0f);
    SetOriginY(0.0f);
    SetWidth(0.0f);
    SetHeight(0.0f);
    SetMinDepth(0.0f);
    SetMaxDepth(1.0f);
}

/**
 * Resets the scissor state to its defaults (an empty rectangle at the origin).
 */
void ScissorStateInfo::SetDefault() {
    SetOriginX(0);
    SetOriginY(0);
    SetWidth(0);
    SetHeight(0);
}

/**
 * Resets the viewport/scissor state to its defaults: scissoring disabled and no viewports or
 * scissors.
 */
void ViewportScissorStateInfo::SetDefault() {
    SetScissorEnabled(false);
    SetViewportStateInfoArray(nullptr, 0);
    SetScissorStateInfoArray(nullptr, 0);
}

}  // namespace nn::gfx
